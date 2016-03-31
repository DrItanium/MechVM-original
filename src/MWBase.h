////////////////////////////////////////////////////////////////////////////////
// MWBase.h
// Copyright Bjoern Ganster 2007-2009
////////////////////////////////////////////////////////////////////////////////

#ifndef MWBase__H
#define MWBase__H

#include "BGString.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////
// File types and names

static const int FT_Unsupported = 0;
static const int FT_XEL = 1;
static const int FT_WTB = 2;
static const int FT_Mech = 3;
static const int FT_PRJ = 4;
static const int FT_SHP = 5;
static const int FT_SFL = 6;
static const int FT_WAV = 7;
static const int FT_COL = 8;
static const int FT_DLL = 9;
static const int FT_EFA = 10;
static const int FT_FORM = 11;
static const int FT_FNT = 12;
static const int FT_BWD = 13;
static const int FT_MW2_DB = 14;
static const int FT_TEX = 15;
static const int FT_PCX = 16;
static const int FT_MW2_555 = 17;
static const int FT_MW3_555 = 18;

static const int FT_ZBD = 100; // MW3 project files
static const int FT_FLT = 101; // MW3 mech files

static const int FT_ISO_9660 = 200;

int getFileTypeFromExtension (const TCHAR* FileName);
const TCHAR* getMWFileExt (int FileTypeID);
int getFileTypeFromContent (const MemoryBlock* mb);

////////////////////////////////////////////////////////////////////////////////
// Weapon description records

static const size_t MaxAmmoTypes = 20;

const static DWORD MWWeaponFlag_Ammo = 1;
const static DWORD MWWeaponFlag_MWCanFire = 2;
const static DWORD MWWeaponFlag_IncludeInWeaponList = 4;
const static DWORD MWWeaponFlag_CountedInMEK = 8;

const int LongWeaponNameLen = 30;
const int ShortWeaponNameLen = 11;

// Combinations of flags above
const static DWORD MWWeaponFlagsStdWeapon = 
   MWWeaponFlag_MWCanFire | MWWeaponFlag_IncludeInWeaponList |
   MWWeaponFlag_CountedInMEK;
const static DWORD MWWeaponFlagsStdAmmo = 
   MWWeaponFlag_Ammo | MWWeaponFlag_CountedInMEK;

struct MWWeapons {
   int Heat; // Heat when fired
   int MaxDamage; // Maximum weapon damage
   int MinDamage; // Minimum weapon damage at maximum range
   int MinimumRangeInMeters; // Minimum range for firing the weapon
   int MaxDamageRange; // Maximum range where the weapon does its full damage
   int MaxRangeInMeters; // The maximum range for the weapon does minimum damage
   int TonnageTimes100; // Weapon weight
   int Criticals; // Number of criticals used by the weapon
   int ShotsPerTon; // If positive, this record describes ammo
   int AvailableInYear; // Year of field/open market availability
   int CBills; // Price in C-Bills
   int AmmoTypes[MaxAmmoTypes]; // Ammo that this weapon can use
   //BGString longName, shortName;
   char longName[LongWeaponNameLen], shortName[ShortWeaponNameLen];
   WORD MW2_31st_ID, MW2_ID, MaxCount; // ID's used by MW2
   int inInventory;
   DWORD flags;
   // Missing: reload time, TechType, BV, shells fired per shot, recoil angle amplitude

   const char* getLongName() const
   { return &longName[0]; }
   const char* getShortName() const
   { return &shortName[0]; }
};

static const int ID_UNUSED_MW2 = -1000;

static const int Unused_Critical_Slot = 0;

