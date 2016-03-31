////////////////////////////////////////////////////////////////////////////////
// Matrix.h
// Header for matrix operations
// Copyright Björn Ganster in 2001
////////////////////////////////////////////////////////////////////////////////

#ifndef Matrix__h
#define Matrix__h

#include <vector>
#include <list>
#include <algorithm>
using namespace std;

#include "Point3D.h"

////////////////////////////////////////////////////////////////////////////////
// Class Matrix, the base class of all matrices (descendants differ only in 
// constructors offered)

class Matrix: public VirtualClass{
public:
   double m[16];

   // Construct unit matrix
   Matrix ();

   // Copy constructor
   inline Matrix (const Matrix& m)
      : VirtualClass (MatrixID)
   { operator= (m); }

   // Construct from double array, assigning NULL will reset matrix
   inline Matrix (const double* d)
      : VirtualClass (MatrixID)
   { operator= (d); }

   // Apply matrix to a point
   Point3D Transform (const Point3D& p) const;

   // Pass matrix to OpenGL
   inline void glLoadMatrix () const
   { glLoadMatrixd (m); }
   inline void glMultMatrix () const
   { glMultMatrixd (m); }

   // Get matrix from OpenGL, parameter matrix is usually GL_MODELVIEW_MARIX
   inline void glGetMatrix (int matrix) 
   { glGetDoublev (matrix, m); }

   // Matrix multiplication
   Matrix operator* (const Matrix& m) const;

   // Assignment operators
   void operator = (const Matrix& M);
   void operator = (const double* d);
};

////////////////////////////////////////////////////////////////////////////////
// Simple Matrix stack

class MatrixStack: public VirtualClass {
private:
   list <Matrix> matrices;
public:
   inline MatrixStack ()
      : VirtualClass (MatrixStackID)
   { clear (); }

   // Clear Stack
   inline void clear()
   { 
      matrices.clear (); 
      // Push identity
      Matrix id;
      matrices.push_back (id);
   }

   // Push a matrix onto the stack
   inline void push (const Matrix& m)
   { matrices.push_back (m); }

   // Multiplies the current top with m and pushes the new Matrix onto the stack
   inline void multiplyMatrix (const Matrix& M)
   {
      Matrix newTop = top () * M;
      matrices.push_back (newTop);
   }

   // Pop operation
   inline void pop ()
   { matrices.pop_back(); }

   // Get the top matrix on the stack
   inline const Matrix& top ()
   { return matrices.back(); }

   // Get render matrix stack (singleton)
   static MatrixStack* getRenderMatrixStack ();
};

////////////////////////////////////////////////////////////////////////////////
// Load a translation matrix

class  TranslationMatrix: public Matrix {
public: 
   TranslationMatrix (const Point3D& p);
   TranslationMatrix (double x, double y, double z);
};

////////////////////////////////////////////////////////////////////////////////
// Load scaling matrix

class  ScaleMatrix: public Matrix {
public: ScaleMatrix (double sx, double sy, double sz, double sw);
};

////////////////////////////////////////////////////////////////////////////////
// Load perspective projection matrix

class  GLUPerspectiveMatrix: public Matrix {
public: GLUPerspectiveMatrix (double nearClippingPlane, 
                              double farClippingPlane, double tanFOVY);

};

////////////////////////////////////////////////////////////////////////////////
// Load rotation matrix 

class  RotationMatrix: public Matrix {
public: RotationMatrix (double angle, double axeX, double axeY, double axeZ);
};

////////////////////////////////////////////////////////////////////////////////
// LookDirs
// ViewingDir, UpDir are assumed to be unit length and orthogonal to each other

class LookDirsMatrix: public Matrix {
public:
   LookDirsMatrix (const Point3D& CameraPos, 
      const Point3D& ViewingDir, 
      const Point3D& UpDir);
};

////////////////////////////////////////////////////////////////////////////////
// Load GLU look at matrix

class  GluLookAtMatrix: public Matrix {
public: GluLookAtMatrix (const Point3D& CameraPos, const Point3D& ViewingDir, 
                         const Point3D& UpDir);
};

////////////////////////////////////////////////////////////////////////////////
// Load a GL matrix

class  MatrixFromOpenGL: public Matrix {
public: MatrixFromOpenGL (GLenum matrix);
};

#endif
