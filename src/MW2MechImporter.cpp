///////////////////////////////////////////////////////////////////////////////
// MW2MechImporter.cpp
// Copyright Bjoern Ganster 2009-2011
///////////////////////////////////////////////////////////////////////////////

#include "MW2MechImporter.h"
#include "MWBase.h"
#include "MechVM.h"
#include "XMLTree.h"
#include "Config.h"
#include "Mesh.h"
#include "Vehicles.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor

MW2MechImporter::MW2MechImporter()
{
   prj = NULL;
   palette = NULL;
   tc = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor

MW2MechImporter::~MW2MechImporter()
{
   if (prj != NULL)
      delete prj;

   for (size_t i = 0; i < textures.getSize(); i++) {
      MW2TextureTableEntry* entry = textures[i];
      if (entry->texture != NULL)
         delete entry->texture;
      delete entry;
   }

   if (palette != NULL)
      delete palette;
}

///////////////////////////////////////////////////////////////////////////////
// Open MW2.PRJ

bool MW2MechImporter::setPRJ (const char* PRJFN)
{
   prj = new MechWarriorIIPRJ();
   if (!prj->open(PRJFN)) {
      printf ("Could not open file for import: %s\n", PRJFN);
      return false;
   }
   if (!prj->enterSubDir("BWD")) {
      printf ("Strange PRJ file without BWD subdirectory\n");
      return false;
   } else
      return true;
}

///////////////////////////////////////////////////////////////////////////////
// Build texture table

void MW2MechImporter::buildTextureTable (const char* BWDFN)
{
   size_t fileNum = prj->getEntryNumberByName(BWDFN);
   if (fileNum < prj->getEntryCount())
      buildTextureTable (fileNum);
   else
      printf ("%s not found - cannot build texture table\n", BWDFN);
}

void MW2MechImporter::buildTextureTable (DWORD fileNum)
{
   // Enter BWD directory
   if (!prj->enterSubDir ("BWD"))
      return;

   // Get file number for BWD file
   if (fileNum >= prj->getEntryCount()) 
      return;

   // Get memory block for BWD file
   size_t bwdSize = prj->getFileSize(fileNum);
   MemoryBlock* mb = new MemoryBlock (bwdSize);
   prj->getFileChunk(fileNum, 0, mb->getPtr(), bwdSize);
   char BWDFN[20];
   prj->getEntryName(fileNum, BWDFN, 19);
   printf ("Analyzing %s\n", BWDFN);

   // Scan BWD's
   size_t offset = 0x34, bmidCount = 0, inclCount = 0, palgCount = 0;
   WORD BMPJ = -1;

   while (offset < mb->getSize()) {
      const char* tagName = (const char*) mb->getPtr (offset);
      if (memcmp (tagName, "INCL", 4) == 0) {
         BGString newBWDName = (const char*) mb->getPtr (offset+10);
         //newBWDName += ".BWD";
         //buildTable (newBWDName);
         DWORD inclFileNum = mb->getWord(offset+8)-MW2SkippedFiles;
         buildTextureTable (inclFileNum);
         inclCount++;
      } else if (memcmp (tagName, "BMPJ", 4) == 0) {
         BMPJ = mb->getWord(offset+8);
      } else if (memcmp (tagName, "BMID", 4) == 0) {
         WORD BMID = mb->getWord(offset+8);
         MW2TextureTableEntry* entry = new MW2TextureTableEntry();
         textures.add(entry);
         entry->BMID = BMID;
         entry->BMPJ = BMPJ-MW2SkippedFiles;
         entry->texture = NULL;
         entry->manText = NULL;
         bmidCount++;
      } else if (memcmp (tagName, "PALG", 4) == 0) {
         //paletteFileNum = mb->getWord (offset+8);
         palgCount++;
         printf (BWDFN);
      }

      DWORD tagSize = mb->getDWord(offset+4);
      offset += tagSize;
   }

   printf("%s contains %i BMID tags, %i INCL tags, %i PALG tags\n", 
          BWDFN, bmidCount, inclCount, palgCount);
   delete mb;
}

///////////////////////////////////////////////////////////////////////////////
// Get file ID

int MW2MechImporter::getTextureNumFromID (int textureID)
{
   for (size_t i = 0; i < textures.getSize(); i++) {
      MW2TextureTableEntry* entry = textures[i];
      if (entry->BMID = textureID)
         return entry->BMPJ;
   }

   return -1;
}

///////////////////////////////////////////////////////////////////////////////
// Retrieve texture, loading it when necessary

ManagedTexture* MW2MechImporter::getTexture (int textureID)
{
   for (size_t i = 0; i < textures.getSize(); i++) {
      MW2TextureTableEntry* entry = textures[i];
      if (entry->BMID == textureID) {
         //printf ("BMID %i -> BMPJ %i\n", entry->BMID, entry->BMPJ);
         if (entry->texture == NULL) {
            char fn [20];
            prj->enterSubDir("CEL");
            if (prj->getEntryName(entry->BMPJ, fn, 18)) {
               printf("Loading %s\n", fn);
               MemoryBlock* mb = prj->getMemoryBlock(entry->BMPJ);
               int ft = getFileTypeFromExtension (fn);
               switch (ft) {
                  case FT_TEX:
                     entry->texture = prj->loadTEX(mb->getPtr(), mb->getSize());
                     break;
                  case FT_XEL:
                     entry->texture = prj->loadTexture(mb->getPtr(), mb->getSize(), palette);
                     break;
                  case FT_MW2_555:
                     entry->texture = Decode_MW2_555(mb->getPtr(), mb->getSize());
                     break;
                  default:
                     printf ("Unknown format: %s\n", fn);
               }
               if (entry->texture != NULL) {
                  entry->manText = tc->add(entry->texture);
                  entry->used++;
               }
               delete mb;
               return entry->manText;
            } else
               printf ("Cannot open texture with BMPJ=%i, BMID=%i\n", 
                       entry->BMPJ, entry->BMID);
         } else {
            if (entry->manText == NULL)
               entry->manText = tc->add(entry->texture);
            entry->used++;
            return entry->manText;
         }
      }
   }

   printf ("Found no match for BMID=%i\n", textureID);
   return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Set palette

void MW2MechImporter::setPalette (MemoryBlock* mb, bool deepCopy)
{
   if (mb == NULL)
      return;
   if (mb->getSize() != 768)
      return;

   // Create deep or shallow palette copy - destroy it in destructor
   if (deepCopy) {
      palette = new MemoryBlock(mb->getSize());
      palette->copy(0, mb->getPtr(), mb->getSize());
   } else
      palette = mb;

   size_t i = 0;
   while (i < palette->getSize()) {
      MW2TextureTableEntry * entry = new MW2TextureTableEntry;
      entry->BMID = i;
      entry->BMPJ = i;
      entry->texture = new Texture (4, 4);
      unsigned char r = palette->getByte(i++);
      unsigned char g = palette->getByte(i++);
      unsigned char b = palette->getByte(i++);
      for (int x = 0; x < 4; x++)
         for (int y = 0; y < 4; y++)
            entry->texture->setRGB(x, y, r, g, b);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Copy MW2 mech config file (*.MEK)

void MW2MechImporter::addMW2Stats (XMLTreeConstIterator& MW2MechTreeIter, 
   const BGString& outputPath)
{
   const char* MechName = MW2MechTreeIter.getAttributeValueByName ("name");
   const char* MechFN =  MW2MechTreeIter.getAttributeValueByName ("mekfile");

   if (MechName == NULL || MechFN == NULL)
      return;

   prj->leaveDir();
   size_t mekDir = prj->getEntryNumberByName("MEK");
   if (!prj->enterSubDir(mekDir)) {
      printf ("MEK dir not found!\n");
      return;
   }
   size_t mekNum = prj->getEntryNumberByName(MechFN);

   if (mekNum >= prj->getEntryCount()) {
      printf ("Cannot import %s of %s\n", MechFN, MechName);
      return;
   }

   size_t mekSize = prj->getFileSize(mekNum);
   MemoryBlock* mekF = new MemoryBlock (mekSize);
   if (prj->getFileChunk(mekNum, 0, mekF->getPtr(0), mekSize)) {
      BGString FN = outputPath;
      FN += OSPathSep;
      FN += MechName;
      FN += ".mek";
      mekF->saveToFile(FN.getChars());
   } else {
      printf ("Importing %s from %s failed\n", MechName, MechFN);
   }
   delete mekF;
   prj->leaveDir();
}

///////////////////////////////////////////////////////////////////////////////
// Load mech limb from MemoryBlock

Mesh* MW2MechImporter::loadLimb (const MemoryBlock* block)
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

         if (tc == NULL)
            tc = new TextureCompiler();
      }

      if (bc <= block->getSize() && pointCount > 0 && polygonCount > 0) {
         Mesh* mesh = new Mesh();

         // Read points
         size_t offset = pointBaseOffset;
         for (size_t i = 0; i < pointCount; i++) {
            int x = block->getDWord (offset);
            int y = block->getDWord (offset+4);
            int z = block->getDWord (offset+8);
            size_t newp = mesh->addPoint(x, y, z);
            offset += pointRecordSize;
         }

         // Read polygons
         while (offset + shortPolyRecordSize <= block->getSize())
         {
            size_t count = block->getWord (offset+pointCountInPolygon);
            ManagedTexture* manText = NULL;

            if (tc != NULL) {
               //int textureNum = block->getWord(offset);
               int textureNum = block->getByte(offset)+256;
               manText = getTexture (textureNum);
            }

            if (offset + 2*count+firstPointInPolygon <= block->getSize()) {
               MeshPolygon* poly = mesh->addPolygon(count);
               poly->m_manText = manText;

               for (size_t i = 0; i < count; i++) {
                  size_t polyIndex = i;
                  size_t pointNum = block->getWord (offset+firstPointInPolygon+i*2);
                  if (pointNum >= mesh->getPointCount()) 
                     pointNum = 0;
                  if (flipPolygons)
                     polyIndex = count-i-1;
                  poly->setPoint(polyIndex, pointNum);

                  // Retrieve uv coordinates
                  if (tc != NULL && manText != NULL) {
                     size_t offset = pointBaseOffset + pointNum * pointRecordSize;
                     int u = block->getWord (offset+12);
                     int v = block->getWord (offset+14);
                     manText->checkUVs(u,v);
                     double ud = ((double) u) / ((double) manText->texture->getWidth());
                     double vd = ((double) v) / ((double) manText->texture->getHeight());
                     poly->setTexCoords(polyIndex, ud, vd);
                  }
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

   prj->enterSubDir ("POLY");

   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Copy data from mw2-mechs.xml into a new .mvmk file

void MW2MechImporter::createMVMKFile (
   XMLTreeConstIterator& MW2MechTreeIter, 
   const char* outputPath, const char* outputFN)
{
   XMLTree MechVMMechTree;
   XMLTreeIterator* MechVMMechTreeIter = MechVMMechTree.getIterator();
   const char* vehicleType = MW2MechTreeIter.getNodeName();
   const char* mechName = MW2MechTreeIter.getAttributeValueByName("name");
   size_t MechVMMechTreeIterNum = MechVMMechTreeIter->addChild(vehicleType);
   MechVMMechTreeIter->goToChild(MechVMMechTreeIterNum);
   MechVMMechTreeIter->addAttribute("name", mechName);

   for (size_t i = 0; i < MW2MechTreeIter.getChildCount(); i++) {
      MW2MechTreeIter.goToChild(i);
      const char* nodeName = MW2MechTreeIter.getNodeName();
      if (strcmp (nodeName, "stats") == 0) {
         addMW2Stats (MW2MechTreeIter, outputPath);
      } else {
         const TCHAR* limbName = MW2MechTreeIter.getAttributeValueByName("mw2prj");
         size_t limbNum = MechVMMechTreeIter->addChild(nodeName);
         MechVMMechTreeIter->goToChild(limbNum);
         if (limbName != NULL) {
            BGString limbPath = limbName;
            limbPath += ".obj";
            MechVMMechTreeIter->addAttribute("path", limbPath.getChars());

            const char* jx = MW2MechTreeIter.getAttributeValueByName("jx");
            const char* jy = MW2MechTreeIter.getAttributeValueByName("jy");
            const char* jz = MW2MechTreeIter.getAttributeValueByName("jz");

            if (jx != NULL)
               MechVMMechTreeIter->addAttribute("jx", jx);
            if (jy != NULL)
               MechVMMechTreeIter->addAttribute("jy", jy);
            if (jz != NULL)
               MechVMMechTreeIter->addAttribute("jz", jz);
         } else {
            for (unsigned int j = 0; j < MW2MechTreeIter.getAttributeCount(); j++) {
               const char* attrName = MW2MechTreeIter.getAttributeName(j);
               const char* attrVal  = MW2MechTreeIter.getAttributeValueByIndex(j);
               MechVMMechTreeIter->addAttribute(attrName, attrVal);
            }
         }
         MechVMMechTreeIter->goToParent();
      }
      MW2MechTreeIter.goToParent();
   }

   MechVMMechTreeIter->goToParent();

   MechVMMechTree.saveToFile (outputFN);

   delete MechVMMechTreeIter;
}

////////////////////////////////////////////////////////////////////////////////
// Copy single mech's limbs from MW2.PRJ

struct MechLimbToSave {
   Mesh* mesh;
   BGString FN;
};

bool MW2MechImporter::copyMechComponents (XMLTreeConstIterator& MW2MechTreeIter, 
   const TCHAR* targetPath, const TCHAR* mechName)
{
   size_t copied = 0;
   //prj->enterSubDir("POLY");
   size_t importedLimbs = 0;
   BGString missingLimbs;
   BGVector <MechLimbToSave*, BGVector_MechLimbToSave> limbs;

   for (size_t i = 0; i < MW2MechTreeIter.getChildCount(); i++) {
      MW2MechTreeIter.goToChild(i);
      const TCHAR* limbName = MW2MechTreeIter.getAttributeValueByName("mw2prj");
      bool imported = false;
      if (limbName != NULL) {
         prj->enterSubDir ("POLY");
         size_t fileNum = prj->getEntryNumberByName (limbName);
         if (fileNum < prj->getEntryCount()) {
            // Import limb from MW2 PRJ
            MemoryBlock* mb = prj->getMemoryBlock(fileNum);
            /*size_t WTBSize = prj->getFileSize(fileNum);
            MemoryBlock* mb = new MemoryBlock(WTBSize);
            prj->getFileChunk(fileNum, 0, mb->getPtr(), WTBSize);*/
            Mesh* mesh = loadLimb(mb);
            if (mesh != NULL) {
               // Translate points
               BGString txstr = MW2MechTreeIter.getAttributeValueByName("tx");
               BGString tystr = MW2MechTreeIter.getAttributeValueByName("ty");
               BGString tzstr = MW2MechTreeIter.getAttributeValueByName("tz");
               double tx, ty, tz;
               if(!txstr.toDouble(tx))
                  tx = 0.0;
               if(!tystr.toDouble(ty))
                  ty = 0.0;
               if(!tzstr.toDouble(tz))
                  tz = 0.0;
               for (size_t j = 0; j < mesh->getPointCount(); j++) {
                  Point3D p;
                  mesh->getPoint(j, p);
                  p = Point3D (p.x+tx, p.y+ty, p.z+tz);
                  mesh->setPoint(j, p);
               }

               // Build atlas
               // Adjust texture coordinates

               // Add limb to list of meshes to save later
               BGString objFN = targetPath;
               objFN += OSPathSep;
               objFN += limbName;
               objFN += ".obj";
               MechLimbToSave* limb = new MechLimbToSave;
               limb->mesh = mesh;
               limb->FN = objFN;
               limbs.add(limb);
               copied++;
            }
            delete mb;
            imported = true;
         }
         if (!imported) {
            //printf ("Could not import %s from %s\n", limbName, 
            //        MW2MechTreeIter.getNodeName());
            if (missingLimbs.getLength() != 0)
               missingLimbs += ", ";
            missingLimbs += limbName;
         } else {
            importedLimbs++;
         }
      }

      MW2MechTreeIter.goToParent();
   }

   if (limbs.getSize() > 0) {
      bgmkdir (targetPath);

      // Build texture atlas
      if (tc != NULL) {
         if (tc->getTextureCount() > 0) {
            Texture* atlas = tc->buildAtlas();
            BGString atlasFN = targetPath;
            atlasFN.appendNoDups (OSPathSep);
            atlasFN += mechName;
            atlasFN += ".BMP";
            atlas->saveToFile(atlasFN.getChars(), 0, 0, atlas->getWidth(), atlas->getHeight());
            delete atlas;
         }

         // Adjust UVs
         for (size_t i = 0; i < limbs.getSize(); i++) {
            MechLimbToSave* limb = limbs[i];
            Mesh* mesh = limb->mesh;
            for (size_t j = 0; j < mesh->getPolygonCount(); j++) {
               MeshPolygon* poly = mesh->getPolygon(j);
               ManagedTexture* manText = poly->m_manText;
               if (manText != NULL) 
                  for (size_t k = 0; k < poly->getPointCount(); k++) {
                     double u, v;
                     poly->getTexCoords (k, u, v);
                     u = manText->transformU(u);// / manText->texture->getWidth());
                     v = manText->transformV(v);// / manText->texture->getHeight());
                     poly->setTexCoords (k, u, v);
                  }
            }
         }
      }

      // Save limbs
      for (size_t i = 0; i < limbs.getSize(); i++) {
         MechLimbToSave* limb = limbs[i];
         limb->mesh->saveToObj(limb->FN.getChars(), true);
         delete limb->mesh;
         delete limb;
         limbs[i] = NULL;
      }
   }

   // Clean up managed textures
   for (size_t i = 0; i < textures.getSize(); ++i) {
      MW2TextureTableEntry* entry = textures[i];
      if (entry->manText != NULL) {
         entry->manText->texture->data = NULL;
         delete entry->manText;
         entry->manText = NULL;
         if (entry->used > 0) {
            printf ("BMID %i -> BMPJ %i: %i times\n", entry->BMID, entry->BMPJ, entry->used);
            entry->used = 0;
         }
      }
   }

   if (tc != NULL) {
      delete tc;
      tc = NULL;
   }

   if (importedLimbs > 0) {
      if (missingLimbs.getLength() == 0) {
         printf ("Completely imported %s\n", mechName);
      } else {
         printf ("Imported %s, but the following limbs are missing: %s\n", 
                 mechName, missingLimbs.getChars());
      }
      return true;
   } else {
      printf ("Could not import %s\n", mechName);
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////
// Import

int MW2MechImporter::import ()
{
   // Build texture table
   const char* importEntryPoint = "BET1SCN1.BWD";
   DWORD fileNum = prj->getEntryNumberByName(importEntryPoint);

   // Add palette
   prj->enterSubDir ("PAL");
   //fileNum = prj->getEntryNumberByName("TANA0FDA.COL");
   //fileNum = prj->getEntryNumberByName("BROW0NDA.COL");
   fileNum = prj->getEntryNumberByName("ICEB0FDA.COL");
   //fileNum = prj->getEntryNumberByName("GREY0FDA.COL");
   //fileNum = prj->getEntryNumberByName("REDA0NDA.COL");
   //fileNum = prj->getEntryNumberByName("FIRE0FDU.COL");
   MemoryBlock* mb = prj->getMemoryBlock(fileNum);
   if (mb != NULL)
      setPalette(mb, false);

   // Load MAP files
   prj->enterSubDir ("BWD");
   //buildTextureTable("MW2_MAP1.BWD");
   //buildTextureTable("MW2_MAP2.BWD");
   //buildTextureTable("MW2_MAP3.BWD");
   //buildTextureTable("BET1SCN1.BWD");
   buildTextureTable("BET1MAP1.BWD");
   //buildTextureTable("BRONSCN1.BWD");
   //buildTextureTable("TIMBRWLF.BWD");

   // Load import control file
   BGString mw2path = getExecDir();
   mw2path += OSPathSep;
   mw2path += "mw2-mechs.xml";
   XMLTree MW2MechTree;

   if (!prj->enterSubDir("POLY")) {
      printf ("Strange PRJ file without POLY subdirectory\n");
      return 0;
   }

   if (!MW2MechTree.loadFromFile(mw2path.getChars())) {
      displayMsg ("Import file missing.\n");
   }

   XMLTreeConstIterator* MW2MechTreeIter = MW2MechTree.getConstIterator();
   //MW2MechTreeIter->goToChild(0);
   mechsImported = 0;

   for (size_t i = 0; i < MW2MechTreeIter->getChildCount(); i++) {
      // Take mech from MW2MechTree
      MW2MechTreeIter->goToChild(i);
      const char* mechName = MW2MechTreeIter->getAttributeValueByName("name");

      BGString targetPath = getExecDir();
      targetPath += OSPathSep;
      targetPath += "mechs";
      targetPath += OSPathSep;
      targetPath += "mw2-";
      targetPath += mechName;

      BGString outFN = targetPath;
      outFN += OSPathSep;
      outFN += mechName;
      outFN += ".mvmk";

      if (!fileExists (outFN.getChars())) {
         // Copy files from MW2 project file
         if (copyMechComponents (*MW2MechTreeIter, targetPath.getChars(), mechName)) {
            createMVMKFile (*MW2MechTreeIter, targetPath.getChars(), outFN.getChars());
            mechsImported++;
         }
      }
      MW2MechTreeIter->goToParent();
   }

   delete MW2MechTreeIter;
   return mechsImported;
}

///////////////////////////////////////////////////////////////////////////////
// Import mechs from a MW2 installation

size_t importMW2Files(const char* source)
{
   // Check for MW2 installation
   BGString PRJPath = source;
   PRJPath += OSPathSep;
   BGString netMechPath = PRJPath;
   netMechPath += "net.prj";
   PRJPath += "mw2.prj";

   MW2MechImporter importer;
   if (importer.setPRJ(PRJPath.getChars())) {
      displayMsg("Importing from MW2 installation\n");
      importer.import();
   } else if (importer.setPRJ(netMechPath.getChars())) {
      displayMsg("Importing from MW2 installation\n");
      importer.import();
   } else {
      displayMsg ("Cannot import - MW2.PRJ not found\n");
   }

   return importer.getImportedMechCount();
}
