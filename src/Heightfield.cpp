////////////////////////////////////////////////////////////////////////////////
// HeightField.cpp
// Height field support for our mech sim
// Copyright Bjoern Ganster 2007-2008
////////////////////////////////////////////////////////////////////////////////

#include "Heightfield.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor, destructor

HeightField::HeightField(size_t xsize, size_t ysize)
: RenderableObject (HeightFieldID, NULL, "Heightfield"),
  m_sizex (xsize), 
  m_sizey (ysize),
  m_mesh (NULL)
{
   heights.resize(m_sizex*m_sizey);
   for (size_t y = 0; y < m_sizey; y++)
      for (size_t x = 0; x < m_sizex; x++) {
         setHeight (x, y, 0);
      }
}

HeightField::~HeightField()
{
   delete m_mesh;
}

////////////////////////////////////////////////////////////////////////////////
// Create random height field

void HeightField::createHeightField(int mountCount, 
   double mountHeight, double mountSlope, double noise, size_t smoothTimes)
{
   /*addPeaks (0.5*mountHeight, mountHeight, 0, 0, m_sizex, m_sizey, mountCount);
   smoothen (0, 0, m_sizex, m_sizey, 3);
   propagateValues(mountSlope);
   addNoise (0, noise);*/

   randomize();
   addPeaks(0.25*mountHeight, mountHeight, 0, 0, m_sizex, m_sizey, mountCount);
   propagateValues(mountSlope);
   addNoise(0, noise);
   smoothen (0, 0, m_sizex-1, m_sizey-1, 5, smoothTimes);
   createMesh();
}

////////////////////////////////////////////////////////////////////////////////
// Add noise to entire height field

void HeightField::addNoise (double min, double max)
{
   for (size_t x = 1; x < m_sizex-1; x++)
      for (size_t y = 1; y < m_sizey-1; y++) {
         double oldH = getHeight(x, y);
         double diff = min+frandom()*(max-min);
         setHeight(x, y, oldH + diff);
      }
}

////////////////////////////////////////////////////////////////////////////////
// Add peaks to restricted area

