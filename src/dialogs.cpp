////////////////////////////////////////////////////////////////////////////////
// This file declares functions to display standard dialogs
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#else
#include <qfiledialog.h>
#include <qapplication.h>
#endif

//#include "GLWindow.h"
#include "dialogs.h"

//#include <GL/glut.h>

////////////////////////////////////////////////////////////////////////////////

bool getOpenFileName (BGString& FN)
{
   bool success = false;

   #ifdef _WIN32
   OPENFILENAME ofnStruct;
   memset (&ofnStruct, 0, sizeof (ofnStruct));
   ofnStruct.lStructSize = sizeof (ofnStruct);
   ofnStruct.nMaxFile = 512;
   ofnStruct.lpstrFile = new char[ofnStruct.nMaxFile];
   ofnStruct.lpstrFile[0] = 0;

   if (GetOpenFileName (&ofnStruct) != 0) {
      FN = ofnStruct.lpstrFile;
      success = true;
   }

   delete ofnStruct.lpstrFile;
   #else
   const char* str = QFileDialog::getOpenFileName ().toLatin1();
   FN = str;
   if (FN.getLength () != 0)
      success = true;
   #endif

   return success;
}

////////////////////////////////////////////////////////////////////////////////

bool getSaveFileName (BGString& FN)
{
   bool success = false;

   #ifdef _WIN32
   OPENFILENAME ofnStruct;
   memset (&ofnStruct, 0, sizeof (ofnStruct));
   ofnStruct.lStructSize = sizeof (ofnStruct);
   ofnStruct.nMaxFile = 512;
   ofnStruct.lpstrFile = new char[ofnStruct.nMaxFile];
   ofnStruct.lpstrFile[0] = 0;

   if (GetSaveFileName (&ofnStruct) != 0) {
      FN = ofnStruct.lpstrFile;
      success = true;
   }

   delete ofnStruct.lpstrFile;
   #else
   const char* str = QFileDialog::getSaveFileName ().toLatin1();
   FN = str;
   if (FN.getLength () != 0)
      success = true;
   #endif

   return success;
}

////////////////////////////////////////////////////////////////////////////////

int messageBox (const TCHAR* title, const TCHAR* message, int utype)
{
   #ifdef WIN32
   return MessageBox(0, message, title, utype);
   #else
   /*fixme();
   logLn (title);
   logLn (message);
   return ID_CANCEL;*/
   QString qTitle (title);
   QString qMessage (message);
   switch (utype) {
      case MB_OK:
      {
         QMessageBox mb (qTitle, qMessage, QMessageBox::NoIcon, QMessageBox::Ok, 
                         QMessageBox::NoButton, QMessageBox::NoButton);
         mb.setModal (true);
         return mb.exec();
      }
      break;
      case MB_OKCANCEL:
      {
         QMessageBox mb (qTitle, qMessage, QMessageBox::NoIcon, QMessageBox::Ok, 
                         QMessageBox::Cancel, QMessageBox::NoButton);
         mb.setModal (true);
         return mb.exec();
      }
      break;
      case MB_YESNO:
      {
         QMessageBox mb (qTitle, qMessage, QMessageBox::NoIcon, QMessageBox::Yes, 
			 QMessageBox::No, QMessageBox::NoButton);
         mb.setModal (true);
         return mb.exec();
      }
      case MB_YESNOCANCEL:
      {
         QMessageBox mb (qTitle, qMessage, QMessageBox::NoIcon, QMessageBox::Yes, QMessageBox::No,
                        QMessageBox::Cancel);
         mb.setModal (true);
         return mb.exec();
      }
      break;
      default:
	return -1;
   }
   #endif
}

////////////////////////////////////////////////////////////////////////////////
// Present user with several options to a question

// Attempting to display a separate window fails, because glutCreateSubWindow
// displays the sub window *inside* the main window. It is not possible to
// move the sub window outside the main window.

/*int width, height;

void renderSubScene()
{
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();

   glClearColor (0.7f, 0.7f, 0.7f, 1.0f);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glDisable (GL_DEPTH_TEST);

   glColor3d (1, 1, 1);
   glBegin (GL_TRIANGLES);
      glVertex2d(0, 0);
      glVertex2d(1,0);
      glVertex2d(1,1);
   glEnd();

   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) width), -2/((double) height), 1.0);
   glColor3d (0, 0, 0);
   renderText ("Hello subwin!", 10, 10, 100);
   glutSwapBuffers();  
}

void resizeSub (GLsizei _width, GLsizei _height)
{
   width = _width;
   height = _height;

   glViewport (0, 0, width+1, height+1);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();
}

int ask (const TCHAR* title, const TCHAR* msg, const TCHAR* buttonTexts)
{
   // Store previously current window
   int previousWin = glutGetWindow();

   // Create new window
   glutCreateSubWindow(1, 100, 100, 200, 200);
   //glutMouseFunc (buttonStateChange);
   //glutMotionFunc (mouseMoved);
   glutReshapeFunc (resizeSub);
   //glutKeyboardFunc (keyPressed);
   //glutSpecialFunc (specialKeyPressed);
   glutDisplayFunc (renderSubScene);
   //glutIdleFunc(renderSubScene);

   // Restore previous window
   glutSetWindow (previousWin);
   //glutMainLoop();
   return 0;
}
*/
