/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameplayCamera extends GearCameraBase
	config(Camera)
	native(Camera);

// NOTE FOR REFERENCE
// >> IS LOCAL->WORLD (no transpose)
// << IS WORLD->LOCAL (has the transpose)

/** GoW global macros */


/** Last actual camera origin position, for lazy cam interpolation. It's only applied to player's origin, not view offsets, for faster/smoother response */
var	transient vector	LastActualCameraOrigin;

/** obstruction pct from origin to worstloc origin */
var		float	WorstLocBlockedPct;
/** camera extent scale to use when calculating penetration for this segment */
var()	float	WorstLocPenetrationExtentScale;

/** Time to transition from blocked location to ideal position, after camera collision with geometry. */
var()	float	PenetrationBlendOutTime;
/** Time to transition from ideal location to blocked position, after camera collision with geometry. (used only by predictive feelers) */
var()	float	PenetrationBlendInTime;
/** Percentage of distance blocked by collision. From worst location, to desired location. */
var private float	PenetrationBlockedPct;
/** camera extent scale to use when calculating penetration for this segment */
var()	float	PenetrationExtentScale;



/**
 * Last pawn relative offset, for slow offsets interpolation.
 * This is because this offset is relative to the Pawn's rotation, which can change abruptly (when snapping to cover).
 * Used to adjust the camera origin (evade, lean, pop up, blind fire, reload..)
 */
var		transient	vector	LastActualOriginOffset;
var		transient	rotator	LastActualCameraOriginRot;
/** origin offset interpolation speed */
var()				float	OriginOffsetInterpSpeed;

/** View relative offset. This offset is relative to Controller's rotation, mainly used for Pitch positioning. */
var		transient	vector	LastViewOffset;
/** last CamFOV for war cam interpolation */
var		transient	float	LastCamFOV;


/*********** CAMERA VARIABLES ***********/
/******* CAMERA MODES *******/
/** Base camera position when walking */
var(Camera)	private editinline	GearGameplayCameraMode		GearCamDefault;
/** Cover camera position when in cover */
var(Camera) private editinline	GearGameplayCameraMode		GearCamCover;
/** Cover camera position when in cover and targeting */
var(Camera) private editinline	GearGameplayCameraMode		GearCamCoverTargeting;
/** Cover camera position when in cover and targeting with grenades */
var(Camera) private editinline	GearGameplayCameraMode		GearCamCoverTargetingGrenade;
/** crouch camera mode */
var(Camera)	private editinline	GearGameplayCameraMode		GearCamCrouch;
/** Roadie run camera mode */
var(Camera)	private editinline	GearGameplayCameraMode		GearCamRoadie;
/** targeting camera mode */
var(Camera)	private editinline	GearGameplayCameraMode		GearCamTargeting;
/** targeting camera mode special to grenades */
var(Camera)	private editinline	GearGameplayCameraMode		GearCamTargetingGrenade;
/** DBNO camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamDBNO;
/** Death camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamDeath;
/** Death camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamRagdoll;
/** Vehicle camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamVehicle;
/** Vehicle turret camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamVehicleTurret;
/** Vehicle turret camera mode, while targeting */
var(Camera) private editinline	GearGameplayCameraMode		GearCamVehicleTurretTargeting;
/** Ride reaver camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamVehicleRideReaver;
/** Centaur death cam mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamCentaurDeath;
/** Stationary turret camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamTurret;
/** Stationary turret camera mode (targeting mode) */
var(Camera) private editinline	GearGameplayCameraMode		GearCamTurretTargeting;
/** Special camera mode for cinematic-esque forcewalk conversations. */
var(Camera) private editinline	GearGameplayCameraMode		GearCamConversation;
/** Camera mode for when using a mounted heavy weapon. */
var(Camera) private editinline	GearGameplayCameraMode		GearCamMountedHeavyWeapon;
/** Camera mode for when shield is deployed. */
var(Camera) private editinline	GearGameplayCameraMode		GearCamDeployedShield;
/** Player riding brumak camera mode */
var(Camera) private editinline	GearGameplayCameraMode		GearCamBrumakDriver, GearCamBrumakGunner;
/** Player targeting while riding brumak */
var(Camera) private	editinline	GearGameplayCameraMode		GearCamBrumakDriverTargeting, GearCamBrumakGunnerTargeting;
/** Brumak death cam mode */
var(Camera) private	editinline	GearGameplayCameraMode		GearCamBrumakDeath;
/** Player targeting while riding brumak */
var(Camera) private	editinline	GearGameplayCameraMode		GearCamSniperZoom;
/** Mantling camera */
var(Camera) private editinline	GearGameplayCameraMode		GearCamMantle;


