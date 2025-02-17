/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTWeap_BioRifle_Content extends UTWeapon;

/** maximum number of globs we can load for one shot */
var int MaxGlobStrength;

var int GlobStrength;

var SoundCue WeaponLoadSnd;

var float AdditionalCoolDownTime;
var float CoolDownTime;

/** Array of all the animations for the various primary fires*/
var array<name> PrimaryFireAnims; // special case for Bio; if this needs to be promoted perhaps an array of arrays?

/** arm animations corresponding to above*/
var array<name> PrimaryArmAnims;

/** the primary fire animation currently playing */
var int CurrentFireAnim;

var ParticleSystemComponent ChargingSystem;
var name WeaponChargeAnim;
var name ArmsChargeAnim;

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	SkeletalMeshComponent(Mesh).AttachComponentToSocket(ChargingSystem,MuzzleFlashSocket);
}
/*********************************************************************************************
 * Hud/Crosshairs
 *********************************************************************************************/

simulated event float GetPowerPerc()
{
	return 0.0;
}

/**
 * GetAdjustedAim begins a chain of function class that allows the weapon, the pawn and the controller to make
 * on the fly adjustments to where this weapon is pointing.
 */
simulated function Rotator GetAdjustedAim( vector StartFireLoc )
{
	local rotator R;

	// Start the chain, see Pawn.GetAdjustedAimFor()
	if( Instigator != None )
	{
		R = Instigator.GetAdjustedAimFor( Self, StartFireLoc );

		if ( PlayerController(Instigator.Controller) != None )
		{
			R.Pitch = R.Pitch & 65535;
			if ( R.Pitch < 16384 )
			{
				R.Pitch += (16384 - R.Pitch)/16;
			}
			else if ( R.Pitch > 49152 )
			{
				R.Pitch += 1024;
			}
		}
	}

	return R;
}

/**
 * Take the projectile spawned and if it's the proper type, adjust it's strength and speed
 */
simulated function Projectile ProjectileFire()
{
	local Projectile SpawnedProjectile;

	SpawnedProjectile = super.ProjectileFire();
	if ( UTProj_BioGlob(SpawnedProjectile) != None )
	{
		UTProj_BioGlob(SpawnedProjectile).InitBio(self, GlobStrength);
	}

	return SpawnedProjectile;
}

/**
 * Tells the weapon to play a firing sound (uses CurrentFireMode)
 */
simulated function PlayFiringSound()
{
	if (CurrentFireMode<WeaponFireSnd.Length)
	{
		// play weapon fire sound
		if ( WeaponFireSnd[CurrentFireMode] != None )
		{
			MakeNoise(1.0);
			if(CurrentFireMode == 1 && GetPowerPerc() > 0.75)
			{
				WeaponPlaySound( WeaponFireSnd[2] );
			}
			else
			{
				WeaponPlaySound( WeaponFireSnd[CurrentFireMode] );
			}
		}
	}
}
/*********************************************************************************************
 * State WeaponLoadAmmo
 * In this state, ammo will continue to load up until MAXLOADCOUNT has been reached.  It's
 * similar to the firing state
 *********************************************************************************************/

