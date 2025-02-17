//------------------------------------------------------------------------------
// This class implements a combiner node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxCombinerNode.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxCombinerNodeVersion 0

FX_IMPLEMENT_CLASS(FxCombinerNode, kCurrentFxCombinerNodeVersion, FxFaceGraphNode)

FxCombinerNode::FxCombinerNode()
{
}

FxCombinerNode::~FxCombinerNode()
{
}

FxFaceGraphNode* FxCombinerNode::Clone( void )
{
	FxCombinerNode* pNode = new FxCombinerNode();
	CopyData(pNode);
	return pNode;
}

void FxCombinerNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		Super::CopyData(pOther);
	}
}

void FxCombinerNode::Serialize( FxArchive& arc )
{
	Super::Serialize(arc);

	arc.SerializeClassVersion("FxCombinerNode");
}

} // namespace Face

} // namespace OC3Ent
