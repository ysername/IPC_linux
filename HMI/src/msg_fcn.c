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
 *  \param[in] component- the "component" id of the calling process
 *	\param[in] socket_fd- the fd of the socket that will be used for the tx of the message
 *	\param[in] id 		- the id of the message
 *	\param[in] tid 		- the tid of the process you wish to send the message
 *	\param[in] data		- pointer to the message
 *	\param[in] size		- number of bytes in the message
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
int32_t TxBufMsg(	uint8_t component, 	int socket_fd,	uint16_t id, 
					pid_t tid,			uint8_t * data,	uint32_t size)
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

