////////////////////////////////////////////////////////////////////////////////
// MechWarrior 3 ZBD file system
// Copyright Bjoern Ganster 2007, 2008
////////////////////////////////////////////////////////////////////////////////

#include "MechWarrior3ZBD.h"
#include "Texture.h"
#include "Mesh.h"
#include "XMLTree.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

MechWarrior3ZBD::MechWarrior3ZBD()
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

MechWarrior3ZBD::~MechWarrior3ZBD()
{
}

////////////////////////////////////////////////////////////////////////////////
// Open file

bool MechWarrior3ZBD::open (const TCHAR* PRJFN, bool readOnly)
{
   bool openedSuccessfully;
   if (readOnly)
      openedSuccessfully = fc.openReadOnly(PRJFN);
   else
      openedSuccessfully = fc.openReadWrite(PRJFN);

   if (!openedSuccessfully) 
      return false;

   BGString PrjStr = PRJFN;
   size_t contentsTableOffset, recordSize, fileOffsetInRecord,
          fileSizeInRecord, fileNameInRecord, fileCount, type = 0;

   switch (fc.getFileSize()) {
      case 92824738: // English soundsH.zbd
         contentsTableOffset = 0x586230E;
         type = 1;
         break;
      case 96578674: // German soundsH.zbd
         contentsTableOffset = 0x5BF688E;
         type = 1;
         break;
      case 2406873: // MechLib.ZBD, eng=ger
         contentsTableOffset = 0x249849;
         type = 1;
         break;
      case 6007773: // Motion.ZBD, eng=ger
         contentsTableOffset = 0x5B16AD;
         type = 1;
         break;
      case 41914526: // RImage.ZBD, ger
      case 42528982: // RImage.ZBD, eng
      case 32910360: // RMechTex.ZBD, eng=ger
      case 9195816: // RMechTexS.ZBD, eng=ger
      case 3041548: // c1\Texture.ZBD
      case 5306760: // C1\RTexture.ZBD
         type = 2;
         break;
   }

   if (type > 0) {
      if (type == 1) {
         recordSize = 148;
         fileOffsetInRecord = 0;
         fileSizeInRecord = 4;
         fileNameInRecord = 8;
         fileCount = (fc.getFileSize()-contentsTableOffset) / recordSize;
      } else if (type == 2) {
         contentsTableOffset = 24;
         recordSize = 40;
         fileOffsetInRecord = 32;
         fileSizeInRecord = 36;
         fileNameInRecord = 0;
         //fileCount = (fc.getFileSize()-contentsTableOffset) / recordSize;
         fileCount = fc.readDWord(0x0c);
      }

      clearFileEntries();
      char* buf = new char[recordSize];
      for (size_t i = 0; i < fileCount; i++) {
         size_t offset = contentsTableOffset+recordSize*i;
         size_t FileOffset, FileSize;
         fc.read (offset+fileOffsetInRecord, (char*) (&FileOffset), 4);
         fc.read (offset+fileSizeInRecord, (char*) (&FileSize), 4);
         fc.read (offset+fileNameInRecord, buf, recordSize);
         addFile (buf, FileSize, FileOffset);
      }
      calculateFileSizesFromOffsetDifferences();
      return true;
   } else {
      printf (PRJFN);
      printf (": unknown format\n");
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Decode MW3 555 image

Texture* Decode_MW3_555 (unsigned char* mem, size_t size)
{
   if (size < 16)
      return NULL;

   size_t w = *((WORD*)(&mem[4]));
   size_t h = *((WORD*)(&mem[6]));

   if (size < 2*w*h+16)
      return NULL;

   Texture* t = new Texture (w, h);

   for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
         size_t index = 2*(y*w+x)+16;
         unsigned char b1 = mem[index++];
         unsigned char b2 = mem[index++];

         // r5g6b5
         unsigned char b = b1 & 31;
         unsigned char g = (b1 >> 5) + ((b2 & 7) << 3);
         unsigned char r = (b2 >> 3);

         t->setRGB (x, h-y-1, r << 3, g << 2, b <<3);
      }
   }

   return t;
}

