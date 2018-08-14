/**************************************************************************************/
/*!
 *  \file		App_cfg.c
 *
 *  \copyright	Yazaki 2016-17
 *
 *  \brief		Application configuration management functions.  This software component
 *				reads the application configuration INI format file and parses the 
 *				entries so that they can be used by the FF HUD application.
 *
 *  \author		D. Kageff
 *
 *  \version	$Revision: 1.10 $  
 *				$Log: App_cfg.c  $
 *				Revision 1.10 2017/07/27 17:12:25CDT Daniel Kageff (10011932) 
 *				Added single precision float key value handler
 *				Revision 1.9 2017/04/24 19:09:05EDT Daniel Kageff (10011932) 
 *				String key value handler uses key value max size for copy.
 *				Revision 1.8 2017/03/24 11:07:25EDT Daniel Kageff (10011932) 
 *				Changes to make the configuration file processor more reusable
 *				Revision 1.7 2017/02/16 10:25:26EST Daniel Kageff (10011932) 
 *				Added support for new HMI config option parameters
 *				Revision 1.6 2017/02/09 15:20:03EST Daniel Kageff (10011932) 
 *				Fixed build error
 *				Revision 1.5 2017/02/09 13:46:55EST Daniel Kageff (10011932) 
 *				Changed config file processing to table-driven architecture
 *
 ***************************************************************************************
 * \page sw_component_overview Software Component Overview page
 *	This optional comment section can be used to create an overview page for this 
 *	software component that is accessable from the navigation pane of the gnerated 
 *	design document.
 *
 *  TODO LIST:
 */
/***************************************************************************************/
#define _APP_CFG_C		/*!< File label definition */

#define APPCFG_TBL_VER		// Enable table-driven verison

/***********************************
		   INCLUDE FILES
***********************************/
#include <INTEGRITY.h>
#include <stdlib.h>
#include <stdio.h>

#include "gp_cfg.h"         // Common GP program configuration settings
#include "gp_types.h"       // Common GP program data type definitions
#include "gp_utils.h"       // Common GP program utility functions
#include "ini.h"			// INI format file reader/processor.

#include "App_cfg.h"		// App configuration managment functions.


/***********************************
	Private Macros and Typedefs
***********************************/


/***********************************
	Private Data and Structures
***********************************/

/*! Pointer to application configuration key processing table */
static const KeyProcEntry_t *p_keyproc_tbl;

static int num_keys = 0;		/*!< Number of entries in the key processing table */

/*! App configuration processing initialization function */
gp_retcode_t AppCfgInit(const KeyProcEntry_t *p_keytbl);



/***********************************
	Private Function Prototypes
***********************************/
static int handler_app(void* user, const char* section, const char* name,
                   const char* value);


/************ Start of code ******************/
/*! \defgroup appcfg_public Application configuration management public API
 */

/**************************************************************************************/
/*! \fn AppCfgRead(APP_CFG_CTRL_T *p_cfg)
 *
 *	\param[in] p_cfg   -  Pointer to configuration control structure.
 *
 *  \par Description:	  
 *   This function processes the INI file passed to it and fills out the data storage
 *	 pointed by the configuration control table entries of type ::KeyProcEntry_t. 
 *
 *  \returns Return code of type ::gp_retcode_t
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appcfg_public
 **************************************************************************************/
gp_retcode_t AppCfgRead(APP_CFG_CTRL_T *p_cfg)
{
	/* Save key processing table information */
	if(p_cfg->p_keytbl != NULL)
	{
		p_keyproc_tbl = p_cfg->p_keytbl;
		num_keys = p_cfg->num_keys;

		/* Process the configuration file */
	    if (ini_parse(p_cfg->cfgfile, handler_app, p_cfg) < 0) 
	    {
	        printf("Can't load '%s'\n", p_cfg->cfgfile);
	        return GP_INIT_ERR;
	    }
	    printf("Config loaded from %s\n", p_cfg->cfgfile);
        return GP_SUCCESS;
	}
	else
	{
		return GP_GENERR;	// error
	}
}