/** Default camera mode to use instead of GearCamDefault.  Can be set at runtime (ie via Kismet) */
var private GearGameplayCameraMode							CustomDefaultCamMode;


//
// Player 'GearCam' camera mode system
//

/** Current GearCam Mode */
var()			GearGameplayCameraMode	CurrentCamMode;

//
// Focus Point adjustment
//

/** last offset adjustment, for smooth blend out */
var transient	float	LastHeightAdjustment;
/** last adjusted pitch, for smooth blend out */
var transient	float	LastPitchAdjustment;
/** last adjusted Yaw, for smooth blend out */
var transient	float	LastYawAdjustment;
/** pitch adjustment when keeping target is done in 2 parts.  this is the amount to pitch in part 2 (post view offset application) */
var transient	float	LeftoverPitchAdjustment;

/**  move back pct based on move up */
var(Focus)			float		Focus_BackOffStrength;
/** Z offset step for every try */
var(Focus)			float		Focus_StepHeightAdjustment;
/** number of tries to have focus in view */
var(Focus)			int			Focus_MaxTries;
/** time it takes for fast interpolation speed to kick in */
var(Focus)			float		Focus_FastAdjustKickInTime;
/** Last time focus point changed (location) */
var transient protected float		LastFocusChangeTime;
var transient protected vector		ActualFocusPointWorldLoc;
/** Last focus point location */
var	transient protected vector		LastFocusPointLoc;

/** Camera focus point definition */
struct native CamFocusPointParams
{
	/** Actor to focus on. */
	var()	Actor		FocusActor;
	/** Bone name to focus on.  Ignored if FocusActor is None or has no SkeletalMeshComponent */
	var()	Name		FocusBoneName;
	/** Focus point location in world space.  Ignored if FocusActor is not None. */
	var()	vector		FocusWorldLoc;

	/** If >0, FOV to force upon camera while looking at this point (degrees) */
	var()	float		CameraFOV;
	/** Interpolation speed (X=slow/focus loc moving, Y=fast/focus loc steady/blending out) */
	var()	vector2d	InterpSpeedRange;
	/** FOV where target is considered in focus, no correction is made.  X is yaw tolerance, Y is pitch tolerance. */
	var()	vector2d	InFocusFOV;
	/** If FALSE, focus only if point roughly in view; if TRUE, focus no matter where player is looking */
	var()	bool		bAlwaysFocus;
	/** If TRUE, camera adjusts to keep player in view, if FALSE the camera remains fixed and just rotates in place */
	var()	bool		bAdjustCamera;
	/** If TRUE, ignore world trace to find a good spot */
	var()	bool		bIgnoreTrace;

	/** Offsets the pitch.  e.g. 20 will look 20 degrees above the target */
	var()	float		FocusPitchOffsetDeg;
};
/** current focus point */
var(Focus)	CamFocusPointParams	FocusPoint;
/** do we have a focus point set? */
var		bool					bFocusPointSet;
/** Internal.  TRUE if the focus point was good and the camera looked at it, FALSE otherise (e.g. failed the trace). */
var protected transient bool	bFocusPointSuccessful;

/** Vars for code-driven camera turns */
var	private	float			TurnCurTime;
var	private	bool			bDoingACameraTurn;
var	private int				TurnStartAngle;
var	private	int				TurnEndAngle;
var	private float			TurnTotalTime;
var	private	float			TurnDelay;
var	private	bool			bTurnAlignTargetWhenFinished;
/** Saved data for camera turn "align when finished" functionality */
var private transient int	LastPostCamTurnYaw;

/** toggles debug mode */
var()	bool				bCamDebug;

/** direct look vars */
var		transient	int		DirectLookYaw;
var		transient	bool	bDoingDirectLook;
var()				float	DirectLookInterpSpeed;

var() float					WorstLocInterpSpeed;
var transient vector		LastWorstLocationLocal;

/** Last rotation of the camera, cached before camera modifiers are applied. Used by focus point code. */
var transient rotator		LastPreModifierCameraRot;

