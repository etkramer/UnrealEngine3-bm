/**
 * COG AR-15 Assault Rifle
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_AssaultRifle extends GearWeapon;

var transient bool bStartedRipping;

var ForceFeedbackWaveForm RippingWaveForm;

/** Melee Idle Animation */
var GearPawn.BodyStance BS_MeleeIdle;

/** AudioComponents to play sounds on */
var AudioComponent AC_IdleLoop;

var AudioComponent AC_RipLoop;
var SoundCue ChainSawRipLoopSound;

var	AudioComponent AC_SlashLoop;
var SoundCue ChainSawSlashLoopSound;

var SoundCue SpecialMeleeAttackStartSound;
var SoundCue SpecialMeleeAttackIdleLoopSound;
var SoundCue SpecialMeleeAttackStopSound;
var SoundCue SpecialMeleeAttackSputterSound;


/** Temporarily disable blood FX for this attack? */
var transient bool bDisableChainsawBloodFX;

/** chainsaw-specific friction params */
var() config vector2d	SpecialMeleeAttackFrictionMultiplierRange;
var() config float		SpecialMeleeAttackPeakFrictionRadiusScale;
var() config float		SpecialMeleeAttackPeakFrictionHeightScale;


/** Whether or not this weapon should spawn a decal.  Perf improvement when the AI doesn't always spawn a decal.  Especially as most of them are behind you or on the cover in front of you **/
var bool bShouldSpawnDecal;

/** Sounds to play when firing loop stops. */
var protected const SoundCue	StopFireSound;
var protected const SoundCue	StopFireSound_Player;
/** Audiocomponent for looping firing sound. */
var protected AudioComponent	FireLoopAC;


/** TRUE when playing looping muzzle flash effect */
var()	bool		bPlayingMuzzleFlashEffect;


/** The ChainSaw MaterialInstaneConstant**/
var MaterialInstanceConstant ChainSawMIC;


/** Effect to play when the chainsaw attack impacts the mob **/
var ParticleSystemComponent ChainSawImpactEffect;
var ParticleSystem ChainSawImpactEffect_FromForward;
var ParticleSystem ChainSawImpactEffect_FromBehind;
/** Whether or not this Chainsaw is sawing from behind **/
var bool bAttackFromBehind;

/** The PS to play when extreme content is off **/
var ParticleSystem ChainSawImpactEffect_NoGore;

var ParticleSystemComponent ChainSawSmokeEffect;

/** ChainSaw sounds */
var bool bPlayingSlashSound;
var SoundCue ChainsawSlashStartSound;
var SoundCue ChainsawSlashStopSound;


/** Chainsaw sputter sound.  We need our own audio component so that the sound is not played in the world and we hear it spatializing as we run **/
var	AudioComponent AC_SputterSound;

/** Chainsaw revving! **/
var bool bRevTapping;
var bool bRevHolding;
var bool bRevReady;

/** Muzzle smoke effect drop rate, in effects per seconds. */
var() private float			MuzzleSmokeRate;

var() protected const config float CameraFOVWhileChainsawing;

simulated event PostBeginPlay()
{
	SkeletalMeshComponent(Mesh).AttachComponentToSocket(ChainSawImpactEffect, MuzzleSocketName);
	SkeletalMeshComponent(Mesh).AttachComponentToSocket(ChainSawSmokeEffect, MuzzleSocketName);

	ChainSawMIC = new(outer) class'MaterialInstanceConstant';
	ChainSawMIC.SetParent( Material'COG_AssaultRifle.BlurringBlades' );
	Mesh.SetMaterial( 1, ChainSawMIC );

	Super.PostBeginPlay();
}

simulated function Destroyed()
{
	if ( FireLoopAC != None )
	{
		FireLoopAC.FadeOut(0.2f,0.f);
		FireLoopAC = None;
	}

	// Shut down slash looping sound
	if( AC_SlashLoop != None )
	{
		AC_SlashLoop.FadeOut(0.2f, 0.f);
		AC_SlashLoop = None;
	}

	// Shut down rip looping sound
	if( AC_RipLoop != None )
	{
		AC_RipLoop.FadeOut(0.2f, 0.f);
		AC_RipLoop = None;
	}

	if( AC_SputterSound != None )
	{
		AC_SputterSound.FadeOut(0.2f, 0.f);
		AC_SputterSound = None;
	}

	Super.Destroyed();
}

