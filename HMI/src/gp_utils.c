/**************************************************************************************/
/*!
 *  \file		gp_utils.c
 *
 *  \copyright	Yazaki 2016-17
 *
 *    \brief    Source file containing the common software utility functions used by
 *				software components in the GP software.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.20 $  
 *				$Log: gp_utils.c  $
 *				Revision 1.20 2017/07/20 13:47:22CDT Daniel Kageff (10011932) 
 *				Removed Doxygen comment
 *				Revision 1.19 2017/06/26 18:35:36EDT Daniel Kageff (10011932) 
 *				Defined Little/Big Endian values for GP multibyte read/write functions
 *				Revision 1.18 2017/06/02 18:47:27EDT Daniel Kageff (10011932) 
 *				Added runtime endian selection to the 16/32 bit store and read functions.
 *				Revision 1.17 2017/05/04 18:11:50EDT Daniel Kageff (10011932) 
 *				Updated comment
 *				Revision 1.16 2017/04/21 18:57:03EDT Daniel Kageff (10011932) 
 *				Improved strlcpy() to handle zero length string parameter
 *				Revision 1.15 2017/03/21 16:13:33EDT Daniel Kageff (10011932) 
 *				strlcpy() fixed to limit total string length to 'size'
 *				Revision 1.14 2017/03/16 17:06:29EDT Eudora Gunarta (10031829) 
 *				Changes for gp_ReadFloat, added more store/read functions.
 *				Revision 1.13 2017/03/02 10:32:35EST Daniel Kageff (10011932) 
 *				Added initial implementation of log buffer management functions (needs to be debugged)
 *				Revision 1.12 2017/02/22 18:18:41EST Daniel Kageff (10011932) 
 *				Updated Doxygen comments, _GH_Check, _Yz_Check
 *
 */
/***************************************************************************************/
#define _GP_UTILS_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/
//#include <SYSTEMYAZ.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
//#include <bsp.h>
//#include <util/setargs.h>
//#include <util/error_string.h>
#include <string.h>
#include <sys/time.h>

#include "gp_cfg.h"			// Common GP program configuration settings
#include "gp_types.h"		// Common GP program data type definitions
#include "crc32.h"			// CRC-32 check value calculation functions.
#include "md5.h"			// MD5 check value calculation functions
#include "gp_utils.h"		// Common GP program utility functions

/***********************************
	Private Macros and Typedefs
***********************************/

#define MAX_PRTBUF 256				/*!< Maximum gp_Printf() print buffer size */
#define DLFT_LOGBUF_SZ	(4 * 1024)	/*!< Default log buffer size in chars */
#define Success 0
/***********************************
		Private storage
***********************************/
static char *pLogBuf = NULL;
static int LogBufSize = 0;
static int LogBufIdx = 0;

/***********************************
	Private Function Prototypes
***********************************/



/**************** START OF MODULE CODE ******************/
/*! \defgroup utilfcns_public GP utility public API
 */

