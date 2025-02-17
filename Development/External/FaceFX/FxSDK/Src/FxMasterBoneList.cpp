//------------------------------------------------------------------------------
// This class defines the master bone list for an FxActor.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMasterBoneList.h"
#include "FxUtil.h"

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxMasterBoneListVersion 2

FxMasterBoneList::FxMasterBoneListEntry::
FxBoneLink::FxBoneLink()
	: nodeIndex(FxInvalidIndex)
{
}

FxMasterBoneList::FxMasterBoneListEntry::
FxBoneLink::FxBoneLink( FxSize iNodeIndex, 
					    const FxName& optimizedBoneName,
					    const FxVec3& optimizedBonePos, 
					    const FxQuat& optimizedBoneRot, 
					    const FxVec3& optimizedBoneScale )
	: nodeIndex(iNodeIndex)
	, optimizedBone(optimizedBoneName, optimizedBonePos, optimizedBoneRot, optimizedBoneScale)
{
}

FxMasterBoneList::FxMasterBoneListEntry::
FxBoneLink::~FxBoneLink()
{
}

FxArchive& operator<<( FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink& link )
{
	return arc << link.nodeIndex << link.optimizedBone;
}

FxMasterBoneList::
FxMasterBoneListEntry::FxMasterBoneListEntry()
	: index(FX_INT32_MAX)
	, refWeight(1.0f)
	, weight(1.0f)
{
}

FxMasterBoneList::
FxMasterBoneListEntry::FxMasterBoneListEntry( const FxBone& bone )
	: refBone(bone)
	, refBoneInverseRot(bone.GetRot().GetInverse())
	, index(FX_INT32_MAX)
	, refWeight(1.0f)
	, weight(1.0f)
{
}

FxMasterBoneList::
FxMasterBoneListEntry::~FxMasterBoneListEntry()
{
}

void FxMasterBoneList::
FxMasterBoneListEntry::PullBone( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg )
{
	// Clear out any previous links.
	links.Clear();
	// Find the reference bone in any of the bone pose nodes and create the 
	// links.  This is two passes to prevent memory fragmentation.
	// First pass: count the number of bone pose nodes that contain the 
	// reference bone.
	FxSize numLinks = 0;
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator bonePoseNodeEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(bonePoseNodeIter.GetNode());
		FxAssert(pBonePoseNode);
		FxSize numBones = pBonePoseNode->GetNumBones();
		for( FxSize i = 0; i < numBones; ++i )
		{
			const FxBone& poseBone = pBonePoseNode->GetBone(i);
			if( poseBone.GetName() == refBone.GetName() )
			{
				numLinks++;
			}
		}
	}
	// Second pass: reserve the exact amount of space and create the links.
	links.Reserve(numLinks);
	bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(bonePoseNodeIter.GetNode());
		FxAssert(pBonePoseNode);
		FxSize numBones = pBonePoseNode->GetNumBones();
		for( FxSize i = 0; i < numBones; ++i )
		{
			const FxBone& poseBone = pBonePoseNode->GetBone(i);
			if( poseBone.GetName() == refBone.GetName() )
			{
				links.PushBack(FxBoneLink(cg.FindNodeIndex(pBonePoseNode->GetName()), 
					                      poseBone.GetName(), 
									      (poseBone.GetPos() - refBone.GetPos()), 
										  poseBone.GetRot(), 
										  (poseBone.GetScale() - refBone.GetScale())));
			}
		}
	}
}

void FxMasterBoneList::
FxMasterBoneListEntry::PushBone( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg )
{
	FxSize numLinks = links.Length();
	for( FxSize i = 0; i < numLinks; ++i )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(faceGraph.FindNode(cg.nodes[links[i].nodeIndex].name));
		FxAssert(pBonePoseNode);
		FxBone poseBone = links[i].optimizedBone;
		poseBone.SetPos(refBone.GetPos() + poseBone.GetPos());
		poseBone.SetScale(refBone.GetScale() + poseBone.GetScale());
		pBonePoseNode->AddBone(poseBone);
	}
}

