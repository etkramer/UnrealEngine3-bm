/**
 * Troika Cabal turret
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret_TroikaBase extends GearTurret
	config(Pawn)
	native;

/** Spotter: defines target to shoot at */
var	GearPawn	Spotter;
/** Loader: reloads Troika */
var	GearPawn Loader;

/** Rotation of controller on turret bone (smoothed aiming) */
var rotator TurretControlRot;

/** Normal gunner track speed */
var()	float	TrackSpeed;
/** Speed of tracking when gunner has a spotter */
var()	float	TrackSpeedWithSpotter;
/** Speed of searching */
var()	float	SearchSpeed;

/** Target is too close, abandon gun */
var()	float	TooCloseDist;

/** Current marked location by spotter */
var		Vector	SpottedLoc;
/** Desired marked location by spotter */
var		Vector	DesiredSpottedLoc;

/** Turret look at controllers */
var SkelControlLookAt	Pivot_Latitude, Pivot_Longitude;

var	Name	Pivot_Latitude_BoneName;
var Name	MuzzleSocketName, PlayerRefSocketName;

/** Animation node playing the magazine spinning animation. */
var AnimNodeSequence	SpinSeqNode;

var float	EntryRadius;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	// Cache animation nodes.
	CacheAnimNodes();
}

simulated function Destroyed()
{
	// Clear references to animations nodes
	ClearAnimNodes();

	Super.Destroyed();
}

simulated protected function CacheAnimNodes()
{
	Pivot_Latitude	= SkelControlLookAt(Mesh.FindSkelControl('Pivot_Latitude'));
	Pivot_Longitude = SkelControlLookAt(Mesh.FindSkelControl('Pivot_Longitude'));
	SpinSeqNode		= AnimNodeSequence(Mesh.FindAnimNode('SpinSeqNode'));
}


/** Clear references to anim nodes. */
simulated function protected ClearAnimNodes()
{
	Pivot_Latitude	= None;
	Pivot_Longitude	= None;
	SpinSeqNode		= None;
}

simulated event vector GetEntryLocation()
{
	local vector	EntryLoc;
	local rotator	EntryRot;

	// Get location of player reference socket.
	Mesh.GetSocketWorldLocationAndRotation(PlayerRefSocketName, EntryLoc, EntryRot);
	EntryLoc.Z += 32.f;

	return EntryLoc ;
}

simulated function bool CanEnterVehicle(Pawn P)
{
	return (VSize(P.Location - GetEntryLocation()) <= EntryRadius &&
			Super.CanEnterVehicle(P));
}

simulated function bool IsWithinRotationClamps(vector Loc)
{
	local rotator rot;
	local vector FireLoc;

	FireLoc			= GetPhysicalFireStartLoc( vect(0,0,0) );
	rot				= Rotator(Loc - FireLoc);
	return !TurretClampYaw(rot);
}

/************************************************************************************
 * AI
 ***********************************************************************************/

/**
 * Perform update for AI controller.
 * tracks targets and such...
 */
