These files allow to install MechWarrior2-31st century combat under Windows XP.

Please download and install VDMSound from http://sourceforge.net/projects/vdmsound.

To enable cd audio support download SapuCDEx from 

http://vogons.zetafleet.com/viewtopic.php?t=1715 

Extract it to your windows\system32 folder and edit the following few files:

In autoexec.nt (which is also located in your system32 folder), replace the line:

lh %SystemRoot%\system32\mscdexnt.exe

with

lh %SystemRoot%\system32\sapucdex.exe

Finally add "REM" (no quotes) to the line beginning something like:

Set blaster = A220 I5 etc..

e.g.

REM SET BLASTER = A220 etc...

Save the edited file.

Now locate autoexec.vdms (it's in the folder where you installed vdmsound). 
Edit this and again replace the line ending with mscdexnt.exe with sapcdex.exe 
as before. 

Finally, run the game using _mw2clans.bat