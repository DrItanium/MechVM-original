#ifndef ExceptionBase_H
#define ExceptionBase_H

#include <sstream>
#include "BGString.h"

class ExceptionBase {
public:
   ExceptionBase (const char* msg);

   void append (const char* additionalText);

   void appendInt (int i);
public:
	std::stringstream msg;
};
#endif // end ExceptionBase_H

