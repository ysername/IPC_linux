/**************************************************************************************/
/*!
 *  \file		Datapool_mgr_as.c
 *
 *  \copyright	Yazaki 2016	- 2017
 *
 *  \brief		Top level FF HUD application datapool manager functions.  This software component
 *				initializes the datapool mananger subsystem.
 *              NOTE: Be aware this file among others in this project had been 
 *              modified in order to be able to run in the linux environment
 *              and to be able to produce an application that would not crash
 *              despite there are several components not ported yet.
 *
 *  \author		D. Kageff  modified by Leonardo Reatiga
 *
 *  \version	$Revision: 1.18 $  
 *				$Log Datapool_mgr_as.c $
 *
 *
 ***************************************************************************************
 * \page sw_component_overview Application Software Component Overview page
 *	The Datapool Manager application software component is responsible for managing all
 *	global application specific data. It sends and receives pool data via inter address 
 *	space messages implemented in msg_api.c. The top level datapool manager functions are
 *	implemented in Datapool_mgr_as.c.
 *
 *  TODO LIST:
 *	\todo  Eudora - Fix warnings.
 */
/***************************************************************************************/
#define _DATAPOOL_MGR_AS_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gp_cfg.h"		// Common GP program configuration settings
#include "gp_types.h"		// Common GP program data type definitions
#include "gp_utils.h"		// Common GP program utility functions

#include "msg_buf.h"
#include "msg_api_signals.h"
#include "msg_def.h"
#include "msg_fcn.h"
#include "App_cfg.h"

#include "Datapool.h"	// Also includes pool_def.h
#include "Hmi_demo.h"
#include "identification_data.h"


/***********************************
	Private Macros and Typedefs
***********************************/
/*! Holds info about an INTEGRITY Connection message */
typedef struct {
    uint32_t BufSz;  /*!< Size (in bytes) of data buffer */
    uint32_t BufId; /*!< Index of data buffer */
    uint8_t * Buf;   /*!< Pointer to data buffer */
} BufInfoType;

/***********************************
	              Config Macros
***********************************/
/*!< Manager Version/Revision.  Format: vvrr. */
#define VER_REV 0x0103

/*!< List of TskInfoType entries */
enum {
    BUF_HM_1 = 0,
    BUFINFO_NUM_ENTRIES
};
static BufInfoType TskBufInfo[BUFINFO_NUM_ENTRIES];

#ifdef DEBUG
 #define CONN_TIMEOUT 1000
#else
 #define CONN_TIMEOUT 2
#endif

/* Operation command processing definitions */
#define OPCMD_HDR_SZ 2			/*!< Op cmd IPC message header size in bytes */

/* Size of buffer used for receiving messages on PmAs_UmAsCon1 connection */
#define UMASCON1_BUF_SZ 4//IntTsk_UmasBufRxHandler

/* Size of buffer used for receiving messages on PmAs_HmCon1 connection */
#define HMASCON1_BUF_SZ 4

/* Size of buffer used for receiving messages on PmAs_EmAsCon1 connection */
#define EMASCON1_BUF_SZ 128

/* Size of buffer used for receiving messages on PmAs_ImAsCon1 connection
   Size is in bytes and should be at least the max size of a SPI message */
#define IMASCON1_BUF_SZ 128

#define IM_READY 10

/***********************************
	Private Data and Structures
***********************************/
static uint8_t  cmp_buf_num = MAIN_COMPONENT;
static uint8_t  component = DP_MGR_AS;
/*********************************/
/*  Memory allocation            */
/*********************************/

/***********************************
	Private Function Prototypes
***********************************/

void IntTsk_HmasBufRxHandler(void * data);//HMASCON1_BUF_S

function_cb_t msg_receive_handler[] ={
    IntTsk_HmasBufRxHandler
};
static int32_t ProcHbtReq(uint8_t * data, uint32_t size);
static int32_t ProcPoolCopyReq(void);
static int32_t ProcSetElemMsg(uint8_t * data, uint32_t size);
static int32_t ProcGetElemMsg(int8_t socket_fd, pid_t tid, uint8_t ImComponent, uint8_t * data, uint8_t dataSz);

static gp_retcode_t PmProcOpMode(uint8_t *p_buf, int cmdlen);
static gp_retcode_t PmProcOpInitData(uint8_t *p_buf, int cmdlen);

void MsgRxHandler(int sig, siginfo_t *si, void *uc);

