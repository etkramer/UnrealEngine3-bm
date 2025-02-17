//------------------------------------------------------------------------------
// This class implements a current time node in the %Face Graph.
//
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurrentTimeNode_H__
#define FxCurrentTimeNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

/// A current time node in an FxFaceGraph.
/// Current time nodes receive the current time during the
/// \ref OC3Ent::Face::FxActorInstance::Tick() "FxActorInstance::Tick()" call,
/// and that time is passed out as the value of the node in GetValue().
/// \ingroup faceGraph
class FxCurrentTimeNode : public FxFaceGraphNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxCurrentTimeNode, FxFaceGraphNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCurrentTimeNode)
public:
	/// Constructor.
	FxCurrentTimeNode();
	/// Destructor.
	virtual ~FxCurrentTimeNode();

	/// Clones the current time node.
	virtual FxFaceGraphNode* Clone( void );
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Disallow adding input links to this type of node.
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );

	/// Serializes the current time node to an archive.
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face

} // namespace OC3Ent

#endif
