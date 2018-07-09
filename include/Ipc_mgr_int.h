//TODO: The function ConectionObjectNumber is defined 
//		as part of the INTEGRITY api (object_inlines.h)
// for that reason is being commented, form the kernel 
// guide, page 140:
//'ConnectionObjectNumber() takes an ObjectIndex and converts 
// it into the corresponding Connection.', that's why the 
//#define constants were leaved as uint.
/*
#define ImAs_UmAsCon1	ConnectionObjectNumber(10U)
#define ImAs_DmAsCon1	ConnectionObjectNumber(11U)
#define ImAs_HmCon1	ConnectionObjectNumber(12U)
#define ImAs_ReqTsk	TaskObjectNumber(13U)
#define Hm_ImAsCon1	ConnectionObjectNumber(14U)
#define ImAs_UmAsCon2	ConnectionObjectNumber(15U)
#define ImAs_UmAsCon4	ConnectionObjectNumber(16U)
#define ImAs_PmAsCon1	ConnectionObjectNumber(17U)
#define ImAs_WrkTsk2	TaskObjectNumber(18U)
#define ImAs_IntTskCon	ConnectionObjectNumber(19U)
#define ImAs_WrkTsk2Con	ConnectionObjectNumber(20U)
#define ImAs_UmAsCon3	ConnectionObjectNumber(22U)
*/


#define ImAs_UmAsCon1 	10U
#define ImAs_DmAsCon1	11U
#define ImAs_HmCon1		12U
#define ImAs_ReqTsk		13U
#define Hm_ImAsCon1		14U
#define ImAs_UmAsCon2	15U
#define ImAs_UmAsCon4	16U
#define ImAs_PmAsCon1	17U
#define ImAs_WrkTsk2	18U
#define ImAs_IntTskCon	19U
#define ImAs_WrkTsk2Con	20U
#define ImAs_UmAsCon3	22U
