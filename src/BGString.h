////////////////////////////////////////////////////////////////////////////////
// BGString declaration
// Copyright Bjoern Ganster 2005-2011
////////////////////////////////////////////////////////////////////////////////

#ifndef BGString__H
#define BGString__H

#include <stdio.h>
#include <stdlib.h>
#include <sstream>

#include "BGBase.h"
#ifndef _WIN32
#include <wchar.h> // for wcslen, wide character string length computation
#include <unistd.h>
#endif
#include "ExceptionBase.h"
#include "BGVector.h"

////////////////////////////////////////////////////////////////////////////////
// Constants

// Operating system path separator
#ifdef WIN32
const char OSPathSep = '\\';
const TCHAR newLine[] = {0x0d, 0x0a};
#else
const char OSPathSep = '/';
const TCHAR newLine[] = "\n";
#endif

const TCHAR tabChar = 9;
const char ciphers[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


///////////////////////////////////////////////////////////////////////////////
// bgstrlen, version with MaxChars

template<typename C>
int bgstrlen (C* a, unsigned int MaxChars)
{
	unsigned int i = 0;

	// Find end of a 
	while (i < MaxChars-1 && a[i] != 0) {
		i++;
	}

   return i;
}

///////////////////////////////////////////////////////////////////////////////
// bgstrlen, version without MaxChars

template<typename C>
int bgstrlen (C* a)
{
   if (a != NULL) {
	unsigned int i = 0;

	// Find end of a 
	while (a[i] != 0) {
		i++;
	}

   return i;
   } else 
      return 0;
}

///////////////////////////////////////////////////////////////////////////////

template<typename C>
int bgstrcmp (const C* a, const C* b)
{
   if (a == NULL && b == NULL)
      return 0;
   if (a == NULL)
      return -1;
   if (b == NULL)
      return 1;
	unsigned int i = 0;

	// Find end of a 
	while (true) {
      if (a[i] == 0 || b[i] == 0 || a[i] != b[i])
         return a[i]-b[i];
		i++;
	}

   return 0;
}

///////////////////////////////////////////////////////////////////////////////
// bgstrlen, version with MaxChars

template<typename C>
int bgstrnlen (const C* a, size_t maxLen)
{
	unsigned int i = 0;

	// Find end of a 
	while (a[i] != 0 && i != maxLen) {
		i++;
	}

   return i;
}

///////////////////////////////////////////////////////////////////////////////
// Scan a string for a char

template<typename C>
bool bgstrscan (C* str, char c, size_t start, size_t stop, size_t& pos)
{
   pos = start;

   while (str[pos] != c && pos < stop)
      pos++;

   return (str[pos] == c);
}

// Reverse scan
template<typename C>
bool bgstrrscan (C* str, char c, size_t start, size_t stop, size_t& pos)
{
   pos = start;

   while (str[pos] != c && pos > stop)
      pos--;

   return (str[pos] == c);
}

// String copy with size limit
template<typename C, typename D>
void bgstrcpymax(C* target, const D* source, size_t maxcount)
{
   if (target == NULL)
      return;

   size_t i = 0;

   while (i < maxcount && source[i] != 0) {
      target[i] = (C) (source[i]);
      ++i;
   }

   if (i < maxcount) 
      target[i] = 0;
   else if (maxcount > 0)
      target[maxcount-1] = 0;
}

// Replace chars in a string
template<typename C>
void bgstrReplace (C* buf, C oldC, C newC)
{
   size_t i = 0;
   while (buf[i] != 0) {
      if (buf[i] == oldC)
         buf[i] = newC;
      ++i;
   }
}

////////////////////////////////////////////////////////////////////////////////
// Strings

template <typename C>
class TBGString: VirtualClass
{
protected:
   C* chars;
   size_t length, reserved;

    size_t preferredReserve (size_t newLen)
   { return 2*newLen + 10; }
   //{ return 2*newLen + 30; }

public:
   // (Default) Constructor
   TBGString (const C* str = NULL)
   : VirtualClass(BGStringID),
     chars(NULL), 
     length(0), 
     reserved(0)
   {
      operator=(str);
   }

   // Copy constructor
   TBGString (const TBGString<C>& other)
      : VirtualClass(other.getClassID()), 
        chars(NULL), 
        length(0), 
        reserved(0)
   {
      operator=(other.getChars());
   }

   // Destructor
   virtual ~TBGString()
   {
      if (this->chars != NULL)
         delete this->chars;
   }

   // Get length
    size_t getLength () const 
   { return length; }

   // Get characters
    const C* getChars() const 
   {
      if (this->chars != NULL)
         return this->chars; 
      else
         return "";
   }

   // Secure get character at index (using operator[] on getChars is faster but 
   // less secure)
    bool getChar (size_t index, C& c) const
   {
      if (index < length) {
         c = this->chars[index];
         return true;
      } else
         return false;
   }

   // Set string length
   void setLength (size_t newlen)
   {
      if (newlen < this->reserved) {
         this->chars[newlen] = 0;
      } else if (newlen > this->reserved) {
         C* newchars = new C [newlen+2]; // lower added values cause heap corruption?
         memcpy (newchars, this->chars, sizeof(C) * this->reserved);
         this->reserved = newlen+1;
         if (this->chars != NULL)
            delete this->chars;
         this->chars = newchars;
      }
      length = newlen;
   }

   // Set string reserve
   void setReserve (size_t newReserve)
   {
      if (newReserve > this->reserved) {
         C* newchars = new C [newReserve+1];
         memcpy (newchars, this->chars, sizeof(C) * length);
         this->reserved = newReserve+1;
         if (this->chars != NULL)
            delete this->chars;
         this->chars = newchars;
      }
   }

   // Set character at index
    void setChar (size_t index, char c)
   {
      if (index < length)
         this->chars[index] = c;
   }

   // Assignment operators
   void operator = (const TBGString& other)
   {
      operator=(other.getChars());
   }
   void operator = (const char* str)
   {
      if (str != NULL) {
         size_t newlen = strlen (str);
         setLength(newlen);
         bgstrcpymax(this->chars, str, length+1);
      } else {
         clear();
      }
   }
   void operator = (const wchar_t* str)
   {
      if (str != NULL) {
         length = bgstrlen (str);
         setLength(length);
         bgstrcpymax(this->chars, str, length+1);
      } else {
         clear();
      }
   }
   void operator = (C c)
   {
      setLength(1);
      this->chars[0] = c;
      this->chars[1] = 0;
   }

   // Appending operators
   void operator += (const char* str)
   {
      if (str != NULL) {
         size_t len = bgstrlen (str);
         appendPart (str, 0, len);
      }
   }
   void operator += (const wchar_t* str)
   {
      if (str != NULL) {
         size_t len = bgstrlen (str);
         appendPart (str, 0, len);
      }
   }
   void operator += (char c)
   {
      size_t oldLen = length;
      if (length+1 >= this->reserved)
         setLength (2*length+10);
      this->chars[oldLen] = (C) c;
      this->chars[oldLen+1] = 0;
      length = oldLen+1;
   }
   void operator += (wchar_t c)
   {
      size_t oldLen = length;
      if (length >= this->reserved)
         setLength (2*length+10);
      this->chars[oldLen] = (C) c;
      this->chars[oldLen+1] = 0;
      length = oldLen+1;
   }
   void operator += (TBGString<char>& str)
   { operator+= (str.getChars()); }
   void operator += (TBGString<wchar_t>& str)
   { operator+= (str.getChars()); }

   // Assign or append part of another string
   void copyPart (const C* str, size_t start, size_t stop)
   {
      clear();
      appendPart (str, start, stop);
   }

   void appendPart (const char* str, size_t start, size_t stop)
   {
      if (stop <= start) {
         return;
      }

      // Determine length of string
      size_t len = 0;
      if (str != NULL) {
         len = strlen (&str[start]);
      }

      if (start + len < stop)
         stop = start + len;

      for (size_t i = start; i < stop; i++) {
          operator += (str[i]);
      }
   }
   void appendPart (const wchar_t* str, size_t start, size_t stop)
   {
      // Determine length of string
      size_t len = 0;
      if (str != NULL) {
         #ifdef _WIN32
         len = lstrlenW (str);
         #else
         len = wcslen (str);
         #endif
      }

      if (len < stop)
         stop = len;

      for (size_t i = start; i < stop; i++) {
          operator += ((char) (str[i]));
      }
   }

   // Remove whitespace at the beginning of a string
   void removeLeadingWhitespace()
   {
      if (this->chars != NULL) {
         size_t deleted = 0;
         while (  (this->chars[deleted] == ' '
                || this->chars[deleted] == 0x09
                || this->chars[deleted] == 0x0a
                || this->chars[deleted] == 0x0d)
         &&     deleted < length)
         {
            ++deleted;
         }

         if (deleted > 0) {
            memcpy(this->chars, this->chars+deleted, length-deleted);
            length -= deleted;
            this->chars[length] = 0;
         }
      }
   }

   // Remove whitespace at end of string
   void removeTrailingWhitespace()
   {
      // Memory for removed whitespace is not released but kept
      // as reserved memory
      if (this->chars != NULL) {
         while (  (this->chars[length-1] == ' '
                || this->chars[length-1] == 0x09
                || this->chars[length-1] == 0x0a
                || this->chars[length-1] == 0x0d)
         &&     length > 0)
         {
            length--;
         }
         this->chars[length] = 0;
      }
   }

   // Find character, sets pos to length if not found
   bool findChar (C c, size_t& pos) const
   {
      size_t i = 0;
      while (i < getLength()) {
         if (this->chars[i] == c) {
            pos = i;
            return true;
         } else
            i++;
      }

      pos = length;
      return false; // nothing found
   }
   bool findChar (C c, int start, int stop, int step, size_t& pos) const
   {
      int i = start;
      assert (step != 0);

      while ((step > 0 && i <= stop) || (step < 0 && i >= stop)) {
         if (this->chars[i] == c) {
            pos = i;
            return true;
         }
         i += step;
      }

      pos = length;
      return false; // nothing found
   }


   // Convert numbers to string
   void assignInt (int value, unsigned int base = 10)
   {
     clear();
     appendUInt (value, base);
   }
   void appendInt (int value, unsigned int base = 10)
   {
      if (value == 0)
         operator += ("0");
      else {
         if (value < 0) {
            operator += ("-");
            value = -value;
         }

         appendUInt ((size_t) value, base);
      }
   }
   void assignUInt (size_t value, unsigned int base = 10)
   {
     clear();
     appendUInt (value, base);
   }
   void appendUInt (size_t value, unsigned int base = 10)
   {
      // Count number of ciphers needed
      size_t tmp = value;
      size_t ciphersNeeded = 1;
      while (tmp >= base) {
         tmp = tmp / base;
         ciphersNeeded ++;
      }

      // Assign result
      if (value > 0) {
         setLength (length + ciphersNeeded);
         size_t currCipher = length - 1;
         while (value > 0) {
            assert (currCipher <= getLength());
            size_t cipher = value % ((size_t) base);
            char cipherChar = ciphers[cipher];
            this->chars[currCipher] = cipherChar;
            value = value / ((size_t) base);
            currCipher--;
         }
         this->chars[length] = 0;
      } else
         operator+= ('0');
   }
   void assignDouble (double value, unsigned int precision = 3)
   {
      clear();
      appendDouble (value, precision);
   }
   void appendDouble (double value, unsigned int precision = 3)
   {
      const char ciphers[] = "0123456789";
      if (value < 0) {
         operator+= ('-');
         value = -value;
      }

      // Round
      double roundSum = 0.5;
      for (unsigned int i = 0; i < precision; i++)
         roundSum /= 10.0;
      value += roundSum;

      appendInt ((int) floor (value), 10);
      value = value - floor (value);
      if (value < 0.0 || value >= 1.0) {
         printf ("Internal error 1\n");
         return;
      }

      if (precision > 0) {
         operator+= ('.');
         while (precision > 0) {
            precision--;
            value = 10*value;
            unsigned int cipher = (unsigned int) floor (value);
            assert (cipher < 10);
            char cipherChar = ciphers[cipher];
            operator+= (cipherChar);
            value = value - floor (value);
         }
      }
   }
   void appendPointer (void* p)
   { appendUInt(size_t (p), 16); }

   // Test if string equals another string
   bool equals (const C* other) const
   { return (bgstrcmp(this->chars, other) == 0); }

   // Create size string, e.g. 1024MB for 1024*1024
   void appendSizeStr (size_t size)
   {
      if (size < 2 * 1024) {
         // Report size in bytes
         appendInt ((int) size);
         operator+= (" bytes");
      } else if (size < 2 * 1024 * 1024) {
         // Report size in kbytes
         appendDouble ((double) size / 1024.0, 1);
         operator+= (" kB");
      } else if (size < ((size_t) (1024 * 1024 * 1024))) {
         // Report size in MBytes
         appendDouble ((double) size / (1024.0 * 1024.0), 1);
         operator+= (" MB");
      } else {
         // Report size in GBytes
         appendDouble ((double) size / (1024.0 * 1024.0 * 1024.0), 1);
         operator+= (" GB");
      } 
   }

   // Append time string
   void appendTimeStr (int ms)
   {
      if (ms < 2000) {
         appendDouble (ms/1000.0, 2);
         operator+= (" s");
      } else if (ms < 60 * 1000) {
         appendDouble (ms/1000.0, 1);
         operator+= (" s");
      } else if (ms < 60*60*1000) {
         appendInt (ms/60000);
         operator+= (" min ");
         ms = ms % 60000;
         appendInt (ms/1000);
         operator+= (" s");
      } else {
         appendInt (ms/(60*60000));
         operator+= (" h ");
         ms = ms % (60 * 60000);
         appendInt (ms/60000);
         operator+= (" min");
      }
   }

   // Array access
   const TCHAR& operator[] (size_t index) const
   {
      if (index < length)
         // If length > 0, then chars should not be NULL
         return this->chars[index];
      else {
         /*ExceptionBase e ("Access to index ");
         e.appendInt ((int) index);
         e.append (", for string of size ");
         e.appendInt ((int) length);
         throw e;*/
         throw;
      }
   }
   C& operator[] (size_t index)
   {
      if (index < length)
         // If length > 0, then chars should not be NULL
         return this->chars[index];
      else {
         /*ExceptionBase e ("Access to index ");
         e.appendInt ((int) index);
         e.append (", for string of size ");
         e.appendInt ((int) length);
         throw e;*/
	 throw;
      }
   }

   // Retrieve integer from string
   int toInt(int base=10) const
   {
      int result = 0;
      size_t i = 0;
      bool negate = false;

      if (length > 0) {
         int v;

         if (this->chars[i] == '-') {
            negate = true;
            i++;
         }
         
         while (i < length) {
            switch (this->chars[i]) {
               case '0': v = 0; break;
               case '1': v = 1; break;
               case '2': v = 2; break;
               case '3': v = 3; break;
               case '4': v = 4; break;
               case '5': v = 5; break;
               case '6': v = 6; break;
               case '7': v = 7; break;
               case '8': v = 8; break;
               case '9': v = 9; break;
               case 'a': v = 10; break;
               case 'b': v = 11; break;
               case 'c': v = 12; break;
               case 'd': v = 13; break;
               case 'e': v = 14; break;
               case 'f': v = 15; break;
               case 'A': v = 10; break;
               case 'B': v = 11; break;
               case 'C': v = 12; break;
               case 'D': v = 13; break;
               case 'E': v = 14; break;
               case 'F': v = 15; break;
               default: v = -1;
            }

            if (v >= 0 && v < base) {
               result = result*base + v;
               ++i;
            } else
               i = length;
         }
      }

      if (negate)
         result= -result;
      return result;
   }

   // Retrieve double
   bool toDouble (double& result) const
   {
      if (length == 0)
         return false;
      size_t pos = 0;
      bool negate = (this->chars[0] == '-');
      if (negate)
         pos++;

      // Empty operands are left 0. This is required for unary minus. Thus,
      // -5 is interpreted as 0-5
      result = 0;
      bool stop = false;
      while (!stop)
      {
         if (this->chars[pos] >= '0'
         &&  this->chars[pos] <= '9')
         {
            result = 10 * result + (this->chars[pos] -'0');
            pos++;
            if (pos >= length)
               stop = true;
         } else {
            if (this->chars[pos] == '.')
               stop = true;
            else
               return false;
         }
      }

      if (this->chars[pos] == '.') {
         pos++;
         double factor = 0.1;
         while (pos < length)
         {
            if (this->chars[pos] >= '0'
            &&  this->chars[pos] <= '9') 
            {
            result = result + factor * (this->chars[pos] -'0');
            factor = factor / 10;
            pos++;
            } else
               return false;
         }
      }

      if (negate)
         result = -result;

      return true;
   }

   // Clear string
   void clear ()
   { length = 0; if (this->chars != NULL) this->chars[0] = 0; }

   // Print string to stdout
    void print () const
   //{ log (this->chars); }
   #ifdef UNICODE
   { wprintf (this->chars); }
   #else
   { printf (this->chars); }
   #endif

   // Print string to stdout and add a line feed
    void printLn () const
   //{ logLn (this->chars); }
   #ifdef UNICODE
   { wprintf (this->chars); wprintf (newLine); }
   #else
   { printf (this->chars); printf (newLine); }
   #endif

   // Assign word from another string, returns offset of end of word
   // len gives the max. number of input characters to scan
   size_t assignWord (const char* other, int word, char separator, size_t len)
   {
      size_t pos = 0;
      clear();

      if (other == NULL)
         return 0;

      // Skip to word
      while (word > 0 && pos < len) {
         while (other[pos] != separator && pos < len)
            pos++;
         while (other[pos] == separator && pos < len)
            pos++;
         word--;
      }

      // Read until next separator
      while (pos < len && other[pos] != separator) {
         operator +=(other[pos++]);
      }

      return pos;
   }

   bool lastCharEquals (C c)
   {
      C last = c+1;
      if (getChar(getLength()-1, last)) 
         return (last == c);
      else
         return false;
   }

   // Test if string ends with another string
   bool endsWith(const C* buf)
   {
      size_t otherLen = strlen(buf);
      if (otherLen > length)
         return false;

      bool correct = true;
      size_t i = 0;
      size_t base = length-otherLen;
      while (correct && i < otherLen) {
         if (buf[i] == this->chars[base+i])
            i++;
         else
            correct = false;
      }

      return correct;
   }

   void appendNoDups (C c)
   {
      if (!lastCharEquals (c))
         operator += (c);
   }

   // other has the same chars or is equal
   bool startsWith (const C* other) const
   {
      size_t olen = 0;
      if (other != NULL)
         olen = bgstrlen(other);

      if (olen > getLength())
         return false;

      return (memcmp(this->chars, other, olen) == 0);
   }

   // Save string to file
   bool saveToFile (const char* FN)
   {
      #ifdef WIN32
      FILE* f;
      fopen_s(&f, FN, "wb");
      #else
      FILE* f = fopen(FN, "wb");
      #endif
      if (f == NULL) 
         return false;
      size_t written = fwrite (this->chars, 1, length, f);
      fclose (f);
      return written == length;
   }
   #ifdef WIN32
   bool saveToFile (const wchar_t* FN)
   {
      HANDLE h = 
         CreateFileW(FN, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (h == INVALID_HANDLE_VALUE)
         return false;
      DWORD written;
      WriteFile (h, this->chars, length*sizeof(C), &written, NULL);
      CloseHandle (h);
      return (written == length*sizeof(C));
   }
   #endif

   // Load string from file
   #ifdef WIN32
   bool loadFromFile (const char* FN)
   {
      HANDLE h = 
         CreateFileA(FN, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (h == INVALID_HANDLE_VALUE)
         return false;
      DWORD size, read;
      size = GetFileSize(h, NULL);
      setLength(size / sizeof(C));
      ReadFile (h, this->chars, size, &read, NULL);
      CloseHandle (h);
      return (read == size);
   }
   bool loadFromFile (const wchar_t* FN)
   {
      HANDLE h = 
         CreateFileW(FN, GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (h == INVALID_HANDLE_VALUE)
         return false;
      DWORD size, read;
      size = GetFileSize(h, NULL);
      setLength(size / sizeof(C));
      ReadFile (h, this->chars, size, &read, NULL);
      CloseHandle (h);
      return (read == size);
   }
   #else
   bool loadFromFile (const char* FN)
   {
      FILE* f = fopen(FN, "rb");
      if (f == NULL) 
         return false;
      //size_t newLen = getFileSize (FN);
      struct stat fstat;
      stat (FN, fstat);
      setLength (fstat.st_size);
      size_t read = fread (this->chars, 1, length, f);
      fclose (f);
      return (read == length);
   }
   #endif

   void appendOSError()
   {
      #ifdef _WIN32
      DWORD winErrCode = GetLastError();
      const size_t winErrSize = 500;
      TCHAR winErr[winErrSize];
      FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, winErrCode, 
                     0, (TCHAR*) (&winErr), winErrSize, NULL);
      operator += (winErr);
      #endif
   }

   // Count occurences of character c
   size_t count(char c) const
   {
      size_t result = 0;

      if (this->chars != NULL) {
         for (size_t i = 0; i < length; ++i) 
            if (this->chars[i] == c)
               ++result;
      }

      return result;
   }

   bool readLine (size_t& offset, TBGString<C>& result) const
   {
      bool done = false;
      result = "";

      // Find start of newline, not including whitespace
      do {
         if (offset < getLength()) {
            char c = this->chars[offset];
            if (c == 0x09
            ||  c == 0x0a
            ||  c == 0x0d
            ||  c == 0x20)
            {
               ++offset;
            } else
               done = true;
         } else 
            done = true;
      } while (!done);

      // Append chars until end of line
      done = false;
      do {
         if (offset < getLength()) {
            char c = this->chars[offset];
            if (c == 0x0a
            ||  c == 0x0d)
            {
               done = true;
            } else {
               result += c;
               ++offset;
            }
         } else 
            done = true;
      } while (!done);

      if (offset < getLength() || result.getLength() > 0)
         return true;
      else
         return false;
   }
};

typedef TBGString<char> BGString;
typedef TBGString<wchar_t> BGUString;

///////////////////////////////////////////////////////////////////////////////
// Convenience class for constructing paths

template <class C>
class BGFilePath: public TBGString<C> {
public:
   // Constructor
   BGFilePath (const C* path = NULL)
      : TBGString<C>(path),
        PathSep (OSPathSep),
        placeNextRelative (false)
   {}

   // Copy constructor
   BGFilePath (const BGFilePath& other)
      : TBGString<C> (other.chars),
        PathSep (other.PathSep),
        placeNextRelative (other.placeNextRelative)
   {}

   // Destructor
   ~BGFilePath()
   {}

   // Add path element, separated by PathSep
   void operator += (const C* element)
   {
      if (!placeNextRelative) {
         this->appendNoDups (PathSep);
      } else {
         placeNextRelative = false;
      }
      TBGString<C>::operator+= (element);
   }

   // Set path separator
   void setPathSep (C newPathSep)
   { PathSep = newPathSep; }

   // Set relative base, next element will be added without a path separator
   void setRelativePath (const C* base) 
   {
      operator = (base);
      placeNextRelative = true;
   }

   void getCWD ()
   {
      // Linux may require get_current_dir_name ();
      #ifdef WIN32
      if (sizeof(C) == 1) {
         DWORD newLen = GetCurrentDirectoryA(0, NULL);
         setReserve(newLen+1);
         GetCurrentDirectoryA(this->reserved, this->chars);
      } else {
         DWORD newLen = GetCurrentDirectoryW(0, NULL);
         setReserve(newLen+1);
         GetCurrentDirectoryW(this->reserved, (wchar_t*) this->chars);
      }
      #else
      while (get_current_dir_name (this->chars, this->reserved) == NULL) {
         setReserve (2*this->reserved);
      }
      #endif
      this->length = bgstrlen(this->chars);
   }

   bool changeExtension (const char* newExt)
   {
      bool result = false;

      if (this->length > 0) {
         size_t pos;
         if (bgstrrscan(this->chars, '.', this->length-1, 0, pos)) {
            this->setLength(pos+1);
            placeNextRelative = true;
            operator += (newExt);
            result = true;
         }
      }

      return result;
   }

   bool exists () const
   {
      return fileExists(this->chars);
   }
private:
   C PathSep;
   bool placeNextRelative;
};

////////////////////////////////////////////////////////////////////////////////

template <typename C> 
class TBGStringList {
public:
   // Constructor
   TBGStringList()
      : m_strings()
   {
   }

   // Copy constructor
   TBGStringList(const TBGStringList& other)
      : m_strings(other.getSize())
   {
      for (size_t i = 0; i < other.getSize(); ++i) {
         const C* ostr = other.get(i);
         set (i, ostr);
      }
   }

   // Destructor
   virtual ~TBGStringList()
   {
      for (size_t i = 0; i < m_strings.getSize(); ++i) {
         delete m_strings[i];
         m_strings[i] = NULL;
      }
   }


   // Get string
    const C* get (size_t index) const
   {
      if (index < m_strings.getSize())
         return m_strings[index];
      else
         return NULL;
   }

   // Set string
    void set (size_t index, const C* newStr)
   {
      if (index >= m_strings.getSize())
         m_strings.setSize(index+1);

      if (newStr != NULL) {
      size_t len = bgstrlen(newStr);
      m_strings[index] = new C[len+1];
      memcpy (m_strings[index], newStr, sizeof(C)*(len+1));
      } else
         m_strings[index] = NULL;
   }

   // Add string
    void add (const C* newStr)
   {
      set (m_strings.getSize(), newStr);
   }

   // Get size
    size_t getSize() const
   { return m_strings.getSize(); }
private:
   BGVector <C*, BGVector_string_ID> m_strings;
};

typedef TBGStringList<char> BGStringList;
typedef TBGStringList<wchar_t> BGUStringList;

////////////////////////////////////////////////////////////////////////////////
// Simple string functions

// Check if string a has string b at position start
bool strPartsEqual (const char* a, size_t start, const char* b);
bool strPartsEqual (const char* a, size_t start, size_t end, const char* b);

// Test if character c appears in str between start and stop, and set pos
// to the occurence if found. Returns true if c is found, false otherwise
bool hasChar (char c, const char* str, size_t start, size_t stop, size_t& pos);

// Return lower case character
TCHAR lowerCase (TCHAR a);

////////////////////////////////////////////////////////////////////////////////
// Exception base


////////////////////////////////////////////////////////////////////////////////
// Logging

// Abstract definition of logging classes
class abstractLog {
public:
   virtual void log (const char* str) = 0;
   virtual void log (const BGString& str) = 0;
   virtual void logLn (const BGString& str) = 0;
   virtual ~abstractLog() {}
};

// Get, set logging class
void setLog (abstractLog* logClass);
abstractLog* getLog();

// Log string
void log (const char* str);
void log (const BGString& str);
void logLn (const BGString& str);

////////////////////////////////////////////////////////////////////////////////
// Get/set execution directory (singleton)

void setExecDir (const TCHAR* newDir, bool parse);
const TCHAR* getExecDir ();
const TCHAR* getDataDir ();
void getDosBoxCDir (BGString& str);
bool isRelativePath (const TCHAR* path);

////////////////////////////////////////////////////////////////////////////////

class BGStringAttribute: VirtualClass {
public: 
   BGStringAttribute(const char* initVal = NULL)
      : VirtualClass (),
        value (initVal)
   {
   }

   ~BGStringAttribute()
   {
   }

    const TCHAR* get () const
   { return value.getChars(); }

    void set (const char* newVal)
   { value = newVal; }

    void set (const BGString& newVal)
   { value = newVal; }

private:
   BGString value;
};

#endif
