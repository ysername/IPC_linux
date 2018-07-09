/**************************************************************************************/
/*!
 *    \file    	spi_common_config.h
 *
 *    \copyright (c) 2013 - 2016 Yazaki North America
 *
 *    \brief 	SPI Library Common Configuration - spi_common_config.h
 *		This file contains common definitions required by the SPI communications 
 *	    software residing on both the the VP and GP.  It is generated from the 
 *      template file "spi_common_config_h.txt" that is supplied with each 
 *      release of the SPI communications library.
 *  
 *      NOTE: THE SAME VERSION OF THIS FILE **MUST** BE USED ON BOTH THE VP 
 *      AND GP.  WHEN A CHANGE IS MADE IN THIS FILE THE SPI COMMUNICATION 
 *      SUBSYSTEMS ON BOTH NODES, INCLUDING THE SPI COMMUNICATIONS LIBRARY, 
 *      MUST BE RE-COMPILED TO ENSURE EACH NODE IS CONFIGURED CORRECTLY!!!
 *
 *      NOTE: DO NOT CHANGE THE NAMES OF THE PARAMETERS IN THIS FILE, ONLY PARAMETER
 *      VALUES MAY BE CHANGED.
 *
 *    \author   G. Palarski, D. Kageff
 *
 *    \version	$Revision: 1.16 $  
 *				$Log: spi_common_config.h  $
 *				Revision 1.16 2017/05/16 16:42:15CDT Carlos Serrato Moreno (10013229) 
 *				- Add more IPC messages definitions
 *				Revision 1.15 2017/03/30 07:12:15CST Eudora Gunarta (10031829) 
 *				Changes for adding sleep request message that is sent from VP to GP.
 *				Revision 1.14 2017/03/22 15:57:28EDT Sergio Espinoza Lopez (10012599) 
 *				type fiels was missing from spi signal timeout message
 *				Revision 1.13 2017/03/21 15:31:07CST Eudora Gunarta (10031829) 
 *				Added definitions for GP_UnitStatusMsg.
 *				Revision 1.12 2017/03/21 10:56:40EDT Isaias Acosta Martinez (10014447) 
 *				Added definitions for UDS manager IPC messages
 *				Revision 1.11 2017/03/20 20:23:13CST Sergio Espinoza Lopez (10012599) 
 *				* CAN signal timeout reporting infrastructure
 *				* Update to use common definitions for IPC messages
 *				Revision 1.10 2017/03/17 15:30:51CST Eudora Gunarta (10031829) 
 *				Added macros for GP_UnitStatusMsg message, correction for a VP_SwVersionInfo macro.
 *				Revision 1.9 2017/02/15 18:31:03EST Daniel Kageff (10011932) 
 *				Added comments to CAN def message offsets
 *
 * 	  \copyright (C) 2016 Yazaki North America
 *
 * License: Yazaki License, All Rights Reserved
 *
 * This document is protected by the copyright law of the United States and
 * foreign countries.  The work embodied in this document is confidential and
 * constitutes a trade secret of Yazaki North America, Inc.  Any copying of
 * this document and/or any use or disclosure of this document or the work
 * embodied in it without the written authority of Yazaki North America, Inc.,
 * will constitute copyright and trade secret infringement and will violate the
 * rights of Yazaki North America, Inc.
 */
/***************************************************************************************/
/* Standard multi-include avoidance */
#ifndef __SPI_COMMON_CONFIG_H__
#define __SPI_COMMON_CONFIG_H__


/******************************************************************************/
/*     I N C L U D E   F I L E S                                              */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/

