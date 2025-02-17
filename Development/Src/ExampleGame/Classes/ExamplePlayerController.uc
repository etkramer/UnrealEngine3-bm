//=============================================================================
// ExamplePlayerController
// Example specific playercontroller
// PlayerControllers are used by human players to control pawns.
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class ExamplePlayerController extends GamePlayerController
	config(Game);

var()				float			WeaponImpulse;
var()				float			HoldDistanceMin;
var()				float			HoldDistanceMax;
var()				float			ThrowImpulse;
var()				float			ChangeHoldDistanceIncrement;

var()				RB_Handle		PhysicsGrabber;
var					float			HoldDistance;
var					Quat			HoldOrientation;
var					UIScene			ExamplePauseMenuResource;


/** Inits online stats object */
simulated event PostBeginPlay()
{
	Super.PostBeginPlay();
}

/**
 * Registers any game specific datastores
 */
simulated protected function RegisterCustomPlayerDataStores()
{
	Super.RegisterCustomPlayerDataStores();
}

/**
 * Unregisters any game specific datastores
 */
simulated function UnregisterPlayerDataStores()
{
	Super.UnregisterPlayerDataStores();
}

/** Start as PhysicsSpectator by default */
auto state PlayerWaiting
{
Begin:
	PlayerReplicationInfo.bOnlySpectator = false;
	WorldInfo.Game.bRestartLevel = false;
	WorldInfo.Game.RestartPlayer( Self );
	WorldInfo.Game.bRestartLevel = true;
	SetCameraMode('ThirdPerson');
}


// Previous weapon entrypoint. Here we just alter the hold distance for the physics weapon
exec function PrevWeapon()
{
	HoldDistance += ChangeHoldDistanceIncrement;
	HoldDistance = FMin(HoldDistance, HoldDistanceMax);
}

// Next weapon entrypoint. Here we just alter the hold distance for the physics weapon
exec function NextWeapon()
{
	HoldDistance -= ChangeHoldDistanceIncrement;
	HoldDistance = FMax(HoldDistance, HoldDistanceMin);
}

//
// Primary fire has been triggered. This sends impulses from the default physics gun
//
exec function StartFire( optional byte FireModeNum )
{
	local vector		CamLoc, StartShot, EndShot, X, Y, Z;
	local vector		HitLocation, HitNormal, ZeroVec;
	local actor			HitActor;
	local TraceHitInfo	HitInfo;
	local rotator		CamRot;

	GetPlayerViewPoint(CamLoc, CamRot);

	GetAxes( CamRot, X, Y, Z );
	ZeroVec = vect(0,0,0);

	if ( PhysicsGrabber.GrabbedComponent == None )
	{
		// Do simple line check then apply impulse
		StartShot	= CamLoc;
		EndShot		= StartShot + (10000.0 * X);
		HitActor	= Trace(HitLocation, HitNormal, EndShot, StartShot, True, ZeroVec, HitInfo, TRACEFLAG_Bullet);

		if ( HitActor != None && HitInfo.HitComponent != None )
		{
			HitInfo.HitComponent.AddImpulse(X * WeaponImpulse, HitLocation, HitInfo.BoneName);
			HitActor.TakeDamage(10, self, HitLocation, WeaponImpulse*HitNormal, class'ExampleDamageType', HitInfo, self);
		}
	}
	else
	{
		PhysicsGrabber.GrabbedComponent.AddImpulse(X * ThrowImpulse, , PhysicsGrabber.GrabbedBoneName);
		PhysicsGrabber.ReleaseComponent();
	}
}

