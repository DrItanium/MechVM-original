////////////////////////////////////////////////////////////////////////////////
// Mesh.cpp
// Class Implementation
// Copyright Bjoern Ganster 2001, 2002
////////////////////////////////////////////////////////////////////////////////

#include "Mesh.h"
#include "Matrix.h"
#include "FileCache.h"
#include "BGString.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Constructor

Mesh::Mesh (RenderableObject* _parent, const char* _name)
   : RenderableObject (MeshID, _parent, _name),
     texture (NULL),
     points (),
     polygons ()
{
   DefaultPolygonColor = Color (1.0, 1.0, 1.0);
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Mesh::~Mesh ()
{
   for (size_t i = 0; i < polygons.getSize(); i++)
   {
      MeshPolygon* poly = polygons[i];
      if (poly != NULL)
         delete poly;
   }

/*   for (PointSetIterator iter = points.begin ();
        iter < points.end ();
        iter++)
   {
      if (*iter != NULL)
         delete *iter;
   }*/

/*   for (size_t i = 0; i < points.getSize(); ++i)
      delete points[i];*/
}

////////////////////////////////////////////////////////////////////////////////
// Add a point

size_t Mesh::addPoint (double x, double y, double z)
{
   size_t base = points.getSize();
   points[base] = x;
   points[++base] = y;
   points[++base] = z;
   return base/3;
}

////////////////////////////////////////////////////////////////////////////////

bool Mesh::getPoint (size_t index, Point3D& p) const
{
   size_t base = 3*index;
   if (3*index < points.getSize()) {
      p.x = points[base];
      p.y = points[++base];
      p.z = points[++base];
      p.w = 1.0;
      return true; 
   } else
      return false;
}

void Mesh::setPoint (size_t index, Point3D& p) 
{
   size_t base = 3*index;
   points[base] = p.x;
   points[++base] = p.y;
   points[++base] = p.z;
}

////////////////////////////////////////////////////////////////////////////////
// Add polygon, use MeshPolygon::AddPoint to add points to it,
// finally, call MeshPolygon::FindOppesedEdges()

MeshPolygon* Mesh::addPolygon (size_t pointCount)
{
   MeshPolygon* newPoly = new MeshPolygon (pointCount, this);
   newPoly->num = polygons.getSize ();
   polygons.add (newPoly);
   //newPoly->color = DefaultPolygonColor;

   return newPoly;
}


////////////////////////////////////////////////////////////////////////////////
// Remove polygon, update polygon numbers

void Mesh::removePolygon (MeshPolygon* poly)
{
   size_t deletions = 0;
   size_t polycount = polygons.getSize ();
   size_t i = 0;
   bool DoContinue = true;

   while (DoContinue) {
      if (polygons[i] == poly) {
         deletions++;
      } else {
         i++;
      }
      
      DoContinue = i+deletions < polycount;
      
      if (deletions != 0 && DoContinue) {
         polygons[i] = polygons[i+deletions];
         polygons[i]->num = i;
      }
   }

   polygons.setSize (polycount - deletions);
   delete poly;
   assert (deletions == 1);
}

////////////////////////////////////////////////////////////////////////////////
// Remove point, update point numbers

/*void Mesh::removePoint (Point3D* p)
{
   // Delete all of p's triangles
   while (p->triangles.size () > 0) {
      removePolygon (p->triangles[0]);
   }

   // Remove all copies of p from points
   size_t deletions = 0;
   size_t pcount = points.getSize ();
   size_t i = 0; 
   while (i+deletions < pcount-1) {
      if (points[i] == p) {
         deletions++;
      } else {
         i++;
      }
      
      if (deletions != 0) {
         points[i] = points[i+deletions];
         //points[i]->num = i;
      }
   }
   //points.resize (pcount - deletions);
   
   // Delete p
   delete p;
}*/

////////////////////////////////////////////////////////////////////////////////
// Render a polygon set

void Mesh::renderSelf ()
{
   //activateParameters();

   if (visible) {
      bool textured;
      if (texture != NULL) {
         glEnable (GL_TEXTURE_2D);
         glEnableClientState (GL_TEXTURE_COORD_ARRAY);
         texture->use();
         textured = true;
      } else {
         glDisable (GL_TEXTURE_2D);
         textured = false;
      }

      glEnableClientState (GL_VERTEX_ARRAY);

      color.glColor();

      for (size_t i = 0; i < polygons.getSize(); ++i)
      {
         MeshPolygon& poly = *polygons[i];
         poly.render(textured);
      }

      glDisableClientState (GL_VERTEX_ARRAY);

      if (texture != NULL) {
         glDisable (GL_TEXTURE_2D);
         glDisableClientState (GL_TEXTURE_COORD_ARRAY);
      }
   }

   //deactivateParameters();
}

////////////////////////////////////////////////////////////////////////////////
// Get min/max mesh coords to decide about grid sizes

Point3D Mesh::getMinCoords () const 
{
   Point3D result, candidate;
   
   if (points.getSize() != 0) {
      getPoint(0, result);
      for (unsigned int i = 1; i < points.getSize(); i++) {
         getPoint(i, candidate);
         result = MinCoords (result, candidate);
      }
   }

   return result;
}

Point3D Mesh::getMaxCoords () const 
{
   Point3D result, candidate;
   
   if (points.getSize() != 0) {
      getPoint(0, result);
      for (unsigned int i = 1; i < points.getSize(); i++) {
         getPoint(i, candidate);
         result = MaxCoords (result, candidate);
      }
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// Mark front-facing polygons

void Mesh::markFrontFacingPolygons (const Point3D& ls)
{
   for (unsigned int i = 0; i < polygons.getSize (); i++) {
      MeshPolygon& poly = *polygons[i];
      size_t pi0 = poly.getPoint (0);
      size_t pi1 = poly.getPoint (1);
      size_t pi2 = poly.getPoint (2);
      Point3D p0, p1, p2, normal;
      if (getPoint(pi0, p0) && getPoint(pi1, p1) && getPoint(pi2, p2)) {
         Point3D dir1 = p0-p1;
         Point3D dir2 = p0-p2;
         dir1.unitLength ();
         dir2.unitLength ();
         Point3D normal = (dir1).crossMultiply (dir2);
      }
      if (normal * (ls-p0) > 0.0) {
         poly.mark = 1;
      } else {
         poly.mark = -1;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Render shadow volume

void Mesh::renderShadowVolumes (const Point3D& ls) const
{
/*   for (unsigned int i = 0; i < polygons.getSize (); i++) {
      MeshPolygon& poly = *polygons[i];
      for (unsigned int j = 0; j < poly.getPointCount (); j++) {
         //MeshPolygonEdge& edge = poly.getEdge (j);

         // A shadow polygon is created for this edge if either there is no
         // opposed edge or the edge and the opposed edge differ in lighting
         bool RenderShadowEdge = false;
         if (edge.opposedEdge != NULL) {
            if (poly.mark > edge.opposedEdge->poly->mark) {
               RenderShadowEdge = true;
            }
         } else {
            RenderShadowEdge = true;
         }

         if (RenderShadowEdge) {
            // p1, q1 represent the edge
            Point3D& p1 = *edge.point;
            Point3D& q1 = *poly.getPoint ((j+1) % poly.getPointCount ());

            // p2, q2 are the directions from ls to p1, q1, at infinity
            Point3D p2 = p1-ls;
            Point3D q2 = q1-ls;

            // Render shadow polygon
            glBegin (GL_QUADS);
               p1.glVertex ();
               q1.glVertex ();
               glVertex4d (q2.x, q2.y, q2.z, 0.0);
               glVertex4d (p2.x, p2.y, p2.z, 0.0);
            glEnd ();
         }
      }
   }*/
}

///////////////////////////////////////////////////////////////////////////////
// Load a mesh from a .obj

bool Mesh::loadFromObj (const char* FileName, Texture* textureToUse)
{
   if (!fileExists (FileName))
      return false;

   FileCache file;
   file.openReadOnly (FileName);
   bool KeepReading = file.getFileSize() > 0;

   if (!KeepReading)
      return false;

   // Read file
   FileOffset offset = 0;
   BGString line;
   file.readLine (offset, line);
   BGVector <double, BGVectorBufID> texCoords;

   while (KeepReading) {
      if (line.getLength() > 2) {
         if (line.getChars()[0] == 'v'
         &&  line.getChars()[1] == ' ') {
            // Read three coordinates from the file
            double x, y, z;
            BGString xStr, yStr, zStr;
            xStr.assignWord (line.getChars(), 1, ' ', line.getLength());
            yStr.assignWord (line.getChars(), 2, ' ', line.getLength());
            zStr.assignWord (line.getChars(), 3, ' ', line.getLength());

            if (xStr.toDouble(x) && yStr.toDouble(y) && zStr.toDouble(z))
               addPoint (x, y, z);
         } else 
         if (line.getChars()[0] == 'f'
         &&  line.getChars()[1] == ' ') 
         {
            // Count words
            size_t offset = 2;
            size_t pointCount = 0;
            while (offset < line.getLength()) {
               BGString num;
               offset = num.assignWord (line.getChars(), (int) pointCount+1, 
                                        ' ', line.getLength());
               size_t j = num.toInt();
               if (j > 0)
                  pointCount++;
            }

            // Set polygon points
            MeshPolygon* newPoly = addPolygon(pointCount);
            offset = 2;
            size_t word = 1;
            size_t pnum = 0;
            
            while (pnum < pointCount && word < line.getLength()) {
               bool DoContinue = false;
               BGString vertexData, vnum, tnum;
               offset = vertexData.assignWord (line.getChars(), (int) word, ' ', 
                                        line.getLength());
               vnum.assignWord(vertexData.getChars(), 0, '/', vertexData.getLength());
               tnum.assignWord(vertexData.getChars(), 1, '/', vertexData.getLength());
               unsigned int newV = vnum.toInt();
               word++;

               // Check if vertex number is legal
               if (newV <= points.getSize () && newV != 0) {
                  size_t pc = newPoly->getPointCount();
                  double u = pc % 2;
                  double v = 0;
                  size_t t = tnum.toInt()-1;
                  if (t < texCoords.getSize()) {
                     u = texCoords[2*t];
                     v = texCoords[2*t+1];
                  } else if (pc > 1) {
                     u = 1-u;
                     v = 1;
                  }
                  newPoly->setPoint(pnum, newV-1);
                  newPoly->setTexCoords(pnum, u, v);
                  pnum++;
               }
            }
         } else
         if (line.getChars()[0] == 'v'
         &&  line.getChars()[1] == 't') {
            BGString ustr, vstr;
            double u, v;
            ustr.assignWord(line.getChars(), 1, ' ', line.getLength());
            vstr.assignWord(line.getChars(), 2, ' ', line.getLength());
            ustr.toDouble(u);
            vstr.toDouble(v);
            texCoords.add(u);
            texCoords.add(v);
         }
      }

      KeepReading = (offset < file.getFileSize());
      if (KeepReading) {
         file.readLine (offset, line);
      } else {
         line = "#";
      }
   }

   calculateNormals ();
   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Calculate normals for polygons and vertices

void Mesh::calculateNormals ()
{
   // Calculate normals for polygons
   unsigned int i;
   for (i = 0; i < polygons.getSize (); i++) {
      bool found = false;
      MeshPolygon& poly = *polygons[i];
      Point3D normal (1.0, 0.0, 0.0);

      if (poly.getPointCount() > 2) {
         size_t pi0 = poly.getPoint (0);
         size_t pi1 = poly.getPoint (1);
         size_t pi2 = poly.getPoint (2);
         Point3D p0, p1, p2;
         if (getPoint(pi0, p0) && getPoint(pi1, p1) && getPoint(pi2,p2)) 
            normal = (p0-p1).crossMultiply (p0-p2);

         double length = normal.length ();
         if (length != 0.0) {
            normal = normal / length;
            found = true;
         }
      }

      poly.normal = normal;
   }

   // Calculate normals for the vertices from surrounding triangles
/*   for (i = 0; i < points.getSize (); i++) {
      Point3D& p = *points[i];
      p.normal = Point3D (0.0, 0.0, 0.0);
      for (j = 0; j < p.triangles.size (); j++) {
         p.normal = p.normal + p.triangles[j]->normal;
      }

      if (p.normal.length () == 0.0)
         p.normal = Point3D (1.0, 0.0, 0.0);
      else
         p.normal.UnitLength ();
   }*/
}

///////////////////////////////////////////////////////////////////////////////
// Save to obj

bool Mesh::saveToObj (const char* FileName, bool saveUVs)
{
   FileCache file;
   file.create(FileName);
   size_t uvNum = 1;

   // Write vertices
   unsigned int i, j;
   BGString line;
   for (i = 0; i < points.getSize (); ) {
      line = "v ";
      line.appendDouble(points[i++]);
      line += " ";
      line.appendDouble(points[i++]);
      line += " ";
      line.appendDouble(points[i++]);
      line += "\n";
      file.append(line.getChars(), line.getLength());
   }

   for (i = 0; i < polygons.getSize (); i++) {
      MeshPolygon& poly = *polygons[i];

      // Write UVs - optional
      if (saveUVs && poly.hasTextureCoords()) {
         for (j = 0; j < poly.getPointCount (); j++) {
            double u, v;
            poly.getTexCoords(j, u, v);
            line = "vt ";
            line.appendDouble (u);
            line += " ";
            line.appendDouble (v);
            line += "\n";
            file.append(line.getChars(), line.getLength());
         }
      }

      // Write faces
      line = "f";
      for (j = 0; j < poly.getPointCount (); j++) {
         line += " ";
         line.appendInt((int) (poly.getPoint (j)+1));
         if (saveUVs) {
            line += "/";
            line.appendInt (uvNum);
            uvNum++;
         }
      }
      line += "\n";
      file.append(line.getChars(), line.getLength());
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////
// Reduce polygons to triangles

// Not all algorithms can cope with polygons consisting of more than 3 vertices
// (that is, non-triangles). Therefore, this method reduces such polygons to 
// triangles. This is done by connecting all edges to the first vertex of the 
// polygon for polygons consisting of four points. For other polygons, a new
// point is created in the middle of the polygon and connected to the edges of 
// the polygon.

void Mesh::reducePolygonsToTriangles ()
{
/*   unsigned int i = 0;
   while (i < polygons.size ()) {
      MeshPolygon* poly = polygons[i];
      if (poly->getEdgeCount () == 4) {
         addTriangle (poly->getPoint (0)->num,
                      poly->getPoint (1)->num,
                      poly->getPoint (2)->num);
         addTriangle (poly->getPoint (0)->num,
                      poly->getPoint (2)->num,
                      poly->getPoint (3)->num);

         // Delete old triangle and remove it from the vector
         removePolygon (poly);
      } else if (poly->getEdgeCount () > 4) {
         // Compute center of polygon
         unsigned int j;
         Point3D center = *poly->getPoint (0);
         size_t polysize = poly->getEdgeCount ();
         for (j = 1; j < polysize; j++) {
            center = center + *poly->getPoint (j);
         }
         center = center / polysize;
         Point3D* centerpoint = addPoint (center.x, center.y, center.z);
         size_t centernum = centerpoint->num;

         // Create new triangles from polygon
         for (j = 0; j < polysize; j++) {
            unsigned int k = (j+1) % polysize;
            addTriangle (centernum, 
                         poly->getPoint (j)->num,
                         poly->getPoint (k)->num);
         }

         // Remove old triangle
         removePolygon (poly);
      } else {
         // Process next polygon
         i++;
      }
   }
   calculateNormals ();*/
}

///////////////////////////////////////////////////////////////////////////////
// Set per vertex lighting

void Mesh::setPerVertexLighting (bool newVal)
{
   for (unsigned int i = 0; i < polygons.getSize (); i++) {
      polygons[i]->PerVertexLighting = newVal;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Recursive Part of mesh size calculation

void Mesh::getSizeRecursive (Point3D& minCoords, Point3D& maxCoords, 
                             size_t& polygons, MatrixStack& m)
{
   for (size_t i = 0; i < points.getSize(); i++) {
      Point3D p;
      getPoint(i,p);
      p = m.top().Transform(p);
      minCoords = MinCoords (minCoords, p);
      maxCoords = MaxCoords (maxCoords, p);
   }
   polygons += this->polygons.getSize();
}

////////////////////////////////////////////////////////////////////////////////

void Mesh::calcCircumcircle ()
{
   Point3D min = points[0], mid = points[0], max = points[0];

   for (size_t i = 1; i < points.getSize(); ++i) {
      Point3D p = points[i];
      min.x = bgmin(min.x, p.x);
      min.y = bgmin(min.y, p.y);
      min.z = bgmin(min.z, p.z);

      max.x = bgmax(max.x, p.x);
      max.y = bgmax(max.y, p.y);
      max.z = bgmax(max.z, p.z);

      mid = mid + points[i];
   }

   Point3D mid1 = mid / points.getSize();
   Point3D mid2 = 0.5 *(min+max);
   double r1= (mid1-points[0]).length();
   double r2= (mid2-points[0]).length();

   for (size_t i = 1; i < points.getSize(); ++i) {
      double r1 = bgmin(r1, (mid1-points[i]).length());
      double r2 = bgmin(r2, (mid1-points[i]).length());
   }

   if (r1 < r2) {
      circumcircleCenter = mid1;
      circumcircleRadius = r1;
   } else {
      circumcircleCenter = mid2;
      circumcircleRadius = r2;
   }
}

void Mesh::getCircumcircle (Point3D& center, double& radius, const Matrix& M)
{
   center = M.Transform(circumcircleCenter);
   radius = circumcircleRadius;
}
