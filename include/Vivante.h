/**************************************************************************************/
/*!
 *    \file     Vivante.h
 *
 *    \copyright 2016
 *
 *    \brief    Header file containing the public interfaces and data for the application 
 *				component loading functions.
 *
 *    \author   D. Kageff
 *
 *    \version  $Revision: 1.2 $  
 *				$Log $
 *
 */
/***************************************************************************************/
#ifndef _VIVANTI_H
 #define _VIVANTI_H
#endif

/***********************************
	Public Macros and Typedefs
***********************************/
#define FixedFromInt(n)	(GLfixed) ((n) << 16)
#define ZERO			FixedFromInt(0)
#define ONE				FixedFromInt(1)

/* Main Vivnte demo function */
int StartVivanteDemo(void);
