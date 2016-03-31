////////////////////////////////////////////////////////////////////////////////
// Declaration of configuration class for plab
// Copyright Björn Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef plabConfig__H
#define plabConfig__H

#include "FileCache.h"
#include "Color.h"
#include "vector"
using namespace std;

////////////////////////////////////////////////////////////////////////////////

class Config {
public:
   // Constructor
   Config();

   // Destructor
   virtual ~Config();

   // Read configuration
   bool readFromFile (const char* FileName);

   // Get settings
   inline Color& getViewerBackgroundColor()
   { return ViewerBackgroundColor; }
   inline Color& getInactiveModeButtonColor()
   { return inactiveModeButtonColor; }
   inline Color& getActiveModeButtonColor()
   { return ActiveModeButtonColor; }
   inline Color& getTextColor()
   { return TextColor; }
   inline const TCHAR* getMechPath() const
   { return MechPath.getChars(); }
   inline bool experimentalFeaturesEnabled() const
   { return m_experimentalFeaturesEnabled; }

   // Add new pair of key and values
   void addKeyValuePair (const TCHAR* key, const TCHAR* value); 

   // Get keys and values
   const TCHAR* getValue (const TCHAR* key);
   const TCHAR* getValue (size_t i)
   { return values[i].getChars(); }
   const TCHAR* getKey (size_t i)
   { return keys[i].getChars(); }

   // Find first line after given line that contains str
   int FindFirstKeyAfterLine (int line, const char* str);

   // Get number of key/value pairs
   size_t getKeyCount() const
   { return keys.size(); }

   // Change key/value pair
   void setKeyValuePair (size_t i, const TCHAR* key, const TCHAR* value);

   // Queries
   bool hasKeyValuePair (const char* key, const char* value);
   int findValue (const char* value);

private:
   Color ViewerBackgroundColor, inactiveModeButtonColor, ActiveModeButtonColor,
         TextColor;
   BGString MechPath, ConfigFN;
   vector <BGString> keys, values;
   bool m_experimentalFeaturesEnabled;
};

////////////////////////////////////////////////////////////////////////////////
// Access config

Config* getConfig();
Config* createConfig();
void addKeyValuePair (const TCHAR* key, const TCHAR* value);

inline bool experimentalFeaturesEnabled ()
{ return getConfig()->experimentalFeaturesEnabled(); }

#endif
