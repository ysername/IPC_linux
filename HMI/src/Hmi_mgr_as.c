/**************************************************************************************/
/*!
 *  \file		Hmi_mgr_as.c
 *
 *  \copyright	Yazaki 2016
 *
 *  \brief		Top level FF HUD application HMI manager functions.  This software component
 *				initializes and starts the HMI.NOTE: Be aware this file among 
 *              others in this project had been modified in order to be able 
 *              to run in the linux environment and to be able to produce 
 *              an application that would not crash despite there are several
 *              components not ported yet.
 *
 *  \author		E. Gunarta, D. Kageff modified by Leonardo Reatiga Miranda
 *
 *  \version	$Revision: 1.29 $  
 *				$Log: Hmi_mgr_as.c  $
 *				Revision 1.29 2018/02/13 11:02:15CST Daniel Kageff (10011932) 
 *				1) Added HMI development mode support
 *				2) Added startup timing instrumentation code
 *				Revision 1.28 2017/10/23 17:08:16EDT Daniel Kageff (10011932) 
 *				Updated conditional code statements for builds using the HMI_STRESS_TEST #define
 *				Revision 1.27 2017/10/13 11:01:40EDT Daniel Kageff (10011932) 
 *				Added conditional compile to force HMI to start if either ENABLE_HMI_STARTANIM or HMI_STRESS_TEST are defined.
 *				Revision 1.26 2017/09/21 14:54:42EDT Daniel Kageff (10011932) 
 *				WORK IN PROGRESS, changes to support bootloader <-> app graphics 'handoff'
 *				Revision 1.25 2017/09/05 17:59:05EDT Daniel Kageff (10011932) 
 *				Added support for 'bootloader animation done' flag.
 *				Revision 1.24 2017/08/25 14:32:32EDT Daniel Kageff (10011932) 
 *				1) Added SendAppStatusMsg function
 *				2) Formatting changes
 *				Revision 1.23 2017/06/20 11:15:57EDT Daniel Kageff (10011932) 
 *				In main(), changed APP_DIR to HMI_BASE_DIR (defined in gp_cfg.h)
 *				Revision 1.22 2017/05/10 10:05:15EDT Eudora Gunarta (10031829) 
 *				Moved time constants to clk_api.h, adjusted scheduling of next alarm to be closer to 30msec.
 *				Revision 1.21 2017/05/09 10:41:40EDT Eudora Gunarta (10031829) 
 *				Changes for measuring HMI task execution time.
 *				Revision 1.20 2017/03/31 14:02:14EDT Eudora Gunarta (10031829) 
 *				Changes for GP reset request.
 *				Revision 1.19 2017/03/17 12:59:17EDT Eudora Gunarta (10031829) 
 *				Changed all transfers with with other tasks to be buffer type.
 *				Replaced printf() with gp_printf().
 *				Revision 1.18 2017/02/17 17:08:56EST Daniel Kageff (10011932) 
 *				1) Fixed timeout for calls to Msg_SyncTxBuf()
 *				2) Reformatted some code
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	This optional comment section can be used to create an overview page for this 
 *	software component that is accessable from the navigation pane of the gnerated 
 *	design document.
 *
 *  TODO LIST:
 */
/***************************************************************************************/
#define _HMI_MGR_AS_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>			// Required for chdir()
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include "gp_cfg.h"         // Common GP program configuration settings
#include "gp_types.h"       // Common GP program data type definitions
#include "gp_utils.h"       // Common GP program utility functions


#include "msg_buf.h"
#include "msg_api_signals.h"
#include "msg_def.h"
#include "msg_fcn.h"

#include "Datapool.h"
#include "cmd_conn_L.h"

//#include "altiaUserMain.h"	// Altia HMI functions
#include "hmi_ss.h"
//#include "Vivante.h"			// HMI stress test functions
//#include "HMI_versions.h"		// HMI program verion information
#include "gp_types.h"
#include "clk_api_linux.h"		// Standard clock functions

#ifdef HMI_EN_CHRONO
#include "chrono.h"
#endif
#include "Hmi_mgr_as_worktask1.h"
#ifndef _IDENTIFICATION_DATA_H_
#include "identification_data.h"
#endif