uint8_t components[] = {HMI_MGR_WRKTSK1};
component_info_t componentsId[BUFINFO_NUM_ENTRIES];

/************ Start of code ******************/
/*! \defgroup dpmgr_public Datapool Manager public API*/

/**
    @brief main() Main entry point of the Datapool manager component. 

*/
int main()
{
  printf("CB_FALSE is:%d\nCB_TRUE is:%d\nCLKSETALARM%d\n", CB_FALSE, CB_TRUE, CLKSETALARM);
    gp_retcode_t rc;
    int32_t ret;
    component_info_t tmpCom[BUFINFO_NUM_ENTRIES];
    
    
#ifdef DEBUG
    gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: Started\n");
#endif /* DEBUG */
    /*Wait to get the TID of the components that will comunicate with this process*/
    const char connection[] = CommonDp_ConPath;
    do{
        rc = GetTids(connection, components, sizeof(components)/sizeof(uint8_t), tmpCom, component);
        if(rc != GP_SUCCESS){
            printf("Msg_InitBufs,failed in Component:%i\n",component);
        }
    }while(rc != GP_SUCCESS);

    /* Init data pool */
    do 
    {
		rc = InitPool();//needs minimum changes
		if(rc != GP_SUCCESS) 
		{    
		    gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: InitPool() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);

    /* Init msg bufs */
    do 
    {
		rc = Msg_InitBufs(MAIN_COMPONENT);
		if(rc != GP_SUCCESS) 
		{   
		    gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: Msg_InitBufs() error %d\n", rc);
		}
    } while(rc != GP_SUCCESS);

    // Start rx buf msgs from HMIMGR_WRKTSK1
    TskBufInfo[BUF_HM_1].BufSz = HMASCON1_BUF_SZ;
    do{
        TskBufInfo[BUF_HM_1].Buf = Msg_GetBuf(TskBufInfo[BUF_HM_1].BufSz, &TskBufInfo[BUF_HM_1].BufId, MAIN_COMPONENT);
        if(TskBufInfo[BUF_HM_1].Buf == NULL){
            printf("Msg_GetBuf, failed in Component:%i\n", component );
        }
    }while(TskBufInfo[BUF_HM_1].Buf == NULL);
    
    //create a server socket to comunicate with HMI_mgr_wrktsk1
    do{
        tmpCom[0].Fd = SetRxOn(HmiMgrWrkTsk1DpMgr_ConPath);
        if(tmpCom[0].Fd < 0){
            printf("errno after is:%s\n",strerror(errno) );
            printf("Msg_WaitForRx, failed in Component:%i\n", component);
        }
    }while(tmpCom[0].Fd < 0);

    do 
    {
        sigset_t mask;
        struct sigaction sa;

        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = MsgRxHandler;
        sigemptyset(&sa.sa_mask);
        sigaddset(&sa.sa_mask, SIGPOLL);
        sigaddset(&sa.sa_mask, SIGURG);
        rc = sigaction(CB_TRUE, &sa, NULL);

        if(rc != 0) 
        {
            printf("\nHMAS_INTTSK: Msg_AsyncRxEvent(alarm) error %d\n", rc);
        }
    } while(rc != 0);


    memcpy(&componentsId[0], &tmpCom[0], sizeof(tmpCom));
    PostSemaphore(dp_semaphore, 2);

    while(1){int inside_infinite_while = 456;};
    
    /* Set up connection to the HMI Manager */
#ifdef PM_PRINTF_OUT    
    printf("PMAS_INTTSK: Setting up HMI connection\n");
#endif /* PM_PRINTF_OUT */
    

}
/**
    @brief MsgRxHandler()   this function gets called when a message is 
                    received, and depending on which "component" sent the
                    message (see the man page for sigqueue for more info)
                    a specific callback is executed.

*/
void MsgRxHandler(int sig, siginfo_t *si, void *uc){

    uint8_t buffer[256], ret = 0;
    memset(&buffer[0], 0, sizeof(buffer));
    for (int i = 0; i < (sizeof(components)/sizeof(uint8_t)); ++i)
    {
        if (componentsId[i].Component == si->si_value.sival_int){
            ret = read(componentsId[i].Fd, &buffer[0],sizeof(buffer));
            msg_receive_handler[i](&buffer[0]);
            break;
        }
    }   
    sem_unlink(dp_semaphore);
}

/**************************************************************************************/
/*! \fn IntTsk_UmasBufRxHandler(uint32_t data, uint32_t size)
 *
 *	\param[in] data	- data received
 *	\param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Callback for when a message is received from unit manager.
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/

/*void IntTsk_UmasBufRxHandler(void * data){
   
    gp_retcode_t rc;
    int32_t ret;
    uint16_t msgId;
    uint8_t * msgDt;
    int offset;

    // Process the message based on its message ID 
    msgDt = (uint8_t *)data;
    offset = gp_Read16bit(&msgId, &msgDt[0]);
    switch(msgId) 
    {
	    // Process heartbeat request message 
	case HbtReq:
	    // This is a periodic request, so if the following fails just log and continue 
	    ret = ProcHbtReq(&msgDt[offset], (TskBufInfo[BUF_UM_1].BufSz - offset));
	    if(ret != 0) {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcHbtReq() error %d\n", ret);
	    }
	    break;
	    // Unknown message ID - Do nothing
	default:
	    break;
    }

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
static int32_t ProcHbtReq(uint8_t * data, uint32_t size){//TODO: who is te receiver  of this request?

    uint8_t status[1];

    status[0] = TSK_OK;
    return(TxBufMsg( component, componentsId[0].Fd, HbtPmAsIntTsk, componentsId[0].Tid, &status[0], sizeof(status)));


}

/**************************************************************************************/
/*! \fn IntTsk_HmasBufRxHandler(uint32_t data, uint32_t size)
 *
 *	\param[in] data	- data received
 *	\param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Callback for when a message is received from HMI manager.
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void IntTsk_HmasBufRxHandler(void * data)
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
	    /* Process datapool copy request.  Treat all copy pool requests the same. */	
	case PoolCopyReq:
	    ret = ProcPoolCopyReq();//need changes
	    if(ret != 0) 
	    {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcPool32CopyReq() error %d\n", ret);
	    }else{
            printf("ProcPoolCopyReq Success!\n");
        }
	    break;
	    /* Unknown/unsupported request - Do nothing */		    
	default:
	    break;
    }
    
}

