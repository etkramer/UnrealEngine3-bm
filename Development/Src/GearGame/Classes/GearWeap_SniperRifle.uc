/**
 * Sniper Rifle
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_SniperRifle extends GearWeapon;

/** sound of shells colliding against ground*/
var() protected const SoundCue				ShellCollisionSound;

/** FOV (in deg) of the zoomed state of this weapon */
var() protected const config float			ZoomFOV;

/** separate fire camera shake for zoomed mode, since the regular camera shake feels enormous when zoomed */
var() protected config ScreenShakeStruct	FireCameraShake_Zoomed;

/** Zoom in / out sounds **/
var protected const SoundCue				ZoomActivatedSound;
var protected const SoundCue				ZoomDeActivatedSound;

var float TimeSinceTargeting;

/**
 * adjustment applied to player rot to offset differences in camera loc.  goal is for
 * zoomed and unzoomed to aim at same place.  note that a constant offset isn't perfect
 * since offset should scale with target range, but this is easy and it helps.
 */
var() const rotator TargetingRotAdjustment;

simulated function float GetWeaponRating()
{
	local float Rating;

	Rating = Super.GetWeaponRating();

	// AI prefers sniper at long range
	if ( GearAI(Instigator.Controller) != None && Instigator.Controller.Enemy != None &&
		!GearAI(Instigator.Controller).IsShortRange(Instigator.Controller.Enemy.Location) )
	{
		Rating += 0.5;
	}

	return Rating;
}

simulated state MeleeAttacking
{
	simulated function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
		SetZoomed(FALSE,TRUE);
	}
}

simulated function TargetingModeChanged(GearPawn P)
{
	TimeSinceTargeting = WorldInfo.TimeSeconds;
}
/**
 * State WeaponFiring
 * Fires a bullet at a time
 */
simulated state WeaponFiring
{
	simulated function TargetingModeChanged(GearPawn P)
	{
		local GearPC PC;

		TimeSinceTargeting = WorldInfo.TimeSeconds;
		if( (Instigator != none) && GearPawn(Instigator).bIsZoomed )
		{
			// we want to leave zoomed state if we stop targeting, but stay zoomed across a reload
			// (which involves leaving targeting mode for a moment).
			PC = GearPC(Instigator.Controller);
			if ( PC != None )
			{
				if ( !PC.bIsTargeting && !IsReloading() )
				{
					SetZoomed(FALSE,TRUE);
				}
				else
				{
					SetOwnerNoSee(PC.bIsTargeting);
				}
			}
		}
	}

	simulated function EndState(name NextStateName)
	{
		super.EndState(NextStateName);

		if ( NextStateName != 'Active' )
		{
			SetOwnerNoSee(FALSE);
		}
	}
}

simulated function bool ShouldAutoReload()
{
	if (Instigator != None && Instigator.IsHumanControlled() && PendingFire(0))
	{
		// don't autoreload while still holding the fire button
		return FALSE;
	}

	return Super.ShouldAutoReload();
}

simulated function PlayWeaponReloading()
{
	Super.PlayWeaponReloading();
	// force the player to remain in the aiming pose after a reload
	if (GearPawn(Instigator) != None)
	{
		GearPawn(Instigator).SetWeaponAlert(8.f);
	}
}

simulated function PlayShellCaseEject()
{
	//SetTimer( 1.70, FALSE, nameof(PlayShellCaseEject_Sniper) );
}


simulated function PlayBoltEject()
{
	Super.PlayShellCaseEject();

	if( !bSuppressAudio )
	{
		Playsound( SoundCue'Weapon_Sniper.Reloads.CogSniperAmmoEjectCue', true );
	}
	SetTimer( 0.750, FALSE, nameof(PlayShellCaseLandImpact) );
}


simulated function PlayShellCaseLandImpact()
{
	if ( (WorldInfo.NetMode != NM_DedicatedServer) && !bSuppressAudio )
	{
		Instigator.PlaySound( ShellCollisionSound, true );
	}
}

simulated function PlayFireEffects(byte FireModeNum, optional vector HitLocation)
{
	local Emitter	TrailEmitter;
	local Vector	SpawnLoc;
	local Rotator	SpawnRot;

	SpawnLoc = GetMuzzleLoc() + vect(0,0,10); // move it up a little particle and socket interaction is a bit off

	if ( UseFirstPersonCamera() )
	{
		SpawnLoc -= vect(0,0,10);
	}

	// if we are on the client when we will not have correct HitLocation so we will calculate an
	// approximate one for our tracer to use.
	if( Role < ROLE_Authority )
	{
		HitLocation = SpawnLoc + vector(GetAdjustedAim(SpawnLoc)) * GetTraceRange();
	}

	SpawnRot = Rotator(HitLocation - SpawnLoc);

	TrailEmitter = GetImpactEmitter( ParticleSystem'COG_SniperRifle.Effects.P_COG_SniperRifle_Beam', SpawnLoc, SpawnRot );
	TrailEmitter.ParticleSystemComponent.SetFloatParameter('BeamLength', VSize(HitLocation - SpawnLoc));
	TrailEmitter.ParticleSystemComponent.ActivateSystem();

	super.PlayFireEffects(FireModeNum, HitLocation);
}

