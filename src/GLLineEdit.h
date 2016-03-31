////////////////////////////////////////////////////////////////////////////////
// GLLineEdit.h
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef GLLineEdit__H
#define GLLineEdit__H

#include "GLWindow.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLLineEdit: public GLWindow {
public:
   Color textColor;

   // Constructor
   GLLineEdit (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0);

   // Destructor
   virtual ~GLLineEdit();

   // Text access
   inline const TCHAR* getText () const
   { return text.getChars(); }
   inline void setText (const char* newText)
   { text = newText; }

   // Set text to numerical values
   void setDouble (double newVal)
   { text.assignDouble (newVal); }
   void setInt (int newVal)
   { text.assignInt (newVal); }

   // Try to interpret text as numerical values
   bool getDouble (double& value) const;
   bool getInt (int& value) const;

   // Render
   virtual void render(int x, int y, int w, int h);

   // Keyboard events
   virtual void keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down);

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int button, bool down, 
                                         int /*x*/, int /*y*/);

   virtual void mouseMoved (int /*x*/, int /*y*/);

   // Event: widget looses focus
   virtual void loseFocus ();

protected:
   BGString text;
   int cursorPos;
};

#endif
