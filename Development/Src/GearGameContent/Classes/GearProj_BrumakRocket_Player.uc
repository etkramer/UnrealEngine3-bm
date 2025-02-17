/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BrumakRocket_Player extends GearProj_BrumakRocket
	config(Weapon);


function SetupProjectile( GearWeapon W, Controller C, Pawn Brumak, Vector StartLocation, Rotator AimRot, float RocketAccelRate, optional bool bStraightRocket )
{
	local GearPC PC;

	local Rotator		CamRot;
	local Vector		CamLoc, StartTrace, EndTrace;
	local ImpactInfo	Impact;
	local Vector		LockedTargetVect;
	local Vector		AccelAdjustment, CrossVec;
	local float			VelBoost;

	PC = GearPC(C);

	PC.GetPlayerViewPoint( CamLoc, CamRot );
	AimRot		= CamRot;
	StartTrace	= CamLoc;
	EndTrace	= CamLoc + vector(CamRot)*10000;
	Impact		= W.CalcWeaponFire( StartTrace, EndTrace );
	Target		= Impact.HitLocation;
	bFinalTarget = bStraightRocket;
	
	LockedTargetVect = Target;

	VelBoost = 700.f;
	if( !bStraightRocket )
	{
		// Set their initial velocity
		CrossVec	= Vect(0,0,1);
		CrossVec   *= (FRand()>0.5f ? 1 : -1);

		AccelAdjustment = (vector(AimRot) Cross CrossVec) * RocketAccelRate;
		AccelAdjustment.Z += ((300.0 * FRand()) - 100.0) * ( FRand() * 2.f);

		KillRange			= 1024;
		bFinalTarget		= FALSE;
		SecondTarget		= LockedTargetVect;
		SwitchTargetTime	= 0.5;
	}
	else
	{
		VelBoost = 2500.f;
	}

	ArmMissile(AccelAdjustment, Vector(AimRot) * (VelBoost + VSize(Brumak.Velocity)) );
}

defaultproperties
{
	MyDamageType=class'GDT_BrumakCannonPlayer'

	TrailTemplate=ParticleSystem'Locust_Brumak.Effects.P_Brumak_Rideable_Rocket_Trail'

	Speed=2500.0
	AccelRate=750
	MaxSpeed=5000.0

	Begin Object Name=ExploTemplate0
	    MyDamageType=class'GDT_BrumakCannonPlayer'
		ExplosionSound=SoundCue'Locust_Brumak_Efforts2.Brumak.Brumak_RocketFireImpact2_Cue'
		FractureMeshRadius=300.0
		FracturePartVel=500.0
	End Object
}