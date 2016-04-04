////////////////////////////////////////////////////////////////////////////////
// MWBase.cpp
// Copyright Bjoern Ganster 2007-2009
////////////////////////////////////////////////////////////////////////////////

#include "MWBase.h"
#include "string.h"
#include "BGVector.h"

// Prevent warning "Use strcpy_s instead of strcpy" 
//#pragma warning( disable : 4996 )

//#ifndef strcpy_s
//#define strcpy_s strcpy
//#endif

//#ifndef strcat_s
//#define strcat_s strcat
//#endif

MWWeapons* weapons;

////////////////////////////////////////////////////////////////////////////////
// Get file type ID from name

int getFileTypeFromExtension (const TCHAR* FN)
{
   size_t len = strlen (FN);
   if (len > 3) {
      char a = lowerCase (FN[len-3]);
      char b = lowerCase (FN[len-2]);
      char c = lowerCase (FN[len-1]);

      if (a == 'x'
      &&  b == 'e'
      &&  c == 'l')
      {
         return FT_XEL;
      } else
      if (a == 'w'
      &&  b == 't'
      &&  c == 'b')
      {
         return FT_WTB;
      } else
      if (a == 'm'
      &&  b == 'e'
      &&  c == 'k')
      {
         return FT_Mech;
      } else
      if (a == 'p'
      &&  b == 'r'
      &&  c == 'j')
      {
         return FT_PRJ;
      } else
      if (a == 's'
      &&  b == 'h'
      &&  c == 'p')
      {
         return FT_SHP;
      } else
      if (a == 's'
      &&  b == 'f'
      &&  c == 'l')
      {
         return FT_SFL;
      } else
      if (a == 'w'
      &&  b == 'a'
      &&  c == 'v')
      {
         return FT_WAV;
      } else
      if (a == 'c'
      &&  b == 'o'
      &&  c == 'l')
      {
         return FT_COL;
      } else
      if (a == 'e'
      &&  b == 'f'
      &&  c == 'a')
      {
         return FT_EFA;
      } else
      if (a == 'b'
      &&  b == 'w'
      &&  c == 'd')
      {
         return FT_BWD;
      } else
      if (a == '1'
      &&  b == '0'
      &&  c == '0')
      {
         return FT_FNT;
      }
      if (a == 'f'
      &&  b == 'l'
      &&  c == 't')
      {
         return FT_FLT;
      } else
      if (a == 'z'
      &&  b == 'b'
      &&  c == 'd')
      {
         return FT_ZBD;
      } else
      if (a == 'm'
      &&  b == 'w'
      &&  c == '2')
      {
         return FT_MW2_DB;
      } else
      if (a == 't'
      &&  b == 'e'
      &&  c == 'x')
      {
         return FT_TEX;
      } else
      if (a == 'p'
      &&  b == 'c'
      &&  c == 'x')
      {
         return FT_PCX;
      } else
      if (a == '5'
      &&  b == '5'
      &&  c == '5')
      {
         // Warning! Both MW2 and MW3 use format named .555, but they
         // have a different header ...
         return FT_MW2_555;
      } else 
      if (a == 'i'
      &&  b == 'm'
      &&  c == 'g')
      {
         return FT_ISO_9660;
      } else 
      if (a == 'i'
      &&  b == 's'
      &&  c == 'o')
      {
         return FT_ISO_9660;
      }
   }

   return FT_Unsupported;
}

////////////////////////////////////////////////////////////////////////////////
// Get file extension from file type ID

