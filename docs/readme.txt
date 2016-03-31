What this is for
==========

This is a testbed for deciphering the MW2 file formats.
All code was written from scratch, without disassembling.

When you run the program, a toolbar with the following 
buttons appears:

- Play game
  Well, this doesn't do much yet, but it deserves that special place.
  Displays the currently available mechs. Navigate using 
  the following keys: a, s, d, w, n, m. If you compiled from sources,
  you still require the data from the win32 executable. Point
  the path defined in MechVM.cfg to the mech data. Furthermore,
  run "Import data" first.
- View MW2 file
  This allows you to look inside MW2.prj, database.mw2, archjf.mw2,
  archwo.mw2  files, and it supports displaying various file formats 
  found inside these archives, or loading the supported files directly 
  (see supported file formats, below). 
- Install game
  This will install the game to your hard disk, given proper CD's
- Import data
  Extracts mech models for use in the mech lab and the game
- Unpack MW2 archive
  This will unpack a MW2.prj, database.mw2, archJF.mw2, archWO.mw2 
  or DBSound.mw2

You should be able to compile this for Windows using MSVC,
Intel Compiler using the batch file in icc32, or Linux
by typing "qmake; make" in the source directory.

Have fun with it!

Björn "Skyfaller" Ganster, Lead Developer

=============
Supported file types
=============

SHP: Images/Textures, partially supported
SFL: Sound files, partially supported
WTB: Mesh, supported (not displayed, only exported as .OBJ file)
XEL: Textures, partially supported

=====
License
=====

The software is licensed under GPL version 2, not version 3.
See license.txt. Please inform of any improvements to the code
by sending one or several emails to mail@bjoern-ganster.de.

=====
Thanks
=====

"Whammy" - mech models

Köntzä and Slug - patched DLL's

Sir MMPD Radick - discussions and advice

Col. Kell - discussions and advice

EaterOfPies - help with MW3 mechs

quota4stupid - contributed knowledge on MW2 texturing and tags

Demolishun - deciphered the SHP file format

ecpeterson - contributed source code for playing MW2's SFL files (DPCM)

fOSSiL - created i5comp, used for installing MW3
