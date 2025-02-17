/**
 * Mortar Bomblet Projectile
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearProj_ClusterMortarBombletBase extends GearProj_ExplosiveBase
	abstract
	config(Weapon);

var() bool bShowDamageRadii;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	Acceleration.X = 0;
	Acceleration.Y = 0;
	Acceleration.Z = GetGravityZ() * GravityScale;
}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
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

// overridden for debug rendering
simulated function TriggerExplosion(vector HitLocation, vector HitNormal, Actor HitActor)
{
	super.TriggerExplosion(HitLocation, HitNormal, HitActor);

	if (bShowDamageRadii)
	{
		DrawDebugSphere(HitLocation, ExplosionTemplate.DamageRadius, 8, 255, 255, 0, TRUE);
	}
}


defaultproperties
{
	//bShowDamageRadii=TRUE
    LifeSpan=10.f

	//Speed=500					// velocity is explicitly set on spawn
    MaxSpeed=10000

    Physics=PHYS_Projectile
    GravityScale=2.f

    Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
        HiddenGame=false
        CastShadow=false
		CollideActors=false
		BlockActors=false
		BlockZeroExtent=false
		BlockNonZeroExtent=false
		BlockRigidBody=false
		bDisableAllRigidBody=TRUE
        Scale=1.0f
	Rotation=(Yaw=-16384)
	LightEnvironment=MyLightEnvironment
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

}
