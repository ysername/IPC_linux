/*****************************************************************************
 * Copyright (C) 2010 Yazaki North America
 *
 * License: Yazaki License, All Rights Reserved
 *
 * This document is protected by the copyright law of the United States and
 * foreign countries.  The work embodied in this document is confidential and
 * constitutes a trade secret of Yazaki North America, Inc.  Any copying of
 * this document and/or any use or disclosure of this document or the work
 * embodied in it without the written authority of Yazaki North America, Inc.,
 * will constitute copyright and trade secret infringement and will violate the
 * rights of Yazaki North America, Inc.
******************************************************************************/
/* Standard multi-include avoidance */
#ifndef __SPI_LIB_TYPES_H__
#define __SPI_LIB_TYPES_H__


/******************************************************************************
 * Title: SPI Library - spi_lib_types.h
 *		This file contains type definitions for the common SPI communications 
 *	    library used on the the VP and GP.  
 *
******************************************************************************/

/******************************************************************************/
/*     I N C L U D E   F I L E S                                              */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*     M A C R O S                                                            */
/******************************************************************************/

/******************************************************************************/
/*     T Y P E S   A N D   E N U M E R A T I O N S                            */
/******************************************************************************/

/* 
** Note: For now we know both VP and GP are 32-bit processors so the following
** type definitions are sufficient.  In the future we may need to support 
** non-32-bit architectures.  Those cases will be handled when needed.
*/
typedef unsigned char U8;
typedef unsigned short int U16;
typedef unsigned int U32;
typedef short int S16;
typedef int S32;

#ifndef BOOL
typedef unsigned char BOOL;
#endif

#ifndef FALSE
#define FALSE	(0)
#endif

#ifndef TRUE
#define TRUE	(1)
#endif

/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/



/******************************************************************************/
/*     F U N C T I O N   P R O T O T Y P E S                                  */
/******************************************************************************/



#ifdef __cplusplus
}
#endif

#endif /* __SPI_LIB_TYPES_H__ */
