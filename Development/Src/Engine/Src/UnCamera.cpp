/*=============================================================================
	UnCamera.cpp: Unreal Engine Camera Actor implementation
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineInterpolationClasses.h"
#include "EngineSequenceClasses.h"
#include "EngineAnimClasses.h"

IMPLEMENT_CLASS(ACamera);
IMPLEMENT_CLASS(ACameraActor);
IMPLEMENT_CLASS(ADynamicCameraActor);
IMPLEMENT_CLASS(AAnimatedCamera);
IMPLEMENT_CLASS(UCameraAnim);
IMPLEMENT_CLASS(UCameraAnimInst);

IMPLEMENT_CLASS(UCameraModifier);

/*------------------------------------------------------------------------------
	ACamera
------------------------------------------------------------------------------*/


/**
 * Set a new ViewTarget with optional transition time
 */
void ACamera::SetViewTarget(class AActor* NewTarget, struct FViewTargetTransitionParams TransitionParams)
{
	// Make sure view target is valid
	if( NewTarget == NULL )
	{
		NewTarget = PCOwner;
	}

	// Update current ViewTargets
	CheckViewTarget(ViewTarget);
	if( PendingViewTarget.Target )
	{
		CheckViewTarget(PendingViewTarget);
	}

	// if different then new one, then assign it
	if( NewTarget != ViewTarget.Target )
	{
		// if a transition time is specified, then set pending view target accordingly
		if( TransitionParams.BlendTime > 0 )
		{
			// band-aid fix so that eventEndViewTarget() gets called properly in this case
			if (PendingViewTarget.Target == NULL)
			{
				PendingViewTarget.Target = ViewTarget.Target;
			}

			BlendParams		= TransitionParams;
			BlendTimeToGo	= TransitionParams.BlendTime;
			
			AssignViewTarget(NewTarget, PendingViewTarget, TransitionParams);
			CheckViewTarget(PendingViewTarget);
		}
		else
		{
			// otherwise, assign new viewtarget instantly
			AssignViewTarget(NewTarget, ViewTarget);
			CheckViewTarget(ViewTarget);
			// remove old pending ViewTarget so we don't still try to switch to it
			PendingViewTarget.Target = NULL;
		}
	}
	else
	{
		// we're setting the viewtarget to the viewtarget we were transitioning away from,
		// just abort the transition.
		// @fixme, investigate if we want this case to go through the above code, so AssignViewTarget et al
		// get called
		if (PendingViewTarget.Target != NULL)
		{
			if (!PCOwner->LocalPlayerController() && WorldInfo->NetMode != NM_Client)
			{
				PCOwner->eventClientSetViewTarget(NewTarget, TransitionParams);
			}
		}
		PendingViewTarget.Target = NULL;
	}
}


void ACamera::AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams)
{
	if( !NewTarget || (NewTarget == VT.Target) )
	{
		return;
	}

	AActor* OldViewTarget	= VT.Target;
	VT.Target				= NewTarget;
	// Set aspect ratio with default.
	VT.AspectRatio			= DefaultAspectRatio;

	// Set FOV with default.
	VT.POV.FOV				= DefaultFOV;

	VT.Target->eventBecomeViewTarget(PCOwner);
	
	if( OldViewTarget )
	{
		OldViewTarget->eventEndViewTarget(PCOwner);
	}

	if (!PCOwner->LocalPlayerController() && WorldInfo->NetMode != NM_Client)
	{
		PCOwner->eventClientSetViewTarget(VT.Target, TransitionParams);
	}
}


/** 
 * Make sure ViewTarget is valid 
 */
