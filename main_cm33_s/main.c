/*****************************************************************************
 * File Name        : main.c
 *
 * Description: This is the source code for Main CM33 secure application
 *
 * Related Document : See README.md
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

/******************************************************************************
 * Header Files
 *****************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg_peripherals.h"
#include "retarget_io_init.h"

#include "partition_ARMCM33.h"
#include "partition_psc3.h"

#include "transport_pmbus.h"
#include "cy_dfu.h"
#include "cy_dfu_logging.h"
#include "mtb_hal_i2c.h"
#include "cy_scb_i2c.h"
#include "cy_sysint.h"
#include "cybsp.h"

#include "ppca_init.h"
#include "cycfg_pmbus.h"
#include "ipc_defs.h"
#include "device_state.h"

/*******************************************************************************
 * Macros
 *******************************************************************************/

/* Timeout for Cy_DFU_Continue(), in milliseconds */
#define DFU_SESSION_TIMEOUT_MS (1u)

/* DFU idle timeout: 300 seconds */
#define DFU_IDLE_TIMEOUT_MS (300000u)

/* DFU command timeout: 5 seconds */
#define DFU_COMMAND_TIMEOUT_MS (5000u)

/* Image Magic */
#define IMAGE_MAGIC (0x96F3B83D)

/* Image version magic */
#define IMAGE_VER_MAGIC (0x5A3C)

#define TIMER_IRQ_PRIORITY (2u)

/* Priority of the IPC notify interrupts used to communicate with the PPCA cores */
#define IPC_IRQ_PRIORITY   (3u)

#define LED_COUNT  (10000u)

/*******************************************************************************
 * Defines
 *******************************************************************************/

/** Image version.  All fields are in little endian. */
typedef struct
{
    uint8_t iv_major;
    uint8_t iv_minor;
    uint16_t iv_revision;
    uint16_t iv_build_num;
    uint16_t iv_magic;
} image_version_t;

/** Image header.  All fields are in little endian byte order. */
typedef struct
{
    uint32_t ih_magic;
    uint32_t ih_load_addr;
    uint16_t ih_hdr_size;         /* Size of image header (bytes). */
    uint16_t ih_protect_tlv_size; /* Size of protected TLV area (bytes). */
    uint32_t ih_img_size;         /* Does not include header. */
    uint32_t ih_flags;            /* IMAGE_F_[...]. */
    image_version_t ih_ver;
    uint32_t _pad1;
} image_header_t;

/* Reset Handler function prototype */
typedef void (*reset_handler_t)(void);

/*******************************************************************************
 * Function Prototype
 *******************************************************************************/

static char *dfu_status_in_str(cy_en_dfu_status_t dfu_status);

static bool validate_image(void);
static void switch_app(void);

/* Waits for any pending DFU (RWW) flash write to complete; defined in dfu_user.c. */
extern cy_en_dfu_status_t app_dfu_flash_wait_complete(void);

/* Non-blocking erase of the staging-bank header row on failed validation;
 * defined in dfu_user.c. */
extern cy_en_dfu_status_t app_dfu_erase_staging_header(void);

void dfu_pmbus_isr(void);
static void dfu_pmbus_transport_init(void);
void Main_Timer_Handler(void);
void Main_IPC0_Handler(void);
void Main_IPC1_Handler(void);
static void ppca_ipc_rx_init(void);
static void ppca_state_init_default(void);
bool is_live_update(void);
cy_rslt_t safe_bsp_init(void);

/*******************************************************************************
 * Global Variables
 *******************************************************************************/

/* DFU command data storage */
uint8_t DfuCmdData[DFU_PMBUS_BUFFER_SIZE];

mtb_pmbus_stc_t             dfuPMBusObj;
cy_stc_scb_i2c_context_t    dfuPMBusContext;

mtb_pmbus_stc_config_hal_t dfu_pmbus_hal_config =
{
    .hw_ptr = dfu_pmbus_I2C_HW,
    .pdl_i2c_context = &dfuPMBusContext,
};

/* Image Headers */
static const image_header_t *pImgHdr = (image_header_t *)CY_FLASH_BASE;
static const image_header_t *pNewImgHdr = (image_header_t *)CY_DUAL_FLASH_S_SBUS_BASE;

