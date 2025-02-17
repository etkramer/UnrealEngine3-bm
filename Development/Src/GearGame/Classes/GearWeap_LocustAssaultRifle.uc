/**
 * Locust Assault Rifle: Hammerburst
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_LocustAssaultRifle extends GearWeapon;

/** How much extra recoil to add each time the weapon is fired. */
var() protected const config float	ExtraRecoilPerShot;
/** How much extra recoil to add each time the weapon is fired for AR-bonus shots. */
var() protected const config float	ExtraRecoilPerShotAR;

/** How much extra recoil there is currently (this diminishes over time) */
var protected transient float		CurrentExtraRecoil;

/** FOV (in deg) of the zoomed state of this weapon */
var() protected const config float	ZoomFOV;

/** Zoom in / out sounds **/
var protected const SoundCue		ZoomActivatedSound;
var protected const SoundCue		ZoomDeActivatedSound;

/** number of shots remaining in AI quick fire */
var int BotFannedShots;


/** Weapons get a crack at handling controls */
simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	local GearPC PC;

	if( super.HandleButtonPress( ButtonName ) == FALSE )
	{
		if( InputCanZoom(ButtonName) )
		{
			PC = GearPC(Instigator.Controller);

			//@todo should be able to click on R3 and zoom in target instantly
			if( ( PC != None ) && ( PC.bIsTargeting == TRUE ) )
			{
				SetZoomed(!GearPawn(Instigator).bIsZoomed);

				if( GearPawn(Instigator).bIsZoomed == TRUE )
				{
					GearWeaponPlaySoundLocal(ZoomActivatedSound,, TRUE);
				}
				else
				{
					GearWeaponPlaySoundLocal(ZoomDeactivatedSound,, TRUE);
				}

				return TRUE;
			}
		}
	}

	return FALSE;
}


/**
* Returns FOV, in deg, of this weapon when it is in targeting mode.
*/
simulated function float GetTargetingFOV( float BaseFOV )
{
	if( GearPawn(Instigator).bIsZoomed == TRUE )
	{
		return ZoomFOV;
	}
	else
	{
		return Super.GetTargetingFOV( BaseFOV );
	}
}

/** Returns the value to scale the cross hair by */
simulated function float GetCrossHairScale()
{
	if( GearPawn(Instigator).bIsZoomed == TRUE )
	{
		return 0.5f;
	}
	else
	{
		return 0.7f;
	}
}

/** Overridden to add extra recoil to subsequent shots. */
simulated function PlayOwnedFireEffects( byte FireModeNum, vector HitLocation  )
{
	local GearPawn GP;

	super.PlayOwnedFireEffects(FireModeNum, HitLocation);

	GP = GearPawn(Instigator);
	if ((GP != None) && (GP.bActiveReloadBonusActive))
	{
		CurrentExtraRecoil += ExtraRecoilPerShotAR;
	}
	else
	{
		CurrentExtraRecoil += ExtraRecoilPerShot;
	}

}

simulated function Tick(float DeltaTime)
{
	super.Tick(DeltaTime);

	// ExtraRecoil diminishes such that it is zero at the default fire rate (only if we are not a GearAIController)
	if( (GearAIController == none) && (CurrentExtraRecoil > 0.f) )
	{
		CurrentExtraRecoil -= DeltaTime * ExtraRecoilPerShot / GetFireInterval(0);
		CurrentExtraRecoil = FMax(0.f, CurrentExtraRecoil);
	}
}

simulated function StartFire(byte FireModeNum)
{
	local Actor Target;

	// don't bother firing if the enemy is really far away
	if (GearAIController != None && FireModeNum == 0)
	{
		Target = (GearAIController.FireTarget != None) ? GearAIController.FireTarget : GearAIController.Enemy;
		if (Target != None && VSize(Target.Location - GearAIController.Pawn.Location) > Range_Long * 1.5)
		{
			return;
		}
	}

	Super.StartFire(FireModeNum);
}

function SetupWeaponFire(byte FireModeNum)
{
	local GearAI_TDM Bot;

	Super.SetupWeaponFire(FireModeNum);

	// high level bots sometimes do the hammer on the button to fire faster thing
	Bot = GearAI_TDM(Instigator.Controller);
	if ( Bot != None && Bot.GetDifficultyLevel() > DL_Normal && Bot.MyGearPawn != None &&
		(Bot.MyGearPawn.IsInCover() || Bot.MyGearPawn.bIsTargeting) &&
		FRand() < Bot.GetDifficultyLevel() * 0.25 )
	{
		BotFannedShots = 3 + Rand(4);
	}
	else
	{
		BotFannedShots = 0;
	}
}

