/**
 * Heavy Gatling Gun Weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_HeavyMiniGunBase extends GearWeap_HeavyBase
	abstract;


// test code
struct immutable Range
{
	var() float Min, Max;
};
struct immutable IntRange
{
	var() int Min, Max;
};

const FIREMODE_SPINUP = 4;
const FIREMODE_SPINDOWN = 5;

/** Sound to play when ammo feed starts */
var protected const SoundCue BarrelSpinningStartCue;
/** AudioComponent for the AmmoFeedStart sound. */
var protected AudioComponent BarrelSpinningStartAC;
/** Looping SoundCue for ammo feeder. */
var protected const SoundCue BarrelSpinningLoopCue;
/** The AudioComponent that is used for the looping ammo loading sound **/
var protected AudioComponent BarrelSpinningLoopAC;
/** Sound to play when ammo feed stops */
var protected const SoundCue BarrelSpinningStopCue;


/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue;
/** Looping SoundCue for firing. */
var protected const SoundCue FireLoopCue_Player;
/** The AudioComponent that is used for the looping shot sound **/
var protected AudioComponent FireLoopAC;

var protected const SoundCue HeatBuildupCue;
var protected AudioComponent HeatBuildUpAC;

var protected const SoundCue CasingImpactCue;


/** Sound to play when firing stops */
var protected const SoundCue FireStopCue;
/** Sound to play when firing stops */
var protected const SoundCue FireStopCue_Player;
/** Sound to play when firing stops due to overheating */
var protected const SoundCue FireStopCue_Overheat;


/** How much to scale the default camera shake when mounted. */
var() const protected float		FireCameraShakeScaleMounted;
/** Range of how much to scale the default firing camera shake.  Mapped to normalized accuracy values. */
var() const protected vector2d	FireCameraShakeScaleRange;

var() protected const config float	SpinUpTime;
var() protected const config float	SpinDownTime;
var() protected const config float	SpinDownTime_ActiveCooling;
var protected transient bool		bSpinningUp;

/** Roll Angle of barrel */
var transient protected float	BarrelRollAngle;
var transient protected float	CrankRotAngle;
var transient protected float	CurrentBarrelRotAccel;

/** How fast the barrel is turning, in rotator units per second. */
var() protected transient float CurrentBarrelRotRate;
/** How fast the barrel wants to be turning. */
var protected transient float GoalBarrelRotRate;

/** Above this rotation speed, the weapon can fire. */
var() protected const float BarrelRotRateFiringThreshold;
var() protected const float MaxBarrelRotRate;

/** Min = fire rate at BarrelRotRateFiringThreshold.  Max = fire rate at MaxBarrelRotRate. */
var() protected const config Range WeaponRateOfFireRange;

/** Barrel Bone Controller. */
var SkelControlSingleBone	BarrelSkelCtrl;

/** Smoke emitter at the muzzle */
var ParticleSystemComponent		PSC_BarrelSmoke;
/** Tracks how long this weapon has been firing, in seconds. */
var protected transient float	FiringDuration;
/** Length of time the weapon needs to fire before we can turn on the barrel smoke emiiter. */
var() protected const float		BarrelSmokeMinFiringDuration;


/** Longest sustained firing burst before overheating, in seconds. */
var() protected const config float	MaxFiringTimeBeforeOverheat;

/** When gun overheats, how long you must wait until the gun begins to cool down again. */
var() protected const config float	OverheatPenaltyTime;

var protected transient bool		bOverheated;

/** How long it takes the barrel to cool down from fully heated, in seconds. */
var() protected const config float	CooldownTime;
/** How long it takes the barrel to cool down from fully heated while ActiveCooling(tm) is in effect, in seconds. */
var() protected const config float	CooldownTimeActiveCooling;

var protected transient bool bActivelyCooling;
var protected const SoundCue ActiveCoolingLoopCue;
var protected const SoundCue ActiveCoolingStartCue;
var protected const SoundCue ActiveCoolingStopCue;
var protected transient AudioComponent	ActiveCoolingSteamAudioLoop;

var protected ParticleSystemComponent	PSC_ActiveCoolingSteam;

