////////////////////////////////////////////////////////////////////////////////
// intersections.cpp
// Implementation of intersection tests
// Copyright Bjoern Ganster in 2001
////////////////////////////////////////////////////////////////////////////////

#include "intersections.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// OneDimObject

OneDimObject::OneDimObject ()
{
}

OneDimObject::OneDimObject (const Point3D& _p, const Point3D& _q)
{
   p = _p;
   q = _q;
   dir = q-p;
   assert (dir.length () != 0.0);
}

OneDimObject::OneDimObject (const OneDimObject& object)
{
   p = object.p;
   q = object.q;
   dir = q-p;
   assert (dir.length () != 0.0);
}

////////////////////////////////////////////////////////////////////////////////
// Find a point between (i.e., lying on the closest connection) the two objects
// given. Returns either ObjectsParallel or ObjectsDontIntersect. If the objects
// are parallel, no additional effort is made to determine whether they are
// equal or if they intersect.

double sarrus (const Point3D& a, const Point3D& b, const Point3D& c)
{ 
   return a.x * b.y * c.z
        + b.x * c.y * a.z
        + c.x * a.y * b.z
        - a.z * b.y * c.x
        - b.z * c.y * a.x
        - c.z * a.y * b.x;
}

int OneDimObject::FindPointBetween (const OneDimObject& object,
                                    Point3D& intersection) const
{
   // Test if pDir is multiple of qDir
   double d = dir.x / object.dir.x;
   if (dir.y / object.dir.y == d && dir.z / object.dir.z == d)
      return ObjectsParallel;

   Point3D dist = object.p-p;
	Point3D cross = dir.crossMultiply (object.dir);

   double det = sarrus (dir, cross, -object.dir);
   if (det == 0.0) {
      return ObjectsParallel;
   }

	double s = sarrus (dist, cross, -object.dir) / det;
	double t = sarrus (dir, cross, dist) / det;

   if (!validParam (s))
      s = putInRange (0.0, s, 1.0);
   if (!object.validParam (t))
      t = putInRange (0.0, t, 1.0);

   Point3D p1 = p+s*dir;
   Point3D p2 = object.p+t*object.dir;   
   intersection = (p1+p2) * 0.5;
   return ObjectsDontIntersect;
}

int OneDimObject::intersects (const TwoDimObject& object,
                              Point3D& intersection) const
{
   // See "Realistic Ray Tracing", Peter Shirley, page 28
   double a = object.r.x - object.s.x;
   double b = object.r.y - object.s.y;
   double c = object.r.z - object.s.z;
   double d = object.r.x - object.t.x;
   double e = object.r.y - object.t.y;
   double f = object.r.z - object.t.z;
   double g = dir.x;
   double h = dir.y;
   double i = dir.z;
   double j = object.r.x - p.x;
   double k = object.r.y - p.y;
   double l = object.r.z - p.z;

   double bl_kc = b*l - k*c;
   double jc_al = j*c - a*l;
   double ak_jb = a*k - j*b;
   double dh_eg = d*h - e*g;
   double gf_di = g*f - d*i;
   double ei_hf = e*i - h*f;

   double M = a*ei_hf + b*gf_di + c*dh_eg;

   if (M == 0.0)
      return ObjectsParallel;

   double rs =  (j*ei_hf + k*gf_di + l*dh_eg) / M;
   double rt =  (i*ak_jb + h*jc_al + g*bl_kc) / M;
   double pq = -(f*ak_jb + e*jc_al + d*bl_kc) / M;

   if (object.validParams (rs, rt) && validParam (pq))
   {
      intersection = p + pq*dir;
      return ObjectsIntersect;
   } else {
      return ObjectsDontIntersect;
   }
}

// Compute the distance to a point
double OneDimObject::distanceTo (const Point3D& point)
{
   // Use dir as the normal of a plane that contains point. Then the distance
   // of p to that plane is the factor by which we have to multiply dir to
   // obtain the closest point on the straight line through p and q.
   Point3D UnitDir = dir;
   UnitDir.unitLength ();
   double dist = UnitDir * (point-p);
   
   // Check the coefficient
   if (!validParam (dist))
      dist = putInRange (0.0, dist, 1.0);
   
   Point3D ClosestPoint = p + dist * UnitDir;
   
   return (point-ClosestPoint).length ();
}

// Calculate parameters for intersection with another straigt line
// Thoroughly tested for line segments using plab's testLSIntersections.xml
double OneDimObject::distanceTo (const OneDimObject& other)
{
   Point3D cross = dir.crossMultiply (other.dir);

   // Solve t1*dir + t2*cross + t3*other.dir = -p-other.p
   // using Sarrus's and Cramer's rules
   double det = sarrus (dir, cross, other.dir);
   if (det != 0) {
      double t1 = sarrus (other.p-p, cross, other.dir) / det;
      double t2 = sarrus (dir, cross, p-other.p) / det;

      t1 = findClosestParam (t1);
      t2 = findClosestParam (t2);

      Point3D p1 = p + t1*dir;
      Point3D p2 = other.p + t2*other.dir;
      double dist = (p2-p1).length();
      return dist;
   } else
      return -1;
}

////////////////////////////////////////////////////////////////////////////////
// TwoDimObject

