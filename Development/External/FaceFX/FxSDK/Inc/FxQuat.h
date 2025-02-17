//------------------------------------------------------------------------------
// This class implements a quaternion.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxQuat_H__
#define FxQuat_H__

#include "FxPlatform.h"
#include "FxUtil.h"
#include "FxMath.h"
#include "FxArchive.h"

namespace OC3Ent
{

namespace Face
{

/// A quaternion.
/// \ingroup support
class FxQuat
{
public:

	/// The w component of the quaternion.
	FxReal w;
	/// The x component of the quaternion.
	FxReal x;
	/// The y component of the quaternion.
	FxReal y;
	/// The z component of the quaternion.
	FxReal z;

	/// Default Constructor.  Constructs <1,0,0,0>.
	FxQuat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
	/// Constructor.  Constructs <iW,iX,iY,iZ>.
	FxQuat( FxReal iW, FxReal iX, FxReal iY, FxReal iZ ) 
		: w(iW), x(iX), y(iY), z(iZ) {}

	/// Tests for quaternion equality.
	FxBool operator==( const FxQuat& other ) const;
	/// Tests for quaternion inequality.
	FxBool operator!=( const FxQuat& other ) const;

	/// Multiplies the quaternion by another.
	FxQuat operator*( const FxQuat& rhs ) const;
	/// Multiplies this quaternion by another.
	const FxQuat& operator*=( const FxQuat& rhs );

	/// Quaternion addition.
	FxQuat operator+( const FxQuat& rhs ) const;
	/// Quaternion addition.
	const FxQuat& operator+=( const FxQuat& rhs );

	/// Quaternion subtraction.
	FxQuat operator-( const FxQuat& rhs ) const;
	/// Quaternion subtraction.
	const FxQuat& operator-=( const FxQuat& rhs );

	/// Multiplies the quaternion by a scalar.
	FxQuat operator*( const FxReal scalar ) const;
	/// Multiplies this quaternion by a scalar.
	const FxQuat& operator*=( const FxReal scalar );

	/// Divides the quaternion by a scalar.  Note that scalar is not checked
	/// for zero.
	FxQuat operator/( const FxReal scalar ) const;
	/// Divides this quaternion by a scalar.  Note that scalar is not checked
	/// for zero.
	const FxQuat& operator/=( const FxReal scalar );

	void ToEuler( FxReal& yaw, FxReal& pitch, FxReal& roll )
	{
		roll   = FxAtan((2.0f*(w*z + x*y))/(1.0f-(2.0f*(FxSquare(y)+FxSquare(z)))));
		pitch = FxAsin(2.0f*(w*y - z*x));
		yaw  = FxAtan((2.0f*(w*x + y*z))/(1.0f-(2.0f*(FxSquare(x)+FxSquare(y)))));
	}

	/// Returns the inverse of the quaternion.
	FxQuat GetInverse( void ) const;

	/// Normalizes this quaternion.
	/// \return A reference to this, so it can be used in a larger expression.
	const FxQuat& Normalize( void );

	/// Resets the quaternion to identity <1,0,0,0>.
	void Reset( void );

	/// Force this quaternion to point to same side of the hypersphere as other.
	/// Assumes all quaternions involved are unit (normalized) quaternions
	/// and thus does not normalize the result.
	/// \param other The quaternion with which to align.
	void AlignWith( const FxQuat& other );

	/// Corner-cutting spherical linear interpolation.
	/// \param other The quaternion to which to slerp.
	/// \param t Parametric time <tt>[0, 1]</tt>.
	/// \return The slerped quaternion.
	/// Assumes all quaternions involved are unit (normalized) quaternions
	/// and thus does not normalize the result.
	FxQuat Slerp( const FxQuat& other, FxReal t );