static const int IS_Energy_Weapon_Base = Unused_Critical_Slot + 1;
static const int IS1_Flamer = IS_Energy_Weapon_Base + 0;
static const int IS1_Small_Laser = IS_Energy_Weapon_Base + 1;
static const int IS1_Medium_Laser = IS_Energy_Weapon_Base + 2;
static const int IS1_Large_Laser = IS_Energy_Weapon_Base + 3;
static const int IS1_PPC = IS_Energy_Weapon_Base + 4;
static const int IS2_Small_Pulse_Laser = IS_Energy_Weapon_Base + 5;
static const int IS2_Medium_Pulse_Laser = IS_Energy_Weapon_Base + 6;
static const int IS1_Large_Pulse_Laser = IS_Energy_Weapon_Base + 7;
static const int IS2_ER_PPC = IS_Energy_Weapon_Base + 8;
static const int IS2_ER_Small_Laser = IS_Energy_Weapon_Base + 9;
static const int IS2_ER_Medium_Laser = IS_Energy_Weapon_Base + 10;
static const int IS2_ER_Large_Laser = IS_Energy_Weapon_Base + 11;

static const int IS_Ballistic_Weapons = IS_Energy_Weapon_Base+12;
static const int IS1_Machine_Gun = IS_Ballistic_Weapons + 0;
static const int IS1_Machine_Gun_Ammo = IS_Ballistic_Weapons + 1;

// Ordering the AC/20 before the AC/2 prevents parsing errors
static const int IS1_Autocannon_20 = IS_Ballistic_Weapons + 2;
static const int IS1_Autocannon_10 = IS_Ballistic_Weapons + 3;
static const int IS1_Autocannon_5 = IS_Ballistic_Weapons + 4;
static const int IS1_Autocannon_2 = IS_Ballistic_Weapons + 5;
static const int IS1_Autocannon_Ammo = IS_Ballistic_Weapons + 6;
static const int IS2_Anti_Missile_System = IS_Ballistic_Weapons + 7; // Uses MG ammo?
static const int IS2_Rotary_AC_2 = IS_Ballistic_Weapons + 8;
static const int IS2_Rotary_AC_5 = IS_Ballistic_Weapons + 9;
static const int IS2_Gauss = IS_Ballistic_Weapons + 10;
static const int IS2_Gauss_Ammo = IS_Ballistic_Weapons + 11;
//static const int IS2_Light_Gauss = IS_Ballistic_Weapons + 12;
//static const int IS2_Light_Gauss_Ammo = IS_Ballistic_Weapons + 13;
//static const int IS2_Heavy_Gauss = IS_Ballistic_Weapons + 14;
//static const int IS2_Heavy_Gauss_Ammo = IS_Ballistic_Weapons + 15;
static const int IS2_Ultra_Autocannon_20 = IS_Ballistic_Weapons + 12;
static const int IS2_Ultra_Autocannon_10 = IS_Ballistic_Weapons + 13;
static const int IS2_Ultra_Autocannon_5 = IS_Ballistic_Weapons + 14;
static const int IS2_Ultra_Autocannon_2 = IS_Ballistic_Weapons + 15;
static const int IS2_LB_X_Autocannon_20 = IS_Ballistic_Weapons + 16;
static const int IS2_LB_X_Autocannon_10 = IS_Ballistic_Weapons + 17;
static const int IS2_LB_X_Autocannon_5 = IS_Ballistic_Weapons + 18;
static const int IS2_LB_X_Autocannon_2 = IS_Ballistic_Weapons + 19;
//static const int IS2_LB_X_Autocannon_Cluster_Ammo = IS_Ballistic_Weapons + 24;

