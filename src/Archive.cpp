////////////////////////////////////////////////////////////////////////////////
// Archive.h
// Abstract base class for MechWarrior archive files
// Copyright Bjoern Ganster 2008
////////////////////////////////////////////////////////////////////////////////

#include "BGBase.h"
#include "Archive.h"
#include "MechWarriorIIPRJ.h"
#include "Database_MW2.h"
#include "MechWarrior3ZBD.h"
#include "ISO9660.h"

////////////////////////////////////////////////////////////////////////////////
// Check if type is an archive

bool isArchiveType (int type)
{
   return (type == FT_PRJ || type == FT_MW2_DB || type == FT_ZBD || 
           type == FT_ISO_9660);
}

////////////////////////////////////////////////////////////////////////////////
// Open archive

MWArchive* openArchive (const TCHAR* archivePath)
{
   #ifndef TitanFarPatcher
   // Determine archive type
   int ft = getFileTypeFromExtension (archivePath);
   MWArchive* arch = NULL;

   // Create and open archive
   switch (ft) {
      case FT_PRJ:    
         arch = new MechWarriorIIPRJ();
         break;
      case FT_MW2_DB: 
         arch = new Database_MW2();
         break;
      case FT_ZBD:    
         arch = new MechWarrior3ZBD();
         break;
      case FT_ISO_9660:
         arch = new ISO9660();
         break;
   }
   #else
   MWArchive* arch = new MechWarriorIIPRJ();
   #endif

   if (arch != NULL) {
      if (!arch->open (archivePath, false)) {
         delete arch;
         arch = NULL;
      } else {
         printf ("Viewing archive %s\n", archivePath);
      }
   }

   return arch;
}

////////////////////////////////////////////////////////////////////////////////
// Unpack archive

void unpackArchive (MWArchive* archive, const TCHAR* target)
{
   if (archive != NULL) {
      archive->exportToDirectory (target);
      BGString listFN = target;
      listFN += ".txt";
//      archive->createFileList (listFN.getChars());
   }
}

////////////////////////////////////////////////////////////////////////////////
// Export all files in a mw2.prj file