void HeightField::addPeaks (double min, double max, size_t x1, size_t y1, 
  size_t x2, size_t y2, size_t count)
{
   for (size_t i = 0; i < count; i++) {
      size_t x = (size_t) random ((int) x1, (int) x2);
      size_t y = (size_t) random ((int) y1, (int) y2);
      double h = frandom (min, max);
      setHeight (x, y, h);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Propagate height values

void HeightField::propagateValues(double mountSlope)
{
   // For every height field cell, subtract mountSlope from the highest 
   // neighbour to find this cell's height
   int passes = 100; //2*mountHeight/mountSlope;
   for (int i = 0; i < passes; i++) {
      for (size_t x = 1; x < m_sizex-1; x++)
         for (size_t y = 1; y < m_sizey-1; y++) {
            double max = getHeight(x,y);
            bool changed = false;
            for (int dx = -1; dx < 2; dx++)
               for (int dy = -1; dy < 2; dy++) {
                  double h = getHeight(x+dx, y+dy);
                  if (h > max) {
                     max = h;
                     changed = true;
                  }
               }
            if (changed)
               setHeight(x, y, max-mountSlope);
         }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Smoothen landscape

void HeightField::smoothen (size_t x1, size_t y1, size_t x2, size_t y2, 
   size_t range, size_t times)
{
   for (size_t i = 0; i < times; i++) {
      printf("%i/%i\n", i, times);
      for (size_t y = 0; y < m_sizey; y++) {
         for (size_t x = 0; x < m_sizex; x++) {
            double sum = 0;
            //size_t count = 0;
            int sx = bgmax((int) (x-range), 0);
            int sy = bgmax((int) (y-range), 0);
            int ex = bgmin (m_sizex-1, x+range);
            int ey = bgmin (m_sizey-1, y+range);
            for (int ry = sy; ry <= ey; ry++) {
               for (int rx = sx; rx <= ex; rx++) {
                  sum += getHeight (rx, ry);
                  //count++;
               }
            }
            size_t count = (ex-sx+1)*(ey-sy+1);
            setHeight (x, y, sum / count);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Create mesh

void HeightField::createMesh()
{
   m_mesh = new Mesh2();
   m_mesh->color = Color (1, 1, 1);
   for (size_t y = 0; y < m_sizey; y++)
      for (size_t x = 0; x < m_sizex; x++) {
         Point3D p (x, getHeight (x, y), y);
         Point3D n1, n2, n;
         double h = getHeight (x, y);

         if (x == 0)
            n1 = Point3D (1, h+getHeight(x-1, y), 0);
         else if (x == m_sizex-1)
            n1 = Point3D (1, h-getHeight(x+1, y), 0);
         else if (x > 0 && x < m_sizex-1) {
            n1 = Point3D (2, getHeight(x-1, y)-getHeight(x+1, y), 0);
         }

         if (y == 0)
            n2 = Point3D (0, h+getHeight(x, y-1), 1);
         else if (y == m_sizey-1)
            n2 = Point3D (0, h-getHeight(x, y+1), 1);
         else if (y > 0 && y < m_sizey-1) {
            n2 = Point3D (0, getHeight(x, y-1)-getHeight(x, y+1), 2);
         }

         n1.unitLength();
         n2.unitLength();
         n = n2.crossMultiply(n1);
         n.unitLength();
         //n = Point3D(0, 1, 0);

         m_mesh->addPoint (p, n, x % 2, y%2);
         //m_mesh->addPoint (p.x, p.y, p.z);
         //m_mesh->addNormal (n.x, n.y, n.z);
         //m_mesh->addTextureCoords(x % 2, y%2);
      }

   Point3D n (0, 1, 0);
   for (size_t y = 0; y < m_sizey-1; y++)
      for (size_t x = 0; x < m_sizex-1; x++) {
         size_t i = x+m_sizex*y;
         //MeshPolygon* poly = 
            m_mesh->addQuad(i, i+1, i+m_sizex+1, i+m_sizex);
         //m_mesh->getNormal(i, poly->normal);
         //poly->PerVertexLighting = false;
         //poly->color = Color (0.5, 0.5, 0.5);
      }
      m_mesh->color = Color (0.5, 0.5, 0.5);

   /*Texture* material = new Texture();
   if (material->loadFromFile ("Material\\grass1.bmp"))
       m_mesh->setTexture (material);
   else
      delete material;*/
}

////////////////////////////////////////////////////////////////////////////////
// Render

void HeightField::renderSelf()
{
   m_mesh->renderSelf();

   /*glBegin(GL_QUADS);
   for (size_t x = 0; x < m_sizex-1; x++)
      for (size_t y = 0; y < m_sizey-1; y++) {
         double h = getHeight(x,y);
         glColor3d(h/50.0, h/50.0, h/50.0);
         glVertex3d (x, h, y);

         h = getHeight(x+1, y);
         glColor3d(h/50.0, h/50.0, h/50.0);
         glVertex3d (x+1, h, y);

         h = getHeight(x+1,y+1);
         glColor3d(h/50.0, h/50.0, h/50.0);
         glVertex3d (x+1, h, y+1);

         h = getHeight(x,y+1);
         glColor3d(h/50.0, h/50.0, h/50.0);
         glVertex3d (x, h, y+1);
      }
   glEnd();*/
}

////////////////////////////////////////////////////////////////////////////////
// Render

double HeightField::getHeight (size_t x, size_t y)
{
   if (x < m_sizex && y < m_sizey) 
      return heights[m_sizex*y+x];
   else
      return 0.0;
}

double HeightField::getHeightD (double x, double y)
{
   int x1 = (int) x;
   int x2 = x1+1;
   int y1 = (int) y;
   int y2 = y1+1;

   if (x1 >= 0 
   &&  x2 < (int) m_sizex 
   &&  y1 >= 0
   &&  y2 < (int) m_sizey) 
   {
      double dx = x - x1;
      double dy = y - y1;
      double hy1 = dx * heights[m_sizex*y1+x2] + (1.0 - dx) * heights[m_sizex*y1+x1];
      double hy2 = dx * heights[m_sizex*y2+x2] + (1.0 - dx) * heights[m_sizex*y2+x1];
      return dy * hy2 + (1.0 - dy) * hy1;
   } else
      return 0.0;
}
