/**
 * Flamethrower spray!
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class FlameThrowerSprayBase extends Actor
	abstract
	config(Weapon)
	native(Weapon);

/** How long this flame has been alive, in seconds. */
var transient float				CurrentAge;
var protected transient bool	bSkeletonHasBeenUpdated;
//
// Effects-related vars
//

/** The skeletal mesh that represents the core of the spray effect */
var() const protected SkeletalMeshComponent SkeletalSprayMesh;

/** Ref to Prune skelcontrol, used for collapsing the mesh on collisions. */
var protected transient GearSkelCtrl_Prune	PruneSkelControl;
/** Names for the shoulder skelcontrol nodes for each leg (from the animtree) */
var protected const Name					PruneSkelControlName;

/** Ref to Prune skelcontrol, used for collapsing the mesh on collisions. */
var protected transient GearSkelCtrl_FlamethrowerScaling	ScalingSkelControl;


/** Scales the gravity to apply to this object. X, Y define ramp-in range. */
var() const protected vector2d				GravityScaleRange;
/** How long it takes the gravity to fully ramp in. */
var() const protected float					GravityScaleInTime;

/** Per-bone info for the flame bone chain. */
struct native FlameBoneInfo
{
	var() const name	BoneName;
	var int				BoneIndex;				// transient, filled in at startup

	// per-bone effects info
	var() const float EffectScale;

	var() ParticleSystemComponent BonePSC0;
	var() ParticleSystemComponent BonePSC1;

	/** This denotes the location of this bone along the seed stream.  Filled in by the skel control when it aligns the bones to the seeds. */
	var float SeedChainLoc;
	var vector LastLoc;			// most recently known location

	var() float ParticleActivationDelay;
};
/** Defines the main bone chain.  Should be sorted front to back. */
var() protected array<FlameBoneInfo>		BoneChain;

/** True to spawn the per-bone FX, false to skip it. */
var() private const bool					bDoPerBoneFireFX;

/** Easy parameter for scaling all per-bone fx. */
var() private const float					PerBoneFireFXGlobalScale;

/** X = delay for first bone, Y = delay for last bone, delay for intermediate bones is lerped */
var() protected const vector2d				PerBoneFireFXActivationDelayRange;
var protected transient float				PerBoneFireFXActivationDelayTimer;
var() protected const float					PerBoneFireFXActivationDist;

/** Splashes and impact FX */
var protected const ParticleSystem				SplashGlancingEffect;
var protected const ParticleSystem				SplashDirectEffect;
var protected const ParticleSystem				SplashPawnEffect;

var protected transient ParticleSystemComponent	CurrentSplashEffect;
var protected ParticleSystemComponent	SplashGlancingPSC;
var protected ParticleSystemComponent	SplashDirectPSC;
var protected ParticleSystemComponent	SplashPawnPSC;
var protected ParticleSystemComponent	SplashMaterialBasedPSC;

/** Looping audio for the various splashes. */
var protected const AudioComponent			SplashPawnAC;
var protected const AudioComponent			SplashDirectAC;
var protected const AudioComponent			SplashGlancingAC;
var protected const AudioComponent			SplashMaterialBasedAC;
var protected transient AudioComponent		CurrentSplashAC;

var() protected const float					SplashGlancingDotLimit;
var() protected const int					LastBoneChainIndexThatCanSpawnSplashEffects;

/** Vars for splash orientation smoothing */
var() protected const float					SplashRotInterpSpeed;
var() protected const float					SplashLocInterpSpeed;
var transient protected rotator				LastSplashRot;
var transient protected vector				LastSplashLoc;

/** Glow effect only visible by the firing player. */
var protected ParticleSystemComponent		PSC_OwnerGlow;
var() protected const int					OwnerGlowBoneChainIndex;
var() protected const float					OwnerGlowScaleInTime;

struct native FlameSprayLight
{
	var PointLightComponent Light;
	var int					BoneChainIndex;
	var float				FlickerIntensity;
	var float				FlickerInterpSpeed;
	var float				LastLightBrightness;
};
var() protected array<FlameSprayLight>		FlameLights;


/**
 * Materials (index 1 and 2) in the mesh that have a parameter we want to set.
 * A parameter named 'Heat' will ramp from MaterialHeatRange.X to MaterialHeatRange.Y
 * over MaterailHeatRampTime seconds.
 */
var protected MaterialInstanceConstant			MIC_FlameMat0;
var protected MaterialInstanceConstant			MIC_FlameMat1;
var protected MaterialInstanceConstant			MIC_FlameMat2;
var() protected const vector2d					MaterialHeatRange;
var() protected const float						MaterialHeatRampTime;

var() protected const float						MatFadePow;

var() protected const float						MaterialFadeOutTime;
var protected transient float					MaterialCurrentFadeVal;

/** This emitter is spawned when firing is stopped, and is attached to the front bone. */
//var protected ParticleSystemComponent			EndFirePSC;
var protected ParticleSystemComponent			StartFirePSC;


//
// Damage & Gameplay vars
//

/** Damage rates. X = damage closest to nozzle, Y = damage at far end of flame. */
var() protected const config vector2d		FlameDamagePerSec;
/** Damage rates AR flame. X = damage closest to nozzle, Y = damage at far end of flame. */
var() protected const config vector2d		FlameDamagePerSecAR;
/** Range over which to scale the damage (see FlameDamagePerSec vars). */
var() const vector2d						FlameDamageScaleDistRange;

/** Damage type to deliver. */
var() protected const class<GearDamageType>	MyDamageType;

var() protected const config float			SplashDamageRadius;
var() protected const config float			SplashDamagePerSecond;
var() protected const config float			SplashDamagePerSecondAR;
var() protected const config float			SplashDamageFalloffExponent;


/** Scales the momentum for damage taken. */
var() protected const float					MomentumScale;

//
// Audio vars
//

/** Sound for starting to fire. */
var() const protected	SoundCue			FireSprayStartSound;
/** Sound for stopping firing. */
var() const protected	SoundCue			FireSprayStopSound;
/** Sample for looping sound played while firing. */
var() const protected	SoundCue			FireSprayLoopSound;
/** AudioComponent for looping sound played while firing. */
var transient protected	AudioComponent		AC_FireSprayLoop;

/** Range of rotational velocities that gets directly mapped to the pitch/volume ranges below. */
var() const protected vector2d				FireAudioAdj_RotVelRange;
var() const protected vector2d				FireAudioAdj_PitchRange;
var() const protected vector2d				FireAudioAdj_VolumeRange;

var() const protected float					SprayAudioAdjInterpSpeed;
var transient protected float				CurrentSprayPitchMult;
var transient protected float				CurrentSprayVolumeMult;

