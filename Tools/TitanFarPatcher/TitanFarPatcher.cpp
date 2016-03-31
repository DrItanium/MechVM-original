// TitanFarPatcher.cpp : Defines the entry point for the console application.
//

//#include "..\src\MechWarriorIIPRJ.h"
#include "..\src\dialogs.h"
#include "..\src\FileCache.h"
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// Set MW2:Titanium far clipping plane distance

const char* setRenderingDistance (const TCHAR* prjFN, DWORD newDist)
{
   MemoryBlock mb;
   const char sig[] = {0x56, 0x49, 0x45, 0x57, 0x10, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00};
   size_t hits = 0;
   newDist *= 100;

   if (mb.loadFromFile(prjFN)) {
      for (size_t i = 0; i < mb.getSize()-16; i++) {
         size_t j = 0;
         bool found = true;
         while (j < 12 && sig[j] == mb.getByte(i+j)) {
            j++;
         }
         if (j == 12) {
            mb.setByte(i+12, newDist & 255);
            mb.setByte(i+13, (newDist >> 8) & 255);
            mb.setByte(i+14, (newDist >> 16) & 255);
            mb.setByte(i+15, (newDist >> 24) & 255);
            hits++;
         }
      }
      mb.saveToFile(prjFN);
      printf ("%i hits\n", hits);
      return "Mission successful";
   } else
      return "Could not open file";
}

/*void setRenderingDistanceInBWD (const char* fileBuf, size_t BWDSize, int newDist)
{
   if (BWDSize < 50)
      return;

   if (fileBuf[0] != 'B'
   &&  fileBuf[1] != 'W'
   &&  fileBuf[2] != 'D'
   &&  fileBuf[3] != 0)
      return;

   size_t offset = 0x34;
   size_t tagSize = 8;

   while (offset + 8 < BWDSize && tagSize > 0) {
      tagSize = *((DWORD*) (&fileBuf[offset+4]));
      char tag[5];
      memcpy (tag, &fileBuf[offset], 4);
      tag[4] = 0;
      // Alter VIEW tag in *PLT1.BWD files
      if (strcmp (tag, "VIEW") == 0 && offset + 16 < BWDSize) 
         // Set new rendering distance
         *((DWORD*) (&fileBuf[offset+12])) = newDist*100;
      offset += tagSize;
   }
}*/

/*const char* setRenderingDistance (const TCHAR* prjFN)
{
   MechWarriorIIPRJ prj;
   const char * result = "Aborted";
   if (prj.open(prjFN, false)) {
      // Create backup
      BGString backupFN = prjFN;
      backupFN += ".bak";
      prj.saveToFile (backupFN.getChars());

      if (prj.enterSubDir("BWD")) {
         for (size_t i = 0; i < prj.getEntryCount(); i++) {
            size_t BWDSize = prj.getFileSize(i);
            char* fileBuf = new char [BWDSize];
            prj.getFileChunk(i, 0, fileBuf, BWDSize);
            setRenderingDistanceInBWD (fileBuf, BWDSize);
            prj.setFile (i, fileBuf, BWDSize);
            delete fileBuf;
         }

         result = "Setting rendering distance: mission successful.";
      } else
         result = "BWD directory not found";
   } else {
      result = "Could not open file";
   }

   return result;
}*/

/*const char* setRenderingDistance (const TCHAR* prjFN, DWORD newDist)
{
   MechWarriorIIPRJ prj;
   const char * result = "Aborted";
   if (prj.open(prjFN, false)) {
      // Create backup
      BGString backupFN = prjFN;
      backupFN += ".bak";
      prj.saveToFile (backupFN.getChars());

      if (prj.enterSubDir("BWD")) {
         for (size_t i = 0; i < prj.getEntryCount(); i++) {
            const size_t bufSize = 20;
            char buf[bufSize];
            prj.getEntryName(i, buf, bufSize);
            if (strcmp (&buf[4], "PLT1.BWD") == 0) {
               size_t BWDSize = prj.getFileSize(i);
               char* fileBuf = new char [BWDSize];
               prj.getFileChunk(i, 0, fileBuf, BWDSize);
               setRenderingDistanceInBWD (fileBuf, BWDSize, newDist);
               prj.setFile (i, fileBuf, BWDSize);
               delete fileBuf;
            }
         }

         result = "Setting rendering distance: mission successful.";
      } else
         result = "BWD directory not found";
   } else {
      result = "Could not open file";
   }

   return result;
}*/

//int _tmain(int argc, _TCHAR* argv[])
int main (int argc, char** argv)
{
   int dist = 8000;
   printf ("TitanFarPatcher - Written by Skyfaller\n");
   printf ("Use at your risk, enjoy!\n");

   const char* result;

   BGString prjFN = "mw2.prj";
   char cwd[300];
   _getcwd (cwd, 299);
   bool OK = (argc == 2);
   if (!OK) {
      if (fileExists ("mw2.prj")) {
         printf ("Changing %s\\MW2.prj\n", cwd);
         OK = true;
      } else {
         BGString prjFN = "NET.prj";
         if (fileExists ("NET.prj")) {
            printf ("Changing %s\\NET.prj\n", cwd);
            OK = true;
         } else {
            printf ("Found neither MW2.PRJ nor NET.PRJ in %s\n", cwd);
            printf ("Please select PRJ to patch\n");
            OK = getOpenFileName(prjFN);
            char str[20];
            printf ("File to be processed: %s\n", prjFN.getChars());
         }
      }
      char str[20];
      printf ("Please enter new maximum viewing distance in meters> ");
      scanf ("%19s", str);
      dist = atoi (str);
   } else {
      prjFN = argv[1];
      dist = atoi (argv[2]);
   }

   if (OK) {
      result = setRenderingDistance (prjFN.getChars(), dist);
      printf (result);
   }

   printf ("\n\nPress <return> to quit\n");

   getchar ();
   getchar ();
	return 0;
}
