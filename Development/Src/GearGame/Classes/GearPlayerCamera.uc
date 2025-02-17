/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPlayerCamera extends AnimatedCamera
	config(Camera)
	native(Camera);

// NOTE FOR REFERENCE
// >> IS LOCAL->WORLD (no transpose)
// << IS WORLD->LOCAL (has the transpose)

/** GoW global macros */


/////////////////////
// Camera Modes
/////////////////////

enum EGearPlayerCameraType
{
	GCT_None,
	GCT_GamePlay,
	GCT_Spectating,
	GCT_DebugFaceCam,
	GCT_Fixed,
	GCT_Screenshot,
};
/** Currently selected camera type. */
var transient protected EGearPlayerCameraType		CamType;

/** Implements normal gameplay camera. */
var() editinline transient GearGameplayCamera		GameplayCam;
/** Implements spectator camera. */
var() editinline transient GearSpectatorCamera		SpectatorCam;
/** Implements the debug face-cam, used for previewing FaceFX stuff. */
var() editinline transient GearDebugFaceCamera		DebugFaceCam;
/** Implements the fixed, used for viewing through pre-placed camera actors. */
var() editinline transient GearFixedCamera			FixedCam;
/** Camera for screen shots */
var() editinline transient GearScreenshotCamera		ScreenshotCam;

/** Which camera is currently active. */
var transient GearCameraBase	CurrentCamera;




/////////////////////
// Camera Modifiers
/////////////////////

/** Camera modifier for camera bone animation */
var(Camera) private editinline	GearCameraModifier	GearCamMod_CameraBone;
/** Camera modifier for screen shakes */
var(Camera) editinline	GearCamMod_ScreenShake		GearCamMod_ScreenShake;


/////////////////////
// FOV Overriding
/////////////////////

/** Should the FOV be overridden? */
var transient bool		bUseForcedCamFOV;
/** If bUseForcedCamFOV is true, use this angle */
var transient float		ForcedCamFOV;



/////////////////////
// "Lens" effects
/////////////////////

/** CameraBlood emitter attached to this camera */
var protected transient array<Emit_CameraLensEffectBase> CameraLensEffects;


/////////////////////
// Interpolation
/////////////////////

var transient bool						bInterpolateCamChanges;

var transient private Actor						LastViewTarget;
var transient private EGearPlayerCameraType		LastCameraType;

/** Indicates if we should reset interpolation on whichever active camera processes next. */
var transient private bool				bResetInterp;


/////////////////////
// Camera Shakes
/////////////////////

/** Scalar applied to all screen shakes in splitscreen. Normally used to dampen, since shakes feel more intense in a smaller viewport. */
var() protected const float SplitScreenShakeScale;


/////////////////////
// Shaky-cam management
/////////////////////

/** The pawn that was last used to cache the animnodes used for shakycam.  Changing viewtarget pawns will trigger a re-cache. */
var private transient Pawn						ShakyCamAnimNodeCachePawn;
/** AnimNode names for the "standing idle" camera animation. */
var() private const array<name>					StandingIdleSequenceNodeNames;
/** Cached refs to the standing idle animations. */
var private transient array<AnimNodeSequence>	StandingIdleSequenceNodes;


/////////////////////
// Etc
/////////////////////

// dealing with situations where camera target is based on another actor
var transient protected Actor LastTargetBase;
var transient protected matrix LastTargetBaseTM;


cpptext
{
	virtual void ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const;
};


private function GearCameraBase CreateGearCamera(class<GearCameraBase> CameraClass)
{
	local GearCameraBase NewCam;
	NewCam = new(Outer) CameraClass;
	NewCam.PlayerCamera = self;
	NewCam.Init();
	return NewCam;
}

private function GearCameraModifier CreateCameraModifier(class<GearCameraModifier> ModifierClass)
{
	local GearCameraModifier NewMod;
	NewMod = new(Outer) ModifierClass;
	NewMod.Init();
	NewMod.AddCameraModifier( Self );
	return NewMod;
}


