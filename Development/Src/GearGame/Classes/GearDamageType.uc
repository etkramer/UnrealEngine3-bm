/**
 * This class is the base class for Gear damage types
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearDamageType extends DamageType
	native
	config(Weapon)
	DependsOn(GUDManager,GearTypes,GearPlayerCamera,GearSoundGroup)
	abstract;

/** FXInfo IDs for phys material based effects */
var const EImpactTypeBallistic ImpactTypeBallisticID;
var const EImpactTypeExplosion ImpactTypeExplosionID;


/** ==== General damage attributes ==== */

/** Amount to scale Damage in the event of a head shot (0.f == no head shots) */
var const config float HeadShotDamageScale;

/** Chance a shot from this damage type will remove a helmet */
var const config float HelmetRemovalChance;

/** If this weapon does head shots, is it capable of knocking the head off entirely for the special death? */
var const config bool bAllowHeadShotGib;

/** If this weapon does head shots, is it allowed to skip DBNO and execute the player immediately? */
var const config bool bAllowHeadShotExecution;

/** Should this damage be applied regardless of cover? */
var const config bool bIgnoreCover;

/** Stopping power of this damage type, MovementPct per damage. */
var const config float StoppingPower;

/** Is this damage type able to kill a victim currently in a DBNO state? */
var const config bool bLethal;

/** TRUE if this damage type come from the 'environment' and not a weapon */
var const bool bEnvironmentalDamage;

/** If TRUE, play a high kick death animation. Suitable for explosives, or high kick weapons such as shotguns */
var const config bool	bHighKickDeathAnimation;

/** if TRUE, everyone will be damaged regardless of team */
var const config bool	bAlwaysDamageFriendlies;

/** How much chainsaw interrupt does this damage type contribute? */
var const config int ChainsawInterruptValue;

/** ==== AI specific damage attributes ==== */

/** Whether or not this damage type should cause stumbles **/
var const config bool bShouldCauseStumbles;
/** The percent of a stumble for this damage type (0 - 1.f) **/
var const config float StumblePercent;
/** the damage value to pretend to do when evaluating a near-miss (to determine whether or not to flee from a near miss) */
var const config int NearMissPretentDamageValue;

/** Chance to play a full body hit reaction animation */
var const config FLOAT	FullBodyHitReactionAnimChance;

/** ==== Physics specific damage attributes ==== */

/** How long the physical hit reaction lasts. */
var config const float HitReactionTimeScale;

/** When this is TRUE, it forces a straight rag doll death, without playing death animations, nor motorized physics. */
var const bool bForceRagdollDeath;

/** When bShouldGib is set, DistFromHitLocToGib creates a cylinder of influence around the HitLocation, along the HitDirection,
 * And any constraints within that cylinder will be broken.	 Default value of -1.f breaks the whole body. */
var config const float DistFromHitLocToGib;


/** ==== Feedback related attributes ==== */

/** Killed by Icon **/
var const CanvasIcon KilledByIcon;
var const CanvasIcon HeadshotIcon;
var const float IconScale;

/** GUD event to throw when someone gets killed by this damage type.  Defaults to GUDEvent_None to fallback to default events. */
var const EGUDEventID KillGUDEvent;

/** GUD event to throw when someone gets hurt by this damage type.	Defaults to GUDEvent_None to fallback to default events. */
var const EGUDEventID DamageGUDEvent;
var const EGUDEventID DamageHeavyGUDEvent;


/** TRUE to do a code-driven camera shake, as described by OnDamageCodeCameraShake. */
var() protected config bool bDoOnDamageCodeCameraShake;
/** Take Damage Camera Shake (Code-driven) */
var() protected config ScreenShakeStruct OnDamageCodeCameraShake;

/** TRUE to do a code-driven camera shake, as described by OnDamageAnimCameraShake. */
var() protected config bool bDoOnDamageAnimCameraShake;
/** Take Damage Camera Shake (Anim-driven) */
var() protected config ScreenShakeAnimStruct OnDamageAnimCameraShake;

/** Scale properties of the screenshake based on the amount of damage taken in relation to the total health of our pawn */
var() protected config float OnDamageCameraShakeScale;

/**
 * This is the Damage Type's suppress impact effects bool
 * Some weapons such as the InkGrenade cloud and flamethrower should not play impact effects.
 **/
var bool bSuppressImpactFX;

/**
 * This is the Damage Type's suppress blood decals
 * Some weapons such as the flamethrower are so expensive that we don't want to have more costs associated with them per frame
 **/
var bool bSuppressBloodDecals;


/**
 * Some explosion particle systems already have the blood and guts inside the Explosion so we do not want to play the additional
 * effect.
 **/
var bool bSuppressPlayExplosiveRadialDamageEffects;

var() const GearVoiceEffortID		DeathScreamEffortID;

/** Use the location for the hit indicator rather than the updated actor location. */
var config bool bUseLocationForIndicator;

/** @STATS -- Holds the tag that associated this damage type with a weapon type */
var EWeaponClass WeaponID;

