
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class Turret extends Vehicle
	native
	abstract;

/**
 * Base turret code implementation
 *
 */

var Controller Claim;

/** Default inventory added via AddDefaultInventory() */
var array<class<Inventory> >	DefaultInventory;

/** Current Turret Aim  */
var	Rotator	AimDir;

/** desired aim dir */
var Rotator DesiredAimDir;

/** limit, in rotator units, of how far from center the turret can rotate in the yaw axis */
var() Vector2D YawLimit;

/** Player turning rate scale */
var()		float	TurretTurnRateScale;
/** Player turning rate scale when aiming */
var()		float	AimingTurretTurnRateScale;

var()	Vector	CannonFireOffset;	// from pitch bone, aligned to DesiredAim
var		name	PitchBone;
var		name	BaseBone;

var		vector	EntryPosition;		// Offset for entry

/** True if exit positions are relative offsets to vehicle's location, false if they are absolute world locations */
var		bool			bRelativeExitPos;

/** Pawn confined to this turret is not allowed to leave until this flag is unchecked */
var		bool			bUnableToLeave;

struct native sPointOfView
{
	var()	vector	DirOffset;		// BaseRotation relative Offset
	var()	float	Distance;		// relative offset dist
	var()	float	fZAdjust;		// Adjust Z based on pitch
};

var() sPointOfView	POV;	// Camera info


/** TRUE when spotter is tracking something */
var		bool	bSpottedSomething;

/** Action displayed on the HUD */
var ActionInfo InteractAction;

/** custom replicated weapon reference */
var repnotify Weapon MyWeapon;

//debug
var(Debug) bool bDebugTurret;

/** bones where the turret controller places their hands **/
var name LeftHandBoneHandleName;
var name RightHandBoneHandleName;

var float InitialEntryZOffset;

replication
{
	if( (bNetInitial || (!bNetOwner && bNetDirty)) && Role==ROLE_Authority )
		AimDir, MyWeapon;
	if(ROLE==ROLE_Authority && bNetDirty)
		InitialEntryZOffset;
}

cpptext
{
	virtual UBOOL IgnoreBlockingBy( const AActor *Other) const;
};

native simulated function UpdateDriver(FLOAT DeltaTime);

/**
 * Get Driver idle location when manning this turret.
 * Physics will try to place the player there. See UpdateDriver().
 */
simulated event vector GetDriverIdealPosition()
{
	// Code is implented in GetEntryLocation().
	return GetEntryLocation();
}

simulated event rotator GetDriverIdealRotation()
{
	return AimDir;
}

simulated function PostBeginPlay()
{
	local PlayerController PC;

	super.PostBeginPlay();

	// define AimDir, as where the turret is originally looking at when placed in the level
	AimDir			= Rotation;
	DesiredAimDir	= AimDir;

	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		GearPC(PC).bCheckVehicles = true;
	}
}

function Restart()
{
	if (Role == ROLE_Authority)
	{
		AddDefaultInventory();
	}
}

simulated function Tick(float DeltaTime)
{
	local GearWeapon MyGearWeapon;
	local bool	bIsControllerFireInputPressed, bIsWeaponPendingFire, bIsPawnCommitToFiring;
	local byte	AltFiringMode;


	super.Tick(DeltaTime);

	// If locally controlled, handle firing
	if( IsLocallyControlled() )
	{
		bIsWeaponPendingFire = IsWeaponPendingFire(0);
		bIsControllerFireInputPressed = IsControllerFireInputPressed(0);
		bIsPawnCommitToFiring = PawnCommitToFiring(0);
		if( !bIsWeaponPendingFire && bIsPawnCommitToFiring )
		{
			StartFire( 0 );
		}
		else
		if( bIsWeaponPendingFire && !bIsControllerFireInputPressed )
		{
			StopFire( 0 );
		}

		MyGearWeapon = GearWeapon(MyWeapon);
		if( MyGearWeapon != None && MyGearWeapon.bAltFireWeapon )
		{
			AltFiringMode = class'GearWeapon'.const.ALTFIRE_FIREMODE;
			bIsWeaponPendingFire = IsWeaponPendingFire(AltFiringMode);
			bIsControllerFireInputPressed = IsControllerFireInputPressed(AltFiringMode);
			bIsPawnCommitToFiring = PawnCommitToFiring(AltFiringMode);

			if( !bIsWeaponPendingFire && bIsPawnCommitToFiring )
			{
				StartFire(AltFiringMode);
			}
			else if( bIsWeaponPendingFire && !bIsControllerFireInputPressed )
			{
				StopFire(AltFiringMode);
			}
		}
	}
}

final function bool IsWeaponPendingFire( byte InFiringMode )
{
	return (InvManager != None && InvManager.PendingFire[InFiringMode] > 0);
}

