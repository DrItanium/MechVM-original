////////////////////////////////////////////////////////////////////////////////
// Class TextureCompiler
// Copyright Björn Ganster in 2005-2006
// This file declares a class for compiling several smaller textures into an
// atlas. While it cannot guarantee that the resulting texture fits into 
// the limits imposed by GL_MAX_TEXTURE_SIZE, it does its best to keep the
// resulting texture as small as possible.
////////////////////////////////////////////////////////////////////////////////

#ifndef TextureCompiler__h
#define TextureCompiler__h

////////////////////////////////////////////////////////////////////////////////
// Includes

#include "Texture.h"

#include <list>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Managed Texture 

class ManagedTexture: public VirtualClass {
public:
   Texture* texture;
   double u1, v1, u2, v2;

   // Constructor
   ManagedTexture()
      : VirtualClass (ManagedTextureID),
        xreps (1),
        yreps (1)
   {}

   // Destructor
   virtual ~ManagedTexture()
   {}

   // Get/Set material parameter
   /*void setMaterial (RenderParameter* newParam)
   { material = newParam; }
   const RenderParameter* getMaterial () const
   { return material; }
   RenderParameter* getMaterial ()
   { return material; }*/

   // Transform coordinates local to texture into global coordinates
   inline double transformU (double u) const
   { return u1 + (u2-u1) * u; }
   inline double transformV (double v) const
   { return v1 + (v2-v1) * v; }

   inline void glTexCoord2d(double u, double v) const
   {
      u = transformU(u);
      v = transformV(v);
      ::glTexCoord2d(u,v);
   }

   inline int getID () const
   { return ID; }

   inline void setID (int newVal)
   { ID = newVal; }

   void checkUVs (int u, int v)
   {
      if (u > 0) {
         int newXreps = (u+((int) texture->getWidth())-1) / ((int) texture->getWidth());
         if (newXreps > xreps)
            xreps = newXreps;
      }
      if (v > 0) {
         int newYreps = (v+((int) texture->getHeight())-1) / ((int) texture->getHeight());
         if (newYreps > yreps)
            yreps = newYreps;
      }
   }

   inline int getXreps() const
   { return xreps; }
   inline int getYreps() const
   { return yreps; }
private:
//   RenderParameter* material;
   int ID, xreps, yreps; //, _u1, _u2, _v1, _v2, uvCount;
};

////////////////////////////////////////////////////////////////////////////////
// Texture compiler compiles several managed textures into an 

class TextureCompiler: public VirtualClass {
public:
   // Constructor
   TextureCompiler (const TCHAR* _name = "");
   
   // Destructor
   virtual ~TextureCompiler ();

   // Add texture to list of managed textures
   ManagedTexture* add (Texture* texture, int ID = 0);

   // Load from files, possibly with wildcards
   // Stores filenames as atlas name
   //int LoadFromFiles(const TCHAR* FN, int BaseID, int IDstride);
   
   // Build OpenGL texture from list of textures, user should free its memory
   Texture* buildAtlas ();
   
   // Get width/height
   inline size_t GetWidth () { return usedWidth; }
   inline size_t GetHeight () { return usedHeight; }

   // Find named texture
   ManagedTexture*  findNamedTexture(const char* name) const;
   ManagedTexture* findTextureByID(int ID) const;

   // Get atlas name
   inline const TCHAR* getName() const
   { return name.getChars(); }

   // Get texture
   ManagedTexture* getTexture(size_t i)
   { 
      if (textures.size() > 0)
         return textures[i%textures.size()];
      else
         return NULL;
   }

   // Get number of included textures
   inline size_t getTextureCount() const
   { return textures.size(); }

   // Get atlas
   inline Texture* getAtlas()
   {
      if (atlas != NULL)
         return atlas; 
      else
         return buildAtlas();
   }

   // Manage textures by ID
   bool removeAndDeleteTextureByID (int ID);
   ManagedTexture* getTextureByID (int ID);

   // Reassign UV mappings in all managed textures
   void reassignPatches();

private:
   Texture* atlas;
   BGString name;
   vector<ManagedTexture*> textures;
   size_t usedWidth, usedHeight;

   inline void invalidateGLTexture()
   {
      if (atlas != NULL) {
         delete atlas;
         atlas = NULL;
      }
   }
};

#endif
