/**************************************************************************************/
/*!
 *  \file		pool_def.h
 *
 *  \copyright	Yazaki 2016-17
 *
 *  \brief		Header file with datapool item IDs, control table, and data allocation.
 *				To add a new datapool item, do the following:
 *				-# Add an ID definition to the datapool ID enumeration, #DP_ELEMENT_IDS.
 *				-# Add a storage entry for the datapool item to ::DP_ITEM_STORAGE_T.
 *				-# Add a datapool access table entry of type ::DP_ITEM_ENTRY_T for the datapool item.
 *
 *  \author		E. Gunarta, D. Kageff
 *
 *  \version	$Revision: 1.14 $
*   $Log: pool_def.h  $
*   Revision 1.14 2017/10/17 13:12:46CDT Eudora Gunarta (10031829) 
*   Short, med and tall mirror position values will be updated with value in its corresponding PreWarpData.csv file.
*   Revision 1.13 2017/10/12 16:32:11EDT Daniel Kageff (10011932) 
*   Added data pool element YzTdAutoDrvCtrl
*   Revision 1.12 2017/09/05 16:48:30EDT Eudora Gunarta (10031829) 
*   Prewarp related changes (mesh reload, display options, new data type).
*   Revision 1.11 2017/03/21 16:40:02EDT Daniel Kageff (10011932) 
*   Increased the size of the VP SWver and part# elements
*   Revision 1.10 2017/03/20 09:36:23EDT Eudora Gunarta (10031829) 
*   Changes related to adding VP_SwVersionInfo SPI message handling.
*   Code cleanup.
*   Revision 1.9 2017/03/15 10:24:04EDT Eudora Gunarta (10031829) 
*   Changed name of YzTdWarpOn and YzTdWarpCalMode and their default values.
*   Revision 1.8 2017/02/24 15:17:44EST Daniel Kageff (10011932) 
*   Redesigned to handle multiple data types and add other features
*   Revision 1.7 2017/02/16 13:32:05EST Daniel Kageff (10011932) 
*   Added HMI option datapool item IDs
*   Revision 1.6 2016/12/12 18:53:50EST Daniel Kageff (10011932) 
*   Added Nav and audio display option datapool items
*   Revision 1.5 2016/12/05 16:29:45EST Daniel Kageff (10011932) 
*   Added datapool item IDs, changed pool copy res structures
*   Revision 1.4 2016/11/29 14:12:26EST Eudora Gunarta (10031829) 
*   Updates for dynamic prewarp.
*   Revision 1.3 2016/10/10 15:27:54EDT Fernando Villarreal Garza (10011234) 
*   added test pattern data pool definition
*   Revision 1.2 2016/10/03 09:49:10CDT Eudora Gunarta (10031829) 
*   Replaced "POOL_ITEM" with "ELEM".
*   Revision 1.1 2016/06/17 09:55:56EDT Eudora Gunarta (10031829) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Common/project.pj
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	The header file pool_def.h contains the datapool element definitions.
 *
 */
/***************************************************************************************/
#ifndef POOL_DEF_H
#define POOL_DEF_H

/*****************************************************************************/
/*    I N C L U D E   F I L E S                                              */
/*****************************************************************************/  
#include "gp_types.h"
#include "gp_cfg.h"
#include "IPC/spi_common_config.h"		// Get common GP-VP IPC definitions

/*****************************************************************************/
/*    M A C R O S                                                            */
/*****************************************************************************/ 
/* Size of element id in bytes */
#define ELEM_ID_SZ (2)		/*!< Size in bytes of the data element ID */

/* Size of element value in bytes */
#define ELEM32_SZ (4)		/*!< Size in bytes of a 32 bit data element */
#define ELEM64_SZ (8)		/*!< Size in bytes of a 64 bit data element */

/*! Max size of an element in bytes */
#define ELEM_MAX_SZ (ELEM64_SZ)

