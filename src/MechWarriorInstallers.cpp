////////////////////////////////////////////////////////////////////////////////
// MechWarrior II installers
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "MechWarriorInstallers.h"
#include "dialogs.h"
#include "FileCache.h"
#include "XMLTree.h"
#include "Config.h"
#include "MechWarriorIIPRJ.h"
//#include "Mesh.h"
#include "MWBase.h"
//#include "Vehicles.h"
#include "MechVM.h"
#include "MechWarrior3ZBD.h"
#include "MW2MechImporter.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Check if directory source has a certain file or directory

bool directoryExists(const TCHAR* source, const TCHAR* FN)
{
   BGString path = source;
   if (!path.lastCharEquals (OSPathSep))
      path += OSPathSep;
   path += FN;
#ifdef _WIN32
   WIN32_FIND_DATA info;
   path += "\\*";
   HANDLE h = FindFirstFile (path.getChars(), &info);
   bool found = (h != INVALID_HANDLE_VALUE);
   FindClose(h);
#else
   struct stat fileStat;
   stat (path.getChars(), &fileStat);
   bool found = S_ISDIR(fileStat.st_mode);
#endif

   return found;
}

////////////////////////////////////////////////////////////////////////////////

void buildSubDirPath (const TCHAR* source, const TCHAR* subDir, 
                      bool wantTrailingSlash, BGString& retStr)
{
   retStr = source;
   if (strlen(subDir) != 0) {
      if (!retStr.lastCharEquals (OSPathSep))
         retStr += OSPathSep;
      retStr += subDir;
   }

   if (!retStr.lastCharEquals (OSPathSep) && wantTrailingSlash)
      retStr += OSPathSep;
}

////////////////////////////////////////////////////////////////////////////////

void mkdir (const TCHAR* target, const TCHAR* dirName)
{
   BGString fullPath = target;
   fullPath += OSPathSep;
   fullPath += dirName;
   bgmkdir (fullPath.getChars());
   //CreateDirectory (fullPath.getChars(), NULL);
}

void copyFile (const TCHAR* source, const TCHAR* FN, const TCHAR* target, 
               const TCHAR* targetFN)
{
   BGString fullSource, fullTarget;
   buildSubDirPath (source, FN, false, fullSource);
   buildSubDirPath (target, targetFN, false, fullTarget);

   if (fileExists (fullSource.getChars()))
      copyFile (fullSource.getChars(), fullTarget.getChars());
   else
      printf ("Could not find %s\n", fullSource.getChars());
}

#ifdef _WIN32
void copyFiles (const TCHAR* source, const TCHAR* mask, const TCHAR* target)
{
   BGString fullsource = source;
   fullsource.appendNoDups (OSPathSep);
   fullsource += mask;

   WIN32_FIND_DATA info;
   HANDLE h = FindFirstFile (fullsource.getChars(), &info);
   bool found = (h != INVALID_HANDLE_VALUE);

   while (found) {
      bool noDir = ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
      if (noDir) {
         BGFilePath<TCHAR> sourceStr = source;
         sourceStr += info.cFileName;
         BGFilePath<TCHAR> targetStr = target;
         targetStr += info.cFileName;
         copyFile (sourceStr.getChars(), targetStr.getChars());
      }
      if (!FindNextFile (h, &info))
         found = false;
   }
   FindClose(h);
}

#else // Linux

void copyFiles (const TCHAR* source, const TCHAR* /*mask*/, const TCHAR* target)
{
   struct dirent *dp;
   DIR *dir = opendir(source);
   if (dir == NULL) {
      printf ("%s does not exist - cannot copy\n", source);
      return;
   }

   while ((dp=readdir(dir)) != NULL) {
      BGFilePath<TCHAR> sourceFN = source;
      sourceFN += dp->d_name;

      BGFilePath<TCHAR> targetFN = target;
      targetFN += dp->d_name;

      //printf("Copying %s to %s\n", sourceFN.getChars(), targetFN.getChars());
      copyFile (sourceFN.getChars(), targetFN.getChars());
   }

   closedir(dir);
}
#endif

/*#ifdef _WIN32
void copyDir (const TCHAR* source, const TCHAR* subDir, const TCHAR* target, 
              const TCHAR* targetSubDir)
{
   BGString fullsource;
   buildSubDirPath (source, subDir, true, fullsource);
   BGString searchPath = fullsource;
   searchPath += "*";

   BGString fulltarget;
   buildSubDirPath (target, targetSubDir, true, fulltarget);

   WIN32_FIND_DATA info;
   HANDLE h = FindFirstFile (searchPath.getChars(), &info);
   bool found = (h != INVALID_HANDLE_VALUE);

   while (found) {
      bool noDir = ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0);
      if (noDir) {
         BGString sourceStr = fullsource;
         sourceStr += info.cFileName;
         BGString targetStr = fulltarget;
         targetStr += info.cFileName;
         copyFile (sourceStr.getChars(), targetStr.getChars());
      }
      if (!FindNextFile (h, &info))
         found = false;
   }
   FindClose(h);
}

#else // Linux
*/
void copyDir (const TCHAR* source, const TCHAR* subDir, const TCHAR* target, 
              const TCHAR* targetSubDir)
{
   BGString fullsource, fulltarget;
   buildSubDirPath (source, subDir, true, fullsource);
   buildSubDirPath (target, targetSubDir, true, fulltarget);
   copyFiles (fullsource.getChars(), "*", fulltarget.getChars());
}

//#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void setWin95Mode (const TCHAR* path, const TCHAR* exeName, bool color256 = false)
{
   const TCHAR keyPath[] = "Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers";
   BGString subKey = path;
   subKey += "\\";
   subKey += exeName;
   HKEY hKey;
   const TCHAR* value = "WIN95";
   if (color256)
      value = "WIN95 256COLOR";
   RegCreateKeyEx (HKEY_CURRENT_USER, keyPath, 0, 0, 
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, 0);	
   RegSetValueEx (hKey, subKey.getChars(), 0, REG_SZ, (BYTE*) value, 
                  (DWORD) strlen(value));
   RegCloseKey (hKey);
}

