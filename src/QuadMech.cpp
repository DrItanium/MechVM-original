////////////////////////////////////////////////////////////////////////////////
// QuadMech.h
// BipedMech inplementation
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

// Quad Mechs are also referred to as four-legged Mechs in the BattleTech
// Master Rules, where they are described in Special Case Rules, 
// p. 75 (v1), p. 82 (v2)

#include "QuadMech.h"

#include "Mesh3.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

QuadMech::QuadMech()
{
   torsoTilt = 0;
   torsoTwist = 0;
   speed = 0;

   for (int i = 0; i < Quad_Mech_Sections; i++) {
      FrontArmor[i] = 0;
      RearArmor[i] = 0;
      InternalStructure[i] = 0;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

QuadMech::~QuadMech()
{
}

////////////////////////////////////////////////////////////////////////////////

const char* QuadMech::getSectionName (int i) const
{
   if (i < 4)
      return tr (i+4);
   else
      return tr (i+34);
}

int QuadMech::getCriticals (int section, int criticalSlot) const
{
   if (section >= Quad_Mech_Sections)
      return Unused_Critical_Slot;
   if (criticalSlot >= (int) getCriticalCount(section))
      return Unused_Critical_Slot;

   return criticals[section][criticalSlot];
}

////////////////////////////////////////////////////////////////////////////////
// Query/set data

int QuadMech::getInteger (int info) const
{
   return Vehicle::getInteger(info);
}

bool QuadMech::setInteger (int info, int newVal)
{
   return Vehicle::setInteger(info, newVal);
}

double QuadMech::getAnimParam (size_t level, size_t ID)
{
   // todo
   return 0.0;
}

void QuadMech::setAnimParam (size_t level, size_t ID, double val)
{
   // todo
}

////////////////////////////////////////////////////////////////////////////////
// Add weapon

bool QuadMech::addWeapon(int currSection, int weaponNo)
{
   const MWWeapons* weapon = getWeaponStats(weaponNo);

   if (weapon == NULL)
      return false;

   int freeCriticals = countWeaponCriticals (currSection, Unused_Critical_Slot);

   if (weaponNo == IS_Heat_Sink
   ||  weaponNo == IS_Double_Heat_Sink
   ||  weaponNo == Clan_Double_Heat_Sink)
   {
      if (weaponNo != HeatSinkType) {
         printf ("Incompatible heat sink type\n");
         return false;
      }

      if (heatSinks < engineRating / 25) {
         // Heat sink that does not need critical space, add and abort
         ++heatSinks;
         printf("Internal heat sink added - %i\n", heatSinks);
         return true;
      } else {
         if (freeCriticals >= weapon->Criticals) {
            // Heat sink that needs normal critical space, treated normally
            ++heatSinks;
            printf("External heat sink added - %i\n", heatSinks);
         } else
            // Heat sink needs critical space, but no criticals available
            return false;
      }
   }

   // Add criticals
   if (freeCriticals < weapon->Criticals)
      return false;

   WeaponSystem* ws = NULL;
   if ((weapon->flags & MWWeaponFlag_IncludeInWeaponList) > 0) {
      WeaponSystem& wref = weapons.add();
      ws = &wref;
      wref.weaponNum = weaponNo;
      wref.weapon = weapon;
      wref.isInGroup[0] = true;

      for (size_t i = 0; i < MaxCritCount; ++i) {
         wref.section[i] = -1;
         wref.critical[i] = -1;
      }

      for (size_t i = 1; i < MaxGroupCount; ++i) {
         wref.isInGroup[i] = false;
      }
   }

   int added = 0;
   int i = 0;
   int max = getCriticalCount(currSection);

   while (i < max && added < weapon->Criticals) {
      if (criticals[currSection][i] == Unused_Critical_Slot) {
         criticals[currSection][i] = weaponNo;

         if (ws != NULL) {
            ws->section[added] = currSection;
            ws->critical[added] = i;
         }

         ++added;
      } 
      ++i;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////
// Remove weapon - returns number of criticals removed

int QuadMech::removeWeapon(int currSection, int critNo, int weaponNo, 
                           int critCount)
{
   if (weaponNo == -1)
      weaponNo = criticals[currSection][critNo];

   if (weaponNo == IS_Heat_Sink
   ||  weaponNo == IS_Double_Heat_Sink
   ||  weaponNo == Clan_Double_Heat_Sink)
   {
      --heatSinks;
      printf ("External heat sink removed\n");
   }

   int removed = 0;
   int i = critNo;
   int max = getCriticalCount(currSection);

   if (critCount == -1) {
      const MWWeapons* weapon = getWeaponStats(weaponNo);
      if (weapon == NULL)
         return 0;
      critCount = weapon->Criticals;
   }

   int oneCrit = -1;
   while (removed < critCount 
   &&     i < 2*max) // test against twice crit count if tests start in last critical
   {
      if (criticals[currSection][i%max] == weaponNo) {
         criticals[currSection][i%max] = Unused_Critical_Slot;
         oneCrit = i%max;
         ++removed;
      } 
      ++i;
   }

   // Check and update weapon systems table
   if (removed > 0 && oneCrit >= 0) {
      size_t i = 0, j;
      while (i < weapons.getSize()) {
         j = 0;
         while (j < MaxCritCount) {
            if (weapons[i].section[j] == currSection 
            &&  weapons[i].critical[j] == oneCrit)
            {
               weapons.remove(i);
               j = MaxCritCount;
            } else
               ++j;
         }
         ++i;
      }
   }

   return removed;
}

////////////////////////////////////////////////////////////////////////////////
// Check - and try to correct - config problems 
// Returns 0 on success, and the number of a warning for the tr() function otherwise

int QuadMech::checkAndFixEngineCrits(int section)
{
   int engineCritsCurrentlyInSection = countWeaponCriticals (section, CE_Engine);
   int reqSideTorsoSlots = 3;
   if (techType == Biped_Mech_Tech_Type_Clan)
      reqSideTorsoSlots = 2;

   if (engineCritsCurrentlyInSection == reqSideTorsoSlots) 
      return 0;

   int unusedCritsInSection = countWeaponCriticals (section, Unused_Critical_Slot);
   if (reqSideTorsoSlots - engineCritsCurrentlyInSection < unusedCritsInSection) {
      // Add missing side torso engine slots
      while (engineCritsCurrentlyInSection < reqSideTorsoSlots) {
         addWeapon (section, CE_Engine);
         ++engineCritsCurrentlyInSection;
      }
      return 0;
   } else {
      // Can't add missing side torso slots because there not enough free criticals
      return 27;
   }
}

int QuadMech::check()
{
   // Overweight?
   if (getWeight() > tonnage)
      return 26;

   // XL engine criticals in side torsi?
   if (getInteger(Info_EngineType) == ENGINE_TYPE_XL) {
      int engRes = checkAndFixEngineCrits(Quad_Mech_Section_Left_Torso);
      if (engRes > 0)
         return engRes;

      engRes = checkAndFixEngineCrits(Quad_Mech_Section_Right_Torso);
      if (engRes > 0)
         return engRes;
   }

   // Check for correct number of endo steel internals
   int endoCritsIS = countWeaponInAllSections(IS_Endo_Steel);
   int endoCritsClan = countWeaponInAllSections(Clan_Endo_Steel_Internals);
   if ((endoCritsIS != 0   && endoCritsIS != 14)
   ||  (endoCritsClan != 0 && endoCritsClan != 7))
      return 36;

   // Check for correct number of endo steel internals
   int ffCritsIS = countWeaponInAllSections(IS_Ferro_Fibrous);
   int ffCritsClan = countWeaponInAllSections(Clan_Ferro_Fibrous);
   if ((ffCritsIS != 0   && ffCritsIS != 14)
   ||  (ffCritsClan != 0 && ffCritsClan != 7))
      return 37;

   // Check if the jump jets are less or equals to the walking movement rate
   int jjCrits = countWeaponInAllSections(CE_Jump_Jet);
   if (jjCrits > getInteger (Info_EngineRating) / tonnage)
      return 43;

   // Check leg criticals
   for (size_t i = Quad_Mech_Section_Left_Front_Leg; 
        i <= Quad_Mech_Section_Right_Rear_Leg;
        ++i)
   {
      int feet = countWeaponCriticals(i, CE_Foot);

      if (feet == 0) {
         int unused = countWeaponCriticals(i, Unused_Critical_Slot);
         if (unused > 0) {
            addWeapon(i, CE_Foot);
         } else {
            return 46;
         }
      }
   }

   // No errors found
   return 0;
}

////////////////////////////////////////////////////////////////////////////////

double QuadMech::calcInternalStructureWeight() const
{
   if (InternalStructureType == Unused_Critical_Slot)
      return tonnage / 10.0;
   else
      return tonnage / 20.0;
}

double QuadMech::calcJJWeight () 
{
   jumpJets = countWeaponInAllSections(CE_Jump_Jet);

   if (tonnage > 85)
      return 2*jumpJets;
   else if (tonnage > 55)
      return jumpJets;
   else
      return jumpJets / 2.0;
}

////////////////////////////////////////////////////////////////////////////////

int QuadMech::countArmorPoints() const
{
   int armorPoints = RearArmor[Quad_Mech_Section_Center_Torso] +
                     RearArmor[Quad_Mech_Section_Left_Torso] +
                     RearArmor[Quad_Mech_Section_Right_Torso];
                
   for (int i = 0; i < 8; i++) 
      armorPoints += FrontArmor[i];

   return armorPoints;
}

////////////////////////////////////////////////////////////////////////////////

double QuadMech::calcRealWeight()
{
   // Add other equipment
   double equipmentTonnage = 0;
   for (int i = 0; i < Quad_Mech_Sections; i++) {
      for (size_t j = 0; j < getCriticalCount (i); j++) {
         int crit = criticals[i][j];
         const MWWeapons* weapon = getWeaponStats (crit);
         if (weapon != NULL)
            if (weapon->TonnageTimes100 > 0.0)
               equipmentTonnage += weapon->TonnageTimes100 / (100.0*weapon->Criticals);
      }
   }

   double engineWeight = getEngineWeight (engineRating, EngineType);
   double internalStructureWeight = calcInternalStructureWeight();
   double armorWeight = calcArmorWeight();
   double gyroWeight = (engineRating + 95) / 100;
   double HeatSinkWeight = bgmax(heatSinks-10,0);
   double cockpitWeight = 3.0;
   double jjWeight = calcJJWeight();

   int MASCTonsAndSlots = calcMASCTonsAndSlots();
   int IS_MASC_slots = countWeaponInAllSections (IS_MASC);
   int Clan_MASC_slots = countWeaponInAllSections (Clan_MASC);

   if (IS_MASC_slots < MASCTonsAndSlots
   &&  Clan_MASC_slots < MASCTonsAndSlots)
   {
      MASCTonsAndSlots = 0;
   }

   realWeight = engineWeight + internalStructureWeight + gyroWeight +
                armorWeight + equipmentTonnage + cockpitWeight +
                HeatSinkWeight + jjWeight + MASCTonsAndSlots;

   return realWeight;
}

////////////////////////////////////////////////////////////////////////////////
// Load mech limb

void QuadMech::loadPart (XMLTreeConstIterator& iter, int partNumber, 
   const BGString& path)
{
   const char* MW2PartName = iter.getAttributeValueByName ("mw2prj");
   const char* PartFileName = iter.getAttributeValueByName ("path");
   if ((MW2PartName != NULL)
   ||  PartFileName != NULL) {
      double tx, ty, tz, jx, jy, jz;
      BGString txc = iter.getAttributeValueByName("tx");
      BGString tyc = iter.getAttributeValueByName("ty");
      BGString tzc = iter.getAttributeValueByName("tz");
      if (!txc.toDouble (tx))
         tx = 0;
      if (!tyc.toDouble (ty))
         ty = 0;
      if (!tzc.toDouble (tz))
         tz = 0;
      BGString jxc = iter.getAttributeValueByName("jx");
      BGString jyc = iter.getAttributeValueByName("jy");
      BGString jzc = iter.getAttributeValueByName("jz");
      if(!jxc.toDouble (jx))
         jx = 0;
      if(!jyc.toDouble (jy))
         jy = 0;
      if(!jzc.toDouble (jz))
         jz = 0;

      BGString mechPath = path;
      mechPath.appendNoDups(OSPathSep);
      mechPath += PartFileName;
      Mesh* mesh = new Mesh();
      //printf ("Loading %s from %s\n", PartFileName, mechPath.getChars());
      if (mesh->loadFromObj(mechPath.getChars())) {
         mesh->scaleNormals(-1.0);
         //m_joint[partNumber] = Point3D (jx, jy, jz);
         addGeometry(partNumber, mesh, Point3D(tx, ty, tz));
      } else {
         printf("Could not load ");
         printf(mechPath.getChars());
         printf("\n");
         delete mesh;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Load entire mech

void QuadMech::loadVehicleGeometry (XMLTreeConstIterator& iter, 
                                    const BGString& path, Texture* texture)
{
   iter.goToChild(0);
   BGString mechName = iter.getAttributeValueByName("name");
   printf ("Loading %s\n", mechName.getChars());
   setName (mechName.getChars());

   // Read scale
   double scale;
   bool hasStats = false;
   BGString scaleText = iter.getAttributeValueByName("scale");

   if (scaleText.toDouble(scale))
       p_scale.set(scale);

   for (size_t i = 0; i < Mech_MAX_PARTS; ++i) {
      Mesh3* mesh = new Mesh3(this);
      mesh->setTexture(texture, false);
      childs[i] = mesh;
   }

   // Load mech parts
   for (size_t j = 0; j < iter.getChildCount(); j++) {
      iter.goToChild(j);
      const char* partName = iter.getNodeName();
      int partNumber = Mech_Hip;
      bool LoadPart = true;

      if (strcmp (partName, "hip") == 0)
         partNumber = Mech_Hip;
      else if (strcmp (partName, "torso") == 0)
         partNumber = Mech_Torso;
      else if (strcmp (partName, "LeftArm") == 0)
         partNumber = Mech_LeftArm;
      else if (strcmp (partName, "RightArm") == 0)
         partNumber = Mech_RightArm;
      else if (strcmp (partName, "LeftUpperLeg") == 0)
         partNumber = Mech_LeftUpperLeg;
      else if (strcmp (partName, "LeftLowerLeg") == 0)
         partNumber = Mech_LeftLowerLeg;
      else if (strcmp (partName, "LeftFoot") == 0)
         partNumber = Mech_LeftFoot;
      else if (strcmp (partName, "RightUpperLeg") == 0)
         partNumber = Mech_RightUpperLeg;
      else if (strcmp (partName, "RightLowerLeg") == 0)
         partNumber = Mech_RightLowerLeg;
      else if (strcmp (partName, "RightFoot") == 0)
         partNumber = Mech_RightFoot;
      else if (strcmp (partName, "windshield") == 0) {
         partNumber = Mech_Windshield;
      /*} else if (strcmp (partName, "anim") == 0) {
         LoadPart = false;
         addAnim (iter);*/
      /*} else if (strcmp (partName, "stats") == 0) {
         LoadPart = false;
         if (hasStats) {
            BipedMech* nuMech = new BipedMech (*this);
            nuMech->loadStats (iter);
            mechs.push_back (nuMech);
         } else {
            loadStats (iter);
            hasStats = true;
         }*/
      } else
         LoadPart = false;

      if (LoadPart) {
         loadPart (iter, partNumber, path);
      }
      iter.goToParent();
   }
}

////////////////////////////////////////////////////////////////////////////////
// Add geometry for a vehicle part

/*void BipedMech::addMW2Geometry (int geometryType, const TCHAR* FN, double tx, 
   double ty, double tz, double jx, double jy, double jz)
{
   MemoryBlock* block = m_MW2PRJ->loadFile (FN);
   if (block != NULL) {
      Mesh* part = m_MW2PRJ->loadMesh(block);
      delete block;
      part->calculateNormals();
      addGeometry(geometryType, part, tx, ty, tz, jx, jy, jz);
   } else {
      printf ("Could not load ");
      printf (FN);
      printf ("\n");
   }
}*/

void QuadMech::addGeometry (int geometryType, Mesh* geometry, 
   const Point3D& dist)
{
   Mesh3*& oldGeo = (Mesh3*&) (childs[geometryType]);
   size_t basePoints = 0;

   if (oldGeo != NULL) 
      basePoints = oldGeo->getPointCount();
   else
      oldGeo = new Mesh3(this);

   for (size_t i = 0; i < geometry->getPolygonCount(); i++) {
      MeshPolygon* poly = geometry->getPolygon(i);
      size_t polySize = poly->getPointCount();
      size_t polyIndex = oldGeo->addPolygon(polySize);

      for (size_t j = 0; j < polySize; j++) {
         // We need a separate copy of every point, because it may 
         // have different texture coordinates
         Point3D p;
         size_t index = poly->getPoint(j);
         geometry->getPoint(index, p);
         size_t oldGeoIndex = oldGeo->addVertex(p+dist);
         oldGeo->setVertex(polySize, polyIndex, j, oldGeoIndex);

         double u, v;
         poly->getTexCoords(j, u, v);
         oldGeo->setTexCoords(polySize, polyIndex, j, u, v);
      }
   }
   delete geometry;
}

////////////////////////////////////////////////////////////////////////////////
// Set current animation matrices

void QuadMech::setMatrices ()
{
   double arr[16] = {p_left.get().x, p_up.get().x, p_front.get().x, 0.0, 
                     p_left.get().y, p_up.get().y, p_front.get().y, 0.0, 
                     p_left.get().z, p_up.get().z, p_front.get().z, 0.0, 
                     0.0, 0.0, 0.0, 1.0};

   ScaleMatrix s (p_scale.get(), p_scale.get(), p_scale.get(), 1.0);
   TranslationMatrix t1 (p_position.get().x, p_position.get().y, p_position.get().z);
   Matrix M = t1 * s * Matrix(arr);

   for (size_t i = 0; i < getObjectCount(); ++i) {
      RenderableObject* ro = getObject(i);
      ro->setMatrix(M);
   }
}

////////////////////////////////////////////////////////////////////////////////

DWORD QuadMech::getArmor(int section, int ArmorType) const
{
   if (section < Quad_Mech_Sections) {
      if (ArmorType == Vehicle::Armor_Front)
         return FrontArmor[section];
      else if (ArmorType == Vehicle::Armor_Rear)
         return RearArmor[section];
   }

   return 0;
}

bool QuadMech::setArmor(int section, DWORD newVal, int ArmorType)
{ 
   bool maySet = false;

   if (ArmorType == Armor_Front) {
      if (section == Quad_Mech_Section_Center_Torso
      ||  section ==  Quad_Mech_Section_Left_Torso
      ||  section ==  Quad_Mech_Section_Right_Torso)
      {
         if ((int) newVal + RearArmor[section] <= 2* InternalStructure[section])
            maySet = true;
      } else if (section == Quad_Mech_Section_Head) {
         if (newVal < 10)
            maySet = true;
      } else {
         if ((int) newVal <= 2* InternalStructure[section])
            maySet = true;
      }
   } else {
      if (section == Quad_Mech_Section_Center_Torso
      ||  section ==  Quad_Mech_Section_Left_Torso
      ||  section ==  Quad_Mech_Section_Right_Torso)
      {
         if ((int) newVal + FrontArmor[section] <= 2* InternalStructure[section])
            maySet = true;
      }
   }

   if (maySet) {
      if (ArmorType == Armor_Front) {
         FrontArmor[section] = newVal;
      } else {
         RearArmor[section] = newVal;
      }
      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////

void QuadMech::setInternalStructure (int section, DWORD newVal)
{
   if (section < Quad_Mech_Sections) {
      InternalStructure[section] = newVal;
   }
}

////////////////////////////////////////////////////////////////////////////////

void QuadMech::setCriticals (int section, int slot, int weaponType)
{
   if (section < Quad_Mech_Sections) {
      criticals[section][slot] = weaponType;
   }
}
