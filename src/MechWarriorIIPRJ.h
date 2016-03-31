////////////////////////////////////////////////////////////////////////////////
// MechWarrior II PRJ file system
// Copyright Bjoern Ganster 2007-2009
////////////////////////////////////////////////////////////////////////////////

#ifndef MechWarriorIIPRJ__H
#define MechWarriorIIPRJ__H

#include "BGString.h"
#include "BGVector.h"
#include "FileCache.h"
#include "Archive.h"
#include "XMLTree.h"
#include "BGVector.h"

#ifndef TitanFarPatcher
class Texture;
class Mesh;
class XMLTree;
class MW2TextureTable;
#endif

////////////////////////////////////////////////////////////////////////////////
// MW2 PRJ record definitions

#pragma pack (push)
#pragma pack(2) // MW2 file data are WORD aligned?

struct MW2_PRJ_main_directory_entry {
   char dirName[4];
   DWORD dirOffset;
   char unknown[16];
};

struct MW2_PRJ_directory_header {
   char marker[4]; // Marker string "INDX"
   DWORD structSize; // Size of this record, plus MW2_PRJ_Directory_Entries, minus 8
   DWORD unknown;
   char DirName[4];
   WORD reservedCount; // Total number of directory entries 
   WORD altCount; // Alternative number of entries, in most cases the same as entries
   WORD entries; // Used number of entries, plus one
   //char unknown2[8]; // zero in all tested cases
}; // 22 bytes

struct MW2_PRJ_Directory_Entries {
   DWORD offset;
   DWORD size;
}; // 8 bytes

struct MW2_PRJ_local_file_header {
   char marker[4]; // Marker string DATA
   DWORD SizeOfThisFile,      // Size of file plus 54 bytes
         unknown1;
   char parentDir[4];         // Name of directory containing the file
   char unknown2[8]; 
   WORD FileNumberInParentDir; // Entry number of this file in parent dir + 1
   char unknown3[4];          // A flag? 
   char nameWithoutExt[16];   // Null-terminated file name without extension
   char nameWithExt[16];      // Null-terminated file name with extension
}; // 62 bytes

struct SymbolTableHeader {
   char marker[4]; // Marker string "SYMB"
   DWORD size; // Size of this record, plus file name records, minus 8
   char parentDir[4];         // Name of directory containing the file
   WORD maxEntries;
   WORD altCount; // Exact number of entries, same as entries, below?
   WORD unknown;
   DWORD entries; // Exact number of entries
}; // 16h=22 bytes

struct SymbolTableEntry {
   char FileName[16]; // File name without extension, zero-padded
   WORD FileNum; // 1 for first file, 2 for second, ...
}; // 12h=18 bytes

#pragma pack (pop)

////////////////////////////////////////////////////////////////////////////////
// todo: if everything works well for some months after 2009-03-19,
// eliminate this constant, otherwise test setting it to 1

const int MW2SkippedFiles = 0;

////////////////////////////////////////////////////////////////////////////////
// Class to access MW2.PRJ files

class MechWarriorIIPRJ: public MWArchive {
public:
   // Constructor
   MechWarriorIIPRJ();

   // Destructor
   ~MechWarriorIIPRJ();

   // Open file
   virtual bool open (const TCHAR* PRJFN, bool readOnly = true);

   // Close file
   virtual void close();

   // Get number of entries in directory
   //virtual size_t getEntryCount() const
   //{ return entries; }

   // Enter subdirectory
   virtual bool enterSubDir (size_t index);
   virtual bool enterSubDir (const char* dirName);

   // Leave subdirectory
   virtual bool leaveDir();

   // Read bufSize bytes from file numbered index at offset into buf
   virtual bool getFileChunk(size_t index, size_t offset, void* buf, size_t bufSize);

   // Load texture, geometry
   #ifndef TitanFarPatcher
   static Texture* loadTexture (const unsigned char* buf, size_t bufSize,
      MemoryBlock* palette);
   static Mesh* loadWTB (const MemoryBlock* block);
   #endif

   // Read bufSize bytes from file numbered index at offset into buf
   #ifndef TitanFarPatcher
   static Texture* loadPCX (const unsigned char* buf, size_t bufSize);
   #endif

   // Convert SFL to Wav file
   static MemoryBlock* SFL2Wav (const MemoryBlock* mb);

   // SHP suport
   #ifndef TitanFarPatcher
   static inline size_t getSHPFrameCount(const MemoryBlock& mb) 
   { return mb.getDWord(4); }
   static Texture* loadSHP (const MemoryBlock& mb, size_t picNum, 
                            MemoryBlock* palette);
   #endif

   // Check if currently in sub directory
   virtual bool inSubDir() const
   { return (m_offset > 0x18); }

   // Export BWD file as XML
   #ifndef TitanFarPatcher
   bool BWD2XML (const MemoryBlock& mb, XMLTreeIterator& iter, 
                 bool followIncludes, const char* WTB_outputDir);
   #endif

   // Load MW2 PRJ file into a MemoryBlock
   MemoryBlock* loadFile (const TCHAR* FN);

   // TEX file introduced in MW2:Mercs
   #ifndef TitanFarPatcher
   static Texture* loadTEX (const unsigned char* buf, size_t bufSize);
   #endif

   // Save PRJ file to new file name
   bool saveToFile (const TCHAR* fn);

   // Replace numbered file in current directory
   //bool setFile (size_t fileNum, const char* fileBuf, size_t BWDSize);

   // Replace  file
   virtual bool replaceFile (size_t fileNum, const TCHAR* srcFN);

   // Check if archive type supports file replacement
   virtual bool replaceFilePossible () const
   {
      if (fc.getFileSize() > 15 * 1024 * 1024) 
         return (fc.isReadOnly() == false); 
      else
         return false; // Prevent NetMech fiddling
   }

   // Check if entry index is a directory
   virtual bool isDirectory (int /*index*/) const
   {
      return (!inSubDir());
   }

   // Add files to archive
   virtual void addFileToArchive (const TCHAR* FN);
   virtual bool canAddFilesToArchive () const
   { return true; }

   // Get file name for a file in another directory w/ochanging into that directory
   bool getEntryNameFromOtherDir (const TCHAR* dirName, size_t index, TCHAR* buf, 
      size_t bufSize);

   // Get file from another directory withoutchanging into that directory
   MemoryBlock* getFileFromOtherDir (const TCHAR* dirName, size_t index);
private:
   size_t m_offset;
   WORD m_mainDirEntryCount;
   MW2_PRJ_main_directory_entry* m_mainDirEntries;
   BGString m_dirName;
   BGVector <MW2_PRJ_Directory_Entries*, BGVectorBufID> m_dirEntries;
   BGVector <MW2_PRJ_directory_header, BGVectorBufID> m_dirHeaders;

   // Close file
   inline bool readMainDir();

   // Find directory
   size_t findDirectory(const TCHAR* dirName);
};

// Set MW2:Titanium far clipping plane distance
//const char* setRenderingDistance (const TCHAR* prjFN, DWORD newDist);

// Decode MW2 555 image
#ifndef TitanFarPatcher
Texture* Decode_MW2_555 (unsigned char* mem, size_t size);
#endif

#endif