private function CacheShakyCamAnimNodes(Pawn TargetPawn)
{
	local SkeletalMeshComponent SkelComp;
	local name TmpName;
	local int NameIdx;

	// cache anim nodes used for shakycam management
//	RoadieRunSequenceNode = SkelComp.FindAnimNode(RoadieRunSequenceNodeName);

	SkelComp = TargetPawn.Mesh;

	StandingIdleSequenceNodes.Length = StandingIdleSequenceNodeNames.Length;
	foreach StandingIdleSequenceNodeNames(TmpName, NameIdx)
	{
		StandingIdleSequenceNodes[NameIdx] = AnimNodeSequence(SkelComp.FindAnimNode(TmpName));
	}

	ShakyCamAnimNodeCachePawn = TargetPawn;
}

function PostBeginPlay()
{
	super.PostBeginPlay();

	// Setup camera modes
	if (GameplayCam == None)
	{
		GameplayCam = GearGameplayCamera(CreateGearCamera(class'GearGameplayCamera'));
	}
	if (SpectatorCam == None)
	{
		SpectatorCam = GearSpectatorCamera(CreateGearCamera(class'GearSpectatorCamera'));
	}
	if (DebugFaceCam == None)
	{
		DebugFaceCam = GearDebugFaceCamera(CreateGearCamera(class'GearDebugFaceCamera'));
	}
	if (FixedCam == None)
	{
		FixedCam = GearFixedCamera(CreateGearCamera(class'GearFixedCamera'));
	}

	// Setup camera modifiers
	if( GearCamMod_ScreenShake == None )
	{
		GearCamMod_ScreenShake = GearCamMod_ScreenShake(CreateCameraModifier(class'GearCamMod_ScreenShake'));
	}
	if( GearCamMod_CameraBone == None )
	{
		GearCamMod_CameraBone = GearCamMod_CameraBone(CreateCameraModifier(class'GearCamMod_CameraBone'));
	}
}

// reset the camera to a good state
function Reset()
{
	bUseForcedCamFOV = false;
}

/**
 * Camera Shake
 * Plays camera shake effect
 *
 * @param	Duration			Duration in seconds of shake
 * @param	newRotAmplitude		view rotation amplitude (pitch,yaw,roll)
 * @param	newRotFrequency		frequency of rotation shake
 * @param	newLocAmplitude		relative view offset amplitude (x,y,z)
 * @param	newLocFrequency		frequency of view offset shake
 * @param	newFOVAmplitude		fov shake amplitude
 * @param	newFOVFrequency		fov shake frequency
 */
final function CameraShake
(
	float	Duration,
	vector	newRotAmplitude,
	vector	newRotFrequency,
	vector	newLocAmplitude,
	vector	newLocFrequency,
	float	newFOVAmplitude,
	float	newFOVFrequency
)
{
	GearCamMod_ScreenShake.StartNewShake( Duration, newRotAmplitude, newRotFrequency, newLocAmplitude, newLocFrequency, newFOVAmplitude, newFOVFrequency );
}


/**
 * Play a camera shake
 */
final function PlayCameraShake(const out ScreenShakeStruct ScreenShake)
{
	GearCamMod_ScreenShake.AddScreenShake( ScreenShake );
}

function SetCameraType(EGearPlayerCameraType NewType)
{
	CamType = NewType;
}