#else // Linux
void setWin95Mode (const TCHAR* /*path*/, TCHAR* /*exeName*/, bool /*color256*/ = false)
{
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void patch42 (const char* target, const char* fn)
{
   BGString fullFN = target;
   fullFN.appendNoDups(OSPathSep);
   fullFN += fn;

   MemoryBlock mb;
   if (mb.loadFromFile (fullFN.getChars())) {
      printf ("Patching %s\n", fullFN.getChars());
      mb.replaceAll ("GDI32.dll", "GDI42.DLL");
      mb.replaceAll ("kernel32.dll", "kernel42.dll");
      mb.replaceAll ("KERNEL32.dll", "kernel42.dll");
      mb.replaceAll ("ADVAPI32.dll", "ADVAPI42.dll");
      mb.saveToFile(fullFN.getChars());
   } else
      printf ("Could not patch %s because it could not be opened\n", 
         fullFN.getChars());
}

bool patch(const char* dir, const char* diffFN, const char* targetFN)
{
   BGString fullDiffFN = dir;
   fullDiffFN.appendNoDups (OSPathSep);
   fullDiffFN += diffFN;

   BGString fullTargetFN = dir;
   fullTargetFN.appendNoDups (OSPathSep);
   fullTargetFN += targetFN;

   bool result = patch (fullDiffFN.getChars(), fullTargetFN.getChars());
   DeleteFile (fullDiffFN.getChars());
   return result;
}

#else // Linux

void patch42 (const char* /*target*/, const char* /*fn*/)
{
   // Just leave the files untouched ...
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Build install path

void getInstallPath (BGString& path, const TCHAR* subPath)
{
   path = getDataDir();
   path += OSPathSep;
   path += "games";
   path += OSPathSep;
   path += subPath;
}

////////////////////////////////////////////////////////////////////////////////

//#include <windows.h>
//#include <stdio.h>

bool isWOW64Proc()
{
#ifdef _WIN32
   typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

   BOOL bIsWow64 = FALSE;
   LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  
    if (fnIsWow64Process != NULL)
    {
        if (fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            //return bIsWow64; // VS: "performance warning"
            if (bIsWow64)
               return true;
            else
               return false;
        }
    }
    return false;
#else
   return false;
#endif
}

bool hasWin16Subsystem()
{
   // Win16 subsystem not present in Win64
   return !isWOW64Proc();
}

////////////////////////////////////////////////////////////////////////////////

void appendFileList (const TCHAR* source, FileCache& fc)
{
   BGString str = newLine;
   str += "File listing for ";
   str += source;
   str += newLine;
   fc.append (str.getChars());

   // List files
   FileList fl;
   BGFilePath<TCHAR> fp = source;
   fp += "*";
   if (fl.start (fp.getChars())) {
      do {
         str = fl.getFileName();
         if (fl.isDirectory())
            str += " <dir>";
         else {
            str += " ";
            str.appendInt (fl.getSize());
	 }
         str += newLine;
         fc.append (str.getChars());
      } while (++fl);
   }

   // Recurse
   if (fl.start (fp.getChars())) {
      do {
         if (fl.isDirectory()
         &&  strcmp (fl.getFileName(), ".") != 0
         &&  strcmp (fl.getFileName(), "..") != 0) 
         {
            BGFilePath<TCHAR> dir = source;
            dir += fl.getFileName();
            appendFileList(dir.getChars(), fc);
         }
      } while (++fl);
   }
}

bool createInstallReport(const TCHAR* source, const TCHAR* target, bool queryFN)
{
   BGFilePath<TCHAR> ofn;
   if (queryFN) {
      if (!getSaveFileName(ofn))
         return false;
   } else {
      ofn = target;
      ofn += "install-report.txt";
   }
   FileCache fc;
   fc.openReadWrite(ofn.getChars());

   BGString str = getVersionStr ();
   str += newLine;
   fc.append(str.getChars());

   // Append OS version
   #ifdef _WIN32
   BGString winVer = "Windows ";
   DWORD dwVersion = GetVersion();
   DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
   DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
   DWORD dwBuild = (DWORD)(HIWORD(dwVersion));

   winVer.appendInt(dwMajorVersion);
   winVer += ".";
   winVer.appendInt(dwMinorVersion);
   winVer += ".";
   winVer.appendInt(dwBuild);

   if (isWOW64Proc())
      winVer += "(64 bit)";
   else
      winVer += "(32 bit)";
   winVer += newLine;
   fc.append (winVer.getChars());
   #endif

   // Append source and target
   if (source != NULL) {
      str = "Source: ";
      str += source;
      str += newLine;
      fc.append(str.getChars());
   }

   if (target != NULL) {
      str = "Target: ";
      str += target;
      str += newLine;
      fc.append(str.getChars());
   }

   if (source != NULL)
      appendFileList (source, fc);
   if (target != NULL)
      appendFileList (target, fc);
   return true;
}

////////////////////////////////////////////////////////////////////////////////

const int makeBATfile_InGameDir = 1; // if not specified, uses games\bat
const int makeBATfile_Win95Compatibility = 2;
const int makeBATfile_RunAsAdmin = 4;
const int makeBATfile_256Color = 8;
const int makeBATfile_SetupIPX = 16;
const int makeBATfile_NoLFB = 32;
const int makeBATfile_cdWithQuotes = 64;

void makeBATfile (const TCHAR* BATFN, const TCHAR* gameDir, const char* EXEFN, 
                  const TCHAR* IMGFN, int flags = 0)
{
   // Create BAT file for launching the game through DosBox
   BGFilePath<TCHAR> fullBATFN;

   if ((flags & makeBATfile_InGameDir) == 0) {
      fullBATFN = getExecDir();
      fullBATFN += "games";
      fullBATFN += "bat";
      bgmkdir (fullBATFN.getChars());
   } else
      fullBATFN = gameDir;
   fullBATFN += BATFN;

   BGString bat = "@echo off\n";
   if (IMGFN != NULL) {
      if (strlen(IMGFN) > 0) {
         bat += "z:\\imgmount d \"";
         BGFilePath<TCHAR> CUEFN = IMGFN;
         bool appendIMGFN = true;
         if (CUEFN.changeExtension("CUE"))
            appendIMGFN = !CUEFN.exists();
         if (appendIMGFN)
            bat += IMGFN;
         else
            bat += CUEFN;
         bat += "\" -t iso\n";
      }
   }

   if ((flags & makeBATfile_SetupIPX) != 0)
      bat += "ipxnet connect mech2.org 10000\n";

   BGString compat;
   if ((flags & makeBATfile_Win95Compatibility) > 0)
      compat += "Win95 ";
   if ((flags & makeBATfile_RunAsAdmin) > 0)
      compat += "RunAsAdmin ";
   if ((flags & makeBATfile_256Color) > 0)
      compat += "256Color ";

   if (compat.getLength() > 0) {
      bat += "set __COMPAT_LAYER=";
      bat += compat;
      bat += "\n";
   }

   bool relPath = true;
   if (strlen(gameDir) > 2)
      if (gameDir[1] == ':') 
         relPath = false;

   if ((flags & makeBATfile_NoLFB) > 0)
      bat += "NoLFB\n";

   if (relPath) {
      bat += "c:\n";
      bat += "cd \\";
      if ((flags & makeBATfile_cdWithQuotes) > 0)
         bat += "\"";
      bat += gameDir;
      if ((flags & makeBATfile_cdWithQuotes) > 0)
         bat += "\"";
      bat += "\n";
   } else {
      bat += gameDir[0];
      bat += ":\ncd";
      bat += &gameDir[2];
      bat += "\n";
   }

   bat += EXEFN;
   bat += " %1\nexit\n";
   bat.saveToFile(fullBATFN.getChars());
}

////////////////////////////////////////////////////////////////////////////////

void addVarKey (const char* varName, const char* varVal)
{
   BGString key = varName;
   key += "=";
   key += varVal;
   addKeyValuePair("SetShellVar", key.getChars());
}

////////////////////////////////////////////////////////////////////////////////

void addKeysForNativeGame(const char* gameName, const char* gamePath, 
                          const char* EXEFN, bool createNewWindow = false)
{
   addKeyValuePair("GameName", gameName);
   addKeyValuePair("StartMethod", "RunNative");
   addKeyValuePair("GamePath", gamePath);
   addKeyValuePair("compatibility", "Win95");

   BGString exeFN;
   if (createNewWindow)
      exeFN = "cmd /c start ";
   exeFN += gamePath;
   exeFN += OSPathSep;
   exeFN += EXEFN;
   addKeyValuePair("ExecutableFN", exeFN.getChars());
   addKeyValuePair("", "");
}

////////////////////////////////////////////////////////////////////////////////

void addKeysForDosBox(const char* GameName, const char* batFile)
{
   addKeyValuePair("GameName", GameName);
   addKeyValuePair("StartMethod", "RunNative");

   BGString DosBoxPath;
   getDosBoxPath(DosBoxPath);

   BGFilePath<TCHAR> DosBoxExe = DosBoxPath.getChars();
   DosBoxExe += "dosbox.exe";
   addKeyValuePair("ExecutableFN", DosBoxExe.getChars());
   addKeyValuePair("GamePath", DosBoxPath.getChars());

   BGString parameters = "-conf \"";
   parameters += DosBoxPath;
   parameters += OSPathSep;
   parameters += "DosBox-0.74.conf\" -c ";
   parameters += batFile;
   addKeyValuePair("parameters", parameters.getChars());
   addKeyValuePair("", "");
}

////////////////////////////////////////////////////////////////////////////////

void addKeysForMixedDosBoxGame (const char* gameName, const char* gamePathInDosBox, 
   const char* campaignFile, const char* EXEFN, const char* MEKFileVersion,
   const char* weaponsLocker)
{
   addKeyValuePair("GameName", gameName);
   addKeyValuePair("StartMethod", "InternalShell");
   addKeyValuePair("CampaignFile", campaignFile);
   addVarKey("GamePathInDosBox", gamePathInDosBox);
   addVarKey("SimType", "DosBox");
   addVarKey("SimExec", EXEFN);
   addVarKey("mission", "0");
   addVarKey("persistent", "mission");
   addVarKey("MEKFileVersion", MEKFileVersion);
   addVarKey("weaponsLocker", weaponsLocker);

   BGFilePath<TCHAR> MEKdir = getExecDir();//gamePath;
   MEKdir += "games";
   MEKdir += gamePathInDosBox;
   MEKdir += "mek";
   addVarKey("MEKdir", MEKdir.getChars());

   BGFilePath<TCHAR> archive1Path = getExecDir();
   archive1Path += "games";
   archive1Path += gamePathInDosBox;
   archive1Path += "database.mw2";
   addKeyValuePair ("archive", archive1Path.getChars());

   BGFilePath<TCHAR> archive2Path = getExecDir();
   archive2Path += "games";
   archive2Path += gamePathInDosBox;
   archive2Path += "mw2.prj";
   addKeyValuePair ("archive", archive2Path.getChars());
   addKeyValuePair("", "");
}

////////////////////////////////////////////////////////////////////////////////

void addKeysForMixedNativeGame (const char* gameName, const char* gamePath, 
   const char* campaignFile, const char* EXEFN, const char* MEKFileVersion,
   const char* weaponsLocker)
{
   addKeyValuePair("GameName", gameName);
   addKeyValuePair("StartMethod", "InternalShell");
   addKeyValuePair("GamePath", gamePath);
   addKeyValuePair("CampaignFile", campaignFile);
   addKeyValuePair("SetShellVar", "SimType=native");
   addKeyValuePair("SetShellVar", "compatibility=Win95");
   addVarKey("mission", "0");
   addVarKey("persistent", "mission");
   addVarKey("MEKFileVersion", MEKFileVersion);
   addVarKey("weaponsLocker", weaponsLocker);

   BGFilePath<TCHAR> MEKdir = gamePath;
   MEKdir += "mek";
   BGString MEKdirVar = "MEKdir=";
   MEKdirVar += MEKdir;
   addKeyValuePair("SetShellVar", MEKdirVar.getChars());

   addVarKey ("GamePath", gamePath);

   BGString execVar = "SimExec=";
   execVar += gamePath;
   execVar.appendNoDups(OSPathSep);
   execVar += EXEFN;
   addKeyValuePair("SetShellVar", execVar.getChars());

   BGFilePath<TCHAR> archive1Path = gamePath;
   archive1Path += "database.mw2";
   addKeyValuePair ("archive", archive1Path.getChars());

   BGFilePath<TCHAR> archive2Path = gamePath;
   archive2Path += "mw2.prj";
   addKeyValuePair ("archive", archive2Path.getChars());
   addKeyValuePair("", "");
}

////////////////////////////////////////////////////////////////////////////////

bool install_MW2_31stcc_DOS(const TCHAR* source, int CDtype, const TCHAR* imgFN)
{
   BGString gamePath;
   bool DosCanRunNatively = hasWin16Subsystem();
   const char* gamePathPart = "mw2-31st";
   getInstallPath (gamePath, gamePathPart);
   displayMsg ("Installing MW2: 31st Century Combat (DOS)");
   bool hasDosVer = (CDtype == MW2_31stcc_DOS || CDtype == MW2_NetMech);
   bool hasWinVer = (CDtype == MW2_31stcc_PE  || CDtype == MW2_NetMech);

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "giddi");
   mkdir (gamePath.getChars(), "mek");
   mkdir (gamePath.getChars(), "SETUP");
   mkdir (gamePath.getChars(), "SND");
   mkdir (gamePath.getChars(), "vfx");
   mkdir (gamePath.getChars(), "DEMODATA");

   // Copy directories
   copyDir (source, "giddi", gamePath.getChars(), "giddi");
   copyDir (source, "giddidos", gamePath.getChars(), "giddi");
   copyDir (source, "giddiwin", gamePath.getChars(), "giddi");
   copyDir (source, "snd", gamePath.getChars(), "SND");
   copyDir (source, "vfx", gamePath.getChars(), "VFX");
   copyDir (source, "mech2", gamePath.getChars(), "");
   copyDir (source, "cfgs", gamePath.getChars(), "");

   BGString installPath = getExecDir();
   installPath += OSPathSep;
   installPath += "install";
   copyDir (installPath.getChars(), "MW2-31st-DOS", gamePath.getChars(), "");
   copyDir (installPath.getChars(), "NoLFB", gamePath.getChars(), "");
   copyDir (installPath.getChars(), "MW2-31st-DOS\\SND", gamePath.getChars(), "SND");

   // Copy files
   copyFile (source, "vesachk.exe", gamePath.getChars(),"SETUP\\VESACHK.EXE");
   copyFile (source, "fixboot.exe", gamePath.getChars(), "SETUP\\FIXBOOT.EXE");
   copyFile (source, "inwin.exe", gamePath.getChars(), "SETUP\\INWIN.EXE");
   copyFile (source, "german.lrm", gamePath.getChars(), "GERMAN.LRM");
   copyFile (source, "readme.txt", gamePath.getChars(), "README.TXT");
   copyFile (installPath.getChars(), "tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");

   if (hasDosVer && DosCanRunNatively)
      copyFile (installPath.getChars(), "tools\\mouse2kv.exe", gamePath.getChars(), "mouse2kv.exe");

   if (hasWinVer) {
      copyDir (installPath.getChars(), "MW2-31st-PE", gamePath.getChars(), "");
      copyFiles (source, "*.mw2", gamePath.getChars());
      copyFiles (source, "*.bwd", gamePath.getChars());
      copyFiles (source, "about.*", gamePath.getChars());
      copyFiles (source, "tech.*", gamePath.getChars());
      copyFiles (source, "mw2*", gamePath.getChars());

      copyFile (source, "input.bak", gamePath.getChars(), "input.bak");
      copyFile (source, "netmechw.dll", gamePath.getChars(), "netmechw.dll");
      copyFile (source, "readme.txt", gamePath.getChars(), "readme.txt");
      copyFile (source, "smackw32.dll", gamePath.getChars(), "smackw32.dll");
      copyFile (source, "symlog.txt", gamePath.getChars(), "symlog.txt");
      //copyFile (source, "tmp.out", gamePath.getChars(), "tmp.out");
      copyFile (source, "wail32.dll", gamePath.getChars(), "wail32.dll");
      copyFile (source, "mech2.ex_", gamePath.getChars(), "mech2win.exe");
      
      BGString subDirMech2 = source;
      subDirMech2 += OSPathSep;
      subDirMech2 = "mech2";
      copyFile (subDirMech2.getChars(), "gamekey.map", gamePath.getChars(), "GAMEKEY.MAP");
      copyFile (subDirMech2.getChars(), "input.map", gamePath.getChars(), "input.map");

      copyDir (installPath.getChars(), "MW2-Dll42", gamePath.getChars(), "");
   }

   // Create registry keys for running the DOS version natively
   if (hasDosVer && DosCanRunNatively) {
      setWin95Mode (gamePath.getChars(), "MW2.exe");
      setWin95Mode (gamePath.getChars(), "MECH2.exe");
   }

   // Get image file name
   BGString imgFNStr = imgFN;
   bool haveIMGFN = (imgFN != NULL);
   if (hasDosVer && !haveIMGFN) {
  	   messageBox("CD Image file needed", "Please select the location of a CD "
                 "image file. This is needed to run the game in DosBox.");
      haveIMGFN = getOpenFileName(imgFNStr);
      if (!haveIMGFN)
         displayMsg ("Cannot add game to game list - missing CD image file");
   }

   // Create registry keys for the Win95 version and patch it 
   if (hasWinVer) {
      setWin95Mode (gamePath.getChars(), "MW2WIN.exe");
      patch42 (gamePath.getChars(), "MECH2WIN.EXE");
      //patch42 (gamePath.getChars(), "mech2.exe");
      patch42 (gamePath.getChars(), "mw2.dll");
      patch42 (gamePath.getChars(), "mw2shell.dll");
   }

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());

   // Add entries for launching the game from MechVM
   BGFilePath<TCHAR> archive1Path = gamePath.getChars();
   BGFilePath<TCHAR> archive2Path = archive1Path;
   archive1Path += "DATABASE.MW2";
   archive2Path += "MW2.PRJ";

   if (haveIMGFN) {
      // Start game in DosBox
      addKeysForDosBox("MW2: 31st Century Combat (DOS, DosBox)", "mw2");

      // Create BAT file for launching the game through DosBox
      makeBATfile ("mw2.bat", gamePathPart, "mech2.exe", imgFNStr.getChars());

      // Use MechVM shell, but run sim through DosBox
         makeBATfile ("mw2ml.bat", gamePathPart, "mw2.exe", imgFNStr.getChars());
         addKeysForMixedDosBoxGame ("MW2: 31st Century Combat, Clan Jade Falcon (MechVM shell + DosBox)", 
            gamePathPart, "campaign-CJF.xml", "mw2ml.bat", "0", "31stcc");
         addKeysForMixedDosBoxGame ("MW2: 31st Century Combat, Clan Wolf (MechVM shell + DosBox)", 
            gamePathPart, "campaign-WC.xml", "mw2ml.bat", "0", "31stcc");
   }

   if (DosCanRunNatively) {
      // Start game natively
      makeBATfile ("_mw2.bat", gamePath.getChars(), "mech2.exe", NULL,
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | makeBATfile_NoLFB);
      addKeysForNativeGame("MW2: 31st Century Combat (DOS, native)",
         gamePath.getChars(), "_mw2.bat", true);

      // Use MechVM's shell, but run MW2 sim natively
         addKeysForMixedNativeGame("MW2: 31st Century Combat, Clan Jade Falcon (MechVM shell + native sim)", 
            gamePath.getChars(), "campaign-CJF.xml", "mw2.exe", "0", "31stcc");
         addKeysForMixedNativeGame("MW2: 31st Century Combat, Clan Wolf (MechVM shell + native sim)", 
            gamePath.getChars(), "campaign-WC.xml", "mw2.exe", "0", "31stcc");
   }

   // Add MechVM.cfg entries for Win95 edition
   if (hasWinVer) {
      makeBATfile ("_mw2win.bat", gamePath.getChars(), "mech2win.exe", NULL,
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
         makeBATfile_RunAsAdmin | makeBATfile_cdWithQuotes);
      addKeysForNativeGame ("MW2: 31st Century Combat (Win95, native)",
         gamePath.getChars(), "_mw2win.bat");
   }

   createInstallReport(source, gamePath.getChars());

   bool isXP = false;
   #ifdef __WINDOWS
   DWORD dwVersion = GetVersion();
   DWORD dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
   DWORD dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));
   
   if (dwMajorVersion < 6) {
      isXP = true;
   }
   #endif
   
   if (isXP) {
      displayMsg("Install complete, please read readmeXP.txt");
   } else {
      BGString str = "Installation complete, ";
      str.appendUInt(importedMechs);
      str += " mechs imported";
      displayMsg(str.getChars());
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////

bool installGBL (const TCHAR* source, int CDtype, const TCHAR* imgFN)
{
   BGString gamePath, gamePathPart;
   bool DosCanRunNatively = false;
   bool hasDosVer = (CDtype == MW2_GBL_DOS || CDtype == MW2_GBL_Hybrid_DOS_Win);
   bool hasWinVer = (CDtype == MW2_GBL_PE  || CDtype == MW2_GBL_Hybrid_DOS_Win);
   if (hasDosVer)
      DosCanRunNatively = hasWin16Subsystem();
   gamePathPart = "mw2-gbl";
   getInstallPath (gamePath, gamePathPart.getChars());
   displayMsg ("Installing MW2: Ghost Bear's Legacy");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "MEK");
   mkdir (gamePath.getChars(), "SETUP");
   mkdir (gamePath.getChars(), "SND");
   mkdir (gamePath.getChars(), "VFX");

   // Copy 
   copyDir (source, "gbl", gamePath.getChars(), "");
   copyDir (source, "giddi", gamePath.getChars(), "GIDDI");
   copyDir (source, "snd", gamePath.getChars(), "SND");
   copyDir (source, "vfx", gamePath.getChars(), "VFX");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", 
      gamePath.getChars(), "_farPatcher.exe");

   BGString installPath = getExecDir();
   installPath += OSPathSep;
   installPath += "install";

   if (hasDosVer) {
      copyDir (installPath.getChars(), "NoLFB", gamePath.getChars(), "");
      copyDir (installPath.getChars(), "MW2-GBL-DOS", gamePath.getChars(), "");
      copyDir (installPath.getChars(), "MW2-31ST-DOS\\SND", gamePath.getChars(), "SND");

      /// Copy single files
      copyFile (source, "german.lrm", gamePath.getChars(), "GERMAN.LRM");
      copyFile (source, "readme.txt", gamePath.getChars(), "README.TXT");
      copyFile (source, "gblinfo.txt", gamePath.getChars(), "GBLInfo.TXT");
      copyFile (source, "cfgs\\mw2.phx", gamePath.getChars(), "MW2.PHX");
      
      BGFilePath<TCHAR> setupPath = gamePath.getChars();
      setupPath += "SETUP";
      copyFile (source, "fixboot.exe", setupPath.getChars(), "FixBoot.exe");
      copyFile (source, "inwin.exe", setupPath.getChars(), "INWIN.EXE");
      copyFile (source, "mw2reg.exe", setupPath.getChars(), "MW2REG.EXE");
      copyFile (source, "mw2reg.ini", setupPath.getChars(), "MW2REG.INI");
      copyFile (source, "vesachk.exe", setupPath.getChars(), "VESACHK.EXE");

      if (DosCanRunNatively) {
         setWin95Mode (gamePath.getChars(), "GBL.EXE");
         copyFile (getExecDir(), "install\\tools\\mouse2kv.exe", 
            gamePath.getChars(), "mouse2kv.exe");
      }
   }

   if (hasWinVer) {
      copyFile (source, "mw2.dll", gamePath.getChars(), "mw2.dll");
      copyFile (source, "mw2shell.dll", gamePath.getChars(), "mw2shell.dll");
      copyFile (source, "smackw32.dll", gamePath.getChars(), "smackw32.dll");
      copyFile (source, "wail32.dll", gamePath.getChars(), "wail32.dll");

      BGString path = "gbl";
      path += OSPathSep;
      path += "gblwin.exe";
      copyFile (source, path.getChars(), gamePath.getChars(), "GBLWin.exe");

      copyDir (installPath.getChars(), "MW2-Dll42", gamePath.getChars(), "");
      copyDir (installPath.getChars(), "MW2-GBL-PE", gamePath.getChars(), "");

      patch42 (gamePath.getChars(), "mw2.dll");
      patch42 (gamePath.getChars(), "mw2shell.dll");
      setWin95Mode (gamePath.getChars(), "GBLWin.exe");
   }

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());

   // Add entries for launching the game from MechVM
   BGFilePath<TCHAR> archive1Path = gamePath.getChars();
   BGFilePath<TCHAR> archive2Path = archive1Path;
   archive1Path += "DATABASE.MW2";
   archive2Path += "MW2.PRJ";

   BGString imgFNstr = imgFN;
   bool haveIMGFN = (imgFN != NULL);
   if (hasDosVer && !haveIMGFN) {
      messageBox("CD Image file needed", "Please select the location of a CD "
                 "image file. This is needed to run the game in DosBox.");
      if (getOpenFileName(imgFNstr))
         haveIMGFN = true;
      else
         displayMsg ("Cannot add game to game list - missing CD image file");
   }

   if (hasDosVer && haveIMGFN) {
      makeBATfile ("gbl.bat", gamePathPart.getChars(), "gbl.exe", 
         imgFNstr.getChars());
      addKeysForDosBox("MW2: Ghost Bear's Legacy (DOS, DosBox)", "gbl.bat");

      makeBATfile ("_gbldbml.bat", gamePathPart.getChars(), "mw2.exe", 
         imgFNstr.getChars());
      addKeysForMixedDosBoxGame("MW2: Ghost Bear's Legacy (MechVM shell + DosBox)",
         gamePathPart.getChars(), "campaign-gbl.xml", "_gbldbml.bat", "0", "gbl");
   }

   if (DosCanRunNatively && hasDosVer) {
      makeBATfile ("_gbldos.bat", gamePath.getChars(), "gbl.exe", NULL, 
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | makeBATfile_NoLFB);
      addKeysForNativeGame("MW2: Ghost Bear's Legacy (DOS, native)", 
         gamePath.getChars(), "_gbldos.bat", true);

      //if (experimentalFeaturesEnabled()) {
         makeBATfile ("_gblml.bat", gamePath.getChars(), "gbl.exe", NULL, 
            makeBATfile_InGameDir | makeBATfile_Win95Compatibility);
         addKeysForMixedNativeGame("MW2: Ghost Bear's Legacy (MechVM shell + native DOS sim)", 
            gamePath.getChars(), "campaign-gbl.xml", "_gblml.bat", "0", "GBL");
      //}
   }

   if (hasWinVer) {
      makeBATfile ("_gblwin.bat", gamePath.getChars(), "gblwin.exe", NULL, 
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
         makeBATfile_RunAsAdmin | makeBATfile_cdWithQuotes);
      addKeysForNativeGame ("MW2: Ghost Bear's Legacy (Win95, native)",
         gamePath.getChars(), "_gblwin.bat");

      //if (experimentalFeaturesEnabled()) {
         // GBL-Win95 sim is launched using gblwin.exe, so use same BAT file
         addKeysForMixedNativeGame("MW2: Ghost Bear's Legacy (MechVM shell + native Win95 sim)",
            gamePath.getChars(), "campaign-gbl.xml", "_gblwin.bat", "0", "GBL");
      //}
   }

   createInstallReport(source, gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());
   return true;
}

