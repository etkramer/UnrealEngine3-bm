/**
 * Physics Gun
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_PhysicsGun extends GearWeapon
	hidedropdown;


var()				float			WeaponImpulse;
var()				float			HoldDistanceMin;
var()				float			HoldDistanceMax;
var()				float			ThrowImpulse;
var()				float			ChangeHoldDistanceIncrement;

var					RB_Handle		PhysicsGrabber;
var					float			HoldDistance;
var					Quat			HoldOrientation;

simulated function StartFire(byte FireModeNum)
{
	local vector					StartShot, EndShot;
	local vector					HitLocation, HitNormal, Extent;
	local actor						HitActor;
	local float						HitDistance;
	local Quat						PawnQuat, InvPawnQuat, ActorQuat;
	local TraceHitInfo				HitInfo;
	local SkeletalMeshComponent		SkelComp;
	local Rotator					Aim;

	// Do ray check and grab actor
	StartShot	= Instigator.GetWeaponStartTraceLocation();
	Aim			= GetAdjustedAim( StartShot );
	EndShot		= StartShot + (10000.0 * Vector(Aim));
	Extent		= vect(0,0,0);
	HitActor	= Trace(HitLocation, HitNormal, EndShot, StartShot, True, Extent, HitInfo);
	HitDistance = VSize(HitLocation - StartShot);

	if( HitActor != None &&
		HitActor != WorldInfo &&
		HitDistance > HoldDistanceMin &&
		HitDistance < HoldDistanceMax )
	{
		// If grabbing a bone of a skeletal mesh, dont constrain orientation.
		PhysicsGrabber.GrabComponent(HitInfo.HitComponent, HitInfo.BoneName, HitLocation, PlayerController(Instigator.Controller).bRun==0);

		// If we succesfully grabbed something, store some details.
		if (PhysicsGrabber.GrabbedComponent != None)
		{
			HoldDistance	= HitDistance;
			PawnQuat		= QuatFromRotator( Rotation );
			InvPawnQuat		= QuatInvert( PawnQuat );

			if ( HitInfo.BoneName != '' )
			{
				SkelComp = SkeletalMeshComponent(HitInfo.HitComponent);
				ActorQuat = SkelComp.GetBoneQuaternion(HitInfo.BoneName);
			}
			else
				ActorQuat = QuatFromRotator( PhysicsGrabber.GrabbedComponent.Owner.Rotation );

			HoldOrientation = QuatProduct(InvPawnQuat, ActorQuat);
		}
	}
}

simulated function StopFire(byte FireModeNum)
{
	if ( PhysicsGrabber.GrabbedComponent != None )
		PhysicsGrabber.ReleaseComponent();
}

simulated function bool DoOverridePrevWeapon()
{
	HoldDistance += ChangeHoldDistanceIncrement;
	HoldDistance = FMin(HoldDistance, HoldDistanceMax);
	return true;
}

simulated function bool DoOverrideNextWeapon()
{
	HoldDistance -= ChangeHoldDistanceIncrement;
	HoldDistance = FMax(HoldDistance, HoldDistanceMin);
	return true;
}

simulated function Tick( float DeltaTime )
{
	local vector	NewHandlePos, StartLoc;
	local Quat		PawnQuat, NewHandleOrientation;
	local Rotator	Aim;

	if ( PhysicsGrabber.GrabbedComponent == None )
	{
		GotoState( 'Active' );
		return;
	}

	PhysicsGrabber.GrabbedComponent.WakeRigidBody( PhysicsGrabber.GrabbedBoneName );

	// Update handle position on grabbed actor.
	StartLoc		= Instigator.GetWeaponStartTraceLocation();
	Aim				= GetAdjustedAim( StartLoc );
	NewHandlePos	= StartLoc + (HoldDistance * Vector(Aim));
	PhysicsGrabber.SetLocation( NewHandlePos );

	// Update handle orientation on grabbed actor.
	PawnQuat				= QuatFromRotator( Rotation );
	NewHandleOrientation	= QuatProduct(PawnQuat, HoldOrientation);
	PhysicsGrabber.SetOrientation( NewHandleOrientation );
}

defaultproperties
{
	WeaponFireAnim=Shoot
	FireSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleFireCue'
	FiringStatesArray(0)="WeaponFiring"
	FiringStatesArray(1)="Holding"

	HoldDistanceMin=50.0
	HoldDistanceMax=750.0
	WeaponImpulse=600.0
	ThrowImpulse=800.0
	ChangeHoldDistanceIncrement=50.0

	FireOffset=(X=68,Y=0,Z=9)

	Begin Object Class=RB_Handle Name=RB_Handle0
	End Object
	PhysicsGrabber=RB_Handle0
	Components.Add(RB_Handle0)

	// Weapon Mesh Transform
	// Weapon Mesh
	Begin Object Name=WeaponMesh
	    SkeletalMesh=SkeletalMesh'COG_AssaultRifle.Mesh.COGAssaultRifle'
		AnimSets(0)=AnimSet'COG_AssaultRifle.Animations.COG_AssaultRifle'
	End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		Brightness=3
		LightColor=(R=64,G=160,B=255,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
}
