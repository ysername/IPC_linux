/**************************************************************************************/
/*!
 *    \file     cmd_conn.h
 *
 *    \copyright 2016
 *
 *    \brief    Header file containing the public interfaces and data for the command 
 *				connection management functions.  Command connections are used to provide
 *				communication between address spaces/dynamic downloads in the system.
 *
 *    \author   D. Kageff
 *
 *    \version  $Revision: 1.11 $  
 *				$Log $
 *
 */
/***************************************************************************************/
#ifndef _CMD_CONN_H
#define _CMD_CONN_H

/***********************************
	Public Macros and Typedefs
***********************************/
#define CONN_NAME_LEN  30		/*!< Maximum length of connection name string*/

#define OPCMD_TIMEOUT_DFLT -1	/*!< Use default timouts if time:seconds and time:fraction set to this. */

/*! Connection state enum */
typedef enum{
	CONN_NOT_INIT = 0,			/*!< Connection not initialized */
	CONN_ACTIVE,				/*!< Connection is active */
	CONN_FAULTED				/*!< Connection is faulted */
} CONN_STATE_T;

/*! Connection registering and usage options */
typedef enum {
	CONN_OPT_NONLINKABLE 	= (1<<0),		/*!< Bitflag for registering a non-linkable resource */
	CONN_OPT_LINKABLE 		= (1<<1),		/*!< Bitflag for registering a linkable resource */
	CONN_OPT_ASYNCH			= (1<<2),		/*!< Bitflag for creating an asynchronous connection */
	CONN_OPT_NO_CREATE		= (1<<3)		/*!< Do not create a connection pair (already created by GHS Integrate). */
} CMD_CONN_OPT_T;

/*! Command connection entry structure 
typedef struct
{
	//Connection conn;replace with connection_path					/*!< Shell command client connection object 
	char cname[CONN_NAME_LEN];			/*!< Connection name string 
	uint32_t conn_opts;					/*!< Connection registering and use options 
	Activity Act;						/*!< Connection activity if asynchronous 
	int Act_id;							/*!< Activity ID 
	Time timeout;						/*!< Timeout value structure 
	Buffer bfr;							/*!< Connection buffer if required 
	int conn_state;						/*!< Connection state of type ::CONN_STATE_T 
	bool status_recvd;					/*!< Set true when the first status response is received 
} CMD_CONN_DESCR_T;
*/
/* Cmmand message definitions */
/*! Special command field offsets in status buffer */
enum {
	OFFSET_CMD_ID	= 0,		/*!< Special command value, typically CMD_CODE_ID */
	OFFSET_SUBID	= 1,		/*!< Special command subid */
	OFFSET_CMD_LEN	= 2,		/*!< 16 bit command data length, not including CMD_ID - CMD_LEN fields */
	OFFSET_CMD_DATA = 4			/*!< Start of command data */
};
#define CMD_HDR_LEN OFFSET_CMD_DATA	/*!< Length of the command msg header in bytes */

/*! Special command codes.  These codes are in the CMD_ID buffer location and indicate that the command
	is a non-text transfer. */
#define CMD_CODE_ID	 0xff		/*!< Indicates that a general command code is in the buffer */
#define CMD_DIAG_ID	 0xfe		/*!< Indicates that a diagnostic command code is in the buffer */

/*! General command subcodes (sub IDs).  These sub Ids are used with the CMD_CODE_ID command 
	code ID and follow it the  in the status buffer */
typedef enum {
	CMD_SUBID_VERSIONS	= 1,		/*!< Request executable versions (numeric form) */
	CMD_SUBID_INFO_TXT,				/*!< Request system information (text form) */
	CMD_SUBID_STATUS_TXT,			/*!< Request system status */
	CMD_SUBID_APP_CTRL,			/*!< Request application stop/unload */
	CMD_SUBID_APPVERS,				/*!< Request application executable versions (numeric form) */
	CMD_SUBID_APPINFO_TXT,			/*!< Request system information (text form) */
	CMD_SUBID_STATUS,				/*!< General status message.  */
	CMD_SUBID_CMDSZ,				/*!< Get command response size, in bytes, from receiver.  The cmd
										 sub ID of the command response size desired is passed in this 
										 message. */
	CMD_SUBID_RDLOG,				/*!< Get log from receiver */
	CMD_SUBID_HWINFO,				/*!< Get hardware info in text form */
	NUM_CMD_IDS,
	CMD_SUBID_ERR_RESP	= 255		/*!< command error response */
} CMD_SUBIDS_T;

/*! Diagnostic command subcodes (sub IDs).  These sub Ids are used with the CMD_DIAG_ID command 
	code ID and follow it the  in the status buffer */
