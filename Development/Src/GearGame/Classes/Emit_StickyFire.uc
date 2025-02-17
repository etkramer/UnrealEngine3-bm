/** 
 * Semi-persistent bits of fire stuck to the world.  Deposited by flamethrower, etc.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Emit_StickyFire extends SpawnedGearEmitter
	deprecated
	config(Weapon);

var() protected const float		MaxDrawScale;

var() protected const float		GrowTime;
/** Specified TOTAL burn time, including Growtime and FadeTime */
var() protected const config vector2d	TotalBurnTimeRange;
var() protected const float				FadeTime;

var() protected const config float		DamagePerSecond;

var protected transient float	RandomizedTotalBurnTime;
var protected transient float	CurrentTime;

var() protected float			DamageApplicationInterval;
var protected float				NextDamageTime;


// Since these cause damage, we want the server to be authoritative about where they spawn.  
// So we want them to replicate on creation, but tear off immediately.


simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	NextDamageTime = WorldInfo.TimeSeconds + DamageApplicationInterval;
}

simulated function Tick(float DeltaTime)
{
	local float NewDrawScale;
	local GearPawn GP;

	super.Tick(DeltaTime);

	// hacktastic!  gotta love the prototype code.
	if (RandomizedTotalBurnTime == 0.f)
	{
		RandomizedTotalBurnTime = RandRange(TotalBurnTimeRange.X, TotalBurnTimeRange.Y);
		RandomizedTotalBurnTime = FMax(RandomizedTotalBurnTime, GrowTime+FadeTime);
	}

	CurrentTime += DeltaTime;

	// hack for now, until we see what the real effect needs
	NewDrawScale = 1.f;
	if (CurrentTime < GrowTime)
	{
		//NewDrawScale = Lerp(1.f, MaxDrawScale, CurrentTime/GrowTime);
	}
	else if (CurrentTime > RandomizedTotalBurnTime)
	{
		// turn off emitter, should result in destruction once particles die
		ParticleSystemComponent.DeactivateSystem();
	}
	else if (CurrentTime > (RandomizedTotalBurnTime - FadeTime))
	{
		// eww
		//NewDrawScale = Lerp(MaxDrawScale, 1.f, (CurrentTime-RandomizedTotalBurnTime+FadeTime)/FadeTime);
	}
	else
	{
		NewDrawScale = MaxDrawScale;
	}

	SetDrawScale(NewDrawScale);

	// do damage!
	if (Role == ROLE_Authority)
	{
		if (WorldInfo.TimeSeconds > NextDamageTime)
		{
			ForEach TouchingActors(class'GearPawn', GP)
			{
				if (!GP.IsEvading())
				{
					GP.TakeDamage(DamagePerSecond*DamageApplicationInterval, Instigator.Controller, GP.Location, vect(0,0,1), class'GDT_Fire');
				}
			}

			NextDamageTime = WorldInfo.TimeSeconds + DamageApplicationInterval;
		}
	}
}


defaultproperties
{
	Sound=None
//	ParticleSystem=ParticleSystem'COG_Flamethrower.Effects.P_FX_Flamethrower_GroundBurn_01'

	bNetTemporary=TRUE
	RemoteRole=ROLE_SimulatedProxy

	bCollideActors=true
	Begin Object Class=CylinderComponent Name=CollisionCylinder0
		CollisionRadius=+0020.000000
		CollisionHeight=+0010.000000
		CollideActors=TRUE
	End Object
	CollisionComponent=CollisionCylinder0
	Components.Add(CollisionCylinder0)

	GrowTime=1.f
	FadeTime=2.f
	MaxDrawScale=5.f

	DamageApplicationInterval=0.25f

}
