////////////////////////////////////////////////////////////////////////////////
// FileCache.h
// Copyright Bjoern Ganster 2005-2011
////////////////////////////////////////////////////////////////////////////////

#ifndef FileCache__H
#define FileCache__H

////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include "BGString.h"

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

////////////////////////////////////////////////////////////////////////////////
// Header definition for a diff file

struct DiffHeader {
   char signature[16];
   DWORD size1, size2, diffs, reserved;
};

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

////////////////////////////////////////////////////////////////////////////////
// Class FileCache
// FilePos could be stored, but would have to be tracked and reset if new 
// operation is read and last was write or vice versa

class FileCache: public VirtualClass {
   private:
      // Property variables and private members
      FileOffset m_CacheStartPos;
      CacheOffset m_CacheSize;
      FileOffset m_FileSize;
      char* cache;
      FileOffset UsedCacheSize;
      bool MustWriteCacheBack;
      #ifdef _WIN32
      HANDLE m_FileHandle;
      #else
      FileHandle m_FileHandle;
      #endif

      // Delay writing back if set to ReadOnly until necessary ...
      // MustWriteCacheBack stores necessary state
      bool m_isReadOnly;

      // Write cache back to file
      bool writeBackCache ();

      // Fill cache
      bool fillCache ();

      // Invalidate Cache
      void invalidateCache ();

      // Copy bytes from buf
      bool copyBytesFromBuf (const char* buf, FileOffset offset, 
                             FileOffset count);

      bool enlargeFile (FileOffset NewSize);

      // Check if handle is valid
      inline bool isHandleValid() const
      {
      #ifdef _WIN32
      return (m_FileHandle != INVALID_HANDLE_VALUE);
      #else
      return (m_FileHandle != 0);
      #endif
      }

   public:
      // Controls whether the destructor closes the file
      bool CloseFileOnDestroy;

      // Properties
      bool setCacheStartPos (FileOffset newPos);
      FileOffset getCacheStartPos () { return m_CacheStartPos; }

      bool setCacheSize (FileOffset newSize);
      FileOffset getCacheSize () 
      { return m_CacheSize; }

      FileOffset getFileSize () const 
      { return m_FileSize; }

      // Constructor, destructor
      FileCache (FileOffset NewCacheSize = 4096);
      virtual ~FileCache ();

      // Open, close file
      bool openReadOnly (const TCHAR* FileName);
      bool create (const TCHAR* FileName);
      bool openReadWrite (const TCHAR* FileName);
      void close ();

      // Read, write access
      bool read (FileOffset offset, char* buf, FileOffset count);
      bool write (FileOffset offset, const char* buf, FileOffset count);
      bool readLine (FileOffset& offset, BGString& line);
      inline int readInt32 (FileOffset offset)
      {
         int i = 0;
         read (offset, (char*) (&i), 4);
         return i;
      }
      inline DWORD readDWord (FileOffset offset)
      {
         DWORD result;
         read (offset, (char*) (&result), 4);
         return result;
      }

      // Append
      bool append (const char* buf, FileOffset count);
      bool append (const char* str)
      { return append (str, (FileOffset) strlen (str)); }
      bool append (const BGString& str)
      #ifdef UNICODE
      { return append ((const char*) (str.getChars()), 2*str.getLength()); }
      #else
      { return append (str.getChars(), (FileOffset) str.getLength()); }
      #endif

      // Is given path absolute?
      static bool isPathAbsolute (const char* path);

      // Path parsing
      static bool parseFileName (const char* path, BGString& absPath, 
                                 BGString& FileName);

      // Flush file
      void flush ();

      bool isReadOnly () const
      { return m_isReadOnly; }
};

////////////////////////////////////////////////////////////////////////////////

class MemoryBlock: public VirtualClass {
public:
   // Default constructor
   MemoryBlock (size_t newSize = 0);

   // Clone constructor
   MemoryBlock (unsigned char* newBuf, size_t newSize, bool freeLater = true);

   // Destructor
   virtual ~MemoryBlock();

   // Resize MemoryBlock
   void resize (size_t newSize);