void ACamera::CheckViewTarget(FTViewTarget& VT)	
{
	if( !VT.Target )
	{
		VT.Target = PCOwner;
	}

	// Update ViewTarget PlayerReplicationInfo (used to follow same player through pawn transitions, etc., when spectating)
	if( VT.Target == PCOwner || (VT.Target->GetAPawn() && (VT.Target == PCOwner->Pawn)) ) 
	{	
		VT.PRI = NULL;
	}
	else if( VT.Target->GetAController() )
	{
		VT.PRI = VT.Target->GetAController()->PlayerReplicationInfo;
	}
	else if( VT.Target->GetAPawn() )
	{
		VT.PRI = VT.Target->GetAPawn()->PlayerReplicationInfo;
	}
	else if( Cast<APlayerReplicationInfo>(VT.Target) )
	{
		VT.PRI = Cast<APlayerReplicationInfo>(VT.Target);
	}
	else
	{
		VT.PRI = NULL;
	}

	if( VT.PRI && !VT.PRI->bDeleteMe )
	{
		if( !VT.Target || VT.Target->bDeleteMe || !VT.Target->GetAPawn() || (VT.Target->GetAPawn()->PlayerReplicationInfo != VT.PRI) )
		{
			VT.Target = NULL;

			// not viewing pawn associated with RealViewTarget, so look for one
			// Assuming on server, so PRI Owner is valid
			if( !VT.PRI->Owner )
			{
				VT.PRI = NULL;
			}
			else
			{
				AController* PRIOwner = VT.PRI->Owner->GetAController();
				if( PRIOwner )
				{
					AActor* PRIViewTarget = PRIOwner->Pawn;
					if( PRIViewTarget && !PRIViewTarget->bDeleteMe )
					{
						AssignViewTarget(PRIViewTarget, VT);
					}
					else
					{
						VT.PRI = NULL;
					}
				}
				else
				{
					VT.PRI = NULL;
				}
			}
		}
	}

	if( !VT.Target || VT.Target->bDeleteMe )
	{
		check(PCOwner);
		if( PCOwner->Pawn && !PCOwner->Pawn->bDeleteMe && !PCOwner->Pawn->bPendingDelete )
		{
			AssignViewTarget(PCOwner->Pawn, VT);
		}
		else
		{
			AssignViewTarget(PCOwner, VT);
		}
	}

	// Keep PlayerController in synch
	PCOwner->ViewTarget		= VT.Target;
	PCOwner->RealViewTarget	= VT.PRI;
}


/** 
 * Returns current ViewTarget 
 */
AActor* ACamera::GetViewTarget()
{
	// if blending to another view target, return this one first
	if( PendingViewTarget.Target )
	{
		CheckViewTarget(PendingViewTarget);
		if( PendingViewTarget.Target )
		{
			return PendingViewTarget.Target;
		}
	}

	CheckViewTarget(ViewTarget);
	return ViewTarget.Target;
}


UBOOL ACamera::PlayerControlled()
{
	return (PCOwner != NULL);
}


void ACamera::ApplyCameraModifiers(FLOAT DeltaTime,FTPOV& OutPOV)
{
	// Loop through each camera modifier
	for( INT ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++ )
	{
		// Apply camera modification and output into DesiredCameraOffset/DesiredCameraRotation
		if( ModifierList(ModifierIdx) != NULL &&
			!ModifierList(ModifierIdx)->IsDisabled() )
		{
			// If ModifyCamera returns true, exit loop
			// Allows high priority things to dictate if they are
			// the last modifier to be applied
			if( ModifierList(ModifierIdx)->ModifyCamera(this, DeltaTime, OutPOV) )
			{
				break;
			}
		}
	}
}

UBOOL UCameraModifier::ModifyCamera(class ACamera* Camera,FLOAT DeltaTime,FTPOV& OutPOV)
{
	return FALSE;
}

UBOOL UCameraModifier::IsDisabled() const
{
	return bDisabled;
}
/*------------------------------------------------------------------------------
	ACameraActor
------------------------------------------------------------------------------*/

/** 
 *	Use to assign the camera static mesh to the CameraActor used in the editor.
 *	Because HiddenGame is true and CollideActors is false, the component should be NULL in-game.
 */
void ACameraActor::Spawned()
{
	Super::Spawned();

	if(MeshComp)
	{
		if( !MeshComp->StaticMesh)
		{
			UStaticMesh* CamMesh = LoadObject<UStaticMesh>(NULL, TEXT("EditorMeshes.MatineeCam_SM"), NULL, LOAD_None, NULL);
			FComponentReattachContext ReattachContext(MeshComp);
			MeshComp->StaticMesh = CamMesh;
		}
	}

	// Sync component with CameraActor frustum settings.
	UpdateDrawFrustum();
}

/** Used to synchronise the DrawFrustumComponent with the CameraActor settings. */
void ACameraActor::UpdateDrawFrustum()
{
	if(DrawFrustum)
	{
		DrawFrustum->FrustumAngle = FOVAngle;
		DrawFrustum->FrustumStartDist = 10.f;
		DrawFrustum->FrustumEndDist = 1000.f;
		DrawFrustum->FrustumAspectRatio = AspectRatio;
	}
}

/** Ensure DrawFrustumComponent is up to date. */
void ACameraActor::UpdateComponentsInternal(UBOOL bCollisionUpdate)
{
	Super::UpdateComponentsInternal(bCollisionUpdate);
	UpdateDrawFrustum();
}

/** Used to push new frustum settings down into preview component when modifying camera through property window. */
void ACameraActor::PostEditChange(UProperty* PropertyThatChanged)
{
	UpdateDrawFrustum();
	Super::PostEditChange(PropertyThatChanged);
}


