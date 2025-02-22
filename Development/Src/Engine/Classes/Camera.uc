/**
 *	Camera: defines the Point of View of a player in world space.
 * 	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Camera extends Actor
	notplaceable
	native;

/** PlayerController Owning this Camera Actor */
var		PlayerController	PCOwner;

/** Camera Mode */
var		Name 	CameraStyle;
/** default FOV */
var		float	DefaultFOV;
/** true if FOV is locked to a constant value*/
var		bool	bLockedFOV;

/** If we should insert black areas when rendering the scene to ensure an aspect ratio of ConstrainedAspectRatio */
var		bool	bConstrainAspectRatio;
/** If we should apply FadeColor/FadeAmount to the screen. */
var		bool	bEnableFading;

var bool bEnableAudioFading;

/** Indicates if CamPostProcessSettings should be used when using this Camera to view through. */
var		bool	bCamOverridePostProcess;
/** Turn on scaling of color channels in final image using ColorScale property. */
var		bool	bEnableColorScaling;
/** Should interpolate color scale values */
var		bool	bEnableColorScaleInterp;
/** value FOV is locked at */
var		float	LockedFOV;

/** If bConstrainAspectRatio is true, add black regions to ensure aspect ratio is this. Ratio is horizontal/vertical. */
var		float	ConstrainedAspectRatio;
/** Default aspect ratio */
var		float	DefaultAspectRatio;

/** Color to fade to. */
var		color	FadeColor;
/** Amount of fading to apply. */
var		float	FadeAmount;

/** Post-process settings to use if bCamOverridePostProcess is TRUE. */
var		PostProcessSettings	CamPostProcessSettings;

/** Allows control over scaling individual color channels in the final image. */
var		vector	ColorScale;
/** Desired color scale which ColorScale will interpolate to */
var		vector	DesiredColorScale;
/** Color scale value at start of interpolation */
var		vector	OriginalColorScale;
/** Total time for color scale interpolation to complete */
var		float	ColorScaleInterpDuration;
/** Time at which interpolation started */
var		float	ColorScaleInterpStartTime;

// BM1
var float SoundFadeAmount;
var float FarCullDistance;

/** The actors which the camera shouldn't see. Used to hide actors which the camera penetrates. */
//var array<Actor> HiddenActors;

/* Caching Camera, for optimization */
struct native TCameraCache
{
	/** Cached Time Stamp */
	var float	TimeStamp;
	/** cached Point of View */
	var TPOV	POV;
};
var	TCameraCache	CameraCache;


/**
 * View Target definition
 * A View Target is responsible for providing the Camera with an ideal Point of View (POV)
 */
struct native TViewTarget
{
	/** Target Actor used to compute ideal POV */
	var()	Actor					Target;
	/** Controller of Target (only for non Locally controlled Pawns) */
	var()	Controller				Controller;
	/** Point of View */
	var()	TPOV					POV;
	/** Aspect ratio */
	var()	float					AspectRatio;
	/** PlayerReplicationInfo (used to follow same player through pawn transitions, etc., when spectating) */
	var()	PlayerReplicationInfo	PRI;

};

/** Current ViewTarget */
var TViewTarget	ViewTarget;
/** Pending view target for blending */
var	TViewTarget	PendingViewTarget;
/** Time left when blending to pending view target */
var float		BlendTimeToGo;

enum EViewTargetBlendFunction
{
	/** Camera does a simple linear interpolation. */
	VTBlend_Linear,
	/** Camera has a slight ease in and ease out, but amount of ease cannot be tweaked. */
	VTBlend_Cubic,
	/** Camera immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by BlendExp. */
	VTBlend_EaseIn,
	/** Camera smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by BlendExp. */
	VTBlend_EaseOut,
	/** Camera smoothly accelerates and decelerates.  Ease amount controlled by BlendExp. */
	VTBlend_EaseInOut,
};

/** A set of parameters to describe how to transition between viewtargets. */
struct native ViewTargetTransitionParams
{
	/** Total duration of blend to pending view target.  0 means no blending. */
	var() float						BlendTime;
	/** Function to apply to the blend parameter */
	var() EViewTargetBlendFunction	BlendFunction;
	/** Exponent, used by certain blend functions to control the shape of the curve. */
	var() float						BlendExp;

    var() bool bResetCameraBehindPlayer;
    var() bool bKeepBatmanOnScreen;
    var() bool bDisableCamerCollisionDuringBlend;

	structdefaultproperties
	{
		BlendFunction=VTBlend_Cubic
		BlendExp=2.f
        bResetCameraBehindPlayer=true
        bKeepBatmanOnScreen=false
        bDisableCamerCollisionDuringBlend=false
	}

