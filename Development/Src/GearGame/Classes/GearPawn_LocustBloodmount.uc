
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBloodmount extends GearPawn
	config(Pawn)
	abstract;

/** GearPawn that is the driver */
var repnotify GearPawn	Driver;
/** GearPawn class to spawn for Driver */
var class<GearPawn>		DriverClass;
/** AnimTrees to use for Driver */
var AnimTree			DriverAnimTree;
/** Name of the socket driver should be attached to */
var Name				DriverSocket;

/** Scalar to apply to groundspeed when the driver is knocked off **/
var config float		NoDriverSpeed;

/** chance that the driver will stay in the saddle when it dies (and flop around OH BOY!)*/
var() config float		DriverStayInSaddleChance;

/** speed to use when charging (with a driver)*/
var() config float ChargeSpeed;

/** sound that plays when the bloodmount does his melee attack */
var SoundCue AttackSound;

/** sound that plays when his helmet is blown off and he freaks out */
var SoundCue HeadShotCue;

/** sound that plays when we first acquire an enemy */
var SoundCue EnemyAcquiredCue;

/** sound that plays when our driver gets blown off and we start moving faster */
var SoundCue DriverGoneCue;

/** sound that plays every so often when we're not in combat */
var SoundCue AmbientCue;
/** Sound that plays every so often when we are in combat */
var SoundCue CombatAmbientCue;
/** sound to play when the bloodmount is charging his enemy */
var SoundCue ChargeCue;
/** sound that plays when the bloodmount's body is falling to the ground */
var SoundCue BodyFallSound;

var Vector2d AmbientTimeRange;

var StaticMeshComponent HeadSlotStaticMesh;

var class<Item_HelmetBase> HelmetItem;

var float lastChargeSoundTime;

var BodyStance	LastPlayedHitReactionStance;

/** Node for playing custom anims on the head */
var GearAnim_Slot	HeadSlot;
/** How often roar anim plays */
var vector2D	RoarInterval;


var AIAvoidanceCylinder AvoidanceCylinder;


replication
{
	if( bNetInitial )
		Driver;
}


simulated function PostBeginPlay()
{
	local GearAI DriverController;

	Super.PostBeginPlay();

	if( Role == ROLE_Authority )
	{
		// Spawn Driver
		if(DriverClass != none)
		{
			Driver = Spawn(DriverClass,,,,,, TRUE); // ignore encroach on spawn
			if( Driver != None )
			{
				DriverController = GearAI(Spawn(Driver.ControllerClass));
				DriverController.SetTeam(1);
				DriverController.Possess(Driver, FALSE);
				DriverController.SetSquadName('Alpha');
				Driver.SetOwner(Self);

				Driver.SetBase(NONE);
				Driver.SetPhysics(PHYS_None);
				Driver.SetCollision(TRUE, FALSE);
				Driver.SetHardAttach(TRUE);
				Driver.SetBase(Self,, Mesh, 'Driver');
				Driver.bCollideWorld = FALSE;

				DoDriverAttachment(Driver);

				Driver.AddDefaultInventory();
				Driver.PeripheralVision = -0.4f;
				DriverController = GearAI(Driver.Controller);
				if( DriverController != None )
				{
					SetTimer( 0.1,,nameof(PushDriverCommand) );
				}
			}

		}
	}

	SetTimer(RandRange(RoarInterval.X,RoarInterval.Y),FALSE,nameof(PlayRoarAnim));

	SetTimer(RandRange(AmbientTimeRange.X,AmbientTimeRange.Y),FALSE,nameof(DoAmbientSound));

	SetupHelmet();

	SetupAvoidanceCylinder();

}

simulated function bool ShouldTorqueBowArrowGoThroughMe(GearProjectile Proj, TraceHitInfo HitInfo, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	return FALSE;
}

function SetupAvoidanceCylinder()
{
	local float AvoidRadius;

	if( !bDeleteMe && AvoidanceCylinder == None )
	{
		AvoidRadius = GetCollisionRadius() * 2f;
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);

		if(AvoidanceCylinder!=none)
		{

			AvoidanceCylinder.SetBase(self);
			AvoidanceCylinder.SetRelativeLocation(vect(75.f,0.f,0.f));
			AvoidanceCylinder.SetCylinderSize(AvoidRadius,AvoidRadius*2.0f);
			AvoidanceCylinder.SetAvoidanceTeam(TEAM_COG);
			AvoidanceCylinder.SetEnabled(true);
		}
		//DrawDebugSphere(Location,AvoidRadius,16,255,0,0,TRUE);
	}

}