var() protected const vector2d		BarrelHeatMaterialParamRange_Heat;
var() protected const vector2d		BarrelHeatMaterialParamRange_Brightness;

var() protected const vector2d		FireLoopPitchRange;
var() protected const vector2d		BarrelLoopPitchRange;

replication
{
	if (Role == ROLE_Authority && bNetOwner)
		bOverheated;
}

// test code
simulated static final function float LerpRange(Range R, float Alpha)
{
	return R.Min + ( (R.Max - R.Min) * Alpha );
}
simulated static final function float GetAlphaFromRange(Range R, float Val)
{
	return (Val - R.Min) / (R.Max - R.Min);
}
simulated static final function float RandInRange(Range R)
{
	return R.Min + (R.Max - R.Min) * FRand();
}

////////////////////////////////////
// AR bonus stuff
/////////////////////////////////////

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	SkeletalMeshComponent(Mesh).AttachComponentToSocket(PSC_BarrelSmoke, MuzzleSocketName);
	SkeletalMeshComponent(Mesh).AttachComponentToSocket(PSC_ActiveCoolingSteam, 'Steam');
}

simulated function PerformWeaponAttachment(SkeletalMeshComponent MeshCpnt, optional Name SocketName)
{
	Super.PerformWeaponAttachment(MeshCpnt, SocketName);

	// Cache AnimNodes
	BarrelSkelCtrl = SkelControlSingleBone(SkeletalMeshComponent(Mesh).FindSkelControl('BarrelRotation'));
}

simulated protected function StartSpinUpEffects()
{
	local GearPawn			InstigatorGP;
	local AnimNodeSequence	CrankStartAnimNodeSeq;

	InstigatorGP = GearPawn(Instigator);

	// play the start-firing audio
	BarrelSpinningStartAC = GearWeaponPlaySoundLocalEx(BarrelSpinningStartCue,, BarrelSpinningStartAC);

	// start spinning up the actual barrel.  get to firing rate within spinuptime
	GoalBarrelRotRate = MaxBarrelRotRate;
	CurrentBarrelRotAccel = BarrelRotRateFiringThreshold / SpinUpTime;

	// If barrel is not spinning much yet, play an "effort" animation on the character.
	// To make him look like he's putting some weight on the crank.
	if( CurrentBarrelRotRate < BarrelRotRateFiringThreshold * 0.5f )
	{
		if( InstigatorGP != None )
		{
			CrankStartAnimNodeSeq = InstigatorGP.AnimTreeRootNode.GetGroupSynchMaster('GatlingCrankStart');
			if( CrankStartAnimNodeSeq != None && !CrankStartAnimNodeSeq.bPlaying )
			{
				CrankStartAnimNodeSeq.PlayAnim(FALSE, 1.f, 0.f);
			}
		}
	}

	bSpinningUp = TRUE;
}

simulated protected function StartSpinDownEffects()
{
	//	`log(self@GetFuncName(),bDebug);
	// play spindown sounds
	if (CurrentBarrelRotRate > BarrelRotRateFiringThreshold)
	{
		GearWeaponPlaySoundLocal(BarrelSpinningStopCue);
		if (!bOverheated)
		{
			GearWeaponPlaySoundLocal(FireStopCue, FireStopCue_Player,, 1.f);
		}
		GearWeaponPlaySoundLocal(CasingImpactCue);
	}

	// stop any firing or spinning up sounds
	if (BarrelSpinningLoopAC != None)
	{
		BarrelSpinningLoopAC.FadeOut(0.2f, 0.0f);
	}
	if (FireLoopAC != None)
	{
		FireLoopAC.FadeOut(0.2f, 0.0f);
	}
	if (HeatBuildUpAC != None)
	{
		HeatBuildUpAC.FadeOut(0.2f, 0.0f);
	}
	if (BarrelSpinningStartAC != None)
	{
		BarrelSpinningStartAC.FadeOut(0.2f, 0.0f);
	}

	// start spinning down the actual barrel.  get back to zero within spindowntime
	GoalBarrelRotRate = 0.f;
	CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime;

	// maybe turn on barrel smoke
	if (FiringDuration > BarrelSmokeMinFiringDuration)
	{
		PSC_BarrelSmoke.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(PSC_BarrelSmoke.Template, GetMuzzleLoc()));
		PSC_BarrelSmoke.ActivateSystem(TRUE);
	}
}

