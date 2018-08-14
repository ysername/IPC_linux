

/************************************************************************************/
/*!
*  \file		msg_buf.c  
*
*  \copyright	Yazaki 2016
*   			Unpublished, Yazaki North America Inc. All rights reserved. 
*  				This document is protected by the copyright law of the United States and 
*  				foreign countries.  The work embodied in this document is confidential and 
*  				constitutes a trade secret of Yazaki North America, Inc.  Any copying of 
*  				this document and any use or disclosure of this document or the work 
*  				embodied in it without the written authority of Yazaki North America, Inc., 
*  				will constitute copyright and trade secret infringement and will violate the
*  				rights of Yazaki North America, Inc.
*
*
*  \brief		GP IPC (Inter-address space/process communication) messaging buffer 
*				management functions.  The message buffers are available in fixed sizes
*				(4, 8, 16, 32, 64, 128, 256 byte) and do not use dynamic memory allocation.
*
*  \author		Daniel Kageff,  Eudora Gunarta
*
*  \version		$Revision: 1.6 $
*
*  Modification history:
*   $Log: msg_buf.c  $
*   Revision 1.6 2017/07/20 13:50:45CDT Daniel Kageff (10011932) 
*   1) Did some code reformatting
*   2) Added/edited comments for Doxygen design spec generation.
*   Revision 1.5 2017/03/30 11:15:15EDT Eudora Gunarta (10031829) 
*   Added 256 byte buffer.
*   Revision 1.4 2017/03/15 09:47:17EDT Eudora Gunarta (10031829) 
*   Changes for return codes, increased MAX_NUM_POOLS, updated function baners.
*   Revision 1.3 2017/02/02 11:17:16EST David Gahutu (10015686) 
*   To add annotations (for Doxygen)
*   Revision 1.2 2016/09/21 17:00:30EDT Eudora Gunarta (10031829) 
*   Changes so that Msg_GetBuf() will search through all (any size) possible buffers.
*   Revision 1.1 2016/05/18 09:47:57EDT Daniel Kageff (10011932) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Common/project.pj
******************************************************************************/
#define MSG_BUF_C

/*****************************************************************************/
/*    I N C L U D E   F I L E S                                              */
/*****************************************************************************/
#include <string.h>
#include <stdint.h>

#include "gp_types.h"


/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 
/* Max number of buffer pools per VAS */
#define MAX_NUM_POOLS 4

/* Max number of buffers (for each buffer type) */
#define MAX_NUM_BUF4  64 	/*!< Maximum number of 4 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/4u) */
#define MAX_NUM_BUF8  32 	/*!< Maximum number of 8 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/8u) */
#define MAX_NUM_BUF16 16 	/*!< Maximum number of 16 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/16u)	*/
#define MAX_NUM_BUF32  8 	/*!< Maximum number of 32 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/32u)	*/
#define MAX_NUM_BUF64  4 	/*!< Maximum number of 64 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/64u)	*/
#define MAX_NUM_BUF128 4 	/*!< Maximum number of 128 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/128u) */
#define MAX_NUM_BUF256 2 	/*!< Maximum number of 256 byte buffers(YZ_SPI_MAX_MSG_SIZE*2/128u) */

/* Starting value of invalid task index */
#define INVALID_TSK_IDX (MAX_NUM_POOLS)		/*!< Task index limit */

#define CTL_BYTES 3

/*****************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                            */
/*****************************************************************************/
/*! 4 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[4+CTL_BYTES];			/*!< Data buffer */			
	uint16_t BytesUsed;		/*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf4Type;
/*! 8 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[8+CTL_BYTES];			/*!< Data buffer */			
	uint16_t BytesUsed;		/*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf8Type;
/*! 16 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[16+CTL_BYTES];	   /*!< Data buffer */			
	uint16_t BytesUsed;	   /*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf16Type;
/*! 32 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[32+CTL_BYTES];	   /*!< Data buffer */			
	uint16_t BytesUsed;	   /*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf32Type;
/*! 64 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[64+CTL_BYTES];	  /*!< Data buffer */			
	uint16_t BytesUsed;	  /*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf64Type;
/*! 128 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[128+CTL_BYTES];	  /*!< Data buffer */			
	uint16_t BytesUsed;	  /*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf128Type;
/*! 256 byte buffer type descriptor */
typedef struct {
	uint8_t Buf[256+CTL_BYTES];	 /*!< Data buffer */			
	uint16_t BytesUsed;	 /*!< 0= Buffer is available, Non-zero= Buffer is in use */
} Buf256Type;