simulated function bool PawnCommitToFiring( byte InFiringMode )
{
	return (IsWeaponPendingFire(InFiringMode) || IsControllerFireInputPressed(InFiringMode));
}

final simulated function bool IsControllerFireInputPressed(byte InFiringMode)
{
	if( InFiringMode == class'GearWeapon'.const.DEFAULT_FIREMODE )
	{
		return (Controller != None && Controller.bFire == 1);
	}

	if( InFiringMode == class'GearWeapon'.const.ALTFIRE_FIREMODE )
	{
		return (GearPC(Controller) != None && GearPC(Controller).IsButtonActive(GB_LeftTrigger) );
	}

	return FALSE;
}


/** @See Vehicle::DriverEnter */
function bool DriverEnter(Pawn P)
{
	local bool	bSuccess;

	// if we don't have pre-defined exit positions here, we use the original player location as an exit point
	if( !bRelativeExitPos )
	{
		ExitPositions[0] =  P.Location + Vect(0,0,16);
	}

	// Controller possesses turret.
	bSuccess = Super.DriverEnter(P);

	if (bSuccess)
	{
		// force the controller to be looking in the same direction as the turret
		if( Controller != None )
		{
			Controller.SetRotation(AimDir);
		}
		TriggerEventClass(class'SeqEvt_VehicleDriven',P,0);
	}

	InitialEntryZOffset = Driver.Location.Z - GetEntryLocation().Z;
	return bSuccess;
}

event bool DriverLeave( bool bForceLeave )
{
	local Pawn OldDriver;

	if( bUnableToLeave && !bForceLeave )
	{
		return FALSE;
	}

	OldDriver = Driver;
	if (Super.DriverLeave(bForceLeave))
	{
		bUnableToLeave = FALSE;
		TriggerEventClass(class'SeqEvt_VehicleDriven',OldDriver,1);
		return TRUE;
	}
	return FALSE;
}

function bool ClaimTurret( Controller C )
{
	Claim = C;
	bStasis = FALSE;
	return TRUE;
}

function bool UnclaimTurret( Controller C )
{
	if( Claim == C )
	{
		Claim = None;
		bStasis = TRUE;

		return TRUE;
	}

	return FALSE;
}


/**
 * Overridden to iterate through the DefaultInventory array and
 * give each item to this Pawn.
 *
 * @see			GameInfo.AddDefaultInventory
 */
function AddDefaultInventory()
{
	local int		i;
	local Inventory	Inv;

	for( i=0; i<DefaultInventory.Length; i++ )
	{
		// Ensure we don't give duplicate items
		if( FindInventoryType(DefaultInventory[i]) == None )
		{
			// spawn with self as starting owner so it is spawned in the same streaming level the turret is in
			Inv = Spawn(DefaultInventory[i], self);
			if (Inv != None)
			{
				if(InvManager != None)
				{
					InvManager.AddInventory(Inv);
				}

				if (Weapon(Inv) != None)
				{
					// make weapon always replicated so it works on clients automatically
					Inv.bAlwaysRelevant = true;
					// don't allow default weapon to be thrown out
					Weapon(Inv).bCanThrow = FALSE;
					SetActiveWeapon(Weapon(Inv));
				}
			}
		}
	}
}

simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	// set replicated var
	MyWeapon = NewWeapon;
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'MyWeapon')
	{
		PlayWeaponSwitch(Weapon, MyWeapon);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
	Super.PossessedBy(C, bVehicleTransition);

	// make sure our new owner has the correct weapon set
	if (Weapon != None)
	{
		Weapon.bNetDirty = true;
		Weapon.bForceNetUpdate = TRUE;
		Weapon.ClientWeaponSet(false);
	}
}

simulated function ClientRestart()
{
	Super.ClientRestart();

	// sanity check we got the weapon set up
	if (Weapon != None)
	{
		Weapon.ClientWeaponSet(false);
	}
}

/** @See Pawn::ProcessViewRotation */
simulated function ProcessViewRotation(float DeltaTime, out rotator OutViewRot, out Rotator OutDeltaRot)
{
	local float RateScale;
	local GearPC GPC;

	// Change scale if aiming
	GPC = GearPC(Controller);
	RateScale = ((GPC != None) && GPC.bIsTargeting) ? AimingTurretTurnRateScale : TurretTurnRateScale;

	// Scale player turning input
	OutDeltaRot *= RateScale;
	super.ProcessViewRotation(DeltaTime, OutViewRot, OutDeltaRot);
}


/** @See Pawn::FaceRotation */
simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	// Define relative turret rotation as being the controller rotation
	AimDir = Controller.Rotation;
}


/**
 * Perform update for AI controller.
 * tracks targets and such...
 */
