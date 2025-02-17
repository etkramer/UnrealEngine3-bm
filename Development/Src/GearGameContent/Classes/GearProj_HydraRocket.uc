/**
 * Hydra Rocket Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_HydraRocket extends GearProj_HomingRocket
	config(Weapon);

var StaticMeshComponent MeshComp;
var int DamageToDetonate, TakenDamage;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	SetTimer( 0.25f,FALSE,nameof(EnableCollision) );
}

function EnableCollision()
{
	MeshComp.SetTraceBlocking(TRUE,FALSE);
	SetCollision(TRUE,FALSE);
}

simulated function TakeDamage(int Dmg, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	if (GearExplosionActor(DamageCauser) == None)
	{
		TakenDamage += Dmg;
		if(TakenDamage >= DamageToDetonate)
		{
			TriggerExplosion(Location,vect(0,0,1),None);
		}
 	}

	super.TakeDamage(Dmg, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
}

defaultproperties
{
	DamageToDetonate=10
	bProjTarget=TRUE

//	Speed=3000			// set RocketLaunchSpeed in the weapons ini
	MaxSpeed=5000
	MyDamageType=class'GDT_HydraRocket'

	bUseInterpPhysics=TRUE

	TrailTemplate=ParticleSystem'Locust_Uber_Reaver.Effects.P_Uber_Reaver_Projectile_Trail'

	Begin Object Name=CollisionCylinder
		CollideActors=FALSE
	End Object

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_Brumak.Locust_Brumak_Rocket_SM'
		HiddenGame=false
		CastShadow=false
		CollideActors=TRUE
		BlockActors=false
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
		Scale=2.25
		Rotation=(Yaw=-16384)
		LightEnvironment=MyLightEnvironment
		bAcceptsDynamicDecals=FALSE
	End Object
	MeshComp=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	InFlightSoundTemplate=SoundCue'Weapon_Boomer.Firing.BoomerInAirLoopCue'

	// explosion point light
	Begin Object Class=PointLightComponent Name=ExploLight0
		Radius=400.000000
		Brightness=500.000000
		LightColor=(B=35,G=185,R=255,A=255)
		Translation=(X=16)
		CastShadows=FALSE
		CastStaticShadows=FALSE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		bEnabled=FALSE
	End Object

	// explosion
	Begin Object Class=GearExplosion Name=ExploTemplate0
		MyDamageType=class'GDT_HydraRocket'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType

		ParticleEmitterTemplate=ParticleSystem'Locust_Uber_Reaver.Effects.P_Uber_Reaver_Projectile_Impact_Explo'
		ExplosionSound=SoundCue'Weapon_Boomer.Impacts.BoomerExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bDoExploCameraAnimShake=TRUE
		ExploAnimShake=(AnimScale=0.35,AnimBlendInTime=0.1f,bUseDirectionalAnimVariants=TRUE,Anim=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Front',Anim_Left=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Left',Anim_Right=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Right',Anim_Rear=CameraAnim'Effects_Camera.Explosions.CA_Grenade_Explo_Back')
		ExploShakeInnerRadius=1300
		ExploShakeOuterRadius=3000

		FractureMeshRadius=100.0
		FracturePartVel=300.0

		bAllowPerMaterialFX=TRUE
	End Object
	ExplosionTemplate=ExploTemplate0
}


