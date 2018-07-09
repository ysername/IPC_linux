/*****************************************************************************/
/*!
 * 	  \file    	spi_lib.h
 *
 *	  \copyright (C) 2010 Yazaki North America
 * 		This document is protected by the copyright law of the United States and
 * 		foreign countries.  The work embodied in this document is confidential and
 * 		constitutes a trade secret of Yazaki North America, Inc.  Any copying of
 * 		this document and/or any use or disclosure of this document or the work
 * 		embodied in it without the written authority of Yazaki North America, Inc.,
 * 		will constitute copyright and trade secret infringement and will violate the
 * 		rights of Yazaki North America, Inc.
 *
 *    \brief 	Public interface for VP-GP SPI communication library.
 * 		License: Yazaki License, All Rights Reserved
 *
 *    \author	G. Palarski
 *
 *    \version	$Revision: 1.1 $  $Log $
 *
 *
******************************************************************************/
/* Standard multi-include avoidance */
#ifndef __SPI_LIB_H__
#define __SPI_LIB_H__


/******************************************************************************
 * Title: SPI Library - spi_lib.h
 *		This file contains definitions and functions for the common SPI communications 
 *	    library used on the the VP and GP.  
 *
******************************************************************************/

/******************************************************************************/
/*     I N C L U D E   F I L E S                                              */
/******************************************************************************/
#include "spi_lib_types.h"
#include "spi_common_config.h"
#include "spi_node_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/

/*!
	The following "return codes" are returned by SPI library functions.  The
	typical meaning of the codes is as follows however the meaning may be 
	different for specific functions (see function descriptions for more info).
*/
#define  SL_SUCCESS            (1)		/*!< Operations performed by function completely successfully */
#define  SL_SUCCESS_STATUS_MSG (0)		/*!< Status message successfully unloaded */
#define  SL_SEQ_ERROR          (-1)		/*!< calling sequence error; the function called should NOT have
											 been called at this time (see documentation describing proper call sequence). */
#define  SL_INVALID_ID         (-2)		/*!< message id is out of range */
#define  SL_MSG_TOO_LONG       (-3)		/*!< invalid message length (too long) */
#define  SL_INVALID_PAD        (-4)		/*!< invalid pad bytes found in packet */
#define  SL_INVALID_CRC        (-5)	    /*!< CRC integrity check on packet failed */
#define  SL_INVALID_SN         (-6)		/*!< packet contains unexpected sequence number */
#define  SL_NO_MORE_MSGS       (-7)		/*!< no more messages to unload from packet */
#define  SL_EOP_ERROR          (-8)		/*!< end of packet reached before end of message data */
#define  SL_TPL_XFER_LIMIT     (-9)		/*!< the maximum number of TPL tx messages allowed already in progress. */

/*!
	Identification macros - Used to identify the target system.
*/
#define  VP_NODE   (0x2011)				/*!< Used for identifying the VP node. */
#define  GP_NODE   (0x2012)				/*!< Used for identifying the GP node. */

/*!
	Initialization codes.  These values are used for the "reason_for_init"
	parameter when calling the SL_Initialize function.  Other values may be defined
	specific to a product as long as the values are different from these.
	  RFI_CPU_RESET - 
	  RFI_WD_TIMEOUT -   
*/
#define RFI_CPU_RESET    (0x60)			/*!< use this value following startup following reset */ 
#define RFI_WD_TIMEOUT   (0x61)			/*!< the VP uses this value when a watchdog timeout occurs
					   						 waiting for the GP to do a SPI transfer). */

/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/

/*!
	SL_STATS_RECORD - This structure contains the statistics maintained by 
	 the SPI library.  By design each statistic field increments when 
	 the associated condition occurs.  Each field will be initialized to
	 zero by the C run-time library and is therefore NOT set to zero by
	 SL_Initialize (by design) so that each field maintains the cumulative
	 value. 
*/
typedef struct 
{
   U32  crc_errors;    		/*!< Times invalid CRC found during packet Rx */
   U32  msg_id_errors; 		/*!< Times invalid msg id found during packet Rx */
   U32  sn_errors;     		/*!< Times invalid SN found during packet Rx */
   U32  EOP_errors;    		/*!< Times end of packet error occurred during packet Rx */
   U32  pkts_discarded; 	/*!< Number of packets discarded due to Rx error */ 
   U32  pkts_consumed;  	/*!< Number of packets consumed (Rx'd w/o error) */
   U32  nack_rx_nz;    		/*!< Times non-zero NACK found during packet Rx */
   U32  pkt_tx_cnt;    		/*!< Number of packet transmits */
   U32  pkt_tx_retries;		/*!< Number of packet re-transmits */
   U32  msgs_loaded;   		/*!< Number of messages loaded for Tx */
   U32  msgs_unloaded; 		/*!< Number of messages unloaded during Rx */
} SL_STATS_RECORD;


/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/


