////////////////////////////////////////////////////////////////////////////////
// Class Texture
// Copyright Bjoern Ganster in 2000-2007
////////////////////////////////////////////////////////////////////////////////

#ifndef Texture__h
#define Texture__h

////////////////////////////////////////////////////////////////////////////////
// Includes

//#define MATERIAL_SUPPORT
//define TextureRasterizers

#include "RenderableObject.h"
#include "BGBase.h"
#include "Color.h"
#include "assert.h"
#include "BGString.h" // Linux does not define TCHAR, but BGString.h does

////////////////////////////////////////////////////////////////////////////////
// Beginning with OpenGL 1.2, GL_BGR_EXT is replaced by GL_BGR.
// For compatibility with older systems, we define GL_BGR ourselves, if it is
// not provided.

#ifndef GL_BGR
   #ifdef GL_BGR_EXT
      #define GL_BGR GL_BGR_EXT
   #endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Class declaration

class Texture: public VirtualClass
{
public:
   VirtualClass* data; // user-defined data

   // Constructor
   Texture (size_t sizex = 0, size_t sizey = 0, 
            GLenum _format = GL_BGR, const char* newname = NULL);

   // Destructor
   virtual ~Texture ();

   // Load a texture from a file
   bool loadFromFile (const TCHAR* FileName);

   // Save a texture to a file
   //void saveToFile (const wchar_t* FileName);
   bool saveToFile (const TCHAR* FileName, size_t left, size_t top, 
                    size_t saveWidth, size_t saveHeight);

   // Read a texture from framebuffer
   void readFromFrameBuffer (unsigned int x, unsigned int y);

   // Prepare texture for rendering (will activate texture when needed)
   void use ();
   void useMipmap ();

   // Deactivate texture
   void deactivate();

   // Resize the texture (contents are lost)
   bool resize (size_t sizex, size_t sizey, GLenum _format);

   // Some simple get/set functions
   inline size_t getWidth () const { return width; }
   inline size_t getHeight () const { return height; }
   inline unsigned char* getPicData () { return texels; }
   inline const unsigned char* getPicData () const { return texels; }

   // Copy part of another texture into this one
   void copy (Texture* other, size_t x1, size_t y1, size_t x2, size_t y2, 
              size_t tx, size_t ty);

   inline void setRGB (size_t u, size_t v, unsigned char r, 
                       unsigned char g, unsigned char b)
   {
      if (u < width && v < height) {
         size_t index = texelComponents*(v*potWidth+u); 
         texels [index + redOffset] = r;
         texels [index + greenOffset] = g;
         texels [index + blueOffset] = b;
      }
   }

   inline void setRGBA (unsigned int u, unsigned int v, unsigned char r, 
                        unsigned char g, unsigned char b, unsigned char a)
   {
      size_t index = texelComponents*(v*potWidth+u); 
      texels [index + redOffset] = r;
      texels [index + greenOffset] = g;
      texels [index + blueOffset] = b;

      if (format == GL_RGBA)
         texels [index+alphaOffset] = a;
   }

   inline void getRGB (size_t u, size_t v, unsigned char& r, 
                       unsigned char& g, unsigned char& b) const
   {
      size_t index = texelComponents*(v*potWidth+u); 
      r = texels [index + redOffset];
      g = texels [index + greenOffset];
      b = texels [index + blueOffset];
   }

   inline void getRGBA (size_t u, size_t v, unsigned char& r, 
                        unsigned char& g, unsigned char& b, unsigned char& a) 
                        const
   {
      size_t index = texelComponents*(v*potWidth+u); 
      r = texels [index + redOffset];
      g = texels [index + greenOffset];
      b = texels [index + blueOffset];

      if (format == GL_RGBA)
         a = texels [index+alphaOffset];
   }


   inline Color getColor (unsigned int u, unsigned int v) const
   {
      unsigned char r, g, b;
      getRGB (u, v, r, g, b);
      return Color (((ColorBase) r)/255, ((ColorBase) g)/255, ((ColorBase) b)/255);
   }

   inline void setColor (unsigned int u, unsigned int v, const Color& c)
   {
      unsigned char r = (unsigned char) (c.r * 255);
      unsigned char g = (unsigned char) (c.g * 255);
      unsigned char b = (unsigned char) (c.b * 255);
      setRGB (u, v, r, g, b);
   }

   // Get/set texture's name
   inline const char* getName() const 
   { return name; }
   void setName (const char* newName);

   // Get/Set material parameter
   #ifdef MATERIAL_SUPPORT
   inline void setMaterial (RenderParameter* newParam)
   { material = newParam; }
   inline const RenderParameter* getMaterial () const
   { return material; }
   inline RenderParameter* getMaterial ()
   { assert (this != NULL); return material; }
   #endif

   // Query for alpha component
   inline bool hasAlpha () const
   { return format == GL_RGBA; }  

   inline unsigned char getComponentCount()
   { return texelComponents; }

   inline void glTexCoord (double u, double v) const
   { 
      translateCoords(u,v); 
      glTexCoord2d(u,v); 
   }

   // Translate coordinates
   inline void translateCoords (double& u, double& v) const
   {
      u = u * ((double) (width)) / ((double) (potWidth));
      v = v * ((double) (height)) / ((double) (potHeight));
   }

   #ifdef TextureRasterizers
   // Render a line (includes line endpoint)
   void renderLine (int x1, int y1, int x2, int y2, 
                    unsigned char r, unsigned char g, unsigned char b);

   // Fill triangle
   void fillTriangle (int x1, int y1, int x2, int y2, int x3, int y3,
                      unsigned char r, unsigned char g, unsigned char b);

   // Fill star-shaped polygon given as a vector
   void fillPolygon (const vector <int>& polygon, unsigned char r,
                     unsigned char g, unsigned char b, int cx, int cy);

   // Fill square
   void fillSquare (int x1, int y1, int x2, int y2, unsigned char r,
                    unsigned char g, unsigned char b);
   #endif

   bool isActive() const
   { return active; }

private:
   size_t width, height, potWidth, potHeight; // pot: power-of-two
   GLuint GLHandle;
   unsigned char* texels;
   GLenum format;
   unsigned char texelComponents; // Number of components in a texel
   size_t redOffset, blueOffset, greenOffset, alphaOffset;
   char* name;
   #ifdef MATERIAL_SUPPORT
   RenderParameter* material;
   #endif
   bool active;
};

#endif
