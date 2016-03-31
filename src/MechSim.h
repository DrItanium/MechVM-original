////////////////////////////////////////////////////////////////////////////////
// MechSim.h
// Simulation window for MechVM
// Copyright Bjoern Ganster 2007-2008
////////////////////////////////////////////////////////////////////////////////

#ifndef MechSim__h
#define MechSim__h

#include "GLWindow.h"
#include "RenderableObject.h"
#include "MechWarriorIIPRJ.h"
#include "Vehicles.h"
#include "VehicleAI.h"
#include "XMLTree.h"
#include "Heightfield.h"

////////////////////////////////////////////////////////////////////////////////
// Mech simulator window

class MechSim: public GLWindow
{
public:
   // Constructor, destructor
   MechSim();
   virtual ~MechSim();

   // Load mechs
   //void loadMechs ();

   // Receive messages
   virtual void receiveMessage (VirtualClass* sender, int MessageType);

   // Update title with model graph name and framerate
   void updateTitle ();

   // Render
   virtual void render(int x, int y, int w, int h);

   // Keyboard event handlers
   virtual void keypressed(BGKey key, bool shiftHeld, bool controlHeld, bool down);
   //virtual void specialKeyPressed (char key, bool shiftHeld, bool controlHeld);

   void setReturnVis(GLWindow* newVis)
   { returnVis = newVis; }

   //Vehicle* addVehicle (const TCHAR* name, const TCHAR* MVMKFN, 
   //   const TCHAR* textureFN);
   void addVehicle (RenderableObject* vehicle)
   { m_sceneObjects->addChild(vehicle); }
   PlayerBipedMech* setPlayerMech(BipedMech* vehicle);

   void placeVehicleRandomly(Vehicle* vehicle)
   {
      int x = random(0, heightfield.getSizeX()-1);
      int y = random(0, heightfield.getSizeY()-1);
      double h = heightfield.getHeight(x, y);
      Point3D min, max;
      size_t polyCount;
      vehicle->getSize(min, max, polyCount);
      vehicle->p_position.set(Point3D(x, h-vehicle->p_scale.get()*min.y, y));
   }

   HeightField* getHeightfield ()
   { return &heightfield; }

   const HeightField* getHeightfield () const 
   { return &heightfield; }
private:
   Point3D viewerPos, viewerDir;
   RenderableObject* m_sceneObjects;
   HeightField heightfield;
   GLWindow* returnVis;
   PlayerVehicle* playerVehicle;
};

#endif
