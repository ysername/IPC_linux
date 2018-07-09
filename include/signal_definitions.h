#ifndef _SIGNALS_DEFINITIONS_H_
#define _SIGNALS_DEFINITIONS_H_
#endif
#include <signal.h>

#define CB_FALSE 		SIGRTMIN
#define CB_TRUE			SIGRTMIN + 1
#define CLKSETALARM1	SIGRTMIN + 2
#define CLKSETALARM2	SIGRTMIN + 3

/*
typedef enum
{
	CB_FALSE 	= SIGRTMIN, //use this flag when the receiver process of a message must execute a cb upon receive
	CB_TRUE,				//use this flag when the receiver process of a message must not execute a cb upon receive
	CLKSETALARM 
}named_rt_signals_t;*/