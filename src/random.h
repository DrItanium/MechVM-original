////////////////////////////////////////////////////////////////////////////////
// Random.h
// Copyright Bjoern Ganster 2007
////////////////////////////////////////////////////////////////////////////////

#ifndef Random__h
#define Random__h

////////////////////////////////////////////////////////////////////////////////
// Linear congruential generator - see 
// http://en.wikipedia.org/wiki/Linear_congruential_generator

template <class T>
class LinearCongruentialGenerator {
public:
   // Initialize
   inline void init (T& _a, T& b, T& c)
   { a = _a; b = _b; c = _c; }

   // Produce next pseudo-random number
   inline T& next()
   { state = (a*state+b) mod c; }

   // Get value
   inline T& get() const
   { return state; }
private:
   T a, b, c, state;
};

////////////////////////////////////////////////////////////////////////////////

template <class T>
class EvenDistribution {
public:
   // Initialize
   inline void init (T& _min, T& _max, T& a, T& b, T& c)
   { min = _min; max = _max; lcg.init (a, b, c); }

   // Produce next value
   inline T& next()
   { return min + (max-min)*lcg.next(); }

   // Get value
   inline T& get() const
   { return lcg.get(); }
private:
   LinearCongruentialGenerator<T> lcg;
};

#endif
