////////////////////////////////////////////////////////////////////////////////
// GLWindow.h
// Copyright Bjoern Ganster 2005-2006
////////////////////////////////////////////////////////////////////////////////

#ifndef GLWindow__H
#define GLWindow__H

#include "Color.h"
#include "BGString.h"
#include <SDL.h>

//typedef Uint16 BGKey;
typedef SDL_keysym BGKey;

////////////////////////////////////////////////////////////////////////////////
// Base class for a single windows

class GLWindow: public MessageListener {
public:
   int userData;

   // Constructor
   GLWindow (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0)
   : MessageListener (GLWindowID),
     userData (0),
     left (_left), top (_top), width (_width), height (_height),
     backgroundColor (0.0, 0.0, 0.0),
     parent (_parent),
     listener (NULL)
   {}

   // Destructor
   virtual ~GLWindow();

   // Render
   virtual void render(int x, int y, int w, int h) = 0;
   virtual void prepareToRender (int x, int y, int w, int h);
   virtual void setViewport (int x, int y, int w, int h);

   // Events
   virtual void receiveMessage (VirtualClass* /*sender*/, int /*MessageType*/)
   {}
   virtual void keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down) = 0;

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, bool /*down*/, 
                                         int /*x*/, int /*y*/)
   { return NULL; }

   virtual void mouseMoved (int /*x*/, int /*y*/)
   {}

   // Getters for window position
   inline int getLeft () const
   { return left; }
   inline int getTop () const
   { return top; }
   inline int getWidth () const
   { return width; }
   inline int getHeight () const
   { return height; }
   inline int getRight () const
   { return left + width; }
   inline int getBottom () const
   { return top + height; }

   // Setters for window position
   // All widgets are placed relative to the top left window corner, not 
   // relative to their parent
   inline void setLeft (int newVal) 
   { left = newVal; }
   inline void setTop (int newVal) 
   { top = newVal; }
   inline void setWidth (int newVal) 
   { width = newVal; }
   inline void setHeight (int newVal) 
   { height = newVal; }
   virtual void setGeometry (int _left, int _top, int _width, int _height)
   { left = _left, top = _top, width = _width, height = _height; }

   // Get/set window background
   inline void setBackgroundColor (Color& col)
   { backgroundColor = col; }
   inline void setBackgroundColor (double r, double g, double b, double a = 1.0)
   { backgroundColor = Color (r, g, b, a); }
   Color& getBackgroundColor ()
   { return backgroundColor; }
   const Color& getBackgroundColor () const
   { return backgroundColor; }

   // Add/remove children
   virtual void add (GLWindow* /*win*/)
   {}
   virtual int remove (GLWindow* /*win*/)
   { return 0;}
   virtual size_t getWindowCount () const
   { return 0; }

   // Get parent or main window
   inline GLWindow* getParent ()
   { return parent; }
   GLWindow* getMainWindow();

   // Event: widget looses focus
   virtual void loseFocus ()
   {}

   // Transform to parent window coordinates
   void transformToParentWindowCoordinates (GLWindow* win, int& x, int& y)
   {
      if (win != this && parent != NULL) {
         x += left;
         y += top;
         parent->transformToParentWindowCoordinates (win, x, y);
      }
   }

   // Set listener for events
   inline void setListener (MessageListener* newListener)
   { listener = newListener; }

protected:
   int left, top, width, height;
   Color backgroundColor;
   GLWindow* parent;
   MessageListener* listener;
};

////////////////////////////////////////////////////////////////////////////////
// Render window elements

bool loadFont (const TCHAR* fn, size_t _xs, size_t _ys);

// Render a shaded rectangle
void renderShadedRectangle (double x1, double y1, double x2, double y2);

// Print no more chars of txt than fit into maxWidth
void renderText (const char* txt, int x, int y, size_t maxWidth);

// Obtain length of text
int textWidth (const char* text);

// Fill rectangle in current color
void fillRect(double x1, double y1, double x2, double y2);

void storeScreenSettings (SDL_Surface * screen);

#endif
