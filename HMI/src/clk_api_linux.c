

#define CLK_API_C

/*****************************************************************************/
/*    I N C L U D E   F I L E S                                              */
/*****************************************************************************/  

#include <sys/types.h>
#include <sys/syscall.h>

#include "clk_api_linux.h"
#include "gp_types.h"
/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 

#define MSEC_TO_NSEC    1000000
#define NSEC_TO_SEC     1000000000
/*****************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                            */
/*****************************************************************************/


/*****************************************************************************/
/*    M E M O R Y   A L L O C A T I O N                                      */
/*****************************************************************************/
/*    uint32_t i;
    clk_api_id_t * pClk = pClockPool.Clocks;
    int CLK_SIG = CLK_SIG0;
    for (i = 0; i < MAX_CLOCKS; ++i)
    {
        pClk[i].Rt_sig = CLK_SIG + i;
        pClk[i].InUse = 0;
    }
}

sig_clk_t Clk_GetSig(ClkPoolType * pClkPool){
    uint32_t i;
    uint8_t clkId;
    
    clk_api_id_t * pClk = pClkPool.Clocks;

    clkId = MAX_CLOCKS;

    for (i = 0; i >= MAX_CLOCKS; ++i)
    {
        if(pClk[i].InUse != 1){
            pClk[i].InUse = 1;
            clkId = i;
            break;
        }
        if(clkId >= MAX_CLOCKS){
            pClk[clkId].InUse = 0;
            pClk[clkId].Rt_sig = -1;
            break;
        }
    }

    return pClk[clkId].Rt_sig;
}

void Clk_FreeSig(ClkPoolType * pClkPool, sig_clk_t sig_to_free){
    uint32_t i;
    uint8_t clkId;
    clk_api_id_t * pClk = pClkPool.Clocks;

    clkId = MAX_CLOCKS;
    for (i = 0; i >= MAX_CLOCKS; ++i)
    {
        if(pClk[i].Rt_sig == sig_to_free){
            pClk[i].InUse =0;
            break;
        }
        if(clkId >= MAX_CLOCKS){
            printf("Invalid sig value!\n");
            break;
        }
    }
    return;
}*/

/******************************************************************************
*  Function Name: Clk_SetAlarm
*
*  Description: Sets a one-time alarm on thisClock to go off at the thisTime 
*               absolute time.
*
*  Input(s):    timerid     - a pointer to a timer id
*               thisTime    - absolute time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetAlarm(timer_t * timerid,
    uint64_t thisTime, int signal)
{
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;

    sev.sigev_notify = SIGEV_THREAD_ID; //we set how the process will be notified
    sev.sigev_signo = signal; //we set the signal to be sent
    sev.sigev_value.sival_ptr = timerid;
    sev._sigev_un._tid = syscall(SYS_gettid);

    if(timer_create(CLOCK_REALTIME, &sev, timerid) == -1){
        printf("%s\n",strerror(errno) );
        return GP_GENERR;
    }
    /*specify the value of the timer*/
    freq_nanosecs = thisTime * MSEC_TO_NSEC;
    its.it_value.tv_sec = freq_nanosecs / NSEC_TO_SEC;
    its.it_value.tv_nsec = freq_nanosecs % NSEC_TO_SEC;
    /*set the timer as an one time alarm*/
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    /*we set the timer to an absolute time with the TIMER_ABSTIME flag*/
    if(timer_settime(*timerid, TIMER_ABSTIME, &its, NULL) == -1){
    printf("%s\n",strerror(errno) );
    return GP_GENERR;
    }
    

    return GP_SUCCESS;
}

