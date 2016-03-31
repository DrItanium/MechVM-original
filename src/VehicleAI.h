////////////////////////////////////////////////////////////////////////////////
// Class VehicleAI and subclasses
// Classes that control vehicle movement, including physics
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#ifndef VehicleAI__h
#define VehicleAI__h

#include "BipedMech.h"
#include "Vehicles.h"
#include "Heightfield.h"

//class Heightfield;

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for vehicles and Mechs that are controlled by AI or
// players

class VehicleAI: public VirtualClass {
public:
   // Calculate movement for next frame
   virtual void calcNextFrameMovement() = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Abstract base class for vehicles and Mechs that are controlled by players

class PlayerVehicle: public VehicleAI {
public:
   // Keyboard event handlers
   virtual void keypressed(char key, bool shiftHeld, bool controlHeld, bool down) = 0;
   //virtual void specialKeyPressed (int key, bool shiftHeld, bool controlHeld) = 0;

   // Button state changed over a window: returns child that was clicked
   virtual void buttonStateChanged (int /*button*/, int /*state*/, 
                                         int /*x*/, int /*y*/) = 0;

   virtual void mouseMoved (int /*x*/, int /*y*/) = 0;

   // Calculate movement for next frame
   virtual void calcNextFrameMovement() = 0;

   virtual const Point3D& getPosition() const = 0;

   virtual const Point3D& getFront() const = 0;
private:
};

////////////////////////////////////////////////////////////////////////////////
// Class for Mechs that are controlled by local players

class PlayerBipedMech: public PlayerVehicle {
public:
   // Constructor
   PlayerBipedMech (BipedMech* mech);

   // Destructor
   ~PlayerBipedMech()
   { }

   // Keyboard event handlers
   virtual void keypressed(char key, bool shiftHeld, bool controlHeld, bool down);
   //virtual void specialKeyPressed (int key, bool shiftHeld, bool controlHeld);

   // Button state changed over a window: returns child that was clicked
   virtual void buttonStateChanged (int /*button*/, int /*state*/, 
                                         int /*x*/, int /*y*/);

   virtual void mouseMoved (int /*x*/, int /*y*/);

   // Calculate movement for next frame
   virtual void calcNextFrameMovement();

   virtual const Point3D& getPosition() const
   { return m_Mech->p_position.get(); }

   virtual const Point3D& getFront() const
   { return m_Mech->p_front.get(); }

   void setHeightfield (HeightField* heightfield)
   { m_heightfield = heightfield; }
private:
   bool m_groundContact, m_jumpJetsActive;
   BipedMech* m_Mech;
   Point3D m_movementVector;
   double m_speed, m_walkingDirectionDegrees, m_turnDelta, m_speedDelta;
   HeightField* m_heightfield;
};

#endif