/**************************************************************************************/
/*! \fn _GhCheck(Error e, char *err_string, int errln)
 *
 *	\param[in]	e			- Error code, currently from GHS INTEGRITY definitions.
 *	\param[in]	err_string	- Error string to print, typically source filename.
 *	\param[in]	errln		- Line number of source file.
 *
 *  \par Description:	  
 *   Check result of GHS INTEGRITY API call.  If an error occured halt the current task
 *	 if DEBUG_ERR_HALT preprocessor symbol is defined.	  
 *
 *  \returns GHS INTEGRITY code of type Error. 	'Success' if successful,
 *			 see INTEGRITY_enum_error.h for error codes 
 *
 *  \par Limitations/Caveats:
 *	 1) If DEBUG_ERR_HALT is defined.  The current task will halt if there is an error.
 *
 *	\req_link None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int32_t _GhCheck(int32_t e, char *err_string, int errln)
{
    if(e != Success)
    {
    	//gp_Printf(VRB_DEBUG1, "GhCheck: Error %s (%d), File: %s, ln: %d\n", ErrorString(e), e,  err_string, errln);
        gp_Printf(VRB_DEBUG1, "GhCheck: Error (%d), File: %s, ln: %d\n", e,  err_string, errln);
 #ifdef DEBUG_ERR_HALT
		HaltTask(CurrentTask());//replace
 #endif
    }
    return e;
}

/**************************************************************************************/
/*! \fn _YzCheck(int32_t code, char *err_string, int errln)
 *
 *	\param[in]	code	    - GP program error code, typically of type ::gp_retcode_t.
 *	\param[in]	err_string	- Error string to print, typically source filename.
 *	\param[in]	errln		- Line number of source file.
 *
 *  \par Description:	  
 *   Check Yazaki return code result.  If an error occured halt the current task (if the
 *	 DEBUG_ERR_HALT preprocessor symbol is defined.
 *
 *  \returns	code - Returns error code that was passed to it. 
 *
 *  \par Limitations/Caveats:
 *	 - Assumes that code == 0 means no error has occured.
 *
 *	\req_link None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int32_t _YzCheck(int32_t code, char *err_string, int errln)
{
    if(code != GP_SUCCESS) 
    {
		gp_Printf(VRB_DEBUG1, "YzCheck error@ line %d of %s, errcode %d\n", errln, err_string, code);
#ifdef DEBUG_ERR_HALT
		HaltTask(CurrentTask());//replace
#endif
    }
	return code;
}

/**************************************************************************************/
/*! \fn gp_Store16bitE(uint16_t value, uint8_t *p_buf, int endian)
 *
 *	\param[in]	value	- Unsigned 16 bit value to store.
 *	\param[in]  p_buf	- Buffer pointer
 *	\param[in]  endian	- Selects Little (=0) or Big Endian (!=0) format.
 *						  See ::GP_RW_ENDIAN_T definition.
 *
 *  \par Description:	  
 *   Store multibyte value into the non-16bit aligned location pointed to by p_buf.  
 *	 Byte order is is controlled by the 'endian' parameter.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store16bitE(uint16_t value, uint8_t *p_buf, int endian)
{
	if(endian == GP_RW_LE)
	{
		/* Store in Little Endian format */
		*(uint16_t *)p_buf = value;
	}
	else
	{
		/* Store in Big Endian format */
		p_buf[0] = (uint8_t)((value >> 8) & 0xff);
		p_buf[1] = (uint8_t)(value & 0xff);
	}
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Store16bitBE(uint16_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Unsigned 16 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-16bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store16bitBE(uint16_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 8) & 0xff);
	p_buf[1] = (uint8_t)(value & 0xff);
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Store16bitSigned(int16_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Signed 16 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-16bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 1) Change code to use 'endian' parameter
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store16bitSigned(int16_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 8) & 0xff);
	p_buf[1] = (uint8_t)(value & 0xff);
#else
	/* Store in Little Endian format */
	p_buf[0] = (uint8_t)(value & 0xff);
	p_buf[1] = (uint8_t)((value >> 8) & 0xff);
#endif
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Store32bitE(uint32_t value, uint8_t *p_buf, int endian)
 *
 *	\param[in]	value	- Unsigned 32 bit value to store.
 *	\param[in]  p_buf	- Buffer pointer
 *	\param[in]  endian	- Selects Little (=0) or Big Endian (!=0) format. 
 *						  See ::GP_RW_ENDIAN_T definition.
 *
 *  \par Description:	  
 *   Store multibyte value into the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is is controlled by the 'endian' parameter.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store32bitE(uint32_t value, uint8_t *p_buf, int endian)
{
	if(endian == GP_RW_LE)
	{
		/* Read in Little Endian format */
		*(uint32_t *)p_buf = value;
	}
	else
	{
		/* Store in Big Endian format */
		p_buf[0] = (uint8_t)((value >> 24) & 0xff);
		p_buf[1] = (uint8_t)((value >> 16) & 0xff);
		p_buf[2] = (uint8_t)((value >> 8) & 0xff);
		p_buf[3] = (uint8_t)(value & 0xff);
	}
	return 4;
}


