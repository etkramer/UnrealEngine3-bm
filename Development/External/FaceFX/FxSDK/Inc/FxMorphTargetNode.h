//------------------------------------------------------------------------------
// This class makes it easy to implement FaceFX controlled morph targets.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMorphTargetNode_H__
#define FxMorphTargetNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxGenericTargetNode.h"

namespace OC3Ent
{

namespace Face
{

/// The index where the "Morph Target Name" user property is located in the
/// FxMorphTargetNode's user properties.  Use this index when calling
/// FxFaceGraphNode::GetUserProperty() rather than calling 
/// FxFaceGraphNode::FindUserProperty().
/// \ingroup faceGraph
#define FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX 0

//@todo compiled Revise this documentation.
/// A morph target node in an FxFaceGraph.
/// A morph target node is a node in the %Face Graph that allows the programmer
/// to easily add FaceFX controlled morph targets.  The FxMorphTargetNode 
/// contains the name of the morph target it wishes to control as a user 
/// property of the node.  When dealing with %Face Graph node types that reference
/// game engine data via FxGenericTargetProxy derived objects, it is usually
/// necessary to perform additional steps during the render loop for each 
/// FxActorInstance to hook the proxies to the render data for that particular
/// instance because the %Face Graph is shared among all FxActorInstances of
/// a particular FxActor.
/// \ingroup faceGraph
class FxMorphTargetNode : public FxGenericTargetNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxMorphTargetNode, FxGenericTargetNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxMorphTargetNode)
public:
	/// Constructor.
	FxMorphTargetNode();
	/// Destructor.
	virtual ~FxMorphTargetNode();

	/// Clones the morph target node.
	virtual FxFaceGraphNode* Clone( void );

	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Serializes an FxMorphTargetNode to an archive.
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face

} // namespace OC3Ent

#endif
