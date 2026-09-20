/*******************************************************************************
 * File Name:   ppca_init.c
 *
 * Description: This file contains the initialization routine for the
 *              PPCA Cores
 *
 * Related Document:
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
 *******************************************************************************/
#include "cy_device.h"
#include "cy_device_headers.h"
#include "cmsis_compiler.h"
#include "cy_syslib.h"
#include "cy_pdl.h"
#include "cy_system_ppca_init.h"
#include "cy_syspm_ppu.h"

#include "ppca_init.h"
#include "ipc_defs.h"
#include <stdio.h>
#include "cymem_CM33_0_S.h"
#include "device_state.h"

/*******************************************************************************
 * Macros
 ********************************************************************************/

#define PPCA_IS_EVEN_BUILD(bn) (((bn) & 1U) == 0u)

/* PPCA Image Size */
#if ((CYMEM_CM33_0_S_ppca0_code_A_SIZE != CYMEM_CM33_0_S_ppca0_code_B_SIZE) || \
     (CYMEM_CM33_0_S_ppca1_code_A_SIZE != CYMEM_CM33_0_S_ppca1_code_B_SIZE) || \
     (CYMEM_CM33_0_S_ppca0_code_A_SIZE != CYMEM_CM33_0_S_ppca1_code_A_SIZE))
#error "PPCA code sizes must match"
#else
#define PPCA_IMAGE_SIZE  (CYMEM_CM33_0_S_ppca0_code_A_SIZE)
#endif

/* PPCA Image Vector Offset */
#define PPCA_IMAGE_VECTOR_OFFSET                                               \
	(PPCA_IS_EVEN_BUILD(IMAGE_BUILD_NUM) ? CYMEM_CM33_0_S_ppca0_code_A_OFFSET  \
										 : CYMEM_CM33_0_S_ppca0_code_B_OFFSET)

/* PPCA Core Images Flash Addresses */
#define PPCA_CORE0_IMAGE_ADDRESS (CYMEM_CM33_0_S_ppca0_nvm_C_S_START)
#define PPCA_CORE1_IMAGE_ADDRESS (CYMEM_CM33_0_S_ppca1_nvm_C_S_START)

/* Time to wait for enabling PPCA */
#define PPCA_ENABLE_WAIT_TIME_US (100u)

/* Timeout for copying PPCA image */
#define PPCA_IMAGE_COPY_TIMEOUT_MS (1000u)

/* Timeout to wait for a PPCA core to report it is up and running */
#define PPCA_CORE_UP_TIMEOUT_MS (1000u)

/* IPC channels used to request a PPCA core to switch to the new image
 * (Main CPU -> PPCA core, see ipc_defs.h). */
#define PPCA_IPC_SWITCH_REQ_CORE0 PPCA_IPC_STRUCT2
#define PPCA_IPC_SWITCH_REQ_CORE1 PPCA_IPC_STRUCT3

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
/* xCount is hardcoded to 256 bytes. and yCount will vary based on the image size. */
#define XCOUNT_VALUE (256UL)
static cy_stc_dma_descriptor_config_t descriptor_config =
{
    .retrigger = CY_DMA_RETRIG_IM,
    .interruptType = CY_DMA_DESCR_CHAIN,
    .triggerOutType = CY_DMA_DESCR,
    .channelState = CY_DMA_CHANNEL_ENABLED,
    .triggerInType = CY_DMA_DESCR,
    .dataSize = CY_DMA_WORD,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .descriptorType = CY_DMA_2D_TRANSFER,
    .srcAddress = NULL,
    .dstAddress = NULL,
    .srcXincrement = 1,
    .dstXincrement = 1,
    .xCount = XCOUNT_VALUE,
    .srcYincrement = (int32_t)XCOUNT_VALUE,
    .dstYincrement = (int32_t)XCOUNT_VALUE,
    .yCount = 32,
    .nextDescriptor = NULL,
};
static cy_stc_dma_descriptor_t descriptor_0 =
{
    .ctl = 0UL,
    .src = 0UL,
    .dst = 0UL,
    .xCtl = 0UL,
    .yCtl = 0UL,
    .nextPtr = 0UL,
};
static cy_stc_dma_channel_config_t channelConfig =
{
    .descriptor = &descriptor_0,
    .preemptable = true,
    .priority = 3,
    .enable = false,
    .bufferable = true,
};

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/

static void PPCA_EnableCM33_0(uint32_t vectorTableOffset, uint32_t waitus);
static void PPCA_EnableCM33_1(uint32_t vectorTableOffset, uint32_t waitus);
static void PPCA_ImageCopy(cy_en_ppca_core_t core_number);
void static PPCA_StartCore(cy_en_ppca_core_t core_number);
static bool PPCA_WaitCoreRunning(uint32_t core, uint32_t timeout_ms);
static void PPCA_RequestSwitch(uint32_t core);

