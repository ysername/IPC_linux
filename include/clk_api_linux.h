/**
	@file clk_api_linux.h
	@version 1.0
	@date 7/8/2018
	@author Leonardo Reatiga Miranda
	@brief 
		This is an API intended to set timers using signals. This file,
		among others in this project depends on the signals_definitions.h
		file to work properly.
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h> 
#include <errno.h>
#include <string.h>
#include "gp_types.h"
#include "signal_definitions.h"
/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 
#define MSEC_15  15u
#define MSEC_30  30u
typedef unsigned long uint64_t ;
/**
  	@brief Clk_SetAlarm Sets a one-time alarm on a timer referenced by timerid
						to go off at the thisTime absolute time.
	@param [in] timerid  - 	pointer to the id of the timer
	@param [in] thisTime - 	absolute time in msec
	@param [in] signal 	 - 	the signal that will be used to notify to the 
							process the timer has expired.		
	@return 	Returns 0 if OK, non-0 if error.
*/
gp_retcode_t Clk_SetAlarm(timer_t * timerid, uint64_t thisTime, int signal);

/**
  	@brief  Clk_SetTimer Sets an alarm on a timer referenced by timerid go off 
						once every thisInterval with/without repeating.

	@param [in]	timerid -	pointer to the id of the timer
	@param [in]	repeat  -	flag whether this alarm should repeat or not
	@param [in] thisInterval - interval time in msec
	@param [in] signal 	-	the signal that will be used to notify to the 
							process	the timer has expired.

  	@return Returns 0 if OK, non-0 if error.
*/
gp_retcode_t Clk_SetTimer(timer_t * timerid, Boolean repeat, uint64_t thisInterval, int signal);

/**
 	@brief Clk_SetAlarmAndTimer Sets an alarm on the timer referenced by timerid 
								to go off every thisInterval starting at the 
								absolute time of thisTime.

  	@param [in]	timerid	 -	pointer to the id of the timer	
 	@param [in] thisTime -	absolute time in msec
	@param [in] thisInterval - interval time in msec
	@param [in] signal 	-	the signal that will be used to notify to the 
							process	the timer has expired.

  	@retun Returns 0 if OK, non-0 if error.
*/
gp_retcode_t Clk_SetAlarmAndTimer(timer_t * timerid, uint64_t thisTime, uint64_t thisInterval, int signal);

/**
	@brief Clk_ClearAlarmAndTimer Clears thisClock's alarm.

	@param [in] timerid - the id of the timer you wish to clear

	@return Returns 0 if OK, non-0 if error.
*/
gp_retcode_t Clk_ClearAlarmAndTimer(timer_t * timerid);

/**
  	@brief Clk_GetDeltaTime Calculates time elapsed in usec from start_time to 
							end_time.
	@param [in]	start_time	-	start time pointer Time structure
	@param [in] end_time 	-	end time pointer Time structure
	@param [in] delta_usec	-	pointer to the in wich the elapsed time
								will be stored.

  	@return Nothing.
*/
void Clk_GetDeltaTime(struct timeval * start_time, struct timeval* end_time, int64_t * delta_usec);