private function EGearPlayerCameraType FindBestCameraType(Actor CameraTarget)
{
	local EGearPlayerCameraType BestType;
	local GearPawn TargetGP;

	TargetGP = GearPawn(CameraTarget);

	if (CameraStyle == 'screenshot')
	{
		BestType = GCT_Screenshot;
	}
	else
	// @fixme, is this too inflexible?
	if (CameraStyle != 'default')
	{
		// not using a Gears camera, we'll let the engine handle it
		BestType = GCT_None;
	}
	else if (GearPC(PCOwner).bDebugFaceCam)
	{
		BestType = GCT_DebugFaceCam;
	}
	else if ( (PCOwner.IsSpectating() && TargetGP == None && Vehicle(CameraTarget) == None )	||		// spectating something other than a pawn
			  (GearSpectatorPoint(CameraTarget) != None) )				// looking through a spectator point
	{
		// if spectating or attached to a SpectatorPoint, use Spectator
		BestType = GCT_Spectating;
	}
	else if (CameraActor(CameraTarget) != None)
	{
		// if attached to a CameraActor and not spectating, use Fixed
		BestType = GCT_Fixed;
	}
	else if (Pawn(CameraTarget) != None)
	{
		// looking at a pawn.  Note this could be a vehicle or turret, which aren't GearPawns.
		if ( (TargetGP != None) && (TargetGP.CameraVolumes.length > 0) )
		{
			// in a camera volume, use the fixed camera there
			BestType = GCT_Fixed;
		}
		else
		{
			BestType = GCT_Gameplay;
		}
	}

	return BestType;
}


/**
 * Query ViewTarget and outputs Point Of View.
 *
 * @param	OutVT		ViewTarget to use.
 * @param	DeltaTime	Delta Time since last camera update (in seconds).
 */
function UpdateViewTarget(out TViewTarget OutVT, float DeltaTime)
{
	local Pawn P;
	local EGearPlayerCameraType NewCameraType;
	local GearCameraBase		NewCamera;
	local int					Idx;
    local CameraActor CamActor;


	//local float Time;
	//CLOCK_CYCLES(Time);

	// Make sure we have a valid target
	if( OutVT.Target == None )
	{
		`log("Camera::UpdateViewTarget OutVT.Target == None");
		return;
	}

	P = Pawn(OutVT.Target);

	NewCameraType = FindBestCameraType(OutVT.Target);

	if (NewCameraType != CamType)
	{
		switch (NewCameraType)
		{
		case GCT_Gameplay:
			NewCamera = GameplayCam;
			break;
		case GCT_Spectating:
			NewCamera = SpectatorCam;
			break;
		case GCT_Fixed:
			NewCamera = FixedCam;
			break;
		case GCT_DebugFaceCam:
			NewCamera = DebugFaceCam;
			break;
		case GCT_Screenshot:
			NewCamera = ScreenshotCam;
			break;
		case GCT_None:
		default:
			NewCamera = None;
		}

		if (CurrentCamera != NewCamera)
		{
			if (CurrentCamera != None)
			{
				CurrentCamera.OnBecomeInActive();
			}
			if (NewCamera != None)
			{
				NewCamera.OnBecomeActive();
			}
		}

		CamType = NewCameraType;
		CurrentCamera = NewCamera;
	}

	// update current camera
	if (CurrentCamera != None)
	{
		// we wait to apply this here in case the above code changed currentcamera on us
		if (bResetInterp && !bInterpolateCamChanges)
		{
			CurrentCamera.ResetInterpolation();
		}

		// Make sure overridden post process settings have a chance to get applied
		CamActor = CameraActor(OutVT.Target);
		if( CamActor != None )
		{
		    CamActor.GetCameraView(DeltaTime, OutVT.POV);

			// Check to see if we should be constraining the viewport aspect.  We'll only allow aspect
			// ratio constraints for fixed cameras (non-spectator)
			if( CamType == GCT_Fixed && CamActor.bConstrainAspectRatio )
			{
				// Grab aspect ratio from the CameraActor
				bConstrainAspectRatio = true;
				OutVT.AspectRatio = CamActor.AspectRatio;
			}

			// See if the CameraActor wants to override the PostProcess settings used.
			bCamOverridePostProcess = CamActor.bCamOverridePostProcess;
			if( bCamOverridePostProcess )
			{
				CamPostProcessSettings = CamActor.CamOverridePostProcess;
			}
		}

		CurrentCamera.UpdateCamera(P, DeltaTime, OutVT);
	}
	else
	{
		//`log("got here, camerastyle"@CameraStyle@P);
		super.UpdateViewTarget(OutVT, DeltaTime);
	}

	// check for forced fov
	if (bUseForcedCamFOV)
	{
		OutVT.POV.FOV = ForcedCamFOV;
	}

	// adjust FOV for splitscreen, 4:3, whatever
	OutVT.POV.FOV = AdjustFOVForViewport(OutVT.POV.FOV, P);

	// set camera's loc and rot, to handle cases where we are not locked to view target
	SetRotation(OutVT.POV.Rotation);
	SetLocation(OutVT.POV.Location);

	// update any attached camera lens effects (e.g. blood)
	for (Idx=0; Idx<CameraLensEffects.length; ++Idx)
	{
		if (CameraLensEffects[Idx] != None)
		{
			CameraLensEffects[Idx].UpdateLocation(OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV);
		}
	}

	// update any shakycam weights
	if (P != None)
	{
		UpdateShakyCam(P);
	}

	// store
	CacheLastTargetBaseInfo(OutVT.Target.Base);

	bResetInterp = FALSE;

	//UNCLOCK_CYCLES(Time);
	//`log(GetFuncName()@"took"@Time);
}