/* Placed in the no-init live-persistent region.
 * armclang only emits a section as ZI (NOBITS, i.e. NOT written into the image/
 * hex) when its name begins with ".bss"; a plain named section is PROGBITS and
 * its zero bytes would be programmed. GCC instead relies on its (NOLOAD) output
 * section. The scatter/linker selects this section and marks it UNINIT so it is
 * also not zeroed at startup. */
#if defined(__ARMCC_VERSION)
__attribute__((section(".bss.cy_live_persistent_data")))
#else
__attribute__((section(".cy_live_persistent_data")))
#endif
volatile device_state_t device_state;

/*******************************************************************************
 * Function Name: dfu_status_in_str
 ********************************************************************************
 * Summary:
 *  This is the function to convert DFU status in elaborative text
 *
 * Parameters:
 *  dfu_status
 *
 * Return:
 *  string pointer
 *
 *******************************************************************************/
static char *dfu_status_in_str(cy_en_dfu_status_t dfu_status)
{
    switch (dfu_status)
    {
        case CY_DFU_SUCCESS:
            return "Success";

        case CY_DFU_ERROR_VERIFY:
            return "Packet verification failed";

        case CY_DFU_ERROR_LENGTH:
            return "The length of the packet is outside of the expected range";

        case CY_DFU_ERROR_DATA:
            return "The data in the received packet is invalid";

        case CY_DFU_ERROR_CMD:
            return "The command is not recognized";

        case CY_DFU_ERROR_CHECKSUM:
            return "The checksum does not match the expected value ";

        case CY_DFU_ERROR_ADDRESS:
            return "Wrong address";

        case CY_DFU_ERROR_TIMEOUT:
            return "The command timed out";

        case CY_DFU_ERROR_BAD_PARAM:
            return "One or more of input parameters are invalid";

        case CY_DFU_ERROR_UNKNOWN:
            return "Unkown error";

        default:
            return "Unkown error";
    }
}

/*******************************************************************************
 * Function Name: validate_image
 ********************************************************************************
 * Summary:
 *  This function validates the update image staged in the alternate bank.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  The status - true(Valid) or false(Invalid).
 *
 *******************************************************************************/
static bool validate_image(void)
{
    uint32_t *pVTOR;
    bool valid = false;

    if (pNewImgHdr->ih_magic == IMAGE_MAGIC)
    {
        if ((pNewImgHdr->ih_ver.iv_magic == IMAGE_VER_MAGIC) &&
            (pNewImgHdr->ih_ver.iv_build_num > pImgHdr->ih_ver.iv_build_num) &&
            ((pNewImgHdr->ih_ver.iv_build_num ^ pImgHdr->ih_ver.iv_build_num) & 1U))
        {
            pVTOR = (uint32_t *)(CY_DUAL_FLASH_S_SBUS_BASE + pNewImgHdr->ih_hdr_size);

            if ((pVTOR[0] != 0U) && (pVTOR[1] != 0U) && ((pVTOR[1] & 0x1U) == 0x1U))
            {
                valid = true;
            }
        }
    }

    return valid;
}