/**************************************************************************************/
/*! \fn ProcPoolCopyReq()
 *
 *	param - No parameters
 *
 *  \par Description:	  
 *   Services a datapool copy request by sending the copy to HMI manager. 
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static int32_t ProcPoolCopyReq(void)//TODO: wich process will receie this message? a callback should be executed upon tx?
{
    gp_retcode_t rc;
    PoolCopyResType Msg;

    Msg.Id = PoolCopyRes;
    rc = GetPool(&Msg.Dt);
    if(rc != GP_SUCCESS) 
    {
		return -1;
    }
    
    rc = TxMsg(componentsId[0].Fd, componentsId[0].Tid, component,(uint8_t *)&Msg, sizeof(Msg.Dt), true);

    if(rc != GP_SUCCESS) 
    {
        return -2;
    }
    return 0;
}
   
/**************************************************************************************/
/*! \fn IntTsk_UmasBufRxHandler2(uint32_t data, uint32_t size)
 *
 *	\param[in] data	- data received
 *	\param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Callback for when a buffer message is received from unit manager.
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	 \todo change to support multiple commands in the same transfer.
 **************************************************************************************/
/*void IntTsk_UmasBufRxHandler2(void * data)//need changes unknow
{

    gp_retcode_t rc;
    gp_retcode_t retval = GP_OPCMD_ERR;

    // Process op command type 
    switch(TskBufInfo[BUF_UM_2].Buf[0])
    {
		case CmdMsgSetOpMode:
		    retval = PmProcOpMode(TskBufInfo[BUF_UM_2].Buf, TskBufInfo[BUF_UM_2].BufSz);//need changes unknow*
		    break;
		case CmdMsgSetInitData:
		    retval = PmProcOpInitData(TskBufInfo[BUF_UM_2].Buf, TskBufInfo[BUF_UM_2].BufSz);//needs changes
		    break;
		default:
		    // Respond with 'not supported' 
		    TskBufInfo[2].Buf[2] = (uint8_t)GP_OPCMD_NOTSUPP;
		    retval = GP_OPCMD_NOTSUPP;
		    break;
    }

    // Send response back to the Unit Manager 
    TskBufInfo[BUF_UM_2].Buf[2] = retval;
    rc=(TxMsg(componentsId[0].Fd, componentsId[0].Tid, component, TskBufInfo[BUF_UM_2].Buf,TskBufInfo[BUF_UM_2].BufSz, false));

    if(rc != GP_SUCCESS) 
    {
        retval = GP_OPCMD_ERR;
    }
    //TODO: add timeout
}*/