/*******************************************************************************
 * Function: SL_Initialize
 *      Initializes the SPI library.  THIS FUNCTION MUST BE CALLED BEFORE ANY
 *      OTHER FUNCTION IN THE LIBRARY!  This function is also called when
 *      restarting SPI communications due to a fatal error condition.
 *
 *      This function "releases" buffers to the SPI library.  After being 
 *      released the buffers are managed by the library and can only be accessed 
 *      as defined via the library API.  The size of each buffer must be at least
 *      YZ_SPI_PKT_SIZE bytes.
 *
 * Parameters:
 *		buffer1_ptr - pointer to the first buffer being released
 *		buffer2_ptr - pointer to the second buffer being released
 *      reason_for_init - reason why this function was called.  Constants that
 *         can be used for this parameter can be found above (beginning with
 *         "RFI_").
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
void SL_Initialize(U8  *buffer1_ptr, U8 *buffer2_ptri, U8 reason_for_init);


/*******************************************************************************
 * Function: SL_GetPacketToTx
 *      Determines if there is a packet ready to Tx or a new packet needs to
 *      be loaded for Tx.  If a packet is ready the pointer to the first byte
 *      in the packet is returned.
 *
 * Parameters:
 *	    packet_ptr - non-NULL pointer to the packet to Tx if the packet to be Tx'd is
 *         already buffered.  Will be NULL if a new packet needs to be loaded.
 *	     
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *	    SL_SUCCESS - successful, if packet_ptr is non-NULL then Tx that packet;
 *         otherwise, load a new packet by calling SL_LoadPacketStart.
 ******************************************************************************/
int SL_GetPacketToTx(U8 **packet_ptr);

/*******************************************************************************
 * Function: SL_LoadPacketStart
 *      Begins the packet loading process.  After this function is called
 *      the SL_LoadPacketMsg and SL_LoadPacketStatusMsg may be called 
 *      multiple times to load message into the packet.  When all messages 
 *      have been loaded SL_LoadPacketFinish is called.
 *
 * Parameters:
 *	    bytes_left - upon successful return, contains the bytes left in the packet 
 *         for message data (bytes_left reflects the amount of space left for data
 *         for a "regular" (not "status") message; there are 4 fewer bytes available
 *         for a "status message).
 *
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *	    SL_SUCCESS - successsful (proceed to loading the packet).
 ******************************************************************************/
int SL_LoadPacketStart(int *bytes_left);

/*******************************************************************************
 * Function: SL_LoadPacketMsg
 *       Loads the next message in the packet buffer.
 *
 * Parameters:
 *		msg_id - id of the message to be loaded. 
 *      msg_size_bytes - message size in bytes.
 *      msg_data - pointer to message data.      
 *	    bytes_left - upon successful return, contains the bytes left in the packet 
 *         for message data (bytes_left reflects the amount of space left for data
 *         for a "regular" (not "status") message; there are 4 fewer bytes available
 *         for a "status message).
 *
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *	    SL_SUCCESS - successful
 *      SL_INVALID_ID - the message id parameter is out of range. 
 *      SL_MSG_TOO_LONG - size of message is invalid (greater than YZ_SPI_PKT_MAX_MSG_SIZE or 
 *         won't fit in packet).
 ******************************************************************************/
int SL_LoadPacketMsg(int msg_id, int msg_size_bytes, U8 *msg_data, int *bytes_left);

/*******************************************************************************
 * Function: SL_LoadPacketStatusMsg
 *       Loads a status message into a packet. 
 *
 * Parameters:
 *		status_msg_id - id of the status message to be loaded. 
 *      status_msg_size_bytes - status message size in bytes.
 *      msg_data - pointer to message data.      
 *    bytes_left - upon successful return, contains the bytes left in the packet 
 *         for message data (bytes_left reflects the amount of space left for data
 *         for a "regular" (not "status") message; there are 4 fewer bytes available
 *         for a "status message). 
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *	    SL_SUCCESS - successful
 *      SL_MSG_TOO_LONG- size of message is invalid (greater than YZ_SPI_PKT_MAX_MSG_SIZE or 
 *         won't fit in packet).
 ******************************************************************************/
int SL_LoadPacketStatusMsg(int status_msg_id, int status_msg_size_bytes, U8 *msg_data, int *bytes_left);

/*******************************************************************************
 * Function: SL_LoadPacketFinish
 *       Completes loading of a packet.  Upon successful return, the packet is
 *       ready to transmit on SPI bus.
 *
 * Parameters:
 *	     None.	
 *
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *	    SL_SUCCESS - successful
 ******************************************************************************/
int SL_LoadPacketFinish(void);

/*******************************************************************************
 * Function: SL_UnloadPacketStart
 *      Begins the process of unloading messages from a packet. 
 *
 * Parameters:
 *	    packet_ptr - pointer to first byte of the packet to start unloading. 
 *
 * Returns: 
 *      SL_SEQ_ERROR - call sequence error.
 *      SL_SUCCESS - successful
 *      SL_INVALID_CRC - packet contains invalid CRC (discard packet and proceed
 *         to getting/loading packet for Tx). 
 *      SL_EOP_ERROR - end of packet reached while parsing messages (discard packet 
 *         and proceed to getting/loading packet for Tx). 
 *      SL_INVALID_SN - sequence number in packet is invalid. 
 ******************************************************************************/
