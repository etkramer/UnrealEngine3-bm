//------------------------------------------------------------------------------
// This class defines a vector in 2-space.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxVec2_H__
#define FxVec2_H__

#include "FxPlatform.h"
#include "FxUtil.h"
#include "FxMath.h"
#include "FxArchive.h"

namespace OC3Ent
{

namespace Face
{

/// A vector in 2-space.
/// \ingroup support
class FxVec2
{
public:
	/// The x component of the vector.
	FxReal x;
	/// The y component of the vector.
	FxReal y;

	/// Default constructor.  Constructs <0, 0>.
	FxVec2() : x(0.0f), y(0.0f) {}
	/// Constructor.  Constructs <iX, iY>.
	FxVec2( FxReal iX, FxReal iY ) : x(iX), y(iY) {}

	/// Tests for vector equality.
	FxBool operator==( const FxVec2& other ) const;
	/// Tests for vector inequality.
	FxBool operator!=( const FxVec2& other ) const;

	/// Multiplies this vector by a scalar.
	FxVec2& operator*=( FxReal scalar );
	/// Multiplies the vector by a scalar.
	FxVec2 operator*( FxReal scalar ) const;
	/// Divides this vector by a scalar.
	FxVec2& operator/=( FxReal scalar );
	/// Divides this vector by a scalar.
	FxVec2 operator/( FxReal scalar ) const;
	/// Adds a scalar to this vector.
	FxVec2 operator+=( FxReal scalar );
	/// Adds a scalar to this vector.
	FxVec2 operator+( FxReal scalar ) const;
	/// Subtracts a scalar from this vector.
	FxVec2 operator-=( FxReal scalar );
	/// Subtracts a scalar from this vector.
	FxVec2 operator-( FxReal scalar ) const;

    /// Multiplies this vector with another vector.
	FxVec2& operator*=( const FxVec2& rhs );
	/// Multiplies this vector with another vector.
	FxVec2 operator*( const FxVec2& rhs ) const;
	/// Divides this vector by another vector.
	FxVec2& operator/=( const FxVec2& rhs );
	/// Divides this vector by another vector.
	FxVec2 operator/( const FxVec2& rhs ) const;
	/// Adds a vector to this vector.
	FxVec2& operator+=( const FxVec2& rhs );
	/// Adds a vector to the vector.
	FxVec2 operator+( const FxVec2& rhs ) const;
	/// Subtracts a vector from this vector.
	FxVec2& operator-=( const FxVec2& rhs );
	/// Subtracts a vector from the vector.
	FxVec2 operator-( const FxVec2& rhs ) const;

	/// Negates the vector.
	FxVec2 operator-( void ) const;

	/// Returns the length of the vector.
	FxReal Length( void ) const;
	/// Returns the squared length of the vector.
	FxReal LengthSquared( void ) const;
	/// Normalizes the vector.
	FxVec2& Normalize( void );
	/// Returns the normalized vector without changing the internal data.
	FxVec2 GetNormalized( void ) const;
	/// Sets the length of the vector.
	void SetLength( FxReal length );

	/// Resets the vector to <0,0>.
	void Reset( void );

	/// Linearly interpolates between this vector and another vector.
	/// \param other The vector to which to linearly interpolate.
	/// \param t Parametric time <tt>[0, 1]</tt>
	/// \return The linearly interpolated vector.
	/// For example, <tt>a.Lerp( b, 0.25f )</tt> would return the vector a quarter of the
	/// way from \a a to \a b.
	FxVec2 Lerp( const FxVec2& other, FxReal t ) const;

	/// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, FxVec2& vec );
};

FxArchive& operator<<( FxArchive& arc, FxVec2& vec );

FX_INLINE FxBool FxVec2::operator==( const FxVec2& other ) const
{
	return FxRealEquality(x, other.x) && FxRealEquality(y, other.y);
}

FX_INLINE FxBool FxVec2::operator!=( const FxVec2& other ) const
{
	return !operator==(other);
}

FX_INLINE FxVec2& FxVec2::operator*=( FxReal scalar )
{
	x *= scalar;
	y *= scalar;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator*( FxReal scalar ) const
{
	return FxVec2(*this) *= scalar;
}

FX_INLINE FxVec2& FxVec2::operator/=( FxReal scalar )
{
	if( scalar != 0.0f )
	{
		x /= scalar;
		y /= scalar;
	}
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator/( FxReal scalar ) const
{
	return FxVec2(*this) /= scalar;
}

FX_INLINE FxVec2 FxVec2::operator+=( FxReal scalar )
{
	x += scalar;
	y += scalar;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator+( FxReal scalar ) const
{
	return FxVec2(*this) += scalar;
}

FX_INLINE FxVec2 FxVec2::operator-=( FxReal scalar )
{
	x -= scalar;
	y -= scalar;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator-( FxReal scalar ) const
{
	return FxVec2(*this) -= scalar;
}

FX_INLINE FxVec2& FxVec2::operator*=( const FxVec2& rhs )
{
	x *= rhs.x;
	y *= rhs.y;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator*( const FxVec2& rhs ) const
{
	return FxVec2(*this) *= rhs;
}

FX_INLINE FxVec2& FxVec2::operator/=( const FxVec2& rhs )
{
	if( rhs.x != 0.0f )
	{
		x /= rhs.x;
	}
	if( rhs.y != 0.0f )
	{
		y /= rhs.y;
	}
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator/( const FxVec2& rhs ) const
{
	return FxVec2(*this) /= rhs;
}

FX_INLINE FxVec2& FxVec2::operator+=( const FxVec2& rhs )
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator+( const FxVec2& rhs ) const
{
	return FxVec2(*this) += rhs;
}

FX_INLINE FxVec2& FxVec2::operator-=( const FxVec2& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

FX_INLINE FxVec2 FxVec2::operator-( const FxVec2& rhs ) const
{
	return FxVec2(*this) -= rhs;
}

FX_INLINE FxVec2 FxVec2::operator-( void ) const
{
	return FxVec2(-x, -y);
}

FX_INLINE FxReal FxVec2::Length( void ) const
{
	return FxSqrt(FxSquare(x) + FxSquare(y));
}

FX_INLINE FxReal FxVec2::LengthSquared( void ) const
{
	return FxSquare(x) + FxSquare(y);
}

FX_INLINE FxVec2& FxVec2::Normalize( void )
{
	FxReal length = Length();
	if( length != 0.0f )
	{
		FxReal invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;
	}
	return *this;
}

FX_INLINE FxVec2 FxVec2::GetNormalized( void ) const
{
	FxVec2 temp = *this;
	temp.Normalize();
	return temp;
}

FX_INLINE void FxVec2::Reset( void )
{
	x = 0.0f;
	y = 0.0f;
}

FX_INLINE FxVec2 FxVec2::Lerp( const FxVec2& other, FxReal t ) const
{
	return (other - *this) * t;
}

} // namespace Face

} // namespace OC3Ent

#endif