simulated protected function StartFiringEffects()
{
	// begin firing audio
	BarrelSpinningLoopAC = GearWeaponPlaySoundLocalEx(BarrelSpinningLoopCue,, BarrelSpinningLoopAC, 0.f);
	FireLoopAC = GearWeaponPlaySoundLocalEx(FireLoopCue, FireLoopCue_Player, FireLoopAC);
	HeatBuildUpAC = GearWeaponPlaySoundLocalEx(HeatBuildupCue,, HeatBuildUpAC);

	// make sure barrel is still spinning
	GoalBarrelRotRate = MaxBarrelRotRate;
	CurrentBarrelRotAccel = BarrelRotRateFiringThreshold / SpinUpTime;

	if (!PSC_BarrelSmoke.bSuppressSpawning)
	{
		PSC_BarrelSmoke.DeactivateSystem();
	}
}

simulated protected function OverheatPenaltyExpired()
{
	bOverheated = FALSE;
}


simulated state WeaponFiring
{
	simulated event BeginState( Name PrevStateName )
	{
		//`log(WorldInfo.TimeSeconds@GetFuncName()@GetStateName()@`showvar(PrevStateName));
		Super.BeginState(PrevStateName);
	}

	simulated event EndState( Name NextStateName )
	{
		//`log(WorldInfo.TimeSeconds@GetFuncName()@GetStateName()@`showvar(NextStateName));
		Super.EndState(NextStateName);

		// When leaving state, make sure to transition to spining down, to stop the effects.
		SetCurrentFireMode(FIREMODE_SPINDOWN);
	}
};

simulated state Inactive
{
	simulated protected function SpinUpFinished(){};

	simulated function BeginState(Name PrevStateName)
	{
		//`log(WorldInfo.TimeSeconds@GetFuncName()@GetStateName()@`showvar(PrevStateName)@`showvar(FireLoopAC)@`showvar(FireLoopAC.IsPlaying())@`showvar(BarrelSpinningLoopAC)@`showvar(BarrelSpinningLoopAC.IsPlaying()));
		if( ( (FireLoopAC != None) && (FireLoopAC.IsPlaying()) )
			|| ( (BarrelSpinningLoopAC != None) && (BarrelSpinningLoopAC.IsPlaying()) )
			)
		{
			StartSpinDownEffects();
		}
		CurrentBarrelRotRate=0;
		CurrentBarrelRotAccel=0;
		bSpinningUp=false;
		super.BeginState(PrevStateName);
	}
};

/** Do a little clean up.  Audio Components to none!.  **/
simulated event Destroyed()
{
	if( BarrelSpinningLoopAC != none )
	{
		BarrelSpinningLoopAC.FadeOut( 0.1f, 0.0f );
		BarrelSpinningLoopAC = None;
	}

	if( FireLoopAC != None )
	{
		FireLoopAC.FadeOut( 0.1f, 0.0f );
		FireLoopAC = None;
	}

	if (HeatBuildUpAC != None)
	{
		HeatBuildUpAC.FadeOut(0.1f, 0.0f);
		HeatBuildUpAC = None;
	}

	if( BarrelSpinningStartAC != none )
	{
		BarrelSpinningStartAC.FadeOut( 0.1f, 0.0f );
		BarrelSpinningStartAC = None;
	}

	Super.Destroyed();
}

/** Overloaded to scale FireCameraShakes. */
simulated function PlayFireCameraEffects()
{
	local float NormAcc;

	// set the shake scale value.  do this before calling the super.
	NormAcc = GearInventoryManager(InvManager).GetPlayerNormalizedAccuracy();

	if (IsMounted())
	{
		FireCameraShakeScale = FireCameraShakeScaleMounted;
	}
	else
	{
		// use normal accuracy values
		FireCameraShakeScale = Lerp(FireCameraShakeScaleRange.X, FireCameraShakeScaleRange.Y, NormAcc);
	}

	super.PlayFireCameraEffects();
}

