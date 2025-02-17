//------------------------------------------------------------------------------
// This class implements a delta node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxDeltaNode_H__
#define FxDeltaNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

/// A delta node in an FxFaceGraph.
/// Delta nodes calculate a delta between the current value of the first input 
/// link and the value of the first input link the last time the delta node was 
/// evaluated.  Delta nodes are restricted to one input since %Face Graph nodes can 
/// only output one value at a time.
/// \ingroup faceGraph
class FxDeltaNode : public FxFaceGraphNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxDeltaNode, FxFaceGraphNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxDeltaNode)
public:
	/// FxActorInstances are our friends.
	friend class FxActorInstance;

	/// Constructor.
	FxDeltaNode();
	/// Destructor.
	virtual ~FxDeltaNode();

	/// Clones the delta node.
	virtual FxFaceGraphNode* Clone( void );
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Adds an input link to the delta node and returns FxTrue.
	/// Adds an input link to the delta node.  This is overridden so that the
	/// min and max of the delta node can be computed as -+(_inputs[0]._max -
	/// _inputs[0].min), respectively.
	/// \param newInputLink The new input link to add to the node.
	/// \return \p FxFalse if the input link was not added, \p FxTrue otherwise.
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );

	/// Serializes an FxDeltaNode to an archive.
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face

} // namespace OC3Ent

#endif
