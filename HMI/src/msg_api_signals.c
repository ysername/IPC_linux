#include "msg_api_signals.h"
#include <semaphore.h>
#include <fcntl.h>  
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
extern uint8_t component;

#define MAX_NUM_COMPONENTS  4
#define INVALID_CONNECTION  -1
#define MAX_NUM_MSGS        2

/**************************************************************************************/
/*! \fn void WaitForTids(const char connection_path[], uint8_t  components[], uint8_t connectionsToWait,  component_info_t * componentsId)
 *
 *  param[in] 
 *      -connection_path[]: this is the name of the socket file the "server" side will
 *                          be listen to connections
 *      -components[]:      an array containing the components expected to set a 
 *                          connection.
 *      -connectionsToWait: the size of the array "components"
 *      -componentsId:      an array of structs containing the tid and a "component" id
 *
 *  \par Description:
 *      This function will set a socket server in the function that calls it, waiting 
 *      for the number of processes specified in "conectionToWait" to send a message, 
 *      speciying its own tid and "component" id, the function will
 *      only recognize a valid "ready" message when the process that send the message 
 *      is listed in the "components" array, and thus this function, besides acting as
 *      a way to get the tid of the processes at runtime(its main purpose), could also
 *      be use as aa semaphore.
 *      The array of structs component_info_t, relates the tid with his corresponding
 *      "component" id.
 *  \retval 
 *      -1 on error, 0 on success
 *
 *  \par Limitations/Caveats:
 *  there is no timeout argument.
 *
 *  TODO: Add a timeout
 **************************************************************************************/
int8_t GetTids(const char connection_path[], uint8_t  components[], uint8_t connectionsToWait,  component_info_t * componentsId, uint8_t ImComponent) {

    pid_t bufferRead[2];
    pid_t bufferWrite[2];
    uint32_t i;
    struct sockaddr_un address; 
    int socket_fd, connection_fd;
    socklen_t address_length;
    uint8_t component_found;
    
    socket_fd = socket(PF_UNIX, SOCK_STREAM,0); //create an unix domain socket
    if(socket_fd < 0)
    {
        printf("socket() failed: %s\n", strerror(errno));
        return -1;
    }

    unlink((const char *)connection_path);      //ensure the path for the connection is clean
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,256, "%s",connection_path);  
    
    if(bind(socket_fd,                          //bind the socket
           (struct sockaddr *) &address,
           sizeof(struct sockaddr_un)) != 0)
    {
        printf("bind() failed: %s\n", strerror(errno));
        return -1;
    }

    if(listen(socket_fd, 5) != 0)    //set the server socket to the listen state
    {
        printf("listen() failed: %s\n", strerror(errno));
        return -1;
    }
    printf("my tid is: %lu\n", syscall(SYS_gettid) );
    bufferWrite[0] = syscall(SYS_gettid);
    bufferWrite[1] = ImComponent;
    address_length = sizeof(address);
    do{
        printf("socket_fd is: %d\n",socket_fd );
        connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length);
        if(connection_fd == -1){
            printf("accept() failed: %s\n",strerror(errno) );
            return -1;
        }
        /*read the content of the connection*/
        memset(&bufferRead[0],0,sizeof(bufferRead));
        if(read(connection_fd, &bufferRead[0],sizeof(bufferRead)) == -1){
            printf("read() failed: %s\n",strerror(errno) );
        }

        component_found = 0;
        
        for(i = 0; i < MAX_NUM_COMPONENTS; i++){
            if(bufferRead[1] == components[i]){
                if(write(connection_fd, &bufferWrite[0],sizeof(bufferWrite)) == -1){
                    printf("write() failed: %s\n",strerror(errno) );
                }
                connectionsToWait--;
                componentsId[i].Tid = bufferRead[0];
                componentsId[i].Component = bufferRead[1];
                component_found = 1;
                printf("pending connections: %d\n", connectionsToWait );
                break;
            }
        }

        if(component_found == 0){
            bufferWrite[0] = INVALID_CONNECTION;
            if(write(connection_fd, &bufferWrite[0],sizeof(bufferWrite)) == -1){
                printf("write() failed: %s\n",strerror(errno) );
            }
        }
        
    }while((connectionsToWait > 0) && (connection_fd > -1));
    errno = fsync(socket_fd);
    /*once the message is send, close the client socket, so the server socket can handle new connections*/
    errno = close(socket_fd);
    return 0;
}

