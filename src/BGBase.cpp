////////////////////////////////////////////////////////////////////////////////
// BGBase
// Copyright Bjoern Ganster 2005-2010
////////////////////////////////////////////////////////////////////////////////

#include "BGBase.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef SimpleMemoryCounters
size_t ClassesInUse [ClassIDCount];
size_t MemoryInUse [ClassIDCount];
#endif 

size_t TypeSize [ClassIDCount];

////////////////////////////////////////////////////////////////////////////////

VirtualClass::VirtualClass(int classID)
{
   #ifdef UseMemMan
   if (name != NULL)
      AddPointer (this, sizeof (*this), name);
   else
      AddPointer (this, sizeof (*this), "Unnamed VirtualClass descendant");
   #endif
   #ifdef storeClassID
   this->classID = classID;
   #endif
   #ifdef SimpleMemoryCounters
   ClassesInUse[classID]++;
   MemoryInUse[classID] += TypeSize [classID];
   #endif
}

// Copy Constructor
VirtualClass::VirtualClass (const VirtualClass& other)
{
   #ifdef UseMemMan
   if (name != NULL)
      AddPointer (this, sizeof (*this), name);
   else
      AddPointer (this, sizeof (*this), "Unnamed VirtualClass descendant");
   #endif
   #ifdef SimpleMemoryCounters
   classID = other.classID;
   ClassesInUse[classID]++;
   MemoryInUse[classID] += TypeSize [classID];
   #endif
}

VirtualClass::~VirtualClass() 
{
   #ifdef SimpleMemoryCounters
   ClassesInUse[classID]--;
   MemoryInUse[classID] -= TypeSize[classID];
   #endif

   #ifdef UseMemMan
   ReleasePointer (this);
   #endif
}

bool VirtualClass::sendMessage (MessageListener* receiver, int MessageType)
{ 
   if (receiver != NULL) {
      receiver->receiveMessage (this, MessageType); 
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// MemMan

#ifdef UseMemMan

MemMan memman;

void ListUnfreed()
{
   memman.ListUnfreed();
}

void AddPointer (void* p, size_t size, const char* _name)
{
   memman.store (p, size, _name);
}

void ReleasePointer (void* p)
{
   memman.release (p);
}

void printPointer (void* p) 
{
   memman.printPointer (p);
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Clear memory counters

#ifdef SimpleMemoryCounters

void* traceAlloc (size_t bytes, int classID)
{ 
   ClassesInUse[classID]++;
   MemoryInUse[classID] += bytes;
   return malloc (bytes); 
}

void traceFree (void* p, int classID, size_t usedMem)
{
   ClassesInUse[classID]--;
   MemoryInUse[classID] -= usedMem; // unknown amount
   free (p);
}

void clearClassUsage ()
{
   for (int i = 0; i < ClassIDCount; i++) {
      ClassesInUse[i] = 0;
      MemoryInUse[i] = 0;
   }
}

// Dump memory counters for all classes
void dumpMemoryUsage ()
{
   size_t totalMem = 0;
   for (int i = 0; i < ClassIDCount; i++) {
      if (ClassesInUse[i] > 0 || MemoryInUse[i] > 0) {
         BGString msg;
         msg.appendInt (i);
         msg += ": ";
         msg.appendUInt (ClassesInUse[i]);
         msg += " use ";
         msg.appendUInt (MemoryInUse[i]);
         msg += " bytes (";
         msg.appendSizeStr (MemoryInUse[i]);
         msg += ")\n";
         msg.print();
         totalMem += MemoryInUse[i];
      }
   }

   BGString bcStr, sizeStr;
   bcStr.assignUInt(totalMem);
   sizeStr.appendSizeStr(totalMem);
   printf ("Total mem use known: %s bytes (%s)\n", bcStr.getChars(), sizeStr.getChars());
}

size_t getMemoryUsedByType (int typeID)
{
   return MemoryInUse[typeID];
}

#else

// Dump memory counters for all classes
void dumpMemoryUsage ()
{ printf ("Memory counters are inactive\n"); }

#endif

#include "XMLTree.h"
#include "TextureCompiler.h"
#include "Mesh.h"
#include "Mesh2.h"
#include "dialogs.h"
#include "GLLabel.h"
#include "Vehicles.h"
#include "Heightfield.h"

void initTypeSizes ()
{
   TypeSize[UndefinedClassID] = 0;
   TypeSize[BGStringID] = sizeof (BGString);
   TypeSize[XMLAttributeID] = sizeof (XMLAttribute);
   TypeSize[XMLTreeNodeID] = sizeof (XMLTreeNode);
   TypeSize[XMLTreeID] = sizeof (XMLTree);
   TypeSize[XMLTreeConstIteratorID] = sizeof (XMLTreeConstIterator);
   TypeSize[XMLTreeIteratorID] = sizeof (XMLTreeIterator);
   TypeSize[TextureID] = sizeof (Texture);
   TypeSize[Point3DID] = sizeof (Point3D);
   TypeSize[ColorID] = sizeof (Color);
   TypeSize[MatrixID] = sizeof (Matrix);
   TypeSize[MatrixStackID] = sizeof (MatrixStack);
   TypeSize[FileCacheID] = sizeof (FileCache);
   TypeSize[TextureCompilerID] = sizeof (TextureCompiler);
   TypeSize[ManagedTextureID] = sizeof (ManagedTexture);
   TypeSize[GLWindowID] = sizeof (GLWindow);
   TypeSize[RenderableObjectID] = sizeof (RenderableObject);
//   TypeSize[Mesh2PointsID] = sizeof (Mesh2Point);
//   TypeSize[Mesh2TrianglesID] = sizeof (Mesh2Triangle);
//   TypeSize[MeshPoint3DDataID] = sizeof (MeshPoint3DData);
   TypeSize[MeshID] = sizeof (Mesh);
   TypeSize[MeshPolygonID] = sizeof (MeshPolygon);
//   TypeSize[MeshPolygonEdgeID] = sizeof (MeshPolygonEdge);
   TypeSize[MemoryBlockID] = sizeof (MemoryBlock);
   TypeSize[Mesh2ID] = sizeof (Mesh2);
   TypeSize[VehicleID] = sizeof (Vehicle);
//   TypeSize[MechBayChassisTableID] = sizeof (MechBayChassisTable);
//   TypeSize[Mesh2NormalsID] = sizeof (Mesh2Normals);
//   TypeSize[Mesh2TexCoordsID] = sizeof (Mesh2TexCoords);
   TypeSize[BGVectorBufID] = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Translation support

MemoryBlock* langFile;
BGVector <const char*, BGVector_const_char_p> trStrings;

bool loadLangFile (const TCHAR* FileName)
{
   // Load strings
   langFile = new MemoryBlock();
   if (!langFile->loadFromFile(FileName)) {
      return false;
   }

   // Obtain string pointers
   size_t i = 0;
   int strStart = 0;
   int strEnd = 0;

   while (i < langFile->getSize()) {
      char c = langFile->getByte(i);
      if (c == 0x0a || c == 0x0d) {
         if (i == (size_t)(strEnd+1)) {
            int len = strEnd-strStart+1;
            char* line = new char [len+1];
            memcpy((void*) line, (void*) langFile->getPtr(strStart), len);
            line[len] = (char) 0;
            trStrings.add(line);
            strStart = -1;
            strEnd = -1;
         } else
            ++i;
      } else {
         if (strStart == -1)
            strStart = i;
         strEnd = i;
         ++i;
      }
   }

   return true;
}

const char* tr (int strNum)
{
   return trStrings[strNum];
}
