//------------------------------------------------------------------------------
// FaceFx utility code.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxUtil_H__
#define FxUtil_H__

#include "FxPlatform.h"

#ifdef FX_XBOX_360
	#include <ppcintrinsics.h>
#endif // FX_XBOX_360

namespace OC3Ent
{

namespace Face
{

/// Declares a function argument unused (for compiling in warning level 4).
#define FxUnused(var) /* var */

/// An invalid index.
/// \ingroup support
static const FxSize FxInvalidIndex = static_cast<FxSize>(-1);
/// An invalid value.
/// \ingroup support
static const FxReal FxInvalidValue = static_cast<FxReal>(FX_INT32_MIN);

/// Performs an XOR swap.
/// \ingroup support
#define FX_XOR_SWAP(x, y) (x)^=(y); (y)^=(x); (x)^=(y)

/// Performs byte swapping.
/// \ingroup support
FX_INLINE void FX_CALL FxByteSwap( FxByte* byte, FxSize size )
{
	FxSize i = 0;
	FxSize j = size - 1;
	while( i < j )
	{
		FX_XOR_SWAP(byte[i], byte[j]);
		i++, j--;
	}
}

/// Square function.
template<typename T> FX_INLINE T FX_CALL FxSquare( const T& a )
{
	return a * a;
}

/// Max function.
template<typename T> FX_INLINE T FX_CALL FxMax( const T& a, const T& b )
{
	return (a >= b) ? a : b;
}

#ifdef FX_XBOX_360
// Specialized version for FxReal on 360.
template<> FX_INLINE FxReal FX_CALL FxMax<FxReal>( const FxReal& a, const FxReal& b )
{
	return static_cast<FxReal>(fpmax(a, b));
}
// Specialized version for FxDReal on 360.
template<> FX_INLINE FxDReal FX_CALL FxMax<FxDReal>( const FxDReal& a, const FxDReal& b )
{
	return fpmax(a, b);
}
#endif // FX_XBOX_360

/// Min function.
template<typename T> FX_INLINE T FX_CALL FxMin( const T& a, const T& b )
{
	return (a <= b) ? a : b;
}

#ifdef FX_XBOX_360
// Specialized version for FxReal on 360.
template<> FX_INLINE FxReal FX_CALL FxMin<FxReal>( const FxReal& a, const FxReal& b )
{
	return static_cast<FxReal>(fpmin(a, b));
}
// Specialized version for FxDReal on 360.
template<> FX_INLINE FxDReal FX_CALL FxMin<FxDReal>( const FxDReal& a, const FxDReal& b )
{
	return fpmin(a, b);
}
#endif // FX_XBOX_360

/// Clamp function.
template<typename T> FX_INLINE T FX_CALL FxClamp( const T& valMin, const T& val, 
								                  const T& valMax )
{
	return FxMax(FxMin(val, valMax), valMin);
}

/// Swap function.
template<typename T> FX_INLINE void FX_CALL FxSwap( T& first, T& second )
{
	T temp = first;
	first  = second;
	second = temp;
}

/// Returns the nearest integer to an FxDReal value.
FX_INLINE FxInt32 FX_CALL FxNearestInteger( FxDReal value )
{
	FxInt32 result = 0;
	if( value > 0.0 )
	{
		value += 0.5;
		if( value >= FX_INT32_MAX )
		{
			result = FX_INT32_MAX;
		}
		else
		{
			result = static_cast<FxInt32>(value);
		}
	}
	else if( value < 0.0 )
	{
		value -= 0.5;
		if( value <= FX_INT32_MIN )
		{
			result = FX_INT32_MIN;
		}
		else
		{
			result = static_cast<FxInt32>(value);
		}
	}
	return result;
}

/// Returns the nearest integer to an FxReal value.
FX_INLINE FxInt32 FX_CALL FxNearestInteger( FxReal value )
{
	FxInt32 result = 0;
	if( value > 0.0f )
	{
		value += 0.5f;
		if( value >= FX_INT32_MAX )
		{
			result = FX_INT32_MAX;
		}
		else
		{
			result = static_cast<FxInt32>(value);
		}
	}
	else if( value < 0.0f )
	{
		value -= 0.5f;
		if( value <= FX_INT32_MIN )
		{
			result = FX_INT32_MIN;
		}
		else
		{
			result = static_cast<FxInt32>(value);
		}
	}
	return result;
}

} // namespace Face

} // namespace OC3Ent

#endif
