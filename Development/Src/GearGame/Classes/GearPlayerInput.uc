/**
 * GearPlayerInput
 * Gear player input post processing
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPlayerInput extends GearPlayerInput_Base
	native
	transient;

/************************************************************************/
/* Member variables, structs, enums, etc.                               */
/************************************************************************/

var config float TargetingModeViewScalePct;

/** Enable view acceleration? */
var() bool bViewAccelEnabled;
/** Acceleration multiplier */
var() float YawAccelMultiplier;
/** Threshold above when Yaw Acceleration kicks in*/
var() float YawAccelThreshold;
/** Time for Yaw Accel to ramp up */
var() float YawAccelRampUpTime;
/** Yaw acceleration percentage applied when fading in or out */
var float YawAccelPct;

/** auto pitch centering if true */
var() config bool	bAutoCenterPitch;
/** Pitch auto centering speed */
var() config float	PitchAutoCenterSpeed;
/** Pitch auto centering speed when roadie running */
var() config float	PitchAutoCenterSpeedRoadieRun;
/** delay before starting auto centering when moving */
var() config float	PitchAutoCenterDelay;
/** count for delay */
var float PitchAutoCenterDelayCount;
/** Offset applied to the autocentering pitch window to enable manipulation of the "horizon", in rotator units. */
var float PitchAutoCenterHorizonOffset;

var transient bool bPlayerInterruptedRoadieRunPitchAutoCentering;

var bool bAppliedFriction;

/**
 * Defines the size of the pitch "window".	Centering will stop/not start if the pitch is in this window .
 * This is a min/max pair of angle values, in rotator units.
 */
var() vector2d PitchAutoCenterTargetPitchWindow;

/**
 * The value at which we should transition from walk to run and back to walk.
 * @see PreProcessInput
 **/
var() config float RunWalkTransitionThreshold;

//debug
var bool bDebugFriction, bDebugAdhesion;
var private float LastDistToTarget, LastDistMultiplier, LastDistFromAimZ, LastDistFromAimY, LastFrictionMultiplier, LastAdhesionAmtY, LastAdhesionAmtZ;
var private float LastTargetWidth, LastTargetHeight, LastDistFromAimYa, LastDistFromAimZa, LastAdjustY, LastAdjustZ;
var private Vector LastCamLoc;
var private Rotator LastDeltaRot;
var private float LastTargVelDotTurnDir, LastCamRotDotTargY;

/** test, velocity bound by joystick */
var() bool bVelocityScaleTest;

/** Last time that the player gave any input */
var transient float LastInputTime;

/** True if pitch is being force-centered, false otherwise. */
var private transient bool	bForcePitchCentering;
/** How fast the camera centers when using forced pitch centering. */
var() private float			ForcePitchCenteringSpeed;
/** Horizon offset to be applied only for this particular instance of forced pitch centering */
var private transient float ForcedPitchCenteringHorizonOffset;
/** True if forced pitch centering can be cancelled by user input. */
var private transient bool	bForcePitchCenteringIsInterruptable;

/** aStrafe value from last tick. */
var float LastStrafe;
/** aForward value from last tick. */
var float LastForward;
/** Cached value of aStrafe from this frame (since the axes get cleared in the low level code) */
var float CachedStrafe;
/** Cached value of aStrafe from this frame (since the axes get cleared in the low level code) */
var float CachedForward;

/** Are we currently recording a demo? */
var bool bRecording;
/** Which recording are on? */
var int DemoCount;


/************************************************************************/
/* Native Script functions                                              */
/************************************************************************/

/** Will return the BindName based on the BindCommand */
native function String GetGearBindNameFromCommand( String BindCommand );

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/** Sets the input handlers for the button handling list - set by subclasses */
event InitializeButtonHandlingHandlers()
{
	InputButtonDataList[GB_A].ButtonHandler					= HandleButtonInput_A;
	InputButtonDataList[GB_X].ButtonHandler					= bUseAlternateControls ? HandleNonCoverMoves : HandleButtonInput_X;
	InputButtonDataList[GB_Y].ButtonHandler					= bUseAlternateControls ? HandleButtonInput_X : HandleButtonInput_Y;
	InputButtonDataList[GB_B].ButtonHandler					= HandleButtonInput_B;
	InputButtonDataList[GB_Start].ButtonHandler				= HandleButtonInput_Start;
	InputButtonDataList[GB_Back].ButtonHandler				= HandleButtonInput_Back;
	InputButtonDataList[GB_LeftTrigger].ButtonHandler		= HandleButtonInput_LeftTrigger;
	InputButtonDataList[GB_LeftBumper].ButtonHandler		= HandleButtonInput_LeftBumper;
	InputButtonDataList[GB_RightTrigger].ButtonHandler		= HandleButtonInput_RightTrigger;
	InputButtonDataList[GB_RightBumper].ButtonHandler		= HandleButtonInput_RightBumper;
	InputButtonDataList[GB_DPad_Up].ButtonHandler			= HandleButtonInput_DPad_Up;
	InputButtonDataList[GB_DPad_Left].ButtonHandler			= HandleButtonInput_DPad_Left;
	InputButtonDataList[GB_DPad_Down].ButtonHandler			= HandleButtonInput_DPad_Down;
	InputButtonDataList[GB_DPad_Right].ButtonHandler		= HandleButtonInput_DPad_Right;
	InputButtonDataList[GB_LeftStick_Push].ButtonHandler	= HandleButtonInput_LeftStickButton;
	InputButtonDataList[GB_RightStick_Push].ButtonHandler	= bUseAlternateControls ? HandleButtonInput_Y : HandleButtonInput_RightStickButton;
}

/** Called when the top most input object is popped off of the stack and now this input object will become first in line. */
function PoppedToTopOfInputStack( GearPlayerInput_Base PrevInputObject, bool bWipePrevInputState )
{
	// If the previous input object was derived from this class, we're going to copy the state of the previous input object
	if ( ClassIsChildOf(PrevInputObject.Class, Class) )
	{
		bWipePrevInputState = false;
	}

	Super.PoppedToTopOfInputStack( PrevInputObject, bWipePrevInputState );
}

/**
 * Pass TRUE to start forced pitch centering, FALSE to stop it.	 Centering will
 * occur until it is complete and then stop -- note it doesn't lock pitch to 0.
 * GoalPitch is the pitch we'll move toward.
 * If bCancelOnUserInput is TRUE, any attempts by the player to change pitch will cancel the pitch centering (to avoid fighting).
 */
simulated final function ForcePitchCentering(bool bCenter, optional bool bCancelOnUserInput, optional float GoalPitch, optional float InterpSpeed)
{
	bForcePitchCentering = bCenter;
	bForcePitchCenteringIsInterruptable = bCancelOnUserInput;
	ForcedPitchCenteringHorizonOffset = GoalPitch;
	ForcePitchCenteringSpeed = (InterpSpeed == 0.f) ? default.ForcePitchCenteringSpeed : InterpSpeed;
}

