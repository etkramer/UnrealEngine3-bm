//------------------------------------------------------------------------------
// The interface for all FaceFx plug-ins.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxTool.h"
#include "FxSDK.h"
#include "FxArchive.h"
#include "FxArchiveStoreFile.h"
#include "FxToolLog.h"
#include "FxFastFaceGraphBaker.h"

#include <fstream>
using std::ifstream;
namespace OC3Ent
{

namespace Face
{

#define kCurrentFxToolVersion 0

// Implement the class.
FX_IMPLEMENT_CLASS(FxTool, kCurrentFxToolVersion, FxObject);

// Constructor.
FxTool::FxTool()
	: _actor(NULL)
	, _isBatchOp(FxFalse)
	, _faceGraphNodesOverwritten(FxFalse)
	, _allBonesPruned(FxFalse)
	, _doMorph(FxTrue)
	, _shouldDisplayWarningDialogs(FxTrue)
{
}

// Destructor.
FxTool::~FxTool()
{
}

//------------------------------------------------------------------------------
// Initialization and shutdown.
//------------------------------------------------------------------------------

// Starts up the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxTool::Startup( void )
{
	// Startup the FaceFX SDK with memory pooling disabled.
	FxMemoryAllocationPolicy allocationPolicy;
	allocationPolicy.bUseMemoryPools = FxFalse;
	FxSDKStartup(allocationPolicy);
	// Display FaceFX SDK version information.
	FxToolLog( "Using FaceFX SDK version %f", static_cast<FxReal>(FxSDKGetVersion()/1000.0f) );
	FxToolLog( "FaceFX SDK licensee: %s", FxSDKGetLicenseeName().GetData() );
	FxToolLog( "FaceFX SDK licensee project: %s", FxSDKGetLicenseeProjectName().GetData() );

	_actor = new FxActor();
	_actor->SetName("NewActor");

	return FxTrue;
}

// Shuts down the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxTool::Shutdown( void )
{
	if( _actor )
	{
		delete _actor;
		_actor = NULL;
	}
	// Shutdown the FaceFX SDK.
	FxSDKShutdown();
	return FxTrue;
}

//------------------------------------------------------------------------------
// Actor level operations.
//------------------------------------------------------------------------------

// Returns the name of the actor or an empty string if there is no current
// actor.
FxString FxTool::GetActorName( void )
{
	if( _actor )
	{
		return _actor->GetNameAsString();
	}
	return "";
}

// Creates a new actor with the specified name.
FxBool FxTool::CreateActor( const FxString& name )
{
	if( _actor )
	{
		delete _actor;
		_actor = NULL;
	}
	_actor = new FxActor();
	_actor->SetName(name.GetData());
	return FxTrue;
}

// Loads an actor.
FxBool FxTool::LoadActor( const FxString& filename )
{
	if( _actor )
	{
		delete _actor;
		_actor = NULL;
	}
	_actor = new FxActor();
	_actor->SetName("NewActor");
	// Note this does not use the high level actor loading function so that information 
	// can be printed to the log.
	FxArchive actorArchive(FxArchiveStoreFile::Create(filename.GetData()), FxArchive::AM_LinearLoad);
	actorArchive.Open();
	if( actorArchive.IsValid() )
	{
		if( IsNoSave() &&
		  ( FxString(actorArchive.GetLicenseeName()) != "OC3 Entertainment, Inc." ||
			FxString(actorArchive.GetLicenseeProjectName()) != "sample content evaluation" ) )
		{
			FxToolLog("Could not load Actor.  Only authorized content can be loaded by this version of the product.");
			return FxFalse;
		}
		else
		{
			actorArchive << *_actor;
			_actor->DecompileFaceGraph();
			// Display actor version information.
			FxToolLog("Actor created with FaceFX SDK version %f", static_cast<FxReal>(actorArchive.GetSDKVersion()/1000.0f));
			FxToolLog("Actor belongs to %s for %s", actorArchive.GetLicenseeName(), actorArchive.GetLicenseeProjectName());
			return FxTrue;
		}
	}
	return FxFalse;
}

// Saves an actor.
FxBool FxTool::SaveActor( const FxString& filename )
{
	if( _actor )
	{
		if(IsNoSave())
		{
			FxToolLog("Could not save FXA file.  Saving is disabled.");
		}
		if( FxSaveActorToFile(*_actor, filename.GetData()) )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

//------------------------------------------------------------------------------
// Bone level operations.
//------------------------------------------------------------------------------

// Batch imports bone poses using the specified configuration file.
FxBool FxTool::BatchImportBonePoses( const FxString& filename, 
									 FxInt32& startFrame, FxInt32& endFrame,
									 FxBool onlyFindFrameRange )
{
	FxInt32 tmpStartFrame = FX_INT32_MAX;
	FxInt32 tmpEndFrame   = FX_INT32_MIN;
	FxArray<FxImportExportPosesFileEntry> poses;
	if( _parseBatchImportExportFile(filename, poses) )
	{
		_isBatchOp = FxTrue;
		for( FxSize i = 0; i < poses.Length(); ++i )
		{
			if( poses[i].poseFrame < tmpStartFrame )
			{
				tmpStartFrame = poses[i].poseFrame;
			}
			if( poses[i].poseFrame > tmpEndFrame )
			{
				tmpEndFrame = poses[i].poseFrame;
			}
			if( !onlyFindFrameRange )
			{
				ImportBonePose(poses[i].poseName, poses[i].poseFrame);
			}
		}
		startFrame = tmpStartFrame;
		endFrame   = tmpEndFrame;
		_isBatchOp = FxFalse;
		return FxTrue;
	}
	return FxFalse;
}

// Batch exports bone poses using the specified configuration file.
FxBool FxTool::BatchExportBonePoses( const FxString& filename )
{
	FxArray<FxImportExportPosesFileEntry> poses;
	if( _parseBatchImportExportFile(filename, poses) )
	{
		_isBatchOp = FxTrue;
		for( FxSize i = 0; i < poses.Length(); ++i )
		{
			ExportBonePose(poses[i].poseFrame, poses[i].poseName);
		}
		_isBatchOp = FxFalse;
		return FxTrue;
	}
	return FxFalse;
}
//------------------------------------------------------------------------------
// Get and Set operations
//------------------------------------------------------------------------------
	
// Set the _shouldDisplayWarningDialogs variable.
void FxTool::SetShouldDisplayWarningDialogs( FxBool shouldDisplayWarningDialogs )
{
	_shouldDisplayWarningDialogs = shouldDisplayWarningDialogs;
	if( shouldDisplayWarningDialogs )
	{
		FxToolLog("Turned on dialog box warning messages.");
	}
	else
	{
		FxToolLog("Turned off dialog box warning messages.");
	}
}

// Get the _shouldDisplayWarningDialogs variable.
FxBool FxTool::GetShouldDisplayWarningDialogs( void ) const
{
	return _shouldDisplayWarningDialogs;
}

//------------------------------------------------------------------------------
// Query operations.
//------------------------------------------------------------------------------

// Returns the number of animation groups in the current actor or zero if
// there is no current actor.
FxSize FxTool::GetNumAnimGroups( void )
{
	if( _actor )
	{
		return _actor->GetNumAnimGroups();
	}
	return 0;
}

// Returns the name of the group at index or an empty string if there is
// no current actor.
FxString FxTool::GetAnimGroupName( FxSize index )
{
	if( _actor )
	{
		return _actor->GetAnimGroup(index).GetNameAsString();
	}
	return "";
}

// Returns the number of animations in the specified group or zero if there
// is no current actor or the group does not exist.
FxSize FxTool::GetNumAnims( const FxString& group )
{
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			return _actor->GetAnimGroup(groupIndex).GetNumAnims();
		}
	}
	return 0;
}

// Returns the name of the animation at index in the specified group or an
// empty string if there is no current actor or the group does not exist.
FxString FxTool::GetAnimName( const FxString& group, FxSize index )
{
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			return _actor->GetAnimGroup(groupIndex).GetAnim(index).GetNameAsString();
		}
	}
	return "";
}

