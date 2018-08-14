/**************************************************************************************/
/*!
 *  \file		Hmi_mgr_as.c
 *
 *  \copyright	Yazaki 2016
 *
 *  \brief		Top level FF HUD application HMI manager functions.  This software component
 *				initializes and starts the HMI. NOTE: Be aware this file among 
 *              others in this project had been modified in order to be able 
 *              to run in the linux environment and to be able to produce an 
 *              application that would not crash despite there are several 
 *              components not ported yet.
 *
 *  \author		E. Gunarta, D. Kageff modified by Leonardo Reatiga Miranda
 *
 *  \version	$Revision: 1.11 $  
 *				$Log $
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
#define _HMI_MGR_AS_WORKTASK1_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#include "gp_cfg.h"         // Common GP program configuration settings
#include "gp_types.h"       // Common GP program data type definitions
#include "gp_utils.h"       // Common GP program utility functions

#include "clk_api_linux.h"
#include "msg_buf.h"
#include "msg_api_signals.h"
#include "msg_def.h"
#include "msg_fcn.h"

#include "Datapool.h"		// Datapool access functions
#include "pool_def.h"		// Global datapool definitions.
#include "identification_data.h"

#include "Hmi_mgr_int.h"	// Definitions from Integrate file

/***********************************
	Private Macros and Typedefs
***********************************/
typedef struct {
    uint32_t BufSz;  /*!< Size (in bytes) of data buffer */
    uint32_t BufId; /*!< Index of data buffer */
    uint8_t * Buf;   /*!< Pointer to data buffer */
} BufInfoType;

static uint8_t  component = HMI_MGR_WRKTSK1;

/***********************************
	               Config Macros
***********************************/
/*!< List of TskInfoType entries */
enum {
    BUF_DM_1 = 0,
    BUFINFO_NUM_ENTRIES
};
/*!< Size of buffer used for receiving messages on Hm_UmAsCon2 connection */
#define UMASCON2_BUF_SZ 4

#define GET_CONN_WAIT_TIME_SEC	1		/*!< Number of seconds to wait between attempts to get a connection 
											 obejct from the Resource Manager */

#define NUM_RSC_RETRIES		10		/*!< Number of times to try and get a resource from the resource manager
										 before giving up. */


/***********************************
	Private Data and Structures
***********************************/
/* INTEGRITY Connection messages */
static BufInfoType TskBufInfo[BUFINFO_NUM_ENTRIES];

/* Used for prewarp in driver.c */
uint8_t WarpLoad = 0;
uint8_t WarpDisplay = 0;
int32_t MirrorPos = 0;

/*! Connection to the Unit Manager */
//Connection HmAs_UmAsCon2;

/*! Connection to the Datapool Manager */
//Connection HmAs_PmAsCon1;

/***********************************
	Private Function Prototypes
***********************************/
static void MsgRxHandler(int sig, siginfo_t *si, void *uc);
void DplTsk_HmiAlarmHandler(int sig, siginfo_t *si, void *uc);
void DplTsk_HmiReceiveHandler(void * data);
static gp_retcode_t GetConnections(void);
static int32_t ProcHbtReq(uint8_t * data, uint32_t size);

function_cb_t msg_receive_handler[] ={
    DplTsk_HmiReceiveHandler
};

/************ Start of code ******************/


/**************************************************************************************/
/*! \fn Hm_DisplayTskMain()
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
static timer_t HmWrkTskClk;
component_info_t componentsId[BUFINFO_NUM_ENTRIES];
static uint8_t components[] = {U_MGR_AS,DP_MGR_AS};
void * Hm_DplTskMain(void * ignore)
{
    gp_retcode_t rc;
    component_info_t tmpCom[BUFINFO_NUM_ENTRIES];

    gp_Printf(DFLT_DBG_PRNTLVL, "\nHM_DPLTSK: Started\n");

    //Get connections to other application managers 
    const char connection[] = CommonDp_ConPath;
    PostTid(connection, component, &tmpCom[0]);
    //init buffers
    do 
    {
        rc = Msg_InitBufs(SUBCOMPONENT1);
        if(rc != GP_SUCCESS) 
        {
            gp_Printf(VRB_RUNTIME, "\nHMAS_INTTSK: Msg_InitBufs() error %d\n", rc);
        }
    } while(rc != GP_SUCCESS);

    // Start rx buf msgs from Dp_MGR 
    TskBufInfo[BUF_DM_1].BufSz = UMASCON2_BUF_SZ;
    do{
        TskBufInfo[BUF_DM_1].Buf = Msg_GetBuf(TskBufInfo[BUF_DM_1].BufSz, &TskBufInfo[BUF_DM_1].BufId, SUBCOMPONENT1);
        if(TskBufInfo[BUF_DM_1].Buf == NULL){
            printf("Msg_GetBuf, failed in Component:%i\n", component );
        }
    }while(TskBufInfo[BUF_DM_1].Buf == NULL);
    
    //create a server socket to comunicate with dp_mgr
    do{
        tmpCom[0].Fd = SetTxOn(HmiMgrWrkTsk1DpMgr_ConPath);//because we already put a Rx side in Datapool_mgr_as.c
        if(tmpCom[0].Fd < 0){
            printf("SetTxOn, failed in Component:%i\n", component);
        }
    }while(tmpCom[0].Fd < 0);
    printf("SetTxOn passed!\n");

    /* Start hmi alarm */
    do 
    {
		sigset_t mask;
        struct sigaction sa;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = DplTsk_HmiAlarmHandler;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, CB_TRUE);
        rc = sigaction(CLKSETALARM2, &sa, NULL);

		if(rc != 0) 
		{
		    gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: Msg_AsyncRxEvent(alarm) error %d\n", rc);
		}
    } while(rc != 0);
    printf("WrkTsk1 passed sigaction\n");
    
    do 
    {
        sigset_t mask;
        struct sigaction sa;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = MsgRxHandler;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, CLKSETALARM2);
        //sigaddset(&sa.sa_mask, SIGURG);
        rc = sigaction(CB_TRUE, &sa, NULL);

        if(rc != 0) 
        {
            printf("\nHMAS_INTTSK: Msg_AsyncRxEvent(alarm) error %d\n", rc);
        }
    } while(rc != 0);

    /*do{
        rc = WaitSemaphore(dp_semaphore);
        printf("rc after WaitSemaphore is:%i\n",rc );
    }while(rc != 0);*/
    memcpy(&componentsId[0], &tmpCom[0], sizeof(tmpCom));
    rc = WaitSemaphore(dp_semaphore);
    printf("rc after WaitSemaphore is:%i\n",rc );
    printf("WaitSemaphore passed!\n");
    
    do 
    {
		
		rc = Clk_SetTimer(&HmWrkTskClk, false, MSEC_15, CLKSETALARM2);
		if(rc != GP_SUCCESS) 
		{
		    gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: Clk_SetTimer() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);
    printf("WrkTsk1 passed Clk_SetTimer\n");
    
    //after we have installed the signal handler we can start to TxRx messages, so we unlock/wait for a given semaphore
    
    
    
    /* Main HMI manager loop */
    while(1);
}