////////////////////////////////////////////////////////////////////////////////
// Import MW3 mechs from ZBD files

MW3MechImporter::MW3MechImporter (const char* mw3Dir, const char* outputDir, 
                     const char* importXML)
{
   XMLTree importXMLTree;
   importXMLTree.loadFromFile(importXML);
   XMLTreeConstIterator* xmlIter = importXMLTree.getConstIterator();
   const char* sourcePath;

   // Open mech library file mechlib.zbd
   MechWarrior3ZBD mechLib;
   BGFilePath<TCHAR> mechLibZBDFN = mw3Dir;
   mechLibZBDFN += "zbd";
   mechLibZBDFN += "mechlib.zbd";

   // Open mech texture file 
   BGFilePath<TCHAR> textureLibFN = mw3Dir;
   textureLibFN += "zbd";
   textureLibFN += "rmechtex.zbd";
   if (fileExists (textureLibFN.getChars())) {
      m_textureLib = new MechWarrior3ZBD ();
      if (!m_textureLib->open(textureLibFN.getChars())) {
         delete m_textureLib;
         m_textureLib = NULL;
      }  
   } else 
      m_textureLib = NULL;

   // Open texture list
   BGFilePath<TCHAR> textListPath = getExecDir();
   textListPath += "textureList.txt";
   MemoryBlock mb;
   mb.loadFromFile(textListPath.getChars());
   BGString line;
   for (size_t offset = 0; offset < mb.getSize(); offset++) {
      char c = mb.getByte(offset);
      if (c == 0x0A) {
         textureFNs.push_back (line.getChars());
         line.clear();
      } else {
         line += c;
      }
   }      

   if (xmlIter->goToChild(0) && mechLib.open (mechLibZBDFN.getChars())) {
      for (size_t i = 0; i < xmlIter->getChildCount(); i++) {
         xmlIter->goToChild(i);

         // Import geometry
         sourcePath = xmlIter->getAttributeValueByName ("source");
         m_targetPath = xmlIter->getAttributeValueByName ("target");
         printf ("Importing %s\n", m_targetPath.getChars());

         if (sourcePath != NULL && m_targetPath.getLength() != 0) {
            size_t fileNum = mechLib.getEntryNumberByName(sourcePath);
            MemoryBlock flt;
            flt.resize(mechLib.getFileSize(fileNum));
            mechLib.getFileChunk(fileNum, 0, flt.getPtr(), flt.getSize());
            m_mechOutputDir = getExecDir();
            m_mechOutputDir += outputDir;
            m_mechOutputDir += m_targetPath.getChars();
            bgmkdir (m_mechOutputDir.getChars());
            exportFLT (&flt);

            // Generate description file
            BGString mvmkFN = m_mechOutputDir;
            mvmkFN.appendNoDups(OSPathSep);
            mvmkFN += m_targetPath.getChars();
            mvmkFN += ".mvmk";
            XMLTree mvmkTree (xmlIter->getNodeName());
            XMLTreeIterator* mvmkIter = mvmkTree.getIterator();
            mvmkIter->copyFromChilds(*xmlIter);
            mvmkTree.saveToFile(mvmkFN.getChars());

            // Copy fake mech
            BGString input = getExecDir();
            input.appendNoDups(OSPathSep);
            input += "fake.mek";
            BGString output = m_mechOutputDir;
            output.appendNoDups(OSPathSep);
            output += m_targetPath.getChars();
            output += "-fake.MEK";
            MemoryBlock mb;
            if (mb.loadFromFile (input.getChars()))
               mb.saveToFile(output.getChars());

            delete mvmkIter;
         }

         xmlIter->goToParent();
      }
   }

   delete xmlIter;
}

MW3MechImporter::~MW3MechImporter ()
{
   if (m_textureLib != NULL)
      delete m_textureLib;
   //if (m_tc != NULL) // deleted in non-rec. MW3MechImporter::exportFLT
   //   delete m_tc;
}

////////////////////////////////////////////////////////////////////////////////
// Open mech geometry

