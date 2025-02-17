/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTProj_FlakShard extends UTProjectile
	native;

/** # of times they can bounce */
var int Bounces;

/** How fast damage is reduced per second from when the chunk is fired */
var float DamageAttenuation;

var float ShrinkTimer;

var ParticleSystem BounceTemplate;
var ParticleSystem RockSmokeTemplate;

/** reference to impact effect we created; we wait for this before being destroyed */
var ParticleSystemComponent LastImpactEffect;

var MaterialSoundEffect DefaultHitSound;
var SoundCue HitPawnSound;

/** if set, play camera anim for killer when kill at very short range (set to false for shards spawned by alt shell) */
var bool bCheckShortRangeKill;
/** the camera anim to play */
var CameraAnim ShortRangeKillAnim;

/* if true, shard is fading out */
var bool bShrinking;

cpptext
{
	virtual void TickSpecial(float DeltaTime);
}

function Init(vector Direction)
{
	Super.Init(Direction);

	if (PhysicsVolume.bWaterVolume)
	{
		Velocity *= 0.65;
	}

	SetRotation(RotRand());
}

simulated function SpawnFlightEffects()
{
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		if (WorldInfo.IsConsoleBuild())
		{
			// spawning all those shards at once is expensive so we delay the flight effects a tick to spread out the work a little
			SetTimer(WorldInfo.DeltaSeconds + 0.01, false, 'InternalSpawnFlightEffects');
		}
		else
		{
			Super.SpawnFlightEffects();
		}
	}
}

simulated function InternalSpawnFlightEffects()
{
	if (!bShuttingDown)
	{
		Super.SpawnFlightEffects();
	}
}

/**
  * Attenuate shard damage at long range
  */
simulated function float GetDamage(Actor Other, vector HitLocation)
{
	return Max(5, Damage - DamageAttenuation * FMax(0.f, (default.Lifespan - LifeSpan - 1)));
}

simulated function float GetMomentumTransfer()
{
	return MomentumTransfer;
}

simulated function ProcessTouch(Actor Other, Vector HitLocation, Vector HitNormal)
{
	local Vehicle V;
	local UTPlayerController PC;

	if ( (UTProj_FlakShard(Other) == none) && (Physics == PHYS_Falling || Other != Instigator) )
	{
		speed = VSize(Velocity);
		if (Speed > 400)
		{
			Other.TakeDamage(GetDamage(Other, HitLocation), InstigatorController, HitLocation, GetMomentumTransfer() * Normal(Velocity), MyDamageType,, self);
			if ( Role == ROLE_Authority )
			{
				V = Vehicle(Other);
				if ( V != None)
				{
					if ( PlayerController(V.Controller) != None )
					{
						PlayerController(V.Controller).ClientPlaySound(DefaultHitSound.Sound);
					}
					bWaitForEffects = SpawnImpactEffect(HitLocation, HitNormal);
				}
				else if (Pawn(Other) != None)
				{
					PlaySound(HitPawnSound, true);

					// play camera anim if short range kill
					if ( bCheckShortRangeKill && Pawn(Other).Health < 0 && Physics == PHYS_Projectile && Instigator != None &&
						VSize(Instigator.Location - Other.Location) < 150.0 )
					{
						PC = UTPlayerController(InstigatorController);
						if (PC != None)
						{
							PC.ClientPlayCameraAnim(ShortRangeKillAnim);
						}
					}
				}
			}
		}
		Shutdown();
	}
}

simulated function Landed(vector HitNormal, Actor FloorActor)
{
	HitWall(HitNormal, FloorActor, None);
}

simulated function bool SpawnImpactEffect(vector HitLocation, vector HitNormal)
{
	if (EffectIsRelevant(HitLocation, false, MaxEffectDistance))
	{
		LastImpactEffect = new(Outer) class'UTParticleSystemComponent';
		LastImpactEffect.SetAbsolute(true, true, true);
		LastImpactEffect.SetTranslation(HitLocation);
		LastImpactEffect.SetRotation(rotator(HitNormal));
		LastImpactEffect.SetTemplate((Bounces > 0) ? BounceTemplate : RockSmokeTemplate);
		LastImpactEffect.OnSystemFinished = MyOnParticleSystemFinished;
		AttachComponent(LastImpactEffect);
		return true;
	}
	return false;
}