typedef enum {
	CMD_SUBID_DIAG_REQ,				/*!< General diagnostic request message in UDS format */
	CMD_SUBID_READ_REQ,				/*!< Diagnostic read data item request message */
	CMD_SUBID_WRITE_REQ,			/*!< Diagnostic write data item request message */
	CMD_SUBID_DRTN_REQ,				/*!< Diagnostic execute diagnostic routine request message */
	CMD_SUBID_DLOAD_REQ,			/*!< Diagnostic download request message */
	CMD_SUBID_DL_XFER_REQ,			/*!< Diagnostic download block transfer message */
	CMD_SUBID_UDS_XFER,				/*!< UDS message transfer to/from the VP */
	CMD_SUBID_UDS_XFER_EXIT,		/*!< UDS message transfer mode exit message */
	CMD_SUBID_RUN_STATE,			/*!< Application run state message */
	CMD_SUBID_DISP_STATUS,			
	CMD_SUBID_DIAG_NACK,			/*!< Diagnostic command NACK message */
	CMD_NUM_SUBIDS					/*!< Number of valid cmd sub IDs.  This must be last. */
} DIAG_SUBIDS_T;

/* Diagnostic command message return codes */
#define CMD_DIAG_ACK	0			/*!< General diagnostic acknowledgement code.  Other positive codes added will be
										 greater than 0. */
/*! Diagnostic message NACK codes */
typedef enum {
	DIAG_NACK_GENERR	= -1,		/*!< General diag command error */
	DIAG_NACK_NOTSUPP	= -2,		/*!< Diag command not supported error */
	DIAG_NACK_INVALID	= -3,		/*!< Diag command invalid error */
	DIAG_NACK_NOTFND	= -4,		/*!< Diag command not found */
	DIAG_NACK_OOR		= -5,		/*!< Diag request or parameter Out Of Range */
} DIAG_NACK_CODES_T;

#define NUM_DIAG_NACK_CODES (ABS(DIAG_NACK_OOR)		/*!< Number of NACK codes */

/*! Diagnostic cmd NACK message offsets */
#define CMD_DIAG_NACK_SUBID	(OFFSET_CMD_DATA + 0)	/*!< Offset to NACK'd command sub ID field */
#define CMD_DIAG_NACK_CODE	(OFFSET_CMD_DATA + 1)	/*!< Offset to NACK'd command error code field. Use UDS values
														 defined in uds_core.h */

/*! Diagnostic DID read/write cmd message offsets */
#define CMD_DIAG_DID		(OFFSET_CMD_DATA + 0)	/*!< Offset to DID field */
#define CMD_DIAG_DID_LEN	(OFFSET_CMD_DATA + 2)	/*!< Offset to DID data length field */
#define CMD_DIAG_DID_VAL	(OFFSET_CMD_DATA + 3)	/*!< Offset to the start of the DID data value field */

/*! Diagnostic command execute routine message offsets */
#define CMD_DIAG_RTN_CTRL	(OFFSET_CMD_DATA + 0)	/*!< Offset to diagnostic routine control value. Uses 
														 Routine Control (0x31) sub ID values. */ 
#define CMD_DIAG_RTN_ID		(OFFSET_CMD_DATA + 1)	/*!< Offset to 16bit diagnostic routine ID. */
#define CMD_DIAG_RTN_OPTS	(OFFSET_CMD_DATA + 3)	/*!< Offset to the start of diagnostic routine ID arguments.*/
 

/*! Diagnostic command download request message offsets */
#define CMD_DIAG_DLOAD_ID	(OFFSET_CMD_DATA + 0)	/*!< Offset to download block ID.  This is used to determine the 
														 proper setup and processing of the download request */
#define CMD_DIAG_DLOAD_LEN	(OFFSET_CMD_DATA + 4)	/*!< Offset to download length field */
#define CMD_DIAG_DLREQ_MLEN	8						/*!< Length of download request fields, in bytes */

/*! Diagnostic download block transfer message offsets */
#define CMD_DIAG_DL_BLK_SEQ	(OFFSET_CMD_DATA + 0)	/*!< Offset to 16bit download block sequence number.  This is used to  
														 confirm the proper order of received blocks. */
#define CMD_DIAG_DL_BLK_LEN	(OFFSET_CMD_DATA + 2)	/*!< Offset to download block length field */
#define CMD_DIAG_DL_BLK_DATA (OFFSET_CMD_DATA + 6)	/*!< Offset to start of block data. */

/*! Diagnostic block transfer exit command message */
#define CMD_DIAG_EXIT_PARAM (OFFSET_CMD_DATA + 0)	/*!< Offset to start of transfer exit parameter field. */

/*! Application run state message field offsets */
#define CMD_RS_STATE 	(OFFSET_CMD_DATA + 0)		/*!< Offset to start of run state parameter field. */
#define CMD_RS_CONFIRM	(OFFSET_CMD_DATA + 1) 		/*!< 32bit confirmation value. XOR this with GP_RS_CONF_SHUTDOWN */