/** interp speed for Camera rotational interpolation that is used while spectating in network games */
var() private float			SpectatorCameraRotInterpSpeed;

/** True if camera wants to be in conversation mode. False otherwise. */
var transient bool			bConversationMode;






/**
 * Struct defining a feeler ray used for camera penetration avoidance.
 */
struct native PenetrationAvoidanceFeeler
{
	/** rotator describing deviance from main ray */
	var() Rotator	AdjustmentRot;
	/** how much this feeler affects the final position if it hits the world */
	var() float		WorldWeight;
	/** how much this feeler affects the final position if it hits a Pawn (setting to 0 will not attempt to collide with pawns at all) */
	var() float		PawnWeight;
	/** extent to use for collision when firing this ray */
	var() vector	Extent;
};
var() array<PenetrationAvoidanceFeeler> PenetrationAvoidanceFeelers;


enum EAltDefaultCameraModes
{
	DefaultCam_Normal,
};


cpptext
{
protected:
	/** Returns the focus location, adjusted to compensate for the third-person camera offset. */
	FVector GetEffectiveFocusLoc(const FVector& CamLoc, const FVector& FocusLoc, const FVector& ViewOffset);
	void AdjustToFocusPointKeepingTargetInView(class APawn* P, FLOAT DeltaTime, FVector& CamLoc, FRotator& CamRot, const FVector& ViewOffset);
	void AdjustToFocusPoint(class APawn* P, FLOAT DeltaTime, FVector& CamLoc, FRotator& CamRot);
	void PreventCameraPenetration(class APawn* P, const FVector& WorstLocation, FVector& DesiredLocation, FLOAT DeltaTime, FLOAT& DistBlockedPct, FLOAT CameraExtentScale, UBOOL bSingleRayOnly=FALSE);
	void UpdateForMovingBase(class AActor* BaseActor);

public:

};



/** Internal. */
private final function GearGameplayCameraMode CreateCameraMode(class<GearGameplayCameraMode> ModeClass)
{
	local GearGameplayCameraMode NewMode;
	NewMode = new(Outer) ModeClass;
	NewMode.GameplayCam = self;
	return NewMode;
}


// reset the camera to a good state
function Reset()
{
	bResetCameraInterpolation = true;
}


simulated function Init()
{
	// Setup camera modes
	if ( GearCamDefault == None )
	{
		GearCamDefault					= CreateCameraMode(class'GameplayCam_Default');
		GearCamCover					= CreateCameraMode(class'GameplayCam_Cover');
		GearCamCoverTargeting			= CreateCameraMode(class'GameplayCam_CoverTargeting');
		GearCamCoverTargetingGrenade	= CreateCameraMode(class'GameplayCam_CoverTargetingGrenade');
		GearCamCrouch					= CreateCameraMode(class'GameplayCam_Crouch');
		GearCamRoadie					= CreateCameraMode(class'GameplayCam_RoadieRun');
		GearCamTargeting				= CreateCameraMode(class'GameplayCam_Targeting');
		GearCamTargetingGrenade			= CreateCameraMode(class'GameplayCam_TargetingGrenade');
		GearCamDBNO						= CreateCameraMode(class'GameplayCam_DBNO');
		GearCamDeath					= CreateCameraMode(class'GameplayCam_Death');
		GearCamRagdoll					= CreateCameraMode(class'GameplayCam_Ragdoll');
		GearCamVehicle					= CreateCameraMode(class'GameplayCam_Vehicle');
		GearCamVehicleTurret			= CreateCameraMode(class'GameplayCam_VehicleTurret');
		GearCamVehicleTurretTargeting	= CreateCameraMode(class'GameplayCam_VehicleTurretTargeting');
		GearCamVehicleRideReaver		= CreateCameraMode(class'GameplayCam_Vehicle_RideReaver');
		GearCamCentaurDeath				= CreateCameraMode(class'GameplayCam_CentaurDeath');
		GearCamTurret					= CreateCameraMode(class'GameplayCam_Turret');
		GearCamTurretTargeting			= CreateCameraMode(class'GameplayCam_TurretTargeting');
		GearCamConversation				= CreateCameraMode(class'GameplayCam_Conversation');
		GearCamMountedHeavyWeapon		= CreateCameraMode(class'GameplayCam_MountedHeavyWeapon');
		GearCamDeployedShield			= CreateCameraMode(class'GameplayCam_DeployedShield');
		GearCamBrumakDriver				= CreateCameraMode(class'GameplayCam_BrumakDriver');
		GearCamBrumakDriverTargeting	= CreateCameraMode(class'GameplayCam_BrumakTargeting');
		GearCamBrumakGunner				= CreateCameraMode(class'GameplayCam_BrumakGunner');
		GearCamBrumakGunnerTargeting	= CreateCameraMode(class'GameplayCam_BrumakGunnerTargeting');
		GearCamBrumakDeath				= CreateCameraMode(class'GameplayCam_BrumakDeath');
		GearCamSniperZoom				= CreateCameraMode(class'GameplayCam_SniperZoom');
		GearCamMantle					= CreateCameraMode(class'GameplayCam_Mantle');
	}
}

