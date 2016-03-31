////////////////////////////////////////////////////////////////////////////////
// MeshPolygon.h
// Class Declaration
// Copyright Bjoern Ganster 2001, 2002
////////////////////////////////////////////////////////////////////////////////

#ifndef MeshPolygon__h
#define MeshPolygon__h

#include "Point3D.h"
#include "Texture.h"
#include "TextureCompiler.h"

class Mesh;
class MeshPolygon;

////////////////////////////////////////////////////////////////////////////////
// Mesh Polygon

class MeshPolygon: public VirtualClass
{
private:
   //MeshEdgeSet edges;
   size_t m_pointCount;
   size_t* m_pointIndices;
   double* m_pointCoords;
   double* m_textureCoords;
   //double* m_points;
   //size_t* m_normals;
   Mesh* m_parent;

public:
   bool isVisible;          // Render polygon?
   //Color color;             // For a single-colored polygon
   size_t num;              // The number of the polygon
   int mark;                // User-defined data markings on the polygon
   Point3D normal;
   bool PerVertexLighting;  // true: set color and normals for each vertex
                            // false: set color and normal from above values
   ManagedTexture* m_manText;

   // Constructor
   MeshPolygon (size_t pointCount, Mesh* parent);

   // Destructor
   virtual ~MeshPolygon ();

   // Add a point to the polygon
   bool setPoint (size_t indexInPoly, size_t indexInMesh);

   // Apply this polygon's normal
   inline void glNormal () const
   { normal.glNormal (); }

   // Find opposed edges for all edges in the polygon
   void findOpposedEdges();

   // Get points
   inline size_t getPointCount () const
   { return m_pointCount; }
   inline size_t getPoint (size_t num) const
   { return m_pointIndices[num]; }

   // Render a polygon
   void render(bool hasTexture);

   // Query owning Mesh
   inline Mesh* getParent()
   { return m_parent; }
   inline const Mesh* getParent() const
   { return m_parent; }

   // Get/set texture coordinates
   inline void getTexCoords (size_t i, double& u, double& v) const
   { u = m_textureCoords[2*i]; v = m_textureCoords[2*i+1]; }
   void setTexCoords (size_t i, double u, double v)
   {
      //if (texture != NULL)
         //texture->translateCoords(u,v);
      if (m_textureCoords == NULL)
         m_textureCoords = new double[2*m_pointCount];
      m_textureCoords[2*i] = u;
      m_textureCoords[2*i+1] = v;
   }
   inline bool hasTextureCoords () const
   {
      if (m_textureCoords != NULL)
         return true;
      else
         return false;
   }

};

#endif
