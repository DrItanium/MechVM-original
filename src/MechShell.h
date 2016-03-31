////////////////////////////////////////////////////////////////////////////////
// MechShell.h
// Mech shell window for MechVM
// Copyright Bjoern Ganster 2008-2010
////////////////////////////////////////////////////////////////////////////////

#ifndef MechShell__h
#define MechShell__h

#include "GLWindowContainer.h"
#include "Database_MW2.h"
#include "MechWarriorIIPRJ.h"
#include "XMLTree.h"
#include "Texture.h"
#include "GLTableView.h"
#include "MechSim.h"

struct MechShellImage {
   Texture* img;
   int x1, y1, x2, y2;
};

////////////////////////////////////////////////////////////////////////////////
// Mech shell class

class MechShell: public GLWindowContainer, public GLTableViewDataSource
{
public:
   // Constructor, destructor
   MechShell(GLWindow* mainWin);
   virtual ~MechShell();

   // Close archives
   void closeArchives();

   // Receive messages
   virtual void receiveMessage (VirtualClass* sender, int MessageType);

   // Render
   virtual void render(int x, int y, int w, int h);

   // Keyboard event handler
   void keypressed(char key, bool shiftHeld, bool controlHeld);

   // Button state changed over a window: returns child that was clicked
   virtual GLWindow* buttonStateChanged (int /*button*/, bool down,
                                         int /*x*/, int /*y*/);

   virtual void mouseMoved (int /*x*/, int /*y*/);

   // Init
   bool startCampaign (const char* gameName);

   // Enter next room
   bool enterRoom();

   // Execute a command
   void execute(const BGString& cmd);

   // Variables
   bool setVar (const char* assignment, bool evaluate);
   bool setVar (const char* varName, const char* value);

   bool checkCondition (const char* condition);
   bool evaluate (BGString& str, bool opsAllowed);
   void setStr (const char* varName, const char* value);

   // Store variable in config
   void writeBackVar (const char* varName);

   // Simple path setters
   bool setCampaignFilePath (const char* path)
   { return tree.loadFromFile(path); }

   // Load archive to access for pics and other things
   bool loadArchive (const TCHAR* path);

   inline void printVarCount() const
   { printf ("%i vars\n", vars.size()); }

   // Called by Mech Lab to indicate that user saved a new MEK file and
   // intends to use it
   void useConfig(const TCHAR* MEKFN);

   virtual void getDimensions (size_t /*senderID*/, size_t& rowCount, size_t& colCount)
   {
      colCount = 1;
      rowCount = strings.getSize();
   }

   // Get string for a cell
   virtual bool getString (size_t /*senderID*/, int row, int /*col*/, BGString& str)
   {
      if (row < (int) strings.getSize()) {
         str.copyPart((const char*) strings[row], 0, stringSizes[row]);
         return true;
      } else
         return false;
   }

   // Request context menu
   virtual void showContextMenu(VirtualClass* /*sender*/, int /*x*/, int /*y*/)
   {
   }

   Vehicle* loadPlayerVehicle(const char* MEKname, const char* chassisName);

private:
   GLWindow* m_mainWin;
   MWArchive* archive, * archive2;
   BGString archive2name;
   XMLTree tree;
   XMLTreeConstIterator* treeIter;
   BGString room;
   vector<BGString> vars;
   BGVector<MechShellImage*, BGVector_MechShellImage> images;
   int screenSizeX, screenSizeY;
   BGVector<const unsigned char*, BGVector_int> strings;
   BGVector<size_t, BGVector_string> stringSizes;
   GLTableView* tableView;
   GLWindow* sim;

   // Functions carried out on enterRoom()
   Texture* loadImageFromArchive(const char* filePath);
   void clearImages();
   void loadImages();
   void loadTexts();
   void buildScrollableTextFromBWD (MemoryBlock* mb, int x1, int y1, int x2, int y2);

   bool rangeCheck (const char* str);

   // Return index of variable, or -1
   int findVar (const char* name) const;

   const char* retrieveStr (const char* varName) const 
   {
      int index = findVar (varName);
      if (index >= 0) 
         return vars[index+1].getChars();
      else
         return NULL;
   }

   // Select a string from a comma-separated list where the first entry gives the index
   bool selectStr (const char* select, BGString& result);

   void renderTextButton(int x, int y, int w, int h);
   void renderBar(int x, int y, int w, int h);

   // Call DosBox to start the sim
   bool runSim ();
   bool runInternalSim(bool& missionSuccessful);
   bool runExternalSim(bool useDosBox, bool& missionSuccessful);

   // Get path where a game edition is installed
   void getDosBoxGameDir (BGString& gameDir);

   // Show Mech Lab
   void showMechLab(const char* cmd);

   // Update title with model graph name and framerate
   void updateTitle ();

   // Build string list of mech configs for a chassis
   void fetchMEKs(const BGString& args);

   // Extract Mech comment from MEK
   void fetchMEKComment(const BGString& cmd);

   bool setChassisAndConfig (MemoryBlock& userStarBWD, size_t& offset, 
      const char* selChassisStr, const char* selMEKStr);
};

#endif
