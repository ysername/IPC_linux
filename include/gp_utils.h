/**************************************************************************************/
/*!
 *    \file     gp_utils.h
 *
 *    \copyright 2016
 *
 *    \brief    Header file containing the public interfaces and data for common software 
 *				component utility functions.
 *
 *    \author   D. Kageff
 *
 *    \version  $Revision: 1.17 $  $Log $
 *
 */
/***************************************************************************************/
#ifndef _GP_UTILS_H
#define _GP_UTILS_H

//TODO: replace dependencies from this include

#include "gp_types.h"		// global GP data type definitions
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************
	Public Macros and Typedefs
***********************************/

/*! Defined debug print verbosity levels */
typedef enum {
	VRB_NONE	= 0,		/*!< Don't print anything */
	VRB_RUNTIME,			/*!< Standard run time verbosity */
	VRB_DEBUG1,				/*!< Minimal debug verbosity */
	VRB_DEBUG2,				/*!< Debug verbosity level 2 */
	VRB_MAX,				/*!< Maximum verbosity */
	VRB_LEVELS
} DbgVerbosity_t;

/*! GP read/write functions endian flag */
typedef enum {
	GP_RW_LE	= 0,		/*!< Read/write Little Endian format */
	GP_RW_BE				/*!< Read/write Big Endian format */
} GP_RW_ENDIAN_T;


/***********************************
		Public API Functions
***********************************/
/* standard error checking and handling functions */
int32_t _GhCheck(int32_t e, char *err_string, int errln);
#define GhCheck(errcode) _YzCheck(errcode, __FILE__, __LINE__)	/*!< Wrapper macro function that supplies filename and line number to _GhCheck() */

int32_t _YzCheck(int32_t code, char *err_string, int errln);
#define YzCheck(errcode) _YzCheck(errcode, __FILE__, __LINE__)	/*!< Wrapper macro function that supplies filename and line number to _YzCheck() */

/* Functions that store multibyte values into a buffer, in a byte-packed manner */ 
int gp_Store16bitE(uint16_t value, uint8_t *p_buf, int endian);		// Store 16 bit value BE or LE format
int gp_Store16bitBE(uint16_t value, uint8_t *p_buf);				// Store 16 bit value BE format
int gp_Store16bitSigned(int16_t value, uint8_t *p_buf);				// Store 16 bit signed value BE or LE format
int gp_Store32bitE(uint32_t value, uint8_t *p_buf, int endian);		// Store 32 bit value BE or LE format
int gp_Store32bitBE(uint32_t value, uint8_t *p_buf);				// Store 32 bit value BE format
int gp_Store32bitSigned(int32_t value, uint8_t *p_buf);				// Store 32 bit signed value BE or LE format
int gp_StoreFloat(float value, uint8_t *p_buf);						// Store single precision float
int gp_Store64bit(uint64_t value, uint8_t *p_buf);					// Store 64 bit value BE or LE format
int gp_Store64bitSigned(int64_t value, uint8_t *p_buf);				// Store 64 bit signed value BE or LE format
int gp_StoreDouble(double value, uint8_t *p_buf);               	// Store double value
int gp_Read16bitE(uint16_t *p_value, uint8_t *p_buf, int endian);	// Read a 16 bit value from the buffer in BE or LE format
int gp_Read16bitBE(uint16_t *p_value, uint8_t *p_buf);				// Read a 16 bit value from the buffer in BE format
int gp_Read16bitSigned(int16_t *p_value, uint8_t *p_buf);			// Read a 16 bit signed value from the buffer in BE or LE format
int gp_Read32bitE(uint32_t *p_value, uint8_t *p_buf, int endian);	// Read a 32 bit value from the buffer
int gp_Read32bitBE(uint32_t *p_value, uint8_t *p_buf);				// Read a 32 bit value from the buffer in Big Endian format.
int gp_Read32bitSigned(int32_t *p_value, uint8_t *p_buf);			// Read a 32 bit signed value from the buffer
int gp_ReadFloat(uint32_t *p_value, uint8_t *p_buf);				// Read a single precision float value from the buffer
int gp_Read64bit(uint64_t *p_value, uint8_t *p_buf);				// Read a 64 bit value from the buffer
int gp_Read64bitSigned(int64_t *p_value, uint8_t *p_buf);			// Read a 64 bit signed value from the buffer
int gp_ReadDouble(uint64_t *p_value, uint8_t *p_buf);           	// Read a double value from buffer

void gp_Printf(DbgVerbosity_t vrb, const char * format, ...);		// Special debug print function
void gp_OpenLogBuf(char **pBuf, int size);							// Create or set a logging buffer
void gp_GetLogBuf(char **pBuf, int *pSize);							// Return log buffer information

void gp_Cnvrt2Wstr(char *p_wstr, char *p_str);						// Convert 8bit char string to 16bit string

/* Macro functions for backward compatability */
#ifdef IPC_BIG_ENDIAN
 #define gp_Read16bit(p_value, p_buf) gp_Read16bitE(p_value, p_buf, 1);
#else
 #define gp_Read16bit(p_value, p_buf) gp_Read16bitE(p_value, p_buf, 0);
#endif
#define gp_Read16bitLE(p_value, p_buf) gp_Read16bitE(p_value, p_buf, 0);

#ifdef IPC_BIG_ENDIAN
 #define gp_Store16bit(value, p_buf) 	gp_Store16bitE(value, p_buf, 1);
#else
 #define gp_Store16bit(value, p_buf)	gp_Store16bitE(value, p_buf, 0);
#endif
#define gp_Store16bitLE(value, p_buf) gp_Store16bitE(value, p_buf, 0);

#ifdef IPC_BIG_ENDIAN
 #define gp_Read32bit(p_value, p_buf) gp_Read32bitE(p_value, p_buf, 1);
#else
 #define gp_Read32bit(p_value, p_buf) gp_Read32bitE(p_value, p_buf, 0);
#endif
#define gp_Read32bitLE(p_value, p_buf) gp_Read32bitE(p_value, p_buf, 0);

#ifdef IPC_BIG_ENDIAN
 #define gp_Store32bit(value, p_buf) gp_Store32bitE(value, p_buf, 1);
#else
 #define gp_Store32bit(value, p_buf) gp_Store32bitE(value, p_buf, 0);
#endif
#define gp_Store32bitLE(value, p_buf) gp_Store32bit(value, p_buf, 0);



#ifndef __cplusplus
 //size_t strlcpy(char * restrict dst, const	char * restrict	src, size_t size);	// String copy that adds NULL to the end of the string
 size_t strlcpy(char * dst, const char * src, size_t size);
#endif

void gp_TimerSub(struct timeval *stop, struct timeval *start, struct timeval *result);	// Calc time difference

#ifdef __cplusplus
}
#endif

#endif      // end _GP_UTILS_H

