////////////////////////////////////////////////////////////////////////////////
// FramerateCounter.h
// Copyright Björn Ganster 2002
// This is the header for a framerate counter
////////////////////////////////////////////////////////////////////////////////

/*

This class uses a ring buffer to store framerate counts over several seconds.
This allows for more precise framerate calculations, but a larger buffer may
slow down changes in the framerate from affecting the framerate count. 

*/

#ifndef FramerateCounter__h
#define FramerateCounter__h

////////////////////////////////////////////////////////////////////////////////
// Includes

#include <time.h>
#include <vector>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// FramerateCounter class declaration

class FramerateCounter {
public:
   // Constructor, entries: one entry is used per second 
   FramerateCounter (unsigned int entries = 5);

   // Destructor
   ~FramerateCounter ();

   // Start or reset framerate counter
   void reset ();

   // Add entry
   bool NewFrame ();

   // Read framerate
   inline double get () const { return framerate; }

private:
   double framerate;
   vector<time_t> LastSeconds;
   vector<int> LastFrames;
   unsigned int MaxEntries, CurrEntry;

   // Small aid function for calculating frame rates
   double SumOverFrames (unsigned int a, unsigned int b) const;
};

////////////////////////////////////////////////////////////////////////////////

#ifndef WIN32
int GetTickCount ();
#endif

#endif
