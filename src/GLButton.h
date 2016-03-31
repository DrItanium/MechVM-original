////////////////////////////////////////////////////////////////////////////////
// GLButton.h
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef GLButton__H
#define GLButton__H

#include "GLWindow.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLButton: public GLWindow {
public:
   // Constructor
   GLButton (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0);

   // Destructor
   virtual ~GLButton();

   // Title access
   inline const TCHAR* getTitle () const
   { return m_title.getChars(); }
   inline void setTitle (const char* newTitle) 
   { m_title = newTitle; }
   inline void setTitle (int textNum)
   { m_title = tr(textNum); }

   // Render
   virtual void render(int x, int y, int w, int h);

   // Events
   virtual void keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down);

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, bool down, 
                                         int /*x*/, int /*y*/);
   
   virtual void mouseMoved (int /*x*/, int /*y*/)
   {}

   // Event: widget looses focus
   virtual void loseFocus ();

protected:
   BGString m_title;
};

#endif