const TCHAR* getMWFileExt (int FileTypeID)
{
   switch (FileTypeID) {
      case FT_XEL:
         return ".XEL";
      case FT_WTB:
         return ".WTB";
      case FT_Mech:
         return ".MEK";
      case FT_PRJ:
         return ".PRJ";
      case FT_SHP:
         return "SHP";
      case FT_SFL:
         return "SFL";
      case FT_WAV:
         return "WAV";
      case FT_COL:
         return "COL";
      case FT_DLL:
         return "DLL";
      case FT_EFA:
         return "EFA";
      case FT_FORM:
         return "FORM";
      case FT_FNT:
         return "FNT";
      case FT_BWD:
         return "BWD";
      case FT_ZBD:
         return "ZBD";
      case FT_FLT:
         return "FLT";
      case FT_TEX:
         return "TEX";
      case FT_PCX:
         return "PCX";
      case FT_MW2_555:
      case FT_MW3_555:
         return "555";
      default:
         return nullptr;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Get file type ID from file contents

int getFileTypeFromContent (const MemoryBlock* block)
{
   if (block->getSize() > 4) {
      if (block->getUByte (0) == '1'
      &&  block->getUByte (1) == '.'
      &&  block->getUByte (2) == '1'
      &&  block->getUByte (3) == '0')
      {
         return FT_SHP;
      } else
      if (block->getUByte (0) == 'R'
      &&  block->getUByte (1) == 'I'
      &&  block->getUByte (2) == 'F'
      &&  block->getUByte (3) == 'F')
      {
         return FT_WAV;
      } else
      if (block->getUByte (0) == 'M'
      &&  block->getUByte (1) == 'Z')
      {
         return FT_DLL;
      } else
      if (block->getUByte (4) == 0xEF
      &&  block->getUByte (5) == 0x0A
      &&  block->getUByte (6) == 0x05
      &&  block->getUByte (7) == 0x01)
      {
         return FT_EFA;
      } else
      if (block->getUByte (5) == 'F'
      &&  block->getUByte (6) == 'O'
      &&  block->getUByte (7) == 'R'
      &&  block->getUByte (8) == 'M')
      {
         return FT_FORM;
      } else
      if (block->getUByte (0) == '1'
      &&  block->getUByte (1) == '.'
      &&  block->getUByte (2) == 0
      &&  block->getUByte (3) == 0)
      {
         return FT_FNT;
      } else
      if (block->getUByte (0) == 'B'
      &&  block->getUByte (1) == 'W'
      &&  block->getUByte (2) == 'D')
      {
         return FT_BWD;
      } else
      if (block->getUByte (0) == '1'
      &&  block->getUByte (1) == '.'
      &&  block->getUByte (2) == '2'
      &&  block->getUByte (3) == '2')
      {
         return FT_TEX;
      } else
      if (block->getUByte (0) == 0x0A
      &&  block->getUByte (1) == 0x05
      &&  block->getUByte (2) == 0x01
      &&  block->getUByte (3) == 0x08)
      {
         return FT_PCX;
      } else
      if (block->getUByte (0) == 0x05
      &&  block->getUByte (1) == 0x00
      &&  block->getUByte (2) == 0x00
      &&  block->getUByte (3) == 0x00)
      {
         return FT_MW3_555;
      }
   }

   // Check for ZBD
   size_t i = 0;
   char c;
   do {
      c = block->getByte(i);
      ++i;
   } while (i < 29 && c != '.');
   if (c != '.') {
      if (block->getByte(i) == 'f'
      &&  block->getByte(i+1) == 'l'
      &&  block->getByte(i+2) == 't')
      {
         return FT_FLT;
      }
   }

   return FT_Unsupported;
}

////////////////////////////////////////////////////////////////////////////////

void copyWeapon (int oldID, int ID)
{
   weapons[ID].Heat = weapons[oldID].Heat;
   weapons[ID].MaxDamage = weapons[oldID].MaxDamage;
   weapons[ID].MinDamage = weapons[oldID].MinDamage;
   weapons[ID].MinimumRangeInMeters = weapons[oldID].MinimumRangeInMeters;
   weapons[ID].MaxDamageRange = weapons[oldID].MaxDamageRange;
   weapons[ID].MaxRangeInMeters = weapons[oldID].MaxRangeInMeters;
   weapons[ID].TonnageTimes100 = weapons[oldID].TonnageTimes100;
   weapons[ID].Criticals = weapons[oldID].Criticals;
   weapons[ID].ShotsPerTon = weapons[oldID].ShotsPerTon;
   weapons[ID].AvailableInYear = weapons[oldID].AvailableInYear;
   weapons[ID].CBills = weapons[oldID].CBills;
   memcpy(&weapons[ID].longName[0], weapons[oldID].getLongName(), LongWeaponNameLen);
   memcpy(&weapons[ID].shortName[0], weapons[oldID].getShortName(), ShortWeaponNameLen);
   weapons[ID].MW2_31st_ID = weapons[oldID].MW2_31st_ID;
   weapons[ID].MW2_ID = weapons[oldID].MW2_ID;
   weapons[ID].MaxCount = weapons[oldID].MaxCount;
   weapons[ID].inInventory = weapons[oldID].inInventory;
   weapons[ID].flags = weapons[oldID].flags;

   for (size_t i = 0; i < MaxAmmoTypes; i++)
      weapons[ID].AmmoTypes[i] = weapons[oldID].AmmoTypes[i];
}

void setWeaponNames(int ID, const char* shortName, const char* longName)
{
   int slen = strlen(shortName);
   int llen = strlen(longName);

   if (slen+1 < ShortWeaponNameLen) {
      memcpy (&weapons[ID].shortName[0], shortName, slen+1);
   } else {
      weapons[ID].shortName[0] = 0;
      printf ("%s: too long as a short weapon name (%i>%i)\n", shortName, slen,
         ShortWeaponNameLen);
   }
   if (llen+1 < LongWeaponNameLen) {
      memcpy (&weapons[ID].longName[0], longName, llen+1);
   } else {
      weapons[ID].longName[0] = 0;
      printf ("%s: too long as a long weapon name (%i>%i)\n", longName, llen,
         LongWeaponNameLen);
   }
}

inline void setWeaponNames(int ID, const char* name)
{
   setWeaponNames(ID, name, name);
}

void setWeaponAmmoPair(int wID, int aID)
{
   int i = 0;
   MWWeapons& w = weapons[wID];
   while (w.AmmoTypes[i] != 0 && i < MaxAmmoTypes)
      ++i;
   if (i < MaxAmmoTypes)
      w.AmmoTypes[i] = aID;
   else
      printf ("%s has too many ammo types\n", w.longName);

   int j = 0;
   MWWeapons& a = weapons[aID];
   while (a.AmmoTypes[j] != 0 && j < MaxAmmoTypes)
      ++j;
   if (j < MaxAmmoTypes)
      a.AmmoTypes[j] = wID;
   else
      printf ("%s has too few weapon types\n", a.longName);
}

////////////////////////////////////////////////////////////////////////////////

// Initialize MW weapons list
// While doing this in a function requires a lot of code, it is nicely
// self-commenting 
void fillWeaponsList()
{
   weapons = new MWWeapons[WeaponsCount];
   BGString shortName, longName;
   //printf("%i bytes for weapon table\n", sizeof(MWWeapons) * WeaponsCount);

   for (size_t ID = 0; ID < WeaponsCount; ID++) {
      weapons[ID].Heat = 0;
      weapons[ID].MaxDamage = 0; // Maximum weapon damage
      weapons[ID].MinDamage = 0; // Minimum weapon damage at maximum range
      weapons[ID].MaxDamageRange = 0; // Maximum range where the weapon does its full damage
      weapons[ID].MaxRangeInMeters = 0; // The maximum range for the weapon does minimum damage
      weapons[ID].MinimumRangeInMeters = 0;
      weapons[ID].TonnageTimes100 = 0;
      weapons[ID].Criticals = 1;
      weapons[ID].ShotsPerTon = 0; // If positive, this record describes ammo
      weapons[ID].AvailableInYear = 2820; // Year of field/open market availability
      weapons[ID].CBills = 0;
      for (size_t j = 0; j < MaxAmmoTypes; j++)
         weapons[ID].AmmoTypes[j] = 0; // Ammo that this weapon can use
      setWeaponNames(ID, "");

      // If a weapon does not exist in 31stcc+gbl DOS, setting MW2_31st_ID = 0 
      // and MaxCount = 300 causes MechVM to misunderstand the weapon
      weapons[ID].MW2_31st_ID = ID_UNUSED_MW2;

      weapons[ID].MW2_ID = ID_UNUSED_MW2;
      weapons[ID].MaxCount = 100;
      weapons[ID].inInventory = 100;
      weapons[ID].flags = MWWeaponFlagsStdWeapon;
   }

   int ID = IS1_Flamer;
   weapons[ID].Heat = 3;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2025; // Year of field/open market availability
   weapons[ID].CBills = 7500;
   setWeaponNames (ID, "Flamer");
   weapons[ID].MW2_ID = 5100;
 
   ID = IS1_Small_Laser;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 3; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 2025; // Year of field/open market availability
   weapons[ID].CBills = 11250;
   setWeaponNames (ID, "SLaser", "Small Laser");
   weapons[ID].MW2_ID = 5700;

   ID = IS1_Medium_Laser;
   weapons[ID].Heat = 3;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2025; // Year of field/open market availability
   weapons[ID].CBills = 40000;
   setWeaponNames (ID, "MLaser", "Medium Laser");
   weapons[ID].MW2_ID = 5600;

   ID = IS1_Large_Laser;
   weapons[ID].Heat = 8;
   weapons[ID].MaxDamage = 8; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 2025; // Year of field/open market availability
   weapons[ID].CBills = 100000;
   setWeaponNames (ID, "LLaser", "Large Laser");
   weapons[ID].MW2_ID = 5500;

   ID = IS1_PPC;
   weapons[ID].Heat = 10;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].Criticals = 3;
   weapons[ID].AvailableInYear = 2460; // Year of field/open market availability
   weapons[ID].CBills = 200000;
   setWeaponNames (ID, "PPC");
   weapons[ID].MW2_ID = 5300;

   ID = IS2_Small_Pulse_Laser;
   weapons[ID].Heat = 2;
   weapons[ID].MaxDamage = 3; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = 16000;
   setWeaponNames (ID, "SPLaser", "Small Pulse Laser");
   weapons[ID].MW2_ID = 6000;

   ID = IS2_Medium_Pulse_Laser;
   weapons[ID].Heat = 4;
   weapons[ID].MaxDamage = 6; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 180; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 200;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = 60000;
   setWeaponNames (ID, "MPLaser", "Medium Pulse Laser");
   weapons[ID].MW2_ID = 5900;

   ID = IS1_Large_Pulse_Laser;
   weapons[ID].Heat = 10;
   weapons[ID].MaxDamage = 9; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 300; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = 175000;
   setWeaponNames (ID, "LPLaser", "Large Pulse Laser");
   weapons[ID].MW2_ID = 5800;

   ID = IS2_ER_Small_Laser;
   weapons[ID].Heat = 2;
   weapons[ID].MaxDamage = 3; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 150; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 2620; // Year of field/open market availability
   weapons[ID].CBills = 11250;
   weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
   setWeaponNames (ID, "ER SLas", "ER Small Laser");

   ID = IS2_ER_Medium_Laser;
   weapons[ID].Heat = 5;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 360; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2620; // Year of field/open market availability
   weapons[ID].CBills = 80000;
   weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
   setWeaponNames (ID, "ER MLas", "ER Medium Laser");

   ID = IS2_ER_Large_Laser;
   weapons[ID].Heat = 12;
   weapons[ID].MaxDamage = 8; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 570; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 2620; // Year of field/open market availability
   weapons[ID].CBills = 200000;
   weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
   setWeaponNames (ID, "ER LLaser", "ER Large Laser");
   weapons[ID].MW2_ID = 5400;

   ID = IS2_ER_PPC;
   weapons[ID].Heat = 15;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 690; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].Criticals = 3;
   weapons[ID].AvailableInYear = 2760; // Year of field/open market availability
   weapons[ID].CBills = 300000;
   setWeaponNames (ID, "ER PPC", "ER PPC");
   weapons[ID].MW2_ID = 5200;

   ID = IS1_Machine_Gun;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 1900; // Year of field/open market availability
   weapons[ID].CBills = 5000;
   //weapons[ID].AmmoTypes[0] = IS1_Machine_Gun_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "MG", "Machine Gun");
   weapons[ID].MW2_ID = 4300; //???
   
   ID = IS1_Machine_Gun_Ammo;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   //weapons[ID].Criticals = 1;
   weapons[ID].ShotsPerTon = 200; // If positive, this record describes ammo
   weapons[ID].AvailableInYear = 1900; // Year of field/open market availability
   weapons[ID].CBills = 1000;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   setWeaponNames (ID, "A(MG)", "Ammo (Machine Gun)");
   weapons[ID].MW2_ID = 15400; // ???
   setWeaponAmmoPair (IS1_Machine_Gun, IS1_Machine_Gun_Ammo);

   const char* ACnums[] = {"20", "10", "5", "2"};
   for (int i = 0; i < 4; i++) {
      ID = IS1_Autocannon_20+i;
      BGString shortName = "AC";
      shortName += ACnums[i];
      BGString longName = "Autocannon/";
      longName += ACnums[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      weapons[ID].Heat = 1;
      weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
      weapons[ID].MW2_ID = 4800-i*100;
      weapons[ID].AvailableInYear = 2250; // Year of field/open market availability
      //weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo; // Ammo that this weapon can use
      setWeaponAmmoPair (ID, IS1_Autocannon_Ammo);
   }

   ID = IS1_Autocannon_2;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 720; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 600;
   //weapons[ID].Criticals = 1;
   weapons[ID].CBills = 75000;

   ID = IS1_Autocannon_5;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 800;
   weapons[ID].Criticals = 4;
   weapons[ID].CBills = 125000;

   ID = IS1_Autocannon_10;
   weapons[ID].Heat = 3;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].Criticals = 7;
   weapons[ID].CBills = 200000;
 
   ID = IS1_Autocannon_20;
   weapons[ID].Heat = 7;
   weapons[ID].MaxDamage = 20; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1400;
   weapons[ID].Criticals = 10;
   weapons[ID].CBills = 300000;

   ID = IS1_Autocannon_Ammo;
   weapons[ID].MaxRangeInMeters = 800; // The range where the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].ShotsPerTon = 45; // If positive, this record describes ammo
   weapons[ID].AvailableInYear = 2250; // Year of field/open market availability
   weapons[ID].CBills = 1000;
   //weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "Ammo(AC)", "Ammo(Autocannon)");
   weapons[ID].MW2_ID = 14500;
   weapons[ID].MaxCount = 400; // ID's used by MW2
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   for (int i = 0; i < 4; i++) {
      int wID = IS1_Autocannon_20+i;
      setWeaponAmmoPair (wID, IS1_Autocannon_Ammo); 
   }

   ID = IS2_Anti_Missile_System;
   weapons[ID].Heat = 1;
   weapons[ID].MaxRangeInMeters = 90; // The range where the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 3040; // Year of field/open market availability
   weapons[ID].CBills = 100000;
   weapons[ID].AmmoTypes[0] = IS1_Machine_Gun_Ammo; // Ammo that this weapon can use
   weapons[ID].flags = MWWeaponFlag_IncludeInWeaponList | MWWeaponFlag_CountedInMEK;
   setWeaponNames (ID, "AMS", "Anti-Missile System");
   weapons[ID].MW2_ID = 6200;
   setWeaponAmmoPair (IS2_Anti_Missile_System, IS1_Machine_Gun_Ammo);

   ID = IS2_Rotary_AC_2;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 800;
   weapons[ID].Criticals = 3;
   weapons[ID].AvailableInYear = 3062; // Year of field/open market availability
   weapons[ID].CBills = 175000;
   //weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "RAC/2", "Rotary Autocannon/2");
   setWeaponAmmoPair (ID, IS1_Autocannon_Ammo);

   ID = IS2_Rotary_AC_5;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1000;
   weapons[ID].Criticals = 6;
   weapons[ID].AvailableInYear = 3062; // Year of field/open market availability
   weapons[ID].CBills = 275000;
   //weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "RAC/5", "Rotary Autocannon/5");
   setWeaponAmmoPair (ID, IS1_Autocannon_Ammo);

   for (int i = 0; i < 4; i++) {
      ID = IS2_Ultra_Autocannon_20+i;
      weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo;
      shortName = "UAC";
      shortName += ACnums[i];
      longName = "Ultra Autocannon/";
      longName += ACnums[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      weapons[ID].Heat = 1;
      weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
      weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
      weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo; // Ammo that this weapon can use
      setWeaponAmmoPair (ID, IS1_Autocannon_Ammo);
   }

   ID = IS2_Ultra_Autocannon_2;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 750;
   weapons[ID].Criticals = 3;
   weapons[ID].CBills = 120000;
   //weapons[ID].MW2_ID = 4500; // Not in MW2

   ID = IS2_Ultra_Autocannon_5;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 600; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 900;
   weapons[ID].Criticals = 5;
   weapons[ID].CBills = 200000;
   weapons[ID].MW2_ID = 4900;

   ID = IS2_Ultra_Autocannon_10;
   weapons[ID].Heat = 4;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1300;
   weapons[ID].Criticals = 7;
   weapons[ID].CBills = 320000;

   ID = IS2_Ultra_Autocannon_20;
   weapons[ID].Heat = 8;
   weapons[ID].MaxDamage = 20; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 300; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1500;
   weapons[ID].Criticals = 10;
   weapons[ID].CBills = 480000;

   for (int i = 0; i < 4; i++) {
      ID = IS2_LB_X_Autocannon_20+i;
      //weapons[ID].AmmoTypes[0] = IS2_LB_X_Autocannon_Ammo;
      weapons[ID].AmmoTypes[0] = IS1_Autocannon_Ammo;
      shortName = "LB-X AC";
      shortName = ACnums[i];
      longName = "LB-X Autocannon/";
      longName += ACnums[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      weapons[ID].Heat = 1;
      weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
      setWeaponAmmoPair (ID, IS1_Autocannon_Ammo);
      //setWeaponAmmoPair (ID, IS2_LB_X_Autocannon_Cluster_Ammo);
   }

   ID = IS2_LB_X_Autocannon_2;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 810; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 600;
   weapons[ID].Criticals = 4;
   weapons[ID].CBills = 150000;

   ID = IS2_LB_X_Autocannon_5;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 630; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 800;
   weapons[ID].Criticals = 5;
   weapons[ID].CBills = 250000;

   ID = IS2_LB_X_Autocannon_10;
   weapons[ID].Heat = 2;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1100;
   weapons[ID].Criticals = 6;
   weapons[ID].CBills = 400000;
   weapons[ID].MW2_ID = 5000;

   ID = IS2_LB_X_Autocannon_20;
   weapons[ID].Heat = 6;
   weapons[ID].MaxDamage = 20; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 360; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1400;
   weapons[ID].Criticals = 11;
   weapons[ID].CBills = 600000;

   ID = IS2_Gauss;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 15; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 660; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1500;
   weapons[ID].Criticals = 7;
   weapons[ID].AvailableInYear = 3040; // Year of field/open market availability
   weapons[ID].CBills = 300000;
   //weapons[ID].AmmoTypes[0] = IS2_Gauss_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "Gauss");
   weapons[ID].MW2_ID = 4400;
   setWeaponAmmoPair (IS2_Gauss, IS2_Gauss_Ammo);

   ID = IS2_Gauss_Ammo;
   //weapons[ID].Heat = 0;
   weapons[ID].MaxDamage = 15; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 660; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 3040; // Year of field/open market availability
   weapons[ID].CBills = 20000;
   //weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
   setWeaponNames (ID, "A(Gauss)", "Ammo(Gauss)");
   weapons[ID].MW2_ID = 14400;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   const char* SRMnames[] = {"SRM 2", "SRM 4", "SRM 6"};
   for (int i = 0; i < 3; i++) {
      ID = IS1_SRM_2+i;
      weapons[ID].MW2_ID = 4100-100*i;
      //weapons[ID].AmmoTypes[0] = IS1_SRM_Ammo; // Ammo that this weapon can use
      weapons[ID].AvailableInYear = 2370; // Year of field/open market availability
      setWeaponNames(ID, SRMnames[i]);
      weapons[ID].TonnageTimes100 = 100*(i+1);
      weapons[ID].Heat = 2+i;
      setWeaponAmmoPair (ID, IS1_SRM_Ammo);
   }
   weapons[IS1_SRM_2].CBills = 10000;
   weapons[IS1_SRM_4].CBills = 60000;

   ID = IS1_SRM_6;
   weapons[ID].Criticals = 2;
   weapons[ID].CBills = 80000;

   for (ID = IS1_SRM_Ammo; ID <= IS1_SRM_6_Ammo; ++ID) {
      weapons[ID].MaxDamage = 2; // Maximum weapon damage
      weapons[ID].MinDamage = 2; // Minimum weapon damage at maximum range
      weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
      weapons[ID].TonnageTimes100 = 100;
      weapons[ID].ShotsPerTon = 50; // If positive, this record describes ammo
      weapons[ID].AvailableInYear = 2370; // Year of field/open market availability
      weapons[ID].CBills = 27000;
      //weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
      setWeaponNames (ID, "Ammo(SRM)");
      weapons[ID].MW2_ID = 13900;
      //weapons[ID].MaxCount = 300; // ID's used by MW2
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   setWeaponNames(IS1_SRM_2_Ammo, "A(SRM 2)");
   setWeaponAmmoPair(IS1_SRM_2, IS1_SRM_2_Ammo);

   setWeaponNames(IS1_SRM_4_Ammo, "A(SRM 4)");
   setWeaponAmmoPair(IS1_SRM_4, IS1_SRM_4_Ammo);

   setWeaponNames(IS1_SRM_6_Ammo, "A(SRM 6)");
   setWeaponAmmoPair(IS1_SRM_6, IS1_SRM_6_Ammo);

   const char* LRMnames[] = {"LRM 5", "LRM 10", "LRM 15", "LRM 20"};
   for (int i = 0; i < 4; i++) {
      ID = IS1_LRM_5+i;
      weapons[ID].AvailableInYear = 2300;
      //weapons[ID].AmmoTypes[0] = IS1_LRM_Ammo;
      weapons[ID].MW2_ID = 3800-i*100;
      setWeaponNames(ID, LRMnames[i]);
      setWeaponAmmoPair (ID, IS1_LRM_Ammo);
   }

   ID = IS1_LRM_5;
   weapons[ID].Heat = 6;
   weapons[ID].TonnageTimes100 = 200;
   weapons[ID].CBills = 30000;

   ID = IS1_LRM_10;
   weapons[ID].Heat = 4;
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].Criticals = 2;
   weapons[ID].CBills = 100000;
   
   ID = IS1_LRM_15;
   weapons[ID].Heat = 5;
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].Criticals = 3;
   weapons[ID].CBills = 175000;

   ID = IS1_LRM_20;
   weapons[ID].Heat = 6;
   weapons[ID].TonnageTimes100 = 1000;
   weapons[ID].Criticals = 5;
   weapons[ID].CBills = 250000;

   for (ID = IS1_LRM_Ammo; ID <= IS1_LRM_20_Ammo; ++ID) {
      weapons[ID].MinimumRangeInMeters = 180;
      weapons[ID].MaxRangeInMeters = 630; // The maximum range for the weapon does minimum damage
      weapons[ID].TonnageTimes100 = 100;
      weapons[ID].ShotsPerTon = 24; // If positive, this record describes ammo
      weapons[ID].AvailableInYear = 2300; // Year of field/open market availability
      weapons[ID].CBills = 30000;
      //weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
      setWeaponNames (ID, "A(LRM)", "Ammo(LRM)");
      weapons[ID].MW2_ID = 13500;
      weapons[ID].MaxCount = 400; // ID's used by MW2
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   setWeaponNames (IS1_LRM_5_Ammo, "A(LRM5)", "Ammo(LRM 5)");
   setWeaponAmmoPair(IS1_LRM_5_Ammo, IS1_LRM_5_Ammo);

   setWeaponNames (IS1_LRM_10_Ammo, "A(LRM10)", "Ammo(LRM 10)");
   setWeaponAmmoPair(IS1_LRM_10_Ammo, IS1_LRM_10_Ammo);

   setWeaponNames (IS1_LRM_15_Ammo, "A(LRM15)", "Ammo(LRM 15)");
   setWeaponAmmoPair(IS1_LRM_15_Ammo, IS1_LRM_15_Ammo);

   setWeaponNames (IS1_LRM_20_Ammo, "A(LRM20)", "Ammo(LRM 20)");
   setWeaponAmmoPair(IS1_LRM_20_Ammo, IS1_LRM_20_Ammo);

   ID = IS2_MRM_10;
   weapons[ID].Heat = 4;
   weapons[ID].TonnageTimes100 = 300;
   weapons[ID].Criticals = 2;
   weapons[ID].CBills = 50000;
   setWeaponNames (ID, "MRM 10", "Medium-Range Missiles 10");

   ID = IS2_MRM_20;
   weapons[ID].Heat = 6;
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].Criticals = 3;
   weapons[ID].CBills = 125000;
   setWeaponNames (ID, "MRM 20", "Medium-Range Missiles 20");

   ID = IS2_MRM_30;
   weapons[ID].Heat = 10;
   weapons[ID].TonnageTimes100 = 1000;
   weapons[ID].Criticals = 5;
   weapons[ID].CBills = 225000;
   setWeaponNames (ID, "MRM 30", "Medium-Range Missiles 30");

   ID = IS2_MRM_40;
   weapons[ID].Heat = 12;
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].Criticals = 7;
   weapons[ID].CBills = 350000;
   setWeaponNames (ID, "MRM 40", "Medium-Range Missiles 40");

   for (size_t i = 0; i < 4; ++i) {
      ID = IS2_MRM_10+i;
      weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
      weapons[ID].AvailableInYear = 3058; // Year of field/open market availability
      weapons[ID].AmmoTypes[0] = IS2_MRM_Ammo; // Ammo that this weapon can use
      setWeaponAmmoPair(ID, IS2_MRM_Ammo);
   }

   ID = IS2_MRM_Ammo;
   weapons[ID].MaxDamage = 1; // Maximum weapon damage
   weapons[ID].MinDamage = 1; // Minimum weapon damage at maximum range
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].ShotsPerTon = 24; // If positive, this record describes ammo
   weapons[ID].AvailableInYear = 3058; // Year of field/open market availability
   weapons[ID].CBills = 27000;
   weapons[ID].AmmoTypes[0] = 0; // Ammo that this weapon can use
   setWeaponNames (ID, "Ammo(MRM)");
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   for (int i = 0; i < 9; i++) {
      ID = i + IS2_SRT_2;
      int oldID = i+IS1_SRM_2;
      copyWeapon (oldID, ID);
      if (ID < IS2_SRT_Ammo) {
         weapons[ID].shortName[2] = 'T';
      } else {
         weapons[ID].shortName[7] = 'T';
      }
      weapons[ID].MW2_31st_ID = ID_UNUSED_MW2;
      weapons[ID].MW2_ID = ID_UNUSED_MW2;
   }

   for (int i = 0; i < 3; ++i) 
      setWeaponAmmoPair(IS2_SRT_2+i, IS2_SRT_Ammo);
   for (int i = 0; i < 4; ++i) 
      setWeaponAmmoPair(IS2_LRT_5+i, IS2_LRT_Ammo);

   for (int i = 0; i < 3; i++) {
      ID = IS2_Streak_SRM_2+i;
      weapons[ID].Heat = 2+i;
      weapons[ID].TonnageTimes100 = 150*(i+1);
      //weapons[ID].AmmoTypes[0] = IS2_Streak_SRM_Ammo;
      weapons[ID].AvailableInYear = 3058; // Year of field/open market availability
      longName = "Streak ";
      longName += SRMnames[i];
      shortName = 'S';
      shortName += SRMnames[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      //weapons[ID].flags = MWWeaponFlagsStdWeapon;
      setWeaponAmmoPair(ID, IS2_Streak_SRM_Ammo);
   }

   ID = IS2_Streak_SRM_2;
   weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
   weapons[ID].CBills = 15000;
   weapons[ID].MW2_ID = 4200;
   
   ID = IS2_Streak_SRM_4;
   weapons[ID].CBills = 90000;

   ID = IS2_Streak_SRM_6;
   weapons[ID].Criticals = 2;
   weapons[ID].CBills = 120000;
   
   ID = IS2_Streak_SRM_Ammo;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MinDamage = 2; // Minimum weapon damage at maximum range
   weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].ShotsPerTon = 50; // If positive, this record describes ammo
   weapons[ID].AvailableInYear = 2370; // Year of field/open market availability
   weapons[ID].CBills = 27000;
   setWeaponNames (ID, "A(SSRM)", "Ammo(Streak SRM)");
   weapons[ID].MW2_ID = 13900;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   ID = IS2_NARC;
   weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 200;
   weapons[ID].Criticals = 3;
   weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
   weapons[ID].CBills = 100000;
   setWeaponNames (ID, "NARC");
   weapons[ID].MW2_ID = 6100;
   weapons[ID].flags = MWWeaponFlagsStdWeapon;
   //weapons[ID].AmmoTypes[0] = IS2_NARC_AMMO;

   ID = IS2_NARC_AMMO;
   weapons[ID].MaxDamage = 0; // Maximum weapon damage
   weapons[ID].MinDamage = 0; // Minimum weapon damage at maximum range
   weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   //weapons[ID].Criticals = 1;
   weapons[ID].ShotsPerTon = 6; // If positive, this record describes ammo
   weapons[ID].AvailableInYear = 3035; // Year of field/open market availability
   weapons[ID].CBills = 6000;
   setWeaponNames (ID, "A(NARC)", "Ammo(NARC)");
   weapons[ID].MW2_ID = 16100;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   setWeaponAmmoPair(IS2_NARC, IS2_NARC_AMMO);

   ID = IS2_Arrow_IV;
   setWeaponNames (ID, "Arrow IV");
   weapons[ID].MW2_ID = 6500;
   //weapons[ID].AmmoTypes[0] = IS2_Arrow_IV_Ammo;

   ID = IS2_Arrow_IV_Ammo;
   setWeaponNames (ID, "A(Arr.IV)", "Ammo(Arrow IV)");
   weapons[ID].MW2_ID = 16500;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   setWeaponAmmoPair(IS2_Arrow_IV, IS2_Arrow_IV_Ammo);

   ID = IS2_SRM_Ammo_Inferno;
   setWeaponNames (ID, "A(SRM I)", "Ammo(Inferno SRM)");
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   for (size_t i = 0; i < 3; ++i) 
      setWeaponAmmoPair(IS1_SRM_2+i, IS2_SRM_Ammo_Inferno);

   ID = CE_Engine;
   setWeaponNames (ID, "Engine");
   weapons[ID].MW2_ID = 7850;
   weapons[ID].MW2_31st_ID = 5850;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Gyro;
   setWeaponNames (ID, "Gyro", "Gyroscope");
   weapons[ID].MW2_ID = 7800;
   weapons[ID].MW2_31st_ID = 5800;
   weapons[ID].MaxCount = 50;
   weapons[ID].Criticals = 4;
   weapons[ID].flags = 0;

   ID = CE_Life_Support;
   setWeaponNames (ID, "Life S.", "Life Support");
   weapons[ID].MW2_ID = 7900;
   weapons[ID].MW2_31st_ID = 5900;
   weapons[ID].MaxCount = 50;
   weapons[ID].Criticals = 2;
   weapons[ID].flags = 0;

   ID = CE_Cockpit;
   setWeaponNames (ID, "Cockpit");
   weapons[ID].MW2_ID = 7750;
   weapons[ID].MW2_31st_ID = 5750;
   weapons[ID].MaxCount = 50;
   //weapons[ID].Criticals = 1;
   weapons[ID].flags = 0;

   ID = CE_Sensors;
   setWeaponNames (ID, "Sensors");
   weapons[ID].MW2_ID = 7700;
   weapons[ID].MW2_31st_ID = 5700;
   weapons[ID].MaxCount = 50;
   weapons[ID].Criticals = 2;
   weapons[ID].flags = 0;

   ID = CE_Shoulder;
   setWeaponNames (ID, "Shoulder");
   weapons[ID].MW2_ID = 7300;
   weapons[ID].MW2_31st_ID = 5300;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = CE_Upper_Arm_Actuator;
   setWeaponNames (ID, "UAA", "Upper Arm Actuator");
   weapons[ID].MW2_ID = 7350;
   weapons[ID].MW2_31st_ID = 5350;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = CE_Lower_Arm_Actuator;
   setWeaponNames (ID, "LAA", "Lower Arm Actuator");
   weapons[ID].MW2_ID = 7400;
   weapons[ID].MW2_31st_ID = 5400;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Hand;
   setWeaponNames (ID, "Hand");
   weapons[ID].MW2_ID = 7450;
   weapons[ID].MW2_31st_ID = 5450;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Hip;
   setWeaponNames (ID, "Hip");
   weapons[ID].MW2_ID = 7500;
   weapons[ID].MW2_31st_ID = 5500;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Upper_Leg_Actuator;
   setWeaponNames (ID, "ULA", "Upper Leg Actuator");
   weapons[ID].MW2_ID = 7550;
   weapons[ID].MW2_31st_ID = 5550;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Lower_Leg_Actuator;
   setWeaponNames (ID, "LLA", "Lower Leg Actuator");
   weapons[ID].MW2_ID = 7600;
   weapons[ID].MW2_31st_ID = 5600;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Foot;
   setWeaponNames (ID, "Foot");
   weapons[ID].MW2_ID = 7650;
   weapons[ID].MW2_31st_ID = 5650;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = CE_Jump_Jet;
   setWeaponNames (ID, "Jump Jet");
   weapons[ID].MW2_ID = 8500;
   weapons[ID].MW2_31st_ID = 7000;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = 0;

   ID = IS_Heat_Sink;
   weapons[ID].Heat = -1;
   setWeaponNames (ID, "HS", "Heat Sink");
   weapons[ID].MW2_ID = 8000;
   weapons[ID].MW2_31st_ID = 5450;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = IS_Double_Heat_Sink;
   weapons[ID].Heat = -2;
   weapons[ID].Criticals = 3;
   weapons[ID].MW2_ID = 8000;
   setWeaponNames (ID, "DHS", "Double Heat Sink");
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = IS_CASE;
   setWeaponNames (ID, "CASE");
   weapons[ID].MW2_ID = 9550;
   weapons[ID].MaxCount = 50;
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].flags = 0;

   ID = IS_Ferro_Fibrous;
   setWeaponNames (ID, "FF", "Ferro-Fibrous Armor");
   weapons[ID].MW2_ID = 9500;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = IS_Endo_Steel;
   setWeaponNames (ID, "Endo", "Endo Steel Internals");
   weapons[ID].MW2_ID = 9000;
   weapons[ID].MaxCount = 50;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = IS_Beagle;
   setWeaponNames (ID, "Beagle");

   ID = IS_MASC;
   setWeaponNames (ID, "MASC");
   weapons[ID].flags = 0;

   ID = IS_Targeting_Computer;
   setWeaponNames (ID, "TaCo", "Targeting Computer");
   weapons[ID].flags = 0;

   ID = IS_TAG;
   setWeaponNames (ID, "TAG");

   ID = IS_Hatchet;
   setWeaponNames (ID, "Hatchet");

   ID = IS_Sword;
   setWeaponNames (ID, "Sword");

   ID = Clan_ER_Large_Laser;
   weapons[ID].Heat = 12;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 750; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 400;
   weapons[ID].AvailableInYear = 2620; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 200000;
   setWeaponNames (ID, "ER LLaser", "ER Large Laser");
   weapons[ID].MW2_31st_ID = 2200;
   weapons[ID].MW2_ID = 2200;

   ID = Clan_ER_Medium_Laser;
   weapons[ID].Heat = 5;
   weapons[ID].MaxDamage = 6; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].CBills = clanFactor * 80000;
   setWeaponNames (ID, "ER MLaser", "ER Medium Laser");
   weapons[ID].MW2_31st_ID = 2300;
   weapons[ID].MW2_ID = 2300;

   ID = Clan_ER_Small_Laser;
   weapons[ID].Heat = 2;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 180; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].CBills = clanFactor * 11250;
   setWeaponNames (ID, "ER SLaser", "ER Small Laser");
   weapons[ID].MW2_31st_ID = 2400;
   weapons[ID].MW2_ID = 2400;
   weapons[ID].MaxCount = 100;

   ID = Clan_ER_Micro_Laser;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 120; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 25;
   weapons[ID].AvailableInYear = 3060; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 10000;
   setWeaponNames (ID, "ER MicLas", "ER Micro Laser");

   ID = Clan_Large_Heavy_Laser;
   weapons[ID].Heat = 18;
   weapons[ID].MaxDamage = 16; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 400;
   weapons[ID].Criticals = 3;
   weapons[ID].AvailableInYear = 3059; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 250000;
   setWeaponNames (ID, "LHLaser", "Large Heavy Laser");

   ID = Clan_Medium_Heavy_Laser;
   weapons[ID].Heat = 7;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 570; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 3059; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 100000;
   setWeaponNames (ID, "MHLaser", "Medium Heavy Laser");

   ID = Clan_Small_Heavy_Laser;
   weapons[ID].Heat = 3;
   weapons[ID].MaxDamage = 6; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 3059; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 20000;
   setWeaponNames (ID, "SHLaser", "Small Heavy Laser");

   ID = Clan_Flamer;
   weapons[ID].Heat = 3;
   weapons[ID].MaxDamage = 2; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 2025; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 7500;
   setWeaponNames (ID, "Flamer");
   weapons[ID].MW2_ID = 2000; // See Linebacker C, LBK03STD.MEK
   weapons[ID].MW2_31st_ID = 2000;

   ID = Clan_ER_PPC;
   weapons[ID].Heat = 15;
   weapons[ID].MaxDamage = 15; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 690; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 600;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 2760; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 300000;
   setWeaponNames (ID, "ER PPC");
   weapons[ID].MW2_31st_ID = 2100;
   weapons[ID].MW2_ID = 2100;

   ID = Clan_Large_Pulse_Laser;
   weapons[ID].Heat = 10;
   weapons[ID].MaxDamage = 10; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 600; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 600;
   weapons[ID].Criticals = 2;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 175000;
   setWeaponNames (ID, "LPLaser", "Large Pulse Laser");
   weapons[ID].MW2_31st_ID = 2500;
   weapons[ID].MW2_ID = 2500;

   ID = Clan_Medium_Pulse_Laser;
   weapons[ID].Heat = 5;
   weapons[ID].MaxDamage = 6; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 450; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 200;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 60000;
   setWeaponNames (ID, "MPLaser", "Medium Pulse Laser");
   weapons[ID].MW2_31st_ID = 2600;
   weapons[ID].MW2_ID = 2600;

   ID = Clan_Small_Pulse_Laser;
   weapons[ID].Heat = 2;
   weapons[ID].MaxDamage = 5; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 180; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 16000;
   setWeaponNames (ID, "SPLaser", "Small Pulse Laser");
   weapons[ID].MW2_31st_ID = 2700;
   weapons[ID].MW2_ID = 2700;

   ID = Clan_Micro_Pulse_Laser;
   weapons[ID].Heat = 1;
   weapons[ID].MaxDamage = 3; // Maximum weapon damage
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 2609; // Year of field/open market availability
   weapons[ID].CBills = clanFactor * 12500;
   setWeaponNames (ID, "miPLaser", "Micro Pulse Laser");

   for (int ID = Clan_Ballistic_Weapons; ID < Clan_Missile_Weapons; ID++) {
         weapons[ID].MaxRangeInMeters = 270; // The maximum range for the weapon does minimum damage
   }

   ID = Clan_Anti_Missile_System;
   weapons[ID].Heat = 1;
   weapons[ID].MaxRangeInMeters = 90; // The maximum range where the weapon does damage
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].AvailableInYear = 2617; // Year of field/open market availability
   weapons[ID].CBills = 100000;
   //weapons[ID].AmmoTypes[0] = Clan_Machine_Gun_Ammo; // Ammo that this weapon can use
   //weapons[ID].flags = MWWeaponFlagsStdWeapon;
   setWeaponNames (ID, "AMS", "Anti-Missile System");
   weapons[ID].MW2_ID = 2900;
   weapons[ID].MW2_31st_ID = 2900;
   //weapons[ID].MaxCount = 100;

   ID = Clan_Anti_Missile_System_Ammo;
   setWeaponNames(ID, "A(AMS)", "Ammo(Anti-Missile System)");
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].MW2_ID = 12900;
   weapons[ID].MW2_31st_ID = 12900;
   setWeaponAmmoPair(Clan_Anti_Missile_System, Clan_Anti_Missile_System_Ammo);
   weapons[ID].ShotsPerTon = 24;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   ID = Clan_Gauss;
   weapons[ID].Heat = 1;
   weapons[ID].MaxRangeInMeters = 660; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].Criticals = 6;
   weapons[ID].AvailableInYear = 2590; // Year of field/open market availability
   weapons[ID].CBills = 300000;
   //weapons[ID].AmmoTypes[0] = Clan_Gauss_Ammo; // Ammo that this weapon can use
   setWeaponNames (ID, "Gauss", "Gauss Rifle");
   weapons[ID].MW2_31st_ID = 1100;
   weapons[ID].MW2_ID = 1100;

   ID = Clan_Gauss_Ammo;
   weapons[ID].MaxDamage = 15;
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].AvailableInYear = 2590;
   weapons[ID].CBills = 20000;
   setWeaponNames (ID, "A(Gauss)", "Ammo(Gauss Rifle)");
   weapons[ID].MW2_31st_ID = 11100;
   weapons[ID].MW2_ID = 11100;
   //weapons[ID].MaxCount = 100;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   setWeaponAmmoPair(Clan_Gauss, Clan_Gauss_Ammo);

   // Clan LB-X
   for (int i = 0; i < 4; i++) {
      ID = Clan_LB_20_X_AC+i;
      weapons[ID].Heat = 1;
      weapons[ID].Criticals = 6-i;
      setWeaponAmmoPair(ID, Clan_LB_X_Ammo);
      setWeaponAmmoPair(ID, Clan_Autocannon_Ammo);
      shortName = "LB ";
      shortName += ACnums[i];
      shortName += "X AC";
      longName = "LB ";
      longName += ACnums[i];
      longName += "-X Autocannon";
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      int MW2_ID = 1500-100*i;
      weapons[ID].MW2_31st_ID = MW2_ID;
      weapons[ID].MW2_ID = MW2_ID;
      weapons[ID].AvailableInYear = 2595; // Year of field/open market availability
      weapons[ID].Heat = 1;
      weapons[ID].Criticals = 6-i;
   }

   ID = Clan_LB_2_X_AC;
   weapons[ID].MaxRangeInMeters = 900; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].CBills = 150000;

   ID = Clan_LB_5_X_AC;
   weapons[ID].MaxRangeInMeters = 720; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].CBills = 250000;

   ID = Clan_LB_10_X_AC;
   weapons[ID].Heat = 2;
   weapons[ID].MaxRangeInMeters = 540; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1000;
   weapons[ID].CBills = 400000;

   ID = Clan_LB_20_X_AC;
   weapons[ID].Heat = 6;
   weapons[ID].MaxRangeInMeters = 360; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].Criticals = 9;
   weapons[ID].CBills = 600000;

   for (int i = 0; i < 4; i++) {
      ID = Clan_UAC_20+i;
      weapons[ID].Heat = 1;
      weapons[ID].MaxRangeInMeters = 810; // The maximum range for the weapon does minimum damage
      //weapons[ID].AmmoTypes[0] = Clan_Autocannon_Ammo;
      weapons[ID].MW2_31st_ID = 1900-100*i;
      weapons[ID].MW2_ID = weapons[ID].MW2_31st_ID;
      shortName = "UAC/";
      shortName += ACnums[i];
      longName = "Ultra Autocannon/";
      longName += ACnums[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      setWeaponAmmoPair(ID, Clan_Autocannon_Ammo);
   }

   ID = Clan_UAC_2;
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].CBills = 120000;
   weapons[ID].Criticals = 2;

   ID = Clan_UAC_5;
   weapons[ID].TonnageTimes100 = 700;
   weapons[ID].CBills = 200000;
   weapons[ID].Criticals = 3;

   ID = Clan_UAC_10;
   weapons[ID].Heat = 3;
   weapons[ID].TonnageTimes100 = 1000;
   weapons[ID].CBills = 320000;
   weapons[ID].Criticals = 4;

   ID = Clan_UAC_20;
   weapons[ID].Heat = 7;
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].CBills = 480000;
   weapons[ID].Criticals = 8;

   setWeaponNames (Clan_Light_Machine_Gun, "LMG", "Light Machine Gun");
   setWeaponNames (Clan_Heavy_Machine_Gun, "MG", "Heavy Machine Gun");

   ID = Clan_Machine_Gun;
   weapons[ID].MaxRangeInMeters = 90; // The maximum range for the weapon does minimum damage
   weapons[ID].TonnageTimes100 = 25;
   weapons[ID].CBills = 5000;
   setWeaponNames (ID, "MG", "Machine Gun");
   weapons[ID].MW2_31st_ID = 1000;
   weapons[ID].MW2_ID = 1000;
   setWeaponAmmoPair(Clan_Machine_Gun, Clan_Machine_Gun_Ammo);

   for (ID = Clan_Autocannon_Ammo; ID <= Clan_UAC_2_Ammo; ++ID) {
      weapons[ID].MaxDamage = 1;
      weapons[ID].TonnageTimes100 = 100;
      //weapons[ID].Criticals = 1;
      weapons[ID].CBills = 20000;
      weapons[ID].MW2_31st_ID = 11700;
      weapons[ID].MW2_ID = 11700;
      //weapons[ID].MaxCount = 100; // Used with many weapon codes
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   // MW2 needs ammo types for each weapon type ...
   for (int i = 0; i < 4; ++i) {
      setWeaponAmmoPair(Clan_UAC_20+i, Clan_UAC_20_Ammo+i);
      weapons[Clan_UAC_20_Ammo+i].MW2_31st_ID = 11900-100*i;
   }

   setWeaponNames(Clan_Autocannon_Ammo, "Ammo(AC)", "Ammo(Autocannon)");
   setWeaponNames(Clan_UAC_2_Ammo, "A(UAC2)", "Ammo(Ultra Autocannon 2)");
   setWeaponNames(Clan_UAC_5_Ammo, "A(UAC5)", "Ammo(Ultra Autocannon 5)");
   setWeaponNames(Clan_UAC_10_Ammo, "A(UAC10)", "Ammo(Ultra Autocannon 10)");
   setWeaponNames(Clan_UAC_20_Ammo, "A(UAC20)", "Ammo(Ultra Autocannon 20)");

   for (ID = Clan_LB_X_Ammo; ID <= Clan_LB_2_X_AC_Ammo; ++ID) {
      weapons[ID].MaxDamage = 1;
      weapons[ID].TonnageTimes100 = 100; // 
      //weapons[ID].Criticals = 1;
      weapons[ID].CBills = 20000;
      //weapons[ID].MW2_31st_ID = 11300;
      weapons[ID].MW2_ID = 11300;
      //weapons[ID].MaxCount = 100; // Used with many weapon codes
      weapons[ID].flags = MWWeaponFlag_Ammo | MWWeaponFlag_CountedInMEK;
   }

   // MW2 needs ammo types for each weapon type ...
   for (int i = 0; i < 4; ++i) {
      setWeaponAmmoPair(Clan_LB_20_X_AC+i, Clan_LB_20_X_AC_Ammo+i);
      weapons[Clan_LB_20_X_AC_Ammo+i].MW2_31st_ID = 11500-100*i;
   }

   setWeaponNames(Clan_LB_X_Ammo, "A(LBX AC)", "Ammo(LB-X Autocannon)");
   setWeaponNames(Clan_LB_20_X_AC_Ammo, "A(LB 20X)", "Ammo(LB 20-X Autocannon)");
   setWeaponNames(Clan_LB_10_X_AC_Ammo, "A(LB 10X)", "Ammo(LB 10-X Autocannon)");
   setWeaponNames(Clan_LB_5_X_AC_Ammo, "A(LB 5X)", "Ammo(LB 5-X Autocannon)");
   setWeaponNames(Clan_LB_2_X_AC_Ammo, "A(LB 2X)", "Ammo(LB 2-X Autocannon)");

   ID = Clan_Machine_Gun_Ammo;
   weapons[ID].MaxDamage = 1;
   weapons[ID].TonnageTimes100 = 100; // 
   //weapons[ID].Criticals = 1;
   weapons[ID].CBills = 1000;
   setWeaponNames (ID, "A(MG)", "Ammo (Machine Gun)");
   weapons[ID].MW2_31st_ID = 11000; // Not confirmed?
   weapons[ID].MW2_ID = 11000;
   //weapons[ID].MaxCount = 100; // Used with many weapon codes
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   // Clan ATMs follow simple formulas
   for (int i = 0; i < 4; i++) {
      ID = i+Clan_ATM_3;
      weapons[ID].Heat = 2*(i+1);
      weapons[ID].Criticals = i+1;
      BGString name = "ATM ";
      name.appendInt (3*(i+1));
      setWeaponNames (ID, name.getChars());
      setWeaponAmmoPair(ID, Clan_ATM_Std_Ammo);
      setWeaponAmmoPair(ID, Clan_ATM_ER_Ammo);
      setWeaponAmmoPair(ID, Clan_ATM_HE_Ammo);
   }
   weapons[Clan_ATM_3].TonnageTimes100 = 150;
   weapons[Clan_ATM_6].TonnageTimes100 = 350;
   weapons[Clan_ATM_9].TonnageTimes100 = 500;
   weapons[Clan_ATM_12].TonnageTimes100 = 700;

   setWeaponNames(Clan_ATM_Std_Ammo, "A(ATM)", "Ammo(ATM Standard)");
   setWeaponNames(Clan_ATM_ER_Ammo, "A(ATM ER)", "Ammo(ATM Extended Range)");
   setWeaponNames(Clan_ATM_HE_Ammo, "A(ATM HE)", "Ammo(ATM High-Explosive)");

   // Clan LRM
   for (int i = 0; i < 4; i++) {
      ID = Clan_LRM_5+i;
      //weapons[ID].AmmoTypes[0] = Clan_LRM_Ammo;
      //weapons[ID].Criticals = 1;
      weapons[ID].MW2_ID = 100*(3-i);
      weapons[ID].MW2_31st_ID = weapons[ID].MW2_ID;
      weapons[ID].CBills = weapons[IS1_LRM_5+i].CBills;
      setWeaponNames (ID, LRMnames[i]);
      setWeaponAmmoPair(ID, Clan_LRM_Ammo);
      setWeaponAmmoPair(ID, Clan_LRM_5_Ammo+i);
   }

   ID = Clan_LRM_5;
   weapons[ID].Heat = 2;
   weapons[ID].TonnageTimes100 = 100;

   ID = Clan_LRM_10;
   weapons[ID].Heat = 4;
   weapons[ID].TonnageTimes100 = 250;

   ID = Clan_LRM_15;
   weapons[ID].Heat = 5;
   weapons[ID].TonnageTimes100 = 350;
   weapons[ID].Criticals = 2;

   ID = Clan_LRM_20;
   weapons[ID].Heat = 6;
   weapons[ID].TonnageTimes100 = 500;
   weapons[ID].Criticals = 4;

   ID = Clan_NARC_Missile_Beacon;
   //weapons[ID].Heat = 0;
   weapons[ID].TonnageTimes100 = 200;
   //weapons[ID].Criticals = 1;
   setWeaponNames (ID, "NARC");
   weapons[ID].MW2_ID = 2800;
   weapons[ID].MW2_31st_ID = 2800;

   ID = Clan_NARC_Missile_Beacon_Ammo;
   setWeaponNames (ID, "A(NARC)", "Ammo(NARC)");
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   weapons[ID].MW2_31st_ID = 12800;
   weapons[ID].MW2_ID = 12800;
   setWeaponAmmoPair(Clan_NARC_Missile_Beacon, Clan_NARC_Missile_Beacon_Ammo);

   // Clan SRMs follow simple formulas
   for (int i = 0; i < 3; i++) {
      ID = i+Clan_SRM_2;
      weapons[ID].Heat = i+2;
      //weapons[ID].Criticals = 1;
      weapons[ID].TonnageTimes100 = (i+1)*50;
      weapons[ID].CBills = weapons[IS1_SRM_2+i].CBills;
      //weapons[ID].AmmoTypes[0] = Clan_SRM_Ammo;
      //weapons[ID].AmmoTypes[1] = Clan_SRM_Ammo_Inferno;
      setWeaponNames (ID, SRMnames[i]);
      weapons[ID].MW2_31st_ID = (6-i)*100;
      weapons[ID].MW2_ID = (6-i)*100;
      setWeaponAmmoPair(ID, Clan_SRM_Ammo);
      setWeaponAmmoPair(ID, Clan_SRM_Ammo_Inferno);
      setWeaponAmmoPair(ID, Clan_SRM_2_Ammo+i);
   }

   // SRM2 Inferno is a special weapon in GBL, MW2:Mercs
   // Outside of MW2, use an SRM2 launcher with Inferno ammunition
   ID = Clan_SRM_2_Inferno;
   weapons[ID].Heat = 2;
   weapons[ID].TonnageTimes100 = 50;
   weapons[ID].CBills = weapons[IS1_SRM_2].CBills;
   setWeaponNames (ID, "SRM2Inf", "SRM2 Inferno");
   weapons[ID].MW2_31st_ID = 3100;
   weapons[ID].MW2_ID = 3100;
   setWeaponAmmoPair(ID, Clan_SRM_Ammo_Inferno);

   // Clan streak SRMs follow simple formulas
   for (int i = 0; i < 3; i++) {
      ID = i+Clan_Streak_SRM_2;
      weapons[ID].Heat = i+2;
      //weapons[ID].Criticals = 1;
      weapons[ID].TonnageTimes100 = (i+1)*100;
      weapons[ID].CBills = weapons[IS2_Streak_SRM_2+i].CBills;
      weapons[ID].AmmoTypes[0] = Clan_SSRM_Ammo;
      weapons[ID].AmmoTypes[1] = Clan_SRM_Ammo_Inferno;
      shortName = 'S';
      shortName += SRMnames[i];
      longName = "Streak ";
      longName += SRMnames[i];
      setWeaponNames(ID, shortName.getChars(), longName.getChars());
      weapons[ID].MW2_31st_ID = (9-i)*100;
      weapons[ID].MW2_ID = (9-i)*100;
      setWeaponAmmoPair(ID, Clan_SSRM_Ammo);
      //setWeaponAmmoPair(ID, Clan_SSRM_Ammo_Inferno);
      setWeaponAmmoPair(ID, Clan_Streak_SRM_2_Ammo+i);
   }
   weapons[Clan_Streak_SRM_6].Criticals = 2;

   setWeaponNames (Clan_SRM_Ammo,  "A(SRM)");
   setWeaponNames (Clan_SRM_2_Ammo, "A(SRM 2)");
   setWeaponNames (Clan_SRM_4_Ammo, "A(SRM 4)");
   setWeaponNames (Clan_SRM_6_Ammo, "A(SRM 6)");

   for (ID = Clan_SRM_Ammo; ID < Clan_LRM_Ammo; ++ID) {
      weapons[ID].TonnageTimes100 = 100;
      weapons[ID].MaxDamage = 2;
      weapons[ID].MaxDamageRange = 270;
      weapons[ID].ShotsPerTon = 100;
      //weapons[ID].MaxCount = 100;
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   for (size_t i = 0; i < 3; ++i) {
      weapons[Clan_SRM_2_Ammo+i].MW2_31st_ID = 10600-100*i;
   }

   for (ID = Clan_Streak_SRM_2_Ammo; ID <= Clan_Streak_SRM_6_Ammo; ++ID) {
      weapons[ID].TonnageTimes100 = 100;
      weapons[ID].MaxDamage = 2;
      weapons[ID].MaxDamageRange = 360;
      weapons[ID].ShotsPerTon = 100;
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   setWeaponNames(Clan_SSRM_Ammo, "A(SSRM)", "Ammo(Streak SRM)");
   setWeaponNames(Clan_Streak_SRM_2_Ammo, "A(SSRM2)", "Ammo(Streak SRM 2)");
   setWeaponNames(Clan_Streak_SRM_4_Ammo, "A(SSRM4)", "Ammo(Streak SRM 4)");
   setWeaponNames(Clan_Streak_SRM_6_Ammo, "A(SSRM6)", "Ammo(Streak SRM 6)");

   for (size_t i = 0; i < 3; ++i) {
      weapons[Clan_Streak_SRM_2_Ammo+i].MW2_31st_ID = 10900-100*i;
   }

   // Streak SRM2 Inferno is a special weapon in GBL, MW2:Mercs
   // Outside of MW2, use a Streak SRM2 launcher with Inferno ammunition
   ID = Clan_Streak_SRM_2_Inferno;
   weapons[ID].Heat = 2;
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].CBills = weapons[IS2_Streak_SRM_2].CBills;
   setWeaponNames (ID, "SSRM2Inf", "Streak SRM2 Inferno");
   weapons[ID].MW2_31st_ID = 3300;
   weapons[ID].MW2_ID = 3300;
   setWeaponAmmoPair(ID, Clan_SSRM_Inferno_Ammo);

   ID = Clan_SSRM_Inferno_Ammo;
   setWeaponNames(ID, "A(SSRM2I)", "Ammo(Streak SRM Inferno)");
   weapons[ID].TonnageTimes100 = 100;
   weapons[ID].CBills = Clan_SSRM_Inferno_Ammo;
   weapons[ID].ShotsPerTon = 50;
   weapons[ID].MW2_31st_ID = 13300;
   weapons[ID].MW2_ID = 13300;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   setWeaponNames(Clan_LRM_Ammo, "Ammo(LRM)");
   setWeaponNames(Clan_LRM_5_Ammo, "A(LRM 5)");
   setWeaponNames(Clan_LRM_10_Ammo, "A(LRM 10)");
   setWeaponNames(Clan_LRM_15_Ammo, "A(LRM 15)");
   setWeaponNames(Clan_LRM_20_Ammo, "A(LRM 20)");

   for (ID = Clan_LRM_Ammo; ID <= Clan_LRM_20_Ammo; ++ID) {
      weapons[ID].TonnageTimes100 = 100;
      weapons[ID].MaxDamage = 1;
      weapons[ID].MaxDamageRange = 630;
      weapons[ID].ShotsPerTon = 120;
      //weapons[ID].MaxCount = 100;
      weapons[ID].flags = MWWeaponFlagsStdAmmo;
   }

   for (size_t i = 0; i < 4; ++i) {
      int ID = Clan_LRM_5_Ammo+i;
      int mw2ID = 10300-100*i;
      weapons[ID].MW2_31st_ID = mw2ID;
      weapons[ID].MW2_ID = mw2ID;
   }

   ID = Clan_SRM_Ammo_Inferno;
   weapons[ID].TonnageTimes100 = 100;
   //weapons[ID].Criticals = 1;
   weapons[ID].ShotsPerTon = 5;
   weapons[ID].CBills = 10000;
   weapons[ID].ShotsPerTon = 100;
   setWeaponNames (ID, "A(SRM)I", "Ammo (SRM) Inferno");
   weapons[ID].MW2_ID = 13100;
   weapons[ID].MW2_31st_ID = 13100;
   //weapons[ID].MaxCount = 100;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;

   ID = Clan_Arrow_IV;
   weapons[ID].TonnageTimes100 = 1200;
   weapons[ID].Criticals = 12;
   weapons[ID].CBills = 450000;
   weapons[ID].MaxRangeInMeters = 6000;
   weapons[ID].AmmoTypes[0] = Clan_Arrow_IV_Ammo;
   setWeaponNames (ID, "Arrow IV");
   weapons[ID].MW2_ID = 3400;
   weapons[ID].MW2_31st_ID = 3400;
   //weapons[ID].MaxCount = 100;

   //MWWeapons* w = weapons[Clan_Arrow_IV_Ammo];
   ID = Clan_Arrow_IV_Ammo;
   weapons[ID].TonnageTimes100 = 100;
   //weapons[ID].Criticals = 1;
   weapons[ID].ShotsPerTon = 5;
   weapons[ID].CBills = 10000;
   weapons[ID].MaxRangeInMeters = 6000;
   setWeaponNames (ID, "A(Arr.IV)", "Ammo (Arrow IV)");
   weapons[ID].MW2_ID = 13400;
   weapons[ID].MW2_31st_ID = 13400;
   //weapons[ID].MaxCount = 100;
   weapons[ID].flags = MWWeaponFlagsStdAmmo;
   setWeaponAmmoPair(Clan_Arrow_IV, Clan_Arrow_IV_Ammo);

   ID = Clan_Nuke;
   weapons[ID].MaxRangeInMeters = 6000;
   setWeaponNames (ID, "Nuke");
   weapons[ID].MW2_ID = 3000;
   //weapons[ID].MaxCount = 100;

   // Copy torpedos from SRM, LRM
   for (int i = 0; i < 9; i++) {
      ID = i + Clan_SRT_2;
      int oldID = i+Clan_SRM_2;
      copyWeapon (oldID, ID);
      if (ID < Clan_LRT_5) 
         setWeaponAmmoPair(ID, Clan_SRT_Ammo);
      else if (ID < Clan_SRT_Ammo)
         setWeaponAmmoPair(ID, Clan_LRT_Ammo);
      weapons[ID].MW2_31st_ID = ID_UNUSED_MW2;
      weapons[ID].MW2_ID = ID_UNUSED_MW2;
   }

   setWeaponNames (Clan_SRT_2, "SRT2");
   setWeaponNames (Clan_SRT_4, "SRT4");
   setWeaponNames (Clan_SRT_6, "SRT6");
   setWeaponNames (Clan_LRT_5, "LRT5");
   setWeaponNames (Clan_LRT_10, "LRT10");
   setWeaponNames (Clan_LRT_15, "LRT15");
   setWeaponNames (Clan_LRT_20, "LRT20");
   setWeaponNames (Clan_SRT_Ammo, "A(SRT)", "Ammo(SRT)");
   setWeaponNames (Clan_LRT_Ammo, "A(LRT)", "Ammo(LRT)");

   weapons[Clan_SRT_4].MW2_ID = 3200;
   weapons[Clan_SRT_4].MW2_31st_ID = 3200;
   //weapons[Clan_SRT_4].MaxCount = 100;
   weapons[Clan_SRT_Ammo].MW2_ID = 13200;
   weapons[Clan_SRT_Ammo].MW2_31st_ID = 13200;
   //weapons[Clan_SRT_Ammo].MaxCount = 100;
   weapons[Clan_SRT_Ammo].flags = MWWeaponFlagsStdAmmo;
   weapons[Clan_LRT_Ammo].flags = MWWeaponFlagsStdAmmo;

   ID = Clan_Double_Heat_Sink;
   weapons[ID].Heat = -2;
   weapons[ID].TonnageTimes100 = 0; // Should not be included in weight calc
   weapons[ID].Criticals = 2;
   setWeaponNames (ID, "DHS", "Double Heat Sink");
   weapons[ID].MW2_31st_ID = 6000;
   weapons[ID].MW2_ID = 8000;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = Clan_Targeting_Computer;
   //weapons[ID].Heat = 0;
   weapons[ID].TonnageTimes100 = 0; // Depends on number of connected weapons
   //weapons[ID].Criticals = 1; // Depends on number of connected weapons
   setWeaponNames (ID, "TaCo", "Targeting Computer");
   weapons[ID].flags = 0;

   ID = Clan_Ferro_Fibrous;
   //weapons[ID].Criticals = 1; // Actually 7, but 1 is better to handle in mech lab
   setWeaponNames (ID, "FF", "Ferro Fibrous Armor");
   weapons[ID].MW2_31st_ID = 9000;
   weapons[ID].MW2_ID = 9500;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = Clan_Endo_Steel_Internals;
   //weapons[ID].Criticals = 1; // Actually 7, but 1 is better to handle in mech lab
   setWeaponNames (ID, "Endo", "Endo Steel Internals");
   weapons[ID].MW2_31st_ID = 8000;
   weapons[ID].MW2_ID = 9000;
   weapons[ID].flags = MWWeaponFlag_CountedInMEK;

   ID = Clan_MASC;
   //weapons[ID].Criticals = 1; // Depends on number of connected weapons
   //weapons[ID].TonnageTimes100 = 1; // Use BipedMech::calcMASCTonsAndSlots instead
   setWeaponNames (ID, "MASC");
   weapons[ID].MW2_31st_ID = 5000;
   weapons[ID].MW2_ID = 7000;
   weapons[ID].flags = 0;

   /*for (size_t ID = 1; ID < WeaponsCount; ID++) {
      if (weapons[ID].shortName.getLength() == 0) {
         printf ("%i: %s lacks a short name\n", ID, weapons[ID].longName.getChars());
      }
      if (weapons[ID].longName.getLength() == 0 )
         weapons[ID].longName = weapons[ID].shortName;
   }*/

   // Store weapons table
   /*BGString str;

   str += "MW2 weapon IDs since Mercs\n";
   for (size_t ID = 0; ID < WeaponsCount; ID++) {
      if ((int) (weapons[ID].MW2_ID) != (WORD) ID_UNUSED_MW2) {
         str.appendInt(weapons[ID].MW2_ID);
         str += " ";
         str += weapons[ID].longName;
         if (ID >= Clan_Energy_Weapon_Base)
            str += " (Clan)";
         str += "\n";
      }
   }

   str += "\nMW2 weapon IDs before Mercs\n";
   for (size_t ID = 0; ID < WeaponsCount; ID++) {
      if ((int) (weapons[ID].MW2_31st_ID) != (WORD) ID_UNUSED_MW2) {
         str.appendInt(weapons[ID].MW2_31st_ID);
         str += " ";
         str += weapons[ID].longName;
         //if (ID < Clan_Energy_Weapon_Base)
         //   str += " (Clan)";
         str += "\n";
      }
   }

   FileCache fc;
   fc.create("d:\\temp\\mw2-weapons.txt");
   fc.append(str.getChars(), str.getLength());
   fc.close();*/
}

////////////////////////////////////////////////////////////////////////////////
// Get statistics for a specific weapon

const MWWeapons* getWeaponStats (size_t index)
{
   if (index > 0 && index < WeaponsCount)
      return &weapons[index];
   else
      return NULL;
}

double engineWeight[] = {0.0, 0.5, 0.5, 0.5, 0.5, // 0-20
                         0.5, 1.0, 1.0, 1.0, 1.0, // 25-45 
                         1.5, 1.5, 1.5, 2.0, 2.0, // 50-70
                         2.0, 2.5, 2.5, 3.0, 3.0, // 75-95
                         3.0, 3.5, 3.5, 4.0, 4.0, // 100-120
                         4.0, 4.5, 4.5, 5.0, 5.0, // 125-145
                         5.5, 5.5, 6.0, 6.0, 6.0, // 150-170
                         7.0, 7.0, 7.5, 7.5, 8.0, // 175-195
                         8.5, 8.5, 9.0, 9.5, 10.0, // 200-220
                         10.0, 10.5, 11.0, 11.5, 12.0, // 225-245
                         12.5, 13.0, 13.5, 14.0, 14.5, // 250-270
                         15.5, 16.0, 16.5, 17.5, 18.0, // 275-295
                         19.0, 19.5, 20.5, 21.5, 22.5, // 300-320
                         23.5, 24.5, 25.5, 27.0, 28.5, // 325-345
                         29.5, 31.5, 33.0, 34.5, 36.5, // 350-370
                         38.5, 41.0, 43.5, 46.0, 49.0, // 375-395
                         52.5, 56.5, 61.0, 66.5, 72.5, // 400-420
                         79.5, 87.5, 97.0, 107.5, 119.5, // 425-445
                         133.5, 150, 168.5, 190.0, 214.5, // 450-470
                         243.0, 275.5, 313.0, 356, 405.5, // 475-495
                         462.5}; // 500

double getEngineWeight (unsigned int engineRating, int EngineType)
{
   if (engineRating <= 500) {
      if (EngineType == ENGINE_TYPE_STD)
         return engineWeight[engineRating/5];
      else
         return engineWeight[engineRating/5] / 2;
   } else
      return 500;
}

////////////////////////////////////////////////////////////////////////////////

void getDosBoxPath (BGString& DosBoxPath)
{
   DosBoxPath = getExecDir();
   DosBoxPath += OSPathSep;
   DosBoxPath += "DosBox-0.74";
}

