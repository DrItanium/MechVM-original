////////////////////////////////////////////////////////////////////////////////
// GLButton.h
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "GLButton.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLButton::GLButton (GLWindow* _parent, int _left, int _top, int _width, 
                    int _height)
: GLWindow (_parent, _left, _top, _width, _height)
{
   backgroundColor.r = 0.5;
   backgroundColor.g = 0.5;
   backgroundColor.b = 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLButton::~GLButton()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLButton::render(int x, int y, int w, int h)
{
   prepareToRender(x, y, w, h);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);

   glColor3d (0.0, 0.0, 0.0);

   int textLeft = (width - textWidth (getTitle())) / 2;
   int textTop = (height - 17) / 2;
   //glLoadIdentity();
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);
   renderText (getTitle(), textLeft, textTop, width);
}

////////////////////////////////////////////////////////////////////////////////
// Events

void GLButton::keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down)
{
}

////////////////////////////////////////////////////////////////////////////////
// Button state changed over a window: returns child that was clicked

GLWindow* GLButton::buttonStateChanged (int button, bool down, 
   int x, int y)
{ 
   if (button == 1 
   &&  !down
   &&  listener != NULL
   &&  x >= left
   &&  x < left+width
   &&  y >= top
   &&  y < top+height)
   {
      sendMessage (listener, ButtonClickedEvent);
   }
   return NULL; 
}

////////////////////////////////////////////////////////////////////////////////
// Event: widget looses focus

void GLButton::loseFocus ()
{
}