	// providing the constructor by hand here, because we pass this as an optional parameter
	// and when the parameter isn't there, the default contructor is called.
	structcpptext
	{
		FViewTargetTransitionParams()
		{}
		FViewTargetTransitionParams(EEventParm)
		: BlendTime(0.f), BlendFunction(VTBlend_Cubic), BlendExp(2.f), bResetCameraBehindPlayer(TRUE), bKeepBatmanOnScreen(FALSE), bDisableCamerCollisionDuringBlend(FALSE)
		{}
	}
};

var ViewTargetTransitionParams BlendParams;

/** List of camera modifiers to apply during update of camera position/ rotation */
var Array<CameraModifier>	ModifierList;

/** Distance to place free camera from view target */
var float		FreeCamDistance;

/** Offset to Z free camera position */
var vector		FreeCamOffset;

/** camera fade management */
var vector2d FadeAlpha;
var float FadeTime, FadeTimeRemaining;

cpptext
{
	void	AssignViewTarget(AActor* NewTarget, FTViewTarget& VT, struct FViewTargetTransitionParams TransitionParams=FViewTargetTransitionParams(EC_EventParm));
	AActor* GetViewTarget();
	virtual UBOOL	PlayerControlled();
	virtual void	ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const {};
}

/**
 * Apply modifiers on Camera.
 * @param	DeltaTime	Time is seconds since last update
 * @param	OutPOV		Point of View
 */

native function ApplyCameraModifiers(float DeltaTime, out TPOV OutPOV);

/**
 * Initialize Camera for associated PlayerController
 * @param	PC	PlayerController attached to this Camera.
 */
function InitializeFor(PlayerController PC)
{
	CameraCache.POV.FOV = DefaultFOV;
	PCOwner				= PC;

	SetViewTarget(PC.ViewTarget);

	// set the level default scale
	SetDesiredColorScale(WorldInfo.DefaultColorScale, 5.f);

	// Force camera update so it doesn't sit at (0,0,0) for a full tick.
	// This can have side effects with streaming.
	UpdateCamera(0.f);
}


/**
 * returns camera's current FOV angle
 */
function float GetFOVAngle()
{
	if( bLockedFOV )
	{
		return LockedFOV;
	}

	return CameraCache.POV.FOV;
}


/**
 * Lock FOV to a specific value.
 * A value of 0 to beyond 170 will unlock the FOV setting.
 */
function SetFOV(float NewFOV)
{
	if( NewFOV < 1 || NewFOV > 170 )
	{
		bLockedFOV = FALSE;
		return;
	}

	bLockedFOV	= TRUE;
	LockedFOV	= NewFOV;
}


/**
 * Master function to retrieve Camera's actual view point.
 * do not call this directly, call PlayerController::GetPlayerViewPoint() instead.
 *
 * @param	OutCamLoc	Camera Location
 * @param	OutCamRot	Camera Rotation
 */
final function GetCameraViewPoint(out vector OutCamLoc, out rotator OutCamRot)
{
	// @debug: find out which calls are being made before the camera has been ticked
	//			and have therefore one frame of lag.
	/*
	if( CameraCache.TimeStamp != WorldInfo.TimeSeconds )
	{
		`Log(WorldInfo.TimeSeconds @ GetFuncName() @ "one frame of lag");
		ScriptTrace();
	}
	*/

	OutCamLoc = CameraCache.POV.Location;
	OutCamRot = CameraCache.POV.Rotation;
}

/**
 * Sets the new desired color scale and enables interpolation.
 */
simulated function SetDesiredColorScale(vector NewColorScale, float InterpTime)
{
	// if color scaling is not enabled
	if (!bEnableColorScaling)
	{
		// set the default color scale
		bEnableColorScaling = TRUE;
		ColorScale.X = 1.f;
		ColorScale.Y = 1.f;
		ColorScale.Z = 1.f;
	}

	// Don't bother interpolating if we're already scaling at the desired color
	if( NewColorScale != ColorScale )
	{
		// save the current as original
		OriginalColorScale = ColorScale;
		// set the new desired scale
		DesiredColorScale = NewColorScale;
		// set the interpolation duration/time
		ColorScaleInterpStartTime = WorldInfo.TimeSeconds;
		ColorScaleInterpDuration = InterpTime;
		// and enable color scale interpolation
		bEnableColorScaleInterp = TRUE;
	}
}

/**
 * Performs camera update.
 * Called once per frame after all actors have been ticked.
 */
