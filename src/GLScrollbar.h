////////////////////////////////////////////////////////////////////////////////
// GLScrollBar.h
// Copyright Björn Ganster 2006-2009
////////////////////////////////////////////////////////////////////////////////

#ifndef GLScrollBar__H
#define GLScrollBar__H

#include "GLWindow.h"

class GLScrollbar: public GLWindow {
public:
   // Constructor
   GLScrollbar (GLWindow* _parent);

   // Destructor
   virtual ~GLScrollbar();

   // Render
   virtual void render(int x, int y, int w, int h);
   void renderHorizontalButtonsAndScrollbar ();
   void renderHorizontalButtonsOnly();
   void renderVerticalButtonsAndScrollbar ();
   void renderVerticalButtonsOnly();

   // Events
   virtual void keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down);
   virtual GLWindow* buttonStateChanged (int button, bool down, int x, int y);
   
   virtual void mouseMoved (int x, int y);

   // Access members
   inline void setMin (int newVal)
   { min = newVal; }
   inline void setMax (int newVal)
   { max = newVal; }
   inline void setHorizontal (bool newVal)
   { horizontal = newVal; }
   inline bool getHorizontal ()
   { return horizontal; }
   inline int getMin ()
   { return min; }
   inline int getMax ()
   { return max; }
   inline int getSize ()
   { return size; }
   inline void setStart (int newVal)
   { start = newVal; }
   inline void setSize (int newVal)
   { size = newVal; }
   inline void setButtonSpeed (double newVal)
   { buttonSpeed = newVal; }
   inline double getButtonSpeed ()
   { return buttonSpeed; }

   // Gets the current start of the scrollbar
   int getStart ();

private:
   bool horizontal, // Determines whether this is a horizontal or vertical 
                    // scrollbar
      moved;        // True when user drags the scrollbar
   int start,       // Start position of the scrollbar
      size,         // Size of the scrollbar
      min,          // Minimum position of the scrollbar
      max,          // Maximum position of the scrollbar
      moveStart,    // column for horizontal scrollbar, row for vertical 
                    // scrollbar. Stores start of movement.
      lastClick,    // Time of last click into the scrollbar
      dragStart,    // Scrollbar position where drag started
      movement;
   double buttonSpeed; // Speed in min/max units per second of movement
                       // using the scrollbar buttons
};

#endif
