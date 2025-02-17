/**
 * Shotgun "Gnasher"
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_Shotgun extends GearWeapon;


var array<vector2D> PelletSpread;

/** delay before playing the cock sound */
var()	config float	ShotgunCockDelay;

/** angle at which we autoaim at targets in melee range */
var config float MeleeAutoAimDot;

/** Shotgun cock sound */
var		SoundCue	ShotgunCockSound;

/** delay before playing the ejected shell impact sound **/
var     float       ShotgunCockShellImpactDelay;

/** Shotgun cock shell impact sound */
var		SoundCue	ShotgunCockShellImpactSound;

/** Shotgun cocking after firing */
var()	GearPawn.BodyStance	BS_PawnShotgunCock;
var()	name				WeaponCockAnim;

/** Distance to back up from the trace start, to avoid starting the trace inside an enemy */
var()	float			StartTraceAdjustDist;

/** Temporary value used by FireAllPellets/GetFireModeDamage to handle the special damage values depending on number of hits */
var transient int TransientShotgunDamage;

var float TimeSinceTargeting;

var transient float LastImpactSoundTime;

/** Struct used in FireAllPellets to track individual pawn hits */
struct PawnHitInfo
{
	var GearPawn HitPawn;
	var int NumHits;
	var ImpactInfo Impact;
};

simulated state ShotgunCocking
{
	simulated function BeginState(Name PreviousStateName)
	{
		// animation is played when receiving the updated firing mode.
		SetCurrentFireMode(SHOTGUN_COCK_FIREMODE);
		SetTimer( GetFireInterval(0)*0.75f, FALSE, nameof(FinishedCockingShotgun) );
	}

	simulated function EndState(Name NextStateName)
	{
		// switch back to default firing mode.
		EndFire(SHOTGUN_COCK_FIREMODE);

		NotifyWeaponFinishedFiring( CurrentFireMode );
		SetCurrentFireMode(0);
	}

	simulated function FlashLocationUpdated(byte FiringMode, vector FlashLocation, bool bViaReplication)
	{
		// if we received an update from the server, it indicates the server is ahead of us
		// and already finished this phase, so exit immediately
		if (bViaReplication)
		{
			ClearTimer(nameof(FinishedCockingShotgun));
			FinishedCockingShotgun();
		}

		Super.FlashLocationUpdated(CurrentFireMode, FlashLocation, bViaReplication);
	}

	simulated function FinishedCockingShotgun()
	{
		// switch back to default firing mode.
		EndFire(SHOTGUN_COCK_FIREMODE);
		SetCurrentFireMode(0);

		// if switching to another weapon, abort firing and put down right away
		if( bWeaponPutDown )
		{
			PutDownWeapon();

			// Set weapon as not firing
			ClearFlashCount();
			ClearFlashLocation();
			return;
		}

		// If weapon should keep on firing, then do not leave state and fire again.
		if( ShouldRefire() )
		{
			GotoState('WeaponFiring');
			return;
		}

		// Set weapon as not firing
		ClearFlashCount();
		ClearFlashLocation();

		// Otherwise we're done firing, so go back to active state.
		GotoState('Active');

		// if out of ammo, then call weapon empty notification
		if( !HasAnyAmmo() )
		{
			WeaponEmpty();
		}
	}

	simulated function bool IsFiring()
	{
		return TRUE;
	}
}

simulated function FinishedCockingShotgun();


/**
 * Event called when Pawn.FiringMode has been changed.
 * bViaReplication indicates if this was the result of a replication call.
 */
simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	super.FireModeUpdated(FiringMode, bViaReplication);

	// if we're reloading our weapon, play anim
	if( FiringMode == SHOTGUN_COCK_FIREMODE )
	{
		PlayShotgunCocking();
	}
}


