////////////////////////////////////////////////////////////////////////////////
// MechWarrior II PRJ file system
// Copyright Bjoern Ganster 2007-2009
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "MechWarriorIIPRJ.h"
#include "Mesh.h"
#include "FileCache.h"
#include "MW2MechImporter.h"

#ifndef TitanFarPatcher
//#include "TextureCompiler.h" // not needed
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor

MechWarriorIIPRJ::MechWarriorIIPRJ()
{
   m_offset = 0;
   //m_dirName[0] = 0;
   m_mainDirEntries = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

MechWarriorIIPRJ::~MechWarriorIIPRJ()
{
   close();
}

////////////////////////////////////////////////////////////////////////////////
// Open file

bool MechWarriorIIPRJ::open (const TCHAR* PRJFN, bool readOnly)
{
   bool openedSuccessfully;
   if (readOnly)
      openedSuccessfully = fc.openReadOnly(PRJFN);
   else
      openedSuccessfully = fc.openReadWrite(PRJFN);

   // Read main directory
   bool MainDirReadSuccessfully = false;
   if (openedSuccessfully) {
      char sig[4];
      if (fc.getFileSize() > 4) {
         fc.read (0, sig, 4);
         if (memcmp (sig, "PROJ", 4) == 0) {
            if (!fc.read (0x18, (char*) (&m_mainDirEntryCount), 2))
               return false;
            size_t entrySize = 24*m_mainDirEntryCount;
            m_mainDirEntries = new MW2_PRJ_main_directory_entry[m_mainDirEntryCount];
            if (!fc.read (50, (char*) m_mainDirEntries, entrySize))
               return false;
            MainDirReadSuccessfully = readMainDir();
            m_dirHeaders.reserve(m_mainDirEntryCount);
         }
      }
   }

   // Read subdirectory data
   if (MainDirReadSuccessfully) {
      for (size_t i = 0; i < m_mainDirEntryCount; i++) {
         size_t dirEntryOffset = m_mainDirEntries[i].dirOffset;
         MW2_PRJ_directory_header& dirHeader = m_dirHeaders[i];
         fc.read (dirEntryOffset, (char*) (&dirHeader), sizeof (dirHeader));
         size_t dirEntryCount = dirHeader.entries;
         MW2_PRJ_Directory_Entries* entries = new MW2_PRJ_Directory_Entries [dirEntryCount];
         fc.read (dirEntryOffset+sizeof(MW2_PRJ_directory_header), 
                  (char*) entries, sizeof(MW2_PRJ_Directory_Entries)*(dirEntryCount));
         m_dirEntries[i] = entries;
      }
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Close file

void MechWarriorIIPRJ::close()
{
   m_offset = 0xffffFFFF;

   fc.close();

   if (m_mainDirEntries != NULL) {
      for (size_t i = 0; i < m_mainDirEntryCount; i++)
         delete m_dirEntries[i];
      delete m_mainDirEntries;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Read main directory entries

bool MechWarriorIIPRJ::readMainDir()
{
   clearFileEntries();
   m_offset = 0x18; // Marks main directory

   // Read main directory entry names
   for (int i = 0; i < ((int) m_mainDirEntryCount)-1; i++) {
      MW2_PRJ_main_directory_entry& entry = m_mainDirEntries[i];
      BGString str;
      str.copyPart(entry.dirName, 0, 4);
      addFile (str.getChars(), 0, entry.dirOffset);
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Enter subdirectory

bool MechWarriorIIPRJ::enterSubDir (size_t index)
{
   m_dirName.copyPart(m_dirHeaders[index].DirName, 0, 4);
   clearFileEntries();

   // Load & check directory header
   m_offset = m_mainDirEntries[index].dirOffset;
   MW2_PRJ_directory_header& dirHeader = m_dirHeaders[index];

   if (dirHeader.entries > 0) {
      // Read entries
      size_t totalEntries = dirHeader.entries-MW2SkippedFiles; // Count
      MW2_PRJ_Directory_Entries* entryBuf = m_dirEntries[index];
      MW2_PRJ_local_file_header localHeader;

      for (size_t i = MW2SkippedFiles; i < totalEntries; i++) {
         // Read file name from local file header
         // Do not filter out NULL entries, leads to problems for files
         // referenced by number in the PRJ or for replacing files!
         bool OK = false;
         if (entryBuf[i].offset > 0 
         &&  entryBuf[i].offset + entryBuf[i].size < fc.getFileSize())
         {
            if (fc.read(entryBuf[i].offset, (char*) (&localHeader), sizeof(localHeader)))
            {
               OK = true;
               addFile (localHeader.nameWithExt, entryBuf[i].size-62, entryBuf[i].offset);
            } 
         }
         if (!OK)
            addFile (NULL, 0, 0);
      }
   }
   return true;
}

bool MechWarriorIIPRJ::enterSubDir (const char* newDirName)
{
   if (m_dirName.getLength() > 0) {
      if (m_dirName.equals(newDirName))
         return true; // already in that directory
      else
         leaveDir();
   }

   char buf[50];
   size_t i = 0; 
   while (i < getEntryCount()) {
      getEntryName(i, buf, 50);
      if (strcmp (newDirName, buf) == 0) {
         enterSubDir (i);
         return true;
      } else
         i++;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////
// Find directory

size_t MechWarriorIIPRJ::findDirectory(const TCHAR* dirName)
{
   size_t i = 0, dirOffset = 0;
   while (i < m_mainDirEntryCount) {
      BGString testDirName;
      testDirName.copyPart(m_mainDirEntries[i].dirName, 0, 4);
      if (testDirName.equals(dirName)) {
      //if (strcmp (dirName, mainDirEntries[i].dirName) == 0) {
         return i;
      } else
         i++;
   }

   return m_mainDirEntryCount;
}

////////////////////////////////////////////////////////////////////////////////
// Get file name for a file in another directory w/ochanging into that directory

bool MechWarriorIIPRJ::getEntryNameFromOtherDir (const TCHAR* dirName, 
   size_t index, TCHAR* buf, size_t bufSize)
{
   size_t dirNum = findDirectory(dirName);
   if (dirNum == m_mainDirEntryCount)
      return false;

   // Get offset to local header
   MW2_PRJ_Directory_Entries& dirEntry = m_dirEntries[dirNum][index];
   MW2_PRJ_local_file_header localHeader;
   if (fc.read(dirEntry.offset, (char*) (&localHeader), sizeof(localHeader))) {
      bgstrcpymax((char*) buf, (char*) (&localHeader.nameWithExt), sizeof(buf));
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Get file from another directory withoutchanging into that directory

MemoryBlock* MechWarriorIIPRJ::getFileFromOtherDir (const TCHAR* dirName, 
   size_t index)
{
   size_t dirNum = findDirectory(dirName);
   if (dirNum == m_mainDirEntryCount)
      return false;

   // Get file offset
   size_t fileOffset = m_dirEntries[dirNum][index].offset;
   size_t fileSize = m_dirEntries[dirNum][index].size;

   // Read
   MemoryBlock* mb = new MemoryBlock (fileSize);
   if (!fc.read(fileOffset+sizeof(MW2_PRJ_local_file_header), 
                (char*) mb->getPtr(), fileSize)) 
   {
      delete mb;
      return NULL;
   } else
      return mb;
}

////////////////////////////////////////////////////////////////////////////////
// Leave subdirectory

bool MechWarriorIIPRJ::leaveDir()
{
   m_dirName.clear();
   return readMainDir();
}

////////////////////////////////////////////////////////////////////////////////
// Read bufSize bytes from file numbered index at offset into buf

bool MechWarriorIIPRJ::getFileChunk(size_t index, size_t readOffset, void* buf, 
                                    size_t bufSize)
{
   size_t startOfFileOffset = getFileOffset (index);
//   printf("Loaded %i bytes from offset %i\n", bufSize, startOfFileOffset);
   return fc.read(startOfFileOffset + readOffset + 62, (char*) buf, bufSize);
}

////////////////////////////////////////////////////////////////////////////////
// Load MW2 PRJ file into a MemoryBlock

MemoryBlock* MechWarriorIIPRJ::loadFile (const TCHAR* FN)
{
   size_t entryCount = getEntryCount();
   size_t entryNum = entryCount+1;

   // Find index of file to load
   for (size_t i = 0; i < entryCount; i++) {
      TCHAR fnBuf[20];
      getEntryName (i, fnBuf, 19);
      if (strcmp (fnBuf, FN) == 0)
         entryNum = i;
   }

   // Load file
   if (entryNum < entryCount) {
      size_t size = getFileSize (entryNum);
      MemoryBlock* block = new MemoryBlock(size);
      if (getFileChunk(entryNum, 0, block->getPtr(0), size))
         return block;
      else
         delete block;
   }

   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Save PRJ file to new file name

bool MechWarriorIIPRJ::saveToFile (const TCHAR* /*fn*/)
{
   return false; 
}

////////////////////////////////////////////////////////////////////////////////
// Replace numbered file in current directory

/*bool MechWarriorIIPRJ::setFile (size_t fileNum, const char* fileBuf, size_t fileSize)
{
   size_t fileOffset = getFileOffset (fileNum);
   //return fc.read(startOfFileOffset + readOffset + 62, (char*) buf, bufSize);
   return fc.write(fileOffset+62, fileBuf, fileSize);
}*/

///////////////////////////////////////////////////////////////////////////////
// Replace  file

bool MechWarriorIIPRJ::replaceFile (size_t fileNum, const TCHAR* srcFN)
{
   MemoryBlock block;
   block.loadFromFile(srcFN);
   size_t oldSize = getFileSize (fileNum);
   char oldFN[20];
   getEntryName (fileNum, oldFN, 19);
   DWORD oldOffset = (DWORD) getFileOffset (fileNum);

   if (fileNum >= getEntryCount())
      return false;

   if (block.getSize() <= oldSize) {
      printf ("Replacing %s with %s in-place\n", oldFN, srcFN);
      fc.write(oldOffset+62, (char*) block.getPtr(0), block.getSize());
   } else {
      printf ("Replacing %s by appending %s\n", oldFN, srcFN);

      // Update directory entry
      MW2_PRJ_Directory_Entries entry;
      entry.offset = fc.getFileSize();
      entry.size = block.getSize()+62;
      size_t dirEntryOffset = m_offset + sizeof(MW2_PRJ_directory_header) + 
                              fileNum * sizeof(entry);
      fc.write(dirEntryOffset, (char*) (&entry), sizeof(entry));

      // Update old local file header and copy it to the end PRJ
      MW2_PRJ_local_file_header header;
      fc.read(oldOffset, (char*) (&header), sizeof(header));
      header.SizeOfThisFile = block.getSize() + 54;
      fc.append((char*) (&header), sizeof(header));

      // Append new file to the PRJ
      fc.append((const char*) block.getPtr(0), block.getSize());
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Add files to archive

void MechWarriorIIPRJ::addFileToArchive (const TCHAR* FN)
{ 
   MemoryBlock block;
   if (!block.loadFromFile(FN))
      return;

   // Load directory header
   MW2_PRJ_directory_header dirHeader;
   fc.read(m_offset, (char*) (&dirHeader), sizeof(dirHeader));

   // Fill local header for new file
   //DWORD newFileHeaderOffset = fc.getFileSize();
   //DWORD newFileOffset = newFileHeaderOffset + sizeof (MW2_PRJ_local_file_header);
   MW2_PRJ_local_file_header localFileHeader;
   localFileHeader.marker[0] = 'D';
   localFileHeader.marker[1] = 'A';
   localFileHeader.marker[2] = 'T';
   localFileHeader.marker[3] = 'A';
   localFileHeader.FileNumberInParentDir = getEntryCount();

   size_t pos;
   bgstrrscan(FN, OSPathSep, strlen(FN)-1, 0, pos);
   const char* fnWOPath = FN+pos+1;
   bgstrcpymax(localFileHeader.nameWithExt, fnWOPath, sizeof(localFileHeader.nameWithExt));
   bgstrcpymax((char*) &localFileHeader.nameWithExt, fnWOPath, sizeof(localFileHeader.nameWithExt));
   bgstrcpymax((char*) &localFileHeader.nameWithoutExt, fnWOPath, sizeof(localFileHeader.nameWithoutExt));
   bgstrReplace (localFileHeader.nameWithoutExt, '.', (char) 0);
   bgstrcpymax (localFileHeader.parentDir, m_dirName.getChars(), sizeof(localFileHeader.parentDir));
   localFileHeader.SizeOfThisFile = block.getSize();

   // Write local header
   size_t localHeaderOffset = fc.getFileSize();
   fc.append ((const char*) (&localFileHeader), sizeof (localFileHeader));

   // Append new file
   fc.append ((const char*) block.getPtr(), block.getSize());

   // Write new directory header
   ++dirHeader.entries;
   DWORD newDirOffset = fc.getFileSize();
   fc.append ((char*) (&dirHeader), sizeof(dirHeader));

   // todo: Copy old directory entries and append new entry
   DWORD* entryBuf = new DWORD [2*dirHeader.entries];
   fc.read (m_offset+30, (char*) entryBuf, 8*(dirHeader.entries-1));
   entryBuf[2*dirHeader.entries-2] = localHeaderOffset;
   entryBuf[2*dirHeader.entries-1] = block.getSize();
   fc.append ((char*) entryBuf, 8*dirHeader.entries);
   delete entryBuf;

   // Find and update offset to new directory in the main directory table
   int MainDirEntryCount = 0; // Clear higher-order bits
   fc.read (m_offset, (char*) (&MainDirEntryCount), 2);
   MainDirEntryCount--;
   for (int i = 0; i < MainDirEntryCount; i++) {
      size_t dirOffsetInMainDir = 54 + 24*i;
      DWORD testedOffset;
      fc.read (dirOffsetInMainDir, (char*) (&testedOffset), sizeof (DWORD));
      if (testedOffset == m_offset) {
         fc.write (dirOffsetInMainDir, (const char*) (&newDirOffset), 4);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Read bufSize bytes from file numbered index at offset into buf

#ifndef TitanFarPatcher
Texture* MechWarriorIIPRJ::loadTexture (const unsigned char* buf, 
                                        size_t bufSize, MemoryBlock* palette)
{
   if (palette == NULL)
      return NULL;
   size_t ysize = buf[2] + 256*buf[3];
   size_t xsize = buf[0] + 256*buf[1];
   size_t size = xsize*ysize+4;

   if (bufSize == size) {
      Texture* texture = new Texture();
      texture->resize (xsize, ysize, GL_RGBA);
      for (unsigned int x = 0; x < xsize; x++)
         for (unsigned int y = 0; y < ysize; y++) {
            unsigned char c = buf[x+(ysize-y-1)*xsize+4];
            if (c != 0xFF) {
               char r = 4*palette->getByte(3*c+0);
               char g = 4*palette->getByte(3*c+1);
               char b = 4*palette->getByte(3*c+2);
               texture->setRGB(x, y, r, g, b);
            } else
               texture->setRGBA(x, y, 255, 255, 255, 255);
         }
      return texture;
   } else
      return NULL;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// TEX file introduced in MW2:Mercs

#ifndef TitanFarPatcher
Texture* MechWarriorIIPRJ::loadTEX (const unsigned char* buf, size_t bufSize)
{
   if (bufSize < 20)
      return NULL;
   if (*((DWORD*) (buf)) != 0x32322e31)
      return NULL;

   WORD format = *((WORD*) (&buf[4]));
   WORD xsize = *((WORD*) (&buf[6]));
   WORD ysize = *((WORD*) (&buf[8]));
   const size_t colorTableStart = 12;
   WORD colorTableSize = *((WORD*) (&buf[10]));
   const size_t colorEntrySize = 4;
   WORD texelStart = colorTableStart+colorEntrySize*colorTableSize;
   size_t fsize = texelStart+xsize*ysize;

   if (fsize != bufSize)
      return NULL;

   printf ("Format: %i\n", format);
   Texture* texture = new Texture(xsize, ysize, GL_RGBA);
   for (unsigned int y = 0; y < ysize; y++) {
      for (unsigned int x = 0; x < xsize; x++) {
         unsigned char r, g, b, a, i;
         i = buf[texelStart+y*xsize+x];
         if (i < colorTableSize) {
            i = colorTableStart+colorEntrySize*i;
            b = buf[i];
            g = buf[++i];
            r = buf[++i];
            a = buf[++i];
            texture->setRGBA(xsize-x-1, ysize-y-1, r, g, b, a);
         }
      }
   }

   return texture;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#ifndef TitanFarPatcher
Texture* MechWarriorIIPRJ::loadSHP (const MemoryBlock& mb, size_t picNum,
                                    MemoryBlock* palette)
{
   // Read and check basic numbers
   if (mb.getSize() < 8 + 16 * picNum)
      return NULL;

   size_t picCount = mb.getDWord(4);
   if (picNum >= picCount || picCount == 0)
      return NULL;

   size_t picStart = mb.getDWord(8+8*picNum);
   if (picStart > mb.getSize())
      return NULL;

   //size_t xsize = mb.getWord (picStart+2);
   //size_t ysize = mb.getWord (picStart);
   size_t xsize = mb.getWord (picStart+16);
   size_t ysize = mb.getWord (picStart+20);
   const unsigned char* buf = mb.getPtr(0);

   // Check header
   if (mb.getByte(0) == '1'
   &&  mb.getByte(1) == '.'
   &&  mb.getByte(2) == '1'
   &&  mb.getByte(3) == '0'
   &&  xsize > 0
   &&  ysize > 0
   &&  xsize < 1000
   &&  ysize < 1000)
//   &&  buf[picStart+16] == buf[picStart+2]
   //&&  buf[picStart+17] == buf[picStart+3]
   //&&  buf[picStart+20] == buf[picStart]
   //&&  buf[picStart+21] == buf[picStart+1] )
   {
      // Load palette
      char r[256], b[256], g[256]; // Palette
      size_t palOffset = mb.getDWord(12);
      if (palOffset > 0) {
         // Use in-file palette
         size_t colorCount = mb.getByte(palOffset);
         const double colorScale = 2.0;
         for (size_t i = 0; i <= colorCount; i++)
         { 
            unsigned char j = mb.getByte(palOffset+4*i);
            r[j] = (char) (colorScale * mb.getByte(palOffset+4*i+1));
            g[j] = (char) (colorScale * mb.getByte(palOffset+4*i+2));
            b[j] = (char) (colorScale * mb.getByte(palOffset+4*i+3));
         }
      } else if (palette != NULL) {
         // Use previously loaded palette
         const double colorScale = 2.0;
         for (size_t i = 0; i < palette->getSize()/3; i++)
         { 
            r[i] = (char) (colorScale * palette->getByte(3*i+0));
            g[i] = (char) (colorScale * palette->getByte(3*i+1));
            b[i] = (char) (colorScale * palette->getByte(3*i+2));
         }
      } else {
         // Use default palette
         //const double colorScale = 2.0;
         for (size_t i = 0; i < 256; i++)
         { 
            r[i] = (char) i;
            g[i] = (char) i;
            b[i] = (char) i;
         }
      }

      // Init pixels
      xsize++; ysize++;
      Texture* texture = new Texture(xsize, ysize);
      for (size_t x = 0; x < texture->getWidth(); x++)
         for (size_t y = 0; y < texture->getHeight(); y++)
            texture->setRGB(x, y, 0, 0, 0);

      // Load pixels
      size_t x = 0, y = texture->getHeight()-1, offset = picStart+16+8*picCount;
      while (y < ysize && offset < mb.getSize()) {
         unsigned char ctrl = buf[offset];
         unsigned char data = buf[offset+1];

         if (ctrl == 0) {
            x = 0; y--; offset++;
         } else if (ctrl == 1) {
            x += data; offset += 2;
         } else if ((ctrl & 1) == 0) {
            // Write data (ctrl/2) times
            for (int j = 0; j < (ctrl >> 1); j++) {
               texture->setRGB(x, y, r[data], g[data], b[data]);
               x++;
            }
            offset += 2;
         } else {
            // Write (ctrl/2) bytes
            offset++;
            for (int j = 0; j < (ctrl >> 1); j++) {
               unsigned char c = buf[offset];
               texture->setRGB(x, y, r[c], g[c], b[c]);
               x++;
               offset++;
            }
         }
      }

      return texture;
   } else
      return NULL;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Read bufSize bytes from file numbered index at offset into buf

#ifndef TitanFarPatcher
Texture* MechWarriorIIPRJ::loadPCX (const unsigned char* buf, size_t bufSize)
{
   int w = *((WORD*) (&buf[8]));
   int h = *((WORD*) (&buf[10]));
   unsigned char palR[256], palG[256], palB[256];

   // All PCX enountered need to have an odd number of pixels?
   if ((w & 1) == 0)
      w++;

   // Load the palette
   size_t i = 0;
   size_t offset = bufSize - 768;
   while (buf[offset] != 0x0c && offset > 0)
      offset--;
   while (i < 256) {
      palR[i] = buf[++offset];
      palG[i] = buf[++offset];
      palB[i] = buf[++offset];
      i++;
   }

   // Load the image
   Texture* texture = new Texture (w, h, GL_RGB);
   offset = 128;
   int x = 0;
   int y = h-1;
   while (y > 0 && offset < bufSize) {
      unsigned char b = buf[offset];
      offset++;
      if (b < 0xC0) {
         texture->setRGB(x, y, palR[b], palG[b], palB[b]);
         x++;
         if (x > w) {
            x = 0;
            y--;
         }
      } else {
         // Repeat byte x-C0h times
         unsigned char c = buf[offset++];
         while (b > 0xc0) {
            texture->setRGB(x, y, palR[c], palG[c], palB[c]);
            x++;
            if (x > w) {
               x = 0;
               y--;
            }
            if (y > h)
               b = 0;
            else
               b--;
         }
      }
   }

   return texture;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Read mesh from WTB

#ifndef TitanFarPatcher
Mesh* MechWarriorIIPRJ::loadWTB (const MemoryBlock* block)
{
   //bool flipPolygons = true; // Change polygon winding
   bool flipPolygons = false; // Do not change polygon winding
   size_t pointRecordSize = 40;
   size_t pointBaseOffset = 32;
   size_t shortPolyRecordSize = 40;
   size_t longPolyRecordSize = 46;
   size_t longPolyRecordVertices = 5;
   size_t pointCountInPolygon = 30;
   size_t firstPointInPolygon = 32;

   if (block->getSize() > 32
   &&  memcmp (block->getPtr(0), "WTBO", 4) == 0)
   {
      size_t pointCount = block->getWord (0x18);
      size_t polygonCount = block->getWord (0x1a);
      size_t bc = pointCount*pointRecordSize + polygonCount*shortPolyRecordSize + 
                  pointBaseOffset;

      if (bc > block->getSize()) {
         // Try alternate access constants
         //flipPolygons = true; // Change polygon winding
         flipPolygons = false; // Change polygon winding
         pointRecordSize = 16;
         pointBaseOffset = 32;
         shortPolyRecordSize = 12;
         longPolyRecordSize = 18;
         longPolyRecordVertices = 5;
         pointCountInPolygon = 2;
         firstPointInPolygon = 4;
         bc = pointCount*pointRecordSize + polygonCount*shortPolyRecordSize + 
                 pointBaseOffset;
      }

      if (bc <= block->getSize() && pointCount > 0 && polygonCount > 0) {
         Mesh* mesh = new Mesh();

         // Read points
         size_t offset = pointBaseOffset;
         for (size_t i = 0; i < pointCount; i++) {
            int x = block->getDWord (offset);
            int y = block->getDWord (offset+4);
            int z = block->getDWord (offset+8);
            mesh->addPoint(x, y, z);
            offset += pointRecordSize;
         }

         // Read polygons
         while (   offset + shortPolyRecordSize <= block->getSize()
                && mesh->getPolygonCount() < polygonCount)
         {
            size_t count = block->getWord (offset+pointCountInPolygon);
            if (offset + 2*count+firstPointInPolygon <= block->getSize()
            &&  count > 0) 
            {
               MeshPolygon* poly = mesh->addPolygon(count);
               for (size_t i = 0; i < count; i++) {
                  size_t pointNum;
                  pointNum = block->getWord (offset+firstPointInPolygon+i*2);
                  if (pointNum >= mesh->getPointCount()) 
                     pointNum = 0;
                  if (flipPolygons)
                     poly->setPoint (count-i-1, pointNum);
                  else
                     poly->setPoint(i, pointNum);
               }
            }

            // Find next marker
            if (count < longPolyRecordVertices)
               offset += shortPolyRecordSize;
            else
               offset += longPolyRecordSize;
         }

         //printf ("Loaded polygon with %i vertices and %i polygons\n",
         //   mesh->getPointCount(), mesh->getPolygonCount());

         return mesh;
      } else {
         printf("Size prediction error\n");
      }
   } else {
      printf ("Not a WTBO file?\n");
   }

   return NULL;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Export BWD file as XML

#ifndef TitanFarPatcher
bool MechWarriorIIPRJ::BWD2XML (const MemoryBlock& mb, XMLTreeIterator& iter, 
                                bool followIncludes, const char* WTB_outputDir)
{
   // Check signature
   if (mb.getByte(0) != 'B'
   ||  mb.getByte(1) != 'W'
   ||  mb.getByte(2) != 'D')
      return false;

   XMLTree* tree = new XMLTree ();
   size_t offset = 0x34;
   bool insideRepresentation = false;

   // Visit all BWD tags and update XML tree 
   while (offset < mb.getSize()) {
      BGString tagName;
      tagName.copyPart ((const char*) mb.getPtr(offset), 0, 4);
      size_t tagSize = mb.getDWord(offset+4);

      if (tagName.getLength() > 4)
         tagName[4] = 0;

      /*if (tagName.equals("MON")) {
         size_t childNum = iter.addChild("Mech");
         iter.goToChild(childNum);
      } else if (tagName.equals("MOFF")) {
         iter.goToParent();
      } else*/ if (tagName.equals("BLK")) {
         size_t childNum = iter.addChild("Block");
         iter.goToChild(childNum);
      } else if (tagName.equals("ENDB")) {
         iter.goToParent();
      } else if (tagName.equals("REPR")) {
         if (insideRepresentation) 
            iter.goToParent();
         else
            insideRepresentation = true;
         size_t childNum = iter.addChild("Representation");
         iter.goToChild(childNum);
      } else if (tagName.equals("ENDR")) {
         iter.goToParent();
         insideRepresentation = false;
      } else if (tagName.equals("OBJ")) {
         int rel = mb.getInt16(offset+10);
         bool found = false;
         if (rel >= 0) {
            int i = 0;
            while (i < (int) iter.getChildCount() && !found) {
               iter.goToChild(i);
               if (iter.getIntAttributeValueByName("num") == rel) {
                  found = true;
               } else {
                  ++i;
                  iter.goToParent();
               }
            }
         }

         size_t childNum = iter.addChild("Object");
         iter.goToChild(childNum);

         int16 fileNum = mb.getInt16(offset+56);
         char buf[50];
         getEntryNameFromOtherDir ("POLY", fileNum, buf, 50);

         // Export WTB file?
         if (WTB_outputDir != NULL) {
            MemoryBlock* mb = getFileFromOtherDir("POLY", fileNum);
            if (mb != NULL) {
               Mesh* mesh = MechWarriorIIPRJ::loadWTB(mb);
               if (mesh != NULL) {
                  BGString objFN = buf;
                  objFN += ".obj";
                  BGString objFNwithPath = WTB_outputDir;
                  objFNwithPath.appendNoDups(OSPathSep);
                  objFNwithPath += objFN.getChars();
                  mesh->saveToObj(objFNwithPath.getChars());

                  BGString wtbFN = WTB_outputDir;
                  wtbFN.appendNoDups(OSPathSep);
                  wtbFN += buf;
                  mb->saveToFile(wtbFN.getChars());

                  delete mesh;
               }
               delete mb;
            }
         } else {
            iter.addAttribute("mw2prj", buf);
         }

         iter.addAttribute("num", mb.getInt16(offset+8));
         iter.addAttribute("rel", rel);
         iter.addAttribute("tx", mb.getInt16(offset+38));
         iter.addAttribute("ty", mb.getInt16(offset+42));
         iter.addAttribute("tz", mb.getInt16(offset+46));

         if (found)
            iter.goToParent();

         iter.goToParent();
      } else if (tagName.equals("OBJL")) {
         size_t childNum = iter.addChild("ObjectLink");
         iter.goToChild(childNum);
         int16 val1 = mb.getInt16 (offset+8);
         int16 val2 = mb.getInt16 (offset+10);
         BGString str;
         str.assignInt(val1);
         iter.addAttribute ("val1", str.getChars());
         str.assignInt(val2);
         iter.addAttribute ("val2", str.getChars());
         iter.goToParent();
      } else if (tagName.equals("THNG")) {
         size_t childNum = iter.addChild("THNG");
         iter.goToChild(childNum);
         DWORD val = mb.getDWord (offset+8);
         BGString str;
         str.assignInt(val);
         iter.addAttribute ("value", str.getChars());
         iter.goToParent();
      } else if (tagName.equals("TSK")) {
         DWORD val = mb.getDWord (offset+8);
         int16 val2 = mb.getInt16 (offset+8);
         if (val != 3 || val2 != 3) {
            size_t childNum = iter.addChild("TSK");
            iter.goToChild(childNum);
            BGString str;
            str.assignInt(val);
            iter.addText (str.getChars());
            str.assignInt(val2);
            iter.addText (str.getChars());
            iter.addText ((const char*) mb.getPtr (offset+14));
            iter.goToParent();
         }
      } else if (tagName.equals("INCL") && tagSize == 20) {
         BGString fn;
         fn.copyPart((const char*) mb.getPtr(offset + 10), 0, 9);
         if (followIncludes) {
            WORD fileNo = mb.getWord(offset+8);
            MemoryBlock* otherMB = getMemoryBlock(fileNo); //fn.getChars());
            if (otherMB != NULL) {
               printf("Entering %s, %i\n", fn.getChars(), fileNo);
               BWD2XML(*otherMB, iter, true, WTB_outputDir);
               delete otherMB;
               printf ("Done with %s, %i\n", fn.getChars(), fileNo);
            } else {
               printf("Unable to get %s\n", fn.getChars());
            }
         } else {
            size_t childNum = iter.addChild("include");
            iter.goToChild(childNum);
            iter.addAttribute("file", fn.getChars());
            int16 val = mb.getWord (offset + 8);
            BGString str;
            str.assignInt(val);
            iter.addAttribute ("num", str.getChars());
            iter.goToParent();
         }
      } else if (tagName.equals("BTHG") && tagSize == 12) {
         size_t childNum = iter.addChild("BTHG");
         iter.goToChild(childNum);
         int16 val = mb.getWord (offset + 8);
         BGString str;
         str.assignInt(val);
         iter.addAttribute ("num", str.getChars());
         iter.goToParent();
      } else if (tagName.equals("XPLO") && tagSize == 16) {
         size_t childNum = iter.addChild("explosion");
         iter.goToChild(childNum);
         for (int i = 0; i < 4; i++) {
            BGString attrStr = "val";
            attrStr.appendInt(i);
            BGString valStr;
            valStr.assignInt(mb.getWord(offset+8+2*i));
            iter.addAttribute(attrStr.getChars(), valStr.getChars());
         }
         iter.goToParent();
      } else if (tagName.equals("BMID") && tagSize == 12) {
         size_t childNum = iter.addChild("BitmapID");
         iter.goToChild(childNum);
         int16 val1 = mb.getInt16 (offset+8);
         int16 val2 = mb.getInt16 (offset+10);
         BGString str;
         str.assignInt(val1);
         iter.addAttribute ("val1", str.getChars());

         if (val2 != -1) {
            printf ("BMID tag has an unusual value: %i\n", val2);
         }

         //str.assignInt(val2);
         //iter.addAttribute ("val2", str.getChars());
         iter.goToParent();
      } else if (tagName.equals("BMPJ") && tagSize == 20) {
         size_t childNum = iter.addChild("includeBMP");
         iter.goToChild(childNum);
         int16 val = mb.getWord (offset + 8);
         BGString str;
         str.assignInt(val);
         iter.addAttribute ("num", str.getChars());
         str.copyPart((const char*) mb.getPtr(offset + 10), 0, 9);
         str += ".XEL";
         iter.addAttribute("file", str.getChars());
         iter.goToParent();
      } else {
         size_t childNum = iter.addChild(tagName.getChars());
         iter.goToChild(childNum);
         BGString str;
         str.assignInt((int) tagSize);
         iter.addAttribute ("length", str.getChars());
         iter.goToParent();

         str = "Unknown tag ";
         str += tagName.getChars();
         str += ", length ";
         str.appendInt ((int) tagSize);
         str.printLn();
      }

      offset += tagSize;
   }

   return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Convert SFL to Wav file

/*void decodeBlock4 (size_t& offset, size_t& wavOffset, const MemoryBlock* mb, 
                   MemoryBlock* wavBlock)
{
   unsigned char c, lastChar = 128;
   ++offset;

   if (wavOffset+64 >= wavBlock->getSize())
      wavBlock->resize(2*wavBlock->getSize()+64);

   for (size_t i = 0; i < 64; i++) {
      c = mb->getByte(offset+i+16);

      //wavBlock->setByte(wavOffset++, 0);
      //wavBlock->setByte(wavOffset++, 0);

      wavBlock->setByte(wavOffset++, c);
      wavBlock->setByte(wavOffset++, c);
   }
   offset += 80;
}

MemoryBlock* MechWarriorIIPRJ::SFL2Wav (const MemoryBlock* mb)
{
   DWORD blockCount = mb->getDWord(8);
   size_t maxBlockSize = mb->getWord(12);
   DWORD BitsPerSample = 8; // 8, 16, or 24
   WORD channels = 1; // 1 for mono, 2 for stereo
   DWORD SampleRate = 11025;
   DWORD BlockAlign = SampleRate*BitsPerSample/8;
   DWORD formatTag = 1;

   // Create header for copy in WAV format
   // http://de.wikipedia.org/wiki/WAV_(Format)
   MemoryBlock* wavBlock = new MemoryBlock(blockCount * maxBlockSize + 44);
   wavBlock->copy(0, (unsigned char*) "RIFF    WAVE", 12);
   wavBlock->setDWord(4, (DWORD) wavBlock->getSize()-8);

   // fmt chunk
   wavBlock->copy(12, (unsigned char*) "fmt ", 4);
   wavBlock->setDWord (16, 16); // length of remaining fmt header
   wavBlock->setWord (20, formatTag); 
   wavBlock->setWord (22, channels);
   wavBlock->setDWord (24, SampleRate); // sample rate
   wavBlock->setDWord (28, SampleRate); //*BlockAlign);
   wavBlock->setDWord (32, 1); //BlockAlign); 
   wavBlock->setDWord (34, BitsPerSample); // mono

   // Data chunk
   wavBlock->copy (36, (unsigned char*) "data", 4); // mono
   size_t wavOffset = 44;
   size_t offset = 14;
   DWORD blockNum = 0;
   size_t blockOffset = maxBlockSize;
   unsigned char c, lastChar = 128;

   int stats[256];

   for (size_t i = 0; i < 256; i++)
      stats[i] = 0;

   do {
      c = mb->getByte(offset);
      ++stats[c];

      if (c == 2) {
         offset += 19;
      } else if (c == 3) {
         offset += 37;
      } else if (c == 4) {
         //decodeBlock4 (offset, wavOffset, mb, wavBlock);
         offset += 81;
      } else if (c == 5) {
         ++offset;
         size_t count = bgmin (maxBlockSize, mb->getSize()-offset);
         if (wavOffset + count > wavBlock->getSize())
            wavBlock->resize(2*(mb->getSize()+count));
         wavBlock->copy (wavOffset, mb->getPtr (offset), count);
         offset += count;
         wavOffset += count;
      } else {
         BGString str = "SFL block ";
         str.appendInt (c);
         str += " at offset ";
         str.appendInt((DWORD) offset);

         //++offset;
         size_t startOffset = offset;

         bool done = false;
         do {
            ++offset;
            if (offset < mb->getSize()) {
               c = mb->getByte(offset);
               done = (c >= 2) && (c <= 5);
            } else
               done = true;
         } while (!done);

         str += ", block size ";
         str.appendInt((int) (offset-startOffset));
         str.printLn();
      }

      if (wavOffset >= wavBlock->getSize())
         wavBlock->resize (2*wavBlock->getSize());
   } while (offset < mb->getSize());

   for (int i = 0; i < 256; i++)
      if (stats[i] > 0) {
         BGString str;
         str.appendInt(i);
         str += ": ";
         str.appendInt (stats[i]);
         str.printLn();
      }

   wavBlock->setDWord(4,  (DWORD) wavOffset-8); // file size
   wavBlock->setDWord(40, (DWORD) wavOffset-44); // data chunk size
   wavBlock->resize (wavOffset);
   return wavBlock;
} */

#define MAX_BLOCKSIZE 256 // well, _i_ haven't seen one larger than this :P

// utility function.  linearly interpolates the string of (size)-many bytes in
// ptr to get a string of (size*2)-many bytes
void interpolate2(char* incoming, int size) {
    char buffer[MAX_BLOCKSIZE * 4];
    char *bufferptr = buffer, *dataptr = incoming;

    // while we still have bytes left...
    while (dataptr - incoming < size) {
        // ... interpolate
        *bufferptr = *dataptr;
        *(bufferptr+1) = (*dataptr + *(dataptr+1))/2;
        bufferptr += 2;
        dataptr++;
    }

    memcpy(incoming, buffer, size*2);

    return;
}

// same thing, but outputs a string of length (size*4) bytes
void interpolate4(char* incoming, int size) {
    char buffer[MAX_BLOCKSIZE * 4];
    char *bufferptr = buffer, *dataptr = incoming;

    // while we still have bytes left...
    while (dataptr - incoming < size) {
        // ... interpolate
        *bufferptr = *dataptr;
        *(bufferptr+1) = (*dataptr * 3 + *(dataptr+1))/4;
        *(bufferptr+2) = (*dataptr + *(dataptr+1))/2;
        *(bufferptr+3) = (*dataptr + *(dataptr+1) * 3)/4;
        bufferptr += 4;
        dataptr++;
    }

    memcpy(incoming, buffer, size*4);

    return;
}

// factored out code that does the table and dpcm decoding, look for invocation
// in the block decoding loop in sflx.
#define MAX_DELTAS      16      // 4 bits max means 16 deltas max
#define SILENCE         0x80    // pcm is shifted by 128, this is secretly zero
int process_dpcm(char *inptr, char *frame, int framesize,
               unsigned int *accumulator, int bits_per_word) {
    long deltas[MAX_DELTAS];
    int frame_index = 0;
    int ret = 0;

    // grab our deltas
    int delta_index;
    for (delta_index = 0; delta_index < (1 << bits_per_word); delta_index++) {
        deltas[delta_index] = (inptr[ret++] << 1) - SILENCE;
    }

    do {
        int word_index;
        int in_word = *(inptr + ret);
        ret++;

        for (word_index = 0; word_index < (8 / bits_per_word); word_index++) {
            // build the new byte from the appropriate delta and
            // whatever's stored in the DPCM accumulator
            int new_byte =
                deltas[(in_word >> word_index*bits_per_word) &
                       ((1 << bits_per_word) - 1)] + *accumulator;

            // clip it, clip it good
            if (new_byte > 127)
                new_byte = 127;
            else if (new_byte < -127)
                new_byte = -127;

            // store our processed byte to the accumlator and to
            // the frame, shift our frame position
            *accumulator = new_byte;
            frame[frame_index++] = new_byte + SILENCE;
        }
    } while (frame_index < framesize);

    return ret;
}

// a number of flag masks used in the sflx format
#define HALF_FRAME      0x40
#define QUARTER_FRAME   0x80
#define CONTROL_MASK    0x0f
#define CONTROL_ZEROES  0x00
#define CONTROL_NOP     0x01
#define CONTROL_RAW     0x05
#define CONTROL_DPCM1   0x02 // use one bit to choose a delta
#define CONTROL_DPCM2   0x03 // use two bits to choose a delta
#define CONTROL_DPCM4   0x04 // use four bits to choose a delta

// sits at the start of a sflx file.  the __attribute__ compiler tag is to
// signal gcc not to pad this out to 16 bytes.  don't know if there's an msvc
// equivalent, since that's what MechVM seems to be using.
#pragma pack(2) // MW2 file data are WORD aligned?
typedef struct {
    char title[4];
    long filesize;
    long block_count;
    short blocksize;
} SFLX_HEADER;

// decodes an SFLX file sitting in incoming, writes pcm to outgoing
char* sflx(char *incoming, char *outgoing) {
    // keep track of where we're writing to and reading from
    char* outptr = outgoing, *inptr = incoming;
    // used for the D in DPCM :)
    unsigned int accumulator = 0;
    // the rest will get set up in the header
    long iter = 0, blocksize = 0;

    // decode the header to see how many blocks we're expecting
    SFLX_HEADER *header = (SFLX_HEADER*)incoming;
    blocksize = header->blocksize;
    iter = header->block_count;
    inptr += sizeof(SFLX_HEADER);

    // now we loop through the blocks
    do {
        char frame[MAX_BLOCKSIZE * 4];
        int framesize;
        int header = *inptr;

        // the basic structure of the block is to have a header at the front,
        // which contains info about ...
        //  1) what sample rate we're storing, and
        //  2) what decoding method to use.
        // this is the information that we strip out first.

        // use the header to decode how large we want our frame to be
        if (header & HALF_FRAME)
            framesize = blocksize / 2;
        else if (header & QUARTER_FRAME)
            framesize = blocksize / 4;
        else
            framesize = blocksize;

        inptr++; // start looking at actual data
        switch (*(inptr-1) & CONTROL_MASK) { // various decoding methods follow:

            case CONTROL_ZEROES:
                // this control code means we fill the frame with silence
                memset(frame, SILENCE, framesize);
                accumulator = 0;
            break;

            case CONTROL_NOP:
                // don't do anything.
            break;

            // from here, all the dpcm blocks are similarly structured.
            // following their one-byte header, which contains information
            // about the granularity of the sound to follow (to be explained in
            // a moment), they contain a table of signed values we call deltas.
            // after we read in the delta table, all the data that follows is
            // broken down into 1, 2, or 4-bit "words".  each of these words
            // selects which delta value to use, which we add to whatever our
            // pcm was just sitting at (hence "delta") and then loop.

            case CONTROL_DPCM1:
                inptr += process_dpcm(inptr, frame, framesize, &accumulator, 1);
            break;

            case CONTROL_DPCM2:
                inptr += process_dpcm(inptr, frame, framesize, &accumulator, 2);
            break;

            case CONTROL_DPCM4:
                inptr += process_dpcm(inptr, frame, framesize, &accumulator, 4);
            break;

            case CONTROL_RAW:
                memcpy(frame, inptr, framesize);
                inptr += framesize;
                accumulator = frame[framesize - 1] - 0x80;
            break;

            // shouldn't ever get here, i don't think.
            default:
                fprintf(stderr, "Error decoding SFLX file (%d).\n", *(inptr-1) & CONTROL_MASK);
            break;
        }

        // now we need to rescale if this segment was sampled at a lower rate
        if (header & HALF_FRAME)
            interpolate2(frame, blocksize);
        else if (header & QUARTER_FRAME)
            interpolate4(frame, blocksize);

        // finally, store the data to outptr
        memcpy(outptr, frame, blocksize);
        outptr += blocksize;
    } while (--iter);

    return outgoing;
}

MemoryBlock* MechWarriorIIPRJ::SFL2Wav (const MemoryBlock* mb)
{
   DWORD blockCount = mb->getDWord(8);
   size_t maxBlockSize = mb->getWord(12);
   DWORD BitsPerSample = 8; // 8, 16, or 24
   WORD channels = 1; // 1 for mono, 2 for stereo
   DWORD SampleRate = 11025;
   //DWORD BlockAlign = SampleRate*BitsPerSample/8;
   WORD formatTag = 1;

   // Create header for copy in WAV format
   // http://de.wikipedia.org/wiki/WAV_(Format)
   MemoryBlock* wavBlock = new MemoryBlock(blockCount * maxBlockSize + 44);
   //ZeroMemory(wavBlock->getPtr(), wavBlock->getSize());
   wavBlock->fill (0, blockCount * maxBlockSize + 44, 0);
   wavBlock->copy(0, (unsigned char*) "RIFF    WAVE", 12);
   wavBlock->setDWord(4, (DWORD) wavBlock->getSize()-8);

   // fmt chunk
   wavBlock->copy(12, (unsigned char*) "fmt ", 4);
   wavBlock->setDWord (16, 16); // length of remaining fmt header
   wavBlock->setWord (20, formatTag); 
   wavBlock->setWord (22, channels);
   wavBlock->setDWord (24, SampleRate); // sample rate
   wavBlock->setDWord (28, SampleRate); //*BlockAlign);
   wavBlock->setDWord (32, 1); //BlockAlign); 
   wavBlock->setDWord (34, BitsPerSample); // mono

   // Data chunk
   wavBlock->copy (36, (unsigned char*) "data", 4); // mono
   size_t wavOffset = 44;
   //size_t offset = 14;
   //DWORD blockNum = 0;
   //size_t blockOffset = maxBlockSize;
   //unsigned char c, lastChar = 128;

   sflx((char*) mb->getPtr(), (char*) wavBlock->getPtr()+wavOffset);

   wavBlock->setDWord(4,  (DWORD) wavBlock->getSize()-8); // file size
   wavBlock->setDWord(40, (DWORD) wavBlock->getSize()-44); // data chunk size
   //wavBlock->resize (wavOffset);
   return wavBlock;
}

////////////////////////////////////////////////////////////////////////////////
// Decode MW2 555 image

Texture* Decode_MW2_555 (unsigned char* mem, size_t size)
{
   if (size < 4)
      return NULL;

   size_t w = *((WORD*)(&mem[0]));
   size_t h = *((WORD*)(&mem[2]));

   if (size < 2*w*h+4)
      return NULL;

   Texture* t = new Texture (w, h);

   for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
         size_t index = 2*(y*w+x)+4;
         unsigned char b1 = mem[index++];
         unsigned char b2 = mem[index++];

         // r5g6b5
         /*unsigned char b = b1 & 31;
         unsigned char g = (b1 >> 5) + ((b2 & 7) << 3);
         unsigned char r = (b2 >> 3);*/

         // r5g5b5a1
         /*unsigned char b = b1 & 31;
         unsigned char g = (b1 >> 5) + ((b2 & 3) << 3);
         unsigned char r = (b2 >> 3);
         unsigned char a = (b2 & 128);*/

         // a1r5g5b5
         unsigned char a = 1; //(b2 & 128);
         unsigned char r = (b2 >> 2) & 7;
         unsigned char g = (b2 & 3) + ((b1 & 7) << 2);
         //unsigned char b = ((b2 & 3) << 3) + (b1 & 7);
         unsigned char b = b1 >> 3;

         if (a > 0)
            t->setRGBA (x, h-y-1, r << 3, g << 3, b <<3, 255);
         else
            t->setRGBA (x, h-y-1, 0, 0, 0, 0);
      }
   }

   return t;
}