// Extract nodes recursively
void MW3MechImporter::extractNodes (MemoryBlock* flt, size_t& offset, Matrix m)
{
   // MechWarrior 3 - textures missing
   const int Polygon_Information_Size = 36; 
   const int Node_Header_Size = 444;
   const int AdditionalPolygonDataSize = 0;
   const int UnknownPolygonDataSize = 12;
   const int EvenMorePolygonData = 0;
   const bool PiratesMoon = false;
   const bool invertPolygons = true;

   // Pirate's Moon - not working yet
   //const int Polygon_Information_Size = 40;
   //const int Node_Header_Size = 452;
   //const int AdditionalPolygonDataSize = 4; //???
   //const int UnknownPolygonDataSize = 20; // Could be 12, if UV flag is interpreted correctly
                                          // or if UVs are forced
   //const int EvenMorePolygonData = 60;
   //const bool PiratesMoon = true;

   if (offset+Node_Header_Size >= flt->getSize())
      return;

   size_t childNodes = flt->getByte(offset+92);
   char flags = flt->getByte(offset+37);
   bool hasPolygons = (flags == 3 || flags == 7);

   if (hasPolygons) {
      char partName[36];
      bgstrcpymax (partName, (const char*) flt->getPtr(offset), 35);
      double tx = flt->getFloat(offset+292);
      double ty = flt->getFloat(offset+296);
      double tz = flt->getFloat(offset+300);
      double rx = flt->getFloat(offset+232);
      double ry = flt->getFloat(offset+236);
      double rz = flt->getFloat(offset+240);
      size_t polygonCount = flt->getDWord(offset+352+16);
      size_t pointCount1 = flt->getDWord(offset+352+20);
      size_t pointCount2 = flt->getDWord(offset+352+24); // Normals?
      Point3D translation = Point3D(tx, ty, tz);
      m = m * TranslationMatrix (translation.x, translation.y, translation.z);
      m = m * RotationMatrix (rx, -1, 0, 0);
      m = m * RotationMatrix (ry, 0, -1, 0);
      m = m * RotationMatrix (rz, 0, 0, -1);
      offset += Node_Header_Size;
      Mesh* mesh = new Mesh();
      printf ("Loading limb %s with %i vertices and %i polygons\n", partName, 
              pointCount1, polygonCount);

      // Extract points
      size_t i = 0;
      while (offset + 12 < flt->getSize() && i < pointCount1) {
         double x = flt->getFloat (offset);
         double y = flt->getFloat (offset+4);
         double z = flt->getFloat (offset+8);
         offset += 12;
         Point3D p (x, y, z);
         p = m.Transform(p);
         mesh->addPoint(p);
         i++;
      }

      // Skip point set 2
      offset += 12*pointCount2;

      // Extract polygons
      size_t polyData = offset + Polygon_Information_Size * polygonCount;
      for (size_t i = 0; i < polygonCount && offset + 36 < flt->getSize(); i++) {
         size_t vertexCount = flt->getByte (offset);
         size_t TextureNum = flt->getDWord(offset+28)-2;
         bool hasPointIndex2 = (flt->getByte(offset+14) > 0); // 15?
         bool hasUVs = (flt->getByte(offset+18) > 0); // 19?
         offset += Polygon_Information_Size;
         MeshPolygon* poly = mesh->addPolygon(vertexCount);
         ManagedTexture* manText = NULL;

         // Select texture or load it
         if (m_textureLib != NULL && hasUVs && TextureNum < textureFNs.size()) {
            const char* textureFN = textureFNs[TextureNum].getChars();
            TextureNum = m_textureLib->getEntryNumberByName(textureFN);
            manText = m_tc->findTextureByID(TextureNum);
            if (manText == NULL 
            && TextureNum < m_textureLib->getEntryCount()) 
            {
               MemoryBlock mb;
               mb.resize(m_textureLib->getFileSize(TextureNum));
               //size_t entryNum = m_textureLib->getEntryNumberByName(str.getChars());
               size_t entryNum = TextureNum;
               char FN[200];
               m_textureLib->getEntryName(entryNum, FN, 199);
               if (m_textureLib->getFileChunk(entryNum, 0, mb.getPtr(0), mb.getSize())) {
                  Texture* newText = Decode_MW3_555 (mb.getPtr(0), mb.getSize());
                  if (newText != NULL) {
                     manText = m_tc->add(newText);
                     manText->setID (TextureNum);
                  }
               }
            }
         }

         polyData += AdditionalPolygonDataSize;
         //for (size_t j = 0; j < vertexCount; j++) 
         //   poly->setPoint(j, 0);
         for (size_t j = 0; j < vertexCount && polyData + 4 < flt->getSize(); j++) {
            DWORD index = flt->getDWord (polyData);
            polyData += 4;
            if (invertPolygons)
               poly->setPoint(vertexCount-j-1, index);
            else
               poly->setPoint(j, index);
         }

         // Skip data
         if (hasPointIndex2)
            polyData += 4*vertexCount; // PointIndex2

         // Read texture coordinates
         if (manText != NULL) { // (hasUVs is tested above)
            for (size_t j = 0; j < vertexCount && polyData + 8 < flt->getSize(); j++) {
               double u = flt->getFloat(polyData);
               polyData += 4;
               double v = flt->getFloat(polyData);
               polyData += 4;
               u = putInRange (0.0, u, 1.0);
               v = putInRange (0.0, v, 1.0);
               if (invertPolygons)
                  poly->setTexCoords(vertexCount-j-1, u, 1-v);
               else
                  poly->setTexCoords(j, u, 1-v);
               poly->m_manText = manText;
            }
         } else {
            for (size_t j = 0; j < vertexCount; j++) {
               poly->setTexCoords(j, 0, 0);
            }
         }

         // Skip unknown data
         polyData += UnknownPolygonDataSize*vertexCount;
      }
      offset = polyData; //+EvenMorePolygonData;
      if (PiratesMoon) {
         while (flt->getDWord(offset) < 1000000)
            offset += 4;
      }

      // Store results until final UVs are known
      if (pointCount1 > 0 && polygonCount > 0) {
         BGString path = m_mechOutputDir;
         path.appendNoDups(OSPathSep);
         path += partName;
         path += ".OBJ";
         OBJFNs.push_back(path);
         meshes.push_back(mesh);
      }
   } else
      offset += 352;

   // Extract child nodes
   while (childNodes > 0 && offset < flt->getSize()) {
      extractNodes (flt, offset, m);
      childNodes--;
   }
}

