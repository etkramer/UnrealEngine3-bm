//------------------------------------------------------------------------------
// This class defines the master bone list for an FxActor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxMasterBoneList_H__
#define FxMasterBoneList_H__

#include "FxPlatform.h"
#include "FxArchive.h"
#include "FxArray.h"
#include "FxBone.h"
#include "FxFaceGraph.h"
#include "FxCompiledFaceGraph.h"
#include "FxBonePoseNode.h"

namespace OC3Ent
{

namespace Face
{

/// The master bone list for an actor.
/// An actor's master bone list contains the reference position of all the bones
/// in the bone pose nodes.
/// \ingroup blending
class FxMasterBoneList
{
public:
	/// Constructor.
	FxMasterBoneList();
	/// Destructor.
	virtual ~FxMasterBoneList();

	/// Removes all reference bones from the master bone list.
	void Clear( void );

	/// Returns the number of reference bones.
	FX_INLINE FxSize GetNumRefBones( void ) const { return _refBones.Length(); }

	/// Returns a reference bone.
	FX_INLINE const FxBone& GetRefBone( FxSize index ) const { return _refBones[index].refBone; }

	/// Add a reference bone.
	void AddRefBone( const FxBone& bone );

	/// Set a reference bone's index in the client animation system.
	void SetRefBoneClientIndex( FxSize refBoneIndex, FxInt32 clientIndex );
	/// Retrieve a reference bone's index in the client animation system.
	FX_INLINE FxInt32 GetRefBoneClientIndex( FxSize refBoneIndex ) const { return _refBones[refBoneIndex].index; }
	
	/// Sets the weight of the reference bone named 'refBoneName'.
	/// \note For internal use only.
	void SetRefBoneWeight( const FxName& refBoneName, FxReal weight );
	/// Returns the weight of the reference bone named 'refBoneName' or 
	/// \p FxInvalidValue if the reference bone does not exist.
	/// \note For internal use only.
	FxReal GetRefBoneWeight( const FxName& refBoneName ) const;
	/// Sets the current weight of the reference bone named 'refBoneName'.
	/// \note For internal use only.
    void SetRefBoneCurrentWeight( const FxName& refBoneName, FxReal weight );
	/// Returns the current weight of the reference bone named 'refBoneName' or
	/// \p FxInvalidValue if the reference bone does not exist.
	/// \note For internal use only.  You should not need to call this as the
	/// bone's weight is returned in from GetBlendedBone().  This is here
	/// for completeness.
	FxReal GetRefBoneCurrentWeight( const FxName& refBoneName ) const;
	/// Resets all reference bone weights to their initial weight.
	/// \note For internal use only.
	void ResetRefBoneWeights( void );

	/// Pulls the bones from the bone pose nodes in \a faceGraph into the master
	/// bone list.
	void PullBones( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg );
	/// Pushes the bones from the master bone list into the bone pose nodes in 
	/// \a faceGraph.
	void PushBones( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg );

	/// Prunes unmoved bones from bonePoseNodes to save memory and 
	/// speed up bone blending.  This function is called from the tools.  Make
	/// sure that this function is only called before 
	/// \ref OC3Ent::Face::FxActor::Link() "FxActor::Link()" because it may
	/// cause a reallocation of the bone array in a bone pose node.
	/// \param faceGraph The %Face Graph containing the nodes to prune.
	void Prune( FxFaceGraph& faceGraph );

	/// Returns a blended bone for a given state of the %Face Graph.
	/// \param index The index of the reference bone.
	/// \param cg The compiled face graph.
	/// \param bonePos The bone transformation's position component.
	/// \param boneRot The bone transformation's rotation component.
	/// \param boneScale The bone transformation's scale component.
	/// \param boneWeight The current weight of the bone.
	/// \note Assumes the %Face Graph has been updated for the current frame.
	void GetBlendedBone( FxSize index, FxCompiledFaceGraph& cg, FxVec3& bonePos, 
		                 FxQuat& boneRot, FxVec3& boneScale, 
						 FxReal& boneWeight ) const;

	/// Serializes the master bone list to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxMasterBoneList& name );
	
private:
	/// A master bone list entry.
	class FxMasterBoneListEntry
	{
	public:
		/// \internal
		/// A link to a bone in a bone pose node.
		class FxBoneLink
		{
		public:
			/// Constructor.
			FxBoneLink();
			/// Constructor.
			FxBoneLink( FxSize iNodeIndex, 
				        const FxName& optimizedBoneName,
				        const FxVec3& optimizedBonePos, 
						const FxQuat& optimizedBoneRot, 
						const FxVec3& optimizedBoneScale );
			/// Destructor.
			~FxBoneLink();

			/// The index to this bone's bone pose node.
			FxSize nodeIndex;
			/// A local copy of the bone in the bone pose node in optimized form.
			/// Having this be an actual bone instead of a pointer also improves
			/// cache coherency.
			FxBone optimizedBone;
		};