/**************************************************************************************/
/*! \fn gp_retcode_t PmProcOpMode(uint8_t p_buf, int cmdlen)
 *
 *	param[in] none
 *
 *  \par Description:	  
 *  Process datapool manager operation mode command from Unit Mgr.  
 *
 *  \retval	result of type ::gp_retcode_t - 0 = Success, -1 = Error 
 *
 *  \par Limitations/Caveats:
 *	 - This is currently a blocking call with a timeout.
 *
 *	 \todo Change to use TimedSynchronousReceive() and TimedSynchronousSend()
 **************************************************************************************/
static gp_retcode_t PmProcOpMode(uint8_t *p_buf, int cmdlen)//need changes unknow
{
	
    gp_retcode_t retval = GP_OPCMD_ERR;

	/* Process op command type */
	switch(p_buf[1])
	{
		case OPMODE_NORMAL:
			SetElem32(YzTdTestPattern , 0);// make sure that no test screen is selected
			retval = GP_SUCCESS;
			break;
		case OPMODE_DEMO:
			HmiDemoStart();	//need changes unknow	/* Start demo task */
			SetElem32(YzTdTestPattern , 0);	// make sure that no test screen is selected
			retval = GP_SUCCESS;
			break;
		case OPMODE_TSTPATT:
			SetElem32(YzTdTestPattern , p_buf[2]);
			retval = GP_SUCCESS;
			break;
		default:
			retval = GP_OPCMD_NOTSUPP;
			break;
	}
	return retval;
}

/**************************************************************************************/
/*! \fn gp_retcode_t PmProcOpInitData(uint8_t p_buf, int cmdlen)
 *
 *	\param[in] p_buf  - Pointer to command buffer.
 *	\param[in] cmdlen - Pointer to command length, in bytes.
 *
 *  \par Description:	  
 *  Process datapool initialization command values from Unit Mgr.  
 *
 *  \retval	result of type ::gp_retcode_t - 0 = Success, -1 = Error 
 *
 *  \par Limitations/Caveats:
 *	 - This is currently a blocking call.
 *
 *	 \todo Change to use TimedSynchronousReceive() and TimedSynchronousSend()
 **************************************************************************************/
static gp_retcode_t PmProcOpInitData(uint8_t *p_buf, int cmdlen)//needs changes
{
    gp_retcode_t retval = GP_SUCCESS;
	int i;
	int offset = OPCMD_HDR_SZ;
	uint32_t temp32;
	uint16_t temp16;
  	uint16_t id;

	/* Put data iems in the datapool. Data data format is <datapool_id><data_val> */
	/* For each data item */
	for(i=0; i < p_buf[OFFSET_NUM_ITEMS]; i++)
	{
		offset += gp_Read16bit(&id, &p_buf[offset]); 
		/* Process datapool item */
		switch(id)
		{
			case YzTdoNavOpts:
				offset += gp_Read32bit(&temp32, &p_buf[offset]);//safe to reuse
				SetElem(YzTdoNavOpts , (void *)&temp32);	//needs changes		// Set the datapool item value
				retval = GP_SUCCESS;
				break;
			case YzTdoAudioOpts:
				offset += gp_Read32bit(&temp32, &p_buf[offset]);//safe to reuse
				SetElem(YzTdoAudioOpts, (void *)&temp32);		// Set the datapool item value
				retval = GP_SUCCESS;
				break;
			case YzTdoNavSimFrames:
				offset += gp_Read16bit(&temp16, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoNavSimFrames, (void *)&temp16);		// Set the datapool item value
				break;
			case YzTdoNavFrameDly:
				offset += gp_Read16bit(&temp16, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoNavFrameDly, (void *)&temp16);		// Set the datapool item value
				break;
			case YzTdoNavLoopDly:
				offset += gp_Read32bit(&temp32, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoNavLoopDly, (void *)&temp32);		// Set the datapool item value
				break;
			case YzTdoAudioSimFrames:
				offset += gp_Read16bit(&temp16, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoAudioSimFrames, (void *)&temp16);		// Set the datapool item value
				break;
			case YzTdoAudioFrameDly:
				offset += gp_Read16bit(&temp16, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoAudioFrameDly, (void *)&temp16);		// Set the datapool item value
				break;
			case YzTdoAudioLoopDly:
				offset += gp_Read32bit(&temp32, &p_buf[offset]); //safe to reuse
			 	SetElem(YzTdoAudioLoopDly, (void *)&temp32);		// Set the datapool item value
				break;
			default:
				retval = GP_OPCMD_NOTSUPP;
				break;
		}
		if(retval != GP_SUCCESS)
		{
			break;
		}
	}
	return retval;
}