static const int IS_Missile_Weapons = IS_Ballistic_Weapons + 20;
static const int IS1_SRM_2 = IS_Missile_Weapons;
static const int IS1_SRM_4 = IS1_SRM_2 + 1;
static const int IS1_SRM_6 = IS1_SRM_2 + 2;
static const int IS1_LRM_5 = IS1_SRM_2 + 3;
static const int IS1_LRM_10 = IS1_SRM_2 + 4;
static const int IS1_LRM_15 = IS1_SRM_2 + 5;
static const int IS1_LRM_20 = IS1_SRM_2 + 6;
static const int IS1_SRM_Ammo = IS1_SRM_2 + 7;
static const int IS1_SRM_2_Ammo = IS1_SRM_Ammo+1;
static const int IS1_SRM_4_Ammo = IS1_SRM_2_Ammo + 1;
static const int IS1_SRM_6_Ammo = IS1_SRM_4_Ammo + 1;
static const int IS1_LRM_Ammo = IS1_SRM_6_Ammo + 1;
static const int IS1_LRM_5_Ammo = IS1_LRM_Ammo + 1;
static const int IS1_LRM_10_Ammo = IS1_LRM_Ammo + 2;
static const int IS1_LRM_15_Ammo = IS1_LRM_Ammo + 3;
static const int IS1_LRM_20_Ammo = IS1_LRM_Ammo + 4;
static const int IS2_MRM_10 = IS1_LRM_20_Ammo + 1;
static const int IS2_MRM_20 = IS2_MRM_10 + 1;
static const int IS2_MRM_30 = IS2_MRM_10 + 2;
static const int IS2_MRM_40 = IS2_MRM_10 + 3;
static const int IS2_MRM_Ammo = IS2_MRM_10 + 4;

static const int IS2_SRT_2 = IS2_MRM_10 + 5;
static const int IS2_SRT_4 = IS2_SRT_2 + 1;
static const int IS2_SRT_6 = IS2_SRT_2 + 2;
static const int IS2_LRT_5 = IS2_SRT_2 + 3;
static const int IS2_LRT_10 = IS2_LRT_5 + 1;
static const int IS2_LRT_15 = IS2_LRT_5 + 2;
static const int IS2_LRT_20 = IS2_LRT_5 + 3;
static const int IS2_SRT_Ammo = IS2_LRT_5 + 4;
static const int IS2_LRT_Ammo = IS2_LRT_5 + 5;

static const int IS2_Streak_SRM_2 = IS2_LRT_Ammo + 1;
static const int IS2_Streak_SRM_4 = IS2_Streak_SRM_2 + 1;
static const int IS2_Streak_SRM_6 = IS2_Streak_SRM_2 + 2;
static const int IS2_Streak_SRM_Ammo = IS2_Streak_SRM_2 + 3;
static const int IS2_NARC = IS2_Streak_SRM_Ammo + 1;
static const int IS2_NARC_AMMO = IS2_NARC + 1;
static const int IS2_Arrow_IV = IS2_NARC_AMMO+1;
static const int IS2_Arrow_IV_Ammo = IS2_Arrow_IV;
static const int IS2_SRM_Ammo_Inferno = IS2_Arrow_IV_Ammo + 1;
//static const int IS1_Rocket_Launcher_10 = IS_Missile_Weapons + 15;
//static const int IS1_Rocket_Launcher_15 = IS_Missile_Weapons + 16;
//static const int IS1_Rocket_Launcher_20 = IS_Missile_Weapons + 17;

static const int Common_Equipment = IS2_SRM_Ammo_Inferno+1;
static const int CE_Engine = Common_Equipment+0;
static const int CE_Gyro = CE_Engine+1;
static const int CE_Life_Support = CE_Gyro+1;
static const int CE_Cockpit = CE_Life_Support+1;
static const int CE_Sensors = CE_Cockpit+1;
static const int CE_Shoulder = CE_Sensors+1;
static const int CE_Upper_Arm_Actuator = CE_Shoulder+1;
static const int CE_Lower_Arm_Actuator = CE_Upper_Arm_Actuator+1;
static const int CE_Hand = CE_Lower_Arm_Actuator+1;
static const int CE_Hip = CE_Hand+1;
static const int CE_Upper_Leg_Actuator = CE_Hip+1;
static const int CE_Lower_Leg_Actuator = CE_Upper_Leg_Actuator+1;
static const int CE_Foot = CE_Lower_Leg_Actuator+1;
static const int CE_Jump_Jet = CE_Foot+1;

