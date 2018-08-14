/**
	@file msg_api_signals.h
	@version 	1.0
	@date 		8/8/2018
	@author		Leonardo Reatiga Miranda
	@brief		This API has two purposes, the first one is to create an easy
				to use mechanism to send and receive data, and the second one
				is to provide a way to identify wHich process are allowed to
				comunicate between each others.
*/
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>        
#include "types.h"
#include "signal_definitions.h"
/**
	@brief 	component_info_t this struct is used to group the relevant data of 
			every "component" at runtime.
*/
typedef struct{
	uint8_t Component; 	/*!< This is the id of the process defined by the user*/
	pid_t 	Tid;		/*!< This is the id of the process as seen by the kernel*/
	int8_t 	Fd;			/*!< This is the file descriptor of the socket used for msg*/
}component_info_t;
/**
	@brief GetTids() 	This function is used to retrieve the tid (unique for 
						each process) at runtime, of the processes specified 
						in components[]. Be aware that if by any reason one of 
						the processes specified is not able to properly identy
						itself, the function will block, as no time-out is
						specified. This function is a counterpart of PostTid().
	@param[in] const char connection_path[]	This char array contains the unix
						domain socket that must be common for the processes 
						both getting and posting their id.
	@param[in] uint8_t	components[]	This array contains the id of the 
						processes you want to get their id's, you should use 
						the id's that are defined in the enum component_id_t 
						defined in the identification_data.h file
	@param[in] uint8_t	the amount of elements in the components[]	array
	@param[in] component_info_t * componentsId	This is usually an array 
						of the type component_info_t described above, and it is
						used to register/share/validate the relevant info of
						each process
	@param[in] uint8_t 	ImComponent This variable holds the process id as 
						defined by the user
						
	@return Returns -1 if an error ocurred, 0 on success
*/
int8_t GetTids(const char connection_path[], uint8_t  components[], uint8_t connectionsToWait,  component_info_t * componentsId, uint8_t ImComponent);
/**
	@brief PostTids() 	This function is used to "post" the tid (unique for 
						each process) at runtime, of the processes specified 
						in component. This function is a counterpart of 
						GetTids().
	@param[in] const char connection_path[]	This char array contains the unix
						domain socket that must be common for the processes 
						both getting and posting their id.
	@param[in] uint8_t	components	should contain the id of the processes that
						is calling this function it is advised you use 
						the id's that are defined in the enum component_id_t 
						defined in the identification_data.h file
	@param[in] component_info_t * componentsId	This is were the info of the 
						server side is saved as a type of component_info_t 
						described above, and it is used to register/share/validate
						the relevant info of each process
						
	@return Returns -1 if an error ocurred, 0 on success
*/
int8_t PostTid(const char connection_path[], uint8_t component, component_info_t * serverInfo);
/**
	@brief SetRxOn()	This function will set a socket server in the function 
						that calls it, the intend is that this function in 
						addition to SetTxOn() will provide the inter-process
 						communication between two processes sharing the same 
 						"connection_path". Once the connection is stablished, 
 						both processes should be able to perform read/write 
 						operations in the socket file using RxMsg() and TxMsg() 
 						functions (defined in this file).
 						NOTE: SetRxOn() and SetTxon() are intended to use in 
 						separate files.
	@param[in] const char connection_path[] The name of the socket file to 
						"listen" on.
	@return On error this function return -1, on Succes the listening socket fd 
			is return
*/
int8_t SetRxOn(const char connection_path[]);
/**
	@brief RxMSg() 	This function will read the contents of the socket with the
					file descriptor "socket_fd", and store the data in the 
					previously allocated memory at "data" the size in bytes of 
					"dataSz"
	@param[in] int8_t socket_fd A socket file descriptor previously created
					with a call to SetRxOn() function.
	@param[in] uint8_t * data A pointer to a previously allocated memory 
					(usually a char array)
	@param[in] uint8_t dataSz The size in bytes of the allocated memory
	@return  On error this function return -1, on Succes 0 is returned

*/
int8_t RxMsg(int8_t socket_fd, uint8_t * data, uint8_t dataSz);
/**
	@brief SetTxOn() 	This function will set a client socket in the function
						that calls this function, the intendi s that this 
						function in addition to SetRxOn() will provide the 
						inter-process communication between two processes 
						sharing the same "connection_path". Once the connection 
						is stablished both processes should be able to perform
						read/write operations in the socket file using RxMsg() 
						and TxMsg() functions.
 	@param[in] const char connection_path[] The name of the socket file the 
 						"client" socket will attemp to connect.
	@return On error this function return -1, on Succes 0 is returned
*/
int8_t SetTxOn(const char connection_path[]);
/**
	@brief TxMsg() 	This function will write the content of "data" into the 
					socket file refered by socked_fd
	@param[in] int8_t socket_fd	The file descriptor of the socket file you want 
					to write
	@param[in] pid_t tid The tid of the process you want to send the message to
	@param[in] uint8_t ImComponent The "user defined" id as defined in 
					identification_data.h
	@param[in] uint8_t * data A pointer to the data you wish to transmit, be 
					aware that you are expected to use a unsigned char array
					in order to mantain a proper communication using this API
	@param[in] uint8_t dataSz The size in bytes of the message you wish to send
	@param[in] bool cb Wheter or not a callback should be executed when the 
					message is received.
	@return On error this function return -1, on Succes 0 is returned
*/
int8_t TxMsg(int8_t socket_fd, pid_t tid, uint8_t ImComponent, uint8_t * data, uint8_t dataSz, bool cb);
/**
	@brief WaitSemaphore() 	This function will wait till the specified semaphore
							is available.
	@param[in] const char named_semaphore[] the name of the semaphore to wait 
							for.
	@return On error this function return -1, on Succes 0 is returned.
*/
int8_t WaitSemaphore(const char named_semaphore[]);
/**
	@brief PostSemaphore 	This function will increment the semaphore specified 
							in named_semaphore, the amount specified in 
							semaphores_count
	@param[in] const char named_semaphore[] The name of the sempahore to be 
							posted.
	@param[in] uint8_t semaphores_count the aomuont the count will be 
							incremented.
	@return  On error this function return -1, on Succes 0 is returned
*/
int8_t PostSemaphore(const char named_semaphore[], uint8_t semaphores_count);