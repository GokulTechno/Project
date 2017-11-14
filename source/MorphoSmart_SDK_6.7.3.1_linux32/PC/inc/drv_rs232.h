// The present software is not subject to the US Export Administration Regulations (no exportation license required), May 2012
#ifndef _DRV_RS232_H
#define _DRV_RS232_H

#include "MSO_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

//+---------------------------------------------------------------------------
//
//  Function:   RS232_Initialize
//
//  Synopsis:	This function open the Port Com	and	set : 
//															DCB
//															COMMTIMEOUTS
//									
//
//  Arguments:	C *		i_pc_strName : Port name
//				UC		i_uc_XonValue : Xon value : 0x11
//				UC		i_uc_XoffValue : Xoff value :0x13
//				DWORD	i_dw_BaudRate : 
//										9600 bauds		: CBR_9600
//										115200 bauds	: CBR_115200
//
//
//	Returned value :	RS232_OK
//						RS232ERR_ERROR
//
//  History:    05-04_2001	Cwynar Cédric		Created
//				25-06-2001	Geslin Alexandre	Modified
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Initialize(
						C *		i_pc_strName,
						UC		i_uc_XonValue, 
						UC		i_uc_XoffValue,
						DWORD	i_dw_BaudRate);

//+---------------------------------------------------------------------------
//
//  Function:   RS232_SetBaudRate
//
//  Synopsis:	This function open set : 	DCB
//											COMMTIMEOUTS
//									
//
//
//  Arguments:	UC		i_uc_XonValue : Xon value : 0x11
//				UC		i_uc_XoffValue : Xoff value :0x13
//				DWORD	i_dw_BaudRate : 9600 bauds		: CBR_9600
//										.........
//										115200 bauds	: CBR_115200
//
//
//	Returned value :	RS232_OK
//						RS232ERR_ERROR
//
//  History:    20-07-2001	Geslin Alexandre	Created
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_SetBaudRate(
						UC		i_uc_XonValue, 
						UC		i_uc_XoffValue,
						DWORD	i_dw_BaudRate);

//+---------------------------------------------------------------------------
//
//  Function:   RS232_Close
//
//  Synopsis:	This function close the Port Com
//									
//
//  Arguments:	Nothing
//
//	Returned value :	RS232_OK
//						RS232ERR_ERROR
//
//  History:    05-04_2001	Cwynar Cédric		Created
//				
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Close(void);


//+---------------------------------------------------------------------------
//
//  Function:   RS232_Write
//
//  Synopsis:	This function set the write timeouts and write on the Port Com
//							
//
//  Arguments: UC *i_puc_Buf : the data to write
//			   UL i_ul_Size : Number of byte to write
//
//
//	Returned value :	RS232_OK
//						RS232ERR_INIT
//						RS232ERR_IO_PENDING
//						RS232ERR_IO_INCOMPLETE
//						RS232ERR_TIMEOUT
//						RS232ERR_ERROR
//
//  History:    05-04_2001	Cwynar Cédric		Created
//				25-06-2001	Geslin Alexandre	Modified
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Write(
				   UC *i_puc_Buf,
				   UL i_ul_Size,
				   UL *o_pul_BytesWritten);


//+---------------------------------------------------------------------------
//
//  Function:   RS232_Read
//
//  Synopsis:	This function set the read timeouts and read on the Port Com
//							
//
//  Arguments: UC *i_puc_Buf : output of the data read
//			   UL i_ul_Size : Number of byte to read
//			   DWORD  i_dw_ReadTotalTimeoutConstant
//
//
//	Returned value :	RS232_OK
//						RS232ERR_INIT
//						RS232ERR_IO_PENDING
//						RS232ERR_IO_INCOMPLETE
//						RS232ERR_TIMEOUT
//						RS232ERR_ERROR
//
//  History:    05-04_2001	Cwynar Cédric		Created
//				25-06-2001	Geslin Alexandre	Modified
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Read(
				  UC *i_puc_Buf,
				  UL i_ul_Size,
				  DWORD	i_dw_ReadTotalTimeoutConstant);

//+---------------------------------------------------------------------------
//
//  Function:   RS232_Break
//
//  Synopsis:	This function set a break state on the Port Com
//							
//
//  Arguments: UC *i_puc_Buf : output of the data read
//
//
//	Returned value :	RS232_OK
//						RS232ERR_INIT
//						RS232ERR_ERROR
//
//  History:    10-07_2001	Cousin Emmanuel		Created
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Break(UL i_ul_Time);

//+---------------------------------------------------------------------------
//
//  Function:   RS232_IsInitialize
//
//  Synopsis:	This function verify if the Port Com is initialize
//							
//
//  Arguments: Nothing
//
//
//	Returned value :	return s_bFlagInit
//
//
//  History:    05-04_2001	Cwynar Cédric		Created
//
//
//  Notes:		
//
//----------------------------------------------------------------------------
BOOLEAN RS232_IsInitialize(void);


//+---------------------------------------------------------------------------
//
//  Function:   RS232_ReturnHandlePort
//
//  Synopsis:	This function return the handle of the port
//							
//
//  Arguments: Nothing
//
//
//	Returned value :return s_h_Port
//
//
//  History:    05-04_2001	Cwynar Cédric		Created
//
//
//  Notes:		
//
//----------------------------------------------------------------------------
HANDLE RS232_ReturnHandlePort(void);

//+---------------------------------------------------------------------------
//
//  Function:  RS232_GetConfig 
//
//  Synopsis:	
//							
//
//  Arguments: Nothing
//
//
//	Returned value :
//
//
//  History:    22-11_2002	Alexandre GESLIN		Created
//
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_GetConfig(void);

//+---------------------------------------------------------------------------
//
//  Function:   RS232_Clear
//
//  Synopsis:	This function clears the Port Com buffers and errors,
//				interrupting all transmissions
//							
//
//  Arguments: none
//
//
//	Returned value :	RS232_OK
//						RS232ERR_ERROR
//
//  History:    22-03-2006	Nicolas MARC		Created
//
//  Notes:		
//
//----------------------------------------------------------------------------
I RS232_Clear(void);

#ifdef __cplusplus
}
#endif

#endif
