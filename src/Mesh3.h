////////////////////////////////////////////////////////////////////////////////
// Mesh3 Class Declaration
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#ifndef Mesh3__h
#define Mesh3__h

#include "BGBase.h"
#include "BGVector.h"
#include "RenderableObject.h"
#include "Texture.h"

////////////////////////////////////////////////////////////////////////////////

//class Mesh3;

class PolygonVector: RenderableObject
{
public:
   // Constructor
   PolygonVector(size_t vertexCount)
      : RenderableObject (Mesh3ID),
        m_polygonCount (0),
        m_vertexCount (vertexCount)
   {
   }

   // Destructor
   ~PolygonVector()
   {
      m_polygonCount = 0; 
      m_vertexCount = 0;
   }

   // Add polygon
   inline size_t addPolygon ()
   {
      size_t result = m_polygonCount;
      ++m_polygonCount;
      m_pointIndices.setSize (m_vertexCount * m_polygonCount);
      return result;
   }

   inline size_t getVertexIndex (size_t polyNum, size_t indexInPoly) const
   {
      size_t index = m_vertexCount * polyNum + indexInPoly;
      if (index < m_pointIndices.getSize())
         return m_pointIndices[index];
      else
         return 0;
   }

   inline void setVertex (size_t polyNum, size_t indexInPoly, size_t indexInMesh)
   {
      size_t index = m_vertexCount * polyNum + indexInPoly;
      if (index < m_pointIndices.getSize())
         m_pointIndices[index] = indexInMesh;
   }

   // Render polygons
   void render()
   {
      if (m_polygonCount <= 0)
         return;

      if (m_vertexCount == 3)
         glDrawElements(GL_TRIANGLES, 3*m_polygonCount, GL_UNSIGNED_INT, 
            m_pointIndices.getRawBuf());
      else if (m_vertexCount == 4)
         glDrawElements(GL_QUADS, 4*m_polygonCount, GL_UNSIGNED_INT, 
            m_pointIndices.getRawBuf());
      else if (m_vertexCount > 4)
         for (size_t i = 0; i < m_polygonCount; ++i)
            glDrawElements(GL_POLYGON, m_vertexCount, GL_UNSIGNED_INT, 
               &m_pointIndices[i*m_vertexCount]);

      /*Mesh3* m3 = m_parent;
      for (size_t i = 0; i < m_polygonCount; ++i) {
         glBegin(GL_POLYGON);
         for (size_t j = 0; j < m_polygonCount; ++j) {
            size_t index = m_pointIndices[j];
            Point3D v = m3->getVertex(index);
            v.glVertex();
         }
         glEnd();
      }*/
   }

   inline size_t getPolygonCount() const
   { return m_polygonCount; }
private:
   size_t m_polygonCount, m_vertexCount;
   BGVector <size_t, BGVector_PolygonListBufferID> m_pointIndices;
};

////////////////////////////////////////////////////////////////////////////////

class Mesh3: public RenderableObject 
{
public:
   // Constructor
   Mesh3(RenderableObject* parent)
      : RenderableObject (Mesh3ID, parent), 
        m_vertexCount (0),
        m_deleteTextureWhenDone (false),
        m_texture (NULL),
        m_vertexCoords (), 
        m_texCoords (), 
        m_normals (), 
        m_polygonVectors ()
   {
   }

   // Destructor
   virtual ~Mesh3()
   {
      for (size_t i = 0; i < m_polygonVectors.getSize(); ++i) {
         delete m_polygonVectors[i];
      }

      if (m_texture != NULL
      &&  m_deleteTextureWhenDone)
      {
         delete m_texture;
         m_texture = NULL;
      }
   }

   // Add polygon
   size_t addPolygon (size_t vertexCount)
   {
      m_vertexCount += vertexCount;
      m_vertexCoords.setSize (3*m_vertexCount);
      m_texCoords.setSize (2*m_vertexCount);
      m_normals.setSize (3*m_vertexCount);

      while (m_polygonVectors.getSize() <= vertexCount)
         m_polygonVectors.add (new PolygonVector (vertexCount));

      return m_polygonVectors[vertexCount]->addPolygon ();
   }

   // Add vertex
   inline size_t addVertex (double x, double y, double z)
   {
      size_t base = 3*m_vertexCount;
      size_t result = m_vertexCount;

      m_vertexCoords.setSize(base+3);
      m_texCoords.setSize(2*m_vertexCount+2);
      m_normals.setSize(3*m_vertexCount+3);

      m_vertexCoords[base]   = x;
      m_vertexCoords[base+1] = y;
      m_vertexCoords[base+2] = z;
      ++m_vertexCount;
      return result;
   }

   inline size_t addVertex (const Point3D& p) 
   { return addVertex (p.x, p.y, p.z); }