/*******************************************************************************
 * Function Name: switch_app
 ********************************************************************************
 * Summary:
 * This is the function for switching the image
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
CY_SECTION_RAMFUNC_BEGIN CY_NOINLINE static void switch_app(void)
{
    uint32_t *pVTOR;
    uint32_t stack_pointer;
    uint32_t flash_vt_base;
    reset_handler_t reset_handler;

    /* Extract Stack pointer and Reset handler from the staged (alternate bank)
     * image. Because both banks are linked identically, these values are the
     * correct addresses once the bank mapping is toggled. */
    pVTOR = (uint32_t *)(CY_DUAL_FLASH_S_SBUS_BASE + pNewImgHdr->ih_hdr_size);
    stack_pointer = pVTOR[0];
    reset_handler = (reset_handler_t)(pVTOR[1]);

    /* After the bank swap, the new image's vector table is mapped at
     * CY_FLASH_BASE + header size. The new image's Reset_Handler relies on
     * VTOR pointing here so that interrupts remain serviceable while it
     * repopulates the SRAM vector table (see startup live-update path). */
    flash_vt_base = (uint32_t)CY_FLASH_BASE + pNewImgHdr->ih_hdr_size;

    /* Mark the boot as a live update so the new image takes the fast path:
     * skip clock/system re-init and skip .bss/.data re-init, preserving the
     * running peripherals and persistent state. */
    device_state.live_update = LIVE_UPDATE_ACTIVE;
    __DSB();

    /* Minimize the IRQ-off window to just the bank remap + VTOR/MSP switch. */
    __disable_irq();

    /* Toggle the Flash bank mapping to make the staged image the active bank */
    FLASHC_FLASH_CTL ^= (1u << FLASHC_FLASH_CTL_BANK_MAPPING_Pos);
    __DSB();

    /* Invalidate cache so instruction fetches come from the newly mapped bank */
    ICACHE0->CMD = ICACHE0->CMD | ICACHE_CMD_INV_Msk;
    /* wait for invalidation complete */
    while (ICACHE0->CMD & ICACHE_CMD_INV_Msk)
    {
    }

    /* Point VTOR at the new image's flash vector table and set its stack */
    SCB->VTOR = flash_vt_base;
    __set_MSP(stack_pointer);
    __DSB();
    __ISB();

    /* Re-enable interrupts: they are now serviced through the new image's
     * flash vector table until its Reset_Handler switches to the SRAM table. */
    __enable_irq();

    /* Launch the new App */
    reset_handler();

    /* Should not reach here */
    CY_ASSERT(0);
}
CY_SECTION_RAMFUNC_END

