/**
 * Locust Burst pistol
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_LocustBurstPistolBase extends GearWeap_PistolBase
	abstract
	config(Weapon);

/** Delay in between bursts */
var() protected const config float	BurstDelay;

/** Number of shots fired in a row for one trigger press */
var() protected const config byte	NumShotsPerBurst;

/** Counter for burst shots */
var protected transient	byte		BurstCount;

var protected transient	bool		bWasTargeting;

/** Half of the "gap" between the barrels, used to simulate 2 barrels firing in a "stitching" pattern. */
var() protected const float			HalfBarrelSeparation;

var protected transient AudioComponent	FireSoundAC;

var StaticMeshComponent	MagazineMesh2;

simulated function bool CanReload()
{
	local GearPawn_LocustKantusBase KantusPawn;

	KantusPawn = GearPawn_LocustKantusBase(Instigator);
	// Small hack, prevent Kantus from reloading his weapon if he's throwing a grenade.
	if( KantusPawn != None && KantusPawn.IsThrowingOffHandGrenade() )
	{
		return FALSE;
	}

	return Super.CanReload();
}

simulated protected function PlayFireSound()
{
	// all 8 shots are baked into a single sound, so we only
	// want to play the fire sound on the first shot.
	if ( (FireSoundAC == None) || !FireSoundAC.IsPlaying() )
	{
		FireSoundAC = GearWeaponPlaySoundLocalEx( FireSound, FireSound_Player, FireSoundAC );
	}

	if ( (WorldInfo.Netmode != NM_Client) && (Instigator != None) )
	{
		Instigator.MakeNoise( 1.f );
	}
}



simulated function PlayWeaponReloading()
{
	Super.PlayWeaponReloading();

	// so here we need to set a timer as a hack to make certain our clips are scaled to zero as there
	// is some crazy animation bug where they do not correctly get their notifies
	// so even tho the clips will not show up on the weapon at least they will not be floating in the air all crazy like
	// the failed reload is 4 seconds long.  So we wait 400 ms to make certain a tick has occurred and then if this timer
	// has not been cleared by the actual anim notify firing we will scale the magazines to zero
	SetTimer( 4.4f, FALSE, nameof(ForceScaleMagazinesToZero) );
}

simulated function ForceScaleMagazinesToZero()
{
	SetWeaponAmmoBoneDisplay( FALSE, 'AmmoControlLeft' );
	SetWeaponAmmoBoneDisplay( FALSE, 'AmmoControlRight' );

//	`log( "ForceScaleMagazinesToZeroForceScaleMagazinesToZeroForceScaleMagazinesToZeroForceScaleMagazinesToZero" );
}



simulated state WeaponFiring
{
	simulated function BeginState(Name PreviousStateName)
	{
		bWasTargeting = GearPawn(Instigator) != None && GearPawn(Instigator).bIsTargeting;
		BurstCount = 0;

		// play force feedback here so it is once per burst
		if( GearPC(Instigator.Controller) != none )
		{
			GearPC(Instigator.Controller).ClientPlayForceFeedbackWaveform( WeaponFireWaveForm );
		}

		super.BeginState(PreviousStateName);
	}

	simulated function EndState(Name NextStateName)
	{
		FireSoundAC = None;
		super.EndState(NextStateName);
	}

	simulated function bool ShouldForceTargeting()
	{
		// keep the player targeting while bursting
		return bWasTargeting;
	}

	/**
	 * Timer event, call is set up in Weapon::TimeWeaponFiring().
	 * The weapon is given a chance to evaluate if another shot should be fired.
	 * This event defines the weapon's rate of fire.
	 */
	simulated function RefireCheckTimer()
	{
		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();
			return;
		}

		// If weapon should keep on firing, then do not leave state and fire again.
		if( ShouldRefire() )
		{
			FireAmmunition();
			return;
		}

		bWasTargeting = FALSE;

		// Force player to wait before re-firing
		GotoState('BurstDelaying');
	}
}


/**
 * handle burst refiring.
 */
simulated function bool ShouldRefire()
{
	// if doesn't have ammo to keep on firing, then stop
	if( !HasAmmo(CurrentFireMode) )
	{
		return FALSE;
	}

	if( BurstCount < NumShotsPerBurst )
	{
		return TRUE;
	}

	return FALSE;
}

simulated function EndOfBurstDelay();

simulated state BurstDelaying
{
	simulated function BeginState(Name PreviousStateName)
	{
		if( !HasAmmo(CurrentFireMode) )
		{
			// end prematurely so we can reload
			EndOfBurstDelay();
		}
		else
		{
			SetTimer( BurstDelay, FALSE, nameof(EndOfBurstDelay) );
		}

		if( Instigator.IsHumanControlled() )
		{
			// must release trigger and refire
			EndFire( CurrentFireMode );
			Instigator.Controller.bFire = 0;
		}
	}

	simulated function EndState(Name NextStateName)
	{
		ClearTimer('EndOfBurstDelay');

		// stop the force feedback here so we get a semi clear separation between burst force feedbacks
		if( GearPC(Instigator.Controller) != none )
		{
			GearPC(Instigator.Controller).ClientStopForceFeedbackWaveform( WeaponFireWaveForm );
		}
	}

	simulated function EndOfBurstDelay()
	{
		GotoState('Active');
	}

	simulated function StartFire(byte FireModeNum)
	{
		if (FireModeNum == 0)
		{
			// no firing in this state.  make player repress trigger.
			EndFire( CurrentFireMode );
			Instigator.Controller.bFire = 0;
		}
		else
		{
			// end prematurely so we can reload
			EndOfBurstDelay();
			Super.StartFire(FireModeNum);
		}
	}
}

simulated function FireAmmunition()
{
	BurstCount++;
	super.FireAmmunition();
}

