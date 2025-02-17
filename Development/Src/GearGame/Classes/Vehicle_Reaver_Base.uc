/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Reaver_Base extends GearVehicle
	native(Vehicle)
	abstract
	config(Pawn)
	nativereplication
	notplaceable;

/** Default health. */
var()	config int		DefaultReaverHealth;

/** How low health has to go for reaver to land. */
var()	config int		HealthForLanding;

/** Amount to scale damage done to reaver when you hit it while flying. */
var()	config float	FlyingTakeDamageScale;

/** Sound used when flying around */
var	protected transient AudioComponent	FlyingSoundLoopAC;
var protected const SoundCue			FlyingSoundLoop;
var protected const SoundCue			FlyingSoundLoop_Player;
/** True to signal a change in driving status such that the flying audio may need to be changed. */
var protected transient bool			bRestartFlyingLoop;

/** Sound to play when reaver dies */
var		SoundCue		ReaverDeathSound;

/** Particle effect for when reaver dies through shooting. */
var		ParticleSystem	ReaverExplodeEffect;
var     ParticleSystem  ReaverExplodeNoGoreEffect;

/** Particle effect for when reaver dies with ragdoll + explode */
var		ParticleSystem	ReaverPreExplodeEffect;
/** Sound to play when reaver explodes */
var		SoundCue		ReaverGibDeathSound;


/** This reaver will instantly gibbing upon death, instead of ragdolling for a couple of seconds. */
var()	bool	bGibStraightAway;

/** Time between ragdoll and gib explosion */
var		config float	ReaverGibbageDelay;

/** PhysicsAsset to use when gibbing. */
var		PhysicsAsset	ReaverRagdollPhysicsAsset;

var ParticleSystem	GoreImpactParticle;

/** Limiter on gib effects */
var int EffectCount;

/** Should this AI spawn blood trail decals **/
var() bool bSpawnBloodTrailDecals;

/** Minimum time between calls to PlayGibEffect. */
var	float TimeBetweenGibEffects;

/** Speed that a gib has to move at to fire effects (ie call PlayGibEffect event). */
var	float GibEffectsSpeedThreshold;

/** If a gib physics body moves further than this from the Actor location we kill it */
var float	GibKillDistance;

var protected const SoundCue	ReaverTakeOffSound;
var protected const SoundCue	ReaverTakeOffSound_Player;
var protected const SoundCue	LegAttackSound;
var protected const SoundCue	ReaverFootstepSound;


/** General material used to control common pawn material parameters (e.g. burning) */
var protected transient MaterialInstanceConstant MIC_PawnMat;
var protected transient MaterialInstanceConstant MIC_PawnMat2;

//////////////////////////////////////////////////////////////////////////
// FLYING

/** Allow the reaver to land when it takes enough damage. */
var bool				bAllowLanding;

/** Set of non-initial flight paths to choose between.  */
var InterpData			FlightPaths[16];

/** Initial flight path to use */
var	InterpData			InitialFlightPath;

/** Current position within flight path. */
var	float				CurrentInterpTime;

/** Current position with landing/take-off transition */
var float				CurrentTransitionTime;

/** Bool indicating if we have completed the initial flight */
var bool				bHasPlayedInitialFlight;

/** If TRUE, destroy the reaver when the next pass ends. */
var bool				bDestroyOnNextPassEnd;

/**
 *	Index of flight currently being executed.
 *	0 indicates InitialFlightPath - >0 indicates FlightPaths[CurrentFlightIndex-1]
 */
var int					CurrentFlightIndex;

/** Incremented each time a new flight is chosen. */
var repnotify int		FlightCount;

//////////////////////////////////////////////////////////////////////////
// ANIM

/** Used to replicate restarting the main bone anim, to keep it in sync between client and server */
var		repnotify int					RestartMainBoneAnimCount;
/** Used to catch when animation  */
var		transient float					LastMainBoneAnimPos;

/** Used to scale how we modify the turning AimOffset based on angular velocity. */
var() float				AngVelAimOffsetScale;

/** Used to control how quickly the AimOffset input changes. */
var() float				AngVelAimOffsetChangeSpeed;

struct native ReaverPlayAnimInfo
{
	var	name		AnimName;

	var	byte		bNewData;
};

/** Struct used to hold state of anim we want to play on reaver - used for replication*/
var		ReaverPlayAnimInfo	AnimRepInfo;

/** Mouth is currently open 0 = not open, 1 = Roar A, 2 = Roar B */
var		repnotify	Byte				bMouthOpen;
/** Range for timer opening mouth again */
var()	config		vector2d			MouthOpenTime;
/** Scalar for reaver being shot in the mouth */
var()	config		float				MouthDamageBoost;
/** Component that takes damage when the mouth is shot to boost damage */
var()				CylinderComponent	MouthComp;

/** Scalar for reaver being shot in the belly */
var()	config		float				BellyDamageBoost;
/** Component that takes damage under belly to boost damage */
var()				CylinderComponent	BellyComp;

/** When we last played a bit anim */
var					float				LastHitAnimTime;
/** Min time between hit anims */
var()				float				MinHitAnimInterval;

//////////////////////////////////////////////////////////////////////////
// LANDING

/** How much 'in front' of reaver a point has to be to land there. */
var() float				MinLandingPointDot;

/** Indicates that ClientFinishedLanding has been called. */
var	bool				bCurrentlyLanding;
/** Indicates that ClientFinishedTakingOff has been called. */
var bool				bCurrentlyTakingOff;

/** If non-empty, reaver can only land at these points. */
var array<ReaverLandingPoint>	AssignedLandingPoints;

var() ScreenShakeStruct	LandingViewShake;
var() float				LandingShakeInnerRadius, LandingShakeOuterRadius, LandingShakeFalloff;


struct native ReaverTransitionInfo
{
	/** If TRUE, reaver is currently landing. */
	var bool		bLanding;
	/** If TRUE, reaver is currently taking off. */
	var bool		bTakingOff;

	/** End of transition spline. */
	var	vector		EndPosition;
	/** Tangent at end of transition spline. */
	var vector		EndTangent;
	/** Rotation at end of transition spline. */
	var rotator		EndRotation;
};

/** Start of transition spline. */
var vector		TransitionStartPosition;
/** Tangent at start of transition spline. */
var vector		TransitionStartTangent;
/** Rotation at start of transition spline. */
var rotator		TransitionStartRotation;

/** Shapes curve for landing */
var() float		LandSplineTangent;
/** Z offset from landing node to interp to */
var() float		LandZOffset;
/** Offset from socket to spawn effect */
var() float		LandEffectZOffset;

/** Information replicated to perform landing or take-off transition. */
var	repnotify ReaverTransitionInfo	TransitionInfo;

/** Indicates reaver is walking on ground - used for replication when client joins while walking. */
var repnotify bool					bWalking;

/** When taking off the time in the path we are moving to. */
var	repnotify float					TakeOffTargetInterpTime;

/** Time taken to reach landing position. */
var()	float			LandingTime;
/** Time take to reach flying path from ground. */
var()	float			TakeOffTime;

/** Point that you are landing at. */
var NavigationPoint LandingPoint;

//////////////////////////////////////////////////////////////////////////
// LEGS

struct native ReaverLegInfo
{
	/** Name of socket indicating end point of leg. */
	var name				TipSocketName;
	/** Default position for foot tip - component reference frame. */
	var vector				DefaultLegPos;
	/** Control for trailing when flying. */
	var SkelControlTrail	TrailControl;
	/** Control for doing CCD IK on legs. */
	var GearSkelCtrl_CCD_IK	IKControl;
	/** Indicates leg target is currently being interpolated. */
	var bool				bStepping;
	/** Current position within interpolation */
	var float				CurrentStepTime;
	/** How long step should take in total. */
	var float				TotalStepTime;
	/** Start position of step spline */
	var vector				StepStartPosition;
	/** End position of step spline */
	var vector				StepEndPosition;
	/** Leans step spline at end of movement.  */
	var float				EndSlope;
	/** Increases spline along Z by extending tangents. */
	var float				ZTangent;
	/** Indicates a leg that has been cut off via chainsaw. */
	var bool				bCutOff;

	structcpptext
	{
		/** Util to calculate position of foot at a point in the step. */
		FVector CalcFootPos(FLOAT Alpha) const;
	}
};

/**
 *	Information about each leg
 *	Order is AL, BL, CL, AR, BR, CR
 */
var()	ReaverLegInfo	LegInfo[6];

/** Blend between walking/flying */
var AnimNodeBlendList	FlyingBlendNode;
/** Node playing walking animation */
var	AnimNodeSequence	WalkAnimNode;
/** Node used to play landing/takeoff anims. */
var AnimNodeSequence	TransitionNode;
/** Slot node used for playing attack anims. */
var	AnimNodeSlot		AttackSlotNode;
/** Random blend for head */
var AnimNodeBlendList	RandHeadBlendNode;
/** Node to blend in pain loop on left/right arms. */
var GearAnim_BlendPerBone	PainLoopBlend;
/** Node that is used for playing main bone overall motion */
var	AnimNodeSequence		MainBoneAnimNode;
/** Node to play anims on entre body */
var	GearAnim_Slot		FullBodySlotNode;

/** Additional 'fake' velocity for tentacles. */
var()	vector			TentacleFakeVelocity;

