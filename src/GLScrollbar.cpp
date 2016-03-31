////////////////////////////////////////////////////////////////////////////////
// GLScrollBar.cpp
// Copyright Bjoern Ganster 2006-2009
////////////////////////////////////////////////////////////////////////////////

#include "GLScrollbar.h"
#include "FramerateCounter.h" // for linux version of GetTickCount

int minScrollBarSpace = 6;

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLScrollbar::GLScrollbar (GLWindow* _parent)
: GLWindow (_parent)
{
   horizontal = false;
   start = 0;
   size = 10;
   min = 0;
   max = 100;
   moved = false;
   buttonSpeed = 10;
   movement = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLScrollbar::~GLScrollbar()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLScrollbar::render(int x, int y, int w, int h)
{
   GLWindow::prepareToRender(x, y, w, h);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   glColor3d (1.0, 1.0, 1.0);
   glLoadIdentity ();
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);

   if (horizontal) {
      if (width > 2 * height) {
         renderHorizontalButtonsAndScrollbar ();
      } else if (height > minScrollBarSpace) {
         renderHorizontalButtonsOnly();
      }
   } else {
      if (height > 2 * width) {
         renderVerticalButtonsAndScrollbar ();
      } else if (height > minScrollBarSpace) {
         renderVerticalButtonsOnly();
      }
   }
}

void GLScrollbar::renderHorizontalButtonsOnly()
{
   // Display two buttons only, starting with background
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (0, 0);
      glVertex2i (width, 0);
      glVertex2i (width, height);
      glVertex2i (0, height);
   glEnd ();

   // Render button frames
   int half = width/2;
   renderShadedRectangle (0, 0, half, height);
   renderShadedRectangle (half+1, 0, width, height);

   // Render button arrows
   glColor3i (0,0,0);
   glBegin (GL_POLYGON);
      glVertex2d (height/3, height/2);
      glVertex2d (2*height/3, 4*height/5);
      glVertex2d (2*height/3, height/5);
   glEnd ();
   glBegin (GL_POLYGON);
      glVertex2d (half+2*height/3, height/2);
      glVertex2d (half+height/3, height/5);
      glVertex2d (half+height/3, 4*height/5);
   glEnd ();
}

void GLScrollbar::renderHorizontalButtonsAndScrollbar ()
{
// Display two buttons only, starting with background
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (0, 0);
      glVertex2i (width, 0);
      glVertex2i (width, height);
      glVertex2i (0, height);
   glEnd ();

   // Render button frames
   renderShadedRectangle (0, 0, height, height);
   renderShadedRectangle (0, width-height, width, height);

   // Render button arrows
   glColor3i (0,0,0);
   glBegin (GL_POLYGON);
      glVertex2d (height/3, height/2);
      glVertex2d (2*height/3, height/5);
      glVertex2d (2*height/3, 4*height/5);
   glEnd ();
   int base = width-height;
   glBegin (GL_POLYGON);
      glVertex2d (base+2*height/3, height/2);
      glVertex2d (base+height/3, height/5);
      glVertex2d (base+height/3, 4*height/5);
   glEnd ();

   // Render scrollbar
   glColor3d (0.25, 0.25, 0.25);
   glBegin (GL_POLYGON);
      glVertex2i (height, 0);
      glVertex2i (width-height, 0);
      glVertex2i (width-height, height);
      glVertex2i (height, height);
   glEnd ();
   int s1 = height + (width-2*height) * getStart () / max;
   int s2 = height + (width-2*height) * (getStart () + size) / max;
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (s1, 0);
      glVertex2i (s2, 0);
      glVertex2i (s2, height);
      glVertex2i (s1, height);
   glEnd ();
   renderShadedRectangle (s1, 0, s2, height);
}

void GLScrollbar::renderVerticalButtonsOnly()
{
   // Display two buttons only, starting with background
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (0, 0);
      glVertex2i (width, 0);
      glVertex2i (width, height);
      glVertex2i (0, height);
   glEnd ();

   // Render button frames
   int half = height/2;
   renderShadedRectangle (0, 0, width, half);
   renderShadedRectangle (0, half+1, width, height);

   // Render button arrows
   glColor3i (0,0,0);
   glBegin (GL_POLYGON);
      glVertex2d (width/2, half/3);
      glVertex2d (4*width/5, 2*half/3);
      glVertex2d (width/5, 2*half/3);
   glEnd ();
   glBegin (GL_POLYGON);
      glVertex2d (width/2, half+2*half/3);
      glVertex2d (4*width/5, half+half/3);
      glVertex2d (width/5, half+half/3);
   glEnd ();
}

