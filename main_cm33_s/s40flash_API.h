/*******************************************************************************
// $File: //depot/icm/proj/s40flash/icmrel/3.0-rel/fw/s40flash/source/src/s40flash_API/s40flash_API.h $
// $DateTime: 2024/11/18 18:11:52 $
// $Revision: #8 $
// $Author: icmAdmin $
// Author's Email : bsn@cypress.com
// Description    : s40flash API.
//
*******************************************************************************
* (c) 2024-2026, Infineon Technologies AG, or an affiliate of Infineon
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

#ifndef S40FLASH_API_H
#define S40FLASH_API_H

#ifdef S40FLASHTC2
#include "s40flashtc_regs.h"
#include "cm0ikmcu.h"
#include "core_cm0.h"
#define SFLASH_ROW_NUMBER 1
#else
/*  PSOC6 integration */
//#include "mxs40_regs_defines.h"

#endif

// uncomment in next Si (ROM change)
#define ROM_A1_FIX          1U  //JIRA CDT_004885-111


#define SFLASH_BANK_NUMBER          1U   // JIRA 17: In BOY2, SM rows moved from sector 0 to 1
#define SFLASH_ROW_HV_TRIMS_NUMBER  0U
#define SFLASH_ROW_HV_PARAMS_NUMBER 2U

// Margin Mode Error Codes
#define ERR_NO_VTLO         1U  /* VtLo not reached even in vpos domain */ /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */
#define ERR_NO_VTE          2U  /* Vte not reached                      */ /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */
#define ERR_NO_VTP          4U  /* Vtp not reached                      */ /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */
#define ERR_NO_VTHI         3U  /* VtHi not reached even in vneg domain */ /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */

#define MAX_MM_ROWS         1024U
#define MM_ALL_ONES_FLAG    4U
#define MM_ALL_ZERO_FLAG    2U
#define STATUS_MM_ERROR     0xF0000030U

//=========================================
//  SFLASH row #
//=========================================

//=========================================
//  S40FLASH P2 TRIM BITS
//=========================================

//#define TRIM_SHORT_WAIT_TIME.

#ifdef TRIM_SHORT_WAIT_TIME
#define NITIM_TRIM_VALS 4U  // 8
#define NIDAC_TRIM_VALS 2U  // 4
#define NVBG_TRIM_VALS 4U   // 8
#define NIPREF_TRIM_VALS 4U // 8
#define NICREF_TRIM_VALS 4U // 8
#define CNT_15US_LOOP 2U
#else
#define NITIM_TRIM_VALS     64U
#define NIDAC_TRIM_VALS     256U
#define NVBG_TRIM_VALS      64U
#define NIPREF_TRIM_VALS    32U
#define NICREF_TRIM_VALS    64U
#define CNT_15US_LOOP       15U // metered on 8 MHz
#endif

//Updated based on CXZ-483**
#define ITIM_MIN_VAL        26U
#define ITIM_MID_VAL        40U
#define ITIM_MAX_VAL        48U
#define ITIM_STEP_VAL       2U
#define IDAC_MIN_VAL        20U
#define IDAC_MID_VAL        40U
#define IDAC_MAX_VAL        54U
#define IDAC_STEP_VAL       4U
#define VBG_MIN_VAL         28U
#define VBG_MID_VAL         45U
#define VBG_MAX_VAL         53U
#define VBG_STEP_VAL        2U
#define IPREF_MIN_VAL       11U
#define IPREF_MID_VAL       15U
#define IPREF_MAX_VAL       19U
#define IPREF_STEP_VAL      2U
#define ICREF_MIN_VAL       24U
#define ICREF_MID_VAL       39U
#define ICREF_MAX_VAL       49U
#define ICREF_STEP_VAL      1U

//=========================================
//  FM_ ADDRESSES
//=========================================
#ifdef S40FLASHTC2
#define MMIO0_BASE_ADDR         0x40200000
#define FLASHC_BASE_ADDR        MMIO0_BASE_ADDR+0x00060000     // 0x40260000
#define FM_REGION_BASE_ADDR     ((uint32)FLASHC_BASE_ADDR + 0x00001000U)    // 0x40261000
#define RBUS_BASE_ADDR          MMIO1_BASE_ADDR
#define RBUS_AXA_OFFSET         0x00000000U
#define RBUS_BXA_OFFSET         0x00000000U
#define RBUS_COL33_BASE_ADDR    MMIO1_BASE_ADDR
#define RBUS_COL33_AXA_OFFSET   0x00500000U
#define RBUS_COL33_BXA_OFFSET   0x00000000U
#define RBUS_COL33_OFFSET       0x00400000U
#else
/* PSOC6: FM_CTL_ADDR comes from mxs40_regs_defines.h */
#define FLASHC_BASE_ADDR        FLASHC_BASE
#define FM_REGION_BASE_ADDR     ((uint8_t *)(FLASHC_FM_CTL)) // JIRA CDT_004885-104
#define MMIO1_BASE_ADDR         FLASH_BASE_ADDR
#define RBUS_TSRAM_CODE_ADDR    MMIO1_BASE_ADDR + (91 * 4)
#define RBUS_BASE_ADDR          0x02000000U
#define RBUS_AXA_OFFSET         0x013A0000U
#define RBUS_BXA_OFFSET         0x01800000U
#define RBUS_COL33_BASE_ADDR    0x03A00000U
#define RBUS_COL33_AXA_OFFSET   0x00100000U
#define RBUS_COL33_BXA_OFFSET   0x00180000U
#endif

//=========================================
//  FLASHC MMIO
//=========================================
#define FLASHC_CTL_ADDR         FLASHC_BASE_ADDR + 0x000
#define REG_FLASHC_CTL          REG32(FLASHC_CTL_ADDR)

#ifndef S40FLASHTC2
#define CPUSS_BASE_ADDR             0x42100000U
#define CPUSS_MXI_CACHE_1_2_ADDR    (CPUSS_BASE_ADDR + 0x3008U)
#define REG_MXI_CACHE_1_2           REG32(CPUSS_MXI_CACHE_1_2_ADDR)
#define CPUSS_CACHE_INV_MASK        0x00000001U
#endif

#define FC_FLASH_LOCK_ADDR          (FLASHC_BASE_ADDR + 0x204U)
#define REG_FC_FLASH_LOCK           REG32(FC_FLASH_LOCK_ADDR)
#define FC_FLASH_LOCK_LOCKED_MASK   0x80000000U
#define FC_FLASH_LOCK_PC_MASK       0x0000000FU

#define FC_ENFORCE_PC_LOCK_MASK     0x01000000U

