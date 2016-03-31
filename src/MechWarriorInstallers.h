////////////////////////////////////////////////////////////////////////////////
// MechWarrior installers
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef MechWarriorIIInstaller__H
#define MechWarriorIIInstaller__H

#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// CD types

const int unknownCD = 0;
const int MW2_31stcc_DOS = 1;
const int MW2_31stcc_PE = 2;
const int MW2_31stcc_Hybrid_DOS_Win = 3;
const int MW2_NetMech = 4; // NetMech + 31stcc DOS + 31stcc Win
const int MW2_NetMechInstall = 5;
const int MW2_31stcc_BP = 6;
const int MW2_31stcc_TITAN = 7;
const int MW2_31stcc_3DFX = 8;
const int MW2_31stcc_ATI = 9;
const int MW2_31stcc_TT_BP = 10;  // Install software, 8-bit Battlepack version from Titanium Trilogy
const int MW2_31stcc_Matrox = 11;
const int MW2_GBL_DOS = 20;
const int MW2_GBL_PE = MW2_GBL_DOS+1;
const int MW2_GBL_Hybrid_DOS_Win = MW2_GBL_DOS+2;
const int MW2_GBL_BP = MW2_GBL_DOS+3;
const int MW2_GBL_TT_BP = MW2_GBL_DOS+4; // Install software, 8-bit Battlepack version from Titanium Trilogy
const int MW2_GBL_TITAN = MW2_GBL_DOS+5;
const int MW2_Mercs_DOS = 30;
//const int MW2_Mercs_Win = MW2_Mercs_DOS+1; // Windows variant of basic CD, currently unused
const int MW2_Mercs_3DFX = MW2_Mercs_DOS+2;
const int MW2_Mercs_TT_BP = MW2_Mercs_DOS+3; // Install software, 8-bit Battlepack version from Titanium Trilogy
const int MW2_Mercs_TITAN = MW2_Mercs_DOS+4;

const int MW3_CD_eng = 50;
const int MW3_CD_ger = 51;
const int MW3_eng_installed = 60;
const int MW3_ger_installed = 61;
const int MW3_PM_CD = 80;
const int MW3_PM_installed = 81;

////////////////////////////////////////////////////////////////////////////////
// Exported functions

// Install method
bool install (int CDtype, const TCHAR* source, const TCHAR* imgFN);

// Determine type of CD to install from
int getCDType (const TCHAR* source);

//void appendInstallPath (BGString& str, int cdType);

bool createInstallReport(const TCHAR* source, const TCHAR* target, 
                         bool queryFN = false);

bool installNetMech();

const char* getCDName (int CDID);

#endif