/**************************************************************************************/
/*! \fn void WaitForComponents(const char connection_path[], uint8_t  components[], uint8_t connectionsToWait,  component_info_t * componentsId)
 *
 *  param[in] 
 *      -connection_path[]: this is the name of the socket file the "server" side will
 *                          be listen to connections
 *      -components[]:      an array containing the components expected to set a 
 *                          connection.
 *      -connectionsToWait: the size of the array "components"
 *      -componentsId:      an array of structs containing the tid and a "component" id
 *
 *  \par Description:
 *      This function will set a socket server in the function that calls it, waiting 
 *      for the number of processes specified in "conectionToWait" to send a message, 
 *      speciying its own tid and "component" id, the function will
 *      only recognize a valid "ready" message when the process that send the message 
 *      is listed in the "components" array, and thus this function, besides acting as
 *      a way to get the tid of the processes at runtime(its main purpose), could also
 *      be use as aa semaphore.
 *      The array of structs component_info_t, relates the tid with his corresponding
 *      "component" id.
 *  \retval 
 *      None.
 *
 *  \par Limitations/Caveats:
 *  there is no timeout argument.
 *
 *  TODO: Add a timeout
 **************************************************************************************/
int8_t PostTid(const char connection_path[], uint8_t component, component_info_t * serverInfo){
    
    pid_t buffer[2];
    uint32_t i;
    struct sockaddr_un address; 
    int socket_fd, connection_fd;
    socklen_t address_length;
    uint8_t pending_connections = 1;
    printf("sizeof buffer is: %lu\n",sizeof(buffer) );
    
    socket_fd = socket(PF_UNIX, SOCK_STREAM,0); //create an unix domain socket
    if(socket_fd < 0)
    {
        printf("socket() failed: %s\n", strerror(errno));
        return -1;
    }

    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,256, "%s",connection_path);  
    
    if(connect(socket_fd,                          //bind the socket
           (struct sockaddr *) &address,
           sizeof(struct sockaddr_un)) != 0)
    {
        printf("connect() failed: %s\n", strerror(errno));
        return -1;
    }
    printf("my tid is: %lu\n", syscall(SYS_gettid) );
    memset(&buffer[0],0,sizeof(buffer));
    
    buffer[0] = syscall(SYS_gettid);
    buffer[1] = component;
    if((write(socket_fd,&buffer[0],sizeof(buffer))) == -1){
        printf("%s\n",strerror(errno) );
        return -1;
    }
    if((read(socket_fd,&buffer[0],sizeof(buffer))) == -1){
        printf("%s\n",strerror(errno) );
        return -1;
    }
    serverInfo->Tid = buffer[0];
    serverInfo->Component = buffer[1];
    fsync(socket_fd);
    close(socket_fd);
    for (int i = 0; i < sizeof(buffer)/sizeof(pid_t); ++i)
   
    return 0;
}    
/**************************************************************************************/
/*! \fn int8_t SetRxOn(const char connection_path[])
 *
 *  param[in] 
 *		-connection_path[]: this is the name of the socket file the "server" side will
 *							be listen to connections
 *
 *  \par Description:
 *		This function will set a socket server in the function that calls it, the intend
 *		is that this function in addition to SetTxOn() will provide the inter-process
 *		communication between two processes sharing the same "connection_path".
 *		once the connection is stablished, both processes should be able to perform
 *		read/write operations in the socket file using RxMsg() and TxMsg() functions.
 *		NOTE: do not use SetTxOn() function on the same file with the same argument for 
 *		connection_path, SetRxOn() and SetTxon() are intended to use in separate files.
 *  
 *  \retval 
 *		On error this function return -1, on Succes the listening socket fd is return
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO: set the connection _fd as non_blocking
 **************************************************************************************/