/*------------------------------------------------------------------------------
	AAnimatedCamera
------------------------------------------------------------------------------*/

/** Returns an available CameraAnimInst, or NULL if no more are available. */
UCameraAnimInst* AAnimatedCamera::AllocCameraAnimInst()
{	
	UCameraAnimInst* FreeAnim = (FreeAnims.Num() > 0) ? FreeAnims.Pop() : NULL;
	if (FreeAnim)
	{
		ActiveAnims.Push(FreeAnim);
		FreeAnim->TransientScaleModifier = 1.f;
		FreeAnim->SourceAnimNode = NULL;

		// make sure any previous anim has been terminated correctly
		check( (FreeAnim->MoveTrack == NULL) && (FreeAnim->MoveInst == NULL) && (FreeAnim->SourceAnimNode == NULL) );
	}

	return FreeAnim;
}

/** Returns an available CameraAnimInst, or NULL if no more are available. */
void AAnimatedCamera::ReleaseCameraAnimInst(UCameraAnimInst* Inst)
{	
	ActiveAnims.RemoveItem(Inst);
	FreeAnims.Push(Inst);
}

/** Returns first existing instance of the specified camera anim, or NULL if none exists. */
UCameraAnimInst* AAnimatedCamera::FindExistingCameraAnimInst(UCameraAnim const* Anim)
{
	INT const NumActiveAnims = ActiveAnims.Num();
	for (INT Idx=0; Idx<NumActiveAnims; Idx++)
	{
		if (ActiveAnims(Idx)->CamAnim == Anim)
		{
			return ActiveAnims(Idx);
		}
	}

	return NULL;
}



/** Applies interpolatable properties from a camera actor to this POV. */
void AAnimatedCamera::ApplyInterpPropertiesFromCameraActor(ACameraActor const* CamActor, FTPOV& OutPOV)
{
	// Note:  All keys are relative, so deltas are computed and added to the camera's properties

	// Position keys are considered to be in camera-local space, so convert these offsets to world space
	FVector LocalOffset = FRotationMatrix(OutPOV.Rotation).TransformNormal( CamActor->Location );		// Local->World

	// Apply world space offsets
	OutPOV.Location += LocalOffset;
	OutPOV.Rotation += CamActor->Rotation.GetNormalized();

	ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();
	OutPOV.FOV += CamActor->FOVAngle - DefaultCamActor->FOVAngle;

	// Note re post-process effects -- these will actually be applied in ModifyPostProcessSettings(), which
	// the engine will call when it is ready for the deltas to be added.
}

void AAnimatedCamera::ApplyCameraModifiersNative(FLOAT DeltaTime, FTPOV& OutPOV)
{
	// clear out the accumulator
	ResetTempCameraActor(AccumulatorCameraActor);

	// apply each camera anim
	for (INT Idx=0; Idx<ActiveAnims.Num(); ++Idx)
	{
		UCameraAnimInst* const AnimInst = ActiveAnims(Idx);

		if (!AnimInst->bFinished)
		{
			// clear out animated camera actor
			ResetTempCameraActor(AnimCameraActor);

			// evaluate the animation at the new time
			AnimInst->AdvanceAnim(DeltaTime, FALSE);

			// Add weighted properties to the accumulator actor
			if (AnimInst->CurrentBlendWeight > 0.f)
			{
				AddScaledInterpProperties(AnimCameraActor, AccumulatorCameraActor, AnimInst->CurrentBlendWeight);
				AccumulatorCameraActor->Location += AnimCameraActor->Location * AnimInst->CurrentBlendWeight;
				AccumulatorCameraActor->Rotation += AnimCameraActor->Rotation.GetNormalized() * AnimInst->CurrentBlendWeight;
			}
		}

		// handle animations that have finished
		if (AnimInst->bFinished && AnimInst->bAutoReleaseWhenFinished)
		{
			ReleaseCameraAnimInst(AnimInst);
			Idx--;		// we removed this from the ActiveAnims array
		}

		// changes to this are good for a single update, so reset this to 1.f after processing
		AnimInst->TransientScaleModifier = 1.f;
	}

	// copy from accumulator to real camera
	ApplyInterpPropertiesFromCameraActor(AccumulatorCameraActor, OutPOV);

	// need to zero this when we are done with it.  playing another animation
	// will calc a new InitialTM for the move track instance based on these values.
	AnimCameraActor->Location = FVector(0.f);
	AnimCameraActor->Rotation = FRotator(0,0,0);
}