/**
 * Overridden to add hooks for view acceleration and roadie run.
 */
function PreProcessInput(float DeltaTime)
{
	local GearPawn	P;
	local GearVehicle WV;
	local GearWeaponPawn WP;
	local int SeatIndex;
	local GearWeapon W;
	local bool		bIsDoingASpecialMove;
	local float TurnScale, LookUpDownScale;

	Super.PreProcessInput(DeltaTime);

	bAppliedFriction = FALSE;

	P = GearPawn(Pawn);
	if( P != None )
	{
		bIsDoingASpecialMove = P.IsDoingASpecialMove();
		// Special Moves can affect input
		if( bIsDoingASpecialMove )
		{
			P.SpecialMoves[P.SpecialMove].PreProcessInput(Self);
		}

		// apply view friction
		W = P.MyGearWeapon;
		if (W != None)
		{
			if (P.bIsTargeting)
			{
				ViewFriction(DeltaTime,W);
			}

			W.GetRotationControlScale(TurnScale, LookUpDownScale);
			aTurn *= TurnScale;
			aLookUp *= LookUpDownScale;
		}

		// set run based on how much the stick is pressed
		bRun = (Square(RawJoyUp) + Square(RawJoyRight)) > Square(RunWalkTransitionThreshold) ? 1 : 0;
	}
	else
	{
		// Handle posessing a GearWeaponPawn
		WV = GearVehicle(Pawn);
		if (WV == None)
		{
			WP = GearWeaponPawn(Pawn);
			if (WP != None)
			{
				WV = WP.MyVehicle;
				SeatIndex = WP.MySeatIndex;
			}
		}

		// Got a vehicle
		if(WV != None)
		{
			W = WV.Seats[SeatIndex].Gun;
			if (W != None)
			{
				if (bIsTargeting || WV.bAlwaysViewFriction)
				{
					ViewFriction(DeltaTime, W);
				}

				W.GetRotationControlScale(TurnScale, LookUpDownScale);
				aTurn *= TurnScale;
				aLookUp *= LookUpDownScale;
			}
		}
	}

	// Accelerate Joystick turning rate
	if( bViewAccelEnabled && !bAppliedFriction )
	{
		ViewAcceleration(DeltaTime);
	}
}

/**
 * Overloaded to handle various game features.
 */
event PlayerInput( float DeltaTime )
{
	local vector CamLoc;
	local rotator CamRot;

	// clear the friction flag
	bAppliedFriction = FALSE;

	if (Pawn != None && !Pawn.bCanStrafe && GearVehicle(Pawn) == None && GearWeaponPawn(Pawn) == None)
	{
		aStrafe = 0.f;
	}

	// save some prev values
	LastStrafe = CachedStrafe;
	LastForward = CachedForward;

	Super.PlayerInput(DeltaTime);

	// save cur values
	CachedStrafe = aStrafe;
	CachedForward = aForward;

	// this needs to happen after aForward has been updated.
	if( bAutoCenterPitch && !bAppliedFriction && Pawn != None)
	{
		AutoPitchCentering( DeltaTime );
	}
	// remap the joy inputs
	RemappedJoyRight = RawJoyRight;
	RemappedJoyUp = RawJoyUp;
	if (IsInCoverState())
	{
		GetPlayerViewPoint(CamLoc, CamRot);
		ControlsRemapRotation = CamRot;
		RemapControlsByRotation(Normalize(Pawn.Rotation - ControlsRemapRotation), RemappedJoyRight, RemappedJoyUp);
	}

	if (!IsIdle())
	{
		LastInputTime = WorldInfo.TimeSeconds;
	}

	// This will apply the sensitivity scaling to the controller inputs
	ApplySensitivityScaling();
}

/**
 * Automatic pitch centering.
 */
final function AutoPitchCentering( float DeltaTime )
{
	local float	CurrentPitch, TargetPitch, Delta, PitchWindowOffset;
	local GearPawn GP;
	local vector2d PitchWindow;
	local vector PawnX, PawnY, PawnZ, SurfaceFwd, SurfaceRight, SurfaceUp;
	local rotator SurfaceRot;
	local bool bRoadieRunning;

	CurrentPitch = float(NormalizeRotAxis(Rotation.Pitch));

	GP = GearPawn(Pawn);
	bRoadieRunning = (GP != None) && GP.IsDoingSpecialMove(SM_RoadieRun);

	// if player tries to look around while roadie running, cancel RR-based pitch centering until
	// another roadie run begins
	if (bRoadieRunning)
	{
		if (aLookUp != 0)
		{
			bPlayerInterruptedRoadieRunPitchAutoCentering = TRUE;
		}
	}
	else
	{
		bPlayerInterruptedRoadieRunPitchAutoCentering = FALSE;
	}

	// see if pitch centering was interrupted by the user changing pitch manually
	if ( bForcePitchCenteringIsInterruptable && (aLookUp != 0.f) )
	{
		bForcePitchCentering = FALSE;
	}

	if (!bForcePitchCentering)
	{
		if (IsInCoverState())
		{
			if (aLookUp != 0 ||
				VSize(Pawn.Velocity) < 1.f ||
				bIsTargeting ||
				GP.IsWeaponPendingFire(0))
			{
				PitchAutoCenterDelayCount = 0.f;
				return;
			}
		}
		else
		if( (aLookUp != 0) ||
			(aForward == 0 && aStrafe == 0) ||
			(Pawn != None && VSize(Pawn.Velocity) < 1.f) )
		{
			PitchAutoCenterDelayCount = 0.f;
			return;
		}

		if ( (PitchAutoCenterDelayCount < PitchAutoCenterDelay) && (!bRoadieRunning || bPlayerInterruptedRoadieRunPitchAutoCentering) )
		{
			PitchAutoCenterDelayCount += DeltaTime;
			return;
		}
	}

	// maybe modify pitchwindow (eg if roadie running on slope)
	PitchWindow = PitchAutoCenterTargetPitchWindow;
	if (bRoadieRunning && !bPlayerInterruptedRoadieRunPitchAutoCentering)
	{
		// construct the player's orientation "stuck" to the surface of the ground
		GetAxes(GP.Rotation, PawnX, PawnY, PawnZ);
		SurfaceUp = GP.Floor;
		SurfaceRight = SurfaceUp cross PawnX;
		SurfaceFwd = SurfaceUp cross SurfaceRight;
		SurfaceRot = OrthoRotation(SurfaceFwd, SurfaceRight, SurfaceUp);

		// adjust pitch window accordingly
		PitchWindowOffset = -SurfaceRot.Pitch;
	}

	// apply any horizon adjustments
	PitchWindowOffset += PitchAutoCenterHorizonOffset;
	if (bForcePitchCentering)
	{
		PitchWindowOffset += ForcedPitchCenteringHorizonOffset;
	}

	// calc final pitch window
	PitchWindow.X = NormalizeRotAxis(PitchWindow.X + PitchWindowOffset);
	PitchWindow.Y = NormalizeRotAxis(PitchWindow.Y + PitchWindowOffset);

	if (PitchWindow.Y < CurrentPitch)
	{
		TargetPitch = PitchWindow.Y;
	}
	else if (PitchWindow.X > CurrentPitch)
	{
		TargetPitch = PitchWindow.X;
	}
	else
	{
		// current pitch is in the acceptable window, do nothing;
		return;
	}

	// calc how much to interpolate
	if (bForcePitchCentering)
	{
		Delta = FInterpTo(CurrentPitch, TargetPitch, DeltaTime, ForcePitchCenteringSpeed) - CurrentPitch;
	}
	else if (bRoadieRunning && !bPlayerInterruptedRoadieRunPitchAutoCentering)
	{
		Delta = FInterpTo(CurrentPitch, TargetPitch, DeltaTime, PitchAutoCenterSpeedRoadieRun) - CurrentPitch;
	}
	else
	{
		if (bIsTargeting||
			(WalkVolume(Pawn.PhysicsVolume) != None &&
			 WalkVolume(Pawn.PhysicsVolume).bActive))
		{
			Delta = FInterpTo(CurrentPitch, TargetPitch, DeltaTime, PitchAutoCenterSpeed * 0.5f) - CurrentPitch;
		}
		else
		{
			Delta = FInterpTo(CurrentPitch, TargetPitch, DeltaTime, PitchAutoCenterSpeed) - CurrentPitch;
		}
	}

	// the 5 is arbitrary, chosen because it just feels good
	if (Abs(Delta) < 5.f)
	{
		// finished centering!
		bForcePitchCentering = FALSE;
	}

	aLookup += Delta;
}


