/**
	@file identification_data.h
	@version 1.0
	@date 8/8/2018
	@author Leonardo Reatiga Miranda
	@brief 	This file is intended to be used alonside with the msg_api.h file
			in order to maintain proper interaction beetwen processes.
			In this file an "id" of the processes are defined inside the enum
			component_id_t, and a connections paths are defined as well.

*/
#ifndef _IDENTIFICATION_DATA_H_
#define _IDENTIFICATION_DATA_H_

/*C O N N E C T I O N S 	P A T H S*/
#define HmiMgrDpMgr_ConPath "./hmiMgr_to_dataPoolMgr_connection_path"
#define HmiMgrWrkTsk1DpMgr_ConPath "./hmiMgrWrkTsk1_to_dataPoolMgr_connection_path"
#define UnitMgrDpMgr_ConPath "./UnitMgr_to_dataPoolMgr_connection_path"
#define UnitMgrDpMgr_ConPath2 "./UnitMgr_to_dataPoolMgr_connection_path2"
#define EMgrDpMgr_ConPath "./EMgr_to_dataPoolMgr_connection_path"
#define IMgrDpMgr_ConPath "./IMgr_to_dataPoolMgr_connection_path"

#define UnitMgrHmiMgr_ConPath "./UnitMgr_to_HmiMgr_connection_path"
#define CommonDp_ConPath "./common_Dp"
#define CommonHmi_ConPath "./common_Hmi"
#define CommonHmi2_ConPath "./common_Hmi2"

/*N A M E D 	S E M A P H O R E S*/
#define dp_semaphore "/dp_semaphore"
#define hmi_semaphore "/hmi_semaphore"
#define hmi_semaphore2 "/hmi_semaphore2"
#define example_semaphore "/example_semaphore"


/**
	@brief enum to hold an "id" of the processes we want to interact with
*/
typedef enum{
	IPC_MGR_AS = 0,
	IPC_MGR_WRKTSK1,
	IPC_MGR_WRKTSK2,
	HMI_MGR_AS,
	HMI_MGR_WRKTSK1,
	DP_MGR_AS,
	U_MGR_AS,
	TOTAL_COMPONENTS
}component_id_t;

#endif