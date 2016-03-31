////////////////////////////////////////////////////////////////////////////////
// Class Point3D
// Copyright Bjoern Ganster 2000-2001
////////////////////////////////////////////////////////////////////////////////

#ifndef Point3D__h
#define Point3D__h

#include "BGBase.h"

#include <algorithm>
using namespace std;

#include <math.h>
#include <GL/gl.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

////////////////////////////////////////////////////////////////////////////////
// Class Point3D

class Point3D: public VirtualClass {
   public:
      double x, y, z, w;

      // Constructors
      Point3D (double _x = 0.0, double _y = 0.0, double _z = 0.0, double _w = 1.0);
      Point3D (const Point3D& a);

      // Destructors
      virtual ~Point3D()
      {}

      // Unary -
      inline Point3D operator - () const
      { return Point3D (-x, -y, -z, w); }
      
      // Binary +
      inline Point3D operator + (const Point3D& a) const
      {
         if (w == 1.0 && a.w == 1.0) {
            return Point3D (x+a.x, y+a.y, z+a.z, 1.0);
         } else if (w != 0.0 && a.w != 0.0) {
            return Point3D ((x / w) + (a.x / a.w), (y / w) + (a.y / a.w), 
                            (z / w) + (a.z / a.w), 1.0); 
         } else if (w == 0.0) {
            return Point3D (x, y, z, w);
         } else if (a.w == 0.0) {
            return Point3D (a);
         } else {
            return Point3D (x+a.x, y+a.y, z+a.z, 0.0);
         }
//         return Point3D (x+a.x, y+a.y, z+a.z, 1.0);
      }

      // Binary -
      inline Point3D operator - (const Point3D& a) const
      { 
         if (w == 1.0 && a.w == 1.0) {
            return Point3D (x-a.x, y-a.y, z-a.z, 1.0);
         } else if (w != 0.0 && a.w != 0.0) {
            return Point3D ((x / w) - (a.x / a.w), (y / w) - (a.y / a.w), 
                            (z / w) - (a.z / a.w), 1.0); 
         } else if (w == 0.0) {
            return Point3D (x, y, z, w);
         } else if (a.w == 0.0) {
            return -a;
         } else {
            return Point3D (x-a.x, y-a.y, z-a.z, 0.0);
         }
//         return Point3D (x+a.x, y+a.y, z+a.z, 1.0);
      }

      // Cross product
      inline Point3D crossMultiply (const Point3D& a) const
      {
         Point3D result;
         result.x =   y*a.z - z*a.y;
         result.y = - x*a.z + z*a.x;
         result.z =   x*a.y - y*a.x;
         result.w = (w + a.w) / 2;
         return result;
      }

      // Binary scalar product
      inline double operator * (const Point3D& a) const
      { return (x*a.x + y*a.y + z*a.z) / (w*a.w); } 

      // Scalar multiplication
      inline friend Point3D operator * (double f, const Point3D& p)
      { return Point3D (p.x*f, p.y*f, p.z*f, p.w); }

      inline friend Point3D operator * (const Point3D& p, double f)
      { return Point3D (p.x*f, p.y*f, p.z*f, p.w); }

      // Divide a vector by scalar
      inline Point3D operator / (double a) const
      { return Point3D (x/a, y/a, z/a, w); }

      // Compare
      inline bool operator == (const Point3D& a) const
      { 
         if (w == 0.0) {
            if (a.w != 0.0)
               return false;
            else 
               return x == a.x && y == a.y && z == a.z;      
         } else {
            return x/w == a.x/a.w && y/w == a.y/a.w && z/w == a.z/a.w; 
         }
      }

      // Compare
      inline bool operator != (const Point3D& a) const
      { return !operator == (a); }
      
      // Assignment operator
      inline void operator= (const Point3D& a)
      {
         x = a.x;
         y = a.y;
         z = a.z;
         w = a.w;
      }

      // Increment operator
      inline void operator += (const Point3D& a)
      {
         x += a.x;
         y += a.y;
         z += a.z;
      }
      
      // Calculate a vector's length / distance to origin
      inline double length () const
      { 
         if (w != 0.0 && w != 1.0)
            return sqrt (x*x + y*y + z*z) / w;
         else
            return sqrt (x*x + y*y + z*z);
      }

      // Make itself unit length
      inline bool unitLength ()
      {
         double l = length ();
      	if (l != 0) {
          	x /= l;
            y /= l;
            z /= l;
            w = 1.0;
            return true;
         } else
            return false;
      }

      // Apply as vertex
      inline void glVertex () const
      //{ glVertex4d (x, y, z, w); } // ???
      { glVertex3d (x, y, z); }

      // Apply as normal
      inline void glNormal () const
      { glNormal3d (x, y, z); }
      
      // Debug output
//      inline void debugPrint (char* text) const
//      { cout << text << "(" << x << ", " << y << ", " << z << ")" << endl; }
      
      // Calculate the angle between ab, bc
      inline static double angle (const Point3D& a, const Point3D& b, const Point3D& c)
      { return angle (a-b, c-b); }
      static double angle (const Point3D& a, const Point3D& b);

      // Find two vectors that are pependicular to self
      bool findPerps (Point3D& b, Point3D& c) const;

   inline void copy3DCoordsToArray (double* array) const
   {
      array[0] = x;
      array[1] = y;
      array[2] = z;
   }
   inline void copy3DCoordsToArray (float* array) const
   {
      array[0] = (float) x;
      array[1] = (float) y;
      array[2] = (float) z;
   }
};

