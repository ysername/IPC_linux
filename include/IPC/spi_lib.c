/*******************************************************************************/
/*!
*  \file	spi_lib.c		
*
*  \copyright	Copyright 2012
*               Yazaki North America
*               All Rights Reserved
*  	Unpublished, Yazaki North America Inc. All rights reserved. 
*  	This document is protected by the copyright law of the United States and 
*  	foreign countries.  The work embodied in this document is confidential and 
*  	constitutes a trade secret of Yazaki North America, Inc.  Any copying of 
*  	this document and any use or disclosure of this document or the work 
*  	embodied in it without the written authority of Yazaki North America, Inc., 
*  	will constitute copyright and trade secret infringement and will violate the
*  	rights of Yazaki North America, Inc.
*
*  \brief
*   The SPI communications library code resides in this file.  The SPI 
*   library supports communication between the VP (vehicle processor) and 
*   GP (graphics processor) per the protocol defined in the "Hydra SPI 
*   Communication Protocol Specification" document.  For more information
*   on the library refer to the "Hydra SPI Library Design Specification"
*   document.
 *
 *  \author		G. Palarski
 *
 *  \version	$Revision: 1.4 $  
 *				$Log spi_lib.c $ 
*  ----------------------------------------------------------------------------
*	\todo Finish adding Doxygen instrumented comments.
*/

/******************************************************************************/
/*     I N C L U D E   F I L E S                                              */
/******************************************************************************/
#include <stddef.h>
#include <stdio.h>                                                                                       

#include <string.h>
#include <stdlib.h>
#include "spi_lib.h"
#include "spi_callbacks.h"

/******************************************************************************/
/*     C O N F I G U R A T I O N   P A R A M A T E R  C H E C K S             */
/******************************************************************************/

#if ((YZ_NODE_ID != VP_NODE) && (YZ_NODE_ID != GP_NODE))
   #error YZ_NODE configuration parameter is invalid.
#endif

/*!
	Macros defining the versions of node specific and common configuration
	files that are supported by this version of the library.
*/
#define  SUPPORTED_NODE_CONFIG_VERSION (1)		/*!< node specific config version supported */ 
#define  SUPPORTED_COMMON_CONFIG_VERSION (1) 	/*!< common config version supported */

/*
** Confirm that the user owned configuration files support the same configuration
** as the library.
*/
#if (YZ_COMMON_CONFIG_VERSION != SUPPORTED_COMMON_CONFIG_VERSION)
   #error YZ_COMMON_CONFIG_VERSION configuration parameter is invalid.
#endif

#if (YZ_NODE_CONFIG_VERSION != SUPPORTED_NODE_CONFIG_VERSION)
   #error YZ_NODE_CONFIG_VERSION configuration parameter is invalid.
#endif

#if ((YZ_SPI_PKT_SIZE != 128) && (YZ_SPI_PKT_SIZE != 256) && (YZ_SPI_PKT_SIZE != 512))
   #error YZ_SPI_PKT_SIZE must be 128, 256, or 512.
#endif

#if (YZ_SPI_PKT_MAX_MSG_SIZE > (YZ_SPI_PKT_SIZE/2)) 
   #error YZ_SPI_PKT_MAX_MSG_SIZE is too large.
#endif

#if (YZ_SPI_MAX_MSG_SIZE > 65535) 
   #error YZ_SPI_MAX_MSG_SIZE is too large.
#endif

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/

#if (defined(SL_TEST) && defined(SL_PRINTF_DEBUG))
   /* Support printf output for testing & debug */
   #define PRINTF printf
#else
   /* Default is no printf output */
   #define PRINTF 
#endif

#if (defined(SL_LOGMSG))
   /* LogMsg must be provided external to the SPI lib so output can be sent to right place (screen, log file, etc.) */
   void LogMsg(char buf[]);
   char log_buff[150];
#endif

/*!
   SUPPORTED_SPI_PROTOCOL_VERSION - Defines the version of the "Hydra SPI 
   Protocol Specification" document that this version of the SPI library 
   is compatible with.  Decimal points are not included in the version 
   (ex: 13 will be returned when the SPI library is compatible with version 
   1.3 of the Hydra SPI Protocol Specification).
*/
#define SUPPORTED_SPI_PROTOCOL_VERSION   ((U16)(17))

/*!
	MAX_PKT_SN - maximum value of SN that can be used in a packet.
*/
#define  MAX_PKT_SN  (65535)

/*!
	PKT_HDR_BYTES - number of bytes in a packet header.   
*/
#define PKT_HDR_BYTES    (8)

/*!
	MSG_HDR_BYTES - number of bytes in a message header.    
*/
#define MSG_HDR_BYTES    (4)

/*
  Packet header offsets (from beginning of packet)
*/
#define  PKT_CRC_LSB       (0)		/*!< Offset of crc least sig byte in packet */
#define  PKT_CRC_MSB       (1)		/*!< Offset of crc most sig byte in packet */
#define  PKT_MSG_COUNT_LSB (2)		/*!< Offset of msg count least sig byte in packet */
#define  PKT_MSG_COUNT_MSB (3)		/*!< Offset of msg count most sig byte in packet */
#define  PKT_SN_LSB        (4)		/*!< Offset of SN least sig byte in packet */
#define  PKT_SN_MSB        (5)		/*!< Offset of SN most sig byte in packet */
#define  PKT_NACK_LSB      (6)		/*!< Offset of NACK least sig byte in packet */
#define  PKT_NACK_MSB      (7)		/*!< Offset of NACK most sig byte in packet */

/*
  Message header offsets (from beginning of header)
*/
#define  MSG_ID_LSB  (0)	/*!<  offset of msg id least sig byte in message hdr */
#define  MSG_ID_MSB  (1)	/*!<  offset of msg id most sig byte in message hdr	*/
#define  MSG_LEN_LSB (2)	/*!<  offset of msg length least sig byte in message hdr */
#define  MSG_LEN_MSB (3)	/*!<  offset of msg length id most sig byte in message hdr */
  
#define OFFSET_LAST_PKT_BYTE  (YZ_SPI_PKT_SIZE - 1)	/*!< offset of last byte in packet buffer. */ 

/*!
 Macros for get the least and most significant byte of the input argument.
 The input argument should be of type U16.
*/
#define GET_MSB(x)   ((U8) ((x) / 256))		/*!< returns the most sig byte */
#define GET_LSB(x)   ((U8) ((x) & 0xFF))	/*!< returns the least sig byte */

#define STAT_CLAMP_VAL  ((U32) 0xFFFFFF5A)		/*!< SPI comm statistics clamp value */
/*!
 Macros to increment, with clamp, a statistics field.  A statistics counter
 is clamped at 0xFFFFFF5A (prevents overflow and is an unlikely value 
 in a corruption scenario).
*/
#define INCREMENT_STAT(stat)  (stat = ((stat<STAT_CLAMP_VAL) ? ++stat : stat))

/*
** MESSAGE TYPE ID's
**    ID_STATUS_MSG - the ID of a status message in a packet.
**    ID_TPL_MSG    - the ID of a TPL message in a packet.
*/
#define ID_STATUS_MSG  ((U16)0)
#define ID_TPL_MSG     ((U16)0xFFFF)

/*
** LAST_MSG_DISPATCH_TABLE_ID - The index of the last Rx message handler function
** in msg_dispatch_table.
*/
#define LAST_MSG_DISPATCH_TABLE_ID (sizeof(msg_dispatch_table)/sizeof(MsgHandler)-2)

/*
** STATUS MESSAGE ID's 
**    ID_COMM_RESET_MSG - ID of the communications reset status message
**    ID_PROTOCOL_VERSION_MSG  - ID of the protocol vesion status message
**    ID_TPL_TX_ABT_MSG - ID of the transport layer Tx abort messsage
**
**    LEN_COMM_RESET_MSG - length, in bytes, of the COMM RESET msg
**    LEN_PROTOCOL_VERSION_MSG - length, in bytes, of the COMM RESET msg
**    LEN_TPL_TX_ABT_MSG - length, in bytes, of the COMM RESET msg
**    
*  NOTE: The values of these messages are dictated by the SPI Protocol Specification
*/
#define ID_COMM_RESET_MSG        (1)
#define ID_PROTOCOL_VERSION_MSG  (2)
#define ID_TPL_TX_ABT_MSG        (3)
#define LEN_COMM_RESET_MSG       (5)
#define LEN_PROTOCOL_VERSION_MSG (2)
#define LEN_TPL_TX_ABT_MSG       (5)

#define YZ_SPI_TPL_MAX_SIM_TFRS   (2)

/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/

/*
 * Typedef: tx_info 
 *
 * 	Record holding Tx related information 
 */
typedef struct TX_INFO
{
	int min_msg_id;      /* Min msg_id that can be Tx'd */ 
	int max_msg_id;      /* Max msg_id that can be Tx'd */
	int next_sn;         /* Sequence number to Tx in next packet */
	int next_nack;       /* NACK value to Tx in next packet */ 
    int msg_count;       /* Number of messages loaded in packet */
    int load_index;      /* Index to next byte to load in packet buffer */
    int go_back;         /* Maintains what packet to Tx in "go back" scenarios */ 
    U16 last_sn_txd;     /* The SN sent in the last packet Tx'd */ 
    U8 *oldest_pkt_ptr;  /* Pointer to oldest packet (next to overwrite when a new packet loaded for Tx) */
    U8 *newest_pkt_ptr;  /* Pointer to newest packet */ 
    U8 *pkt_ptr;         /* Pointer to packet to load next */
    U8 comm_reset_flag;  /* a non-0 value indicates Tx of a COMM RESET status message has been requested */ 
    U8 comm_reset_reason; /* indicates reason for Txing COMM RESET message */
} TX_INFO;

/*
 * Typedef: rx_info 
 *
 * 	Record holding Rx related information 
 */
typedef struct RX_INFO
{
	int min_msg_id;      /* Min msg_id that can be Rx'd */
	int max_msg_id;      /* Max msg_id that can be Rx'd */
	int next_sn;         /* Sequence number expected in next Rx packet */
    int msgs_left;       /* Number of messages left to unload from Rx packet*/
    U8 *pkt_ptr;         /* Pointer to start of Rx packet */
    int unload_index;    /* Index to next byte to unload from Rx packet */
} RX_INFO;

/*
** Typedef: API_STATE
**
**  Defines the states which are used to check the order of calls made to the API. 
**  The SL_Initialize function can be called anytime to reset the library due
**  to an error condition.  Other API functions can be called only when the 
**  API state is appropriate.
*/
typedef enum API_STATE
{
	WAIT_INIT = 0,     /* SL_Initialize is valid */ 
    WAIT_GET_TX_PKT,   /* SL_GetPacketToTx is valid */
	WAIT_LOAD_START,   /* SL_LoadPacketStart is valid */ 
	LOADING,           /* SL_LoadPacketMsg, SL_LoadPacketStatusMsg and SL_LoadPacketFinish are valid */
	WAIT_UNLOAD_START, /* SL_UnloadPacketStart is valid */
	UNLOADING,         /* SL_UnloadPacketMsg is valid */
} API_STATE;


/*
** Typedef: MsgHandler
** 
**  Defines a type for Rx message handlers.
*/
typedef void (*MsgHandler)(int, U8 * const);


/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/
static int ComputeNextSN(int current_sn);
static U16 CalculateCrc(U8 * pBuf, int length);
static void SL_VPSpecificInit(void);
static void SL_GPSpecificInit(void);
static int UnloadStatusMsg(int *msg_id, int *msg_size_bytes, U8 **buff_ptr);
static void LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left);
static void SendCommResetMsg(void);

#if (YZ_SPI_TPL_ENABLE != 0)
   static void UnloadTPLMsg(int msg_length);
   static int  LoadOneTPLMsg(int msg_id, int msg_size_bytes, U8 *msg_data, int bytes_remaining, int seg_number);
   static void LoadTPLMsgs(void);
   static void LoadTPLMsgsFill(unsigned int fill_packet);
   static void TPLClearUnload(void);
   static void TPLCheckUnload(void);
   static void TPLInit(void);
   static void TPLAbortTxMsgs(void);
   static void TPLAbortRxMsgs(void);
   static void TPLIncomingTxAbort(int msg_id, U8 error_code);
   static int FindRxTableRec(int msg_id);
#endif

/******************************************************************************/
/*     M E M O R Y   A L L O C A T I O N                                      */
/******************************************************************************/

/*
** Variable: api_state
** Maintains the API state used to determine which API's may be called.
*/
static int api_state = WAIT_INIT;

/*
** Variable: tx_info
**   A structure containing information used for loading and transmitting packets.
*/
static TX_INFO  tx_info;

/*
** Variable: rx_info
**   A structure containing information used packet receiving and unloading packets.
*/
static RX_INFO  rx_info;

/*
** Variable: stats
**   A structure containing statistics information.
*/
static SL_STATS_RECORD stats;

/*
** Constant: msg_dispatch_table
**   This table contains a list of pointers to functions which are called to 
**   handle incoming "regular" messages.
*/
static const MsgHandler msg_dispatch_table[] =
{
   NULL,    /* msg id's of 0 are not allowed in regular messages, this is a placeholder */
   PS_RX_MSG_HANDLER_LIST
   NULL
};


/******************************************************************************/
/*     F U N C T I O N   D E F I N I T I O N S                                */
/******************************************************************************/
/*
** TODO: Should all msg id range checks be discarded?  If an invalid msg
** gets through may just increment a counter?
*/