/** Cache pointer to head slot */
simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	HeadSlot = GearAnim_Slot(Mesh.FindAnimNode('Custom_Head'));
}

function NotifyCharging(Actor ChargeTarget)
{
	if(ChargeTarget != none)
	{
		if(TimeSince(lastChargeSoundTime)>3.0)
		{
			PlaySound(ChargeCue);
			lastChargeSoundTime=worldinfo.timeseconds;
		}

		// if we have no driver, dont' mess with speed
		if(Driver != none && Driver.IsAliveAndWell())
		{
			GroundSpeed = ChargeSpeed;
		}

	}
	else
	{
		if(Driver != none && Driver.IsAliveAndWell())
		{
			GroundSpeed = DefaultGroundSpeed;
		}
		else
		{
			GroundSpeed = NoDriverSpeed;
		}
	}
}
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	return FALSE;
}

function PlayAttackSound()
{
	PlaySound(AttackSound);
}

simulated function RemoveHelmet()
{
	PlaySound(HeadShotCue);
	ShootOffHelmet();
}

/**
 * To remove blood mount helmets we do it differently than the normal GearPawn_Infantry.
 *
 * This is triggered from an anim notify
 * which is triggered by sufficient damage to the head
 *
 * Check out: AIReactCond_BloodMountHeadDamage for the internals of this behavior
 *
 * NOTE: you can not test this with ce dumbai.  The bloodmount needs to be alive and trying to kill you for it to work.
 **/
simulated function ShootOffHelmet()
{
	local SpawnableItemDatum ItemDatum;
	local Vector SpawnLoc;
	local Rotator SpawnRot;
	local Item_HelmetBase AHelmet;

	HeadSlotStaticMesh.SetHidden( TRUE );
	Mesh.DetachComponent(HeadSlotStaticMesh);

	ItemDatum = HelmetItem.static.GetSpawnableItemDatum();

	Mesh.GetSocketWorldLocationAndRotation( ItemDatum.AttachSocketName, SpawnLoc, SpawnRot );

	AHelmet = Spawn( HelmetItem,,, SpawnLoc,,, TRUE );
	AHelmet.StaticMeshComponent.SetStaticMesh( ItemDatum.TheMesh );

	AHelmet.CollisionComponent.AddImpulse( Vector(Rotation) * RandRange(25,45), vect(0,0,100) );
	AHelmet.CollisionComponent.SetRBAngularVelocity( vect(0,100,300), TRUE );

	AHelmet.PlayShotOffSound();

	// >>> play particle effect for when we're shot off

	if( AHelmet != None )
	{
		AHelmet.TurnCollisionOff();

		AHelmet.SetTimer( 0.100f, FALSE, nameof(AHelmet.TurnCollisionOn) ); // to stop inter penetrating and then OOE as physics corrects and shoots it off
	}
}

/** This will set up all of the MICs on the helmets if this pawn has one **/
simulated function SetupHelmetMaterialInstanceConstant()
{
	if( ( HeadSlotStaticMesh != None )  )
	{
		MIC_PawnMatHelmet = HeadSlotStaticMesh.CreateAndSetMaterialInstanceConstant(0);
	}
}


/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin(float HeatIncrement)
{
	Super.HeatSkin( HeatIncrement );

	if( ( HeadSlotStaticMesh != None ) && ( MIC_PawnMatHelmet != None ) )
	{
		MIC_PawnMatHelmet.SetScalarParameterValue('Heat', CurrentSkinHeat);
	}
}

/** Char skin to the given absolute char amount [0..1].  Charring does not diminish over time. */
simulated function CharSkin(float CharIncrement)
{
	Super.CharSkin( CharIncrement );

	if( ( HeadSlotStaticMesh != None )  && ( MIC_PawnMatHelmet != None ) )
	{
		MIC_PawnMatHelmet.SetScalarParameterValue('Burn', CurrentSkinChar);
	}
}


function PlayEnemyAcquiredSound()
{
	PlaySound(EnemyAcquiredCue);
}

function PlayDriverGoneSound()
{
	PlaySound(DriverGoneCue);
}


