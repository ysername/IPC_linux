/**************************************************************************************/
/*!
 *  \file		Hmi_demo.c
 *
 *  \copyright	Yazaki 2016	- 2017
 *
 *  \brief		HMI demo functions.  This software component initializes and starts
 *				the task that generates simulated vehicle parameters used by the HMI.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.8 $  
 *				$Log: Hmi_demo.c  $
 *				Revision 1.8 2018/02/13 10:32:45CST Daniel Kageff (10011932) 
 *				Added conditionally compiled HMI dev mode controls simulation code
 *				Revision 1.7 2017/07/26 13:13:19EDT Daniel Kageff (10011932) 
 *				Moved time conversion definitions to gp_types.h
 *				Revision 1.6 2017/06/26 13:12:56EDT Daniel Kageff (10011932) 
 *				Update Doxygen SW component comment block
 *				Revision 1.5 2017/02/22 16:14:04EST Daniel Kageff (10011932) 
 *				Changes to work with redesigned datapool management functions and tables
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	The <b>HMI demo</b> software component is repsonsible for starting and running the vehicle
 *	data simulator in the GP if it is enabled in app_start.ini.  The top level functions
 *	are implemented in Hmi_demo.c.
 *
 */
/***************************************************************************************/
#define _HMI_DEMO_C		/*!< File label definition */

/***********************************
		   INCLUDE FILES
***********************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "gp_cfg.h"         // Common GP program configuration settings
#include "gp_types.h"       // Common GP program data type definitions
#include "gp_utils.h"       // Common GP program utility functions

#include "msg_buf.h"
#include "msg_api_signals.h"
#include "msg_def.h"
#include "pool_def.h"		// Global datapool definitions.
#include "Datapool.h"

//#include "..\DataSimulator\vdatasim.h"
#include "Hmi_demo.h"

#ifdef HMI_DEV_SIM
 #include "..\..\HMI\HMI_common.h"				// Global HMI definitions
 #include "..\..\HMI\HMILink\TestPatternLink.h"
#endif

/***********************************
	 Private Macros
***********************************/
#define DFLT_TEST_PATTERN	1		/*!< Default test pattern ID to display */

/* New Vehicle simulator definitions */
#define PFC_FILE CFG_DIR VSIM_CFG_FNAME		/*!< Path and filename for vehicle data simulator configuration. */

#define VSIM_TICK_MS	100ul		/*!< Msec per vehicle simulator tick */

/***********************************
	Private Data and Structures
***********************************/
#if 0
/*! HMI demo task TCB definition */
Task demo_task;


/***********************************
	Private Functions
***********************************/

void HmiDemoTask(void);

#ifdef HMI_DEV_SIM
 static void CheckForHmiDevMode(void);
#endif
	
/************ Start of code ******************/
/*! \defgroup hmi_demo_public_api GP application HMI demo public API
 */
#endif
/**************************************************************************************/
/*! \fn HmiDemoStart()
 *
 *	param - No parameters
 *
 *  \par Description:	  
 *   Startup function for the HMI demo/vehicle data simulator component that updates the
 *	 datapool.  
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 *	\ingroup hmi_demo_public_api
 **************************************************************************************/

#define Success 0
void HmiDemoStart(void)//needs changes unknow level
{
	/*// Initialize vehicle data simulator
	if(InitVdataSim(PFC_FILE, VSIM_TICK_MS) == 0)
	{
		//int32_t err = CommonCreateTask(1, (Address)HmiDemoTask, 0x1000, "HmiDemotask", &demo_task);//replace
		int32_t err =0;
		if(err != Success)
		{
			gp_Printf(VRB_DEBUG1, "Error creating the simulator task\n");
		}
		else
		{
			//RunTask(demo_task);
        }
	}
	/* Could not start the simulator */
	/*else
	{
		gp_Printf(VRB_DEBUG1, "Vehicle data simulator not started\n");
	}*/
}
#if 0

/**************************************************************************************/
/*! \fn HmiDemoTask(void)
 *
 *	param - No parameters
 *
 *  \par Description:	  
 *   Demo task that runs the vehicle simulator periodic function every VSIM_TICK_MS.
 *
 *  \retval	None
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void HmiDemoTask(void)
{
	 struct timeval start_tod;
	 struct timeval stop_tod;
	 struct timeval diff_tod;
	 long usec;

	for(;;)
	{
		/* Get the start time, run a simulation step, then get the stop time */
		gettimeofday(&start_tod, NULL);
		VdataSimTick();
		gettimeofday(&stop_tod, NULL);

		/* Calculate the time to run the step and subtract it from the tick period */
		gp_TimerSub(&stop_tod, &start_tod, &diff_tod);
		usec = (diff_tod.tv_sec * USEC_PER_SEC) + diff_tod.tv_usec;
		usec = (VSIM_TICK_MS * USEC_PER_MSEC) - usec;
		if(usec > 0)
		{
			usleep(usec);
		}
		else
		{
			gp_Printf(VRB_DEBUG2, "HmiDemoTask: Tick OVERFLOW!!!\n");
		}
	}
}