/**************************************************************************************/
/*! \fn SL_Initialize(U8 *buffer1_ptr, U8 *buffer2_ptr, U8 reason_for_init)
 *
 *  \param[in] buffer1_ptr  - pointer to the first buffer being released
 *  \param[in] buffer2_ptr  - pointer to the second buffer being released
 *  \param[in] reason_for_init - reason why this function was called.  Constants that 
 *         						 can be used for this parameter can be found above (beginning with
 *         						 "RFI_").
 *  \par Description:	  
 *  Initializes the SPI library.  THIS FUNCTION MUST BE CALLED BEFORE ANY
 *  OTHER FUNCTION IN THE LIBRARY!  This function is also called when
 *  restarting SPI communications due to a fatal error condition.
 *
 *  This function "releases" buffers to the SPI library.  After being 
 *  released the buffers are managed by the library and can only be accessed 
 *  as defined via the library API.  The size of each buffer must be at least
 *  YZ_SPI_PKT_SIZE bytes.
 *
 *  \returns none 
 *
 *  \par Limitations/Caveats:
 *	None
 *
 *  \ingroup spi_public
 **************************************************************************************/
void SL_Initialize(U8 *buffer1_ptr, U8 *buffer2_ptr, U8 reason_for_init)
{
   api_state = WAIT_GET_TX_PKT;

   tx_info.next_sn = 0x1; 
   tx_info.next_nack = 0x00;    
   tx_info.pkt_ptr = NULL;
   tx_info.go_back = 0;
   tx_info.newest_pkt_ptr = buffer1_ptr;
   tx_info.oldest_pkt_ptr = buffer2_ptr;
   tx_info.comm_reset_flag = 1;
   tx_info.comm_reset_reason = reason_for_init;

#if (YZ_NODE_ID == VP_NODE)
   SL_VPSpecificInit();
#else
   SL_GPSpecificInit();
#endif

   /*
   ** Set Rx SN to 0 to indicate that any SN should be accepted in the first packet received 
   ** following initialization.
   */
   rx_info.next_sn = 0x00;

#if (YZ_SPI_TPL_ENABLE != 0)
   TPLInit();
#endif
}


/**************************************************************************************/
/*! \fn SL_VPSpecificInit(void)
 *
 *  param None
 *
 *  \par Description:	  
 *  Initializes variables with information specific to VP node.
 *
 *  \returns none 
 *
 *  \par Limitations/Caveats:
 *	None
 *
 **************************************************************************************/
static void SL_VPSpecificInit(void)
{
   tx_info.min_msg_id = VP_TX_MIN_MSG_ID_MINUS_1 + 1;
   tx_info.max_msg_id = VP_TX_MAX_MSG_ID_PLUS_1 - 1; 

   rx_info.min_msg_id = GP_TX_MIN_MSG_ID_MINUS_1 + 1;
   rx_info.max_msg_id = GP_TX_MAX_MSG_ID_PLUS_1 - 1; 
}


/**************************************************************************************/
/*! \fn SL_GPSpecificInit(void)
 *
 *  param None
 *
 *  \par Description:	  
 *  Initializes variables with information specific to GP node.
 *
 *  \returns none 
 *
 *  \par Limitations/Caveats:
 *	None
 *
 **************************************************************************************/
static void SL_GPSpecificInit(void)
{
   tx_info.min_msg_id = GP_TX_MIN_MSG_ID_MINUS_1 + 1;
   tx_info.max_msg_id = GP_TX_MAX_MSG_ID_PLUS_1 - 1; 

   rx_info.min_msg_id = VP_TX_MIN_MSG_ID_MINUS_1 + 1;
   rx_info.max_msg_id = VP_TX_MAX_MSG_ID_PLUS_1 - 1; 
}


/**************************************************************************************/
/*! \fn SL_GetPacketToTx(U8 **packet_ptr)
 *
 *  \param[out] packet_ptr - non-NULL pointer to the packet to Tx if the packet to be Tx'd is
 *         				    already buffered.  Will be NULL if a new packet needs to be loaded.
 *
 *  \par Description:	  
 *  Determines if there is a packet ready to Tx or a new packet needs to
 *  be loaded for Tx.  If a packet is ready the pointer to the first byte
 *  in the packet is returned.
 *
 *  \retval SL_SEQ_ERROR - call sequence error.
 *	\retval SL_SUCCESS - successful, if packet_ptr is non-NULL then Tx that packet;
 *          otherwise, load a new packet by calling SL_LoadPacketStart.
 *
 *  \par Limitations/Caveats:
 *	None
 *
 *  \ingroup spi_public
 **************************************************************************************/
int SL_GetPacketToTx(U8 **packet_ptr)
{
   /*
   ** Confirm state is valid to get Tx pointer.
   */
   if (api_state != WAIT_GET_TX_PKT)
   {
      return(SL_SEQ_ERROR);
   }

   if (tx_info.pkt_ptr == NULL)
   {
      /*
      ** No packet is ready to Tx, need to load a new one.
      */
      api_state = WAIT_LOAD_START;
      *packet_ptr = NULL;
   }
   else
   {
      /*
      ** Packet is ready to Tx (or retransmit).  Caller will Tx the packet then must
      ** start unloading after SPI transfer completes.
      */
      INCREMENT_STAT(stats.pkt_tx_cnt);
      api_state = WAIT_UNLOAD_START;
      *packet_ptr = tx_info.pkt_ptr;
   }
   return(SL_SUCCESS);
}


/**************************************************************************************/
/*! \fn SL_LoadPacketStart(int *bytes_left)
 *
 *  \param[out] bytes_left - upon successful return, contains the bytes left in the packet
 *         				for message data (bytes_left reflects the amount of space left for data
 *         				for a "regular" (not "status") message; there are 4 fewer bytes available
 *         				for a "status message).
 *
 *  \par Description:	  
 *  Begins the packet loading process.  After this function is called
 *  the SL_LoadPacketMsg and SL_LoadPacketStatusMsg may be called 
 *  multiple times to load message into the packet.  When all messages 
 *  have been loaded SL_LoadPacketFinish is called.
 *
 *  \retval SL_SEQ_ERROR - call sequence error.
 *	\retval SL_SUCCESS - successful, if packet_ptr is non-NULL then Tx that packet;
 *          otherwise, load a new packet by calling SL_LoadPacketStart.
 *
 *  \par Limitations/Caveats:
 *	None
 *
 *  \ingroup spi_public
 **************************************************************************************/
int SL_LoadPacketStart(int *bytes_left)
{
   /*
   ** Confirm state is valid to start loading.
   */
   if (api_state != WAIT_LOAD_START)
   {
      return(SL_SEQ_ERROR);
   }
   api_state = LOADING; 
   
   /* 
   ** Advance to next packet buffer. 
   */
   tx_info.pkt_ptr = tx_info.oldest_pkt_ptr;     
   tx_info.oldest_pkt_ptr = tx_info.newest_pkt_ptr;
   tx_info.newest_pkt_ptr = tx_info.pkt_ptr;

   /* 
   ** Fill the packet with the "PAD BYTE" 
   */
   memset(tx_info.pkt_ptr, YZ_SPI_PKT_PAD_BYTE, YZ_SPI_PKT_SIZE);

   /*
   ** Setup control variables for loading messages.
   */
   tx_info.msg_count = 0;
   tx_info.load_index = PKT_HDR_BYTES;

   /*
   ** Send COMM Reset message if triggered.
   */
   if (tx_info.comm_reset_flag != 0)
   {
      SendCommResetMsg();
      tx_info.comm_reset_flag = 0;
   } 

#if (YZ_SPI_TPL_ENABLE != 0)
   /*
   ** If TPL is enabled, load the next segment of each TPL message xfer in progress.
   */
   LoadTPLMsgs();
#endif

   /*
   ** If there is not enough room for a 1-byte message return 0 bytes left.
   ** Note that bytes_left may be <0 so we need to account for this. 
   */
   *bytes_left = YZ_SPI_PKT_SIZE - (tx_info.load_index + MSG_HDR_BYTES);
   if (*bytes_left < 1)
   {
      *bytes_left = 0;
   }

   return(SL_SUCCESS);
}

/**************************************************************************************/
/*! \fn SL_LoadPacketMsg(int msg_id, int msg_size_bytes, U8 *msg_data, int *bytes_left)
 *
 *	\param[in] msg_id - id of the message to be loaded. 
 *  \param[in] msg_size_bytes - message size in bytes.
 *  \param[in] msg_data - pointer to message data.      
 *	\param[in] bytes_left - upon successful return, contains the bytes left in the packet 
 *         					for message data (bytes_left reflects the amount of space left for data
 *         					for a "regular" (not "status") message; there are 4 fewer bytes available
 *         					for a "status message).
 *
 *  \par Description:	  
 *  Loads the next message in the packet buffer.
 *
 *  \retval SL_SEQ_ERROR - call sequence error.
 *	\retval SL_SUCCESS - successful
 *  \retval SL_INVALID_ID - the message id parameter is out of range. 
 *  \retval SL_MSG_TOO_LONG - size of message is invalid (greater than YZ_SPI_PKT_MAX_MSG_SIZE or 
 *         					  won't fit in packet).
 *
 *  \par Limitations/Caveats:
 *	None
 *
 *  \ingroup spi_public
 **************************************************************************************/
int SL_LoadPacketMsg(int msg_id, int msg_size_bytes, U8 *msg_data, int *bytes_left)
{
   /*
   ** Confirm state is valid to load.
   */
   if (api_state != LOADING)
   {
      return(SL_SEQ_ERROR);
   }

   /*
   ** Confirm id is valid.
   */
   if ((msg_id < tx_info.min_msg_id) || (msg_id > tx_info.max_msg_id))
   {
      return(SL_INVALID_ID);
   }

   /*
   ** Check that message size is valid.
   */
   if (msg_size_bytes > YZ_SPI_PKT_MAX_MSG_SIZE) 
   {
      return(SL_MSG_TOO_LONG);
   }

   /*
   ** Confirm message will fit.
   */
   if (((tx_info.load_index-1) + MSG_HDR_BYTES + msg_size_bytes) > OFFSET_LAST_PKT_BYTE) 
   {
      return(SL_MSG_TOO_LONG);
   }

   /* 
   ** Load message into packet.
   */
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_LSB  ] = GET_LSB(msg_id);
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_MSB  ] = GET_MSB(msg_id);
   
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_LSB ] = GET_LSB(msg_size_bytes);
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_MSB ] = GET_MSB(msg_size_bytes);
  
   memcpy(&tx_info.pkt_ptr[ tx_info.load_index + MSG_HDR_BYTES ], msg_data, msg_size_bytes);
   INCREMENT_STAT(stats.msgs_loaded);

   /*
   ** Update load control variables.
   */
   ++tx_info.msg_count;
   tx_info.load_index += (msg_size_bytes + MSG_HDR_BYTES); 

   /*
   ** If there is not enough room for a 1-byte message return 0 bytes left.
   ** Note that bytes_left may be <0 so we need to account for this. 
   */
   *bytes_left = YZ_SPI_PKT_SIZE - (tx_info.load_index + MSG_HDR_BYTES);
   if (*bytes_left < 1)
   {
      *bytes_left = 0;
   }

   return(SL_SUCCESS);
}

/**************************************************************************************/
/*! \fn SL_LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left)
 *
 *	\param[in] status_msg_id - id of the status message to be loaded. 
 *  \param[in] status_msg_size_bytes - status message size in bytes.
 *  \param[in] msg_data - pointer to message data.      
 *  \param[out] bytes_left - upon successful return, contains the bytes left in the packet 
 *         					for message data (bytes_left reflects the amount of space left for data
 *         					for a "regular" (not "status") message; there are 4 fewer bytes available
 *         					for a "status message). 
 *
 *  \par Description:	  
 *  Loads a status message into a packet. 
 *
 *  \retval SL_SEQ_ERROR - call sequence error.
 *	\retval SL_SUCCESS - successful
 *  \retval SL_MSG_TOO_LONG - size of message is invalid (greater than YZ_SPI_PKT_MAX_MSG_SIZE or 
 *         					  won't fit in packet).
 *
 *  \par Limitations/Caveats:
 *	None
 *
 *  \ingroup spi_public
 **************************************************************************************/
int SL_LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left)
{
   /*
   ** Confirm state is valid to load.
   */
   if (api_state != LOADING)
   {
      return(SL_SEQ_ERROR);
   }

   /*
   ** Check that message ID is valid.
   */
   if ((status_msg_id == ID_COMM_RESET_MSG) && (status_msg_id == ID_PROTOCOL_VERSION_MSG)  &&
       (status_msg_id == ID_TPL_TX_ABT_MSG))
   {
      return(SL_INVALID_ID);
   }

   /*
   ** Check that message size is valid.
   */
   if (status_msg_size_bytes > YZ_SPI_PKT_MAX_MSG_SIZE) 
   {
      return(SL_MSG_TOO_LONG);
   }

   /*
   ** Confirm message will fit.
   */
   if (((tx_info.load_index-1) + (MSG_HDR_BYTES*2) + status_msg_size_bytes) > OFFSET_LAST_PKT_BYTE) 
   {
      return(SL_MSG_TOO_LONG);
   }

   /*
   ** Load message into packet.
   */
  LoadPacketStatusMsg(status_msg_id, status_msg_size_bytes, msg_data, bytes_left);
  return(SL_SUCCESS);
}