FX_INLINE void FxMasterBoneList::
FxMasterBoneListEntry::Prune( FxFaceGraph& faceGraph )
{
	// Find the reference bone in any of the bone pose nodes and prune if required.
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator faceGraphEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != faceGraphEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(bonePoseNodeIter.GetNode());
		if( pBonePoseNode )
		{
			for( FxSize i = 0; i < pBonePoseNode->GetNumBones(); ++i )
			{
				const FxBone& poseBone = pBonePoseNode->GetBone(i);
				if( poseBone.GetName() == refBone.GetName() )
				{
					if( poseBone == refBone )
					{
						pBonePoseNode->RemoveBone(i);
						i--;
					}
				}
			}
		}
	}
}

FxArchive& operator<<( FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry& entry )
{
	if( arc.GetSDKVersion() < 1700 && arc.IsLoading() )
	{
		// Since this is the same version as the master bone list itself, as of
		// version 1.7 these are no longer have individual versions.
		FxUInt16 version = kCurrentFxMasterBoneListVersion;
		arc << version;
	}

	return arc << entry.refBone << entry.refBoneInverseRot << entry.index 
		       << entry.refWeight << entry.links;
}

FxMasterBoneList::FxMasterBoneList()
{
}

FxMasterBoneList::~FxMasterBoneList()
{
}

void FxMasterBoneList::Clear( void )
{
	_refBones.Clear();
}

void FxMasterBoneList::AddRefBone( const FxBone& bone )
{
	// If the bone is already in the list, simply update it.
	FxBone temp(bone);
	FxQuat rot(bone.GetRot());
	rot.Normalize();
	temp.SetRot(rot);
	FxBool alreadyInList = FxFalse;
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		if( _refBones[i].refBone.GetName() == temp.GetName() )
		{
			_refBones[i].refBone = temp;
			alreadyInList = FxTrue;
		}
	}
	// Otherwise add it to the list.
	if( !alreadyInList )
	{
		FxMasterBoneListEntry newEntry(temp);
		_refBones.PushBack(newEntry);
	}
}

void FxMasterBoneList::
SetRefBoneClientIndex( FxSize refBoneIndex, FxInt32 clientIndex )
{
	_refBones[refBoneIndex].index = clientIndex;
}

void FxMasterBoneList::PullBones( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg )
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].PullBone(faceGraph, cg);
	}
}

void FxMasterBoneList::PushBones( FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg )
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].PushBone(faceGraph, cg);
	}
}

void FxMasterBoneList::Prune( FxFaceGraph& faceGraph )
{
	// Clean out any bones from all bone pose nodes that are not in the master
	// bone list.
	FxSize numRefBones = _refBones.Length();
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator faceGraphEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != faceGraphEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(bonePoseNodeIter.GetNode());
		if( pBonePoseNode )
		{
			for( FxSize i = 0; i < pBonePoseNode->GetNumBones(); ++i )
			{
				const FxBone& poseBone = pBonePoseNode->GetBone(i);
				FxBool foundBone = FxFalse;
				for( FxSize j = 0; j < numRefBones; ++j )
				{
					if( poseBone.GetName() == _refBones[j].refBone.GetName() )
					{
						foundBone = FxTrue;
					}
				}
				if( !foundBone )
				{
					pBonePoseNode->RemoveBone(i);
					i--;
				}
			}
		}
	}
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].Prune(faceGraph);
	}
}

FxArchive& operator<<( FxArchive& arc, FxMasterBoneList& mbl )
{
	FxUInt16 version = arc.SerializeClassVersion("FxMasterBoneList", FxTrue, kCurrentFxMasterBoneListVersion);

	if( version < 2 )
	{
		if( arc.IsLoading() )
		{
			mbl._refBones.Clear();
			FxSize length = 0;
			arc << length;
			mbl._refBones.Reserve(length);
			for( FxSize i = 0; i < length; ++i )
			{
				FxBone bone;
				arc << bone;
				FxMasterBoneList::FxMasterBoneListEntry newEntry(bone);
				FxInt32 index;
				arc << index;
				newEntry.index = index;
				if( version >= 1 )
				{
					FxReal refWeight;
					arc << refWeight;
					newEntry.refWeight = refWeight;
				}
				mbl._refBones.PushBack(newEntry);
			}
		}
	}
	else
	{
		arc << mbl._refBones;
	}

	return arc;
}

} // namespace Face

} // namespace OC3Ent
