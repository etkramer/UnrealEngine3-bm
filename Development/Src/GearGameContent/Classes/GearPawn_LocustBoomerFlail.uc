
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustBoomerFlail extends GearPawn_LocustBoomer
	config(Pawn)
	implements(AIObjectAvoidanceInterface);

/** BlendNode used when boomer ducks behind shield. */
var transient	AnimNodeBlend		CoverBlend;
var transient	GearAnim_BlendList	BoomerStateAnimNode;
/** Flag when it is allowed to trigger melee attack notify */
var transient	bool				bReceivedSpinToMeleeTransitionNotify;
/** Replicated state for flail boomer */
var	repnotify	byte				BoomerAnimState;
/** Angle step for proximity check */
var				INT					ProximityAngleStep;
/** Current trace direction for proximity checks. */
var				Rotator				TraceRotation;
/** Last time proximity check has hit something */
var				float				LastProximityHitTime;

var protected const float			MinTimeBetweenTelegraphs;
var protected transient float		LastTelegraphTime;

/** last time we triggered a flail explosion from the animnotify, used to make sure we only do one per swing */
var float LastFlailExplosionTime;

var AIAvoidanceCylinder AvoidanceCylinder;


replication
{
	// Replicated to ALL but Owning Player
	if( Role == ROLE_Authority)
		BoomerAnimState;
}


/************************************************************************
 * Avoidance Cylinder stuff                                             *
 ************************************************************************/
function SetupAvoidanceCylinder()
{
	local float AvoidRadius;

	if( !bDeleteMe && AvoidanceCylinder == None )
	{
		AvoidRadius = GetCollisionRadius() * 2.5f;
		AvoidanceCylinder = Spawn(class'AIAvoidanceCylinder',self,,Location,,,TRUE);

		if(AvoidanceCylinder!=none)
		{

			AvoidanceCylinder.SetBase(self);
			AvoidanceCylinder.SetRelativeLocation(vect(150.f,0.f,0.f));
			AvoidanceCylinder.SetCylinderSize(AvoidRadius,AvoidRadius*2.0f);
			AvoidanceCylinder.SetAvoidanceTeam(TEAM_COG);
			AvoidanceCylinder.SetEnabled(true);
		}
		//DrawDebugSphere(Location,AvoidRadius,16,255,0,0,TRUE);
	}

}

simulated function Destroyed()
{
	Super.Destroyed();

	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = none;
	}
}

function bool Died(Controller Killer, class<DamageType> GearDamageType, vector HitLocation)
{

	if(AvoidanceCylinder != none)
	{
		AvoidanceCylinder.Destroy();
		AvoidanceCylinder = none;
	}
	return Super.Died( Killer, GearDamageType, HitLocation );
}

function bool ShouldAvoid(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	return AskingAI.IsEnemyVisible(self);
}

function bool ShouldEvade(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	return TRUE;
}

function bool ShouldRoadieRun(GearAI AskingAI, AIAvoidanceCylinderComponent TriggeringComponent)
{
	return TRUE;
}
/************************************************************************
* END Avoidance Cylinder stuff                                             *
************************************************************************/

simulated function PostBeginPlay()
{
	super.PostBeginPlay();
	SetupAvoidanceCylinder();
}

simulated function TelegraphAttack()
{
	// throttle telegraphs for mauler
	if (TimeSince(LastTelegraphTime) > MinTimeBetweenTelegraphs)
	{
		super.TelegraphAttack();
		LastTelegraphTime = WorldInfo.TimeSeconds;
	}
}


function AdjustPawnDamage
(
	out	int					Damage,
	Pawn					InstigatedBy,
	Vector					HitLocation,
	out Vector				Momentum,
	class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
)
{
	// Maulers don't take damage from hail (hold shield up anyway... this allows Adam to spawn them in the hail w/o them dying instantly)
	// also hardcode them not taking damage from the flail
	if (ClassIsChildOf(GearDamageType, class'GDT_Hail') || GearDamageType == class'GDT_Boomer_Flail')
	{
		Damage = 0;
		return;
	}

	Super.AdjustPawnDamage( Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo );

}