/**************************************************************************************/
/*	  Configuration file parsing key value handler.  Called by the INI parser.		  */
/**************************************************************************************/

		
/**************************************************************************************/
/*! \fn handler_app(void* user, const char* section, const char* name, const char* value)
 *
 *	\param[in] user  - 	Pointer to section/key data structure
 *	\param[in] section  -  Pointer to INI file section name string.
 *	\param[in] name  - 	Pointer to INI file section key name string.
 *	\param[in] value -  Pointer to INI file key value string. 
 *
 *  \par Description:	  
 *   This function processes the read INI file sections and keys and stores them if they
 *	 are valid
 *
 *  \retval	0 == Error, 1 == Success
 *
 *  \par Limitations/Caveats:
 *
 **************************************************************************************/
static int handler_app(void* user, const char* section, const char* name,
                   	   const char* value)
{
	int key_idx;
	
	/* Scan through key processing table */
	for(key_idx=0; key_idx < num_keys; key_idx++)
	{
		/* If not at the end of the table and the section/key is found */
		if(	  (p_keyproc_tbl[key_idx].handler != NULL)
		   && (strcmp(section, p_keyproc_tbl[key_idx].sect_name) == 0)
		   && (strcmp(name, p_keyproc_tbl[key_idx].key_name) == 0) )
		{
			/* Call the key value handler */
			p_keyproc_tbl[key_idx].handler(&(p_keyproc_tbl[key_idx]), value);
			break;
		}
	}
	/* If the key was found return 'success' */
	if(key_idx < num_keys)
	{
		return 1;		// success
	}
	else
	{
		return 0;		// key not found 
	}			
}


/**************************************************************************************/
/*! \fn AppCfgStoreInt(const KeyProcEntry_t *p_entry, const char *value)
 *
 *	\param[in] p_entry  - Pointer to section/key processing table entry.
 *	\param[in] value	- Pointer to key value string
 *
 *  \par Description:	  
 *   Convert and store key value as an integer.
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appcfg_public
 **************************************************************************************/
void AppCfgStoreInt(const KeyProcEntry_t *p_entry, const char *value)
{
	*((int *)p_entry->p_dest) = atoi(value);
}

/**************************************************************************************/
/*! \fn AppCfgStoreStr(const KeyProcEntry_t *p_entry, const char *value)
 *
 *	\param[in] p_entry  - Pointer to section/key processing table entry.
 *	\param[in] value	- Pointer to key value string
 *
 *  \par Description:	  
 *   Store key value as a string.
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appcfg_public
 **************************************************************************************/
void AppCfgStoreStr(const KeyProcEntry_t *p_entry, const char *value)
{
	strlcpy(p_entry->p_dest, value, p_entry->size);
}


/**************************************************************************************/
/*! \fn AppCfgStoreBool(const KeyProcEntry_t *p_entry, const char *value)
 *
 *	\param[in] p_entry  - Pointer to section/key processing table entry.
 *	\param[in] value	- Pointer to key value string
 *
 *  \par Description:	  
 *   Convert and store key value as a true/false boolean.
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appld_public
 **************************************************************************************/
void AppCfgStoreBool(const KeyProcEntry_t *p_entry, const char *value)
{
	*(bool *)p_entry->p_dest = false;
	if(strcmp("true", value) == 0)
	{
		*(bool *)p_entry->p_dest = true;
	}
}

/**************************************************************************************/
/*! \fn AppCfgStoreHex(const KeyProcEntry_t *p_entry, const char *value)
 *
 *	\param[in] p_entry  - Pointer to section/key processing table entry.
 *	\param[in] value	- Pointer to key value string
 *
 *  \par Description:	  
 *   Convert and store key value as a hex value.
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appcfg_public
 **************************************************************************************/
void AppCfgStoreHex(const KeyProcEntry_t *p_entry, const char *value)
{
	*(uint32_t *)p_entry->p_dest = (uint32_t)strtol(value, NULL, 0);		// Convert hex format number
}

/**************************************************************************************/
/*! \fn AppCfgStoreFloat(const KeyProcEntry_t *p_entry, const char *value)
 *
 *	\param[in] p_entry  - Pointer to section/key processing table entry.
 *	\param[in] value	- Pointer to key value string
 *
 *  \par Description:	  
 *   Convert and store key value as a single precision float value.
 *
 *  \retval	none
 *
 *  \par Limitations/Caveats:
 *
 *  \ingroup appcfg_public
 **************************************************************************************/
void AppCfgStoreFloat(const KeyProcEntry_t *p_entry, const char *value)
{
	*(float *)p_entry->p_dest = (float)atof(value);		// Convert floating pt format number
}