/*******************************************************************************
 * Function Name: PPCA_EnableCM33_0
 ******************************************************************************
 *
 * Sets vector table base address and enables the Cortex-M33 core0.
 *
 * \note If the CPU is already enabled, it is reset and then enabled.
 *
 * \param vectorTableOffset The offset of the vector table base address from
 * memory address 0x00000000. The offset should be multiple to 512 bytes.
 * \param waitus The timeout value in microsecond used to wait for core to be
 * booted. value zero is for infinite wait till the core is booted successfully.
 *
 *******************************************************************************/
static void PPCA_EnableCM33_0(uint32_t vectorTableOffset, uint32_t waitus)
{
    uint32_t interruptState;
    uint32_t regValue;

    /* Protect only the keyed enable/reset register write. The boot wait loop
     * below must NOT run with interrupts disabled (it can take many us), so the
     * critical section is closed immediately after the keyed write. */
    interruptState = Cy_SysLib_EnterCriticalSection();

    PPCA_CPUSS_CNFG_MXCM330->CM33_NS_VECTOR_TABLE_BASE = vectorTableOffset;

    regValue = PPCA_CPUSS_CNFG_MXCM330->CM33_CMD & ~(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_ENABLED_Msk | PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_VECTKEYSTAT_Msk);
    regValue |= _VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_VECTKEYSTAT, CY_SYS_CORE_PWR_CTL_KEY_OPEN); /* set key bits */
    regValue |= _VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_ENABLED, CY_SYS_CM33_IN_RESET);             /* reset enable bit */
    PPCA_CPUSS_CNFG_MXCM330->CM33_CMD = regValue;

    Cy_SysLib_ExitCriticalSection(interruptState);

    if (waitus == CY_SYS_CORE_WAIT_INFINITE)
    {
        while (Cy_SysGetCM33_0_Status() != CY_SYS_CORE_STATUS_ACTIVE)
        {
            /* Wait for the power mode to take effect */
        }
    }
    else
    {
        while ((Cy_SysGetCM33_0_Status() != CY_SYS_CORE_STATUS_ACTIVE) && (waitus))
        {
            Cy_SysLib_DelayUs(1u);
            waitus--;
        }

        if (waitus == 0u)
        {
            printf("\r\n  Failed to enable PPCA CM330\r\n");
        }
    }

    PPCA_CPUSS_CNFG_MXCM330->CM33_CTL &= ~(_VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CTL_CPU_WAIT, CY_SYS_CPU_WAIT_DEFAULT));

    /* Enable Event from other CM33 */
    PPCA_CPUSS_CNFG_MXCM330->CM33_EVENT_CTL = CM33_CPU_EVENT;
}

/*******************************************************************************
 * Function Name: PPCA_EnableCM33_1
 ******************************************************************************
 *
 * Sets vector table base address and enables the Cortex-M33 core1.
 *
 * \note If the CPU is already enabled, it is reset and then enabled.
 *
 * \param vectorTableOffset The offset of the vector table base address from
 * memory address 0x00000000. The offset should be multiple to 512 bytes.
 * \param waitus The timeout value in microsecond used to wait for core to be
 * booted. value zero is for infinite wait till the core is booted successfully.
 *
 *******************************************************************************/
static void PPCA_EnableCM33_1(uint32_t vectorTableOffset, uint32_t waitus)
{
    uint32_t interruptState;
    uint32_t regValue;

    /* Protect only the keyed enable/reset register write. The boot wait loop
     * below must NOT run with interrupts disabled (it can take many us), so the
     * critical section is closed immediately after the keyed write. */
    interruptState = Cy_SysLib_EnterCriticalSection();

    PPCA_CPUSS_CNFG_MXCM331->CM33_NS_VECTOR_TABLE_BASE = vectorTableOffset;

    regValue = PPCA_CPUSS_CNFG_MXCM331->CM33_CMD & ~(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_ENABLED_Msk | PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_VECTKEYSTAT_Msk);
    regValue |= _VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_VECTKEYSTAT, CY_SYS_CORE_PWR_CTL_KEY_OPEN); /* set key bits */
    regValue |= _VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CMD_ENABLED, CY_SYS_CM33_IN_RESET);             /* reset enable bit */
    PPCA_CPUSS_CNFG_MXCM331->CM33_CMD = regValue;

    Cy_SysLib_ExitCriticalSection(interruptState);

    if (waitus == CY_SYS_CORE_WAIT_INFINITE)
    {
        while (Cy_SysGetCM33_1_Status() != CY_SYS_CORE_STATUS_ACTIVE)
        {
            /* Wait for the power mode to take effect */
        }
    }
    else
    {
        while ((Cy_SysGetCM33_1_Status() != CY_SYS_CORE_STATUS_ACTIVE) && (waitus))
        {
            Cy_SysLib_DelayUs(1u);
            waitus--;
        }

        if (waitus == 0u)
        {
            printf("\r\n  Failed to enable PPCA CM331\r\n");
        }
    }

    PPCA_CPUSS_CNFG_MXCM331->CM33_CTL &= ~(_VAL2FLD(PPCA_CPUSS_CNFG_MXCM33_CM33_CTL_CPU_WAIT, CY_SYS_CPU_WAIT_DEFAULT));

    /* Enable Event from other CM33 */
    PPCA_CPUSS_CNFG_MXCM331->CM33_EVENT_CTL = CM33_CPU_EVENT;
}