//=========================================
//  FM C_BUS ADDRESS OFFSET
//=========================================
#define FM_CTL_ADDR                 (FM_REGION_BASE_ADDR + 0xa10U)   // Flash macro control - in the xls it is named FLASH_MACRO_CTL - added here by BIG because I am not sure S40FLASHTC2 ever gets defined
#define FM_STATUS_ADDR              (FM_REGION_BASE_ADDR + 0xa14U)   // Status
#define FM_MEM_ADDR_ADDR            (FM_REGION_BASE_ADDR + 0xa18U)   // Flash macro  address
#define FM_BOOKMARK_ADDR            (FM_REGION_BASE_ADDR + 0xa1cU)   // Bookmark register - keeps the current FW HV seq
#define FM_GEOMETRY_ADDR            (FM_REGION_BASE_ADDR + 0xa20U)   // Regular flash geometry
#define FM_GEOMETRY_SUP_ADDR        (FM_REGION_BASE_ADDR + 0xa24U)   // Supervisory flash geometry
#define FM_ANA_CTL0_ADDR            (FM_REGION_BASE_ADDR + 0xa28U)   // Analog control 0
#define FM_ANA_CTL1_ADDR            (FM_REGION_BASE_ADDR + 0xa2cU)   // Analog control 1
#define FM_TEST_CTL_ADDR            (FM_REGION_BASE_ADDR + 0x000U)   // Test mode control
#define FM_WAIT_CTL_ADDR            (FM_REGION_BASE_ADDR + 0xa48U)   // Wait State control
#define FM_TIMER_CLK_CTL_ADDR       (FM_REGION_BASE_ADDR + 0xa50U)   // High voltage control
#define FM_TIMER_CTL_ADDR           (FM_REGION_BASE_ADDR + 0xa54U)   // Timer control
#define FM_ACLK_CTL_ADDR            (FM_REGION_BASE_ADDR + 0xa58U)   // Aclk control
#define FM_INTR_ADDR                (FM_REGION_BASE_ADDR + 0xa5cU)   // Interrupt    
#define FM_INTR_SET_ADDR            (FM_REGION_BASE_ADDR + 0xa60U)   // Interrupt set
#define FM_INTR_MASK_ADDR           (FM_REGION_BASE_ADDR + 0xa64U)   // Interrupt mask
#define FM_INTR_MASKED_ADDR         (FM_REGION_BASE_ADDR + 0xa68U)   // Interrupt masked
#define FM_CAL_CTL0_ADDR            (FM_REGION_BASE_ADDR + 0xa6cU)   // Cal control 0: VCT, VBG, CDAC, IPREFA trim bits
#define FM_CAL_CTL1_ADDR            (FM_REGION_BASE_ADDR + 0xa70U)   // Cal control 1: ICREF IPREF trim bits
#define FM_CAL_CTL2_ADDR            (FM_REGION_BASE_ADDR + 0xa74U)   // Cal control 2: IDAC, VREF_SEL
#define FM_CAL_CTL3_ADDR            (FM_REGION_BASE_ADDR + 0xa78U)   // Cal control 3: osc trim bits, fdiv, vddhi, LP-ULP controls
#define FM_CAL_CTL4_ADDR            (FM_REGION_BASE_ADDR + 0xa7cU)   // Cal control 4: vlim, sdac, itim (ULP), UGB
#define FM_CAL_CTL5_ADDR            (FM_REGION_BASE_ADDR + 0xa80U)   // Cal control 5: vlim, sdac, itim (LP), AMUX_SEL
#define FM_CAL_CTL6_ADDR            (FM_REGION_BASE_ADDR + 0xa84U)   // Cal control 6: SA trims T1, T4, T5, T6
#define FM_CAL_CTL7_ADDR            (FM_REGION_BASE_ADDR + 0xa88U)   // Cal control 7: ERSx8, fm_active, turbo_ext_hv, npdac_hwctl_dis_hv, PTRIM
#define FM_RED_CTL01_ADDR           (FM_REGION_BASE_ADDR + 0x040U)   // Redundancy Control normal sectors 0,1
#define FM_RED_CTL23_ADDR           (FM_REGION_BASE_ADDR + 0x044U)   // Redundancy Controll normal sectors 2,3
#define FM_RGRANT_DELAY_PRG_ADDR    (FM_REGION_BASE_ADDR + 0xa4cU)   // Rgrant blocking during Seq and PE changes for PRG
#define FM_PW_SEQ12_ADDR            (FM_REGION_BASE_ADDR + 0xa44U)   // Seq1 and Seq2 Pre PW
#define FM_PW_SEQ23_ADDR            (FM_REGION_BASE_ADDR + 0xa40U)   // Seq2 Post and Seq 3 PW
#define FM_RGRANT_SCALE_ERS_ADDR    (FM_REGION_BASE_ADDR + 0xa3cU)   // Rgrant block SCALE during Seq and PE changes for ERS
#define FM_RGRANT_DELAY_ERS_ADDR    (FM_REGION_BASE_ADDR + 0xa38U)   // Rgrant blocking during Seq and PE changes for ERS
#define FM_REFRESH_ADDR_ADDR        (FM_REGION_BASE_ADDR + 0xa34U)   // Address bit to point to scratch area and Col 33
#define FM_PL_WRALL_DATA_ADDR       (FM_REGION_BASE_ADDR + 0xa30U)   // Flash macro high Voltage page latches data (for all page latches)
#define FM_PL_DATA_ADDR             (FM_REGION_BASE_ADDR + 0x800U)   // Flash macro high Voltage page latches data
#define FM_PL_ECC_ADDR              (FM_REGION_BASE_ADDR + 0xb00U)   // Flash Macro Page Latches ECC
#define FM_MEM_DATA_ADDR            (FM_REGION_BASE_ADDR + 0xc00U)   // Flash macro memory sense amplifier and column decoder data
#define FM_MEM_ECC_ADDR             (FM_REGION_BASE_ADDR + 0xf00U)   // Flash Macro Memory Sense Amplifier ECC bits for cbus read

//=========================================
//  FM_ REGISTERS
//=========================================
#define REG_FM_CTL                  REG32(FM_CTL_ADDR)              // Flash macro control
#define REG_FM_STATUS               REG32(FM_STATUS_ADDR)           // Status
#define REG_FM_MEM_ADDR             REG32(FM_MEM_ADDR_ADDR)         // Flash macro  address
#define REG_FM_BOOKMARK             REG32(FM_BOOKMARK_ADDR)         // Bookmark register - keeps the current FW HV seq
#define REG_FM_GEOMETRY             REG32(FM_GEOMETRY_ADDR)         // Regular flash geometry
#define REG_FM_GEOMETRY_SUP         REG32(FM_GEOMETRY_SUP_ADDR)     // Supervisory flash geometry
#define REG_FM_ANA_CTL0             REG32(FM_ANA_CTL0_ADDR)         // Analog control 0
#define REG_FM_ANA_CTL1             REG32(FM_ANA_CTL1_ADDR)         // Analog control 1
#define REG_FM_TEST_CTL             REG32(FM_TEST_CTL_ADDR)         // Test mode control
#define REG_FM_WAIT_CTL             REG32(FM_WAIT_CTL_ADDR)         // Wiat State control
#define REG_FM_TIMER_CLK_CTL        REG32(FM_TIMER_CLK_CTL_ADDR)    // Timer CLK control
#define REG_FM_TIMER_CTL            REG32(FM_TIMER_CTL_ADDR)        // Timer control
#define REG_FM_ACLK_CTL             REG32(FM_ACLK_CTL_ADDR)         // Aclk control
#define REG_FM_INTR                 REG32(FM_INTR_ADDR)             // Interrupt
#define REG_FM_INTR_SET             REG32(FM_INTR_SET_ADDR)         // Interrupt set
#define REG_FM_INTR_MASK            REG32(FM_INTR_MASK_ADDR)        // Interrupt mask
#define REG_FM_INTR_MASKED          REG32(FM_INTR_MASKED_ADDR)      // Interrupt masked
#define REG_FM_CAL_CTL0             REG32(FM_CAL_CTL0_ADDR)         // Cal control 0: VCT, CDAC, VBG, VBG_TC, IPREF trim bits
#define REG_FM_CAL_CTL1             REG32(FM_CAL_CTL1_ADDR)         // Cal control 1: ICREF, ICREF_TC, IPREF, IPREF_TC trim bits
#define REG_FM_CAL_CTL2             REG32(FM_CAL_CTL2_ADDR)         // Cal control 2: IDAC ULP, VREF_SEL ULP, IDAC LP, VREF_SEL LP,  
#define REG_FM_CAL_CTL3             REG32(FM_CAL_CTL3_ADDR)         // Cal control 3: osc trim bits, pm_en, reg_act, turbo
#define REG_FM_CAL_CTL4             REG32(FM_CAL_CTL4_ADDR)         // Cal control 4: vlim trim bits, sdac, itim, T8, vbst_dis ULP, ugb_en.
#define REG_FM_CAL_CTL5             REG32(FM_CAL_CTL5_ADDR)         // Cal control 5: vlim trim bits, sdac, itim, T8, vbst_dis LP, amux_sel.
#define REG_FM_CAL_CTL6             REG32(FM_CAL_CTL6_ADDR)         // Cal control 6: SA T1,T4, T5, T6 ULP, SA T1,T4, T5, T6 LP.
#define REG_FM_CAL_CTL7             REG32(FM_CAL_CTL7_ADDR)         // Cal control 7: ersx8, PTRIM_ULP, PTRIM LP.
#define REG_FM_RED_CTL01            REG32(FM_RED_CTL01_ADDR)        // Redundancy Control normal sectors 0,1
#define REG_FM_RED_CTL23            REG32(FM_RED_CTL23_ADDR)        // Redundancy Controll normal sectors 2,3
#define REG_FM_RGRANT_DELAY_PRG     REG32(FM_RGRANT_DELAY_PRG_ADDR) // Redundancy Controll special sectors 0,1
#define REG_FM_HVPULSE_SEQ12        REG32(FM_PW_SEQ12_ADDR)         // Redundancy Controll special sectors 0,1
#define REG_FM_HVPULSE_SEQ23        REG32(FM_PW_SEQ23_ADDR)         // Redundancy Controll special sectors 0,1
#define REG_FM_RGRANT_SCALE_ERS     REG32(FM_RGRANT_SCALE_ERS_ADDR) // Redundancy Controll special sectors 0,1
#define REG_FM_RGRANT_DELAY_ERS     REG32(FM_RGRANT_DELAY_ERS_ADDR) // Redundancy Controll special sectors 0,1
#define REG_FM_REFRESH              REG32(FM_REFRESH_ADDR_ADDR)     // Address bit to point to scratch area
#define REG_FM_PL_WRALL_DATA        REG32(FM_PL_WRALL_DATA_ADDR)    // Flash macro high Voltage page latches data (for all page latches)
#define REG_FM_PL_DATA              REG32(FM_PL_DATA_ADDR)          // Flash macro high Voltage page latches data
#define REG_FM_PL_ECC               REG32(FM_PL_ECC_ADDR)           // Flash Macro Page Latches ECC
#define REG_FM_MEM_DATA             REG32(FM_MEM_DATA_ADDR)         // Flash macro memory sense amplifier and column decoder data
#define REG_FM_MEM_ECC              REG32(FM_MEM_ECC_ADDR)          // Flash Macro Memory Sense Amplifier ECC bits for cbus read

