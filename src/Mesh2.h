////////////////////////////////////////////////////////////////////////////////
// Declaration of Mesh2
// Base class for using vertex arrays
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef Mesh2__h
#define Mesh2__h

////////////////////////////////////////////////////////////////////////////////
// Includes & Forwards

#ifdef _WIN32
#include "windows.h"
#endif

#include "RenderableObject.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////
// Class Declaration

class Mesh2: public RenderableObject {
   public:
      Color color;

      // Constructor
      Mesh2 (RenderableObject* parent = NULL);

      // Destructor
      virtual ~Mesh2 ();

      // Render entire scene		
      virtual void renderSelf ();

      // Mark front-facing polygons (requires neighbourhood information)
      //virtual void markFrontFacingPolygons (const Point3D& ls) = 0;

      // Render shadow volumes
      //virtual void renderShadowVolumes (const Point3D& ls) const = 0;

      // Return number of contained polygons
      //virtual size_t getRecursivePolygonCount () const = 0;

      // Return number of contained points
      //virtual size_t getRecursivePointCount () const = 0;

      // Mesh size calculation
      //void getSize(Point3D& minCoords, 
      //   Point3D& maxCoords, size_t& polygons);

      // Recursive Part of mesh size calculation
      //virtual void getSizeRecursive(Point3D& minCoords, Point3D& maxCoords, 
      //                              size_t& polygons, MatrixStack& m);

      // Add points
      inline size_t addPoint (const Point3D& p, const Point3D& n, double u, double v)
      { return addPoint (p.x, p.y, p.z, n.x, n.y, n.z, u, v); }
      size_t addPoint (double x, double y, double z,
                             double nx, double ny, double nz,
                             double u, double v);

      // Set normal
      inline void setNormal (size_t i, double x, double y, double z)
      { normals[3*i] = x; normals[3*i+1] = y; normals[3*i+2] = z; }

      // Add triangle
      void addTriangle (size_t p1, size_t p2, size_t p3);
      size_t addQuad (size_t p1, size_t p2, size_t p3, size_t p4);

      // Add polygons
      void startNewPolygon();
      void addToPolygon (size_t p1, const Point3D& n1);

      // Get number of points stored
      inline size_t getPointCount() const
      { return pointCount; }

      // Get stored point
      Point3D getPoint (size_t num)
      { return Point3D (points[pointComponents*num], points[pointComponents*num+1], 
                        points[pointComponents*num+2], points[pointComponents*num+3]); }

      // Get number of triangles, quads and polygons
      inline GLsizei getTriangleCount () const
      { return triangleCount; }
      inline GLsizei getQuadCount () const
      { return quadCount; }
      //inline size_t getPolygonCount () const
      //{ return polygonCount; }

      // Forget triangles, quads, and polygons, but keep storage
      void clearGeometry()
      { pointCount = 0; triangleCount = 0; quadCount = 0; 
        /*polygonCount = 0; normalCount = 0;*/ }

      // Query triangles, quads
      bool getTriangle (GLsizei num, size_t& p0, size_t& p1, size_t& p2);
      bool getQuad (GLsizei num, size_t& p0, size_t& p1, size_t& p2, size_t& p3);

      // Add all twig polygons to another container, useful for saving hierarchies
      virtual void gatherPolygons (Mesh2* mesh);

      // Save as obj
      void saveAsObj(const TCHAR* FN);

      // Set/get texture
      // Warning: texture not destroyed when this mesh is destroyed
      inline void setTexture (Texture* newText)
      { texture = newText; }
      inline const Texture* getTexture () const
      { return texture; }
      inline Texture* getTexture ()
      { return texture; }

      // Get texture coordinates
      inline void getTexCoords (size_t i, double& u, double& v) const
      {
         if (texCoords != NULL) {
            u = texCoords[2*i]; 
            v = texCoords[2*i+1];
         }
      }

   protected:
      GLsizei pointCount, pointReserve;
      double* points;
      GLsizei* triangles;
      GLsizei* quads;
      //size_t normalCount;
      double* normals;
      double* texCoords;
      GLsizei triangleCount, triangleReserve;
      GLsizei quadCount, quadReserve;
      //GLdouble* triangleTexCoords, * quadTexCoords;
      Texture* texture;

      static const int pointComponents = 3;
};

#endif
