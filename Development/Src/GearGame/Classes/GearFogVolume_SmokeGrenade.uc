/**
 * Spawnable smoke grenade fog volume.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearFogVolume_SmokeGrenade extends GearFogVolume_Spawnable
	config(Weapon)
	native(Weapon);

cpptext
{
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);
}

function PostBeginPlay()
{
	Super.PostBeginPlay();

	VisBlocker = new(self) AutomaticMeshComponent.Class(AutomaticMeshComponent);
	VisBlocker.SetScale(AutomaticMeshComponent.Scale * VisBlockerScale);
	VisBlocker.SetBlockRigidBody(false);
	VisBlocker.SetActorCollision(true, false, false);
	VisBlocker.SetTraceBlocking(true, false);
	VisBlocker.SetHidden(true);
	AttachComponent(VisBlocker);
}



defaultproperties
{
	FogVolumeArchetype=FogVolumeSphericalDensityInfo'Effects_FogVolumes.FogVolumes.FV_Spherical_SmokeGrenade'
	DamageTypeToUseForPerLevelMaterialEffects=class'GDT_SmokeGrenade'

	FadeInTime=2.f
	FadeOutTime=2.f

	bMovable=false
	bCollideActors=true
	bBlockActors=false
	bCollideWorld=false
	bCollideWhenPlacing=false
}