/*! Application run control state definitions */
typedef enum {
	CMD_RS_STATE_STOP	= 0,			/*!< Stop the application */
	CMD_RS_STATE_START,					/*!< Start a loaded application */
	CMD_RS_STATE_CONT,					/*!< Continue the application(s) from where it/they stopped */
	CMD_RS_STATE_RESTART,				/*!< Restart/reboot the application(s) */
	CMD_RS_STATE_RESET,
	CMD_RS_STATE_SHUTDOWN,				/*!< Shutdown/unload the any applications */
	NUM_RS_STATES						/*!< this must be last */
} CMD_RS_STATES_T;

#define GP_RS_CONF_VALUE (uint32_t)(0x5aa5dead)	/*!< GP run control state confirmation value.  XOR'd with value
											 		 received from VP.  XOR result of zero confirms operation */


/*! Special command subcodes field data definitions */
#define STOP_ADD_ID 0xa55a

/*! General status (CMD_SUBID_STATUS) message definitions */
#define APP_STAT_DATLEN 3
#define APP_STAT_OP		(OFFSET_CMD_DATA + 0)	/*!< Application status operation type, ::APP_STAT_OP_T */
#define APP_STAT_ID 	(OFFSET_CMD_DATA + 1)	/*!< Target object of application status, type ::APP_STAT_TARG_T */
#define APP_STAT_CODE	(OFFSET_CMD_DATA + 2)	/*!< Target app status, type ::APP_STAT_CODE_T */

/*! Application status operation type */
typedef enum {
	APP_STAT_REQ,				/*!< Application status request */
	APP_STAT_RESP,				/*!< Application status response */
	APP_STAT_RPT,				/*!< Application status asynchronous report */
	APP_STAT_OPS				/*!< Number of defined app status operations.  This must be last in the list. */
} APP_STAT_OP_T;

/*! Application status target app definitions */
typedef enum {
	APP_ID_GENERAL = 0,		/*!< General (no-target specific) status. */
	APP_ID_BTLDR,				/*!< Bootloader status */
	APP_ID_MAIN,				/*!< GP main app */
	APP_ID_HMI,				/*!< HMI */
	NUM_APP_STAT_IDS		/*!< Number of defined app status target IDs.  This must be last in the list. */
} APP_STAT_ID_T;

/*! Application status definitions. Used by response and report operation types. */
typedef enum {
	APP_STAT_NOT_STARTED = 0,		/*!< Application has not started */
	APP_STAT_STARTED,				/*!< Application has started */
	APP_STAT_RUNNING,				/*!< Application is running */
	APP_STAT_PAUSED,				/*!< Application is paused */
	APP_STAT_STOPPED,				/*!< Application has stopped */
	APP_STAT_DIAGMODE,				/*!< Application is running in a diagnostic mode */
	APP_STAT_SHUTDOWN,				/*!< Application has been shutdown */
	APP_STAT_ERROR,					/*!< Application start/run error */
	APP_STAT_ANIM_DONE,
	NUM_APP_STATS					/*!< Number of defined app status code.  This must be last in the list. */
} APP_STAT_CODE_T;

/*! Display status information (CMD_SUBID_DISP_STATUS) message definitions*/
#define DISP_STAT_OP (OFFSET_CMD_DATA + 0)
#define DISP_OP_LEN 1
#define DISP_STAT_FF_OFFSET (OFFSET_CMD_DATA + DISP_OP_LEN)
#define DISP_STAT_FILES (OFFSET_CMD_DATA + 2)
#define DISP_STAT_STR (OFFSET_CMD_DATA + 1)
#define DISP_STAT_ARG (OFFSET_CMD_DATA + 1)

typedef enum{
	DISP_OP_INIT,
	DISP_OP_STR,
	DISP_OP_STOP,
	NUM_DISP_OPS
} DISP_STAT_OP_T;
/***********************************
		Public API Functions
***********************************/
/* Initialize and register a command connection 
int SetCmdConnection(CMD_CONN_DESCR_T *p_cdescr, char *loc_cname, char *rem_cname, bool asynch);

/* Close a command connection 
int CloseCmdConnection(CMD_CONN_DESCR_T *p_cdescr);

/* Get a command connection 
int GetCmdConnection(CMD_CONN_DESCR_T *p_cdescr, char *loc_cname, bool asynch, int retries);

/* Send a command and get the response 
int SendCmd(CMD_CONN_DESCR_T *p_cdescr, uint8_t *p_buf, int *p_len, int buflen);

/* Receive a command
int RecvCmd(CMD_CONN_DESCR_T *p_cdescr, uint8_t *p_buf, int *p_len, int buflen);*/

#endif
