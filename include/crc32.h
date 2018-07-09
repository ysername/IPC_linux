/********************************************************************************************/
/*!
*  \file crc32.h
*
*  \brief Public interface for the CRC-32 calculation function.
*      	  This program generates the CRC-32 values for the data in the supplied buffer.
*      	  These are the same CRC-32 values used by GZIP, PKZIP, and ZMODEM.  
*
*      	  THIS CRC ALGORITHM IS PUBLIC-DOMAIN SOFTWARE.
*      	  Based on the byte-oriented implementation "File Verification Using CRC"
*      	  by Mark R. Nelson in Dr. Dobb's Journal, May 1992, pp. 64-67.

*  \author D. Kageff
*
*  Modification history:
*  \version $Revision: 1.1 $
*   $Log: crc32.h  $
*   Revision 1.1 2017/06/21 17:02:39CDT Daniel Kageff (10011932) 
*   Initial revision
*   Member added to project /Projects/Faraday Future/2018_FFHUD/Software Development/Eng/GP/Common/project.pj
*
********************************************************************************************/
#ifndef CRC32_H
#define CRC32_H

/******************************************************************************/
/*                      I N C L U D E   F I L E S                             */
/******************************************************************************/


/******************************************************************************/
/*                 F U N C T I O N   P R O T O T Y P E S                      */
/******************************************************************************/

/******************************************************************************/
/*  \fn CalcCrc32(uint32_t in_crc, uint8_t *p_buf, int offset, int bufLen)
 *
 *  \par Description
 *  Calculate a CRC32 value over the supplied buffer data.
 *
 *	\param[in] in_crc - Starting CRC32 value 
 *	\param[in] buf	  - Buffer that holds the data to calculate the CRC over.
 *	\param[in] bufLen - Number of bytes to calculate CRC for.
 *
 *  \returns Calculated CRC-32 value. 
 *
 *  \par Limitations/Caveats:
 *
 *******************************************************************************/
uint32_t CalcCrc32(uint32_t in_crc, uint8_t *p_buf, int offset, int bufLen);


#endif