/**************************************************************************************/
/*! \fn LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left)
 *
 *  param None
 *
 *  \par Description:	  
 *  Loads a status message into a packet. 
 *
 *	\returns none
 *
 *  \par Limitations/Caveats:
 *	None
 *
 **************************************************************************************/
static void LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left)
{
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_LSB ] = (U8) 0;  /* ID of packet status msg is 0*/
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_MSB ] = (U8) 0;  /* ID of packet status msg is 0*/
   
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_LSB ] = GET_LSB(status_msg_size_bytes + MSG_HDR_BYTES);
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_MSB ] = GET_MSB(status_msg_size_bytes + MSG_HDR_BYTES);
  
   tx_info.load_index += MSG_HDR_BYTES;

   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_LSB ] = GET_LSB(status_msg_id);  /* ID of status msg */
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_MSB ] = GET_MSB(status_msg_id);

   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_LSB ] = GET_LSB(status_msg_size_bytes); /* size of status msg */
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_MSB ] = GET_MSB(status_msg_size_bytes);
  
   memcpy(&tx_info.pkt_ptr[ tx_info.load_index + MSG_HDR_BYTES ], msg_data, status_msg_size_bytes);
   INCREMENT_STAT(stats.msgs_loaded);

   /*
   ** Update load control variables.
   */
   ++tx_info.msg_count;
   tx_info.load_index += (status_msg_size_bytes + MSG_HDR_BYTES);

   /*
   ** If there is not enough room for a 1-byte message return 0 bytes left.
   ** Note that bytes_left may be <0 so we need to account for this. 
   */
   *bytes_left = YZ_SPI_PKT_SIZE - (tx_info.load_index + MSG_HDR_BYTES);
   if (*bytes_left < 1)
   {
      *bytes_left = 0;
   }
}


/*************************************************************************************/
/*! \fn SL_LoadPacketFinish(void)
 *
 * Parameters:
 *	     None.	
 *
 *	\par Description
 *  Completes loading of a packet.  Upon successful return, the packet is
 *  ready to transmit on SPI bus.
 *
 *  \retval SL_SEQ_ERROR - call sequence error.
 *	\retval SL_SUCCESS - successful
 *
 *  \par Limitations/Caveats:
 *	None
 *
 **************************************************************************************/
int SL_LoadPacketFinish(void)
{
   U16 crc;

   /*
   ** Confirm state is valid to finish loading.
   */
   if (api_state != LOADING)
   {
      return(SL_SEQ_ERROR);
   }

#if (YZ_SPI_TPL_ENABLE != 0)
   /*
   ** If TPL is enabled, fill the remaining bytes left in packet with TPL messages. 
   */
   LoadTPLMsgsFill(1);
#endif

   /* 
   ** Load packet header fields first, then compute and load CRC.
   */
   tx_info.pkt_ptr[ PKT_MSG_COUNT_LSB ] =  GET_LSB(tx_info.msg_count);
   tx_info.pkt_ptr[ PKT_MSG_COUNT_MSB ] =  GET_MSB(tx_info.msg_count);

   tx_info.pkt_ptr[ PKT_SN_LSB ] = GET_LSB(tx_info.next_sn);
   tx_info.pkt_ptr[ PKT_SN_MSB ] = GET_MSB(tx_info.next_sn);
   
   tx_info.pkt_ptr[ PKT_NACK_LSB ] = GET_LSB(tx_info.next_nack);
   tx_info.pkt_ptr[ PKT_NACK_MSB ] = GET_MSB(tx_info.next_nack);

   crc = CalculateCrc(&tx_info.pkt_ptr[sizeof(crc)], (YZ_SPI_PKT_SIZE-sizeof(crc)));

   tx_info.pkt_ptr[ PKT_CRC_LSB ] = GET_LSB(crc);
   tx_info.pkt_ptr[ PKT_CRC_MSB ] = GET_MSB(crc);

#if 0
#if (defined(SL_LOGMSG))
   sprintf(log_buff, ">>>>>SL_LoadPacketFinish: TX packet w/SN=%d  NACK=%d \n", 
       (tx_info.pkt_ptr[ PKT_SN_LSB ] + (tx_info.pkt_ptr[ PKT_SN_MSB ] * 256)),
       (tx_info.pkt_ptr[ PKT_NACK_LSB ] + (tx_info.pkt_ptr[ PKT_NACK_MSB ] * 256)));
   LogMsg(log_buff);
#endif
#endif

   tx_info.last_sn_txd = tx_info.next_sn;
   tx_info.next_sn = ComputeNextSN(tx_info.next_sn);

   api_state = WAIT_GET_TX_PKT;

   return(SL_SUCCESS);
}

/*******************************************************************************
 * Function: RetransmitPacket
 *      Retransmits the last packet transmitted.  
 *
 * Parameters:
 *	    nack - the nack value to send in the packet.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
static void RetransmitPacket(int nack)
{
   U16 crc;

   INCREMENT_STAT(stats.pkt_tx_retries);
   if ((tx_info.go_back == 0) || (tx_info.go_back == -1))
   {
      tx_info.newest_pkt_ptr[ PKT_NACK_LSB ] = GET_LSB(nack);
      tx_info.newest_pkt_ptr[ PKT_NACK_MSB ] = GET_MSB(nack);
      crc = CalculateCrc(&tx_info.newest_pkt_ptr[sizeof(crc)], (YZ_SPI_PKT_SIZE-sizeof(crc)));
      tx_info.newest_pkt_ptr[ PKT_CRC_LSB ] = GET_LSB(crc);
      tx_info.newest_pkt_ptr[ PKT_CRC_MSB ] = GET_MSB(crc);
      tx_info.last_sn_txd = tx_info.newest_pkt_ptr[ PKT_SN_LSB ] + (tx_info.newest_pkt_ptr[ PKT_SN_MSB ] * 256);
#if (defined(SL_LOGMSG))
      sprintf(log_buff, ">>>>>RetransmitPacket:(1)Retransmit packet (SN=%d, NACK/last_sn_txd=%d) \n", tx_info.last_sn_txd, nack);
      LogMsg(log_buff);
#endif
      tx_info.pkt_ptr = tx_info.newest_pkt_ptr; 
   }
   else 
   {
      /*
      ** There was no check of the go_back value on the else by design.  If the go_back becomes corrupted we
      ** will transmit the oldest packet then move on to the next packet in the TransmitNextPacket function. 
      */
      tx_info.oldest_pkt_ptr[ PKT_NACK_LSB ] = GET_LSB(nack);
      tx_info.oldest_pkt_ptr[ PKT_NACK_MSB ] = GET_MSB(nack);
      crc = CalculateCrc(&tx_info.oldest_pkt_ptr[sizeof(crc)], (YZ_SPI_PKT_SIZE-sizeof(crc)));
      tx_info.oldest_pkt_ptr[ PKT_CRC_LSB ] = GET_LSB(crc);
      tx_info.oldest_pkt_ptr[ PKT_CRC_MSB ] = GET_MSB(crc);
      tx_info.last_sn_txd = tx_info.oldest_pkt_ptr[ PKT_SN_LSB ] + (tx_info.oldest_pkt_ptr[ PKT_SN_MSB ] * 256);
#if (defined(SL_LOGMSG))
      sprintf(log_buff, ">>>>>RetransmitPacket:(2)Retransmit packet (SN=%d, NACK/last_sn_txd=%d) \n", tx_info.last_sn_txd, nack);
      LogMsg(log_buff);
#endif
      tx_info.pkt_ptr = tx_info.oldest_pkt_ptr; 
   }
}

/*******************************************************************************
 * Function: TransmitNextPacket 
 *      Transmits the next packet 
 *
 * Parameters:
 *	    nack - the nack value to send in the packet.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
static void TransmitNextPacket(int nack)
{
   U16 crc;

   if ((tx_info.go_back == 0) || (tx_info.go_back == -1))
   {
      tx_info.next_nack = nack;
      tx_info.go_back = 0;
      tx_info.pkt_ptr = NULL; 
   }
   else 
   {
      /*
      ** There was no check of the go_back value on the else by design.  If the go_back becomes corrupted we
      ** will transmit the newest packet then assign go_back a value which will cause loading of a new
      ** packet the next time.
      */
      INCREMENT_STAT(stats.pkt_tx_retries);
      tx_info.newest_pkt_ptr[ PKT_NACK_LSB ] = GET_LSB(nack);
      tx_info.newest_pkt_ptr[ PKT_NACK_MSB ] = GET_MSB(nack);
      crc = CalculateCrc(&tx_info.newest_pkt_ptr[sizeof(crc)], (YZ_SPI_PKT_SIZE-sizeof(crc)));
      tx_info.newest_pkt_ptr[ PKT_CRC_LSB ] = GET_LSB(crc);
      tx_info.newest_pkt_ptr[ PKT_CRC_MSB ] = GET_MSB(crc);
      tx_info.last_sn_txd = tx_info.newest_pkt_ptr[ PKT_SN_LSB ] + (tx_info.newest_pkt_ptr[ PKT_SN_MSB ] * 256);
      tx_info.pkt_ptr = tx_info.newest_pkt_ptr; 
      tx_info.go_back = -1;
#if (defined(SL_LOGMSG))
      sprintf(log_buff, ">>>>>TransmitNextPacket: go_back=-1 (SN=%d, NACK/last_sn_txd=%d) \n", tx_info.last_sn_txd, nack);
      LogMsg(log_buff);
#endif
   }
}

/*******************************************************************************
 * Function: PickTxPacket 
 *      Decides what packet should be Tx'd next. 
 *
 * Parameters:
 *	    rx_nack - the nack value received in last Rx packet. 
 *	    tx_nack - the nack value to send in Tx packet. 
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
static void PickTxPacket(int rx_nack, int tx_nack)
{
   if (rx_nack == 0)
   {
      /* 
      ** NACK of zero indicates no NACK, so move on and transmit next packet.
      */
      TransmitNextPacket(tx_nack);
   } 
   else 
   { 
      /* 
      ** Non-zero NACK indicates other node is NACKing.  If NACK is for the last packet 
      ** Tx'd assume that the other node Rx'd it successfully and advance to next packet.  
      ** If NACK is for next packet, also need to advance to next packet.
      */
      if ((rx_nack == tx_info.last_sn_txd) || (rx_nack == ComputeNextSN(tx_info.last_sn_txd)))
      {
#if (defined(SL_LOGMSG))
         sprintf(log_buff, ">>>>>PickTxPacket: trans next packet (rx_nack=%d, last_sn_txd=%d, tx_nack=%d) \n", rx_nack, tx_info.last_sn_txd, tx_nack);
         LogMsg(log_buff);
#endif
         TransmitNextPacket(tx_nack);
      }
      else
      {
         /* NACK must be for previous packet, so go back */
#if (defined(SL_LOGMSG))
         sprintf(log_buff, ">>>>>PickTxPacket: go_back=-2 (rx_nack=%d, last_sn_txd=%d, tx_nack=%d) \n", rx_nack, tx_info.last_sn_txd, tx_nack);
         LogMsg(log_buff);
#endif
         tx_info.go_back = -2;
         RetransmitPacket(rx_info.next_sn);
      }
   }
}

/*******************************************************************************
 * Function: SL_UnloadPacketStart
 *
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
int SL_UnloadPacketStart(U8 *packet_ptr)
{
   U16 crc_expected;
   U16 crc_packet;
   int msg;
   int sn;
   int msg_id;
   int msg_len;
   int next_index;
   int nack;
   int msg_cnt;

   /*
   ** Confirm state is valid to start unloading.
   */
   if (api_state != WAIT_UNLOAD_START)
   {
      return(SL_SEQ_ERROR);
   }

#if (YZ_SPI_TPL_ENABLE != 0)
   /*
   ** If TPL is enabled, clear the MSG_UNLOADED flag for each message xfer in progress.
   */
   TPLClearUnload();
#endif

#if 0
#if (defined(SL_LOGMSG))
   sprintf(log_buff, ">>>>>SL_UnloadPacketStart: RX packet w/SN=%d  NACK=%d \n", 
       (packet_ptr[ PKT_SN_LSB ] + (packet_ptr[ PKT_SN_MSB ] * 256)),
       (packet_ptr[ PKT_NACK_LSB ] + (packet_ptr[ PKT_NACK_MSB ] * 256)));
   LogMsg(log_buff);
#endif
#endif

   /* 
   ** Verify CRC is valid 
   */
   crc_expected = CalculateCrc(&packet_ptr[sizeof(crc_packet)], (YZ_SPI_PKT_SIZE-sizeof(crc_packet)));
   crc_packet = (U16) (packet_ptr[ PKT_CRC_LSB ] + (packet_ptr[ PKT_CRC_MSB ] * 256));

#if (defined(SL_LOGMSG) && defined(SL_INJECT_RX_ERRORS))
   if ((rand() % CRC_ERR_MOD) == CRC_ERR_VAL)
   {
      crc_packet = crc_expected + 2;
      sprintf(log_buff, ">>>>>SL_UnloadPacketStart: inject CRC error crc_packet=0x%X, crc_expected=0x%X) \n", crc_packet, crc_expected);
      LogMsg(log_buff);
   }