simulated function float GetPlayerAimError()
{
	local GearPawn GP;
	GP = GearPawn(Instigator);
	if (GP != None && WorldInfo.GRI.IsMultiplayerGame() && (!GP.bIsTargeting || TimeSince(TimeSinceTargeting) < 0.4f))
	{
		return WeaponAimError.Y;
	}
	else
	{
		return Super.GetPlayerAimError();
	}
}

simulated event WeaponStoppedFiring(byte FiringMode)
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "FiringMode:" @ FiringMode );
	if (Instigator != None)
	{
		Instigator.WeaponStoppedFiring(FALSE);
	}

	StopFireEffects(FiringMode);

	/*
	// if we're not currently reloading our weapon, play idle animation
	if( FiringMode != RELOAD_FIREMODE )
	{
	GearPawn(Instigator).PlayWeaponStanceIdle();
	}
	*/

	// turn off shell eject particle system
	if( PSC_ShellEject != None )
	{
		PSC_ShellEject.DeActivateSystem();
		SetTimer( 3.0f, FALSE, nameof(HidePSC_ShellEject) );
	}
}


simulated function PlayMuzzleFlashEffect()
{
	// do not play the muzzle flash effect if you are zoomed and the local player
	if (Instigator == None || !Instigator.IsLocallyControlled() || !GearPawn(Instigator).bIsZoomed)
	{
		Super.PlayMuzzleFlashEffect();
	}
}


/**
 * Returns FOV, in deg, of this weapon when it is in targeting mode.
 */
simulated function float GetTargetingFOV(float BaseFOV)
{
	if (GearPawn(Instigator).bIsZoomed)
	{
		return ZoomFOV;
	}
	else
	{
		return Super.GetTargetingFOV(BaseFOV);
	}
}

/** Returns the value to scale the cross hair by */
simulated function float GetCrossHairScale()
{
	if( GearPawn(Instigator).bIsZoomed )
	{
		return 1.0f; // we have a new sniper reticle
	}
	else
	{
		return 0.7f;
	}
}

simulated function Texture2D GetCrosshairIcon(GearPC PC, out float YOffset)
{
	return GearPawn(Instigator).bIsZoomed ? CrosshairIconSecondary : CrosshairIcon;
}

simulated function TimeWeaponEquipping()
{
	SetZoomed(FALSE,TRUE);
	Super.TimeWeaponEquipping();
}

simulated function TimeWeaponPutDown()
{
	SetZoomed(FALSE,TRUE);
	Super.TimeWeaponPutDown();
}


