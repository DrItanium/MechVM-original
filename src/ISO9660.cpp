////////////////////////////////////////////////////////////////////////////////
// ISO9660.cpp
// Class for reading ISO 9660 image files
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#include "ISO9660.h"
#include "stdio.h"

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for MechWarrior archive files
// Currently implemented by MechWarriorIIPRJ

// Open file
bool ISO9660::open (const TCHAR* PRJFN, bool /*readOnly*/)
{
   bool openedSuccessfully = fc.openReadOnly(PRJFN);

   if (openedSuccessfully) {
      char data[SectorSize];
      int offset = 16*SectorSize;
      bool readSuccess = fc.read(offset, data, SectorSize);
      openedSuccessfully =
         readSuccess &&
         (data[17] == 'C') &&
         (data[18] == 'D') &&
         (data[19] == '0') &&
         (data[20] == '0') &&
         (data[21] == '1');
      DWORD rootDirSector = *((DWORD*) (&data[174]));
      DWORD rootDirSize = *((DWORD*) (&data[182]));
      //printf ("Root directory sector: %i, %i bytes\n", rootDirSector, rootDirSize);
      readDirectory(rootDirSector, rootDirSize);
      m_parentDirStartSector.add(rootDirSector);
      m_parentDirSize.add(rootDirSize);
   }

   return openedSuccessfully;
}

// Close file
void ISO9660::close()
{
   fc.close();
   forgetDirectoryContents();
}

void ISO9660::forgetDirectoryContents()
{
   FileSizes.clear();
   FileOffsets.clear();
   FileNames.clear();
   m_isDirectory.clear();
}

// Read extent (directory or file data)
bool ISO9660::readExtent(int startSector, void* buf, int toRead, int startOffset)
{
   if (startOffset > SectorSize) {
      startSector += (startOffset + SectorSize-1) / SectorSize;
      startOffset = startOffset % SectorSize;
   }

   int readNow = bgmin(toRead, 2048);
   int offset = startSector*SectorSize + 16 + startOffset;
   if (fc.read(offset, (char*) buf, readNow)) {
      toRead -= readNow;
      if (toRead > 0)
         return readExtent(startSector+1, &((char*)buf)[readNow], toRead, 0);
      else
         return true;
   } else
      return false;
}

// Read bufSize bytes from file numbered index at offset into buf
bool ISO9660::getFileChunk(size_t index, size_t offset, void* buf, size_t bufSize)
{
   int toRead = bgmin(bufSize, FileSizes[index]);
   return readExtent(FileOffsets[index], buf, toRead, offset);
}

// Enter subdirectory
bool ISO9660::enterSubDir (size_t index)
{
   if (index < FileOffsets.getSize()) {
      int dirStartSector = FileOffsets[index];
      int dirSize = FileSizes[index];
      m_parentDirStartSector.add(dirStartSector);
      m_parentDirSize.add(dirSize);
      forgetDirectoryContents();
      return readDirectory(dirStartSector, dirSize);
   } else
      return false;
}

// Leave subdirectory
bool ISO9660::leaveDir()
{
   if (inSubDir()) {
      m_parentDirStartSector.removeLast();
      m_parentDirSize.removeLast();
      int last = m_parentDirStartSector.getSize()-1;
      int dirStartSector = m_parentDirStartSector[last];
      int dirSize = m_parentDirSize[last];
      forgetDirectoryContents();
      return readDirectory(dirStartSector, dirSize);
   } else
      return false;
}

// Check if currently in sub directory
bool ISO9660::inSubDir() const
{
   return (m_parentDirStartSector.getSize()>1);
}

// Check if entry index is a directory
bool ISO9660::isDirectory (int index) const
{
   return m_isDirectory[index];
}

///////////////////////////////////////////////////////////////////////////////
// Read directory entries
#include <cstdint>

typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
#pragma pack (1)
struct ISO_9660_Directory_Entry {
   UINT8 entryLen;
   UINT8 ExtendedAttributeRecordLength;
   UINT32 startSector_LE;
   UINT32 startSector_BE;
   UINT32 fileSize_LE;
   UINT32 fileSize_BE;
   char recordDateTime[7];
   UINT8 flags;
   UINT8 unitSize;
   UINT8 gapSize;
   UINT16 VolumeSeqNo_LE;
   UINT16 VolumeSeqNo_BE;
   UINT8 FileNameLength;
};

bool ISO9660::readDirectory(int sector, int dirSize)
{
   char* data = new char[dirSize];
   int offset = sector*SectorSize;
   //bool readSuccess = fc.read(offset, data, SectorSize);
   bool readSuccess = readExtent(sector, data, dirSize, 0);
   int found = 0;

   if (readSuccess) {
      offset = 0;
      while (offset < dirSize) {
         ISO_9660_Directory_Entry* entry = (ISO_9660_Directory_Entry*)&data[offset];
         if (entry->entryLen == 0) {
            //offset = dirSize; // Null-entry, done/abort
            offset = ((offset / 2048) + 1) * 2048;
         } else {
            BGString fn;
            int nameOffset = offset+sizeof(ISO_9660_Directory_Entry);
            fn.copyPart(data, nameOffset, nameOffset+entry->FileNameLength);

            size_t stopPos;
            if (fn.findChar(';', stopPos)) {
               fn.setLength(stopPos);
            }

            //if (fn.getLength() > 0 && sector != entry->startSector_LE) {
            ++found;
            if (found > 2) {
               addFile (fn.getChars(), entry->fileSize_LE, entry->startSector_LE);
               if ((entry->flags & 2) > 0)
                  m_isDirectory.add(true);
               else
                  m_isDirectory.add(false);
            }
            offset += entry->entryLen;
         }
      }
   }

   delete data;
   return readSuccess;
}

ISO9660::~ISO9660() {
	m_isDirectory.clear();
}