simulated state Active
{
	simulated function BeginState( Name PreviousStateName )
	{
		super.BeginState(PreviousStateName);

		if (PreviousStateName != FiringStatesArray[0])
		{
			CurrentExtraRecoil = 0.f;
		}
	}

	simulated function TargetingModeChanged(GearPawn P)
	{
		local GearPC PC;

		Global.TargetingModeChanged(P);

		if( (Instigator != none) && GearPawn(Instigator).bIsZoomed )
		{
			// we want to leave zoomed state if we stop targeting, but stay zoomed across a reload
			// (which involves leaving targeting mode for a moment).
			PC = GearPC(Instigator.Controller);
			if ( PC != None && !PC.bIsTargeting && !IsReloading() )
			{
				SetZoomed(FALSE);
			}
		}
	}
}


simulated function float GetFireInterval(byte FireModeNum)
{
	local float Result;

	Result = Super.GetFireInterval(FireModeNum);
	// reduce fire interval directly instead of doing the human player path to make sure
	// all the usual AI notifications are called
	if (GearAIController != None && BotFannedShots > 0)
	{
		Result *= (0.6 - 0.2 * FRand());
	}
	return Result;
}

function GetAIAccuracyCone(out vector2D AccCone_Min, out vector2D AccCone_Max)
{
	Super.GetAIAccuracyCone(AccCone_Min, AccCone_Max);

	// pitch accuracy penalty while fanning to mimic recoil
	if (BotFannedShots > 0)
	{
		AccCone_Min.X += 1.0;
	}
}

simulated state WeaponFiring
{
	simulated function RefireCheckTimer()
	{
		if (BotFannedShots > 0)
		{
			BotFannedShots--;
		}

		Super.RefireCheckTimer();
	}

	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);
		SetTimer(0.1,FALSE,nameof(DelayedReturnToActive));
	}
}

simulated function DelayedReturnToActive()
{
	if (IsInState('WeaponFiring'))
	{
		GotoState('Active');
	}
}

/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	// Skip on dedicated servers, below is just cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// Hide Ammo part on weapon mesh
	SetWeaponAmmoBoneDisplay( FALSE );

	// Attach magazine mesh to left hand
	SetPawnAmmoAttachment(TRUE);
}


