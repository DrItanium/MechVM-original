////////////////////////////////////////////////////////////////////////////////
// Implementation of class Texture
// Copyright Bjoern Ganster in 2001-2007
////////////////////////////////////////////////////////////////////////////////

#include "Texture.h"
#include "FileCache.h"
#include "BGString.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>

// In order to use DevIL, define UseDevIL here and link the dll
// Could not figure out how to get DevIL working with icc yet
//#define UseDevIL

#ifdef UseDevIL
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor

Texture::Texture (size_t sizex, size_t sizey, GLenum _format, const char* name)
: VirtualClass (TextureID),
  data (NULL),
  GLHandle (0),
  texels (NULL),
  #ifdef MATERIAL_SUPPORT
  material (NULL),
  #endif
  active (false)
{ 
   resize (sizex, sizey, _format); 
   if (name != NULL) {
      size_t namelen = strlen (name);
      this->name = new char[namelen+1];
      memcpy (this->name, name, namelen+1);
   } else
      this->name = NULL;
   
   #ifdef UseMemMan
   if (name != NULL)
      AddPointer (this, sizeof (*this), name);
   else
      AddPointer (this, sizeof (*this), "Unnamed texture");
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Texture::~Texture ()
{
   if (texels != NULL)
      delete texels;
   if (data != NULL)
      delete data;
   if (name != NULL)
      delete name;
   deactivate();
   #ifdef MATERIAL_SUPPORT
   if (material != NULL)
      delete material;
   material = NULL;
   #endif

   texels = NULL;
   data = NULL;
   name = NULL;
   name = NULL;
   GLHandle = 0;
   active = false;

   #ifdef UseMemMan
   ReleasePointer (this);
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Read a word (i.e., two bytes) from a buffer

unsigned short readWord (unsigned char buffer[], int offset)
{
   return buffer[offset] + 256*buffer[offset+1];
}

////////////////////////////////////////////////////////////////////////////////
// Read a long (i.e., a 32-bit unsigned integer) from a buffer

unsigned int readLong (unsigned char buffer[], int offset)
{
   return readWord (buffer, offset) + 65536 * readWord (buffer, offset+2);
}

////////////////////////////////////////////////////////////////////////////////
// Load from file
// .bmp bitmaps are stored upside down and in bgr format for 24 bpp

#ifndef UseDevIL

bool Texture::loadFromFile (const char* FileName)
{
   deactivate();

   // Load entire file into memory for processing
   FileCache fc;
   if (!fc.openReadOnly(FileName)) {
      return false; 
   }
   size_t filesize = fc.getFileSize();
   unsigned char* fileBytes = new unsigned char[filesize];
   if (!fc.read(0, (char*) fileBytes, fc.getFileSize())) {
      delete fileBytes;
      return false;
   }
   fc.close();

   // Read Header
   texels = NULL;
   //const size_t headerLen = 54;
   bool LoadedSuccessfully = false;

   // Read rest of file and close
   if (fileBytes[0] == 'B' && fileBytes[1] == 'M') {
      size_t bmpWidth = readLong (fileBytes, 18);
      size_t bmpHeight = readLong (fileBytes, 22);
      size_t bpp = readLong(fileBytes, 28) / 8;
      if (bpp == 3)
         resize (bmpWidth, bmpHeight, GL_RGB);
      else
         resize (bmpWidth, bmpHeight, GL_RGBA);
      size_t picOffs = readLong (fileBytes, 14);
      //unsigned int picSize = readLong (fileBytes, 34);
      size_t lineLen = (texelComponents*bmpWidth+3) / 4 * 4;

      for (size_t y = 0; y < bmpHeight; y++)
         for (size_t x = 0; x < bmpWidth; x++) {
            size_t index = texelComponents*x+y*lineLen;
            unsigned char r = fileBytes[picOffs + index + 16];
            unsigned char g = fileBytes[picOffs + index + 15];
            unsigned char b = fileBytes[picOffs + index + 14];
            setRGB (x, y, r, g, b);
         }

      LoadedSuccessfully = true;
   }

   delete fileBytes;
   return LoadedSuccessfully;
}

#else

void logILError (const char* msg)
{
   ILenum error = ilGetError();
   if (error != IL_NO_ERROR) {
      printf (msg);
      switch (error) {
      case ILUT_COULD_NOT_OPEN_FILE:
         printf ("Could not open the file specified. The file may already be open by another app or may not exist.");
         break;
      case ILUT_ILLEGAL_OPERATION:
         printf ("The operation attempted is not allowable in the current state. The function returns with no ill side effects.");
         break;
      case ILUT_INVALID_ENUM:
         printf ("An unacceptable enumerated value was passed to a function.");
         break;
      case ILUT_INVALID_PARAM:
         printf ("An invalid parameter was passed to a function, such as a NULL pointer.");
         break;
      case ILUT_INVALID_VALUE:
         printf ("An invalid value was passed to a function or was in a file.");
         break;
      case ILUT_NOT_SUPPORTED:
         printf ("A type is valid but not supported in the current build.");
         break;
      case ILUT_OUT_OF_MEMORY:
         printf ("Could not allocate enough memory in an operation.");
         break;
      case ILUT_STACK_OVERFLOW:
         printf ("One of the internal stacks was already filled, and the user tried to add on to the full stack.");
         break;
      case ILUT_STACK_UNDERFLOW:
         printf ("One of the internal stacks was empty, and the user tried to empty the already empty stack.");
         break;
//      case ILU_ILLEGAL_OPERATION:
//         printf ("The operation attempted is not allowable in the current state. The function returns with no ill side effects.");
//         break;
      case ILU_INTERNAL_ERROR:
         printf ("A serious error has occurred. Please e-mail an admin with the conditions leading up to this error being reported.");
         break;
//      case ILU_INVALID_ENUM:
//         printf ("An unacceptable enumerated value was passed to a function.");
//         break;
//      case ILU_INVALID_PARAM:
//         printf ("An invalid parameter was passed to a function, such as a NULL pointer.");
//         break;
//      case ILU_INVALID_VALUE:
//         printf ("An invalid value was passed to a function or was in a file.");
//         break;
//      case ILU_OUT_OF_MEMORY:
//         printf ("Could not allocate enough memory in an operation.");
//         break;
//      case IL_COULD_NOT_OPEN_FILE:
//         printf ("Could not open the file specified. The file may already be open by another app or may not exist.");
         //break;
      case IL_FILE_ALREADY_EXISTS:
         printf ("The filename specified already belongs to another file. To overwrite files by default read more at ilEnable function.");
         break;
      case IL_FORMAT_NOT_SUPPORTED:
         printf ("The format a function tried to use was not able to be used by that function.");
         break;
      case IL_ILLEGAL_FILE_VALUE:
         printf ("An illegal value was found in a file trying to be loaded.");
         break;
//      case IL_ILLEGAL_OPERATION:
//         printf ("The operation attempted is not allowable in the current state. The function returns with no ill side effects.");
//         break;
//      case IL_INTERNAL_ERROR:
//         printf ("A serious error has occurred. Please e-mail an admin with the conditions leading up to this error being reported.");
//         break;
      case IL_INVALID_CONVERSION:
         printf ("An invalid conversion attempt was tried.");
         break;
//      case IL_INVALID_ENUM:
//         printf ("An unacceptable enumerated value was passed to a function.");
//         break;
      case IL_INVALID_EXTENSION:
         printf ("The extension of the specified filename was not correct for the type of image-loading function.");
         break;
      case IL_INVALID_FILE_HEADER:
         printf ("A file's header was incorrect.");
         break;
//      case IL_INVALID_PARAM:
//         printf ("An invalid parameter was passed to a function, such as a NULL pointer.");
//         break;
//      case IL_INVALID_VALUE:
//         printf ("An invalid value was passed to a function or was in a file.");
//         break;
      case IL_LIB_JPEG_ERROR:
         printf ("An error occurred in the libjpeg library.");
         break;
      case IL_LIB_PNG_ERROR:
         printf ("An error occurred in the libpng library.");
         break;
      case IL_NO_ERROR:
         printf ("No detectable error has occured.");
         break;
      case IL_OUT_FORMAT_SAME:
         printf ("Tried to convert an image from its format to the same format.");
         break;
//      case IL_OUT_OF_MEMORY:
//         printf ("Could not allocate enough memory in an operation.");
//         break;
//      case IL_STACK_OVERFLOW:
//         printf ("One of the internal stacks was already filled, and the user tried to add on to the full stack.");
//         break;
//      case IL_STACK_UNDERFLOW:
//         printf ("One of the internal stacks was empty, and the user tried to empty the already empty stack.");
//         break;
      case IL_UNKNOWN_ERROR:
         printf ("No function sets this yet, but it is possible (not probable) it may be used in the future.");
      }
   }
}

TCHAR lowerCase (TCHAR a)
{
   if (a > 'A' && a < 'Z')
      return a-'A'+'a';
   else
      return a;
}

int getILType (const TCHAR* FileName, bool& upsideDown)
{
   size_t len = bgstrlen (FileName);
   upsideDown = false;
   //size_t len = strlen (FileName);
   if (len >= 4) {
      if (lowerCase (FileName[len-3]) == 'b'
      &&  lowerCase (FileName[len-2]) == 'm'
      &&  lowerCase (FileName[len-1]) == 'p') {
         upsideDown = true;
         return IL_BMP;
      } else
      if (lowerCase (FileName[len-3]) == 'p'
      &&  lowerCase (FileName[len-2]) == 'n'
      &&  lowerCase (FileName[len-1]) == 'g')
         return IL_PNG;
      else
      if (lowerCase (FileName[len-3]) == 'j'
      &&  lowerCase (FileName[len-2]) == 'p'
      &&  lowerCase (FileName[len-1]) == 'g')
         return IL_JPG;
      else
      if (lowerCase (FileName[len-3]) == 't'
      &&  lowerCase (FileName[len-2]) == 'i'
      &&  lowerCase (FileName[len-1]) == 'f')
         return IL_TIF;
      else
      if (lowerCase (FileName[len-3]) == 'g'
      &&  lowerCase (FileName[len-2]) == 'i'
      &&  lowerCase (FileName[len-1]) == 'f')
         return IL_GIF;
   }

   return IL_BMP;
}

bool Texture::loadFromFile (const TCHAR* FileName)
{
   ILuint ilHandle;
   ilGenImages (1, &ilHandle);
   logILError ("ilGenImages");
   ilBindImage (ilHandle);
   logILError ("Bind error: ");

   //ilLoadImage (FileName);

   FileCache fc;
   fc.open (FileName);
   size_t size = fc.getFileSize();
   char* chars = new char[size];
   fc.read(0, chars, size);
   fc.close();
   bool upsideDown;
   int type = getILType (FileName, upsideDown);
   ilLoadL (type, chars, size);
   logILError ("Load error: ");

   ilConvertImage (IL_BGR, IL_UNSIGNED_BYTE);
   logILError ("Conversion error: ");
   //ilConvertPal (IL_PAL_RGB24);
   //logILError ("Palette conversion error: ");
   int w = ilGetInteger (IL_IMAGE_WIDTH);
   int h = ilGetInteger (IL_IMAGE_HEIGHT);
   resize (w, h, GL_BGR);

   //memcpy (texels, ilGetData(), 3*w*h);
   unsigned char* data = ilGetData();
   if (upsideDown) {
      for (unsigned int y = 0; y < h; y++)
         for (unsigned int x = 0; x < w; x++) {
            size_t index = 3*(y*w+x);
            unsigned char r = data[index+2];
            unsigned char g = data[index+1];
            unsigned char b = data[index];
            setRGB (x, y, r, g, b);
         }
   } else {
      for (unsigned int y = 0; y < h; y++)
         for (unsigned int x = 0; x < w; x++) {
            size_t index = 3*(y*w+x);
            unsigned char r = data[index+2];
            unsigned char g = data[index+1];
            unsigned char b = data[index];
            setRGB (x, h-y, r, g, b);
         }
   }

   ilDeleteImages (1, &ilHandle);
   logILError ("Cleanup error: ");
   return true;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Capture screen to BMP

inline void WriteDWord (unsigned char* buffer, int index, unsigned int value)
{
   for (unsigned int i = 0; i < 4; i++)
      buffer[index+i] = (value >> (i*8) & 255);
}

bool Texture::saveToFile (const char* FileName, size_t left, size_t top, 
                          size_t saveWidth, size_t saveHeight)
{
   if (left+saveWidth > width
   ||  top + saveHeight > height)
   {
      return false;
   }

   // Texture save support must copy pot-texture buffer to separate buffer of
   // fitting size
   bool result = true;
   const unsigned int HeaderSize = 54;
   size_t LineLen = 3*saveWidth;
   int map[] = {0, 3, 2, 1};
   size_t LineEndOverhead = map[LineLen & 3];
   LineLen += LineEndOverhead;
   size_t BufferSize = LineLen*saveHeight;
   size_t TotalFileSize = HeaderSize + BufferSize;
   unsigned char* header = new unsigned char [HeaderSize];
   unsigned char* buffer = new unsigned char [BufferSize];

   // Set header data
   for (size_t i = 0; i < HeaderSize; i++)
      header [i] = 0;
   header [ 0] = 'B'; // Magic number
   header [ 1] = 'M'; // Magic number
   WriteDWord (header, 2, (unsigned int) TotalFileSize);
   WriteDWord (header, 10, (unsigned int) HeaderSize);
   header [10] = HeaderSize; // Offset to data
   header [14] = 40; // Size of info header
   WriteDWord (header, 18, (unsigned int) saveWidth);
   WriteDWord (header, 22, (unsigned int) saveHeight);
   header [26] = 1;  // Number of bit planes
   header [28] = 24; // Bits per pixel
   WriteDWord (header, 34, (unsigned int) BufferSize);

   // Set buffer content
   size_t offset = 0;
   for (int y = 0; y < (signed) saveHeight; y++) {
      for (int x = 0; x < (signed) saveWidth; x++) {
         unsigned char r, g, b;
         getRGB (left + x, top + y, r, g, b);
         buffer [offset] = b; ++offset;
         buffer [offset] = g; ++offset;
         buffer [offset] = r; ++offset;
      }
      offset += LineEndOverhead;
   }

   // Write file
   FILE* fp = fopen(FileName, "wb");
   if (fp != NULL) {
      size_t headerWritten = fwrite(header, 1, HeaderSize, fp);

      if (headerWritten == HeaderSize) {
         size_t dataWritten = fwrite(buffer, 1, BufferSize, fp);
         if (dataWritten != BufferSize) {
            printf ("Failed to write bmp data!");
            result = false;
         }
      } else {
         printf ("Failed to write bmp header!");
         result = false;
      }
      fclose(fp);
   }

   delete header;
   delete buffer;

   return result;
}

///////////////////////////////////////////////////////////////////////////////
// Prepare texture for rendering (use activate() first)

Texture* activeTexture;

void Texture::use () 
{
   if (!active) {
      // Activate texture in OpenGL
      glGenTextures (1, &GLHandle);
      glBindTexture (GL_TEXTURE_2D, GLHandle);
      glTexImage2D (GL_TEXTURE_2D, 0, texelComponents, 
                    (unsigned int) potWidth, (unsigned int) potHeight, 
                    0, format, GL_UNSIGNED_BYTE, getPicData ());

      // Set texture parameters for that texture
      glTexEnvi (GL_TEXTURE, GL_TEXTURE_ENV_MODE, GL_BLEND);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      active = true;
   } else if (activeTexture != this) {
      // Activate texture by handle
      glBindTexture (GL_TEXTURE_2D, GLHandle);
      activeTexture = this;
   }
}

void Texture::useMipmap ()
{
   if (!active) {
      // Activate texture in OpenGL
      glGenTextures (1, &GLHandle);
      glBindTexture (GL_TEXTURE_2D, GLHandle);
      gluBuild2DMipmaps (GL_TEXTURE_2D, texelComponents, 
         (unsigned int) potWidth, (unsigned int) potHeight, 
         format, GL_UNSIGNED_BYTE, getPicData ());

      // Set texture parameters for that texture
      glTexEnvi (GL_TEXTURE, GL_TEXTURE_ENV_MODE, GL_BLEND);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
      active = true;
   } else if (activeTexture != this) {
      // Activate texture by handle
      glBindTexture (GL_TEXTURE_2D, GLHandle);
      activeTexture = this;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Deactivate texture

void Texture::deactivate()
{
   if (GLHandle != 0)
      glDeleteTextures(1, &GLHandle);
   if (active)
      glDeleteTextures (1, &GLHandle);
   GLHandle = 0;
   active = false;
}

////////////////////////////////////////////////////////////////////////////////
// Resize texture (content becomes undefined)

bool Texture::resize (size_t sizex, size_t sizey, GLenum _format)
{
   deactivate();

   try {
      // Free old data
      if (texels != NULL) {
         delete texels;
         texels = NULL;
      }

      // Determine data format
      format = _format;
      switch (_format) {
         case GL_RGB:  texelComponents = 3;
                       redOffset = 0;
                       greenOffset = 1;
                       blueOffset = 2;
                       alphaOffset = 0;
                       break;
         case GL_RGBA: texelComponents = 4;
                       redOffset = 0;
                       greenOffset = 1;
                       blueOffset = 2;
                       alphaOffset = 3;
                       break;
         case GL_BGR:  texelComponents = 3;
                       redOffset = 2;
                       greenOffset = 1;
                       blueOffset = 0;
                       alphaOffset = 0;
                       break;
      }

      // Determine power-of-two texture sizes
      if (sizex > 0) {
         potWidth = 1;
         while (potWidth < sizex)
            potWidth += potWidth;
      } else
         potWidth = 0;

      if (sizey > 0) {
         potHeight = 1;
         while (potHeight < sizey)
            potHeight += potHeight;
      } else
         potHeight = 0;

      // Allocate buffer
      if (potWidth > 0
      &&  potHeight > 0)
      {
         texels = new unsigned char [potHeight*potWidth*texelComponents];
      }
      width = sizex;
      height = sizey;
      return true;
   } 
   catch (...) {
      printf ("Texture resizeing failed\n");
      return false;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Read a texture from framebuffer at the given coordinates
// Parameters used to be x1, y1, x2, y2 and the texture was resized 
// automatically. This has been removed to prevent resizing the texture twice,
// once at creation and once at reading from frame buffer.

void Texture::readFromFrameBuffer (unsigned int /*x*/, unsigned int /*y*/)
{
   // Not currently supported!
   // glReadBuffer (GL_FRONT);
   // glReadPixels (x, y, width, height, GL_BGR, GL_UNSIGNED_BYTE, picData);
}

////////////////////////////////////////////////////////////////////////////////
// Copy part of another texture into this one

void Texture::copy (Texture* other, size_t x1, size_t y1, size_t x2, size_t y2, 
                    size_t tx, size_t ty)
{
   for (size_t x = x1; x < x2; x++)
      for (size_t y = y1; y < y2; y++) {
         unsigned char r, g, b;
         other->getRGB (x, y, r, g, b);
         setRGB (x-x1+tx, y-y1+ty, r, g, b);
      }
}

////////////////////////////////////////////////////////////////////////////////

void Texture::setName (const char* newName)
{ 
   if (name != NULL)
   delete name;
   size_t newNameLen = strlen (newName)+1;
   name = new char [newNameLen];
   memcpy (name, newName, newNameLen); 
}

///////////////////////////////////////////////////////////////////////////////
// Render a line (includes line endpoint)

#ifdef TextureRasterizers

void Texture::renderLine (int x1, int y1, int x2, int y2, 
                 unsigned char r, unsigned char g, unsigned char b)
{
   if (x1 == x2) {
      if (y1 < y2)
         for (int y = y1; y <= y2; y++)
            setRGB(x1, y, r, g, b);
      else 
         for (int y = y2; y <= y1; y++)
            setRGB(x1, y, r, g, b);
   } else {
      if (y1 == y2) {
         if (x1 < x2)
            for (int x = x1; x <= x2; x++)
               setRGB(x, y1, r, g, b);
         else
            for (int x = x2; x <= x1; x++)
               setRGB(x, y1, r, g, b);
      } else {
         if (abs (x2-x1) < abs (y2-y1)) {
            if (y1 < y2) {
               for (int y = y1; y <= y2; y++) {
                  int x = x1 + (x2-x1) * (y-y1) / (y2-y1);
                  setRGB (x, y, r, g, b);
               }
            } else {
               for (int y = y2; y <= y1; y++) {
                  int x = x2 + (x1-x2) * (y-y2) / (y1-y2);
                  setRGB (x, y, r, g, b);
               }
            }
         } else {
            if (x1 < x2) {
               for (int x = x1; x <= x2; x++) {
                  int y = y1 + (y2-y1) * (x-x1) / (x2-x1);
                  setRGB (x, y, r, g, b);
               }
            } else {
               for (int x = x2; x <= x1; x++) {
                  int y = y2 + (y1-y2) * (x-x2) / (x1-x2);
                  setRGB (x, y, r, g, b);
               }
            }
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Fill triangle

void Texture::fillTriangle (int x1, int y1, int x2, int y2, int x3, int y3,
                            unsigned char r, unsigned char g, unsigned char b)
{
   int x[3], y[3];

   // Sort the points so that y[0] < y[1] < y[2]
   if (y1 < y2) {
      if (y1 < y3) {
         if (y2 < y3) {
            x[0] = x1; x[1] = x2; x[2] = x3; y[0] = y1; y[1] = y2; y[2] = y3; // 1 2 3
         } else {
            x[0] = x1; x[1] = x3; x[2] = x2; y[0] = y1; y[1] = y3; y[2] = y2; // 1 3 2
         }
      } else {
         x[0] = x3; x[1] = x1; x[2] = x2; y[0] = y3; y[1] = y1; y[2] = y2; // 3 1 2
      }
   } else {
      if (y1 < y3) {
         x[0] = x2; x[1] = x1; x[2] = x3; y[0] = y2; y[1] = y1; y[2] = y3; // 2 1 3
      } else {
         if (y2 < y3) {
            x[0] = x2; x[1] = x3; x[2] = x1; y[0] = y2; y[1] = y3; y[2] = y1; // 2 3 1
         } else {
            x[0] = x3; x[1] = x2; x[2] = x1; y[0] = y3; y[1] = y2; y[2] = y1; // 3 2 1
         }
      }
   }

   // Render upper half
   for (int yc = max (y[0], 0); yc < mymin (y[1], (int) height-1); yc++) {
      int lxa = x[0] + (x[1]-x[0]) * (yc-y[0]) / (y[1]-y[0]);
      int lxb = x[0] + (x[2]-x[0]) * (yc-y[0]) / (y[2]-y[0]);
      int lx1, lx2;
      if (lxa < lxb) {
         lx1 = putInRange (0, lxa, (int) width-1);
         lx2 = putInRange (0, lxb, (int) width-1);
      } else {
         lx1 = putInRange (0, lxb, (int) width-1);
         lx2 = putInRange (0, lxa, (int) width-1);
      }
      for (int xc = lx1; xc < lx2; xc++) 
         setRGB (xc, yc, r, g, b);
   }

   // Render lower half
   for (int yc = max (y[1], 0); yc < mymin (y[2], (int) height-1); yc++) {
      int lxa = x[1] + (x[2]-x[1]) * (yc-y[1]) / (y[2]-y[1]); // 2 1
      int lxb = x[0] + (x[2]-x[0]) * (yc-y[0]) / (y[2]-y[0]); // 0 2
      int lx1, lx2;
      if (lxa < lxb) {
         lx1 = putInRange (0, lxa, (int) width-1);
         lx2 = putInRange (0, lxb, (int) width-1);
      } else {
         lx1 = putInRange (0, lxb, (int) width-1);
         lx2 = putInRange (0, lxa, (int) width-1);
      }
      for (int xc = lx1; xc < lx2; xc++) 
         setRGB (xc, yc, r, g, b);
   }
}

///////////////////////////////////////////////////////////////////////////////
// Fill star-shaped polygon given as a vector

void Texture::fillPolygon (const vector <int>& polygon, unsigned char r,
                           unsigned char g, unsigned char b, int cx, int cy)
{
   size_t size = polygon.size();
   for (size_t i = 0; i < size-2; i+= 2)
      fillTriangle (polygon[i], polygon[i+1], polygon[i+2], polygon[i+3], cx, cy, r, g, b);
   fillTriangle (polygon[size-2], polygon[size-1], polygon[0], polygon[1], cx, cy, r, g, b);

/*   size_t size = polygon.size();
   for (size_t i = 0; i < size-4; i+= 2) {
      renderLine (polygon[i], polygon[i+1], polygon[i+2], polygon[i+3], r, g, b);
      renderLine (polygon[i], polygon[i+1], sx, sy, r, g, b);
   }
   renderLine (polygon[size-2], polygon[size-1], polygon[0], polygon[1], r, g, b);
   renderLine (polygon[size-2], polygon[size-1], sx, sy, r, g, b);*/
}

///////////////////////////////////////////////////////////////////////////////
// Fill square

void Texture::fillSquare (int x1, int y1, int x2, int y2, unsigned char r,
                           unsigned char g, unsigned char b)
{
   for (int x = mymax (0, x1); x < mymin ((int) width-1, x2); x++)
      for (int y = mymax (0, y1); y < mymin ((int) height-1, y2); y++)
         setRGB (x, y, r, g, b);
}


#endif