/** Walking animation rate scale based on linear velocity */
var()	float			WalkAnimLinRateScale;
/** Walking animation rate scale based on angular velocity (Z) */
var()	float			WalkAnimAngRateScale;
/** How long the walk cycle is (how long for all feet to take a step). Used to calculate next foot pos. */
var()	float			WalkCycleDuration;
/** Controls shape of foot curve. */
var()	float			StepZTangent;
/** How much 'ahead' of default position to put foot each step. */
var()	float			StepAdvanceFactor;
/** Increases how much the step shape slopes as it approaches its end. */
var()	float			StepEndSlope;
/** How close player has to get to a leg to force it to take a step (in XY plane). */
var()	float			LegForceStepPlayerDist;
/** How far away the leg will step randomly to avoid the player */
var()	float			LegForceStepPlayerRand;
/** If leg is further than this from DefaultPosition, take a forced step. */
var()	float			LegForceStepErrorDist;


/** Whether to draw step debug info. */
var()	bool			bDrawStepDebug;
/** Whether to draw transition info. */
var()	bool			bDrawTransitionDebug;

//////////////////////////////////////////////////////////////////////////
// ATTACKING

/** If TRUE, don't allow to fire rockets in air. */
var()	bool			bSuppressRockets;
/** If TRUE, reaver will not fire rockets when on the ground */
var()	bool			bSuppressRocketsOnLand;
/** First world time Reaver is allowed to fire rockets */
var		float			AllowRocketFireTime;

/** Center position for right foot attack. */
var()	vector			RAttackPoint;
/** Center position for left foot attack. */
var()	vector			LAttackPoint;
/** How far from attack points player has to be to get stabbed. */
var()	float			LegAttackDist;
/** Sound to play when reaver begins leg attack. */
var()	SoundCue		ReaverAttackSound;
/** Scream sound */
var()	SoundCue		ReaverScreamSound;
/** Pain sound */
var	protected const SoundCue		ReaverPainSound;
var	protected const SoundCue		ReaverPainSound_Player;

/** When reaver last saw the player */
var		float			LastSeenPlayerTime;
/** See player sound */
var()	SoundCue		ReaverSeePlayerSound;
/** Howl sound */
var()	SoundCue		ReaverHowlSound;
/** Sound when the reaver lands */
var()	SoundCue		ReaverLandSound;
var()	SoundCue		ReaverLandSound_Player;
/** Random sound that can be played when the reaver stops for a second */
var()	array<SoundCue>		ReaverRandomSounds;
/** When to play next random sound */
var		float			NextReaverRandomSound;
/** Min/max interval between random sounds */
var()	vector2D		ReaverRandomSoundInterval;
/** Component to play random sounds on (we hold onto it so we can stop them) */
var		AudioComponent	RandomSoundComp;

/** Set when reaver has been hurt enough to leave and go away. Only set on client */
var		bool			bLeavingDefeated;

/** Struct containing info about attack - replicated to client to start attack there. */
struct native ReaverLegAttackInfo
{
	/** State of left attack leg */
	var bool	bLLegAttacking;
	/** State of right attack leg */
	var bool	bRLegAttacking;
	/** Position that leg will stab at. */
	var vector	LegAttackTarget;
};

/** Struct containing info about attack - replicated to client to start attack there. */
var	repnotify ReaverLegAttackInfo	AttackInfo;

/** Time it takes for the leg to raise */
var()	config float	LegRaiseTime;
/** How quickly the attack happens */
var()	float	LegAttackTime;
/** When next attack can happen. */
var float NextAttackTime;
/** Time between leg attacks. */
var() float LegAttackInterval;


/** Template for leg-hitting-ground explosion */
var GearExplosion	StrikeTemplate;
var() config float	StrikeBaseDamage;
var() config float	StrikeDamageFalloff;
var() config float	StrikeDamageRadius;

/** GearPawn class to spawn for Driver */
var class<GearPawn> DriverClass;
/** GearPawn class to spawn for Gunner */
var class<GearPawn> GunnerClass;
/** Offset from main bone for gunner */
var() vector GunnerOffset;

/** reference to the Gunner */
var repnotify GearPawn Gunner;

/** The Gore SkeletalMeh **/
var SkeletalMesh	GoreSkeletalMesh;
/** The Gore PhysAsset **/
var PhysicsAsset	GorePhysicsAsset;
/** Should we spawn soft bits upon death? */
var bool			bSpawnSoftGibs;


var()	float	GoreExplosionRadius;
var()	float	GoreExplosionVel;

var bool			bHasBrokenConstraints;
var bool			bIsGore;
var bool			bGoreSetupForDeath;


/** Skin "heating" data. */
var transient float				CurrentSkinHeat;
/** Min heat value we can go down to.  Useful for when you died by flames and want to leave the pawn "burning a bit" **/
var transient float				CurrentSkinHeatMin;

var() const float				SkinHeatDamagePctToFullyHeat;
var() protected float		    SkinHeatFadeTime;

/** additional damage over time based on skin heat */
var config float HeatDamagePerSecond;
/** player that caused us to be on fire */
var Controller HeatDamageInstigator;
/** fractional heat damage (applied when >= 1) */
var float HeatDamageFraction;
var protected transient bool	bSkipCharFade;
var protected transient bool	bSkipHeatFade;

/** Skin "charring" data. */
var protected transient float	CurrentSkinChar;
/** Min heat value we can go down to.  Useful for when you died by flames and want to leave the pawn "charred a bit" **/
var transient float				CurrentSkinCharMin;

var() protected float			SkinCharFadeTime;


replication
{
	if(bNetInitial && (Role==ROLE_Authority))
		CurrentInterpTime, bWalking, Gunner;

	if(Role == ROLE_Authority)
		FlightPaths, InitialFlightPath, CurrentFlightIndex, FlightCount, RestartMainBoneAnimCount, AnimRepInfo;

	if(Role == ROLE_Authority)
		TransitionInfo, TakeOffTargetInterpTime, AttackInfo, bMouthOpen;
}

cpptext
{
	// Actor interface
	/** Special case interpolation for reavers following matinee paths and landing. */
	virtual void physInterpolating(FLOAT DeltaTime);
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags);

	// Reaver interface
	/** Moves Reaver along a matinee path. */
	void ReaverFlying(FLOAT DeltaTime);
	/** Moves Reaver along spline to its landing point. */
	void ReaverTransitioning(FLOAT DeltaTime);

	/** Util for getting current InterpGroup used by this Reaver. */
	class UInterpGroup* GetCurrentInterpGroup();

	virtual void CheckForErrors();
};



/** Internal. Plays a sound locally (does not replicate), returns the component.  AltPlayerSound is for non-spatialized player-only sounds. */
simulated function protected AudioComponent ReaverPlayLocalSound(SoundCue Sound, optional SoundCue AltPlayerSound, optional AudioComponent AC, optional float FadeInTime)
{
	local GearPC GPC;

	GPC = GearPC(Controller);

	if ( (AltPlayerSound != None) && (GPC != None) && GPC.IsLocalPlayerController() && GPC.IsViewingTarget(self) )
	{
		// use nonspatalized alt sound
		if ( (AC == None) || AC.IsPendingKill() || (AC.SoundCue != AltPlayerSound) )
		{
			AC = CreateAudioComponent(AltPlayerSound, FALSE, TRUE);
		}
		if (AC != None)
		{
			//AC.bUseOwnerLocation = TRUE;
			AC.bAllowSpatialization = FALSE;
			AC.bAutoDestroy = TRUE;
			AC.bStopWhenOwnerDestroyed = TRUE;
			AC.FadeIn(FadeInTime, 1.f);

			return AC;
		}
	}

	if( Sound != None )
	{
		if ( (AC == None) || AC.IsPendingKill() || (AC.SoundCue != Sound) )
		{
			AC = CreateAudioComponent(Sound, FALSE, TRUE);
			if( AC != None )
			{
				AC.bUseOwnerLocation	= TRUE;
				AC.bAutoDestroy			= TRUE;
				AC.bStopWhenOwnerDestroyed = TRUE;
				AttachComponent(AC);
			}
		}
		if (AC != None)
		{
			AC.FadeIn(FadeInTime, 1.f);
			return AC;
		}
	}

	return AC;
}




/** Looks to see if reaver can currently land (is in valid section of flight path). */
native simulated function bool CanLand();
/** Turns on IK for legs and sets their targets to be on ground. */
native simulated function InitLegs();
/** UTIL - Compose first frame of walk anim and output position of leg tips. */
native simulated function CalcDefaultLegPositions();
/** Lift this foot and place it in a good position, based on current lin/ang vel and default position. */
native simulated function TakeStep(INT FootIndex, FLOAT RandomOffset, FLOAT StepTime, FLOAT ZTangentScale);
/** Start moving this foor to  a specific location over Time. Used by TakeStep. */
native simulated function MoveFootToPos(int FootIndex, vector EndPos, float Time, float InZTangent, float InEndSlope);
/** At current location, find path index (within FlightPaths) and time on that path that is closest. */
native simulated function bool FindNearestPathAndTime(out int OutFlightIndex, out float OutInterpTime, out vector OutFlightPos, out rotator OutFlightRot, out vector OutFlightVel);

