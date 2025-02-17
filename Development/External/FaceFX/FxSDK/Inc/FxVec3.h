//------------------------------------------------------------------------------
// This class defines a vector in 3-space.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxVec3_H__
#define FxVec3_H__

#include "FxPlatform.h"
#include "FxUtil.h"
#include "FxMath.h"
#include "FxArchive.h"

namespace OC3Ent
{

namespace Face
{

/// A vector in 3-space.
/// \ingroup support
class FxVec3
{
public:
	/// The x component of the vector.
	FxReal x;
	/// The y component of the vector.
	FxReal y;
	/// The z component of the vector.
	FxReal z;

	/// Default constructor.  Constructs the <0,0,0> vector.
	FxVec3() : x(0.0f), y(0.0f), z(0.0f) {}
	/// Constructor.  Constructs a vector <iX, iY, iZ>.
	FxVec3( FxReal iX, FxReal iY, FxReal iZ ) : x(iX), y(iY), z(iZ) {}

	/// Tests for vector equality.
	FxBool operator==( const FxVec3& other ) const;
	/// Tests for vector inequality.
	FxBool operator!=( const FxVec3& other ) const;

	/// Multiplies this vector by a scalar.
	FxVec3& operator*=( FxReal scalar );
	/// Multiplies the vector by a scalar.
	FxVec3 operator*( FxReal scalar ) const;
	/// Divides this vector by a scalar.
	FxVec3& operator/=( FxReal scalar );
	/// Divides this vector by a scalar.
	FxVec3 operator/( FxReal scalar ) const;
	/// Adds a scalar to this vector.
	FxVec3 operator+=( FxReal scalar );
	/// Adds a scalar to this vector.
	FxVec3 operator+( FxReal scalar ) const;
	/// Subtracts a scalar from this vector.
	FxVec3 operator-=( FxReal scalar );
	/// Subtracts a scalar from this vector.
	FxVec3 operator-( FxReal scalar ) const;

	/// Multiplies this vector with another vector.
	FxVec3& operator*=( const FxVec3& rhs );
	/// Multiplies this vector with another vector.
	FxVec3 operator*( const FxVec3& rhs ) const;
	/// Divides this vector by another vector.
	FxVec3& operator/=( const FxVec3& rhs );
	/// Divides this vector by another vector.
	FxVec3 operator/( const FxVec3& rhs ) const;
	/// Adds a vector to this vector.
	FxVec3& operator+=( const FxVec3& rhs );
	/// Adds a vector to the vector.
	FxVec3 operator+( const FxVec3& rhs ) const;
	/// Subtracts a vector from this vector.
	FxVec3& operator-=( const FxVec3& rhs );
	/// Subtracts a vector from the vector.
	FxVec3 operator-( const FxVec3& rhs ) const;

	/// Negates the vector.
	FxVec3 operator-( void ) const;

	/// Returns the length of the vector.
	FxReal Length( void ) const;
	/// Normalizes the vector.
	FxVec3& Normalize( void );
	/// Returns the normalized vector without changing the internal data.
	FxVec3 GetNormalized( void ) const;
	/// Sets the length of the vector.
	void SetLength( FxReal length );

	/// Resets the vector to <0,0,0>.
	void Reset( void );

	/// Linearly interpolates between this vector and another vector.
	/// \param other The vector to which to linearly interpolate.
	/// \param t Parametric time <tt>[0, 1]</tt>
	/// \return The linearly interpolated vector.
	/// For example, <tt>a.Lerp( b, 0.25f )</tt> would return the vector a quarter of the
	/// way from \a a to \a b.
	FxVec3 Lerp( const FxVec3& other, FxReal t ) const;

