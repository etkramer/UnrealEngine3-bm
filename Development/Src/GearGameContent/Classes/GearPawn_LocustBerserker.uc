/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class GearPawn_LocustBerserker extends GearPawn_LocustBerserkerBase
	config(Pawn);


/** Material instance used to glow the Berserker based on damage */
var MaterialInstanceConstant GlowMaterialInstance;

/** Name of the scalar parameter that controls the glow on the material instance */
var Name GlowParameterName;

/** Current glow percentage, used for smooth interpolation */
var transient float GlowPercentage;

/** the "damaged by HOD" particle system **/
var ParticleSystem PS_HitByHOD_;

/** The PSC that is attached to the Mob when being HOD'd**/
var ParticleSystemComponent HODImpactPSC;

/** The PS to play when the berserker is running.  Played at each foot step **/
var ParticleSystem PS_FootstepDust;
var ParticleSystem PS_ChargeDust;

var ParticleSystem PS_ChargeHitPlayer;
var ParticleSystem PS_ChargeHitWall;
var SoundCue FootSound;
/** Name of the scalar parameter that controls the glow on the material instance */
var Name ShimmerSmokeParameterName;
var ParticleSystemComponent PCS_ShimmerSmoke;

/** Breath PSCs **/
var ParticleSystemComponent PSC_Breath_ChargeOne;
var ParticleSystemComponent PSC_Breath_ChargeTwo;
var ParticleSystemComponent PSC_Breath_Idle;
var ParticleSystemComponent PSC_Breath_Dazed;

/** When the berserker is hurt and dying it will play afraid sounds **/

/** we cache the afraid-ness state so we are not constantly polling and calculating **/
var bool bIsAfraid;
var float LastTimePlayAfraidSound;
var float TimeToWaitBeforePlayingAfraidSound;
var AudioComponent Breathing;
var SoundCue BreathingAfraid;
var SoundCue BreathingNormal;

var float PercentOfHealthToPlayAfraidSound;

/** If we just got trowned by a hammer of dawn, scream **/
var bool bHitRecently;
var AudioComponent PainScream;

/** Sound to play when charging **/
var SoundCue ChargeSound;
var SoundCue PrepareToAttackSound;
var AudioComponent AttackSound;
var AudioComponent AttackScream;
var SoundCue AttackScreamSound;

replication
{
	if(Role == ROLE_Authority) bHitRecently;
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	Mesh.AttachComponent( PCS_ShimmerSmoke, 'Spine1' );

	Mesh.AttachComponent( PSC_Breath_ChargeOne, 'Tongue1' );
	Mesh.AttachComponent( PSC_Breath_ChargeTwo, 'Tongue1' );
	Mesh.AttachComponent( PSC_Breath_Idle, 'Tongue1' );
	Mesh.AttachComponent( PSC_Breath_Dazed, 'Tongue1' );

	Breathing = CreateAudioComponent(BreathingNormal, FALSE, TRUE);
	Breathing.bUseOwnerLocation = true;
	Breathing.bAutoDestroy = false;
	AttachComponent(Breathing);
	PainScream = CreateAudioComponent( SoundCue'Locust_Berserker_Efforts.BerserkerEfforts.BerzerkerEfforts_DeathLongCue', FALSE, TRUE);
	AttachComponent(PainScream);
	PainScream.location = location;
	PainScream.bUseOwnerLocation = true;
}


simulated function StopScream()
{
	if(AttackSound != none)
		AttackSound.FadeOut(0.2f,0.0f);
	if(AttackScream != none)
		AttackScream.FadeOut(0.2f,0.0f);
}

simulated function Sniff()
{
	PlaySound(SoundCue'Locust_Berserker_Efforts.BerserkerEfforts.BerzerkerEfforts_SniffingCue');
}

simulated function StartScream()
{
	if(AttackScream == none)
	{
		AttackScream= CreateAudioComponent(AttackScreamSound, false, true);
		AttackScream.bUseOwnerlocation = true;
		Attackscream.bautodestroy = false;
		AttachComponent(AttackScream);
	}
	AttackScream.Play();
}

simulated function PrepareToAttack()
{
	if(AttackSound == none)
	{
		AttackSound = CreateAudioComponent(PrepareToAttackSound, FALSE, TRUE);
		AttackSound.bUseOwnerLocation = true;
		AttackSound.bAutoDestroy = false;
		AttachComponent(AttackSound);
	}
	AttackSound.Play();
	Breathing.FadeOut(0.5,0);
}


