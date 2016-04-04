////////////////////////////////////////////////////////////////////////////////
// ISO9660.h
// Class for reading ISO 9660 image files
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#ifndef ISO9660__h
#define ISO9660__h

#include "Archive.h"

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for MechWarrior archive files
// Currently implemented by MechWarriorIIPRJ

class ISO9660: public MWArchive {
public:
	virtual ~ISO9660();
   // Open file
   virtual bool open (const TCHAR* PRJFN, bool readOnly = true);

   // Close file
   virtual void close();

   // Read bufSize bytes from file numbered index at offset into buf
   virtual bool getFileChunk(size_t index, size_t offset, void* buf, size_t bufSize);

   // Enter subdirectory
   virtual bool enterSubDir (size_t index);

   // Leave subdirectory
   virtual bool leaveDir();

   // Check if currently in sub directory
   virtual bool inSubDir() const;

   // Check if entry index is a directory
   virtual bool isDirectory (int index) const;
private:
   static const int SectorSize = 2352;

   BGVector <bool, BGVectorBufID> m_isDirectory;
   BGVector <int, BGVectorBufID> m_parentDirStartSector;
   BGVector <int, BGVectorBufID> m_parentDirSize;

   // Read directory entries
   bool readDirectory(int sector, int dirSize);

   // Read extent (directory or file data)
   bool readExtent(int startSector, void* buf, int toRead, 
      int startOffset);

   void forgetDirectoryContents();
};

#endif
