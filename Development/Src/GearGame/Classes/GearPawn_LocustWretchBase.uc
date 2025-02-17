
/**
 * Locust Wretch
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustWretchBase extends GearPawn
	abstract
	native(Pawn)
	config(Pawn);


cpptext
{
	virtual void MarkEndPoints(ANavigationPoint* EndAnchor, AActor* Goal, const FVector& GoalLocation);
}


/** Forearm Bone names **/
var name LeftForeArmBoneName;
var name RightForeArmBoneName;

var AudioComponent AC_HandStepLeft;
var AudioComponent AC_HandStepRight;


var bool bWaitForKnockOff;
var bool bSuicidal;
var bool bFleshTimer;
var GearPawn Victim;

var float LeapSpeed;

/** replicated Floor property, used to set Floor on the client to play the proper anims */
var repnotify vector ReplicatedFloor;

replication
{
	if (Physics == PHYS_Spider)
		ReplicatedFloor;
}

event PostAIFactorySpawned(SeqAct_AIFactory SpawningFactory, INT SpawnSetIdx)
{
	Super.PostAIFactorySpawned(SpawningFactory,SpawnSetIdx);
	// make sure we have a wretch melee weapon
	AddDefaultInventory();
}

simulated function Destroyed()
{
	super.Destroyed();

	// @fixme laurent, this is very dangerous
	// all custom animations should not be forced to be stopped on the victim pawn
	// code could be waiting for some notify and get stuck
	// we should use a special move on the victim here and stop only specific animations, not ALL!
	if( Victim != None )
	{
		Victim.BS_StopAll(0.25f);
		Victim = None;
	}
}

function bool Died(Controller Killer, class<DamageType> GearDamageType, vector HitLocation)
{
	DeathCry();
	//PlaySound(SoundCue'Locust_Wretch_Efforts.Wretch.WretchADeathsCue');
	//PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyChunkMediumCue');

	return Super.Died( Killer, GearDamageType, HitLocation );
}

simulated function DeathCry();


simulated function UpdateMeshBoneControllers(float DeltaTime);


function bool DoWallLeap( Actor Target )
{
	local Vector JumpVel;

	if( !bIsCrouched	&&
		!bWantsToCrouch &&
		(Physics == PHYS_Walking || Physics == PHYS_Spider) )
	{
		if( Role == ROLE_Authority )
		{
			if( WorldInfo.Game != None && WorldInfo.Game.GameDifficulty > 2 )
			{
				MakeNoise(0.1 * WorldInfo.Game.GameDifficulty);
			}
		}

		SuggestJumpVelocity( JumpVel, Target.Location, Location );
		Velocity = JumpVel;
		SetPhysics( PHYS_Falling );

	return true;
	}
    return false;
}

function bool SpecialMoveTo( NavigationPoint Start, NavigationPoint End, Actor Next )
{
	local ReachSpec		CurrentPath;
`if(`notdefined(FINAL_RELEASE))
	local GearAI	AI;

	AI = GearAI(Controller);
`endif
	if( Start == End )
	{
		//debug
		`AILog_Ext( self@GetFuncName()@Start@End@"FAIL -- SAME DEST", 'Move', AI );

		return FALSE;
	}

	//debug
	`AILog_Ext( self@"Wretch"@GetFuncName()@Start@End@Start.GetReachSpecTo( End ), 'Move', AI );

	CurrentPath = Start.GetReachSpecTo( End );
	if( CurrentPath != None )
	{
		if( CurrentPath.IsA( 'WallTransReachSpec' ) )
		{
			return SpecialMoveTo_WallTrans( Start, End, TRUE );
		}
		else if( CurrentPath.IsA( 'FloorToCeilingReachSpec' ) )
		{
			return SpecialMoveTo_CeilingTrans( Start, End, TRUE );
		}
	}

	return super.SpecialMoveTo( Start, End, Next );
}

function bool SpecialMoveTo_WallTrans( NavigationPoint Start, NavigationPoint End, bool bDirect )
{
	DoWallLeap( End );
	Controller.bPreparingMove = TRUE;
	return TRUE;
}

function bool SpecialMoveTo_CeilingTrans( NavigationPoint Start, NavigationPoint End, bool bDirect )
{
	local GearAI_Wretch AI;

	AI = GearAI_Wretch(Controller);
	if( AI != None )
	{
		//debug
		`AILog_Ext( GetFuncName()@Start@End@bDirect@Physics, 'Wretch', AI );

		if( Physics != PHYS_Spider )
		{
			class'AICmd_Move_JumpToCeiling'.static.Jump( AI, Start, End );
		}
		else
		{
			class'AICmd_Move_DropFromCeiling'.static.Jump( AI, Start, End );
		}
	}
	return TRUE;
}

