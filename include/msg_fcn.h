/**************************************************************************************/
/*!
 *    \file     msg_fcn.h
 *
 *    \copyright 2016
 *
 *    \brief    
 *
 *    \author   
 *
 *    \version  $Revision: 1.1 $  
 *				$Log $
 *
 */
/***************************************************************************************/
#ifndef MSG_FCN_H
#define MSG_FCN_H

/***********************************
	Public Macros and Typedefs
***********************************/


/*******************************
	Public API functions
 ******************************/
int32_t TxBufMsg(uint8_t component, int socket_fd, uint16_t id, pid_t tid, uint8_t * data, uint32_t size);
//int32_t TxSetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void * elemValue);
//int32_t TxGetElemMsg(Object obj, uint16_t elemId, GP_DATATYPES_T elemType, void * elemValue);

#endif