/** Event called when DesiredRootRotation has been updated */
simulated function DesiredRootRotationUpdated()
{
	//`log( WorldInfo.TimeSeconds @ GetFuncName() @ "DesiredYawRootRotation:" @ DesiredYawRootRotation );

	if( HasValidDesiredRootRotation() )
	{
		// If Berserker is already doing the charge special move,
		// We now have a proper direction, so let's start charging!
		if( IsDoingSpecialMove(SM_Berserker_Charge) )
		{
			GSM_Berserker_Charge(SpecialMoves[SM_Berserker_Charge]).StartCharging();
		}
	}
}


// Can always melee
simulated function bool CanEngageMelee()
{
	if( IsDoingSpecialMove( SM_Berserker_Charge ) ||
		IsDoingSpecialMove( SM_Berserker_Stunned ) ||
		IsDoingSpecialMove( SM_Berserker_Collide ) )
	{
		return FALSE;
	}

	return TRUE;
}

simulated function StartMeleeAttack()
{
	DoSpecialMove( SM_Berserker_Smash );
}

/**
 * Overridden to handle updating vulnerability stats.
 */
simulated function Tick(float DeltaTime)
{
	local GearAI_Berserker BerzAI;
	Super.Tick(DeltaTime);
	UpdateVulnerability(DeltaTime);

	// check to see if afraid and that some time has passed new sound
	/* New Sound pass removes breathing.
	if( health > 0&& ( (WorldInfo.TimeSeconds - LastTimePlayAfraidSound) > TimeToWaitBeforePlayingAfraidSound))
	{
			PlayBreathSound(bIsAfraid);
	}
	*/
	BerzAI = GearAI_Berserker(Controller);
	if( !bAttackInProgress && BerzAI != none && BerzAI.bPreparingCharge)
	{
		bAttackInProgress = true;
		PrepareToAttack();
	}
}


/** Return whether or not the berserker is afraid **/
simulated function bool ShouldBeAfraid()
{
	local bool Retval;

	// if we are below PercentOfHealthToPlayAfraidSound then we are afraid
	if( ( DefaultHealth * PercentOfHealthToPlayAfraidSound ) > Health)
	{
		Retval = TRUE;
	}
	else
	{
		Retval = FALSE;
	}

	return Retval;
}