/** Play the indicated CameraAnim on this camera.  Returns the CameraAnim instance, which can be stored to manipulate/stop the anim after the fact. */
UCameraAnimInst* AAnimatedCamera::PlayCameraAnim(class UCameraAnim* Anim, FLOAT Rate, FLOAT Scale, FLOAT BlendInTime, FLOAT BlendOutTime, UBOOL bLoop, UBOOL bRandomStartTime, FLOAT Duration, UBOOL bSingleInstance)
{
	if (bSingleInstance)
	{
		// try to update the existing instance with the new data
		UCameraAnimInst* ExistingInst = FindExistingCameraAnimInst(Anim);
		if (ExistingInst)
		{
			ExistingInst->Update(Rate, Scale, BlendInTime, BlendOutTime, Duration);
			return ExistingInst;
		}
	}

	// get a new instance and play it
	UCameraAnimInst* const Inst = AllocCameraAnimInst();
	if (Inst)
	{
		Inst->Play(Anim, AnimCameraActor, Rate, Scale, BlendInTime, BlendOutTime, bLoop, bRandomStartTime, Duration);
		return Inst;
	}

	return NULL;
}

/** Stops all instances of the given CameraAnim from playing. */
void AAnimatedCamera::StopAllCameraAnimsByType(class UCameraAnim* Anim, UBOOL bImmediate)
{
	// find cameraaniminst for this.
	for (INT Idx=0; Idx<ActiveAnims.Num(); ++Idx)
	{
		if (ActiveAnims(Idx)->CamAnim == Anim)
		{
			ActiveAnims(Idx)->Stop(bImmediate);
		}
	}
}

/** Stops all instances of the given CameraAnim from playing. */
void AAnimatedCamera::StopAllCameraAnims(UBOOL bImmediate)
{
	for (INT Idx=0; Idx<ActiveAnims.Num(); ++Idx)
	{
		ActiveAnims(Idx)->Stop(bImmediate);
	}
}

/** Stops the given CameraAnim instances from playing.  The given pointer should be considered invalid after this. */
void AAnimatedCamera::StopCameraAnim(class UCameraAnimInst* AnimInst, UBOOL bImmediate)
{
	if (AnimInst != NULL)
	{
		AnimInst->Stop(bImmediate);
	}
}


/** Zeroes out the specified temporary CameraActor. */
void AAnimatedCamera::ResetTempCameraActor(class ACameraActor* CamActor) const
{
	if (CamActor)
	{
		CamActor->Location = FVector(0.f);
		CamActor->Rotation = FRotator(0,0,0);
		ResetInterpProperties(CamActor);
	}
}

void AAnimatedCamera::ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const
{
	if (ActiveAnims.Num() > 0)
	{
		// We just take the settings from the accumulator camera actor.  If this becomes problematic,
		// we could copy the settings from the the accumulator to a storage variable in AnimatedCamera, 
		// but that doesn't seem necessary at the moment.
		FPostProcessSettings const& AdditiveSettings = AccumulatorCameraActor->CamOverridePostProcess;
		ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();
		if (DefaultCamActor)
		{
			FPostProcessSettings const& DefaultSettings = DefaultCamActor->CamOverridePostProcess;

			// Add all of the "interp" properties.  Didn't try and get cute here, since the struct is 
			// just a struct and doesn't have UProperty info
			PPSettings.Bloom_Scale += AdditiveSettings.Bloom_Scale - DefaultSettings.Bloom_Scale;
			PPSettings.DOF_FalloffExponent += AdditiveSettings.DOF_FalloffExponent - DefaultSettings.DOF_FalloffExponent;
			PPSettings.DOF_BlurKernelSize += AdditiveSettings.DOF_BlurKernelSize - DefaultSettings.DOF_BlurKernelSize;
			PPSettings.DOF_MaxNearBlurAmount += AdditiveSettings.DOF_MaxNearBlurAmount - DefaultSettings.DOF_MaxNearBlurAmount;
			PPSettings.DOF_MaxFarBlurAmount += AdditiveSettings.DOF_MaxFarBlurAmount - DefaultSettings.DOF_MaxFarBlurAmount;
			PPSettings.DOF_FocusInnerRadius += AdditiveSettings.DOF_FocusInnerRadius - DefaultSettings.DOF_FocusInnerRadius;
			PPSettings.DOF_FocusDistance += AdditiveSettings.DOF_FocusDistance - DefaultSettings.DOF_FocusDistance;
			PPSettings.MotionBlur_MaxVelocity += AdditiveSettings.MotionBlur_MaxVelocity - DefaultSettings.MotionBlur_MaxVelocity;
			PPSettings.MotionBlur_Amount += AdditiveSettings.MotionBlur_Amount - DefaultSettings.MotionBlur_Amount;
			PPSettings.MotionBlur_CameraRotationThreshold += AdditiveSettings.MotionBlur_CameraRotationThreshold - DefaultSettings.MotionBlur_CameraRotationThreshold;
			PPSettings.MotionBlur_CameraTranslationThreshold += AdditiveSettings.MotionBlur_CameraTranslationThreshold - DefaultSettings.MotionBlur_CameraTranslationThreshold;
			PPSettings.Scene_Desaturation += AdditiveSettings.Scene_Desaturation - DefaultSettings.Scene_Desaturation;
			PPSettings.Scene_HighLights += AdditiveSettings.Scene_HighLights - DefaultSettings.Scene_HighLights;
			PPSettings.Scene_MidTones += AdditiveSettings.Scene_MidTones - DefaultSettings.Scene_MidTones;
			PPSettings.Scene_Shadows += AdditiveSettings.Scene_Shadows - DefaultSettings.Scene_Shadows;
		}
	}
}

