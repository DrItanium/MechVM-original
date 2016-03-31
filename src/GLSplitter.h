////////////////////////////////////////////////////////////////////////////////
// GLSplitter.h
// Copyright Björn Ganster 2006
////////////////////////////////////////////////////////////////////////////////

#ifndef GLSplitter__H
#define GLSplitter__H

#include "GLWindow.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for splitter

class GLSplitter: public GLWindow {
public:
   // Constructor
   GLSplitter (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0);

   // Destructor
   virtual ~GLSplitter();

   // Render
   virtual void render(int x, int y, int w, int h);

   // Events
   virtual void keypressed (char /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/)
   {}
   virtual void specialKeyPressed (char /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/)
   {}

   virtual GLWindow* buttonStateChanged (int button, bool down, int x, int y);
   
   virtual void mouseMoved (int x, int y);

   virtual void setGeometry (int _left, int _top, int _width, int _height);

   // Member access
   inline void setHorizontal (bool newVal)
   { horizontal = newVal; }
   inline bool getHorizontal ()
   { return horizontal; }
   inline GLWindow* getBeforeWin ()
   { return beforeWin; }
   inline void setBeforeWin (GLWindow* newWin)
   { beforeWin = newWin; }
   inline GLWindow* getAfterWin ()
   { return afterWin; }
   inline void setAfterWin (GLWindow* newWin)
   { afterWin = newWin; }
   inline int getBreadth ()
   { return breadth; }
   inline void setBreadth (int newVal)
   { breadth = newVal; }
   void setAspect (double newVal);
   inline double getAspect ()
   { return aspect; }

protected:
   bool horizontal, // Determines whether this is a horizontal or vertical 
                    // splitter
        moved;      // True while user drags the scrollbar
   int moveStart,   // column for horizontal splitter, row for vertical 
                    // splitter. Stores start of movement.
       breadth;     // breadth of splitter
   GLWindow* beforeWin, * afterWin;
   double aspect;

   // Update child after change to geometry or aspect ratio
   void updateGeometry ();
};

#endif