//========================================================================================
// FM SEQUENCE
//========================================================================================
#define FM_SEQ0                     0U
#define FM_SEQ1                     1U
#define FM_SEQ2                     2U
#define FM_SEQ3                     3U

//=========================================
//  FM MODES
//=========================================
#define FM_MODE_READ                0U
#define FM_MODE_RESET_HV            3U

// ERASE MODES
#define FM_MODE_ERASE_PAGE          7U
#define FM_MODE_ERASE_SUBSECT       8U
#define FM_MODE_ERASE_SECTOR        9U
#define FM_MODE_ERASE_BULK          10U

// PROGRAM MODES
#define FM_MODE_PROGRAM_SUBSECT     6U // to be used only as preprogram
#define FM_MODE_PROGRAM_PAGE        11U
#define FM_MODE_PROGRAM_SECTOR      12U // SECTOR PROGRAM EVEN/ODD

#define FM_MODE_PROGRAM_SECTOR_ALL  13U // SECTOR PROGRAM ALL

#define FM_MODE_PROGRAM_BULK        14U     // BULK PROGRAM EVEN/ODD
#define FM_MODE_PROGRAM_BULK_ALL    15U // BULK PROGRAM ALL

#define FM_PROGRAM_GEN_PRE_PROG     0U
#define FM_PROGRAM_GEN_PROGRAM      1U

//=========================================
//  FM #defines
//=========================================
#define CBUS_ADDR_STEP              4U       //  address step within C-bus address space
#define FM_ROW_SIZE_32_COLS         128U     // 4096/32 Number of x32 bits words in a FM row without col 33
#define FM_ROW_SIZE                 132U     // 4096/32 Number of x32 bits words in a FM row plus col 33
#define FM_ROW_SIZE128              32U      // 4096/128 Number of x128 bits words in a FM row
#define FM_MAX_COLUMNS              33U      // 4096/128 Number of x128 bits words in a FM row plus col 33
#define MAX_MM_ROW_COUNTER          1024U    // for margin mode the maximum number of rows per run
#define FM_X32_WORDS_IN_DOUT        4U       // Number of the page latch words in a column dout
#define FM_COLUMN_ADDR_STEP         (FM_X32_WORDS_IN_DOUT * CBUS_ADDR_STEP)

#define FM_SUBSECTOR_ROWS           8U                           // Number of Rows in a subsector
#define FM_HALF_SUBSECTOR_ROWS      (FM_SUBSECTOR_ROWS / 2U)     // Number of Rows in a subsector
#define MAX_CA_SIZE                 5U

#define RAM_BUFFER_SIZE             (8U * FM_ROW_SIZE)
#define RAM_BUFFER_SIZE_HALF        (RAM_BUFFER_SIZE >> 1U)
#define RAM_BUFFER_WITH_ECC_SIZE    (RAM_BUFFER_SIZE + (8U * FM_MAX_COLUMNS))

#define ALL_ONES                    0xFFFFFFFFU
#define ALL_ZERO                    0x00000000U
#define ALL_AA                      0xAAAAAAAAU
#define ALL_55                      0x55555555U
#define ALL_A5                      0xA5A5A5A5U
#define ALL_5A                      0x5A5A5A5AU

#define FM_RESET_MM                 (0x00000003U | FM_IF_SEL_MASK)
#define FM_TIMER_SCALE_MS           (1U << 15U)
#define WORD_SIZE                   32U

// JIRA CDT_004885-16
#define ECC_BIT_NUM                 9U          /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */
#define ECC_BIT_MASK                0x000001FFU /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */

#define VT_POS_MM_VAL               0x80U
#define VT_NEG_MM_VAL               0U

//========================================================================================
// ADDR INDEX IN ADDRESS PARAMETERS
//========================================================================================
#define AXA_IDX                     0U
#define BA_IDX                      1U
#define RA_IDX                      2U
#define WORD_ADDR_IDX               3U
#define RAM_ADDR_IDX                4U

#define SM_ROW1_ADDR                0x01000001U
#define EN_CBUS                     1U              /* PRQA S 4640 # This macro is distinctive from the ones defined in errno.h */
#define DIS_CBUS                    0U

#define VTP_POS_RES                 24U      // 8 bit result position for Vtp
#define VTHI_POS_RES                16U      // 8 bit result position for VtHi
#define VTE_POS_RES                 8U       // 8 bit result position for Vte
#define VTLO_POS_RES                0U       // 8 bit result position for VtLo

//=========================================
// FM PARAM INDEXES with respect to aFmFuncAddr
//=========================================
#define FMPARAMETERS_IDX            0U
#define FUNCPARAM_IDX               5U
#define FUNCRESULTS_IDX             37U
#define HVPARAMTABLE_IDX            69U

//=========================================
// MDAC PARAMS
//=========================================
#define FM_MDAC_BITS                7U
#define FM_MDAC_HALF                ((uint32)1U << (FM_MDAC_BITS - 1U))

//=========================================
// REG SIZES
//=========================================
#define FM_MDAC_SIZE                ((6U - 0U) + 1U)
#define FM_PDAC_SIZE                ((15U - 8U) + 1U)
#define FM_NDAC_SIZE                ((7U - 0U) + 1U)
#define FM_CDAC_SIZE                ((7U - 5U) + 1U)
#define FM_ITIM_SIZE                ((12U - 7U) + 1U)

#define FM_DAA_MUX_SIZE             ((23U - 16U) + 1U)

#define FM_MAX_BA_SIZE              ((23U - 16U) + 1U)
#define FM_MAX_RA_SIZE              ((15U - 0U) + 1U)
#define FM_MODE_SIZE                ((3U - 0U) + 1U)
#define FM_SEQ_SIZE                 ((9U - 8U) + 1U)
#define FM_SDAC_SIZE                ((6U - 5U) + 1U)
#define FM_WAIT_MEM_RD_SIZE         ((3U - 0U) + 1U)
#define FM_WAIT_HVPL_RD_SIZE        ((11U - 8U) + 1U)
#define FM_WAIT_HVPL_WR_SIZE        ((18U - 16U) + 1U)

#define FM_TM_SIZE                  ((4U - 0U) + 1U)
#define FM_RWW_MODE_SIZE            ((25U - 24U) + 1U)

//------------- trim bits sizes -------------

#define VCT_TRIM_LO_HV_SIZE         ((4U - 0U) + 1U)
#define CDAC_HV_SIZE                ((7U - 5U) + 1U)
#define VBG_TRIM_HV_SIZE            ((13U - 8U) + 1U)
#define VBG_TC_TRIM_HV_SIZE         ((17U - 14U) + 1U)
#define IPREF_TRIM_HV_SIZE          ((14U - 10U) + 1U)
#define IDAC_TRIM_HV_SIZE           ((17U - 10U) + 1U)

#define ITIM_TRIM_HV_SIZE           ((12U - 7U) + 1U)
#define ICREF_TRIM_HV_SIZE          ((5U - 0U) + 1U)
#define ICREF_TC_TRIM_HV_SIZE       ((9U - 6U) + 1U)
#define VREF_SEL_HV_SIZE            1U
#define FM_ACTIVE_HV_SIZE           1U
#define TURBO_EXT_HV_SIZE           1U

#define OSC_TRIM_HV_SIZE            ((3U - 0U) + 1U)
#define OSC_RANGE_TRIM_HV_SIZE      1U
#define SDAC_HV_SIZE                ((6U - 5U) + 1U)
#define ITIM_HV_SIZE                ((12U - 7U) + 1U)
#define VDDHI_HV_SIZE               1U
#define TURBO_PULSEW_HV_SIZE        ((14U - 13U) + 1U)

//------------- trim bits masks -------------

#define VCT_TRIM_LO_HV_MASK         (((uint32)1U << VCT_TRIM_LO_HV_SIZE) - 1U)
#define CDAC_HV_MASK                (((uint32)1U << CDAC_HV_SIZE) - 1U)
#define VBG_TRIM_HV_MASK            (((uint32)1U << VBG_TRIM_HV_SIZE) - 1U)
#define VBG_TC_TRIM_HV_MASK         (((uint32)1U << VBG_TC_TRIM_HV_SIZE) - 1U)
#define IPREF_TRIM_HV_MASK          (((uint32)1U << IPREF_TRIM_HV_SIZE) - 1U)
#define IDAC_TRIM_HV_MASK           (((uint32)1U << IDAC_TRIM_HV_SIZE) - 1U)

