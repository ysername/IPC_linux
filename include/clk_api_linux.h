#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h> 
#include <errno.h>
#include <string.h>
//#include <sys/time.h>
#include "gp_types.h"
#include "signal_definitions.h"
/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 
//#define USEC_PER_SEC  1000000u
#define MSEC_15  15u
#define MSEC_30  30u
typedef unsigned long uint64_t ;
/******************************************************************************
*  Function Name: Clk_SetAlarm
*
*  Description: Sets a one-time alarm on thisClock to go off at the thisTime 
*               absolute time.
*
*  Input(s):    thisClock - INTEGRITY Clock object
*               thisTime - absolute time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetAlarm(timer_t * timerid, uint64_t thisTime, int signal);

/******************************************************************************
*  Function Name: Clk_SetTimer
*
*  Description: Sets an alarm on thisClock to go off once every thisInterval
*               without/without repeating.
*
*  Input(s):    thisClock - INTEGRITY Clock object
*               repeat - flag whether this alarm should repeat
*               thisInterval - interval time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetTimer(timer_t * timerid, Boolean repeat, uint64_t thisInterval, int signal);

/******************************************************************************
*  Function Name: Clk_SetAlarmAndTimer
*
*  Description: Sets an alarm on thisClock to go off every thisInterval starting
*               at the absolute time of thisTime.
*
*  Input(s):    thisClock - INTEGRITY Clock object
*               repeat - flag whether this alarm should repeat
*               thisTime - absolute time in msec
*               thisInterval - interval time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetAlarmAndTimer(timer_t * timerid, uint64_t thisTime, uint64_t thisInterval, int signal);

/******************************************************************************
*  Function Name: Clk_ClearAlarmAndTimer
*
*  Description: Clears thisClock's alarm.
*
*  Input(s):    thisClock - INTEGRITY Clock object
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_ClearAlarmAndTimer(timer_t * timerid);

/******************************************************************************
*  Function Name: Clk_GetDeltaTime
*
*  Description: Calculates time elapsed in usec from start_time to end_time.
*
*  Input(s):    start_time - start time Time structure
*               end_time - end time Time structure
*
*  Outputs(s):  delta_usec - time elapsed in usec
*
*  Returns:     None.
******************************************************************************/
void Clk_GetDeltaTime(struct timeval * start_time, struct timeval* end_time, int64_t * delta_usec);