static void MsgRxHandler(int sig, siginfo_t *si, void *uc){

    uint8_t buffer[256];
    memset(&buffer[0], 0, sizeof(buffer));
    for (int i = 0; i < (sizeof(componentsId)/sizeof(component_info_t)); ++i)
    {
        if (componentsId[i].Component == si->si_value.sival_int){
            read(componentsId[i].Fd, &buffer[0],sizeof(buffer));
            msg_receive_handler[i](&buffer[0]);
            break;
        }
    }   
}
/**************************************************************************************/
/*! \fn DplTsk_UmasRxHandler(uint32_t data, uint32_t size)
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
void DplTsk_UmasRxHandler(void * data, uint32_t size)
{
    gp_retcode_t rc;
    int32_t ret;
    uint16_t msgId;
    uint8_t * msgDt;
    int offset;

    msgDt = (uint8_t *)data;
    offset = gp_Read16bit(&msgId, &msgDt[0]);
    switch(msgId) {

	case HbtReq:
	    /* This is a periodic request, so if the
	       following fails just log and continue */
	    ret = ProcHbtReq(&msgDt[offset], (size - offset));
	    if(ret != 0) {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nDMAS_INTTSK: ProcHbtReq() error %d\n", ret);
	    }
	    break;

	default:
	    break;
    }

}

/**************************************************************************************/
/*! \fn DplTsk_HmiAlarmHandler(uint32_t data, uint32_t size)
 *
 *	param[in] data	- ignore since this is an event
 *	param[in] size	- ignore since this is an event
 *
 *  \par Description:	  
 *   Handle HMI screen update timer.  
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void DplTsk_HmiAlarmHandler(int sig, siginfo_t *si, void *uc)
{
    gp_retcode_t rc;
    int32_t ret;
    PoolCopyResType Msg;
    uint32_t transferred = 0;
    uint8_t data[4];

    /* Get copy of the datapool from the Datapool Manager */

    /* Send the datapool copy request */
    
    

    ret = TxBufMsg(component, componentsId[0].Fd, PoolCopyReq, componentsId[0].Tid, &data[0], sizeof(PoolCopyReq));
    //TxBufMsg(HMI_MGR_WRKTSK1, connection_to_HmiMgr, PoolCopyReq, NULL, 0);
    if(ret != 0) {
	gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: TxBufMsg(PM) error %d\n", ret);
    }else{
        printf("TxBufMsg success!\n" );
    } 
    
}

void DplTsk_HmiReceiveHandler(void * data){
	
	PoolCopyResType Msg = *((PoolCopyResType *)data);
	gp_retcode_t rc;
	/* If the datapool was received correctly then update the local datapool */
	if(Msg.Id == PoolCopyRes){
		SetPool(&Msg.Dt);
	}

	/* Update local data parameters from the datapool */
	GetElem(YzTdWarpLoad, &WarpLoad);
	GetElem(YzTdWarpDisplay, &WarpDisplay);
	GetElem(YzTdMirrorPos, &MirrorPos);
	
	/* Start HMI alarm for periodic datapool requests */
    do {
		rc = Clk_SetTimer(&HmWrkTskClk, false, MSEC_30, CLKSETALARM2);
		if(rc != GP_SUCCESS) {
		    gp_Printf(DFLT_DBG_PRNTLVL, "\nHMAS_DPLTSK: Clk_SetTimer() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);

}

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
static int32_t ProcHbtReq(uint8_t * data, uint32_t size)//TODO: if this is intented to connect to the datapool a conection must be created
{
    uint8_t status[1];

    status[0] = TSK_OK;
    
    return (TxBufMsg(component, componentsId[0].Fd, HbtHmDplTsk, componentsId[0].Tid, &status[0], sizeof(status)));
}