final function ViewFriction( float DeltaTime, GearWeapon W )
{
	local Actor FrictionTarget;
	local Vector CamLoc, X, Y, Z, CamToTarget, AimLoc, TargetLoc, RealTargetLoc, TargetCenter;
	local Rotator CamRot;
	local float DistToTarget, DistMultiplier, DistFromAimZ, DistFromAimY;
	local float TargetWidth, TargetHeight;
	local float FrictionMultiplier;
	local bool bTargetMoving;
	local Vector TargX, TargY, TargZ;
	local Vector TurnY;
	local float TargVelDotTurnDir, CamRotDotTargY;

//	local float Time;
//	CLOCK_CYCLES(time);

	if( Pawn == None )
	{
		return;
	}

	// Setup some initial data
	GetPlayerViewPoint( CamLoc, CamRot );
	GetAxes( CamRot, X, Y, Z );

	// Look for a friction target
	FrictionTarget = GetFrictionAdhesionTarget(W.MaxFrictionDistance);

	// If we have a valid friction target
	if( FrictionTarget != None )
	{
		// Grab collision info from target
		FrictionTarget.GetAimFrictionExtent(TargetWidth, TargetHeight, TargetCenter);

		RealTargetLoc = TargetCenter + (W.FrictionTargetOffset >> CamRot);
		CamToTarget = (RealTargetLoc - CamLoc);
		DistToTarget = VSize(CamToTarget);
		AimLoc = CamLoc + (X * DistToTarget);

		// Get directions for turning and target movement
		bTargetMoving = VSize(FrictionTarget.Velocity) > 50.f && Abs(aTurn) > 0;
		GetAxes( Rotator(CamToTarget), TargX, TargY, TargZ );
		TurnY = Y;
		if( aTurn < 0 )
		{
			TurnY = -Y;
			TargY = -TargY;
		}

		//debug
		//DrawDebugLine( FrictionTarget.Location, FrictionTarget.Location + FrictionTarget.Velocity, 255, 0, 0 );
		//DrawDebugLine( Pawn.GetPawnViewLocation(), Pawn.GetPawnViewLocation() + TurnY * 128.f, 255, 0, 0 );
		//DrawDebugLine( Pawn.GetPawnViewLocation(), Pawn.GetPawnViewLocation() + TargY * 128.f, 0, 0, 255 );
		//DrawDebugCylinder(FrictionTarget.Location+vect(0,0,1)*TargetHeight, FrictionTarget.Location-vect(0,0,1)*TargetHeight, TargetWidth, 12, 255, 0, 0);

		// Calculate the aim friction multiplier
		// Y component
		TargetLoc	 = RealTargetLoc;
		TargetLoc.Z	 = AimLoc.Z;
		DistFromAimY = PointDistToLine(AimLoc,(TargetLoc - CamLoc),CamLoc);
		// Z component
		TargetLoc	 = RealTargetLoc;
		TargetLoc.X	 = AimLoc.X;
		TargetLoc.Y	 = AimLoc.Y;
		DistFromAimZ = PointDistToLine(AimLoc,(TargetLoc - CamLoc),CamLoc);

		// Calculate the distance multiplier
		DistMultiplier = 0.f;
		if( DistToTarget >= W.MinFrictionDistance &&
			DistToTarget <= W.MaxFrictionDistance )
		{
			if( DistToTarget <= W.PeakFrictionDistance )
			{
				// Ramp up to peak
				DistMultiplier = FClamp((DistToTarget - W.MinFrictionDistance)/(W.PeakFrictionDistance - W.MinFrictionDistance),0.f,1.f);
			}
			else
			{
				// Ramp down from peak
				DistMultiplier = FClamp(1.f - (DistToTarget - W.PeakFrictionDistance)/(W.MaxFrictionDistance - W.PeakFrictionDistance),0.f,1.f);
			}

			// Scale target radius by distance
			TargetWidth *= 1.f + (W.PeakFrictionRadiusScale * DistMultiplier);
			TargetHeight *= 1.f + (W.PeakFrictionHeightScale * DistMultiplier);
		}

		// If we should apply friction - must be within friction collision box
		if( DistFromAimY < TargetWidth &&
			DistFromAimZ < TargetHeight )
		{
			// Calculate the final multiplier (only based on horizontal turn)
			FrictionMultiplier = GetRangeValueByPct( W.FrictionMultiplierRange, 1.f - (DistFromAimY/TargetWidth) );

			if (GearAimAssistActor(FrictionTarget) != None)
			{
				FrictionMultiplier *= GearAimAssistActor(FrictionTarget).FrictionScale;
			}

			// If target is moving
			if( bTargetMoving )
			{
				TargVelDotTurnDir = Normal(FrictionTarget.Velocity) DOT TurnY;
				CamRotDotTargY = X DOT TargY;
				// If target is moving away from your crosshair and you are trailing him
				if( TargVelDotTurnDir > 0.5f && CamRotDotTargY < 0.f )
				{
					// No friction!
					FrictionMultiplier = 0.f;
				}
			}

			if( FrictionMultiplier > 0.f )
			{
				bAppliedFriction = TRUE;

				// Apply the friction
				aTurn *= FMax((1.f - FrictionMultiplier), 0.f);

				// GAAAs apply friction vertically as well
				if (GearAimAssistActor(FrictionTarget) != None)
				{
					aLookUp *= FMax((1.f - FrictionMultiplier), 0.f);
				}

				// Keep the friction target for possible use with adhesion
				LastFrictionTargetTime	= WorldInfo.TimeSeconds;
				LastFrictionTarget		= FrictionTarget;
			}
		}
	}

	// debug
	LastDistToTarget = DistToTarget;
	LastDistMultiplier = DistMultiplier;
	LastDistFromAimZ = DistFromAimZ;
	LastDistFromAimY = DistFromAimY;
	LastFrictionMultiplier = FrictionMultiplier;
	LastTargetWidth = TargetWidth;
	LastTargetHeight = TargetHeight;
	LastCamLoc = CamLoc;
	LastTargVelDotTurnDir = TargVelDotTurnDir;
	LastCamRotDotTargY = CamRotDotTargY;

	//UNCLOCK_CYCLES(Time);
	//`log("ViewFriction time"@Time);
}