/** Weapons get a crack at handling controls */
simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	local GearPC		PC;
	local GearPawn	InstigatorWP;
	local rotator	RotAdj;

	if( !Super.HandleButtonPress(ButtonName) )
	{
		if ( InputCanZoom(ButtonName) )
		{
			PC = GearPC(Instigator.Controller);
			InstigatorWP = GearPawn(Instigator);

			//@todo should be able to click on R3 and zoom in target instantly
			if ( (PC != None) && (InstigatorWP != None) && PC.bIsTargeting && ( IsInState( 'MeleeAttacking' ) == FALSE ) )
			{
				SetZoomed(!GearPawn(Instigator).bIsZoomed, TRUE);

				// flip the yaw adjustment if player is mirrored
				RotAdj = TargetingRotAdjustment;

				if (InstigatorWP.bIsMirrored)
				{
					RotAdj.Yaw = -RotAdj.Yaw;
				}

				if( GearPawn(Instigator).bIsZoomed )
				{
					GearWeaponPlaySoundLocal(ZoomActivatedSound,, TRUE);
					PC.ExtraRot += RotAdj;
				}
				else
				{
					GearWeaponPlaySoundLocal(ZoomDeactivatedSound,, TRUE);
					PC.ExtraRot -= RotAdj;
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}

simulated function ScreenShakeStruct GetFireViewShake()
{
	return (GearPawn(Instigator).bIsZoomed ? FireCameraShake_Zoomed : FireCameraShake);
}

/**
 * Zoomed sniper rifle wants first person camera.
 * @returns true if Weapon would like a first person camera to be used when targeting
 */
simulated function bool UseFirstPersonCamera()
{
	return GearPawn(Instigator).bIsZoomed;
}

/**
 * Notification from pawn that it has been unpossessed.
 */
function NotifyUnpossessed()
{
	SetZoomed(FALSE,TRUE);
}

simulated state Active
{
	simulated function Tick(float DeltaTime)
	{
		if( ShouldAutoReload() )
		{
			ForceReload();
		}

		Super.Tick(DeltaTime);
	}

	simulated function TargetingModeChanged(GearPawn P)
	{
		local GearPC PC;

		TimeSinceTargeting = WorldInfo.TimeSeconds;
		if( (Instigator != none) && GearPawn(Instigator).bIsZoomed )
		{
			// we want to leave zoomed state if we stop targeting, but stay zoomed across a reload
			// (which involves leaving targeting mode for a moment).
			PC = GearPC(Instigator.Controller);
			if ( PC != None )
			{
				if ( !PC.bIsTargeting && !IsReloading() )
				{
					SetZoomed(FALSE,TRUE);
				}
				else
				{
					SetOwnerNoSee(PC.bIsTargeting);
				}
			}
		}
	}

	simulated function ForceReload()
	{
		// don't make it actually force for the sniper so we can stay zoomed as long as we want
		if (ShouldAutoReload())
		{
			Super.ForceReload();
		}
	}

	simulated function BeginState(name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);

		SetOwnerNoSee(GearPawn(Instigator).bIsZoomed);
	}

	simulated function EndState(name NextStateName)
	{
		super.EndState(NextStateName);

		if ( NextStateName != 'WeaponFiring' )
		{
			SetOwnerNoSee(FALSE);
		}
	}
}

simulated state Reloading
{
	ignores ForceReload;

	simulated function TargetingModeChanged(GearPawn P)
	{
		local GearPC PC;

		TimeSinceTargeting = WorldInfo.TimeSeconds;
		if( (Instigator != none) && GearPawn(Instigator).bIsZoomed )
		{
			// we want to leave zoomed state if we stop targeting, but stay zoomed across a reload
			// (which involves leaving targeting mode for a moment).
			PC = GearPC(Instigator.Controller);
			if ( (PC != None) && !PC.bIsTargeting )
			{
				SetZoomed(FALSE,TRUE);
			}
		}
	}
}

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.SniperRifle;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.SniperRifle;
}
*/

simulated function bool ShouldUseMuzzleFlashSocketForShellEjectAttach()
{
	return TRUE;
}

defaultproperties
{
	bWeaponCanBeReloaded=TRUE
	bAutoSwitchWhenOutOfAmmo=FALSE
	bSuperSteadyCamWhileTargeting=TRUE
	TargetingRotAdjustment=(Pitch=45,Yaw=73,Roll=0)

	FireSound=SoundCue'Weapon_Sniper.Sniper.CogSniperRifleFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_Sniper.Sniper.CogSniperRifleFirePlayerCue'
	FireNoAmmoSound=SoundCue'Weapon_Sniper.Firing.CogSniperFireEmptyCue'

	WeaponReloadSound=SoundCue'Weapon_Sniper.Reloads.CogSniperAmmoInsertCue'
	WeaponWhipSound=SoundCue'Weapon_Sniper.Firing.CogSniperWhipCue'
	ShellCollisionSound=SoundCue'Weapon_Sniper.Reloads.CogSniperAmmoBounceBigCue'

	WeaponEquipSound=SoundCue'Weapon_Sniper.Actions.CogSniperRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_Sniper.Actions.CogSniperLowerCue'

	PickupSound=SoundCue'Weapon_Sniper.Actions.CogSniperPickupCue'

	AIRating=1.f

	InstantHitDamageTypes(0)=class'GDT_SniperRifle'
	AmmoTypeClass=class'GearAmmoType_SniperRifle'

	// Weapon Animation
	Begin Object Class=AnimNodeSequence Name=WeaponAnimNode
	End Object

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'COG_SniperRifle.LongShot'
		PhysicsAsset=PhysicsAsset'COG_SniperRifle.LongShot_Physics'
		AnimSets(0)=AnimSet'COG_SniperRifle.LongShot'
		Animations=WeaponAnimNode
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	FireOffset=(X=105,Y=-2,Z=11)

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'COG_SniperRifle.Effects.P_COG_SniperRifle_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'COG_SniperRifle.Effects.P_COG_SniperRifle_MuzzleFlash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'COG_SniperRifle.Effects.P_COG_SniperRifle_MuzzleFlash_AR'

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
	    LightColor=(R=213,G=137,B=91,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.2f
	MuzzleLightPulseFreq=20
	MuzzleLightPulseExp=1.5

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'COG_SniperRifle.P_COG_Sniper_shell'
		Translation=(X=-100,Y=0,Z=0)
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp

	// make the sniper rifle's damage duration longer due to how it is used
//	ActiveReload_BonusDurationTier=(0.0f,4.0f,4.0f,4.0f);

	DamageTypeClassForUI=class'GDT_SniperRifle'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=337,V=4,UL=173,VL=46)
	AnnexWeaponIcon=(U=0,V=119,UL=128,VL=25)
	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Longshot'
	CrosshairIconSecondary=Texture2D'Warfare_HUD.HUD_Xhair_COG_Sniper'

	bIsSuppressive=FALSE
	bSniping=true
	bInstantHit=true
	bCanNegateMeatShield=true
	bIgnoresExecutionRules=true

	bAllowTracers=FALSE
	TracerType=WTT_MinigunFastFiring

	// easy on the autoaim, sniper is a precision weapon, esp when zoomed
	AimAssistScale=0.5f
	AimAssistScaleWhileTargeting=0.f

	HUDDrawData			= (DisplayCount=1, AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=296,UL=106,VL=7),ULPerAmmo=43)
	HUDDrawDataSuper	= (DisplayCount=1, AmmoIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=296,UL=106,VL=7),ULPerAmmo=43)

	ZoomActivatedSound=SoundCue'Weapon_Sniper.Reloads.CogSniperEyeActivateCue'
	ZoomDeActivatedSound=SoundCue'Weapon_Sniper.Reloads.CogSniperEyeDeActivateCue'

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.AnimSetMarcus_CamSkel_LongShot'
	WeaponAnimType=EWAT_SniperRifle

	WeaponFireAnim=""
	WeaponReloadAnim="LS_Reload"
	WeaponReloadAnimFail="LS_Reload_Fail"

	BS_PawnWeaponReload={(
		AnimName[BS_Std_Up]				="LS_Idle_Ready_Reload",
		AnimName[BS_CovStdIdle_Up]		="LS_Cov_Std_Reload",
		AnimName[BS_CovMidIdle_Up]		="LS_Cov_Mid_Reload"
	)}

	BS_PawnWeaponReloadFail={(
		AnimName[BS_Std_Up]				="LS_Idle_Ready_Reload_Fail",
		AnimName[BS_CovStdIdle_Up]		="LS_Cov_Std_Reload_Fail",
		AnimName[BS_CovMidIdle_Up]		="LS_Cov_Mid_Reload_Fail"
	)}

	MeleeImpactSound=SoundCue'Weapon_Sniper.Actions.CogSniperDropCue'

	LC_EmisDefaultCOG=(R=1.0,G=3.0,B=10.0,A=1.0)
	LC_EmisDefaultLocust=(R=16.0,G=0.8,B=0.2,A=1.0)

	Recoil_Hand={(
				LocAmplitude=(X=-6,Y=0,Z=0),
				LocFrequency=(X=15,Y=0,Z=0),
				LocParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				RotAmplitude=(X=3000,Y=0,Z=0),
				RotFrequency=(X=5,Y=0,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				TimeDuration=0.33f
				)}

	Recoil_Spine={(
				RotAmplitude=(X=750,Y=0,Z=0),
				RotFrequency=(X=12,Y=0,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				TimeDuration=0.67f
				)}

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.500)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

    WeaponID=WC_Longshot

	// CQC Long Executions
	CQC_Long_KillerAnim="CTRL_LongShot"
	CQC_Long_VictimAnim="DBNO_LongShot"
	CQC_Long_CameraAnims.Empty
	CQC_Long_CameraAnims(0)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_LongShot_Cam01'
	CQC_Long_CameraAnims(1)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_LongShot_Cam02'
	CQC_Long_CameraAnims(2)=CameraAnim'COG_MarcusFenix.Camera_Anims.DBNO_LongShot_Cam03'
	CQC_Long_bHolsterWeapon=FALSE
	CQC_Long_VictimDeathTime=1.97f
	CQC_Long_VictimRotStartTime=0.f
	CQC_Long_VictimRotInterpSpeed=0.f
	CQC_Long_EffortID=GearEffort_SniperLongExecutionEffort
	CQC_Long_DamageType=class'GDT_LongExecution_Sniper'

	CQC_Quick_KillerAnim="CTRL_Quick_Sniper"
	CQC_Quick_VictimDeathTime=0.67f
	CQC_Quick_EffortID=GearEffort_SniperQuickExecutionEffort
	CQC_Quick_DamageType=class'GDT_QuickExecution_Sniper'
}