simulated state WeaponLoadAmmo
{
	simulated function WeaponEmpty();

	simulated event float GetPowerPerc()
	{
		local float p;
		p = float(GlobStrength) / float(MaxGlobStrength);
		p = FClamp(p,0.0,1.0);

		return p;
	}

	simulated function bool TryPutdown()
	{
		bWeaponPutDown = true;
		return true;
	}

	/**
	 * Adds a rocket to the count and uses up some ammo.  In Addition, it plays
	 * a sound so that other pawns in the world can here it.
	 */
	simulated function IncreaseGlobStrength()
	{
		if (GlobStrength < MaxGlobStrength && HasAmmo(CurrentFireMode))
		{
			// Add the glob
			GlobStrength++;
			ConsumeAmmo(CurrentFireMode);
		}
	}

	function bool IsFullyCharged()
	{
		return (GlobStrength >= MaxGlobStrength);
	}

	/**
	 * Fire off a shot w/ effects
	 */
	simulated function WeaponFireLoad()
	{
		ProjectileFire();
		PlayFiringSound();
		InvManager.OwnerEvent('FiredWeapon');
	}

	/**
	 * This is the timer event for each shot
	 */
	simulated event RefireCheckTimer()
	{
		IncreaseGlobStrength();
	}

	simulated function SendToFiringState( byte FireModeNum )
	{
		return;
	}


	/**
	 * We need to override EndFire so that we can correctly fire off the
	 * current load if we have any.
	 */
	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);

		if (FireModeNum == CurrentFireMode)
		{
			// Fire the load
			ChargingSystem.DeactivateSystem();

			WeaponFireLoad();

			// Cool Down
			AdditionalCoolDownTime = GetTimerRate('RefireCheckTimer') - GetTimerCount('RefireCheckTimer');
			GotoState('WeaponCoolDown');
		}
	}

	/**
	 * Initialize the loadup
	 */
	simulated function BeginState(Name PreviousStateName)
	{
		local UTPawn POwner;
		local UTAttachment_BioRifle ABio;

		GlobStrength = 0;

		super.BeginState(PreviousStateName);

		POwner = UTPawn(Instigator);
		if (POwner != None)
		{
			POwner.SetWeaponAmbientSound(WeaponLoadSnd);
			ABio = UTAttachment_BioRifle(POwner.CurrentWeaponAttachment);
			if(ABio != none)
			{
				ABio.StartCharging();
			}
		}
		ChargingSystem.ActivateSystem();
		PlayWeaponAnimation( WeaponChargeAnim, MaxGlobStrength*FireInterval[1], false);
		PlayArmAnimation(ArmsChargeAnim, MaxGlobStrength*FireInterval[1],false);
	}

	/**
	 * Insure that the GlobStrength is 1 when we leave this state
	 */
	simulated function EndState(Name NextStateName)
	{
		local UTPawn POwner;

		Cleartimer('RefireCheckTimer');

		GlobStrength = 1;

		POwner = UTPawn(Instigator);
		if (POwner != None)
		{
			POwner.SetWeaponAmbientSound(None);
			POwner.SetFiringMode(0); // return to base fire mode for network anims
		}
		ChargingSystem.DeactivateSystem();

		Super.EndState(NextStateName);
	}

	simulated function bool IsFiring()
	{
		return true;
	}

	/**
	 * This determines whether or not the Weapon can have ViewAcceleration when Firing.
	 *
	 * When you are FULLY charged up and running around the level looking for someone to Glob,
	 * you need to be able to view accelerate
	 **/
	simulated function bool CanViewAccelerationWhenFiring()
	{
		return TRUE;
	}


Begin:
	IncreaseGlobStrength();
	TimeWeaponFiring(CurrentFireMode);

}

simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	if(FireModeNum == 0)
	{
		CurrentFireAnim = (CurrentFireAnim+1)%(PrimaryFireAnims.Length);
		WeaponFireAnim[0] = PrimaryFireAnims[CurrentFireAnim];
		ArmFireAnim[0] = PrimaryArmAnims[CurrentFireAnim];
	}
	super.PlayFireEffects(FireModeNum,HitLocation);
}

simulated state WeaponCoolDown
{
	simulated function WeaponCooled()
	{
		GotoState('Active');
	}

	simulated function EndState(name NextStateName)
	{
		ClearFlashCount();
		ClearFlashLocation();

		ClearTimer('WeaponCooled');
		super.EndState(NextStateName);
	}

begin:
	SetTimer(CoolDownTime + AdditionalCoolDownTime,false,'WeaponCooled');
}