simulated function PlayShotgunCocking()
{
	local GearPawn	P;
	local float		AnimTime;

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		P = GearPawn(Instigator);

		AnimTime = GetFireInterval(0)*0.75f;

		if( P != None )
		{
			// play shotgun cocking body stance
			P.BS_PlayByDuration(BS_PawnShotgunCock, AnimTime, 0.3f, 0.3f, FALSE, TRUE);
		}

		// Play animation on weapon as well
		PlayWeaponAnimByDuration(WeaponCockAnim, AnimTime);

		// Play Weapon fire animation
		//PlayWeaponAnimByDuration(WeaponFireAnim, GetFireInterval(FireModeNum));
		SetTimer( ShotgunCockDelay, FALSE, nameof(PlayShotgunCockSound) );
		SetTimer( ShotgunCockShellImpactDelay, FALSE, nameof(PlayShotgunCockShellImpactSound) );
	}
}


/** Overriden to half firing duration, because cocking is in a separate state */
simulated function TimeWeaponFiring( byte FireModeNum )
{
	if( !IsTimerActive('RefireCheckTimer') )
	{
		SetTimer( GetFireInterval(FireModeNum)*0.25f, TRUE, nameof(RefireCheckTimer) );
	}
}


simulated state WeaponFiring
{
	simulated function bool ShouldRefire()
	{
		return FALSE;
	}

	simulated function EndState(Name NextStateName)
	{
		ClearTimer('RefireCheckTimer');
		// shotgun cocking is a must happen. Then it handles transition to other states.
		GotoState('ShotgunCocking');
	}
}


simulated function CustomFire()
{
	local Vector		StartTrace, EndTrace, ShotDir;
	local Rotator		AimRot;
	local ImpactInfo	Impact;
	local float BestAim, BestDist;

	if( WorldInfo.NetMode != NM_Client )
	{
		// grab from the pawn
		StartTrace	= Instigator.GetWeaponStartTraceLocation();

		AimRot		= GetAdjustedAim(StartTrace);
		ShotDir = vector(AimRot);

		// if there is an enemy in melee range, perform a small autoaim in their general direction
		if (Instigator.IsHumanControlled())
		{
			BestAim = MeleeAutoAimDot;
			BestDist = Range_Melee;
			Impact.HitActor = Instigator.Controller.PickTarget(class'Pawn', BestAim, BestDist, ShotDir, StartTrace, Range_Melee);
			if (Impact.HitActor != None)
			{
				Impact.HitLocation = Impact.HitActor.Location;
			}
		}
		// if we didn't autoaim, trace normally
		if (Impact.HitActor == None)
		{
			EndTrace = StartTrace + (ShotDir * GetTraceRange());
			Impact = CalcWeaponFire(StartTrace, EndTrace);
		}

		// Set FlashLocation to be the point of impact.
		// A completely new set of traces will be performed on remote clients.
		// But we keep the general area of impact. Individual pellets will be slightly different on every client.
		SetFlashLocation(Impact.HitLocation);

		// Shoot all pellets
		FireAllPellets(StartTrace, AimRot, CurrentFireMode);

	}
	else if( Instigator.IsLocallyControlled() )
	{
		// local player simulates firing locally, to give instant response.
		// FlashLocation will only replicate the exact impact location, and play impact effects.
		WeaponFired(CurrentFireMode);
	}
}

/** Overridden to check for our transient damage value. */
simulated function float GetFireModeDamage( Vector HitLocation, optional Actor HitActor )
{
	local float ModifiedDamage;

	// use the transient damage value
	ModifiedDamage = TransientShotgunDamage;
	ModifyDamage(ModifiedDamage, HitLocation, HitActor);
	return ModifiedDamage;
}

simulated function TargetingModeChanged(GearPawn P)
{
	TimeSinceTargeting = WorldInfo.TimeSeconds;
}

