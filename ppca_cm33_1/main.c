/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the source code for PPCA CM331 core
 *
 * Related Document: See README.md
 *
 *******************************************************************************
 * (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 ********************************************************************************/

#include "cy_pdl.h"
#include "cy_utils.h"
#include "cycfg.h"
#include "ipc_defs.h"
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Macros
 ********************************************************************************/

/* Macro to disable optimization for a function  */
#if defined(__ARMCC_VERSION)
#define CY_NO_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__)
#define CY_NO_OPTIMIZE __attribute__((optimize("O0")))
#elif defined(__ICCARM__)
#define CY_NO_OPTIMIZE _Pragma("optimize=none")
#endif

#define PPCA_IS_EVEN_BUILD(bn) (((bn) & 1U) == 0u)

/* PPCA Image Alternate Vector Offset */
#define PPCA_IMAGE_ALT_VECTOR_OFFSET                                           \
	(PPCA_IS_EVEN_BUILD(IMAGE_BUILD_NUM) ? CYMEM_CM33_0_S_ppca1_code_B_OFFSET  \
										 : CYMEM_CM33_0_S_ppca1_code_A_OFFSET)

/* PPCA Image Alternate Slot Size */
#define PPCA_IMAGE_ALT_SLOT_SIZE                                               \
    (PPCA_IS_EVEN_BUILD(IMAGE_BUILD_NUM) ? CYMEM_CM33_0_S_ppca1_code_B_SIZE    \
                                         : CYMEM_CM33_0_S_ppca1_code_A_SIZE)

/* PPCA SRAM data region used by this core */
#define PPCA_SRAM_DATA_START CYMEM_ppca1_data_START
#define PPCA_SRAM_DATA_SIZE  CYMEM_ppca1_data_SIZE

/* Distinct LED patterns used for fault indication */
#define FAILURE_BLINK_COUNT (5u)
#define FAILURE_BLINK_ON_TIME_MS   (100u)
#define FAILURE_BLINK_OFF_TIME_MS  (100u)
#define FAILURE_PATTERN_PAUSE_MS   (1900u)

#define PWM_IRQ_PRIORITY (2u)

/* IPC priority for the switch-request notify line */
#define IPC_IRQ_PRIORITY (3u)

#define LED_COUNT  (10000u)

/* IPC channel / structures used to notify the Main CPU that this core is up
 * (PPCA Core 1 -> Main CPU). */
#define PPCA_IPC_CORE_UP_STRUCT  PPCA_IPC_STRUCT1
#define PPCA_IPC_CORE_UP_CH      IPC_CH_CORE1_TO_MAIN

/* IPC channel / structures used to receive a switch request from the Main CPU
 * (Main CPU -> PPCA Core 1). */
#define PPCA_IPC_SWITCH_STRUCT      PPCA_IPC_STRUCT3
#define PPCA_IPC_SWITCH_INTR_STRUCT PPCA_IPC_INTR_STRUCT3
#define PPCA_IPC_SWITCH_CH          IPC_CH_MAIN_TO_CORE1
#define PPCA_IPC_SWITCH_IRQ         ppca_ipc_3_IRQn

/* Reset Handler function prototype */
typedef void (*reset_handler_t)(void);

typedef struct {
uint32_t start_magic;
uint32_t live_update;
uint32_t ppca1_pwm_count;
uint32_t end_magic;
}ppca_state_t;

/*******************************************************************************
 * Global Variables
 ********************************************************************************/
/* No-init live-persistent region: armclang only emits a section as ZI (NOBITS,
 * not written into the hex) when its name begins with ".bss". */
#if defined(__ARMCC_VERSION)
__attribute__((section(".bss.cy_live_persistent_data")))
#else
__attribute__((section(".cy_live_persistent_data")))
#endif
volatile ppca_state_t ppca1_state;

/* Set by the IPC notify handler when the Main CPU requests an app switch. */
static volatile bool switch_pending = false;

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

static void switch_app(void);
static void indicate_failure(void);
static bool is_valid_app_vector(uint32_t stack_pointer, uint32_t reset_vector);
static void read_alt_vector_table(uint32_t *stack_pointer_out, uint32_t *reset_vector_out);
static bool ppca_is_live_update(void);
static void ppca_state_init_default(void);
static void ipc_send_core_up(void);
static void ipc_switch_rx_init(void);
void PPCA1_PWM_Handler(void);
void PPCA1_IPC_Handler(void);

/*******************************************************************************
 * Function Name: indicate_failure
 *******************************************************************************
 * Summary:
 * Traps the core in a repeating LED pattern that identifies the failure cause.
 *
 * Parameters:
 *  None
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void indicate_failure(void)
{
    uint32_t blink_count;

    for (;;)
    {
        for (blink_count = 0u; blink_count < FAILURE_BLINK_COUNT; ++blink_count)
        {
            Cy_GPIO_Inv(CYBSP_USER_LED6_PORT, CYBSP_USER_LED6_PIN);
            Cy_SysLib_Delay(FAILURE_BLINK_ON_TIME_MS);
            Cy_GPIO_Inv(CYBSP_USER_LED6_PORT, CYBSP_USER_LED6_PIN);
            Cy_SysLib_Delay(FAILURE_BLINK_OFF_TIME_MS);
        }

        Cy_SysLib_Delay(FAILURE_PATTERN_PAUSE_MS);
    }
}

/*******************************************************************************
 * Function Name: is_valid_app_vector
 *******************************************************************************
 * Summary:
 * Performs basic vector table sanity checks before jumping to the alternate app.
 *
 * Parameters:
 *  stack_pointer - candidate MSP value from the alternate vector table
 *  reset_vector  - candidate Reset_Handler address from the alternate vector table
 *
 * Return:
 *  true if the vector contents look valid, else false
 *
 *******************************************************************************/
static bool is_valid_app_vector(uint32_t stack_pointer, uint32_t reset_vector)
{
    uint32_t reset_handler_addr = reset_vector & (~0x1u);
    uint32_t sram_data_start = (uint32_t)PPCA_SRAM_DATA_START;
    uint32_t sram_data_end = sram_data_start + (uint32_t)PPCA_SRAM_DATA_SIZE;
    uint32_t alt_slot_start = (uint32_t)PPCA_IMAGE_ALT_VECTOR_OFFSET;
    uint32_t alt_slot_end = alt_slot_start + (uint32_t)PPCA_IMAGE_ALT_SLOT_SIZE;

    /* The stack pointer must lie in this core's SRAM data region and the reset
     * handler in the alternate image slot. These range checks also exclude the
     * 0x00000000 / 0xFFFFFFFF erased-flash patterns. */
    return ((stack_pointer >= sram_data_start) &&
            (stack_pointer <= sram_data_end) &&
            (reset_handler_addr >= alt_slot_start) &&
            (reset_handler_addr < alt_slot_end));
}

/*******************************************************************************
 * Function Name: ppca_is_live_update
 *******************************************************************************
 * Summary:
 * Returns true when this boot is a live update (persistent state is valid and
 * the live-update flag is set), meaning the persistent state must be retained.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  true if this is a live update boot, else false
 *
 *******************************************************************************/
static bool ppca_is_live_update(void)
{
    return ((ppca1_state.start_magic == LIVE_DATA_START_MAGIC) &&
            (ppca1_state.end_magic == LIVE_DATA_END_MAGIC) &&
            (ppca1_state.live_update == LIVE_UPDATE_ACTIVE));
}

/*******************************************************************************
 * Function Name: ppca_state_init_default
 *******************************************************************************
 * Summary:
 * Initializes the persistent state to defaults on a cold boot.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void ppca_state_init_default(void)
{
    ppca1_state.start_magic     = LIVE_DATA_START_MAGIC;
    ppca1_state.live_update     = LIVE_UPDATE_INACTIVE;
    ppca1_state.ppca1_pwm_count = 0u;
    ppca1_state.end_magic       = LIVE_DATA_END_MAGIC;
}

/*******************************************************************************
 * Function Name: ipc_send_core_up
 *******************************************************************************
 * Summary:
 * Notifies the Main CPU over IPC that this core is up and running.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void ipc_send_core_up(void)
{
    (void)Cy_IPC_Drv_SendMsgWord(PPCA_IPC_CORE_UP_STRUCT,
                                 IPC_NOTIFY_MASK(PPCA_IPC_CORE_UP_CH),
                                 (uint32_t)IPC_MSG_CORE_UP);
}

/*******************************************************************************
 * Function Name: ipc_switch_rx_init
 *******************************************************************************
 * Summary:
 * Enables the IPC notify interrupt used to receive app-switch requests from
 * the Main CPU.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void ipc_switch_rx_init(void)
{
    Cy_IPC_Drv_SetInterruptMask(PPCA_IPC_SWITCH_INTR_STRUCT,
                                CY_IPC_NO_NOTIFICATION,
                                IPC_NOTIFY_MASK(PPCA_IPC_SWITCH_CH));
    NVIC_SetPriority(PPCA_IPC_SWITCH_IRQ, IPC_IRQ_PRIORITY);
    NVIC_EnableIRQ(PPCA_IPC_SWITCH_IRQ);
}

/*******************************************************************************
 * Function Name: PPCA1_IPC_Handler
 *******************************************************************************
 * Summary:
 * IPC notify interrupt handler. Receives the app-switch request from the Main
 * CPU and sets a flag handled in the main loop.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void PPCA1_IPC_Handler(void)
{
    uint32_t msg = (uint32_t)IPC_MSG_NONE;

    if (CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_ReadMsgWord(PPCA_IPC_SWITCH_STRUCT, &msg))
    {
        if ((ipc_msg_t)msg == IPC_MSG_SWITCH_REQ)
        {
            switch_pending = true;
        }
        (void)Cy_IPC_Drv_LockRelease(PPCA_IPC_SWITCH_STRUCT, CY_IPC_NO_NOTIFICATION);
    }

    Cy_IPC_Drv_ClearInterrupt(PPCA_IPC_SWITCH_INTR_STRUCT,
                              CY_IPC_NO_NOTIFICATION,
                              IPC_NOTIFY_MASK(PPCA_IPC_SWITCH_CH));
}

/*******************************************************************************
 * Function Name: read_alt_vector_table
 *******************************************************************************
 * Summary:
 * Reads the stack pointer and reset vector from the alternate image vector
 * table. Optimization is disabled to prevent the compiler from eliminating
 * the memory reads when the alternate slot is at address 0x0 (slot A).
 *
 * Parameters:
 *  stack_pointer_out - pointer to receive the MSP value
 *  reset_vector_out  - pointer to receive the Reset_Handler address
 *
 * Return:
 *  void
 *
 *******************************************************************************/
CY_NO_OPTIMIZE
static void read_alt_vector_table(uint32_t *stack_pointer_out, uint32_t *reset_vector_out)
{
    const uint32_t *pVTOR = (const uint32_t *)(PPCA_IMAGE_ALT_VECTOR_OFFSET);
    *stack_pointer_out = pVTOR[0];
    *reset_vector_out  = pVTOR[1];
}

/*******************************************************************************
 * Function Name: switch_app
 ********************************************************************************
 * Summary:
 * This is the function for switching the app
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void switch_app(void)
{
    uint32_t stack_pointer;
    uint32_t reset_vector;
    reset_handler_t reset_handler;

    /* Extract Stack pointer and Reset handler */
    read_alt_vector_table(&stack_pointer, &reset_vector);

    if (!is_valid_app_vector(stack_pointer, reset_vector))
    {
        indicate_failure();
    }

    reset_handler = (reset_handler_t)(reset_vector);

    /* Mark the boot as a live update so the new image retains persistent
     * state instead of re-initializing it. */
    ppca1_state.live_update = LIVE_UPDATE_ACTIVE;
    __DSB();

    /* Briefly disable interrupts only for the vector table / stack switch so
     * interrupts remain serviceable for nearly the entire switch. */
    __disable_irq();

    /* Update VTOR */
    SCB->VTOR = (uint32_t)(PPCA_IMAGE_ALT_VECTOR_OFFSET);

    /* Set MSP */
    __set_MSP(stack_pointer);
    __DSB();
    __ISB();

    __enable_irq();

    /* Launch the new App */
    reset_handler();

    /* The new image must take control and never return here. */
    indicate_failure();
}

