////////////////////////////////////////////////////////////////////////////////
// GLWindowContainer.h
// Copyright Bjoern Ganster 2006
////////////////////////////////////////////////////////////////////////////////

#include "GLWindowContainer.h"

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLWindowContainer::~GLWindowContainer()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLWindowContainer::render(int x, int y, int w, int h)
{
   size_t i;
   for (i = 0; i < windows.size(); i++) {
      GLWindow* win = windows[i];
      int ww = bgmin (width, w - left);
      int wh = bgmin (height, h - top);
      win->render (x+left, y+top, ww, wh);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Events

GLWindow*  GLWindowContainer::buttonStateChanged (int button, bool down, 
                                                  int x, int y)
{
   size_t i;
   GLWindow* clicked = NULL;

   // Transform click coordinates to container's local coordinate system
   x -= left;
   y -= top;

   // Find clicked window
   for (i = 0; i < windows.size(); i++) {
      GLWindow* win = windows[i];
      if (win->getLeft () < x
      &&  x < win->getLeft () + win->getWidth ()
      &&  win->getTop () < y
      &&  y < win->getTop () + win->getHeight ())
      {
         clicked = win;
      }
   }

   if (clicked != NULL)
      return clicked->buttonStateChanged (button, down, x, y);
   else
      return NULL;
}

void GLWindowContainer::mouseMoved (int /*x*/, int /*y*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Remove children

int GLWindowContainer::remove (GLWindow* win)
{
   int deleted = 0;

   vector <GLWindow*>::iterator iter = windows.begin();
   while (iter != windows.end()) 
   {
      GLWindow* other = (*iter);
      if (other == win) {
         iter = windows.erase (iter);
         deleted++;
      } else 
         iter++;
   }

   //if (focusedWidget == win)
   //   focusedWidget = NULL;

   return deleted;
}
