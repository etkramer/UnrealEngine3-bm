/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 */
class GearPawn_LocustBoomerBase extends GearPawn_LocustBase
	config(Pawn);

/** This is needed to have the cooker correctly cook things that are currently DLO'd from config files **/
var class<Inventory> DefaultWeapon;

/** Weapon Firing animation */
var()	BodyStance	BS_WeaponFire;
/** Melee Attack animation */
var()	BodyStance	BS_MeleeAttack;


/** The PS to play when the boomer is walking.  Played at each foot step **/
var ParticleSystem PS_FootstepDust;

/** Boomer uses a single footstep sound for all matter types **/
var SoundCue FootStepSound;

/** How far away the foot steps can be felt **/
var config float DistanceToFeelFootSteps;

/** How much damage the boomer does when meleeing **/
var config float BoomerMeleeDamage;

/** e.g. "Boom", "Crush", etc. */
var protected const array<SoundCue>		BoomerAttackTelegraphDialogue;

var protected const array<SoundCue>		BoomerChargeTelegraphDialogue;

var protected const array<SoundCue>		BoomerAttackChuckleDialogue;
var protected const float BoomerAttackChuckleChance;

var				Array<Name>		HitReactionAnims;
var				INT				LastHitReactionAnimIndex;
var				FLOAT			LastHitReactionAnimTime;


/** PSCs for the particle emitters on the boomer's feet **/
var transient ParticleSystemComponent PSC_LeftFoot;
var transient ParticleSystemComponent PSC_RightFoot;

var bool bPlayWeaponFireAnim;


simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	PSC_RightFoot = new(self) class'ParticleSystemComponent';
	PSC_RightFoot.SetTemplate( PS_FootstepDust );
	Mesh.AttachComponent( PSC_RightFoot, 'b_MF_Foot_R' );

	PSC_LeftFoot = new(self) class'ParticleSystemComponent';
	PSC_LeftFoot.SetTemplate( PS_FootstepDust );
	Mesh.AttachComponent( PSC_LeftFoot, 'b_MF_Foot_L' );
}


/** Boomers can't be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

/** "chaaarrrrrgeee" */
simulated function TelegraphCharge()
{
	local int RandIdx;

	`SpeechLog(self@"telegraphing CHARGE"@BoomerAttackTelegraphDialogue.length);

	// speak telegraph line
	if (BoomerAttackTelegraphDialogue.length > 0)
	{
		RandIdx = Rand(BoomerChargeTelegraphDialogue.length);

		// make addressee AI's target?
		SpeakLine(None, BoomerChargeTelegraphDialogue[RandIdx], "", 0.f, Speech_Immediate,,,,,, TRUE);
	}
}

simulated function TelegraphAttack()
{
	local int RandIdx;

	`SpeechLog(self@"telegraphing Attack"@BoomerAttackTelegraphDialogue.length);

	// speak telegraph line
	if (BoomerAttackTelegraphDialogue.length > 0)
	{
		RandIdx = Rand(BoomerAttackTelegraphDialogue.length);

		// make addressee AI's target?
		SpeakLine(None, BoomerAttackTelegraphDialogue[RandIdx], "", 0.f, Speech_Immediate,,,,,, TRUE);

		if ( (BoomerAttackChuckleDialogue.length > 0) && (FRand() < BoomerAttackChuckleChance) )
		{
			SetTimer( RandRange(2.5f,3.5f), FALSE, nameof(PlayAttackChuckle) );
		}
	}

	super.TelegraphAttack();
}


