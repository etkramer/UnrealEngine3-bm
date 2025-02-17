/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_Centaur_Base extends GearVehicle
	native(Vehicle)
	abstract
	config(Pawn);

/** Default health. */
var		config int		DefaultCentaurHealth;

/** How fast turret can rotate */
var()	config float	CentaurTurretSpeed;

/** FOV to use when targetting. */
var() config float TargetingFOV;
/** FOV to use when boosting. */
var() config float BoostingFOV;
/** How quickly boost fuel is used up when boosting. */
var() config float BoostUseRate;
/** How quickly boost regenerates when not boosting. */
var() config float BoostRechargeRate;
/** Max boost fuel. */
var() config float MaxBoostFuel;
/** Time between boosts */
var() config float ReBoostDelay;
/** How long a boost will go on for at least, even if you release the button */
var() config float MinBoostDuration;
/** Current amount of fuel we have. */
var	float BoostFuel;
/** Whether centaur can boost  (usually set via kismet action) */
var() bool bAllowBoosting;

/** Sound of locust squishing **/
var SoundCue SquishSound;

/** Whether we were on the ground last frame */
var bool bOnGroundLastFrame;

/** Time of last runover event notification */
var float LastTimeActorRunOver;
/** Minimum time between notifications */
var float MinTimeBetweenRunOverEvents;

/** Enable / disable the cannon firing */
var() bool bSuppressCannonFire;

/** The time that we can next boost. */
var float NextBoostTime;
/** The time we last started a boost. */
var float LastBoostTime;
/** Name of the parameter for the boost heat effect */
var name BoostHeatEffectName;

/** Indicates if we are currently boosting. */
var bool bIsBoosting;

var		ParticleSystemComponent	PSC_BoostWheelEffect[2];

/** Enumeration of different surfaces centaur can be on */
enum ECentaurOnSurface
{
	ECOS_Snow,
	ECOS_Ice,
	ECOS_Cave
};

var repnotify ECentaurOnSurface CentaurSurface;

var()	ParticleSystem			SnowWheelEffect;
var()	ParticleSystem			SnowPeelOutEffect;
var()	ParticleSystem			SnowBoostEffect;
var protected const SoundCue	SnowTireAudioLoop;

var()	ParticleSystem			IceWheelEffect;
var()	ParticleSystem			IcePeelOutEffect;
var()	ParticleSystem			IceBoostEffect;
var protected const SoundCue	IceTireAudioLoop;

var()	ParticleSystem			CaveWheelEffect;
var()	ParticleSystem			CavePeelOutEffect;
var()	ParticleSystem			CaveBoostEffect;
var protected const SoundCue	CaveTireAudioLoop;

var protected transient AudioComponent TireAudioLoopAC;
var() protected const vector2d TireAudioLoopVolumeRange;
var() protected const vector2d TireAudioLoopVolumeVelocityRange;

/** Size of impulse to apply to vehicle when firing cannon. */
var() config float RecoilLinStrength;
/** Size of impulse to apply to vehicle when firing cannon. */
var() config float RecoilAngStrength;

/** Spotlight component */
var SpotLightComponent SpotLightComp;
/** Spotlight particle effect */
var ParticleSystem SpotlightConeEffect;
/** Spotlight cone effect */
var ParticleSystemComponent PSC_Spotlight;
/** MIC for section 0, that contains spotlights - used for turning on/off to match light status */
var	MaterialInstanceConstant SpotLightMIC;

/** Sound for ambient spotlight on */
var SoundCue SpotLightSound;
/** Sound for spotlight coming on */
var SoundCue SpotLightFlickerSound;
/** Sound for spotlight turning off */
var SoundCue SpotLightOffSound;
/** Holds the ambient spotlight */
var AudioComponent AC_SpotlightSound;

/** Sound of a horn */
var SoundCue HornSound;

/** The snow particles that are attached to vehicle **/
var ParticleSystem SnowSprayTemplate;
/** Component for spawning particles */
var ParticleSystemComponent PSC_SnowSpray;
/** Socket effect is attached to */
var name SnowFieldSocketName;
/** Used to track whether snow field is active. */
var repnotify bool bSnowFieldActive;

/** Socket name for the tailpipe */
var name TailpipeSocketName;
/** The tailpipe effect during boost */
var() ParticleSystem TailpipeBoostEffect;
/** Component for tailpipe effect */
var ParticleSystemComponent PSC_TailpipeBoost;

/** Sound to play when boost starts */
var SoundCue BoostStartSound;
/** Sound to play when boost ends */
var SoundCue BoostEndSound;

/**
* This is the MIC which will help control the boost heat effect
**/
var transient MaterialInstanceConstant MIC_VehicleSkin;

/** Camera anim to play when your centaur boosts. */
var CameraAnim BoostCameraAnim;

/** If TRUE, when boosting push camera back to center. */
var() config bool bAutoCenterCamOnBoost;

/** Speed you have to be going when you hit a reaver to kill it. */
var() config float	KillReaverVel;
/** Speed you have to be going to kill crowd members */
var() config float	KillCrowdVel;
/** How hard centaur pushes nearby crowd members */
var() config float	PushCrowdForce;
/** Radius around centaur that we push crowd members */
var() config float	PushCrowdRadius;
/** Radius around centaur that we push crowd members */
var() config float	KillCrowdRadius;

/** Cam anim to play when you run creature over */
var() CameraAnim RunOverCreatureCamAnim;

/** Camera effect to play when boosting */
var class<Emit_CameraLensEffectBase> BoostCameraEffectClass;

var() SoundCue CentaurDeathSound;
var()	ParticleSystem CentaurDeathEffect;
/** Camera anim to play when your centaur explodes. */
var() CameraAnim CentaurDeathCamAnim;

var() CameraAnim CentaurCannonCamAnim;
var() float	TargetingCannonCamAnimScale;
var() SoundCue CentaurCannonSound;
var() ParticleSystem CentaurCannonMuzzzleEffect;
var ParticleSystemComponent PSC_MuzzleEffect;

var()	editconst PointLightComponent	CentaurCannonMuzzleLight;
var()	float							CentaurCannonMuzzleLightTime;

/** The world time after which we will start to recover health. */
var float	NextDamageRecoverTime;
/** Store partial health points recovered between frames. */
var float	HealthRecoverRemainder;

/** How quickly we recover health */
var() config float	DamageRecoverRate;
/** How long after taking damage we start recovering health. */
var() config float	DamageRecoverDelay;

/** Socket to attach damage effect to. */
var name DamageEffectSocketName;

/** Baird lines to play when he starts repairing. */
var array<SoundCue> StartRepairDialog;
/** Baird lines to play when he's done repairing */
var array<SoundCue> DoneRepairDialog;
/** Indicates we have been damaged past stage 1 since last heal */
var bool bDamagedPastStage1;
/** Indicates we are coming from full health */
var bool bFromFullHealth;

var() config float BairdRepairDialogChance;

/** used to play baird's repair lines */
var AudioComponent BairdRepairAC;


/** Container of damaged and on fire sound **/
var AudioComponent AC_OnFireSound;

var ParticleSystem Stage1DamageEffect;
var ParticleSystemComponent PSC_Stage1Damage;
var float Stage1Health;

var ParticleSystem Stage2DamageEffect;
var ParticleSystemComponent PSC_Stage2Damage;
var float Stage2Health;

var ParticleSystem Stage3DamageEffect;
var ParticleSystemComponent PSC_Stage3Damage;
var float Stage3Health;