/*
** YZ_SPI_TFR_RATE_MS  - The periodic rate of SPI transfers in milliseconds.
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_TFR_RATE_MS     (10u)

/*
** YZ_SPI_PKT_SIZE - The size, in bytes, of packets transmitted during each SPI transfer.
**   The size must be divisible by 2.
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
//#define  YZ_SPI_PKT_SIZE (512u)
#define  YZ_SPI_PKT_SIZE (256u)  

/*
** YZ_SPI_CLOCK_FREQ_HZ - The SPI clock frequency, in HZ, use for each SPI transfer. 
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_CLOCK_FREQ_HZ (1000000ul)

/*
** YZ_SPI_MAX_MSG_SIZE - The maximum message size, in bytes, that can be sent by SPI clients.  
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_MAX_MSG_SIZE (128u)

/*
** YZ_SPI_PKT_MAX_MSG_SIZE -  The maximum message size, in bytes, that can be sent in a 
**   single packet (does not include the Packet or message headers to be sent with the message).
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_PKT_MAX_MSG_SIZE (128u)

/*
** YZ_SPI_PACKET_PAD_BYTE - The byte value used pad (fill) the unused bytes in each packet.  
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_PKT_PAD_BYTE (0x5Au) 

/*
** YZ_SPI_TFR_WD_TIMEOUT_MS - Time in milliseconds the VP waits after requesting a SPI 
**   transfer before declaring a timeout.  Should be at least 3X the YZ_SPI_TFR_RATE_MS. 
**   APPLICABLE TO: VP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_TFR_WD_TIMEOUT_MS  (60u) 

/*	  Might need 400
** YZ_SPI_TFR_RETRY_LIMIT - The number of times a SPI transfer is retried without success 
**   before a fatal error condition is declared. 
**   APPLICABLE TO: VP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_TFR_RETRY_LIMIT (10) 

/*
** YZ_SPI_TPL_ENABLE - Enables support for the transport layer.  Both sides must be configured 
**   the same.  A value of 0 disables, non-zero enables. 
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_TPL_ENABLE (0) 

/*
** YZ_SPI_TPL_MAX_MSG_SIZE - The maximum size, in bytes, of a transport layer message within 
**   a single packet.  Note that this parameter is applicable only when YZ_SPI_TPL_ENABLE is
**   non-zero.
**   APPLICABLE TO: VP and GP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_TPL_MAX_MSG_SIZE (0) 

/*
** YZ_SPI_BOOT_WD_TIMEOUT_MS - Time in milliseconds the VP waits for the GP to perform the first 
**   SPI transfer following re-boot. 
**   APPLICABLE TO: VP.
**   NOTE: See the Hydra SPI Protocol Specification for more information on this parameter.
*/
#define  YZ_SPI_BOOT_WD_TIMEOUT_MS   (300000u) 

/*
** YZ_COMMON_CONFIG_VERSION - Specifies the version of the common configuration file this
**   file is compliant with.  The value of this macro is used to confirm the version of the 
**   this file is compatible with the SPI communication library that uses it.  Version numbers 
**   start with 1 and increment by 1 for each new version.  
**   APPLICABLE TO: VP and GP.
*/
#define  YZ_COMMON_CONFIG_VERSION (1) 

/*
** YZ_SPI_REQUEST_RETRY_COUNT - Counter to delay and generate SPI Retry Request pulse to GP  
**    						 for SPI Transfer.
**   
**   
*/
#define  YZ_SPI_REQUEST_RETRY_COUNT  (50u)


/*********************************************************************************
 *   VP_BCanSigDataMsg / VP_CCanSigDataMsg CAN signl data 
 */

/**********************************************************************************
 *   ____________________________________________________________________________
 *   |           |                |         |            |          |            |
 *   |   <-- SIG ID (2bytes) ---> |   <---- SIG DATA (up to 4 bytes) -------->   |
*    |           |                |         |            |          |            |
*    -----------------------------------------------------------------------------
*/

#define SIGNAL_ID_IDX        0
#define SIGNAL_ID_SZ         2

#define SIGNAL_DATA_IDX      2
#define SIGNAL_DATA_SZ_MAX   4



/*********************************************************************************
 *   GP_CanNetDefnMsg message definitions
 */

/**************************************************************************
 *    ____________________________________________________
 *   |                       |     |     |    |           |
 *   | <- DATA ID(1 byte)->  |  <--  Value (4 bytes)-->   | 
 *   |_______________________|_____|_____|____|___________|
 *
 */

#define  NM_DATA_ID_IDX   0
#define NM_DATA_ID_SZ     1

#define NM_DATA_IDX       1
#define NM_DATA_SZ        4  

#define NM_DEF_MSG_SZ 5 

//NM Data ID definitions 
#define NM_DEF_NUM_PARAMS 		(7u)
#define NM_TIMEOUT              0x01		/*!< Network timeout parameter ID */
#define NM_REMOTE_TIMEOUT       0x02		/*!< Network remote timeout parameter ID */
#define NM_WAITBUSTIMEOUT       0x03   		/*!< Network bus timeout parameter ID */
#define NM_PARTIAL_NET_ENABLE   0x04		/*!< Network partial network support? enable */
#define NM_CHANNEL              0x06  		/*!< Network channel? */
#define NM_NUM_MSG_DEFS         0x07		/*!< Number of messages defined (supported) for this network. */
#define NM_NUM_SIG_DEFS         0x08		/*!< Number of message signals defined (supported) for this network. */


