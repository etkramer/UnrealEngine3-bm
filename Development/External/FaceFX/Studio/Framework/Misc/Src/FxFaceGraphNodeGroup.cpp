//------------------------------------------------------------------------------
// Encapsulates a group of Face Graph nodes into a single named unit.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFaceGraphNodeGroup.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceGraphNodeVersion 0

FX_IMPLEMENT_CLASS(FxFaceGraphNodeGroup, kCurrentFxFaceGraphNodeVersion, FxNamedObject);

FxFaceGraphNodeGroup::FxFaceGraphNodeGroup()
{
}

FxFaceGraphNodeGroup::~FxFaceGraphNodeGroup()
{
}

FxSize FxFaceGraphNodeGroup::GetNumNodes( void ) const
{
	return _nodes.Length();
}

const FxName& FxFaceGraphNodeGroup::GetNode( FxSize index ) const
{
	return _nodes[index];
}

FxBool FxFaceGraphNodeGroup::IsNodeInGroup( const FxName& node ) const
{
	for( FxSize i = 0; i < _nodes.Length(); ++i )
	{
		if( _nodes[i] == node )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

void FxFaceGraphNodeGroup::AddNode( const FxName& node )
{
	if( !IsNodeInGroup(node) )
	{
		_nodes.PushBack(node);
	}
}

void FxFaceGraphNodeGroup::RemoveNode( const FxName& node )
{
	for( FxSize i = 0; i < _nodes.Length(); ++i )
	{
		if( _nodes[i] == node )
		{
			_nodes.Remove(i);
			break;
		}
	}
}

void FxFaceGraphNodeGroup::Serialize( FxArchive& arc )
{
	Super::Serialize( arc );

	arc.SerializeClassVersion("FxFaceGraphNodeGroup");
	
	arc << _nodes;
}

} // namespace Face

} // namespace OC3Ent