int SL_UnloadPacketStart(U8 *packet_ptr);

/*******************************************************************************
 * Function: SL_UnloadPacketMsg
 *      Unloads the next message from the packet. 
 *
 * Parameters:
 *	     msg_id - upon successful return, contains id of the next message unloaded
 *          from the packet.  
 *       msg_size_bytes - upon successful return, length (in byte) of the message 
 *          unloaded from the packet.
 *       buffer_ptr - upon successful return, returns a pointer to the next message
 *          unloaded from the packet.
 *
 * Returns: 
 *     SL_SEQ_ERROR - call sequence error.
 *	   SL_SUCCESS - successfully unloaded a "regular" (not "status") message
 *	   SL_SUCCESS_STATUS_MSG - successfully unloaded a "status" message
 *     SL_NO_MORE_MSGS - no more messages left to unload from packet.
 ******************************************************************************/
int SL_UnloadPacketMsg(int *msg_id, int *msg_size_bytes, U8 **buff_ptr);

/*******************************************************************************
 * Function: SL_TxTPLMsg 
 *      Transmits a message using the transport layer.  This function is used 
 *      to send messages whose length exceeds the maximum size allowed in a
 *      single packet (YZ_SPI_PKT_MAX_MSG_SIZE).  When SL_SUCCESS is returned
 *      the message will be transfer in a series of segments in N packets.  
 *      While message transfer is active the messsage data CANNOT be accessed.
 *      When message receipt is complete the "SLx_TPLTxMsgComplete" callback function 
 *      will be called.  At that time the buffer is "returned" and the buffer may
 *      be deallocated, returned to the pool, etc.
 *     
 * Parameters:
 *      id - id of the message to transmit
 *      msg_sz_bytes - size of message, in bytes, to transmit
 *      msg_data - pointer to first byte of message to transmit
 *      min_bytes_packet - size, in bytes, of the minimum message segement to be 
 *         transmitted per packet.
 *
 * Returns: 
 *	    SL_SUCCESS - successful 
 *      SL_INVALID_ID - the "id" parameter is invalid (id out of range or a TPL 
 *         transfer with this id is already in progress)
 *      SL_MSG_TO_LONG - either the "msg_size_bytes" parameter or "min_bytes_packet"
 *         parameter is invalid
 *      SL_TPL_XFER_LIMIT - the maximum number of transfers allowed is already
 *         in progress
 ******************************************************************************/
int SL_TxTPLMsg(int id, int msg_sz_bytes, U8 *msg_data, int min_bytes_packet);

/*******************************************************************************
 * Function: SL_DispatchMsg
 *      Dispatches a USER DEFINED receive function to process the message specified 
 *      by the input parameters.  USER DEFINED functions are configured in the 
 *      spi_node_config.h file via the PS_RX_MSG_HANDLER_LIST macro.  This function
 *      is intended to be called immediately after the SL_UnloadPacketMsg function
 *      is called (when SL_UnloadPacketMsg returns SL_SUCCESS).  The parameters
 *      passed to this function should be as returned by SL_UnloadPacketMsg.
 *
 *      NOTE, THE FOLLOWING CONDITIONS MUST BE OBSERVED!
 *
 *      (1) The dispatch function MUST use the message data in "read only" 
 *      fashion.  It can be copied to another buffer (and then modified) but 
 *      the original buffer CANNOT be modified (note the const qualifier on the parameter).
 *
 *      (2) The message data buffer is valid only during execution of the dispatch
 *      function.  The dispatch function CANNOT save a pointer to the message data
 *      and access it later on (copy the message data to another buffer to do this).
 *
 * Parameters:
 *      msg_id - the id of the message to be dispatched 
 *      msg_size_bytes - the size of the message in bytes
 *      msg_data - pointer to the message data (SEE NOTES ABOVE)
 *
 * Returns:
 *		See documenation in spi_lib.h
 ******************************************************************************/
void SL_DispatchMsg(int msg_id, int msg_size_bytes, U8 * const msg_data);

/*******************************************************************************
 * Function: SL_GetProtocolVersion
 *      Returns the version of the "Hydra SPI Protocol Specification" document that
 *      this version of the SPI library is compatible with.  Decimal points are not 
 *      included in the version (ex: 13 will be returned when the SPI library is
 *      compatible with version 1.3 of the Hydra SPI Protocol Specification).
 *
 * Parameters:
 *		None
 *
 * Returns: 
 *	    The version of the Hydra SPI Protocol Specification supported by this
 *      version of the SPI library.
 ******************************************************************************/
int SL_GetProtocolVersion(void);

/*******************************************************************************
 * Function: SL_GetStatistics
 *      Returns the current value of statistics maintained by the SPI library.  
 * 
 * Parameters:
 *	    stats_buff - pointer to a buffer where statistics information is returned.	
 *         See description of the SL_STATS_RECORD type for a detailed description of 
 *         the statistics returned.
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
void SL_GetStatistics(SL_STATS_RECORD *stats_buff);

/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/



#ifdef __cplusplus
}
#endif

#endif /* __SPI_LIB_H__ */