/*! Buffer pool structure */
typedef struct {
    Buf4Type 	Buf4[MAX_NUM_BUF4];			/*!< Array of 4 byte buffer entries */
    Buf8Type 	Buf8[MAX_NUM_BUF8];			/*!< Array of 8 byte buffer entries */
    Buf16Type 	Buf16[MAX_NUM_BUF16];		/*!< Array of 16 byte buffer entries */
    Buf32Type 	Buf32[MAX_NUM_BUF32];		/*!< Array of 32 byte buffer entries */
    Buf64Type 	Buf64[MAX_NUM_BUF64];		/*!< Array of 64 byte buffer entries */
    Buf128Type 	Buf128[MAX_NUM_BUF128];		/*!< Array of 128 byte buffer entries */
    Buf256Type 	Buf256[MAX_NUM_BUF256];		/*!< Array of 256 byte buffer entries */
} BufPoolType;

/*****************************************************************************/
/*    M E M O R Y   A L L O C A T I O N                                      */
/*****************************************************************************/

/*! Array of buffer pools available to a VAS */
static BufPoolType BufPool[MAX_NUM_POOLS];

//TODO: adapt the code to use an enum type instead of the explicit buffer size
//		as control for the switch cases


/************ Start of code ******************/

/**************************************************************************************/
/*! \fn Msg_InitBufs(void)
 *
 *  \par Description:	  
 *  Assigns a buffer pool to the task, then initializes the task's buffer pool.
 *
 *  \retval	Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *  None.
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
gp_retcode_t Msg_InitBufs(uint8_t component)
{
    BufPoolType * pBufPool;
    
    pBufPool = &BufPool[component];

    memset(&pBufPool->Buf4, 0, sizeof(Buf4Type)*MAX_NUM_BUF4);
    memset(&pBufPool->Buf8, 0, sizeof(Buf8Type)*MAX_NUM_BUF8);
    memset(&pBufPool->Buf16, 0, sizeof(Buf16Type)*MAX_NUM_BUF16);
    memset(&pBufPool->Buf32, 0, sizeof(Buf32Type)*MAX_NUM_BUF32);
    memset(&pBufPool->Buf64, 0, sizeof(Buf64Type)*MAX_NUM_BUF64);
    memset(&pBufPool->Buf128, 0, sizeof(Buf128Type)*MAX_NUM_BUF128);
    memset(&pBufPool->Buf256, 0, sizeof(Buf256Type)*MAX_NUM_BUF256);
    return GP_SUCCESS;
}

/**************************************************************************************/
/*! \fn Msg_GetBuf(uint32_t reqSz, uint32_t *bufSz, uint32_t *bufIdx)
 *
 *	\param[in] reqSz  - Requested buffer size in bytes
 *	\param[out] bufSz - Pointer to size of buffer reserved, updated by this function.
 *	\param[out] bufIdx - Pointer to the index of buffer reserved, updated by this function.
 *
 *  \par Description:	  
 *  Reserves a buffer from the task's buffer pool. Finds an available buffer of the
 *	appropriate size in the buffer pool, marks it as 'assigned', then returns a pointer
 *	to the buffer.
 *
 *  \retval	Pointer to reserved buffer if ok, NULL pointer if no buffer found or there
 *			were errors.
 *
 *  \par Limitations/Caveats:
 *  None.
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
uint8_t *Msg_GetBuf(uint8_t reqSz, uint32_t *bufIdx, uint8_t component)
{
    uint32_t i;
    uint8_t * pBuf = NULL;
    BufPoolType * pBufPool;
    uint32_t tskIdx;

	/* Get the address of the buffer pool for this task */
    pBufPool = &BufPool[component];

	/* If the request buffer size is 1 - 4 bytes then */
    if(reqSz <= 4) 
    {
		/* Look for an available 4 byte buffer */
		for(i=0; i<MAX_NUM_BUF4; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf4[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf4[i].Buf;			// get buffer address 
				pBufPool->Buf4[i].BytesUsed = reqSz;	// save request size and mark as 'reserved'
				//*bufSz = 4+3;								// return the max buffer size
				*bufIdx = (uint32_t)i;							// return the buffer index
				return pBuf;
		    }
		}
    }
    
	/* Else if the request buffer size is 5 - 8 bytes then */
    else if( (reqSz > 4) && (reqSz <= 8) ) 
    {
		/* Look for an available 8 byte buffer */
		for(i=0; i<MAX_NUM_BUF8; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf8[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf8[i].Buf;		   // get buffer address 
				pBufPool->Buf8[i].BytesUsed = reqSz;   // save request size and mark as 'reserved'
				//*bufSz = 8+3;							   // return the max buffer size
				*bufIdx = (uint32_t)i;						   // return the buffer index
				return pBuf;
		    }
		}
    }

	/* Else if the request buffer size is 9 - 16 bytes then */
    else if( (reqSz > 8) && (reqSz <= 16) ) 
    {
		/* Look for an available 16 byte buffer */
		for(i=0; i<MAX_NUM_BUF16; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf16[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf16[i].Buf;		   // get buffer address 
				pBufPool->Buf16[i].BytesUsed = reqSz;  // save request size and mark as 'reserved'
				//*bufSz = 16+3;						   // return the max buffer size
				*bufIdx = (uint32_t)i;						   // return the buffer index
				return pBuf;
		    }
		}
    }

	/* Else if the request buffer size is 17 - 32 bytes then */
    else if( (reqSz > 16) && (reqSz <= 32) ) 
    {
		/* Look for an available 32 byte buffer */
		for(i=0; i<MAX_NUM_BUF32; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf32[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf32[i].Buf;		   // get buffer address 
				pBufPool->Buf32[i].BytesUsed = reqSz;  // save request size and mark as 'reserved'
				//*bufSz = 32+3;						   // return the max buffer size
				*bufIdx = (uint32_t)i;						   // return the buffer index
				return pBuf;
		    }
		}
    }

	/* Else if the request buffer size is 33 - 64 bytes then */
    else if( (reqSz > 32) && (reqSz <= 64) ) 
    {
		/* Look for an available 64 byte buffer */
		for(i=0; i<MAX_NUM_BUF64; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf64[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf64[i].Buf;		   // get buffer address 
				pBufPool->Buf64[i].BytesUsed = reqSz;  // save request size and mark as 'reserved'
				//*bufSz = 64+3;						   // return the max buffer size
				*bufIdx = (uint32_t)i;						   // return the buffer index
				return pBuf;
		    }
		}
    }

	/* Else if the request buffer size is 65 - 128 bytes then */
    else if( (reqSz > 64) && (reqSz <= 128) ) 
    {
		/* Look for an available 128 byte buffer */
		for(i=0; i<MAX_NUM_BUF128; i++) 
		{
			/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf128[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf128[i].Buf;			// get buffer address 
				pBufPool->Buf128[i].BytesUsed = reqSz;	// save request size and mark as 'reserved'
				//*bufSz = 128+3;							// return the max buffer size
				*bufIdx = (uint32_t)i;							// return the buffer index
				return pBuf;
		    }
		}
    }

	/* Else if the request buffer size is 129 - 256 bytes then */
    /*else*/ if( (reqSz > 128) && (reqSz <= 256) ) 
    {
		/* Look for an available 256 byte buffer */
		for( i=0; i<MAX_NUM_BUF256; i++) 
		{
	/* If an available buffer is found, reserve it and return buffer info */
		    if(pBufPool->Buf256[i].BytesUsed == 0) 
		    {
				pBuf = pBufPool->Buf256[i].Buf;			 // get buffer address 
				pBufPool->Buf256[i].BytesUsed = reqSz;	 // save request size and mark as 'reserved'
			//	//*bufSz = 256+3;							 // return the max buffer size
				*bufIdx = (uint32_t)i;							 // return the buffer index
				//printf("bufIdx is: %i\n",bufIdx );
				return pBuf;
		    }
		}
    }
    return pBuf;
}