/** 
 * Scales interpolatable properties and adds the result to the dest object.  (DestObj.Prop += SrcObj.Prop * Scale) 
 * Seems generic enough, could be moved to a deeper class.
 */
void AAnimatedCamera::AddScaledInterpProperties(ACameraActor const* SrcCam, ACameraActor* DestCam, FLOAT Scale)
{
	if (SrcCam && DestCam)
	{
		// Note:  initially went with a property-iteration approach, which is more flexible, but settled on this 
		// more brute method for speed and clarity.
		ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();

		DestCam->AspectRatio += (SrcCam->AspectRatio - DefaultCamActor->AspectRatio) * Scale;
		DestCam->FOVAngle += (SrcCam->FOVAngle - DefaultCamActor->FOVAngle) * Scale;
		DestCam->DrawScale += (SrcCam->DrawScale - DefaultCamActor->DrawScale) * Scale;
		DestCam->DrawScale3D += (SrcCam->DrawScale3D - DefaultCamActor->DrawScale3D) * Scale;

		FPostProcessSettings const& SrcPPSettings = SrcCam->CamOverridePostProcess;
		FPostProcessSettings const& DefaultSettings = DefaultCamActor->CamOverridePostProcess;
		FPostProcessSettings& DestPPSettings = DestCam->CamOverridePostProcess;

		DestPPSettings.Bloom_Scale += (SrcPPSettings.Bloom_Scale - DefaultSettings.Bloom_Scale) * Scale;
		DestPPSettings.DOF_FalloffExponent += (SrcPPSettings.DOF_FalloffExponent - DefaultSettings.DOF_FalloffExponent) * Scale;
		DestPPSettings.DOF_BlurKernelSize += (SrcPPSettings.DOF_BlurKernelSize - DefaultSettings.DOF_BlurKernelSize) * Scale;
		DestPPSettings.DOF_MaxNearBlurAmount += (SrcPPSettings.DOF_MaxNearBlurAmount - DefaultSettings.DOF_MaxNearBlurAmount) * Scale;
		DestPPSettings.DOF_MaxFarBlurAmount += (SrcPPSettings.DOF_MaxFarBlurAmount - DefaultSettings.DOF_MaxFarBlurAmount) * Scale;
		DestPPSettings.DOF_FocusInnerRadius += (SrcPPSettings.DOF_FocusInnerRadius - DefaultSettings.DOF_FocusInnerRadius) * Scale;
		DestPPSettings.DOF_FocusDistance += (SrcPPSettings.DOF_FocusDistance - DefaultSettings.DOF_FocusDistance) * Scale;
		DestPPSettings.MotionBlur_MaxVelocity += (SrcPPSettings.MotionBlur_MaxVelocity - DefaultSettings.MotionBlur_MaxVelocity) * Scale;
		DestPPSettings.MotionBlur_Amount += (SrcPPSettings.MotionBlur_Amount - DefaultSettings.MotionBlur_Amount) * Scale;
		DestPPSettings.MotionBlur_CameraRotationThreshold += (SrcPPSettings.MotionBlur_CameraRotationThreshold - DefaultSettings.MotionBlur_CameraRotationThreshold) * Scale;
		DestPPSettings.MotionBlur_CameraTranslationThreshold += (SrcPPSettings.MotionBlur_CameraTranslationThreshold - DefaultSettings.MotionBlur_CameraTranslationThreshold) * Scale;
		DestPPSettings.Scene_Desaturation += (SrcPPSettings.Scene_Desaturation - DefaultSettings.Scene_Desaturation) * Scale;
		DestPPSettings.Scene_HighLights += (SrcPPSettings.Scene_HighLights - DefaultSettings.Scene_HighLights) * Scale;
		DestPPSettings.Scene_MidTones += (SrcPPSettings.Scene_MidTones - DefaultSettings.Scene_MidTones) * Scale;
		DestPPSettings.Scene_Shadows += (SrcPPSettings.Scene_Shadows - DefaultSettings.Scene_Shadows) * Scale;
	}
}


