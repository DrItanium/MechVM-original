////////////////////////////////////////////////////////////////////////////////
// BipedMech.cpp
// BipedMech inplementation
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "BipedMech.h"
#include "FileCache.h"
#include "FramerateCounter.h"
#include "Config.h"
#include "MWBase.h"

#include "Mesh3.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

BipedMech::BipedMech()
: legsParallelToGround (false)
{
   m_joint = new Point3D[Mech_MAX_PARTS];
   childs.resize (Mech_MAX_PARTS);

   for (size_t i = 0; i < Mech_MAX_PARTS; i++) {
      childs[i] = NULL;
      m_joint[i] = Point3D(0, 0, 0);
   }

   for (int i = 0; i < Biped_Mech_Sections; i++) {
      FrontArmor[i] = 0;
      RearArmor[i] = 0;
      InternalStructure[i] = 0;
   }

   for (int i = 0; i < Biped_Mech_Sections; i++)
      for (size_t j = 0; j < getCriticalCount (i); j++)
            criticals[i][j] = Unused_Critical_Slot;
}

BipedMech::BipedMech (const BipedMech& /*other*/)
{
//   syntax_error(); // todo: fill with functionality!
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

BipedMech::~BipedMech()
{
//   delete m_joint;
}

////////////////////////////////////////////////////////////////////////////////

void BipedMech::parseCrit (int section, BGString& crit)
{
   size_t pos = 0;
   size_t critPos = 0;
   //printf ("Loading stats for %s, %s\n", MechTypeName.getChars(), 
   //        BipedMech::getSectionName(section));

   while (pos < crit.getLength()) {
      // Remove leading whitespace
      int ciphers = 0;
      int critNum = 1;
      while (crit[pos] == ' ' || crit[pos] == 9 || crit[pos] == 0x0a || crit[pos] == 0x0d)
         pos++;

      // Check for leading ciphers
      size_t cipherStart = pos;
      while (crit[pos] >= '0' && crit[pos] <= '9') {
         ciphers++;
         pos++;
      }

      if (cipherStart < pos) {
         BGString numStr;
         numStr.copyPart(crit.getChars(), cipherStart, pos-cipherStart);
         critNum = numStr.toInt();
         pos++;
      }

      // Check if a critical count is given instead of a weapon count
      bool isCritCount = false;
      const char* critStr = &crit.getChars()[pos];
      if (strlen (critStr) > 10)
         if (memcmp (critStr, "criticals ", 10) == 0) {
            pos += 10;
            isCritCount = true;
         }

      // Read weapon entry
      size_t scanStart = IS_Energy_Weapon_Base;
      size_t scanStop = Clan_Energy_Weapon_Base;
      if (techType == Biped_Mech_Tech_Type_Clan) {
         scanStart = Common_Equipment;
         scanStop = WeaponsCount;
      }

      const MWWeapons* weapon = NULL;
      size_t i = scanStart;
      while (i < scanStop && weapon == NULL) {
         bool found = false;
         weapon = getWeaponStats (i);
         const char* sname = weapon->getShortName();
         const char* lname = weapon->getLongName();
         size_t slen = strlen(sname);
         size_t llen = strlen(lname);
         if (pos+slen <= crit.getLength()) {
            found = memcmp (sname, &crit.getChars()[pos], slen) == 0;
            if (found)
               pos += slen;
         }
         if (pos+llen <= crit.getLength() && !found) {
            found = memcmp (lname, &crit.getChars()[pos], llen) == 0;
            if (found)
               pos += llen;
         }
         if (!found) {
            i++;
            weapon = NULL;
         }
      }

      // Write results to table
      if (weapon != NULL) {
         if (!isCritCount) 
            critNum = critNum*weapon->Criticals;
         size_t maxCrits = getCriticalCount (section);
         while (critPos < maxCrits && critNum > 0) {
            if (criticals[section][critPos] == Unused_Critical_Slot) {
               criticals[section][critPos] = i;
               critNum--;
            }
            critPos++;
         }
      } else if (pos < crit.getLength()) {
         size_t EOL = pos;
         while ((crit[EOL] != 0x0a) 
         &&     (crit[EOL] != 0x0d) 
         &&     (EOL < crit.getLength()-1))
         {
            EOL++;
         }
         pos = EOL+1;
         BGString unparsable;
         unparsable.copyPart(crit.getChars(), pos, EOL-pos);
         printf ("Parse error: %s\n", unparsable.getChars());
      }
   }
}

void BipedMech::fillCriticalHitsTable (XMLTreeConstIterator& iter, int section)
{
   // Armor & internal structure
   FrontArmor[section] = iter.getIntAttributeValueByName ("af");
   if (FrontArmor[section] == 0) {
      FrontArmor[section] = iter.getIntAttributeValueByName ("a");
   }
   RearArmor[section] = iter.getIntAttributeValueByName ("ar");
   InternalStructure[section] = iter.getIntAttributeValueByName ("is");

   // Now for the real meat ...
   for (size_t i = 0; i < iter.getChildCount(); i++) {
      iter.goToChild(i);
      BGString crit = iter.getText();
      parseCrit (section, crit);
      iter.goToParent();
   }
}

void BipedMech::loadStats (XMLTreeConstIterator& iter)
{
   // Load base stats
   setName (iter.getAttributeValueByName("name"));
   techType = Biped_Mech_Tech_Type_IS_Level_1;
   BGString techTypeStr = iter.getAttributeValueByName("tech");
   if (techTypeStr.equals ("IS2"))
      techType = Biped_Mech_Tech_Type_IS_Level_2;
   else if (techTypeStr.equals ("Clan"))
      techType = Biped_Mech_Tech_Type_Clan;

   tonnage = iter.getIntAttributeValueByName("mass");

   heatSinks = iter.getIntAttributeValueByName("doubleHeatSinks");
   if (heatSinks == 0) {
      heatSinks = iter.getIntAttributeValueByName("heatSinks");
      HeatSinkType = IS_Heat_Sink;
   } else {
      if (techType == Biped_Mech_Tech_Type_IS_Level_2)
         HeatSinkType = IS_Double_Heat_Sink;
      else
         HeatSinkType = Clan_Double_Heat_Sink;
   }

   BGString engineStr = iter.getAttributeValueByName ("engine");
   if (engineStr.endsWith ("XL")) {
      BGString shortStr;
      shortStr.copyPart(engineStr.getChars(), 0, engineStr.getLength()-2);
      EngineType = ENGINE_TYPE_XL;
      engineRating = shortStr.toInt();
   } else {
      EngineType = ENGINE_TYPE_STD;
      engineRating = engineStr.toInt();
   }

   for (size_t i = 0; i < iter.getChildCount(); i++) {
      iter.goToChild(i);
      const char* nodeName = iter.getNodeName();
      for (int j = 0; j < Biped_Mech_Sections; j++) {
         const char* sectionName = getSectionName(j);
         if (strcmp (nodeName, sectionName) == 0)
            fillCriticalHitsTable (iter, j);
      }
      iter.goToParent();
   }

   // todo: count jump jets in critical hits table
}

////////////////////////////////////////////////////////////////////////////////
// Add anim parameters

void BipedMech::addAnim (XMLTreeConstIterator& iter)
{
   double phase = 0, ulegAngle = 0, llegAngle = 0, footAngle = 0;
   iter.getDoubleAttributeByName ("phase", phase);
   iter.getDoubleAttributeByName ("ulegAngle", ulegAngle);
   iter.getDoubleAttributeByName ("llegAngle", llegAngle);
   iter.getDoubleAttributeByName ("footAngle", footAngle);

   animAngles.add(phase);
   animAngles.add(ulegAngle);
   animAngles.add(llegAngle);
   animAngles.add(footAngle);
 
   if (iter.getIntAttributeValueByName ("parallelFeet") != 0)
      legsParallelToGround = true;
}

////////////////////////////////////////////////////////////////////////////////
// Load mech limb

void BipedMech::loadPart (XMLTreeConstIterator& iter, int partNumber, 
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
         mesh->calculateNormals();
         mesh->scaleNormals(-1.0);
         m_joint[partNumber] = Point3D (jx, jy, jz);
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

void BipedMech::loadVehicleGeometry (XMLTreeConstIterator& iter, 
                                     const BGString& path, Texture* texture)
{
   //iter.goToChild(0);
   BGString mechName = iter.getAttributeValueByName("name");
   printf ("Loading %s\n", mechName.getChars());
   setName (mechName.getChars());

   // Read scale
   double scale;
   //bool hasStats = false;
   BGString scaleText = iter.getAttributeValueByName("scale");

   if (scaleText.toDouble(scale))
       p_scale.set(scale);

   for (size_t i = 0; i < Mech_MAX_PARTS; ++i) {
      Mesh* mesh = new Mesh(this);
      mesh->setTexture(texture); //, false);
      //Mesh3* mesh = new Mesh3(this);
      //mesh->setTexture(texture, false);
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
      else if (strcmp (partName, "windshield") == 0)
         partNumber = Mech_Windshield;
      else if (strcmp (partName, "anim") == 0) {
         LoadPart = false;
         addAnim (iter);
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

void BipedMech::addGeometry (int geometryType, Mesh* geometry, 
   const Point3D& dist)
{
   // Check whether BipedMech::loadVehicleGeometry created Mesh or Mesh3
   Mesh3* oldGeo3 = dynamic_cast <Mesh3*> (childs[geometryType]);

   if (oldGeo3 != NULL) {
      //size_t basePoints = oldGeo3->getPointCount();

      for (size_t i = 0; i < geometry->getPolygonCount(); i++) {
	 MeshPolygon* poly = geometry->getPolygon(i);
	 size_t polySize = poly->getPointCount();
	    size_t polyIndex = oldGeo3->addPolygon(polySize);

	 for (size_t j = 0; j < polySize; j++) {
	    // We need a separate copy of every point, because it may 
	    // have different texture coordinates
	    Point3D p;
	    size_t index = poly->getPoint(j);
	    geometry->getPoint(index, p);
	       size_t oldGeoIndex = oldGeo3->addVertex(p+dist);
	       oldGeo3->setVertex(polySize, polyIndex, j, oldGeoIndex);

	       Point3D& n = poly->normal;
	       oldGeo3->setNormal(polySize, polyIndex, j, n.x, n.y, n.z);

	       double u, v;
	       poly->getTexCoords(j, u, v);
	       oldGeo3->setTexCoords(polySize, polyIndex, j, u, v);
	    }
	 }
	 delete geometry;
      } else {
	 Mesh* oldGeo = dynamic_cast <Mesh*> (childs[geometryType]);
	 size_t basePoints = oldGeo->getPointCount();

	 for (size_t i = 0; i < geometry->getPointCount(); i++) {
	    Point3D p;
	    geometry->getPoint(i, p);
	    p += dist;
	    oldGeo->setPoint(basePoints+i, p);
	 }

	 for (size_t i = 0; i < geometry->getPolygonCount(); i++) {
	    MeshPolygon* poly = geometry->getPolygon(i);
	    size_t polySize = poly->getPointCount();

	    MeshPolygon* newPoly = oldGeo->addPolygon(polySize);
	    newPoly->normal = poly->normal;
	    //newPoly->setTexture(geometry->getTexture(), true);

	    for (size_t j = 0; j < polySize; j++) {
	       size_t pnum = poly->getPoint(j);
	       newPoly->setPoint(j, basePoints+pnum);
	    double u, v;
	    poly->getTexCoords(j, u, v);
	       newPoly->setTexCoords(j, u, v);
	 }
      }
   delete geometry;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Render entire scene

inline void setMatrixIfNN (RenderableObject* ro, const Matrix& M)
{
   if (ro != NULL)
      ro->setMatrix(M);
}

void BipedMech::setMatrices ()
{
   const bool renderPoints = false;
   double ulegAngle1 = 0, ulegAngle2 = 0, llegAngle1 = 0, llegAngle2 = 0, 
      footAngle1 = 0, footAngle2 = 0, phase1 = 0, phase2 = 0.5;

   if (animAngles.getSize() > 2*BipedAnimFootAngle) {
      phase1 =     animAngles[0];
      ulegAngle1 = animAngles[1];
      llegAngle1 = animAngles[2];
      footAngle1 = animAngles[3];
      phase2 =     animAngles[4];
      ulegAngle2 = animAngles[5];
      llegAngle2 = animAngles[6];
      footAngle2 = animAngles[7];
   }

   double arr[16] = {p_left.get().x, p_up.get().x, p_front.get().x, 0.0, 
                     p_left.get().y, p_up.get().y, p_front.get().y, 0.0, 
                     p_left.get().z, p_up.get().z, p_front.get().z, 0.0, 
                     0.0, 0.0, 0.0, 1.0};

   ScaleMatrix s (p_scale.get(), p_scale.get(), p_scale.get(), 1.0);
   TranslationMatrix t1 (p_position.get().x, p_position.get().y, p_position.get().z);
   Matrix M = t1 * s * Matrix(arr);

   //const int Mech_Hip = 0;
   setMatrixIfNN(childs[0], M);

   //const int Mech_Torso = 1;
   setMatrixIfNN(childs[1], M);

   //const int Mech_Windshield = 10;
   setMatrixIfNN(childs[10], M);

   //const int Mech_LeftArm = 2;
   setMatrixIfNN(childs[2], M);

   //const int Mech_RightArm = 3;
   setMatrixIfNN(childs[3], M);

   //const int Mech_LeftUpperLeg = 4;
   Matrix LeftLegMatrix (M);
   const double secondsPerStep = 1;
   const double kneeAngleAmp = 45.0;
   const double time = GetTickCount()/1000.0;

   double phase = M_PI*time/secondsPerStep;
   double upperLegAngle = ulegAngle1 + (ulegAngle2-ulegAngle1)*sin(phase);
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (m_joint[4]);
   LeftLegMatrix = LeftLegMatrix * RotationMatrix (M_PI*upperLegAngle/180.0, 1, 0, 0);
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (-m_joint[4]);
   setMatrixIfNN(childs[4], LeftLegMatrix);

   //const int Mech_LeftLowerLeg = 5;
   phase = M_PI*(time/secondsPerStep+0.5);
   double lowerLegAngle = llegAngle1 + (llegAngle2-llegAngle1)*sin(phase);
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (m_joint[5]);
   LeftLegMatrix = LeftLegMatrix * RotationMatrix (M_PI*lowerLegAngle/180.0, 1, 0, 0);
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (-m_joint[5]);
   setMatrixIfNN(childs[5], LeftLegMatrix);

   //const int Mech_LeftFoot= 6;
   double footAngle;
   if (legsParallelToGround) {
      footAngle = -upperLegAngle-lowerLegAngle;
   } else {
      phase = M_PI*(time/secondsPerStep);
      footAngle = footAngle1 + (footAngle2-footAngle1)*sin(phase);
   }
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (m_joint[6]);
   LeftLegMatrix = LeftLegMatrix * RotationMatrix (M_PI*footAngle/180.0, 1, 0, 0);
   LeftLegMatrix = LeftLegMatrix * TranslationMatrix (-m_joint[6]);

   setMatrixIfNN(childs[6], LeftLegMatrix);

   //const int Mech_RightUpperLeg = 7;
   Matrix RightLegMatrix (M);
   phase = M_PI*(time/secondsPerStep + 1);
   upperLegAngle = ulegAngle1 + (ulegAngle2-ulegAngle1)*sin(phase);
   RightLegMatrix = RightLegMatrix * TranslationMatrix (m_joint[7]);
   RightLegMatrix = RightLegMatrix * RotationMatrix (M_PI*upperLegAngle/180.0, 1, 0, 0);
   RightLegMatrix = RightLegMatrix * TranslationMatrix (-m_joint[7]);
   setMatrixIfNN(childs[7], RightLegMatrix);

   //const int Mech_RightLowerLeg = 8;
   phase = M_PI*(time/secondsPerStep+1.5);
   lowerLegAngle = llegAngle1 + (llegAngle2-llegAngle1)*sin(phase);
   RightLegMatrix = RightLegMatrix * TranslationMatrix (m_joint[8]);
   RightLegMatrix = RightLegMatrix * RotationMatrix (M_PI*lowerLegAngle/180.0, 1, 0, 0);
   RightLegMatrix = RightLegMatrix * TranslationMatrix (-m_joint[8]);
   setMatrixIfNN(childs[8], RightLegMatrix);

   //const int Mech_RightFoot = 9;
   if (legsParallelToGround) {
      footAngle = -upperLegAngle-lowerLegAngle;
   } else {
      phase = M_PI*(time/secondsPerStep+1);
      footAngle = footAngle1 + (footAngle2-footAngle1)*sin(phase);
   }
   RightLegMatrix = RightLegMatrix * TranslationMatrix (m_joint[9]);
   RightLegMatrix = RightLegMatrix * RotationMatrix (M_PI*footAngle/180.0, 1, 0, 0);
   RightLegMatrix = RightLegMatrix * TranslationMatrix (-m_joint[9]);

   setMatrixIfNN(childs[9], RightLegMatrix);
}

////////////////////////////////////////////////////////////////////////////////
// Save config to XML file

bool BipedMech::saveToTree (XMLTreeIterator& MechVMMechTreeIter) 
{
   // Basic stats: tonnage, engine, heat sinks
   size_t statsChild = MechVMMechTreeIter.addChild("stats");
   MechVMMechTreeIter.goToChild(statsChild);
   MechVMMechTreeIter.addAttribute ("mass", tonnage);

   BGString engineStr;
   engineStr.assignInt (engineRating);
   switch (EngineType) {
      case ENGINE_TYPE_LARGE: engineStr += "L"; break;
      case ENGINE_TYPE_XL: engineStr += "XL"; break;
      case ENGINE_TYPE_XXL: engineStr += "XXL"; break;
      case ENGINE_TYPE_LXL: engineStr += "LXL"; break;
      case ENGINE_TYPE_LXXL: engineStr += "LXXL"; break;
      case ENGINE_TYPE_COMPACT: engineStr += "C"; break;
   }
   MechVMMechTreeIter.addAttribute ("engine", engineStr.getChars());

   if (HeatSinkType == IS_Heat_Sink)  {
      MechVMMechTreeIter.addAttribute ("heatSinks", heatSinks);
   } else
      MechVMMechTreeIter.addAttribute ("doubleHeatSinks", heatSinks);
   MechVMMechTreeIter.addAttribute ("name", getName());

   // Section stats: armor, internal structure
   for (size_t i = 0; i < Biped_Mech_Sections; i++) {
      size_t sectionChild = MechVMMechTreeIter.addChild(BipedMech::getSectionName(i));
      MechVMMechTreeIter.goToChild(sectionChild);

      if (i == Biped_Mech_Section_Center_Torso 
      ||  i == Biped_Mech_Section_Left_Torso
      ||  i == Biped_Mech_Section_Right_Torso) {
         MechVMMechTreeIter.addAttribute ("af", FrontArmor[i]);
         MechVMMechTreeIter.addAttribute ("ar", RearArmor[i]);
      } else
         MechVMMechTreeIter.addAttribute ("a", FrontArmor[i]);
      MechVMMechTreeIter.addAttribute ("is", InternalStructure[i]);
      int criticalCount = BipedMech::getCriticalCount (i);

      for (int j = 0; j < criticalCount; j++) {
         int crit = criticals[i][j];
         // Report each critical type at most once
         bool found = false;
         for (int k = 0; k < j; k++) 
            if (criticals[i][k] == crit)
               found = true;
         if (!found && crit > 0) {
            // Count number of occurences for this critical type
            int count = 1;
            for (int k = j+1; k < criticalCount; k++) 
               if (criticals[i][k] == crit)
                  count++;

            const MWWeapons* w = getWeaponStats(crit);
            if (w != NULL) {
               BGString str;
               if (count == w->Criticals) {
                  str = w->getShortName();
               } else if (count % w->Criticals == 0) {
                  str.appendInt (count / w->Criticals);
                  str += " ";
                  str += w->getShortName();
               } else {
                  str.appendInt (count);
                  str += " criticals of ";
                  str += w->getShortName();
               }
               MechVMMechTreeIter.addText(str.getChars());
            }
         }
      }

      MechVMMechTreeIter.goToParent();
   }

   MechVMMechTreeIter.goToParent();

   return false;
}

////////////////////////////////////////////////////////////////////////////////
// Calculate actual mech weight

int BipedMech::countArmorPoints() const
{
   int armorPoints = RearArmor[Biped_Mech_Section_Center_Torso] +
                     RearArmor[Biped_Mech_Section_Left_Torso] +
                     RearArmor[Biped_Mech_Section_Right_Torso];
                
   for (int i = 0; i < 8; i++) 
      armorPoints += FrontArmor[i];

   return armorPoints;
}

double BipedMech::calcInternalStructureWeight() const
{
   if (InternalStructureType == Unused_Critical_Slot)
      return tonnage / 10.0;
   else
      return tonnage / 20.0;
}

double BipedMech::calcJJWeight () 
{
   jumpJets = countWeaponInAllSections(CE_Jump_Jet);

   if (tonnage > 85)
      return 2*jumpJets;
   else if (tonnage > 55)
      return jumpJets;
   else
      return jumpJets / 2.0;
}

double BipedMech::calcRealWeight()
{
   // Add other equipment
   double equipmentTonnage = 0;
   for (int i = 0; i < Biped_Mech_Sections; i++) {
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
// Obtain mech information

int BipedMech::getInteger (int info) const
{
   return Vehicle::getInteger(info);
}

bool BipedMech::setInteger (int info, int newVal)
{
   return Vehicle::setInteger(info, newVal);
}

////////////////////////////////////////////////////////////////////////////////
// Add weapon to mech section

bool BipedMech::addWeapon(int currSection, int weaponNo)
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
// Remove weapon

int BipedMech::removeWeapon(int currSection, int critNo, int weaponNo, int critCount)
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

int BipedMech::checkAndFixEngineCrits(int section)
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

////////////////////////////////////////////////////////////////////////////////
// Check - and try to correct - config problems 
// Returns 0 on success, and the number of a warning for the tr() function otherwise

int BipedMech::check()
{
   // Overweight?
   if (realWeight > tonnage)
      return 26;

   // XL engine criticals in side torsi?
   if (getInteger(Info_EngineType) == ENGINE_TYPE_XL) {
      int engRes = checkAndFixEngineCrits(Biped_Mech_Section_Left_Torso);
      if (engRes > 0)
         return engRes;

      engRes = checkAndFixEngineCrits(Biped_Mech_Section_Right_Torso);
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

   // No errors found
   return 0;
}

////////////////////////////////////////////////////////////////////////////////

DWORD BipedMech::getArmor(int section, int ArmorType) const
{
   if (section < Biped_Mech_Sections) {
      if (ArmorType == Vehicle::Armor_Front)
         return FrontArmor[section];
      else if (ArmorType == Vehicle::Armor_Rear)
         return RearArmor[section];
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////////////

/*bool BipedMech::setFrontArmor(int section, int newVal)
{
   if (section > 7)
      return false;
   if (section == Biped_Mech_Section_Head && newVal < 10
   ||  newVal + RearArmor[section] <= 2* InternalStructure[section])
   {
      FrontArmor[section] = newVal;
      return true;
   } else
      return false;

   bool maySet = false;
   if (section == Biped_Mech_Section_Center_Torso
   ||  section ==  Biped_Mech_Section_Left_Torso
   ||  section ==  Biped_Mech_Section_Right_Torso)
   {
      if (newVal + RearArmor[section] <= 2* InternalStructure[section])
         maySet = true;
   } else if (section == Biped_Mech_Section_Head) {
      if (newVal < 10)
         maySet = true;
   } else {
      if (newVal <= 2* InternalStructure[section])
         maySet = true;
   }
}

bool BipedMech::setRearArmor(int section, int newVal)
{
   if (section >= Biped_Mech_Section_Center_Torso 
   &&  section <= Biped_Mech_Section_Right_Torso
   &&  FrontArmor[section] + newVal <= 2*InternalStructure[section])
   {
      RearArmor[section] = newVal;
      return true;
   } else
      return false;
}*/

bool BipedMech::setArmor(int section, DWORD newVal, int ArmorType)
{ 
   bool maySet = false;

   if (ArmorType == Armor_Front) {
      if (section == Biped_Mech_Section_Center_Torso
      ||  section ==  Biped_Mech_Section_Left_Torso
      ||  section ==  Biped_Mech_Section_Right_Torso)
      {
         if ((int) newVal + RearArmor[section] <= 2* InternalStructure[section])
            maySet = true;
      } else if (section == Biped_Mech_Section_Head) {
         if (newVal < 10)
            maySet = true;
      } else {
         if ((int) newVal <= 2* InternalStructure[section])
            maySet = true;
      }
   } else {
      if (section == Biped_Mech_Section_Center_Torso
      ||  section ==  Biped_Mech_Section_Left_Torso
      ||  section ==  Biped_Mech_Section_Right_Torso)
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

void BipedMech::setInternalStructure (int section, DWORD newVal)
{
   if (section < Biped_Mech_Sections) {
      InternalStructure[section] = newVal;
   }
}

void BipedMech::setCriticals (int section, int slot, int weaponType)
{
   if (section < Biped_Mech_Sections) {
      criticals[section][slot] = weaponType;
   }
}