/***********************************
	Private Macros and Typedefs
***********************************/
typedef struct {
    uint32_t BufSz;  /*!< Size (in bytes) of data buffer */
    uint32_t BufId; /*!< Index of data buffer */
    uint8_t * Buf;   /*!< Pointer to data buffer */
} BufInfoType;

static uint8_t  component = HMI_MGR_AS;
/***********************************
	               Config Macros
***********************************/
#ifndef HMI_STRESS_TEST
 #define TX_APPSTAT_RETRIES	100		/*!< Number of app status tx retries for normal builds */
#else
 #define TX_APPSTAT_RETRIES 1		/*!< Number of app status tx retries for stress test builds */
#endif

/*!< List of TskInfoType entries */
enum {
    BUF_UM_1 = 0,
    BUFINFO_NUM_ENTRIES
};

/*!< Size of buffer used for receiving messages on Hm_UmAsCon1 connection */
#define UMASCON1_BUF_SZ 	8		/*!< Buffer size for messages received from the main app Unit Mgr */

#define GET_CONN_WAIT_TIME_SEC	1		/*!< Number of seconds to wait between attempts to get a connection 
											 obejct from the Resource Manager */

#define NUM_RSC_RETRIES		10		/*!< Number of times to try and get a resource from the resource manager
										 before giving up. */
#define MAX_WAIT_CNT	200			/*!< Countdown value for HMI video start timeout. */

/***********************************
	Private Data and Structures
***********************************/
/* INTEGRITY Connection messages */
static BufInfoType TskBufInfo[BUFINFO_NUM_ENTRIES];

/*! Connection to the Unit Manager */
//Connection HmAs_UmAsCon1;

/*! Connection to the IPC Manager */
//Connection HmAs_ImAsCon1;

uint8_t HmiSleepReq = 0;
uint8_t HmiSleepReady = 0;
#ifndef Success
#define Success 0
#endif

#ifdef HMI_EN_CHRONO
CHRONO_BUFF cm_hmi_periodic_task;
#endif

///* HMI startup task data definitions */
//static bool ok_to_start = false;			/*!< If TRUE, then start the HMI. Set in IntTsk_UmasRxHandler()
//												 and checked in StartHMI(). */
static bool hmi_started = false;			/*!< Set 'true' if HMI has been started */


/***********************************
	Private Function Prototypes
***********************************/
void IntTsk_HmiAlarmHandler(int sig, siginfo_t *si, void *uc);

//static int32_t SendVideoReadyMsg(uint8_t data, int retries);
//static int32_t ProcHbtReq(uint8_t * data, uint32_t size);
static int32_t SendAppStatusMsg(uint8_t status, int retries);
static gp_retcode_t StartHMI(void);
void MsgRxHandler(int sig, siginfo_t *si, void *uc);
/**************** HMI DEVELOPMENT MODE CODE ******************/
#ifdef ENABLE_HMI_DEVMODE
 #include "HMI_common.h"				// Global HMI definitions
 #include "TestPatternLink.h"
 #define MODE_CHG_CNT  20
 #define MODE_UPD_MSEC  2000
 static int hmi_dev_chkctr = 50;		/*!< Number of times to check for the HMI dev mode */
 static int hmi_dev_mode = -1;			/*!< HMI dev mode state: 0 = off, 1 == on, -1 = uninitialized */
 static uint32_t hdev_ctrls = 0;		/*!< HMI dev mode user play controls */
 static int hdev_ctr = 0;
 static void CheckForHmiDevMode(void);
 static void ProcHmiDevControls(void);
 static void GetHmiDevControls(void);
 static void SimHmiDevControls(void);		/* HMI dev mod controls simulation */
#endif
/**************** HMI DEVELOPMENT MODE CODE ******************/


/************ Start of code ******************/

