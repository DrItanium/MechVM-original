////////////////////////////////////////////////////////////////////////////////
// glutContextMenu.h
// Copyright Björn Ganster 2005-2006
////////////////////////////////////////////////////////////////////////////////

#ifndef glutContextMenu__H
#define glutContextMenu__H

#include "GLWindow.h"
#include "BGString.h"
#include "BGBase.h"

#include <vector>
using namespace std;

class Texture;

////////////////////////////////////////////////////////////////////////////////
// Class for storing context menu entries

struct glutContextMenuEntry: public VirtualClass {
   Texture* icon;
   BGString name;
   const char* shortcut;
   MessageListener* listener;
   int ID;
};

////////////////////////////////////////////////////////////////////////////////
// Context menu class

class glutContextMenu: public GLWindow {
public:
   // Constructor
   glutContextMenu (GLWindow* _parent, int left, int top);

   // Destructor
   virtual ~glutContextMenu();

   // Add an entry, name and texture should persist until menu deletion
   bool addMenuItem (Texture* icon, const char* name, const char* shortcut,
                     MessageListener* listener, int ID);

   // Events
   virtual void keypressed (BGKey key, bool shiftHeld, bool ctrlHeld, bool down);
   virtual GLWindow* buttonStateChanged (int /*button*/, bool down, 
                                    int /*x*/, int /*y*/);
   
   virtual void mouseMoved (int x, int y);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Event: widget looses focus
   virtual void loseFocus ()
   { delete this; }

private:
   vector <glutContextMenuEntry> entries;
};

#endif