////////////////////////////////////////////////////////////////////////////////

bool installMercs_DOS (const TCHAR* source, const TCHAR* imgFN)
{
   BGString gamePath;
   BGString gamePathPart = "mw2-merx";
   bool DosCanRunNatively = hasWin16Subsystem();
   getInstallPath (gamePath, gamePathPart.getChars());
   displayMsg ("Installing MW2: Mercenaries (DOS)");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "MEK");
   mkdir (gamePath.getChars(), "SND");
   mkdir (gamePath.getChars(), "VFX");
   mkdir (gamePath.getChars(), "MISSIONS");
   mkdir (gamePath.getChars(), "RMG");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "DLL");

   // Copy directories
   copyDir (source, "giddi", gamePath.getChars(), "GIDDI");
   copyDir (source, "snd", gamePath.getChars(), "SND");
   copyDir (source, "vfx", gamePath.getChars(), "VFX");
   copyDir (source, "missions", gamePath.getChars(), "MISSIONS");
   copyDir (source, "rmg", gamePath.getChars(), "RMG");
   copyDir (source, "smk", gamePath.getChars(), "SMK");
   copyDir (source, "dll", gamePath.getChars(), "DLL");
   copyDir (source, "mercs", gamePath.getChars(), "");
   copyDir (source, "dos", gamePath.getChars(), "");
   copyDir (getExecDir(), "install\\MW2-Mercs-DOS", gamePath.getChars(), "");
   copyDir (getExecDir(), "install\\NoLFB", gamePath.getChars(), "");

   /// Copy single files
   //copyFile (source, "FixBoot.EXE", gamePath.getChars(), "SETUP\\FixBoot.exe");
   //copyFile (source, "VESACHK.EXE", gamePath.getChars(), "SETUP\\VESACHK.EXE");
   //copyFile (source, "GERMAN.LRM", gamePath.getChars(), "GERMAN.LRM");
   //copyFile (source, "README.TXT", gamePath.getChars(), "README.TXT");
   copyFile (getExecDir(), "install\\MW2-Mercs-DOS\\SND\\DIG.INI", gamePath.getChars(), "SND\\DIG.INI");
   copyFile (getExecDir(), "install\\tools\\mouse2kv.exe", gamePath.getChars(), "mouse2kv.exe");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");

   // Install compatibility layer
   BGString installPath = getExecDir();
   installPath += OSPathSep;
   installPath += "install";
   copyDir (installPath.getChars(), "MW2-Dll42", gamePath.getChars(), "");
   patch42 (gamePath.getChars(), "MECH2WIN.EXE");
   patch42 (gamePath.getChars(), "MW2.DLL");
   patch42 (gamePath.getChars(), "MW2SHELL.DLL");

   // Create registry key
   setWin95Mode (gamePath.getChars(), "MERCS.EXE");
   setWin95Mode (gamePath.getChars(), "MERCSWIN.EXE");

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   // Add entries for launching the game from MechVM
   BGFilePath<TCHAR> archive1Path = gamePath.getChars();
   BGFilePath<TCHAR> archive2Path = archive1Path;
   archive1Path += "DATABASE.MW2";
   archive2Path += "MW2.PRJ";

   // Add MercNet
   BGString imgFNstr = imgFN;
   bool haveIMGFN = (imgFN != NULL);
   if (!haveIMGFN) {
      messageBox("CD Image file needed", "Please select the location of a CD "
         "image file. This is needed to run MW2:Mercenaries in DosBox and host "
         "games on MercNet.");
      haveIMGFN = getOpenFileName(imgFNstr);
   }

   if (haveIMGFN) {
      makeBATfile ("mercs.bat", gamePathPart.getChars(), "mercs.exe", 
         imgFNstr.getChars());
      addKeysForDosBox("MW2: Mercenaries (DOS, DosBox)", "mercs.bat");

      if (experimentalFeaturesEnabled()) {
         makeBATfile ("_mercsml.bat", gamePathPart.getChars(), "mw2.exe", 
            imgFNstr.getChars());
         addKeysForMixedDosBoxGame ("MW2: Mercenaries (MechVM shell + DosBox)",
            gamePathPart.getChars(), "campaign-mercs.xml", "_mercsml.bat", "1",
            "all");
      }
   } else
      displayMsg ("Cannot add game to game list - missing CD image file");

   if (DosCanRunNatively) {
      makeBATfile ("_mercsdos.bat", gamePath.getChars(), "mercs.exe", NULL, 
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | makeBATfile_NoLFB);
      addKeysForNativeGame ("MW2: Mercenaries (DOS, native)",
         gamePath.getChars(), "_mercsdos.bat", true);

      if (experimentalFeaturesEnabled()) {
         addKeysForMixedNativeGame("MW2: Mercenaries (MechVM shell + native DOS sim)",
            gamePath.getChars(), "campaign-mercs.xml", "mw2.exe", "1", "all");
      }
   }

   makeBATfile ("_mercswin.bat", gamePath.getChars(), "mercswin.exe", NULL, 
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_cdWithQuotes);
   addKeysForNativeGame ("MW2: Mercenaries (Win95, native)",
      gamePath.getChars(), "_mercswin.bat");

   if (experimentalFeaturesEnabled()) {
      addKeysForMixedNativeGame ("MW2: Mercenaries (MechVM shell + native Win95 sim)",
         gamePath.getChars(), "campaign-mercs.xml", "_mercswin.bat", "1", "all");
   }

   makeBATfile ("mercnet.bat", gamePathPart.getChars(), "mercnet.exe", 
      imgFNstr.getChars(), makeBATfile_SetupIPX);
   addKeysForDosBox("MW2: MercNet (DOS, DosBox)", "mercnet");

   createInstallReport(source, gamePath.getChars());
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Install MW2 31st Century Combat, Battlepack edition

