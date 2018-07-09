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
*  File: spi_node_config.h
*
*  Author: Greg Palarski
*
*  Description: SPI Library Node Configuration
*  This file contains node specific definitions required by the SPI library
*  used on the the VP and GP (VP and GP will have their own instances of this
*  file). This file should be generated from the template file 
*  "spi_node_config_h.txt" that is supplied with each release of the SPI comm 
*  library.
*  NOTE: DO NOT CHANGE THE NAMES OF THE PARAMETERS IN THIS FILE, ONLY PARAMETER
*  VALUES MAY BE CHANGED.
*
*  Modification history:
*   $Log: spi_node_config.h  $
*   Revision 1.9 2017/03/30 09:14:04CST Eudora Gunarta (10031829) 
*   Added sleep request message.
*   Revision 1.8 2017/03/21 17:34:23EDT Eudora Gunarta (10031829) 
*   Fixed names of message handlers.
*   Revision 1.7 2017/03/21 17:21:13EDT Eudora Gunarta (10031829) 
*   Added handlers for new messages added in spi_common_config.h.
*   Revision 1.6 2017/03/17 15:55:14EDT Eudora Gunarta (10031829) 
*   Changes related to adding VP_SwVersionInfo SPI message handling.
*   Revision 1.5 2016/11/30 14:53:01EST Eudora Gunarta (10031829) 
*   Updates for dynamic prewarp.
*   Revision 1.4 2016/09/29 10:15:42EDT Eudora Gunarta (10031829) 
*   Changes related to separating VP_VehSigDataMsg into VP_BCanSigDataMsg and VP_CCanSigDataMsg.
*   Revision 1.3 2016/09/12 15:24:05EDT Eudora Gunarta (10031829) 
*   Updated list of rx handlers, fixed complile error about include file.
*   Revision 1.2 2016/06/07 10:17:57EDT Daniel Kageff (10011932) 
*   Initial check in of Eudora's framework code
******************************************************************************/
#ifndef __SPI_NODE_CONFIG_H__
#define __SPI_NODE_CONFIG_H__

/*****************************************************************************/
/*     I N C L U D E   F I L E S                                             */
/*****************************************************************************/
#include "spi_lib.h"

#ifdef __cplusplus
extern "C" {
#endif
/*****************************************************************************/
/*     M A C R O S                                                           */
/*****************************************************************************/
/*
** YZ_NODE_ID - ID of the node this configuration file is used on. 
**    Valid values for this parameter as as follows:
**    "VP_NODE" - specifies VP node. 
**    "GP_NODE" - specifies GP node. 
*/
#define  YZ_NODE_ID  (GP_NODE)

/*
** YZ_NODE_CONFIG_VERSION - Specifies the version of the node specific configuration 
**   file this file is compliant with.  The value of this macro is used to confirm 
**   the version of the this file is compatible with the SPI communication library that 
**   uses it.  Version numbers start with 1 and increment by 1 for each new version.  
**   APPLICABLE TO: VP and GP.
*/
#define  YZ_NODE_CONFIG_VERSION (1) 

/* Define this symbol for test purposes */
#define SL_TEST  1
#define SL_PRINTF_DEBUG 1
//#define SL_LOGMSG   1

/*****************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                           */
/*****************************************************************************/

#ifdef __cplusplus
}
#endif

/*
** PS_RX_MSG_HANDLER_LIST - Populate this macro with a list of product specific 
** message handlers.  Each message handler must be a function with the following signature:
**
**    void MsgHandlerFunction(int msg_size_bytes, U8 * const msg_data)
**
** The list defines the set of "callback" functions that are invoked to process incoming 
** "regular" (not "status") messages.  Callback functions are invoked when the SL_DispatchMsg 
** function is called following successful unloading of a regular message via the 
** SL_UnloadPacketMsg function.  
**
** The following is an example of how to format this list (note that the "\" character 
** MUST be the last character on the line).  It is recommended that handler function names
** be descriptive to increase usability.
**
** void MsgId1Handler(int msg_size_bytes, U8 * const msg_data);
** void MsgId2Handler(int msg_size_bytes, U8 * const msg_data);
** void MsgId3Handler(int msg_size_bytes, U8 * const msg_data);
**
** #define PS_RX_MSG_HANDLER_LIST \
**     MsgId1Handler,          \
**     MsgId2Handler,          \
**     MsgId3Handler,          
*/
void VP_SystemStatusMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_ReqCanDefnUpdateMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_BCanSigDataMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_CCanSigDataMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_CCanMirrorStatusMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_SwVersionInfoHandler(int msg_size_bytes, U8 * const msg_data);
void VP_CanMsgTimeoutStatusHandler(int msg_size_bytes, U8 * const msg_data);
void VP_CanSignalTimeoutStatusHandler(int msg_size_bytes, U8 * const msg_data);
void VP_PrinfHandler(int msg_size_bytes, U8 * const msg_data);
void VP_UdsResponseMsgHandler(int msg_size_bytes, U8 * const msg_data);
void VP_SleepRequesMsgHandler(int msg_size_bytes, U8 * const msg_data);

#define PS_RX_MSG_HANDLER_LIST \
    VP_SystemStatusMsgHandler, \
    VP_ReqCanDefnUpdateMsgHandler, \
    VP_BCanSigDataMsgHandler, \
    VP_CCanSigDataMsgHandler, \
    VP_CCanMirrorStatusMsgHandler, \
    VP_SwVersionInfoHandler, \
    VP_CanMsgTimeoutStatusHandler, \
    VP_CanSignalTimeoutStatusHandler, \
    VP_PrinfHandler, \
    VP_UdsResponseMsgHandler, \
    VP_SleepRequesMsgHandler,

#endif /* __SPI_NODE_CONFIG_H__ */