simulated function SetupHelmet()
{
	local SpawnableItemDatum ItemDatum;

	ItemDatum = HelmetItem.static.GetSpawnableItemDatum();

	HeadSlotStaticMesh.SetStaticMesh(ItemDatum.TheMesh);
	HeadSlotStaticMesh.SetShadowParent(Mesh);
	HeadSlotStaticMesh.SetLightEnvironment(LightEnvironment);
	HeadSlotStaticMesh.SetHidden(FALSE);
	Mesh.AttachComponentToSocket(HeadSlotStaticMesh, ItemDatum.AttachSocketName);

	SetupHelmetMaterialInstanceConstant();

}


event PostAIFactorySpawned(SeqAct_AIFactory SpawningFactory, INT SpawnSetIdx)
{
	Super.PostAIFactorySpawned(SpawningFactory,SpawnSetIdx);
	// add the driver (if any) to the factory's watch list
	if(Driver != none)
	{
		SpawningFactory.SpawnSets[SpawnSetIdx].WatchList.AddItem(Driver);
	}
}

/** Play a roar animation every so often */
simulated function PlayRoarAnim()
{
	if(IsAliveAndWell())
	{
		HeadSlot.PlayCustomAnim('ADD_Roar_Overlay', 1.0, 0.1, 0.2, FALSE, TRUE);

		if(Health > 0)
		{
			SetTimer(RandRange(RoarInterval.X,RoarInterval.Y),FALSE,nameof(PlayRoarAnim));
		}
	}
}

simulated function DoAmbientSound()
{
	if(IsAliveAndWell())
	{
		if(MyGearAI != none && MyGearAI.IsInCombat())
		{
			PlaySound(CombatAmbientCue,TRUE);
		}
		else
		{
			PlaySound(AmbientCue,TRUE);
		}
		SetTimer(RandRange(AmbientTimeRange.X,AmbientTimeRange.Y),FALSE,nameof(DoAmbientSound));
	}
}

function PushDriverCommand()
{
	local GearAI GAI;

	GAI = GearAI(Driver.Controller);
	// abort current combat command if there is one
	GAI.AbortCommand(None, GAI.DefaultCommand);
	// switch to mounted gunner
	class'AICmd_Base_MountedGunner'.static.InitCommand(GearAI(Driver.Controller));
}

