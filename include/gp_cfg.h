/******************************************************************************/
/*!
 *  \file		gp_cfg.h
 *
 *  \copyright	Yazaki 2016-17
 *
 *  \brief		Common GP bootloader and application configuration settings and definitions.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.32 $ 
 *				$Log: gp_cfg.h  $
 *				Revision 1.32 2017/07/20 12:13:25CDT Daniel Kageff (10011932) 
 *				Added MAX_UPD_FNAME defintion
 *				Revision 1.31 2017/06/27 18:08:22EDT Daniel Kageff (10011932) 
 *				Set download processing task priority higher (DFLT_DLPROC_PRIORITY)
 *				Revision 1.30 2017/06/21 17:28:26EDT Daniel Kageff (10011932) 
 *				Added default download processing task priority constant
 *				Revision 1.29 2017/06/02 11:04:35EDT Daniel Kageff (10011932) 
 *				Updated file path defintions
 *				Revision 1.28 2017/04/24 19:09:25EDT Daniel Kageff (10011932) 
 *				Added BASE_DIR definition
 *				Revision 1.27 2017/03/24 11:09:17EDT Daniel Kageff (10011932) 
 *				Added system configuration filename definition
 *				Revision 1.26 2017/03/21 16:35:57EDT Daniel Kageff (10011932) 
 *				Moved VP SWver and part# element length definitions to pool_def.h
 *				Revision 1.25 2017/03/20 15:20:24EDT Eudora Gunarta (10031829) 
 *				Added macros used for handling VP_SwVersionInfo SPI message.
 *				Revision 1.24 2017/03/16 12:34:42EDT Daniel Kageff (10011932) 
 *				Added connection names for runtime command connections.
 *				Revision 1.23 2017/02/22 18:17:44EST Daniel Kageff (10011932) 
 *				1) Update debug print verbosity level definition
 *				2) Added comments
 *				Revision 1.22 2017/02/17 17:49:17EST Daniel Kageff (10011932) 
 *				Added IPC communication timeout value constants
 *				Revision 1.21 2017/02/16 17:31:59EST Daniel Kageff (10011932) 
 *				1) Changed GRAPHICS_DIR definition 
 *				2) Added HMI_DEMO_DIR definition
 *				3) Added IPC message OPCMD_BUF_LEN definition
 *				Revision 1.20 2017/02/09 14:28:35EST Daniel Kageff (10011932) 
 *				Added MAX_FEXT_LEN, max file extension string length
 *
 *
 ***************************************************************************************/
#ifndef _GP_CFG_H
#define _GP_CFG_H

/*************************************************************/
/*! \brief 	RELEASE_BUILD - Debug/release build macro definition.
 *					If defined, disables any debug related
 *					code in the project.
 *************************************************************/	
#ifdef DEBUG_BUILD
  /* Enable debug options */
#endif

#ifdef USE_HOST_IO
  /* Use Host I/O path.  Note: This is specific to a development PC.  If your paths are different, edit a local copy of this file and modify the paths. */
  #define ROOT_DIR "C:/Working Directory/Software Projects Archives-N/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Object"
  #define CFG_DIR "C:/Working Directory/Software Projects Archives-N/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Object"
  #define CANDEF_DIR "C:/Working Directory/Software Projects Archives-N/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Object"
  #define APP_DIR "C:/Working Directory/Software Projects Archives-N/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Object"
  #define UPDATE_DIR "C:/Working Directory/Software Projects Archives-N/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Object"
  #define GRAPHICS_DIR "Graphics"	/*!< Assumes the CWD to /mydisk has been done.  Does not use Host I/O (Not a path on the development PC) */
  #define HMI_BASE_DIR "/mydisk"	/*!< Base directory for HMI application.  Does not use a Host I/O path on the development PC. */
  #define MAX_PATH_LEN	200			/*!< Maximum file path string buffer length. */
  #define MAX_UPD_FNAME_LEN	200		/*!< Maximum update filename length */
#else
  /* Use flash file system on target */
  #define ROOT_DIR "/mydisk"			/*!< Root file directory */
  #define CFG_DIR "/mydisk"
  #define CANDEF_DIR "/mydisk" 
  #define APP_DIR "/mydisk"
  #define UPDATE_DIR "/mydisk"		/*!< directory where update files are located.  Used by NOR_Update */
  #define HMI_BASE_DIR "/mydisk"	/*!< Base directory for HMI application.  Does not use a Host I/O path on the development PC. */
  #define GRAPHICS_DIR "Graphics"	/*!< Assumes the CWD to /mydisk has been done */
  #define MAX_PATH_LEN	40			/*!< Maximum file path string buffer length. */
  #define MAX_UPD_FNAME_LEN	80		/*!< Maximum update filename length */
#endif

#define BASE_DIR "/mydisk"			/*!< Base download directory used by diagnostic file or image transfers */
#define HMI_DEMO_DIR "Demo"			/*!< Demo graphics files directory. Assumes the CWD to APP_DIR has been done */
#define GRAPHICS_DIR "Graphics"		/*!< Assumes the CWD to APP_DIR has been done */