/*******************************************************************************
 * Function Name: dfu_pmbus_isr
 ********************************************************************************
 * Summary:
 *  DFU PMBus hardware interrupt callback
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_isr(void)
{
    mtb_pmbus_i2c_isr(&dfuPMBusObj);
}

/*******************************************************************************
 * Function Name: dfu_pmbus_transport_init
 ********************************************************************************
 * Summary:
 *  Configure DFU PMBus transport to receive data from DFU Host Tool
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void dfu_pmbus_transport_init(void)
{
    cy_en_scb_i2c_status_t pdlStatus;
    cy_en_sysint_status_t  sysIntStatus;
    mtb_pmbus_status_t     pmbusStatus;
    pdlStatus = Cy_SCB_I2C_Init(dfu_pmbus_I2C_HW, &dfu_pmbus_I2C_config, &dfuPMBusContext);
    if (CY_SCB_I2C_SUCCESS != pdlStatus)
    {
       CY_DFU_LOG_ERR("An error occurred during PMBus PDL initialization. Status: %X", pdlStatus);
    }
    else
    {
        cy_stc_sysint_t pmbusIsrCfg =
        {
           .intrSrc = dfu_pmbus_I2C_IRQ,
           .intrPriority = 3U,
        };
        sysIntStatus = Cy_SysInt_Init(&pmbusIsrCfg, dfu_pmbus_isr);
        if (CY_SYSINT_SUCCESS != sysIntStatus)
        {
            CY_DFU_LOG_ERR("An error occurred during PMBus interrupt initialization. Status: %X", sysIntStatus);
        }
        else
        {
            pmbusStatus = mtb_pmbus_init(&dfuPMBusObj, &dfu_pmbus_config);
            if (MTB_PMBUS_STATUS_SUCCESS != pmbusStatus)
            {
                CY_DFU_LOG_ERR("An error occurred during PMBus initialization. Status: %X", pmbusStatus);
            }
            else
            {
                cy_stc_dfu_transport_pmbus_cfg_t pmbusTransportCfg =
                {
                    .pmbus = &dfuPMBusObj,
                    .cmdCode = dfu_pmbus_config.cmd_table[0].cmd_code,
                    .cmdData = DfuCmdData,
                };
                Cy_DFU_TransportPMBusConfig(&pmbusTransportCfg);
                CY_DFU_LOG_INF("PMBus initialization was successful.");
            }
        }
    }
}

/*******************************************************************************
 * Function Name: dfu_pmbus_hw_irq_enable
 ********************************************************************************
 * Summary:
 *  Callback to enable DFU PMBus hardware interrupt
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_hw_irq_enable(void)
{
    NVIC_EnableIRQ((IRQn_Type) dfu_pmbus_I2C_IRQ);
    CY_DFU_LOG_INF("PMBus IRQ is enabled");
}

/*******************************************************************************
 * Function Name: dfu_pmbus_hw_irq_disable
 ********************************************************************************
 * Summary:
 *  Callback to disable DFU PMBus hardware interrupt
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_hw_irq_disable(void)
{
    NVIC_DisableIRQ((IRQn_Type) dfu_pmbus_I2C_IRQ);
    CY_DFU_LOG_INF("PMBus IRQ is disabled");
}

/*******************************************************************************
 * Function Name: dfu_pmbus_hw_resource_ctrl
 ********************************************************************************
 * Summary:
 *  Callback to enable or disable DFU PMBus transport
 *
 * Parameters:
 *  action : Callback trigger
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_hw_resource_ctrl(mtb_pmbus_hw_resources_ctrl_action_t action)
{
    if (action == MTB_PMBUS_HW_RESOURCES_ENABLE)
    {
        Cy_SCB_I2C_Enable(dfu_pmbus_I2C_HW);
        CY_DFU_LOG_INF("PMBus transport is enabled");
    }
    else if (action == MTB_PMBUS_HW_RESOURCES_DISABLE)
    {
        Cy_SCB_I2C_Disable(dfu_pmbus_I2C_HW, &dfuPMBusContext);
        CY_DFU_LOG_INF("PMBus transport is disabled");
    }
}

/*******************************************************************************
 * Function Name: dfu_pmbus_gen_callback
 ********************************************************************************
 * Summary:
 *  DFU PMBus general callback
 *
 * Parameters:
 *  event : Event that triggered the callback
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_gen_callback(mtb_pmbus_events_t event)
{
    CY_UNUSED_PARAM(event);
}

/*******************************************************************************
 * Function Name: dfu_pmbus_error_callback
 ********************************************************************************
 * Summary:
 *  DFU PMBus error callback
 *
 * Parameters:
 *  events : Event that triggered the callback
 *  cmd_code : Command code for which the error occurred
 *  cmd_is_ext : Whether the command is an extended command
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void dfu_pmbus_error_callback(uint32_t events, uint8_t cmd_code, bool cmd_is_ext)
{
    CY_UNUSED_PARAM(events);
    CY_UNUSED_PARAM(cmd_code);
    CY_UNUSED_PARAM(cmd_is_ext);
}


/*******************************************************************************
 * Function Name: Main_Timer_Handler
 ********************************************************************************
 * Summary:
 *  Periodic timer (TCPWM) interrupt handler for the main core. Uses the
 *  persistent counter to toggle the user LED at a fixed cadence so the blink
 *  rate is preserved across a live firmware update.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void Main_Timer_Handler(void)
{
    Cy_TCPWM_ClearInterrupt(MAIN_TC_HW, MAIN_TC_NUM, CY_TCPWM_INT_ON_TC);

    /* Use the persistent counter so the LED cadence is preserved across a live
     * firmware update (the timer keeps running and this handler keeps being
     * serviced through the new image's vector table). */
    if (device_state.main_timer_count >= LED_COUNT)
    {
        device_state.main_timer_count = 0u;
        Cy_GPIO_Inv(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN);
    }
    else
    {
        device_state.main_timer_count++;
    }
}

/*******************************************************************************
 * Function Name: Main_IPC0_Handler
 ********************************************************************************
 * Summary:
 *  IPC notify interrupt handler for messages received from PPCA Core 0
 *  (channel 0: PPCA_IPC_STRUCT0 / PPCA_IPC_INTR_STRUCT0).
 *
 *******************************************************************************/
void Main_IPC0_Handler(void)
{
    uint32_t msg = (uint32_t)IPC_MSG_NONE;

    if (CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_ReadMsgWord(PPCA_IPC_STRUCT0, &msg))
    {
        if (((ipc_msg_t)msg == IPC_MSG_CORE_UP))
        {
            device_state.ppca0_state = PPCA_STATE_RUNNING;
        }

        /* Release the channel so the core can send the next message */
        (void)Cy_IPC_Drv_LockRelease(PPCA_IPC_STRUCT0, CY_IPC_NO_NOTIFICATION);
    }

    /* Clear the notify interrupt for this channel */
    Cy_IPC_Drv_ClearInterrupt(PPCA_IPC_INTR_STRUCT0, CY_IPC_NO_NOTIFICATION,
                              IPC_NOTIFY_MASK(IPC_CH_CORE0_TO_MAIN));
}