/**************************************************************************************/
/*! \fn Msg_FreeBuf(uint32_t bufSz, uint32_t bufIdx)
 *
 *	\param[in] bufSz - Size of buffer to free
 *	\param[in] bufIdx - Index of buffer to free
 *
 *  \par Description:	  
 *  Frees a buffer from the task's buffer pool by marking it as 'available'.
 *
 *  \retval	None.
 *
 *  \par Limitations/Caveats:
 *  None.
 *
 *	\ingroup msgfcns_public
 **************************************************************************************/
void Msg_FreeBuf(uint8_t bufSz, uint32_t bufIdx, uint8_t component)
{
    BufPoolType * pBufPool;
    uint32_t tskIdx;
    
    pBufPool = &BufPool[component];
    
    switch(bufSz) 
    {
		/* Release a 4 byte buffer */
		case RSIZE4:
		    if(bufIdx < MAX_NUM_BUF4) 
		    {
				pBufPool->Buf4[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 8 byte buffer */
		case RSIZE8:
		    if(bufIdx < MAX_NUM_BUF8) {
			pBufPool->Buf8[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 16 byte buffer */
		case RSIZE16:
		    if(bufIdx < MAX_NUM_BUF16) {
			pBufPool->Buf16[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 32 byte buffer */
		case RSIZE32:
		    if(bufIdx < MAX_NUM_BUF32) {
			pBufPool->Buf32[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 64 byte buffer */
		case RSIZE64:
		    if(bufIdx < MAX_NUM_BUF64) {
			pBufPool->Buf64[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 128 byte buffer */
		case RSIZE128:
		    if(bufIdx < MAX_NUM_BUF128) {
			pBufPool->Buf128[bufIdx].BytesUsed = 0;
		    }
		    break;
		/* Release a 256 byte buffer */
		case RSIZE256:
		    if(bufIdx < MAX_NUM_BUF256) {
			pBufPool->Buf256[bufIdx].BytesUsed = 0;
		    }
		    break;
		default:
		    break;
    }
   
}

/* MSG_BUF_C */
