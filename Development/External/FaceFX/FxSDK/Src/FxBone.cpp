//------------------------------------------------------------------------------
// This class defines a FaceFx bone.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxBone.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxBoneVersion 0

FX_IMPLEMENT_CLASS(FxBone, kCurrentFxBoneVersion, FxNamedObject)

FxBone::FxBone()
{
}

FxBone::FxBone( const FxName& name, const FxVec3& iPos, 
				const FxQuat& iRot, const FxVec3& iScale )
				: Super(name)
				, _pos(iPos)
				, _rot(iRot)
				, _scale(iScale)
{
}

FxBone::~FxBone()
{
}

void FxBone::SetPos( const FxVec3& iPos )
{
	_pos = iPos;
}

void FxBone::SetRot( const FxQuat& iRot )
{
	_rot = iRot;
}

void FxBone::SetScale( const FxVec3& iScale )
{
	_scale = iScale;
}

void FxBone::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxBone");

	arc << _pos << _rot << _scale;
}

} // namespace Face

} // namespace OC3Ent
