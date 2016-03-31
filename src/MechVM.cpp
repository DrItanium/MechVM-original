///////////////////////////////////////////////////////////////////////////////
// MechVM
// Copyright Bjoern Ganster 2007-2010
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Global constants

int margin = 10;
int InitialWindowWidth = 400;
int InitialWindowHeight = 300;

const int viewFileButtonID = 0;
const int MechLabButtonID = 1;
const int installButtonID = 2;
const int playGameButtonID = 3;
const int upDirButtonID = 4;
const int MW2PRJTableID = 5;
const int MW2PRJTableContextMenu = 8;
const int importPathEditID = 10;
const int startImportButtonID = 11;
const int searchForCDButtonID = 12;
const int gameLauncherTableID = 13;
const int startCampaignID = 14;
const int instantActionID = 15;
const int installFromDriveButtonID = 100;
const int importFromDriveButtonID = 200;
const int reportCDRomButtonID = 300;
const int installNetMechID = 400;
const int installFromCDImageID = 500;

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "Texture.h"
#include "BGString.h"
#include "GLLineEdit.h"
#include "GLSlider.h"
#include "Config.h"
#include "dialogs.h"
#include "FileCache.h"
#include "XMLTree.h"
#include "RenderableObject.h"
#include "GLContextMenu.h"

#include "MWBase.h"
#include "MechVM.h"
#include "MechWarriorIIPRJ.h"
#include "MechWarriorInstallers.h"
#include "MW2MechImporter.h"
#include "MechWarrior3ZBD.h"
#include "Database_MW2.h"
#include "MechSim.h"
#include "MechLab.h"
#include "FramerateCounter.h"
#include "Mut.h"
#include "MechShell.h"

#include "Mesh.h"

#include <GL/glu.h>
//#include <SDL.h>

#include "LZdecode.h"

#ifdef _WIN32
#include <shellApi.h>
#else
//#include "../lib/FramerateCounter.h" // Linux GetTickCount
#include <qfiledialog.h>
#include <qapplication.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Global constants

bool maximized, s_done;
GLWindow* focused;
MechVM* mainWin;
MechSim* sim;
GLWindow* currVis;

///////////////////////////////////////////////////////////////////////////////
// Constructor, destructor

MechVM::MechVM()
: mwArchive (NULL),
  shell (NULL),
  PRJTable (NULL),
  texture (NULL),
  pallette (NULL),
  mesh (NULL)
{
   createToolbar();
}

MechVM::~MechVM()
{
   if (pallette != NULL)
      delete pallette;
}

///////////////////////////////////////////////////////////////////////////////
// Called to set new layout

void MechVM::setGeometry (int /*left*/, int /*top*/, int newWidth, int newHeight)
{
   GLWindow::setGeometry (0, 0, newWidth, newHeight);
   //toolbar->setWidth (newWidth-2*margin);
   //setWidth (newWidth - mainWin->getLeft());
   //setHeight (newHeight - mainWin->getTop());
   //parametersPanel->setHeight (newWidth - margin - parametersPanel->getTop());

   if (PRJTable != NULL) {
      int width = newWidth-margin-PRJTable->getLeft();
      if (width > 250)
         width = 250;
      PRJTable->setGeometry (PRJTable->getLeft(), PRJTable->getTop(),
                             width, newHeight-120);
   }
}

///////////////////////////////////////////////////////////////////////////////
// Create a button

GLButton* MechVM::addButton(int num, const TCHAR* name, int ID)
{
   const int buttonTop = 20;
   const int buttonHeight = 30;
   const int buttonLeft = 20;
   const int buttonWidth = 150;
   const int buttonDist = 20;

   int x = buttonLeft+num*(buttonWidth+buttonDist);
   GLButton* newButton = new GLButton (this, x, buttonTop, buttonWidth, 
                                       buttonHeight);
   newButton->setTitle(name);
   newButton->setBackgroundColor (0.5, 0.5, 0.5);
   newButton->userData = ID;
   newButton->setListener (this);
   add (newButton);
   return newButton;
}

///////////////////////////////////////////////////////////////////////////////
// Called to set new layout

void MechVM::createToolbar()
{
   playGame = addButton(0, tr(0), playGameButtonID);
   MechLabButton = addButton(1, tr(1), MechLabButtonID);
   viewFileButton = addButton(2, tr(2), viewFileButtonID);
   installButton = addButton(3, tr(3), installButtonID);
   label = new GLLabel (this, 300, 50, 600, 30);
   label->textColor = getConfig()->getTextColor();
   add (label);
}

////////////////////////////////////////////////////////////////////////////////
// Misc functions

void displayMsg (const TCHAR* msg)
{
   mainWin->displayMsg (msg);
}

