#ifndef ExceptionBase_H
#define ExceptionBase_H

#include <sstream>

class ExceptionBase {
public:
	std::stringstream msg;
   ExceptionBase (const char* msg = "");

   void append (const char* additionalText);

   void appendInt (int i);
};
#endif // end ExceptionBase_H