/**************************************************************************************/
/*! \fn WrtTsk_EmasBufRxHandler(uint32_t data, uint32_t size)
 *
 *	param[in] data	- address of data received
 *	param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Callback for when a message > 4-byte is received from ethernet manager.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
/*void IntTsk_EmasBufRxHandler(void * data)//TODO:check this function
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
	case SetElemReq:
	    ret = ProcSetElemMsg(&msgDt[offset], (TskBufInfo[BUF_EM_1].BufSz - offset));
	    ret =0;
	    if(ret != 0)
	    {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcSetElemMsg() error %d\n", ret);
	    }
	    break;
	case GetElemReq:
	    ret = ProcGetElemMsg(componentsId[0].Fd, componentsId[0].Tid, component, &msgDt[offset], (TskBufInfo[BUF_EM_1].BufSz - offset));
	    ret = 0;
	    if(ret != 0)
	    {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcGetElemMsg() error %d\n", ret);
	    }
	    break;
	default:
	    break;
    }
}*/

/**************************************************************************************/
/*! \fn IntTsk_ImasBufRxHandler(uint32_t data, uint32_t size)
 *
 *	param[in] data	- address of data received
 *	param[in] size	- number of bytes received
 *
 *  \par Description:	  
 *   Callback for when a message > 4-byte is received from IPC manager.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
/*void IntTsk_ImasBufRxHandler(void * data)
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
	case SetElemReq:
	    ret =  ProcSetElemMsg(&msgDt[offset], (TskBufInfo[BUF_IM_1].BufSz - offset));
	    if(ret != 0)
	    {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcSetElemMsg() error %d\n", ret);
	    }
	    break;
	case GetElemReq:
	    ret = ProcGetElemMsg(componentsId[0].Fd, componentsId[0].Tid, component, &msgDt[offset], (TskBufInfo[BUF_IM_1].BufSz - offset));
	    if(ret != 0)
	    {
		gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: ProcSetElemMsg() error %d\n", ret);
	    }
	    break;
	default:
	    break;
    }
}*/

