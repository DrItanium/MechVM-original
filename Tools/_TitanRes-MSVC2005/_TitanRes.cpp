// _TT_Resolution-MSVC-2005.cpp : Defines the entry point for the console application.
//

#include "stdlib.h"
#include "stdio.h"
#include "..\src\FileCache.h"

//int _tmain(int argc, _TCHAR* argv[])
int main ()
{
   char str[20];
   int xres, yres;
   printf ("Titanium Resolution Changer - Written by Skyfaller\n"
           "Use at your risk, enjoy!\n\n"
           "Please enter new horizontal in pixels> ");
   scanf ("%19s", str);
   xres = atoi (str);

   printf ("Please enter new vertical resolution in pixels> ");
   scanf ("%19s", str);
   yres = atoi (str);

   INT64 fs = getFileSize ("MercsW.DLL");
   if (fs == 733696) {
      MemoryBlock* mb = new MemoryBlock();
      mb->loadFromFile ("MercsW.DLL");
      mb->saveToFile ("MercsW.DLL.BAK");

      mb->setDWord (0x45CEA, xres);
      mb->setDWord (0x45CF4, yres);

      mb->setDWord (0x46661, xres);
      mb->setDWord (0x46671, yres);

      mb->setDWord (0x466EF, xres-1);
      mb->setDWord (0x466FC, yres-1);

      mb->setDWord (0x46765, xres);
      mb->setDWord (0x4676F, yres);

      mb->setDWord (0x47206, xres);
      mb->setDWord (0x47210, yres);

      mb->saveToFile ("MercsW.DLL");
      delete mb;
      printf ("Mission successful.");
   } else if (fs = 732672) {
      MemoryBlock* mb = new MemoryBlock();
      mb->loadFromFile ("MercsW.DLL");
      mb->saveToFile ("MercsW.DLL.BAK");

      mb->setDWord (285674, xres);
      mb->setDWord (285863, xres);
      mb->setDWord (287774, xres);
      mb->setDWord (288034, xres);

      mb->setDWord (268015, yres);
      mb->setDWord (285684, yres);
      mb->setDWord (287790, yres);
      mb->setDWord (288044, yres);

      mb->setDWord (287916, xres-1);
      mb->setDWord (287929, yres-1);

      // Don't cares?
      /*mb->setDWord (242803, xres);
      mb->setDWord (290755, xres);
      mb->setDWord (285858, yres);
      mb->setDWord (290765, yres);
      mb->setDWord (287959, xres-1);
      mb->setDWord (106159, yres-1);
      mb->setDWord (287969, yres-1);*/

      mb->saveToFile ("MercsW.DLL");
      delete mb;
      printf ("Mission successful.");
   } else {
      printf ("Mission failed - Unsupported version.\n");
   }

   getchar ();
   getchar ();
	return 0;
}

