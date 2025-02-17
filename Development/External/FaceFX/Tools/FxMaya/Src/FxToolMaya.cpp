//------------------------------------------------------------------------------
// The FaceFx Maya plug-in interface.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxMayaMain.h"
#include "FxActorInstance.h"
#include "FxToolMaya.h"

#include "FxFastFaceGraphBaker.h"

#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MVector.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MAnimControl.h>
#include <maya/MFnIkJoint.h>
#include <maya/MGlobal.h>

// Morph specific includes.
#include <maya/MFnAnimCurve.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MPlugArray.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MObjectArray.h>
#include <maya/MDGModifier.h>

#ifdef __UNREAL__
	#include "../../../../Src/Engine/FaceFX/UnFaceFXMorphNode.h"
	#define FX_MORPH_TARGET_NODE_CLASS_TYPE FUnrealFaceFXMorphNode
	#define FX_MORPH_TARGET_NODE_USERPROP_INDEX FX_MORPH_NODE_TARGET_NAME_INDEX
#else
	#include "FxMorphTargetNode.h"
	#define FX_MORPH_TARGET_NODE_CLASS_TYPE FxMorphTargetNode
	#define FX_MORPH_TARGET_NODE_USERPROP_INDEX FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxToolMayaVersion 0

// Implement the class.
FX_IMPLEMENT_CLASS(FxToolMaya, kCurrentFxToolMayaVersion, FxTool);

// Constructor.
FxToolMaya::FxToolMaya()
{
}

// Destructor.
FxToolMaya::~FxToolMaya()
{
}

//------------------------------------------------------------------------------
// Initialization and shutdown.
//------------------------------------------------------------------------------

// Starts up the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxToolMaya::Startup( void )
{
#ifdef __FX_USE_NATIVE_COORDS__
	FxToolLog("compiled with __FX_USE_NATIVE_COORDS__");
#endif
	return Super::Startup();
}

// Shuts down the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxToolMaya::Shutdown( void )
{
	return Super::Shutdown();
}

//------------------------------------------------------------------------------
// Actor level operations.
//------------------------------------------------------------------------------

// Creates a new actor with the specified name.
FxBool FxToolMaya::CreateActor( const FxString& name )
{
	FxBool result;
	if( 0 == name.Length() )
	{
		FxToolLog("Attempted to create new actor with the NULL name. Creating new actor called NewActor instead");
		result = Super::CreateActor("NewActor");
	}
	else
	{
		FxToolLog("attempting to create actor %s", name.GetData());
		result = Super::CreateActor(name);
	}
	if( result && _actor )
	{
		// The actor has changed, so alert any listening MEL script.
		_actorChanged();
		FxToolLog("created actor %s", _actor->GetName().GetAsCstr());
	}
	return result;
}

