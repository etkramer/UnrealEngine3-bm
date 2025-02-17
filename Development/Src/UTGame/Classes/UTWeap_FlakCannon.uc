/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTWeap_FlakCannon extends UTWeapon
native;

var float SpreadDist;
var UTSkeletalMeshComponent SkeletonFirstPersonMesh;

var int curTensOdometer;
var int curOnesOdometer;
var float OdometerMaxPerSecOnes;
var float OdometerMaxPerSecTens;
var name OnesPlaceSkelName;
var name TensPlaceSkelName;
var class<Projectile> CenterShardClass;

cpptext
{
	virtual void TickSpecial( FLOAT DeltaSeconds );
}

simulated function Projectile ProjectileFire()
{
	local vector		RealStartLoc;
	local UTProj_FlakShell	SpawnedProjectile;

	// tell remote clients that we fired, to trigger effects
	IncrementFlashCount();

	if( Role == ROLE_Authority )
	{
		// this is the location where the projectile is spawned.
		RealStartLoc = GetPhysicalFireStartLoc();

		// Spawn projectile
		SpawnedProjectile = Spawn(class'UTProj_FlakShell',,, RealStartLoc);
		if( SpawnedProjectile != None && !SpawnedProjectile.bDeleteMe )
		{
			SpawnedProjectile.Init( Vector(GetAdjustedAim( RealStartLoc )) );
		}

		// Return it up the line
		return SpawnedProjectile;
	}

	return None;
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

		if ( (PlayerController(Instigator.Controller) != None) && (CurrentFireMode == 1) )
		{
			R.Pitch = R.Pitch & 65535;
			if ( R.Pitch < 16384 )
			{
				R.Pitch += (16384 - R.Pitch)/32;
			}
			else if ( R.Pitch > 49152 )
			{
				R.Pitch += 512;
			}
		}
	}

	return R;
}

simulated event ReplicatedEvent(name VarName)
{
	if ( VarName == 'AmmoCount')
	{
		UpdateAmmoCount();
	}

	Super.ReplicatedEvent(VarName);
}

simulated state WeaponFiring
{
	simulated event ReplicatedEvent(name VarName)
	{
		if ( VarName == 'AmmoCount')
		{
			UpdateAmmoCount();
		}
		Super.ReplicatedEvent(VarName);
	}
}

function int AddAmmo( int Amount )
{
	local int rvalue;
	rvalue = super.AddAmmo(Amount);
	UpdateAmmoCount();
	return rvalue;
}

simulated function HideFlakCannonAmmo()
{
    local SkelControlSingleBone SkelControl;
    if (AmmoCount <= 1)
    {
	    //Hide the flak cannon ammo in the chamber
	    SkelControl = SkelControlSingleBone(SkeletonFirstPersonMesh.FindSkelControl('AmmoScale'));
	    if(SkelControl != none)
	    {
	    	SkelControl.SetSkelControlStrength(1.0f, 0.0f);
	    }
    }
}

simulated function UpdateAmmoCount()
{
	local SkelControlSingleBone SkelControl;
	local int AmmoCountOnes;
	local int AmmoCountTens;

	if(Instigator.IsHumanControlled() && Instigator.IsLocallyControlled())
	{
		AmmoCountOnes = AmmoCount%10;
		AmmoCountTens = (AmmoCount - AmmoCountOnes)/10;
		SkelControl = SkelControlSingleBone(SkeletonFirstPersonMesh.FindSkelControl('OnesDisplay'));
		if(SkelControl != none)
		{
			SkelControl.BoneRotation.Pitch = -AmmoCountOnes*6554;
		}
		SkelControl = SkelControlSingleBone(SkeletonFirstPersonMesh.FindSkelControl('TensDisplay'));
		if(SkelControl != none)
		{
			SkelControl.BoneRotation.Pitch = -AmmoCountTens*6554;
		}

	    if (AmmoCount == 1)
    	{
		    //Hide the flak cannon ammo in the chamber
		    //Delay the hiding a fraction of time so the user doesn't see it happen	(time from anim)
	    	SetTimer(0.40f, false, 'HideFlakCannonAmmo');
    	}
    	else if (AmmoCount > 1)
    	{
	    	//Show the flak cannon ammo in the chamber
	    	ClearTimer('HideFlakCannonAmmo');
		    SkelControl = SkelControlSingleBone(SkeletonFirstPersonMesh.FindSkelControl('AmmoScale'));
	    	if(SkelControl != none)
	    	{
	    		SkelControl.SetSkelControlStrength(0.0f, 0.0f);
    		}
	    }
    }
}

