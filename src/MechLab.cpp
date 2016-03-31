////////////////////////////////////////////////////////////////////////////////
// MechLab.cpp
// Mech Configuration window for MechVM
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#include "MechLab.h"

#include "dialogs.h"
#include "MechVM.h"
#include "Mesh2.h"
#include "Config.h"
#include "GLCheckBox.h"

#include "GL/glu.h"

const double FOVY = 60.0;

////////////////////////////////////////////////////////////////////////////////

void MechList::addMech (const BGString& name, const BGString& xml, 
   const BGString& mek, const BGString& textFN)
{
   MechNames.push_back (name);
   MechXML.push_back (xml);
   MechCFG.push_back (mek);
   MechText.push_back (textFN);
}

////////////////////////////////////////////////////////////////////////////////

void MechList::addMVMK (const TCHAR* mechDir, const TCHAR* MVMKfile)
{
   BGFilePath<TCHAR> textureFN;
   BGFilePath<TCHAR> textureMask = mechDir;
   textureMask += "*.BMP";
   FileList textList;
   if (textList.start(textureMask.getChars())) {
      textureFN = mechDir;
      textureFN += textList.getFileName();
   }

   BGString mechName;
   BGFilePath<TCHAR> cfgFN = mechDir;
   XMLTree xmlTree;
   xmlTree.loadFromFile(MVMKfile);
   XMLTreeConstIterator* iter = xmlTree.getConstIterator();
   iter->goToChild(0);
   for (size_t i = 0; i < iter->getChildCount(); ++i) {
      iter->goToChild(i);
      BGString nodeName = iter->getNodeName();
      if (nodeName.equals("config")) {
         cfgFN += iter->getAttributeValueByName("file");
         mechName = iter->getAttributeValueByName("mechName");
      }
      iter->goToParent();
   }

   if (!cfgFN.equals(mechDir)) {
      addMech (mechName, MVMKfile, cfgFN, textureFN);
   } else {
      // MVMK does not mention a config, use all configs in that directory
      BGFilePath<TCHAR> mask2 = mechDir;
      mask2 += "*.mek"; // must be lower case for Linux
      FileList mekList;
      if (mekList.start (mask2.getChars())) {
         do {
            cfgFN = mechDir; // must be reset
            cfgFN += mekList.getFileName();
            mechName.copyPart(mekList.getFileName(), 0, strlen(mekList.getFileName())-4);

            addMech (mechName, MVMKfile, cfgFN, textureFN);
         } while (++mekList);
      }
   }

   delete iter;
}

////////////////////////////////////////////////////////////////////////////////
// Load mechs from XML file

void MechList::loadMechs (const TCHAR* mechDir)
{
   // Search all subdirectories for mech definitions
   BGFilePath<TCHAR> searchPath, searchMask; 
   if (isRelativePath (mechDir)) {
      searchPath = getDataDir ();
   }
   searchPath += mechDir;
   searchMask = searchPath;
   searchMask += "*";
   printf ("Searching %s for mechs...\n", searchPath.getChars());

   FileList list;
   if (list.start(searchMask.getChars())) {
      do {
         BGFilePath<TCHAR> mechDir = searchPath;
         mechDir += list.getFileName();

         BGFilePath<TCHAR> mask = mechDir;
         mask += "*.mvmk"; // must be lower case for Linux
         FileList MVMKlist;
         bool found = MVMKlist.start (mask.getChars());

         while (found) {
            BGFilePath<TCHAR> MVMKfile = mechDir.getChars();
            MVMKfile += MVMKlist.getFileName();
            addMVMK (mechDir.getChars(), MVMKfile.getChars());
            found = ++MVMKlist;
         }
      } while (++list);
   }

   printf ("Found %i mechs \n", MechNames.size());
}

inline const char* MechList::getMechName(size_t index) const
{
   if (index < MechNames.size())
      return MechNames[index].getChars();
   else 
      return NULL;
}

inline const char* MechList::getMVMKFN(size_t index) const
{
   if (index < MechXML.size())
      return MechXML[index].getChars();
   else 
      return NULL;
}

inline const char* MechList::getTextureFN(size_t index) const
{
   if (index < MechText.size())
      return MechText[index].getChars();
   else 
      return NULL;
}

const char* MechList::getConfigFN(size_t index) const
{
   if (index < MechCFG.size())
      return MechCFG[index].getChars();
   else 
      return NULL;
}

Vehicle* MechList::loadMech(size_t index)
{
   if (MechXML.size() == 0) {
      return NULL;
   }

   // Load mech geometry from XML description
   size_t lastPathSep;
   BGString partPath = MechXML[index];
   if (partPath.findChar(OSPathSep, partPath.getLength()-1, 0, -1, lastPathSep)) {
      partPath.setLength (lastPathSep);
   }

   Vehicle* vehicle =
      loadVehicle(MechNames[index].getChars(), MechXML[index].getChars(), 
                  partPath.getChars(), //MechCFG[index].getChars(), 
                  MechText[index].getChars());

   if (vehicle != NULL)
      vehicle->loadConfig(MechCFG[index].getChars());

   return vehicle;
}

////////////////////////////////////////////////////////////////////////////////
// Constructor

