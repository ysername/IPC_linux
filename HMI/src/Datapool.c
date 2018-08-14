/**************************************************************************************/
/*!
 *  \file		Datapool.c
 *
 *  \copyright	Yazaki 2016-17
 *
 *  \brief		Datapool and datapool read/write and management functions.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.8 $  
 *				$Log: Datapool.c  $
 *				Revision 1.8 2017/03/21 15:36:35CST Daniel Kageff (10011932) 
 *				Added GP_ARRAY type support to GetElem()
 *				Revision 1.7 2017/03/21 15:07:17EDT Daniel Kageff (10011932) 
 *				1) Corrected GP_STRING length handling
 *				2) Added GP_ARRAY handler case
 *				Revision 1.6 2017/03/02 12:50:51EST Eudora Gunarta (10031829) 
 *				Changes to match new names of YzTdWarpOn and YzTdWarpCalMode.
 *				Revision 1.5 2017/02/24 13:09:43EST Daniel Kageff (10011932) 
 *				Redesigned to handle multiple data types and add other features
 *
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	This optional comment section can be used to create an overview page for this 
 *	software component that is accessable from the navigation pane of the gnerated 
 *	design document.
 *
 */
/***************************************************************************************/
#define _DATAPOOL_C		/*!< File label definition */

/***********************************
		       Include Files
***********************************/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
//#include <bsd/string.h>

#include <gp_types.h>		// Yazaki GP processor type definitions
#include <gp_cfg.h>			// Common system configuration settings
#include <gp_utils.h>		// Common GP program utility functions

#include "pool_def.h"		// Datapool public definitions

/***********************************
	Private Macros and Typedefs
***********************************/
#define Success 0
/***********************************
	      Private Config Macros
***********************************/

/***********************************
	Private Data and Structures
***********************************/

/*! Locking semaphore used to read/write the datapool */
//static LocalMutex DataPoolLock = NULL;
static pthread_mutex_t dataPoolLock;
/*err = pthread_mutex_init(datapoolLock,NULL)
if(err != 0)
{
	printf("Failed to create mutex!");
}*/



/***********************************
	Private Function Prototypes
***********************************/
static void dpSetDfltVal(unsigned int id);


/************ Start of code ******************/
/*! \defgroup dpfcns_public Datapool access public API
 */

/**************************************************************************************/
/*! \fn InitPool(void)
 *
 *  \par Description:	  
 *  Initialize datapool items to default values.     
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 Shall only be called by IntTsk
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t InitPool(void)
{
    uint32_t err;

	/* Create the datapool access mutex */
    err = pthread_mutex_init((pthread_mutex_t *restrict)&dataPoolLock,NULL);
    if(err != Success) 
    {
		return GP_INIT_ERR;
    }

	/* Lock the datapool */	
	err = pthread_mutex_lock(&dataPoolLock);
	if(err != Success) 
	{
	    return GP_DP_ACCESS_ERR;
	}
	/* Clear the datapool */    
    memset((void *)&dp_data, 0, sizeof(DP_ITEM_STORAGE_T));

	/* Set specific default values */
	/*dpSetDfltVal(YzTdWarpMesh);			// WarpMesh
	dpSetDfltVal(YzTdWarpContent);		// WarpContent
	dpSetDfltVal(YzTdMirrorPosShort);	// MirrorPosShort
	dpSetDfltVal(YzTdMirrorPosMed);		// MirrorPosMed
	dpSetDfltVal(YzTdMirrorPosTall);	// MirrorPosTall
	dpSetDfltVal(YzTdMirrorPos);		// MirrorPos
	*/dpSetDfltVal(YzTdoNavSimFname);		// NavSimFname
	dpSetDfltVal(YzTdoAudioSimFname);	// AudioSimFname

	/* Release the datapool */ 
	err = pthread_mutex_unlock(&dataPoolLock);
	if(err != Success) 
	{
	    return GP_DP_ACCESS_ERR;
	}

    return GP_SUCCESS;
}

