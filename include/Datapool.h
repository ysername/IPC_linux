/**************************************************************************************/
/*!
 *  \file		Datapool.h
 *
 *  \copyright	Yazaki 2016
 *
 *  \brief		Datapool and datapool read/write functions.
 *
 *  \author		
 *
 *  \version	$Revision: 1.3 $  $Log $
 *
 *
 **************************************************************************************/
#ifndef _DATAPOOL_H
#define _DATAPOOL_H

/***********************************
		       Include Files
***********************************/
#include "pool_def.h"		// Datapool item definitions */

/***********************************
	  Public Macros and Typedefs
***********************************/

/***********************************
	        Public Config Macros
***********************************/

/***********************************
	  Public Data and Structures
***********************************/

/***********************************
	  Public Function Prototypes
***********************************/
/* Initialize datapool items to default values */
gp_retcode_t InitPool(void);

/* Return the type and data length of the requested datapool item. */
gp_retcode_t GetElemInfo(int id, GP_DATATYPES_T *p_type, int *p_len);

/* Set the data from a datapool item */
gp_retcode_t SetElem(int id, void *p_value);

/* Retrieve the data from a datapool item */
gp_retcode_t GetElem(int id, void *p_value);

/* Copy the datapool storage to the datapool image pointed to by p_data */
gp_retcode_t SetPool(DP_ITEM_STORAGE_T *p_data);

/* Copy the datapool image to the storage pointed to by p_data */
gp_retcode_t GetPool(DP_ITEM_STORAGE_T *p_data);

/************* Legacy functions *****************/
/* 	  These will eventually be eliminated 		*/
/************************************************/

/* Write unsigned 32 bit integer to the datapool element. */
int32_t SetElem32(int id, int32_t value);

/* Read unsigned 32 bit integer from the datapool element. */
int32_t GetElem32(int id, uint32_t *value);


#endif /* _DATAPOOL_H */
