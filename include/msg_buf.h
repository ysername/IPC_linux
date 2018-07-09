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
*   $Log: msg_buf.h  $
*   Revision 1.2 2017/03/14 15:09:02CST Eudora Gunarta (10031829) 
*   Changes for return codes.
*   Revision 1.1 2016/05/18 09:49:07EDT Daniel Kageff (10011932) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Common/project.pj
******************************************************************************/
#ifndef MSG_BUF_H
#define MSG_BUF_H
#include "gp_types.h"
#include <stdio.h>
/*****************************************************************************/
/*    I N C L U D E   F I L E S                                              */
/*****************************************************************************/  

/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 

/*****************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                            */
/*****************************************************************************/

/*****************************************************************************/
/*    M E M O R Y   A L L O C A T I O N                                      */
/*****************************************************************************/

/******************************************************************************
*  Function Name: Msg_InitBufs
*
*  Description: This function shall NOT be preempted by another task. Therefore,
*    tasks shall implement thread-protection when calling this function.
*
*  Input(s):    None.
*
*  Outputs(s):  None.
*
*  Returns:     
******************************************************************************/
gp_retcode_t Msg_InitBufs(uint8_t component);

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
uint8_t * Msg_GetBuf(uint32_t reqSz, uint32_t * bufIdx, uint8_t component);

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
void Msg_FreeBuf(uint32_t bufSz, uint32_t bufIdx, uint8_t component);

#endif
/* MSG_BUF_H */
