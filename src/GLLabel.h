////////////////////////////////////////////////////////////////////////////////
// GLLabel.h
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef GLLabel__H
#define GLLabel__H

#include "GLWindow.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLLabel: public GLWindow {
public:
   Color textColor;

   // Constructor
   GLLabel (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0);

   // Destructor
   virtual ~GLLabel();

   // Title access
   inline const TCHAR* getTitle () const
   { return title.getChars(); }
   inline void setTitle (const char* newTitle) 
   { title = newTitle; }
   inline void setTitle (int textNum)
   { title = tr(textNum); }
   inline void setInt (int newVal) 
   { title.assignInt(newVal); }

   // Render
   virtual void render(int x, int y, int w, int h);

   // Events
   virtual void keypressed (BGKey /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/, bool /*down*/)
   {}

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, int /*state*/, 
                                         int /*x*/, int /*y*/)
   { return NULL; }
   
   virtual void mouseMoved (int /*x*/, int /*y*/)
   {}

protected:
   BGString title;
};

#endif