/** Called after anim tree is initialised - grab pointers to controls/nodes */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local int TentacleIdx;

	if (SkelComp == Mesh)
	{
		LegInfo[0].TrailControl = SkelControlTrail(Mesh.FindSkelControl('AL_Trail'));
		LegInfo[0].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('AL_IK'));

		LegInfo[1].TrailControl = SkelControlTrail(Mesh.FindSkelControl('BL_Trail'));
		LegInfo[1].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('BL_IK'));

		LegInfo[2].TrailControl = SkelControlTrail(Mesh.FindSkelControl('CL_Trail'));
		LegInfo[2].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('CL_IK'));

		LegInfo[3].TrailControl = SkelControlTrail(Mesh.FindSkelControl('AR_Trail'));
		LegInfo[3].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('AR_IK'));

		LegInfo[4].TrailControl = SkelControlTrail(Mesh.FindSkelControl('BR_Trail'));
		LegInfo[4].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('BR_IK'));

		LegInfo[5].TrailControl = SkelControlTrail(Mesh.FindSkelControl('CR_Trail'));
		LegInfo[5].IKControl = GearSkelCtrl_CCD_IK(Mesh.FindSkelControl('CR_IK'));

		FlyingBlendNode = AnimNodeBlendList(Mesh.FindAnimNode('FlyBlend'));
		WalkAnimNode = AnimNodeSequence(Mesh.FindAnimNode('WalkNode'));
		AttackSlotNode = AnimNodeSlot(Mesh.FindAnimNode('AttackSlot'));
		FullBodySlotNode = GearAnim_Slot(Mesh.FindAnimNode('FullBodySlot'));
		PainLoopBlend = GearAnim_BlendPerBone(Mesh.FindAnimNode('PainLoopBlend'));
		TransitionNode = AnimNodeSequence(Mesh.FindAnimNode('Transition'));
		MainBoneAnimNode = AnimNodeSequence(Mesh.FindAnimNode('MainBoneAnim'));
		RandHeadBlendNode = AnimNodeBlendList(Mesh.FindAnimNode('RandHeadBlend'));

		// Make sure tree is in correct state
		FlyingBlendNode.SetActiveChild(0, 0.0);
		WalkAnimNode.StopAnim();

		// Turn on trail controls and off ik
		for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
		{
			LegInfo[TentacleIdx].TrailControl.SetSkelControlActive(TRUE);
			LegInfo[TentacleIdx].TrailControl.FakeVelocity = TentacleFakeVelocity;
			LegInfo[TentacleIdx].IKControl.SetSkelControlActive(FALSE);
		}

		//CalcDefaultLegPositions();
	}
}

simulated event PostBeginPlay()
{
	local GearAI GunnerController;

	Super.PostBeginPlay();

	if (Role == ROLE_Authority)
	{
		Health = DefaultReaverHealth;
		HealthMax = DefaultReaverHealth;

		Driver = Spawn(DriverClass,,,,,, true);
		if (Driver != None)
		{
			Driver.Mesh.SetShadowParent(Mesh);
			WorldInfo.Game.SetPlayerDefaults(Driver);
			GearPawn(Driver).bCanDBNO = false;
			bDriving = true;
			Driver.StartDriving(self);
			//@note: AI for driver, and thus for this vehicle, is spawned by whatever is creating us (AI factory or whatever)
		}

		if(GunnerClass != None)
		{
			Gunner = Spawn(GunnerClass,,,,,, true);
			if( Gunner != None )
			{
				Gunner.Mesh.SetShadowParent(Mesh);
				Gunner.AddDefaultInventory();
				Gunner.PeripheralVision = -0.3f;
				Gunner.bCanDBNO = false;
				GunnerController = Spawn( class'GearAI_ReaverGunner' );
				GunnerController.SetTeam(1);
				GunnerController.Possess(Gunner, false);
				AttachGunner();
			}
		}
	}

	FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player, FlyingSoundLoopAC, 0.5f);

	Mesh.AttachComponentToSocket( MouthComp, 'EatStuff' );
	Mesh.AttachComponentToSocket( BellyComp, 'InkTrail' );

	MIC_PawnMat = Mesh.CreateAndSetMaterialInstanceConstant(0);
	MIC_PawnMat2 = Mesh.CreateAndSetMaterialInstanceConstant(1);
}

/** attaches the Gunner pawn to the appropriate place */
simulated function AttachGunner()
{
	Gunner.SetPhysics(PHYS_None);
	Gunner.SetBase(self,, Mesh, 'b_Main');
	Gunner.SetRelativeLocation(GunnerOffset);
	Gunner.SetRelativeRotation(rot(0,16384,0));
	Gunner.bDisableMeshTranslationChanges = TRUE;
}

/** Called when vars change via network. */
simulated event ReplicatedEvent( name VarName )
{
	if(VarName == 'TransitionInfo')
	{
		if(TransitionInfo.bLanding)
		{
			ClientBeginLandingAnim();
		}
		else if(bCurrentlyLanding)
		{
			ClientFinishedLanding();
		}

		if(TransitionInfo.bTakingOff)
		{
			ClientBeginTakeOffAnim();
		}
		else if(bCurrentlyTakingOff)
		{
			ClientFinishedTakingOff();
		}
	}
	else if(VarName == 'AttackInfo')
	{
		if(AttackInfo.bLLegAttacking)
		{
			LegAttackRaise(vect(0,0,0), TRUE);
		}
		else if(AttackInfo.bRLegAttacking)
		{
			LegAttackRaise(vect(0,0,0), FALSE);
		}
	}
	else if(VarName =='FlightCount')
	{
		//`log("FLIGHT RESET"@FlightCount);
		CurrentInterpTime = 0.0;
	}
	else if(VarName == 'TakeOffTargetInterpTime')
	{
		CurrentInterpTime = TakeOffTargetInterpTime;
	}
	else if(VarName == 'bWalking')
	{
		if(bWalking)
		{
			ClientForceWalking();
		}
	}
	else if (VarName == 'Gunner')
	{
		if (Gunner != None)
		{
			AttachGunner();
		}
	}
	else if(VarName == 'RestartMainBoneAnimCount')
	{
		//`log("REAVER CLIENT RESTART ANIM");
		MainBoneAnimNode.PlayAnim(TRUE, 1.0, 0.0);
	}
	else if( VarName == 'bMouthOpen' )
	{
		if( bMouthOpen != 0 )
		{
			PlayMouthOpenAnim();
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function bool CanEnterVehicle(Pawn P)
{
	// Never allow humans to drive reaver
	if(GearPC(P.Controller) != None)
	{
		return FALSE;
	}

	return Super.CanEnterVehicle(P);
}

simulated function SitDriver(GearPawn WP, int SeatIndex)
{
	Super.SitDriver(WP, SeatIndex);

	// leave driver shootable
	if (SeatIndex == 0 && WP.IsA('GearPawn_LocustBase'))
	{
		WP.SetCollision(true, false);
	}
}

function DriverDied(class<DamageType> DamageType)
{
	if( Driver != None )
	{
		Driver.StopDriving( Self );
		Driver = None;
	}
}

function GunnerDied(GearPawn GP)
{
	Gunner = None;
}

simulated function Tick(float DeltaSeconds)
{
	local float MainBoneAnimPos;
	//local SoundCue RandSound;

	Super.Tick(DeltaSeconds);

	// Update anim rate
	if(Physics == PHYS_RigidBody)
	{
		WalkAnimNode.Rate = FClamp((VSize(Velocity) * WalkAnimLinRateScale) + (AngularVelocity.Z * WalkAnimAngRateScale), -1.5, 1.5);
	}
	else
	{
		WalkAnimNode.Rate = 1.0;
	}

	`if(`notdefined(FINAL_RELEASE))
	if(bDrawStepDebug)
	{
		DrawDebugBox(Location + (LAttackPoint >> Rotation), vect(10,10,10), 0, 0, 255);
		DrawDebugBox(Location + (RAttackPoint >> Rotation), vect(10,10,10), 255, 0, 0);
	}
	`endif

	if(Role == ROLE_Authority)
	{
		// See if the root anim time is before last frame - if so we have looped - so tell clients
		MainBoneAnimPos = MainBoneAnimNode.CurrentTime;
		if(MainBoneAnimPos < LastMainBoneAnimPos)
		{
			//`log("REAVER ROOT ANIM LOOP");
			RestartMainBoneAnimCount++;
		}

		// Save position
		LastMainBoneAnimPos = MainBoneAnimPos;
	}

	// See if there is an animation to play
	if(AnimRepInfo.bNewData == 1)
	{
		FullBodySlotNode.PlayCustomAnim(AnimRepInfo.AnimName, 1.0, 0.2, 0.3, FALSE, TRUE);
		AnimRepInfo.bNewData = 0;
	}

	// Do random sounds if walking and alive and it's time
	//if((Physics == PHYS_RigidBody) && (Health > 0) && (WorldInfo.TimeSeconds > NextReaverRandomSound))
	//{
	//	// Skip random sound if attacking
	//	if(!AttackInfo.bLLegAttacking && !AttackInfo.bRLegAttacking)
	//	{
	//		RandSound = ReaverRandomSounds[Rand(ReaverRandomSounds.length)];
	//		RandomSoundComp.Stop();
	//		RandomSoundComp.SoundCue = RandSound;
	//		RandomSoundComp.Play();
	//	}

	//	// Set next time to do one
	//	NextReaverRandomSound = WorldInfo.TimeSeconds + (ReaverRandomSoundInterval.X + (FRand() * (ReaverRandomSoundInterval.Y - ReaverRandomSoundInterval.X)));
	//}

	if (bRestartFlyingLoop)
	{
		if(FlyingSoundLoopAC != None)
		{
			FlyingSoundLoopAC.FadeOut(0.2f, 0.f);
		}
		FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player,, 0.2f);
	}

	FadeSkinEffects( DeltaSeconds );
}

simulated function UpdateLookSteerStatus()
{
	bUsingLookSteer = FALSE;
}

