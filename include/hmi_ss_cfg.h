/*******************************************************************************
*  $Log: hmi_ss_cfg.h  $
*  Revision 1.4 2017/10/16 17:08:58CDT Daniel Kageff (10011932) 
*  Added support for autonomous mode HMI display
*  Revision 1.3 2017/10/02 11:41:20CDT Fernando Villarreal Garza (10011234) 
*  Update model to have a test pattern disable return
*  Revision 1.2 2016/11/23 11:05:47CST Fernando Villarreal Garza (10011234) 
*  changes for Hmi 2 Version
*  Revision 1.1 2016/09/26 10:27:58EDT Daniel Kageff (10011932) 
*  Initial revision
*  Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/HMI/HMILink/project.pj
*  Revision 1.38 2015/07/23 08:57:23CDT Jose Rodriguez Gonzalez (10012325) 
*  [ SP_ISSUE_ID_2917 ] Adding event for new DCT mode variable
*  Revision 1.37 2015/07/14 02:51:48KST Kevin Russo (10032877) 
*  Update for Sharepoint 2760
*  Revision 1.36 2015/06/26 13:14:16EDT Sergio Espinoza Lopez (10012599) 
*  [SP_ISSUE_ID_786] Partial. New priorities added.
*  Revision 1.35 2015/06/18 13:14:57CDT Roberto Flores Estrada (10014232) 
*  Added missing events
*  Revision 1.34 2015/06/12 15:31:58CDT Roberto Flores Estrada (10014232)
*  added DISP_ECALL_OFF and DISP_ECALL_ON
*  Revision 1.33 2015/06/12 07:58:20CDT Ryan Stanley (10013507)
*  [SP_ISSUE_1899] Wireless charging [SP_ISSUE_1121_AND_2428] Door animation issues
*  Revision 1.32 2015/06/03 09:51:55EDT Roberto Flores Estrada (10014232)
*  Added BSD Degrade OFF event
*  Revision 1.31 2015/05/27 09:49:48CDT Roberto Flores Estrada (10014232)
*  Added 4WD events
*  Revision 1.30 2015/05/20 04:24:10CDT Eric Sanchez Briones (10012885)
*  DBC Pop-Up implementation
*  Revision 1.29 2015/05/08 21:44:03KST Roberto Flores Estrada (10014232)
*  [ SP_ISSUE_ID_1884 ][ SP_ISSUE_ID_2371 ] BSD and ESC OFF relies on the mode variable,
*  if their conditions are off then the popup will be closed.
*
*  Moved event filter conditions from state machine to a callback and added a filter for
*  goodbye. No group 02 events will be queued during goodbye.
*  Removed IMS and Flexsteer events at hmi_ss_cfg.h
*  Revision 1.28 2015/05/07 12:19:30CDT Ryan Stanley (10013507)
*  [SP_ISSUE_2284] New mode LKAS Cancel added
*  Revision 1.27 2015/05/01 13:42:16EDT Kevin Russo (10032877)
*  Update for Sharepoint 2238
*  Revision 1.26 2015/04/16 19:14:18EDT Kevin Russo (10032877)
*  update for Sharepoint 2011, 2155
*  Revision 1.25 2015/04/15 19:01:37EDT Roberto Flores Estrada (10014232)
*  [ SP_ISSUE_ID_2036 ] Redesigned state machine G07
*  Revision 1.24 2015/04/01 11:36:52CST Roberto Flores Estrada (10014232)
*  [ SP_ISSUE_ID_1075 ] Cruise and Set may be displayed independently.
*  [ SP_ISSUE_ID_1730 ] No blank at goodbye when Speed limiter tab or cruise tab are visible at IGN ON
*  [ SP_ISSUE_ID_1814 ] No blinking between speed limiter tab and cruise tab
*  Revision 1.23 2015/03/06 14:50:07CST Jose Rodriguez Gonzalez (10012325)
*  SP_ISSUE_ID_1732. SP_ISSUE_ID_1864. Adding DISP_BSD_DEGRADE_ON
*  Revision 1.22 2015/02/19 18:12:55EST Sergio Espinoza Lopez (10012599)
*  add new TCF event definition
*  Revision 1.21 2015/02/05 13:10:07CST Jose Rodriguez Gonzalez (10012325)
*  Adding DISP_LIGHTS_ON, DISP_FRONT_WIPERS_ON, DISP_REAR_WIPERS_ON
*  Revision 1.20 2015/01/30 13:55:18CST Kevin Russo (10032877)
*  Add DISP_AEB_RADAR_FAIL_ON
*  Revision 1.19 2015/01/28 12:46:57EST Kevin Russo (10032877)
*  remerge code
*  Revision 1.18 2015/01/28 12:39:08EST Kevin Russo (10032877)
*  add Cruise and SpeedLimiter Events
*  Revision 1.17 2015/01/27 11:26:46EST Jose Rodriguez Gonzalez (10012325)
*  Adding DISP_DTE1KMMODE_ON
*  Revision 1.16 2015/01/26 13:12:22EST Kevin Russo (10032877)
*  add DISP_ACTIVEHOODFAIL_ON
*  Revision 1.15 2015/01/24 15:26:37EST Kevin Russo (10032877)
*  Add DISP_OK_5SEC
*  Revision 1.14 2015/01/22 10:08:04EST Jose Rodriguez Gonzalez (10012325)
*  SW Issue #825. Updating task from 25us to 30us
*  Revision 1.13 2015/01/08 10:30:01CST Jose Rodriguez Gonzalez (10012325)
*  Adding event DISP_RCTA_ON. SW Issue #631
*  Revision 1.12 2014/12/23 11:21:06CST Kevin Russo (10032877)
*  Add event for AEB Fail
*  Revision 1.11 2014/12/01 10:48:07EST Kevin Russo (10032877)
*  add eb_DisplayAEBCrashMode,
*      DISP_AEB_ON
*  remove DISP_BSD_OFF,
*         DISP_DCT_OFF,
*         DISP_DRHOODABOVE1KM_OFF,
*         DISP_EPB_OFF,
*         DISP_FCW_OFF,
*         DISP_ISG1_OFF,
*         DISP_ISG2_OFF,
*         DISP_LDWS_OFF,
*         DISP_LKASHANDSON_OFF,
*         DISP_PAS_OFF,
*         DISP_BSD_OFF,
*         DISP_SPAS_OFF,
*         DISP_HIGHTEMP_OFF
*  Revision 1.10 2014/10/14 15:35:30EDT Kevin Russo (10032877)
*  add events DISP_HIGHTEMP_OFF, DISP_HIGHTEMP_ON
*  Revision 1.9 2014/10/02 14:28:53EDT Kevin Russo (10032877)
*  Add three events
*  Revision 1.8 2014/09/25 11:39:18EDT Oleg Gaidoukevitch (10031157)
*  Disabled Frame Rate calculation
*  Revision 1.7 2014/07/30 22:20:25EDT Edgar Rios Lucas (10032297)
*  HMI Update
*  Revision 1.5 2014/07/23 16:10:29CDT Kevin Russo (10032877)
*  HMI release to application 07/23/14
*  Revision 1.4 2014/06/03 12:39:28CDT Edgar Rios (10032297)
*  Releasing HMI 1.0
*  Release notes and change analysis pending
*  Revision 1.3 2014/06/02 08:29:37CDT Kevin Russo (kevinadm)
*  Member moved from hmi_ss_cfg.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/Application/Include/project.pj to hmi_ss_cfg.h in project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/HMI/Link/project.pj.
*  Revision 1.2 2014/05/31 13:10:04EDT Kevin Russo (10032877)
*  compile fixes
*  Revision 1.1 2014/04/17 13:11:49EDT Gregory Palarski (10030324)
*  Initial revision
*  Member added to project /Projects/Hyundai/2016_KiaQLIPC/Software Development/Eng/Application/Include/project.pj
*
*                           (c) Copyright 2012
*                           Yazaki North America
*                           All Rights Reserved
*  ----------------------------------------------------------------------------
*
*   Unpublished, Yazaki North America Inc. All rights reserved.
*  This document is protected by the copyright law of the United States and
*  foreign countries.  The work embodied in this document is confidential and
*  constitutes a trade secret of Yazaki North America, Inc.  Any copying of
*  this document and any use or disclosure of this document or the work
*  embodied in it without the written authority of Yazaki North America, Inc.,
*  will constitute copyright and trade secret infringement and will violate the
*  rights of Yazaki North America, Inc.
*
*******************************************************************************/
/********************************************************************************************
*  File:  hmi_ss_cfg.h
*
*  Description: This is the configuration file for the HMI subsystem and contains parameters
*     whose values can be set to customize operation of the HMI SS.
********************************************************************************************/
#ifndef HMI_SS_CFG_H
#define HMI_SS_CFG_H

