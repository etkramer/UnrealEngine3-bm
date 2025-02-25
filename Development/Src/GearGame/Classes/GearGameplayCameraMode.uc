/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameplayCameraMode extends Object
	config(Camera)
	native(Camera);

/**
 * Contains all the information needed to define a camera
 * mode.
 */

/** Ref to the camera object that owns this mode object. */
var transient GearGameplayCamera	GameplayCam;

/** Pawn relative offset. It's relative from Pawn's location, aligned to Pawn's rotation */
var() vector						PawnRelativeOffset;

/** FOV for camera to use */
var() const config float			FOVAngle;

/** Blend Time to and from this view mode */
var() const float					BlendTime;

/**
 * True if, while in this mode, the camera should be tied to the viewtarget rotation.
 * This is typical for the normal walking-around camera, since the controls rotate the controller
 * and the camera follows.  This can be false if you want free control of the camera, independent
 * of the viewtarget's orient -- we use this for vehicles.  Note that if this is false,
 */
var() const protected bool bLockedToViewTarget;

/**
 * True if, while in this mode, looking around should be directly mapped to stick position
 * as opposed to relative to previous camera positions.
 */
var() const protected bool bDirectLook;

/**
 * True if, while in this mode, the camera should interpolate towards a following position
 * in relation to the target and it's motion.  Ignored if bLockedToViewTarget is set to true.
 */
var() const protected bool bFollowTarget;

/*
 * How fast the camera should track to follow behind the viewtarget.  0.f for no following.
 * Only used if bLockedToViewTarget is FALSE
 */
var() const protected float		FollowingInterpSpeed_Pitch;
var() const protected float		FollowingInterpSpeed_Yaw;
var() const protected float		FollowingInterpSpeed_Roll;

/** Actual following interp speed gets scaled from FollowingInterpSpeed to zero between velocities of this value and zero. */
var() const protected float		FollowingCameraVelThreshold;


/** True means camera will attempt to smoothly interpolate to its new position.  False will snap it to it's new position. */
var() protected bool		bInterpLocation;
/** Controls interpolation speed for location.  Ignored if bInterpLocation is false. */
//@note - removed 'const' as this doesn't seem dangerous to dynamically change
var() /*const*/ protected float	InterpLocSpeed;

var() protected bool		bUsePerAxisLocInterp;
var() protected vector		PerAxisLocInterpSpeed;

/** True means camera will attempt to smoothly interpolate to its new rotation.  False will snap it to it's new rotation. */
var() protected bool		bInterpRotation;
/** Controls interpolation speed for rotation.  Ignored if bInterpRotation is false. */
var() const protected float	InterpRotSpeed;


/** Adjustment vector to apply to camera view offset when target is strafing to the left */
var() const protected vector	StrafeLeftAdjustment;
/** Adjustment vector to apply to camera view offset when target is strafing to the right */
var() const protected vector	StrafeRightAdjustment;
/** Velocity at (and above) which the full adjustment should be applied. */
var() const protected float		StrafeOffsetScalingThreshold;
/** Interpolation speed for interpolating to a NONZERO strafe offsets.  Higher is faster/tighter interpolation. */
var() const protected float		StrafeOffsetInterpSpeedIn;
/** Interpolation speed for interpolating to a ZERO strafe offset.  Higher is faster/tighter interpolation. */
var() const protected float		StrafeOffsetInterpSpeedOut;
/** Strafe offset last tick, used for interpolation. */
var protected transient vector LastStrafeOffset;

/** Adjustment vector to apply to camera view offset when target is moving forward */
var() const protected vector	RunFwdAdjustment;
/** Adjustment vector to apply to camera view offset when target is moving backward */
var() const protected vector	RunBackAdjustment;
/** Velocity at (and above) which the full adjustment should be applied. */
var() const protected float		RunOffsetScalingThreshold;
/** Interpolation speed for interpolating to a NONZERO offset.  Higher is faster/tighter interpolation. */
var() const protected float		RunOffsetInterpSpeedIn;
/** Interpolation speed for interpolating to a ZERO offset.  Higher is faster/tighter interpolation. */
var() const protected float		RunOffsetInterpSpeedOut;
/** Run offset last tick, used for interpolation. */
var protected transient vector LastRunOffset;

/** offset from viewtarget, to calculate worst camera location. Is also mirrored like player. */
var() const protected vector	WorstLocOffset;

