//------------------------------------------------------------------------------
// FaceFx math routines.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMath_H__
#define FxMath_H__

#include "FxPlatform.h"
#include "FxUtil.h"

// For now all platforms will use ANSI math routines.
#include "FxMathANSI.h"

namespace OC3Ent
{

namespace Face
{

/// Relaxed epsilon used in bone pruning.
/// \ingroup support
#define FX_REAL_RELAXED_EPSILON 0.001

/// Everyone's favorite constant.
/// \ingroup support
static const FxReal FX_PI = 3.1415926535f;

/// Everyone's favorite constant divided by two.
/// \ingroup support
static const FxReal FX_PI_OVER_TWO = 3.1415926535f / 2.0f;

/// FaceFX interpolation types.
enum FxInterpolationType
{
	IT_First      = 0,
	IT_Hermite    = IT_First,
	IT_Linear     = 1,
	IT_Last       = IT_Linear,
	IT_NumInterps = IT_Last + 1
};

/// Relaxed test of two FxReals for equality used in bone pruning.
FX_INLINE FxBool FX_CALL FxRealEqualityRelaxed( FxReal first, FxReal second )
{
	FxReal difference = first - second;
	return (difference < FX_REAL_RELAXED_EPSILON && difference > -FX_REAL_RELAXED_EPSILON);
}

/// Test two FxReals for equality.
FX_INLINE FxBool FX_CALL FxRealEquality( FxReal first, FxReal second )
{
	FxReal difference = first - second;
	return (difference < FX_REAL_EPSILON && difference > -FX_REAL_EPSILON);
}

/// Test two FxDReals for equality.
FX_INLINE FxBool FX_CALL FxDRealEquality( FxDReal first, FxDReal second )
{
	FxDReal difference = first - second;
	return (difference < FX_REAL_EPSILON && difference > -FX_REAL_EPSILON);
}

/// Hermite interpolation.
FX_INLINE FxReal FX_CALL FxHermiteInterpolate( FxReal start, FxReal end, FxReal alpha )
{
	FxReal a = FxClamp(0.0f, alpha, 1.0f);
	return a * (a * (a * (2.0f*start - 2.0f*end) + (-3.0f*start + 3.0f*end))) + start;
}

/// Linear interpolation.
FX_INLINE FxReal FX_CALL FxLinearInterpolate( FxReal start, FxReal end, FxReal alpha )
{
	FxReal a = FxClamp(0.0f, alpha, 1.0f);
	return (end - start) * a + start;
}

} // namespace Face

} // namespace OC3Ent

#endif