// @see GearPC.ApplyAdhesion
// NOTE: this was reworked to work for GearAimAttractors for the Hydra level.  Prolly needs a little
// love to work in general gamewide cases.
final function ViewAdhesion( float DeltaTime, GearWeapon W, out int out_YawRot, out int out_PitchRot )
{
	local Vector	RealTargetLoc, TargetCenter, CamLoc ;
	local Vector	X, Y, Z;
	local Rotator	CamRot, DeltaRot;
	local float		TargetRadius, TargetHeight;
	local GearPawn	P;
	local GearAimAssistActor GAAA;
	local Actor		AdhesionTarget;

	local vector ClosestPt;
	local float DistFromAim, AdhScale;
	local rotator NewRotation;

	P = GearPawn(Pawn);
	if( W == None ||
		(P != None && P.IsDoingASpecialMove()) ||
		((aTurn == 0.f) && (aLookUp == 0.f)) )
	{
		return;
	}

	// Setup some initial data
	GetPlayerViewPoint( CamLoc, CamRot );
	GetAxes( CamRot, X, Y, Z );

	// attempt to use the friction target if available
	AdhesionTarget = LastFrictionTarget;
	if (AdhesionTarget == None || TimeSince(LastFrictionTargetTime) > W.MaxAdhesionTime)
	{
		// otherwise look for a new target
		AdhesionTarget = GetFrictionAdhesionTarget(W.MaxAdhesionDistance, TRUE);
	}

	// If still within adhesion time constraints, and the target is still alive
	if (AdhesionTarget != None)
	{
		GAAA = GearAimAssistActor(AdhesionTarget);

		// Grab collision info from target
		AdhesionTarget.GetAimAdhesionExtent( TargetRadius, TargetHeight, TargetCenter );
		RealTargetLoc = TargetCenter;	// + (W.FrictionTargetOffset >> CamRot);

		if( ((RealTargetLoc - CamLoc) DOT Vector(CamRot) > 0.f) )
		{
			DistFromAim = PointDistToLine(RealTargetLoc, vector(Rotation), CamLoc, ClosestPt);

			if (DistFromAim < TargetRadius)
			{
				AdhScale = (1.f - (DistFromAim / TargetRadius)) * GAAA.AdhesionScale;
				AdhScale = FMax(AdhScale, 0.5f);
				if (AdhScale > 0.f)
				{
					NewRotation = Rotator(RealTargetLoc - CamLoc);
					NewRotation = RInterpTo(Normalize(Rotation), Normalize(NewRotation), DeltaTime, AdhScale);
					DeltaRot		= Normalize(NewRotation - Rotation);

					//`log("scale"@AdhScale@"turn"@aTurn@DeltaRot.Yaw@"up"@aLookUp@DeltaRot.Pitch);
					//`log("... "@DistFromAim@TargetRadius@1.f - DistFromAim/TargetRadius);

					//// allow pulling away
					//if ( ((aTurn > 0.f) && (DeltaRot.Yaw < 0.f)) || ((aTurn < 0.f) && (DeltaRot.Yaw > 0.f)) )
					//{
					//	`LOG(".. neutered yaw");
					//	DeltaRot.Yaw = 0;
					//}
					//if ( ((aLookUp > 0.f) && (DeltaRot.Pitch < 0.f)) || ((aLookUp < 0.f) && (DeltaRot.Pitch > 0.f)) )
					//{
					//	`LOG(".. neutered pitch");
					//	DeltaRot.Pitch = 0;
					//}
					out_YawRot		+= DeltaRot.Yaw;
					out_PitchRot	+= DeltaRot.Pitch;
				}
			}
		}
	}

	//debug
//	LastAdhesionAmtY = AdhesionAmtY;
//	LastAdhesionAmtZ = AdhesionAmtZ;
//	LastDistFromAimZa = DistFromAimZ;
//	LastDistFromAimYa = DistFromAimY;
	LastDeltaRot = DeltaRot;
//	LastAdjustY = AdjustY;
//	LastAdjustZ = AdjustZ;
}

/**
 * Accelerate Joystick turning rate. (Player control helper).
 *
 * @param	DeltaTime	delta time seconds since last update.
 */
final function ViewAcceleration( float DeltaTime )
{
	// Skip if no point in doing it
	if (!IsLookInputIgnored())
	{
		// If above threshold, accelerate Yaw turning rate
		if( Abs(aTurn) > YawAccelThreshold	)
		{
			if( YawAccelPct < 1.f )
			{
				YawAccelPct += DeltaTime / YawAccelRampUpTime;
			}

			if( YawAccelPct > 1.f )
			{
				YawAccelPct = 1.f;
			}
		}
		else
		{
			// Otherwise ramp down to normal rate
			if( YawAccelPct > 0.f )
			{
				YawAccelPct -= (4.f * DeltaTime) / YawAccelRampUpTime;
			}

			if( YawAccelPct < 0.f )
			{
				YawAccelPct = 0.f;
			}
		}
		if( aTurn != 0.f )
		{
			aTurn *= (1.f + YawAccelMultiplier*YawAccelPct);
		}
	}

	if( bIsTargeting )
	{
		aTurn *= TargetingModeViewScalePct;
	}
}

/**
 * Remap movement controls. Rotates them by DeltaRot rotation.
 *
 * @param	DeltaRot	Delta Rotation used to remap controls.
 * @param	NewRight	new Left/Right raw movement
 * @param	NewUp		new Up/Down raw movement
 */