/** True to turn do predictive camera avoidance, false otherwise */
var() const protected bool		bDoPredictiveAvoidance;

/** TRUE to do a raytrace from camera base loc to worst loc, just to be sure it's cool.  False to skip it */
var() const protected bool		bValidateWorstLoc;

/** If TRUE, all camera collision is disabled */
var() const protected bool		bSkipCameraCollision;

/** Offset, in the camera target's local space, from the camera target to the camera's origin. */
var() const protected vector TargetRelativeCameraOriginOffset;

/** Supported viewport configurations. */
enum EGearCam_ViewportTypes
{
	CVT_16to9_Full,
	CVT_16to9_VertSplit,
	CVT_16to9_HorizSplit,
	CVT_4to3_Full,
	CVT_4to3_HorizSplit,
	CVT_4to3_VertSplit,
};

struct native ViewOffsetData
{
	/** View point offset for high player view pitch */
	var() vector OffsetHigh;
	/** View point offset for medium (horizon) player view pitch */
	var() vector OffsetMid;
	/** View point offset for low player view pitch */
	var() vector OffsetLow;
};

/** contains offsets from camera target to camera loc */
var() const protected ViewOffsetData	ViewOffset;

/** viewoffset adjustment vectors for each possible viewport type, so the game looks the same in each */
var() const protected ViewOffsetData	ViewOffset_ViewportAdjustments[EGearCam_ViewportTypes.EnumCount];


/** Optional parameters for DOF adjustments. */
var() protected bool	bAdjustDOF;
var() protected bool	bDOFUpdated;
var() protected float	DOF_FalloffExponent;
var() protected float	DOF_BlurKernelSize;
var() protected float	DOF_FocusInnerRadius;
var() protected float	DOF_MaxNearBlurAmount;
var() protected float	DOF_MaxFarBlurAmount;

var protected transient float	LastDOFRadius;
var protected transient float	LastDOFDistance;
var() protected const float		DOFDistanceInterpSpeed;
var() protected const vector	DOFTraceExtent;

/** Maps out how the DOF inner radius changes over distance. */
var() protected const float		DOF_RadiusFalloff;
var() protected const vector2d	DOF_RadiusRange;
var() protected const vector2d	DOF_RadiusDistRange;

cpptext
{
	// GearGameplayCameraMode interface

	/**
	 * Calculates and returns the ideal view offset for the specified camera mode.
	 * The offset is relative to the Camera's pos/rot and calculated by interpolating
	 * 2 ideal view points based on the player view pitch.
	 *
	 * @param	ViewedPawn			Camera target pawn
	 * @param	DeltaTime			Delta time since last frame.
	 * @param	ViewRotation		Rot of the camera
	 */
	FVector GetViewOffset(class APawn* ViewedPawn, FLOAT DeltaTime, const FRotator& ViewRotation);

	/** Returns View relative offsets */
	virtual void GetBaseViewOffsets(class APawn* ViewedPawn, BYTE ViewportConfig, FLOAT DeltaTime, FVector& out_Low, FVector& out_Mid, FVector& out_High);

	/** Returns true if mode should be using direct-look mode, false otherwise */
	virtual UBOOL UseDirectLookMode(class APawn* CameraTarget);

	/** Returns true if mode should lock camera to view target, false otherwise */
	virtual UBOOL LockedToViewTarget(class APawn* CameraTarget);

	/**
	 * Returns true if this mode should do target following.  If true is returned, interp speeds are filled in.
	 * If false is returned, interp speeds are not altered.
	 */
	virtual UBOOL ShouldFollowTarget(class APawn* CameraTarget, FLOAT& PitchInterpSpeed, FLOAT& YawInterpSpeed, FLOAT& RollInterpSpeed);

	/** Returns an offset, in pawn-local space, to be applied to the camera origin. */
	virtual FVector GetTargetRelativeOriginOffset(class APawn* TargetPawn);

	/**
	 * Returns location and rotation, in world space, of the camera's basis point.  The camera will rotate
	 * around this point, offsets are applied from here, etc.
	 */
	virtual void GetCameraOrigin(class APawn* TargetPawn, FVector& OriginLoc, FRotator& OriginRot);

	/** Returns time to interpolate location/rotation changes. */
	virtual FLOAT GetBlendTime(class APawn* Pawn);

	/** Returns time to interpolate FOV changes. */
	virtual FLOAT GetFOVBlendTime(class APawn* Pawn);
};


