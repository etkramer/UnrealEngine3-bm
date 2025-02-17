/**
 * Pistol Base Class
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearWeap_PistolBase extends GearWeapon
	native(Weapon)
	abstract;

/** FOV (in deg) of the zoomed state of this weapon */
var() protected const config float ZoomFOV;

/** Zoom in / out sounds **/
var protected const SoundCue ZoomActivatedSound;
var protected const SoundCue ZoomDeActivatedSound;

var() float FireAnimRateScale;

/** Override animations for Shield Reloads */
var BodyStance	ShieldReloadOverride, ShieldReloadFailedOverride, CrateCarryReloadOverride, CrateCarryReloadFailedOverride;
var BodyStance	BS_MeleeAttackKidnapper, BS_MeleeAttackHostage;

/** Reference to duel grenade weapon */
var GearWeap_GrenadeBase	GrenWeap;

/** Weapons get a crack at handling controls */
simulated function bool HandleButtonPress( coerce Name ButtonName )
{
	local GearPC PC;
	local GearPawn InstigatorGP;

	if( super.HandleButtonPress( ButtonName ) == FALSE )
	{
		if( InputCanZoom(ButtonName) )
		{
			PC = GearPC(Instigator.Controller);
			InstigatorGP = GearPawn(Instigator);

			//@todo should be able to click on R3 and zoom in target instantly
			if ( (PC != None) && (InstigatorGP != None) && (PC.bIsTargeting || InstigatorGP.IsDeployingShield()) )
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

simulated function float GetAdjustedFOV(float BaseFOV)
{
	local GearPawn InstigatorGP;

	if (GearPawn(Instigator).bIsZoomed)
	{
		InstigatorGP = GearPawn(Instigator);
		if ( (InstigatorGP != None) && InstigatorGP.IsDeployingShield() )
		{
			// deploying shield isn't "targeting" mode, so we need to support the zoom fov during that
			// action in here
			return ZoomFOV;
		}
	}

	return super.GetAdjustedFOV(BaseFOV);
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


simulated function float GetWeaponRating()
{
	// always equip pistol when kidnapping
	if (GearAIController != None && GearAIController.MyGearPawn != None && GearAIController.MyGearPawn.IsAKidnapper())
	{
		return 10.0;
	}
	return 0.4f;
}

simulated state Active
{
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

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	Super.FireModeUpdated(FiringMode, bViaReplication);

	// Reloading status may have changed, check if we should update IK or not.
	UpdatePistolLeftHandIK();
}

simulated function TargetingModeChanged(GearPawn P)
{
	Super.TargetingModeChanged(P);

	// targeting mode changed, see if left hand ik should be toggled.
	UpdatePistolLeftHandIK();
}

/** Notification called when Pawn.bDoing360Aiming flag changes. */
simulated function On360AimingChangeNotify()
{
	Super.On360AimingChangeNotify();

	// 360 aiming changed, see if left hand ik should be toggled.
	UpdatePistolLeftHandIK();
}

/**
 * Updates Left Hand IK
 * sees if it should be on or off depending on context.
 */
simulated function UpdatePistolLeftHandIK()
{
	local GearPawn P;

	P = GearPawn(Instigator);

	// When left hand should be IK'd
	// When doing 360 aiming, when targeting, when reloading weapon
	if( P.IsAKidnapper() || P.bDoing360Aiming || P.bIsTargeting || P.IsReloadingWeapon() )
	{
		bDisableLeftHandIK = FALSE;
		P.UpdateBoneLeftHandIK();
	}
	else
	{
		bDisableLeftHandIK = TRUE;
		P.UpdateBoneLeftHandIK();
	}
}

simulated function TimeWeaponEquipping()
{
	Super.TimeWeaponEquipping();

	// weapon equipped, update status of left hand ik
	UpdatePistolLeftHandIK();
}

simulated function EjectClip();

simulated function CockOpen();

simulated function CockClose();

simulated function InsertClip();

simulated function Jammed();

simulated function HandSlam();

simulated function ClipImpact();

/** Add support for Shield and MeatShield specific reloads */
simulated function PlayWeaponReloading()
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);

	// Override reload animations with shield override if carrying shield
	BS_PawnWeaponReload = default.BS_PawnWeaponReload;
	if( PawnOwner != None )
	{
		if( PawnOwner.CarriedCrate != None )
		{
			OverrideBodyStance(BS_PawnWeaponReload, CrateCarryReloadOverride);
		}
		else if( PawnOwner.IsCarryingShield() )
		{
			OverrideBodyStance(BS_PawnWeaponReload, ShieldReloadOverride);
		}
	}

	// Clear weapon reload animations if a kidnapper or carrying shield.
	// We have simplified animations for those.
	if( ShouldPlaySimplePistolReload() )
	{
		WeaponReloadAnim = '';
		WeaponReloadAnimFail = '';
	}
	else
	{
		WeaponReloadAnim = default.WeaponReloadAnim;
		WeaponReloadAnimFail = default.WeaponReloadAnimFail;
	}

	Super.PlayWeaponReloading();

	// If we're a kidnapper, have the hostage play those animations too!
	if( PawnOwner.IsAKidnapper() )
	{
		PawnOwner.InteractionPawn.BS_Play(BS_PawnWeaponReload, 1.f, 0.25f, 0.33f, FALSE, TRUE, 'WeaponReload');
	}

}


/** 
 * If this is TRUE, play simpler version for kidnapper and shield carrying state.
 * Less fancy.
 */
simulated final function bool ShouldPlaySimplePistolReload()
{
	local GearPawn	PawnOwner;

	PawnOwner = GearPawn(Instigator);
	if( PawnOwner != None && (PawnOwner.IsAKidnapper() || PawnOwner.IsCarryingShield()) )
	{
		return TRUE;
	}

	return FALSE;
}

/** Utility function to override a given body stance with another one. */
simulated function OverrideBodyStance(out BodyStance BaseStance, out BodyStance OverrideStance)
{
	local int OverrideIndex;

	// Empty base stance.
	BaseStance.AnimName.Length = 0;
	for(OverrideIndex=0; OverrideIndex<OverrideStance.AnimName.Length; OverrideIndex++)
	{
		if( OverrideStance.AnimName[OverrideIndex] != '' )
		{
			BaseStance.AnimName[OverrideIndex] = OverrideStance.AnimName[OverrideIndex];
		}
	}
}

simulated function EndReloadAnimations()
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);

	// Stop hostage reload animations too!
	if( PawnOwner.IsAKidnapper() )
	{
		PawnOwner.InteractionPawn.BS_Stop(BS_PawnWeaponReload, 0.33f);
		PawnOwner.InteractionPawn.BS_Stop(BS_PawnWeaponReloadSuccess, 0.33f);
		PawnOwner.InteractionPawn.BS_Stop(BS_PawnWeaponReloadFail, 0.33f);
	}
}