static const int IS_Equipment = CE_Jump_Jet+1;
static const int IS_Heat_Sink = IS_Equipment+0;
static const int IS_Double_Heat_Sink = IS_Heat_Sink+1;
static const int IS_CASE = IS_Double_Heat_Sink+1;
static const int IS_Ferro_Fibrous = IS_CASE+1;
static const int IS_Endo_Steel = IS_Ferro_Fibrous+1;
static const int IS_Beagle = IS_Endo_Steel+1;
static const int IS_MASC = IS_Beagle+1;
static const int IS_Targeting_Computer = IS_MASC+1;
static const int IS_TAG = IS_Targeting_Computer+1;
static const int IS_Hatchet = IS_TAG+1;
static const int IS_Sword = IS_Hatchet+1;

static const int clanFactor = 10; // Price difference factor to IS
static const int Clan_Energy_Weapon_Base = IS_Sword +1;
static const int Clan_ER_Large_Laser = Clan_Energy_Weapon_Base + 0;
static const int Clan_ER_Medium_Laser = Clan_ER_Large_Laser + 1;
static const int Clan_ER_Small_Laser = Clan_ER_Medium_Laser + 1;
static const int Clan_ER_Micro_Laser = Clan_ER_Small_Laser + 1;
static const int Clan_Large_Heavy_Laser = Clan_ER_Micro_Laser + 1;
static const int Clan_Medium_Heavy_Laser = Clan_Large_Heavy_Laser + 1;
static const int Clan_Small_Heavy_Laser = Clan_Medium_Heavy_Laser + 1;
static const int Clan_Flamer = Clan_Small_Heavy_Laser + 1;
static const int Clan_ER_PPC = Clan_Flamer + 1;
static const int Clan_Large_Pulse_Laser = Clan_ER_PPC + 1;
static const int Clan_Medium_Pulse_Laser = Clan_Large_Pulse_Laser + 1;
static const int Clan_Small_Pulse_Laser = Clan_Medium_Pulse_Laser + 1;
static const int Clan_Micro_Pulse_Laser = Clan_Small_Pulse_Laser + 1;

static const int Clan_Ballistic_Weapons = Clan_Micro_Pulse_Laser +1;
static const int Clan_Anti_Missile_System = Clan_Ballistic_Weapons;
static const int Clan_Gauss = Clan_Anti_Missile_System+1;
static const int Clan_LB_20_X_AC = Clan_Gauss+1;
static const int Clan_LB_10_X_AC = Clan_LB_20_X_AC+1;
static const int Clan_LB_5_X_AC = Clan_LB_10_X_AC+1;
static const int Clan_LB_2_X_AC = Clan_LB_5_X_AC+1;
static const int Clan_Light_Machine_Gun = Clan_LB_2_X_AC+1; // Stats missing
static const int Clan_Machine_Gun = Clan_Light_Machine_Gun+1;
static const int Clan_Heavy_Machine_Gun = Clan_Machine_Gun+1; // Stats missing
static const int Clan_UAC_20 = Clan_Heavy_Machine_Gun+1;
static const int Clan_UAC_10 = Clan_UAC_20+1;
static const int Clan_UAC_5 = Clan_UAC_10+1;
static const int Clan_UAC_2 = Clan_UAC_5+1;
static const int Clan_Autocannon_Ammo = Clan_UAC_2+1;
static const int Clan_UAC_20_Ammo = Clan_Autocannon_Ammo+1;
static const int Clan_UAC_10_Ammo = Clan_Autocannon_Ammo+2;
static const int Clan_UAC_5_Ammo  = Clan_Autocannon_Ammo+3;
static const int Clan_UAC_2_Ammo  = Clan_Autocannon_Ammo+4;
static const int Clan_LB_X_Ammo = Clan_UAC_2_Ammo+1;
static const int Clan_LB_20_X_AC_Ammo = Clan_LB_X_Ammo+1;
static const int Clan_LB_10_X_AC_Ammo = Clan_LB_X_Ammo+2;
static const int Clan_LB_5_X_AC_Ammo  = Clan_LB_X_Ammo+3;
static const int Clan_LB_2_X_AC_Ammo  = Clan_LB_X_Ammo+4;
static const int Clan_Machine_Gun_Ammo = Clan_LB_2_X_AC_Ammo+1;
static const int Clan_Gauss_Ammo = Clan_Machine_Gun_Ammo+1;
static const int Clan_Anti_Missile_System_Ammo = Clan_Gauss_Ammo+1;

