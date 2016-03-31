////////////////////////////////////////////////////////////////////////////////
// GLWindow.cpp
// Copyright Bjoern Ganster 2006
////////////////////////////////////////////////////////////////////////////////

#include "GLWindow.h"
#include "BGString.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////

SDL_Surface * s_screen;

void storeScreenSettings (SDL_Surface * screen)
{ 
   s_screen = screen;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLWindow::~GLWindow()
{ 
   if (parent != NULL)
      parent->remove (this); 
}

////////////////////////////////////////////////////////////////////////////////

GLWindow* GLWindow::getMainWindow()
{
   GLWindow* curr = this;
   while (curr->getParent() != NULL)
      curr = curr->getParent ();
   return curr;
}

////////////////////////////////////////////////////////////////////////////////

void GLWindow::setViewport (int x, int y, int w, int h)
{
   GLWindow* mainWin = getMainWindow();
   int rx = x+left;
   #ifdef WIN32
   int totalHeight = mainWin->getHeight ();
   int ry = totalHeight-y-top-height;
   #else
   int ry = s_screen->h-y-top-height;
   #endif
   glViewport (rx, ry, width, height);

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();
}

////////////////////////////////////////////////////////////////////////////////

void GLWindow::prepareToRender (int x, int y, int w, int h)
{
   // Get main window
   GLWindow* mainWin = getMainWindow();

   // Render frame
   int rx = x+left;
   #ifdef WIN32
   int totalHeight = mainWin->getHeight ();
   int ry = totalHeight-y-top-height;
   #else
   int ry = s_screen->h-y-top-height;
   #endif
   glViewport (rx-1, ry-1, width+1, height+1);

   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();

   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);

   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   glDepthMask (0); // Prevent writes to depth buffer
   //glClearColor(1.0,1.0,1.0,0.0); // Defines color for blending
   //glBlendColor4f (1, 1, 1, 1);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   renderShadedRectangle (1, 0, width-1, height-1);

   glBegin (GL_POLYGON);
      backgroundColor.glColor();
      glVertex2i (2, 1);
      glVertex2i (width-2, 1);
      glVertex2i (width-2, height-2);
      glVertex2i (2, height-2);
   glEnd();

   glViewport (x+left, ry, width, height);
   glDepthMask (~0); // Allow writes to depth buffer
   glLoadIdentity ();
   glDepthMask (~0);
   glDisable(GL_BLEND);
}

////////////////////////////////////////////////////////////////////////////////
// Render a shaded rectangle

void renderShadedRectangle (double x1, double y1, double x2, double y2)
{
   glBegin (GL_LINES);
      glColor3d (0.8, 0.8, 0.8);
      glVertex2d (x1, y1);
      glVertex2d (x1, y2);
      glVertex2d (x1, y1);
      glVertex2d (x2, y1);

      glColor3d (0.6, 0.6, 0.6);
      glVertex2d (x1, y2);
      glVertex2d (x2, y2);
      glVertex2d (x2, y1);
      glVertex2d (x2, y2);
   glEnd ();
}

////////////////////////////////////////////////////////////////////////////////

Texture* font;
size_t xs, ys;

bool loadFont (const TCHAR* fn, size_t _xs, size_t _ys)
{
   Texture* temp = new Texture();
   bool result = false;
   if (temp->loadFromFile(fn)) {
      font = new Texture(temp->getWidth(), temp->getHeight(), GL_RGBA);
      xs = _xs;
      ys = _ys;
      for (size_t x = 0; x < font->getWidth(); x++)
         for (size_t y = 0; y < font->getHeight(); y++) {
            unsigned char r, g, b, a;
            temp->getRGB(x, y, r, g, b);
            if (b > 100)
               a = 255;
            else
               a = 0;
            font->setRGBA(x, y, r, g, b, a);
         }

      result = true;
   }

   delete temp;
   return result;
}

////////////////////////////////////////////////////////////////////////////////
// Render text using freeglut

// Simple version without clamping
/*void renderText (const char* txt, int x, int y, int maxLen)
{
   size_t len = strlen (txt);
   glRasterPos2f(x, y+15);
   for (size_t i = 0; i < len; i++) {
      glutBitmapCharacter (GLUT_BITMAP_9_BY_15, txt[i]);
   }
}*/