bool install_MW2_BattlePack (const TCHAR* source, int CDtype, const TCHAR* subDir)
{
   BGString gamePath;
   getInstallPath (gamePath, subDir);

   displayMsg ("Installing MW2: 31st Century Combat (Battlepack)");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "VFX");
   mkdir (gamePath.getChars(), "Windll");

   // Copy files
   copyDir (source, "cfgs", gamePath.getChars(), "cfgs");
   copyDir (source, "giddi", gamePath.getChars(), "giddi");
   copyDir (source, "smk", gamePath.getChars(), "smk");
   copyDir (source, "VFX", gamePath.getChars(), "VFX");
   copyDir (source, "Windll", gamePath.getChars(), "windll");
   copyDir (getExecDir(), "INSTALL\\MW2-Dll42", gamePath.getChars(), "");

   copyFile (source, "HELP.EXE", gamePath.getChars(), "HELP.EXE");
   copyFile (source, "README.TXT", gamePath.getChars(), "README.TXT");
   copyFiles (source, "*.CNT", gamePath.getChars());
   copyFiles (source, "*.HLP", gamePath.getChars());
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");

   BGString gameDirVar = "GamePath=";
   gameDirVar += gamePath;
   BGFilePath<TCHAR> archive1Path = gamePath.getChars();
   BGFilePath<TCHAR> archive2Path = archive1Path;
   archive1Path += "DATABASE.MW2";
   archive2Path += "MW2.PRJ";

   if (CDtype == MW2_31stcc_BP) {
      copyDir (source, "MW2", gamePath.getChars(), "");
      copyFile (source, "ARCHJF.MW2", gamePath.getChars(), "ARCHJF.MW2");
      copyFile (source, "ARCHWO.MW2", gamePath.getChars(), "ARCHWO.MW2");
      mkdir (gamePath.getChars(), "Missions");
      copyDir (source, "Missions", gamePath.getChars(), "Missions");
      copyDir (getExecDir(), "install\\MW2-31stcc-bp", gamePath.getChars(), "");

      setWin95Mode (gamePath.getChars(), "MW2WIN.exe", true);

      makeBATfile("_mw2bp.bat", gamePath.getChars(), "mw2win.exe", NULL,
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility 
         | makeBATfile_RunAsAdmin | makeBATfile_cdWithQuotes);
      addKeyValuePair("GameName", "MW2: 31st Century Combat (BattlePack)");
      addKeyValuePair("StartMethod", "RunNative");
      BGString exeFN = gamePath;
      exeFN += OSPathSep;
      exeFN += "_mw2bp.bat";
      addKeyValuePair("compatibility", "Win95");
      addKeyValuePair("GamePath", gamePath.getChars());
      addKeyValuePair("ExecutableFN", exeFN.getChars());
      addKeyValuePair("", "");

      if (experimentalFeaturesEnabled()) {
         addKeysForMixedNativeGame("MW2: 31st Century Combat, Clan Jade Falcon (BattlePack, MechVM shell + native Win32 sim)",
            gamePath.getChars(), "campaign-CJF.xml", "_mw2bp.bat", "1", "GBL");
         addKeysForMixedNativeGame("MW2: 31st Century Combat, Clan Wolf (BattlePack, MechVM shell + native Win32 sim)",
            gamePath.getChars(), "campaign-WC.xml", "_mw2bp.bat", "1", "GBL");
      }

   } else {
      copyDir (source, "GBL", gamePath.getChars(), "");
      mkdir (gamePath.getChars(), "giddiwin");
      copyDir (source, "giddiwin", gamePath.getChars(), "giddiwin");
      setWin95Mode (gamePath.getChars(), "GBLWIN.exe", true);

      makeBATfile("_gblbp.bat", gamePath.getChars(), "gblwin.exe", NULL,
         makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
         makeBATfile_RunAsAdmin | makeBATfile_cdWithQuotes);

      addKeyValuePair("GameName", "MW2: Ghost Bear's Legacy (BattlePack)");
      addKeyValuePair("StartMethod", "RunNative");
      BGString exeFN = gamePath;
      exeFN += OSPathSep;
      exeFN += "_gblbp.bat";
      addKeyValuePair("compatibility", "Win95");
      addKeyValuePair("GamePath", gamePath.getChars());
      addKeyValuePair("ExecutableFN", exeFN.getChars());
      addKeyValuePair("", "");

      if (experimentalFeaturesEnabled()) {
         addKeysForMixedNativeGame("MW2: Ghost Bear's Legacy (BattlePack, MechVM shell + native Win32 sim)",
            gamePath.getChars(), "campaign-gbl.xml", "_gblbp.bat", "1", "GBL");
      }
   }

   patch42(gamePath.getChars(), "MW2.DLL");
   patch42(gamePath.getChars(), "battle.DLL");
   patch42(gamePath.getChars(), "MW2SHELL.DLL");
   patch42(gamePath.getChars(), "NETSHELL.DLL");
   createInstallReport(source, gamePath.getChars());
   displayMsg("Installation successful");
   return true;
}