simulated event ReplicatedEvent( name VarName )
{
	if( VarName == 'BoomerAnimState' )
	{
		BoomerFlailStateChanged();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

/** Cache additional animation nodes. */
simulated function CacheAnimNodes()
{
	Super.CacheAnimNodes();

	// Cover blend node.
	CoverBlend = AnimNodeBlend(Mesh.FindAnimNode('CoverBlend'));
	BoomerStateAnimNode = GearAnim_BlendList(Mesh.FindAnimNode('BoomerStateNode'));

	// Reset blend node just in case.
	CoverBlend.SetBlendTarget(0.f, 0.f);
	// Set reference to special move as well. LocustBoomerFlail is not defined in GearGame unfortunately. :(
	VerifySMHasBeenInstanced(SM_CoverHead);
	GSM_Boomer_ShieldDefense(SpecialMoves[SM_CoverHead]).CoverBlend = CoverBlend;
}

simulated function ClearAnimNodes()
{
	Super.ClearAnimNodes();

	CoverBlend = None;
	BoomerStateAnimNode = None;
	if(Specialmoves.length > SM_CoverHead)
	{
		GSM_Boomer_ShieldDefense(SpecialMoves[SM_CoverHead]).CoverBlend = None;
	}
}

function AddDefaultInventory()
{
	local GearInventoryManager Inv;
	local GearWeapon Weap;

	Super.AddDefaultInventory();

	Inv = GearInventoryManager(InvManager);
	if( Inv != None && Inv.Shield != None )
	{
		EquipShield( Inv.Shield );
	}

	//debug
	ForEach InvManager.InventoryActors(class'GearWeapon', Weap)
	{
		Weap.SetInfiniteMagazineSize();	// disable reloads
		Weap.SetInfiniteSpareAmmo();	// unlimited spare ammo
	}
}

/** Attachment bone and offsets for Shield. */
simulated function GetShieldAttachBone(out Name OutBoneName, out Rotator OutRelativeRotation, out Vector OutRelativeLocation, out Vector OutRelativeScale)
{
	local SkeletalMeshSocket	ShieldSocket;

	ShieldSocket = Mesh.GetSocketByName('Shield');

	OutBoneName = ShieldSocket.BoneName;
	OutRelativeRotation = ShieldSocket.RelativeRotation;
	OutRelativeLocation = ShieldSocket.RelativeLocation;
}

/** @see Pawn::WeaponFired */
simulated function WeaponFired(bool bViaReplication, optional vector HitLocation)
{
	// Skip parent boomer version.
	Super(GearPawn_Infantry).WeaponFired(bViaReplication, HitLocation);
}

/**
 * Play flail attack animation on pawn.
 * animation parameters are provided as input variables, so they're all set from the same place and remain identical.
 */
simulated function float PlayFlailAttack(float InRate, float InBlendInTime, float InBlendOutTime)
{
	local float	AnimTime;

	// Play boom shot firing animation
	AnimTime = BS_Play(BS_WeaponFire, InRate, InBlendInTime, InBlendOutTime, FALSE, TRUE);
	BS_SetAnimEndNotify(BS_WeaponFire, TRUE);
	bDoingMeleeAttack = TRUE;

	TelegraphAttack();

	return AnimTime;
}

simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	if( BS_SeqNodeBelongsTo(SeqNode, BS_WeaponFire) )
	{
		// Notification that the flail melee attack is done playing.
		// We resynchronize the spinning animation to create a smooth transition.
		AnimTreeRootNode.ForceGroupRelativePosition('FlailSpin', 0.3f);
		BS_SetAnimEndNotify(BS_WeaponFire, FALSE);
		bDoingMeleeAttack = FALSE;
		return;
	}

	Super.BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
}

/** Called from an anim notify. */
simulated function Notify_FlailImpact()
{
	local GearWeap_Boomer_Flail FlailWeap;
	local vector ExploLoc, HitLocation, HitNormal;

	if (WorldInfo.TimeSeconds - LastFlailExplosionTime > 1.0)
	{
		LastFlailExplosionTime = WorldInfo.TimeSeconds;
		FlailWeap = GearWeap_Boomer_Flail(Weapon);
		ExploLoc = SkeletalMeshComponent(FlailWeap.Mesh).GetBoneLocation('b_Flail_ChainEnd');
		// make sure flail didn't penetrate floor
		if (Trace(HitLocation, HitNormal, ExploLoc, ExploLoc + vect(0,0,100), false) != None)
		{
			ExploLoc = HitLocation;
		}
		FlailWeap.CauseFlailExplosion(ExploLoc, vect(0,0,1));
	}
}

simulated function FlailSpinToMeleeTransition()
{
//	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName());
	bReceivedSpinToMeleeTransitionNotify = TRUE;
}

simulated function Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);

	// Clear notify
	bReceivedSpinToMeleeTransitionNotify = FALSE;

	if( Role == ROLE_Authority )
	{
		UpdateProximityCheck();
		UpdateFlailBoomerAnimState();
	}
}