/** Plays an breathing sound, takes a bool for if afraid or not */
simulated function PlayBreathSound(bool bPlayAsAfraid)
{
	`log("Playing Afraid"@Health);
	if(bPlayAsAfraid)
	{
		Breathing.SoundCue = BreathingAfraid; //PlaySound( BreathingAfraid );
	}
	else
	{
		Breathing.SoundCue = BreathingNormal; //PlaySound( BreathingNormal );
	}
	Breathing.Play();

	LastTimePlayAfraidSound = WorldInfo.TimeSeconds;
	TimeToWaitBeforePlayingAfraidSound = 5 + Frand() * 5;  // min of 5, max of 10 seconds between afraid sounds
}


/**
 * Handles updating vulnerability timers and decaying the damage count.
 */
simulated function UpdateVulnerability(float DeltaTime)
{
	local float LastGlowPercentage, TargetPercentage;

	if (Role == ROLE_Authority)
	{
		// update the vulnerability timer
		if (RemainingVulnerableTime > 0.f)
		{
			RemainingVulnerableTime -= DeltaTime;
			if (RemainingVulnerableTime <= 0.f)
			{
				ToggleVulnerability(FALSE);
				// next time we're hit, we'll want to base it on the new health:
				bHitRecently = false;
			}
		}
		// reduce the damage count as well
		else if (VulnerabilityDamageCount > 0.f)
		{
			VulnerabilityDamageCount -= DeltaTime * VulnerabilityDamageRecoverRate;
		}
	}
	// update the glow percentage
	LastGlowPercentage = GlowPercentage;
	TargetPercentage = FClamp(VulnerabilityDamageCount/VulnerabilityDamageThreshold,0.f,1.f) * 0.5f;
	// add a pulse if currently vulnerable
	if (bVulnerableToDamage)
	{
		TargetPercentage += 0.5f * ((WorldInfo.TimeSeconds * 3.f) % 1.f);
	}
	// interpolate smoothly
	GlowPercentage = FInterpTo(GlowPercentage,TargetPercentage,DeltaTime,4.f);
	// and apply the glow if it's different than the current value
	if (GlowPercentage != LastGlowPercentage)
	{
		UpdateGlowMaterialInstance();
	}
}

/**
 * Updates the scalar parameter on the material instance to reflect
 * vulnerability damage.
 */
simulated function UpdateGlowMaterialInstance()
{
	if( Mesh != None )
	{
		// create the instance if necessary
		if( GlowMaterialInstance == None )
		{
			GlowMaterialInstance = new(None) class'MaterialInstanceConstant';
			GlowMaterialInstance.SetParent( Mesh.GetMaterial(0) );
			Mesh.SetMaterial( 0, GlowMaterialInstance );
		}

		// set the scalar parameter based on the current damage count
		GlowMaterialInstance.SetVectorParameterValue( GlowParameterName, MakeLinearColor( GlowPercentage, 0.0f, 0.0f, 1.0f) );
		PCS_ShimmerSmoke.SetFloatParameter( ShimmerSmokeParameterName, GlowPercentage );
	}
}

/**
 * Returns true if this is a damage type that we're always vulnerable to.
 */
function bool IsAlwaysVulnerableDamageType(class<DamageType> DamageType)
{
	local int Idx;
	local class<DamageType> CheckDamageType;
	for (Idx = 0; Idx < AlwaysVulnerableDamageTypes.Length; Idx++)
	{
		CheckDamageType = AlwaysVulnerableDamageTypes[Idx];
		if (CheckDamageType != None &&
			(CheckDamageType == DamageType ||
			 ClassIsChildOf(DamageType,CheckDamageType)))
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Returns true if this is a damage type that we're vulnerable to.
 */
function bool IsVulnerableDamageType(class<DamageType> DamageType)
{
	local int Idx;
	local class<DamageType> CheckDamageType;

	for (Idx = 0; Idx < VulnerabilityDamageTypes.Length; Idx++)
	{
		CheckDamageType = VulnerabilityDamageTypes[Idx];
		if (CheckDamageType != None &&
			(CheckDamageType == DamageType ||
			 ClassIsChildOf(DamageType,CheckDamageType)))
		{
			return TRUE;
		}
	}
	return FALSE;
}


function TurnOffHODHitReaction()
{
	if( IsDoingSpecialMove(SM_Berserker_HODHitReaction) )
	{
		// Do for AI through state change
		EndSpecialMove();
	}

	//HODImpactPSC.DeactivateSystem();
}


/** Raam always modifies his damage **/
simulated function SetDontModifyDamage( bool NewValue );


/**
 * Overridden to handle the Berserker's unique damage system.
 */
event TakeDamage(int Damage, Controller instigatedBy, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	local GearAI_Berserker AI;
	local bool bReact;

	AI = GearAI_Berserker(Controller);
	if( AI == None )
		return;

	// check to see if this damage type causes vulnerability
	if( IsVulnerableDamageType(damageType) )
	{
		if( LastHODDamageTime == WorldInfo.TimeSeconds )
			return;
		LastHODDamageTime = WorldInfo.TimeSeconds;

		// We've been hit by the Hammer of Dawn. So if we're not playing the hit reaction animation already, do so
		bReact = AI.NotifyHitByHOD();
		if( bReact && !bHitRecently )
		{
			bHitRecently = true;
			PainScream.Play();
			SetTimer( 1.0f,false,nameof(StopHitRecently) );
		}

		// Set timer to turn off the hit reaction animation
		// If this is called multiple time, it's fine, it's just going to delay it.
		SetTimer( HODHitReactionTime, FALSE, nameof(TurnOffHODHitReaction) );

		// apply the damage
		VulnerabilityDamageCount += Damage;

		// check to see if we've exceeded the vulnerability damage threshold
		if( VulnerabilityDamageCount > VulnerabilityDamageThreshold )
		{
			ToggleVulnerability(TRUE);
		}
	}

	// if vulnerable, or the damage type is one of our weaknesses,
	if (bVulnerableToDamage || IsAlwaysVulnerableDamageType(damageType))
	{
		// take normal damage
		Super.TakeDamage(Damage, instigatedBy, hitlocation, momentum, damageType, HitInfo, DamageCauser);

		bIsAfraid = bIsAfraid || ShouldBeAfraid();
	}
	// spawn some impact effects even tho we are not doing any damage
	else
	{
		//@see  GearPawn DoDamageEffects
		// we need to copy here as there are a billion checks for 0 damage and such that will short
		// circuit things
		PlayTakeHit(Damage, InstigatedBy.Pawn, HitLocation, DamageType, Momentum, HitInfo);
	}
}

function StopHitRecently()
{
	bHitRecently = false;
}

/**
 * Handles turning Berserker vulnerability on/off, fire off any effects, etc.
 */
function ToggleVulnerability( bool bMakeVulnerable )
{
	local GearAI AI;
	local Pawn P;

	// Only process this function if we actually change something.
	if( bVulnerableToDamage == bMakeVulnerable )
	{
		return;
	}

	//debug
	AI = GearAI(Controller);
	if( AI != None )
	{
		`AILog_Ext( GetFuncName()@bMakeVulnerable, 'None', AI );
		AI.MessagePlayer( GetFuncName()@bMakeVulnerable );
	}

	if( bMakeVulnerable )
	{
		bVulnerableToDamage = TRUE;
		RemainingVulnerableTime = VulnerabilityTimer;
		VulnerabilityDamageCount = VulnerabilityDamageThreshold;
	}
	else
	{
		PainScream.FadeOut(0.15f,0.0f);
		bVulnerableToDamage = FALSE;
		RemainingVulnerableTime = 0.f;
		VulnerabilityDamageCount = 0.f;
	}

	if( AI != None && AI.Squad != None )
	{
		// Give Dom a chance to shoot at Berserker while she is vulnerable
		foreach AI.Squad.AllEnemies( class'Pawn', P )
		{
			if( GearAI(P.Controller) != None )
			{
				GearAI(P.Controller).CheckCombatTransition();
			}
		}
	}

	VulnerabilityChanged();
}

