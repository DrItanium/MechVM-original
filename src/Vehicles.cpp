////////////////////////////////////////////////////////////////////////////////
// Vehicle.cpp
// Vehicle control
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "Vehicles.h"
#include "FileCache.h"
#include "FramerateCounter.h"
#include "Config.h"
#include "MWBase.h"

#include "Mesh3.h"

////////////////////////////////////////////////////////////////////////////////

Vehicle::Vehicle()
: RenderableObject(VehicleID, NULL, NULL),
  p_position (Point3D()),
  p_front (Point3D(0.0, 0.0, 1.0)),
  p_left (Point3D(-1.0, 0.0, 0.0)),
  p_up (Point3D(0.0, 1.0, 0.0)),
  p_scale (0.01),
  texture (NULL)
{
}

////////////////////////////////////////////////////////////////////////////////
// Check for items that the user may not add

bool Vehicle::userMayAddDirectly (int section, int weaponNo)
{
   if (weaponNo == CE_Jump_Jet
   &&  !mayMountJJs(section))
   {
      return false;
   }

   if (weaponNo == Unused_Critical_Slot
   ||  weaponNo == CE_Engine
   ||  weaponNo == CE_Gyro
   ||  weaponNo == CE_Life_Support
   ||  weaponNo == CE_Cockpit
   ||  weaponNo == CE_Sensors
   ||  weaponNo == CE_Shoulder
   ||  weaponNo == CE_Upper_Arm_Actuator
   ||  weaponNo == CE_Hip
   ||  weaponNo == CE_Upper_Leg_Actuator
   ||  weaponNo == CE_Lower_Leg_Actuator
   ||  weaponNo == CE_Foot)
   //||  weaponNo == CE_Jump_Jet)
   {
      return false;
   } else {
      return true;
   }
}

////////////////////////////////////////////////////////////////////////////////

