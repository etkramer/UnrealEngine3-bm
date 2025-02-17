//------------------------------------------------------------------------------
// This class implements a combiner node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCombinerNode_H__
#define FxCombinerNode_H__

#include "FxPlatform.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

/// A combiner node in an FxFaceGraph.
/// A combiner node stores no data.  Its purpose is to modify values as they
/// flow through the %Face Graph, and to provide a place to store final values
/// that will be used elsewhere in the game system via the registers.
/// \ingroup faceGraph
class FxCombinerNode : public FxFaceGraphNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxCombinerNode, FxFaceGraphNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxCombinerNode)
public:
	/// Constructor.
	FxCombinerNode();
	/// Destructor.
	virtual ~FxCombinerNode();

	/// Clones the combiner node.
	virtual FxFaceGraphNode* Clone( void );
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Serializes an FxCombinerNode to an archive.
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face

} // namespace OC3Ent

#endif