function UpdateAIController(GearAI_Cover C, float DeltaTime)
{
	local float		InterpSpeed, DistToTarg;
	local Vector	TargLoc, FireLoc;
	`if(`notdefined(FINAL_RELEASE))
	local Rotator R;
	`endif

	// by default, slowest speed
	InterpSpeed = SearchSpeed;

	// if we have a spotter, track faster what he's tracking
	if( Spotter != None )
	{
		InterpSpeed		= TrackSpeedWithSpotter;
	}

	// if we have a visible target, track it down
	if( C.HasValidTarget() )
	{
		InterpSpeed		= TrackSpeed;
		TargLoc			= C.GetFireTargetLocation();


		// If distance to target is inside our fire offset range
		DistToTarg = VSize(Location-TargLoc);
		if( C.FireTarget != self && DistToTarg < TooCloseDist )
		{
			// Notify AI that enemy is too close - abandon gun
			C.TargetInsideFireArc();
			return;
		}

		FireLoc			= GetPhysicalFireStartLoc( vect(0,0,0) );
		DesiredAimDir	= Rotator(TargLoc - FireLoc);
	}
	// keep desired aim within limits
	if( TurretClampYaw(DesiredAimDir) )
	{
		`if(`notdefined(FINAL_RELEASE))
		//debug
		if( bDebugTurret )
		{
			FlushPersistentDebugLines();
			DrawDebugBox(TargLoc, vect(10,10,10), 255, 0, 0, TRUE );
			DrawDebugBox(GetPhysicalFireStartLoc( vect(0,0,0) ), vect(10,10,10), 255, 0, 0, TRUE );
			DrawDebugLine(TargLoc, GetPhysicalFireStartLoc( vect(0,0,0) ), 255, 0, 0, TRUE );
			DrawDebugLine(GetPhysicalFireStartLoc( vect(0,0,0) ) + Vector(Rotation)*256.f, GetPhysicalFireStartLoc( vect(0,0,0) ), 0, 255, 0, TRUE );
			R = Rotation;
			R.Yaw += YawLimit.Y;
			DrawDebugLine(GetPhysicalFireStartLoc( vect(0,0,0) ) + Vector(R)*256.f, GetPhysicalFireStartLoc( vect(0,0,0) ), 0, 255, 0, TRUE );
			R = Rotation;
			R.Yaw += YawLimit.X;
			DrawDebugLine(GetPhysicalFireStartLoc( vect(0,0,0) ) + Vector(R)*256.f, GetPhysicalFireStartLoc( vect(0,0,0) ), 0, 255, 0, TRUE );
		}
		`endif
		TurretOverRotated();
	}

	// Interpolate to the desired aim direction
	AimDir = RInterpTo( AimDir, DesiredAimDir, DeltaTime, InterpSpeed );

	// Update controllers rotation
	C.DesiredRotation = AimDir;
}


/**
 * Called every second when AI is Idle.
 */
function AIIdleNotification(GearAI_Cover C)
{
	// pick a random orientation only if not guided by spotter
	if( (Spotter == None || !bSpottedSomething) && FRand() < 0.33 )
	{
		DesiredAimDir = GetRandomRotForTurretScan(C);
	}
}

