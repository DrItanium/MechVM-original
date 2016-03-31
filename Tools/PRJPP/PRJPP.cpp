///////////////////////////////////////////////////////////////////////////////
// PRJPP.cpp: MW2.PRJ patch processor
// Copyright Björn Ganster 2009
///////////////////////////////////////////////////////////////////////////////

#include "BGString.h"
#include "MechWarriorIIPRJ.h"
#include "Mesh.h"

///////////////////////////////////////////////////////////////////////////////
// PRJPP extract <MW2.PRJ> <dir> <file> <output>
// extract <file> from <dir> and produce <output> in <MW2.PRJ>

bool extract(TCHAR* argv[])
{
   MechWarriorIIPRJ mw2prj;
   if (!mw2prj.open(argv[2])) {
      printf("Failed to open %s\n", argv[2]);
      return false;
   }

   if (!mw2prj.enterSubDir(argv[3])) {
      printf("Failed to enter directory %s\n", argv[3]);
      return false;
   }

   MemoryBlock* mb = mw2prj.getMemoryBlock(argv[4]);
   if (mb == NULL) {
      printf("Unable to open file %s\n", argv[4]);
      return false;
   }
   if (mb->getSize() == 0) {
      printf("Warning: %s has size 0\n", argv[4]);
   }

   bool result;
   if (mb->saveToFile(argv[5])) {
      printf("%s: %i bytes extracted successfully\n", argv[4], mb->getSize());
      result = true;
   } else {
      printf("Extract: Saving to %s failed\n", argv[5]);
      result = false;
   }

   delete mb;
   return result;
}

///////////////////////////////////////////////////////////////////////////////
// PRJPP diff <file1> <file2> <diff>
// compare <file1>, <file2> and produce <diff>

