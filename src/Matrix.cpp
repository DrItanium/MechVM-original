////////////////////////////////////////////////////////////////////////////////
// Matrix.cpp
// Implementation of matrix operations
// Copyright Bjoern Ganster in 2001
////////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <assert.h>

#include "Matrix.h"

////////////////////////////////////////////////////////////////////////////////
// Construct unit matrix

Matrix::Matrix ()
: VirtualClass (MatrixID)
{   
   for (unsigned int i = 0; i < 16; i++)
      if (i % 4 == i / 4)
         m[i] = 1.0; // let diagonals contain 1
      else 
         m[i] = 0.0; // let other entries contain 0
}

////////////////////////////////////////////////////////////////////////////////
// Apply matrix to a point

Point3D Matrix::Transform (const Point3D& p) const
{
   return Point3D (p.x*m[ 0] + p.y*m[ 4] + p.z*m[ 8] + p.w*m[12],
                   p.x*m[ 1] + p.y*m[ 5] + p.z*m[ 9] + p.w*m[13],
                   p.x*m[ 2] + p.y*m[ 6] + p.z*m[10] + p.w*m[14],
                   p.x*m[ 3] + p.y*m[ 7] + p.z*m[11] + p.w*m[15]);
}

////////////////////////////////////////////////////////////////////////////////
// Matrix multiplication

Matrix Matrix::operator* (const Matrix& b) const
{
   // Looped version
   Matrix res;
   for (unsigned int x = 0; x < 4; x++)
      for (unsigned int y = 0; y < 4; y++) {
         res.m[x*4+y] = 0.0;
         for (unsigned int z = 0; z < 4; z++)
            res.m[x*4+y] += m[z*4+y]*b.m[x*4+z];
      }

   return res;
}

////////////////////////////////////////////////////////////////////////////////
// Assignment operators

void Matrix::operator = (const Matrix& M)
{
   for (unsigned int i = 0; i < 16; i++)
      m[i] = M.m[i];
}

void Matrix::operator = (const double* d)
{
   for (unsigned int i = 0; i < 16; i++)
      m[i] = d[i];
}

////////////////////////////////////////////////////////////////////////////////
// Get render matrix stack (singleton)

MatrixStack* renderMatrixStack = 0;

MatrixStack* MatrixStack::getRenderMatrixStack ()
{
   if (renderMatrixStack == NULL) {
      renderMatrixStack = new MatrixStack();
   }

   return renderMatrixStack;
}

////////////////////////////////////////////////////////////////////////////////
// Load a translation matrix