#endif
   if (crc_packet != crc_expected)
   {
      /*
      ** Packet Rx error, per the Hydra SPI protocol the action taken is to retransmit the last packet.
      */
#if (defined(SL_LOGMSG))
      sprintf(log_buff, ">>>>>SL_UnloadPacketStart: Invalid CRC (crc_packet=0x%X  crc_expected=0x%X) \n", crc_packet, crc_expected);
      LogMsg(log_buff);
#endif
      RetransmitPacket(rx_info.next_sn);
      INCREMENT_STAT(stats.crc_errors);
      INCREMENT_STAT(stats.pkts_discarded);
      api_state = WAIT_GET_TX_PKT;
      return(SL_INVALID_CRC);
   }
   /*
   ** Do the final checks to confirm the integrity of the packet.  The final integrity checks
   ** are done here rather than "on the fly" during message unloading because we don't want
   ** to unload some messages (and deliver to applications) then find an issue that 
   ** requires retransmit of the packet resulting in delivery of duplicate messages 
   ** to the application (tracking what messages what delivered and not delivering them
   ** a second time is possible but too tricky).  The integrity checks done are 
   ** as follows:
   ** (1) Confirm end-of-packet is not reached during unloading of messages.
   ** (2) Confirm SN is valid.
   */
   msg_cnt = packet_ptr[ PKT_MSG_COUNT_LSB ] + (packet_ptr[ PKT_MSG_COUNT_MSB ] * 256);
   next_index = PKT_HDR_BYTES;
   for (msg = 0; msg < msg_cnt; ++msg )
   {
      msg_id = packet_ptr[ next_index + MSG_ID_LSB ] + (packet_ptr[ next_index + MSG_ID_MSB ] * 256);
      msg_len = packet_ptr[ next_index + MSG_LEN_LSB ] + (packet_ptr[ next_index + MSG_LEN_MSB ] * 256);
      if (msg_id == ID_STATUS_MSG)
      {
         /*
         ** Process status message. 
         */
         next_index += MSG_HDR_BYTES;
         msg_len = packet_ptr[ next_index + MSG_LEN_LSB ] + (packet_ptr[ next_index + MSG_LEN_MSB ] * 256);
      }
      next_index += msg_len + MSG_HDR_BYTES;

#if (defined(SL_LOGMSG) && defined(SL_INJECT_RX_ERRORS))
   if ((rand() % SL_EOP_ERR_MOD) == SL_EOP_ERR_VAL)
   {
      next_index = OFFSET_LAST_PKT_BYTE + 10;
      sprintf(log_buff, ">>>>>SL_UnloadPacketStart: EOP error");
      LogMsg(log_buff);
   }
#endif

      if ((next_index-1) > OFFSET_LAST_PKT_BYTE)  
      {
         /*
         ** Treat EOP error just like a CRC error. 
         */
#if (defined(SL_LOGMSG))
         sprintf(log_buff, ">>>>>SL_UnloadPacketStart: EOP, re-transmit last packet w/NACK=%d \n", rx_info.next_sn);
         LogMsg(log_buff);
#endif
         RetransmitPacket(rx_info.next_sn);
         INCREMENT_STAT(stats.pkts_discarded);
         INCREMENT_STAT(stats.EOP_errors);
         api_state = WAIT_GET_TX_PKT;
         return(SL_EOP_ERROR);  
      }
   } 

   /*
   ** If we get here the packet has passed validity checks, get NACK and SN values.
   */
   nack = packet_ptr[ PKT_NACK_LSB ] + (packet_ptr[ PKT_NACK_MSB ] * 256);
   if (nack != 0)
   {
      INCREMENT_STAT(stats.nack_rx_nz);
   }

   sn = (U16) (packet_ptr[ PKT_SN_LSB ] + (packet_ptr[ PKT_SN_MSB ] * 256));

#if (defined(SL_LOGMSG) && defined(SL_INJECT_RX_ERRORS))
   if ((rand() % SN_ERR_MOD) == SN_ERR_VAL)
   {
      sn = sn-1;
      sprintf(log_buff, ">>>>>SL_UnloadPacketStart: inject bad sn=%d\n", sn);
      LogMsg(log_buff);
   }
#endif

   /*
   ** Check for a COMM_RESET status message, which per the SPI Protocol Specification, must be
   ** the first message in a packet.  When received reset the SN expected.  The message will
   ** be unloaded and processed later.
   */
   if ((sn == 1) && (nack == 0))
   {
      msg_id  = packet_ptr[ PKT_HDR_BYTES + MSG_ID_LSB ] + (packet_ptr[ PKT_HDR_BYTES + MSG_ID_MSB ] * 256);
      msg_len = packet_ptr[ PKT_HDR_BYTES + MSG_LEN_LSB ] + (packet_ptr[ PKT_HDR_BYTES + MSG_LEN_MSB ] * 256);
      if ((msg_id == ID_STATUS_MSG) && (msg_len == LEN_COMM_RESET_MSG))
      {
         msg_id = packet_ptr[ PKT_HDR_BYTES + MSG_HDR_BYTES + MSG_ID_LSB ] + 
                 (packet_ptr[ PKT_HDR_BYTES + MSG_HDR_BYTES + MSG_ID_MSB ] * 256);
         if (msg_id == ID_COMM_RESET_MSG)
         {
            rx_info.next_sn = 1;
         }
      }
   }

   /*
   ** Next SN to Rx is set to 0 during initilaization and since the other node may have been up 
   ** and running already just accept it's sequence number as valid and continue from there.
   */
   if (rx_info.next_sn == 0)
   {
      rx_info.next_sn = sn; 
   }
   
   /*
   ** Verify the SN is as expected.
   */
   if (sn != rx_info.next_sn)
   {
      /*
      ** SN is not as expected, so the packet must be discarded.  The packet to Tx will 
      ** be based on the NACK received.
      */
#if (defined(SL_LOGMSG))
      sprintf(log_buff, ">>>>SL_UnloadPacketStart:Invalid SN expected SN %d got %d (NACK=%d)\n", rx_info.next_sn, sn, nack);
      LogMsg(log_buff);
#endif
      PickTxPacket(nack, rx_info.next_sn);
      INCREMENT_STAT(stats.sn_errors);
      INCREMENT_STAT(stats.pkts_discarded);
      api_state = WAIT_GET_TX_PKT;
      
      return(SL_INVALID_SN);
   }

   /*
   ** If we get here, the incoming packet is valid so prepare for unloading.
   */
   INCREMENT_STAT(stats.pkts_consumed);
   api_state = UNLOADING;
   rx_info.msgs_left =  msg_cnt;
   rx_info.next_sn = ComputeNextSN(rx_info.next_sn); 
   rx_info.unload_index = PKT_HDR_BYTES;
   rx_info.pkt_ptr = packet_ptr;

   /*
   ** Lastly, decide what packet to Tx next (which will be based on the NACK received).
   */
   PickTxPacket(nack, 0);

   return(SL_SUCCESS);
}

/*******************************************************************************
 * Function: SL_UnloadPacketMsg
 *
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
int SL_UnloadPacketMsg(int *msg_id, int *msg_size_bytes, U8 **buff_ptr)
{
   int rc = 0;

   /*
   ** Confirm state is valid for unloading.
   */
   if (api_state != UNLOADING)
   {
      return(SL_SEQ_ERROR);
   }

   /*                                                                  
   ** Loop until next valid message, or end of packet, is found.
   */
   do 
   {
      if (rx_info.msgs_left == 0)
      {
         /*
         ** No more messages to unload, next API state is to get (or load) next Tx packet.
         */
         api_state = WAIT_GET_TX_PKT;

#if (YZ_SPI_TPL_ENABLE != 0)
   /*
   ** If TPL is enabled, check the MSG_UNLOADED flag for each message xfer in progress.
   */
   TPLCheckUnload();
#endif

         return(SL_NO_MORE_MSGS);
      }
	   
      /*
      ** Unload message id and length. 
      */
	  *msg_id = rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_LSB ] + 
                (rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_MSB ] * 256);
      *msg_size_bytes = rx_info.pkt_ptr[ rx_info.unload_index + MSG_LEN_LSB ] + 
                        (rx_info.pkt_ptr[ rx_info.unload_index + MSG_LEN_MSB ] * 256);
      --rx_info.msgs_left;

      if (*msg_id == ID_STATUS_MSG)
      {
         rx_info.unload_index += MSG_HDR_BYTES;  /* Advance to StatusMsg ID */
         INCREMENT_STAT(stats.msgs_unloaded);
         rc = UnloadStatusMsg(msg_id, msg_size_bytes, buff_ptr);
         if (rc != 0)
         {
            return(SL_SUCCESS_STATUS_MSG);
         }
		 /* I added - RyanS
		 ** Reason: We check rc != 0 to prevent messages like COMM RESET from being processed
		 ** as a valid message, but this isn't the case. UnloadStatusMsg changes *msg_id, which will cause
		 ** the function to take the message as a regular message in the WHILE LOOP below
		 */
		 *msg_id = 0;
      }
     
#if (YZ_SPI_TPL_ENABLE != 0)
      /*
      ** If TPL is enabled, check for/unload TPL message.
      */
      else if (*msg_id == ID_TPL_MSG)
      {
         rx_info.unload_index += MSG_HDR_BYTES;  /* Advance to TPLMsgID */
         UnloadTPLMsg(*msg_size_bytes);
         INCREMENT_STAT(stats.msgs_unloaded);
      }
#endif
   
      /*
      ** Check to see if the "regular" message unloaded has a valid ID.
      */
      else if ( (*msg_id < rx_info.min_msg_id) || (*msg_id > rx_info.max_msg_id))
      {
         rx_info.unload_index += MSG_HDR_BYTES + *msg_size_bytes;  /* Advance past msg w/invalid ID */
         INCREMENT_STAT(stats.msg_id_errors);
      }
   } while ( (*msg_id < rx_info.min_msg_id) || (*msg_id > rx_info.max_msg_id) );
   
   /*
   ** A valid "regular" message was unloaded.
   */
   INCREMENT_STAT(stats.msgs_unloaded);
   *buff_ptr = &rx_info.pkt_ptr[ rx_info.unload_index + MSG_HDR_BYTES ]; 
   rx_info.unload_index += MSG_HDR_BYTES + *msg_size_bytes;
   return(SL_SUCCESS);
}

/*******************************************************************************
 * Function: SL_DispatchMsg
 *
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
void SL_DispatchMsg(int msg_id, int msg_size_bytes, U8 * const msg_data)
{
   if ((msg_id > 0) && (msg_id <= LAST_MSG_DISPATCH_TABLE_ID))
   {
      //PRINTF("SL_Dispatch: invoking dispatch function\n");
      (*msg_dispatch_table[ msg_id ])(msg_size_bytes, msg_data);
   }
   else
   {
      //PRINTF(">>>>>SL_Dispatch: called with invalid msg_id=%d\n", msg_id);
   }
}


/*******************************************************************************
 * Function: SL_GetProtocolVersion
 *
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
int SL_GetProtocolVersion(void)
{
   return(SUPPORTED_SPI_PROTOCOL_VERSION);
}

/*******************************************************************************
 * Function: SL_GetStatistics
 *		See documentation in spi_lib.h.
 * 
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns: 
 *		See documentation in spi_lib.h.
 ******************************************************************************/
void SL_GetStatistics(SL_STATS_RECORD *stats_buff)
{
   *stats_buff = stats;
}

/*******************************************************************************
 * Function: ComputeNextSN
 *      Computes the next SN to be used based on the current SN supplied as an 
 *      input.
 *
 * Parameters:
 *	    current_sn - the current SN	
 *
 * Returns:
 *		The next sequence number to use. 
 ******************************************************************************/
static int ComputeNextSN(int current_sn)
{
   int next_sn;

   next_sn = current_sn + 1;
   if (next_sn > MAX_PKT_SN)
   {
      next_sn = 1;
   }

   return(next_sn);
}

/********************************************************************************************
*  Function: SendCommResetMsg
*       Sends a COMM reset status message and a Protocol ID status message.
*
*  Parameters:    
*       None. 
*
*  Returns:     
*       None. 
********************************************************************************************/ 
static void SendCommResetMsg(void)
{
   U8 protocol_version[LEN_PROTOCOL_VERSION_MSG];
   int bytes_left;

    /*
    ** The COMM reset msg must be sent first per the protocol definition.  There is no need to 
    ** check return codes since these messages are the first loaded in a packet.
    */
   (void) LoadPacketStatusMsg(ID_COMM_RESET_MSG, sizeof(tx_info.comm_reset_reason), &tx_info.comm_reset_reason, &bytes_left);

   protocol_version[0] = SUPPORTED_SPI_PROTOCOL_VERSION % 256;  /* LSB */
   protocol_version[1] = SUPPORTED_SPI_PROTOCOL_VERSION / 256;  /* MSB */
   (void) LoadPacketStatusMsg(ID_PROTOCOL_VERSION_MSG, sizeof(protocol_version), &protocol_version[0], &bytes_left);
}

/*******************************************************************************
 * Function: UnloadStatusMsg
 *      Unloads an incoming status message...the following MUST be true when this
 *      function is called:
 *
 *      rx_info.pkt_ptr[ rx_info.unload_index ] MUST point to first byte of the 
 *         Status Message ID field 
 *
 * Parameters:
 *      msg_id - ID of the status message unloaded
 *      msg_size_bytes - length of status message unloaded
 *      msg_data - pointer to first byte in status message unloaded
 *      
 * Returns: 
 *      0 - Status message was unloaded and consumed
 *	    1 - Status message was unloaded but NOT consumed (the message should be returned
 *          to the MM for processing)
 *      
 ******************************************************************************/