#define ITIM_TRIM_HV_MASK           (((uint32)1U << ITIM_TRIM_HV_SIZE) - 1U)
#define ICREF_TRIM_HV_MASK          (((uint32)1U << ICREF_TRIM_HV_SIZE) - 1U)
#define ICREF_TC_TRIM_HV_MASK       (((uint32)1U << ICREF_TC_TRIM_HV_SIZE) - 1U)
#define VREF_SEL_HV_MASK            (((uint32)1U << VREF_SEL_HV_SIZE) - 1U)
#define FM_ACTIVE_HV_MASK           (((uint32)1U << FM_ACTIVE_HV_SIZE) - 1U)
#define TURBO_EXT_HV_MASK           (((uint32)1U << TURBO_EXT_HV_SIZE) - 1U)

#define OSC_TRIM_HV_MASK            (((uint32)1U << OSC_TRIM_HV_SIZE) - 1U)
#define OSC_RANGE_TRIM_HV_MASK      (((uint32)1U << OSC_RANGE_TRIM_HV_SIZE) - 1U)
#define SDAC_HV_MASK                (((uint32)1U << SDAC_HV_SIZE) - 1U)
#define ITIM_HV_MASK                (((uint32)1U << ITIM_HV_SIZE) - 1U)
#define VDDHI_HV_MASK               (((uint32)1U << VDDHI_HV_SIZE) - 1U)
#define TURBO_PULSEW_HV_MASK        (((uint32)1U << TURBO_PULSEW_HV_SIZE) - 1U)

//------------- trim bits WR masks -------------

#define VCT_TRIM_LO_HV_WRMASK       ( ~((uint32)VCT_TRIM_LO_HV_MASK) )
#define CDAC_HV_WRMASK              ( ~((uint32)CDAC_HV_MASK << 5U) )
#define VBG_TRIM_HV_WRMASK          ( ~((uint32)VBG_TRIM_HV_MASK << 8U) )
#define VBG_TC_TRIM_HV_WRMASK       ( ~((uint32)VBG_TC_TRIM_HV_MASK << 14U) )
#define IPREF_TRIM_HV_WRMASK        ( ~((uint32)IPREF_TRIM_HV_MASK << 10U) )
#define IDAC_TRIM_HV_WRMASK         ( ~((uint32)IDAC_TRIM_HV_MASK << 10U) )

#define ITIM_TRIM_HV_WRMASK         ( ~((uint32)ITIM_TRIM_HV_MASK << 7U) )
#define ICREF_TRIM_HV_WRMASK        ( ~((uint32)ICREF_TRIM_HV_MASK) )
#define ICREF_TC_TRIM_HV_WRMASK     ( ~((uint32)ICREF_TC_TRIM_HV_MASK << 6U) )
#define VREF_SEL_ULP_WRMASK         ( ~((uint32)VREF_SEL_HV_MASK << 9U) )
#define VREF_SEL_LP_WRMASK          ( ~((uint32)VREF_SEL_HV_MASK << 19U) )
#define FM_ACTIVE_HV_WRMASK         ( ~((uint32)FM_ACTIVE_HV_MASK << 2U) )
#define TURBO_EXT_HV_WRMASK         ( ~((uint32)TURBO_EXT_HV_MASK << 3U) )

#define OSC_TRIM_HV_WRMASK          ( ~((uint32)OSC_TRIM_HV_MASK) )
#define OSC_RANGE_TRIM_HV_WRMASK    ( ~((uint32)OSC_RANGE_TRIM_HV_MASK << 4U) )
#define IDAC_HV_WRMASK              ( ~((uint32)IDAC_HV_MASK << 10U) )
#define SDAC_HV_WRMASK              ( ~((uint32)SDAC_HV_MASK << 5U) )
#define ITIM_HV_WRMASK              ( ~((uint32)ITIM_HV_MASK << 7U) )
#define VDDHI_HV_WRMASK             ( ~((uint32)VDDHI_HV_MASK << 12U) )
#define TURBO_PULSEW_HV_WRMASK      ( ~((uint32)TURBO_PULSEW_HV_MASK << 13U) )

//=========================================
// MASKS
//=========================================
#define FM_AXA_CODE                 0x80000001U
#define BEGIN_SUBSECTOR_MASK        0xFFFFFFF8U  // 8 rows in subsector
#define BIT0_MASK                   0x00000001U
#define BIT31_MASK                  0x80000000U
#define BIT30_MASK                  0x40000000U
#define BIT21_MASK                  0x00200000U
#define BIT18_MASK                  0x00040000U
#define BIT16_MASK                  0x00010000U
#define BIT10_MASK                  0x00000400U  // JIRA CDT_004885-54
#define BIT7_MASK                   0x00000080U

#define BIT30_WR_MASK               ( ~BIT30_MASK )
#define BIT21_WR_MASK               ( ~BIT21_MASK )

#define BYTE0_MASK                  0x000000FFU
#define BYTE1_MASK                  0x0000FF00U
#define BYTE2_MASK                  0x00FF0000U
#define BYTE3_MASK                  0xFF000000U
#define BYTE0_MASK_B                ( ~(BYTE0_MASK) )

#define TIMER_PERIOD_MASK           0x00007FFFU
#define TIMER_SCALE_MASK            0x00008000U
#define TIMER_SCALE_PER_MASK        (TIMER_SCALE_MASK | TIMER_PERIOD_MASK)

#define DW0_MASK                    0x0000FFFFU
#define DW1_MASK                    0xFFFF0000U

#define READY_RESTART_WR_MASK       ((uint32)1U << 16U)
#define FM_RBUSMODE_MASK            ((uint32)3U << 24U)

#define FM_CA_MASK                  (((uint32)1U << MAX_CA_SIZE) - 1U)

#define FM_MODE_MASK                (((uint32)1U << FM_MODE_SIZE) - 1U)
#define FM_SEQ_MASK                 (((uint32)1U << FM_SEQ_SIZE) - 1U)

#define FM_DAA_MASK                 (((uint32)1U << FM_DAA_MUX_SIZE) - 1U)
#define FM_BA_MASK                  (((uint32)1U << FM_MAX_BA_SIZE) - 1U)
#define FM_RA_MASK                  (((uint32)1U << FM_MAX_RA_SIZE) - 1U)
#define FM_BXA_MASK                 1U
#define FM_CXA_MASK                 1U
#define REFRESH_ROWS_COUNT          4U

#define FM_MDAC_MASK                (((uint32)1U << FM_MDAC_SIZE) - 1U)
#define FM_PDAC_MASK                (((uint32)1U << FM_PDAC_SIZE) - 1U)
#define FM_NDAC_MASK                (((uint32)1U << FM_NDAC_SIZE) - 1U)

#define FM_CDAC_MASK                (((uint32)1U << FM_CDAC_SIZE) - 1U)
#define FM_ITIM_MASK                (((uint32)1U << FM_ITIM_SIZE) - 1U)
#define FM_VCCSEL_MASK              1U
#define FM_VREFSEL_MASK             1U
#define FM_IREFSEL_MASK             1U
#define FM_FLIPAMUX_MASK            1U

#define FM_SDAC_MASK                (((uint32)1U << FM_SDAC_SIZE) - 1U)

#define FM_TM_MASK                  (((uint32)1U << FM_TM_SIZE) - 1U)

#define FM_WAIT_MEM_RD_MASK         (((uint32)1U << FM_WAIT_MEM_RD_SIZE) - 1U)
#define FM_WAIT_HVPL_RD_MASK        (((uint32)1U << FM_WAIT_HVPL_RD_SIZE) - 1U)
#define FM_WAIT_HVPL_WR_MASK        (((uint32)1U << FM_WAIT_HVPL_WR_SIZE) - 1U)
#define FM_RGRANTCTL_MASK           ((uint32)1U << 20U)
#define FM_RGRANTCTL_WRMASK         ( ~(FM_RGRANTCTL_MASK) )
#define FM_MBA_MASK                 ((uint32)1U << 23U)

#define FM_RWW_MODE_MASK            (((uint32)1U << FM_RWW_MODE_SIZE) - 1U)

#define FM_AUTO_HVPULSE_HV_MASK     BIT18_MASK

#define FM_ADDR_AXA_FORMAT          BIT31_MASK

#define FM_OP_SUCCESS_VAL           1U
#define FM_OP_FAILURE_VAL           0U
#define MAJORITY_DATA_STATUS_BIT    BIT31_MASK

//--------------------------------------
// WRITE MASKS
//--------------------------
#define FM_IF_SEL_MASK              ((uint32)1U << 24U)
#define FM_IF_SEL_MASK_WR           (~((uint32)FM_IF_SEL_MASK))
#define FM_WR_EN_MASK               ((uint32)1U << 25U)
#define FM_WR_EN_MASK_WR            (~((uint32)FM_WR_EN_MASK))
#define FM_DAA_WRMASK               (~((uint32)FM_DAA_MASK << 16U))

