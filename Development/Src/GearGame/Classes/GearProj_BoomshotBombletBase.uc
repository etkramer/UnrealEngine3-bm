/**
 * Boomshot Bomblet Weapon Projectile (emitted by GearProj_Boomshot, like a cluster bomb).
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_BoomshotBombletBase extends GearProj_ExplosiveBase
	abstract
	config(Weapon);

/**
 * This will tell the client how long to set the Client's LifeSpan to.
 * This will made the bomblets correctly explode in client server games
 **/
var repnotify float TimeTilExplosion;

replication
{
	if( Role == ROLE_Authority )
		TimeTilExplosion;
}

simulated event ReplicatedEvent( name VarName )
{
	if( VarName == 'TimeTilExplosion' )
	{
		//`Log( "ReplicatedEvent: " $ VarName $ " TimeTilExplosion: " $ TimeTilExplosion  );
		LifeSpan = TimeTilExplosion ;
	}
	else
	{
		super.ReplicatedEvent( VarName );
	}
}

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Acceleration.X = 0;
	Acceleration.Y = 0;
	Acceleration.Z = GetGravityZ() * GravityScale;

}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	//if ( Other == Instigator )
	//	return;

	if ( !Other.IsA('Projectile') || Other.bProjTarget )
	{
		if ( Other != Instigator )
		{
			TriggerExplosion(Location, HitNormal, Other);
		}
	}
}


simulated function HitWall(vector HitNormal, actor Wall, PrimitiveComponent WallComp)
{
	TriggerExplosion(Location, HitNormal, None);
}

simulated event Destroyed()
{
	if (!bHasExploded)
	{
		TriggerExplosion(Location, -Normal(Velocity), None);
	}
	Super.Destroyed();
}


defaultproperties
{
	Speed=500
	MaxSpeed=3000

	Physics=PHYS_Projectile
	GravityScale=0.65f

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
        HiddenGame=false
        CastShadow=false
		CollideActors=false
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
        Scale=1.0
		Rotation=(Yaw=-16384)
    End Object
	Components.Add(StaticMeshComponent0)


	// remove light environment
	Begin Object Name=MyLightEnvironment
	    bEnabled=FALSE
	End Object
	Components.Remove(MyLightEnvironment)
	ProjLightEnvironment=None


	RemoteRole=ROLE_SimulatedProxy

	bNetInitialRotation=true

	// explosion point light
    Begin Object Class=PointLightComponent Name=ExploLight0
        Radius=400.000000
        Brightness=200.000000
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
		MyDamageType=class'GDT_Boomshot'
		MomentumTransferScale=1.f	// Scale momentum defined in DamageType
	
		ParticleEmitterTemplate=ParticleSystem'Locust_Boomshot.Effects.P_Boomshot_Explo_littleones'
		ExplosionSound=SoundCue'Weapon_Boomer.Impacts.BoomerExplosionCue'
		ExploLight=ExploLight0
		ExploLightFadeOutTime=0.17f

		bCausesFracture=FALSE
	End Object
	ExplosionTemplate=ExploTemplate0
}


