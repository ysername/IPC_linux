#ifndef _IDENTIFICATION_DATA_H_
#define _IDENTIFICATION_DATA_H_

// /*C O N N E C T I O N S 	P A T H S*/
// const char HmiMgrDpMgr_ConPath[] = "./hmiMgr_to_dataPoolMgr_connection_path";
// const char UnitMgrDpMgr_ConPath[] ="./UnitMgr_to_dataPoolMgr_connection_path";
// const char UnitMgrDpMgr_ConPath2[] ="./UnitMgr_to_dataPoolMgr_connection_path2";
// const char EMgrDpMgr_ConPath[] = "./EMgr_to_dataPoolMgr_connection_path";
// const char IMgrDpMgr_ConPath[] = "./IMgr_to_dataPoolMgr_connection_path";

// const char UnitMgrHmiMgr_ConPath[] = "./UnitMgr_to_HmiMgr_connection_path";
// const char CommonDp_ConPath[] = "./common_Dp";
// const char CommonHmi_ConPath[] = "./common_Hmi";
// /*const char server2client1_connection_path[] = "./server_to_client1";
// const char server2client2_connection_path[] = "./server_to_client2";*/

// /*N A M E D 	S E M A P H O R E S*/
// const char dp_semaphore[] = "/dp_semaphore";
// const char hmi_semaphore[] = "/hmi_semaphore";
// const char hmi_semaphore2[] = "/hmi_semaphore2";
// const char example_semaphore[] = "/example_semaphore";


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
/*const char server2client1_connection_path[] = "./server_to_client1"
const char server2client2_connection_path[] = "./server_to_client2"*/

/*N A M E D 	S E M A P H O R E S*/
#define dp_semaphore "/dp_semaphore"
#define hmi_semaphore "/hmi_semaphore"
#define hmi_semaphore2 "/hmi_semaphore2"
#define example_semaphore "/example_semaphore"



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

typedef enum{
	MAIN_COMPONENT = 0,
	SUBCOMPONENT1,
	SUBCOMPONENT2,
	SUBCOMPONENT3,
	MAX_NUM_COMPONENTS
}component_type_t;
#endif