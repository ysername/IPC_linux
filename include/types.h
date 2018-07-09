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
*  File:  types.h 
*
*  Author: Sergei Stelmak
*
*  Description: Typical types definitions
*
*  Modification history:
*   $Log: types.h  $
*   Revision 1.6 2017/04/25 16:13:01CDT Katherine May (10014495) 
*   Updated per MISRA review: 2018_FFHUD_PRR_VP_MISRA_17.xls
*   Revision 1.5 2017/02/15 16:19:19EST Katherine May (10014495) 
*   Updated per MISRA review: 2018_FFHUD_PRR_VP_MISRA_1.xls
*   Revision 1.4 2016/12/21 10:08:50EST Katherine May (10014495) 
*   Added min type values
*   Revision 1.3 2016/12/19 11:02:49EST Katherine May (10014495) 
*   Added some max type values
*   Revision 1.2 2016/06/24 18:41:11EDT Sergio Espinoza Lopez (10012599) 
*   adding some useful types
*   Revision 1.1 2016/03/09 14:30:44CST Sergio Espinoza Lopez (10012599) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/VP/BSP/Include/project.pj
********************************************************************************************/
#ifndef TYPES_H
#define TYPES_H
#include <stdint.h>


typedef unsigned char   byte;
typedef unsigned short  word;
typedef unsigned long   doubleword;
typedef unsigned char   Boolean;
typedef unsigned char   bool;

#ifndef uint8_t
typedef signed char        int8_t;
typedef signed short       int16_t;  
typedef signed int         int32_t; 
//typedef signed long long   int64_t;  
typedef unsigned char      uint8_t;  
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t; 
//typedef unsigned long long uint64_t; 
#endif  /*uint8_t commented because linux already defined this types*/

/******************************************************************************
  Size    Unsigned      Signed
  ------------------------------
  8-Bit   BOOL          -
  8-Bit   BYTE          SHORTINT
  16-Bit  WORD          INTEGER
  32-Bit  LONGWORD      LONGINT
******************************************************************************/
typedef unsigned char BYTE;
typedef char SHORTINT;
typedef unsigned short WORD;
typedef short INTEGER;
typedef unsigned long LONGWORD;
typedef long LONGINT;
typedef unsigned char BOOL;


typedef void (*FuncPtrType)(void);

#ifndef false
#define false   (0)
#endif

#ifndef true
#define true    (1)
#endif

#ifndef FALSE
#define FALSE   ((BOOL)0)
#endif

#ifndef TRUE
#define TRUE    ((BOOL)1)
#endif

#ifndef NULL
#define NULL    ((void *)0) 
#endif

#ifndef IS_SUCCESS
#define IS_SUCCESS  (0)
#endif

#ifndef IS_FAILURE
#define IS_FAILURE  (-1)
#endif

#define	MAX_UNSIGNED_CHAR   (BYTE)(0xFFU)
#define	MAX_UNSIGNED_SHORT  (WORD)(0xFFFFU)
#define	MAX_UNSIGNED_LONG   (LONGWORD)(0xFFFFFFFFU)
#define	MAX_SIGNED_CHAR     (SHORTINT)(0x7F)
#define	MAX_SIGNED_SHORT    (INTEGER)(0x7FFF)
#define	MAX_SIGNED_LONG     (LONGINT)(0x7FFFFFFF)

#define	MIN_UNSIGNED_CHAR   (BYTE)(0U)
#define	MIN_UNSIGNED_SHORT  (WORD)(0U)
#define	MIN_UNSIGNED_LONG   (LONGWORD)(0U)
#define	MIN_SIGNED_CHAR     (SHORTINT)(-128)
#define	MIN_SIGNED_SHORT    (INTEGER)(-32768)
#define	MIN_SIGNED_LONG     (LONGINT)(-2147483648)

#endif
/* End of file */