final simulated function FireAllPellets(vector StartLocation, rotator InAimRot, byte FireModeNum)
{
	local vector			EndTrace, AimDir;
	local Array<ImpactInfo>	ImpactList;
	local int Number, Idx, ImpactIdx, BaseDamage;
	local GearPawn          GP, InstigatorGP;
	local Rotator AimRot;
	local array<PawnHitInfo> PawnHitList;
	local float SpreadModifier;
	// clamp the pitch when blindfiring over cover
	InstigatorGP = GearPawn(Instigator);
	// penalize accuracy if recently targeting or recently evading
	if (InstigatorGP.bIsTargeting && TimeSince(TimeSinceTargeting) > 0.4f && TimeSince(InstigatorGP.LastEvadeTime) > 1.5f)
	{
		SpreadModifier = 0.4f;
	}
	else if (TimeSince(InstigatorGP.LastEvadeTime) < 1.5f)
	{
		SpreadModifier = 1.f;
	}
	else
	{
		SpreadModifier = 0.8f;
	}
	// reset the transient damage for the non-pawn hit cases
	BaseDamage = (GearAI(Instigator.Controller) != None && GearAI_TDM(Instigator.Controller) == None) ? WeaponDamageAI : WeaponDamage;
	TransientShotgunDamage = BaseDamage;
	// for each pellet to fire
	for (Number = 0; Number < PelletSpread.Length; Number++)
	{
		// Calculate Spread.
		AimRot = InAimRot;
		// less vertical, more horizontal
		AimRot.Yaw += PelletSpread[Number].X * SpreadModifier;
		AimRot.Pitch += PelletSpread[Number].Y * SpreadModifier;

		AimDir		= Vector(AimRot);
		EndTrace	= StartLocation + AimDir * GetTraceRange();

		CalcWeaponFire( StartLocation, EndTrace, ImpactList );

		// spawn a tracer for the core shots
		if (Number < 4)
		{
			SpawnTracerEffect(ImpactList[0].HitLocation, VSize(ImpactList[0].HitLocation - StartLocation));
		}

		// for each thing we hit,
		for (ImpactIdx = 0; ImpactIdx < ImpactList.length; ImpactIdx++)
		{
			//`LogExt("Impact:"@Impact.HitActor);
			// if it's a pawn then track the hits separately
			GP = GearPawn(ImpactList[ImpactIdx].HitActor);
			if (GP != None)
			{
				Idx = PawnHitList.Find('HitPawn', GP);
				if (Idx == INDEX_NONE)
				{
					Idx = PawnHitList.Add(1);
					PawnHitList[Idx].HitPawn = GP;
					PawnHitList[Idx].Impact = ImpactList[ImpactIdx];
				}
				PawnHitList[Idx].NumHits++;
			}
			else
			{
				if (CrowdAgent(ImpactList[ImpactIdx].HitActor) != None)
				{
					TransientShotgunDamage = BaseDamage * 0.1f;
				}
				// otherwise apply damage normally
				ProcessInstantHit(FireModeNum, ImpactList[ImpactIdx]);
				// reset the transient damage
				TransientShotgunDamage = BaseDamage;
			}
		}
		// clear the list for the next iteration
		ImpactList.Length = 0;
	}
	// for each pawn we hit,
	for (Idx = 0; Idx < PawnHitList.Length; Idx++)
	{
		//`LogExt("Hit:"@PawnHitList[Idx].HitPawn@"NumHits:"@PawnHitList[Idx].NumHits);
		// if we hit 75% then apply full damage
		if (PawnHitList[Idx].NumHits >= PelletSpread.Length * 0.75f)
		{
			TransientShotgunDamage = BaseDamage;
		}
		else
		// if we hit 50% then apply partial damage
		if (PawnHitList[Idx].NumHits >= PelletSpread.Length * 0.5f)
		{
			TransientShotgunDamage = BaseDamage * 0.65f;
		}
		else
		{
			// otherwise scale by number of hits
			TransientShotgunDamage = BaseDamage * PawnHitList[Idx].NumHits/float(PelletSpread.Length);
		}
		// damage bonus if within very short range
		if (VSize2D(PawnHitList[Idx].HitPawn.Location - Instigator.Location) < 256.f)
		{
			TransientShotgunDamage *= 1.2f;
		}
		//`log("shotgun dmg:"@TransientShotgunDamage@PawnHitList[Idx].HitPawn@"hits:"@PawnHitList[Idx].NumHits@"dist:"@VSize2D(PawnHitList[Idx].HitPawn.Location - Instigator.Location));
		// apply the damage
		//@note: ProcessInstantHit will call GetFireModeDamage which we override to return TransientShotgunDamage instead of WeaponDamage
		ProcessInstantHit( FireModeNum, PawnHitList[Idx].Impact );
		// reset the transient damage
		TransientShotgunDamage = BaseDamage;
	}
}