/** Flag indicates we have scaled health based on difficulty - ensures we only do it once */
var bool bHasSetHealth;

var		ParticleSystemComponent	PSC_PeelOutEffect[4];
var()	CameraAnim PeelOutCamAnim;
var()	float	PeelOutMaxWheelVel;

var	bool bPeelOutEffectsActive;

var()	SoundCue	PeelOutSound;
var	float LastPeelOutSound;

/** Animation node used to play firing anim etc. */
var		AnimNodeSequence	ActionNode;

//////////////////////////////////////////////////////////////////////////
// cam

/** Used to calculate timestep for updating camera */
var		float	OldTime;

/** Stiffness of camera spring */
var(CentaurCam)		vector	CamStiffness;
/** Damping of camera spring */
var(CentaurCam)		vector	CamDamping;
/** Max distance camera is allowed to displace */
var(CentaurCam)		vector	CamLimits;

/** Current 'sprung' position of camera. */
var		vector	CamPos;
/** Current 'sprung' velocity of camera. */
var		vector	CamVel;

var(CentaurCam)		float	CamYawStiffness;
var(CentaurCam)		float	CamYawDamping;

var		float	CamYawPos;
var		float	CamYawVel;

/** MI that is used by the post process, that is modified as centaur boots */
var(CentaurCam)		MaterialInstance	OverlayMI;
/** Centaur additional post process chain */
var(CentaurCam)		PostProcessChain	OverlayEffect;
/** Amount to increase 'spreead' in effect as we boost */
var(CentaurCam)		float				OverlaySpreadFactor;
/** Amount to blend in effect as we boost */
var(CentaurCam)		float				OverlayEffectFactor;

var(CentaurCam)		vector				SplitDriverCamViewOffsetHigh;
var(CentaurCam)		vector				SplitDriverCamViewOffsetMid;
var(CentaurCam)		vector				SplitDriverCamViewOffsetLow;


/** replicated flag for toggling the spotlight on the client */
var repnotify bool bSpotlightActive;

/** Time the spotlight started turning off/on */
var float SpotlightToggleTime;
/** Current state of light flicker */
var bool bSpotlightFlickerOn;
/** Min time between flicker */
var float LightFlickerTimeMin;
/** Total time light flickering lasts */
var float LightFlickerDuration;

struct CheckpointRecord
{
	var bool bSpotlightActive;
	var bool bSnowEffectsActive;
	var bool bAllowBoosting;
	var ECentaurOnSurface CentaurOnSurface;
	var float WheelLongGripScale;
	var float WheelLatGripScale;
	var float WheelExtraGraphicalSpin;
	var float MaxSpeed;
};

var protected transient AudioComponent EnginePlayerAmbientLoopAC;
var protected const SoundCue EnginePlayerAmbientLoopCue;

var() protected const vector2d EnginePlayerAmbientVolumeRange;
var() protected const vector2d EnginePlayerAmbientVelocityRange;


var protected const SoundCue CentaurSuspensionSound;
var protected transient float LastSuspensionSoundTime;
var() protected const float CentaurSuspensionSoundMinTimeBetween;
var() protected const float CentaurSuspensionSoundThreshold;

var protected const SoundCue CentaurLandSound;
var protected transient bool bPlayLandSoundUponLanding;

var protected AudioComponent CentaurTurretRotationAC;
var() protected const vector2d CentaurTurretRotationVolumeVelRange;
var() protected const vector2d CentaurTurretRotationVolumeRange;
var() protected const vector2d CentaurTurretRotationPitchVelRange;
var() protected const vector2d CentaurTurretRotationPitchRange;



struct native CentaurGearInfo
{
	var vector2d PitchRPMRange;
	var vector2d PitchRange;
	var vector2d VolumeRPMRange;
	var vector2d VolumeRange;

	var SoundCue EngineLoopCue;
};

var() protected const CentaurGearInfo CentaurGear;

var protected transient AudioComponent CurrentEngineLoopAC;

var protected const SoundCue EngineStartSound;
var protected const SoundCue EngineStopSound;

var repnotify bool bEngineSoundEnabled;

var protected transient float DebugLastRPM;

/** action info associated with turning on the spot. */
var const ActionInfo ActionTurnOnSpot;
/** Speed below which we show 'turn on spot' tooltip */
var() float	TurnTooltipVelThresh;
/** Time before turn-in-place tooltip comes up */
var() float TurnToolTipDelay;
/** How long we have been slow enough to display tooltip */
var transient float TimeBelowTooltipVel;

replication
{
	if (bNetDirty)
		BoostFuel, CentaurSurface, bSpotlightActive, bSnowFieldActive, bEngineSoundEnabled, DefaultCentaurHealth, bAllowBoosting;
}

cpptext
{
public:
	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& Info0, const FRigidBodyCollisionInfo& Info1, const FCollisionImpactData& RigidCollisionData);
	virtual void ApplyWeaponRotation(INT SeatIndex, FRotator NewRotation);
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;
	virtual void SetWheelEffectParams(USVehicleWheel* VW, FLOAT SlipVel);
	virtual FLOAT GetGravityZ();

};


/** Internal.  Plays given sound, non-replicated. */
simulated event protected AudioComponent CentaurPlayLocalSound(SoundCue Sound, optional float FadeInTime)
{
	local AudioComponent AC;

	if( Sound != None )
	{
		AC = CreateAudioComponent(Sound, FALSE, TRUE);
		if( AC != None )
		{
			AC.bUseOwnerLocation	= TRUE;
			AC.bAutoDestroy			= TRUE;
			AC.bStopWhenOwnerDestroyed = TRUE;
			AttachComponent(AC);
		}
		if (AC != None)
		{
			AC.FadeIn(FadeInTime, 1.f);
			return AC;
		}
	}

	return AC;
}