////////////////////////////////////////////////////////////////////////////////
// Class Point3

template <class T>
class Point3: public VirtualClass {
public:
   T x, y, z;
   
   // Constructor
   Point3 (T _x = 0.0, T _y = 0.0, T _z = 0.0)
      : VirtualClass (Point3f_ID),
        x (_x), y (_y), z (_z)
   {}
   Point3 (const Point3D& p)
      : VirtualClass (Point3f_ID),
        x ((T) p.x), y ((T) p.y), z ((T) p.z)
   {}
   
   // Destructor
   virtual ~Point3()
   {}

   // Assign
   void operator= (const Point3<float>& b)
   {
      x = b.x;
      y = b.y;
      z = b.z;
   }
   void operator= (const Point3<double>& b)
   {
      x = b.x;
      y = b.y;
      z = b.z;
   }
   void operator= (const Point3D& b)
   {
      x = float (b.x / b.w);
      y = float (b.y / b.w);
      z = float (b.z / b.w);
   }

   inline void operator+= (const Point3<T>& p)
   {
      x += p.x; y += p.y; z += p.z;
   }

   // Binary +
   inline Point3<T> operator + (const Point3<T>& a) const
   {
      return Point3<T> (x+a.x, y+a.y, z+a.z);
   }

   // Binary -
   inline Point3<T> operator - (const Point3<T>& a) const
   {
      return Point3<T> (x-a.x, y-a.y, z-a.z);
   }

   // Calculate a vector's length / distance to origin
   inline float length () const
   { return sqrt (x*x + y*y + z*z); }

   // Make itself unit length
   inline bool unitLength ()
   {
      float l = length ();
   	if (l != 0) {
       	x /= l;
         y /= l;
         z /= l;
         return true;
      } else
         return false;
   }

   inline void copy3DCoordsToArray (T* array) const
   {
      array[0] = x;
      array[1] = y;
      array[2] = z;
   }

   // Cross product
   inline Point3<T> crossMultiply (const Point3<T>& a) const
   {
      Point3<T> result;
      result.x =   y*a.z - z*a.y;
      result.y = - x*a.z + z*a.x;
      result.z =   x*a.y - y*a.x;
      return result;
   }
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class Point3v: public VirtualClass {
public:
   // Constructor. Point3v does not delete m_data when done. If needed, call
   // deleteData() instead
   Point3v (T* data, T x = 0.0, T y = 0.0, T z = 0.0)
   {
      m_data = data;
      m_data[0] = x;
      m_data[1] = y;
      m_data[2] = z;
   }

   // Destructor. Point3v does not delete m_data when done. If needed, call
   // deleteData() instead
   ~Point3v()
   {
   }

   void useData (double* p)
   { m_data = p; }

   // Delete data
   void deleteData()
   {
      delete m_data;
      m_data = NULL;
   }

   // Get component
   T getX() const
   { return m_data[0]; }
   T getY() const
   { return m_data[1]; }
   T getZ() const
   { return m_data[2]; }

   // Set component
   void setX (T newVal)
   { m_data[0] = newVal; }
   void setY (T newVal)
   { m_data[1] = newVal; }
   void setZ (T newVal)
   { m_data[2] = newVal; }

   // Set from Point3D
   void operator = (const Point3D& p)
   {
      m_data[0] = p.x;
      m_data[1] = p.y;
      m_data[2] = p.z;
   }

   // Get as Point3D
   void get (Point3D& p) const
   {
      p.x = m_data[0];
      p.y = m_data[1];
      p.z = m_data[2];
   }
private:
   T* m_data;
};

////////////////////////////////////////////////////////////////////////////////
// Point comparison

inline Point3D MinCoords (const Point3D& a, const Point3D& b)
{ 
   Point3D minRes (a);
   if (a.x > b.x)
      minRes.x = b.x;
   if (a.y > b.y)
      minRes.y = b.y;
   if (a.z > b.z)
      minRes.z = b.z;
   return minRes;
}

inline Point3D MaxCoords (const Point3D& a, const Point3D& b)
{ 
   Point3D maxRes (a);
   if (a.x < b.x)
      maxRes.x = b.x;
   if (a.y < b.y)
      maxRes.y = b.y;
   if (a.z < b.z)
      maxRes.z = b.z;
   return maxRes;
}

////////////////////////////////////////////////////////////////////////////////
// Predefined, useful points

const Point3D NullVect (0.0, 0.0, 0.0);
const Point3D xVect (1.0, 0.0, 0.0);
const Point3D yVect (0.0, 1.0, 0.0);
const Point3D zVect (0.0, 0.0, 3.0);

////////////////////////////////////////////////////////////////////////////////

inline double dist3D (const Point3D& a, const Point3D& b)
{
   if (a.w == 1.0 && b.w == 1.0) {
      // No homogeneous division required
      double dx = a.x - b.x;
      double dy = a.y - b.y;
      double dz = a.z - b.z;
      return sqrt(dx*dx + dy*dy + dz*dz);
   } else if (a.w == 0.0 || b.w == 0.0) {
      // Assume all directions to be at no distance ...
      return 0.0;
   } else {
      // Homogeneous division required
      double dx = (a.x/a.w) - (b.x/b.w);
      double dy = (a.y/a.w) - (b.y/b.w);
      double dz = (a.z/a.w) - (b.z/b.w);
      return sqrt(dx*dx + dy*dy + dz*dz);
   }
}

#endif
