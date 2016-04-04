////////////////////////////////////////////////////////////////////////////////
// Archive.h
// Abstract base class for MechWarrior archive files
// Copyright Bjoern Ganster 2008
////////////////////////////////////////////////////////////////////////////////

#ifndef Archive__h
#define Archive__h

#include "BGBase.h"
#include "MWBase.h"
#include "BGString.h"
#include "BGVector.h"

class MemoryBlock;

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for MechWarrior archive files
// Currently implemented by MechWarriorIIPRJ

class MWArchive: public VirtualClass {
public:
	MWArchive();
	virtual ~MWArchive();
   // Open file
   virtual bool open (const TCHAR* PRJFN, bool readOnly = true) = 0;

   // Close file
   virtual void close() = 0;

   // Get number of entries in directory
   virtual size_t getEntryCount() const
   { return FileOffsets.getSize(); }

   // Get name of entry
   virtual bool getEntryName (size_t index, TCHAR* buf, size_t bufSize);

   // Get size of entry, 0: directory or error
   virtual size_t getFileSize (size_t index)
   {
      if (index < FileSizes.getSize())
         return FileSizes[index];
      else
         return 0;
   }

   // Get offset of file start inside prj file
   virtual size_t getFileOffset (size_t index)
   {
      if (index < FileOffsets.getSize())
         return FileOffsets[index];
      else
         return 0;
   }

   // Read bufSize bytes from file numbered index at offset into buf
   virtual bool getFileChunk(size_t index, size_t offset, void* buf, size_t bufSize);

   // Enter subdirectory
   virtual bool enterSubDir (size_t /*index*/)
   {
      return false; // Not supported by archive type
   }

   virtual bool enterSubDir (const char* /*dirName*/)
   {
      return false; // Not supported by archive type
   }

   // Leave subdirectory
   virtual bool leaveDir()
   {
      return false; // Not supported by archive type
   }

   // Check if currently in sub directory
   virtual bool inSubDir() const
   {
      return false; // Not supported by archive type
   }

   // Replace  file
   virtual bool replaceFile (size_t /*fileNum*/, const TCHAR* /*srcFN*/)
   {
      return false;
   }

   // Check if archive type supports file replacement
   virtual bool replaceFilePossible () const
   { return false; }

   // Check if entry index is a directory
   virtual bool isDirectory (int /*index*/) const
   { return false; }

   // Create a list of files
   void createFileList(const TCHAR* FN);

   // Export all files in a mw2.prj file
   void exportToDirectory(const TCHAR* saveFN);

   size_t getArchiveSize () const
   { return fc.getFileSize(); }

   // Return index of file in current directory, or entryCount+1 if not found
   size_t getEntryNumberByName(const char* name);

   // Add files to archive
   virtual void addFileToArchive (const TCHAR* /*FN*/)
   { }
   virtual bool canAddFilesToArchive () const
   { return false; }

   // Get memory block for a file inside the archive
   MemoryBlock* getMemoryBlock (size_t fileNum);
   inline MemoryBlock* getMemoryBlock (const char* fn)
   {
      size_t fno = getEntryNumberByName(fn);
      return getMemoryBlock(fno);
   }
protected:
   FileCache fc;
   BGVector <size_t, BGVectorBufID> FileSizes;
   BGVector <size_t, BGVectorBufID> FileOffsets;
   BGVector <TCHAR*, BGVectorBufID> FileNames;

   // Calculate file sizes from offset differences
   void calculateFileSizesFromOffsetDifferences();

   void addFile (const TCHAR* name, size_t size, size_t offset);

   void clearFileEntries();

   void createFileListEntry(FileCache& fc, const TCHAR* subDirName, int index);
};

////////////////////////////////////////////////////////////////////////////////

// Check if type is an archive
bool isArchiveType (int type);

// Open archive
MWArchive* openArchive (const TCHAR* archivePath);

// Unpack archive
void unpackArchive (MWArchive* archive, const TCHAR* target);

#endif

