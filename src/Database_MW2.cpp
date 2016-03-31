////////////////////////////////////////////////////////////////////////////////
// MechWarrior II database.mw2 file system
// Copyright Bjoern Ganster 2007, 2008
////////////////////////////////////////////////////////////////////////////////

#include "Database_MW2.h"
#include "MWBase.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

Database_MW2::Database_MW2()
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Database_MW2::~Database_MW2()
{
}

////////////////////////////////////////////////////////////////////////////////
// Open file

bool Database_MW2::open (const TCHAR* PRJFN, bool readOnly)
{
   bool openedSuccessfully;
   if (readOnly)
      openedSuccessfully = fc.openReadOnly(PRJFN);
   else
      openedSuccessfully = fc.openReadWrite(PRJFN);

   if (openedSuccessfully) {
      if (fc.getFileSize() > 4) {
         size_t entries = fc.readDWord (0);
         if (fc.getFileSize() < 5*entries+4)
            return false;

         // Read file offsets
         for (size_t i = 0; i < entries; i++) {
            FileOffset fileOffset = fc.readDWord (4*i+4);
            size_t toRead = bgmin ((FileOffset) 50, fc.getFileSize()-fileOffset);
            MemoryBlock* mb = new MemoryBlock (toRead);
            fc.read (fileOffset, (char*) mb->getPtr(0), toRead);
            int fileType = getFileTypeFromContent (mb);
            fileTypes.add (fileType);
            addFile ("", 0, fileOffset);
            delete mb;
         }

         calculateFileSizesFromOffsetDifferences();

         return true;
      }
   }
   
   return false;
}

////////////////////////////////////////////////////////////////////////////////
// Close file

void Database_MW2::close()
{
   fc.close();
}

////////////////////////////////////////////////////////////////////////////////
// Get name of entry

bool Database_MW2::getEntryName (size_t index, TCHAR* buf, size_t bufSize)
{
   BGString str;
   str.assignUInt(index);
   str += '.';
   str += getMWFileExt (fileTypes[index]);
   size_t toCopy = str.getLength()+1;
   if (toCopy > bufSize)
      toCopy = bufSize;
   memcpy (buf, str.getChars(), toCopy);
   buf[bufSize-1] = 0;
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Replace  file

bool Database_MW2::replaceFile (size_t fileNum, const TCHAR* srcFN)
{
   // Load srcFN into MemoryBlock
   MemoryBlock mb;
   if (!mb.loadFromFile(srcFN))
      return false;

   // Append  MemoryBlock to archive
   DWORD newOffset = (DWORD) fc.getFileSize();
   const unsigned char* udata = mb.getPtr();
   const char* data = (const char*)((const void*) udata); // casting oddyssey ...
   fc.append(data, mb.getSize());

   // Update file data on archive
   int fileType = getFileTypeFromContent (&mb);
   FileSizes[fileNum] = mb.getSize();
   FileOffsets[fileNum] = newOffset;
   fileTypes[fileNum] = fileType;

   // Update archive file
   fc.write(4*fileNum+4, (const char*) (&newOffset), 4);
   close();
   return true;
}