/** Resets all float, vector, and color interpolatable properties for the given object. */
void AAnimatedCamera::ResetInterpProperties(ACameraActor *CamActor)
{
	if (CamActor)
	{
		ACameraActor const* const DefaultCamActor = ACameraActor::StaticClass()->GetDefaultObject<ACameraActor>();

		CamActor->AspectRatio = DefaultCamActor->AspectRatio;
		CamActor->FOVAngle = DefaultCamActor->FOVAngle;
		CamActor->DrawScale = DefaultCamActor->DrawScale;
		CamActor->DrawScale3D = DefaultCamActor->DrawScale3D;

		FPostProcessSettings const& DefaultPPSettings = DefaultCamActor->CamOverridePostProcess;
		FPostProcessSettings& PPSettings = CamActor->CamOverridePostProcess;

		PPSettings.Bloom_Scale = DefaultPPSettings.Bloom_Scale;
		PPSettings.DOF_FalloffExponent = DefaultPPSettings.DOF_FalloffExponent;
		PPSettings.DOF_BlurKernelSize = DefaultPPSettings.DOF_BlurKernelSize;
		PPSettings.DOF_MaxNearBlurAmount = DefaultPPSettings.DOF_MaxNearBlurAmount;
		PPSettings.DOF_MaxFarBlurAmount = DefaultPPSettings.DOF_MaxFarBlurAmount;
		PPSettings.DOF_FocusInnerRadius = DefaultPPSettings.DOF_FocusInnerRadius;
		PPSettings.DOF_FocusDistance = DefaultPPSettings.DOF_FocusDistance;
		PPSettings.MotionBlur_MaxVelocity = DefaultPPSettings.MotionBlur_MaxVelocity;
		PPSettings.MotionBlur_Amount = DefaultPPSettings.MotionBlur_Amount;
		PPSettings.MotionBlur_CameraRotationThreshold = DefaultPPSettings.MotionBlur_CameraRotationThreshold;
		PPSettings.MotionBlur_CameraTranslationThreshold = DefaultPPSettings.MotionBlur_CameraTranslationThreshold;
		PPSettings.Scene_Desaturation = DefaultPPSettings.Scene_Desaturation;
		PPSettings.Scene_HighLights =  DefaultPPSettings.Scene_HighLights;
		PPSettings.Scene_MidTones =  DefaultPPSettings.Scene_MidTones;
		PPSettings.Scene_Shadows =  DefaultPPSettings.Scene_Shadows;
	}
}

void AAnimatedCamera::ApplyCameraModifiers(FLOAT DeltaTime,FTPOV& OutPOV)
{
	Super::ApplyCameraModifiers(DeltaTime, OutPOV);

	// will handle this in native code
	ApplyCameraModifiersNative(DeltaTime, OutPOV);
}
/*------------------------------------------------------------------------------
	UCameraAnimInst
------------------------------------------------------------------------------*/

