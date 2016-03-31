////////////////////////////////////////////////////////////////////////////////
// GLLabel.cpp
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "GLLabel.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLLabel::GLLabel (GLWindow* _parent, int _left, int _top, int _width, 
                  int _height)
: GLWindow (_parent, _left, _top, _width, _height),
  textColor (0, 0, 0)
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLLabel::~GLLabel()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLLabel::render(int x, int y, int w, int h)
{
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();

   setViewport (x, y, w, h);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   textColor.glColor();
   //glLoadIdentity();
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);
   renderText (getTitle(), 0, 0, width);
}