void MechVM::displayMsg (const TCHAR* msg)
{
   printf ("%s\n", msg);
   label->setTitle (msg);
   render(0, 0, mainWin->getWidth(), mainWin->getHeight());
   SDL_GL_SwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////

void MechVM::unpackPrj()
{
   displayMsg("Unpacking...");
   BGString saveFN;
   if (getSaveFileName (saveFN)) {
      unpackArchive (mwArchive, saveFN.getChars());
      displayMsg("Done");
   } else
      displayMsg("Aborted");
}

///////////////////////////////////////////////////////////////////////////////

void openFile (const char* FN)
{
#ifdef _WIN32
   ShellExecute (0, "open", FN, NULL, NULL, SW_SHOWNORMAL);
#else
   printf ("Executing %s is not supported\n", FN);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// TableView implementation: get number of rows and columns

void MechVM::getDimensions (size_t senderID, size_t& rowCount, size_t& colCount)
{
   switch (senderID) {
      case MW2PRJTableID:
         colCount = 3;
         rowCount = mwArchive->getEntryCount();
         break;
      case gameLauncherTableID:
         {
            Config* config = getConfig();
            rowCount = 0;
            colCount = 1;
            for (size_t i = 0; i < config->getKeyCount(); i++) {
               BGString key = config->getKey(i);
               if (key.startsWith ("GameName"))
                  rowCount++;
            }
            break;
         }
   }
}

///////////////////////////////////////////////////////////////////////////////
// TableView implementation: Get string for a cell

bool MechVM::getString (size_t senderID, int row, int col, BGString& str)
{
   switch (senderID) {
      case MW2PRJTableID:
         if ((size_t) row < mwArchive->getEntryCount()) {
            if (col == 0) {
               str.assignInt(row);
            } else if (col == 1) {
               TCHAR entryName[21];
               if (mwArchive->getEntryName (row, entryName, 20)) {
                  str = entryName;
                  return true;
               } else
                  str.clear();
            } else {
               if (!mwArchive->isDirectory(row)) {
                  str.clear();
                  size_t size = mwArchive->getFileSize(row);
                  str.appendSizeStr(size);
               } else
                  str = "Dir";
               return true;
            }
         }
         break;
      case gameLauncherTableID:
         {
            size_t i = 0;
            Config* config = getConfig();
            while (i < config->getKeyCount()) {
               BGString key = config->getKey(i);
               if (key.startsWith ("GameName")) {
                  if (row == 0) {
                     str = config->getValue(i);
                     return true;
                  }
                  row--;
               }
               i++;
            }
         }
         return false;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////

void MechVM::viewMesh (const MemoryBlock* mb, const char* /*fileName*/)
{
   if (mesh != NULL)
      delete mesh;

   mesh = MechWarriorIIPRJ::loadWTB(mb);
   if (mesh != NULL) {
      /*BGString str = "Loaded ";
      str.appendInt ((int) mesh->getPointCount());
      str += " points and ";
      str.appendInt ((int) mesh->getPolygonCount());
      str += " polyons from ";
      str += fileName;
      str +="\n";
      str.print();*/

      mesh->calculateNormals();
      Point3D min = mesh->getMinCoords();
      Point3D max = mesh->getMaxCoords();
      meshCenter = 0.5*(max+min);
      viewDist = 2*(max-min).length();
   }
}

///////////////////////////////////////////////////////////////////////////////

void MechVM::showMech (const MemoryBlock* mb, const char* fileName)
{
   BipedMech* mech = new BipedMech ();
   mech->loadFromMW2MEK(mb);

   // Print mech data
   BGString str = fileName;
   str += " (";
   str.appendDouble(mech->getWeight());
   str += "t/";
   str.appendInt (mech->getMaxWeight());
   str += "t)";

   label->setTitle(str.getChars());

   str.printLn();

   int heatSinks = mech->getInteger (BipedMech::Info_Heat_Sinks);
   int HeatSinkType = mech->getInteger (BipedMech::Info_Heat_Sink_Type);
   if (HeatSinkType == IS_Heat_Sink)
      printf ("Heat Sinks: %i\n", heatSinks);
   else
      printf ("Heat Sinks: %i (%i)\n", heatSinks, 2*heatSinks);

   int walkingHexes = mech->getInteger (BipedMech::Info_WalkingHexes);
   int runningHexes = mech->getInteger (BipedMech::Info_RunningHexes);
   int jumpJets = mech->getInteger (BipedMech::Info_JumpJets);
   printf ("Walking: %i, Running: %i, Jumping: %i\n", walkingHexes, 
           runningHexes, jumpJets);

   int EngineType = mech->getInteger (BipedMech::Info_EngineType);
   //int tonnage = mech->getInteger (BipedMech::Info_Tonnage);
   int EngineRating = mech->getInteger (BipedMech::Info_EngineRating);
   if (EngineType == ENGINE_TYPE_STD)
      printf ("Engine: %i\n", EngineRating);
   else 
      printf ("Engine: %iXL\n", EngineRating);

   int InternalStructureType = mech->getInteger (BipedMech::Info_Internal_Structure_Type);
   if (InternalStructureType != Unused_Critical_Slot)
      printf ("Endo Steel\n");
   int ArmorType = mech->getInteger (BipedMech::Info_Armor_Type);
   if (ArmorType == ARMOR_TYPE_FERRO_FIBROUS)
      printf ("Ferro Fibrous\n");

   printf ("\nSection: <Front Armor> <Rear Armor> <Internal Structure>\n");
   for (int i = 0; i < Biped_Mech_Sections; i++) {
      printf ("%s: %i, %i, %i\n", mech->getSectionName(i), 
              mech->getArmor(i, Vehicle::Armor_Front), 
              mech->getArmor(i, Vehicle::Armor_Rear), mech->getInternalStructure(i));
   }
   printf ("\n");

   // Print criticals
   for (int i = 0; i < Biped_Mech_Sections; i++) {
      printf ("%s:\n", mech->getSectionName(i));
      for (size_t j = 0; j < mech->getCriticalCount (i); j++) {
         int critical = mech->getCriticals(i, j);
         const MWWeapons* w = getWeaponStats(critical);
         if (w != NULL) {
            if (j < 9)
               printf (" ");
            printf ("  %i: %s (%ft)\n", j+1, w->getLongName(), 
                    w->TonnageTimes100/100.0);
            /*if (techType == Biped_Mech_Tech_Type_IS_Level_1)
               printf (" (IS1)");
            else if (techType == Biped_Mech_Tech_Type_IS_Level_2)
               printf (" (IS2)");
            else if (techType == Biped_Mech_Tech_Type_IS_Clan)
               printf (" (Clan)");
            printf (", code: %i\n", ID);*/
         }
      }
   }
   printf ("\n");
   delete mech;
}

#ifdef _WIN32
bool MechVM::playWAV (const MemoryBlock* mb)
{
   // http://msdn2.microsoft.com/en-us/library/ms713269.aspx
   //UINT flags = SND_ASYNC | SND_MEMORY | SND_NODEFAULT; // Not working?
   UINT flags = SND_MEMORY | SND_NODEFAULT;
   if (sndPlaySound(LPCSTR (mb->getPtr(0)), flags))
      return true;
   else
      return false;
}
#else
bool MechVM::playWAV (const MemoryBlock* /*mb*/)
{
   return false;
}
#endif

///////////////////////////////////////////////////////////////////////////////
// View external file

void MechVM::viewFile(const char* fileName, MemoryBlock* block)
{
   if (texture != NULL) {
      delete texture;
      texture = NULL;
   }

   if (mesh != NULL) {
      delete mesh;
      mesh = NULL;
   }

   int FileType = getFileTypeFromExtension(fileName);
   displayMsg ("");

   if (FileType == FT_Unsupported) {
      FileType = getFileTypeFromContent (block);
      if (FileType == FT_Unsupported) {
         displayMsg ("Unsupported file type");
         return;
      }
   }

   printf ("Loading file %s\n", fileName);

   //XMLTree* xmlTree = NULL;
   BGString outFN;

   switch (FileType) {
   case FT_XEL:
      texture = MechWarriorIIPRJ::loadTexture(block->getPtr(), 
                   block->getSize(), pallette);
      break;
   case FT_SHP:
      texture = MechWarriorIIPRJ::loadSHP(*block, 0, pallette);
      break;
   case FT_WTB:
      viewMesh (block, fileName);
      break;
   case FT_Mech:
      showMech (block, fileName);
      break;
   case FT_SFL:
      {
         MemoryBlock* wavBlock = MechWarriorIIPRJ::SFL2Wav(block);
         if (!playWAV (wavBlock)) {
            printf ("Playing %s failed\n", fileName);
         }
         delete wavBlock;
      }
      break;
   case FT_WAV:
      if (!playWAV (block)) {
         printf ("Playing %s failed\n", fileName);
      }
      break;
   case FT_COL:
      if (pallette == NULL)
         pallette = new MemoryBlock ();
      pallette->resize (block->getSize());
      pallette->copy(0, block->getPtr(0), block->getSize());
      for (size_t i = 0; i < pallette->getSize(); i++)
         pallette->setByte(i, 4*pallette->getByte(i));
      break;
   case FT_EFA:
      {
         MemoryBlock* pcx = LZdecode(block);
         texture = MechWarriorIIPRJ::loadPCX(pcx->getPtr(), pcx->getSize());
         delete pcx;
         break;
      }
   case FT_TEX:
      texture = MechWarriorIIPRJ::loadTEX(block->getPtr(), block->getSize());
      break;
   case FT_PCX:
      texture = MechWarriorIIPRJ::loadPCX(block->getPtr(), block->getSize());
      break;
   case FT_MW2_555:
      texture = Decode_MW2_555 (block->getPtr(), block->getSize());
      break;
   case FT_MW3_555:
      texture = Decode_MW3_555 (block->getPtr(), block->getSize());
      break;
/*   case FT_FLT:
      {
         BGString outputPath config->getMechPath();
         outputPath += OSPathSep;
         outputPath += "MW3-";
         MechWarrior3ZBD::exportFLT (block, outputPath.getChars());
         break;
      }*/
   }
}

///////////////////////////////////////////////////////////////////////////////
// View external file

void MechVM::viewExternalFile()
{
   BGString FN;

   if (getOpenFileName(FN)) {

      // Remove old widgets
      clear();
      createToolbar();

      if (mwArchive != NULL) {
         delete mwArchive;
         mwArchive = NULL;
      }

      int fileType = getFileTypeFromExtension(FN.getChars());
      if (!isArchiveType (fileType)) {
         MemoryBlock* block = new MemoryBlock();
         block->loadFromFile(FN.getChars());
         viewFile (FN.getChars(), block);
         delete block;
      } else {
         mwArchive = openArchive (FN.getChars());
         if (mwArchive != NULL) {
            PRJTable = new GLTableView (this, 20, 100, 300, height-120, MW2PRJTableID);
            PRJTable->setDataSource(this);
            PRJTable->setColWidth(2, 50, 50);
            PRJTable->setColWidth(1, 150, 50);
            PRJTable->setRowHeights(mwArchive->getEntryCount(), 20);
            //PRJTable->setRowHeight(mwArchive->getEntryCount()-1, 20, 20);
            PRJTable->setBackgroundColor (1, 1, 1);
            PRJTable->userData = MW2PRJTableID;
            PRJTable->setListener (this);
            add (PRJTable);

            upDirButton = new GLButton(this, 20, 60, 250, 30);
            upDirButton->setTitle ("..");
            upDirButton->userData = upDirButtonID;
            upDirButton->setListener(this);
            add (upDirButton);
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// View file inside a PRJ file

void MechVM::getSelectedMW2PRJFileName (BGString& FN)
{
   char fileName[20];
   size_t fileNum = PRJTable->getSelectedRow();
   mwArchive->getEntryName(fileNum, fileName, 20);
   FN = fileName;
}

MemoryBlock* MechVM::getSelectedMW2PRJFile ()
{
   size_t fileNum = PRJTable->getSelectedRow();
   //size_t offset = mwArchive->getFileOffset (fileNum);
   size_t fileSize = mwArchive->getFileSize(fileNum);

   if (fileSize > mwArchive->getArchiveSize()) {
      printf ("Can't load file\n");
      return NULL;
   }

   MemoryBlock* block = new MemoryBlock(fileSize);
   if (mwArchive->getFileChunk(fileNum, 0, block->getPtr(), fileSize)) {
      return block;
   } else {
      delete block;
      return NULL;
   }
}

///////////////////////////////////////////////////////////////////////////////
// View file inside a PRJ file

void MechVM::ViewPRJFile()
{
   BGString fileName;
   getSelectedMW2PRJFileName (fileName);
   MemoryBlock* block = getSelectedMW2PRJFile ();
   if (block != NULL) {
      //int FileType = getFileTypeFromContent(block);
      viewFile (fileName.getChars(), block);
      delete block;
   }
}

///////////////////////////////////////////////////////////////////////////////
// Request context menu

const int MW2PRJContextMenu_SaveAs = 0;
const int MW2PRJContextMenu_ReplaceWith = 1;
const int MW2PRJContextMenu_AddFiles = 2;
const int MW2PRJContextMenu_SHP2BMP = 3;
const int MW2PRJContextMenu_XEL2BMP = 4;
const int MW2PRJContextMenu_SFL2WAV = 5;
const int MW2PRJContextMenu_WTB2OBJ = 6;
const int MW2PRJContextMenu_BWD2XML = 7;
const int MW2PRJContextMenu_BWD2XML_rec = 8;
const int MW2PRJContextMenu_ImportLevel = 9;
const int MW2PRJContextMenu_TEX2BMP = MW2PRJContextMenu_ImportLevel+1;
const int MW2PRJContextMenu_EFA2PCX = MW2PRJContextMenu_ImportLevel+2;
const int MW2PRJContextMenu_EFA2BMP = MW2PRJContextMenu_ImportLevel+3;
const int MW2PRJContextMenu_OBJ2WTB = MW2PRJContextMenu_ImportLevel+4;
const int MW2PRJContextMenu_unpackAll = MW2PRJContextMenu_ImportLevel+5;

void MechVM::showContextMenu(VirtualClass* sender, int x, int y)
{
   if (sender == PRJTable) {
      BGString fileName;
      getSelectedMW2PRJFileName (fileName);
      int FileType = getFileTypeFromExtension(fileName.getChars());
      glutContextMenu* cm = new glutContextMenu (this, x, y);
      cm->addMenuItem (NULL, "Save as ...", "", this, MW2PRJContextMenu_SaveAs);
      cm->addMenuItem (NULL, "Extract all", "", this, MW2PRJContextMenu_unpackAll);

      switch (FileType) {
      case FT_XEL:
         cm->addMenuItem (NULL, "Export as BMP", "", this, MW2PRJContextMenu_XEL2BMP);
         break;
      case FT_SHP:
         cm->addMenuItem (NULL, "Export as BMP", "", this, MW2PRJContextMenu_SHP2BMP);
         break;
      case FT_WTB:
         cm->addMenuItem (NULL, "Export as OBJ", "", this, MW2PRJContextMenu_WTB2OBJ);
         cm->addMenuItem (NULL, "Replace with OBJ", "", this, MW2PRJContextMenu_OBJ2WTB);
         break;
      case FT_SFL:
         cm->addMenuItem (NULL, "Export as WAV", "", this, MW2PRJContextMenu_SFL2WAV);
         break;
      case FT_BWD:
         cm->addMenuItem (NULL, "Export as XML", "", this, MW2PRJContextMenu_BWD2XML);
         cm->addMenuItem (NULL, "Export as XML (rec.)", "", this, MW2PRJContextMenu_BWD2XML_rec);
         cm->addMenuItem (NULL, "Import Level", "", this, MW2PRJContextMenu_ImportLevel);
         break;
      case FT_TEX:
         cm->addMenuItem (NULL, "Export as BMP", "", this, MW2PRJContextMenu_TEX2BMP);
         break;
      /*case FT_Mech:
         cm->addMenuItem (NULL, "Save As ...", "", this, MW2PRJContextMenu_SaveAs);
         break;
      case FT_COL:
         cm->addMenuItem (NULL, "Save As ...", "", this, MW2PRJContextMenu_SaveAs);
         break;*/
      case FT_EFA:
         cm->addMenuItem (NULL, "Save as PCX", "", this, MW2PRJContextMenu_EFA2PCX);
         cm->addMenuItem (NULL, "Save as BMP", "", this, MW2PRJContextMenu_EFA2BMP);
         break;
      }

      if (mwArchive->replaceFilePossible())
         cm->addMenuItem (NULL, "Replace with ...", "", this, MW2PRJContextMenu_ReplaceWith);
      cm->addMenuItem (NULL, "Add files ...", "", this, MW2PRJContextMenu_AddFiles);
      cm->userData = MW2PRJTableContextMenu;
      add (cm);
   }
}

///////////////////////////////////////////////////////////////////////////////

class GLInstallButton: public GLButton {
public:
   // Constructor
   GLInstallButton (GLWindow* parent, size_t& count, const TCHAR* title, 
                    const BGString& sourcePath, int InstallAlgorithmID)
   : GLButton (parent, 20, 100 + count*40, 830, 30)
   {
      //setListener(parent);
      m_variantName = title;
      BGString newTitle = tr(47);
      newTitle += title;
      newTitle += tr(48);
      newTitle += sourcePath.getChars();
      setTitle (newTitle.getChars());
      m_sourcePath = sourcePath;
      m_InstallAlgorithmID = InstallAlgorithmID;
      parent->add (this);
      setListener (dynamic_cast<MessageListener*> (parent));
      userData = installFromDriveButtonID;
      count++;
      printf ("Install option: %s\n", newTitle.getChars());
   }

   // Destructor
   virtual ~GLInstallButton()
   {}

   // Get data
   const TCHAR* getSourcePath() const
   { return m_sourcePath.getChars(); }
   int getInstallAlgorithmID () const
   { return m_InstallAlgorithmID; }
private:
   BGString m_sourcePath, m_variantName;
   int m_InstallAlgorithmID;
};

///////////////////////////////////////////////////////////////////////////////

/*void MechVM::createInstallButton(size_t& count, char driveLetter)
{
   BGString driveName;
   driveName += driveLetter;
   driveName += ":\\";
   int dt = GetDriveType(driveName.getChars());
   if (dt == DRIVE_CDROM) {
      GLButton* button = new GLButton (this, 20, (int) (60 + count*40), 180, 30);
      button->setTitle(driveName.getChars());
      button->userData = installFromDriveButtonID + driveLetter;
      button->setListener(this);
      add (button);
      count++;
   }
}*/

void MechVM::createInstallButton(size_t& count, const TCHAR* CDpath)
{
   int CDID = getCDType (CDpath);
   GLButton * b1 = NULL, * b2 = NULL;
   switch (CDID) {
         case MW2_31stcc_DOS:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (DOS)", CDpath, CDID);
            break;
         case MW2_31stcc_PE:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (Pentium Edition)", CDpath, CDID);
            break;
         case MW2_31stcc_Hybrid_DOS_Win:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (DOS+Win)", CDpath, CDID);
            break;
         case MW2_NetMech:
            b1 = new GLInstallButton(this, count, "MechWarrior 2: 31stCC (NetMech edition, DOS+Win)", CDpath, CDID);
            break;
         case MW2_GBL_DOS:
            b1 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (DOS)", CDpath, CDID);
            break;
         case MW2_Mercs_3DFX:
         case MW2_Mercs_DOS:
            b1 = new GLInstallButton(this, count, "MW2: Mercenaries (DOS+Windows)", CDpath, CDID);
            break;
         case MW2_31stcc_TITAN:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (Titanium Trilogy)", CDpath, CDID);
            b2 = new GLInstallButton(this, count, "MW2: 31st Century Combat (Battlepack)", CDpath, MW2_31stcc_TT_BP);
            break;
         case MW2_GBL_TITAN:
            b1 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (Titanium Trilogy)", CDpath, CDID);
            b2 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (Battlepack)", CDpath, MW2_GBL_TT_BP);
            break;
         case MW2_Mercs_TITAN:
            b1 = new GLInstallButton(this, count, "MW2: Mercenaries (Titanium Trilogy)", CDpath, CDID);
            b2 = new GLInstallButton(this, count, "MW2: Mercenaries (Battlepack)", CDpath, MW2_Mercs_TT_BP);
            break;
//         case MW2NetMechInstall:
         case MW2_GBL_PE:
            b1 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (Pentium Edition)", CDpath, CDID);
            break;
         case MW2_GBL_Hybrid_DOS_Win:
            b1 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (DOS+Win)", CDpath, CDID);
            break;
         case MW2_31stcc_BP:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (Battlepack)", CDpath, CDID);
            break;
         case MW2_GBL_BP:
            b1 = new GLInstallButton(this, count, "MW2: Ghost Bear's Legacy (Battlepack)", CDpath, CDID);
            break;
         case MW2_31stcc_3DFX:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (3DFX)", CDpath, CDID);
            break;
         case MW2_31stcc_Matrox:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (Matrox Mystique)", CDpath, CDID);
            break;
         case MW2_31stcc_ATI:
            b1 = new GLInstallButton(this, count, "MW2: 31st Century Combat (ATI Edition)", CDpath, CDID);
            break;
         case MW3_CD_eng:
         case MW3_CD_ger:
            b1 = new GLInstallButton(this, count, "MechWarrior 3", CDpath, CDID);
            break;
	 default:
	    printf ("Nothing to install found in %s\n", CDpath);
   }
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
size_t MechVM::showCDROMWidgets()
{
   size_t count = 0;
   size_t v = 1;
   char driveLetter = 'A';

   // Drives is a bitmask indicating which drives are present
   DWORD drives = GetLogicalDrives();

   while (v < drives) {
      size_t x = v & drives;
      if (x != 0) {
         BGString driveName;
         driveName += driveLetter;
         driveName += ":\\";
         int dt = GetDriveType(driveName.getChars());
         if (dt == DRIVE_CDROM) {
            BOOL mounted = GetVolumeInformation(driveName.getChars(), 0, 0, 0, 0, 0, 0, 0); 
            if (mounted) {
               createInstallButton(count, driveName.getChars());
            }
         }
      }
      v += v;
      driveLetter++;
   }
   return count;
}

#else

size_t MechVM::showCDROMWidgets()
{
   // MemoryBlock can't read /proc/mounts, because that files says it has size 0
   size_t count = 0;
   size_t BytesToRead = 10*1024;
   char* buf = new char[BytesToRead];
   FILE* newHandle = fopen ("/proc/mounts", "rb");
   size_t size = fread (buf, 1, BytesToRead, newHandle);
   fclose (newHandle);

   size_t i = 0;
   //printf("/proc/mounts has %i bytes, scanning ...\n", size);

   while (i < size) {
      // Find end of line
      size_t j = i+1;
      while (buf[j] != 0x0a && buf[j] != 0x0d && j < size) {
         ++j;
      }

      BGString str;
      str.copyPart((const char*) buf, i, j);
      //printf ("Line starting at %i: %s\n", i, str.getChars());

      if (j-i > 5) {
         if (buf[i]   == '/' 
         &&  buf[i+1] == 'd' 
         &&  buf[i+2] == 'e' 
         &&  buf[i+3] == 'v' 
         &&  buf[i+4] == '/')
         {
            while (buf[i] != ' ' && i < j) {
               ++i;
            }

            ++i;
            size_t k = i;
            while (buf[k] != ' ' && k < j) {
               ++k;
            }

            BGString mountPoint;
            mountPoint.copyPart((const char*) buf, i, k);
            //printf("Found potential mount point: \"%s\" ... not?\n", mountPoint.getChars());
            createInstallButton(count, mountPoint.getChars());
         }
      }
      i = j+1;
   }

   return count;
}

#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
void MechVM::showCDROMReportWidgets(int y)
{
   size_t x = 0;
   size_t v = 1;
   const int ButtonWidth = 100;
   const int ButtonHeight = 30;
   //const int margin = 10;
   bool found = false;

   // Drives is a bitmask indicating which drives are present
   DWORD drives = GetLogicalDrives();
   char driveLetter = 'A';
   size_t xpos = 20;
   while (v < drives) {
      size_t x = v & drives;
      if (x != 0) {
         BGString driveName;
         driveName += driveLetter;
         driveName += ":\\";
         int dt = GetDriveType(driveName.getChars());
         if (dt == DRIVE_CDROM) {
            BOOL mounted = GetVolumeInformation(driveName.getChars(), 0, 0, 0, 0, 0, 0, 0); 
            if (mounted) {
               if (!found) {
                  GLLabel* label = new GLLabel (this, 20, y, 800, 30);
                  label->setTitle(34);
                  add(label);
                  y += ButtonHeight + margin;
                  found = true;
               }

               //createInstallButton(count, driveName.getChars());
               GLButton* button = new GLButton(this, xpos, y, ButtonWidth, ButtonHeight);
               xpos += ButtonWidth + margin;
               button->setTitle(driveName.getChars());
               add(button);
               button->setListener (dynamic_cast<MessageListener*> (this));
               button->userData = reportCDRomButtonID;
            }
         }
      }
      v += v;
      driveLetter++;
   }

   if (!found) {
      label = new GLLabel (this, 20, y, 800, 30);
      label->setTitle(35);
      add(label);
      found = true;
   }

   Config* config = getConfig();
   if (!config->hasKeyValuePair ("GameName", "NetMech")) {
      xpos = 20;
      y += ButtonHeight + margin;
      GLButton* button = new GLButton(this, xpos, y, 200, ButtonHeight);
      button->setTitle("Install NetMech");
      add(button);
      button->setListener (dynamic_cast<MessageListener*> (this));
      button->userData = installNetMechID;
   }

   xpos = 20;
   y += ButtonHeight + margin;
   GLButton* button = new GLButton(this, xpos, y, 200, ButtonHeight);
   button->setTitle("Install from CD image");
   add(button);
   button->setListener (dynamic_cast<MessageListener*> (this));
   button->userData = installFromCDImageID;
}

#else

void MechVM::showCDROMReportWidgets(int /*y*/)
{
}

#endif

///////////////////////////////////////////////////////////////////////////////

void installFromCDImage()
{
   BGString imgFN;
   if (getOpenFileName(imgFN)) {
      int fileType = getFileTypeFromExtension(imgFN.getChars());
      if (isArchiveType (fileType)) {
         displayMsg ("Analyzing CD");
         MWArchive* image = openArchive (imgFN.getChars());
         BGFilePath<TCHAR> path = getExecDir();
         path += "temp";
         bgmkdir(path.getChars());
         unpackArchive(image, path.getChars());
         int cdType = getCDType (path.getChars());

         BGString str = "Installing ";
         str += getCDName (cdType);
         displayMsg (str.getChars());
         install(cdType, path.getChars(), imgFN.getChars());
         deleteTree(path.getChars());
         //displayMsg ("Installation complete");
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Show widgets for starting installation

void MechVM::showInstallWidgets()
{
   // Remove other widgets
   clear(); 
   createToolbar();

   // Display install widgets
   size_t count = showCDROMWidgets();

   if (count == 0) {
      label = new GLLabel (this, 20, (int)(105+count*40), 800, 30);
      label->setTitle(30);
      add(label);
      count = 1;
   }

   // Display import widgets
   GLLabel* label = new GLLabel (this, 20, (int)(105+count*40), 140, 30);
   label->setTitle(31);
   add(label);
   importPathEdit = new GLLineEdit (this, 140, (int)(100+count*40), 450, 30);
   #ifdef _WIN32
   importPathEdit->setText ("D:\\");
   #else
   importPathEdit->setText ("/media/");
   #endif
   add (importPathEdit);

   GLButton* button = new GLButton (this, 600, (int)(100+count*40), 200, 30);
   button->setTitle(32);
   button->userData = startImportButtonID;
   button->setListener (this);
   add (button);

   button = new GLButton (this, 600, (int)(140+count*40), 200, 30);
   button->setTitle(33);
   button->userData = searchForCDButtonID;
   button->setListener (this);
   add (button);
   count += 2;

   showCDROMReportWidgets((int)(140+count*40));
}

///////////////////////////////////////////////////////////////////////////////

void startSim()
{
   if (sim == NULL)
      sim = new MechSim();
   //sim->loadMechs();
   currVis = sim;
}

void returnToMechVMMainWindow()
{
   currVis = mainWin;
}

//#ifdef _WIN32
void MechVM::startCampaign()
{
   size_t sel = gameTable->getSelectedRow();
   BGString gameName, StartMethod, executableFN, parameters, GamePath;
   if (getString (gameLauncherTableID, sel, 0, gameName)) {
      Config* config = getConfig();
      int found = -1;

      // Find settings
      for (size_t i = 0; i < config->getKeyCount(); ++i) {
         BGString key = config->getKey(i);
         BGString value = config->getValue(i);
         if (key.equals("GameName") && value.equals(gameName.getChars())) {
            found = i;
         }
      }

      // Retrieve settings
      if (found >= 0) {
         for (size_t i = found+1; i < config->getKeyCount(); ++i) {
            BGString key = config->getKey(i);
            BGString value = config->getValue(i);

            if (key.equals("StartMethod")) {
               StartMethod = value;
               if (value.equals("InternalShell"))
                  shell = new MechShell(this);
	         #ifdef _WIN32
            } else if (key.equals("compatibility")) {
               SetEnvironmentVariable("__COMPAT_LAYER", value.getChars()); 
	         #endif
            } else if (key.equals("archive")) {
               if (shell != NULL) {
                  if (!shell->loadArchive(value.getChars()))
                     printf ("Unable to open %s\n", value.getChars());
               } else {
                  printf ("Cannot set archive before StartMethod InternalShell "
                          "has been stated\nIgnored: %s\n", value.getChars());
               }
            } else if (key.equals("GamePath")) {
                  GamePath = value;
            } else if (key.equals("CampaignFile")) {
               if (shell != NULL) {
                  BGString CampaignFile = getDataDir();
                  CampaignFile += OSPathSep;
                  CampaignFile += "campaigns";
                  CampaignFile += OSPathSep;
                  CampaignFile += value;
                  if(!shell->setCampaignFilePath (CampaignFile.getChars())) {
                     printf ("Unable to load campaign file %s - aborting\n",
                        CampaignFile.getChars());
                     delete shell;
                     shell = NULL;
                  }
               } else {
                  printf ("Cannot set campaign file before StartMethod InternalShell "
                          "has been stated\nIgnored: %s\n", value.getChars());
               }
            } else if (key.equals("SetShellVar")) {
               if (shell != NULL) {
                  shell->setVar(value.getChars(), false);
               } else {
                  printf ("Cannot set variables before StartMethod InternalShell "
                          "has been stated\nIgnored: %s\n", value.getChars());
               }
            } else if (key.equals("ExecutableFN")) {
               executableFN = value.getChars();
            } else if (key.equals("parameters")) {
               parameters = value.getChars();
            } else if (key.equals("GameName")) {
               // New "GameName" entry concludes previous game definition
               i = config->getKeyCount();
            } 
         }
      } else
         displayMsg ("Internal Error - unknown game name");

      if (shell != NULL) {
         if (shell->startCampaign (gameName.getChars()))
            currVis = shell;
         else {
            delete shell;
            shell = NULL;
         }
      } else {
	 #ifdef _WIN32
         if (StartMethod.equals("RunNative")) {
            BGString cmdLine = executableFN;
            cmdLine += " ";
            cmdLine += parameters;

            char* cmdLineChars = new char[cmdLine.getLength()+1];
            memcpy (cmdLineChars, cmdLine.getChars(), cmdLine.getLength()+1);

            // Create process
            PROCESS_INFORMATION processInfo;
            ZeroMemory(&processInfo, sizeof(processInfo));
            STARTUPINFO startInfo;
            ZeroMemory(&startInfo, sizeof(startInfo));
            startInfo.cb = sizeof(startInfo);
            BOOL processCreated = 
               CreateProcess (NULL, cmdLineChars, NULL, NULL, false, 
                              NORMAL_PRIORITY_CLASS, NULL, GamePath.getChars(), 
                              &startInfo, &processInfo);
            if (processCreated) {
               // Wait for decompressor to complete
               DWORD exitCode;
               BGString str = "Running ";
               str += cmdLine;
               displayMsg (str.getChars());

               do {
                  if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
                     BGString str = "Unable to run ";
                     str += cmdLine;
                     displayMsg (str.getChars());
                     exitCode = STILL_ACTIVE+1;
                  }
                  Sleep(25);
               } while (exitCode == STILL_ACTIVE);

               str = "Process ended with code "; 
               str.appendInt (exitCode);
               displayMsg (str.getChars());
            } else {
               BGString str ="Invocation failed: ";
               str += cmdLine;
               #ifdef _WIN32
               str += ", ";
               str.appendOSError();
               #endif
               displayMsg (str.getChars());
            }
            delete cmdLineChars;
            SetEnvironmentVariable("__COMPAT_LAYER", ""); 
         }
         #endif
      }
   }
}
//#else
//void MechVM::startCampaign()
//{
//}
//#endif

void MechVM::startMechLab()
{
   MechLab* mechLab = new MechLab(MechLab_AllButtonsID, this);
   mechLab->setGeometry(0, 0, getWidth(), getHeight());
   mechLab->addWeapons("all");
   currVis = mechLab;

   Config* cfg = getConfig();
   const char* mekFormatNum = cfg->getValue("MEKFileVersion");
   int MEKFileVersion = 0;
   if (mekFormatNum != NULL)
      MEKFileVersion = atoi(mekFormatNum);
   mechLab->setMEKformat(MEKFileVersion);

   mechLab->showChassisTable();
   mechLab->showMech((size_t) 0);
   mechLab->setReturnVis(this);
}

///////////////////////////////////////////////////////////////////////////////
// Export filters

void MechVM::exportWTB ()
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      mesh = MechWarriorIIPRJ::loadWTB(block);
      if (mesh != NULL) {
         mesh->calculateNormals();
         Point3D min = mesh->getMinCoords();
         Point3D max = mesh->getMaxCoords();
         meshCenter = 0.5*(max+min);
         viewDist = 2*(max-min).length();

         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            mesh->saveToObj(outFN.getChars());
         }
         delete mesh;
      }
      delete block;
   }
}

void MechVM::exportSFL ()
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      MemoryBlock* wavBlock = MechWarriorIIPRJ::SFL2Wav(block);
      if (wavBlock != NULL) {
         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            wavBlock->saveToFile(outFN.getChars());
         }
         delete wavBlock;
      }
      delete block;
   }
}

void MechVM::exportSHP ()
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      Texture* texture = MechWarriorIIPRJ::loadSHP(*block, 0, pallette);
      if (texture != NULL) {
         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            size_t w = texture->getWidth();
            size_t h = texture->getHeight();
            texture->saveToFile(outFN.getChars(), 0, 0, w, h);
         }
         delete texture;
      } else {
         messageBox(getVersionStr(), "Unsupported image");
         return;
      }
      delete block;
   }
}

void MechVM::exportBWD (bool recursive)
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   MechWarriorIIPRJ* mw2prj = dynamic_cast <MechWarriorIIPRJ*> (mwArchive);
   if (block != NULL && mw2prj != NULL) {
      XMLTree xmlTree;
      XMLTreeIterator* iter = xmlTree.getIterator();
      iter->addChild("BWD");
      iter->goToChild(0);
      mw2prj->BWD2XML(*block, *iter, recursive, NULL);
      BGString outFN;
      if (getSaveFileName (outFN)) {
         DeleteFile (outFN.getChars());
         xmlTree.saveToFile(outFN.getChars());
      }
      delete block;
      delete iter;
   }
}

// Import a level from a BWD
void MechVM::importLevel ()
{
   BGString outputDir = getDataDir(), BWDFN;
   outputDir.appendNoDups (OSPathSep);
   outputDir += "campaigns";
   outputDir.appendNoDups (OSPathSep);
   getSelectedMW2PRJFileName (BWDFN);
   outputDir += BWDFN.getChars();
   bgmkdir (outputDir.getChars());

   MemoryBlock* block = getSelectedMW2PRJFile();
   MechWarriorIIPRJ* mw2prj = dynamic_cast <MechWarriorIIPRJ*> (mwArchive);
   if (block != NULL && mw2prj != NULL) {
      XMLTree xmlTree;
      XMLTreeIterator* iter = xmlTree.getIterator();
      iter->addChild("BWD");
      iter->goToChild(0);

      // Call import function
      mw2prj->BWD2XML(*block, *iter, true, outputDir.getChars());
      BGString outFN = outputDir.getChars();
      outFN.appendNoDups (OSPathSep);
      outFN += BWDFN;
      outFN += ".xml";
      DeleteFile (outFN.getChars());
      xmlTree.saveToFile(outFN.getChars());
      delete block;
      delete iter;
   }
}

void MechVM::exportXEL ()
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      Texture* texture = MechWarriorIIPRJ::loadTexture(block->getPtr(), block->getSize(), 
                                              pallette);
      if (texture != NULL) {
         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            size_t w = texture->getWidth();
            size_t h = texture->getHeight();
            texture->saveToFile(outFN.getChars(), 0, 0, w, h);
         }
         delete texture;
      } else {
         messageBox(getVersionStr(), "Unsupported image");
         return;
      }
      delete block;
   }
}