/** Util for choosing hit anim */
simulated function name GetHitAnimName(vector HitDir, class<DamageType> DamageType)
{
	local float DmgHeading;
	local bool bExplosive;

	// First check for HOD
	if(ClassIsChildOf(DamageType, class'GearGame.GDT_HOD'))
	{
		return 'HitReaction_HOD';
	}

	// Check we haven't played hit anim too recently
	if(WorldInfo.TimeSeconds < LastHitAnimTime + MinHitAnimInterval)
	{
		return '';
	}

	// see if its explosive damage
	bExplosive = ClassIsChildOf(DamageType, class'GearGame.GDT_Explosive');

	// for no explosive damage - only play hit with some chance
	if(!bExplosive)
	{
		if(FRand() > 0.1)
		{
			return '';
		}
	}

	LastHitAnimTime = WorldInfo.TimeSeconds;

	// Get hit heading in local space, pointing towards shot dir
	DmgHeading = GetHeadingAngle(-1.0 * (HitDir << Rotation));
	if(DmgHeading < -0.75*PI || DmgHeading > 0.75*PI)
	{
		return (bExplosive) ? 'HitReaction_Big_Back' : 'HitReaction_Back';
	}
	else if(DmgHeading < -0.25*PI)
	{
		return (bExplosive) ? 'HitReaction_Big_Left' : 'HitReaction_Left';
	}
	else if(DmgHeading > 0.25*PI)
	{
		return (bExplosive) ? 'HitReaction_Big_Right' : 'HitReaction_Right';
	}
	else
	{
		return (bExplosive) ? 'HitReaction_Big_Front' : 'HitReaction_Front';
	}
}

/** Used to trigger landing when too damaged. */
simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local name HitAnimName;
// Allow reavers to be killed like normal (TTP 93672)
/*	// do no damage when leaving defeated
	if(bLeavingDefeated)
	{
		return;
	}*/

	// TAke more damage when flying
	if(Physics == PHYS_Interpolating)
	{
		Damage = (Damage * FlyingTakeDamageScale);
	}
	else
	// Take more damage when shot in the mouth
	if( bMouthOpen != 0 && HitInfo.HitComponent == MouthComp )
	{
		Damage *= MouthDamageBoost;
	}
	else
	// Take more damage when shot in the belly
	if( HitInfo.HitComponent == BellyComp )
	{
		Damage *= BellyDamageBoost;
	}

	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	if(bAllowLanding && (Health < HealthForLanding))
	{
		OnReaverLand(None);
	}

	// If walking
	if(Physics == PHYS_RigidBody && Health > 0)
	{
		HitAnimName = GetHitAnimName(Momentum, DamageType);
		if(HitAnimName != '')
		{
			AnimRepInfo.AnimName = HitAnimName;
			AnimRepInfo.bNewData = 1;
		}
	}

	if (TimeSince(LastPainTime) > 3.f)
	{
		ReaverPlayLocalSound(ReaverPainSound, ReaverPainSound_Player);
		LastPainTime = WorldInfo.TimeSeconds;
	}
}

/** Called when reaver has 'left the field' and flown off - destroy. */
event ReaverFlownOff()
{
	if(LifeSpan == 0.0)
	{
		LifeSpan = 0.01;
		if(Driver != None)
		{
			Driver.LifeSpan = 0.01;
		}

		if(Gunner != None)
		{
			Gunner.LifeSpan = 0.01;
		}
	}
}

function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local GearPC PC;
	if( Super.Died(Killer, DamageType, HitLocation) )
	{
		PC = GearPC(Killer);
		if (PC != None)
		{
			PC.UpdateSeriously();
		}
		if( Driver != None )
		{
			Driver.Died(Killer, DamageType, HitLocation);
			Driver = None;
		}
		if( Gunner != None )
		{
			Gunner.Died(Killer, DamageType, HitLocation);
			Gunner = None;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

simulated function BlowupVehicle(Controller Killer)
{
	// Kill rider
	if( Gunner != None )
	{
		Gunner.Died(Killer, HitDamageType, Location);
		Gunner = None;
	}

	// kill driver
	if( Driver != None )
	{
		Driver.Died(Killer, HitDamageType, Location);
		Driver = None;
	}

	Super.BlowupVehicle(Killer);
}

simulated event Destroyed()
{
	Super.Destroyed();

	if(Driver != None)
	{
		Driver.LifeSpan = 0.01;
		Driver = None;
	}

	if(Gunner != None)
	{
		Gunner.LifeSpan = 0.01;
		Gunner = None;
	}
}

simulated event TornOff()
{
	// assume dead if bTearOff
	if ( !bPlayedDeath )
	{
		PlayDying(HitDamageType,TakeHitLocation);
	}
}


function PlayHit(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo)
{
	local class<GDT_Fire> Fire;

	Fire = class<GDT_Fire>(DamageType);
	if( Fire != None )
	{
		Fire.static.HandleFireDamageForReavers( self, Damage );
	}

	Super.PlayHit(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);
}


/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin(float HeatIncrement)
{
	CurrentSkinHeat += HeatIncrement;
	CurrentSkinHeat = FClamp(CurrentSkinHeat, 0.f, 1.f);
	MIC_PawnMat.SetScalarParameterValue('Heat', CurrentSkinHeat);
	MIC_PawnMat2.SetScalarParameterValue('Heat', CurrentSkinHeat);


	if (HeatIncrement > 0.f)
	{
		bSkipHeatFade = TRUE;
	}
}

/** Char skin to the given absolute char amount [0..1].  Charring does not diminish over time. */
simulated function CharSkin(float CharIncrement)
{
	CurrentSkinChar += CharIncrement;
	CurrentSkinChar = FClamp(CurrentSkinChar, 0.f, 1.f);
	MIC_PawnMat.SetScalarParameterValue('Burn', CurrentSkinChar);
	MIC_PawnMat2.SetScalarParameterValue('Burn', CurrentSkinChar);

	if (CharIncrement > 0.f)
	{
		bSkipCharFade = TRUE;
	}
}


final simulated function FadeSkinEffects(float DeltaTime)
{
	// fade skin heat
	if ( (CurrentSkinHeat > CurrentSkinHeatMin) && !bSkipHeatFade && (SkinHeatFadeTime > 0.f) )
	{
		HeatSkin(-DeltaTime / SkinHeatFadeTime);
	}
	bSkipHeatFade = FALSE;


	// fade charring
	if ( (CurrentSkinChar > CurrentSkinCharMin) && !bSkipCharFade && (SkinCharFadeTime > 0.f) )
	{
		CharSkin(-DeltaTime / SkinCharFadeTime);
	}
	bSkipCharFade = FALSE;
}


simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local class<GearDamageType>	GearDamageType;
	local bool					bExplosiveDamage;
	local Emitter				ExplodeEmitter;
	local int					TentacleIdx;
	local name					DyingAnimName;
	local GearVehicleSimHover	HoverObj;

// 	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "DamageType:" @ DamageType);
// 	ScriptTrace();

	GearDamageType = class<GearDamageType>(DamageType);
	// ensure a default
	if( GearDamageType == None )
	{
`if(`notdefined(FINAL_RELEASE))
		`LogExt("Invalid gear damage type!"@damageType);
		ScriptTrace();
`endif
		GearDamageType = class'GearDamageType';
	}

	bCanTeleport		= FALSE;
	bReplicateMovement	= FALSE;
	bTearOff			= TRUE;
	bPlayedDeath		= TRUE;
	bIgnoreForces		= FALSE;

	// Increase limit speed when dead
	AirSpeed = 5000.0;

	HoverObj = GearVehicleSimHover(SimObj);
	HoverObj.LongDamping = 0.0;
	HoverObj.LatDamping = 0.0;
	HoverObj.StabilizationForceMultiplier = 0.0;

	// Handle case of reaver attached to moving thing - its velocity will not be correct
	if(Base != None)
	{
		Velocity = Base.Velocity;
		if(Base.Base != None)
		{
			Velocity += Base.Base.Velocity;
		}
	}

	// Disable collision
	SetCollision(TRUE, FALSE, TRUE);
	Mesh.SetTraceBlocking(TRUE, FALSE);

	// Make sure we have a valid TearOffMomentum
	if( TearOffMomentum == vect(0,0,0) )
	{
		TearOffMomentum = Vector(Rotation);
	}

	RandomSoundComp.Stop(); // Stop random sound
	FlyingSoundLoopAC.Stop(); // Stop flying sound

	ReaverPlayLocalSound(ReaverDeathSound);

	bExplosiveDamage = ClassIsChildOf(DamageType, class'GDT_Explosive');

	// If non-explosive death - ragdoll, then gib
	if( !bExplosiveDamage && !bGibStraightAway )
	{
		// Spawn emitter
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(ReaverPreExplodeEffect, Location, rot(0,0,0));
		ExplodeEmitter.ParticleSystemComponent.ActivateSystem();
		ExplodeEmitter.SetBase( self,, Mesh, 'PreExplosion' );
		// To guarantee the glow emitter stops when the explosion goes off, we have it hide itself after the delay
		ExplodeEmitter.SetTimer( ReaverGibbageDelay, FALSE, nameof(ExplodeEmitter.HideSelf) );

		DyingAnimName = (Physics == PHYS_Interpolating) ? 'Death_Flying_Twitch' : 'Death_Standing_Twitch';

		SetPhysics(PHYS_RigidBody);
		InitVehicleRagdoll(Mesh.SkeletalMesh, ReaverRagdollPhysicsAsset, vect(0,0,0), FALSE);

		// Play an animation
		FullBodySlotNode.PlayCustomAnim(DyingAnimName, 1.0, 0.2, -1.0, FALSE, TRUE);

		SetTimer( ReaverGibbageDelay, FALSE, nameof(ReaverGibbage) );
	}
	else
	{
		ReaverGibbage();
	}

	// Turn off controls
	for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
	{
		LegInfo[TentacleIdx].IKControl.SetSkelControlActive(FALSE);
		LegInfo[TentacleIdx].TrailControl.SetSkelControlActive(FALSE);
	}
}