/*********************************************************************************
 *   GP_CanMsgDefnMsg message definitions
 */

//MSG DATA PID
#define CAN_DEF_VER			0x01		/*!< CAN definitions version */
#define CAN_MSG_DEFN		0x02		/*!< Base CAN message definition */
#define CAN_MSG_SIGNAL		0x03		/*!< CAN message signal basic definition */
#define CAN_SIG_CONV		0x04		/*!< CAN message signal conversion factor definition */
#define CAN_MSG_RAW			0x05		/*!< Always send the 'raw' message data for the indicated message ID */
#define CAN_MSG_DEFN_EXT	0x06		/*!< Extended CAN message definition.  This PID includes N number of signal definitions. */


/**********************************************************************************
* CAN DATA MSG  (GP_CanMsgDefnMsg)
*   -----------------------------------------------------------------------------
*   |                         |        |        |        |      |
*   | <-- MSG DEF( 0x02 ) --> |  <-- CAN MSG ID (4 bytes) -->   | <--  -->
*   |_________________________|________|________|________|______|_______________
*/

// CAN MSG NORMAL/RAW definitions data fields

#define CANMSGID_IDX		    0
#define CANMSGID_SZ		        4

#define CANMSGDLC_IDX 	        4
#define CANMSGDLC_SZ		    1

#define CANMSGTYPE_IDX		    5
#define CANMSGTYPE_SZ		    1

#define CANMSGBUS_IDX		    6
#define CANMSGBUS_SZ		    1

#define CANMSGRATE_IDX		    7
#define CANMSGRATE_SZ		    2

#define CANMSGTIMEOUT_IDX		9
#define CANMSGTIMEOUT_SZ		1


#define CANMSG_DEF_MSG_SZ ( CANMSGID_SZ \
                            + CANMSGDLC_SZ \
                            + CANMSGTYPE_SZ \
                            + CANMSGBUS_SZ \
                            + CANMSGRATE_SZ \
                            + CANMSGTIMEOUT_SZ )
 



/**********************************************************************************
* CAN SIGNAL DEFINITIONS  (GP_CanMsgDefnMsg)
*   -------------------------------------------------------------------------------
*   |                               |      |    |   |     |          |         |
*   | <-- SIGNAL DEF ( 0x04 ) -->   |   <-- CAN SIG ID--> | <-START->|<--DLC-->|
*   |______________________________ |______|____|___|_____|__________|_________|
*/


// CAN MSG Signal definitions data fields

#define CANSIGID_IDX		    0
#define CANSIGID_SZ		        2

#define CANSIGSTARTBIT_IDX		2
#define CANSIGSTARTBIT_SZ		1

#define CANSIGDLC_IDX		    3
#define CANSIGDLC_SZ		    1

#define CANMSGSIGID_IDX	        4
#define CANMSGSIGID_SZ	        4

#define CANSIGBUSID_IDX	        8
#define CANSIGBUSID_SZ	        1

//sum all fields
#define CANSIG_DEF_MSG_SZ  ( CANSIGID_SZ \
                             + CANMSGSIGID_SZ \
                             + CANSIGSTARTBIT_SZ \
                             + CANSIGDLC_SZ \
							 + CANSIGBUSID_SZ)


/**************************************************************************
 *  VP_SwVersionInfo message definitions
 */

/**************************************************************************
 *    _____________________________________________________________________
 *   |  | | | | | | | | | | | | | | | |    |   |    |   |   |   |    |    |
 *   |  <--SW VERSION (16 bytes)-->   |  <--  SW PART NUMBER (8 bytes)--> |
 *   |__|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|____|___|____|___|___|___|___|_____| 
 *
 */
#define  VP_SW_VERSION_IDX         0
#define  VP_SW_VERSION_SZ          16

#define  VP_SW_PART_NUMBER_IDX     16
#define  VP_SW_PART_NUMBER_SZ      8

// sum all fields
#define SW_VERSION_MSG_SZ        (VP_SW_VERSION_SZ \
                               +  VP_SW_PART_NUMBER_SZ )

/**************************************************************************
 *
 *  GP_UnitStatusMsg message definitions
 */

/**************************************************************************
 */
#define UNIT_STATE_SZ                 1
#define UNIT_STATE_NORMAL             2
#define UNIT_STATE_SHUTTING_DOWN     10
#define UNIT_STATE_SHUTDOWN_COMPLETE 11