void MechVM::exportTEX ()
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      texture = MechWarriorIIPRJ::loadTEX(block->getPtr(), block->getSize());
      if (texture != NULL) {
         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            size_t w = texture->getWidth();
            size_t h = texture->getHeight();
            texture->saveToFile(outFN.getChars(), 0, 0, w, h);
         }
         delete texture;
      }
      delete block;
   }
}

void MechVM::exportEFA (int format)
{
   MemoryBlock* block = getSelectedMW2PRJFile();
   if (block != NULL) {
      MemoryBlock* pcx = LZdecode(block);
      if (pcx != NULL) {
         BGString outFN;
         if (getSaveFileName (outFN)) {
            DeleteFile (outFN.getChars());
            if (format == MW2PRJContextMenu_EFA2PCX) {
               pcx->saveToFile (outFN.getChars());
            } else {
               Texture* texture = MechWarriorIIPRJ::loadPCX(pcx->getPtr(), pcx->getSize());
               texture->saveToFile(outFN.getChars(), 0, 0, texture->getWidth(),
                  texture->getHeight());
               delete texture;
            }
         }
         delete pcx;
      }
      delete block;
   }
}

MemoryBlock* createWTB (Mesh* mesh)
{
   WORD color = 27648; //128;

   // Compute WTB size
   size_t wtbSize = 32 + mesh->getPointCount() * 16;

   for (size_t i = 0; i < mesh->getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh->getPolygon(i);
      if (poly->getPointCount() < 5)
         wtbSize += 12;
      else
         wtbSize += 18;
   }

   MemoryBlock* wtb = new MemoryBlock(wtbSize);

   // Fill WTB header
   memcpy (wtb->getPtr(), "WTBO", 4);
   wtb->fill(4, 20, 0); // unknown fields
   wtb->setWord(0x18, mesh->getPointCount());
   wtb->setWord(0x1a, mesh->getPolygonCount());
   wtb->setDWord(0x1c, 2); // 31stcc, BM1_HEAD: 3

   // Copy vertices
   size_t offset = 32;
   for (size_t i = 0; i < mesh->getPointCount(); ++i) {
      Point3D p; 
      mesh->getPoint(i, p);
      wtb->setDWord(offset, (DWORD) p.x);
      wtb->setDWord(offset+4, (DWORD) p.y);
      wtb->setDWord(offset+8, (DWORD) p.z);
      WORD u = random(0, 128);
      WORD v = random(0, 128);
      wtb->setWord(offset+12, u);
      wtb->setWord(offset+14, v);
      offset += 16;
   }

   // Copy polygons
   for (size_t i = 0; i < mesh->getPolygonCount(); ++i) {
      MeshPolygon* poly = mesh->getPolygon(i);
      if (poly->getPointCount() < 8) {
         wtb->setWord(offset, color);
         wtb->setWord(offset+2, poly->getPointCount());

         for (size_t j = 0; j < poly->getPointCount(); ++j) {
            wtb->setWord(offset+4+2*j, poly->getPoint(j));
         }

         if (poly->getPointCount() < 5)
            offset += 12;
         else
            offset += 18;
      }
   }

   return wtb;
}