/** Plays looping audio effects for weapon firing */
simulated protected function SpinUpFinished()
{
	bSpinningUp = FALSE;
	StartFiringEffects();

	SetCurrentFireMode(0);
	GotoState('WeaponFiring');
}

simulated state SpinningUp
{
	simulated function BeginState(Name PreviousStateName)
	{
		//`log(WorldInfo.TimeSeconds@GetFuncName()@GetStateName()@`showvar(PreviousStateName));
		if (CurrentBarrelRotRate < BarrelRotRateFiringThreshold)
		{
			SetCurrentFireMode(FIREMODE_SPINUP);
		}
		else
		{
			// if already spinning transition to firing immediately
			SpinUpFinished();
		}

		super.BeginState(PreviousStateName);
	}

	simulated function EndFire(byte FireModeNum)
	{
		Global.EndFire(FireModeNum);

		// If fire is released, go back to active state
		//`log("going back to active");
		//ScriptTrace();
		GotoState('Active');
		SetCurrentFireMode(FIREMODE_SPINDOWN);
	}

};

simulated function FireModeUpdated(byte FiringMode, bool bViaReplication)
{
	if (FiringMode == FIREMODE_SPINUP)
	{
		EndActiveCooling();
		StartSpinUpEffects();
	}
	else if (FiringMode != 0)
	{
		StartSpinDownEffects();
	}

	super.FireModeUpdated(FiringMode, bViaReplication);
}

simulated function Tick(float DeltaTime)
{
	local float		T;
	local int		NormalizedCrankAngle;
	local float		RelativeAnimPosition;
	local GearPawn	MyGearPawn;

	Super.Tick(DeltaTime);

	if (CurrentBarrelRotRate < GoalBarrelRotRate)
	{
		CurrentBarrelRotRate += CurrentBarrelRotAccel * DeltaTime;
		CurrentBarrelRotRate = FClamp(CurrentBarrelRotRate, 0.f, GoalBarrelRotRate);

		// spinning up, see if we're finished
		if ( bSpinningUp && (CurrentBarrelRotRate >= BarrelRotRateFiringThreshold) )
		{
			SpinUpFinished();
		}
	}
	else if (CurrentBarrelRotRate > GoalBarrelRotRate)
	{
		CurrentBarrelRotRate -= CurrentBarrelRotAccel * DeltaTime;
		CurrentBarrelRotRate = FMax(CurrentBarrelRotRate, GoalBarrelRotRate);
	}

	// keep track of firing duration and barrel heat
	if ( IsFiring() && (CurrentBarrelRotRate > BarrelRotRateFiringThreshold) )
	{
		FiringDuration += DeltaTime;
		CurrentBarrelHeat += DeltaTime / MaxFiringTimeBeforeOverheat;
		CurrentBarrelHeat = FMin(CurrentBarrelHeat, 1.f);

		if (Instigator.IsLocallyControlled())
		{
			if ( (CurrentBarrelHeat >= 1.f) && !bOverheated )
			{
				// overheated!  stop firing and don't allow a restart for some time
				Overheated();
			}
		}
	}
	else
	{
		FiringDuration = 0.f;
		if (bActivelyCooling)
		{
			CurrentBarrelHeat -= DeltaTime / CooldownTimeActiveCooling;

			if (Instigator.IsLocallyControlled())
			{
				if (CurrentBarrelHeat <= 0.f)
				{
					EndActiveCooling();
				}
			}
		}
		else
		{
			CurrentBarrelHeat -= DeltaTime / CooldownTime;
		}
		CurrentBarrelHeat = FMax(CurrentBarrelHeat, 0.f);
	}


	// pitch shifting for audio loops?
	T = (CurrentBarrelRotRate - BarrelRotRateFiringThreshold) / (MaxBarrelRotRate - BarrelRotRateFiringThreshold);
	T = FClamp(T, 0.f, 1.f);
	if (FireLoopAC != None)
	{
		FireLoopAC.PitchMultiplier = GetRangeValueByPct(FireLoopPitchRange, T);
	}
	if (BarrelSpinningLoopAC != None)
	{
		BarrelSpinningLoopAC.PitchMultiplier = GetRangeValueByPct(BarrelLoopPitchRange, T);
	}

	// Calculate current normalized barrel roll angle
	BarrelRollAngle += CurrentBarrelRotRate * DeltaTime;

	// Barrel roll
	if( BarrelSkelCtrl != None )
	{
		BarrelSkelCtrl.BoneRotation.Roll = BarrelRollAngle;
	}

	CrankRotAngle += (CurrentBarrelRotRate * DeltaTime) / 3.f;
	NormalizedCrankAngle = NormalizeRotAxis(CrankRotAngle);

	// Turn that into a relative time to be used by a looping animation.
	RelativeAnimPosition = float(NormalizedCrankAngle < 0 ? NormalizedCrankAngle + 65536 : NormalizedCrankAngle) / 65536.f;

	// Weapon crank animation
	if( AnimTreeRootNode != None )
	{
		AnimTreeRootNode.ForceGroupRelativePosition('GatlingCrank', RelativeAnimPosition);
	}

	// Pawn crank animation
	MyGearPawn = GearPawn(Instigator);
	if( MyGearPawn != None && MyGearPawn.AnimTreeRootNode != None )
	{
		MyGearPawn.AnimTreeRootNode.ForceGroupRelativePosition('GatlingCrank', RelativeAnimPosition);
	}
}