////////////////////////////////////////////////////////////////////////////////

// Install 16-bit, hardware-accelerated version of Titanium 31st CC
bool installMW2_Titan_16bit (const TCHAR* source)
{
   BGString gamePath;
   getInstallPath (gamePath, "mw2-31stcc-tt");

   displayMsg ("Installing MW2: 31st Century Combat (Titanium Trilogy)");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "GIDDIWIN");
   mkdir (gamePath.getChars(), "MEK");
   mkdir (gamePath.getChars(), "Missions");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "plugin");

   // Copy files
   BGString installDir = source;
   installDir.appendNoDups(OSPathSep);
   installDir += "Install";
   installDir.appendNoDups(OSPathSep);

   BGString dataDir = installDir;
   dataDir.appendNoDups(OSPathSep);
   dataDir += "data";
   copyDir (dataDir.getChars(), "", gamePath.getChars(), "");
   copyDir (dataDir.getChars(), "cfgs", gamePath.getChars(), "CFGS");
   copyDir (dataDir.getChars(), "giddi", gamePath.getChars(), "GIDDI");
   copyDir (dataDir.getChars(), "giddiwin", gamePath.getChars(), "GIDDIWIN");
   copyDir (dataDir.getChars(), "smk", gamePath.getChars(), "SMK");

   BGString dir16 = installDir;
   dir16.appendNoDups(OSPathSep);
   dir16 += "16bit";
   copyDir (dir16.getChars(), "", gamePath.getChars(), "");
   copyDir (dir16.getChars(), "plugin", gamePath.getChars(), "plugin");
   copyDir (dir16.getChars(), "Missions", gamePath.getChars(), "Missions");

   #ifdef _WIN32
   copyDir (getExecDir(), "install\\MW2-31st-Titan", gamePath.getChars(), "");
   copyDir (getExecDir(), "install\\MW2-Dll42", gamePath.getChars(), "");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");
   copyFile (getExecDir(), "install\\tools\\_titanRes3.exe", gamePath.getChars(), "_titanRes3.exe");

   // Patch mw2shell.dll
   patch(gamePath.getChars(), "MW2shell.dll.diff", "MW2shell.dll");

   // Create registry key
   setWin95Mode (gamePath.getChars(), "MW2WIN.exe");
   #else
   // MW2.PRJ has capital letters in Titanium Trilogy ... change to lower case
   // for compatibility between Linux and the DOS CDs
   BGString FN1 = gamePath.getChars();
   FN1.appendNoDups(OSPathSep);
   FN1 += "MW2.PRJ";
   BGString FN2 = gamePath.getChars();
   FN2.appendNoDups(OSPathSep);
   FN2 += "mw2.prj";
   rename (FN1.getChars(), FN2.getChars());
   #endif

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   // Add entries for launching the game from MechVM
   makeBATfile("_mw2-31stcc.bat", gamePath.getChars(), "mw2win", NULL,
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_256Color | makeBATfile_cdWithQuotes);
   BGString exeFN = gamePath;
   exeFN += OSPathSep;
   exeFN += "_mw2-31stcc.bat";
   addKeysForNativeGame ("MW2: 31st Century Combat (Titanium Trilogy)",
      gamePath.getChars(), "_mw2-31stcc.bat");

   // 31stcc-TT does not support launching into missions directly!
   // GBL does, and GBL-TT's MW2.PRJ contains all 31stcc missions ...
   /*addKeysForMixedNativeGame("MW2: Jade Falcon Campaign (MechVM shell + Titanium sim)", 
      gamePath.getChars(), "campaign-CJF.xml", "_mw2.bat", "2", "GBL");
   addKeysForMixedNativeGame("MW2: Clan Wolf Campaign (MechVM shell + Titanium sim)", 
      gamePath.getChars(), "campaign-WC.xml", "_mw2.bat", "2", "GBL");*/

   createInstallReport(source, gamePath.getChars());
   return true;
}

// Install 8-bit, software-accelerated version (Battlepack)
bool installMW2_Titan_8bit (const TCHAR* source, int CDtype)
{
   BGString gamePath;
   if (CDtype == MW2_31stcc_TT_BP) {
      getInstallPath (gamePath, "mw2-31stcc-bp");
      displayMsg ("Installing MW2: 31st Century Combat (Battlepack/Titanium 8-bit)");
   } else if (CDtype == MW2_GBL_TT_BP) {
      getInstallPath (gamePath, "mw2-gbl-bp");
      displayMsg ("Installing MW2: Ghost Bear's Legacy (Battlepack/Titanium 8-bit)");
   } else {
      getInstallPath (gamePath, "mw2-mercs-bp");
      displayMsg ("Installing MW2: Mercenaries (Battlepack/Titanium 8-bit)");
   }

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "GIDDIWIN");
   mkdir (gamePath.getChars(), "help");
   mkdir (gamePath.getChars(), "keating");
   mkdir (gamePath.getChars(), "Missions");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "splash");
   mkdir (gamePath.getChars(), "Windll");

   // Copy files
   copyDir (source, "install\\data", gamePath.getChars(), "");
   copyDir (source, "Install\\data\\cfgs", gamePath.getChars(), "cfgs");
   copyDir (source, "Install\\data\\giddi", gamePath.getChars(), "giddi");
   copyDir (source, "Install\\data\\giddiwin", gamePath.getChars(), "giddiwin");
   copyDir (source, "install\\data\\smk", gamePath.getChars(), "smk");
   copyDir (source, "install\\8bit", gamePath.getChars(), "");
   copyDir (source, "help", gamePath.getChars(), "help");
   copyDir (source, "keating", gamePath.getChars(), "keating");
   copyDir (source, "Install\\8bit\\Missions", gamePath.getChars(), "Missions");
   copyDir (source, "splash", gamePath.getChars(), "splash");
   copyDir (source, "windll", gamePath.getChars(), "windll");
   copyDir (getExecDir(), "install\\MW2-31stcc-bp", gamePath.getChars(), "");
   copyDir (getExecDir(), "INSTALL\\MW2-Dll42", gamePath.getChars(), "");

   if (CDtype == MW2_Mercs_TT_BP) {
      mkdir (gamePath.getChars(), "rmg");
      copyDir (source, "Install\\data\\rmg", gamePath.getChars(), "rmg");
   }

   // Mech.ico, copied from H:\Install\data, must be deleted, otherwise the game won't run
   const TCHAR* exeFN;
   if (CDtype == MW2_31stcc_TT_BP) {
      exeFN= "MW2WIN.exe";
   } else if (CDtype == MW2_GBL_TT_BP) {
      exeFN= "GBLWIN.exe";
   } else {
      exeFN= "MERCSWIN.exe";
   }
   BGFilePath<TCHAR> exePath = gamePath.getChars();
   exePath += exeFN;

   #ifdef _WIN32
   BGString mech2_ico_path = gamePath;
   mech2_ico_path += OSPathSep;
   mech2_ico_path += "Mech.ico";
   DeleteFile (mech2_ico_path.getChars());

   // Create registry key
   
   setWin95Mode (gamePath.getChars(), exeFN, true);
   #endif

   patch42(gamePath.getChars(), "MW2.DLL");
   patch42(gamePath.getChars(), "battle.DLL");
   patch42(gamePath.getChars(), "MW2SHELL.DLL");
   patch42(gamePath.getChars(), "NETSHELL.DLL");

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());

   // Add entries for launching the game from MechVM
   makeBATfile("_mw2.bat", gamePath.getChars(), exePath.getChars(), NULL,
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_256Color | makeBATfile_cdWithQuotes);

   if (CDtype == MW2_31stcc_TT_BP) {
      addKeysForNativeGame ("MW2: 31st Century Combat (Battlepack/Titanium 8-bit)",
         gamePath.getChars(), "_mw2.bat");
   } else if (CDtype == MW2_GBL_TT_BP) {
      addKeysForNativeGame ("MW2: Ghost Bear's Legacy (Battlepack/Titanium 8-bit)",
         gamePath.getChars(), "_mw2.bat");
   } else {
      addKeysForNativeGame ("MW2: Mercenaries (Battlepack/Titanium 8-bit)",
         gamePath.getChars(), "_mw2.bat");
   }

   createInstallReport(source, gamePath.getChars());

   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   return true;
}

////////////////////////////////////////////////////////////////////////////////

