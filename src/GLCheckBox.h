////////////////////////////////////////////////////////////////////////////////
// GLCheckBox.h
// Copyright Bjoern Ganster 2009
////////////////////////////////////////////////////////////////////////////////

#ifndef GLCheckBox__H
#define GLCheckBox__H

#include "GLButton.h"
#include "GLLabel.h"

////////////////////////////////////////////////////////////////////////////////
// Base class for a button

class GLCheckBox: public GLWindowContainer {
public:
   // Constructor
   GLCheckBox (GLWindow* _parent = NULL, int _left = 0, int _top = 0, 
             int _width = 0, int _height = 0)
   : GLWindowContainer (_parent, _left, _top, _width, _height)
   {
      button = new GLButton (_parent, left, 0, _height, _height);
      add (button);
      button->setListener (this);
      label = new GLLabel (_parent, _left+2*height, 0, _width-2*height, _height);
      add (label);
   }

   // Destructor
   virtual ~GLCheckBox()
   {}

   // Title access
   inline const TCHAR* getTitle () const
   { return label->getTitle(); }
   inline void setTitle (const char* newTitle) 
   { label->setTitle (newTitle); }

   virtual void receiveMessage (VirtualClass* sender, int MessageType)
   {
      if (sender == button && MessageType == ButtonClickedEvent) {
         if (isChecked()) {
            button->setTitle ("");
            if (listener != NULL)
               listener->receiveMessage(this, CheckBoxUnchecked);
         } else {
            button->setTitle ("X");
            if (listener != NULL)
               listener->receiveMessage(this, CheckBoxChecked);
         }
      }
   }

   inline bool isChecked () const
   { 
      return strcmp (button->getTitle(), "X") == 0;
   }
   inline void setChecked (bool newVal)
   {
         if (newVal)
            button->setTitle ("X");
         else
            button->setTitle ("");
   }

protected:
   GLLabel* label;
   GLButton* button;
};

#endif