TranslationMatrix::TranslationMatrix (double x, double y, double z)
{
   m[ 0] = 1.0; m[ 4] = 0.0; m[ 8] = 0.0; m[12] = x;
   m[ 1] = 0.0; m[ 5] = 1.0; m[ 9] = 0.0; m[13] = y;
   m[ 2] = 0.0; m[ 6] = 0.0; m[10] = 1.0; m[14] = z;
   m[ 3] = 0.0; m[ 7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}

TranslationMatrix::TranslationMatrix (const Point3D& p)
{
   m[ 0] = 1.0; m[ 4] = 0.0; m[ 8] = 0.0; m[12] = p.x;
   m[ 1] = 0.0; m[ 5] = 1.0; m[ 9] = 0.0; m[13] = p.y;
   m[ 2] = 0.0; m[ 6] = 0.0; m[10] = 1.0; m[14] = p.z;
   m[ 3] = 0.0; m[ 7] = 0.0; m[11] = 0.0; m[15] = 1.0;
}

////////////////////////////////////////////////////////////////////////////////
// Load scaling matrix

ScaleMatrix::ScaleMatrix (double sx, double sy, double sz, double sw)
{
   m[ 0] =  sx; m[ 3] = 0.0; m[ 8] = 0.0; m[12] = 0.0;
   m[ 1] = 0.0; m[ 5] =  sy; m[ 9] = 0.0; m[13] = 0.0;
   m[ 2] = 0.0; m[ 6] = 0.0; m[10] =  sz; m[14] = 0.0;
   m[ 3] = 0.0; m[ 7] = 0.0; m[11] = 0.0; m[15] =  sw;
}

////////////////////////////////////////////////////////////////////////////////
// Load perspective projection matrix

GLUPerspectiveMatrix::GLUPerspectiveMatrix (double nearClippingPlane, 
                                            double farClippingPlane, 
                                            double tanFOVY)
{
   // This is how it's done in GLU
   // Let's assume aspect to be 1.0, meaning that width == height
   // (This makes sense for HemiPlane/HemiCube pictures, because the x/y axes
   // are more or less arbitrary)
   double deltaZ = farClippingPlane - nearClippingPlane;
   double cotan = 1.0 / tanFOVY;

   m[ 0] = cotan; // cotan / aspect;
   m[ 5] = cotan;
   m[10] = -(farClippingPlane + nearClippingPlane) / deltaZ;
   m[11] = -1.0;
   m[14] = -2.0 * nearClippingPlane * farClippingPlane / deltaZ;
   m[15] = 0.0;
}

////////////////////////////////////////////////////////////////////////////////
// Load rotation matrix 
// See Graphics Gems by Andrew S. Glassner, p. 466

RotationMatrix::RotationMatrix (double angle, double x, double y, double z)
{
   double s = (double) sin(angle);
   double c = (double) cos(angle);
   double t = 1 - c;

   m[ 0] = t*x*x+c;   m[ 4] = t*x*y+s*z; m[ 8] = t*x*z-s*y; m[12] = 0.0;
   m[ 1] = t*x*y-s*z; m[ 5] = t*y*y+c;   m[ 9] = t*y*z+s*x; m[13] = 0.0;
   m[ 2] = t*x*z+s*y; m[ 6] = t*y*z-s*x; m[10] = t*z*z+c;   m[14] = 0.0;
   m[ 3] = 0.0;       m[ 7] = 0.0;       m[11] = 0.0;       m[15] = 1.0;
}

////////////////////////////////////////////////////////////////////////////////
// LookDirs
// ViewingDir, UpDir are assumed to be unit length and orthogonal to each other

LookDirsMatrix::LookDirsMatrix (const Point3D& CameraPos, 
                    const Point3D& ViewingDir, 
                    const Point3D& UpDir)
{
   // We need a third vector, orthogonal to ViewingDir and UpDir
   Point3D side = ViewingDir.crossMultiply (UpDir);
   side.unitLength ();

   // We specify the transpose of the following matrix:
   // (1, 0, 0) becomes side,
   // (0, 1, 0) becomes UpDir,
   // (0, 0, 1) becomes ViewingDir
   // Since this matrix contains vectors that are orthogonal and that have
   // unit length, the matrix is orthonormal. Therefore, the transpose matrix
   // is the inverse matrix of the one described above.
   Matrix R; // rotational matrix
   R.m[ 0] = side.x;        R.m[ 4] = side.y;        R.m[ 8] = side.z;
   R.m[ 1] = UpDir.x;       R.m[ 5] = UpDir.y;       R.m[ 9] = UpDir.z;
   R.m[ 2] = -ViewingDir.x; R.m[ 6] = -ViewingDir.y; R.m[10] = -ViewingDir.z;

   // Translate into CameraPos
   TranslationMatrix T (-CameraPos);
   ((Matrix*) this)->operator= (R*T);
}

////////////////////////////////////////////////////////////////////////////////
// New version with ViewCenter

GluLookAtMatrix::GluLookAtMatrix (const Point3D& CameraPos, 
                                  const Point3D& ViewCenter, 
                                  const Point3D& UpDir)
{
   // We need a third vector, orthogonal to ViewingDir and UpDir
   Point3D dist = ViewCenter - CameraPos;
   Point3D distU = dist;
   distU.unitLength();
   Point3D UpDirU = UpDir;
   UpDirU.unitLength();
   Point3D side = distU.crossMultiply (UpDirU);
   UpDirU = side.crossMultiply (distU);

   // We specify the transpose of the following matrix:
   // (1, 0, 0) becomes side,
   // (0, 1, 0) becomes UpDir,
   // (0, 0, 1) becomes ViewingDir
   // Since this matrix contains vectors that are orthogonal and that have
   // unit length, the matrix is orthonormal. Therefore, the transpose matrix
   // is the inverse matrix of the one described above.
   Matrix R; // rotational matrix
   R.m[ 0] = side.x;   R.m[ 4] = side.y;   R.m[ 8] = side.z;
   R.m[ 1] = UpDirU.x; R.m[ 5] = UpDirU.y; R.m[ 9] = UpDirU.z;
   R.m[ 2] = -distU.x; R.m[ 6] = -distU.y; R.m[10] = -distU.z;

   // Translate into CameraPos
   TranslationMatrix T (-CameraPos);
   ((Matrix*) this)->operator= (R*T);
}

////////////////////////////////////////////////////////////////////////////////
// Load a GL matrix

MatrixFromOpenGL ::MatrixFromOpenGL (GLenum matrix)
{
   glGetDoublev (matrix, m);
}