void GLScrollbar::renderVerticalButtonsAndScrollbar ()
{
   // Display two buttons only, starting with background
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (0, 0);
      glVertex2i (width, 0);
      glVertex2i (width, height);
      glVertex2i (0, height);
   glEnd ();

   // Render button frames
   renderShadedRectangle (0, 0, width, width);
   renderShadedRectangle (0, height-width, width, height);

   // Render button arrows
   glColor3i (0,0,0);
   glBegin (GL_POLYGON);
      glVertex2d (width/2, width/3);
      glVertex2d (4*width/5, 2*width/3);
      glVertex2d (width/5, 2*width/3);
   glEnd ();
   int base = height-width;
   glBegin (GL_POLYGON);
      glVertex2d (width/2, base+2*width/3);
      glVertex2d (4*width/5, base+width/3);
      glVertex2d (width/5, base+width/3);
   glEnd ();

   // Render scrollbar
   glColor3d (0.25, 0.25, 0.25);
   glBegin (GL_POLYGON);
      glVertex2i (0, width);
      glVertex2i (width, width);
      glVertex2i (width, height-width);
      glVertex2i (0, height-width);
   glEnd ();
   int s1 = width + (height - 2 * width) * getStart () / max;
   int s2 = width + (height - 2 * width) * (getStart () + size) / max;
   glColor3d (0.5, 0.5, 0.5);
   glBegin (GL_POLYGON);
      glVertex2i (0, s1);
      glVertex2i (width, s1);
      glVertex2i (width, s2);
      glVertex2i (0, s2);
   glEnd ();
   renderShadedRectangle (0, s1, width, s2);
}

////////////////////////////////////////////////////////////////////////////////
// Events

void GLScrollbar::keypressed (BGKey /*key*/, bool /*shiftHeld*/, 
                              bool /*ctrlHeld*/, bool down)
{
}

GLWindow* GLScrollbar::buttonStateChanged (int button, bool down, int x, int y)
{
   x = x - left;
   y = y - top;

   if (button == 1 && down) {
      if (horizontal) {
         int s1 = height + (width-2*height) * getStart () / max;
         int s2 = height + (width-2*height) * (getStart () + size) / max;
         if (x < s1) {
            movement = -1;
         } else if (x > s2) {
            movement = 1;
         } else {
            movement = 0;
            moved = true;
            moveStart = x;
            dragStart = start;
         }
      } else {
         int s = getStart();
         int s1 = width + (height - 2 * width) * s / max;
         int s2 = width + (height - 2 * width) * (s + size) / max;
         if (y < s1) {
            movement = -1;
         } else if (y > s2) {
            movement = 1;
         } else {
            movement = 0;
            moved = true;
            moveStart = y;
            dragStart = start;
         }
      }
      lastClick = GetTickCount ();
   } else if (button == 1 && !down) {
      moved = false;
      movement = 0;
   }

   // Scrollbars remain focused while mouse is held
   if (moved)
      return this;
   else
      return NULL;
}

void GLScrollbar::mouseMoved (int x, int y)
{
   x = x -left;
   y = y - top;

   if (moved) {
      if (horizontal) 
         start = dragStart + (max-min) * (x - moveStart) / (width - 2 * height);
      else
         start = dragStart + (max-min) * (y - moveStart) / (height - 2 * width);

      if (start < min)
         start = min;
      if (start > max-size)
         start = max-size;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Gets the current start of the scrollbar

int GLScrollbar::getStart ()
{
   if (movement != 0) {
      int ticks = GetTickCount ();
      if (ticks != lastClick) {
         start = start + movement * ((int) (buttonSpeed * (ticks - lastClick) / 100.0));

         lastClick = ticks;
      }

      if (start < min)
         start = min;
      if (start > max-size)
         start = max-size;
   }

   return start; 
}