// Server only, updates state of flail boomer
function UpdateFlailBoomerAnimState()
{
	if( BoomerStateAnimNode == None )
	{
		return;
	}

	// If in Hail, then play shield raise animation
	if( IsInHailVolume() )
	{
		BoomerAnimState = 2;
	}
	// no swing if no enemies.
	else if( GearAI(Controller) != None && GearAI(Controller).Enemy == None )
	{
		BoomerAnimState = 3;
	}
	// Short swing
	else if( ShouldUseShortSwing() )
	{
		BoomerAnimState = 0;
	}
	// Large swing
	else
	{
		BoomerAnimState = 1;
	}

	if( BoomerStateAnimNode.ActiveChildIndex != BoomerAnimState )
	{
		// Force replication now to clients
		bForceNetUpdate = TRUE;
		BoomerFlailStateChanged();
	}
}

// Server only, do traces for proximity check
function UpdateProximityCheck()
{
	local Vector	StartTrace, EndTrace;
	local float		HeightRandomizer;

	TraceRotation.Yaw += ProximityAngleStep;
	TraceRotation = Normalize(TraceRotation);

	HeightRandomizer = GetCollisionHeight() * 0.67f * (FRand() - 0.5f);
	StartTrace = Location + vect(0,0,1) * HeightRandomizer;
	EndTrace = StartTrace + Vector(TraceRotation) * 200.f;
// 	DrawDebugLine(StartTrace, EndTrace, 255, 0, 0);
	if( !FastTrace(EndTrace, StartTrace) )
	{
		LastProximityHitTime = WorldInfo.TimeSeconds;
	}
}

/**
 * Determine if we should use our short swing as opposed to long swing
 * by attaching a trigger to use, and seeing if anything collides with it.
 */
function bool ShouldUseShortSwing()
{
	// If we've hit the world in the last second, then use the short swing.
	return TimeSince(LastProximityHitTime) < 1.f;
}

// Both client and server. Updates boomer state based on replicated variable BoomerAnimState.
simulated function BoomerFlailStateChanged()
{
	BoomerStateAnimNode.SetActiveChild(BoomerAnimState, 0.67f);

	// If protecting from hail, then open up shield.
	if( BoomerAnimState == 2 )
	{
		OpenShield( FALSE );
	}
}

simulated function bool ShouldDeployShield()
{
	// we'll control when we want to deploy, thank you very much
	return false;
}

simulated function OpenShield( bool bModGroundSpeed, optional float ModGroundSpeed, optional bool bDoSpecialMove )
{
	if( !IsInHailVolume() )
	{
		if( EquippedShield != None && !EquippedShield.bExpanded )
		{
			EquippedShield.Expand();
			if( bModGroundSpeed )
			{
				ShouldCrouch( TRUE );
			}
		}

		if( bDoSpecialMove && !IsDoingSpecialMove( SM_CoverHead ) )
		{
			DoSpecialMove( SM_CoverHead );
		}

		if( bModGroundSpeed )
		{
			GroundSpeed = ModGroundSpeed;
		}
	}
}

simulated function CloseShield()
{
	if( EquippedShield != None && EquippedShield.bExpanded )
	{
		EquippedShield.Retract();
		ShouldCrouch( FALSE );
	}

	if( IsDoingSpecialMove( SM_CoverHead ) )
	{
		EndSpecialMove();
	}

	GroundSpeed = DefaultGroundSpeed;
}

/** Boomer never damaged hail, he has shield! */
simulated function bool TryToRaiseShieldOverHead()
{
	return TRUE;
}

simulated final function bool IsInHailVolume()
{
	return (PhysicsVolume != None && PhysicsVolume.bPainCausing && ClassIsChildOf(PhysicsVolume.DamageType, class'GDT_Hail'));
}

simulated function bool CanEngageMelee()
{
	// Cannot attack while in Hail volume.
	if( IsInHailVolume() )
	{
		return FALSE;
	}

	return Super.CanEngageMelee();
}

/** Delay to start melee attack until proper animation transition */
simulated function bool ShouldDelayMeleeAttack()
{
	// If we cant trigger melee attack, do so.
	if( bReceivedSpinToMeleeTransitionNotify )
	{
		return FALSE;
	}

	return TRUE;
}

/** Play Hit Reaction animation */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	return FALSE;
}

