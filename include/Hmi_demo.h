/**************************************************************************************/
/*!
*    \file    	Hmi_demo.h
*
*    \copyright Yazaki 2016	- 2018
*
*    \brief Header file containing the public interfaces and data HMI demo function.
*
*    \author   D. Kageff
*
*    Modification history:
*    $Log: Hmi_demo.h  $
*    Revision 1.2 2018/02/13 10:32:45CST Daniel Kageff (10011932) 
*    Added conditionally compiled HMI dev mode controls simulation code
*    Revision 1.1 2016/10/13 13:40:44EDT Daniel Kageff (10011932) 
*    Initial revision
*    Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Application/Datapool_mgr/project.pj
*
*/
/***************************************************************************************/

#ifndef HMI_DEMO_H
#define HMI_DEMO_H

void HmiDemoStart(void);
void HmiDemoStep(void);
void HmiDemoSimDevCtrls(void);

#endif