/******************************************************************************
 * Function Name: PPCA_ImageCopy
 ******************************************************************************
 *
 * Copies image from flash to PPCA RAM for Core 0 or Core1.
 *
 * \note Enables DMA and uses channel 0 for copying. Disables DMA after copying the image.
 *
 * \param core Core 0 or Core 1.
 *
 ******************************************************************************/
static void PPCA_ImageCopy(cy_en_ppca_core_t core_number)
{
    uint32_t timeout = PPCA_IMAGE_COPY_TIMEOUT_MS;

    /* Configure and start the DMA copy. The transfer itself runs in the
     * background (no critical section is held for the copy duration) and the
     * channel is triggered directly in software instead of through TrigMux. */
    Cy_DMA_Enable(DW0);
    (void)Cy_DMA_Descriptor_Init(&descriptor_0, &descriptor_config);
    Cy_DMA_Descriptor_SetYloopDataCount(&descriptor_0, (PPCA_IMAGE_SIZE / (XCOUNT_VALUE * 4)));

    if (core_number == CY_PPCA_CORE_1)
    {
        Cy_DMA_Descriptor_SetSrcAddress(&descriptor_0, (void *)(PPCA_CORE1_IMAGE_ADDRESS));
        Cy_DMA_Descriptor_SetDstAddress(&descriptor_0, (void *)(CY_SRAM_M2_BASE + PPCA_IMAGE_VECTOR_OFFSET));
    }
    else
    {
        Cy_DMA_Descriptor_SetSrcAddress(&descriptor_0, (void *)(PPCA_CORE0_IMAGE_ADDRESS));
        Cy_DMA_Descriptor_SetDstAddress(&descriptor_0, (void *)(CY_SRAM_M0_BASE + PPCA_IMAGE_VECTOR_OFFSET));
    }

    /* Init channel 0 on DMA0 HW Block*/
    (void)Cy_DMA_Channel_Init(DW0, DW_CHANNEL_0, &channelConfig);
    (void)Cy_DMA_Channel_Enable(DW0, DW_CHANNEL_0);

    /* Trigger the DMA channel directly via software */
    Cy_DMA_Channel_SetSWTrigger(DW0, DW_CHANNEL_0);

    while ((Cy_DMA_Channel_GetInterruptStatus(DW0, DW_CHANNEL_0) == 0UL) && (timeout))
    {
        Cy_SysLib_Delay(1u);
        timeout--;
    }
    if (timeout == 0)
    {
        printf("\r\n  Failed to load PPCA Image\r\n");
    }
    Cy_DMA_Disable(DW0);
}

/******************************************************************************
 * Function Name: PPCA_StartCore
 ******************************************************************************
 *
 * Starts PPCA Core 0 or Core 1 on a cold boot by enabling the core at its
 * image vector table offset.
 *
 * \param core Core 0 or Core 1.
 *
 ******************************************************************************/
void static PPCA_StartCore(cy_en_ppca_core_t core_number)
{
    if (core_number == CY_PPCA_CORE_0)
    {
        PPCA_EnableCM33_0(PPCA_IMAGE_VECTOR_OFFSET, PPCA_ENABLE_WAIT_TIME_US);
    }
    else
    {
        PPCA_EnableCM33_1(PPCA_IMAGE_VECTOR_OFFSET, PPCA_ENABLE_WAIT_TIME_US);
    }
}

/******************************************************************************
 * Function Name: PPCA_WaitCoreRunning
 ******************************************************************************
 *
 * Waits (with a timeout) until the specified PPCA core reports it is up and
 * running. The core run state is updated by the Main CPU IPC notify handlers
 * when a CORE_UP / SWITCH_ACK message is received.
 *
 * \param core       0 for Core 0, 1 for Core 1.
 * \param timeout_ms Timeout in milliseconds.
 *
 * \return true if the core is running, false on timeout.
 *
 ******************************************************************************/