		/// Constructor.
		FxMasterBoneListEntry();
		/// Constructor.
		FxMasterBoneListEntry( const FxBone& bone );
		/// Destructor.
		~FxMasterBoneListEntry();

		/// Pulls this reference bone from the bone pose nodes in \a faceGraph.
		void PullBone( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg );
		/// Pushes this reference bone into the bone pose nodes in \a faceGraph.
		void PushBone( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg );

		/// Prunes this reference bone from the bone pose nodes.
		void Prune( FxFaceGraph& faceGraph );

		/// The bone in it's reference transformation.
		FxBone refBone;
		/// The bone's inverse reference rotation quaternion.
		FxQuat refBoneInverseRot;
		/// The index of this bone in the client animation system.
		FxInt32 index;
		/// The initial reference weight of this bone.
		FxReal refWeight;
		/// The current weight of this bone from the currently playing animation.
		FxReal weight;
        /// Links to bone pose nodes this bone is in and also a pointer to
		/// the bone itself in that bone pose node.
		FxArray<FxBoneLink> links;
	};
	/// The master bone list.
	FxArray<FxMasterBoneListEntry> _refBones;

	/// Returns the master bone list entry index for the reference bone named
	/// 'refBoneName' or \p FxInvalidIndex if it is not found.
	FxSize _findMasterBoneListEntryIndex( const FxName& refBoneName ) const;

public:
	/// Serializes the bone link to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink& link );
	/// Serializes the entry to an archive.
	friend FxArchive& operator<<( FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry& entry );
};

FX_INLINE void FxMasterBoneList::SetRefBoneWeight( const FxName& refBoneName, FxReal weight )
{
	FxSize refBoneIndex = _findMasterBoneListEntryIndex(refBoneName);
	if( FxInvalidIndex != refBoneIndex )
	{
		_refBones[refBoneIndex].refWeight = weight;
	}
}

FX_INLINE FxReal FxMasterBoneList::GetRefBoneWeight( const FxName& refBoneName ) const
{
	FxSize refBoneIndex = _findMasterBoneListEntryIndex(refBoneName);
	if( FxInvalidIndex != refBoneIndex )
	{
		return _refBones[refBoneIndex].refWeight;
	}
	return FxInvalidValue;
}

FX_INLINE void FxMasterBoneList::SetRefBoneCurrentWeight( const FxName& refBoneName, FxReal weight )
{
	FxSize refBoneIndex = _findMasterBoneListEntryIndex(refBoneName);
	if( FxInvalidIndex != refBoneIndex )
	{
		_refBones[refBoneIndex].weight = weight;
	}
}

FX_INLINE FxReal FxMasterBoneList::GetRefBoneCurrentWeight( const FxName& refBoneName ) const
{
	FxSize refBoneIndex = _findMasterBoneListEntryIndex(refBoneName);
	if( FxInvalidIndex != refBoneIndex )
	{
		return _refBones[refBoneIndex].weight;
	}
	return FxInvalidValue;
}

FX_INLINE void FxMasterBoneList::ResetRefBoneWeights( void )
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].weight = _refBones[i].refWeight;
	}
}

FX_INLINE void FxMasterBoneList::GetBlendedBone( FxSize index, FxCompiledFaceGraph& cg, 
												 FxVec3& bonePos, FxQuat& boneRot, 
												 FxVec3& boneScale, FxReal& boneWeight ) const
{
	// Set up a blend operation for the specified bone.
	const FxMasterBoneListEntry& boneToBlend = _refBones[index];
	const FxBone& blendFrom = boneToBlend.refBone;
	bonePos    = blendFrom.GetPos();
	boneScale  = blendFrom.GetScale();
	boneRot    = blendFrom.GetRot();
	FxQuat blendFromRot = boneRot;
	boneWeight = boneToBlend.weight;

	// Blend in each bone pose node's contribution.
	FxSize numLinks = boneToBlend.links.Length();
	for( FxSize i = 0; i < numLinks; ++i )
	{
		const FxMasterBoneListEntry::FxBoneLink& link = boneToBlend.links[i];
		FxReal value = cg.nodes[link.nodeIndex].finalValue;
		if( !FxRealEquality(value, 0.0f) )
		{
			bonePos   += link.optimizedBone.GetPos() * value;
			boneRot   *= boneToBlend.refBoneInverseRot * blendFromRot.Slerp(link.optimizedBone.GetRot(), value);
			boneScale += link.optimizedBone.GetScale() * value;
		}
	}
	// Return the final blended bone.
	boneRot = boneRot.Normalize();
}

FX_INLINE FxSize FxMasterBoneList::_findMasterBoneListEntryIndex( const FxName& refBoneName ) const
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		if( _refBones[i].refBone.GetName() == refBoneName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

FxArchive& operator<<( FxArchive& arc, FxMasterBoneList& name );

} // namespace Face

} // namespace OC3Ent

#endif