/**************************************************************************************/
/*! \fn GetElemInfo(int id, GP_DATATYPES_T *p_type, int *p_len)
 *
 *	\param[in] id 	  - Element id as defined in #DP_ELEMENT_IDS (pool_def.h)
 *	\param[in] p_type - Pointer to the datapool element type. Updated by this function.
 *	\param[in] p_len  -	Pointer to element length in bytes. Updated by this function.
 *						A length of zero is returned if the ID is invalid.
 *
 *  \par Description:	  
 *  Return the type and data length of the requested datapool item.     
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 None.
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t GetElemInfo(int id, GP_DATATYPES_T *p_type, int *p_len)
{
	/* If the ID is valid then return the type and data length */
    if(id <= ELEM_MAX_ID) 
    {
		*p_type = (GP_DATATYPES_T)dp_tbl[id].type;
		*p_len = dp_tbl[id].datlen;
		return GP_SUCCESS;
	}
	/* Else the ID is invalid */
	else
	{
		*p_len = 0;
		return GP_DP_PARMS_ERR;
	}
}

			
/**************************************************************************************/
/*! \fn SetElem(int id, void *p_value)
 *
 *	\param[in] id 	   - Element id as defined in #DP_ELEMENT_IDS (pool_def.h)
 *	\param[in] p_value - Void pointer to the new element value
 *
 *  \par Description:	  
 *  Set the datapool item to the value pointed to by p_value.  This function determines
 *	the data type and storage location using the datapool control table, dp_tbl.     
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 1) This function does not check if the supplied data is the proper data type.
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t SetElem(int id, void *p_value)
{
	gp_retcode_t retval = GP_SUCCESS;
    uint32_t err;

	/* If the parameters are valid */
    if((p_value != NULL) && (id >= ELEM_MIN_ID) && (id <= ELEM_MAX_ID))
    {
    	/* Lock the datapool */	
		err = pthread_mutex_lock(&dataPoolLock);
		if(err != Success) 
		{
		    return GP_DP_ACCESS_ERR;
		}

		/* Update the datapool item value based on its data type */
		switch(dp_tbl[id].type)
		{
			case GP_INT32:
				*(int *)(dp_tbl[id].p_data) = *(int *)p_value;
				break;

			case GP_UINT32:
				*(uint32_t *)(dp_tbl[id].p_data) = *(uint32_t *)p_value;
				break;

			case GP_INT64:
				*(int64_t *)(dp_tbl[id].p_data) = *(int64_t *)p_value;
				break;

			case GP_UINT64:
				*(uint64_t *)(dp_tbl[id].p_data) = *(uint64_t *)p_value;
				break;

			case GP_FLOAT:
				*(float *)(dp_tbl[id].p_data) = *(float *)p_value;
				break;

			case GP_DBL:
				*(double *)(dp_tbl[id].p_data) = *(double *)p_value;
				break;

			case GP_STRING:
				{
				 int len = strlen((char *)p_value);
				 if(len < dp_tbl[id].datlen)		// comparison allows for NULL char
				 {
					strlcpy((char *)dp_tbl[id].p_data, (char *)p_value, dp_tbl[id].datlen);
				 }
				 else
				 {
					retval = GP_DP_DATA_ERR;
				 }
				 break;
				}
			case GP_ARRAY:
				memcpy(dp_tbl[id].p_data, p_value, dp_tbl[id].datlen);
				break;

			case GP_INT16:
				*(int16_t *)(dp_tbl[id].p_data) = *(int16_t *)p_value;
				break;

			case GP_UINT16:
				*(uint16_t *)(dp_tbl[id].p_data) = *(uint16_t *)p_value;
				break;

			default:
				retval = GP_DP_DATA_ERR;
				break;
		}

		/* Release the datapool */ 
		err = pthread_mutex_unlock(&dataPoolLock);
		if(err != Success) 
		{
		    printf("\nReleaseLocalMutex() error %d\n", err);
		    retval = GP_DP_ACCESS_ERR;
		}
    }
	/* Else the datapool item ID is invalid */
	else
	{
		retval = GP_DP_PARMS_ERR;
	}
    return retval;
}