/******************************************************************************
*  Function Name: Clk_SetTimer
*
*  Description: Sets an alarm on thisClock to go off once every thisInterval
*               without/without repeating.
*
*  Input(s):    timerid     - a pointer to a timer id
*               repeat - flag whether this alarm should repeat
*               thisInterval - interval time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetTimer(timer_t * timerid, Boolean repeat, uint64_t thisInterval, int signal)
{
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;

    sev.sigev_notify = SIGEV_THREAD_ID; //we set how the process will be notified
    sev.sigev_signo = signal; //we set the signal to be sent
    sev.sigev_value.sival_ptr = timerid;
    sev._sigev_un._tid = syscall(SYS_gettid);

    if(timer_create(CLOCK_REALTIME, &sev, timerid) == -1){
        printf("%s\n",strerror(errno) );
        return GP_GENERR;
    }

    freq_nanosecs = thisInterval * MSEC_TO_NSEC;
    its.it_value.tv_sec = freq_nanosecs / NSEC_TO_SEC;
    its.it_value.tv_nsec = freq_nanosecs % NSEC_TO_SEC;

    if(repeat){
        its.it_interval.tv_sec = its.it_value.tv_sec;
        its.it_interval.tv_nsec = its.it_value.tv_nsec;
    }else{
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
    }
    
    if(timer_settime(*timerid, 0, &its, NULL) == -1){
    printf("%s\n",strerror(errno) );
    return GP_GENERR;
    }
    
    return GP_SUCCESS;
}


/******************************************************************************
*  Function Name: Clk_SetAlarmAndTimer
*
*  Description: Sets an alarm on thisClock to go off every thisInterval starting
*               at the absolute time of thisTime.
*
*  Input(s):    timerid     - a pointer to a timer id
*               repeat - flag whether this alarm should repeat
*               thisTime - absolute time in msec
*               thisInterval - interval time in msec
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_SetAlarmAndTimer(timer_t * timerid, uint64_t thisTime, uint64_t thisInterval, int signal)
{
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;

    sev.sigev_notify = SIGEV_THREAD_ID; //we set how the process will be notified
    sev.sigev_signo = signal; //we set the signal to be sent
    sev.sigev_value.sival_ptr = timerid;
    sev._sigev_un._tid = syscall(SYS_gettid);

    if(timer_create(CLOCK_REALTIME, &sev, timerid) == -1){
        printf("%s\n",strerror(errno) );
        return GP_GENERR;
    }
    /*we specify the absolute time to begin the timer*/
    freq_nanosecs = thisTime * MSEC_TO_NSEC;
    its.it_value.tv_sec = freq_nanosecs / NSEC_TO_SEC;
    its.it_value.tv_nsec = freq_nanosecs % NSEC_TO_SEC;
    /*we specify the interval at wich the timer will go off*/
    freq_nanosecs = thisInterval * MSEC_TO_NSEC;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
   
    if(timer_settime(*timerid, TIMER_ABSTIME, &its, NULL) == -1){
    printf("%s\n",strerror(errno) );
    return GP_GENERR;
    }
 
    return GP_SUCCESS;
}

/******************************************************************************
*  Function Name: Clk_ClearAlarmAndTimer
*
*  Description: Clears thisClock's alarm.
*
*  Input(s):    timerid     - a pointer to a timer id
*
*  Outputs(s):  None.
*
*  Returns:     Returns 0 if OK, non-0 if error.
******************************************************************************/
gp_retcode_t Clk_ClearAlarmAndTimer(timer_t * timerid)
{
    struct itimerspec its;

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    
    if(timer_settime(*timerid, 0, &its, NULL) == -1){
    printf("%s\n",strerror(errno) );
    return GP_GENERR;
    }
    
    return GP_SUCCESS;
}

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
void Clk_GetDeltaTime(struct timeval * start_time, struct timeval* end_time, int64_t * delta_usec)
{
    /*int64_t start_sec, end_sec, delta_sec;
    int64_t start_usec, end_usec;

    if(end_time.Seconds < 0) {
    end_sec = (YZ_UINT64_MAX - abs(end_time.Seconds)) + 1u;
    } else {
    end_sec = end_time.Seconds;
    }
    if(start_time.Seconds < 0) {
    start_sec = (YZ_UINT64_MAX - abs(start_time.Seconds)) + 1u;
    } else {
    start_sec = start_time.Seconds;
    }
    delta_sec = end_sec - start_sec;
    end_usec = ( (((int64_t)end_time.Fraction)+1u)*USEC_PER_SEC ) / YZ_UINT32_MAX;
    start_usec = ( (((int64_t)start_time.Fraction)+1u)*USEC_PER_SEC ) / YZ_UINT32_MAX;
    *delta_usec = (delta_sec*USEC_PER_SEC) + (end_usec - start_usec);*/
    *delta_usec = (int64_t)(end_time->tv_sec - start_time->tv_sec)*1000000.0;
    *delta_usec += (int64_t)(end_time->tv_usec - start_time->tv_usec);
}