/** returns camera mode desired FOV */
event float GetDesiredFOV( Pawn ViewedPawn )
{
	if ( bFocusPointSet && (FocusPoint.CameraFOV > 0.f) && bFocusPointSuccessful )
	{
		return FocusPoint.CameraFOV;
	}

	return CurrentCamMode.GetDesiredFOV(ViewedPawn);
}


/**
* Player Update Camera code
*/
simulated function UpdateCamera(Pawn P, float DeltaTime, out TViewTarget OutVT)
{
	// give pawn chance to hijack the camera and do it's own thing.
	if( P.CalcCamera(DeltaTime, OutVT.POV.Location, OutVT.POV.Rotation, OutVT.POV.FOV) )
	{
		//`log("P.CalcCamera returned TRUE somehow"@P@P.Controller);
		//@fixme, move this call up into GearPlayerCamera??? look into it.
		PlayerCamera.ApplyCameraModifiers(DeltaTime, OutVT.POV);
		return;
	}
	else
	{
		UpdateCameraMode(P);
		PlayerUpdateCameraNative(P, DeltaTime, OutVT);

		CurrentCamMode.UpdatePostProcess(OutVT, DeltaTime);

		// prints local space camera offset (from pawnloc).  useful for determining camera anim test offsets
		//`log("***"@((OutVT.POV.Location - P.Location) << P.Controller.Rotation));
	}

	// if we had to reset camera interpolation, then turn off flag once it's been processed.
	bResetCameraInterpolation = FALSE;
}


/** internal camera updating code */
final native protected function PlayerUpdateCameraNative(Pawn P, float DeltaTime, out TViewTarget OutVT);

/**
* Initiates a camera rotation.
* @param		StartAngle		Starting Yaw offset (in Rotator units)
* @param		EndAngle		Finishing Yaw offset (in Rotator units)
* @param		TimeSec			How long the rotation should take
* @param		DelaySec		How long to wait before starting the rotation
*/
final simulated function BeginTurn(int StartAngle, int EndAngle, float TimeSec, optional float DelaySec, optional bool bAlignTargetWhenFinished)
{
	bDoingACameraTurn = TRUE;
	TurnTotalTime = TimeSec;
	TurnDelay = DelaySec;
	TurnCurTime = 0.f;
	TurnStartAngle = StartAngle;
	TurnEndAngle = EndAngle;
	bTurnAlignTargetWhenFinished = bAlignTargetWhenFinished;
}

/**
* Stops a camera rotation.
*/
final simulated native function EndTurn();

/**
* Adjusts a camera rotation.  Useful for situations where the basis of the rotation
* changes.
* @param	AngleOffset		Yaw adjustment to apply (in Rotator units)
*/
final function AdjustTurn(int AngleOffset)
{
	TurnStartAngle += AngleOffset;
	TurnEndAngle += AngleOffset;
}

/********************************************
 * Focus Point functionality
 ******************************/