/********************* HMI dev mode control simulator code ************************/
#ifdef HMI_DEV_SIM

#define MODE_CHG_CNT  20
#define MODE_UPD_MSEC  2000
#define RST_FLAGS_CNT	2
static int hmi_dev_chkctr = 50;		/*!< Number of times to check for the HMI dev mode */
static int hmi_dev_mode = -1;		/*!< HMI dev mode state: 0 = off, 1 == on, -1 = uninitialized */
static uint32_t hdev_ctrls = 0;		/*!< HMI dev mode user play controls */
static int hdev_ctr = 0;

/**************************************************************************************/
/*! \fn CheckForHmiDevMode(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *  Determine if the HMI development mode is active and set the hmi_dev_mode flag if
 *	it is.
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
static void CheckForHmiDevMode(void)
{
	uint32_t TestPatternValue;

	/* If the HMI dev mode flag is uninitialized then check it */
	if(hmi_dev_chkctr > 0)
	{
		/* Check if the HMI development mode is active */
		GetElem(YzTdTestPattern,&TestPatternValue);
		if(DEV_TST_PATT_ID == TestPatternValue)
		{
			hmi_dev_mode = 1;
			hmi_dev_chkctr = 0;
		}
		else
		{
			hmi_dev_mode = 0;
		}
		hmi_dev_chkctr--;
	}
}


/**************************************************************************************/
/*! \fn HmiDemoSimDevCtrls(void)
 *
 *	param[in] None
 *
 *  \par Description:	  
 *  Simulates the play controls for the HMI development mode. Modifies the HMI 
 *	development mode controls datapool item. 
 *
 *  returns none
 *
 *  \par Limitations/Caveats:
 *	 None
 *
 **************************************************************************************/
void HmiDemoSimDevCtrls(void)
{
	static int mode_ctr = MODE_CHG_CNT;
	static int active_mode = 0;
	static int last_mode = 0;
	static int rst_flags = false;
	static bool update_ctrls = false;

	CheckForHmiDevMode();
	if(hmi_dev_mode == 1)
	{
		/* Need to reset step up/down and reset flags */
		if(rst_flags > 0)
		{
			rst_flags--;
			if(rst_flags <= 0)
			{
				hdev_ctrls &= ~((uint32_t)(HMIDEV_CTRL_STEPUP|HMIDEV_CTRL_STEPDN|HMIDEV_CTRL_RST));
				update_ctrls = true;
			}
		}
		/* If time to run simulation */
		if(hdev_ctr <= 0)
		{
			bool reset_hctr = true;

			update_ctrls = true;

			/* If time to change the mode */
			mode_ctr--;
			if(mode_ctr <= 0)
			{
				mode_ctr = MODE_CHG_CNT;
				active_mode++;
				if(active_mode > 2)
				{
					active_mode = 0;
				}
			}
			/* Set the mode controls */
			switch(active_mode)
			{
				case 0:				// Step up
					gp_Printf(VRB_DEBUG2, "HMI dev sim: Step up\n");
					hdev_ctrls = HMIDEV_CTRL_STEP | HMIDEV_CTRL_STEPUP;
					rst_flags = RST_FLAGS_CNT;
					break;
				case 1:				// Step down
					gp_Printf(VRB_DEBUG2, "HMI dev sim: Step down\n");
					hdev_ctrls = HMIDEV_CTRL_STEP | HMIDEV_CTRL_STEPDN;
					rst_flags = RST_FLAGS_CNT;
					break;
				case 2:				// Play
					if(active_mode != last_mode)		// only print the 1st time
					{
						gp_Printf(VRB_DEBUG2, "HMI dev sim: Play\n");
					}
					hdev_ctrls = HMIDEV_CTRL_PLAY;
					break;
				case 3:				// Reset frame counter
					gp_Printf(VRB_DEBUG2, "HMI dev sim: Reset frame cntr\n");
					hdev_ctrls = HMIDEV_CTRL_RST;
					rst_flags = RST_FLAGS_CNT;
					reset_hctr = false;				// this will cause the SM to move to the next mode immediately
					break;
				default:
					break;
			}
			/* Reset the update counter */
			if(reset_hctr == true)
			{
				hdev_ctr = (MODE_UPD_MSEC / HMI_FRAME_TIME);		// reset sim counter
			}	 	
		}
		/* Else decr counter */
		else
		{
			hdev_ctr--;
		}
		/* If updating the controls in the HMI */
		if(update_ctrls == true)
		{
			/* Update the dev mode */
			SetElem(YzTdHmiDevCtrls, (void *)&hdev_ctrls);		// put a breakpoint here to change the controls
			last_mode = active_mode;
			update_ctrls = false;
		}
	}
}
#else
/* Dummy function */
void HmiDemoSimDevCtrls(void)
{
}
#endif

#endif