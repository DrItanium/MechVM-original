////////////////////////////////////////////////////////////////////////////////
// Point3D implementation
// Copyright Bjoern Ganster 2000-2011

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <assert.h>

#include "math.h"
#include "Point3D.h"

////////////////////////////////////////////////////////////////////////////////
// Default constructor

Point3D::Point3D (double _x, double _y, double _z, double _w)
: VirtualClass (Point3DID)
{ 
   x = _x;
	y = _y;
	z = _z;
   w = _w;
}

////////////////////////////////////////////////////////////////////////////////
// Copy constructor

Point3D::Point3D (const Point3D& a)
: VirtualClass (Point3DID)
{
   x = a.x;
	y = a.y;
	z = a.z;
   w = a.w;
}

////////////////////////////////////////////////////////////////////////////////
// Calculate the angle between ab, bc

double Point3D::angle (const Point3D& a, const Point3D& b)
{
   double alen = a.length ();
   double blen = b.length ();
   if (alen != 0.0 && blen != 0.0) {
      double fraction = (a * b) / (alen * blen);
      return acos (putInRange (-1.0, fraction, 1.0));
   } else {
      return 0.0;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Find two vectors that are pependicular to a given vector

bool Point3D::findPerps (Point3D& b, Point3D& c) const
{
   double len = length();
   if (len == 0)
      return false;

   Point3D a1 = (*this);
   a1 = a1/len;
   b = Point3D (1, 0, 0);
   c = Point3D (0, 1, 0);

   // Choose the vector with the bigger absolute difference to a
   double abdist = min (Point3D(a1-b).length(), Point3D(a1+b).length());
   double acdist = min (Point3D(a1-c).length(), Point3D(a1+c).length());

   if (abdist > acdist) {
      // Base c on cross product of a, b
      c = a1.crossMultiply (b);
      c.unitLength();
      b = a1.crossMultiply (c);
   } else {
      // Base c on cross product of a, b
      b = a1.crossMultiply (c);
      b.unitLength();
      c = a1.crossMultiply (b);
   }

   return true;
}