	/// Serializes a quaternion to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxQuat& quat );
};

FxArchive& operator<<( FxArchive& arc, FxQuat& quat );

FX_INLINE FxBool FxQuat::operator==( const FxQuat& other ) const
{
	return FxRealEquality(other.w, w) && FxRealEquality(other.x, x) && 
		   FxRealEquality(other.y, y) && FxRealEquality(other.z, z);
}

FX_INLINE FxBool FxQuat::operator!=( const FxQuat& other ) const
{
	return !operator==(other);
}

FX_INLINE FxQuat FxQuat::operator*( const FxQuat &rhs ) const
{
	return FxQuat(
		w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,	// w.
		w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,	// x.
		w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,	// y.
		w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w); // z.
}

FX_INLINE const FxQuat& FxQuat::operator*=( const FxQuat &rhs )
{
	FxReal iW = w;
	FxReal iX = x;
	FxReal iY = y;
	FxReal iZ = z;
	w = iW * rhs.w - iX * rhs.x - iY * rhs.y - iZ * rhs.z;
	x = iW * rhs.x + iX * rhs.w + iY * rhs.z - iZ * rhs.y;
	y = iW * rhs.y - iX * rhs.z + iY * rhs.w + iZ * rhs.x;
	z = iW * rhs.z + iX * rhs.y - iY * rhs.x + iZ * rhs.w;
	return *this;
}

FX_INLINE FxQuat FxQuat::operator+( const FxQuat& rhs ) const
{
	return FxQuat(w + rhs.w, x + rhs.x, y + rhs.y, z + rhs.z);
}

FX_INLINE const FxQuat& FxQuat::operator+=( const FxQuat& rhs )
{
	*this = *this + rhs;
	return *this;
}

FX_INLINE FxQuat FxQuat::operator-( const FxQuat& rhs ) const
{
	return FxQuat(w - rhs.w, x - rhs.x, y - rhs.y, z - rhs.z);
}

FX_INLINE const FxQuat& FxQuat::operator-=( const FxQuat& rhs )
{
	*this = *this - rhs;
	return *this;
}

FX_INLINE FxQuat FxQuat::operator*( const FxReal scalar ) const
{
	return FxQuat(w * scalar, x * scalar, y * scalar, z * scalar);
}

FX_INLINE const FxQuat& FxQuat::operator*=( const FxReal scalar )
{
	*this = *this * scalar;
	return *this;
}

FX_INLINE FxQuat FxQuat::operator/( const FxReal scalar ) const
{
	return FxQuat(w / scalar, x / scalar, y / scalar, z / scalar);
}

FX_INLINE const FxQuat& FxQuat::operator/=( const FxReal scalar )
{
	*this = *this / scalar;
	return *this;
}

FX_INLINE FxQuat FxQuat::GetInverse( void ) const 
{ 
	return FxQuat(w, -x, -y, -z); 
}

FX_INLINE const FxQuat& FxQuat::Normalize( void )
{
	FxReal norm = FxSquare(w) + FxSquare(x) + FxSquare(y) + FxSquare(z);
	if( !FxRealEquality(norm, 1.0f) )
	{
		norm = FxSqrt(norm);
		if( norm < FX_REAL_EPSILON )
		{
			w = 1.0f;
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		else
		{
			// Invert norm to save on divisions.
			norm = 1.0f / norm;
			w *= norm;
			x *= norm;
			y *= norm;
			z *= norm;
		}
	}
	return *this;
}

FX_INLINE void FxQuat::Reset( void )
{
	w = 1.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

FX_INLINE void FxQuat::AlignWith( const FxQuat& other )
{
	FxReal componentDifference = FxSquare(x-other.x) + FxSquare(y-other.y) + 
								 FxSquare(z-other.z) + FxSquare(w-other.w);
	FxReal componentSum        = FxSquare(x+other.x) + FxSquare(y+other.y) + 
								 FxSquare(z+other.z) + FxSquare(w+other.w);

	if( componentDifference > componentSum )
	{
		w = -w;	
		x = -x;
		y = -y;
		z = -z;
	}
}

// Assumes all quaternions involved are unit (normalized) quaternions
// and as such does not normalize the result.  This optimized technique is
// based on the Inner Product article "Hacking Quaternions" by Jonathan Blow 
// found in the March 2002 issue of Game Developer Magazine.
FX_INLINE FxQuat FxQuat::Slerp( const FxQuat& other, FxReal t )
{
	// Ensure this quaternion and the other point to the same side of the 
	// hypersphere.  This is essential because a property of quaternions is 
	// that q and -q represent the same end rotation.  However, one will 
	// produce a path along the greater great-arc around the hypersphere, so 
	// we need to be sure and use the one that produces the shorter great-arc.
	AlignWith(other);

	// Calculate the cosine of the angle (theta) between the two quaternions.
	FxReal cosTheta = x * other.x + y * other.y + z * other.z + w * other.w;

	// As a speed optimization, linearly interpolate each component of the 
	// quaternions based on a cutoff of the cosine of the angle between them.  
	// The constant 0.55f was tweaked by hand.
	if( cosTheta > 0.55f )
	{
		// Linear interpolation.
		FxReal scale0 = 1.0f - t;
		return FxQuat(
			scale0 * w + t * other.w,
			scale0 * x + t * other.x,
			scale0 * y + t * other.y,
			scale0 * z + t * other.z);
	}
	// If all else fails, actually do the spherical linear interpolation.  
	// This case is very rare because normally the quaternions are not so 
	// far "apart."
	else
	{	
		// Slerp.
		FxReal theta = FxAcos(cosTheta);
		FxReal sininv = 1.0f / FxSin(theta);
		FxReal scale0 = FxSin((1.0f - t) * theta) * sininv;
		FxReal scale1 = FxSin(t * theta) * sininv;

		return FxQuat(
			scale0 * w + scale1 * other.w,
			scale0 * x + scale1 * other.x,
			scale0 * y + scale1 * other.y,
			scale0 * z + scale1 * other.z);	
	}
}


} // namespace Face

} // namespace OC3Ent

#endif
