////////////////////////////////////////////////////////////////////////////////
// Toolbar declaration
// Copyright Bjoern Ganster 2005
////////////////////////////////////////////////////////////////////////////////

#ifndef Toolbar__H
#define Toolbar__H

#include <vector>
using namespace std;

#include "GLWindow.h"

class Texture;
class Toolbar;

////////////////////////////////////////////////////////////////////////////////
// ToolbarIcon

class ToolbarIcon: public GLWindow {
   friend class Toolbar;
public:
   // Constructor
   ToolbarIcon(GLWindow* _parent, int ID, bool RadioButton);
   
   // Destructor
   ~ToolbarIcon();

   // Load icon
   bool loadIcon (const char* iconFileName);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Member access
   inline int getID ()
   { return ID; }

   // Event handlers
   virtual void keypressed (BGKey /*c*/, bool, bool, bool /*down*/)
   {}
   virtual void mouse (int /*x*/, int /*y*/, int /*buttons*/)
   {}
   virtual void ToolbarButtonClicked (int /*ID*/)
   {}

   virtual void buttonDown (int /*x*/, int /*y*/)
   {
   }

   inline void setRadioButton (ToolbarIcon* first, ToolbarIcon* next)
   {
      firstInGroup = first;
      nextInGroup = next;
      isRadioButton = true;
   }

protected:
   Texture* texture;
   int ID;
   bool isRadioButton, clicked;
   ToolbarIcon * firstInGroup, * nextInGroup;
};

////////////////////////////////////////////////////////////////////////////////

class ToolbarMouseCaptureIcon: public ToolbarIcon {
public:
   // Constructor
   ToolbarMouseCaptureIcon (GLWindow* _parent, int ID, bool RadioButton)
      : ToolbarIcon (_parent, ID, RadioButton)
   {
      active = false;
   }

   GLWindow* buttonStateChanged (int button, int x, int y, bool down)
   {
      if (button == 1
      &&  down) 
      {
         active = true;
         //glutSetCursor (GLUT_CURSOR_NONE);
         startx = x;
         starty = y;
         lastx = 0;
         lasty = 0;
      } else
      if (button == 1
      &&  !down
      &&  active) 
      {
         active = false;
         //glutSetCursor (GLUT_CURSOR_LEFT_ARROW);
//         getInternalWM ()->setFocusedWidget (NULL);
         //cout << "Mouse capture ended at " << x << ", " << y << endl;
      }

      return NULL;
   }

   virtual void mouseMoved (int x, int y)
   {
      if (active
      &&  (x != lastx || y != lasty)) 
      {
         //controller->capturedMouseMoved (getID(), x-lastx, y-lasty);
//         glutWarpPointer (startx, starty);
         lastx = x;
         lasty = y;
      }
   }

   virtual void buttonDown (int /*x*/, int /*y*/)
   {
   }

private:
   bool active;
   int startx, starty, lastx, lasty;
};

////////////////////////////////////////////////////////////////////////////////

class Toolbar: public GLWindow {
public:
   // Constructor
   Toolbar (GLWindow* _parent);

   // Destructor
   virtual ~Toolbar();
   
   // Events
   virtual void keypressed (SDLKey /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/, bool /*down*/)
   {}
   virtual void mouse (int /*x*/, int /*y*/, int /*buttons*/)
   {}
   virtual GLWindow* buttonStateChanged (int /*button*/, //int /*state*/, 
                                    int /*x*/, int /*y*/, bool down);
   virtual void mouseMoved (int x, int y);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Create mouse viewer modes
   void constructButtonsGroup (const char* groupIcon, int num);

   // Add an icon to the toolbar
   ToolbarIcon* addIcon (const char* fn);

   // Set listener
   inline void setListener (MessageListener* newListener)
   { listener = newListener; }
private:
   vector <ToolbarIcon*> icons;
   ToolbarIcon* selected;
   bool traceMouse;
   MessageListener* listener;

   // Find selected button
   void findSelected (int x, int y);
};
#endif