simulated function PostBeginPlay()
{
	local int i;

	super.PostBeginPlay();

	if(Role == ROLE_Authority)
	{
		Health = DefaultCentaurHealth;
		HealthMax = DefaultCentaurHealth;
	}

	// Set turret speed from ini
	for(i=0; i<Seats[0].TurretControllers.length; i++)
	{
		Seats[0].TurretControllers[i].LagDegreesPerSecond = CentaurTurretSpeed;
	}

	// Create MIC for section 0, so we can turn light on and off in it
	SpotLightMIC = Mesh.CreateAndSetMaterialInstanceConstant(0);

	// Attach light
	Mesh.AttachComponentToSocket(SpotLightComp, 'SpotLightSocket');

	if(SnowSprayTemplate != None)
	{
		PSC_SnowSpray = new(self) class'ParticleSystemComponent';
		PSC_SnowSpray.SetTickGroup( TG_PostUpdateWork );
		Mesh.AttachComponentToSocket( PSC_SnowSpray, SnowFieldSocketName );
		PSC_SnowSpray.SetTemplate( SnowSprayTemplate );
		PSC_SnowSpray.ActivateSystem();
		bSnowFieldActive = TRUE;
	}

	PSC_MuzzleEffect = new(self) class'ParticleSystemComponent';
	PSC_MuzzleEffect.SetTickGroup( TG_PostUpdateWork );
	PSC_MuzzleEffect.bAutoActivate = FALSE;
	Mesh.AttachComponentToSocket( PSC_MuzzleEffect, Seats[0].GunSocket[0] );
	PSC_MuzzleEffect.SetTemplate( CentaurCannonMuzzzleEffect );

	// Damage effects

	PSC_Stage1Damage = new(self) class'ParticleSystemComponent';
	PSC_Stage1Damage.SetTickGroup( TG_PostUpdateWork );
	PSC_Stage1Damage.bAutoActivate = FALSE;
	Mesh.AttachComponentToSocket( PSC_Stage1Damage, DamageEffectSocketName );
	PSC_Stage1Damage.SetTemplate( Stage1DamageEffect );
	Stage1Health = 0.75 * DefaultCentaurHealth;

	PSC_Stage2Damage = new(self) class'ParticleSystemComponent';
	PSC_Stage2Damage.SetTickGroup( TG_PostUpdateWork );
	PSC_Stage2Damage.bAutoActivate = FALSE;
	Mesh.AttachComponentToSocket( PSC_Stage2Damage, DamageEffectSocketName );
	PSC_Stage2Damage.SetTemplate( Stage2DamageEffect );
	Stage2Health = 0.5 * DefaultCentaurHealth;

	PSC_Stage3Damage = new(self) class'ParticleSystemComponent';
	PSC_Stage3Damage.SetTickGroup( TG_PostUpdateWork );
	PSC_Stage3Damage.bAutoActivate = FALSE;
	Mesh.AttachComponentToSocket( PSC_Stage3Damage, DamageEffectSocketName );
	PSC_Stage3Damage.SetTemplate( Stage3DamageEffect );
	Stage3Health = 0.25 * DefaultCentaurHealth;

	// Boost effects
	PSC_TailpipeBoost = new(self) class'ParticleSystemComponent';
	PSC_TailpipeBoost.SetTickGroup( TG_PostUpdateWork );
	PSC_TailpipeBoost.bAutoActivate = FALSE;
	Mesh.AttachComponentToSocket( PSC_TailpipeBoost, TailpipeSocketName );
	PSC_TailpipeBoost.SetTemplate( TailpipeBoostEffect );

	// Access the boost heat pipe on the centaur material
	MIC_VehicleSkin = Mesh.CreateAndSetMaterialInstanceConstant(1);

	// Spotlight effects
	PSC_Spotlight = new(self) class'ParticleSystemComponent';
	PSC_Spotlight.SetTickGroup( TG_PostUpdateWork );
	Mesh.AttachComponentToSocket(PSC_Spotlight, 'SpotlightSocket');
	PSC_Spotlight.SetTemplate(SpotlightConeEffect);

	Mesh.AttachComponentToSocket(AC_SpotlightSound, 'SpotLightSocket');

	// Wheel effects

	// Slip on all 4 wheels
	for(i=0; i<4; i++)
	{
		PSC_PeelOutEffect[i] = new(self) class'ParticleSystemComponent';
		PSC_PeelOutEffect[i].bAutoActivate = FALSE;
		AttachComponent(PSC_PeelOutEffect[i]);
		PSC_PeelOutEffect[i].SetTemplate(SnowPeelOutEffect);
	}

	// Boost on rear 2
	for(i=0; i<2; i++)
	{
		PSC_BoostWheelEffect[i] = new(self) class'ParticleSystemComponent';
		PSC_BoostWheelEffect[i].bAutoActivate = FALSE;
		AttachComponent(PSC_BoostWheelEffect[i]);
		PSC_BoostWheelEffect[i].SetTemplate(SnowBoostEffect);
	}

	// defualt tire audio
	TireAudioLoopAC = CentaurPlayLocalSound(SnowTireAudioLoop, 0.2f);
	TireAudioLoopAC.VolumeMultiplier = 0.f;

	bFromFullHealth = TRUE;

	// Init effects for snow
	SetCentaurWheelEffects(ECOS_Snow);
}

simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	Super.PostInitAnimTree(SkelComp);

	if(SkelComp == Mesh)
	{
		ActionNode = AnimNodeSequence(Mesh.FindAnimNode('ActionNode'));
	}
}