/** when this is enabled, damage will be scaled proportionately to the player health scalar, so that it will do the same amount of proportionate damage across all difficulty levels */
var config bool bCompensateDamageForDifficulty;

/** Should the Pawn explode into chunky bits when killed with this damage type? */
static function bool ShouldGib(Pawn TestPawn, Pawn Instigator)
{
	return FALSE;
}

/** Should the Pawn play the special head shot death? */
static function bool ShouldHeadShotGib(Pawn TestPawn, Pawn Instigator)
{
	local GearPawn GP;
	// if we are capable
	if (default.bAllowHeadShotGib)
	{
		// return TRUE if the last hit was a head shot
		GP = GearPawn(TestPawn);
		if (GP != None && GP.bLastHitWasHeadShot)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/** Should damage type ignore execution rules and allow ranged kill of DBNO enemy */
static function bool ShouldIgnoreExecutionRules( Pawn TestPawn, Pawn Instigator )
{
	return ShouldHeadShotGib(TestPawn, Instigator) || ShouldGib( TestPawn, Instigator );
}

/** Should the Pawn transition to DBNO when killed with this damage type? */
static function bool ShouldDBNO(Pawn TestPawn, Pawn Instigator, vector HitLocation,TraceHitInfo HitInfo)
{
	local GearPawn GP;
	GP = GearPawn(TestPawn);
	if (GP != None)
	{
		// make sure the pawn support DBNO at all
		if (!GP.bCanDBNO)
		{
			return FALSE;
		}
		// if we don't want to DBNO right now stop checking
		if (!GP.WantsToDBNO(Instigator,HitLocation,HitInfo))
		{
			return FALSE;
		}
		// check for the revive limit in multiplayer
		//`log(GetFuncName()@"MaxDownCount:"@GP.GetMaxDownCount());
		if ( GP.GetMaxDownCount() >= 0 && GP.DownCount >= GP.GetMaxDownCount() )
		{
			return FALSE;
		}
		// make sure we're not expecting a headshot gib instead of DBNO
		if (default.HeadShotDamageScale != 1.f && GP.bLastHitWasHeadShot && default.bAllowHeadShotExecution)
		{
			return FALSE;
		}
	}
	return TRUE;
}

/** Called when a Pawn receives damage of this type, allows special case modifications (default implementation handles head shots) */
static function ModifyDamage(Pawn Victim, Controller InstigatedBy, out int Damage, TraceHitInfo HitInfo, vector HitLocation, vector Momentum)
{
	local GearPawn	GP, InstigatedGP;
	local byte		bHasHelmet;
	local INT		BodyIndex;
	local GearPC	GPC;
	local float		CompensationFactor;

	GP = GearPawn(Victim);
	if (GP != None)
	{
		InstigatedGP = InstigatedBy != None ? GearPawn(InstigatedBy.Pawn) : None;
		bHasHelmet = 0;
		GP.bLastHitWasHeadShot = FALSE;
		if (default.HeadShotDamageScale != 1.f && (GearAI(InstigatedBy) == None || GearAI(InstigatedBy).bCanHeadshot))
		{
			// head shot damage scaling
			if (GP.TookHeadShot(HitInfo.BoneName,HitLocation,Momentum,bHasHelmet))
			{
				// if it has a helmet then negate damage (except if active reload bonus is active)
				if (bHasHelmet == 1 && (InstigatedGP == None || !InstigatedGP.bActiveReloadBonusActive))
				{
					// scale down the headshot damage scale slightly
					Damage *= 1.f + ((default.HeadshotDamageScale - 1.f) * 0.5f);
				}
				else
				{
					// otherwise scale the crap out of it
					Damage *= default.HeadShotDamageScale;
				}
				GP.bLastHitWasHeadShot = TRUE;
			}
		}

		// flappy bit damage ignore
		if( GP.Mesh != None && GP.Mesh.PhysicsAsset != None )
		{
			BodyIndex = GP.Mesh.PhysicsAsset.FindBodyIndex(HitInfo.BoneName);
			if( BodyIndex != INDEX_NONE &&  GP.Mesh.PhysicsAsset.BodySetup[BodyIndex].bAlwaysFullAnimWeight )
			{
				Damage = 0;
			}
		}

		if(default.bCompensateDamageForDifficulty)
		{
			GPC = GearPC(GP.Controller);
			if(GPC != none && !GP.WorldInfo.GRI.IsMultiplayerGame())
			{
				CompensationFactor = GearPRI(GPC.PlayerReplicationInfo).Difficulty.default.PlayerHealthMod / class'DifficultySettings_Hardcore'.default.PlayerHealthMod;
				//`log(GetFuncName()@Victim@Damage@"Compensating damage for difficulty!"@default.class@Damage * CompensationFactor @ GearPRI(GPC.PlayerReplicationInfo).Difficulty.default.PlayerHealthMod@GearPRI(GPC.PlayerReplicationInfo).Difficulty);
				Damage *= CompensationFactor;
			}
		}
	}
}

/** Called when a Pawn is killed with this damage type */
static function HandleKilledPawn(Pawn KilledPawn, Pawn Instigator);

/** Called when a Pawn is knocked DBNO with this damage type */
static function HandleDBNOPawn(Pawn DBNOPawn);

/**
 * Called when a Pawn is hurt with this damage type, after the damage has been applied.  Server side only.
 */
static function HandleDamagedPawn(Pawn DamagedPawn, Pawn Instigator, int DamageAmt, vector Momentum);

/**
 * Called on all clients when a pawn has taken damage, used to kick off damage type specific FX.
 */
static function HandleDamageFX(GearPawn DamagedPawn, const out TakeHitInfo HitInfo)
{
	local GearPC PC;
	local ScreenShakeStruct	CodeShake;
	local ScreenShakeAnimStruct	AnimShake;
	local float	ShakeScale;
	local bool bDoPainFX;

	bDoPainFX = (DamagedPawn.WorldInfo.TimeSeconds - DamagedPawn.LastPainTime > 0.95f);

	PC = DamagedPawn.GetPC();
	if (PC != None && PC.IsLocalPlayerController())
	{
		// Don't send hit locator data if the instigator is too close or null
		if ( HitInfo.InstigatedBy != None && (VSizeSq(HitInfo.InstigatedBy.Location - DamagedPawn.Location) > (DamagedPawn.GetCollisionRadius() * DamagedPawn.GetCollisionRadius())) )
		{
			// add the hud hit indicator
			PC.MyGearHUD.AddNewHitLocatorData(HitInfo.InstigatedBy, HitInfo.Damage, default.bUseLocationForIndicator ? HitInfo.HitLocation : vect(0,0,0));
		}

		// do time-limited pain effects
		if (bDoPainFX)
		{
			// do a lil' camera shake
			// figure out the scale based on percent of total health damaged
			ShakeScale = FClamp((HitInfo.Damage/float(DamagedPawn.DefaultHealth))*default.OnDamageCameraShakeScale, 0.15f, 1.f);

			// apply the camera shake(s)
			if (default.bDoOnDamageCodeCameraShake)
			{
				CodeShake = default.OnDamageCodeCameraShake;
				CodeShake.FOVAmplitude *= ShakeScale;
				CodeShake.FOVFrequency *= ShakeScale;
				CodeShake.RotAmplitude *= ShakeScale;
				PC.ClientPlayCameraShake(CodeShake);
			}
			if (default.bDoOnDamageAnimCameraShake)
			{
				AnimShake = default.OnDamageAnimCameraShake;
				AnimShake.AnimScale *= ShakeScale;
				PC.ClientPlayCameraShakeAnim(AnimShake);
			}

			if( ShouldPlayForceFeedback( DamagedPawn ) )
			{
				// fire off the damage waveform
				PC.ClientPlayForceFeedbackWaveform(default.DamagedFFWaveform);
			}
		}
	}

	if (bDoPainFX)
	{
		// let me hear you say "ungh", nanananana
		if ( (DamagedPawn.Health > 0.f) && (DamagedPawn.WorldInfo.NetMode != NM_DedicatedServer) )
		{
			DamagedPawn.SoundGroup.PlayTakeHit(DamagedPawn, HitInfo.Damage, DamagedPawn.Health/float(DamagedPawn.DefaultHealth) );
		}

		DamagedPawn.LastPainTime = DamagedPawn.WorldInfo.TimeSeconds;
	}
}

static function bool ShouldPlayForceFeedback( Pawn DamagedPawn )
{
	return TRUE;
}

static function bool IsScriptedDamageType()
{
	return FALSE;
}

static function HandleDeadPlayer(GearPC Player);

static function int GetChainsawInterruptValue(int Damage)
{
	return default.ChainsawInterruptValue;
}

defaultproperties
{
	// physics
	KDamageImpulse=100

	bSuppressImpactFX=FALSE
	bSuppressPlayExplosiveRadialDamageEffects=FALSE

	// feedback
	DamageGUDEvent=GUDEvent_None
	DamageHeavyGUDEvent=GUDEvent_None
	KillGUDEvent=GUDEvent_KilledEnemyGeneric
	DeathScreamEffortID=GearEffort_DeathScream

	// Short "pop" of damage
	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveform0
		Samples(0)=(LeftAmplitude=64,RightAmplitude=96,LeftFunction=WF_LinearDecreasing,RightFunction=WF_LinearDecreasing,Duration=0.25)
	End Object
	DamagedFFWaveform=ForceFeedbackWaveform0

	// Pretty violent rumble
	Begin Object Class=ForceFeedbackWaveform Name=ForceFeedbackWaveform1
		Samples(0)=(LeftAmplitude=100,RightAmplitude=100,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.75)
	End Object
	KilledFFWaveform=ForceFeedbackWaveform1
	KilledByIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=0,V=120,UL=77,VL=29)
	HeadshotIcon=(Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=78,V=30,UL=77,VL=29)
	IconScale=1.f

	// This is a hack.  The Stats system uses weapon id to determine what weapon did damage.  We default to an impossible id and reject anything
	// we see with this.

	WeaponID=WC_WretchMeleeSlash

}