static int UnloadStatusMsg(int *msg_id, int *msg_size_bytes, U8 **msg_data)
{
   int rc = 1;
   U16 protocol_id;

   /*
   ** Unload "status" message id and length (note their are no id checks for status messages).
   */
   *msg_id = rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_LSB ] + 
                   (rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_MSB ] * 256);
   *msg_size_bytes = rx_info.pkt_ptr[ rx_info.unload_index + MSG_LEN_LSB ] + 
                           (rx_info.pkt_ptr[ rx_info.unload_index + MSG_LEN_MSB ] * 256);
   *msg_data = &rx_info.pkt_ptr[ rx_info.unload_index + MSG_HDR_BYTES ];

   switch (*msg_id)
   {
      case ID_COMM_RESET_MSG:
#if (YZ_SPI_TPL_ENABLE != 0)
         TPLAbortTxMsgs();
         TPLAbortRxMsgs();
         TPLInit();
#endif
         SLx_ErrorCallback( ERC_COMM_RESET_RXD );
         rc = 0;
      break;

      case ID_PROTOCOL_VERSION_MSG:
		  //We modified (RyanS)
         protocol_id = (*msg_data[0] + ((*msg_data+1)[0] * 256 ));
         if (protocol_id != SUPPORTED_SPI_PROTOCOL_VERSION)
         {
            SLx_ErrorCallback( ERC_SPI_LIB_VER_MISMATCH );
         }
         rc = 0;
      break;

      case ID_TPL_TX_ABT_MSG:
#if (YZ_SPI_TPL_ENABLE != 0)
         *msg_id = rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_LSB + MSG_HDR_BYTES ] + 
                  (rx_info.pkt_ptr[ rx_info.unload_index + MSG_ID_MSB + MSG_HDR_BYTES ] * 256);
		 //I changed (RyanS) from msg_id to *msg_id
		 //  also changed from *msg_data to *(msg_data[0]+2)
         TPLIncomingTxAbort(*msg_id, *(msg_data[0]+2));
         rc = 0;
#endif
      break;
   }
   /*
   ** Adjust unload index to point to the next message.
   */
   rx_info.unload_index += MSG_HDR_BYTES + *msg_size_bytes;
   return(rc);
}

/********************************************************************************************
*  Function: CalculateCrc
*       Calculate the Cyclic Redundancy Code for the contents of the buffer.
*
*  Parameters:    
*       pbuf - Pointer to the buffer
*       length - number of bytes in the buffer. 
*
*  Returns:     
*       The CRC value. 
********************************************************************************************/ 
#define POLYNOMIAL 0x1021  // x^16+x^12+x^5+1 = (1) 0001 0000 0010 0001 = 0x1021
static U16 CalculateCrc(U8 * pBuf, int length) {
int rem=0;
int i;
int j;
int Byte;

for(i=0; i < length;i++, pBuf++)
{
  Byte = (int)(*pBuf);
  rem = rem ^ (Byte << 8);
  for(j = 1; j < 8; j++)  // Assuming 8 bits per byte
  {
     if(rem & 0x8000)     // if leftmost (most significant) bit is set
       rem = ((rem << 1) ^ POLYNOMIAL);
     else
       rem = (rem << 1);
     rem = rem & 0xFFFF;  // Trim remainder to 16 bits
   } // for(j) 
} // for(i)

// CRC of 0 is not allowed to prevent a packet of all 0's being considered a valid packet.
if (rem == 0)
{
  rem = 0x5aa5;
}
return rem;  
} // end of CalculateCrc()


#if  0   /* CURRENTLY NO USED */
/********************************************************************************************
*  Function: CalculateChecksum
*       Calculate the Checksum (sum of the values for the contents of the buffer.
*
*  Parameters:   
*       pbuf - Pointer to the buffer
*       length - number of bytes in the buffer. 
*
*  Returns:     
*       The checksum value.
********************************************************************************************/ 
static U16 CalculateChecksum(char * pBuf, int length) {
int rem=0;
int i;
int Byte;

for(i=0; i < length;i++, pBuf++)
{
  Byte = (int)(*pBuf);
  rem += Byte;
} // for(i)

rem = rem & 0xFFFF;  // Trim remainder to 16 bits  
return rem;  
} // end of CalculateCrc()
#endif


#if (YZ_SPI_TPL_ENABLE != 0)
/*******************************************************************************************/
/*******************************************************************************************/
/*****  TPL FUNCTIONS - THE FOLLOWING FUNCTIONS ARE NEEDED ONLY WHEN THE TRANSPORT     *****/ 
/*****  LAYER IS ENABLED.  ALL TPL FUNCTIONS SHOULD BE PLACED IN THIS SECTION.         *****/
/*******************************************************************************************/
/*******************************************************************************************/

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/
/*
** TPL Message header offsets (from beginning of header)
**   TPL_MSG_ID_LSB - offset of msg id least sig byte in message hdr
**   TPL_MSG_ID_MSB - offset of msg id most sig byte in message hdr
**   TPL_MSG_LEN_MSB - offset of msg length least sig byte in message hdr 
**   TPL_MSG_LEN_LSB - offset of msg length id most sig byte in message hdr 
**   TPL_MSG_BYTES_REM_LSB - offset of bytes remaining least sig byte in message hdr 
**   TPL_MSG_BYTES_REM_MSB - offset of bytes remaining most sig byte in message hdr 
*/
#define TPL_MSG_HDR_BYTES     (6)
#define TPL_MSG_ID_LSB        (0)
#define TPL_MSG_ID_MSB        (1)
#define TPL_SEG_NUMBER_LSB    (2)
#define TPL_SEG_NUMBER_MSB    (3)
#define TPL_MSG_BYTES_REM_LSB (4)
#define TPL_MSG_BYTES_REM_MSB (5)

/*
** TPL_RX_RECORDS - Number of records in the tpl_rx_table
** TPL_TX_RECORDS - Number of records in the tpl_tx_table
*/
#define TPL_TX_RECORDS   (sizeof(tpl_tx_table)/sizeof(TPL_TX_RECORD))
#define TPL_RX_RECORDS   (sizeof(tpl_rx_table)/sizeof(TPL_RX_RECORD))

/*
** Bits flags for "tpl_rx_table[ i].flags" field.
**     TPL_RXF_IN_USE     - non-zero if record in use; 0 if record is free
**     TPL_RXF_GEN_TX_ABT - a TxABT needs to be generated 
*/
#define TPL_RXF_IN_USE       (0x0001u)
#define TPL_RXF_GEN_TX_ABT   (0x0002u)
#define TPL_RXF_MSG_UNLOADED (0x0004u)

/*
** NULL_NODE - value used to indicate there is no more nodes in the Tx message list.
*/
#define NULL_NODE (-1)

/*
** MAX_PKT_BYTES_FOR_TPL_MSGS - maximum number of packet bytes that can be dedicated
** to TPL messages.
*/
#define MAX_PKT_BYTES_FOR_TPL_MSGS ((YZ_SPI_PKT_SIZE/4)*3)

/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/
/*
** TPL_ABT_MSG - structure of a TxABT message.
*/
typedef struct TPL_ABT_MSG
{
   U16 msg_id;               /* ID of message being TxABT being generated for */
   U16 error_code;           /* reason TxABT was generated */
} TPL_ABT_MSG;

/*
 * Typedef: TPL_TX_RECORD
 *
 * 	Record holding TPL Tx Msg information
 */
typedef struct TPL_TX_RECORD
{
	int   msg_id;             /* Message id */
	U8   *buffer_ptr;         /* Pointer to first byte of Tx buffer OR NULL if record not in use */
    int   msg_sz_bytes;       /* Message size in bytes */
    int   min_bytes_packet;   /* Min number of bytes to send in a single packet */
    int   next_seg_num;       /* Next seg # to Tx */
    int   load_index;         /* Offset of next byte to load */
    int   next_node;          /* Offset of next node in the Tx message list */
} TPL_TX_RECORD;

/*
 * Typedef: TPL_RX_RECORD
 *
 * 	Record holding TPL Rx Msg information
 */
typedef struct TPL_RX_RECORD
{
	int   msg_id;          /* Message id */
	U8   *buffer_ptr;      /* Pointer to first byte of Rx buffer */
    int   msg_sz_bytes;    /* Message size in bytes */
    int   next_seg_num;    /* Next seg # expected */
    int   unload_index;    /* Offset of next byte to unload */
    int   flags;           /* IN_USE: !=0 for record in use; =0 for record free */ 
                           /* TX_ABT: !=0 for generated TxTPLABT status message */ 
                           /* MSG_UNLOADED: =0 if no message unloaded since last clear; !=0 for message unloaded */ 
    int   tx_abort_code;   /* Reason for sending TxTPLABT (applies only when TX_ABT flag is !=0) */

} TPL_RX_RECORD;

/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/
static void LinkTPLTxNode(int new_node, int id);
static void UnlinkTPLTxNode(int msg_id);
static int FindFreeRxTableRec(void);
static int FindTxTableRec(int msg_id);

/******************************************************************************/
/*     M E M O R Y   A L L O C A T I O N                                      */
/******************************************************************************/
/*
** tpl_tx_table - table holding information describing TPL Tx messages currently
** in progress (each record that is "IN_USE" describes one transfer).
*/
TPL_TX_RECORD tpl_tx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];

/*
** tpl_rx_table - table holding information describing TPL Rx messages currently
** in progress (each record that is "IN_USE" describes one transfer).
*/
TPL_RX_RECORD tpl_rx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];

/*
** tx_msg_list_head - "points" to the first node in the list of TPL Tx messages in the tpl_tx_table.  The list is 
** maintained in priority order, highest priority messages first in list.
*/
int   tx_msg_list_head = NULL_NODE;


/*******************************************************************************
 * Function: TPLInit 
 *      Performs TPL initialization.
 *
 * Parameters:
 *      None.
 *
 * Returns: 
 *      None.
 ******************************************************************************/
 static void TPLInit(void)
 {
   int i;

   tx_msg_list_head = NULL_NODE;

   for (i=0; i<TPL_TX_RECORDS; ++i)
   {   
      tpl_tx_table[ i ].buffer_ptr = NULL;
      tpl_tx_table[ i ].next_node = NULL_NODE;

      tpl_rx_table[ i ].flags = 0;
      tpl_rx_table[ i ].tx_abort_code = 0;
   }
 }

/*******************************************************************************
 * Function: SL_TxTPLMsg 
 *      Transmits a message using the transport layer.
 *
 * Parameters:
 *		See documentation in spi_lib.h.
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
int SL_TxTPLMsg(int msg_id, int msg_sz_bytes, U8 *msg_data, int min_bytes_packet)
{
   int i;
   int empty_record_idx;
   int tpl_bytes_packet;

   /*
   ** Confirm id is valid.
   */
   if ((msg_id < tx_info.min_msg_id) || (msg_id > tx_info.max_msg_id))
   {
      return(SL_INVALID_ID);
   }

   /*
   ** Check that message size and min bytes per packet are valid.
   */
   if ((msg_sz_bytes > YZ_SPI_MAX_MSG_SIZE) || (min_bytes_packet > YZ_SPI_TPL_MAX_MSG_SIZE))
   {
      return(SL_MSG_TOO_LONG);
   }

   /*
   ** Get sum of min_packet_bytes for all messages.  Find an empty record at the same time.
   */
   tpl_bytes_packet = min_bytes_packet + MSG_HDR_BYTES + TPL_MSG_HDR_BYTES;
   empty_record_idx = -1;
   for (i=0; i<TPL_TX_RECORDS; ++i)
   {   
      
	   if (tpl_tx_table[ i ].buffer_ptr != NULL) 
      {
        if (tpl_tx_table[ i ].msg_id == msg_id) 
         {
            /* 
            ** TPL transfer for this message id is already in progress and multiple not allowed
            */
            return(SL_INVALID_ID);
         }
	
         tpl_bytes_packet += tpl_tx_table[ i ].min_bytes_packet + MSG_HDR_BYTES + TPL_MSG_HDR_BYTES;
      }
      else
      {
         empty_record_idx = i;
      }
   }

   /*
   ** Confirm the sum of min_packet_bytes for all messages is within the limit.
   */
   if (tpl_bytes_packet > MAX_PKT_BYTES_FOR_TPL_MSGS) 
   {
      return(SL_MSG_TOO_LONG);
   }

   /*
   ** Confirm an empty record was found (if not the max number of simultaneous transfers is in progress). 
   */
   if (empty_record_idx == -1) 
   {
      return(SL_TPL_XFER_LIMIT);
   }

   /*
   ** Initialize the record.
   */
   LinkTPLTxNode(empty_record_idx, msg_id);
   tpl_tx_table[ empty_record_idx ].buffer_ptr = msg_data; 
   tpl_tx_table[ empty_record_idx ].msg_sz_bytes = msg_sz_bytes;
   tpl_tx_table[ empty_record_idx ].min_bytes_packet = min_bytes_packet;
   tpl_tx_table[ empty_record_idx ].next_seg_num = 1;
   tpl_tx_table[ empty_record_idx ].load_index = 0;

   return(SL_SUCCESS);
}

/*******************************************************************************
 * Function: LinkTPLTxNode
 *      Adds a node into the linked list of currently active TPL message transfers.
 *      The new node will be inserted such that message id's are in ascending order
 *      (highest msg_id's first).
 *
 * Parameters:
 *      msg_id - id of message in node to be added to the linked list.
 *      new_node - record offset within the tpl_tx_table of the new node to be inserted
 *
 * Returns: 
 *	    None. 
 *      
 ******************************************************************************/
