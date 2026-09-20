/*******************************************************************************
 * File Name:   device_state.h
 *
 * Description:  This file is the public interface of maintaining device state
 *               required for live update.
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

#ifndef _DEVICE_STATE_H_
#define _DEVICE_STATE_H_

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Global Variables
 *******************************************************************************/
typedef struct {
uint32_t start_magic;
uint32_t live_update; /* not requested, requested */
uint32_t ppca0_state; /* ppca0 off, ppca0 running, ppca0 update requested */
uint32_t ppca1_state; /* ppca1 off, ppca1 running, ppca1 update requested */
uint32_t main_timer_count;
uint32_t end_magic;
}device_state_t;

extern volatile device_state_t device_state;

/*******************************************************************************
 * Function prototypes
 *******************************************************************************/

/* Returns true when the current boot is a live update. */
bool is_live_update(void);

/* Track the run state of each PPCA core in persistent
 * device state. 'core' is 0 for Core 0 and 1 for Core 1. State values use
 * ppca_core_state_t from ipc_defs.h. */
void main_set_ppca_state(uint32_t core, uint32_t state);
uint32_t main_get_ppca_state(uint32_t core);


#endif /* _PPCA_INIT_H_ */