bool installGBL_Titan (const TCHAR* source)
{
   BGString gamePath;
   getInstallPath (gamePath, "mw2-gbl-tt");

   displayMsg ("Installing MW2: Ghost Bear's Legacy (Titanium Trilogy)");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "GIDDIWIN");
   mkdir (gamePath.getChars(), "MEK");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "plugin");

   // Copy files
   BGString installDir = source;
   installDir.appendNoDups(OSPathSep);
   installDir += "Install";
   installDir.appendNoDups(OSPathSep);

   BGString dataDir = installDir;
   dataDir.appendNoDups(OSPathSep);
   dataDir += "data";
   copyDir (dataDir.getChars(), "", gamePath.getChars(), "");
   copyDir (dataDir.getChars(), "cfgs", gamePath.getChars(), "CFGS");
   copyDir (dataDir.getChars(), "giddi", gamePath.getChars(), "GIDDI");
   copyDir (dataDir.getChars(), "giddiwin", gamePath.getChars(), "GIDDIWIN");
   copyDir (dataDir.getChars(), "smk", gamePath.getChars(), "SMK");

   BGString dir16 = installDir;
   dir16.appendNoDups(OSPathSep);
   dir16 += "16bit";
   copyDir (dir16.getChars(), "", gamePath.getChars(), "");
   copyDir (dir16.getChars(), "plugin", gamePath.getChars(), "plugin");

   copyDir (getExecDir(), "install\\MW2-GBL-Titan", gamePath.getChars(), "");
   copyDir (getExecDir(), "install\\MW2-Dll42", gamePath.getChars(), "");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");
   copyFile (getExecDir(), "install\\tools\\_titanRes3.exe", gamePath.getChars(), "_titanRes3.exe");

   // Create registry key
   //setWin95Mode (gamePath.getChars(), "gblwin.exe");

   // Patch mw2shell.dll
   #ifdef _WIN32
   patch(gamePath.getChars(), "MW2shell.dll.diff", "MW2shell.dll");
   patch(gamePath.getChars(), "MercsW.dll.diff", "MercsW.dll");
   #else
   // MW2.PRJ has capital letters in Titanium Trilogy ... for compatibility
   // Linux and the DOS CDs, change to lower case
   BGString FN1 = gamePath.getChars();
   FN1.appendNoDups(OSPathSep);
   FN1 += "MW2.PRJ";
   BGString FN2 = gamePath.getChars();
   FN2.appendNoDups(OSPathSep);
   FN2 += "mw2.prj";
   rename (FN1.getChars(), FN2.getChars());
   #endif

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   // Create keys for running the 31stc missions
   // Needs to find 31stcc's DataBase.MW2
   Config* config = getConfig();
   BGFilePath<TCHAR> database_31stcc;
   int line = config->findValue ("MW2: 31st Century Combat (Titanium Trilogy)");
   if (line >= 0) {
      line = config->FindFirstKeyAfterLine (line, "GamePath");
      database_31stcc = config->getValue(line);
      database_31stcc += "database.mw2";
   } else {
      line = config->findValue ("MW2: 31st Century Combat, Clan Jade Falcon (MechVM shell + DosBox)");
      if (line >= 0) {
         line = config->FindFirstKeyAfterLine (line, "archive");
         database_31stcc = config->getValue(line);
      } else {
         line = config->findValue ("MW2: 31st Century Combat (Win95, native)");
         if (line >= 0) {
            line = config->FindFirstKeyAfterLine (line, "GamePath");
         database_31stcc = config->getValue(line);
            database_31stcc += "database.mw2";
         } else
         line = config->getKeyCount();
      }
   }

   if (database_31stcc.getLength () > 0) {
      // Add keys for playing the Jade Falcon campaign using the GBL engine
      addKeyValuePair("GameName", 
         "MW2: 31st Century Combat - Jade Falcon Campaign (MechVM shell + Titanium sim)");
      addKeyValuePair("StartMethod", "InternalShell");
      addKeyValuePair("GamePath", gamePath.getChars());
      addKeyValuePair("CampaignFile", "campaign-CJF.xml");
      addKeyValuePair("SetShellVar", "SimType=native");
      addKeyValuePair("SetShellVar", "compatibility=Win95");
      addVarKey("mission", "0");
      addVarKey("persistent", "mission");
      addVarKey("MEKFileVersion", "2");
      addVarKey("weaponsLocker", "GBL");

      BGFilePath<TCHAR> MEKdir = gamePath.getChars();
      MEKdir += "mek";
      BGString MEKdirVar = "MEKdir=";
      MEKdirVar += MEKdir;
      addKeyValuePair("SetShellVar", MEKdirVar.getChars());

      addVarKey ("GamePath", gamePath.getChars());

      BGString execVar = "SimExec=";
      execVar += gamePath;
      execVar.appendNoDups(OSPathSep);
      execVar += "_mw2-gbl.bat";
      addKeyValuePair("SetShellVar", execVar.getChars());

      addKeyValuePair ("archive", database_31stcc.getChars());

      BGFilePath<TCHAR> archive2Path = gamePath.getChars();
      archive2Path += "mw2.prj";
      addKeyValuePair ("archive", archive2Path.getChars());
      addKeyValuePair("", "");

      // Add keys for playing the Wolf Clan campaign using the GBL engine
      addKeyValuePair("GameName",  
         "MW2: 31st Century Combat - Wolf Clan Campaign (MechVM shell + Titanium sim)");
      addKeyValuePair("StartMethod", "InternalShell");
      addKeyValuePair("GamePath", gamePath.getChars());
      addKeyValuePair("CampaignFile", "campaign-WC.xml");
      addKeyValuePair("SetShellVar", "SimType=native");
      addKeyValuePair("SetShellVar", "compatibility=Win95");
      addVarKey("mission", "0");
      addVarKey("persistent", "mission");
      addVarKey("MEKFileVersion", "2");
      addVarKey("weaponsLocker", "GBL");

      addKeyValuePair("SetShellVar", MEKdirVar.getChars());
      addVarKey ("GamePath", gamePath.getChars());
      addKeyValuePair("SetShellVar", execVar.getChars());
      addKeyValuePair ("archive", database_31stcc.getChars());
      addKeyValuePair ("archive", archive2Path.getChars());
      addKeyValuePair("", "");
   }

   // Create registry key
   // Seems to be required despite setting compatibility in BAT file, below
   setWin95Mode (gamePath.getChars(), "gblwin.exe");

   // Add entries for launching the game from MechVM
   makeBATfile("_mw2-gbl.bat", gamePath.getChars(), "gblwin", NULL,
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_256Color | makeBATfile_cdWithQuotes);
   BGFilePath<TCHAR> exeFN = gamePath.getChars();
   exeFN += "_mw2-gbl.bat";
   addKeysForNativeGame ("MW2: Ghost Bear's Legacy (Titanium Trilogy)",
      gamePath.getChars(), "_mw2-gbl.bat");

   //if (experimentalFeaturesEnabled()) {
      addKeysForMixedNativeGame("MW2: Ghost Bear's Legacy (MechVM shell + Titanium sim)", 
         gamePath.getChars(), "campaign-gbl.xml", "_mw2-gbl.bat", "2", "GBL");
   //}

   createInstallReport(source, gamePath.getChars());
   return true;
}

////////////////////////////////////////////////////////////////////////////////

bool installMercs_Titan (const TCHAR* source)
{
   BGString gamePath;
   getInstallPath (gamePath, "mw2-mercs-tt");
   displayMsg ("Installing MW2: Mercenaries (Titanium Trilogy)");

   // Create directories
   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "CFGS");
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "GIDDIWIN");
   mkdir (gamePath.getChars(), "MEK");
   mkdir (gamePath.getChars(), "SMK");
   mkdir (gamePath.getChars(), "plugin");
   mkdir (gamePath.getChars(), "RMG");
   mkdir (gamePath.getChars(), "Missions");

   // Copy files
   BGString installDir = source;
   installDir.appendNoDups(OSPathSep);
   installDir += "Install";
   installDir.appendNoDups(OSPathSep);

   BGString dataDir = installDir;
   dataDir.appendNoDups(OSPathSep);
   dataDir += "data";
   copyDir (dataDir.getChars(), "", gamePath.getChars(), "");
   copyDir (dataDir.getChars(), "cfgs", gamePath.getChars(), "CFGS");
   copyDir (dataDir.getChars(), "giddi", gamePath.getChars(), "GIDDI");
   copyDir (dataDir.getChars(), "giddiwin", gamePath.getChars(), "GIDDIWIN");
   copyDir (dataDir.getChars(), "RMG", gamePath.getChars(), "RMG");
   copyDir (dataDir.getChars(), "smk", gamePath.getChars(), "SMK");

   BGString dir16 = installDir;
   dir16.appendNoDups(OSPathSep);
   dir16 += "16bit";
   copyDir (dir16.getChars(), "", gamePath.getChars(), "");
   copyDir (dir16.getChars(), "plugin", gamePath.getChars(), "plugin");
   copyDir (dir16.getChars(), "Missions", gamePath.getChars(), "Missions");

   copyDir (getExecDir(), "install\\MW2-mercs-Titan", gamePath.getChars(), "");
   copyDir (getExecDir(), "install\\MW2-Dll42", gamePath.getChars(), "");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");
   copyFile (getExecDir(), "install\\tools\\_titanRes3.exe", gamePath.getChars(), "_titanRes3.exe");

   // Create registry key
   setWin95Mode (gamePath.getChars(), "MercsWin.exe");

   // Patch mw2shell.dll
   #ifdef _WIN32
   patch(gamePath.getChars(), "MW2shell.dll.diff", "MW2shell.dll");
   patch(gamePath.getChars(), "MercsW.dll.diff", "MercsW.dll");
   #else
   // MW2.PRJ has capital letters in Titanium Trilogy ... for compatibility
   // Linux and the DOS CDs
   BGString FN1 = gamePath.getChars();
   FN1.appendNoDups(OSPathSep);
   FN1 += "MW2.PRJ";
   BGString FN2 = gamePath.getChars();
   FN2.appendNoDups(OSPathSep);
   FN2 += "mw2.prj";
   rename (FN1.getChars(), FN2.getChars());
   #endif

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   makeBATfile("_mw2-mercs.bat", gamePath.getChars(), "mercswin.exe", NULL,
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_256Color | makeBATfile_cdWithQuotes);
   BGString exeFN = gamePath;
   exeFN += OSPathSep;
   exeFN += "_mw2-mercs.bat";
   addKeysForNativeGame ("MW2: Mercenaries (Titanium Trilogy)",
      gamePath.getChars(), "_mw2-mercs.bat");

   addKeysForMixedNativeGame("MW2: Mercenaries (MechVM shell + Titanium sim)", 
      gamePath.getChars(), "campaign-mercs.xml", "_mw2-mercs.bat", "2", "all");

   createInstallReport(source, gamePath.getChars());
   return true;
}

////////////////////////////////////////////////////////////////////////////////

/*bool install_MW2_GBL_PE (const TCHAR* source)
{
   BGString gamePath;
   getInstallPath (gamePath, "mw2-gbl-pe");
   displayMsg ("Installing MW2: Ghost Bear's Legacy (Pentium Edition)");

   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "MEK");

   copyDir (source, "GBL", gamePath.getChars(), "");
   copyDir (source, "GIDDI", gamePath.getChars(), "GIDDI");
   copyDir (getExecDir(), "INSTALL\\MW2-GBL-PE", gamePath.getChars(), "");
   copyDir (getExecDir(), "INSTALL\\MW2-Dll42", gamePath.getChars(), "");

   copyFile (source, "MW2.DLL", gamePath.getChars(), "MW2.DLL");
   copyFile (source, "MW2SHELL.DLL", gamePath.getChars(), "MW2SHELL.DLL");
   copyFile (source, "SMACKW32.DLL", gamePath.getChars(), "SMACKW32.DLL");
   copyFile (source, "WAIL32.DLL", gamePath.getChars(), "WAIL32.DLL");
   copyFile (source, "GBL\\GBLWin.exe", gamePath.getChars(), "GBLWin.exe");

   // Create registry key
   #ifdef _WIN32
   setWin95Mode (gamePath.getChars(), "GBLWin.exe");
   #endif

   patch42 (gamePath.getChars(), "MW2.DLL");
   patch42 (gamePath.getChars(), "MW2SHELL.DLL");

   addKeyValuePair("GameName", "MW2: Ghost Bear's Legacy (Win95, native)");
   addKeyValuePair("StartMethod", "RunNative");
   BGString exeFN = gamePath;
   exeFN += OSPathSep;
   exeFN += "gblwin.EXE";
   addKeyValuePair("compatibility", "Win95");
   addKeyValuePair("GamePath", gamePath.getChars());
   addKeyValuePair("ExecutableFN", exeFN.getChars());
   addKeyValuePair("", "");

   if (experimentalFeaturesEnabled()) {
      addKeyValuePair("GameName", "MW2: Ghost Bear's Legacy (MechVM shell + native Win95 sim)");
      addKeyValuePair("StartMethod", "InternalShell");
      addKeyValuePair("GamePath", gamePath.getChars());
      addKeyValuePair("CampaignFile", "campaign-31stcc.xml");
      addKeyValuePair("SetShellVar", "SimType=internal");
      addKeyValuePair("SetShellVar", "compatibility=Win95");
   }

   BGString simExec = "SimExec=\"";
   simExec += exeFN;
   simExec += "\"";
   addKeyValuePair("SetShellVar", simExec.getChars());

   BGFilePath archivePath = gamePath.getChars();
   archivePath += "DATABASE.MW2";
   addKeyValuePair("archive", archivePath.getChars());
   addKeyValuePair("", "");

   createInstallReport(source, gamePath.getChars());
   displayMsg ("Installation complete");
   return true;
}*/

////////////////////////////////////////////////////////////////////////////////
// Single installer for MW2:31stCC ATI, Matrox and 3DFX editions
// (ATI: untested)