#define FM_MODE_WRMASK              (~((uint32)FM_MODE_MASK))
#define FM_SEQ_WRMASK               (~((uint32)FM_SEQ_MASK << 8U))

#define FM_AXA_MASK                 ((uint32)1U << 24U)
#define FM_AXA_WRMASK               (~((uint32)FM_AXA_MASK))
#define FM_BA_WRMASK                (~((uint32)FM_BA_MASK << 16U))
#define FM_RA_WRMASK                (~((uint32)FM_RA_MASK))
#define FM_ADDR_WRMASK              (~((uint32)FM_AXA_WRMASK | (uint32)FM_BA_WRMASK | (uint32)FM_RA_WRMASK))

#define FM_AUTO_SEQ_MASK            ((uint32)1U << 24U)
#define FM_SCALE_PER_WRMASK         (~TIMER_SCALE_PER_MASK)

#define DRMM_SET_MASK               ((uint32)1U << 22U)
#define DRMM_RESET_MASK             (~((uint32)DRMM_SET_MASK))

#define PL_SOFT_SET_MASK            ((uint32)1U << 29U)

#define FM_RWW_MODE_RD_MASK         ((uint32)FM_RWW_MODE_MASK << 24U)

#define FM_PRE_PROG_MASK            ((uint32)1U << 25U)
#define FM_PRE_PROG_WRMASK          (~((uint32)FM_PRE_PROG_MASK))
#define FM_PRE_PROG_CSL_MASK        ((uint32)1U << 26U)
#define FM_PRE_PROG_CSL_WRMASK      (~((uint32)FM_PRE_PROG_CSL_MASK))

#define FM_PUMP_EN_MASK             ((uint32)1U << 29U)
#define FM_ACLK_EN_MASK             ((uint32)1U << 30U)
#define FM_TIMER_EN_MASK            ((uint32)1U << 31U)
#define FM_TIMER_PUMP_MASK          ((uint32)FM_TIMER_EN_MASK | (uint32)FM_PUMP_EN_MASK)
#define FM_TIMER_ACLK_MASK          ((uint32)FM_TIMER_EN_MASK | (uint32)FM_ACLK_EN_MASK)

#define FM_CDAC_WRMASK              (~(FM_CDAC_MASK << 5U))
#define FM_ITIM_WRMASK              (~(FM_ITIM_MASK << 7U))

#define FM_LP_ULP_MASK              ((uint32)1U << 19U)
#define FM_LP_ULP_WRMASK            (~((uint32)FM_LP_ULP_MASK))

#define FM_READY_RESTART_SET_MASK   ((uint32)1U << 16U)
#define FM_READY_RESTART_CLR_MASK   (~((uint32)FM_READY_RESTART_SET_MASK))
#define FM_READY_DIS_POLL_MASK      ((uint32)1U << 5U)
#define FM_BUSY_POLL_MASK           ((uint32)1U << 8U)

#define FM_MDAC_WRMASK              (~((uint32)FM_MDAC_MASK))
#define FM_PDAC_WRMASK              (~((uint32)FM_PDAC_MASK << 8U))
#define FM_NDAC_WRMASK              (~((uint32)FM_NDAC_MASK))
#define FM_SDAC_WRMASK              (~((uint32)FM_SDAC_MASK << 5U))

#define FM_PN_CTL_NEG               0U
#define FM_PN_CTL_POS               1U

#define FM_TM_WRMASK                (~((uint32)FM_TM_MASK))
#define FM_PN_CTL_MASK              ((uint32)1U << 8U)
#define FM_PN_CTL_WRMASK            (~((uint32)FM_PN_CTL_MASK))
#define FM_TM_PE_MASK               ((uint32)1U << 9U)
#define FM_TM_PE_WRMASK             (~((uint32)FM_TM_PE_MASK))
#define FM_TM_DISPOS_MASK           ((uint32)1U << 10U)
#define FM_TM_DISPOS_WRMASK         (~((uint32)FM_TM_DISPOS_MASK))
#define FM_TM_DISNEG_MASK           ((uint32)1U << 11U)
#define FM_TM_DISNEG_WRMASK         (~((uint32)FM_TM_DISNEG_MASK))
#define FM_TM_CMPR_MASK             ((uint32)1U << 12U)
#define FM_TM_CMPR_WRMASK           (~((uint32)FM_TM_CMPR_MASK))
#define FM_EN_CLK_MON_MASK          ((uint32)1U << 16U)
#define FM_EN_CLK_MON_WRMASK        (~((uint32)FM_EN_CLK_MON_MASK))

#define FM_TEST_CTL_SETPUMP_MASK    ((uint32)FM_TM_MASK | (uint32)FM_TM_PE_MASK | (uint32)FM_PN_CTL_MASK)

#define FM_WAIT_MEM_RD_WRMASK       (~((uint32)FM_WAIT_MEM_RD_MASK))
#define FM_WAIT_HVPL_RD_WRMASK      (~((uint32)FM_WAIT_HVPL_RD_MASK << 8U))
#define FM_WAIT_HVPL_WR_WRMASK      (~((uint32)FM_WAIT_HVPL_WR_MASK << 16U))

#define FM_ECC_ENC_MASK             ((uint32)1U << 8U)
#define FM_ECC_ENC_WRMASK           (~((uint32)FM_ECC_ENC_MASK))

#define FM_RGRANT_PRG_LOADED_MASK   BIT31_MASK

#define FM_RST_SFT_HVPL_MASK        BIT10_MASK

#define FM_INTR_CLEAR_MASK          1U
#define FM_INTR_MASK_SET_MASK       1U
#define FM_INTR_MASK_CLEAR_MASK     0U

#define FM_RWW_WITH_RGRANT          2U

#define FM_OPERATION_BLOCKING       0U

//------------- Trim Bits Mask ----------------

#define HV_PARAM_SEQ12_MASK         0x00000000U
#define HV_PARAM_SEQ23_MASK         0x00000000U
#define HV_PARAM_ANA_CTL0_MASK      0x00000FFFU
#define HV_PARAM_ANA_CTL1_MASK      0x0000FFFFU
#define HV_PARAM_TIMER_CLK_MASK     0x00000000U
#define HV_PARAM_TIMER_CTL_MASK     0xFBFFFFFFU
#define HV_PARAM_SCALE_ERS_MASK     0x00000000U
#define HV_PARAM_DELAY_ERS_MASK     0x00000000U
#define HV_PARAM_WAIT_CTL_MASK      0xDFFFFFFFU
#define HV_PARAM_DELAY_PRG_MASK     0x00000000U

// -- Redundancy Mask --------
#define FM_RED0_HV_WRMASK           (~(BYTE0_MASK))
#define FM_RED1_HV_WRMASK           (~(BYTE1_MASK))

//=========================================
//  FM HV #DEFINES OFFSETS
// Offset from the table base [96]
//=========================================
#define FM_FUNCTION_PARAMETERS_SIZE 32U
#define FM_FUNCTIONS_RESULTS_SIZE   31U

// JIRA CDT_004885-57
#define FM_HV_TRIM_SIZE             128U // SM Row 0
#define FM_HV_PARAM_SIZE            37U  // SM Row 2

#define FM_TRIM_BITS_SIZE           10U // SW_SDAC to SW_VCT_TRIM_HV

//====== START OF HVPARAM TABLE_DEFAULT  OFFSETS ============//

