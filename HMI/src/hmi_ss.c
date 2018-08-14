/*******************************************************************************
*  $Log: hmi_ss.c  $
*  Revision 1.1 2016/10/05 14:16:32CDT Daniel Kageff (10011932) 
*  Initial revision
*  Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/HMI/HMILink/project.pj
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
*  File:  hmi_ss.c
*
*  Description: This file contains the implementation for the HMI Interface Layer of the HMI
*     subsystem component. 
*
********************************************************************************************/
#define HMI_SS_C  

/*******************************************************************************************/
/*    I N C L U D E   F I L E S                                                            */
/*******************************************************************************************/
#include "hmi_ss.h"
#include "hmi_priv.h"
#include <string.h>
#include <limits.h>
//#include "Dis_Mgr.h"
//#include "altia.h"

/*******************************************************************************************/
/*    M A C R O S                                                                          */
/*******************************************************************************************/

/*
** Configuration parameter checks.
*/
#if (FRAME_RATE_INSTANT_SAMPLES > 1000)
   #error FRAME_RATE_INSTANT_SAMPLES is too large.
#endif
#if (FRAME_RATE_INSTANT_SAMPLES < 1)
   #error FRAME_RATE_INSTANT_SAMPLES is too small.
#endif

/*
** MICROSEC_PER_MILLISEC - The number of microseconds per millisecond.
*/
#define  MICROSEC_PER_MILLISEC    ((unsigned int) 1000)  

/*
** SZ_EVENT_Q - size of the the HMI event queue.
*/
#define    SZ_EVENT_Q       (20)

/*
** MICROSEC_PER_SEC - The number of microseconds per second
*/
#define  MICROSEC_PER_SEC    ((unsigned int) 1000000)  


/*******************************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                                          */
/*******************************************************************************************/


/*
** HMI_EVENT_QUEUE - structure for holding HMI event queue information
**    q - the queue where events are stored
**    next_in - the position where the next incoming event will be stored 
**    next_out - the position where the next event to be processed is located
**    count - the number of events in the queue 
**    overflow_count - the number of times an incoming event was discarded because the
**       queue was full (for diagnostics).
**    max_count - largest number of events held in the queue simultaneously (for diagnostics).
*/
typedef struct
{
   HMI_EVENT_TYPE  q[ SZ_EVENT_Q ];
   unsigned int next_in;
   unsigned int next_out;
   unsigned int count;
   unsigned int overflow_count;
   unsigned int max_count;
} HMI_EVENT_QUEUE;

/*
** FRAME_RATE_INFO - Structure for holding frame rate information 
**
**    instant_next_in - position of the oldest data in the "instant_et_us" array
**    instant_samples - nummber of samples held in the "instant_et_us" array
**    instant_total_et_us - total et of all instantaneous samples
**    instant_total_us - array hold the last N execution times samples 
**    tot_samples - total number of execution time samples (for the avg frame rate)
**    tot_et_us - total execution time (for the avg frame rate)
*/
typedef struct 
{
  unsigned int       instant_next_in; 
  unsigned int       instant_samples;
  unsigned int       instant_total_et_us;
  unsigned int       instant_et_us[ FRAME_RATE_INSTANT_SAMPLES ];
  unsigned long      total_samples;
  unsigned long long total_et_us;
} FRAME_RATE_INFO;

/*
** INIT_TYPE - enumeration are used to differentiate types of initialization.  A partial initialization
** is done if an error is encountered during normal operation and the hardware needs to be initialized but
** not the HMI (so the HMI state machine retains context and re-displays the last active screen).
**    INIT_REQUEST_NONE - No initialization is necessary (already completed)
**    INIT_REQUEST_FULL - A full initialization needs to be done
**    INIT_REQUEST_PARTIAL - A partial initialization needs to be done (initialize everything except HMI model).
*/
typedef enum 
{
   INIT_REQUEST_NONE=0,
   INIT_REQUEST_FULL,   
   INIT_REQUEST_PARTIAL, 
}INIT_REQUEST;

/*******************************************************************************************/
/*    F U N C T I O N   P R O T O T Y P E S                                                */
/*******************************************************************************************/
static void HMI_Initialize(void);
char *EventToString(int event);

static void InitHMIEventQ(void);

#if (HMI_ENABLE_FRAME_RATE_SUPPORT != 0)
   static void UpdateFrameRateInfo(CHRONO_BUFF *task_et);
#endif


/*******************************************************************************************/
/*    M E M O R Y   A L L O C A T I O N                                                    */
/*******************************************************************************************/


/*
** init_request - indicates if initialization is necessary, and if so, what type (full or partial).  
**   Initialization is necessary following micro reset, disconnect of the Yamaha ROM Writer after it has been connected,
**   and for recovery following an error condition. 
*/
static INIT_REQUEST init_request = INIT_REQUEST_FULL;

/*
** event_q - stores incoming HMI events and related information.
*/
static HMI_EVENT_QUEUE event_q;

