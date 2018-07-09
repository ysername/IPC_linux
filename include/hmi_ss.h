/*******************************************************************************
*  $Log: hmi_ss.h  $
*  Revision 1.1 2016/10/04 17:00:50CDT Daniel Kageff (10011932) 
*  Initial revision
*  Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/HMI/HMILink/project.pj
*  Revision 1.3 2014/06/03 12:39:27CDT Edgar Rios Lucas (10032297) 
*  Releasing HMI 1.0
*  Release notes and change analysis pending
*  Revision 1.2 2014/06/02 08:29:37CDT Kevin Russo (kevinadm) 
*  Member moved from hmi_ss.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/Application/Include/project.pj to hmi_ss.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/HMI/Link/project.pj.
*  Revision 1.1 2014/04/17 13:11:28EDT Gregory Palarski (10030324) 
*  Initial revision
*  Member added to project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/Application/Include/project.pj
*
*                           (c) Copyright 2012
*                           Yazaki North America
*                           All Rights Reserved
*  ----------------------------------------------------------------------------
*
*   Unpublished, Yazaki North America Inc. All rights reserved. 
*  This document is protected by the copyright law of the United States and 
*  foreign countries.  The work embodied in this document is confidential and 
*  constitutes a trade secret of Yazaki North America, Inc.  Any copying of 
*  this document and any use or disclosure of this document or the work 
*  embodied in it without the written authority of Yazaki North America, Inc., 
*  will constitute copyright and trade secret infringement and will violate the
*  rights of Yazaki North America, Inc.
*
*******************************************************************************/
/********************************************************************************************
*  File:  hmi_ss.h 
*
*  Description: This is the public interface file for the HMI subsystem component and contains
*    information related to the public API for this component.
********************************************************************************************/
#ifndef HMI_SS_H   
#define HMI_SS_H   

/*******************************************************************************************/
/*    I N C L U D E   F I L E S                                                            */
/*******************************************************************************************/
#include "hmi_ss_cfg.h"
#include <stdint.h>

/*******************************************************************************************/
/*    M A C R O S                                                                          */
/*******************************************************************************************/


/*******************************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                                          */
/*******************************************************************************************/


/*******************************************************************************************/
/*    F U N C T I O N   P R O T O T Y P E S                                                */
/*******************************************************************************************/

/********************************************************************************************
*  Function Name: HMISS_StartupInit
*
*  Description: Startup initialization for the HMI SS component.  This function must be called  
*     during startup initialization following reset (and done before periodic tasks are scheduled).
*    
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void HMISS_StartupInit(void);

/********************************************************************************************
*  Function Name: HMISS_Sleep
*
*  Description: Prepares the HMI subsystem component for sleep (low power state). 
*    
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void HMISS_Sleep(void);

/********************************************************************************************
*  Function Name: HMISS_Wakeup
*
*  Description: Prepares the HMISS for normal operation.  This function should be called when
*     exiting the low power state.
*
*  Input(s):    None. 
*
*  Outputs(s):  None.
*
*  Returns:     None. 
********************************************************************************************/
void HMISS_Wakeup(void);

/********************************************************************************************
*  Function Name: HMISS_PeriodicTask 
*
*  Description: Periodic task for the HMI SS component.  The HMI state machine is executed from .  
*    NOTE: This functin must be called at the rate defined in the HMI_SCHEDULING_RATE_MS macro.
*
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void HMISS_PeriodicTask(void);

/********************************************************************************************
*  Function Name: HMISS_PostHMIEvent
*
*  Description: This function posts an event to the HMI event queue.  Events posted to the   
*     queue are dequeued and delivered to the HMI state machine when the HMISS_Periodic task is 
*     scheduled.
*
*  Input(s):    event - the event to be posted (MUST be a name from the "HMI_EVENT_TYPE" enumeration).
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void HMISS_PostHMIEvent(HMI_EVENT_TYPE event);

int8_t AltiaInit(void);
#endif
/* End of file */