simulated function PlayShellCaseEject()
{
	local GearPawn	GP;
	local Rotator	NewRot;

	// shell eject particle system
	if( PSC_ShellEject != None )
	{
		GP = GearPawn(Instigator);

		// If pawn is mirrored, then mirror shell ejection Y axis as well
		if( GP != None )
		{
			// we need to manually flip the meshes on the emitters so they are not backwards
			if( GP.bIsMirrored )
			{
				PSC_ShellEject.SetVectorParameter( 'MeshOrientation', vect(1,1,1) );
			}
			else
			{
				PSC_ShellEject.SetVectorParameter( 'MeshOrientation', vect(0,0,0) );
			}

			// LAURENT: forced offsets, because Component.default.Property doesn't work ATM.
			if( GP.bIsMirrored )
			{
				NewRot.Yaw = 16384;
			}
			else
			{
				NewRot.Yaw = -16384;
			}

			if( PSC_ShellEject.Rotation != NewRot )
			{
				PSC_ShellEject.SetRotation(NewRot);
			}
		}

		if (IsTimerActive( 'HidePSC_ShellEject' ))
		{
			ClearTimer( 'HidePSC_ShellEject' );
			if (PSC_ShellEject != none)
			{
				PSC_ShellEject.ActivateSystem();
				PSC_ShellEject.RewindEmitterInstances();
			}
		}

		if (PSC_ShellEject.HiddenGame == TRUE)
		{
			PSC_ShellEject.SetHidden( FALSE );
			PSC_ShellEject.ActivateSystem();
		}
	}
}

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if (FiringMode == MELEE_ATTACK_FIREMODE)
	{
		StartBlade();
	}
	else if (PreviousFiringMode == MELEE_ATTACK_FIREMODE)
	{
		StopBlade();
	}

	super.FireModeUpdated(FiringMode, bViaReplication);
}

simulated function float PlayMeleeAttackAnimation(GearPawn P)
{
}


simulated state MeleeAttacking
{
	simulated function BeginState(Name PreviousStateName)
	{
		local GearPawn GP;

		GP = GearPawn(Instigator);
		if (GP != None && !GP.CanDoSpecialMove(SM_ChainsawHold))
		{
			`log("Aborting melee attack due to !CanDoSpecialMove(SM_ChainsawHold)"@`showvar(GP.SpecialMove)@`showvar(GP.Weapon));
			GP.ForceStopMeleeAttack(TRUE);
			return;
		}

		//`log(self@GetStateName()@GetFuncName());
		Super.BeginState(PreviousStateName);

		if ( (Role == ROLE_Authority) && !bSlaveWeapon )
		{
			// Alert AI about chainsaw sound and set timer for continuation of sound
			Instigator.MakeNoise(1.f,'MeleeAttack');
			SetTimer( 0.5f, TRUE, nameof(SpecialMeleeAttackMakeNoiseTimer) );
		}

		// if server or locally owned client
		if (Instigator.Controller != None)
		{
			// Set up chainsaw-specific friction params
			FrictionMultiplierRange = SpecialMeleeAttackFrictionMultiplierRange;
			PeakFrictionRadiusScale = SpecialMeleeAttackPeakFrictionRadiusScale;
			PeakFrictionHeightScale = SpecialMeleeAttackPeakFrictionHeightScale;
			// start the special moves
			GearPawn(Instigator).DoSpecialMove(SM_ChainsawHold);
		}

		SetTimer( 0.5f,FALSE,nameof(EnableRev) );
	}

	simulated function EndState( Name NextStateName )
	{
		local GearPawn GP;
		//`log(self@GetStateName()@GetFuncName());
		
		// if server or locally owned client
		GP = GearPawn(Instigator);
		if (Instigator != None && Instigator.Controller != None)
		{
			// Set up chainsaw-specific friction params
			FrictionMultiplierRange = default.FrictionMultiplierRange;
			PeakFrictionRadiusScale = default.PeakFrictionRadiusScale;
			PeakFrictionHeightScale = default.PeakFrictionHeightScale;
			// cancel out the hold special move
			if (GP != None)
			{
				GP.EndSpecialMove(SM_ChainsawHold);
			}
		}

		// stop the fx
		if (GP != None && !GP.IsDoingSpecialMove(SM_ChainsawAttack_Object) && !GP.IsDoingSpecialMove(SM_ChainsawAttack_Object_NoCamera))
		{
			//`log(`showvar(GP.SpecialMove));
			ChainSawImpactEffect.DeActivateSystem();
			StopBloodTrails( GearPawn(Instigator) );
		}

		ClearTimer('SpecialMeleeAttackMakeNoiseTimer');

		Super.EndState( NextStateName );
	}

	simulated function bool HandleButtonPress( coerce Name ButtonName )
	{
		if( ButtonName == 'R2' )
		{
			// just rev
			bRevTapping = TRUE;
			return TRUE;
		}
		else
		{
			return Global.HandleButtonPress(ButtonName);
		}
	}

	simulated function Tick( float DeltaTime )
	{
		local GearPC PC;
		local GearPawn GP;

		Super.Tick( DeltaTime );

		GP = GearPawn(Instigator);
		PC = GearPC(Instigator.Controller);
		if( PC != None && PC.IsLocalPlayerController() )
		{
			if (bRevTapping && bRevReady && GP != None && GP.IsDoingSpecialMove(SM_ChainsawHold))
			{
				bRevTapping = FALSE;
				bRevReady = FALSE;
				SetTimer( 0.85f,FALSE,nameof(EnableRev) );
				WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawRevMediumCue');
				if (bSlaveWeapon)
				{
					ServerDoChainsawRev();
				}
			}
		}
    }

	// When melee button is released, we go back to the active state.
	// There is code in GearPawn::Tick() to prevent the button from being released if we're in the middle of an attack
	simulated function EndFire(byte FireModeNum)
	{
		Super.EndFire(FireModeNum);

		if (FireModeNum == CurrentFireMode)
		{
			GotoState('Active');
		}
	}
}

unreliable server function ServerDoChainsawRev()
{
	WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawRevMediumCue');
}

simulated function EnableRev()
{
	bRevReady = TRUE;
}

/** @see AGearWeapon::PlayFireEffects */
simulated function PlayFireEffects( byte FireModeNum, optional vector HitLocation )
{
	super.PlayFireEffects( FireModeNum, HitLocation );

	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// Play looping weapon fire animation
		if( !IsPlayingAnim(WeaponFireAnim, TRUE) )
		{
			PlayWeaponAnim(WeaponFireAnim, 1.f, 0.f, 0.f, TRUE);
		}
	}
}

