/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_LocustSire extends GearPawn_LocustSireBase
	config(Pawn);

/** How much health to give back to sire when he falls down */
var config int KnockdownRestoreHealth;

/** sound that plays when the sire first sees someone and charges after them */
var SoundCue ChargeSound;
/** sound that plays when the sire does his melee attack */
var SoundCue AttackSound;
/** sound that plays when we ragdoll down */
var SoundCue KnockedDownSound;
/** sound that plays all the time we're alive */
var instanced AudioComponent BreathingAudioComp;
/** sire uses a single footstep sound for all matter types **/
var SoundCue FootStepSound;


var float LastChargeSoundPlayTime;

/** Indicates taht on next tick, sire should fall down */
var bool	bPendingKnockdown;
/** Name of bone at top of left arm - hits below this will count as 'left arm' */
var	array<name>	LeftArmBoneNames;
/** Current health of left arm */
var	config int		LeftArmHealth;

/** Name of bone at top of right arm - hits below this will count as 'right arm' */
var	array<name>	RightArmBoneNames;

/** Current health of left arm */
var	config int		RightArmHealth;

var name LastArmhitBone;

// to re-break on ragdoll recovery
var array<name> BrokenBoneNames;

/** Min time that sire will stay down after falling down. */
var config float SireKnockDownMinDuration;
/** Max time that sire will stay down after falling down. */
var config float SireKnockDownMaxDuration;
/** How much angular velocity to apply to ragdoll when falling down. */
var config float SireKnockDownSpinVel;
/** How many times the sire can get back of from being knocked down (irrespective of arm lossage) **/
var config int SireMaxNumberOfKnockDowns;
/** Num times this sire has been knocked down **/
var transient int NumTimesKnockedDown;


// breath/drool particle effects
var ParticleSystem			IdleEffect;
var ParticleSystem			RunEffect;
var ParticleSystemComponent EffectComp;
var name					EffectSocket;
var bool					bPlayingRunEffect;

var name					FaceHitReactionAnimnName,LegHitReactionAnimName;
var name					RightArmShotOffAnim,LeftArmShotOffAnim;


replication
{
	if(Role == ROLE_Authority)
		LastArmhitBone;
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();

	SetTimer(0.75f,TRUE,nameof(UpdateParticleEffects));
}

simulated function UpdateParticleEffects()
{
	if(EffectComp == none)
	{
		EffectComp = new(self) class'ParticleSystemComponent';
		EffectComp.SetTemplate(IdleEffect);
		Mesh.AttachComponentToSocket(EffectComp,EffectSocket);
		EffectComp.SetHidden(FALSE);
		EffectComp.ActivateSystem();
	}

	if(VSizeSq(Velocity) > 0.5f)
	{
		if(!bPlayingRunEffect)
		{
			EffectComp.SetTemplate(RunEffect);
			EffectComp.ActivateSystem();
			bPlayingRunEffect = TRUE;
		}
	}
	else if(bPlayingRunEffect)
	{
		EffectComp.SetTemplate(IdleEffect);
		EffectComp.ActivateSystem();
		bPlayingRunEffect = FALSE;
	}
}