	/// Serialization.
	friend FxArchive& operator<<( FxArchive& arc, FxVec3& vec );
};

FxArchive& operator<<( FxArchive& arc, FxVec3& vec );

FX_INLINE FxReal DotProduct( const FxVec3& lhs, const FxVec3& rhs )
{ 
	return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z); 
}

FX_INLINE FxVec3 CrossProduct( const FxVec3& lhs, const FxVec3& rhs )
{ 
	return FxVec3(lhs.y * rhs.z - lhs.z * rhs.y, 
		          lhs.z * rhs.x - lhs.x * rhs.z, 
				  lhs.x * rhs.y - lhs.y * rhs.x ); 
}

//------------------------------------------------------------------------------
// Note that most binary arithmetic operators are not implemented in terms of 
// the corresponding assign/arithmetic operator for performance reasons.
//------------------------------------------------------------------------------

FX_INLINE FxBool FxVec3::operator==( const FxVec3& other ) const
{
	return FxRealEquality(other.x, x) && FxRealEquality(other.y, y) &&
		   FxRealEquality(other.z, z);
}

FX_INLINE FxBool FxVec3::operator!=( const FxVec3& other ) const
{
	return !operator==(other);
}

FX_INLINE FxVec3& FxVec3::operator*=( FxReal scalar )
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator*( FxReal scalar ) const
{
	return FxVec3(x * scalar, y * scalar, z * scalar);
}

FX_INLINE FxVec3& FxVec3::operator/=( FxReal scalar )
{
	if( scalar != 0.0f )
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
	}
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator/( FxReal scalar ) const
{
	if( scalar == 0.f ) 
	{
		scalar = 1.f;
	}
	else
	{
		// Invert scalar to save on some divisions.
		scalar = 1.f / scalar;
	}
	return FxVec3(x * scalar, y * scalar, z * scalar);
}

FX_INLINE FxVec3 FxVec3::operator+=( FxReal scalar )
{
	x += scalar;
	y += scalar;
	z += scalar;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator+( FxReal scalar ) const
{
	return FxVec3(x + scalar, y + scalar, z + scalar);
}

FX_INLINE FxVec3 FxVec3::operator-=( FxReal scalar )
{
	x -= scalar;
	y -= scalar;
	z -= scalar;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator-( FxReal scalar ) const
{
	return FxVec3(x - scalar, y - scalar, z - scalar);
}

FX_INLINE FxVec3& FxVec3::operator*=( const FxVec3& rhs )
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator*( const FxVec3& rhs ) const
{
	return FxVec3(x * rhs.x, y * rhs.y, z * rhs.z);
}

FX_INLINE FxVec3& FxVec3::operator/=( const FxVec3& rhs )
{
	if( rhs.x != 0.0f )
	{
		x /= rhs.x;
	}
	if( rhs.y != 0.0f )
	{
		y /= rhs.y;
	}
	if( rhs.z != 0.0f )
	{
		z /= rhs.z;
	}
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator/( const FxVec3& rhs ) const
{
	// Note this uses operator/=() to save on writing some nasty logic.  This
	// function is not performance-critical for the FaceFX SDK.
	return FxVec3(*this) /= rhs;
}

FX_INLINE FxVec3& FxVec3::operator+=( const FxVec3& rhs )
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator+( const FxVec3& rhs ) const
{
	return FxVec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

FX_INLINE FxVec3& FxVec3::operator-=( const FxVec3& rhs )
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

FX_INLINE FxVec3 FxVec3::operator-( const FxVec3& rhs ) const
{
	return FxVec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

FX_INLINE FxVec3 FxVec3::operator-( void ) const
{
	return FxVec3(-x, -y, -z);
}

FX_INLINE FxReal FxVec3::Length( void ) const
{
	return FxSqrt(FxSquare(x) + FxSquare(y) + FxSquare(z));
}

FX_INLINE FxVec3& FxVec3::Normalize( void )
{
	FxReal length = Length();
	if( length != 0.0f )
	{
		FxReal invLength = 1.0f / length;
		x *= invLength;
		y *= invLength;
		z *= invLength;
	}
	return *this;
}

FX_INLINE FxVec3 FxVec3::GetNormalized( void ) const
{
	FxVec3 temp = *this;
	temp.Normalize();
	return temp;
}

FX_INLINE void FxVec3::Reset( void )
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

FX_INLINE FxVec3 FxVec3::Lerp( const FxVec3& other, FxReal t ) const
{
	return (other - *this) * t;
}

} // namespace Face

} // namespace OC3Ent

#endif
