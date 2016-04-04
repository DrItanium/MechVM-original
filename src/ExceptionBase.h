#ifndef ExceptionBase_H
#define ExceptionBase_H

#include <sstream>

class ExceptionBase {
public:
   ExceptionBase (const char* msg);
   ExceptionBase (const ExceptionBase& other);

   void append (const char* additionalText);

   void appendInt (int i);
public:
	std::stringstream msg;
};
#endif // end ExceptionBase_H