/** Overloaded to allow for variable fire rate, depending on barrel rot speed. */
simulated function float GetRateOfFire()
{
	local float T, ROF;
	T = (CurrentBarrelRotRate - BarrelRotRateFiringThreshold) / (MaxBarrelRotRate - BarrelRotRateFiringThreshold);
	ROF = LerpRange(WeaponRateOfFireRange, T);

	if( (Instigator != None) && !Instigator.IsHumanControlled() )
	{
		ROF *= AI_RateOfFire_Scale;
	}

	return ROF;
}

simulated function ConsumeAmmo(byte FireModeNum)
{
	local int SpareAmmoUsed;

	Super.ConsumeAmmo(FireModeNum);

	// since we don't have a normal clip, draw directly from spare ammo until it is used up
	if (bInfiniteSpareAmmo)
	{
		AmmoUsedCount = 0;
	}
	else if (SpareAmmoCount > 0)
	{
		SpareAmmoUsed = Min(SpareAmmoCount, AmmoUsedCount);
		AmmoUsedCount -= SpareAmmoUsed;
		SpareAmmoCount -= SpareAmmoUsed;
	}
}

simulated function bool IsCriticalAmmoCount()
{
	return (SpareAmmoCount + GetMagazineSize() - AmmoUsedCount < 100);
}

/** Overridden to allow 'WeaponFiring' to be a valid fire state since it's not in the FirignStatesArray for this class **/
protected function bool IsFireState( name StateName )
{
	return (StateName == 'WeaponFiring' || Super.IsFireState(StateName));
}

/** Overridden to return TRUE. */
simulated function bool ShouldUseMuzzleFlashSocketForShellEjectAttach()
{
	return TRUE;
}

/** Overridden to disallow firing while overheated. */
simulated function BeginFire(byte FireModeNum)
{
	if (!bOverheated)
	{
		Super.BeginFire(FireModeNum);
	}
}

reliable server function ServerToggleActiveCooling(bool bActive)
{
	if (bActive != bActivelyCooling)
	{
		if (!bActive)
		{
			EndActiveCooling();
		}
		else
		{
			BeginActiveCooling();
		}
	}
}

simulated function Overheated()
{
	bOverheated = TRUE;
	WeaponPlaySound(FireStopCue_Overheat);
	StopFire(0);
	if (Role == ROLE_Authority)
	{
		SetTimer( OverheatPenaltyTime,, nameof(OverheatPenaltyExpired) );
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GatlingOverheated, Instigator, None, 0.35f);
	}
	else
	if (Role < ROLE_Authority)
	{
		ServerOverheated();
	}
}

reliable server function ServerOverheated()
{
	Overheated();
}

