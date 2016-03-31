///////////////////////////////////////////////////////////////////
// i5comp.h
//  
// InstallShield Compression and Maintenance util
// fOSSiL - 1999
//
// *Any use is authorized, granted the proper credit is given*
//
// No support will be provided for this code
//

#ifndef _I5COMP_H
#define _I5COMP_H

typedef struct tagRWHANDLES {
	HANDLE hRead;
	HANDLE hWrite;
	DWORD BytesIn;
	DWORD BytesOut;
} RWHANDLES, *LPRWHANDLES;

typedef struct tagCABFILELIST {
	DWORD CabIndex;
	struct tagCABFILELIST* pNext;
	char FileName[];
} CABFILELIST, *LPCABFILELIST;

typedef struct tagDISKFILELIST {
	struct tagDISKFILELIST* pNext;
	DWORD CabDirInd;
	LPSTR DiskDir;
	LPSTR CabDir;
	char FileName[];
} DISKFILELIST, *LPDISKFILELIST;

typedef struct tagDIRARRAY {
	DWORD Count;
	LPSTR Dirs[];
} DIRARRAY, *LPDIRARRAY;

typedef enum {
	ver50 = 0,
	ver51,
	ver55
} ISVersion;

#endif // _I5COMP_H
