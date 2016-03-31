////////////////////////////////////////////////////////////////////////////////
// MechLab.h
// Mech Configuration window for MechVM
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef MechLab__h
#define MechLab__h

#include "GLButton.h"
#include "GLLabel.h"
#include "GLWindowContainer.h"
#include "GLSlider.h"
#include "RenderableObject.h"
#include "MechWarriorIIPRJ.h"
#include "Vehicles.h"
#include "XMLTree.h"
#include "Mesh.h"
#include "intersections.h"
#include "GLTableView.h"
#include "GLComboBox.h"

////////////////////////////////////////////////////////////////////////////////

const int MechLab_selectChassisButtonID = 1;
const int MechLab_createTextureButtonID = 2;
const int MechLab_animateMechButtonID = 4;
const int MechLab_outfitMechButtonID = 8;
const int MechLab_equipmentButtonID = 16;
const int MechLab_weaponGroupsID = 32;
const int MechLab_exitButtonID = 64;
const int MechLab_AllButtonsID = 
   MechLab_selectChassisButtonID | MechLab_createTextureButtonID |
   MechLab_animateMechButtonID | MechLab_outfitMechButtonID |
   MechLab_equipmentButtonID | MechLab_weaponGroupsID | MechLab_exitButtonID;

const int MechLab_MechsTableID = 1024;
const int MechLab_WeaponsTableID = 1025;
const int MechLab_SectionID = 1026;
//const int MechLab_SectionTableID = 1027;
//const int MechLab_SectionSelectionID = 1028;
const int MechLab_SectionSelectionTable = 1029;
const int MechLab_WeaponGroupTable = 1030;

const int MechLab_Anim_ULegAngle1ID   = 1050;
const int MechLab_Anim_LLegAngle1ID   = 1051;
const int MechLab_Anim_FootAngle1ID   = 1052;
const int MechLab_Anim_ULegAngle2ID   = 1053;
const int MechLab_Anim_LLegAngle2ID   = 1054;
const int MechLab_Anim_FootAngle2ID   = 1055;
const int MechLab_Anim_ParallelFeetID = 1056;

// +0 accessing the slider, +1 for label
const int MechLab_Equipment_EngineRatingID = 2048;
const int MechLab_Equipment_EngineTypeID = 2050;
const int MechLab_Equipment_HeatSinkCount = 2052;
const int MechLab_Equipment_HeatSinkType = 2054;
const int MechLab_Equipment_InternalStructureType = 2056;
const int MechLab_Equipment_ArmorType = 2058;
const int MechLab_Equipment_LeftLowerArmActuator = 2060;
const int MechLab_Equipment_LeftHandActuator = 2062;
const int MechLab_Equipment_RightLowerArmActuator = 2064;
const int MechLab_Equipment_RightHandActuator = 2066;
const int MechLab_Equipment_ArmorFront = 2068; // uses 8 IDs
const int MechLab_Equipment_ArmorRear = 2084; // uses 8 IDs
const int MechLab_CommentID = 2094;
const int WeaponGroupAssignerID = 2100;

////////////////////////////////////////////////////////////////////////////////
// Weapons that can be mounted on mechs

class WeaponsLocker: public VirtualClass {
public:
   // Constructor, destructor
   WeaponsLocker()
      : VirtualClass (WeaponsLockerID)
   {}
   ~WeaponsLocker()
   {}

   // Get total number of different weapons in the locker
   inline size_t getWeaponCount()
   { return weaponsInStock.getSize() / 2+1; }

   // Get weapons count by index
   inline int getWeaponNo (int index)
   { return weaponsInStock[2*index-2]; }

   // Get number of a specific weapon by index
   inline int getWeaponCount (int index)
   { return weaponsInStock[2*index-1]; }

   // Find weapon - returns index if found, or -1 otherwise
   int findWeapon (int weaponNo);

   // Add weapons to locker
   void addWeaponToStock (int weaponNo, int count);
   void addWeaponsToStock (int weaponStartNo, int weaponEndNo, int count);

   // Remove weapons from locker
   bool removeWeapon (int weaponNo, int count);

   // Fill locker with standard layouts
   void addISLevel1Weapons(size_t weaponCount, size_t ammoCount);
   void addClanWeapons(size_t energyWeaponCount, 
      size_t ballisticWeaponCount, size_t ballisticAmmoCount,
      size_t GaussAndMachineGuns,
      size_t missileWeaponCount, size_t missileAmmoCount,
      size_t equipmentCount, bool GBL);
   void addGBLUnderwaterWeapons(size_t weaponCount, size_t ammoCount);
private:
   BGVector<int, BGVector_int> weaponsInStock;
};

////////////////////////////////////////////////////////////////////////////////
// Class to list the mech library

class MechList: public VirtualClass {
public:
   void loadMechs (const TCHAR* mechDir);

   void addMVMK (const TCHAR* mechDir, const TCHAR* MVMKfile);

   void addMech (const BGString& name, const BGString& xml, 
                 const BGString& mek, const BGString& textFN);

   inline size_t getMechCount() const
   { return MechNames.size(); }

   const char* getMechName(size_t index) const;
   const char* getMVMKFN(size_t index) const;
   const char* getTextureFN(size_t index) const;
   const char* getConfigFN(size_t index) const;