static void  LinkTPLTxNode(int new_node, int msg_id)
{
   int last_node;
   int next_node;
   int link_found = 0;
   int temp;

   tpl_tx_table[ new_node ].msg_id = msg_id;   

   if (tx_msg_list_head == NULL_NODE)
   {
      /* 
      ** Start new list.
      */
      tx_msg_list_head = new_node;
      tpl_tx_table[ new_node ].next_node = NULL_NODE;
   }
   else if (msg_id < tpl_tx_table[ tx_msg_list_head ].msg_id)
   {
      /* 
      ** Add to start of list.
      */
      temp = tx_msg_list_head;
      tx_msg_list_head = new_node;
      tpl_tx_table[ new_node ].next_node = temp;
   }
   else
   {
      /*
      ** Traverse list until spot to insert is found. 
      */
      last_node = tx_msg_list_head;
      next_node = tpl_tx_table[ last_node ].next_node;
      while ((tpl_tx_table[ next_node ].next_node != NULL) && (link_found == 0))
      {
         if (msg_id < tpl_tx_table[ next_node ].msg_id)
         {
            link_found = 1;
         }
         else
         {
            last_node = next_node;
            next_node = tpl_tx_table[ next_node ].next_node;
         }
      }
      /*
      ** Insert in list.
      */
      temp = tpl_tx_table[ last_node ].next_node;
      tpl_tx_table[ last_node ].next_node = new_node;
      tpl_tx_table[ new_node ].next_node = temp;
   }
}
/*******************************************************************************
 * Function: UnlinkTPLTxNode
 *      Removes a node from the linked list of currently active TPL message transfers.
 * 
 *
 * Parameters:
 *      None.
 *
 * Returns: 
 *	    None. 
 *      
 ******************************************************************************/
static void  UnlinkTPLTxNode(int msg_id)
{
   int last_node;
   int node;

   if (tpl_tx_table[ tx_msg_list_head ].msg_id == msg_id)
   {
      /*
      ** Remove head of list.
      */
      tx_msg_list_head = tpl_tx_table[ tx_msg_list_head  ].next_node;
   }
   else
   {
      /*
      ** Traverse list until the node to remove is found.
      */
      last_node = tx_msg_list_head;
      node = tpl_tx_table[ last_node ].next_node;
      while ((tpl_tx_table[ node ].msg_id != msg_id) && (tpl_tx_table[ node ].next_node != NULL_NODE)) 
      {
         last_node = node;
         node = tpl_tx_table[ last_node ].next_node;
      }

      if (tpl_tx_table[ node ].msg_id == msg_id)
      {
         /*
         ** Remove node from list.
         */
         tpl_tx_table[ last_node ].next_node = tpl_tx_table[ node ].next_node;
      }
   }
}

/*******************************************************************************
 * Function: TPLIncomingAbort
 *      Processes an incoming TPL Tx ABT message. 
 *
 * Parameters:
 *      msg_id - id of the message to be aborted. 
 *      error_code - the reason why the Tx ABT was generated. 
 *
 * Returns: 
 *	    None.
 *      
 ******************************************************************************/
static void TPLIncomingTxAbort(int msg_id, U8 error_code)
{
   int record_id;
   record_id = FindTxTableRec(msg_id);
   if (record_id != -1)
   {
      SLx_TPLTxMsgComplete(tpl_tx_table[ record_id ].msg_id, tpl_tx_table[ record_id ].msg_sz_bytes,
                           tpl_tx_table[ record_id ].buffer_ptr, error_code);
      tpl_tx_table[ record_id ].buffer_ptr = NULL;
      UnlinkTPLTxNode(msg_id);
   }
}


/*******************************************************************************
 * Function: FindFreeRxTableRec
 *      Finds a free Rx Table record that is available for use.
 *
 * Parameters:
 *      void 
 *
 * Returns: 
 *	    -1 for table full (no free records), otherwise the index of a free 
 *      record is returned.
 *      
 ******************************************************************************/
static int FindFreeRxTableRec(void)
{
   int i;
   for (i=0; (i < TPL_RX_RECORDS); ++i)
   {
      if ((tpl_rx_table[ i ].flags & TPL_RXF_IN_USE) == 0)
      {
         return(i); 
      }
   }
   return(-1);
}

/*******************************************************************************
 * Function: FindRxTableRec
 *      Finds the IN_USE TPL Rx Table record containing the specified message id. 
 *      
 *
 * Parameters:
 *      msg_id - message id to find in the table.
 *
 * Returns: 
 *	    -1 for message id not found in table, otherwise the index to the record 
 *      containing the message id is returned.
 *      
 ******************************************************************************/
static int FindRxTableRec(int msg_id)
{
   int i;
   for (i=0; (i < TPL_RX_RECORDS); ++i)
   {
      if ((tpl_rx_table[ i ].flags & TPL_RXF_IN_USE) != 0)
      {
		  if ( msg_id == tpl_rx_table[ i ].msg_id)
         {
            return(i); 
         }
      }
   }
   return(-1);
 } 
 
 /*******************************************************************************
 * Function: FindTxTableRec
 *      Finds the Tx Table record containing the specified message id. 
 *
 * Parameters:
 *      msg_id - message id to find in the table.
 *
 * Returns: 
 *	    -1 for message id not found in table, otherwise the index to the record 
 *      containing the message id is returned.
 *      
 ******************************************************************************/
static int FindTxTableRec(int msg_id)
{
   int i;
   for (i=0; (i < TPL_TX_RECORDS); ++i)
   {
      if (tpl_tx_table[ i ].buffer_ptr != NULL)
      {
         if (msg_id == tpl_tx_table[ i ].msg_id)
         {
            return(i); 
         }
      }
   }
   return(-1);
}

/*******************************************************************************
 * Function: UnloadTPLMsg
 *      Unloads and incoming TPL message...the following MUST be true when this
 *      function is called:
 *
 *      rx_info.pkt_ptr[ rx_info.unload_index ] MUST point to the message ID
 *         field in the transport header.
 *
 * Parameters:
 *      msg_length - length of the incoming transport layer message in bytes
 *
 * Returns: 
 *	    None. 
 *      
 ******************************************************************************/
static void UnloadTPLMsg(int msg_length)
{
   int i;
   int msg_id;
   int bytes_remaining;
   int seg_number;
   int seg_length;
   TPL_RX_RECORD  *tpl_rx_rec;

   msg_id = rx_info.pkt_ptr[ rx_info.unload_index + TPL_MSG_ID_LSB ] +
           (rx_info.pkt_ptr[ rx_info.unload_index + TPL_MSG_ID_MSB ] * 256);
  
   bytes_remaining = rx_info.pkt_ptr[ rx_info.unload_index + TPL_MSG_BYTES_REM_LSB ] +
                    (rx_info.pkt_ptr[ rx_info.unload_index + TPL_MSG_BYTES_REM_MSB ] * 256);

   seg_number = rx_info.pkt_ptr[ rx_info.unload_index + TPL_SEG_NUMBER_LSB ] +
               (rx_info.pkt_ptr[ rx_info.unload_index + TPL_SEG_NUMBER_MSB ] * 256);

   rx_info.unload_index += TPL_MSG_HDR_BYTES;
   seg_length = msg_length - TPL_MSG_HDR_BYTES;

   /*
   ** Find msg ID.
   */
   i = FindRxTableRec(msg_id);
   if (i == -1)
   {
      /* 
      ** Message ID was not found in TPL Rx table, find a free record for new xfer.
      */
      i = FindFreeRxTableRec();
      if (i == -1)
      {
         /* 
         ** This should never happen since YZ_SPI_TPL_MAX_SIM_TFRS is the same on both nodes. 
         ** TODO: Decide if any action needs done here.
         ** Can't send Tx ABT since no records available...send COM reset???
         */
		 rx_info.unload_index += seg_length;
         return;
      }
      else
      {
		 tpl_rx_rec = &tpl_rx_table[ i ];
         
         if (seg_number != 1) 
         {
            /*
            ** Segment number must start at 1, generate a Tx ABT.
            */
            tpl_rx_rec->flags = (TPL_RXF_IN_USE | TPL_RXF_GEN_TX_ABT);
            tpl_rx_rec->tx_abort_code = TPL_ABT_INVL_SEG_NUM;  
			rx_info.unload_index += seg_length;
            return;
         }
         /* 
         ** Get a buffer where the incoming message will be stored.  
         */ 

         tpl_rx_rec->buffer_ptr = SLx_GetTPLMsgBuff(msg_id, (seg_length + bytes_remaining + 2)); 
         if (tpl_rx_rec->buffer_ptr == NULL) 
         {
            /*
            ** Buffer not allocated, generate a Tx ABT.
            */
            tpl_rx_rec->flags = (TPL_RXF_IN_USE | TPL_RXF_GEN_TX_ABT);
            tpl_rx_rec->tx_abort_code = TPL_ABT_NO_BUFF_AVAIL;
         }
         else
         {
            /*
            ** Store the first fragment.
            */
            (void) memcpy(tpl_rx_rec->buffer_ptr, &rx_info.pkt_ptr[ rx_info.unload_index ], seg_length); 
            tpl_rx_rec->msg_id = msg_id;
            tpl_rx_rec->msg_sz_bytes = seg_length + bytes_remaining;
            tpl_rx_rec->next_seg_num = 2;
            tpl_rx_rec->flags = (TPL_RXF_IN_USE | TPL_RXF_MSG_UNLOADED);
            tpl_rx_rec->unload_index = seg_length;
			tpl_rx_rec->tx_abort_code = 0;
         }
      }
   }
   else
   {
      /*
      ** Message ID found in TPL Rx Table.
      */
      tpl_rx_rec = &tpl_rx_table[ i ];
	  
	  if( (tpl_rx_rec->flags & TPL_RXF_GEN_TX_ABT) != 0)
	  {
		  /*
		  ** In this case we have already processed atleast one segment for 
		  **  this message in the current packet which generated an abort condition, so we don't need to process this segment
		  */
		  rx_info.unload_index += seg_length;
          return;
	  }

      if (seg_number != tpl_rx_rec->next_seg_num)
      {
         /*
         ** Seg number is not the next expected, schedule a TxABT.
         */
         tpl_rx_rec->flags = (TPL_RXF_IN_USE | TPL_RXF_GEN_TX_ABT);
         tpl_rx_rec->tx_abort_code = TPL_ABT_INVL_SEG_NUM;
		 rx_info.unload_index += seg_length;
         return;
      }
      if (bytes_remaining != (tpl_rx_rec->msg_sz_bytes - (tpl_rx_rec->unload_index + seg_length)) )
      {
         /*
         ** Bytes remaining is not as expected, schedule a TxABT.
         */
         tpl_rx_rec->flags = (TPL_RXF_IN_USE | TPL_RXF_GEN_TX_ABT);
         tpl_rx_rec->tx_abort_code = TPL_ABT_INVL_BYTES_REM;
		 rx_info.unload_index += seg_length;
		 return;
      }
      /*
      ** Accept this fragment and copy to Rx buffer.
      */
      (void) memcpy(&tpl_rx_rec->buffer_ptr[ tpl_rx_rec->unload_index ], 
                    &rx_info.pkt_ptr[ rx_info.unload_index ], seg_length); 
      tpl_rx_rec->unload_index += seg_length; 
	  tpl_rx_rec->next_seg_num++;
	  tpl_rx_rec->flags |= TPL_RXF_MSG_UNLOADED;
      if (bytes_remaining == 0)
      {
         /*
         ** This was the last segment so free the TPL Rx table record and deliver
         ** the message.
         */
         tpl_rx_rec->flags = 0;
         SLx_TPLRxMsgComplete(tpl_rx_rec->msg_id, tpl_rx_rec->msg_sz_bytes,
                   tpl_rx_rec->buffer_ptr, SL_SUCCESS);
      }
   }

   /* 
   ** Advance to next message in packet.
   */
   rx_info.unload_index += seg_length;
}

/*******************************************************************************
 * Function: TPLClearUnload
 *      Clears unload flags for all currently active messages.  Call this function
 *      BEFORE any messages in a packet are unloaded (from UnloadPacketStart).
 *
 * Parameters:
 *      None.
 *
 * Returns: 
 *	    None. 
 *      
 ******************************************************************************/
static void TPLClearUnload(void)
{
   int i;
   for (i=0; i<TPL_RX_RECORDS; ++i)
   {
      tpl_rx_table[ i ].flags &= ~(TPL_RXF_MSG_UNLOADED);
   }
}

/*******************************************************************************
 * Function: TPLCheckUnload
 *      Checks unload flags for all currently active messages.  Call this function
 *      AFTER any messages in a packet are unloaded (with SL_NO_MORE_MSGS returned).
 *
 * Parameters:
 *      None.
 *
 * Returns: 
 *	    None. 
 *      
 ******************************************************************************/
static void TPLCheckUnload(void)
{
   int i;
   TPL_RX_RECORD  *tpl_rx_rec;

   for (i=0; i<TPL_RX_RECORDS; ++i)
   {
      tpl_rx_rec = &tpl_rx_table[ i ];
	  if (((tpl_rx_rec->flags & TPL_RXF_IN_USE) !=0) && 
		  ((tpl_rx_rec->flags & TPL_RXF_MSG_UNLOADED) == 0) &&
		  ((tpl_rx_rec->flags & TPL_RXF_GEN_TX_ABT) == 0))
      {
         /*
         ** Record in use and a message wasn't received as expected in the just received 
         ** packet. This is an error condition and requires a TX ABT to be generated.
         */
         tpl_rx_rec->flags |= TPL_RXF_GEN_TX_ABT;
         tpl_rx_rec->tx_abort_code = TPL_ABT_NO_SEG_PKT;
      }
   }
}

