#ifndef Abstraction_H
#define Abstraction_H
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#ifdef UNICODE
typedef wchar_t TCHAR;
#pragma message "Producing unicode executable"
#else
typedef char TCHAR;
#endif
#endif
////////////////////////////////////////////////////////////////////////////////
// Constants
typedef short int16;
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
inline void bgmkdir (const char* newDir)
{ _mkdir (newDir); }
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
typedef int64_t INT64;
typedef u_int32_t DWORD;
typedef u_int16_t WORD;
inline void bgmkdir (const char* newDir)
{ mkdir (newDir, 0777); }
#endif

#ifdef _WIN32
typedef DWORD FileOffset;
#else
typedef size_t FileOffset;
#endif
typedef FileOffset CacheOffset;
typedef FILE* FileHandle;

// Operating system path separator
#ifdef WIN32
const char OSPathSep = '\\';
const TCHAR newLine[] = {0x0d, 0x0a};
#else
const char OSPathSep = '/';
const TCHAR newLine[] = "\n";
#endif

const TCHAR tabChar = 9;
const char ciphers[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
////////////////////////////////////////////////////////////////////////////////
// Low-level functions and OS abstractions

// Test if file exists
bool fileExists (const char* FN);

// Copy single file
void copyFile (const TCHAR* sourceFN, const TCHAR* targetFN);

#ifdef _WIN32
#include <direct.h>
#else
inline void mkdir (const TCHAR* newDir)
//{ CreateDirectory (newDir, NULL); } // WIN32 has mkdir
{ mkdir (newDir, 0777); }
#endif

#ifndef _WIN32
inline void DeleteFile (const TCHAR* fn)
{ remove (fn); }
#endif

// Return file size, given the file's name
INT64 getFileSize (const TCHAR* FN);

// Patch targetFN using a previously produced diffFile
bool patch(const TCHAR* diffFile, const TCHAR* targetFN);

// Delete all files in dirName
void deleteTree (const TCHAR* dirName);
#endif // end Abstraction_H