simulated function PlayMuzzleFlashEffect()
{
	if( !bPlayingMuzzleFlashEffect && !bSuppressMuzzleFlash && IsMuzzleFlashRelevant() )
	{
		bPlayingMuzzleFlashEffect = TRUE;

		// activate muzzle flash particle system
		if( MuzFlashEmitter != None )
		{
			ClearTimer( 'HideMuzzleFlashEmitter' );
			MuzFlashEmitter.SetHidden( FALSE );
			MuzFlashEmitter.ActivateSystem();
		}

		if( MuzzleFlashLight != None && !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator) )
		{
			MuzzleFlashLight.SetEnabled(TRUE);
		}

		// maybe spawn lingering muzzle smoke
		if( (MuzSmokeParticleSystem != None)
			&& !GearGRI(WorldInfo.GRI).ShouldDisableEffectsDueToFramerate(Instigator) 
			)
		{
			SetTimer( 1.f/MuzzleSmokeRate, TRUE, nameof(SpawnMuzzleSmoke) );
		}
	}
}

// Spawns a lingering muzzle smoke emitter
simulated function SpawnMuzzleSmoke()
{
	local vector		SpawnLoc;
	local Emitter	SpawnedSmokeEmitter;

	SpawnLoc = GetMuzzleLoc();

	SpawnedSmokeEmitter = GetImpactEmitter( MuzSmokeParticleSystem, SpawnLoc, rot(0,0,0) );
	SpawnedSmokeEmitter.ParticleSystemComponent.ActivateSystem();
}

simulated function WeaponStoppedFiring( byte FiringMode )
{
	super.WeaponStoppedFiring( FiringMode );

	// Stop playing weapon animation.
	FreezeWeaponAnim();

	if( bPlayingMuzzleFlashEffect )
	{
		bPlayingMuzzleFlashEffect = FALSE;

		// deactivate muzzle flash particle system
		if( MuzFlashEmitter != None )
		{
			MuzFlashEmitter.DeActivateSystem();
			SetTimer( 0.1f, FALSE, nameof(HideMuzzleFlashEmitter) );
		}

		// turn off muzzle flash light
		if( MuzzleFlashLight != None )
		{
			MuzzleFlashLight.SetEnabled(FALSE);
		}

		ClearTimer('SpawnMuzzleSmoke');
	}
}


