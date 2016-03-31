////////////////////////////////////////////////////////////////////////////////
// MechWarrior 3 ZBD file system
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef MechWarrior3ZBD__H
#define MechWarrior3ZBD__H

#include "BGString.h"
#include "BGVector.h"
#include "FileCache.h"
#include "Archive.h"
#include "BGVector.h"
#include "Point3D.h"
#include "Matrix.h"
#include "TextureCompiler.h"

#include <vector>
using namespace std;

class Texture;
class Mesh;
class XMLTree;

////////////////////////////////////////////////////////////////////////////////

// Decode 555 image
Texture* Decode_MW3_555 (unsigned char* mem, size_t size);

////////////////////////////////////////////////////////////////////////////////

class MechWarrior3ZBD: public MWArchive {
public:
   // Constructor
   MechWarrior3ZBD();

   // Destructor
   ~MechWarrior3ZBD();

   // Open file
   virtual bool open (const TCHAR* PRJFN, bool readOnly = true);

   // Close file
   virtual void close()
   { fc.close(); }

};

////////////////////////////////////////////////////////////////////////////////
// Class to import MW3 mechs from ZBD files

class MW3MechImporter {
public:
   // Constructor
   MW3MechImporter (const char* zbdDir, const char* outputDir, const char* importXML);

   // Destructor
   ~MW3MechImporter();

   // Open mech geometry
   void exportFLT (MemoryBlock* flt);

private:
   MechWarrior3ZBD* m_mechLib, * m_textureLib;
   TextureCompiler* m_tc;
   BGFilePath<TCHAR> m_mechOutputDir, m_targetPath;
   vector<BGString> OBJFNs;
   vector<Mesh*> meshes;
   vector<BGString> textureFNs;

   // Open mech geometry (recursive part)
   void extractNodes (MemoryBlock* flt, size_t& offset, Matrix m);
};

#endif