// Returns the number of animation curves in the specified animation in the
// specified group or zero if there is no current actor or the group (or
// animation) does not exist.
FxSize 
FxTool::GetNumAnimCurves( const FxString& group, const FxString& anim )
{
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& theAnim = animGroup.GetAnim(animIndex);
				return theAnim.GetNumAnimCurves();
			}
		}
	}
	return 0;
}

// Returns the name of the animation curve at index in the specified 
// animation in the specified group or an empty string if there is no
// current actor or the group (or animation) does not exist.
FxString 
FxTool::GetAnimCurveName( const FxString& group, const FxString& anim, 
						  FxSize index )
{
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& theAnim = animGroup.GetAnim(animIndex);
				return theAnim.GetAnimCurve(index).GetNameAsString();
			}
		}
	}
	return "";
}

// Returns an array of strings containing the names of each face graph node
// of the specified type in the face graph or an empty array if there is no
// current actor or no nodes of the specified type exist in the face graph.
// An empty string array is also returned if type is a non-recognized FaceFx
// type.
FxArray<FxString> FxTool::GetNodesOfType( const FxString& type )
{
	if( _actor )
	{
		const FxClass* pClass = FxClass::FindClassDesc(type.GetData());
		if( pClass )
		{
			FxArray<FxString> nodeNames;
			FxFaceGraph::Iterator Iter    = _actor->GetDecompiledFaceGraph().Begin(pClass);
			FxFaceGraph::Iterator IterEnd = _actor->GetDecompiledFaceGraph().End(pClass);
			for( ; Iter != IterEnd; ++Iter )
			{
				FxFaceGraphNode* pNode = Iter.GetNode();
				if( pNode )
				{
					nodeNames.PushBack(pNode->GetNameAsString());
				}
			}
			return nodeNames;
		}
	}
	return FxArray<FxString>();
}