/** Called when Camera mode becomes active */
function OnBecomeActive(Pawn TargetPawn, GearGameplayCameraMode PrevMode);

/** Called when camera mode becomes inactive */
function OnBecomeInActive(Pawn TargetPawn, GearGameplayCameraMode NewMode);

simulated function GearPC GetGearPC()
{
	return GearPC(GameplayCam.PlayerCamera.PCOwner);
}


/** Get Pawn's relative offset (from location based on pawn's rotation */
event vector GetPawnRelativeOffset( Pawn P )
{
	local vector	FinalOffset;
	local GearPawn	WP;

	WP = GearPawn(P);

	FinalOffset = PawnRelativeOffset;

	// if target is a warpawn and pawn is facing left, then mirror Y axis.
	if( (WP != None) && WP.bIsMirrored )
	{
		FinalOffset.Y = -FinalOffset.Y;
	}
	return FinalOffset;
}

/** returns camera mode desired FOV */
function float GetDesiredFOV( Pawn ViewedPawn )
{
	local GearWeapon GW;

	GW = GearWeapon(ViewedPawn.Weapon);

	if (GW != None)
	{
		return GW.GetAdjustedFOV(FOVAngle);
	}
	else
	{
		return FOVAngle;
	}
}

/**
 * Notification from GearInventoryManager when my player's weapon changes
 */
function WeaponChanged(Controller C, Weapon OldWeapon, Weapon NewWeapon);

/**
 * Returns the "worst case" camera location for this camera mode.
 * This is the position that the camera ray is shot from, so it should be
 * a guaranteed safe place to put the camera.
 */