/**************************************************************************************/
/*! \fn GetElem(int id, void *p_value)
 *
 *	\param[in] id 	   - Element id as defined in #DP_ELEMENT_IDS (pool_def.h)
 *	\param[in] p_value - Void pointer to storage for the read element value
 *
 *  \par Description:	  
 *  Set the datapool item to the value pointed to by p_value. This function determines
 *	the data type and storage location using the datapool control table, dp_tbl.     
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 1) The caller must ensure that the destination provided is the proper size 
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t GetElem(int id, void *p_value)//needs minimum changes
{
	gp_retcode_t retval = GP_SUCCESS;
    uint32_t err;

	/* If the parameters are valid */
    if((p_value != NULL) && (id >= ELEM_MIN_ID) && (id <= ELEM_MAX_ID))
    {
    	/* Lock the datapool */	
		err = pthread_mutex_lock(&dataPoolLock);
		if(err != Success) 
		{
		    return GP_DP_ACCESS_ERR;
		}

		/* Update the datapool item value based on its data type */
		switch(dp_tbl[id].type)
		{
			case GP_INT32:
				*(int *)p_value = *(int *)(dp_tbl[id].p_data);
				break;

			case GP_UINT32:
				*(uint32_t *)p_value = *(uint32_t *)(dp_tbl[id].p_data);
				break;

			case GP_INT64:
				*(int64_t *)p_value = *(int64_t *)(dp_tbl[id].p_data);
				break;

			case GP_UINT64:
				*(uint64_t *)p_value = *(uint64_t *)(dp_tbl[id].p_data);
				break;

			case GP_FLOAT:
				*(float *)p_value = *(float *)(dp_tbl[id].p_data);
				break;

			case GP_DBL:
				*(double *)p_value = *(double *)(dp_tbl[id].p_data);
				break;

			case GP_STRING:
				{
				 int len = strlen((char *)dp_tbl[id].p_data);
				 if(len < dp_tbl[id].datlen)		// should never be an error
				 {
					strlcpy((char *)p_value, (char *)dp_tbl[id].p_data, len);
				 }
				 else
				 {
					retval = GP_DP_DATA_ERR;
				 }
				 break;
				}

			case GP_ARRAY:
				memcpy(dp_tbl[id].p_data, p_value, dp_tbl[id].datlen);
				break;

			case GP_INT16:
				*(int16_t *)p_value = *(int16_t *)(dp_tbl[id].p_data);
				break;

			case GP_UINT16:
				*(uint16_t *)p_value = *(uint16_t *)(dp_tbl[id].p_data);
				break;

			default:
				retval = GP_DP_DATA_ERR;
				break;
		}

		/* Release the datapool */ 
		err = pthread_mutex_unlock(&dataPoolLock);
		if(err != Success) 
		{
		    retval = GP_DP_ACCESS_ERR;
		}
    }
	/* Else datapool item ID is invalid */
	else
	{
		retval = GP_DP_PARMS_ERR;
	}
    return retval;
}

/**************************************************************************************/
/*! \fn SetPool(DP_ITEM_STORAGE_T *p_data)
 *
 *	\param[out] p_data - Pointer to the buffer to where the datapool image will be copied
 *						 from.
 *
 *  \par Description:	  
 *  Store the contents of the supplied datapool image pointed to by p_data into the
 *	datapool storage area.  Block size is set by ::DP_ITEM_STORAGE_T.     
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t SetPool(DP_ITEM_STORAGE_T *p_data)
{
    uint32_t err;

	/* Get access to the datapool */    
    err = pthread_mutex_lock(&dataPoolLock);
    if(err != Success) 
    {
		return GP_DP_ACCESS_ERR;
    }

	/* Copy datapool image to the datapool */	
	memcpy(&dp_data, p_data, sizeof(DP_ITEM_STORAGE_T));

	/* Release the datapool */
    err = pthread_mutex_unlock(&dataPoolLock);
    if(err != Success) 
    {
		return GP_DP_ACCESS_ERR;
    }

    return GP_SUCCESS;
}