function HitWall( Vector HitNormal, Actor Wall, PrimitiveComponent WallComp )
{
`if(`notdefined(FINAL_RELEASE))
	local GearAI AI;

	//debug
	AI = GearAI(Controller);
	if( AI != None )
	{
		`AILog_Ext( GetFuncName()@HitNormal@WalkableFloorZ@Wall@IsDoingSpecialMove( GSM_LeapToCeiling ), 'Wretch', AI );
	}
`endif

	super.HitWall( HitNormal, Wall, WallComp );

	// If leaping to ceiling AND
	// hit a ceiling surface (defined by inverted floor normal)
	if( IsDoingSpecialMove( GSM_LeapToCeiling ) &&
		HitNormal.Z <= -WalkableFloorZ )
	{
`if(`notdefined(FINAL_RELEASE))
		if( AI != None )
		{
			`AILog_Ext( "Goto phys spider", 'Wretch', AI );
		}
`endif

		// Change to phys spidering
		SetPhysics( PHYS_Spider );
		SetBase( Wall, HitNormal );

		PlayLandedOnWallSound();
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'ReplicatedFloor')
	{
		Floor = ReplicatedFloor;
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

function bool ShouldDealDamageToEnemy()
{
	local Vector VectToEnemy;

	VectToEnemy = Controller.Enemy.Location - Location;

	// Enemy must not be too far
	if( VSize(VectToEnemy) > (CylinderComponent.CollisionRadius + Controller.Enemy.CylinderComponent.CollisionRadius) * 2.5f )
	{
		return FALSE;
	}

	// Wretch must be somewhat facing enemy
	if( Normal(VectToEnemy) dot Vector(Rotation) < 0.0f )
	{
		return FALSE;
	}

	return TRUE;
}


function DoPounceAttackDamage()
{
	local GearWeap_WretchMelee	Wpn;

	if( IsDoingSpecialMove(GSM_PounceAttack) && ShouldDealDamageToEnemy() )
	{
		Wpn = GearWeap_WretchMelee(Weapon);
		if( Wpn != None )
		{
			// Scale melee damage by 50% here, because this function is called twice during the animation
			// dual hand attack FTW!
			Wpn.DoMeleeDamage( Controller.Enemy, Controller.Enemy.Location, 0.5f );
			//PlaySound(SoundCue'Locust_Wretch_Efforts.Wretch.WretchAAttackLargesCue');
		}
	}
}


/** Do melee damage from swipe attack to enemy */
function DoSwipeAttackDamage()
{
	local GearWeap_WretchMelee	Wpn;

	if( IsDoingSpecialMove(GSM_SwipeAttack) && ShouldDealDamageToEnemy() )
	{
		Wpn = GearWeap_WretchMelee(Weapon);
		if( Wpn != None )
		{
			Wpn.DoMeleeDamage( Controller.Enemy, Controller.Enemy.Location, 1.f );
			//PlaySound(SoundCue'Locust_Wretch_Efforts.Wretch.WretchAAttackLargesCue');
		}
	}
}


function PlayFleshSound()
{
	//PlaySound(SoundCue'Foley_Flesh.Flesh.PhysicsFleshSlideCue');
	bFleshTimer = false;
}


simulated function bool CanEngageMelee()
{
	return TRUE;
}

simulated function bool ShouldTorqueBowArrowGoThroughMe(GearProjectile Proj, TraceHitInfo HitInfo, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	// gib!
	TakeDamage(9999.f,Proj.InstigatorController,HitLocation,Momentum,class'GDT_TorqueBow_Impact',HitInfo);
	// torque bow arrows always go through wretches
	return true;
}


function PlayLandedOnWallSound();
function PlayLeapSound();
function PlaySwipeAttackSound();
function PlaySwipeAttackHitSound();
function PlaySwipeAttackMissSound();
function PlayLandedOnEnemySound();


/**********************************
******* MINE CART FUNCTIONS *******
**********************************/
singular simulated function BaseChange()
{
	local GearInterActorAttachableBase Cart;

	if (Role == ROLE_Authority)
	{
		if( Base == None && Physics == PHYS_None )
		{
			if( !IsDoingSpecialMove(GSM_LeapToCeiling) &&
				!IsDoingSpecialMove(GSM_DropFromCeiling) )
			{
				SetPhysics(PHYS_Falling);
			}
		}
		else if( Pawn(Base) != None &&
			    (DrivenVehicle == None || !DrivenVehicle.IsBasedOn(Base)) )
		{
			// Jump off Pawn if falling on it
			if( !Pawn(Base).CanBeBaseForPawn(Self) )
			{
				//`log(WorldInfo.TimeSeconds @ GetFuncName() @ "JumpOffPawn!");
				Base.TakeDamage( (1-Velocity.Z/400)* Mass/Pawn(Base).Mass, controller,Location,0.5 * Velocity , class'GDT_Melee');
				JumpOffPawn();
			}
		}

		// If we are based on a minecart
		Cart = GearInterActorAttachableBase(Base);
		if (Cart != None)
		{
			Controller.PushState('SubAction_ClimbCart');
		}
	}

	if (Base != None)
	{
		if (Role == ROLE_Authority)
		{
			ReplicatedFloor = Floor;
		}
		else
		{
			Floor = ReplicatedFloor;
		}
	}
}

// Never go to falling if taking damage while on a mine cart
function SetMovementPhysics()
{
	if( GearInterActorAttachableBase(Base) != None )
	{
		return;
	}

	super.SetMovementPhysics();
}


/**
 * wretches don't get weapon attachments at this time
 * What happens is the various actor factories get re-used and they still have the old
 * inventory settings.  So IF the wretches actually had the same socket names as other pawns
 * we would have wretchy wretches with weapons!
 *
 **/
simulated function AttachWeaponToHand(GearWeapon Weap);
simulated function AttachWeaponToSlot(GearWeapon W);
simulated function AdjustWeaponDueToMirror();

/** Wretches don't play hit wall effects as they are sneaky! **/
simulated function DoRun2CoverWallHitEffects();


/** SCREEEAMMMM!!! **/
simulated function PlayScreamSound();


defaultproperties
{
	DefaultInventory(0)=class'GearGame.GearWeap_WretchMelee'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=441,V=0,UL=48,VL=63)

	ControllerClass=class'GearAI_Wretch'

	Begin Object Name=CollisionCylinder
		CollisionRadius=+34.0
		CollisionHeight=+52.0
	End Object

	bTranslateMeshByCollisionHeight=FALSE // wretch root is not between his feet.
	Begin Object Name=GearPawnMesh
		// needs to be true for flamethrower to detect hits, also for PhysicsBodyImpact
		bHasPhysicsAssetInstance=TRUE
	End Object

	PelvisBoneName="b_W_Pelvis"
	NeckBoneName="b_W_Neck"
	MeleeDamageBoneName="b_W_Spine_02"

	LeftFootBoneName="b_W_Foot_L"
	RightFootBoneName="b_W_Foot_R"

	LeftKneeBoneName="b_W_Calf_L"
	RightKneeBoneName="b_W_Calf_R"

	LeftHandBoneName="b_W_Hand_L"
	RightHandBoneName="b_W_Hand_R"

	LeftForeArmBoneName="b_W_Forearm_L"
	RightForeArmBoneName="b_W_Forearm_R"

	JumpZ=2100.f
	LeapSpeed=600.f
	MaxFallSpeed=+20000.0
	bDirectHitWall=TRUE
	bCanCrouch=FALSE

	SpecialMoveClasses(SM_DeathAnim)		=class'GSM_DeathAnim'
	SpecialMoveClasses(SM_Run2MidCov)		=class'GSM_Wretch_Run2Cover'
	SpecialMoveClasses(SM_Run2StdCov)		=class'GSM_Wretch_Run2Cover'
	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_Wretch_MidLvlJumpOver'
	SpecialMoveClasses(SM_MantleUpLowCover) = class'GSM_Wretch_MantleUpLowCover'

	SpecialMoveClasses(SM_EvadeFwd)			=class'GSM_Wretch_EvadeFwd'
	SpecialMoveClasses(SM_EvadeBwd)			=class'GSM_Wretch_EvadeBwd'
	SpecialMoveClasses(SM_EvadeLt)			=class'GSM_Wretch_EvadeLt'
	SpecialMoveClasses(SM_EvadeRt)			=class'GSM_Wretch_EvadeRt'

	SpecialMoveClasses(SM_Emerge_Type1)		=class'GSM_Emerge'
	SpecialMoveClasses(SM_Emerge_Type2)		=class'GSM_Emerge'

	SpecialMoveClasses(GSM_PounceAttack)	=class'GSM_Wretch_PounceAttack'
	SpecialMoveClasses(GSM_SwipeAttack)		=class'GSM_Wretch_SwipeAttack'
	SpecialMoveClasses(GSM_Scream)			=None
	SpecialMoveClasses(GSM_LeapToCeiling)	=class'GSM_Wretch_LeapToCeiling'
	SpecialMoveClasses(GSM_DropFromCeiling)	=class'GSM_Wretch_DropFromCeiling'
	SpecialMoveClasses(GSM_Cart_ClimbUp)	=class'GSM_Wretch_CartClimb'
	SpecialMoveClasses(GSM_Cart_SwipeAttack)=class'GSM_Wretch_CartAttack'
	SpecialMoveClasses(GSM_Elevator_Climb)  =class'GSM_Wretch_ElevatorClimb'

	SpecialMoveClasses(SM_ChainSawVictim)		=class'GSM_Wretch_ChainsawVictim'
	SpecialMoveClasses(SM_RecoverFromRagdoll)	=class'GSM_Wretch_RecoverFromRagdoll'
	SpecialMoveClasses(SM_Wretch_GrabAttack)	=class'GSM_Wretch_GrabAttack'

	// Fast rotation rate
	RotationRate=(Pitch=100000,Yaw=100000,Roll=100000)

	AimAttractors(0)=(OuterRadius=128.f,InnerRadius=32.f,BoneName="b_W_Clavicle_L_")
	AimAttractors(1)=(OuterRadius=96.f,InnerRadius=32.f,BoneName="b_W_Head")

	// Animation
	bPlayDeathAnimations=TRUE
	bCanPlayHeadShotDeath=FALSE
	bCanClimbCeilings=TRUE
	DeathAnimStdBwd=("death_back_a","death_back_b")
	DeathAnimStdLt=("death_left")
	DeathAnimStdRt=("death_right")

	bBlockCamera=FALSE

	CameraNoRenderCylinder_High=(Radius=50,Height=60)
	CameraNoRenderCylinder_Low=(Radius=50,Height=60)

	NoticedGUDSPriority=30
	NoticedGUDSEvent=GUDEvent_NoticedWretch

	bCanDBNO=false

	TimeToScaleLimits=1.f
	PhysicsImpactBlendOutTime=1.f
}
