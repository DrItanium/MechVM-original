////////////////////////////////////////////////////////////////////////////////
// GLSlider.h
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef GLSlider__H
#define GLSlider__H

#include "GLWindow.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLSlider: public GLWindow {
public:
   Color dialColor;

   // Constructor
   GLSlider (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0)
   : GLWindow (_parent, _left, _top, _width, _height),
     dialColor (0, 0, 0),
     m_min (0),
     m_max (1),
     curr (0.5),
     steps(0),
     leftButtonDown(false)
   {
   }

   // Destructor
   virtual ~GLSlider()
   {
   }

   // Render
   virtual void render(int x, int y, int w, int h);

   // Events
   virtual void keypressed (BGKey /*key*/, bool /*shiftHeld*/, 
      bool /*ctrlHeld*/, bool /*down*/)
   {}

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int button, bool down, 
                                         int /*x*/, int /*y*/)
   {
      if (button == 1)
         leftButtonDown = down;
      return this; 
   }
   
   virtual void mouseMoved (int x, int /*y*/)
   {
      if (!leftButtonDown)
         return;

      if (steps == 0) {
         curr = m_min + (x-left)*(m_max-m_min)/width;
         BGString str = "Value: ";
         str.appendDouble (curr);
         str.printLn();
      } else {
         int newStep = (steps-1) * (x-left)/width;
         curr = m_min + newStep * m_max / (steps-1);
      }

      if (curr < m_min)
         curr = m_min;
      else if (curr > m_max)
         curr = m_max;
      sendMessage (listener, ValueChangedEvent);
   }

   // Get/Set minimum, maximum value
   inline void setMinMax (double newMin, double newMax)
   { m_min = newMin; m_max = newMax; }
   inline double getMin () const
   { return m_min; }
   inline double getMax () const
   { return m_max; }

   // Get/Set value
   bool setValue (double newVal)
   {
      if (newVal >= m_min && newVal <= m_max) {
         curr = newVal;
         return true;
      } else
         return false;
   }
   inline double getValue() const
   {
      return curr;
   }

   // Get/set number of steps, zero steps: continuous values
   void setDialCount(int newCount)
   { steps = newCount; }
   int getDialCount()
   { return steps; }
protected:
   double m_min, m_max, curr;
   int steps; // 0: continuous
   bool leftButtonDown;
};

#endif