/*******************************************************************************
 * Function Name: Main_IPC1_Handler
 ********************************************************************************
 * Summary:
 *  IPC notify interrupt handler for messages received from PPCA Core 1
 *  (channel 1: PPCA_IPC_STRUCT1 / PPCA_IPC_INTR_STRUCT1).
 *
 *******************************************************************************/
void Main_IPC1_Handler(void)
{
    uint32_t msg = (uint32_t)IPC_MSG_NONE;

    if (CY_IPC_DRV_SUCCESS == Cy_IPC_Drv_ReadMsgWord(PPCA_IPC_STRUCT1, &msg))
    {
        if (((ipc_msg_t)msg == IPC_MSG_CORE_UP))
        {
            device_state.ppca1_state = PPCA_STATE_RUNNING;
        }

        (void)Cy_IPC_Drv_LockRelease(PPCA_IPC_STRUCT1, CY_IPC_NO_NOTIFICATION);
    }

    Cy_IPC_Drv_ClearInterrupt(PPCA_IPC_INTR_STRUCT1, CY_IPC_NO_NOTIFICATION,
                              IPC_NOTIFY_MASK(IPC_CH_CORE1_TO_MAIN));
}

/*******************************************************************************
 * Function Name: ppca_ipc_rx_init
 ********************************************************************************
 * Summary:
 *  Configures the IPC notify interrupts the Main CPU uses to receive messages
 *  from the PPCA cores (CORE_UP / SWITCH_ACK). The interrupt vectors are
 *  provided by the (flash) vector table, so only the IPC interrupt masks and
 *  the NVIC priority/enable need to be configured here.
 *
 *******************************************************************************/
static void ppca_ipc_rx_init(void)
{
    /* Enable notify events from PPCA Core 0 (channel 0) and Core 1 (channel 1). */
    Cy_IPC_Drv_SetInterruptMask(PPCA_IPC_INTR_STRUCT0, CY_IPC_NO_NOTIFICATION,
                                IPC_NOTIFY_MASK(IPC_CH_CORE0_TO_MAIN));
    Cy_IPC_Drv_SetInterruptMask(PPCA_IPC_INTR_STRUCT1, CY_IPC_NO_NOTIFICATION,
                                IPC_NOTIFY_MASK(IPC_CH_CORE1_TO_MAIN));

    NVIC_SetPriority(ppca_ipc_0_IRQn, IPC_IRQ_PRIORITY);
    NVIC_SetPriority(ppca_ipc_1_IRQn, IPC_IRQ_PRIORITY);
    NVIC_EnableIRQ(ppca_ipc_0_IRQn);
    NVIC_EnableIRQ(ppca_ipc_1_IRQn);
}


/*******************************************************************************
 * Function Name: ppca_state_init_default
 ********************************************************************************
 * Summary:
 *  Initializes the persistent device state to its cold-boot defaults (magics,
 *  inactive live-update flag, PPCA cores off and timer count cleared).
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
    device_state.start_magic = LIVE_DATA_START_MAGIC;
    device_state.live_update = LIVE_UPDATE_INACTIVE;
    device_state.ppca0_state = (uint32_t)PPCA_STATE_OFF;
    device_state.ppca1_state = (uint32_t)PPCA_STATE_OFF;
    device_state.main_timer_count = 0;
    device_state.end_magic  = LIVE_DATA_END_MAGIC;
}




/*******************************************************************************
 * Function Name: safe_bsp_init
 ********************************************************************************
 * Summary:
 *  Placeholder peripheral re-init for the live-update boot path. Intended for
 *  re-initialization that must not disturb the PPCA application (running the
 *  previous boot image) or its peripherals.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  CY_RSLT_SUCCESS on success.
 *
 *******************************************************************************/