// Returns the number of reference bones in the current actor or zero if
// there is no current actor.
FxSize FxTool::GetNumRefBones( void )
{
	if( _actor )
	{
		return _actor->GetMasterBoneList().GetNumRefBones();
	}
	return 0;
}

// Returns the name of the reference bone at index or an empty string if 
// there is no current actor.
FxString FxTool::GetRefBoneName( FxSize index )
{
	if( _actor )
	{
		return _actor->GetMasterBoneList().GetRefBone(index).GetNameAsString();
	}
	return "";
}

// Returns the number of bones in the specified pose in the current actor
// or zero if there is no current actor or the specified pose does not
// exist.
FxSize FxTool::GetNumBones( const FxString& pose )
{
	if( _actor )
	{
		FxFaceGraphNode* pNode = _actor->GetDecompiledFaceGraph().FindNode(pose.GetData());
		if( pNode )
		{
			FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(pNode);
			if( pBonePoseNode )
			{
				return pBonePoseNode->GetNumBones();
			}
		}
	}
	return 0;
}

// Returns the name of the bone at index in the specified pose or an empty
// string if there is no current actor or the specified pose does not exist.
FxString FxTool::GetBoneName( const FxString& pose, FxSize index )
{
	if( _actor )
	{
		FxFaceGraphNode* pNode = _actor->GetDecompiledFaceGraph().FindNode(pose.GetData());
		if( pNode )
		{
			FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(pNode);
			if( pBonePoseNode )
			{
				return pBonePoseNode->GetBone(index).GetNameAsString();
			}
		}
	}
	return "";
}

// Returns an array of key values for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to construct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
FxArray<FxReal> FxTool::
GetRawCurveKeyValues( const FxString& group, const FxString& anim, const FxString& curve )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key values.  This feature is disabled.");		
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				FxSize curveIndex = anim.FindAnimCurve(curve.GetData());
				if( FxInvalidIndex != curveIndex )
				{
					FxArray<FxReal> keyInfo;
					const FxAnimCurve& animCurve = anim.GetAnimCurve(curveIndex);
					for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
					{
						keyInfo.PushBack(animCurve.GetKey(i).GetValue());
					}
					return keyInfo;
				}
			}
		}
	}
	return FxArray<FxReal>();
#endif
}

// Returns an array of key times for the specified curve in the specified
// group and animation.  Most users will want to get "baked" curves instead 
// to construct a curve that accurately reflects a node's value over time.  
// The function returns an empty array if the curve can't be found.
FxArray<FxReal> FxTool::
GetRawCurveKeyTimes( const FxString& group, const FxString& anim, 
			    const FxString& curve )
{
#ifdef MOD_DEVELOPER_VERSION
		FxToolLog("Could not return key times.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				FxSize curveIndex = anim.FindAnimCurve(curve.GetData());
				if( FxInvalidIndex != curveIndex )
				{
					FxArray<FxReal> keyInfo;
					const FxAnimCurve& animCurve = anim.GetAnimCurve(curveIndex);
					for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
					{
						keyInfo.PushBack(animCurve.GetKey(i).GetTime());
					}
					return keyInfo;
				}
			}
		}
	}
	return FxArray<FxReal>();