simulated function PlayActiveReloadSuperSuccess()
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);
	if( PawnOwner.IsAKidnapper() )
	{
		// If we have a player reload success animation, play it
		if( BS_PawnWeaponReloadSuccess.AnimName.Length > 0 )
		{
			// Override normal reload animation with success one
			PawnOwner.InteractionPawn.BS_Override(BS_PawnWeaponReloadSuccess);
		}
	}

	Super.PlayActiveReloadSuperSuccess();
}

simulated function ScaleWeaponAnimPlayRate(float Scale)
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);
	if( PawnOwner.IsAKidnapper() )
	{
		// If we have a player reload success animation, play it
		if( BS_PawnWeaponReloadSuccess.AnimName.Length == 0 )
		{
			// Scale player reload animation
			PawnOwner.InteractionPawn.BS_ScalePlayRate(BS_PawnWeaponReload, Scale);
		}
	}

	Super.ScaleWeaponAnimPlayRate(Scale);
}

simulated function PlayActiveReloadSuccess()
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);
	if( PawnOwner.IsAKidnapper() )
	{
		// If we have a player reload success animation, play it
		if( BS_PawnWeaponReloadSuccess.AnimName.Length > 0 )
		{
			// Override normal reload animation with success one
			PawnOwner.InteractionPawn.BS_Override(BS_PawnWeaponReloadSuccess);
		}
	}

	Super.PlayActiveReloadSuccess();
}