simulated event UpdateCamera(float DeltaTime)
{
	local TPOV		NewPOV;
	local float		DurationPct, BlendPct;

	// update color scale interpolation
	if (bEnableColorScaleInterp)
	{
		BlendPct = FClamp(TimeSince(ColorScaleInterpStartTime)/ColorScaleInterpDuration,0.f,1.f);
		ColorScale = VLerp(OriginalColorScale,DesiredColorScale,BlendPct);
		// if we've maxed
		if (BlendPct == 1.f)
		{
			// disable further interpolation
			bEnableColorScaleInterp = FALSE;
		}
	}

	// Reset aspect ratio and postprocess override associated with CameraActor.
	bConstrainAspectRatio = FALSE;
	bCamOverridePostProcess = FALSE;

	// Update main view target
	CheckViewTarget(ViewTarget);
	UpdateViewTarget(ViewTarget, DeltaTime);

	// our camera is now viewing there
	NewPOV					= ViewTarget.POV;
	ConstrainedAspectRatio	= ViewTarget.AspectRatio;

	// if we have a pending view target, perform transition from one to another.
	if( PendingViewTarget.Target != None )
	{
		BlendTimeToGo -= DeltaTime;

		// Reset aspect ratio.  The call to UpdateViewTarget() may turn this back on.
		bConstrainAspectRatio = FALSE;

		// Update pending view target
		CheckViewTarget(PendingViewTarget);
		UpdateViewTarget(PendingViewTarget, DeltaTime);

		// blend....
		if( BlendTimeToGo > 0 )
		{
			DurationPct	= 1.f - BlendTimeToGo / BlendParams.BlendTime;

			switch (BlendParams.BlendFunction)
			{
			case VTBlend_Linear:
				BlendPct = Lerp(0.f, 1.f, DurationPct);
				break;
			case VTBlend_Cubic:
				BlendPct = FCubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
				break;
			case VTBlend_EaseIn:
				BlendPct = FInterpEaseIn(0.f, 1.f, DurationPct, BlendParams.BlendExp);
				break;
			case VTBlend_EaseOut:
				BlendPct = FInterpEaseOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
				break;
			case VTBlend_EaseInOut:
				BlendPct = FInterpEaseInOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
				break;
			}
			//BlendPct	= FCubicInterp(0.f, class'DialogueManager'.default.OutTan, 1.f, class'DialogueManager'.default.InTan, 1.f - DurationPct);

			// Update pending view target blend
			NewPOV = BlendViewTargets(ViewTarget, PendingViewTarget, BlendPct);
		}
		else
		{
			// we're done blending, set new view target
			ViewTarget = PendingViewTarget;

			// clear pending view target
			PendingViewTarget.Target		= None;
			PendingViewTarget.Controller	= None;

			BlendTimeToGo = 0;

			// our camera is now viewing there
			NewPOV = PendingViewTarget.POV;
		}

		if( bConstrainAspectRatio )
		{
			// NOTE: We don't interpolate aspect ratio since either the prior or pending view target's AspectRatio
			//       may be the default value (1.3333) unless the view target has a camera actor set to override
			//       the aspect ratio.  We'll just use the pending view target's aspect.
			ConstrainedAspectRatio = PendingViewTarget.AspectRatio;
		}
	}

	// Cache results
	FillCameraCache(NewPOV);

	if (bEnableFading && FadeTimeRemaining > 0.0)
	{
		FadeTimeRemaining = FMax(FadeTimeRemaining - DeltaTime, 0.0);
		if (FadeTime > 0.0)
		{
			FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining/FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
		}
	}
}


/**
 * Blend 2 viewtargets.
 *
 * @param	A		Source view target
 * @paramn	B		destination view target
 * @param	Alpha	Alpha, % of blend from A to B.
 */
final function TPOV BlendViewTargets(const out TViewTarget A,const out TViewTarget B, float Alpha)
{
	local TPOV	POV;

	POV.Location	= VLerp(A.POV.Location, B.POV.Location, Alpha);
	POV.FOV			= Lerp(A.POV.FOV, B.POV.FOV, Alpha);
	POV.Rotation	= RLerp(A.POV.Rotation, B.POV.Rotation, Alpha, TRUE);

	return POV;
}


/**
 * Cache update results
 */
final function FillCameraCache(const out TPOV NewPOV)
{
	CameraCache.TimeStamp	= WorldInfo.TimeSeconds;
	CameraCache.POV			= NewPOV;
}


/**
 * Make sure ViewTarget is valid
 */
native function CheckViewTarget(out TViewTarget VT);


/**
 * Query ViewTarget and outputs Point Of View.
 *
 * @param	OutVT		ViewTarget to use.
 * @param	DeltaTime	Delta Time since last camera update (in seconds).
 */
