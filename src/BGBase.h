////////////////////////////////////////////////////////////////////////////////
// Virtual class/memory tracer declaration
// Copyright Bjoern Ganster 2005-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef BGBase__H
#define BGBase__H

#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#ifdef WIN32
#include "Windows.h"
#endif

#define storeClassID
//#define SimpleMemoryCounters

////////////////////////////////////////////////////////////////////////////////
// Linux does not define TCHAR

#ifdef _WIN32
#include <windows.h>
#else
#ifdef UNICODE
typedef wchar_t TCHAR;
#pragma message "Producing unicode executable"
#else
typedef char TCHAR;
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Virtual class: introduces memory profiling and message passing to all
// subclasses

class MessageListener;

class VirtualClass {
public:
   // Default constructor
   VirtualClass(int classID = 0);

   // Copy constructor
   VirtualClass (const VirtualClass& other);

   // Destructors of classes with virtual functions must also be virtual
   virtual ~VirtualClass();

   #ifdef storeClassID
   int getClassID () const
   { return classID; }
   #endif
protected:
   // Send messages to other objects
   bool sendMessage (MessageListener* receiver, int MessageType);
private:
   #ifdef storeClassID
   int classID;
   #endif
};

////////////////////////////////////////////////////////////////////////////////
// Class for message listeners

class MessageListener: public VirtualClass {
public:
   // Constructor
   MessageListener (int ID = 0)
      : VirtualClass (ID)
   {}

   // Destructor
   virtual ~MessageListener ()
   {}

   // Function to receive messages
   virtual void receiveMessage (VirtualClass* sender, int MessageType)= 0;
};

////////////////////////////////////////////////////////////////////////////////
// Memory profiling support

#ifdef UseMemMan
void addPointer (void* p, size_t size, const char* _name);
void releasePointer (void* p);
void listUnfreed();

void printPointer (void* p);
#endif

// Clear memory counters
void clearClassUsage ();

// Get amount of memory used by typeID
size_t getMemoryUsedByType (int typeID);

// Dump memory counters for all classes
void dumpMemoryUsage ();

void initTypeSizes();

////////////////////////////////////////////////////////////////////////////////
// Allocate memory with trace support, if enabled

#ifdef UseMemMan
inline void* traceAlloc (size_t bytes, int classID)
{
   void* result = malloc (bytes);
   #ifdef UseMemMan
   addPointer (result);
   #endif
   return result;
}
#else
#ifdef SimpleMemoryCounters
void* traceAlloc (size_t bytes, int classID);
#else
inline void* traceAlloc (size_t bytes, int /*classID*/)
{ return malloc (bytes); }
#endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Free memory with trace support, if enabled

#ifdef SimpleMemoryCounters
void traceFree (void* p, int classID, size_t usedMem = 0);
#else
inline void traceFree (void* p, int /*classID*/, size_t /*usedMem*/ = 0)
{
   free (p);
   #ifdef UseMemMan
   releasePointer (p);
   #endif
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Random functions

inline void randomize ()
{ 
   time_t now;
   time(&now);
   srand ((unsigned int) now);
}

inline float frandom (float a = 0.0, float b = 1.0)
{ return a + (b-a) * ((float) rand ()) / ((float) (RAND_MAX + 1.0f)); }

inline double drandom (double a = 0.0, double b = 1.0)
{ return a + (b-a) * ((double) rand ()) / ((double) (RAND_MAX + 1.0f)); }

// Integer random, see man 3 rand
inline int random (int a, int  b)
{ return a + ((int) (((double) (b-a+1)) * rand () / (RAND_MAX + 1.0))); }

////////////////////////////////////////////////////////////////////////////////
// Put b into the range of a, c
template <class Type>
   const Type& putInRange (
      const Type& min, 
      const Type& val,
      const Type& max)
{
   if (val < min)
      return min;
   else if (val < max)
      return val;
   else
      return max;
}

////////////////////////////////////////////////////////////////////////////////
// Minimum, maximum computation

// Calculate minimum, maximum at the same time
template <class T>
inline void minmax (const T& a, const T& b, T& min, T& max)
{
   if (a < b) {
      T help = b; // Keep a copy in case of swap 
      min = a;
      max = help;
   } else {
      T help = a; // Keep a copy in case of swap 
      min = b;
      max = help;
   }
}

// Minimum template function
template <class T>
inline T bgmin (const T& a, const T& b)
{
   if (a < b) {
      return a;
   } else {
      return b;
   }
}

// Maximum template function
template <class T>
inline T bgmax (const T& a, const T& b)
{
   if (a > b) {
      return a;
   } else {
      return b;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Class IDs

const int UndefinedClassID = 0;
const int BGVectorID = 1;
const int BGStringID = 2;
const int BGStringBufID = 3;
const int Point3DID = 4;
const int ColorID = 5;
const int TextureID = 6;
const int MatrixID = 7;
const int MatrixStackID = 8;
const int XMLAttributeID = 9;
const int XMLTreeNodeID = 10;
const int XMLTreeID = 11;
const int XMLTreeConstIteratorID = 12;
const int XMLTreeIteratorID = 13;
const int FileCacheID = 14;
const int BGVector_string_ID = 15;
const int TextureCompilerID = 16;
const int ManagedTextureID = 17;
const int GLWindowID = 18;
const int RenderableObjectID = 19;
const int Mesh2PointsID = 20;
const int Mesh2TrianglesID = 20;
const int MeshPoint3DDataID = 21;
const int MeshID = 22;
const int MeshPolygonID = 23;
const int MeshPolygonEdgeID = 24;
const int MemoryBlockID = 25;
const int Mesh2ID = 26;
const int VehicleID = 27;
const int MechBayChassisTableID = 28;
const int HeightFieldID = 33;
const int Mesh2NormalsID = 34;
const int Mesh2TexCoordsID = 35;
const int BGVectorBufID = 36;
const int BGVector_MW2_TextureTableEntry = 37;
const int BGVector_MechLimbToSave = 38;
const int BGVector_const_char_p = 39;
const int BGVector_int = 40;
const int BGVector_MechShellImage = 41;
const int BGVector_string = 42;
const int BGVector_MeshPolygon = 43;
const int BGVector_PolygonListBufferID = 44;
const int BGVector_BGVector_PolygonVectorID = 45;
const int WeaponsLockerID = 46;
const int Mesh3ID = 47;
const int WeaponSystemID = 48;
//const int TextureID = 49;

const int ClassIDCount = 50;

////////////////////////////////////////////////////////////////////////////////
// Message IDs

const int ButtonClickedEvent = 0;
const int ToolbarButtonClickedEvent = 1;
const int UpdateWidgetsEvent = 2;
const int ValueChangedEvent = 3; // Sent by GLLineEdit, GLSlider
const int ContextMenuClickEvent = 4; // Sent by glutContextMenuEntry
const int TableItemSelected = 5; // Sent by GLTableView
const int ContextMenuRequested = 6;
const int CheckBoxChecked = 7;
const int CheckBoxUnchecked = 8;

////////////////////////////////////////////////////////////////////////////////
// Translation support

bool loadLangFile (const TCHAR* FileName);
const char* tr (int strNum);

////////////////////////////////////////////////////////////////////////////////

template <class T> 
class BGProperty {
public:
   // Constructor
   BGProperty (const T& initVal)
      : value (initVal)
   {
   }

   inline const T& get () const
   { return value; }
   inline T& get ()
   { return value; }

   inline void set (const T& newVal)
   { value = newVal; }

private:
   T value;
};

#endif
