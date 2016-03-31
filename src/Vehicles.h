////////////////////////////////////////////////////////////////////////////////
// Vehicle.h
// Vehicle control
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef Vehicle__h
#define Vehicle__h

#include "Mesh.h"
#include "MechWarriorIIPRJ.h"
#include "XMLTree.h"

////////////////////////////////////////////////////////////////////////////////
// Mech mission goals

const int TargetNearest = 1; // Target nearest enemy in less than distance d
const int StartOnTarget = 2; // Start mech on target acquisition
const int VisitNav      = 3; // Go to nav point p_i
const int DestroyTarget = 4; // Destroy target o_i
const int DefendTarget  = 5; // Attack anyone attacking object o_i
const int Wait          = 6; // Wait n seconds
const int ShutDown      = 7; // Shut down
const int GoToGoal      = 8; // Go To goal g_i

////////////////////////////////////////////////////////////////////////////////
// Mech mesh identifiers

const int Mech_Hip = 0;
const int Mech_Torso = 1;
const int Mech_LeftArm = 2;
const int Mech_RightArm = 3;
const int Mech_LeftUpperLeg = 4;
const int Mech_LeftLowerLeg = 5;
const int Mech_LeftFoot= 6;
const int Mech_RightUpperLeg = 7;
const int Mech_RightLowerLeg = 8;
const int Mech_RightFoot = 9;
const int Mech_Windshield = 10;

const size_t Mech_MAX_PARTS = 11;

////////////////////////////////////////////////////////////////////////////////
// Basic player definition

class BasicPlayer {
public:
   // Add goal
   virtual void addGoal (int /*goalNumber*/, int /*parameter*/)
   {}

   // Plan action
   virtual void plan (int /*planAheadTicks*/)
   {}

   // Get position
   virtual void getMatrix ()
   {}
private:
   vector <int> goals;
};

////////////////////////////////////////////////////////////////////////////////

const int MaxCritCount = 20;
const int MaxGroupCount = 5;

struct WeaponSystem {
   // Weapon type
   int weaponNum;
   const MWWeapons* weapon;

   // Pairs of these values describe mech/vehicle location
   int section[MaxCritCount], critical[MaxCritCount];

   // Store if a weapon is in a certain group
   bool isInGroup[MaxGroupCount];
};

////////////////////////////////////////////////////////////////////////////////
// Vehicle base class

class Vehicle: public RenderableObject
{
public:
   // Constructor, destructor
   Vehicle();
   virtual ~Vehicle()
   {
      if (texture != NULL)
         delete texture;
   }

   // Receive messages
   //virtual void receiveMessage (VirtualClass* sender, int MessageType);

   // Add geometry for a vehicle part
   virtual void addGeometry (int geometryType, Mesh* geometry,
      const Point3D& dist) = 0;
      // double tx, double ty, double tz, double jx, double jy, double jz) = 0;

   // Get section and critical information
   virtual size_t getSectionCount () const = 0;
   virtual const char* getSectionName (int i) const = 0;
   virtual size_t getCriticalCount (int section) const = 0;
   virtual int getCriticals (int section, int criticalSlot) const = 0;
   virtual void setCriticals (int section, int slot, int weaponType) = 0;

   // Query/set data
   static const int Info_Tonnage = 0;
   static const int Info_EngineRating = 1;
   static const int Info_EngineType = 2;
   static const int Info_JumpJets = 3;
   static const int Info_WalkingHexes = 4;
   static const int Info_RunningHexes = 5;
   static const int Info_Heat_Sinks = 6;
   static const int Info_Heat_Sink_Type = 7;
   static const int Info_Internal_Structure_Type = 8;
   static const int Info_Armor_Type = 9;
   static const int Info_TechType = 10;
   virtual int getInteger (int info) const;
   virtual bool setInteger (int info, int newVal);
   virtual double getAnimParam (size_t level, size_t ID) = 0;
   virtual void setAnimParam (size_t level, size_t ID, double val) = 0;

   static const int Armor_Front = 0;
   static const int Armor_Rear = 1;
   virtual DWORD getArmor(int section, int ArmorType) const = 0;
   virtual bool setArmor(int section, DWORD newVal, int ArmorType) = 0;
   virtual void setInternalStructure (int section, DWORD newVal) = 0;
   virtual DWORD getInternalStructure(int section) const = 0;

   virtual double getWeight () const = 0;

   virtual double calcInternalStructureWeight() const
   { return tonnage / 10.0; }

   double calcArmorWeight() const;
   virtual int countArmorPoints() const = 0;

   // Count number of criticals belonging to a weapon type in a section
   virtual int countWeaponCriticals (int currSection, int weaponNo) const;
   int countWeaponInAllSections(int weaponNo) const;

   // Add weapon
   virtual bool addWeapon(int currSection, int weaponNo) = 0;

