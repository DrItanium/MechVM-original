////////////////////////////////////////////////////////////////////////////////
// MechSim.cpp
// Simulation window for MechVM
// Copyright Bjoern Ganster 2007-2008
////////////////////////////////////////////////////////////////////////////////

#include "MechSim.h"

#include "BGString.h"
#include "Vehicles.h"
#include "Config.h"
#include "MechVM.h"

#include "GL/glu.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor, destructor

MechSim::MechSim()
: m_sceneObjects (new RenderableObject(RenderableObjectID, NULL)),
//  m_MW2PRJ (NULL),
  heightfield (512, 512),
  playerVehicle (NULL)
{
   // Softly curved
   heightfield.createHeightField(50, 100, 0.5, 1, 5);

   // Cornered hills
   //heightfield.createHeightField(50, 100, 0.25, 1, 0);

   double x = heightfield.getSizeX()/2;
   double y = heightfield.getSizeY()/2;
   //viewerDir = Point3D (1, 0, 0);
   viewerDir = Point3D (0, 0, 1);
   viewerPos = Point3D (x, heightfield.getHeight((int)x, (int)y)+15.0, y);
}


MechSim::~MechSim()
{
}

////////////////////////////////////////////////////////////////////////////////

PlayerBipedMech* MechSim::setPlayerMech(BipedMech* vehicle)
{ 
   m_sceneObjects->addChild(vehicle);
   if (playerVehicle != NULL)
      delete playerVehicle;
   playerVehicle = new PlayerBipedMech (vehicle);
   return (PlayerBipedMech*) playerVehicle;
}

////////////////////////////////////////////////////////////////////////////////

/*void MechSim::loadMechs ()
{
   printf ("Loading mech models\n");
   m_sceneObjects = new RenderableObject (RenderableObjectID);
   XMLTree tree;
   BGString path = getDataDir();
   path += OSPathSep;
   path += "mechs.xml";
   vector <BipedMech*> mechs;
   if (tree.loadFromFile(path.getChars())) {
      XMLTreeConstIterator iter = tree.getConstIterator();
      iter.goToChild(0);
      for (size_t i = 0; i < iter.getChildCount(); i++) {
         BipedMech* mech = new BipedMech ();
         //mech->setMW2_PRJ(m_MW2PRJ);
         //mech->setPosition (0.50*heightfield.getSizeX()+20*i, 
         //                   53.0, 0.4*heightfield.getSizeY());
         mechs.push_back(mech);
         iter.goToChild(i);
         mech->loadMech (iter, path.getChars());
         iter.goToParent();
      }

      for (size_t i = 0; i < mechs.size(); i++) {
         m_sceneObjects->addChild(mechs[i]);
      }
   } else
      printf ("Error loading %s\n", path.getChars());
}*/

////////////////////////////////////////////////////////////////////////////////
// Receive messages

void MechSim::receiveMessage (VirtualClass* sender, int MessageType)
{
}

////////////////////////////////////////////////////////////////////////////////
// Update title with model graph name and framerate

void MechSim::updateTitle ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void MechSim::render(int x, int y, int w, int h)
{
   if (playerVehicle != NULL) {
      playerVehicle->calcNextFrameMovement();
      viewerDir = playerVehicle->getFront();
      viewerPos = playerVehicle->getPosition() - 50 * viewerDir;
      viewerPos.y += 20;
   }

   glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glViewport(x, y, w, h);

   // Activate light source
   glEnable (GL_LIGHTING);
   glEnable (GL_COLOR_MATERIAL);
   //glEnable (GL_TEXTURE_2D);
   //glFrontFace (GL_CCW);
   //glDisable (GL_LIGHT_MODEL_TWO_SIDE);
   //glCullFace (GL_BACK);
   //glEnable (GL_TEXTURE_2D);

   GLfloat pos[4] = {0.5f, 1.0f, 0.250f, 0.0f};
   GLfloat color[3] = {0.7f, 0.7f, 0.7f};
   GLfloat ambient[3] = {0.3f, 0.3f, 0.3f};
   GLfloat k0 = 1.0;
   GLfloat k1 = 0.0;
   GLfloat k2 = 0.0;

   glEnable (GL_LIGHT0);
   glLightfv (GL_LIGHT0, GL_POSITION, pos);
   glLightfv (GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv (GL_LIGHT0, GL_DIFFUSE, color);
   glLightfv (GL_LIGHT0, GL_SPECULAR, color);
   glLightfv (GL_LIGHT0, GL_CONSTANT_ATTENUATION, &k0);
   glLightfv (GL_LIGHT0, GL_LINEAR_ATTENUATION, &k1);
   glLightfv (GL_LIGHT0, GL_QUADRATIC_ATTENUATION, &k2);

   // Set viewing projection
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective (60, ((double) w) / ((double) h), 10, 10000);
   gluLookAt (viewerPos.x, viewerPos.y, viewerPos.z,
              viewerPos.x+viewerDir.x, viewerPos.y+viewerDir.y, viewerPos.z+viewerDir.z, 
              0, 1, 0);
   glMatrixMode (GL_MODELVIEW);

   // Render
   glLoadIdentity();
   glEnable (GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glDisable(GL_TEXTURE_2D);

   heightfield.render();

   glColor3d(1, 1, 1);
   if (m_sceneObjects != NULL)
      m_sceneObjects->render();
   glDisable (GL_LIGHT0);
   glDisable (GL_LIGHTING);

   //viewerPos.x += 0.5;
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard event handler

void MechSim::keypressed(BGKey key, bool shiftHeld, bool controlHeld, bool down)
{
   if (key.sym == 'q') {
      setCurVis (returnVis);
      return;
   }

   if (playerVehicle != NULL) {
      playerVehicle->keypressed(key.sym, shiftHeld, controlHeld, down);
   } else {
      if (key.sym == 'a') {
         viewerDir = RotationMatrix(-M_PI*5.0/180.0, 0, 1.0, 0).Transform(viewerDir);
         viewerDir.unitLength();
      } else if (key.sym == 'd') {
         viewerDir = RotationMatrix(M_PI*5.0/180.0, 0, 1.0, 0).Transform(viewerDir);
         viewerDir.unitLength();
      } else if (key.sym == 'w') {
         viewerPos = viewerPos + viewerDir;
      } else if (key.sym == 's') {
         viewerPos = viewerPos - viewerDir;
      } else if (key.sym == 'm') {
         viewerPos.y += 1;
      } else if (key.sym == 'n') {
         viewerPos.y -= 1;
      } //else if (key == 'q') {
         //setCurVis (returnVis);
      //}
   }
}

////////////////////////////////////////////////////////////////////////////////

/*Vehicle* MechSim::addVehicle (const TCHAR* name, const TCHAR* MVMKFN, 
      const TCHAR* textureFN)
{
   Vehicle * vehicle = new BipedMech ();
   vehicle->setName(name);

   if (!vehicle->loadGeometry(MVMKFN, textureFN))
   {
      delete vehicle;
      return NULL;
   } else {
      m_sceneObjects->addChild(vehicle);
      return vehicle;
   }
}*/