simulated function PlayActiveReloadFailed()
{
	local GearPawn PawnOwner;

	PawnOwner = GearPawn(Instigator);
	if( PawnOwner.IsAKidnapper() )
	{
		// Play active reload failed animation
		if( BS_PawnWeaponReloadFail.AnimName.Length > 0 )
		{
			PawnOwner.InteractionPawn.BS_Override(BS_PawnWeaponReloadFail);
		}
	}
	
	// Override reload animations with shield override if carrying shield
	BS_PawnWeaponReloadFail = default.BS_PawnWeaponReloadFail;
	if( PawnOwner != None )
	{
		if( PawnOwner.CarriedCrate != None )
		{
			OverrideBodyStance(BS_PawnWeaponReloadFail, CrateCarryReloadFailedOverride);
		}
		else if( PawnOwner.IsCarryingShield() )
		{
			OverrideBodyStance(BS_PawnWeaponReloadFail, ShieldReloadFailedOverride);
		}
	}

	Super.PlayActiveReloadFailed();
}

function bool VerifyGrenadeWeapon();
function ForceThrowGrenade( GearAI AI );

simulated function float PlayMeleeAttackAnimation(GearPawn P)
{
	if( P.IsAKidnapper() )
	{
		P.InteractionPawn.BS_Play(BS_MeleeAttackHostage, 1.f, 0.1f, 0.25f, FALSE, TRUE);
		return P.BS_Play(BS_MeleeAttackKidnapper, 1.f, 0.1f, 0.25f, FALSE, TRUE);
	}

	return Super.PlayMeleeAttackAnimation(P);
}

/** 
 * TRUE if reloading weapon takes out of TargetingMode, TRUE for most weapons. 
 * When deploying shield, allow to reload in that position.
 */
simulated event bool ShouldReloadingPreventTargeting()
{
	local GearPawn GPOwner;

	GPOwner = GearPawn(Instigator);
	return bReloadingWeaponPreventsTargeting && GPOwner != None && !GPOwner.IsDeployingShield();
}

/** Give you infinite pistol ammo when carrying crate */
simulated function bool HasInfiniteSpareAmmo()
{
	local GearPawn GPOwner;

	GPOwner = GearPawn(Instigator);
	if(GPOwner != None && GPOwner.CarriedCrate != None)
	{
		return TRUE;
	}
	else
	{
		return Super.HasInfiniteSpareAmmo();
	}
}

defaultproperties
{
	FiringStatesArray(0)="WeaponFiring"

	WeaponType=WT_Holster
	WeaponAnimType=EWAT_Pistol

	// Weapon anim set
	CustomAnimSets(0)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_CamSkel_Pistol'
	CustomAnimSets(1)=AnimSet'COG_MarcusFenix.Animations.AnimSetMarcus_Overlay_Pistol'
	AimOffsetProfileNames(0)="Pistol"

	BS_MeleeAttack.Empty()
	BS_MeleeAttack(0)=(AnimName[BS_Std_Up]="AR_Melee_Smack_A",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_A")
	BS_MeleeAttack(1)=(AnimName[BS_Std_Up]="AR_Melee_Smack_B",AnimName[BS_Std_Idle_Lower]="AR_Melee_Smack_B")

	/** Special overrides for shield*/
	ShieldReloadOverride=(AnimName[BS_Std_Up]="BS_Pistol_Reload",AnimName[BS_Shield_Hunkered]="BS_Deployed_Pistol_Reload")
	ShieldReloadFailedOverride=(AnimName[BS_Std_Up]="BS_Pistol_Reload_Fail",AnimName[BS_Shield_Hunkered]="BS_Deployed_Pistol_Reload_Fail")
	
	/** Special overrides for Box Carry */
	CrateCarryReloadOverride=(AnimName[BS_Std_Up]="Box_Pistol_Reload")
	CrateCarryReloadFailedOverride=(AnimName[BS_Std_Up]="Box_Pistol_Reload_Fail")

	BS_MeleeAttackKidnapper=(AnimName[BS_FullBody]="ADD_Kidnapper_MeleeA")
	BS_MeleeAttackHostage=(AnimName[BS_FullBody]="ADD_Hostage_MeleeA")

	// Weapon Animation
	Begin Object Class=AnimNodeSequence Name=WeaponAnimNode
	    AnimSeqName=""
	End Object
	WeaponFireAnim="HG_Fire"
	WeaponReloadAnim=""
	WeaponReloadAnimFail=""

	ZoomActivatedSound=SoundCue'Interface_Audio.Interface.WeaponZoomIn_Cue'
	ZoomDeActivatedSound=SoundCue'Interface_Audio.Interface.WeaponZoomOut_Cue'

	EquipTime=0.5f
	PutDownTime=0.5f

	bCanDisplayReloadTutorial=TRUE

	bCanEquipWithShield=TRUE

	bAllowTracers=TRUE
}