// JIRA CDT_004885-57
#define SW_PgPrePgmPW_Lo            0U
#define SW_PgPrePgmPW_Hi            0U
#define SW_PgPrePgm_NDAC            0U
#define SW_PgPrePgm_PDAC            0U
#define SW_PgPrePgm_CDAC            1U
#define SW_PgPrePgm_MDAC            1U
#define SW_NOT_USED_0               1U
#define SW_NOT_USED_1               1U
#define SW_SubSectPrePgmPW_Lo       2U
#define SW_SubSectPrePgmPW_Hi       2U
#define SW_SubSectPrePgm_NDAC       2U
#define SW_SubSectPrePgm_PDAC       2U
#define SW_SubSectPrePgm_CDAC       3U
#define SW_SubSectPrePgm_MDAC       3U
#define SW_NOT_USED_2               3U
#define SW_NOT_USED_3               3U
#define SW_SectPrePgmPW_Lo          4U
#define SW_SectPrePgmPW_Hi          4U
#define SW_SectPrePgm_NDAC          4U
#define SW_SectPrePgm_PDAC          4U
#define SW_SectPrePgm_CDAC          5U
#define SW_SectPrePgm_MDAC          5U
#define SW_NOT_USED_4               5U
#define SW_NOT_USED_5               5U
#define SW_BulkPrePgmPW_Lo          6U
#define SW_BulkPrePgmPW_Hi          6U
#define SW_BulkPrePgm_NDAC          6U
#define SW_BulkPrePgm_PDAC          6U
#define SW_BulkPrePgm_CDAC          7U
#define SW_BulkPrePgm_MDAC          7U
#define SW_NOT_USED_6               7U
#define SW_NOT_USED_7               7U
#define SW_PgPgmPW_Lo               8U
#define SW_PgPgmPW_Hi               8U
#define SW_PgPgm_NDAC               8U
#define SW_PgPgm_PDAC               8U
#define SW_PgPgm_CDAC               9U
#define SW_PgPgm_MDAC               9U
#define SW_NOT_USED_8               9U
#define SW_NOT_USED_9               9U
#define SW_SectPgmPW_Lo             10U
#define SW_SectPgmPW_Hi             10U
#define SW_SectPgm_NDAC             10U
#define SW_SectPgm_PDAC             10U
#define SW_SectPgm_CDAC             11U
#define SW_SectPgm_MDAC             11U
#define SW_NOT_USED_10              11U
#define SW_NOT_USED_11              11U
#define SW_BulkPgmPW_Lo             12U
#define SW_BulkPgmPW_Hi             12U
#define SW_BulkPgm_NDAC             12U
#define SW_BulkPgm_PDAC             12U
#define SW_BulkPgm_CDAC             13U
#define SW_BulkPgm_MDAC             13U
#define SW_NOT_USED_12              13U
#define SW_NOT_USED_13              13U
#define SW_PgErsPW_Lo               14U
#define SW_PgErsPW_Hi               14U
#define SW_PgErs_NDAC               14U
#define SW_PgErs_PDAC               14U
#define SW_PgErs_CDAC               15U
#define SW_NOT_USED_14              15U
#define SW_NOT_USED_15              15U
#define SW_NOT_USED_16              15U
#define SW_SubSectErsPW_Lo          16U
#define SW_SubSectErsPW_Hi          16U
#define SW_SubSectErs_NDAC          16U
#define SW_SubSectErs_PDAC          16U
#define SW_SubSectErs_CDAC          17U
#define SW_NOT_USED_17              17U
#define SW_NOT_USED_18              17U
#define SW_NOT_USED_19              17U
#define SW_SectErsPW_Lo             18U
#define SW_SectErsPW_Hi             18U
#define SW_SectErs_NDAC             18U
#define SW_SectErs_PDAC             18U
#define SW_SectErs_CDAC             19U
#define SW_NOT_USED_20              19U
#define SW_NOT_USED_21              19U
#define SW_NOT_USED_22              19U
#define SW_BulkErsPW_Lo             20U
#define SW_BulkErsPW_Hi             20U
#define SW_BulkErs_NDAC             20U
#define SW_BulkErs_PDAC             20U
#define SW_BulkErs_CDAC             21U
#define SW_NOT_USED_23              21U
#define SW_NOT_USED_24              21U
#define SW_NOT_USED_25              21U
#define SW_Seq1PW_Lo                22U
#define SW_Seq1PW_Hi                22U
#define SW_Seq2PrePW_Lo             22U
#define SW_Seq2PrePW_Hi             22U
#define SW_Seq2PostPW_Lo            23U
#define SW_Seq2PostPW_Hi            23U
#define SW_Seq3PW_Lo                23U
#define SW_Seq3PW_Hi                23U
#define SW_NPDAC_STEP_TIME          24U
#define SW_NPDAC_ZERO_TIME          24U
#define SW_NOT_USED_26              24U
#define SW_NOT_USED_27              24U
#define SW_NDAC_MIN                 25U
#define SW_PDAC_MIN                 25U
#define SW_SCALE_SEQ01              25U
#define SW_SCALE_SEQ12              25U
#define SW_SCALE_SEQ23              26U
#define SW_SCALE_SEQ30              26U
#define SW_SCALE_PE_ON              26U
#define SW_SCALE_PE_OFF             26U
#define SW_TIMER_CLOCK_FRQ          27U
#define SW_RGRANT_DELAY_PEON        27U
#define SW_RGRANT_DELAY_PEOFF       27U
#define SW_RGRANT_DELAY_SEQ01       27U
#define SW_RGRANT_DELAY_SEQ12       28U
#define SW_RGRANT_DELAY_SEQ23       28U
#define SW_RGRANT_DELAY_SEQ30       28U
#define SW_RGRANT_DELAY_CLK         28U
#define SW_PREPROG_CSL              29U
#define SW_NOT_USED_28              29U
#define SW_NOT_USED_29              29U
#define SW_NOT_USED_30              29U
#define SW_NegMM_NDAC               30U
#define SW_NegMM_PDAC               30U
#define SW_NegMM_CDAC               30U
#define SW_PosMM_NDAC               30U
#define SW_PosMM_PDAC               31U
#define SW_PosMM_CDAC               31U
#define SW_TtmdDelay_Lo             31U
#define SW_TtmdDelay_Hi             31U
#define SW_PUMP_OFF_DELAY           32U
#define SW_MDAC_OFF_DELAY           32U
#define SW_NOT_USED_31              32U
#define SW_NOT_USED_32              32U
#define SW_SCALE_ERS_SEQ01          33U
#define SW_SCALE_ERS_SEQ12          33U
#define SW_SCALE_ERS_SEQ23          33U
#define SW_SCALE_ERS_PEON           33U
#define SW_SCALE_ERS_PEOFF          34U
#define SW_RGRANT_DELAY_ERS_PEON    34U
#define SW_RGRANT_DELAY_ERS_PEOFF   34U
#define SW_NOT_USED_33              34U
#define SW_RGRANT_DELAY_ERS_SEQ01   35U
#define SW_RGRANT_DELAY_ERS_SEQ12   35U
#define SW_RGRANT_DELAY_ERS_SEQ23   35U
#define SW_NOT_USED_34              35U
#define SW_PL_SOFT_SET_EN           36U
#define SW_NOT_USED_35              36U
#define SW_NOT_USED_36              36U
#define SW_NOT_USED_37              36U

//======== END OF HVPARAM TABLE_DEFAULT  OFFSETS ============//

//====== START OF HV TRIMS  OFFSETS ============//