simulated function ReplicatedEvent(name VarName)
{
	if (VarName == nameof(CentaurSurface))
	{
		SetCentaurWheelEffects(CentaurSurface);
	}
	else if (VarName == nameof(bSpotlightActive))
	{
		SetSpotlightEnabled(bSpotlightActive, true);
	}
	else if (VarName == nameof(bSnowFieldActive))
	{
		SetSnowEffectsActive(bSnowFieldActive);
	}
	else if(VarName == nameof(bEngineSoundEnabled))
	{
		EnableEngineSound(bEngineSoundEnabled);
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/**
* Play the horn for this vehicle
*/
function PlayHorn()
{
	// time between sounds check in GearPC
	PlaySound(HornSound);
}

event bool DriverLeave(bool bForceLeave)
{
	local bool bLeft;
	local GearPC GPC;

	GPC = GearPC(Controller);

	bLeft = Super.DriverLeave(bForceLeave);
	if(bLeft && GPC != None)
	{
		ClientSetCameraEffect(GPC, FALSE);
	}
	return bLeft;
}

function PassengerLeave(int SeatIndex)
{
	local GearPC GPC;

	if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
	}

	Super.PassengerLeave(SeatIndex);
	if(GPC != None)
	{
		ClientSetCameraEffect(GPC, FALSE);
	}
}

function bool PassengerEnter(Pawn P, int SeatIndex)
{
	local bool bEntered;
	local GearPC GPC;

	GPC = GearPC(P.Controller);

	bEntered = Super.PassengerEnter(P, SeatIndex);
	if(bEntered && GPC != None)
	{
		ClientSetCameraEffect(GPC, TRUE);
	}
	return bEntered;
}

function bool DriverEnter(Pawn P)
{
	local bool bEntered;
	local GearPC GPC;

	GPC = GearPC(P.Controller);

	bEntered = Super.DriverEnter(P);
	if(bEntered && GPC != None)
	{
		ClientSetCameraEffect(GPC, TRUE);
	}
	return bEntered;
}

simulated function OnToggleBoostEffect(SeqAct_ToggleBoostEffect Action)
{
	local GearPC GPC;
	local int Idx;
	for (Idx = 0; Idx < Seats.Length; Idx++)
	{
		if (Seats[Idx].SeatPawn != None)
		{
			GPC = GearPC(Seats[Idx].SeatPawn.Controller);
			if (Action.InputLinks[0].bHasImpulse)
			{
				ClientSetCameraEffect(GPC,TRUE);
			}
			else if (Action.InputLinks[1].bHasImpulse)
			{
				ClientSetCameraEffect(GPC,FALSE);
			}
			else
			{
				ClientSetCameraEffect(GPC,OverlayMI==None);
			}
		}
	}
}


simulated function Tick(float DeltaTime)
{
	local int i;
	local float NormalizedGroundSpeed;
	local int GainHealth, PrevHealth;

	super.Tick(DeltaTime);

	NormalizedGroundSpeed = FClamp(VSize2D(Velocity) / AirSpeed, 0.0, 1.0);

	if(PSC_SnowSpray != None)
	{
		PSC_SnowSpray.SetFloatParameter( 'CentaurSpeed', NormalizedGroundSpeed );
	}

	// If we are recovering health
	if((Role == ROLE_Authority) && (WorldInfo.TimeSeconds > NextDamageRecoverTime) && (Health < DefaultCentaurHealth) && (Health > 0))
	{
		// Find integer health units recovered
		HealthRecoverRemainder += (DamageRecoverRate * DeltaTime);
		GainHealth = FFloor(HealthRecoverRemainder);

		// Ensure we won't go over max
		GainHealth = Min(GainHealth, DefaultCentaurHealth - Health);

		if(GainHealth > 0)
		{
			// Add health units, and remove from accumulator
			PrevHealth = Health;
			Health += GainHealth;
			HealthRecoverRemainder -= GainHealth;

			ReceivedHealthChange(PrevHealth);
		}
	}
	else
	{
		// Reset accumulator
		HealthRecoverRemainder = 0.0;
	}

	// Show HUD element about turning on spot when on the ice lake
	if(GearPC(Controller) != None)
	{
		if((VSize2D(Velocity) < TurnTooltipVelThresh) && bVehicleOnGround && (CentaurSurface == ECOS_Ice))
		{
			TimeBelowTooltipVel += DeltaTime;
		}
		else
		{
			TimeBelowTooltipVel = 0.0;
		}

		if( TimeBelowTooltipVel >= TurnToolTipDelay )
		{
			GearPC(Controller).MyGearHUD.SetActionInfo( AT_VehicleControl, ActionTurnOnSpot );
		}
		else
		{
			GearPC(Controller).MyGearHUD.ClearActionInfoByType( AT_VehicleControl );
		}
	}

	//Deactivate boost/peelout effect when we leave the ground
	if (bOnGroundLastFrame && !bVehicleOnGround)
	{
	   // Slip on all 4 wheels
	   for(i=0; i<4; i++)
	   {
		   PSC_PeelOutEffect[i].SetActive(False);
	   }

	   // Boost on rear 2
	   for(i=0; i<2; i++)
	   {
		   PSC_BoostWheelEffect[i].SetActive(False);
	   }
	}
	bOnGroundLastFrame = bVehicleOnGround;

	UpdateOverlayEffects();
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
	local class<DifficultySettings> Difficulty;

	if (GearPC(C) != None)
	{
		DefaultCentaurHealth = default.DefaultCentaurHealth * GearPRI(C.PlayerReplicationInfo).Difficulty.default.PlayerHealthMod;
		HealthMax = DefaultCentaurHealth;
	}

	//Set PP effects here as Pawn may have already been set
    //via Driver/Passenger Enter before possession by controller
	ClientSetCameraEffect(C, TRUE);

	Super.PossessedBy(C, bVehicleTransition);

	if(!bHasSetHealth && Role == ROLE_Authority && GearPC(C) != None)
	{
		Difficulty = GearPRI(C.PlayerReplicationInfo).Difficulty;

		DefaultCentaurHealth = default.DefaultCentaurHealth * Difficulty.default.PlayerHealthMod; // We scale this because its used for omen-drawing
		HealthMax = DefaultCentaurHealth;

		Health = DefaultCentaurHealth;
		Stage1Health = 0.75 * DefaultCentaurHealth;
		Stage2Health = 0.5 * DefaultCentaurHealth;
		Stage3Health = 0.25 * DefaultCentaurHealth;

		DamageRecoverRate = default.DamageRecoverRate * Difficulty.default.PlayerHealthRechargeSpeedMod;
		DamageRecoverDelay = default.DamageRecoverDelay * Difficulty.default.PlayerHealthRechargeDelayMod;

		bHasSetHealth = TRUE;
	}
}

/** Play a line of baird dialog */
simulated function PlayBairdLine(SoundCue Sound)
{
	if(BairdRepairAC.IsPlaying())
	{
		BairdRepairAC.Stop();
	}

	BairdRepairAC.SoundCue = Sound;
	BairdRepairAC.SubtitlePriority = 1; // jag: should this be different? needed to make subtitles show up
	BairdRepairAC.Play();
}

/** Called when health changes (including when health is replicated to us) */
simulated event ReceivedHealthChange(int OldHealth)
{
	local SoundCue PlayCue;

	//`log("HEALTH"@Health);

	// HEALING
	if(Health > OldHealth)
	{
		if((OldHealth < Stage1Health) && (Health >= Stage1Health))
		{
			HealedPastStage(1);
		}
		else if((OldHealth < Stage2Health) && (Health >= Stage2Health))
		{
			HealedPastStage(2);
		}
		else if((OldHealth < Stage3Health) && (Health >= Stage3Health))
		{
			HealedPastStage(3);
		}

		//
		if(Health == DefaultCentaurHealth)
		{
			if(bDamagedPastStage1)
			{
				// Play baird dialog
				if(FRand() < BairdRepairDialogChance && !IsTimerActive(nameof(DelayRepairDialogueEnd)))
				{
					PlayCue = DoneRepairDialog[Rand(DoneRepairDialog.length)];
					PlayBairdLine(PlayCue);
					SetTimer(35.f,FALSE,nameof(DelayRepairDialogueEnd));
				}

				bDamagedPastStage1 = FALSE;
			}

			bFromFullHealth = TRUE;
		}
	}
	// DAMAGING
	else
	{
		if((OldHealth >= Stage1Health) && (Health < Stage1Health))
		{
			DamagedPastStage(1);

			if(bFromFullHealth)
			{
				// Play baird dialog (assuming not insta death)
				if(FRand() < BairdRepairDialogChance && Health > Stage3Health && bFromFullHealth && !IsTimerActive(nameof(DelayRepairDialogueBegin)))
				{
					PlayCue = StartRepairDialog[Rand(StartRepairDialog.length)];
					PlayBairdLine(PlayCue);
					SetTimer(35.f,FALSE,nameof(DelayRepairDialogueBegin));
				}

				bFromFullHealth = FALSE;
			}

			bDamagedPastStage1 = TRUE;
		}
		else if((OldHealth >= Stage2Health) && (Health < Stage2Health))
		{
			DamagedPastStage(2);
		}
		else if((OldHealth >= Stage3Health) && (Health < Stage3Health))
		{
			DamagedPastStage(3);
		}
	}
}

simulated function DelayRepairDialogueBegin();
simulated function DelayRepairDialogueEnd();

simulated function HealedPastStage(int Stage)
{

	//`log("HealedPastStage"@Stage);



	// Turn off relevant effects
	if(Stage <= 3)
	{
		PSC_Stage3Damage.DeactivateSystem();
	}

	if(Stage <= 2)
	{
		PSC_Stage2Damage.DeactivateSystem();
	}

	if(Stage <= 1)
	{
		PSC_Stage1Damage.DeactivateSystem();

		if (AC_OnFireSound.IsPlaying())
		{
			AC_OnFireSound.Stop();
		}
	}
}

simulated function DamagedPastStage(int Stage)
{
	//`log("DamagedPastStage"@Stage);

	if(Stage >= 3)
	{
		PSC_Stage3Damage.ActivateSystem();
	}

	if(Stage >= 2)
	{
		PSC_Stage2Damage.ActivateSystem();
	}

	if(Stage >= 1)
	{
		PSC_Stage1Damage.ActivateSystem();
		AC_OnFireSound.Play();
	}
}

simulated event float GetCameraFOV(int SeatIndex)
{
	local GearPC PC;

	if (Seats[SeatIndex].SeatPawn != None)
	{
		PC = GearPC(Seats[SeatIndex].SeatPawn.Controller);
	}

	if (PC != None && PC.bIsTargeting)
	{
		return TargetingFOV;
	}
	else if (bIsBoosting)
	{
		return BoostingFOV;
	}
	else
	{
		return DefaultFOV;
	}
}

/** Do cam shake (on driver and gunner controller) */
simulated function PlayCamAnimOnAllControllers(CameraAnim Anim)
{
	local GearPC GPC;

	// Driver
	GPC = GearPC(Controller);
	if(GPC != None && LocalPlayer(GPC.Player) != None)
	{
		GearPlayerCamera(GPC.PlayerCamera).PlayCameraAnim(Anim,,, 0.05f, 0.1f);
	}

	// Gunner
	if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
		if(GPC != None && LocalPlayer(GPC.Player) != None)
		{
			GearPlayerCamera(GPC.PlayerCamera).PlayCameraAnim(Anim,,, 0.05f, 0.1f);
		}
	}
}

