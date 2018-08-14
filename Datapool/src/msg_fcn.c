/**************************************************************************************/
/*!
 *  \file		msg_fcn.c
 *
 *  \copyright	Yazaki 2016
 *
 *  \brief		
 *	This software componenet provides standard messaging services 
 *	between address spaces (aka processes) in the GP bootloader and application programs.
 *
 *  \author		Eudora Gunarta
 *
 *  \version	$Revision: 1.4 $  
 *				$Log msg_fcn.c $
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	The <GP inter-address space messaging component</b> provides standard messaging services 
 *	between address spaces (aka processes) in the GP bootloader and application programs.
 *	The primary messaging functions are implemented in the file msg_fcn.c.
 */
/***************************************************************************************/
#define MSG_FCN_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/
#include <string.h>
#include <stdlib.h>

#include "gp_cfg.h"         // Common GP program configuration settings
#include "gp_types.h"       // Common GP program data type definitions
#include "gp_utils.h"       // Common GP program utility functions

#include "msg_buf.h"
#include "msg_api_signals.h"
#include "msg_def.h"

#include "pool_def.h"

/***********************************
	Private Macros and Typedefs
***********************************/
		
/***********************************
	Private Data and Structures
***********************************/

/***********************************
	Private Function Prototypes
***********************************/


/************ Start of code ******************/

/**************************************************************************************/
/*! \fn TxBufMsg(Object obj, uint16_t id, uint8_t * data, uint32_t size, uint64_t timeout)
 *
 *  \param[in] obj		- INTEGRITY Connection to be used for the transfer
 *	\param[in] data		- pointer to the message
 *	\param[in] size		- number of bytes in the message
 *	\param[in] timeout	- timeout in msec
 *
 *  \par Description:	  
 *   Send a message with the given id and given connection.  This function blocks
 *   until either the transfer completed, a transfer error ocurred, or timeout
 *   expired. If timeout is zero, function will block until either the transfer
 *   completed or a transfer error ocurred.
 *
 *  \returns 0 of no errors else non-zero if error
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
int32_t TxBufMsg(	uint8_t component, int socket_fd, 
					uint16_t id,//the id of the message
					pid_t tid,//the tid of the component you wish to sent the message
					uint8_t * data,
					uint32_t size)
{
    
    gp_retcode_t rc;
    Boolean cb = true;
    uint32_t * pBufId = (uint32_t *)malloc(sizeof(uint32_t));
    int offset;
    uint8_t * pMsg;

    pMsg = Msg_GetBuf(size, pBufId, component);
    if(pMsg == NULL){
    	return -1;
    }

    /* Compose requested message into the buffer */
    offset = gp_Store16bit(id, pMsg);		// Store the IPC message ID
    if(pMsg != NULL) 
    {
		memcpy((pMsg+offset), &data[0], size);	// Copy the message payload
    }

    /* Send the message */
    rc = TxMsg(socket_fd, tid, component, pMsg, size, cb);

    /* Return error if any transfer error ocurred */
    if(rc != GP_SUCCESS) 
    {
		Msg_FreeBuf(size,*pBufId, component);
		free(pBufId);
		return -2;		// Tx failure
    }
    Msg_FreeBuf(size,*pBufId, component);
    free(pBufId);
    return 0;			// Success
}