/** Rotation last tick.  Used to modulate pitch on looping audio. */
var private transient Rotator				LastRotation;
/** How fast the spray is rotating */
var private transient float					RotationSpeed;

/** True if the splash effect is playing, false otherwise. */
var private transient bool					bSplashActive;


struct native FlameMeshContact
{
	/** ChainIndex of the bone that made contact. */
	var int					BoneChainIndex;
	var Actor				Actor;
	var vector				ContactPosition;
	var vector				ContactNormal;
	var PhysicalMaterial	PhysicalMaterial;
};
/** This is the highest (i.e. in the bone hierarchy) contact of the flame spray mesh. */
var private transient FlameMeshContact		HighestFlameMeshContactThisTick;
var private transient bool					bFlameMeshCollidedThisTick;
var private transient bool					bFlameMeshCollidedLastTick;
/** How long the flame mesh has been touching something */
var protected transient float				bFlameMeshCollisionDuration;

var protected transient GearAnim_BlendList	AnimBlendNode;
var protected transient AnimNodeSequence	StartFireSeqNode;
var protected transient AnimNodeSequence	StartARFireSeqNode;
var protected transient AnimNodeSequence	EndFireSeqNode;


var() config protected const bool	bLeaveStickyFire;
/** Radius around the spray's instigator that will get no sticky fire, to protect against self-inflicted damage. */
var() config protected const float	StickyFireOwnerSafeRadius;
/** Minimun time, in seconds, between sticky fire spawns. */
var() config protected const float	StickyFireMinTimeBetweenSpawns;
/** Last world time that a sticky fire was spawned. */
var private transient float			StickyFireLastSpawnTime;

/** Length of time the flame must be touching something before sticky fire may be left. */
var() config protected const float	StickyFireInitialDelay;

/** Location that interpolates towards current contact. */
var protected transient vector		StickyFireTestLocation;
/** How fast the test loc moves towards the contact pos. */
var() config protected const float	StickyFireTestLocInterpSpeed;
/** How close the test location must be to the current contact before sticky fire may be left. */
var() config protected const float	StickyFireTestLocThreshold;

/** Toggles bone rendering.  For debugging. */
var() private bool bDebugShowBones;
/** Toggles contact rendering.  For debugging. */
var() private bool bDebugShowContacts;
/** Toggles collision rendering.  For debugging. */
var() private bool bDebugShowCollision;
/** Toggles rendering of the splash damage radius.  For debugging. */
var() private bool bDebugShowSplashRadius;
/** True to make player use nonplayer particle systems. */
var() private bool bDebugForceNonPlayerParticles;

/** Enables per-poly collision, instead of simplified. */
var() private bool bTestCollideComplex;

/**
 * Struct defining a spray "seed".  The seeds are spewed like particles from the
 * spray's origin, and the spray skeletal mesh is fitted to this seed chain.
 */
struct native FlameSpraySeed
{
	var vector	Location;
	var vector	Velocity;
	var float	Age;
};
var transient array<FlameSpraySeed>	Seeds;

/** How fast the seeds are traveling at spawn.  Higher speeds == stiffer feeling spray. */
var() protected const float SeedSprayVel;
/** Seed deceleration as it travels.  Higher deceleration == softer spray at the far end. */
var() protected const float SeedDecel;
/** SeedSprayVel while AR is active */
var() protected const float SeedSprayVelAR;
/** SeedDecel while AR is active */
var() protected const float SeedDecelAR;
/** Seeds expire after this time.  Keep as short as is reasonable for memory/efficiency reasons. */
var() protected const float SeedMaxAge;

/** Don't expire any seeds if seed chain length is below this (set this to be longer than the flame mesh). */
var() protected const float SeedMinChainLength;
/** Fixed-timestep simulation frequency for updating seed chain*/
var() protected const float SeedSimFreq;
/** Accumlator for simulation time, holds leftovers between updates. */
var protected transient float SeedSimTimeRemaining;
/** Toggles rendering of flame seeds.  For debugging. */
var() private bool			bDebugShowSeeds;

var() protected float		SeedWarmupTime;

/** GearPawn who is firing us, or None if appropriate.  Use this instead of Instigator. */
var protected transient GearPawn OwningGearPawn;

var protected transient bool bWaitingToDestroy;

var protected transient bool bDetached;

cpptext
{
private:
	void GetSeedChainLoc(FVector& OutLoc, FLOAT SeedCoord);

public:
	virtual FLOAT GetGravityZ()
	{
		// interpolate using the range
		FLOAT const Pct = Min<FLOAT>((CurrentAge / GravityScaleInTime), 1.f);
		return Lerp<FLOAT>(GravityScaleRange.X, GravityScaleRange.Y, Pct) * GWorld->GetGravityZ();
	}
};


/** */
native final function UpdateFlameSeeds(float DeltaTime);


/** Internal.  Sets up references to AnimNodes we need to manipulate. */
simulated private function CacheAnimNodes()
{
	AnimBlendNode		= GearAnim_BlendList(SkeletalSprayMesh.FindAnimNode('FlameBlendNode'));
	StartFireSeqNode	= AnimNodeSequence(SkeletalSprayMesh.FindAnimNode('StartSeqNode'));
	StartARFireSeqNode	= AnimNodeSequence(SkeletalSprayMesh.FindAnimNode('ARStartSeqNode'));
	EndFireSeqNode		= AnimNodeSequence(SkeletalSprayMesh.FindAnimNode('EndSeqNode'));
	PruneSkelControl	= GearSkelCtrl_Prune(SkeletalSprayMesh.FindSkelControl(PruneSkelControlName));
	ScalingSkelControl	= GearSkelCtrl_FlamethrowerScaling(SkeletalSprayMesh.FindSkelControl('FlameScale'));
}

/** Internal.  Clears cached refs to AnimNodes that we stored earlier. */
simulated private function ClearAnimNodes()
{
	AnimBlendNode		= None;
	StartFireSeqNode	= None;
	StartARFireSeqNode	= None;
	EndFireSeqNode		= None;
	PruneSkelControl	= None;
}

/** Internal.  Used to randomly flicker the point lights attached to the spray. */
simulated private function float GetFlickerVal(float BaseVal, float Intensity, float Last, float DeltaTime, float InterpSpeed)
{
	local float GoalVal;
	GoalVal = BaseVal + RandRange(-Intensity, Intensity);
	return FInterpTo(Last, GoalVal, DeltaTime, InterpSpeed);
}

/**
 * Calculate actual damage.
 * Server only.
 */