simulated function MeleeImpactNotify(GearAnimNotify_MeleeImpact Notify)
{
	local GearPawn GP;

	GP = GearPawn(Instigator);
	if (Notify.bImpactStart)
	{
		if (!bStartedRipping)
		{
			//`log(WorldInfo.TimeSeconds@`location@`showvar(ChainsawImpactEffect.Template)@`showvar(Notify.bImpactStart));
			if (GP != None && (GP.IsDoingSpecialMove(SM_ChainsawAttack) || GP.IsDoingSpecialMove(SM_ChainsawAttack_Object) || GP.IsDoingSpecialMove(SM_ChainsawAttack_Object_NoCamera)))
			{
				// start the ripping fx/sounds
				StartRipping();
				bStartedRipping = TRUE;
			}
		}
	}
	else
	{
		if (bStartedRipping)
		{
			//`log(WorldInfo.TimeSeconds@`location@`showvar(ChainsawImpactEffect.Template)@`showvar(Notify.bImpactStart));
			// locally stop the fx
			StopRipping();

			// forward to the special move so it can kill the victim, etc
			if (GP != None && GP.IsDoingSpecialMove(SM_ChainsawAttack))
			{
				GSM_ChainsawAttack(GP.SpecialMoves[SM_ChainsawAttack]).ChainsawAnimNotify();
			}
			else if (GP != None && (GP.IsDoingSpecialMove(SM_ChainsawAttack_Object) || GP.IsDoingSpecialMove(SM_ChainsawAttack_Object_NoCamera)))
			{
				GSM_ChainsawAttack_Object(GP.SpecialMoves[SM_ChainsawAttack_Object]).ChainsawAnimNotify();
			}
			bStartedRipping = FALSE;
		}
		else
		{
			SetTimer(0.5f,FALSE,nameof(StopSlashing));
		}
	}
}

function StartAttackObject(bool bSkipBlood, ParticleSystem OverrideFX)
{
	StartSlashing(bSkipBlood,OverrideFX);
	if (Role == ROLE_Authority && !Instigator.IsLocallyControlled())
	{
		ClientStartAttackObject(bSkipBlood,OverrideFX);
	}
}

reliable client function ClientStartAttackObject(bool bSkipBlood, ParticleSystem OverrideFX)
{
	StartSlashing(bSkipBlood,OverrideFX);
}

/** Starts the slashing sound (from an attack), and sets a timer to trigger the looping cue. */
simulated function StartSlashing(optional bool bSkipBlood, optional ParticleSystem OverrideFX)
{
	bDisableChainsawBloodFX = bSkipBlood;
	// always set this even if none - normal chainsaws will choose from the _FromForward/_FromBehind
	ChainsawImpactEffect.SetTemplate(OverrideFX);
	// Play looping sound
	WeaponPlaySound(ChainsawSlashStartSound);
	SetTimer( ChainsawSlashStartSound.GetCueDuration(), FALSE, nameof(StartSlashLoop) );
}

/** Starts the looping slash audio. */
simulated function StartSlashLoop()
{
	if( AC_SlashLoop == None )
	{
		AC_SlashLoop = GearWeaponPlaySoundLocalEx(ChainSawSlashLoopSound,, AC_SlashLoop, 0.1f);
	}
}

/** Stops the loop and plays the outro sound cue */
simulated function StopSlashing()
{
	if( AC_SlashLoop != None )
	{
		AC_SlashLoop.FadeOut(0.2f,0.f);
		AC_SlashLoop = None;
	}

	ClearTimer(nameof(StartSlashLoop));
	ClearTimer(nameof(StopSlashing));

	// kick off the slash stop sound
	WeaponPlaySound(ChainsawSlashStopSound);

	// reset the blood flag since it's a one-time usePawnOwner.TargetedGDO == None
	bDisableChainsawBloodFX = default.bDisableChainsawBloodFX;
}