function bool ClaimTurret( Controller C )
{
	local GearAI_Cover AI;

	if( super.ClaimTurret( C ) )
	{
		AI = GearAI_Cover(C);
		if( AI != None )
		{
			AI.CurrentTurret = self;
			AI.bForceDesiredRotation = TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

function bool UnclaimTurret( Controller C )
{
	local GearAI_Cover AI;

	if( super.UnclaimTurret( C ) )
	{
		AI = GearAI_Cover(C);
		if( AI != None )
		{
			AI.CurrentTurret = None;
			AI.bForceDesiredRotation = FALSE;
		}

		return TRUE;
	}

	return FALSE;
}


/************************************************************************************
 * Spotter
 ***********************************************************************************/

/**
 * Returns TRUE if player can be a spotter for this Troika.
 */
function bool CanBeSpotter(GearAI_Cover C)
{
	// only allow one spotter per troika
	if( Spotter == None &&
		GearPawn(C.Pawn) != None )
	{
		return TRUE;
	}

	return FALSE;
}


/**
 * Called when a new AI guy become the spotter for the Troika
 */
function bool ClaimSpotter( GearAI_Cover AI )
{
	if( CanBeSpotter( AI ) )
	{
		Spotter = GearPawn(AI.Pawn);
		bStasis	= FALSE;
		AI.CurrentTurret = self;

		return TRUE;
	}

	return FALSE;
}

function bool UnclaimSpotter( GearAI_Cover AI )
{
	if( Spotter == AI.Pawn ||
		(Spotter != None &&
			(Spotter.Controller == None || Spotter.Health <= 0)))
	{
		Spotter = None;
		bSpottedSomething = FALSE;
		AI.CurrentTurret = None;

		if( Claim == None )
		{
			bStasis = TRUE;
		}

		return TRUE;
	}

	return FALSE;
}

function StartSpotting()
{
	local GearAI_Cover AI;

	AI = GearAI_Cover(Spotter.Controller);
	if( AI != None )
	{
		bSpottedSomething = TRUE;
		AI.Focus = AI.FireTarget;
	}
}

function StopSpotting()
{
	bSpottedSomething = FALSE;
	Spotter.Controller.Focus = None;
}

simulated function Tick(float DeltaTime)
{
	super.Tick( DeltaTime );

	UpdateTurret(DeltaTime);

	if( Driver != None )
	{
		UpdateDriver(DeltaTime);
	}
}

simulated event rotator GetDriverIdealRotation()
{
	return TurretControlRot;
}

simulated function UpdateTurret(float DeltaTime)
{
	local vector AimTargetWorld, AimTargetLoc, PivotLatLoc;

	// Get location of latitude pivot
	PivotLatLoc = Mesh.GetBoneLocation(Pivot_Latitude_BoneName);
	AimTargetWorld = PivotLatLoc + Vector(GetTurretAimDir()) * 16384;
	AimTargetLoc = (AimTargetWorld - Location) << Rotation;		// the skelcontrols expect actor-local space

	if( Pivot_Latitude != None )
	{
		Pivot_Latitude.TargetLocation = (WorldInfo.NetMode != NM_Client || IsLocallyControlled()) ? AimTargetLoc : VInterpTo(Pivot_Latitude.TargetLocation, AimTargetLoc, DeltaTime, 2.0);
	}
	if( Pivot_Longitude != None )
	{
		Pivot_Longitude.TargetLocation = (WorldInfo.NetMode != NM_Client || IsLocallyControlled()) ? AimTargetLoc : VInterpTo(Pivot_Longitude.TargetLocation, AimTargetLoc, DeltaTime, 2.0);
	}

	// Use smooth location for pawn rotation as well
	TurretControlRot = rotator(Pivot_Latitude.TargetLocation >> Rotation);

	if( SpinSeqNode != None )
	{
		if( FiringMode == class'GearWeapon'.const.FIREMODE_CHARGE || IsFiring() )
		{
			// If wasn't firing, make sure we start from zero.
			if( !SpinSeqNode.bPlaying )
			{
				SpinSeqNode.Rate = 0.f;
			}

			SpinSeqNode.bPlaying = TRUE;
			SpinSeqNode.Rate = FMin(SpinSeqNode.Rate + DeltaTime, 1.f);
		}
		else if( SpinSeqNode.bPlaying )
		{
			SpinSeqNode.Rate = FInterpTo(SpinSeqNode.Rate, 0.f, DeltaTime, 2.f);
			//SpinSeqNode.Rate = FMax(SpinSeqNode.Rate * (1.f - DeltaTime), 0.f);

			if( SpinSeqNode.Rate <= 0.001f )
			{
				SpinSeqNode.bPlaying = FALSE;
			}
		}
	}
}

function bool PlaceExitingDriver(optional Pawn Who)
{
	// Do not re position driver when exiting.
	return TRUE;
}


/**
 * Player just changed weapon
 * Network: Server, Local Player
 *
 * @param	OldWeapon	Old weapon held by Pawn.
 * @param	NewWeapon	New weapon held by Pawn.
 */
simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	local GearWeapon	OldWeap, NewWeap;

	Super.PlayWeaponSwitch(OldWeapon, NewWeapon);

	OldWeap = GearWeapon(OldWeapon);
	NewWeap	= GearWeapon(NewWeapon);

	//`log("PlayWeaponSwitch OldWeapon:" $ OldWeapon @ "NewWeapon:" $ NewWeapon );
	// detach old weapon
	if( OldWeap != None )
	{
		OldWeap.DetachWeapon();
	}

	// attach new weapon
	if( NewWeap != None )
	{
		NewWeap.AttachWeaponTo(Mesh, 'rt_barrel_bone');
	}
}

function SetMovementPhysics()
{
	// If in invalid physics mode - change it to None
	if(Physics != PHYS_Interpolating && Physics != PHYS_None)
	{
		SetPhysics(PHYS_None);
	}
}

/**
 * list important Pawn variables on canvas.  HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	super.DisplayDebug(HUD, out_YL, out_YPos);

	// Driver
	if( Driver != None )
	{
		HUD.Canvas.SetDrawColor(255,0,0);
		HUD.Canvas.DrawText("DRIVER");
		out_YPos += out_YL;
		HUD.Canvas.SetPos(4, out_YPos);

		Driver.DisplayDebug(HUD, out_YL, out_YPos);
	}
}
simulated function TurretOverRotated()
{
	local GearAI_Cover DriveAI, SpotAI;

	if( Driver != None )
	{
		DriveAI = GearAI_Cover(Controller);
	}
	if( Spotter != None )
	{
		SpotAI = GearAI_Cover(Spotter.Controller);
	}

	if( DriveAI != None )
	{
		DriveAI.NotifyTurretRotationClamped();
	}
	if( SpotAI != None )
	{
		SpotAI.NotifyTurretRotationClamped();
	}

	ViewRotVel = vect(0,0,0);
}

simulated function ProcessViewRotation(float DeltaTime, out rotator OutViewRot, out Rotator OutDeltaRot)
{
	// let parents have their way...
	super.ProcessViewRotation(DeltaTime, OutViewRot, OutDeltaRot);

	if( !InFreeCam() )
	{
		// then put the clamps on!
		TurretClampYaw(OutViewRot);
	}
}

simulated native event vector GetPhysicalFireStartLoc( vector FireOffset );


simulated native event Rotator GetViewRotation();


simulated function bool CalcCamera(float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV)
{
	// don't do special camera stuff, normal camera works, using warcam_turret mode
	return FALSE;
}


/** @See Pawn::FaceRotation */
simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	// short circuit camera for free cam.
	if( InFreeCam() )
	{
		// only affect pitch in free cam.
		AimDir.Pitch = Controller.Rotation.Pitch;
		return;
	}

	// Define relative turret rotation as being the controller rotation
	AimDir = Controller.Rotation;
}


/*
simulated function WeaponFired(bool bViaReplication, optional vector HitLocation)
{
	Super.WeaponFired(bViaReplication, HitLocation);

	// Play magazine spinning animation
	if( SpinSeqNode != None )
	{
		SpinSeqNode.bPlaying = TRUE;
	}
}

simulated function WeaponStoppedFiring(bool bViaReplication)
{
	Super.WeaponStoppedFiring(bViaReplication);

	// Stop magazine spinning animation
	if( SpinSeqNode != None )
	{
		SpinSeqNode.bPlaying = FALSE;
	}
}
*/

/** Overridden to get xhair when targeting. */
simulated function bool WantsCrosshair(PlayerController PC)
{
	local GearPC GPC;
	GPC = GearPC(Controller);
	return ( (GPC != None) && GPC.bIsTargeting );
}

simulated event Attach(Actor Other)
{
	Super.Attach(Other);

	// don't allow driver to become attached to us
	if (Other == Driver)
	{
		Other.SetBase(Base);
	}
}

defaultproperties
{
	// camera offsets
	CameraViewOffsetHigh=(X=-280,Y=0,Z=25)
	CameraViewOffsetLow=(X=-280,Y=0,Z=110)
	CameraViewOffsetMid=(X=-300,Y=0,Z=60)
	CameraTargetingViewOffsetHigh=(X=-180,Y=0,Z=50)
	CameraTargetingViewOffsetLow=(X=-140,Y=0,Z=36)
	CameraTargetingViewOffsetMid=(X=-143,Y=0,Z=36)
	ViewRotInterpSpeed=40.0

	bCollideWorld=FALSE

	Begin Object Name=CollisionCylinder
		CollisionHeight=50.000000
		CollisionRadius=15.000000
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=TRUE
		BlockActors=TRUE
		CollideActors=TRUE
		Translation=(X=70)
	End Object

	Begin Object Class=SkeletalMeshComponent Name=SkelMeshComponent0
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockActors=TRUE
		BlockRigidBody=TRUE
		AlwaysLoadOnClient=TRUE
		AlwaysLoadOnServer=TRUE
		LightEnvironment=TurretLightEnvironment
		RBChannel=RBCC_Vehicle
	End Object
	Mesh=SkelMeshComponent0
	CollisionComponent=SkelMeshComponent0
	Components.Add(SkelMeshComponent0)

	TrackSpeedWithSpotter=2.0f
	TrackSpeed=2.0f
	SearchSpeed=0.4f

	TurretTurnRateScale=0.3
	AimingTurretTurnRateScale=0.45
	ExitPositions(0)=(X=-110)
	EntryPosition=(X=-110)
	EntryRadius=100
	bRelativeExitPos=TRUE
	bAttachDriver=FALSE
	bUpdateSimulatedPosition=false
	PitchBone=b_troika_gun
	//PitchBone=Gun_pivot_bone
	ViewPitchMin=-6000
	ViewPitchMax=4400	// limit to IK doesn't stretch arms. do not change!
	POV=(DirOffset=(X=-9,Y=2,Z=4.5),Distance=25,fZAdjust=-100)
	CannonFireOffset=(X=250,Y=0,Z=24)

	InventoryManagerClass=class'GearInventoryManager'

	TooCloseDist=355
}