simulated function WeaponCooled()
{
	`log("BioRifle: Weapon Cooled outside WeaponCoolDown, is in"@GetStateName());
}

// AI Interface
function float GetAIRating()
{
	local UTBot B;
	local float EnemyDist;
	local vector EnemyDir;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return AIRating;

	// if retreating, favor this weapon
	EnemyDir = B.Enemy.Location - Instigator.Location;
	EnemyDist = VSize(EnemyDir);
	if ( EnemyDist > 1500 )
		return 0.1;
	if ( B.IsRetreating() )
		return (AIRating + 0.4);
	if ( (B.Enemy.Weapon != None) && B.Enemy.Weapon.bMeleeWeapon )
		return (AIRating + 0.35);
	if ( -1 * EnemyDir.Z > EnemyDist )
		return AIRating + 0.1;
	if ( EnemyDist > 1000 )
		return 0.35;
	return AIRating;
}

/* BestMode()
choose between regular or alt-fire
*/
function byte BestMode()
{
	if ( FRand() < 0.8 )
		return 0;
	return 1;
}

function float SuggestAttackStyle()
{
	local UTBot B;
	local float EnemyDist;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return 0.4;

	EnemyDist = VSize(B.Enemy.Location - Instigator.Location);
	if ( EnemyDist > 1500 )
		return 1.0;
	if ( EnemyDist > 1000 )
		return 0.4;
	return -0.4;
}

function float SuggestDefenseStyle()
{
	local UTBot B;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return 0;

	if ( VSize(B.Enemy.Location - Instigator.Location) < 1600 )
		return -0.6;
	return 0;
}

defaultproperties
{
	WeaponColor=(R=64,G=255,B=64,A=255)

	PlayerViewOffset=(X=-1.0,Y=-2.0,Z=0.0)
	EquipTime=0.8
	DrawScale3D=(X=1.0,Y=1.05,Z=1.0)

	Begin Object class=AnimNodeSequence Name=MeshSequenceA
	End Object

	Begin Object Name=FirstPersonMesh
		SkeletalMesh=SkeletalMesh'WP_BioRifle.Mesh.SK_WP_BioRifle_1P'
		AnimSets(0)=AnimSet'WP_BioRifle.Anims.K_WP_BioRifle_Base'
		Animations=MeshSequenceA
		PhysicsAsset=None
		Scale=1.0
		Translation=(X=0,Y=0,Z=0)
		Rotation=(Yaw=0)
		FOV=60.0
	End Object


	MuzzleFlashSocket=Bio_MF
	MuzzleFlashPSCTemplate=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_MF'
	MuzzleFlashDuration=0.33

	FireInterval(0)=+0.35
	FireInterval(1)=+0.35
	WeaponFireTypes(0)=EWFT_Projectile

	Spread(0)=0.0

	WeaponProjectiles(0)=class'UTProj_BioShot'
	WeaponProjectiles(1)=class'UTProj_BioGlob'

	WeaponPutDownSnd=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_Lower_Cue'
	WeaponEquipSnd=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_Raise_Cue'

	WeaponFireSnd[0]=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireMain_Cue'
	WeaponFireSnd[1]=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireAltLarge_Cue'
	WeaponFireSnd[2]=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireAltSmall_Cue'

	PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_CGBio_Cue'

	WeaponLoadSnd=SoundCue'A_Weapon_BioRifle.Weapon.A_BioRifle_FireAltChamber_Cue'

	MaxDesireability=0.75
	AIRating=+0.55
	CurrentRating=0.55
	bInstantHit=false
	bSplashJump=false
	bRecommendSplashDamage=false
	bSniping=false
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0

	GlobStrength=1
	MaxGlobStrength=10
	GroupWeight=0.51
	FireOffset=(X=19,Y=10,Z=-10)

	FiringStatesArray(1)=WeaponLoadAmmo

	AmmoCount=25
	LockerAmmoCount=50
	MaxAmmoCount=50

	LockerRotation=(Pitch=0,Roll=-16384)

	IconX=382
	IconY=82
	IconWidth=27
	IconHeight=42

	AttachmentClass=class'UTGame.UTAttachment_BioRifle'

	// Pickup Mesh
	Begin Object Name=PickupMesh
		SkeletalMesh=SkeletalMesh'WP_BioRifle.Mesh.SK_WP_BioRifle_3P_mid'
	End Object

	InventoryGroup=3

 	WeaponFireAnim(0)=WeaponFire1
 	WeaponFireAnim(1)=WeaponAltFire
 	WeaponPutDownAnim=WeaponPutDown
	WeaponEquipAnim=WeaponEquip
	WeaponChargeAnim=WeaponAltCharge

	CoolDownTime=0.33;
	CrossHairCoordinates=(U=192,V=0,UL=64,VL=64)
	ArmsAnimSet=AnimSet'WP_BioRifle.Anims.K_WP_BioRifle_Arms'
	ArmFireAnim(0)=WeaponFire1
	ArmFireAnim(1)=WeaponAltFire
	ArmsPutDownAnim=WeaponPutDown
	ArmsEquipAnim=WeaponEquip
	ArmsChargeAnim=WeaponAltCharge


	PrimaryFireAnims(0)=WeaponFire1
	PrimaryFireAnims(1)=WeaponFire2
	PrimaryFireAnims(2)=WeaponFire3
	PrimaryArmAnims(0)=WeaponFire1
	PrimaryArmAnims(1)=WeaponFire2
	PrimaryArmAnims(2)=WeaponFire3
	CurrentFireAnim=0;
	IconCoordinates=(U=600,V=399,UL=128,VL=62)

	QuickPickGroup=1
	QuickPickWeight=0.8

	Begin Object Class=ParticleSystemComponent Name=ChargePart
		Template=ParticleSystem'WP_BioRifle.Particles.P_WP_Bio_Alt_MF'
		bAutoActivate=false
		DepthPriorityGroup=SDPG_Foreground
	End Object
	ChargingSystem=ChargePart
	Components.Add(ChargePart)
}
