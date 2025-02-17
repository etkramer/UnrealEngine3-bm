//------------------------------------------------------------------------------
// This class implements a bone pose node in an FxFaceGraph.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxBonePoseNode_H__
#define FxBonePoseNode_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxArray.h"
#include "FxBone.h"
#include "FxFaceGraphNode.h"

namespace OC3Ent
{

namespace Face
{

/// A bone pose node in an FxFaceGraph.
/// A bone pose node is a node in the %Face Graph which, in addition to performing
/// all the responsibilities of a FxFaceGraphNode, stores the bones of a bone pose.
/// \ingroup faceGraph
class FxBonePoseNode : public FxFaceGraphNode
{
	/// Declare the class.
	FX_DECLARE_CLASS(FxBonePoseNode, FxFaceGraphNode)
	/// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxBonePoseNode)
public:
	/// Constructor.
	FxBonePoseNode();
	/// Destructor.
	virtual ~FxBonePoseNode();

	/// Clones the bone pose node.
	virtual FxFaceGraphNode* Clone( void );
	/// Copies the data from this object into the other object.
	virtual void CopyData( FxFaceGraphNode* pOther );

	/// Returns the number of bones in this node.
	FX_INLINE FxSize GetNumBones( void ) const { return _bones.Length(); }
	
	/// Returns a constant reference to the bone at index.
	FX_INLINE const FxBone& GetBone( FxSize index ) const { return _bones[index]; }
	
	/// Adds the bone to the node.
	void AddBone( const FxBone& bone );
	/// Removes the bone at index.
	void RemoveBone( FxSize index );
	/// Removes all bones contained in the node.
	void RemoveAllBones( void );

	/// Serialization.
	virtual void Serialize( FxArchive& arc );

private:
	/// The array of bones.
	FxArray<FxBone> _bones;
};

} // namespace Face

} // namespace OC3Ent

#endif
