////////////////////////////////////////////////////////////////////////////////
// Class Color
// Copyright Bjoern Ganster 2010
////////////////////////////////////////////////////////////////////////////////

#include "Color.h"
#include "BGString.h"

////////////////////////////////////////////////////////////////////////////////
// Read color from a line

void parseColor (Color& c, const char* line)
{
   if (line != NULL) {
      BGString num;
      size_t lineLen = strlen (line);
      num.assignWord (line, 0, ' ', lineLen);
      c.r = num.toInt(16)/255.0;

      num.assignWord (line, 1, ' ', lineLen);
      c.g = num.toInt(16)/255.0;

      num.assignWord (line, 2, ' ', lineLen);
      c.b = num.toInt(16)/255.0;

      num.assignWord (line, 3, ' ', lineLen);
      if (num.getLength() != 0) 
         c.a = num.toInt(16)/255.0;
      else
         c.a = 1.0;
   }
}
