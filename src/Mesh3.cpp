include "Mesh3.h"

template <int pointCount> 
class PolygonVector: VirtualClass
{
public:
   // Constructor
   PolygonVector();

   // Destructor
   ~PolygonVector();

   // Add polygon
   size_t addPolygon ();

   // Set polygon properties
   void setVertex (size_t indexInPoly, size_t indexInMesh);
   void setTexCoords (size_t indexInPoly, double u, double v);
   void setNormal (size_t indexInPoly, double nx, double ny, double nz);

   // Render polygons
   void render();