/** Advances animation by DeltaTime.  Changes are applied to the group's actor. */
void UCameraAnimInst::AdvanceAnim(FLOAT DeltaTime, UBOOL bJump)
{
	// check to see if our animnodeseq has been deleted.  not a fan of 
	// polling for this, but we want to stop this immediately, not when
	// GC gets around to cleaning up.
	if (SourceAnimNode)
	{
		if ( (SourceAnimNode->SkelComponent == NULL) || SourceAnimNode->SkelComponent->IsPendingKill() )
		{
			SourceAnimNode = NULL;		// clear this ref so GC can release the node
			Stop(TRUE);
		}
	}


	if ( (CamAnim != NULL) && !bFinished )
	{
		// will set to true if anim finishes this frame
		UBOOL bAnimJustFinished = FALSE;

		FLOAT const ScaledDeltaTime = DeltaTime * PlayRate;

		// find new times
		CurTime += ScaledDeltaTime;
		if (bBlendingIn)
		{
			CurBlendInTime += DeltaTime;
		}
		if (bBlendingOut)
		{
			CurBlendOutTime += DeltaTime;
		}

		// see if we've crossed any important time thresholds and deal appropriately
		if (bLooping)
		{
			if (CurTime > CamAnim->AnimLength)
			{
				// loop back to the beginning
				CurTime -= CamAnim->AnimLength;
			}
		}
		else
		{
			if (CurTime > CamAnim->AnimLength)
			{
				// done!!
				bAnimJustFinished = TRUE;
			}
			else if (CurTime > (CamAnim->AnimLength - BlendOutTime))
			{
				// start blending out
				bBlendingOut = TRUE;
				CurBlendOutTime = CurTime - (CamAnim->AnimLength - BlendOutTime);
			}
		}

		if (bBlendingIn)
		{
			if (CurBlendInTime > BlendInTime)
			{
				// done blending in!
				bBlendingIn = FALSE;
			}
		}
		if (bBlendingOut)
		{
			if (CurBlendOutTime > BlendOutTime)
			{
				// done!!
				CurBlendOutTime = BlendOutTime;
				bAnimJustFinished = TRUE;
			}
		}
		
		// calculate blend weight. calculating separately and taking the minimum handles overlapping blends nicely.
		{
			FLOAT BlendInWeight = (bBlendingIn) ? (CurBlendInTime / BlendInTime) : 1.f;
			FLOAT BlendOutWeight = (bBlendingOut) ? (1.f - CurBlendOutTime / BlendOutTime) : 1.f;
			CurrentBlendWeight = ::Min(BlendInWeight, BlendOutWeight) * BasePlayScale * TransientScaleModifier;
		}

		// this will update tracks and apply the effects to the group actor (except move tracks)
		InterpGroupInst->Group->UpdateGroup(CurTime, InterpGroupInst, FALSE, bJump);

		// UpdateGroup won't handle the movement track, need to deal with it separately.
		AActor* const GroupActor = InterpGroupInst->GetGroupActor();
		if (GroupActor != NULL && MoveTrack != NULL && MoveInst != NULL)
		{
			GroupActor->MoveWithInterpMoveTrack(MoveTrack, MoveInst, CurTime, DeltaTime);
		}

		if (bAnimJustFinished)
		{
			// completely finished
			Stop(TRUE);
		}
		else if (RemainingTime > 0.f)
		{
			// handle any specified duration
			RemainingTime -= DeltaTime;
			if (RemainingTime <= 0.f)
			{
				// stop with blend out
				Stop();
			}
		}
	}
}

/** Updates this active instance with new parameters. */
void UCameraAnimInst::Update(FLOAT NewRate, FLOAT NewScale, FLOAT NewBlendInTime, FLOAT NewBlendOutTime, FLOAT NewDuration)
{
	if (bBlendingOut)
	{
		bBlendingOut = FALSE;
		CurBlendOutTime = 0.f;

		// stop any blendout and reverse it to a blendin
		bBlendingIn = TRUE;
		CurBlendInTime = NewBlendInTime * (1.f - CurBlendOutTime / BlendOutTime);
	}

	PlayRate = NewRate;
	BasePlayScale = NewScale;
	BlendInTime = NewBlendInTime;
	BlendOutTime = NewBlendOutTime;
	RemainingTime = (NewDuration > 0.f) ? (NewDuration - BlendOutTime) : 0.f;
	bFinished = FALSE;
}


/** Starts this instance playing the specified CameraAnim. */
void UCameraAnimInst::Play(class UCameraAnim* Anim, class AActor* CamActor, FLOAT InRate, FLOAT InScale, FLOAT InBlendInTime, FLOAT InBlendOutTime, UBOOL bInLooping, UBOOL bRandomStartTime, FLOAT Duration)
{
	if (Anim && Anim->CameraInterpGroup)
	{
		// make sure any previous anim has been terminated correctly
		Stop(TRUE);

		CurTime = bRandomStartTime ? (appFrand() * Anim->AnimLength) : 0.f;
		CurBlendInTime = 0.f;
		CurBlendOutTime = 0.f;
		bBlendingIn = TRUE;
		bBlendingOut = FALSE;
		bFinished = FALSE;

		// copy properties
		CamAnim = Anim;
		PlayRate = InRate;
		BasePlayScale = InScale;
		BlendInTime = InBlendInTime;
		BlendOutTime = InBlendOutTime;
		bLooping = bInLooping;
		RemainingTime = (Duration > 0.f) ? (Duration - BlendOutTime) : 0.f;

		// init the interpgroup
		InterpGroupInst->InitGroupInst(CamAnim->CameraInterpGroup, CamActor);

		// cache move track refs
		for (INT Idx = 0; Idx < InterpGroupInst->TrackInst.Num(); ++Idx)
		{
			MoveTrack = Cast<UInterpTrackMove>(CamAnim->CameraInterpGroup->InterpTracks(Idx));
			if (MoveTrack != NULL)
			{
				MoveInst = CastChecked<UInterpTrackInstMove>(InterpGroupInst->TrackInst(Idx));
				// only 1 move track per group, so we can bail here
				break;					
			}
		}	
	}
}

