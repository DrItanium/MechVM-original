////////////////////////////////////////////////////////////////////////////////
// Toolbar implementation
// Copyright Bjoern Ganster 2005-2007
////////////////////////////////////////////////////////////////////////////////

#include "Toolbar.h"
#include "Texture.h"
#include "BGString.h"

#ifdef _WIN32
const char* path = "..\\icons\\";
#else
const char* path = "../icons/";
#endif

////////////////////////////////////////////////////////////////////////////////
// ToolbarIcon

// Constructor
ToolbarIcon::ToolbarIcon (GLWindow* _parent, int newID, bool RadioButton)
: GLWindow (_parent),
  texture (NULL),
  ID (newID),
  isRadioButton (RadioButton),
  clicked (false)
{
}

// Destructor
ToolbarIcon::~ToolbarIcon()
{
   if (texture != NULL)
      delete texture;
}

// Load icon
bool ToolbarIcon::loadIcon (const char* iconFileName)
{
   if (texture != NULL)
      delete texture;
   texture = new Texture();
   BGString fn = path;
   fn += iconFileName;
   bool loadSuccess = texture->loadFromFile (fn.getChars());

   // Do not try to activate yet - OpenGL may not be initialized!
   if (!loadSuccess) {
      BGString msg = "Failed to load ";
      msg += fn;
      logLn (msg);
   }

   return loadSuccess;
}

////////////////////////////////////////////////////////////////////////////////
// Render

void ToolbarIcon::render(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
   int margin = 10;
   renderShadedRectangle (left-1-margin, top-margin, left+width-margin, 
                          top+height+1-margin);
   glEnable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);
   texture->deactivate();
   texture->use();

   if (clicked)
      glColor3d (0.5, 0.5, 0.5);
   else
      glColor3d (1.0, 1.0, 1.0);

   glBegin (GL_POLYGON);
      texture->glTexCoord (0.0, 1.0);
      glVertex2d (left-margin, top-margin);
      texture->glTexCoord (1.0, 1.0);
      glVertex2d (left+width-margin, top-margin);
      texture->glTexCoord (1.0, 0.0);
      glVertex2d (left+width-margin, top+height-margin);
      texture->glTexCoord (0.0, 0.0);
      glVertex2d (left-margin, top+height-margin);
   glEnd ();
}

////////////////////////////////////////////////////////////////////////////////
// Constructor

int ToolbarButtonMargin = 20;
int h = 24;
int w = 24;

ToolbarIcon* Toolbar::addIcon (const char* fn)
{
   ToolbarIcon* button = new ToolbarIcon(this, 0, false);
   button->loadIcon (fn);
   button->ID = (int) icons.size();
   button->setGeometry ((button->ID+1)*ToolbarButtonMargin+button->ID*w, 
                        ToolbarButtonMargin, h, w);
   icons.push_back (button);
   return button;
}

////////////////////////////////////////////////////////////////////////////////
// Constructor

Toolbar::Toolbar (GLWindow* _parent)
: GLWindow (_parent),
  selected (NULL),
  traceMouse (false),
  listener (NULL)
{
   addIcon ("filenew.bmp");
   addIcon ("fileopen.bmp");
   addIcon ("filesave.bmp");
   //addIcon ("eye.bmp");
   //addIcon ("stop-rendering.bmp");

   constructButtonsGroup ("MouseLeft.bmp", 5);
   constructButtonsGroup ("MouseRight.bmp", 11);
   constructButtonsGroup ("MouseBoth.bmp", 17);

   setBackgroundColor (0.7, 0.7, 0.7);

   // Activate preselected mouse modes
   icons[5]->clicked = true;
   icons[13]->clicked = true;
   icons[18]->clicked = true;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Toolbar::~Toolbar()
{
}

void Toolbar::constructButtonsGroup (const char* groupIcon, int num)
{
   ToolbarIcon* button1 = addIcon (groupIcon);
   ToolbarIcon* button2 = addIcon ("arrow.bmp");
   ToolbarIcon* button3 = addIcon ("Rotate.bmp");
   ToolbarIcon* button4 = addIcon ("Translate.bmp");
   ToolbarIcon* button5 = addIcon ("zoom.bmp");
   ToolbarIcon* button6 = addIcon ("move.bmp");

   button1->setRadioButton (button1, button2);
   button2->setRadioButton (button1, button3);
   button3->setRadioButton (button1, button4);
   button4->setRadioButton (button1, button5);
   button5->setRadioButton (button1, button6);
   button6->setRadioButton (button1, NULL);
}

////////////////////////////////////////////////////////////////////////////////
// Render

void Toolbar::render(int x, int y, int w, int h)
{
   GLWindow::prepareToRender (x, y, w, h);
   glDisable (GL_DEPTH_TEST);
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);

   for (unsigned int i = 0; i < icons.size(); i++) {
      ToolbarIcon* icon = icons[i];
      icon->render (x, y, w, h);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Find selected button

void Toolbar::findSelected (int x, int y)
{
   selected = NULL;

   for (unsigned int i = 0; i < icons.size(); i++) {
      ToolbarIcon* icon = icons[i];
      if (x >= icon->getLeft()
      &&  y >= icon->getTop()
      &&  x < icon->getRight()
      &&  y < icon->getBottom())
      {
         // Deselect all radio buttons in this icon's group
         if (icon->isRadioButton) {
            ToolbarIcon* curr = icon->firstInGroup;
            while (curr != NULL) {
               curr->clicked = false;
               curr = curr->nextInGroup;
            }
         }

         icon->clicked = true;
         selected = icon;
      } else if (!icon->isRadioButton)
         icon->clicked = false;
   }
}

////////////////////////////////////////////////////////////////////////////////

GLWindow* Toolbar::buttonStateChanged (int button, int x, int y, bool down)
{
   traceMouse = false;

   if (button == 1) {
      if (down) {
         findSelected (x, y);
         traceMouse = true;
      } else if (selected != NULL) {
         listener->receiveMessage (selected, ToolbarButtonClickedEvent);
         if (!selected->isRadioButton)
            selected->clicked = false;
         selected = NULL;
      }
   }

   // Toolbar has focus now
   return this;
}

////////////////////////////////////////////////////////////////////////////////

void Toolbar::mouseMoved (int x, int y)
{
   if (traceMouse) {
      findSelected (x, y);
   }
}