function RemapControlsByRotation( Rotator DeltaRot, out float NewRight, out float NewUp )
{
	local Vector	Controls, NewControls;

	Controls.X	= RawJoyRight;
	Controls.Y	= RawJoyUp;

	// don't do anything if controls are blank
	if( IsZero(Controls) )
	{
		NewRight	= 0.f;
		NewUp		= 0.f;
		return;
	}

	// rotate by DeltaRot
	DeltaRot.Pitch = 0;
	//@note we can't normalize here since that will delete any -1.f < x < 0 < x < 1.f values
	//NewControls = Normal(Controls >> DeltaRot);
	NewControls = Controls >> DeltaRot;

	NewRight	= NewControls.X;
	NewUp		= NewControls.Y;
	//`log( "Controls:" $ Controls @ "NewControls:" $ NewControls );
}

// DEBUG FUNCTIONS

/** Draw FOV for debugging */
function DebugDrawFOV( vector2D FOV, vector POVLoc, Rotator POVRot, Color FOVColor, float FOVMaxDist )
{
	local vector	FOVAzimDotLoc, FOVAzimOffset, FOVElevDotLoc, FOVElevOffset;
	local vector	DrawOrigin, ViewDirX, ViewDirY, ViewDirZ;
	local float		DistToAzimDotLoc, DistToElevDotLoc;

	GetAxes( POVRot, ViewDirX, ViewDirY, ViewDirZ );

	if( Pawn == None )
	{
		DrawOrigin = POVLoc + ViewDirX*16;;
	}
	else
	{
		DrawOrigin = Pawn.Location;
	}

	// Draw adhesion FOV bounding box
	DistToAzimDotLoc	= FOVMaxDist * Cos(FOV.X*Pi/180.f);
	FOVAzimDotLoc		= POVLoc + ViewDirX * DistToAzimDotLoc;
	FOVAzimOffset		= ViewDirY * Sqrt(FOVMaxDist**2 - DistToAzimDotLoc**2);

	// FOV Left edge
	DrawDebugLine(DrawOrigin, FOVAzimDotLoc - FOVAzimOffset, FOVColor.R, FOVColor.G, FOVColor.B );

	// FOV right edge
	DrawDebugLine(DrawOrigin, FOVAzimDotLoc + FOVAzimOffset, FOVColor.R, FOVColor.G, FOVColor.B );

	DistToElevDotLoc = FOVMaxDist * Cos(FOV.Y*Pi/180.f);
	FOVElevDotLoc	 = POVLoc + ViewDirX * DistToElevDotLoc;
	FOVElevOffset	 = ViewDirZ * Sqrt(FOVMaxDist**2 - DistToElevDotLoc**2);

	// FOV top edge
	DrawDebugLine(DrawOrigin, FOVElevDotLoc + FOVElevOffset, FOVColor.R, FOVColor.G, FOVColor.B );

	// FOV bottom edge
	DrawDebugLine(DrawOrigin, FOVElevDotLoc - FOVElevOffset, FOVColor.R, FOVColor.G, FOVColor.B );

	// Link em
	DrawDebugLine(FOVAzimDotLoc - FOVAzimOffset, FOVElevDotLoc + FOVElevOffset, FOVColor.R, FOVColor.G, FOVColor.B );
	DrawDebugLine(FOVElevDotLoc + FOVElevOffset, FOVAzimDotLoc + FOVAzimOffset, FOVColor.R, FOVColor.G, FOVColor.B );
	DrawDebugLine(FOVAzimDotLoc + FOVAzimOffset, FOVElevDotLoc - FOVElevOffset, FOVColor.R, FOVColor.G, FOVColor.B );
	DrawDebugLine(FOVElevDotLoc - FOVElevOffset, FOVAzimDotLoc - FOVAzimOffset, FOVColor.R, FOVColor.G, FOVColor.B );
}

`if(`notdefined(FINAL_RELEASE))
function DrawHUD( HUD H )
{
	local Vector XAxis, YAxis, ZAxis, /*Dir, */TargLoc, CamLoc;
	local Rotator CamRot;
	local int X, Y;
	local float UnusedW, UnusedH;

	super.DrawHUD( H );

	X = 15;
	Y = 150;

	if( bDebugFriction )
	{
		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Target:"@LastFrictionTarget@"["$LastDistToTarget$"] ["$LastDistMultiplier$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Aim distance (Y/Z): ["$LastDistFromAimY$"] ["$LastDistFromAimZ$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Target collision (R/H): ["$LastTargetWidth$"] ["$LastTargetHeight$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Friction multiplier: ["$LastFrictionMultiplier$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("TargVelDotTurnDir: ["$LastTargVelDotTurnDir$"] CamRotDotTargY ["$LastCamRotDotTargY$"]");
		Y += 15;

		if( LastFrictionTarget != None )
		{
			LastFrictionTarget.GetAimFrictionExtent(UnusedW, UnusedH, TargLoc);

			// Draw line to our friction target (GREEN)
			DrawDebugLine(Pawn.GetPawnViewLocation(),TargLoc,0,255,0);

			GetPlayerViewPoint( CamLoc, CamRot );
			GetAxes( CamRot, XAxis, YAxis, ZAxis );

			// Draw the friction radius around target (BLUE)
			DrawDebugLine( TargLoc + (YAxis * LastTargetWidth) + (ZAxis * LastTargetHeight),
						   TargLoc + (YAxis * LastTargetWidth) - (ZAxis * LastTargetHeight),
						   0,0,255 );
			DrawDebugLine( TargLoc - (YAxis * LastTargetWidth) + (ZAxis * LastTargetHeight),
						   TargLoc - (YAxis * LastTargetWidth) - (ZAxis * LastTargetHeight),
						   0,0,255 );
			DrawDebugLine( TargLoc + (YAxis * LastTargetWidth) + (ZAxis * LastTargetHeight),
						   TargLoc - (YAxis * LastTargetWidth) + (ZAxis * LastTargetHeight),
						   0,0,255 );
			DrawDebugLine( TargLoc + (YAxis * LastTargetWidth) - (ZAxis * LastTargetHeight),
						   TargLoc - (YAxis * LastTargetWidth) - (ZAxis * LastTargetHeight),
						   0,0,255 );
		}
	}

	X = 15;
	Y += 50;

	if( bDebugAdhesion )
	{
		H.Canvas.DrawColor = MakeColor(255, 255, 0, 255);

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Target:"@LastFrictionTarget@"["$LastDistToTarget$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Aim distance (Y/Z): ["$LastDistFromAimYa$"] ["$LastDistFromAimZa$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("DeltaRot:"@LastDeltaRot);
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Adhesion Amt (Y/Z): ["$LastAdhesionAmtY$"] ["$LastAdhesionAmtZ$"]");
		Y += 15;

		H.Canvas.SetPos(X,Y);
		H.Canvas.DrawText("Adhesion Adjust (Y/Z): ["$LastAdjustY$"] ["$LastAdjustZ$"]");
		Y += 15;
	}
}
`endif


/** Return if no detected input this tick, false otherwise */
simulated function bool IsIdle()
{
	if ( (aBaseX != 0) ||
		 (aBaseY != 0) ||
		 (aBaseZ != 0) ||
		 (aMouseX != 0) ||
		 (aMouseY != 0) ||
		 (aForward != 0) ||
		 (aTurn != 0) ||
		 (aStrafe != 0) ||
		 (aUp != 0) ||
		 (aLookUp != 0) )
	{

		return FALSE;
	}

	return TRUE;
}

/** Whether the button name is the POI button */
function bool IsPOIButtonName(Name ButtonName)
{
	if (bUseAlternateControls)
	{
		return (ButtonName == 'R3');
	}
	else
	{
		return (ButtonName == 'Y');
	}
}


/** GAMEPLAY BUTTON INPUT HANDLING */

/**
 * Checks all the cases for button input fitering, called solely by ButtonPress()/ButtonRelease().
 *
 * @return TRUE if the input should be filtered (ignored)
 */
function bool FilterButtonInput(Name ButtonName,bool bPressed,int ButtonIdx)
{
	local SeqEvt_Input InputEvt;
	local bool bTrappedInput;
	local GearWeapon Weap;

	if ( IsPOIButtonName(ButtonName) && !bPressed && bLookingAtPointOfInterest )
	{
		//@fixme - explicitly don't filter this button.  hacky, yes.
		return FALSE;
	}
	// Never filter the start button so we can always pause
	// never filter back so we can always skip
	else if ( ButtonName == 'Start' || ButtonName == 'Back' )
	{
		return FALSE;
	}

	// check to see if global input has been disabled (UI scene, etc)
	if (InputIsDisabledCount != 0)
	{
		//`log("input disabled, ignoring:"@ButtonName@bPressed);
		return TRUE;
	}
	if (MyGearPawn != None)
	{
		if (bPressed)
		{
			// no button input during a conversation except X (skip) or Y (poi)
			if ( MyGearPawn.bIsConversing && (ButtonName != 'Back') && !IsPOIButtonName(ButtonName) )
			{
			//`log("conversing, ignoring:"@ButtonName@bPressed);
				return TRUE;
			}

			// check to see if the current weapon would trap the input
			if( MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.HandleButtonPress(ButtonName))
			{
				//`log("weapon trap, ignoring:"@ButtonName@bPressed);
				return TRUE;
			}
			// check to see if the current special move would trap the input
			if (MyGearPawn.SpecialMove != SM_None &&
				MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove] != None &&
				MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].ButtonPress(ButtonName))
			{
				//`log("sm trap, ignoring:"@ButtonName@bPressed);
				return TRUE;
			}
		}
		else
		{
			// check to see if the current weapon would trap the input
			if( MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.HandleButtonRelease(ButtonName))
			{
				//`log("weapon trap, ignoring:"@ButtonName@bPressed);
				return TRUE;
			}
		}
	}
	else // then MyGearPawn was none.. try to see if we have a gear vehicle
	{
		if (GearVehicleBase(Pawn) != None || GearTurret(Pawn) != None)
		{
			Weap = GearWeapon(Pawn.Weapon);
			if (Weap != None)
			{
				if ( (bPressed && Weap.HandleButtonPress(ButtonName)) ||
					 (!bPressed && Weap.HandleButtonRelease(ButtonName)) )
				{
					return TRUE;
				}
			}
		}
	}

