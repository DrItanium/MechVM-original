////////////////////////////////////////////////////////////////////////////////
// Utilities for MechVM
// Copyright Bjoern Ganster 2008
////////////////////////////////////////////////////////////////////////////////

#include "Mut.h"
#include "FileCache.h"

////////////////////////////////////////////////////////////////////////////////
// Create a report that compares two files iFN1, iFN2

void compareFiles (const TCHAR* iFN1, const TCHAR* iFN2, const TCHAR* oFN)
{
   // Open files
   MemoryBlock* mb1 = new MemoryBlock();
   mb1->loadFromFile (iFN1);

   MemoryBlock* mb2 = new MemoryBlock();
   mb2->loadFromFile (iFN2);

   DeleteFile (oFN);
   FileCache* out = new FileCache();
   out->create (oFN);

   // Create report header
   BGString line = "Left: ";
   line += iFN1;
   line += ", ";
   line.appendUInt (mb1->getSize());
   if (mb1->getSize() > 1024) {
      line += " (";
      line.appendSizeStr (mb1->getSize());
      line += ")\n";
   }
   out->append(line.getChars());

   line = "Right: ";
   line += iFN2;
   line += ", ";
   line.appendUInt (mb2->getSize());
   if (mb2->getSize() > 1024) {
      line += " (";
      line.appendSizeStr (mb2->getSize());
      line += ")\n";
   }
   out->append(line.getChars());

   if (mb1->getSize() != mb2->getSize())
      out->append ("Files differ in length...\n");

   // Decide what to compare
   size_t startOfs1 = 0;
   size_t count = mb1->getSize();
   size_t startOfs2 = 0;

   if (startOfs1 > mb1->getSize()) {
      startOfs1 = mb1->getSize();
      count=0;
   }
   if (startOfs2 > mb2->getSize()) {
      startOfs2 = mb2->getSize();
      count=0;
   }
   if (startOfs1+count > mb1->getSize())
      count = mb1->getSize()-startOfs1;
   if (startOfs2+count > mb2->getSize())
      count = mb2->getSize()-startOfs2;
   if (count == 0) {
      out->append ("Nothing to compare or illegal compare offsets\n");
      return;
   }
   line = "Comparing ";
   line.appendUInt (count);
   line += " bytes from offset ";
   line.appendUInt (startOfs1);
   line += " (left) rsp. ";
   line.appendUInt (startOfs2);
   line += " (right)\n\n";
   out->append (line);

   // Write report body
   size_t diffs = 0;
   const unsigned char maxAllowableDiff = 5;
   for (size_t i = 0; i < count; i++) {
      unsigned char c1 = mb1->getByte (startOfs1+i);
      unsigned char c2 = mb2->getByte (startOfs2+i);
      unsigned char absDiff = abs (c1-c2);

      if (absDiff > maxAllowableDiff) {
         line.assignUInt (startOfs1+i, 16);
         line += ": ";
         line.appendUInt (c1, 16);
         line += " - ";
         line.appendUInt (startOfs2+i,16);
         line += ": ";
         line.appendUInt (c2, 16);
         line += ", diff: ";
         line.appendUInt (absDiff);
         diffs++;
         line += "\n";
         out->append (line.getChars());
      }
   }

   line = "\n";
   line.appendUInt(diffs);
   line += " bytes differ by more than ";
   line.appendUInt (maxAllowableDiff);
   line += " total.\n";
   out->append (line.getChars());
   out->close();

   delete out;
   delete mb1;
   delete mb2;
}