static bool PPCA_WaitCoreRunning(uint32_t core, uint32_t timeout_ms)
{
    while ((main_get_ppca_state(core) != (uint32_t)PPCA_STATE_RUNNING) && (timeout_ms != 0u))
    {
        Cy_SysLib_Delay(1u);
        timeout_ms--;
    }

    return (main_get_ppca_state(core) == (uint32_t)PPCA_STATE_RUNNING);
}

/******************************************************************************
 * Function Name: PPCA_RequestSwitch
 ******************************************************************************
 *
 * Sends an IPC notify message requesting the specified PPCA core to switch to
 * the newly staged image (Main CPU -> PPCA core).
 *
 * \param core 0 for Core 0, 1 for Core 1.
 *
 ******************************************************************************/
static void PPCA_RequestSwitch(uint32_t core)
{
    IPC_STRUCT_Type *pIPC = (core == 0u) ? PPCA_IPC_SWITCH_REQ_CORE0 : PPCA_IPC_SWITCH_REQ_CORE1;
    uint32_t ch = (core == 0u) ? IPC_CH_MAIN_TO_CORE0 : IPC_CH_MAIN_TO_CORE1;

    if (CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_SendMsgWord(pIPC, IPC_NOTIFY_MASK(ch), (uint32_t)IPC_MSG_SWITCH_REQ))
    {
        printf("\r\n PPCA CORE%u switch request failed (channel busy)\r\n", (unsigned int)core);
    }
    main_set_ppca_state(core, (uint32_t)PPCA_STATE_UPDATE_REQUESTED);
}

/******************************************************************************
 * Function Name: PPCA_Init
 ******************************************************************************
 *
 * Initializes and starts both PPCA Core0 and Core1
 *
 ******************************************************************************/
void PPCA_Init(void)
{
    if (!is_live_update())
    {
        /* Cold boot: enable PPCA RAM, load both images, start the cores and
         * wait for each to report it is up and running over IPC. */
        Cy_System_PPCA_RAM_Enable();

        printf("\r\n Loading PPCA Images to PPCA CODE SRAM: 0x%04X - 0x%04X\r\n",
               PPCA_IMAGE_VECTOR_OFFSET, PPCA_IMAGE_VECTOR_OFFSET + PPCA_IMAGE_SIZE - 1);
        PPCA_ImageCopy(CY_PPCA_CORE_0);
        PPCA_ImageCopy(CY_PPCA_CORE_1);

        printf("\r\n Starting PPCA CORE 0\r\n");
        PPCA_StartCore(CY_PPCA_CORE_0);
        printf("\r\n Starting PPCA CORE 1\r\n");
        PPCA_StartCore(CY_PPCA_CORE_1);

        if (!PPCA_WaitCoreRunning(0u, PPCA_CORE_UP_TIMEOUT_MS))
        {
            printf("\r\n PPCA CORE0 did not report running\r\n");
        }
        if (!PPCA_WaitCoreRunning(1u, PPCA_CORE_UP_TIMEOUT_MS))
        {
            printf("\r\n PPCA CORE1 did not report running\r\n");
        }
    }
    else
    {
        /* Live update: load the new images into the shadow slots, then request
         * each running core to switch. Wait for each core to restart on the new
         * image and report it is running again over IPC. */
        printf("\r\n Loading New PPCA Images to PPCA CODE SRAM: 0x%04X - 0x%04X\r\n",
               PPCA_IMAGE_VECTOR_OFFSET, PPCA_IMAGE_VECTOR_OFFSET + PPCA_IMAGE_SIZE - 1);

        PPCA_ImageCopy(CY_PPCA_CORE_0);
        PPCA_ImageCopy(CY_PPCA_CORE_1);

        printf("\r\n Requesting PPCA CORE 0 to switch to new Firmware\r\n");
        PPCA_RequestSwitch(0u);
        printf("\r\n Requesting PPCA CORE 1 to switch to new Firmware\r\n");
        PPCA_RequestSwitch(1u);

        if (!PPCA_WaitCoreRunning(0u, PPCA_CORE_UP_TIMEOUT_MS))
        {
            printf("\r\n PPCA CORE0 did not switch to the new image\r\n");
        }
        if (!PPCA_WaitCoreRunning(1u, PPCA_CORE_UP_TIMEOUT_MS))
        {
            printf("\r\n PPCA CORE1 did not switch to the new image\r\n");
        }
    }
}
