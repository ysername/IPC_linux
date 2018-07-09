/******************************************************************************
 * Copyright (C) 2010 Yazaki North America
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
******************************************************************************/
/* Standard multi-include avoidance */
#ifndef __SPI_CALLBACKS_H__
#define __SPI_CALLBACKS_H__


/******************************************************************************
 * Title: SPI Callbacks - spi_callbacks.h
 *		This file contains prototypes of callback functions invoked by the SPI
 *      library. 
 *
******************************************************************************/

/******************************************************************************/
/*     I N C L U D E   F I L E S                                              */
/******************************************************************************/
#include <spi_lib_types.h>
#include <spi_common_config.h>
#include <spi_node_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/
/*
** ERC_x - error callback codes.  These codes are returned by the "SLx_ErrorCallback"
** function.
**
**    ERC_COMM_RESET_RXD - a COMM RESET request was received from the other node
**    ERC_SPI_LIB_VER_MISMATCH - the SPI library version received from the 
**       other node did not match the local SPI library version
*/
#define ERC_COMM_RESET_RXD          (0x30)
#define ERC_SPI_LIB_VER_MISMATCH    (0x31)

/*
** Status codes returned by either SLx_TPLTxMsgComplete or SLx_TPLRxMsgComplete.
** These codes indicate the reason for failure when a message transferred by
** by the transport layer is aborted (these are message transfers initiated when 
** the SL_TxTPLMsg function is called).
**
**    TPL_ABT_INVL_SEG_NUM - Rx node received a message segment that was out of order 
**    TPL_ABT_INVL_BYTES_REM - Rx node received invalid bytes_remaining value
**    TPL_ABT_NO_BUFF_AVAIL - No Rx buffer was available on Rx node 
**    TPL_ABT_NO_SEG_PKT - Rx node received packet with no segment during mesage transfer
**    TPL_ABT_COMM_RESET - Transfer aborted due to communication reset condition
*/
#define TPL_ABT_INVL_SEG_NUM      ((U8)(0x40))
#define TPL_ABT_INVL_BYTES_REM    ((U8)(0x41))
#define TPL_ABT_NO_BUFF_AVAIL     ((U8)(0x42))
#define TPL_ABT_NO_SEG_PKT        ((U8)(0x43))
#define TPL_ABT_COMM_RESET        ((U8)(0x44))


/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/

/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/

/*******************************************************************************
 * Function: SLx_TPLTxMsgComplete
 *      This callback function is invoked by the SPI library upon completion of
 *      message Tx attempt by the transport layer.  If the value of the 
 *      "tx_status" parameter is SL_SUCCESS the message transfer was successful.
 *
 *      if "tx_status" is not SL_SUCCESS then message Tx failed and the value of
 *      "tx_status" indicates the reason for failure.
 *
 *      When this function is called the buffer specified by the "msg_data" and
 *      "msg_sze_bytes" parameter is "returned" and may be deallocated, returned
 *      to a buffer pool, etc. (this is the same buffer that was an input 
 *      parameter to the "SL_TxTPLMsg" function).
 * 
 *      NOTE: THIS FUNCTION IS USED ONLY WHEN THE TRANSPORT LAYER IS ENABLED
 *      VIA THE "YZ_SPI_TPL_ENABLE" CONFIGURATION PARAMETER.
 *
 * Parameters:
 *      id - id of the message to transmit
 *      msg_size_bytes - size of message, in bytes, to transmit
 *      msg_data - pointer to first byte of message to transmit
 *      tx_status - transmit status, indicates if Tx was successful, and if not,
 *         indicates the reason for failure.
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
void SLx_TPLTxMsgComplete(int msg_id, int msg_size_bytes, U8 *msg_data, U8 tx_status);

/*******************************************************************************
 * Function: SLx_TPLRxMsgComplete
 *      This callback function is invoked by the SPI library upon completion of
 *      of message Rx attempt by the transport layer.  If the value of the 
 *      "rx_status" parameter is SL_SUCCESS the message transfer was successful
 *      and: "msg_id" = id of message received; "msg_data" points to the buffer
 *      containing the message, and "msg_sz_bytes" contains the length of the
 *      message.  
 *
 *      if "rx_status" is not SL_SUCCESS then message failed and the value of
 *      "rx_status" indicates the reason for failure.
 *
 *      When this function is called the buffer specified by the "msg_data" and
 *      "msg_sze_bytes" parameter is "returned" and may be deallocated, returned
 *      to a buffer pool, etc. (this is the same buffer that was returned by 
 *      "SLx_GetTPLMsgBuff" function).
 *
 *      NOTE: THIS FUNCTION IS USED ONLY WHEN THE TRANSPORT LAYER IS ENABLED
 *      VIA THE "YZ_SPI_TPL_ENABLE" CONFIGURATION PARAMETER.
 *
 * Parameters:
 *      msg_id - the id of the message received
 *      msg_sz_bytes - length of the message received in bytes
 *      msg_data - pointer to the buffer containing message data
 *      rx_status - receive status, indicates if Rx was successful, and if not,
 *         indicates the reason for failure.
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
void SLx_TPLRxMsgComplete(int msg_id, int msg_sz_bytes, U8 *msg_data, U8 rx_status);

/*******************************************************************************
 * Function: SLx_GetTPLMsgBuff
 *      This callback function is invoked by the SPI library when a buffer is 
 *      needed for receiving a messasge via the transport layer.  A pointer
 *      to the buffer allocated is returned from this function.  The buffer 
 *      CANNOT be accessed while the message transfer is active (as it is being
 *      filled with incoming message bytes).  When message receipt is complete the 
 *      "SLx_TPLRxMsgComplete" callback function will be called.  At that time the 
 *      buffer is "returned" and the incoming message can be processed and the buffer
 *      deallocated, returned to the pool, etc. when processing is complete. 
 *     
 *      If a buffer cannot be allocated for some reason then NULL MUST be returned.
 *      Of course when NULL is returned the message transfer will be aborted. 
 *
 *      NOTE: THIS FUNCTION IS USED ONLY WHEN THE TRANSPORT LAYER IS ENABLED
 *      VIA THE "YZ_SPI_TPL_ENABLE" CONFIGURATION PARAMETER.
 *
 * Parameters:
 *      msg_id - the id of the message to be received
 *      buffer_sz_bytes - length of the buffer, in bytes, that is needed to 
 *         store the incoming mesasge.
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
U8 *SLx_GetTPLMsgBuff(int msg_id, int buffer_sz_bytes); 


/*******************************************************************************
 * Function: SLx_ErrorCallback
 *      This callback function is invoked by the SPI library to report an error
 *      condition encountered within the library.  The action appropriate for the
 *      error code reported should be taken within this function.
 *
 * Parameters:
 *      error - indicates the error condition encountered (all error codes that
 *         can be returned here are uniquely prefixed with "ERC_" and defined 
 *         above in this file)
 *
 * Returns: 
 *	    None. 
 ******************************************************************************/
void SLx_ErrorCallback( int error );


#ifdef __cplusplus
}
#endif

#endif /* __SPI_CALLBACKS_H__ */
