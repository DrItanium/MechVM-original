///////////////////////////////////////////////////////////////////////////////
// MW2MechImporter.h
// Copyright Bjoern Ganster 2009
///////////////////////////////////////////////////////////////////////////////

#ifndef MW2MechImporter__h
#define MW2MechImporter__h

#include "XMLTree.h"
#include "Texture.h"
#include "MechWarriorIIPRJ.h"
#include "TextureCompiler.h"

// Import MW2 files for use in internal engine
size_t importMW2Files(const char* source);

///////////////////////////////////////////////////////////////////////////////
// Help structs

struct MW2TextureTableEntry {
   int BMID;   // ID assigned in BMID tag in BWD
   int BMPJ; // Number of file according to BMPJ tag in BWD
   Texture* texture;
   ManagedTexture* manText;
   size_t used; // Counts how often the texture was used, for debugging
};

///////////////////////////////////////////////////////////////////////////////
// Class for importing MW2 mechs

class MW2MechImporter {
public:
   // Constructor
   MW2MechImporter();

   // Destructor
   ~MW2MechImporter();

   // Open MW2.PRJ
   bool setPRJ (const char* PRJFN);

   // Import
   int import ();

   // Get number of imported mechs
   inline size_t getImportedMechCount() const
   { return mechsImported; }

private:
   MechWarriorIIPRJ* prj;
   size_t mechsImported;
   MemoryBlock* palette;
   BGVector <MW2TextureTableEntry*, BGVector_MW2_TextureTableEntry> textures;
   TextureCompiler* tc;

   // Copy data from mw2-mechs.xml into a new .mvmk file
   void createMVMKFile (
      XMLTreeConstIterator& MW2MechTreeIter, 
      const char* outputPath, const char* outputFN);

   // Copy MW2 mech config file (*.MEK)
   void addMW2Stats (XMLTreeConstIterator& MW2MechTreeIter, 
      const BGString& outputPath);

   // Copy single mech's limbs from MW2.PRJ
   bool copyMechComponents (XMLTreeConstIterator& MW2MechTreeIter, 
      const TCHAR* targetPath, const TCHAR* mechName);

   // Load mech limb from MemoryBlock
   Mesh* loadLimb (const MemoryBlock* block);

   // Build texture table
   void buildTextureTable (const char* BWDFN);
   void buildTextureTable (DWORD fileNum);

   // Retrieve texture, loading it when necessary
   ManagedTexture* getTexture (int textureID);

   // Get file ID
   int getTextureNumFromID (int textureID);

   // Set palette
   void setPalette (MemoryBlock* mb, bool deepCopy);
};

#endif
