////////////////////////////////////////////////////////////////////////////////
// FramerateCounter.cpp
// Copyright Björn Ganster 2002
// This file implements a framerate counter
////////////////////////////////////////////////////////////////////////////////

#include "FramerateCounter.h"

#ifndef _WIN32
#include "sys/time.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// Constructor

FramerateCounter::FramerateCounter (unsigned int entries)
   : LastSeconds (entries),
     LastFrames (entries)
{
   MaxEntries = entries;
   reset ();
}

////////////////////////////////////////////////////////////////////////////////
// Destructor

FramerateCounter::~FramerateCounter ()
{
}

////////////////////////////////////////////////////////////////////////////////
// Start or reset framerate counter

void FramerateCounter::reset ()
{
   CurrEntry = 0;
   framerate = 0.0;
   for (unsigned int i = 0; i < MaxEntries; i++) {
      LastFrames [i] = 0;
      LastSeconds [i] = time (NULL);
   }
}

////////////////////////////////////////////////////////////////////////////////
// Add entry
// Returns true if framerate has changed

bool FramerateCounter::NewFrame ()
{
   time_t now = time (NULL);
   LastFrames [CurrEntry % MaxEntries]++;

   // Store this second's frame rate, if a new second has begun
   if (now != LastSeconds [CurrEntry % MaxEntries]) {
      CurrEntry++;
      LastSeconds [CurrEntry % MaxEntries] = now;
      LastFrames [CurrEntry % MaxEntries] = 0;

      //  Calculate new frame rate
      if (CurrEntry == 0) {
         framerate = LastFrames[0];
      } else if (CurrEntry < MaxEntries) {
         framerate = SumOverFrames (0, CurrEntry-1);
      } else {
         framerate = SumOverFrames (CurrEntry+1-MaxEntries, CurrEntry-1);
      }

      return true;
   } else
      return false;
}

////////////////////////////////////////////////////////////////////////////////
// Sum over recorded frames

double FramerateCounter::SumOverFrames (unsigned int a, unsigned int b) const 
{
   time_t ta = LastSeconds [a % MaxEntries];
   time_t duration = time (NULL) - ta;
   unsigned int frames = 0;

   for (unsigned int i = a; i <= b; i++)
      frames += LastFrames [i % MaxEntries];

   double fraction = ((double) frames) / ((double) duration);

   return fraction;
}

////////////////////////////////////////////////////////////////////////////////

#ifndef _WIN32
int GetTickCount ()
{
   timeval t;
   gettimeofday (&t, NULL);
   int val = (t.tv_sec * 1000) + (t.tv_usec / 1000);
   return val;
}
#endif