/**
 * Calculate weapon impacts for remote clients.
 * Remote clients may only get a HitLocation replicated,
 * so we need to figure out the HitNormal and HitActor to play proper effects.
 */

simulated function CalcRemoteImpactEffects( byte FireModeNum, vector GivenHitLocation, bool bViaReplication )
{
	local Vector	StartTrace, EndTrace;
	local rotator	AimRot;

	//`log( GetFuncName() @ WorldInfo.TimeSeconds );
	// this isn't exactly where the shot originated on the server, but it's
	// close enough as far as effects are concerned
	if (Instigator != None)
	{
		StartTrace	= Instigator.GetWeaponStartTraceLocation();
	}
	else
	{
		StartTrace = GetPhysicalFireStartLoc();
	}

	EndTrace	= GivenHitLocation;
	AimRot		= Rotator( Normal(EndTrace-StartTrace) );

	// Shoot all pellets
	FireAllPellets(StartTrace, AimRot, FireModeNum);
}


/** disable default shell eject when firing, as we want to spawn it when player cocks weapon. See below. */
simulated function PlayShellCaseEject();

/** temp solution */
simulated function PlayShotgunCockSound()
{
	if (!bSuppressAudio)
	{
		Owner.PlaySound(ShotgunCockSound, true);
	}

	// Here we force spawning the shell eject particle system.
	// As we want it to play when we cock the gun, and not when we fire it.
	super(GearWeapon).PlayShellCaseEject();
}


simulated function PlayShotgunCockShellImpactSound()
{
	Owner.PlaySound( ShotgunCockShellImpactSound, true );
}

simulated function float GetWeaponRating()
{
	local float Rating;
	local GearAI AI;
	local vector EnemyLoc;

	Rating = super.GetWeaponRating();

	if(  Rating > 0 &&
		 !Instigator.IsHumanControlled() &&
		 Instigator.Controller != None &&
		 Instigator.Controller.Enemy != None )
	{
		AI = GearAI(Instigator.Controller);
		if (AI != None)
		{
			EnemyLoc = AI.GetEnemyLocation();
			// shotgun is really good at short range
			if (AI.IsShortRange(EnemyLoc))
			{
				Rating = 1.5f;
			}
			// and nearly worthless at very long range
			else if (VSize(EnemyLoc - AI.Pawn.Location) > AI.EnemyDistance_Long)
			{
				Rating = 0.25;
			}
		}
	}

	return Rating;
}

simulated event DummyFire(byte FireModeNumm, vector TargetLoc, optional Actor AttachedTo, optional float AimErrorDeg, optional Actor TargetActor)
{
	local Actor HitActor;
	local vector StartLoc, EndLoc, HitLocation, HitNormal;
	local rotator AimRot;
	local float AimErrorUnr;

	DummyFireParent = AttachedTo;
	bDummyFireWeapon = TRUE;

	// impact effects
	StartLoc = GetPhysicalFireStartLoc();

	AimRot = rotator(TargetLoc - StartLoc);

	// modify Aim to add player aim error
	// and convert aim error from degrees to Unreal Units ( * 65536 / 360 )
	if (AimErrorDeg != 0.f)
	{
		AimErrorUnr = AimErrorDeg * 182.044;
		AimRot.Pitch += AimErrorUnr * (0.5 - FRand());
		AimRot.Yaw	 += AimErrorUnr * (0.5 - FRand());
	}

	EndLoc = StartLoc + vector(AimRot) * 30000;		// way out there
	HitActor = Trace(HitLocation, HitNormal, EndLoc, StartLoc, true, vect(0,0,0),, TRACEFLAG_Bullet);
	if (HitActor != None)
	{
		WeaponFired(FireModeNumm, HitLocation);
		CalcRemoteImpactEffects(FireModeNumm, HitLocation, FALSE);
	}
	else
	{
		WeaponFired(FireModeNumm, EndLoc);
	}
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.ShotGun;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.ShotGun;
}
*/