/** Tells camera to focus on the given world position. */
simulated function SetFocusOnLoc
(
	vector			FocusWorldLoc,

	Vector2d		InterpSpeedRange,
	Vector2d		InFocusFOV,
	optional float	CameraFOV,
	optional bool	bAlwaysFocus,
	optional bool	bAdjustCamera,
	optional bool	bIgnoreTrace,
	optional float	FocusPitchOffsetDeg
)
{
	// if replacing a bAdjustCamera focus point with a !bAdjustCamera focus point,
	// do a clear first so the !bAdjustCamera one will work relative to where the first
	// one was at the time of the interruption
	if ( ((LastPitchAdjustment != 0) || (LastYawAdjustment != 0))
		&& !bAdjustCamera
		&& FocusPoint.bAdjustCamera )
	{
		ClearFocusPoint(TRUE);
	}

	FocusPoint.FocusWorldLoc	= FocusWorldLoc;
	FocusPoint.FocusActor		= None;
	FocusPoint.FocusBoneName	= '';

	FocusPoint.InterpSpeedRange	= InterpSpeedRange;
	FocusPoint.InFocusFOV		= InFocusFOV;
	FocusPoint.CameraFOV		= CameraFOV;
	FocusPoint.bAlwaysFocus		= bAlwaysFocus;
	FocusPoint.bAdjustCamera	= bAdjustCamera;
	FocusPoint.bIgnoreTrace		= bIgnoreTrace;
	FocusPoint.FocusPitchOffsetDeg = FocusPitchOffsetDeg;
	bFocusPointSet				= TRUE;

	LastFocusChangeTime			= PlayerCamera.WorldInfo.TimeSeconds;
	LastFocusPointLoc			= GetActualFocusLocation();
	bFocusPointSuccessful		= FALSE;
}

/** Tells camera to focus on the given actor. */
simulated function SetFocusOnActor
(
	Actor			FocusActor,
	Name			FocusBoneName,

	Vector2d		InterpSpeedRange,
	Vector2d		InFocusFOV,
	optional float	CameraFOV,
	optional bool	bAlwaysFocus,
	optional bool	bAdjustCamera,
	optional bool	bIgnoreTrace,
	optional float	FocusPitchOffsetDeg
)
{
	// if replacing a bAdjustCamera focus point with a !bAdjustCamera focus point,
	// do a clear first so the !bAdjustCamera one will work relative to where the first
	// one was at the time of the interruption
	if ( ((LastPitchAdjustment != 0) || (LastYawAdjustment != 0))
		&& !bAdjustCamera
		&& FocusPoint.bAdjustCamera )
	{
		ClearFocusPoint(TRUE);
	}

	FocusPoint.FocusActor		= FocusActor;
	FocusPoint.FocusBoneName	= FocusBoneName;
	FocusPoint.InterpSpeedRange	= InterpSpeedRange;
	FocusPoint.InFocusFOV		= InFocusFOV;
	FocusPoint.CameraFOV		= CameraFOV;
	FocusPoint.bAlwaysFocus		= bAlwaysFocus;
	FocusPoint.bAdjustCamera	= bAdjustCamera;
	FocusPoint.bIgnoreTrace		= bIgnoreTrace;
	FocusPoint.FocusPitchOffsetDeg = FocusPitchOffsetDeg;
	bFocusPointSet				= TRUE;

	LastFocusChangeTime			= PlayerCamera.WorldInfo.TimeSeconds;
	LastFocusPointLoc			= GetActualFocusLocation();
	bFocusPointSuccessful		= FALSE;
}


/** Clear focus point */
simulated function ClearFocusPoint(optional bool bLeaveCameraRotation)
{
	local GearPC GPC;

	bFocusPointSet = FALSE;

	// note that bAdjustCamera must be true to leave camera rotation.
	// otherwise, there will be a large camera jolt as the player and camera
	// realign themselves (which they have to do, since the camera rotated away from
	// the player)
	if ( bLeaveCameraRotation && FocusPoint.bAdjustCamera )
	{
		LastPitchAdjustment = 0;
		LastYawAdjustment = 0;
		LeftoverPitchAdjustment = 0;
		if (PlayerCamera.PCOwner != None)
		{
			PlayerCamera.PCOwner.SetRotation(LastPreModifierCameraRot);
		}
	}

	GPC = GearPC(PlayerCamera.PCOwner);
	if (GPC != None)
	{
		GPC.CameraLookAtFocusActor = None;
	}
}

