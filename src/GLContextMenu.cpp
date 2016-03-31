////////////////////////////////////////////////////////////////////////////////
// glutContextMenu.cpp
// Copyright Björn Ganster 2005-2006
////////////////////////////////////////////////////////////////////////////////

#include "GLContextMenu.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor
glutContextMenu::glutContextMenu (GLWindow* _parent, int left, int top)
: GLWindow (_parent, left, top)
{
   backgroundColor = Color (0.7, 0.7, 0.7);
}

// Destructor
glutContextMenu::~glutContextMenu()
{
}

// Add an entry, name and texture should persist until menu deletion
bool glutContextMenu::addMenuItem (Texture* icon, const char* name, 
   const char* shortcut, MessageListener* listener, int ID)
{
   glutContextMenuEntry newEntry;
   newEntry.icon = icon;
   newEntry.name = name;
   newEntry.shortcut = shortcut;
   newEntry.listener = listener;
   newEntry.ID = ID;
   entries.push_back (newEntry);
   
   height = 16 * (int) entries.size()+3;
   int entryWidth = 10 * (int) strlen (name);
   if (entryWidth > width)
      width = entryWidth;
   
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Events

void glutContextMenu::keypressed (BGKey /*key*/, bool /*shiftHeld*/, bool /*ctrlHeld*/,
                                  bool down)
{
}

GLWindow* glutContextMenu::buttonStateChanged (int button, bool down, int x, int y)
{
   if (left <= x
   &&  x < left + width
   &&  top <= y
   &&  y < top + height
   &&  button == 1
   &&  !down) 
   {
      int num = bgmin ((y-top) / 16, (int) entries.size()-1);
      if (entries[num].listener != NULL)
         entries[num].listener->receiveMessage (&entries[num], ContextMenuClickEvent);
      delete this;
      return NULL;
   }
   return this;
}

void glutContextMenu::mouseMoved (int /*x*/, int /*y*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void glutContextMenu::render(int x, int y, int w, int h)
{
   GLWindow::prepareToRender(x, y, w, h);
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);   // Render rule graph nodes
   glEnable (GL_TEXTURE_2D);

   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);

   glColor3d (0.0, 0.0, 0.0);
   for (size_t i = 0; i < entries.size(); i++) {
      const glutContextMenuEntry& entry = entries[i];
      renderText (entry.name.getChars (), 0, (int) i*16, width);
   }
}