simulated event ReplicatedEvent(name VarName)
{
	if( VarName == 'Driver' )
	{
		if( Driver != None )
		{
			Driver.SetCollision(TRUE, FALSE);
			Driver.SetHardAttach(TRUE);
			Driver.SetBase(Self,, Mesh, 'Driver');

			DoDriverAttachment(Driver);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function DoDriverAttachment(GearPawn InDriver)
{
	// fix me
	InDriver.Mesh.SetTranslation( vect(0,0,0) );

	if( DriverAnimTree != None && DriverAnimTree != InDriver.Mesh.AnimTreeTemplate )
	{
		// Backup his original Animation Tree.
		InDriver.RecoverFromRagdollAnimTree = InDriver.Mesh.AnimTreeTemplate;
		// Set driver AnimTree
		InDriver.Mesh.SetAnimTreeTemplate(DriverAnimTree);
		// Update AnimNodes
		InDriver.ClearAnimNodes();
		InDriver.CacheAnimNodes();
	}

	InDriver.bCanDBNO = FALSE;
	InDriver.Mesh.SetShadowParent( Mesh );

	// Do this to avoid weird collision 
	InDriver.bDisableKicksCorpseAfterRecovery = TRUE;

	// Switch default death animation special move to falling from beast.
	InDriver.SpecialMoveClasses[SM_DeathAnim] = class'GSM_DeathAnimFallFromBeast';
	InDriver.SpecialMoveClasses[SM_DeathAnimFire] = None;
	InDriver.SpecialMoveClasses[SM_StumbleFromMelee] = None;
	InDriver.SpecialMoveClasses[SM_StumbleFromMelee2] = None;

	InDriver.bDisableMeshTranslationChanges = TRUE;

	// Sync animations between mount and driver
	InDriver.AnimTreeRootNode.ForceGroupRelativePosition('IdleAnims', AnimTreeRootNode.GetGroupRelativePosition('IdleAnims'));
	InDriver.AnimTreeRootNode.ForceGroupRelativePosition('RunWalk', AnimTreeRootNode.GetGroupRelativePosition('RunWalk'));
}

// simulated function Tick(float DeltaTime)
// {
// 	Super.Tick(DeltaTime);
//
// 	if( Driver != None )
// 	{
// 		WorldInfo.AddOnScreenDebugMessage(0, 1.f, MakeColor(128,128,128,255), "IdleAnims, BloodMount:" @ AnimTreeRootNode.GetGroupRelativePosition('IdleAnims') @ "Rider:" @ Driver.AnimTreeRootNode.GetGroupRelativePosition('IdleAnims'));
// 		WorldInfo.AddOnScreenDebugMessage(1, 1.f, MakeColor(128,128,128,255), "RunWalk, BloodMount:" @ AnimTreeRootNode.GetGroupRelativePosition('RunWalk') @ "Rider:" @ Driver.AnimTreeRootNode.GetGroupRelativePosition('RunWalk'));
// 	}
// }

simulated function PlayMountedDeathFor(GearPawn Rider,class<GearDamageType> GearDamageType, vector HitLoc, bool bShowGore)
{
	local Vector		ApplyImpulse;
	local TraceHitInfo	HitInfo;

	Rider.SetBase(NONE);
	Rider.SetPhysics(PHYS_None);
	Rider.SetHardAttach(TRUE);
	Rider.SetBase(Self,, Mesh, 'Driver');
	Rider.SetPhysics(PHYS_None);

	if( Rider.Mesh.PhysicsAssetInstance != None )
	{
		// fix the proper bones
		Rider.Mesh.PhysicsAssetInstance.SetNamedBodiesFixed(TRUE,Rider.BloodMountFixedBoneList,Rider.Mesh,TRUE);
		Rider.Mesh.PhysicsAssetInstance.SetNamedMotorsAngularPositionDrive(TRUE, TRUE, Rider.BloodMountFixedBoneList, Rider.Mesh, TRUE);
	}

	Rider.StopAllAnimations();
	// Wake up rigid body
	Rider.Mesh.WakeRigidBody();

	// Enable full physics on body
	Rider.Mesh.PhysicsWeight = 1.f;

	// always reduce constraints here cuz it looks better to be less flippityfloppity on the back of the bloodmount
	ReduceConstraintLimits();

	// make sure we're going the slow speed since he's still strapped to my back!
	GroundSpeed = DefaultGroundSpeed;

	// disable timer that destroys dead guys so he doesn't get cleaned up when you look away
	Rider.ClearTimer();

	SetTimer(RandRange(2,6),FALSE,nameof(DetachDeadDriver));

	ApplyImpulse = GetImpactPhysicsImpulse(GearDamageType, HitLoc, Rider.TearOffMomentum, HitInfo);
	if( !IsZero(ApplyImpulse) )
	{
		Rider.Mesh.AddImpulse(ApplyImpulse*1.5f, HitLoc, HitInfo.BoneName);
	}
	Driver = Rider;
}

simulated function DetachDeadDriver()
{
	if( Driver != none && Driver.Mesh.PhysicsAssetInstance != None )
	{
		// un-fix the proper bones
		Driver.Mesh.PhysicsAssetInstance.SetNamedBodiesFixed(FALSE,Driver.BloodMountFixedBoneList,Driver.Mesh,FALSE);
		Driver.Mesh.PhysicsAssetInstance.SetNamedMotorsAngularPositionDrive(FALSE, FALSE, Driver.BloodMountFixedBoneList, Driver.Mesh, FALSE);
		Driver.SetBase(None);
		Driver.SetHardAttach(FALSE);
		Driver.Mesh.SetShadowParent( None );
	}
}

function EjectDriver()
{
	local GearPawn	OldDriver;
	local vector	Impulse;

	if( Driver != None && Driver.Base == Self )
	{
		OldDriver = Driver;
		OldDriver.SetBase(None);
		OldDriver.SetHardAttach(FALSE);
		OldDriver.SetPhysics(PHYS_Falling);
		OldDriver.SetCollision(TRUE, TRUE);
		OldDriver.bCollideWorld = TRUE;
		OldDriver.Mesh.SetShadowParent( None );
		OldDriver.bCanDBNO = OldDriver.default.bCanDBNO;
		// Reset mesh translation when driver ragdolls
		OldDriver.Mesh.SetTranslation( OldDriver.default.Mesh.Translation );

		Impulse = Velocity;
		if( VSizeSq(Velocity) < 100.f )
		{
			Impulse += vector(Rotation) * 100.0f;
		}

		OldDriver.bInvalidMeleeTarget = TRUE;
		if( OldDriver.Health <= 0 )
		{
			OldDriver.PlayRagDollDeath(None, OldDriver.Location, FALSE);
			// start the cleanup timer
			OldDriver.SetTimer(2.0f,true);
		}
		else
		{
			OldDriver.Knockdown(Impulse * 1.25f + vect(0,0,150), VRand() * 15.f);
		}
	}
}

simulated function Destroyed()
{
	Super.Destroyed();
	EjectDriver();

	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = none;
	}
}

/** Overridden to throw driver off. */
function bool Died(Controller Killer, class<DamageType> GearDamageType, vector HitLocation)
{
	ClearTimer(nameof(DoAmbientSound));
	EjectDriver();

	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = none;
	}
	return Super.Died( Killer, GearDamageType, HitLocation );
}

// if our driver gets off, speed up
event Detach(Actor Detachee)
{
	Super.Detach(Detachee);

	if( Detachee == Driver )
	{
		Driver = None;
		GroundSpeed = NoDriverSpeed;
		GearAI_Bloodmount(MyGearAI).NotifyDriverDetached();
		PlayDriverGoneSound();
	}
}

function UpdateDriver();
function UpdateGunner();

function DoSwipeAttackDamage()
{
	local GearWeap_BloodMountMelee	Wpn;
	local float Tolerance, DotP;
	local actor HitActor;
	local vector Start, End;
	local GearPawn EnemyGP;
	local ImpactInfo FirstHitInfo;

	if( IsDoingSpecialMove(GSM_BloodMount_Melee) )
	{
		Wpn = GearWeap_BloodMountMelee(Weapon);
		Tolerance = (Controller.Enemy.GetCollisionRadius() * 1.5) + GetAIMeleeAttackRange();

		if( Wpn != None )
		{
			PlayAttackSound();

			Start = Location;
			End = Start + vector(Rotation) * Tolerance;
			Start += Normal(End-Start) * GetCollisionRadius();

			FirstHitInfo = Wpn.CalcWeaponFire(Start, End,, vect(25.f,25.f,25.f));
			HitActor = FirstHitInfo.HitActor;

/*
			`log( "...."@HitActor@Controller.Enemy );
			FlushPersistentDebugLines();
			DrawDebugBox( Start, vect(5,5,5), 0, 255, 0, TRUE );
			DrawDebugBox( End, vect(5,5,5), 255, 0, 0, TRUE );
			DrawDebugBox( HitLocation, vect(5,5,5), 0, 0, 255, TRUE );
			DrawDebugLine( Start, End, 255, 0, 0, TRUE );
*/

			EnemyGP = GearPawn(Controller.Enemy);
			if( EnemyGP != None && EnemyGP.IsAKidnapper() && (HitActor == EnemyGP || HitActor == EnemyGP.InteractionPawn) )
			{
				DotP = Normal(Location-EnemyGP.Location) DOT Vector(EnemyGP.Rotation);
				if( DotP < 0 )
				{
					HitActor = EnemyGP;
				}
				else
				{
					HitActor = EnemyGP.InteractionPawn;
				}
			}

			if( HitActor == Controller.Enemy ||
				(EnemyGP != None && EnemyGP.IsAKidnapper() && HitActor == EnemyGP.InteractionPawn) )
			{
				Wpn.DoMeleeDamage( HitActor, FirstHitInfo.HitLocation, 1.f );
			}
		}
	}
}

simulated function OnDeathAnimBodyRest()
{
	Super.OnDeathAnimBodyRest();
	PlaySound(BodyFallSound);
}


/** Turn off controllers and get pawn ready for being put into physics */
simulated function ReadyPawnForRagdoll()
{
	local SkelControlLimb SCL;

	Super.ReadyPawnForRagdoll();

	// turn off the IK on this guy when he is RagDolled
	SCL = SkelControlLimb(Mesh.FindSkelControl('IKCtrl_LeftFoot'));
	if( SCL != None )		SCL.SetSkelControlActive(FALSE);

	SCL	= SkelControlLimb(Mesh.FindSkelControl('IKCtrl_RightFoot'));
	if( SCL != None )		SCL.SetSkelControlActive(FALSE);
}

/** Play Hit Reaction animation */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	local BodyStance	BS_HitReaction;
	local Name			AnimName;
	local float			DotFront, DotRight;
	local Vector		HitNormal, LookDir, RightDir;

	// Don't override animations playing except if it's the last hit reaction we've played
	if( FullBodyNode == None ||
		(FullBodyNode.bIsPlayingCustomAnim && !BS_IsPlaying(LastPlayedHitReactionStance)) )
	{
		return FALSE;
	}

	HitNormal	= Normal(-Impact.RayDir);
	LookDir		= vector(Rotation);
	RightDir	= vect(0,0,1) cross LookDir;

	DotFront = HitNormal dot LookDir;
	DotRight = HitNormal dot RightDir;

	if( Abs(DotFront) > Abs(DotRight) )
	{
		if( DotFront > 0.f )
		{
			AnimName = FRand() > 0.5f ? 'ADD_Hit_Reaction_Fwd_A' : 'ADD_Hit_Reaction_Fwd_B';
		}
		else
		{
			AnimName = 'ADD_Hit_Reaction_Back_A';
		}
	}
	else if( DotRight > 0.f )
	{
		AnimName = 'ADD_Hit_Reaction_Rt_A';
	}
	else
	{
		AnimName = 'ADD_Hit_Reaction_Lft_A';
	}

	BS_HitReaction.AnimName[BS_FullBody] = AnimName;
	NextHitReactionAnimTime = WorldInfo.TimeSeconds + RandRange(0.33f, 0.67f);

	if( BS_Play(BS_HitReaction, 0.67f, 0.05f, 0.2f) > 0.f )
	{
		LastPlayedHitReactionStance = BS_HitReaction;
		// If we have a rider, have play the same animation in sync.
		if( Driver != None )
		{
			Driver.BS_Play(BS_HitReaction, 0.67f, 0.05f, 0.2f);
		}
		return TRUE;
	}

	return FALSE;
}

