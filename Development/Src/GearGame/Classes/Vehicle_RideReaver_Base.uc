/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Vehicle_RideReaver_Base extends GearVehicle
	native(Vehicle)
	config(Pawn)
	abstract;

/** Skeletal mesh component for the driver */
var() editconst SkeletalMeshComponent DriverComp;
/** Skeletal mesh component for the turrets. */
var() editconst SkeletalMeshComponent TurretComp;

// DAMAGE

/** config version of health for ride reaver. */
var()	int						DefaultRideReaverHealth;
/** How quickly ridereaver heals per second. */
var()	config float			HealthRechargeRate;
/** Used to keep track of partial heal points between ticks. */
var		float					PartialHealth;

struct native RideReaverDamageInfo
{
	var()	name	DmgType;
	var()	int		DamageDone;
};

/** Set of damage types and how much damage they do */
var()	config array<RideReaverDamageInfo> DamageInfos;

//

/** FOV to use when targetting on the reaver. */
var()	float			TargetingFOV;
/** Normal FOV */
var()	float			RideReaverDefaultFOV;

var		AnimTree				GunnerAnimTree;

var		array<AnimSet>			GunnerAnimSets;

var	protected const SoundCue			RocketFireSound;
var	protected const SoundCue			RocketFireSound_Player;
var	protected const SoundCue			MinigunFireLoop;
var	protected const SoundCue			MinigunFireLoop_Player;
var const protected SoundCue			MinigunFireStopCue;
var const protected SoundCue			MinigunFireStopCue_Player;
var protected transient AudioComponent	MinigunFiringLoopAC;
var protected const SoundCue			HeatBuildupCue;
var protected AudioComponent			HeatBuildUpAC;


var protected const SoundCue	ReaverTakeOffSound;
var protected const SoundCue	ReaverTakeOffSound_Player;
var protected const SoundCue	ReaverLeanCue_Player;
var protected const SoundCue	ReaverDodgeCue_Player;
/** Random movement sounds, triggered occasionally */
var protected const SoundCue	ReaverMovementCue;

var protected const array<SoundCue>	RandomReaverGrowls;


/** Pain sound */
var	protected const SoundCue		ReaverPainSound;


var		AudioComponent			FacialAudioComp;

var()	editconst SpotLightComponent		FrontSpotlightComp;
var()	editconst SpotLightComponent		RearSpotlightComp;

var()	editconst PointLightComponent		GlowLightComp;

var(MuzzleEffects)	editconst PointLightComponent	RocketMuzzleLight;
var(MuzzleEffects)	float							RocketMuzzleLightTime;

var(MuzzleEffects)	ParticleSystem					RocketMuzzleEffect;
var					ParticleSystemComponent			PSC_RocketMuzzleEffect;

var(MuzzleEffects)	editconst PointLightComponent	MinigunMuzzleLight;
var(MuzzleEffects)	float							MinigunMuzzleLightTime;

var(MuzzleEffects)	ParticleSystem					MinigunMuzzleEffect;
var					ParticleSystemComponent			PSC_MinigunMuzzleEffect;

var					ParticleSystem					PS_StrikeTemplate;


/** Last position passed to AdjustInterpTrackMove... used for worst camera location */
var		Vector					LastOrigInterpLocation;

/** Controls how much the camera moves when dodging */
var()	float					DodgeCamAmount;

var()	float					WeaponTransitionCamRotBlendTime;
var()	float					WeaponTransitionCamTranslationBlendTime;
var()	float					WeaponTransitionCamBlendDuration;

var		float					WeapTransitionRemaining;

var		AnimNodeBlend			BodyAnimBlend;

var		AnimNodeBlendList		FlyingBlendNode;

var		AnimNodeSequence		WalkAnimNode;

/** Node used to play landing/takeoff anims. */
var		AnimNodeSequence		TransitionNode;
var		array<name>				TipSocketName;

/** Time taken to play landing anim */
var()	float			LandingTime;
/** Time take to play flying anim */
var()	float			TakeOffTime;
/** Offset from socket to spawn effect */
var() float		LandEffectZOffset;

/** Howl sound */
var()	SoundCue		ReaverHowlSound;
/** Sound when the reaver lands */
var()	SoundCue		ReaverLandSound;
var()	SoundCue		ReaverLandSound_Player;
/** Scream sound */
var()	SoundCue		ReaverScreamSound;

var()	float					MaxDodgeAmount;
var()	float					DodgeSpeed;

/** Where we want dodge to be - replicated to clients */
var		vector					TargetDodge;
/** Current actual dodge offset */
var		vector					CurrentDodgeAmount;
/** Current speed of dodge movement */
var		vector					CurrentDodgeVel;
/** How much rotation is applied to reaver as we dodge */
var()	float					DodgeRotationAmount;
/** Current rotation due to dodging */
var		vector					DodgeRotVelHistory[10];
/** Slot in history to insert next entry */
var		int						DodgeRotVelSlot;

var		repnotify bool			bPlayLanding;
/** Indicates animation mode (legs up and flying/legs down and walking) */
var		repnotify bool			bLegsWalking;
/** If TRUE, player cannot 'dodge' the reaver. */
var()	bool					bDisableDodging;
/** If TRUE, player cannot dodge up of down */
var()	bool					bDisableVerticalDodging;
/** If TRUE, both spotlights are disabled. */
var()	bool					bSpotlightsDisabled;