simulated protected function BeginActiveCooling()
{
	bActivelyCooling = TRUE;
	ServerToggleActiveCooling(TRUE);

	// adjust to the AC spindown rate
	CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime_ActiveCooling;

	ActiveCoolingSteamAudioLoop = GearWeaponPlaySoundLocalEx(ActiveCoolingLoopCue,, ActiveCoolingSteamAudioLoop, 0.3f);
	WeaponPlaySound(ActiveCoolingStartCue);

	PSC_ActiveCoolingSteam.ActivateSystem(TRUE);
}
simulated protected function EndActiveCooling()
{
	if (bActivelyCooling)
	{
		// turn off hiss sound
		if (ActiveCoolingSteamAudioLoop != None)
		{
			ActiveCoolingSteamAudioLoop.FadeOut(0.3f, 0.f);
			ActiveCoolingSteamAudioLoop = None;

			WeaponPlaySound(ActiveCoolingStopCue);
		}

		// turn off effects
		PSC_ActiveCoolingSteam.DeactivateSystem();

		// adjust to the normal spindown rate
		CurrentBarrelRotAccel = CurrentBarrelRotRate / SpinDownTime;

		// done
		bActivelyCooling = FALSE;
		ServerToggleActiveCooling(FALSE);
	}
}

simulated state Active
{
	simulated function Tick(float DeltaTime)
	{
		local GearPC GPC;
		local GearPlayerInput GPI;
		local bool bR1Pressed;

		// poll input directly, for more robustness with with respect to when the
		// R1 button goes up/down

		GPC = (Instigator != None) ? GearPC(Instigator.Controller) : None;
		GPI = (GPC != None) ? GearPlayerInput(GPC.PlayerInput) : None;

		if (GPI != None)
		{
			bR1Pressed = GPI.IsButtonActive(GB_RightBumper);
			if (bActivelyCooling && !bR1Pressed)
			{
				EndActiveCooling();
			}
			else if ( !bActivelyCooling && bR1Pressed && (CurrentBarrelHeat > 0.f) )
			{
				BeginActiveCooling();
			}
		}

		Global.Tick(DeltaTime);
	}

	simulated function EndState(Name NextStateName)
	{
		//`log(WorldInfo.TimeSeconds@GetFuncName()@GetStateName()@`showvar(NextStateName));
		Super.EndState(NextStateName);
		// make sure active cooling is disabled
		EndActiveCooling();
	}
};

simulated function bool IsHeatLimited()
{
	return TRUE;
}

/** Returns TRUE if gun is being mounted. Overridden from GearWeap_HeavyBase. */
simulated function bool IsBeingMounted()
{
	local GearPawn	MyGearPawn;

	MyGearPawn = GearPawn(Instigator);
	return (MyGearPawn != None) && !bIsMounted && MyGearPawn.IsDoingSpecialMove(SM_TargetMinigun);
}

/** Returns TRUE if gun is being unmounted. Overridden from GearWeap_HeavyBase. */
simulated function bool IsBeingUnMounted()
{
	local GearPawn	MyGearPawn;

	MyGearPawn = GearPawn(Instigator);
	return (MyGearPawn != None) && MyGearPawn.IsDoingSpecialMove(SM_UnMountMinigun);
}

/** Returns the number of ammo icons this weapon will display in a clip */
simulated function float GetAmmoIconCount()
{
	return 0;
}

simulated protected function UpdateBarrelHeatMaterial()
{
	if (MIC_WeaponSkin != None)
	{
		MIC_WeaponSkin.SetScalarParameterValue('GattlingHeat', GetRangeValueByPct(BarrelHeatMaterialParamRange_Heat, CurrentBarrelHeat));
		MIC_WeaponSkin.SetScalarParameterValue('Brightness', (FiringDuration > 0.f) ? BarrelHeatMaterialParamRange_Brightness.Y : BarrelHeatMaterialParamRange_Brightness.X);
	}
}

simulated function bool CanPickupAmmo()
{
	return AmmoUsedCount > 0;
}

function int AddAmmo( int Amount )
{
	local int OldAmmoUsedCount;
	OldAmmoUsedCount = AmmoUsedCount;
	AmmoUsedCount = Max(AmmoUsedCount - Amount, 0);
	AmmoUpdated();
	return OldAmmoUsedCount - AmmoUsedCount;
}

simulated function int GetDefaultAmmoAmount()
{
	return 100;
}