simulated function CustomFire()
{
	local int i,j;
   	local vector RealStartLoc, AimDir, YDir, ZDir;
	local Projectile Proj;
	local class<Projectile> ShardProjectileClass;
	local float Mag;

	IncrementFlashCount();

	if (Role == ROLE_Authority)
	{
		// this is the location where the projectile is spawned
		RealStartLoc = GetPhysicalFireStartLoc();
		// get fire aim direction
		GetAxes(GetAdjustedAim(RealStartLoc),AimDir, YDir, ZDir);

		// special center shard
		Proj = Spawn(CenterShardClass,,, RealStartLoc);
		if (Proj != None)
		{
			Proj.Init(AimDir);
		}

		// one shard in each of 9 zones (except center)
		ShardProjectileClass = GetProjectileClass();
		for ( i=-1; i<2; i++)
		{
			for ( j=-1; j<2; j++ )
			{
				if ( (i != 0) || (j != 0) )
				{
					Mag = (abs(i)+abs(j) > 1) ? 0.7 : 1.0;
					Proj = Spawn(ShardProjectileClass,,, RealStartLoc);
					if (Proj != None)
					{
						Proj.Init(AimDir + (0.3 + 0.7*FRand())*Mag*i*SpreadDist*YDir + (0.3 + 0.7*FRand())*Mag*j*SpreadDist*ZDir );
					}
				}
			}
	    }
	}
}

//-----------------------------------------------------------------
// AI Interface

/* BestMode()
choose between regular or alt-fire
*/
function byte BestMode()
{
	local vector EnemyDir;
	local float EnemyDist;
	local UTBot B;

	B = UTBot(Instigator.Controller);
	if ( (B == None) || (B.Enemy == None) )
		return 0;

	EnemyDir = B.Enemy.Location - Instigator.Location;
	EnemyDist = VSize(EnemyDir);
	if ( EnemyDist > 750 )
	{
		if ( EnemyDir.Z < -0.5 * EnemyDist )
			return 1;
		return 0;
	}
	else if ( (B.Enemy.Weapon != None) && B.Enemy.Weapon.bMeleeWeapon )
		return 0;
	else if ( (EnemyDist < 400) || (EnemyDir.Z > 30) )
		return 0;
	else if ( FRand() < 0.65 )
		return 1;
	return 0;
}

function float GetAIRating()
{
	local UTBot B;
	local float EnemyDist;
	local vector EnemyDir;

	B = UTBot(Instigator.Controller);
	if ( B == None )
		return AIRating;

	if ( B.Enemy == None )
		return AIRating;

	EnemyDir = B.Enemy.Location - Instigator.Location;
	EnemyDist = VSize(EnemyDir);
	if ( EnemyDist > 750 )
	{
		if ( EnemyDist > 1700 )
		{
			if ( EnemyDist > 2500 )
				return 0.2;
			return (AIRating - 0.3);
		}
		if ( EnemyDir.Z < -0.5 * EnemyDist )
			return (AIRating - 0.3);
	}
	else if ( (B.Enemy.Weapon != None) && B.Enemy.Weapon.bMeleeWeapon )
		return (AIRating + 0.35);
	else if ( EnemyDist < 400 )
		return (AIRating + 0.2);
	return FMax(AIRating + 0.2 - (EnemyDist - 400) * 0.0008, 0.2);
}

function float SuggestAttackStyle()
{
	if ( (AIController(Instigator.Controller) != None)
		&& (AIController(Instigator.Controller).Skill < 3) )
		return 0.4;
    return 0.8;
}

