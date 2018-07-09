/******************************************************************************/
/*!
 *  \file		gp_types.h
 *
 *  \copyright	Yazaki 2016
 *
 *  \brief		Common data typedefs for GP application.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.19 $  
 *				$Log $
 *
 *
 ***************************************************************************************/
#ifndef _GP_TYPES_H
#define _GP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include "types.h"

typedef void (*function_cb_t)(void *);

#define MAX_MSG_BUFFER 256 

typedef enum {
    RSIZE4 = 0,
    RSIZE8,
    RSIZE16,
    RSIZE32,
    RSIZE64,
    RSIZE128,
    RSIZE256
}requested_size_t;

/*! Standard GP application initialization/shutdown types */
typedef enum {
	INIT_COLD	= 0,			/*!< Cold/POR start initialization */
	INIT_WARM,					/*!< Warm restart */
	INIT_WAKEUP,				/*!< Wakeup initialization */
	SHUTDN_NORM,				/*!< Shutdown request due to normal shutdown */
	SHUTDN_ERR,					/*!< Shutdown request due to fatal error. */
	SHUTDN_SLEEP,				/*!< Shutdown due to sleep request. */
	NUM_INIT_TYPES
} gp_init_types_t;

/*! Standard GP application return codes */
typedef enum {
	GP_SUCCESS = 0,				/*!< Successful operation */
	GP_GENERR = -1,				/*!< General (non-specific) error */
	GP_NOTSUPP = -2,			/*!< Operation not supported error */
	GP_INIT_ERR = -3,			/*!< Initialization error */
	GP_IPC_TXERR = -4,			/*!< IPC transmit request error */
	GP_IPC_RXERR = -5,			/*!< IPC receive request error */
	GP_IPC_GENERR = -6,			/*!< General IPC request error */
	GP_CAN_DEF_ERR = -7,		/*!< CAN definition error */
	GP_CAN_DEF_SIGS = -8,		/*!< CAN definition too many message signals error */
	GP_OPCMD_ERR = -9,			/*!< Operation command error */
	GP_OPCMD_NOTSUPP = -10,		/*!< Operation command not supported */
	GP_OPCMD_TIMEOUT = -11,		/*!< Operation command communication timeout */
	GP_CFGFILE_OPENERR = -12,	/*!< Configuration file open error */
	GP_CFGFILE_INVALID = -13,	/*!< Configuration file invalid */
	GP_CFGFILE_PRCERR = -14,	/*!< Configuration file processing error */
	GP_DP_PARMS_ERR = -15,		/*!< Invalid datapool Set/Get parameters */
	GP_DP_ACCESS_ERR = -16,		/*!< Error accessing datapool item data */
	GP_DP_DATA_ERR = -17,		/*!< Datapool data type mismatch */
	GP_ACT_CRTERR = -18,        /*!< Failed to create an activity */
	GP_ACT_NOTFOUND = -19,      /*!< No activity found with associated activity ID */
	GP_ACT_RSTERR = -20,        /*!< Failed to reset the activity */
	GP_ACT_SETPRIOERR = -21,    /*!< Failed to set message's activity priority */
	GP_MSG_NONEAVAIL = -23,     /*!< No message object available from the task's message pool */
	GP_MSG_TRERR = -24,         /*!< Failed to transfer the message */
	GP_TSK_IDERR = -26,         /*!< Failed to get task id */
	GP_TSK_IDXERR = -26,        /*!< Invalid task index */
	GP_CLK_SETERR = -27,	    /*!< Error setting clock object */
	GP_FILE_OPEN_ERR = -28,     /*!< Open file error */ 
	GP_FILE_SIZE_IVLD = -29,    /*!< Invalid file size */
	GP_FILE_READ_ERR = -30,     /*!< File read error */
	GP_FILE_PROC_ERR = -31,     /*!< File processing error */
	GP_NOTINITD = -32,          /*!< Was not initialized */
	GP_FNC_PARAM_IVLD = -33,    /*!< A function parameter had an invalid value */
	GP_MALLOC_ERR = -34,        /*!< Dynamic memory allocation error */
	NUM_GP_RETCODES
} gp_retcode_t;

/*! Data types enumeration. Used by vehicle data simulator, datapool, etc. */
typedef enum {
	
	GP_INT32	= 0,		/*!< 32 bit integer */
	GP_UINT32,				/*!< Unsigned 32 bit integer */
	GP_INT64,				/*!< 64 bit integer */
	GP_UINT64,				/*!< Unsigned 64 bit integer */
	GP_FLOAT,				/*!< Float */
	GP_DBL,					/*!< Double precision float */
	GP_STRING,				/*!< String type */
	GP_ARRAY,				/*!< Byte array type */
	GP_INT16,				/*!< 16 bit integer */
	GP_UINT16,				/*!< Unsigned 16 bit integer */
	PCF_NUM_DTYPES,
	GP_UINT8 /*leo*/
} GP_DATATYPES_T;

/*! Faraday Future software major version type definitions */
typedef enum {
	SWVER_ALPHA	= 0x00,		/*!< Alpha release major version value */
	SWVER_BETA	= 0x01,		/*!< Beta release major version value */
	SWVER_GAMMA	= 0x02,		/*!< Gamma release major version value */
	SWVER_VP	= 0x03,		/*!< VP(?) release major version value */
	SWVER_SOP	= 0x04,		/*!< Start OF Production release major version value */
} CUST_SWVER_T;

#define SWBLD_INFO_LEN 25	/*!< Length in bytes of software build information field */

/*! Standard software information structure.  Used to report version, revision, etc. */
typedef struct {
	int	version;					/*!< Software major version */
	int revision;					/*!< Software minor version (revision) */
	char bldinfo[SWBLD_INFO_LEN];	/*!< Freeform field for software build information */
} SW_INFO_T;

/*! Limits */
#define YZ_UINT64_MAX  18446744073709551615u
#define YZ_UINT32_MAX  4294967296u

/* Common macro functions */
#define ABS(x) (((x)<0) ? -(x) : (x))		/*!< Find the absolute value of x */

/*! Download image check value types */
typedef enum {
	DL_CHK_CRC32	= 0,		/*!< CRC32 check */
	DL_CHK_SUM32,				/*!< 32 bit arithmetic checksum */
	DL_CHK_MD5,					/*!< MD5 check */
	DL_CHK_SHA256,				/*!< SHA-256 check */
	NUM_DL_CHKTYPES				/*!< Number of check value types.  This must be last in the list. */
} DL_CHKTYPES_T;

/*!< Time constant conversion definitions */
#define USEC_PER_MSEC	1000ul		/*!< Number of microseconds per millisecond */
#define MSEC_PER_SEC	1000ul		/*!< Number of milliseconds per second */
#define USEC_PER_SEC	1000000ul	/*!< Number of microseconds per second */

#ifdef __cplusplus
}
#endif


#endif			// End _GP_TYPES_H