#define TSK_OK (0x5Au)		/*!< Used for IPC communication */

/* Configuration file loading definitions */
#define BT_CFG_FNAME "app_start.ini"					/*!< Default bootloader, GP app boot time configuration filename */
#define SYS_CFG_FNAME "hud_cfg.ini"						/*!< Default HUD system configuration filename */
#define MAX_FNAME_LEN 40								/*!< Max program filename length */
#define MAX_FEXT_LEN  6									/*!< Max file extension string length */

/* Dynamic download application loading definitions */
#define MAX_PGMFILE_LEN	(MAX_PATH_LEN+MAX_FNAME_LEN)	/*!< Max filename/path string length. */
#define DFLT_DD_EXT	".elf"		/*!< Default DD executable file extension */

/* Common HMI application definitions */
#define HMI_APP_NAME "hmi"		/*!< Base HMI application executable name */

/* IPC communication control definitions */
#define IPC_GEN_TIMEOUT		(1000)				/*!< General (default) IPC communication timeout */
#define IPC_HBT_TIMEOUT		(1000)				/*!< Unit Manager heartbeat message IPC comm timeout */
#define IPC_DP_HMI_TIMEOUT	(1000)				/*!< DP-HMI IPC communication timeout, in msec */

/* Common Unit Manager - HMI Manager interface definitions */
#define UM_HMI_TASK_CON "UmHmIntTskCon"		/*!< Name for connection between Unit mgr and HMI subsystem initial task.
											 	It is the remote end (Hm_UmAsCon1) of the UM-HMI connection */
#define UM_HMI_DPLTASK_CON "UmHmDplTskCon"	/*!< Name for connection between Unit mgr and HMI subsystem datapool task.
											 	It is the remote end (Hm_UmAsCon1) of the UM-HMI connection */

/* Common Datapool Manager - HMI Manager interface definitions */
#define PM_HMI_TASK_CON "PmHmIntTskCon"		/*!< Name for connection between Datapool mgr and HMI subsystem.
											 	It is the remote end (Hm_PmAsCon1) of the PM-HMI connection */

/* Common IPC Manager - HMI Manager interface definitions */
#define IM_HMI_TASK_CON "ImHmIntTskCon"		/*!< Name for connection between IPC mgr and HMI subsystem.
											 	It is the remote end (Hm_ImAsCon1) of the IM-HMI connection */

/* Diagnostic handler command connection definitions */
#define DIAG_BTLDR_CNAME_LOC "DiagBtldrLocConn"	/*!< Name of local connection for the GP bootloader main */
#define DIAG_BTLDR_CNAME_REM "DiagBtldrRemConn"	/*!< Name of remote connection for the GP bootloader main */
#define DIAG_APP_CNAME_LOC	"DiagAppLocConn"	/*!< Name of local connection for the GP application Diagnostic Manager */
#define DIAG_APP_CNAME_REM	"DiagAppRemConn"	/*!< Name of remote connection for the GP application Diagnostic Manager */
//#define IPC_MGR_CNAME_LOC	"IpcMgrLocConn"		/*!< Name of local connection for the GP application IPC Manager */
//#define IPC_MGR_CNAME_REM	"IpcMgrRemConn"		/*!< Name of remote connection for the GP application IPC Manager */

/* OpCmd IPC message global configuration definitions */
#define OPCMD_BUF_LEN (128)					/*!< Maximum size of OpCmd IPC message buffer.  Limit is currently 128 bytes. */

/* CAN definition file processing definitions */
#define DFLT_CANDEF_XML1 "out_Chassis.xml"	/*!< Chassis CAN (CCAN) message definitions. */
#define DFLT_CANDEF_XML2 "out_body.xml"		/*!< Body CAN (BCAN) message definitions. */
#define MAX_IPC_MSGLEN	128					/*!< Maximum length of an IPC message, in bytes */

#ifdef DEBUG
 #define OPCMD_TIMEOUT_SEC 1000		/*!< Default operation command seconds timeout for debug builds */
 #define OPCMD_TIMEOUT_FRAC 0		/*!< Default operation command fraction timeout for debug builds */
#else
 #define OPCMD_TIMEOUT_SEC 2		/*!< Default operation command seconds timeout for release builds */
 #define OPCMD_TIMEOUT_FRAC 0		/*!< Default operation command fraction timeout for debug builds */
#endif

/* Download session configuration definitions */
#define DFLT_DLPROC_PRIORITY	155		/*!< Default download processing task priority. Same priority as DoIP tasks, 
											 BASE_TASK_PRIORITY, set in basic_thread.cpp */

/******************************************************/
/*  Vehicle data simulator configuration and profile
 *	files.
 ******************************************************/
#define VSIM_CFG_FNAME "/VsimProfileConfig.txt"		/*!< Vsim profile configuration filename */

/*****************************************************
		Set debug print verbosity level
 ****************************************************/
#include "gp_utils.h"			// for vebosity levels
#define DFLT_DBG_PRNTLVL  VRB_DEBUG1	/*!< Assign gp_Printf output verbosity based on ::DbgVerbosity_t in gp_utils.h */
		
#endif
