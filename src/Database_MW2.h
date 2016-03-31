////////////////////////////////////////////////////////////////////////////////
// MechWarrior II database.mw2 file system
// Copyright Bjoern Ganster 2007, 2008
////////////////////////////////////////////////////////////////////////////////

#ifndef Database_MW2__H
#define Database_MW2__H

#include "Archive.h"

////////////////////////////////////////////////////////////////////////////////

class Database_MW2: public MWArchive {
public:
   // Constructor
   Database_MW2();

   // Destructor
   ~Database_MW2();

   // Open file
   virtual bool open (const TCHAR* PRJFN, bool readOnly = true);

   // Close file
   virtual void close();

   // Get name of entry
   virtual bool getEntryName (size_t index, TCHAR* buf, size_t bufSize);

   // Replace  file
   virtual bool replaceFile (size_t fileNum, const TCHAR* srcFN);

   // Check if archive type supports file replacement
   virtual bool replaceFilePossible () const
   { return true; }
private:
   BGVector <int, BGVectorBufID> fileTypes;
};

#endif