simulated native function CacheLastTargetBaseInfo(Actor TargetBase);


simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas	Canvas;

	Super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255,255,255);

	Canvas.DrawText("	GameplayCam CameraOrigin:" @ GameplayCam.LastActualCameraOrigin @ "LastViewOffset:" @ GameplayCam.LastViewOffset );
	out_YPos += out_YL;
	Canvas.SetPos(4,out_YPos);
}

function UpdateShakyCam(Pawn TargetPawn)
{
	local GearPawn TargetGP;
	local GearPC GPC;
	local float StandingIdleWeight;
	local bool bInCombat;
	local AnimNodeSequence TmpAnimNode;

	TargetGP = GearPawn(TargetPawn);
	if (TargetGP == None)
	{
		return;
	}
	GPC = GearPC(TargetGP.Controller);			// ok to be null


	// make sure everything is cached properly
	// handles target changes, which should be rare
	if (ShakyCamAnimNodeCachePawn != TargetGP)
	{
		CacheShakyCamAnimNodes(TargetGP);
	}

	// don't do anything to roadie run, actually

	// apply an increased amount of camera bone motion if we've been recently shot at
	bInCombat = ( TargetGP.WorldInfo.TimeSeconds - TargetGP.LastShotAtTime < 5.0f ) || ( GPC != None && GPC.IsInCombat() );

	// maybe scale back the idle camera anim
	if (!bInCombat && IsZero(TargetGP.Velocity) && (GPC != None) && Abs(GPC.RemappedJoyRight) < GPC.DeadZoneThreshold && Abs(GPC.RemappedJoyUp) < GPC.DeadZoneThreshold)
	{
		// negate the idle shake entirely
		StandingIdleWeight = 0.f;
	}
	else if (TargetGP.IsInAimingPose())
	{
		// steadier when aiming
		// @fixme, this will eventually be set in the AnimTree, and then this can be removed
		StandingIdleWeight = 0.5f;
	}
	else
	{
		StandingIdleWeight = 1.f;
	}

	// @fixme HACK disabled for now, until we can get these working better
	StandingIdleWeight = 0.f;

	// apply scaling to the nodes in question.
	foreach StandingIdleSequenceNodes(TmpAnimNode)
	{
		if (TmpAnimNode.ActiveCameraAnimInstance != None)
		{
			TmpAnimNode.ActiveCameraAnimInstance.ApplyTransientScaling(StandingIdleWeight);
		}
	}

}


/** Finds the first instance of a lens effect of the given class, using linear search. */
function Emit_CameraLensEffectBase FindCameraLensEffect(class<Emit_CameraLensEffectBase> LensEffectEmitterClass)
{
	local Emit_CameraLensEffectBase		LensEffect;

	foreach CameraLensEffects(LensEffect)
	{
		if ( (LensEffect.Class == LensEffectEmitterClass) && !LensEffect.bDeleteMe )
		{
			return LensEffect;
		}
	}

	return None;
}