/** Do cam shake (on driver and gunner controller) */
simulated function PlayDirtEffectOnAllControllers()
{
	local GearPC GPC;

	// Driver
	GPC = GearPC(Controller);
	if(GPC != None)
	{
		GPC.ClientSpawnCameraLensEffect(BoostCameraEffectClass);
	}

	// Gunner
	if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
		if(GPC != None)
		{
			GPC.ClientSpawnCameraLensEffect(BoostCameraEffectClass);
		}
	}
}

simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int PrevHealth;

	if(EventInstigator == Controller)
	{
		return;
	}

	if(Role == ROLE_Authority)
	{
		// Delay recovery
		NextDamageRecoverTime = WorldInfo.TimeSeconds + DamageRecoverDelay;

		PrevHealth = Health;
	}

	Super.TakeDamage(Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	if(Role == ROLE_Authority)
	{
		ReceivedHealthChange(PrevHealth);
	}
}

/** Handle hitting other gear vehicles. */
event HitOtherGearVehicle(GearVehicle OtherGearV)
{
	if(Role == ROLE_Authority)
	{
		if((Vehicle_Reaver_Base(OtherGearV) != None) && (VSize(Velocity) > KillReaverVel))
		{
			// Deal damage to reaver
			OtherGearV.TakeDamage(100000000.0, Controller, Location, Velocity, RanOverDamageType);

			// Shake our cam a bit
			PlayCamAnimOnAllControllers(RunOverCreatureCamAnim);
		}
	}
}

/** Make sure we don't kill brumaks when encroaching on them. */
event bool EncroachingOn(Actor Other)
{
	if(GearPawn_LocustBrumakBase(Other) != None)
	{
		return FALSE;
	}

	return Super.EncroachingOn(Other);
}

/**
* RanInto() called for encroaching actors which successfully moved the other actor out of the way
*
* @param	Other 		The pawn that was hit
*/
event RanInto(Actor Other)
{
	local float SpeedSq;
	local GearPawn GP;

	GP = GearPawn(Other);

	if ( GP == None || !GP.bCanBeRunOver || Other == Instigator || Other.Role != ROLE_Authority )
		return;

	Super.RanInto(Other);

    //Notify Kismet we ran over something recently
	if (WorldInfo.TimeSeconds - LastTimeActorRunOver > MinTimeBetweenRunOverEvents)
	{
		SpeedSq = VSizeSq(Velocity);
		if (SpeedSq > Square(MinRunOverSpeed))
		{
			if ( WorldInfo.GRI.OnSameTeam(self,Other) == False )
			{
				TriggerEventClass(class'SeqEvt_CentaurSquish', self);
				LastTimeActorRunOver = WorldInfo.TimeSeconds;
				PlaySound(SquishSound, False, False, False, Other.Location, False);
			}
		}
	}

}

/** Allow vehicle to boost when there is fuel. */
simulated event bool CanBoost()
{
	return (BoostFuel > 0.0) && (WorldInfo.TimeSeconds > NextBoostTime) && bAllowBoosting;
}

/** Called to force the vehicle to boost. */
simulated event bool ForceBoost()
{
	return (WorldInfo.TimeSeconds - LastBoostTime) < MinBoostDuration;
}

/** When boosting, see if we want to center. */
simulated function bool ShouldCenterCamSpaceCamera()
{
	if(bAutoCenterCamOnBoost)
	{
		return bIsBoosting;
	}
	else
	{
		return FALSE;
	}
}


/** Called when boosting starts. */
simulated event StartBoost()
{
	CentaurPlayLocalSound(BoostStartSound);
	PlayCamAnimOnAllControllers(BoostCameraAnim);

	LastBoostTime = WorldInfo.TimeSeconds;

	PlayDirtEffectOnAllControllers();

	//Boost exhaust effect
	if (PSC_TailpipeBoost != None)
	{
		PSC_TailpipeBoost.ActivateSystem();
	}

	//`log("START BOOST");
}
/** Called when boosting finished. */
simulated event EndBoost()
{
	CentaurPlayLocalSound(BoostEndSound);
	NextBoostTime = WorldInfo.TimeSeconds + ReBoostDelay;
	//`log("END BOOST");
}

/** See if we are suppressing rocket firing. */
simulated function bool OverrideBeginFire(byte FireModeNum)
{
	if(bSuppressCannonFire)
	{
		return TRUE;
	}

	return Super.OverrideBeginFire(FireModeNum);
}

simulated function StartFire(byte FireModeNum)
{
	// if a human player is in the passenger seat, he/she gets control of the gun
	if (Seats.length < 2 || Seats[1].SeatPawn == None || PlayerController(Seats[1].SeatPawn.Controller) == None)
	{
		Super.StartFire(FireModeNum);
	}
}

simulated event bool ShouldDoTankSteer()
{
	local GearVehicleSimCar GearCar;

	GearCar = GearVehicleSimCar(SimObj);

	return(	GearCar.bDoTankSteering &&
			Abs(Throttle) < GearCar.TankSteerThrottleThreshold &&
			Abs(GearCar.ActualThrottle) < GearCar.TankSteerThrottleThreshold &&
			Abs(ForwardVel) < 100.0f &&
			!bIsBoosting );
}

simulated function UpdateLookSteerStatus()
{
	local GearPC GPC;

    //Slow speed tank steering and boosting disable look steer
	if (ShouldDoTankSteer() || bIsBoosting)
	{
		if (bUsingLookSteer)
		{
			GPC = GearPC(Controller);
			//Recalculate the relative rotation before starting non-looksteer
			GPC.RelativeToVehicleViewRot.Yaw = GPC.Rotation.Yaw - Rotation.Yaw;
			GPC.RelativeToVehicleViewRot.Roll = 0;
			GPC.RelativeToVehicleViewRot.Pitch = GPC.Rotation.Pitch;
		}
		bUsingLookSteer = False;
	}
	else
	{
	   bUsingLookSteer = True;
	}
}

simulated native function bool ShouldClamp();

