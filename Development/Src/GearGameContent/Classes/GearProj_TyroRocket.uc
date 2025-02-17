/**
 * RPG Rocket Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_TyroRocket extends GearProjectile
	config(Weapon);

/** Impact Effect particle system */
var		ParticleSystem		ImpactEffect;
/** explosion sound */
var		SoundCue			ExplosionSound;

/** Screenshake to play when this grenade explodes */
var()	ScreenShakeStruct		ExploShake;
/** Radius to within which to play full-powered screenshake (will be scaled within radius) */
var()	float					ExploShakeInnerRadius;
/** Between inner and outer radii, scale shake from full to zero */
var()	float					ExploShakeOuterRadius;


simulated function Destroyed()
{
	// Clean up trail if spawned
	if( TrailEmitter != None )
	{
		TrailEmitter.bDestroyOnSystemFinish = true;
		TrailEmitter.ParticleSystemComponent.DeactivateSystem();
		TrailEmitter.SetBase( None );
	}

	super.Destroyed();
}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	if ( Other == Instigator )
		return;

	if ( !Other.IsA('Projectile') || Other.bProjTarget )
	{
		if ( Other != Instigator )
		{
			Explode(HitLocation, HitNormal);
		}
	}
}

simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	Explode(Location, HitNormal);
}

simulated function Explode(vector HitLocation, vector HitNormal)
{
	if ( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// @fixme jf: set up using new explosion system if necessary
//		Instigator.Spawn( class'Emit_FragGrenadeExplosionInAir',,, HitLocation + HitNormal*3, rotator(HitNormal) );
		class'GearPlayerCamera'.static.PlayWorldCameraShake(ExploShake, self, HitLocation, ExploShakeInnerRadius, ExploShakeOuterRadius, 2.0f, TRUE );
	}

	if ( Role == Role_Authority )
	{	// Hurt Radius
		HurtRadius( default.Damage, default.DamageRadius, default.MyDamageType, default.MomentumTransfer, HitLocation+HitNormal*3);

		MakeNoise( 1.0 );
	}

	Destroy();
}

defaultproperties
{
	Speed=3000
	MaxSpeed=3000
	Damage=200
	DamageRadius=400
	MyDamageType=class'GDT_FragGrenade'
	MomentumTransfer=1600

	ExplosionSound=none
	ExploShake=(TimeDuration=1.f,FOVAmplitude=0,LocAmplitude=(X=1,Y=1,Z=1),LocFrequency=(X=5,Y=50,Z=100),RotAmplitude=(X=300,Y=100,Z=200),RotFrequency=(X=90,Y=40,Z=75))
	ExploShakeInnerRadius=256
	ExploShakeOuterRadius=768

	ImpactEffect=ParticleSystem'COG_Frag_Grenade.EffectS.P_COG_Frag_Grenade_Air_Burst'
	TrailTemplate=ParticleSystem'COG_RPG.Particle_Systems.Smoke_trail_FX'

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
    	StaticMesh=StaticMesh'COG_AssaultRifle.Mesh.BulletChunk'
        HiddenGame=false
        CastShadow=false
		CollideActors=false
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
        Scale=0.25
    End Object
	Components.Add(StaticMeshComponent0)

	RemoteRole=ROLE_SimulatedProxy

	// remove light environment
	Begin Object Name=MyLightEnvironment
	    bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None

    Lifespan=2.0
	bNetInitialRotation=true
}