// only do flame damage reduction if our shield is deployed since we hold the shield at an off angle othwrewise
function float ReduceDamageForShieldHolder(vector ShotDirection, class<GearDamageType> DamType, const out TraceHitInfo HitInfo, float Damage, optional bool bOnlyReduceIfHitShield)
{
	local vector	Dir;
	local float		DotAngle, Factor, ProtAngle;

	Dir = -Normal(ShotDirection);
	DotAngle = Dir dot vector(Rotation);
	// If being shot from the front
	if (DotAngle > 0.f)
	{
		ProtAngle = IsDeployingShield() ? DeployedShieldHalfAngleOfProtection : ShieldHalfAngleOfProtection;
		Factor = FClamp(Square(1.f - DotAngle) * (90.f / ProtAngle), 0.f, 1.f);
	}
	else
	{
		Factor = 1.0;
	}

	if ( EquippedShield.bExpanded && (
		ClassIsChildOf(DamType, class'GDT_Explosive') || ClassIsChildOf( DamType, class'GDT_Fire' ) || ClassIsChildOf( DamType, class'GDT_Melee' )
									 )
	   )
	{
		//		`log(WorldInfo.TimeSeconds @ Self$"::"$GetStateName()$"::"$GetFuncName() @ "Factor:" @ Factor @ "Orig Damage:" @ Damage @ "Reduced Damage:" @ (Damage*Factor));

		// Reduce damage
		Damage *= Factor;
	}
	else
	{
		// did we hit the shield itself?
		if (ComponentIsAnEquippedShield(HitInfo.HitComponent))
		{
			Damage = 0.f;
		}
		else if (IsDeployingShield())
		{
			// give some damage reduction in front even in the areas not covered by the shield
			Damage *= FMax(Factor, 0.5);
		}
	}
	//`log("*** Reducing damage for shield"@HitInfo.HitComponent@Damage);

	return Damage;
}

defaultproperties
{
	ProximityAngleStep=8000	// spread it over a few frames
	bCanPlayPhysicsHitReactions=FALSE	// turn off because doesn't play well with physics flail.

	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter")
	FAS_ChatterNames.Add("Locust_Boomer.FaceFX.boomer_FaceFX_Boomer2Chatter_Dup")

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_Boomer_Flail'
	DefaultInventory(1)=class'GearGameContent.BoomerShield'

	HelmetType=class'Item_Helmet_LocustBoomerFlail'

	BS_WeaponFire=(AnimName[BS_Std_Up]="FL_MeleeAttack_Overhand",AnimName[BS_Std_Idle_Lower]="FL_MeleeAttack_Overhand")
	BS_MeleeAttack=(AnimName[BS_Std_Up]="FL_MeleeAttack_Overhand",AnimName[BS_Std_Idle_Lower]="FL_MeleeAttack_Overhand")

	SpecialMoveClasses(SM_CoverHead)=class'GSM_Boomer_ShieldDefense'

	Begin Object Name=GearPawnMesh
		bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		AnimSets.Add(AnimSet'Locust_Boomer.Animations.AnimSetBoomer_CamSkel_Cleaver')
		AnimSets.Add(AnimSet'Locust_Boomer.Animations.AnimSetBoomer_CamSkel_Flail')
		AnimTreeTemplate=AnimTree'Locust_Boomer.Animations.AT_LocustBoomerFlail'
	End Object

	// flail boomer (aka Mauler) is boomerA, says "Crush"
	BoomerAttackTelegraphDialogue.Empty
	BoomerAttackTelegraphDialogue(0)=SoundCue'Locust_Boomer2_Chatter_Cue.DupedRefsForCode.Boomer2Chatter_Crush_Loud01Cue_Code'
	BoomerAttackTelegraphDialogue(1)=SoundCue'Locust_Boomer_Chatter_Cue.DupedRefsForCode.BoomerChatter_Crush01Cue_Code'
	BoomerAttackTelegraphDialogue(2)=SoundCue'Locust_Boomer1_Chatter_Cue.AttackingEnemy.Boomer1Chatter_Crush_Loud04Cue'

	MinTimeBetweenTelegraphs=4.f

	BoomerChargeTelegraphDialogue.Empty
	BoomerChargeTelegraphDialogue(0)=SoundCue'Locust_Boomer1_Chatter_Cue.AttackingEnemy.Boomer1Chatter_Charge_Loud02Cue'
	BoomerChargeTelegraphDialogue(1)=SoundCue'Locust_Boomer_Chatter_Cue.DupedRefsForCode.BoomerChatter_Charge02Cue_Code'
	BoomerChargeTelegraphDialogue(2)=SoundCue'Locust_Boomer_Chatter_Cue.DupedRefsForCode.BoomerChatter_ScreamCharging03Cue_Code'

	NoticedGUDSEvent=GUDEvent_NoticedMauler

}