simulated function vector AdjustStartTraceLocation(vector StartLoc, vector AimDir)
{
	local vector AimX, AimY, AimZ;

	GetAxes(rotator(AimDir), AimX, AimY, AimZ);

	// left/right is randomized
	if (FRand() < 0.5f)
	{
		StartLoc -= AimY * HalfBarrelSeparation;
	}
	else
	{
		StartLoc += AimY * HalfBarrelSeparation;
	}

	return StartLoc;
}

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	local SkeletalMeshComponent	SkelMesh;
	local Vector				MagLoc;
	local Rotator				MagRot;

	if( ShouldPlaySimplePistolReload() )
	{
		return;
	}

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay(FALSE, 'AmmoControlLeft');

	// Spawn physics magazine instead
	SkelMesh = SkeletalMeshComponent(Mesh);
	SkelMesh.GetSocketWorldLocationAndRotation('MagazineLeft', MagLoc, MagRot);

	// Spawn physics Magazine
	SpawnPhysicsMagazine(MagLoc, MagRot);

	ClearTimer( nameof(ForceScaleMagazinesToZero) );
}

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease2()
{
	local SkeletalMeshComponent	SkelMesh;
	local Vector				MagLoc;
	local Rotator				MagRot;

	if( ShouldPlaySimplePistolReload() )
	{
		return;
	}

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay(FALSE, 'AmmoControlRight');

	// Spawn physics magazine instead
	SkelMesh = SkeletalMeshComponent(Mesh);
	SkelMesh.GetSocketWorldLocationAndRotation('MagazineRight', MagLoc, MagRot);

	// Spawn physics Magazine
	SpawnPhysicsMagazine(MagLoc, MagRot);

	ClearTimer( nameof(ForceScaleMagazinesToZero) );
}

/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	if( ShouldPlaySimplePistolReload() )
	{
		return;
	}

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE,,, Rot(0,0,32768));

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE, MagazineMesh2, 'b_MF_Weapon_L');
}

simulated function UpdateMagazineDueToMirror(GearPawn P)
{
	// If player happened to be in the middle of a weapon reload animation
	// handle case where clip was attached to left hand
	if( MagazineMesh != None && P.Mesh != None && P.Mesh.IsComponentAttached(MagazineMesh) )
	{
		SetPawnAmmoAttachment(FALSE);
		SetPawnAmmoAttachment(TRUE,,, Rot(0,0,32768));
	}
	if( MagazineMesh2 != None && P.Mesh != None && P.Mesh.IsComponentAttached(MagazineMesh2) )
	{
		SetPawnAmmoAttachment(FALSE, MagazineMesh2);
		SetPawnAmmoAttachment(TRUE, MagazineMesh2, 'b_MF_Weapon_L');
	}
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	if( ShouldPlaySimplePistolReload() )
	{
		return;
	}

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Detach magazine mesh from instigator's left
	SetPawnAmmoAttachment(FALSE);

	// Show Ammo part on weapon mesh.
	SetWeaponAmmoBoneDisplay(TRUE, 'AmmoControlLeft');
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload2()
{
	if( ShouldPlaySimplePistolReload() )
	{
		return;
	}

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Detach magazine mesh from instigator's left
	SetPawnAmmoAttachment(FALSE, MagazineMesh2);

	// Show Ammo part on weapon mesh.
	SetWeaponAmmoBoneDisplay(TRUE, 'AmmoControlRight');
}


defaultproperties
{
	AIRating=1.f

//	FireAnimRateScale=2.5

	HalfBarrelSeparation=15.f

	bWeaponCanBeReloaded=TRUE

	InstantHitDamageTypes(0)=class'GDT_LocustBurstPistol'
	AmmoTypeClass=class'GearAmmoType_LocustBurstPistol'

	NoAmmoFireSoundDelay=0.25f

	// Weapon Mesh
	Begin Object Name=WeaponMesh
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	WeaponFireAnim=""
	WeaponReloadAnim="BP_Reload"
	WeaponReloadAnimFail="BP_Reload_Fail"

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_BurstPistol.Effects.P_BurstPistol_Muzzle_Flash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'Locust_Pistol.EffectS.P_Geist_Pistol_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	// Muzzle Flash point light
	Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=235,G=145,B=48,A=255)
	End Object
	MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.02f
	MuzzleLightPulseFreq=15
	MuzzleLightPulseExp=1.5

	Recoil_Hand={(
		LocAmplitude=(X=-15,Y=0,Z=0),
		LocFrequency=(X=8,Y=20,Z=15),
		LocParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Random),
		RotAmplitude=(X=5000,Y=200,Z=0),
		RotFrequency=(X=8,Y=20,Z=8),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Zero),
		TimeDuration=0.17f,
	)}

	Recoil_Spine={(
		RotAmplitude=(X=800,Y=0,Z=0),
		RotFrequency=(X=13,Y=10,Z=0),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Y=ERS_Zero),
		TimeDuration=0.27f,
	)}

	bInstantHit=TRUE
	bIsSuppressive=FALSE

	NeedReloadNotifyThreshold=2

	LC_EmisDefaultCOG=(R=0.0,G=2.0,B=25.0,A=1.0)
	LC_EmisDefaultLocust=(R=15.0,G=2.0,B=0.0,A=1.0)

	WeaponID=WC_LocustBurstPistol

	BS_PawnWeaponReload={(
		AnimName[BS_Hostage_Upper]		="Hostage_BurstPistol_reload",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_BurstPistol_reload"
	)}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Hostage_Upper]		="Hostage_BurstPistol_reload_fail",
		AnimName[BS_Kidnapper_Upper]	="Kidnapper_BurstPistol_reload_fail"
	)}
}
