////////////////////////////////////////////////////////////////////////////////
// FileCache.cpp
// Copyright Bjoern Ganster 2005-2011
////////////////////////////////////////////////////////////////////////////////
#include "abstraction.h"
#include "FileCache.h"

//#include <iostream>
#include <algorithm>
#include <limits>
#include <assert.h>
using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#define LINUX
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Choose between the functions below to enable or disable logging

//#define DebugOut printf
//#define DebugOut 

 void debugOut (const char*)
{
}

 void reportRange (FileOffset , FileOffset)
{
}

////////////////////////////////////////////////////////////////////////////////
// Elementary file operations

 FileHandle fileOpen (const TCHAR* FileName, bool readOnly)
{
   FileHandle newHandle;
   #ifdef UNICODE
      newHandle = _wfopen (FileName, L"r+b");
      if (newHandle == 0) {
         newHandle = _wfopen (FileName, L"w+b");
      }
   #else
   try {
      if (readOnly) {
         //newHandle = fopen (FileName, "rb");
         newHandle = fopen (FileName, "rb");
      } else {
         newHandle = fopen (FileName, "r+b");
         if (newHandle == 0) {
            newHandle = fopen (FileName, "w+b");
         }
      }
   } catch (...) {
      return 0;
   }
   #endif
   return newHandle; 
}

 void fileClose (FileHandle& file)
{ fclose (file); }

 size_t fileRead (FileHandle& file, void* buf, size_t BytesToRead)
{ return fread (buf, 1, BytesToRead, file); }

 size_t fileWrite (FileHandle& file, const void* buf, size_t BytesToWrite)
{ return fwrite (buf, 1, BytesToWrite, file); }

 bool fileSeek (FileHandle& file, size_t newPos, int)
{ 
   int res = fseek (file, (long) newPos, SEEK_SET);
   return (res == 0);
}

//#ifdef LINUX
 long _getFileSize (FileHandle& file)
{
   if (file != NULL) {
      long oldPos = ftell (file);
      fseek (file, 0, SEEK_END);
      long filesize = ftell (file);
      fseek (file, oldPos, SEEK_SET);
      return filesize;
   } else
      return 0;
}
//#endif

/*#ifdef WIN32
 long GetFileSize (FileHandle& file)
{
   DWORD FileSizeHi;
   DWORD m_FileSize = GetFileSize (file, &FileSizeHi);
   return m_FileSize + (FileSizeHi << 32);
}
#endif*/


