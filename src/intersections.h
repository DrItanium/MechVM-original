////////////////////////////////////////////////////////////////////////////////
// intersections.h
// Header for intersection tests
// Copyright Björn Ganster in 2001
////////////////////////////////////////////////////////////////////////////////

#ifndef Intersection__h
#define Intersection__h

#include "Point3D.h"

class OneDimObject;
class TwoDimObject;

////////////////////////////////////////////////////////////////////////////////
// Intersection results

enum IntersectionResult {
   ObjectsIntersect,
//   ObjectsEqual,
   ObjectsDontIntersect,
   ObjectsParallel,
//   ObjectsParallelButNotIntersecting,
//   ObjectsParallelAndIntersecting
};

////////////////////////////////////////////////////////////////////////////////
// OneDimObject
// Base Class for all objects that can be intersected and that are described in
// one coordinate

class OneDimObject {
public:
   Point3D p, q, dir;
   
   // Constructors
   OneDimObject ();
   OneDimObject (const Point3D& _p, const Point3D& _q);
   OneDimObject (const OneDimObject& object);
   
   // Destructor must be virtual
   virtual ~OneDimObject()
   {}

   // Find a point between (i.e., lying on the closest connection) the two
   // objects given (see the cpp for add. comments)
   int FindPointBetween (const OneDimObject& object, Point3D& result) const;

   // Conduct an intersection test by creating IntersectionParameters for both
   // intersected objects and testing them on the objects
   // The returned value is one of IntersectionResult
   int intersects (const TwoDimObject& object, Point3D& intersection) const;

   // Test if IntersectionParameters specify an intersection of the object
   virtual bool validParam (double pq) const = 0;

   // Compute the distance to another object
   double distanceTo (const Point3D& point);
   double distanceTo (const OneDimObject& other);

   // Find closest valid parameter for dir
   virtual double findClosestParam (double d)
   { return d; }
};

////////////////////////////////////////////////////////////////////////////////
// TwoDimObject
// Base Class for all objects that can be intersected and that are described in
// two coordinates

class TwoDimObject {
public:
   Point3D r, s, t, n;

   // Constructors
   TwoDimObject ();
   TwoDimObject (const Point3D& _r, const Point3D& _s, const Point3D& _t);
   TwoDimObject (const TwoDimObject& object);

   // Destructor must be virtual
   virtual ~TwoDimObject()
   {}

   // Conduct an intersection test by creating IntersectionParameters for both
   // intersected objects and testing them on the objects
   virtual int intersects (const OneDimObject& obj,
                           Point3D& intersection) const;
/*   virtual int intersects (const TwoDimObject& obj,
                            IntersectionParameters& result);*/ //todo

   // Test if IntersectionParameters specify an intersection of the object
   virtual bool validParams (double rs, double rt) const = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Ray

class Ray: public OneDimObject {
public:
   // Constructors
   Ray ();
   Ray (const OneDimObject& object);
   Ray (const Point3D& _p, const Point3D& _q);

   // Destructor must be virtual
   virtual ~Ray()
   {}

   // Intersection Test
   virtual bool validParam (double pq) const;

   // Find closest valid parameter for dir
   virtual double findClosestParam (double d);
};

////////////////////////////////////////////////////////////////////////////////
// StraightLine

class StraightLine: public OneDimObject {
public:
   // Constructors
   StraightLine ();
   StraightLine (const OneDimObject& object);
   StraightLine (const Point3D& _p, const Point3D& _q);

   // Destructor must be virtual
   virtual ~StraightLine()
   {}

   // Intersection Test
   virtual bool validParam (double pq) const;
};

////////////////////////////////////////////////////////////////////////////////
// LineSegment

class LineSegment: public OneDimObject {
public:
   // Constructors
   LineSegment ();
   LineSegment (const OneDimObject& object);
   LineSegment (const Point3D& _p, const Point3D& _q);

   // Destructor must be virtual
   virtual ~LineSegment()
   {}

   // Intersection Test
   virtual bool validParam (double pq) const;

   // Find closest valid parameter for dir
   virtual double findClosestParam (double d);
};

////////////////////////////////////////////////////////////////////////////////
// Triangle

class Triangle: public TwoDimObject {
public:
   // Constructors
   Triangle ();
   Triangle (const TwoDimObject& object);
   Triangle (const Point3D& _r, const Point3D& _s, const Point3D& _t);

   // Destructor must be virtual
   virtual ~Triangle()
   {}

   // Intersection Test
   virtual bool validParams (double rs, double rt) const;
};

////////////////////////////////////////////////////////////////////////////////
// Rectangle

class Rectangle: public TwoDimObject {
public:
   // Constructors
   Rectangle ();
   Rectangle (const TwoDimObject& object);
   Rectangle (const Point3D& _r, const Point3D& _s, const Point3D& _t);

   // Destructor must be virtual
   virtual ~Rectangle()
   {}

   // Intersection Test
   virtual bool validParams (double rs, double rt) const;
};

////////////////////////////////////////////////////////////////////////////////
// Plane

class Plane: public TwoDimObject {
public:
   // Constructors
   Plane ();
   Plane (const TwoDimObject& object);
   Plane (const Point3D& _r, const Point3D& _s, const Point3D& _t);

   // Destructor must be virtual
   virtual ~Plane()
   {}

   // Intersection Test
   virtual bool validParams (double rs, double rt) const;

   // Test two planes for an intersection
   virtual int intersects (const Plane& obj, StraightLine& intersection) const;

   // Test whether p lies on the positive (1) or negative (-1) side of the plane, 
   // or in the plane (0). Return value 0 is unreliable.
   inline double halfSpaceTest (Point3D p)
   { return n * (r-p); }
};

#endif