function UpdateAIController(GearAI_Cover C, float DeltaTime)
{
	// if we have an target, track it down
	if( C.HasValidTarget() )
	{
		DesiredAimDir = Rotator( C.GetFireTargetLocation() - Location);
	}

	AimDir = RInterpTo(AimDir, DesiredAimDir, DeltaTime, 1.f);
	DrawDebugLine(Location + vector(GetTurretAimDir()) * 16384, Location, 255, 128, 0);
}


/**
 * Called every second when AI is Idle.
 */
function AIIdleNotification(GearAI_Cover C)
{
	// pick a random orientation
	if( FRand() < 0.33 )
	{
		DesiredAimDir = GetRandomRotForTurretScan(C);
	}
}

/**
 * Get a random rotation for turret scanning.
 */
function Rotator GetRandomRotForTurretScan(GearAI_Cover C)
{
	local Rotator DesiredRot;

	// pick a random orientation
	// if we saw an target, pick a destination close to where we last saw him
	if( C.HasValidTarget() )
	{
		DesiredRot = Rotator( C.GetFireTargetLocation() - C.Pawn.Location);
	}
	// otherwise just scan the area around where turret is facing
	else
	{
		DesiredRot	= Rotation;
	}

	DesiredRot.Pitch	+= Rand(2048)	- 512;
	DesiredRot.Yaw		+= Rand(8192)	- 4096;

	return DesiredRot;
}


/**
 *	Calculate camera view point, when viewing this pawn.
 *
 * @param	fDeltaTime	delta time seconds since last update
 * @param	out_CamLoc	Camera Location
 * @param	out_CamRot	Camera Rotation
 * @param	out_FOV		Field of View
 *
 * @return	true if Pawn should provide the camera point of view.
 */
simulated function bool CalcCamera(float fDeltaTime, out vector out_CamLoc, out rotator out_CamRot, out float out_FOV)
{
	local Rotator	TempRotation, AimRot;
	local vector	CamLookAt, HitLocation, HitNormal;
	local vector	dirX, dirY, dirZ;
	local float		Elevation;

	AimRot = GetTurretAimDir();

	//CamLookAt = Mesh.GetBoneLocation( PitchBone ) + vector(AimRot) * 2048;
	CamLookAt = Location + vector(AimRot) * 16384;

	// Base Direction (Only YAW)
	TempRotation.Pitch	= 0;
	TempRotation.Yaw	= AimRot.Yaw;
	TempRotation.Roll	= 0;
	GetAxes(TempRotation, dirX, dirY, dirZ);
	out_CamLoc = Location + POV.Distance * (dirX*POV.DirOffset.X + dirY*POV.DirOffset.Y + dirZ*POV.DirOffset.Z);

	Elevation = float(NormalizeRotAxis( AimRot.Pitch ));	// elevation angle the controller is aiming at

	// Z Adjust based on pitch
	out_CamLoc = out_CamLoc + vect(0,0,1) * POV.fZAdjust * (Elevation / FMax(-ViewPitchMin,ViewPitchMax));

	if( Trace( HitLocation, HitNormal, out_CamLoc, Location, false, vect(10, 10, 10) ) != None )
	{
		out_CamLoc = HitLocation + HitNormal * 10;
	}

	//
	// rotate camera to match focus point
	//
	out_CamRot = Rotator(CamLookAt - out_CamLoc);
	return true;
}

/** Physical fire start location. (from weapon's barrel in world coordinates) */
simulated native event vector GetPhysicalFireStartLoc(vector FireOffset);

simulated native event Vector GetPawnViewLocation();

/**
 * returns base Aim Rotation without any adjustment (no aim error, no autolock, no adhesion.. just clean initial aim rotation!)
 *
 * @return	base Aim rotation.
 */
simulated singular event Rotator GetBaseAimRotation()
{
	local vector	POVLoc;
	local rotator	POVRot;

	// If we have a controller, by default we aim at the player's 'eyes' direction
	// that is by default Controller.Rotation for AI, and camera (crosshair) rotation for human players.
	if( Controller != None && !InFreeCam() )
	{
		Controller.GetPlayerViewPoint(POVLoc, POVRot);
		return POVRot;
	}

	// If we have no controller, we simply use our rotation
	POVRot = GetTurretAimDir();

	// If our Pitch is 0, then use RemoveViewPitch
	if( POVRot.Pitch == 0 )
	{
		POVRot.Pitch = RemoteViewPitch << 8;
	}

	return POVRot;
}