/* GP-VP IPC message information definitions */
#define MAX_VPSWVERSION_LEN  (VP_SW_VERSION_SZ+1)		/*!< VP SW version string.  Allow for NULL termination */
#define MAX_VPPARTNUMBER_LEN (VP_SW_PART_NUMBER_SZ+1)	/*!< VP part number string.  Allow for NULL termination */

/*****************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                            */
/*****************************************************************************/
/*! Datapool element IDs enumeration */
enum DP_ELEMENT_IDS
{
    ELEM_MIN_ID = 0,					/*!< Starting ID for datapool items */
    YzTdTurnLeftSig = ELEM_MIN_ID,		/*!< ID 0: Left turn signal state */
    YzTdTurnRightSig,					/*!< ID 1: Right turn signal state */
    YzTdSpeedValue,						/*!< ID 2: Speed signal value */
    YzTdPRNDL,							/*!< ID 3: PRNDL signal state */
    YzTdTestPattern,					/*!< ID 4: Test pattern selection */
    YzTdCruise,							/*!< ID 5: Cruise control signal state */
    YzTdHazard,							/*!< ID 6: Hazard indicators signal state */
    YzTdWarpLoad,						/*!< ID 7: Which mesh data to load */
    YzTdWarpDisplay,					/*!< ID 8: Which content to display/which mesh to use */
    YzTdReserved1,					/*!< ID 9: TBD */
    YzTdReserved2,					/*!< ID 10: TBD */
    YzTdReserved3,					/*!< ID 11: TBD */
    YzTdMirrorPos,						/*!< ID 12: Mirror current position */
	YzTdoSpeedMotorFront,				/*!< ID 13: Front motor speed from PTC_EMotorSpeed CAN message */
	YzTdoSpeedMotorRL,					/*!< ID 14: Left rear motor speed from PTC_EMotorSpeed CAN message */
	YzTdoSpeedMotorRR,					/*!< ID 15: Right rear motor speed from PTC_EMotorSpeed CAN message */
	YzTdoTorqueActualFront,				/*!< ID 16: Front motor torque from PTC_EMotorStatus CAN message */
	YzTdoTorqueActualRL,				/*!< ID 17: Left rear motor torque from PTC_EMotorStatus CAN message */
	YzTdoTorqueActualRR,				/*!< ID 18: Right rear motor torque from PTC_EMotorStatus CAN message */
	YzTdoNavOpts,						/*!< ID 19: HMI Navigation display control options */
	YzTdoAudioOpts,						/*!< ID 20: HMI audio display control options */
	YzTdoNavSimFname,					/*!< ID 21: HMI NavSimFname: Base simulation filename */
	YzTdoNavSimFrames,					/*!< ID 22: HMI NavSimFrames: Number of simulation frames */
	YzTdoNavFrameDly,					/*!< ID 23: HMI NavSimFrameDlyMs: Inter frame delay in msec */
	YzTdoNavLoopDly,					/*!< ID 24: HMI NavSimLoopDlyMs: Inter loop delay in msec */
	YzTdoAudioSimFname,					/*!< ID 25: HMI AudioSimFname: Base simulation filename */
	YzTdoAudioSimFrames,				/*!< ID 26: HMI AudioSimFrames: Number of simulation frames */
	YzTdoAudioFrameDly,					/*!< ID 27: HMI AudioSimFrameDlyMs: Inter frame delay in msec */
	YzTdoAudioLoopDly,					/*!< ID 28: HMI AudioSimLoopDlyMs: Inter loop delay in msec */
	YzTdVpSwVersion,                    /*!< ID 29: VP SW verson */
	YzTdVpPartNumber,                   /*!< ID 30: VP part number */
	YzTdAutoDrvCtrl,                   	/*!< ID 31: Autonomous drive mode control */
	ELEM_MAX_ID							/*!< Number of datapool items.  This entry must be last */
};


/******************************************************************************/
/*             T Y P E S   A N D   E N U M E R A T I O N S                    */
/******************************************************************************/

