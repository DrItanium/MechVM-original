////////////////////////////////////////////////////////////////////////////////
// Implementation of TextureCompiler
// Copyright Bjoern Ganster in 2005-2008
////////////////////////////////////////////////////////////////////////////////

#include "TextureCompiler.h"
#include "BGString.h"
#include "FileCache.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Constructor

TextureCompiler::TextureCompiler (const TCHAR* _name)
: VirtualClass (TextureCompilerID),
  atlas (NULL),
  usedWidth (0),
  usedHeight (0),
  name (_name)
{
   #ifdef UseMemMan
   AddPointer (this, sizeof (*this), "TextureCompiler");
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

TextureCompiler::~TextureCompiler ()
{
   // mantext is destroyed by objSaver::execute
   #ifdef UseMemMan
   ReleasePointer (this);
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Add texture to list of managed textures, returns NULL if unsucessful
// The texture will be deleted when TextureCompiler is deleted

ManagedTexture*  TextureCompiler::add (Texture* texture, int ID)
{
   // Find smallest unused space that texture fits into
   assert (texture != NULL);
   ManagedTexture* mantext = NULL;
   mantext = new ManagedTexture();
   mantext->texture = texture;
   mantext->setID (ID);
   textures.push_back(mantext);

   invalidateGLTexture();

   return mantext;
}

////////////////////////////////////////////////////////////////////////////////
// Build OpenGL texture from list of textures, user should free its memory

Texture* TextureCompiler::buildAtlas ()
{
   reassignPatches();
   printf ("Creating %ix%i atlas\n", usedWidth, usedHeight);

   // Construct texture for icons
   atlas = new Texture (usedWidth, usedHeight, GL_RGBA);
   for (vector<ManagedTexture*>::iterator i = textures.begin (); 
        i != textures.end (); 
        i++) 
   {
      ManagedTexture* mantext = (*i);
      Texture* texture = mantext->texture;
      for (size_t x = 0; (int) x < mantext->getXreps(); ++x)
         for (size_t y = 0; (int) y < mantext->getYreps(); ++y)
            atlas->copy (texture, 0, 0, texture->getWidth (),
                         texture->getHeight (), 
                         (size_t) mantext->u1+x*texture->getWidth(), 
                         (size_t) mantext->v1+y*texture->getHeight());
      mantext->u1 = (mantext->u1-0.5) / usedWidth;
      mantext->v1 = (mantext->v1-0.5) / usedHeight;
      mantext->u2 = (mantext->u2+0.5) / usedWidth;
      mantext->v2 = (mantext->v2+0.5) / usedHeight;
   }

   //atlas->saveToFile("d:\\temp\\test.bmp");

   return atlas;
}

////////////////////////////////////////////////////////////////////////////////
// Find named texture

ManagedTexture* TextureCompiler::findNamedTexture(const char* name) const
{
   // If name is NULL, report NULL immediately
   if (name == NULL)
      return NULL;

   // Test all texture names
   for (vector<ManagedTexture*>::const_iterator i = textures.begin (); 
        i != textures.end (); 
        i++) 
   {
      ManagedTexture* mantext = (*i);
      const char* textureName = mantext->texture->getName();
      if (textureName != NULL) {
         if (strcmp (textureName, name) == 0) {
            return mantext;
         }
      }
   }

   // If nothing was found, return NULL
   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Load from files, possibly with wildcards
// Stores filenames as atlas name

/*int TextureCompiler::LoadFromFiles(const TCHAR* path, int BaseID, int IDstride)
{
   // Find first file
   name = path;
   FileLister fl;
   fl.listFiles(path, "*.bmp");
   int ID = BaseID;

   while (fl.found()) {
      BGString FN = path;
      FN += OSPathSep;
      FN += fl.getFN();
      Texture* newText = new Texture();
      if (newText->loadFromFile(FN.getChars())) {
         add(newText, ID);
         ID += IDstride;
      } else
         delete newText;

      // Find next file
      fl.getNext();
   }

   if (ID != BaseID)
      invalidateGLTexture();

   return (ID-BaseID) / IDstride;
}*/

////////////////////////////////////////////////////////////////////////////////
// Manage textures by ID

bool TextureCompiler::removeAndDeleteTextureByID (int ID)
{
   bool found = false;
   vector<ManagedTexture*>::iterator i = textures.begin();
   while (i != textures.end())
   {
      ManagedTexture* candidate = (*i);
      if (candidate->getID() == ID) {
         delete candidate->texture;
         delete candidate;
         i = textures.erase (i);
         found = true;
         invalidateGLTexture();
      } else
         i++;
   }

   return found;
}

ManagedTexture* TextureCompiler::getTextureByID (int ID)
{
   ManagedTexture* result = NULL;;
   vector<ManagedTexture*>::const_iterator i = textures.begin();
   while (i != textures.end())
   {
      ManagedTexture* candidate = (*i);
      if (candidate->getID() == ID) {
         return (*i);
      } else
         i++;
   }

   return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Reassign UV mappings in all managed textures

void TextureCompiler::reassignPatches()
{
   if (textures.size() > 0) {
      // Determine biggest height parameter and sum of widths
      ManagedTexture* mt = textures[0];
      Texture* text = mt->texture;
      usedHeight = text->getHeight();
      usedWidth = text->getWidth();

      for (size_t i = 1; i < textures.size(); i++) {
         ManagedTexture* mt = textures[i];
         Texture* text = mt->texture;
         if (text->getHeight()*mt->getYreps() > usedHeight)
            usedHeight = text->getHeight()*mt->getYreps();
         usedWidth += text->getWidth()*mt->getXreps();
      }

      // Create power-of-two texture
      size_t w = 1, h = 1;
      while (w < usedWidth)
         w += w;
      while (h < usedHeight)
         h += h;
      usedWidth = w;
      usedHeight = h;

      // Assign coordinates
      double u = 0;
      for (size_t i = 0; i < textures.size(); i++) {
         ManagedTexture* mt = textures[i];
         Texture* text = mt->texture;
         double udelta = text->getWidth();
         mt->u1 = u;
         mt->u2 = u + udelta;
         mt->v1 = 0.0;
         mt->v2 = text->getHeight();
         u += udelta*mt->getXreps();
      }

      invalidateGLTexture();
   }
}

////////////////////////////////////////////////////////////////////////////////

ManagedTexture* TextureCompiler::findTextureByID(int ID) const
{
   // Test all textures
   for (size_t i = 0; i < textures.size(); i++) {
      ManagedTexture* mantext = textures[i];
      if (mantext->getID() == ID) {
         return mantext;
      }
   }

   // If nothing was found, return NULL
   return NULL;
}