/**************************************************************************************/
/*! \fn gp_Store32bitBE(uint32_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Unsigned 32 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store32bitBE(uint32_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 24) & 0xff);
	p_buf[1] = (uint8_t)((value >> 16) & 0xff);
	p_buf[2] = (uint8_t)((value >> 8) & 0xff);
	p_buf[3] = (uint8_t)(value & 0xff);
	return 4;
}


/**************************************************************************************/
/*! \fn gp_Store32bitSigned(int32_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Signed 32 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store32bitSigned(int32_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 24) & 0xff);
	p_buf[1] = (uint8_t)((value >> 16) & 0xff);
	p_buf[2] = (uint8_t)((value >> 8) & 0xff);
	p_buf[3] = (uint8_t)(value & 0xff);
#else
	/* Store in Little Endian format */
	p_buf[3] = (uint8_t)((value >> 24) & 0xff);
	p_buf[2] = (uint8_t)((value >> 16) & 0xff);
	p_buf[1] = (uint8_t)((value >> 8) & 0xff);
	p_buf[0] = (uint8_t)(value & 0xff);
#endif
	return 4;
}


/**************************************************************************************/
/*! \fn gp_StoreFloat(float value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Single precision floating point value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\todo Needs to be debugged
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_StoreFloat(float value, uint8_t *p_buf)
{
#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	 p_buf[0] = ((uint8_t *)&value)[3];
	 p_buf[1] = ((uint8_t *)&value)[2];
	 p_buf[2] = ((uint8_t *)&value)[1];
	 p_buf[3] = ((uint8_t *)&value)[0];
#else
	/* Store in Little Endian format */
	 p_buf[0] = ((uint8_t *)&value)[3];
	 p_buf[1] = ((uint8_t *)&value)[2];
	 p_buf[2] = ((uint8_t *)&value)[1];
	 p_buf[3] = ((uint8_t *)&value)[0];
#endif
	return 4;
}


/**************************************************************************************/
/*! \fn gp_Store64bit(uint64_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Unsigned 64 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store64bit(uint64_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 56) & 0xff);
	p_buf[1] = (uint8_t)((value >> 48) & 0xff);
	p_buf[2] = (uint8_t)((value >> 40) & 0xff);
	p_buf[3] = (uint8_t)((value >> 32) & 0xff);
	p_buf[4] = (uint8_t)((value >> 24) & 0xff);
	p_buf[5] = (uint8_t)((value >> 16) & 0xff);
	p_buf[6] = (uint8_t)((value >> 8) & 0xff);
	p_buf[7] = (uint8_t)(value & 0xff);
#else
	/* Store in Little Endian format */
	p_buf[7] = (uint8_t)((value >> 56) & 0xff);
	p_buf[6] = (uint8_t)((value >> 58) & 0xff);
	p_buf[5] = (uint8_t)((value >> 40) & 0xff);
	p_buf[4] = (uint8_t)((value >> 32) & 0xff);
	p_buf[3] = (uint8_t)((value >> 24) & 0xff);
	p_buf[2] = (uint8_t)((value >> 16) & 0xff);
	p_buf[1] = (uint8_t)((value >> 8) & 0xff);
	p_buf[0] = (uint8_t)(value & 0xff);
#endif
	return 8;
}


/**************************************************************************************/
/*! \fn gp_Store64bitSigned(int64_t value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Signed 64 bit value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Store64bitSigned(int64_t value, uint8_t *p_buf)
{
//	uint8_t *p_put = p_buf;

#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	p_buf[0] = (uint8_t)((value >> 56) & 0xff);
	p_buf[1] = (uint8_t)((value >> 48) & 0xff);
	p_buf[2] = (uint8_t)((value >> 40) & 0xff);
	p_buf[3] = (uint8_t)((value >> 32) & 0xff);
	p_buf[4] = (uint8_t)((value >> 24) & 0xff);
	p_buf[5] = (uint8_t)((value >> 16) & 0xff);
	p_buf[6] = (uint8_t)((value >> 8) & 0xff);
	p_buf[7] = (uint8_t)(value & 0xff);
#else
	/* Store in Little Endian format */
	p_buf[7] = (uint8_t)((value >> 56) & 0xff);
	p_buf[6] = (uint8_t)((value >> 58) & 0xff);
	p_buf[5] = (uint8_t)((value >> 40) & 0xff);
	p_buf[4] = (uint8_t)((value >> 32) & 0xff);
	p_buf[3] = (uint8_t)((value >> 24) & 0xff);
	p_buf[2] = (uint8_t)((value >> 16) & 0xff);
	p_buf[1] = (uint8_t)((value >> 8) & 0xff);
	p_buf[0] = (uint8_t)(value & 0xff);