/*! Datapool control table entry structure definition */
typedef struct {
    uint16_t 		id;					/*!< Datapool item ID */
	GP_DATATYPES_T 	type;				/*!< Data type */
	int				datlen;				/*!< Length of the datapool item in bytes */
	void 		   *p_data;				/*!< Pointer to the datapool item storage */
	void		   *p_default;			/*!< Pointer to the element default value */
} DP_ITEM_ENTRY_T;

/*! Datapool item storage structure definition. Element size must match settings in dp_tbl[] */
typedef struct {
	uint32_t TurnLeftSig;				/*!< ID 0: Left turn signal */
	uint32_t TurnRightSig;				/*!< ID 1: Right turn signal */
    int32_t  SpeedValue;				/*!< ID 2: Speed signal value */
    uint32_t PRNDL;						/*!< ID 3: PRNDL signal state */
    uint32_t Test_Pattern;				/*!< ID 4: Test pattern selection */
    uint32_t Cruise;					/*!< ID 5: Cruise control signal state */
    uint32_t Hazard;					/*!< ID 6: Hazard indicators signal state */
    uint8_t WarpLoad;					/*!< ID 7: Which mesh data to load */
    uint8_t WarpDisplay;				/*!< ID 8: Which content to display/which mesh to use */
    uint32_t Reserved1;			/*!< ID 9: TBD */
    uint32_t Reserved2;				/*!< ID 10: TBD */
    uint32_t Reserved3;				/*!< ID 11: TBD */
    uint32_t MirrorPos;					/*!< ID 12: Mirror current position */
	uint32_t SpeedMotorFront;			/*!< ID 13: Front motor speed from PTC_EMotorSpeed CAN message */
	uint32_t SpeedMotorRL;				/*!< ID 14: Left rear motor speed from PTC_EMotorSpeed CAN message */
	uint32_t SpeedMotorRR;				/*!< ID 15: Right rear motor speed from PTC_EMotorSpeed CAN message */
	int32_t  TorqueActualFront;			/*!< ID 16: Front motor torque from PTC_EMotorStatus CAN message */
	int32_t  TorqueActualRL;			/*!< ID 17: Left rear motor torque from PTC_EMotorStatus CAN message */
	int32_t  TorqueActualRR;			/*!< ID 18: Right rear motor torque from PTC_EMotorStatus CAN message */
	uint32_t NavOpts;					/*!< ID 19: HMI Navigation display control options */
	uint32_t AudioOpts;					/*!< ID 20: HMI audio display control options */
	char NavSimFname[MAX_FNAME_LEN] ;	/*!< ID 21: HMI NavSimFname: Base simulation filename */
	uint16_t NavSimFrames;				/*!< ID 22: HMI NavSimFrames: Number of simulation frames */
	uint16_t NavFrameDly;				/*!< ID 23: HMI NavSimFrameDlyMs: Inter frame delay in msec */
	uint32_t NavLoopDly;				/*!< ID 24: HMI NavSimLoopDlyMs: Inter loop delay in msec */
	char AudioSimFname[MAX_FNAME_LEN];	/*!< ID 25: HMI AudioSimFname: Base simulation filename */
	uint16_t AudioSimFrames;			/*!< ID 26: HMI AudioSimFrames: Number of simulation frames */
	uint16_t AudioFrameDly;				/*!< ID 27: HMI AudioSimFrameDlyMs: Inter frame delay in msec */
	uint32_t AudioLoopDly;				/*!< ID 28: HMI AudioSimLoopDlyMs: Inter loop delay in msec */
	char VpSwVersion[MAX_VPSWVERSION_LEN];		/*!< ID 29: VP SW verson */
	char VpPartNumber[MAX_VPPARTNUMBER_LEN];	/*!< ID 30: VP part number */
    uint32_t AutoDriveCtrl;				/*!< ID 31: Autonomous drive mode control */
} DP_ITEM_STORAGE_T;