function UpdateViewTarget(out TViewTarget OutVT, float DeltaTime)
{
	local vector		Loc, Pos, HitLocation, HitNormal;
	local rotator		Rot;
	local Actor			HitActor;
	local CameraActor	CamActor;
	local bool			bDoNotApplyModifiers;
	local TPOV			OrigPOV;

	// store previous POV, in case we need it later
	OrigPOV = OutVT.POV;

	// Default FOV on viewtarget
	OutVT.POV.FOV = DefaultFOV;

	// Viewing through a camera actor.
	CamActor = CameraActor(OutVT.Target);
	if( CamActor != None )
	{
		CamActor.GetCameraView(DeltaTime, OutVT.POV);

		// Grab aspect ratio from the CameraActor.
		bConstrainAspectRatio	= bConstrainAspectRatio || CamActor.bConstrainAspectRatio;
		OutVT.AspectRatio		= CamActor.AspectRatio;

		// See if the CameraActor wants to override the PostProcess settings used.
		bCamOverridePostProcess = CamActor.bCamOverridePostProcess;
		CamPostProcessSettings = CamActor.CamOverridePostProcess;
	}
	else
	{
		// Give Pawn Viewtarget a chance to dictate the camera position.
		// If Pawn doesn't override the camera view, then we proceed with our own defaults
		if( Pawn(OutVT.Target) == None ||
			!Pawn(OutVT.Target).CalcCamera(DeltaTime, OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV) )
		{
			// don't apply modifiers when using these debug camera modes.
			bDoNotApplyModifiers = TRUE;

			switch( CameraStyle )
			{
				case 'Fixed'		:	// do not update, keep previous camera position by restoring
										// saved POV, in case CalcCamera changes it but still returns false
										OutVT.POV = OrigPOV;
										break;

				case 'ThirdPerson'	: // Simple third person view implementation
				case 'FreeCam'		:
										Loc = OutVT.Target.Location;
										Rot = OutVT.Target.Rotation;

										//OutVT.Target.GetActorEyesViewPoint(Loc, Rot);
										if( CameraStyle == 'FreeCam' )
										{
											Rot = PCOwner.Rotation;
										}
										Loc += FreeCamOffset >> Rot;

										Pos = Loc - Vector(Rot) * FreeCamDistance;
										// @fixme, respect BlockingVolume.bBlockCamera=false
										HitActor = Trace(HitLocation, HitNormal, Pos, Loc, FALSE, vect(12,12,12));
										OutVT.POV.Location = (HitActor == None) ? Pos : HitLocation;
										OutVT.POV.Rotation = Rot;
										break;

				case 'FirstPerson'	: // Simple first person, view through viewtarget's 'eyes'
				default				:	OutVT.Target.GetActorEyesViewPoint(OutVT.POV.Location, OutVT.POV.Rotation);
										break;

			}
		}
	}

	if( !bDoNotApplyModifiers )
	{
		// Apply camera modifiers at the end (view shakes for example)
		ApplyCameraModifiers(DeltaTime, OutVT.POV);
	}
	//`log( WorldInfo.TimeSeconds  @ GetFuncName() @ OutVT.Target @ OutVT.POV.Location @ OutVT.POV.Rotation @ OutVT.POV.FOV );
}


/**
 * Set a new ViewTarget with optional BlendTime
 */
native final function SetViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams);


/**
 * Give each modifier a chance to change view rotation/deltarot
 */
function ProcessViewRotation(float DeltaTime, out rotator OutViewRotation, out Rotator OutDeltaRot)
{
	local int ModifierIdx;

	for( ModifierIdx = 0; ModifierIdx < ModifierList.Length; ModifierIdx++ )
	{
		if( ModifierList[ModifierIdx] != None )
		{
			if( ModifierList[ModifierIdx].ProcessViewRotation(ViewTarget.Target, DeltaTime, OutViewRotation, OutDeltaRot) )
			{
				break;
			}
		}
	}
}



function bool AllowPawnRotation()
{
	return TRUE;
}

/**
 * list important Camera variables on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Vector	EyesLoc;
	local Rotator	EyesRot;
	local Canvas	Canvas;

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255,255,255);

	Canvas.DrawText("	Camera Style:" $ CameraStyle @ "main ViewTarget:" $ ViewTarget.Target);
	out_YPos += out_YL;
	Canvas.SetPos(4,out_YPos);

	Canvas.DrawText("   CamLoc:" $ CameraCache.POV.Location @ "CamRot:" $ CameraCache.POV.Rotation @ "FOV:" $ CameraCache.POV.FOV);
	out_YPos += out_YL;
	Canvas.SetPos(4,out_YPos);

	Canvas.DrawText("   AspectRatio:" $ ConstrainedAspectRatio);
	out_YPos += out_YL;
	Canvas.SetPos(4,out_YPos);

	if( ViewTarget.Target != None )
	{
		ViewTarget.Target.GetActorEyesViewPoint(EyesLoc, EyesRot);
		Canvas.DrawText("   EyesLoc:" $ EyesLoc @ "EyesRot:" $ EyesRot);
		out_YPos += out_YL;
		Canvas.SetPos(4,out_YPos);
	}
}

defaultproperties
{
	DefaultFOV=90.f
	DefaultAspectRatio=1.333333333 // 4/3
	bHidden=TRUE
	RemoteRole=ROLE_None
	FreeCamDistance=256.f
}