function private float GetDamage(float BurnTime, float HitDist)
{
	local vector2d DPSRange, DistRange;
	local float Damage, Pct;
	local GearWeap_FlameThrowerBase Weap;
	local bool bARActive;

	bARActive = ((OwningGearPawn != None) && OwningGearPawn.bActiveReloadBonusActive);
	if (OwningGearPawn != none && OwningGearPawn.Weapon != none)
	{
		Weap = GearWeap_FlameThrowerBase(OwningGearPawn.Weapon);
		if(Weap != none)
		{
			bARActive = Weap.IsActiveReloadActive(OwningGearPawn);
		}
	}

	// DEBUG
	//`log(GetFuncName()@self@bARActive);

	DPSRange = bARActive ? FlameDamagePerSecAR : FlameDamagePerSec;

	DistRange = FlameDamageScaleDistRange;
	if (bARActive)
	{
		DistRange.X *= 1.2f;
		DistRange.Y *= 1.2f;
	}

	Pct = FClamp(GetRangePctByValue(DistRange, HitDist), 0.f, 1.f);
	Damage = BurnTime * GetRangeValueByPct(DPSRange, Pct);

	return Damage;
}


/** Creates an AudioComponent if needed */
simulated private function AudioComponent PlayLoopingSound(SoundCue Cue, AudioComponent AC, float FadeInTime)
{
	if( Cue != None )
	{
		if (AC == None)
		{
			AC = CreateAudioComponent(Cue, FALSE, TRUE);
			if (AC != None)
			{
				AC.bUseOwnerLocation	= TRUE;
				AC.bAutoDestroy			= TRUE;
				AttachComponent(AC);
			}
		}
		if (AC != None)
		{
			AC.FadeIn(FadeInTime, 1.f);
		}
	}

	return AC;
}


/** Stop active-relaod version of the spray, transition back to normal version. */
private function FinishActiveReloadSpray()
{
	local bool bOldPlayActiveChild;

	// ensure we don't play this anim, we want to blend to the end state
	bOldPlayActiveChild = AnimBlendNode.bPlayActiveChild;
	AnimBlendNode.bPlayActiveChild = FALSE;

	StartFireSeqNode.SetPosition(StartFireSeqNode.AnimSeq.SequenceLength, FALSE);		// make sure it's at the end
	AnimBlendNode.SetActiveChild(0, 0.25f);

	AnimBlendNode.bPlayActiveChild = bOldPlayActiveChild;
}

/** Internal. */
simulated protected function GearPawn FindOwningGearPawn()
{
	return GearPawn(Instigator);
}

