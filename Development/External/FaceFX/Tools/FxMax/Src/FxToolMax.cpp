//------------------------------------------------------------------------------
// The FaceFx Max plug-in interface.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxActorInstance.h"
#include "FxToolMax.h"
#include "FxMaxMain.h"
#include "FxFastFaceGraphBaker.h"
#include "FxToolLog.h"

#ifdef __UNREAL__
	#include "../../../../Src/Engine/FaceFX/UnFaceFXMorphNode.h"
	#define FX_MORPH_TARGET_NODE_CLASS_TYPE FUnrealFaceFXMorphNode
	#define FX_MORPH_TARGET_NODE_USERPROP_INDEX FX_MORPH_NODE_TARGET_NAME_INDEX
#else
	#include "FxMorphTargetNode.h"
	#define FX_MORPH_TARGET_NODE_CLASS_TYPE FxMorphTargetNode
	#define FX_MORPH_TARGET_NODE_USERPROP_INDEX FX_MORPH_TARGET_NODE_USERPROP_TARGET_NAME_INDEX
#endif // __UNREAL__

// Removes scaling from originalMatrix.
inline Matrix3 RemoveNonUniformScaling( const Matrix3& originalMatrix )
{          
	AffineParts matrixParts;
	decomp_affine(originalMatrix, &matrixParts);
	Matrix3 nodeMatrixNoScale;
	matrixParts.q.MakeMatrix(nodeMatrixNoScale);
	nodeMatrixNoScale.SetTrans(matrixParts.t);
	return nodeMatrixNoScale;
}

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxToolMaxVersion 0

// Implement the class.
FX_IMPLEMENT_CLASS(FxToolMax, kCurrentFxToolMaxVersion, FxTool);

// Constructor.
FxToolMax::FxToolMax()
	: _pMaxInterface(NULL)
	, _normalizeScale(FxTrue)
{
}

// Destructor.
FxToolMax::~FxToolMax()
{
}

//------------------------------------------------------------------------------
// Initialization and shutdown.
//------------------------------------------------------------------------------

// Starts up the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxToolMax::Startup( void )
{
#ifdef __FX_USE_NATIVE_COORDS__
	FxToolLog("compiled with __FX_USE_NATIVE_COORDS__");
#endif
	return Super::Startup();
}

// Shuts down the tool.  If this is overloaded, make sure to call the
// base class version first.
FxBool FxToolMax::Shutdown( void )
{
	return Super::Shutdown();
}

//------------------------------------------------------------------------------
// Actor level operations.
//------------------------------------------------------------------------------

// Creates a new actor with the specified name.
FxBool FxToolMax::CreateActor( const FxString& name )
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
		FxToolLog("created actor %s", _actor->GetName().GetAsCstr());
	}
	return result;
}

