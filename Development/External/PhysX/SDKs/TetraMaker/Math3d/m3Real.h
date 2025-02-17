#ifndef M3_REAL_H
#define M3_REAL_H

//---------------------------------------------------------------------------

#include "math3d.h"
#include <float.h>

#define m3Pi     3.1415926535897932f
#define m3HalfPi 1.5707963267948966f
#define m3TwoPi  6.2831853071795865f
#define m3RealMax  FLT_MAX
#define m3RealMin  -FLT_MAX
#define m3Epsilon  FLT_EPSILON
#define m3RadToDeg 57.295779513082321f
#define m3DegToRad 0.0174532925199433f 
#define m3Sqrt2    1.4142135623730950f
#define m3Sqrt3    1.7320508075688773f

typedef float m3Real;

//---------------------------------------------------------------------------

inline m3Real m3Clamp(m3Real &r, m3Real min, m3Real max)
{
	if (r < min) return min;
	else if (r > max) return max;
	else return r;
}

//---------------------------------------------------------------------------

inline m3Real m3Min(m3Real r1, m3Real r2)
{
	if (r1 <= r2) return r1;
	else return r2;
}

//---------------------------------------------------------------------------

inline m3Real m3Max(m3Real r1, m3Real r2)
{
	if (r1 >= r2) return r1;
	else return r2;
}

//---------------------------------------------------------------------------

inline m3Real m3Abs(m3Real r)
{
	return r < 0.0f ? -r : r;
}

//---------------------------------------------------------------------------

inline m3Real m3Sign(m3Real r)
{
	return r < 0.0f ? -1.0f : 1.0f;
}

//---------------------------------------------------------------------------

inline m3Real m3Random(m3Real min, m3Real max)
{
	return min + ((m3Real)rand() / RAND_MAX) * (max - min);
}

//---------------------------------------------------------------------------

inline m3Real m3Cos(m3Real r)
{
	return cosf(r);
}

//---------------------------------------------------------------------------

inline m3Real m3Sin(m3Real r)
{
	return sinf(r);
}

//---------------------------------------------------------------------------

inline m3Real m3Acos(m3Real r)
{
	// result is between 0 and pi
	if (r < -1.0f) r = -1.0f;
	if (r >  1.0f) r =  1.0f;
	return acos(r);
}

//---------------------------------------------------------------------------

inline m3Real m3Asin(m3Real r)
{
	// result is between -halfPi and halfPi
	if (r < -1.0f) r = -1.0f;
	if (r >  1.0f) r =  1.0f;
	return asin(r);
}

//---------------------------------------------------------------------------

inline m3Real m3Sqrt(m3Real r)
{
	return sqrtf(r);
}

//---------------------------------------------------------------------------

inline m3Real m3Disturb(m3Real r)
{
	return r * (1.0f + m3Epsilon * (-10 + rand() % 20)); 
}


#endif
