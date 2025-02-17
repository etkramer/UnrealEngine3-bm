//------------------------------------------------------------------------------
// ANSI math routine wrappers.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMathANSI_H__
#define FxMathANSI_H__

#include "FxPlatform.h"

#include <cmath>

#if defined(_MSC_VER)
	#pragma intrinsic(fabs)
#endif

namespace OC3Ent
{

namespace Face
{

/// Absolute value of an FxInt32.
FX_INLINE FxInt32 FX_CALL FxAbs( FxInt32 x )
{
	return fx_std(abs)(x);
}

/// Absolute value of an FxReal.
FX_INLINE FxReal FX_CALL FxAbs( FxReal x )
{
	return (FxReal)fx_std(fabs)(x);
}

/// Absolute value of an FxDReal.
FX_INLINE FxDReal FX_CALL FxAbs( FxDReal x )
{
	return fx_std(fabs)(x);
}

/// The ceiling of an FxReal.
FX_INLINE FxReal FX_CALL FxCeil( FxReal x )
{
	return (FxReal)fx_std(ceil)(x);
}

/// The ceiling of an FxDReal.
FX_INLINE FxDReal FX_CALL FxCeil( FxDReal x )
{
	return fx_std(ceil)(x);
}

/// The floor of an FxReal.
FX_INLINE FxReal FX_CALL FxFloor( FxReal x )
{
	return (FxReal)fx_std(floor)(x);
}

/// The floor of an FxDReal.
FX_INLINE FxDReal FX_CALL FxFloor( FxDReal x )
{
	return fx_std(floor)(x);
}

/// The square root of an FxReal.
FX_INLINE FxReal FX_CALL FxSqrt( FxReal x )
{
	return (FxReal)fx_std(sqrt)(x);
}

/// The square root of an FxDReal.
FX_INLINE FxDReal FX_CALL FxSqrt( FxDReal x )
{
	return fx_std(sqrt)(x);
}

/// The log of an FxReal.
FX_INLINE FxReal FX_CALL FxLog( FxReal x )
{
	return (FxReal)fx_std(log)(x);
}

/// The log of an FxDReal.
FX_INLINE FxDReal FX_CALL FxLog( FxDReal x )
{
	return fx_std(log)(x);
}

/// The exponential value of an FxReal.
FX_INLINE FxReal FX_CALL FxExp( FxReal x )
{
	return (FxReal)fx_std(exp)(x);
}

/// The exponential value of an FxDReal.
FX_INLINE FxDReal FX_CALL FxExp( FxDReal x )
{
	return fx_std(exp)(x);
}

/// The power of an FxReal to an FxReal.
FX_INLINE FxReal FX_CALL FxPow( FxReal x, FxReal y )
{
	return (FxReal)fx_std(pow)(x, y);
}

/// The power of an FxDReal to an FxDReal.
FX_INLINE FxDReal FX_CALL FxPow( FxDReal x, FxDReal y )
{
	return fx_std(pow)(x, y);
}

/// The sine of an FxReal.
FX_INLINE FxReal FX_CALL FxSin( FxReal x )
{
	return (FxReal)fx_std(sin)(x);
}

/// The sine of an FxDReal.
FX_INLINE FxDReal FX_CALL FxSin( FxDReal x )
{
	return fx_std(sin)(x);
}

/// The cosine of an FxReal.
FX_INLINE FxReal FX_CALL FxCos( FxReal x )
{
	return (FxReal)fx_std(cos)(x);
}

/// The cosine of an FxDReal.
FX_INLINE FxDReal FX_CALL FxCos( FxDReal x )
{
	return fx_std(cos)(x);
}

/// The tangent of an FxReal.
FX_INLINE FxReal FX_CALL FxTan( FxReal x )
{
	return (FxReal)fx_std(tan)(x);
}

/// The tangent of an FxDReal.
FX_INLINE FxDReal FX_CALL FxTan( FxDReal x )
{
	return fx_std(tan)(x);
}

/// The arc sine of an FxReal.
FX_INLINE FxReal FX_CALL FxAsin( FxReal x )
{
	return (FxReal)fx_std(asin)(x);
}

/// The arc sine of an FxDReal.
FX_INLINE FxDReal FX_CALL FxAsin( FxDReal x )
{
	return fx_std(asin)(x);
}

/// The arc cosine of an FxReal.
FX_INLINE FxReal FX_CALL FxAcos( FxReal x )
{
	return (FxReal)fx_std(acos)(x);
}

/// The arc cosine of an FxDReal.
FX_INLINE FxDReal FX_CALL FxAcos( FxDReal x )
{
	return fx_std(acos)(x);
}

/// The arc tangent of an FxReal.
FX_INLINE FxReal FX_CALL FxAtan( FxReal x )
{
	return (FxReal)fx_std(atan)(x);
}

/// The arc tangent of an FxDReal.
FX_INLINE FxDReal FX_CALL FxAtan( FxDReal x )
{
	return fx_std(atan)(x);
}

/// Trigonometric arctangent of y / x.
FX_INLINE FxReal FX_CALL FxAtan2( FxReal x, FxReal y )
{
	return (FxReal)fx_std(atan2)(x, y);
}

/// Trigonometric arctangent of y / x.
FX_INLINE FxDReal FX_CALL FxAtan2( FxDReal x, FxDReal y )
{
	return fx_std(atan2)(x, y);
}

/// Returns a random number.
FX_INLINE FxInt32 FX_CALL FxRand( void )
{
	return fx_std(rand)();
}

/// The maximum number that can be returned from FxRand().
/// \ingroup support
#define FX_RANDMAX RAND_MAX

} // namespace Face

} // namespace OC3Ent

#endif