   // Set polygon properties
   Point3D getVertex(size_t i) const
   { 
      i *= 3;
      if (i + 2 < m_vertexCoords.getSize())
         return Point3D (m_vertexCoords[i], m_vertexCoords[i+1], m_vertexCoords[i+2]);
      else
         return Point3D (0.0, 0.0, 0.0, 1.0);
   }

   void setVertex (size_t polyonVertexCount, size_t polyNum, size_t indexInPoly, 
      size_t indexInMesh)
   {
      PolygonVector* polygonVector = m_polygonVectors[polyonVertexCount];
      if (polygonVector != NULL) {
         polygonVector->setVertex (polyNum, indexInPoly, indexInMesh);
      }
   }

   void setTexture (Texture* texture, bool deleteWhenDone)
   {
      m_texture = texture;
      m_deleteTextureWhenDone = deleteWhenDone;
   }

   void setTexCoords (size_t polyonVertexCount, size_t polyNum, size_t indexInPoly, 
      double u, double v)
   {
      PolygonVector* polygonVector = m_polygonVectors[polyonVertexCount];
      if (polygonVector != NULL) {
         size_t index = 2 * polygonVector->getVertexIndex(polyNum, indexInPoly);
         if (index+1 < m_texCoords.getSize()) {
            m_texCoords[index]   = u;
            m_texCoords[index+1] = v;
         }
      }
   }

   void setNormal (size_t polyonVertexCount, size_t polyNum, size_t indexInPoly, 
      double nx, double ny, double nz)
   {
      PolygonVector* polygonVector = m_polygonVectors[polyonVertexCount];
      if (polygonVector != NULL) {
         size_t index = 3 * polygonVector->getVertexIndex(polyNum, indexInPoly);
         if (index+2 < m_texCoords.getSize())
            m_normals[index]   = nx;
            m_normals[index+1] = ny;
            m_normals[index+2] = nz;
      }
   }

   size_t getPointCount () const
   { return m_vertexCount; }

   virtual void renderSelf ()
   {
      if (m_vertexCoords.getSize() == 0)
         return;

      /*glEnableClientState (GL_VERTEX_ARRAY);
      glVertexPointer (3, GL_DOUBLE, 0, m_vertexCoords.getRawBuf());

      if (m_texture != NULL) {
         glEnableClientState (GL_TEXTURE_COORD_ARRAY);
         glTexCoordPointer (2, GL_DOUBLE, 0, m_texCoords.getRawBuf());
         m_texture->useMipmap();
      } else {
         glDisable(GL_TEXTURE_2D);
         glColor3d (1.0, 1.0, 1.0);
      }

      glEnableClientState (GL_NORMAL_ARRAY);
      glNormalPointer (GL_DOUBLE, 0, m_normals.getRawBuf());

      for (size_t i = 3; i < m_polygonVectors.getSize(); ++i) {
         PolygonVector* polies = m_polygonVectors[i];
         polies->render();
      }

      glDisableClientState (GL_VERTEX_ARRAY);
      if (m_texture != NULL) {
         glDisableClientState (GL_TEXTURE_COORD_ARRAY);
         glDisableClientState (GL_NORMAL_ARRAY);
         glDisableClientState (GL_TEXTURE_COORD_ARRAY);
         glDisableClientState (GL_VERTEX_ARRAY);
      }*/

      glColor3d (1.0, 1.0, 1.0);
      if (m_texture != NULL) 
         m_texture->useMipmap();
      else
         glDisable(GL_TEXTURE_2D);

      for (size_t i = 3; i < m_polygonVectors.getSize(); ++i) {
         PolygonVector* polies = m_polygonVectors[i];
         for (size_t j = 0; j < polies->getPolygonCount(); ++j) {
            glBegin(GL_POLYGON);
            for (size_t k = 0; k < i; ++k) {
               size_t index = polies->getVertexIndex(j, k);

               double nx = m_normals[3*index];
               double ny = m_normals[3*index+1];
               double nz = m_normals[3*index+2];
               glNormal3d(nx, ny, nz);
               if (m_texture != NULL) {
                  double u = m_texCoords[2*index];
                  double v = m_texCoords[2*index+1];
                  glTexCoord2d(u,v);
               }
               Point3D v = getVertex(index);
               v.glVertex();
            }
            glEnd();
         }
      }
   }

private:
   size_t m_vertexCount;
   bool m_deleteTextureWhenDone;
   Texture* m_texture;
   BGVector <double, BGVector_PolygonListBufferID> m_vertexCoords;
   BGVector <double, BGVector_PolygonListBufferID> m_texCoords;
   BGVector <double, BGVector_PolygonListBufferID> m_normals;
   BGVector <PolygonVector*, BGVector_BGVector_PolygonVectorID> m_polygonVectors;
};

#endif
