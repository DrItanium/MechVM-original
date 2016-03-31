////////////////////////////////////////////////////////////////////////////////
// Implementation of Mesh2
// Base class for using vertex arrays
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "Mesh2.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

Mesh2::Mesh2 (RenderableObject* parent)
: RenderableObject (Mesh2ID, parent),
  color (1, 1, 1),
  pointCount (0),
  pointReserve (0),
  points (NULL),
  triangles (NULL),
  //normalCount (0),
  quads (NULL),
  normals (NULL),
  texCoords (NULL),
  triangleCount (0),
  triangleReserve (0),
  quadCount (0),
  quadReserve (0),
  //polygons (NULL),
  texture (NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Mesh2::~Mesh2 ()
{
   traceFree (points, Mesh2PointsID);
   traceFree (normals, Mesh2NormalsID);
   traceFree (triangles, Mesh2TrianglesID);
   traceFree (quads, Mesh2TrianglesID);
   traceFree (texCoords, Mesh2TexCoordsID);
   //delete polygons;
}

////////////////////////////////////////////////////////////////////////////////
// Render entire scene

void Mesh2::renderSelf ()
{
   // Shiny fast new glDraw code
   glEnableClientState (GL_VERTEX_ARRAY);
   glEnableClientState (GL_NORMAL_ARRAY);
   glVertexPointer (pointComponents, GL_DOUBLE, 0, points);
   glNormalPointer (GL_DOUBLE, 0, normals);

   if (texture != NULL) {
      glEnable(GL_TEXTURE_2D);
      texture->use();
      glEnableClientState (GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer (2, GL_DOUBLE, 0, texCoords);
      glColor3d (1, 1, 1);
   } else {
      color.glColor();
   }
   if (triangleCount > 0) 
      glDrawElements (GL_TRIANGLES, 3*triangleCount, GL_UNSIGNED_INT, triangles);
   if (quadCount > 0) 
      glDrawElements (GL_QUADS, 4*quadCount, GL_UNSIGNED_INT, quads);

   // Old slow glVertex code
   //color.glColor();
   /*glColor3d(1, 1, 1);
   glEnable(GL_TEXTURE_2D);
   glDisableClientState (GL_NORMAL_ARRAY);
   texture->use();
   glBegin (GL_QUADS);
   for (size_t i = 0; i < quadCount; i++) {
      size_t j = 3*quads[i*4];
      Point3D p (points[j], points[j+1], points[j+2]);
      Point3D n (normals[j], normals[j+1], normals[j+2]);
      n.glNormal();
      p.glVertex();

      j = 3*quads[i*4+1];
      p = Point3D (points[j], points[j+1], points[j+2]);
      n = Point3D (normals[j], normals[j+1], normals[j+2]);
      n.glNormal();
      p.glVertex();

      j = 3*quads[i*4+2];
      p = Point3D (points[j], points[j+1], points[j+2]);
      n = Point3D (normals[j], normals[j+1], normals[j+2]);
      n.glNormal();
      p.glVertex();

      j = 3*quads[i*4+3];
      p = Point3D (points[j], points[j+1], points[j+2]);
      n = Point3D (normals[j], normals[j+1], normals[j+2]);
      n.glNormal();
      p.glVertex();
   }
   glEnd ();*/

   renderChilds();
}

////////////////////////////////////////////////////////////////////////////////
// Add points

size_t Mesh2::addPoint (double x, double y, double z, 
                        double nx, double ny, double nz,
                        double u, double v)
{
   if (pointCount >= pointReserve) {
      pointReserve = bgmax (4, 2 * pointReserve);
      size_t memNeeded = pointComponents * pointReserve * sizeof (double);
      double* newPoints = (double*) (traceAlloc (memNeeded, Mesh2PointsID));
      double* newNormals = (double*) (traceAlloc (memNeeded, Mesh2NormalsID));
      if (points != NULL && newPoints != NULL && pointCount != 0) {
         size_t oldMem = pointComponents * pointCount * sizeof (double);
         memcpy (newPoints, points, oldMem);
         memcpy (newNormals, normals, oldMem);
         traceFree (points, Mesh2PointsID);
         traceFree (normals, Mesh2NormalsID);
      }
      memNeeded = 2*pointReserve*sizeof(double);
      double* newTexCoords = (double*) (traceAlloc (memNeeded, Mesh2TexCoordsID));
      if (texCoords != NULL) {
         memcpy (newTexCoords, texCoords, 2*pointCount*sizeof(double));
         traceFree(texCoords, Mesh2TexCoordsID);
      }
      points = newPoints;
      normals = newNormals;
      texCoords = newTexCoords;
   }

   size_t index = pointComponents * pointCount;
   points[index] = x;
   points[index+1] = y;
   points[index+2] = z;
   //points[index+3] = w;

   normals[index] = nx;
   normals[index+1] = ny;
   normals[index+2] = nz;

   index = 2* pointCount;
   texCoords[index++] = u;
   texCoords[index]   = v;
   return pointCount++;
}

////////////////////////////////////////////////////////////////////////////////
// Add triangles

void Mesh2::addTriangle (size_t p1, size_t p2, size_t p3)
{
   if (triangleCount >= triangleReserve) {
      triangleReserve = bgmax (3, 2 * triangleReserve);
      size_t memNeeded = 3 * triangleReserve * sizeof(unsigned int);
      GLsizei* newTriangles = 
         (GLsizei*) (traceAlloc (memNeeded, Mesh2TrianglesID));
      if (triangles != NULL && newTriangles != NULL && triangleCount > 0) {
         size_t oldMem = 3 * triangleCount * sizeof(unsigned int);
         memcpy (newTriangles, triangles, oldMem);
      }

      triangles = newTriangles;
   }

   size_t index = 3 * triangleCount;
   triangles[index] = (GLsizei) p1;
   triangles[index+1] = (GLsizei) p2;
   triangles[index+2] = (GLsizei) p3;
   triangleCount++;
}

////////////////////////////////////////////////////////////////////////////////
// Add quads

size_t Mesh2::addQuad (size_t p1, size_t p2, size_t p3, size_t p4)
{
   if (quadCount >= quadReserve) {
      quadReserve = bgmax ((GLsizei) 4, 2 * quadReserve);
      size_t memNeeded = 4 * quadReserve * sizeof(unsigned int);
      GLsizei* newQuads = 
         (GLsizei*) (traceAlloc (memNeeded, Mesh2TrianglesID));
      if (quads != NULL && newQuads != NULL && quadCount > 0) {
         size_t oldMem = 4 * quadCount * sizeof(unsigned int);
         memcpy (newQuads, quads, oldMem);
      }

      quads = newQuads;
   }

   size_t index = 4 * quadCount;
   quads[index] = (GLsizei) p1;
   quads[index+1] = (GLsizei) p2;
   quads[index+2] = (GLsizei) p3;
   quads[index+3] = (GLsizei) p4;
   quadCount++;
   return quadCount-1;
}

////////////////////////////////////////////////////////////////////////////////
// Add polygons

void Mesh2::startNewPolygon()
{
}

void Mesh2::addToPolygon (size_t p1, const Point3D& n1)
{
}

////////////////////////////////////////////////////////////////////////////////
// Query triangles, quads

bool Mesh2::getTriangle (GLsizei num, size_t& p0, size_t& p1, size_t& p2)
{
   if (num < triangleCount) {
      size_t index = 3*num;
      p0 = triangles[index];
      p1 = triangles[++index];
      p2 = triangles[++index];
      return true;
   } else
      return false;
}

bool Mesh2::getQuad (GLsizei num, size_t& p0, size_t& p1, size_t& p2, size_t& p3)
{
   if (num < quadCount) {
      size_t index = 4*num;
      p0 = quads[index];
      p1 = quads[++index];
      p2 = quads[++index];
      p3 = quads[++index];
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Add all twig polygons to another container, useful for saving hierarchies

void Mesh2::gatherPolygons (Mesh2* other)
{
   // Copy points
   size_t pointBase = other->getPointCount();
   for (GLsizei i = 0; i < pointCount; i++) {
      Point3D p = getPoint(i);
      double u, v;
      getTexCoords (i, u, v);
      other->addPoint(p, Point3D(normals[3*i], normals[3*i+1], normals[3*i+2]), u, v);
   }

   // Copy triangles
   for (GLsizei i = 0; i < triangleCount; i++) {
      other->addTriangle(pointBase+triangles[3*i], pointBase+triangles[3*i+1],
                         pointBase+triangles[3*i+2]);
   }

   // Copy quads
   for (GLsizei i = 0; i < quadCount; i++) {
      other->addQuad(pointBase+quads[4*i], pointBase+quads[4*i+1],
                     pointBase+quads[4*i+2], pointBase+quads[4*i+3]);
   }

   for (size_t i = 0; i < getObjectCount(); i++)
      ((Mesh2*)getObject(i))->gatherPolygons(other);
}

////////////////////////////////////////////////////////////////////////////////
// Save as obj

void Mesh2::saveAsObj(const TCHAR* FN)
{
   //ofstream file (FileName);
   FileCache file;
   file.create(FN);

   // Write vertices
   BGString line;
   for (GLsizei i = 0; i < pointCount; i++) {
      line = "v ";
      line.appendDouble(points[3*i]);
      line += " ";
      line.appendDouble(points[3*i+1]);
      line += " ";
      line.appendDouble(points[3*i+2]);
      line += "\nvt ";
      line.appendDouble(texCoords[2*i]);
      line += " ";
      line.appendDouble(texCoords[2*i+1]);
      line += "\n";
      file.append(line.getChars(), line.getLength());
   }

   // Write triangles
   for (GLsizei i = 0; i < triangleCount; i++) {
      line = "f ";
      for (size_t j = 0; j < 3; j++) {
         GLsizei vn = triangles[3*i+j]+1;
         line.appendInt(vn);
         line +="/";
         line.appendInt(vn);
         line += " ";
      }
      line += "\n";
      file.append(line.getChars(), line.getLength());
   }

   // Write quads
   for (GLsizei i = 0; i < quadCount; i++) {
      line = "f ";
      for (size_t j = 0; j < 4; j++) {
         GLsizei vn = quads[4*i+j]+1;
         line.appendInt(vn);
         line +="/";
         line.appendInt(vn);
         line += " ";
      }
      line += "\n";
      file.append(line.getChars(), line.getLength());
   }
}