#define SOH_STATUS_SZ                   1
#define SOH_STATUS_NO_FAILURES          0
#define SOH_STATUS_COMP_CRASHED         1
#define SOH_STATUS_COMP_FAILED_TO_RESP  2

#define SOH_FC_ID_SZ     4

#define RST_REQ_SZ    1
#define RST_REQ_NONE  0
#define RST_REQ       1

#define GP_STATUS_MSG_SZ  ( UNIT_STATE_SZ \
                             + SOH_STATUS_SZ \
                             + SOH_FC_ID_SZ \
                             + RST_REQ_SZ )

/**************************************************************************
*  VP_CCanMirrorStatusMsg message definitions
*/

/*********************************************************************
*   _________________________
*   |        |      |       |
*   |  TYPE  |  <--DATA --> |       
*   |________|_____ |_______|
*        0       1       2
*
*    For report types:
*    0x01: Mirror Position
*    0x02: Error
*    
*   VP Mirror Position Message
*   ________________________
*   |         |     |      |
*   | (0x01)  |  POSITION  |
*   |_________|_____|______|
*       0        1      2 
**/


/* Mirror status message */
#define MIRROR_STATUS_MSG_TYPE_IDX  0
#define MIRROR_STATUS_MSG_DATA_IDX   1

#define MIRROR_STATUS_MSG_POSITION  0x01
#define MIRROR_STATUS_MSG_ERROR     0x02

#define MIRROR_STATUS_LIMIT_SWITCH_ERROR    0x01
#define MIRROR_STATUS_ERROR_OTHER           0x02


#define MIRROR_STATUS_MSG_SZ   3

/**************************************************************************
*  VP_CanMsgTimeoutStatus message definitions
*/

/**********************************************************************************
*   ------------------------------------------------------
*   |             |     |     |     |     |              |
*   | <-- CHAN--> |<--MSG ID(4 bytes) --> | <---Type---> |
*   |_____________|_____|_____|_____|_____|______________|
*/

#define CAN_MSG_TIMEOUT_SZ 6
 
#define CAN_MSG_TIMEOUT_CHAN_IDX  0
#define CAN_MSG_TIMEOUT_CHAN_CHASSIS    1
#define CAN_MSG_TIMEOUT_CHAN_BODY       2

/*channel ID*/
#define CAN_MSG_TIMEOUT_ID_IDX    1
#define CAN_MSG_TIMEOUT_ID_SZ     4

/* type */
#define CAN_MSG_TIMEOUT_TYPE_IDX    5
#define CAN_MSG_TIMEOUT             1
#define CAN_MSG_NORMAL              2

/**************************************************************************
*  VP_CanSignalTimeoutStatus message definitions
*/
/**********************************************************************************
*   -------------------------------------------------------------
*   |                   |          |             |              |
*   | <-- CCAN/BCAN --> |<--Signal ID(2 bytes)-->| <---Type---> |
*   |___________________|__________|_____________|______________|
*/

#define CAN_SIG_TIMEOUT_SZ 4
 
/*Channel*/
#define CAN_SIG_TIMEOUT_CHAN_IDX        0
#define CAN_SIG_TIMEOUT_CHAN_CHASSIS    1
#define CAN_SIG_TIMEOUT_CHAN_BODY       2

/*signal ID*/
#define CAN_SIG_TIMEOUT_ID_IDX    1
#define CAN_SIG_TIMEOUT_ID_SZ     2

/* type */
#define CAN_SIG_TIMEOUT_TYPE_IDX    3
#define CAN_SIG_TIMEOUT             1
#define CAN_SIG_NORMAL              2

/**************************************************************************
*  VP_SleepRequesMsg message definitions
*/
/**********************************************************************************
*/

#define SLEEP_REQ_MSG_SZ  1
#define SLEEP_REQ_MSG_IDX  2

/* Values for sleep mode field */
#define SLEEP_MODE_1  1
#define SLEEP_MODE_2  2


/**************************************************************************
*  GP_CanDefnVerMsg message definitions
*/
/**********************************************************************************
*/
#define DB_GP_CCANVER_MSG_IDX 	0
#define DB_GP_BCANVER_MSG_IDX 	2
#define DB_GP_VER_MSG_SZ  	4


/**************************************************************************
*  VP_CanDefnVerMsg message definitions
*/
/**********************************************************************************
*/
#define DB_VP_CCANVER_MSG_IDX 			0
#define DB_VP_BCANVER_MSG_IDX 			2
#define DB_VP_MAX_MSGNUM_MSG_IDX	4
#define DB_VP_MAX_SIGMSG_MSG_IDX	5
#define DB_VP_MAX_SIG_MSG_IDX		6
#define DB_VP_VER_MSG_SZ  			7