/*! Datapool copy request response message structure definition */
typedef struct {
    uint8_t Id;				/*!< IPC message ID */
    DP_ITEM_STORAGE_T Dt;	/*!< Datapool image */
} PoolCopyResType;

		
/*****************************************************************************/
/*    				M E M O R Y   A L L O C A T I O N                        */
/*****************************************************************************/
/* Only allocate storage if this file is being included by Datapool.c. */
#ifdef _DATAPOOL_C

/*! Datapool item storage structure allocation */
static DP_ITEM_STORAGE_T dp_data;				/*!< Datapool item storage */


/*! Datapool item default value storage structure. It has the same structure format as the datapool 
	storage structure. */
static DP_ITEM_STORAGE_T dp_dfltvals =
{
	0, 0, 0, 0, 0,							/* TurnLeftSig, TurnRightSig, SpeedValue, PRNDL, Test_Pattern, */
	0, 0, 0, 0,								/* Cruise, Hazard, WarpMesh, WarpContent, */
	0, 0, 0, 0,					/* MirrorPosShort, MirrorPosMed, MirrorPosTall, MirrorPos, */		
	0, 0, 0,								/* SpeedMotorFront, SpeedMotorRL, SpeedMotorRR, */
	0, 0, 0,								/* TorqueActualFront, TorqueActualRL, TorqueActualRR */		
	0, 0,									/* NavOpts, AudioOpts */
	"nav_frame_",				            /* NavSimFname */
	0, 0, 0,								/* NavSimFrames, NavFrameDly, NavLoopDly */
	"audio_frame_",				            /* AudioSimFname */
	0, 0, 0,								/* AudioSimFrames, AudioFrameDly, AudioLoopDly */
	"",                                     /* YzTdVpSwVersion */
	"",                                     /* YzTdVpPartNumber */
	0										/* YzTdAutoDrvCtrl */
};	


/*! Datapool item access table allocation. This is indexed by the datapool item ID enum
   so entries must be in the same order as defined in the datapool ID enumeration. */