/**************************************************************************************/
/*! \fn ProcSetElemMsg(Object obj, uint8_t * data, uint32_t size)
 *
 *      param[in] obj	- INTEGRITY Connection to be used for transfers
 *	param[in] data	- pointer to the message
 *	param[in] size	- number of bytes in the message
 *
 *  \par Description:	  
 *   Process a set element request message by parsing the message and setting the 
 *   appropiate datapool element with the given value.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/

static int32_t ProcSetElemMsg(uint8_t * data, uint32_t size)//needs minimum changes
{
    
    gp_retcode_t rc;
    int32_t ret;
    int offset;
    uint16_t elemId;
    GP_DATATYPES_T elemType;
    int32_t elemLen;
    uint32_t sizeCounter;

    /* Read element id */
    if(size < ELEM_ID_SZ) {
	return -1;
    }
    offset = gp_Read16bit(&elemId, &data[0]);
    sizeCounter = size - ELEM_ID_SZ;

    /* Read element type */
    rc = GetElemInfo(elemId, &elemType, &elemLen);//safe to reuse
    if(rc != GP_SUCCESS) {
	return -2;
    }
    
    /* Set element value */
    ret = 0;
    switch(elemType)
    {
	/*case GP_UINT8:
	    {
		uint8_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    temp = data[offset];
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }*/
	case GP_INT16:
	    {
		int16_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read16bitSigned(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_UINT16:
	    {
		uint16_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read16bit(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_INT32:
	    {
		int32_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read32bitSigned(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_UINT32:
	    {
		uint32_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read32bit(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_INT64:
	    {
		int64_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read64bitSigned(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }

	case GP_UINT64:
	    {
		uint64_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_Read64bit(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_FLOAT:
	    {
		uint32_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_ReadFloat(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_DBL:
	    {
		uint64_t temp;
		if(sizeCounter < elemLen) {
		    ret = -3;
		} else {
		    gp_ReadDouble(&temp, &data[offset]);
		    rc = SetElem(elemId, (void *)&temp);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	case GP_STRING:
	    {
		if(sizeCounter < 1) {
		    ret = -3;
		} else {
		    rc = SetElem(elemId, (void *)&data[offset]);
		    if(rc != 0) {
			ret = -4;
		    }
		}
		break;
	    }
	default:
	    ret = -5;
	    break;
    }

    if(ret != 0) {
	gp_Printf(DFLT_DBG_PRNTLVL, "\nPMAS_INTTSK: SetElem(%d) error %d\n", elemId, rc);
    }

    return ret;
    
}

/**************************************************************************************/
/*! \fn ProcGetElemMsg(Object obj, uint8_t * data, uint32_t size)
 *
 *      param[in] obj	- INTEGRITY Connection to be used for transfers
 *	param[in] data	- pointer to the message
 *	param[in] size	- number of bytes in the message
 *
 *  \par Description:	  
 *   Process a get element request message by parsing the message and sending the value
 *   of requested datapool element.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static int32_t ProcGetElemMsg(int8_t socket_fd, pid_t tid, uint8_t ImComponent, uint8_t * data, uint8_t dataSz)
{
    gp_retcode_t rc;
    int32_t ret;
    uint16_t msgId;
    uint8_t * pMsg;
    uint32_t * pBufId = (uint32_t *)malloc(sizeof(uint32_t));
    uint16_t elemId;
    GP_DATATYPES_T elemType;
    int32_t elemLen;
    int offset;

    /* Get element id from get element request message */
    gp_Read16bit(&elemId, &data[0]);

    /* Get element type */
    rc = GetElemInfo(elemId, &elemType, &elemLen);
    if(rc != GP_SUCCESS) {
	return -1;
    }

    /* Compose get element response message into buffer */
    ret = 0;
    msgId = GetElemRes;
    switch(elemType)
    {
	/*case GP_UINT8:
	    {
		uint8_t temp;
		GetElem(elemId, (void *)&temp);
		txMsg.MsgSize = Msg_GetBufSize(MSG_ID_SZ + ELEM_ID_SZ + 1);

		txMsg.pMsg = Msg_GetBuf(txMsg.MsgSize, pBufId, txMsg.Component);
		if(txMsg.pMsg == NULL) {
		    ret = -2;
		} else {
		    offset = gp_Store16bit(msgId, txMsg.pMsg);
		    offset += gp_Store16bit(elemId, (txMsg.pMsg+offset));
		    *((uint8_t *)txMsg.pMsg+offset) = temp;
		}
		break;
	    }*/
	case GP_INT16:
	    {
		int16_t temp;
        GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((int16_t *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_UINT16:
	    {
		uint16_t temp;
        GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((uint16_t *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_INT32:
	    {
		int32_t temp;
        GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((int32_t *)pMsg+offset) = temp;
		}
		break;
	    }
	case GP_UINT32:
	    {
		uint32_t temp;
        GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((uint32_t *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_INT64:
	    {
		int64_t temp;
        GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((int64_t *)pMsg+offset) = temp;
        }
		break;
	    }

	case GP_UINT64:
	    {
		uint64_t temp;
		GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((uint64_t *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_FLOAT:
	    {
		float temp;
		GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((float *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_DBL:
	    {
		double temp;
		GetElem(elemId, (void *)&temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            *((double *)pMsg+offset) = temp;
        }
		break;
	    }
	case GP_STRING:
	    {
		char temp[MAX_FNAME_LEN];
		GetElem(elemId, (void *)&temp[0]);
        int len = strlen(temp);

        pMsg = Msg_GetBuf(dataSz, pBufId, MAIN_COMPONENT);
        if(pMsg == NULL) {
            ret = -2;
        } else {
            offset = gp_Store16bit(msgId, pMsg);
            offset += gp_Store16bit(elemId, (pMsg+offset));
            strncpy((((char *)(pMsg))+offset),&temp[0],len);
        }
		break;
	    }
	default:
	    ret = -3;
	    break;
    }

    if(ret != 0) {
	Msg_FreeBuf(dataSz, *pBufId, MAIN_COMPONENT);
    free(pBufId);
	return ret;
    }

    /* Send the message */
    rc = TxMsg(socket_fd, tid, ImComponent, pMsg, dataSz, true);
    Msg_FreeBuf(dataSz, *pBufId, MAIN_COMPONENT);
    free(pBufId);
    if(rc != GP_SUCCESS) {
	return -4;
    }

    return 0;
    
}
