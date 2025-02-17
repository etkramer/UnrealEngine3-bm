/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* this is so that we can have separate config settings for the minigun when controlled by boomers
**/
class GearWeap_Hydra_Minigun extends GearWeap_HeavyMinigun;

var float TracerAdvance;

simulated function GearProj_BulletTracer GetTracer( vector SpawnLocation, rotator SpawnRotation )
{
	local GearProj_BulletTracer NewTracer;

	NewTracer = GearGRI(WorldInfo.GRI).GOP.GetTracer( TracerType, 0, SpawnLocation + (DummyFireParent.Velocity * TracerAdvance), SpawnRotation );
	NewTracer.SetOwner(DummyFireParent);
	NewTracer.bAllowTracersMovingFromTarget = TRUE;

	return NewTracer;
}

simulated function Actor GetTraceOwner()
{
	return DummyFireParent;
}

simulated function GearProj_BulletTracer SpawnTracerEffect( vector HitLocation, float HitDistance )
{
	local GearProj_BulletTracer NewTracer;

	NewTracer = Super.SpawnTracerEffect(HitLocation, HitDistance);
	NewTracer.Velocity += DummyFireParent.Velocity;

	NewTracer.Velocity += 2500.0 * vector(NewTracer.Rotation);

	return NewTracer;
}

defaultproperties
{
	TracerAdvance=0.033

	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_Uber_Reaver.Effects.P_Uber_Reaver_Gun_MuzzleFlash'
	End Object

	TracerType=WTT_BrumakPlayer
}