#endif
	return 8;
}


/**************************************************************************************/
/*! \fn gp_StoreDouble(double value, uint8_t *p_buf)
 *
 *	\param[in]	value	- Single precision floating point value to store.
  *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Store multibyte value into the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\todo Needs to be debugged
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_StoreDouble(double value, uint8_t *p_buf)
{
#ifdef IPC_BIG_ENDIAN
	/* Store in Big Endian format */
	 p_buf[0] = ((uint8_t *)&value)[7];
	 p_buf[1] = ((uint8_t *)&value)[6];
	 p_buf[2] = ((uint8_t *)&value)[5];
	 p_buf[3] = ((uint8_t *)&value)[4];
	 p_buf[4] = ((uint8_t *)&value)[3];
	 p_buf[5] = ((uint8_t *)&value)[2];
	 p_buf[6] = ((uint8_t *)&value)[1];
	 p_buf[7] = ((uint8_t *)&value)[0];
#else
	/* Store in Little Endian format */
	 /*p_buf[0] = ((uint8_t *)&value)[3];
	 p_buf[1] = ((uint8_t *)&value)[2];
	 p_buf[2] = ((uint8_t *)&value)[1];
	 p_buf[3] = ((uint8_t *)&value)[0];*/
#endif
	return 8;
}


/**************************************************************************************/
/*! \fn gp_Read16bitE(uint16_t *p_value, uint8_t *p_buf, int endian)
 *
 *	\param[in]	p_value	- Pointer to unsigned 16 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *	\param[in]  endian	- Selects Little (=0) or Big Endian (!=0) format.
 *						  See ::GP_RW_ENDIAN_T definition.
 *
 *  \par Description:	  
 *   Read multibyte value from the non-16bit aligned location pointed to by p_buf.  
 *	 Byte order is is controlled by the 'endian' parameter.
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read16bitE(uint16_t *p_value, uint8_t *p_buf, int endian)
{
	uint16_t value;

	if(endian == GP_RW_LE)
	{
		/* Store in Little Endian format */
		value = *(uint16_t *)p_buf;
	}
	else
	{
		/* Read in Big Endian format */
		value = (uint16_t)p_buf[1];			// MSByte goes to lower byte 
		value |= (uint16_t)p_buf[0] << 8; 	// LSByte goes to higher byte
	}
	*p_value = value;
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Read16bitBE(uint16_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to unsigned 16 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read Big Endian multibyte value from the non-16bit aligned location pointed to by   
 *	 p_buf and store in the location pointed to by p_value.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read16bitBE(uint16_t *p_value, uint8_t *p_buf)
{
	/* Read in Big Endian format */
	*p_value = (uint16_t)p_buf[1];			// MSByte goes to lower byte 
	*p_value |= (uint16_t)p_buf[0] << 8; 	// LSByte goes to higher byte
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Read16bitSigned(int16_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to signed 16 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte value from the non-16bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read16bitSigned(int16_t *p_value, uint8_t *p_buf)
{
	int16_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = (int16_t)p_buf[1];			// MSByte goes to lower byte 
	value |= (int16_t)p_buf[0] << 8; 	// LSByte goes to higher byte
#else
	/* Store in Little Endian format */
	value = *(int16_t *)p_buf;
#endif
	*p_value = value;
	return 2;
}


/**************************************************************************************/
/*! \fn gp_Read32bitE(uint32_t *p_value, uint8_t *p_buf, int endian)
 *
 *	\param[in]	p_value	- Pointer to unsigned 32 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *	\param[in]  endian	- Selects Little (=0) or Big Endian (!=0) format.
 *						  See ::GP_RW_ENDIAN_T definition. *
 *
 *  \par Description:	  
 *   Read multibyte value from the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is is controlled by the 'endian' parameter.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read32bitE(uint32_t *p_value, uint8_t *p_buf, int endian)
{
	uint32_t value;

	if(endian == GP_RW_LE)
	{
		/* Read in Little Endian format */
		value = *(uint32_t *)p_buf;
	}
	else
	{
		/* Read in Big Endian format */
		value = ((uint32_t)p_buf[0]) << 24;
		value |= ((uint32_t)p_buf[1]) << 16;
		value |= ((uint32_t)p_buf[2]) << 8;
		value |= (uint32_t)p_buf[3];
	}
	*p_value = value;
	return 4;
}


/**************************************************************************************/
/*! \fn gp_Read32bitBE(uint32_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to unsigned 32 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read Big Endian multibyte value from the non-32bit aligned location pointed to by   
 *	 p_buf and store in the location pointed to by p_value.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read32bitBE(uint32_t *p_value, uint8_t *p_buf)
{
	/* Read in Big Endian format */
	*p_value = ((uint32_t)p_buf[0]) << 24;
	*p_value |= ((uint32_t)p_buf[1]) << 16;
	*p_value |= ((uint32_t)p_buf[2]) << 8;
	*p_value |= (uint32_t)p_buf[3];

	return 4;
}


