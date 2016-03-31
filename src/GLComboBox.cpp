////////////////////////////////////////////////////////////////////////////////
// GLComboBox.cpp
// Copyright Björn Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "GLComboBox.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLComboBox::GLComboBox (GLWindow* _parent, int _left, int _top, int _width, 
                  int _height)
: GLWindowContainer (_parent, _left, _top, _width, _height),
  textColor (0, 0, 0),
  edit (this),
  button (this),
  menu(NULL)
{
   add (&edit);
   button.setTitle("V");
   button.setListener(this);
   add(&button);
   setGeometry (_left, _top, _width, _height);
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLComboBox::~GLComboBox()
{
}

////////////////////////////////////////////////////////////////////////////////
// Show context menu

void GLComboBox::showContextMenu ()
{
   // The context menu needs to be added to the main window, using the main
   // window's coordinate system
   GLWindow* mainWin = getMainWindow();
   int x = 0, y = height;
   transformToParentWindowCoordinates (mainWin, x, y);
   glutContextMenu* menu = new glutContextMenu (mainWin, x, y);
   for (size_t i = 0; i < choices.size(); i++)
      menu->addMenuItem (NULL, choices[i].getChars(), "", this, (int) i);
   mainWin->add (menu);
}

////////////////////////////////////////////////////////////////////////////////
// Receive a message from edit, button, or the context menu

void GLComboBox::receiveMessage (VirtualClass* sender, int MessageType)
{
   glutContextMenuEntry* entry =
      dynamic_cast <glutContextMenuEntry*> (sender);
   if (entry != NULL) {
      setText (entry->name.getChars());
      sendMessage (listener, ValueChangedEvent);
      delete menu;
      menu = NULL;
   } else {
      GLButton* b = dynamic_cast <GLButton*> (sender);
      if (b != NULL) {
         showContextMenu();         
      }
   }
}
