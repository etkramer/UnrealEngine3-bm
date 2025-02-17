//------------------------------------------------------------------------------
// Manages all of the face graph node groups in a session.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodeGroupManager_H__
#define FxFaceGraphNodeGroupManager_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxFaceGraphNodeGroup.h"

namespace OC3Ent
{

namespace Face
{

// Manages multiple groups of face graph nodes.
class FxFaceGraphNodeGroupManager
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxFaceGraphNodeGroupManager);
public:

	// The name of the group representing all nodes.
	static const FxString& GetAllNodesGroupName();

	// Returns the number of groups in the group manager.
	static FxSize GetNumGroups( void );

	// Add a group to the group manager.
	static void AddGroup( const FxName& group );
	// Remove a group from the group manager.
	static void RemoveGroup( const FxName& group );

	// Returns a pointer to the specified group or NULL if the group does not
	// exist.
	static FxFaceGraphNodeGroup* GetGroup( const FxName& group );
	// Returns a pointer to the group at index.
	static FxFaceGraphNodeGroup* GetGroup( FxSize index );

	// Removes all groups from the manager.
	static void FlushAllGroups( void );
	
	// Serialization.
	static void Serialize( FxArchive& arc );

protected:
	// The face graph node groups.
	static FxArray<FxFaceGraphNodeGroup> _groups;
	// The all node groups name
	static FxString _allNodesGroup;
};

} // namespace Face

} // namespace OC3Ent

#endif