// Constructors
TwoDimObject::TwoDimObject ()
{
}

TwoDimObject::TwoDimObject (const Point3D& _r, const Point3D& _s, 
                            const Point3D& _t)
{
   r = _r;
   s = _s;
   t = _t;
   n = (r-s).crossMultiply (r-t);
   n.unitLength ();
}

TwoDimObject::TwoDimObject (const TwoDimObject& object)
{
   r = object.r;
   s = object.s;
   t = object.t;
   n = (r-s).crossMultiply (r-t);
   n.unitLength ();
}

// Conduct an intersection test by creating IntersectionParameters for both
// intersected objects and testing them on the objects
int TwoDimObject::intersects (const OneDimObject& obj,
                              Point3D& intersection) const
{
    return obj.intersects (*this, intersection);
}

////////////////////////////////////////////////////////////////////////////////
// Ray

// Constructors
Ray::Ray ()
   : OneDimObject ()
{
}

Ray::Ray (const OneDimObject& object)
   : OneDimObject (object)
{
}

Ray::Ray (const Point3D& _p, const Point3D& _q)
   : OneDimObject (_p, _q)
{
}
   
bool Ray::validParam (double pq) const
{
   return (pq >= 0.0);
}

// Find closest valid parameter for dir
double Ray::findClosestParam (double d)
{ 
   return max (d, 0.0); 
}

////////////////////////////////////////////////////////////////////////////////
// StraightLine

StraightLine::StraightLine ()
   : OneDimObject ()
{
}

StraightLine::StraightLine (const OneDimObject& object)
   : OneDimObject (object)
{
}

StraightLine::StraightLine (const Point3D& _p, const Point3D& _q)
   : OneDimObject (_p, _q)
{
}

bool StraightLine::validParam (double) const
{
   return true;
}

////////////////////////////////////////////////////////////////////////////////
// LineSegment

// Constructors
LineSegment::LineSegment ()
   : OneDimObject ()
{
}

LineSegment::LineSegment (const OneDimObject& object)
   : OneDimObject (object)
{
}

LineSegment::LineSegment (const Point3D& _p, const Point3D& _q)
   : OneDimObject (_p, _q)
{
}

bool LineSegment::validParam (double pq) const
{
   return (pq >= 0.0 && pq <= 1.0);
}

// Find closest valid parameter for dir
double LineSegment::findClosestParam (double d)
{ 
   //return putInRange (0, d, 1); 
   if (d < 0.0)
      return 0.0;
   else if (d > 1.0)
      return 1.0;
   else
      return d;
}

////////////////////////////////////////////////////////////////////////////////
// Triangle

// Constructors
Triangle::Triangle ()
   : TwoDimObject ()
{
}

Triangle::Triangle (const TwoDimObject& object)
   : TwoDimObject (object)
{
}

Triangle::Triangle (const Point3D& _r, const Point3D& _s, const Point3D& _t)
   : TwoDimObject (_r, _s, _t)
{
}

bool Triangle::validParams (double rs, double rt) const
{
   return (rs >= 0.0
   &&      rt >= 0.0
   &&      rs+rt <= 1.0);
}


////////////////////////////////////////////////////////////////////////////////
// Rectangle

// Constructors
Rectangle::Rectangle ()
   : TwoDimObject ()
{
}

Rectangle::Rectangle (const TwoDimObject& object)
   : TwoDimObject (object)
{
}

Rectangle::Rectangle (const Point3D& _r, const Point3D& _s, const Point3D& _t)
   : TwoDimObject (_r, _s, _t)
{
}

bool Rectangle::validParams (double rs, double rt) const
{
   return (rs >= 0.0
   &&      rs <= 1.0
   &&      rt >= 0.0
   &&      rt <= 1.0);
}

////////////////////////////////////////////////////////////////////////////////
// Plane

// Constructors
Plane::Plane ()
   : TwoDimObject ()
{
}

Plane::Plane (const TwoDimObject& object)
   : TwoDimObject (object)
{
}

Plane::Plane (const Point3D& _r, const Point3D& _s, const Point3D& _t)
   : TwoDimObject (_r, _s, _t)
{
}

bool Plane::validParams (double, double) const
{
   return true;
}

int Plane::intersects (const Plane& obj, StraightLine& intersection) const
{
   // Proceed if poly1 and poly2 are not parallel or equal   
   if (n != obj.n) {
      Point3D sr = s-r;
      Point3D tr = t-r;

      assert (sr.length () != 0.0);
      assert (tr.length () != 0.0);
      tr.unitLength ();
      sr.unitLength ();
      assert (tr != sr && tr != -sr);
      
      // Compute the intersection of poly1 and poly2
      Point3D i0, i1;
      int res1 = StraightLine (r, s).intersects (obj, i0);
      int res2 = StraightLine (r+tr, s+tr).intersects (obj, i1);
      if (res1 != ObjectsIntersect
      ||  res2 != ObjectsIntersect
      ||  i0 == i1) 
      {
         StraightLine (r, t).intersects (obj, i0);
         StraightLine (r+sr, t+sr).intersects (obj, i1);
         assert (i0 != i1);
      }
      intersection = StraightLine (i0, i1);
      return ObjectsIntersect;
   } else {
      return ObjectsParallel;
   }
}