/** Starts the ripping sound/fx (once an attack connects, called from anim notify) */
simulated function StartRipping()
{
	local GearPawn GP;
	local SoundCue RipLoop;
	local GearPC PCOwner;

	GP = GearPawn(Instigator);
	PCOwner = GearPC(GP.Controller);
	if( AC_RipLoop == None )
	{
		if ( (GP != None) && (GP.TargetedGDO != None) )
		{
			RipLoop = GP.TargetedGDO.ChainSawRipSound;
		}
		if ( GP != None && (GP.IsDoingSpecialMove(SM_ChainsawAttack_Object) || GP.IsDoingSpecialMove(SM_ChainsawAttack_Object_NoCamera)))
		{
			RipLoop = GSM_ChainsawAttack_Object(GP.SpecialMoves[GP.SpecialMove]).ChainsawTrigger.ChainSawRipSound;
		}
		if (RipLoop == None)
		{
			RipLoop = ChainSawRipLoopSound;
		}
		AC_RipLoop = GearWeaponPlaySoundLocalEx(RipLoop,, AC_RipLoop, 0.2f);
		if (PCOwner != None && PCOwner.IsLocalPlayerController())
		{
			PCOwner.ClientPlayForceFeedbackWaveform(RippingWaveForm);
		}
	}

	//@note - if this flag is set then we've most likely set an override template already (@see StartSlashing), so don't break it!
	if (!bDisableChainsawBloodFX)
	{
		if( WorldInfo.GRI.ShouldShowGore() )
		{
			if( bAttackFromBehind == FALSE )
			{
				ChainSawImpactEffect.SetTemplate( ChainSawImpactEffect_FromForward );
			}
			else
			{
				ChainSawImpactEffect.SetTemplate( ChainSawImpactEffect_FromBehind );
			}
			if (GP != None)
			{
				GP.StartBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Ground', 0.05f );
				GP.StartBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Wall' );
			}
		}
		else 
		{
			ChainSawImpactEffect.SetTemplate( ChainSawImpactEffect_NoGore );
		}
	}
	// if we have a template then show the FX
	if (ChainsawImpactEffect.Template != None)
	{
		ChainSawImpactEffect.SetHidden( FALSE );
		ChainSawImpactEffect.ActivateSystem();
	}
}

/** Stops the ripping loop sound, and hides blood */
simulated function StopRipping()
{
	local GearPC PCOwner;
	if( AC_RipLoop != None )
	{
		AC_RipLoop.FadeOut(0.2f,0.f);
		AC_RipLoop = None;
	}

	PCOwner = GearPC(Instigator.Controller);
	if (PCOwner != None && PCOwner.IsLocalPlayerController())
	{
		PCOwner.ClientStopForceFeedbackWaveform(RippingWaveForm);
	}

	ChainSawImpactEffect.DeActivateSystem();
	// Stop the slash altogether (only if it's started, ignore if we got interrupted)
	if( AC_SlashLoop != None || IsTimerActive('StartSlashLoop') )
	{
		SetTimer( 0.5f, FALSE, nameof(StopSlashing) );
	}

	StopBloodTrails( GearPawn(Instigator) );
}

simulated function StopBloodTrails( GearPawn TheOwner )
{
	if( TheOwner != none )
	{
		TheOwner.StopBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Ground' );
		TheOwner.StopBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Wall' );
	}
}

/** Starts the blade spinning */
simulated function StartBlade()
{
	ClearTimer('HalfStopBlade');
	ClearTimer('FullStopBlade');
	ChainSawMIC.SetScalarParameterValue( 'BladeSpeed', 1.0f);

	// Play Idle looping sound
	if( AC_IdleLoop == None )
	{
		AC_IdleLoop = GearWeaponPlaySoundLocalEx(SpecialMeleeAttackIdleLoopSound,, AC_IdleLoop, 0.3f);
	}

	// start the smoke
	ClearTimer( 'HideSmokeEffect' );
	ChainSawSmokeEffect.SetHidden( FALSE );
	ChainSawSmokeEffect.ActivateSystem();
}

/** Stops the blade spinning */
simulated function StopBlade()
{
	ChainSawMIC.SetScalarParameterValue( 'BladeSpeed', 0.7f);
	SetTimer( 1.0f, false, nameof(HalfStopBlade) );
	SetTimer( 2.0f, FALSE, nameof(FullStopBlade) );
	// Shut down idle looping sound
	if( AC_IdleLoop != None )
	{
		AC_IdleLoop.FadeOut(0.2f, 0.f);
		AC_IdleLoop = None;
	}
	// clear the smoke
	ChainSawSmokeEffect.DeActivateSystem();
	SetTimer( 2.0f, FALSE, nameof(HideSmokeEffect) );
}

simulated function HalfStopBlade()
{
	ChainSawMIC.SetScalarParameterValue( 'BladeSpeed', 0.25f);
}
simulated function FullStopBlade()
{
	ChainSawMIC.SetScalarParameterValue( 'BladeSpeed', 0.0f);
}

simulated function PlayMeleeScreenShake(GearPawn Victim)
{
	// already have a separate constant shake for the chainsaw
}

simulated function HideSmokeEffect()
{
	ChainSawSmokeEffect.SetHidden( TRUE );
}

reliable client function ClientForceEndMelee()
{
	EndFire(MELEE_ATTACK_FIREMODE);
}