function DropFrom(vector StartLocation, vector StartVelocity)
{
	// put "dropped" ammo in the single clip the minigun has
	if (GearAI(Instigator.Controller) != None && GearAI_TDM(Instigator.Controller) == None)
	{
		AmmoUsedCount = Max(0, WeaponMagSize - GetDropFromAIAmmoCount());

		// prevent the default code from adding extra "spare" ammo
		AmmoFromDrop.X = 0;
		AmmoFromDrop.Y = 0;
	}

	Super.DropFrom(StartLocation, StartVelocity);
}

defaultproperties
{
	bSupportsBarrelHeat=TRUE

	// no reloading in the new "overheat" weapon concept
	bWeaponCanBeReloaded=FALSE

	// reloading weapon doesn't break targeting mode.
	bReloadingWeaponPreventsTargeting=FALSE
	bNoAnimDelayFiring=TRUE

	FiringStatesArray(0)="SpinningUp"

	AmmoTypeClass=class'GearAmmoType_AssaultRifle'

	InstantHitDamageTypes(0)=class'GDT_Mulcher'

	FireOffset=(X=0,Y=0,Z=0)

	// Weapon Mesh
	Begin Object Name=WeaponMesh
	End Object
	Mesh=WeaponMesh
	DroppedPickupMesh=WeaponMesh

	// Muzzle Flash point light
    Begin Object Name=WeaponMuzzleFlashLightComp
		LightColor=(R=255,G=192,B=128,A=255)
    End Object
    MuzzleFlashLight=WeaponMuzzleFlashLightComp
	MuzzleLightDuration=0.05f
	MuzzleLightPulseFreq=60
	MuzzleLightPulseExp=1.5
	bLoopMuzzleFlashLight=TRUE

	// MF flash
	MuzFlashEmitter=PSC_WeaponMuzzleMuzFlashComp

	PSC_ShellEject=PSC_WeaponShellCaseComp

	Begin Object Class=ParticleSystemComponent Name=PSC_BarrelSmoke0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_BarrelSmoke=PSC_BarrelSmoke0

	// tracers
	bAllowTracers=TRUE
	TracerType=WTT_MinigunFastFiring

	bKillDuringLevelTransition=TRUE

	// Weapon Animation
	WeaponFireAnim="AR_Fire"
	WeaponReloadAnim="AR_Reload"
	WeaponReloadAnimFail="AR_Reload_Fumble"

	bInstantHit=TRUE
	bHose=TRUE

	RotAudioStartVelThreshold=1500.f
	RotAudioStopVelThreshold=100.f
	RotAudioVolumeVelRange=(X=5000.f,Y=15000.f)
	RotAudioVolumeRange=(X=0.1f,Y=1.f)

	NoAmmoFireSoundDelay=0.25f

	MinTimeBetweenBulletWhips=0.5f
	BulletWhipChance=0.75f

	FireCameraShakeScaleRange=(X=1.f,Y=2.f)
	FireCameraShakeScaleMounted=0.5f

	WeaponID=WC_Minigun

	BarrelRotRateFiringThreshold=100000
	MaxBarrelRotRate=300000

	BarrelSmokeMinFiringDuration=2.f

	Begin Object Class=ParticleSystemComponent Name=ActiveCoolingSteam0
		bAutoActivate=FALSE
		TickGroup=TG_PostUpdateWork
	End Object
	PSC_ActiveCoolingSteam=ActiveCoolingSteam0

	BarrelHeatMaterialParamRange_Heat=(X=0.f,Y=-0.65)
	BarrelHeatMaterialParamRange_Brightness=(X=0.15f,Y=0.65)

	LC_EmisDefaultCOG=(R=0.127,G=0.517,B=1.0,A=2.0)
	LC_EmisDefaultLocust=(R=3.0,G=0.517.0,B=0.127,A=2.0)

	FireLoopPitchRange=(X=1.f,Y=1.1f)
	BarrelLoopPitchRange=(X=1.f,Y=1.3f)

	Begin Object Name=ForceFeedbackWaveformShooting0
		Samples(0)=(LeftAmplitude=60,RightAmplitude=60,LeftFunction=WF_Constant,RightFunction=WF_Constant,Duration=0.300)
	End Object
}