simulated final function bool TurretClampYaw( out Rotator Rot )
{
	local int	DeltaFromCenter, YawAdj;
	local bool	bResult;

	DeltaFromCenter = NormalizeRotAxis(Rot.Yaw - Rotation.Yaw);

	if( DeltaFromCenter > YawLimit.Y )
	{
		YawAdj	= YawLimit.Y - DeltaFromCenter;
		bResult = TRUE;
	}
	else if( DeltaFromCenter < YawLimit.X )
	{
		YawAdj	= -(DeltaFromCenter - YawLimit.X);
		bResult = TRUE;
	}


	Rot.Yaw += YawAdj;
	//`log("limit:"@YawLimit@"DeltaFromCEnteR"@DeltaFromCenter@"YawAdj"@YawAdj@"turret:"@Rotation.Yaw@"controller:"@OutViewRot.Yaw);

	return bResult;
}

simulated function TurretOverRotated();

/** returns absolute Aim direction of turret */
simulated native function rotator GetTurretAimDir();

simulated function name GetDefaultCameraMode( PlayerController RequestedBy )
{
	return 'Default';
}

simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	Destroy();
}

/** Turrets don't take damage */
event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser);

function StartSpotting();
function StopSpotting();

function AISpotterIdleNotification(GearAI_Cover C);
function UpdateAISpotter(GearAI_Cover C, float DeltaTime);
function bool ClaimSpotter( GearAI_Cover AI );
function bool UnclaimSpotter( GearAI_Cover AI );

function bool ClaimLoader( GearAI_Cover AI );
function bool UnclaimLoader( GearAI_Cover AI );

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
	local Canvas	Canvas;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255, 255, 255);

	Canvas.DrawText(" AimDir:" @ AimDir);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);
}

/************************************************************************************
 * Weapon / Firing
 ***********************************************************************************/


/**
 * Forward FlashLocation updates to weapon.
 */
simulated function FlashLocationUpdated( bool bViaReplication )
{
	if( GearWeapon(MyWeapon) != None )
	{
		GearWeapon(MyWeapon).FlashLocationUpdated( FiringMode, FlashLocation, bViaReplication );
	}
}


/**
 * Forward FlashCount updates to weapon.
 */

simulated function FlashCountUpdated( bool bViaReplication )
{
	if( GearWeapon(MyWeapon) != None )
	{
		GearWeapon(MyWeapon).FlashCountUpdated( FlashCount, FiringMode, bViaReplication );
	}
}


/** @see Pawn::WeaponFired */
simulated function WeaponFired( bool bViaReplication, optional vector HitLocation )
{
	// increment number of consecutive shots.
	ShotCount++;
}


/** @see Pawn::WeaponStoppedFiring */
simulated function WeaponStoppedFiring( bool bViaReplication )
{
	// reset number of consecutive shots fired.
	ShotCount = 0;
}


/** Play proper character animation when firing mode changes */
simulated function FiringModeUpdated( bool bViaReplication )
{
	if( MyWeapon != None )
	{
		MyWeapon.FireModeUpdated(FiringMode, bViaReplication);
	}
}

function Reset()
{
	StopFiring();
	DetachFromController();
}

function UnPossessed()
{
	StopFiring();
	Super.UnPossessed();
}

simulated function StartFire(byte FireModeNum)
{
	if (Controller != None)
	{
		Super.StartFire(FireModeNum);
	}
}

/** overridden so it's simulated for GearPC checks and so we skip the WorldInfo.Game check as we doesn't care about that */
simulated function bool CanEnterVehicle(Pawn P)
{
	if( Driver != None ||
		 P.bIsCrouched ||
		 P.DrivenVehicle != None ||
		 P.Controller == None ||
		!P.Controller.bIsPlayer ||
		 P.IsA('Vehicle') ||
		 Health <= 0 )
	{
		return false;
	}

	return true;
}

defaultproperties
{
	Physics=PHYS_None
	AimingTurretTurnRateScale=1.f
	TurretTurnRateScale=1.f
	DriverDamageMult=1.f
	LandMovementState=PlayerTurreting
	ViewPitchMin=-4096
	ViewPitchMax=8192
	YawLimit=(X=-16384,Y=+16384)
	POV=(DirOffset=(X=-5,Y=0,Z=4),Distance=200,fZAdjust=-350)
	bAttachDriver=FALSE
	bPathColliding=TRUE
	bStationary=TRUE
	bSimulateGravity=false

	Begin Object Class=CylinderComponent Name=PathCollisionCylinder
		CollisionHeight=50.000000
	    CollisionRadius=60.000000
		BlockZeroExtent=TRUE
		BlockNonZeroExtent=TRUE
		BlockActors=TRUE
		CollideActors=TRUE
		AlwaysLoadOnClient=FALSE	// exists only in editor
		AlwaysLoadOnServer=FALSE
		Translation=(X=70)
	End Object
	Components.Add(PathCollisionCylinder)

	InteractAction={(
		ActionName=UseTurret,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32))), // X Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=109,V=425,UL=122,VL=87)))	),
		)}

	SupportedEvents.Add(class'SeqEvt_VehicleDriven')
}
