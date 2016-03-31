////////////////////////////////////////////////////////////////////////////////
// GLComboBox.h
// Copyright Björn Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef GLComboBox__H
#define GLComboBox__H

#include "GLLineEdit.h"
#include "GLButton.h"
#include "GLWindowContainer.h"
#include "GLContextMenu.h"

#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLComboBox: public GLWindowContainer {
public:
   Color textColor;

   // Constructor
   GLComboBox (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0);

   // Destructor
   virtual ~GLComboBox();

   // Text access
   inline const char* getText () const
   { return edit.getText(); }
   inline void setText (const char* newText) 
   { edit.setText(newText); }

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int button, bool down, int x, int y)
   { return GLWindowContainer::buttonStateChanged(button, down, x, y); }

   virtual void setGeometry (int _left, int _top, int _width, int _height)
   //{ left = _left, top = _top, width = _width, height = _height; }
   {
      GLWindow::setGeometry(_left, _top, _width, _height);
      edit.setGeometry (0, 0, _width-_height, _height);
      button.setGeometry (edit.getWidth(), 0, _height, _height);
   }

   // Access choices
   size_t getChoicesCount() const
   { return choices.size(); }
   const TCHAR* getChoice(size_t i)
   { return choices[i].getChars(); }
   void addChoice (const TCHAR* newChoice)
   { choices.push_back (BGString (newChoice)); }

   // Receive a message from edit, button, or the context menu
   void receiveMessage (VirtualClass* sender, int MessageType);

   void setListener (MessageListener* _listener)
   { 
      edit.setListener (_listener); 
      listener = _listener;
   }
protected:
   GLLineEdit edit;
   GLButton button;
   glutContextMenu* menu;
   vector<BGString> choices;

   // Show context menu
   void showContextMenu ();
};

#endif