simulated function ReaverGibbage()
{
	local Emitter ExplodeEmitter;

	InitVehicleRagdoll(GoreSkeletalMesh, GorePhysicsAsset, vect(0,0,0), TRUE);

	// Let pawn kick gibs around
	Mesh.SetRBChannel(RBCC_Default);

	bIsGore = TRUE;

	// Freeze the light environment state at the state when the pawn was gibbed; this prevents the light environment being computed
	// for the large aggregate bounds of the gibs, which usually doesn't give good results.
	// we need to wait a number of seconds here so we do not capture the explosion light
	// NOTE: this can look bad when the gib is near a bright colored light and then is kicked out away from it into darkness
	SetTimer( 5.0f, FALSE, nameof(SetLightEnvironmentToNotBeDynamic) );

	//DrawDebugSphere(HitLocation, GoreExplosionRadius, 10, 255, 0, 0, TRUE);
	Mesh.AddRadialImpulse(Location, GoreExplosionRadius, GoreExplosionVel, RIF_Linear, TRUE);

	// Spawn emitter
	if( WorldInfo.GRI.ShouldShowGore() )
	{
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(ReaverExplodeEffect, Location, rot(0,0,0));
	}
	else
	{
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(ReaverExplodeNoGoreEffect, Location, rot(0,0,0));
	}

	ExplodeEmitter.ParticleSystemComponent.ActivateSystem();

	// sound
	ReaverPlayLocalSound(ReaverGibDeathSound);

	// Set up cleanup
	if( Role == ROLE_Authority )
	{
		SetTimer( 5.0, FALSE, nameof(RemoveIfNotSeen) );
		SetTimer( 15.0, FALSE, nameof(ForceRemove) );
	}

	//`log( "blood " $ HitLoc @ SquaredForce @ EffectCount );
	SpawnABloodTrail_GibImpact( Location ); // this is too small
}

/** This will turn "off" the light environment so it will no longer update **/
simulated final function SetLightEnvironmentToNotBeDynamic()
{
	LightEnvironment.bDynamic = FALSE;
}


//@ todo this needs to be moved to be based off the skel mesh maybe?
simulated event PlayGibEffect(vector HitLoc, vector HitNorm, float SquaredForce)
{
	local Emitter ImpactEmitter;

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLoc, 2000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return;
	}

	// this is to make it so we don't spawn more than 2 decals per time period (aka when you kick a body)
	// around all of it's pieces are moving along the ground and spamming out blood
	if( ++EffectCount > 2 )
	{
		return;
	}

	if(SquaredForce < 17500.0f)
		PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkSmallCue',false,false,false,HitLoc);
	else if(SquaredForce < 20000.0f)
		PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkMediumCue',false,false,false,HitLoc);
	else if(SquaredForce < 40000.0f)
		PlaySound(SoundCue'Foley_Flesh.Flesh.PhysicsFleshThumpCue',false,false,false,HitLoc);
	else if(SquaredForce < 70000.0f)
		PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkLargeCue',false,false,false,HitLoc);
	else if(SquaredForce < 130000.0f)
		PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkHugeCue',false,false,false,HitLoc);
	else // when kicking bodies around play this
		PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkMediumCue',false,false,false,HitLoc);


	if( WorldInfo.GRI.ShouldShowGore() )
	{
		ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( GoreImpactParticle, HitLoc, rotator(HitNorm) );
		ImpactEmitter.ParticleSystemComponent.ActivateSystem();

		//`log( "blood " $ HitLoc @ SquaredForce @ EffectCount );
		SpawnABloodTrail_GibbedReaver( HitLoc );
	}

	if(!IsTimerActive('ResetEffects'))
	{
		SetTimer( 0.1, FALSE, nameof(ResetEffects) );
	}
}

final simulated function ResetEffects()
{
	EffectCount = 0;
}



/** This is the Delegate for the Tracing Policy for LeaveADecal **/
simulated delegate DecalTrace( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	`log( "DecalTrace Delegate was not set" );
	ScriptTrace();
}

/** This is the Delegate for the Decal Choice Policy for LeaveADecal **/
simulated delegate DecalChoice( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	`log( "DecalChoice Delegate was not set" );
	ScriptTrace();
}

/** This is the Delegate for the MITV Params Policy for LeaveADecal **/
simulated delegate DecalTimeVaryingParams( out MaterialInstance MI_Decal )
{
	`log( "DecalTimeVaryingParams Delegate was not set" );
	ScriptTrace();
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated final function SpawnABloodTrail_GibImpact( vector HitLoc )
{
	if( ( WorldInfo.GRI.ShouldShowGore() )
		&& ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLoc, 3000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		&& bSpawnBloodTrailDecals
		)
	{
		LeaveADecal( BloodDecalTrace_TraceGibImpact, BloodDecalChoice_GibImpact, BloodDecalTimeVaryingParams_Default, HitLoc );
	}
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated final function SpawnABloodTrail_GibbedReaver( vector HitLoc )
{
	if( ( WorldInfo.GRI.ShouldShowGore() )
		&& ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLoc, 3000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		&& bSpawnBloodTrailDecals
		)
	{
		LeaveADecal( BloodDecalTrace_TraceGibImpact, BloodDecalChoice_GibExplode_Ground, BloodDecalTimeVaryingParams_Default, HitLoc );
	}
}

// @ todo pass in a hit normal so we can get hits on walls also
simulated final function BloodDecalTrace_TraceGibImpact( out vector out_TraceStart, out vector out_TraceDest, const float RandomOffsetRadius, optional vector ForceStartLocation )
{
	out_TraceStart = ForceStartLocation + ( Vect(0,0,15));
	out_TraceDest =  out_TraceStart - ( Vect(0,0,100));
}

/**
 * This will choose the Decals to use for the generic blood splatter
 **/
simulated final function BloodDecalChoice_GibImpact( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDTT_GibImpact, WorldInfo );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

simulated final function BloodDecalChoice_GibExplode_Ground( const out TraceHitInfo HitInfo, out float out_DecalRotation, out DecalData out_DecalData )
{
	out_DecalData = class'GearPhysicalMaterialProperty'.static.GetBloodDecalData( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( Hitinfo ), GDDT_ReaverGibExplode, WorldInfo );
	//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_6' );

	if (out_DecalData.bRandomizeRotation)
	{
		out_DecalRotation = FRand() * 360.0f;
	}
}

/**
 * This will set the params for typical blood decals.
 **/
simulated final function BloodDecalTimeVaryingParams_Default( out MaterialInstance out_MI_Decal )
{
	MaterialInstanceTimeVarying(out_MI_Decal).SetDuration( 45.0f );
}

/**
 * This function takes a number of delegates that are used to control the core behavior of leaving a decal.
 *
 * DecalTrace:  What to trace against to get the Actor that has been hit.  (e.g. down from some location / out from a gun / etc.)
 * DecalChoice:  Which decal to actually use.  This could involve getting it from some list or determining based on LOD, etc.
 * TimeVaryingParms:  What params to modifiy over time (for fading in/out, having an over time effect, etc.)
 *
 * To use this just pass in the delegates/policies you want to use and voila a decal shall be placed into the world.
 *
 * @todo move this to some global object that exists so we can use the same code from anywhere
 **/
simulated final function LeaveADecal( delegate<DecalTrace> DecalTraceFunc, delegate<DecalChoice> DecalChoiceFunc, delegate<DecalTimeVaryingParams> DecalTimeVaryingParamsFunc, optional vector ForceStartLocation )
{
	local GearDecal GD;
	local DecalData DecalData;
	local float RotationToUseForDecal;

	local Actor TraceActor;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;

	local float RandomScale;


	if (!bSpawnBloodTrailDecals)
	{
		return;
	}


	DecalChoice = DecalChoiceFunc;
	DecalChoice( HitInfo, RotationToUseForDecal, DecalData );
	DecalChoice = None;

	if( DecalData.DecalMaterial == None )
	{
		//`warn( "DecalMaterial was none!!" );
		//ScriptTrace();
		return;
	}

	DecalTrace = DecalTraceFunc;
	DecalTrace( TraceStart, TraceDest, DecalData.RandomRadiusOffset, ForceStartLocation );
	DecalTrace = None;

	TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, FALSE, TraceExtent, HitInfo, TRACEFLAG_PhysicsVolumes );

	`if(`notdefined(FINAL_RELEASE))
	if( FALSE )
	{
		DrawDebugLine( TraceStart, TraceDest , 255, 1, 1, TRUE );
		`log( "About to spawn a decal: " $ DecalData.DecalMaterial );
		ScriptTrace();
	}
	`endif

	if( TraceActor != None )
	{
		//`log( "Blood DecalMaterial: " $ DecalData.DecalMaterial );
		//`log( Material(MaterialInstanceTimeVarying(DecalData.DecalMaterial).Parent).bUsedWithDecals );

		// @todo this needs to be cached

		RandomScale = GetRangeValueByPct( DecalData.RandomScalingRange, FRand() );
		DecalData.Width *= RandomScale;
		DecalData.Height *= RandomScale;

		GD = GearGRI(WorldInfo.GRI).GOP.GetDecal_Blood( out_HitLocation );
		if( GD != none )
		{
			GD.MITV_Decal.SetParent( DecalData.DecalMaterial );
			// @TODO:  pass in a struct with the decal params you want to use
			WorldInfo.MyDecalManager.SetDecalParameters( 
				GD, 
				GD.MITV_Decal, 
				out_HitLocation, 
				rotator(-out_HitNormal), 
				DecalData.Width, 
				DecalData.Height, 
				Max(DecalData.Thickness,class'GearDecal'.default.MinExplosionThickness), 
				!DecalData.ClipDecalsUsingFastPath, 
				RotationToUseForDecal, 
				HitInfo.HitComponent, 
				TRUE, 
				FALSE, 
				HitInfo.BoneName, 
				INDEX_NONE, 
				INDEX_NONE, 
				999999.0 /*DecalData.LifeSpan*/, 
				INDEX_NONE, 
				class'GearDecal'.default.DepthBias,
				DecalData.BlendRange 
				);
			GearGRI(WorldInfo.GRI).GOP.AttachComponent(GD);

			//LocationOfLastBloodTrail = out_HitLocation;

			GD.MITV_Decal.SetDuration( 999999.0 /*DecalData.LifeSpan*/ );

			//DecalTimeVaryingParams = DecalTimeVaryingParamsFunc;
			//DecalTimeVaryingParams( GD.MITV_Decal );
			//DecalTimeVaryingParams = None;

			`if(`notdefined(FINAL_RELEASE))
				if( FALSE )
				{
					`log( "  SPAWNED " );
					//FlushPersistentDebugLines();
					DrawDebugCoordinateSystem( out_HitLocation, rotator(-out_HitNormal), 3.0f, TRUE );
				}
				`endif
		}
	}
}


