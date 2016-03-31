@echo off
echo Setting up 32 Bit build environment
call "C:\Programme\Intel\Compiler\C++\9.1\IA32\Bin\iclvars.bat"
set PSDKPath="C:\Programme\Microsoft Platform SDK for Windows Server 2003 R2"

rem Copy libs
copy %psdkPath%\lib\user32.lib
copy %psdkPath%\lib\comdlg32.lib
copy %psdkPath%\lib\kernel32.lib
copy %psdkPath%\lib\uuid.lib
copy %psdkPath%\lib\winMM.lib
copy %psdkPath%\lib\comdlg32.lib
copy %psdkPath%\lib\shell32.lib
copy %psdkPath%\lib\openGL32.lib
copy %psdkPath%\lib\advapi32.lib
copy %psdkPath%\lib\gdi32.lib
copy %psdkPath%\lib\glu32.lib
copy ..\freeglut.*

rem Build
rem icl /I%PSDKPath%\Include /ID:\libs\pthreads-w32-2-8-0-release\include /GR /GX /EHa ..\src\Archive.cpp ..\src\BGBase.cpp ..\src\BGString.cpp ..\src\dialogs.cpp ..\src\FileCache.cpp ..\src\MechWarriorIIPRJ.cpp ..\TitanFarPatcher\TitanFarPatcher.cpp /FeMechVM /link comdlg32.lib kernel32.lib 
icl /I%PSDKPath%\Include /GR /GX /EHa ..\src\BGBase.cpp ..\src\BGString.cpp ..\src\dialogs.cpp ..\src\FileCache.cpp ..\TitanFarPatcher\TitanFarPatcher.cpp /Fe_farPatcher /link comdlg32.lib kernel32.lib 

rem Clean up
del *.obj
del *.lib
del *.dll

pause