int8_t SetRxOn(const char connection_path[]){
    
    struct sockaddr_un address; 
    int socket_fd,connection_fd;
    socklen_t address_length;

    socket_fd = socket(PF_UNIX, SOCK_STREAM,0); //create an unix domain socket
    if(socket_fd < 0){
        printf("\n socket() failed: %s\n", strerror(errno));
        return -1;
    }

    unlink((const char *)connection_path);      //ensure the path for the connection is clean
    memset(&address, 0, sizeof(struct sockaddr_un));

    address.sun_family = AF_UNIX;
    snprintf(address.sun_path,256, "%s",connection_path);  
    //bind the socket
    if(bind(socket_fd,(struct sockaddr *) &address,
           sizeof(struct sockaddr_un)) != 0){
        printf("\n bind() failed: %s\n", strerror(errno));
        return -1;
    }

    if(listen(socket_fd, MAX_NUM_MSGS) != 0)    //set the server socket to the listen state
    {
        printf("\n listen() failed: %s\n", strerror(errno));
        return -1;
    }
    connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length);
    if(connection_fd < 0){
    	printf("\n accept() failed: %s\n", strerror(errno));
    }
    
    return connection_fd;        
}
/**************************************************************************************/
/*! \fn int8_t RxMsg(const char connection_path[])
 *
 *  param[in] 
 *		-socked_fd:			the file descriptor of the socket file you want to read from
 *		-data:				a pointer in which you will receive the content of the reading
 *		-dataSz:			the expected size in bytes of the message
 *
 *
 *  \par Description:
 *		This function will read the content of the file descriptor socket_fd and will 
 *		store it in data, data should be a pointer with previously allocated memory
 *  
 *  \retval 
 *		On error this function return -1, on Succes 0 is returned
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO:
 **************************************************************************************/