final simulated protected event UpdateFocusPoint( Pawn P )
{
	local GearPawn	GP;
	local GearPC	GPC;

	GP = GearPawn(P);
	GPC = GearPC(P.Controller);

	if (bDoingACameraTurn)
	{
		// no POIs during camera turns
		ClearFocusPoint();
	}
	else if ( (CurrentCamMode == GearCamDeath) && (GP != None) && (GP.Mesh != None) )
	{
		// note that death-cam POIs take priority over others
		SetFocusOnLoc( GP.GetDeathCamLookatPos(), vect2d(6,6), vect2d(2,2), , TRUE , , TRUE );
	}
	else if( GPC != None && GPC.CameraLookAtFocusActor != None )
	{
		// still need this here, since this function polls the gamestate for focus points.
		// nothing to really do here, though.
	}
	else if( GP != None && GP.IsDoingASpecialMove() && GP.SpecialMoveClasses[GP.SpecialMove].default.bCameraFocusOnPawn )
	{
		// if evading, or doing a special move, then track head of player.
		SetFocusOnActor( GP, GP.NeckBoneName, vect2d(3,3), vect2d(8,11) );
	}
	// give the camera mode a crack at it
	else if ( (CurrentCamMode == None) || (CurrentCamMode.SetFocusPoint(P) == FALSE) )
	{
		// Otherwise, clear focus point
		ClearFocusPoint();
	}

	if (bFocusPointSet)
	{
		// store old focus loc
		LastFocusPointLoc = ActualFocusPointWorldLoc;
		ActualFocusPointWorldLoc = GetActualFocusLocation();
	}

}

/** Internal.  Returns the world space position of the current focus point. */
simulated protected function vector GetActualFocusLocation()
{
	local vector FocusLoc;
	local SkeletalMeshComponent ComponentIt;

	local GearPointOfInterest POI;

	if (FocusPoint.FocusActor != None)
	{
		POI = GearPointOfInterest(FocusPoint.FocusActor);
		if (POI != None)
		{
			// looking at an actual POI, let it tell us where to look
			FocusLoc = POI.GetActualLookatLocation();
		}
		else
		{
			// actor's loc by default
			FocusLoc = FocusPoint.FocusActor.Location;

			// ... but use bone loc if possible
			if (FocusPoint.FocusBoneName != '')
			{
				foreach FocusPoint.FocusActor.ComponentList(class'SkeletalMeshComponent',ComponentIt)
				{
					FocusLoc = ComponentIt.GetBoneLocation(FocusPoint.FocusBoneName);
					break;
				}
			}
		}
	}
	else
	{
		// focused world location, just use that
		FocusLoc = FocusPoint.FocusWorldLoc;
	}

	return FocusLoc;
}


/** use this if you keep the same focus point, but move the camera basis around underneath it */
simulated function AdjustFocusPointInterpolation(rotator Delta)
{
	if (bFocusPointSet && FocusPoint.bAdjustCamera)
	{
		Delta = Normalize(Delta);
		LastYawAdjustment -= Delta.Yaw;
		LastPitchAdjustment -= Delta.Pitch;
	}
}


