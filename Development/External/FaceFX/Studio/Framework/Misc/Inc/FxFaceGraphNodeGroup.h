//------------------------------------------------------------------------------
// Encapsulates a group of Face Graph nodes into a single named unit.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxFaceGraphNodeGroup_H__
#define FxFaceGraphNodeGroup_H__

#include "FxPlatform.h"
#include "FxNamedObject.h"
#include "FxArray.h"

namespace OC3Ent
{

namespace Face
{

// A group of face graph nodes.
class FxFaceGraphNodeGroup : public FxNamedObject
{
	// Declare the class.
	FX_DECLARE_CLASS(FxFaceGraphNodeGroup, FxNamedObject);
public:
	// Constructor.
	FxFaceGraphNodeGroup();
	// Destructor.
	virtual ~FxFaceGraphNodeGroup();

	// Returns the number of nodes in the group.
	FxSize GetNumNodes( void ) const;
	// Returns the node in the group at index.
	const FxName& GetNode( FxSize index ) const;

	// Query the group for a certain node.  If the node is in the group, FxTrue
	// is returned.  If the node is not in the group, FxFalse is returned.
	FxBool IsNodeInGroup( const FxName& node ) const;
	// Add a node to the group.
	void AddNode( const FxName& node );
	// Remove a node from the group.
	void RemoveNode( const FxName& node );

	// Serialization.
	virtual void Serialize( FxArchive& arc );

protected:
	// The nodes in the group.
	FxArray<FxName> _nodes;
};

} // namespace Face

} // namespace OC3Ent

#endif
