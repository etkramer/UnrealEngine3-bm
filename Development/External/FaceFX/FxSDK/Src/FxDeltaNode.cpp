//------------------------------------------------------------------------------
// This class implements a delta node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxDeltaNode.h"
#include "FxMath.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxDeltaNodeVersion 0

FX_IMPLEMENT_CLASS(FxDeltaNode, kCurrentFxDeltaNodeVersion, FxFaceGraphNode)

FxDeltaNode::FxDeltaNode()
{
}

FxDeltaNode::~FxDeltaNode()
{
}

FxFaceGraphNode* FxDeltaNode::Clone( void )
{
	FxDeltaNode* pNode = new FxDeltaNode();
	CopyData(pNode);
	return pNode;
}

void FxDeltaNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		Super::CopyData(pOther);
	}
}

FxBool FxDeltaNode::AddInputLink( const FxFaceGraphNodeLink& newInputLink )
{
	if( _inputs.Length() == 0 )
	{
		Super::AddInputLink(newInputLink);
		const FxFaceGraphNode* pNode = _inputs[0].GetNode();
		if( pNode )
		{
			FxReal difference =  pNode->GetMax() - pNode->GetMin();
			SetMax(difference);
			SetMin(-difference);
		}
		return FxTrue;
	}
	return FxFalse;
}

void FxDeltaNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxDeltaNode");
}
	
} // namespace Face

} // namespace OC3Ent
