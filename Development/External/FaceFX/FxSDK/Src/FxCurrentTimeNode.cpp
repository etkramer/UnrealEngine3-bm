//------------------------------------------------------------------------------
// This class implements a current time node in the face graph.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxCurrentTimeNode.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxCurrentTimeNodeVersion 0

FX_IMPLEMENT_CLASS(FxCurrentTimeNode, kCurrentFxCurrentTimeNodeVersion, FxFaceGraphNode)

FxCurrentTimeNode::FxCurrentTimeNode()
{
}

FxCurrentTimeNode::~FxCurrentTimeNode()
{
}

FxFaceGraphNode* FxCurrentTimeNode::Clone( void )
{
	FxCurrentTimeNode* pNode = new FxCurrentTimeNode();
	CopyData(pNode);
	return pNode;
}

void FxCurrentTimeNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		Super::CopyData(pOther);
	}
}

FxBool FxCurrentTimeNode::AddInputLink( const FxFaceGraphNodeLink& FxUnused(newInputLink) )
{
	return FxFalse;
}

void FxCurrentTimeNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxCurrentTimeNode");
}

} // namespace Face

} // namespace OC3Ent