/** notification fired from Pawn animation when player throws ammo */
simulated function Notify_AmmoThrow()
{
	local Vector	MagLoc;
	local Rotator	MagRot;

	if (Instigator != None)
	{
		// Detach magazine mesh from instigator's left
		SetPawnAmmoAttachment(FALSE);
		// Spawn physics Magazine from pawn's hand
		Instigator.Mesh.GetSocketWorldLocationAndRotation(GearPawn(Instigator).GetLeftHandSocketName(), MagLoc, MagRot);
		SpawnPhysicsMagazine(MagLoc, MagRot);
	}
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

/*
simulated function LinearColor GetWeaponEmisColor_COG()
{
	return WorldInfo.Gears_COGWeapons.Hammerburst;
}

simulated function LinearColor GetWeaponEmisColor_Locust()
{
	return WorldInfo.Gears_LocustWeapons.Hammerburst;
}
*/

simulated function int GetWeaponRecoil()
{
	return super.GetWeaponRecoil() + CurrentExtraRecoil;
}

defaultproperties
{
	bWeaponCanBeReloaded=TRUE

	InstantHitDamageTypes(0)=class'GDT_LocustAssaultRifle'
	AmmoTypeClass=class'GearAmmoType_LocustAssaultRifle'

	FireSound=SoundCue'Weapon_AssaultRifle.LocustAssault.LocustARifleFireEnemyCue'
	FireSound_Player=SoundCue'Weapon_AssaultRifle.LocustAssault.LocustARifleFirePlayerCue'
    FireNoAmmoSound=SoundCue'Weapon_AssaultRifle.Firing.LocustRifleFireEmptyCue'

	WeaponReloadSound=None
	WeaponWhipSound=SoundCue'Weapon_AssaultRifle.Firing.LocustRifleWhipCue'

	WeaponEquipSound=SoundCue'Weapon_AssaultRifle.Actions.LocustRifleRaiseCue'
	WeaponDeEquipSound=SoundCue'Weapon_AssaultRifle.Actions.LocustRifleLowerCue'

	PickupSound=SoundCue'Weapon_AssaultRifle.Actions.LocustRiflePickupCue'
	WeaponDropSound=SoundCue'Weapon_AssaultRifle.Actions.LocustRifleDropCue'

	AIRating=1.f

	FireOffset=(X=44,Y=-1,Z=11)

	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=SkeletalMesh'Locust_Hammerburst.Locust_Hammerburst'
		PhysicsAsset=PhysicsAsset'Locust_Hammerburst.Locust_Hammerburst_Physics'
		AnimTreeTemplate=AnimTree'Locust_Hammerburst.AT_HammerBurst'
		AnimSets(0)=AnimSet'Locust_Hammerburst.Locust_Hammerburst_Anims'
    End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	// muzzle flash emitter
	Begin Object Name=PSC_WeaponMuzzleMuzFlashComp
		Template=ParticleSystem'Locust_Hammerburst.EffectS.P_Locust_Hammerburst_MuzzleFlash'
	End Object
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	MuzFlashParticleSystem=ParticleSystem'Locust_Hammerburst.EffectS.P_Locust_Hammerburst_MuzzleFlash'
	MuzFlashParticleSystemActiveReload=ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_MuzzleFlash_AR'

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=242,G=88,B=30,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5
	bLoopMuzzleFlashLight=FALSE

	// shell case ejection emitter
	Begin Object Name=PSC_WeaponShellCaseComp
		Template=ParticleSystem'Locust_Hammerburst.EffectS.P_Locust_Hammerburst_Shells'
		Translation=(X=16,Z=6)
	End Object
	PSC_ShellEject=PSC_WeaponShellCaseComp

	// reload barrel smoke
	Begin Object Name=PSC_WeaponReloadBarrelSmokeComp
		Template=ParticleSystem'Locust_Hammerburst.Effects.P_Locust_Hammerburst_Smoke'
	End Object
	PSC_ReloadBarrelSmoke=PSC_WeaponReloadBarrelSmokeComp

	Begin Object Class=StaticMeshComponent Name=MagazineMesh0
		StaticMesh=StaticMesh'Locust_Hammerburst.Hammerburst_Magazine'
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
	DamageTypeClassForUI=class'GDT_LocustAssaultRifle'
	WeaponIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=192,V=6,UL=143,VL=46)
	AnnexWeaponIcon=(U=0,V=0,UL=128,VL=40)

	bInstantHit=true

	TracerType=WTT_Hammerburst

	CrosshairIcon=Texture2D'Warfare_HUD.HUD.HUD_Xhair_COG_Assault'

	NeedReloadNotifyThreshold=3

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Hammerburst'

	// Weapon Animation
	WeaponReloadAnim="HB_Reload"
	WeaponReloadAnimFail="HB_Reload_Fail"

	MeleeImpactSound=SoundCue'Weapon_AssaultRifle.Reloads.CogRifleHitCue'
	ZoomActivatedSound=SoundCue'Interface_Audio.Interface.WeaponZoomIn_Cue'
	ZoomDeActivatedSound=SoundCue'Interface_Audio.Interface.WeaponZoomOut_Cue'

	LC_EmisDefaultCOG=(R=0.5,G=8.0,B=50.0,A=1.0)
	LC_EmisDefaultLocust=(R=40.0,G=1.4,B=0.0,A=1.0)


	Recoil_Hand={(
				LocAmplitude=(X=-8,Y=0,Z=0),
				LocFrequency=(X=10,Y=0,Z=0),
				LocParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				RotAmplitude=(X=4000,Y=250,Z=0),
				RotFrequency=(X=5,Y=20,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				TimeDuration=0.2f
				)}

	Recoil_Spine={(
				RotAmplitude=(X=2500,Y=500,Z=0),
				RotFrequency=(X=5,Y=5,Z=0),
				RotParams=(X=ERS_Zero,Y=ERS_Random,Z=ERS_Zero),
				TimeDuration=0.25f
				)}

	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveformShooting1
		Samples(0)=(LeftAmplitude=50,RightAmplitude=50,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.300)
	End Object
	WeaponFireWaveForm=ForceFeedbackWaveformShooting1

	WeaponID=WC_Hammerburst

	bAllowTracers=TRUE

	bMuzFlashEmitterIsLooping=FALSE

	bSupportsBarrelHeat=TRUE
	BarrelHeatInterpSpeed=6.f
	BarrelHeatPerShot=0.3f
	BarrelHeatCooldownTime=1.25f
	MaxBarrelHeat=2.f
	BarrelHeatMaterialParameterName="Hammerburst_Heat"
}
