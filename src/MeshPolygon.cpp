////////////////////////////////////////////////////////////////////////////////
// MeshPolygon.cpp
// Class Implementation
// Copyright Bjoern Ganster 2001, 2002
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "Mesh.h"
#include "MeshPolygon.h"
//#include "TransparencyRenderer.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

MeshPolygon::MeshPolygon (size_t pointCount, Mesh* parent)
: VirtualClass (MeshPolygonID),
  //color (1, 1, 1),
  m_pointCount (pointCount), 
  m_pointIndices (new size_t [pointCount]),
  m_pointCoords (new double [3*pointCount]),
  m_textureCoords (NULL),
//  m_normals (new size_t[pointCount]),
//  m_points (new double[3*pointCount]),
  m_parent (parent)
{
   m_manText = NULL;
   isVisible = true;
   PerVertexLighting = false;
   mark = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

MeshPolygon::~MeshPolygon ()
{
  delete m_pointIndices;
  delete m_textureCoords;
//  delete m_normals;
//  delete m_points;
}

////////////////////////////////////////////////////////////////////////////////
// Add a point to the polygon

bool MeshPolygon::setPoint (size_t indexInPoly, size_t indexInMesh)
{ 
   if (indexInPoly < m_pointCount
   &&  indexInMesh < m_parent->getPointCount()) {
      m_pointIndices[indexInPoly] = indexInMesh;
      Point3D p;
      m_parent->getPoint(indexInMesh, p);
      m_pointCoords[3*indexInPoly]   = p.x;
      m_pointCoords[3*indexInPoly+1] = p.y;
      m_pointCoords[3*indexInPoly+2] = p.z;
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Find opposed edges for all edges in the polygon

void MeshPolygon::findOpposedEdges()
{
   // Finding opposed edges works only once all edges of the polygon have been
   // created (otherwise, the next edge, containing the next point, is always
   // missing)
/*   for (unsigned int i = 0; i < m_pointCount; i++) {
      edges[i].FindOpposedEdge ();
   }*/
}

////////////////////////////////////////////////////////////////////////////////
// Render a polygon

/*void MeshPolygon::render(bool skipTransparentPolygons)
{
   // Copy points
/*   for (size_t i = 0; i < m_pointCount; ++i) {
      Point3D p;
      m_parent->getPoint(m_pointIndices[i],p);
      m_points[3*i] = p.x;
      m_points[3*i+1] = p.y;
      m_points[3*i+2] = p.z;
   }

   // Using these commands to render the polygons did not lead to a measurable
   // speedup in rendering. In order to achieve that, polygons in a mesh would
   // have to be sorted by texture, triangles, quads and polygons. This may
   // lead to a small speedup.
   glNormal();
   Texture* text = getTexture();
   text->use();
   glVertexPointer (3, GL_DOUBLE, 0, m_points);
   glTexCoordPointer (2, GL_DOUBLE, 0, m_textureCoords);
   glDrawArrays (GL_POLYGON, 0, m_pointCount);

   if (isVisible) {
      color.glColor();
      Texture* text = getTexture();
      if (text != NULL) {
         double u, v;
         /*if (text->hasAlpha() && skipTransparentPolygons) {
            TransparencyRenderer::getTransparencyRenderer()->add (this);
         } else {
            text->use();
            normal.glNormal();
            glBegin (GL_POLYGON);
               if (PerVertexLighting) {
                  for (unsigned int i = 0; i < m_pointCount; i++) {
                     //MeshPolygonEdge& edge = edges[i];
                     getTexCoords(i, u, v);
                     glTexCoord2d (u,v);
                     //glNormal (i);
                     size_t pi = m_pointIndices[i];
                     m_parent->glVertex(pi);
                  }
               } else {
                  glNormal ();
                  for (unsigned int i = 0; i < m_pointCount; i++) {
                     getTexCoords(i, u, v);
                     glTexCoord2d (u,v);
                     size_t pi = m_pointIndices[i];
                     //if (pi < m_parent->getPointCount())
                        m_parent->glVertex(pi);
                  }
               }
            glEnd ();
         }
      } else {
         if (PerVertexLighting) {
            //glNormal ();
            glBegin (GL_POLYGON);
               for (unsigned int i = 0; i < m_pointCount; i++) {
                  //Point3D& p = *edges[i].point;
                  //p.glNormal ();
                  //p->glColor ();
                  m_parent->glVertex(i);
               }
            glEnd ();
         } else { 
            glNormal ();
            color.glColor ();
            glBegin (GL_POLYGON);
               for (unsigned int i = 0; i < m_pointCount; i++) {
                  glVertex3d(m_pointCoords[3*i], m_pointCoords[3*i+1], m_pointCoords[3*i+2]);
               }
            glEnd ();
         }
      }
   }
}*/

void MeshPolygon::render(bool hasTexture)
{
   glVertexPointer (3, GL_DOUBLE, 0, m_pointCoords);

   if (m_textureCoords != NULL && hasTexture) {
      glTexCoordPointer (2, GL_DOUBLE, 0, m_textureCoords);
   }

   /*if (m_normals != NULL) {
      glEnableClientState (GL_NORMAL_ARRAY);
      glNormalPointer (GL_DOUBLE, 0, m_normals);
   } else {*/
      normal.glNormal();
   //}

   //color.glColor ();

   glDrawArrays (GL_POLYGON, 0, m_pointCount);
}