// Loads an actor.
FxBool FxToolMax::LoadActor( const FxString& filename )
{
	FxToolLog("attempting to load actor file %s", filename.GetData());
	FxBool result = Super::LoadActor(filename);
	if( result && _actor )
	{
		FxToolLog("loaded actor named %s", _actor->GetName().GetAsCstr());
	}
	else
	{
		if( _shouldDisplayWarningDialogs )
		{
			if(IsNoSave())
			{
				::MessageBox(NULL, _T("Could not load file.  Only authorized content can be loaded by this version of the product."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
			else
			{
				::MessageBox(NULL, _T("Could not load file.  Make sure the most recent version of the plug-in is installed."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
		}
		FxToolLog("failed to load actor file %s", filename.GetData());
	}
	return result;
}

// Saves an actor.
FxBool FxToolMax::SaveActor( const FxString& filename )
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
FxBool FxToolMax::ImportRefPose( FxInt32 frame )
{
	FxToolLog("attempting to import reference pose to frame %d", frame);
	if( _actor ) 
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Initialize the animation controller.
		_initializeAnimController();

		// Go to the time corresponding to the frame passed to the command.
		GetCOREInterface()->SetTime(frame * GetTicksPerFrame());

		// Import each bone from the reference pose.
		for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
		{
			const FxBone& refBone = _actor->GetMasterBoneList().GetRefBone(i);
			if( !_setBoneTransform(refBone) )
			{
				FxToolLog("_setBoneTransform failed for bone %s", refBone.GetNameAsCstr());
			}
		}
		// Refresh the Max rendering views.
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		FxToolLog("imported reference pose");
		return FxTrue;
	}
	return FxFalse;
}

// Exports the reference pose from the specified frame containing the 
// specified bones.
FxBool FxToolMax::ExportRefPose( FxInt32 frame, const FxArray<FxString>& bones )
{
	FxToolLog("attempting to export reference pose from frame %d", frame);
	if( _actor )
	{
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Clear the current reference pose.
		_actor->GetMasterBoneList().Clear();

		// Save the current time and go to the time corresponding to the frame
		// passed to the command.  
		TimeValue oldTime = GetCOREInterface()->GetTime();
		GetCOREInterface()->SetTime(frame * GetTicksPerFrame());

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
		GetCOREInterface()->SetTime(oldTime);

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
FxBool FxToolMax::ImportBonePose( const FxString& name, FxInt32 frame )
{
	FxToolLog("attempting to import bone pose %s to frame %d", name.GetData(), frame);
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
			if( !_isBatchOp )
			{
				// Initialize the animation controller.
				_initializeAnimController();
				// Max specific optimization.
				_animController.PrepareRotationControllers();
			}

			// Go to the time corresponding to the frame passed to the command.
			GetCOREInterface()->SetTime(frame * GetTicksPerFrame());

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

			if( !_isBatchOp )
			{
				// Max specific optimization.
				_animController.RestoreRotationControllers();
				// Refresh the Max rendering views.
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
			}

			FxToolLog("imported bone pose");
			return FxTrue;
		}
		else
		{
			FxToolLog("could not find bone pose!");
		}
	}
	return FxFalse;
}

// Exports a bone pose from a specified frame with the specified name.
FxBool FxToolMax::ExportBonePose( FxInt32 frame, const FxString& name )
{
	FxToolLog("attempting to export bone pose from frame %d", frame);
	if( _actor )
	{
        // Stop any currently playing animation.
		_stopAnimationPlayback();

		if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
		{
			if( _shouldDisplayWarningDialogs )
			{
				::MessageBox(NULL, _T("Please export a reference pose first."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
			return FxFalse;
		}

		// Save the current time and go to the time corresponding to the frame
		// passed to the command.
		TimeValue oldTime = GetCOREInterface()->GetTime();
		GetCOREInterface()->SetTime(frame * GetTicksPerFrame());

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
					FxToolLog("WARNING face graph node being overwritten is not a bone pose node");
				}

				// Overwrite the node.
				if( !_overwriteFaceGraphNode(pNodeToDelete, pBonePoseNode) )
				{
					FxToolLog("WARNING could not overwrite face graph node");
				}
				else
				{
					if( !_isBatchOp )
					{
						if( _shouldDisplayWarningDialogs )
						{
							::MessageBox(NULL, _T("Face Graph node overwritten.  Check the log file for details."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
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
		GetCOREInterface()->SetTime(oldTime);

		if( 0 == pBonePoseNode->GetNumBones() )
		{
			if( !_isBatchOp )
			{
				if( _shouldDisplayWarningDialogs )
				{
					::MessageBox(NULL, _T("Exported bone pose is identical to the Reference Pose and contains no bones.\n"), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
				}
			}
			else
			{
				_allBonesPruned = FxTrue;
			}
		}

		FxToolLog("exported bone pose named %s with %d bones removed via pruning", name.GetData(), numPruned);

		return FxTrue;
	}
	return FxFalse;
}

// Batch imports bone poses using the specified configuration file.
FxBool FxToolMax::BatchImportBonePoses( const FxString& filename, 
									    FxInt32& startFrame, FxInt32& endFrame,
										FxBool onlyFindFrameRange )
{
	FxBool result = FxFalse;
	if( _actor )
	{
		FxToolLog("attempting to batch import bone poses from file %s", filename.GetData());
		// Stop any currently playing animation.
		_stopAnimationPlayback();

		// Disable viewport redrawing to speed up the import process.
		GetCOREInterface()->DisableSceneRedraw();

		// Initialize the animation controller.
		_initializeAnimController();

		// Clear out all animation curves associated with the bones contained
		// in the actor.
		_animController.Clean();

		// Max specific optimization.
		_animController.PrepareRotationControllers();
		
		// Batch import the bone poses.
		startFrame = FxInvalidIndex;
		endFrame   = FxInvalidIndex;
		// Only find the frame range (this is because 3dsmax is really stupid and
		// doesn't let you set keys for times that aren't currently visible
		// on the timeline).
		result = Super::BatchImportBonePoses(filename, startFrame, endFrame, FxTrue);
		if( FxInvalidIndex != startFrame && FxInvalidIndex != endFrame )
		{
			// Set the Max animation range.
			TimeValue animBeginTime = startFrame * GetTicksPerFrame();
			TimeValue animEndTime   = endFrame * GetTicksPerFrame();
			Interval animInterval(animBeginTime, animEndTime);
			GetCOREInterface()->SetAnimRange(animInterval);
		}
		// Now actually import the poses.
		result = Super::BatchImportBonePoses(filename, startFrame, endFrame);
		if( FxInvalidIndex != startFrame && FxInvalidIndex != endFrame )
		{
			// Set the Max animation range.
			TimeValue animBeginTime = startFrame * GetTicksPerFrame();
			// Go back to the first frame of the animation.
			GetCOREInterface()->SetTime(animBeginTime);
		}
		
		// Max specific optimization.
		_animController.RestoreRotationControllers();

		if( result )
		{
			FxToolLog("batch imported bone poses - range [%d, %d]", startFrame, endFrame);
		}
		// Enable viewport redrawing.
		GetCOREInterface()->EnableSceneRedraw();
		// Refresh the Max rendering views.
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	return result;
}

// Batch exports bone poses using the specified configuration file.
FxBool FxToolMax::BatchExportBonePoses( const FxString& filename )
{
	FxBool result = FxFalse;
	if( _actor )
	{
		FxToolLog("attempting to batch export bone poses from file %s", filename.GetData());
		if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
		{
			if( _shouldDisplayWarningDialogs )
			{
				::MessageBox(NULL, _T("Please export a reference pose first."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
			return FxFalse;
		}
		// Disable viewport redrawing to speed up the import process.
		GetCOREInterface()->DisableSceneRedraw();
		result = Super::BatchExportBonePoses(filename);
		if( result )
		{
			FxToolLog("batch exported bone poses");
		}
		if( _allBonesPruned )
		{
			if( _shouldDisplayWarningDialogs )
			{
				::MessageBox(NULL, _T("Some exported bone poses are identical to the Reference Pose and contain no bones.\n  Check the log file for details."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
			_allBonesPruned = FxFalse;
		}
		if( _faceGraphNodesOverwritten )
		{
			if( _shouldDisplayWarningDialogs )
			{
				::MessageBox(NULL, _T("Some Face Graph nodes were overwritten.  Check the log file for details."), _T("FaceFX Warning"), MB_OK | MB_ICONWARNING);
			}
			_faceGraphNodesOverwritten = FxFalse;
		}
		// Enable viewport redrawing.
		GetCOREInterface()->EnableSceneRedraw();
		// Refresh the Max rendering views.
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
	}
	return result;
}

//------------------------------------------------------------------------------
// Animation level operations.
//------------------------------------------------------------------------------

// Import the specified animation at the specified frame rate.
FxBool 
FxToolMax::ImportAnim( const FxString& group, const FxString& name, FxReal fps )
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
				// Disable viewport redrawing to speed up the import process.
				GetCOREInterface()->DisableSceneRedraw();

				// Initialize the animation controller.
				_initializeAnimController();

				// Clear out all animation curves associated with the bones contained
				// in the actor.
				_animController.Clean();

				const FxAnim& anim = animGroup.GetAnim(animIndex);
				// Set the Max animation range.
				TimeValue animBeginTime = (FxInt32)(anim.GetStartTime() * 4800);
				TimeValue animEndTime   = (FxInt32)(anim.GetEndTime() * 4800);
				Interval animInterval(animBeginTime, animEndTime);
				GetCOREInterface()->SetAnimRange(animInterval);
                
				if( _doMorph )
				{
					FxFaceGraph& faceGraph = _actor->GetDecompiledFaceGraph();
					FxArray<FX_MORPH_TARGET_NODE_CLASS_TYPE*> morphTargetNodes;
					morphTargetNodes.Reserve(faceGraph.CountNodes(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()));
					FxFaceGraph::Iterator morphTargetNodeIter    = faceGraph.Begin(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()); 
					FxFaceGraph::Iterator morphTargetNodeEndIter = faceGraph.End(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()); 
					for( ; morphTargetNodeIter != morphTargetNodeEndIter; ++morphTargetNodeIter ) 
					{ 
						FX_MORPH_TARGET_NODE_CLASS_TYPE* pMorphTargetNode = FxCast<FX_MORPH_TARGET_NODE_CLASS_TYPE>(morphTargetNodeIter.GetNode());
						if( pMorphTargetNode )
						{
							morphTargetNodes.PushBack(pMorphTargetNode);
						}
					}

					INode* pRootNode = GetCOREInterface()->GetRootNode();
					if( pRootNode )
					{
						OC3Ent::Face::FxArray<MorphR3*> MorpherArray;
						_recurseForMorpher( pRootNode, MorpherArray );
						for( OC3Ent::Face::FxSize i = 0; i < MorpherArray.Length(); ++i )
						{
							MorphR3* pMorphMod = MorpherArray[i];
							IParamBlock* paramBlock;
							Control * c;
							if( pMorphMod )
							{
								FxToolLog("attempting to find morph targets and the face graph morph nodes that drive them...");
								FxArray<FxName> nodesToBake;
								FxArray<morphChannel*> morphChannels;
								for( OC3Ent::Face::FxSize j=0; j<MR3_NUM_CHANNELS; j++ )
								{
									if( pMorphMod->chanBank[j].mActive )
									{
										morphChannel *pChan = &pMorphMod->chanBank[j];
										TSTR morphName = pChan->mName;
										FxChar* fxCharMorphName = (FxChar*) morphName.data();
										FxSize numMorphTargetNodes = morphTargetNodes.Length();
										for( FxSize morphNodeIndex = 0; morphNodeIndex < numMorphTargetNodes; ++morphNodeIndex ) 
										{ 
											FX_MORPH_TARGET_NODE_CLASS_TYPE* pMorphTargetNode = morphTargetNodes[morphNodeIndex];
											if( pMorphTargetNode )
											{
												const FxFaceGraphNodeUserProperty& morphTargetNameUserProperty = 
													pMorphTargetNode->GetUserProperty(FX_MORPH_TARGET_NODE_USERPROP_INDEX);
												if( morphTargetNameUserProperty.GetStringProperty() == fxCharMorphName )
												{
													morphChannels.PushBack(pChan);
													nodesToBake.PushBack(pMorphTargetNode->GetName());
												}
											}
										}
									}
								}
								FxFastFaceGraphBaker baker(_actor);
								baker.SetAnim(group.GetData(), name.GetData());
								if( baker.Bake(nodesToBake) )
								{
									// Make sure the baker has returned a curve for every name.  Since we know
									// that each name is a valid node in the face graph, this should always be
									// the case.
									if(baker.GetNumResultAnimCurves() == nodesToBake.Length())
									{
										for( FxSize bakerIndex = 0; bakerIndex < nodesToBake.Length(); ++bakerIndex ) 
										{ 
											morphChannel *pChan = morphChannels[bakerIndex];
											TSTR morphName = pChan->mName;
											FxChar* fxCharMorphName = (FxChar*) morphName.data();

											const FxAnimCurve& animCurve = baker.GetResultAnimCurve(bakerIndex);
											paramBlock = pChan->cblock;
											c = paramBlock->GetController(0);
											if( c )
											{
												IKeyControl *ikc;
												ikc = GetKeyControlInterface(c);
												if( ikc )
												{
													ikc->SetNumKeys(0);
													SuspendAnimate();
													AnimateOn();
													if( c->ClassID() == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID , 0) )
													{
														IBezFloatKey bezFKey;
														SetInTanType(bezFKey.flags,BEZKEY_SLOW); 
														SetOutTanType(bezFKey.flags,BEZKEY_SLOW);
														for( OC3Ent::Face::FxSize k = 0; k< animCurve.GetNumKeys(); k++ )
														{
															bezFKey.val = (float)(100 * animCurve.GetKey(k).GetValue());
															bezFKey.time = (int) (TIME_TICKSPERSEC * animCurve.GetKey(k).GetTime());
															
															FxReal slopeIn = animCurve.GetKey(k).GetSlopeIn();
															FxReal slopeOut = animCurve.GetKey(k).GetSlopeOut();
															if( !FxRealEquality(slopeIn,0.0f) ||
																!FxRealEquality(slopeOut,0.0f) )
															{
																bezFKey.inLength = 	0.333f;
																bezFKey.outLength = 0.333f;
																bezFKey.intan = -slopeIn;
																bezFKey.outtan = slopeOut;
															}
															ikc->AppendKey(&bezFKey);
														}
														// Make sure there is at least one key in the morph channel
														if( 0 == animCurve.GetNumKeys() )
														{
															bezFKey.val = 0.0f;
															bezFKey.time = 0;
															ikc->AppendKey(&bezFKey);
															FxToolLog("Added 1 key at time 0 to the %s morph channel morpher modifier number %d.", fxCharMorphName, i);
														}
														else
														{
															FxToolLog("Added %d keys to the %s morph channel morpher modifier number %d.", animCurve.GetNumKeys(), fxCharMorphName, i);
														}
													}
													else
													{
														FxToolLog("Unsupported morph key controller detected on the %s morph channel. The following Class IDs are supported: HYBRIDINTERP_FLOAT_CLASS_ID", fxCharMorphName);
													}
													ResumeAnimate();
												}
											}
										}
									}
								}
							}
						}
					}
				}

				// Max specific optimization.
				_animController.PrepareRotationControllers();
				// Import the animation.
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
					GetCOREInterface()->SetTime((FxInt32)(i * 4800));
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
				// Max specific optimization.
				_animController.RestoreRotationControllers();
				// Go back to the first frame of the animation.
				GetCOREInterface()->SetTime(animBeginTime);
				// Enable viewport redrawing.
				GetCOREInterface()->EnableSceneRedraw();
				// Refresh the Max rendering views.
				GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
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
// Max level operations.
//------------------------------------------------------------------------------

// Set the Max interface pointer.
void FxToolMax::SetMaxInterfacePointer( Interface* pMaxInterface )
{
	_pMaxInterface = pMaxInterface;
	_animController.SetMaxInterfacePointer(_pMaxInterface);
}

// Set the normalize scale option (the default is FxTrue).
void FxToolMax::SetNormalizeScale( FxBool normalizeScale )
{
	_normalizeScale = normalizeScale;
}

// Get the normalize scale option (the default is FxTrue).
FxBool FxToolMax::GetNormalizeScale( void ) const
{
	return _normalizeScale;
}

// Cleans up all animation curves in Max associated with the current
// actor.
void FxToolMax::CleanBonesMorph( void )
{
	if( _actor )
	{
		// Turn off warning messages.  If there are bones with invalid controllers, 
		// there won't be anything to clean up in the first place.
		OC3Ent::Face::FxBool warningDialogState = GetShouldDisplayWarningDialogs();
		SetShouldDisplayWarningDialogs(FxFalse);

		// Stop any currently playing animation.
		_stopAnimationPlayback();
		// Disable viewport redrawing to speed up the import process.
		GetCOREInterface()->DisableSceneRedraw();
		
		// Initialize the animation controller.
		_initializeAnimController();

		// Clear out all animation curves associated with the bones contained
		// in the actor.
		_animController.Clean();

		// Import the reference pose at frame 0.  This will set the current time to
		// 0 after the clean operation, and make sure the character is not stuck with 
		// no keys in the middle of an animation.
		ImportRefPose(0);

		if( _doMorph )
		{
			FxFaceGraph& faceGraph = _actor->GetDecompiledFaceGraph();
			FxArray<FX_MORPH_TARGET_NODE_CLASS_TYPE*> morphTargetNodes;
			morphTargetNodes.Reserve(faceGraph.CountNodes(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()));
			FxFaceGraph::Iterator morphTargetNodeIter    = faceGraph.Begin(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()); 
			FxFaceGraph::Iterator morphTargetNodeEndIter = faceGraph.End(FX_MORPH_TARGET_NODE_CLASS_TYPE::StaticClass()); 
			for( ; morphTargetNodeIter != morphTargetNodeEndIter; ++morphTargetNodeIter ) 
			{ 
				FX_MORPH_TARGET_NODE_CLASS_TYPE* pMorphTargetNode = FxCast<FX_MORPH_TARGET_NODE_CLASS_TYPE>(morphTargetNodeIter.GetNode());
				if( pMorphTargetNode )
				{
					morphTargetNodes.PushBack(pMorphTargetNode);
				}
			}

			INode* pRootNode = GetCOREInterface()->GetRootNode();
			if( pRootNode )
			{
				OC3Ent::Face::FxArray<MorphR3*> MorpherArray;
				_recurseForMorpher( pRootNode, MorpherArray );
				for( OC3Ent::Face::FxSize i = 0; i < MorpherArray.Length(); ++i )
				{
					MorphR3* pMorphMod = MorpherArray[i];
					IParamBlock* paramBlock;
					Control * c;
					if( pMorphMod )
					{
						for( OC3Ent::Face::FxSize j=0; j<MR3_NUM_CHANNELS; j++ )
						{
							if( pMorphMod->chanBank[j].mActive )
							{
								morphChannel *pChan = &pMorphMod->chanBank[j];
								TSTR morphName = pChan->mName;
								FxChar* fxCharMorphName = (FxChar*) morphName.data();
								FxSize numMorphTargetNodes = morphTargetNodes.Length();
								for( FxSize morphNodeIndex = 0; morphNodeIndex < numMorphTargetNodes; ++morphNodeIndex ) 
								{ 
									FX_MORPH_TARGET_NODE_CLASS_TYPE* pMorphTargetNode = morphTargetNodes[morphNodeIndex];
									if( pMorphTargetNode )
									{
										const FxFaceGraphNodeUserProperty& morphTargetNameUserProperty = 
											pMorphTargetNode->GetUserProperty(FX_MORPH_TARGET_NODE_USERPROP_INDEX);

										if( morphTargetNameUserProperty.GetStringProperty() == fxCharMorphName )
										{
											paramBlock = pChan->cblock;
											c = paramBlock->GetController(0);
											if( c )
											{
												IKeyControl *ikc;
												ikc = GetKeyControlInterface(c);
												if( ikc )
												{
													SuspendAnimate();
													AnimateOn();
													ikc->SetNumKeys(0);
													if( c->ClassID() == Class_ID(HYBRIDINTERP_FLOAT_CLASS_ID , 0) )
													{
														// Set a key with zero value at time zero. This deletes any residual 
														// values left over from the animation.
														IBezFloatKey bezFKey;
														SetInTanType(bezFKey.flags,BEZKEY_SLOW); 
														SetOutTanType(bezFKey.flags,BEZKEY_SLOW);
														bezFKey.val = 0.0f;
														bezFKey.time = 0;
														ikc->AppendKey(&bezFKey);
													}
													ResumeAnimate();
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		// Enable viewport redrawing.
		GetCOREInterface()->EnableSceneRedraw();
		// Refresh the Max rendering views.
		GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
		FxToolLog("cleaned animation timeline");

		// Return the warning dialog state back to what it was.
		SetShouldDisplayWarningDialogs(warningDialogState);
	}
}
// Finds the specified bone in the Max scene and returns it's transformation.
// This only returns the bone's transformation in the current frame.  
// Returns FxTrue if the bone was found and the transform is valid or
// FxFalse otherwise.
FxBool FxToolMax::_getBoneTransform( const FxString& boneName, FxBone& bone )
{
	// Find the bone in the Max scene.
	INode* pNode = GetCOREInterface()->GetINodeByName(_T(boneName.GetData()));
	if( pNode )
	{
		// Set the bone name.
		bone.SetName(boneName.GetData());

		// Get the node's transformation matrix.
		TimeValue currentTime = GetCOREInterface()->GetTime();
		Matrix3 nodeTransformationMatrix;
		INode* pParentNode = pNode->GetParentNode();

		if( pParentNode )
		{
			nodeTransformationMatrix = pNode->GetNodeTM(currentTime) *
				Inverse(pParentNode->GetNodeTM(currentTime));
		}
		else
		{
			nodeTransformationMatrix = pNode->GetNodeTM(currentTime);
		}
		
		// Get rid of the scale components if requested.
		if( _normalizeScale )
		{
			Matrix3 nodeMatrix = pNode->GetNodeTM(currentTime);
			Matrix3 nodeMatrixNoScale = RemoveNonUniformScaling(nodeMatrix);

			if( pParentNode )
			{
				Matrix3 parentMatrix = pParentNode->GetNodeTM(currentTime);
				Matrix3 parentMatrixNoScale = RemoveNonUniformScaling(parentMatrix);
				
				nodeTransformationMatrix = nodeMatrixNoScale * 
					Inverse(parentMatrixNoScale);
			}
			else
			{
				nodeTransformationMatrix = nodeMatrixNoScale;
			}
		}

		// Decompose the node's transformation matrix.
		AffineParts nodeTransformationMatrixParts;
		decomp_affine(nodeTransformationMatrix, &nodeTransformationMatrixParts);

#ifdef __FX_USE_NATIVE_COORDS__
		// Set the translation.
		bone.SetPos(FxVec3(nodeTransformationMatrixParts.t.x,
							nodeTransformationMatrixParts.t.y,
							nodeTransformationMatrixParts.t.z));
		
		// Set the scale.
		bone.SetScale(FxVec3(nodeTransformationMatrixParts.k.x,
								nodeTransformationMatrixParts.k.y,
								nodeTransformationMatrixParts.k.z));

		// Set the rotation.
		bone.SetRot(FxQuat(nodeTransformationMatrixParts.q.w,
							nodeTransformationMatrixParts.q.x,
							nodeTransformationMatrixParts.q.y,
							nodeTransformationMatrixParts.q.z));
#else
		// Set the translation.
		bone.SetPos(FxVec3(nodeTransformationMatrixParts.t.x,
				            nodeTransformationMatrixParts.t.z,
				            nodeTransformationMatrixParts.t.y));

		// Set the scale.
		bone.SetScale(FxVec3(nodeTransformationMatrixParts.k.x,
				                nodeTransformationMatrixParts.k.z,
				                nodeTransformationMatrixParts.k.y));

		// Set the rotation.
		bone.SetRot(FxQuat(nodeTransformationMatrixParts.q.w,
				            nodeTransformationMatrixParts.q.x,
				            nodeTransformationMatrixParts.q.z,
				            nodeTransformationMatrixParts.q.y));
#endif

		// Found the bone.
		return FxTrue;
	}

	// The bone was not found.
	return FxFalse;
}

// Sets the specified bone's transform for the current frame.
// Returns FxTrue if the transform was set or FxFalse otherwise.
FxBool FxToolMax::_setBoneTransform( const FxBone& bone )
{
	// Find the bone animation curve associated with the passed in bone.
	FxMaxAnimCurve* pBoneAnimCurve = _animController.FindAnimCurve(bone.GetNameAsString());
	if( pBoneAnimCurve )
	{
		if( pBoneAnimCurve->pPositionController && pBoneAnimCurve->pPositionKeys )
		{
			if( !pBoneAnimCurve->pPositionController->IsKeyAtTime(GetCOREInterface()->GetTime(), KEYAT_POSITION) )
			{
				pBoneAnimCurve->pPositionController->AddNewKey(GetCOREInterface()->GetTime(), ADDKEY_INTERP);
			}

			FxInt32 keyIndex = pBoneAnimCurve->pPositionController->GetKeyIndex(GetCOREInterface()->GetTime());
			if( -1 != keyIndex )
			{
				if( Class_ID(HYBRIDINTERP_POSITION_CLASS_ID, 0) == pBoneAnimCurve->pPositionController->ClassID() )
				{
					IBezPoint3Key bezPosKey;
					bezPosKey.time  = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					bezPosKey.val.x = bone.GetPos().x;
					bezPosKey.val.y = bone.GetPos().y;
					bezPosKey.val.z = bone.GetPos().z;
#else
					bezPosKey.val.x = bone.GetPos().x;
					bezPosKey.val.y = bone.GetPos().z;
					bezPosKey.val.z = bone.GetPos().y;
#endif
					pBoneAnimCurve->pPositionKeys->SetKey(keyIndex, &bezPosKey);
				}
				else if( Class_ID(TCBINTERP_POSITION_CLASS_ID, 0) == pBoneAnimCurve->pPositionController->ClassID() )
				{
					// When compiling in .NET, TCB controllers do 
					// not work properly between the keys we set, because 
					// the tension, continuity, and bias are screwed up.  
					// This only happens when compiling in .NET.  
					// To fix it, GetTCBDefaultParams() is called to get the
					// Max system-wide TCB parameters, which are then set
					// for each key.
					ITCBPoint3Key tcbPosKey;
					float tension    = 0.0f;
					float continuity = 0.0f;
					float bias       = 0.0f;
					float easeIn     = 0.0f;
					float easeOut    = 0.0f;
					GetTCBDefaultParams(tension, continuity, bias, easeIn, easeOut);
					tcbPosKey.tens    = tension; 
					tcbPosKey.cont    = continuity; 
					tcbPosKey.bias    = bias;
					tcbPosKey.easeIn  = easeIn;
					tcbPosKey.easeOut = easeOut;
					tcbPosKey.time    = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					tcbPosKey.val.x   = bone.GetPos().x;
					tcbPosKey.val.y   = bone.GetPos().y;
					tcbPosKey.val.z   = bone.GetPos().z;
#else
					tcbPosKey.val.x   = bone.GetPos().x;
					tcbPosKey.val.y   = bone.GetPos().z;
					tcbPosKey.val.z   = bone.GetPos().y;
#endif
					pBoneAnimCurve->pPositionKeys->SetKey(keyIndex, &tcbPosKey);
				}
				else if( Class_ID(LININTERP_POSITION_CLASS_ID, 0) == pBoneAnimCurve->pPositionController->ClassID() )
				{
					ILinPoint3Key linPosKey;
					linPosKey.time  = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					linPosKey.val.x = bone.GetPos().x;
					linPosKey.val.y = bone.GetPos().y;
					linPosKey.val.z = bone.GetPos().z;
#else
					linPosKey.val.x = bone.GetPos().x;
					linPosKey.val.y = bone.GetPos().z;
					linPosKey.val.z = bone.GetPos().y;
#endif
					pBoneAnimCurve->pPositionKeys->SetKey(keyIndex, &linPosKey);
				}
			}
		}

		if( pBoneAnimCurve->pScaleController && pBoneAnimCurve->pScaleKeys 
#ifdef __FX_MAX9__
			// Max 9 has a bug with setting scale keys at negative time.
			// Change Request #930503
			// Not fixed as of Service pack 2.
			&&   GetCOREInterface()->GetTime() >= 0 
#endif
			)
		{
			if( !pBoneAnimCurve->pScaleController->IsKeyAtTime(GetCOREInterface()->GetTime(), KEYAT_SCALE) )
			{
				pBoneAnimCurve->pScaleController->AddNewKey(GetCOREInterface()->GetTime(), ADDKEY_INTERP);
			}

			FxInt32 keyIndex = pBoneAnimCurve->pScaleController->GetKeyIndex(GetCOREInterface()->GetTime());
			if( -1 != keyIndex )
			{
				if( Class_ID(HYBRIDINTERP_SCALE_CLASS_ID, 0) == pBoneAnimCurve->pScaleController->ClassID() )
				{
					IBezPoint3Key bezScaleKey;
					bezScaleKey.time  = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					bezScaleKey.val.x = bone.GetScale().x;
					bezScaleKey.val.y = bone.GetScale().y;
					bezScaleKey.val.z = bone.GetScale().z;
#else
					bezScaleKey.val.x = bone.GetScale().x;
					bezScaleKey.val.y = bone.GetScale().z;
					bezScaleKey.val.z = bone.GetScale().y;
#endif

					pBoneAnimCurve->pScaleKeys->SetKey(keyIndex, &bezScaleKey);
				}
				else if( Class_ID(TCBINTERP_SCALE_CLASS_ID, 0) == pBoneAnimCurve->pScaleController->ClassID() )
				{
					// When compiling in .NET, TCB controllers do 
					// not work properly between the keys we set, because 
					// the tension, continuity, and bias are screwed up.  
					// This only happens when compiling in .NET.  
					// To fix it, GetTCBDefaultParams() is called to get the
					// Max system-wide TCB parameters, which are then set
					// for each key.
					ITCBPoint3Key tcbScaleKey;
					float tension    = 0.0f;
					float continuity = 0.0f;
					float bias       = 0.0f;
					float easeIn     = 0.0f;
					float easeOut    = 0.0f;
					GetTCBDefaultParams(tension, continuity, bias, easeIn, easeOut);
					tcbScaleKey.tens    = tension; 
					tcbScaleKey.cont    = continuity; 
					tcbScaleKey.bias    = bias;
					tcbScaleKey.easeIn  = easeIn;
					tcbScaleKey.easeOut = easeOut;
					tcbScaleKey.time    = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					tcbScaleKey.val.x   = bone.GetScale().x;
					tcbScaleKey.val.y   = bone.GetScale().y;
					tcbScaleKey.val.z   = bone.GetScale().z;
#else
					tcbScaleKey.val.x   = bone.GetScale().x;
					tcbScaleKey.val.y   = bone.GetScale().z;
					tcbScaleKey.val.z   = bone.GetScale().y;
#endif
					pBoneAnimCurve->pScaleKeys->SetKey(keyIndex, &tcbScaleKey);
				}
				else if( Class_ID(LININTERP_SCALE_CLASS_ID, 0) == pBoneAnimCurve->pScaleController->ClassID() )
				{
					ILinPoint3Key linScaleKey;
					linScaleKey.time  = GetCOREInterface()->GetTime();
#ifdef __FX_USE_NATIVE_COORDS__
					linScaleKey.val.x = bone.GetScale().x;
					linScaleKey.val.y = bone.GetScale().y;
					linScaleKey.val.z = bone.GetScale().z;
#else
					linScaleKey.val.x = bone.GetScale().x;
					linScaleKey.val.y = bone.GetScale().z;
					linScaleKey.val.z = bone.GetScale().y;
#endif
					pBoneAnimCurve->pScaleKeys->SetKey(keyIndex, &linScaleKey);
				}
			}
		}

		if( pBoneAnimCurve->pRotationController && pBoneAnimCurve->pRotationKeys )
		{
			if( !pBoneAnimCurve->pRotationController->IsKeyAtTime(GetCOREInterface()->GetTime(), KEYAT_ROTATION) )
			{
				pBoneAnimCurve->pRotationController->AddNewKey(GetCOREInterface()->GetTime(), ADDKEY_INTERP);
			}

			FxInt32 keyIndex = pBoneAnimCurve->pRotationController->GetKeyIndex(GetCOREInterface()->GetTime());
			if( -1 != keyIndex )
			{
				if( Class_ID(TCBINTERP_ROTATION_CLASS_ID, 0) == pBoneAnimCurve->pRotationController->ClassID() )
				{
					// TCB rotations in Max are stored with each successive quaternion relative to the
					// previous quaternion.  This makes inserting keys at "random" locations in the key
					// list problematic because the complete quaternion "chain" needs to be recomputed.
					// Several methods were attempted to do this correctly, but the documentation was
					// incomplete and the SDK appeared to be missing some functionality to handle this
					// operation.  The operation worked fine in MaxScript and the Max GUI, but the SDK
					// code just did not work the same way.  That is where this hack-ish procedure came
					// from - it was either this or drop support for TCB rotations completely.

					// Brute force switching rotation controllers by hacking into MaxScript.
					FxString maxScript = "tempobj = getnodebyname \"";
					maxScript = FxString::Concat(maxScript, pBoneAnimCurve->boneName);
					maxScript = FxString::Concat(maxScript, "\"");
					GUP* pGP = (GUP*)CreateInstance(GUP_CLASS_ID, Class_ID(470000002, 0));
					pGP->ExecuteStringScript(_T(const_cast<FxChar*>(maxScript.GetData())));
					pGP->ExecuteStringScript(_T("tempobj.rotation.controller = Linear_Rotation()"));

					// Hook into the new controllers.
					pBoneAnimCurve->pRotationController = pBoneAnimCurve->pNode->GetTMController()->GetRotationController();
					pBoneAnimCurve->pRotationKeys       = GetKeyControlInterface(pBoneAnimCurve->pRotationController);

					if( pBoneAnimCurve->pRotationKeys )
					{
						FxInt32 newKeyIndex = pBoneAnimCurve->pRotationController->GetKeyIndex(GetCOREInterface()->GetTime());
						if( -1 != newKeyIndex )
						{
							Quat newQuat;
#ifdef __FX_USE_NATIVE_COORDS__
							newQuat.w = bone.GetRot().w;
							newQuat.x = bone.GetRot().x;
							newQuat.y = bone.GetRot().y;
							newQuat.z = bone.GetRot().z;
#else
							newQuat.w = bone.GetRot().w;
							newQuat.x = bone.GetRot().x;
							newQuat.y = bone.GetRot().z;
							newQuat.z = bone.GetRot().y;
#endif
							ILinRotKey linRotKey;
							linRotKey.time = GetCOREInterface()->GetTime();
							linRotKey.val  = newQuat;
							pBoneAnimCurve->pRotationKeys->SetKey(keyIndex, &linRotKey);
						}
					}

					// Brute force switching rotation controllers by hacking into MaxScript.
					pGP->ExecuteStringScript(_T("tempobj.rotation.controller = TCB_Rotation()"));

					// Hook into the new controllers.
					pBoneAnimCurve->pRotationController = pBoneAnimCurve->pNode->GetTMController()->GetRotationController();
					pBoneAnimCurve->pRotationKeys       = GetKeyControlInterface(pBoneAnimCurve->pRotationController);
				}
				else if( Class_ID(HYBRIDINTERP_ROTATION_CLASS_ID, 0) == pBoneAnimCurve->pRotationController->ClassID() )
				{
					Quat newQuat;
#ifdef __FX_USE_NATIVE_COORDS__
					newQuat.w = bone.GetRot().w;
					newQuat.x = bone.GetRot().x;
					newQuat.y = bone.GetRot().y;
					newQuat.z = bone.GetRot().z;
#else
					newQuat.w = bone.GetRot().w;
					newQuat.x = bone.GetRot().x;
					newQuat.y = bone.GetRot().z;
					newQuat.z = bone.GetRot().y;
#endif
					IBezQuatKey bezRotKey;					
					bezRotKey.time = GetCOREInterface()->GetTime();
					bezRotKey.val  = newQuat;
					pBoneAnimCurve->pRotationKeys->SetKey(keyIndex, &bezRotKey);
				}
				else if( Class_ID(LININTERP_ROTATION_CLASS_ID, 0) == pBoneAnimCurve->pRotationController->ClassID() )
				{
					Quat newQuat;
#ifdef __FX_USE_NATIVE_COORDS__
					newQuat.w = bone.GetRot().w;
					newQuat.x = bone.GetRot().x;
					newQuat.y = bone.GetRot().y;
					newQuat.z = bone.GetRot().z;
#else
					newQuat.w = bone.GetRot().w;
					newQuat.x = bone.GetRot().x;
					newQuat.y = bone.GetRot().z;
					newQuat.z = bone.GetRot().y;
#endif
					ILinRotKey linRotKey;					
					linRotKey.time = GetCOREInterface()->GetTime();
					linRotKey.val  = newQuat;
					pBoneAnimCurve->pRotationKeys->SetKey(keyIndex, &linRotKey);
				}
			}
		}
		// Found the bone animation curve.
		return FxTrue;
	}
	return FxFalse;
}

// Initializes the animation controller.
FxBool FxToolMax::_initializeAnimController( void )
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

// Stops any currently playing animation in Max.
void FxToolMax::_stopAnimationPlayback( void )
{
	if( GetCOREInterface()->IsAnimPlaying() )
	{
		GetCOREInterface()->EndAnimPlayback();
	}
}
// Returns an array of Morpher Modifiers found in the scene.
bool FxToolMax::_recurseForMorpher( INode* node, OC3Ent::Face::FxArray<MorphR3*>& MorpherArray )
{
	if( node )
	{
		MorphR3* pMorphMod = _findMorpherModifier(node);
		if( pMorphMod )
		{
			MorpherArray.PushBack(pMorphMod);
			FxToolLog("Found morpher modifier for %s", (OC3Ent::Face::FxChar*)(node->GetName()));
		}
		int numChildren = node->NumChildren();
		int i;
		for (i = 0 ; i < numChildren ; ++i)
		{
			 _recurseForMorpher(node->GetChildNode(i), MorpherArray);
		}
	}
	return true;
}

MorphR3* FxToolMax::_findMorpherModifier( INode* pNode )
{
	if( pNode )
	{
		// Get the node object.
		Object* pObject = pNode->GetObjectRef();
		if( pObject )
		{
			// Is it a derived object?
			if( GEN_DERIVOB_CLASS_ID == pObject->SuperClassID() )
			{
				// Iterate over all entries of the modifier stack.
				IDerivedObject* pDerivedObject = static_cast<IDerivedObject*>(pObject);
				OC3Ent::Face::FxInt32 modStackIndex = 0;
				while( modStackIndex < pDerivedObject->NumModifiers() )
				{
					Modifier* pModifier = pDerivedObject->GetModifier(modStackIndex);
					if( MR3_CLASS_ID == pModifier->ClassID() )
					{
						return static_cast<MorphR3*>(pModifier);
					}
					modStackIndex++;
				}
			}
		}
	}
	return NULL;
}

} // namespace Face

} // namespace OC3Ent