function AddCameraLensEffect(class<Emit_CameraLensEffectBase> LensEffectEmitterClass)
{
	local vector CamLoc;
	local rotator CamRot;
	local Emit_CameraLensEffectBase LensEffect;

	if (LensEffectEmitterClass != None)
	{
		if (!LensEffectEmitterClass.default.bAllowMultipleInstances)
		{
			LensEffect = FindCameraLensEffect(LensEffectEmitterClass);

			if (LensEffect != None)
			{
				LensEffect.NotifyRetriggered();
			}
		}

		if (LensEffect == None)
		{
			// spawn with viewtarget as the owner so bOnlyOwnerSee works as intended
			LensEffect = Spawn( LensEffectEmitterClass, PCOwner.GetViewTarget() );
			if (LensEffect != None)
			{
				GetCameraViewPoint(CamLoc, CamRot);
				LensEffect.UpdateLocation(CamLoc, CamRot, GetFOVAngle());
				LensEffect.RegisterCamera(self);

				CameraLensEffects.AddItem(LensEffect);
			}
		}
	}
}

/** Removes this particular lens effect from the camera. */
function RemoveCameraLensEffect(Emit_CameraLensEffectBase Emitter)
{
	CameraLensEffects.RemoveItem(Emitter);
}

/** Removes all Camera Lens Effects. */
function ClearCameraLensEffects()
{
	local Emit_CameraLensEffectBase		LensEffect;

	foreach CameraLensEffects(LensEffect)
	{
		LensEffect.Destroy();
	}

	// empty the array.  unnecessary, since destruction will call RemoveCameraLensEffect,
	// but this gets it done in one fell swoop.
	CameraLensEffects.length = 0;
}

/** Add and remove a pawn from the hidden actor's array */
final native function AddGearPawnToHiddenActorsArray( Pawn PawnToHide );
final native function RemoveGearPawnFromHiddenActorsArray( Pawn PawnToShow );

/**
* Sets the new color scale
*/
simulated function SetColorScale( vector NewColorScale )
{
	if( bEnableColorScaling == TRUE )
	{
		// set the default color scale
		bEnableColorScaling = TRUE;
		ColorScale = NewColorScale;
		bEnableColorScaleInterp = false;
	}
}

/** Stop interpolation for this frame and just let everything go to where it's supposed to be. */
simulated function ResetInterpolation()
{
	bResetInterp = TRUE;
}


/**
* Give cameras a chance to change player view rotation
*/
function ProcessViewRotation(float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot)
{
	if( CurrentCamera != None )
	{
		CurrentCamera.ProcessViewRotation(DeltaTime, ViewTarget.Target, out_ViewRotation, out_DeltaRot);
	}
}

/**
* Returns desired camera fov.
*/
//final simulated native function float GetGearCamFOV(Pawn CameraTargetPawn);

/**
* Given a horizontal FOV that assumes a 16:9 viewport, return an appropriately
* adjusted FOV for the viewport of the target pawn.
* Used to correct for splitscreen.
*/
final simulated native function float AdjustFOVForViewport(float inHorizFOV, Pawn CameraTargetPawn);


/** Plays radial code-driven camera shake for THIS camera. */
simulated function PlayRadialCameraShake(const out ScreenShakeStruct Shake, vector Epicenter, float InnerRadius, float OuterRadius, float Falloff, bool bTryForceFeedback)
{
	local ScreenShakeStruct	ScaledShake;
	local Vector			POVLoc;
	local float				DistPct, ShakeScale;
	local GearPC			GPC;

	// @fixme, should use camera's loc, so things like spectators work?
	GPC = GearPC(PCOwner);
	POVLoc = (GPC.Pawn != None) ? GPC.Pawn.Location : GPC.Location;

	DistPct = (VSize(Epicenter - POVLoc) - InnerRadius) / (OuterRadius - InnerRadius);
	DistPct = 1.f - FClamp(DistPct, 0.f, 1.f);
	ShakeScale = DistPct ** Falloff;

// `log("Shake scaled by"@DistPct@ShakeInstigator@VSize(Epicenter - POVLoc));

	if (ShakeScale > 0.f)
	{
		if (class'Engine'.static.IsSplitScreen())
		{
			ShakeScale *= SplitScreenShakeScale;
		}

		ScaledShake = Shake;
		ScaledShake.RotAmplitude *= ShakeScale;
		ScaledShake.LocAmplitude *= ShakeScale;
		ScaledShake.FOVAmplitude *= ShakeScale;

		GPC.ClientPlayCameraShake( ScaledShake, bTryForceFeedback );
	}
}