`if(`notdefined(FINAL_RELEASE))
	if ( (bDebugGUDBrowser || bDebugEffortBrowser) && bPressed )
	{
		if (GearGame(WorldInfo.Game).UnscriptedDialogueManager.HandleButtonPress(ButtonName))
		{
			return TRUE;
		}
	}
`endif

	// check to see if there are any Kismet events trapping this input
	foreach InputEvents(InputEvt)
	{
		if (InputEvt.CheckInputActivate(EGameButtons(ButtonIdx),bPressed) && InputEvt.bTrapInput)
		{
			bTrappedInput = TRUE;
		}
	}

	// input was trapped by one or more events so filter out for gameplay
	if (bTrappedInput)
	{
		//`log("kismet trap, ignoring:"@ButtonName@bPressed);
		return TRUE;
	}

	// all clear, don't filter
	return FALSE;
}


/**
 * Entry point for button press handling.
 */
exec function ButtonPress(coerce Name ButtonName)
{
	LastInputTime = WorldInfo.TimeSeconds;

	Super.ButtonPress( ButtonName );
}

/**
 * Entry point for button release handling.
 */
exec function ButtonRelease(coerce Name ButtonName)
{
	LastInputTime = WorldInfo.TimeSeconds;

	Super.ButtonRelease( ButtonName );
}

/**
* Perform all of the encompassing actions associated with pressing the action button
*	@param bRunActions - only perform roadie run and evades (for alternate input config)
*/
function HandleActionButton(bool bPressed, optional bool bDblClickMove, optional bool bRunActions)
{
	local CovPosInfo CoverCanRunTo;
	local int NoCameraAutoAlign;

	if (bPressed)
	{
		if (IsSpectating())
		{
		}
		// if in reviving then allow button mashing for speed boost
		else if (Outer.IsInState('Reviving'))
		{
			if ( !bRunActions || !bUseAlternateControls  )
			{
				PressedActionButton_Reviving();
			}
		}
		else if( Outer.IsInState('PlayerBrumakGunner') )
		{
			if( !bRunActions || !bUseAlternateControls )
			{
			}
		}
		else if ( Outer.IsInState('PlayerDriving') )
		{
			// Vehicle code handles 'A' button
		}
		else if ( Outer.IsInState('Engaging') )
		{
			if ( !bRunActions || !bUseAlternateControls )
			{
				Outer.ServerHandleButtonPress( 'A' );
			}
		}
		else if (IsTimerActive('PickingUpAWeapon'))
		{
			// do nothing while picking up a weapon
		}
		// door kick
		else if (!IsDoingASpecialMove() && !IsInCoverState() && DoorTriggerDatum.bInsideDoorTrigger)
		{
			if ( !bRunActions || !bUseAlternateControls )
			{
				DoDoorOpen(SM_DoorKick);
			}
		}
		else if ( DoubleClickDir == DCLICK_Active )
		{
			if ( !bRunActions || !bUseAlternateControls )
			{
				if ( !TryToRunToCover(true) && !bUseAlternateControls )
				{
					TryToEvade();
				}
			}
			else if ( bUseAlternateControls && bRunActions )
			{
				TryToEvade();
			}
		}
		// default special move
		else
		{
			if ( !TryASpecialMove(bRunActions) )
			{
				if ( !bRunActions && bUseAlternateControls && !IsInCoverState() && CanTryToRunToCover() )
				{
					if ( CanRunToCover(CoverCanRunTo, NoCameraAutoAlign) )
					{
						if ( IsDoingSpecialMove(SM_RoadieRun) )
						{
							EndSpecialMove();
						}

						TryToRunToCover();
					}
				}
			}
		}
	}
	else
	{
		if (IsSpectating())
		{
		}
		// abort cover/roadie run on button release
		else if ( (bRunActions || !bUseAlternateControls) && (IsDoingSpecialMove(SM_CoverRun) || IsDoingSpecialMove(SM_RoadieRun)) )
		{
			EndSpecialMove();
		}
		else if (IsInCoverState())
		{
			if ( IsTimerActive('TryToRoadieRun') && TryToEvade())
			{
				// do nothing else in this case
			}
			else
			// we only want to push out of cover when we release the button as it feels better release and your guy releases from cover
			// check for a break from cover
			if( bPressAToBreakFromCover && !bBreakFromCover && Abs(RemappedJoyRight) < DeadZoneThreshold && Abs(RemappedJoyUp) < DeadZoneThreshold )
			{
				if ( !bRunActions || !bUseAlternateControls )
				{
					if (CanDoSpecialMove(SM_PushOutOfCover) && TimeSince(MyGearPawn.LastCoverTime) > TimeBeforeAButtonPressLeavesCover)
					{
						DoSpecialMove(SM_PushOutOfCover);
					}
				}
			}
		}
		else
		{
			if( IsTimerActive('TryToRoadieRun') )
			{
				ClearTimer('TryToRoadieRun');
				// Evade has to be included in the CanTryToRunToCover() check, otherwise it leads to some retard rolls.
				if ( CanTryToRunToCover() )
				{
					// if not already doing a special move
					if( (!bUseAlternateControls || !bRunActions) && !IsInCoverState() && TryToRunToCover() )
					{
						// run2cover move
					}
					else if ( !bUseAlternateControls || bRunActions )
					{
						// Otherwise, try to do an evade move
						TryToEvade( bDblClickMove ? CurrentDoubleClickDir : DCLICK_None );
					}
				}
			}
			else if( MyGearPawn.IsCarryingShield() && MyGearPawn.IsDoingSpecialMove(SM_DeployShield) && MyGearPawn.EquippedShield.CanDeployShield() )
			{
				DoSpecialMove(SM_PlantShield, TRUE);
			}
		}

		// Reset timer and mark move as done
		FinishDoubleClick( FALSE );
	}
}