void logError (const TCHAR* msg)
{
#ifdef _WIN32
   BGString str = msg;
   str += ": ";
   str.appendOSError();
   str.print();
#else
   printf (msg);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Constructor, destructor

FileCache::FileCache (FileOffset NewCacheSize)
: VirtualClass (FileCacheID)
{
   cache = NULL;
   m_CacheSize = 0;

   #ifdef WIN32
   m_FileHandle = INVALID_HANDLE_VALUE;
   #else
   m_FileHandle = 0;
   #endif

   m_FileSize = 0;
   m_isReadOnly = true;
   invalidateCache();
   setCacheSize (NewCacheSize);
}

FileCache::~FileCache ()
{
   setCacheSize (0);
   close();
}

////////////////////////////////////////////////////////////////////////////////
// Property setters

bool FileCache::setCacheStartPos (FileOffset newPos)
{
   if (!writeBackCache())
      return false;
   m_CacheStartPos = newPos;
   return fillCache();
}

bool FileCache::setCacheSize (FileOffset newSize)
{
   writeBackCache();
   if (newSize != 0) {
      char* NewMem = new char[newSize];
      if (NewMem != NULL) {
         if (cache  != NULL)
            delete cache;
         cache = NewMem;
         m_CacheSize = newSize;
         UsedCacheSize = 0;
         fillCache();
         return true;
      } else {
         cache = NULL;
         m_CacheSize = 0;
         return false;
      }
   } else {
      if (cache  != NULL)
         delete cache;
      cache = NULL;
      return true;
   }
}

/*bool FileCache::setFileHandle (FileHandle NewFileHandle)
{
   writeBackCache();
   if (CloseFileOnDestroy && _FileHandle != 0)
      fileClose (_FileHandle);
   _FileHandle = NewFileHandle;
   if (NewFileHandle != 0) {
      m_FileSize = ::getFileSize (_FileHandle);
   } else
      m_FileSize = 0;
   CloseFileOnDestroy = false;
   MustWriteCacheBack = false;
   return fillCache();
}*/

////////////////////////////////////////////////////////////////////////////////
// Invalidate Cache

void FileCache::invalidateCache()
{
   m_CacheStartPos = 0;
   UsedCacheSize = 0;
   MustWriteCacheBack = false;
}

////////////////////////////////////////////////////////////////////////////////
// Fill cache

bool FileCache::fillCache()
{
   if (m_CacheSize != 0
   &&  cache != NULL
   &&  isHandleValid())
   {
      writeBackCache();
      #ifdef _WIN32
      if (SetFilePointer (m_FileHandle, m_CacheStartPos, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
         logError ("Could not set file pointer: ");
      #else
      if (!fileSeek (m_FileHandle, m_CacheStartPos, 0)) {
         printf ("File seek failed\n");
         return false;
      }
      #endif

      CacheOffset toRead = min (m_CacheSize, CacheOffset (m_FileSize - m_CacheStartPos));
      #ifdef _WIN32
      if (ReadFile (m_FileHandle, cache, toRead, &UsedCacheSize, NULL) == 0)
         logError ("Read error: ");
      #else
      UsedCacheSize = fileRead (m_FileHandle, cache, toRead);
      #endif
      if (UsedCacheSize == toRead) {
         MustWriteCacheBack = false;
      } else {
         printf ("Read failure\n");
         return false;
      }
      return true;
   } else {
      invalidateCache();
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Write cache back to file

bool FileCache::writeBackCache ()
{
   if (m_CacheSize != 0
   &&  cache != NULL
   &&  isHandleValid()
   &&  MustWriteCacheBack
   &&  UsedCacheSize > 0)
   {
      #ifdef _WIN32
      if (SetFilePointer (m_FileHandle, m_CacheStartPos, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
         return false;    
      #else
      if (!fileSeek (m_FileHandle, m_CacheStartPos, 0))
         return false;
      #endif

      #ifdef _WIN32
      FileOffset BytesWritten;
      WriteFile(m_FileHandle, cache, UsedCacheSize, &BytesWritten, NULL);
      #else
      FileOffset BytesWritten = fileWrite (m_FileHandle, cache, UsedCacheSize);
      #endif
      MustWriteCacheBack = false;
      m_FileSize = max (m_FileSize,m_CacheStartPos + UsedCacheSize);
      return (BytesWritten == UsedCacheSize);
   } else
      return true; // need not write back, so operation was successful
}

////////////////////////////////////////////////////////////////////////////////
// Open, close file

bool FileCache::create (const TCHAR* FileName)
{
   m_isReadOnly = false;
   bool result = false;
   #ifdef _WIN32
   m_FileHandle = CreateFile (FileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, 
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (m_FileHandle == INVALID_HANDLE_VALUE) {
      BGString str = "Could not open ";
      str += FileName;
      str += ": ";
      logError(str.getChars());
   } else {
      m_FileSize = GetFileSize (m_FileHandle, NULL);
      CloseFileOnDestroy = true;
      result = true;
   }
   #else
   if (fileExists (FileName))
      remove (FileName);
   m_FileHandle = fileOpen (FileName, m_isReadOnly);
   if (m_FileHandle != NULL) {
      m_FileSize = _getFileSize(m_FileHandle);
      result = true;
      CloseFileOnDestroy = true;
   }
   #endif
   return result;
}

bool FileCache::openReadOnly (const TCHAR* FileName)
{
   m_isReadOnly = true;
   bool result = false;
   #ifdef _WIN32
   m_FileHandle = CreateFile (FileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (m_FileHandle == INVALID_HANDLE_VALUE) {
      BGString str = "Could not open ";
      str += FileName;
      logError(str.getChars());
   } else {
      m_FileSize = GetFileSize (m_FileHandle, NULL);
      CloseFileOnDestroy = true;
      result = true;
   }
   #else
   m_FileHandle = fopen (FileName, "rb");
   if (m_FileHandle != NULL) {
      m_FileSize = _getFileSize(m_FileHandle);
      result = true;
      CloseFileOnDestroy = true;
   }
   #endif
   return result;
}

bool FileCache::openReadWrite (const TCHAR* FileName)
{
   m_isReadOnly = false;
   #ifdef _WIN32
   m_FileHandle = CreateFile (FileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, 
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (m_FileHandle == INVALID_HANDLE_VALUE) {
      BGString str = "Could not open ";
      str += FileName;
      logError(str.getChars());
   } else {
      m_FileSize = GetFileSize (m_FileHandle, NULL);
   }
   #else
   m_FileHandle = fileOpen (FileName, m_isReadOnly);
   m_FileSize = _getFileSize (m_FileHandle);
   #endif
   CloseFileOnDestroy = true;
   return isHandleValid();
}

void FileCache::close()
{
   writeBackCache();
   if (CloseFileOnDestroy && isHandleValid()) {
      #ifdef _WIN32
      CloseHandle(m_FileHandle);
      m_FileHandle = INVALID_HANDLE_VALUE;
      #else
      fileClose (m_FileHandle);
      m_FileHandle = 0;
      #endif
   }

   // Reset FileCache in case it is reused
   m_CacheStartPos = 0;
   m_CacheSize = 0;
   m_FileSize = 0;
   UsedCacheSize = 0;
}

////////////////////////////////////////////////////////////////////////////////

bool memCopy (const char* source, char* target, FileOffset sourceOffs, 
              FileOffset targetOffs, FileOffset count)
{
   try {
      for (DWORD i = 0; i < count; i++)
         target[i+targetOffs] = source [i+sourceOffs];
      return true;
   } catch (...) {
      debugOut ("memCopy failed\n");
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Read bytes from file

bool FileCache::read (FileOffset offset, char* buf, FileOffset count)
{
   FileOffset BytesRead;
   if (m_FileHandle != 0) {
      if (count <= m_CacheSize) {
         if (offset < m_CacheStartPos
         ||  offset + count > m_CacheStartPos + UsedCacheSize) 
         {
            // Read miss in cache
            if (!setCacheStartPos (offset))
               return false;
            if (UsedCacheSize >= count) {
               return memCopy (cache, buf, offset - m_CacheStartPos, 0, count);
            } else
               return false;
         } else {
            // Read hit in cache
            return memCopy (cache, buf, offset - m_CacheStartPos, 0, count);
         }
      } else {
         // Amount to read is larger than cache
         writeBackCache();
         #ifdef _WIN32
         if (SetFilePointer (m_FileHandle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
            return false;    
         #else
         if (!fileSeek (m_FileHandle, offset, 0))
            return false;
         #endif
         #ifdef _WIN32
         ReadFile (m_FileHandle, buf, count, &BytesRead, NULL);
         #else
         BytesRead = fileRead (m_FileHandle, buf, count);
         #endif
         UsedCacheSize = 0;
         return (BytesRead == count);
      }
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Read a line from a file - will skip empty lines
// Returns false at end of file

bool FileCache::readLine (FileOffset& offset, BGString& line)
{
   line.clear();
   char c;
   bool done = false;
   bool skipEOLs = false;

   // Read chars until an EOL is encountered
   while (!done) {
      bool success = read (offset, &c, 1);
      if (success) {
         if (c == 0x0a || c == 0x0d) {
            done = true;
            skipEOLs = true;
         } else {
            line += c;
            offset++;
         }
      } else
         done = true;
   }

   // Skip EOL chars
   char lastEOLChar = ' ';
   size_t EOLcharsRead = 0;
   while (skipEOLs) {
      bool success = read (offset, &c, 1);
      if (success) {
         if (c == 0x0a || c == 0x0d) {
            EOLcharsRead++;
            if (c == lastEOLChar || EOLcharsRead > 1)
               skipEOLs = false;
            offset++;
            lastEOLChar = c;
         } else
            skipEOLs = false;
      } else
         skipEOLs = false;
   }

   return (line.getLength() > 0 || EOLcharsRead > 0);
}

////////////////////////////////////////////////////////////////////////////////
// Write bytes to file

bool FileCache::enlargeFile (FileOffset NewSize)
{
   writeBackCache();
   #ifdef _WIN32
   if (SetFilePointer (m_FileHandle, m_FileSize, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
      return false;    
   #else
   if (!fileSeek (m_FileHandle, m_FileSize, 0))
      return false;
   #endif

   for (unsigned int i = 0; i < m_CacheSize; i++) 
      cache[i] = 0;
   while (m_FileSize < NewSize) {
      FileOffset written;
      size_t toWrite = NewSize - m_FileSize;
      if (toWrite > m_CacheSize)
         toWrite = m_CacheSize;
      #ifdef _WIN32
      WriteFile (m_FileHandle, cache, toWrite, &written, NULL);
      #else
      written = fileWrite (m_FileHandle, cache, toWrite);
      #endif
      m_FileSize += written;
      if (written != toWrite)
         return false;
   }
   return true;
}

bool FileCache::write (FileOffset offset, const char* buf, 
                       FileOffset count)
{
   assert (buf != NULL);
   if (!m_isReadOnly) {
      // Test if we need to enlarge the file
      if (offset > m_FileSize) {
         if (!enlargeFile (offset))
            return false;
      }

      if (count < m_CacheSize) {
         if (offset == m_FileSize)
         {
            append (buf, count);
         } else
         // Treat write hits & misses
         if (offset < m_CacheStartPos
         ||  offset + count >= m_CacheStartPos + UsedCacheSize) 
         {
            // Cache write miss
            setCacheStartPos (offset);
         }
         // Now we eiter have a write hit or this case can now be treated
         // like one
         MustWriteCacheBack = true;
         return memCopy (buf, cache, 0, offset - m_CacheStartPos, count);
      } else {
         // todo: Workaround for linux. Should not be necessary but fixes
         // problems when writing to newly created files
         if (offset + count > m_FileSize)
            enlargeFile (offset + count);

         // New data does not fit into cache, but has to be written directly
         #ifdef _WIN32
         SetFilePointer (m_FileHandle, offset, NULL, FILE_BEGIN);
         FileOffset BytesWritten;
         WriteFile (m_FileHandle, buf, count, &BytesWritten, NULL);
         #else
         if (!fileSeek (m_FileHandle, offset, 0))
            return false;
         size_t BytesWritten = fileWrite (m_FileHandle, buf, count);
         #endif

         // If write range overlaps with cached range, copy part of the cache
         // from written bytes
//         WriteBackCache();
//         UsedCacheSize = 0;
         copyBytesFromBuf (buf, offset, count);
         return (BytesWritten == count);
      }
   } else
      return false;
}

bool FileCache::append (const char* buf, FileOffset count)
{
   if (m_isReadOnly)
      return false;

   if (count > m_CacheSize) {
      // Append directly without caching
      writeBackCache();
      m_FileSize += count;

      // New data does not fit into cache, but has to be written directly
      #ifdef _WIN32
      SetFilePointer (m_FileHandle, 0, NULL, FILE_END);
      FileOffset BytesWritten;
      WriteFile (m_FileHandle, buf, count, &BytesWritten, NULL);
      #else
      if (!fileSeek (m_FileHandle, m_FileSize, 0))
         return false;
      size_t BytesWritten = fileWrite (m_FileHandle, buf, count);
      #endif
      return BytesWritten == count;
   } else {
      // Append to file using the cache
      if (m_FileSize + count > m_CacheStartPos + UsedCacheSize)
         if (!setCacheStartPos (m_FileSize))
            return false;
      MustWriteCacheBack = true;
      UsedCacheSize = m_FileSize + count - m_CacheStartPos;

      if (memCopy (buf, cache, 0, m_FileSize - m_CacheStartPos, count)) {
         m_FileSize = m_CacheStartPos + UsedCacheSize;
         return true;
      } else
         return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Flush file

void FileCache::flush ()
{
   writeBackCache();
   #ifndef _WIN32
   fflush (m_FileHandle);
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Copy bytes from buf, used when reads or writes request more data than fits
// into the cache

bool FileCache::copyBytesFromBuf (
   const char* buf, FileOffset offset, FileOffset count)
{
   if (offset > m_CacheStartPos
   &&  offset + count < m_CacheStartPos + UsedCacheSize)
   { 
      // buf lies entirely inside the cache
      debugOut ("a\n");
      MustWriteCacheBack = true;
      return memCopy (buf, cache, 0, offset - m_CacheStartPos, count);
   } else
   if (offset > m_CacheStartPos
   &&  offset < m_CacheStartPos + UsedCacheSize)
   {
      // Start of buf lies in cache
      debugOut ("b\n");
      MustWriteCacheBack = true;
      return memCopy (buf, cache, 0, offset - m_CacheStartPos, 
                      m_CacheStartPos + UsedCacheSize - offset);
   } else 
   if (offset + count > m_CacheStartPos
   &&  offset + count < m_CacheStartPos + UsedCacheSize)
   {
      // End of buf affects cache or cache lies entirely in buf
      debugOut ("c\n");
      MustWriteCacheBack = true;
      return memCopy (buf, cache, 0, 0,
                         offset + count - m_CacheStartPos);
   } else
      // Cache and written range do not overlap, trivial success
      return true;
}

////////////////////////////////////////////////////////////////////////////////
// Is given path absolute?

bool FileCache::isPathAbsolute (const char* path) 
{
   #ifdef WIN32
   if (strlen (path) > 3) {
      if (path[0] == '\"'
      &&  path[2] == ':'
      &&  path[3] == OSPathSep)
      {
         return true;
      }
   }

   if (strlen (path) > 2) {
      if (path[1] == ':'
      &&  path[2] == OSPathSep)
      {
         return true;
      }
   }
   return false;
   #else
      if (strlen(path) > 0) {
         return (path[0] == OSPathSep);
      } else
         return false;
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Path parsing

#ifdef WIN32

bool convertLetterToWindowsDriveNumber (char drvLetter, int& driveNum)
{
   if (drvLetter >= 'a'
   &&  drvLetter <= 'z')
   {
         driveNum = drvLetter - 'a' + 1;
         return true;
   } else
   if (drvLetter >= 'A'
   &&  drvLetter <= 'Z')
   {
         driveNum = drvLetter - 'A' + 1;
         return true;
   } else {
      return false; // Error: illegal drive character
   }
}

char getCurrWinDriveLetter()
{
   return _getdrive () + 'A' -1;
}

void getDrivePath (int driveNum, BGString& path)
{
   const char* drivePath = _getdcwd (driveNum, NULL, 0);
   path = drivePath;
   free ((void*) drivePath);
}

bool FileCache::parseFileName (const char* _path, BGString& absPath, 
                               BGString& FileName)
{
   // Convert unix path separators
   BGString path;
   size_t i;
   for (i = 0; i < strlen (_path); i++)
      if (_path[i] != '/')
         path += _path[i];
      else
         path += OSPathSep;

   // Preparse: make sure path is absolute
   if (path.getLength () == 0) {
      getDrivePath (_getdrive(), absPath);
      FileName = "";
      return true;
   } else if (path.getLength () == 1) {
      if (path[0] == OSPathSep) {
         absPath = getCurrWinDriveLetter();
         absPath += ":\\";
         FileName = "";
         return true;
      } else if (path[0] == '.') {
         getDrivePath (_getdrive(), absPath); 
         FileName = "";
         return true;
      } else {
         getDrivePath (_getdrive(), absPath); 
         absPath += OSPathSep;
         absPath += path[0];
         FileName = "";
         return true;
      }
   } else
   if (path.getLength() == 2) {
      if (path[1] == ':') {
         // path[0] denotes drive
         int drvNum = 0;
         if (convertLetterToWindowsDriveNumber (path[0], drvNum)) {
             getDrivePath (drvNum, absPath); 
             FileName = ""; 
             return true;
         } else {
            absPath = path;
            FileName = "";
            return false; // Error: illegal drive character
         }
      } else {
         if (path[0] == OSPathSep) {
            // Path is absolute on current drive
            absPath = getCurrWinDriveLetter();
            absPath += ':';
            absPath += path.getChars();
         } else {
            // Path is relative to current directory
            const char* drivePath = _getcwd (NULL, 0);
            absPath = drivePath;
            absPath += OSPathSep;
            absPath += path.getChars();
         }
      }
   } else if (path.getLength() == 3) {
      if (path[1] == ':'
      &&  path[2] == OSPathSep)
      {
         // Absolute path to root of given drive
         absPath = path;
         FileName = "";
         return true;
      } else
      if (path[0] == OSPathSep
      &&  path[1] == OSPathSep)
      {
         absPath = path;
         FileName = "";
         return true;
      } else if (path[1] == ':') {
         // Relative path on given drive
         int drvNum = 0;
         if (convertLetterToWindowsDriveNumber (path[0], drvNum)) {
             getDrivePath (drvNum, absPath);
             if (path[2] != '.')
                FileName = path[2];
             return true;
         }
      } else if (path[0] == OSPathSep) {
         // Path is absolute on current drive
         absPath = getCurrWinDriveLetter();
         absPath += ":\\";
         absPath.appendPart(path.getChars(), 1 , 3);
         // Do not exit yet .. "\.." still requires special treatment
      } else {
         const char* drivePath = _getcwd (NULL, 0);
         absPath = drivePath;
         absPath += OSPathSep;
         absPath += path.getChars();
      }
   } else if (path.getLength() > 3) {
      if (   (path[1] == ':'
          &&  path[2] == OSPathSep)
      ||  (   path[0] == OSPathSep
          &&  path[1] == OSPathSep))
      {
         // Absolute path with given directory
         absPath = path;
      } else 
      if (path[1] == ':') {
         // Relative path on drive
         int drvNum = 0;
         if (convertLetterToWindowsDriveNumber (path[0], drvNum)) {
             getDrivePath (drvNum, absPath); 
             BGString temp;
             temp.copyPart (path.getChars(), 2, path.getLength());
             absPath += OSPathSep;
             absPath += temp.getChars();
             FileName = "";
         } else {
            absPath = path;
            FileName = "";
            return false; // Error: illegal drive character
         }
      } else if (path[0] == OSPathSep) {
         // Absolute path on current drive
         absPath = getCurrWinDriveLetter();
         absPath += ':';
         absPath += path.getChars();
         FileName = "";
      } else {
         const char* drivePath = _getcwd (NULL, 0);
         absPath = drivePath;
         absPath += OSPathSep;
         absPath += path.getChars();
      }
   }

   // Find static part of absolute path
   size_t staticPathEnd = 0;
   if (absPath.getLength() >= 3)
   {
      if (absPath[0] == OSPathSep
      &&  absPath[1] == OSPathSep)
      {
         staticPathEnd = 1;
      } else
      if (absPath[1] == ':'
      &&  absPath[2] == OSPathSep)
      {
         staticPathEnd = 2;
      }
   } else
      return false;

   // Split off file name
   i = absPath.getLength()-1;
   while (i > staticPathEnd
   &&     absPath.getChars() [i] != OSPathSep)
   {
      i--;
   }
   BGString absPathCandidate;
   if (i == staticPathEnd) {
      FileName.copyPart (absPath.getChars(), staticPathEnd+1, 
                         absPath.getLength());
      absPathCandidate.copyPart (absPath.getChars(), 0, staticPathEnd+1);
   } else {
      FileName.copyPart (absPath.getChars(), i+1, 
                         absPath.getLength());
      absPathCandidate.copyPart (absPath.getChars(), 0, i);
   }
   if (!FileName.equals (".")
   &&  !FileName.equals (".."))
   {
      absPath = absPathCandidate.getChars();
   } else
      FileName = "";

   // Remove all ., .. at beginning
   bool found = true;
   while (staticPathEnd + 2 < absPath.getLength()
   &&     found) 
   {
      found = false;
      const TCHAR* chars = absPath.getChars();
      if (chars[staticPathEnd+1] == '.'
      &&  chars[staticPathEnd+2] == OSPathSep)
      {
         BGString temp;
         temp.copyPart (absPath.getChars(), 0, staticPathEnd+1);
         temp.appendPart (absPath.getChars(), staticPathEnd+3, absPath.getLength());
         absPath = temp.getChars();
         found = true;
      } else
      if (staticPathEnd+3 < absPath.getLength ()) {
         if (chars[staticPathEnd+1] == '.'
         &&  chars[staticPathEnd+2] == '.'
         &&  chars[staticPathEnd+3] == OSPathSep)
         {
            BGString temp;
            temp.copyPart (absPath.getChars(), 0, staticPathEnd+1);
            temp.appendPart (absPath.getChars(), staticPathEnd+4, absPath.getLength());
            absPath = temp.getChars();
            found = true;
         }
      }
   }

   // Remove all ./
   i = staticPathEnd+1;
   while (i + 1 < absPath.getLength()) {
      if (absPath.getChars() [i-1] == OSPathSep
      &&  absPath.getChars() [i] == '.'
      &&  absPath.getChars() [i+1] == OSPathSep)
      {
         BGString temp;
         temp.copyPart (absPath.getChars(), 0, i);
         temp.appendPart (absPath.getChars (), i+2, absPath.getLength());
         absPath = temp.getChars();
      } else
         i++;
   }

   // Remove */../
   if (absPath.getLength() > 3) {
      i = absPath.getLength()-3;
      while (i > staticPathEnd) {
         const TCHAR* chars = &absPath.getChars()[i];
         if (chars[0] == OSPathSep
         &&  chars[1] == '.'
         &&  chars[2] == '.'
         &&  chars[3] == OSPathSep)
         {
            size_t j = i-1;
            while (j > staticPathEnd+1
            &&     absPath[j] != OSPathSep)
            {
               j--;
            }

            BGString temp;
            temp.copyPart (absPath.getChars(), 0, j);
            if (j == staticPathEnd+1)
               i++;
            temp.appendPart (absPath.getChars(), i+3, absPath.getLength());
            absPath = temp.getChars();
            i = j-1;
         } else
            i--;
      }
   }

   // Remove */.. at end of path
   if (staticPathEnd + 2 < absPath.getLength ()) {
      i = absPath.getLength()-3;
      const TCHAR* chars = &absPath.getChars() [i];
      if (chars[0] == OSPathSep
      &&  chars[1] == '.'
      &&  chars[2] == '.')
      {
         size_t j = max (i, staticPathEnd+1);
         if (i > staticPathEnd+1) {
            j = i-2;
            const TCHAR* chars = absPath.getChars();
            while (j > staticPathEnd + 1
            &&     chars[j] != OSPathSep)
            {
               j--;
            }
         }

         BGString temp;
         temp.copyPart (absPath.getChars(), 0, j);
         temp.appendPart (absPath.getChars(), i+3, absPath.getLength());
         absPath = temp.getChars();
      }
   }

   // Remove . at end
   if (absPath[absPath.getLength()-2] == OSPathSep
   &&  absPath[absPath.getLength()-1] == '.')
   {
      if (absPath.getLength() > staticPathEnd+2) {
         absPath.setLength (absPath.getLength()-2);
      } else {
         absPath.setLength (absPath.getLength()-1);
      }
   }

   return true;
}

#else

bool FileCache::parseFileName (const char* path, BGString& absPath, 
                               BGString& FileName)
{
   absPath = "";
   FileName = "";

   if (path != NULL) {
      size_t pathLen = strlen (path);
      if (pathLen > 0) {
         if (path[0] != OSPathSep) {
            // Create absolute path 
            //cout << "Create absolute path: " << absPath.getChars() << endl;
            absPath = get_current_dir_name ();
            absPath += OSPathSep;
            absPath += path;
         } else
            absPath = path;

         // Split off file name
         //cout << "Split off file name " << absPath.getChars() << endl;
         size_t i = absPath.getLength ();
         if (i > 0)
            i--;
         while (i > 0 
         &&     absPath.getChars () [i] != OSPathSep)
         {
            assert (i > 0); 
            i--;
            assert (i < absPath.getLength ());
         }
         if (i > 0) {
            FileName.appendPart (absPath.getChars(), i+1, absPath.getLength());
            if (FileName.equals (".") || FileName.equals ("..")) {
               FileName = "";
            } else {
               assert (i > 0);
               assert (i < absPath.getLength());
               absPath.setLength (i);
            }
         }

         // Remove /. unless path becomes empty
         //cout << "Remove /. unless path becomes empty: " << absPath.getChars() << endl;
         i = 1;
         while (i < absPath.getLength ()-1) {
            char a = 'x', b = 'x', c = 'x';
            absPath.getChar (i-1, a);
            absPath.getChar (i, b);
            absPath.getChar (i+1, c);
            if (a == OSPathSep
            &&  b == '.'
            &&  c == OSPathSep)
            {
               BGString temp;
               temp.copyPart (absPath.getChars(), 0, i);
               temp.appendPart (absPath.getChars(), i+2, absPath.getLength());
               absPath = temp.getChars();
            } else
               i++;
         }

         // Test for /. at end of string
         //cout << "Test for /. at end of string: " << absPath.getChars() << endl;
         if (absPath.getLength () > 2) {
            char a = 'x', b = 'x';
            absPath.getChar (absPath.getLength ()-2, a);
            absPath.getChar (absPath.getLength ()-1, b);
            if (a == OSPathSep
            &&  b == '.')
            {
               // Ends with /., but is longer than that: cut off /.
               absPath.setLength (absPath.getLength ()-2);
            }
         } else if (strcmp (absPath.getChars(), "/.") == 0)
            absPath = '/';

         // Reduce /../ at beginning to / 
         //cout << "Reduce /../ at beginning to /: " << absPath.getChars() << endl;
         bool KeepScanning = true;
         while (absPath.getLength () > 4 && KeepScanning) {
            if (absPath.getChars() [0] == OSPathSep
            &&  absPath.getChars() [1] == '.'
            &&  absPath.getChars() [2] == '.'
            &&  absPath.getChars() [3] == OSPathSep)
            {
               BGString temp;
               temp.copyPart (absPath.getChars(), 3, absPath.getLength());
               //cout << "-> Reduced to " << temp.getChars() << endl;
               absPath = temp.getChars();
            } else
               KeepScanning = false;
         }

         // Remove */.. unless path becomes empty
         //cout << "Remove */.. unless path becomes empty: " << absPath.getChars() << endl;
         i = 1;
         while (i+3 < absPath.getLength()) {
            if (absPath.getChars()[i] == OSPathSep
            &&  absPath.getChars()[i+1] == '.'
            &&  absPath.getChars()[i+2] == '.'
            &&  absPath.getChars()[i+3] == OSPathSep)
            {
               //cout << "-> Found /../ at " << i << endl;
               size_t j = i-1;
               while (absPath.getChars()[j] != OSPathSep
               &&     j > 0)
               {
                  j--;
               }

               if (absPath.getChars()[j] == OSPathSep) {
                  //cout << "-> Removing */../ between chars " << j << " and "
                  //     << i << " from " << absPath.getChars() << endl;
                  BGString str1, str2;
                  str1.copyPart (absPath.getChars (), 0, j+1);
                  str2.copyPart (absPath.getChars (), i+4, absPath.getLength());
                  //cout << "-> Part before: " << str1.getChars() << endl;
                  //cout << "-> Part after: " << str2.getChars() << endl;
                  absPath = str1.getChars();
                  absPath += str2.getChars();
                  //cout << "Result is " << absPath.getChars() << endl;
                  i = j;
               }
            } else {
               i++;
            }
         }

         // Remove */.. at end of path (can occur only once after previous step)
         //cout << "Remove */.. at end of path: " << absPath.getChars() << endl;
         if (absPath.getLength () > 3) {
            i = absPath.getLength();
            if (absPath.getChars()[i-3] == OSPathSep
            &&  absPath.getChars()[i-2] == '.'
            &&  absPath.getChars()[i-1] == '.')
            {
               //cout << "-> found" << endl;
               i = i - 4;
               while (i > 0
               &&     absPath.getChars()[i] != OSPathSep)
               {
                  i--;
               }

               BGString str;
               str.copyPart (absPath.getChars(), 0, i);
               absPath = str.getChars();
            }
         }

         // Reduce /., /.. to / 
         //cout << "Reduce /.. /: " << absPath.getChars() << endl;
         if (absPath.equals ("/..")
         ||  FileName.equals ("/."))
         {
            absPath = '/';
         }
      }
   }

   return true;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Test if file exists

bool fileExists (const char* FN)
{
#ifdef WIN32
   WIN32_FIND_DATAA fileData;
   bool found = false;

   HANDLE hFind = FindFirstFileA (FN, (&fileData));
   if (hFind != INVALID_HANDLE_VALUE) {
      DWORD v1 = FILE_ATTRIBUTE_DIRECTORY;
      DWORD v2 = fileData.dwFileAttributes & v1;
      if (v2 == 0)
         found = true;
   }

   FindClose (hFind);

   return found;
#else
   struct stat fileStat;
      return (stat (FN, &fileStat) == 0);
/*   if (stat (FN, &fileStat) == 0)
         bool found = !S_ISDIR(fileStat.st_mode);
   if (found)
      printf ("Found: %s\n", FN);
   else
      printf ("Not found: %s\n", FN);
   return found;*/
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Return file size, given the file's name

INT64 getFileSize (const char* FN)
{
#ifdef WIN32
   WIN32_FIND_DATAA fileData;
   INT64 result = 0;

   HANDLE hFind = FindFirstFileA (FN, (&fileData));
   if (hFind != INVALID_HANDLE_VALUE) {
      result = fileData.nFileSizeLow;
   }

   FindClose (hFind);

   return result;
#else
   struct stat fileStat;
   stat (FN, &fileStat);
   return fileStat.st_size;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void copyFile (const TCHAR* sourceFN, const TCHAR* targetFN)
{
   FileCache source, target;
   if (source.openReadOnly (sourceFN) && target.create (targetFN)) {
      const size_t bufSize = 16384;
      char* buf = new char[bufSize];
      size_t offset = 0;

      while (offset < source.getFileSize()) {
         size_t toRead = min (bufSize, source.getFileSize()-offset);
         source.read (offset, buf, toRead);
         target.write (offset, buf, toRead);
         offset += toRead;
      }
   }

   source.close();
   target.close();
}

////////////////////////////////////////////////////////////////////////////////
// Default constructor

MemoryBlock::MemoryBlock (size_t newSize)
   : VirtualClass (MemoryBlockID)
{
   size = newSize;
   buf = new unsigned char[size];
   needToFreeMem = true;
}

// Clone constructor
MemoryBlock::MemoryBlock (unsigned char* newBuf, size_t newSize, bool freeLater)
   : VirtualClass (MemoryBlockID)
{
   size = newSize;
   buf = newBuf;
   needToFreeMem = freeLater;
}

// Destructor
MemoryBlock::~MemoryBlock()
{
   resize (0);
}

// Resize MemoryBlock
void MemoryBlock::resize (size_t newSize)
{
   unsigned char* tempBuf = NULL;
   if (newSize != 0) {
      tempBuf = new unsigned char[newSize];
      needToFreeMem = true;
   } else
      needToFreeMem = false;

   if (tempBuf != NULL) {
      size_t count = bgmin (size, newSize);
      memcpy (tempBuf, buf, count);
   }

   if (size > 0 && buf != NULL && needToFreeMem) {
      delete buf;
   }

   size = newSize;
   buf = tempBuf;
}

////////////////////////////////////////////////////////////////////////////////
// Get memory block for a file

bool MemoryBlock::loadFromFile (const TCHAR* FN)
{
   FileCache fc;
   if (fc.openReadOnly (FN)) {
      size_t fs = fc.getFileSize();
      resize (fs);
      return fc.read(0, (char*) buf, fs);
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Save to file

bool MemoryBlock::saveToFile (const TCHAR* FN)
{
   FileCache fc;
   if (fc.create (FN)) {
      return fc.write(0, (const char*) buf, size);
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Replace all occurences of old with new string

size_t MemoryBlock::replaceAll (const char* oldStr, const char* newStr)
{
   size_t len = strlen (oldStr);
   size_t replaced = 0;
   if (len != strlen (newStr) || size < len)
      return 0;

   for (size_t i = 0; i < size-len; ++i) {
      bool matches = true;
      size_t j = 0; 
      while (j < len && matches) {
         if (buf[i+j] == oldStr[j])
            ++j;
         else
            matches = false;
      }

      if (matches) {
         ++replaced;
         for (size_t k = 0; k < len; ++k)
            buf[i+k] = newStr[k];
      }
   }

   printf ("%i matches for %s replaced with %s\n", replaced, oldStr, newStr);
   return replaced;
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
// Constructor: list all files, with wild cards
FileList::FileList ()
{
   hFind = INVALID_HANDLE_VALUE;
}

// Destructor
FileList::~FileList ()
{
   if (hFind != INVALID_HANDLE_VALUE)
      FindClose(hFind);
}

bool FileList::start (const TCHAR* path)
{
   if (hFind != INVALID_HANDLE_VALUE)
      FindClose(hFind);

   hFind = FindFirstFile(path, &FindFileData);
   return (hFind != INVALID_HANDLE_VALUE);
}

// Get members
const TCHAR* FileList::getFileName() const
{ return FindFileData.cFileName; }
size_t FileList::getSize() const
{ return FindFileData.nFileSizeLow; }

// Navigate
bool FileList::operator++()
{
   return (FindNextFile (hFind, &FindFileData) > 0);
}

bool FileList::isDirectory()
{
   if ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
      return true;
   else
      return false;
}

#else

// Constructor: list all files, with wild cards
FileList::FileList ()
{
}

// Destructor
FileList::~FileList ()
{
   closedir(dir);
}

void FileList::getStats(const char* fn)
{
   BGFilePath<char> fullFN = m_path.getChars();
   fullFN += fn;
   struct stat s;
   stat(fullFN.getChars(), &s);
   size = s.st_size; 
   isDir = S_ISDIR(s.st_mode);

   printf ("Stats for %s: ", fullFN.getChars());
   if (isDir)
      printf(" <dir>\n");
   else
      printf(" %i\n", size);
}

bool FileList::start (const TCHAR* path)
{
   mask = path;
   size_t pathSepPos;
   m_path = path;

   // Todo: this code probably does not handle all necessary cases
   if (!m_path.equals("*"))
   if (m_path.findChar(OSPathSep, mask.getLength(), 0, -1, pathSepPos)) {
      mask.copyPart(path, pathSepPos+1, mask.getLength());
      m_path.setLength(pathSepPos);
   }

   dir  = opendir(m_path.getChars());
   if (dir == NULL) 
      return false;

   entry = readdir (dir);
   if (entry == NULL)
      return false;

   bool doContinue;
   do {
      if (matches()) {
	getStats (entry->d_name);
        return true;
      } else {
        doContinue = (operator++());
      }
   } while (doContinue);
   return false;
}

// Get members
const TCHAR* FileList::getFileName() const
{ return entry->d_name; }

// Navigate
bool FileList::operator++()
{
   entry = readdir(dir);

   if (entry == NULL)
      return false;

   if (mask.getLength() == 0)
      return true;

   bool done;
   do {
      if (matches()) {
	 getStats (entry->d_name);
         return true;
      } else {
         done = !operator++();
      }
   } while (!done);

   return false;
}

size_t FileList::getSize() const
{
   return size; 
}


bool FileList::isDirectory()
{
   return isDir;
}

// Check if entry matches mask
bool FileList::matches() const
{
   if (mask.getLength() == 0)
      return true;

   size_t hashPos;
   if (mask.findChar('*', hashPos)) {
      size_t nameLen = strlen(entry->d_name);
      if (nameLen >= mask.getLength()-1) {
         if (hashPos > 0) {
            const TCHAR* p1 = mask.getChars();
            const TCHAR* p2 = entry->d_name;
            if (memcmp (p1, p2, hashPos-1))
               return false;
         }
         if (hashPos < mask.getLength()-1) {
            size_t matchLen = mask.getLength()+hashPos;
            const TCHAR* p1 = &(mask.getChars()[hashPos+1]);
            const TCHAR* p2 = &(entry->d_name[strlen(entry->d_name)-matchLen+1]);
            if (memcmp (p1, p2, matchLen))
               return false;
         }
         return true;
      } else
         return false;
   } else {
      // No hash: only exact matches allowed
      return mask.equals (entry->d_name);
   }
}
#endif

// Find first match for mask
bool FileList::findFirstMatch (const TCHAR* mask, BGString& result)
{
   FileList list;
   if (list.start (mask)) {
      result = list.getFileName();
      return true;
   } else {
      result.clear();
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////
// Patch targetFN using a previously produced diffFile

bool patch(const TCHAR* diffFile, const TCHAR* targetFN)
{
   // Open <diff>, <file>
   MemoryBlock diff, mb;
   if (!diff.loadFromFile(diffFile)) {
      printf ("Unable to open %s\n", diffFile);
      return false;
   }
   if (!mb.loadFromFile(targetFN)) {
      printf ("Unable to open %s\n", targetFN);
      return false;
   }

   // Check if <diff> applies
   DiffHeader* header = (DiffHeader*) diff.getPtr(0);
   if (strcmp(header->signature, "PRJPP diff") != 0) {
      printf("%s: not a diff file\n", diffFile);
      return false;
   }

   if (header->size1 != mb.getSize()) {
      printf("%s does not apply to %s (wrong size)\n", diffFile, targetFN);
      return false;
   }

   size_t offset = sizeof(*header);
   for (size_t i = 0; i < header->diffs; i++) {
      size_t outOffset = diff.getDWord(offset);
      char oldVal1 = diff.getByte(offset+4);
      char oldVal2 = mb.getByte(outOffset);
      if (oldVal1 != oldVal2) {
         printf ("%s does not apply to %s (difference at offset %i: %i-%i)\n", 
                 diffFile, targetFN, outOffset, oldVal1, oldVal2);
         return false;
      }
      offset += 6;
   }

   // Apply diffs
   offset = sizeof(*header);
   for (size_t i = 0; i < header->diffs; i++) {
      size_t outOffset = diff.getDWord(offset);
      char newVal = diff.getByte(offset+5);
      mb.setByte(outOffset, newVal);
      offset += 6;
   }

   mb.append(diff.getPtr(offset), diff.getSize()-offset);

   // Write back results
   if (mb.saveToFile(targetFN)) {
      printf ("Patch operation successful\n");
      return true;
   }else {
      printf("Patch: writing %s failed\n", targetFN);
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// FileList constructor

/*FileList::FileList (const TCHAR* path)
{
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind = FindFirstFile(path, &FindFileData);
   FileListInfo* last = NULL;

   if (hFind != INVALID_HANDLE_VALUE) 
   {
      do {
         FileListInfo* newInfo = new FileListInfo();
         newInfo->next = NULL;
         newInfo->previous = last;
         newInfo->fileName = FindFileData.cFileName;
         newInfo->size = FindFileData.nFileSizeLow;
         if (last != NULL)
            last->next = newInfo;
         last = newInfo;
      } while (FindNextFile (hFind, &FindFileData));
      FindClose(hFind);
   } 
}
*/

// Delete all files in dirName
void deleteTree (const TCHAR* dirName)
{
   FileList list;
   BGFilePath<TCHAR> searchMask = dirName;
   searchMask += "*";
   if (list.start(searchMask.getChars())) {
      do {
         if (strcmp (list.getFileName(), ".") != 0
         &&  strcmp (list.getFileName(), "..") != 0) 
         {
            BGFilePath<TCHAR> next = dirName;
            next += list.getFileName();
            if (list.isDirectory()) {
               deleteTree(next.getChars());
#if _WIN32
               _rmdir(next.getChars());
#else
			   rmdir(next.getChars());
#endif
            } else {
               DeleteFile(next.getChars());
            }
         }
      }
      while (++list);
   }
}
