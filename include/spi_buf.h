/******************************************************************************
*
*                           (c) Copyright 2012
*                           Yazaki North America
*                           All Rights Reserved
*  ----------------------------------------------------------------------------
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
******************************************************************************/
/******************************************************************************
*  File:  
*
*  Author: 
*
*  Description: 
*
*  Modification history:
*   $Log: spi_buf.h  $
*   Revision 1.6 2017/07/19 17:24:23CDT Daniel Kageff (10011932) 
*   1) Added and edited comments for Doxygen documentation generation.
*   2) Added other code comments
*   Revision 1.5 2017/03/30 11:13:33EDT Eudora Gunarta (10031829) 
*   Improvements to GP-VP SPI, added function headers.
*   Revision 1.4 2017/01/31 12:05:38EST Eudora Gunarta (10031829) 
*   GP-VP SPI fixes/improvements:
*   Linked list code redo.
*   Revision 1.3 2016/09/16 09:49:51EDT Eudora Gunarta (10031829) 
*   Added list of error return codes.
*   Revision 1.2 2016/09/12 09:38:40EDT Eudora Gunarta (10031829) 
*   Added error return codes, clean up code.
*   Revision 1.1 2016/06/01 14:29:15EDT Daniel Kageff (10011932) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Application/Ipc_mgr/project.pj
******************************************************************************/
#ifndef SPI_BUF_H
#define SPI_BUF_H

/***********************************
	Public Macros and Typedefs
***********************************/
/* Only two types of messages allowed */
#define SB_STATUS_MESSAGE_TYPE  (2U)
#define SB_REGULAR_MESSAGE_TYPE (1U)

/* Return codes */
#define SB_SUCCESS               ( 0)
#define SB_LOCK_GET_FAILED       (-1)
#define SB_LOCK_FREE_FAILED      (-2)
#define SB_INIT_NOTDONE          (-3)
#define SB_MSG_TYPE_INVALID      (-4)
#define SB_MSG_ID_INVALID        (-5)
#define SB_MSG_BUFF_NOTAVAIL     (-6)
#define SB_MSG_DEQUEUE_FAILED    (-7)
#define SB_MSG_LOADPCKET_FAILED  (-8)

/***********************************
	Public API functions
***********************************/

/*******************************************************************************************/
/* \fn SB_Initialize(void)
 *
 *	param - None
 *
 *  \par Description:	  
 *   Initializes the buffer pool.  THIS FUNCTION MUST BE CALLED BEFORE ANY OTHER FUNCTION IN
 *     THE BUFFER POOL!  This function is also called when restarting SPI communications due
 *     to a fatal error condition.
 *
 *  \retval	0 if no errors, non-zero otherwise.
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *******************************************************************************************/
S32 SB_Initialize(void);

/*******************************************************************************************/
/* \fn SB_QueueMsg(U8 msgType, U16 msgID, U16 numBytes, U8 * msgData)
 *
 *	param[in] msgType - BP_STATUS_MESSAGE_TYPE or BP_REGULAR_MESSAGE_TYPE
 *      param[in] msgID - message ID
 *      param[in] numBytes - number of bytes in the message (not including ID)
 *      param[in] msgData - message data
 *
 *  \par Description:	  
 *   This function can be called by managers other than the SPI message manager to queue a
 *     message. When called, the message will be placed into the buffer pool. Message will
 *     be sent by the SPI message manager when is time to do a transfer.
 *
 *  \retval	0 if no errors, non-zero otherwise.
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *******************************************************************************************/
S32 SB_QueueMsg(U8 msgType, U16 msgID, U16 numBytes, U8 * msgData);

/*******************************************************************************************/
/* \fn SB_DequeueMsgs(int numBytesInPacket)
 *
 *	param[in] numBytesInPacket - number of bytes available in the packet
 *
 *  \par Description:	  
 *   This function is called by the SPI message manager when is time to do a transfer. When
 *     called, this function will dequeue messages from the buffer pool and place them into
 *     a transfer packet. Returns once transfer packet is full or message is too big to fit
 *     into the packet or error ocurred.
 *
 *  \retval	0 if no errors, non-zero otherwise.
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *******************************************************************************************/
S32 SB_DequeueMsgs(int numBytesInPacket);

/*******************************************************************************************/
/*! \fn SB_ObtainLock(void)
 *
 *	param - None
 *
 *  \par Description:	  
 *   This function is called by SB_Initialize(), SB_QueueMsg() and SB_DequeueMsgs() to
 *      protect the buffer pool against multiple threads calling this function simultaneously.
 *      This function must be implemented by user and it shall implement a lock mechanism.
 *
 *  \retval	0 if no errors, non-zero otherwise.
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *******************************************************************************************/
S32 SB_ObtainLock(void);

/*******************************************************************************************/
/*! \fn SB_ObtainLock(void)
 *
 *	param - None
 *
 *  \par Description:	  
 *   This function is called by SB_Initialize(), SB_QueueMsg() and SB_DequeueMsgs() to
 *      protect the buffer pool against multiple threads calling this function simultaneously.
 *      This function must be implemented by user and it shall implement a lock mechanism.
 *
 *  \retval	0 if no errors, non-zero otherwise.
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *******************************************************************************************/
S32 SB_ReleaseLock(void);

#endif
/* SPI_BUF_H */