void MWArchive::exportToDirectory(const TCHAR* saveFN)
{
   bgmkdir (saveFN);
   size_t count = getEntryCount();

   for (size_t i = 0; i < count; i++) {
      if (isDirectory (i)) {
         BGFilePath<TCHAR> dir = saveFN;
         //dir += "\\";
         TCHAR subDirName[20];
         getEntryName(i, subDirName, 20);
         dir += subDirName;
         bgmkdir (dir.getChars());
         enterSubDir(i);
         exportToDirectory (dir.getChars());
         leaveDir();
      } else {
         BGString FN = saveFN;
         char prjFN[50];
         FN+= "\\";
         getEntryName(i, prjFN, 50);
         FN += prjFN;
         //FN += ".";
         //FN += subDirName; // suffix
         size_t fileSize = getFileSize(i);
         if (fileSize > 0) {
            char* memFile = new char[fileSize];
            getFileChunk(i, 0, memFile, fileSize);
            FileCache fc;
            fc.create(FN.getChars());
            fc.write(0, memFile, fileSize);
            delete memFile;
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Create a list of files

void MWArchive::createFileListEntry(FileCache& fc, const TCHAR* subDirName, int index)
{
   char prjFN[50];
   getEntryName(index, prjFN, 50);
   size_t fileSize = getFileSize(index);
   size_t fileOffset = getFileOffset (index);

   BGString line;
   if (subDirName != NULL) {
      line += subDirName;
      line += "\\";
      line += prjFN;
   } else
      line = prjFN;
   line += tabChar;
   line.appendInt((int) index);
   line += tabChar;
   line.appendInt((int) fileSize);
   line += tabChar;
   line.appendInt((int) fileOffset);
   line += newLine;
   fc.append(line);
}

void MWArchive::createFileList(const TCHAR* FN)
{
   size_t count = getEntryCount();
   FileCache list;
   list.create(FN);

   //const TCHAR tabChar = 9;
   BGString line = "Filename\tEntry Number\tFile size\tFile offset\n";
   list.append(line);

   // Simple traversion supports only one nested directory, but so do
   // archive types that are currently supported
   for (size_t i = 0; i < count; i++) {
      if (isDirectory (i)) {
         TCHAR subDirName[20];
         getEntryName(i, subDirName, 20);
         enterSubDir(i);
         for (size_t j = 0; j < getEntryCount(); j++) {
            createFileListEntry (list, (TCHAR*) &subDirName, j);
         }
         leaveDir();
      } else {
         createFileListEntry (list, NULL, i);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Calculate file sizes from offset differences

void MWArchive::calculateFileSizesFromOffsetDifferences()
{
   // Calculate file size from distance to next file
   size_t entries = FileOffsets.getSize();
   for (size_t i = 0; i < entries-1; i++) {
      int fileSize = ((int) FileOffsets[i+1]) - ((int) FileOffsets[i]);
      FileSizes[i] = fileSize;
   }

   // File size for last file is computed from distance to EOF
   int _EOF = (int) fc.getFileSize();
   size_t i = entries-1;
   int lastFileOffset = (int) FileOffsets[i];
   if (lastFileOffset < _EOF)
      FileSizes[i] = _EOF - lastFileOffset;
   else
      FileSizes[i] = 0; // Error
}

////////////////////////////////////////////////////////////////////////////////

void MWArchive::addFile (const TCHAR* name, size_t size, size_t offset)
{
   if (name != NULL) {
      size_t nameLen = strlen (name);
      TCHAR* strCopy = new TCHAR[nameLen+1];
      memcpy (strCopy, name, nameLen+1);
      FileNames.add(strCopy);
   } else {
      FileNames.add(NULL);
   }

   FileOffsets.add (offset);
   FileSizes.add (size);
}

void MWArchive::clearFileEntries()
{
   for (size_t i = 0; i < FileNames.getSize(); i++)
      delete FileNames[i];
   FileNames.clear();
   FileSizes.clear();
   FileOffsets.clear();
}

////////////////////////////////////////////////////////////////////////////////
// Get name of entry

bool MWArchive::getEntryName (size_t index, TCHAR* buf, size_t bufSize)
{
   if (index < FileNames.getSize()) {
      //BGString& str = FileNames[index];
      //size_t lastChar = bgmin(bufSize, str.getLength());
      //memcpy (buf, str.getChars(), lastChar);
      const TCHAR* str = FileNames[index];
      if (str != NULL) {
         size_t lastChar = bgmin(bufSize, strlen(str));
         memcpy (buf, str, lastChar);
         buf[lastChar] = 0;
      } else
         buf[0] = 0;

      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Read bufSize bytes from file numbered index at offset into buf

bool MWArchive::getFileChunk(size_t index, size_t offset, void* buf, size_t bufSize)
{
   if (index >= FileSizes.getSize())
      return false;

   if (offset + bufSize > FileSizes[index])
      return false;

   return fc.read(FileOffsets[index], (char*) buf, bufSize);
}

////////////////////////////////////////////////////////////////////////////////
// Return index of file in current directory, or entries+1 if not found

size_t MWArchive::getEntryNumberByName(const TCHAR* name)
{
   //const size_t bufSize = 20;
   size_t nameLen = strlen (name);
   TCHAR* buf = new TCHAR[nameLen+2];
   for (size_t i = 0; i < getEntryCount(); i++) {
      if (getEntryName(i, buf, nameLen+1))
         if (bgstrcmp (buf, name) == 0)
            return i;
   }
   delete buf;

   // Nothing found
   return getEntryCount()+1;
}

////////////////////////////////////////////////////////////////////////////////
// Get memory block for a file inside the archive

MemoryBlock* MWArchive::getMemoryBlock (size_t fileNum)
{
   if (fileNum < getEntryCount()) {
      size_t fileSize = FileSizes[fileNum];
      MemoryBlock* mb = new MemoryBlock(fileSize);
      getFileChunk(fileNum, 0, mb->getPtr (), fileSize);
      return mb;
   } else
      return NULL;
}

MWArchive::MWArchive() {

}

MWArchive::~MWArchive() {
}