/** Notification called when bVulnerableToDamage changes */
simulated function VulnerabilityChanged()
{
	if( bVulnerableToDamage )
	{
		// Scale animations play rate depending on state
		if( AnimTreeRootNode != None )
		{
			AnimTreeRootNode.SetGroupRateScale('RunWalk', HeatedUpSpeedScale);
			AnimTreeRootNode.SetGroupRateScale('Idle', HeatedUpSpeedScale);
		}

		if( WorldInfo.NetMode != NM_DedicatedServer )
		{
			PCS_ShimmerSmoke.ActivateSystem();
		}
	}
	else
	{
		// Scale animations play rate depending on state
		if( AnimTreeRootNode != None )
		{
			AnimTreeRootNode.SetGroupRateScale('RunWalk', 1.f);
			AnimTreeRootNode.SetGroupRateScale('Idle', 1.f);
		}

		if( WorldInfo.NetMode != NM_DedicatedServer )
		{
			PCS_ShimmerSmoke.DeactivateSystem();
		}
	}
}


/**
 *  Event from c++ land that tells us to play a footstep
 **/
simulated event PlayFootStepSound( int FootDown )
{
	//local ParticleSystemComponent PSC_Dust;
	local ScreenShakeStruct Shake;
	local float DistanceMod;
	local GearPC WPC;
	local PlayerController PC;
	local Emitter WE;
	local vector SpawnLocation;
	local rotator SpawnRotator;


	//PSC_Dust= GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( PS_FootstepDust );

	if( FootDown == 1 )
	{
		//Mesh.AttachComponentToSocket( PSC_Dust, 'DustRightFoot' );
		Mesh.GetSocketWorldLocationAndRotation( 'DustRightFoot', SpawnLocation, SpawnRotator );
	}
	else
	{
		//Mesh.AttachComponentToSocket( PSC_Dust, 'DustLeftFoot' );
		Mesh.GetSocketWorldLocationAndRotation( 'DustLeftFoot', SpawnLocation, SpawnRotator );
	}

	WE = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_FootstepDust, SpawnLocation, SpawnRotator );

	PlaySound(FootSound);

	// Tell AI that Berserker has stepped
	if( Role == ROLE_Authority || IsLocallyControlled() )
	{
		MakeNoise( 1.0, 'NOISETYPE_FootStep_Berserker' );
	}

	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		if( ( PC.Pawn != none ) )
		{

			//PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeBigShort);
			WPC = GearPC(PC);
			DistanceMod = VSize(PC.Pawn.Location - Location)/2000;
			if(WPC != none && DistanceMod < 1.0f)
			{
				DistanceMod = 1/DistanceMod;
				DistanceMod /= 4;
				Shake.TimeDuration=0.8;
				Shake.RotAmplitude=Vect(0,0,0);
				Shake.RotFrequency=Vect(0,0,0);
				Shake.LocAmplitude=(Vect(0,1,0)*(DistanceMod))+((Vect(0,0,1)*(5*DistanceMod)));
				Shake.LocFrequency=(Vect(0,1,0)*(DistanceMod))+((Vect(0,0,1)*(DistanceMod*10)));
				Shake.FOVAmplitude=0;
				Shake.FOVFrequency=0;
				WPC.ClientPlayCameraShake(Shake,false);
			}
		}
	}

	//PSC_Dust.ActivateSystem();
	WE.ParticleSystemComponent.ActivateSystem();
}


