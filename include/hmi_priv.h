/*******************************************************************************
*  $Log: hmi_priv.h  $
*  Revision 1.1 2015/03/31 17:09:20CST Daniel Kageff (10011932) 
*  Initial revision
*  Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/HMI/HMILink/project.pj
*  Revision 1.4 2014/06/03 12:39:26CDT Edgar Rios Lucas (10032297) 
*  Releasing HMI 1.0
*  Release notes and change analysis pending
*  Revision 1.3 2014/06/02 08:29:37CDT Kevin Russo (kevinadm) 
*  Member moved from hmi_priv.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/Application/Include/project.pj to hmi_priv.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/HMI/Link/project.pj.
*  Revision 1.2 2014/05/31 13:09:50EDT Kevin Russo (10032877) 
*  compile fixes
*  Revision 1.1 2014/04/17 13:10:33EDT Gregory Palarski (10030324) 
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
*  File:  hmi_priv.h 
*
*  Description: This file contains interaces private to the HMI subsystem.
********************************************************************************************/
#ifndef HMI_PRIV_H   
#define HMI_PRIV_H   

/*******************************************************************************************/
/*    I N C L U D E   F I L E S                                                            */
/*******************************************************************************************/
#include "hmi_ss.h"


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
*  Function Name:  Initialize_HMIModel
*
*  Description:  General HMI model intialization function.
*
*  Input(s):    None. 
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void Initialize_HMIModel(void);

/********************************************************************************************
*  Function Name: ProcessEvents
*
*  Description: This function processes events that have been queued to the HMI by delivering
*     them to the HMI model for processing.
*
*  Input(s):    None. 
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void ProcessEvents(void);

/********************************************************************************************
*  Function Name: QueueHMIEvent
*
*  Description: Places an event in the HMI event queue.
*
*  Input(s):    event - the event to be queued (MUST be a name from the "HMI_EVENT_TYPE" enumeration).
*
*  Outputs(s):  None.
*
*  Returns:     None.
********************************************************************************************/
void QueueHMIEvent(HMI_EVENT_TYPE event);

/********************************************************************************************
*  Function Name: DeQueueHMIEvent
*
*  Description: Retrieves and returns the next event to be processed from the HMI event queue. 
*
*  Input(s):    None. 
*
*  Outputs(s):  event - will contain the next event to be processed if return is non-zero.
*
*  Returns:     0 = No events to return (queue empty)
*               1 = Event returned in the "event" parameter
********************************************************************************************/
unsigned int DeQueueHMIEvent(HMI_EVENT_TYPE *event);

/********************************************************************************************
*  Function Name: HMISS_RequestPartialInit
*
*  Description: Requests partial initialization of HMI subsystem. 
*    
*  Input(s):    None.  
*
*  Outputs(s):  None.
*
*  Returns:     None
*
*  Change History:
*       05/30/2014     First version (Greg Palarski)
********************************************************************************************/
void HMISS_RequestPartialInit(void);

#endif
/* End of file */