MechLab::MechLab(int buttons, GLWindow* returnWin)
: m_returnWin (returnWin),
  vehicle(NULL),
  m_buttons (buttons),
  importGeometryButton (NULL),
  assignPoliesButton (NULL),
  createTextureButton (NULL),
  animateMechButton (NULL), 
  repairMechButton (NULL), 
  configureMechButton (NULL),
  equipmentButton (NULL), 
  weaponGroupsButton (NULL), 
  exitButton (NULL),
  leftButtonDown (false), 
  rightButtonDown (false),
  renderMech (true),
  LimitedStock (true),
  changed (false),
  currSection (0),
  mechTable (NULL), 
  weaponsTable (NULL),
  sectionTable (NULL),
  sectionSelectionTable (NULL)
{
   Config* config = getConfig();
   const TCHAR* path = config->getMechPath();
   mechList.loadMechs (path);
   weaponsLocker = new WeaponsLocker();

   initDialog();
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

MechLab::~MechLab()
{
   delete weaponsLocker;
   delete vehicle;
}

////////////////////////////////////////////////////////////////////////////////

void MechLab::initDialog()
{
   clear();

   if ((m_buttons & MechLab_selectChassisButtonID) != 0) {
      importGeometryButton = addToolbarButton(tr(20), MechLab_selectChassisButtonID);
   }
   if ((m_buttons & MechLab_animateMechButtonID) != 0) {
      animateMechButton = addToolbarButton(tr(21), MechLab_animateMechButtonID);
   }
   if ((m_buttons & MechLab_outfitMechButtonID) != 0) {
      animateMechButton = addToolbarButton(tr(22), MechLab_outfitMechButtonID);
   }
   if ((m_buttons & MechLab_equipmentButtonID) != 0) {
      equipmentButton = addToolbarButton(tr(23), MechLab_equipmentButtonID);
   }
   if ((m_buttons & MechLab_weaponGroupsID) != 0) {
      weaponGroupsButton = addToolbarButton(tr(44), MechLab_weaponGroupsID);
   }
   if ((m_buttons & MechLab_exitButtonID) != 0) {
      exitButton = addToolbarButton(tr(24), MechLab_exitButtonID);
   }

   label = new GLLabel (this, 80, 80, 600, 20);
   label->setTitle ("");
   add (label);

   weaponsTable = NULL;
   mechTable = NULL;
   renderMech = true;
}

const int buttonTop = 20;
const int buttonHeight = 30;
const int buttonLeft = 20;
const int buttonWidth = 150;
const int buttonDist = 20;

GLButton* MechLab::addToolbarButton(const TCHAR* buttonName, int ID)
{
   size_t x = buttonDist + (buttonWidth+buttonDist) * windows.size();
   GLButton* newButton = new GLButton (this, (int) x, buttonTop, buttonWidth, 
                                       buttonHeight);
   newButton->setTitle(buttonName);
   newButton->setBackgroundColor (0.5, 0.5, 0.5);
   newButton->userData = ID;
   newButton->setListener (this);
   add (newButton);
   return newButton;
}

////////////////////////////////////////////////////////////////////////////////

void MechLab::addContextButton(const TCHAR* bname, int ID, int BaseID)
{
   size_t y = buttonDist + (buttonHeight+buttonDist) * (ID+1);
   GLButton* newButton = new GLButton (this, buttonDist, (int) y, buttonWidth, 
                                       buttonHeight);
   newButton->setTitle(bname);
   newButton->setBackgroundColor (0.5, 0.5, 0.5);
   newButton->userData = BaseID+ID;
   newButton->setListener (this);
   add (newButton);
}

/*void MechLab::showMechPartControls(int BaseID)
{
   addContextButton("Hip", Mech_Hip, BaseID);
   addContextButton("Torso", Mech_Torso, BaseID);
   addContextButton("Left Arm", Mech_LeftArm, BaseID);
   addContextButton("Right Arm", Mech_RightArm, BaseID);
   addContextButton("Left Upper Leg", Mech_LeftUpperLeg, BaseID);
   addContextButton("Left Lower Leg", Mech_LeftLowerLeg, BaseID);
   addContextButton("Left Foot", Mech_LeftFoot, BaseID);
   addContextButton("Right Upper Leg", Mech_RightUpperLeg, BaseID);
   addContextButton("Right Lower Leg", Mech_RightLowerLeg, BaseID);
   addContextButton("Right Foot", Mech_RightFoot, BaseID);
}*/

////////////////////////////////////////////////////////////////////////////////

void MechLab::addWeapons(const BGString& description)
{
   if (description.equals("GBL"))
      weaponsLocker->addClanWeapons(100, 100, 100, 100, 100, 100, 100, true);
   else if (description.equals("GBL_UW"))
      weaponsLocker->addGBLUnderwaterWeapons(20, 50);
   else if (description.equals("31stcc"))
      weaponsLocker->addClanWeapons(100, 100, 100, 100, 100, 100, 100, false);
   else if (description.equals("all"))
      weaponsLocker->addWeaponsToStock (IS_Energy_Weapon_Base, Clan_MASC, 55);
   else  if (description.equals("nmc1"))
      weaponsLocker->addClanWeapons(100, 0, 0, 0, 0, 0, 100, false);
   else  if (description.equals("nmc4"))
      weaponsLocker->addClanWeapons(100, 2, 2, 100, 2, 2, 100, false);
   else  if (description.equals("nmc5"))
      weaponsLocker->addClanWeapons(100, 100, 100, 100, 4, 12, 100, false);
   else
      weaponsLocker->addISLevel1Weapons(20, 50);
}

////////////////////////////////////////////////////////////////////////////////

void MechLab::showChassisTable()
{
   mechTable = new GLTableView (this, 20, 100, 250, height-120, 
                                MechLab_selectChassisButtonID);
   mechTable->setDataSource(this);
   mechTable->setColWidth(1, 50, 150);
   mechTable->setBackgroundColor (1, 1, 1);
   mechTable->userData = MechLab_MechsTableID;
   mechTable->setListener (this);
   mechTable->setRowHeights(getMechCount(), 20);
   add (mechTable);
}

////////////////////////////////////////////////////////////////////////////////

//void MechLab::showMech(size_t num)
void MechLab::showMech(Vehicle* newVehicle)
{
   if (vehicle != NULL)
      delete vehicle;
   vehicle = newVehicle;

   // Set display parameters
   viewDist = 30;
   meshCenter = Point3D(0, 0, 0);
   eye = Point3D (0, 0, viewDist);
   center = Point3D (0, 0, 0);
   xDir = Point3D (1, 0, 0);
   yDir = Point3D (0, 1, 0);
   zDir = Point3D (0, 0, 1);
   meshSize = 10;
}

////////////////////////////////////////////////////////////////////////////////

void MechLab::showMechTonnage()
{
   vehicle->calcRealWeight();
   BGString msg = vehicle->getName();
   msg += ": ";
   msg.appendDouble(vehicle->getWeight(), 2);
   msg += "t/";
   msg.appendInt (vehicle->getInteger(Vehicle::Info_Tonnage));
   msg += "t";

   // Add warning?
   int msgNum = vehicle->check();

   // Add overweight warning?
   if (msgNum > 0)
      msg += tr (msgNum);

   label->setTitle (msg.getChars());
   label->setLeft ((getWidth()-textWidth (msg.getChars()))/2);
}

////////////////////////////////////////////////////////////////////////////////

void MechLab::showAnimControls()
{
   if (vehicle == NULL) {
      label->setTitle (tr(25));
      return;
   }

   GLLineEdit* edit;
   edit = (GLLineEdit*) createControls (MechLab_Anim_ULegAngle1ID, 0, "Upper Leg Angle");
   edit->setDouble(vehicle->getAnimParam(0, BipedAnimUpperLegAngle));

   edit = (GLLineEdit*) createControls (MechLab_Anim_LLegAngle1ID, 0, "Lower Leg Angle");
   edit->setDouble(vehicle->getAnimParam(0, BipedAnimLowerLegAngle));

   edit = (GLLineEdit*) createControls (MechLab_Anim_FootAngle1ID, 0, "Foot Angle");
   edit->setDouble(vehicle->getAnimParam(0, BipedAnimFootAngle));

   edit = (GLLineEdit*) createControls (MechLab_Anim_ULegAngle2ID, 0, "Upper Leg Angle");
   edit->setDouble(vehicle->getAnimParam(1, BipedAnimUpperLegAngle));

   edit = (GLLineEdit*) createControls (MechLab_Anim_LLegAngle2ID, 0, "Lower Leg Angle");
   edit->setDouble(vehicle->getAnimParam(1, BipedAnimLowerLegAngle));

   edit = (GLLineEdit*) createControls (MechLab_Anim_FootAngle2ID, 0, "Foot Angle");
   edit->setDouble(vehicle->getAnimParam(1, BipedAnimFootAngle));

   createControls (MechLab_Anim_ParallelFeetID, 0, "Feet parallel to ground");
   //edit->setDouble(mech->getAnimParam(0, BipedAnimUpperLegAngle);
}

///////////////////////////////////////////////////////////////////////////////

void MechLab::showOutfitMechControls()
{
   if (vehicle == NULL) {
      label->setTitle (tr(25));
      return;
   }

   const int margin = 10;
   int top = buttonTop + buttonHeight + margin;
   sectionSelectionTable = new GLTableView (this, margin, top, 200, 180, MechLab_SectionSelectionTable);
   sectionSelectionTable->setColWidth(3, 50, 100);
   sectionSelectionTable->setColWidth(0, 250, 100);
   sectionSelectionTable->setRowHeights(8, 20);
   sectionSelectionTable->setBackgroundColor (1, 1, 1);
   sectionSelectionTable->userData = MechLab_SectionSelectionTable;
   sectionSelectionTable->setDataSource(this);
   sectionSelectionTable->setListener (this);
   sectionSelectionTable->setSelectedRow (currSection);
   add (sectionSelectionTable);

   top = sectionSelectionTable->getTop() + sectionSelectionTable->getHeight() + margin;
   sectionTable = new GLTableView (this, margin, top, 200, 250, MechLab_SectionID);
   sectionTable->setColWidth(3, 50, 100);
   sectionTable->setColWidth(0, 250, 100);
   sectionTable->setRowHeights(12, 20);
   sectionTable->setBackgroundColor (1, 1, 1);
   sectionTable->userData = MechLab_SectionID;
   sectionTable->setDataSource(this);
   sectionTable->setListener (this);
   add (sectionTable);

   top = sectionTable->getTop() + sectionTable->getHeight() + margin;
   weaponsTable = new GLTableView (this, margin, top, width-2*margin, 
                                   height-top-margin, MechLab_WeaponsTableID);
   weaponsTable->setColWidth(6, 50, 100);
   weaponsTable->setColWidth(0, 250, 100);
   weaponsTable->setRowHeights(WeaponsCount, 20);
   weaponsTable->setBackgroundColor (1, 1, 1);
   weaponsTable->userData = MechLab_WeaponsTableID;
   weaponsTable->setDataSource(this);
   weaponsTable->setListener (this);
   add (weaponsTable);

   showMechTonnage();
}

///////////////////////////////////////////////////////////////////////////////

int y = 100;
int labelW = 200;
int lineH = 20;
int marginX = 20;
int marginY = 10;
int sliderW = 400;
int editWidth = 250;

///////////////////////////////////////////////////////////////////////////////
// Create a pair of a label and another control
// type: 0=LineEdit, 1=Slider, 2=ComboBox

const int labelHeight = 20;
const int controlHeight = 30;
const int margin = 10;

GLWindow* MechLab::createControls (int ID, int type, const char* name)
{
   int num = (int) getWindowCount() / 2-2;
   int controlLeft, controlTop, groupHeight;
   GLLabel* label = NULL;
   GLWindow* result = NULL;

   /*if (getConfig()->broadDisplay()) {
      controlLeft = 220;
      groupHeight = controlHeight + margin;
      controlTop = num*groupHeight;
   } else {*/
      controlLeft = margin;
      groupHeight = labelHeight + controlHeight + 2*margin;
      controlTop = num*groupHeight+labelHeight+margin+100;
   //}

   label = new GLLabel (this, margin, num*groupHeight+100, 200, labelHeight);
   switch (type) {
      case 0: // LineEdit
         result = new GLLineEdit (this, controlLeft, controlTop, 
                                  200, controlHeight);
         break;
      case 1: // Slider
         result = new GLSlider (this, controlLeft, controlTop, 
                                200, controlHeight);
         break;
      case 2: // ComboBox
         result = new GLComboBox (this, controlLeft, controlTop, 
                                  200, controlHeight);
         break;
   }

   add (label);
   label->setTitle (name);
   label->textColor = Color (0, 0, 0);
   add (result);
   result->setListener (this);
   result->userData = ID;

   return result;
}

void MechLab::addCheckbox (const char* labelTitle, int& y, bool initialState, int ID)
{
   GLCheckBox* cb = new GLCheckBox (this, marginX, y, width, lineH);
   cb->setTitle(labelTitle);
   cb->userData = ID;
   cb->setChecked(initialState);
   cb->setListener(this);
   add (cb);

   y += lineH + marginY;
}

GLSlider* MechLab::addSlider (const char* labelTitle, int min, int pos, int max, int& y,
                              int ID)
{
   GLLabel* label = new GLLabel (this, marginX, y, labelW, lineH);
   label->setTitle(labelTitle);
   add (label);

   GLSlider* slider = new GLSlider (this, labelW + 2*marginX, y, sliderW, lineH);
   slider->setMinMax(min, max);
   slider->setValue(max-min);
   slider->setDialCount(max-min+1);
   slider->setValue(pos);
   slider->userData = ID;
   slider->setListener(this);
   add (slider);

   /*GLLineEdit * edit = new GLLineEdit (this, labelW + 3*marginX + sliderW, y,
                                       editWidth, lineH);
   edit->setInt(pos);
   add (edit);*/

   label =  new GLLabel (this, labelW + 3*marginX + sliderW, y,
                               editWidth, lineH);
   label->setInt (pos);
   label->userData = ID+1;
   add (label);

   y += lineH + marginY;
   return slider;
}

GLSlider* MechLab::addArmorSlider (int section, bool front, int& y)
{
   int pos, ID;
   int min = 0;
   int max = 2*vehicle->getInternalStructure(section);
   BGString title = vehicle->getSectionName(section);

   if (front) {
      pos = vehicle->getArmor(section, Vehicle::Armor_Front);
      ID = MechLab_Equipment_ArmorFront+2*section;
   } else {
      title += " (rear)";
      pos = vehicle->getArmor(section, Vehicle::Armor_Rear);
      ID = MechLab_Equipment_ArmorRear+2*section;
   }
   return addSlider (title.getChars(), min, pos, max, y, ID);
}

GLComboBox* MechLab::addComboBox (const char* labelTitle, const char* cbTitle, int& y,
                                  int ID)
{
   GLLabel* label = new GLLabel (this, marginX, y, labelW, lineH);
   label->setTitle(labelTitle);
   add (label);

   // ComboBox: Engine Type
   GLComboBox* cb = new GLComboBox (this, labelW + 2*marginX, y, sliderW, lineH);
   cb->setText(cbTitle);
   cb->userData = ID;
   cb->setListener(this);
   add (cb);

   label =  new GLLabel (this,  labelW + 3*marginX + sliderW, y,
                                editWidth, lineH);
   label->userData = ID+1;
   add (label);
   y += lineH + marginY;

   return cb;
}

///////////////////////////////////////////////////////////////////////////////

void MechLab::showEquipmentControls()
{
   if (vehicle == NULL) {
      label->setTitle (tr(25));
      return;
   }

   int y = 100;
   renderMech = false;

   int engineRating = vehicle->getInteger (Vehicle::Info_EngineRating);
   GLSlider* engineSlider = addSlider ("Engine Rating", 10, engineRating, 500, y,
                                       MechLab_Equipment_EngineRatingID);
   engineSlider->setDialCount(101);

   const char* engineTypeStr = "Standard";
   if (vehicle->getInteger(Vehicle::Info_EngineType) != ENGINE_TYPE_STD) 
      engineTypeStr = "XL";
   GLComboBox* cb = addComboBox ("Engine Type", engineTypeStr, y,
                                 MechLab_Equipment_EngineTypeID);
   cb->addChoice("Standard");
   cb->addChoice("XL");

   int heatSinks = vehicle->getInteger(Vehicle::Info_Heat_Sinks);
   addSlider ("Heat Sinks", 0, heatSinks, 51, y, MechLab_Equipment_HeatSinkCount);

   int hsType = vehicle->getInteger (Vehicle::Info_Heat_Sink_Type);
   const char* text = "Single";
   if (hsType != IS_Heat_Sink)
      text = "Double";
   cb = addComboBox ("Heat Sink Type", text, y, MechLab_Equipment_HeatSinkType);
   cb->addChoice("Single");
   cb->addChoice("Double");

   text = "Standard";
   if (vehicle->getInteger(Vehicle::Info_Internal_Structure_Type) != Unused_Critical_Slot)
      text = "Endo Steel";
   cb = addComboBox ("Internal Structure", text, y, 
                     MechLab_Equipment_InternalStructureType);
   cb->addChoice("Standard");
   cb->addChoice("Endo Steel");

   text = "Standard";
   if (vehicle->getInteger(Vehicle::Info_Armor_Type) != ARMOR_TYPE_STD)
      text = "Ferro Fibrous";
   cb = addComboBox ("Armor Type", text, y, MechLab_Equipment_ArmorType);
   cb->addChoice("Standard");
   cb->addChoice("Ferro Fibrous");

   // Lower arm and hand activators
   if (dynamic_cast <BipedMech*> (vehicle) != NULL) {
      bool hasActuator = vehicle->countWeaponCriticals (Biped_Mech_Section_Left_Arm, 
                                                        CE_Lower_Arm_Actuator) > 0;
      addCheckbox ("Left Lower Arm Actuator", y, hasActuator, 
                   MechLab_Equipment_LeftLowerArmActuator);

      hasActuator = vehicle->countWeaponCriticals (Biped_Mech_Section_Left_Arm, 
                                                   CE_Hand) > 0;
      addCheckbox ("Left Hand Actuator", y, hasActuator, 
                   MechLab_Equipment_LeftHandActuator);

      hasActuator = vehicle->countWeaponCriticals (Biped_Mech_Section_Right_Arm, 
                                                   CE_Lower_Arm_Actuator) > 0;
      addCheckbox ("Right Lower Arm Actuator", y, hasActuator, 
                   MechLab_Equipment_RightLowerArmActuator);

      hasActuator = vehicle->countWeaponCriticals (Biped_Mech_Section_Right_Arm, 
                                                   CE_Hand) > 0;
      addCheckbox ("Right Hand Actuator", y, hasActuator, 
                   MechLab_Equipment_RightHandActuator);
   }

   // Armor
   GLSlider* slider = addArmorSlider (Biped_Mech_Section_Head, true, y);
   if (slider != NULL) {
      slider->setMinMax(0, 9);
      slider->setDialCount (10);
      slider->setValue(vehicle->getArmor(0, Vehicle::Armor_Front));
   }

   addArmorSlider (Biped_Mech_Section_Center_Torso, true, y);
   addArmorSlider (Biped_Mech_Section_Center_Torso, false, y);

   CenterTorsoArmorSumLabel = 
      new GLLabel (this, (int) (1.15*labelW + 3*marginX + sliderW),
                   y - 2*lineH - marginY/2, editWidth, lineH);
   CenterTorsoArmorSumLabel->setTitle("30/30");
   add (CenterTorsoArmorSumLabel);

   addArmorSlider (Biped_Mech_Section_Left_Torso, true, y);
   addArmorSlider (Biped_Mech_Section_Left_Torso, false, y);

   LeftTorsoArmorSumLabel = 
      new GLLabel (this, (int) (1.15*labelW + 3*marginX + sliderW), 
                   y - 2*lineH - marginY/2, editWidth, lineH);
   LeftTorsoArmorSumLabel->setTitle("30/30");
   add (LeftTorsoArmorSumLabel);

   addArmorSlider (Biped_Mech_Section_Right_Torso, true, y);
   addArmorSlider (Biped_Mech_Section_Right_Torso, false, y);

   RightTorsoArmorSumLabel = 
      new GLLabel (this, (int) (1.15*labelW + 3*marginX + sliderW), 
                   y - 2*lineH - marginY/2, editWidth, lineH);
   RightTorsoArmorSumLabel->setTitle("30/30");
   add (RightTorsoArmorSumLabel);

   addArmorSlider (Biped_Mech_Section_Left_Arm, true, y);
   addArmorSlider (Biped_Mech_Section_Right_Arm, true, y);
   addArmorSlider (Biped_Mech_Section_Left_Leg, true, y);
   addArmorSlider (Biped_Mech_Section_Right_Leg, true, y);

   updateEquipmentLabels();
}

///////////////////////////////////////////////////////////////////////////////

void MechLab::updateEquipmentLabels()
{
   // Engine rating label
   int engineRating = vehicle->getInteger(Vehicle::Info_EngineRating);
   int tonnage = vehicle->getInteger(Vehicle::Info_Tonnage);
   int engineType = vehicle->getInteger (Vehicle::Info_EngineType);
   int techType = vehicle->getInteger(Vehicle::Info_TechType);

   BGString text;
   text.assignInt(engineRating);
   if (engineType != ENGINE_TYPE_STD)
      text += "XL (";
   else
      text += " (";
   text.appendDouble (getEngineWeight(engineRating, engineType), 2);
   text += " t), ";
   text.appendDouble (1.5*10.8*engineRating/tonnage, 1);
   text += "km/h";
   GLLabel* label = findLabel (MechLab_Equipment_EngineRatingID+1);
   label->setTitle(text.getChars());

   // Heat sink count label
   int heatSinkCount = vehicle->getInteger(Vehicle::Info_Heat_Sinks);
   text.assignInt(heatSinkCount);
   int heatSinkType = vehicle->getInteger(Vehicle::Info_Heat_Sink_Type);
   if (heatSinkType != IS_Heat_Sink) {
      text += "(";
      text.appendInt(2*heatSinkCount);
      text += "), ";
   } else
      text += ", ";

   if (heatSinkCount > 10) {
      text.appendInt (heatSinkCount-10);
      text += " t, ";
   } else
      text += " 0t, ";

   int HeatSinksInEngine = engineRating / 25;
   if (heatSinkCount > HeatSinksInEngine) {
      if (heatSinkType == IS_Heat_Sink)
         text.appendInt (heatSinkCount-HeatSinksInEngine);
      else if (techType == Biped_Mech_Tech_Type_Clan) {
         text.appendInt (2*(heatSinkCount-HeatSinksInEngine));
      } else
         text.appendInt (3*(heatSinkCount-HeatSinksInEngine));
      text += " criticals";
   } else
      text += "0 criticals";

   label = findLabel (MechLab_Equipment_HeatSinkCount+1);
   label->setTitle(text.getChars());

   // Internal structure label
   text.assignDouble(vehicle->calcInternalStructureWeight());
   text += " t";
   label = findLabel (MechLab_Equipment_InternalStructureType+1);
   label->setTitle(text.getChars());

   // Armor total
   //int ArmorType = vehicle->getInteger(Vehicle::Info_Armor_Type);
   int armorPoints = vehicle->countArmorPoints();
   text.assignInt (armorPoints);
   text += " points, ";
   text.appendDouble(vehicle->calcArmorWeight());
   text += " t";
   label = findLabel (MechLab_Equipment_ArmorType+1);
   label->setTitle(text.getChars());

   // Front armor labels
   for (int i = 0; i < 8; i++) {
      int armorPoints = vehicle->getArmor(i, Vehicle::Armor_Front);
      text.assignInt(armorPoints);
      if (i == 0) {
         text += "/9";
      } else if (i > 3) {
         text += "/";
         text.appendInt(2*vehicle->getInternalStructure(i));
      }
      GLLabel* label = findLabel (MechLab_Equipment_ArmorFront+2*i+1);
      label->setTitle(text.getChars());
   }

   // Rear armor labels
   for (int i = 1; i < 4; i++) {
      int armorPoints = vehicle->getArmor(i, Vehicle::Armor_Rear);
      text.assignInt(armorPoints);
      GLLabel* label = findLabel (MechLab_Equipment_ArmorRear+2*i+1);
      label->setTitle(text.getChars());
   }

   // Summed up armor
   text.assignInt(vehicle->getArmor(1, Vehicle::Armor_Front) + 
      vehicle->getArmor(1, Vehicle::Armor_Rear));
   text += "/";
   text.appendInt(2*vehicle->getInternalStructure(1));
   CenterTorsoArmorSumLabel->setTitle (text.getChars());

   text.assignInt(vehicle->getArmor(2, Vehicle::Armor_Front) + 
                  vehicle->getArmor(2, Vehicle::Armor_Rear));
   text += "/";
   text.appendInt(2*vehicle->getInternalStructure(2));
   LeftTorsoArmorSumLabel->setTitle (text.getChars());

   text.assignInt(vehicle->getArmor(3, Vehicle::Armor_Front) + 
                  vehicle->getArmor(3, Vehicle::Armor_Rear));
   text += "/";
   text.appendInt(2*vehicle->getInternalStructure(3));
   RightTorsoArmorSumLabel->setTitle (text.getChars());

   showMechTonnage();
}

///////////////////////////////////////////////////////////////////////////////

void MechLab::showWeaponGroups()
{
   GLLabel* label = NULL;
   label = new GLLabel (this, margin, 107, 200, labelHeight);
   label->setTitle(tr(45));
   add (label);

   GLLineEdit * edit = new GLLineEdit (this, margin + 200, 100, 
                                       300, controlHeight);
   edit->userData = MechLab_CommentID;

   if (vehicle != NULL)
      edit->setText (vehicle->getComment());
   edit->setListener (this);
   add(edit);
   //edit->setDouble(vehicle->getAnimParam(1, BipedAnimFootAngle));

   if (MEKformat > 0) {
      for (size_t i = 0; i < vehicle->getWeaponListEntryCount(); ++i) {
         const char* weaponName = vehicle->getWeaponListEntryName(i);
         int y = 107 +(i+1) * (labelHeight + margin);
         GLLabel* label = new GLLabel (this, margin, y, 
                                       200, labelHeight);
         label->setTitle(weaponName);
         add (label);

         for (size_t j = 0; j < MaxGroupCount; ++j) {
            BGString str;
            if (!vehicle->getInGroup(i, j))
               str.assignInt(j+1);
            else
               str = "+";
            GLButton* button = new GLButton (this, 200+margin+30*j, y, 30, controlHeight);
            button->setTitle(str.getChars());
            button->userData = WeaponGroupAssignerID + 5*i + j;
            button->setListener(this);
            add(button);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Receive messages

GLLabel* MechLab::findLabel (int ID)
{
   for (size_t i = 0; i < windows.size(); i++) {
      GLWindow* win = windows[i];
      if (win->userData == ID) {
         GLLabel* label = dynamic_cast <GLLabel*> (win);
         if (label != NULL)
            return label;
      }
   }

   return false;
}

void MechLab::receiveMessage (VirtualClass* sender, int MessageType)
{
   GLWindow* win = dynamic_cast <GLWindow*> (sender);
   size_t row, weapon;
   MechVM* mechvm = NULL;
   bool reallyExit = true;

   if (win == NULL)
      return;

   GLSlider* slider = dynamic_cast <GLSlider*> (win);
   GLComboBox* cb = dynamic_cast <GLComboBox*> (win);
   GLLineEdit* edit = dynamic_cast <GLLineEdit*> (sender);

   switch (win->userData) {
      case MechLab_selectChassisButtonID:
         initDialog ();
         showChassisTable();
         break;
      case MechLab_animateMechButtonID:
         initDialog ();
         showAnimControls();
         break;
      case MechLab_outfitMechButtonID:
         initDialog ();
         showOutfitMechControls();
         break;
      case MechLab_equipmentButtonID:
         initDialog ();
         showEquipmentControls();
         break;
      case MechLab_weaponGroupsID:
         initDialog();
         showWeaponGroups();
         break;
      case MechLab_exitButtonID:
         if (vehicle != NULL && changed && MEKFN.getLength() != 0) {
            int checkResult = vehicle->check();
            if (checkResult == 0) {
               if (messageBox("Save changes?", MEKFN.getChars(), MB_YESNO) == IDYES) {
                  vehicle->saveToMW2MEK(MEKFN.getChars(), MEKformat);
                  MechShell* shell = dynamic_cast<MechShell*> (m_returnWin);
                  if (shell != NULL) {
                     shell->useConfig(MEKFN.getChars());
                  }
                  mechList.addMech (vehicle->getName(), vehicle->MVMKFN.get(), 
                     MEKFN, vehicle->textureFN.get());
               }
            } else
               if (messageBox("Mech Config Invalid", "Really Exit?", MB_YESNO) == IDNO) {
                  reallyExit = false;
               }
         }

         if (reallyExit) {
            setCurVis (m_returnWin);
            mechvm = dynamic_cast <MechVM*> (m_returnWin);
            if (mechvm != NULL) {
               mechvm->clear();
               mechvm->createToolbar();
            }
            //delete this; // todo: causes heap corruption
         }
         break;
      case MechLab_MechsTableID:
         row = mechTable->getSelectedRow();
         showMech(row);         
         break;
      case MechLab_WeaponsTableID:
         row = weaponsTable->getSelectedRow();
         weapon = weaponsLocker->getWeaponNo(row);
         if (vehicle->userMayAddDirectly(currSection, weapon)) {
            if (vehicle->addWeapon(currSection, weapon)) {
               weaponsLocker->removeWeapon (weapon, 1);
               changed = true;
            }
         }
         showMechTonnage();
         break;
      case MechLab_SectionID:
         {
            row = sectionTable->getSelectedRow();
            int weaponNo = vehicle->getCriticals(currSection, row);
            if (vehicle->userMayRemoveDirectly(weaponNo)) {
               vehicle->removeWeapon(currSection, row, weaponNo);
               weaponsLocker->addWeaponToStock(weaponNo, 1);
               changed = true;
            }
            showMechTonnage();
         }
         break;
      case MechLab_SectionSelectionTable:
         currSection = sectionSelectionTable->getSelectedRow();
         break;
      case MechLab_Anim_ULegAngle1ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (0, BipedAnimUpperLegAngle, val);
         }
         break;
      case MechLab_Anim_LLegAngle1ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (0, BipedAnimLowerLegAngle, val);
         }
         break;
      case MechLab_Anim_FootAngle1ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (0, BipedAnimFootAngle, val);
         }
         break;
      case MechLab_Anim_ULegAngle2ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (1, BipedAnimUpperLegAngle, val);
         }
         break;
      case MechLab_Anim_LLegAngle2ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (1, BipedAnimLowerLegAngle, val);
         }
         break;
      case MechLab_Anim_FootAngle2ID:
         if (edit != NULL) {
            double val;
            if (edit->getDouble(val))
               vehicle->setAnimParam (1, BipedAnimFootAngle, val);
         }
         break;
      /*case MechLab_Anim_ParallelFeetID:
         {
            GLLineEdit* edit = dynamic_cast <GLLineEdit*> (sender);
            if (edit != NULL) {
               double val;
               if (edit->getDouble(val))
                  mech->setAnimParam (1, BipedAnimFootAngle, val);
            }
         }
         break;*/
      case MechLab_Equipment_EngineRatingID:
         if (slider != NULL) {
            changed = true;
            int newVal = (int) slider->getValue();
            vehicle->setInteger (Vehicle::Info_EngineRating, newVal);
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_EngineTypeID:
         if (cb != NULL) {
            changed = true;
            if (strcmp (cb->getText(), "XL") == 0) {
               // updateEquipmentLabels calls showMechTonnage, which checks
               // continually whether the needed crits can be added
               vehicle->setInteger(Vehicle::Info_EngineType, ENGINE_TYPE_XL);
            } else {
               vehicle->setInteger(Vehicle::Info_EngineType, ENGINE_TYPE_STD);
               vehicle->removeWeapon(Biped_Mech_Section_Left_Torso, 0, CE_Engine, 3);
               vehicle->removeWeapon(Biped_Mech_Section_Right_Torso, 0, CE_Engine, 3);
            }
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_HeatSinkCount:
         if (slider != NULL) {
            changed = true;
            int targetVal = (int) slider->getValue();
            int hsType = vehicle->getInteger(Vehicle::Info_Heat_Sink_Type);
            const MWWeapons* stats = getWeaponStats (hsType);
            int engineRating = vehicle->getInteger(Vehicle::Info_EngineRating);
            int IntegralHeatSinks = engineRating / 25;

            int currentlyUsedHSCrits = 0;
            int heatSinksThatUseCrits = 0;
            for (size_t i = 0; i < vehicle->getSectionCount(); ++i) {
               currentlyUsedHSCrits += vehicle->countWeaponCriticals (i, hsType);
            }
            if (stats != NULL)
               heatSinksThatUseCrits = currentlyUsedHSCrits / stats->Criticals;

            size_t section = 0;
            if (targetVal > IntegralHeatSinks + heatSinksThatUseCrits) {
               // Increase number of heat sinks
               while (section < vehicle->getSectionCount() 
               &&     heatSinksThatUseCrits + IntegralHeatSinks < targetVal)
               {
                  if (vehicle->addWeapon(section, hsType)) {
                     ++heatSinksThatUseCrits;
                  } else {
                     ++section;
                  }
               }
            } else {
               // Reduce number of heat sinks
               while (section < vehicle->getSectionCount() 
               &&     heatSinksThatUseCrits + IntegralHeatSinks > targetVal)
               {
                  if (vehicle->removeWeapon(section, 0, hsType, stats->Criticals) > 0) {
                     --heatSinksThatUseCrits;
                  } else {
                     ++section;
                  }
               }

               if (targetVal < IntegralHeatSinks)
                  IntegralHeatSinks = targetVal;
            }
            int resultingHSCount = IntegralHeatSinks + heatSinksThatUseCrits;
            vehicle->setInteger (Vehicle::Info_Heat_Sinks, resultingHSCount);
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_HeatSinkType:
         if (cb != NULL) {
            changed = true;
            int oldHSType = vehicle->getInteger(Vehicle::Info_Heat_Sink_Type);
            int newHSType = IS_Heat_Sink;
            int techType = vehicle->getInteger(Vehicle::Info_TechType);
            if (strcmp (cb->getText(), "Single") != 0) {
               if (techType != Biped_Mech_Tech_Type_Clan) {
                  newHSType = IS_Double_Heat_Sink;
               } else {
                  newHSType = Clan_Double_Heat_Sink;
               }
            }
            int hsCrits = getWeaponStats(newHSType)->Criticals;
            vehicle->removeAll(oldHSType);
            vehicle->setInteger(Vehicle::Info_Heat_Sink_Type, newHSType);
            int integralHS = vehicle->getInteger(Vehicle::Info_EngineRating) / 25;
            int HScount = vehicle->getInteger(Vehicle::Info_Heat_Sinks);
            if (HScount > integralHS) {
               vehicle->autoAddCriticals (newHSType, HScount-integralHS, hsCrits);
            }
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_InternalStructureType:
         if (cb != NULL) {
            changed = true;
            int oldISType = vehicle->getInteger(Vehicle::Info_Internal_Structure_Type);
            int isType = Unused_Critical_Slot; // Standard structure
            int techType = vehicle->getInteger(Vehicle::Info_TechType);

            if (strcmp (cb->getText(), "Standard") != 0) {
               isType = IS_Endo_Steel;
               int isCrits = 14;
               if (techType == Biped_Mech_Tech_Type_Clan) {
                  isType = Clan_Endo_Steel_Internals;
                  isCrits = 7;
               }
               int autoAdded = vehicle->autoAddCriticals (isType, isCrits, 1);
               if (autoAdded < isCrits)
                  weaponsLocker->addWeaponToStock(isType, isCrits-autoAdded);
            } else {
               vehicle->removeAll(oldISType);
               weaponsLocker->removeWeapon(oldISType, 50);
            }
            vehicle->setInteger(Vehicle::Info_Internal_Structure_Type, isType);
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_ArmorType:
         if (cb != NULL) {
            changed = true;
            int armorType = ARMOR_TYPE_STD;
            int armorCritCount = 0;
            int techType = vehicle->getInteger(Vehicle::Info_TechType);
            if (strcmp (cb->getText(), "Standard") != 0) {
               armorType = ARMOR_TYPE_FERRO_FIBROUS;
               if (techType == Biped_Mech_Tech_Type_Clan)
                  armorCritCount = 7;
               else 
                  armorCritCount = 14;
            }

            int armorCritType = IS_Ferro_Fibrous;
            if (techType == Biped_Mech_Tech_Type_Clan)
               armorCritType = Clan_Ferro_Fibrous;

            if (armorType == ARMOR_TYPE_STD) {
               vehicle->removeAll (armorCritType);
               weaponsLocker->removeWeapon(armorCritType, 50);
            } else {
               int autoAdded = vehicle->autoAddCriticals (armorCritType, armorCritCount, 1);
               if (autoAdded < armorCritCount)
                  weaponsLocker->addWeaponToStock(armorCritType, armorCritCount-autoAdded);
            }
            vehicle->setInteger(Vehicle::Info_Armor_Type, armorType);
            updateEquipmentLabels();
         }
         break;
      case MechLab_Equipment_LeftLowerArmActuator:
         changed = true;
         if (MessageType == CheckBoxChecked) {
            if (!vehicle->addWeapon(Biped_Mech_Section_Left_Arm, CE_Lower_Arm_Actuator)) {
               GLCheckBox* cb = dynamic_cast <GLCheckBox*> (sender);
               cb->setChecked(false);
               label->setTitle(tr(28));
            }
         } else {
            vehicle->removeWeapon(Biped_Mech_Section_Left_Arm, 0, CE_Lower_Arm_Actuator, 1);
         }
         break;
      case MechLab_Equipment_LeftHandActuator:
         changed = true;
         if (MessageType == CheckBoxChecked) {
            if (!vehicle->addWeapon(Biped_Mech_Section_Left_Arm, CE_Hand)) {
               GLCheckBox* cb = dynamic_cast <GLCheckBox*> (sender);
               cb->setChecked(false);
               label->setTitle(tr(28));
            }
         } else {
            vehicle->removeWeapon(Biped_Mech_Section_Left_Arm, 0, CE_Hand, 1);
         }
         break;
      case MechLab_Equipment_RightLowerArmActuator:
         changed = true;
         if (MessageType == CheckBoxChecked) {
            if (!vehicle->addWeapon(Biped_Mech_Section_Right_Arm, CE_Lower_Arm_Actuator)) {
               GLCheckBox* cb = dynamic_cast <GLCheckBox*> (sender);
               cb->setChecked(false);
               label->setTitle(tr(28));
            }
         } else {
            vehicle->removeWeapon(Biped_Mech_Section_Right_Arm, 0, CE_Lower_Arm_Actuator, 1);
         }
         break;
      case MechLab_Equipment_RightHandActuator:
         changed = true;
         if (MessageType == CheckBoxChecked) {
            if (!vehicle->addWeapon(Biped_Mech_Section_Right_Arm, CE_Hand)) {
               GLCheckBox* cb = dynamic_cast <GLCheckBox*> (sender);
               cb->setChecked(false);
               label->setTitle(tr(28));
            }
         } else {
            vehicle->removeWeapon(Biped_Mech_Section_Right_Arm, 0, CE_Hand, 1);
         }
         break;
      case MechLab_CommentID:
         if (edit != NULL) {
            changed = true;
            vehicle->setComment (edit->getText());
         }
         break;
      default:
         if (win->userData < MechLab_Equipment_ArmorRear) {
            changed = true;
            int section = (win->userData - MechLab_Equipment_ArmorFront) / 2;
            int newVal = (int) slider->getValue();
            vehicle->setArmor(section, newVal, Vehicle::Armor_Front);
            updateEquipmentLabels();
         } else if (win->userData < MechLab_Equipment_ArmorRear+8) {
            int section = (win->userData - MechLab_Equipment_ArmorRear) / 2;
            int newVal = (int) slider->getValue();
            vehicle->setArmor(section, newVal, Vehicle::Armor_Rear);
            changed = true;
            updateEquipmentLabels();
         } else if (win->userData >= WeaponGroupAssignerID) {
            int weaponNo = (win->userData - WeaponGroupAssignerID) / 5;
            int groupNo = win->userData % 5;
            vehicle->setInGroup(weaponNo, groupNo, true);
            showWeaponGroups();
         }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Render

void MechLab::render(int x, int y, int w, int h)
{
   glClearColor (0.7f, 0.7f, 0.7f, 1.0f);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_NORMALIZE);

   glViewport(0, 0, w, h);

   DWORD ticks = GetTickCount();

   // Render mesh
   if (vehicle != NULL && renderMech) {
      // Set viewing projection
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      gluPerspective (FOVY, ((double) w) / ((double) h),
                      0.1*viewDist, 
                      100*viewDist);
      glMatrixMode (GL_MODELVIEW);

      // Render
      glColor3d(1, 1, 1);
      glLoadIdentity();

      //double angle = 2 * M_PI * (ticks%2000)/2000.0;
      //eye = Point3D (viewDist*sin(angle), 0, viewDist*cos(angle));
      //center = Point3D (0, 0, 0);
      //up = Point3D (0, 1, 0);
      gluLookAt (eye.x, eye.y, eye.z, center.x, center.y, center.z, 
                 yDir.x, yDir.y, yDir.z);

      //gluLookAt (0, 0, viewDist,
      //           meshCenter.x, meshCenter.y, meshCenter.z, 
      //           0, 1, 0);
      //glRotated (360*(GetTickCount()%2000)/2000.0, 0, 1, 0);

      // Activate light source
      glEnable (GL_LIGHTING);
      glEnable (GL_COLOR_MATERIAL);

      GLfloat pos[4] = {0.5f, 1.0f, -1.0f, 0.0f};
      GLfloat color[3] = {0.5f, 0.5f, 0.5f};
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

      // Render selected point
      double d = abs((int) (ticks%200-100))/100.0;
      glColor3d (d, d, d);
      glPointSize (5.0);
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_TEXTURE_2D);
      glBegin(GL_POINTS);
         selPoint.glVertex();
      glEnd();
      glColor3d (1, 1, 1);

      // Render!
      glEnable (GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);

      //glScaled(0.01, 0.01, 0.01);

      vehicle->setMatrices();
      vehicle->render();
      glDisable (GL_LIGHT0);
      glDisable (GL_LIGHTING);
   }

   GLWindowContainer::render (x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////
// Get ray direction for mouse coordinates

Point3D MechLab::getMouseRay (double x, double y)
{
   //double dx = 2.0 - 4.0 * x;
   //double dy = 2.0 - 4.0 * y;
   x = -x;
   y = -y;
   Point3D zDir = center - eye;
   zDir.unitLength();
   Point3D xDir = yDir.crossMultiply(zDir);
   double tanFOVY = viewDist * tan (FOVY * M_PI / 360.0f);
   return viewDist * zDir + (tanFOVY * x * xDir) + (tanFOVY * y * yDir);
}

////////////////////////////////////////////////////////////////////////////////
// Select geometry using a mouse ray

MeshPolygon* MechLab::mouseRaySelection (int x, int y)
{
   if(vehicle != NULL) {
      double dx = 4*((double) (x-(width/2))) / ((double) height);
      double dy = 4*((double) (y-(height/2))) / ((double) height);
      selPoint = getMouseRay (dx, dy);

      MeshPolygon* nearestPoly = NULL;
      double minDistSoFar = 500 * meshSize;
      for (size_t i = 0; i < Mech_MAX_PARTS; i++)
         TraverseScene (eye, selPoint, vehicle->getObject(i), minDistSoFar, nearestPoly);

      return nearestPoly;
   } else
      return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// If the ray given by origin, dir intersects the Triangle, this function
// returns the distance of the intersection, otherwise, it returns -1

void MechLab::testProximity (const Triangle& T, const Point3D& origin, 
   const Point3D& dir, MeshPolygon* poly, double& minDistSoFar,
   MeshPolygon*& nearestPolySoFar)
{
   Point3D intersection;
   Ray ray (origin, dir);
   int testResult = ray.intersects (T, intersection);
   double dist = (intersection - origin).length();

/*   if (testResult == ObjectsIntersect) {
      assert (parent != NULL);
      parent->setMinDistIntersection (intersection);
   }*/

   if (testResult == ObjectsIntersect && dist < minDistSoFar) {
      minDistSoFar = dist;
      nearestPolySoFar = poly;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Test if the ray given by origin, dir intersects ro or one of its childs,
// at a nearer distance than minDistSoFar. If it does, set minDistSoFar,
// and set new selected object in plab
// Returns the nearest intersected object

void MechLab::TraverseScene (const Point3D& origin, 
   const Point3D& dir, RenderableObject* ro, double& minDistSoFar,
   MeshPolygon*& nearestPolySoFar)
{
   // Test if ro is ComplexObject or Mesh
   RenderableObject* co = dynamic_cast <RenderableObject*> (ro);
   Ray ray (origin, origin + dir);
   if (co != NULL) {
      // Traverse co's children with co's matrix
      TraversalStack.multiplyMatrix (co->getMatrix());
      for (unsigned int i = 0; i < co->getObjectCount(); i++) { 
         RenderableObject* child = co->getObject (i);
         TraverseScene (origin, dir, child, minDistSoFar, nearestPolySoFar);
      }
      TraversalStack.pop ();
   }

   Mesh* mesh = dynamic_cast <Mesh*> (ro);
   if (mesh != NULL) {
      // Test the ray given by origin, dir against the polygons in mesh
      unsigned int i = 0; 
      while (i < mesh->getPolygonCount()) {
         MeshPolygon* poly = mesh->getPolygon(i);
         if (poly->getPointCount() == 3) {
            size_t pi0 = poly->getPoint (0);
            size_t pi1 = poly->getPoint (1);
            size_t pi2 = poly->getPoint (2);
            Point3D p0, p1, p2;
            mesh->getPoint(pi0, p0);
            mesh->getPoint(pi1, p1);
            mesh->getPoint(pi2, p2);

            // Apply the current matrix to the mesh's polygons
            Point3D pt0 = TraversalStack.top().Transform (p0);
            Point3D pt1 = TraversalStack.top().Transform (p1);
            Point3D pt2 = TraversalStack.top().Transform (p2);

            Triangle T (pt0, pt1, pt2);
            testProximity (T, origin, dir, poly, minDistSoFar, nearestPolySoFar);
         } else if (poly->getPointCount() == 4) {
            size_t pi0 = poly->getPoint (0);
            size_t pi1 = poly->getPoint (1);
            size_t pi2 = poly->getPoint (2);
            size_t pi3 = poly->getPoint (3);
            Point3D p0, p1, p2, p3;
            if (mesh->getPoint(pi0, p0) 
            && mesh->getPoint(pi1, p1)
            && mesh->getPoint(pi2, p2)
            && mesh->getPoint(pi3, p3))
            {
               // Apply the current matrix to the mesh's polygon.
               Point3D pt0 = TraversalStack.top().Transform (p0);
               Point3D pt1 = TraversalStack.top().Transform (p1);
               Point3D pt2 = TraversalStack.top().Transform (p2);
               Point3D pt3 = TraversalStack.top().Transform (p3);
               Triangle T1 (pt0, pt1, pt2);
               Triangle T2 (pt0, pt2, pt3);
               testProximity (T1, origin, dir, poly, minDistSoFar, nearestPolySoFar);
               testProximity (T2, origin, dir, poly, minDistSoFar, nearestPolySoFar);
            }
         } else {
            // For polygons with more than 3 vertices, form triangles from one fixed
            // vertex and all its non-adjacent edges.
            printf ("Cannot pick polygons with more than 4 vertices yet\n");
         }
         i++;
      }
   }
}

////////////////////////////////////////////////////////////////////////////////
// Assign polygon to a limb

void MechLab::reassign (MeshPolygon* /*poly*/)
{
   //delete poly;
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard event handler

void MechLab::keypressed(char /*key*/, bool /*shiftHeld*/, bool /*controlHeld*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Button state changed over a window: returns child that was clicked

GLWindow* MechLab::buttonStateChanged (int button, bool down, int x, int y)
{ 
   lastMouseX = x;
   lastMouseY = y;
   GLWindow* win = GLWindowContainer::buttonStateChanged(button, down, x, y);
   if (win != NULL) {
      return win;
   } else {
      if (button == 1) {
         leftButtonDown = down;
      } else if (button == 3)
         rightButtonDown = down;
   }
   return NULL;
}

void MechLab::mouseMoved (int x, int y)
{
   if (leftButtonDown && rightButtonDown) {
      // Translate view
      //center.x += ((double)(lastMouseX-x)*meshSize)/width;
      //center.y += ((double)(y-lastMouseY)*meshSize)/height;

      // Zoom
      viewDist += ((double)(y-lastMouseY)*meshSize)/height;
      eye = zDir*viewDist;
   } else if (leftButtonDown) {
      // Select geometry
      MeshPolygon* poly = mouseRaySelection (x, y);
      if (poly != NULL)
         reassign(poly);
   } else if (rightButtonDown) {
      // Rotate view
      double dx = ((double) (x-lastMouseX)) / ((double) width);
      double dy = ((double) (y-lastMouseY)) / ((double) height);
      Point3D axis = dx * yDir + dy * xDir;
      if (axis.length () != 0.0) {
         axis.unitLength ();
         double rotSpeed = 2.0 * M_PI * sqrt (dx*dx+dy*dy);
         Matrix m = RotationMatrix (rotSpeed, axis.x, axis.y, axis.z);
         assert (axis.length () != 0.0);
         xDir = m.Transform (xDir);
         yDir = m.Transform (yDir);
         zDir = m.Transform (zDir);
         xDir.unitLength ();
         yDir.unitLength ();
         zDir.unitLength ();
         eye = zDir*viewDist;
      }
   }
   lastMouseX = x;
   lastMouseY = y;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation of GLTableViewDataSource

bool MechLab::getString (size_t senderID, int row, int col, BGString& str)
{
   switch (senderID) {
      case MechLab_selectChassisButtonID:
         if ((size_t)row < getMechCount() && row >= 0) {
            str = mechList.getMechName(row);
         }
         break;
      case MechLab_WeaponsTableID:
         if (row == 0) {
            switch (col) {
               case 0: str = "Name"; break;
               case 1: str = "Criticals"; break;
               case 2: str = "Tonnage"; break;
               case 3: str = "Heat"; break;
               case 4: str = "Damage"; break;
               case 5: str = "Range"; break;
               case 6: str = "In Stock"; break;
            }
         } else if (row-1 < (int) WeaponsCount) {
            //int weaponNo = row;
            int weaponNo = weaponsLocker->getWeaponNo(row);
            const MWWeapons* stats = getWeaponStats (weaponNo);
            switch (col) {
               case 0: str = stats->longName; break;
               case 1: str.assignInt (stats->Criticals); break;
               case 2: str.assignDouble (stats->TonnageTimes100 * 0.01); break;
               case 3: str.assignDouble (stats->Heat); break;
               case 4: str.assignDouble (stats->MaxDamage); break;
               case 5: str.assignDouble (stats->MaxRangeInMeters); break;
               case 6: str.assignInt (weaponsLocker->getWeaponCount(row));
            }
         }
         return true;
      case MechLab_SectionID:
         {
            int weaponNum = vehicle->getCriticals(currSection, row);
            const MWWeapons* weapon = getWeaponStats (weaponNum);
            if (weapon != NULL) 
               str = weapon->shortName;
         }
         return true;
      case MechLab_SectionSelectionTable:
         str = vehicle->getSectionName (row);
         return true;
   }
   return false;
}

void MechLab::getDimensions (size_t senderID, size_t& rowCount, size_t& colCount)
{
   switch (senderID) {
      case MechLab_selectChassisButtonID:
         rowCount = getMechCount(); 
         colCount = 1;
         break;
      case MechLab_WeaponsTableID:
         //rowCount = WeaponsCount;
         rowCount = weaponsLocker->getWeaponCount();
         if (LimitedStock)
            colCount = 7;
         else 
            colCount = 6;
         break;
      case MechLab_SectionID:
         rowCount = vehicle->getCriticalCount (currSection);
         colCount = 1;
         break;
      case MechLab_SectionSelectionTable:
         rowCount = vehicle->getSectionCount(); 
         colCount = 1;
         break;
   }
}

////////////////////////////////////////////////////////////////////////////////
// WeaponsLocker implementation

// Find weapon - returns index if found, or -1 otherwise
int WeaponsLocker::findWeapon (int weaponNo)
{
   for (size_t i = 0; i < weaponsInStock.getSize(); i += 2) {
      if (weaponsInStock[i] == weaponNo)
         return i;
   }

   return false;
}

// Add weapons to locker
void WeaponsLocker::addWeaponToStock (int weaponNo, int count)
{
   if (count == 0)
      return;

   // Check if weapon is already in list
   bool found = false, stop = false;
   size_t i = 0;

   while (!found && 2*i < weaponsInStock.getSize()) {
      if (weaponsInStock[2*i] == weaponNo) {
         found = true;
         weaponsInStock[2*i+1] += count;
      } else {
         ++i;
      }
   }

   if (!found) {
      weaponsInStock.add (weaponNo);
      weaponsInStock.add (count);
   }
}

void WeaponsLocker::addWeaponsToStock (int weaponStartNo, int weaponEndNo, int count)
{
   if (count > 0)
      for (int i = weaponStartNo; i <= weaponEndNo; ++i)
         addWeaponToStock (i, count);
}

// Remove weapons from locker
bool WeaponsLocker::removeWeapon (int weaponNo, int count)
{
   // Check if weapon is already in list
   //bool stop = false;
   size_t i = 0;

   while (i < weaponsInStock.getSize()) {
      if (weaponsInStock[i] == weaponNo) {
         int newCount = weaponsInStock[i+1] - count;
         if (newCount <= 0) {
            while (i+2 < weaponsInStock.getSize()) {
               weaponsInStock[i] = weaponsInStock[i+2];
               i += 2;
            }
            weaponsInStock.setSize(weaponsInStock.getSize()-2);
            return true;
         } else { //if (newCount > 0) {
            weaponsInStock[i+1] = newCount;
            return true;
         }
      } else {
         i += 2;
      }
   }

   return false;
}

void WeaponsLocker::addISLevel1Weapons(size_t weaponCount, size_t ammoCount)
{
   addWeaponsToStock (IS1_Flamer, IS1_PPC, weaponCount);
   addWeaponsToStock (IS1_Autocannon_20, IS1_Autocannon_2, weaponCount);
   addWeaponToStock (IS1_Autocannon_Ammo, ammoCount);
   addWeaponToStock (IS1_Machine_Gun, weaponCount);
   addWeaponToStock (IS1_Machine_Gun_Ammo, ammoCount);
   addWeaponsToStock (IS1_SRM_2, IS1_LRM_20, weaponCount);
   addWeaponsToStock (IS1_SRM_2_Ammo, IS1_SRM_6_Ammo, ammoCount);
   addWeaponsToStock (IS1_LRM_5_Ammo, IS1_LRM_20_Ammo, ammoCount);
   addWeaponToStock (IS_Heat_Sink, 100);
}

void WeaponsLocker::addClanWeapons(size_t energyWeaponCount, 
   size_t ballisticWeaponCount, size_t ballisticAmmoCount,
   size_t GaussAndMachineGuns,
   size_t missileWeaponCount, size_t missileAmmoCount,
   size_t equipmentCount, bool GBL)
{
   addWeaponsToStock (Clan_ER_Large_Laser, Clan_ER_Small_Laser, energyWeaponCount);
   addWeaponsToStock (Clan_ER_PPC, Clan_Small_Pulse_Laser, energyWeaponCount);
   addWeaponsToStock (Clan_LB_20_X_AC, Clan_LB_2_X_AC, ballisticWeaponCount);
   addWeaponsToStock (Clan_LB_20_X_AC_Ammo, Clan_LB_2_X_AC_Ammo, ballisticAmmoCount);
   addWeaponsToStock (Clan_UAC_20, Clan_UAC_2, ballisticWeaponCount);
   addWeaponsToStock (Clan_UAC_20_Ammo, Clan_UAC_2_Ammo, ballisticAmmoCount);

   if (GBL) {
      addWeaponToStock (Clan_Anti_Missile_System, ballisticWeaponCount);
      addWeaponToStock (Clan_Anti_Missile_System_Ammo, ballisticAmmoCount);
   }

   addWeaponToStock (Clan_Gauss, GaussAndMachineGuns);
   addWeaponToStock (Clan_Gauss_Ammo, GaussAndMachineGuns);
   addWeaponToStock (Clan_Machine_Gun, GaussAndMachineGuns);
   addWeaponToStock (Clan_Machine_Gun_Ammo, GaussAndMachineGuns);

   addWeaponsToStock (Clan_LRM_5, Clan_LRM_20, missileWeaponCount);
   addWeaponsToStock (Clan_LRM_5_Ammo, Clan_LRM_20_Ammo, missileAmmoCount);
   addWeaponsToStock (Clan_SRM_2, Clan_SRM_6, missileWeaponCount);
   addWeaponsToStock (Clan_SRM_2_Ammo, Clan_SRM_6_Ammo, missileAmmoCount);
   addWeaponsToStock (Clan_Streak_SRM_2, Clan_Streak_SRM_6, missileWeaponCount);
   addWeaponsToStock (Clan_Streak_SRM_2_Ammo, Clan_Streak_SRM_6_Ammo, missileAmmoCount);

   if (GBL) {
      addWeaponToStock (Clan_NARC_Missile_Beacon, missileWeaponCount);
      addWeaponToStock (Clan_NARC_Missile_Beacon_Ammo, missileAmmoCount);
      addWeaponToStock (Clan_Arrow_IV, missileWeaponCount);
      addWeaponToStock (Clan_Arrow_IV_Ammo, missileAmmoCount);
      addWeaponToStock (Clan_SRM_2_Inferno, missileWeaponCount);
      addWeaponToStock (Clan_SRM_Ammo_Inferno, missileAmmoCount);
      addWeaponToStock (Clan_Streak_SRM_2_Inferno, missileWeaponCount);
      addWeaponToStock (Clan_SSRM_Inferno_Ammo, missileAmmoCount);
   }

   addWeaponToStock (CE_Jump_Jet, equipmentCount);
   addWeaponToStock (Clan_Double_Heat_Sink, equipmentCount);
}

void WeaponsLocker::addGBLUnderwaterWeapons(size_t weaponCount, size_t ammoCount)
{
   addWeaponsToStock (Clan_ER_Large_Laser, Clan_ER_Small_Laser, weaponCount);
   addWeaponsToStock (Clan_ER_PPC, Clan_Small_Pulse_Laser, weaponCount);
   addWeaponToStock (Clan_SRT_4, weaponCount);
   addWeaponToStock (Clan_SRT_Ammo, ammoCount);
   addWeaponToStock (Clan_Double_Heat_Sink, 100);
}

////////////////////////////////////////////////////////////////////////////////
// Select file name to store MEK file from given directory

void MechLab::selectMEKFNInDirectory (const TCHAR* dir)
{
   int i = 0;

   do {
      ++i;
      MEKFN = dir;
      MEKFN += OSPathSep;
      MEKFN += "user-";
      MEKFN.appendInt (i);
      MEKFN += ".MEK";
   } while (fileExists(MEKFN.getChars()));
}

void MechLab::selectMEKFNInGameDirectory (const TCHAR* gameDir, const TCHAR* prefix)
{
   int i = 0;

   do {
      MEKFN = gameDir;
      MEKFN += OSPathSep;
      MEKFN += prefix;
      if (i < 10)
         MEKFN += "0";
      MEKFN.appendInt (i);
      MEKFN += "USR.MEK";
      ++i;
   } while (fileExists(MEKFN.getChars()));
}

////////////////////////////////////////////////////////////////////////////////

bool MechLab::showMech(size_t num)
{
   changed = false;
   bool result = false;
   Vehicle * newVehicle = mechList.loadMech(num);

   BGString MVMKFN = mechList.getMVMKFN(num);
   size_t pos, MEKnum = 1;
   
   if (bgstrrscan (MVMKFN.getChars(), OSPathSep, MVMKFN.getLength()-1, 0, pos)) {
      BGString MEKdir;
      //bool found;
      //do {
         MEKdir.copyPart(MVMKFN.getChars(), 0, pos);
         selectMEKFNInDirectory(MEKdir.getChars());
         /*MEKFN += OSPathSep;
         MEKFN += "user-";
         MEKFN.appendInt(MEKnum);
         MEKFN += ".MEK";
         found = fileExists (MEKFN.getChars());
         ++MEKnum;
      } while (found);*/
   }

   if (newVehicle != NULL) {
      newVehicle->MVMKFN.set(mechList.getMVMKFN(num));
      newVehicle->textureFN.set(mechList.getTextureFN(num));
      showMech(newVehicle);
      //result = vehicle->loadConfig (MechCFG[num].getChars());
      showMechTonnage();
   }
   
   return result;
}