function DelayedEnterScreenshotMode()
{
	ConsoleCommand("ScreenshotMode");
}

/**
 * Handle roadie running and evading
 * (used by alternate controls to pull the above functionality out of the HandleButtonInput_A() function.
 */
function HandleNonCoverMoves(bool bPressed, optional bool bDblClickMove)
{
	HandleActionButton(bPressed, bDblClickMove, TRUE);
}

/**
 * Button A Handler
 */
function HandleButtonInput_A(bool bPressed, optional bool bDblClickMove)
{
	HandleActionButton(bPressed, bDblClickMove, FALSE);
}

function SkipConversation()
{
	local int PlayerIndex;
	if (Role == ROLE_Authority && (!IsSplitscreenPlayer(PlayerIndex) || PlayerIndex == 0))
	{
		// conversation abort (host only)
		if (bHavingAnAbortableConversation)
		{
			`log("aborting conversation");
			ServerAbortConversation();
		}
		else
		{
			`log("skipping cinematic");
			ConsoleCommand("CANCELCINE 2.0");
		}
	}
}

/**
 * Button X Handler
 */
function HandleButtonInput_X(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if( Outer.IsInState('PlayerBrumakDriver') ||
			Outer.IsInState('PlayerBrumakGunner') )
		{
			// do nothing
		}
		else if (Outer.IsInState('PlayerDriving'))
		{
			if (GearVehicleBase(Pawn) != None)
			{
				GearVehicleBase(Pawn).ServerPressedX();
			}
		}
		else if ( Outer.IsInState('Engaging') )
		{
			Outer.ServerHandleButtonPress( 'X' );
		}
		// if we're already pushing an object
		else if (MyGearPawn != None && MyGearPawn.IsDoingSpecialMove(SM_PushObject))
		{
			// then keep pushing
			GSM_PushObject(MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove]).ButtonPressed();
			// tell server to do the same
			ServerKeepPushing();
		}
		// attempt to interact with an object
		else
		{
			if ( !ClientUse() )
			{
				ServerUse();
			}
		}
	}
}

/**
 * Button Y Handler
 */
function HandleButtonInput_Y(bool bPressed, optional bool bDblClickMove)
{
	if( bPressed )
	{
		// Check if we have a DBNO player around us, and we can do a finishing move!
		if( CanDoSpecialMove(SM_CQCMove_PunchFace) )
		{
			ServerDoCQCMove(GB_Y);
			return;
		}
		else
		// don't allow a force look while door kicking 1) you shouldn't be able to 2) it can break the door kick
		// also not while chainsawing.	also not while on a turret
		if ( (MyGearPawn == None) || (MyGearPawn.SpecialMove == SM_None || !MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bDisablePOIs) && 
			 (ForcedAdhesionTarget == None) )
		{
			TryPOILookAt();
		}
	}
	else
	{
		TryPOILookAway();
	}
}

/**
 * Button B Handler
 */
function HandleButtonInput_B(bool bPressed, optional bool bDblClickMove)
{
	local Trigger_ChainsawInteraction TCI;
	if( bPressed )
	{
		if( MyGearPawn != None && MyGearPawn.bInDuelingMiniGame )
		{
			// Play a feedback sound
			MyGearPawn.MyGearWeapon.PlaySound(MyGearPawn.MyGearWeapon.FireNoAmmoSound, FALSE, TRUE,, MyGearPawn.Location);
			// Replicate button press to server.
			MyGearPawn.ReportDuelingMiniGameButtonPress();
		}
		else if( Outer.IsInState('PlayerBrumakDriver') )
		{
			foreach MyGearPawn.TouchingActors( class'Trigger_ChainsawInteraction', TCI )
			{
				break;
			}

			if( TCI != None )
			{
				DoSpecialMove( GSM_Brumak_OverlayLftArmSwing );
			}
			else
			{
				DoSpecialMove( GSM_Brumak_OverlayRtArmSwing );
			}
		}
		else
		if( Outer.IsInState('PlayerBrumakGunner') )
		{

		}
		else
		if (Outer.IsInState('Reviving'))
		{
		}
		// start a melee attack
		else
		{
			MyGearPawn.StartMeleeAttack();
		}
	}
	else
	{
		// abort out of any melee attacks
		MyGearPawn.StopMeleeAttack();
	}
}

/**
 * Button Start Handler
 */
function HandleButtonInput_Start(bool bPressed, optional bool bDblClickMove)
{
	if (!bPressed)
	{
		ConditionalPauseGame();
	}
}

/**
 * Button Back Handler
 */
function HandleButtonInput_Back(bool bPressed, optional bool bDblClickMove)
{
	if (MyGearHud != None)
	{
		MyGearHud.ToggleScoreboard(bPressed);
	}

	if (bPressed)
	{
		SkipConversation();
	}
}

/**
 * Button LeftTrigger Handler
 */
