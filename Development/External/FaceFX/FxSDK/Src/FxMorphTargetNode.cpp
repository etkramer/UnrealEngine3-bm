//------------------------------------------------------------------------------
// This class makes it easy to implement FaceFX controlled morph targets.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMorphTargetNode.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxMorphTargetNodeVersion 0

FX_IMPLEMENT_CLASS(FxMorphTargetNode, kCurrentFxMorphTargetNodeVersion, FxGenericTargetNode)

FxMorphTargetNode::FxMorphTargetNode()
{
#ifdef __UNREAL__
	_isPlaceable = FxFalse;
#else
	_isPlaceable = FxTrue;
#endif // __UNREAL__
	FxFaceGraphNodeUserProperty morphTargetNameProperty(UPT_String);
	morphTargetNameProperty.SetName("Morph Target Name");
	morphTargetNameProperty.SetStringProperty(FxString("Target"));
	AddUserProperty(morphTargetNameProperty);
}

FxMorphTargetNode::~FxMorphTargetNode()
{
}

FxFaceGraphNode* FxMorphTargetNode::Clone( void )
{
	FxMorphTargetNode* pNode = new FxMorphTargetNode();
	CopyData(pNode);
	return pNode;
}

void FxMorphTargetNode::CopyData( FxFaceGraphNode* pOther )
{
	Super::CopyData(pOther);
}

void FxMorphTargetNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxMorphTargetNode");
}

} // namespace Face

} // namespace OC3Ent