// Loads an actor.
FxBool FxToolMaya::LoadActor( const FxString& filename )
{
	FxToolLog("attempting to load actor file %s", filename.GetData());
	FxBool result = Super::LoadActor(filename);
	// The actor has changed, so alert any listening MEL script.
	_actorChanged();
	if( result && _actor )
	{
		FxToolLog("loaded actor named %s", _actor->GetName().GetAsCstr());
	}
	else
	{
		if( GetShouldDisplayWarningDialogs() )
		{
			if(IsNoSave())
			{
				MGlobal::executeCommand("confirmDialog -message \"Could not load file.  Only authorized content can be loaded by this version of the product.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}
			else
			{
				MGlobal::executeCommand("confirmDialog -message \"Could not load file. Make sure the most recent version of the plug-in is installed.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}
		}
		FxToolLog("failed to load actor file %s", filename.GetData());
	}
	return result; 
}

// Saves an actor.
FxBool FxToolMaya::SaveActor( const FxString& filename )
{
	FxToolLog("attempting to save actor file %s", filename.GetData());
	FxBool result = Super::SaveActor(filename);
	if( result )
	{
		FxToolLog("saved actor");
	}
	else
	{
		FxToolLog("failed to save actor file %s", filename.GetData());
	}
	return result;
}

//------------------------------------------------------------------------------
// Bone level operations.
//------------------------------------------------------------------------------

// Imports the reference pose to the specified frame.
FxBool FxToolMaya::ImportRefPose( FxInt32 frame )
{
	FxToolLog("attempting to import reference pose to frame %d", frame);
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Initialize the animation controller.
		_initializeAnimController();

		// Go to the time corresponding to the frame passed to the command.
		MTime newTime;
		newTime.setValue(frame);
		MAnimControl::setCurrentTime(newTime);
		
		// Import each bone from the reference pose.
		for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
		{
			const FxBone& refBone = _actor->GetMasterBoneList().GetRefBone(i);
			if( !_setBoneTransform(refBone) )
			{
				FxToolLog("_setBoneTransform failed for bone %s", refBone.GetNameAsCstr());
			}
		}
		// Refresh the GUI.
		MAnimControl::setCurrentTime(newTime);
		FxToolLog("imported reference pose");
		return FxTrue;
	}
	return FxFalse;
}

// Exports the reference pose from the specified frame containing the 
// specified bones.
FxBool FxToolMaya::ExportRefPose( FxInt32 frame, const FxArray<FxString>& bones )
{
	FxToolLog("attempting to export reference pose from frame %d", frame);

	// Make sure none of the Reference bones contain pipe characters, which are used by Maya
	// to specify a more complete path in the event of a name conflict.  We don't support 
	// having multiple bones with the same name in the same scene yet, so if there is a 
	// pipe character, we fail and warn the user.  Additionally, FxNames can not contain pipe 
	// characters, so when we do support multiple bones with the same name, we will need
	// to auto-convert pipe characters to some other character.
	for( FxSize i = 0; i < bones.Length(); ++i )
	{
		const char c = '|';
		if(bones[i].FindFirst(c) != FxInvalidIndex)
		{
			MGlobal::executeCommand("confirmDialog -message \"Could not export reference pose.  Bone name exists that contains a pipe character.  Make sure there are no duplicate bone names.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			return FxFalse;
		}
	}
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Clear the current reference pose.
		_actor->GetMasterBoneList().Clear();

		// Save the current time and go to the time corresponding to the frame
		// passed to the command.
		MTime oldTime;
		oldTime = MAnimControl::currentTime();
		MTime newTime;
		newTime.setValue(frame);
		MAnimControl::setCurrentTime(newTime);

		// Export the reference pose.
		for( FxSize i = 0; i < bones.Length(); ++i )
		{
			// Get the bone transform.
			FxBone refBone;
			if( _getBoneTransform(bones[i], refBone) )
			{
				// Add the reference bone to the master bone list for the
				// actor.
				_actor->GetMasterBoneList().AddRefBone(refBone);
			}
		}

		// Prune any bone pose nodes.
		_actor->GetMasterBoneList().Prune(_actor->GetDecompiledFaceGraph());
		// Compile - this also "pulls" the bones.
		_actor->CompileFaceGraph();
			
		// Go back to the previously saved time for convenience.
		MAnimControl::setCurrentTime(oldTime);

		// The reference bones have changed, so update any listening MEL script.
		_refBonesChanged();

		FxToolLog("exported reference pose containing %d bones:", bones.Length());
		for( FxSize i = 0; i < bones.Length(); ++i )
		{
			FxToolLog("    %s", bones[i].GetData());
		}
		return FxTrue;
	}
	return FxFalse;
}

// Imports specified bone pose to the specified frame.
FxBool FxToolMaya::ImportBonePose( const FxString& name, FxInt32 frame )
{
	FxToolLog("attempting to import pose %s to frame %d", name.GetData(), frame);
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Search for the bone pose node.
		FxBonePoseNode* pBonePoseNode = NULL;
		FxFaceGraph::Iterator bonePoseNodeIter    = _actor->GetDecompiledFaceGraph().Begin(FxBonePoseNode::StaticClass());
		FxFaceGraph::Iterator bonePoseNodeIterEnd = _actor->GetDecompiledFaceGraph().End(FxBonePoseNode::StaticClass());
		for( ; bonePoseNodeIter != bonePoseNodeIterEnd; ++bonePoseNodeIter )
		{
			FxFaceGraphNode* pNode = bonePoseNodeIter.GetNode();
			if( pNode )
			{
				if( pNode->GetName().GetAsString() == name )
				{
					pBonePoseNode = FxCast<FxBonePoseNode>(pNode);
					break;
				}
			}
		}
		if( pBonePoseNode )
		{
			// Initialize the animation controller.
			_initializeAnimController();

			// Go to the time corresponding to the frame passed to the command.
			MTime newTime;
			newTime.setValue(frame);
			MAnimControl::setCurrentTime(newTime);

			// Import each bone from the bone pose node.  If there is a bone in
			// the reference pose that is not in the bone pose node, use the
			// reference pose transform.  This is essential because bones are
			// pruned from the bone pose nodes.
			for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
			{
				const FxBone& refBone = _actor->GetMasterBoneList().GetRefBone(i);
				FxBone bone = refBone;
				// Search for the bone in the bone pose node.
				for( FxSize j = 0; j < pBonePoseNode->GetNumBones(); ++j )
				{
					if( pBonePoseNode->GetBone(j).GetName() == refBone.GetName() )
					{
						bone = pBonePoseNode->GetBone(j);
						break;
					}
				}
				if( !_setBoneTransform(bone) )
				{
					FxToolLog("_setBoneTransform failed for bone %s", bone.GetNameAsCstr());
				}
			}
			// Refresh the GUI.
			MAnimControl::setCurrentTime(newTime);
			FxToolLog("imported bone pose");
			return FxTrue;
		}
		else
		{
			FxToolLog("could not find bone pose");
		}
	}
	return FxFalse;
}

// Exports a bone pose from a specified frame with the specified name.
FxBool FxToolMaya::ExportBonePose( FxInt32 frame, const FxString& name )
{
	FxToolLog("attempting to export bone pose from frame %d", frame);
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
		{
			if( GetShouldDisplayWarningDialogs() )
			{
				MGlobal::executeCommand("confirmDialog -message \"Please export a reference pose first.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}
			return FxFalse;
		}

		// Save the current time and go to the time corresponding to the frame
		// passed to the command.
		MTime oldTime;
		oldTime = MAnimControl::currentTime();
		MTime newTime;
		newTime.setValue(frame);
		MAnimControl::setCurrentTime(newTime);

		// Create the bone pose node.
		FxBonePoseNode* pBonePoseNode = new FxBonePoseNode();
		pBonePoseNode->SetName(name.GetData());
		// Export each reference bone to the bone pose node.
		for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
		{
			// Get the bone transform.
			FxBone bone;
			if( _getBoneTransform(_actor->GetMasterBoneList().GetRefBone(i).GetNameAsString(), bone) )
			{
				pBonePoseNode->AddBone(bone);
			}
		}
		// Add the bone pose node to the face graph.
		if( FxFalse == _actor->GetDecompiledFaceGraph().AddNode(pBonePoseNode) )
		{
			// If the node was already in the face graph, we should
			// overwrite it but issue an indication to the user.
			FxToolLog("overwriting face graph node named %s", name.GetData());

			FxFaceGraphNode* pNodeToDelete = _actor->GetDecompiledFaceGraph().FindNode(pBonePoseNode->GetName());
			if( pNodeToDelete )
			{
				// Overwriting a non-bone pose node issues a special warning because the new bone pose
				// node can't possibly maintain all of the previous node properties.
				if( !pNodeToDelete->IsA(FxBonePoseNode::StaticClass()) )
				{
					FxToolLog("WARNING Face Graph node being overwritten is not a bone pose node");
				}

				// Overwrite the node.
				if( !_overwriteFaceGraphNode(pNodeToDelete, pBonePoseNode) )
				{
					FxToolLog("WARNING could not overwrite Face Graph node");
				}
				else
				{
					if( !_isBatchOp )
					{
						if( GetShouldDisplayWarningDialogs() )
						{
							MGlobal::executeCommand("confirmDialog -message \"Face Graph node overwritten.  Check the log file (located in the Maya user profile directory) for details.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
						}
					}
					else
					{
						_faceGraphNodesOverwritten = FxTrue;
					}
				}
			}
		}

		// Prune any bone pose nodes.
		_actor->GetMasterBoneList().Prune(_actor->GetDecompiledFaceGraph());
		FxSize numPruned = _actor->GetMasterBoneList().GetNumRefBones() - pBonePoseNode->GetNumBones();
		// Compile - this also "pulls" the bones.
		_actor->CompileFaceGraph();
		
		// Go back to the previously saved time for convenience.
		MAnimControl::setCurrentTime(oldTime);

		if( 0 == pBonePoseNode->GetNumBones() )
		{
			if( !_isBatchOp )
			{
				if( GetShouldDisplayWarningDialogs() )
				{
					MGlobal::executeCommand("confirmDialog -message \"Exported bone pose is identical to the Reference Pose and contains no bones.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
				}
			}
			else
			{
				_allBonesPruned = FxTrue;
			}
		}
		
		FxToolLog("exported bone pose named %s with %d bones removed via pruning", name.GetData(), numPruned);

		// The bone poses have changed, so we should update any listening MEL
		// script.
		_bonePosesChanged();
		return FxTrue;
	}
	return FxFalse;
}

// Batch imports bone poses using the specified configuration file.
FxBool FxToolMaya::BatchImportBonePoses( const FxString& filename, 
									     FxInt32& startFrame, FxInt32& endFrame,
										 FxBool onlyFindFrameRange )
{
	FxBool result = FxFalse;
	if( _actor )
	{
		FxToolLog("attempting to batch import bone poses from file %s", filename.GetData());
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Clear out all animation curves associated with the bones contained
		// in the actor.
		Clean();

		// Initialize the animation controller.
		_initializeAnimController();

		// Batch import the bone poses.
		startFrame = FxInvalidIndex;
		endFrame   = FxInvalidIndex;

		result = Super::BatchImportBonePoses(filename, startFrame, endFrame);
		if( result )
		{
			if( FxInvalidIndex != startFrame && FxInvalidIndex != endFrame )
			{

				MTime newTime;
				// Set the Maya animation range.
				newTime.setValue(startFrame);
				MAnimControl::setMinTime(newTime);
				MAnimControl::setAnimationStartTime(newTime);

				newTime.setValue(endFrame);
				MAnimControl::setMaxTime(newTime);
				MAnimControl::setAnimationEndTime(newTime);

				// Go back to the first frame of the animation.
				newTime.setValue(startFrame);
				MAnimControl::setCurrentTime(newTime);
			}
			FxToolLog("batch imported bone poses - range [%d, %d]", startFrame, endFrame);
		}
	}
	return result;
}

// Batch exports bone poses using the specified configuration file.
FxBool FxToolMaya::BatchExportBonePoses( const FxString& filename )
{
	FxBool result = FxFalse;
	if( _actor )
	{
		FxToolLog("attempting to batch export bone poses from file %s", filename.GetData());
		if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
		{
			if( GetShouldDisplayWarningDialogs() )
			{
				MGlobal::executeCommand("confirmDialog -message \"Please export a reference pose first.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}
			return FxFalse;
		}
		result = Super::BatchExportBonePoses(filename);
		if( result )
		{
			FxToolLog("batch exported bone poses");
		}
		if( _allBonesPruned )
		{
			if( GetShouldDisplayWarningDialogs() )
			{
				MGlobal::executeCommand("confirmDialog -message \"Some exported bone poses are identical to the Reference Pose and contain no bones.\\n  Check the log file for details.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}

			_allBonesPruned = FxFalse;
		}
		if( _faceGraphNodesOverwritten )
		{
			if( GetShouldDisplayWarningDialogs() )
			{
				MGlobal::executeCommand("confirmDialog -message \"Some Face Graph nodes were overwritten.  Check the log file (located in the Maya user profile directory) for details.\" -title \"FaceFX Warning\" -button \"OK\" -defaultButton \"OK\" -cancelButton \"OK\" -dismissString \"OK\"");
			}
			_faceGraphNodesOverwritten = FxFalse;
		}
	}
	return result;
}

//------------------------------------------------------------------------------
// Animation level operations.
//------------------------------------------------------------------------------

// Get an array of FxMayaBlendShapePlugInfo objects for morph curves that 
// match morph target names contained in morph target Face Graph nodes.
FxArray<FxToolMaya::FxMayaBlendShapePlugInfo> FxToolMaya::_getMayaBlendShapePlugInfos( void )
{
	MStatus status;
	FxArray<FxMayaBlendShapePlugInfo> returnArray;
	// Create an iterator to go through all blend shapes.
	MItDependencyNodes blendShapeIterator(MFn::kBlendShape);

	while( !blendShapeIterator.isDone() )
	{
  		// Attach the function set to the object.
		MFnBlendShapeDeformer fn(blendShapeIterator.item());
		FxToolLog("Accessing weight plug array for MFnBlendShapeDeformer: %s", fn.name().asChar());

		MIntArray indexList;
		indexList.clear();
		status =fn.weightIndexList(indexList);

		// Get the array plug for the weight attribute.  
		MObject			weightObject	= fn.attribute("weight",&status);
		MPlug			weightArrayPlug(fn.object(&status), weightObject);

		// Iterate through all the targets.
		for( OC3Ent::Face::FxSize j = 0; j < indexList.length(); ++j )
		{
			MPlug targetPlug = weightArrayPlug.elementByPhysicalIndex(indexList[j], &status);

			// Now we see if this plug corresponds to a morph node (specifically, the target name property
			// of a morph node in our Face Graph.

			// Its name will be in the form "blendShape1.Eat".
			OC3Ent::Face::FxString  mayaBlendShapeFullName	= targetPlug.name().asChar();
			OC3Ent::Face::FxString  mayaBlendShapeName = mayaBlendShapeFullName.AfterFirst('.');

			FxFaceGraph& faceGraph = _actor->GetDecompiledFaceGraph();
			FxFaceGraph::Iterator morphTargetNodeEndIter = faceGraph.End(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()); 
			FxFaceGraph::Iterator morphTargetNodeIter = faceGraph.Begin(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass());
			for( ; morphTargetNodeIter != morphTargetNodeEndIter; ++morphTargetNodeIter ) 
			{ 
				FX_MORPH_TARGET_NODE_CLASS_TYPE* pMorphTargetNode = FxCast<FX_MORPH_TARGET_NODE_CLASS_TYPE>(morphTargetNodeIter.GetNode());
				if( pMorphTargetNode )
				{
		            const FxFaceGraphNodeUserProperty& morphTargetNameUserProperty = 
						pMorphTargetNode->GetUserProperty(FX_MORPH_TARGET_NODE_USERPROP_INDEX);
					if( morphTargetNameUserProperty.GetStringProperty() == mayaBlendShapeName )
					{
						returnArray.PushBack(FxMayaBlendShapePlugInfo(pMorphTargetNode->GetName(), mayaBlendShapeName.GetData(), mayaBlendShapeFullName.GetData(), targetPlug));
					}
				}
			}
		}
		// Get next blend shapes.
		blendShapeIterator.next(); 
	}
	return returnArray;
}

void FxToolMaya::_deleteMorphCurves( void )
{
	if( _doMorph )
	{
		MStatus status;
		MDGModifier dgMod;
		FxArray<FxMayaBlendShapePlugInfo> mayaPlugInfos = _getMayaBlendShapePlugInfos();
		for( OC3Ent::Face::FxSize i = 0; i < mayaPlugInfos.Length(); ++i )
		{
			MString command = "cutKey -time \":\" -option keys -clear ";
			MString morphName(mayaPlugInfos[i].targetFullName.GetData());
			command += morphName;
			MGlobal::executeCommand(command);	
		}
	}
}

// Import the specified animation at the specified frame rate.
FxBool 
FxToolMaya::ImportAnim( const FxString& group, const FxString& name, FxReal fps )
{
#ifdef MOD_DEVELOPER_VERSION
	return FxTrue;
#else
	FxToolLog("attempting to import animation %s from group %s", name.GetData(), group.GetData());
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Get the animation group.
		FxSize groupIndex = _actor->FindAnimGroup(group.GetData());
		if( FxInvalidIndex != groupIndex )
		{
			FxAnimGroup& animGroup = _actor->GetAnimGroup(groupIndex);
			// Get the animation from the group.
			FxSize animIndex = animGroup.FindAnim(name.GetData());
			if( FxInvalidIndex != animIndex )
			{
				Clean();

				// Initialize the animation controller.
				_initializeAnimController();

				if( _doMorph )
				{
					// This factor taken from the FCollada Maya importer.
					FxReal mayaTimeFactor = 3.0f *(FxReal) MTime(1.0, MTime::kSeconds).as(MTime::uiUnit());

					MStatus status;
					OC3Ent::Face::FxArray<OC3Ent::Face::FxName> nodesToBake;
					FxArray<FxMayaBlendShapePlugInfo> mayaPlugInfos = _getMayaBlendShapePlugInfos();
					for( OC3Ent::Face::FxSize i = 0; i < mayaPlugInfos.Length(); ++i )
					{
						nodesToBake.PushBack(mayaPlugInfos[i].ownerNodeName);
					}

					OC3Ent::Face::FxFastFaceGraphBaker baker(_actor);
					baker.SetAnim(group.GetData(), name.GetData());
					if( baker.Bake(nodesToBake) )
					{
						// Make sure the baker has returned a curve for every name.  Since we know
						// that each name is a valid node in the face graph, this should always be
						// the case.
						if(baker.GetNumResultAnimCurves() == nodesToBake.Length())
						{
							for( OC3Ent::Face::FxSize i = 0; i < nodesToBake.Length(); ++i )
							{
								const FxAnimCurve& animCurve = baker.GetResultAnimCurve(i);
								MObject MyAnimCurveObject;

								// This call fails when there is no existing AnimCurve.
								MFnAnimCurve MyAnimCurve(mayaPlugInfos[i].targetPlug, &status);
								// This call fails when there is an existing anim curve, but the above line lets us add keys to the existing curve.
								MyAnimCurveObject = MyAnimCurve.create(mayaPlugInfos[i].targetPlug, MFnAnimCurve::kAnimCurveTU, NULL, &status);

								OC3Ent::Face::FxSize AnimKeyIndex;
								for( OC3Ent::Face::FxSize keyIndex = 0; keyIndex < animCurve.GetNumKeys(); ++keyIndex )
								{
									MTime keyFrame(animCurve.GetKey(keyIndex).GetTime(), MTime::kSeconds);
									OC3Ent::Face::FxReal value = animCurve.GetKey(keyIndex).GetValue();
									AnimKeyIndex = MyAnimCurve.addKey(keyFrame, value, MFnAnimCurve::kTangentClamped , MFnAnimCurve::kTangentClamped , NULL, &status);
								

									FxReal slopeIn = animCurve.GetKey(keyIndex).GetSlopeIn();
									FxReal slopeOut = animCurve.GetKey(keyIndex).GetSlopeOut();
									if( !FxRealEquality(slopeIn,0.0f) ||
										!FxRealEquality(slopeOut,0.0f) )
									{
										FxReal time = animCurve.GetKey(keyIndex).GetTime();
										MyAnimCurve.setTangentsLocked(AnimKeyIndex,false);
										if(keyIndex > 0)
										{
											FxReal timeDiff = time - animCurve.GetKey(keyIndex-1).GetTime();
											FxReal inTX = (timeDiff / 3.0f) * mayaTimeFactor;
											FxReal inTY = timeDiff * slopeIn;
											MyAnimCurve.setTangent (AnimKeyIndex, inTX, inTY, true);
										}
										if(keyIndex < animCurve.GetNumKeys() -1 )
										{
											FxReal timeDiff = animCurve.GetKey(keyIndex+1).GetTime() - time;
											FxReal outTX = (timeDiff / 3.0f) * mayaTimeFactor;
											FxReal outTY = timeDiff * slopeOut;
											MyAnimCurve.setTangent (AnimKeyIndex,  outTX, outTY, false);
										}
									}
								}
								// Make sure there is at least one key in all the curves.
								if( 0 == animCurve.GetNumKeys() )
								{
									MTime keyFrame(0.0, MTime::kSeconds);
									AnimKeyIndex = MyAnimCurve.addKey(keyFrame, 0.0, MFnAnimCurve::kTangentClamped, MFnAnimCurve::kTangentClamped, NULL, &status);
								}
								AnimKeyIndex++;
								FxToolLog("Added %d keys to %s.", AnimKeyIndex, mayaPlugInfos[i].targetName.GetAsCstr());
							}
						}
					}
				}
				const FxAnim& anim = animGroup.GetAnim(animIndex);
				// Set the Maya animation range.

				// Import the animation.
				MTime newTime;
				newTime.setUnit(MTime::kSeconds);
				// Set the Maya animation range.
				newTime.setValue(anim.GetStartTime());
				MAnimControl::setMinTime(newTime);
				MAnimControl::setAnimationStartTime(newTime);

				newTime.setValue(anim.GetEndTime());
				MAnimControl::setMaxTime(newTime);
				MAnimControl::setAnimationEndTime(newTime);

				FxActorInstance actorInstance;
				actorInstance.SetActor(_actor);
				actorInstance.PlayAnim(name.GetData(), group.GetData());
				FxDReal animStep = 1.0 / static_cast<FxDReal>(fps);
				FxDReal startTime = static_cast<FxDReal>(actorInstance.GetCurrentAnim()->GetStartTime());
				FxDReal endTime = static_cast<FxDReal>(actorInstance.GetCurrentAnim()->GetEndTime());
				for( FxDReal i = startTime; i < endTime; i += animStep )
				{
					// Tick the animation.
					actorInstance.Tick(i, -1.0f);
					// Go to the frame corresponding to the current animation time.
					newTime.setValue(i);
					MAnimControl::setCurrentTime(newTime);
					// Import each bone.
					for( FxSize j = 0; j < _actor->GetMasterBoneList().GetNumRefBones(); ++j )
					{
						FxVec3 pos, scale;
						FxQuat rot;
						FxReal weight;
                        actorInstance.GetBone(j, pos, rot, scale, weight);
						FxBone bone = _actor->GetMasterBoneList().GetRefBone(j);
						bone.SetPos(pos);
						bone.SetRot(rot);
						bone.SetScale(scale);
						if( !_setBoneTransform(bone) )
						{
							FxToolLog("_setBoneTransform failed for bone %s", bone.GetNameAsCstr());
						}
					}
				}
				// Go back to the first frame of the animation.
				newTime.setValue(anim.GetStartTime());
				MAnimControl::setCurrentTime(newTime);
				FxToolLog("imported animation");
				return FxTrue;
			}
			else
			{
				FxToolLog("could not find animation");
			}
		}
		else
		{
			FxToolLog("could not find group");
		}
	}
	return FxFalse;
#endif
}

//------------------------------------------------------------------------------
// Maya level operations.
//------------------------------------------------------------------------------

// Cleans up all animation curves in Maya associated with the current
// actor.  After calling Clean, you need to call FxMayaAnimController::Initialize
// before setting keys again.
void FxToolMaya::Clean( void )
{
	if( _actor )
	{
		// We issue cutKey command as opposed to using the FxMayaAnimController's
		// Clean() function because we don't want the face to implode from having
		// curves with no keys in them.
		for(FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i)
		{
			MString command = "cutKey -time \":\" -option keys -clear ";
			MString boneName(_actor->GetMasterBoneList().GetRefBone(i).GetNameAsString().GetData());
			command += boneName;
			MGlobal::executeCommand(command);	
		}

		// Delete animation curves for blendshapes that match nodes in the face graph.
		_deleteMorphCurves();
	}
}

// Finds the specified bone in Maya's DAG and returns it's transformation.
// This only returns the bone's transformation in the current frame.  
// Returns FxTrue if the bone was found and the transform is valid or
// FxFalse otherwise.
FxBool FxToolMaya::_getBoneTransform( const FxString& boneName, FxBone& bone )
{
	// Iterate through Maya's DAG looking for the bone and it's transform.
	MStatus status;
	MItDag dagIterator(MItDag::kDepthFirst, MFn::kInvalid, &status);
	if( !status ) 
	{
		status.perror("MItDag constructor");
		return FxFalse;
	}

	for( ; !dagIterator.isDone(); dagIterator.next() ) 
	{
		MDagPath dagPath;
		status = dagIterator.getPath(dagPath);
		if( !status )
		{
			status.perror("MItDag::getPath");
			continue;
		}

		MFnDagNode dagNode(dagPath, &status);
		if( !status )
		{
			status.perror("MFnDagNode constructor");
			continue;
		}

		if( FxString(dagNode.name().asChar()) ==  boneName )
		{
			// Set the bone name.
			bone.SetName(boneName.GetData());

			// Get the transform node.
			MObject transformNode = dagPath.transform(&status);
			MFnDagNode transform(transformNode, &status);

			// Extract the transformation matrix.
			MTransformationMatrix matrix(transform.transformationMatrix());

#ifdef __FX_USE_NATIVE_COORDS__
			// Set the translation.
			MVector translation = matrix.translation(MSpace::kWorld);
			bone.SetPos(FxVec3(static_cast<FxReal>(translation.x), 
				               static_cast<FxReal>(translation.y), 
				               static_cast<FxReal>(translation.z)));

			// Set the scale.
			FxDReal scale[3];
			matrix.getScale(scale, MSpace::kWorld);
			bone.SetScale(FxVec3(static_cast<FxReal>(scale[0]), 
				                 static_cast<FxReal>(scale[1]), 
				                 static_cast<FxReal>(scale[2])));

			// Set the rotation.
			MQuaternion rotation = matrix.rotation();
			bone.SetRot(FxQuat(static_cast<FxReal>(-rotation.w),
				               static_cast<FxReal>(rotation.x),
				               static_cast<FxReal>(rotation.y),
				               static_cast<FxReal>(rotation.z)));
#else
			// Set the translation.
			MVector translation = matrix.translation(MSpace::kWorld);
			bone.SetPos(FxVec3(static_cast<FxReal>(-translation.x), 
							   static_cast<FxReal>(translation.y), 
							   static_cast<FxReal>(translation.z)));

			// Set the scale.
			FxDReal scale[3];
			matrix.getScale(scale, MSpace::kWorld);
			bone.SetScale(FxVec3(static_cast<FxReal>(scale[0]), 
								 static_cast<FxReal>(scale[1]), 
								 static_cast<FxReal>(scale[2])));

			// Set the rotation.
			MQuaternion rotation = matrix.rotation();
			bone.SetRot(FxQuat(static_cast<FxReal>(-rotation.w),
							   static_cast<FxReal>(-rotation.x),
							   static_cast<FxReal>(rotation.y),
							   static_cast<FxReal>(rotation.z)));
#endif

			// Found the bone.
			return FxTrue;
		}
	}
	// The bone was not found.
	return FxFalse;
}

// Sets the specified bone's transform for the current frame.
// Returns FxTrue if the transform was set or FxFalse otherwise.
FxBool FxToolMaya::_setBoneTransform( const FxBone& bone )
{
	// Find the bone animation curve associated with the passed in bone.
	FxMayaAnimCurve* pBoneAnimCurve = _animController.FindAnimCurve(bone.GetNameAsString());
	if( pBoneAnimCurve )
	{
		// Hook to the bone.
		MStatus	status;
		MFnIkJoint ikJoint(pBoneAnimCurve->boneObject, &status);
		if( !status )
		{	
			// If the bone could not be hooked, fail.
			return FxFalse;
		}

		// Go to the current frame.
		MTime currentTime;
		currentTime = MAnimControl::currentTime();

		// Set keys for each translation attribute.
		FxVec3 boneTrans = bone.GetPos();
		MVector mayaTrans;
#ifdef __FX_USE_NATIVE_COORDS__
		mayaTrans.x = boneTrans.x;
		mayaTrans.y = boneTrans.y;
		mayaTrans.z = boneTrans.z;
#else
		mayaTrans.x = -boneTrans.x;
		mayaTrans.y = boneTrans.y;
		mayaTrans.z = boneTrans.z;
#endif
		if( pBoneAnimCurve->isTranslateXValid )	pBoneAnimCurve->translateX.addKeyframe(currentTime, mayaTrans.x);
		if( pBoneAnimCurve->isTranslateYValid ) pBoneAnimCurve->translateY.addKeyframe(currentTime, mayaTrans.y);
		if( pBoneAnimCurve->isTranslateZValid ) pBoneAnimCurve->translateZ.addKeyframe(currentTime, mayaTrans.z);

		
		FxQuat boneQuat = bone.GetRot();
#ifndef __FX_USE_NATIVE_COORDS__
		boneQuat.x = -boneQuat.x;
#endif

		// We need to remove the Rotate axis's contribution to the boneQuat
		MQuaternion mayaRotOrient = ikJoint.rotateOrientation(MSpace::kTransform);
		FxQuat rotationOrientation;
		rotationOrientation.w = static_cast<FxReal>(mayaRotOrient.w);
		rotationOrientation.x = static_cast<FxReal>(mayaRotOrient.x);
		rotationOrientation.y = static_cast<FxReal>(mayaRotOrient.y);
		rotationOrientation.z = static_cast<FxReal>(mayaRotOrient.z);				
		FxQuat tmp = rotationOrientation * boneQuat ;

		// Multiply the rotation quaternion by the joint orientation.
		MQuaternion mayaJointOrient;
		ikJoint.getOrientation(mayaJointOrient);
		FxQuat jointOrient;
		jointOrient.w = static_cast<FxReal>(mayaJointOrient.w);
		jointOrient.x = static_cast<FxReal>(mayaJointOrient.x);
		jointOrient.y = static_cast<FxReal>(mayaJointOrient.y);
		jointOrient.z = static_cast<FxReal>(mayaJointOrient.z);
		tmp *= jointOrient;

		// Create a rotation matrix.
		MTransformationMatrix tranformationMatrix;
		tranformationMatrix.setRotationQuaternion(static_cast<FxDReal>(tmp.x),
												  static_cast<FxDReal>(tmp.y),
												  static_cast<FxDReal>(tmp.z),
												  static_cast<FxDReal>(-tmp.w));

		// Extract the rotation from the matrix.
		FxDReal rot[3];
		MTransformationMatrix::RotationOrder order;
		tranformationMatrix.getRotation(rot, order);

		// Set keys for each rotation attribute.
		if( pBoneAnimCurve->isRotateXValid ) pBoneAnimCurve->rotateX.addKeyframe(currentTime, rot[0]);
		if( pBoneAnimCurve->isRotateYValid ) pBoneAnimCurve->rotateY.addKeyframe(currentTime, rot[1]);
		if( pBoneAnimCurve->isRotateZValid ) pBoneAnimCurve->rotateZ.addKeyframe(currentTime, rot[2]);

		// Set keys for each scale attribute.
		FxVec3 boneScale = bone.GetScale();
		MVector mayaScale(static_cast<FxDReal>(boneScale.x), static_cast<FxDReal>(boneScale.y), static_cast<FxDReal>(boneScale.z));
		if( pBoneAnimCurve->isScaleXValid ) pBoneAnimCurve->scaleX.addKeyframe(currentTime, mayaScale.x);
		if( pBoneAnimCurve->isScaleYValid ) pBoneAnimCurve->scaleY.addKeyframe(currentTime, mayaScale.y);
		if( pBoneAnimCurve->isScaleZValid ) pBoneAnimCurve->scaleZ.addKeyframe(currentTime, mayaScale.z);

		return FxTrue;
	}
	return FxFalse;
}

// Initializes the animation controller.
FxBool FxToolMaya::_initializeAnimController( void )
{
	if( _actor )
	{
		// Initialize the animation controller.
		FxArray<FxString> bones;
		for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
		{
			bones.PushBack(_actor->GetMasterBoneList().GetRefBone(i).GetNameAsString());
		}

		if( !_animController.Initialize(bones) )
		{
			FxToolLog("some bones could not be initialized in _initializeAnimController()");
		}
		return FxTrue;
	}
	return FxFalse;
}

// Stops any currently playing animation in Maya.
void FxToolMaya::_stopAnimationPlayback( void )
{
	if( MAnimControl::isPlaying() )
	{
		MAnimControl::stop();
	}
}

// Called when the actor has changed so that a MEL script function can be
// called to alert any MEL scripts that define the "callback" function
// __FaceFxOnActorChanged().
void FxToolMaya::_actorChanged( void )
{
	FxInt32 procExists = 0;
	MGlobal::executeCommand("exists __FaceFxOnActorChanged", procExists);
	if( 1 == procExists )
	{
		MGlobal::executeCommand("__FaceFxOnActorChanged();");
	}
	else
	{
		FxToolLog("could not find MEL proc __FaceFxOnActorChanged() - make sure the MEL script is loaded");
	}
}

// Called when the list of reference bones has changed so that a MEL script
// function can be called to alert any MEL scripts that define the 
// "callback" function __FaceFxOnRefBonesChanged().
void FxToolMaya::_refBonesChanged( void )
{
	FxInt32 procExists = 0;
	MGlobal::executeCommand("exists __FaceFxOnRefBonesChanged", procExists);
	if( 1 == procExists )
	{
		MGlobal::executeCommand("__FaceFxOnRefBonesChanged();");
	}
	else
	{
		FxToolLog("could not find MEL proc __FaceFxOnRefBonesChanged() - make sure the MEL script is loaded");
	}
}

// Called when the list of bone poses has changed so that a MEL script
// function can be called to alert any MEL scripts that define the
// "callback" function __FaceFxOnBonePosesChanged().
void FxToolMaya::_bonePosesChanged( void )
{
	FxInt32 procExists = 0;
	MGlobal::executeCommand("exists __FaceFxOnBonePosesChanged", procExists);
	if( 1 == procExists )
	{
		MGlobal::executeCommand("__FaceFxOnBonePosesChanged();");
	}
	else
	{
		FxToolLog("could not find MEL proc __FaceFxOnBonePosesChanged() - make sure the MEL script is loaded");
	}
}

} // namespace Face

} // namespace OC3Ent
