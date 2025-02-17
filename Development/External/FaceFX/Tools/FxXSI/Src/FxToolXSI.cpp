//------------------------------------------------------------------------------
// The FaceFx XSI plug-in interface.
//

#include "FxActorInstance.h"
#include "FxToolXSI.h"
#include "FxToolLog.h"
#include "FxXSIHelper.h"
#include "FxFaceGraphBaker.h"

//XSI interfaces
#include <xsi_application.h>
#include <xsi_value.h>
#include <xsi_model.h>
#include <xsi_fcurve.h>
#include <xsi_parameter.h>
#include <xsi_kinematicstate.h>
#include <xsi_kinematics.h>
#include <xsi_quaternion.h>
#include <xsi_string.h>
#include <xsi_ppglayout.h>

#include <FxSDK.h>

using namespace XSI;

namespace OC3Ent
{

	namespace Face
	{

#define kCurrentFxToolXSIVersion 0

		// Implement the class.
		FX_IMPLEMENT_CLASS(FxToolXSI, kCurrentFxToolXSIVersion, FxTool);

		// Constructor.
		FxToolXSI::FxToolXSI()
		{
		}

		// Destructor.
		FxToolXSI::~FxToolXSI()
		{
		}

		//------------------------------------------------------------------------------
		// Initialization and shutdown.
		//------------------------------------------------------------------------------

		// Starts up the tool.  If this is overloaded, make sure to call the
		// base class version first.
		FxBool FxToolXSI::Startup( void )
		{
			FxMemoryAllocationPolicy allocationPolicy = FxGetMemoryAllocationPolicy();
			if(allocationPolicy.userAllocate == NULL)
			{

				Application app;
				CRefArray props = app.GetActiveSceneRoot().GetProperties();
				for(int i = 0; i < props.GetCount();++i)
				{
					if(SIObject(props[i]).GetName().IsEqualNoCase(L"FaceFX"))
					{
						m_UI = props[i];
						break;
					}
				}
				
				return Super::Startup();
			}
			return FxTrue;
		}

		// Shuts down the tool.  If this is overloaded, make sure to call the
		// base class version first.
		FxBool FxToolXSI::Shutdown( void )
		{
			FxMemoryAllocationPolicy allocationPolicy = FxGetMemoryAllocationPolicy();
			if(allocationPolicy.userAllocate != NULL)
			{
				return Super::Shutdown();
			}
			return FxTrue;
		}

		//------------------------------------------------------------------------------
		// Actor level operations.
		//------------------------------------------------------------------------------