bool Vehicle::userMayRemoveDirectly (int weaponNo)
{
   if (weaponNo == Unused_Critical_Slot
   ||  weaponNo == CE_Engine
   ||  weaponNo == CE_Gyro
   ||  weaponNo == CE_Life_Support
   ||  weaponNo == CE_Cockpit
   ||  weaponNo == CE_Sensors
   ||  weaponNo == CE_Shoulder
   ||  weaponNo == CE_Upper_Arm_Actuator
   ||  weaponNo == CE_Hip
   ||  weaponNo == CE_Upper_Leg_Actuator
   ||  weaponNo == CE_Lower_Leg_Actuator
   ||  weaponNo == CE_Foot)
   //||  weaponNo == CE_Jump_Jet)
   {
      return false;
   } else {
      return true;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Remove all equipment crititcals of given type

int Vehicle::removeAll(int eqptType)
{
   int removed = 0;

   for (size_t i = 0; i < getSectionCount(); ++i) {
      int crits = countWeaponCriticals(i, eqptType);
      if (crits > 0)
         if (removeWeapon(i, 0, eqptType, crits))
            ++removed;
   }

   return removed;
}

////////////////////////////////////////////////////////////////////////////////
// Obtain mech information

int Vehicle::getInteger (int info) const
{
   switch (info) {
      case Info_Tonnage: return tonnage;
      case Info_EngineRating: return engineRating;
      case Info_EngineType: return EngineType;
      case Info_JumpJets: return jumpJets;
      case Info_WalkingHexes: return engineRating / tonnage;
      case Info_RunningHexes: return ((int) ((engineRating / tonnage + 0.5)*1.5));
      case Info_Heat_Sinks: return heatSinks;
      case Info_Heat_Sink_Type: return HeatSinkType;
      case Info_Internal_Structure_Type: return InternalStructureType;
      case Info_Armor_Type: return ArmorType;
      case Info_TechType: return techType;
   }

   return 0;
}

bool Vehicle::setInteger (int info, int newVal)
{
   switch (info) {
      case Info_Tonnage: 
         tonnage = newVal;
         break;
      case Info_EngineRating: 
         engineRating = newVal;
         break;
      case Info_EngineType: 
         EngineType = newVal;
         break;
      case Info_JumpJets: 
         jumpJets = newVal;
         break;
      case Info_Heat_Sinks: 
         heatSinks = newVal;
         break;
      case Info_Heat_Sink_Type: 
         HeatSinkType = newVal;
         break;
      case Info_Internal_Structure_Type: 
         InternalStructureType = newVal;
         break;
      case Info_Armor_Type: 
         ArmorType = newVal;
         break;
      default:
         return false;
   }

   calcRealWeight();
   return true;
}

////////////////////////////////////////////////////////////////////////////////

bool Vehicle::loadConfig (const TCHAR* cfgFN)
{
   MemoryBlock mekFile;
   if (mekFile.loadFromFile(cfgFN)) {
      loadFromMW2MEK(&mekFile);
      calcRealWeight();
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Count number of criticals belonging to a weapon type in a section

int Vehicle::countWeaponCriticals (int currSection, int weaponNo) const
{
   int i = 0;
   int found = 0;
   int max = getCriticalCount(currSection);

   while (i < max) {
      int w = getCriticals(currSection, i);
      if (w == weaponNo) {
         ++found;
      } 
      ++i;
   }

   return found;
}

int Vehicle::countWeaponInAllSections(int weaponNo) const
{
   int count = 0;

   for (size_t i = 0; i < getSectionCount(); ++i) {
      count += countWeaponCriticals(i, weaponNo);
   }

   return count;
}

////////////////////////////////////////////////////////////////////////////////

int Vehicle::calcMASCTonsAndSlots() const
{
   if (techType == Biped_Mech_Tech_Type_Clan) {
      return (int) ceil(tonnage / 25.0);
   } else {
      return (int) ceil(tonnage / 20.0);
   }
}

////////////////////////////////////////////////////////////////////////////////

double Vehicle::calcArmorWeight() const
{
   int armorPoints = countArmorPoints();
   double result = 0.0;

   if (ArmorType == ARMOR_TYPE_STD) 
      result = armorPoints / 16.0;
   else {
      if (techType == Biped_Mech_Tech_Type_Clan) {
         //result = armorPoints / 19.16; // average ap/tons, including rounding
         //return armorPoints / (1.2 * 16.0); // exact value, ignoring rounding

         // BattleTech exact value
         while (result * 16 * 1.2 < armorPoints)
            result += 0.5;
      } else {
         //result = armorPoints / 17,89; // average ap/tons, including rounding
         //return armorPoints / (1.12 * 16.0); // exact value, ignoring rounding

         // BattleTech exact value
         while (result * 16 * 1.12 < armorPoints)
            result += 0.5;
      }
   }

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// Constants for accessing MEK files

const size_t MW2_MEK_SectionOffset[8] = {
   0x018, // Head, 6 criticals
   0x068, // Center Torso, 12 criticals
   0x090, // Left Torso, 12 criticals
   0x040, // Right Torso, 12 criticals
   0x0E0, // Left Arm, 12 criticals
   0x0B8, // Right Arm, 12 criticals
   0x108, // Left Leg, 6 criticals
   0x130}; // Right Leg, 6 criticals

const int criticalOffset = 12;

////////////////////////////////////////////////////////////////////////////////
// Help functions for loading MEK files

size_t Vehicle::countCriticals (const MemoryBlock* mb, WORD eqptID)
{
   size_t result = 0;

   for (size_t section = 0; section < getSectionCount(); section++) 
   {
      int base = MW2_MEK_SectionOffset[section];
      for (size_t critical = 0; 
           critical < getCriticalCount(section);
           ++critical) 
      {
         DWORD offset = base+criticalOffset+2*critical;
         WORD val = mb->getWord(offset);
         if (val == eqptID) {
            result++;
         }
      }
   }

   return result;
}

size_t Vehicle::countCriticals (const MemoryBlock* mb, WORD minID, WORD maxID)
{
   size_t result = 0;

   for (size_t section = 0; section < 8; section++) 
      for (size_t critical = 0; 
           critical < getCriticalCount(section);
           ++critical) 
      {
         WORD val = mb->getWord(MW2_MEK_SectionOffset[section]+criticalOffset+2*critical);
         if (val >= minID && val < maxID) {
            result++;
         }
      }

   return result;
}

void Vehicle::checkBasicMechProperties (const MemoryBlock* mb, bool& subType31stCC)
{
   tonnage = mb->getByte(0);
   int walkingHexes = mb->getByte(4);
   engineRating = tonnage * walkingHexes;
   jumpJets = mb->getByte(8);
   heatSinks = mb->getByte(12); // unreliable, may be corrected later
   subType31stCC = false;
   techType = Biped_Mech_Tech_Type_IS_Level_1;
   int heatSinksInEngine = getInteger(Info_EngineRating) / 25;
   int firstHeadCritical = mb->getWord(MW2_MEK_SectionOffset[0]+criticalOffset);
   if (firstHeadCritical == 5900)
      subType31stCC = true;

   size_t CritsPerHeatSink, totalHeatSinkCrits, endoCrits, ffCrits, engineCrits;

   if (subType31stCC) {
      // MW2 supports standard and extra-large (XL) engines
      engineCrits = countCriticals (mb, 5850);
      ffCrits = countCriticals(mb, 9000, 9050);
      endoCrits = countCriticals (mb, 8000, 8050);
      techType = Biped_Mech_Tech_Type_Clan;
   } else {
      engineCrits = countCriticals (mb, 7850);
      endoCrits = countCriticals (mb, 9000, 9050);
      ffCrits = countCriticals(mb, 9500, 9550);
   }

   size_t ClanWeaponCrits = countCriticals (mb, 1, 3499);
   size_t ISWeaponCrits =  countCriticals (mb, 3500, 6999);
   if (ClanWeaponCrits > ISWeaponCrits)
      techType = Biped_Mech_Tech_Type_Clan;

   // Check for Endo Steel, Ferro Firbous and XL engines first
   // These are very reliable in determining whether we are
   // dealing with a IS/Clan Mech
   if (endoCrits >= 14) {
      InternalStructureType = IS_Endo_Steel;
      techType = Biped_Mech_Tech_Type_IS_Level_2;
   } else if (endoCrits >= 7) {
      InternalStructureType = Clan_Endo_Steel_Internals;
      techType = Biped_Mech_Tech_Type_Clan;
   } else 
      InternalStructureType = Unused_Critical_Slot;

   if (ffCrits == 7) {
      ArmorType = ARMOR_TYPE_FERRO_FIBROUS;
      techType = Biped_Mech_Tech_Type_Clan;
   } else if (ffCrits == 14) {
      ArmorType = ARMOR_TYPE_FERRO_FIBROUS;
      techType = Biped_Mech_Tech_Type_IS_Level_2;
   } else
      ArmorType = ARMOR_TYPE_STD;

   if (engineCrits == 10) {
      EngineType = ENGINE_TYPE_XL;
      techType = Biped_Mech_Tech_Type_Clan;
   } else if (engineCrits == 12) {
      EngineType = ENGINE_TYPE_XL; // Not in DOS versions of 31stcc, GBL?
      techType = Biped_Mech_Tech_Type_IS_Level_2;
   } else {
      EngineType = ENGINE_TYPE_STD;
   }

   if (subType31stCC) {
      // Determine number and type of heat sinks
      HeatSinkType = IS_Heat_Sink;
      CritsPerHeatSink = countCriticals (mb, 6001);
      totalHeatSinkCrits = countCriticals (mb, 6000, 6100);
      if (CritsPerHeatSink == 0) {
         // Internal heat sinks only
         if (totalHeatSinkCrits == 0) {
            // No heat sink criticals: unclear whether there are heat sinks
            // that require tonnage (10 < #hs < engine / 25)
            HeatSinkType = Clan_Double_Heat_Sink;
            heatSinks /= 2; // convert double->single
            int minReasonableHS = bgmin(10, heatSinksInEngine);
            if (heatSinks < minReasonableHS)
               heatSinks = minReasonableHS;
            if (heatSinks > heatSinksInEngine)
               heatSinks = heatSinksInEngine; 
         } else {
            // Error: first heat sink number skipped?
            heatSinks = heatSinksInEngine + totalHeatSinkCrits;
         }
      }
   } else {
      // Determine number and type of heat sinks
      CritsPerHeatSink = countCriticals (mb, 8001);
      totalHeatSinkCrits = countCriticals (mb, 8001, 8100);
      if (CritsPerHeatSink == 0) {
         // No heat sink criticals: unclear whether there are heat sinks
         // that require tonnage (10 < #hs < engine / 25)
         if (techType == Biped_Mech_Tech_Type_Clan) {
            HeatSinkType = Clan_Double_Heat_Sink;
         } else if (heatSinks > 2*heatSinksInEngine) {
            heatSinks = heatSinksInEngine; // no crits, no more heat sinks
            HeatSinkType = IS_Double_Heat_Sink;
         } else if (heatSinks > heatSinksInEngine && heatSinks % 2 == 0) {
            HeatSinkType = IS_Double_Heat_Sink;
         } else {
            HeatSinkType = IS_Heat_Sink;
            int minReasonableHS = bgmin(10, heatSinksInEngine);
            if (heatSinks < minReasonableHS)
               heatSinks = minReasonableHS;
         }
      }
   }

   // Determine number and type of heat sinks
   switch (CritsPerHeatSink) {
      //case 0: break; // treated above ...
      case 1:
         HeatSinkType = IS_Heat_Sink;
         heatSinks = heatSinksInEngine + totalHeatSinkCrits;
         break;
      case 2:
         HeatSinkType = Clan_Double_Heat_Sink;
         heatSinks = heatSinksInEngine + totalHeatSinkCrits / 2;
         techType = Biped_Mech_Tech_Type_Clan;
         break;
      case 3:
         HeatSinkType = IS_Double_Heat_Sink;
         heatSinks = heatSinksInEngine + totalHeatSinkCrits / 3;
         techType = Biped_Mech_Tech_Type_IS_Level_2;
         break;
   }
}

int Vehicle::decodeMW2Critical (int16 ID, int HeatSinkType, bool subType31stCC)
{
   int result = ID_UNUSED_MW2;

   // MW2 treats Infernos as special weapons, we treat them as special munitions
   /*if ((ID > 3100 && ID < 3200 && !subType31stCC)
   ||  (ID > 3300 && ID < 3400 && !subType31stCC)) {
      result = Clan_SRM_2;
   } else
   if ((ID > 13300 && ID < 13400 && !subType31stCC) 
   ||  (ID > 13100 && ID < 13200 && !subType31stCC)) {
      result = Clan_SRM_Ammo_Inferno;
   } else
   if ((ID > 8000 && ID < 8100 && !subType31stCC)
   ||  (ID > 6000 && ID < 6100 && subType31stCC)) {
      result = HeatSinkType;
   } else*/ 

   if (ID > 0) {
      int cmpID = IS_Energy_Weapon_Base;
      while (result == ID_UNUSED_MW2 && cmpID < (int) WeaponsCount) {
         const MWWeapons* w = getWeaponStats(cmpID);
         if ((w->MW2_ID <= ID && ID < w->MW2_ID + w->MaxCount && !subType31stCC)
         ||  (w->MW2_31st_ID <= ID && ID < w->MW2_31st_ID + w->MaxCount && subType31stCC))
         {
            result = cmpID;
         } else {
            cmpID++;
         }
      }
   }

   if (result == ID_UNUSED_MW2)
      result = Unused_Critical_Slot;

   return result;
}

////////////////////////////////////////////////////////////////////////////////

void Vehicle::setInGroup(int entry, int group, bool newVal)
{
   for (size_t i = 0; i < MaxGroupCount; ++i)
      weapons[entry].isInGroup[i] = false;
   weapons[entry].isInGroup[group] = newVal;
}

int Vehicle::getGroup(int entry)
{
   for (size_t i = 0; i < MaxGroupCount; ++i)
      if (weapons[entry].isInGroup[i])
         return i;

   return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Load config from MechWarrior 2 MEK file

bool Vehicle::loadFromMW2MEK (const MemoryBlock* mb) 
{
   bool subType31stCC;
   checkBasicMechProperties (mb, subType31stCC);

   // Extract stats & criticals
   for (size_t i = 0; i < getSectionCount(); i++) {
      size_t base = MW2_MEK_SectionOffset[i];

      DWORD InternalStructure = mb->getDWord(base+8);
      setInternalStructure (i, InternalStructure);

      DWORD FrontArmor = mb->getDWord(base);
      DWORD RearArmor = mb->getDWord(base+4);
      setArmor (i, FrontArmor, Vehicle::Armor_Front);
      setArmor (i, RearArmor, Vehicle::Armor_Rear);

      for (size_t j = 0; j < getCriticalCount(i); j++) {
         WORD ID = mb->getWord(base+12+2*j);
         if (ID > 0) {
            int crit = decodeMW2Critical (ID, HeatSinkType, subType31stCC);
            setCriticals(i, j, crit);
         } else
            setCriticals(i, j, Unused_Critical_Slot);
      }
   }

   size_t offset = 0x158 + mb->getDWord(0x10)*8 + mb->getDWord(0x14)*8;
   comment.copyPart((const TCHAR*) mb->getPtr(), offset, mb->getSize());

   // Correct critical hits table
   if (techType == Biped_Mech_Tech_Type_Clan) {
      for (size_t i = 0; i < getSectionCount(); i++) {
         for (size_t j = 0; j < getCriticalCount(i); j++) {
            int16 crit = getCriticals(i, j);
            if (crit == IS_Endo_Steel)
               setCriticals (i, j, Clan_Endo_Steel_Internals);
            else if (crit == IS_Ferro_Fibrous)
               setCriticals (i,j, Clan_Ferro_Fibrous);
            else if (crit == IS_Heat_Sink)
               setCriticals (i,j, Clan_Double_Heat_Sink);
         }
      }
   }

   // Construct weapon table
   DWORD WeaponCount = mb->getDWord(0x10);
   weapons.setSize(WeaponCount);
   for (size_t i = 0; i < WeaponCount; ++i) {
      size_t base = 0x158 + i * 8;
      WORD wID = mb->getWord(base);
      weapons[i].weaponNum = decodeMW2Critical (wID, HeatSinkType, subType31stCC);
      weapons[i].weapon = getWeaponStats(weapons[i].weaponNum);
      size_t weaponGroup = mb->getWord(base+5);
      size_t j;

      for (j = 0; j < MaxCritCount; ++j) {
         weapons[i].section[j] = -1;
         weapons[i].critical[j] = -1;
      }

      j = 0;
      size_t k = 0, crits = 0;
      while (j < getSectionCount()) {
         size_t base = MW2_MEK_SectionOffset[j];
         while (k < getCriticalCount(j)) {
            WORD oID = mb->getWord(base+12+2*k);
            if (crits >= MaxCritCount) {
               printf("%s: too many criticals\n", weapons[i].weapon->getLongName());
            } else if (wID == oID) { // Exact match needed
               weapons[i].section[crits] = j;
               weapons[i].critical[crits] = k;
               ++crits;
            }
            ++k;
         }
         ++j;
         k = 0;
      }

      for (size_t j = 0; j < MaxGroupCount; ++j)
         weapons[i].isInGroup[j] = false;
      if (weaponGroup < 0 || weaponGroup > MaxGroupCount)
         weaponGroup = 0;
      weapons[i].isInGroup[weaponGroup] = true;
   }

   calcRealWeight();

   return true; 
}

////////////////////////////////////////////////////////////////////////////////
// Save Mech config to MEK file
// File versions: 0: 31stcc/GBL, 1: Mercs, 2: Titanium

void Vehicle::saveToMW2MEK (const TCHAR* mekFN, int fileVersion)
{
   size_t listedWeapons = 0;
   size_t ammoCount = 0;

   int weaponCritCount [WeaponsCount];
   int weaponTableCount[WeaponsCount];
   for (size_t i = 0; i < WeaponsCount; ++i) {
      weaponCritCount[i] = 0;
      weaponTableCount[i] = 0;
   }

   // Count equipment criticals
   for (size_t i = 0; i < getSectionCount(); i++) {
      for (size_t j = 0; j < getCriticalCount(i); j++) {
         WORD ID = getCriticals(i, j);
         ++weaponCritCount[ID];
      }
   }

   // Determine size of table for exported weapons and ammo
   for (size_t i = 0; i < WeaponsCount; ++i) {
      int critCount = weaponCritCount[i];
      const MWWeapons* w = getWeaponStats(i);
      if (w != NULL && critCount > 0) {
         if ((w->flags & MWWeaponFlag_Ammo) > 0)
            ammoCount += critCount;
         else if ((w->flags & MWWeaponFlag_IncludeInWeaponList) > 0)
            listedWeapons += critCount / w->Criticals;
      }
   }

   size_t mbSize = 394 + 8 * listedWeapons + 8 * ammoCount;

   // Allocate MemoryBlock
   MemoryBlock* mb = new MemoryBlock(mbSize);
   mb->fill(0, mbSize, 0);

   // Set basic information
   mb->setByte(0, tonnage);
   int walkingHexes = engineRating / tonnage;
   mb->setByte(4, walkingHexes);
   engineRating = tonnage * walkingHexes;
   mb->setByte(8, jumpJets);
   mb->setByte(12, heatSinks); // unreliable, may be corrected later
   mb->setByte(16, listedWeapons);
   mb->setByte(20, ammoCount);

   if (fileVersion == 2 && HeatSinkType != IS_Heat_Sink)
      mb->setByte(12, 2*heatSinks);
   else
      mb->setByte(12, heatSinks);

   // Set stats & criticals
   for (size_t i = 0; i < getSectionCount(); i++) {
      size_t base = MW2_MEK_SectionOffset[i];
      mb->setDWord(base, getArmor(i, Armor_Front));
      mb->setDWord(base+4, getArmor(i, Armor_Rear));
      mb->setDWord(base+8, getInternalStructure(i));
      mb->setByte(base+36, getCriticalCount(i));
      mb->setByte(base+38, 1);

      for (size_t j = 0; j < getCriticalCount(i); j++) {
         WORD ID = getCriticals(i, j);
         const MWWeapons* w = getWeaponStats(ID);
         WORD MW2ID = ID_UNUSED_MW2;
         if (w != NULL) {
            if (fileVersion == 0) {
               MW2ID = w->MW2_31st_ID;
            } else {
               MW2ID = w->MW2_ID;
            }
            if (w->Criticals > 0 && (w->flags & MWWeaponFlag_CountedInMEK) > 0) {
               MW2ID += weaponTableCount[ID]/w->Criticals + 1;
               ++weaponTableCount[ID];
            }
         } else if (fileVersion == 0)
            MW2ID = 0;
         mb->setWord(base+12+2*j, MW2ID);
      }
   }

   // Reset weaponTableCount
   for (size_t i = 0; i < WeaponsCount; ++i) {
      const MWWeapons* w = getWeaponStats(i);
      if (w != NULL)
         if (w->Criticals != 0)
            weaponTableCount[i] = weaponCritCount[i] / w->Criticals;
   }

   // Fill in weapons list
   /*size_t offset = 0x150 + 8*listedWeapons;
   for (size_t i = 0; i < WeaponsCount; i++) {
      const MWWeapons* w = getWeaponStats(i);
      if (w != NULL)
      if ((w->flags & MWWeaponFlag_IncludeInWeaponList) > 0)
      if ((w->flags & MWWeaponFlag_Ammo) == 0)
      {
         while (weaponTableCount[i] > 0) {
            WORD weaponID = w->MW2_ID;
            if (fileVersion == 0)
               weaponID = w->MW2_31st_ID;
            if ((w->flags & MWWeaponFlag_CountedInMEK) > 0)
               weaponID += weaponTableCount[i];
            mb->setDWord(offset, weaponID);
            mb->setDWord(offset+4, 0xffFFffFF);
            offset -= 8;
            weaponTableCount[i] -= 1;
         }
      }
   }*/

   // Fill in weapons list
   for (size_t i = 0; i < weapons.getSize(); ++i) {
      size_t offset = 0x158 + 8*i;
      int section = weapons[i].section[0];
      int crit = weapons[i].critical[0];
      size_t base = MW2_MEK_SectionOffset[section];
      int weaponID = mb->getWord(base+12+2*crit);
      mb->setDWord(offset, weaponID);
      //mb->setDWord(offset+4, 0xffFFffFF);
      mb->setDWord(offset+4, 0);
      mb->setByte(offset+5, getGroup(i));
   }

   // Scan all sections for ammo and write the ammo table
   size_t offset = 0x158 + 8*listedWeapons;
   for (size_t i = 0; i < getSectionCount(); ++i) {
      size_t base = MW2_MEK_SectionOffset[i];
      for (size_t j = 0; j < getCriticalCount(i); ++j) {
         int MW2ID = mb->getWord(base+12+2*j);
         int ID = decodeMW2Critical(MW2ID, HeatSinkType, fileVersion == 0);
         const MWWeapons* a = getWeaponStats(ID);

         if (a != NULL)
         if ((a->flags & MWWeaponFlag_Ammo) > 0)
         {
            // Find weapon type associated with this weapon
            // (there can be only one in MW2)
            int wCount = 0;
            size_t i = 0; 
            const MWWeapons * w = NULL;
            while (i < MaxAmmoTypes && w == NULL) {
               int wID = a->AmmoTypes[i];
               w = getWeaponStats(wID);
               if (w != NULL) {
                  wCount = weaponCritCount[wID] / w->Criticals;
                  if (wCount == 0) {
                     w = NULL;
                  }
               }
               ++i;
            }

            // Write ammo/weapon pair entries
            mb->setDWord(offset, MW2ID);

            int wFID = 0;
            if (wCount > 0) {
               wFID = w->MW2_ID;
               if (fileVersion == 0)
                  wFID = w->MW2_31st_ID;

               // Use MW2ID to map this ammo to a weapon
               int ammoNum = MW2ID - w->MW2_ID;
               if (fileVersion == 0) {
                  ammoNum = MW2ID - w->MW2_31st_ID - 1;
               }
               wFID += ammoNum % wCount + 1;
            }

            mb->setDWord(offset+4, wFID);
            offset += 8;
         }
      }
   }

   // Write user comments
   //unsigned char comment[50] = "MechVM Mech";
   //mb->copy(offset, (const unsigned char*) (&comment), 50);
   if (comment.getLength() > 49)
      comment.setLength(49);
   mb->copy(offset, (const unsigned char*) comment.getChars(), comment.getLength());

   mb->saveToFile(mekFN);
}

////////////////////////////////////////////////////////////////////////////////
// Load entire mech

bool Vehicle::loadGeometry (const TCHAR* xmlFN, const char* partPath, 
                           XMLTreeConstIterator& iter, 
                           const TCHAR* textureFN)
{
   // Texture must be loaded first
   if (textureFN != NULL) {
      if (texture == NULL)
         texture = new Texture();
      if (!texture->loadFromFile(textureFN)) {
         delete texture;
         texture = NULL;
      }
   }

   loadVehicleGeometry (iter, partPath, texture);
   return true;
}

////////////////////////////////////////////////////////////////////////////////

#include "BipedMech.h"
#include "QuadMech.h"

Vehicle* loadVehicle (const char* name, const char* mvmkFN, const char* partPath,
                      const char* textureFN)
{
   XMLTree xml;
   if (!xml.loadFromFile(mvmkFN)) 
      return NULL;

   Vehicle * vehicle = NULL;
   XMLTreeConstIterator* iter = xml.getConstIterator();
   const char * nodeName = iter->getNodeName();
   if (strcmp(nodeName, "mech") == 0)
      vehicle = new BipedMech ();
   else
      vehicle = new QuadMech();

   vehicle->setName(name);

   if (!vehicle->loadGeometry(mvmkFN, partPath, *iter, textureFN))
   {
      delete vehicle;
      delete iter;
      return NULL;
   } else
      delete iter;
      return vehicle;
}