/*******************************************************************************************/
/*    I N C L U D E   F I L E S                                                            */
/*******************************************************************************************/


/*******************************************************************************************/
/*    M A C R O S                                                                          */
/*******************************************************************************************/

/*
**  HMI_SCHEDULING_RATE_MS - This value of this macro must be set to indicate the periodic
**     scheduling rate of the "HMISS_PeriodicTask" function.
*/
#define HMI_SCHEDULING_RATE_MS  (17u)	//58.8 FPS ~60FPS

/*
** HMI_ENABLE_CHRONOMETRICS - Macro that controls whether HMI chronometrics are captured
**    Set to 0 to DISable chronometrics
**    Set to non-0 to ENable chronometrics
*/
#define HMI_ENABLE_CHRONOMETRICS (0u)


/*
** HMI_USE_VS_STUBS - Defines if Visual State stub functions should be included.
**    0 = Do NOT include stubs
**    non-0 = Include stubs (for compiling/linking and testing without HMI model).
*/
#define HMI_USE_VS_STUBS (0)

/*
**  FRAME_RATE_INSTANT_SAMPLES   - The number of samples used to compute instanteous
**     frame rate.  A good value for this parameter is 1000/HMI_SCHEDULING_RATE_MS
**     (instanteous frame rate compute over samples taken in the last second).
**     The value range for this parameter is 1..1000.
*/
#define  FRAME_RATE_INSTANT_SAMPLES   (1000/HMI_SCHEDULING_RATE_MS)


/*
** HMI_ENABLE_FRAME_RATE_SUPPORT - Macro that controls whether HMI frame rate computation is supported.
**    Set to 0 to disable frame rate support
**    Set to non-0 to enable frame rate support
*/
#define HMI_ENABLE_FRAME_RATE_SUPPORT  (0)

/*******************************************************************************************/
/*    T Y P E S   A N D   E N U M E R A T I O N S                                          */
/*******************************************************************************************/

/*
** HMI_EVENT_TYPE - List of events that can be posted to the HMI event queue (as arguments
**   to the HMISS_PostHMIEvent function).
*/
typedef enum
{
	SEM_RESET,
	//TRIGERLESS_TRIGER,
	SCREEN_REDRAW,
	TEST_PATTERN_EVENT,
    TEST_PATTERN_DISABLE,
    AUTODRIVE_ACTIVE,
    AUTODRIVE_AVAILABLE,
    AUTODRIVE_NOT_AVAILABLE,
    MAX_EVENT_VALUE,                 /* Defines the maximum event value allowed */
} HMI_EVENT_TYPE;

/*******************************************************************************************/
/*    F U N C T I O N   P R O T O T Y P E S                                                */
/*******************************************************************************************/

#endif
/* End of file */