/** Cached pointers to trail controls. */
var		SkelControlTrail		TrailControls[6];

/** Additional 'fake' velocity for tentacles. */
var()	vector					TentacleFakeVelocity;
/** TrailRelaxation to use for trail controls */
var()	float					TentacleTrailRelaxation;

/** How long you press against limit for before changing seat */
var()	config float		LookPressSeatChangeTime;
/** Accumulator for holding against limit - switch when reaches LookPressSeatChangeTime */
var		float				CurrentLookPressTime;

var		bool				bWasFacingBackwards;

var		AnimNodeBlend		GunnerSeatBlendNode;
var		GearAnim_Slot		GunnerFullBodySlotNode;

var repnotify vector TurretFlashLocation;

/** If TRUE, gunner starts using the minigun */
var()	bool				bStartOnMinigun;

var		bool				bPendingMoveDriver;

var		transient bool		bHasAlreadySeated;


//////////////////////////////////////////////////////////////////////////
// GORE
/** The Gore SkeletalMeh **/
var SkeletalMesh	GoreSkeletalMesh;
/** The Gore PhysAsset **/
var PhysicsAsset	GorePhysicsAsset;
var()	float	GoreExplosionRadius;
var()	float	GoreExplosionVel;
/** Sound to play when reaver dies */
var		SoundCue		ReaverDeathSound;
/** Particle effect for when reaver dies through shooting. */
var		ParticleSystem	ReaverExplodeEffect;
var		ParticleSystem	ReaverExplodeNoGoreEffect;

///////////////////////////////////////////////////////////////////////////
// POST PROCESS EFFECT
/** MI that is used by the post process, that is modified as Reaver rider looks around */
var(PostProcess)		MaterialInstance	OverlayMI;
/** Centaur additional post process chain */
var(PostProcess)		PostProcessChain	OverlayEffect;


/** Amount to increase 'spreead' in effect as we boost */
var(PostProcess)		float				OverlaySpreadFactor;
/** Amount to blend in effect as we boost */
var(PostProcess)		float				OverlayEffectFactor;


/** Sound used when flying around */
var	protected transient AudioComponent	FlyingSoundLoopAC;
var protected const SoundCue			FlyingSoundLoop;
var protected const SoundCue			FlyingSoundLoop_Player;
/** True to signal a change in driving status such that the flying audio may need to be changed. */
var protected transient bool			bRestartFlyingLoop;

var transient bool bDodgeAudioTempDisabled;


replication
{
	if (bNetDirty)
		TargetDodge, bLegsWalking, bSpotlightsDisabled, TurretFlashLocation, bPlayLanding;
}

cpptext
{
	virtual void TickSpecial(FLOAT DeltaSeconds);
	virtual void AdjustInterpTrackMove(FVector& Pos, FRotator& Rot, FLOAT DeltaTime);
}



simulated protected function DoRandomMovementSound()
{
	ReaverPlayLocalSound(ReaverMovementCue);
	SetTimer( RandRange(7.f,12.f), FALSE, nameof(DoRandomMovementSound) );
};
//simulated protected function DoRandomGrowlSound()
//{
//	ReaverPlayLocalSound(RandomReaverGrowls[Rand(RandomReaverGrowls.length)]);
//	SetTimer( RandRange(7.f,12.f), FALSE, nameof(DoRandomGrowlSound) );
//};