/** Will clean up if not seen - otherwise will wait 0.5 secs and try again */
function RemoveIfNotSeen()
{
	// Not seen - clean up now
	if(Mesh.LastRenderTime < WorldInfo.TimeSeconds - 0.5)
	{
		LifeSpan=0.01;
		ClearTimer('ForceRemove');
	}
	// Is being seen - check again in 0.5 secs
	else
	{
		SetTimer( 0.5, FALSE, nameof(RemoveIfNotSeen) );
	}
}

/** Will force actor to be destroyed */
function ForceRemove()
{
	LifeSpan=0.01;
	ClearTimer('RemoveIfNotSeen');
}

/** Handler for AssignReaverFlights action, that tells Reaver about its flight paths. */
simulated function OnAssignReaverFlights(SeqAct_AssignReaverFlights Action)
{
	local Array<InterpData> IDataArray;
	local Array<Object> LandingArray;
	local ReaverLandingPoint LandingP;
	local int idx, FlightIdx;
	local InterpData IData;
	local GearVWeap_ReaverCannonBase Cannon;

	// Clear current flight info.
	InitialFlightPath = None;
	for(FlightIdx=0; FlightIdx<16; FlightIdx++)
	{
		FlightPaths[FlightIdx] = None;
	}

	// Get initial flight data
	Action.GetInterpDataVars(IDataArray,"InitialFlight");
	for( idx=0; idx<IDataArray.Length; idx++ )
	{
		IData = IDataArray[idx];
		if(IData != None)
		{
			InitialFlightPath = IData;
			break;
		}
	}

	// Get other flight data.
	Action.GetInterpDataVars(IDataArray,"OtherFlights");
	FlightIdx = 0;
	for( idx=0; idx<IDataArray.Length; idx++ )
	{
		IData = IDataArray[idx];
		if(IData != None)
		{
			FlightPaths[FlightIdx] = IData;
			FlightIdx++;
		}
	}

	// Get landing points.
	AssignedLandingPoints.length = 0; // empty existing array
	Action.GetObjectVars(LandingArray,"LandingPoints");
	for( idx=0; idx<LandingArray.Length; idx++ )
	{
		LandingP = ReaverLandingPoint(LandingArray[idx]);
		AssignedLandingPoints[AssignedLandingPoints.length] = LandingP;
	}

	// Set bAllowLanding from kismet action
	bAllowLanding = Action.bAllowLanding;
	// Set bSuppressRockets from kismet action
	bSuppressRockets = Action.bSuppressRockets;
	// Set bSpawnSoftGibs
	bSpawnSoftGibs=!Action.bDisableSoftGibs;
	// Set bRagdollUponDeath
	bGibStraightAway = Action.bGibStraightAway;

	// Set bDestroyOnNextPassEnd if we fired the STOP input
	if(Action.InputLinks[1].bHasImpulse)
	{
		bDestroyOnNextPassEnd = TRUE;
	}

	Cannon = GearVWeap_ReaverCannonBase(Weapon);
	if( Cannon != None )
	{
		Cannon.MinTimeBetweenRockets = Action.MinTimeBetweenRockets;
	}

}

/** Called when one pass ends and another is about to begin. */
event ReaverPassEnded()
{
	local int i;
	local SeqEvt_ReaverEndedPass PassEvt;

	// Fire 'reaver pass ended' events
	for (i=0; i<GeneratedEvents.Length; i++)
	{
		PassEvt = SeqEvt_ReaverEndedPass(GeneratedEvents[i]);
		if(PassEvt != None)
		{
			PassEvt.CheckActivate(self, self, FALSE);
		}
	}
}

/** Get the set of possible landing points. */
simulated function array<ReaverLandingPoint> GetLandingPointSet()
{
	local ReaverLandingPoint P;
	local array<ReaverLandingPoint> AllLandingPoints;

	// If we have an assigned set - only look in those
	if(AssignedLandingPoints.length > 0)
	{
		return AssignedLandingPoints;
	}
	// If not, all landing points are possible.
	else
	{
		foreach WorldInfo.AllNavigationPoints(class'ReaverLandingPoint',P)
		{
			AllLandingPoints[AllLandingPoints.length] = P;
		}
		return AllLandingPoints;
	}
}

/** Find desired point to land. */
simulated function ReaverLandingPoint ChooseLandingPoint()
{
	local array<ReaverLandingPoint> LandingPoints;
	local ReaverLandingPoint NearestP, P;
	local Pawn OtherPawn;
	local float NearestDist, Dist;
	local vector ForwardDir, ToLandingDir;
	local bool bInUse;
	local int PIdx;

	if(!CanLand())
	{
		return None;
	}

	ForwardDir = vector(Rotation);
	NearestDist = 100000000000.0;

	// Iterate over set of possible landing points
	LandingPoints = GetLandingPointSet();
	for(PIdx=0; PIdx<LandingPoints.length; PIdx++)
	{
		P = LandingPoints[PIdx];

		// Find direction and distance of this landing point
		ToLandingDir = P.Location - Location;
		Dist = VSize(P.Location - Location);
		ToLandingDir /= Dist;

		// See if it is ahead of me
		if((ToLandingDir Dot ForwardDir) > MinLandingPointDot)
		{
			// If so, see if its closer than what I have so far
			if(Dist < NearestDist)
			{
				// make sure there aren't any Pawns standing on the landing point
				// nor other Reavers already landing there
				bInUse = false;
				foreach WorldInfo.AllPawns(class'Pawn', OtherPawn)
				{
					if ( OtherPawn.Anchor == P ||
						(OtherPawn.IsA('Vehicle_Reaver_Base') && Vehicle_Reaver_Base(OtherPawn).LandingPoint == P) )
					{
						bInUse = true;
						break;
					}
				}
				if (!bInUse)
				{
					NearestDist = Dist;
					NearestP = P;
				}
			}
		}
	}

	return NearestP;
}

/** Go ahead, find a landing spot and complete the landing. */
function ServerBeginLanding(NavigationPoint UsePoint)
{
	if(UsePoint != None)
	{
		LandingPoint = UsePoint;

		TransitionInfo.bLanding = TRUE;

		TransitionInfo.EndPosition = LandingPoint.Location + (LandZOffset * vect(0,0,1));
		TransitionInfo.EndTangent = LandSplineTangent * vect(0,0,1);
		TransitionInfo.EndRotation = Rotation;

		// Do anim stuff
		ClientBeginLandingAnim();
	}
}

