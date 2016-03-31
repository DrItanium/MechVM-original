////////////////////////////////////////////////////////////////////////////////
// HeightField.h
// Height field support for our mech sim
// Copyright Bjoern Ganster 2007-2008
////////////////////////////////////////////////////////////////////////////////

#ifndef Heightfield__h
#define Heightfield__h

#include "Mesh.h"
#include "Mesh2.h"

////////////////////////////////////////////////////////////////////////////////
// Height field

class HeightField: public RenderableObject
{
public:
   // Constructor, destructor
   HeightField(size_t xsize, size_t ysize);
   virtual ~HeightField();

   // Create random height field
   void createHeightField(int mountCount, double mountHeight, 
      double mountSlope, double noise, size_t smoothTimes);

   // Add noise to entire height field
   void addNoise (double min, double max);

   // Add peaks to restricted area
   void addPeaks (double min, double max, size_t x1, size_t y1, size_t x2,
                  size_t y2, size_t count);

   // Propagate height values
   void propagateValues(double mountSlope);

   // Smoothen landscape
   void smoothen (size_t x1, size_t y1, size_t x2, size_t y2, size_t range, 
      size_t times);

   inline size_t getSizeX() const
   { return m_sizex; }
   inline size_t getSizeY() const
   { return m_sizey; }

   // Access heights
   double getHeight (size_t x, size_t y);
   double getHeightD (double x, double y);

   inline void setHeight (size_t x, size_t y, double newVal)
   { heights[m_sizex*y+x] = newVal; }

   // Create mesh
   void createMesh();

   // Render
   virtual void renderSelf();

private:
   // Descendant of RenderableObjects, inherits ability to have its own childs
   size_t m_sizex, m_sizey;
   vector <double> heights;
   Mesh2* m_mesh;
};

#endif