// Alternate-fire has been triggered.  This is the physics grabber
exec function StartAltFire( optional byte FireModeNum )
{
	local vector					CamLoc, StartShot, EndShot, X, Y, Z;
	local vector					HitLocation, HitNormal, Extent;
	local actor						HitActor;
	local float						HitDistance;
	local Quat						PawnQuat, InvPawnQuat, ActorQuat;
	local TraceHitInfo				HitInfo;
	local SkeletalMeshComponent		SkelComp;
	local rotator					CamRot;
	local FracturedStaticMeshActor FracActor;

	GetPlayerViewPoint(CamLoc, CamRot);

	// Do ray check and grab actor
	GetAxes( CamRot, X, Y, Z );
	StartShot	= CamLoc;
	EndShot		= StartShot + (10000.0 * X);
	Extent		= vect(0,0,0);
	HitActor	= Trace(HitLocation, HitNormal, EndShot, StartShot, True, Extent, HitInfo);
	HitDistance = VSize(HitLocation - StartShot);

	if( HitActor != None &&
		HitActor != WorldInfo &&
		HitDistance > HoldDistanceMin &&
		HitDistance < HoldDistanceMax )
	{
		// If grabbing a bone of a skeletal mesh, dont constrain orientation.
		PhysicsGrabber.GrabComponent(HitInfo.HitComponent, HitInfo.BoneName, HitLocation, bRun==0);

		// If we succesfully grabbed something, store some details.
		if( PhysicsGrabber.GrabbedComponent != None )
		{
			HoldDistance	= HitDistance;
			PawnQuat		= QuatFromRotator( CamRot );
			InvPawnQuat		= QuatInvert( PawnQuat );

			if ( HitInfo.BoneName != '' )
			{
				SkelComp = SkeletalMeshComponent(HitInfo.HitComponent);
				ActorQuat = SkelComp.GetBoneQuaternion(HitInfo.BoneName);
			}
			else
			{
				ActorQuat = QuatFromRotator( PhysicsGrabber.GrabbedComponent.Owner.Rotation );
			}

			HoldOrientation = QuatProduct(InvPawnQuat, ActorQuat);
		}
		else
		{
			foreach CollidingActors(class'FracturedStaticMeshActor', FracActor, 100, HitLocation, TRUE)
			{
				FracActor.BreakOffPartsInRadius(HitLocation, 100, 20, true);
			}
		}
	}
}

// Alt-fire is a grab, so a release function is needed
exec function StopAltFire( optional byte FireModeNum )
{
	if ( PhysicsGrabber.GrabbedComponent != None )
	{
		PhysicsGrabber.ReleaseComponent();
	}
}

//
// Logic tick for the player controller. Just updates physics weapon logic
//
event PlayerTick(float DeltaTime)
{
	local vector	CamLoc, NewHandlePos, X, Y, Z;
	local Quat		PawnQuat, NewHandleOrientation;
	local rotator	CamRot;

	super.PlayerTick(DeltaTime);

	if ( PhysicsGrabber.GrabbedComponent == None )
	{
		return;
	}

	PhysicsGrabber.GrabbedComponent.WakeRigidBody( PhysicsGrabber.GrabbedBoneName );

	// Update handle position on grabbed actor.
	GetPlayerViewPoint(CamLoc, CamRot);
	GetAxes( CamRot, X, Y, Z );
	NewHandlePos = CamLoc + (HoldDistance * X);
	PhysicsGrabber.SetLocation( NewHandlePos );

	// Update handle orientation on grabbed actor.
	PawnQuat = QuatFromRotator( CamRot );
	NewHandleOrientation = QuatProduct(PawnQuat, HoldOrientation);
	PhysicsGrabber.SetOrientation( NewHandleOrientation );
}

exec function ShowMenu()
{
	DisplayGameMenu();
}

function DisplayGameMenu()
{
	if ( ExamplePauseMenuResource != None )
	{
		ExamplePauseMenuResource.OpenScene(ExamplePauseMenuResource, LocalPlayer(Player));
	}
}

defaultproperties
{
	CameraClass=class'ExamplePlayerCamera'
	CheatClass=class'ExampleCheatManager'
	ExamplePauseMenuResource=UIScene'EngineScenes.GameMenu'

	HoldDistanceMin=50.0
	HoldDistanceMax=750.0
	WeaponImpulse=600.0
	ThrowImpulse=800.0
	ChangeHoldDistanceIncrement=50.0

	Begin Object Class=RB_Handle Name=RB_Handle0
	End Object
	PhysicsGrabber=RB_Handle0
	Components.Add(RB_Handle0)
}