/** Try and begin to take off. Returns if successful. (server only) */
function bool ServerBeginTakeOff()
{
	local int NearestFlightIndex;
	local float NearestFlightInterpTime;
	local vector NearestFlightPos, NearestFlightVel;
	local rotator NearestFlightRot;

	if( FindNearestPathAndTime(NearestFlightIndex, NearestFlightInterpTime, NearestFlightPos, NearestFlightRot, NearestFlightVel) )
	{
		TransitionInfo.bTakingOff = TRUE;
		bWalking = FALSE;
		// stop reaver from firing rockets after it leaves
		bSuppressRockets = TRUE;

		SetPhysics(PHYS_Interpolating);

		TransitionInfo.EndPosition = NearestFlightPos;
		TransitionInfo.EndTangent = NearestFlightVel;
		TransitionInfo.EndRotation = NearestFlightRot;

		TakeOffTargetInterpTime = NearestFlightInterpTime;
		CurrentInterpTime = NearestFlightInterpTime;
		CurrentFlightIndex = NearestFlightIndex + 1;

		ClientBeginTakeOffAnim();

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** Function that will keep trying to land. */
function TryLanding()
{
	local ReaverLandingPoint LP;

	LP = ChooseLandingPoint();
	if(LP != None)
	{
		ServerBeginLanding(LP);
		ClearTimer('TryLanding');
	}
}

/** Handler for ReaverLand function, that makes reaver land. */
function OnReaverLand(SeqAct_ReaverLand Action)
{
	local ReaverLandingPoint LP;
	local GearVWeap_ReaverCannonBase Cannon;

	if( Action != None && !Action.bTakeOff )
	{
		bSuppressRocketsOnLand = Action.bSuppressRocketsOnLand;
	}

	// Only do things if flying and not already landing (and alive!)
	if(!TransitionInfo.bTakingOff && !TransitionInfo.bLanding && (Role == ROLE_Authority) && !IsTimerActive('TryLanding') && (Health > 0))
	{
		if (Action != None && Action.bTakeOff && Physics == PHYS_RigidBody)
		{
			ServerBeginTakeOff();
		}
		// If flying and at least one leg left
		else if(Physics == PHYS_Interpolating && !bLeavingDefeated)
		{
			// Play 'want to land' sound
			PlaySound(SoundCue'Locust_Reaver_Efforts.Reaver.Reaver_BeforeLandCue', FALSE, FALSE, TRUE, Location, FALSE);

			// See if we can land right now.
			LP = ChooseLandingPoint();
			if(LP != None)
			{
				ServerBeginLanding(LP);
			}
			// If not we keep trying each second.
			else
			{
				SetTimer( 1.0, TRUE, nameof(TryLanding) );
			}
		}
	}

	Cannon = GearVWeap_ReaverCannonBase(Weapon);
	if( Cannon != None && Action != None )
	{
		Cannon.MinTimeBetweenRockets = Action.MinTimeBetweenRockets;
	}
}

/** Called on client to set up animations for landing */
simulated function ClientBeginLandingAnim()
{
	local int TentacleIdx;
	local float DesiredRate;

	// Change blend to walking mode slowly
	FlyingBlendNode.SetActiveChild(1, 0.1);
	TransitionNode.SetAnim('Land');
	DesiredRate = 1.9 / LandingTime; // WE want to land at the 1.9 sec mark of the landing anim
	TransitionNode.PlayAnim(FALSE, DesiredRate, 0.0);

	// Turn off trail controls
	for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
	{
		LegInfo[TentacleIdx].TrailControl.SetSkelControlActive(FALSE);
	}

	TransitionStartPosition = Location;
	TransitionStartTangent = Velocity;
	TransitionStartRotation = Rotation;
	CurrentTransitionTime = 0.f;
	bCurrentlyLanding = TRUE;

	// Reset physics to stop it spinning on second landing
	GearVehicleSimChopper(SimObj).bHeadingInitialized = FALSE;
}

// called from animnotify on landing anim
simulated function PlayLandingSound()
{
	ReaverPlayLocalSound(ReaverLandSound, ReaverLandSound_Player);
}

/** Script event called only on server from C++ when landing complete. */
event ServerFinishedLanding()
{
	local int i;
	local SeqEvt_ReaverLanded LandEvt;
	local GearVWeap_ReaverCannonBase Cannon;

	SetPhysics(PHYS_RigidBody);

	LandingPoint = None;
	bWalking = TRUE;

	TransitionInfo.bLanding = FALSE;
	ClientFinishedLanding();

	// Fire 'reaver landed' events
	for (i=0; i<GeneratedEvents.Length; i++)
	{
		LandEvt = SeqEvt_ReaverLanded(GeneratedEvents[i]);
		if(LandEvt != None)
		{
			LandEvt.CheckActivate(self, self, FALSE);
		}
	}

	CloseMouth();

	Cannon = GearVWeap_ReaverCannonBase(Weapon);
	if( Cannon != None )
	{
		AllowRocketFireTime = WorldInfo.TimeSeconds + Cannon.MinTimeBetweenRockets;
	}
}

/** Script event called only everywhere from C++ when landing complete. */
simulated function ClientFinishedLanding()
{
	local int i;
	local vector ImpactLoc;
	local Emitter ImpactEffect;

	InitLegs();
	FlyingBlendNode.SetActiveChild(2, 0.1);
	WalkAnimNode.PlayAnim(TRUE, 0.0, 0.0);
	bCurrentlyLanding = FALSE;

	// Spawn dust where each tentacle hits
	for(i=0; i<6; i++)
	{
		Mesh.GetSocketWorldLocationAndRotation(LegInfo[i].TipSocketName, ImpactLoc);
		ImpactLoc += (LandEffectZOffset * vect(0,0,1));

		ImpactEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(StrikeTemplate.ParticleEmitterTemplate, ImpactLoc, rotator(vect(0,0,1)));
		ImpactEffect.ParticleSystemComponent.ActivateSystem();
	}

	FlyingSoundLoopAC.Stop(); // Stop flying sound

	// Play camera shake when reaver lands

	class'GearPlayerCamera'.static.PlayWorldCameraShake( LandingViewShake, self, Location, LandingShakeInnerRadius, LandingShakeOuterRadius, LandingShakeFalloff, TRUE );

	// Set next time for random sound
//	NextReaverRandomSound = WorldInfo.TimeSeconds + (ReaverRandomSoundInterval.X + (FRand() * (ReaverRandomSoundInterval.Y - ReaverRandomSoundInterval.X)));
}

simulated function ClientBeginTakeOffAnim()
{
	// Change blend to transition node
	FlyingBlendNode.SetActiveChild(1, 0.1);
	TransitionNode.SetAnim('takeoff_start'); // play first part OF take-off anim
	TransitionNode.PlayAnim(FALSE, 1.0, 0.0);

	// Play scream as it flies off
	ReaverPlayLocalSound(ReaverTakeOffSound, ReaverTakeOffSound_Player);

	TransitionStartPosition = Location;
	TransitionStartTangent = Velocity;
	TransitionStartRotation = Rotation;
	CurrentTransitionTime = 0.f;

	// Pause before part 2
	SetTimer( TransitionNode.AnimSeq.SequenceLength, FALSE, nameof(ClientContinueTakeOffAnim) );
}

simulated function ClientContinueTakeOffAnim()
{
	local int TentacleIdx;
	local float DesiredRate;

	TransitionNode.SetAnim('Takeoff'); // Play second part OF take-off anim while moving up
	DesiredRate = TransitionNode.AnimSeq.SequenceLength / TakeOffTime;
	TransitionNode.PlayAnim(FALSE, DesiredRate, 0.0);

	// Turn on trail controls
	for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
	{
		LegInfo[TentacleIdx].IKControl.SetSkelControlActive(FALSE);
		LegInfo[TentacleIdx].TrailControl.SetSkelControlActive(TRUE);
	}

	FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player, FlyingSoundLoopAC, 0.5f);

	bCurrentlyTakingOff = TRUE;
}

event ServerFinishedTakingOff()
{
	local GearVWeap_ReaverCannonBase Cannon;

	TransitionInfo.bTakingOff = FALSE;
	ClientFinishedTakingOff();

	Cannon = GearVWeap_ReaverCannonBase(Weapon);
	if( Cannon != None )
	{
		AllowRocketFireTime = WorldInfo.TimeSeconds + Cannon.MinTimeBetweenRockets;
	}
}

simulated function ClientFinishedTakingOff()
{
	FlyingBlendNode.SetActiveChild(0, 0.4);

	bCurrentlyTakingOff = FALSE;
}

/** Skip straight to walking state - used when connecting while reaver is on ground. */
simulated function ClientForceWalking()
{
	local int TentacleIdx;
	// Turn off trail controls
	for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
	{
		LegInfo[TentacleIdx].TrailControl.SetSkelControlActive(FALSE);
	}
	InitLegs();
	FlyingBlendNode.SetActiveChild(2, 0.0);
	WalkAnimNode.PlayAnim(TRUE, 0.0, 0.0);
}

//////////////////////////////////////////////////////////////////////////
// ATTACKING

/** attempts a leg attack
 * @param Target - Pawn to try attacking
 * @return true if we started an attack or one is already in progress, false otherwise
 */
final function bool CheckLegAttack(Pawn Target)
{
	local float LAttackDist, RAttackDist;

	// Do nothing if already attacking, or too soon
	if((WorldInfo.TimeSeconds < NextAttackTime) || AttackInfo.bLLegAttacking || AttackInfo.bRLegAttacking)
	{
		return true;
	}

	// See if he is close enough to be attacked
	if (Target != None)
	{
		// If attack leg is not stepping - see if we want to attack.
		// Won't quite be right on clients...
		LAttackDist = 1000000.0;
		if(!LegInfo[2].bStepping)
		{
			LAttackDist = VSize(Target.Location - (Location + (LAttackPoint >> Rotation)));
		}

		RAttackDist = 1000000.0;
		if(!LegInfo[5].bStepping)
		{
			RAttackDist = VSize(Target.Location - (Location + (RAttackPoint >> Rotation)));
		}

		// See if we're in danger zone
		if(LAttackDist < LegAttackDist || RAttackDist < LegAttackDist)
		{
			LegAttackRaise(Target.Location, (LAttackDist < RAttackDist));
			return true;
		}
	}

	return false;
}

/** Start of attack sequence. */
simulated function LegAttackRaise(vector AttackLocation, bool bLeftAttackLeg)
{
	local vector HitLocation, HitNormal, AttackCenter;
	local actor TraceHit;
	local int AttackLegIndex, OtherLegIndex;
	local name AnimName;

	if(bLeftAttackLeg)
	{
		if(Role == ROLE_Authority)
		{
			AttackInfo.bLLegAttacking = TRUE;
		}

		AttackCenter = Location + (LAttackPoint >> Rotation);
		AttackLegIndex = 2;
		OtherLegIndex = 5;
		AnimName = 'Attack_L_Start';
	}
	else
	{
		if(Role == ROLE_Authority)
		{
			AttackInfo.bRLegAttacking = TRUE;
		}

		AttackCenter = Location + (RAttackPoint >> Rotation);
		AttackLegIndex = 5;
		OtherLegIndex = 2;
		AnimName = 'Attack_R_Start';
	}

	if(Role == ROLE_Authority)
	{
		AttackInfo.LegAttackTarget = AttackLocation;
		TraceHit = Trace(HitLocation, HitNormal, AttackLocation - vect(0,0,200), AttackLocation, FALSE);
		if(TraceHit != None)
		{
			AttackInfo.LegAttackTarget = HitLocation;
		}
	}

	ReaverPlayLocalSound(ReaverAttackSound);

	AttackSlotNode.PlayCustomAnimByDuration(AnimName, LegRaiseTime, 0.2, -1.0, FALSE);

	TakeStep(OtherLegIndex, 0.f, 0.33f, 1.f);
	MoveFootToPos(AttackLegIndex, (0.5*(AttackCenter + Location)) + vect(0,0,150), LegRaiseTime, 150.0, 0.0);
	SetTimer( LegRaiseTime, FALSE, nameof(LegAttackStrike) );
}

/** Leg begins strike towards ground. */
simulated function LegAttackStrike()
{
	local int AttackLegIndex;
	local name AnimName;

	if(AttackInfo.bLLegAttacking)
	{
		AttackLegIndex = 2;
		AnimName = 'Attack_L_End';
	}
	else
	{
		AttackLegIndex = 5;
		AnimName = 'Attack_R_End';
	}

	AttackSlotNode.PlayCustomAnimByDuration(AnimName, LegAttackTime, 0.0, -1.0, FALSE);

	MoveFootToPos(AttackLegIndex, AttackInfo.LegAttackTarget, LegAttackTime, 0.0, 1.5);

	SetTimer( LegAttackTime, FALSE, nameof(LegAttackImpact) );

	ReaverPlayLocalSound(LegAttackSound);
}

simulated protected event NotifyStepFinished(int LegIdx)
{
	ReaverPlayLocalSound(ReaverFootstepSound);
}

/** Leg impacts ground. */
simulated function LegAttackImpact()
{
	local name AnimName;
	local GearExplosionActor ExplosionActor;

	if(AttackInfo.bLLegAttacking)
	{
		AnimName = 'Attack_L_Pullout';
	}
	else
	{
		AnimName = 'Attack_R_Pullout';
	}

	AttackSlotNode.PlayCustomAnim(AnimName, 1.0, 0.0, 0.2, FALSE);

	ExplosionActor = Spawn(class'GearExplosionActor', self,, AttackInfo.LegAttackTarget, rotator(vect(0,0,1)));
	StrikeTemplate.Damage = StrikeBaseDamage;
	StrikeTemplate.DamageRadius = StrikeDamageRadius;
	StrikeTemplate.DamageFalloffExponent = StrikeDamageFalloff;
	StrikeTemplate.ExploShakeInnerRadius = StrikeDamageRadius;
	StrikeTemplate.ExploShakeOuterRadius = StrikeDamageRadius*2.f;
	ExplosionActor.Explode(StrikeTemplate);
}

function float GetLegAttackInterval()
{
	local float AttackScale;

	AttackScale = 1.f;
	if( Driver == None || Driver.Health <= 0 ) // riderless
	{
		AttackScale = 0.5f;
	}

	return LegAttackInterval * AttackScale;
}

/** Leg withdraws from ground - fired from notify in script. */
simulated function LegAttackWithdraw()
{
	local int AttackLegIndex;

	if(AttackInfo.bLLegAttacking)
	{
		AttackLegIndex = 2;
	}
	else
	{
		AttackLegIndex = 5;
	}

	if(Role == ROLE_Authority)
	{
		AttackInfo.bLLegAttacking = FALSE;
		AttackInfo.bRLegAttacking = FALSE;

		// Attack finished - note when we can next do an attack
		NextAttackTime = WorldInfo.TimeSeconds + GetLegAttackInterval();
	}

	// Move the foot to its stepping position
	TakeStep(AttackLegIndex, 0.f, 0.66f, 1.f);
}


simulated function bool CanBeBaseForPawn(Pawn APawn)
{
	// make sure driver and gunner stay attached
	return (APawn == Driver || APawn == Gunner || Super.CanBeBaseForPawn(APawn));
}

/** See if we are suppressing rocket firing. */
simulated function bool OverrideBeginFire(byte FireModeNum)
{
	if((bSuppressRockets && Physics == PHYS_Interpolating) ||
	   (bSuppressRocketsOnLand && Physics != PHYS_Interpolating))
	{
		return TRUE;
	}

	return Super.OverrideBeginFire(FireModeNum);
}


/** Called from kismet to play an animation. (server only) */
function OnReaverPlayAnim(SeqAct_ReaverPlayAnim Action)
{
	if(Action.InputLinks[0].bHasImpulse)
	{
		AnimRepInfo.AnimName = Action.AnimName;
		AnimRepInfo.bNewData = 1;
	}
}

/** Called from AI (server only) */
function ReaverSeePlayer(Pawn Seen)
{
	// If we are seeing a human player while walking
	if((Seen != None) && (GearPC(Seen.Controller) != None) && (Physics == PHYS_RigidBody))
	{
		// See if we haven't seen anyone for a while
		if(LastSeenPlayerTime < WorldInfo.TimeSeconds - 4.0)
		{
			// use regular playsound here to replicate
			PlaySound(ReaverSeePlayerSound, FALSE, FALSE, TRUE, Location, FALSE);
		}

		LastSeenPlayerTime = WorldInfo.TimeSeconds;
	}
}

simulated function DrivingStatusChanged()
{
	bRestartFlyingLoop = TRUE;
	super.DrivingStatusChanged();
}

simulated event OnAnimEnd( AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime )
{
	if( SeqNode.AnimSeqName == 'Overlay_Head_Roar_A' ||
		SeqNode.AnimSeqName == 'Overlay_Head_Roar_B' )
	{
		CloseMouth();
	}

	Super.OnAnimEnd( SeqNode, PlayedTime, ExcessTime );
}

function float GetNextMouthOpenTime()
{
	local float Delay;

	Delay = RandRange( MouthOpenTime.X, MouthOpenTime.Y );
	// If driver is dead, open mouth more often
	if( Driver == None || Driver.Health <= 0 )
	{
		Delay *= 0.5f;
	}

	return Delay;
}

function OpenMouth()
{
	if( bMouthOpen == 0 )
	{
		bMouthOpen = (FRand() < 0.5f) ? 1 : 2;
		PlayMouthOpenAnim();
	}
	else
	if( bWalking )
	{
		SetTimer( GetNextMouthOpenTime(), FALSE, nameof(OpenMouth) );
	}
}

function CloseMouth()
{
	bMouthOpen = 0;
	if( bWalking )
	{
		SetTimer( GetNextMouthOpenTime(), FALSE, nameof(OpenMouth) );
	}
	RandHeadBlendNode.SetActiveChild( Rand(4), 0.1f );
}

simulated function PlayMouthOpenAnim()
{
	if( bMouthOpen != 0 )
	{
		if( bMouthOpen == 1 )
		{
			RandHeadBlendNode.SetActiveChild( 5, 0.1f );
		}
		else
		{
			RandHeadBlendNode.SetActiveChild( 6, 0.1f );
		}
	}
}

defaultproperties
{
	Begin Object Name=CollisionCylinder
		CollisionRadius=72.0
		CollisionHeight=120.0
	End Object

	bAllowLanding=TRUE

	bSpawnSoftGibs=FALSE
	bSpawnBloodTrailDecals=TRUE
	GibEffectsSpeedThreshold=125.0
	TimeBetweenGibEffects=0.25

	Physics=PHYS_Interpolating
	LandingTime=1.5
	TakeOffTime=1.5
	MinLandingPointDot=0.2
	WalkAnimLinRateScale=0.015
	WalkAnimAngRateScale=0.40
	LegForceStepPlayerDist=150.0
	LegForceStepPlayerRand=200.0
	LegForceStepErrorDist=250.0
	bUpdateSimulatedPosition=FALSE // Reaver is either on matinee path or using rigid-body vehicle replication

	LandSplineTangent=-400.0
	LandZOffset=150.0
	LandEffectZOffset=50.0

	StepZTangent=400.0
	//bDrawStepDebug=TRUE
	StepAdvanceFactor=0.6
	StepEndSlope=1.5

	LAttackPoint=(X=340,Y=-125,Z=-200)
	RAttackPoint=(X=340,Y=125,Z=-200)
	LegAttackDist=200.0
	LegAttackTime=0.16
	LegAttackInterval=1.5

	LegInfo(0)=(TipSocketName="AL_Tip", DefaultLegPos=(X=300.0,Y=-8.0,Z=-215.0))
	LegInfo(1)=(TipSocketName="BL_Tip", DefaultLegPos=(X=320.0,Y=-310.0,Z=-215.0))
	LegInfo(2)=(TipSocketName="CL_Tip", DefaultLegPos=(X=225.0,Y=260.0,Z=-215.0))
	LegInfo(3)=(TipSocketName="AR_Tip", DefaultLegPos=(X=-300.0,Y=8.0,Z=-215.0))
	LegInfo(4)=(TipSocketName="BR_Tip", DefaultLegPos=(X=-320.0,Y=-310.0,Z=-215.0))
	LegInfo(5)=(TipSocketName="CR_Tip", DefaultLegPos=(X=-225.0,Y=260.0,Z=-215.0))

	AngVelAimOffsetScale=0.25
	AngVelAimOffsetChangeSpeed=3.0

	bCanFly=false
	bCanWalk=true
	bJumpCapable=false
	bCanJump=false
	bCanSwim=false
	bCanClimbLadders=false
	bCanBeBaseForPawns=true
	bCanStrafe=true
	bTurnInPlace=true
	bNoEncroachCheck=true
	bFollowLookDir=true
	ExtraReachDownThreshold=150.0
	//bRespondToExplosions=FALSE
	bIgnoreForces=TRUE

	PeripheralVision=-1

	//NoticedGUDSPriority=120
	//NoticedGUDSEvent=GUDEvent_NoticedReaver

	SkinHeatFadeTime=0.4f
	SkinHeatDamagePctToFullyHeat=0.3f
	CurrentSkinHeatMin=0.0f
	SkinCharFadeTime=1.f


}
