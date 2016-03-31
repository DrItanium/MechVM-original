////////////////////////////////////////////////////////////////////////////////
// GLLineEdit.h
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "GLLineEdit.h"
//#include <SDL/SDL.h>

////////////////////////////////////////////////////////////////////////////////
// Constructor

GLLineEdit::GLLineEdit (GLWindow* _parent, int _left, int _top, 
   int _width, int _height)
: GLWindow (_parent, _left, _top, _width, _height),
  textColor (0,0,0),
  cursorPos (0)
{
   backgroundColor = Color (1, 1, 1);
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

GLLineEdit::~GLLineEdit()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void GLLineEdit::render(int x, int y, int w, int h)
{
   prepareToRender(x, y, w, h);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   textColor.glColor();
   int textTop = (height - 17) / 2;
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);
   glDepthMask (0);
   renderText (getText(), 2, textTop, width);

   BGString str;
   str.copyPart (text.getChars(), 0, cursorPos);
   int screenPos = textWidth (str.getChars());

   glColor3d (0, 0, 0);
   glBegin (GL_LINES);
     glVertex2i (2+screenPos, textTop);
     glVertex2i (2+screenPos, textTop+17);
   glEnd ();

   glDepthMask (~0);
}

////////////////////////////////////////////////////////////////////////////////
// Events

void GLLineEdit::keypressed (BGKey key, bool shiftHeld, bool /*ctrlHeld*/, bool down)
{
   BGString str;

   if (!down)
      return;

   switch (key.sym) {
      case 8: // Backspace
         if (cursorPos > 0)
            str.copyPart (text.getChars(), 0, cursorPos-1);
         else
            str.clear();
         str.appendPart (text.getChars(), cursorPos, text.getLength());
         text = str;
         if (cursorPos > 0)
            cursorPos--;
         break;
      case 127: // Delete key
         str.copyPart (text.getChars(), 0, cursorPos);
         str.appendPart (text.getChars(), cursorPos+1, text.getLength());
         text = str;
         break;
      case SDLK_LEFT:
         if (cursorPos > 0)
            cursorPos--;
         break;
      case SDLK_RIGHT:
         if (cursorPos < (int) text.getLength()) 
            cursorPos++;
         break;
      case SDLK_HOME:
         cursorPos = 0;
         break;
      case SDLK_END:
         cursorPos = (int) text.getLength();
         break;
      case SDLK_RSHIFT:
      case SDLK_LSHIFT:
         break;
      default:
         if (key.unicode >= 32) { // Append normal key
            BGString str;
            str.copyPart (text.getChars (), 0, cursorPos);
            str += (char) key.unicode;
            str.appendPart (text.getChars(), cursorPos, text.getLength());
            text = str;
            cursorPos++;
         } else { // Print unknown key
            BGString str = "Pressed key ";
            str.appendInt (key.sym);
            str.printLn ();
         }
   }

   sendMessage (listener, ValueChangedEvent);
}

/*void GLLineEdit::specialKeyPressed (char key, bool shiftHeld, 
                                    bool ctrlHeld)
{
   switch (key) {
      case 100: // Cursor left
         if (cursorPos > 0)
            cursorPos--;
         break;
      case 102: // Cursor right
         if (cursorPos < (int) text.getLength()) 
            cursorPos++;
         break;
      case 106: // Home key
         cursorPos = 0;
         break;
      case 107: // End key
         cursorPos = (int) text.getLength();
         break;
      default:
         BGString str = "Special key pressed: ";
         str.appendInt (key);
         str.printLn();
   }

   //sendMessage (listener, TextChanged);
}*/

////////////////////////////////////////////////////////////////////////////////
// Button state changed over a window: returns child that was clicked

GLWindow* GLLineEdit::buttonStateChanged (int button, bool down, int x, int y)
{
    if (button == 1 && !down) {
      int screenPos;
      cursorPos= text.getLength();
      bool doContinue;

      do {
         BGString str;
         str.copyPart (text.getChars(), 0, cursorPos);
         screenPos = textWidth (str.getChars());

         if (left + screenPos + 2 > x && cursorPos > 0) {
            --cursorPos;
            doContinue = true;
         } else
            doContinue = false;
      } while (doContinue);

      return this;
   } else
      return NULL;
}

void GLLineEdit::mouseMoved (int /*x*/, int /*y*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Event: widget looses focus

void GLLineEdit::loseFocus ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Try to interpret text as numerical values

bool GLLineEdit::getDouble (double& value) const
{
   return text.toDouble (value);
}

bool GLLineEdit::getInt (int& value) const
{
   value = atoi (text.getChars());
   return true;
}
