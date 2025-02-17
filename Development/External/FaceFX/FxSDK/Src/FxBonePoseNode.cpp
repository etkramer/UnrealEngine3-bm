//------------------------------------------------------------------------------
// This class implements a bone pose node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxBonePoseNode.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxBonePoseNodeVersion 0

FX_IMPLEMENT_CLASS(FxBonePoseNode, kCurrentFxBonePoseNodeVersion, FxFaceGraphNode)

FxBonePoseNode::FxBonePoseNode()
{
	_isPlaceable = FxFalse;
}

FxBonePoseNode::~FxBonePoseNode()
{
}

FxFaceGraphNode* FxBonePoseNode::Clone( void )
{
	FxBonePoseNode* pNode = new FxBonePoseNode();
	CopyData(pNode);
	return pNode;
}

void FxBonePoseNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		Super::CopyData(pOther);
		FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(pOther);
		if( pBonePoseNode )
		{
			pBonePoseNode->_bones = _bones;
		}
	}
}

void FxBonePoseNode::AddBone( const FxBone& bone )
{
	FxBone temp(bone);
	FxQuat rot = temp.GetRot();
	rot.Normalize();
	temp.SetRot(rot);
	_bones.PushBack(temp);
}

void FxBonePoseNode::RemoveBone( FxSize index )
{
	_bones.Remove(index);
}

void FxBonePoseNode::RemoveAllBones( void )
{
	_bones.Clear();
}

void FxBonePoseNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxBonePoseNode");
	
	arc << _bones;
}

} // namespace Face

} // namespace OC3Ent
