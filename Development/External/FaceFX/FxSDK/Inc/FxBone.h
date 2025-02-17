//------------------------------------------------------------------------------
// This class defines a FaceFx bone.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxBone_H__
#define FxBone_H__

#include "FxPlatform.h"
#include "FxQuat.h"
#include "FxVec3.h"
#include "FxArchive.h"
#include "FxNamedObject.h"

namespace OC3Ent
{

namespace Face
{

/// A bone, storing position, rotation and scale.
class FxBone : public FxNamedObject
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxBone, FxNamedObject)
public:
	/// Constructor.
	FxBone();
	/// Construct a bone from a name, position, rotation, and scale.
	FxBone( const FxName& name, const FxVec3& iPos, const FxQuat& iRot,
		    const FxVec3& iScale );
	/// Destructor.
	virtual ~FxBone();

	/// Tests for bone equality.
	FxBool operator==( const FxBone& other ) const;
	/// Tests for bone inequality.
	FxBool operator!=( const FxBone& other ) const;

	/// Returns the position component.
	FX_INLINE const FxVec3& GetPos( void ) const { return _pos; }
	/// Returns the rotation component.
	FX_INLINE const FxQuat& GetRot( void ) const { return _rot; }
	/// Returns the scale component.
	FX_INLINE const FxVec3& GetScale( void ) const { return _scale; }

	/// Sets the position component.
	void SetPos( const FxVec3& iPos );
	/// Sets the rotation component.
	void SetRot( const FxQuat& iRot );
	/// Sets the scale component.
	void SetScale( const FxVec3& iScale );

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

private:
	/// The position component.
	FxVec3 _pos;
	/// The rotation component.
	FxQuat _rot;
	/// The scale component.
	FxVec3 _scale;
};

FX_INLINE FxBool FxBone::operator==( const FxBone& other ) const
{
	return(GetName() == other.GetName()                    &&
		   FxRealEqualityRelaxed(_pos.x, other._pos.x)     &&
		   FxRealEqualityRelaxed(_pos.y, other._pos.y)     &&
		   FxRealEqualityRelaxed(_pos.z, other._pos.z)     &&
		   FxRealEqualityRelaxed(_rot.w, other._rot.w)     &&
		   FxRealEqualityRelaxed(_rot.x, other._rot.x)     &&
		   FxRealEqualityRelaxed(_rot.y, other._rot.y)     &&
		   FxRealEqualityRelaxed(_rot.z, other._rot.z)     &&
		   FxRealEqualityRelaxed(_scale.x, other._scale.x) &&
		   FxRealEqualityRelaxed(_scale.y, other._scale.y) &&
		   FxRealEqualityRelaxed(_scale.z, other._scale.z));
}

FX_INLINE FxBool FxBone::operator!=( const FxBone& other ) const
{
	return !(operator==(other));
}

} // namespace Face

} // namespace OC3Ent

#endif
