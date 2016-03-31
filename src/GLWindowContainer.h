////////////////////////////////////////////////////////////////////////////////
// GLWindowContainer.h
// Copyright Bj�rn Ganster 2006
////////////////////////////////////////////////////////////////////////////////

#ifndef GLWindowContainer__H
#define GLWindowContainer__H

#include "GLWindow.h"
#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Base class for container windows

class GLWindowContainer: public GLWindow {
public:
   // Constructor
   GLWindowContainer (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
                      int _width = 0, int _height = 0)
   : GLWindow (_parent, _left, _top, _width, _height)
   {}

   // Destructor
   virtual ~GLWindowContainer();

   // Render
   virtual void render(int x, int y, int w, int h);

   // Events
   virtual void keypressed (BGKey /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/, bool /*down*/)
   { }
   //virtual void specialKeyPressed (char /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/);
   virtual GLWindow* buttonStateChanged (int /*button*/, bool /*down*/, 
                                    int /*x*/, int /*y*/);
   virtual void mouseMoved (int /*x*/, int /*y*/);

   // Add/remove children
   virtual void add (GLWindow* win)
   { windows.push_back (win); }
   virtual int remove (GLWindow* win);
   inline void clear ()
   { windows.clear(); }
   virtual size_t getWindowCount () const
   { return windows.size(); }
   virtual GLWindow* getSubWin (size_t i)
   { return windows[i]; }
   
protected:
   vector <GLWindow*> windows;
};

#endif
