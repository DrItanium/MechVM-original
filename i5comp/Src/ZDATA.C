///////////////////////////////////////////////////////////////////
// zdata.c
//  
// InstallShield Compression and Maintenance util
// fOSSiL - 1999
//
// *Any use is authorized, granted the proper credit is given*
//
// No support will be provided for this code
//

#include <windows.h>
#include "zdata.h"

HINSTANCE hInstZ = NULL;

LPSTR funcnames[] = { "ZDataSetup",
					  "ZDataUnSetup",
					  "ZDataSetInfo",
					  "ZDataGetLastError",
					  "ZDataStart",
					  "ZDataEnd",
					  "ZDataCompress",
					  "ZDataDecompress",
					  NULL };

#pragma pack(4)
int (FAR WINAPI* ZDataSetup)() = NULL;
int (FAR WINAPI* ZDataUnSetup)() = NULL;
int (FAR WINAPI* ZDataSetInfo)() = NULL;
int (FAR WINAPI* ZDataGetLastError)() = NULL;
int (FAR WINAPI* ZDataStart)() = NULL;
int (FAR WINAPI* ZDataEnd)() = NULL;
int (FAR WINAPI* ZDataCompress)() = NULL;
int (FAR WINAPI* ZDataDecompress)() = NULL;
#pragma pack()

DWORD WINAPI ZDataInit(DWORD Version)
{
	CHAR dllname[100] = "zd";
	LPSTR* pfuncname;
	FARPROC* pfunc = &ZDataSetup;

	
	ltoa(Version, dllname + 2, 10);
	strcat(dllname, ".dll");
	hInstZ = LoadLibrary(dllname);
	if (!hInstZ)
		return GetLastError();
	
	pfuncname = funcnames;
	while (*pfuncname) {
		*pfunc = GetProcAddress(hInstZ, *pfuncname);
		if (!*pfunc) {
			FreeLibrary(hInstZ);
			hInstZ = NULL;
			return GetLastError();
		}
		pfunc++;
		pfuncname++;
	}
	ZDataSetup(2, 0xfa372dea, 0);
	return 0;
}

void WINAPI ZDataDeinit()
{
	if (hInstZ) {
		ZDataUnSetup();
		FreeLibrary(hInstZ);
	}
	hInstZ = NULL;
}