/** Plays radial anim-driven camera shake for THIS camera. */
simulated function PlayRadialCameraShakeAnim(ScreenShakeAnimStruct Shake, vector Epicenter, float InnerRadius, float OuterRadius, float Falloff, bool bTryForceFeedback)
{
	local Vector	POVLoc;
	local float		ShakeScale, Duration;
	local bool		bRandomStart, bLoop;
	local GearPC	GPC;
	local CameraAnim ShakeAnim;

	// @fixme, should use camera's loc, so things like spectators work?
	GPC = GearPC(PCOwner);
	POVLoc = (GPC.Pawn != None) ? GPC.Pawn.Location : GPC.Location;

	// be careful here: if the first ShakeScale value is large due to having small range of the divisor then the ShakeScale value will be 0 due to 1-FClamp and you will get no shake
	ShakeScale = (VSize(Epicenter - POVLoc) - InnerRadius) / Max((OuterRadius - InnerRadius), 1);
	ShakeScale = 1.f - FClamp(ShakeScale, 0.f, 1.f);
	ShakeScale = Shake.AnimScale * ShakeScale ** Falloff;

	//`log("Shake scaled by"@ShakeScale@VSize(Epicenter - POVLoc)@Shake.AnimScale@FClamp(ShakeScale, 0.f, 1.f));
	if (ShakeScale > 0.f)
	{
		if (class'Engine'.static.IsSplitScreen())
		{
			ShakeScale *= SplitScreenShakeScale;
		}

		if (Shake.bRandomSegment)
		{
			bLoop = TRUE;
			bRandomStart = TRUE;
			Duration = Shake.RandomSegmentDuration;
		}

		ShakeAnim = Shake.bUseDirectionalAnimVariants ? ChooseDirectionalCameraShakeAnim(Shake, Epicenter) : Shake.Anim;

		//`log( Shake.Anim @ Shake.AnimPlayRate @ ShakeScale @ Shake.AnimBlendInTime );
		PlayCameraAnim(ShakeAnim, Shake.AnimPlayRate, ShakeScale, Shake.AnimBlendInTime, Shake.AnimBlendOutTime, bLoop, bRandomStart, Duration);
	}		
}


/** Plays normal non-attenuated anim-driven camera shake for THIS camera. */
simulated function CameraAnimInst PlayCameraShakeAnim(const out ScreenShakeAnimStruct Shake)
{
	local float		Duration, Scale;
	local bool		bRandomStart, bLoop;
	
	if (Shake.bRandomSegment)
	{
		bLoop = TRUE;
		bRandomStart = TRUE;
		Duration = Shake.RandomSegmentDuration;
	}

	Scale = Shake.AnimScale;

	if (Scale > 0.f)
	{
		if (class'Engine'.static.IsSplitScreen())
		{
			Scale *= SplitScreenShakeScale;
		}

		return PlayCameraAnim(Shake.Anim, Shake.AnimPlayRate, Scale, Shake.AnimBlendInTime, Shake.AnimBlendOutTime, bLoop, bRandomStart, Duration, Shake.bSingleInstance);
	}

	return None;
}


/**
 * Static.  Plays a code-driven camera shake in world space to all LOCAL players, with distance-based attenuation.
 */