int8_t RxMsg(int8_t socket_fd, uint8_t * data, uint8_t dataSz){
	if(read(socket_fd, data ,dataSz) != dataSz){
		printf(" write() failed: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void EncodeMsg(uint8_t * dataToEncode,uint8_t bufSz, bool cb){
    
    uint8_t tmpBuf[bufSz+1];
    memset((void *)&tmpBuf[0], 0, sizeof(tmpBuf));//start with a clean tmp buffer
    memcpy(&tmpBuf[0], dataToEncode, bufSz);//copy the data to be transmited to the tmp buffer
    if(cb){
    	memset((void *)dataToEncode, 1, 1);
    }else{
    	memset((void *)dataToEncode, 0, 1);
    }
    memcpy((void *)dataToEncode+1, (const void *)&tmpBuf[0], bufSz+1);  
}
/**************************************************************************************/
/*! \fn int8_t SetTxOn(const char connection_path[])
 *
 *  param[in] 
 *		-connection_path[]: this is the name of the socket file the "client" side will
 *							attempt to connect
 *
 *  \par Description:
 *		This function will set a client socket in the function that calls it, the intend
 *		is that this function in addition to SetRxOn() will provide the inter-process
 *		communication between two processes sharing the same "connection_path".
 *		once the connection is stablished both processes should be able to perform
 *		read/write operations in the socket file using RxMsg() and TxMsg() functions.
 *		NOTE: do not use SetRxOn() function on the same file with the same argument for 
 *		connection_path, SetRxOn() and SetTxon() are intended to use in separate files.
 *  
 *  \retval 
 *		On error this function return -1, on Succes 0 is returned
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO:
 **************************************************************************************/
int8_t SetTxOn(const char connection_path[]){

    struct sockaddr_un address; 
    int socket_fd;
    socklen_t address_length;
    /*sending the message*/
    socket_fd = socket(PF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);/*first we create the socket*/
    if(socket_fd == -1 ){
        printf("\n socket() failed: %s\n", strerror(errno));
        return -1;
    }
    /*set the address of the connection of the socket*/
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    
    snprintf(address.sun_path, 256,"%s", (const char *)connection_path);
    /*make the connection*/
    if(connect(socket_fd,
            (struct sockaddr *) &address,
            sizeof(struct sockaddr_un)) == -1)
    {
        printf("\n connect() failed: %s\n", strerror(errno));
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}
/**************************************************************************************/
/*! \fn int8_t TxMsg(const char connection_path[])
 *
 *  param[in] 
 *		-socked_fd:			the file descriptor of the socket file you want to write
 *		-data:				a pointer in which you will store the data to be transmited
 *		-dataSz:			the size in bytes of the message
 *		-cb:				a flag indicating wheter or not a Callback should be executed
 *
 *  \par Description:
 *		This function will write the content of "data" into the socket file refered by  
 *		socked_fd
 *  
 *  \retval 
 *		On error this function return -1, on Succes 0 is returned
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO:
 **************************************************************************************/
int8_t TxMsg(int8_t socket_fd, pid_t tid, uint8_t ImComponent, uint8_t * data, uint8_t dataSz, bool cb){
	int8_t rc = 0;
    union sigval component;
    component.sival_int = ImComponent;
    if (cb)
    {
        rc = write(socket_fd, data ,dataSz);
        if(rc != dataSz){
            printf("TxMsg of component (%d), write() failed: %s\n", ImComponent, strerror(errno));
            return -1;
        }else{
            sigqueue(tid, CB_TRUE, (const union sigval)component);    
        }
    }else{
        rc = write(socket_fd, data ,dataSz);
        if(rc != dataSz){
            printf("TxMsg of component (%d), write() failed: %s\n", ImComponent, strerror(errno));
            return -1;
        }else{
            sigqueue(tid, CB_FALSE, (const union sigval)component);    
        }
    }
	printf("%s, rc = %d\n", __FUNCTION__, rc);
	return 0;
}

/**************************************************************************************/
/*! \fn int8_t WaitSemaphore(const char named_semaphore[])
 *
 *  param[in] 
 *      -named_semaphore:   the name of the semaphore to wait for
 *
 *  \par Description:
 *      This function will wait till the specified semaphore is available
 *  
 *  \retval 
 *      On error this function return -1, on Succes 0 is returned
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO:
 **************************************************************************************/
int8_t WaitSemaphore(const char named_semaphore[]){
    
    int rc;
    sem_t * pSemaphore;
    mode_t semaphore_mode = S_IRGRP | S_IWGRP;
    
    pSemaphore = sem_open(named_semaphore, O_CREAT, semaphore_mode,0);
    if(pSemaphore == SEM_FAILED){
        printf("couldn't create/open dp_semaphore in Hmi_mgr_as%s\n",strerror(errno));
        rc = -1;
    }else{
        if((rc = (sem_wait(pSemaphore))) == -1){
            printf("error while waiting for semaphore!%s\n",strerror(errno));
            sem_unlink(named_semaphore);
            return rc;
        }else{
            sem_unlink(named_semaphore);
            printf("processes are ready!%i\n",rc);
            return rc;
        }
    }
}
/**************************************************************************************/
/*! \fn int8_t WaitSemaphore(const char named_semaphore[])
 *
 *  param[in] 
 *      -named_semaphore:   the name of the semaphore to increment
 *      -semaphores_count:  the amount the sempahore will be increased
 *
 *  \par Description:
 *      This function will increment the semaphore specified in named_semaphore, the 
 *      amount specified in semaphores_count
 *  
 *  \retval 
 *      On error this function return -1, on Succes 0 is returned
 *
 *  \par Limitations/Caveats:
 *  None (yet).
 *
 *  TODO:
 **************************************************************************************/
int8_t PostSemaphore(const char named_semaphore[], uint8_t semaphores_count){
    
    int rc;
    sem_t * pSemaphore;
    mode_t semaphore_mode = S_IRGRP | S_IWGRP;

    pSemaphore = sem_open(named_semaphore, O_CREAT, semaphore_mode, 0);
    if(pSemaphore != SEM_FAILED){
        for (int i = 0; i < semaphores_count; ++i)
        {
            sem_post(pSemaphore);
        }    
        sem_unlink(named_semaphore);
        return 0;
    }else{
        printf("error, could not post semaphore!\n");
        sem_unlink(named_semaphore);
        return -1;
    }
}