/*static AtConnectId sg_altiaConnectionId;*///leo

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
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
void HMISS_StartupInit(void)
{
   /*
   ** Set init flag to indicate FULL initialization is needed, ROM writer "not connected", initialize
   ** event queue, and frame rates
   */
   init_request = INIT_REQUEST_FULL;
   /*AltiaInit();
   InitHMIEventQ();
   u1g_Dis_Init();*///TODO: uncomment this
}

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
*
*  Change History:
*       03/26/2014     First version (Greg Palarski)
********************************************************************************************/
void HMISS_Sleep(void)
{

}

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
*
*  Change History:
*       04/03/2014     First version (Greg Palarski)
********************************************************************************************/
void HMISS_Wakeup(void)
{
   /*
   ** We want the HMI state to be maintained across sleep/wakeup.  The VC1N must be reset and 
   ** re-initialized  since it loses power when the switched supplies go off.  In order maintain state
   ** we need to call an Altia function that re-writes palette table and the registers calle by altia.
   ** All the above is accomplished by doing a partial initialization. 
   */
   init_request = INIT_REQUEST_PARTIAL;
   InitHMIEventQ();   
}

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
void HMISS_PostHMIEvent(HMI_EVENT_TYPE event)
{
   if (event < MAX_EVENT_VALUE)
   {
      QueueHMIEvent(event);
   }
}

/********************************************************************************************
*  Function Name: HMISS_PeriodicTask 
*
*  Description: Periodic task for the HMI SS component.  The HMI state machine is executed from 
*    this task followed by updating the pixel content on the 4.2" TFT.  
*    NOTE: This functin must be called at the rate defined in the HMI_SCHEDULING_RATE_MS macro.
*
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     None.
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
void HMISS_PeriodicTask(void)
{

  /*
  ** Do initialization if necessary.
  */
  if (init_request != INIT_REQUEST_NONE)
  {
     HMI_Initialize();
  }
  
  if (init_request == INIT_REQUEST_NONE) 
  {
    /* u1g_Dis_Task();
     QueueHMIEvent(SCREEN_REDRAW);
     ProcessEvents(); TODO: uncoment this*/
 }   
   
}

/********************************************************************************************
*  Function Name: HMI_Initialize  
*             
*  Description: Initializes the VC1N and Yamaha middleware layer.
*
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     None.
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
static void HMI_Initialize(void)
{


  if (init_request == INIT_REQUEST_FULL)
  {
    // Initialize_HMIModel();TODO: uncomment this
  }
  

  /* 
  ** Initialization was successful so clear the initialization request flag and complete
  ** initialization.
  */
  init_request = INIT_REQUEST_NONE;

}


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
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
static void  InitHMIEventQ(void)
{

  
   /*
   ** Begin critical section
   */


   event_q.count = 0;
   event_q.next_in = 0;
   event_q.next_out = 0;
   event_q.overflow_count = 0;
   event_q.max_count = 0;

   /*
   ** End critical section
   */
}

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
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
void QueueHMIEvent(HMI_EVENT_TYPE event)
{

  
   /*
   ** Begin critical section
   */

   if ( event == SEM_RESET )
   {
       event_q.count = 0;
       event_q.next_in = 0;
       event_q.next_out = 0;
       event_q.overflow_count = 0;
       event_q.max_count = 0;
   }
   if (event_q.count < SZ_EVENT_Q)
   {
      event_q.q[ event_q.next_in ] = event;
      event_q.next_in = (event_q.next_in + 1) % SZ_EVENT_Q;
      ++event_q.count;
      if (event_q.count > event_q.max_count)
      {
          event_q.max_count = event_q.count;
      }
   }
   else
   {
      ++event_q.overflow_count;
   }

   /*
   ** End critical section
   */
}

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
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
unsigned int DeQueueHMIEvent(HMI_EVENT_TYPE *event)
{
   unsigned int rc = 0;

  
   /*
   ** Begin critical section
   */


   if (event_q.count != 0)
   {
      *event = event_q.q[ event_q.next_out ];
      event_q.next_out = (event_q.next_out + 1) % SZ_EVENT_Q;
      --event_q.count;
      rc = 1;
   }

   /*
   ** End critical section
   */

#if (0)
   /* For test/debug only */
   if ((rc == 1) && (*event != SCREEN_REDRAW))
   {
      printf("HMIEvent: %s\r\n", EventToString(*event));
   }
#endif
   return(rc);
}

/********************************************************************************************
*  Function Name: EventToString
*
*  Description: Translates an event value to corresponding string.
*
*  Input(s):    event - event to translate. 
*
*  Outputs(s):  None.
*
*  Returns:     char * - pointer to the corresponding string. 
*
*  Change History:
*        3/06/2014     First version (Greg Palarski)
********************************************************************************************/
char *EventToString(int event)
{
   char *event_string;

   switch (event)
   {
      default:
         event_string = "UNKNOWN";
      break;
   }
   return(event_string);
}



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
void HMISS_RequestPartialInit(void)
{
   init_request = INIT_REQUEST_PARTIAL;
}



int8_t AltiaInit(void)
{
   /* int8_t retVal = 0;
     sg_altiaConnectionId = AtOpenConnection(NULL, NULL, 0, NULL);
     if (sg_altiaConnectionId < 0)
    {
        retVal = -1;
    }
    else if (altiaCacheOutput(1) < 0)
    {
        retVal = -2;
    }
    return (retVal);*/return 0;
}

/* End of file */