   // Remove weapon - returns number of criticals removed
   virtual int removeWeapon(int currSection, int critNo, int weaponNo, 
                            int CritCount = -1) = 0;

   // Remove all equipment crititcals of given type
   int removeAll(int eqptType);

   int autoAddCriticals (int weaponType, int count, int /*critsPerUnit*/)
   {
      int added = 0;
      int section = getSectionCount()-1;

      while (section >= 0 && added < count) {
         int addedNow = addWeapon(section, weaponType);
         if (addedNow > 0) {
            added += addedNow;
         } else {
            --section;
         }
      }

      return added;
   }

   // Check for items that the user may not add or remove
   bool userMayAddDirectly (int section, int weaponNo);
   virtual bool userMayRemoveDirectly (int weaponNo);

   // Load entire mech
   //virtual bool load (const TCHAR* xmlFile, const TCHAR* cfgFile) = 0;
   virtual bool loadGeometry (const TCHAR* xmlFN, const char* partPath, 
                              XMLTreeConstIterator& iter, 
                              const TCHAR* textureFN);
   virtual void loadVehicleGeometry (XMLTreeConstIterator& iter, 
      const BGString& path, Texture* texture) = 0;
   virtual bool loadConfig (const TCHAR* cfgFN);

   bool loadFromMW2MEK (const MemoryBlock* mb);

   // Save Mech config to MEK file
   // File versions: 0: 31stcc/GBL, 1: Mercs, 2: Titanium
   void saveToMW2MEK (const TCHAR* mekFN, int fileVersion);

   // Check - and try to correct - config problems 
   // Returns 0 on success, and the number of a warning for the tr() function otherwise
   virtual int check() = 0;

   BGStringAttribute CFGFN, MVMKFN, textureFN, typeName;

   BGProperty <Point3D> p_position, p_front, p_left, p_up;
   BGProperty <double> p_scale;

   virtual double calcRealWeight() = 0;

   // Set current animation matrices
   virtual void setMatrices ()
   {}

   // Get/set Mech comment
   const char* getComment() const
   { return comment.getChars(); }
   void setComment( const char* newComment)
   { comment = newComment; }

   // Access weapon list
   size_t getWeaponListEntryCount() const
   { return weapons.getSize(); }
   const char* getWeaponListEntryName(int entry)
   { return weapons[entry].weapon->longName; }
   void setInGroup(int entry, int group, bool newVal);
   int getGroup(int entry);
   bool getInGroup(int entry, int group) const
   { return weapons[entry].isInGroup[group]; }

protected:
   // Time and position state
   //int lastTicks, nextTicks;
   //Point3D lastPos, nextPos, lastXDir, lastYDir, lastZDir, nextXDir, nextYDir, 
   //   nextZDir;
   int techType, ArmorType, InternalStructureType, HeatSinkType, EngineType;
   int tonnage, jumpJets, heatSinks, engineRating;
   Texture* texture;
   BGString comment;
   BGVector <WeaponSystem, WeaponSystemID> weapons;

   int calcMASCTonsAndSlots() const;
   int decodeMW2Critical (int16 ID, int HeatSinkType, bool subType31stCC);
   void checkBasicMechProperties (const MemoryBlock* mb, bool& subType31stCC);
   size_t countCriticals (const MemoryBlock* mb, WORD minID, WORD maxID);
   size_t countCriticals (const MemoryBlock* mb, WORD eqptID);

   // Check if a section may mount jump jets
   virtual bool mayMountJJs (int section) = 0;
};

////////////////////////////////////////////////////////////////////////////////
// Ground vehicles include tanks, wheeled cars, and hovercars

class GroundVehicle: public Vehicle
{
private:
   // Time and position state
   int lastTicks, nextTicks;
   Point3D lastPos, nextPos, lastXDir, lastYDir, lastZDir, nextXDir, nextYDir, 
      nextZDir;
   double turretTilt, turretTwist, speed;
};

////////////////////////////////////////////////////////////////////////////////
// Helicopters

class Helicopters: public Vehicle
{
private:
   // Time and position state
   int lastTicks, nextTicks;
   Point3D lastPos, nextPos, lastXDir, lastYDir, lastZDir, nextXDir, nextYDir, 
      nextZDir;
   double turretTilt, turretTwist, speed;
};

////////////////////////////////////////////////////////////////////////////////
// Winged aircraft

class WingedAircraft: public Vehicle
{
private:
   // Time and position state
   int lastTicks, nextTicks;
   Point3D lastPos, nextPos, lastXDir, lastYDir, lastZDir, nextXDir, nextYDir, 
      nextZDir;
   double turretTilt, turretTwist, speed;
};

////////////////////////////////////////////////////////////////////////////////

Vehicle* loadVehicle (const char* name, const char* partPath, 
                      const char* mvmkFN, /*const char* mekFN, */
                      const char* textureFN);

#endif