/*******************************************************************************
 * Function: LoadOneTPLMsg
 *      Loads a TPL related messages in an outgoing packet. 
 *
 * Parameters:
 *	    msg_id - id of message
 *      msg_sz_bytes - size of message in bytes
 *      msg_data - pointer to message data
 *      bytes_remaining - bytes remaining in message AFTER this message
 *      seg_number = segment number of this message
 *
 * Returns: 
 *	    SL_SUCCESS if successful
 *      SL_MSG_TO_LONG if message wont fit in available space in outgoing packet.
 ******************************************************************************/
static int LoadOneTPLMsg(int msg_id, int msg_sz_bytes, U8 *msg_data, int bytes_remaining, int seg_number)
{
   /*
   ** Confirm message will fit.
   */
   if (((tx_info.load_index-1) + MSG_HDR_BYTES + TPL_MSG_HDR_BYTES + msg_sz_bytes) > OFFSET_LAST_PKT_BYTE) 
   {
      return(SL_MSG_TOO_LONG);
   }

   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_LSB ] = (U8) 0xFF;  /* ID of TPL msg is 0xFFFF*/
   tx_info.pkt_ptr[ tx_info.load_index + MSG_ID_MSB ] = (U8) 0xFF;  /* ID of TPL msg is 0xFFFF*/
   
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_LSB ] = GET_LSB(msg_sz_bytes + TPL_MSG_HDR_BYTES);
   tx_info.pkt_ptr[ tx_info.load_index + MSG_LEN_MSB ] = GET_MSB(msg_sz_bytes + TPL_MSG_HDR_BYTES);

   tx_info.load_index += MSG_HDR_BYTES;

   tx_info.pkt_ptr[ tx_info.load_index + TPL_MSG_ID_LSB ] = GET_LSB(msg_id);
   tx_info.pkt_ptr[ tx_info.load_index + TPL_MSG_ID_MSB ] = GET_MSB(msg_id);

   tx_info.pkt_ptr[ tx_info.load_index + TPL_MSG_BYTES_REM_LSB ] = GET_LSB(bytes_remaining); 
   tx_info.pkt_ptr[ tx_info.load_index + TPL_MSG_BYTES_REM_MSB ] = GET_MSB(bytes_remaining);
  
   tx_info.pkt_ptr[ tx_info.load_index + TPL_SEG_NUMBER_LSB ] = GET_LSB(seg_number); 
   tx_info.pkt_ptr[ tx_info.load_index + TPL_SEG_NUMBER_MSB ] = GET_MSB(seg_number);

   memcpy(&tx_info.pkt_ptr[ tx_info.load_index + TPL_MSG_HDR_BYTES ], msg_data, msg_sz_bytes);
   INCREMENT_STAT(stats.msgs_loaded);

   /*
   ** Update load control variables.
   */
   ++tx_info.msg_count;
   tx_info.load_index += (msg_sz_bytes + TPL_MSG_HDR_BYTES);
   return(SL_SUCCESS);
}

/*******************************************************************************
 * Function: LoadTPLMsgs
 *      Loads TPL related messages in an outgoing packet.  This function is 
 *      to be called from the LoadStart function so that these are the first
 *      messages loaded in the packet.
 *
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
static void LoadTPLMsgs(void)
{
   int i;
   int rc;
   int bytes_left; 
   TPL_ABT_MSG    abt_msg;
   TPL_RX_RECORD  *tpl_rx_rec = &tpl_rx_table[ 0 ];

   /*
   ** First take care of any Tx ABT's that need to be sent.
   */
   for (i=0; i<TPL_RX_RECORDS; ++i)
   {
      if (((tpl_rx_rec->flags & TPL_RXF_IN_USE) !=0) && ((tpl_rx_rec->flags & TPL_RXF_GEN_TX_ABT) != 0))
      {
         abt_msg.msg_id = tpl_rx_rec->msg_id;
         abt_msg.error_code = tpl_rx_rec->tx_abort_code;
         rc = SL_LoadPacketStatusMsg(ID_TPL_TX_ABT_MSG, sizeof(abt_msg), (U8 *) &abt_msg, &bytes_left);
         tpl_rx_rec->flags = 0;  /* Mark record as free */
		 /*
		 ** To prevent unwanted RxMsgComplete calls, make sure the buffer is not NULL
		 */
		 if(tpl_rx_rec->buffer_ptr != NULL)
				SLx_TPLRxMsgComplete(tpl_rx_rec->msg_id, tpl_rx_rec->msg_sz_bytes,
					 tpl_rx_rec->buffer_ptr, tpl_rx_rec->tx_abort_code);
         if (rc != SL_SUCCESS)
         {
            /* We should never here, if we do treat as fatal error. */
         }
      }
      ++tpl_rx_rec;
   }

   /*
   ** Now load each TPL message with "min bytes per packet" message length.
   */
   LoadTPLMsgsFill(0);
}

/*******************************************************************************
 * Function: LoadTPLMsgsFill
 *      Loads TPL messages in an outgoing packet.  The "fill_packet" parameter
 *      specifies if the "min bytes per packet" is to be loaded or the maximum
 *      number of bytes that will fit should be loaded...per the protocol the
 *      distinction is whether this loading occurs during the LoadStart or
 *      LoadFinish function. 
 *
 * Parameters:
 *	    fill_packet - =0 -> load min_bytes_packet bytes in each packet
 *                   !=0 -> load as many bytes as will fit.
 * Returns: 
 *	    None. 
 ******************************************************************************/
#define EMPTY_SLOT    (-1)
static void LoadTPLMsgsFill(unsigned int fill_packet)
{
   int rc;
   int node = tx_msg_list_head; 
   int bytes_to_tx;
   int bytes_remaining;
   int bytes_left = YZ_SPI_PKT_SIZE - tx_info.load_index;
   TPL_TX_RECORD  *tpl_tx_rec = &tpl_tx_table[ 0 ];

   /*
   ** Traverse the Tx message list and load the next segment for all active message transfers until
   ** end of the Tx list is reached or the packet is full.
   */
   while ((node != NULL_NODE) && (bytes_left > (MSG_HDR_BYTES + TPL_MSG_HDR_BYTES)))
   {
      tpl_tx_rec = &tpl_tx_table[ node ];
      /*
      ** Calculate number of bytes desired to Tx (either bytes remaining in message or min_bytes_packet).
      */
      bytes_remaining = tpl_tx_rec->msg_sz_bytes - tpl_tx_rec->load_index;
      if (bytes_remaining > tpl_tx_rec->min_bytes_packet)
      {
         bytes_to_tx = (fill_packet == 0) ? tpl_tx_rec->min_bytes_packet : bytes_remaining;
      }
      else
      {
         bytes_to_tx = bytes_remaining;
      }
      /*
      ** Clamp bytes to Tx based on bytes left in packet.
      */
      if (bytes_to_tx > (YZ_SPI_PKT_SIZE - tx_info.load_index - MSG_HDR_BYTES - TPL_MSG_HDR_BYTES))
      {
         bytes_to_tx = (YZ_SPI_PKT_SIZE - tx_info.load_index - MSG_HDR_BYTES - TPL_MSG_HDR_BYTES);
      }

      bytes_remaining =  (tpl_tx_rec->msg_sz_bytes - tpl_tx_rec->load_index) - bytes_to_tx;

      rc = LoadOneTPLMsg(tpl_tx_rec->msg_id, bytes_to_tx, &tpl_tx_rec->buffer_ptr[ tpl_tx_rec->load_index ], 
                                     bytes_remaining, tpl_tx_rec->next_seg_num);
      if (rc != SL_SUCCESS)
      {
               /* We should never here, if we do treat as fatal error. */
      }
      if (bytes_remaining != 0)
      {
         tpl_tx_rec->load_index += bytes_to_tx;
         ++tpl_tx_rec->next_seg_num;
      }
      else
      {
         /*
         ** Message transfer is done, so do the following:
         ** (1) Notify, via callback function, that the message transfer is complete.
         ** (2) Unlink the node from the Tx message list.
         ** (3) Free up the node in the TPL Tx table
         */
         SLx_TPLTxMsgComplete(tpl_tx_rec->msg_id, tpl_tx_rec->msg_sz_bytes, tpl_tx_rec->buffer_ptr, SL_SUCCESS);
         UnlinkTPLTxNode(tpl_tx_rec->msg_id);
         tpl_tx_rec->buffer_ptr = NULL;
		 tpl_tx_rec->next_node = NULL_NODE;
      }
      bytes_left =  YZ_SPI_PKT_SIZE - tx_info.load_index;
	  //RyanS added IF statement 
	  //Reason: If tpl_tx_table[ node ] is not the same as tpl_tx_rec (which it is initially set to) then a message was UNLINKED
	  //	and it was the head message. We must NOT move to the next_node in this case.
	  if (&tpl_tx_table[ node ] == tpl_tx_rec)
		  node = tpl_tx_table[ node ].next_node;
   }
}

/*******************************************************************************
 * Function: TPLAbortTxMsgs
 *      Aborts all TPL Tx messages in progress due to COMM_RESET.
 *
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
static void TPLAbortTxMsgs(void)
 {
   int i;
   TPL_TX_RECORD *tpl_tx_rec;

   for (i=0; i<TPL_RX_RECORDS; ++i)
   {
      tpl_tx_rec = &tpl_tx_table[ i ];
      if (tpl_tx_rec->buffer_ptr != NULL) 
      {
         SLx_TPLTxMsgComplete(tpl_tx_rec->msg_id, tpl_tx_rec->msg_sz_bytes, tpl_tx_rec->buffer_ptr, TPL_ABT_COMM_RESET);
         UnlinkTPLTxNode(tpl_tx_rec->msg_id);
         tpl_tx_rec->buffer_ptr = NULL;
      }
   }
   tx_msg_list_head = NULL_NODE;
 }

/*******************************************************************************
 * Function: TPLAbortRxMsgs
 *      Aborts all TPL Rx messages in progress due to COMM_RESET.
 *
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
static void TPLAbortRxMsgs(void)
 {
   int i;
   TPL_RX_RECORD  *tpl_rx_rec;

   for (i=0; i<TPL_RX_RECORDS; ++i)
   {
      tpl_rx_rec = &tpl_rx_table[ i ];
      if ((tpl_rx_rec->flags & TPL_RXF_IN_USE) !=0) 
      {
         SLx_TPLTxMsgComplete(tpl_rx_rec->msg_id, tpl_rx_rec->msg_sz_bytes, tpl_rx_rec->buffer_ptr, TPL_ABT_COMM_RESET);
         tpl_rx_rec->flags = 0;
      }
   }
 }



#endif /* #if (YZ_SPI_TPL_ENABLE != 0) */

#ifdef SL_TEST
/*******************************************************************************************/

/*******************************************************************************************/
/*****  TEST FUNCTIONS - THE FOLLOWING FUNCTIONS ARE PROVIDED FOR TESTING ONLY.        *****/
/*****  THEY ARE NOT INTENDED TO BE USED DURING NORMAL OPERATIONAL USAGE AND ARE       *****/
/*****  THEREFORE NOT INCLUDED IN spi_lib.h.                                           *****/
/*******************************************************************************************/
/*******************************************************************************************/


/********************************************************************************************
*  Function: SL_TEST_CalculateCrc
*       THIS IS A FUNCTION TO SUPPORT UNIT TEST ONLY!!!!!
*       Computes the Cyclic Redundancy Code for the contents of the buffer.
*
*  Parameters:    
*       pbuf - Pointer to the buffer
*       length - number of bytes in the buffer. 
*
*  Returns:     
*       The CRC value. 
********************************************************************************************/ 
U16 SL_TEST_CalculateCrc(U8 * pBuf, int length) 
{
    return(CalculateCrc(pBuf, length));
}

/********************************************************************************************
*  Function: SL_TEST_Unload
*       THIS IS A FUNCTION TO SUPPORT UNIT TEST ONLY!!!!!
*       Sets the api_state to WAIT_UNLOAD_START to support packet unload testing.
*
*  Parameters:    
*       pbuf - Pointer to the buffer
*       length - number of bytes in the buffer. 
*
*  Returns:     
*       None.
********************************************************************************************/ 
void SL_TEST_Unload(void)
{
   api_state = WAIT_UNLOAD_START;
}

/*******************************************************************************
 * Function: SL_TEST_GetPacketFields
 *      THIS IS A FUNCTION TO SUPPORT UNIT TEST ONLY!!!!!
 *
 * Parameters:
 *		pkt_ptr - pointer to the packet to get fields from
 *      crc - crc value from packet is returned here 
 *      msg_cnt - msg count from packet is returned here
 *      sn - sequence number from packet is returned here
 *      nack - nack from packet is returned here
 *
 * Returns: 
 *	    None.
 ******************************************************************************/
void SL_TEST_GetPacketFields(U8 *pkt_ptr, U16 *crc, U16 *msg_cnt, U16 *sn, U16 *nack)
{
   *crc = pkt_ptr[ PKT_CRC_LSB ] + (pkt_ptr[ PKT_CRC_MSB ] * 256);
   *msg_cnt = pkt_ptr[ PKT_MSG_COUNT_LSB ] + (pkt_ptr[ PKT_MSG_COUNT_MSB ] * 256);
   *sn = pkt_ptr[ PKT_SN_LSB ] + (pkt_ptr[ PKT_SN_MSB ] * 256);
   *nack = pkt_ptr[ PKT_NACK_LSB ] + (pkt_ptr[ PKT_NACK_MSB ] * 256);
}