function HandleButtonInput_LeftTrigger(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if( MyGearPawn != None && MyGearPawn.bInTriggerMiniGame )
		{
			MyGearPawn.ReportTriggerPressMiniGame(FALSE);
		}
		else if (Outer.IsInState('Dead'))
		{
			if (WorldInfo.GRI != None && WorldInfo.GRI.IsMultiplayerGame())
			{
				TransitionToSpectate('Spectating');
			}
		}
		else if (IsSpectating())
		{
			// Handled by GearUISceneSpectator
		}
		else if (Outer.IsInState('PlayerDriving'))
		{
			VehicleReversePressedAmount = 1.f;
		}
	}
	else
	{
		SetLastWeaponInfoTime(WorldInfo.TimeSeconds);

		// OMG this is so horrible. don't centralize everything here.
		if( Outer.IsInState('PlayerDriving') )
		{
			VehicleReversePressedAmount = 0.f;
		}
	}
}

function DelayedToggleDemoRecording()
{
	if (!WorldInfo.GRI.IsMultiplayerGame())
	{
		if (bRecording)
		{
			ConsoleCommand("DEMOSTOP");
			bRecording = FALSE;
			ClientMessage("Stopped recording demo");
		}
		else
		{
			ConsoleCommand("DEMOREC"@GetURLMap()$"_"$DemoCount);
			DemoCount++;
			bRecording = TRUE;
			ClientMessage("Started recording demo");
		}
	}
}

/**
 * Button LeftBumper Handler
 */
function HandleButtonInput_LeftBumper(bool bPressed, optional bool bDblClickMove)
{
	if ( !Outer.IsSpectating() )
	{
		if (bPressed)
		{
			EnableAssessMode();
			// if driving then release the gas
			if (Outer.IsInState('PlayerDriving'))
			{
				VehicleGasPressedAmount = 0.f;
				VehicleReversePressedAmount = 0.f;
			}
		}
		else
		{
			DisableAssessMode();
			if ( IsButtonActive(GB_RightTrigger, true) )
			{
				HandleButtonInput_RightTrigger( true );
			}
		}
	}
}

/**
 * Button RightTrigger Handler
 */
function HandleButtonInput_RightTrigger(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if( MyGearPawn != None && MyGearPawn.bInTriggerMiniGame )
		{
			MyGearPawn.ReportTriggerPressMiniGame(TRUE);
		}
		else if (Outer.IsInState('PlayerWaiting') && IsCoop())
		{
			ServerRestartPlayer();
		}
		else if (IsSpectating())
		{
			// Handled by GearUISceneSpectator
		}
		else if (Outer.IsInState('Reviving'))
		{
			if (GearWeap_GrenadeBase(Pawn.Weapon) != None && MyGearPawn.MyGearWeapon.HasAmmo(0) && MyGearPawn.MyGearWeapon.IsInState('Active'))
			{
				GearWeap_GrenadeBase(Pawn.Weapon).ArmGrenade();
				if ( MyGearHUD != None )
				{
					MyGearHUD.ClearActionInfoByType(AT_SuicideBomb);
					MyGearHUD.SetActionInfo(AT_StayingAlive, ActionStayingAlive);
					SetReviveIconAnimationSpeed();
				}
			}
			else
			{
				PressedRightTrigger_Reviving();
			}
		}
		else if (Outer.IsInState('Dead'))
		{
			StartFire();
		}
		else if (Outer.IsInState('PlayerDriving'))
		{
			Pawn.StartFire( 0 );
		}
		else
		{
			bFire = 1;
		}
	}
	else
	{
		bFire = 0;
		StopFire(0);
		SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
	}
}

/**
 * Button RightBumper Handler
 */
function HandleButtonInput_RightBumper(bool bPressed, optional bool bDblClickMove)
{
	local GearVehicleBase GV;
	local GearWeapon GW;

	if (bPressed)
	{
		if (Outer.IsSpectating())
		{
		}
		else if (MyGearPawn != None && MyGearPawn.MyGearWeapon != None)
		{
			MyGearPawn.MyGearWeapon.ForceReload();
		}
		else if (MyGearPawn == None)
		{
			GV = GearVehicleBase(Pawn);
			if (GV != None)
			{
				GW = GearWeapon(GV.Weapon);
				if (GW != None)
				{
					GW.ForceReload();
				}
			}
		}
	}
	else
	{
		SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
	}
}

/**
 * Button DPad Handler
 */
function HandleButtonInput_DPad(EGameButtons Button, bool bPressed, optional bool bDblClickMove)
{
	// all DPad buttons do the same behavior, sorted out in these 2 functions
	if (bPressed)
	{
		PressedDPad(Button);
	}
	else
	{
		ReleasedDPad(Button);
	}
}

/** These handlers exist so that we can pass along the button that was pressed for better response. */
function HandleButtonInput_DPad_Up(bool bPressed, optional bool bDblClickMove) { HandleButtonInput_DPad(GB_DPad_Up,bPressed,bDblClickMove); }
function HandleButtonInput_DPad_Left(bool bPressed, optional bool bDblClickMove) { HandleButtonInput_DPad(GB_DPad_Left,bPressed,bDblClickMove); }
function HandleButtonInput_DPad_Down(bool bPressed, optional bool bDblClickMove) { HandleButtonInput_DPad(GB_DPad_Down,bPressed,bDblClickMove); }
function HandleButtonInput_DPad_Right(bool bPressed, optional bool bDblClickMove) { HandleButtonInput_DPad(GB_DPad_Right,bPressed,bDblClickMove); }

/**
 * Button LeftStickButton Handler
 */
function HandleButtonInput_LeftStickButton(bool bPressed, optional bool bDblClickMove)
{
	if (bPressed)
	{
		if (IsInCoverState())
		{
			// crouching -> standing
			MyGearPawn.ShouldCrouch(!MyGearPawn.bWantsToCrouch);
		}
		else
		if( Outer.IsInState('PlayerBrumakGunner') )
		{
		}
		else
		if (GearVehicle(Pawn) != None)
		{
		   PlayVehicleHorn();
		}
	}
}

/**
 * Button RightStickButton Handler
 */
function HandleButtonInput_RightStickButton(bool bPressed, optional bool bDblClickMove)
{
	if( bPressed )
	{
		if( Outer.IsInState('PlayerBrumakDriver') )
		{
			ServerPlayBrumakRoar();
		}
	}
}

defaultproperties
{
	bEnableFOVScaling=TRUE
	bVelocityScaleTest=TRUE

	bViewAccelEnabled=TRUE
	YawAccelThreshold=0.95f
	YawAccelMultiplier=2.5f
	YawAccelRampUpTime=1.3f

	PitchAutoCenterTargetPitchWindow=(X=0,Y=0)

	ForcePitchCenteringSpeed=3.f

	// This PlayerInput will send kismet events if needed.
	bActivateKismetInputEvents=true

	//debug
	//bDebugFriction=TRUE
}