simulated function PostBeginPlay()
{
	local int ChainIdx, Idx;

	super.PostBeginPlay();

	// find owner pawn, handles boomer special case.
	OwningGearPawn = FindOwningGearPawn();

	CacheAnimNodes();
	SetupPerBoneFireFX();

	// we want to set splash pos/rot in world space
	SplashDirectPSC.SetAbsolute(true, true, true);
	SplashMaterialBasedPSC.SetAbsolute(true, true, true);
	SplashGlancingPSC.SetAbsolute(true, true, true);
	SplashPawnPSC.SetAbsolute(true, true, true);

	// set up material parameters
	MIC_FlameMat0 = SkeletalSprayMesh.CreateAndSetMaterialInstanceConstant(0);
	MIC_FlameMat1 = SkeletalSprayMesh.CreateAndSetMaterialInstanceConstant(1);
	MIC_FlameMat2 = SkeletalSprayMesh.CreateAndSetMaterialInstanceConstant(2);

	// look up and cache bone indices
	for (ChainIdx=0; ChainIdx<BoneChain.length; ++ChainIdx)
	{
		BoneChain[ChainIdx].BoneIndex = SkeletalSprayMesh.MatchRefBone(BoneChain[ChainIdx].BoneName);
		if (BoneChain[ChainIdx].BoneIndex == INDEX_NONE)
		{
			`log("Warning!  Main chain bone"@BoneChain[ChainIdx].BoneName@"not found in FlameThrowerSpray skeletal mesh!  Bad things may happen.  Bad things.");
		}
	}

	// set up splash audio.  attach to last bone, bone scaling will take care of putting tbat bone
	// at the splash location.
	SkeletalSprayMesh.AttachComponent(SplashDirectAC,			BoneChain[BoneChain.Length-1].BoneName);
	SkeletalSprayMesh.AttachComponent(SplashPawnAC,				BoneChain[BoneChain.Length-1].BoneName);
	SkeletalSprayMesh.AttachComponent(SplashGlancingAC,			BoneChain[BoneChain.Length-1].BoneName);
	SkeletalSprayMesh.AttachComponent(SplashMaterialBasedAC,	BoneChain[BoneChain.Length-1].BoneName);

	// spawn starting emitter
	SkeletalSprayMesh.AttachComponent(StartFirePSC, 'bone15');

	// attach on the lights
	for (Idx=0; Idx<FlameLights.length; ++Idx)
	{
		SkeletalSprayMesh.AttachComponent(FlameLights[Idx].Light, BoneChain[FlameLights[Idx].BoneChainIndex].BoneName);
	}
	SkeletalSprayMesh.AttachComponent(PSC_OwnerGlow, BoneChain[OwnerGlowBoneChainIndex].BoneName);
}

simulated function BeginSpray()
{
	local int Idx;

	SetHidden(FALSE);
	bStasis = FALSE;
	bDetached = FALSE;
	bSkeletonHasBeenUpdated=FALSE;

	// make sure pruning is off to begin
	ClearMeshPruning();

	// reinit various transients
	MaterialCurrentFadeVal = default.MaterialCurrentFadeVal;
	MIC_FlameMat0.SetScalarParameterValue('Fade', MaterialCurrentFadeVal);
	MIC_FlameMat1.SetScalarParameterValue('Fade', MaterialCurrentFadeVal);
	MIC_FlameMat2.SetScalarParameterValue('Fade', MaterialCurrentFadeVal);

	ScalingSkelControl.ResetTransients();

	CurrentSplashAC = None;
	if(bSplashActive)
	{
		CurrentSplashEffect.DeactivateSystem();
		if (CurrentSplashAC != None)
		{
			CurrentSplashAC.FadeOut(0.3f, 0.f);
		}
	}
	CurrentSplashEffect = None;
	CurrentSprayPitchMult = default.CurrentSprayPitchMult;
	CurrentSprayVolumeMult = default.CurrentSprayVolumeMult;
	PerBoneFireFXActivationDelayTimer = default.PerBoneFireFXActivationDelayTimer;
	bSplashActive = FALSE;
	bFlameMeshCollidedThisTick = FALSE;
	bFlameMeshCollidedLastTick = FALSE;
	HighestFlameMeshContactThisTick.BoneChainIndex = 0;
	bFlameMeshCollisionDuration = 0.f;
	Seeds.length = 0;
	SeedSimTimeRemaining = 0.f;
	bWaitingToDestroy = FALSE;
	ClearTimer(nameof(DestroyIfAllEmittersFinished));

	// start audio
	if (FireSprayStartSound != None)
	{
		PlaySound(FireSprayStartSound, TRUE);
	}
	if (FireSprayLoopSound != None)
	{
		AC_FireSprayLoop = PlayLoopingSound(FireSprayLoopSound, AC_FireSprayLoop, 1.2f);
	}

	// start animation
	StartFireSeqNode.SetPosition(0.f, FALSE);
	StartARFireSeqNode.SetPosition(0.f, FALSE);
	if ( (OwningGearPawn != None) && OwningGearPawn.bActiveReloadBonusActive )
	{
		// ar start anim
		StartARFireSeqNode.bPlaying = TRUE;
		AnimBlendNode.SetActiveChild(2, 0.f);
	}
	else
	{
		// normal start anim
		StartFireSeqNode.bPlaying = TRUE;
		AnimBlendNode.SetActiveChild(0, 0.f);
	}

	for (Idx=0; Idx<FlameLights.length; ++Idx)
	{
		FlameLights[Idx].Light.SetEnabled(TRUE);
	}
	PSC_OwnerGlow.ActivateSystem(TRUE);

	SetTimer( 0.50f, TRUE, nameof(MaybeLeaveScorchDecal) );

	// get some seeds out there, if desired
	if (SeedWarmupTime > 0.f)
	{
		UpdateFlameSeeds(SeedWarmupTime);
	}

	CurrentAge = 0.f;
}

simulated function Destroyed()
{
	local int Idx;

	// make sure fire particles got destroyed
	for (Idx = 0; Idx < BoneChain.length; ++Idx)
	{
		if (BoneChain[Idx].BonePSC0 != None)
		{
			BoneChain[Idx].BonePSC0.DeactivateSystem();
		}
		if (BoneChain[Idx].BonePSC1 != None)
		{
			BoneChain[Idx].BonePSC1.DeactivateSystem();
		}
	}

	if( StartFirePSC != None )
	{
		StartFirePSC.DeactivateSystem();
	}

	//if( EndFireEmitter != None )
	//{
 //		EndFireEmitter.Destroy();
 //		EndFireEmitter = None;
	//}

	// Clear references to animations nodes
	ClearAnimNodes();
	Super.Destroyed();
}


simulated protected function ClearMeshPruning()
{
	if (PruneSkelControl != None)
	{
		PruneSkelControl.SetSkelControlActive(FALSE);
		PruneSkelControl.StartBoneName = '';
	}
}

simulated function DetachAndFinish()
{
	bDetached = TRUE;
	if (!bFlameMeshCollidedLastTick && !bFlameMeshCollidedThisTick)
	{
		HighestFlameMeshContactThisTick.BoneChainIndex = 0;
	}

	// play "stopped firing" sound
	if (FireSprayStopSound != None)
	{
		PlaySound(FireSprayStopSound, TRUE);
	}
	// stop looping fire sound
	if (AC_FireSprayLoop != None)
	{
		AC_FireSprayLoop.FadeOut(0.3f, 0.f);
		AC_FireSprayLoop = None;
	}

	// start ending animation
	EndFireSeqNode.bPlaying = TRUE;
	if ( StartFireSeqNode.bPlaying || StartARFireSeqNode.bPlaying )
	{
		// blend if start anim is playing as well
		AnimBlendNode.SetActiveChild(1, 0.15f);
	}
	else
	{
		AnimBlendNode.SetActiveChild(1, 0.f);
	}
	EndFireSeqNode.bCauseActorAnimEnd = TRUE;				// we want the OnAnimEnd notify

	// turn on end-of-firing smoke emitter
//	EndFireEmitter = Spawn(class'SpawnedGearEmitter',,,,,, TRUE);
//	if (EndFireEmitter != None)
//	{
//		EndFireEmitter.SetBase(self, , SkeletalSprayMesh, 'bone01');
//		EndFireEmitter.SetTemplate(EndFireParticleSystem, TRUE);
//	}
}

simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local int Idx;

	if (SeqNode == EndFireSeqNode)
	{
		// turn off fire particles
		for (Idx=0; Idx<BoneChain.length; ++Idx)
		{
			if (BoneChain[Idx].BonePSC0 != None)
			{
				BoneChain[Idx].BonePSC0.DeactivateSystem();
			}
			if (BoneChain[Idx].BonePSC1 != None)
			{
				BoneChain[Idx].BonePSC1.DeactivateSystem();
			}
		}

		// start wait for systems to finish so we can destroy
		SetTimer(0.5f, TRUE, nameof(DestroyIfAllEmittersFinished));
		bWaitingToDestroy = TRUE;

		// deactivate any splashes
		if (CurrentSplashEffect != None)
		{
			CurrentSplashEffect.DeactivateSystem();
		}
		if (CurrentSplashAC != None)
		{
			CurrentSplashAC.FadeOut(0.3f, 0.f);
		}
		bSplashActive = FALSE;

		// turn off ownerglow
		PSC_OwnerGlow.DeactivateSystem();

		// turn off lights
		for (Idx=0; Idx<FlameLights.length; ++Idx)
		{
			FlameLights[Idx].Light.SetEnabled(FALSE);
		}

		// reset anims
		StartFireSeqNode.SetPosition(0.f, FALSE);
		StartARFireSeqNode.SetPosition(0.f, FALSE);
	}

	super.OnAnimEnd(SeqNode, PlayedTime, ExcessTime);
}

simulated native private function DestroyIfAllEmittersFinished();


/** Internal. */
simulated private function SetupPerBoneFireFX()
{
	local int Idx, NumChainBones;

	if ( bDoPerBoneFireFX && (PerBoneFireFXGlobalScale > 0.f) )
	{
		NumChainBones = BoneChain.Length;
		for (Idx=0; Idx<NumChainBones; ++Idx)
		{
			if ( (BoneChain[Idx].EffectScale > 0.f) && (BoneChain[Idx].BonePSC0 != None) )
			{
				SkeletalSprayMesh.AttachComponent(BoneChain[Idx].BonePSC0, BoneChain[Idx].BoneName);
				BoneChain[Idx].BonePSC0.SetFloatParameter('FlameScale', BoneChain[Idx].EffectScale*PerBoneFireFXGlobalScale);
			}
			if ( (BoneChain[Idx].EffectScale > 0.f) && (BoneChain[Idx].BonePSC1 != None) )
			{
				SkeletalSprayMesh.AttachComponent(BoneChain[Idx].BonePSC1, BoneChain[Idx].BoneName);
				BoneChain[Idx].BonePSC1.SetFloatParameter('FlameScale', BoneChain[Idx].EffectScale*PerBoneFireFXGlobalScale);
			}

			if (bDebugForceNonPlayerParticles)
			{
				BoneChain[Idx].BonePSC0.SetOwnerNoSee(BoneChain[Idx].BonePSC0.bOnlyOwnerSee);
				BoneChain[Idx].BonePSC0.SetOnlyOwnerSee(FALSE);

				BoneChain[Idx].BonePSC1.SetOwnerNoSee(BoneChain[Idx].BonePSC1.bOnlyOwnerSee);
				BoneChain[Idx].BonePSC1.SetOnlyOwnerSee(FALSE);
			}

			PerBoneFireFXActivationDelayTimer = 0.f;
		}
	}
}

simulated function AttachToWeapon(Weapon Weap, Name SocketName)
{
	local Actor BaseActor;
	local MeshComponent MeshToAttachTo;
	local GearWeap_FlameThrowerBase FT;

	FT = GearWeap_FlameThrowerBase(Weap);
	MeshToAttachTo = Weap.mesh;
	if(FT != none)
	{
		MeshToAttachTo = FT.WeaponMesh;
	}

	BaseActor = (Pawn(Weap.Owner.Base) != None) ? Weap.Owner.Base : Weap.Owner;
	SetBase(BaseActor,, SkeletalMeshComponent(MeshToAttachTo), SocketName);
}

simulated function UpdatePerBoneFireFX(float DeltaTime)
{
//	local float PctActivated;
	local int ChainIdx, NumChainBones;

	// these are chain indices
	local int ActiveFXRangeMax;

	NumChainBones = BoneChain.Length;

	// handle delayed activation
//	PerBoneFireFXActivationDelayTimer += DeltaTime;
//	PctActivated = FClamp((PerBoneFireFXActivationDelayTimer - PerBoneFireFXActivationDelayRange.X) / (PerBoneFireFXActivationDelayRange.Y - PerBoneFireFXActivationDelayRange.X), 0.f, 1.f);
//	ActiveFXRangeMax = int(PctActivated * PctActivated * NumChainBones);
//
// 	ActiveFXRangeMin = NumChainBones;
// 	if (bSkeletonHasBeenUpdated)
// 	{
// 		for (ChainIdx=0; ChainIdx<NumChainBones; ++ChainIdx)
// 		{
// 			if ( VSize(BoneChain[ChainIdx].LastLoc - Location) > PerBoneFireFXActivationDist )
// 			{
// 				ActiveFXRangeMin = ChainIdx;
// 				break;
// 			}
// 		}
// 	}


	ActiveFXRangeMax = NumChainBones;

	// figure where to stop allowing effects
	if ( bFlameMeshCollidedThisTick || bDetached )
	{
		if (HighestFlameMeshContactThisTick.BoneChainIndex == INDEX_NONE)
		{
			ActiveFXRangeMax = 0;
		}
		else
		{
			ActiveFXRangeMax = HighestFlameMeshContactThisTick.BoneChainIndex - 1;
		}
	}

	// turn on what needs turned on
	for (ChainIdx=0; ChainIdx<NumChainBones; ++ChainIdx)
	{
		if ( (ChainIdx <= ActiveFXRangeMax) && (CurrentAge > BoneChain[ChainIdx].ParticleActivationDelay) )
		{
			// make sure it's on
			SetBoneSpawnParticlesActive(BoneChain[ChainIdx].BonePSC0, TRUE);
			SetBoneSpawnParticlesActive(BoneChain[ChainIdx].BonePSC1, TRUE);
			//DrawDebugBox(BoneChain[ChainIdx].LastLoc, vect(6,6,6), 64, 255, 64);
		}
		else
		{
			// make sure its off
			SetBoneSpawnParticlesActive(BoneChain[ChainIdx].BonePSC0, FALSE);
			SetBoneSpawnParticlesActive(BoneChain[ChainIdx].BonePSC1, FALSE);
			//DrawDebugBox(BoneChain[ChainIdx].LastLoc, vect(6,6,6), 255, 64, 64);
		}
	}
}

/** Internal.  Turns bone spawns on/off. */
simulated protected function SetBoneSpawnParticlesActive(ParticleSystemComponent PSC, bool bActive)
{
	if (PSC != None)
	{
		//// nothing gets reactivated after detach
		//if (bDetached)
		//{
		//	bActive = FALSE;
		//}

		PSC.SetActive(bActive);
	}
}

/** For debugging.  Renders boxes at the bone positions. */
simulated native final function DebugRenderBones();

/** Loc and Norm must be valid. */
//simulated protected function MaybeLeaveStickyFire(float DeltaTime, vector Loc, vector Norm)
//{
//	if ( bLeaveStickyFire &&
//		bFlameMeshCollidedThisTick &&
//		(HighestFlameMeshContactThisTick.Actor == None || !HighestFlameMeshContactThisTick.Actor.IsA('Pawn')) &&
//		(bFlameMeshCollisionDuration > StickyFireInitialDelay) )
//	{
//		if (bFlameMeshCollidedLastTick)
//		{
//			StickyFireTestLocation = VInterpTo(StickyFireTestLocation, Loc, DeltaTime, StickyFireTestLocInterpSpeed);
//		}
//		else
//		{
//			// first touch, don't
//			StickyFireTestLocation = Loc;
//		}
//		//DrawDebugSphere(StickyFireTestLocation, 32.f, 10, 255, 255, 0);
//
//		if ( (VSize(StickyFireTestLocation - Loc) < StickyFireTestLocThreshold) )
//		{
//			if ( (StickyFireLastSpawnTime + StickyFireMinTimeBetweenSpawns) < WorldInfo.TimeSeconds )
//			{
//				if ( VSize(Loc-OwningGearPawn.Location) > StickyFireOwnerSafeRadius )
//				{
//					// spawn!
//					Spawn(class'Emit_StickyFire', self,, Loc, rotator(Norm),, TRUE);
//					StickyFireLastSpawnTime = WorldInfo.TimeSeconds;
//				};
//			}
//		}
//	}
//}

/** Figure out what the flame hit. */
native final function DoFlameCollisionDetection(float DeltaTime);

simulated function Tick(float DeltaTime)
{
	local Actor TouchActor;
	local float Dmg, DistToHitActor, MatFade;
	local vector Momentum;
	local float CurrentHeat, CurScale;
	local int Idx;

	local vector LightPosWorld, LightPosLocal;
	local rotator DummyRot0, DummyRot1;
	local bool bDeltDamage;
	local GearPRI GPRI;

	super.Tick(DeltaTime);

	CurrentAge += DeltaTime;

	`if(`notdefined(FINAL_RELEASE))
	// debug
	if (bDebugShowBones)
	{
		DebugRenderBones();
	}
	`endif

	// calc rot speed, used by animations and audio
	RotationSpeed = RDiff(Rotation, LastRotation) / DeltaTime;

	// hacky flicker for lights.
	for (Idx=0; Idx<FlameLights.length; ++Idx)
	{
		FlameLights[Idx].LastLightBrightness = FMax(0.f, GetFlickerVal(default.FlameLights[Idx].Light.Brightness, FlameLights[Idx].FlickerIntensity, FlameLights[Idx].LastLightBrightness, DeltaTime, FlameLights[Idx].FlickerInterpSpeed));
		FlameLights[Idx].Light.SetLightProperties(FlameLights[Idx].LastLightBrightness);
//		DrawDebugSphere(FlameLights[Idx].Light.GetOrigin(), FlameLights[Idx].Light.Radius, 10, 255, 128, 0);
	}

	// handle "heating up" material parameter.  it seems only mat 2 gets this
	CurrentHeat = GetRangeValueByPct(MaterialHeatRange, FMin(CurrentAge / MaterialHeatRampTime, 1.f));
	MIC_FlameMat2.SetScalarParameterValue('Heat', CurrentHeat);

	// handle "fading out" material parameter
	if (bDetached)
	{
		MaterialCurrentFadeVal -= DeltaTime / MaterialFadeOutTime;
		MaterialCurrentFadeVal = FMax(0.f, MaterialCurrentFadeVal);
		MatFade = MaterialCurrentFadeVal ** MatFadePow;
		MIC_FlameMat0.SetScalarParameterValue('Fade', MatFade);
		MIC_FlameMat1.SetScalarParameterValue('Fade', MatFade);
		MIC_FlameMat2.SetScalarParameterValue('Fade', MatFade);
	}

	// handle ownerglow scaling in
	CurScale = FMin((CurrentAge / OwnerGlowScaleInTime), 1.f);
	PSC_OwnerGlow.SetScale(CurScale);

	if (bDetached)
	{
		// detached.  if attached effects are past the last collision point, just kill it so it doesn't poke through
		if (HighestFlameMeshContactThisTick.BoneChainIndex <= OwnerGlowBoneChainIndex)
		{
			PSC_OwnerGlow.DeactivateSystem();
		}
		for (Idx=0; Idx<FlameLights.length; ++Idx)
		{
			if (HighestFlameMeshContactThisTick.BoneChainIndex <= FlameLights[Idx].BoneChainIndex)
			{
				FlameLights[Idx].Light.SetEnabled(FALSE);
			}
		}
	}

	// do flame collision checks to see what we're touching.  note this relies on the current seed locations,
	// so don't update the seeds until after this is called
	DoFlameCollisionDetection(DeltaTime);

	// update the seeds
	if (!bWaitingToDestroy && !bStasis)
	{
		UpdateFlameSeeds(DeltaTime);

		// server stuff, damage and whatnot
		if (WorldInfo.NetMode != NM_Client)
		{
			// damage whatever the flame mesh ran into
			if (bFlameMeshCollidedThisTick)
			{
				TouchActor = HighestFlameMeshContactThisTick.Actor;

				if ( (TouchActor != None) && (TouchActor != OwningGearPawn) )
				{
					Momentum = TouchActor.Location - OwningGearPawn.Location;
					DistToHitActor = VSize(Momentum);
					Momentum *= (MomentumScale / DistToHitActor);

					Dmg = GetDamage(DeltaTime, DistToHitActor);
					TouchActor.TakeDamage(Dmg, OwningGearPawn.Controller, TouchActor.Location, Momentum, MyDamageType,, self);

                    // Only track if you are hitting another pawn
					if ( Pawn(TouchActor) != none )
					{
						bDeltDamage = true;
					}
				}
			}

			// Track how much time we have been firing and how much time we do damage

			GPRI = GearPRI(OwningGearPawn.PlayerReplicationInfo);

			if(GPRI != none)
			{
				GPRI.TotalFlameTime += DeltaTime;

				if (bDeltDamage)
				{
					GPRI.TotalFlameDmgTime += DeltaTime;
				}
			}

			// note: the splash may also do some damage, but not to the direct-hit actor again.
			// @see UpdateSplashes().
		}

		UpdateSplashes(DeltaTime);
		UpdateFireLoopSound(DeltaTime);
		if (bDoPerBoneFireFX)
		{
			UpdatePerBoneFireFX(DeltaTime);
		}

		// update lights
		for (Idx=0; Idx<FlameLights.length; ++Idx)
		{
			if ( bFlameMeshCollidedThisTick && (FlameLights[Idx].BoneChainIndex >= HighestFlameMeshContactThisTick.BoneChainIndex) )
			{
				// light is in the "collapsed" portion of the mesh
				// set position out a little from the touch point
				//LightPosWorld = LastSplashLoc + SplashNormal * 64.f;
				LightPosWorld = LastSplashLoc + HighestFlameMeshContactThisTick.ContactNormal * 64.f;
				SkeletalSprayMesh.TransformToBoneSpace( BoneChain[FlameLights[Idx].BoneChainIndex].BoneName, LightPosWorld, DummyRot0, LightPosLocal, DummyRot1 );
				FlameLights[Idx].Light.SetTranslation(LightPosLocal);
			}
			else
			{
				// zero offset
				FlameLights[Idx].Light.SetTranslation(vect(0,0,0));
			}
		}

		// deal with AR stuff
		if (AnimBlendNode.ActiveChildIndex == 2)			// ick.  2 is the AR-start child
		{
			if ( (OwningGearPawn != None) && !OwningGearPawn.bActiveReloadBonusActive )
			{
				FinishActiveReloadSpray();
			}
		}

		// handle flame collisions
		if (bFlameMeshCollidedThisTick)
		{
			if (HighestFlameMeshContactThisTick.BoneChainIndex == INDEX_NONE)
			{
				PruneSkelControl.StartBoneName = BoneChain[0].BoneName;
			}
			else if (HighestFlameMeshContactThisTick.BoneChainIndex < (BoneChain.length-1))
			{
				PruneSkelControl.StartBoneName = BoneChain[HighestFlameMeshContactThisTick.BoneChainIndex].BoneName;
			}

			if (PruneSkelControl.StrengthTarget == 0.f)
			{
				// need to activate the control
				PruneSkelControl.SetSkelControlActive(TRUE);
			}

	//		MaybeLeaveStickyFire(DeltaTime, HighestFlameMeshContactThisTick.ContactPosition, HighestFlameMeshContactThisTick.ContactNormal);
	//		DrawDebugSphere(HighestFlameMeshContactThisTick.ContactPosition, 12, 8, 255, 0, 0, FALSE);
		}
		else if (!bDetached)
		{
			// don't turn off the pruning if we're animating out.  once a bone gets turned off, it's off for good
			if (PruneSkelControl.StrengthTarget > 0.f)
			{
				// turn off the control
				PruneSkelControl.SetSkelControlActive(FALSE);
			}
		}
	}

	// update collision duration
	if (bFlameMeshCollidedThisTick)
	{
		bFlameMeshCollisionDuration += DeltaTime;
	}
	else
	{
		bFlameMeshCollisionDuration = 0.f;
	}

	bFlameMeshCollidedLastTick = bFlameMeshCollidedThisTick;
	bFlameMeshCollidedThisTick = FALSE;

	LastRotation = Rotation;
}

simulated private final function UpdateSplashes(float DeltaTime)
{
	local ParticleSystem MaterialBasedSplashFX;
	local ParticleSystemComponent DesiredSplashPSC;
	local AudioComponent DesiredSplashAC;
	local vector DesiredSplashLoc;
	local rotator DesiredSplashRot;
	local vector IncidentDirection, SplashX, SplashY, SplashZ;
	local vector SplashNormal, Momentum;
	local GearPawn HitGP;
	local Actor SplashTouchActor;
	local float Dot, Dmg;

	// check for world hits
	if ( bFlameMeshCollidedThisTick && (HighestFlameMeshContactThisTick.BoneChainIndex <= LastBoneChainIndexThatCanSpawnSplashEffects) )
	{
		// choose splash

		// see if material wants to dictate splash
		MaterialBasedSplashFX = class'GearPhysicalMaterialProperty'.static.DetermineImpactParticleSystem(HighestFlameMeshContactThisTick.PhysicalMaterial, MyDamageType, FALSE, WorldInfo, OwningGearPawn, TRUE);
		if (MaterialBasedSplashFX != None)
		{
			DesiredSplashAC = SplashMaterialBasedAC;
			DesiredSplashAC.SoundCue = class'GearPhysicalMaterialProperty'.static.DetermineImpactSound( HighestFlameMeshContactThisTick.PhysicalMaterial, MyDamageType, FALSE, WorldInfo);

			DesiredSplashPSC = SplashMaterialBasedPSC;
			if (DesiredSplashPSC.Template != MaterialBasedSplashFX)
			{
				DesiredSplashPSC.SetTemplate(MaterialBasedSplashFX);
			}

			SplashNormal = HighestFlameMeshContactThisTick.ContactNormal;
			DesiredSplashRot = rotator(SplashNormal);
			DesiredSplashLoc = HighestFlameMeshContactThisTick.ContactPosition + HighestFlameMeshContactThisTick.ContactNormal * 16;
		}
		else
		{
			// material doesn't care, check for other cases we handle..
			HitGP = GearPawn(HighestFlameMeshContactThisTick.Actor);
			if (HitGP != None)
			{
				// we hit a pawn, do pawn splashes
				DesiredSplashPSC = SplashPawnPSC;
				DesiredSplashAC = SplashPawnAC;
				SplashNormal = Normal(Location - HighestFlameMeshContactThisTick.ContactPosition);
				DesiredSplashLoc = HighestFlameMeshContactThisTick.ContactPosition + HighestFlameMeshContactThisTick.ContactNormal * 16;
				DesiredSplashRot = rotator(SplashNormal);
			}
			else
			{
				// we hit something we'll treat as the world, decide which world splash to play
				IncidentDirection = Normal(HighestFlameMeshContactThisTick.ContactPosition - Location);
				Dot = HighestFlameMeshContactThisTick.ContactNormal dot IncidentDirection;

				if (Dot > SplashGlancingDotLimit)
				{
					// do the glancing-splash effect
					DesiredSplashPSC = SplashGlancingPSC;
					DesiredSplashAC = SplashGlancingAC;
					SplashZ = HighestFlameMeshContactThisTick.ContactNormal;
					SplashY = SplashZ cross IncidentDirection;
					SplashX = SplashY cross SplashZ;

					DesiredSplashLoc = HighestFlameMeshContactThisTick.ContactPosition + HighestFlameMeshContactThisTick.ContactNormal * 16;
					DesiredSplashRot = OrthoRotation(SplashX, SplashY, SplashZ);
					SplashNormal = SplashZ;
				}
				else
				{
					// do the direct-splash effect.  note the different orientation
					DesiredSplashPSC = SplashDirectPSC;
					DesiredSplashAC = SplashDirectAC;
					SplashNormal = HighestFlameMeshContactThisTick.ContactNormal;
					DesiredSplashRot = rotator(SplashNormal);
					DesiredSplashLoc = HighestFlameMeshContactThisTick.ContactPosition + HighestFlameMeshContactThisTick.ContactNormal * 16;
				}

				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc+vector(DesiredSplashRot)*350, 0, 255, 255);
				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc-Normal(IncidentDirection)*400, 255, 255, 0);
				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc+HighestFlameMeshContactThisTick.ContactNormal*450, 255, 0, 0);
				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc+SplashX*350, 255, 0, 0);
				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc+SplashY*400, 0, 255, 0);
				//DrawDebugLine(DesiredSplashLoc, DesiredSplashLoc+SplashZ*450, 0, 0, 255);
			}
		}
	}

	// make sure we are using the proper splash
	if (DesiredSplashPSC != None)
	{
		if (CurrentSplashEffect != DesiredSplashPSC)
		{
			// shut down current...
			if (CurrentSplashEffect != None)
			{
				CurrentSplashEffect.DeactivateSystem();
			}
			bSplashActive = FALSE;

			// set new, will start after loc/rot is set...
			CurrentSplashEffect = DesiredSplashPSC;
		}

		// move to proper loc/rot, don't do interp if turning it on for the first time
		if (bSplashActive)
		{
			LastSplashRot = RInterpTo(LastSplashRot, DesiredSplashRot, DeltaTime, SplashRotInterpSpeed);
			LastSplashLoc = VInterpTo(LastSplashLoc, DesiredSplashLoc, DeltaTime, SplashLocInterpSpeed);
		}
		else
		{
			LastSplashRot = DesiredSplashRot;
			LastSplashLoc = DesiredSplashLoc;
		}
		CurrentSplashEffect.SetRotation(LastSplashRot);
		CurrentSplashEffect.SetTranslation(LastSplashLoc);
//		DrawDebugLine(LastSplashLoc, LastSplashLoc+vector(LastSplashRot)*200, 255, 255, 0);
//		DrawDebugSphere(LastSplashLoc, 32, 10, 255, 255, 255);

		// turn it on?
		if (!bSplashActive)
		{
			// note we need to do this AFTER the setting the loc/rot of the component, else
			// the location tracking gets messed up and SpawnPerUnit spawns some rogue particles
			CurrentSplashEffect.ActivateSystem(TRUE);
			bSplashActive = TRUE;
		}

		// debug
		//GetAxes(LastSplashRot, SplashX, SplashY, SplashZ);
		//DrawDebugLine(LastSplashLoc, LastSplashLoc+SplashX*350, 255, 0, 0);
		//DrawDebugLine(LastSplashLoc, LastSplashLoc+SplashY*400, 0, 255, 0);
		//DrawDebugLine(LastSplashLoc, LastSplashLoc+SplashZ*450, 0, 0, 255);

		// Check for folks to damage.
		if (WorldInfo.NetMode != NM_Client)
		{
			`if(`notdefined(FINAL_RELEASE))
			// also do splash damage (to pawns and grenades only, for now)
			if (bDebugShowSplashRadius)
			{
				DrawDebugSphere(LastSplashLoc, SplashDamageRadius, 10, 255, 255, 0);
			}
			`endif

			foreach VisibleCollidingActors(class'Actor', SplashTouchActor, SplashDamageRadius, LastSplashLoc, TRUE)
			{
				if ((GearPawn(SplashTouchActor) != None || GearProj_Grenade(SplashTouchActor) != None) && SplashTouchActor != HighestFlameMeshContactThisTick.Actor)
				{
					// Don't do splash damage to blind firing instigator
					if( SplashTouchActor == OwningGearPawn && OwningGearPawn.IsBlindFiring(OwningGearPawn.CoverAction) )
					{
						continue;
					}

					Dmg = ((OwningGearPawn != None) && OwningGearPawn.bActiveReloadBonusActive) ? SplashDamagePerSecondAR : SplashDamagePerSecond;  // base damage
					Dmg *= DeltaTime;	// scale for time
					Dmg *= FClamp(1.f - VSize(SplashTouchActor.Location - LastSplashLoc) / SplashDamageRadius, 0.f, 1.f) ** SplashDamageFalloffExponent;   // scaled by dist

					Momentum = (SplashTouchActor == OwningGearPawn) ? vector(OwningGearPawn.Rotation) : Normal(SplashTouchActor.Location - OwningGearPawn.Location);
					Momentum *= MomentumScale;

					SplashTouchActor.TakeDamage(Dmg, OwningGearPawn.Controller, SplashTouchActor.Location, Momentum, class'GDT_FlamethrowerSpray',, self);
				}
			}
		}
	}
	else
	{
		if (bSplashActive)
		{
			// deactivate
			CurrentSplashEffect.DeactivateSystem();
			CurrentSplashEffect = None;
			bSplashActive = FALSE;
		}
	}

	// handle looping splash audio
	if (DesiredSplashAC != CurrentSplashAC)
	{
		// turn off current audio
		if (CurrentSplashAC != None)
		{
			CurrentSplashAC.FadeOut(0.3f, 0.f);
		}

		if (DesiredSplashAC != None)
		{
			// turn on desired
			DesiredSplashAC.FadeIn(0.3f, 1.f);
		}

		CurrentSplashAC = DesiredSplashAC;
	}
	if (DesiredSplashAC != None)
	{
		// play at splash loc
		DesiredSplashAC.Location = DesiredSplashLoc;
	}
}


/** Handles pitch modulation for the flame whip audio effects, etc */
simulated private final function UpdateFireLoopSound(float DeltaTime)
{
	local float GoalPitch, GoalVolume, RotVelParam;

	RotVelParam = FClamp( (RotationSpeed - FireAudioAdj_RotVelRange.X) / (FireAudioAdj_RotVelRange.Y - FireAudioAdj_RotVelRange.X), 0.f, 1.f );

	GoalPitch = FireAudioAdj_PitchRange.X + RotVelParam * (FireAudioAdj_PitchRange.Y - FireAudioAdj_PitchRange.X);
	GoalVolume = FireAudioAdj_VolumeRange.X + RotVelParam * (FireAudioAdj_VolumeRange.Y - FireAudioAdj_VolumeRange.X);

	CurrentSprayPitchMult = FInterpTo(CurrentSprayPitchMult, GoalPitch, DeltaTime, SprayAudioAdjInterpSpeed);
	CurrentSprayVolumeMult = FInterpTo(CurrentSprayVolumeMult, GoalVolume, DeltaTime, SprayAudioAdjInterpSpeed);

	if (AC_FireSprayLoop != None)
	{
		AC_FireSprayLoop.PitchMultiplier = CurrentSprayPitchMult;
		AC_FireSprayLoop.SetFloatParameter('FlailVolume', CurrentSprayVolumeMult);
	}
}

simulated function ScorchDecalTraceFunc( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	out_TraceStart = HighestFlameMeshContactThisTick.ContactPosition + HighestFlameMeshContactThisTick.ContactNormal * 4.f;
	out_TraceDest = out_TraceStart - HighestFlameMeshContactThisTick.ContactNormal * 32.f;
//	`log("tracefunc called!"@out_TraceStart@out_TraceDest);
}


