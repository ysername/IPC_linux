/**************************************************************************************/
/*!
 *    \file     msg_def.h
 *
 *    \copyright (c) Copyright 2016, Yazaki North America, All Rights Reserved
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
 *    \brief    Header file containing the public interfaces and definitions for the  
 *				FF HUD IPC messages.
 *
 *    \author   E. Gunarta, D. Kageff
 *
 *    \version  $Revision: 1.14 $
 *  
 *  Modification history:
 *   $Log: msg_def.h  $
 *   Revision 1.14 2017/07/20 13:52:13CDT Daniel Kageff (10011932) 
 *   1) Added IPC message field offset definitiions
 *   2) Edited comments.
 *   Revision 1.13 2017/03/30 11:16:04EDT Eudora Gunarta (10031829) 
 *   Changes for adding uds response and sleep request messages.
 *   Revision 1.12 2017/03/17 16:02:06EDT Eudora Gunarta (10031829) 
 *   Changes related to adding VP_SwVersionInfo SPI message handling.
 *   Revision 1.11 2017/03/15 14:10:35EDT Eudora Gunarta (10031829) 
 *   Changed ID of a message.
 *   Revision 1.10 2017/03/15 10:07:37EDT Eudora Gunarta (10031829) 
 *   Changed ID name of some messages.
 *   Revision 1.9 2017/02/06 16:40:46EST Eudora Gunarta (10031829) 
 *   Heartbeat fixes/improvements:
 *   Added HMI datapool task hearbeat response.
 *   Revision 1.8 2016/12/12 17:06:49EST Daniel Kageff (10011932) 
 *   Added 'set initialization data' operation command type definitions.
 *   Revision 1.7 2016/11/30 15:00:39EST Eudora Gunarta (10031829) 
 *   Updates for dynamic prewarp.
 *   Revision 1.6 2016/11/09 10:00:04EST Eudora Gunarta (10031829) 
 *   Added message for video ready signal.
 *   Revision 1.5 2016/10/24 12:20:52EDT Eudora Gunarta (10031829) 
 *   Added VP hearbeat msg (SystemStatusMsg) to msg list.
 *   Revision 1.4 2016/10/24 10:29:16EDT Daniel Kageff (10011932) 
 *   Added message definitions
 *   Revision 1.3 2016/09/28 14:32:29EDT Eudora Gunarta (10031829) 
 *   Replaced CanMsg with BCanSigDataMsg and CCanSigDataMsg, deleted EthMsg.
 *   Revision 1.2 2016/09/15 14:58:45EDT Eudora Gunarta (10031829) 
 *   Updatad list of messages.
 *   Revision 1.1 2016/07/07 14:58:05EDT Daniel Kageff (10011932) 
 *   Initial revision
 *   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Common/project.pj
 *
 */
/***************************************************************************************/
#ifndef MSG_DEF_H
#define MSG_DEF_H

/*****************************************************************************/
/*    I N C L U D E   F I L E S                                              */
/*****************************************************************************/  
#ifndef _CMD_CONN_H
#include "cmd_conn_L.h"
#endif
/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 
/* Msg priority */
#define PRIORITY_LOW (1)
#define PRIORITY_MEDIUM (127)
#define PRIORITY_HIGH (255)

/* Msg Id size in bytes */
#define MSG_ID_SZ (2)

/*****************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                            */
/*****************************************************************************/
/* Msg Id */
typedef enum MsgId {
    MsgIdMin = 0,

    HbtReq = MsgIdMin,
    HbtImAsIntTsk,  	/*  1: */
    HbtPmAsIntTsk,  	/*  2: */
    HbtEmAsIntTsk,  	/*  3: */
    HbtDmAsIntTsk,  	/*  4: */
    HbtImAsReqTsk,  	/*  5: */
    HbtImAsWrkTsk2, 	/*  6: */
    HbtHmIntTsk,    	/*  7: */
    HbtHmDplTsk,    	/*  8: */
    SpiTxReq,       	/*  9: */
    GetElemReq,    		/* 10: */
    GetElemRes,    		/* 11: */
    SetElemReq,     	/* 12: */
    PoolCopyReq,  		/* 13: */
    PoolCopyRes,  		/* 14: */
    BCanSigDataMsg, 	/* 15: */
    CCanSigDataMsg, 	/* 16: */
    OpCmdMsg,       	/* 17: General command message/response */
    SystemStatusMsg,	/* 18: VP heartbeat message */
    VideoReady,     	/* 19: Video ready signal to be sent to VP */
    MirrorStatusMsg,	/* 20: Mirror status (position/error) sent by VP */
    VpSwVersionMsg, 	/* 21: Contents of VP_SwVersionInfo SPI message */
    UdsRequestMsg, 		/* 22: UDS response message from VP */
    UdsResponseMsg, 	/* 23: UDS response message from VP */
    SleepRequestMsg, 	/* 24: Sleep request message from VP */
    AppStatusMsg,      /* 25: Aplication status message. Uses ::APP_STAT_CODE_T and :: APP_STAT_TARG from cmd_conn.h*/
    MsgIdMax = AppStatusMsg,
    MsgIdInvalid,
} MsgId;