/**************************************************************************************/
/*! \fn gp_Read32bitSigned(int32_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to signed 32 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte value from the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read32bitSigned(int32_t *p_value, uint8_t *p_buf)
{
	int32_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = ((int32_t)p_buf[0]) << 24;
	value |= ((int32_t)p_buf[1]) << 16;
	value |= ((int32_t)p_buf[2]) << 8;
	value |= (int32_t)p_buf[3];
#else
	/* Read in Little Endian format */
	value = *(int32_t *)p_buf;
#endif
	*p_value = value;
	return 4;
}


/**************************************************************************************/
/*! \fn gp_ReadFloat(float *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to single precision floating point variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte float value from the non-32bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *
 *	 \todo Needs to be debugged
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_ReadFloat(uint32_t *p_value, uint8_t *p_buf)
{
	uint32_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = (uint32_t)p_buf[0] << 24;
	value |= (uint32_t)p_buf[1] << 16;
	value |= (uint32_t)p_buf[2] << 8;
	value |= (uint32_t)p_buf[3];
#else
	/* Read in Little Endian format */
	value = *(uint32_t *)p_buf;
#endif
	*p_value = (uint32_t)value;
	return 4;
}


/**************************************************************************************/
/*! \fn gp_Read64bit(uint64_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to unsigned 64 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte value from the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read64bit(uint64_t *p_value, uint8_t *p_buf)
{
	uint64_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = ((uint64_t)p_buf[0]) << 56;
	value |= ((uint64_t)p_buf[1]) << 48;
	value |= ((uint64_t)p_buf[2]) << 40;
	value |= ((uint64_t)p_buf[3]) << 32;
	value |= ((uint64_t)p_buf[4]) << 24;
	value |= ((uint64_t)p_buf[5]) << 16;
	value |= ((uint64_t)p_buf[6]) << 8;
	value |= (uint64_t)p_buf[7];
#else
	/* Read in Little Endian format */
	value = *(uint64_t *)p_buf;
#endif
	*p_value = value;
	return 8;
}


/**************************************************************************************/
/*! \fn gp_Read64bitSigned(int64_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to signed 64 bit variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte value from the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_Read64bitSigned(int64_t *p_value, uint8_t *p_buf)
{
	int64_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = ((int64_t)p_buf[0]) << 56;
	value |= ((int64_t)p_buf[1]) << 48;
	value |= ((int64_t)p_buf[2]) << 40;
	value |= ((int64_t)p_buf[3]) << 32;
	value |= ((int64_t)p_buf[4]) << 24;
	value |= ((int64_t)p_buf[5]) << 16;
	value |= ((int64_t)p_buf[6]) << 8;
	value |= (int64_t)p_buf[7];
#else
	/* Read in Little Endian format */
	value = *(int64_t *)p_buf;
#endif
	*p_value = value;
	return 8;
}


