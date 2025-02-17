/**
 * Boomer Weapon Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_RocketCannon extends GearProj_Boomshot
	config(Weapon);

// this is the centaur cannon

defaultproperties
{
	Speed=15000
	MaxSpeed=15000
	LifeSpan=5.000

	GravityScale=0.f
	bIgnoreInstigatorCollision=TRUE

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_Centaur.Meshes.S_Centaur_Projectile'
	End Object

	TrailTemplate=ParticleSystem'COG_Centaur.Effects.P_Centaur_Projectile_Trail'

	InFlightSoundTemplate=SoundCue'Vehicle_Centaur.Centaur.Centaur_RocketInAirLoopBCue'

	// explosion point light
	Begin Object Name=ExploLight0
		Radius=1200.000000
		Brightness=1000.000000
		LightColor=(B=76,G=107,R=249,A=255)
	End Object

	// explosion
	Begin Object Name=ExploTemplate0
		MyDamageType=class'GDT_RocketCannon'
		ExploLightFadeoutTime=0.3f
		ParticleEmitterTemplate=ParticleSystem'COG_Centaur.Effects.P_COG_Centaur_Impact_Explo'
		ExplosionSound=SoundCue'Vehicle_Centaur.Centaur.Centaur_RocketExplosionCue'

		FractureMeshRadius=500.0
		FracturePartVel=300.0

		bAllowPerMaterialFX=TRUE
	End Object
}