static const DP_ITEM_ENTRY_T dp_tbl[] = {
/*	*id*					*type*		*datlen* *p_data*				*/
{ 	YzTdTurnLeftSig, 		GP_UINT32,	4,		(void *)&dp_data.TurnLeftSig, (void *)&dp_dfltvals.TurnLeftSig },
{	YzTdTurnRightSig, 		GP_UINT32,	4,		(void *)&dp_data.TurnRightSig, (void *)&dp_dfltvals.TurnRightSig },	
{  	YzTdSpeedValue,			GP_INT32,	4,		(void *)&dp_data.SpeedValue, (void *)&dp_dfltvals.SpeedValue },			
{  	YzTdPRNDL,				GP_UINT32,	4,		(void *)&dp_data.PRNDL, (void *)&dp_dfltvals.PRNDL },							
{  	YzTdTestPattern,		GP_UINT32,	4,		(void *)&dp_data.Test_Pattern, (void *)&dp_dfltvals.Test_Pattern },		
{  	YzTdCruise,				GP_UINT32,	4,		(void *)&dp_data.Cruise, (void *)&dp_dfltvals.Cruise },				
{  	YzTdHazard,				GP_UINT32,	4,		(void *)&dp_data.Hazard, (void *)&dp_dfltvals.Hazard },				
{  	YzTdWarpLoad,			GP_UINT8,	1,		(void *)&dp_data.WarpLoad, (void *)&dp_dfltvals.WarpLoad },				
{  	YzTdWarpDisplay,		GP_UINT8,	1,		(void *)&dp_data.WarpDisplay, (void *)&dp_dfltvals.WarpDisplay },		
{  	YzTdReserved1,		GP_UINT32,	4,		(void *)&dp_data.Reserved1, (void *)&dp_dfltvals.Reserved1 },	
{  	YzTdReserved2,		GP_UINT32,	4,		(void *)&dp_data.Reserved2, (void *)&dp_dfltvals.Reserved2 },		
{  	YzTdReserved3,		GP_UINT32,	4,		(void *)&dp_data.Reserved3, (void *)&dp_dfltvals.Reserved3 },		
{  	YzTdMirrorPos,			GP_UINT32,	4,		(void *)&dp_data.MirrorPos, (void *)&dp_dfltvals.MirrorPos },			
{  	YzTdoSpeedMotorFront, 	GP_UINT32,	4,		(void *)&dp_data.SpeedMotorFront, (void *)&dp_dfltvals.SpeedMotorFront },	
{  	YzTdoSpeedMotorRL,		GP_UINT32,	4,		(void *)&dp_data.SpeedMotorRL, (void *)&dp_dfltvals.SpeedMotorRL },		
{  	YzTdoSpeedMotorRR,		GP_UINT32,	4,		(void *)&dp_data.SpeedMotorRR, (void *)&dp_dfltvals.SpeedMotorRR },		
{  	YzTdoTorqueActualFront, GP_INT32,	4,		(void *)&dp_data.TorqueActualFront, (void *)&dp_dfltvals.TorqueActualFront },	
{  	YzTdoTorqueActualRL,	GP_INT32,	4,		(void *)&dp_data.TorqueActualRL, (void *)&dp_dfltvals.TorqueActualRL },	
{  	YzTdoTorqueActualRR,	GP_INT32,	4,		(void *)&dp_data.TorqueActualRR, (void *)&dp_dfltvals.TorqueActualRR },	
{  	YzTdoNavOpts,			GP_UINT32,	4,		(void *)&dp_data.NavOpts, (void *)&dp_dfltvals.NavOpts },			
{  	YzTdoAudioOpts,			GP_UINT32,	4,		(void *)&dp_data.AudioOpts, (void *)&dp_dfltvals.AudioOpts },			
{  	YzTdoNavSimFname,		GP_STRING, MAX_FNAME_LEN,	(void *)&dp_data.NavSimFname, (void *)&dp_dfltvals.NavSimFname },		
{  	YzTdoNavSimFrames,		GP_UINT16,	2,		(void *)&dp_data.NavSimFrames, (void *)&dp_dfltvals.NavSimFname },		
{  	YzTdoNavFrameDly,		GP_UINT16,	2,		(void *)&dp_data.NavFrameDly, (void *)&dp_dfltvals.NavFrameDly },		
{  	YzTdoNavLoopDly,		GP_UINT32,	4,		(void *)&dp_data.NavLoopDly, (void *)&dp_dfltvals.NavLoopDly },		
{  	YzTdoAudioSimFname,		GP_STRING, MAX_FNAME_LEN,	(void *)&dp_data.AudioSimFname, (void *)&dp_dfltvals.AudioSimFname },		
{  	YzTdoAudioSimFrames, 	GP_UINT16,	2,		(void *)&dp_data.AudioSimFrames, (void *)&dp_dfltvals.AudioSimFrames },	
{  	YzTdoAudioFrameDly,		GP_UINT16,	2,		(void *)&dp_data.AudioFrameDly, (void *)&dp_dfltvals.AudioFrameDly },		
{  	YzTdoAudioLoopDly,		GP_UINT32,	4,		(void *)&dp_data.AudioLoopDly, (void *)&dp_dfltvals.AudioLoopDly },
{   YzTdVpSwVersion,        GP_STRING, MAX_VPSWVERSION_LEN, (void *)&dp_data.VpSwVersion, (void *)&dp_dfltvals.VpSwVersion },
{   YzTdVpPartNumber,       GP_STRING, MAX_VPPARTNUMBER_LEN, (void *)&dp_data.VpPartNumber, (void *)&dp_dfltvals.VpPartNumber },
{  	YzTdAutoDrvCtrl,		GP_UINT32,	4,		(void *)&dp_data.AutoDriveCtrl, (void *)&dp_dfltvals.AutoDriveCtrl },			
};																		

#endif		// End ifdef _DATAPOOL_C


#endif			/* end POOL_DEF_H */
