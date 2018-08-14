/**
	@file signal_definitions.h
	@version 1.0
	@date 8/8/2018
	@author Leonardo Reatiga Miranda
	@brief 	This file is used in order to maintain the use of the real-time 
			signals provided by the linux kernel in a organized way. This file 
			is specially relevant for the clk_api_linux.h and the 
			msg_apisignals.h files.
*/
#ifndef _SIGNALS_DEFINITIONS_H_
#define _SIGNALS_DEFINITIONS_H_
#endif
#include <signal.h>

#define CB_FALSE 		SIGRTMIN
#define CB_TRUE			SIGRTMIN + 1
#define CLKSETALARM1	SIGRTMIN + 2
#define CLKSETALARM2	SIGRTMIN + 3
