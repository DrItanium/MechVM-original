////////////////////////////////////////////////////////////////////////////////
// BipedMech.cpp
// BipedMech inplementation
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef BipedMech__h
#define BipedMech__h

#include "Vehicles.h"

////////////////////////////////////////////////////////////////////////////////
// Mech section identifiers

const int Biped_Mech_Section_Head = 0;
const int Biped_Mech_Section_Center_Torso = 1;
const int Biped_Mech_Section_Left_Torso = 2;
const int Biped_Mech_Section_Right_Torso = 3;
const int Biped_Mech_Section_Left_Arm = 4;
const int Biped_Mech_Section_Right_Arm = 5;
const int Biped_Mech_Section_Left_Leg = 6;
const int Biped_Mech_Section_Right_Leg = 7;

const int Biped_Mech_Sections = 8;
const int MaxBipedCriticalsPerSection = 12;

const size_t BipedCriticalCount[Biped_Mech_Sections] = 
   {6, 12, 12, 12, 12, 12, 6, 6};

////////////////////////////////////////////////////////////////////////////////
// State for a bipedal mech

const int BipedAnimPhase = 0;
const int BipedAnimUpperLegAngle = 1;
const int BipedAnimLowerLegAngle = 2;
const int BipedAnimFootAngle = 3;

class BipedMech: public Vehicle
{
public:
   // Constructor, destructor
   BipedMech();
   BipedMech (const BipedMech& other);
   ~BipedMech();

   // Load entire mech
   virtual void loadVehicleGeometry (XMLTreeConstIterator& iter, 
      const BGString& path, Texture* texture);

   // Load mech limb
   void loadPart (XMLTreeConstIterator& iter, int partNumber, 
      const BGString& path);

   // Add geometry for a vehicle part
   virtual void addGeometry (int geometryType, Mesh* geometry, 
      const Point3D& dist);
   //void addMW2Geometry (int geometryType, const TCHAR* FN, double tx, 
   //   double ty, double tz, double jx, double jy, double jz);

   // Parse XML stats
   void loadStats (XMLTreeConstIterator& iter);
   void fillCriticalHitsTable (XMLTreeConstIterator& iter, int section);
   void parseCrit (int section, BGString& crit);

   // Add anim parameters
   void addAnim (XMLTreeConstIterator& iter);

   virtual void setAnimParam (size_t level, size_t ID, double val)
   { animAngles[4*level+ID] = val; }

   virtual double getAnimParam (size_t level, size_t ID)
   { return animAngles[4*level+ID]; }

   // Load/save config from MechWarrior 2 MEK file
   //virtual bool loadFromMW2MEK (const MemoryBlock* mb);

   // Save config to XML file
   bool saveToTree (XMLTreeIterator& MechVMMechTreeIter);

   virtual size_t getSectionCount () const
   { return 8; }

   virtual const char* getSectionName (int i) const
   { return tr (i+4); }

   virtual size_t getCriticalCount (int section) const
   {
      if (section < (int) getSectionCount() && section >= 0)
         return BipedCriticalCount[section]; 
      else
         return 0;
   }

   virtual int getCriticals (int section, int criticalSlot) const
   { 
      if (section < (int) getSectionCount() && section >= 0
      &&  criticalSlot < (int) BipedCriticalCount[section] && criticalSlot >= 0)
      {
         return criticals[section][criticalSlot]; 
      } else
         return 0;
   }

   // Get maximum weight mech chassis can carry
   virtual int getMaxWeight() const
   { return tonnage; }

   // Calculate actual mech weight
   virtual double calcRealWeight();
   virtual double getWeight () const
   { return realWeight; }
   virtual int countArmorPoints() const;
   virtual double calcInternalStructureWeight() const;
   double calcJJWeight ();

   // Obtain mech information
   virtual int getInteger (int info) const;
   virtual bool setInteger (int info, int newVal);

   /*int getFrontArmor(int section) const
   { return FrontArmor[section]; }
   int getRearArmor(int section) const
   { return RearArmor[section]; }*/
   virtual DWORD getArmor(int section, int ArmorType) const;
   virtual bool setArmor(int section, DWORD newVal, int ArmorType);
   virtual DWORD getInternalStructure(int section) const
   { return InternalStructure[section]; }

   // Add weapon to mech section
   virtual bool addWeapon(int currSection, int weaponNo);

   // Remove weapon from mech section
   virtual int removeWeapon(int currSection, int critNo,  int weaponNo, 
                            int critCount = -1);

   // Check - and try to correct - config problems 
   // Returns 0 on success, and the number of a warning for the tr() function otherwise
   virtual int check();

   // Set current animation matrices
   virtual void setMatrices ();

   virtual void setInternalStructure (int section, DWORD newVal);
   virtual void setCriticals (int section, int slot, int weaponType);

private:
   double torsoTilt, torsoTwist, speed;
   //double ulegAngle1, ulegAngle2, llegAngle1, llegAngle2, footAngle1, footAngle2;
   bool legsParallelToGround;
   Point3D* m_joint;
   BGVector <double, BGVectorBufID> animAngles;

   // Basic mech data
   double realWeight;
   int criticals[Biped_Mech_Sections][MaxBipedCriticalsPerSection];

   // Armor and internal structure
   int FrontArmor[Biped_Mech_Sections];
   int RearArmor[Biped_Mech_Sections];
   int InternalStructure[Biped_Mech_Sections];

   //void checkBasicMechProperties (const MemoryBlock* mb, bool& subType31stCC);

   int checkAndFixEngineCrits(int section);

protected:
   // Check if a section may mount jump jets
   virtual bool mayMountJJs (int section)
   { 
      if (section != Biped_Mech_Section_Head
      &&  section != Biped_Mech_Section_Left_Arm
      &&  section != Biped_Mech_Section_Right_Arm)
      {
         return true;
      } else {
         return false;
      }
   }

};

////////////////////////////////////////////////////////////////////////////////
// Biped mech AI - players controlled by the computer

class BipedMechAI: public BasicPlayer {
};

////////////////////////////////////////////////////////////////////////////////
// Biped mech local player - vehicle controlled by local human player

class BipedMechLocalPlayer: public BasicPlayer {
};

////////////////////////////////////////////////////////////////////////////////
// Biped mech network player - vehicle controlled by a network player or bot

class BipedMechNetworkPlayer: public BasicPlayer {
};

#endif