/** Used to apply recoil impulse to centaur. */
simulated function VehicleWeaponFireEffects(vector HitLocation, int SeatIndex)
{
	local vector FireLoc, FireDir, RecoilTorqueAxis;
	local rotator FireRot;
	local float AnimScale;
	local GearPC GPC;

	Super.VehicleWeaponFireEffects(HitLocation, SeatIndex);

	// Play firing sound
	PlaySound(CentaurCannonSound);

	// Cam anim
	// Driver
	GPC = GearPC(Controller);
	if(GPC != None && LocalPlayer(GPC.Player) != None)
	{
		AnimScale = GPC.bIsTargeting ? TargetingCannonCamAnimScale : 1.0;
		GearPlayerCamera(GPC.PlayerCamera).PlayCameraAnim(CentaurCannonCamAnim,,AnimScale, 0.05f, 0.1f);
	}

	// Gunner
	if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
		if(GPC != None && LocalPlayer(GPC.Player) != None)
		{
			AnimScale = GPC.bIsTargeting ? TargetingCannonCamAnimScale : 1.0;
			GearPlayerCamera(GPC.PlayerCamera).PlayCameraAnim(CentaurCannonCamAnim,,AnimScale, 0.05f, 0.1f);
		}
	}

	// Trigger firing effect
	PSC_MuzzleEffect.ActivateSystem(FALSE);

	// Trigger light
	TriggerVehicleMuzzleLight(CentaurCannonMuzzleLight, Seats[0].GunSocket[0], CentaurCannonMuzzleLightTime);

	// Play firing anim
	ActionNode.SetAnim('Fire');
	ActionNode.PlayAnim();

	// Apply recoil impulse to vehicle (if not on ice)
	if(CentaurSurface != ECOS_Ice)
	{
		Mesh.GetSocketWorldLocationAndRotation(Seats[0].GunSocket[0], FireLoc, FireRot);
		FireDir = vector(FireRot);


		Mesh.AddImpulse(-FireDir * RecoilLinStrength, FireLoc);

		RecoilTorqueAxis = -FireDir Cross vect(0,0,1);
		Mesh.SetRBAngularVelocity(RecoilTorqueAxis * RecoilAngStrength, TRUE);
	}
}

simulated function BlowupVehicle(Controller Killer)
{
	local Emitter ExplodeEmitter;

	PlayCamAnimOnAllControllers(CentaurDeathCamAnim);

	Super.BlowupVehicle(Killer);

	PlaySound(CentaurDeathSound);

	//Kill the boost exhaust effect
	if (PSC_TailpipeBoost != None)
	{
		PSC_TailpipeBoost.DeactivateSystem();
	}

	//Stop the spotlight sound
	if (AC_SpotlightSound.IsPlaying())
	{
		AC_SpotlightSound.Stop();
	}

	// Shut baird up when dies
	if(BairdRepairAC.IsPlaying())
	{
		BairdRepairAC.Stop();
	}

	ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(CentaurDeathEffect, Location, Rotation);
	ExplodeEmitter.ParticleSystemComponent.ActivateSystem();

	ActionNode.SetAnim('Explode');
	ActionNode.PlayAnim();

	Throttle = 0.0;
	Steering = 0.0;
	OutputSteering = 0.0;
}

/** toggles the spotlight */
simulated function SetSpotlightEnabled(bool bNowEnabled, optional bool bReplicated)
{
	if ((bSpotlightActive != bNowEnabled) || bReplicated)
	{
		bSpotlightActive = bNowEnabled;
		SpotlightToggleTime = WorldInfo.TimeSeconds;

		bSpotlightFlickerOn = bSpotlightActive;
		LightFlickerTimeMin = 0.1;

		SpotlightFlicker();

		// Play the flickering sound
		if(bNowEnabled && SpotLightFlickerSound != None)
		{
			PlaySound(SpotLightFlickerSound, TRUE, TRUE, TRUE, Location, TRUE);
		}
		else if(!bNowEnabled && SpotLightOffSound != none)
		{
			//Stop the ambient spotlight
			if (AC_SpotlightSound.IsPlaying())
			{
				AC_SpotlightSound.Stop();
			}

			PlaySound(SpotLightOffSound, TRUE, TRUE, TRUE, Location, TRUE);
		}
	}
}

/** simulates 'flicker' for the spotlight by randomly turning the light on/off based on time to finish */
simulated function SpotlightFlicker()
{
	local float LightIntensity;
	local float TimeSinceStart, TimeToNext;
	local bool bIsLastTime;

	TimeSinceStart = WorldInfo.TimeSeconds - SpotlightToggleTime;

	//Keep doing this, getting faster each time
	if (TimeSinceStart < LightFlickerDuration)
	{
		TimeToNext = FMin(LightFlickerTimeMin, (LightFlickerDuration - TimeSinceStart) * 0.25f);
		SetTimer( TimeToNext, false, nameof(SpotlightFlicker) );
	}
	else
	{
		bIsLastTime = true;
	}

	// turn on
	if ((bSpotlightFlickerOn && !bIsLastTime) || (bIsLastTime && bSpotlightActive))
	{
		if (bIsLastTime)
		{
			LightIntensity = 10.0f;
			AC_SpotlightSound.Play();
		}
		else
		{
			LightIntensity = 0.1f + FRand() * 2.0f;
		}

		SpotLightComp.SetEnabled(True);
		SpotLightMIC.SetScalarParameterValue('AllLightsBrightness', LightIntensity);

		PSC_Spotlight.SetActive(True);
	}
	// turn off
	else
	{
		SpotLightComp.SetEnabled(False);
		SpotLightMIC.SetScalarParameterValue('AllLightsBrightness', 0.0);

		PSC_Spotlight.SetActive(False);
		PSC_Spotlight.KillParticlesForced();
	}

	//`log("Flicker timesince"@TimeSinceStart@"next:"@TimeToNext@"islast:"@bIsLastTime@"flick:"@bSpotlightFlickerOn@"active:"@bSpotlightActive@"inten:"@LightIntensity);
	bSpotlightFlickerOn = !bSpotlightFlickerOn;
}

/** toggles snow spray */
simulated function SetSnowEffectsActive(bool bNowActive)
{
	if(PSC_SnowSpray == None)
	{
		return;
	}

	bSnowFieldActive = bNowActive;
	// Turn ON
	if (bSnowFieldActive && !PSC_SnowSpray.bIsActive)
	{
		PSC_SnowSpray.ActivateSystem();
	}
	// Turn OFF
	else if (!bSnowFieldActive)
	{
		PSC_SnowSpray.DeactivateSystem();
	}
}

/** Handle the 'Centaur Settings'action. */
function OnCentaurSettings(SeqAct_CentaurSettings Action)
{
	SetSpotlightEnabled(Action.bEnableSpotlight);
	SetSnowEffectsActive(Action.bEnableSnowEffects);

	// set boosting
	bAllowBoosting = Action.bEnableBoost;

	// set cannon firing
	bSuppressCannonFire = Action.bSuppressCannonFire;

	// set max speed
	MaxSpeed = Action.CentaurMaxSpeed;

	// engine sound
	if(bEngineSoundEnabled != !Action.bDisableEngineNoise)
	{
		bEngineSoundEnabled = !Action.bDisableEngineNoise;
		if(Role == ROLE_Authority)
		{
			EnableEngineSound(bEngineSoundEnabled);
		}
	}

	// Copy wheels grip settings
	WheelLongGripScale = Action.CentaurLongGripScale;
	WheelLatGripScale = Action.CentaurLatGripScale;

	WheelExtraGraphicalSpin = Action.CentaurExtraWheelSpin;

	if(CentaurSurface != Action.CentaurOnSurface)
	{
		// This is replicated to client
		CentaurSurface = Action.CentaurOnSurface;
		SetCentaurWheelEffects(CentaurSurface); // update effects locally
	}
}

/** Return offsets to use for vehicle camera */
simulated event GetVehicleViewOffsets(int SeatIndex, bool bSplitScreen, out vector out_Low, out vector out_Mid, out vector out_High)
{
	if(SeatIndex == 0 && bSplitScreen)
	{
		out_Low		= SplitDriverCamViewOffsetLow;
		out_Mid 	= SplitDriverCamViewOffsetMid;
		out_High	= SplitDriverCamViewOffsetHigh;
	}
	else
	{
		out_Low		= Seats[SeatIndex].CameraViewOffsetLow;
		out_Mid 	= Seats[SeatIndex].CameraViewOffsetMid;
		out_High	= Seats[SeatIndex].CameraViewOffsetHigh;
	}
}