simulated state WeaponEquipping
{
	/**
	 * We want to being this state by setting up the timing and then notifying the pawn
	 * that the weapon has changed.
	 */

	simulated function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);

		if (Instigator.IsLocallyControlled() && Instigator.IsHumanControlled())
		{
			UpdateAmmoCount();
		}
	}

}
function float SuggestDefenseStyle()
{
	return -0.4;
}

function float GetOptimalRangeFor(Actor Target)
{
	// short range so bots try to maximize shards that hit
	return 750.0;
}

defaultproperties
{
	WeaponColor=(R=255,G=255,B=128,A=255)
	FireInterval(0)=+1.1
	FireInterval(1)=+1.1
	MaxPitchLag=500
	MaxYawLag=500
	PlayerViewOffset=(X=-6.0,Y=-4.0,Z=0.5)
	SmallWeaponsOffset=(X=12.0,Y=6.0,Z=-6.0)

	Begin Object Name=FirstPersonMesh
		SkeletalMesh=SkeletalMesh'WP_FlakCannon.Mesh.SK_WP_FlakCannon_1P'
		PhysicsAsset=None
		FOV=70
		AnimTreeTemplate=AnimTree'WP_FlakCannon.Anims.AT_FlakCannon'
		AnimSets(0)=AnimSet'WP_FlakCannon.Anims.K_WP_FlakCannon_1P_Base'
	End Object
	AttachmentClass=class'UTGame.UTAttachment_FlakCannon'
	SkeletonFirstPersonMesh = FirstPersonMesh;

	ArmsAnimSet=AnimSet'WP_FlakCannon.Anims.K_WP_FlakCannon_1P_Arms'

	Begin Object Name=PickupMesh
		SkeletalMesh=SkeletalMesh'WP_FlakCannon.Mesh.SK_WP_FlakCannon_3P_Mid'
	End Object

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=WP_FlakCannon.Effects.P_WP_FlakCannon_Muzzle_Flash
	MuzzleFlashDuration=0.33
	MuzzleFlashLightClass=class'UTGame.UTRocketMuzzleFlashLight'

	WeaponFireSnd[0]=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireCue'
	WeaponFireSnd[1]=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_FireAltCue'

	WeaponFireTypes(0)=EWFT_Custom
	WeaponFireTypes(1)=EWFT_Projectile
	WeaponProjectiles(0)=class'UTProj_FlakShard'
	WeaponProjectiles(1)=class'UTProj_FlakShell'
	CenterShardClass=class'UTProj_FlakShardMain'

	FireOffset=(X=8,Y=10,Z=-10)

	MaxDesireability=0.75
	AIRating=+0.75
	CurrentRating=+0.75
	bInstantHit=false
	bSplashJump=false
	bRecommendSplashDamage=false
	bSniping=false
	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0

	InventoryGroup=7
	GroupWeight=0.5

	PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Flak_Cue'
	WeaponPutDownSnd=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_LowerCue'
	WeaponEquipSnd=SoundCue'A_Weapon_FlakCannon.Weapons.A_FlakCannon_RaiseCue'

	AmmoCount=8
	LockerAmmoCount=16
	MaxAmmoCount=24

	SpreadDist=0.1

	IconX=394
	IconY=38
	IconWidth=60
	IconHeight=38

	EquipTime=+0.75
	CrossHairCoordinates=(U=64,V=64,UL=64,VL=64)

	LockerRotation=(Pitch=0,Roll=-16384)
	IconCoordinates=(U=131,V=429,UL=132,VL=52)

	OdometerMaxPerSecOnes=21845.0;
	OdometerMaxPerSecTens=10950.0;
	curTensOdometer=0;
	curOnesOdometer=0;
	OnesPlaceSkelName=OnesDisplay
	TensPlaceSkelName=TensDisplay

	QuickPickGroup=4
	QuickPickWeight=0.9

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=90,RightAmplitude=50,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.200)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1
}