/*******************************************************************************
 * Function Name: PPCA1_PWM_Handler
 ********************************************************************************
 * Summary:
 *  PWM (TCPWM) interrupt handler for PPCA Core 1. Uses the persistent count to
 *  toggle the user LED at a fixed cadence so the blink rate is preserved across
 *  a live firmware update.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void PPCA1_PWM_Handler(void)
{
	Cy_TCPWM_ClearInterrupt(PPCA1_PWM_HW, PPCA1_PWM_NUM, CY_TCPWM_INT_ON_CC0);
	if(ppca1_state.ppca1_pwm_count >= LED_COUNT)
	{
		ppca1_state.ppca1_pwm_count = 0;
		Cy_GPIO_Inv(CYBSP_USER_LED6_PORT, CYBSP_USER_LED6_PIN);
	}
	else
	{
		ppca1_state.ppca1_pwm_count++;
	}
	
}
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  This is the main function for PPCA CM331 CPU. It does...
*    1. Indicates the core is up and running
     2. LED 6 Toggle
*    3. Periodic check for app switch request
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    /* On a cold boot, initialize the persistent state, enable interrupts and
     * enable the IPC notify line used to receive switch requests. On a live
     * update the persistent state, the enabled interrupts and the IPC RX
     * configuration are all retained across the switch. */
    if (!ppca_is_live_update())
    {
        ppca_state_init_default();

        __enable_irq();

        /* Enable the IPC notify line used to receive switch requests and notify
         * the Main CPU that this core is up and running. */
        ipc_switch_rx_init();
    }

    ipc_send_core_up();

    /* On a cold boot, configure, enable and start the PWM. On a live update the
     * PWM (and its NVIC line) keep running across the switch, so they must not
     * be re-initialized. */
    if (!ppca_is_live_update())
    {
        /* Configure PPCA1 PWM */
        if (CY_TCPWM_SUCCESS != Cy_TCPWM_PWM_Init(PPCA1_PWM_HW, PPCA1_PWM_NUM, &PPCA1_PWM_config))
        {
            CY_ASSERT(0);
        }

        /* Enable the initialized PWM */
        Cy_TCPWM_PWM_Enable(PPCA1_PWM_HW, PPCA1_PWM_NUM);

        NVIC_SetPriority(PPCA1_PWM_IRQ, PWM_IRQ_PRIORITY);
        NVIC_EnableIRQ(PPCA1_PWM_IRQ);

        /* Start the PWM */
        Cy_TCPWM_TriggerStart_Single(PPCA1_PWM_HW, PPCA1_PWM_NUM);
    }

    for (;;)
    {
        if (switch_pending)
        {
            switch_pending = false;
            switch_app();
        }
        __WFI();
    }
}