simulated event vector GetCameraStart(int SeatIndex)
{
	local vector DesiredCamPos, CamError, CamAcc;
	local float DeltaTime;

	// IsInPain will return true if in a pain-causing volume, which we will assume means impending death
	if (bDeadVehicle || IsInPain())
	{
		// camera stays put after centuar dies
		DesiredCamPos = CamPos;
	}
	else
	{
		DesiredCamPos = GetCameraFocus(SeatIndex);
		//DesiredCamPos = Super.GetCameraStart(SeatIndex);
	}

	// First time, initialise position.
	if(CamPos == vect(0,0,0))
	{
		CamPos = DesiredCamPos;
	}

	DeltaTime = (WorldInfo.TimeSeconds - OldTime);

	if(DeltaTime > 0.0)
	{
		CamError = DesiredCamPos - CamPos;
		// Add spring/damper force
		CamAcc = (CamError * CamStiffness) + (-CamVel * CamDamping);
		// Integrator
		CamVel += (CamAcc * DeltaTime);
		CamPos += (CamVel * DeltaTime);

		// Limit camera to box around desired position
		CamPos.X = FClamp(CamPos.X, DesiredCamPos.X - CamLimits.X, DesiredCamPos.X + CamLimits.X);
		CamPos.Y = FClamp(CamPos.Y, DesiredCamPos.Y - CamLimits.Y, DesiredCamPos.Y + CamLimits.Y);
		CamPos.Z = FClamp(CamPos.Z, DesiredCamPos.Z - CamLimits.Z, DesiredCamPos.Z + CamLimits.Z);

		OldTime = WorldInfo.TimeSeconds;
	}

	return CamPos;
}

simulated function vector GetCameraWorstCaseLoc(int SeatIndex)
{
	local vector WorstLoc;

	if (bDeadVehicle)
	{
		// camera stays put after centuar dies
		WorstLoc = CamPos;
	}
	else
	{
		Mesh.GetSocketWorldLocationAndRotation('CameraOrigin', WorstLoc);
		WorstLoc += (Seats[SeatIndex].WorstCameraLocOffset >> Rotation);
	}

	return WorstLoc;
}

/** Util for ensuring an angle is between +/-180 */
simulated function float UnwindAngle(float InAng)
{
	while(InAng > 180.0)
		InAng -= 360.0;

	while(InAng < -180.0)
		InAng += 360.0;

	return InAng;
}

/** Return the rotation of the vehicle used as basis for vehicle-space camera */
simulated function rotator GetVehicleSpaceCamRotation(float DeltaTime, bool bPassenger)
{
	return Rotation;

	/**
	local float DesiredYaw, YawError, YawAcc;
	local rotator OutRot;

	// Find desired yaw (degrees)
	DesiredYaw = UnwindAngle(Rotation.Yaw * (360.0/65535.0));
	// Find error
	YawError = UnwindAngle(DesiredYaw - CamYawPos);

	// Integrate
	YawAcc = (YawError * CamYawStiffness) + (-CamYawVel * CamYawDamping);
	CamYawVel += (YawAcc * DeltaTime);
	CamYawPos += (CamYawVel * DeltaTime);
	CamYawPos = UnwindAngle(CamYawPos);

	// And use sim yaw in output
	OutRot = Rotation;
	OutRot.Yaw = CamYawPos * (65535.0/360.0);

	return OutRot;
	*/
}

/** Called from C++ when vehicle starts to peel out/spin wheels */
simulated event OnPeelOutBegin()
{
	// If we have a sound, and it has been long enough since we last played it - play now
	if(PeelOutSound != None && (WorldInfo.TimeSeconds - LastPeelOutSound) > 3.f)
	{
		PlaySound(PeelOutSound, TRUE, TRUE, TRUE, Location, TRUE);
		LastPeelOutSound = WorldInfo.TimeSeconds;
	}

	// Cam anim
	PlayCamAnimOnAllControllers(PeelOutCamAnim);
}

/** Change the effects used on the wheels (client and server) */
simulated function SetCentaurWheelEffects(ECentaurOnSurface NewSurface)
{
	local ParticleSystem NewWheelEffect, NewPeelOutEffect, NewBoostEffect;
	local SoundCue TireLoop;
	local int i;

	// Pick correct material
	if(NewSurface == ECOS_Snow)
	{
		NewWheelEffect = SnowWheelEffect;
		NewPeelOutEffect = SnowPeelOutEffect;
		NewBoostEffect = SnowBoostEffect;
		BoostCameraEffectClass = class'Emit_Camera_SnowyDirtLarge';
		TireLoop = SnowTireAudioLoop;
	}
	else if(NewSurface == ECOS_Ice)
	{
		NewWheelEffect = IceWheelEffect;
		NewPeelOutEffect = IcePeelOutEffect;
		NewBoostEffect = IceBoostEffect;
		BoostCameraEffectClass = class'Emit_Camera_WaterSplashLarge';
		TireLoop = IceTireAudioLoop;
	}
	else if(NewSurface == ECOS_Cave)
	{
		NewWheelEffect = CaveWheelEffect;
		NewPeelOutEffect = CavePeelOutEffect;
		NewBoostEffect = CaveBoostEffect;
		BoostCameraEffectClass = class'Emit_Camera_DirtSmall';
		TireLoop = CaveTireAudioLoop;
	}

	// Now apply
	if(NewWheelEffect != None)
	{
		SetAllWheelParticleSystem(NewWheelEffect);

		for(i=0; i<4; i++)
		{
			PSC_PeelOutEffect[i].SetTemplate(NewPeelOutEffect);
		}

		for(i=0; i<2; i++)
		{
			PSC_BoostWheelEffect[i].SetTemplate(NewBoostEffect);
		}
	}

	if (TireLoop != None)
	{
		if (TireAudioLoopAC != None)
		{
			TireAudioLoopAC.FadeOut(0.2f, 0.f);
		}
		TireAudioLoopAC = CentaurPlayLocalSound(TireLoop, 0.2f);
	}
}

/** Update the overlay parameters for a local GearPC */
simulated function UpdateOverlayEffects()
{
	local float Boost;

	Boost = GearVehicleSimCar(SimObj).ActualBoost;
	if(Boost > 0.01)
	{
		// Make sure effect is on
		SetCameraEffectForAllControllers(TRUE);

		// Update MI
		if (OverlayMI != None)
		{
			OverlayMI.SetScalarParameterValue('UseEffect', Boost * OverlayEffectFactor);
			OverlayMI.SetScalarParameterValue('OverallSpread', Boost * OverlaySpreadFactor);
		}
	}
	else
	{
		// Turn off effect if not needed
		SetCameraEffectForAllControllers(FALSE);
	}
}

/** turns on or off the camera effect */
simulated function SetCameraEffect(Controller C, bool bEnabled)
{
	local GearPC PC;
	local LocalPlayer LP;
	local MaterialEffect NewEffect;
	local int i;

	PC = GearPC(C);
	if (PC != None)
	{
		LP = LocalPlayer(PC.Player);
		if (LP != None)
		{
			if (bEnabled && !PC.bHasCentaurOverlay)
			{
				LP.InsertPostProcessingChain(OverlayEffect, INDEX_NONE, true);

				NewEffect = MaterialEffect(LP.PlayerPostProcess.FindPostProcessEffect('MotionEffect'));
				if(NewEffect != None)
				{
					OverlayMI = MaterialInstance(NewEffect.Material);
				}

				PC.bHasCentaurOverlay = TRUE;
			}
			else if (!bEnabled && PC.bHasCentaurOverlay)
			{
				for (i = 0; i < LP.PlayerPostProcessChains.length; i++)
				{
					if (LP.PlayerPostProcessChains[i].FindPostProcessEffect('MotionEffect') != None)
					{
						LP.RemovePostProcessingChain(i);
						i--;
					}
				}

				PC.bHasCentaurOverlay = FALSE;
			}
		}
	}
}