simulated event vector GetCameraWorstCaseLoc(Pawn TargetPawn)
{
	local GearPawn WP;
	local GearTurret WT;
	local vector WorstLocation;

	WP = GearPawn(TargetPawn);

	if (WP != None)
	{
		WorstLocation = WP.Location + (WorstLocOffset >> WP.Rotation);
	}
	else
	{
		WT = GearTurret(TargetPawn);
		if (WT != None)
		{
			WorstLocation = WT.GetCameraWorstCaseLoc();
		}
		else
		{
			`log("Warning, using default WorstLocation in "@GetFuncName());
			WorstLocation = TargetPawn.Location;
		}
	}

	return WorstLocation;
}

/**
 * Camera mode has a chance to set a focus point, if it so chooses.
 * Return true if setting one, false if not.
 */
simulated function bool SetFocusPoint(Pawn ViewedPawn)
{
	// no focus point by default
	return FALSE;
}

simulated function ProcessViewRotation( float DeltaTime, Actor ViewTarget, out Rotator out_ViewRotation, out Rotator out_DeltaRot );


simulated protected function vector GetDOFFocusLoc(Actor TraceOwner, vector StartTrace, vector EndTrace)
{
	local GearPawn	GP;
	local vector FocusLoc;

	// see if weapon wants to define focus loc
	GP = GearPawn(TraceOwner);
	if ( (GP != None) && (GP.MyGearWeapon != None) )
	{
		if (GP.MyGearWeapon.GetCameraDOFFocusLocation(FocusLoc))
		{
			return FocusLoc;
		}
	}

	return DOFTrace(TraceOwner, StartTrace, EndTrace);

}

/** Modelled after CalcWeaponFire. */
simulated protected function vector DOFTrace(Actor TraceOwner, vector StartTrace, vector EndTrace)
{
	local vector	HitLocation, HitNormal;
	local Actor		HitActor;

	// Perform trace to retrieve hit info
	HitActor = TraceOwner.Trace(HitLocation, HitNormal, EndTrace, StartTrace, TRUE, DOFTraceExtent,, TraceOwner.TRACEFLAG_Bullet);

	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if( HitActor == None )
	{
		HitLocation	= EndTrace;
	}

//	`log("DOF trace hit"@HitActor);

	// check to see if we've hit a trigger.
	// In this case, we want to add this actor to the list so we can give it damage, and then continue tracing through.
	if( HitActor != None )
	{
		if ( !HitActor.bBlockActors && 
			 ( HitActor.IsA('Trigger') || HitActor.IsA('TriggerVolume') ) )
		{	
			// disable collision temporarily for the trigger so that we can catch anything inside the trigger
			HitActor.bProjTarget = false;
			// recurse another trace
//			`log("... recursing!");
			HitLocation = DOFTrace(TraceOwner, HitLocation, EndTrace);
			// and reenable collision for the trigger
			HitActor.bProjTarget = true;
		}
	}

	return HitLocation;
}


/** Gives mode a chance to adjust/override postprocess as desired. */
simulated function UpdatePostProcess(const out TViewTarget VT, float DeltaTime)
{
	local vector FocusLoc, StartTrace, EndTrace, CamDir;
	local float FocusDist, SubjectDist, Pct;
	local GearPC GPC;

	bDOFUpdated = FALSE;

	if (bAdjustDOF)
	{
		GPC = GearPC(GameplayCam.PlayerCamera.PCOwner);
		if ( (GPC == None) || (!GPC.bDisableCameraTargetingDOF) )
		{
			// nudge forward a little, in case camera gets itself in a wierd place.
			CamDir = vector(VT.POV.Rotation);
			StartTrace = VT.POV.Location + CamDir * 10;
			EndTrace = StartTrace + CamDir * 50000;
			FocusLoc = GetDOFFocusLoc(VT.Target, StartTrace, EndTrace);
			SubjectDist = VSize(FocusLoc - StartTrace);

			if (!GameplayCam.bResetCameraInterpolation)
			{
				FocusDist = FInterpTo(LastDOFDistance, SubjectDist, DeltaTime, DOFDistanceInterpSpeed);
			}
			else
			{
				FocusDist = SubjectDist;
			}
			LastDOFDistance = FocusDist;

			// find focus radius
			// simpler method, with better-feeling results than actual optics math
			Pct = GetRangePctByValue(DOF_RadiusDistRange, FocusDist);
			LastDOFRadius = GetRangeValueByPct(DOF_RadiusRange, FClamp(Pct, 0.f, 1.f)**DOF_RadiusFalloff);

			bDOFUpdated = TRUE;
	//		`log("Focus rad:dist"@FocusRadius@FocusDist@Pct);
		}
	}
}

simulated function ModifyPostProcessSettings(out PostProcessSettings PP)
{
	if (bDOFUpdated)
	{
		PP.bEnableDOF = TRUE;
		PP.DOF_FalloffExponent = DOF_FalloffExponent;
		PP.DOF_BlurKernelSize = DOF_BlurKernelSize;
		PP.DOF_MaxNearBlurAmount = DOF_MaxNearBlurAmount;
		PP.DOF_FocusType = FOCUS_Distance;
		PP.DOF_FocusInnerRadius = DOF_FocusInnerRadius;

		PP.DOF_FocusDistance = LastDOFDistance;
		PP.DOF_FocusInnerRadius = LastDOFRadius;

		bDOFUpdated = FALSE;
	}
}

defaultproperties
{
	BlendTime=0.67

	bLockedToViewTarget=TRUE
	bDoPredictiveAvoidance=TRUE
	bValidateWorstLoc=TRUE

	bInterpLocation=TRUE
	InterpLocSpeed=8

	StrafeLeftAdjustment=(X=0,Y=0,Z=0)
	StrafeRightAdjustment=(X=0,Y=0,Z=0)
	StrafeOffsetInterpSpeedIn=12.f
	StrafeOffsetInterpSpeedOut=20.f
	RunFwdAdjustment=(X=0,Y=0,Z=0)
	RunBackAdjustment=(X=0,Y=0,Z=0)
	RunOffsetInterpSpeedIn=6.f
	RunOffsetInterpSpeedOut=12.f

	WorstLocOffset=(X=-8,Y=1,Z=90)

	/** all offsets are hand-crafted for this mode, so they should theoretically be all zeroes for all modes */
	ViewOffset_ViewportAdjustments(CVT_16to9_Full)={(
		OffsetHigh=(X=0,Y=0,Z=0),
		OffsetLow=(X=0,Y=0,Z=0),
		OffsetMid=(X=0,Y=0,Z=0),
		)}

	bAdjustDOF=FALSE
	DOF_FalloffExponent=1.f
	DOF_BlurKernelSize=3.f
//	DOF_FocusInnerRadius=2500.f
	DOF_MaxNearBlurAmount=0.6f
	DOF_MaxFarBlurAmount=1.f
	DOFDistanceInterpSpeed=10.f
	DOFTraceExtent=(X=0.f,Y=0.f,Z=0.f)

	DOF_RadiusFalloff=1.f
	DOF_RadiusRange=(X=2500,Y=60000)
	DOF_RadiusDistRange=(X=1000,Y=50000)
}