/*******************************************************************************
 * Function: SL_SetPacketFields
 *		Sets header fields in a packet
 * 
 * Parameters:
 *		pkt_ptr - pointer to the packet to set fields in
 *      crc - crc value to set in the packet
 *      msg_cnt - msg count value to set in the packet
 *      sn - sequence number value to set in the packet
 *      nack - nack value to set in the packet
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_SetPacketFields(U8 *pkt_ptr, U16 crc, U16 msg_cnt, U16 sn, U16 nack)
{
   pkt_ptr[ PKT_CRC_LSB ] =  GET_LSB(crc);
   pkt_ptr[ PKT_CRC_MSB ] =  GET_MSB(crc);

   pkt_ptr[ PKT_MSG_COUNT_LSB ] =  GET_LSB(msg_cnt);
   pkt_ptr[ PKT_MSG_COUNT_MSB ] =  GET_MSB(msg_cnt);

   pkt_ptr[ PKT_SN_LSB ] = GET_LSB(sn);
   pkt_ptr[ PKT_SN_MSB ] = GET_MSB(sn);
  
   pkt_ptr[ PKT_NACK_LSB ] = GET_LSB(nack);
   pkt_ptr[ PKT_NACK_MSB ] = GET_MSB(nack);
}

/*******************************************************************************
 * Function: SL_TEST_DumpPacket
 *      Dumps (prints) packet information
 *		
 * Parameters:
 *		pkt_ptr - pointer to the packet to dump
 *
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_DumpPacket(U8 *pkt_ptr)
{
   U16  crc, msg_cnt, sn, nack;
   int i, msg, unload_index, msg_id, msg_size_bytes;
#if (YZ_SPI_TPL_ENABLE != 0)
   int bytes_remaining, seg_number;
   U8 *seg_data;
#endif   

   crc = pkt_ptr[ PKT_CRC_LSB ] + (pkt_ptr[ PKT_CRC_MSB ] * 256);
   msg_cnt = pkt_ptr[ PKT_MSG_COUNT_LSB ] + (pkt_ptr[ PKT_MSG_COUNT_MSB ] * 256);
   sn = pkt_ptr[ PKT_SN_LSB ] + (pkt_ptr[ PKT_SN_MSB ] * 256);
   nack = pkt_ptr[ PKT_NACK_LSB ] + (pkt_ptr[ PKT_NACK_MSB ] * 256);
   
   PRINTF("Packet Print:\n");
   PRINTF("     HDR crc=0x%04X  msg_cnt=%d  sn=%d  nack=%d\n", crc, msg_cnt, sn, nack);

   unload_index = PKT_HDR_BYTES;
   for (msg=0; msg < msg_cnt; ++msg)
   {
      msg_id = pkt_ptr[ unload_index + MSG_ID_LSB ] + (pkt_ptr[ unload_index + MSG_ID_MSB ] * 256);
      msg_size_bytes = pkt_ptr[ unload_index + MSG_LEN_LSB ] + (pkt_ptr[ unload_index + MSG_LEN_MSB ] * 256);

      if (msg_id == ID_STATUS_MSG)
      {
         unload_index += MSG_HDR_BYTES; 
         msg_id = pkt_ptr[ unload_index + MSG_ID_LSB ] + (pkt_ptr[ unload_index + MSG_ID_MSB ] * 256);
         msg_size_bytes = pkt_ptr[ unload_index + MSG_LEN_LSB ] + (pkt_ptr[ unload_index + MSG_LEN_MSB ] * 256);
         PRINTF("     SMSG(%d) msg_id=%d  msg_len=%d  msg_data=", msg, msg_id, msg_size_bytes); 
		 unload_index += MSG_HDR_BYTES;
      }

#if (YZ_SPI_TPL_ENABLE != 0)
	  else if (msg_id == ID_TPL_MSG)
	  {
		  msg_size_bytes = pkt_ptr[ unload_index + MSG_LEN_LSB ] + (pkt_ptr[ unload_index + MSG_LEN_MSB ] * 256) - TPL_MSG_HDR_BYTES;
		  unload_index += MSG_HDR_BYTES;
		  msg_id = pkt_ptr[ unload_index + TPL_MSG_ID_LSB ] + (pkt_ptr[ unload_index + TPL_MSG_ID_MSB ] * 256);
		  bytes_remaining = pkt_ptr[ unload_index + TPL_MSG_BYTES_REM_LSB ] + (pkt_ptr[ unload_index + TPL_MSG_BYTES_REM_MSB ] * 256);
		  seg_number = pkt_ptr[ unload_index + TPL_SEG_NUMBER_LSB ] + (pkt_ptr[ unload_index + TPL_SEG_NUMBER_MSB ] * 256);
		  seg_data = &pkt_ptr[ unload_index + TPL_MSG_HDR_BYTES ] ;
		  PRINTF("     TPLMSG(%d) msg_id=%d  bytes_remaining=%d  seg_number=%d  seg_length=%d  seg_data=", msg, msg_id, bytes_remaining, seg_number, strlen(seg_data)); 
		  unload_index += TPL_MSG_HDR_BYTES;
	  }
#endif
      else
      {
         PRINTF("     MSG (%d) msg_id=%d  msg_len=%d  msg_data=", msg, msg_id, msg_size_bytes); 
		 unload_index += MSG_HDR_BYTES;
      }
      
      for (i=0; i<msg_size_bytes; ++i)
      {
         PRINTF("%c", pkt_ptr[unload_index++]);
      }
	  PRINTF("\n");
   }
#if 0
   PRINTF("     PAD (%d bytes):", (OFFSET_LAST_PKT_BYTE-unload_index)+1);
   while(unload_index <= OFFSET_LAST_PKT_BYTE)  
   {
      PRINTF("%2X ", pkt_ptr[unload_index]);
      if (pkt_ptr[unload_index] != YZ_SPI_PKT_PAD_BYTE)
      {
         PRINTF("<<<< ");
      }
      ++unload_index;
      if ((unload_index % 30) == 0)
      {
           PRINTF("\n");
      }
   }
   PRINTF("\n");
#endif
}

/*
** The following variables are prefixed with either "vp_" or "gp_" to 
** represent the state information that is maintained to support node-to-node
** testing.  During node-to-node testing packets are "transmitted" (via memcpy) 
** between the VP and GP nodes.  Doing so requires node specific state information 
** to be maintained by the library.  The library is inheritly single instance and
** making the library multi-instance will make the code more difficult to 
** maintain, larger, and slower so the approach taken is to store copies
** of global variables for each node and copy node specific variables to the
** library global variables when the "focus" is changed to a new node via the  
** SL_TEST_SetFocusVP and SL_TEST_SetFocusGP functions.
*/
static int vp_api_state = WAIT_INIT;
static int gp_api_state = WAIT_INIT;

static TX_INFO  vp_tx_info;
static TX_INFO  gp_tx_info;

static RX_INFO  vp_rx_info;
static RX_INFO  gp_rx_info;

static SL_STATS_RECORD vp_stats;
static SL_STATS_RECORD gp_stats;

#if (YZ_SPI_TPL_ENABLE != 0)
   static TPL_TX_RECORD gp_tpl_tx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];
   static TPL_TX_RECORD vp_tpl_tx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];

   static TPL_RX_RECORD gp_tpl_rx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];
   static TPL_RX_RECORD vp_tpl_rx_table[ YZ_SPI_TPL_MAX_SIM_TFRS ];

   static int   vp_tx_msg_list_head = NULL_NODE;
   static int   gp_tx_msg_list_head = NULL_NODE;
#endif

static int current_focus;  /* Stores what node currently has focus */

/*******************************************************************************
 * Function: SL_TEST_InitNodes
 *      Initializes node global variables for both VP and GP nodes.
 *		
 * Parameters:
 *      vp_buffer1_ptr - pointer to first packet Tx buffer for VP
 *      vp_buffer2_ptr - pointer to second packet Tx buffer for VP
 *      gp_buffer1_ptr - pointer to first packet Tx buffer for GP
 *      gp_buffer2_ptr - pointer to second packet Tx buffer for GP
 *
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_InitNodes(U8 *vp_buffer1_ptr, U8 *vp_buffer2_ptr, U8 *gp_buffer1_ptr, U8 *gp_buffer2_ptr)
{
   SL_Initialize(vp_buffer1_ptr, vp_buffer2_ptr, RFI_CPU_RESET );
   SL_VPSpecificInit();
   vp_api_state = api_state;
   vp_tx_info = tx_info;
   vp_rx_info = rx_info;
#if (YZ_SPI_TPL_ENABLE != 0)
   memcpy(vp_tpl_tx_table, tpl_tx_table, sizeof(tpl_tx_table) );
   memcpy(vp_tpl_rx_table, tpl_rx_table, sizeof(tpl_rx_table) );
   vp_tx_msg_list_head = tx_msg_list_head;

   memcpy(gp_tpl_tx_table, tpl_tx_table, sizeof(tpl_tx_table) );
   memcpy(gp_tpl_rx_table, tpl_rx_table, sizeof(tpl_rx_table) );
   gp_tx_msg_list_head = tx_msg_list_head;
#endif
   vp_stats = stats;

   SL_Initialize(gp_buffer1_ptr, gp_buffer2_ptr, RFI_CPU_RESET );
   SL_GPSpecificInit();
   gp_api_state = api_state;
   gp_tx_info = tx_info;
   gp_rx_info = rx_info;
   

   gp_stats = stats;

   /* Set focus to GP_NODE since is was initialized last. */
   current_focus = GP_NODE;
}

/*******************************************************************************
 * Function: SL_TEST_SetFocusVP
 *      Sets the "focus" to the VP node.  After this call SL_* functions 
 *      will use state (global) information associated with the VP node
 *      until the focus is changed by calling the SL_TEST_SetFocusGP function.
 *		
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_SetFocusVP(void)
{
   if (current_focus == GP_NODE)
   {
      /*
      ** Current focus is GP so a swap to VP is necessary.
      */
      gp_api_state = api_state;
      gp_tx_info = tx_info;
      gp_rx_info = rx_info;
      gp_stats = stats;
#if (YZ_SPI_TPL_ENABLE != 0)
	  memcpy(gp_tpl_tx_table, tpl_tx_table, sizeof(tpl_tx_table) );
      memcpy(gp_tpl_rx_table, tpl_rx_table, sizeof(tpl_rx_table) );
	  gp_tx_msg_list_head = tx_msg_list_head;

	  memcpy(tpl_tx_table, vp_tpl_tx_table, sizeof(vp_tpl_tx_table) );
      memcpy(tpl_rx_table, vp_tpl_rx_table, sizeof(vp_tpl_rx_table) );
	  tx_msg_list_head = vp_tx_msg_list_head;
#endif
      api_state = vp_api_state;
      tx_info = vp_tx_info;
      rx_info = vp_rx_info;
	  
      stats = vp_stats;
      current_focus = VP_NODE;
   }
}

/*******************************************************************************
 * Function: SL_TEST_SetFocusGP
 *      Sets the "focus" to the GP node.  After this call SL_* functions 
 *      will use state (global) information associated with the GP node
 *      until the focus is changed by calling the SL_TEST_SetFocusVP function.
 *		
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_SetFocusGP(void)
{
   if (current_focus == VP_NODE)
   {
      /*
      ** Current focus is VP so a swap to GP is necessary.
      */
      vp_api_state = api_state;
      vp_tx_info = tx_info;
      vp_rx_info = rx_info;
      vp_stats = stats;
#if (YZ_SPI_TPL_ENABLE != 0)
	  memcpy(vp_tpl_tx_table, tpl_tx_table, sizeof(tpl_tx_table) );
      memcpy(vp_tpl_rx_table, tpl_rx_table, sizeof(tpl_rx_table) );
	  vp_tx_msg_list_head = tx_msg_list_head;

      memcpy(tpl_tx_table, gp_tpl_tx_table, sizeof(gp_tpl_tx_table) );
      memcpy(tpl_rx_table, gp_tpl_rx_table, sizeof(gp_tpl_rx_table) );
	  tx_msg_list_head = gp_tx_msg_list_head;
#endif
	  api_state = gp_api_state;
      tx_info = gp_tx_info;
      rx_info = gp_rx_info;
      stats = gp_stats;
      current_focus = GP_NODE;
   }
}

/*******************************************************************************
 * Function: SL_TEST_ResetStats
 *      Resets all statistics counters.
 *		
 * Parameters:
 *	    None.	
 *
 * Returns: 
 *		None.
 ******************************************************************************/
void SL_TEST_ResetStats(void)
{
   vp_stats.crc_errors = 0;
   vp_stats.EOP_errors = 0;
   vp_stats.msg_id_errors = 0;
   vp_stats.msgs_loaded = 0;
   vp_stats.msgs_unloaded = 0;
   vp_stats.nack_rx_nz = 0;
   vp_stats.pkt_tx_cnt = 0;
   vp_stats.pkt_tx_retries = 0;
   vp_stats.pkts_consumed = 0;
   vp_stats.pkts_discarded = 0;
   vp_stats.sn_errors = 0;

   gp_stats = vp_stats;
   stats = vp_stats;
}



/*******************************************************************************************/
/*******************************************************************************************/
/*****  END OF TEST FUNCTIONS                                                          *****/
/*******************************************************************************************/
/*******************************************************************************************/

#endif /* #ifdef SL_TEST */