simulated function StartFire(byte FireModeNum)
{
	local Actor Target;
	local GearPawn GP;

	// don't bother firing if the enemy is really far away
	if (GearAIController != None && FireModeNum == 0)
	{
		Target = (GearAIController.FireTarget != None) ? GearAIController.FireTarget : GearAIController.Enemy;
		if (Target != None && VSize(Target.Location - GearAIController.Pawn.Location) > Range_Long * 1.5)
		{
			return;
		}
	}

	// disallow melee if chainsaw is interrupted
	if (FireModeNum == MELEE_ATTACK_FIREMODE)
	{
		GP = GearPawn(Instigator);
		if (GP != None && GP.IsChainsawInterrupted())
		{
			if (!GP.IsLocallyControlled())
			{
				// make sure client knows we rejected the chainsaw attempt
				ClientForceEndMelee();
			}
			return;
		}
	}

	Super.StartFire(FireModeNum);
}

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	local SkeletalMeshComponent	SkelMesh;
	local Vector				MagLoc;
	local Rotator				MagRot;

	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay( FALSE );

	// Spawn physics magazine instead
	SkelMesh = SkeletalMeshComponent(Mesh);
	SkelMesh.GetSocketWorldLocationAndRotation('Magazine', MagLoc, MagRot);

	// Spawn physics Magazine
	SpawnPhysicsMagazine(MagLoc, MagRot);
}


/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE);
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Detach magazine mesh from instigator's left
	SetPawnAmmoAttachment(FALSE);

	// Show Ammo part on weapon mesh.
	SetWeaponAmmoBoneDisplay(TRUE);
}

/**
 * Event called when melee attack begins.
 * This means when weapon is entering melee attacking state on local player and server.
 * When Pawn.FiringMode == MELEE_ATTACK_FIREMODE is replicated on remote clients.
 */
simulated function MeleeAttackStarted()
{
	// Forward event to pawn owner
	GearPawn(Instigator).MeleeAttackStarted(self);
}

/**
 * Play Melee Attack
 * This function plays the melee attack animation
 * and sets a timer to call EndOfMeleeAttack(), which will trigger this end of the move
 * (and state transition).
 */
