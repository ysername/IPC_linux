/*******************************************************************************
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
*******************************************************************************/
/********************************************************************************************
*  File:  util.h
*
*  Author: Sergei Stelmak
*
*  Description: Utility functions
*
*  Modification history:
*   $Log: util.h  $
*   Revision 1.9 2017/04/26 12:55:59CDT Katherine May (10014495) 
*   Updated per MISRA review: 2018_FFHUD_PRR_VP_MISRA_15.xls
*   Revision 1.8 2017/04/24 17:46:09EDT Isaias Acosta Martinez (10014447) 
*   Replace memcpy from string.h with memcopy from util.c
*   * A macro is added into util.h to replace any reference of memcpy to memcopy
*   * A macro is added into util.h to replace any reference of memset to memoryset
*   * Remove any reference to string.h
*   Revision 1.7 2017/02/16 09:14:22CST Katherine May (10014495) 
*   Updated per MISRA review: 2018_FFHUD_PRR_VP_MISRA_1.xls
*   Revision 1.6 2017/01/11 10:40:24EST Sergio Espinoza Lopez (10012599) 
*   Create / Add dtc mgr unit tests
*   Better expose some of the databse configuration types
*   Revision 1.5 2016/07/25 09:28:05CDT Eudora Gunarta (10031829) 
*   Added caution statement to DelayMicroseconds banner.
*   Revision 1.4 2016/05/18 11:42:11EDT Fernando Villarreal Garza (10011234) 
*   Added Cangateway to the project 
*   **working on the interaction from CAN To SPI
*   Revision 1.3 2016/05/05 16:11:24CDT Fernando Villarreal Garza (10011234) 
*   Added macros to use with Cpp and changed memcopy to unsigned inputs
*   Revision 1.2 2016/03/17 10:48:42CST Sergio Espinoza Lopez (10012599) 
*   Eudora's PRE_CHECKOUT release
********************************************************************************************/
#ifndef _UTIL_H
#define _UTIL_H

/****************************************************************************
*    Include files                                                          *
*****************************************************************************/
#include "types.h"

/****************************************************************************
*    Description:        Delay functions									*
*****************************************************************************/

#ifndef UNIT_TEST
void wait_loop_us(unsigned int time_us);
#endif

/****************************************************************************
*    Function:           WordWordTableInterpolation                         *
*    Description:        Gets a table with XY points where X is argument and*
*                        Y is result, finds two pairs of XY so a_Argument   *
*                        fits between Xs and calculates Y for the given     *
*                        a_Argument. XY points are words.                   *
*                        X in the table must be incremental                 *
*****************************************************************************/
unsigned short WordWordTableInterpolation 
								( 	unsigned short	x_Arg,
                               		unsigned short	Size,
									unsigned short	X_Ref[],
                               		unsigned short	Y_Ref[]);

#ifdef __cplusplus
extern "C"
#endif

/****************************************************************************
*  Function Name: memcopy
*
*  Description: Copies count bytes from memory area Source to memory area Dest.
*               The memory areas must not overlap
*    
*  Input(s):    Dest   - Destintation buffer
*               Source - Source buffer
*               count  - Count of bytes to copy
*
*  Output(s):   None.
*
*  Returns:     An integer less than, equal to, or greater than zero if the
*               first n bytes of ptr1 is found, respectively, to be less than,
*               to match, or be greater than the first n bytes of ptr2.
*****************************************************************************/
void memcopy(void *Dest, void *Source, int count);

#define memcpy(Dest, Source, count) memcopy(Dest, Source, count)

/****************************************************************************
*  Function Name: memcomp
*
*  Description: Compares the first n bytes (each interpreted as unsigned char) of the memory
*    
*  Input(s):    Pointer to the data to compare
*
*  Output(s):   None.
*
*  Returns:     An integer less than, equal to, or greater than zero if the
*               first n bytes of ptr1 is found, respectively, to be less than,
*               to match, or be greater than the first n bytes of ptr2.
*****************************************************************************/
int memcomp(void *ptr1, void *ptr2, int count);

#define memcmp(ptr1, ptr2, count) memcomp(ptr1, ptr2, count)


/****************************************************************************
*    Function:           FastMemorySet                                      *
*    Description:        This function is to stuff a memory area with the   *
*                        same value.                                        *
*****************************************************************************/
void FastMemorySet(unsigned char* p8, unsigned char Pattern8, unsigned int	Size8);

#ifdef __cplusplus
extern "C"
#endif

/****************************************************************************
*  Function Name: WriteProtected                                            *
*  Description: Writes to requested protected register with procedure       *
*                 described in section 3.15 of DJ4 user manual.             *
*               This function will continuously re-try the write operation  *  
*                 if write failed or was interrupted by an interrupt.       *
*               This function protects against multiple calls that might    * 
*                 happen at the same time (OS tasks scheduling is running); * 
*                 however, in this case it should only be used when a       *
*                 minimal number of protected registers need to be written. *
*  Input(s):    protreg - protection command register                       *
*               protstat - protection status register                       *
*               reg - protected register                                    *
*               value - value to be written to protected register           *
*  Output(s):   None                                                        *
*  Returns:     None                                                        *
*****************************************************************************/
void WriteProtected( volatile unsigned long * protreg,
                     volatile unsigned char * protstat,
                     volatile unsigned long * reg,
                     unsigned long value );

/****************************************************************************
*  Function Name: DelayMicroseconds
*
*  Description: Delays for the specified number of microseconds then returns.
*               CAUTION when delay value is > watchdog timeout. If this is the
*                 case do delay in increments of a value < watchdog timeout
*                 and refresh watchdog in between delays.
*    
*  Input(s):    microseconds - number of microseconds to delay
*
*  Output(s):   None.
*
*  Returns:     None.
*****************************************************************************/
void DelayMicroseconds(unsigned long microseconds);

/****************************************************************************
*  Function Name: memset
*
*  Description: set a value into an array. 
*    
*  Input(s):    Pointer to the data, value to be copy, number of copies
*
*  Output(s):   None.
*
*  Returns:     None.
*****************************************************************************/
void memoryset(void *Dest, unsigned char  value, int count);

#define memset(Dest, value, count) memoryset(Dest, value, count)

#endif
/* End of file */
