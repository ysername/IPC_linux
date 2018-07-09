/**************************************************************************************/
/*!
 *    \file     App_cfg.h
 *
 *    \copyright 2016-17
 *
 *    \brief    Header file containing the public interfaces and data for the application 
 *				configuration processing functions.
 *
 *    \author   D. Kageff
 *
 *    \version  $Revision: 1.8 $  
 *				$Log: App_cfg.h  $
 *				Revision 1.8 2017/07/27 17:12:25CDT Daniel Kageff (10011932) 
 *				Added single precision float key value handler
 *				Revision 1.7 2017/04/24 19:08:08EDT Daniel Kageff (10011932) 
 *				Added key value size/length field to key table entry structure
 *				Revision 1.6 2017/03/24 11:07:25EDT Daniel Kageff (10011932) 
 *				Changes to make the configuration file processor more reusable
 *				Revision 1.5 2017/02/15 19:01:10EST Daniel Kageff (10011932) 
 *				Added support for new HMI config option parameters
 *				Revision 1.4 2017/02/09 13:46:44EST Daniel Kageff (10011932) 
 *				Changed config file processing to table-driven architecture
 *
 */
/***************************************************************************************/
#ifndef _APP_CFG_H
#define _APP_CFG_H

/***********************************
	Public Macros and Typedefs
***********************************/
#define MAX_KEYNAME_LEN	30		/*!< Maximum key name string length */

/*! INI file key value processing structure declaration */
struct KEY_PROC_ENTRY;

/*! Config file key processing handler prototype definition. */ 
typedef void (*KeyvalHandler_t)(const struct KEY_PROC_ENTRY *p_entry, const char *value);

/* Config file key processing table entry definition. */
typedef struct KEY_PROC_ENTRY 
{
	char sect_name[MAX_KEYNAME_LEN];		/*!< Name of section */
	char key_name[MAX_KEYNAME_LEN];			/*!< Name of section key */
    KeyvalHandler_t handler;    			/*!< Handler service function */
	char *p_dest;							/*!< Location to store key value */
	int size;								/*!< Size of destination field */
} KeyProcEntry_t;


/*! General configuration parser control structure */
typedef struct
{
	char cfgfile[MAX_PATH_LEN];			/*!< Configuration filename */
	KeyProcEntry_t *p_keytbl;			/*!< Pointer to key processing table */
	int num_keys;						/*!< Number of keys in the table */
} APP_CFG_CTRL_T;

#define APP_CFG_DATA_SZ	sizeof(APP_CFG_CTRL_T)

/***********************************
		Public API Functions
***********************************/

/* Process application loading config sections */
gp_retcode_t AppCfgRead(APP_CFG_CTRL_T *p_cfg);

/* Key value handler function prototypes. */
void AppCfgStoreInt(const KeyProcEntry_t *p_entry, const char *value);		/*!< Store key value as integer */
void AppCfgStoreStr(const KeyProcEntry_t *p_entry, const char *value);		/*!< Store key value as string */
void AppCfgStoreBool(const KeyProcEntry_t *p_entry, const char *value);		/*!< Store key value as true/false boolean */
void AppCfgStoreHex(const KeyProcEntry_t *p_entry, const char *value);		/*!< Store key value as hex value */
void AppCfgStoreFloat(const KeyProcEntry_t *p_entry, const char *value);	/*!< Store key value as single precision float value */

#endif