/**
* Evaluates the current situation and returns the camera mode
* that best matches, ie targeting/crouched/etc.
*
* @return 	  	new camera mode to use
*/
final simulated function GearGameplayCameraMode FindBestCameraMode(Pawn P)
{
	local GearGameplayCameraMode NewCamMode;
	local GearWeapon			W;
	local GearWeap_HeavyBase	HW;
	local bool					bShouldZoom, bGrenade, bSniperZoom;
	local GearPawn				GP;
	local GearVehicle			GearV;
	local GearWeaponPawn		GearWP;
	local GearPawn_LocustBrumakBase GearBrumak;

	GP = GearPawn(P);

	GearWP = GearWeaponPawn(P);
	if(GearWP != None)
	{
		GearV = GearWP.MyVehicle;
	}
	else
	{
		GearV = GearVehicle(P);
	}

	// Look for a brumak
	if(GearV == None)
	{
		GearBrumak = GearPawn_LocustBrumakBase(P);
		if(GearBrumak == None)
		{
			GearBrumak = GearPawn_LocustBrumakBase(P.Base);
		}
	}

	if ( (GP != None) && GP.IsDBNO() )
	{
		NewCamMode = GearCamDBNO;
	}
	else if (GearV != None)
	{
		// target pawn is a vehicle
		if(Vehicle_RideReaver_Base(GearV) != None)
		{
			NewCamMode = GearCamVehicleRideReaver;
		}
		else if ( GearV.bDeadVehicle && (Vehicle_Centaur_Base(GearV) != None) )
		{
			NewCamMode = GearCamCentaurDeath;
		}
		else
		{
			NewCamMode = GearCamVehicle;
		}
	}
	else if(GearBrumak != None)
	{
		// target pawn is a brumak
		if(GearBrumak.Health <= 0)
		{
			NewCamMode = GearCamBrumakDeath;
		}
		else
		{
			if( PlayerCamera.PCOwner.Pawn != None )
			{
				W = GearWeapon(PlayerCamera.PCOwner.Pawn.Weapon);
			}			
			bShouldZoom = ((W != None) && W.ShouldTargetingModeZoomCamera());

			// Driver
			if( GearBrumak == PlayerCamera.PCOwner.Pawn )
			{
				if( bShouldZoom )
				{
					NewCamMode = GearCamBrumakDriverTargeting;
				}
				else
				{
					NewCamMode = GearCamBrumakDriver;
				}
			}
			// Gunner
			else
			{
				if( bShouldZoom )
				{
					NewCamMode = GearCamBrumakGunnerTargeting;
				}
				else
				{
					NewCamMode = GearCamBrumakGunner;
				}
				
			}
		}
	}
	else if ( (P.Health <= 0 || P.bPlayedDeath) && 
			  ( (GP == None) || 
			    ( (GP.SpecialMove != SM_MidLvlJumpOver) && (GP.SpecialMove != SM_RecoverFromRagdoll) ) ) )
	{
		// target pawn is dead
		NewCamMode = GearCamDeath;
	}
	else if (GP != None && ((GP.Physics == PHYS_RigidBody) || GP.IsDoingSpecialMove(SM_RecoverFromRagdoll)) )
	{
		NewCamMode = GearCamRagdoll;
	}
	else if ( GearWeaponPawn(P) != None )
	{
		// manning a vehicle weapon, do we want to make
		// something other than default here?
		NewCamMode = GearWeaponPawn(P).ShouldTargetingModeZoomCamera() ? GearCamVehicleTurretTargeting : GearCamVehicleTurret;
	}
	else if ( GearTurret(P) != None )
	{
		NewCamMode = GearTurret(P).ShouldTargetingModeZoomCamera() ? GearCamTurretTargeting : GearCamTurret;
	}
	else if (bConversationMode)
	{
		NewCamMode = GearCamConversation;
	}
	else
	{
		if (GP != None)
		{
			W = GearWeapon(P.Weapon);
			HW = GearWeap_HeavyBase(W);

			// see if we should enter targeting mode view
			bShouldZoom =	( (W != None) && W.ShouldTargetingModeZoomCamera() &&		// weapon must not be reloaded
				(GP.CoverType == CT_None || GP.IsLeaning() || GP.bDoing360Aiming || GP.IsPoppingUp() ) && // not in cover, or leaning in cover
				!GP.bDoingMirrorTransition );	// not in the middle of a mirror

			bGrenade = (GearWeap_GrenadeBase(W) != None);
			bSniperZoom = (GearWeap_SniperRifle(W) != None) && GP.bIsZoomed;

			if( (HW != None) && (HW.IsMounted() || HW.IsBeingMounted()) )
			{
				// heavy weapons, while propped up, get turret cam for now
				NewCamMode = GearCamMountedHeavyWeapon;
			}
			else if (GP.IsDeployingShield())
			{
				NewCamMode = GearCamDeployedShield;
			}
			else if (GP.SpecialMove == SM_MidLvlJumpOver)
			{
				NewCamMode = GearCamMantle;
			}
			else if( GP.CoverType != CT_None )
			{
				if( bShouldZoom )
				{
					NewCamMode = bSniperZoom ? GearCamSniperZoom : (bGrenade ? GearCamCoverTargetingGrenade : GearCamCoverTargeting);
				}
				else
				{
					NewCamMode = GearCamCover;
				}
			}
			else
			{
				if( bShouldZoom )
				{
					NewCamMode = bSniperZoom ? GearCamSniperZoom : (bGrenade ? GearCamTargetingGrenade : GearCamTargeting);
				}
				else if( GP.bIsCrouched )
				{
					NewCamMode = GearCamCrouch;
				}
				else if( GP.SpecialMove == SM_RoadieRun )
				{
					NewCamMode = GearCamRoadie;
				}
				else
				{
					NewCamMode = (CustomDefaultCamMode != None) ? CustomDefaultCamMode : GearCamDefault;
				}
			}
		}
	}

	if (NewCamMode == None)
	{
		`log("Could not find appropriate camera mode, using GearCamDefault!");
		NewCamMode = GearCamDefault;
	}

	return NewCamMode;
}


/**
* Update Camera modes. Pick Best, handle transitions
*/
final protected function UpdateCameraMode(Pawn P)
{
	local GearGameplayCameraMode	NewCamMode;

	// Pick most suitable camera mode
	NewCamMode = FindBestCameraMode(P);
	//	if( NewCamMode == None )
	//	{
	//		`CamDLog("FindBestCameraMode returned none!!");
	//	}

	// set new warcam
	if( NewCamMode != CurrentCamMode )
	{
		if( CurrentCamMode != None )
		{
			CurrentCamMode.OnBecomeInActive(P, NewCamMode);
		}
		if( NewCamMode != None )
		{
			NewCamMode.OnBecomeActive(P, CurrentCamMode);
		}

		CurrentCamMode = NewCamMode;
	}
}


/**
* Give GearCams a chance to change player view rotation
*/
simulated function ProcessViewRotation( float DeltaTime, Actor ViewTarget, out Rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	/*
	// if player is reloading in cover, disable any input rotation, as we control the camera with SetFocusPoint()
	if( GearPC(PCOwner).IsInCoverState() && GearWeapon(PCOwner.Pawn.Weapon).IsReloading() )
	{
	out_DeltaRot = rot(0,0,0);
	}
	*/

	if( CurrentCamMode != None )
	{
		CurrentCamMode.ProcessViewRotation(DeltaTime, ViewTarget, out_ViewRotation, out_DeltaRot);
	}
}


/**
* Begin using this camera mode as the default (noncover, nontargeting) camera mode.
*/
simulated function StartCustomDefaultCameraMode(EAltDefaultCameraModes Mode)
{
	switch (Mode)
	{
	default:
		CustomDefaultCamMode = None;
	}
}

/**
 * Stop using this camera mode as the default mode.
 */
simulated function ClearCustomDefaultCameraMode()
{
	CustomDefaultCamMode = None;
}

/** Called when Camera mode becomes active */
function OnBecomeActive()
{
	if(!PlayerCamera.bInterpolateCamChanges)
	{
		Reset();
	}
	super.OnBecomeActive();
}

simulated event ModifyPostProcessSettings(out PostProcessSettings PP)
{
	CurrentCamMode.ModifyPostProcessSettings(PP);
}

function ResetInterpolation()
{
	Super.ResetInterpolation();

	LastHeightAdjustment = 0.f;
	LastYawAdjustment = 0.f;
	LastPitchAdjustment = 0.f;
	LeftoverPitchAdjustment = 0.f;
}



defaultproperties
{
	PenetrationBlendOutTime=0.15f
	PenetrationBlendInTime=0.1f
	PenetrationBlockedPct=1.f
	PenetrationExtentScale=1.f

	WorstLocPenetrationExtentScale=1.f
	WorstLocInterpSpeed=8

	bResetCameraInterpolation=TRUE	// set to true by default, so first frame is never interpolated

	OriginOffsetInterpSpeed=8

	Focus_BackOffStrength=0.33f
	Focus_StepHeightAdjustment= 64
	Focus_MaxTries=4
	Focus_FastAdjustKickInTime=0.5

	bDoingACameraTurn=FALSE
	bCamDebug=FALSE

	DirectLookInterpSpeed=6.f
	SpectatorCameraRotInterpSpeed=14.f

	// ray 0 is the main ray
	PenetrationAvoidanceFeelers(0)=(AdjustmentRot=(Pitch=0,Yaw=0,Roll=0),WorldWeight=1.f,PawnWeight=1.f,Extent=(X=14,Y=14,Z=14))

	// horizontally offset
	PenetrationAvoidanceFeelers(1)=(AdjustmentRot=(Pitch=0,Yaw=3072,Roll=0),WorldWeight=0.75f,PawnWeight=0.75f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(2)=(AdjustmentRot=(Pitch=0,Yaw=-3072,Roll=0),WorldWeight=0.75f,PawnWeight=0.75f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(3)=(AdjustmentRot=(Pitch=0,Yaw=6144,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(4)=(AdjustmentRot=(Pitch=0,Yaw=-6144,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))

	// vertically offset
	PenetrationAvoidanceFeelers(5)=(AdjustmentRot=(Pitch=3640,Yaw=0,Roll=0),WorldWeight=1.f,PawnWeight=1.f,Extent=(X=0,Y=0,Z=0))
	PenetrationAvoidanceFeelers(6)=(AdjustmentRot=(Pitch=-3640,Yaw=0,Roll=0),WorldWeight=0.5f,PawnWeight=0.5f,Extent=(X=0,Y=0,Z=0))
}

