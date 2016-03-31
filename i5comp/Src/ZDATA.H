///////////////////////////////////////////////////////////////////
// zdata.h
//  
// ZData definitions
//
// InstallShield Compression and Maintenance util
// fOSSiL - 1999
//
// *Any use is authorized, granted the proper credit is given*
//
// No support will be provided for this code
//

#ifndef ZDATA_INC
#define ZDATA_INC

#include <windef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagZCOMPRESSBUF {
	DWORD Ptr;
	WORD Len;
} ZCOMPRESSBUF, *LPZCOMPRESSBUF;

// ZDataSetup(2, 0xfa372dea, 0)
extern int (FAR WINAPI* ZDataSetup)(DWORD, DWORD, DWORD);
extern int (FAR WINAPI* ZDataUnSetup)();
// ZDataSetInfo(INFO_func, Param)
extern int (FAR WINAPI* ZDataSetInfo)(DWORD, DWORD);
extern int (FAR WINAPI* ZDataGetLastError)();
// ZDataStart(SessionId)
extern int (FAR WINAPI* ZDataStart)(DWORD);
extern int (FAR WINAPI* ZDataEnd)();
// ZDataCompress
extern int (FAR WINAPI* ZDataCompress)(DWORD, LPZCOMPRESSBUF);
// ZDataDecompress(SessionId)
extern int (FAR WINAPI* ZDataDecompress)(DWORD);

#define INFO_BUFFER_SIZE	1	// Param = buffer size
#define INFO_CALLBACK_PARAM	3	// Param = param passed to callback function
#define INFO_READ_CALLBACK	4	// Param = addr of Read function
#define INFO_WRITE_CALLBACK	5	// Param = addr of Write function
#define INFO_BUFFER_PTR		6	// Param = addr of buffer

#define STD_DECOMP_BUFFER	0x7d00
#define STD_COMP_BUFFER		0x2c00

//int ZDATACALLBACK (LPVOID pBuf, LPDWORD pBufLen, DWORD CallbackParam);

DWORD WINAPI ZDataInit(DWORD Version);
void WINAPI ZDataDeinit();

#ifdef __cplusplus
}
#endif

#endif // ZDATA_INC