/** turns on or off the camera effect */
reliable client function ClientSetCameraEffect(Controller C, bool bEnabled)
{
	SetCameraEffect(C, bEnabled);
}

/** Control camera effect for all local players */
simulated function SetCameraEffectForAllControllers(bool bEnabled)
{
	local GearPC GPC;

	// Driver
	GPC = GearPC(Controller);
	if(GPC != None && LocalPlayer(GPC.Player) != None)
	{
		SetCameraEffect(GPC, bEnabled);
	}

	// Gunner
	if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
		if(GPC != None && LocalPlayer(GPC.Player) != None)
		{
			SetCameraEffect(GPC, bEnabled);
		}
	}
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.bSpotlightActive = bSpotLightActive;
	Record.bSnowEffectsActive = bSnowFieldActive;
	Record.bAllowBoosting = bAllowBoosting;
	Record.CentaurOnSurface = CentaurSurface;
	Record.WheelLongGripScale = WheelLongGripScale;
	Record.WheelLatGripScale = WheelLatGripScale;
	Record.WheelExtraGraphicalSpin = WheelExtraGraphicalSpin;
	Record.MaxSpeed = MaxSpeed;
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	local int Seat, i;

	SetSpotlightEnabled(Record.bSpotlightActive);
	SetSnowEffectsActive(Record.bSnowEffectsActive);

	CentaurSurface = Record.CentaurOnSurface;
	SetCentaurWheelEffects(CentaurSurface);

	for (Seat = 0; Seat < Seats.Length; Seat++)
	{
		for (i = 0; i < Seats[Seat].TurretControls.Length; i++)
		{
			Seats[Seat].TurretControllers[i].InitTurret(Rotation, Mesh);
		}
	}

	bAllowBoosting = Record.bAllowBoosting;
	WheelLongGripScale = Record.WheelLongGripScale;
	WheelLatGripScale = Record.WheelLatGripScale;
	WheelExtraGraphicalSpin = Record.WheelExtraGraphicalSpin;
	MaxSpeed = Record.MaxSpeed;
}

/** Toggle the engine sound */
simulated function EnableEngineSound(bool bEnabled)
{
	local GearPC GPC;

	if (bEnabled)
	{
		// start engine
		CentaurPlayLocalSound(EngineStartSound);
		CurrentEngineLoopAC = CentaurPlayLocalSound(CentaurGear.EngineLoopCue, 0.2f);

		GPC = GearPC(Driver.Controller);
		if ( (GPC != None) && GPC.IsLocalPlayerController() )
		{
			EnginePlayerAmbientLoopAC = CentaurPlayLocalSound(EnginePlayerAmbientLoopCue, 0.2f);
			EnginePlayerAmbientLoopAC.bAllowSpatialization = FALSE;
		}
	}
	else
	{
		// stop engine
		CentaurPlayLocalSound(EngineStopSound);
		CurrentEngineLoopAC.FadeOut(0.2f, 0.f);

		if (EnginePlayerAmbientLoopAC != None)
		{
			EnginePlayerAmbientLoopAC.FadeOut(0.3f, 0.f);
		}
		if (CurrentEngineLoopAC != None)
		{
			CurrentEngineLoopAC.FadeOut(0.3f, 0.f);
		}
	}
}

simulated function DrivingStatusChanged()
{
	bEngineSoundEnabled = bDriving;
	if(Role == ROLE_Authority)
	{
		EnableEngineSound(bEngineSoundEnabled);
	}

	super.DrivingStatusChanged();
}


/**
* list important centauir variables on canvas.	 HUD will call DisplayDebug() on the current ViewTarget when
* the ShowDebug exec is used
*
* @param	HUD		- HUD with canvas to draw on
* @input	out_YL		- Height of the current font
* @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
*/
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas	Canvas;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255, 255, 255);

	Canvas.DrawText("RPM:" @ DebugLastRPM );
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

//	Canvas.DrawText("Squeal:  LatSlip" @ Abs(Wheels[0].LatSlipAngle) @ "LatThresh" @ SquealLatThreshold @ "LongSlip" @ Abs(Wheels[0].LongSlipRatio) @ "LongThresh" @ SquealThreshold @ Wheels[0].bIsSquealing );
//	out_YPos += out_YL;
//	Canvas.SetPos(4, out_YPos);
}

simulated function SuspensionHeavyShift(float Delta)
{
	if ( (Delta > CentaurSuspensionSoundThreshold) &&
		 (TimeSince(LastSuspensionSoundTime) > CentaurSuspensionSoundMinTimeBetween) )
	{
		CentaurPlayLocalSound(CentaurSuspensionSound);
		LastSuspensionSoundTime = WorldInfo.TimeSeconds;
	}

	super.SuspensionHeavyShift(Delta);
}

/** Called when the driver speaks a line - shut baird up */
simulated event DriverSpeaking()
{
	if(BairdRepairAC.IsPlaying())
	{
		BairdRepairAC.Stop();
	}
	// prevent repair dialogue for a bit
	if (GetRemainingTimeForTimer(nameof(DelayRepairDialogueBegin)) < 10.f)
	{
		SetTimer(10.f,FALSE,nameof(DelayRepairDialogueBegin));
	}
	if (GetRemainingTimeForTimer(nameof(DelayRepairDialogueEnd)) < 10.f)
	{
		SetTimer(10.f,FALSE,nameof(DelayRepairDialogueEnd));
	}

}

event SquishedSomething();

defaultproperties
{
	Begin Object Class=SpotLightComponent Name=SpotLightComponent0
		LightAffectsClassification=LAC_DYNAMIC_AND_STATIC_AFFECTING
		CastShadows=TRUE
		CastStaticShadows=TRUE
		CastDynamicShadows=FALSE
		bForceDynamicLight=FALSE
		UseDirectLightMap=FALSE
		LightingChannels=(BSP=TRUE,Static=TRUE,Dynamic=TRUE,bInitialized=TRUE)
		InnerConeAngle=8.0
		OuterConeAngle=25.0
		Radius=20000.0
		Brightness=5.0
		bEnabled=TRUE
	End Object
	SpotLightComp=SpotLightComponent0
	bSpotlightActive=true

	Begin Object Name=MyLightEnvironment
		ShadowFilterQuality=SFQ_Medium
	End Object

	CamStiffness=(X=25.0,Y=25.0,Z=25.0)
	CamDamping=(X=5.5,Y=5.5,Z=2.2)
	CamLimits=(X=150.0,Y=150.0,Z=150.0)

	ReverseIsForwardThreshold=-0.8

	SupportedEvents.Add(class'SeqEvt_CentaurSquish')
	MinTimeBetweenRunOverEvents=0.5

	LightFlickerDuration=0.5

	EnginePlayerAmbientVolumeRange=(X=0.3f,Y=1.4f)
	EnginePlayerAmbientVelocityRange=(X=0.f,Y=750.f);

}