simulated function PlayMeleeAttack()
{
	local GearPawn P;

	bSuppressMeleeAttackCameraBoneAnim = FALSE;

	Super.PlayMeleeAttack();

	P = GearPawn(Instigator);
	P.SoundGroup.PlayEffort(P, GearEffort_ChainSawAttackEffort);

	WeaponPlaySound(SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashStart01Cue');
}

simulated function SpecialMeleeAttackMakeNoiseTimer()
{
	// sometimes you can get your gun into a state where it is stuck in MeleeAttacking with no owner
	if( Instigator == None )
	{
		ClearTimer('SpecialMeleeAttackMakeNoiseTimer');
	}
	else
	{
		Instigator.MakeNoise( 0.75f, 'SpecialMeleeAttackMakeNoiseTimer' );
	}
}

/**
 * For AI controlled weapons we only spawn 1/2 decals.  For Human players we spawn 1/1 decals
 **/
simulated function SpawnImpactDecal( const class<GearDamageType> GearDmgType, const out ImpactInfo Impact, Physicalmaterial PhysMaterial )
{
	// always spawn a decal for the local player
	if( Instigator != None && Instigator.IsHumanControlled() && Instigator.IsLocallyControlled() )
	{
		Super.SpawnImpactDecal( GearDmgType, Impact, PhysMaterial );
	}
	// 50% of the time spawn a decal for others
	else
	{
		if( bShouldSpawnDecal )
		{
			Super.SpawnImpactDecal( GearDmgType, Impact, PhysMaterial );
		}
		bShouldSpawnDecal = !bShouldSpawnDecal;
	}
}

function float GetAIMeleeAttackRange()
{
	// override to the distance that GSM_ChainsawHold checks for targets
	return 95.0; // 64.0 sphere check, but it uses overlap so add in smallest cylinder radius
}

simulated protected function PlayFireSound()
{
	if ( (FireLoopAC == None) || !FireLoopAC.IsPlaying() )
	{
		// start loop
		FireLoopAC = GearWeaponPlaySoundLocalEx(FireSound, FireSound_Player, FireLoopAC);
		if( FireLoopAC != None )
		{
			FireLoopAC.bStopWhenOwnerDestroyed = TRUE;
		}
	}

	// Maybe alert AI of the noise
	if ( (WorldInfo.Netmode != NM_Client) && Instigator != None )
	{
		Instigator.MakeNoise( 1.f,'PlayFireSound' );
	}
}

simulated function StopFireEffects(byte FireModeNum)
{
	super.StopFireEffects(FireModeNum);

	if (FireModeNum == 0)
	{
		StopFireLoopingAudio();
	}
}

simulated protected function StopFireLoopingAudio()
{
	if (FireLoopAC != None)
	{
		FireLoopAC.FadeOut(0.1f, 0.f);
		FireLoopAC = None;
	}
	GearWeaponPlaySoundLocal(StopFireSound, StopFireSound_Player);
}

simulated function float GetAdjustedFOV(float BaseFOV)
{
	if ( (CurrentFireMode == MELEE_ATTACK_FIREMODE) )
	{
		return CameraFOVWhileChainsawing;
	}

	return BaseFOV;
}


defaultproperties
{
	bWeaponCanBeReloaded=TRUE

	InstantHitDamageTypes(0)=class'GDT_AssaultRifle'
	InstantHitDamageTypes(MELEE_ATTACK_FIREMODE)=class'GDT_ChainSaw'

	AmmoTypeClass=class'GearAmmoType_AssaultRifle'

	BS_MeleeAttack.Empty()
	BS_MeleeAttack(0)=(AnimName[BS_Std_Up]="AR_Melee_Saw_A",AnimName[BS_Std_Idle_Lower]="AR_Melee_Saw_A")
	BS_MeleeAttack(1)=(AnimName[BS_Std_Up]="AR_Melee_Saw_Low",AnimName[BS_Std_Idle_Lower]="AR_Melee_Saw_Low")

	MeleeAttackCameraBoneAnims(0)=(AnimName="Camera_Melee_Saw_A",CollisionTestVector=(X=68.f,Y=149.f,Z=78.f))
	MeleeAttackCameraBoneAnims(1)=(AnimName="Camera_Melee_Saw_B",CollisionTestVector=(X=-36.f,Y=97.f,Z=129.f))
	MeleeAttackCameraBoneAnims(2)=(AnimName="Camera_Melee_Saw_C",CollisionTestVector=(X=68.f,Y=-162.f,Z=74.f))
	MeleeAttackCameraBoneAnims(3)=(AnimName="Camera_Melee_Saw_D",CollisionTestVector=(X=-14.f,Y=-111.f,Z=95.f))

	SpecialMeleeAttackStartSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawStart01Cue'
	SpecialMeleeAttackIdleLoopSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawLoopIdle01Cue'
	SpecialMeleeAttackStopSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawStop01Cue'
	SpecialMeleeAttackSputterSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawStallCue'


	ChainsawSlashStartSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashStart01Cue'
	ChainSawSlashLoopSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashLoop01Cue'
	ChainsawSlashStopSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawSlashStop01Cue'

	ChainSawRipLoopSound=SoundCue'Weapon_AssaultRifle.Melee.ChainsawRipFleshCue'

	FireSound=SoundCue'Weapon_AssaultRifle.CogAssault.CogARifleFireLoopEnemyCue'			// looping
	FireSound_Player=SoundCue'Weapon_AssaultRifle.CogAssault.CogARifleFireLoopPlayerCue'	// looping
	StopFireSound=SoundCue'Weapon_AssaultRifle.CogAssault.CogARifleFireStopEnemyCue'
	StopFireSound_Player=SoundCue'Weapon_AssaultRifle.CogAssault.CogARifleFireStopPlayerCue'

	FireNoAmmoSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleFireEmptyCue'
	NoAmmoFireSoundDelay=0.25f

	WeaponReloadSound=None
	WeaponWhipSound=SoundCue'Weapon_AssaultRifle.Firing.CogRifleWhipCue'

	WeaponEquipSound=SoundCue'Weapon_AssaultRifle.Actions.CogRiflePickupCue'
	WeaponDeEquipSound=SoundCue'Weapon_AssaultRifle.Actions.CogRifleLowerCue'

	PickupSound=SoundCue'Weapon_AssaultRifle.Actions.CogRiflePickupCue'
	WeaponDropSound=SoundCue'Weapon_AssaultRifle.Actions.CogRifleDropCue'

	AIRating=1.f

	// Weapon Animation
	WeaponFireAnim="AR_Fire"
	WeaponReloadAnim="AR_Reload"
	WeaponReloadAnimFail="AR_Reload_Fumble"

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_AssaultRifle.Mesh.COGAssaultRifle'
		PhysicsAsset=PhysicsAsset'COG_AssaultRifle.Mesh.COGAssaultRifle_Physics'
		AnimTreeTemplate=AnimTree'COG_AssaultRifle.AT_COG_AssaultRifle'
		AnimSets(0)=AnimSet'COG_AssaultRifle.Animations.COG_AssaultRifle'
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	FireOffset=(X=61,Y=-0.5,Z=7.25)

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_AssaultRifle.EffectS.P_COG_AssaultRifle_MuzzleFlash_Constant'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'COG_AssaultRifle.EffectS.P_COG_AssaultRifle_MuzzleFlash_Constant'
	MuzFlashParticleSystemActiveReload=ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_MuzzleFlash_Constant_Charge1'
	MuzzleSmokeRate=5.f

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=245,G=174,B=122,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
	    Template=ParticleSystem'COG_AssaultRifle.Effects.P_GOG_AssaultRifle_Shells'
		Translation=(X=16,Z=6)
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'COG_AssaultRifle.EffectS.P_COG_AssaultRifle_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	Begin Object Class=ParticleSystemComponent Name=ChainSawImpactEffect0
	    bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	ChainSawImpactEffect=ChainSawImpactEffect0

	ChainSawImpactEffect_FromForward=ParticleSystem'Effects_Gameplay.Blood.P_ChainSaw_Blood_01'
	ChainSawImpactEffect_FromBehind=ParticleSystem'Effects_Gameplay.Blood.P_ChainSaw_Blood_Reverse_01'
	ChainSawImpactEffect_NoGore=ParticleSystem'Effects_Gameplay.Chainsaw.P_Chainsaw_Sparks_NoGore'

	Begin Object Class=ParticleSystemComponent Name=ChainSawSmokeEffect0
	    bAutoActivate=FALSE
		Template=ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_ChainSaw_Smoke'
		TickGroup=TG_PostUpdateWork
	End Object
	ChainSawSmokeEffect=ChainSawSmokeEffect0

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		StaticMesh=StaticMesh'COG_AssaultRifle.Assault_Rifle_Magazine'
		CollideActors=FALSE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=FALSE
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=FALSE
	End Object
	MagazineMesh=MagazineMesh0

	bCanDisplayReloadTutorial=TRUE
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=222,V=100,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=305,UL=128,VL=34)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Assault'

	bInstantHit=true

	TracerType=WTT_LongSkinny

	NeedReloadNotifyThreshold=5

	LC_EmisDefaultCOG=(R=2.0,G=4.0,B=8.0,A=1.0)
	LC_EmisDefaultLocust=(R=60.0,G=1.0,B=0.1,A=1.0)

	Recoil_Hand={(
		TimeDuration=0.2f,
		RotAmplitude=(X=1000,Y=150,Z=0),
		RotFrequency=(X=10,Y=10,Z=0),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
		LocAmplitude=(X=-8,Y=0,Z=0),
		LocFrequency=(X=10,Y=0,Z=0),
		LocParams=(X=ERS_Zero,Y=ERS_Zero,Z=ERS_Random)
	)}

	Recoil_Spine={(
		TimeDuration=0.2f,
		RotAmplitude=(X=500,Y=150,Z=0),
		RotFrequency=(X=10,Y=10,Z=0),
		RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero)
		)}

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=30,RightAmplitude=30,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.100)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformRipping1
		Samples(0)=(LeftAmplitude=80,RightAmplitude=80,LeftFunction=WF_Noise,RightFunction=WF_Noise,Duration=5.000)
		bIsLooping=TRUE
	End Object
	RippingWaveForm=ForceFeedbackWaveformRipping1

	WeaponID=WC_Lancer

	BS_MeleeIdle={(
		AnimName[BS_Std_Idle_Upper]	="AR_Melee_Saw_Idle",
		AnimName[BS_Std_Idle_Lower]	="AR_Melee_Saw_Idle",
		AnimName[BS_Std_Walk_Upper]	="AR_Melee_Saw_Idle",
		AnimName[BS_Std_Run_Upper]	="AR_Melee_Saw_Run"
		)}

	bUseMeleeHitTimer=FALSE

	MeleeTraceExtent=(X=32,Y=32,Z=32)
	MeleeImpactSound=SoundCue'Weapon_AssaultRifle.Reloads.CogRifleHitCue'

	bShouldSpawnDecal=TRUE

	bAllowTracers=TRUE
	bCanParryMelee=false

	bSupportsBarrelHeat=TRUE
	BarrelHeatInterpSpeed=6.f
	BarrelHeatPerShot=0.3f
	BarrelHeatCooldownTime=1.25f
	MaxBarrelHeat=2.f
	BarrelHeatMaterialParameterName="AssaultRifle_Heat"
}