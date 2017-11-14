// The present software is not subject to the US Export Administration Regulations (no exportation license required), May 2012
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include "stdio.h"

#define IDT_ANTILATENCY				0
#define IDT_BSP						1
#define IDT_CONFIG_COM				2
#define IDT_SPRS232_HEX				3
#define IDT_SPRS232_INFO_PLUS		4 
#define IDT_SPRS232_ERR_DRV			5
#define IDT_SPRS232_ERR_SP_RS232	6
#define IDT_RELEASE					7
#define	IDT_PKG_DATA				8					
#define	TRACE_NB_FILE				9

#ifdef __cplusplus
extern "C"
{
#endif

void
InitLogTrace(int Idt, TCHAR * i_szFileName);

void 
CloseLogTrace(int Idt);

void
LogTrace(int Idt, const TCHAR * i_szFormat , ...);

void
LogTraceNoTime(int Idt, const TCHAR * i_szFormat , ...);

#ifdef _DEBUG	
void
LogFile(TCHAR * i_szFileName,TCHAR * i_p_Data,unsigned long i_n_DataSize);
#endif

#ifdef __cplusplus
}
#endif

#endif