#endif
}

// Returns an array of key input slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to construct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
FxArray<FxReal> FxTool::
GetRawCurveKeySlopeIn( const FxString& group, const FxString& anim, 
					   const FxString& curve )
{
#ifdef MOD_DEVELOPER_VERSION
		FxToolLog("Could not return key slopes.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				FxSize curveIndex = anim.FindAnimCurve(curve.GetData());
				if( FxInvalidIndex != curveIndex )
				{
					FxArray<FxReal> keyInfo;
					const FxAnimCurve& animCurve = anim.GetAnimCurve(curveIndex);
					for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
					{
						keyInfo.PushBack(animCurve.GetKey(i).GetSlopeIn());
					}
					return keyInfo;
				}
			}
		}
	}
	return FxArray<FxReal>();
#endif
}

// Returns an array of key output slopes for the specified curve in the 
// specified group and animation.  Most users will want to get "baked" curves
// instead to construct a curve that accurately reflects a node's value over 
// time.  Baked curves always have input and output slopes of 0.  The 
// function returns an empty array if the curve can't be found.
FxArray<FxReal> FxTool::
GetRawCurveKeySlopeOut( const FxString& group, const FxString& anim, 
						const FxString& curve )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key slopes.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				FxSize curveIndex = anim.FindAnimCurve(curve.GetData());
				if( FxInvalidIndex != curveIndex )
				{
					FxArray<FxReal> keyInfo;
					const FxAnimCurve& animCurve = anim.GetAnimCurve(curveIndex);
					for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
					{
						keyInfo.PushBack(animCurve.GetKey(i).GetSlopeOut());
					}
					return keyInfo;
				}
			}
		}
	}
	return FxArray<FxReal>();
#endif
}

// Returns an array of key values for the specified node in the specified 
// group and animation.  The function evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function returns an empty 
// array if the node or animation can't be found.
FxArray<FxReal> FxTool::
GetBakedCurveKeyValues( const FxString& group, const FxString& anim, 
					    const FxString& nodeName )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key values.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxArray<FxName> nodesToBake;
		nodesToBake.PushBack(nodeName.GetData());
		FxFastFaceGraphBaker baker(_actor);
		baker.SetAnim(group.GetData(), anim.GetData());

		if( baker.Bake(nodesToBake) )
		{
			FxArray<FxReal> keyInfo;

			// We passed in one FxName to bake.  If there isn't one curve available,
			// then the FxName did not correspond to a node in the Face Graph. Had we 
			// passed in multiple FxNames, we could have used the GetNumResultAnimCurves
			// function and looped over all of the available curves.
			if(baker.GetNumResultAnimCurves() == 1)
			{
				const FxAnimCurve& animCurve = baker.GetResultAnimCurve(0);
				for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
				{
					keyInfo.PushBack(animCurve.GetKey(i).GetValue());
				}
				return keyInfo;
			}
			else
			{
				FxToolLog("The following node was not found in the Face Graph: %s", nodeName.GetData());
			}
		}
	}
	return FxArray<FxReal>();
#endif
}

// Returns an array of key times for the specified node in the specified 
// group and animation.  The function evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function returns an empty 
// array if the node or animation can't be found.
FxArray<FxReal> FxTool::
GetBakedCurveKeyTimes( const FxString& group, const FxString& anim, 
					   const FxString& nodeName )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key times.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxArray<FxName> nodesToBake;
		nodesToBake.PushBack(nodeName.GetData());
		FxFastFaceGraphBaker baker(_actor);
		baker.SetAnim(group.GetData(), anim.GetData());
		if( baker.Bake(nodesToBake) )
		{
			FxArray<FxReal> keyInfo;

			// We passed in one FxName to bake.  If there isn't one curve available,
			// then the FxName did not correspond to a node in the Face Graph. Had we 
			// passed in multiple FxNames, we could have used the GetNumResultAnimCurves
			// function and looped over all of the available curves.
			if(baker.GetNumResultAnimCurves() == 1)
			{
				const FxAnimCurve& animCurve = baker.GetResultAnimCurve(0);
				for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
				{
					keyInfo.PushBack(animCurve.GetKey(i).GetTime());
				}
				return keyInfo;
			}
			else
			{
				FxToolLog("The following node was not found in the Face Graph: %s", nodeName.GetData());
			}
		}
	}
	return FxArray<FxReal>();