/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/

/* 
** Enum: VP_SPI_TX_MESSAGES
**	Defines the messages ids (by name) that are transmitted by VP SPI clients.
**  Edit the names in this enumeration to define the message id to be supported.
**
**  NOTE: CONSTANTS "VP_TX_MIN_MSG_ID_MINUS_1" AND "VP_TX_MAX_MSG_ID_PLUS_1" ARE 
**  PROVIDED SO THEY MAY BE USED TO CHECK THE VALIDITY OF MESSAGES IDS (VALID 
**  MESSAGE IDS MUST BE BETWEEN THESE TWO VALUES).  THEREFORE, THESE TWO NAMES 
**  MUST BE AT THE BEGINNING AND END OF THE ENUMERATION!!! 
*/
typedef enum VP_SPI_TX_MESSAGES
{
	VP_TX_MIN_MSG_ID_MINUS_1 = 0,   	/* Placeholder since message ids must start at 1 */
    VP_SystemStatusMsg = 0x0001,        /* MessageID 1 - VP status */
    VP_CanDefnVerMsg = 0x0002,   	/* MessageID 2 - Request updated CAN definitions */
    VP_BCanSigDataMsg = 0x0003,     	/* MessageID 3 - Send BCAN signal data */
    VP_CCanSigDataMsg = 0x0004,     	/* MessageID 4 - Send CCAN signal data */
    VP_CCanMirrorStatusMsg = 0x0005,	/* MessageID 5 - Mirror status message */
	VP_SwVersionInfo = 0x0006,			/* MessageID 6 - VP software version information */
    VP_CanMsgTimeoutStatus = 0x0007,    /* MessageID 7 - CAN timeout notification */
    VP_CanSignalTimeoutStatus = 0x0008, /* MessageID 8 - CAN signal timeout notification */
    VP_Printf = 0x0009,                 /* MessageID 9 */
    VP_UdsResponseMsg = 0x000A,         /* MessageID 10 - UDS response message  */
    VP_SleepRequesMsg = 0x000B,         /* MessageID 11 - Sleep request message */
	VP_TX_MAX_MSG_ID_PLUS_1
} VP_SPI_TX_MESSAGES;

/* 
** Enum: GP_SPI_TX_MESSAGES
**	Defines the messages ids (by name) that are transmitted by GP SPI clients.
**  Edit the names in this enumeration to define the message id to be supported.
**
**  NOTE: CONSTANTS "GP_TX_MIN_MSG_ID_MINUS_1" AND "GP_TX_MAX_MSG_ID_PLUS_1" ARE 
**  PROVIDED SO THEY MAY BE USED TO CHECK THE VALIDITY OF MESSAGES IDS (VALID 
**  MESSAGE IDS MUST BE BETWEEN THESE TWO VALUES).  THEREFORE, THESE TWO NAMES 
**  MUST BE AT THE BEGINNING AND END OF THE ENUMERATION!!! 
*/
typedef enum GP_SPI_TX_MESSAGES
{
	GP_TX_MIN_MSG_ID_MINUS_1 = 0,   	/* Placeholder since message ids must start at 1 */
    GP_UnitStatusMsg = 0x0001,          /* MessageID 1 - GP status */
    GP_CanNetDefnMsg = 0x0002,          /* MessageID 2 - CAN network parameter definitions. */
	GP_CanMsgDefnMsg = 0x0003,			/* MessageID 3 - CAN network message definitions. */
	GP_CanDefnVerMsg = 0x0004,			/* MessageID 3 - CAN network message definitions. */
    GP_VideoReadyMsg = 0x0005,          /* MessageID 5 */
    GP_UdsRequestMsg = 0x0006,          /* MessageID 6 - UDS request message  */
//    VP_CANCompassStatusMsg,         /* MessageID 4 */
//    VP_CANEngineInformation1Msg,    /* MessageID 5 */
//    VP_CANEngineInformation2Msg,    /* MessageID 6 */
//    VP_CANCruiseControl,            /* MessageID 7 */
//    VP_BluetoothVehicleData,        /* MessageID 8 */

    GP_TX_MAX_MSG_ID_PLUS_1
} GP_SPI_TX_MESSAGES;


/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/



#ifdef __cplusplus
}
#endif

#endif /* __SPI_COMMON_CONFIG_H__ */