void MW3MechImporter::exportFLT (MemoryBlock* flt) 
{
   if (m_textureLib != NULL)
      m_tc = new TextureCompiler;

   size_t offset = 0;
   double rx = 0, ry = 0, rz = 0;
   Matrix m = RotationMatrix(M_PI, 0, 1, 0);
   extractNodes (flt, offset, m);

   // Prepare mech texture
   if (m_tc != NULL) {
      Texture* finalMechTexture = m_tc->buildAtlas();
      size_t w = finalMechTexture->getWidth();
      size_t h = finalMechTexture->getHeight();
      BGString textFN = m_mechOutputDir;
      textFN.appendNoDups(OSPathSep);
      textFN += m_targetPath.getChars();
      textFN += ".BMP";
      finalMechTexture->saveToFile(textFN.getChars(), 0, 0, w, h);
      delete finalMechTexture;
      delete m_tc;

      // Adjust UVs
      for (size_t i = 0; i < meshes.size(); i++) {
         Mesh* mesh = meshes[i];
         for (size_t j = 0; j < mesh->getPolygonCount(); j++) {
            MeshPolygon* poly = mesh->getPolygon(j);
            ManagedTexture* manText = poly->m_manText;
            if (manText != NULL) 
            for (size_t k = 0; k < poly->getPointCount(); k++) {
               double u, v;
               poly->getTexCoords (k, u, v);
               u = manText->transformU(u);
               v = manText->transformV(v);
               poly->setTexCoords (k, u, v);
            }
         }
      }
   }

   // Save .OBJs
   for (size_t i = 0; i < meshes.size(); i++) {
      meshes[i]->saveToObj(OBJFNs[i].getChars(), true);
      delete meshes[i];
   }

   meshes.clear();
   OBJFNs.clear();
}