simulated static function PlayWorldCameraShake(const out ScreenShakeStruct Shake, Actor ShakeInstigator, vector Epicenter, float InnerRadius, float OuterRadius, float Falloff, bool bTryForceFeedback )
{
 	local GearPawn			GP;
	local GearPC			GPC;
	local GearPlayerCamera	Cam;

	if( ShakeInstigator != None )
	{
		foreach ShakeInstigator.WorldInfo.AllPawns(class'GearPawn', GP)
		{
			if( VSize(GP.Location - Epicenter) < InnerRadius + ((OuterRadius - InnerRadius) * 0.25f) )
			{
				// Trigger a CringeOverlayAnim
				GP.TryPlayNearMissCringe();
			}
		}

		foreach ShakeInstigator.LocalPlayerControllers(class'GearPC', GPC)
		{
			Cam = GearPlayerCamera(GPC.PlayerCamera);
			if (Cam != None)
			{
				Cam.PlayRadialCameraShake(Shake, Epicenter, InnerRadius, OuterRadius, Falloff, bTryForceFeedback);
			}
		}
	}
}


/**
 * Static.  Plays an anim-driven camera shake in world space to all LOCAL players, with distance-based attenuation.
 */
simulated static function PlayWorldCameraShakeAnim(const out ScreenShakeAnimStruct Shake, Actor ShakeInstigator, vector Epicenter, float InnerRadius, float OuterRadius, float Falloff, bool bTryForceFeedback )
{
 	local GearPawn			GP;
	local GearPC			GPC;
	local GearPlayerCamera	Cam;

	if( ShakeInstigator != None )
	{
		foreach ShakeInstigator.WorldInfo.AllPawns(class'GearPawn', GP)
		{
			if( VSize(GP.Location - Epicenter) < InnerRadius + ((OuterRadius - InnerRadius) * 0.25f) )
			{
				// Trigger a CringeOverlayAnim
				GP.TryPlayNearMissCringe();
			}
		}

		foreach ShakeInstigator.LocalPlayerControllers(class'GearPC', GPC)
		{
			Cam = GearPlayerCamera(GPC.PlayerCamera);
			if (Cam != None)
			{
				Cam.PlayRadialCameraShakeAnim(Shake, Epicenter, InnerRadius, OuterRadius, Falloff, bTryForceFeedback);
			}
		}
	}
}

simulated protected function CameraAnim ChooseDirectionalCameraShakeAnim(const out ScreenShakeAnimStruct Shake, vector Epicenter)
{
	local vector CamX, CamY, CamZ, ToEpicenter;
	local float FwdDot, RtDot;
	local CameraAnim ChosenAnim;
	local Rotator NoPitchRot;

	ToEpicenter = Epicenter - Location;
	ToEpicenter.Z = 0.f;
	ToEpicenter = Normal(ToEpicenter);
	NoPitchRot = Rotation;
	NoPitchRot.Pitch = 0.f;
	GetAxes(NoPitchRot, CamX, CamY, CamZ);

	FwdDot = CamX dot ToEpicenter;
	if (FwdDot > 0.707f)
	{
		// use forward
		ChosenAnim = Shake.Anim;
	}
	else if (FwdDot > -0.707f)
	{
		// need to determine r or l
		RtDot = CamY dot ToEpicenter;
		ChosenAnim = (RtDot > 0.f) ? Shake.Anim_Right : Shake.Anim_Left;
	}
	else
	{
		// use back
		ChosenAnim = Shake.Anim_Rear;
	}

	if (ChosenAnim == None)
	{
		// fall back to forward
		ChosenAnim = Shake.Anim;
	}

	return ChosenAnim;
}


defaultproperties
{
	DefaultFOV=70.f

//	RoadieRunSequenceNodeName=("AR_RoadieRun_Fwd")
	StandingIdleSequenceNodeNames=("AR_Idle_Ready","AR_Idle_Ready_Aim")

	CameraStyle=Default

	SplitScreenShakeScale=0.5f
}