simulated function PlayChargeHitPlayerEffect( vector HitLocation, vector HitNormal )
{
	local Emitter AnEmitter;
	//`log( "PlayChargeHitPlayerEffect" );
	//DrawDebugCoordinateSystem(HitLocation, Rotator(HitNormal), 100, TRUE);

	AnEmitter= GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ChargeHitPlayer, HitLocation, rotator(HitNormal) );

	AnEmitter.ParticleSystemComponent.ActivateSystem();
	PlaySound(SoundCue'Foley_BodyMoves.BodyMoves.BerserkerArmImpactFlesh_Cue');
}


simulated function PlayChargeHitWallEffect( vector HitLocation, vector HitNormal )
{
	local Emitter AnEmitter;
	local ScreenShakeStruct Shake;
	local float DistanceMod;
	local GearPC WPC;
	local PlayerController PC;
	//`log( "PlayChargeHitWallEffect" );
	//DrawDebugCoordinateSystem(HitLocation, Rotator(HitNormal), 100, TRUE);

	PlaySound(SoundCue'Foley_Crashes.SoundCues.Crash_Car_Large');
	PlaySound(SoundCue'Foley_Crashes.SoundCues.Impact_Stone_LargeLouder');
	AnEmitter= GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ChargeHitWall, HitLocation, rotator(HitNormal) );

	AnEmitter.ParticleSystemComponent.ActivateSystem();

	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		if( ( PC.Pawn != none ) )
		{

			//PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeBigShort);
			WPC = GearPC(PC);
			DistanceMod = VSize(PC.Pawn.Location - Location)/2000;
			if(WPC != none && DistanceMod < 1.0f)
			{
				DistanceMod = 1/DistanceMod;
				DistanceMod /= 4;
				Shake.TimeDuration=0.8;
				Shake.RotAmplitude=Vect(0,0,0);
				Shake.RotFrequency=Vect(0,0,0);
				Shake.LocAmplitude=(Vect(0,1,0)*(DistanceMod))+((Vect(0,0,1)*(5*DistanceMod)));
				Shake.LocFrequency=(Vect(0,1,0)*(DistanceMod))+((Vect(0,0,1)*(DistanceMod*10)));
				Shake.FOVAmplitude=0;
				Shake.FOVFrequency=0;
				WPC.ClientPlayCameraShake(Shake,false);
			}
		}
	}
}


simulated function PlayChargetTakeOffEffect( vector PawnLocation, vector HitNormal )
{
	local Emitter AnEmitter;
	//`log( "PlayChargetTakeOffEffect" );
	//DrawDebugCoordinateSystem(PawnLocation, Rotator(HitNormal), 100, TRUE);

	AnEmitter= GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS_ChargeDust, PawnLocation, rotator(HitNormal) );

	AnEmitter.ParticleSystemComponent.ActivateSystem();

	//PlaySound( ChargeSound );
}

simulated function PlayBreathChargeHitOne()
{
	//`log( "PlayBreathChargeHitOne" );
	PSC_Breath_ChargeOne.ActivateSystem();
}

simulated function PlayBreathChargeHitTwo()
{
	//`log( "PlayBreathChargeHitTwo" );
	PSC_Breath_ChargeTwo.ActivateSystem();
}

simulated function PlayBreathIdle()
{
	//`log( "PlayBreathIdle" );
	PSC_Breath_Idle.ActivateSystem();
}

simulated function PlayBreathDazed()
{
	//`log( "PlayBreathDazed" );
	PSC_Breath_Dazed.ActivateSystem();
}