/** Handle repnotify vars */
simulated function ReplicatedEvent(name VarName)
{
	if(VarName == 'bLostLeftArm' || VarName == 'bLostRightArm' || VarName =='LastArmHitBone')
	{
		ArmStateChanged();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function PlayChargeSound()
{
	if(TimeSince(LastChargeSoundPlayTime)>5.0f)
	{
		LastChargeSoundPlayTime=WorldInfo.TimeSeconds;
		PlaySound(ChargeSound);
	}
}

function PlayAttackSound()
{
	PlaySound(AttackSound);
}

simulated event PlayFootStepSound( int FootDown )
{
	// if we are dead or ragdolled then don't play footstep sounds
	if ( (Health <= 0) || (Physics == PHYS_RigidBody) )
	{
		return;
	}

	Super.PlayFootStepSound( FootDown );

	PlaySound( FootStepSound, TRUE );
}

function DoSwipeAttackDamage()
{
	local GearWeap_SireMelee	Wpn;
	local float Tolerance;
	local vector Start, End;
	local ImpactInfo FirstHitInfo;

	if( IsDoingSpecialMove(GSM_Sire_MeleeHeadGrab) )
	{
		Wpn = GearWeap_SireMelee(Weapon);
		Tolerance = Controller.Enemy.GetCollisionRadius() + GetAIMeleeAttackRange();

		if( Wpn != None )
		{
			Start = Location;
			End = Start + vector(Rotation) * Tolerance;
			Start += Normal(End-Start) * GetCollisionRadius();

			FirstHitInfo = Wpn.CalcWeaponFire(Start, End,, vect(25.f,25.f,25.f));

			if(FirstHitInfo.HitActor == Controller.Enemy)
			{
				PlayAttackSound();
				Wpn.DoMeleeDamage( Controller.Enemy, Controller.Enemy.Location, 1.f );
			}
			//`log(GetFuncName()@HitActor);
		}
	}
}

function ANIM_NOTIFY_GoRagdoll()
{
	Knockdown(vect(0,0,0),vect(0,0,0));
	SetTimer( RandRange(SireKnockDownMinDuration,SireKnockDownMaxDuration), FALSE, nameof(SireGetBackUpFromKnockDown) );
}

simulated function ANIM_NOTIFY_EnableCollision()
{
	if(IsDoingSpecialMove(GSM_Sire_TankFall))
	{
		SpecialMoves[GSM_Sire_TankFall].TogglePawnCollision(Self, TRUE);
	}
}

simulated function ChainSawGore()
{
	BreakConstraint( vect(100,0,0), vect(0,10,0), 'Spine3' );
}

/** Util to see if a bone it relevant to a particular damage bone */
simulated function bool BoneForDamage(name Bone, name Parent)
{
	return (Bone != 'None') && (Bone == Parent || Mesh.BoneIsChildOf(Bone, Parent));
}

/** Used to track damage done to arms */
function TakeDamage(int Damage, Controller InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
{
	Super.TakeDamage(Damage, InstigatedBy, HitLocation, Momentum, DamageType, HitInfo, DamageCauser);

	// Only track arm health on server - will replicate state to client via bLostLeftArm/bLostRightArm
	if(Role == ROLE_Authority)
	{
		if(LeftArmHealth > 0 && BoneForDamage(HitInfo.BoneName, LeftArmBoneNames[0]))
		{
			LastArmHitBone=HitInfo.BoneName;
			LeftArmHealth -= Damage;
			if(LeftArmHealth <= 0)
			{
				bLostLeftArm = TRUE;
				ArmStateChanged();
			}
		}

		if(RightArmHealth > 0 && BoneForDamage(HitInfo.BoneName, RightArmBoneNames[0]))
		{
			LastArmHitBone=HitInfo.BoneName;
			RightArmHealth -= Damage;
			if(RightArmHealth <= 0)
			{
				bLostRightArm = TRUE;
				ArmStateChanged();
			}
		}
	}
}

/** Used to catch 'killing shots' and disallow them - causing a fall down instead */
function AdjustPawnDamage(out int Damage, Pawn InstigatedBy, Vector HitLocation, out Vector	Momentum, class<GearDamageType> GearDamageType, optional out TraceHitInfo HitInfo)
{
	// If this would be a killing blow, and not already on ground, and we still have at least one arm - just fall down instead of dying
	if(Damage < 10000.0f && Damage >= Health && !IsInState('KnockedDown') && (!bLostLeftArm || !bLostRightArm) && (NumTimesKnockedDown < SireMaxNumberOfKnockDowns) )
	{
		`AILog_EXT(GetFuncName()@"Setting bPendingKnockdown",,MyGearAI);

		Damage = 0;
		bPendingKnockdown = TRUE;
		NumTimesKnockedDown++;
	}
	else
	{
		Super.AdjustPawnDamage(Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo);
	}
}


/** Turn off all of the Sire Effects **/
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	`AILog_EXT(GetFuncName(),,MyGearAI);
	ClearTimer( nameof(UpdateParticleEffects) );
	ClearTimer( 'SireGetBackUpFromKnockDown' );
	bPendingKnockdown = FALSE;
	BreathingAudioComp.FadeOut( 0.0, 0.5f );
	EffectComp.DeactivateSystem();

	Super.PlayDying( DamageType, HitLoc );
}


/** blow up guy when he is dead **/
simulated function PlayDeath(class<GearDamageType> GearDamageType, vector HitLoc)
{
	Super.PlayDeath( GearDamageType, HitLoc );
	`AILog_EXT(GetFuncName(),,MyGearAI);

	// jag: Don't gib on death any more (as ordained by DaveN etc)
/*
	if(!GearDamageType.static.ShouldGib(self,none))
	{
		if( !bIsGore )
		{
			// Abort current special move
			if( IsDoingASpecialMove() )
			{
				EndSpecialMove();
			}

			CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);
		}

		if( bIsGore )
		{
			GoreExplosion( vect(1,1,2), Location, class'GDT_Explosive', TRUE );
		}
	}
*/
	HideBrokenBones();
}


simulated function Tick(float DeltaTime)
{
	local float DownDuration;
	local vector KnockAngVel;

	if((Role == ROLE_Authority) && bPendingKnockdown)
	{
		// fall down
		KnockAngVel = VRand() * SireKnockDownSpinVel;
		Knockdown(vect(0,0,0), KnockAngVel);
		// give back some health
		Health = KnockdownRestoreHealth;
		// set timer for getting back up
		DownDuration = RandRange(SireKnockDownMinDuration,SireKnockDownMaxDuration);
		SetTimer( DownDuration, FALSE, nameof(SireGetBackUpFromKnockDown) );
		// clear flag
		bPendingKnockdown = FALSE;
		PlaySound(KnockedDownSound);
	}

	super.Tick(DeltaTime);
}
simulated function float GetKnockDownFailSafeTimeout()
{
	return SireKnockDownMaxDuration*1.2f;
}
/** Get Back up from a knock down special move */
function SireGetBackUpFromKnockDown()
{
	`AILog_EXT(GetFuncName(),,MyGearAI);

	// Clear Timer to be safe
	ClearTimer('SireGetBackUpFromKnockDown');
	EndKnockedDownState();

	ServerDoSpecialMove(SM_RecoverFromRagdoll, TRUE);
	GotoState('');
}

simulated function name GetBreakBonename(name HitBone, bool bLeft)
{
	local int i;
	if(!bLeft)
	{
		for(i=RightArmBoneNames.length-1;i>=0;--i)
		{
			if(BoneForDamage(HitBone,RightArmBoneNames[i]))
			{
				return RightArmBoneNames[i];
			}
		}
	}
	else
	{
		for(i=LeftArmBoneNames.length-1;i>=0;--i)
		{
			if(BoneForDamage(HitBone,LeftArmBoneNames[i]))
			{
				return LeftArmBoneNames[i];
			}
		}
	}

	return '';
}

/**
* This will create the gore skeleton which will be constructed to break apart.
*/
simulated final function CreateCosmeticGoreSkeleton(SkeletalMesh TheSkeletalMesh, PhysicsAsset ThePhysicsAsset)
{
	local Array<Attachment> PreviousAttachments;
	local int				i, Idx;
	local array<texture>	Textures;

	// Need proper physics asset and mesh
	if( TheSkeletalMesh == None || ThePhysicsAsset == None )
	{
		return;
	}

	PreviousAttachments = Mesh.Attachments;

	// so only if the phys asset is different shall we change it out
	if( ThePhysicsAsset != Mesh.PhysicsAsset )
	{
		Mesh.SetPhysicsAsset(None);
	}

	Mesh.bDisableWarningWhenAnimNotFound = TRUE;
	Mesh.SetSkeletalMesh( TheSkeletalMesh, TRUE, TRUE );
	Mesh.bDisableWarningWhenAnimNotFound = FALSE;

	// so only if the phys asset is different shall we change it out
	if( ThePhysicsAsset != Mesh.PhysicsAsset )
	{
		Mesh.SetPhysicsAsset(ThePhysicsAsset);
	}

	SetMainBodyMaterialToNoneToClearItForGoreMaterial();

	// so if we have not had strechies fixed then hard weight everything
	if( bUsingNewSoftWeightedGoreWhichHasStretchiesFixed == FALSE )
	{
		for( i = 0; i < GoreBreakableJoints.Length; ++i )
		{
			//`log( "setting InstanceVertexWeight on: " $ GoreBreakableJoints[i] );
			Mesh.AddInstanceVertexWeightBoneParented( GoreBreakableJoints[i] );
		}
	}

	for( Idx = 0; Idx < PreviousAttachments.length; ++Idx )
	{
		//`log( "reattaching: " $ PreviousAttachments[Idx].Component );
		Mesh.AttachComponent( PreviousAttachments[Idx].Component
			, PreviousAttachments[Idx].BoneName
			, PreviousAttachments[Idx].RelativeLocation
			, PreviousAttachments[Idx].RelativeRotation
			, PreviousAttachments[Idx].RelativeScale
			);
	}

	// now the mesh has been changed to the GoreSkeleton so we want to tell them to be resident for 15 seconds
	for( Idx = 0; Idx < Mesh.SkeletalMesh.Materials.Length; ++Idx )
	{
		Textures = Mesh.SkeletalMesh.Materials[Idx].GetMaterial().GetTextures();

		for( i = 0; i < Textures.Length; ++i )
		{
			//`log( "Texture setting SetForceMipLevelsToBeResident( 15.0f ): " $ Textures[i] );
			Texture2D(Textures[i]).SetForceMipLevelsToBeResident( 15.0f );
		}
	}
}

simulated function ReadyPawnForRagdoll()
{
	Super.ReadyPawnForRagdoll();
	HideBrokenBones();
}


simulated function HideBrokenBones()
{
	local int i;
	for(i=0;i<BrokenBoneNames.length;i++)
	{
		//`log("Hiding bone:"@BrokenBoneNames[i]);
		mesh.HideBone(Mesh.MatchRefBone(BrokenBoneNames[i]),TRUE);
	}
}

/** Called on client + server when bLostLeftArm or bLostRightArm changes, so update mesh */
simulated function ArmStateChanged()
{
	local BodyStance BS_HitReact;
	local Name AnimName;
	local name BreakBonename;

	if(bLostRightArm || bLostLeftArm && LastArmHitBone != '')
	{
		BreakBonename = GetBreakBonename(LastArmHitBone,bLostLeftArm&&!bLostRightArm);
		if(bLostLeftArm)
		{
			if(!bLostRightArm)
			{
				CreateCosmeticGoreSkeleton(GoreSkeletalMesh,GorePhysicsAsset);
			}
			AnimName = LeftArmShotOffAnim;
		}

		if(bLostRightArm)
		{
			if(!bLostLeftArm)
			{
				CreateCosmeticGoreSkeleton(GoreSkeletalMesh,GorePhysicsAsset);
			}
			AnimName = RightArmShotOffAnim;
		}

		Mesh.PhysicsWeight = 1.0;
		Mesh.BreakConstraint(vect(0,0,10),mesh.GetBoneLocation(BreakBonename),BreakBonename);
		BrokenBoneNames.AddItem(BreakBonename);


		//`log("Armstate changed: left:"@bLostLeftArm@"right:"@bLostRightArm@BreakBonename@AnimName);

		`AILog_Ext("Armstate changed: left:"@bLostLeftArm@"right:"@bLostRightArm@BreakBonename@AnimName,,MyGearAI);
		if(AnimName != '')
		{
			BS_HitReact.AnimName[BS_FullBody] = AnimName;
			BS_Play(BS_HitReact,1.f,0.2f,0.33f);

			if (MyGearAI != None)
			{
				// abort active melee commands so we don't get stuck in it
				ServerDoSpecialMove(SM_None,TRUE);
				MyGearAI.AbortCommand(none,class'AICmd_Attack_SireMelee');
				class'AICmd_Pause'.static.Pause(MyGearAI,BS_GetTimeLeft(BS_HitReact));
			}
		}
	}
}


/** Play Hit Reaction animation */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	local BodyStance	BS_HitReaction;
	local Name			AnimName;
	local vector		PelvisLoc;

	if(LastArmHitBone == '')
	{
		PelvisLoc = mesh.GetBoneLocation(PelvisBoneName);

		if(Impact.HitLocation.Z > PelvisLoc.Z)
		{
			AnimName = FaceHitReactionAnimnName;
		}
		else
		{
			AnimName = LegHitReactionAnimName;
		}

		if( AnimName != '' )
		{
			BS_HitReaction.AnimName[BS_FullBody] = AnimName;
			NextHitReactionAnimTime = WorldInfo.TimeSeconds + FRand();
			// abort active melee commands so we don't get stuck in it
			MyGearAI.AbortCommand(none,class'AICmd_Attack_SireMelee');
			ServerDoSpecialMove(SM_None,TRUE);
			return (BS_Play(BS_HitReaction, 1.f, 0.2f, 0.33f) > 0.f);
		}

	}

	return FALSE;
}

/** Override state Begin from GearPawn so it doesn't get straight back up again */
state KnockedDown
{
Begin:
	// apply the impulse
	ApplyKnockdownImpulse();
}


defaultproperties
{
	DefaultInventory.Empty
	DefaultInventory(0)=class'GearWeap_SireMelee'
	ControllerClass=class'GearAI_Sire'

	LeftArmBoneNames=("L_shoulder","L_elbow","L_wrist")
	RightArmBoneNames=("R_shoulder","R_elbow","R_wrist")

	FaceHitReactionAnimnName="ADD_hit_react_face"
	LegHitReactionAnimName="ADD_hit_react_low"
	RightArmShotOffAnim="right_arm_hit_react"
	LeftArmShotOffAnim="left_arm_hit_react"


	BS_KnockDownAnim=(AnimName[BS_FullBody]="Run_Fwd")
	bPlayMotorAnimOnKnockDown=TRUE
	KnockDownMotorStrength=100.0
	KnockDownMotorDamping=10.0
	KnockDownMotorScale=(Points=((InVal=0.0,OutVal=1.0),(InVal=0.5,OutVal=0.1),(InVal=2.0,OutVal=0.0)))

	SightBoneName=none

	AimAttractors(0)=(OuterRadius=64.f,InnerRadius=4.f,BoneName="Spine3")
	AimAttractors(1)=(OuterRadius=32.f,InnerRadius=10.f,BoneName="head")
	AimAttractors(2)=(OuterRadius=32.f,InnerRadius=10.f,BoneName="L_Knee")
	AimAttractors(3)=(OuterRadius=32.f,InnerRadius=10.f,BoneName="R_Knee")

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Sire.Locust_Sire_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Sire.Locust_Sire_Physics'

	GoreBreakableJointsTest=("R_knee","R_rev_knee","L_knee","L_ankle","Spine3","Head","Jaw","L_shoulder","L_elbow","L_wrist","R_shoulder","R_elbow","R_wrist")
	GoreBreakableJoints=("R_knee","R_rev_knee","L_knee","L_ankle","Spine3","Head","Jaw","L_shoulder","L_elbow","L_wrist","R_shoulder","R_elbow","R_wrist")
	JointsWithDependantBreaks=((ParentBone="Spine3",DependantBones=("R_shoulder")),(ParentBone="Head",DependantBones=("Jaw"))))
	HostageHealthBuckets.Empty()


	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Sire.Locust_Sire'
		PhysicsAsset=PhysicsAsset'Locust_Sire.Locust_Sire_Physics'
		AnimTreeTemplate=AnimTree'Locust_Sire.Locust_Sire_Animtree'
		AnimSets.Empty()
		AnimSets(0)=AnimSet'Locust_Sire.Anims.Locust_Sire_Animset'
		Translation=(Z=-63)
		bEnableFullAnimWeightBodies=true
		MinDistFactorForKinematicUpdate=0.0f
	End Object

	MeatShieldUnfixedBoneList.Empty()
	SpecialMoveClasses(GSM_Sire_MeleeHeadGrab)	=class'GSM_Sire_MeleeHeadGrab'
	SpecialMoveClasses(GSM_Sire_TankFall)		=class'GSM_Sire_TankFall'
	SpecialMoveClasses(SM_RecoverFromRagdoll)	=class'GSM_RecoverFromRagdoll_Sire'
	SpecialMoveClasses(SM_ChainSawVictim)		=class'GSM_Sire_ChainsawVictim'
	SpecialMoveClasses(SM_DeathAnimFire)        =none // this is needed to make it so the pawn doesn't play the fire death anim and then be stuck in ragdoll forever
	SpecialMoveClasses(SM_StumbleFromMelee)		=none

	MeleeDamageBoneName="Spine3"
	HeadSocketName="Head_socket"
	RightHandSocketName="right_hand"
	LeftHandSocketName="left_hand"

	LeftFootBoneName="L_Ankle"
	RightFootBoneName="R_Ankle"
	LeftKneeBoneName="L_Knee"
	RightKneeBoneName="R_Knee"
	LeftHandBoneName="L_Wrist"
	RightHandBoneName="R_Wrist"

	PelvisBoneName="pelvis"
	NeckBoneName="neck"

	ChargeSound=SoundCue'Locust_Sire_Efforts.Sire.SiresChatter_ScreamsCue'
	AttackSound=SoundCue'Locust_Bloodmount_Efforts.BloodMount.BloodMount_BodyArmImpactCue'
	KnockedDownSound=SoundCue'Locust_Sire_Efforts.Sire.SiresChatter_ScreamTransitionCue'

	Begin Object Class=AudioComponent Name=BreathingAudioComp0
		SoundCue=SoundCue'Locust_Sire_Efforts.Sire.SiresChatter_BreathingRoadieRunCue'
		bAutoPlay=true
		bStopWhenOwnerDestroyed=true
	End object
	Components.Add(BreathingAudioComp0)
	BreathingAudioComp=BreathingAudioComp0

	IdleEffect=ParticleSystem'Locust_Sire.Effects.P_Sire_Breath_Idle'
	RunEffect=ParticleSystem'Locust_Sire.Effects.P_Sire_Breath_Run'
	EffectSocket=face_socket

	DrawScale=1.15
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_Sire'

	bReplicateRigidBodyLocation=false

	FootStepSound=SoundCue'Foley_Footsteps.FootSteps.BoomerFootstepsCue'

}