simulated function ScorchDecalChoiceFunc( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_FlameThrower_FlameIsBurning, WorldInfo );
	//`log("choicefunc called!"@out_DecalRotation@Hitinfo.PhysMaterial@out_DecalData.DecalMaterial);

	if( out_DecalData.bRandomizeRotation == TRUE )
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}


simulated function MaybeLeaveScorchDecal()
{
	//`log(WorldInfo.TimeSeconds@"might leave a decal now!"@Instigator@HighestFlameMeshContactThisTick.Actor);
	if (bFlameMeshCollidedLastTick)
	{
		if( (GearPawn(HighestFlameMeshContactThisTick.Actor) == None)
			&& (OwningGearPawn != None)
			&& ( WorldInfo.GRI.IsCoopMultiplayerGame() == FALSE ) // do not spawn flamethrower decals in horde
			)
		{
			//`log("   trying to leave one");
			OwningGearPawn.GearPawnFX.LeaveADecal(ScorchDecalTraceFunc, ScorchDecalChoiceFunc, OwningGearPawn.GearPawnFX.BloodDecalTimeVaryingParams_Default);
		}
	}
}

/** Calls ResetParticles on all contained PSCs.  Good to call when expected to be inactive for some time, as it reduces memory footprint. */
simulated native function ParticleSystemCleanUp();

defaultproperties
{
	bHidden=TRUE
	bStasis=TRUE
	bDetached=TRUE

	FlameDamageScaleDistRange=(X=100,Y=1000) // needs to be here for GearPC red crosshair check
	// all other default properties moved to the GearGameContentWeapons .uc

	SeedSprayVel=5000.f
	SeedDecel=13000.f
	SeedSprayVelAR=6000.f
	SeedDecelAR=13000.f
	SeedMaxAge=0.4f
	SeedMinChainLength=0.f
	SeedSimFreq=60.f
	SeedWarmupTime=0.25f

//	bDebugShowBones=TRUE
//	bDebugShowContacts=TRUE
//	bDebugShowCollision=TRUE
//	bDebugShowSplashRadius=TRUE
}