/**************************************************************************************/
/*! \fn gp_ReadDouble(uint64_t *p_value, uint8_t *p_buf)
 *
 *	\param[in]	p_value	- Pointer to single precision floating point variable to update.
 *	\param[in]  p_buf	- Buffer pointer
 *
 *  \par Description:	  
 *   Read multibyte float value from the non-64bit aligned location pointed to by p_buf.  
 *	 Byte order is Big-Endian if IPC_BIG_ENDIAN is defined.	  
 *
 *  \returns - Offset to the buffer location after the stored data.
 *
 *  \par Limitations/Caveats:
 *
 *	 \todo Needs to be debugged
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
int gp_ReadDouble(uint64_t *p_value, uint8_t *p_buf)
{
	uint64_t value;

#ifdef IPC_BIG_ENDIAN
	/* Read in Big Endian format */
	value = (uint64_t)p_buf[0] << 56;
	value |= (uint64_t)p_buf[1] << 48;
	value |= (uint64_t)p_buf[2] << 40;
	value |= (uint64_t)p_buf[3] << 32;
	value |= (uint64_t)p_buf[4] << 24;
	value |= (uint64_t)p_buf[5] << 16;
	value |= (uint64_t)p_buf[6] << 8;
	value |= (uint64_t)p_buf[7];
#else
	/* Read in Little Endian format */
	value = *(uint64_t *)p_buf;
#endif
	*p_value = (uint64_t)value;
	return 8;
}


/**************************************************************************************/
/*! \fn gp_Printf(DbgVerbosity_t vrb, const char * format, ...)
 *
 *	\param[in]	vrb	- Requested debug print verbosity level of type ::DbgVerbosity_t.
 *	\param[in]  format	- 'C' Format string
 *	\param[in]	...	  	- Variable list of 'C' string formatting parameters.
 *
 *  \par Description:	  
 *   Debug print function.  Generates different levels of debugging output depending  
 *	 on the value of 'vrb'.	 If a log buffer is available and has room, it will write
 *	 the debug string to it.  Uses a variable length parameter list.
 *
 *  \returns - None.
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
void gp_Printf(DbgVerbosity_t vrb, const char * format, ...)
{
	if(vrb >= DFLT_DBG_PRNTLVL)
	{
		char buffer[MAX_PRTBUF];
		va_list args;
		va_start(args, format);
		vsnprintf(buffer, MAX_PRTBUF, format, args);
		if(vrb <= DFLT_DBG_PRNTLVL)
		{
			/* If logging to a buffer and it has room copy string to the log buffer */
			int len = strlen(buffer);
			if((pLogBuf != NULL) && ((LogBufIdx + len) < LogBufSize))
			{
				strncpy(&pLogBuf[LogBufIdx], buffer, len);
				LogBufIdx += len;
			}
			/* Print the string */	
			printf("%s\n",buffer);
		}
		va_end(args);
	}
}


/**************************************************************************************/
/*! \fn gp_OpenLogBuf(char **pBuf, int size)
 *
 *	\param[in]  pBuf	- Pointer to a log buffer pointer variable. This is used if the 
 *						  caller wants an existing buffer to be used. 
 *	\param[in]	size	- Log buffer size in bytes.  If zero, then the default size will
 *						  be used.
 *
 *  \par Description:	  
 *   Set up debug log buffer or use an existing log buffer.	If there is common log buffer,
 *	 it can be supplied to this function for use by gp_Printf().
 *
 *  \returns - None.
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
void gp_OpenLogBuf(char **pBuf, int size)
{
	LogBufIdx = 0;

	/* If a log buffer is already defined use that buffer */
	if((*pBuf != NULL) && (size > 0))
	{
		pLogBuf = *pBuf;
		LogBufSize = size;
	}
	/* Else allocate a local log buffer */
	else
	{
		/* If log buffer size is supplied then use it else use the default size */
		if(size > 0)
		{
			LogBufSize = size;
		}
		else
		{
			LogBufSize = DLFT_LOGBUF_SZ;
		}
		/* Allocate buffer */
		pLogBuf = malloc(LogBufSize);

		/* If a buffer pointer was supplied then return buffer pointer */
		if(pBuf	!= NULL)
		{
			*pBuf = pLogBuf;
		}
	}
}