static const int Clan_Missile_Weapons = Clan_Anti_Missile_System_Ammo+1;
static const int Clan_ATM_3 = Clan_Missile_Weapons;
static const int Clan_ATM_6 = Clan_ATM_3+1;
static const int Clan_ATM_9 = Clan_ATM_6+1;
static const int Clan_ATM_12 = Clan_ATM_9+1;
static const int Clan_ATM_Std_Ammo = Clan_ATM_12+1; // Stats missing
static const int Clan_ATM_ER_Ammo = Clan_ATM_12+1; // Stats missing
static const int Clan_ATM_HE_Ammo = Clan_ATM_ER_Ammo+1; // Stats missing
static const int Clan_SRM_2 = Clan_ATM_HE_Ammo+1;
static const int Clan_SRM_4 = Clan_SRM_2+1;
static const int Clan_SRM_6 = Clan_SRM_4+1;
static const int Clan_SRM_2_Inferno = Clan_SRM_6+1; // Needed for GBL
static const int Clan_LRM_5 = Clan_SRM_2_Inferno+1;
static const int Clan_LRM_10 = Clan_LRM_5+1;
static const int Clan_LRM_15 = Clan_LRM_10+1;
static const int Clan_LRM_20 = Clan_LRM_15+1;
static const int Clan_SRM_Ammo = Clan_LRM_20+1;
static const int Clan_SRM_2_Ammo = Clan_SRM_Ammo+1;
static const int Clan_SRM_4_Ammo = Clan_SRM_2_Ammo+1;
static const int Clan_SRM_6_Ammo = Clan_SRM_4_Ammo+1;
static const int Clan_LRM_Ammo = Clan_SRM_6_Ammo+1;
static const int Clan_LRM_5_Ammo = Clan_LRM_Ammo+1;
static const int Clan_LRM_10_Ammo = Clan_LRM_5_Ammo+1;
static const int Clan_LRM_15_Ammo = Clan_LRM_10_Ammo+1;
static const int Clan_LRM_20_Ammo = Clan_LRM_15_Ammo+1;
static const int Clan_SRM_Ammo_Inferno = Clan_LRM_20_Ammo + 1;
static const int Clan_NARC_Missile_Beacon = Clan_SRM_Ammo_Inferno+1;
static const int Clan_NARC_Missile_Beacon_Ammo = Clan_NARC_Missile_Beacon+1;
static const int Clan_Streak_SRM_2 = Clan_NARC_Missile_Beacon_Ammo+1;
static const int Clan_Streak_SRM_4 = Clan_Streak_SRM_2+1;
static const int Clan_Streak_SRM_6 = Clan_Streak_SRM_4+1;
static const int Clan_Streak_SRM_2_Inferno = Clan_Streak_SRM_6+1; // Needed for GBL
static const int Clan_SSRM_Ammo = Clan_Streak_SRM_2_Inferno+1;
static const int Clan_SSRM_Inferno_Ammo = Clan_SSRM_Inferno_Ammo+1;
static const int Clan_Streak_SRM_2_Ammo = Clan_SSRM_Ammo+1;
static const int Clan_Streak_SRM_4_Ammo = Clan_Streak_SRM_2_Ammo+1;
static const int Clan_Streak_SRM_6_Ammo = Clan_Streak_SRM_4_Ammo+1;
static const int Clan_Arrow_IV = Clan_Streak_SRM_6_Ammo + 1;
static const int Clan_Arrow_IV_Ammo = Clan_Arrow_IV + 1;
static const int Clan_Nuke = Clan_Arrow_IV_Ammo+1;