bool installMW2_31stCC_3D_enhanced (const TCHAR* source, int CDtype)
{
   BGString gamePath;
   const char* gamePathPart, * editionName;

   switch (CDtype) {
      case MW2_31stcc_ATI:
         gamePathPart = "mw2-31stcc-ati";
         editionName = "MW2: 31st Century Combat (ATI edition)";
         break;
      case MW2_31stcc_Matrox:
         gamePathPart = "mw2-31stcc-matrox";
         editionName = "MW2: 31st C entury Combat (Matrox edition)";
         break;
      case MW2_31stcc_3DFX:
         gamePathPart = "mw2-31stcc-3dfx";
         editionName = "MW2: 31st Century Combat (3DFX edition)";
         break;
      default:
         return false;
   }

   BGString msg = "Installing ";
   msg += editionName;
   displayMsg (msg.getChars());
   getInstallPath (gamePath, gamePathPart);

   bgmkdir (gamePath.getChars());
   mkdir (gamePath.getChars(), "GIDDI");
   mkdir (gamePath.getChars(), "MEK");

   copyFiles (source, "*.MW2", gamePath.getChars());
   copyFiles (source, "*.bwd", gamePath.getChars());
   copyFiles (source, "about.*", gamePath.getChars());
   copyFiles (source, "tech.*", gamePath.getChars());
   copyFiles (source, "MW2*", gamePath.getChars());

   copyDir (source, "GIDDI", gamePath.getChars(), "GIDDI");
   //copyDir (source, "MECH2", gamePath.getChars(), "");
   //copyDir (getExecDir(), "INSTALL\\MW2-GBL-PE", gamePath.getChars(), "");
   copyDir (getExecDir(), "INSTALL\\MW2-Dll42", gamePath.getChars(), "");
   copyFile (getExecDir(), "install\\tools\\_farPatcher.exe", gamePath.getChars(), "_farPatcher.exe");

   copyFile (source, "mech2.ex_", gamePath.getChars(), "Mech2.exe");
   copyFile (source, "mw2.prj", gamePath.getChars(), "mw2.prj");
   copyFile (source, "mw2.dll", gamePath.getChars(), "MW2.DLL");
   copyFile (source, "mw2shell.dll", gamePath.getChars(), "MW2SHELL.DLL");
   copyFile (source, "smackw32.dll", gamePath.getChars(), "SMACKW32.DLL");
   copyFile (source, "netmechw.dll", gamePath.getChars(), "NETMECHW.DLL");
   copyFile (source, "wail32.dll", gamePath.getChars(), "WAIL32.DLL");
   copyFile (source, "skygnd.par", gamePath.getChars(), "SKYGND.PAR");
   copyFile (source, "gamekey.map", gamePath.getChars(), "GAMEKEY.MAP");
   copyFile (source, "input.map", gamePath.getChars(), "input.map");
   copyFile (source, "input.bak", gamePath.getChars(), "input.bak");
   copyFile (source, "readme.txt", gamePath.getChars(), "readme.txt");
   copyFile (source, "symlog.txt", gamePath.getChars(), "symlog.txt");

   if (CDtype == MW2_31stcc_Matrox) {
      copyFile (source, "lod.par", gamePath.getChars(), "lod.par");
      copyFile (source, "mystique.par", gamePath.getChars(), "mystique.par");
   }

   // Create registry key
   setWin95Mode (gamePath.getChars(), "mech2.exe");

   patch42 (gamePath.getChars(), "MW2.DLL");
   patch42 (gamePath.getChars(), "MW2SHELL.DLL");
   patch42 (gamePath.getChars(), "NETMECHW.DLL");

   createInstallReport(source, gamePath.getChars());

   makeBATfile("_mw2.bat", gamePath.getChars(), "mech2.exe", NULL,
      makeBATfile_InGameDir | makeBATfile_Win95Compatibility | 
      makeBATfile_RunAsAdmin | makeBATfile_256Color);
   BGString exeFN = gamePath;
   exeFN += OSPathSep;
   exeFN += "_mw2-mercs.bat";
   addKeysForNativeGame (editionName, gamePath.getChars(), "_mw2.bat");

   displayMsg("Installation complete");
   return true;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
bool installNetMech()
{
   BGFilePath<TCHAR> source, gamePath;
   getInstallPath (gamePath, "NetMech");

   if (!getOpenFileName(source)) {
      displayMsg("Installation aborted");
      return false;
   }

   BGString cmdLine = getExecDir();
   cmdLine.appendNoDups(OSPathSep);
   cmdLine += "unzip.exe -o ";
   cmdLine += source;
   char* cmdLineChars = new char[cmdLine.getLength()+1];
   memcpy (cmdLineChars, cmdLine.getChars(), cmdLine.getLength()+1);

   // Create process for decompression
   bgmkdir (gamePath.getChars());
   PROCESS_INFORMATION processInfo;
   ZeroMemory(&processInfo, sizeof(processInfo));
   STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   BOOL processCreated = 
      CreateProcess (NULL, cmdLineChars, NULL, NULL, false, 
                     NORMAL_PRIORITY_CLASS, NULL, gamePath.getChars(), &startInfo, &processInfo);
   if (processCreated) {
      // Wait for decompressor to complete
      DWORD exitCode;
      do {
         if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
            displayMsg ("Unable to obtain process status for decompressor\n");
            return false;
         }
      } while (exitCode == STILL_ACTIVE);

      printf ("Decompressor ended with code %i\n", exitCode);
   }

   delete cmdLineChars;

   // Import mechs
   displayMsg("Installation successful, now importing mechs");
   size_t importedMechs = importMW2Files (gamePath.getChars());
   BGString str = "Installation complete, ";
   str.appendUInt(importedMechs);
   str += " mechs imported";
   displayMsg(str.getChars());

   makeBATfile("netmech.bat", "NetMech", "netmech.exe", NULL, 0);
   addKeysForDosBox("NetMech", "netmech.bat");
   createInstallReport(source.getChars(), gamePath.getChars());

   const char * NetMechProfileNames[] =
      {"NetMechLab (C1 - Energy Weapons)",
       "NetMechLab (C4 - Limited Ballistic and Missile Weapons)",
       "NetMechLab (C5 - Limited Missile Weapons)",
       "NetMechLab (C6 - Unlimited in design)"};
   const char * NetMechProfiles[] =
      {"nmc1", "nmc4", "nmc5", "31stcc"};

   for (size_t i = 0; i < 4; ++i) {
      addKeyValuePair("GameName", NetMechProfileNames[i]);
      addKeyValuePair("StartMethod", "InternalShell");
      addKeyValuePair("CampaignFile", "campaign-NetMechLab.xml");

      addVarKey("GamePath", "SetShellVar");
      addVarKey("MEKFileVersion", "0");

      BGFilePath<TCHAR> MEKdir = gamePath;
      MEKdir += "mek";
      addVarKey("MEKdir", MEKdir.getChars());
      addVarKey("weaponsLocker", NetMechProfiles[i]);

      BGFilePath<TCHAR> archivePath = gamePath;
      archivePath += "net.prj";
      addKeyValuePair("archive", archivePath.getChars());
      addKeyValuePair("", "");
   }

   return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
bool installMW3 (const TCHAR* source)
{
   BGString gamePath;
   getInstallPath (gamePath, "mw3");

   BGString cmdLine = getExecDir();
   cmdLine.appendNoDups(OSPathSep);
   cmdLine += "i5comp\\i5comp.exe x ";
   cmdLine += source;
   cmdLine.appendNoDups(OSPathSep);
   cmdLine += "data1.cab";
   char* cmdLineChars = new char[cmdLine.getLength()+1];
   memcpy (cmdLineChars, cmdLine.getChars(), cmdLine.getLength()+1);

   // Create process for decompression
   bgmkdir (gamePath.getChars());
   PROCESS_INFORMATION processInfo;
   ZeroMemory(&processInfo, sizeof(processInfo));
   STARTUPINFO startInfo;
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   BOOL processCreated = 
      CreateProcess (NULL, cmdLineChars, NULL, NULL, false, 
                     NORMAL_PRIORITY_CLASS, NULL, gamePath.getChars(), &startInfo, &processInfo);
   if (processCreated) {
      // Wait for decompressor to complete
      DWORD exitCode;
      do {
         if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
            displayMsg ("Unable to obtain process status for decompressor\n");
            return false;
         }
      } while (exitCode == STILL_ACTIVE);

      printf ("Decompressor ended with code %i\n", exitCode);

      BGString importXML = getExecDir();
      importXML.appendNoDups(OSPathSep);
      importXML += "mw3-mechs.xml";
      displayMsg ("Importing from MechWarrior 3");
      MW3MechImporter (gamePath.getChars(), 
                       getConfig()->getMechPath(), 
                       importXML.getChars());
      displayMsg ("Done");

      // Create registry key
      setWin95Mode (gamePath.getChars(), "mech3.exe");

      // to do: more registry keys needed to run this smoothly ...
      /*const TCHAR keyPath[] = "HKEY_CURRENT_USER\Software\MicroProse\MechWarrior 3\1.0";
      BGString subKey = path;
      subKey += "\\";
      subKey += exeName;
      HKEY hKey;
      const TCHAR value[] = "WIN95";
      RegCreateKeyEx (HKEY_CURRENT_USER, keyPath, 0, 0, 
                      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, 0);	

      BGString value = "DefaultCtlConfg"
      RegSetValueEx (hKey, "ActiveCtlCfg", 0, REG_SZ, value.getChars(), 
                     value.getLength()+1);
      RegSetValueEx (hKey, "CDVolume", 0, REG_Binary, 0x0000003f);

      RegCloseKey (hKey);*/
      return true;

   } else {
      displayMsg ("Installing MW3 failed: Failed to start decompressor\n");
      return false;
   }

   createInstallReport(source, gamePath.getChars());
   delete cmdLineChars;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Determine type of CD to install from

int getCDType (const TCHAR* source)
{
   // Check for MW3 CDs
   BGFilePath<TCHAR> MW3Data1Cab = source;
   MW3Data1Cab += "data1.cab";
   if (fileExists (MW3Data1Cab.getChars())) {
      INT64 cabSize = getFileSize (MW3Data1Cab.getChars());
      switch (cabSize) {
         case 197377050: return MW3_CD_eng;
         case 194477962: return MW3_CD_ger;
         case 189090233: return MW3_PM_CD;
      }
   }

   // Check for MW3 installs
   BGFilePath<TCHAR> MW3MechLibPath = source;
   MW3MechLibPath += "zbd";
   MW3MechLibPath += "mechlib.zbd";
   if (fileExists (MW3MechLibPath.getChars())) {
      INT64 mechlibSize = getFileSize (MW3MechLibPath.getChars());
      switch (mechlibSize) {
         case 2406873: return MW3_eng_installed; // mech3.exe 1.1.1.21
         //case 2406873: return MW3_ger_installed; //
         case 3073867: return MW3_PM_installed; // mech3.exe 1.0.11.5
      }
   }

   BGString PRJInMainDir = source;
   PRJInMainDir += OSPathSep;
   PRJInMainDir += "mw2.prj";
   switch (getFileSize (PRJInMainDir.getChars())) {
      case 43499380: return MW2_31stcc_3DFX;
      case 46409421: return MW2_31stcc_ATI;
      case 58771999: return MW2_31stcc_Matrox;
      case 28897985: return MW2_31stcc_PE;
   }

   if (directoryExists (source, "Windll")) {
      printf ("WinDLL directory found\n");
      BGFilePath<TCHAR> str = source;
      str += "mercs";
      str += "MW2.PRJ";
      INT64 mw2_prj_size = getFileSize(str.getChars());
      printf ("Found Mercs\\MW2.PRJ with %i bytes\n", mw2_prj_size); // debug!!!
      if (mw2_prj_size == 41479843) {
         printf ("MW2 Mercs 1.05 with WinDLL detected\n"); // debug!!!
         return MW2_Mercs_DOS; // Version 1.05 with WinDLL directory
      }

      // Battlepack GBL CD?
      str = source;
      str += "GBL";
      str += "MW2.PRJ";
      if (getFileSize(str.getChars()) == 53579644) 
         return MW2_GBL_BP;
      else {
         // Make sure it is 31stcc BattlePack and not a rare Mercs edition
         // http://www.mech2.org/forum/viewtopic.php?f=8&t=918
         BGFilePath<TCHAR> str = source;
         str += "MW2";
         str += "MW2.PRJ";
         return MW2_31stcc_BP;
      }
   }

   // Check for Titanium
   BGString ttPath = source;
   ttPath.appendNoDups(OSPathSep);
   ttPath += "Install";
   ttPath.appendNoDups(OSPathSep);
   ttPath += "16bit";
   ttPath.appendNoDups(OSPathSep);
   BGString prjPath = ttPath;
   prjPath += "mw2.prj";
   ttPath += "GBLwin.exe";

   switch (getFileSize(prjPath.getChars())) {
      case 64024206:
         if (fileExists (ttPath.getChars())) {
            return MW2_GBL_TITAN;
         } else {
            return MW2_31stcc_TITAN;
         }
      case 51841737:
         return MW2_Mercs_TITAN;
   }

   BGFilePath<TCHAR> NETPRJinRoot = source;
   NETPRJinRoot += "NET.PRJ";
   if (fileExists (NETPRJinRoot.getChars())) {
      return MW2_NetMechInstall;
   }

   if (directoryExists(source, "gbl")) {
      // The size of MW2.PRJ is the same for several editions of GBL
      BGFilePath<TCHAR> GBLDir = source;
      GBLDir += "gbl";
      BGFilePath<TCHAR> GBLDOSEXE = GBLDir;
      GBLDOSEXE += "gbl.exe";
      BGFilePath<TCHAR> GBLWinExe = GBLDir;
      GBLWinExe += "gblwin.exe";

      bool GBLDosExeExists = fileExists (GBLDOSEXE.getChars());
      bool GBLWinExeExists = fileExists (GBLWinExe.getChars());

      if (GBLDosExeExists && GBLWinExeExists)
         return MW2_GBL_Hybrid_DOS_Win;
      else if (GBLDosExeExists)
         return MW2_GBL_DOS;
      else if (GBLWinExeExists)
         return MW2_GBL_PE;
   } else if (directoryExists(source, "mech2")) {
      BGFilePath<TCHAR> MW2PRJ = source;
      MW2PRJ += "mech2";
      MW2PRJ += "mw2.prj";
      INT64 MW2PRJsize = getFileSize (MW2PRJ.getChars());
      switch (MW2PRJsize) {
         case 22: 
            return MW2_31stcc_Hybrid_DOS_Win;
         case 21893380: 
            if (directoryExists(source, "netmech")) 
               return MW2_NetMech;
            else
               return MW2_31stcc_PE;
         case 19960257: 
         case 22590909:
            return MW2_31stcc_DOS;
      }
   } else if (directoryExists (source, "mercs")) {
      BGFilePath<TCHAR> MW2PRJ = source;
      MW2PRJ += "mercs";
      MW2PRJ += "mw2.prj";
      INT64 MW2PRJsize = getFileSize (MW2PRJ.getChars());
      switch (MW2PRJsize) {
         case 51841737: // 3DFX version
            return MW2_Mercs_3DFX;
         case 43200694: // German version
         case 41479843: // Normal english version
            return MW2_Mercs_DOS;
      }
   }

   return unknownCD;
}

////////////////////////////////////////////////////////////////////////////////
// Install method

bool install (int CDtype, const TCHAR* source, const TCHAR* imgFN)
{
   switch (CDtype) {
      case MW2_NetMech:
      case MW2_31stcc_DOS:
      case MW2_31stcc_PE:
      case MW2_31stcc_Hybrid_DOS_Win:
         return install_MW2_31stcc_DOS(source, CDtype, imgFN);
      //case MW2_31stcc_PE:
      //   return install_31stCC_PE (source);
      case MW2_GBL_DOS:
      case MW2_GBL_PE:
      case MW2_GBL_Hybrid_DOS_Win:
         return installGBL (source, CDtype, imgFN);
      case MW2_Mercs_DOS:
      case MW2_Mercs_3DFX:
         return installMercs_DOS (source, imgFN);
      case MW2_31stcc_TITAN:
         return installMW2_Titan_16bit (source);
      case MW2_GBL_TITAN:
         return installGBL_Titan (source);
      case MW2_Mercs_TITAN:
         return installMercs_Titan (source);
      case MW2_31stcc_BP:
         return install_MW2_BattlePack (source, CDtype, "mw2-31stcc-bp");
      case MW2_GBL_BP:
         return install_MW2_BattlePack (source, CDtype, "mw2-gbl-bp");
         //case MW2_Mercs_Win:
      case MW2_31stcc_TT_BP:
      case MW2_GBL_TT_BP:
      case MW2_Mercs_TT_BP:
         return installMW2_Titan_8bit (source, CDtype);
      case MW2_31stcc_ATI:
      case MW2_31stcc_Matrox:
      case MW2_31stcc_3DFX:
         return installMW2_31stCC_3D_enhanced (source, CDtype);
      //case MW2_GBL_TT_BP:
      //   install_MW2_GBL_PE (source, target);
      //   break;
      #ifdef _WIN32
      case MW3_CD_ger:
      case MW3_CD_eng:
         return installMW3 (source);
      #endif
      default:

         messageBox("MechVM", "Unknown CD");
         return false;
   }
}

/*void appendInstallPath (BGString& str, int cdType)
{
   switch (cdType) {
      //case unknownCD:
      //   str += "";
      //   return;
      case MW2_31stcc_DOS:
         str += "mw2-31stcc-dos";
         return;
      case MW2_GBL_DOS:
         str += "mw2-gbl-dos";
         return;
      case MW2_Mercs_DOS:
         str += "mw2-mercs-dos";
         return;
      case MW2_31stcc_TITAN:
         str += "mw2-31stcc-tt";
         return;
      case MW2_GBL_TITAN:
         str += "mw2-gbl-tt";
         return;
      case MW2_Mercs_TITAN:
         str += "mw2-mercs-tt";
         return;
      case MW2_NetMechInstall:
         str += "netmech";
         return;
      case MW2_31stcc_PE:
         str += "mw2-31stcc-pe";
         return;
      case MW2_GBL_PE:
         str += "mw2-gbl-pe";
         return;
      case MW2_31stcc_BP:
         str += "mw2-31stcc-bp";
         return;
      case MW2_31stcc_Matrox:
         str += "mw2-31stcc-matrox";
         return;
      case MW2_31stcc_3DFX:
         str += "mw2-31stcc-3dfx";
         return;
      case MW2_31stcc_ATI:
         str += "mw2-31stcc-ati";
         return;
      case MW2_Mercs_3DFX:
         str += "mw2-31stcc-3dfx";
         return;
      //case MW2_Mercs_Win:
         //str += "mw2-31stcc-win";
         //return;
      case MW2_31stcc_TT_BP:
         str += "mw2-31stcc-bp";
         return;
      case MW2_GBL_TT_BP:
         str += "mw2-gbl-bp";
         return;
      case MW2_Mercs_TT_BP:
         str += "mw2-mercs-bp";
         return;
      case MW2_NetMech:
         str += "mw2-netmech";
         return;
      case MW3_CD_eng:
      case MW3_CD_ger:
      case MW3_eng_installed:
      case MW3_ger_installed:
         str += "mw3";
         return;
      case MW3_PM_CD:
      case MW3_PM_installed:
         str += "mw3-pm";
         return;
      default:
         return;
   }
}*/

const char* getCDName (int CDID)
{
   switch (CDID) {
   case MW2_31stcc_DOS:
      return "MW2: 31st Century Combat (DOS)";
   case MW2_31stcc_PE:
      return "MW2: 31st Century Combat (Pentium Edition)";
   case MW2_31stcc_Hybrid_DOS_Win:
      return "MW2: 31st Century Combat (DOS+Win)";
   case MW2_NetMech:
      return "MW2: 31st Century Combat (NetMech)";
   case MW2_NetMechInstall:
      return "MW2: 31st Century Combat (NetMech installed)";
   case MW2_31stcc_BP:
      return "MW2: 31st Century Combat (BattlePack)";
   case MW2_31stcc_TITAN:
      return "MW2: 31st Century Combat (Titanium Trilogy)";
   case MW2_31stcc_3DFX:
      return "MW2: 31st Century Combat (3DFX)";
   case MW2_31stcc_ATI:
      return "MW2: 31st Century Combat (ATI)";
   case MW2_31stcc_TT_BP:
      return "MW2: 31st Century Combat (Battlepack installed from Titanium Trilogy)";
   case MW2_31stcc_Matrox:
      return "MW2: 31st Century Combat (Matrox)";
   case MW2_GBL_DOS:
      return "MW2: Ghost Bear's Legacy (DOS)";
   case MW2_GBL_PE:
      return "MW2: Ghost Bear's Legacy (Pentium Edition)";
   case MW2_GBL_Hybrid_DOS_Win:
      return "MW2: Ghost Bear's Legacy (DOS+Win)";
   case MW2_GBL_BP:
      return "MW2: Ghost Bear's Legacy (Battlepack installed from Titanium Trilogy)";
   case MW2_GBL_TT_BP:
      return "MW2: Ghost Bear's Legacy (Battlepack installed from Titanium Trilogy)";
   case MW2_GBL_TITAN :
      return "MW2: Ghost Bear's Legacy (Titanium Trilogy)";
   case MW2_Mercs_DOS :
      return "MW2: Mercenaries (DOS)";
   case MW2_Mercs_3DFX :
      return "MW2: Mercenaries (3DFX)";
   case MW2_Mercs_TT_BP:
      return "MW2: Mercenaries (Battlepack installed from Titanium Trilogy)";
   case MW2_Mercs_TITAN:
      return "MW2: Mercenaries (Titanium)";
   case MW3_CD_eng:
      return "MW3";
   case MW3_CD_ger:
      return "MW3";
   case MW3_eng_installed:
      return "MW3 (installed)";
   case MW3_ger_installed:
      return "MW3 (installed)";
   case MW3_PM_CD:
      return "MW3: Pirate's Moon";
   case MW3_PM_installed:
      return "MW3: Pirate's Moon (installed)";
   default:
      return "Unknown";
   }
}
