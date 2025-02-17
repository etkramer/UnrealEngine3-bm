//------------------------------------------------------------------------------
// Manages all of the face graph node groups in a session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"
#include "FxFaceGraphNodeGroupManager.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxFaceNodeGroupManagerVersion 0

FxString FxFaceGraphNodeGroupManager::_allNodesGroup;
FxArray<FxFaceGraphNodeGroup> FxFaceGraphNodeGroupManager::_groups;

const FxString& FxFaceGraphNodeGroupManager::GetAllNodesGroupName()
{
	if( _allNodesGroup.IsEmpty() )
	{
		_allNodesGroup = "All Nodes";
	}
	return _allNodesGroup;
}

FxSize FxFaceGraphNodeGroupManager::GetNumGroups( void )
{
	return _groups.Length();
}

void FxFaceGraphNodeGroupManager::AddGroup( const FxName& group )
{
	if( NULL == GetGroup(group) )
	{
		FxFaceGraphNodeGroup newGroup;
		newGroup.SetName(group);
		_groups.PushBack(newGroup);
	}
}

void FxFaceGraphNodeGroupManager::RemoveGroup( const FxName& group )
{
	for( FxSize i = 0; i < _groups.Length(); ++i )
	{
		if( _groups[i].GetName() == group )
		{
			_groups.Remove(i);
			break;
		}
	}
}

FxFaceGraphNodeGroup* FxFaceGraphNodeGroupManager::GetGroup( const FxName& group )
{
	for( FxSize i = 0; i < _groups.Length(); ++i )
	{
		if( _groups[i].GetName() == group )
		{
			return &_groups[i];
		}
	}
	return NULL;
}

FxFaceGraphNodeGroup* FxFaceGraphNodeGroupManager::GetGroup( FxSize index )
{
	return &_groups[index];
}

void FxFaceGraphNodeGroupManager::FlushAllGroups( void )
{
	_groups.Clear();
}

void FxFaceGraphNodeGroupManager::Serialize( FxArchive& arc )
{
	arc.SerializeClassVersion("FxFaceGraphNodeGroupManager", FxTrue, kCurrentFxFaceNodeGroupManagerVersion);

	arc << _groups;
}

} // namespace Face

} // namespace OC3Ent