simulated function PlayAttackChuckle()
{
	local int RandIdx;

	`SpeechLog(self@"doing ATTACK CHUCKLE"@BoomerAttackTelegraphDialogue.length);

	// speak line
	if ( (BoomerAttackChuckleDialogue.length > 0) && (Health > 0) )
	{
		RandIdx = Rand(BoomerAttackChuckleDialogue.length);

		// make addressee AI's target?
		SpeakLine(None, BoomerAttackChuckleDialogue[RandIdx], "", 0.f, Speech_GUDS,,,,,, FALSE);
	}
}

simulated event PlayFootStepSound( int FootDown )
{
	local PlayerController PC;
	local GearGRI GRI;

	// if we are dead then don't play footstep sounds
	if( Health <= 0 )
	{
		return;
	}

	Super.PlayFootStepSound( FootDown );

	// play a rumble when boomer walks around
	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		if( ( PC.Pawn != none ) && ( VSize(PC.Pawn.Location - Location) < DistanceToFeelFootSteps ) )
		{
			PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeBigShort);
		}
	}

	GRI = GearGRI(WorldInfo.GRI);

	if( GRI.IsEffectRelevant( Instigator, Location, 2000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		// play a dust puff at each foot
		//PSC_Dust = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( PS_FootstepDust );

		if( FootDown == 1 )
		{

			PSC_RightFoot.ActivateSystem();
		}
		else
		{

			PSC_LeftFoot.ActivateSystem();
		}
	}

	if( GRI.IsEffectRelevant( Instigator, Location, 3000, FALSE, GRI.CheckEffectDistance_SpawnWithinCullDistance ) )
	{
		PlaySound( FootStepSound, TRUE, TRUE, FALSE, Location, TRUE );
		//`log( "Boomer ActuallyPlayFootstepSound" @ FootStepSound @ self );
	}


}



/** @see Pawn::WeaponFired */
simulated function WeaponFired(bool bViaReplication, optional vector HitLocation)
{
	Super.WeaponFired(bViaReplication, HitLocation);

	if(bPlayWeaponFireAnim)
	{
		BS_Play(BS_WeaponFire, 1.f, 0.1f, 0.2f, FALSE, TRUE);
	}

	// PlaySound(SoundCue'Locust_Boomer_Chatter.BoomerGroup.BoomerChatter_BoomCue');
	// Play boom shot firing animation
}


/**
 * Function allowing the Pawn to override a weapon melee attack with his own version.
 * @param	out_AnimTime	Play length of the animation played.
 * @return					TRUE to override, FALSE to let weapon play default animations.
 */
simulated function bool OverridePlayMeleeAttackAnim(out float Out_AnimTime)
{
	// Play melee attack animation
	Out_AnimTime = BS_Play(BS_MeleeAttack, 1.f, 0.1f, 0.2f, FALSE, TRUE);
	return TRUE;
}

simulated function bool GetMeleeDamageOverride( out float out_Damage )
{
	out_Damage = BoomerMeleeDamage;
	return TRUE;
}

/**
 * Boomer doesn't override boomshot animsets, this is only for Marcus.
 */
simulated function AddAnimSets(Array<AnimSet> CustomAnimSets);
simulated function UpdateAnimSetList();

/** Raam always modifies his damage **/
simulated function SetDontModifyDamage( bool NewValue );


/**
 * We override this so we can tell the boomshot that boomers are using to not use the melee timer.
 * Boomers use a big swipe attack that needs the MeleeImpact to occurr at the apex of the swipe
 **/
function AddDefaultInventory()
{
	local GearWeapon WW;

	Super.AddDefaultInventory();

	WW = GearWeapon(FindInventoryType(DefaultWeapon));

	if( WW != none )
	{
		WW.bUseMeleeHitTimer=FALSE;
	}
}

/** Override Infantry Pawn */
simulated function AnimSetsListUpdated();

simulated function bool PlayNearMissCringe()
{
	local BodyStance	BS_Cringe;

	return FALSE;

	if( !IsAliveAndWell() )
	{
		return FALSE;
	}

	// Do not overwrite currently playing animations on full body slot.
	// Have a delay between cringes.
	if( FullBodyNode != None && !FullBodyNode.bIsPlayingCustomAnim && !IsReloadingWeapon() )
	{
		BS_Cringe.AnimName[BS_FullBody] = 'ADD_HW_Cringe_Light';
		BS_PlayByDuration(BS_Cringe, 1.f, 0.1f, 0.4f);
		return TRUE;
	}

	return FALSE;
}

/** Play Hit Reaction animation */
simulated function bool TryPlayHitReactionAnimation(ImpactInfo Impact)
{
	// Need a bone name to do something...
	if( Impact.HitInfo.BoneName == '' || TimeSince(NextHitReactionAnimTime) < 0 )
	{
		return FALSE;
	}

	// Disable when doing special moves
	if( IsDoingASpecialMove() || bPlayedDeath )
	{
		return FALSE;
	}

	return PlayHitReactionAnimation(Impact);
}