#endif
}
// Returns an array of key output slopes for the specified node in the specified 
// group and animation.  The function evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function returns an empty 
// array if the node or animation can't be found.
FxArray<FxReal> FxTool::
GetBakedCurveKeySlopeOut( const FxString& group, const FxString& anim, 
					   const FxString& nodeName )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key output slopes.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxArray<FxName> nodesToBake;
		nodesToBake.PushBack(nodeName.GetData());
		FxFastFaceGraphBaker baker(_actor);
		baker.SetAnim(group.GetData(), anim.GetData());
		if( baker.Bake(nodesToBake) )
		{
			FxArray<FxReal> keyInfo;

			// We passed in one FxName to bake.  If there isn't one curve available,
			// then the FxName did not correspond to a node in the Face Graph. Had we 
			// passed in multiple FxNames, we could have used the GetNumResultAnimCurves
			// function and looped over all of the available curves.
			if(baker.GetNumResultAnimCurves() == 1)
			{
				const FxAnimCurve& animCurve = baker.GetResultAnimCurve(0);
				for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
				{
					keyInfo.PushBack(animCurve.GetKey(i).GetSlopeOut());
				}
				return keyInfo;
			}
			else
			{
				FxToolLog("The following node was not found in the Face Graph: %s", nodeName.GetData());
			}
		}
	}
	return FxArray<FxReal>();
#endif
}
// Returns an array of key input slopes for the specified node in the specified 
// group and animation.  The function evaluates the face graph at 60fps and
// returns values for a baked curve that is key reduced and accurately 
// reflects a node's value over time.  The function returns an empty 
// array if the node or animation can't be found.
FxArray<FxReal> FxTool::
GetBakedCurveKeySlopeIn( const FxString& group, const FxString& anim, 
					   const FxString& nodeName )
{
#ifdef MOD_DEVELOPER_VERSION
	FxToolLog("Could not return key input slopes.  This feature is disabled.");
	return FxArray<FxReal>();
#else
	if( _actor )
	{
		FxArray<FxName> nodesToBake;
		nodesToBake.PushBack(nodeName.GetData());
		FxFastFaceGraphBaker baker(_actor);
		baker.SetAnim(group.GetData(), anim.GetData());
		if( baker.Bake(nodesToBake) )
		{
			FxArray<FxReal> keyInfo;

			// We passed in one FxName to bake.  If there isn't one curve available,
			// then the FxName did not correspond to a node in the Face Graph. Had we 
			// passed in multiple FxNames, we could have used the GetNumResultAnimCurves
			// function and looped over all of the available curves.
			if(baker.GetNumResultAnimCurves() == 1)
			{
				const FxAnimCurve& animCurve = baker.GetResultAnimCurve(0);
				for( FxSize i = 0; i < animCurve.GetNumKeys(); ++i )
				{
					keyInfo.PushBack(animCurve.GetKey(i).GetSlopeIn());
				}
				return keyInfo;
			}
			else
			{
				FxToolLog("The following node was not found in the Face Graph: %s", nodeName.GetData());
			}
		}
	}
	return FxArray<FxReal>();
#endif
}
// Returns the specified animation length in seconds
FxReal FxTool::GetAnimDuration( const FxString& group, const FxString& anim )
{
	if( _actor )
	{
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			const FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			FxSize animIndex = animGroup.FindAnim(anim.GetData());
			if( FxInvalidIndex != animIndex )
			{
				const FxAnim& theAnim = animGroup.GetAnim(animIndex);
				return theAnim.GetDuration();
			}
		}
	} 
	return 0.0f;
}



