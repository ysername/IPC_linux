#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>        
#include "types.h"
#include "signal_definitions.h"

typedef struct{
	uint8_t Component;
	pid_t 	Tid;
	int8_t 	Fd;
}component_info_t;

int8_t GetTids(const char connection_path[], uint8_t  components[], uint8_t connectionsToWait,  component_info_t * componentsId, uint8_t ImComponent);
int8_t PostTid(const char connection_path[], uint8_t component, component_info_t * serverInfo);
int8_t SetRxOn(const char connection_path[]);
int8_t RxMsg(int8_t socket_fd, uint8_t * data, uint8_t dataSz);
int8_t SetTxOn(const char connection_path[]);
int8_t TxMsg(int8_t socket_fd, pid_t tid, uint8_t ImComponent, uint8_t * data, uint8_t dataSz, bool cb);
int8_t WaitSemaphore(const char named_semaphore[]);
int8_t PostSemaphore(const char named_semaphore[], uint8_t semaphores_count);