/*! OpCmdMsg subcommand types enum */
typedef enum {
	CmdMsgVersionInfo,			/*!< Get/return version and configuration information */
	CmdMsgSetOpMode,			/*!< Set operating mode */
	CmdMsgSetInitData,			/*!< Set initial datapool item values.  Format: <datapool_id><value>,... */
	CmdMsgMaxIds
} OpCmdMsgTypes_t;


/*! OpCmdMsg subcommand message definitions */
/* Set operation mode message (CmdMsgSetOpMode) */
typedef enum {
	OPMODE_NORMAL = 0,		/*!< Normal operation mode */
	OPMODE_DEMO,			/*!< Demo operation mode.  One argument, <type> */
	OPMODE_TSTPATT,			/*!< Display test pattern.  One arg, <test_pattern_id> */
	OPMODE_HMIDEV,
    MAX_OPMODES
} SetOpmodeTypes_t;

/* Misc. operation command definitions */
#define OPCMD_HDR_SZ 2			/*!< Op cmd header size in bytes */
#define OFFSET_NUM_ITEMS 1		/*!< Buffer offset to the number of datapool items */
#define OPCMD_DP_ID_SZ   2
#define OPCMD_DP_ITEM_SZ 6		/*!< Datapool init item size in bytes */

/*! HbtReq message definitions */
#define MSG_HBTREQ_SZ  2
#define MSG_HBTREQ_MODE__OFFSET 0
#define MSG_HBTREQ_MODE_IDX  (MSG_ID_SZ)
#define MSG_HBTREQ_MODE_SZ  1
#define MSG_HBTREQ_MODE_0  0
#define MSG_HBTREQ_MODE_1  1
#define MSG_HBTREQ_MODE_2  2

/*! Common definitions for hearbeat response messages */
#define MSG_HBTRES_SZ  2
#define MSG_HBTRES_MODE_OFFSET 0
#define MSG_HBTRES_STATUS_IDX  (MSG_ID_SZ)
#define MSG_HBTRES_STATUS_SZ  1
#define MSG_HBTRES_STATUS_0  0
#define MSG_HBTRES_MODE_IDX  (MSG_HBTRES_STATUS_IDX + MSG_HBTRES_STATUS_SZ)
#define MSG_HBTRES_MODE_SZ  1
#define MSG_HBTRES_MODE_0  0
#define MSG_HBTRES_MODE_1  1
#define MSG_HBTRES_MODE_2  2

/*! AppStatusMsg definitions*/
#define MSG_STAT_OP     (0)
#define MSG_STAT_ID     (1)
#define MSG_STAT_CODE   (2)
#define MSG_STAT_DATLEN (MSG_STAT_CODE+1)
#define MSG_STAT_SZ     (2 + MSG_STAT_DATLEN)

/*! SpiTxReq message definitions */
#define SPI_HEADER_SZ 4			/*!< Size in bytes of a SPI message header */
#define MSG_SPITXREQ_MIN_SZ (SPI_HEADER_SZ + 1)

/*! Common definitions for a unpacked message from the IPC message payload.  These offsets
	are relative to the end of the IPC message ID field. */
#define IPC_SPI_ID		0					/*!< Offset to SPI message ID field */
#define IPC_SPI_LEN		(IPC_SPI_ID + 2)	/*!< Offset to SPI message length field */
#define IPC_SPI_DATA	(IPC_SPI_LEN + 2)	/*!< Offset to start of SPI message payload data */

		 
/******************************************************************************
*  Function Name: 
*
*  Description: 
*
*  Input(s):    
*
*  Outputs(s):  
*
*  Returns:     
******************************************************************************/

#endif /* MSG_DEF_H */
