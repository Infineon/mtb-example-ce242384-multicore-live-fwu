/*******************************************************************************
 * File Name:   ipc_defs.h
 *
 * Description: Shared definitions for the Main CPU <-> PPCA core IPC notify
 *              mailbox protocol used by the Multicore Live Firmware Update
 *              code example.
 *
 *              The protocol replaces the previous lock-poll scheme with a
 *              notify-interrupt based mailbox so that the PPCA cores no longer
 *              need to busy-poll the IPC lock, keeping their interrupts
 *              serviceable for most of the time.
 *
 *              Channel allocation (each channel is unidirectional, so there is
 *              no lock contention and the routing of the notify interrupt to a
 *              specific CPU's NVIC line is deterministic):
 *
 *   +----+-------------------+-----------------------+---------------+-------------------------------+
 *   | Ch | IPC STRUCT        | IPC INTR STRUCT       | Direction     | Receiver NVIC line            |
 *   +----+-------------------+-----------------------+---------------+-------------------------------+
 *   | 0  | PPCA_IPC_STRUCT0  | PPCA_IPC_INTR_STRUCT0 | core0 -> main | main  ppca_ipc_0_IRQn (#47)    |
 *   | 1  | PPCA_IPC_STRUCT1  | PPCA_IPC_INTR_STRUCT1 | core1 -> main | main  ppca_ipc_1_IRQn (#48)    |
 *   | 2  | PPCA_IPC_STRUCT2  | PPCA_IPC_INTR_STRUCT2 | main -> core0 | core0 ppca_ipc_2_IRQn (#49)    |
 *   | 3  | PPCA_IPC_STRUCT3  | PPCA_IPC_INTR_STRUCT3 | main -> core1 | core1 ppca_ipc_3_IRQn (#50)    |
 *   +----+-------------------+-----------------------+---------------+-------------------------------+
 *
 *              For channel n the sender uses PPCA_IPC_STRUCTn and a notify mask
 *              of (1u << n) (which pulses PPCA_IPC_INTR_STRUCTn). The receiver
 *              enables the matching notify bit (1u << n) in
 *              PPCA_IPC_INTR_STRUCTn's interrupt mask.
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

#ifndef IPC_DEFS_H
#define IPC_DEFS_H

#include <stdint.h>

/*******************************************************************************
 * Persistent live-update state magics (shared with startup detection)
 *******************************************************************************/

/* Start magic stored in word[0] of the persistent state structure. */
#define LIVE_DATA_START_MAGIC   (0xA5A5A5A5UL)

/* End magic stored in the last word of the persistent state structure. */
#define LIVE_DATA_END_MAGIC     (0x5A5A5A5AUL)

/* Value of the live_update flag (word[1]) indicating a live update boot.
 * The shared startup uses word[0] == LIVE_DATA_START_MAGIC and
 * word[1] == LIVE_UPDATE_ACTIVE to take the fast live-update boot path. */
#define LIVE_UPDATE_INACTIVE    (0UL)
#define LIVE_UPDATE_ACTIVE      (1UL)

/*******************************************************************************
 * IPC channel indices
 *******************************************************************************/

#define IPC_CH_CORE0_TO_MAIN    (0u)   /* PPCA_IPC_STRUCT0 / INTR_STRUCT0 */
#define IPC_CH_CORE1_TO_MAIN    (1u)   /* PPCA_IPC_STRUCT1 / INTR_STRUCT1 */
#define IPC_CH_MAIN_TO_CORE0    (2u)   /* PPCA_IPC_STRUCT2 / INTR_STRUCT2 */
#define IPC_CH_MAIN_TO_CORE1    (3u)   /* PPCA_IPC_STRUCT3 / INTR_STRUCT3 */

/* Notify / interrupt-mask bit for channel n. */
#define IPC_NOTIFY_MASK(ch)     (1uL << (ch))

/*******************************************************************************
 * IPC messages
 *******************************************************************************/

typedef enum
{
    IPC_MSG_NONE       = 0u,
    IPC_MSG_CORE_UP    = 1u, /* PPCA core -> main: core booted and is running   */
    IPC_MSG_SWITCH_REQ = 2u, /* main -> PPCA core: switch to the new image       */
} ipc_msg_t;

/*******************************************************************************
 * PPCA core run state (tracked by the Main CPU in persistent state)
 *******************************************************************************/

typedef enum
{
    PPCA_STATE_OFF               = 0u, /* core not started yet                   */
    PPCA_STATE_RUNNING           = 1u, /* core booted and reported CORE_UP       */
    PPCA_STATE_UPDATE_REQUESTED  = 2u, /* switch requested, waiting for restart  */
} ppca_core_state_t;

#endif /* IPC_DEFS_H */