/** Stops this instance playing whatever animation it is playing. */
void UCameraAnimInst::Stop(UBOOL bImmediate)
{
	if ( bImmediate || (BlendOutTime <= 0.f) )
	{
		if (InterpGroupInst->Group != NULL)
		{
			InterpGroupInst->TermGroupInst(TRUE);
		}
		MoveTrack = NULL;
		MoveInst = NULL;
		bFinished = TRUE;
		SourceAnimNode = NULL;
 	}
	else
	{
		// start blend out
		bBlendingOut = TRUE;
		CurBlendOutTime = 0.f;
	}
}

void UCameraAnimInst::ApplyTransientScaling(FLOAT Scalar)
{
	TransientScaleModifier *= Scalar;
}

void UCameraAnimInst::RegisterAnimNode(class UAnimNodeSequence* AnimNode)
{
	SourceAnimNode = AnimNode;
}



/*------------------------------------------------------------------------------
	UCameraAnim
------------------------------------------------------------------------------*/

/** 
 * Construct a camera animation from an InterpGroup.  The InterpGroup must control a CameraActor.  
 * Used by the editor to "export" a camera animation from a normal Matinee scene.
 */
UBOOL UCameraAnim::CreateFromInterpGroup(class UInterpGroup* SrcGroup, class USeqAct_Interp* Interp)
{
	// assert we're controlling a camera actor
#if !FINAL_RELEASE
	{
		UInterpGroupInst* GroupInst = Interp->FindFirstGroupInst(SrcGroup);
		if (GroupInst)
		{
			check( GroupInst->GetGroupActor()->IsA(ACameraActor::StaticClass()) );
		}
	}
#endif
	
	// copy length information
	AnimLength = (Interp && Interp->InterpData) ? Interp->InterpData->InterpLength : 0.f;

	UInterpGroup* OldGroup = CameraInterpGroup;

	if (CameraInterpGroup != SrcGroup)
	{
		// copy the source interp group for use in the CameraAnim
		CameraInterpGroup = (UInterpGroup*)UObject::StaticDuplicateObject(SrcGroup, SrcGroup, this, TEXT("None"));

		if (CameraInterpGroup)
		{
			// delete the old one, if it exists
			if (OldGroup)
			{
				OldGroup->MarkPendingKill();
			}

			// success!
			return TRUE;
		}
		else
		{
			// creation of new one failed somehow, restore the old one
			CameraInterpGroup = OldGroup;
		}
	}
	else
	{
		// no need to perform work above, but still a "success" case
		return TRUE;
	}

	// failed creation
	return FALSE;
}


FBox UCameraAnim::GetAABB(FVector const& BaseLoc, FRotator const& BaseRot, FLOAT Scale) const
{
	FRotationTranslationMatrix const BaseTM(BaseRot, BaseLoc);

	FBox ScaledLocalBox = BoundingBox;
	ScaledLocalBox.Min *= Scale;
	ScaledLocalBox.Max *= Scale;

	return ScaledLocalBox.TransformBy(BaseTM);
}


void UCameraAnim::PreSave()
{
	CalcLocalAABB();
	Super::PreSave();
}

void UCameraAnim::PostLoad()
{
	if (GIsEditor)
	{
		// update existing CameraAnims' bboxes on load, so editor knows they 
		// they need to be resaved
		if (!BoundingBox.IsValid)
		{
			CalcLocalAABB();
			if (BoundingBox.IsValid)
			{
				MarkPackageDirty();
			}
		}
	}

	Super::PostLoad();
}	


void UCameraAnim::CalcLocalAABB()
{
	BoundingBox.Init();

	if (CameraInterpGroup)
	{
		// find move track
		UInterpTrackMove *MoveTrack = NULL;
		for (INT TrackIdx = 0; TrackIdx < CameraInterpGroup->InterpTracks.Num(); ++TrackIdx)
		{
			MoveTrack = Cast<UInterpTrackMove>(CameraInterpGroup->InterpTracks(TrackIdx));
			if (MoveTrack != NULL)
			{
				break;
			}
		}

		if (MoveTrack != NULL)
		{
			FVector Zero(0.f), MinBounds, MaxBounds;
			MoveTrack->PosTrack.CalcBounds(MinBounds, MaxBounds, Zero);
			BoundingBox = FBox(MinBounds, MaxBounds);
		}
	}
}



