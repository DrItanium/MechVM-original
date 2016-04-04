////////////////////////////////////////////////////////////////////////////////
// MechShell.cpp
// Mech shell window for MechVM
// Copyright Bjoern Ganster 2008-2010
////////////////////////////////////////////////////////////////////////////////

#include "MechShell.h"
#include "LZdecode.h"
#include "Config.h"
#include "MechVM.h"
#include "MechLab.h"
#include "GLTableView.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor, destructor

MechShell::MechShell(GLWindow* mainWin)
: GLWindowContainer (NULL),
  m_mainWin (mainWin),
  archive (NULL),
  archive2 (NULL),
  tableView (NULL),
  sim (NULL)
{
   int x = mainWin->getLeft();
   int y = mainWin->getTop();
   int w = mainWin->getWidth();
   int h = mainWin->getHeight();
   setGeometry (x,y,w,h);
}

MechShell::~MechShell()
{
   closeArchives();
   clearImages();

   if (tableView != NULL)
      delete tableView;

   if (treeIter != NULL)
      delete treeIter;
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::closeArchives()
{
   if (archive != NULL)
      delete archive;

   if (archive2 != archive && archive2 != NULL)
      delete archive2;

   archive = NULL;
   archive2 = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Init

bool MechShell::startCampaign (const char* gameName)
{
   treeIter = tree.getConstIterator();
   //treeIter->goToChild (0);
   setStr("GameName", gameName);

   // If no archive2 was specified, use archive1 instead, for NetMech
   if (archive2 == NULL)
      archive2 = archive;

   // Process variable assignments
   for (size_t i = 0; i < treeIter->getAttributeCount(); ++i) {
      BGString command = treeIter->getAttributeName(i);
      
      if (command.equals("setvar")) {
         const char* assignment = treeIter->getAttributeValueByIndex(i);
         setVar (assignment, true);
      } else if (command.equals("setstr")) {
         const char* assignment = treeIter->getAttributeValueByIndex(i);
         setVar (assignment, false);
      } else if (command.equals("exec")) {
         BGString command = treeIter->getAttributeValueByIndex(i);
         execute(command);
      }
   }

   room = treeIter->getAttributeValueByName("startRoom");
   bool result = enterRoom();

   return result;
}

////////////////////////////////////////////////////////////////////////////////

bool MechShell::loadArchive (const TCHAR* path)
{
   if (archive == NULL) {
      archive = openArchive (path);
      if (archive == NULL)
         return false;
      else
         return true;
   } else {
      archive2 = openArchive (path);
      archive2name = path;
      if (archive2 == NULL)
         return false;
      else
         return true;
   }
}

////////////////////////////////////////////////////////////////////////////////

Texture* MechShell::loadImageFromArchive(const char* filePath)
{
   if (archive == NULL)
      return NULL;

   BGString fn = filePath;
   size_t fileSize;
   MemoryBlock* block = NULL;
   Texture* result = NULL;

   // Check if files are supposed to be loaded from an archive
   if (fn.startsWith("1|")) {
      fn = (const char*) (&filePath[2]);
      size_t pos;
      if (fn.findChar('\\', pos)) {
         BGString path;
         path.copyPart (filePath, 0, pos);
         archive->leaveDir();
         if (!archive->enterSubDir(path.getChars()))
            printf ("Can't enter directory %s\n", path.getChars());
         fn.copyPart (filePath, pos+1, strlen(filePath));
      }
      size_t fileNum = archive->getEntryNumberByName(fn.getChars());
      fileSize = archive->getFileSize(fileNum);
      block = new MemoryBlock(fileSize);

      if (fileSize > archive->getArchiveSize()) {
         printf ("Can't load file\n");
         return nullptr;
      }

      if (!archive->getFileChunk(fileNum, 0, block->getPtr(), fileSize)) {
         delete block;
         return nullptr;
      }
   } else if (fn.startsWith("2|")) {
      fn = (const char*) (&filePath[2]);
      size_t pos;
      if (fn.findChar('\\', pos)) {
         BGString path;
         path.copyPart (filePath, 0, pos);
         archive2->leaveDir();
         if (!archive2->enterSubDir(path.getChars()))
            printf ("Can't enter directory %s\n", path.getChars());
         fn.copyPart (filePath, pos+1, strlen(filePath));
      }
      size_t fileNum = archive2->getEntryNumberByName(fn.getChars());
      fileSize = archive2->getFileSize(fileNum);
      block = new MemoryBlock(fileSize);

      if (fileSize > archive2->getArchiveSize()) {
         printf ("Can't load file\n");
         return nullptr;
      }

      if (!archive2->getFileChunk(fileNum, 0, block->getPtr(), fileSize)) {
         delete block;
         return nullptr;
      }
   }

   if (block == NULL)
      return nullptr;

   // Load image
   switch (getFileTypeFromExtension(fn.getChars())) {
      case FT_EFA:
         {
            MemoryBlock* pcx = LZdecode(block);
            result = MechWarriorIIPRJ::loadPCX(pcx->getPtr(), pcx->getSize());
            delete pcx;
         }
         break;
      case FT_PCX:
         result = MechWarriorIIPRJ::loadPCX(block->getPtr(), block->getSize());
         break;
   }

   delete block;
   return result;
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::clearImages()
{
   for (size_t i = 0; i < images.getSize(); ++i) {
      delete images[i]->img;
      delete images[i];
   }
   images.clear();
}

void MechShell::loadImages()
{
   for (size_t i = 0; i < treeIter->getChildCount(); ++i) {
      treeIter->goToChild(i);
      BGString str = treeIter->getNodeName();
      if (str.equals ("img")) {
         const char* inArchiveFN = treeIter->getAttributeValueByName ("inArchive");
         const char* x1 = treeIter->getAttributeValueByName("x1");
         const char* y1 = treeIter->getAttributeValueByName("y1");
         const char* x2 = treeIter->getAttributeValueByName("x2");
         const char* y2 = treeIter->getAttributeValueByName("y2");
         MechShellImage* img = new MechShellImage();
         img->img = loadImageFromArchive (inArchiveFN);
         img->x1 = atoi(x1);
         img->x2 = atoi(x2);
         img->y1 = atoi(y1);
         img->y2 = atoi(y2);
         images.add(img);
      }
      treeIter->goToParent();
   }
}

void MechShell::buildScrollableTextFromBWD (MemoryBlock* mb, int x1, int y1, 
   int x2, int y2)
{
   if (!strcmp((const char*) (mb->getPtr(52)), "ORDR"))
      return;

   size_t endOffs = 52 + mb->getDWord(56);
   if (endOffs > mb->getSize())
      return;

   //const char* text = (const char*) mb->getPtr(60);
   //printf (text);

   if (tableView == NULL)
      tableView = new GLTableView(this);
   tableView->setGeometry(width/2-320+x1, height/2-240+y1, x2-x1, y2-y1);
   tableView->setDataSource(this);
   //tableView->setBackgroundColor(Color(0, 0, 0, 0));
   tableView->textColor = Color (1, 1, 1, 1);

   // Parse text
   size_t i = 60;
   //size_t j = 0; // Line index
   size_t lineStart = i; // Start index of current line
   size_t wordStart = i;
   int x = 0; // Screen position in line
   int charWidth = 9;
   int colWidth = x2-x1-50;

   while (i < mb->getSize()) {
      char c = mb->getByte(i);
      if (c == '\\') {
         strings.add(mb->getPtr(lineStart));
         stringSizes.add(i-lineStart);
         i += 2;
         lineStart = i;
         x = 0;
      } else if (c == ' ') {
         ++i;
         wordStart = i;
         x += charWidth;
      } else {
         ++i;
         x += charWidth;
      }

      if (x > colWidth) {
         size_t lineLen = wordStart-lineStart;
         const unsigned char* str = mb->getPtr(lineStart);
         strings.add(str);
         stringSizes.add(lineLen);
         lineStart = wordStart;
         x = 0;
      }
   }

   // Add the rest
   size_t lineLen = i-lineStart;
   strings.add(mb->getPtr(lineStart));
   stringSizes.add(lineLen);

   size_t lineCount = strings.getSize();
   tableView->setColWidth(0, colWidth, 50);
   tableView->setRowHeights(lineCount, 20);
   tableView->setBackgroundColor(0, 0, 0, 0);
}

void MechShell::loadTexts()
{
   for (size_t i = 0; i < treeIter->getChildCount(); ++i) {
      treeIter->goToChild(i);
      BGString str = treeIter->getNodeName();
      if (str.equals ("scrollableText")) {
         const char* BWDFN = treeIter->getAttributeValueByName ("bwd");
         const char* x1str = treeIter->getAttributeValueByName("x1");
         const char* y1str = treeIter->getAttributeValueByName("y1");
         const char* x2str = treeIter->getAttributeValueByName("x2");
         const char* y2str = treeIter->getAttributeValueByName("y2");

         if (BWDFN != NULL && archive2 != NULL 
         && x1str != NULL && y1str != NULL && x2str != NULL && y2str != NULL) 
         {
            archive2->leaveDir();
            archive2->enterSubDir("BWD");

            BGString fullBWDFN;
            selectStr("mission,%Missions%", fullBWDFN);
            fullBWDFN += BWDFN;

            // Load text from BWD
            MemoryBlock mb;
            int fileNo = archive2->getEntryNumberByName(fullBWDFN.getChars());
            size_t fileSize = archive2->getFileSize(fileNo);

            if (fileSize > 60) {
               mb.resize(fileSize);
               archive2->getFileChunk(fileNo, 0, mb.getPtr(), fileSize);
               int x1 = atoi(x1str);
               int y1 = atoi(y1str);
               int x2 = atoi(x2str);
               int y2 = atoi(y2str);
               buildScrollableTextFromBWD (&mb, x1, y1, x2, y2);
            }
         }
      }
      treeIter->goToParent();
   }
}

////////////////////////////////////////////////////////////////////////////////
// Store variable in config

void MechShell::writeBackVar (const char* varName)
{
   int varIndex = findVar (varName);

   if (varIndex >= 0) {

      Config* config = getConfig();
      if (config != NULL) {
         const char* gameName = retrieveStr("GameName");

         size_t cfgLine = 0;
         bool found = false;

         // Find start of game definition in MechVM.cfg
         while (!found && cfgLine < config->getKeyCount()) {
            BGString key = config->getKey(cfgLine);
            BGString value = config->getValue(cfgLine);
            if (key.equals ("GameName")
            &&  value.equals(gameName))
            {
               found = true;
            } else
               ++cfgLine;
         }

         // Find variable in game definition
         if (found) {
            found = false;

            while (!found && cfgLine < config->getKeyCount()) {
               BGString key = config->getKey(cfgLine);
               BGString value = config->getValue(cfgLine);

               if (key.equals ("SetShellVar")
               &&  value.startsWith(varName))
               {
                  found = true;
                  const char* varValue = vars[varIndex+1].getChars();
                  BGString newVal = varName;
                  newVal += "=";
                  newVal += varValue;
                  config->setKeyValuePair(cfgLine, "SetShellVar", newVal.getChars());
               } else
                  ++cfgLine;
            }
         }
      }

   }
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::execute(const BGString& cmd)
{
   if (cmd.startsWith("FetchMEKs")) {
      fetchMEKs(cmd);
   } else if (cmd.equals("runSim")) {
      runSim();
   } else if (cmd.startsWith("MechLab")) {
      showMechLab(cmd.getChars());
   } else if (cmd.equals("exit")) {
      closeArchives();
      returnToMechVMMainWindow();
   } else if (cmd.startsWith("FetchMEKComment")) {
      fetchMEKComment(cmd);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Enter next room

bool MechShell::enterRoom()
{
   clearImages();

   if (tableView != NULL) {
      delete tableView;
      tableView = NULL;
   }
   strings.clear();
   stringSizes.clear();

   treeIter->goToRoot();
//   treeIter->goToChild(0);
   size_t i = 0;
   bool found = false;

   while (i < treeIter->getChildCount() && !found)
   {
      treeIter->goToChild(i);
      const char* roomName = treeIter->getAttributeValueByName("name");
      if (room.equals (roomName)) {
         found = true;
      } else {
         i++;
         treeIter->goToParent();
      }
   }

   if (found) {
      const char* backgroundInArchiveFN = treeIter->getAttributeValueByName("background");
      Texture* screen = loadImageFromArchive(backgroundInArchiveFN);
      if (screen != NULL) {
         MechShellImage* img = new MechShellImage();
         img->img = screen;
         img->x1 = 0;
         img->y1 = 0;
         img->x2 = screen->getWidth();
         img->y2 = screen->getHeight();
         images.add(img);
         screenSizeX = screen->getWidth();
         screenSizeY = screen->getHeight();
      } else {
         screenSizeX = 640;
         screenSizeY = 480;
      }

      loadImages();
      //fetchMEKs();
      loadTexts();
      printf ("Entered room %s\n", room.getChars());
   } else
      printf ("Room not found %s\n", room.getChars());

   // Write back variables
   BGString persistentVars = retrieveStr("persistent");
   if (persistentVars.getLength() != 0) {
      size_t i = 0, j = 1;

      while (i < j && j < persistentVars.getLength()) {
         if (persistentVars.findChar(',', j, persistentVars.getLength(), 1, j)) {
            BGString persistentVar;
            persistentVar.copyPart(persistentVars.getChars(), i, j);
            writeBackVar(persistentVar.getChars());
            i = j+1;
            j += 2;
         }
      }

      BGString persistentVar;
      persistentVar.copyPart(persistentVars.getChars(), i, persistentVars.getLength());
      writeBackVar(persistentVar.getChars());
   }

   return found;
}

////////////////////////////////////////////////////////////////////////////////
// Receive messages

void MechShell::receiveMessage (VirtualClass* /*sender*/, int /*MessageType*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Update title with model graph name and framerate

void MechShell::updateTitle ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Render

void MechShell::renderTextButton(int x, int y, int /*w*/, int /*h*/)
{
   int x1 = treeIter->getIntAttributeValueByName("x1");
   int y1 = treeIter->getIntAttributeValueByName("y1");
   int x2 = treeIter->getIntAttributeValueByName("x2");
   int y2 = treeIter->getIntAttributeValueByName("y2");
   BGString text = treeIter->getAttributeValueByName ("text");
   if (text.getLength() == 0) {
      // Integer selects a string
      const char* select = treeIter->getAttributeValueByName ("select");
      selectStr (select, text);
   } else {
      // Do not allow operators here, only get variables to prevent 
      // misinterpreting text with "-" as assignments
      evaluate (text, false);
   }

   const char* colorStr = treeIter->getAttributeValueByName ("color");
   Color color;
   parseColor (color, colorStr);
   color.glColor();
   int textW = textWidth (text.getChars());
   int maxLen = 640-x;
   if (textW > maxLen)
      textW = maxLen;
   x = (x1+x2)/2 - textW/2;
   y = (y1+y2)/2 - 7;
   //x = x + getWidth()/2 - screenSizeX/2;
   //y = y + getHeight()/2 - screenSizeY/2;
   renderText(text.getChars(), x, y, maxLen);
}

void MechShell::renderBar(int /*x*/, int /*y*/, int /*w*/, int /*h*/)
{
   int x1 = treeIter->getIntAttributeValueByName("x1");
   int y1 = treeIter->getIntAttributeValueByName("y1");
   int x2 = treeIter->getIntAttributeValueByName("x2");
   int y2 = treeIter->getIntAttributeValueByName("y2");

   const char* colorStr = treeIter->getAttributeValueByName ("color");
   Color color;
   parseColor (color, colorStr);
   color.glColor();

   glBegin(GL_POLYGON);
      glVertex2d(x1, y1);
      glVertex2d(x2, y1);
      glVertex2d(x2, y2);
      glVertex2d(x1, y2);
   glEnd();
}

void MechShell::render(int x, int y, int w, int h)
{
   if (sim != NULL) {
      delete sim;
      sim = NULL;
   }

   glClearColor (0.7f, 0.7f, 0.7f, 1.0f);
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Render images
   glViewport (getWidth()/2  - screenSizeX/2, getHeight()/2 - screenSizeY/2, 
               screenSizeX, screenSizeY);
   glDisable(GL_DEPTH_TEST);
   glEnable(GL_TEXTURE_2D);
   glColor3d(1, 1, 1);

   // Reset matrices in case we return from the sim
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
   glOrtho(0, screenSizeX, screenSizeY, 0, 0, 1);

   for (size_t i = 0; i < images.getSize(); ++i) {
      MechShellImage* img = images[i];
      img->img->use();

      glBegin(GL_POLYGON);
         img->img->glTexCoord(0, 1);
         glVertex2d (img->x1, img->y1);
         img->img->glTexCoord(1, 1);
         glVertex2d (img->x2, img->y1);
         img->img->glTexCoord(1, 0);
         glVertex2d (img->x2, img->y2);
         img->img->glTexCoord(0, 0);
         glVertex2d (img->x1, img->y2);
      glEnd();
   }

   // Render additional text
   glEnable (GL_BLEND);
   glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   for (size_t i = 0; i < treeIter->getChildCount(); i++) {
      treeIter->goToChild(i);
      BGString str = treeIter->getNodeName();
      if (str.equals ("textBtn")) {
         renderTextButton(x, y, w, h);
      } else if (str.equals("bar")) {
         renderBar (x, y, w, h);
      }
      treeIter->goToParent();
   }
   glDisable (GL_BLEND);


   if (tableView != NULL)
      tableView->render(0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////////////
// Keyboard event handler

void MechShell::keypressed(char /*key*/, bool /*shiftHeld*/, bool /*controlHeld*/)
{
}

////////////////////////////////////////////////////////////////////////////////
// Button state changed over a window: returns child that was clicked

GLWindow* MechShell::buttonStateChanged (int button, bool down, int x, int y)
{
   if (button == 1 && !down) {
      x = x - getWidth()/2 + screenSizeX/2;
      y = y - getHeight()/2 + screenSizeY/2;
      printf("x1=\"%i\" y1=\"%i\"\n", x, y);
      const XMLTreeNode* treeNode = NULL;

      for (size_t i = 0; i < treeIter->getChildCount(); i++) {
         treeIter->goToChild(i);
         int x1 = treeIter->getIntAttributeValueByName("x1");
         int y1 = treeIter->getIntAttributeValueByName("y1");
         int x2 = treeIter->getIntAttributeValueByName("x2");
         int y2 = treeIter->getIntAttributeValueByName("y2");

         if (x1 <= x && y1 <= y && x <= x2 && y <= y2) {
            treeNode = treeIter->getTreeNode();
         }
         treeIter->goToParent();
      }
   
      if (treeNode != NULL) {
         // Update variables
         for (size_t i = 0; i < treeNode->getAttributeCount(); i++) {
            const char* attr = treeNode->getAttributeName(i);
            if (strcmp (attr, "setvar") == 0) {
               const char* assignment = treeNode->getAttributeValueByIndex(i);
               setVar (assignment, true);
            } else if (strcmp (attr, "setstr") == 0) {
               const char* assignment = treeNode->getAttributeValueByIndex(i);
               setVar (assignment, false);
            } else if (strcmp (attr, "range") == 0) {
               const char* rangeCheckStr = treeNode->getAttributeValueByIndex(i);
               rangeCheck (rangeCheckStr);
            } else if (strcmp (attr, "exec") == 0) {
               BGString cmd = treeNode->getAttributeValueByIndex(i);
               execute(cmd);
               //return NULL; // Must leave without treeIter->goToParent() or loose room ..
            } else if (strcmp (attr, "goto") == 0) {
               room = treeNode->getAttributeValueByIndex(i);
               evaluate(room, true);
               enterRoom();
            }
         }
      }

   }

   return GLWindowContainer::buttonStateChanged(button, down, x, y);
}

void MechShell::mouseMoved (int /*x*/, int /*y*/)
{
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::getDosBoxGameDir (BGString& gameDir)
{
   int index = findVar ("GamePathInDosBox");
   if (index >= 0) {
      gameDir = getExecDir();
      gameDir += OSPathSep;
      gameDir += "games";
      gameDir += OSPathSep;
      gameDir += vars[index+1].getChars();
   }
}

////////////////////////////////////////////////////////////////////////////////
// Variables

void MechShell::setStr (const char* varName, const char* value)
{
   int index = findVar(varName);
   if (index >= 0) {
      vars[index+1] = value;
   } else {
      // Add new variable
      vars.push_back(BGString(varName));
      vars.push_back(BGString(value));
   }
}

bool MechShell::setVar (const char* varName, const char* value)
{
   int i = findVar(varName);

   if (i >= 0) {
      vars[i+1] = value;
   } else {
      // Add new variable
      vars.push_back(BGString(varName));
      vars.push_back(BGString(value));
   }

   return true;
}

bool MechShell::setVar (const char* assignment, bool doEvaluate)
{
   // Parse assignment
   size_t pos;
   size_t len = bgstrlen (assignment);
    
   if (bgstrscan (assignment, '=', 0, len, pos)) {
      BGString varName, val;
      varName.copyPart(assignment, 0, pos);
      val.copyPart(assignment, pos+1, len);
      if (doEvaluate)
         evaluate(val, true);
      setVar(varName.getChars(), val.getChars());
      return true;
   } else
      return false;   
}

bool MechShell::checkCondition (const char* condition)
{
   size_t pos;
   size_t len = strlen (condition);

   if (bgstrscan(condition, '=', 0, len, pos)) {
      // Check for equality
      BGString str1, str2;
      str1.copyPart (condition, 0, pos);
      str2.copyPart (condition, pos+1, len);
      evaluate (str1, true);
      evaluate (str2, true);
      return str1.equals(str2.getChars());
   } else if (bgstrscan(condition, '<', 0, len, pos)) {
      // Check for less
      BGString str1, str2;
      str1.copyPart (condition, 0, pos);
      str2.copyPart (condition, pos+1, len);
      evaluate (str1, true);
      evaluate (str2, true);
      double d1, d2;
      if (str1.toDouble(d1) && str2.toDouble(d2))
         return (d1 < d2);
   } else if (bgstrscan(condition, '>', 0, len, pos)) {
      // Check for bigger
      BGString str1, str2;
      str1.copyPart (condition, 0, pos);
      str2.copyPart (condition, pos+1, len);
      evaluate (str1, true);
      evaluate (str2, true);
      double d1, d2;
      if (str1.toDouble(d1) && str2.toDouble(d2))
         return (d1 > d2);
   }

   return false; // Error - no comparison?
}

bool MechShell::evaluate (BGString& str, bool opsAllowed)
{
   size_t pos, pos2;

   // Substitute variables
   while (bgstrscan(str.getChars(), '%', 0, str.getLength(), pos)) {
      if (bgstrscan(str.getChars(), '%', pos+1, str.getLength(), pos2)) {
         BGString other, var;
         other.copyPart(str.getChars(), 0, pos);
         var.copyPart (str.getChars(), pos+1, pos2);
         other += retrieveStr(var.getChars());
         other.appendPart(str.getChars(), pos2+1, str.getLength());
         str = other;
      } else
         return false; // unmatched "%" ...
   }

   // Add variables?
   if (opsAllowed
   &&  bgstrscan(str.getChars(), '+', 0, str.getLength(), pos)
   &&  str.getLength() > 1) 
   {
      BGString aStr, bStr;
      aStr.copyPart(str.getChars(), 0, pos);
      bStr.copyPart(str.getChars(), pos+1, str.getLength());
      int a = atoi (aStr.getChars());
      int b = atoi (bStr.getChars());
      str.assignInt(a+b);
   }

   // Subtract
   if (opsAllowed
   &&  bgstrscan(str.getChars(), '-', 1, str.getLength(), pos)
   &&  str.getLength() > 1) 
   {
      BGString aStr, bStr;
      aStr.copyPart(str.getChars(), 0, pos);
      bStr.copyPart(str.getChars(), pos+1, str.getLength());
      int a = atoi (aStr.getChars());
      int b = atoi (bStr.getChars());
      str.assignInt(a-b);
   }

   return true;
}

bool MechShell::rangeCheck (const char* str)
{
   size_t pos, pos2;
   size_t len = strlen(str);

   if (bgstrscan(str, ',', 0, len, pos)
   &&  bgstrscan(str, ',', pos+1, len, pos2)) 
   {
      BGString aStr, bStr, cStr;
      aStr.copyPart(str, 0, pos);
      bStr.copyPart(str, pos+1, pos2);
      cStr.copyPart(str, pos2+1, len);

      evaluate(aStr, true);
      evaluate(cStr, true);

      double a, b, c;
      aStr.toDouble(a);
      cStr.toDouble(c);

      int index = findVar (bStr.getChars());
      if (index >= 0) {
         if (vars[index+1].toDouble(b)) {
            if (b < a)
               vars[index+1] = aStr;
            else if (b > c)
               vars[index+1] = cStr;
         }
      }
   }

   return true;
}

// Return index of variable, or -1
int MechShell::findVar (const char* name) const
{
   size_t i = 0;
   while (i < vars.size()) {
      const BGString& var = vars[i];
      if (var.equals(name)) {
         //printf ("Found %s\n", name);
         return i;
      } else {
         //printf ("%s != %s\n", name, vars[i].getChars());
         i += 2;
      }
   }

   return -1;
}

// Select a string from a comma-separated list where the first entry gives the index
bool MechShell::selectStr (const char* select, BGString& result)
{
   if (select == NULL)
      return false;

   size_t pos1 = 0, pos2 = 0, len = strlen(select), count = 0;

   BGString selectStr = select;
   evaluate(selectStr, false);

   while (pos1 < selectStr.getLength()) {
      char c;
      if (selectStr.getChar(pos1, c) )
         if (c == ',')
            ++count;
      ++pos1;
   }

   if (!bgstrscan(selectStr.getChars(), ',', 0, len, pos1)) 
      return false;
   BGString numStr;
   numStr.copyPart(selectStr.getChars(), 0, pos1);

   int index = findVar (numStr.getChars());
   if (index == -1)
      return false;
   int num = vars[index+1].toInt();
   num %= count;
   ++pos1;

   while (pos1 < selectStr.getLength()) {
      if (!bgstrscan(selectStr.getChars(), ',', pos1, selectStr.getLength(), pos2)) {
         pos2 = selectStr.getLength();
      }
      if (num == 0) {
         // Found!
         result.copyPart(selectStr.getChars(), pos1, pos2);
         return true;
      } else {
         pos1 = pos2+1;
         --num;
      }
   }

   result = "";
   return false;
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::fetchMEKs(const BGString& args)
{
   size_t pos1, pos2;
   if (!args.findChar(',', pos1)
   ||  !args.findChar(',', pos1+1, args.getLength(), 1, pos2))
   {
      return;
   }
   BGString chassisNumVar, chassisListVar, configVarName;
   chassisNumVar.copyPart(args.getChars(), 10, pos1);
   chassisListVar.copyPart(args.getChars(), pos1+1, pos2);
   configVarName.copyPart(args.getChars(), pos2+1, args.getLength()-1);

   BGString chassis, selStr;
   selStr = chassisNumVar;
   selStr += ",%";
   selStr += chassisListVar;
   selStr +="%";
   selectStr(selStr.getChars(), chassis);

   // Check MEK subfolder
   bool missionSuccessful = false;
   BGString configs;
   BGString methodStr = retrieveStr("SimType");

   if (methodStr.equals("internal")) {
      BGFilePath<TCHAR> MEKdir = getExecDir();
      MEKdir += getConfig()->getMechPath();
      MEKdir += chassis.getChars();
      BGFilePath<TCHAR> mask = MEKdir;
      mask += "*.MEK";

      FileList fl;
      if (fl.start(mask.getChars())) {
         if (configs.getLength() > 0)
            configs += ",";
         //configs += MEKdir;
         //configs += OSPathSep;
         configs += fl.getFileName();

         while (++fl) {
            configs += ",";
            configs += fl.getFileName();
         }
      }
   } else {
      if (archive2 == NULL) {
         printf("MW2.PRJ not open - can't fetch configs\n");
         return;
      }

      if (!archive2->enterSubDir("MEK")) {
         printf ("MW2.PRJ without MEK dir? Weird!\n");
         return;
      }

      // Check all mech configs in MW2.PRJ
      for (size_t i = 0; i < archive2->getEntryCount(); ++i) {
         const int bufSize = 20;
         char buf[bufSize+1];
         archive2->getEntryName(i, buf, bufSize);
         BGString tmp = buf;
         if (tmp.startsWith(chassis.getChars())) {
            if (configs.getLength() > 0)
               configs += ",";
            configs += buf;
         }
      }

      BGFilePath<TCHAR> mask = retrieveStr("MEKdir");
      chassis += "*.mek";
      mask += chassis.getChars();

      FileList fl;
      if (fl.start(mask.getChars())) {
         if (configs.getLength() > 0)
            configs += ",";
         configs += fl.getFileName();

         while (++fl) {
            configs += ",";
            configs += fl.getFileName();
         }
      }
   }

   setStr(configVarName.getChars(), configs.getChars());
   //printf("Configs: %s\n", configs.getChars());
}

////////////////////////////////////////////////////////////////////////////////

void MechShell::fetchMEKComment(const BGString& cmd)
{
   size_t pos1, pos2;
   if (!cmd.findChar(',', pos1)
   ||  !cmd.findChar(',', pos1+1, cmd.getLength(), 1, pos2))
   {
      return;
   }

   BGString configVarName, configs, commentVar;
   configVarName.copyPart(cmd.getChars(), 16, pos1);
   configs.copyPart(cmd.getChars(), pos1+1, pos2);
   commentVar.copyPart(cmd.getChars(), pos2+1, cmd.getLength()-1);

   bool found = false;
   MemoryBlock mb;

   BGString MEKFN;
   BGString select = configVarName;
   select += ",";
   select += configs;
   selectStr(select.getChars(), MEKFN);

   // Check if file can be found in file system's MEK directory
   BGFilePath<TCHAR> fn = retrieveStr("MEKdir");
   fn += MEKFN.getChars();
   found = mb.loadFromFile(fn.getChars());

   if (!found && archive2 != NULL) {
      archive2->leaveDir();
      archive2->enterSubDir("MEK");
      size_t findex = archive2->getEntryNumberByName(MEKFN.getChars());
      size_t fsize = archive2->getFileSize(findex);
      if (fsize > 0) {
         mb.resize(fsize);      
         archive2->getFileChunk(findex, 0, mb.getPtr(), fsize);
         found = true;
      }
   }

   if (!found) {
      BGString chassis, configVarName;
      selectStr("chassis,%mechlist%", chassis);

      BGFilePath<TCHAR> FullFN = getExecDir();
      FullFN += getConfig()->getMechPath();
      FullFN += chassis.getChars();
      FullFN += MEKFN.getChars();
      found = mb.loadFromFile(FullFN.getChars());
   }

   if (mb.getSize() > 0x158) {
      DWORD weaponCount = mb.getDWord(0x10);
      DWORD ammoCount = mb.getDWord(0x14);
      const unsigned char* comment = mb.getPtr(0x158+8*weaponCount+8*ammoCount);
      setVar(commentVar.getChars(), (const char*) comment);
   }
}

////////////////////////////////////////////////////////////////////////////////

Vehicle* MechShell::loadPlayerVehicle(const char* MEKname, const char* chassisName)
{
   Config* config = getConfig();
   BGString MVMKFNpart, BMPpart;
   BGFilePath<TCHAR> MVMKdir, MEKFN, MEKdir, MVMKmask, BMPmask, MVMKFN, BMPFN;

   MVMKdir = getExecDir();
   MVMKdir += config->getMechPath();
   MVMKdir += chassisName;

   // If a MEKdir variable is set, load mech config file from that location,
   // otherwise, load it from MVMKdir
   MEKdir = retrieveStr ("MEKdir");
   MEKFN = MEKdir;
   if (MEKFN.getLength() == 0) {
      MEKFN = MVMKdir;
   }
   MEKFN += MEKname;

   BGString MechName = chassisName;
   MechName += " - ";
   MechName += MEKname;

   MVMKmask = MVMKdir;
   MVMKmask += "*.MVMK";
   FileList::findFirstMatch(MVMKmask.getChars(), MVMKFNpart);
   MVMKFN = MVMKdir;
   MVMKFN += MVMKFNpart.getChars();

   BMPmask = MVMKdir;
   BMPmask += "*.BMP";
   FileList::findFirstMatch(BMPmask.getChars(), BMPpart);
   BMPFN = MVMKdir;
   BMPFN += BMPpart.getChars();

   Vehicle* vehicle = loadVehicle(MechName.getChars(), MVMKFN.getChars(),
      MVMKdir.getChars(), BMPFN.getChars());

   if (vehicle != NULL) {
      vehicle->typeName.set(MechName);
      vehicle->MVMKFN.set(MVMKFN);
      vehicle->textureFN.set(BMPFN);
   }

   if (vehicle != NULL) {
      if (!fileExists(MEKFN.getChars()) && archive2 != NULL) {
         // MEK not stored as file: Load MEK file from MW2.PRJ
         archive2->leaveDir();
         archive2->enterSubDir("MEK");
         size_t fileNo = archive2->getEntryNumberByName (MEKname);
         size_t fileSize = archive2->getFileSize(fileNo);
         bool error = true;

         if (fileSize > 0) {
            MemoryBlock mb;
            mb.resize(fileSize);
            if (archive2->getFileChunk(fileNo, 0, mb.getPtr(), fileSize)) {
               // Feed the mech config to the Mech Lab ...
               vehicle->loadFromMW2MEK(&mb);
            }
            error = false;
         }

         if (error)
            printf ("Failed to load %s\n", MEKname);
      } else {
         vehicle->loadConfig(MEKFN.getChars());
      }
   }

   return vehicle;
}

////////////////////////////////////////////////////////////////////////////////
// Show Mech Lab

void MechShell::showMechLab(const char* cmd)
{
   int screens = MechLab_outfitMechButtonID | MechLab_equipmentButtonID | 
      MechLab_weaponGroupsID | MechLab_exitButtonID;
   MechLab* lab = new MechLab(screens, this);
   lab->setGeometry(0, 0, getWidth(), getHeight());
   setCurVis (lab);
   lab->setReturnVis (this);

   BGString MEKname, chassisName, MVMKdir, MVMKmask, MVMKFN, 
      MVMKFNpart, BMPmask, BMPpart, BMPFN, MEKprefix;
   BGString MEKdir = retrieveStr("MEKdir");
   BGFilePath<TCHAR> cfgFN = MEKdir.getChars();

   // Retrieve arguments
   BGString args = cmd, arg1, arg2, arg3, arg4, arg5;
   size_t pos1, pos2, c1, c2, c3, c4;
   if (args.findChar('(', pos1) 
   &&  args.findChar(')', pos2)
   &&  bgstrscan(cmd, ',', pos1, pos2, c1)
   &&  bgstrscan(cmd, ',', c1+1, pos2, c2)
   &&  bgstrscan(cmd, ',', c2+1, pos2, c3)
   &&  bgstrscan(cmd, ',', c3+1, pos2, c4)) 
   {
      BGString qstr1, qstr2, qstr3;

      qstr1.appendPart(cmd, pos1+1, c1);
      qstr1 += ",%";
      qstr1.appendPart(cmd, c1+1, c2);
      qstr1 += "%";
      selectStr(qstr1.getChars(), MEKname);

      qstr2.appendPart(cmd, c2+1, c3);
      qstr2 += ",%";
      qstr2.appendPart(cmd, c3+1, c4);
      qstr2 += "%";
      selectStr(qstr2.getChars(), MEKprefix);

      qstr3.appendPart(cmd, c2+1, c3);
      qstr3 += ",%";
      qstr3.appendPart(cmd, c4+1, pos2);
      qstr3 += "%";
      selectStr(qstr3.getChars(), chassisName);
   }

   // Read data
   Vehicle* playerVehicle = loadPlayerVehicle (MEKname.getChars(), chassisName.getChars());
   if (playerVehicle != NULL) {
      cfgFN += MEKname.getChars();
      lab->selectMEKFNInGameDirectory(MEKdir.getChars(), MEKprefix.getChars());
      lab->showMech(playerVehicle);
      lab->showOutfitMechControls();

      const char* MEKFileVersion = retrieveStr("MEKFileVersion");
      int MEKformat = 0;
      if (MEKFileVersion != NULL)
         MEKformat = atoi(MEKFileVersion);
      lab->setMEKformat(MEKformat);
   }

   BGString mechLabType;
   if (!selectStr("mission,%weaponsLockers%", mechLabType))
      mechLabType = retrieveStr("weaponsLocker");
   lab->addWeapons(mechLabType);
}

////////////////////////////////////////////////////////////////////////////////
// Called by Mech Lab to indicate that user saved a new MEK file and
// intends to use it

void MechShell::useConfig(const TCHAR* MEKFN)
{
   if (MEKFN == NULL)
      return;

   size_t pos;
   size_t len = strlen(MEKFN);

   if (len < 3)
      return;

   BGString MEKFNpart = MEKFN;
   if (bgstrrscan(MEKFN, OSPathSep, len-1, 0, pos))
      MEKFNpart.copyPart(MEKFN, pos+1, len);
   else 
      MEKFNpart = MEKFN;

   int index = findVar ("configs");
   BGString MEKlist = vars[index+1];
   MEKlist += ",";
   MEKlist += MEKFNpart;
   vars[index+1] = MEKlist;

   size_t MEKcount = MEKlist.count(',')+1;
   index = findVar ("config");
   vars[index+1].assignInt(MEKcount-1);
}

////////////////////////////////////////////////////////////////////////////////

bool MechShell::runInternalSim(bool& missionSuccessful)
{
   if (sim == NULL)
      sim = new MechSim();
   MechSim* mechSim = (MechSim*) sim;
   setCurVis (sim);
   mechSim->setReturnVis(this);

   BGString MEKname, chassisName;
   selectStr("config,%configs%", MEKname);
   selectStr("chassis,%mechlist%", chassisName);
   Vehicle* playerVehicle = loadPlayerVehicle(MEKname.getChars(), chassisName.getChars());

   if (playerVehicle != NULL) {
      mechSim->addVehicle(playerVehicle);
      PlayerBipedMech* player = mechSim->setPlayerMech((BipedMech*) playerVehicle);
      mechSim->placeVehicleRandomly(playerVehicle);

      if (player != NULL) {
         player->setHeightfield(mechSim->getHeightfield());
      }

      return true;
   } else
      return false;
}

#ifdef _WIN32

bool MechShell::setChassisAndConfig (MemoryBlock& userStarBWD, size_t& offset, 
                                     const char* selChassisStr, const char* selMEKStr)
{
   if (archive2 != NULL) {
      BGString MEKname;
      selectStr(selMEKStr, MEKname);

      if (MEKname.getLength() == 0)
         return false;

      // Try finding the file in MEKdir first - GBL's MW2.PRJ contains a BHM00USR.MEK ...
      BGFilePath<TCHAR> mekFN = retrieveStr ("MEKdir");
      mekFN += MEKname.getChars();
      if (fileExists(mekFN.getChars())) {
         userStarBWD.setWord(offset+8, 0xfffe);
      } else {
         archive2->enterSubDir("MEK");
         size_t MEKnum = archive2->getEntryNumberByName(MEKname.getChars());
         userStarBWD.setWord(offset+8, MEKnum);
      }

      userStarBWD.copy(offset + 0x2D, (const unsigned char*) MEKname.getChars(), 
         MEKname.getLength()-4);

      BGString BWDname;
      selectStr(selChassisStr, BWDname);

      if (BWDname.getLength() == 0)
         return false;

      BWDname += ".BWD";
      archive2->enterSubDir("BWD");
      size_t BWDnum = archive2->getEntryNumberByName(BWDname.getChars());

      if (BWDnum >= archive2->getEntryCount()) {
         return false;
      }
      userStarBWD.setWord(offset + 0xa, BWDnum);
      userStarBWD.copy(offset + 0x24, (const unsigned char*) BWDname.getChars(), 
         BWDname.getLength()-4);

      offset += 0x64;

      return true;
   } else
      return false;
}

bool MechShell::runExternalSim(bool useDosBox, bool& missionSuccessful)
{
   BGFilePath<TCHAR> gameDir, userStarFN;

   // Create userStar.BWD
   BGFilePath<TCHAR> userStarSourceFN = getExecDir();
   userStarSourceFN += "campaigns";
   userStarSourceFN += "USERSTAR3.BWD";
   size_t offset = 0x34;
   MemoryBlock userStarBWD;
   if (!userStarBWD.loadFromFile(userStarSourceFN.getChars())) {
      printf ("Warning - unable to create %s\n", userStarSourceFN.getChars());
      return false;
   }

   if (!setChassisAndConfig (userStarBWD, offset, "chassis,%BWDName%", 
                             "config,%configs%"))
   {
      printf ("Illegal Mech chassis choice\n");
      return false;
   }

   setChassisAndConfig (userStarBWD, offset, "sm1_chassis,%SM_BWDName%", 
      "sm1_config,%sm1_configs%");
   setChassisAndConfig (userStarBWD, offset, "sm2_chassis,%SM_BWDName%", 
      "sm2_config,%sm2_configs%");
   userStarBWD.resize(offset);
   userStarBWD.setDWord(4, offset);

   // Close MW2.PRJ to allow the game to access the file
   delete archive2;
   archive2 = NULL;

   // Create .BAT file for running DosBox - calling DosBox with a parameter to 
   // run another program from inside DosBox is ok, but that program won't receive
   // further parameters, so "dosbox mw2 gree" won't launch into mission "gree"
   BGString missionName, cmdLine, exeDir;
   BGString missionNameSelector = "mission,";
   int index = findVar("Missions");
   if (index >= 0) {
      missionNameSelector += vars[index+1];
      selectStr(missionNameSelector.getChars(), missionName);
   }

   if (useDosBox) {
      getDosBoxGameDir (gameDir);
      userStarFN = gameDir.getChars();
      userStarFN += "USERSTAR.BWD";
      index = findVar ("GamePathInDosBox");
      const TCHAR* GamePathInDosBox = vars[index+1].getChars();

      // Choose command line
      cmdLine = "\"";
      getDosBoxPath(exeDir);
      cmdLine += exeDir.getChars();
      cmdLine += OSPathSep;
      cmdLine += "DosBox.exe\" -conf \"";
      cmdLine += exeDir.getChars();
      cmdLine += OSPathSep;
      cmdLine += "DosBox-0.74.conf\" -c \"";
   } else {
      index = findVar ("GamePath");
      if (index >= 0) {
         gameDir = vars[index+1].getChars();
         printf("Using gamedir=%s\n", gameDir.getChars());
      } else {
         printf ("GamePath variable not found - aborting\n");
         return false;
      }
      userStarFN = gameDir.getChars();
      userStarFN += "USERSTAR.BWD";
      printf("Using UserStarFN=%s\n", userStarFN.getChars());

      index = findVar ("GamePath");
      if (index >= 0)
         exeDir = vars[index+1];
   }

   index = findVar ("SimExec");
   if (index >= 0) {
      cmdLine += vars[index+1];
      cmdLine += " ";
      cmdLine += missionName.getChars(); 
      cmdLine += "SCN1";
      if (useDosBox) 
         cmdLine += "\"";
   } else
      printf ("Executable name missing - aborting\n");

   userStarBWD.saveToFile(userStarFN.getChars());

   // Create DosBox process
   BOOL processCreated;
   PROCESS_INFORMATION processInfo;
   char* cmdLineChars = NULL;
   if (cmdLine.getLength() > 0) {
      ZeroMemory(&processInfo, sizeof(processInfo));
      STARTUPINFO startInfo;
      ZeroMemory(&startInfo, sizeof(startInfo));
      startInfo.cb = sizeof(startInfo);
      cmdLineChars = new char[cmdLine.getLength()+1];
      memcpy (cmdLineChars, cmdLine.getChars(), cmdLine.getLength()+1);
      printf("Invoking %s\n", cmdLineChars);
      processCreated = 
         CreateProcess (NULL, cmdLineChars, NULL, NULL, false, 
                        NORMAL_PRIORITY_CLASS, NULL, exeDir.getChars(), 
                        &startInfo, &processInfo);
   }

   if (processCreated) {
      // Wait for decompressor to complete
      DWORD exitCode;
      do {
         if (!GetExitCodeProcess(processInfo.hProcess, &exitCode)) {
            displayMsg ("Unable to obtain process status for DosBox");
            return false;
         }
         Sleep(25);
      } while (exitCode == STILL_ACTIVE);

      printf ("Process ended with code %i\n", exitCode);
   } else {
      BGString str ="Invocation failed: ";
      str += cmdLine;
      #ifdef _WIN32
      str.appendOSError();
      #endif
      displayMsg (str.getChars());
   }

   // Check mission results
   BGFilePath<TCHAR> resultsFN = gameDir;
   resultsFN += "mw2msn.cfg";
   MemoryBlock mb;
   missionSuccessful = false;

   if (mb.loadFromFile(resultsFN.getChars())) {
      DWORD objectivesCount = mb.getDWord(4);
      missionSuccessful = true;
      for (DWORD i = 0; i < objectivesCount; ++i) {
         if (mb.getDWord(36+52*i) == 1) { // Primary objective
            if (mb.getDWord(32+52*i) == -1) {
               missionSuccessful = false;
            }
         }
      }
   }

   loadArchive (archive2name.getChars());

   if (cmdLineChars != NULL)
      delete cmdLineChars;
   return true;
}

#else
bool MechShell::runExternalSim(bool useDosBox, bool& missionSuccessful)
{
  printf ("Running an external simulation is not currently supported under Linux\n");
  return false;
}
#endif

bool MechShell::runSim ()
{
   const char* methodStr = NULL;
   int startMethod = 0;

   int index = findVar("SimType");
   bool missionSuccessful = false;
   bool startCorrectly = false;
   if (index >= 0) {
      methodStr = vars[index+1].getChars();

      if (strcmp(methodStr, "DosBox") == 0)
         startCorrectly = runExternalSim (true, missionSuccessful);
      else if (strcmp(methodStr, "native") == 0)
         startCorrectly = runExternalSim (false, missionSuccessful);
      else if (strcmp(methodStr, "internal") == 0)
         startCorrectly = runInternalSim(missionSuccessful);
      else
         displayMsg ("Unsupported SimType");

   } else
      displayMsg ("Missing SimType");

   if (startCorrectly) {
      if (missionSuccessful) {
         int index = findVar("SuccessRoom");
         room = vars[index+1];
      } else {
         int index = findVar("FailureRoom");
         room = vars[index+1];
      }
      enterRoom();
   } //else
     // treeIter->goToParent();

   return false;
}