/**************************************************************************************/
/*! \fn TxSetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void * elemValue)
 *
 *	param[in] obj - INTEGRITY Object
 *	param[in] elemId - element ID
 *	param[in] elemType - element type
 *	param[in] elemValue - pointer to element value
 *
 *  \par Description:	  
 *   Sends a message to datapool manager to request to set the element of given ID and type
 *     with given value.
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *  None.
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
#if 0
int32_t TxSetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void * elemValue)
{
    gp_retcode_t rc;
    int32_t ret;
    uint16_t msgId;
    uint32_t msgSz;
    uint8_t * buf;
    uint32_t bufSz;
    uint32_t bufIdx;
    int offset;

    msgId = SetElemReq;

    /* Compose set element message */
    ret = 0;
    switch(elemType)
    {	
	case GP_INT32:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 4;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store32bitSigned(*(int *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_UINT32:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 4;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store32bit(*(uint32_t *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_INT64:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 8;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store64bitSigned(*(int64_t *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_UINT64:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 8;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store64bit(*(uint64_t *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_FLOAT:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 4;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_StoreFloat(*(float *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_DBL:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 8;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_StoreDouble(*(double *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_STRING:
	    {
		int len = strlen((char *)elemValue);
		msgSz = MSG_ID_SZ + ELEM_ID_SZ + len + 1;
		buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
		if(buf == 0) {
		    ret = -1;
		} else {
		    offset = gp_Store16bit(msgId, &buf[0]);
		    offset += gp_Store16bit(elemId, &buf[offset]);
		    strncpy((char *)&buf[offset], (char *)elemValue, len);
		}
		break;
	    }
	    
	case GP_INT16:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 2;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store16bitSigned(*(int16_t *)elemValue, &buf[offset]);
	    }
	    break;
	    
	case GP_UINT16:
	    msgSz = MSG_ID_SZ + ELEM_ID_SZ + 2;
	    buf = Msg_GetBuf(msgSz, &bufSz, &bufIdx);
	    if(buf == 0) {
		ret = -1;
	    } else {
		offset = gp_Store16bit(msgId, &buf[0]);
		offset += gp_Store16bit(elemId, &buf[offset]);
		gp_Store16bit(*(uint16_t *)elemValue, &buf[offset]);
	    }
	    break;
	    
	default:
	    ret = -2;
	    break;
    }
    if(ret != 0) {
	Msg_FreeBuf(bufSz, bufIdx);
	return ret;
    }

    /* Send the message */
    rc = Msg_SyncTxBuf(obj, buf, msgSz, IPC_GEN_TIMEOUT);
    Msg_FreeBuf(bufSz, bufIdx);
    if(rc != GP_SUCCESS) {
	return -3;
    }

    return 0;
}


/**************************************************************************************/
/*! \fn TxGetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void *elemValue)
 *
 *	\param[in] obj - INTEGRITY Object
 *	\param[in] elemId - element ID
 *	\param[in] elemType - element type
 *	\param[in] elemValue - pointer to element value
 *
 *  \par Description:	  
 *   Sends a message to datapool manager to request to get the value of an element of given ID 
 *   and type.
 *
 *  \retval	Returns 0 if OK, non-0 if error.
 *
 *  \par Limitations/Caveats:
 *  None.
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
int32_t TxGetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void *elemValue)
{
    gp_retcode_t rc;
    int32_t ret;
    uint16_t temp16;
    uint16_t txMsgId;
    uint32_t txMsgSz;
    uint8_t * txBuf;
    uint32_t txBufSz;
    uint32_t txBufIdx;
    uint16_t rxMsgId;
    uint32_t rxMsgSz;
    uint8_t * rxBuf;
    uint32_t rxBufSz;
    uint32_t rxBufIdx;
    int offset;

    /* Get buffer for receiving */
    rxMsgSz = 64;
    rxBuf = Msg_GetBuf(rxMsgSz, &rxBufSz, &rxBufIdx);
    if(rxBuf == 0) {
	return -1;
    }

    /* Get buffer for sending */
    txMsgSz = MSG_ID_SZ + ELEM_ID_SZ;
    txBuf = Msg_GetBuf(txMsgSz, &txBufSz, &txBufIdx);
    if(txBuf == 0) 
    {
		return -2;
    }

    /* Compose get element request message into buffer and send it */
    txMsgId = GetElemReq;
    offset = gp_Store16bit(txMsgId, &txBuf[0]);
    offset += gp_Store16bit(elemId, &txBuf[offset]);
    rc = Msg_SyncTxBuf(obj, &txBuf[0], txMsgSz, IPC_GEN_TIMEOUT);

    /* Free buffer used for sending */
    Msg_FreeBuf(txBufSz, txBufIdx);

    /* Return error if any transmit error ocurred */
    if(rc != GP_SUCCESS) {
	return -3;
    }

    /* Wait to receive response message */
    rc = Msg_SyncRxBuf(obj, &rxBuf[0], rxMsgSz, IPC_GEN_TIMEOUT, NULL);

    /* Return error if any receive error ocurred */
    if(rc != GP_SUCCESS) {
	Msg_FreeBuf(rxBufSz, rxBufIdx);
	return -4;
    }

    /* Process received message to get element value */
    offset = gp_Read16bit(&rxMsgId, &rxBuf[0]);
    offset += gp_Read16bit(&temp16, &rxBuf[offset]);
    if( (rxMsgId != GetElemRes) || (temp16 != elemId) ) 
    {
		Msg_FreeBuf(rxBufSz, rxBufIdx);
		return -5;
    }
    ret = 0;
    switch(elemType) {
	case GP_INT32:
	    {
		int32_t temp;
		gp_Read32bitSigned(&temp, &rxBuf[offset]);
		*(int32_t *)elemValue = temp;
		break;
	    }
	case GP_UINT32:
	    {
		uint32_t temp;
		gp_Read32bit(&temp, &rxBuf[offset]);
		*(uint32_t *)elemValue = temp;
		break;
	    }
	case GP_INT64:
	    {
		int64_t temp;
		gp_Read64bitSigned(&temp, &rxBuf[offset]);
		*(int64_t *)elemValue = temp;
		break;
	    }

	case GP_UINT64:
	    {
		uint64_t temp;
		gp_Read64bit(&temp, &rxBuf[offset]);
		*(uint64_t *)elemValue = temp;
		break;
	    }
	case GP_FLOAT:
	    {
		uint32_t temp;
		gp_ReadFloat(&temp, &rxBuf[offset]);
		*(uint32_t *)elemValue = temp;
		break;
	    }
	case GP_DBL:
	    {
		uint64_t temp;
		gp_ReadDouble(&temp, &rxBuf[offset]);
		*(uint64_t *)elemValue = temp;
		break;
	    }
	case GP_STRING:
	    {
		int len = strlen((const char *)&rxBuf[4]);
		strlcpy((char *)elemValue, (char *)&rxBuf[4], len);
		break;
	    }
	case GP_INT16:
	    {
		int16_t temp;
		gp_Read16bitSigned(&temp, &rxBuf[offset]);
		*(int16_t *)elemValue = temp;
		break;
	    }
	case GP_UINT16:
	    {
		uint16_t temp;
		gp_Read16bit(&temp, &rxBuf[offset]);
		*(uint16_t *)elemValue = temp;
		break;
	    }
	default:
	    ret = -6;
	    break;
    }
    if(ret != 0) {
	return ret;
    }

    /* Free buffer used for receiving */
    Msg_FreeBuf(rxBufSz, rxBufIdx);

    return 0;
}

#endif