/**************************************************************************************/
/*! \fn gp_GetLogBuf(char **pBuf, int *pSize)
 *
 *	\param[in]  pBuf	- Pointer to a log buffer pointer variable. This is used if 
 *						  the caller wants an existing buffer to be used. 
 *	\param[in]	pSize	- Log buffer size in bytes.  If zero, then the default size will
 *						  be used.
 *
 *  \par Description:	  
 *   Return debug log buffer pointer and size.
 *
 *  \returns - None.
 *
 *  \par Limitations/Caveats:
 *	1) Does nothing if either pBuf or pSize are NULL.
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
void gp_GetLogBuf(char **pBuf, int *pSize)
{
	if((pBuf != NULL) && (pSize != NULL))
	{
		*pBuf = pLogBuf;
		*pSize = LogBufSize;
	}
}
 
/**************************************************************************************/
/*! \fn strlcpy(char * restrict dst, const	char * restrict	src, size_t size)
 *
 *	\param[in]	dst	- Pointer to destination buffer.
 *	\param[in]  src - Pointer to source string buffer.
 *	\param[in]	size - Length of destination buffer. String length plus NULL terminator
 *					   must not exceed this. If it does, it will be terminated to fit.
 *
 *  \par Description:	  
 *   Similar to strncpy() except that it always NULL terminates the copied string. It also   
 *	 makes sure that the source string is not too long for the destination buffer.
 *
 *  \returns - Number of bytes/chars copied.
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
//TODO: for some reason gcc seems not to recognize the strict keyword
//size_t strlcpy(char *restrict dst, const char *restrict	src, size_t size)
size_t strlcpy(char * dst, const char * src, size_t size)
{
	if(size > 0)
	{
		int slen = strlen(src);
		if(slen >= size)		// src string is too long, truncate it
		{
			slen = size-1;		// allow for null char at the end
		}
		strncpy(dst, src, slen);
		dst[slen] = 0;			// null terminate the string
		return slen;
	}
	else
	{
		return 0;		// operation not performed
	}
}


/**************************************************************************************/
/*! \fn gp_TimerSub(struct timeval *stop, struct timeval *start, struct timeval *result)
 *
 *	\param[in]	stop   - Pointer to stop timeval structure defined in sys/time.h.
 *	\param[in]  start  - Pointer to start timeval structure defined in sys/time.h.
 *	\param[out]	result - Pointer to result timeval structure defined in sys/time.h.
 *
 *  \par Description:	  
 *   Calculates the time between the stop and start times and places the result in the
 *	 'result' time structure.
 *
 *  \returns - none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
void gp_TimerSub(struct timeval *stop, struct timeval *start, struct timeval *result)
{
    if ((stop->tv_usec - start->tv_usec) < 0) 
    {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_usec = stop->tv_usec - start->tv_usec + 1000000000;
    } 
    else 
    {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_usec = stop->tv_usec - start->tv_usec;
    }
    return;
}


/**************************************************************************************/
/*! \fn gp_Cnvrt2Wstr(char *p_wstr, char *p_str)
 *
 *	\param[in]	p_wstr - Pointer to destination 16 bit (wide) char string buffer.
 *	\param[in]	p_str  - Pointer to source 8 bit char string buffer.
 *
 *  \par Description:	  
 *   Convert an 8bit char string pointed to by 'p_str' to a 16bit char string pointed
 *	 to by 'p_wstr.
 *
 *  \returns - none
 *
 *  \par Limitations/Caveats:
 *	1) Buffer pointed to by p_wstr must be at least 2x the length of the string 
 *	   pointed to by p_str.
 *
 *  \ingroup utilfcns_public
 **************************************************************************************/
void gp_Cnvrt2Wstr(char *p_wstr, char *p_str)
{
	int char_cnt = strlen(p_str);
	int i;
	int wi = 0;

	/* Convert each 8bit char into a 16bit char */
	for(i=0; i < char_cnt; i++)
	{
		wi = i*2;
		p_wstr[wi] = p_str[i];
		p_wstr[wi+1] = 0;
	}
	/* Add the null terminator */
	char_cnt *= 2;
	p_wstr[char_cnt] = 0;
	p_wstr[char_cnt+1] = 0;
}