const int GLUT_BITMAP_9_BY_15 = 9;
int glutBitmapWidth (int font, char c)
{ return GLUT_BITMAP_9_BY_15; }

// Print no more chars of txt than fit into maxWidth
void renderText (const char* txt, int x, int y, size_t maxWidth)
{
   // Check number of characters that fit into maxWidth
   size_t len = strlen (txt);
//   size_t count = bgmin(maxWidth/9, len);
   size_t pos = glutBitmapWidth (GLUT_BITMAP_9_BY_15, txt[0]);
   size_t count = 0;
   while (count < len && pos < maxWidth) {
      count++;
      pos += glutBitmapWidth (GLUT_BITMAP_9_BY_15, txt[count]);
   }

   // Render part of string
   //BGString str;
   //str.appendPart (txt, 0, count);
   /*pos = 0;
   glRasterPos2i(x, y+15);
   for (size_t i = 0; i < count; i++)
      glutBitmapCharacter (GLUT_BITMAP_9_BY_15, txt[i]);*/

   // Render part of string
   font->use();
   double y1 = y;
   double y2 = y+ys;
   size_t w = font->getWidth();
   size_t h = font->getHeight();

   glEnable(GL_TEXTURE_2D);
   glEnable(GL_ALPHA_TEST);
   glAlphaFunc(GL_GREATER, 0.5);
   glBegin(GL_QUADS);
   for (size_t i = 0; i < count; i++) {
      double x1 = x + i*xs;
      double x2 = x1+xs;
      char c = txt[i];
      int u1 = (c&15)*xs;
      int u2 = u1+xs;
      int v1 = (18-(c/16))*ys;
      int v2 = v1+ys;
      glTexCoord2d(double(u1)/w, double(v2)/h);
      glVertex2d(x1, y1);
      glTexCoord2d(double(u2)/w, double(v2)/h);
      glVertex2d(x2, y1);
      glTexCoord2d(double(u2)/w, double(v1)/h);
      glVertex2d(x2, y2);
      glTexCoord2d(double(u1)/w, double(v1)/h);
      glVertex2d(x1, y2);
   }
   glEnd();
   glDisable(GL_ALPHA_TEST);
   glDisable(GL_TEXTURE_2D);
}

////////////////////////////////////////////////////////////////////////////////
// Obtain length of text

int textWidth (const char* text)
{
   int width = 0;

   if (text != NULL) {
      size_t i = 0;

      while (text[i] != 0) {
         width += glutBitmapWidth (GLUT_BITMAP_9_BY_15, text[i]);
         i++;
      }
   }

   return width;
}

////////////////////////////////////////////////////////////////////////////////
// Render text using own fonts

/*void NodeEditor::renderText (const char* txt, int x, int y, int size)
{
   size_t len = strlen (txt);
   for (size_t i = 0; i < len; i++) {

      fontTexture->use();
      int cx = txt[i] % 16;
      int cy = txt[i] / 16;

      glBegin (GL_POLYGON);
         glTexCoord2d (cx*64/1024.0, 1.0-cy*64/1024.0);
         glVertex2d (x, y);

         glTexCoord2d ((cx*64+63)/1024.0, 1.0-cy*64/1024.0);
         glVertex2d (x+size-1, y);

         glTexCoord2d ((cx*64+63)/1024.0, 1.0-(cy*64+63)/1024.0);
         glVertex2d (x+size-1, y+size-1);

         glTexCoord2d (cx*64/1024.0, 1.0-(cy*64+63)/1024.0);
         glVertex2d (x, y+size-1);
      glEnd();

      x += size;
   }
}*/

////////////////////////////////////////////////////////////////////////////////
// Fill rectangle in current color

void fillRect(double x1, double y1, double x2, double y2)
{
   glBegin (GL_POLYGON);
      glVertex2d (x1, y1);
      glVertex2d (x2, y1);
      glVertex2d (x2, y2);
      glVertex2d (x1, y2);
   glEnd ();
}
