////////////////////////////////////////////////////////////////////////////////
// MechVM.h
// Main window for MechVM
// Copyright Bjoern Ganster 2007-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef MechVM__h
#define MechVM__h

#include "MWBase.h"

#include "Toolbar.h"
#include "FramerateCounter.h"
#include "MechWarriorIIPRJ.h"

#include "GLSplitter.h"
#include "GLButton.h"
#include "GLSlider.h"
#include "GLLabel.h"
#include "GLComboBox.h"
#include "GLLineEdit.h"
#include "GLTableView.h"
#include "Archive.h"
#include "MechShell.h"

////////////////////////////////////////////////////////////////////////////////
// Misc functions

void displayMsg (const TCHAR* msg);
inline const char* getVersionStr ()
{
   return "MechVM-2011-04-04";
}

// Set visual responsible for rendering
void setCurVis (GLWindow* vis);
void returnToMechVMMainWindow();

////////////////////////////////////////////////////////////////////////////////
// MechVM main window

class MechVM: public GLWindowContainer, public GLTableViewDataSource
{
public:
   // Constructor, destructor
   MechVM();
   virtual ~MechVM();

   // Set window layout
   void setGeometry (int left, int top, int newWidth, int newHeight);

   // Receive messages
   virtual void receiveMessage (VirtualClass* sender, int MessageType);

   // Process toolbar button event
   void processToolbarButtonEvent(VirtualClass* sender);

   // Update title with model graph name and framerate
   void updateTitle ();

   // TableView implementation: get number of rows and columns
   virtual void getDimensions (size_t senderID, size_t& rowCount, size_t& colCount);

   // TableView implementation: Get string for a cell
   virtual bool getString (size_t senderID, int row, int col, BGString& str);

   // Render
   virtual void render(int x, int y, int w, int h);

   void startMechLab();

   void startCampaign();

   void displayMsg (const TCHAR* msg);

   // Called to set new layout
   void createToolbar();

private:
   GLButton * playGame, * MechLabButton, * viewFileButton, * installButton, 
      * unpackPrjButton, * upDirButton, * setRenderingDistanceButton;
   MWWeapons* weapons;
   size_t weaponsCount;
   MWArchive* mwArchive;
   MechShell* shell;
   GLTableView* PRJTable, * gameTable;
   GLLineEdit* importPathEdit;
   Texture* texture;
   MemoryBlock* pallette;
   Mesh* mesh;
   Point3D meshCenter;
   double viewDist;
   GLLabel* label;

   // Create a button
   GLButton* addButton(int num, const TCHAR* name, int ID);

   // Create a pairs of a label and another control
   // type: 0=LineEdit, 1=Slider, 2=ComboBox
   GLWindow* createControls (const char* title, int num, int type);

   // View file
   void viewFile(const char* fileName, MemoryBlock* block);
   void viewExternalFile();
   void ViewPRJFile();

   // Request context menu
   virtual void showContextMenu(VirtualClass* sender, int x, int y);

   void getSelectedMW2PRJFileName (BGString& FN);
   MemoryBlock* getSelectedMW2PRJFile ();

   // View, show, play
   void viewMesh (const MemoryBlock* mb, const char* fileName);
   void showMech (const MemoryBlock* mb, const char* fileName);
   bool playWAV (const MemoryBlock* mb);

   // Unpack PRJ file 
   void unpackPrj();

   // Export various file types
   void exportTEX ();
   void exportSHP ();
   void exportBWD (bool recursive);
   void exportWTB ();
   void exportSFL ();
   void exportXEL ();
   void exportEFA (int format);

   // Import a level from a BWD
   void importLevel ();

   // Replace a WTB file inside a PRJ with an external OBJ file
   void replaceWTBwithOBJ();

   // Convert SFL file to WAV
   MemoryBlock* SFL2Wav (const MemoryBlock* mb);

   // Show widgets for starting installation or data import
   void showInstallWidgets();
   size_t showCDROMWidgets();
   void showCDROMReportWidgets(int y);
   void createInstallButton(size_t& count, const TCHAR* CDpath);
};

#endif