#define SW_RED_ADDR_S0              32U    
#define SW_RED_ADDR_S1              33U
#define SW_RED_ADDR_S2              34U
#define SW_RED_ADDR_S3              35U
#define SW_TRIM_NOT_USED_20         36U
#define SW_TRIM_NOT_USED_19         37U
#define SW_TRIM_NOT_USED_18         38U
#define SW_TRIM_NOT_USED_17         39U
#define SW_TRIM_NOT_USED_16         40U
#define SW_TRIM_NOT_USED_15         41U
#define SW_RED_EN_S_7_0             42U
#define SW_TRIM_NOT_USED_14         43U
#define SW_VCT_TRIM_HV              44U 
#define SW_CDAC_HV                  45U 
#define SW_VBG_TRIM_HV              46U 
#define SW_VBG_TC_TRIM_HV           47U
#define SW_IPREF_TRIMA_HV           48U 
#define SW_SPARE_CTL0_HV            49U 
#define SW_ICREF_TRIM_HV            50U
#define SW_ICREF_TC_TRIM_HV         51U
#define SW_IPREF_TRIM_HV            52U
#define SW_IPREF_TC_HV              53U
#define SW_SPARE_CTL1_HV            54U
#define SW_IDAC_ULP_HV              55U
#define SW_SPARE_ULP_CTL2_HV        56U
#define SW_VREF_SEL_ULP_HV          57U
#define SW_IDAC_LP_HV               58U
#define SW_SPARE_LP_CTL2_HV         59U
#define SW_VREF_SEL_LP_HV           60U
#define SW_OSC_TRIM_HV              61U
#define SW_OSC_RANGE_TRIM_HV        62U
#define SW_VPROT_ACT_HV             63U
#define SW_OSC_TEMPCO_HV            64U
#define SW_LAT_DIS3_HV              65U
#define SW_PM_EN_HV                 66U
#define SW_REG_ACT_HV               67U
#define SW_FDIV_TRIM_HV             68U
#define SW_VDDHI_HV                 69U
#define SW_TURBO_PULSEW_HV          70U
#define SW_IOSC_TRIM_HV             71U
#define SW_CL_ISO_DIS_HV            72U
#define SW_R_GRANT_EN_HV            73U
#define SW_LP_ULP_SW_HV             74U
#define SW_VLIM_TRIM_ULP_HV         75U
#define SW_SPARE_CTL4_ULP_HV        76U
#define SW_SDAC_ULP_HV              77U
#define SW_ITIM_ULP_HV              78U
#define SW_FM_READY_DEL_ULP_HV      79U
#define SW_SA_CTL_TRIM_T8_ULP_HV    80U
#define SW_READY_RESTART_N_HV       81U
#define SW_VBST_S_DIS_HV            82U
#define SW_AUTO_HVPULSE_HV          83U
#define SW_UGB_EN_HV                84U
#define SW_VLIM_TRIM_LP_HV          85U
#define SW_SPARE_CTL5_LP_HV         86U
#define SW_SDAC_LP_HV               87U
#define SW_ITIM_LP_HV               88U
#define SW_FM_READY_DEL_LP_HV       89U
#define SW_SA_CTL_TRIM_T8_LP_HV     90U
#define SW_SPARE2_CTL5_LP_HV        91U
#define SW_AMUX_SEL_HV              92U
#define SW_SA_CTL_TRIM_T1_ULP_HV    93U
#define SW_SA_CTL_TRIM_T4_ULP_HV    94U
#define SW_SA_CTL_TRIM_T5_ULP_HV    95U
#define SW_SA_CTL_TRIM_T6_ULP_HV    96U
#define SW_SA_CTL_TRIM_T1_LP_HV     97U
#define SW_SA_CTL_TRIM_T4_LP_HV     98U
#define SW_SA_CTL_TRIM_T5_LP_HV     99U
#define SW_SA_CTL_TRIM_T6_LP_HV     100U
#define SW_ERSX8_CLK_SEL_HV         101U
#define SW_FM_ACTIVE_HV             102U
#define SW_TURBO_EXT_HV             103U
#define SW_NPDAC_HWCTL_DIS_HV       104U
#define SW_FM_READY_DIS_HV          105U
#define SW_ERSX8_EN_ALL_HV          106U
#define SW_READY_DEL_HV             107U
#define SW_SPARE_CTL7_HV            108U
#define SW_PTRIM_ULP_HV             109U
#define SW_SPARE2_CTL7_ULP_HV       110U
#define SW_PTRIM_LP_HV              111U
#define SW_SPARE3_CTL7_LP_HV        112U
#define SW_TRIM_NOT_USED_6          113U
#define SW_TRIM_NOT_USED_5          114U
#define SW_TRIM_NOT_USED_4          115U
#define SW_TRIM_NOT_USED_3          116U
#define SW_TRIM_NOT_USED_2          117U
#define SW_TRIM_NOT_USED_1          118U
#define SW_TRIM_NOT_USED_0          119U
#define SW_VIRGIN_KEY_VAL_LO0       120U
#define SW_VIRGIN_KEY_VAL_LO1       121U
#define SW_VIRGIN_KEY_VAL_LO2       122U
#define SW_VIRGIN_KEY_VAL_LO3       123U
#define SW_VIRGIN_KEY_VAL_HI0       124U
#define SW_VIRGIN_KEY_VAL_HI1       125U
#define SW_VIRGIN_KEY_VAL_HI2       126U
#define SW_VIRGIN_KEY_VAL_HI3       127U

//======== END OF HV TRIMS  OFFSETS ============//

#define FM_SEQ12_ADDR_VAR(X)        ((X) + (SW_Seq1PW_Lo * 4U))                 /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_SEQ23_ADDR_VAR(X)        ((X) + (SW_Seq2PostPW_Lo * 4U))             /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_ANA_CTL0_ADDR_VAR(X)     ((X) + (SW_NDAC_MIN * 4U))                  /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_ANA_CTL1_ADDR_VAR(X)     ((X) + (SW_NPDAC_STEP_TIME * 4U))           /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_TIMER_CLK_ADDR_VAR(X)    ((X) + (SW_TIMER_CLOCK_FRQ * 4U))           /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_TIMER_CTL_ADDR_VAR(X)    ((X) + (SW_PREPROG_CSL * 4U))               /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_SCALE_ERS_ADDR_VAR(X)    ((X) + (SW_SCALE_ERS_SEQ01 * 4U))           /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_DELAY_ERS_ADDR_VAR(X)    ((X) + (SW_RGRANT_DELAY_ERS_SEQ01 * 4U))    /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_DELAY_PRG_ADDR_VAR(X)    ((X) + (SW_RGRANT_DELAY_SEQ12 * 4U))        /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */
#define FM_WAIT_CTL_ADDR_VAR(X)     ((X) + (SW_PL_SOFT_SET_EN * 4U))            /* PRQA S 3472 # Function-like macro is used to allow logic inlined. */

#define FM_VIRGIN_KEY_BASE_ADDR     (FM_MEM_DATA_ADDR + (4U * SW_VIRGIN_KEY_VAL_LO0))
#define FM_SMROW_RED_ADDR           (FM_MEM_DATA_ADDR + (4U * SW_RED_ADDR_S0))
#define FM_SMROW_REDEN_ADDR         (FM_MEM_DATA_ADDR + (4U * SW_RED_EN_S_7_0))
#define FM_SMROW_RGRANTEN_ADDR      (FM_MEM_DATA_ADDR + (4U * SW_R_GRANT_EN_HV))

#define FM_SMROW_TRIM_ADDR0         (FM_MEM_DATA_ADDR + (4U * SW_VCT_TRIM_HV))
#define FM_SMROW_TRIM_ADDR1         (FM_MEM_DATA_ADDR + (4U * SW_ICREF_TRIM_HV))
#define FM_SMROW_TRIM_ADDR2         (FM_MEM_DATA_ADDR + (4U * SW_IDAC_ULP_HV))
#define FM_SMROW_TRIM_ADDR3         (FM_MEM_DATA_ADDR + (4U * SW_OSC_TRIM_HV))
#define FM_SMROW_TRIM_ADDR4         (FM_MEM_DATA_ADDR + (4U * SW_VLIM_TRIM_ULP_HV))
#define FM_SMROW_TRIM_ADDR5         (FM_MEM_DATA_ADDR + (4U * SW_VLIM_TRIM_LP_HV))
#define FM_SMROW_TRIM_ADDR6         (FM_MEM_DATA_ADDR + (4U * SW_SA_CTL_TRIM_T1_ULP_HV))
#define FM_SMROW_TRIM_ADDR7         (FM_MEM_DATA_ADDR + (4U * SW_ERSX8_CLK_SEL_HV))

#define FM_SMROW_EXEC_ADDR          (FM_MEM_DATA_ADDR + (4U * SW_EXEC_FROM_FM))

#define FM_RED_NADDR_DIV2           2U

#define FM_KEY_SIZE                 8U

/**** Shift table sizes ****/
#define SHIFT_TABLE_0_SIZE          6U
#define SHIFT_TABLE_1_SIZE          5U
#define SHIFT_TABLE_2_SIZE          6U
#define SHIFT_TABLE_3_SIZE          14U
#define SHIFT_TABLE_4_SIZE          10U
#define SHIFT_TABLE_5_SIZE          8U
#define SHIFT_TABLE_6_SIZE          8U
#define SHIFT_TABLE_7_SIZE          12U

/**** HV shift table sizes ****/
#define SHIFT_ANA_CTL0_SIZE         8U
#define SHIFT_ANA_CTL1_SIZE         2U
#define SHIFT_SEQ123_SIZE           4U
#define SHIFT_TIMER_CTL_SIZE        1U
#define SHIFT_SCALE_ERS_SIZE        7U
#define SHIFT_WAIT_CTL_SIZE         1U

//====================================================
extern const uint32 key_expected[FM_KEY_SIZE];
extern const uint32 c_patterns[4];

extern const uint32 shift0[SHIFT_TABLE_0_SIZE];
extern const uint32 shift1[SHIFT_TABLE_1_SIZE];
extern const uint32 shift2[SHIFT_TABLE_2_SIZE];
extern const uint32 shift3[SHIFT_TABLE_3_SIZE];
extern const uint32 shift4[SHIFT_TABLE_4_SIZE];
extern const uint32 shift5[SHIFT_TABLE_5_SIZE];
extern const uint32 shift6[SHIFT_TABLE_6_SIZE];
extern const uint32 shift7[SHIFT_TABLE_7_SIZE];

extern const uint32 shiftanactl0[SHIFT_ANA_CTL0_SIZE];
extern const uint32 shiftanactl1[SHIFT_ANA_CTL1_SIZE];      /* PRQA S 0776 # This code is conformed to C99 standard. */
extern const uint32 shiftseq123[SHIFT_SEQ123_SIZE];
extern const uint32 shifttimerctl[SHIFT_TIMER_CTL_SIZE];
extern const uint32 shiftscaleers[SHIFT_SCALE_ERS_SIZE];    /* PRQA S 0776 # This code is conformed to C99 standard. */
extern const uint32 shiftwaitctl[SHIFT_WAIT_CTL_SIZE];

extern uint32 FmFuncAddr[1];   /* #data# */
extern uint32 FmFuncRetVal[1]; /* #data# */   /* PRQA S 0776, 3449, 3451 # This code is conformed to C99 standard. This global identifier needs to be declared in this location. The same declaration exists in dependency files which is not within our control. */
extern uint32 FmApiStatus[1];  /* #data# */   /* PRQA S 3449, 3451 # This global identifier needs to be declared in this location. The same declaration exists in dependency files which is not within our control. */