/**************************************************************************************/
/*! \fn main()
 *
 *	param - No parameters
 *
 *  \par Description:	  
 *   Main entry point of the HMI Manager component.  
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static timer_t hmClk;

int main()
{
    gp_retcode_t rc;
    int32_t ret;
    component_info_t tmpCom[BUFINFO_NUM_ENTRIES];

#ifdef EN_STARTUP_INSTR
	gp_Printf(VRB_DEBUG1, "$$$ HMI_MGR @MAIN start: %lld msec\n", Clk_GetCurrTimeVal(DEFAULT_CLOCK, CLK_MSEC));
#endif

	hmi_started = false;

    gp_Printf(VRB_DEBUG1, "\n************************************************\n");
   // gp_Printf(VRB_DEBUG1, " Yazaki FF HUD HMI Application v %d.%d\n", HMI_MAIN_VER, HMI_MAIN_REV);TODO: add include files
    gp_Printf(VRB_DEBUG1, " Build date: %s\n", __DATE__ );
    gp_Printf(VRB_DEBUG1, " Includes Altia Deepscreen support\n");
    gp_Printf(VRB_DEBUG1, "************************************************\n");
#ifdef DEBUG    
    gp_Printf(VRB_DEBUG1, "\nHMAS_INTTSK: Started\n");
#endif /* DEBUG */

    /* Init data pool */
    do 
    {
		rc = InitPool();
		if(rc != GP_SUCCESS) 
		{
		    gp_Printf(VRB_RUNTIME, "\nHMAS_INTTSK: InitPool() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);
    printf("passed InitPool\n");
    /* Init msg bufs */
    do 
    {
		rc = Msg_InitBufs(MAIN_COMPONENT);
		if(rc != GP_SUCCESS) 
		{
		    gp_Printf(VRB_RUNTIME, "\nHMAS_INTTSK: Msg_InitBufs() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);
    printf("passed init bufs\n");
    /* Init msg api */

 do 
    {
        sigset_t mask;
        struct sigaction sa;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = IntTsk_HmiAlarmHandler;
        sigemptyset(&sa.sa_mask);
        rc = sigaction(CLKSETALARM1, &sa, NULL);

        if(rc != 0) 
        {
            gp_Printf(VRB_RUNTIME, "\nHMAS_INTTSK: Msg_AsyncRxEvent(alarm) error %d\n", rc);
        }
    } while(rc != 0);

    do 
    {
        rc = Clk_SetTimer(&hmClk, false, MSEC_30,CLKSETALARM1);
        if(rc != GP_SUCCESS) 
        {
            gp_Printf(VRB_RUNTIME, "\nHMAS_INTTSK: Clk_SetTimer() error %d\n", rc);
        }
    } while(rc != GP_SUCCESS);
    printf("wait for 10 seconds...\n");
    sleep(10);

    //Start DataTsk AFTER init data pool, msg bufs and msg api //TODO: add thread fucnionality
  	pthread_t tid;
   	pthread_attr_t attr;

   	pthread_attr_init(&attr);
   	pthread_create(&tid, &attr, Hm_DplTskMain, NULL);

	/* Send application status 'running' message to the Unit Manager */
	gp_Printf(VRB_DEBUG1, "HMI main: Sending HMI running status\n");
	/*if(SendAppStatusMsg(APP_STAT_RUNNING, TX_APPSTAT_RETRIES) != 0)
	{
		gp_Printf(VRB_RUNTIME, "HMI main: Failed sending app status\n");
	}*/
	

	/* Start HMI if starting animation or graphics stress test is enabled ****/
#if (ENABLE_HMI_STARTANIM || HMI_STRESS_TEST)
 #warning "** Hmi_mgr_as: Building HMI animation or stress test version **"
	StartHMI();
#endif
	/*********************************************************************/

    /* Start HMI alarm */
    
   
   // WaitSemaphore(dp_semaphore);
    
        
#ifdef HMI_EN_CHRONO
    ChronoInitBuff(&cm_hmi_periodic_task);
#endif

#ifdef EN_STARTUP_INSTR
	gp_Printf(VRB_DEBUG1, "$$$ HMI_MGR @MAIN end: %lld msec\n", Clk_GetCurrTimeVal(DEFAULT_CLOCK, CLK_MSEC));
#endif

    /* Main HMI manager loop */
    while(1);
}

void MsgRxHandler(int sig, siginfo_t *si, void *uc){

    printf("this should not happpen!\n"); 
}

/**************************************************************************************/
/*! \fn IntTsk_HmiAlarmHandler(uint32_t data, uint32_t size)
 *
 *	param[in] data	- ignore since this is an event
 *	param[in] size	- ignore since this is an event
 *
 *  \par Description:	  
 *   Handle HMI screen update timer.  
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void IntTsk_HmiAlarmHandler(int sig, siginfo_t *si, void *uc)
{
    printf("enter signal handler!\n");
    gp_retcode_t rc;
    int32_t ret;
    struct timeval start_time;
    struct timeval end_time;
    int64_t delta_usec, delta_msec, next_msec;
    static uint32_t counter = 0;
    static uint32_t counter_1000ms = 0;

    gettimeofday(&start_time,NULL);
    counter++;

    if(counter >= 3) 
    {
		// Re-send video ready signal in case IPC failed to service this request 
		//ret = SendVideoReadyMsg(1, 10); ignore since this is a connection to IPC
        ret = 0;
		if(ret != 0) 
		{
		    gp_Printf(VRB_DEBUG1, "\nHMAS_INTTSK: SendDataToImAs() error %d\n", ret);
		}
		counter = 0;
    }

	// Run HMI task if the HMI has been started 
	if(hmi_started == true)
	{
    	HMISS_PeriodicTask();

	    counter_1000ms++;
	    if(counter_1000ms > 30) 
	    {
			counter_1000ms = 0;

    	}
	}

	/**************** HMI DEVELOPMENT MODE CODE ******************/
#ifdef ENABLE_HMI_DEVMODE
	ProcHmiDevControls();
#endif
	/**************** HMI DEVELOPMENT MODE CODE ******************/

    // Set up for next HMI alarm 
    
    gettimeofday(&end_time,NULL);
    Clk_GetDeltaTime(&start_time, &end_time, &delta_usec);
    delta_msec = delta_usec/1000u;
    next_msec = MSEC_30 - ( delta_msec - ((delta_msec/MSEC_30)*MSEC_30) );
    do 
    {
		rc = Clk_SetTimer(hmClk, false, next_msec, CLKSETALARM1);
		if(rc != GP_SUCCESS) 
		{
		    gp_Printf(VRB_DEBUG1, "\nHMAS_INTTSK: Clk_SetTimer() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);
    printf("timer reset in Hmi_mgr_as!\n");
    
}

/**************************************************************************************/
/*! \fn IntTsk_UmasRxHandler(uint32_t data, uint32_t size)
 *
 *	param[in] data	- data received
 *	param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Handle Unit Manager message receive event.  
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void IntTsk_UmasRxHandler(void * data)
{
    gp_retcode_t rc;
    int32_t ret;
    uint16_t msgId;
    uint8_t * msgDt;
    int offset;

    msgDt = (uint8_t *)data;
    offset = gp_Read16bit(&msgId, &msgDt[0]);
    switch(msgId) 
    {
		/* Process hearbeat message */
		case HbtReq:
		    /* This is a periodic request, so if the
		       following fails just log and continue 
		    ret = ProcHbtReq(&msgDt[offset], (TskBufInfo[BUF_UM_1].BufSz - offset));
		    if(ret != 0) 
		    {
				gp_Printf(VRB_DEBUG1, "\nDMAS_INTTSK: ProcHbtReq() error %d\n", ret);
		    }*/
		    break;
		/* Process application status message */
		case AppStatusMsg:
			/* If the Unit Mgr is reporting that the btldr has released the display */
			if(   (msgDt[offset + MSG_STAT_OP] == APP_STAT_RPT)
			   && (msgDt[offset + MSG_STAT_ID] == APP_ID_MAIN)
			   && (msgDt[offset + MSG_STAT_CODE] == APP_STAT_ANIM_DONE) )
			{
				gp_Printf(VRB_DEBUG1, "\nIntTsk_UmasRxHandler: Btldr video ready rcvd, starting HMI\n");
				StartHMI();
//				ok_to_start = true;			// allow HMI to start
			}
			break;
		/* Invalid message type, then do nothing */
		default:
		    break;
    }

}

/**************************************************************************************/
/*! \fn GetConnections()
 *
 *	param[in] None
 *
 *  \par Description:	  
 *   Get connections to the Unit and Datapool managers.  
 *
 *  \retval	result of type ::gp_retcode_t - 0 = Success, -1 = Error 
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static gp_retcode_t GetConnections()//TODO: need changes named semaphores
{
    gp_retcode_t retcode = GP_SUCCESS;
    int rc;
    int retries;
    //sem_t * HmiIpcSemaphore;
    sem_t * HmiDpSemaphore;
    mode_t semaphore_mode = S_IRGRP | S_IWGRP;
    //const char IpcHmiMgrSem[] = "/ipc_mgr_semaphore2";
    const char DpHmiMgrSem[] = "/dp_semaphore2";
    /*
    
    HmiIpcSemaphore = sem_open(IpcHmiMgrSem, O_CREAT, semaphore_mode,0);
    if(HmiIpcSemaphore == SEM_FAILED){
        printf("couldn't create/open ipc_semaphore in Hmi_mgr_as_worktask\n");
        rc = -1;
    }else{
    	// Request Unit Manager connection from resource mgr 
	    gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: Requesting IPC connection...");
	    retries = NUM_RSC_RETRIES;
	    do{
	        rc = sem_trywait(HmiIpcSemaphore);
	        if(rc != 0){
	            sleep(GET_CONN_WAIT_TIME_SEC);
	            retries--;
	        }else{
	        	printf("Ipc_mgr is up!\n");
	        }
	        
	    }while((rc != 0) && (retries > 0));
    }
    


    if(rc == 0){
		gp_Printf(DFLT_DBG_PRNTLVL, "Success\n");
    }else{
		gp_Printf(DFLT_DBG_PRNTLVL, "Failed - %d\n", rc);
		retcode = GP_INIT_ERR;
    }
    */

    /* Request Datapool Manager connection from resource mgr */
    retries = NUM_RSC_RETRIES;
    
    HmiDpSemaphore = sem_open(DpHmiMgrSem, O_CREAT, semaphore_mode,0);
    if(HmiDpSemaphore == SEM_FAILED){
        printf("couldn't create/open dp_semaphore in Hmi_mgr_as%s\n",strerror(errno));
        rc = -1;
    }else{
    	/* Request Unit Manager connection from resource mgr */
	    gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: Requesting PM connection...");
	    retries = NUM_RSC_RETRIES;
	    do{
	        rc = sem_trywait(HmiDpSemaphore);
	        if(rc != 0){
	            sleep(GET_CONN_WAIT_TIME_SEC);
	            retries--;
	        }else{
	        	printf("Datapool_mgr is up!\n");
	        	sem_unlink(DpHmiMgrSem);
	        }
	    }while((rc != 0) && (retries > 0));
    }
    

    if(rc == 0){
		gp_Printf(DFLT_DBG_PRNTLVL, "Success\n");
    }else{
		gp_Printf(DFLT_DBG_PRNTLVL, "Failed - %d\n", rc);
		retcode = GP_INIT_ERR;
    }
    //sem_destroy(HmiIpcSemaphore);
    /*if((sem_unlink(IpcHmiMgrSem)) != 0){
        printf("can't delete Unit manager semaphore from Hmi_mgr_as_worktask1!\n");
    }*/
    
    return retcode;
}

/**************************************************************************************/
/*! \fn static int32_t SendVideoReadyMsg(uint8_t data)
 *
 *	\param[in] data		- Video ready state
 *	\param[in] retries	- Number of retries sending the message
 *
 *  \par Description:	  
 *   Sends VideoReady message with given data to the GP main application IPC Manager.
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
/*static int32_t SendVideoReadyMsg(uint8_t data, int retries)
{
	int32_t retval;

	do
	{
	    retval = TxBufMsg(component, componentsId[0].Fd, VideoReady, componentsId[0].Tid, &data, sizeof(data));// (TxBufMsg(HMI_MGR, connection_to_HmiMgr, VideoReady, &data, sizeof(data)));
	    retval = 0;
		if(retval != 0)
		{
			retries--;
		}
	} while((retval != 0) && (retries > 0));

	return retval;
}*/

/**************************************************************************************/
/*! \fn int32_t ProcHbtReq(uint8_t * data, uint32_t size)
 *
 *	param[in] data	- pointer to the message
 *	param[in] size	- number of bytes in the message
 *
 *  \par Description:	  
 *   Process the heartbeat request message by parsing the message and sending a hearbeat
 *   response back.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
/*static int32_t ProcHbtReq(uint8_t * data, uint32_t size)
{
    uint8_t hbtRes[2];

    // Check if there is sleep mode request in the message 
    if( (data[0] == MSG_HBTREQ_MODE_1) || (data[0] == MSG_HBTREQ_MODE_2) ) 
    {
		HmiSleepReq = 1;
    } 
    else 
    {
		HmiSleepReq = 0;
    }

    //Simulate HMI code set HmiSleepReady = 1:
    HmiSleepReady = 1;

    // Compose response 
    hbtRes[0] = TSK_OK;
    if(HmiSleepReady == 1) 
    {
		hbtRes[1] = MSG_HBTRES_MODE_1;
    } 
    else 
    {
		hbtRes[1] = MSG_HBTRES_MODE_0;
    }

    // Send response to UM 
    return (TxBufMsg(component, componentsId[0].Fd, HbtHmIntTsk, componentsId[0].Tid, &hbtRes[0], sizeof(hbtRes)));
    //(TxBufMsg(HMI_MGR, connection_to_HmiMgr, HbtHmIntTsk, &hbtRes[0], sizeof(hbtRes)));
    
}*/


/**************************************************************************************/
/*! \fn static int32_t SendAppStatusMsg(uint8_t data)
 *
 *	\param[in] status	- Status to send, value type ::APP_STAT_CODE_T.
 *	\param[in] 
 *
 *  \par Description:	  
 *   Sends an Application Status message with given data to the GP main application
 *	 Unit Manager.
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
/*static int32_t SendAppStatusMsg(uint8_t status, int retries)
{
	int32_t retval;
	uint8_t data[MSG_STAT_DATLEN];

	// Build message data 
	data[MSG_STAT_OP] = APP_STAT_RPT;
	data[MSG_STAT_ID] = APP_ID_HMI;
	data[MSG_STAT_CODE] = status;

	// Send the message with N retries, if necessary 
	do
	{
	    retval = (TxBufMsg(component, componentsId[0].Fd, AppStatusMsg, componentsId[0].Tid, &data[0], sizeof(data)));
	    //(TxBufMsg(HMI_MGR, connection_to_HmiMgr, AppStatusMsg, data, sizeof(data)));
	    if(retval != 0)
		{
			retries--;
			usleep(100);
		}
		gp_Printf(VRB_DEBUG1, "\nHMI SendAppStatusMsg: status= %d, result = %d\n", status, retval);
	} while((retval != 0) && (retries > 0));

	return retval;
}*/


/**************************************************************************************/
/*! \fn int32_t StartHMI(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *   Start the task that handles HMI graphics and logic subsystem startup.
 *
 *  \retval	Returns GP_SUCCESS if OK, other values if error. See ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static gp_retcode_t StartHMI(void)
{
	gp_retcode_t retval = GP_SUCCESS;
	int32_t result;

	/* If the HMI has not been started */
	if(hmi_started == false)
	{
#ifdef EN_STARTUP_INSTR
		gp_Printf(VRB_DEBUG1, "$$$ HMI_MGR @START_HMI start: %lld msec\n", Clk_GetCurrTimeVal(DEFAULT_CLOCK, CLK_MSEC));
#endif
#ifndef HMI_STRESS_TEST
	    /* Make 'mydisk' the default directory and start the HMI */
		chdir(HMI_BASE_DIR);
		{
		  char curr_path[MAX_PATH_LEN];
		  getcwd(curr_path, MAX_PATH_LEN-2);
		  gp_Printf(VRB_DEBUG1, "CWD: %s\n", curr_path);
		}
		gp_Printf(VRB_DEBUG1, "Starting HMI\n");
	    HMISS_StartupInit();

#else
 #warning "** Hmi_mgr_as: Building HMI stress test version **"
	    /* Start HMI stress test */
		gp_Printf(VRB_DEBUG1, "!!! Starting HMI stress test !!!\n");
	    //StartVivanteDemo();TODO: replace this
#endif
	    /* Send video ready signal to VP ONLY if HMI startup was ok */
		//result = SendVideoReadyMsg(1, 10);
        result = 0;
		if(result != 0) 
		{
		    gp_Printf(VRB_DEBUG1, "\nStartHMI: SendVideoReadyMsg() error %d\n", result);
		}

		hmi_started = true;

 #ifdef EN_STARTUP_INSTR
		gp_Printf(VRB_DEBUG1, "$$$ HMI_MGR  @START_HMI end: %lld msec\n", Clk_GetCurrTimeVal(DEFAULT_CLOCK, CLK_MSEC));
 #endif
	}
	return retval;
}



/**************** HMI DEVELOPMENT MODE CODE ******************/
#ifdef ENABLE_HMI_DEVMODE

/**************************************************************************************/
/*! \fn ProcHmiDevControls(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *  Processes HMI development mode user play control inputs.
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static void ProcHmiDevControls(void)
{
	CheckForHmiDevMode();

	/* Check if the HMI development mode is active */
	if(1 == hmi_dev_mode)
	{
#if 1
		GetHmiDevControls();		// use the real controls
#else
		SimHmiDevControls();		// generate simulated controls
#endif
	}
}


/**************************************************************************************/
/*! \fn CheckForHmiDevMode(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *  Determine if the HMI development mode is active and set the hmi_dev_mode flag if
 *	it is.
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static void CheckForHmiDevMode(void)
{
	uint32_t TestPatternValue;

	/* If the HMI dev mode flag is uninitialized then check it */
	if(hmi_dev_chkctr > 0)
	{
		/* Check if the HMI development mode is active */
		GET_ELEMENT(YzTdTestPattern,&TestPatternValue);
		if(DEV_TST_PATT_ID == TestPatternValue)
		{
			hmi_dev_mode = 1;
			hmi_dev_chkctr = 0;
		}
		else
		{
			hmi_dev_mode = 0;
		}
		hmi_dev_chkctr--;
	}
}


//#define HMIDEV_RST_CTR 5
/**************************************************************************************/
/*! \fn GetHmiDevControls(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *   Processes the play controls datapool item for the HMI development mode.
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static void GetHmiDevControls(void)
{
	static uint16_t last_dev_ctrls = 0;
	uint16_t dev_ctrls;

	/* Get the current HMI development mode controls */
	GET_ELEMENT(YzTdHmiDevCtrls,&dev_ctrls);

	/* If they have changed then process them */
	if(dev_ctrls != last_dev_ctrls)
	{
		/* Update the dev mode */
		//CbTestPatternHmiCtrl_Link((uint32_t)dev_ctrls);TODO: replace this
		last_dev_ctrls = dev_ctrls;
	}		
}


