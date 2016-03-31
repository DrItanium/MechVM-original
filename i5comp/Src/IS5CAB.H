///////////////////////////////////////////////////////////////////
// is5cab.h
//  
// InstallShield cabinet definitions
//
// InstallShield Compression and Maintenance util
// fOSSiL - 1999
//
// *Any use is authorized, granted the proper credit is given*
//
// No support will be provided for this code
//

#ifndef ISHIELD5_INC
#define ISHIELD5_INC

#pragma pack(1)
typedef struct tagCABHEADER {
	DWORD Signature;		// 0
	DWORD Version;			// 4
	BYTE NextVol;			// 5
	BYTE junk2;				// 6
	WORD junk3;				// 8
	DWORD ofsCabDesc;		// c
	DWORD cbCabDesc;		// 10
	DWORD ofsCompData;		// 14
	DWORD junk1;			// 18
	DWORD FirstFile;		// 1c
	DWORD LastFile;			// 20
	DWORD ofsFirstData;		// 24
	DWORD cbFirstExpanded;	// 28
	DWORD cbFirstHere;		// 2c
	DWORD ofsLastData;		// 30
	DWORD cbLastExpanded;	// 34
	DWORD cbLastHere;		// 38
} CABHEADER, *LPCABHEADER;	// size=3c

#define CAB_SIG		0x28635349

#define CFG_MAX		0x47

typedef struct tagCABDESC {
	DWORD ofsStrings;
	DWORD junk11;
	DWORD ofsCompList;
	DWORD ofsDFT;
	DWORD junk21;
	DWORD cbDFT;
	DWORD cbDFT2;
	DWORD cDirs;
	DWORD junk31, junk32;
	DWORD cFiles;
	DWORD junk41;
	WORD cCompTabInfo;
	DWORD ofsCompTab;
	DWORD junk51, junk52;
	DWORD ofsFileGroup[CFG_MAX];
	DWORD ofsComponent[CFG_MAX];
	DWORD ofsSTypes;
	DWORD ofsSTab;
} CABDESC, *LPCABDESC;

typedef DWORD* DFTABLE;

typedef struct tagFILEDESC {
	DWORD ofsName;
	DWORD DirIndex;
	WORD DescStatus;
	DWORD cbExpanded;
	DWORD cbCompressed;
	DWORD Attrs;
	DWORD FatDate;
	DWORD FatTime;
	DWORD junk1, junk2;
	DWORD ofsData;
} FILEDESC, *LPFILEDESC;

#define DESC_NEXT_VOLUME	1L
#define DESC_ENCRYPTED		2L
#define DESC_COMPRESSED		4L
#define DESC_INVALID		8L

typedef struct tagCOMPFILEENTRY {
	DWORD ofsName;		// 0
	DWORD ofsDesc;		// 4
	DWORD ofsNext;		// 8, size=C
} COMPONENTENTRY, *LPCOMPONENTENTRY, FILEGROUPENTRY, *LPFILEGROUPENTRY;

typedef DWORD* COMPONENTTABLE;
typedef DWORD* FILEGROUPTABLE;

typedef struct tagCOMPONENTDESC {
	DWORD ofsName;						// 0
	DWORD junk11, junk12;				// 4
	DWORD ofsDescription;				// c
	DWORD ofsStatusText;				// 10
	DWORD junk21, junk22;				// 14
	DWORD ofsTargetDir;					// 1c
	DWORD junk31;						// 20
	DWORD ofsPassword;					// 24
	DWORD junk41, junk42, junk43, junk44;	// 28
	DWORD ofsMisc;						// 38
	DWORD junk51, junk52, junk53;		// 3c
	DWORD ofsDispName;					// 48
	DWORD ofsCDRomFolder;				// 4c
	DWORD ofsHTTPLocation;				// 50
	DWORD ofsFTPLocation;				// 54
	DWORD junk61, junk62, junk63, junk64, junk65, junk66;	// 58
	WORD cFileGroups;					// 70
	DWORD ofsFileGroups;				// 72
	WORD cDepends;						// 76
	DWORD ofsDepends;					// 78
	WORD cSubComps;						// 7c
	DWORD ofsSubComps;					// 7e
	WORD cX3;							// 82
	DWORD ofsX3;						// 84
	DWORD ofsNextComponent;				// 88
} COMPONENTDESC, *LPCOMPONENTDESC;		// size=8c

typedef struct tagFILEGROUPDESC {
	DWORD ofsName;
	DWORD cbExpanded;
	DWORD cbCompressed;
	WORD Attr1, Attr2;
	DWORD Shared;
	DWORD SelfRegister;
	DWORD PotentiallyLocked;
	DWORD Compressed;
	DWORD ofsVar1, ofsVar2, ofsVar3;
	DWORD ofsHTTPLocation;
	DWORD ofsFTPLocation;
	DWORD ofsMisc;
	DWORD junk21, junk22, junk23, junk24, junk25;
	DWORD FirstFile;
	DWORD LastFile;
	WORD FirstVolume;
	WORD LastVolume;
	DWORD ofsVar4;
	BYTE junk31;
} FILEGROUPDESC, *LPFILEGROUPDESC;

typedef struct tagSETUPTYPEHEADER {
	DWORD cSTypes;
	DWORD ofsSTypeTab;
} SETUPTYPEHEADER, *LPSETUPTYPEHEADER;

typedef DWORD* SETUPTYPETABLE;

typedef struct tagSETUPTYPEDESC {
	DWORD ofsName;
	DWORD ofsDescription;
	DWORD ofsDispName;
	DWORD junk1;
	DWORD ofsSTab;
} SETUPTYPEDESC, *LPSETUPTYPEDESC;

#pragma pack()

#define GetString(ptr, ofs) ( (LPSTR) (((LPBYTE)ptr) + ((DWORD)ofs)) )

#define GetCompEntry(ptr, ofs) ( (LPCOMPONENTENTRY) (((LPBYTE)ptr) + ofs) )

#define GetFileGroupEntry(ptr, ofs) ( (LPFILEGROUPENTRY) (((LPBYTE)ptr) + ofs) )

#define GetSetupTypeDesc(ptr, stt, i) ( (LPSETUPTYPEDESC) (((LPBYTE)ptr) + ((SETUPTYPETABLE)stt)[i]) )

#define GetCompDesc(ptr, ct, i) ( (LPCOMPONENTDESC) (((LPBYTE)ptr) + ((COMPONENTTABLE)ct)[i]) )

#define GetFileGroupDesc(ptr, fgt, i) ( (LPFILEGROUPDESC) (((LPBYTE)ptr) + ((FILEGROUPTABLE)fgt)[i]) )

#define GetFileDesc(dft, i) ( (LPFILEDESC) (((LPBYTE)dft) + ((DFTABLE)dft)[i]) )


#endif // ISHIELD5_INC