   // Read access
   inline unsigned char getUByte (size_t offset) const
   { return buf[offset]; }
   inline unsigned char getByte (size_t offset) const
   { return ((unsigned char) (buf[offset])); }
   inline WORD getWord (size_t offset) const
   { return *((WORD*) (&buf[offset])); }
   inline int getInt32 (size_t offset) const
   { return *((int*) (&buf[offset])); }
   inline DWORD getDWord (size_t offset) const
   { return *((DWORD*) (&buf[offset])); }
   inline int16 getInt16 (size_t offset) const
   { return *((int16*) (&buf[offset])); }
   inline float getFloat (size_t offset) const
   { return * ((float*) (&buf[offset])); }

   // Get pointer into the buffer
   inline unsigned char* getPtr (size_t offset = 0)
   { return &buf[offset]; }
   inline const unsigned char* getPtr (size_t offset = 0) const
   { return &buf[offset]; }

   // Get size
   inline size_t getSize () const
   { return size; }

   // Load from file
   bool loadFromFile (const TCHAR* FN);

   // Save to file
   bool saveToFile (const TCHAR* FN);

   // Set byte, word or dword
   inline void setByte (size_t offset, unsigned char newVal)
   { buf[offset] = newVal; }
   inline void setWord(size_t offset, WORD newVal)
   { (WORD&) (buf[offset]) = newVal; }
   inline void setDWord(size_t offset, DWORD newVal)
   { (DWORD&) (buf[offset]) = newVal; }

   // Copy data into the buffer
   //inline bool copy (size_t offset, const void* _buf, size_t count)
   inline bool copy (size_t offset, const unsigned char* _buf, size_t count)
   {
      if (offset+count <= size) {
         //memcpy ((void*)(buf[offset]), _buf, count);
         for (size_t i = 0; i < count; i++)
            buf[offset+i] = _buf[i];
         return true;
      } else
         return false;
   }

   // Append
   inline void append (const unsigned char* buf, size_t bufSize)
   {
      size_t offset = size;
      resize (size+bufSize);
      copy (offset, buf, bufSize);
   }

   // Replace all occurences of old with new string
   size_t replaceAll (const char* oldStr, const char* newStr);

   // Fill block with byte
   inline void fill (size_t offset, size_t count, char byte)
   {
      if (offset > size)
         return;
      if (offset + count > size)
         count = size-offset;
      for (size_t i = 0; i < count; ++i)
         buf[offset+i] = byte;
   }
private:
   unsigned char* buf;
   size_t size;
   bool needToFreeMem;
};

////////////////////////////////////////////////////////////////////////////////
// Simple class to list files

/*class FileListInfo: public VirtualClass {
public:
   BGString fileName;
   size_t size;
   FileListInfo* previous, * next;
};

class FileList: public VirtualClass {
public:
   // Constructor: list all files, with wild cards
   FileList (const TCHAR* path);

   // Destructor
   ~FileList ()
   {
      while (first != NULL) {
         FileListInfo* temp = first->next;
         delete first;
         first = temp;
      }
   }

   // Get file count
   size_t getFileCount()
   { return count; }
private:
   FileListInfo* first;
   size_t count;
   friend class FileListIterator;
};

class FileListIterator: public VirtualClass {
public:
   // Constructor
   FileListIterator (FileList& list)
   { m_info = list.first; }

   // Destructor
   ~FileListIterator();

   // Get members
   const TCHAR* getFileName() const
   { return m_info->fileName.getChars(); }
   size_t getSize() const
   { return m_info->size; }

   // Navigate
   bool operator++()
   { 
      if (m_info->next != NULL) {
         m_info = m_info->next;
         return true;
      } else {
         return false;
      }
   }
   bool operator--()
   { 
      if (m_info->previous != NULL) {
         m_info = m_info->previous;
         return true;
      } else {
         return false;
      }
   }
private:
   FileListInfo* m_info;
};*/

class FileList: public VirtualClass {
public:
   // Constructor: list all files, with wild cards
   FileList ();

   // Destructor
   ~FileList ();

   bool start (const TCHAR* path);

   // Get members
   const TCHAR* getFileName() const;
   size_t getSize() const;

   // Navigate
   bool operator++();

   // Find first match for mask
   static bool findFirstMatch (const TCHAR* mask, BGString& result);

   bool isDirectory();
private:
#ifdef _WIN32
   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
#else
   DIR* dir;
   dirent* entry;
   BGString m_path, mask;
   bool matches() const;
   bool isDir;
   size_t size;
   void getStats(const char* fn);
#endif
};

#endif
