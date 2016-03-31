////////////////////////////////////////////////////////////////////////////////
// Class VehicleAI and subclasses
// Classes that control vehicle movement, including physics
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#include "VehicleAI.h"
//#include "GL/glut.h"
#include "Mesh3.h"
#include <SDL.h>

////////////////////////////////////////////////////////////////////////////////
// Constants

const double turnDelta = 1.0;
const double maxSpeed = 1.0;
const double minSpeed = 1.0;
const double gravity = 2/60.0;
const double jumpJetAcceleration = 2.0 * gravity;

////////////////////////////////////////////////////////////////////////////////
// Class for Mechs that are controlled by local players

// Constructor
PlayerBipedMech::PlayerBipedMech (BipedMech* mech)
   : PlayerVehicle(),
     m_groundContact (false),
     m_jumpJetsActive (false),
     m_Mech (mech),
     m_speed (0.0), 
     m_walkingDirectionDegrees (0.0),
     m_turnDelta (0.0),
     m_speedDelta (0.0)
{
}

// Keyboard event handler
void PlayerBipedMech::keypressed(char key, bool /*shiftHeld*/, bool /*controlHeld*/, bool down)
{
   Point3D front = m_Mech->p_front.get();
   Point3D left = m_Mech->p_left.get();

   switch(key) {
      case 'j':
         m_jumpJetsActive = down;
         break;
      case 20: //SDLK_LEFT:
         if (down)
            m_turnDelta = 1.0;
         else
            m_turnDelta = 0.0;
         break;
      case 17: //SDLK_UP:
         if (down)
            m_speedDelta = 0.1;
         else
            m_speedDelta = 0.0;
         break;
      case 19: //SDLK_RIGHT:
         if (down)
            m_turnDelta = -1.0;
         else
            m_turnDelta = 0.0;
         break;
      case 18: //SDLK_DOWN:
         if (down)
            m_speedDelta = -0.1;
         else
            m_speedDelta = 0.0;
         break;
   }

   m_Mech->p_left.set(left);
   m_Mech->p_front.set(front);
}

// Button state changed over a window: returns child that was clicked
void PlayerBipedMech::buttonStateChanged (int /*button*/, int /*state*/, 
                                      int /*x*/, int /*y*/)
{
}

void PlayerBipedMech::mouseMoved (int /*x*/, int /*y*/)
{
}

// Calculate movement for next frame
void PlayerBipedMech::calcNextFrameMovement()
{
   m_Mech->setMatrices();
   const Point3D& pos = m_Mech->p_position.get();
   const Point3D& front = m_Mech->p_front.get();

   double moveUp = 0.0;
   for (size_t i = 0; i < m_Mech->getObjectCount(); ++i) {
      RenderableObject* ro = m_Mech->getObject(i);
      if (ro != NULL) {
         Mesh3* mesh = dynamic_cast <Mesh3*> (ro);
         if (mesh != NULL) {
            for (size_t j = 0; j < mesh->getPointCount(); ++j) {
               Point3D p = mesh->getVertex(j);
               p = mesh->getMatrix().Transform(p);
               double alt = m_heightfield->getHeightD(p.x, p.z);
               if (alt > p.y) {
                  moveUp = max(moveUp, alt-p.y);
               }
            }
         }
      }
   }

   if (moveUp > 0.0) {
      //m_movementVector.y += 0.5 * moveUp;
      double degrees = M_PI * m_walkingDirectionDegrees / 180.0;
      Matrix rot = RotationMatrix (degrees, 0.0, 1.0, 0.0);
      m_Mech->p_front.set(rot.Transform(Point3D(0, 0, 1)));
      m_Mech->p_left.set(rot.Transform(Point3D(-1, 0, 0)));
      m_movementVector = m_speed * front;

      Point3D pos = m_Mech->p_position.get();
      pos.y += moveUp;
      pos += m_movementVector;
      m_Mech->p_position.set(pos);

      m_groundContact = true;

      m_walkingDirectionDegrees = m_walkingDirectionDegrees-m_turnDelta;
      m_speed += m_speedDelta;
   } else {
      m_movementVector.y -= gravity;
      m_groundContact = false;
      m_Mech->p_position.set(pos + m_movementVector);
   }

   if (m_jumpJetsActive)
      m_movementVector.y += jumpJetAcceleration;

   // todo: use high precision timers to scale movement vector
   //QueryPerformanceFrequency
   //QueryPerformanceCounter
}