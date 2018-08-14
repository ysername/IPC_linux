/**
	@file 		msg_fcn.h
	@version 	1.0
	@date 		8/8/2018
	@author 	Leonardo Reatiga Miranda
	@brief		This file is intended to maintain compatibility with the previus 
				implememntation as it only sends a message it may be replaced
				with a call to txMsg() (defined in msg_api_signals.h)
*/
#ifndef MSG_FCN_H
#define MSG_FCN_H

/*******************************
	Public API functions
 ******************************/
/**
	@brief
	@param[in] uint8_t 	component The type of "component" sending the message
						as defined in the enum component_id_t in the 
						indentification_data.h file.
	@param[in] int socket_fd The file descriptor of a previuosly set socket
						(see SetTxOn() on msg_api_signals.h).
	@param[in] uint16_t id The id of the message as defined in the enum MsgId
						in the msg_def.h file.
	@param[in] pid_t tid The tid of the component you wish to send the message
	@param[in] uint8_t * data The data you wish to send (an unsigened char array
						is recomended for this)
	@param[in] uint32_t size the size in bytes of "data"
	@return
*/
int32_t TxBufMsg(uint8_t component, int socket_fd, uint16_t id, pid_t tid, uint8_t * data, uint32_t size);

#endif