simulated function bool HasImpactOverride()
{
	if( bVulnerableToDamage )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

simulated function ParticleSystem GetImpactOverrideParticleSystem()
{
	return ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD';
}

// berserker ignores stopping power
function HandleStoppingPower(class<GearDamageType> GearDamageType, Vector Momentum, float DmgDistance);

defaultproperties
{
	Begin Object Name=GearPawnMesh
		bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		SkeletalMesh=SkeletalMesh'Locust_Berserker.berserker'
		AnimTreeTemplate=AnimTree'Locust_Berserker.Berserker_AnimTree'
		PhysicsAsset=PhysicsAsset'Locust_Berserker.berserker_Physics'
		AnimSets(0)=AnimSet'Locust_Berserker.Berserker_AnimSet'
		BlockZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		Scale=1.15f
		Translation=(Z=-125)
		RootMotionMode=RMM_Velocity
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0060.000000
		CollisionHeight=+0120.000000
	End Object

	SightRadius=200.f			// Sight dist really for smell (matches melee range)
	HearingThreshold=4000.0		// Max hearing distance for max loudness noises
	PeripheralVision=-1.f		//(Full 360 "sight"/smell)

	Health=400
	bCanCrouch=FALSE
	bCanStrafe=FALSE
	bDirectHitWall=FALSE
	bLOSHearing=FALSE
	GroundSpeed=200
	AccelRate=+02048.000000
	DesiredYawRootRotation=-1

	bCanDoRun2Cover=FALSE

	GlowParameterName=BerserkerHeat

	VulnerabilityDamageTypes(0)=class'GDT_HOD'
	VulnerabilityDamageTypes(1)=class'GDT_Fire'

	SpecialMoveClasses(SM_Berserker_Smash)			=class'GSM_Berserker_Smash'
	SpecialMoveClasses(SM_Berserker_Charge)			=class'GSM_Berserker_Charge'
	SpecialMoveClasses(SM_Berserker_Stunned)		=class'GSM_Berserker_Stunned'
	SpecialMoveClasses(SM_Berserker_Collide)		=class'GSM_Berserker_Collide'
	SpecialMoveClasses(SM_Berserker_Slide)			=class'GSM_Berserker_Slide'
	SpecialMoveClasses(SM_Berserker_HODHitReaction)	=class'GSM_Berserker_HODHitReaction'
	SpecialMoveClasses(SM_Berserker_Alert)			=class'GSM_Berserker_Alert'

	RightHandSocketName=RightHand
	LeftHandSocketName=LeftHand

	PS_HitByHOD_=ParticleSystem'COG_HOD.Effects.COG_HOD_Damage'

	PS_FootstepDust=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Step_Dust'
	PS_ChargeDust=ParticleSystem'Locust_Berserker.Particles.P_Berserker_StartRun_Dust'
	FootSound = SoundCue'Foley_Footsteps.FootSteps.BoomerFootStepsCue'

	PS_ChargeHitPlayer=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Charge_Hit'
	PS_ChargeHitWall=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Hit_Reaction_Wall_Dust'

	TimeToWaitBeforePlayingAfraidSound=0.0
	PercentOfHealthToPlayAfraidSound=0.20
	BreathingAfraid=SoundCue'Locust_Berserker_Efforts.BerserkerEfforts.BerzerkerEfforts_BreathingAfraidCue'
	BreathingNormal=SoundCue'Locust_Berserker_Efforts.BerserkerEfforts.BerzerkerEfforts_BreathingSlowCue'

	PrepareToAttackSound=SoundCue'Locust_Berserker_Efforts.CompoundPass.ChargeCue'
	ChargeSound=SoundCue'Locust_Berserker_Efforts.CompoundPass.ChargeCue'
	bIsAfraid = false;
	bHitRecently = false;


	ShimmerSmokeParameterName=HeatAmount
	Begin Object Class=ParticleSystemComponent Name=ShimmerSmoke0
	    bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Heated_Shimmer_Smoke'
	End Object
	PCS_ShimmerSmoke=ShimmerSmoke0


	Begin Object Class=ParticleSystemComponent Name=BreathCharge1
	    bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Breath_Charge_Hit_01'
	End Object
	PSC_Breath_ChargeOne=BreathCharge1

	Begin Object Class=ParticleSystemComponent Name=BreathCharge2
	    bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Breath_Charge_Hit_02'
	End Object
	PSC_Breath_ChargeTwo=BreathCharge2

	Begin Object Class=ParticleSystemComponent Name=BreathIdle0
	    bAutoActivate=FALSE
		Template=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Breath_Idle_01'
	End Object
	PSC_Breath_Idle=BreathIdle0

	Begin Object Class=ParticleSystemComponent Name=BreathDazed0
	   bAutoActivate=FALSE
	   Template=ParticleSystem'Locust_Berserker.Particles.P_Berserker_Breath_Dazed_Loop_01'
	End Object
	PSC_Breath_Dazed=BreathDazed0

	AttackScreamSound = SoundCue'Locust_Berserker_Efforts.BerserkerEfforts.BerzerkerEfforts_ScreamMediumCue'

	bUseSimplePhysicalFireStartLoc=true
}
