/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Base class for all Gear turrets.
 */
class GearTurret extends Turret
	abstract
	config(Pawn)
	native;

/** View point offset for high player view pitch */
var(Camera) editconst vector	CameraViewOffsetHigh;
/** View point offset for medium (horizon) player view pitch */
var(Camera) editconst vector	CameraViewOffsetMid;
/** View point offset for low player view pitch */
var(Camera) editconst vector	CameraViewOffsetLow;

/** True to allow player to target while driving this turret, false otherwise */
var(Camera) editconst bool		bAllowTargetingCamera;

/** View point offset for high player view pitch (targeting mode) */
var(Camera) editconst vector	CameraTargetingViewOffsetHigh;
/** View point offset for medium (horizon) player view pitch (targeting mode) */
var(Camera) editconst vector	CameraTargetingViewOffsetMid;
/** View point offset for low player view pitch (targeting mode) */
var(Camera) editconst vector	CameraTargetingViewOffsetLow;

/** Fov in degrees while using this turret */
var(Camera) config float		CameraFOV;
/** Fov in degrees while using this turret (targeting mode) */
var(Camera) config float		CameraTargetingFOV;

/** Name of worst camera loc socket */
var(Camera) Name				WorstCamLocSocketName;

/** Current rotational velocity */
var protected vector ViewRotVel;

/** Controls how quickly to ramp current velocity toward desired velocity.  Feels like friction in the turret joint */
var() float ViewRotInterpSpeed;

/** True to apply rotational momemtum, so the turret feels heavier.  False for perfectly crisp aiming. */
var() bool bRotationalMomentum;

/** LocalToWorld of the turret's Base actor last tick. */
var protected transient matrix LastBaseTM;

/** TRUE to self-enforce bHardAttach, regardless of bBlockActors. */
var() protected const bool	bEnforceHardAttach;

/** hack */
var protected transient bool bMovingToEnforceHardAttach;

/** The light environment for this Pickup so we can see the item in our statically lit levels **/
var DynamicLightEnvironmentComponent MyLightEnvironment;

struct CheckpointRecord
{
	var bool bHidden;
};

cpptext
{
	virtual void PostBeginPlay();
	virtual UBOOL IgnoreBlockingBy( const AActor *Other) const;
	virtual UBOOL ShouldDriverBeValidTargetFor( const APawn* Other) const
	{
		return TRUE;
	}
};



simulated function bool ShouldTargetingModeZoomCamera()
{
	local GearPC GPC;
	GPC = GearPC(Controller);
	return ( (GPC != None) && GPC.bIsTargeting );
}

function bool DriverEnter(Pawn P)
{
	local GearPawn GP;

	// drop shield if player tries to enter turret while carrying it
	GP = GearPawn(P);
	if (GP != None && GP.EquippedShield != None)
	{
		GP.DropShield();
	}
	ViewRotVel = vect(0,0,0);
	return super.DriverEnter(P);
}

function bool DriverLeave( bool bForceLeave )
{
	local Rotator NewRot;

	if (GearPC(Controller) != None)
	{
		// zero any roll
		NewRot = Controller.Rotation;
		NewRot.Roll = 0;
		Controller.SetRotation(NewRot);
	}

	return super.DriverLeave(bForceLeave);
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
	local GearWeapon MyGW;
	Super.PossessedBy(C, bVehicleTransition);

	// cache a reference to the AI controller
	MyGW = GearWeapon(Weapon);
	if(MyGW != none)
	{
		MyGW.GearAIController = GearAI(Controller);
		MyGW.AIController = MyGW.GearAIController;
	}
}

simulated function OnGearExitVehicle( SeqAct_GearExitVehicle inAction )
{
	local GearAI_Cover GAI;
	if (inAction != None && GearTurret(inAction.TargetVehicle) != None)
	{
		GAI = GearAI_Cover(Controller);
		DriverLeave(TRUE);
		if(GAI != none)
		{
			GAI.StopUsingTurret(self);
		}
	}
}

/** overridden for turrets to give some momentum.  move into turret code? */
simulated function ProcessViewRotation( float DeltaTime, out Rotator out_ViewRotation, out Rotator OutDeltaRot )
{
	local vector DesiredViewRotVel;

	// now apply momentum effects
	if (bRotationalMomentum)
	{
		DesiredViewRotVel.X = float(OutDeltaRot.Roll) / DeltaTime;
		DesiredViewRotVel.Y = float(OutDeltaRot.Pitch) / DeltaTime;
		DesiredViewRotVel.Z = float(OutDeltaRot.Yaw) / DeltaTime;

		ViewRotVel = VInterpTo(ViewRotVel, DesiredViewRotVel, DeltaTime, ViewRotInterpSpeed);

		OutDeltaRot.Roll = NormalizeRotAxis(ViewRotVel.X * DeltaTime);
		OutDeltaRot.Pitch = NormalizeRotAxis(ViewRotVel.Y * DeltaTime);
		OutDeltaRot.Yaw = NormalizeRotAxis(ViewRotVel.Z * DeltaTime);
	}

	// let parents do their thing
	super.ProcessViewRotation(DeltaTime, out_ViewRotation, OutDeltaRot);
}

/**
* Returns the "worst case" camera location for this camera mode.
* This is the position that the camera ray is shot from, so it should be
* a guaranteed safe place to put the camera.
*/
simulated function vector GetCameraWorstCaseLoc()
{
	local vector WorstLoc;
	local rotator UnusedRot;

	Mesh.GetSocketWorldLocationAndRotation(WorstCamLocSocketName, WorstLoc, UnusedRot);
	return WorstLoc;
}

function bool StopFiring()
{
	if( Weapon != None )
	{
		Weapon.StopFire(0);
	}

	return TRUE;
}

/** Handle any motion by the actor this turret is base on. */
simulated protected native function UpdateForMovingBase(Actor BaseActor);

simulated function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);

	if ( (Base != None) && !Base.bStatic )
	{
		UpdateForMovingBase(Base);
	}
}

simulated function bool IsWithinRotationClamps(vector Loc)
{
	return true;
}

simulated function bool WantsCrosshair(PlayerController PC)
{
	return FALSE;
}

function bool ShouldSaveForCheckpoint()
{
	return (Controller == None);
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.bHidden = bHidden;
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	SetHidden(Record.bHidden);
}

defaultproperties
{
	bNoDelete=TRUE
	bPushedByEncroachers=FALSE
	bRotationalMomentum=TRUE
	bAllowTargetingCamera=TRUE
	ViewRotInterpSpeed=40.0

	WorstCamLocSocketName="WorstCamLoc"

	Begin Object Name=PathCollisionCylinder
		CollisionRadius=20.000000
	End Object


	// define here as lot of sub classes which have moving parts will utilize this
	Begin Object Class=DynamicLightEnvironmentComponent Name=TurretLightEnvironment
	    bDynamic=FALSE
		bCastShadows=FALSE
		AmbientGlow=(R=0.3f,G=0.3f,B=0.3f,A=1.0f)
		TickGroup=TG_DuringAsyncWork
	End Object
	MyLightEnvironment=TurretLightEnvironment
	Components.Add(TurretLightEnvironment)

}




