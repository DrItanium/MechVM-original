////////////////////////////////////////////////////////////////////////////////
// GLSplitter.cpp
// Copyright Björn Ganster 2006
////////////////////////////////////////////////////////////////////////////////

#include "GLSplitter.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for splitter

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLSplitter::GLSplitter (GLWindow* _parent, int _left, int _top, 
          int _width, int _height)
: GLWindow (_parent, _left, _top, _width, _height)
{
   horizontal = false;
   moved = false;
   beforeWin = NULL;
   afterWin = NULL;
   aspect = 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLSplitter::~GLSplitter()
{
   if (beforeWin != NULL)
      delete beforeWin;
   if (afterWin != NULL)
      delete afterWin;
}

////////////////////////////////////////////////////////////////////////////////

void GLSplitter::setAspect (double newVal)
{ 
   aspect = newVal; 
   updateGeometry ();
}

////////////////////////////////////////////////////////////////////////////////
// Update child after change to geometry or aspect ratio

void GLSplitter::updateGeometry ()
{ 
   // Place child widget according to aspect ratio
  if (beforeWin != NULL && afterWin == NULL) {
      beforeWin->setGeometry (0, 0, width, height);
   } else if (beforeWin == NULL && afterWin != NULL) {
      afterWin->setGeometry (0, 0, width, height);
   } else if (beforeWin != NULL && afterWin != NULL) {
      if (horizontal) {
         int w1 = ((int) (aspect * (width - breadth)));
         int w2 = ((int) ((1.0-aspect) * (width - breadth)));
         beforeWin->setGeometry (0, 0, w1, height);
         afterWin->setGeometry (w1 + breadth, 0, w2, height);
      } else {
         int h1 = ((int) (aspect * (height - breadth)));
         int h2 = ((int) ((1.0-aspect) * (height - breadth)));
         beforeWin->setGeometry (0, 0, width, h1);
         afterWin->setGeometry (0, h1 + breadth, width, h2);
      }
   }
}

////////////////////////////////////////////////////////////////////////////////

void GLSplitter::setGeometry (int _left, int _top, int _width, int _height)
{
   // Set new geometry
   GLWindow::setGeometry(_left, _top, _width, _height);
   updateGeometry ();
}

////////////////////////////////////////////////////////////////////////////////

GLWindow* GLSplitter::buttonStateChanged (int button, bool down, int x, int y)
{
   GLWindow* clicked = NULL;
   int splitCoord;
   x -= left;
   y -= top;

   if (horizontal)
      splitCoord = ((int) (left + aspect * (width-breadth)));
   else
      splitCoord = ((int) (top + aspect * (height-breadth)));

   if (( horizontal && x < splitCoord)
   ||  (!horizontal && y < splitCoord) )
   {
      clicked = beforeWin;
   }

   if (( horizontal && x > splitCoord + breadth)
   ||  (!horizontal && y > splitCoord + breadth) )
   {
      clicked = afterWin;
   }

   if (clicked != NULL) {
      return clicked->buttonStateChanged (button, down, x, y);
   } else if (button == 1) {
      if (down) 
      {
         // Drag start
         moved = true;
         if (horizontal)
            moveStart = x;
         else
            moveStart = y;
         clicked = this;
      } else {
         // Drag end
         moved = false;
      }      
   }

   return clicked;
}

////////////////////////////////////////////////////////////////////////////////

void GLSplitter::mouseMoved (int x, int y)
{
   if (moved) {
      if (horizontal) {
         if (x < left) 
            x = left;
         if (x >= left + width)
            x = left+width-1;
         setAspect (((double) (x-left)) / ((double) (width)));
/*         if (beforeWin != NULL)
            beforeWin->setWidth (x-beforeWin->getLeft ());
         if (afterWin != NULL) {
            afterWin->setLeft (x + breadth);
            afterWin->setWidth (width - x - breadth);
         }*/
      } else {
         if (y < top) 
            x = top;
         if (x >= top + height)
            x = top + height-1;
         setAspect (((double) (y-top)) / ((double) (height)));
/*         if (beforeWin != NULL)
            beforeWin->setHeight (y-beforeWin->getTop ());
         if (afterWin != NULL) {
            afterWin->setTop (y + breadth);
            afterWin->setHeight (height - y - breadth);
         }*/
      }
   } else {
      if (beforeWin != NULL) {
         if (x >= beforeWin->getLeft()
         &&  x <= beforeWin->getLeft() + beforeWin->getWidth()
         &&  y >= beforeWin->getTop ()
         &&  y <= beforeWin->getTop () + beforeWin->getHeight())
         {
            beforeWin->mouseMoved (x, y);
         }
      }
      if (afterWin != NULL) {
         if (x >= afterWin->getLeft()
         &&  x <= afterWin->getLeft() + afterWin->getWidth()
         &&  y >= afterWin->getTop ()
         &&  y <= afterWin->getTop () + afterWin->getHeight())
         {
            afterWin->mouseMoved (x, y);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLSplitter::render(int x, int y, int w, int h)
{
   //GLWindow::render();
   w = bgmin (w - left, width);
   h = bgmin (h - top, height);
   if (beforeWin != NULL) 
      beforeWin->render(x+left, y+top, w, h);
   if (afterWin != NULL) 
      afterWin->render(x+left, y+top, w, h);
}