/** Internal. Plays a sound locally (does not replicate), returns the component.  AltPlayerSound is for non-spatialized player-only sounds. */
simulated function protected AudioComponent ReaverPlayLocalSound(SoundCue Sound, optional SoundCue AltPlayerSound, optional AudioComponent AC, optional float FadeInTime)
{
	local GearPC GPC;

	GPC = GearPC(Controller);

	if ( (AltPlayerSound != None) && (GPC != None) && GPC.IsLocalPlayerController()/* && GPC.IsViewingTarget(self)*/ )
	{
		// use nonspatalized alt sound
		if ( (AC == None) || AC.IsPendingKill() || (AC.SoundCue != AltPlayerSound) )
		{
			AC = CreateAudioComponent(AltPlayerSound, FALSE, TRUE);
		}
		if (AC != None)
		{
			AC.bUseOwnerLocation = TRUE;
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


/** Attach components etc - (client + server) */
simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	if (Role == ROLE_Authority)
	{
		Health = DefaultRideReaverHealth;
	}

	Mesh.AttachComponent( DriverComp, 'b_Main' );
	// attach light components
	Mesh.AttachComponentToSocket( FrontSpotlightComp, 'RideCannonLight' );
	Mesh.AttachComponentToSocket( RearSpotlightComp, 'RideMinigunLight' );
	Mesh.AttachComponentToSocket( GlowLightComp, 'MovableLight' );


	// Create PSC for rocket muzzle effect
	PSC_RocketMuzzleEffect = new(self) class'ParticleSystemComponent';
	Mesh.AttachComponentToSocket( PSC_RocketMuzzleEffect, Seats[0].GunSocket[0] );
	PSC_RocketMuzzleEffect.SetTemplate( RocketMuzzleEffect );

	// Create PSC for minigun muzzle effect
	PSC_MinigunMuzzleEffect = new(self) class'ParticleSystemComponent';
	Mesh.AttachComponentToSocket( PSC_MinigunMuzzleEffect, Seats[1].GunSocket[0] );
	PSC_MinigunMuzzleEffect.SetTemplate( MinigunMuzzleEffect );

	FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player, FlyingSoundLoopAC, 0.5f);

	SetTimer( RandRange(7.f,12.f), FALSE, nameof(DoRandomMovementSound) );
	//SetTimer( RandRange(7.f,12.f), FALSE, nameof(DoRandomGrowlSound) );

	SetTimer( 1.f, TRUE, nameof(FixupDriverRelativeLocation) );
}

//HACKS
// Ensure driver relative location is zero so he's right on the seat
simulated function FixupDriverRelativeLocation()
{
	local GearPawn_COGGear P;

	foreach BasedActors( class'GearPawn_COGGear', P )
	{
		//test
		`log( self@GetFuncName()@P@P.RelativeLocation );

		P.SetRelativeLocation( vect(0,0,0) );
	}
}

/** Cache pointers to anim nodes - (client + server) */
simulated event PostInitAnimTree(SkeletalMeshComponent SkelComp)
{
	local int TentacleIdx;

	if (SkelComp == Mesh)
	{
		TrailControls[0] = SkelControlTrail(Mesh.FindSkelControl('AL_Trail'));
		TrailControls[1] = SkelControlTrail(Mesh.FindSkelControl('BL_Trail'));
		TrailControls[2] = SkelControlTrail(Mesh.FindSkelControl('CL_Trail'));
		TrailControls[3] = SkelControlTrail(Mesh.FindSkelControl('AR_Trail'));
		TrailControls[4] = SkelControlTrail(Mesh.FindSkelControl('BR_Trail'));
		TrailControls[5] = SkelControlTrail(Mesh.FindSkelControl('CR_Trail'));

		// We turn off the body animation for the ride reaver
		BodyAnimBlend = AnimNodeBlend(SkelComp.FindAnimNode('BodyAnimBlend'));
		BodyAnimBlend.SetBlendTarget(1.0, 0.0);

		FlyingBlendNode = AnimNodeBlendList(Mesh.FindAnimNode('FlyBlend'));
		FlyingBlendNode.SetActiveChild(0, 0.0);

		WalkAnimNode = AnimNodeSequence(Mesh.FindAnimNode('WalkNode'));
		WalkAnimNode.Rate = 0.0; // for ride reaver- just want to stand and not walk

		TransitionNode = AnimNodeSequence(Mesh.FindAnimNode('Transition'));

		// Turn on trail controls
		for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
		{
			TrailControls[TentacleIdx].SetSkelControlActive(TRUE);
			TrailControls[TentacleIdx].FakeVelocity = TentacleFakeVelocity;
			TrailControls[TentacleIdx].TrailRelaxation = TentacleTrailRelaxation;
		}
	}
}

/** Handle repnotify vars (client only) */
simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'bLegsWalking')
	{
		LegModeChanged();
	}
	else
	if( VarName == 'bPlayLanding' )
	{
		PlayLandingOrTakeoffAnim();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** (client + server) */
simulated function int GetVehicleDefaultHealth()
{
	return DefaultRideReaverHealth;
}

simulated protected function ReenableDodgeAudio()
{
	bDodgeAudioTempDisabled = FALSE;
}

simulated function OnToggleHidden(SeqAct_ToggleHidden Action)
{
	Super.OnToggleHidden(Action);

	// Also hide any driver/gunner
	if(Driver != None)
	{
		Driver.OnToggleHidden(Action);
	}

	if(Seats[1].SeatPawn != None)
	{
		Seats[1].SeatPawn.OnToggleHidden(Action);
	}
}

/** (client + server) */
simulated event Tick(float DeltaTime)
{
	local float Amount, DodgeScale;
	local GearPC GPC;
	local int HealAmount;
	local bool bIsFacingBackwards, bIsPassengerController;
	local Rotator ViewRot;
	local float XDist, YDist, UseInputY;

	Super.Tick(DeltaTime);

	// If desired, swap to back seat
	if(bPendingMoveDriver)
	{
		SwitchToBackSeat(FALSE);
		bPendingMoveDriver = FALSE;
	}

	if(WeapTransitionRemaining > 0.0)
	{
		WeapTransitionRemaining = FMax(WeapTransitionRemaining - DeltaTime, 0.0);

		// Update camera blend speed
		Amount = WeapTransitionRemaining/WeaponTransitionCamBlendDuration;
		VehicleSpaceCamBlendTime = Lerp(default.VehicleSpaceCamBlendTime, WeaponTransitionCamRotBlendTime, Amount);
	}

	// Try and find a controller to look at its input
	if(Controller != None)
	{
		GPC = GearPC(Controller);
		DodgeScale = 1.0;
		bIsPassengerController = FALSE;
	}
	else if(Seats[1].SeatPawn != None)
	{
		GPC = GearPC(Seats[1].SeatPawn.Controller);
		DodgeScale = -1.0; // invert dodge controls when facing backwards
		bIsPassengerController = TRUE;
	}

	// Got a controller, pass input to server
	if(GPC != None)
	{
		// Force gunner to point forwards when first seated
		if(bIsPassengerController)
		{
			bHasAlreadySeated = TRUE;
		}
		else if(!bHasAlreadySeated && !bIsPassengerController)
		{
			ViewRot = GetVehicleSpaceCamRotation(0.0, FALSE);
			ViewRot.Roll = 0;
			GPC.SetRotation(ViewRot);
			bHasAlreadySeated = TRUE;
		}

		// Only do this stuff on owning client
		if(LocalPlayer(GPC.Player) != None)
		{
			// Have to send dodging to server via owned pawn
			UseInputY = GPC.PlayerInput.bInvertMouse ? -GPC.PlayerInput.RawJoyUp : GPC.PlayerInput.RawJoyUp;
			if(bIsPassengerController)
			{
				GearWeaponPawn(Seats[1].SeatPawn).ServerSendTwoFloats(DodgeScale * GPC.PlayerInput.RawJoyRight, UseInputY);
			}
			else
			{
				ServerUpdateDodging(DodgeScale * GPC.PlayerInput.RawJoyRight, UseInputY);
			}

			if (!bDodgeAudioTempDisabled)
			{
				XDist = Abs(CurrentDodgeAmount.X - TargetDodge.X) / MaxDodgeAmount;
				YDist = Abs(CurrentDodgeAmount.Y - TargetDodge.Y) / MaxDodgeAmount;

				if ( (XDist > 1.5f) || (YDist > 1.5f) )
				{
					// do the dodge
					ReaverPlayLocalSound(None, ReaverDodgeCue_Player);
					bDodgeAudioTempDisabled = TRUE;
					SetTimer( 0.5f, FALSE, nameof(ReenableDodgeAudio) );
				}
				else if ( (XDist > 0.8f) || (YDist > 0.8f) )
				{
					// do the lean
					ReaverPlayLocalSound(None, ReaverLeanCue_Player);
					bDodgeAudioTempDisabled = TRUE;
					SetTimer( 0.5f, FALSE, nameof(ReenableDodgeAudio) );
				}
			}

			if(bPushingAgainstViewLimit)
			{
				CurrentLookPressTime += DeltaTime;
				if(CurrentLookPressTime >= LookPressSeatChangeTime)
				{
					if(bIsPassengerController)
					{
						GearWeaponPawn(Seats[1].SeatPawn).ServerBecomeDriver();
					}
					else
					{
						SwitchToBackSeat(TRUE);
					}

					if(GPC.MainPlayerInput != None)
					{
						GPC.MainPlayerInput.YawAccelPct = 0.0;
					}

					CurrentLookPressTime = 0.0;
				}
			}
			else
			{
				CurrentLookPressTime = 0.0;
			}
		}

		UpdateOverlayEffects( GPC );
	}

	// We are facing backwards if the zeroth bit of the seatmask is not set
	bIsFacingBackwards = (SeatMask == 2);

	// forward->backward transition
	if(bIsFacingBackwards && !bWasFacingBackwards)
	{
		GunnerFullBodySlotNode.PlayCustomAnim('Passenger_Fwd2Bwd', 1.0, 0.0, 0.2, FALSE, TRUE);
		GunnerSeatBlendNode.SetBlendTarget(1.0, 0.1);
	}
	// backward->forward transition
	else if(!bIsFacingBackwards && bWasFacingBackwards)
	{
		GunnerFullBodySlotNode.PlayCustomAnim('Passenger_Bwd2Fwd', 1.0, 0.0, 0.2, FALSE, TRUE);
		GunnerSeatBlendNode.SetBlendTarget(0.0, 0.1);
	}
	bWasFacingBackwards = bIsFacingBackwards;

	// Handle healing
	if(Role == ROLE_Authority)
	{
		// If hurt, heal
		if(Health < DefaultRideReaverHealth)
		{
			// We need to save off partially healed points
			PartialHealth += DeltaTime * HealthRechargeRate;
			if ( PartialHealth >= 1.f )
			{
				HealAmount = FFloor(PartialHealth);
				PartialHealth -= HealAmount;
				Health += HealAmount;
			}
		}
		// Not healing, reset heal counter
		else
		{
			PartialHealth = 0.0;
		}
	}

	if (bRestartFlyingLoop)
	{
		FlyingSoundLoopAC.FadeOut(0.2f, 0.f);
		FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player,, 0.2f);
		bRestartFlyingLoop = FALSE;
	}
}

event bool DriverLeave(bool bForceLeave)
{
	local bool	 bLeft;
	local GearPC GPC;

	GPC = GearPC(Controller);
	bLeft = Super.DriverLeave( bForceLeave );
	if( bLeft && GPC != None )
	{
		ClientSetCameraEffect( GPC, FALSE );
	}
	return bLeft;
}

/** Update the overlay parameters for a local GearPC */
simulated function UpdateOverlayEffects( GearPC PC )
{
	local Vector  FwdDir, CamLoc;
	local Rotator CamRot;
	local float	  DotP;

	// Only do this for local players
	if( PC == None || OverlayMI == None || LocalPlayer(PC.Player) == None)
		return;

	FwdDir = Vector(Rotation);
	PC.GetPlayerViewPoint( CamLoc, CamRot );

	DotP = Abs(FwdDir DOT Vector(CamRot));
	OverlayMI.SetScalarParameterValue( 'UseEffect',		DotP * OverlayEffectFactor );
	OverlayMI.SetScalarParameterValue( 'OverallSpread', DotP * OverlaySpreadFactor );
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

/** turns on or off the camera effect */
simulated function SetCameraEffect(Controller C, bool bEnabled)
{
	local GearPC PC;
	local LocalPlayer LP;
	local MaterialEffect NewEffect;
	local int i;
	local MaterialInstanceConstant CurrentMI;

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
					// Create instance for this reaver if necessary
					if(OverlayMI == None)
					{
						CurrentMI = MaterialInstanceConstant(NewEffect.Material);
						OverlayMI = new(None) class'MaterialInstanceConstant';
						OverlayMI.SetParent(CurrentMI);
					}

					NewEffect.Material = OverlayMI;
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


simulated function BlowupVehicle(Controller Killer)
{
	Super.BlowupVehicle(Killer);

	// hide driver and turret
	DriverComp.DetachFromAny();
	TurretComp.DetachFromAny();

	// gib!
	ReaverGibbage();
}

simulated function ReaverGibbage()
{
	local Emitter ExplodeEmitter;

	InitVehicleRagdoll(GoreSkeletalMesh, GorePhysicsAsset, vect(0,0,0), TRUE);

	// Freeze the light environment state at the state when the pawn was gibbed; this prevents the light environment being computed
	LightEnvironment.bDynamic = FALSE;

	//DrawDebugSphere(HitLocation, GoreExplosionRadius, 10, 255, 0, 0, TRUE);
	Mesh.AddRadialImpulse(Location, GoreExplosionRadius, GoreExplosionVel, RIF_Linear, TRUE);

	// Spawn emitter
	if(  WorldInfo.GRI.ShouldShowGore() )
	{
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(ReaverExplodeEffect, Location, rot(0,0,0));
	}
	else
	{
		ExplodeEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(ReaverExplodeNoGoreEffect, Location, rot(0,0,0));
	}

	ExplodeEmitter.ParticleSystemComponent.ActivateSystem();

	FlyingSoundLoopAC.FadeOut(0.5f, 0.f); // Stop flying sound
}

/** server only */
function PassengerSentTwoFloats(float A, float B)
{
	ServerUpdateDodging(A, B);
}

/** (server only) */
reliable server function ServerUpdateDodging(float DodgeX, float DodgeY)
{
	local float DodgeMag;

	if(!bDisableDodging)
	{
		if(bDisableVerticalDodging)
		{
			DodgeY = 0.0;
		}

		DodgeMag = Sqrt((DodgeX*DodgeX) + (DodgeY*DodgeY));
		if(DodgeMag > 1.0)
		{
			DodgeX /= DodgeMag;
			DodgeY /= DodgeMag;
		}

		TargetDodge.X = DodgeX * MaxDodgeAmount;
		TargetDodge.Y = DodgeY * MaxDodgeAmount;
	}
	else
	{
		TargetDodge.X = 0.0;
		TargetDodge.Y = 0.0;
	}
}

simulated function SitDriver( GearPawn WP, int SeatIndex)
{
	local int i;
	local AnimSet Anim;

	Super.SitDriver(WP, SeatIndex);

	SetCameraEffect(WP.Controller, TRUE);

	WP.HolsterWeapon();
	WP.bCanDBNO = FALSE;

	WP.Mesh.SetTranslation(vect(0,0,0));

	for(i=0; i<GunnerAnimSets.length; i++)
	{
		Anim = GunnerAnimSets[i];
		WP.KismetAnimSets[0] = Anim;
	}
	WP.UpdateAnimSetList();

	WP.Mesh.SetAnimTreeTemplate(GunnerAnimTree);

	GunnerSeatBlendNode = AnimNodeBlend(WP.Mesh.FindAnimNode('SeatBlend'));
	GunnerFullBodySlotNode = GearAnim_Slot(WP.Mesh.FindAnimNode('FullBodySlot'));

	if(bStartOnMinigun)
	{
		`log("START ON MINIGUN");
		bPendingMoveDriver = TRUE;
		bStartOnMinigun = FALSE;
	}
}

/** (client + server) */
simulated event float GetCameraFOV(int SeatIndex)
{
	local Controller UseC;

	// Find controller to look at for targetting
	if(SeatIndex == 0)
	{
		UseC = Controller;
	}
	else
	{
		UseC = Seats[SeatIndex].SeatPawn.Controller;
	}

	if(UseC != None && GearPC(UseC).bIsTargeting)
	{
		return TargetingFOV;
	}
	else
	{
		return RideReaverDefaultFOV;
	}
}

/** Move driver to seat 1 */
reliable server function SwitchToBackSeat(bool bMatchYaw)
{
	local GearPC PC;
	local Rotator ViewRot;

	PC = GearPC(Controller);

	if(!bMatchYaw)
	{
		ViewRot = GetVehicleSpaceCamRotation(0.0, TRUE);
		ViewRot.Roll = 0;
		//PC.SetRotation(ViewRot);
		PC.ClientSetJustControllerRotation(ViewRot);
	}

	ChangeSeat(Controller, 1);
}

/** Move passenger C to seat 0 */
reliable server function SwitchToFrontSeat(Controller C, bool bMatchYaw)
{
	local GearPC PC;
	local Rotator ViewRot;

	PC = GearPC(C);

	if(!bMatchYaw)
	{
		ViewRot = GetVehicleSpaceCamRotation(0.0, FALSE);
		ViewRot.Roll = 0;
		//PC.SetRotation(ViewRot);
		PC.ClientSetJustControllerRotation(ViewRot);
	}

	ChangeSeat(C, 0);
}

simulated function bool ShouldShowWeaponOnHUD(GearPC PC)
{
	local GearWeaponPawn WP;

	WP = GearWeaponPawn(PC.Pawn);
	return (WP != None && WP.MyVehicle == self);
}

/** Called when a client presses X while in the vehicle. (server only) */
reliable server function ServerPressedX()
{
	SwitchToBackSeat(FALSE);
}

/** Called when a passenger presses X */
simulated function PassengerPressedX(Controller C)
{
	SwitchToFrontSeat(C, FALSE);
}

function PassengerBecomeDriver(Controller C)
{
	SwitchToFrontSeat(C, TRUE);
}

/** Use different base rotation for passenger */
simulated function rotator GetVehicleSpaceCamRotation(float DeltaTime, bool bPassenger)
{
	local quat VehicleQuat;

	if(bPassenger)
	{
		// Apply 180-degree rotation in vehicle space
		VehicleQuat = QuatProduct(QuatFromRotator(Rotation), QuatFromRotator(rot(0,32768,0)));
		return QuatToRotator(VehicleQuat);
	}
	else
	{
		return Rotation;
	}
}

/** Used so we can override damage using our own table */
simulated event TakeDamage(int Damage, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local int DamageDone, i;
	local bool bFoundDmg;
	local GearPC GPC;

	// make sure we are the server and we're alive
	if (Role < ROLE_Authority || Health <= 0 || InGodMode() || Seats[1].SeatPawn.InGodMode() )
	{
		return;
	}

	// Ignore yourself doing damage
	if((EventInstigator != None) && (EventInstigator == Controller || EventInstigator == Seats[1].SeatPawn.Controller))
	{
		return;
	}

	// Look through damage mapping
	for(i=0; i<DamageInfos.length; i++)
	{
		if(DamageType.Name == DamageInfos[i].DmgType)
		{
			DamageDone = DamageInfos[i].DamageDone;
			bFoundDmg = TRUE;
			break;
		}
	}

	// If we found it, apply damage now
	if(bFoundDmg)
	{
		Super.TakeDamage(DamageDone, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);
		if (DamageDone > 0)
		{
			for (i = 0; i < Seats.Length; i++)
			{
				if (Seats[i].SeatPawn != None)
				{
					GPC = GearPC(Seats[i].SeatPawn.Controller);
					if (GPC != None && (EventInstigator == None || EventInstigator.GetTeamNum() != GPC.GetTeamNum()))
					{
						GPC.ClientAddHitLocator(EventInstigator.Pawn,DamageDone,HitLocation);
					}
				}
			}
		}
		//`log("TAKEDAM:"@Health@DamageDone@DamageType@EventInstigator@DamageCauser);
	}
	else
	{
		`log("RIDEREAVER: Unhandled DmgType"@DamageType.Name@Damage);
		foreach WorldInfo.AllControllers(class'GearPC', GPC)
		{
			GPC.ClientMessage("RIDEREAVER: Unhandled DmgType"@DamageType.Name@Damage);
		}
	}
}

simulated function VehicleWeaponFireEffects(vector HitLocation, int SeatIndex)
{
	local float HitDistance;

	Super.VehicleWeaponFireEffects(HitLocation, SeatIndex);

	if(SeatIndex == 0)
	{
		ReaverPlayLocalSound(RocketFireSound, RocketFireSound_Player);

		PSC_RocketMuzzleEffect.ActivateSystem(FALSE);

		TriggerVehicleMuzzleLight(RocketMuzzleLight, Seats[0].GunSocket[0], RocketMuzzleLightTime);
	}
	else if(SeatIndex == 1)
	{
		if (MinigunFiringLoopAC == None)
		{
			MinigunFiringLoopAC = ReaverPlayLocalSound(MinigunFireLoop, MinigunFireLoop_Player);
		}
		if (HeatBuildUpAC == None)
		{
			HeatBuildUpAC = ReaverPlayLocalSound(HeatBuildupCue);
		}

		HitDistance = VSize(HitLocation - Location);
		if (Seats[1].Gun == None && Seats[1].SeatPawn != None)
		{
			Seats[1].Gun = GearVehicleWeapon(Seats[1].SeatPawn.Weapon);
		}
		if (Seats[1].Gun != None)
		{
			Seats[1].Gun.SpawnTracerEffect( HitLocation, HitDistance );
		}

		PSC_MinigunMuzzleEffect.ActivateSystem(FALSE);

		TriggerVehicleMuzzleLight(MinigunMuzzleLight, Seats[1].GunSocket[0], MinigunMuzzleLightTime);
	}
}

simulated function VehicleWeaponStoppedFiring( bool bViaReplication, int SeatIndex )
{
	// stop minigun firing loop
	if (SeatIndex == 1)
	{
		if (MinigunFiringLoopAC != None)
		{
			MinigunFiringLoopAC.FadeOut(0.2f, 0.f);
			MinigunFiringLoopAC = None;
		}

		if (HeatBuildUpAC != None)
		{
			HeatBuildUpAC.FadeOut(0.2f, 0.0f);
			HeatBuildUpAC = None;
		}

		ReaverPlayLocalSound(MinigunFireStopCue, MinigunFireStopCue_Player);
	}




	super.VehicleWeaponStoppedFiring(bViaReplication, SeatIndex);
}

/** Handler for Matinee wanting to play FaceFX animations in the game. */
simulated event bool PlayActorFaceFXAnim(FaceFXAnimSet AnimSet, String GroupName, String SeqName, SoundCue SoundCueToPlay )
{
	return DriverComp.PlayFaceFXAnim(AnimSet, SeqName, GroupName, SoundCueToPlay);
}

/** Handler for Matinee wanting to stop FaceFX animations in the game. */
simulated event StopActorFaceFXAnim()
{
	DriverComp.StopFaceFXAnim();
}

/** Used to let FaceFX know what component to play dialogue audio on. */
simulated event AudioComponent GetFaceFXAudioComponent()
{
	return FacialAudioComp;
}

simulated event DriverSpeakLine(SoundCue Audio, bool bSuppressSubtitles)
{
	local AudioComponent SpeakingComp;

	// If we have a FaceFX animation hooked up, play that instead
	// If the Face FX animation fails, fall back to just playing the sound. A log Warning will be issued in that case
	if( Audio.FaceFXAnimName != "" && PlayActorFaceFXAnim(Audio.FaceFXAnimSetRef, Audio.FaceFXGroupName, Audio.FaceFXAnimName, Audio) )
	{
		SpeakingComp = GetFaceFXAudioComponent();
		//	`log(GetFuncName()@"played line (FaceFX)"@CurrentSpeakLineParams.Audio);
		SpeakingComp.bSuppressSubtitles = bSuppressSubtitles;
		// jack priority for scripted lines
		SpeakingComp.bAlwaysPlay = TRUE;
	}
	else
	{
		SpeakingComp = CreateAudioComponent( Audio, FALSE, TRUE );
		if (SpeakingComp != None)
		{
			AttachComponent(SpeakingComp);

			SpeakingComp.bAutoDestroy = TRUE;
			SpeakingComp.Location = Location;
			SpeakingComp.bSuppressSubtitles = bSuppressSubtitles;

			//SpeakingComp.SubtitlePriority = CurrentSpeakLineParams.Priority * SubtitlePriorityScale;

			// jack priority for scripted lines
			SpeakingComp.bAlwaysPlay = TRUE;

			SpeakingComp.FadeIn(0.2f, 1.f);
		}
	}
}

/** Handles leg mode (server only) */
simulated function OnRideReaverLegsMode(SeqAct_RideReaverLegsMode Action)
{
	// Flying
	if(Action.InputLinks[0].bHasImpulse)
	{
		bLegsWalking = FALSE;
	}
	// Walking
	else if(Action.InputLinks[1].bHasImpulse)
	{
		bLegsWalking = TRUE;
	}

	// To update anim on server
	LegModeChanged();
}

/** Called when bLegsFlying to update anim state. */
simulated function LegModeChanged()
{
	if(bLegsWalking)
	{
		FlyingBlendNode.SetActiveChild(2, 0.5);
	}
	else
	{
		FlyingBlendNode.SetActiveChild(0, 0.5);
	}
}

/** (server only) */
simulated function OnRideReaverSetOptions(SeqAct_RideReaverSetOptions Action)
{
	// Copy options from kismet action
	bDisableDodging = Action.bDisableDodging;
	bDisableVerticalDodging = Action.bDisableVerticalDodging;
	bSpotlightsDisabled = Action.bDisableSpotlight;
}

function OnReaverLand(SeqAct_ReaverLand Action)
{
	bPlayLanding = !Action.bTakeOff;
	PlayLandingOrTakeoffAnim();
}

simulated function PlayLandingOrTakeoffAnim()
{
	local int TentacleIdx;
	local float DesiredRate;

	if( bPlayLanding )
	{
		// Change blend to walking mode slowly
		FlyingBlendNode.SetActiveChild(1, 0.1);
		TransitionNode.SetAnim('Land');
		DesiredRate = 1.9 / LandingTime; // WE want to land at the 1.9 sec mark of the landing anim
		TransitionNode.PlayAnim(FALSE, DesiredRate, 0.0);

		// Turn off trail controls
		for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
		{
			TrailControls[TentacleIdx].SetSkelControlActive(FALSE);
		}

		// Play a howl for landing
		ReaverPlayLocalSound(ReaverHowlSound);
		ReaverPlayLocalSound(ReaverLandSound, ReaverLandSound_Player);

		SetTimer( LandingTime, FALSE, nameof(FinishLandingOrTakeoffAnim) );
	}
	else
	{
		// Change blend to transition node
		FlyingBlendNode.SetActiveChild(1, 0.1);
		TransitionNode.SetAnim('takeoff_start'); // play first part OF take-off anim
		TransitionNode.PlayAnim(FALSE, 1.0, 0.0);

		// Play scream as it flies off
		ReaverPlayLocalSound(ReaverScreamSound);
		ReaverPlayLocalSound(ReaverTakeOffSound, ReaverTakeOffSound_Player);

		// Pause before part 2
		SetTimer( TransitionNode.AnimSeq.SequenceLength, FALSE, nameof(FinishLandingOrTakeoffAnim) );
	}
}

simulated function FinishLandingOrTakeoffAnim()
{
	local int TentacleIdx;
	local float DesiredRate;
	local vector ImpactLoc;
	local Emitter ImpactEffect;

	if( bPlayLanding )
	{
		FlyingBlendNode.SetActiveChild(2, 0.1);
		WalkAnimNode.PlayAnim(TRUE, 0.0, 0.0);

		// Spawn dust where each tentacle hits
		for( TentacleIdx = 0; TentacleIdx < TipSocketName.Length; TentacleIdx++ )
		{
			Mesh.GetSocketWorldLocationAndRotation(TipSocketName[TentacleIdx], ImpactLoc);
			ImpactLoc += (LandEffectZOffset * vect(0,0,1));

			ImpactEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(PS_StrikeTemplate, ImpactLoc, rotator(vect(0,0,1)));
			ImpactEffect.ParticleSystemComponent.ActivateSystem();
		}

		FlyingSoundLoopAC.FadeOut(0.3f, 0.f);
	}
	else
	{
		TransitionNode.SetAnim('Takeoff'); // Play second part OF take-off anim while moving up
		DesiredRate = TransitionNode.AnimSeq.SequenceLength / TakeOffTime;
		TransitionNode.PlayAnim(FALSE, DesiredRate, 0.0);

		// Turn on trail controls
		for(TentacleIdx=0; TentacleIdx<6; TentacleIdx++)
		{
			TrailControls[TentacleIdx].SetSkelControlActive(TRUE);
		}

		FlyingBlendNode.SetActiveChild(0, 0.4);

		FlyingSoundLoopAC = ReaverPlayLocalSound(FlyingSoundLoop, FlyingSoundLoop_Player, FlyingSoundLoopAC, 0.5f);
	}
}

simulated function DrivingStatusChanged()
{
	bRestartFlyingLoop = TRUE;
	super.DrivingStatusChanged();
}

defaultproperties
{
	// so the matinees can link to the reaver correctly - could be done differently if bNoDelete causes pain
	bNoDelete=true

	// matinee controlled, so shouldn't need any of this stuff
	bReplicateMovement=false
	bUpdateSimulatedPosition=false

	Begin Object Class=SkeletalMeshComponent Name=DriverSkel
		LightEnvironment=MyLightEnvironment
		ShadowParent=SVehicleMesh
		CollideActors=FALSE
		Rotation=(Yaw=16384)
	End Object
	DriverComp=DriverSkel

	Begin Object Class=SkeletalMeshComponent Name=TurretSkel
		LightEnvironment=MyLightEnvironment
		CollideActors=FALSE
		ParentAnimComponent=SVehicleMesh
		ShadowParent=SVehicleMesh
	End Object
	Components.Add(TurretSkel)
	TurretComp=TurretSkel

	Begin Object Class=AudioComponent Name=FaceAudioComponent
	End Object
	FacialAudioComp=FaceAudioComponent
	Components.Add(FaceAudioComponent)

	Begin Object Class=SpotLightComponent Name=FrontSpot
	End Object
	FrontSpotlightComp=FrontSpot

	Begin Object Class=SpotLightComponent Name=RearSpot
	End Object
	RearSpotlightComp=RearSpot

	Begin Object Class=PointLightComponent Name=GlowLight
	End Object
	GlowLightComp=GlowLight

	bVehicleSpaceViewLimits=TRUE
	bVehicleSpaceCamera=FALSE
	//bOnlyInheritVehicleSpaceYaw=TRUE
	bShowWeaponOnHUD=FALSE

	WeaponTransitionCamRotBlendTime=0.5
	WeaponTransitionCamTranslationBlendTime=1.0
	WeaponTransitionCamBlendDuration=1.0

	TipSocketName(0)="AL_Tip"
	TipSocketName(1)="BL_Tip"
	TipSocketName(2)="CL_Tip"
	TipSocketName(3)="AR_Tip"
	TipSocketName(4)="BR_Tip"
	TipSocketName(5)="CR_Tip"

	LandingTime=1.5
	TakeOffTime=1.5

	LandEffectZOffset=50.0
	DodgeCamAmount=0.5

	RideReaverDefaultFOV=80.0
	TargetingFOV=30.0
}
