////////////////////////////////////////////////////////////////////////////////
// QuadMech.h
// QuadMech implementation
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

// Quad Mechs are also referred to as four-legged Mechs in the BattleTech
// Master Rules, where they are described in Special Case Rules, 
// p. 75 (v1), p. 82 (v2)

#ifndef QuadMech__h
#define QuadMech__h

#include "Vehicles.h"

////////////////////////////////////////////////////////////////////////////////
// Mech section identifiers

const int Quad_Mech_Section_Head = 0;
const int Quad_Mech_Section_Center_Torso = 1;
const int Quad_Mech_Section_Left_Torso = 2;
const int Quad_Mech_Section_Right_Torso = 3;
const int Quad_Mech_Section_Left_Front_Leg = 4;
const int Quad_Mech_Section_Right_Front_Leg = 5;
const int Quad_Mech_Section_Left_Rear_Leg = 6;
const int Quad_Mech_Section_Right_Rear_Leg = 7;

const int Quad_Mech_Sections = 8;
const int MaxQuadCriticalsPerSection = 12;

const size_t QuadCriticalCount[Quad_Mech_Sections] = 
   {6, 12, 12, 12, 6, 6, 6, 6};

////////////////////////////////////////////////////////////////////////////////
// State for a quad mech

class QuadMech: public Vehicle
{
public:
   // Constructor
   QuadMech();

   // Destructor
   virtual ~QuadMech();

   virtual void addGeometry (int geometryType, Mesh* geometry,
      const Point3D& dist);

   // Get section and critical information
   virtual size_t getSectionCount () const
   { return Quad_Mech_Sections;}
   virtual const char* getSectionName (int i) const;
   virtual size_t getCriticalCount (int section) const
   { 
      if (section < Quad_Mech_Sections && section >= 0) {
         return QuadCriticalCount[section]; 
      } else
         return 0;
   }
   virtual int getCriticals (int section, int criticalSlot) const;

   // Query/set data
   virtual int getInteger (int info) const;
   virtual bool setInteger (int info, int newVal);
   virtual double getAnimParam (size_t level, size_t ID);
   virtual void setAnimParam (size_t level, size_t ID, double val);

   virtual double getWeight () const
   { return realWeight; }

   virtual double calcInternalStructureWeight() const;
   double calcJJWeight ();
   virtual bool setArmor(int section, DWORD newVal, int ArmorType);
   virtual DWORD getArmor(int section, int ArmorType) const;
   virtual DWORD getInternalStructure(int section) const
   { return InternalStructure[section]; }
   virtual int countArmorPoints() const;

   // Add weapon
   virtual bool addWeapon(int currSection, int weaponNo);

   // Remove weapon - returns number of criticals removed
   virtual int removeWeapon(int currSection, int critNo, int weaponNo, 
                            int CritCount = -1);

   // Check - and try to correct - config problems 
   // Returns 0 on success, and the number of a warning for the tr() function otherwise
   virtual int check();

   // Set current animation matrices
   virtual void setMatrices ();

   virtual void setInternalStructure (int section, DWORD newVal);
   virtual void setCriticals (int section, int slot, int weaponType);

private:
   // Time and position state
   // int lastTicks, nextTicks;
   // Point3D lastPos, nextPos, lastXDir, lastYDir, lastZDir, nextXDir, nextYDir, 
   //   nextZDir;
   double torsoTilt, torsoTwist, speed;
   int criticals[Quad_Mech_Sections][MaxQuadCriticalsPerSection];
   double realWeight;

   // Armor and internal structure
   int FrontArmor[Quad_Mech_Sections];
   int RearArmor[Quad_Mech_Sections];
   int InternalStructure[Quad_Mech_Sections];

   int checkAndFixEngineCrits(int section);

   virtual double calcRealWeight();

   virtual void loadVehicleGeometry (XMLTreeConstIterator& iter, 
      const BGString& path, Texture* texture);

   void loadPart (XMLTreeConstIterator& iter, int partNumber, const BGString& path);

protected:
   // Check if a section may mount jump jets
   virtual bool mayMountJJs (int section)
   { return (section != Quad_Mech_Section_Head); }

};

#endif