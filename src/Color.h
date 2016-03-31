////////////////////////////////////////////////////////////////////////////////
// Class Color
// Copyright Bjoern Ganster 2000-2011
////////////////////////////////////////////////////////////////////////////////

#ifndef Color__h
#define Color__h

////////////////////////////////////////////////////////////////////////////////
// Includes

#include "BGBase.h"

#include <algorithm>
using namespace std;

#include <math.h>
#include <GL/gl.h>

////////////////////////////////////////////////////////////////////////////////
// Class Color

typedef double ColorBase;

class Color: public VirtualClass {
   public:
      ColorBase r, g, b, a;

      // Constructor
      inline Color (ColorBase red = 1.0f, ColorBase green = 1.0f, 
                    ColorBase blue = 1.0f, ColorBase transparency = 1.0f)
      : VirtualClass (ColorID)
      { r = red; g = green; b = blue; a = transparency; }

      // Apply
      inline void glColor () const
      { glColor4d (r, g, b, a); }
      
      // Binary addition
      inline Color operator+ (const Color& c) const 
      { return Color (r+c.r, g+c.g, b+c.b, (a + c.a) / 2); }
      
      // Binary multiplication
      inline Color operator* (const Color& c) const 
      { return Color (r*c.r, g*c.g, b*c.b, a*c.a); }
      
      // Multiplication with scalar
      inline Color operator* (ColorBase f) const 
      { return Color (r*f, g*f, b*f); }

      // Comparison with Color
      inline bool operator == (const Color& c) const
      { return fabs (r-c.r) <= 0.5 / 255.0 
            && fabs (g-c.g) <= 0.5 / 255.0
            && fabs (b-c.b) <= 0.5 / 255.0; }

      inline bool operator!= (const Color& c) const
      { return fabs (r-c.r) > 0.5 / 255.0 
            || fabs (g-c.g) > 0.5 / 255.0 
            || fabs (b-c.b) > 0.5 / 255.0; }

      // Compare with scalar: all components must match the value
      inline bool operator == (ColorBase f) const 
      { return fabs (r-f) <= 0.5 / 255.0 
            && fabs (g-f) <= 0.5 / 255.0 
            && fabs (b-f) <= 0.5 / 255.0; }

      static inline Color red ()
      { return Color (1.0, 0.0, 0.0); }
      static inline Color green ()
      { return Color (0.0, 1.0, 0.0); }
      static inline Color blue ()
      { return Color (0.0, 0.0, 1.0); }
      static inline Color white ()
      { return Color (1.0, 1.0, 1.0); }
      static inline Color black ()
      { return Color (0.0, 0.0, 0.0); }
};

////////////////////////////////////////////////////////////////////////////////
// Parse hex. color entries such as ff ff ff (white), 80 80 80 (grey) or ff 00 00 (red)

void parseColor (Color& c, const char* line);

////////////////////////////////////////////////////////////////////////////////
inline Color interpolateColor(ColorBase val, ColorBase minVal, 
   ColorBase maxVal, const Color& minColor, const Color& maxColor)
{
   if (val < minVal)
      return minColor;
   else if (val > maxVal)
      return maxColor;
   else {
      ColorBase v2 = (val-minVal)/(maxVal-minVal);
      ColorBase v1 = 1.0f - v2;
      return Color (v1 * minColor.r + v2 * maxColor.r,
                    v1 * minColor.g + v2 * maxColor.g,
                    v1 * minColor.b + v2 * maxColor.b);
   }
}

#endif
