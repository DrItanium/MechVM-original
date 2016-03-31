////////////////////////////////////////////////////////////////////////////////
// Class RenderableObject
// Base class for renderable objects
// Copyright Bjoern Ganster 2000-2007
////////////////////////////////////////////////////////////////////////////////

#ifndef RenderableObject__h
#define RenderableObject__h

////////////////////////////////////////////////////////////////////////////////
// Includes & Forwards

#ifdef _WIN32
#include "windows.h"
#else
#include "BGString.h" // For TCHAR definition
#endif

#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Color.h"
#include "Point3D.h"
#include "Matrix.h"

class MatrixStack;

////////////////////////////////////////////////////////////////////////////////
// Defines

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifdef WIN32
   const char DirSepChar = '\\';
#endif
#ifndef WIN32
   const char DirSepChar = '/';
#endif

////////////////////////////////////////////////////////////////////////////////
// RenderParameter class, used for light sources in plab

class RenderParameter: public VirtualClass {
public:
   // Constructor
   RenderParameter (int classID)
      : VirtualClass (classID)
   {}

   // Destructor must be virtual
   virtual ~RenderParameter()
   {}

   // Create copy
   virtual RenderParameter* copy () = 0;

   // Activate light source, returns true on success
   virtual bool activate () = 0;

   // Deactivate light source
   virtual void deactivate () = 0;

   // Refresh parameter
   virtual void refresh () = 0;

   // Get parameter state
   virtual bool isActive () const = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Class RenderableObject

class RenderableObject: public VirtualClass {
   public:
      //int data;
		bool visible;
		Color color;
      //int deletions;

      // Constructor
      RenderableObject (int classID, RenderableObject* parent = NULL, 
                        const char* _name = NULL);

      // Destructor
      virtual ~RenderableObject ();

      // Render entire scene
      virtual void render ();
      virtual void renderSelf ()
      { }
      virtual void renderChilds ();

      // Mark front-facing polygons (requires neighbourhood information)
      virtual void markFrontFacingPolygons (const Point3D& /*ls*/)
      {}

      // Render shadow volumes
      virtual void renderShadowVolumes (const Point3D& /*ls*/) const
      {}

      // Return number of contained polygons
      virtual size_t getRecursivePolygonCount () const
      { return 0; }

      // Return number of contained points
      virtual size_t getRecursivePointCount () const;

      // Add contained object
      bool addChild (RenderableObject* obj);

      // Add parent objects
      bool addParent (RenderableObject* obj);

      // Detach an object from its parents
      void detachObject();

      // Get number of contained objects
      virtual size_t getObjectCount () const
      { return childs.size(); }

      // Get contained object
      virtual RenderableObject* getObject (size_t index)
      { return childs[index]; }
      virtual const RenderableObject* getObject (size_t index) const
      { return childs[index]; }

      // Detach a child from this object
      virtual bool removeObject (RenderableObject*, bool);

      // Remove parent objects
      void removeParent (RenderableObject* obj);
      
      // Get/set object's name
      const TCHAR* getName() const;
      void setName(const TCHAR* newName);
      
      // Print parents (used for debugging)
      void printParents();

      // Add render parameter
      void addRenderParameter (RenderParameter* newParam)
      { parameters.push_back (newParam); }

      // Mesh size calculation
      void getSize(Point3D& minCoords, 
         Point3D& maxCoords, size_t& polygons);

      // Recursive Part of mesh size calculation
      virtual void getSizeRecursive(Point3D& minCoords, Point3D& maxCoords, 
                                    size_t& polygons, MatrixStack& m);

      // Should only be called from its parent to indeicate that the object need
      // not detach itself from its parent
      void NeedNotDetach ()
      { NeedsToDetach = false; }

      // Set/get matrix
      inline const Matrix& getMatrix () const
      { return M; }

      inline Matrix& getMatrix ()
      { return M; }

      inline void setMatrix (const Matrix& _M)
      { M = _M; }
   protected:
      Matrix M;

      // Subclasses should call this function before rendering
      void activateParameters ();
      
      // Subclasses should call this function after rendering
      void deactivateParameters ();

      // Subclasses should call this function to refresh render parameters
      void refreshParameters ();

      vector <RenderableObject*> childs;
      vector <RenderableObject*> parents;
      vector <RenderParameter*> parameters;
      char* name;
      bool NeedsToDetach;
};

#endif