// Replace a WTB file inside a PRJ with an external OBJ file
void MechVM::replaceWTBwithOBJ()
{
   BGString objFN, outFN;
   if (getOpenFileName(objFN)) {
      MemoryBlock inFile;
      Mesh mesh;
      if (mesh.loadFromObj(objFN.getChars())) {
         MemoryBlock* wtb = createWTB (&mesh);
         if (wtb != NULL) {
            outFN = objFN;
            outFN += ".WTB";
            wtb->saveToFile(outFN.getChars());
            delete wtb;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Receive messages

void MechVM::receiveMessage (VirtualClass* sender, int /*MessageType*/)
{
   GLWindow* win = dynamic_cast <GLWindow*> (sender);
   if (win != NULL) {

      if ((  win->userData == playGameButtonID
          || win->userData == installButtonID
          || win->userData == startImportButtonID
          || win->userData == MechLabButtonID)
      &&  mwArchive != NULL)
      {
         delete mwArchive;
         mwArchive = NULL;
      }

      switch (win->userData) {
         case playGameButtonID:
         {
            clear();
            createToolbar();
            gameTable = new GLTableView (this, 20, 80, 800, 400, gameLauncherTableID);
            gameTable->userData = gameLauncherTableID;
            gameTable->setDataSource(this);
            gameTable->setColWidth(1, 50, 150);
            gameTable->setRowHeights(100, 20);
            gameTable->setBackgroundColor (1, 1, 1);
            gameTable->setListener (this);
            add (gameTable);

            GLButton* b1 = new GLButton (this, 20, 500, 200, 20);
            b1->userData = startCampaignID;
            b1->setListener (this);
            b1->setTitle (tr(29));
            add (b1);

            /*GLButton* b2 = new GLButton (this, 20, 540, 200, 20);
            b2->userData = instantActionID;
            b2->setListener (this);
            b2->setTitle ("Test internal sim.");
            add (b2);*/
            break;
         }
         case MechLabButtonID:
            startMechLab();
            break;
         case viewFileButtonID:
            clear();
            createToolbar();
            viewExternalFile();
            break;
         case upDirButtonID:
            mwArchive->leaveDir();
            PRJTable->setRowHeights(mwArchive->getEntryCount(), 20);
            PRJTable->clearSelection();
            break;
         case MW2PRJTableID: // Selection changed
            {
               int row = PRJTable->getSelectedRow();
               //if (mwArchive->inSubDir()) {
               if (mwArchive->isDirectory (row)) {
                  mwArchive->enterSubDir(row);
                  PRJTable->setRowHeights(mwArchive->getEntryCount(), 20);
                  PRJTable->clearSelection();
               } else {
                  ViewPRJFile();
               }
            }
            break;
         case installButtonID:
            showInstallWidgets();
            break;
         case startImportButtonID:
            {
               size_t mw2Imports = importMW2Files (importPathEdit->getText());
               if (mw2Imports > 0) {
                  BGString msg;
                  msg.appendInt (mw2Imports);
                  msg += " mechs imported";
                  displayMsg (msg.getChars());
               } else {
                  BGString importXML = getExecDir();
                  importXML.appendNoDups(OSPathSep);
                  importXML += "mw3-mechs.xml";
                  displayMsg ("Importing from MechWarrior 3");
                  MW3MechImporter (importPathEdit->getText(), 
                                 getConfig()->getMechPath(), 
                                 importXML.getChars());
                  displayMsg ("Done");
               }
            }
            break;
         case searchForCDButtonID:
            {
               size_t i = 2;
               createInstallButton(i, importPathEdit->getText());
               break;
            }
         case startCampaignID:
            startCampaign();
            break;
         case instantActionID:
            startSim();
            break;
         default:
            if (win->userData == reportCDRomButtonID) {
               GLButton* b = dynamic_cast <GLButton*> (win);
               BGString str = "Creating contents report for ";
               str += b->getTitle();
               displayMsg (str.getChars());
               if (createInstallReport(b->getTitle(), NULL, true))
                  displayMsg ("Report created.");
               else
                  displayMsg ("Aborted");
            #ifdef _WIN32
            } else if (win->userData == installNetMechID) {
               installNetMech();
            #endif
            } else if (win->userData == installFromCDImageID) {
               installFromCDImage();
            } else if (win->userData >= importFromDriveButtonID) {
               #ifdef _WIN32
               BGString sourcePath;
               sourcePath += ((char) (win->userData-importFromDriveButtonID));
               sourcePath += ":\\";
               importPathEdit->setText (sourcePath.getChars());
               #endif
            } else if (win->userData == installFromDriveButtonID) {
               GLInstallButton* ib = dynamic_cast <GLInstallButton*> (win);
               if (ib != NULL) {
                  clear();
                  createToolbar();

                  // Install selected game type
                  int installType = ib->getInstallAlgorithmID();
                  if (!install (installType, ib->getSourcePath(), NULL)) {
                     displayMsg ("Installation failed");
                  }
               }
            }
      }
   } else {
      glutContextMenuEntry* entry = dynamic_cast <glutContextMenuEntry*> (sender);
      if (entry != NULL) {
         switch (entry->ID) {
            case MW2PRJContextMenu_SaveAs:
               {
                  // Context menu: Save As ...
                  BGString FN;
                     MemoryBlock* block = getSelectedMW2PRJFile();
                  if (block != NULL) {
                     getSelectedMW2PRJFileName (FN);
                     getSaveFileName (FN);
                     block->saveToFile(FN.getChars());
                     //mwArchive->saveToFile(FN.getChars());
                  }
               }
               break;
            case MW2PRJContextMenu_ReplaceWith:
               if (mwArchive->canAddFilesToArchive()) {
                  BGString fn;
                  if (getOpenFileName (fn)) {
                     size_t fileNum = PRJTable->getSelectedRow();
                     mwArchive->replaceFile (fileNum, fn.getChars());
                     delete mwArchive;
                     mwArchive = NULL;
                     delete PRJTable;
                     clear();
                     createToolbar();
                  }
               }
               break;
            case MW2PRJContextMenu_AddFiles:
               {
                  BGString fn;
                  if (getOpenFileName (fn)) {
                     mwArchive->addFileToArchive (fn.getChars());
                     delete mwArchive;
                     mwArchive = NULL;
                     delete PRJTable;
                     clear();
                     createToolbar();
                  }
               }
               break;
            case MW2PRJContextMenu_SHP2BMP:
               exportSHP();
               break;
            case MW2PRJContextMenu_XEL2BMP:
               exportXEL ();
               break;
            case MW2PRJContextMenu_SFL2WAV:
               exportSFL ();
               break;
            case MW2PRJContextMenu_WTB2OBJ:
               exportWTB ();
               break;
            case MW2PRJContextMenu_BWD2XML:
               exportBWD (false);
               break;
            case MW2PRJContextMenu_BWD2XML_rec:
               exportBWD (true);
               break;
            case MW2PRJContextMenu_ImportLevel:
               importLevel ();
               break;
            case MW2PRJContextMenu_TEX2BMP:
               exportTEX ();
               break;
            case MW2PRJContextMenu_EFA2PCX:
               exportEFA (MW2PRJContextMenu_EFA2PCX);
               break;
            case MW2PRJContextMenu_EFA2BMP:
               exportEFA (MW2PRJContextMenu_EFA2BMP);
               break;
            case MW2PRJContextMenu_OBJ2WTB:
               replaceWTBwithOBJ();
               break;
            case MW2PRJContextMenu_unpackAll:
               unpackPrj();
               break;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
// Render

double zoom = 1.0;

void MechVM::render(int x, int y, int w, int h)
{
   glViewport (0, 0, w, h);
   Config* cfg = getConfig();
   Color clearCol = cfg->getViewerBackgroundColor();
   glClearColor ((GLclampf) clearCol.r, (GLclampf) clearCol.g, (GLclampf) clearCol.b, 0.0f);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Render mesh
   if (mesh != NULL) {
      // Set viewing projection
      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      //int w = getWidth();
      //int h = getHeight();
      gluPerspective (60, ((double) w) / ((double) h),
                      viewDist*0.01, 
                      viewDist* 100);
      glMatrixMode (GL_MODELVIEW);

      // Render
      glColor3d(1, 1, 1);
      glViewport (220, 0, w-230, h);
      glLoadIdentity();
      gluLookAt (0, 0, viewDist,
                 meshCenter.x, meshCenter.y, meshCenter.z, 
                 0, 1, 0);
      //glTranslated(0, 0, 1000);
      glEnable(GL_NORMALIZE);
      glRotated (360*(GetTickCount()%2000)/2000.0, 0, 1, 0);

      // Activate light source
      glEnable (GL_LIGHTING);
      glEnable (GL_COLOR_MATERIAL);
      //glEnable (GL_TEXTURE_2D);
      //glFrontFace (GL_CCW);
      //glDisable (GL_LIGHT_MODEL_TWO_SIDE);
      //glCullFace (GL_BACK);
      //glEnable (GL_TEXTURE_2D);

      GLfloat pos[4] = {0.0f, 1.0f, 0.0f, 0.0f};
      GLfloat color[3] = {0.7f, 0.7f, 0.7f};
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

      glEnable (GL_DEPTH_TEST);
      mesh->render();
      glDisable (GL_LIGHT0);
      glDisable (GL_LIGHTING);
   }

   // Render texture
   if (texture != NULL) {
      texture->use();
      //int x = (int) (zoom * width / 2 + 200);
      //int y = (int) (zoom * height / 2 + 100);
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glColor3d(1, 1, 1);
      glViewport (0, 0, w, h);
      glLoadIdentity();
      glBegin(GL_POLYGON);
         texture->glTexCoord(0, 0);
         //glVertex2d (-0.5,-0.5);
         glVertex2d (-(double)(zoom * texture->getWidth()) / (double)(width), 
                     -(double)(zoom * texture->getHeight()) / (double)(height));
         texture->glTexCoord(1, 0);
         //glVertex2d (0.5, -0.5);
         glVertex2d ( (double)(zoom * texture->getWidth()) / (double)(width), 
                     -(double)(zoom * texture->getHeight()) / (double)(height));
         texture->glTexCoord(1,1);
         //glVertex2d (0.5, 0.5);
         glVertex2d ((double)(zoom * texture->getWidth()) / (double)(width), 
                     (double)(zoom * texture->getHeight()) / (double)(height));
         texture->glTexCoord(0, 1);
         //glVertex2d (-0.5, 0.5);
         glVertex2d (-(double)(zoom * texture->getWidth()) / (double)(width), 
                      (double)(zoom * texture->getHeight()) / (double)(height));
      glEnd();
   }

   // Render widgets
   GLWindowContainer::render(x, y, w, h);
}

///////////////////////////////////////////////////////////////////////////////
// Forwards 

void changeSize (GLsizei width, GLsizei height);

void renderScene ()
{
   #ifdef WIN32
   if (!maximized) {
      maximized = true;
      HWND handle = GetActiveWindow();
      WINDOWPLACEMENT placement;
      placement.length = sizeof(WINDOWPLACEMENT);
      GetWindowPlacement (handle, &placement);
      placement.showCmd = SW_SHOWMAXIMIZED;
      SetWindowPlacement (handle, &placement);

      WINDOWINFO wInfo;
      wInfo.cbSize = sizeof (wInfo);
      GetWindowInfo (handle, &wInfo);
      changeSize (wInfo.rcClient.right - wInfo.rcClient.left,
                  wInfo.rcClient.bottom - wInfo.rcClient.top);
   }
   #endif

   try {
      currVis->render(0, 0, mainWin->getWidth(), mainWin->getHeight());
      SDL_GL_SwapBuffers();
   } catch (...) { // Exception
      log ("Caught an unknown exception while rendering the scene\n");
   }
}

// Set visual responsible for rendering
void setCurVis (GLWindow* vis)
{
   currVis = vis;
   renderScene();
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard events

void keyPressed (BGKey key, bool shiftHeld, bool controlHeld, bool down)
{
   try {
      if (key.sym == SDLK_ESCAPE) {
         s_done = true;
/*      } else if (key == '+') {
         zoom *= 2;
      } else if (key == '-') {
         zoom /= 2;*/
      } else {
         if (focused == NULL)
            currVis->keypressed (key, shiftHeld, controlHeld, down);
         else
            focused->keypressed (key, shiftHeld, controlHeld, down);
      }
   } catch (...) { // Exception
      BGString msg = "Caught an unknown exception while handling key ";
      msg.appendInt (key.sym);
      logLn (msg);
   }

   //renderScene();
}

////////////////////////////////////////////////////////////////////////////////

/*void specialKeyPressed (int key, int x, int y)
{
   try {
      bool shiftHeld, controlHeld;
      checkShiftAndControl (shiftHeld, controlHeld);
      if (focused == NULL)
         currVis->specialKeyPressed (key, shiftHeld, controlHeld);
      else
         focused->specialKeyPressed (key, shiftHeld, controlHeld);
   } catch (...) { // Exception
      BGString msg = "Caught an unknown exception while handling special key ";
      msg.appendInt (key);
      logLn (msg);
   }
   renderScene();
}*/

////////////////////////////////////////////////////////////////////////////////
// Window size changed

void changeSize (GLsizei width, GLsizei height)
{
/*   BGString str = "New window size is ";
   str.appendInt(width);
   str+="x";
   str.appendInt(height);
   str.printLn();*/

   try {
      const int StrangeCorrectionConstant = 0; //10;
      currVis->setGeometry (0, 0, width, height+StrangeCorrectionConstant);
      if (currVis != mainWin)
         mainWin->setGeometry (0, 0, width, height+StrangeCorrectionConstant);
      //if (!IdleUpdate)
         //renderScene();
   } catch (...) { // Exception
      BGString msg = "Caught an unknown exception while changing size to ";
      msg.appendInt (width);
      msg += "x";
      msg.appendInt (height);
      logLn (msg);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Button state changed

void buttonStateChange (int button, bool down, int x, int y)
{
   try {
      if (focused != NULL 
      && !down
      && x > focused->getLeft()
      && y > focused->getTop() 
      && x < focused->getRight() 
      && y < focused->getBottom()) 
      {
         focused = focused->buttonStateChanged (button, down, x, y);
      } else {
         focused = currVis->buttonStateChanged (button, down, x, y);
      }
   } catch (...) {
      log ("Caught an unknown exception while processing a mouse"
           "button state change\n");
   }

   //renderScene();
}

////////////////////////////////////////////////////////////////////////////////
// Mouse moved

void mouseMoved (int x, int y)
{
   try {
      if (focused != NULL)
         focused->mouseMoved (x, y);
      else
         currVis->mouseMoved (x, y);
   } catch (...) { // Exception
      log ("Caught an unknown exception while processing a mouse move\n");
   }
}

///////////////////////////////////////////////////////////////////////////////
// Main function

int main (int argc, char* argv[])
{
   printf ("%s\n", getVersionStr());
   printf ("This is the message window for MechVM. If you close it, MechVM terminates.\n");
   setExecDir (argv[0], true);

   BGString configFN = getDataDir();
   configFN += OSPathSep;
   configFN += "MechVM.cfg";
   Config* cfg = createConfig();
   if (!cfg->readFromFile (configFN.getChars())) {
     printf ("Failed to read configuration file %s\n", configFN.getChars());
   }
   fillWeaponsList();

   // Load language file
   const char* langFNbase = cfg->getValue ("Language");
   BGString langFN = getExecDir();
   langFN += OSPathSep;
   langFN += langFNbase;
   if (!loadLangFile (langFN.getChars())) {
      BGString errStr = "Cannot load language definition file - ";
      errStr += langFN.getChars();
      errStr += " - Exiting";
      messageBox("Error", errStr.getChars());
      return 0;
   }

   #ifdef WIN32
   maximized = false;
   #endif

   #ifdef SimpleMemoryCounters
   clearClassUsage();
   #endif

   #ifndef _WIN32
   QApplication app (argc, argv);
   #endif

   BGString path = getExecDir();
   path += OSPathSep;
   path += "GLUT_9_BY_15.bmp";
   if (!loadFont(path.getChars(), 9, 15))
      printf ("Unable to load font from %s\n", path.getChars());

   mainWin = new MechVM ();
   currVis = mainWin;
   focused = NULL;
   sim = NULL;

   // Init Renderer
   SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
   SDL_EnableUNICODE(1);
   SDL_WM_SetCaption(getVersionStr(), getVersionStr());
   //screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
   SDL_Surface * screen = SDL_SetVideoMode (0, 0, 0, SDL_OPENGL|SDL_RESIZABLE);
   storeScreenSettings (screen);
   changeSize(screen->w, screen->h);
   if (screen == NULL) {
      printf ("Unable to obtain rendering window - aborting\n");
      return 0;
   }
   SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

   if (SDL_NumJoysticks() > 0) {
      /*SDL_Joystick * joystick =*/ SDL_JoystickOpen(0);
      printf ("Joystick found and used\n");
   } else {
      printf ("No joysticks found\n");
   } 

   // Print OpenGL information
   const char* vendor = (const char*) glGetString (GL_VENDOR);
   const char* renderer = (const char*) glGetString (GL_RENDERER);
   const char* version = (const char*) glGetString (GL_VERSION);

   BGString str = "GL Vendor: ";
   str += vendor;
   str += "\nGL Renderer: ";
   str += renderer;
   str +="\nGL Version: ";
   str +=version;
   logLn (str);

   s_done = false;

   while (!s_done) {
      try {
         renderScene();
      } catch (...) {
         printf("Caught an exception while rendering\n");
      }

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
         switch(event.type) {
            case SDL_QUIT:
                s_done = 1;
                break;
            case SDL_KEYDOWN:  /* Handle a KEYDOWN event */
               keyPressed (event.key.keysym, 0, 0, true);
               break;
            case SDL_KEYUP:  /* Handle a KEYDOWN event */
               {
                  bool shiftDown =    ((event.key.keysym.mod & KMOD_LSHIFT) != 0)
                                   || ((event.key.keysym.mod & KMOD_RSHIFT) != 0);
	               keyPressed (event.key.keysym, shiftDown, 0, false);
                  break;
               }
	    case SDL_VIDEORESIZE:
	       changeSize(event.resize.w, event.resize.h);
	       break;
            case SDL_MOUSEMOTION:
               mouseMoved(event.motion.x, event.motion.y);
               break;
            case SDL_MOUSEBUTTONDOWN:
               buttonStateChange(event.button.button, true, event.button.x, event.button.y);
               break;
            case SDL_MOUSEBUTTONUP:
               buttonStateChange(event.button.button, false, event.button.x, event.button.y);
               break;
            case SDL_JOYAXISMOTION:
               break;
         }
      }
   }

   return 0;
}
