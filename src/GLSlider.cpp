////////////////////////////////////////////////////////////////////////////////
// GLSlider.cpp
// Copyright Björn Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "GLSlider.h"

////////////////////////////////////////////////////////////////////////////////
// Render

void GLSlider::render(int x, int y, int w, int h)
{
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity ();

   setViewport (x, y, w, h);
   glDisable (GL_TEXTURE_2D);
   glDisable (GL_DEPTH_TEST);

   glLoadIdentity();
   glTranslated (-1.0, 1.0, 0.0);
   glScaled (2/((double) getWidth()), -2/((double) getHeight()), 1.0);

   // Render dials
   dialColor.glColor();
   int y1 = height/3;
   int y2 = 2*height/3;
   glBegin (GL_LINES);
   if (steps > 1) {
      for (int i = 0; i < steps; i++) {
         int x = i*(width-1)/(steps-1)+1;
         glVertex2d (x, y1);
         glVertex2d (x, y2);
      }
   } else {
      for (int i = 0; i <= 10; i++) {
         int x = i*(width-1)/10;
         glVertex2d (x, y1);
         glVertex2d (x, y2);
      }
   }

   /*x = (curr-min)*(double)width/(double)(max-min);
   glColor3f (1.0, 0.0, 0.0);
   glVertex2d (x, y1);
   glVertex2d (x, y2);*/
   glEnd();

   // Render top hand
   const int triSize = height/3;
   x = (int) ((curr-m_min)*(double)(width-1)/(double)(m_max-m_min));
   Color triangleColor (0, 1, 0);
   triangleColor.glColor();
   glBegin (GL_TRIANGLES);
      glVertex2d (x-triSize, 0);
      glVertex2d (x+triSize, 0);
      glVertex2d (x, triSize);
   glEnd();

   // Render bottom hand
   //triangleColor.glColor();
   glBegin (GL_TRIANGLES);
      glVertex2d (x-triSize, height);
      glVertex2d (x+triSize, height);
      glVertex2d (x, 2*triSize);
   glEnd();
}