// Inserts a key into the specified curve.  Will create the curve if it doesn't
// exist, but the group and animation must be present.  Returns index of inserted key.
FxSize FxTool::InsertKey( const FxString& group, const FxString& anim, const FxString& curve, 
	FxReal keyTime, FxReal keyValue, FxReal keySlopeIn, FxReal keySlopeOut )
{
	if( _actor )
	{
		FxAnim* animation = _actor->GetAnimPtr(group.GetData(), anim.GetData());
		if(animation)
		{
			FxSize curveIndex = animation->FindAnimCurve (curve.GetData());  
			if( FxInvalidIndex == curveIndex )
			{
				// Create the curve if it's not there.
				FxAnimCurve createdAnimCurve;
				createdAnimCurve.SetName(curve.GetData());
				animation->AddAnimCurve(createdAnimCurve);
				curveIndex = animation->FindAnimCurve (curve.GetData()); 
			}
			// Sanity check.
			if( FxInvalidIndex != curveIndex )
			{
				FxAnimCurve& animCurve = animation->GetAnimCurveM(curveIndex);
				FxAnimKey key(keyTime, keyValue, keySlopeIn, keySlopeOut);
				return animCurve.InsertKey(key);
			}
		}
	}
	return FxInvalidIndex;
}
						
// Deletes all keys in the specified curve.
FxBool FxTool::DeleteAllKeys( const FxString& group, const FxString& anim, const FxString& curve)
{
	if( _actor )
	{
		FxAnim* animation = _actor->GetAnimPtr(group.GetData(), anim.GetData());
		if(animation)
		{
			FxSize curveIndex = animation->FindAnimCurve (curve.GetData());  
			if( FxInvalidIndex != curveIndex )
			{
				FxAnimCurve& animCurve = animation->GetAnimCurveM(curveIndex);
				animCurve.RemoveAllKeys();
				return FxTrue;
			}
		}
	}
	return FxFalse;
}

// Returns if this version of the plugin is a mod developer version.
FxBool FxTool::IsModDeveloper( )
{
#ifdef MOD_DEVELOPER_VERSION
	return FxTrue;
#else
	return FxFalse;
#endif
}

// Returns if this version of the plugin is a no save version.
FxBool FxTool::IsNoSave( )
{
#ifdef NO_SAVE_VERSION
	return FxTrue;
#else
	return FxFalse;
#endif
}