		// Creates a new actor with the specified name.
		FxBool FxToolXSI::CreateActor( const FxString& name )
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
		FxBool FxToolXSI::LoadActor( const FxString& filename )
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
					XSI::Application app;
					app.LogMessage(L"Failed to load the file");
				}
				FxToolLog("failed to load actor file %s", filename.GetData());
			}
			return result; 
		}

		// Saves an actor.
		FxBool FxToolXSI::SaveActor( const FxString& filename )
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
		FxBool FxToolXSI::ImportRefPose( FxInt32 frame )
		{
			FxToolLog("attempting to import reference pose to frame %d", frame);
			if( _actor )
			{
				// Stop any currently playing animation.
				_stopAnimationPlayback();

				// Initialize the animation controller.
				_initializeAnimController();

				// Go to the time corresponding to the frame passed to the command.

				XSISetCurrentFrame(frame);

				// Import each bone from the reference pose.
				for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
				{
					const FxBone& refBone = _actor->GetMasterBoneList().GetRefBone(i);
					if( !_setBoneTransform(refBone) )
					{
						FxToolLog("_setBoneTransform failed for bone %s", refBone.GetNameAsCstr());
					}
				}
				FxToolLog("imported reference pose");
				return FxTrue;
			}
			return FxFalse;
		}

		// Exports the reference pose from the specified frame containing the 
		// specified bones.
		FxBool FxToolXSI::ExportRefPose( FxInt32 frame, const FxArray<FxString>& bones )
		{
			Application app;
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
					Application app;
					app.LogMessage(L"FxXSI: Could not export reference pose.  One or more bone names contained a pipe character.");
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
				double oldFrame = XSIGetCurrentFrame();
				XSISetCurrentFrame(frame);

				if(bones.Length() == 0)
				{
					app.LogMessage(L"FxXSI: No bones selected for Reference Pose.  Select bones before export to create a valid reference pose.");
				}
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
				XSISetCurrentFrame(oldFrame);

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
		FxBool FxToolXSI::ImportBonePose( const FxString& name, FxInt32 frame )
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
					XSISetCurrentFrame(frame);

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
		FxBool FxToolXSI::ExportBonePose( FxInt32 frame, const FxString& name )
		{
			Application app;
			FxToolLog("attempting to export bone pose from frame %d", frame);
			if( _actor )
			{
				// Stop any currently playing animation.
				_stopAnimationPlayback();

				if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
				{
					if( GetShouldDisplayWarningDialogs() )
					{		
						app.LogMessage(L"FxXSI: Creating Bone Pose failed.  Please export a reference pose first.");
						FxToolLog("Creating Bone Pose failed.  Make sure you export a reference pose first.");
					}
					return FxFalse;
				}

				// Save the current time and go to the time corresponding to the frame
				// passed to the command.
				double oldTime = XSIGetCurrentFrame();
				XSISetCurrentFrame(frame);

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
									app.LogMessage(L"FxXSI: Face Graph node overwritten.  Check the log file for details.");
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
				XSISetCurrentFrame(oldTime);

				if( 0 == pBonePoseNode->GetNumBones() )
				{
					if( !_isBatchOp )
					{
						if( GetShouldDisplayWarningDialogs() )
						{
							app.LogMessage(L"FxXSI: Exported bone pose is identical to the Reference Pose and contains no bones.");
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

/* D.P. 02/27/2008 - Removed Delete Bone Pose functions.
		FxBool FxToolXSI::RemoveBonePose(const FxString& name )
		{
			if( _actor )
			{
				// Stop any currently playing animation.
				_stopAnimationPlayback();
				FxName nameName(name.GetData());

				if(_actor->GetDecompiledFaceGraph().RemoveNode(nameName))
				{
					_actor->GetMasterBoneList().Prune(_actor->GetDecompiledFaceGraph());
					// Compile - this also "pulls" the bones.
					_actor->CompileFaceGraph();
					_bonePosesChanged();
				}
				
				return FxTrue;
			}
			return FxFalse;
		}
*/
		// Batch imports bone poses using the specified configuration file.
		FxBool FxToolXSI::BatchImportBonePoses( const FxString& filename, 
			FxInt32& startFrame, FxInt32& endFrame,
			FxBool FxUnused(onlyFindFrameRange) )
		{
			FxBool result = FxFalse;
			if( _actor )
			{
				FxToolLog("attempting to batch import bone poses from file %s", filename.GetData());
				// Stop any currently playing animation.
				_stopAnimationPlayback();

				// Initialize the animation controller.
				_initializeAnimController();

				// Clear out all animation curves associated with the bones contained
				// in the actor.
				m_animController.Clean();

				// Batch import the bone poses.
				startFrame = FxInvalidIndex;
				endFrame   = FxInvalidIndex;
				result = Super::BatchImportBonePoses(filename, startFrame, endFrame);
				if( result )
				{
					if( FxInvalidIndex != startFrame && FxInvalidIndex != endFrame )
					{
						XSISetSceneEndFrame(endFrame);
						XSISetSceneStartFrame(startFrame);
						XSISetCurrentFrame(startFrame);
					}
					FxToolLog("batch imported bone poses - range [%d, %d]", startFrame, endFrame);
				}
			}
			return result;
		}

		// Batch exports bone poses using the specified configuration file.
		FxBool FxToolXSI::BatchExportBonePoses( const FxString& filename )
		{
			Application app;
			FxBool result = FxFalse;
			if( _actor )
			{
				FxToolLog("attempting to batch export bone poses from file %s", filename.GetData());
				if( 0 == _actor->GetMasterBoneList().GetNumRefBones() )
				{
					if( GetShouldDisplayWarningDialogs() )
					{
						app.LogMessage(L"FxXSI: Creating Bone Pose failed.  Make sure you export a reference pose first.");
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
						app.LogMessage(L"FxXSI: Some exported bone poses are identical to the Reference Pose and contain no bones. Check the log file for details.");
					}
					_allBonesPruned = FxFalse;
				}
				if( _faceGraphNodesOverwritten )
				{
					if( GetShouldDisplayWarningDialogs() )
					{
						app.LogMessage(L"FxXSI: Some Face Graph nodes were overwritten.  Check the log file for details.");
					}
					_faceGraphNodesOverwritten = FxFalse;
				}
			}
			return result;
		}

		//------------------------------------------------------------------------------
		// Animation level operations.
		//------------------------------------------------------------------------------

	
		// Import the specified animation at the specified frame rate.
		FxBool FxToolXSI::ImportAnim( const FxString& group, const FxString& name, FxReal fps )
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
						// Initialize the animation controller.
						_initializeAnimController();

						// Clear out all animation curves associated with the bones contained
						// in the actor.
						m_animController.Clean();
						
						const FxAnim& anim = animGroup.GetAnim(animIndex);
						// Set the Maya animation range.

						// Import the animation.

						XSISetSceneEndFrame(anim.GetEndTime());
						XSISetSceneStartFrame(anim.GetStartTime());
						
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
							XSISetCurrentFrame(i*fps);
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
						XSISetCurrentFrame(anim.GetStartTime());
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
		// actor.
		void FxToolXSI::Clean( void )
		{
			if( _actor )
			{
				// Initialize the animation controller.
				_initializeAnimController();

				// Clear out all animation curves associated with the bones contained
				// in the actor.
				m_animController.Clean();

				// Import the reference pose to frame zero so that the character
				// is not in a funky state.  If a bone attribute has an animation curve
				// plugged into it, but that animation curve contains no keys, 
				// apparently Maya doesn't know what to do with it and reverts the bone
				// attribute to zero rather than just not driving the attribute.
				ImportRefPose(0);
			}
		}

		// Finds the specified bone in Maya's DAG and returns it's transformation.
		// This only returns the bone's transformation in the current frame.  
		// Returns FxTrue if the bone was found and the transform is valid or
		// FxFalse otherwise.
		FxBool FxToolXSI::_getBoneTransform( const FxString& boneName, FxBone& bone )
		{
			Application app;
			Model root = app.GetActiveSceneRoot();
			CString xsiBoneName;
			xsiBoneName.PutAsciiString(boneName.GetData());
			X3DObject boneObject = root.FindChild(xsiBoneName,L"",CStringArray());
			if(!boneObject.IsValid())
			{
				return FxFalse;
			}

			// Set the bone name.
			bone.SetName(boneName.GetData());

			KinematicState localState = boneObject.GetKinematics().GetLocal();
			MATH::CTransformation localTransform = localState.GetTransform();

#ifdef __FX_USE_NATIVE_COORDS__
			// Set the translation.
			MATH::CVector3  translation = localTransform.GetTranslation();
			bone.SetPos(FxVec3(static_cast<FxReal>(translation.GetX()), 
				static_cast<FxReal>(translation.GetY()), 
				static_cast<FxReal>(translation.GetZ())));

			// Set the scale.
			MATH::CVector3  scaling = localTransform.GetScaling();
			bone.SetScale(FxVec3(static_cast<FxReal>(scaling.GetX()), 
				static_cast<FxReal>(scaling.GetY()), 
				static_cast<FxReal>(scaling.GetZ())));

			// Set the rotation.
			MATH::CQuaternion rotation = localTransform.GetRotationQuaternion();
			bone.SetRot(FxQuat(static_cast<FxReal>(-rotation.GetW()),
				static_cast<FxReal>(rotation.GetX()),
				static_cast<FxReal>(rotation.GetY()),
				static_cast<FxReal>(rotation.GetZ())));
#else
			// Set the translation.
			MATH::CVector3  translation = localTransform.GetTranslation();
			bone.SetPos(FxVec3(static_cast<FxReal>(-translation.GetX()), 
				static_cast<FxReal>(translation.GetY()), 
				static_cast<FxReal>(translation.GetZ())));

			// Set the scale.
			MATH::CVector3  scaling = localTransform.GetScaling();
			bone.SetScale(FxVec3(static_cast<FxReal>(scaling.GetX()), 
				static_cast<FxReal>(scaling.GetY()), 
				static_cast<FxReal>(scaling.GetZ())));

			// Set the rotation.
			MATH::CQuaternion rotation = localTransform.GetRotationQuaternion();
			bone.SetRot(FxQuat(static_cast<FxReal>(-rotation.GetW()),
				static_cast<FxReal>(-rotation.GetX()),
				static_cast<FxReal>(rotation.GetY()),
				static_cast<FxReal>(rotation.GetZ())));
#endif

			// Found the bone.
			return FxTrue;
		}

		// Sets the specified bone's transform for the current frame.
		// Returns FxTrue if the transform was set or FxFalse otherwise.
		FxBool FxToolXSI::_setBoneTransform( const FxBone& bone )
		{
			// Find the bone animation curve associated with the passed in bone.
			FxXSIAnimCurve* pBoneAnimCurve = m_animController.FindAnimCurve(bone.GetNameAsString());
			if( pBoneAnimCurve )
			{

				// Go to the current frame.
				double currentFrame = XSIGetCurrentFrame();

				// Set keys for each translation attribute.
				FxVec3 boneTrans = bone.GetPos();
				MATH::CVector3 xsiTrans;
#ifdef __FX_USE_NATIVE_COORDS__
				xsiTrans.PutX(boneTrans.x);
				xsiTrans.PutY(boneTrans.y);
				xsiTrans.PutZ(boneTrans.z);
#else
				xsiTrans.PutX(-boneTrans.x);
				xsiTrans.PutY(boneTrans.y);
				xsiTrans.PutZ(boneTrans.z);
#endif
				pBoneAnimCurve->m_translateX.AddKey(currentFrame, xsiTrans.GetX());
				pBoneAnimCurve->m_translateY.AddKey(currentFrame, xsiTrans.GetY());
				pBoneAnimCurve->m_translateZ.AddKey(currentFrame, xsiTrans.GetZ());


				FxQuat boneQuat = bone.GetRot();
#ifndef __FX_USE_NATIVE_COORDS__
				boneQuat.x = -boneQuat.x;
#endif

				MATH::CRotation rot(MATH::CQuaternion(-boneQuat.w,boneQuat.x,boneQuat.y,boneQuat.z));
				double rotX,rotY,rotZ;
				rot.GetXYZAngles(rotX,rotY,rotZ);

				// Set keys for each rotation attribute.
				pBoneAnimCurve->m_rotateX.AddKey(currentFrame, XSI::MATH::RadiansToDegrees(rotX));
				pBoneAnimCurve->m_rotateY.AddKey(currentFrame, XSI::MATH::RadiansToDegrees(rotY));
				pBoneAnimCurve->m_rotateZ.AddKey(currentFrame, XSI::MATH::RadiansToDegrees(rotZ));

				// Set keys for each scale attribute.
				FxVec3 boneScale = bone.GetScale();
				pBoneAnimCurve->m_scaleX.AddKey(currentFrame, boneScale.x);
				pBoneAnimCurve->m_scaleY.AddKey(currentFrame, boneScale.y);
				pBoneAnimCurve->m_scaleZ.AddKey(currentFrame, boneScale.z);

				return FxTrue;
			}
			return FxFalse;
		}

		// Initializes the animation controller.
		FxBool FxToolXSI::_initializeAnimController( void )
		{
			if( _actor )
			{
				// Initialize the animation controller.
				FxArray<FxString> bones;
				for( FxSize i = 0; i < _actor->GetMasterBoneList().GetNumRefBones(); ++i )
				{
					bones.PushBack(_actor->GetMasterBoneList().GetRefBone(i).GetNameAsString());
				}

				if( !m_animController.Initialize(bones) )
				{
					FxToolLog("some bones could not be initialized in _initializeAnimController()");
				}
				return FxTrue;
			}
			return FxFalse;
		}

		// Stops any currently playing animation in Maya.
		void FxToolXSI::_stopAnimationPlayback( void )
		{
			Application app;
			CValueArray args;
			CValue retVal;
			app.ExecuteCommand(L"PlaybackStop",args,retVal);
		}

		// Called when the actor has changed so that a MEL script function can be
		// called to alert any MEL scripts that define the "callback" function
		// __FaceFxOnActorChanged().
		void FxToolXSI::_actorChanged( void )
		{
			if(m_UI.IsValid())
			{
				//ReferenceBone

				OC3Ent::Face::FxSize numRefBones = GetNumRefBones();
				CValueArray referenceBoneValues(numRefBones*2);
				for( OC3Ent::Face::FxSize i = 0; i < numRefBones; ++i)
				{
					XSI::CString tmpStr;
					tmpStr.PutAsciiString(GetRefBoneName(i).GetData());
					referenceBoneValues[i*2] = tmpStr;
					referenceBoneValues[i*2+1] = (int)i;

				}

				PPGLayout layout = m_UI.GetPPGLayout();
				PPGItem referenceBone = layout.GetItem(L"referenceBone");
				referenceBone.PutUIItems(referenceBoneValues);

				
				CValueArray boneValues;
				FxFaceGraph::Iterator iter = _actor->GetDecompiledFaceGraph().Begin(FxBonePoseNode::StaticClass());
				for(int currBonePose = 0; iter != _actor->GetDecompiledFaceGraph().End(FxBonePoseNode::StaticClass()); ++iter,++currBonePose)
				{
					FxFaceGraphNode* graphNode = iter.GetNode();
					XSI::CString poseName;
					poseName.PutAsciiString(graphNode->GetNameAsCstr());
					boneValues.Add(CValue(poseName));
					boneValues.Add(CValue(currBonePose));
				}

				PPGItem bonePose = layout.GetItem(L"BonePoses");
				bonePose.PutUIItems(boneValues);


				
				OC3Ent::Face::FxSize numAnimGroups = GetNumAnimGroups();
				CValueArray animationGroupValues(numAnimGroups*2);
				for( OC3Ent::Face::FxSize i = 0; i < numAnimGroups; ++i )
				{
					CString strTemp;
					strTemp.PutAsciiString(GetAnimGroupName(i).GetData());
					animationGroupValues[i*2] = strTemp;
					animationGroupValues[i*2+1] = (int)i;
				}

				PPGItem animationGroup = layout.GetItem(L"AnimationGroup");
				animationGroup.PutUIItems(animationGroupValues);

				CValueArray animationValues;
				if(numAnimGroups)
				{
					CValue animationGroupIndex = m_UI.GetParameterValue(L"animationGroup");
					OC3Ent::Face::FxString animGroupNameStr(GetAnimGroupName((long)animationGroupIndex));
					OC3Ent::Face::FxSize numAnims = GetNumAnims(animGroupNameStr);
					CValueArray animationValues(numAnims*2);
					for( OC3Ent::Face::FxSize i = 0; i < numAnims; ++i )
					{
						CString strTemp;
						strTemp.PutAsciiString(GetAnimName(animGroupNameStr,i).GetData());
						animationValues[i*2] = strTemp;
						animationValues[i*2+1] = (int)i;
					}
				}
				
				PPGItem animations = layout.GetItem(L"Animations");
				animations.PutUIItems(animationValues);
				

			}
		
		}

		// Called when the list of reference bones has changed so that a MEL script
		// function can be called to alert any MEL scripts that define the 
		// "callback" function __FaceFxOnRefBonesChanged().
		void FxToolXSI::_refBonesChanged( void )
		{
			if(m_UI.IsValid())
			{
				OC3Ent::Face::FxSize numRefBones = GetNumRefBones();
				CValueArray referenceBoneValues(numRefBones*2);
				for( OC3Ent::Face::FxSize i = 0; i < numRefBones; ++i)
				{
					XSI::CString tmpStr;
					tmpStr.PutAsciiString(GetRefBoneName(i).GetData());
					referenceBoneValues[i*2] = tmpStr;
					referenceBoneValues[i*2+1] = (int)i;

				}

				PPGLayout layout = m_UI.GetPPGLayout();
				PPGItem referenceBone = layout.GetItem(L"referenceBone");
				referenceBone.PutUIItems(referenceBoneValues);
			}
		}

		// Called when the list of bone poses has changed so that a MEL script
		// function can be called to alert any MEL scripts that define the
		// "callback" function __FaceFxOnBonePosesChanged().
		void FxToolXSI::_bonePosesChanged( void )
		{
			if(m_UI.IsValid())
			{
				CValueArray boneValues;
				FxFaceGraph::Iterator iter = _actor->GetDecompiledFaceGraph().Begin(FxBonePoseNode::StaticClass());
				for(int currBonePose = 0; iter != _actor->GetDecompiledFaceGraph().End(FxBonePoseNode::StaticClass()); ++iter,++currBonePose)
				{
					FxFaceGraphNode* graphNode = iter.GetNode();
					XSI::CString poseName;
					poseName.PutAsciiString(graphNode->GetNameAsCstr());
					boneValues.Add(CValue(poseName));
					boneValues.Add(CValue(currBonePose));
				}

				PPGLayout layout = m_UI.GetPPGLayout();
				PPGItem bonePose = layout.GetItem(L"BonePoses");
				bonePose.PutUIItems(boneValues);

			}
		}

	} // namespace Face

} // namespace OC3Ent