event bool EncroachingOn(Actor Other)
{
	if ( !bScriptInitialized )
	{
		return false;
	}
	return Super.EncroachingOn(Other);
}

defaultproperties
{
	bCanSwim=FALSE
	bCanClimbLadders=FALSE
	bCanBeBaseForPawns=TRUE
	bCanDBNO=false
	//bNoEncroachCheck=TRUE

	PeripheralVision=-0.5f

	PelvisBoneName=none
	DriverSocket="Driver"
	SightBoneName=none

	ControllerClass=class'GearAI_Bloodmount'

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0050.000000
	End Object

	SpecialMoveClasses(SM_BloodMount_HitInFace)=class'GSM_BloodMount_HitInFace'
	SpecialMoveClasses(SM_MidLvlJumpOver)=class'GSM_BloodMount_MantleOverCover'
	SpecialMoveClasses(GSM_BloodMount_Melee)=class'GSM_BloodMount_Melee'
	bRespondToExplosions=FALSE

	SpecialMoveClasses(SM_CQC_Killer)				=None
	SpecialMoveClasses(SM_CQC_Victim)				=None
	SpecialMoveClasses(SM_CQCMove_CurbStomp)		=None
	SpecialMoveClasses(SM_CQCMove_PunchFace)		=None
	SpecialMoveClasses(SM_Execution_CurbStomp)		=None
	SpecialMoveClasses(SM_ChainSawHold)				=None
	SpecialMoveClasses(SM_ChainSawAttack)			=None
	SpecialMoveClasses(SM_ChainSawVictim)			=None
	SpecialMoveClasses(SM_RoadieRun)				=None

	AmbientTimeRange=(x=1.0,y=5.0)

	// remove this and spawn dynamically so all infantry don't get this comp
	Begin Object Class=StaticMeshComponent Name=HeadSlotStaticMesh0
		CollideActors=FALSE
		BlockRigidBody=FALSE
		HiddenGame=TRUE
		AlwaysLoadOnClient=TRUE
		bCastDynamicShadow=FALSE
		LightEnvironment=MyLightEnvironment
		MotionBlurScale=0.0
	End Object
	HeadSlotStaticMesh=HeadSlotStaticMesh0

	NoticedGUDSPriority=120
	NoticedGUDSEvent=GUDEvent_NoticedBloodmounts

	AimAttractors(0)=(OuterRadius=90.f,InnerRadius=50.f,BoneName="spine2")
	AimAttractors(1)=(OuterRadius=40.f,InnerRadius=24.f,BoneName="head")

	MPHeadRadius=20

	RoarInterval=(X=2.0,Y=8.0)
}