// Overwrites the pCurrentNode node in the Face Graph with the 
// pNodeToAdd node.
FxBool FxTool::
_overwriteFaceGraphNode( FxFaceGraphNode* pCurrentNode, 
						 FxFaceGraphNode* pNodeToAdd )
{
	if( _actor && pCurrentNode && pNodeToAdd )
	{
		// Copy over all of the node properties and links.
		FxArray<FxFaceGraphNodeLink> nodeInputs;
		FxArray<FxFaceGraphNode*> nodeOutputs;
		FxArray<FxFaceGraphNodeLink> nodeOutputLinks;

		// The node properties can be set right away.
		pNodeToAdd->SetMin(pCurrentNode->GetMin());
		pNodeToAdd->SetMax(pCurrentNode->GetMax());
		pNodeToAdd->SetNodeOperation(pCurrentNode->GetNodeOperation());

		// The node inputs and outputs must wait to be set until after the old 
		// node is deleted.
		for( FxSize i = 0; i < pCurrentNode->GetNumInputLinks(); ++i )
		{
			nodeInputs.PushBack(pCurrentNode->GetInputLink(i));
		}
		for( FxSize i = 0; i < pCurrentNode->GetNumOutputs(); ++i )
		{
			FxFaceGraphNode* pOutputNode = pCurrentNode->GetOutput(i);
			nodeOutputs.PushBack(pOutputNode);
			if( pOutputNode )
			{
				for( FxSize j = 0; j < pOutputNode->GetNumInputLinks(); ++j )
				{
					const FxFaceGraphNodeLink& outputLink = pOutputNode->GetInputLink(j);
					if( outputLink.GetNodeName() == pCurrentNode->GetName() )
					{
						nodeOutputLinks.PushBack(outputLink);
					}
				}
			}
		}
		_actor->GetDecompiledFaceGraph().RemoveNode(pNodeToAdd->GetName());
		_actor->GetDecompiledFaceGraph().AddNode(pNodeToAdd);
		// Set the node inputs and outputs.  Since nodes can't be removed from
		// the face graph within plug-ins, it is safe to cache pointers
		// like this.
		for( FxSize i = 0; i < nodeInputs.Length(); ++i )
		{
			pNodeToAdd->AddInputLink(nodeInputs[i]);
			if( nodeInputs[i].GetNode() )
			{
				const_cast<FxFaceGraphNode*>(nodeInputs[i].GetNode())->AddOutput(pNodeToAdd);
			}
		}
		for( FxSize i = 0; i < nodeOutputs.Length(); ++i )
		{
			if( nodeOutputs[i] )
			{
				nodeOutputLinks[i].SetNode(pNodeToAdd);
				nodeOutputs[i]->AddInputLink(nodeOutputLinks[i]);
				pNodeToAdd->AddOutput(nodeOutputs[i]);
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

// Parses batch import and batch export operation files.  The results are
// stored in the poses array.
FxBool FxTool::
_parseBatchImportExportFile( const FxString& filename, 
			  			     FxArray<FxImportExportPosesFileEntry>& poses )
{
	// Load the entire file to a string.
	ifstream batchFile(filename.GetData());
	if( !batchFile )
	{
		FxString errorString = "Unable to open batch file: ";
		errorString = FxString::Concat(errorString, filename);
		FxToolLog(errorString.GetData());
		return FxFalse;
	}
	FxString batch;
	FxChar c;
	while( batchFile.get(c) )
	{
		batch = FxString::Concat(batch, c);
	}
	batchFile.close();

	// Parse out individual commands and ignore comments.
	FxArray<FxString> commands;
	FxString helperString;
	FxBool inComment = FxFalse;
	FxBool inCommand = FxFalse;
	for( FxSize i = 0; i < batch.Length(); ++i )
	{
		if( inComment )
		{
			if( batch[i] == '\n' )
			{
				inComment = FxFalse;
			}
		}
		else if( batch[i] == '/' )
		{
			if( (i+1) < batch.Length() )
			{
				if( batch[i+1] == '/' )
				{
					inComment = FxTrue;
				}
				else
				{
					helperString = FxString::Concat(helperString, batch[i]);
				}
			}
		}
		// There is a newline or semi-colon indicating the end of the command.
		else if( batch[i] == '\n' || batch[i] == ';' )
		{
			helperString = FxString::Concat(helperString, ';');
			if( helperString.Length() > 1 )
			{
				commands.PushBack(helperString);
			}
			helperString.Clear();
			inCommand = FxFalse;
		}
		// Fill in helperString.
		else if( inCommand )
		{
			helperString = FxString::Concat(helperString, batch[i]);
		}
		else if( batch[i] != ' ' && batch[i] != '\t' )
		{
			helperString = FxString::Concat(helperString, batch[i]);
			inCommand = FxTrue;
		}
	}
	// Make sure the last command is picked up.
	if( helperString.Length() > 1 && !inComment )
	{
		commands.PushBack(helperString);
	}
	helperString.Clear();

	// Parse commands into FxImportExportPosesFileEntry objects.
	for( FxSize i = 0; i < commands.Length(); ++i )
	{
		const FxString& command = commands[i];
		FxArray<FxString> splitCommand;
		FxString helperString;
		FxBool inQuote = FxFalse;
		for( FxSize i = 0; i < command.Length(); ++i )
		{
			// There is a quote, so toggle whether or not we're in a quoted argument.
			if( command[i] == '"' )
			{
				inQuote = !inQuote;
			}
			// There is a space or semi-colon indicating a new element or the end
			// of the command string.
			else if( (command[i] == ' ' || command[i] == ';') && !inQuote )
			{
				splitCommand.PushBack(helperString);
				helperString.Clear();
			}
			// Fill in the helperString.
			else
			{
				helperString = FxString::Concat(helperString, command[i]);
			}
		}
		// Make sure the last argument is picked up.
		if( helperString.Length() > 0 )
		{
			splitCommand.PushBack(helperString);
		}
		helperString.Clear();

		// Create the FxImportExportPosesFileEntry.
		if( splitCommand.Length() >= 2 )
		{
			FxImportExportPosesFileEntry entry;
			entry.poseName  = splitCommand[0];
			entry.poseFrame = FxAtoi(splitCommand[1].GetData());
			poses.PushBack(entry);
		}
	}
	commands.Clear();
	return FxTrue;
}

} // namespace Face

} // namespace OC3Ent