/**************************************************************************************/
/*! \fn SimHmiDevControls(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *   Simulates the play controls for the HMI development mode.
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static void SimHmiDevControls(void)
{
	static int mode_ctr = MODE_CHG_CNT;
	static int active_mode = 0;
	static int last_mode = 0;

	/* If time to run simulation */
	if(hdev_ctr <= 0)
	{
		bool reset_hctr = true;

		/* If time to change the mode */
		mode_ctr--;
		if(mode_ctr <= 0)
		{
			mode_ctr = MODE_CHG_CNT;
			active_mode++;
			if(active_mode > 2)
			{
				active_mode = 0;
			}
		}
		/* Set the mode controls */
		switch(active_mode)
		{
			case 0:				// Step up
				gp_Printf(VRB_DEBUG2, "HMI dev sim: Step up\n");
				hdev_ctrls = HMIDEV_CTRL_STEP | HMIDEV_CTRL_STEPUP;
				break;
			case 1:				// Step down
				gp_Printf(VRB_DEBUG2, "HMI dev sim: Step down\n");
				hdev_ctrls = HMIDEV_CTRL_STEP | HMIDEV_CTRL_STEPDN;
				break;
			case 2:				// Play
				if(active_mode != last_mode)		// only print the 1st time
				{
					gp_Printf(VRB_DEBUG2, "HMI dev sim: Play\n");
				}
				hdev_ctrls = HMIDEV_CTRL_PLAY;
				break;
			case 3:				// Reset frame counter
				gp_Printf(VRB_DEBUG2, "HMI dev sim: Reset frame cntr\n");
				hdev_ctrls = HMIDEV_CTRL_RST;
				reset_hctr = false;				// this will cause the SM to move to the next mode immediately
				break;
			default:
				break;
		}
		/* Update the dev mode */
		//CbTestPatternHmiCtrl_Link(hdev_ctrls);TODO: replace this		// put a breakpoint here to change the controls
		last_mode = active_mode;

		/* Reset the update counter */
		if(reset_hctr == true)
		{
			hdev_ctr = (MODE_UPD_MSEC / HMI_FRAME_TIME);		// reset sim counter
		}	 	
	}
	/* Else decr counter */
	else
	{
		hdev_ctr--;
	}
}

#endif  // end ENABLE_HMI_DEVMODE
/**************** HMI DEVELOPMENT MODE CODE ******************/
