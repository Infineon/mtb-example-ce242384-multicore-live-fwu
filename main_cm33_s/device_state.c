/*******************************************************************************
 * File Name:   device_state.c
 *
 * Description: This file contains the implementation for updating device state
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
#include <stdio.h>
#include "device_state.h"
#include "ipc_defs.h"


/*******************************************************************************
 * Function Name: is_live_update
 ********************************************************************************
 * Summary:
 *  Returns true when the current boot is a live update, determined from the
 *  persistent device state (valid start/end magics and the active flag set).
 *
 * Parameters:
 *  void
 *
 * Return:
 *  true if this boot is a live update, false otherwise.
 *
 *******************************************************************************/
bool is_live_update(void)
{
    return ((device_state.start_magic == LIVE_DATA_START_MAGIC) &&
           (device_state.end_magic   == LIVE_DATA_END_MAGIC) &&
		   (device_state.live_update == LIVE_UPDATE_ACTIVE));
}

/*******************************************************************************
 * Function Name: main_set_ppca_state / main_get_ppca_state
 ********************************************************************************
 * Summary:
 *  Accessors used by ppca_init.c to track the run state of each PPCA core in
 *  the persistent device state. 'core' is 0 for Core 0 and 1 for Core 1.
 *
 *******************************************************************************/
void main_set_ppca_state(uint32_t core, uint32_t state)
{
    if (core == 0u)
    {
        device_state.ppca0_state = state;
    }
    else
    {
        device_state.ppca1_state = state;
    }
}

uint32_t main_get_ppca_state(uint32_t core)
{
    return (core == 0u) ? device_state.ppca0_state : device_state.ppca1_state;
}