static const int Clan_SRT_2 = Clan_Nuke + 1;
static const int Clan_SRT_4 = Clan_SRT_2 + 1;
static const int Clan_SRT_6 = Clan_SRT_2 + 2;
static const int Clan_LRT_5 = Clan_SRT_2 + 3;
static const int Clan_LRT_10 = Clan_SRT_2 + 4;
static const int Clan_LRT_15 = Clan_SRT_2 + 5;
static const int Clan_LRT_20 = Clan_SRT_2 + 6;
static const int Clan_SRT_Ammo = Clan_SRT_2 + 7;
static const int Clan_LRT_Ammo = Clan_SRT_2 + 8;

static const int Clan_Equipment = Clan_LRT_Ammo+1;
static const int Clan_Double_Heat_Sink = Clan_Equipment;
static const int Clan_Targeting_Computer = Clan_Double_Heat_Sink+1;
static const int Clan_Ferro_Fibrous = Clan_Targeting_Computer+1;
static const int Clan_Endo_Steel_Internals = Clan_Ferro_Fibrous+1;
static const int Clan_MASC = Clan_Endo_Steel_Internals+1;

static const size_t WeaponsCount = Clan_MASC+1;

// Initialize MW weapons list
void fillWeaponsList();

// Get statistics for a specific weapon
const MWWeapons* getWeaponStats (size_t index);
double getEngineWeight (unsigned int engineRating, int EngineType);
inline bool weaponUsesAmmo (const MWWeapons* w)
{
   if (w->AmmoTypes[0] != 0)
      return false;
   else
      return true;
}

////////////////////////////////////////////////////////////////////////////////
// Mech weapon and equipment defines

const int Biped_Mech_Tech_Type_Common = 0;
const int Biped_Mech_Tech_Type_IS_Level_1 = 1;
const int Biped_Mech_Tech_Type_IS_Level_2 = 2;
const int Biped_Mech_Tech_Type_Clan = 3;

// Engine types. See Maximum Tech, p.58. Not all of these engine types exist
// in the TechManual, see p. 215
static const int ENGINE_TYPE_STD = 0;
static const int ENGINE_TYPE_LARGE = 1;
static const int ENGINE_TYPE_XL = 2;
static const int ENGINE_TYPE_XXL = 3;
static const int ENGINE_TYPE_LXL = 4;
static const int ENGINE_TYPE_LXXL = 5;
static const int ENGINE_TYPE_COMPACT = 6;

//static const int HEAT_SINK_TYPE_SINGLE = 0;
//static const int HEAT_SINK_TYPE_DOUBLE = 1;
//static const int HEAT_SINK_TYPE_COMPACT = 2; // Max. Tech, p. 80
//static const int HEAT_SINK_TYPE_LASER = 3; // Max. Tech, p.80, too strange?

// Maximum Tech offers additional armor types, p. 72
static const int ARMOR_TYPE_STD = 0;
static const int ARMOR_TYPE_LIGHT_FERRO_FIBROUS = 1;
static const int ARMOR_TYPE_FERRO_FIBROUS = 2;
static const int ARMOR_TYPE_HEAVY_FERRO_FIBROUS = 3;
static const int ARMOR_TYPE_STEALTH = 4;

static const int COCKPIT_TYPE_STD = 0;
static const int COCKPIT_TYPE_SMALL = 1;

////////////////////////////////////////////////////////////////////////////////
// Common functions

void getDosBoxPath (BGString& DosBoxPath);

#endif
