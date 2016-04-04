////////////////////////////////////////////////////////////////////////////////
// BGString implementation
// Copyright Bjoern Ganster 2005-2010
////////////////////////////////////////////////////////////////////////////////

#include "ExceptionBase.h"
#include "BGString.h"

#ifdef WIN32
#include <direct.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Check if string a has string b at position start

bool strPartsEqual (const char* a, size_t start, const char* b)
{
   size_t alen = strlen (a);
   size_t blen = strlen (b);

   if (alen >= start && start + blen <= alen) {
      for (size_t i = 0; i < blen; i++) {
         if (a[i+start] != b[i])
            return false;
      }
      return true;
   } else 
      return false;
}


////////////////////////////////////////////////////////////////////////////////
// Check if string a has string b at position start

bool strPartsEqual (const char* a, size_t start, size_t end, const char* b)
{
   size_t alen = strlen (a);
   size_t blen = strlen (b);

   if (alen >= start 
   &&  start + blen <= alen
   &&  end-start == blen)
   {
      unsigned int i = 0;
      while (i < blen && start + i < end) {
         if (a[i+start] != b[i])
            return false;
         i++;
      }
      return true;
   } else 
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Test if character c appears in str between start and stop, and set pos
// to the occurence if found. Returns true if c is found, false otherwise

bool hasChar (char c, const char* str, size_t start, size_t stop, size_t& pos)
{
   bool found = false;
   if (str != NULL) {
      size_t i = start;
      while (i < stop && !found) {
         if (str[i] == c) {
            pos = i;
            found = true;
         } else
            ++i;
      }
   } 
   return found;
}


TCHAR lowerCase (TCHAR a)
{
   if (a >= 'A' && a <= 'Z')
      return a-'A'+'a';
   else
      return a;
}

////////////////////////////////////////////////////////////////////////////////
// Logging

abstractLog* logger = NULL;

void setLog (abstractLog* logClass)
{
   logger = logClass;
}

abstractLog* getLog()
{ 
   return logger; 
}

void log (const char* str)
{ 
   if (logger != NULL)
      logger->log (str); 
   else
      printf (str);
}

void log (const BGString& str)
{ 
   if (logger != NULL)
      logger->log (str);
   else
      printf (str.getChars());
}

void logLn (const BGString& str) 
{ 
   // Compose single string: with multiple threads, another thread may print
   // a message until we print the carriage return
   BGString str2 = str;
   str2 += "\n";
   log (str2);
}

////////////////////////////////////////////////////////////////////////////////

BGFilePath<TCHAR>* execDir;

void setExecDir (const TCHAR* newDir, bool parse)
{
   execDir = new BGFilePath<TCHAR> (newDir);
   if (parse) {
      size_t start = execDir->getLength()-1, pos;
      bgstrrscan(newDir, OSPathSep, start, 0, pos);
      execDir->setLength(pos);

      // The full path is not always specified
      if (execDir->getLength() == 0 || execDir->equals (".")) {
         const size_t pathLen = 1024;
         TCHAR path[pathLen];
         getcwd (path, pathLen);
         execDir->operator = (path);
      }
   }
}

const TCHAR* getExecDir ()
{
   return execDir->getChars();
}

const TCHAR* getDataDir ()
{
   // todo: should be $home/.MechVM under Linux, c:\users\<user>\appdata\MechVM under Windows
   return getExecDir();
}

void getDosBoxCDir (BGString& str)
{
   str = getDataDir();
   str += OSPathSep;
   str += "DosBox-0.73";
   str += OSPathSep;
   str += "C";
}

bool isRelativePath (const TCHAR* path)
{
   if (path == NULL) {
      return true; // treat empty path as relative
   }

   #ifdef _WIN32
   if (bgstrlen(path) >= 3) {
      if (path[1] == ':' && path[2] == '\\') {
         return false;
      } else {
         return true;
      }
   } else {
      return false;
   }
   #else
   return path[0] != '/';
   #endif
}

ExceptionBase::ExceptionBase (const char* msg)
{
	this->msg << msg;
}

ExceptionBase::ExceptionBase (const ExceptionBase& other) {
	this->msg << other.msg.str();
}

void ExceptionBase::append (const char* additionalText)
{
	this->msg << additionalText;
}

void ExceptionBase::appendInt (int i)
{
	this->msg << i;
}