simulated event HitWall(vector HitNormal, Actor Wall, PrimitiveComponent WallComp)
{
	local audioComponent HitSoundComp;
	local bool bSpawnedImpactEffect;

	bBlockedByInstigator = true;
	bSpawnedImpactEffect = SpawnImpactEffect(Location, HitNormal);

	if ( !Wall.bStatic && !Wall.bWorldGeometry && Wall.bProjTarget )
	{
		Wall.TakeDamage(GetDamage(Wall, Location), InstigatorController, Location, GetMomentumTransfer() * Normal(Velocity), MyDamageType,, self);
		bWaitForEffects = bSpawnedImpactEffect;
		Shutdown();
	}
	else
	{
		ImpactSound = DefaultHitSound.Sound;
		SetPhysics(PHYS_Falling);
		if (Bounces > 0)
		{
			if ( Bounces == 1 )
			{
				LifeSpan += 0.5;
			}
			if ((!WorldInfo.bDropDetail && FRand() < 0.4) || (UTProj_FlakShardMain(self) != none))
			{

				HitSoundComp = CreateAudioComponent( ImpactSound, false, false, true, location );
				if(HitSoundComp != none)
				{
					HitSoundComp.VolumeMultiplier = (Bounces > 1) ? 1.0 : 0.5;
					HitSoundComp.Play();
				}

			}

			Velocity = 0.6 * (Velocity - 2.0 * HitNormal * (Velocity dot HitNormal));
			Bounces = Bounces - 1;
		}
		else
		{
			if (ProjEffects != None)
			{
				ProjEffects.DeactivateSystem();
			}
			bBounce = false;
			SetPhysics(PHYS_None);
			SetTimer(0.5 * FRand(), false, 'StartToShrink');
		}
	}
}

simulated function StartToShrink()
{
	RemoteRole = ROLE_None;
	bShrinking = true;
	ShrinkTimer = FMin(LifeSpan,0.75);
	if ( WorldInfo.NetMode == NM_DedicatedServer )
	{
		Destroy();
	}
	else if ( !bWaitForEffects )
	{
		LifeSpan = ShrinkTimer;
	}
}

simulated function MyOnParticleSystemFinished(ParticleSystemComponent PSC)
{
	if ( bWaitForEffects && (PSC == LastImpactEffect) )
	{
		Destroy();
	}
	else
	{
		if ( PSC == LastImpactEffect )
		{
			LastImpactEffect = None;
		}
		DetachComponent(PSC);
	}
}

defaultproperties
{
	speed=3500.000000
	MaxSpeed=3500.000000
	Damage=18
	DamageRadius=+0.0
	DamageAttenuation=5.0
	MaxEffectDistance=5000.0

	MomentumTransfer=14000
	MyDamageType=class'UTDmgType_FlakShard'
	LifeSpan=2.0
	RotationRate=(Roll=50000)
	DesiredRotation=(Roll=30000)
	bCollideWorld=true
	CheckRadius=20.0

	bBounce=true
	Bounces=2
	NetCullDistanceSquared=+49000000.0

	ProjFlightTemplate=ParticleSystem'WP_FlakCannon.Effects.P_WP_Flak_trails'

	ImpactSound=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireImpactDirtCue'
	DefaultHitSound=(Sound=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireImpactDirtCue')
	HitPawnSound=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireImpactFleshCue'

	bWaitForEffects=false

	BounceTemplate=ParticleSystem'WP_FlakCannon.Effects.P_WP_Flak_Rock_bounce'
	RockSmokeTemplate=ParticleSystem'WP_FlakCannon.Effects.P_WP_Flak_rocksmoke'

	bCheckShortRangeKill=true
	ShortRangeKillAnim=CameraAnim'Camera_FX.Gameplay.C_Impact_CharacterGib_Near'
}