/** Play Hit Reaction animation */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	local BodyStance	BS_HitReaction;
	local INT			AnimIndex;
	local Name			AnimName;

	// Find an animation to play based on which bone was hit.
	if( MatchNameArray(Impact.HitInfo.BoneName, RightShoulderBones) )
	{
		AnimIndex = 0;
	}
	else if( MatchNameArray(Impact.HitInfo.BoneName, LeftShoulderBones) )
	{
		AnimIndex = 1;
	}
	else
	{
		AnimIndex = 2;
	}

	// Make sure we play two different animations in a row if hit less and a second ago.
	if( LastHitReactionAnimIndex == AnimIndex && TimeSince(LastHitReactionAnimTime) < 1.f )
	{
		AnimIndex += FRand() < 0.5 ? +1 : -1;
		if( AnimIndex >= HitReactionAnims.Length )
		{
			AnimIndex = 0;
		}
		else if( AnimIndex < 0 )
		{
			AnimIndex = HitReactionAnims.Length - 1;
		}
	}

	// If we're playing an animation on those nodes, don't override it.
	if( BodyStanceNodes[BS_Std_Idle_Lower].bIsPlayingCustomAnim || BodyStanceNodes[BS_Std_Up].bIsPlayingCustomAnim )
	{
		return FALSE;
	}

	`DLog("Picked Index:" @ AnimIndex @ HitReactionAnims[AnimIndex] @ "Last Index:" @ LastHitReactionAnimIndex @ "TimeSinceLast:" @ TimeSince(LastHitReactionAnimTime));
	AnimName = HitReactionAnims[AnimIndex];
	LastHitReactionAnimIndex = AnimIndex;
	LastHitReactionAnimTime = WorldInfo.TimeSeconds;
	if( AnimName != '' )
	{
//		BS_HitReaction.AnimName[BS_FullBody] = AnimName;
 		BS_HitReaction.AnimName[BS_Std_Idle_Lower] = AnimName;
 		BS_HitReaction.AnimName[BS_Std_Up] = AnimName;
		NextHitReactionAnimTime = WorldInfo.TimeSeconds + RandRange(0.5f, 1.5f);

		return (BS_Play(BS_HitReaction, 0.67f, 0.2f, 0.33f) > 0.f);
	}

	return FALSE;
}



defaultproperties
{
	bCanPlayPhysicsHitReactions=FALSE // not looking too good, so disabled.
	RightShoulderBones=("b_MF_Clavicle_R","b_MF_Armor_Sho_R","b_MF_UpperArm_R","b_MF_Forearm_R","b_MF_Hand_R")
	LeftShoulderBones=("b_MF_Clavicle_L","b_MF_Armor_Sho_L","b_MF_UpperArm_L","b_MF_Forearm_L","b_MF_Hand_L")
	HitReactionAnims=("AR_HitReact_RtSho","AR_HitReact_LtSho","AR_Boomer_HitReact")

	bCanDoRun2Cover=FALSE
	bCanDBNO=false
	bPlayDeathAnimations=FALSE
	bRespondToExplosions=false

	PhysHRMotorStrength=(X=250,Y=0)

	// Don't shrink limits for boomer on death.
	RagdollLimitScaleTable.Empty()
	HostageHealthBuckets.Empty()

	DrawScale=1.1f

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0060.000000
		CollisionHeight=+0090.000000
	End Object

	CrouchHeight=+60.0
	CrouchRadius=+40.0

	BS_WeaponFire=(AnimName[BS_Std_Up]="AR_Fire",AnimName[BS_Std_Idle_Lower]="AR_Fire")
	BS_MeleeAttack=(AnimName[BS_Std_Up]="AR_Swipe",AnimName[BS_Std_Idle_Lower]="AR_Swipe")

	GUDLineRepeatMin=2.0f

	NoticedGUDSPriority=130
	NoticedGUDSEvent=GUDEvent_NoticedBoomer

	SpecialMoveClasses(SM_DeathAnimFire)=None

	SpecialMoveClasses(SM_TargetMinigun)=None
	SpecialMoveClasses(SM_UnMountMinigun)=None
	SpecialMoveClasses(SM_TargetMortar)=None
	SpecialMoveClasses(SM_UnMountMortar)=None

	SpecialMoveClasses(SM_Emerge_Type1)		=None
	SpecialMoveClasses(SM_Emerge_Type2)		=None

	SpecialMoveClasses(SM_ReviveTeammate)	=None

	SpecialMoveClasses(SM_Walk2Idle)		=None
	SpecialMoveClasses(SM_Run2Idle)			=None

	SpecialMoveClasses(SM_Run2MidCov)		=None
	SpecialMoveClasses(SM_Run2StdCov)		=None
	SpecialMoveClasses(SM_PushOutOfCover)	=None

	SpecialMoveClasses(SM_RoadieRun)				=class'GSM_Boomer_Charge'
	SpecialMoveClasses(SM_CoverRun)					=None
	SpecialMoveClasses(SM_CoverSlip)				=None
	SpecialMoveClasses(SM_StdLvlSwatTurn)			=None
	SpecialMoveClasses(SM_MidLvlJumpOver)			=None
	SpecialMoveClasses(SM_MantleUpLowCover)			=None
	SpecialMoveClasses(SM_MantleDown)				=None
	SpecialMoveClasses(SM_EvadeFwd)					=None
	SpecialMoveClasses(SM_EvadeBwd)					=None
	SpecialMoveClasses(SM_EvadeLt)					=None
	SpecialMoveClasses(SM_EvadeRt)					=None
	SpecialMoveClasses(SM_EvadeFromCoverCrouching)	=None
	SpecialMoveClasses(SM_EvadeFromCoverStanding)	=None

	SpecialMoveClasses(SM_WeaponPickup)		=None
	SpecialMoveClasses(SM_DoorPush)			=None
	SpecialMoveClasses(SM_DoorKick)			=None
	SpecialMoveClasses(SM_PushButton)		=None
	SpecialMoveClasses(SM_Engage_Loop)		=None
	SpecialMoveClasses(SM_Engage_Start)		=None
	SpecialMoveClasses(SM_Engage_End)		=None
	SpecialMoveClasses(SM_Engage_Idle)		=None
	SpecialMoveClasses(SM_Engage_ForceOff)	=None
	SpecialMoveClasses(SM_LadderClimbUp)	=None
	SpecialMoveClasses(SM_LadderClimbDown)	=None
	SpecialMoveClasses(SM_PushObject)		=None

	SpecialMoveClasses(SM_FullBodyHitReaction)				=None

	SpecialMoveClasses(SM_StumbleBackOutOfCover)			=None
	SpecialMoveClasses(SM_StumbleGoDown)					=None
	SpecialMoveClasses(SM_StumbleGoDownFromExplosion)		=None
	SpecialMoveClasses(SM_StumbleGoDownFromCloseRangeShot)	=None
	SpecialMoveClasses(SM_StumbleGetUp)						=None
	SpecialMoveClasses(SM_CoverHead)						=None // class'GSM_Boomer_ShieldDefense' for shield boomer
	SpecialMoveClasses(SM_StumbleFromMelee)					=None
	SpecialMoveClasses(SM_StumbleFromMelee2)				=None

	SpecialMoveClasses(SM_CQC_Killer)				=None
	SpecialMoveClasses(SM_CQC_Victim)				=None
	SpecialMoveClasses(SM_CQCMove_CurbStomp)		=None
	SpecialMoveClasses(SM_CQCMove_PunchFace)		=None
	SpecialMoveClasses(SM_Execution_CurbStomp)		=None
	SpecialMoveClasses(SM_ChainSawHold)				=None
	SpecialMoveClasses(SM_ChainSawAttack)			=None
	SpecialMoveClasses(SM_ChainSawVictim)			=None

	SpecialMoveClasses(SM_CQCMove_Shield)			=None
	SpecialMoveClasses(SM_CQCMove_B)				=None
	SpecialMoveClasses(SM_ChainsawDuel_Leader)		=None
	SpecialMoveClasses(SM_ChainsawDuel_Follower)	=None
	SpecialMoveClasses(SM_ChainsawDuel_Draw)		=None
	SpecialMoveClasses(SM_ChainSawAttack_Object)	=None
	SpecialMoveClasses(SM_ChainSawAttack_Object_NoCamera)	=None

	bTargetingNodeIsInIdleChannel=FALSE
	bPlayWeaponFireAnim=true

	bDebugSpeech=1
}