   Vehicle* loadMech(size_t index);

private:
   vector<BGString> MechNames;
   vector<BGString> MechXML;
   vector<BGString> MechCFG;
   vector<BGString> MechText;
};

////////////////////////////////////////////////////////////////////////////////
// Mech lab class

class MechLab: public GLWindowContainer, GLTableViewDataSource
{
public:
   // Constructor, destructor
   MechLab(int buttons, GLWindow* mainWin);
   virtual ~MechLab();

   // Receive messages
   virtual void receiveMessage (VirtualClass* sender, int MessageType);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Keyboard event handler
   void keypressed(char key, bool shiftHeld, bool controlHeld);

   // Set MechWarrior 2 PRJ file
   //void setMW2_PRJ (MechWarriorIIPRJ* MW2PRJ)
   //{ m_MW2PRJ = MW2PRJ; }

   // Set MechLab weapons type
   void addWeapons(const BGString& description);

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, bool down, 
                                         int /*x*/, int /*y*/);

   virtual void mouseMoved (int /*x*/, int /*y*/);

   // Get ray direction for mouse coordinates
   Point3D getMouseRay (double x, double y);

   // If the ray given by origin, dir intersects the Triangle, this function
   // returns the distance of the intersection, otherwise, it returns -1
   void testProximity (const Triangle& T, const Point3D& origin, 
      const Point3D& dir, MeshPolygon* poly, double& minDistSoFar,
      MeshPolygon*& nearestPolySoFar);

   // Test if the ray given by origin, dir intersects ro or one of its childs,
   // at a nearer distance than minDistSoFar. If it does, set minDistSoFar,
   // and set new selected object in plab
   // Returns the nearest intersected object
   void TraverseScene (const Point3D& origin, const Point3D& dir, 
      RenderableObject* ro, double& minDistSoFar, 
      MeshPolygon*& nearestPolySoFar);

   // Select geometry using a mouse ray
   MeshPolygon* mouseRaySelection (int x, int y);

   // Assign polygon to a limb
   void reassign (MeshPolygon* poly);

   // Implementation of GLTableViewDataSource
   virtual void getDimensions (size_t senderID, size_t& rowCount, size_t& colCount);
   virtual bool getString (size_t senderID, int row, int col, BGString& str);

   // Load and display mech given by number from list of mechs
   bool showMech(size_t num);
   void showMech(Vehicle* vehicle);

   void showChassisTable();
   GLWindow* createControls (int ID, int type, const char* name);
   void showAnimControls();
   void showOutfitMechControls();
   void showWeaponGroups();

   void initDialog();

   inline size_t getMechCount() const
   { return mechList.getMechCount(); }

   void showMechTonnage();

   // Set the window that will be displayed if the user clicks exit
   void setReturnVis (GLWindow* returnWin)
   { m_returnWin = returnWin; }

   inline void setCurrMechName (const TCHAR* name)
   { 
      if (vehicle != NULL)
         vehicle->setName(name);
   }

   inline bool loadCFG(const TCHAR* cfgFN) 
   { 
      if (vehicle != NULL)
         return vehicle->loadConfig(cfgFN);
      else
         return false;
   }

   inline bool loadCFG(MemoryBlock* mb) 
   {
      if (vehicle != NULL)
         return vehicle->loadFromMW2MEK(mb);
      else
         return false;
   }

   // Select file name to store MEK file from given directory
   void selectMEKFNInDirectory (const TCHAR* dir);

   // Select file name to store MEK file in game directory, using given prefix
   void selectMEKFNInGameDirectory (const TCHAR* gameDir, const TCHAR* prefix);

   inline void setMEKformat(int newVal)
   { MEKformat = newVal; }

private:
   GLWindow* m_returnWin;
   Vehicle* vehicle;
   double viewDist, meshSize;
   Point3D meshCenter;
   int m_buttons;
   GLButton * importGeometryButton, * assignPoliesButton, *createTextureButton,
      * animateMechButton, * repairMechButton, *configureMechButton,
      * equipmentButton, * weaponGroupsButton, * exitButton;
   GLLabel * label;
   MechList mechList;
   int MEKformat;

   GLLabel* CenterTorsoArmorSumLabel, * LeftTorsoArmorSumLabel,
      * RightTorsoArmorSumLabel;

   // File name used for saving to file
   BGString MEKFN;

   bool leftButtonDown, rightButtonDown, renderMech, LimitedStock, changed;
   int lastMouseX, lastMouseY, currSection;
   Point3D eye, center, xDir, yDir, zDir, selPoint;
   MatrixStack TraversalStack;
   GLTableView * mechTable, * weaponsTable, * sectionTable, * sectionSelectionTable;
   WeaponsLocker* weaponsLocker;

   //void showMechPartControls(int baseID);
   void addContextButton(const TCHAR* bname, int ID, int BaseID);
   GLButton* addToolbarButton(const TCHAR* buttonName, int ID);

   // Load mech from obj and bmp
   void assignPolies();

   void showEquipmentControls();
   void addCheckbox (const char* labelTitle, int& y, bool initialState, int ID);
   GLSlider* addSlider (const char* labelTitle, int min, int pos, int max, int& y, int ID);
   GLComboBox* addComboBox (const char* labelTitle, const char* cbTitle, int& y, int ID);
   GLSlider* addArmorSlider (int section, bool front, int& y);
   GLLabel* findLabel (int ID);
   void updateEquipmentLabels();
};

#endif