bool diff(TCHAR* argv[])
{
   // Open <file1>, <file2>
   MemoryBlock mb1, mb2, diff;
   if (!mb1.loadFromFile(argv[2])) {
      printf("Cannot open %s\n", argv[2]);
      return false;
   }
   if (!mb2.loadFromFile(argv[3])) {
      printf("Cannot open %s\n", argv[3]);
      return false;
   }

   // Count diffs
   size_t minSize = mb1.getSize();
   if (minSize > mb2.getSize())
      minSize = mb2.getSize();
   size_t diffs = 0;
   for (size_t i = 0; i < minSize; i++) {
      if (mb1.getByte(i) != mb2.getByte(i))
         ++diffs;
   }

   // Create diff file
   size_t diffSize = sizeof(DiffHeader) + 6*diffs;
   if (mb2.getSize() > mb1.getSize())
      diffSize += mb2.getSize() - mb1.getSize();
   diff.resize(diffSize);

   DiffHeader* header = (DiffHeader*) diff.getPtr(0);
   for (size_t i = 0; i < sizeof(header->signature); ++i)
      header->signature[i] = 0;
   strcpy_s (header->signature, "PRJPP diff");
   header->size1 = (DWORD) mb1.getSize();
   header->size2 = (DWORD) mb2.getSize();
   header->diffs = (DWORD) diffs;
   header->reserved = 0;

   // Create list of diffs
   size_t offset = sizeof(DiffHeader);
   for (DWORD i = 0; i < minSize; i++) {
      if (mb1.getByte(i) != mb2.getByte(i)) {
         diff.setDWord(offset, i);
         offset += 4;
         diff.setByte(offset++, mb1.getByte(i));
         diff.setByte(offset++, mb2.getByte(i));
      }
   }

   // If necessary, add new bytes from end of <file2>
   if (mb2.getSize() > mb1.getSize()) {
      diff.copy(offset, mb2.getPtr(minSize), mb2.getSize()-minSize);
   }

   if (diff.saveToFile(argv[4])) {
      printf("Diff operation successful, %i differences found\n", diffs);
      return true;
   } else {
      printf("Diff: Failed to write %s\n", argv[4]);
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////
// PRJPP replace <MW2.PRJ> <dir> <oldFile> <newFile>
// Replace <oldFile> in <dir> with <newFile>

bool replace(TCHAR* argv[])
{
   MechWarriorIIPRJ mw2prj;
   if (!mw2prj.open(argv[2], false)) {
      printf("Failed to open %s\n", argv[2]);
      return false;
   }

   if (!mw2prj.enterSubDir(argv[3])) {
      printf("Failed to enter directory %s\n", argv[3]);
      return false;
   }

   bool result = false;
   size_t fno = mw2prj.getEntryNumberByName(argv[4]);
   if (fno >= mw2prj.getEntryCount()) {
      printf ("Could not locate %s\n", argv[4]);
      result = false;
   }

   if (mw2prj.replaceFile(fno, argv[5])) {
      printf("Operation successful\n");
      return true;
   } else {
      printf ("Replacement operation failed\n");
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////

void add(TCHAR* argv[])
{
}

///////////////////////////////////////////////////////////////////////////////

#pragma pack (2)
struct BitMapHeader {
   char magic[2];
   DWORD fileSize;
   WORD reserved1;
   WORD reserved2;
   DWORD dataOffset;
};

struct BitMapInfo {
   DWORD HeaderSize;
   DWORD width;
   DWORD height;
   WORD planes;
   WORD bpp;
   DWORD compression;
   DWORD dataSize;
   DWORD widthPPM; // horizontal resolution in pixels per meter
   DWORD heightPPM; // vertical resolution in pixels per meter
   DWORD sizeOfPalette;
   DWORD numberOfImportantColors;
};

struct TEXHeader {
   char magic[4];
   WORD flags; // 0, 3, or 7?
   WORD width;
   WORD height;
   WORD paletteSize;
};

bool bmp2tex (const TCHAR* bmpFN, const TCHAR* texFN)
{
   MemoryBlock* bmp = new MemoryBlock();
   if (!bmp->loadFromFile(bmpFN)) {
      printf ("Cannot open %s\n", bmpFN);
      return false;
   }
   
   // Check bitmap header
   BitMapHeader* bmpH = (BitMapHeader*) bmp->getPtr(0);
   if (bmpH->magic[0] != 'B'
   ||  bmpH->magic[1] != 'M')
   {
      printf ("%s has invalid magic - not a BMP file?\n", bmpFN);
      return false;
   }

   if (bmpH->fileSize != bmp->getSize()) {
      printf ("%s: header reports size of %i, but file has %i bytes\n", 
              bmpFN, bmpH->fileSize, bmp->getSize()); 
      return false;
   }

   // Check bitmap data
   BitMapInfo* bmi = (BitMapInfo*) bmp->getPtr(sizeof(BitMapHeader));
   if (bmi->HeaderSize != 40) {
      printf ("%s has an unexpected bitmap info size - %i vs 40 expected bytes\n",
              bmpFN, bmi->HeaderSize); 
      return false;
   }
   if (bmi->bpp != 8 || bmi->compression != 0) {
      printf ("%s is not an 8-bit indexed image\n", bmpFN);
      return false;
   }

   // Check size of image and palette
   if (bmi->sizeOfPalette == 0) {
      bmi->sizeOfPalette = 256;
   } else if (bmi->sizeOfPalette > 256) {
      printf ("%s has too many colors in the palette\n", bmpFN);
      return false;
   }
   if (bmi->width > 4096 || bmi->height > 4096) {
      printf ("%s: image too large (%ix%i)\n", bmpFN, bmi->width, bmi->height);
      return false;
   }

   // Start creating the new file
   int newFileSize = sizeof (TEXHeader) + 4*bmi->sizeOfPalette + 
                     bmi->width * bmi->height;
   MemoryBlock* tex = new MemoryBlock (newFileSize);
   TEXHeader* texH = (TEXHeader*) tex->getPtr (0);
   strcpy_s (texH->magic, "1.22");
   texH->width = (WORD) bmi->width;
   texH->height = (WORD) bmi->height;
   texH->paletteSize = (WORD) bmi->sizeOfPalette;
   texH->flags = 0;

   // Copy palette
   unsigned char r, g, b;
   int bmpOffset = sizeof (BitMapHeader) + sizeof (BitMapInfo);
   int texOffset = sizeof (TEXHeader);
   for (DWORD i = 0; i < bmi->sizeOfPalette; ++i) {
      g = bmp->getByte(bmpOffset++);
      b = bmp->getByte(bmpOffset++);
      r = bmp->getByte(bmpOffset++);

      tex->setByte(texOffset++, r);
      tex->setByte(texOffset++, g);
      tex->setByte(texOffset++, b);
      tex->setByte(texOffset++, 0);
   }

   // Copy texels
   bmpOffset = bmpH->dataOffset;
   for (DWORD y = 0; y < bmi->height; ++y) {
      for (DWORD x = 0; x < bmi->width; ++x) {
         b = bmp->getByte(bmpOffset++);
         tex->setByte(texOffset+(bmi->width-y-1)*bmi->width+x, b);
      }
      bmpOffset += (bmpOffset+3) % 4;
   }

   // Write result file
   if (tex->saveToFile(texFN)) {
      printf ("%s created successfully from %s\n", texFN, bmpFN);
      return true;
   } else {
      printf ("Error writing %s\n", texFN);
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////

int wtb_wrap(int checksum)
{
	int foo = (checksum >= 0) ? 0 : -1;
	return ((((checksum ^ foo) - foo) & 0x000fffff) ^ foo) - foo;
}

/*bool obj2wtb (const char* ifn, const char* ofn, const char* textureNumStr,
              const char* textureWidthStr, textureHeightStr)
{
   // Compute size of resulting WTB
   Mesh mesh;
   mesh.loadFromObj (ifn);
   int osize = 32 + 16 * mesh.getPointCount();
   int textureNum = atoi (textureNumStr);
   int textureWidth = atoi (textureWidthStr);
   int textureHeight = atoi (textureHeightStr);

   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      if (poly->getPointCount() <= 4)
         osize += 12;
      else
         osize += 18;
   }

   MemoryBlock mb;
   mb.resize(osize);
   mb.fill(0, osize, 0);

   // Set header data
   mb.setDWord(0, 1329747031); // "WTBO"
   mb.setWord(0x18, mesh.getPointCount());
   mb.setWord(0x1a, mesh.getPolygonCount());

   // Fill out vertex data
   size_t offset = 0x20;
   for (size_t i = 0; i < mesh.getPointCount(); ++i) {
      MeshPoint3D p;
      mesh.getPoint(i, p);
      mb.setDWord(offset, p.x);
      mb.setDWord(offset+4, p.y);
      mb.setDWord(offset+8, p.z);
      //mb.setWord(offset+12, textureWidth*u);
      //mb.setWord(offset+14, textureHeight*v);
      offset += 16;
   }

   // Store polygons
   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      mb.setWord(offset+2, poly->getPointCount());
      for (size_t j = 0; j < poly->getPointCount(); ++j) {
         mb.setWord(offset+4+2*j, poly->getPoint(j));
      }
      if (poly->getPointCount() <= 4)
         offset += 12;
      else
         offset += 18;
   }

   // Write inverted file name to header
   size_t pos = 0;
   size_t len = strlen(ofn);
   if (len > 0)
      bgstrrscan(ofn, OSPathSep, len-1, 0, pos);
   for (size_t i = 0; i < len-pos; ++i) {
      mb.setByte(i+8, -ifn[i+pos]);
   }

   // Compute file checksum - contributed by quota4stupid
	// Add vertex data to checksum
   offset = 32;
   int checksum = 0;
	for (int i = 0; i < mesh.getPointCount(); i++)
	{
		checksum += mb.getInt32(offset) + mb.getInt32(offset+4) + mb.getInt32(offset+8);
		checksum += mb.getInt16(offset+12) + mb.getInt16(offset+14);
      offset += 16;
		checksum = wtb_wrap(checksum);
	}

	// Add polygon data to checksum
	for (int i = 0; i < mesh.getPolygonCount(); i++)
	{
		checksum += mb.getInt16(offset) & 0xffff;
		int iCount = mb.getInt16(offset+2) & 0xffff;
      offset += 4;
		checksum += iCount;

		if (iCount <= 4)
			iCount = 4;
		else if (iCount <= 7)
			iCount = 7;
		//else
		//	throw new IllegalArgumentException("Polygons cannot have more than 7 vertices!");

      while (iCount-- > 0) {
		   checksum += mb.getInt16(offset) & 0xffff;
         offset += 2;
      }

		checksum = wtb_wrap(checksum);
	}
   mb.setDWord(4, checksum);

   // Store result in a file
   return mb.saveToFile(ofn);
}*/

void calcWTBHeaderChecksum(MemoryBlock& mb)
{
   // Compute file checksum - contributed by quota4stupid
	// Add vertex data to checksum
   size_t verticesInAllPolygons = mb.getWord(0x18);
   size_t polygonCount = mb.getWord(0x1a);

   int checksum = 0;
   size_t offset = 32;
	for (size_t i = 0; i < verticesInAllPolygons; i++)
	{
		checksum += mb.getInt32(offset) + mb.getInt32(offset+4) + mb.getInt32(offset+8);
		checksum += mb.getInt16(offset+12) + mb.getInt16(offset+14);
      offset += 16;
		checksum = wtb_wrap(checksum);
	}

	// Add polygon data to checksum
	for (size_t i = 0; i < polygonCount; i++)
	{
		checksum += mb.getInt16(offset) & 0xffff;
		int iCount = mb.getInt16(offset+2) & 0xffff;
      offset += 4;
		checksum += iCount;

      if (iCount <= 4) {
			iCount = 4;
      } else if (iCount <= 7) {
			iCount = 7;
      }
		//else
		//	throw new IllegalArgumentException("Polygons cannot have more than 7 vertices!");

      while (iCount-- > 0) {
		   checksum += mb.getInt16(offset) & 0xffff;
         offset += 2;
      }

		checksum = wtb_wrap(checksum);
	}
   mb.setDWord(4, checksum);
}

// Exporter with texture coordinate support

// An OBJ file stores world space coordinates separate from texture coordinates,
// whereas a WTB vertex combines world space and texture coordinates. As a
// consequence, we store each vertex separately for each polygon.

bool obj2wtb (const char* ifn, const char* ofn, const char* textureNumStr,
              const char* textureWidthStr, const char* textureHeightStr)
{
   Mesh mesh;
   if (!mesh.loadFromObj (ifn)) {
      printf("Failed to load %s\n", ifn);
   }
   printf ("%s has %i vertices and %i polygons\n",  ifn, mesh.getPointCount(),
           mesh.getPolygonCount());

   // Count vertices in all polygons
   WORD verticesInAllPolygons = 0;
   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      verticesInAllPolygons += (WORD) poly->getPointCount();
   }

   // Compute size of resulting WTB
   size_t osize = 32 + 16 * verticesInAllPolygons;
   int textureNum = atoi (textureNumStr);
   int textureWidth = atoi (textureWidthStr);
   int textureHeight = atoi (textureHeightStr);

   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      if (poly->getPointCount() <= 4)
         osize += 12;
      else
         osize += 18;
   }

   MemoryBlock mb;
   mb.resize(osize);
   mb.fill(0, osize, 0);

   // Set header data
   mb.setDWord(0, 1329747031); // "WTBO"
   //mb.setWord(0x18, mesh.getPointCount());
   mb.setWord(0x18, verticesInAllPolygons);
   mb.setWord(0x1a, (WORD) mesh.getPolygonCount());

   // Fill out vertex data
   WORD offset = 0x20;
   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      for (size_t j = 0; j < poly->getPointCount(); ++j) {
         size_t k = poly->getPoint(j);
         Point3D p;
         mesh.getPoint(k, p);

         double u, v;
         poly->getTexCoords(j, u, v);

         mb.setDWord(offset, (DWORD) p.x);
         mb.setDWord(offset+4, (DWORD) p.y);
         mb.setDWord(offset+8, (DWORD) p.z);
         mb.setWord(offset+12, (WORD) (textureWidth*u));
         mb.setWord(offset+14, (WORD) (textureHeight*v));
         offset += 16;
      }
   }

   // Store polygons
   WORD pnum = 0;
   for (size_t i = 0; i < mesh.getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh.getPolygon(i);
      mb.setWord(offset, textureNum);
      mb.setWord(offset+2, (WORD) poly->getPointCount());
      for (size_t j = 0; j < poly->getPointCount(); ++j) {
         mb.setWord(offset+4+2*j, pnum++);
      }
      if (poly->getPointCount() <= 4)
         offset += 12;
      else
         offset += 18;
   }

   // Write inverted file name to header
   size_t pos = 0;
   size_t len = strlen(ofn);
   if (len > 16) {
      printf("%s: file name too long\n", ofn);
      return false;
   }
   if (len > 0)
      bgstrrscan(ofn, OSPathSep, len-1, 0, pos);
   for (size_t i = 0; i < len-pos; ++i) {
      mb.setByte(i+8, -ifn[i+pos]);
   }

   calcWTBHeaderChecksum(mb);

   // Store result in a file
   if (mb.saveToFile(ofn)) {
      printf ("%s created successfully, %i bytes\n", ofn, mb.getSize());
      return true;
   } else {
      printf ("Error writing %s\n", ofn);
      return false;
   }
}

bool wtbJoin (const char* fn1, const char* fn2, const char* ofn)
{
   MemoryBlock inWTB1, inWTB2, outWTB;
   if (!inWTB1.loadFromFile(fn1)) {
      printf ("Could not load %s\n", fn1);
      return false;
   }
   if (!inWTB2.loadFromFile(fn2)) {
      printf ("Could not load %s\n", fn2);
      return false;
   }
   if (inWTB1.getSize() + inWTB2.getSize() > 64*1024) {
      printf ("Cannot join %s and %s - cannot create WTB's greater than 64kB\n");
      return false;
   }

   WORD inWTB1VertexCount = inWTB1.getWord(0x18);
   WORD inWTB1PolygonCount = inWTB1.getWord(0x1a);
   WORD inWTB1VertexBufferSize = 16 * inWTB1VertexCount;
   WORD inWTB1PolygonBufferSize = ((WORD) inWTB1.getSize()) - 32 - inWTB1VertexBufferSize;

   WORD inWTB2VertexCount = inWTB2.getWord(0x18);
   WORD inWTB2PolygonCount = inWTB2.getWord(0x1a);
   WORD inWTB2VertexBufferSize = 16 * inWTB2VertexCount;
   WORD inWTB2PolygonBufferSize= ((WORD) inWTB2.getSize()) - 32 - inWTB2VertexBufferSize;

   printf ("%s has %i vertices and %i polygons\n",  fn1, inWTB1VertexCount,
           inWTB1PolygonCount);
   printf ("%s has %i vertices and %i polygons\n",  fn2, inWTB2VertexCount,
           inWTB2PolygonCount);

   WORD osize = inWTB1.getSize() + inWTB2.getSize() - 32;
   outWTB.resize(osize);

   // Copy data
   WORD offset = 32;
   outWTB.copy(offset, inWTB1.getPtr(32), inWTB1VertexBufferSize);
   offset += inWTB1VertexBufferSize;
   outWTB.copy(offset, inWTB2.getPtr(32), inWTB2VertexBufferSize);
   offset += inWTB2VertexBufferSize;
   outWTB.copy(offset, inWTB1.getPtr(32+inWTB1VertexBufferSize), inWTB1PolygonBufferSize);
   offset += inWTB1PolygonBufferSize;
   WORD WTB2VertexOffset = offset;
   outWTB.copy(offset, inWTB2.getPtr(32+inWTB2VertexBufferSize), inWTB2PolygonBufferSize);
   offset += inWTB2PolygonBufferSize;

   // Polygon indices into vertex buffer must be adjusted for inWTB2
   offset = WTB2VertexOffset;
   for (WORD i = 0; i < inWTB2PolygonCount; ++i)
   while (offset < outWTB.getSize()) {
      WORD count = outWTB.getWord(offset+2);
      WORD nextOffset = offset+12;
      if (count > 4)
         nextOffset = offset+18;
      offset += 4;
      for (size_t j = 0; j < count; ++j) {
         WORD vertexNum = outWTB.getWord(offset);
         vertexNum += inWTB1VertexCount;
         outWTB.setWord(offset, vertexNum);
         offset += 2;
      }
      offset = nextOffset;
   }

   // Fill in header
   outWTB.setDWord(0, 1329747031); // "WTBO"
   outWTB.setWord(0x18, inWTB1VertexCount+inWTB2VertexCount);
   outWTB.setWord(0x1a, inWTB1PolygonCount+inWTB2PolygonCount);
   calcWTBHeaderChecksum(outWTB);

   // Write inverted file name to header
   size_t pos = 0;
   size_t len = strlen(ofn);
   if (len > 0)
      bgstrrscan(ofn, OSPathSep, len-1, 0, pos);
   for (size_t i = 0; i < len-pos; ++i) {
      outWTB.setByte(i+8, -ofn[i+pos]);
   }

   // Store result in a file
   if (outWTB.saveToFile(ofn)) {
      printf ("%s created successfully, %i bytes\n", ofn, outWTB.getSize());
      return true;
   } else {
      printf ("Error writing %s\n", ofn);
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, TCHAR* argv[])
{
   bool syntaxCorrect = false;

   if (argc > 3) {
      if (strcmp(argv[1], "extract") == 0 && argc == 6) {
         extract(argv);
         syntaxCorrect = true;
      }

      if (strcmp(argv[1], "diff") == 0 && argc == 5) {
         diff(argv);
         syntaxCorrect = true;
      }

      if (strcmp(argv[1], "patch") == 0 && argc == 4) {
         patch(argv[2], argv[3]);
         syntaxCorrect = true;
      }

      if (strcmp(argv[1], "replace") == 0 && argc == 6) {
         replace(argv);
         syntaxCorrect = true;
      }

      if (strcmp(argv[1], "add") == 0 && argc == 5) {
         add(argv);
         syntaxCorrect = true;
      }

      if (strcmp(argv[1], "random") == 0 && argc == 4) {
         int a = atoi (argv[2]);
         int b = atoi (argv[3]);

         srand (GetTickCount());
         int c = rand();
         int result = a + (c % (b-a));

         return result;
      }

      if (strcmp(argv[1], "ask") == 0 && argc == 4) {
         int a = atoi (argv[2]);
         int b = atoi (argv[3]);
         int result;
         char buf[20];
         do {
            printf("Please enter a number (%i-%i)\n", a, b);
            gets_s(buf);
            if (strlen(buf) == 0)
               result = a-1;
            else
               result = atoi(buf);
         } while (result < a || b < result);
         return result;
      }

      if (strcmp(argv[1], "bmp2tex") == 0 && argc == 4) {
         return bmp2tex (argv[2], argv[3]);
      }

      if (strcmp(argv[1], "obj2wtb") == 0 && argc == 7) {
         return obj2wtb (argv[2], argv[3], argv[4], argv[5], argv[6]);
      }

      if (strcmp(argv[1], "wtbjoin") == 0 && argc == 5) {
         return wtbJoin (argv[2], argv[3], argv[4]);
      }
   }

   if (!syntaxCorrect) 
      printf("PRJPP-2009-05-01 written by Björn \"Skyfaller\" Ganster \n"
             "\n"
             "Syntax:\n"
             "PRJPP extract <MW2.PRJ> <dir> <file> <output>\n"
             "extract <file> from <dir> and produce <output> in <MW2.PRJ>\n" 
             "\n"
             "PRJPP diff <file1> <file2> <diff>\n"
             "compare <file1>, <file2> and produce <diff>\n"
             "\n"
             "PRJPP patch <diff> <file>\n"
             "Apply changes from previously produced <diff> to <file>\n"
             "\n"
             "PRJPP replace <MW2.PRJ> <dir> <oldFile> <newFile>\n"
             "Replace <oldFile> in <dir> with <newFile>\n"
             "\n"
             "PRJPP add <MW2.PRJ> <dir> <newFile>\n"
             "Add <newFile> to <dir>, maintaining its 8.3 name\n"
             "\n"
             "PRJPP random <a> <b>\n"
             "Return a random number between <a>, <b-1> as ERRORLEVEL\n"
             "\n"
             "PRJPP ask <a> <b>\n"
             "Ask user to enter a number between <a>, <b-1> and return it as ERRORLEVEL\n"
             "\n"
             "PRJPP bmp2tex <BMP file> <TEX file>\n"
             "Create a TEX file from a BMP file\n"
             "\n"
             "PRJPP obj2wtb <OBJ file> <WTB file> <texture number> <texture width> <texture height>\n"
             "Create a WTB file from a OBJ file\n"
             "\n"
             "PRJPP wtbjoin <WTB file 1> <WTB file 2) <WTB output file>"
             "Create a new WTB from two existing ones\n");

	return 0;
}