/**************************************************************************************/
/*! \fn GetPool(DP_ITEM_STORAGE_T *p_data)
 *
 *	\param[out] p_data - Pointer to the buffer to where the datapool image will be copied.
 *
 *  \par Description:	  
 *  Retrieve the contents of the datapool into the supplied datapool area pointed to by 
 *	p_data. Block size is set by ::DP_ITEM_STORAGE_T.    
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\ingroup dpfcns_public
 **************************************************************************************/
gp_retcode_t GetPool(DP_ITEM_STORAGE_T *p_data)
{
    uint32_t err;

	/* Get access to the datapool */    
    err = pthread_mutex_lock(&dataPoolLock);
    if(err != Success) 
    {
		return GP_DP_ACCESS_ERR;
    }

	/* Copy datapool image to the datapool */	
	memcpy(p_data, &dp_data, sizeof(DP_ITEM_STORAGE_T));

	/* Release the datapool */
    err = pthread_mutex_unlock(&dataPoolLock);
    if(err != Success) 
    {
		return GP_DP_ACCESS_ERR;
    }

    return GP_SUCCESS;
}

/**************************************************************************************/
/*! \fn dpSetDfltVal(unsigned int id)
 *
 *	\param[in] id - element id
 *
 *  \par Description:	  
 *  Set the datapool element to its default value.
 *
 *  \returns none 
 *
 *  \par Limitations/Caveats:
 *	 1) Assumes that element ID is valid.
 *
 **************************************************************************************/
void dpSetDfltVal(unsigned int id)
{
	switch(dp_tbl[id].type)
	{
		case GP_INT32:
			*(int *)(dp_tbl[id].p_data) = *(int *)(dp_tbl[id].p_default);
			break;

		case GP_UINT32:
			*(uint32_t *)(dp_tbl[id].p_data) = *(uint32_t *)(dp_tbl[id].p_default);
			break;

		case GP_INT64:
			*(int64_t *)(dp_tbl[id].p_data) = *(int64_t *)(dp_tbl[id].p_default);
			break;

		case GP_UINT64:
			*(uint64_t *)(dp_tbl[id].p_data) = *(uint64_t *)(dp_tbl[id].p_default);
			break;

		case GP_FLOAT:
			*(float *)(dp_tbl[id].p_data) = *(float *)(dp_tbl[id].p_default);
			break;

		case GP_DBL:
			*(double *)(dp_tbl[id].p_data) = *(double *)(dp_tbl[id].p_default);
			break;

		case GP_STRING:
			strlcpy((char *)dp_tbl[id].p_data, (char *)(dp_tbl[id].p_default), strlen((char *)(dp_tbl[id].p_default)));
            break;
            
		case GP_INT16:
			*(int16_t *)(dp_tbl[id].p_data) = *(int16_t *)(dp_tbl[id].p_default);
			break;

		case GP_UINT16:
			*(uint16_t *)(dp_tbl[id].p_data) = *(uint16_t *)(dp_tbl[id].p_default);
			break;

		default:
			break;
	}
}


/************************************************************/
/*						LEGACY FUNCTIONS					*/
/*  These functions are provided for backward compatability */
/*  and will eventually be eliminated 						*/
/************************************************************/


/**************************************************************************************/
/*! \fn SetElem32(int id, int32_t value)
 *
 *	\param[in] id - element id
 *	\param[in] value - element value
 *
 *  \par Description:	  
 *  Write unsigned 32 bit integer to the datapool element.
 *
 *  \returns GHS INTEGRITY code of type Error. 	'Success' if successful,
 *			 see INTEGRITY_enum_error.h for error codes 
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
int32_t SetElem32(int id, int32_t value)
{
	return (int32_t)SetElem(id, (void *)&value);
}


/**************************************************************************************/
/*! \fn GetElem32(int id, uint32_t * value)
 *
 *	\param[in] id - element id
 *	\param[out] value - element value
 *
 *  \par Description:	  
 *  Read unsigned 32 bit integer from the datapool element.
 *
 *  \returns GHS INTEGRITY code of type Error. 	'Success' if successful,
 *			 see INTEGRITY_enum_error.h for error codes 
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
int32_t GetElem32(int id, uint32_t *value)
{
	return (int32_t)GetElem(id, (void *)value);
}