cy_rslt_t safe_bsp_init(void)
{
	/* Add peripheral re-init code here. Ensure that PPCA application (running previous
	 * boot image) and its peripherals are not impacted */
	return CY_RSLT_SUCCESS;
}

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  This is the main function for Main CM33s CPU. It does...
 *    1. Device/Peripheral Initialization
 *    2. Starts PPCA Cores
 *    3. Toggles LED 1
 *    4. Receives update image through DFU MW and stages it in the alternate bank
 *    5. Validates the update image and switches to it.
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
    cy_rslt_t result;
    uint32_t count = 0;
    cy_en_dfu_status_t dfu_status = CY_DFU_ERROR_UNKNOWN;
    uint32_t dfu_state = CY_DFU_STATE_NONE;
    bool dfu_started = false;

    /* Buffer to store DFU commands. */
    CY_ALIGN(4)
    static uint8_t dfu_buffer[CY_DFU_SIZEOF_DATA_BUFFER];
    /* Buffer for DFU data packets for transport API. */
    CY_ALIGN(4)
    static uint8_t dfu_packet[CY_DFU_SIZEOF_CMD_BUFFER];

    /* DFU params, used to configure DFU. */
    cy_stc_dfu_params_t dfu_params =
    {
        .timeout = DFU_SESSION_TIMEOUT_MS,
        .dataBuffer = &dfu_buffer[0],
        .packetBuffer = &dfu_packet[0],
    };

	if(!is_live_update())
	{
		ppca_state_init_default();

    	/* Initialize the device and board peripherals */
		result = cybsp_init();

    	/* Board init failed. Stop program execution */
    	if (result != CY_RSLT_SUCCESS)
   	 	{
        	CY_ASSERT(0);
    	}

    	/* enable interrupts */
    	__enable_irq();
	}
	else
	{
    	/* Initialize the device and board peripherals */
    	/* Note: If PPCA cores are still running from a previous boot, carefully consider
		 * the peripherals they are using. If the new image changes peripheral configurations
		 * that affect PPCA operation, proceed with caution. The correct handling is use case
		 * specific and is NOT shown in this example. */
		result = safe_bsp_init();

    	/* Board init failed. Stop program execution */
    	if (result != CY_RSLT_SUCCESS)
   	 	{
        	CY_ASSERT(0);
    	}
	}   

    /* Initialize retarget-io middleware */
    init_retarget_io();

	if(!is_live_update())
	{
		/* Transmit header to the terminal */
   		/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    	printf("\x1b[2J\x1b[;H");
    	printf("*****************************************************************\r\n");
    	printf("        PSOC Control C3M8: Multicore Live Firmware Update        \r\n");
    	printf("            Image Version: %u.%u.%u+%u\r\n", pImgHdr->ih_ver.iv_major,
           	pImgHdr->ih_ver.iv_minor, pImgHdr->ih_ver.iv_revision, pImgHdr->ih_ver.iv_build_num);
    	printf("*****************************************************************\r\n");

		if (CY_TCPWM_SUCCESS != Cy_TCPWM_Counter_Init(MAIN_TC_HW, MAIN_TC_NUM, &(MAIN_TC_config)))
    	{
        	CY_ASSERT(0);
    	}

    	/* Enable the initialized counter */
    	Cy_TCPWM_Counter_Enable(MAIN_TC_HW, MAIN_TC_NUM);

		/* The Main_Timer_Handler vector is provided by the (flash) vector table,
		 * so only the NVIC priority/enable is configured here. */
		NVIC_SetPriority(MAIN_TC_IRQ, TIMER_IRQ_PRIORITY);
		NVIC_EnableIRQ(MAIN_TC_IRQ);

		/* Then start the counter */
    	Cy_TCPWM_TriggerStart_Single(MAIN_TC_HW, MAIN_TC_NUM);
	}
	else
	{
		printf("\x1b[2J\x1b[;H");
    	printf("*****************************************************************\r\n");
    	printf("        Live Update: Booting New Image        \r\n");
    	printf("            Image Version: %u.%u.%u+%u\r\n", pImgHdr->ih_ver.iv_major,
           	pImgHdr->ih_ver.iv_minor, pImgHdr->ih_ver.iv_revision, pImgHdr->ih_ver.iv_build_num);
    	printf("*****************************************************************\r\n");
	}

    /* Configure the IPC notify interrupts used to receive status from the
     * PPCA cores before they are (re)started. */
    ppca_ipc_rx_init();

    /* Start PPCA Cores */
    PPCA_Init();

    /* Initialize DFU MW */
    dfu_status = Cy_DFU_Init(&dfu_state, &dfu_params);
    if (CY_DFU_SUCCESS != dfu_status)
    {
        printf("DFU MW init failed \r\n");
        CY_ASSERT(0);
    }

    /* Initialize DFU communication. */
    printf("\r\n Starting DFU Transport \r\n\n");

    dfu_pmbus_transport_init();
    Cy_DFU_TransportStart(CY_DFU_PMBUS);

	/* Init Flash to write new image */
	Cy_Flash_Init(true);

    for (;;)
    {
        dfu_status = Cy_DFU_Continue(&dfu_state, &dfu_params);
        count++;
        if (CY_DFU_STATE_FINISHED == dfu_state)
        {
            printf("    [DFU] Image download complete\r\n");

            /* Complete the final pipelined flash write (RWW) before reading back
             * the staged image, and fail the update if any write errored. */
            if ((app_dfu_flash_wait_complete() == CY_DFU_SUCCESS) && (validate_image() == true))
            {
                printf("    [DFU] Image validation success. Switching to new image\r\n");
                while (false == (Cy_SCB_UART_IsTxComplete(DEBUG_UART_HW)));

                switch_app();
            }

            /* Restart */
            printf("    [DFU] Image validation failed\r\n");

            /* Invalidate the staged image: non-blocking erase of the first
             * staging-bank row (holds the image magic + version) so the next
             * download starts clean. The erase is completed by the RWW gate on
             * the first write after DFU restarts. */
            (void)app_dfu_erase_staging_header();

            count = 0u;
            dfu_started = false;

            printf("\r\n Re-starting DFU Transport\r\n\n");
            Cy_DFU_Init(&dfu_state, &dfu_params);
            Cy_DFU_TransportReset();
        }
        else if (CY_DFU_STATE_FAILED == dfu_state)
        {
            printf("    [DFU] Image download failed, %s \r\n", dfu_status_in_str(dfu_status));

            /* An error occurred. Handle it here.
             * This code just restarts the DFU */
            count = 0u;
            dfu_started = false;
            Cy_DFU_Init(&dfu_state, &dfu_params);
            Cy_DFU_TransportReset();
        }
        else if (dfu_state == CY_DFU_STATE_UPDATING)
        {
            if (dfu_status == CY_DFU_SUCCESS)
            {
                if (dfu_started == false)
                {
                    printf("    [DFU] Received DFU request, starting image download\r\n");
                    dfu_started = true;
                }
                count = 0u;
            }
            else if (dfu_status == CY_DFU_ERROR_TIMEOUT)
            {
                if (count >= (DFU_COMMAND_TIMEOUT_MS / DFU_SESSION_TIMEOUT_MS))
                {
                    /* No command has been received since last 5 seconds. Restart DFU */
                    printf("    [DFU] Image download failed, %s\r\n", dfu_status_in_str(dfu_status));
                    count = 0u;
                    dfu_started = false;
                    Cy_DFU_Init(&dfu_state, &dfu_params);
                    Cy_DFU_TransportReset();
                }
            }
            else
            {
                /* Handle other errors */
                printf("    [DFU] Image download failed, %s\r\n", dfu_status_in_str(dfu_status));

                /* Delay because Transport still may be sending error response to a host. */
                Cy_SysLib_Delay(DFU_SESSION_TIMEOUT_MS);

                /* Restart DFU. */
                count = 0u;
                dfu_started = false;
                Cy_DFU_Init(&dfu_state, &dfu_params);
                Cy_DFU_TransportReset();
            }
        }
        else
        {
            /* dfu_state == CY_DFU_STATE_NONE */
            if (count >= (DFU_IDLE_TIMEOUT_MS / DFU_SESSION_TIMEOUT_MS))
            {
                /* No DFU request received in 300 seconds, lets start over.
                 * Final application can change it to either assert, reboot,
                 * enter low power mode etc, based on usecase requirements. */
                count = 0;
            }

            Cy_DFU_Init(&dfu_state, &dfu_params);
        }

        Cy_SysLib_Delay(1);
    }
}