simulated function SpawnImpactSounds( const class<GearDamageType> GearDmgType, ImpactInfo Impact, PhysicalMaterial PhysMaterial )
{
	// kimd of kludgy, intended to only spawn one impact sound per shotgun blast
	if (TimeSince(LastImpactSoundTime) > 0.5f)
	{
		super.SpawnImpactSounds(GearDmgType, Impact, PhysMaterial);
		LastImpactSoundTime = WorldInfo.TimeSeconds;
	}
}

defaultproperties
{
	//ShotgunCockDelay=0.2
	ShotgunCockShellImpactDelay=0.7

	bWeaponCanBeReloaded=TRUE

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Shotgun'

	WeaponAnimType=EWAT_Shotgun
	BS_PawnWeaponReload={(
		AnimName[BS_Std_Up]				="",							// Don't play on normal upper body channel
		AnimName[BS_Std_Upper_NoAim]	="SG_Reload",					// use slot that bypasses aimoffsets
		AnimName[BS_CovStdIdle_Up]		="SG_Cov_Std_Reload",
		AnimName[BS_CovMidIdle_Up]		="SG_Cov_Mid_Reload"
	)}

	BS_PawnWeaponReloadSuccess={(
		AnimName[BS_Std_Up]				="",							// Don't play on normal upper body channel
		AnimName[BS_Std_Upper_NoAim]	="SG_Reload_Success",			// use slot that bypasses aimoffsets
		AnimName[BS_CovStdIdle_Up]		="SG_Cov_Std_Reload_Success",
		AnimName[BS_CovMidIdle_Up]		="SG_Cov_Mid_Reload_Success"
	)}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Std_Up]				="",							// Don't play on normal upper body channel
		AnimName[BS_Std_Upper_NoAim]	="SG_Reload_Fail",				// use slot that bypasses aimoffsets
		AnimName[BS_CovStdIdle_Up]		="SG_Cov_Std_Reload_Fail",
		AnimName[BS_CovMidIdle_Up]		="SG_Cov_Mid_Reload_Fail"
	)}

	BS_PawnShotgunCock={(
		AnimName[BS_Std_Up]				="SG_Idle_Ready_Chamber",
		AnimName[BS_CovStdBlind_Up]		="SG_Cov_Std_Blind_Chamber",
		AnimName[BS_CovStdLean_Up]		="SG_Cov_Std_Lean_Chamber",
		AnimName[BS_CovMidBlindSd_Up]	="SG_Cov_Mid_Blind_Chamber",
		AnimName[BS_CovMidBlindUp_Up]	="SG_Cov_Mid_Blind_Up_Chamber",
		AnimName[BS_CovMidLean_Up]		="SG_Cov_Mid_Lean_Chamber",
		AnimName[BS_CovStd_360_Upper]	="SG_Cov_Std_Back_Chamber",
		AnimName[BS_CovMid_360_Upper]	="SG_Cov_Mid_Back_Chamber",
		AnimName[BS_CovStdIdle_Up]		="SG_Cov_Std_Chamber",
		AnimName[BS_CovMidIdle_Up]		="SG_Cov_Mid_Chamber"
	)}

	FireSound=SoundCue'Weapon_Shotgun.Shotgun.ShotgunFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_Shotgun.Shotgun.ShotgunFirePlayerCue'
	WeaponWhipSound=SoundCue'Weapon_Shotgun.Firing.ShotgunWhipCue'
	ShotgunCockSound=SoundCue'Weapon_Shotgun.Reloads.ShotgunCockCue'
	ShotgunCockShellImpactSound=SoundCue'Weapon_Shotgun.Reloads.ShotgunAmmoBounceCue'

	WeaponReloadSound=None

	WeaponEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_Shotgun.Actions.ShotgunLowerCue'
	PickupSound=SoundCue'Weapon_Shotgun.Actions.ShotgunPickupCue'
	WeaponDropSound=SoundCue'Weapon_Shotgun.Actions.ShotgunDropCue'

	InstantHitDamageTypes(0)=class'GDT_Shotgun'
	WeaponFireTypes(0)=EWFT_Custom
	AmmoTypeClass=class'GearAmmoType_Shotgun'

	// Weapon Animation
	Begin Object Class=AnimNodeSequence Name=WeaponAnimNode
    	AnimSeqName=""
	End Object
	WeaponFireAnim=""
	WeaponReloadAnim="SG_Reload"
	WeaponReloadAnimFail="SG_Reload_Fail"
	WeaponReloadAnimSuccess="SG_Reload_Success"
	WeaponCockAnim="SG_Chamber"

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_Gnasher.Mesh.COG_Gnasher_Skel'
		PhysicsAsset=PhysicsAsset'COG_Gnasher.Mesh.COG_Gnasher_Skel_Physics'
		AnimSets(0)=AnimSet'COG_Gnasher.Animations.COG_Gnasher_Anim'
		Animations=WeaponAnimNode
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	FireOffset=(X=66,Y=-1,Z=1.0)

	// muzzle flash
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_MuzzleFlash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_MuzzleFlash_AR'


	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=213,G=137,B=91,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.4f
	MuzzleLightPulseFreq=10
	MuzzleLightPulseExp=1.5

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'COG_Gnasher.Effects.Gnasher_Shells'
		Translation=(X=24,Y=0,Z=10)
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'COG_Gnasher.Effects.P_COG_Gnasher_Reload'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	bCanDisplayReloadTutorial=TRUE
	DamageTypeClassForUI=class'GDT_Shotgun'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=53,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=362,UL=128,VL=39)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Gnasher'

	StartTraceAdjustDist=-64

	bInstantHit=true

	Recoil_Hand={(
					LocAmplitude=(X=-10,Y=6,Z=-6),
					LocFrequency=(X=15,Y=10,Z=10),
					LocParams=(X=ERS_Zero,Y=ERS_Zero,Z=ERS_Zero),
					RotAmplitude=(X=7000,Y=200,Z=0),
					RotFrequency=(X=8,Y=5,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					TimeDuration=0.5f
					)}

	Recoil_Spine={(
					RotAmplitude=(X=2000,Y=300,Z=0),
					RotFrequency=(X=10,Y=10,Z=0),
					RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
					TimeDuration=0.50f
					)}

	NeedReloadNotifyThreshold=2

	HUDDrawData			= (DisplayCount=8,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=265,UL=117,VL=7),ULPerAmmo=15)
	HUDDrawDataSuper	= (DisplayCount=8,AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=265,UL=117,VL=7),ULPerAmmo=15)

	MeleeImpactSound=SoundCue'Weapon_Boomer.Reloads.BoomerHitCue'

	LC_EmisDefaultCOG=(R=0.0,G=0.0,B=0.0,A=1.0)
	LC_EmisDefaultLocust=(R=0.0,G=0.0,B=0.0,A=1.0)

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
	    Samples(0)=(LeftAmplitude=50,RightAmplitude=50,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.400)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

    WeaponID=WC_Gnasher

    TracerType=WTT_ShortBullet

	PelletSpread(0)=(X=-256,Y=256)
	PelletSpread(1)=(X=-256,Y=-256)
	PelletSpread(2)=(X=256,Y=-256)
	PelletSpread(3)=(X=256,Y=256)
	PelletSpread(4)=(X=-1024,Y=512)
	PelletSpread(5)=(X=-1024,Y=-512)
	PelletSpread(6)=(X=1024,Y=512)
	PelletSpread(7)=(X=1024,Y=-512)

	CQC_Quick_KillerAnim="CTRL_Quick_Shotgun"
	CQC_Quick_VictimDeathTime=0.64f
	CQC_Quick_EffortID=GearEffort_ShotgunQuickExecutionEffort
	CQC_Quick_DamageType=class'GDT_QuickExecution_Shotgun'
}