//-----------------------------------------------------------------
// RAM Location to store the Fm addr
extern uint32 FmRowOrAxa[1];            /* #data# */ 	/* bit 31=1 axa, bit32 = 0 RowAbsAddr format    */
extern uint32 FmBaParam[1];             /* #data# */ 	/* sector                                       */
extern uint32 FmRaParam[1];             /* #data# */	/* row number within a sector                   */
extern uint32 FmBxaParam[1];            /* #data# */	/* flag to access scratch row                   */
extern uint32 FmFuncArg[1];             /* #data# */ 	/* function argument                            */     /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmHvParamTableAddr[1];    /* #data# */    /* Hv Param Address                             */
extern uint32 FmRamAddr[1];             /* #data# */    /* ram Buffer address                           */
extern uint32 FmRwwMode[1];             /* #data# */    /* RWW mode: 0=Non RWW; 1=RWW                   */
extern uint32 FmMode[1];                /* #data# */                                                           /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmHvParamOffset[1];       /* #data# */                                                           /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmEraseWithPreprogram[1]; /* #data# */                                                           /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmHvTrimAddr[1];          /* #data# */    /* Hv Trim Address                              */
extern uint32 FmEccAddr[1];             /* #data# */    /* Scratch data for manual ECC data injection   */

// RAM Location to store the Function parameters
extern uint32 FmFuncParam0[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam1[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam2[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam3[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam4[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam5[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam6[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncParam7[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */

//-----------------------------------------------------------------
// RAM Location to store the FM functions Results
extern uint32 FmFuncResults0[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults1[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults2[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults3[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults4[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults5[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults6[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmFuncResults7[1]; /* #data# */  /* PRQA S 0776 # This code is conformed to C99 standard. */

// RAM Location to store the HV Parameters Table
// JIRA CDT_004885-57
extern uint32 FmHvParamTable[FM_HV_PARAM_SIZE]; /* #data# */ /* Hv parameters   */  /* PRQA S 0776 # This code is conformed to C99 standard. */
extern uint32 FmHvTrimTable[FM_HV_TRIM_SIZE];   /* #data# */ /* Trim parameters */  /* PRQA S 0776 # This code is conformed to C99 standard. */

// RAM Buffer for 8 rows (subsector) manipulation : 4096 x32 bit words
extern uint32 RamFmBuffer[RAM_BUFFER_SIZE];
extern uint32 RamECCBuffer[FM_MAX_COLUMNS];

//====================================================
struct stFmParams	/* PRQA S 3630 # The usage of struct/union is allowed in this project. */
{
    uint32 funcAddr;
    uint32 funcRetVal;
    uint32 apiStatus;
    uint32 axa;
    uint32 ba;
    uint32 ra;
    uint32 bxa;
    uint32 funcArg;
    uint32 hvTableAddr;
    uint32 ramAddr;
    uint32 rwwMode;
    uint32 mode;
    uint32 hvParamOffset;
    uint32 StandAlonePreprogram;
    uint32 hvTrimAddr;
    uint32 eccAddr;
    uint32 funcParam[8];
    uint32 funcResults[8];
};
typedef struct stFmParams typeFmParams;

//-----------------------------------

struct stMisrGen
{
    uint32 addr1;
    uint32 addr2;
    uint32 resultAddr;
};
typedef struct stMisrGen typeMisrGen;

//-----------------------------------

struct stBulkSecErsVer
{
    typeFmParams *p_fmParams;
    uint32 bulkSectSubs;
    uint32 verify;
};
typedef struct stBulkSecErsVer typeBulkSecErsVer;

//-----------------------------------

struct stFmIncAddr
{
    typeFmParams *p_fmParams;
    uint32 bulkSectSubs; // 0: bulk, 1: sect, 2: subsect
    uint32 sectorStep;
    uint32 rowStep;
};
typedef struct stFmIncAddr typeFmIncAddr;

//-----------------------------------

#ifndef CY_FLASH_FIX_SKIP_API_DECL
/* s40flash APIs */
uint32 RdFmIfSel(void);
uint32 RdFmWrEn(void);
uint32 RdFmTimerStatus(void);
uint32 RdFmHvRegsIsolated(void);
uint32 RdFmIllegalOp(void);
uint32 RdFmTurboN(void);
uint32 RdFmWrEnMon(void);                                   /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 RdFmIfSelMon(void);                                  /* PRQA S 0776 # This code is conformed to C99 standard. */
//void WrFmIfSel(uint32 enable);
void WrFmWrEn(uint32 enable);
void WrFmIdacTrim(uint32 din);
void WrFmItimTrim(uint32 din);
void FmSetRwwMode(uint32 local_rwwMode);
uint32 FmErasePage(typeFmParams *p_paramAddr);              /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmEraseSector(typeFmParams *p_paramAddr);            /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmEraseSubSector(typeFmParams *p_paramAddr);         /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmEraseBulkAll(typeFmParams *p_paramAddr);           /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmProgramPage(typeFmParams *p_paramAddr);
uint32 FmProgramSubsectorAll(typeFmParams *p_paramAddr);    /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmProgramSector(typeFmParams *p_paramAddr);          /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmProgramSectorAll(typeFmParams *p_paramAddr);       /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmProgramBulk(typeFmParams *p_paramAddr);            /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmProgramBulkAll(typeFmParams *p_paramAddr);         /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmPreProgramPage(typeFmParams *p_paramAddr);
uint32 FmPreProgramSubsectorAll(typeFmParams *p_paramAddr); /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmPreProgramSectorAll(typeFmParams *p_paramAddr);    /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 FmPreProgramBulkAll(typeFmParams *p_paramAddr);      /* PRQA S 0776 # This code is conformed to C99 standard. */
void FmPageWrite(typeFmParams *p_paramAddr);
uint32 FmProgramVerifyPage(typeFmParams *p_paramAddr);      /* PRQA S 0776 # This code is conformed to C99 standard. */
uint32 read_key(void);
void WriteHvParameters(uint32 paramAddr);
uint32 writeTrimBits(void);
uint32 FmBulkCheckNotAxaBxa(typeFmParams *p_paramAddr);
uint32 FmSectorCheckNotBxa(typeFmParams *p_paramAddr);
uint32 FmCalibrate(uint32 paramAddr);
void FM_RST_SFT_HVPL(void);
void FM_WRITE_ALL_HVPL(uint32 din);
uint32 FmSecurityEraseBulkAll(typeFmParams *p_paramAddr);
void Fm_Lp(void);
void Fm_Ulp(void);
//void Fm_Write_Data_Hvpl(uint32 din);                        /* PRQA S 0776 # This code is conformed to C99 standard. */
void HV_PulseResumeAll(void);

/* s40flash exported functions (used in TSRAM) */
uint32 majorityData(uint32 data);
uint32 RdFmWordSize(void);
uint32 RdFmPageSize(void);
uint32 RdFmRowCount(uint32 axa);
uint32 RdFmSectorCount(uint32 axa);
void WrFmModeSeq(uint32 mode, uint32 seq);
void WrFmSeq(uint32 seq);
void WrFmRa(uint32 ra);
void FmTimer(uint32 period);
void FmTimerPumpEn(uint32 period);                          /* PRQA S 0776 # This code is conformed to C99 standard. */
void FmTimerAclkEn(uint32 period);                          /* PRQA S 0776 # This code is conformed to C99 standard. */
void WrFmMdac(uint32 din);
void WrFmNdac(uint32 din);
void WrFmPdac(uint32 din);
void WrFmCdac(uint32 din);
void WrFmTestMode(uint32 din);
void WrFmIprefTrim(uint32 din);
void WrFmIcrefTrim(uint32 din);
void WrFmVbgTrim(uint32 din);
void WrCXAFromVar(uint32 cxa_din);
void WrFmMba(void);
//void CopyRam2FmPl(uint32 ramAddr);
uint32 WrFmAddr(typeFmParams *p_paramAddr);
uint32 CompareFmRow2FmPl(typeFmParams *p_paramAddr);
//void FmSetReadMode(void);                                   /* PRQA S 0776 # This code is conformed to C99 standard. */
//uint32 FmLoadDacsProgram(const uint32 *dacParams);
//uint32 FmLoadDacsErase(const uint32 *dacParams);            /* PRQA S 0776 # This code is conformed to C99 standard. */
void FM_Lp_Ulp_Gen(uint32 lp_ulp_bit);
void fm_wait_us(uint32 delay);
uint32 setSflashRow(uint32 sflash_row_num);
#endif /* CY_FLASH_FIX_SKIP_API_DECL */

//-----------------------------------
#endif
