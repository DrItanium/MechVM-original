////////////////////////////////////////////////////////////////////////////////
// Implementation of configuration class for MechVM
// Copyright Björn Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#include "Config.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////
// Constructor

Config::Config()
: ViewerBackgroundColor (1, 1, 1), 
  inactiveModeButtonColor (0.33, 0.33, 0.33), 
  ActiveModeButtonColor (0.66, 0.66, 0.66),
  m_experimentalFeaturesEnabled (false)
{
   MechPath = getDataDir();
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

Config::~Config()
{
}

////////////////////////////////////////////////////////////////////////////////
// Logging

/*class LogToScreenAndFile: public abstractLog {
private:
   FileCache* logFile;
public:
   LogToScreenAndFile ()
      : logFile (NULL)
   {
   }

   ~LogToScreenAndFile ()
   { delete logFile; }

   void open (const char* fileName)
   {
      logFile = new FileCache ();
      if (logFile->open (fileName))
         logFile->isReadOnly = false;
      else {
         delete logFile;
         logFile = NULL;
      }
   }

   virtual void log (const char* str)
   { 
      if (logFile != NULL) {
         logFile->append (str);
         logFile->flush();
      }
      printf (str);
   }

   virtual void log (const BGString& str)
   { 
      if (logFile != NULL) {
         logFile->append (str); 
         logFile->flush();
      }
      printf (str.getChars());
   }

   virtual void logLn (const BGString& str)
   { 
      log (str); 
      log ("\n"); 
      printf (str.getChars());
      printf ("\n");
   }
};*/

////////////////////////////////////////////////////////////////////////////////
// Read configuration

bool Config::readFromFile (const char* FileName)
{
   FileCache fc;
   ConfigFN = FileName;

   if (fc.openReadOnly (FileName)) {
      BGString line;
      FileOffset offset = 0;

      while (fc.readLine (offset, line)) {
         size_t endOfFirstWord; 
         line.findChar (' ', endOfFirstWord);
         BGString key, value;
         key.copyPart(line.getChars(), 0, endOfFirstWord);
         value.copyPart(line.getChars(), endOfFirstWord+1, line.getLength());
         keys.push_back(key);
         values.push_back (value);

         if (key.equals ("ViewerBackgroundColor")) {
            parseColor (ViewerBackgroundColor, value.getChars());
         } else if (key.equals ("inactiveModeButtonColor")) {
            parseColor (inactiveModeButtonColor, value.getChars());
         } else if (key.equals ("TextColor")) {
            parseColor (TextColor, value.getChars());
         } else if (key.equals ("ActiveModeButtonColor")) {
            parseColor (ActiveModeButtonColor, value.getChars());
         } else if (key.equals ("MechPath")) {
            MechPath = value;
         } else if (key.equals ("Experimental_Features")) {
            m_experimentalFeaturesEnabled = true;
         }
      }

      return true;
   } else 
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Access config

Config* cfg;

Config* getConfig()
{
   return cfg;
}

Config* createConfig()
{
   cfg = new Config();
   return cfg;
}

////////////////////////////////////////////////////////////////////////////////
// Get/set information on installed games

void Config::addKeyValuePair (const TCHAR* key, const TCHAR* value)
{
   keys.push_back(BGString(key));
   values.push_back(BGString(value));

   // Write back changes to file
   FileCache fc;
   fc.openReadWrite (ConfigFN.getChars());
   BGString line = key;
   if (strlen(key) > 0 && strlen(value) > 0)
      line += " ";
   line += value;
   line += newLine;
   fc.append(line);
}

////////////////////////////////////////////////////////////////////////////////

const TCHAR* Config::getValue (const TCHAR* key)
{
   size_t i = 0;
   const TCHAR* result = NULL;
   bool doContinue;

   do {
      BGString& keyI = keys[i];
      if (keyI.equals (key)) {
         doContinue = false;
         result = values[i].getChars();
      } else {
         i++;
         doContinue = (i < keys.size());
      }
   } while (doContinue);

   return result;
}

////////////////////////////////////////////////////////////////////////////////
// Find first line after given line that contains str

int Config::FindFirstKeyAfterLine (int line, const char* str)
{
   while (line < (int) getKeyCount()) {
      if (strcmp(getKey(line), str) == 0) {
         return line;
      } else
         ++line;
   }

   return -1;
}

////////////////////////////////////////////////////////////////////////////////

void addKeyValuePair (const TCHAR* key, const TCHAR* value)
{
   Config* cfg = getConfig();
   cfg->addKeyValuePair(key, value);
}

////////////////////////////////////////////////////////////////////////////////
// Log string, either to screen or to screen and file

/*void Config::write2log (const char* str)
{
   if (logFile != NULL) {
      logFile->write (logFile->getFileSize(), str, strlen (str));
   } 
   printf (str);
}*/

// Change key/value pair
void Config::setKeyValuePair (size_t i, const TCHAR* key, const TCHAR* value)
{
   if (i < keys.size()) {
      keys[i] = key;
      values[i] = value;

      // Write back changes to file
      BGString cfgStr;
      for (size_t i = 0; i < keys.size(); ++i) {
         BGString key = keys[i];
         BGString value = values[i];
         cfgStr += key;
         cfgStr += " ";
         cfgStr += value;
         cfgStr += "\n";
      }
      cfgStr.saveToFile (ConfigFN.getChars());
   }
}


bool Config::hasKeyValuePair (const char* key, const char* value)
{
   bool found = false;

   for (size_t i = 0; i < keys.size(); ++i) {
      if (keys[i].equals(key)
      &&  values[i].equals(value))
      {
         found = true;
      }
   }

   return found;
}

int Config::findValue (const char* value)
{
   bool found = false;

   for (size_t i = 0; i < keys.size(); ++i) {
      if (values[i].equals(value))
      {
         return i;
      }
   }

   return -1;
} 