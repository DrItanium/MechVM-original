////////////////////////////////////////////////////////////////////////////////
// Mesh.h
// Class Declaration
// Copyright Bjoern Ganster 2001-2006
////////////////////////////////////////////////////////////////////////////////

#ifndef Meshes__H
#define Meshes__H

////////////////////////////////////////////////////////////////////////////////
// Includes

#include "MeshPolygon.h"
#include "RenderableObject.h"
#include "BGVector.h"

class Texture;

//class MeshPoint3D;

////////////////////////////////////////////////////////////////////////////////
// Mesh

class Mesh: public RenderableObject
{
public:
   // Standard RenderableObject members: constructor, destructor, rendering
   Mesh (RenderableObject* _parent = NULL, const char* _name = NULL);
   virtual ~Mesh ();
   virtual void renderSelf ();

   // Create a point and add it to the points' container
   size_t addPoint (double x, double y, double z);

   inline size_t addPoint (const Point3D& p)
   { return addPoint (p.x, p.y, p.z); }

   // Get/set points
   bool getPoint (size_t index, Point3D& p) const;
   void setPoint (size_t index, Point3D& p);

   // Get point statistics
   virtual size_t getRecursivePointCount() const
   { return points.getSize(); }
   inline size_t getPointCount() 
   { return points.getSize()/3; }

   inline void glVertex(size_t i) const
   { i*=3; glVertex3d(points[i], points[i+1], points[i+2]); }

   // Polygon access
   inline size_t getPolygonCount ()
   { return polygons.getSize(); }
   inline MeshPolygon* getPolygon (size_t num)
   { return polygons[num]; }

   // Add polygon, use MeshPolygon::AddPoint to add points to it,
   // finally, call MeshPolygon::FindOppesedEdges()
   MeshPolygon* addPolygon (size_t pointCount);
   inline MeshPolygon* addTriangle ()
   { return addPolygon(3); }
   inline MeshPolygon* addQuad ()
   { return addPolygon(4); }

   // Remove a polygon from the mesh
   void removePolygon (MeshPolygon* poly);

   // Remove point, update point numbers
   //void removePoint (Point3D* p);
   // Load from file
   bool loadFromObj (const char* FileName, Texture* textureToUse = NULL);

   // Save to file
   bool saveToObj (const char* FileName, bool saveUVs = false);

   // Get min/max mesh coords to decide about grid sizes
   Point3D getMinCoords () const;
   Point3D getMaxCoords () const;

   // Reduce polygons to triangles
   void reducePolygonsToTriangles ();

   // Calculate normals for polygons and vertices
   void calculateNormals ();

   // Mark front-facing polygons
   virtual void markFrontFacingPolygons (const Point3D& ls);

   // Render shadow volumes
   virtual void renderShadowVolumes (const Point3D& ls) const;

   // Return number of contained polygons
   virtual size_t getRecursivePolygonCount () const
   { return polygons.getSize(); }

   // Set per vertex lighting
   void setPerVertexLighting (bool newVal);

   void scaleNormals(double factor)
   {
      for (size_t i = 0; i < polygons.getSize(); i++) {
         MeshPolygon* poly = polygons[i];
         poly->normal = poly->normal*factor;
      }
   }

   void calcCircumcircle ();
   void getCircumcircle (Point3D& center, double& radius, const Matrix& M);

   // Texture access
   inline void setTexture (Texture* newText)
   { texture = newText; }
   inline const Texture* getTexture () const
   { return texture; }
   inline Texture* getTexture () 
   { return texture; }

protected:
   Texture* texture;
   Color DefaultPolygonColor;
   Point3D circumcircleCenter;
   double circumcircleRadius;
   BGVector <double, BGVectorBufID> points;
   BGVector <MeshPolygon*, BGVector_MeshPolygon> polygons;

   // Recursive Part of mesh size calculation
   virtual void getSizeRecursive (Point3D& minCoords, Point3D& maxCoords, 
                                  size_t& polygons, MatrixStack& m);
};

#endif
