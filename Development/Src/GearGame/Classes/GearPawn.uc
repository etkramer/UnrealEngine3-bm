 /**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPawn extends GamePawn
	dependson(GearDamageType,GearTypes)
	config(Pawn)
	native(Pawn)
	nativereplication
	placeable			// kludge to make all subclasses placeable
	abstract;

/** Logging pre-processor macros */
/** GoW global macros */
`include( GearGame\GearStats.uci )

var bool bIsOnTurret;

/** Struct used to replicated root body position when knocked down (not dead) */
struct native ReplicatedRootPosInfo
{
	var vector	Position;
	var byte	bNewData;
};

var	ReplicatedRootPosInfo ReplicationRootBodyPos;

const	MaxCoverGuessDist = 256.f;

var bool bScriptedWalking;

var config float MeatflagProtectionPct;

/** Object within GearPawn which handles FX **/
var transient GearPawnFX GearPawnFX;
var class<GearPawnFX> GearPawnFXClass;

/** Additional translation for mesh */
var const transient vector	MeshTranslationOffset;
/** Additional yaw for mesh */
var const transient float MeshYawOffset;

/**
 * This is when meshes are authored with the root bone between their legs.
 * If TRUE, then mesh will be offset down by -CollisionHeight - Magic Number so feet touch the floor properly
 */
var const			bool	bTranslateMeshByCollisionHeight;
var const			float	MeshTranslationNudgeOffset;

var const config float SceneRatingValue;

/** Reference to GearWeapon carried by Pawn. Set in PlayWeaponSwitch() when a new weapon is set. */
var GearWeapon MyGearWeapon;
/** cached cast of Controller to GearAI */
var GearAI MyGearAI;

/** Cached bIsInCombat flag set by the server via GearPC.IsInCombat() */
var bool bIsInCombat;
/** This pawn should never be considered a valid enemy by AI */
var bool bNeverAValidEnemy;
/** This pawn is never a valid melee target */
var bool bInvalidMeleeTarget;

/** Is rotation locked?	 Currently for scripting only */
var bool bLockRotation;

var ParticleSystem	GoreImpactParticle;
/** Name of base blend anim node */
var Name			FullBodyNodeName;
/** Base blend node */
var GearAnim_Slot	FullBodyNode;

/** Character name of this pawn, ie. Dom,Marcus,Locust Drone */
var localized String CharacterName;

enum ESituationMood
{
	SITMOOD_Default,
	SITMOOD_Tense,
};
var	ESituationMood	SitMood;

/** Actor to head-track */
var Actor	HeadLookAtActor;
/** Specific bone on the HeadLookAtActor to track */
var Name	HeadLookAtBoneName;
/** HeadTrackTarget that is controlling this pawn's head, if any */
var transient private HeadTrackTarget CurrentHeadTrackTargetActor;

/** Limiter on gib effects */
var int EffectCount;

/** Defines the current move the player is doing */
enum ESpecialMove
{
	SM_None,
	/** Mantle over cover */
	SM_MidLvlJumpOver,
	/** Swat turns in cover */
	SM_StdLvlSwatTurn,
	/** Run to the edge of cover */
	SM_CoverRun,
	/** Run out of cover until button is released */
	SM_RoadieRun,
	/** Slip out of cover */
	SM_CoverSlip,
	/** Mantle up and down from cover */
	SM_MantleUpLowCover,
	SM_MantleDown,
	/** Evades */
	SM_EvadeFwd,
	SM_EvadeBwd,
	SM_EvadeLt,
	SM_EvadeRt,

	/** Transition to cover */
	SM_Run2MidCov,
	SM_Run2StdCov,

	/** movement 2 idle transitions */
	SM_Run2Idle,
	SM_Walk2Idle,

	/** Push Out of Cover */
	SM_PushOutOfCover,

	// Reaction animations
	SM_FlankedReaction_Left,
	SM_FlankedReaction_Right,
	SM_FlankedReaction_Back,
	SM_FlankedReaction_Front,
	SM_Reaction_InitCombat,
	SM_FullBodyHitReaction,

	/** Stumble Actions */
	SM_StumbleFromMelee,
	SM_StumbleFromMelee2,
	SM_StumbleGoDown,
	SM_StumbleGoDownFromExplosion,
	SM_StumbleGoDownFromCloseRangeShot,
	SM_StumbleGetUp,
	SM_CoverHead,
	SM_StumbleBackOutOfCover,
	SM_DBNO,
	SM_RecoverFromDBNO,
	SM_RoadieRunHitReactionStumble,

	// Other subactions
	SM_Emerge_Type1,
	SM_Emerge_Type2,

	SM_DeathAnim,
	SM_DeathAnimFire,
	SM_RecoverFromRagdoll,

	SM_ChainSawHold,
	SM_ChainSawAttack,
	SM_ChainSawVictim,
	SM_ChainsawDuel_Leader,
	SM_ChainsawDuel_Follower,
	SM_ChainsawDuel_Draw,

	SM_ChainSawAttack_Object,
	SM_ChainSawAttack_Object_NoCamera,

	SM_EvadeFromCoverCrouching,
	SM_EvadeFromCoverStanding,

	// Pawn to Pawn Interactions
	SM_CQC_Killer,
	SM_CQC_Victim,
	SM_CQCMove_CurbStomp,
	SM_CQCMove_PunchFace,
	SM_CQCMove_Shield,
	SM_CQCMove_B,
	SM_Execution_CurbStomp,
	SM_ReviveTeammate,
	SM_GrabWretch,
	SM_GrabWretchFollower,
	SM_Kidnapper,		// Meat Shield, kidnapper
	SM_Hostage,			// Meat Shield, hostage
	SM_Kidnapper_Execution,

	// Interactions
	SM_WeaponPickup,
	SM_DoorPush,
	SM_DoorKick,
	SM_PushButton,
	SM_Engage_Loop,
	SM_Engage_Start,
	SM_Engage_End,
	SM_Engage_Idle,
	SM_Engage_ForceOff,
	SM_LadderClimbUp,
	SM_LadderClimbDown,
	SM_GrapplingHook_Climb,

	SM_PushObject,
	SM_UsingCommLink,
	SM_PutOnHelmet,

	// Special weapon moves
	SM_TargetMortar,
	SM_TargetMinigun,
	SM_UnMountMinigun,
	SM_UnMountMortar,
	SM_DeployShield,
	SM_PlantShield,
	SM_RaiseShieldOverHead,
	SM_ShieldBash,

	// Wretch special moves
	GSM_PounceAttack,	// Leap attack
	GSM_SwipeAttack,	// Ground attack
	GSM_Scream,			// Scream attack
	GSM_LeapToCeiling,
	GSM_DropFromCeiling,
	GSM_Cart_ClimbUp,
	GSM_Cart_SwipeAttack,
	GSM_Elevator_Climb,	// Climbs over the lip of the elevator
	SM_Wretch_GrabAttack,

	// Berserker special moves
	SM_Berserker_Smash,			// Close range melee attack
	SM_Berserker_Charge,		// Distance melee attack
	SM_Berserker_Collide,		// Successful charge collision (ie w/ player or door that got knocked over)
	SM_Berserker_Stunned,		// Stunned after hitting a wall w/ charge attack
	SM_Berserker_Slide,			// No collision happened before charge has ended
	SM_Berserker_HODHitReaction,// Hammer or Dawn Hit Reaction
	SM_Berserker_Alert,			// Alert search mode

	// Brumak
	GSM_Brumak_Roar,
	GSM_Brumak_RtGunHit,
	GSM_Brumak_LtGunHit,
	GSM_Brumak_CannonHit,
	GSM_Brumak_CannonFire,
	GSM_Brumak_MeleeAttack,
	GSM_Brumak_OverlayLftArmSwing,
	GSM_Brumak_OverlayRtArmSwing,
	GSM_Brumak_OverlayBite,
	GSM_Brumak_Scream,
	GSM_Brumak_LtGunPain,
	GSM_Brumak_RtGunPain,
	GSM_Brumak_ExposeDriver,

	// Bloodmount
	SM_BloodMountDriver_CalmMount,
	SM_BloodMount_HitInFace,
	GSM_BloodMount_Melee,

	// Kantus
	GSM_Kantus_KnockDownScream,
	GSM_Kantus_ReviveScream,
	GSM_Kantus_SummoningScream,

	// Sire
	GSM_Sire_MeleeHeadGrab,
	GSM_Sire_TankFall,

};

/** Container for all special move properties */
struct native SMStruct
{
	/** Special Move Enum being performed. */
	var ESpecialMove	SpecialMove;
	/** Interaction Pawn */
	var GearPawn		InteractionPawn;
	/** Additional Replicated Flags */
	var INT				Flags;
};

/** Special move currently performed by Pawn. SM_None, when doing none. */
var	ESpecialMove					SpecialMove, PreviousSpecialMove;
/** Special move pending, activated after the current one terminates. */
var SMStruct						PendingSpecialMoveStruct;
/** Are we currently in the process of ending a special move, and should force new moves to delay? */
var transient bool					bEndingSpecialMove;

/**
 * Special move currently performed by Pawn. SM_None, when doing none. This is necessary
 * because the previous state of SpecialMove is used in DoSpecialMove(), so if the
 * network replication stuff just blows away SpecialMove on the client, then
 * the "from" specialmove information is lost and things no worky.
*/
var	repnotify SMStruct				ReplicatedSpecialMoveStruct;

/** Array matching above enumeration. List of classes to Instance */
var Array<class<GearSpecialMove> >	SpecialMoveClasses;
/** Array of instanced special moves */
var(SpecialMoves) editinline editconst transient Array<GearSpecialMove> SpecialMoves;

/** Pawn ref used for Pawn to Pawn Interactions */
var				GearPawn			InteractionPawn;
/** INT to pack any additional data into special moves, and get it replicated so it's consistent across network. */
var				INT					SpecialMoveFlags;

/** List of Special Moves checked for Pawn to Pawn interactions. */
var	Array<ESpecialMove>				PawnToPawnInteractionList;

/** Useful replication vector that can be used tom communicate a location to the SpecialMove on the client. */
var	vector							SpecialMoveLocation;

/** Special moves that should check to display a HUD indicator (while in cover) */
var array<ESpecialMove>				CoverActionSpecialMoves;

/** Special moves that should check to display a HUD indicator (whether or not in cover) */
var array<ESpecialMove>				ActionSpecialMoves;

/** SpecialMove done when dying. */
var ESpecialMove					SpecialMoveWhenDead;
/** Saved Base when dying */
var Actor							BaseWhenDead;

// CQC moves enum
enum ECQCMove
{
	CQCMove_None,
	CQCMove_MeatBag,
	CQCMove_WeaponKill,
	CQCMove_CharacterKill,
	CQCMove_CurbStomp,
};

/** Current translation applied to */
var	transient float					MeshFloorConformTranslation;
/** How quickly we modify the mesh translation to conform to ground (units per second) */
var(FloorConform) float				MeshFloorConformTransSpeed;
/** How quickly we modify the mesh rotation to conform to ground (degrees per second) */
var(FloorConform) float				MeshFloorConformRotSpeed;
/** If TRUE, rotate mesh back to (0,0,0) unless special move desires floor conforming. */
var bool							bEnableFloorRotConform;

/** If TRUE, translation of mesh is updated to match desired floor translation (0 unles special move desired floor conforming) */
var bool							bDisableMeshTranslationChanges;

/** monitor last time Pawn bumped into something */
var	transient				float		LastBumpTime;

/** Replicated cover information to play transition animations on remote clients. */
var	repnotify				CovPosInfo	AcquiredCoverInfo;


/**
 * TRUE if this Pawn can play the run2cover slide special move.
 * if FALSE, Pawn will acquire cover without playing this transition.
 */
var(SpecialMoves)			bool		bCanDoRun2Cover;

/**
 * Maximum distance allowed from cover to do run 2 cover special move.
 * Above that distance, move will be denied, and player will evade instead.
 */
var(SpecialMoves)	config	float		Run2CoverMaxDist;

/**
* Maximum distance allowed from cover to do run 2 cover special move when moving perpendicular
*/
var(SpecialMoves)	config	float		Run2CoverPerpendicularMaxDist;



/** Max distance to slide to cover at the end of an evade move */
var(SpecialMoves)	config	float		EndEvadeRun2CoverMaxDist;

/** Max Distance to check for a transition from Roadie Run to cover slide */
var(SpecialMoves)	config	float		RoadieRun2CoverSlideDist;

// COVER DEBUGGING
/** Player search FOV */
var globalconfig	bool	bCoverDebug_PlayerFOV;
/** Cover acquisition FOV */
var globalconfig	bool	bCoverDebug_CoverFOV;
/** Cover acquisition volume */
var globalconfig	bool	bCoverDebug_CoverVolume;
/** Considered cover links for search */
var globalconfig	bool	bCoverDebug_ConsideredLinks;
/** Draw found cover position */
var globalconfig	bool	bCoverDebug_FoundCover;
/** Draw validated cover */
var globalconfig	bool	bCoverDebug_ValidatedCover;

// Weapon debugging
var globalconfig	bool	bWeaponDebug_Accuracy;
var globalconfig	bool	bWeaponDebug_DamageRadius;

/** if this is set then the weapon equiping/de equiping / highlighting sounds will not play @see ClientRestart **/
var bool bQuietWeaponEquipping;

/** Represents the type of cover currently being used, for animation */
var ECoverType		CoverType;

/**
 * Represents the current action this pawn is performing at the
 * current cover node.
 */
var ECoverAction	CoverAction, PreviousCoverAction, SavedCoverAction;

/** Time at which cover was entered or left */
var float LastCoverTime;

/** How far away from cover this pawn was the last time he acquired cover (squared) */
var float LastCoverAcquireDistanceSq;

/** Time at which a cover action was taken */
var float LastCoverActionTime;
/** Time at which next cover action can be taken
(< 0 value means left actions ignore, > 0 value means right actions ignore) */
var float NextCoverActionTime;

/**
 * True when animations are mirrored. True if player is looking/moving to the left in cover.
 * Do not set direction, call SetMirroredSide() which will set bWantsToBeMirrored accordingly.
 * Because mirroring requires an animation transition, which introduces a delay.
 * This is not an instant change of direction.
 */
var	const	bool	bIsMirrored;

/**
 * Flag set indicating which side Pawn would like to be.
 * This action can take a little time to complete.
 * bIsMirrored will be updated when mirroring happened.
 */
var			bool	bWantsToBeMirrored;

/**
 * Flag set when Pawn is doing a mirror transition
 */
var	const transient bool	bDoingMirrorTransition;


/** TRUE if should play walk 2 idle transitions. This takes over input when Pawn stops moving */
var	bool	bDoWalk2IdleTransitions;

/** Flag to toggle 360 aiming in cover. */
var bool	bCanDo360AimingInCover;

/** Is player doing 360 aiming? */
var transient bool	bDoing360Aiming, bWasDoing360Aiming;
var transient float	Last360AimingChangeTime;

/** The last time the Pawn was moving */
var transient float LastTimeMoving;

/** Pawn has been injured and is crawling on the ground */
var bool	bIsCrawling;

/** Turn off bNoEncroachCheck when this pawn goes to ragdoll. */
var bool	bEnableEncroachCheckOnRagdoll;

/**
 * When set to TRUE, modify damage in TakeDamage(), otherwise keep the incoming damage value.
 * Used for example by grenade melee to kill the enemy, even if he's behind cover.
 */
var bool	bAllowDamageModification;

/** Whether or not we are in the wait state for the damage sound so we dont' play this over and over **/
var transient bool PlayBurstDamageWarningSoundSemaphore;

/** Used to track when HOD damage was applied */
var float	LastHODDamageTime;

/** Global modifier that effects all movement modes */
var() float	MovementPct;

/** Array of movement scales to set when in the matching CoverType */
var() array<float> CoverMovementPct;

/**
 * if TRUE, steps smoothing code will be performed.
 * This compensates Z movement of the pawn down/up stairs by offsetting the mesh, and interpolating it
 * smoothly back.
 */
var()	bool	bCanDoStepsSmoothing;

/** Default inventory added via AddDefaultInventory() */
var() array< class<Inventory> > DefaultInventory;

/** Configurable health setting */
var() config int DefaultHealth;

/** Configurable ground speed setting */
var() config float DefaultGroundSpeed;

var transient bool bAllowSpeedInterpolation;
var transient float LastMaxSpeedModifier;

var bool bHealthIsRecharging;
var() SoundCue HealthRechargeStartSound;

var SoundCue DeadBodyImpactSound;
var float DeadBodyImpactThreshold;
/** Float value of health to handle deltatime being too small for insane difficulty settings */
var transient float RechargingHealthAmount;

/** Delay before health starts recharging */
var() config float HealthRechargeDelay;

/** Amount of recharge per second */
var() config float HealthRechargePercentPerSecond;

/** Tracks when the Pawn died */
var transient float TimeOfDeath;

/** Whether or not we are scaling to zero **/
var transient protected bool bScalingToZero;

/** Are we currently bleeding out? */
var transient bool bIsBleedingOut;

/** the Pawn who killed me */
var GearPawn KilledByPawn;

/** my location when I died.  used for death cam. */
var Vector	LocationWhenKilled;
var Rotator RotationWhenKilled;

/** True if switched to Gore Skeleton/Physics Asset */
var bool	bIsGore;
/** True if Gore has been setup for Gibs. Sets up some optimizations so Gibs don't kill performance. */
var bool	bGoreSetupForDeath;
/** If TRUE we broke at least one constraints off the Gore mesh. */
var bool	bHasBrokenConstraints;
/** Whether we have GoreExploded or not.  We only want to play GoreExplode effects once **/
var transient bool bHasGoreExploded;

/** Whether or not the pawn is using the new softweighted gore and the stretchies have been fixed.  If yes then do it!  Else use Gear1 break constraints on alllll **/
var bool bUsingNewSoftWeightedGoreWhichHasStretchiesFixed;

/** If TRUE, when recovering from ragdoll, don't allow kicking of corpses any more */
var bool bDisableKicksCorpseAfterRecovery;

/** whether I am currently charging my bow */
var repnotify bool bChargingBow;

/** If a gib physics body moves further than this from the Actor location we kill it */
var float	GibKillDistance;

/** Maximum number of Dead RagDoll Pawns allowed. So frame rate doesn't get too bad. */
var config int	MaxAllowedDeadRagDollPawns;

/** Scale melee damage for AI driven pawns by this */
var config float AI_MeleeDamageScale;

/** List of bone names used for validating head/leg shots. */
var() config array<Name> HeadBoneNames;
var() config array<Name> LegBoneNames;
var() config Name TorsoBoneName;

/** Attached to the neck for HeadShots. To hide the bone scaling ugliness. */
var StaticMeshComponent		HeadShotNeckGoreAttachment;
/** HeadChunks rigidbodies to Spawn when a HeadShot happens */
var array<StaticMesh>		HeadChunks;

/** if TRUE, Play death animation */
var	bool		bPlayDeathAnimations;
/** if TRUE, this pawn can play head shot deaths. */
var bool		bCanPlayHeadShotDeath;
/** TRUE if this Pawn has played a head shot death */
var bool		bHasPlayedHeadShotDeath;
/** List of potential death animations to play */
var	Array<Name>	DeathAnimHighFwd, DeathAnimHighBwd, DeathAnimStdFwd, DeathAnimStdBwd, DeathAnimStdLt, DeathAnimStdRt;
// @See LastDeathAnimHighFwdIndex in GearGRI. Used to not play the same animation twice consecutively.

/** If Pawn can or cannot play physics hit reactions. */
var					bool		bCanPlayPhysicsHitReactions;
/** List of RB bone names which should be UnFixed when playing a Physics Body Impact. */
var(HitReactions) editinline Array<Name> PhysicsBodyImpactBoneList;
/** Internal counter */
var					float		PhysicsImpactBlendTimeToGo;
/** Scale impulses by this amount for Physics Hit Reactions. */
var(HitReactions)	float		PhysicsHitReactionImpulseScale;
/** Time it takes to blend back to animations after being turned into physics. */
var(HitReactions)	float		PhysicsImpactBlendOutTime;
/** Scales the effect of Mass on PhysicsImpacts */
var(HitReactions)	float		PhysicsImpactMassEffectScale;
/** Minimum motor strength when hit */
var(HitReactions)	Vector2D	PhysHRMotorStrength;
var(HitReactions)	Vector2D	PhysHRSpringStrength;
var(HitReactions)	bool		bEnableHitReactionBoneSprings;

// Interface for remapping physical impacts
// Some bones don't react too well to impulses, so we forward them to other bones
// which provide a much better visual result
struct native PhysicsImpactRBRemap
{
	var()	Name	RB_FromName;
	var()	Name	RB_ToName;
};

/** Table to remap RigidBodies for physical impacts */
var(HitReactions) editinline	Array<PhysicsImpactRBRemap> PhysicsImpactRBRemapTable;

/**
* List of RidigBodies where a spring should be attached to animation.
* This helps for e.g. the hand carrying the weapon to not fly away, but remain somewhat close to the animation.
*/
var(HitReactions) editinline	Array<Name>					PhysicsImpactSpringList;


/** Array of Breakable bone names to use when gibbing a GoreMesh. */
var Array<Name>	GoreBreakableJoints;
var Array<Name>	GoreBreakableJointsTest;

/** For a numbr of characters, when the spine breaks we need to break the upper right arm constraint as there is not enough flesh there to look good **/
// list of bones that when broken need to break other bones
struct native DependantBreakInfo
{
	var name ParentBone; // name of the bone to break dependants of
	var array<Name> DependantBones; // list of bones that need to get broken when parent bone does
};
var Array<DependantBreakInfo> JointsWithDependantBreaks;


/** Struct indicating how much to scale a particular joint limit upon complete ragdoll death. */
struct native DeadRagdollLimitScale
{
	var()	name	RB_ConstraintName;
	var()	float	Swing1Scale;
	var()	float	Swing2Scale;
	var()	float	TwistScale;
};

/** Information on how to scale limits for a ragdoll upon complete death - to prevent weird positions. */
var() array<DeadRagdollLimitScale>	RagdollLimitScaleTable;
/** Time taken to scale limits to their target. */
var() float							TimeToScaleLimits;
/** How much longer to scale limits to target. */
var	float							ScaleLimitTimeToGo;

/** The Gore SkeletalMeh **/
var SkeletalMesh GoreSkeletalMesh;
/** The Gore PhysAsset **/
var PhysicsAsset GorePhysicsAsset;
/** MorphTarget sets for Gore Mesh */
var Array<MorphTargetSet>	GoreMorphSets;

/** Speed that a gib has to move at to fire effects (ie call PlayGibEffect event). */
var	float GibEffectsSpeedThreshold;

/** Minimum time between calls to PlayGibEffect. */
var	float TimeBetweenGibEffects;

/** Efect used when using a 'simple' death, using just a particle effect. */
var ParticleSystem	SimpleDeathEffect;
/** Efect used when using a 'simple' death, using just a particle effect. */
var ParticleSystem	SimpleDeathEffectNonGore;
/** Scale to use when firing SimpleDeathEffect */
var float			SimpleDeathEffectScale;

/** Character sound group archetype */
var() GearSoundGroup	SoundGroup;

/**
 * FOV at which a player is considered safe behind cover.
 * Enemy shots with a direction within that FOV will be ignored
 */
var() vector2d CoverProtectionFOV;

/** Time Pawn is going to keep aiming after firing a weapon, before returning to idle/ready pose */
var	config		float	AimTimeAfterFiring;

/** Is Pawn targeting? Used by animations. */
var				bool	bIsTargeting;
var				bool	bIsZoomed;

/** Force pawns into aiming stance */
var transient	float	ForcedAimTime;
/** Replicated ForcedAimTime, because WorldInfo.TimeSeconds is not consistent accross server and clients */
var repnotify transient	BYTE	ReplicatedForcedAimTime;

/** Pending Fire is set when a weapon fire is delayed because of animation blends/transitions. */
var transient	bool	bPendingFire, bPendingAltFire, bPendingMeleeAttack;

//
// Slot attachments
//

/** neck bone name */
var()	name	NeckBoneName;
/** Name of Pelvis Bone */
var()	name	PelvisBoneName;
/** This is the bone to use for melee damage hit location  */
var()	name	MeleeDamageBoneName;
/** Offset from neck bone to get view location */
var()	Vector	ViewLocationOffset;

/** foot bone names **/
var name LeftFootBoneName;
var name RightFootBoneName;

/** "Knee" bone names **/
var name LeftKneeBoneName;
var name RightKneeBoneName;

/** Hand Bone names **/
var name LeftHandBoneName;
var name RightHandBoneName;

/** Structure used to replicate weapon information to remote clients */
struct native WeaponRepInfo
{
	var		class<GearWeapon>	WeaponClass;
	var		BYTE				BYTE_bHadAPreviousWeapon;
};

/** Current weapon replicated to remote clients. */
var		repnotify	WeaponRepInfo	RemoteWeaponRepInfo;

/** Socket names */
var	const	Name	RightHandSocketName, LeftHandSocketName, HeadSocketName;

var		repnotify	class<Inventory>	AttachClass_Shield;

/** replicated slots */
var		repnotify	class<Inventory>	AttachSlotClass_Holster;
var		repnotify	class<Inventory>	AttachSlotClass_Belt;
var		repnotify	class<Inventory>	AttachSlotClass_LeftShoulder;
var		repnotify	class<Inventory>	AttachSlotClass_RightShoulder;

/** Player attachment slots definition */
enum EAttachSlot
{
	EASlot_Holster,			// Carrying Pistol
	EASlot_Belt,			// Carrying	Items/grenades
	EASlot_LeftShoulder,	// Carrying medium or heavy weight weapons
	EASlot_RightShoulder,	// Carrying medium or heavy weight weapons
	EASlot_None,			// MUST ALWAYS BE DEFINED LAST, means weapon has no slot assigned
};

/** player local attachment slot definition */
struct native SAttachSlot
{
	/** Socket name to attach attachment to */
	var()	name								SocketName;
	/** True to attach to bone named [SocketName], rather than a socket of that name. */
	var()	bool								bAttachToBone;
	/** Class of inventory attachment */
	var()	class<Inventory>					InvClass;

	var()	editinline	StaticMeshComponent		StaticMesh;

	var()	editinline	SkeletalMeshComponent	SkeletalMesh;
};

/** local attachment slots */
var()	Array<SAttachSlot>	AttachSlots;

/** This pawn is capable of ... */
var config bool		bCanRoadieRun;
var config bool		bCanBeForcedToRoadieRun; // if designer tells us to, we can roadie run (e.g. for guys we don't want roadie running normally, but their skeleton supports it)
var config bool		bCanBlindFire;

// AI INFO
/** Bone name for determining head direction */
var()	name	SightBoneName;

/** Chance this AI type has to blind fire */
var config float	BlindFirePct;
// FIRE TICKET VARIABLES
/** Number of fire tickets allowed for this pawn -- -1 == Infinite */
/** CALL SetNumberOfFireTickets TO CHANGE */
var config int		NumFireTickets;
/** Claimed fire tickets for this pawn */
var array<Controller> FireTickets;

/** if part of a Horde wave, the index of this enemy in the Horde enemy list */
var int HordeEnemyIndex;

enum EBodyStance
{
	/** Full Body */
	BS_FullBody,
	/** Standing Upper Body */
	BS_Std_Up,
	BS_Std_Upper_Harsh,	// No smooth interpolation along spine	& shoulders
	BS_Std_Upper_NoAim, // Bypasses AimOffsets (turn in place twist)
	/** Standing, idle, full body, only when not targeting or firing. */
	BS_Std_idle_FullBody,
	/** Standing, idle, lower body */
	BS_Std_Idle_Lower,
	BS_Std_Idle_Upper,
	BS_Std_Walk_Upper,
	BS_Std_Run_Upper,
	/** Standing cover */
	BS_CovStdIdle_Up,
	BS_CovStdBlind_Up,
	BS_CovStdLean_Up,
	BS_CovStd_360_Upper,
	/** mid level cover */
	BS_CovMidIdle_Up,
	BS_CovMidBlindSd_Up,
	BS_CovMidBlindUp_Up,
	BS_CovMidLean_Up,
	BS_CovMid_360_Upper,
	/** Overlays/Additive */
	BS_Additive,
	BS_Kidnapper_Upper,
	BS_Hostage_Upper,
	BS_Shield_Hunkered,
	BS_MortarMounted,
};

struct native CustomCameraAnimPropertiesStruct
{
	/**
	* true if the camera system should only do single-ray penetration checks while this
	* anim is playing.	basically disables predictive camera zooming.
	*/
	var bool bSingleRayPenetrationOnly;

	/**
	* true if the camera bone code apply the entirety of the camera bone motion to
	* the camera.  basically disables camera bone motion dampening, useful for special
	* camera moves.
	*/
	var bool bApplyFullMotion;

};

/** Cache nodes for convenience */
var		transient	Array<AnimNodeAimOffset>	AimOffsetNodes;
var()	transient	Array<GearAnim_Slot>		BodyStanceNodes;
var		transient	GearAnim_Slot				CustomCameraAnimNode;
var		CustomCameraAnimPropertiesStruct		CustomCameraAnimProperties;
var		transient	Array<AnimNodeSequence>		ResumePlayNodes;
var		transient	GearAnim_UpperBodyIKHack	IKHackNode;
var		transient	GearAnim_BlendAnimsByAim	BlendByAimNode;
var		transient	GearAnim_BaseBlendNode		BaseBlendNode;
var		transient	GearAnim_BlendList			BlendByAimToggle;

/** MorphNodeWeight for meatshield */
var		transient	MorphNodeWeight				MeatShieldMorphNodeWeight;
var					FLOAT						MeatShieldMorphTimeToGo;
var					FLOAT						MeatShieldMorphBlendTime;
var					bool						bActivateMeatShieldMorph;
var					Name						MeatShieldMorphTargetName;
/**
 * Fix weapon position when in "downsights" animation, so character appears to be aiming.
 * This can be per character, per weapon, as needed.
 */
var transient	vector WeaponAimIKPositionFix;

/** If TRUE, IK Status should be locked */
var bool		bLockIKStatus;

/** camera bone motion scale */
var float	CameraBoneMotionScale;

/** A Body stance variable */
struct native BodyStance
{
	var() Array<Name>	AnimName;
};

var bool bUsesBodyStances;

/** Body Stance used by Kismet Action GearPlayerAnim */
var BodyStance						KismetBodyStance;
/** AnimSets set by Kismet */
var repnotify AnimSet KismetAnimSets[2];
/** Body Stance used for speech gestures. */
var protected transient BodyStance	SpeechGestureBodyStance;

/** bodystance for when pawn is riding junker*/
var BodyStance	BS_Junker_Driving_Idle;
var BodyStance	BS_Junker_Passenger_A_Idle;
var BodyStance	BS_Junker_Passenger_B_Idle;
/** Back up of AnimSets used by Matinee Anim Control Tracks */
var transient	Array<AnimSet>	MATAnimSets;

// Animation state variables

/** TRUE if doing a transition/blend between CoverAction animations. */
var	const	transient	bool			bDoingCoverActionAnimTransition;
var const	transient	INT				CoverActionAnimUpdateTickTag;
/** Last Cover Action set by animations. */
var const	transient	ECoverAction	LastCoverActionAnim, CoverActionAnim;

/** AimOffset interpolation speed */
var()	config	float		AimOffsetInterpSpeedAI;
var()	config	float		AimOffsetInterpSpeedHuman;
var()	config	float		AimOffsetInterpSpeedHeadTrack;

// Interpolate aimoffset to safe range while doing reloads to prevent odd postures
// and ik over extension.
var bool	bReloadingAimInterp;
var float	ReloadingAimInterpTimeToGo;

/** vars for controlling headtrack aimoffset interp. */
var protected transient rotator LastHeadTrackDeltaRot;
var protected transient rotator LastHeadTrackRunDeltaRot;
var protected transient bool	bDoingHeadTrackAimOffsetInterpolation;
var protected transient bool	bDoingHeadTrackRunOffsetInterpolation;

var transient float HeadTrackTime;
var config	  float HeadTrackDuration;
var transient float HeadTrackRunInterpTime;
var config	  float	HeadTrackRunInterpDuration;

// below variables are used blending between aimoffset and head tracking
// this is to handle in/out or between
var config		float HeadTrackBlendSpeed;
// this is when no aim/aim during transition or no cover to cover
var config		float HeadTrackBlendSpeedModiferWhenSwitch;

// current blend speed
var protected transient float HeadTrackCurrentBlendSpeed;
// current weight to target
var protected transient	float HeadTrackBlendWeightToTarget;
// target weight (0 : only aimoffset and 1 : only head tracking)
var protected transient	float HeadTrackBlendTargetWeight;
var protected transient bool  WasHeadTracking;

/**
 * 2d vector, representing a rotation offset percentage from actor's rotation. Used by Aim anim nodes.
 * Range used (-1,-1) to (+1,+1) (angle equivalent : (-90d,-90d) to (+90d,+90d))
 */
var() vector2d	AimOffsetPct;

/** This is the replicated version of AimOffsetPct that other clients will interpolate to
 * compressed: upper 16 bits holds Yaw, lower 16 holds Pitch
 */
var int			ReplicatedAimOffsetPct;
/** Offset used to offset some animations. For example chainsaw dueling on slopes, to play an animation based on height differences. */
var vector2d	PositionAdjustAimOffsetPct;

/**
 * whether or not we are switching weapons.	 this is used as yet another check to see if we can fire our weapon
 *
 * @see GearPC.CanFireWeapon()
 * @see PlayWeaponSwitch()
 * @see TurnOffWeaponSwitchingFlag()
 **/
var bool bSwitchingWeapons, bEquipTransition;

/**
 * FOV used by Pawn to acquire cover.
 * Represents (1.f - dot product) between direction pawn is searching, and direction from Pawn to Cover.
 * 0.f = 0d FOV, 1.f = 180d FOV.
 */
var config float PawnAcquireCoverFOV;
var config float PawnAcquireCoverFOVRoadieRun;

/**
 * FOV relative to Cover to see if Pawn can acquire it.
 * Represents (1.f - dot product) between cover normal direction, and direction from Cover to Pawn..
 * 0.f = 0d FOV, 1.f = 180d FOV
 */
var config float CoverAcquireFOV;
/** Same as above, but when roadie running */
var config float RoadieRunCoverAcquireFOV;

/** Scales Pawn cylinder size, used for cover slot snapping */
var config float CoverSnapScale;

/** Currently acquired cover link */
var CoverLink CurrentLink;

/** Last result from GuessAtCover() */
var CoverInfo CachedCoverGuess;
var float LastCoverGuessTime;

/** bool to track if pawn was in cover, and trigger physics position correction code */
var bool	bWasInCover;

/** Index to current slot being used in CurrentLink */
var int CurrentSlotIdx;
/** Index to the closest slot being used by this pawn */
var int ClosestSlotIdx;
/** Old current slot idx when leave cover - used for swat turns */
var int PreviousSlotIdx;
/** AI is adjusting to this slot */
var CoverSlotMarker	TargetSlotMarker;

/** Current left/right slots, for interpolation (see AGearPawn::calcVelocity) */
var int LeftSlotIdx, RightSlotIdx;

/** Current interpolation pct between LeftSlotIdx & RightSlotIdx (see AGearPawn::calcVelocity) */
var float CurrentSlotPct;

/** Represents the direction we are traveling between cover slots */
var ECoverDirection CurrentSlotDirection;
/** Current cover lean direction for the camera, independent of the cover action */
var transient ECoverDirection CoverDirection;

/** Current cover lean direction sent from client when entering cover (used in FindBestCoverSideFor()) */
var transient ECoverDirection ReplicatedCoverDirection;

/** Make sure ReplicatedCoverDirection kept current */
var float FreshCoverDirectionTime;

/** Is player in stationary cover */
var bool bIsInStationaryCover;

/** Pawn is in a lit area - used for Kyrll interaction */
var bool	bInLitArea;
/** Pawn already has a kyrll controller associated with him */
var bool	bKyrllEnabled;

/** This is the range that Pawns can MeleeAttack in Unarmed. */
var() config float UnarmedMeleeRange;
/** used for replication of melee attacks */
var bool bDoingMeleeAttack;

/** Controller wants to do a melee attack */
var				bool bWantsToMelee;

/** This is what character footstep type this pawn is **/
var ECharacterFootStepType CharacterFootStepType;
var bool bShouldPlayWetFootSteps;

/** Last time this pawn was shot at. (Includes near misses) */
var transient			FLOAT			LastShotAtTime;
var transient repnotify	BYTE			ShotAtCount;

var transient			FLOAT			LastCringeTime;
var transient repnotify	BYTE			CringeCount;

/**
 * Per DamageType cringe history.
 * This is so we can differentiate rapid fire weapons from rare ones like grenade explosions.
 * And do per weapon filtering.
 */
struct native CringeInfo
{
	var class<DamageType>	DamageType;
	var Array<FLOAT>		History;
	var FLOAT				NextCringeTime;
};
var	transient Array<CringeInfo>	NearHitHistory;
var transient Array<FLOAT>		CringeHistory;

/** Last time this pawn heard a bulletwhip sound */
var transient float LastBulletWhipTime;

/** Last time this pawn took damage **/
var transient float LastTookDamageTime;
var transient float LastPlayedDamageSound;

/** Last time this pawn fired a shot.  Recorded at BEGINNING of shot. */
var transient float LastWeaponStartFireTime;

/** Last time this pawn finished firing a weapon.  Recorded at END of shot. */
var transient float LastWeaponEndFireTime;

/** Last time this pawn blindfired their weapon. */
var transient float LastWeaponBlindFireTime;

/** Last time pawn evaded - used by AI */
var transient float LastEvadeTime;
/** Last time pawn evaded into a wall - used for particles*/
var transient float LastEvadeIntoWallTime;

/** Min amount of time between AI evades */
var config	  float	MinTimeBetweenEvades;

/**
 * This is the Percent of GroundSpeed at which a pawn moves when going backwards.
 * So if you want a 20% reduction you would set this value to 0.80
 **/
var() config float BackwardMovementSpeedPercentage;

/** This is the Percent of GroundSpeed at which a pawn moves when carrying a heavy weapon. */
var() config float HeavyWeaponMovementSpeedPercentage;

/** This is the Percent of GroundSpeed at which a pawn moves when carrying a heavy weapon and firing. */
var() config float HeavyWeaponFiringMovementSpeedPercentage;

/** This is the Percent of GroundSpeed at which a pawn moves when carrying a shield. */
var() config float ShieldMovementSpeedPercentage;

/** This is the Percent of GroundSpeed at which a pawn moves when carrying a shield and firing. */
var() config float ShieldFiringMovementSpeedPercentage;

/** Time last cover slipped, used for a temp speed boost for roadie run from slip. */
var transient float RoadieRunBoostTime;

/** Current boost modifier for crawling speed. */
var repnotify transient float CrawlSpeedModifier;
var repnotify transient bool bRaiseHandWhileCrawling;

/** The time we last did a melee attack **/
var transient float LastMeleeAttackTime;

/** The effect to play when impacted by a melee hit **/
var ParticleSystem PS_MeleeImpactEffect;

/** The effect to play when impacted by a radial explosive damage hit **/
var ParticleSystem PS_RadialDamageEffect;

/** CoverAction used when mounting heavy weapon (on top of cover, leaning) */
var transient ECoverAction					HeavyMountedCoverAction;
/* CoverType used when mounting heavy weapon */
var transient ECoverType					HeavyMountedCoverType;

/** Cached Array of recoil nodes */
var transient Array<GearSkelCtrl_Recoil>	RecoilNodes;
var transient SkelControlLookAt				HeadControl;

/** Test node for troika */
var	transient GearAnim_BlendList		TroikaBlendNode;

/** Cached mirror node for this character */
var transient GearAnim_Mirror_Master	MirrorNode;
/** DBNO Guard Blend Node */
var transient AnimNodeBlend				DBNOGuardBlendNode;
/** DBNO Crawl node */
var transient AnimNodeSequence			DBNOCrawlNode;
/** Root AnimTree node */
var	transient AnimTree					AnimTreeRootNode;
/** Used to mirror dom when carrying crate */
var() transient AnimNodeMirror			CrateMirrorNode;
var() transient SkelControlLimb			CrateIKRightHand, CrateIKLeftHand;
var() transient	GearAnim_BlendAnimsByDirection	CrateWalkNode;

// IK Controllers
var() editinline editconst transient SkelControlLimb		IKCtrl_RightHand, IKCtrl_LeftHand, IKBoneCtrl_RightHand, IKBoneCtrl_LeftHand;
var() editinline editconst transient SkelControlSingleBone	IKRotCtrl_RightHand, IKRotCtrl_LeftHand;

/** If TRUE, Pawn can do turn in place animations */
var	bool	bCanDoTurnInPlaceAnim;

/** An aim attractor is a point that the player autoaim code will attempt to target. */
struct native AimAttractor
{
	/** Attraction is scaled from 0 at this radius to 100 at the inner radius. */
	var() float		OuterRadius;

	/** Inside this radius, attraction is 100%. */
	var() float		InnerRadius;

	/** Bone to attach to */
	var() Name		BoneName;
};

/** array of aim attractors attached to this pawn */
var()	Array<AimAttractor>	AimAttractors;
var()	globalconfig	bool bDebugAimAttractors;


//@fixme - move these properties to the special move classes
/** camera shake to play when hitting the wall during a run2cover move */
var() ScreenShakeStruct	Run2CoverCameraShake;

/** Distance threshold for minimum camera shake (scale = 0.f) */
var() float Run2CoverMinCameraShakeDistSqThreshold;
/** Distance threshold for maximum camera shake (scale = 1.f) */
var() float Run2CoverMaxCameraShakeDistSqThreshold;

/** Is this pawn in the process of spawning? (ex. crawling out a hole) */
var bool	bSpawning;


/** Which GUDData-based class I should be using for...umm.. GUD data. */
var array<string>				MasterGUDBankClassNames;

/** Index of the GUDBankClass currently being used by this pawn. */
var transient int				LoadedGUDBank;
/** Container object for currently-loaded GUDS data. */
var transient GUDData			LoadedGUDSData;

/** TRUE if this pawn cannot speak GUDS lines temporarily. */
var protected transient bool	bMuteGUDS;

/** Is this pawn in the process of speaking a line? */
var transient bool				bSpeaking;
/** True to turn on speech debug logging. */
var() protected bool			bDebugSpeech;
/** priority of actively speaking line, used to determine if we should interrupt  */
var protected transient ESpeechPriority	CurrentSpeechPriority;

/** Data that describes a spoken line of dialogue. */
struct native SpeakLineParamStruct
{
	var GearPawn					Addressee;
	var	SoundCue					Audio;
	var String						DebugText;
	var bool						bNoHeadTrack;
	var ESpeakLineBroadcastFilter	BroadcastFilter;
	var bool						bSuppressSubtitle;
	var ESpeechPriority				Priority;
	/** how long to wait until playing the sound */
	var float						DelayTime;
	/** how long after line is finished to keep looking at addressee */
	var float						ExtraHeadTrackTime;
};

/** Spoken line description used to broadcast speech to all clients. */
var protected repnotify SpeakLineParamStruct	ReplicatedSpeakLineParams;
/** spoken line description that's next in the queue */
var protected SpeakLineParamStruct QueuedSpeakLineParams;
/** Description of currently playing spoken line. */
var SpeakLineParamStruct						CurrentSpeakLineParams;

/** handle to audio for speech that is currently playing */
var AudioComponent								CurrentlySpeakingLine;

var float										SpeechPitchMultiplier;

/** AudioComponent used by FaceFX */
var	protected AudioComponent				FacialAudioComp;

/** Pawn Rotation Interpolation between lock/unlock, and cover in/out transitions */
var()	float	PawnRotationInterpolationTime;
/** Counter for the above */
var		float	PawnRotationBlendTimeToGo;
/** Special Interpolation mode for cover transition, Doesn't take shortest path for rotation interp. */
var		bool	bRotInterpCoverTransition;
var		Rotator	LastCoverRotation;

var protected transient repnotify TakeHitInfo LastTakeHitInfo;
/** LastTakeHitInfo will be checked for replication until this time is reached (so new clients don't get random old effects, etc) */
var protected transient float LastTakeHitTimeout;
var FLOAT		NextHitReactionAnimTime;
/** server only flag set when we should go DBNO at the end of the current special move */
var bool bDelayedDBNO;

/** Is this pawn capable of being knocked DBNO from damage? */
var bool bCanDBNO;

/** Is this pawn capable of being revived from DBNO? */
var bool bCanRecoverFromDBNO;

/** Whether pawn responds to explosions or not (ie knocked down from mortar) */
var bool bRespondToExplosions;

/** If TRUE, vehicle do damage when hitting this GearPawn */
var bool bCanBeRunOver;

/** Icon used to draw our head in taccom */
var CanvasIcon HeadIcon;

/**
 * handles to the audiocomponents for revive sounds actively playing.
 * gotta store here since we can't have locals in latent state code where this is used
 */
var AudioComponent ReviveBreathSound;
var AudioComponent FlatLiningSound;
var AudioComponent ReviveHeartbeatSound;
var AudioComponent NearDeathBreathSound;

/** Do we allow inventory drops on death? */
var bool bAllowInventoryDrops;

/** Should this AI allow hit impact decals on its skel mesh**/
var() bool bAllowHitImpactDecalsOnSkelMesh;

/** Should this AI spawn hit effect decals **/
var() bool bSpawnHitEffectDecals;

/** Should this AI spawn blood trail decals **/
var() bool bSpawnBloodTrailDecals;

/** This is to cap the number of "constrain broke" effects we spawn per guy. **/
var() int NumConstraintEffectsSpawnedMax;
var transient int NumConstraintEffectsSpawned;

/** Whether or not this pawn should play the death scream. (i.e. aerial combat or mobs that are far away don't need this on at all)**/
var() bool bDisableDeathScreams;

/** Sound of last pickup */
//@fixme - looks like this is being set, but PlayPickupSound isn't actually called anymore
var SoundCue AmmoPickupSound;

/** True if I am in "using commlink" mode, false otherwise.	 This is true during the transition animations */
var bool bUsingCommLink;
/** True if I want to be in using commlink mode, but I haven't made it there yet */
var repnotify bool bWantToUseCommLink;
/** True if I have fully entered the using-commlink body stance.  False during transition animations */
var bool bInCommLinkStance;

/** True if I am in conversation mode */
var bool bIsConversing;
/** True is I want to be in conversing state, but I haven't made it there yet */
var repnotify bool bWantToConverse;

/** The reviving blood pool **/
/** TimeSeconds of when the pawn went DBNO */
var				bool					bWasDBNO;
var				float					TimeOfDBNO;
var				byte					DBNOTimeExtensionCnt;
var				Controller				ControllerWhoPutMeDBNO;
var				class<GearDamageType>	DamageTypeThatPutMeDBNO;
var				float					TimeStampEnteredRevivingState;
var repnotify	byte					bSpawnABloodPool;

/** This is the Particle system that is played on the pawn when they are DBNO **/
var ParticleSystem			DBNO_BloodCrawlSpurtTemplate;

/** Was the last hit considered a head shot?  Used to see if we need to pop off helmet/head */
var bool bLastHitWasHeadShot;

/** Whether or not to show the fatal death blood effects **/
var bool bShowFatalDeathBlood;

/**
 * If the pawn died by bleeding out.  We need to store this state as we call up into Pawn and then
 * back down into the GearPawn for death events.
 **/
var bool bBledOut;

/** When DBNO, beyond this health threshold the Pawn dies. */
var(MeatShield) config		INT			DBNODeathThreshold;
var(MeatShield) config		INT			HostageDefaultHealth;
var(MeatShield) config		INT			HostageDeathThresholdByHeadshot;
var	repnotify				INT			HostageHealth;
var							Array<Name>	HostageHealthBuckets;

/**
 * Half Angle in degrees of protection. From 0d to 90d (90d being full front protection).
 * Note, that damage is interpolated with x^2. So with 90d HalfAngleOfProtection, damage would be:
 * Shot from 0d = 0% damage. Shot from 45d = 25% damage. Shot from 90d = 100% damage.
 */
var(MeatShield) config		float	KidnapperHalfAngleOfProtection;

/**
 * Set to TRUE when hostage has been turned into pseudo rag doll for hostage gibbing.
 * But Pawn hasn't died yet.
 */
var							bool			bInMeatShieldRagDoll;
/** List of bodies to unfix when in Meat Shield rag doll. */
var(MeatShield) editinline	Array<Name>		MeatShieldUnfixedBoneList;

/**
 * Time when hostage was released.
 * With splash damage weapons, sometimes the hostage is killed before the kidnapper.
 * So the kidnapper is not considered protected anymore and can die.
 * Use this variable, to keep the kidnapper protected for this frame.
 */
var							float			HostageReleaseTime;

// if this is on GetPhysicalFireStartLoc will return Location + (FireOffset >> Rotation);
var							bool			bUseSimplePhysicalFireStartLoc;

enum EGearDeathType
{
	GDT_NORMAL,
	GDT_HEADSHOT,
};

/**
 * This tells us which tier the active reload chaining is on.
 *
 **/
var byte ActiveReload_CurrTier;
var repnotify bool bActiveReloadBonusActive;

/** Which one of my bones to focus on when playing pickup special move */
var Name PickupFocusBoneName;
/** Which one of my bones to focus on when playing kickup version of the pickup special move */
var Name PickupFocusBoneNameKickup;

/** which GUD event to throw when I need reviving and have been waiting a bit.	set to None to get the generic non-pawn-specific version */
var() protected EGUDEventID NeedsRevivedGUDSEvent;
/** which GUD event to throw when I go "down but not out".	set to None to get the generic non-pawn-specific version */
var() protected EGUDEventID WentDownGUDSEvent;
/** which GUD event to throw when I get spotted. */
var() const EGUDEventID NoticedGUDSEvent;
/** How "threatening" I am.  Used by battle status code to prioritize notice events. */
var() const float NoticedGUDSPriority;
/** True to disregard this pawn for noticed events. */
var() const bool bSuppressNoticedGUDSEvents;

// Point of Interest associated with this warpawn
var GearPointOfInterest	POI;

/** Reference to the wheel trigger the warpawn is in for turning valves */
var Trigger_Engage EngageTrigger;

/** Can this pawn pick up weapons from weapon factories */
var bool bCanPickupFactoryWeapons;

/** Reference to a Ladder trigger */
var Trigger_LadderInteraction	LadderTrigger;

/** Roadie Run particle effects **/
var ParticleSystemComponent PSC_RoadieRun;
var ParticleSystem PS_RoadieRun;
var TraceHitInfo LastFootStepTraceInfo;
var PhysicalMaterial LastFootStepPhysMat;

/** Number of times this pawn has been down but not out in MP. */
var int		DownCount;
/** Max number of times this pawn can be down but not out in MP. */
var config int		MaxDownCount;

/** Damage multiplier for making pawns super troopers (mainly for making AI kill faster) */
var float SuperDamageMultiplier;

/** Time this warpawn revived from death */
var float TimeOfRevival;
/** remembers the team we were on pre-death as our PlayerReplicationInfo will be disconnected */
var byte LastTeamNum;

/** The pawn's light environment */
var() DynamicLightEnvironmentComponent LightEnvironment;

/** looping head-covering bodystance */
var private BodyStance BS_HeadCoverLoop;

/** True if pawn is doing head-cover anims */
var transient repnotify bool bCoveringHead;

/** If true, Pawn may not roadie run. */
var transient bool bCannotRoadieRun;

/** Information about the last gudline pawn has played, to avoid rapid line duplication. */
var transient SoundCue		LastGUDLinePlayed;
var transient float			LastGUDLinePlayedTime;
var const float				GUDLineRepeatMin;
/** Min time that must elapse between any 2 GUDS lines. */
var() protected const float	MinTimeBetweenAnyGUDS;


/* Material used to control the pawn colorization based upon distance */
//var MaterialInstanceConstant PawnColorMatInstance;
var transient MaterialInstanceConstant MPRimShader;
var transient MaterialInstanceConstant MPRimShaderHelmet;

/** General material used to control common pawn material parameters (e.g. burning) */
var protected transient MaterialInstanceConstant MIC_PawnMat;
var protected transient MaterialInstanceConstant MIC_PawnHair;
var protected transient MaterialInstanceConstant MIC_PawnMatHelmet;
var protected transient MaterialInstanceConstant MIC_PawnMatShoulderPadLeft;
var protected transient MaterialInstanceConstant MIC_PawnMatShoulderPadRight;

/** Number of active OwnerNoSee requests */
var private transient int OwnerNoSeeCount;

/** inside this cylinder, camera's using this pawn as a target will turn off owner rendering for this pawn */
var() protected cylinder CameraNoRenderCylinder_High;
var() protected cylinder CameraNoRenderCylinder_Low;
var() protected cylinder CameraNoRenderCylinder_High_ViewTarget;
var() protected cylinder CameraNoRenderCylinder_Low_ViewTarget;
var() protected cylinder CameraNoRenderCylinder_FlickerBuffer;

/** Effect to play when the HOD burns someone **/
var()	ParticleSystem				PS_BurnEffect;

/** component for currently playing Burn effect */
var ParticleSystemComponent			BurnEffectPSC;

/** TRUE if this warpawn should block the camera, FALSE otherwise */
var() bool							bBlockCamera;
/** Whether the warpawn is not being rendered because the camera is inside of it */
var bool							bIsHiddenByCamera;

/** standard head sphere radius for multiplayer */
var float MPHeadRadius;

/** standard head center offset from head bone for multiplayer */
var(head) vector MPHeadOffset;


/**
 * SP FaceFXAnimSet which has all of the efforts for this guy.	We link them up at run time
 * so we can maintain nice encapsulated AnimSets on the content side.  Most characters have this
 * but there are  a few exceptions.	 Those will probably be fixed up soonish.
 **/
var array<FaceFXAnimSet> FAS_Efforts;  // always include the efforts for this guy

/**
 * This is the set of FaceFX Animsets that should be looked at by this pawn for dynamic GUDs package creation at cook time
 */
var array<string> FAS_ChatterNames;


/** SP FaceFXAnimSet which has all of the SP chatter **/
//var FaceFXAnimSet FAS_SinglePlayer;

/** SP FaceFXAnimSet which has all of the SP chatter **/
//var FaceFXAnimSet FAS_MultiPlayer;

/** Number of times the needs revived reminder has triggered. */
var() int NeedsRevivedReminderCount;

/** The amount of time which must pass with this pawn's remains not being seen before we destroy it **/
var float DurationBeforeDestroyingDueToNotBeingSeen;

/** Cylinder to use for view friction when pawn is crouched. */
var() cylinder CrouchedFrictionCylinder;

/** The distance which to trace out from the muzzle socket to see if we are attempting to shoot through wall **/
var float ShootingThroughWallTraceDistance;

/** Vars for doing special cam lookats during death cam mode */
var transient bool bOverrideDeathCamLookat;
var transient GearPawn DeathCamLookatPawn;
var transient Name DeathCamLookatBoneName;

/**
 * This is the location of the e-hole where this mob emerged from.	We have z fighting / rendering issues
 * with the bloodpools and the skeletal mesh portions of the e-hole.  So when we die we are going to check
 * that we are a certain distance away from the ehole before spawning a blood pool.	  No blood is better
 * than the terrible visual anomalies.
 **/
var vector LocationOfEholeEmergedFrom;
var config float SafeDistanceFromEholeToSpawnBloodPool;

struct native PhysicsImpulseInfo
{
	/** Linear velocity to apply */
	var vector LinearVelocity;
	/** Angular velocity to apply */
	var vector AngularVelocity;
};
var repnotify PhysicsImpulseInfo KnockdownImpulse;
var vector KnockdownStartingPosition;
var bool bCheckKnockdownFall;

/** Anim to play when knocked down */
var BodyStance BS_KnockDownAnim;
/** Whether to play animtion through physics motors when knocked down */
var	bool	bPlayMotorAnimOnKnockDown;
/** Strength of motors to use for knock-down anim */
var()	float	KnockDownMotorStrength;
/** Damping of motors to use for knock-down anim */
var()	float	KnockDownMotorDamping;
/** How motor strength varies over time after knockdown ('time since knockdown' is input, 'motor scale' is output) */
var()	InterpCurveFloat	KnockDownMotorScale;
/** Time (WorlInfo.TimeSeconds) that we were knocked down */
var		float	KnockDownStartTime;
/** AnimTree to switch to when recovering from rag doll */
var		AnimTree	RecoverFromRagdollAnimTree;

/**
* Range [0..1] to represent the elevation % for the mortar heavy weapon.  Put on the pawn so it replicates,
* since weapons themselves aren't synced.  Seems like there should be a cleaner way to do this,
* but hooray for prototyping.
*/
var transient float MortarElevationPct;

/** Protection range when holding the shield.  Damage is 0% directly in front, and 100% at this angle. */
var protected const config	float	ShieldHalfAngleOfProtection;
/** Protection range when deploying the shield.  Damage is 0% directly in front, and 100% at this angle. */
var protected const config	float	DeployedShieldHalfAngleOfProtection;

/** Currently equipped shield. */
var		GearShield					EquippedShield;

enum PawnOnFireState
{
	POF_None,
	POF_Blazing,
	POF_Smoldering,
};

var protected transient PawnOnFireState				OnFireState;
var protected transient repnotify PawnOnFireState	ReplicatedOnFireState;

/** True if this pawn is on fire, false otherwise. */
var protected transient bool			bOnFire;

/** Particle system to use for on-fire effects. */
var() protected const ParticleSystem	OnFireBlazingParticleSystem;
/** Particle system to use for on-fire effects. */
var() protected const ParticleSystem	OnFireSmolderParticleSystem;
/** ParticleSystemComponent for active on-fire effects. */
var protected ParticleSystemComponent	PSC_OnFire;

/** Audio sample to use for the looping burning sound. */
var() protected const SoundCue			OnFireBlazingBurningSound;
/** Audio sample to use for the looping burning sound. */
var() protected const SoundCue			OnFireSmolderingBurningSound;
/** AudioComponent for active on-fire looping sound. */
var protected AudioComponent			OnFireAudioBurningLoop;

/** Particle system to use for Imulsion damage effects. */
var() protected const ParticleSystem	ImulsionParticleSystem;
/** ParticleSystemComponent for active imulsdion effects. */
var protected ParticleSystemComponent	PSC_Imulsion;

/** Audio sample to use for the looping imulsion sound. */
var() protected const SoundCue			ImulsionBurningSound;
/** AudioComponent for active imulstion looping sound. */
var protected AudioComponent			ImulsionAudioBurningLoop;

/** Emitter that plays when pawn takes a certain amount of flamethrower damage. */
var transient SpawnedGearEmitter		DamagedByFlamethrowerEmitter;

/** Should we enable debug damage text for tracking? */
var globalconfig bool bDebugDamageText;

/** This is the time that we were last in hail, whether or not we actually got damage from it. */
var transient float LastAttemptedHailDamageTime;

/** Grapple rope that we are holding onto */
var transient protected GrappleRopeBase AttachedGrappleRope;

/** Location of the last blood trail decal.  Needed to orient the decal correctly**/
var protected transient vector	LocationOfLastBloodTrail;

/** The max amount of decals that I am allowed to have on this pawn.**/
const MAX_DECALS_ATTACHED = 3;
var int NumDecalsAttachedCurr;
var int CurrDecalIdx;
/** This is times of when decals were attached to this pawn.  **/
var float DecalsAttached[MAX_DECALS_ATTACHED];


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
var() protected const float		SkinHeatFadeDelay;

/** Skin "charring" data. */
var protected transient float	CurrentSkinChar;
/** Min heat value we can go down to.  Useful for when you died by flames and want to leave the pawn "charred a bit" **/
var transient float				CurrentSkinCharMin;

var() protected float			SkinCharFadeTime;

/** Similar to God mode, in that you never die, but you still take damage and get the effects and whatnot. */
var() bool						bUnlimitedHealth;

/** List of camera volumes we're currently inside. Only element 0 will be considered by the camera, but this handles overlapping volumes. */
var transient array<CameraVolume>	CameraVolumes;

/** Currently targeted GDO used for administering damage */
var transient GearDestructibleObject TargetedGDO;

/** class we have been mutated into visually by the Mutate Character Kismet action (@see OnMutate()) */
var repnotify class<GearPawn> MutatedClass;

/** Carried crate pawn */
var repnotify GearPawn_CarryCrate_Base	CarriedCrate;
/** list of bones that should be FIXED for bloodmount death */
var(BloodMountDeath) editinline	Array<Name>		BloodMountFixedBoneList;
/** turned on if we died while mounted on a bloodmount */
var GearPawn_LocustBloodmount BloodMountIDiedOn;

/** movement smoothing parameters */
/** three points of turn smoothing spline (set from movetogoal) */
var transient Actor FinalSplineActor,CurrentSplineActor,PrevSplineActor;
var transient BasedPosition FinalSplinePt; // used when we are not moving to an actor at final (like when we're moving to a set point and we're at the end of our path)
/** the (maximum) turning radius of this pawn when accel smoothing is on */
var() float TurningRadius;
/** the current turning radius we are using (based on TurningRadisu above, and scaled depending on how close to the goal we are) */
var transient float EffectiveTurningRadius;
/** whether or not to allow acceleration smoothing (should we enforce the above turning radius?)*/
var() bool bAllowAccelSmoothing;
/** Acceleration last frame */
var transient vector OldAcceleration;
/** whether to allow spline turn cornering */
var() bool bAllowTurnSmoothing;
/** accel smoothing will be ramped down within this distance */
var() float AccelConvergeFalloffDistance;
/** internal variable used for turn smoothing */
var transient float SmoothTurnStartT;
/** end movement smoothing params **/

/** GUID for checkpoint saved GearPawns so that references to them can be saved even though they will be recreated */
var Guid MyGuid;
/** percent chance that we will try and go DBNO */
var() config float LocustChanceToDBNO;

/** Actor we are forced to be based on unless teleported. This is a hack to keep the Pawn on certain movers (e.g. Derrick)
 * that the physics will occasionally insist we should detach from.
 */
var Actor ClampedBase;

/** Dueling mini game in progress */
var			bool		bInDuelingMiniGame;
/** Number of Button Presses done while chainsaw dueling. */
var			INT			DuelingMiniGameButtonPresses;
/** Duration of Chainsaw dueling mini game. */
var config	FLOAT		ChainsawDuelMiniGameDuration;
/** Angle in Radians between both players to start a chainsaw dueling. Basically telling if they're facing each other or not. */
var config	FLOAT		ChainsawDuelFacingDegreesAngle;
/** number of times AI is considered to have pressed the button during the dueling minigame */
var config	vector2d	AI_ChainsawDuelButtonPresses;

/** Is in Trigger MiniGame */
var			bool		bInTriggerMiniGame;
/** Last Trigger pressed  is right trigger. */
var			bool		bLastTriggerPressedIsRight;

/** Amount of interrupt before disabling chainsaw usage */
var const config int ChainsawInterruptThreshold;
/** Amount of decay per second for the interrupt amount */
var const config int ChainsawInterruptDecayPerSecond;
/** Current interrupt amount, if >= ChainsawInterruptThreshold then chainsaw is disabled */
var transient int ChainsawInterruptAmount;

// Stopping Power

struct native StoppingPowerStruct
{
	/** Direction of the Hit */
	var()	Vector	Direction;
	/** Stopping power */
	var()	FLOAT	StoppingPower;
	/** Life Time of Stopping Power */
	var()	FLOAT	LifeTime;
};

/** Array of currently applied Stopping Powers */
var			Array<StoppingPowerStruct>	StoppingPowerList;
/** Max Stopping Power that can be applied */
var config	float						MaxStoppingPower;
var config  float                       MaxStoppingPowerDistance;
var const config float					StoppingPowerAngle;
var const config float					StoppingPowerHeavyScale;
/** Acceleration for recovery speed for stopping power */
var config	float						StoppingPowerRecoverySpeed;
/** Accumulated Stopping Power Threshold when roadie running to trigger a stumble */
var config	float						StoppingPowerRoadieRunStumbleThreshold;
/** Turns on debugging for stopping power */
var config	bool						bDebugStoppingPower;
/** whether to do stopping power at all for this pawn */
var config	bool						bAllowStoppingPower;

/** Current Marker for Grappling Hook */
var Gear_GrappleHookMarker				CurrentGrapplingMarker;
/** Force animations to use velocity from Base */
var bool								bAnimForceVelocityFromBase;

/** Walkspeed when doing a "forcewalk", e.g. commlink action, camera volume. */
var() protected const config float		ForceWalkingPct;

// kantus revive skin effect mojo
var() config float						KantusSkinEffectFadeDuration;
var transient float						DesiredKantusFadeVal;
var transient float						CurrentKantusFadeVal;
var ParticleSystemComponent				KantusRevivePSC;
var GearPawn_LocustkantusBase			KantusReviver;



// cache packet for GetWeaponStartTraceLocation, bUpToDate toggled off at the end of each tick
var struct native WeaponStartTraceLocationCacheStruct
{
	var bool bUpToDate;
	var vector StartLoc;
	var Weapon WeaponCacheIsValidFor;
} WeaponStartTraceLocationCache;

var config array<PerDamageTypeMod> PerDamageTypeModifiers;

var repnotify bool bDisableShadows;

// location of the start of our last mantle
var vector LastMantleLocation;
// worldtime when we mantled last
var float  LastmantleTime;

/** TRUE if Pawn is fully blended into an aiming animation (ready or targeting, i.e. not relaxed or blending from relaxed).  NON-COVER. */
var transient	bool	bTargetingNodeIsInIdleChannel;
var transient	INT		TargetingNodeIsInIdleChannelTickTag;

// whether or not we should allow ourselves to be revived from automatic revivals (like at the end of combat)
var transient	bool	bAllowAutoRevive;

/** If true, do not emit a GUDS event for this pawn's death. */
var protected const bool	bNoDeathGUDSEvent;

/** replicated copy of material in slot 0 */
var repnotify MaterialInterface ReplicatedMaterial;
/** value for 'BloodOpacity' parameter on material in slot 0 */
var repnotify float BloodOpacity;

// don't allow anchor checks when in these physmodes
var array<EPhysics> ProhibitedFindAnchorPhysicsModes;

/** list of FaceFXAnimSets applied to this Pawn for replication to the clients */
var repnotify FaceFXAnimSet ReplicatedFaceFXAnimSets[3];
/** previously applied ReplicatedFaceFXAnimSets as we need to diff to know what to change locally */
var FaceFXAnimSet PrevReplicatedFaceFXAnimSets[3];

/** vars to deal with moving bases encoraching on based pawns */
var protected transient bool bBaseIsEncroaching;
var protected transient vector BaseEncroachInvNormal;
var protected transient vector BaseEncroachHitLocation;

/** Stats Mask Defines */

const STATS_LEVEL1	= 0x01;
const STATS_LEVEL4	= 0x08;
const STATS_LEVEL5	= 0x10;


cpptext
{
	UBOOL ShouldApplyNudge(ACoverLink *Link, INT SlotIdx, UBOOL bIgnoreCurrentCoverAction = FALSE);

	virtual void	TickSpecial(FLOAT DeltaSeconds);

	virtual FLOAT GetGravityZ();

	virtual FGuid* GetGuid();

	/** smooths out player movement on steps/slopes, primarily by adding mesh translation to spread out the Z change over time
	 * @param OldLocation - the Pawn's previous Location to compare against for adjustments
	 */
	void PerformStepsSmoothing(const FVector& OldLocation, FLOAT DeltaSeconds);

	/** Update the mesh component to make it match the Floor surface normal better */
	void UpdateFloorConform(FLOAT DeltaSeconds);

	virtual FLOAT MaxSpeedModifier();
	FVector CheckForLedges(FVector AccelDir, FVector Delta, FVector GravDir, int &bCheckedFall, int &bMustJump );
	virtual void CalcVelocity(FVector &AccelDir, FLOAT DeltaTime, FLOAT MaxSpeed, FLOAT Friction, INT bFluid, INT bBrake, INT bBuoyant);
	virtual void performPhysics(FLOAT DeltaSeconds);
	virtual void PostProcessPhysics( FLOAT DeltaSeconds, const FVector& OldVelocity );
	// called when physics == PHYS_RigidBody
	virtual void PostProcessRBPhysics(FLOAT DeltaSeconds, const FVector& OldVelocity);

	virtual void SetPostLandedPhysics(AActor *HitActor, FVector HitNormal);
	virtual void physicsRotation(FLOAT DeltaTime, FVector OldVelocity);
	virtual void TickSimulated(FLOAT DeltaSeconds);
	virtual void HandleSerpentineMovement(FVector& out_Direction, FLOAT Distance, const FVector& Dest ) { }
	virtual void stepUp(const FVector& GravDir, const FVector& DesiredDir, const FVector& Delta, FCheckResult &Hit);
	virtual UBOOL ResolveAttachedMoveEncroachment(AActor* EncroachedBase, const FCheckResult& OverlapHit);
	virtual void SyncActorToRBPhysics();

	virtual void	SetAnchor( ANavigationPoint* NewAnchor );
	virtual INT		ModifyCostForReachSpec( UReachSpec* Spec, INT Cost );
	virtual void	InitForPathfinding( AActor* Goal, ANavigationPoint* EndAnchor );

	/** notification when actor has bumped against the level */
	virtual void NotifyBumpLevel(const FVector &HitLocation, const FVector& HitNormal);

	// Never try to jump over a wall
	virtual UBOOL TryJumpUp(FVector Dir, FVector Destination, DWORD TraceFlags, UBOOL bNoVisibility) { return FALSE; }

	// Support for Matinee Anim Control Tracks
	virtual void GetAnimControlSlotDesc(TArray<struct FAnimSlotDesc>& OutSlotDescs);
	virtual void PreviewBeginAnimControl(TArray<class UAnimSet*>& InAnimSets);
	virtual void PreviewSetAnimPosition(FName SlotName, INT ChannelIndex, FName InAnimSeqName, FLOAT InPosition, UBOOL bLooping);
	virtual void PreviewSetAnimWeights(TArray<FAnimSlotInfo>& SlotInfos);
	virtual void PreviewFinishAnimControl();

	virtual void OnRigidBodyCollision(const FRigidBodyCollisionInfo& MyInfo, const FRigidBodyCollisionInfo& OtherInfo, const FCollisionImpactData& RigidCollisionData);

	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );

	UBOOL GetMoveDelta(FVector& out_Delta, const FVector& Dest);
	UBOOL moveToward(const FVector &Dest, AActor *GoalActor );

	void UpdateCoverActionAnimTransition(INT InTickTag, UBOOL bInDoingTransition);
	void UpdateTargetingNodeIsInIdleChannel(INT InTickTag, UBOOL bInAimingStance);

	/** Called each from while the Matinee action is running, to set the animation weights for the actor. */
	virtual void SetAnimWeights( const TArray<struct FAnimSlotInfo>& SlotInfos );

	virtual void PreNetReceive();
	virtual void PostNetReceive();
	virtual void PostNetReceiveLocation();
	virtual void PostNetReceiveBase(AActor* NewBase);

	virtual UBOOL IsAliveAndWell() const;

	virtual UBOOL	IsValidAnchor( ANavigationPoint* AnchorCandidate );

private:
	void HeadTrackInterp( const FRotator & AimDeltaRot, const FRotator & HeadTrackDeltaRot, FRotator & OutDeltaRot );
	void UpdateHeadTrackInterp( const FLOAT & DeltaTime );
}

replication
{
	// Replicated to ALL but Owning Player
	if ((!bNetOwner || bDemoRecording) && Role == ROLE_Authority)
		CoverType, CoverAction, AcquiredCoverInfo, bWantsToBeMirrored, bIsTargeting, bIsInStationaryCover, ReplicatedAimOffsetPct,
		ReplicatedSpecialMoveStruct, bIsZoomed, CrawlSpeedModifier, bRaiseHandWhileCrawling, CurrentSlotDirection;

	// replicatd to all but owning player unless dead
	if ((!bNetOwner || bDemoRecording) && !bTearOff && Role == ROLE_Authority)
		RemoteWeaponRepInfo, AttachSlotClass_Holster, AttachSlotClass_Belt, AttachSlotClass_LeftShoulder, AttachSlotClass_RightShoulder;

	// Replicated to ALL
	if( Role == Role_Authority )
		AttachClass_Shield, bIsInCombat, bWantsToMelee, ReplicatedForcedAimTime, bSpawnABloodPool,
		bChargingBow, bWantToConverse, bWantToUseCommLink, ReplicatedSpeakLineParams,
		SpecialMoveLocation, bActiveReloadBonusActive,
		bCannotRoadieRun, bCoveringHead, EngageTrigger, LadderTrigger,
		MortarElevationPct, ReplicatedOnFireState, KnockdownImpulse, MutatedClass,
		CarriedCrate, ShotAtCount, CringeCount, CurrentGrapplingMarker, DesiredKantusFadeVal, KantusReviver, bLastHitWasHeadShot,
		KismetAnimSets, ReplicatedMaterial, BloodOpacity, bScriptedWalking, DefaultHealth, ClampedBase,
		ReplicatedFaceFXAnimSets, bIsBleedingOut;

	if (Physics == PHYS_RigidBody && !bTearOff && Role == ROLE_Authority)
		ReplicationRootBodyPos;

	if (bNetOwner && Role == ROLE_Authority)
		bCanPickupFactoryWeapons;

	if( Role == Role_Authority && SpecialMove == SM_Hostage )
		HostageHealth;

	if( bNetOwner && (Role == ROLE_Authority) && bInDuelingMiniGame )
		DuelingMiniGameButtonPresses;

	// replicated to ALL only if recently hit
	if (Role == ROLE_Authority && WorldInfo.TimeSeconds < LastTakeHitTimeout)
		LastTakeHitInfo;

	// Replicated to ALL once (at spawn time)
	if( Role == ROLE_Authority && bNetInitial )
		DefaultGroundSpeed, bDisableShadows;

	if( bTearOff && Role==ROLE_Authority )
		DurationBeforeDestroyingDueToNotBeingSeen, bEnableEncroachCheckOnRagdoll, KilledByPawn;
}

// overidden to skip anchor checks when we're in sily phys modes SILLY!
native function NavigationPoint GetBestAnchor( Actor TestActor, Vector TestLocation, bool bStartPoint, bool bOnlyCheckVisible, out float out_Dist );

native function bool IsValidEnemyTargetFor(const PlayerReplicationInfo PRI, bool bNoPRIisEnemy) const;

/** Read access to native APawn::MaxSpeedModifier() */
native final function float	GetMaxSpeedModifier();

native final function bool	IsInCover();
native final function bool	GuessAtCover( out CoverInfo out_Cover );

/** Retrieves the combat zone this navigation point encompassed by */
native final function CombatZone GetCombatZoneForNav( NavigationPoint Nav );
/** Claim cover for this pawn + bookkeeping */
native final function bool ClaimCover( CoverLink Link, int SlotIdx );
/** Unclaim cover for this pawn + bookkeeping */
native final function bool UnclaimCover( CoverLink Link, int SlotIdx, bool bUnclaimAll );

/**
 * Tries to find cover. Looking from FromLoc, towards direction, for MaxDistance units.
 * @param	FromLoc		Location to start looking from.
 * @param	Direction	to look towards to.
 * @param	FOV			Cover has to be within this FOV from Direction. (Dot, 0.f == 0d, 1.f == 180d)
 * @param	MaxDistance	Maximum distance cover can be at.
 * @param	OutCovPos	Cover information, if found.
 * @return	TRUE if valid cover was found.
 */
native final simulated function bool FindCoverFromLocAndDir(Vector FromLoc, Vector Direction, float FOV, float MaxDistance, out CovPosInfo OutCovPosInfo);

/**
  * If we've found potential cover, make sure it's usable.
  * @param	FromLoc					Location of player willing to enter cover
  * @param	Direction				Direction player is looking for cover
  * @param	FOV						Cover has to be within this FOV from Direction. (Dot, 0.f == 0d, 1.f == 180d)
  * @param	OutMaxDistanceSquared	Max Distance squared cover has to be from player. If less, then it is updated with new value.
  * @param	OutCovPosInfo			Cover information to validate.
  */
native final simulated function bool ValidatePotentialCover(Vector FromLoc, Vector Direction, out float OutMinDotFOV, out float OutMaxDistanceSquared, out CovPosInfo OutCovPosInfo);

/**
 *	Fills in covposinfo vars given the cover slot
 */
native final simulated function FillCoverPosInfo( CoverLink Link, int LtSlotIdx, int RtSlotIdx, Vector FromLoc, Vector Direction, float MaxDistance, out CovPosInfo out_CovPosInfo );

// TNT uses this but we do not want to have special compile which will create a new fooClasses.h
// each time so we will just always have this function
//native final function VinceNotifyCoverStatus( bool bIsInCover );

//TNT
exec native function GetCoverInfo();
exec native function GetStreamingInfo( string levelStatus );


function SpawnDefaultController()
{
	Super.SpawnDefaultController();
	// set the default team based on the class type
	if (IsA('GearPawn_COGGear'))
	{
		WorldInfo.Game.ChangeTeam(Controller,0,FALSE);
	}
	else
	{
		WorldInfo.Game.ChangeTeam(Controller,1,FALSE);
	}
}

function PlayerStatsUpdate()
{
	if ( WorldInfo == None || WorldInfo.GRI == None || GearGRI(WorldInfo.GRI) == None || GearGRI(WorldInfo.GRI).GameStatus != GS_EndMatch )
	{

		// We are no longer using PlayerStatsUpdate.  The stat system uses a timeout value and auto-create new lines events when the old event
		// has expired.

//		`RecordStat('PlayerStatsUpdate',Controller,CoverType!=CT_None?"InCover":"NotInCover");
	}
}

simulated function CheckDefaultMeshTranslation()
{
	// If Root bone is between the character's feet. Then Offset mesh properly
	// Down by CollisionHeight plus magic number so feet touch the ground.
	// Because CollisionCylinders slightly hovers above ground.
	if( bTranslateMeshByCollisionHeight && default.Mesh != None )
	{
		// Update value from default component
		default.Mesh.SetTranslation(Vect(0,0,-1) * (CylinderComponent.CollisionHeight + MeshTranslationNudgeOffset));
		// Reflect change on current mesh.
		SetMeshTranslationOffset(MeshTranslationOffset, TRUE);
	}
}

simulated event PostBeginPlay()
{
	local GearPC GPC;
	local int Idx;
	local int i;
	local array<texture> Textures;
	//local MaterialInterface MatInstance;

	Super.PostBeginPlay();

	CheckDefaultMeshTranslation();

	if( GearPawnFX == None )
	{
		Assert(GearPawnFXClass != None);
		GearPawnFX = new(self) GearPawnFXClass;
	}

	// Register with the GUDManager...
	if (MasterGUDBankClassNames.length > 0)
	{
		if (Role == ROLE_Authority)
		{
			GearGame(WorldInfo.Game).UnscriptedDialogueManager.RegisterSpeaker(self);
		}
		else if (Mesh != None && Mesh.SkeletalMesh != None && Mesh.SkeletalMesh.FaceFXAsset != None)
		{
			foreach LocalPlayerControllers(class'GearPC', GPC)
			{
				for (i = 0; i < GPC.ClientGUDSReferences.length; i++)
				{
					if ( GPC.ClientGUDSReferences[i].Bank != None &&
						MasterGUDBankClassNames.Find(GPC.ClientGUDSReferences[i].Bank.SourceGUDBankPath) != INDEX_NONE )
					{
						Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(GPC.ClientGUDSReferences[i].FaceFXData);
					}
				}
				break;
			}
		}
	}

	// Cache animation nodes
	CacheAnimNodes();
	// Set proper parameters on AnimSets
	AnimSetsListUpdated();

	// Initial setup of fire tickets
	SetNumberOfFireTickets( NumFireTickets );

	// Unfix the 'bFullAnimWeight' bodies
	if( Mesh != None && Mesh.PhysicsAssetInstance != None )
	{
		Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);
	}

	// Don't update the skin on dedicated servers
	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		// Get the material instance from the mesh
//		MatInstance = Mesh.GetMaterial(0);
//		if (MatInstance != None)
//		{
//			// Create our constant that will aggregate the existing material
//			PawnColorMatInstance = new(Outer) class'MaterialInstanceConstant';
//			PawnColorMatInstance.SetParent(MatInstance);
//			// Replace the old material with the instance constant version
//			Mesh.SetMaterial(0,PawnColorMatInstance);
//		}
	}

	FacialAudioComp.OnAudioFinished = FaceFXAudioFinished;

	// hook up the FaceFXAnimSets here based on SP or MP gametype
	if (Mesh != None && Mesh.SkeletalMesh != None && Mesh.SkeletalMesh.FaceFXAsset != None)
	{
		for (Idx=0; Idx<FAS_Efforts.length; ++Idx)
		{
			Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet( FAS_Efforts[Idx] );
		}

//		if( ( GearGRI(WorldInfo.GRI).IsMultiplayerGame() )
//		{
//			if( FAS_MultiPlayer != none )
//			{
//				Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet( FAS_MultiPlayer );
//			}
//		}
//		else
//		{
//			if( FAS_SinglePlayer != none )
//			{
//				Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet( FAS_SinglePlayer );
//			}
//		}
	}

	// material instance setup.  do the mp rimshaders first, and then we'll derive from those for
	// subsequent materials
	InitMPRimShader();

	if( Mesh != None )
	{
		// must be after rimshader initialization, so that doens't stomp this material.
		InitMICPawnMaterial();

		// when we spawn we want to make our textures are resident right off the bat so they are not all mipped out
		for( Idx = 0; Idx < Mesh.Materials.length; ++Idx )
		{
			Textures = Mesh.Materials[Idx].GetMaterial().GetTextures();

			for( i = 0; i < Textures.Length; ++i )
			{
				//`log( "Texture setting SetForceMipLevelsToBeResident( 15.0f ): " $ Textures[i] );
				Texture2D(Textures[i]).SetForceMipLevelsToBeResident( 15.0f );
			}
		}
	}

	SetLightEnvironmentSettings();


	`if(`notdefined(FINAL_RELEASE))
		CheckGoreJoints();
	`endif

}

simulated function Destroyed()
{
	local GearPC			GPC;
	local int				i, Idx;
	local array<texture>	Textures;

	// If we're currently doing a special move when we're being destroyed
	if( SpecialMove != SM_None && SpecialMoves[SpecialMove] != None )
	{
		// And end the current special move, as other actors may be relying on that
		EndSpecialMove();
	}

	// Clear references to instanced special move classes.
	SpecialMoves.Length = 0;

	// Clear references to animations nodes
	ClearAnimNodes();

	// Unregister with the GUD manager
	if (MasterGUDBankClassNames.length > 0 && GearGame(WorldInfo.Game) != None)
	{
		GearGame(WorldInfo.Game).UnscriptedDialogueManager.UnregisterSpeaker(self);
	}

	// destroy client-side weapons
	if( GearInventoryManager(InvManager) != None )
	{
		GearInventoryManager(InvManager).DiscardClientSideInventory();
	}

	// destroy slave weapon
	if( MyGearWeapon != None && !MyGearWeapon.bDeleteMe && MyGearWeapon.bSlaveWeapon )
	{
		MyGearWeapon.Destroy();
		MyGearWeapon = None;
	}

	// perform attachment clean up
	CleanupAttachSlots();

	GPC = GearPC(Controller);
	if( GPC != none )
	{
		GearPlayerCamera(GPC.PlayerCamera).SetDesiredColorScale(vect(1.0f,1.0f,1.0f),1.0f);
	}

	if( Role == Role_Authority && POI != None )
	{
		POI.Destroy();
	}

//	if (PawnColorMatInstance != None)
//	{
//		PawnColorMatInstance.SetParent(None);
//		Mesh.SetMaterial(0,None);
//		PawnColorMatInstance = None;
//	}

	// reset all of the gore textures to not be resident
	if( GoreSkeletalMesh != None )
	{
		for( Idx = 0; Idx < GoreSkeletalMesh.Materials.Length; ++Idx )
		{
			Textures = GoreSkeletalMesh.Materials[Idx].GetMaterial().GetTextures();

			for( i = 0; i < Textures.Length; ++i )
			{
				//`log( "Texture setting bForceMiplevelsToBeResident FALSE: " $ Textures[i] );
				Texture2D(Textures[i]).bForceMiplevelsToBeResident = FALSE;
			}
		}
	}

	// clean up any on-fire emitters
	if (OnFireState != POF_None)
	{
		PSC_OnFire.DeactivateSystem();
		DetachComponent(PSC_OnFire);
		PSC_OnFire = None;
	}

	// notify ai vis man we just got popped
	if( GearGame(WorldInfo.Game) != None )
	{
		GearGame(WorldInfo.Game).AIVisMan.NotifyPawnDestroy(self);
	}


	Super.Destroyed();
}


/** This will set up all of the MICs on the helmets if this pawn has one **/
simulated function SetupHelmetMaterialInstanceConstant();

/** This will set up all of the MICs on the shoulder pads if this pawn has one **/
simulated function SetupShoulderPadsMaterialInstanceConstant();


/** This will stream in all of the textures of the character **/
simulated function CharacterTexturesForceMipsResident()
{
	CharacterTextureForceResident( TRUE );
	SetTimer( 5.0f, FALSE, nameof(CharacterTexturesForceMipsNonResident) );
}

/** This will allow the streaming code to stream in or out all of the textures of the character **/
simulated function CharacterTexturesForceMipsNonResident()
{
	CharacterTextureForceResident( FALSE );
}

/** This does the work of getting all textures and setting them to be resident or not **/
simulated function CharacterTextureForceResident( bool Value )
{
	local int i;
	local array<texture> Textures;

	Textures = Mesh.GetMaterial( 0 ).GetMaterial().GetTextures();

	for( i = 0; i < Textures.Length; ++i )
	{
		Texture2D(Textures[i]).bForceMiplevelsToBeResident = Value;
	}
}


simulated function int GetLookAtPriority( GearPC PC, int DefaultPriority )
{
	if ( (PC.Pawn != self) && (IsDBNO() || (Health > 0)) && WorldInfo.GRI.OnSameTeam(PC.Pawn, self) )
	{
		if ( IsDBNO() )
		{
			return class'GearPointOfInterest'.Default.POIPriority_RevivableComrade;
		}
		else
		{
			if ( IsPlayerOwned() )
			{
				return class'GearPointOfInterest'.Default.POIPriority_ComradeHuman;
			}
			return class'GearPointOfInterest'.Default.POIPriority_Comrade;
		}
	}

	return DefaultPriority;
}

/** makes sure all attachments for a nonlocal client pawn are correct */
simulated final function RefreshRemoteAttachments()
{
	if( WorldInfo.NetMode == NM_Client && !IsLocallyControlled() )
	{
		UpdateRemoteWeapon();
		UpdateRemoteShield(AttachClass_Shield);
		SetSlotAttachment(EASlot_Holster, AttachSlotClass_Holster);
		SetSlotAttachment(EASlot_Belt, AttachSlotClass_Belt);
		SetSlotAttachment(EASlot_LeftShoulder, AttachSlotClass_LeftShoulder);
		SetSlotAttachment(EASlot_RightShoulder, AttachSlotClass_RightShoulder);
	}
}

/** replicated event */
simulated event ReplicatedEvent( name VarName )
{
	local MaterialInstanceConstant MIC;
	local bool bFound;
	local int i, j;

	switch( VarName )
	{
		case 'ShotAtCount' : ShotAtCountIncreased(); break;
		case 'CringeCount' : CringeCountIncreased(); break;
		case 'ReplicatedSpecialMoveStruct' :
			`logsm("Received replicated special move:"@ReplicatedSpecialMoveStruct.SpecialMove);
			DoSpecialMoveFromStruct(ReplicatedSpecialMoveStruct, TRUE);
			break;
		case 'CrawlSpeedModifier':
			if (DBNOCrawlNode != None)
			{
				DBNOCrawlNode.Rate = 1.f + CrawlSpeedModifier;
			}
			break;
		case nameof(bRaiseHandWhileCrawling):
			if (bRaiseHandWhileCrawling)
			{
				RaiseHandWhileCrawling();
			}
			else
			{
				LowerHandWhileCrawling();
			}
			break;
		case 'CoverAction' :
			CoverActionChanged();
			break;
		case 'LastTakeHitInfo':
			ReplicatedPlayTakeHitEffects();
			break;
		case 'bIsTargeting' :
			// Notification that targeting mode has been changed.
			TargetingModeChanged();
			break;
		case 'InvManager':
			// everyone's InvManager gets replicated into demos so we need to update attachments after receiving it
			if (WorldInfo.IsPlayingDemo() && InvManager != None)
			{
				RefreshRemoteAttachments();
			}
			break;
		// update right hand weapon attachment
		case 'RemoteWeaponRepInfo':
			UpdateRemoteWeapon();
			break;
		case 'AttachClass_Shield':
			UpdateRemoteShield(AttachClass_Shield);
			break;
		// slot attachments
		case 'AttachSlotClass_Holster':
			SetSlotAttachment( EASlot_Holster, AttachSlotClass_Holster );
			break;
		case 'AttachSlotClass_Belt':
			SetSlotAttachment( EASlot_Belt, AttachSlotClass_Belt );
			break;
		case 'AttachSlotClass_LeftShoulder':
			SetSlotAttachment( EASlot_LeftShoulder, AttachSlotClass_LeftShoulder );
			break;
		case 'AttachSlotClass_RightShoulder':
			SetSlotAttachment( EASlot_RightShoulder, AttachSlotClass_RightShoulder );
			break;
		case 'AcquiredCoverInfo' :
			LastCoverAcquireDistanceSq = VSizeSq2D(AcquiredCoverInfo.Location - Location);
			SetCovPosInfo(AcquiredCoverInfo, FALSE);
			// kick off a special move depending on whether or not we received cover or lost cover
			if( AcquiredCoverInfo.Link != None )
			{
				// do run2cover special move
				if( GetCoverTypeFor(AcquiredCoverInfo) == CT_MidLevel )
				{
					DoSpecialMove(SM_Run2MidCov, TRUE);
				}
				else
				{
					DoSpecialMove(SM_Run2StdCov, TRUE);
				}
			}
			break;
		case 'bChargingBow':
			ToggleChargingBow();
			break;
		case 'bSpawnABloodPool':
			SpawnBloodPool();
			break;

		case 'ReplicatedSpeakLineParams':
			if (ReplicatedSpeakLineParams.Audio != None)
			{
				QueuedSpeakLineParams = ReplicatedSpeakLineParams;
				if (QueuedSpeakLineParams.DelayTime > 0.f)
				{
					// play later
					SetTimer( QueuedSpeakLineParams.DelayTime, false, nameof(PlayQueuedSpeakLine) );
				}
				else
				{
					// play now!
					PlayQueuedSpeakLine();
				}
			}
			break;

		case 'bCoveringHead':
			if (bCoveringHead)
			{
				PlayHeadCoverAnim(-1.f);
			}
			else
			{
				StopPlayingHeadCoverAnim();
			}
			break;
		case 'bWantToConverse':
		case 'bWantToUseCommLink':
			if (VarName == 'bWantToConverse' || bWantToConverse)
			{
				SetConversing(bWantToConverse, bWantToUseCommLink);
				bIsConversing = false; // force reset so bWantToUseCommLink is evaluated again
			}
			break;
		case 'ReplicatedForcedAimTime' :
			SetWeaponAlert(float(ReplicatedForcedAimTime));
			break;
		case 'KnockdownImpulse':
			if (!IsZero(KnockdownImpulse.LinearVelocity) || !IsZero(KnockdownImpulse.AngularVelocity))
			{
				ApplyKnockdownImpulse();
			}
			break;
		case 'ReplicatedOnFireState':
			IgnitePawn(ReplicatedOnFireState);
			break;
		case nameof(MutatedClass):
			if (MutatedClass != None)
			{
				MutatedClass.static.MutatePawn(self);
			}
			break;
		case 'CarriedCrate':
			CarriedCrateStatusChanged();
			break;
		case 'HostageHealth' :
			UpdateHostageHealth();
			break;
		case nameof(bActiveReloadBonusActive):
			if (GearWeapon(Weapon) != None)
			{
				if (bActiveReloadBonusActive)
				{
					GearWeapon(Weapon).TurnOnActiveReloadBonus_VisualEffects();
				}
				else
				{
					GearWeapon(Weapon).TurnOffActiveReloadBonus_VisualEffects();
				}
			}
			break;
		case nameof(bDisableShadows):
			UpdateShadowSettings(!bDisableShadows);
			break;
		case nameof(KismetAnimSets):
			UpdateAnimSetList();
			break;
		case nameof(ReplicatedMaterial):
			if (ReplicatedMaterial != None)
			{
				Mesh.SetMaterial(0, ReplicatedMaterial);
			}
			else
			{
				Mesh.SetMaterial(0, default.Mesh.GetMaterial(0));
			}
			break;
		case nameof(BloodOpacity):
			MIC = MaterialInstanceConstant(Mesh.GetMaterial(0));
			if (MIC != None)
			{
				MIC.SetScalarParameterValue('BloodOpacity', BloodOpacity);
			}
			break;
		case nameof(ReplicatedFaceFXAnimSets):
			for (i = 0; i < ArrayCount(PrevReplicatedFaceFXAnimSets); i++)
			{
				if (PrevReplicatedFaceFXAnimSets[i] != None)
				{
					bFound = false;
					for (j = 0; j < ArrayCount(ReplicatedFaceFXAnimSets); j++)
					{
						if (PrevReplicatedFaceFXAnimSets[i] == ReplicatedFaceFXAnimSets[j])
						{
							bFound = true;
							break;
						}
					}
					if (!bFound)
					{
						Mesh.SkeletalMesh.FaceFXAsset.UnmountFaceFXAnimSet(PrevReplicatedFaceFXAnimSets[i]);
					}
				}
			}
			for (i = 0; i < ArrayCount(ReplicatedFaceFXAnimSets); i++)
			{
				if (ReplicatedFaceFXAnimSets[i] != None)
				{
					Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(ReplicatedFaceFXAnimSets[i]);
				}
				PrevReplicatedFaceFXAnimSets[i] = ReplicatedFaceFXAnimSets[i];
			}
			break;
	}

	Super.ReplicatedEvent(VarName);
}

simulated function ToggleChargingBow()
{
	if( MyGearWeapon != None )
	{
		MyGearWeapon.ToggleCharging(bChargingBow);
	}
}

simulated function NotifyTeamChanged()
{
	local PlayerController PC;

	Super.NotifyTeamChanged();

	// remember last team for the HUD when we die
	if (PlayerReplicationInfo != None && PlayerReplicationInfo.Team != None)
	{
		LastTeamNum = GetTeamNum();
	}

	// Make sure local player's PRI has been received
	ForEach LocalPlayerControllers(class'PlayerController', PC)
		break;

	if( (PC == None) || (PC.PlayerReplicationInfo == None) )
	{
		return;
	}

	// we have everything we need to update attachment visibility
	SetTimer(0.0001,FALSE,nameof(UpdateAttachmentVisibility));

	// TeamNum 1 is locust (we don't want to spawn)
	if( POI == None && Role == ROLE_Authority && GetTeamNum() != 1 &&
		(GearGameSP_Base(WorldInfo.Game) != None || GearGRI(WorldInfo.GRI).IsCoopMultiplayerGame()) )
	{
		//`log( "Spawning POI! " $ GetTeamNum() );
		POI = Spawn( class'GearPointOfInterest', self );
	}

}

/**
 * We call this once per pawn.	Right now this is called from PostBeginPlay()
 *
 * This could be turned into "lazy" loaded and do it on demand.
 */
simulated function CacheAnimNodes()
{
	local GearSkelCtrl_Recoil	RecoilNode;
	local AnimNode				Node;
	local int					Idx;
	local GearAnim_Slot			SlotNode;
	local AnimNodeAimOffset		AimNode;
	local MorphNodePose			MorphNodePose;
 `if(`notdefined(FINAL_RELEASE))
   	local array<Name>			NodeNames;
 `endif

	if( Mesh == None )
		return;

	// Cache anim tree root node
	AnimTreeRootNode = AnimTree(Mesh.Animations);

	// Randomize Idle animations to characters spawned at the same time don't look all identical.
	if( AnimTreeRootNode != None )
	{
		AnimTreeRootNode.ForceGroupRelativePosition('IdleAnims', (FRand() + FRand()) * 0.5f );
	}

	foreach Mesh.AllAnimNodes(class'AnimNode', Node)
	{
		AimNode = AnimNodeAimOffset(Node);

		// Skip if node is not interesting to us
		if( Node.NodeName == '' && AimNode == None )
		{
			continue;
		}

 `if(`notdefined(FINAL_RELEASE))
	   	if ( Node.NodeName != 'None' )
		{
			if ( NodeNames.Find(Node.NodeName) != INDEX_NONE )
			{
				`log("WARNING: Duplicate NodeName detected in animation tree:"@Node.NodeName@self@Mesh.SkeletalMesh);
			}
			else
			{
				NodeNames.AddItem(Node.NodeName);
			}
		}
`endif

		switch( Node.NodeName )
		{
			case 'BaseBlendNode' : BaseBlendNode = GearAnim_BaseBlendNode(Node);	break;
			case 'BlendByAimNode' :
				BlendByAimNode = GearAnim_BlendAnimsByAim(Node);
				break;
			case 'BlendByAimToggle' :
				BlendByAimToggle = GearAnim_BlendList(Node);
				break;
			case 'IKBoneFix' :
				IKHackNode = GearAnim_UpperBodyIKHack(Node);
				break;
			case 'BlendList_Troika':
				// Troika test blend node
				TroikaBlendNode	= GearAnim_BlendList(Node);
				break;
			case 'MirrorNode':
				// Cache mirror node
				MirrorNode = GearAnim_Mirror_Master(Node);
				break;
			case 'DBNO_GuardBlend' :
				DBNOGuardBlendNode = AnimNodeBlend(Node);
				break;
			case 'DBNO_Crawl' :
				DBNOCrawlNode = AnimNodeSequence(Node);
				break;
			default:
				if( AimNode != None )
				{
					AimOffsetNodes[AimOffsetNodes.length] = AimNode;
				}
				else
				{
					SlotNode = GearAnim_Slot(Node);
					if (SlotNode != None)
					{
						switch (Node.NodeName)
						{
							case FullBodyNodeName:
								// Full body custom anim node
								FullBodyNode = SlotNode;
								BodyStanceNodes[BS_FullBody] = FullBodyNode;
								break;
							case 'Custom_Camera':
								// custom camera anim node
								CustomCameraAnimNode = SlotNode;
								break;
							// Body stances
							case 'Custom_UpStand':
								BodyStanceNodes[BS_Std_Up] = SlotNode;
								break;
							case 'Slot_Std_Upper_Harsh':
								BodyStanceNodes[BS_Std_Upper_Harsh] = SlotNode;
								break;
							case 'Slot_Std_Upper_NoAim' :
								BodyStanceNodes[BS_Std_Upper_NoAim] = SlotNode;
								break;
							case 'Slot_Std_Idle_Lower':
								BodyStanceNodes[BS_Std_Idle_Lower] = SlotNode;
								break;
							case 'Slot_Std_Idle_Upper':
								BodyStanceNodes[BS_Std_Idle_Upper] = SlotNode;
								break;
							case 'Slot_Std_Walk_Upper':
								BodyStanceNodes[BS_Std_Walk_Upper] = SlotNode;
								break;
							case 'Slot_Std_Run_Upper':
								BodyStanceNodes[BS_Std_Run_Upper] = SlotNode;
								break;
							case 'Custom_Cov_Std_Idle_Upper':
								BodyStanceNodes[BS_CovStdIdle_Up] = SlotNode;
								break;
							case 'Custom_Cov_Std_Blind_Upper':
								BodyStanceNodes[BS_CovStdBlind_Up] = SlotNode;
								break;
							case 'Custom_Cov_Std_Lean_Upper':
								BodyStanceNodes[BS_CovStdLean_Up] = SlotNode;
								break;
							case 'Slot_Cov_Std_AimBack_Upper':
								BodyStanceNodes[BS_CovStd_360_Upper] = SlotNode;
								break;
							case 'Custom_Cov_Mid_Idle_Upper':
								BodyStanceNodes[BS_CovMidIdle_Up] = SlotNode;
								break;
							case 'Custom_Cov_Mid_Blind_Upper':
								BodyStanceNodes[BS_CovMidBlindSd_Up] = SlotNode;
								break;
							case 'Custom_Cov_Mid_Blind_Up_Upper':
								BodyStanceNodes[BS_CovMidBlindUp_Up] = SlotNode;
								break;
							case 'Custom_Cov_Mid_Lean_Upper':
								BodyStanceNodes[BS_CovMidLean_Up] = SlotNode;
								break;
							case 'Slot_Cov_Mid_AimBack_Upper':
								BodyStanceNodes[BS_CovMid_360_Upper] = SlotNode;
								break;
							case 'Custom_Additive' :
								BodyStanceNodes[BS_Additive] = SlotNode;
								break;
							case 'Slot_Std_idle' :
								BodyStanceNodes[BS_Std_idle_FullBody] = SlotNode;
								break;
							case 'Slot_KidnapperUpperBody' :	BodyStanceNodes[BS_Kidnapper_Upper] = SlotNode;	break;
							case 'Slot_HostageUpperBody' :		BodyStanceNodes[BS_Hostage_Upper] = SlotNode;	break;
							case 'Slot_Shield_Hunkered' :		BodyStanceNodes[BS_Shield_Hunkered] = SlotNode;	break;
							case 'Slot_MortarMounted' :			BodyStanceNodes[BS_MortarMounted] = SlotNode;	break;
							default:
								break;
						}
					}
				}
				break;
		}
	}

	HeadControl = SkelControlLookAt(Mesh.FindSkelControl('HeadLook'));

	// Cache recoil nodes
	RecoilNode = GearSkelCtrl_Recoil(Mesh.FindSkelControl('WeapRecoilNode'));
	if( RecoilNode != None )
	{
		RecoilNodes[RecoilNodes.Length] = RecoilNode;
	}

	RecoilNode = GearSkelCtrl_Recoil(Mesh.FindSkelControl('SpineRecoilNode'));
	if( RecoilNode != None )
	{
		RecoilNodes[RecoilNodes.Length] = RecoilNode;
	}

	RecoilNode = GearSkelCtrl_Recoil(Mesh.FindSkelControl('RightHandRecoilNode'));
	if( RecoilNode != None )
	{
		RecoilNodes[RecoilNodes.Length] = RecoilNode;
	}

	// Cache IK Controllers (used for World location, like Troika handles)
	IKCtrl_RightHand	= SkelControlLimb(Mesh.FindSkelControl('IK_RightHand'));
	IKCtrl_LeftHand		= SkelControlLimb(Mesh.FindSkelControl('IK_LeftHand'));
	IKRotCtrl_RightHand	= SkelControlSingleBone(Mesh.FindSkelControl('IKRot_RightHand'));
	IKRotCtrl_LeftHand	= SkelControlSingleBone(Mesh.FindSkelControl('IKRot_LeftHand'));

	// Cache IK Controllers (Hands to IK Bones)
	IKBoneCtrl_RightHand	= SkelControlLimb(Mesh.FindSkelControl('IKBone_RightHand'));
	IKBoneCtrl_LeftHand		= SkelControlLimb(Mesh.FindSkelControl('IKBone_LeftHand'));

	// if MeatShieldMorphTargetName is set, then set following information
	if ( MeatShieldMorphTargetName != '' )
	{
		// MorphNodeWeight for meatshield.
		MeatShieldMorphNodeWeight = MorphNodeWeight(Mesh.FindMorphNode('MeatShieldMorphNodeWeight'));
		// Set Morph Target Name
		MorphNodePose = MorphNodePose(Mesh.FindMorphNode('Meatshield_Morph'));
		if (MorphNodePose != None)
		{
			MorphNodePose.SetMorphTarget(MeatShieldMorphTargetName);
		}
	}

	//debug
	`AILog_Ext( self@GetFuncName()@BodyStanceNodes.Length, 'None', MyGearAI );
	for( Idx = 0; Idx < BodyStanceNodes.Length; Idx++ )
	{
		`AILog_Ext( self@"BodyStance"@Idx@BodyStanceNodes[Idx], 'None', MyGearAI );
	}
}


/** Clear references to anim nodes so AnimTree can be garbage collected. */
simulated function ClearAnimNodes()
{
	AnimTreeRootNode	= None;
	FullBodyNode		= None;
	HeadControl			= None;
	TroikaBlendNode		= None;
	MirrorNode			= None;
	DBNOGuardBlendNode	= None;
	DBNOCrawlNode		= None;
	IKHackNode			= None;
	BlendByAimNode		= None;
	BlendByAimToggle	= None;
	BaseBlendNode		= None;

	AimOffsetNodes.Length	= 0;
	BodyStanceNodes.Length	= 0;
	RecoilNodes.Length		= 0;

	bTargetingNodeIsInIdleChannel = default.bTargetingNodeIsInIdleChannel;

	IKCtrl_RightHand	= None;
	IKCtrl_LeftHand		= None;

	IKRotCtrl_RightHand	= None;
	IKRotCtrl_LeftHand	= None;
}

simulated function protected CacheCrateAnimNodes()
{
	CrateIKLeftHand		= SkelControlLimb(Mesh.FindSkelControl('IK_LeftHand'));
	CrateIKRightHand	= SkelControlLimb(Mesh.FindSkelControl('IK_RightHand'));

	CrateMirrorNode		= AnimNodeMirror(Mesh.FindAnimNode('CrateMirrorNode'));

	AimOffsetNodes[0]	= AnimNodeAimOffset(Mesh.FindAnimNode('CrateAimOffset'));

	BodyStanceNodes[BS_Std_Up] = GearAnim_Slot(Mesh.FindAnimNode('Custom_UpStand'));

	CrateWalkNode		= GearAnim_BlendAnimsByDirection(Mesh.FindAnimNode('CrateWalkAnim'));
}

simulated function protected ClearCrateAnimNodes()
{
	CrateIKLeftHand			= None;
	CrateIKRightHand		= None;

	CrateMirrorNode			= None;

	AimOffsetNodes.length	= 0;
	BodyStanceNodes.length	= 0;

	CrateWalkNode			= None;
}

simulated protected function InitMPRimShader();

/** This will create the MIC for this pawn's materials **/
simulated protected function InitMICPawnMaterial()
{
	MIC_PawnMat = Mesh.CreateAndSetMaterialInstanceConstant(0);

	// if this pawn has hair (which is in slot 1)
	if( Mesh.GetMaterial(1) != none )
	{
		MIC_PawnHair = Mesh.CreateAndSetMaterialInstanceConstant(1);
	}
}


/** notification fired from Pawn animation when clip is released from weapon. */
simulated function Notify_AmmoRelease()
{
	MyGearWeapon.Notify_AmmoRelease();
}

simulated function Notify_AmmoRelease2()
{
	MyGearWeapon.Notify_AmmoRelease2();
}

/** Notify called when ammo is thrown by player */
simulated function Notify_AmmoThrow()
{
	MyGearWeapon.Notify_AmmoThrow();
}

/** notification fired from Pawn animation when player grabs another magazine. */
simulated function Notify_AmmoGrab()
{
	MyGearWeapon.Notify_AmmoGrab();
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload()
{
	MyGearWeapon.Notify_AmmoReload();
}

/** notification fired from Pawn animation when player puts new magazine in weapon. */
simulated function Notify_AmmoReload2()
{
	MyGearWeapon.Notify_AmmoReload2();
}

simulated function Notify_DoorKick()
{
	// effort sound
	SoundGroup.PlayEffort(self, GearEffort_KickEffort);
	PlayKickSound();
}

simulated function Notify_DoorPush()
{
	// effort sound
	SoundGroup.PlayEffort(self, GearEffort_DoorPullEffort);
}

simulated function Notify_GrenadeSwing()
{
	local GearWeap_GrenadeBase NadeWeap;

	NadeWeap = GearWeap_GrenadeBase(MyGearWeapon);
	if (NadeWeap != None)
	{
		NadeWeap.NotifyGrenadeSwing();
	}
};


/**
 * Handles setting up the helmet for this character based on HelmetType.
 */
simulated function SetupHelmet();

/** if this pawn has a helmet on or not **/
simulated function bool HasHelmetOn();

/**
 * Handles setting up the shoulderpads for this character based on shoulderpad types.
 */
simulated function SetUpShoulderPads();


function Restart()
{
	Super.Restart();

	CurrentLink = None;
	CurrentSlotIdx = -1;
	ClosestSlotIdx = -1;
	SetCoverType( CT_None );
	SetCoverAction( CA_Default );
}

simulated function ClientRestart()
{
	bQuietWeaponEquipping = TRUE;

	super.ClientRestart();

	CurrentLink = None;
	CurrentSlotIdx = -1;
	ClosestSlotIdx = -1;
	SetCoverType( CT_None );
	SetCoverAction( CA_Default );
	SetTargetingMode(false);

	SetTimer( 3.0f, FALSE, nameof(AllowWeaponEquippingSound) );
}


/**
 * We spawn the pawn pretty early so it is around along with all of its weapons and then it is
 * teleported to the correct place.	 During that time it will play sounds which is not wanted.
 * Specifically, we are stopping the weapon Equip/DeEquip sounds which play when the weapons are
 * first created and/or given to the pawn.
 **/
simulated function AllowWeaponEquippingSound()
{
	bQuietWeaponEquipping = FALSE;
}

/** debug function to get all body setup bone names, for locational damage */
exec function LogBodySetupBoneNames()
{
	local int			i, setupmax;
	local PhysicsAsset	PA;
	local Controller	C;

	`log(GetFuncName());

	foreach WorldInfo.AllControllers(class'Controller', C)
	{
		if( C.Pawn != None && C.Pawn.Mesh != None && C.Pawn.Mesh.PhysicsAsset != None )
		{
			PA = C.Pawn.Mesh.PhysicsAsset;

			`log(" logging BodySetup bone names for" @ C.Pawn);

			setupmax = PA.BodySetup.Length;
			for (i=0; i<setupmax; i++)
			{
				`log("	BoneName:" @ PA.BodySetup[i].BoneName );
			}
		}
		else
		{
			`log(" Not valid setup for" @ C);
		}
	}

}

/** Returns TRUE of this hit corresponds to a head shot. */
simulated function bool TookHeadShot(Name BoneName, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	local vector HeadLocation;
	local bool bBoneHitHead;

	if(HeadBoneNames.length > 0)
	{
		HeadLocation = Mesh.GetBoneLocation(HeadBoneNames[0])
			+ MPHeadOffset.X * Mesh.GetBoneAxis(HeadBoneNames[0], AXIS_X)
			+ MPHeadOffset.Y * Mesh.GetBoneAxis(HeadBoneNames[0], AXIS_Y)
			+ MPHeadOffset.Z * Mesh.GetBoneAxis(HeadBoneNames[0], AXIS_Z);

		// Find distance from head location to bullet vector
		bBoneHitHead = ( PointDistToLine(HeadLocation, Normal(Momentum), HitLocation) < MPHeadRadius );

	}

	// in multiplayer game we don't care about helmets
	if( GearGRI(WorldInfo.GRI).IsMultiplayerGame() )
	{
		return bBoneHitHead;
	}
	// for SP we want to remove helmets based on head shots
	else
	{
		if( bBoneHitHead == TRUE )
		{
			// if we have a helmet then note that so that damage is negated, helmet popped off, etc
			if (HasHelmetOn())
			{
				out_bHasHelmet = 1;
			}
			else
			{
				out_bHasHelmet = 0;
			}

			return TRUE;
		}

		return FALSE;
	}
}

final function bool TookLegShot(Name BoneName)
{
	return LegBoneNames.Find(BoneName) != INDEX_NONE;
}

simulated event vector GetHeadLookTargetLocation()
{
	local Pawn P;
	if( !bIsTargeting && HeadLookAtActor != None )
	{
		P = Pawn(HeadLookAtActor);
		if(	(P != None) && (P.Mesh != None) && (HeadLookAtBoneName != '') )
		{
			return P.Mesh.GetBoneLocation(HeadLookAtBoneName);
		}
		else
		{
			return HeadLookAtActor.Location;
		}
	}
	// return straight ahead
	return (Location + vector(Rotation) * 1024.f);
}

/**
 * Draw debug information for AimAttractors
 */
simulated function DrawDebugAimAttractors()
{
	local int AttractorIdx, NumAttractors;
	local AimAttractor Attr;
	local vector AttrPos;

	if ( (Health > 0) && ( (Controller == None) || !Controller.IsLocalPlayerController() ) )		// @fixme work in multi?
	{
		NumAttractors = AimAttractors.Length;

		// loop over array, draw the spheres
		for (AttractorIdx=0; AttractorIdx<NumAttractors; ++AttractorIdx)
		{
			Attr = AimAttractors[AttractorIdx];
			AttrPos = Mesh.GetBoneLocation(Attr.BoneName);
			DrawDebugSphere(AttrPos, Attr.InnerRadius, 8, 255, 16, 16, FALSE);
			DrawDebugSphere(AttrPos, Attr.OuterRadius, 8, 255, 255, 16, FALSE);
		}
	}
}

simulated function Tick(float DeltaTime)
{
	local float PctHealthRemaining;

`if(`notdefined(FINAL_RELEASE))
	local PlayerController PC;
`endif

	//@super-hack
	if (bIsZoomed && Mesh != None)
	{
		Mesh.LastRenderTime = WorldInfo.TimeSeconds;
	}

	// Forward Script Tick event
	if( SpecialMove != SM_None &&  SpecialMoves[SpecialMove] != None )
	{
		SpecialMoves[SpecialMove].Tick(DeltaTime);
	}

	// Update Stopping Power
	UpdateStoppingPower(DeltaTime);

	/*
	// Rotation debugging
	if( AIController(Controller) != None )
	{
		DrawDebugLine(Location, Location + Vector(DesiredRotation) * 200.f,				000, 000, 255);
		DrawDebugLine(Location, Location + Vector(Controller.DesiredRotation) * 200.f,	000, 255, 000);
		DrawDebugLine(Location, Location + Vector(Rotation) * 200.f,					255, 000, 000);
	}
	*/
	`if(`notdefined(FINAL_RELEASE))
	if( bDebugAimAttractors )
	{
		DrawDebugAimAttractors();
	}
	`endif

	if( (BurnEffectPSC != None) && !BurnEffectPSC.bIsActive )
	{
		BurnEffectPSC = None;
	}

	if (Role == ROLE_Authority && HeatDamageInstigator != None && CurrentSkinHeat > 0.0)
	{
		HeatDamageFraction += HeatDamagePerSecond * DeltaTime * CurrentSkinHeat;
		if (HeatDamageFraction > 1.0)
		{
			TakeDamage(int(HeatDamageFraction), HeatDamageInstigator, Location, vect(0,0,-1), class'GDT_FireDOT');
			HeatDamageFraction -= int(HeatDamageFraction);
		}
	}

	FadeSkinEffects(DeltaTime);

	// check to see if we have moved onto another phys material from the last time we activated the roadierun PSC
	if( ( PSC_RoadieRun != None ) && ( LastFootStepTraceInfo.PhysMaterial != LastFootStepPhysMat ) )
	{
		StartRoadieRunPSC();
	}

	if( IsAliveAndWell() )
	{
		// If locally controlled, handle firing
		if( IsLocallyControlled() )
		{
			HandleWeaponFiring();
		}

		// Locally controlled human pawns in cover, do this from GearPC::PlayerTakingCover::PlayerMove()
		if( CoverType == CT_None || !IsLocallyControlled() || !IsHumanControlled() )
		{
			UpdateMeshBoneControllers(DeltaTime);
		}

		// if this takes too much time, methinks we can distribute load over multiple frames
		// slight delays in start/stop auto headtracks are tolerable
		UpdateAutomaticHeadTracking();

		// deal with heavy breathing while very hurt
		PctHealthRemaining = Health/float(DefaultHealth);
		if( (NearDeathBreathSound == None) && (PctHealthRemaining < 0.5f) && (PctHealthRemaining > 0.0f) && GearGRI(WorldInfo.GRI).IsMultiPlayerGame() )
		{
			NearDeathBreathSound = SoundGroup.PlayEffortEx(self, GearEffort_NearDeathBreathe, 1.f);
		}
		else if( (NearDeathBreathSound != None) && (PctHealthRemaining > 0.5f) )
		{
			NearDeathBreathSound.FadeOut(1.f, 0.f);
			NearDeathBreathSound = None;
		}

		// deal with conversation mode
		if( !bIsConversing && bWantToConverse )
		{
			// Want to use commlink
			if( bWantToUseCommLink )
			{
				if (IsDoingMeleeHoldSpecialMove())
				{
					ForceStopMeleeAttack(TRUE);
				}

				// client may already be doing special move from ReplicatedServerMove
				if (IsDoingSpecialMove(SM_UsingCommLink))
				{
					bIsConversing = TRUE;
				}
				else
				// See if local player can enter in CommLink mode now
				if (CanDoSpecialMove(SM_UsingCommLink))
				{
					if (Role == ROLE_Authority)
					{
						ServerDoSpecialMove(SM_UsingCommLink);
					}
					else
					{
						DoSpecialMove(SM_UsingCommLink);
					}

					bIsConversing = TRUE;
				}
			}
			// If don't want to use CommLink, can converse now
			else
			{
				bIsConversing = TRUE;
			}
		}

		if( Role == Role_Authority  )
		{
			// don't do this if dead!
			if( bHealthIsRecharging && Health < DefaultHealth )
			{
				RechargingHealthAmount = FMin(RechargingHealthAmount + (DeltaTime * (HealthRechargePercentPerSecond * 0.01f) * DefaultHealth), DefaultHealth);
				Health = int(RechargingHealthAmount);
			}

			// see if we need to put our shield down
			if (IsDoingSpecialMove(SM_RaiseShieldOverHead))
			{
				if ( WorldInfo.TimeSeconds > (LastAttemptedHailDamageTime + class'GSM_RaiseShieldOverHead'.default.TimeSpentCoveringHead) )
				{
					ServerEndSpecialMove();
				}
			}
		}
	}
	else
	{
		StopFiring();
	}

`if(`notdefined(FINAL_RELEASE))
	if (class'GearGRI'.default.bDebugShowDamage)
	{
		foreach WorldInfo.AllControllers(class'PlayerController',PC)
		{
			PC.AddDebugText(string(Health), self, 1.f, vect(0,0,64.f), vect(0.f,0.f,32.f), class'HUD'.default.WhiteColor, FALSE);
		}
	}
`endif
}

final native simulated protected function UpdateAutomaticHeadTracking();

/**
 * Simple function that forces the pawn from cover, sorting
 * out whether it's an AI or player.
 */
simulated function final BreakFromCover()
{
	local GearPC PC;

	if (MyGearAI != None)
	{
		MyGearAI.InvalidateCover();
	}
	else if (CoverType != CT_None)
	{
		PC = GearPC(Controller);
		if( PC != None )
		{
			PC.LeaveCover();
		}
	}
}


/** Return TRUE to prevent targeting */
final simulated function bool ShouldPreventTargeting()
{
	if( MyGearWeapon != None && MyGearWeapon.ShouldPreventTargeting() )
	{
		return TRUE;
	}

	if( bIsConversing )
	{
		return TRUE;
	}

	return FALSE;
}

/** Return TRUE to force targeting */
final simulated function bool ShouldForceTargeting()
{
	if( MyGearWeapon != None && MyGearWeapon.ShouldForceTargeting() )
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Set Pawn TargetingMode flag (affecting animations, camera and other things...)
 * The controller calls this on the server.
 */
final function SetTargetingMode(bool bNewTargetingMode)
{
	if( bIsTargeting != bNewTargetingMode )
	{
		bIsTargeting	= bNewTargetingMode;

		// Force replication now to clients
		bForceNetUpdate = TRUE;

		// Notification that targeting mode has been changed.
		TargetingModeChanged();
	}
}


/**
 * Notification called when targeting mode has been changed.
 * Network: ALL.
 */
final simulated function TargetingModeChanged()
{
	local GearPC PC;

	// If stopped targeting, force Pawn into aiming stance for a little while
	if( !bIsTargeting )
	{
		SetWeaponAlert(AimTimeAfterFiring);
	}

	// Notify the weapon that targeting mode has been changed.
	if( MyGearWeapon != None )
	{
		MyGearWeapon.TargetingModeChanged(Self);
	}

	// notify the hud about the change
	if( Controller != None )
	{
		PC = GearPC( Controller );
		if( (PC != None) && (PC.myHUD != None) )
		{
			GearHUD_Base(PC.myHUD).TargetingModeChanged(bIsTargeting);
		}
	}

	// See if we should mount/unmount heavy weapons.
	CheckHeavyWeaponMounting();

	// Check if shield should be deployed or not
	CheckShieldDeployment();

	if (bIsTargeting)
	{
		if (IsTimerActive('BodyMove_ChangeTargeting'))
		{
			ClearTimer('BodyMove_ChangeTargeting');
		}
		else
		{
			BodyMove_ChangeTargeting();
		}
	}
	else
	{
		// gets a delay, since the anim is slow to blend
		SetTimer( 0.3f, FALSE, nameof(BodyMove_ChangeTargeting) );
	}
}

final simulated function BodyMove_ChangeTargeting()
{
	SoundGroup.PlayFoleySound(self, GearFoley_Body_Aim);
}

/** AimOffset Pct Interpolation. */
simulated native final function float AimInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed);

/** Origin used for AimOffset calculation. */
simulated event Vector GetAimOffsetOrigin()
{
	local Name					RightHandName;
	local Vector				VectorLoc, SocketLoc, X, Y, Z;
	local Rotator				SocketRot;
	local quat					BoneQuat;
	local GearWeap_HeavyBase	HeavyWeapon;

	// Heavy weapons work as turrets when mounted, so use pivot point as aimoffset origin.
	HeavyWeapon = GearWeap_HeavyBase(Weapon);
	if( HeavyWeapon != None && (HeavyWeapon.IsBeingMounted() || HeavyWeapon.IsMounted()) )
	{
		return Mesh.GetBoneLocation('b_MF_IK_Gun');
	}

	// Use weapon location as aimoffset origin, for better alignment.
	if( IsHumanControlled() && MyGearWeapon != None )
	{
		RightHandName = MyGearWeapon.GetDefaultHandSocketName(Self);
		if( RightHandName != '' )
		{
			// Try as a socket first
			if( Mesh.GetSocketWorldLocationAndRotation(RightHandName, SocketLoc, SocketRot) )
			{
				GetAxes(SocketRot, X, Y, Z);
				return SocketLoc + Z * 12.f;
			}

			// try as a bone reference next;
			if( Mesh.MatchRefBone(RightHandName) != INDEX_NONE )
			{
				BoneQuat = Mesh.GetBoneQuaternion(RightHandName);
				SocketRot = QuatToRotator(BoneQuat);
				GetAxes(SocketRot, X, Y, Z);
				return Mesh.GetBoneLocation(RightHandName) + Z * 12.f;
			}
		}
	}

	if( PelvisBoneName != '' && Mesh.MatchRefBone(PelvisBoneName) != INDEX_NONE )
	{
		VectorLoc = Mesh.GetBoneLocation(PelvisBoneName);
		VectorLoc.Z = GetPawnViewLocation().Z;

		return VectorLoc;
	}

	// Otherwise take default view location
	return Location;
}

/** Native implementation of base AimDir update. */
simulated native function UpdateMeshBoneControllersNative(float DeltaTime);

/**
 * Calculate AimDir 2d vector, used by all Aim AnimNodes.
 * Network: ALL
 */
simulated function UpdateMeshBoneControllers(float DeltaTime)
{
	UpdateMeshBoneControllersNative(DeltaTime);
}

/**
 * Update WeaponAimIKPositionFix vector.
 * used to offset the weapon when the character is in "downsights" animation.
 * weapons have different shapes, and characters different sizes.
 * So we fix here the position of the weapon to prevent any clipping, and ensure character looks like he's properly aiming.
 */
simulated event FixWeaponAimIKPosition(float DeltaTime)
{
	local int		i;
	local vector	TargetWeaponAimIKPositionFix;

	// Interpolate WeaponAimIKPositionFix
	TargetWeaponAimIKPositionFix = GetWeaponAimIKPositionFix();

	// Do Interpolation
	WeaponAimIKPositionFix = VInterpTo(WeaponAimIKPositionFix, TargetWeaponAimIKPositionFix, DeltaTime, 8.f);

	// Do not overwrite IKHackNode if there's no offset, that way we can play with it in the editor.
	if( VSizeSq(WeaponAimIKPositionFix) > 0 )
	{
		// Update IKHackNode
		for(i=0; i<IKHackNode.BoneCopyArray.Length; i++)
		{
			IKHackNode.BoneCopyArray[i].PositionOffset = WeaponAimIKPositionFix;
		}
		// Do mirrored version
		for(i=0; i<IKHackNode.BoneCopyArrayMirrored.Length; i++)
		{
			IKHackNode.BoneCopyArrayMirrored[i].PositionOffset = WeaponAimIKPositionFix;
		}
	}
}

/**
 * @see FixWeaponAimIKPosition for details.
 * Return relative offset from right hand.
 */
simulated function vector GetWeaponAimIKPositionFix()
{
	// Aiming position fix for new HammerBurst mesh.
	if( MyGearWeapon.IsA('GearWeap_LocustAssaultRifle') || MyGearWeapon.IsA('GearWeap_Boomshot') )
	{
		return vect(0, -1, -4);
	}

	return vect(0,0,0);
}

/** Update AimOffset for this Pawn (interpolation and 360 aiming). */
final simulated native function UpdateAimOffset(Vector2d NewAimOffsetPct, FLOAT DeltaTime);

/**
 * Function to test if player would be in 360 aiming given the following parameters.
 * @param	bCurrentlyDoing360Aiming	TRUE if Pawn is currently in 360 aiming. (Affects thresholds).
 * @param	TestAimOffsetPct			Pawn's AimOffsetPct.
 * @param	TestCoverType				Type of Cover Pawn would be in.
 * @param	out_CoverDirection			returns which direction the Pawn would be facing (mirrored or not)
 * @return								TRUE if Pawn would be in 360aiming or FALSE if not.
 */
final simulated native function bool Simulate360Aiming(bool bCurrentlyDoing360Aiming, Vector2D TestAimOffsetPct, ECoverType TestCoverType, out ECoverDirection out_CoverDirection);

/** 360 <-> leaning transitions */
final simulated native function bool IsDoing360ToLeaningTransition();
final simulated native function bool IsDoingLeaningTo360Transition();

/** Notification called when bDoing360Aiming flag changes */
simulated event On360AimingChangeNotify()
{
	// Last time we changed 360 aiming status
	Last360AimingChangeTime = WorldInfo.TimeSeconds;

	// If doing 360 aiming, then go to default cover action.
	if( bDoing360Aiming )
	{
		SetCoverAction(CA_Default);
	}

	// Update our flag of what our last status was.
	bWasDoing360Aiming = !bDoing360Aiming;

	// Forward notification to weapon
	if( MyGearWeapon != None )
	{
		MyGearWeapon.On360AimingChangeNotify();
	}
}


/**
 * returns base Aim Rotation without any adjustment (no aim error, no autolock, no adhesion.. just clean initial aim rotation!)
 *
 * @return	base Aim rotation.
 */
simulated event Rotator GetBaseAimRotation()
{
	local rotator AimRot, DeltaRot;
	local GearPC PC;
	local vector2d NewAimOffsetPct;
	local Rotator TestDeltaRot;


	// While doing melee, aim in front, not what camera looks at.
	if( bDoingMeleeAttack )
	{
		AimRot = Rotation;
		if (Controller != None)
		{
			AimRot.Pitch = Controller.Rotation.Pitch;
		}
		return AimRot;
	}
	else
	{
		PC = GearPC(Controller);
		if ( (PC != None) && (PC.PlayerCamera != None) && ( (PC.PlayerCamera.CameraStyle == 'Fixed') || PC.bDebugFaceCam ) )
		{
			return PC.Rotation;
		}
	}

	AimRot = Super.GetBaseAimRotation();

	if( IsHumanControlled() && GearWeapon(Weapon) != None )
	{
		AimRot += GearWeapon(Weapon).GetHumanAimRotationAdjustment();
	}

	// clamp blindfiring pitch/yaw
	if (CoverType != CT_None && IsBlindFiring(CoverAction))
	{
		// if we're going to be in 360 aiming, we don't want to clamp, which could
		// prevent the pawn from actually doing the 360 aim
		TestDeltaRot = Normalize(AimRot - Rotation);
		// Convert DeltaRot to AimOffsetPct
		NewAimOffsetPct.X = TestDeltaRot.Yaw / 16384.f;
		NewAimOffsetPct.Y = TestDeltaRot.Pitch / 16384.f;

		if (!Simulate360Aiming(FALSE, NewAimOffsetPct, CoverType, CoverDirection))
		{
			// limit the pitch if blind firing up
			if (CoverAction == CA_BlindUp)
			{
				AimRot.Pitch = Max(-600,Normalize(AimRot).Pitch);
			}
			else
			{
				// otherwise limit the yaw
				DeltaRot = Normalize(AimRot - Rotation);
				//`log("pre yaw:"@AimRot.Yaw@DeltaRot.Yaw);
				if (CoverAction == CA_BlindLeft)
				{
					AimRot.Yaw -= DeltaRot.Yaw - Min(700,DeltaRot.Yaw);
				}
				else if (CoverAction == CA_BlindRight)
				{
					AimRot.Yaw -= DeltaRot.Yaw - Max(-700,DeltaRot.Yaw);
				}
				//`log("post yaw:"@AimRot.Yaw@DeltaRot.Yaw);
			}
		}
	}
	return AimRot;
}


/**
 * Adjust Pawn Damage
 * - Adjust based on cover (makes cover safe)
 * - Adjust if Pawn is protected by a hostage.
 * - Vehicle damage adjustment
 */
function AdjustPawnDamage
(
				out	int					Damage,
					Pawn				InstigatedBy,
					Vector				HitLocation,
				out Vector				Momentum,
					class<GearDamageType>	GearDamageType,
	optional	out	TraceHitInfo		HitInfo
)
{
	local bool	bIsMultiplayerGame;
	local bool	bIsAIControlled;
	local int	Index;

	bIsMultiplayerGame = WorldInfo.GRI.IsMultiplayerGame();
	bIsAIControlled = AIController(Controller) != None;

	// scale by the current special move
	if( IsDoingASpecialMove() )
	{
		// Special case for Interactions. Only InteractionPawn is allowed to damage me.
		if( SpecialMoves[SpecialMove].bOnlyInteractionPawnCanDamageMe && InstigatedBy != InteractionPawn && InstigatedBy != Self )
		{
			Damage = 0;
			return;
		}

		Damage *= SpecialMoves[SpecialMove].default.DamageScale;
	}

	// If this pawn has a hostage, reduce frontal damage
	// Also if hostage has just been released this frame, consider Kidnapper still protected.
	// For Splash damage weapons, Hostage may just have been killed.
	if( IsAKidnapper() || (HostageReleaseTime == WorldInfo.TimeSeconds) )
	{
		Damage = ReduceKidnapperDamage(GearDamageType, Momentum, InstigatedBy, HitLocation, Damage, HitInfo);
	}
	else if (IsCarryingShield())
	{
		Damage = ReduceDamageForShieldHolder(Momentum, GearDamageType, HitInfo, Damage);
	}

	// if we've eliminated damage don't bother reducing it further
	if (Damage <= 0)
	{
		return;
	}

	if (!bIsMultiplayerGame || WorldInfo.GRI.IsCoopMultiplayerGame())
	{
		// offer chainsaw protection to players in SP/CT games
		if (!bIsAIControlled && (SpecialMove == SM_ChainSawAttack || IsChainsawDueling()))
		{
			Damage = 0;
			return;
		}
		if (bIsAIControlled && GearAI_TDM(Controller) == None)
		{
			Index = PerDamageTypeModifiers.Find('DamageTypeName', GearDamageType.Name);
			if (Index != INDEX_NONE)
			{
				Damage *= PerDamageTypeModifiers[Index].Multiplier;
			}
		}
	}

	// handle cover protection
	if (!GearDamageType.default.bIgnoreCover && IsInCover())
	{
		// for multiplayer we use a less generous approach to cover protection, unless it was from an AI shot
		if (bIsMultiplayerGame && !WorldInfo.GRI.IsCoopMultiplayerGame())
		{
			Damage = MultiplayerCoverDamageAdjust(Momentum,InstigatedBy,HitLocation,Damage);
		}
		else
		{
			// for singleplayer, if they're in cover then eliminate the damage entirely
			if (IsProtectedByCover(Momentum))
			{
				// if it's an AI blind firing then only reduce damage, otherwise nullify the damage
				Damage *= (IsBlindFiring(CoverAction) && !IsPlayerOwned()) ? 0.5f : 0.f;
				Momentum = vect(0,0,0);
			}
		}
	}

	// Driven Vehicle
	if( DrivenVehicle != None )
	{
		DrivenVehicle.AdjustDriverDamage(Damage, InstigatedBy != None ? InstigatedBy.Controller : None, HitLocation, Momentum, GearDamageType);
	}
}


/** Reduce Kidnapper damage when he's holding a hostage. */
final function float ReduceKidnapperDamage(class<GearDamageType> GearDamageType, vector ShotDirection, Pawn InstigatedBy, vector HitLocation, float Damage, const out TraceHitInfo HitInfo)
{
	local bool		bExplosiveDamage;
	local vector	Dir;
	local float		DotAngle, Factor;

	if (WorldInfo.GRI.IsMultiplayerGame())
	{
		// meatflag doesn't provide normal damage protection
		if (InteractionPawn != None && InteractionPawn.IsA('GearPawn_MeatFlagBase'))
		{
			return Damage;
		}
		// zero damage if they hit the meatshield or weapon hand
		if (HitInfo.HitComponent.Owner != self || HitInfo.BoneName == GetRightHandSocketName())
		{
			return 0;
		}
	}

	// Is this explosive damage?
	bExplosiveDamage = ClassIsChildOf(GearDamageType, class'GDT_Explosive');

	// don't protect against headshots
	if( bLastHitWasHeadShot && !bExplosiveDamage )
	{
		return Damage;
	}

	Dir	= -Normal(ShotDirection);
	DotAngle = Dir dot vector(Rotation);

	// If being shot from the front
	if( DotAngle > 0.f )
	{
		Factor = FClamp(Square(1.f - DotAngle) * (90.f / KidnapperHalfAngleOfProtection), 0.f, 1.f);
		//`log(WorldInfo.TimeSeconds @ Self$"::"$GetStateName()$"::"$GetFuncName() @ "Factor:" @ Factor @ "Orig Damage:" @ Damage @ "Reduced Damage:" @ (Damage*Factor));

		// Reduce damage
		return Damage * Factor;
	}

	return Damage;
}


/** Reduce Kidnapper damage when he's holding a hostage. */
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

	// for explosive damage types, cut damage.  ballistic damage types are handled by colliding the traces
	// with the shield mesh.
	if ( ClassIsChildOf(DamType, class'GDT_Explosive') || ClassIsChildOf( DamType, class'GDT_Fire' ) || ClassIsChildOf( DamType, class'GDT_Melee' ) )
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

/**
 * Modify damage based on cover in MP
 */
simulated final function float MultiplayerCoverDamageAdjust( vector ShotDirection, Pawn InstigatedBy, vector HitLocation, float Damage )
{
	local vector2D	AngularDist;
	local vector	AxisX, AxisY, AxisZ, ShotStart;
	local bool		bIsInFront;
	local float dist;

	// Make sure player has valid cover, and is not leaning
	if( SpecialMove == SM_Run2MidCov ||
		SpecialMove == SM_Run2StdCov ||
		CoverType == CT_None ||
		CoverAction == CA_LeanLeft ||
		CoverAction == CA_LeanRight ||
		CoverAction == CA_PopUp )
	{
		return Damage;
	}

	GetAxes(Rotation, AxisX, AxisY, AxisZ);
	ShotDirection = -1.f * Normal(ShotDirection);
	bIsInFront = GetAngularDistance(AngularDist, ShotDirection, AxisX, AxisY, AxisZ );

	// if shot is from front and on same level
	if( bIsInFront ) //&& Abs(InstigatedBy.Location.Z - Location.Z) < GetCollisionHeight() )
	{
		// check shot's angle of attack against cover's orientation
		GetAngularDegreesFromRadians( AngularDist );
		//`log(GetFuncName()@AngularDist.X@CoverProtectionFOV.X@AngularDist.Y@`showvar(ShotDirection));
		// Use 20 instead of the 30 on CoverProtectionFOV.Y for MP
		if ( Abs(AngularDist.X) < CoverProtectionFOV.X && AngularDist.Y < 20 )
		{
			// no damage if side isn't visible
			if ( InstigatedBy != None )
			{
				ShotStart = HitLocation + VSize(InstigatedBy.Location - Location) * ShotDirection;

				if ( !FastTrace(Location + CylinderComponent.CollisionRadius * Normal(ShotDirection cross vect(0,0,1)), ShotStart)
					&& !FastTrace(Location - CylinderComponent.CollisionRadius * Normal(ShotDirection cross vect(0,0,1)), ShotStart) )
				{
					// smoothly interpolate damage if close up, if completely covered
					Dist = VSize(Location - InstigatedBy.Location);
					return Damage * FClamp((320 - Dist)/320,0,1);
				}
			}

			// smoothly interpolate damage if in cover
			return FMax(Abs(AngularDist.X)/CoverProtectionFOV.X, Abs(AngularDist.Y)/25) * Damage;
		}
	}

	return Damage;
}


/**
 * Returns TRUE if player is safe in cover from the shot direction.
 * This function decides if a shot should be ignored because the player is considered safely protected by its current cover.
 * Not used in MP.
 */
final simulated native function bool IsProtectedByCover( vector ShotDirection, optional bool bSkipPlayerCheck );


/**
 * Change depth priority group for mesh and attached meshes
 */
simulated function SetDepthPriorityGroup(ESceneDepthPriorityGroup NewDepthPriorityGroup)
{
	local int	Idx;
	local bool	bUseViewOwnerDepthPriorityGroup;

	// If we're a kidnapper or a hostage, ignore DepthPriorityGroup setting
	// As this is creating sorting issues because both actors are not owned by the same viewport.
	// We don't really care because you can't go in cover in this context.
	if( IsAHostage() || IsAKidnapper() )
	{
		NewDepthPriorityGroup = SDPG_World;
	}

	if( Mesh != None )
	{
		bUseViewOwnerDepthPriorityGroup = NewDepthPriorityGroup != SDPG_World;

		Mesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup,NewDepthPriorityGroup);
		if( (Weapon != None) && (Weapon.Mesh != None) )
		{
			Weapon.Mesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup,NewDepthPriorityGroup);
		}

		// any attachments
		for(Idx = 0; Idx < AttachSlots.Length; Idx++)
		{
			if( AttachSlots[Idx].StaticMesh != None )
			{
				AttachSlots[Idx].StaticMesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup,NewDepthPriorityGroup);
			}
			if( AttachSlots[Idx].SkeletalMesh != None )
			{
				AttachSlots[Idx].SkeletalMesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup,NewDepthPriorityGroup);
			}
		}

		// Shield
		if( EquippedShield != None && EquippedShield.Mesh != None )
		{
			EquippedShield.Mesh.SetViewOwnerDepthPriorityGroup(bUseViewOwnerDepthPriorityGroup ,NewDepthPriorityGroup);
		}
	}
}

simulated function OnCauseDamage(SeqAct_CauseDamage Action)
{
	// Don't allow damage to be adjusted
	bAllowDamageModification = FALSE;
	if(Action.Instigator == none)
	{
		Action.Instigator = self;
	}
	Super.OnCauseDamage( Action );
	bAllowDamageModification = TRUE;
}

function CrushedBy(Pawn OtherPawn);

function bool ConfineToClampedBase()
{
	local InterpActor_GearBasePlatform PlatBase;

	// If no longer on our clamped base
	if( ClampedBase != None && Base != ClampedBase )
	{
		PlatBase = InterpActor_GearBasePlatform(Base);

// 		`DLog("Base:" @ Base @ "ClampedBase:" @ ClampedBase @ "Base.Base:" @ Base.Base @ "ClampedBase.Base:" @ ClampedBase.Base @ "PlatBase:" @ PlatBase);

		// If not moving onto another platform OR
		// If not moving onto a platform that is our current platforms base or shares a base (used for torture barges)
		if( PlatBase == None ||
			PlatBase.bAlwaysConfineToClampedBase ||
			( PlatBase.Base != None &&
			  PlatBase.Base != ClampedBase &&
			  PlatBase.Base != ClampedBase.Base ) )
		{
			// Confine the pawn to our original clamped platform
			return TRUE;
		}
	}
	return FALSE;
}

event BaseChange()
{
	local bool	bBasedOnInteractionPawn;

	// See if Pawn is based on his interaction pawn (eg. hostage)
	bBasedOnInteractionPawn = InteractionPawn != None && IsDoingASpecialMove() && (Base == InteractionPawn);

	// if necessary, force the Pawn to stay on its old Base
	if( ClampedBase != None && ClampedBase != Base && (bJustTeleported || Physics == PHYS_RigidBody) )
	{
// 		`DLog("Clear ClampedBase:" @ ClampedBase @ "cleared. New Base:" @ Base @ "SpecialMove" @ SpecialMove @ "Physics:" @ Physics @ "bJustTeleported:" @ bJustTeleported);
		ClampedBase = None;
	}

// 	`DLog("ClampedBase:" @ ClampedBase @ "NewBase:" @ Base @ "bHardAttach:" @ bHardAttach @ "bBasedOnInteractionPawn:" @ bBasedOnInteractionPawn @ "ConfineToClampedBase:" @ ConfineToClampedBase());

	// if necessary, force the Pawn to stay on its old Base
	// Don't override Pawns doing a special move which can be forced to be based on their InteractionPawn
	if( ClampedBase != None && !bHardAttach && !bBasedOnInteractionPawn && ConfineToClampedBase() )
	{
// 		`DLog("Restore ClampedBase:" @ ClampedBase @ "instead of keeping NewBase:" @ Base );

		SetBase(ClampedBase);
		if( Physics == PHYS_Falling && Controller != None )
		{
			bCanJump = true;
			Controller.MayFall();
			if (!bCanJump)
			{
				SetPhysics(PHYS_Walking);
			}
		}
	}
	else
	{
// 		`DLog("Calling Super::BaseChange");
		Super.BaseChange();
	}
}

function JumpOffPawn()
{
	local Pawn OtherP;
	OtherP = Pawn(Base);
	// don't actually jump if not really above or inside the other Pawn
	// this fixes some cases where the collidable geometry of the other Pawn changes (e.g. the shield)
	// causing us to think we need to jump off of it
	if (OtherP != None && VSize2D(Location - Base.Location) >= (CylinderComponent.CollisionRadius+OtherP.CylinderComponent.CollisionRadius))
	{
		SetPhysics(PHYS_Falling);
	}
	else
	{
		Velocity += (100 + CylinderComponent.CollisionRadius) * Vector(Rotation);
		if ( VSize2D(Velocity) > FMax(500.0, GroundSpeed) )
		{
			Velocity = FMax(500.0, GroundSpeed) * Normal(Velocity);
		}
		Velocity.Z = 50 + CylinderComponent.CollisionHeight;
		SetPhysics(PHYS_Falling);
	}
}

/** Register a near miss hit */
function RegisterNearHitMiss(ImpactInfo Impact, class<DamageType> InDamageType)
{
	local CringeInfo	OutCringeInfo;
	local FLOAT			ChanceToPlayCringe;

	// Enforce a minimum delay between hits
	if( TimeSince(LastShotAtTime) < 0.2f )
	{
// 		`DLog("Was last shot at" @ TimeSince(LastShotAtTime) @ "seconds ago. So Skipping:" @ InDamageType);
		return;
	}

	// Clear NearHitHistory
	ClearCringeHistory(NearHitHistory, 5.f);
	// We keep a history of cringes for 10 seconds
	ClearFLOATArrayHistory(CringeHistory, 10.f);

	// Keep track of shots through LastShotAtTime;
	ShotAtCount++;
	ShotAtCountIncreased();

	// First case is for impacts hapening really close to the player
	//if( Impact.HitActor != None && VSize(Impact.HitLocation - Location) <= 200 )
	if( TRUE )
	{
		// Keep our history up to date.
		AddCringeHistoryTime(NearHitHistory, InDamageType);
		GetCringeInfo(NearHitHistory, InDamageType, OutCringeInfo);

		// If this is the first time this damage type is triggered for this history,
		// or if not cringes have been played recently... go ahead, Trigger it!
		if( OutCringeInfo.History.Length == 1 /*|| CringeHistory.Length == 0*/ )
		{
			TryPlayNearMissCringe();
		}
		// If this damage type has been repeated then see if we should trigger another cringe for it
		else
		{
			// Chance to play this cringe is decreased by the number of recent cringes we've done and the number of times we've been shot at by this damage type.
			ChanceToPlayCringe = 1.f / (1.f + float(CringeHistory.Length) + float(OutCringeInfo.History.Length) / 4.f);
// 			`DLog("ChanceToPlayCringe:" @ ChanceToPlayCringe @ "CringeHistory:" @ CringeHistory.Length @ "CringeInfoHistory" @ OutCringeInfo.History.Length);
			if( ChanceToPlayCringe > 0.2f && (FRand() < ChanceToPlayCringe) )
			{
				TryPlayNearMissCringe();
			}
		}

// 		WorldInfo.AddOnScreenDebugMessage(0, 5.f, MakeColor(160,160,160,255), "CringeHistory:" @ CringeHistory.Length @ "CringeInfoHistory:" @ OutCringeInfo.History.Length );
	}
}



/**
 * Called when ShotAtCount is increased, and sets ShotAtTime to the current time.
 * Doing it this way because time is different between client and servers.
 */
simulated function ShotAtCountIncreased()
{
	LastShotAtTime = WorldInfo.TimeSeconds;
}

exec function TestShotAt()
{
	// Keep track of shots through LastShotAtTime;
	ShotAtCount++;
	ShotAtCountIncreased();
}

exec function TestCringe()
{
	TryPlayNearMissCringe();
}

simulated function bool TryPlayNearMissCringe()
{
	// Limit the frequency of those guys
	if( TimeSince(LastCringeTime) > 0.3f )
	{
		if( PlayNearMissCringe() )
		{
			CringeHistory[CringeHistory.Length] = WorldInfo.TimeSeconds;
			LastCringeTime = WorldInfo.TimeSeconds;

			// Replication
			if( Role == ROLE_Authority )
			{
				CringeCount++;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/** Replicated and called when it's time to play a Near Miss Cringe Reaction! */
simulated function CringeCountIncreased()
{
	// Near Miss hapened, try to play a cringe overlay animation.
	TryPlayNearMissCringe();
}

/** By default, play nothing. */
simulated function bool PlayNearMissCringe()
{
	return FALSE;
}

simulated final function AddCringeHistoryTime(out Array<CringeInfo> InHistory, class<DamageType> InDamageType)
{
	local INT			Index;
	local CringeInfo	NewCringeInfo;

	// See if we can find an existing entry
	for(Index=0; Index<InHistory.Length; Index++)
	{
		if( InHistory[Index].DamageType == InDamageType )
		{
			InHistory[Index].History[ InHistory[Index].History.Length ] = WorldInfo.TimeSeconds;
			return;
		}
	}

	// If none found, create a new entry
	NewCringeInfo.DamageType = InDamageType;
	NewCringeInfo.History[0] = WorldInfo.TimeSeconds;
	NewCringeInfo.NextCringeTime = WorldInfo.TimeSeconds;
	InHistory[InHistory.Length] = NewCringeInfo;
}

/**
 * Clear anything that's older than LifeTime seconds.
 */
simulated final function ClearCringeHistory(out Array<CringeInfo> InHistory, FLOAT LifeTime)
{
	local INT	HistoryIdx, TimeIdx;
	local FLOAT	CringeTime;

	for(HistoryIdx=InHistory.Length-1; HistoryIdx>=0; HistoryIdx--)
	{
		for(TimeIdx=InHistory[HistoryIdx].History.Length-1; TimeIdx>=0; TimeIdx--)
		{
			CringeTime = TimeSince(InHistory[HistoryIdx].History[TimeIdx]);
			if( CringeTime >= LifeTime || CringeTime <= 0 )
			{
				InHistory[HistoryIdx].History.Remove(0, TimeIdx+1);
				break;
			}
		}

		// If history is empty for this damage type, just get rid of this entry...
		if( InHistory[HistoryIdx].History.Length == 0 )
		{
			InHistory.Remove(HistoryIdx, 1);
		}
	}
}

/**
 * Clear anything that's older than LifeTime seconds.
 */
simulated final function ClearFLOATArrayHistory(out Array<FLOAT> History, FLOAT LifeTime)
{
	local INT	TimeIdx;
	local FLOAT	KeyTime;

	for(TimeIdx=History.Length-1; TimeIdx>=0; TimeIdx--)
	{
		KeyTime = TimeSince(History[TimeIdx]);
		if( KeyTime >= LifeTime || KeyTime <= 0 )
		{
			History.Remove(0, TimeIdx+1);
			break;
		}
	}
}

simulated final function FLOAT GetLastCringeHistoryTime(out const Array<CringeInfo> InHistory, class<DamageType> InDamageType)
{
	local INT Index;

	for(Index=0; Index<InHistory.Length; Index++)
	{
		if( InHistory[Index].DamageType == InDamageType )
		{
			if( InHistory[Index].History.Length > 0 )
			{
				return InHistory[Index].History[ InHistory[Index].History.Length ];
			}
			break;
		}
	}

	return 0.f;
}

simulated final function FLOAT GetNextCringeTime(out const Array<CringeInfo> InHistory, class<DamageType> InDamageType)
{
	local INT	Index;

	for(Index=0; Index<InHistory.Length; Index++)
	{
		if( InHistory[Index].DamageType == InDamageType )
		{
			return InHistory[Index].NextCringeTime;
		}
	}

	return WorldInfo.TimeSeconds;
}

simulated final function bool GetCringeInfo(out const Array<CringeInfo> InHistory, class<DamageType> InDamageType, out CringeInfo OutCringeInfo)
{
	local INT Index;

	for(Index=0; Index<InHistory.Length; Index++)
	{
		if( InHistory[Index].DamageType == InDamageType )
		{
			OutCringeInfo = InHistory[Index];
			return TRUE;
		}
	}

	return FALSE;
}


final function bool IsRespawnInvincible()
{
    return GearPC(Controller) != None && GearPC(Controller).bIsRespawnInvincible;
}

/**
 * Overridden for GoW's specific damage taking needs, whee!
 *
 * @see Actor::TakeDamage
 */
function TakeDamage
(
				int					Damage,
				Controller			InstigatedBy,
				Vector				HitLocation,
				Vector				Momentum,
				class<DamageType>	DamageType,
	optional	TraceHitInfo		HitInfo,
	optional	Actor				DamageCauser
)
{
	local Pawn					InstigatedByPawn;
	local GearPRI				InstigatorPRI, PRI;
	local GearGRI				GRI;
	local class<GearDamageType>	GearDamageType;
`if(`notdefined(FINAL_RELEASE))
	local PlayerController		PC;
`endif
	local float                 DmgDistance;
	local int ChainsawInterrupt;
	local gearpawn_locusttickerbase TickerDamager;

	// make sure we are the server and we're alive
	if (Role < ROLE_Authority || Health <= 0 || InGodMode() )
	{
		return;
	}

	// prevent pain volumes and such from killing us before we have a chance to initialize health values
	if (Controller == None && CreationTime == WorldInfo.TimeSeconds)
	{
		return;
	}

	// make sure the we are not invincible from respawning
	// Note: this is not done in AdjustPawnDamage because outside factors (like being in MP or being
	// hit by an explosion) will negate the adjustment and we need to make sure this happens.
	if ( IsRespawnInvincible() )
	{
		if ( PlayerController(InstigatedBy) != None )
		{
			PlayerController(InstigatedBy).ClientPlaySound(SoundCue'Interface_Audio.Interface.BurstDamageWarning01Cue');
		}
		return;
	}

	GRI = GearGRI(WorldInfo.GRI);
	PRI = GearPRI(PlayerReplicationInfo);

	GearDamageType = class<GearDamageType>(DamageType);
	// if passed an invalid damagetype then use the default
	if( GearDamageType == None )
	{
		GearDamageType = class'GearDamageType';
	}
	// cache the instigator parms
	InstigatedByPawn = (InstigatedBy != None) ? InstigatedBy.Pawn : None;
	InstigatorPRI = (InstigatedBy != None) ? GearPRI(InstigatedBy.PlayerReplicationInfo) : None;

	// if the hit location is invalid then adjust to base location
	if (IsZero(HitLocation))
	{
		HitLocation = GetBaseTargetLocation();
	}

	// limit the size of the momentum
	if( VSize(Momentum) > 4.f )
	{
		Momentum = Normal(Momentum);
	}

	// verify the hit info is valid
	//@fixme this if statement shouldn't be necessary once the real boomershield asset is in, since it's a skeletal mesh.
	// seems kinda fragile code though, to try and enforce skelmesh hits
//	if ( !ComponentIsAnEquippedShield(HitInfo.HitComponent) )
//	{
		CheckHitInfo(HitInfo, Mesh, Momentum, HitLocation);
//	}

	// remember the last time we were shot at
	ShotAtCountIncreased();

	// reset physics if necessary
	// (do not change when in special move - special move will reset when it's ready)
	if( Physics == PHYS_None && DrivenVehicle == None && !IsDoingASpecialMove() )
	{
		SetMovementPhysics();
	}
	// if we're allowed to modify damage
	if( bAllowDamageModification )
	{
		//`log("pre-mod:"@Damage);
		// first allow the damage type a first-pass modifier
		GearDamageType.static.ModifyDamage(self, InstigatedBy, Damage, HitInfo, HitLocation, Momentum);
		//`log("post GDT mod:"@Damage);
		// if the damage wasn't eliminated
		if (Damage != 0)
		{
			// adjust damage based on various stuff (generally just cover)
			AdjustPawnDamage( Damage, InstigatedByPawn, HitLocation, Momentum, GearDamageType, HitInfo);
			//`log("post APD mod:"@Damage);
			// and if damage still wasn't eliminated
			if (Damage != 0)
			{
				// let ticker mess with damage if we're taking damage from him
				if( DamageCauser != None && GearDamageType == class'GDT_TickerExplosion')
				{
					TickerDamager = gearpawn_locusttickerbase(DamageCauser.Owner);

					if(TickerDamager != none)
					{
						TickerDamager.AdjustTickerDamage(Damage,self);
					}
				}

				// notify the game info of the damage, possibly scaling the values
				// - this handles stuff like god mode, friendly fire, etc
				WorldInfo.Game.ReduceDamage(Damage, Self, InstigatedBy, HitLocation, Momentum, GearDamageType);
				//`log("post Game mod:"@Damage);
			}
		}
	}

`if(`notdefined(FINAL_RELEASE))
	if (bDebugDamageText || class'GearGRI'.default.bDebugShowDamage)
	{
		foreach WorldInfo.AllControllers(class'PlayerController',PC)
		{
			PC.AddDebugText(string(Damage),self,5.f,vect(0,0,64.f) ,vect(64.f,64.f,256.f) * VRand(),class'HUD'.default.RedColor,TRUE);
		}
	}
`endif

	// if any damage is actually generated
	if( Damage != 0 )
	{
		// stop any health recharge
		ResetHealthRecharge();

		// call Actor's version to handle any SeqEvent_TakeDamage for scripting
		Super(Actor).TakeDamage( Damage, InstigatedBy, HitLocation, Momentum, GearDamageType, HitInfo, DamageCauser );

		// Log the hit, but only if the round is in progress
		if (GRI.GameStatus == GS_RoundInProgress)
		{
			PRI.ScoreHit(InstigatorPRI, DefaultHealth, Health, Damage, GearDamageType);
		}

		if ( (Damage >= Health) && !bUnlimitedHealth )
		{
			// will die, kick off of turret.
			// note we cannot do this after Health is decremented, because controller won't repossese pawn proprely (see vehicle.driverleave())
			KickPawnOffTurret();
		}

		// apply the damage
		Health -= Damage;

		if( (Controller != None)
			&& Controller.IsLocalPlayerController()
			&& ( Health < (0.40f*DefaultHealth) )
			&& ( !PlayBurstDamageWarningSoundSemaphore )
			)
		{
			PlayerController(Controller).ClientPlaySound(SoundCue'Interface_Audio.Interface.BurstDamageWarning01Cue');
			PlayBurstDamageWarningSoundSemaphore = TRUE;
			SetTimer( 4.0, FALSE, nameof(ResetPlayBDWarningSoundSemaphore) );
		}

		// play any associated fx/sounds
		DoDamageEffects(Damage, InstigatedByPawn, HitLocation, GearDamageType, Momentum, HitInfo);

		// allow the damage type to add any extra fx/etc
		GearDamageType.static.HandleDamagedPawn(self, InstigatedByPawn, Damage, Momentum);

		// handle unlimited health cheat
		if ( (Health <= 0) && bUnlimitedHealth )
		{
			Health = 1;
		}

		// if we are now dead,
		if( Health <= 0 )
		{
			// if we should DBNO
			if( AllowedToDBNO() && (GearDamageType.static.ShouldDBNO(self, InstigatedByPawn, HitLocation, HitInfo) || ShouldForceDBNO(GearDamageType,InstigatedByPawn,HitLocation)) )
			{
				// Send Pawn to DBNO
				EnterDBNO(InstigatedBy, GearDamageType);
			}
			// Kill
			else
			{
				TearOffMomentum	= Momentum;
				if (InstigatorPRI != None)
				{
					// track the kill for the instigator
					InstigatorPRI.ScoreInstantKill(PRI, GearDamageType, bLastHitWasHeadShot ? GDT_HEADSHOT : GDT_NORMAL);
				}
				else if (ClassIsChildOf(GearDamageType, class'GDT_Environment') || GearDamageType == class'GDT_Environment')
				{
					// see if it was environment damage and pass it on to the death messages
					PRI.ScoreDeath(None, GearDamageType, GDT_NORMAL, -100);
				}
				// otherwise die and let PlayDying() sort out gib/ragdoll
				Died(InstigatedBy, damageType, HitLocation);
			}
		}
		else
		{
			// keep track of last time we received damage
			LastTookDamageTime = WorldInfo.TimeSeconds;
			// keep track of who the last person was that damaged us
			LastHitBy = (InstigatedBy != None && InstigatedBy != Controller) ? InstigatedBy : LastHitBy;

			// Handle Stopping Power
			if(bAllowStoppingPower)
			{
				DmgDistance = (InstigatedByPawn != None ? VSize(InstigatedByPawn.Location - Location) : (DamageCauser != None ? VSize(DamageCauser.Location - Location) : 0.f));
				if( GearDamageType.default.StoppingPower > 0.f && DmgDistance < MaxStoppingPowerDistance)
				{
					HandleStoppingPower(GearDamageType, Momentum, DmgDistance);
				}
			}

			// check for chainsaw interrupt
			DamageCauser = DamageCauser != None ? DamageCauser : InstigatedByPawn;
			if (ChainsawInterruptThreshold > 0 && Normal(Location - DamageCauser.Location) dot vector(Rotation) < 0.f)
			{
				ChainsawInterrupt = GearDamageType.static.GetChainsawInterruptValue(Damage);
				ApplyChainsawInterrupt(ChainsawInterrupt);
			}

			// notify the AI of being hit
			if (MyGearAI != None)
			{
				MyGearAI.GearAINotifyTakeHit(InstigatedBy, HitLocation, Damage, GearDamageType, Momentum, HitInfo);
			}
			if( DrivenVehicle != None &&
				GearAI(DrivenVehicle.Controller) != None )
			{
				GearAI(DrivenVehicle.Controller).GearAINotifyTakeHit(InstigatedBy, HitLocation, Damage, GearDamageType, Momentum, HitInfo);
			}
		}
	}

	// fire off any gud events associated with damage.  it's ok if damage is 0 here.
	TriggerDamageGUDEvent(Damage, InstigatedByPawn, GearDamageType);

	// make noise for AI purposes
	MakeNoise(1.f,'TakeDamage');
}

//`define chainsawlog(msg)	`log(`msg)
`define chainsawlog(msg)

final protected function ApplyChainsawInterrupt(int Amt)
{
	`chainsawlog(WorldInfo.TimeSeconds@self@GetFuncName()@`showvar(Amt)@`showvar(ChainsawInterruptAmount));
	// if not already interrupted
	if (!IsChainsawInterrupted())
	{
		ChainsawInterruptAmount += Amt;
		if (ChainsawInterruptAmount >= ChainsawInterruptThreshold)
		{
			ChainsawInterruptAmount = ChainsawInterruptThreshold;
			// trigger an event to interrupt current chainsaw special move if active
			ChainsawInterruptToggled(TRUE);
			// set a larger delay for the interrupt decay
			SetTimer( 2.f,FALSE,nameof(DecayChainsawInterrupt) );
		}
		else
		{
			// set/delay the decay
			if (GetRemainingTimeForTimer('DecayChainsawInterrupt') < 0.5f)
			{
				SetTimer( 0.5f,FALSE,nameof(DecayChainsawInterrupt) );
			}
		}
	}
	else
	{
		// reset the timer to a min of 1 second
		if (GetRemainingTimeForTimer(nameof(DecayChainsawInterrupt)) < 1.f)
		{
			SetTimer( 1.f,FALSE,nameof(DecayChainsawInterrupt) );
		}
	}
}

/** Double check we're allowed to actually DBNO */
final function bool AllowedToDBNO()
{
	local GearPC PC;
	PC = GearPC(Controller);
	if (GearGRI(WorldInfo.GRI).bIsCoop && GetTeamNum() == TEAM_COG && PC != None)
	{
		// check if we have any chance of surviving DBNO
		if (!GearGame(WorldInfo.Game).TeamHasAnyChanceOfSurvival(GetTeamNum(),self))
		{
			`log("no chance of survival, no dbno");
			return FALSE;
		}
		// special case for single player or coop splits
		if (!PC.IsActuallyPlayingCoop() || GearGame(WorldInfo.Game).bInCoopSplit)
		{
			// make sure the difficulty setting allows DBNO
			`log(`showvar(GearPRI(PlayerReplicationInfo).Difficulty)@`showvar(GearPRI(PlayerReplicationInfo).Difficulty.default.bCanDBNO));
			return GearPRI(PlayerReplicationInfo).Difficulty.default.bCanDBNO;
		}
	}
	return TRUE;
}

final protected function DecayChainsawInterrupt()
{
	`chainsawlog(WorldInfo.TimeSeconds@self@GetFuncName()@`showvar(ChainsawInterruptAmount));
	// if we were interrupted
	if (ChainsawInterruptAmount >= ChainsawInterruptThreshold)
	{
		// then clear the interrupt
		ChainsawInterruptAmount = 0;
		ChainsawInterruptToggled(FALSE);
	}
	else
	{
		// otherwise decay the interrupt amount
		ChainsawInterruptAmount = Max(0,ChainsawInterruptAmount - (ChainsawInterruptDecayPerSecond * 0.5f));
		// reset the timer if needed
		if (ChainsawInterruptAmount > 0)
		{
			SetTimer(0.5f,FALSE,nameof(DecayChainsawInterrupt));
		}
	}
}

final protected function ChainsawInterruptToggled(bool bInterrupted)
{
	`chainsawlog(WorldInfo.TimeSeconds@self@GetFuncName()@`showvar(bInterrupted)@`showvar(ChainsawInterruptAmount));
	if (bInterrupted)
	{
		// only interrupt the hold
		if (IsDoingSpecialMove(SM_ChainSawHold))
		{
			ForceStopMeleeAttack(TRUE);
			DoStumbleFromMeleeSpecialMove();
		}
		// notify client
		ClientChainsawInterruptToggled(bInterrupted);
	}
	else
	{
		// notify client
		ClientChainsawInterruptToggled(bInterrupted);
	}
}

reliable client function ClientChainsawInterruptToggled(bool bInterrupted)
{
	local GearPC PC;
	`chainsawlog(WorldInfo.TimeSeconds@self@GetFuncName()@`showvar(bInterrupted)@`showvar(ChainsawInterruptAmount));
	if (bInterrupted)
	{
		// simulate the interrupt value locally
		ChainsawInterruptAmount = ChainsawInterruptThreshold;
	}
	else
	{
		ChainsawInterruptAmount = 0;
		// check to see if they're still holding the button
		PC = GearPC(Controller);
		if (PC != None && PC.IsLocalPlayerController() && GearWeap_AssaultRifle(Weapon) != None)
		{
			if (PC.IsButtonActive(GB_B, FALSE))
			{
				StartMeleeAttack();
			}
		}
	}
}

final simulated function bool IsChainsawInterrupted()
{
	return (GearWeap_AssaultRifle(Weapon) != None && ChainsawInterruptAmount >= ChainsawInterruptThreshold);
}

/**
 *  Stopping Power
 */

function HandleStoppingPower(class<GearDamageType> GearDamageType, vector Momentum, float DmgDistance)
{
	// Don't apply stopping power if player has respawn protection
	if( !IsRespawnInvincible() )
	{
		// Add StoppingPower to the list.
		AddStoppingPower(GearDamageType, Momentum, DmgDistance);

		// Trigger stumbles when roadie running and being shot too much with stopping power.
		if( IsDoingSpecialMove(SM_RoadieRun) && GetResultingStoppingPower() > StoppingPowerRoadieRunStumbleThreshold && CanDoSpecialMove(SM_RoadieRunHitReactionStumble) )
		{
			ServerDoSpecialMove(SM_RoadieRunHitReactionStumble, TRUE);
		}
	}
}

final simulated function AddStoppingPower(class<GearDamageType> GearDamageType, Vector Momentum, float DmgDistance)
{
	local StoppingPowerStruct	NewStoppingPower;

	NewStoppingPower.StoppingPower = GearDamageType.default.StoppingPower * (1.f - (DmgDistance/MaxStoppingPowerDistance));
	NewStoppingPower.Direction = Normal(Momentum);
	NewStoppingPower.LifeTime = 1.f;
	StoppingPowerList[StoppingPowerList.Length] = NewStoppingPower;

	if( Role == ROLE_Authority && Controller != None && Controller.IsA('GearPC') && !IsLocallyControlled() )
	{
		ClientAddStoppingPower(GearDamageType, Momentum, DmgDistance);
	}
}

final simulated reliable client function ClientAddStoppingPower(class<GearDamageType> GearDamageType, Vector Momentum, float DmgDistance)
{
	if(IsLocallyControlled())
	{
		AddStoppingPower(GearDamageType, Momentum, DmgDistance);
	}
}

final simulated function ClearStoppingPower()
{
	StoppingPowerList.Length = 0;
}

final simulated function UpdateStoppingPower(FLOAT DeltaTime)
{
	local INT i;

	for(i=0; i<StoppingPowerList.Length; i++)
	{
		StoppingPowerList[i].LifeTime -= DeltaTime;
		if( StoppingPowerList[i].LifeTime <= 0.f )
		{
			 StoppingPowerList.Remove(i--, 1);
		}
	}
}

/** Returns the Resulting Stopping Power */
native final function FLOAT GetResultingStoppingPower();

simulated function DrawDebugStoppingPower(HUD H)
{
	local Canvas	Canvas;
	local String	Text;
	local FLOAT		XL, YL;
	local INT		i;

	Canvas = H.Canvas;

	Text = "AccumulatedStoppingPower:" @ GetResultingStoppingPower() @ "Velocity:" @ VSize(Velocity);
	Canvas.StrLen(Text, XL, YL);
	Canvas.SetPos((H.SizeX - XL)/2.f, YL * 2.f);
	Canvas.SetDrawColor(255,0,0);
	Canvas.DrawText(TEXT, FALSE);

	for(i=0; i<StoppingPowerList.Length; i++)
	{
		DrawDebugLine(Location, Location + StoppingPowerList[i].Direction * 50.f, BYTE(255.f * (1.f - StoppingPowerList[i].LifeTime)), BYTE(255.f * StoppingPowerList[i].LifeTime), 0, FALSE);
	}
}


/** Give the pawn a chance to override whether it should force a DBNO in a death situation or not */
function bool ShouldForceDBNO( class<GearDamageType> GearDmgType, Pawn InstigatedBy, vector HitLocation )
{
	return FALSE;
}

/** determines whether we would normally want to go DBNO, this is a handy way to turn DBNO on/off without breaking stuff that should never DBNO (e.g. explosions, headshots) */
function bool WantsToDBNO(Pawn InInstigator, vector HitLocation,TraceHitInfo HitInfo)
{
	// default doesn't allow AI to DBNO in SP or COOPMP
	if( !IsHumanControlled() )
	{
		// if this is SP or Horde, and we are not COG, then we only sometimes go DBNO
		if ( (!WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame()) &&
			  GetTeamNum() != TEAM_COG
		   )
		{
			if(!TookLegShot(HitInfo.BoneName) && FRand() > LocustChanceToDBNO)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

/** Give the pawn a chance to override whether it should Gib or not */
function bool ShouldGib(class<GearDamageType> GearDmgType, Pawn InstigatedBy)
{
	return TRUE;
}

/** Internal. */
protected function TriggerDamageGUDEvent(int Damage, Pawn InstigatedByPawn, class<GearDamageType> GearDamageType)
{
	if( InstigatedByPawn != self && IsSameTeam(InstigatedByPawn) )
	{
		if (InstigatedByPawn.IsHumanControlled() && ClassIsChildOf(GearDamageType, class'GDT_Ballistic'))
		{
			// note that we only throw this event for local players -- hearing the AI comment
			// on this when shopt by another AI is odd.
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_DamageTeammate, InstigatedByPawn, self);
		}
	}
	else
	{
		if (Damage > 0)
		{
			if( GearDamageType.default.DamageGUDEvent != GUDEvent_None )
			{
				// damtype-specific damaged event
				GearGame(WorldInfo.Game).TriggerGUDEvent(GearDamageType.default.DamageGUDEvent, InstigatedByPawn, self);
			}
			else if( Damage > 200 )			// threshold here is easily tweakable
			{
				// generic heavy damage event
				GearGame(WorldInfo.Game).TriggerGUDEvent(GearDamageType.default.DamageHeavyGUDEvent, InstigatedByPawn, self);
			}
		}
	}
}


/** Will reset the PlayBDWarningSoundSemaphore so we can play a sound again **/
function ResetPlayBDWarningSoundSemaphore()
{
	PlayBurstDamageWarningSoundSemaphore = FALSE;
}


/**
 * Overridden to handle GearPawn specifics.
 * @NOTE - we don't currently support the gameinfo preventing death (through a mutator etc),
 * so if that behavior is needed this code will have to be refactored.
 */
function bool Died(Controller Killer, class<DamageType> DamageType, vector HitLocation)
{
	local class<GearDamageType> GearDamageType;
	local GearPRI GP;

	if (Role != ROLE_Authority)
	{
`if(`notdefined(FINAL_RELEASE))
		`LogExt("This function shouldn't be called on clients!");
		ScriptTrace();
`endif
		return TRUE;
	}

	`LogSM("");

	//@STATS
	// Update Cover Stats if the Pawn is in cover when they died
	if (CoverType != CT_None)
	{
	 	GP = GearPRI(PlayerReplicationInfo);
		GP.TotalTimeInCover += WorldInfo.TimeSeconds - LastCoverTime;

	}

	GearDamageType = class<GearDamageType>(damageType);
	// ensure a default
	if (GearDamageType == None)
	{
`if(`notdefined(FINAL_RELEASE))
		`LogExt("Invalid gear damage type!"@damageType);
		ScriptTrace();
`endif
		GearDamageType = class'GearDamageType';
	}

	// drop our inventory
	DropExtraWeapons( DamageType );
	if (IsCarryingShield())
	{
		DropShield();
	}
	bHealthIsRecharging = FALSE; // stop health recharging upon death
	bBlocksNavigation	= FALSE; // stop blocking pathing on death

	// broadcast the death event and set the PRI death variables
	if ( GearDamageType != class'GDT_Hostage' )
	{
		SetDeathReplicationData( (Killer == None) ? None : GearPRI(Killer.PlayerReplicationInfo), GearPRI(PlayerReplicationInfo), GearDamageType );
	}

	// trigger the GUD event
	if (!bNoDeathGUDSEvent)
	{
		if (bLastHitWasHeadShot)
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_KilledEnemyHeadShot, (Killer==None)?None:Killer.Pawn, self);
		}
		else
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GearDamageType.default.KillGUDEvent, (Killer==None)?None:Killer.Pawn, self);
		}
	}

	// Remove attachment to our anchor
	// Allows bookkeeping - ie updates combat zone occupancy
	SetAnchor( None );

	if (Role == ROLE_Authority)
	{
		GearDamageType.static.HandleKilledPawn(self, (Killer==None)?None:Killer.Pawn);
	}

	// These are replicated to other clients
	HitDamageType		= damageType;
	TakeHitLocation		= HitLocation;

	if(GearPawn_LocustBloodmount(Base) != none)
	{
		BloodMountIDiedOn=GearPawn_LocustBloodmount(Base);
	}

	// normal death
	return Super.Died(Killer, damageType, HitLocation);
}

/** Sets the replication values dying and for who killed who */
final function SetDeathReplicationData( GearPRI KillerPRI, GearPRI VictimPRI, class<GearDamageType> DamageDiedFrom )
{
	if ( VictimPRI != None )
	{
		// Mark the last player to kill me, and mark the last player the instigator killed in his PRI
		VictimPRI.LastToKillMePRI = KillerPRI;
		VictimPRI.DamageTypeToKillMe = DamageDiedFrom;
		if ( KillerPRI != None )
		{
			KillerPRI.LastIKilledPRI = VictimPRI;
		}

		// Let my PRI know I'm dead
		VictimPRI.bIsDead = true;
	}
}

/** Hook for Kismet instigated player death. */
final event ScriptedDeath()
{
	LastTakeHitInfo.Momentum = vect(0,0,-1);
	Died(Controller,class'GDT_ScriptedGib',Location+vect(0,0,32));
}

/** Specific Pawns can decide which weapons they want to drop **/
function DropExtraWeapons( class<DamageType> DamageType );

/** The Drop Extra Worker which will spawn the specific weapon and then drop it! **/
function DropExtraWeapons_Worker( class<DamageType> DamageType, class<GearWeapon> ClassToSpawn )
{
	local vector TossVel;
	local Vector X,Y,Z;
	local GearWeapon WeaponToDrop;

	TossVel = Vector(GetViewRotation());
	TossVel = TossVel * ((Velocity Dot TossVel) + 500) + Vect(0,0,200);

	WeaponToDrop = Spawn( ClassToSpawn );

	GetAxes(Rotation,X,Y,Z);
	WeaponToDrop.DropFrom(Location + 0.8 * CylinderComponent.CollisionRadius * X - 0.5 * CylinderComponent.CollisionRadius * Y, TossVel);
}

/** Stops any current recharge and resets the timer */
final function ResetHealthRecharge()
{
	bHealthIsRecharging = FALSE;
	SetTimer( HealthRechargeDelay,FALSE,nameof(EnableHealthRecharge) );
}

/** Allows health recharging to kick in */
final function EnableHealthRecharge()
{
	if ( !IsInState('Dying') && Health > 0 )
	{
		bHealthIsRecharging = TRUE;
		RechargingHealthAmount = Health;
	}
}

/** Returns a GearPC if available, handles any special cases (like vehicles) */
simulated function GearPC GetPC()
{
	local GearPC PC;
	PC = GearPC(Controller);
	if (PC == None && DrivenVehicle != None)
	{
		PC = GearPC(DrivenVehicle.Controller);
	}
	return PC;
}

/**
 * Called by a timer from the ReplicatedPlayTakeHitEffects() function that checked to see if a meatshield
 *	saved you from death. Since we can't be sure if the player was saved due to health replication we must
 *	do this health check on a timer.
 *	This code is for determining the eGA_PoundOfFlesh achievement but could be useful for other reasons.
 */
simulated function SavedByAMeatshieldCallback()
{
	`log(GetFuncName()@`showvar(Health));
	if ( Health > 0 )
	{
		GearPC(Controller).ClientUpdateMeatshieldProgress();
	}
}

/**
 * Creates any damage effects, sounds, anims, etc.
 * Called from TakeDamage(), so only called on ROLE_Authority.
 */
function DoDamageEffects(float Damage, Pawn InstigatedBy, vector HitLocation, class<DamageType> DamageType, vector Momentum, TraceHitInfo HitInfo)
{
	local GearPawn				InstigatedByGP;
	local bool					bSuppressFX;
	local class<GearDamageType>	GearDamageType;
	local INT					AnimIndex;

	// this is replicated to clients, so they know what they were hit by (only used by DBNO currently?)
	HitDamageType = DamageType;

	// kick off the replicated hit info if it was a valid shot
	if( Damage > 0 )
	{
		InstigatedByGP = GearPawn(InstigatedBy);
		GearDamageType = class<GearDamageType>(DamageType);

		// See if we should play a full body hit reaction animation.
		if( !bPlayedDeath && CanDoSpecialMove(SM_FullBodyHitReaction) )
		{
			if( !IsHumanControlled() && GearAI_TDM(Controller) == None &&
				GearDamageType != None && FRand() <= GearDamageType.default.FullBodyHitReactionAnimChance )
			{
				AnimIndex = class<GSM_FullBodyHitReaction>(SpecialMoveClasses[SM_FullBodyHitReaction]).static.PickHitReactionAnim(Self, GearDamageType, HitInfo, Momentum, HitLocation);
				if( AnimIndex != INDEX_NONE )
				{
					ServerDoSpecialMove(SM_FullBodyHitReaction, FALSE, None, AnimIndex);
				}
			}
		}

		// and the FX aren't being suppressed on the weapon or it is (dummy fire)    AND the damage type is not suppressing FS
		bSuppressFX = (InstigatedByGP != None && InstigatedByGP.MyGearWeapon != None && InstigatedByGP.MyGearWeapon.bSuppressImpactFX);
		if (!bSuppressFX)
		{
			// @see GearPawn_LocustBerserker TakeDamage for a place that short circuits the if (Damage > 0) as it needs to show impact effects
			// from bullets but not actually take damage, etc, etc
			PlayTakeHit(Damage, InstigatedBy, HitLocation, DamageType, Momentum, HitInfo);
		}
	}
}


/**
 * Play a hit impact effect + impulse.
 * Sets up LastTakeHitInfo for replication.
 */
final function PlayTakeHit(float Damage, Pawn InstigatedBy, vector HitLocation, class<DamageType> DamageType, vector Momentum, TraceHitInfo HitInfo)
{
	local INT	HitBoneIndex;
	// Set up LastTakeHitInfo to replicate damage for instant hit weapons
	if ( LastTakeHitInfo.HitLocation != HitLocation )
	{
		LastTakeHitInfo.HitLocation	= HitLocation;
	}
	else
	{
		LastTakeHitInfo.HitLocation.Z += 1.0f;
	}

	HitBoneIndex = Mesh.MatchRefBone(HitInfo.BoneName);

	LastTakeHitInfo.Momentum			= Momentum;
	LastTakeHitInfo.HitBoneIndex		= (HitBoneIndex != INDEX_NONE) ? byte(HitBoneIndex) : 255;	// converting INT to BYTE, with -1 being non valid!
	LastTakeHitInfo.DamageType			= DamageType;
	LastTakeHitInfo.PhysicalMaterial	= HitInfo.PhysMaterial;
	LastTakeHitInfo.InstigatedBy		= InstigatedBy;
	LastTakeHitInfo.Damage				= Damage;
	LastTakeHitTimeout					= WorldInfo.TimeSeconds + 0.5;

	if( LastTakeHitInfo.PhysicalMaterial == None && HitInfo.Material != None )
	{
		LastTakeHitInfo.PhysicalMaterial = HitInfo.Material.PhysMaterial;
	}

	// play clientside effects
	ReplicatedPlayTakeHitEffects();
}

/** plays clientside hit effects using the data in LastTakeHitInfo */
simulated function ReplicatedPlayTakeHitEffects()
{
	local ImpactInfo			Impact;
	local GearWeapon			WW;
	local bool					bPlayMeleeImpactEffects, bWillShowFatalDeathBlood, bShowGore, bPlayExplosiveRadialDamageEffects;
	local class<GearDamageType>	GearDmgType;
	local Vector				Impulse;

	// call the damage type specific FX handler
	GearDmgType = class<GearDamageType>(LastTakeHitInfo.DamageType);
	if (GearDmgType != None)
	{
		GearDmgType.static.HandleDamageFX(self, LastTakeHitInfo);
	}

	if( LastTakeHitInfo.InstigatedBy == None )
	{
		return;
	}

	WW = GearWeapon(LastTakeHitInfo.InstigatedBy.Weapon);

	// Should we show gore effects?
	bShowGore = WorldInfo.GRI.ShouldShowGore();

	// mark the last take damage time for client simulation
	LastTookDamageTime = WorldInfo.TimeSeconds;

	// fill impactinfo
	Impact.HitActor				= Self;
	Impact.HitLocation			= LastTakeHitInfo.HitLocation;
	Impact.HitNormal			= IsZero(LastTakeHitInfo.Momentum) ? Normal(Impact.HitLocation - Location) : -1.f * Normal(LastTakeHitInfo.Momentum);
	Impact.RayDir				= -1.0 * Impact.HitNormal;
	Impact.HitInfo.PhysMaterial = LastTakeHitInfo.PhysicalMaterial;
	Impact.HitInfo.BoneName		= (LastTakeHitInfo.HitBoneIndex != 255) ? Mesh.GetBoneName(LastTakeHitInfo.HitBoneIndex) : 'None';
	Impact.HitInfo.HitComponent = Mesh;

	// check for non-directional impact fx (@fixme - rename this and make it a property of GearDamageType?)
	if( ClassIsChildOf(LastTakeHitInfo.DamageType, class'GDT_Melee') )
	{
		bPlayMeleeImpactEffects = TRUE;
	}
	else if( ClassIsChildOf(LastTakeHitInfo.DamageType, class'GDT_Explosive' ) )
	{
		// unless we are playing death and then we do not need to spawn the radial damage effect
		if( bPlayedDeath == FALSE )
		{
			bPlayExplosiveRadialDamageEffects = TRUE;
			Impact.HitInfo.BoneName = PelvisBoneName;
		}
	}

	if (WW != None)
	{
		// Are we going to show fatal death blood?
		bWillShowFatalDeathBlood = (Health <= 0) && bShowFatalDeathBlood;
		// Spawn the impact FX
		WW.SpawnImpactFX(GearDmgType,Impact,bWillShowFatalDeathBlood,bPlayMeleeImpactEffects,bPlayExplosiveRadialDamageEffects);
		// if we are really hurt spawn some small blood drops
		if( ( Health < (0.40f * DefaultHealth) )
			&& ( GearDmgType.default.bSuppressBloodDecals == FALSE )
			&& bSpawnBloodTrailDecals
			)
		{
			SpawnABloodTrail_PawnIsReallyHurt();
		}
		// and disable fatal blood
		if( bWillShowFatalDeathBlood )
		{
			bShowFatalDeathBlood = FALSE;
		}
	}

// 	`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "IsAHostage:" @ IsAHostage() @ "bPlayedDeath:" @ bPlayedDeath);

	// Meat Shield Special Case.
	if( IsAHostage() )
	{
		// Make sure Pawn can be special melee attacked. For instance MeatFlag cannot be headshotted, gibbed or killed.
		if( CanBeSpecialMeleeAttacked(GearPawn(LastTakeHitInfo.InstigatedBy)) )
		{
			if( bShowGore && bCanPlayHeadShotDeath && !bHasPlayedHeadShotDeath && GearDmgType.static.ShouldHeadShotGib(self, LastTakeHitInfo.InstigatedBy)
				&& HostageHealth < HostageDeathThresholdByHeadshot )
			{
				PlayHeadShotDeath();

				// On server, force Hostage release upon headshot.
				if( WorldInfo.NetMode != NM_Client )
				{
					InteractionPawn.ServerEndSpecialMove(InteractionPawn.SpecialMove);
				}
			}
		}

		/*
		// Make sure Pawn can be special melee attacked. For instance MeatFlag cannot be headshotted, gibbed or killed.
		if( CanBeSpecialMeleeAttacked(GearPawn(LastTakeHitInfo.InstigatedBy)) )
		{
			// Turn into pseudo rag doll - that should probably be replicated to ALL, not just when a hit is relevant.
// 			if( !bInMeatShieldRagDoll )
// 			{
// 				SetMeatShieldRagDoll();
// 			}

			`log(" Hostage - bShowGore:" @ bShowGore @ "bCanPlayHeadShotDeath:" @ bCanPlayHeadShotDeath @ "bHasPlayedHeadShotDeath:" @ bHasPlayedHeadShotDeath @ "ShouldHeadShotGib:" @ GearDmgType.static.ShouldHeadShotGib(self, LastTakeHitInfo.InstigatedBy)
					@ "ShouldGib1:" @ GearDmgType.static.ShouldGib(self,LastTakeHitInfo.InstigatedBy) @ "ShouldGib2:" @ ShouldGib(GearDmgType,LastTakeHitInfo.InstigatedBy));
			if( bShowGore )
			{
				// check for a head shot death
				if( bCanPlayHeadShotDeath && !bHasPlayedHeadShotDeath && GearDmgType.static.ShouldHeadShotGib(self, LastTakeHitInfo.InstigatedBy) )
				{
					`log(" Hostage - ShouldGib - PlayHeadShotDeath");
					PlayHeadShotDeath();

					// On server, force Hostage release upon headshot.
					if( WorldInfo.NetMode != NM_Client )
					{
						InteractionPawn.ServerEndSpecialMove(InteractionPawn.SpecialMove);
					}
				}
				else
				// Handle Gore / Gibs
				if( GearDmgType.static.ShouldGib(self,LastTakeHitInfo.InstigatedBy) && ShouldGib(GearDmgType,LastTakeHitInfo.InstigatedBy) )
				{
					`log(" Hostage - ShouldGib");

					// See if we need to switch to a gore skeleton, if we haven't already
					if( !bIsGore && GoreSkeletalMesh != None && GorePhysicsAsset != None )
					{
						`log(" Hostage - ShouldGib - CreateGoreSkeleton");
						CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);

// 						// Re-setup MeatShield ragdoll with new mesh & physics asset if needed.
// 						if( bInMeatShieldRagDoll )
// 						{
// 							CancelMeatShieldRagDoll();
// 							SetMeatShieldRagDoll();
// 						}
					}

					// If we're in Gore Skeleton and we have some breakable joints to gib, then do so!
					if( bIsGore && GoreBreakableJoints.Length > 0 )
					{
						`log(" Hostage - ShouldGib - GoreExplosion");
						GoreExplosion(LastTakeHitInfo.Momentum, LastTakeHitInfo.HitLocation, GearDmgType);
					}
				}
			}
		}
		*/

// 		// Handle physics impulses
// 		if( bInMeatShieldRagDoll )
// 		{
// 			Impulse = GetImpactPhysicsImpulse(GearDmgType, LastTakeHitInfo.HitLocation, LastTakeHitInfo.Momentum, Impact.HitInfo);
// 			Mesh.AddImpulse(Impulse, LastTakeHitInfo.HitLocation, Impact.HitInfo.BoneName);
// 		}
	}
	// Otherwise gib when dead
	// Check if this is a gibbing damage type ad we're allowed to show gory stuffs!
	// Gore is not compatible with HeadShot deaths
	// Wait at least a second after time of death to gib. This is to keep head shot death from gibbing right away.
	// Do not gore while doing death animation as this won't work well, because skeleton is setup to use motors.
	else if( bPlayedDeath )
	{
// 		`log(" bPlayedDeath:" @ bPlayedDeath @ "bShowGore:" @ bShowGore @ "ShouldGib1:" @ GearDmgType.static.ShouldGib(self,LastTakeHitInfo.InstigatedBy) @
// 			"ShouldGib2:" @ ShouldGib(GearDmgType,LastTakeHitInfo.InstigatedBy) @ "SpecialMove:" @ SpecialMove );

		// Handle gibbing
		if( bShowGore && GearDmgType.static.ShouldGib(self,LastTakeHitInfo.InstigatedBy) && ShouldGib(GearDmgType,LastTakeHitInfo.InstigatedBy) &&
			(WorldInfo.TimeSeconds - TimeOfDeath > 1.f) && !IsDoingDeathAnimSpecialMove() )
		{
/*			`log(" bPlayedDeath - ShouldGib");*/
			// See if we need to switch to a gore skeleton, if we haven't already
			if( !bIsGore && GoreSkeletalMesh != None && GorePhysicsAsset != None )
			{
/*				`log(" bPlayedDeath - ShouldGib - CreateGoreSkeleton");*/
				CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);
			}

			// If we're in Gore Skeleton and we have some breakable joints to gib, then do so!
			if( bIsGore && GoreBreakableJoints.Length > 0 )
			{
				GoreExplosion(LastTakeHitInfo.Momentum, LastTakeHitInfo.HitLocation, GearDmgType);
			}
		}

		// Add an impulse if in rag doll
		if( Physics == Phys_RigidBody && !GearDmgType.default.bKRadialImpulse)
		{
			Impulse = GetImpactPhysicsImpulse(GearDmgType, LastTakeHitInfo.HitLocation, LastTakeHitInfo.Momentum, Impact.HitInfo);
			Mesh.AddImpulse(Impulse, LastTakeHitInfo.HitLocation, Impact.HitInfo.BoneName);
		}
	}
	// Otherwise play standard 'alive' hit reactions
	else
	{
		// Try to play a hit reaction animation. Otherwise fallback to physics hit reactions.
		if( (GearDmgType == None || !GearDmgType.default.bSuppressImpactFX) && !TryPlayHitReactionAnimation(Impact) )
		{
			// Play Physics Body Impact.
			PlayPhysicsBodyImpact(Impact.HitLocation, LastTakeHitInfo.Momentum, LastTakeHitInfo.DamageType, Impact.HitInfo);
		}
	}
}

/** Play Hit Reaction animation */
simulated function bool TryPlayHitReactionAnimation(ImpactInfo Impact)
{
	/*	`DLog("BoneName:" @ Impact.HitInfo.BoneName);*/
	// Need a bone name to do something...
	if( Impact.HitInfo.BoneName == '' || TimeSince(NextHitReactionAnimTime) < 0 )
	{
		return FALSE;
	}

	// Disable when doing special moves
	if( IsDoingASpecialMove() || bSwitchingWeapons || bIsConversing || bPlayedDeath )
	{
		return FALSE;
	}

	// Disable for locally controlled pawns targeting
	if( IsLocallyControlled() && IsHumanControlled() && bIsTargeting )
	{
		return FALSE;
	}

	return PlayHitReactionAnimation(Impact);
}

/** By default, do not play hit reaction animations */
simulated function bool PlayHitReactionAnimation(ImpactInfo Impact)
{
	return FALSE;
}

/**
 * Plays a Physical Body Impact hit reaction.
 */
final simulated function PlayPhysicsBodyImpact(vector HitLocation, vector Momentum, class<DamageType> DamageType, TraceHitInfo HitInfo)
{
	local Vector	BodyImpactImpulse;
	local bool		bAlreadyHit;
	local float		ImpactScale, CurrentHitPct;

	// If you are a human and you are targeting, need a mesh, don't do that in Phys_RigidBody (ragdoll)
	if( !bCanPlayPhysicsHitReactions || Mesh == None || Mesh.bNotUpdatingKinematicDueToDistance || Physics == Phys_RigidBody ||
		bPlayedDeath || IsInState('Dying') || bInMeatShieldRagDoll || (MyGearWeapon != None && MyGearWeapon.WeaponType == WT_Heavy) )
	{
		return;
	}

	// If on a moving base, disable physics hit reactions, as they tend to look bad.
	if( Base != None && Base.Physics == PHYS_Interpolating )
	{
		return;
	}

	// Those root motion modes don't work well with physics hit reactions. Not sure why.
	if( Mesh != None && (Mesh.RootMotionMode == RMM_Relative || Mesh.RootMotionMode == RMM_Translate) )
	{
		return;
	}

	// Prevent physical hit reactions for gameplay reasons
	if( (IsLocallyControlled() && IsHumanControlled() && bIsTargeting) || IsEvading()
		|| IsDoingSpecialMeleeAttack() || IsChainsawDueling() || IsAKidnapper() )
	{
		return;
	}

	// If this is > 0, we're already being hit
	CurrentHitPct	= PhysicsImpactBlendTimeToGo / PhysicsImpactBlendOutTime;
	bAlreadyHit		= CurrentHitPct > 0.f;

	//  Physics Impulse Given
	BodyImpactImpulse = GetImpactPhysicsImpulse(DamageType, HitLocation, Momentum, HitInfo, TRUE);

	// If hit a bone, then play Physics Body Impact.
	if( HitInfo.BoneName != '' && !IsZero(BodyImpactImpulse) )
	{
		// So if Pawn has been recently hit, randomize impulse to show some impact variations.
		ImpactScale	= bAlreadyHit ? 0.75f * (0.25f + FRand()) : 1.f;

		BodyImpactImpulse = PhysicsHitReactionImpulseScale * ImpactScale * BodyImpactImpulse;

		// When hitting someone not locally controlled by a human player, then increase hit reactions for feedback.
		if( !IsHumanControlled() || !IsLocallyControlled() )
		{
			BodyImpactImpulse *= 2.5f;
		}

// 		`DLog("BodyImpactImpulse:" @ VSize(BodyImpactImpulse) @ "ImpactScale:" @ ImpactScale @ "PhysicsHitReactionImpulseScale:" @ PhysicsHitReactionImpulseScale);
// 		`log("  PlayPhysicsBodyImpact on Bone:" @ HitInfo.BoneName @ "DamageType:" @ DamageType @ "Momentum:" @ Momentum );

		Mesh.AddImpulse(BodyImpactImpulse, HitLocation, HitInfo.BoneName);
		//Mesh.WakeRigidBody();
		StartPhysicsBodyImpact(HitInfo.BoneName, TRUE, class<GearDamageType>(DamageType));
	}
}


/** Get Physical Impulse to apply based on damage type and physics information */
simulated final function vector GetImpactPhysicsImpulse(class<DamageType> DamageType, vector HitLoc, vector Momentum, out TraceHitInfo OutHitInfo, optional bool bIsHitReaction)
{
	local float		TotalMass, ImpulseScale;
	local Vector	BaseImpulse;
// 	local Name		OriginalBoneName;

	// Skip if no damage type or momentum/impulse
	if( DamageType == None || IsZero(Momentum) || (DamageType.default.KDamageImpulse == 0.f && DamageType.default.KDeathUpKick == 0.f) )
	{
`if(`notdefined(FINAL_RELEASE))
		if( IsZero(Momentum) )
		{
			`log(WorldInfo.TimeSeconds @ "Zero Momentum!!!" @ GetFuncName() @ "returning 0." @ "DamageType:" @ DamageType @ "Momentum:" @ Momentum @ "KDamageImpulse:" @ DamageType.default.KDamageImpulse @ "KDeathUpKick:" @ DamageType.default.KDeathUpKick);
			ScriptTrace();
		}
`endif
		return vect(0,0,0);
	}

	BaseImpulse = Momentum * DamageType.default.KDamageImpulse;

	// If we want upward force, then take it into account. We want this when we're dead, but not when taking hit reactions alive.
	if( !bIsHitReaction )
	{
		BaseImpulse += VSize(Momentum) * Vect(0,0,1) * DamageType.default.KDeathUpKick;
	}

	// Try to get a valid bone name to apply the impulse to.
	if( OutHitInfo.BoneName == '' )
	{
		CheckHitInfo(OutHitInfo, Mesh, Normal(Momentum), HitLoc);
	}

	// We do need a bone name to go any further, so if we don't have that, abort.
	if( OutHitInfo.BoneName == '' )
	{
		`log(GetFuncName() @ "No Bone to figure out damage :(");
		return BaseImpulse;
	}

	// If not a physics hit reaction (alive), then don't perform further processing
	if( !bIsHitReaction )
	{
		return BaseImpulse;
	}

	// See if we need to remap the RigidBody bone -- Only do this for physics hit reactions.
	if( PhysicsImpactRBRemapTable.Length > 0 )
	{
// 		OriginalBoneName	= OutHitInfo.BoneName;
		OutHitInfo.BoneName = GetPhysicsImpactRemappedBone(OutHitInfo.BoneName);
	}

	ImpulseScale = 1.f;

	// If we're hitting a BodySetup on the skeletal mesh, try to scale by mass
	// We do this to balance the fact that being hit in the chest won't cause much impact, while being hit in the hand will send it flying.
	if( Mesh.PhysicsAssetInstance != None )
	{
		TotalMass		= Mesh.PhysicsAssetInstance.GetTotalMassBelowBone(OutHitInfo.BoneName, Mesh.PhysicsAsset, Mesh.SkeletalMesh);
		ImpulseScale	= 1.f + (TotalMass - 1.f) * PhysicsImpactMassEffectScale;
// 		`DLog("TotalMass:" @ TotalMass @ "ImpulseScale:" @ ImpulseScale @ "for bone" @ OutHitInfo.BoneName @ "(OriginalBoneName:" @ OriginalBoneName $ ")");
	}

// 	`log("KDamageImpulse:" @ DamageType.default.KDamageImpulse @ "KDeathUpKick:" @ DamageType.default.KDeathUpKick @ "Momentum:" @ VSize(Momentum) );

	// apply a death impulse to the ragdoll
	// Note: TearOffMomentum gives the direction of the impulse, but also the scale factor.
	//return ImpulseScale * ((Momentum * DamageType.default.KDamageImpulse) + (VSize(Momentum) * (Vect(0,0,1) * DamageType.default.KDeathUpKick)));
	return ImpulseScale * BaseImpulse;
}


/** See if InBoneName exists in PhysicsImpactRBRemapTable, and return which BoneName it should be remapped to */
simulated final function Name GetPhysicsImpactRemappedBone(Name InBoneName)
{
	local int i;

	for( i=0; i<PhysicsImpactRBRemapTable.Length; i++)
	{
		if( PhysicsImpactRBRemapTable[i].RB_FromName == InBoneName )
		{
			return PhysicsImpactRBRemapTable[i].RB_ToName;
		}
	}

	return InBoneName;
}


/** Start a Physics Body Impact */
final simulated function StartPhysicsBodyImpact(Name HitBoneName, bool bUseMotors, class<GearDamageType> GearDamageType)
{
	local bool	bFoundBone;
	local int	i;

	if( Mesh == None || Mesh.PhysicsAsset == None || Mesh.PhysicsAssetInstance == None )
	{
		`log(WorldInfo.TimeSeconds @ "StartPhysicsBodyImpact. Pawn is missing needed assets for Body Impact " $ self );
		return;
	}

	for(i=0; i<PhysicsBodyImpactBoneList.Length; i++)
	{
		if( PhysicsBodyImpactBoneList[i] == HitBoneName )
		{
			bFoundBone = TRUE;
			break;
		}
	}

	// Hit a Bone that is kinematic and not dynamic. So don't do anything
	if( !bFoundBone )
	{
		//`log(WorldInfo.TimeSeconds @ "StartPhysicsBodyImpact. Hit a Bone that is kinematic and not dynamic. So don't do anything HitBoneName:" @ HitBoneName);
		return;
	}

	// Unfix bones that should be affected by physics, fix others (Kinematic)
	Mesh.PhysicsAssetInstance.SetNamedBodiesFixed(FALSE, PhysicsBodyImpactBoneList, Mesh, FALSE, TRUE);

	StartPhysicsBodyImpactPhysicsModify();

	if( bUseMotors )
	{
		// Turn on motors
		Mesh.bUpdateJointsFromAnimation = TRUE;

		// Set motor strength
		Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(PhysHRMotorStrength.X, PhysHRMotorStrength.Y, 0.f, Mesh, TRUE);
		Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(TRUE, TRUE, Mesh, TRUE);
	}
	else
	{
		// Turn off motors
		Mesh.bUpdateJointsFromAnimation = FALSE;
		Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, Mesh, TRUE);
	}

	// Setup Hit Reaction bone springs -- to prevent some bones from being pushed too far
	if( bEnableHitReactionBoneSprings && PhysicsImpactSpringList.Length > 0 && Base != None && !Base.bMovable && !bHasBrokenConstraints )
	{
		Mesh.PhysicsAssetInstance.SetNamedRBBoneSprings(TRUE, PhysicsImpactSpringList, PhysHRSpringStrength.X, PhysHRSpringStrength.Y, Mesh);
	}

	// Add Base linear velocity, so it doesn't look strange when on a moving platform.
	AddBaseLinearVelocityUponWhenGoingRagDoll();

	// Make it start simulating
	Mesh.WakeRigidBody();

	// Enable full physics on body
	Mesh.PhysicsWeight = 1.f;

	// Set Blend Out recovery time.
	PhysicsImpactBlendTimeToGo = PhysicsImpactBlendOutTime * GearDamageType.default.HitReactionTimeScale;
}


simulated function StartPhysicsBodyImpactPhysicsModify()
{
	// Disable collision with alive pawns
	Mesh.SetRBChannel(RBCC_Nothing);
	Mesh.SetRBCollidesWithChannel(RBCC_Pawn, FALSE);
}


/** notification that we are done blending back from physics to animations */
final simulated event BodyImpactBlendOutNotify()
{
	Mesh.PhysicsWeight			= 0.f;
	PhysicsImpactBlendTimeToGo	= 0.f;

	// Fix all bones back to animation. Return to being fully Kinematic.
	Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
	Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);

	// Restore collision to defaults.
	BodyImpactBlendOutNotifyPhysicsModify();

	// Turn off motors
	Mesh.bUpdateJointsFromAnimation = FALSE;
	Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, Mesh, TRUE);

	// Turn Off Bone Springs
	if( PhysicsImpactSpringList.Length > 0 )
	{
		Mesh.PhysicsAssetInstance.SetNamedRBBoneSprings(FALSE, PhysicsImpactSpringList, 0.f, 0.f, Mesh);
	}
}

simulated function BodyImpactBlendOutNotifyPhysicsModify()
{
	// Restore collision to defaults.
	Mesh.SetRBChannel(default.Mesh.RBChannel);
	Mesh.SetRBCollidesWithChannel(RBCC_Pawn, default.Mesh.RBCollideWithChannels.Pawn);
}


//@fixme - this function shouldn't be called currently
function PlayHit(float Damage, Controller InstigatedBy, vector HitLocation, class<DamageType> damageType, vector Momentum, TraceHitInfo HitInfo)
{
`if(`notdefined(FINAL_RELEASE))
	`log(GetFuncName()@"is deprecated for Gear, please fix!");
	ScriptTrace();
`endif
}

// some constraints need to be broken when another bone's constraint is.. check for such bones and break them accordingly
simulated final function BreakDependantBones(Vector Impulse, Vector HitLocation, Name InBoneName, optional bool bVelChange)
{
	local int i,j;

	for(i=0;i<JointsWithDependantBreaks.length;i++)
	{
		if(JointsWithDependantBreaks[i].ParentBone == InBoneName)
		{
			for(j=0;j<JointsWithDependantBreaks[i].DependantBones.length;j++)
			{
				//`log(GetFuncName()@"Breaking "@JointsWithDependantBreaks[i].DependantBones[j]@"because"@InBoneName@"was broken!");
				BreakConstraint(Impulse,HitLocation,JointsWithDependantBreaks[i].DependantBones[j],bVelChange);
			}
		}
	}
}

simulated final function BreakConstraint(Vector Impulse, Vector HitLocation, Name InBoneName, optional bool bVelChange)
{
	if( Mesh == None )
	{
	`if(`notdefined(FINAL_RELEASE))
		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Tried to gib with not gore skeleton!!");
		ScriptTrace();
	`endif
		return;
	}

	Mesh.BreakConstraint(Impulse,HitLocation,InBoneName,bVelChange);

	BreakDependantBones(Impulse,HitLocation,InBoneName);

	// Keep track that broke at least one constraint off this gore mesh.
	if( !bHasBrokenConstraints )
	{
		bHasBrokenConstraints = TRUE;

		// only want to call this if we're actually dead (could be hostage, in which case we don't want to call this yet)
		if(bPlayedDeath)
		{
			// This will turn on optimizations for Gibs.
			SetupDeathGibsOptimizations();
		}
	}
}

/**
 * This will cause the mob to blow up into little pieces.
 *
 * NOTE: this is called from ReplicatedPlayTakeHitEffects on bodies that have been gored.  This plays the big bloody particle effect and additionally
 * will break off bones based on the hit location.
 *
 */
simulated function GoreExplosion( Vector Momentum, Vector HitLocation, class<GearDamageType> GearDamageType, optional bool bRandomizeGibImpulse )
{
	local int			i;
	local Vector		ConstraintLoc, HitDir, Impulse;
	local float			DistToGib;
	local TraceHitInfo	HitInfo;
	local Emitter		ImpactEmitter;
	local PlayerController PC;
	local vector RandomImpulse;

	// don't do any thing if we are outside of 8000 units as we are way way far away and this is just semi wasted work
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLocation, 8000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistance ) == FALSE )
	{
		return;
	}


	// Catch Pawns who have not be set up properly, and throw a warning.
	if( GoreBreakableJoints.length == 0 )
	{
/*		`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Has no GoreBreakableJoints array defined!!");*/
		return;
	}

	if( !bIsGore )
	{
`if(`notdefined(FINAL_RELEASE))
		`warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Tried to gib with no gore skeleton!!");
		ScriptTrace();
`endif
		return;
	}


	HitDir		= Normal(Momentum);
	Impulse		= GetImpactPhysicsImpulse(GearDamageType, HitLocation, Momentum, HitInfo);
	DistToGib	= GearDamageType.default.DistFromHitLocToGib;
	// negate current velocity
	if (!IsDoingSpecialMove(SM_MidLvlJumpOver))
	{
		Impulse += -Velocity;
	}

	/*
	`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Impulse:" @ Impulse);
	FlushPersistentDebugLines();
	DrawDebugSphere(HitLocation, DistToGib, 48, 0, 255, 0, TRUE);
	DrawDebugSphere(HitLocation + HitDir * DistToGib, DistToGib, 48, 0, 255, 0, TRUE);
	DrawDebugLine(HitLocation, HitLocation + HitDir * DistToGib, 255, 0, 0, TRUE);
	*/

	PlaySound(SoundCue'Foley_Flesh.Flesh.GibBodyExplodeCue', TRUE); // if you are not playing in extreme content mode you will not get this sound :-(

	if( ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLocation, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		&& ( GearDamageType.default.bSuppressPlayExplosiveRadialDamageEffects == FALSE )
		)
	{

		ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Full_LOD_Death', HitLocation, rot(0,0,0) );
		//Mesh.AttachComponent( ImpactPSC, PelvisBoneName );
		ImpactEmitter.SetBase(self);
		ImpactEmitter.ParticleSystemComponent.ActivateSystem();
		//DrawDebugCoordinateSystem( HitLocation, rot(0,0,0), 3.0f, TRUE );
	}

	// we only want to to play the effects once and if we are spawning blood trail decals
	if( bSpawnBloodTrailDecals && !bHasGoreExploded )
	{
		// delay by a tick to spread out the cost a bit
		SetTimer(WorldInfo.DeltaSeconds + 0.01, false, nameof(SpawnABloodTrail_GibExplode_360));
	}

	// chainsaw uses a special method of breaking the gore
	if( GearDamageType == class'GDT_Chainsaw' )
	{
		ChainsawGore();
		bHasGoreExploded = TRUE;
		return;
	}

	// look and see if we should spawn a little blood splatter on their camera as someone just got gibbed near them
	if (!WorldInfo.bDropDetail && bSpawnHitEffectDecals)
	{
		foreach WorldInfo.LocalPlayerControllers( class'PlayerController', PC )
		{
			// splatter some blood on their camera if they are a human and decently close
			if(	( PC.Pawn != none && VSize(PC.Pawn.Location - Location) < 768.0f )
				&& PC.IsAimingAt( self, 0.1f ) // if we are semi looking in the direction of the gibbed pawn
				)
			{
				PC.ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
			}
		}
	}

	//`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "DamageType:" @ GearDamageType @ "DistToGib:" @ DistToGib);

	// TODO:  we want to defer these so we don't pay the cost all at once in a single frame
	for(i=0; i<GoreBreakableJoints.Length; i++)
	{
		//`log("  Joint:" @ GoreBreakableJoints[i] );

		ConstraintLoc = Mesh.GetBoneLocation(GoreBreakableJoints[i]);
		// If breaks all, or within cylinder of damage.
		if( DistToGib <= 0.f || PointDistToLine(ConstraintLoc, HitDir, HitLocation) <= DistToGib )
		{
			if( bRandomizeGibImpulse || ClassIsChildOf(GearDamageType,class'GDT_Shotgun') )
			{
				RandomImpulse.X = Impulse.X * FRand()*16;
				RandomImpulse.Y = Impulse.Y * FRand()*16;
				RandomImpulse.Z = Impulse.Z * FRand()*16;

				BreakConstraint(RandomImpulse, HitLocation, GoreBreakableJoints[i]);
			}
			else
			{
				BreakConstraint(Impulse, HitLocation, GoreBreakableJoints[i],TRUE);
			}
		}
	}

	// Don't apply linear/angular vel except on initial explosion
	if(!bHasGoreExploded)
	{
		Impulse *= 0.75;
		Impulse.Z *= 0.5;

		Mesh.SetRBLinearVelocity(Impulse,TRUE);
		if (ClassIsChildOf(GearDamageType,class'GDT_Explosive'))
		{
			Impulse *= 0.5;
			Mesh.SetRBAngularVelocity(VRand() * RandRange(8,16),TRUE);
		}
		else if (ClassIsChildOf(GearDamageType,class'GDT_Shotgun'))
		{
			Mesh.SetRBAngularVelocity(VRand() * RandRange(1,2),TRUE);
		}
	}

	bHasGoreExploded = TRUE;
}

/** This will break the specific constraints that need to be broken for the chainsaw cut in half effect. **/
simulated function ChainSawGore();

/** hides the mesh and any attachments */
simulated function HideMesh()
{
	Mesh.SetHidden(true);
}

simulated event TornOff()
{
	// enforce a default momentum
	if (IsZero(TearOffMomentum))
	{
		TearOffMomentum = vect(0,0,1);
	}

	Super.TornOff();

	// destroy client-side weapons
	if ( GearInventoryManager(InvManager) != None )
	{
		GearInventoryManager(InvManager).DiscardClientSideInventory();
	}
}

/** Turn off controllers and get pawn ready for being put into physics */
simulated function ReadyPawnForRagdoll()
{
	// Stop any melee attack
	StopMeleeAttack();

	// Abort current special move
	if( IsDoingASpecialMove() )
	{
		EndSpecialMove();
	}

	// Turn off hit reactions
	if( PhysicsImpactBlendTimeToGo > 0.f )
	{
		BodyImpactBlendOutNotify();
	}

	// If in Meat Shield rag doll, then cancel it
	if( bInMeatShieldRagDoll )
	{
		CancelMeatShieldRagDoll();
	}

	// Ensure we are always updating kinematic
	Mesh.MinDistFactorForKinematicUpdate = 0.0;

	// If we had stopped updating kinematic bodies on this character due to distance from camera, force an update of bones now.
	// Also ensure rigid body collision is on.
	if( Mesh.bNotUpdatingKinematicDueToDistance )
	{
		Mesh.ForceSkelUpdate();
		Mesh.UpdateRBBonesFromSpaceBases(TRUE, TRUE);
		Mesh.SetBlockRigidBody(TRUE);
	}

	// Turn off IK Controllers
	if( IKCtrl_RightHand != None )			IKCtrl_RightHand.SetSkelControlActive(FALSE);
	if( IKCtrl_LeftHand != None )			IKCtrl_LeftHand.SetSkelControlActive(FALSE);
	if( IKBoneCtrl_RightHand != None )		IKBoneCtrl_RightHand.SetSkelControlActive(FALSE);
	if( IKBoneCtrl_LeftHand != None )		IKBoneCtrl_LeftHand.SetSkelControlActive(FALSE);
	if( IKRotCtrl_RightHand != None )		IKRotCtrl_RightHand.SetSkelControlActive(FALSE);
	if( IKRotCtrl_LeftHand != None )		IKRotCtrl_LeftHand.SetSkelControlActive(FALSE);
	if( CrateIKLeftHand != None )			CrateIKLeftHand.SetSkelControlActive(FALSE);
	if( CrateIKRightHand != None )			CrateIKRightHand.SetSkelControlActive(FALSE);

	// Turn off HeadLook controller
	if( HeadControl != None )
	{
		HeadControl.SetSkelControlActive(FALSE);
	}
}

/**
 * Cancel effects of ragdolling, to get up.
 */
simulated function PostRagDollRecovery()
{
	Mesh.MinDistFactorForKinematicUpdate = default.Mesh.MinDistFactorForKinematicUpdate;

	// Turn back on IK Controllers
	if( IKBoneCtrl_RightHand != None )		IKBoneCtrl_RightHand.SetSkelControlActive(TRUE);
	if( IKBoneCtrl_LeftHand != None )		IKBoneCtrl_LeftHand.SetSkelControlActive(TRUE);
}


/**
 * Responsible for playing any death effects, animations, etc.
 *
 * @param	DamageType - type of damage responsible for this pawn's death
 *
 * @param	HitLoc - location of the final shot
 */
simulated function PlayDying(class<DamageType> DamageType, vector HitLoc)
{
	local PlayerController		PC;
	local class<GearDamageType>	GearDamageType;

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
	TimeOfDeath			= WorldInfo.TimeSeconds;
	LocationWhenKilled	= Location;
	RotationWhenKilled	= Rotation;
	SpecialMoveWhenDead	= SpecialMove;

	// Figure out what we were based on...
	BaseWhenDead = Base;
	if( ClampedBase != None )
	{
		BaseWhenDead = ClampedBase;
	}
	if( BaseWhenDead != None && BaseWhenDead == InteractionPawn )
	{
		BaseWhenDead = InteractionPawn.Base;
	}

// 	`log("=> BaseWhenDead:" @ BaseWhenDead @ "Base:" @ Base @ "ClampedBase:" @ "InteractionPawn:" @ InteractionPawn @ "SpecialMoveWhenDead:" @ SpecialMoveWhenDead);

	// End current Special Move for clean up.
	EndSpecialMove();
	Acceleration = Vect(0,0,0);

	// If we're dead and already have a broken constraint, make sure we turn on the Gibs optimizations.
	if( bHasBrokenConstraints )
	{
		SetupDeathGibsOptimizations();
	}

	PC = PlayerController(Controller);
	if( PC != None )
	{
		// play a death rumble
		PC.ClientPlayForceFeedbackWaveform( GearDamageType.default.KilledFFWaveform );
	}

	// handle stopping any current speech, if any
	if (CurrentlySpeakingLine != None)
	{
		`SpeechLog(self@"died, aborting speech"@CurrentlySpeakingLine.SoundCue);
		CurrentlySpeakingLine.FadeOut(0.2f, 0.f);
		SpeakLineFinished();
	}

	// play the death scream
	if( !bDisableDeathScreams && GearDamageType.default.DeathScreamEffortID != GearEffort_None )
	{
		SoundGroup.PlayEffort(self, GearDamageType.default.DeathScreamEffortID,, TRUE);
	}

	// Turn off controllers etc.
	ReadyPawnForRagdoll();

	// Skip to dying state
	GotoState('Dying');

	if (Role < ROLE_Authority && CurrentLink != None)
	{
		//`log("simulating cover unclaim from death:"@CurrentLink@CurrentSlotIdx@self);
		CurrentLink.UnClaim(self,CurrentSlotIdx,FALSE);
	}

	// play death and gore only on viewports
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// make room for the new ragdoll
		ManageRagdolls();

		// Play death. animation or rag doll or motorized death
		PlayDeath(GearDamageType, HitLoc);
	}
	else
	{
		// dedicated server only hides the mesh, no need to do anything else
		HideMesh();
	}
}

/**
 * Handles eliminating old/nonvisible ragdolls to keep performance in check.
 */
final function ManageRagdolls()
{
	local float				OldestTimeOfDeath;
	local GearPawn			P, OldestDeadPawn;
	local int				NumDeadPawns;
	//@fixme - refactor this to delete farthest/nonvisible ragdolls first, also to ignore inactive (sleeping) ragdolls
	// Check how many Rag Dolled Pawns we have around us.
	// It can get slow if we have too many, so we start killing the oldest one if we get beyond MaxAllowedDeadRagDollPawns.
	foreach WorldInfo.AllPawns(class'GearPawn', P)
	{
		// Disregard us, pending delete and hidden pawns.
		if( P != Self && !P.bDeleteMe && !P.bScalingToZero && !P.bHidden && !P.Mesh.HiddenGame && P.bPlayedDeath && P.Physics == PHYS_RigidBody )
		{
			NumDeadPawns++;

			if( OldestTimeOfDeath == 0.f || P.TimeOfDeath < OldestTimeOfDeath )
			{
				OldestTimeOfDeath	= P.TimeOfDeath;
				OldestDeadPawn		= P;
			}
		}
	}

	// Kill Oldest Gore Pawn.
	if( NumDeadPawns >= MaxAllowedDeadRagDollPawns && OldestDeadPawn != None )
	{
		OldestDeadPawn.bScalingToZero = TRUE;
	}
}

/** Allows GearPawn classes to decide to blow up using a very simple particle effect in some situations */
simulated function bool ShouldUseSimpleEffectDeath(class<GearDamageType> GearDamageType)
{
	return FALSE;
}

/** Play death effects: animation, sounds, physics... */
simulated function PlayDeath(class<GearDamageType> GearDamageType, vector HitLoc)
{
	local bool	bShowGore, bPlayedDeathAnim;
	local Actor OldBase;
	local Emitter DeathEffect;

	bPlayedDeathAnim = FALSE;
	bShowGore = WorldInfo.GRI.ShouldShowGore();

// 	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ GearDamageType @ "bForceRagdollDeath:" @ GearDamageType.default.bForceRagdollDeath @
// 		"bShowGore:" @ bShowGore @ "bCanPlayHeadShotDeath:" @ bCanPlayHeadShotDeath @ "ShouldHeadShotGib:" @ GearDamageType.static.ShouldHeadShotGib(Self, KilledByPawn) );

	// See if we want to use super-simple
	if( CarriedCrate != None || ShouldUseSimpleEffectDeath(GearDamageType) )
	{
		if( bShowGore )
		{
			DeathEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(SimpleDeathEffect, Location, Rotation);
		}
		else
		{
			DeathEffect = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter(SimpleDeathEffectNonGore, Location, Rotation);
		}

		DeathEffect.SetDrawScale(SimpleDeathEffectScale);
		DeathEffect.ParticleSystemComponent.ActivateSystem();
		// Just straight up destroy pawn
		Destroy();
		return;
	}

	PlayDeathPhysicsModify();

	// if allowed to show the good stuff...
	if( bShowGore )
	{
		// check for a head shot death
		if( !bHasPlayedHeadShotDeath && bCanPlayHeadShotDeath && !bIsGore && GearDamageType.static.ShouldHeadShotGib(Self, KilledByPawn) )
		{
			PlayHeadShotDeath();
		}
		// check to see if we should create the gore skeleton
		else if( GearDamageType.static.ShouldGib(Self, KilledByPawn) && ShouldGib(GearDamageType, KilledByPawn) )
		{
			if( !bIsGore && GoreSkeletalMesh != None && GorePhysicsAsset != None )
			{
				CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);
			}

			// If we're in Gore Skeleton and we have some breakable joints to gib, then do so!
			if( bIsGore && GoreBreakableJoints.Length > 0 )
			{
				GoreExplosion(TearOffMomentum, HitLoc, GearDamageType);
			}
		}

		//@fixme - make this based off the physics coming to rest
		SetTimer( 15.0f, FALSE, nameof(NoMoreBloodEffectsOnDeadBodies) );
	}

	// if we die on a bloodmount, stay attached to it and go ragdoll from the waste up
	if( BloodMountIDiedOn != None && FRand() <= BloodMountIDiedOn.DriverStayInSaddleChance )
	{
		BloodMountIDiedOn.PlayMountedDeathFor(Self, GearDamageType, HitLoc, bShowGore);
	}
	else
	{
		// check to see if we can play blended death anims
		// HeadShot is a special case, as we don't actually play a death animation, but use the motor setup.
		//	However the character has to be setup for that, so we need to make sure that we have Pawn.bCanPlayHeadShotDeath == TRUE
		if( Physics != PHYS_RigidBody && SpecialMoveWhenDead != SM_RecoverFromRagdoll && SpecialMoveWhenDead != SM_Hostage &&
			!GearDamageType.default.bForceRagdollDeath && (bPlayDeathAnimations || bHasPlayedHeadShotDeath) && (!bIsGore || bHasPlayedHeadShotDeath) )
		{
// 			`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "SM_DeathAnim");

			// Stop walking physics, but preserve Base (in case we're on a mover, etc)
			OldBase = Base;
			SetPhysics(PHYS_None);
			SetBase(OldBase);

			if( CanDoSpecialMove(SM_DeathAnimFire) )
			{
				DoSpecialMove(SM_DeathAnimFire, TRUE);
				bPlayedDeathAnim = TRUE;
			}
			else if( CanDoSpecialMove(SM_DeathAnim) )
			{
				DoSpecialMove(SM_DeathAnim, TRUE);

				// Play animation
				if( GSM_DeathAnimBase(SpecialMoves[SM_DeathAnim]).PlayDeathAnimation(GearDamageType, HitLoc) )
				{
					bPlayedDeathAnim = TRUE;
				}
			}
		}

		// If we haven't played a custom death, like DeathAnimations, then default to simple rag dolling.
		if( !bPlayedDeathAnim )
		{
// 			`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "PlayRagDollDeath");
			PlayRagDollDeath(GearDamageType, HitLoc, bShowGore);
		}
	}

	// death by fire causes permanent charred skin effect
	if( ClassIsChildOf(LastTakeHitInfo.DamageType, class'GDT_Fire') )
	{
		// stop char fading
		CharSkin( 1.0f );
		CurrentSkinCharMin = 0.95f;
		SkinHeatFadeTime = 5.0f; // fade out nicely
		//SkinCharFadeTime = 1.f;
	}

	// dude is dead, will end up as ragdoll somehow, no need for cylinder to block anymore
	if( CollisionComponent != CylinderComponent )
	{
		CylinderComponent.SetTraceBlocking(FALSE, FALSE);
	}
}

simulated function PlayDeathPhysicsModify();

simulated final native function bool IsDoingDeathAnimSpecialMove() const;

/** Event called from Anim Notify when body rests in animation, to trigger transition to full rag doll. */
simulated function OnDeathAnimBodyRest()
{
	if( SpecialMove == SM_DeathAnim )
	{
		GSM_DeathAnim(SpecialMoves[SpecialMove]).OnDeathAnimBodyRest();
	}
}

/**
 * Sometimes you can get a body that is in a corner and vibrating and moving up and down on itself
 * such that he will sit there for ever constantly spewing blood over and over.	 And it looks a little
 * weird.  So this function will set the GibEffectsSpeedThreshold to a large value.
 **/
simulated function NoMoreBloodEffectsOnDeadBodies()
{
	GibEffectsSpeedThreshold = 100000;
}

/** Play HeadShot special death */
simulated function PlayHeadShotDeath()
{
	local SkelControlBase			Control;
	local ParticleSystemComponent	ImpactPSC;
	local GearGRI					GearGRI;

	bHasPlayedHeadShotDeath = TRUE;

	// Scale neck bone down to zero to hide head.
	Control	= Mesh.FindSkelControl('NeckScale');
	if( Control != None )
	{
		Control.SetSkelControlActive(TRUE);
	}

	if( WorldInfo.GRI.ShouldShowGore() )
	{
		if( !bIsGore )
		{
			CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);
		}

		// Gore attachment to hide neck bone scaling.
		if( HeadShotNeckGoreAttachment != None )
		{
			Mesh.AttachComponentToSocket(HeadShotNeckGoreAttachment, 'HeadShotGoreMesh');
		}

		GearGRI = GearGRI(WorldInfo.GRI);

		if( GearGRI.IsEffectRelevant( Instigator, Location, 4000, FALSE, GearGRI.CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		{
			// Force blood emitter to be attached on the neck.
			ImpactPSC = GearGRI.GOP.GetImpactParticleSystemComponent(ParticleSystem'Effects_Gameplay.Blood.P_Blood_HeadShot');
			Mesh.AttachComponentToSocket(ImpactPSC, 'HeadShotBloodPS');
			ImpactPSC.SetLODLevel(GearGRI.GetLODLevelToUse(ImpactPSC.Template, Location));
			ImpactPSC.ActivateSystem();

			SpawnHeadChunks();
		}

		if( bSpawnBloodTrailDecals )
		{
			// start up the blood spurting from the neck
			StartBloodTrail( 'SpawnABloodTrail_HeadShot', 0.2f );
			SetTimer( 3.5f, FALSE, nameof(StopBloodTrail_HeadShot) );
		}
	}

	if( GearGRI.IsEffectRelevant( Instigator, Location, 2000, FALSE, GearGRI.CheckEffectDistance_SpawnWithinCullDistance ) )
	{
		PlaySound( SoundCue'Foley_Flesh.Flesh.GibHeadShotCue' );
	}
}

simulated final function SpawnHeadChunks()
{
	local int				ChunkNb;
	local Vector			HeadLoc, X, Y, Z;
	local Rotator			HeadRot;

	// Spawn several physics Head Chunks
	if( HeadChunks.Length > 0 )
	{
		// HeadRot is pointing up.
		Mesh.GetSocketWorldLocationAndRotation('HeadShotBloodPS', HeadLoc, HeadRot);
		GetAxes(Instigator.Rotation, X, Y, Z);

		// always spawn the head
		SpawnAHeadChunk( HeadLoc, HeadRot, HeadChunks[0] ); // 0 is always the main piece

		// Spawn several physics Head Chunks
		for(ChunkNb=0; ChunkNb<5; ChunkNb++)
		{
			// don't spawn the 0 (head) twice
			SpawnAHeadChunk( HeadLoc, HeadRot, HeadChunks[ Max( 1, Rand(HeadChunks.Length)) ] );
		}
	}
}


/** Does all of the work of spawning a head chunk **/
simulated function SpawnAHeadChunk( vector HeadLoc, rotator HeadRot, StaticMesh TheMesh )
{
	local KActorSpawnable HeadChunk;
	local vector RandDir;

	RandDir = ( vect(1,0,0) * ( (FRand() * 2.f) - 1.f) ) + ( vect(0,1,0) * ( (FRand() * 2.f) -1.0f) ) + vect(0,0,1);

	//`log( "RandDir: " $ RandDir );

	// Get a cached KActor from our object pool
	HeadChunk = GearGRI(WorldInfo.GRI).GOP.GetKActorSpawnable(HeadLoc + RandDir * 10.f, HeadRot);
	HeadChunk.StaticMeshComponent.SetStaticMesh( TheMesh );
	HeadChunk.StaticMeshComponent.WakeRigidBody();
	HeadChunk.SetCollision(FALSE, FALSE);
	HeadChunk.StaticMeshComponent.SetRBCollidesWithChannel( RBCC_Pawn, TRUE ); // make chunks collide with pawns as it is fun to kick them around!

	// Don't replicate this one
	HeadChunk.RemoteRole = Role_None;

	HeadChunk.ApplyImpulse(RandDir, 2.f + 5.f * FRand(), HeadLoc);

	HeadChunk.SetTimer( 15.f, FALSE, nameof(HeadChunk.Recycle) );
	// turn off updating the DLE after a second
	HeadChunk.SetTimer( 1.0f, FALSE, nameof(HeadChunk.SetLightEnvironmentToNotBeDynamic) );
}

function OnKnockdownPawn(SeqAct_KnockdownPawn Action)
{
	local vector LinearVel, AngularVel;
	// knockdown the pawn
	if (Action.InputLinks[0].bHasImpulse)
	{
		LinearVel = vect(0,0,1) * 384.f * Action.LinearVelocityScale;
		LinearVel.X = int(LinearVel.X);
		LinearVel.Y = int(LinearVel.Y);
		LinearVel.Z = Min(LinearVel.Z,0);	// clamp the z to only push down
		AngularVel = Normal(vector(Rotation) cross vect(0,0,1)) * RandRange(-6.f,-4.f) * Action.AngularVelocityScale;
		Knockdown(LinearVel,AngularVel);
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		// this will automatically get up, or set a timer until the physics comes to rest
		CheckGetUp();
	}
}

/** Stops all animations from playing and saves a list of the active ones to be restarted on the next call to ResumeAllAnimations(). */
final simulated function PauseAllAnimations()
{
	local AnimNodeSequence SeqNode;
	// make sure we aren't already paused
	if (ResumePlayNodes.Length == 0)
	{
		foreach Mesh.AllAnimNodes(class'AnimNodeSequence', SeqNode)
		{
			// if it's currently playing then save it to a list for resuming later
			if( SeqNode.bPlaying )
			{
				ResumePlayNodes.AddItem(SeqNode);
			}
			// stop all nodes from playing
			SeqNode.bPlaying = FALSE;
		}
	}
}

/** Starts playing any animations that were playing at the time PauseAllAnimations() was called. */
final simulated function ResumeAllAnimations()
{
	local AnimNodeSequence SeqNode;
	// Resume nodes playing animation.
	foreach ResumePlayNodes(SeqNode)
	{
		SeqNode.bPlaying = TRUE;
	}
	ResumePlayNodes.Length = 0;
}

/** Stops all animations on character */
simulated function StopAllAnimations()
{
	local AnimNodeSequence	SeqNode;

	foreach Mesh.AllAnimNodes(class'AnimNodeSequence', SeqNode)
	{
		SeqNode.bPlaying = FALSE;
	}

	if( !AnimTree(Mesh.Animations).bUseSavedPose )
	{
		// Save a pose using the current mesh
		AnimTree(Mesh.Animations).SetUseSavedPose(TRUE);
		// Clear references to AnimTree nodes.
		ClearAnimNodes();
		// Now clear the rest of the tree, as we are only going to use the saved pose from now on
		AnimTree(Mesh.Animations).Children[0].Anim = None;
		// When dead, no need to animate when not rendered
		Mesh.bUpdateSkelWhenNotRendered = FALSE;
	}
}

simulated function PlayFallDown()
{
	// abort the weapon reload
	if (GearWeapon(Weapon) != None)
	{
		GearWeapon(Weapon).AbortWeaponReload();
	}

	ReadyPawnForRagdoll();

	if( InitRagdoll() )
	{
		// Move into post so that we are hitting physics from last frame, rather than animated from this
		Mesh.SetTickGroup(TG_PostAsyncWork);
		SetTickGroup(TG_PostAsyncWork);

		// Set physics to NOT be updated from Kinematic data
		Mesh.bUpdateKinematicBonesFromAnimation = FALSE;

		PauseAllAnimations();

		if( bPlayMotorAnimOnKnockDown )
		{
			if( Mesh.PhysicsAssetInstance != None )
			{
				// Make sure all motors are on
				Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(TRUE, TRUE, Mesh, TRUE);
				// Set motor strength
				Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(KnockDownMotorStrength, KnockDownMotorDamping, 0.f, Mesh, TRUE);
			}
			// Want to update joints with anim data
			Mesh.bUpdateJointsFromAnimation = TRUE;

			// Play anim
			BS_Play(BS_KnockDownAnim, 1.0, 0.2, 0.2, TRUE, TRUE);
		}
		else
		{
			if( Mesh.PhysicsAssetInstance != None )
			{
				// Make sure all motors are off on joints.
				Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, Mesh, TRUE);
				// Turn on velocity motors, to add some joint friction
				Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(0.f, 0.f, 0.004f, Mesh, TRUE);
				// Don't set motors on dangly bits.
				Mesh.PhysicsAssetInstance.SetAllMotorsAngularVelocityDrive(TRUE, TRUE, Mesh, TRUE);
			}

			Mesh.bUpdateJointsFromAnimation = FALSE;
		}

		// Record when knockdown happened
		KnockDownStartTime = WorldInfo.TimeSeconds;

		PlayFallDownPhysicsModify();

		// we purposefully do NOT turn collide with RBBC_Pawn on
		// we will innerpentrate other ragdolled pawns as we do not collide with the Pawn channel.  This is acceptable because if we do
		// collide with other ragdolled pawns there is a chance that we will have the physics system forcibly push us apart causing one
		// or both the pawns to fly through the world

		ReduceConstraintLimits();
	}
	else
	{
		`Warn("Failed to init ragdoll for"@self);
	}
}


/** This will modify the physics is the correct way for PlayFallDown **/
simulated function PlayFallDownPhysicsModify()
{
	// Allow all ragdoll bodies to collide with all physics objects (ie allow collision with things marked RigidBodyIgnorePawns)
	//Mesh.SetRBChannel(RBCC_Default);

	Mesh.SetRBChannel(RBCC_Untitled3);
	Mesh.SetRBCollidesWithChannel(RBCC_Untitled3, TRUE);
	Mesh.SetRBCollidesWithChannel(RBCC_Pawn, FALSE);
	Mesh.SetRBCollidesWithChannel(RBCC_DeadPawn, FALSE);
}

/**
 * Check to see if we should recover from ragdoll, called on server and owning client.
 * @fixme - should this be server only?	 client/server could disagree when to start the recovery process
 */
function CheckGetUp()
{
	if( Role == ROLE_Authority && IsInState('KnockedDown') )
	{
		// if barely moving,
		if( VSize(Velocity) < 8.f )
		{
			EndKnockedDownState();
		}
		else
		{
			// otherwise check again shortly
			SetTimer( 0.25f, FALSE, nameof(CheckGetUp) );
		}
	}
}


/** Add Base Linear Velocity Upon Death. Takes into account ClampedBase and InteractionPawn */
simulated function AddBaseLinearVelocityUponWhenGoingRagDoll()
{
	local Actor TestBase;

	// Current Base
	TestBase = Base;
	// Forced Base (movers, like Derrick)
	if( ClampedBase != None )
	{
		TestBase = ClampedBase;
	}
	// Base on InteractionPawn (Hostage)
	if( TestBase != None && TestBase == InteractionPawn )
	{
		TestBase = InteractionPawn.Base;
	}
	// Base when dead
	if( BaseWhenDead != None )
	{
		TestBase = BaseWhenDead;
	}

// 	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Base:" @ Base @ "ClampedBase:" @ ClampedBase @ "BaseWhenDead:" @ BaseWhenDead @ "InteractionPawn:" @ InteractionPawn @ "SpecialMove:" @ SpecialMove);
	if( TestBase != None && !TestBase.bStatic )
	{
// 		`log("=> Setting Velocity" @ TestBase.Velocity @ "from" @ TestBase);
		Mesh.SetRBLinearVelocity(TestBase.Velocity, FALSE);
	}
}

/** Play RagDoll Physics death */
simulated function PlayRagDollDeath(class<GearDamageType> GearDamageType, vector HitLoc, bool bShowGore)
{
	local Vector		ApplyImpulse;
	local TraceHitInfo	HitInfo;

	if( InitRagdoll() )
	{
		if( Mesh.PhysicsAssetInstance != None )
		{
			// Make sure all motors are off on joints.
			Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, Mesh, TRUE);

			// Turn on velocity motors, to add some joint friction
			Mesh.PhysicsAssetInstance.SetAllMotorsAngularDriveParams(0.f, 0.f, 0.004f, Mesh, TRUE);
			Mesh.PhysicsAssetInstance.SetAllMotorsAngularVelocityDrive(TRUE, TRUE, Mesh, TRUE);

			// Make sure all bodies are unfixed
			Mesh.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

			// Add velocity of base upon death.
			AddBaseLinearVelocityUponWhenGoingRagDoll();
		}

		// Set physics to NOT be updated from Kinematic data
		Mesh.bUpdateJointsFromAnimation = FALSE;
		Mesh.bUpdateKinematicBonesFromAnimation = FALSE;

		StopAllAnimations();

		// disable unreal collision on ragdolls
		SetCollision(TRUE, FALSE);

		if( bEnableEncroachCheckOnRagdoll )
		{
			bNoEncroachCheck = FALSE;
		}

		// Wake up rigid body
		Mesh.WakeRigidBody();

		// Allow all ragdoll bodies to collide with all physics objects (ie allow collision with things marked RigidBodyIgnorePawns)
		PlayRagDollDeathPhysicsModify();

		ApplyImpulse = GetImpactPhysicsImpulse(GearDamageType, HitLoc, TearOffMomentum, HitInfo);
		if( !IsZero(ApplyImpulse) )
		{
			Mesh.AddImpulse(ApplyImpulse, HitLoc, HitInfo.BoneName,TRUE);
			Mesh.SetRBLinearVelocity(ApplyImpulse * 2.f,TRUE);
		}

		// Decrease joint limits, to prevent nasty poses once in complete ragdoll.
		if( !bIsGore )
		{
			ReduceConstraintLimits();
		}
	}
	else
	{
		`warn("PlayDying Ragdoll init failed");
		Destroy();
	}
}

simulated function PlayRagDollDeathPhysicsModify()
{
	// Allow all ragdoll bodies to collide with all physics objects (ie allow collision with things marked RigidBodyIgnorePawns)
	Mesh.SetRBChannel(RBCC_DeadPawn);
	Mesh.SetRBCollidesWithChannel(RBCC_Pawn, FALSE);
}

/** Utility for scaling joint limits down of specific bones. */
simulated native function ReduceConstraintLimits();

/**
 * This will spawn a helmet/shoulderpad on pawn classes the override this
 **/
function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );
function RemoveAndSpawnAShoulderPadLeft( Vector ApplyImpulse, class<DamageType> DamageType );
function RemoveAndSpawnAShoulderPadRight( Vector ApplyImpulse, class<DamageType> DamageType );
/** Any attachments this pawn has will be spawned and dropped **/
function RemoveAndDropAttachments( Vector ApplyImpulse, class<DamageType> DamageType );

/** This will update the shadow settings for this pawn's mesh **/
simulated event UpdateShadowSettings( bool bInWantShadow )
{
	local bool bNewCastShadow;
	local bool bNewCastDynamicShadow;

	if( Mesh != None )
	{
		bNewCastShadow = default.Mesh.CastShadow && bInWantShadow;
		bNewCastDynamicShadow = default.Mesh.bCastDynamicShadow && bInWantShadow;

		if( (bNewCastShadow != Mesh.CastShadow) || (bNewCastDynamicShadow != Mesh.bCastDynamicShadow) )
		{
			// if there is a pending Attach then this will set the shadow immediately as the flags have changed an a reattached has occurred
			Mesh.CastShadow = bNewCastShadow;
			Mesh.bCastDynamicShadow = bNewCastDynamicShadow;

			// if we are in a poor framerate situation just change the settings even if people are looking at it
			if( WorldInfo.bAggressiveLOD == TRUE )
			{
				ReattachMesh();
			}
			else
			{
				ReattachMeshWithoutBeingSeen();
			}
		}
	}
}


/** reattaches the mesh component **/
simulated function ReattachMesh()
{
	ClearTimer('ReattachMeshWithoutBeingSeen');
	ReattachComponent(Mesh);
}


/** reattaches the mesh component without being seen **/
simulated function ReattachMeshWithoutBeingSeen()
{
	// defer so we do not pop from any settings we have changed (e.g. shadow settings)
	if( LastRenderTime > WorldInfo.TimeSeconds - 1.0 )
	{
		SetTimer( 0.5 + FRand() * 0.5, FALSE, nameof(ReattachMeshWithoutBeingSeen) );
	}
	// we have not been rendered for a bit so go ahead and reattach
	else
	{
		ReattachMesh();
	}
}


State Dying
{
	ignores Bump, HitWall, HeadVolumeChange, PhysicsVolumeChange, Falling, BreathTimer;

	simulated event FellOutOfWorld(class<DamageType> dmgType)
	{
		Destroy();
	}

	simulated function bool CanBeSpecialMeleeAttacked(GearPawn Attacker)
	{
		return false;
	}

	simulated event BeginState( Name NextStateName )
	{
		// cleanup any DBNO/neardeath audio
		FadeOutAudioComponent(ReviveBreathSound,0.1f);
		FadeOutAudioComponent(FlatLiningSound,0.1f);
		FadeOutAudioComponent(ReviveHeartbeatSound,0.1f);
		FadeOutAudioComponent(NearDeathBreathSound,0.1f);

		Super.BeginState(NextStateName);

		// So here we set how long the corpse should stick around before we start looking ever 2.0 seconds to see if we should destroy it	@see Pawn State Dying
		SetTimer( DurationBeforeDestroyingDueToNotBeingSeen, FALSE );
	}

	simulated function byte GetTeamNum()
	{
		// we won't have a PRI here so just use the stored value
		return LastTeamNum;
	}

	simulated function Tick(float DeltaTime)
	{
		if ( (BurnEffectPSC != None) && !BurnEffectPSC.bIsActive )
		{
			BurnEffectPSC = None;
		}

		FadeSkinEffects(DeltaTime);

		// if frame rate is really bad and Pawn hasn't been rendered since last second, then turn off shadow casting
		if( WorldInfo.bDropDetail )
		{
			UpdateShadowSettings( FALSE );
		}

		// if we are scaling to zero then do so
		if( ( bScalingToZero )  )
		{
			SetDrawScale( DrawScale - DeltaTime );
			if( DrawScale < 0.4 )
			{
				// start falling through the floor as scaling byitself looks pretty lame
				//SetCollision( FALSE, FALSE, FALSE );
				Mesh.SetRBChannel(RBCC_Nothing);
				Mesh.SetRBCollidesWithChannel(RBCC_Nothing, FALSE);
				Mesh.SetRBCollidesWithChannel(RBCC_Default, FALSE);
				Mesh.SetRBCollidesWithChannel(RBCC_DeadPawn, FALSE);
				Mesh.SetRBCollidesWithChannel(RBCC_BlockingVolume, FALSE);
				if( DrawScale < 0.02 )
				{
					bScalingToZero = FALSE;
					if( bDeleteMe == FALSE )
					{
						Destroy();
					}
				}
			}
		}
	}


	/**
	 * Start a special move.
	 * @network: local player and server
	 */
	simulated event DoSpecialMove(ESpecialMove NewMove, optional bool bForceMove=FALSE, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0)
	{
		if( NewMove == SM_None || NewMove == SM_DeathAnim || NewMove == SM_DeathAnimFire )
		{
			Global.DoSpecialMove(NewMove, bForceMove, InInteractionPawn, InSpecialMoveFlags);
		}
		else
		{
			`Warn(WorldInfo.TimeSeconds @ Self @ GetStateName() @ GetFuncName() @ "tried to do special move" $ NewMove $ "while Dying...");
		}
	}

	function TakeDamage(int Damage, Controller InstigatedBy, Vector HitLocation, Vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		local class<GearDamageType>	GearDamageType;

		// make sure we are the server
		// but since we're dead, we have nothing to do on a dedicated server here... all cosmetic!
		if( Role < ROLE_Authority || WorldInfo.NetMode == NM_DedicatedServer )
		{
			return;
		}

		// Don't apply hammer of Dawn damage type every frame, the upward impulse sends bodies flying!
		if( DamageType == class'GDT_HOD' )
		{
			if( TimeSince(LastHODDamageTime) < 1.f )
			{
				return;
			}
			LastHODDamageTime = WorldInfo.TimeSeconds;
		}

		if( VSize(Momentum) > 4.f )
		{
			Momentum = Normal(Momentum);
		}

		GearDamageType = class<GearDamageType>(DamageType);

		// play any associated fx/sounds
		DoDamageEffects(Damage, InstigatedBy != None ? InstigatedBy.Pawn : None, HitLocation, GearDamageType, Momentum, HitInfo);
	}
}


function ShouldCrouch(bool bCrouch)
{
	local ECoverType	DesiredCoverType;

	//debug
	`AILog_Ext( GetFuncName()@bCrouch@bWantsToCrouch@IsInCover(), 'Cover', MyGearAI );

	Super.ShouldCrouch(bCrouch);

	// Use Crouch to toggle between midlevel and standing cover.
	if( IsInCover() )
	{
		// If wants to crouch
		if( bWantsToCrouch )
		{
			// If in standing cover or doing 360 aiming in mid level cover. (Aka pop up)
			if( CoverType == CT_Standing || bDoing360Aiming )
			{
				if( CoverType == CT_Standing )
				{
					// Squat
					SetCoverType(CT_MidLevel);
				}
				else
				{
					SetCoverType(CT_Standing);
				}

				// unhh
				SoundGroup.PlayFoleySound(self, GearFoley_ToCrouchFX, TRUE);
				SoundGroup.PlayEffort(self, GearEffort_CrouchEffort, true);
			}
		}
		// Otherwise, if wants to stand
		else
		{
			// Find desired cover type at slot
			DesiredCoverType = FindCoverType();

			// If our covertype is different than what the cover suggests, go back to default.
			if( CoverType != DesiredCoverType )
			{
				// Stand
				SetCoverType(DesiredCoverType);

				// unhh
				SoundGroup.PlayFoleySound(self, GearFoley_FromCrouchFX, TRUE);
				SoundGroup.PlayEffort(self, GearEffort_CrouchEffort, true);
			}
		}
	}
}


/**
 * Handles setting the a new cover type for this pawn, for both
 * animation and physics purposes.
 *
 * @param	newCoverType - new cover type now active
 */

final simulated function SetCoverType(ECoverType NewCoverType)
{
	local ECoverType OldCoverType;
	local gearai_cover GAIC;

	//debug
	`AILog_Ext( GetFuncName()@"Link C/L/R"@CurrentLink@CurrentSlotIdx@LeftSlotIdx@RightSlotIdx@"CT"@CoverType@"to"@NewCoverType, 'Cover', MyGearAI );

	OldCoverType = CoverType;

	// Do nothing if state isn't changing
	if( NewCoverType != CoverType )
	{
		if( GearPC(Controller) != None )
		{
			GearPC(Controller).CoverLog( "newCoverType:" @ newCoverType, GetFuncName() );
		}

		// Update last cover time
		LastCoverTime = WorldInfo.TimeSeconds;

		// set the enum for animation
		CoverType = NewCoverType;

		if( Role == Role_Authority )
		{
			// Force replication
			bForceNetUpdate = TRUE;

			// Force push out of cover transition for AI characters.
			GAIC = GearAI_Cover(Controller);
			if( NewCoverType == CT_None && GAIC != None && !GAIC.bLeaveCoverFast && !bEndingSpecialMove && CanDoSpecialMove(SM_PushOutOfCover) )
			{
				LocalDoSpecialMove(SM_PushOutOfCover, TRUE);
			}

			// reset bLeaveCoverFast
			if(GAIC != none)
			{
				GAIC.bLeaveCoverFast=false;
			}
		}
	}

	// Rotation interpolation when leaving cover.
	if( OldCoverType != CT_None && CoverType == CT_None )
	{
		// If Pawn is not already doing a rotation interp when leaving cover, then do one.
		if( !IsDoingPawnRotationInterpolation() )
		{
			InterpolatePawnRotation(0.f, TRUE);
		}
	}
}


/**
 * Handles setting the new cover action for this pawn
 *
 * @param	NewCoverAction - new cover action type activated
 */

final simulated function SetCoverAction( ECoverAction NewCoverAction )
{
	local GearPC	MyGearPC;

	// Do nothing if state isn't changing
	if( NewCoverAction != CoverAction )
	{
		MyGearPC = GearPC(Controller);
		if( MyGearPC != None )
		{
			MyGearPC.CoverLog( "NewCoverAction:"@NewCoverAction@"Last"@WorldInfo.TimeSeconds-LastCoverActionTime@"Next"@Abs(NextCoverActionTime)-WorldInfo.TimeSeconds, GetFuncName() );
		}

		// That's only for AI...
		if( Role == Role_Authority && !IsHumanControlled() )
		{
			// If leaning or firing left, don't allow non-left actions for a short time
			if( (CoverAction == CA_LeanLeft || CoverAction == CA_BlindLeft) &&
				(NewCoverAction != CA_LeanLeft && NewCoverAction != CA_BlindLeft) )
			{
				NextCoverActionTime = -(WorldInfo.TimeSeconds + 0.35f);
			}
			// Otherwise, if leaning or firing right, don't allow non-right actions for a short time
			else if( (CoverAction == CA_LeanRight || CoverAction == CA_BlindRight) &&
					(NewCoverAction != CA_LeanRight && NewCoverAction != CA_BlindRight) )
			{
				NextCoverActionTime =  (WorldInfo.TimeSeconds + 0.35f);
			}

			// If not moving to default and not enough time has past since last action
			if( NewCoverAction != CA_Default &&
				Abs(NextCoverActionTime) > WorldInfo.TimeSeconds )
			{
				// If old action was left and new action is not left, go to default
				if( NextCoverActionTime < 0.f &&
					NewCoverAction != CA_LeanLeft &&
					NewCoverAction != CA_BlindLeft )
				{
					NewCoverAction = CA_Default;
				}
				// Otherwise, if old action was right and new action is not right, go to default
				else if( NextCoverActionTime > 0.f &&
						NewCoverAction != CA_LeanRight &&
						NewCoverAction != CA_BlindRight )
				{
					NewCoverAction = CA_Default;
				}
			}
		}

		// Heavy weapons don't allow transitioning from leaning to pop-up and vice versa.
		if( !CanDoLeaningTransition() )
		{
			if( IsALeaningAction(NewCoverAction) && IsALeaningAction(CoverAction) )
			{
				return;
			}
		}

		// Set action
		CoverAction = NewCoverAction;
		// Store time
		LastCoverActionTime = WorldInfo.TimeSeconds;

		if( Role == Role_Authority )
		{
			// Force replication
			bForceNetUpdate = TRUE;
		}

		// CoverAction has been updated!
		CoverActionChanged();
	}
}

simulated function CoverActionChanged()
{
	local Actor A;
	local array<SequenceEvent> TouchEvents;
	local int i;

	// Keep track of last cover action set.
	PreviousCoverAction	= SavedCoverAction;
	SavedCoverAction	= CoverAction;

	// If we've just got out of 360 aiming, then don't reset this flag just now.
	// If time has elapsed, then we don't consider just transitioning from that anymore.
	if( bWasDoing360Aiming && TimeSince(Last360AimingChangeTime) > 0.25f )
	{
		bWasDoing360Aiming = FALSE;
	}
	`LogSM(`location@`showvar(PreviousCoverAction)@`showvar(SavedCoverAction));

	// See if we should mount/unmount heavy weapons.
	CheckHeavyWeaponMounting();

	if (IsPoppingUp())
	{
		SoundGroup.PlayFoleySound(self, GearFoley_Body_Popup);
	}
	else if (IsLeaning())
	{
		SoundGroup.PlayFoleySound(self, GearFoley_Body_Lean);
	}

	// check to see if we need to kick off kismet 'geartouch' events when we change cover action (e.g. we just peaked up, now we want to hit the trigger)
	foreach TouchingActors(class'Actor', A)
	{
		if (A.FindEventsOfClass(class'SeqEvt_GearTouch', TouchEvents))
		{
			for (i = 0; i < TouchEvents.length; i++)
			{
				SeqEvt_GearTouch(TouchEvents[i]).CheckCoverStatusTouchActivate(A,self,true,IsLeaning());
			}
			// clear array for next iteration
			TouchEvents.length = 0;
		}
	}
}

/**
 * Returns TRUE if can do leaning to pop transition and vice versa.
 * Disallowed for Heavy Weapons.
 */
simulated final function bool CanDoLeaningTransition()
{
	if( MyGearWeapon != None && MyGearWeapon.IsA('GearWeap_HeavyBase') )
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Returns true if blind firing.
 */
simulated final function bool IsBlindFiring(ECoverAction PawnCA)
{
	return (PawnCA == CA_BlindRight || PawnCA == CA_BlindLeft || PawnCA == CA_BlindUp);
}


/**
 * Return true if Cover Action is leaning (left/right/up)
 */
simulated final native function bool IsLeaning();

/** Is Pawn Leaning? Note: PopUp is considered leaning. */
simulated final function bool IsALeaningAction(ECoverAction InCoverAction)
{
	return (InCoverAction == CA_LeanLeft || InCoverAction == CA_LeanRight || InCoverAction == CA_PopUp);
}

simulated final function bool IsPeeking( ECoverAction PawnCA )
{
	return (PawnCA == CA_PeekRight || PawnCA == CA_PeekLeft || PawnCA == CA_PeekUp);
}

/** Return true if this pawn is popping up, false otherwise */
simulated final native function bool IsPoppingUp();

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	local GearPC PC;

	if (Role == ROLE_Authority)
	{
		if (!WorldInfo.Game.IsA('GearGameSP_Base'))
		{
			Super.FellOutOfWorld(dmgType);
		}
		else
		{
			if (!ClassIsChildOf(dmgType,class'GearDamageType'))
			{
				dmgType = class'GDT_Environment';
			}
			if (IsA('GearPawn_COGGear'))
			{
				`Warn("COG fell out of world, attempting to teleport to a human player"@self);
				// try to teleport near a human player in this evil case
				foreach LocalPlayerControllers(class'GearPC', PC)
				{
					if (PC.FindAvailableTeleportSpot(self))
					{
						// don't die on me!
						return;
					}
					else if (PC.Pawn != None && IsA('GearPawn_CogDom'))
					{
						// try harder for Dom
						SetLocation(PC.Pawn.Location);
						Velocity = vect(0,0,0);
						return;
					}
				}
			}
			if (PlayerController(Controller) != None)
			{
				if (Anchor != None)
				{
					SetLocation(Anchor.Location);
				}
				else
				{
					SetLocation(class'NavigationPoint'.static.GetNearestNavToActor(self).Location);
				}
			}
			else if (Health > 0)
			{
				Super.FellOutOfWorld(dmgType);
			}
			else
			{
				Super(Actor).FellOutOfWorld(dmgType);
			}
		}
	}
}

/**
 * Called by animation code whenever an action animation has finished.
 */
simulated event CoverActionAnimationFinished()
{
	//@todo - notify ai, so that hard-coded delays can be eliminated
}

function bool InGodMode()
{
	local GearPC PC;
	PC = GearPC(Controller);
	return (Super.InGodMode() || (PC != None && (PC.bCameraGodMode || PC.bCinematicMode)));
}

/**
 * Overridden to return the actual player name from this Pawn's
 * PlayerReplicationInfo (PRI) if available.
 */
function String GetDebugName()
{
	// return the actual player name from the PRI if available
	if (PlayerReplicationInfo != None)
	{
		return PlayerReplicationInfo.PlayerName;
	}
	// otherwise return the formatted object name
	return GetItemName(string(self));
}

/**
 * Called from PlayerController UpdateRotation() -> ProcessViewRotation() to (pre)process player ViewRotation
 * adds delta rot (player input), applies any limits and post-processing
 * returns the final ViewRotation set on PlayerController
 *
 * @param	DeltaTime, time since last frame
 * @param	ViewRotation, actual PlayerController view rotation
 * @input	out_DeltaRot, delta rotation to be applied on ViewRotation. Represents player's input.
 * @return	processed ViewRotation to be set on PlayerController.
 */
simulated function ProcessViewRotation( float DeltaTime, out rotator out_ViewRotation, out Rotator out_DeltaRot )
{
	// Give Inventory Manager a chance to affect player's view rotation
	if( GearInventoryManager(InvManager) != None )
	{
		GearInventoryManager(InvManager).ProcessViewRotation( DeltaTime, out_ViewRotation, out_DeltaRot );
	}

	Super.ProcessViewRotation(DeltaTime, out_ViewRotation, out_DeltaRot);
}

/**
 * Overridden to return camera values specific to GearPawn.
 *
 * @param		RequestedBy
 */
simulated function name GetDefaultCameraMode( PlayerController RequestedBy )
{
	return 'default';
}

/**
 * Native hook to allow factories to give AI the default loadout if none is specified.
 */
event AI_AddDefaultInventory()
{
	AddDefaultInventory();
}

/**
 * Called after spawning from a factory.
 */
event PostAIFactorySpawned(SeqAct_AIFactory SpawningFactory, INT SpawnSetIdx);

/**
 * Overridden to iterate through the DefaultInventory array and
 * give each item to this Pawn.
 *
 * @see			GameInfo.AddDefaultInventory
 */
function AddDefaultInventory()
{
	local class<Inventory> InvClass;
	local Inventory Inv;
	local GearWeapon Weap;

	foreach DefaultInventory(InvClass)
	{
		// Ensure we don't give duplicate items
		Inv = FindInventoryType(InvClass);

		// If item not found
		if( Inv == None )
		{
			// Create it and add to inventory chain, only activate if we don't have a weapon currently selected
			CreateInventory(InvClass,Weapon != None);
		}
		// Otherwise, item already in our inventory chain
		else
		{
			// Add extra ammo to the weapon we already have
			Weap = GearWeapon(Inv);
			if( Weap != None )
			{
				Weap.InitializeAmmo();
			}
		}
	}
}

simulated function OnGiveInventory(SeqAct_GiveInventory InAction)
{
	local int Idx;
	local class<Inventory> InvClass;
	local bool bOldAllowDrops;
	local Inventory NewInv, OldInv;
	local class<GearWeapon> GearWeapClass;

	bOldAllowDrops = bAllowInventoryDrops;
	bAllowInventoryDrops = FALSE;

	if (MyGearAI != None)
	{
		Super.OnGiveInventory(InAction);

		// see if he got a shield, and equip it if so
		for (Idx = 0; Idx < InAction.InventoryList.Length; Idx++)
		{
			if (ClassIsChildOf(InAction.InventoryList[idx], class'GearShield'))
			{
				EquipShield(GearInventoryManager(InvManager).Shield);
				break;
			}
		}
	}
	else
	{
		if (InAction.bClearExisting)
		{
			InvManager.DiscardInventory();
		}
		if (InAction.InventoryList.Length > 0 )
		{
			for (Idx = 0; Idx < InAction.InventoryList.Length; Idx++)
			{
				InvClass = InAction.InventoryList[idx];
				if (InvClass != None)
				{
					// only create if it doesn't already exist
					if (FindInventoryType(InvClass,FALSE) == None)
					{
						// look for an empty slot
						if (GearInventoryManager(InvManager).FindFreeSlotForInventoryClass(InvClass) == EASlot_None)
						{
							`log("no free slot");
							// if occupied try to drop the old weapon
							GearWeapClass = class<GearWeapon>(InvClass);
							if (InAction.bForceReplace || GearWeapClass == None)
							{
								OldInv = GearInventoryManager(InvManager).GetInventoryInSlot(GearInventoryManager(InvManager).GetSlotFromType(GearWeapClass));
								`log("tossing"@`showvar(OldInv));
								TossInventory(OldInv);
							}
							else
							{
								// not allowed to replace or not a gearweapon so can't replace this item
								continue;
							}
						}
						// create the new item
						NewInv = InvManager.CreateInventory(InvClass);
// 						`DLog("created" @ `showvar(NewInv));

						if (ClassIsChildOf(InvClass, class'GearShield'))
						{
							EquipShield(GearShield(NewInv));
						}
						// select the new item
						else if (InAction.bForceReplace && GearWeapon(NewInv) != None)
						{
							if (IsCarryingShield())
							{
								DropShield();
							}
							if (!IsLocallyControlled())
							{
								GearWeapon(NewInv).ClientWeaponSet(false);
							}
							else
							{
								InvManager.SetCurrentWeapon(GearWeapon(NewInv));
							}
						}
					}
				}
				else
				{
					InAction.ScriptLog("WARNING: Attempting to give NULL inventory!");
				}
			}
		}
	}

	bAllowInventoryDrops = bOldAllowDrops;
}



/**
 * This will make default inventory for TDM games.
 *
 * @see GearGameDM.RestartPlayer()
 **/
function AddDefaultInventoryTDM()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local UIProviderScriptFieldValue WeaponValue;
	local int ProviderIndex;
	local GearGameWeaponSummary WeaponInfo;
	local class<Inventory> InvClass;
	local string WeaponClassPath;
	local int InitWeaponId;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None )
	{
		// Find the provider id for the weapon
		WeaponValue.PropertyTag = 'WeaponId';
		WeaponValue.PropertyType = DATATYPE_Property;
		InitWeaponId = GearPRI(PlayerReplicationInfo).InitialWeaponType;
		WeaponValue.StringValue = string(InitWeaponId);
		ProviderIndex = GameResourceDS.FindProviderIndexByFieldValue('Weapons', 'WeaponId', WeaponValue);
		// Found it
		if (ProviderIndex != INDEX_NONE)
		{
			WeaponInfo = GameResourceDS.GetWeaponProvider( ProviderIndex );
			WeaponClassPath = WeaponInfo.ClassPathName;
		}
		// Didn't find it... hardcode the gold weapons, sigh
		else
		{
			if (InitWeaponId == 17)
			{
				WeaponClassPath = "GearGameContent.GearWeap_AssaultRifle_Golden";
			}
			else if (InitWeaponId == 18)
			{
				WeaponClassPath = "GearGameContent.GearWeap_LocustAssaultRifle_Golden";
			}
		}
	}

	// use the selected value from the PRI
	if ( WeaponClassPath != "" )
	{
		InvClass = class<Inventory>(FindObject( WeaponClassPath, class'Class' ));
		if ( InvClass != None )
		{
			CreateInventory( InvClass, FALSE );
		}
		else
		{
			CreateInventory( class'GearGame.GearWeap_AssaultRifle', FALSE );
		}
	}
	else
	{
		CreateInventory( class'GearGame.GearWeap_AssaultRifle', FALSE );
	}
	CreateInventory( class'GearGame.GearWeap_Shotgun', FALSE );
	CreateInventory( class'GearGame.GearWeap_COGPistol',TRUE);
	CreateInventory( class'GearGame.GearWeap_SmokeGrenade',TRUE);
}

/**
 * Perform a trace on and to crosshair.
 * the specified Range is calculated so it's relative to the pawn's location and not the camera location (since camera is variable).
 * @note	if HitActor == None, out_HitLocation == EndTrace
 *
 * @param	fRange				trace range from player pawn
 * @output	out_HitLocation		location of hit. returns EndTrace location if no hit.
 * @output	out_HitNormal		hit normal.
 * @param	TraceExtent			extent of trace
 *
 * @return	HitActor			returns hit actor.
 */
final simulated function Actor TraceWithPawnOffset( float fRange, out vector out_HitLocation, out vector out_HitNormal, optional vector TraceExtent, optional bool bSkipActors )
{
	local Vector	StartTrace, EndTrace;
	local Rotator	AimRot;
	local Actor		HitActor;

	// Get adjusted start trace postion. From camera world location + offset to range is constant from pawn.
	StartTrace	= GetWeaponStartTraceLocation();
	// Get base, non corrected, aim direction.
	AimRot		= GetBaseAimRotation();
	// define end trace
	EndTrace	= StartTrace + vector(AimRot) * fRange;
	// perform trace
	HitActor	= Trace(out_HitLocation, out_HitNormal, EndTrace, StartTrace, !bSkipActors, TraceExtent);

	// If we didn't hit anything, then set the HitLocation as being the EndTrace location
	if ( HitActor == None )
	{
		out_HitLocation = EndTrace;
	}
	return HitActor;
}


/**
 * We override this as we have pawns leaning out of cover and their view location is really where
 * they are peeking out from.  We are using the NeckBone location as that is what we are using in other
 * parts of the code.  We should probably make a more general socket based mechanic for this.
 **/
simulated native event Vector GetPawnViewLocation();

/**
 * return muzzle location, pulled into pawn's collision cylinder along -AimDir
 */
final simulated function vector GetPulledInMuzzleLocation(GearWeapon WW, vector AimDir)
{
	local float MuzzleDist;
	local vector MuzzleLoc, PulledInMuzzleLoc, ExtraPullIn, HitLocation, HitNormal;
	local actor HitActor;

	// hack for sniper rifle to put first person camera and shot origin
	// at a nice spot.	here, we're putting at the front of the scope, which
	// minimizes obvious aim error shooting downward and still makes it so that
	// a player must expose himself at least somewhat to snipe
	if (GearWeap_SniperRifle(WW) != None)
	{
		MuzzleLoc = SkeletalMeshComponent(WW.Mesh).GetBoneLocation('b_LS_LensCap');
		MuzzleDist = VSize2D(MuzzleLoc - Location);

		// make sure this position isn't clipping into a wall
		HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, MuzzleLoc, MuzzleLoc - MuzzleDist * AimDir, false,vect(12,12,12),,TRACEFLAG_Bullet);
		return (HitActor == None) ? MuzzleLoc : HitLocation;
	}

	MuzzleLoc = GetPhysicalFireStartLoc( ((WW != None)?WW.FireOffset:vect(0,0,8)) );

	// is muzzle outside pawn's collision cylinder?
	MuzzleDist = VSize2D(MuzzleLoc - Location);
	if ( MuzzleDist > CylinderComponent.CollisionRadius )
	{
		// pull MuzzleLoc back toward cylinder
		PulledInMuzzleLoc = MuzzleLoc - MuzzleDist * AimDir;
		MuzzleDist = VSize2D(PulledInMuzzleLoc - Location);
		if ( MuzzleDist < CylinderComponent.CollisionRadius )
		{
			MuzzleLoc = PulledInMuzzleLoc;
		}
		else
		{
			// only pull in if needed!
			HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, PulledInMuzzleLoc, Location, false,,,TRACEFLAG_Bullet);
			if ( HitActor != None )
			{
				ExtraPullIn = Location - PulledInMuzzleLoc;
				ExtraPullIn.Z = 0;
				ExtraPullIn = (2.0 + MuzzleDist - CylinderComponent.CollisionRadius) * Normal(ExtraPullIn);
			}
			MuzzleLoc = PulledInMuzzleLoc + ExtraPullIn;
		}
	}

	return MuzzleLoc;
}

simulated function SetWeaponStartTraceLocationCache(Weapon ValidWeap, Vector StartLoc)
{
	WeaponStartTraceLocationCache.bUpToDate=true;
	WeaponStartTraceLocationCache.StartLoc = StartLoc;
	WeaponStartTraceLocationCache.WeaponCacheIsValidFor = ValidWeap;
}

//`define GetWeaponStartTraceLocationPerf
`if(`isdefined(GetWeaponStartTraceLocationPerf))
`define	CClock(Time) Clock(`Time)
`define CUnClock(Time) UnClock(`Time)
`else
`define CUnClock(Time)
`define	CClock(Time)
`endif
/**
 * Return world location to start a weapon fire trace from.
 *
 * @return	World location where to start weapon fire traces from
 */
simulated event Vector GetWeaponStartTraceLocation(optional Weapon CurrentWeapon)
{
	local GearWeapon WW;
	local vector MuzzleLoc, StartLoc, EndTrace, HitLocation, HitNormal, AimDir;
	local Actor HitActor;
	local GearPC GPC;
	local float SmallTraceDist;

	`if(`isdefined(GetWeaponStartTraceLocationPerf))
		local float Overall,SumTotal;
		local float Time[20];
		local int Idx;
	`endif

	`CClock(Overall);
	`CClock(Time[0]);
	if(WeaponStartTraceLocationCache.bUpToDate && WeaponStartTraceLocationCache.WeaponCacheIsValidFor == CurrentWeapon)
	{
		return WeaponStartTraceLocationCache.StartLoc;
	}

	// if not owned by a human player shoot from weapon muzzle
	if( !IsHumanControlled() )
	{
		StartLoc = super.GetWeaponStartTraceLocation();
		SetWeaponStartTraceLocationCache(CurrentWeapon,StartLoc);
		return StartLoc;
	}

	if (CurrentWeapon != None)
	{
		WW = GearWeapon(CurrentWeapon);
	}
	if (WW == None)
	{
		WW = MyGearWeapon;
	}

	if (WW == None)
	{
		StartLoc = super.GetWeaponStartTraceLocation();
		SetWeaponStartTraceLocationCache(CurrentWeapon,StartLoc);
		return StartLoc;
	}

	if ( WW.UseFirstPersonCamera() )
	{
		StartLoc = super.GetWeaponStartTraceLocation();
		AimDir = vector(GetAdjustedAimFor(WW, StartLoc));
		MuzzleLoc = GetPulledInMuzzleLocation(WW, AimDir);
		SetWeaponStartTraceLocationCache(CurrentWeapon,MuzzleLoc);
		return MuzzleLoc;
	}
	`CUnClock(Time[0]);

	`CClock(Time[1]);
	GPC = GearPC(Controller);
	StartLoc = super.GetWeaponStartTraceLocation();
	`CUnClock(Time[1]);
	if( GPC != None )
	{
		`CClock(Time[2]);
		AimDir = vector(GetAdjustedAimFor(WW, StartLoc));
		`CUnClock(Time[2]);
		`CClock(Time[8]);
		StartLoc = WW.AdjustStartTraceLocation(StartLoc, AimDir);		// see if the Weapon wants to tweak it
		`CUnClock(Time[8]);
		`CClock(Time[9]);
		MuzzleLoc = GetPulledInMuzzleLocation(WW, AimDir);
		`CUnClock(Time[9]);


		`CClock(Time[3]);
		// first check to see if there is something intervening camera -> muzzle
		SmallTraceDist = VSize(StartLoc - MuzzleLoc);
		HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, StartLoc + AimDir * SmallTraceDist, StartLoc, true,,,TRACEFLAG_Bullet);
		`CUnClock(Time[3]);
		`CClock(Time[4]);
		if (Pawn(HitActor) != None)
		{
			// adjust the trace forward to avoid this case
			StartLoc = StartLoc + AimDir * SmallTraceDist;
		}
		else
		// if shooting from the hip or blindfiring then always shoot from the muzzle
		if ( !bIsTargeting || IsBlindFiring(CoverAction) )
		{
			StartLoc = MuzzleLoc;
		}
		else
		if ( !IsInCover() && IsAttemptingToShootThroughWall(WW,StartLoc + (AimDir * ShootingThroughWallTraceDistance)) )
		{
			`CUnClock(Time[4]);
			`CClock(Time[5]);
			// if not in cover, only do corrected trace if can successfully trace from weapon muzzle
			EndTrace = StartLoc + AimDir * WW.GetTraceRange();
			HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartLoc, false,,,TRACEFLAG_Bullet);
			`CUnClock(Time[5]);

			`CClock(Time[6]);
			if ( HitActor != None )
			{
				EndTrace = HitLocation - AimDir * 32;
			}

			HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, MuzzleLoc, false,,,TRACEFLAG_Bullet);

			StartLoc = (HitActor == None) ? StartLoc : MuzzleLoc;
			`CUnClock(Time[6]);
		}
		else
		{
			`CUnClock(Time[4]);
			`CClock(Time[7]);
			// in cover, pull in muzzle location only if it allows you to hit close proximity enemy
			HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, MuzzleLoc + 128*AimDir, MuzzleLoc, true,,,TRACEFLAG_Bullet);
			if (Pawn(HitActor) != None && !IsSameTeam(Pawn(HitActor)))
			{
				StartLoc = MuzzleLoc;
			}
			`CUnClock(Time[7]);
		}
	}
	`CUnClock(Overall);
	`if(`isdefined(GetWeaponStartTraceLocationPerf))
		`log(GetFuncName()@"STATS _--------------------------------------------------_");
		for(Idx=0;Idx<20;Idx++)
		{
			`log(GetFuncName()@" -- STAT ["$Idx$"]"@Time[Idx]@"("@(Time[Idx]/Overall)*100.f@"%)");
			SumTotal += Time[Idx];
		}
		`log(GetFuncName()@"TOTALS --------------- Overall:"@Overall@"Error:"@Abs(Overall-SumTotal));
	`endif
	SetWeaponStartTraceLocationCache(CurrentWeapon,StartLoc);
	return StartLoc;
}


/**
 * This will check to see if the pawn is attempting to shoot through a wall.
 * It takes the Muzzle Location and then pulls back down the weapon length.	 And then traces forward
 * the distance of the pull back plus an amount which will catch "angle" / "wall shooter" players.
 *
 * Using this also stops the shooting something at a distances and having the angle of the
 * shot be different from where you are aiming such that your bullets will impact the wall even
 * tho you have a clear shot at the target with the reticle.
 *
 * There are still issues with too wide walls when you are shooting downwards.	(solution is to
 * reduce the width of that cover.	(c.f. mp_escalation)
 *
 * Overall this will stop most of the egregious "I should be able to shoot like this"
 **/
final simulated function bool IsAttemptingToShootThroughWall( GearWeapon WW, vector EndPoint )
{
	local vector HitLocation, HitNormal;
	local Actor HitActor;
	local float ToWallDistance,PullBackDistance;
	local vector EndTrace, StartTrace;
	local vector Loc, Dir;
	local rotator Rot;

	// grenades don't have a muzzle and use a simulated grenade projectile
	if( GearWeap_GrenadeBase(WW) == none && WW != None )
	{
		PullBackDistance = 100.0f;
		ToWallDistance = PullBackDistance + ShootingThroughWallTraceDistance;

		WW.GetMuzzleLocAndRot(Loc,Rot);
		Dir = Normal(EndPoint - Loc);
		StartTrace = Loc + (Dir * -PullBackDistance); // X is pointing forward always
		EndTrace = StartTrace + (Dir * ToWallDistance);

		// if we hit something close that means bullets coming from our gun would have hit something
		HitActor = WW.GetTraceOwner().Trace(HitLocation, HitNormal, EndTrace, StartTrace, false,,,TRACEFLAG_Bullet);

		if( HitActor == None )
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}

simulated function Rotator GetAdjustedAimFor( Weapon W, vector StartFireLoc )
{
	// If controller doesn't exist or we're a client, get the where the Pawn is aiming at
	// (unless we are riding a pawn, like a Brumak... then we need to actually get good information)
	if ( Controller == None || (Role < Role_Authority && Pawn(Base) == None) )
	{
		return GetBaseAimRotation();
	}

	// otherwise, give a chance to controller to adjust this Aim Rotation
	return Controller.GetAdjustedAimFor( W, StartFireLoc );
}

/************************************************************************************
 * Interactions
 ***********************************************************************************/

/**
 * Check to see if there is any Pawn around us that we can perform an Interaction with.
 * This function will stop at first valid interaction found.
 * For Human Players, called from ServerUse() function.
 *
 * Returns TRUE if an Interaction has been found.
 * @param	out_SpecialMove		Found special move (interaction)
 * @param	out_InteractionPawn	Found Pawn to perform interaction with.
 */
final simulated function bool CheckPawnToPawnInteractions(out ESpecialMove out_SpecialMove, out GearPawn out_InteractionPawn)
{
	local Actor 		HitA;
	local GearPawn		FoundPawn;
	local ESpecialMove	InteractionSM;
	local vector 		HitL, HitN;
	local bool			bInteractionObstructed;

	// Check for Nearby Pawns
	//ForEach VisibleCollidingActors(class'Actor', FoundActor, 64.f, Location, TRUE, vect(32,32,32), TRUE)
	foreach WorldInfo.AllPawns(class'GearPawn',FoundPawn,Location,128.f)
	{
		// Make sure FoundPawn is in a state that can be interacted with
		// This is kept relatively simple, so the SpecialMove's CanInteractWith() function implements all the specifics
		if( FoundPawn == Self || FoundPawn.bDeleteMe || FoundPawn.bPlayedDeath )
		{
			continue;
		}

		// Iterate through all possible Pawn to Pawn interactions
		bInteractionObstructed = FALSE;
		foreach PawnToPawnInteractionList(InteractionSM)
		{
			// See if we can perform an interaction with this Pawn
			if( CanDoSpecialMovePawnInteraction(InteractionSM, FoundPawn) )
			{
				// verify no intervening geometry
				//@note - delayed until we have a successful interaction since this will be the most expensive portion
				foreach TraceActors(class'Actor',HitA,HitL,HitN,FoundPawn.Location,Location,vect(32,32,32))
				{
					if (HitA != FoundPawn && HitA.bBlockActors)
					{
						//`log(self@GetFuncName()@"interaction obstructed to"@`showvar(FoundPawn)@"due to"@`showvar(HitA));
						bInteractionObstructed = TRUE;
						break;
					}
				}
				if (!bInteractionObstructed)
				{
					// Found an interaction!
					out_InteractionPawn = FoundPawn;
					out_SpecialMove		= InteractionSM;
					return TRUE;
				}
				else
				{
					// since the interaction is now obstructed skip further checks
					break;
				}
			}
		}
	}

	return FALSE;
}


/**
 * See if a Special Move Pawn To Pawn Interaction can be done.
 */
final simulated function bool CanDoSpecialMovePawnInteraction(ESpecialMove InSpecialMove, GearPawn InInteractionPawn)
{
	// Make sure the weapon (if there is one) isn't marked as no interactions
	if ( MyGearWeapon != None && MyGearWeapon.bNoInteractionWhileEquipped )
	{
		return FALSE;
	}

	// Make sure special move has been instanced.
	VerifySMHasBeenInstanced(InSpecialMove);

	return (SpecialMoves[InSpecialMove].CanInteractWithPawn(InInteractionPawn) && CanDoSpecialMove(InSpecialMove));
}

/**
 * Start a Pawn to Pawn interaction.
 * @param	InSpecialMove		SpecialMove to do (interaction)
 * @param	InInteractionPawn	Pawn to do interaction with.
 */
final reliable server function DoPawnToPawnInteraction(ESpecialMove InSpecialMove, GearPawn InInteractionPawn, optional bool bForceMove)
{
	`logSM(InSpecialMove @ "InteractionPawn:" @ InInteractionPawn);

	// Verify that we can do the Interaction on the server
	if( bForceMove || CanDoSpecialMovePawnInteraction(InSpecialMove, InInteractionPawn) )
	{
		ServerDoSpecialMove(InSpecialMove, bForceMove, InInteractionPawn);
	}
`if(`notdefined(FINAL_RELEASE))
	else
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ class @ "CanDoSpecialMovePawnInteraction failed!");
		ScriptTrace();
	}
`endif
}


/************************************************************************************
 * CQC (Close Quarter Combat)
 ***********************************************************************************/

reliable server function ServerDoCQCMove(GearTypes.EGameButtons Button)
{
	// Y Button == Long Taunt Executions
	if( Button == GB_Y )
	{
		if( IsCarryingShield() )
		{
			if( CanDoSpecialMove(SM_CQCMove_Shield) )
			{
				ServerDoSpecialMove(SM_CQCMove_Shield, TRUE, GSM_CQC_Killer_Base(SpecialMoves[SM_CQCMove_Shield]).Follower);
			}
		}
		else
		{
			if( CanDoSpecialMove(SM_CQCMove_PunchFace) )
			{
				ServerDoSpecialMove(SM_CQCMove_PunchFace, TRUE, GSM_CQC_Killer_Base(SpecialMoves[SM_CQCMove_PunchFace]).Follower);
			}
		}
		return;
	}
	// B Button == Quick melee executions. Doesn't block movement.
	else if( Button == GB_B )
	{
		if( CanDoSpecialMove(SM_CQCMove_B) )
		{
			ServerDoSpecialMove(SM_CQCMove_B, TRUE, GSM_CQC_Killer_Base(SpecialMoves[SM_CQCMove_B]).Follower);
		}
	}
}

/**
 * Are we allowing this Pawn to be based on us?
 * Allow Interaction Pawns to be based, since we do a lot of attachment stuff there.
 * @note: Made simulated so locally owned client hostages don't switch to PHYS_Falling making them lose their base.
 */
simulated function bool CanBeBaseForPawn(Pawn APawn)
{
	return bCanBeBaseForPawns || (InteractionPawn == APawn);
}


/************************************************************************************
 * Special Moves
 ***********************************************************************************/

// Retrieve "CurrentSlotIdx" by picking one closest to current slot pct
final simulated function int GetSlotIdxByPct()
{
	if( CurrentSlotPct < 0.5f )
	{
		return LeftSlotIdx;
	}

	return RightSlotIdx;
}


/** Make sure a special move has been instanced */
simulated final function bool VerifySMHasBeenInstanced(ESpecialMove AMove)
{
	if( AMove != SM_None )
	{
		if( AMove >= SpecialMoves.Length || SpecialMoves[AMove] == None )
		{
			if( AMove < SpecialMoveClasses.Length && SpecialMoveClasses[AMove] != None )
			{
				SpecialMoves[AMove] = new(Outer) SpecialMoveClasses[AMove];

				// Cache a reference to the owner to avoid passing parameters around.
				SpecialMoves[AMove].PawnOwner = Self;
			}
			else
			{
				`log(GetFuncName() @ "Failed with special move:" @ AMove @ "class:" @ SpecialMoveClasses[AMove] @ Self);
				SpecialMoves[AMove] = None;
				return FALSE;
			}
		}
		return TRUE;
	}
	return FALSE;
}


/**
 * Convenience function which takes special move params and returns a SMStruct.
 */
simulated final function SMStruct FillSMStructFromParams(ESpecialMove InSpecialMove, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0)
{
	local SMStruct	OutSpecialMoveStruct;

	OutSpecialMoveStruct.SpecialMove = InSpecialMove;
	OutSpecialMoveStruct.InteractionPawn = InInteractionPawn;
	OutSpecialMoveStruct.Flags = InSpecialMoveFlags;

	return OutSpecialMoveStruct;
}

/** Convenience function to spit out a SpecialMoveStruct into a String */
simulated final function String SMStructToString(SMStruct InSMStruct)
{
	return "[SpecialMove:" @ InSMStruct.SpecialMove $ ", InteractionPawn:" @ InSMStruct.InteractionPawn $ ", SpecialMoveFlags:" @ InSMStruct.Flags$"]";
}

simulated final function String SpecialMoveToString(ESpecialMove InSpecialMove, GearPawn InInteractionPawn, INT InSpecialMoveFlags)
{
	return "[SpecialMove:" @ InSpecialMove $ ", InteractionPawn:" @ InInteractionPawn $ ", SpecialMoveFlags:" @ InSpecialMoveFlags $ "]";
}

/**
 * Start a special move.
 * @Note this doesn't handle replication to owning client if called from server.
 * See ServerDoSpecialMove() and LocalDoSpecialMove() for alternatives.
 * @network: local player and server
 */
simulated event DoSpecialMove(ESpecialMove NewMove, optional bool bForceMove, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags)
{
	local ESpecialMove	PrevMove;
	local SMStruct		NewMoveStruct;
	local GearPRI 		GPRI;

	`LogSM("New special move requested:" @ SpecialMoveToString(NewMove, InInteractionPawn, InSpecialMoveFlags) @ "bForceMove:" @ bForceMove);

	// ignore redundant calls to the same move
	if( NewMove == SpecialMove )
	{
		`LogSM("- ignoring redundant call");
		return;
	}

	// Make sure NewMove is instanced.
	if( NewMove != SM_None && !VerifySMHasBeenInstanced(NewMove) )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "couldn't instance special move" @ NewMove);
		return;
	}

	if (Role == ROLE_AUTHORITY)
	{
		// @@STATS
		// Store how much time we ran if we are ending roadie run
		if (SpecialMove == SM_RoadieRun)
		{
			GPRI = GearPRI(PlayerReplicationInfo);
			GPRI.TotalRoadieRunTime += WorldInfo.TimeSeconds - GPRI.RoadieRunStartTime;
		}
	}

	// Create struct for new move.
	NewMoveStruct = FillSMStructFromParams(NewMove, InInteractionPawn, InSpecialMoveFlags);

	// If we're currently in the process of ending the current move
	if( bEndingSpecialMove )
	{
		// Then force the new request to pending
		`LogSM("- bEndingSpecialMove, setting" @ NewMove @ "as pending specialmove, previous PendingSpecialMove:" @ SMStructToString(PendingSpecialMoveStruct));
		PendingSpecialMoveStruct = NewMoveStruct;
		return;
	}

	// if currently doing a special move and not a normal end or is a forced move
	if( SpecialMove != SM_None && !bForceMove && NewMove != SM_None )
	{
		// See if we can override current special move, otherwise just queue new one until current is finished.
		if( SpecialMoves[SpecialMove].CanOverrideMoveWith(NewMove) || SpecialMoves[NewMove].CanOverrideSpecialMove(SpecialMove) )
		{
			bForceMove = TRUE;
			`LogSM("- Overriding" @ SpecialMove @ "with" @ NewMove @ "(previous pending:" @ SMStructToString(PendingSpecialMoveStruct) $ ")");
		}
		else
		{
			// extra check to see if we can chain since non-owning clients can call DoSpecialMove directly in certain cases
			if( SpecialMoves[SpecialMove].CanChainMove(NewMove) )
			{
				`LogSM("- chaining" @ NewMove @ "after" @ SpecialMove @ "(previous pending:" @ SMStructToString(PendingSpecialMoveStruct) $ ")");
				PendingSpecialMoveStruct = NewMoveStruct;
			}
			else
			{
				`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Cannot override, cannot chain." @ NewMove @ "is lost! SpecialMove:" @ SpecialMove @ "Pending:" @ SMStructToString(PendingSpecialMoveStruct) );
			}
			return;
		}
	}

	// Check that we can do special move and special move has been/can be instanced
	if( NewMove != SM_None && !bForceMove && !CanDoSpecialMove(NewMove) )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "cannot do requested special move" @ NewMove);
		return;
	}

	PrevMove = SpecialMove;

	// Stop previous special move
	if( SpecialMove != SM_None )
	{
		`LogSM("- leaving move:" @ SpecialMove);
		bEndingSpecialMove = TRUE;
		// clear the special move so that checks like IsDoingSpecialMove and IsEvading no longer pass
		SpecialMove = SM_None;
		SpecialMoveEnded(PrevMove, NewMove);
		bEndingSpecialMove = FALSE;
	}

	// Set new special move
	`LogSM("- starting move:" @ NewMove);
	SpecialMove = NewMove;
	InteractionPawn = InInteractionPawn;
	SpecialMoveFlags = InSpecialMoveFlags;
// 	ScriptTrace();

	if ( ((WorldInfo.NetMode != NM_Standalone || WorldInfo.IsRecordingDemo()) && Role == ROLE_Authority) &&
		(SpecialMove == SM_None || SpecialMoves[SpecialMove].ShouldReplicate()) )
	{
		// Force replication now to non-owning clients
		ReplicatedSpecialMoveStruct = NewMoveStruct;
		bForceNetUpdate = TRUE;
	}

	// Notification of a special move state change.
	SpecialMoveAssigned(NewMove, PrevMove);

	// if it's a valid special move
	if( NewMove != SM_None )
	{
		// notify the special move it should start
		SpecialMoveStarted(NewMove, PrevMove, bForceMove);

		// if this was a forced move clear any pending moves since this was an interrupt of the current move
		if( bForceMove )
		{
			PendingSpecialMoveStruct = FillSMStructFromParams(SM_None, None, 0);
		}
		// track the event for feedback
		if (WorldInfo.NetMode != NM_Client)
		{
			if (SpecialMove == SM_RoadieRun)
			{
				GearPRI(PlayerReplicationInfo).RoadieRunStartTime = WorldInfo.TimeSeconds;
			}
			`RecordStat(STATS_LEVEL4,'SpecialMoveStarted', Controller, SpecialMove, PendingSpecialMoveStruct.InteractionPawn != None ? PendingSpecialMoveStruct.InteractionPawn.Controller : None);
		}
	}
	else
	// otherwise start the pending special move
	if( PendingSpecialMoveStruct.SpecialMove != SM_None )
	{
		`LogSM("- triggering pending special move:" @ SMStructToString(PendingSpecialMoveStruct));
		NewMoveStruct = PendingSpecialMoveStruct;
		PendingSpecialMoveStruct = FillSMStructFromParams(SM_None, None, 0);
		DoSpecialMoveFromStruct(NewMoveStruct, FALSE);
	}
}

/**
 * Convenience function that takes a SpecialMoveStruct, and calls DoSpecialMove() from its parameters.
 */
simulated final function DoSpecialMoveFromStruct(SMStruct InSpecialMoveStruct, optional bool bForceMove)
{
	DoSpecialMove(InSpecialMoveStruct.SpecialMove, bForceMove, InSpecialMoveStruct.InteractionPawn, InSpecialMoveStruct.Flags);
}

/**
 * Have a locally controlled Pawn start a special move.
 * PlayerControllers will automatically replicate the move to the server. Will then be replicated to ALL.
 * AI is already executing those on the server, so they're all replicated.
 */
simulated function LocalDoSpecialMove(ESpecialMove NewMove, optional bool bForceMove=FALSE, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0)
{
	if( !IsLocallyControlled() )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "not called on a locally controlled Pawn!" @ NewMove);
		ScriptTrace();
		return;
	}

	if( bForceMove || CanDoSpecialMove(NewMove) )
	{
		if( Controller.IsA('GearPC') )
		{
			GearPC(Controller).DoSpecialMove(NewMove, bForceMove, InInteractionPawn, InSpecialMoveFlags);
		}
		else
		{
			DoSpecialMove(NewMove, TRUE, InInteractionPawn, InSpecialMoveFlags);
		}
	}
	else
	{
		`logSM("Couldn't start SpecialMove:" @ NewMove);
	}
}

/** Start a SpecialMove on the Server, and have it locally replicated to owning client. */
simulated function ServerDoSpecialMove(ESpecialMove NewMove, optional bool bForceMove=FALSE, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0, optional ESpecialMove SMToChain)
{
	`logSM( SpecialMoveToString(NewMove, InInteractionPawn, InSpecialMoveFlags) );

	if( WorldInfo.NetMode == NM_Client && !bTearOff )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "called on client!!" @ NewMove);
		ScriptTrace();
		return;
	}

	if( bForceMove || CanDoSpecialMove(NewMove) )
	{
		if( GearPC(Controller) != None )
		{
			GearPC(Controller).ServerDictateSpecialMove(NewMove, InInteractionPawn, InSpecialMoveFlags);
		}
		else
		{
			DoSpecialMove(NewMove, TRUE, InInteractionPawn, InSpecialMoveFlags);
		}
		if (SMToChain != SM_None)
		{
			DoSpecialMove(SMToChain,FALSE);
		}
	}
	else
	{
		`logSM("Couldn't start SpecialMove:" @ NewMove);
	}
}

/**
 * Return TRUE if Special Move can be performed
 * @param bForceCheck - Allows you to skip the single frame condition (which will be incorrect on clients since LastCanDoSpecialMoveTime isn't replicated)
 */
simulated final event bool CanDoSpecialMove(ESpecialMove AMove, optional bool bForceCheck)
{
	// if it is a valid move and we have a class for the move
	if (Physics != PHYS_RigidBody && AMove != SM_None && SpecialMoveClasses.length > AMove && SpecialMoveClasses[AMove] != None)
	{
		// Make sure special move is instanced
		if( VerifySMHasBeenInstanced(AMove) )
		{
			// and check the instance
			return (CanChainSpecialMove(AMove) && SpecialMoves[AMove].CanDoSpecialMove(bForceCheck));
		}
		`log(GetFuncName() @ "Failed with special move:" @ AMove @ "class:" @ SpecialMoveClasses[AMove] @ Self);
	}
	return FALSE;
}


/** Returns TRUE if player is current performing AMove. */
simulated final native function bool IsDoingSpecialMove(ESpecialMove AMove) const;

/** Returns TRUE if Pawn is doing a special move */
simulated final native function bool IsDoingASpecialMove() const;

simulated final native function bool IsDoingMeleeHoldSpecialMove() const;

/** Returns TRUE if player is current performing a Special Melee Attack Move. */
simulated final native function bool IsDoingSpecialMeleeAttack() const;

/** Returns TRUE if player is current performing a Special Melee Attack Move. */
simulated final native function bool IsSpecialMeleeVictim() const;

// use native versions
simulated event GetActorEyesViewPoint( out vector out_Location, out Rotator out_Rotation )
{
	out_Location = GetPawnViewLocation();
	out_Rotation = GetViewRotation();
}

/** Returns TRUE if the pawn can chain this special move after the current one finishes (or if there currently isn't a special move) */
simulated final function bool CanChainSpecialMove(ESpecialMove NextMove)
{
	return (SpecialMove == SM_None || SpecialMoves[SpecialMove].CanChainMove(NextMove) || SpecialMoves[SpecialMove].CanOverrideMoveWith(NextMove) || SpecialMoves[NextMove].CanOverrideSpecialMove(SpecialMove));
}


/************************************************************************************
 * Trigger Mashing MiniGame
 ***********************************************************************************/

/**
 * Begin Trigger Mashing Mini-game
 */
simulated function TriggerMiniGameStartNotification()
{
	// Reset button presses.
	DuelingMiniGameButtonPresses = 0;

	// Start dueling mini game
	bInTriggerMiniGame = TRUE;
}

/**
 * End Trigger Mashing Mini-game
 */
simulated function TriggerMiniGameEndNotification()
{
	// Stop dueling mini game
	// We can't mash the buttom anymore.
	bInTriggerMiniGame = FALSE;
}

/** Every time a trigger is pressed */
simulated function ReportTriggerPressMiniGame(bool bRightTriggerPressed)
{
	`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "bRightTriggerPressed:" @ bRightTriggerPressed @ "bLastTriggerPressedIsRight:" @ bLastTriggerPressedIsRight);

	// If alternating, then report the button press
	if( bLastTriggerPressedIsRight != bRightTriggerPressed )
	{
		ReportDuelingMiniGameButtonPress();
	}
	bLastTriggerPressedIsRight = bRightTriggerPressed;
}

/************************************************************************************
 * Dueling
 ***********************************************************************************/

/** Returns TRUE if Pawn is chainsaw dueling. */
simulated final function bool IsChainsawDueling()
{
	if( bInDuelingMiniGame )
	{
		return TRUE;
	}

	switch( SpecialMove )
	{
		case SM_ChainsawDuel_Leader :
		case SM_ChainsawDuel_Follower :
		case SM_ChainsawDuel_Draw :
				return TRUE;
	}

	return FALSE;
}

/**
 * Notification called from the Engage special move, to let us know that
 * We're done engaging, and we can now actually duel.
 */
simulated function DuelingMiniGameStartNotification()
{
	// Reset button presses.
	DuelingMiniGameButtonPresses = 0;

	// Start dueling mini game
	bInDuelingMiniGame = TRUE;

	if( GearAI(Controller) != None )
	{
		DuelingMiniGameButtonPresses = RandRange(AI_ChainsawDuelButtonPresses.X, AI_ChainsawDuelButtonPresses.Y);
		// difficulty scaling
		DuelingMiniGameButtonPresses *= (0.5 + 0.25 * GearAI(Controller).GetDifficultyLevel());
		`Log("AI Presses" @ DuelingMiniGameButtonPresses @ "times");
	}
}

/**
 * Called when leaving Engage special move.
 */
simulated function DuelingMiniGameEndNotification()
{
	// Stop dueling mini game
	// We can't mash the buttom anymore.
	bInDuelingMiniGame = FALSE;
}


reliable server function ReportDuelingMiniGameButtonPress()
{
	// Have server verify that we're still dueling
	if( bInDuelingMiniGame || bInTriggerMiniGame )
	{
		DuelingMiniGameButtonPresses++;

		// Force replication to happen now
		if( WorldInfo.NetMode != NM_Standalone )
		{
			bForceNetUpdate = TRUE;
		}
	}
}

/**
 * Is Pawn doing a 'dodge' move?
 * Those moves prevent pawn from being dragged into a chainsaw dueling for example,
 * we want him to settle down first before being sucked into an execution interaction.
 */
simulated final function bool IsDoingADodgeMove()
{
	return IsEvading() || IsDoingSpecialMove(SM_MidLvlJumpOver) || IsDoingSpecialMove(SM_MantleDown) ||
			IsDoingSpecialMove(SM_MantleUpLowCover) || IsDoingSpecialMove(SM_StdLvlSwatTurn) || IsDoingSpecialMove(SM_CoverSlip);
}

reliable server function ServerQueueRoadieRunMove(optional bool bForce, optional bool bSkipBoost)
{
	if (Role == ROLE_Authority)
	{
		QueueRoadieRunMove(bForce,bSkipBoost);
	}
}

/** bForce when we want to force the roadie run move. */
simulated function QueueRoadieRunMove(optional bool bForce, optional bool bSkipBoost)
{
	// notify the server as well
	if( Role < ROLE_Authority )
	{
		ServerQueueRoadieRunMove(bForce);
	}
	// if not a dedicated server then perform the move locally as well
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		DoSpecialMove(SM_RoadieRun, bForce);
	}

	if (!bSkipBoost)
	{
		RoadieRunBoostTime = WorldInfo.TimeSeconds;
	}
	if( bForce )
	{
		Velocity		= Vector(Rotation) * GroundSpeed * GetMaxSpeedModifier();
		Acceleration	= Normal(Velocity) * AccelRate;
	}
}


/*
** Ending SpecialMoves **

- EndSpecialMove, just ends the special move locally, no replication is done to keep owning client or server in sync.
	This is good when special moves are reliably going to terminate themselves. (for example timer or animation based).
- ServerEndSpecialMove, called on server (ROLE_Authority), and will replicate to owning client if necessary.
	Ensuring synchronization between server/locally owned client.
- LocalEndSpecialMove, called on locally controlled pawn, if it is a client it will be replicated to server.
*/

/**
 * Request to abort/stop current SpecialMove
 * This is not replicated to owning client. See ClientEndSpecialMove() below for this.
 */
final simulated exec function EndSpecialMove(optional ESpecialMove SpecialMoveToEnd)
{
	`LogSM("SpecialMoveToEnd:" @ SpecialMoveToEnd);

	if( IsDoingASpecialMove() )
	{
		// clear the pending move
		if( SpecialMoveToEnd != SM_None && PendingSpecialMoveStruct.SpecialMove == SpecialMoveToEnd )
		{
			PendingSpecialMoveStruct = FillSMStructFromParams(SM_None);
		}

		// if no move specified, or it matches the current move
		if( SpecialMoveToEnd == SM_None || IsDoingSpecialMove(SpecialMoveToEnd) )
		{
			// force it to end
			DoSpecialMove(SM_None, TRUE);
		}
	}
}

/**
 * Abort current special move.
 * Called on ROLE_Authority, and replicated to owning client if needed.
 */
final event ServerEndSpecialMove(optional ESpecialMove SpecialMoveToEnd)
{
	// make sure we're called on server.
	if( WorldInfo.NetMode == NM_Client )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "not called on server!!!");
		return;
	}

	`LogSM(`showvar(SpecialMoveToEnd)@`showvar(SpecialMove));

	if( SpecialMoveToEnd == SM_None )
	{
		SpecialMoveToEnd = SpecialMove;
	}

	// For replication we require a special move to be specified consistency accross network.
	if( SpecialMoveToEnd != SM_None )
	{
		// Force special move to end locally.
		EndSpecialMove(SpecialMoveToEnd);

		// If we're a player controlled Pawn on the server, replicate this to owning client.
		if( Controller != None && Controller.IsA('GearPC') && !IsLocallyControlled() )
		{
			ReplicatedClientEndSpecialMove(SpecialMoveToEnd);
		}
	}
}

/**
 * Replicated function from Server to Client, called only from ClientEndSpecialMove()
 * Should not be called directly.
 */
final simulated reliable client function ReplicatedClientEndSpecialMove(optional ESpecialMove SpecialMoveToEnd)
{
	`LogSM("SpecialMoveToEnd:" @ SpecialMoveToEnd);
	EndSpecialMove(SpecialMoveToEnd);
}

/**
 * Abort current special move.
 * Called on locally controlled pawn, and replicated to server if needed.
 */
final simulated event LocalEndSpecialMove(optional ESpecialMove SpecialMoveToEnd)
{
	// make sure we're called on a locally controlled pawn.
	if( !IsLocallyControlled() )
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "called on non locally controlled pawn!!!");
		return;
	}

	`LogSM(`showvar(SpecialMoveToEnd)@`showvar(SpecialMove));

	if( SpecialMoveToEnd == SM_None )
	{
		SpecialMoveToEnd = SpecialMove;
	}

	// For replication we require a special move to be specified consistency accross network.
	if( SpecialMoveToEnd != SM_None )
	{
		// End SpecialMove locally
		EndSpecialMove(SpecialMoveToEnd);

		// Replicate to server if needed
		if( Controller != None && Controller.IsA('GearPC') && WorldInfo.NetMode == NM_Client )
		{
			ReplicatedServerEndSpecialMove(SpecialMoveToEnd);
		}
	}
}

/**
 * Replicated function from Client to Server, called only from LocalEndSpecialMove()
 * Should not be called directly.
 */
final simulated reliable server function ReplicatedServerEndSpecialMove(optional ESpecialMove SpecialMoveToEnd)
{
	`LogSM("SpecialMoveToEnd:" @ SpecialMoveToEnd);
	EndSpecialMove(SpecialMoveToEnd);
}

/* Notification called when SpecialMove enum changes. */
simulated function SpecialMoveAssigned(ESpecialMove NewMove, ESpecialMove PrevMove)
{
	`LogSM("NewMove:" @ NewMove @ "PrevMove:" @ PrevMove);

	PreviousSpecialMove = PrevMove;

	// If we just ended SM_CoverRun, update 360 aiming flag for transition
	// This way GearPC.UpdatePlayerPosture() knows about it.
	// Otherwise it can create conflicts and trigger several mirroring transitions in a row.
	if( bCanDo360AimingInCover && PrevMove == SM_CoverRun )
	{
		UpdateAimOffset(AimOffsetPct, 0.f);
	}

	// Update status of left hand IK on special move transitions.
	UpdateBoneLeftHandIK();
}

/** Event called when A new special move has started */
simulated final function SpecialMoveStarted(ESpecialMove NewMove, ESpecialMove PrevMove, bool bForced)
{
	local GearPC	PC;

	`LogSM("NewMove:" @ NewMove @ "PrevMove:" @ PrevMove);

	if( NewMove != SM_None )
	{
		// notify controller that special move started.
		PC = GearPC(Controller);
		if( PC != None )
		{
			PC.SpecialMoveStarted(NewMove);
		}
		// forward notification to special move instance
		if( SpecialMoves[NewMove] != None )
		{
			SpecialMoves[NewMove].SpecialMoveStarted(bForced,PrevMove);
		}
`if(`notdefined(FINAL_RELEASE))
		else
		{
			`log("No class for special move:" @ NewMove @ self);
		}
`endif
	}
}


/** Event called when A new special move has stopped */
simulated final function SpecialMoveEnded(ESpecialMove PrevMove, ESpecialMove NextMove)
{
	`LogSM("PrevMove:" @ PrevMove);

	if( PrevMove != SM_None )
	{
		if( SpecialMoves[PrevMove] != None )
		{
			SpecialMoves[PrevMove].SpecialMoveEnded(PrevMove, NextMove);
		}
`if(`notdefined(FINAL_RELEASE))
		else
		{
			`LogSM("No class for special move:" @ PrevMove);
		}
`endif
		// give ai notification
		if (MyGearAI != None)
		{
			MyGearAI.NotifyEndOfSpecialMove(PrevMove);
		}
	}

	// If we're waiting for a special move to end to enter DBNO, try it now.
	if( Role == ROLE_Authority && bDelayedDBNO )
	{
		EnterDBNO(ControllerWhoPutMeDBNO, DamageTypeThatPutMeDBNO);
	}

	// A special move has ended, update status of shield deployment
	if( NextMove == SM_None && PendingSpecialMoveStruct.SpecialMove == SM_None )
	{
		CheckShieldDeployment();
	}
}

/**
 * Sends a message event to the current special move.
 * returns TRUE if message was correctly processed.
 */
simulated final function bool SpecialMoveMessageEvent(Name EventName, Object Sender)
{
	return(	SpecialMove != SM_None &&
			SpecialMoves[SpecialMove] != None &&
			SpecialMoves[SpecialMove].MessageEvent(EventName, Sender)
			);
}


/** Try to fit actor so it doesn't encroach world, before turning collision back on */
native final simulated function bool FitCollision();

/** Test that Pawn is on a ledge, and can mantle down */
native final simulated function bool CanPerformMantleDown(FLOAT MinMantleHeight, FLOAT MaxMantleHeight, rotator TestRotation);

function PrepareMoveAlong( GearAI_Cover AI, ReachSpec Spec )
{
	if( AI == None )
		return;

	if( Spec.IsA( 'MantleReachSpec' ) )
	{
		// do nothing
	}
	else
	// If not in combat
	if( !AI.IsAlert() )
	{
		//debug
		`AILog_Ext( GetFuncName()@"Reset cover -- not alert"@Spec@CoverType, 'Move', AI );

		AI.ResetCoverType();
	}
	else
	// If can't swat turn or cover slip or adjust along the reach spec
	if( (!bCanSwatTurn  || !Spec.IsA( 'SwatTurnReachSpec' )) &&
		(!bCanCoverSlip || !Spec.IsA( 'CoverSlipReachSpec')) &&
		 !Spec.IsA( 'SlotToSlotReachSpec' ) )
	{
		//debug
		`AILog_Ext( GetFuncName()@"Reset cover -- can't execute special move"@Spec@bCanSwatTurn@bCanCoverSlip@CoverType, 'Move', AI );

		AI.ResetCoverType();
	}
}

/**
 *	Decide which special action to take from Start to End, if any
 */
function bool SpecialMoveTo( NavigationPoint Start, NavigationPoint End, Actor Next )
{
	local ReachSpec CurrentPath;
`if(`notdefined(FINAL_RELEASE))
	local float Dist;
`endif



	if ( Start == None )
	{
		`AILog_Ext(self@GetFuncName()@"invalid start (no anchor?) FIXME FIXME FIXME", 'Move', MyGearAI);
		return FALSE;
	}
	if( Start == End )
	{
		//debug
		`AILog_Ext(self@GetFuncName()@Start@"not doing special move since it dest == start", 'Move', MyGearAI);

		return FALSE;
	}

	CurrentPath = Start.GetReachSpecTo( End );

	if(CurrentPath != Controller.CurrentPath)
	{
		`AILog_Ext(self @ GetFuncName() @"WARNING! CurrentPath does not agree with Controller.CurrentPath -- Start:"@Start@"End:"@End@"Next:"@Next@ "Controller.CurrentPath:" @ Controller.CurrentPath @ "Local CurrentPath:" @ CurrentPath, ,MyGearAI);
		CurrentPath = Controller.CurrentPath;
	}
	if( CurrentPath != None )
	{
		if( CurrentPath.IsA( 'SlotToSlotReachSpec' ) )
		{
			return SpecialMoveTo_SlotToSlot( Start, End );
		}
		else
		if( CurrentPath.IsA( 'MantleReachSpec' ) )
		{
			return SpecialMoveTo_Mantle( Start, End );
		}
		else
		if( CurrentPath.IsA( 'SwatTurnReachSpec' ) )
		{
			return SpecialMoveTo_SwatTurn( Start, End );
		}
		else
		if( CurrentPath.IsA( 'CoverSlipReachSpec' ) )
		{
			return SpecialMoveTo_CoverSlip( Start, End );
		}
		else
		if( CurrentPath.IsA( 'LadderReachSpec' ) )
		{
			return SpecialMoveTo_Ladder( Start, End );
		}
		else
		if( CurrentPath.IsA( 'LeapReachSpec' ) )
		{
			return SpecialMoveTo_Leap( Start, End );
		}
		else if( IsInCover() )
		{
			//debug
			`AILog_Ext("Regular path, leave cover:"@CurrentPath@CurrentPath.Start@CurrentPath.GetEnd(), 'None', MyGearAI );

			LeaveCover();
		}
	}
	else
	{
		//debug
		`AILog_Ext( self@GetFuncName()@"No CURRENT PATH from"@Start@"to"@End@Controller.CurrentPath@GetBestAnchor(self,Location,TRUE,FALSE,Dist), 'Move', MyGearAI );
	}

	return TRUE;
}

/**
 *	Setup slot to slot movement
 */
function bool SpecialMoveTo_SlotToSlot( NavigationPoint Start, NavigationPoint End )
{
	// Only adjust slot to slot if already in cover
	// Otherwise, just run along the reach spec like normal
	if( IsInCover() )
	{
		class'AICmd_Move_SlotToSlot'.static.SlotToSlot( GearAI_Cover(Controller), CoverSlotMarker(Start), CoverSlotMarker(End) );
	}
	return TRUE;
}

/**
 *	Setup mantle movement
 */
function bool SpecialMoveTo_Mantle( NavigationPoint Start, NavigationPoint End )
{
	// sanity check that we didn't get in here with a meat shield (stale path?)
	if (IsAKidnapper())
	{
		bCanMantle = false;
		bCanClimbUp = false;
	}
	// If it is not a mantle down spec OR the pawn can do a mantle down special move
	else if( MantleMarker(Start) == None  || SpecialMoveClasses[SM_MantleDown] != None )
	{
		LastmantleTime = WorldInfo.TimeSeconds;
		LastMantleLocation = Start.Location;
		// Push the command
		class'AICmd_Move_Mantle'.static.Mantle( MyGearAI, Start, End );
	}
	return TRUE;
}

/**
 *	Setup ladder movement
 */
function bool SpecialMoveTo_Ladder( NavigationPoint Start, NavigationPoint End )
{
	class'AICmd_Move_ClimbLadder'.static.ClimbLadder( MyGearAI, LadderMarker(Start), LadderMarker(End) );
	return TRUE;
}

/**
 *	Setup leap movement
 */
function bool SpecialMoveTo_Leap( NavigationPoint Start, NavigationPoint End )
{
	class'AICmd_Move_Leap'.static.Leap( MyGearAI, Start, End );
	return TRUE;
}

/**
 *	Setup swat turn movement
 */
function bool SpecialMoveTo_SwatTurn( NavigationPoint Start, NavigationPoint End )
{
	// Need to be able to move along the path even if we can't swat turn
	if( !bCanSwatTurn || !IsInCover() || IsCarryingAHeavyWeapon() )
	{
		if (GearAI_Cover(MyGearAI) != None)
		{
			GearAI_Cover(MyGearAI).ResetCoverType();
		}
		`AILog_Ext("Unable to do swat turn move" @ bCanSwatTurn @ IsInCover() @ IsCarryingAHeavyWeapon(), 'Move', MyGearAI);
		return TRUE;
	}

	class'AICmd_Move_SwatTurn'.static.SwatTurn( GearAI_Cover(Controller), CoverSlotMarker(Start), CoverSlotMarker(End) );
	return TRUE;
}

/**
 *	Setup cover slip movement
 */
function bool SpecialMoveTo_CoverSlip( NavigationPoint Start, NavigationPoint End )
{
	// cover slip command handles simulating the motion of the coverslip through normal movement
	// if we don't actually support the special move
	class'AICmd_Move_CoverSlip'.static.CoverSlip( GearAI_Cover(Controller), CoverSlotMarker(Start), End );
	return TRUE;
}

simulated function FaceRotation(rotator NewRotation, float DeltaTime)
{
	local Rotator	BlendDelta, Delta;
	local FLOAT		CoverBlendWeight;

	// Do not update Pawn's rotation depending on controller's ViewRotation if in FreeCam.
	if( bLockRotation || InFreeCam() )
	{
		return;
	}

	// check the special move alignment
	if (IsDoingASpecialMove() && SpecialMoves[SpecialMove].AlignToActor != None)
	{
		NewRotation = rotator(SpecialMoves[SpecialMove].AlignToActor.Location - Location);
		NewRotation.Pitch = 0;
		NewRotation.Roll = 0;
	}
	else
	{

		// Don't change pawn rotation during a special move
		if( SpecialMove != SM_None && SpecialMoves[SpecialMove].IsPawnRotationLocked() )
		{
			return;
		}

		if( CoverType != CT_None )
		{
			NewRotation = Normalize(rotator(-GetCoverNormal(TRUE)));
			LastCoverRotation = NewRotation;
		}
		else if( Physics == PHYS_Ladder )
		{
			NewRotation = OnLadder.Walldir;
		}
		else if( (Physics == PHYS_Walking) || (Physics == PHYS_Falling) )
		{
			NewRotation.Pitch = 0;
		}
		// don't use pitch if we're using the base platform no movement hack
		else if (Physics == PHYS_None && InterpActor_GearBasePlatform(Base) != None && InterpActor_GearBasePlatform(Base).bDisallowPawnMovement)
		{
			NewRotation.Pitch = 0;
		}
	}

	// Make sure new rotation is normalized.
	NewRotation = Normalize(NewRotation);

	// Special cover exit rotation interpolation. Matches animation blending.
	if( bRotInterpCoverTransition && BaseBlendNode != None )
	{
		CoverBlendWeight	= 1.f - (BaseBlendNode.NodeTotalWeight * BaseBlendNode.Children[1].Weight);
		Delta				= Normalize(NewRotation - LastCoverRotation);
		BlendDelta			= Delta * CoverBlendWeight;
// 		`DLog("CoverBlendWeight:" @ CoverBlendWeight @ "NewRotation:" @ NewRotation @ "Rotation:" @ Rotation @ "Delta:" @ Delta @ "BlendDelta:" @ BlendDelta);
		NewRotation			= Normalize(LastCoverRotation + BlendDelta);

		if( CoverBlendWeight >= 1.f )
		{
			bRotInterpCoverTransition = FALSE;
		}
	}
	// Interpolate pawn rotation smoothly to desired position,
	// to prevent instant snapping when going in and out of cover.
	// And transitioning between special moves locking rotation.
	else if( PawnRotationBlendTimeToGo > 0.f )
	{
		if( PawnRotationBlendTimeToGo > DeltaTime )
		{
			PawnRotationBlendTimeToGo	-= DeltaTime;
			Delta						= Normalize(NewRotation - Normalize(Rotation));
			BlendDelta					= Delta * FClamp((DeltaTime / PawnRotationBlendTimeToGo), 0.f, 1.f);
// 			`DLog("PawnRotationBlendTimeToGo:" @ PawnRotationBlendTimeToGo @ "NewRotation:" @ NewRotation @ "Rotation:" @ Rotation @ "Delta:" @ Delta @ "BlendDelta:" @ BlendDelta);
			NewRotation					= Normalize(Rotation + BlendDelta);
		}
		else
		{
			PawnRotationBlendTimeToGo	= 0.f;
		}
	}

	SetRotation(NewRotation);
}


/** Function to set Pawn rotation interpolation */
simulated final function InterpolatePawnRotation(optional FLOAT RotationInterpTime, optional bool bCoverTransition)
{
	bRotInterpCoverTransition = bCoverTransition;
	if( !bCoverTransition )
	{
		PawnRotationBlendTimeToGo = (RotationInterpTime > 0.f) ? RotationInterpTime : PawnRotationInterpolationTime;
	}
// 	`DLog("PawnRotationBlendTimeToGo:" @ PawnRotationBlendTimeToGo @ "bRotInterpCoverTransition:" @ bRotInterpCoverTransition);
// 	ScriptTrace();
}

simulated final function bool IsDoingPawnRotationInterpolation()
{
	return (PawnRotationBlendTimeToGo > 0.f || bRotInterpCoverTransition);
}

/**
 * @param bSmoothed		if true, returns a normal that has been smoothed over the
 *						length of the cover.
 */
final simulated function vector GetCoverNormal(optional bool bSmoothed)
{
	local Vector X, Y, Z;
	local Rotator InterpRot;

	if (CurrentLink != None)
	{
		// look for a set of bounding slots
		if( CurrentLink.Slots.Length > 1 )
		{
			if (bSmoothed)
			{
				// if we're at an edge and within nudge distance (CollisionRadius * 0.5f) then snap to current slot
				if((CurrentLink.IsLeftEdgeSlot(CurrentSlotIdx,TRUE) || CurrentLink.IsRightEdgeSlot(CurrentSlotIdx,TRUE)) &&
					VSizeSq2D(Location - CurrentLink.GetSlotLocation(CurrentSlotIdx)) <= (25.f * 25.f)
					)
				{
					InterpRot = CurrentLink.GetSlotRotation(CurrentSlotIdx);
				}
				else
				{
					InterpRot = RLerp(CurrentLink.GetSlotRotation(LeftSlotIdx), CurrentLink.GetSlotRotation(RightSlotIdx), CurrentSlotPct, TRUE);
				}
				X = vector(InterpRot);

			}
			else
			{
				// use the axis based on the slot locations
				Z = vect(0,0,1);
				Y = Normal(CurrentLink.GetSlotLocation(RightSlotIdx) - CurrentLink.GetSlotLocation(LeftSlotIdx));
				X = Normal(Y cross Z);
			}
		}
		else
		{
			// otherwise default to our currently assigned slot's rotation
			X = vector(CurrentLink.GetSlotRotation(0));
		}
	}

	return -X;
}


/************************************************************************************
 * Evade
 ***********************************************************************************/


/** Returns TRUE if player is doing an Evade special move. */
final simulated native function bool IsEvading();

/** Notification called when the character plays the evade land animation */
simulated function PlayEvadeAnimLandedNotify()
{
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		// play physical material based sounds here
		SoundGroup.PlayFoleySound(self, GearFoley_EvadeImpactFX);
		// Notify AI of sound
		MakeNoise( 0.75f );

		// need to check which way we are rolling
		if( SpecialMove == SM_EvadeRt )
		{
			PlayEvadeParticleEffect( Rotation + Rotator(vect(0,32768,0)) );
		}
		else if( SpecialMove == SM_EvadeLt )
		{
			PlayEvadeParticleEffect( Rotation + Rotator(vect(0,-32768,0)) );
		}
		else if( SpecialMove == SM_EvadeBwd )
		{
			PlayEvadeParticleEffect( Rotation + Rotator(vect(0,32768,0)) + Rotator(vect(0,32768,0)) );
		}
		else
		{
			PlayEvadeParticleEffect( Rotation );
		}
	}
}


/**
 * This will play the evade particle effect by tracing down and then spawning a particle there
 **/
simulated function PlayEvadeParticleEffect( Rotator SpawnRotation )
{
	local float CurrHeight;

	local Actor TraceActor;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;

	local Emitter RollDust;

	// don't even try to play nor do any traces if we are not within Relevancy Distance
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return;
	}

	// kk here we need to do a tracez0r down down into the ground baby!
	// NOTE: this will eventually be moved to c++ land
	CurrHeight = GetCollisionHeight();

	TraceStart = Location;
	TraceDest = TraceStart - ( Vect(0, 0, 1 ) * CurrHeight ) - Vect(0, 0, 16 );

	// trace down and see what we are standing on
	TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, TRUE, TraceExtent, HitInfo, TRACEFLAG_Bullet );
	//DrawDebugLine( TraceStart, TraceDest, 255, 0, 0, TRUE);

	if( TraceActor != none )
	{
		//DrawDebugCoordinateSystem( out_HitLocation, SpawnRotation, 100, TRUE );
		RollDust = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( class'GearPhysicalMaterialProperty'.static.DetermineEvadeParticleEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( HitInfo ), WorldInfo), out_HitLocation, SpawnRotation );
		RollDust.ParticleSystemComponent.ActivateSystem();
		RollDust.SetBase( self );

		if( IsHumanControlled() && IsLocallyControlled() )
		{
			GearPC(Controller).ClientSpawnCameraLensEffect( class'GearPhysicalMaterialProperty'.static.DetermineEvadeCameraLensEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( HitInfo ), WorldInfo) );
		}
	}
	// otherwise we hit nothing and are in the air
	else
	{
		//`log( " We are in the air" );
	}

}


simulated event EvadeAnimationFinished()
{
	//debug
	`AILog_Ext( GetFuncName()@IsEvading()@SpecialMove, 'None', MyGearAI );

	if( IsEvading() )
	{
		EndSpecialMove();
	}
}


/************************************************************************************
 * Run2Cover transition
 ***********************************************************************************/


/**
 * Prepare a slide into cover special move.
 * See if can find a good cover spot, and suggest best move.
 * return FALSE, if cannot be done.
 */
simulated final function bool CanPrepareRun2Cover(float CheckDist, out CovPosInfo OutFoundCovInfo, optional vector CheckDir, optional float AcquireCoverFOV, optional vector Start)
{
	local Vector	Dir;

	// Where to check for cover
	if (IsZero(Start))
	{
		Start		= Location;
	}
	if (!IsZero(CheckDir))
	{
		Dir = CheckDir;
	}
	else
	{
		Dir	= Vector(Rotation);
	}
	CheckDist	= FClamp(CheckDist, 0.f, Run2CoverMaxDist);

	if (AcquireCoverFOV == 0.f)
	{
		AcquireCoverFOV = PawnAcquireCoverFOV;
	}

	//FlushPersistentDebugLines();
	//DrawDebugLine(Start, Start+Dir*CheckDist, 0, 255, 0, TRUE);

	// Try to find cover to run to
	return FindCoverFromLocAndDir(Start, Dir, AcquireCoverFOV, CheckDist, OutFoundCovInfo);
}

/**
 * Is the pawn currently at the edge of their current cover?  Uses (scaled) collision radius for the distance check.
 *
 * @param bMirrored			if TRUE then checks the left-most edge, otherwise if FALSE checks the right-most edge
 * @param bAllowLeanAsEdge	if TRUE then allow transitions with leans (standing->midlevel) to be treated as an edge
 *
 * @return TRUE if at the edge, FALSE if not
 */
native final function bool IsAtCoverEdge(bool bMirrored, optional bool bAllowLeanAsEdge, optional float Scale = 1.75f, optional out int EdgeSlotIdx, optional int StartSlotIdx = -1, optional bool bIgnoreCurrentCoverAction);

/**
 * Returns the SlotIdx that the pawn is current closest to, -1 if no valid slots.
 *
 * @param bRequireOverlap	if TRUE then requires the pawn to be within overlap distance of the slot
 */
native final function int PickClosestCoverSlot(bool bRequireOverlap = TRUE, optional float RadiusScale = 0.5f, optional bool bIgnoreCurrentCoverAction);


final function bool IsAtLeftEdgeSlot( optional float InLimit, optional bool bMustLean )
{
	if( CoverType == CT_None )
	{
		return FALSE;
	}

	// Lean requirement
	if( bMustLean && !CurrentLink.Slots[0].bLeanLeft )
	{
		return FALSE;
	}

	// Is located on left edge slot
	if( CurrentLink.Slots.Length == 1 ||			// Single Slot
		(CurrentSlotIdx == 0 && IsOnACoverSlot()) )	// Right on edge slot
	{
		return TRUE;
	}

	// Limit test from edge, assuming InLimit is a percentage in this case
	if( InLimit > 0.f && InLimit <= 1.f)
	{
		if( LeftSlotIdx == 0 && CurrentSlotPct <= InLimit )
		{
			return TRUE;
		}
	}
	// otherwise assume it's a distance check
	else
	{
		// if no distance specified, then default to a scaled collision radius
		if (InLimit == 0.f)
		{
			InLimit = CylinderComponent.CollisionRadius * 1.5f;
		}
		//`LogExt(VSize(CurrentLink.GetSlotLocation(LeftSlotIdx) - Location)@"vs"@InLimit);
		return (LeftSlotIdx == 0 && VSize(CurrentLink.GetSlotLocation(LeftSlotIdx) - Location) <= InLimit);
	}

	// check and see if slot immediately to the left of us is disabled.
	// checking against 0.5 because CalcVelocity will let us get 1/2 way to a disabled slot
	// @fixme, 0.5 thing doesn't seem tobe working right now, when itdoes, uncomment below check
	if ( !CurrentLink.Slots[LeftSlotIdx].bEnabled /*&& P.CurrentSlotPct <= 0.5f*/ )
	{
		return TRUE;
	}

	return FALSE;
}


final function bool IsAtRightEdgeSlot(optional float InLimit, optional bool bMustLean)
{
	local int	RightEdgeSlotIndex;

	if( CoverType == CT_None )
	{
		return FALSE;
	}

	RightEdgeSlotIndex = CurrentLink.Slots.Length - 1;

	// Lean requirement
	if( bMustLean && !CurrentLink.Slots[RightSlotIdx].bLeanRight )
	{
		return FALSE;
	}

	// Is located on right edge slot
	if( CurrentLink.Slots.Length == 1 ||								// Single Slot
		(CurrentSlotIdx == RightEdgeSlotIndex && IsOnACoverSlot()) )	// Right on edge slot
	{
		return TRUE;
	}


	// Limit test from edge, assuming InLimit is a percentage in this case
	if( InLimit > 0.f && InLimit <= 1.f)
	{
		if( RightSlotIdx == RightEdgeSlotIndex && CurrentSlotPct >= (1.f - InLimit) )
		{
			return TRUE;
		}
	}
	// otherwise assume it's a distance check
	else
	{
		// if no distance specified, then default to a scaled collision radius
		if (InLimit == 0.f)
		{
			InLimit = CylinderComponent.CollisionRadius * 1.5f;
		}
		//`LogExt(VSize(CurrentLink.GetSlotLocation(RightSlotIdx) - Location)@"vs"@InLimit);
		return (RightSlotIdx == RightEdgeSlotIndex && VSize(CurrentLink.GetSlotLocation(RightSlotIdx) - Location) <= InLimit);
	}

	// check and see if slot immediately to the right of us is disabled.
	// checking against 0.5 because CalcVelocity will let us get 1/2 way to a disabled slot
	// @fixme, 0.5 thing doesn't seem tobe working right now, when itdoes, uncomment below check
	if ( !CurrentLink.Slots[RightSlotIdx].bEnabled /*&& P.CurrentSlotPct >= 0.5f*/ )
	{
		return TRUE;
	}

	return FALSE;
}

/************************************************************************************
 * Move2Idle transition
 ***********************************************************************************/

/** Return TRUE if Pawn is doing one of the move2idle transition special moves */
native final simulated function bool IsDoingMove2IdleTransition();


/**
 * Event called from C++, when Pawn wants to stop moving,
 * to see if move2idle transition can be started.
 */
simulated event ConditionalMove2IdleTransition()
{
	local float			Speed, DeltaToRun, DeltaToWalk, DotY;
	local ESpecialMove	NewMove;
	local GearPC			PC;
	local vector		X, Y, Z;

	`LogSM("");

	// No move2idle transitions in MP or when not in cover.
	if( WorldInfo.NetMode != NM_Standalone || CurrentLink == None )
	{
		return;
	}

	// no transition if about to slide into cover
	if( PendingSpecialMoveStruct.SpecialMove == SM_Run2StdCov || PendingSpecialMoveStruct.SpecialMove == SM_Run2MidCov )
	{
		return;
	}

	// See if Pawn should be forced into walking or running
	Speed		= VSize2D(Velocity);
	DeltaToRun	= Abs(Speed - GroundSpeed);
	DeltaToWalk	= Abs(Speed - GroundSpeed * WalkingPct);

	if( DeltaToRun < DeltaToWalk )
	{
		NewMove = SM_Run2Idle;
	}
	else
	{
		NewMove = SM_Walk2Idle;
	}

	if( IsInCover() )
	{
		// if we're on a slot then don't move since that'll move us off the slot
		if (IsOnACoverSlot())
		{
			return;
		}

		GetAxes(Rotation, X, Y, Z);
		DotY = Velocity dot Y;

		if ( ((DotY < 0.f) && !bIsMirrored) || ((DotY > 0.f) && bIsMirrored) )
		{
			// backing up in cover, don't to move2idle transition.
			// @fixme, someday we'll want a transition, just one suited for walking backward.
			return;
		}
	}

	// For human players, local player plays and replicates the special move.
	// For AI, it's done on server.
	PC = GearPC(Controller);
	if( PC != None )
	{
		if( PC.IsLocalPlayerController() )
		{
			PC.DoSpecialMove(NewMove);
		}
	}
	else
	{
		DoSpecialMove(NewMove);
	}
}


/**
 * Event called from C++ when transition is either finished or aborted.
 */
simulated event Move2IdleTransitionFinished()
{
	`LogSM("");

	if( IsDoingMove2IdleTransition() )
	{
		EndSpecialMove();
	}
}

simulated event Bump(Actor Other, PrimitiveComponent OtherComp, Vector HitNormal)
{
	local InterpActor Interp;

	Super.Bump(Other, OtherComp, HitNormal);

	// abort if we hit a player mid swat turn
	if( IsDoingSpecialMove(SM_StdLvlSwatTurn) )
	{
		Interp = InterpActor(Other);
		if( Interp == None || Interp.Base != Base )
		{
			BreakFromCover();
			EndSpecialMove();
		}
	}

	// Monitor when Pawn last bumped into something
	LastBumpTime = WorldInfo.TimeSeconds;
}


/************************************************************************************
 * Weapon / Firing
 ***********************************************************************************/


/**
 * Forward FlashLocation updates to weapon.
 */
simulated function FlashLocationUpdated( bool bViaReplication )
{
	if( MyGearWeapon != None )
	{
		MyGearWeapon.FlashLocationUpdated( FiringMode, FlashLocation, bViaReplication );
	}
}


/**
 * Forward FlashCount updates to weapon.
 */

simulated function FlashCountUpdated( bool bViaReplication )
{
	if( MyGearWeapon != None )
	{
		MyGearWeapon.FlashCountUpdated(FlashCount, FiringMode, bViaReplication);
	}
}


/** @see Pawn::WeaponFired */
simulated function WeaponFired( bool bViaReplication, optional vector HitLocation )
{
	local int	i;
	local bool	bIsAKidnapper;

	// increment number of consecutive shots.
	ShotCount++;

	// Keep track of when last shot was fired.
	// Tracking WeaponStoppedFiring because weapons like shotgun have a very long Fire Interval.
	// So instead of weapon firing, we track when weapon stopped firing.
	LastWeaponEndFireTime = WorldInfo.TimeSeconds + Weapon.GetFireInterval(FiringMode);

	// if blind firing then record the time for animations,
	if( CoverType != CT_None && IsBlindFiring(CoverAction) )
	{
		LastWeaponBlindFireTime = WorldInfo.TimeSeconds;
	}

	bIsAKidnapper = IsAKidnapper();

	// Trigger IK Based weapon recoil.
	if( MyGearWeapon != None && MyGearWeapon.bPlayIKRecoil )
	{
		for(i=0; i<RecoilNodes.Length; i++)
		{
			// Fix for low frame rate issue, do not overwrite shakes...
			if( RecoilNodes[i].Recoil.TimeToGo > RecoilNodes[i].Recoil.TimeDuration*0.5f )
			{
				continue;
			}

			// Kidnappers don't play on global WeaponRecoilNode. Hooked up to the IKGunBone, with controls both hands, and hostage.
			if( bIsAKidnapper && RecoilNodes[i].ControlName == 'WeapRecoilNode' )
			{
				continue;
			}

			// RightHandRecoil node is only used for kidnappers. Only affects right hand for pistols.
			if( !bIsAKidnapper && RecoilNodes[i].ControlName == 'RightHandRecoilNode' )
			{
				continue;
			}

			RecoilNodes[i].bPlayRecoil = !RecoilNodes[i].bPlayRecoil;

			// @fixme laurent, only do this at weapon change? No need to reassign it every time...
			if( RecoilNodes[i].ControlName == 'SpineRecoilNode' )
			{
				RecoilNodes[i].Recoil = MyGearWeapon.Recoil_Spine;
			}
			else
			{
				RecoilNodes[i].Recoil = MyGearWeapon.Recoil_Hand;
			}
		}
	}
}


/** @see Pawn::WeaponStoppedFiring */
simulated function WeaponStoppedFiring( bool bViaReplication )
{
	// reset number of consecutive shots fired.
	ShotCount = 0;
}


/** Play proper character animation when firing mode changes */
simulated function FiringModeUpdated( bool bViaReplication )
{
	if( Weapon != None )
	{
		Weapon.FireModeUpdated(FiringMode, bViaReplication);
	}
}


simulated native event vector GetPhysicalFireStartLoc( vector FireOffset );

/** Attach weapon to character inventory slot */
simulated function AttachWeaponToSlot(GearWeapon W)
{
	// current weapon is attached via AttachWeaponToHand()
	// Don't attach grenades on character
	if (W != None && (W != Weapon || W.bTemporaryHolster) && W.CharacterSlot != EASlot_Belt)
	{
		`LogInv("Attach" @ W @ "to slot" @ W.CharacterSlot);
		SetSlotAttachment(W.CharacterSlot, W.Class);
	}
}

/** Detach weapon from character inventory slot */
simulated function DetachWeaponFromSlot(GearWeapon W)
{
	`LogInv("Detach" @ W @ "from slot" @ W.CharacterSlot);
	SetSlotAttachment(W.CharacterSlot, None);
}

/** PRI and team for local playercontroller has been received.
Now we update attachment visibility, since we know the team of the local player
*/
simulated function NotifyLocalPlayerTeamReceived()
{
	if ( PlayerReplicationInfo == None )
		return;

	SetTimer(0.0001,FALSE,nameof(UpdateAttachmentVisibility));
}

/** Update visibility of weapon attachments based on whether this pawn's team and
 * local player's team match
 */
simulated event UpdateAttachmentVisibility()
{
	// update attachment visibility based on teams
	SetAttachmentVisibility(EASlot_Belt, AttachSlots[EASlot_Belt].InvClass);
	SetAttachmentVisibility(EASlot_Holster, AttachSlots[EASlot_Holster].InvClass);
	SetAttachmentVisibility(EASlot_LeftShoulder, AttachSlots[EASlot_LeftShoulder].InvClass);
	SetAttachmentVisibility(EASlot_RightShoulder, AttachSlots[EASlot_RightShoulder].InvClass);
}

/**
 * Assign an attachment to an inventory slot.
 * Update attachment meshes and allocate on demand.
 *
 * @param	Slot		Slot to set up.
 * @param	AttachClass	Weapon class to attach to the slot. None to clear attachment.
 */
simulated function SetSlotAttachment(EAttachSlot Slot, class<Inventory> AttachClass)
{
	local GearWeapon Weap;
	local Inventory Inv;

	`LogInv(AttachClass @ "on" @ Slot);
	if( Slot == EASlot_None )
	{
		`LogInv("Slot == EASlot_None - abort");
		return;
	}

	// For remote clients, Slots are not replicated, so we fix them up here
	if( AttachClass != None && WorldInfo.NetMode == NM_Client && !IsLocallyControlled() )
	{
		// See if we can find the weapon in the Pawn's inventory
		// if not, weapon will be created.
		Inv = RemoteClientGetInventoryByClass(AttachClass);
		if (Inv != None)
		{
			// Then set the proper slot on the weapon
			Weap = GearWeapon(Inv);
			if( Weap != None )
			{
				Weap.CharacterSlot = Slot;
			}
		}
		else if (WorldInfo.IsPlayingDemo())
		{
			// nonlocal inventory is replicated for demos, wait for it
			SetTimer( 0.1, false, nameof(RefreshRemoteAttachments) );
		}
		else
		{
			`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Couldn't set proper slot" @ Slot @ "to attachment" @ AttachClass @ "for remote pawn" @ Self);
		}
	}

	if( AttachSlots[Slot].InvClass == AttachClass )
	{
		`LogInv("AttachSlots[Slot].InvClass == AttachClass - abort");
		return;
	}

	// Update local class
	AttachSlots[Slot].InvClass = AttachClass;

	if( Role == Role_Authority )
	{
		// Set individual classes, for replication to remote clients.
		switch( Slot )
		{
			case EASlot_Belt			: AttachSlotClass_Belt			= AttachClass;	break;
			case EASlot_Holster			: AttachSlotClass_Holster		= AttachClass;	break;
			case EASlot_LeftShoulder	: AttachSlotClass_LeftShoulder	= AttachClass;	break;
			case EASlot_RightShoulder	: AttachSlotClass_RightShoulder	= AttachClass;	break;
		}

		// force replication
		bForceNetUpdate = TRUE;
	}

	SetAttachmentVisibility(Slot, AttachClass);
}

simulated function SetAttachmentVisibility(EAttachSlot Slot, class<Inventory> AttachClass)
{
	local PrimitiveComponent	PrimComponent;
	local StaticMeshComponent	StaticMeshComponent;
	local SkeletalMeshComponent	SkeletalMeshComponent;
	local PlayerController		PC;
	local MaterialInstanceConstant MIC_WeaponSkin;
	local LinearColor LC;
	local int TeamNum;
	local class<GearWeapon> WeapClass;
	local MeshComponent InvClassMesh;

	// skip if dedicated server, below is only cosmetic
	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	`LogInv(AttachClass @ "on" @ Slot);

	// Get the first local player controller
	foreach LocalPlayerControllers(class'PlayerController', PC) { break; }

	// get weapon class, if AttachClass is for a weapon
	WeapClass = class<GearWeapon>(AttachClass);

	// find the mesh component to use, which depends on what class we're dealing with
	if (WeapClass != None)
	{
		InvClassMesh = WeapClass.default.Mesh;
	}

	// Hide attachments if the class is none or if this pawn is on the wrong team
	if( (AttachClass == None)
			|| (!PC.bDemoOwner
			    && PC.PlayerReplicationInfo != None
			    && PlayerReplicationInfo != None
				&& PC.PlayerReplicationInfo.Team != PlayerReplicationInfo.Team
				&& PC.PlayerReplicationInfo.SplitscreenIndex == -1   // we show weapons when we are a splitscreen player that is local (-1 is not a ss player)
				&& PlayerReplicationInfo.SplitscreenIndex == -1   // we show weapons when we are a splitscreen player that is local (-1 is not a ss player)
				)
	  )
	{
		`LogInv("Hiding attachment");

		// turn off attachment
		if( AttachSlots[Slot].StaticMesh != None )
		{
			AttachSlots[Slot].StaticMesh.SetBlockRigidBody(FALSE);
			AttachSlots[Slot].StaticMesh.SetHidden(TRUE);
		}

		if( AttachSlots[Slot].SkeletalMesh != None )
		{
			AttachSlots[Slot].SkeletalMesh.SetHidden(TRUE);
		}
	}
	else if( InvClassMesh != None )
	{
		// update attachment
		// depending on what type of mesh the weapon is, choose the appropriate slot component
		if( InvClassMesh.IsA('SkeletalMeshComponent') )
		{
			if( AttachSlots[Slot].SkeletalMesh == None )
			{
				AttachSlots[Slot].SkeletalMesh = new(Self) class'SkeletalMeshComponent';
			}
			SkeletalMeshComponent	= AttachSlots[Slot].SkeletalMesh;
			PrimComponent			= SkeletalMeshComponent;
		}
		else
		{
			if( AttachSlots[Slot].StaticMesh == None )
			{
				AttachSlots[Slot].StaticMesh = new(Self) class'StaticMeshComponent';
			}
			StaticMeshComponent = AttachSlots[Slot].StaticMesh;
			PrimComponent		= StaticMeshComponent;

			// disable collision that is on by default on staticmeshes
			PrimComponent.SetActorCollision(FALSE, FALSE);
			PrimComponent.SetBlockRigidBody(FALSE);
		}


		// Verify that slot component is attached to Mesh
		if( !Mesh.IsComponentAttached(PrimComponent) )
		{
			if (AttachSlots[Slot].bAttachToBone)
			{
				// attach to bone
				Mesh.AttachComponent(PrimComponent, AttachSlots[Slot].SocketName);
			}
			else
			{
				// attach to socket
				Mesh.AttachComponentToSocket(PrimComponent, AttachSlots[Slot].SocketName);
			}
			PrimComponent.SetShadowParent(Mesh);
			PrimComponent.SetLightEnvironment(LightEnvironment);
			PrimComponent.MotionBlurScale = 0.0;
		}

		// set our staticmesh to be the component
		if( StaticMeshComponent != None )
		{
			StaticMeshComponent.SetStaticMesh( StaticMeshComponent(InvClassMesh).StaticMesh );
		}
		else
		{
			SkeletalMeshComponent.SetSkeletalMesh( SkeletalMeshComponent(InvClassMesh).SkeletalMesh );
		}

		`LogInv( "SetAttachmentVisibility: " $ PrimComponent @ self @ AttachClass );

		if (WeapClass != None)
		{
			//@see GearWeapon.SetMaterialBasedOnTeam
			// we always need to make a new MIC here as we will have different weapons on our backs
			MIC_WeaponSkin = new(outer) class'MaterialInstanceConstant';
			MIC_WeaponSkin.SetParent( InvClassMesh.GetMaterial(0) );
			//`log( "AttachClass: " $ InvClassMesh.GetMaterial(0) );
			//`log( "MeshComponent: " $ MeshComponent(PrimComponent).GetMaterial(0) );
			MeshComponent(PrimComponent).SetMaterial( 0, MIC_WeaponSkin );

			TeamNum = GetMPWeaponColorBasedOnClass();
			//`log( "MP TeamNum" @ TeamNum );

			if( TeamNum == 0 )
			{
				LC = WeapClass.static.GetWeaponEmisColor_COG( WorldInfo, WeapClass.default.WeaponID );
			}
			else
			{
				LC = WeapClass.static.GetWeaponEmisColor_Locust( WorldInfo, WeapClass.default.WeaponID );
			}

			//`log( AttachClass @ "SetAttachmentVisibility LC " $ LC.A @ LC.R @ LC.G @ LC.B );
			MIC_WeaponSkin.SetVectorParameterValue( 'Weap_EmisColor', LC );
		}

		// show this one
		PrimComponent.SetHidden(FALSE);

		// apply the weapon's transforms to this component
		PrimComponent.SetTranslation( InvClassMesh.Translation );
		PrimComponent.SetRotation( InvClassMesh.Rotation );
		PrimComponent.SetScale3D( InvClassMesh.Scale * InvClassMesh.Scale3D );
	}
}


/**
 * perform any required clean up on attachments
 * ShadowParent crashes when garbage collection is performed, clear references.
 */
simulated function CleanupAttachSlots()
{
	local PrimitiveComponent Primitive;
	local int Idx;

	for( Idx=0; Idx<AttachSlots.Length; Idx++ )
	{
		if( AttachSlots[Idx].SkeletalMesh != None )
		{
			Primitive = AttachSlots[Idx].SkeletalMesh;

		}
		else if( AttachSlots[Idx].StaticMesh != None )
		{
			Primitive = AttachSlots[Idx].StaticMesh;
		}

		if(Primitive != None)
		{
			Primitive.SetShadowParent(None);
			Primitive.SetLightEnvironment(None);
		}
	}
}


final simulated function bool IsSlotLeftShoulder(GearPawn.EAttachSlot Slot)
{
	return(	(!bIsMirrored && Slot == EASlot_LeftShoulder) ||
			(bIsMirrored && Slot == EASlot_RightShoulder) );
}

final simulated function bool IsSlotRightShoulder(GearPawn.EAttachSlot Slot)
{
	return(	(bIsMirrored && Slot == EASlot_LeftShoulder) ||
			(!bIsMirrored && Slot == EASlot_RightShoulder) );
}

/**
 * Player just changed weapon
 * Network: Local Player and Server
 *
 * @param	OldWeapon	Old weapon held by Pawn.
 * @param	NewWeapon	New weapon held by Pawn.
 */
simulated function PlayWeaponSwitch(Weapon OldWeapon, Weapon NewWeapon)
{
	local GearWeapon	OldWeap; //, NewWeap;

	`LogInv("OldWeapon:" @ OldWeapon @ "NewWeapon:" @ NewWeapon);

	//`log(WorldInfo.TimeSeconds @ "OldWeapon:" @ OldWeapon @ "NewWeapon:" @ NewWeapon);

	OldWeap = GearWeapon(OldWeapon);

	// detach old weapon from hand
	if( OldWeap != None )
	{
		OldWeap.DetachWeapon();
	}

	// attach old weapon to inventory slot
	AttachWeaponToSlot(OldWeap);

	// Save a reference to carried GearWeapon, so we don't cast all over the place.
	MyGearWeapon = GearWeapon(Weapon);
}

simulated function AttachWeapon()
{
	`LogInv("MyGearWeapon:" @ MyGearWeapon);

	if( MyGearWeapon != None )
	{
		// detach from attachment slots, to attach to hand!
		DetachWeaponFromSlot( MyGearWeapon );

		// attach to hand
		AttachWeaponToHand( MyGearWeapon );

		// Update the animation sets related to the current weapon
		// Adding or removing weapon specific sets.
		UpdateAnimSetList();

		// Force Pawn mesh to be updated.
		Mesh.ForceSkelUpdate();

		// Set a timer to reupdate animations next frame. Sometimes during transitions, things get confused.
		SetTimer(0.01f, FALSE, nameof(UpdateAnimations));
	}
	else
	{
		`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Weapon == None for" @ Self @ "LocallyControlled:" @ IsLocallyControlled() @ "Weapon:" @ MyGearWeapon @ "RemoteWeaponRepInfo:" @ RemoteWeaponRepInfo.WeaponClass);
	}
}

/** Kismet Action to add/remove AnimSets. */
simulated function OnAddAnimSet(SeqAct_AddAnimSet InAction)
{
	local int ExistingIndex, NewIndex;

	// Add AnimSets
	if( InAction.InputLinks[0].bHasImpulse )
	{
		for(NewIndex=0; NewIndex<InAction.AnimSets.Length; NewIndex++)
		{
			for (ExistingIndex = 0; ExistingIndex < ArrayCount(KismetAnimSets); ExistingIndex++)
			{
				if (KismetAnimSets[ExistingIndex] == None)
				{
					KismetAnimSets[ExistingIndex] = InAction.AnimSets[NewIndex];
					break;
				}
			}
			if (ExistingIndex == ArrayCount(KismetAnimSets))
			{
				`Warn("Ran out of KismetAnimSets entries for" @ InAction.AnimSets[NewIndex]);
			}
		}

		UpdateAnimSetList();
	}
	// Remove AnimSets
	else if( InAction.InputLinks[1].bHasImpulse )
	{
		for (NewIndex = InAction.AnimSets.Length - 1; NewIndex >= 0; NewIndex--)
		{
			for(ExistingIndex = ArrayCount(KismetAnimSets) - 1; ExistingIndex >= 0; ExistingIndex--)
			{
				if( KismetAnimSets[ExistingIndex] == InAction.AnimSets[NewIndex] )
				{
					KismetAnimSets[ExistingIndex] = None;
					break;
				}
			}
		}
		UpdateAnimSetList();
	}
}

/** Update list of AnimSets for this Pawn */
simulated function UpdateAnimSetList()
{
	local int i;

	// Restore default AnimSets
	RestoreAnimSetsToDefault();

	// Add weapon list
	if (MyGearWeapon != None && MyGearWeapon.CustomAnimSets.Length > 0)
	{
		AddAnimSets(MyGearWeapon.CustomAnimSets);
	}

	// Add Kismet list
	for (i = 0; i < ArrayCount(KismetAnimSets); i++)
	{
		//`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Adding AnimSet:" @ KismetAnimSets[i] @ "to" @ Self);
		if (KismetAnimSets[i] != None)
		{
			Mesh.AnimSets[Mesh.AnimSets.Length] = KismetAnimSets[i];
		}
	}

	// Once we've built our list of AnimSets, set the proper flags to them
	AnimSetsListUpdated();

	// Force AnimTree to be updated with new AnimSets array
	Mesh.bDisableWarningWhenAnimNotFound = TRUE;
	Mesh.UpdateAnimations();
	Mesh.bDisableWarningWhenAnimNotFound = FALSE;
}

simulated exec function UpdateAnimations()
{
	UpdateAnimSetList();
}

/** AnimSets list updated, post process them */
simulated function AnimSetsListUpdated();

/** Add a given list of anim sets on the top of the list (so they override the other ones */
simulated final function private AddAnimSets(const out array<AnimSet> CustomAnimSets)
{
	local int	i;

	// If there are no AnimSets to remove, and no AnimSets to add, then bail out
	if( CustomAnimSets.Length == 0 )
	{
		return;
	}

	for(i=0; i<CustomAnimSets.Length; i++)
	{
		Mesh.AnimSets[Mesh.AnimSets.Length] = CustomAnimSets[i];
	}
}


simulated event bool RestoreAnimSetsToDefault()
{
	if( MutatedClass != None )
	{
		Mesh.AnimSets = MutatedClass.default.Mesh.AnimSets;
	}
	else
	{
		Mesh.AnimSets = default.Mesh.AnimSets;
	}

	return TRUE;
}


/**
 * Set a new profile on all the AimOffset nodes.
 */
simulated final function SetAimOffsetNodesProfile(Name NewProfileName)
{
	local int i;

	for(i=0; i<AimOffsetNodes.Length; i++)
	{
		AimOffsetNodes[i].SetActiveProfileByName(NewProfileName);
	}
}


/**
 * Spawn a slave weapon for remote clients.
 */
simulated protected function UpdateRemoteWeapon()
{
	local GearWeapon Weap;

	`LogInv("Class:" @ RemoteWeaponRepInfo.WeaponClass @ "bHadAPreviousWeapon:" @ RemoteWeaponRepInfo.BYTE_bHadAPreviousWeapon);

	// If we got same weapon, do nothing.
	if( MyGearWeapon != None && !MyGearWeapon.bDeleteMe && RemoteWeaponRepInfo.WeaponClass == MyGearWeapon.class )
	{
		`LogInv("Got same weapon, do nothing");
		return;
	}

	// If we didn't have a previous weapon, then get rid of the one we have now.
	if (RemoteWeaponRepInfo.BYTE_bHadAPreviousWeapon == 0 && MyGearWeapon != None && !MyGearWeapon.bDeleteMe)
	{
		MyGearWeapon.ClientWeaponThrown();
	}

	// See if we can find the weapon in the Pawn's inventory
	if( RemoteWeaponRepInfo.WeaponClass != None )
	{
		Weap = GearWeapon(RemoteClientGetInventoryByClass(RemoteWeaponRepInfo.WeaponClass));
		if( Weap != None )
		{
			InvManager.SetCurrentWeapon(Weap);
		}
		else
		{
			if( WorldInfo.IsPlayingDemo() )
			{
				// nonlocal inventory is replicated for demos, wait for it
				SetTimer( 0.1, false, nameof(RefreshRemoteAttachments) );
			}
			else
			{
				`DLog("Couldn't switch to new weapon class!!!" @ RemoteWeaponRepInfo.WeaponClass @ "Weap == None");
			}
		}
	}
	else
	{
		if( MyGearWeapon != None )
		{
			MyGearWeapon.ClientWeaponThrown();
		}
	}
}


/**
* Spawn a slave weapon for remote clients.
*/
simulated protected function UpdateRemoteShield(class<Inventory> InvClass)
{
	local Inventory		Inv;
	local GearShield	ShieldInv;

	`LogInv("InvClass:" @ InvClass);

	if (InvClass != None)
	{
		// See if we can find the weapon in the Pawn's inventory.  Will be created
		// if it doesn't exist.
		Inv = RemoteClientGetInventoryByClass(InvClass);

		ShieldInv = GearShield(Inv);

		if (ShieldInv != None)
		{
			EquipShield(ShieldInv);
		}
		else if (WorldInfo.IsPlayingDemo())
		{
			// nonlocal inventory is replicated for demos, wait for it
			SetTimer( 0.1, false, nameof(RefreshRemoteAttachments) );
		}
	}
	else
	{
		if (EquippedShield != None)
		{
			DropShield();
		}
	}
}


/** This takes care of Spawning an Inventory Manager for remote clients if needed */
simulated final function RemoteClientSetupInvManager()
{
	// Need an inventory manager for remote Pawns...
	// but not for demo playback because it gets replicated in that case
	if (InvManager == None && !WorldInfo.IsPlayingDemo() && !IsLocallyControlled())
	{
		InvManager = Spawn(InventoryManagerClass, Self);
		if( InvManager == None )
		{
			`log("Warning! Couldn't spawn InventoryManager" @ InventoryManagerClass @ "for" @ Self @ GetHumanReadableName());
		}
		else
		{
			InvManager.RemoteRole = ROLE_None;
			InvManager.SetupFor(Self);
		}
	}
}

/**
 * Tries to find an inventory item (e.g. a weapon) in the remote's pawn inventory.
 * it if doesn't exist, it is spawned.
 */
simulated function Inventory RemoteClientGetInventoryByClass(class<Inventory> InvClass)
{
	local Inventory Inv;
	local GearWeapon Weap;

	// Make sure we have an inventory manager
	if( InvManager == None )
	{
		`LogInv("InvManager == None, calling RemoteClientSetupInvManager");
		RemoteClientSetupInvManager();
	}

	// First see if we're already carrying this weapon
	Inv = InvManager.FindInventoryType(InvClass);
	// if not, let's see if we can create it
	if( Inv == None )
	{
		if( !IsLocallyControlled() && !WorldInfo.IsPlayingDemo()  )
		{
			Inv = InvManager.CreateInventory(InvClass, TRUE);
			if (Inv != None)
			{
				// if it's a weapon, set up some extra data
				Weap = GearWeapon(Inv);
				if( Weap != None )
				{
					// Set this weapon as being a Slave.
					Weap.bSlaveWeapon	= TRUE;
					Weap.RemoteRole		= Role_None;
				}
			}
			else
			{
				`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Couldn't Spawn new inventory class!!!" @ InvClass);
			}
		}
		else
		{
			`LogInv("Not allowed to Spawn inventory!! Inv:" @ Inv @ "LocallyControlled:" @ IsLocallyControlled() @ "PlayingDemo:" @ WorldInfo.IsPlayingDemo());
		}
	}

	return Inv;
}


/**
 * Attach specified Weapon to Pawn's hand.
 * Supplied weapon Actor is usually Pawn.Weapon.
 * this function sets up the replication for remote clients,
 * and attaches the weapon components to the right socket.
 */
simulated function AttachWeaponToHand(GearWeapon Weap)
{
	// attach to right hand socket
	`LogInv("attach" @ Weap @ "to" @ Weap.GetDefaultHandSocketName(Self));
	if( Weap != none )
	{
		Weap.AttachWeaponTo(Mesh, Weap.GetDefaultHandSocketName(Self));

		if( Weap.Mesh != none )
		{
			Weap.SetMaterialBasedOnTeam( Weap.Mesh, GetTeamNum() );

			// turn off dynamic casting shadows on AI weapons
			if( MyGearAI != none )
			{
				Weap.Mesh.bCastDynamicShadow=FALSE;
			}
		}
	}
}

final simulated function Name GetLeftHandSocketName()
{
	// we have inverted sockets because of animation mirroring
	if( bIsMirrored )
	{
		return RightHandSocketName;
	}
	return LeftHandSocketName;
}


/**
 * Set Pawn mirroring.
 * FALSE = Normal, facing right in cover, cam over right shoulder.
 * TRUE = Mirrored, facing left in cover, cam over left shoulder.
 * This sets the bWantsToBeMirrored flag. This flag will force the animations to find a proper transition ASAP
 * And when that is done, bIsMirrored flag will be updated, and MirroredNotify() will be called.
 */
final simulated event SetMirroredSide(bool bDesiredMirrored)
{
	if( bWantsToBeMirrored != bDesiredMirrored )
	{
		if (Role < ROLE_Authority && (Controller == None || !Controller.IsLocalPlayerController()))
		{
			// non-owning clients just get the flag directly from the server
			return;
		}

		// Tell animations that we'd like to turn...
		bWantsToBeMirrored = bDesiredMirrored;

		if( GearPC(Controller) != None )
		{
			GearPC(Controller).CoverLog("bDesiredMirrored:" @ bDesiredMirrored, GetFuncName());
		}

		if( Role == Role_Authority )
		{
			// Force replication
			bForceNetUpdate = TRUE;
		}
	}
}


/**
 * Notification called from the animation system when player is mirrored.
 * Called when bIsMirrored changes status.
 */
simulated event OnMirroredNotify()
{
	if( Weapon != None && !Weapon.IsA('GearWeap_HeavyBase') )
	{
		AdjustWeaponDueToMirror();
	}
}

/** Weapon mirror notify. Used by heavy weapons. */
simulated function MirrorWeaponAnimNotify()
{
	AdjustWeaponDueToMirror();
}

/** Notification that a mirror transition is finished, and blending out. */
simulated event OnMirrorBlendOutNotify()
{
	//`log(WorldInfo.TimeSeconds @ GetFuncName());

	// Verify that weapon has been properly mirrored and reattached.
	// Re-attach will only be performed if necessary.
	AdjustWeaponDueToMirror();

	// Mirror transition is blending out, prevent IK Controllers from being turned back on,
	// if weapon requested some off.
	InternalUpdateBoneLeftHandIK();
}


/**
 * Update IK Controllers, locked to IK Bones, based on weapon settings.
 * Some weapons, like pistol and grenade, do not want the left hand to be IKed to the IK Bones.
 * @note: You should not call this function directly, call ConditionalUpdateBoneLeftHandIK() instead.
 */
simulated final private function InternalUpdateBoneLeftHandIK()
{
	local bool	bSet;

	// If IK Status is locked, don't do anything!
	if( bLockIKStatus )
	{
		return;
	}

	// Turn on by default.
	bSet = ShouldLeftHandIKBeOn();

	// If mirrored, disable right hand IK
	if( bIsMirrored )
	{
		if( IKBoneCtrl_RightHand != None )
		{
			IKBoneCtrl_RightHand.SetSkelControlActive(bSet);
		}
	}
	// Otherwise disable left hand IK
	else
	{
		if( IKBoneCtrl_LeftHand != None )
		{
			IKBoneCtrl_LeftHand.SetSkelControlActive(bSet);
		}
	}
}

/**
 * Tests if Left Hand IK should be ON or OFF.
 * Extend this function to add more conditions to it.
 */
simulated function bool ShouldLeftHandIKBeOn()
{
	// Disable left hand IK when switching weapons.
	if( bSwitchingWeapons && !bEquipTransition )
	{
		return FALSE;
	}

	// If weapon wants to disable Left Hand IK, then turn it off
	if( MyGearWeapon != None && MyGearWeapon.bDisableLeftHandIK )
	{
		return FALSE;
	}

	// See if Special Move requires to disable left hand IK
	if( SpecialMove != SM_None && SpecialMoves[SpecialMove].bDisableLeftHandIK )
	{
		return FALSE;
	}

	// Otherwise left hand IK should be always on.
	return TRUE;
}


/**
 * Conditional version of the above function.
 * Prevents enabling IK during a mirror transition, which would break mirroring.
 */
simulated final function UpdateBoneLeftHandIK()
{
	// Do not turn on IK on IK Bones during a mirror transition
	// When mirror transition is done, InternalUpdateBoneLeftHandIK() will be called.
	if( bDoingMirrorTransition )
	{
		return;
	}

	InternalUpdateBoneLeftHandIK();
}


/**
 * Used to reattach the weapon to the proper sockets.
 *
 * @see OnMirroredNotify
 */
simulated function AdjustWeaponDueToMirror()
{
	//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Weapon:" @ MyGearWeapon);

	if( MyGearWeapon != None )
	{
		MyGearWeapon.AdjustWeaponDueToMirror(Self);
	}
}

/**
 * Overridden to preserve data from the previous AI controller,
 * if applicable.
 */
function OnAssignController(SeqAct_AssignController inAction)
{
	local GearAI oldController, newController, chkController;
	local TeamInfo team;
	oldController = GearAI(Controller);
	if (oldController != None &&
		ClassIsChildOf(inAction.ControllerClass,class'GearAI'))
	{
		// only update if it's a new class
		if (inAction.ControllerClass != oldController.Class)
		{
			newController = GearAI(Spawn(inAction.ControllerClass));
			if (newController != None)
			{
				// copy over data
				oldController.CopyTo(newController);
				newController.PlayerReplicationInfo = PlayerReplicationInfo;
				// replace any refs
				foreach WorldInfo.AllControllers(class'GearAI', chkController)
				{
					chkController.ReplaceRefs(oldController, newController);
				}
				// possess the pawn
				DetachFromController(true);
				oldController.Destroy();
				newController.Possess(self,false);
				// and update team assignment
				team = PlayerReplicationInfo.Team;
				team.AddToTeam(newController);
			}
		}
	}
	else
	{
		Super.OnAssignController(inAction);
	}
}

/** Set pawn walking state */
function OnSetWalking( SeqAct_SetWalking Action )
{
	bScriptedWalking = Action.bWalking;
}

function OnToggleWeaponSwappable(SeqAct_ToggleWeaponSwappable Action)
{
	local GearWeapon Weap;
	local int Idx;
	for (Idx = 0; Idx < Action.WeaponTypes.Length; Idx++)
	{
		Weap = GearWeapon(InvManager.FindInventoryType(Action.WeaponTypes[Idx]));
		if (Weap != None)
		{
			Weap.bSwappable = Action.InputLinks[0].bHasImpulse;
		}
	}
}

function OnToggleSuperTroopers(SeqAct_ToggleSuperTroopers Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// Set the super damage multiplier
		SuperDamageMultiplier = Action.SuperDamageMultiplier;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		// Reset the super damage multiplier
		SuperDamageMultiplier = 1.f;
	}
}

function KickPawnOffTurret()
{
	local Turret Turr;

	Turr = Turret(DrivenVehicle);
	if (Turr != None)
	{
		if (MyGearAI != none)
		{
			Turr.UnclaimTurret(MyGearAI);
		}

		DrivenVehicle.DriverLeave(TRUE);
	}
}

simulated function OnTeleport(SeqAct_Teleport Action)
{
	local GearPC PC;

	// Clear anchor
	SetAnchor( None );

	// Kick players out of cover
	PC = GearPC( Controller );
	if ( PC != None )
	{
		PC.ClientBreakFromCover();
		PC.ClientResetCameraInterpolation();
	}
	else if (GearAI(Controller) != None)
	{
		GearAI(Controller).InvalidateCover();
	}

	// revive if DBNO
	if (IsDBNO())
	{
		DoRevival(self);
	}

	// kick pawn off of turret
	KickPawnOffTurret();

	// Clear special move
	DoSpecialMove( SM_None );

	// Do teleport
	super.OnTeleport( Action );
}

/**
 * Looks at the current slot index and sets the appropriate cover type.
 */
final function SetCurrentCoverType(optional bool bClear)
{
	//debug
	`AILog_Ext( GetFuncName()@bClear@"L/R/C"@LeftSlotIdx@RightSlotIdx@CurrentSlotIdx@CoverType, 'Cover', MyGearAI );

	if( !bClear && CurrentLink != None )
	{
		SetCoverType( FindCoverType() );
	}
	else
	{
		SetCoverType(CT_None);
	}
}

final function ECoverType FindCoverType()
{
	if (bWantsToCrouch)
	{
		return CT_MidLevel;
	}
	return CurrentLink.Slots[ClosestSlotIdx != -1 ? ClosestSlotIdx : CurrentSlotIdx].CoverType;
}

/**
 * Return cover type fo given cover position info.
 */
final simulated function ECoverType GetCoverTypeFor(CovPosInfo Cover)
{
	local ECoverType	LeftType, RightType;

	if( Cover.Link == None )
	{
		return CT_None;
	}

	if( Cover.Link.Slots.Length > 1 )
	{
		if( Cover.LtToRtPct == 0.f )
		{
			return Cover.Link.Slots[Cover.LtSlotIdx].CoverType;
		}
		else if (Cover.LtToRtPct == 1.f)
		{
			return Cover.Link.Slots[Cover.RtSlotIdx].CoverType;
		}
		else
		{
			// opt for the lower of the two
			LeftType	= Cover.Link.Slots[Cover.LtSlotIdx].CoverType;
			RightType	= Cover.Link.Slots[Cover.RtSlotIdx].CoverType;
			return (LeftType > RightType ? LeftType : RightType);
		}
	}

	return Cover.Link.Slots[0].CoverType;
}


/**
 * Return best facing direction in cover (mirrored or not).
 * Used when acquiring new cover, to find the best starting direction to face.
 */
final simulated function ECoverDirection FindBestCoverSideFor(CovPosInfo CovInfo)
{
	local CoverLink			Link;
	local Vector			CovToPawnDir;
	local bool				bOnRightSlot, bOnLeftSlot, bCanLeanLeft, bCanLeanRight;
	local ECoverType		CovType;
	local ECoverDirection	Out_CovDir360;
	local Rotator			DeltaRot;
	local vector2d			TestAimOffsetPct;
	local vector			CamLoc, LtToRtSlots;
	local rotator			CamRot;
	local float				NudgeDistance;

	// if on server, just use what client told me
	if( !IsLocallyControlled() )
	{
		if( FreshCoverDirectionTime != WorldInfo.TimeSeconds )
		{
			if( Role == ROLE_Authority )
			{
				ScriptTrace();
			}
		}
		else
		{
			return ReplicatedCoverDirection;
		}
	}

	// Find what cover type this is going to be
	CovType	= GetCoverTypeFor(CovInfo);

	// Find out delta angle between pawn's rotation he would face in cover and actual rotation.
	// This simulates the AimOffsetPct the Pawn would have in cover.
	DeltaRot			= Normalize(Rotation - Rotator(-1.f*CovInfo.Normal));
	TestAimOffsetPct.X	= NormalizeRotAxis(DeltaRot.Yaw)	/ 16384.f;
	TestAimOffsetPct.Y	= NormalizeRotAxis(DeltaRot.Pitch)	/ 16384.f;

	// See if Pawn would be entering in 360 aiming in cover
	if( Simulate360Aiming(FALSE, TestAimOffsetPct, CovType, Out_CovDir360) )
	{
		return Out_CovDir360;
	}

	// Test if doing a sideways entry in cover.
	// In that case, camera orientation dictates cover facing direction.
	if( Abs(TestAimOffsetPct.X) > 0.6f )
	{
		if( TestAimOffsetPct.X > 0.f )
		{
			return CD_Right;
		}

		return CD_Left;
	}

	//@experimental - rely on the camera orientation rather than pawn location for the edge cases where to pawn to cover would not line up with what the player is desiring
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).GetPlayerViewPoint(CamLoc,CamRot);
		CovToPawnDir = -vector(CamRot);
	}
	else
	{
		// default to the pawn location if no player is available
		CovToPawnDir = Location - CovInfo.Location;
	}
	CovToPawnDir.Z	= 0.f;
	CovToPawnDir	= Normal(CovToPawnDir);

	Link	= CovInfo.Link;
	// figre out which slot(s) we're standing on
	if (CovInfo.LtSlotIdx != CovInfo.RtSlotIdx)
	{
		NudgeDistance = CylinderComponent.CollisionRadius * 0.5f;
		LtToRtSlots = Link.GetSlotLocation(CovInfo.LtSlotIdx) - Link.GetSlotLocation(CovInfo.RtSlotIdx);

		// Where is cover from the slots?
		bOnLeftSlot			= CovInfo.LtToRtPct <= (NudgeDistance/VSize2D(LtToRtSlots));
		bOnRightSlot		= CovInfo.LtToRtPct >= (1.f - (NudgeDistance/VSize2D(LtToRtSlots)));
	}
	else
	{
		bOnLeftSlot = TRUE;
		bOnRightSlot = TRUE;
	}

	bCanLeanLeft = (bOnLeftSlot && Link.Slots[CovInfo.LtSlotIdx].bLeanLeft) || (bOnRightSlot && Link.Slots[CovInfo.RtSlotIdx].bLeanLeft);
	bCanLeanRight = (bOnRightSlot && Link.Slots[CovInfo.RtSlotIdx].bLeanRight) || (bOnLeftSlot && Link.Slots[CovInfo.LtSlotIdx].bLeanRight);

	// if we can only lean left, go for left
	if (bCanLeanLeft && !bCanLeanRight)
	{
		ReplicatedCoverDirection = CD_Left;
		return CD_Left;
	}
	else
	// same for lean right
	if (bCanLeanRight && !bCanLeanLeft)
	{
		ReplicatedCoverDirection = CD_Right;
		return CD_Right;
	}
	// otherwise we can't lean, or can lean both ways, so pick based on camera
	else
	{
		// either we're approaching a steep angle or a slight one, so pick the best direction based on the tangent
		// looking at left side
		if( CovToPawnDir dot CovInfo.Tangent > 0.f )
		{
			ReplicatedCoverDirection = CD_Left;
			return CD_Left;
		}
		// looking at right side
		else
		{
			ReplicatedCoverDirection = CD_Right;
			return CD_Right;
		}
	}
}


/**
 * Called by physics once we reach a cover slot location.
 */
simulated event ReachedCoverSlot(int SlotIdx)
{
	local GearPC			PC;
	local GearAI_Cover		AI;
	local int				OldSlotIdx;

	//`log(WorldInfo.TimeSeconds @ GetFuncName());

	OldSlotIdx	   = CurrentSlotIdx;
	CurrentSlotIdx = SlotIdx;
	ClosestSlotIdx = SlotIdx;

	// notify our controller
	PC = GearPC(Controller);
	if( PC != None )
	{
		PC.NotifyReachedCoverSlot( SlotIdx, OldSlotIdx );
	}
	else
	{
		AI = GearAI_Cover(Controller);
		if( AI != None )
		{
			//debug
			`AILog_Ext( self@"ReachedCoverSlot"@SlotIdx, 'Cover', AI );

			AI.NotifyReachedCoverSlot( SlotIdx );
		}
	}

	// update the current cover type
	SetCurrentCoverType();
}

/** Returns TRUE if Pawn is standing right on a cover slot */
final simulated function bool IsOnACoverSlot()
{
	return (RightSlotIdx == LeftSlotIdx || IsAtCoverEdge(bIsMirrored,TRUE,1.f));
}

function DumpCoverInfo(CovPosInfo CovInfo)
{
	`log(" Link:" @ CovInfo.Link @ "Location:" @ CovInfo.Location @ "Normal:" @ CovInfo.Normal);
	`log(" LtSlotIdx:" @ CovInfo.LtSlotIdx @ "RtSlotIdx:" @ CovInfo.RtSlotIdx @ "LtToRtPct:" @ CovInfo.LtToRtPct);
}

simulated function CovPosInfo CoverInfoToCovPosInfo(CoverInfo CovInfo)
{
	local CovPosInfo	TheInfo;

	TheInfo.Link		= CovInfo.Link;
	TheInfo.LtSlotIdx	= CovInfo.SlotIdx;
	TheInfo.RtSlotIdx	= CovInfo.SlotIdx;
	TheInfo.Location	= CovInfo.Link.GetSlotLocation(CovInfo.SlotIdx);
	TheInfo.LtToRtPct	= 0.f;
	TheInfo.Normal		= vector(CovInfo.Link.GetSlotRotation(CovInfo.SlotIdx)) * -1.f;

	return TheInfo;
}

/**
 * Event called when cover has been acquired.
 */
simulated function CoverAcquired(CovPosInfo CovInfo)
{
	local ESpecialMove	Move;
	local actor A;
	local array<SequenceEvent> TouchEvents;
	local int i;

	// if we are the server,
	if (Role == ROLE_Authority)
	{
		// log the event for stats tracking
		`RecordStat(STATS_LEVEL4,'EnteredCover',Controller,CovInfo.Link);
	}

	// Set Cover type
	SetCoverAction(CA_Default);

	CurrentSlotDirection = CD_Default;

	// For now translate all cover info properties to old system.
	// but i'd like to use just that one variable, and have AI use that as well.
	SetCovPosInfo(CovInfo, true);

	CurrentLink.Claim(self, CurrentSlotIdx);

	LastCoverAcquireDistanceSq = VSizeSq2D(CovInfo.Location - Location);
	AcquiredCoverInfo	= CovInfo;

	// If pawn can play run2cover slide transition, do the special move
	if( bCanDoRun2Cover )
	{
		// Suggest best move depending on cover information
		if( GetCoverTypeFor(CovInfo) == CT_MidLevel )
		{
			Move = SM_Run2MidCov;
		}
		else
		{
			Move = SM_Run2StdCov;
		}
		DoSpecialMove(Move);
	}

	// Play slide in cover animations locally, and replicate to remote clients
	bForceNetUpdate = TRUE;

	if (Role == ROLE_Authority)
	{
		// check to see if we need to kick off kismet 'geartouch' events when we enter cover
		foreach TouchingActors(class'Actor', A)
		{
			if (A.FindEventsOfClass(class'SeqEvt_GearTouch', TouchEvents))
			{
				for (i = 0; i < TouchEvents.length; i++)
				{
					SeqEvt_GearTouch(TouchEvents[i]).CheckCoverStatusTouchActivate(A,self,true,false);
				}
				// clear array for next iteration
				TouchEvents.length = 0;
			}
		}

		// for location or directional callouts
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_EnteredCover, Self, self);
	}
}

simulated function SetCovPosInfo(CovPosInfo CovInfo, bool bReplicate)
{
	SetCoverInfo(
		CovInfo.Link,
		(CovInfo.LtToRtPct < 0.5f ? CovInfo.LtSlotIdx : CovInfo.RtSlotIdx),
		CovInfo.LtSlotIdx,
		CovInfo.RtSlotIdx,
		CovInfo.LtToRtPct,
		bReplicate);
}

simulated function SetCoverInfo(CoverLink Link, int SlotIdx, int LeftIdx, int RightIdx, float SlotPct, bool bReplicate)
{
	//ScriptTrace();
	//`AILog_Ext(GetFuncname()@Link@SlotIdx@LeftIdx@RightIdx@SlotPct@bReplicate,,MyGearAI);
	// simulate the claims for clients
	if (Role < ROLE_AutonomousProxy)
	{
		if (CurrentLink != None)
		{
			CurrentLink.UnClaim(self,CurrentSlotIdx,FALSE);
		}
	}

	CurrentLink		= Link;
	CurrentSlotIdx	= SlotIdx;
	ClosestSlotIdx  = SlotIdx;
	LeftSlotIdx		= LeftIdx;
	RightSlotIdx	= RightIdx;
	CurrentSlotPct	= SlotPct;
	SetCurrentCoverType();

	if ( bReplicate && (GearPC(Controller) != None) && !GearPC(Controller).bUpdating && (Role == ROLE_AutonomousProxy) )
	{
		// replicate cover transition to server
		// @TODO remove:  make sure using bytes for replication is OK
		assert( (CurrentSlotIdx >= 0) && (LeftSlotIdx >= 0) && (RightSlotIdx >= 0) );

		GearPC(Controller).ServerCoverTransition(CurrentLink, CurrentSlotIdx, LeftSlotIdx, RightSlotIdx, CurrentSlotPct, ReplicatedCoverDirection);
	}

	// simulate the claims for clients
	if (Role < ROLE_AutonomousProxy)
	{
		if (CurrentLink != None)
		{
			CurrentLink.Claim(self,SlotIdx);
		}
	}
}

/** Transform a SlotInfo (CoverInfo) into a CoverInfo (CovPosInfo) */
native simulated function bool SetCoverInfoFromSlotInfo(out CovPosInfo OutCovInfo, CoverInfo SlotInfo);

/*
 * Pawn has left cover - clean up associated vars
 */
exec function LeaveCover()
{
	local array<SequenceEvent> TouchEvents;
	local Actor A;
	local SeqEvt_GearTouch TouchEvt;
	local int Idx;
	local GearPRI GP;

	`LogSM("");

	if (Role == ROLE_Authority)
	{
		// check to see if we need to kick off kismet 'geartouch' events when we leave cover
		foreach TouchingActors(class'Actor', A)
		{
			if (A.FindEventsOfClass(class'SeqEvt_GearTouch', TouchEvents))
			{
				for (Idx = 0; Idx < TouchEvents.Length; Idx++)
				{
					TouchEvt = SeqEvt_GearTouch(TouchEvents[Idx]);
					TouchEvt.CheckCoverStatusTouchActivate(A,self,FALSE,TRUE);
				}
				// clear array for next iteration
				TouchEvents.Length = 0;
			}
		}
		 if(CurrentLink != None)
		 {

			//@STATS
		 	GP = GearPRI(PlayerReplicationInfo);
			GP.TotalTimeInCover += WorldInfo.TimeSeconds - LastCoverTime;
			`RecordStat(STATS_LEVEL4,'LeftCover',Controller,string(WorldInfo.TimeSeconds - LastCoverTime));
		 }
	}

	//debug
	`AILog_Ext( self@"LeaveCover"@CurrentLink@CurrentSlotIdx, 'Cover', MyGearAI );

	// Reset cover transition information
	AcquiredCoverInfo.Link	= None;
	bForceNetUpdate = TRUE;

	PreviousSlotIdx	 = CurrentSlotIdx;
	CurrentSlotPct	 = -1;
	CurrentSlotIdx	 = -1;
	CurrentLink		 = None;

	// Force reset any transition cover data
	SetCurrentCoverType( TRUE );

	// Set the player into an alert mode when leaving cover.
	// Don't do that for heavy weapons, as it forces the player to walk.
	if( MyGearWeapon != None && MyGearWeapon.WeaponType != WT_Heavy )
	{
		SetWeaponAlert(5.f);
	}

	StopBloodTrail( 'SpawnABloodTrail_Wall' );
}


function PlaySlidingSound()
{
	local SoundCue SlideOverride;

	SlideOverride = class'GearPhysicalMaterialProperty'.static.DetermineSlidingSound( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( LastFootStepTraceInfo ), CharacterFootStepType, WorldInfo );

	if(SlideOverride != none)
	{
		PlaySound(SlideOverride);
		return;
	}

	`log("Failed To Find Slide Sound");

	PlaySound(SoundCue'Foley_Footsteps.FootSteps.Footsteps_Locust_DirtSlide_Cue');
}

function SoundCue GetSpecificSlidingSound(GearPhysicalMaterialProperty FootStepSounds)
{
	return none;
}


simulated function PlayLeapingSound()
{
	local SoundCue LeapingSound;

	LeapingSound = class'GearPhysicalMaterialProperty'.static.DetermineLeapingSound( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( LastFootStepTraceInfo ), CharacterFootStepType, WorldInfo );

	if (LeapingSound != none)
	{
		PlaySound(LeapingSound, true);
		//`log( "PlayLeapingSound" @ LeapingSound );
		return;
	}
	`log("Failed To Find Leaping Sound");
}

simulated function PlayLandingSound()
{
	local SoundCue LandingSound;

	if (FRand() > 0.5f)
	{
		SoundGroup.PlayEffort(self, GearEffort_LandLightEffort, true);
	}

	if (Role == ROLE_Authority)
	{
		MakeNoise(0.75f, 'PlayLandingSound');
	}


	LandingSound = class'GearPhysicalMaterialProperty'.static.DetermineLandingSound( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( LastFootStepTraceInfo ), CharacterFootStepType, WorldInfo );

	//trace down for land sounds (climb down, evade when feet hit, mantle)
	if (LandingSound != none)
	{
		PlaySound(LandingSound, true);
		//`log( "PlayLandingSound" @ LandingSound );
		return;
	}
	// leap sounds	(climb down, mantle, evade)
	// slide sounds ( sliding)
	`log("Failed To Find Landing Sound");
}


/**
 *	Event from c++ land that tells us to play a footstep
 **/
simulated event PlayFootStepSound( int FootDown )
{
	local float Loudness;
	local PlayerController PC;

	if( Role == ROLE_Authority )
	{
		// Noise loudness based on movement speed
		Loudness = (VSize(Velocity) / GroundSpeed);
		MakeNoise( Loudness*Loudness*Loudness, 'NOISETYPE_FootStep' );
	}

	// for each local player we want to see if they are relevant for hearing footsteps and seeing particle effects
	ForEach LocalPlayerControllers( class'PlayerController', PC )
	{
		// 300.0f is from the SoundCue DiststanceCrossFade/Attenuation node
		if( (PC.ViewTarget != None) && (VSize(PC.ViewTarget.Location - Location) < 3000.0f) )
		{
			//`log( (VSize(PC.ViewTarget.Location - Location) ));
			ActuallyPlayFootstepSound( FootDown );
			break; // only place this once please :)
		}
	}
}

/**
 * Handles actual playing of sound.  Separated from PlayFootstepSound so we can LOD / distance code
 * determine if we should be playing FootSteps
 */
simulated function ActuallyPlayFootstepSound( int FootDown )
{
	local Actor TraceActor;
	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;

	local SoundCue SC;

	// kk here we need to do a tracez0r down down into the ground baby!
	// NOTE: this will eventually be moved to c++ land
	//TraceStart = Location;
	//TraceDest = TraceStart - ( Vect(0, 0, 1 ) * GetCollisionHeight() ) - Vect(0, 0, 32 );

	if( FootDown == 0 )
	{
		TraceStart = Mesh.GetBoneLocation( LeftKneeBoneName, 0 );
		TraceDest = Mesh.GetBoneLocation( LeftFootBoneName, 0 );
	}
	else
	{
		TraceStart = Mesh.GetBoneLocation( RightKneeBoneName, 0 );
		TraceDest = Mesh.GetBoneLocation( RightFootBoneName, 0 );
	}

	TraceDest = TraceDest + ( ( TraceDest - TraceStart ) * 1.0f); // push the dest lower than the foot so we hit something
	//DrawDebugCoordinateSystem( TraceStart, rot(0,0,0), 3.0f, TRUE );
	//DrawDebugCoordinateSystem( Mesh.GetBoneLocation( 'b_MF_Foot_L', 0 ), rot(0,0,0), 3.0f, TRUE );

	// trace down and see what we are standing on
	TraceActor = Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, TRUE, TraceExtent, LastFootStepTraceInfo, TRACEFLAG_Bullet|TRACEFLAG_PhysicsVolumes );

	//DrawDebugLine( TraceStart, TraceDest, 255, 0, 0, TRUE);
	//`log( TraceActor @ HitInfo.PhysMaterial );

	if( TraceActor != none )
	{
		// check to see if this is a water volume and use that special WaterPhysMaterials
		if( GearWaterVolume(TraceActor) != None )
		{
			LastFootStepTraceInfo.PhysMaterial = GearWaterVolume(TraceActor).WaterPhysMaterial;
		}

		// play the actual sound now
		SC = class'GearPhysicalMaterialProperty'.static.DetermineFootStepSound( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( LastFootStepTraceInfo ), CharacterFootStepType, WorldInfo );

		PlaySound( SC, TRUE, TRUE, FALSE, TraceDest, TRUE );
		//`log( "ActuallyPlayFootstepSound" @ SC @ self );

		if( WorldInfo.NetMode != NM_DedicatedServer )
		{
			// TODO: this needs to be updated with a  Determine_______ in the GearPhysMat
			ActuallyPlayFootParticleEffect( FootDown, LastFootStepTraceInfo, out_HitLocation );
		}
	}
	// otherwise we hit nothing and are in the air
	//else
	//{
	//	`log( " We are in the air" );
	//}
}


/**
 * This will figure out which material we are standing on and then play the
 * correct footstep effect.
 * It looks up the PhysicalMaterial tree until it finds a valid effect to Spawn.
 * If it does not find one then it defaults the effect coded in to the .uc file
 * for that specific character.
 **/
final simulated function ActuallyPlayFootParticleEffect( int FootDown, TraceHitInfo HitInfo, vector HitLocation )
{
	local PhysicalMaterial ParentPhysMaterial;
	local ParticleSystem PS;
	local ParticleSystem FootStepEffect;
	local Emitter FootStepEmitter;
	local vector BoneLocation;

	// if we are in the rain then play water footsteps
	// NOTE: right now terrain does not return material for traces so we need to do this
// 	if( bShouldPlayWetFootSteps )
// 	{
// 		//PlaySound( SoundCue'Foley_Footsteps.FootSteps.Footsteps_Marcus_Water_Cue', FALSE, TRUE );
// 		return;
// 	}

	// if we do not have a PhysicalMaterial
	if( HitInfo.PhysMaterial != none )
	{
		//`log( "we have a physMaterial" );
		FootStepEffect = GetFootStepEffect( HitInfo.PhysMaterial, FootDown );
	}
	else
	{
		// check to see if there is a material
		// if there is no material then we must use the default PS_DefaultImpactEffect
		if( HitInfo.Material != none )
		{
			FootStepEffect = GetFootStepEffect( HitInfo.Material.PhysMaterial, FootDown );
		}
		else
		{
			// "Default footsteps effects" is nothing!
			return;
		}
	}


	// so here we need to look up the "hierarchy" if we have a none in the attribute we want
	// and we probably want to maybe have a code fall back to play some default
	// if the content doens't provide our hungry mouths with data

	// if we have no phys material
	if( HitInfo.PhysMaterial != none )
	{
		ParentPhysMaterial = HitInfo.PhysMaterial.Parent;
	}
	// set the parent phys material here
	else
	{
		// check to see if the material has a phys material
		if( HitInfo.Material.PhysMaterial != none )
		{
			ParentPhysMaterial = HitInfo.Material.PhysMaterial.Parent;
		}
		else
		{
			ParentPhysMaterial = none;
		}
	}


	// this will walk the tree until our parent is null or we have a footstep effect
	// at which point we will break out (which is basically an exception case)
	// but there are no exceptions in .uc land
	while( ( FootStepEffect == none )
		&& ( ParentPhysMaterial != none )
		)
	{
		// look at our parent's data
		FootStepEffect = GetFootStepEffect( ParentPhysMaterial, FootDown );
		ParentPhysMaterial = ParentPhysMaterial.Parent;
	}


	// use the material based effect
	if( FootStepEffect != none )
	{
		//`log( " Playing Effect: " $ FootStepEffect );
		PS = FootStepEffect;
	}
	// do default behavior
	else
	{
		//`log( " Playing Default effect" );
		//PS = None;
		return;
	}

	// these are probably deprecated
// 	if( FootDown == 1 )
// 	{
// 		BoneLocation = Mesh.GetBoneLocation( RightFootBoneName, 0 );
// 	}
// 	else
// 	{
// 		BoneLocation = Mesh.GetBoneLocation( LeftFootBoneName, 0 );
// 	}

	BoneLocation = HitLocation;

	// if we could not find the bone name then the BoneLocation should be vect(0,0,0)
	if( BoneLocation != vect(0,0,0) && WorldInfo.GRI != None )
	{
		//Effects_Gameplay.Player_Movement.P_Player_Footstep_Water
		//Effects_Gameplay.Player_Movement.P_Player_Wading_Water
		//PS = ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Footstep_Water';
		FootStepEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS, BoneLocation, Rotation );
		FootStepEmitter.ParticleSystemComponent.ActivateSystem();
	}
}



simulated function PlayKickSound()
{
	return;
}


/**
 * Looks to see if the Sound on the current PhysicalMaterial is valid.
 **/
final function ParticleSystem GetFootStepEffect( PhysicalMaterial PMaterial, int FootDown )
{
	local ParticleSystem Retval;

	Retval = none;

	// our Specific properties exists now we need to call our function to get out
	// the specificProperty
	if( ( none != PMaterial )
		&& ( none != PMaterial.PhysicalMaterialProperty )
		&& ( none != GearPhysicalMaterialProperty(PMaterial.PhysicalMaterialProperty) )
		)
	{
		Retval = GetSpecificFootStepEffect( GearPhysicalMaterialProperty(PMaterial.PhysicalMaterialProperty), FootDown );
	}
	else
	{
		Retval = none;
	}

	return Retval;
}


/**
 * Each object is going to override this and return the data based off the passed in
 * Object.	(e.g.  GetSpecificFootStepSound called on a marcus will look up the
 * marcus footstep sound, called on a locust will look up the locust sound)
 **/
function ParticleSystem GetSpecificFootStepEffect( GearPhysicalMaterialProperty FootStepSounds, int FootDown )
{
	local ParticleSystem Retval;

	// do possible computation or what not.
	if( GroundSpeed > ( GroundSpeed * WalkingPct ) )
	{
		Retval = FootStepSounds.FootstepEffectRun;
	}
	else
	{
		Retval = FootStepSounds.FootstepEffectWalk;
	}

	return Retval;
}


/**
 * This function allows the Pawn to override the Impact effect to be used.	Usually
 * we want to just use the physical material, but for certain mobs they have state
 * where we need to play a different effect based on that state (e.g. berserker)
 **/
simulated function bool HasImpactOverride()
{
	return FALSE;
}

/** this will return the Particle System to be used when we need to invoke an override **/
simulated function ParticleSystem GetImpactOverrideParticleSystem();


/**
 * list important Pawn variables on canvas.	 HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas	Canvas;
	local GearPC 	PC;
	local String	Text;
	local ECoverLocationDescription LocDesc;
	//local int Idx;

	super.DisplayDebug(HUD, out_YL, out_YPos);

	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255, 255, 255);

	Canvas.DrawText(" CoverType:" @ CoverType @ "CoverAction:" @ CoverAction @ "CoverDirection:" @ CoverDirection @ "CurrentSlotDirection:" @ CurrentSlotDirection @ "IsAtCoverEdge:"@IsAtCoverEdge(bIsMirrored,TRUE) @ "IsWalking:" @ bIsWalking);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" SpecialMove:" @ SpecialMove @ "AimOffset:" @ AimOffsetPct.X @ AimOffsetPct.Y @ "bDoing360Aiming:" @ bDoing360Aiming @ "Offset:"@ MeshTranslationOffset.X @ MeshTranslationOffset.Y @ MeshTranslationOffset.Z);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" bWantsToBeMirrored:" @ bWantsToBeMirrored @ "bIsMirrored:" @ bIsMirrored @ "bDoingMirrorTransition:" @ bDoingMirrorTransition @ "HeadTrackCurrentBlendSpeed: " @ HeadTrackCurrentBlendSpeed @ "HeadTrackBlendTargetWeight: " @ HeadTrackBlendTargetWeight @ "HeadTrackBlendTargetWeight : " @HeadTrackBlendWeightToTarget );
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" bCoverActionAnimTransition:" @ bDoingCoverActionAnimTransition @ "CoverActionAnim:" @ CoverActionAnim @ "LastCoverActionAnim:" @ LastCoverActionAnim);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" bPendingFire:" @ bPendingFire @ "CommitToFiring:" @ PawnCommitToFiring(0) @ "CanFireWeapon:" @ CanFireWeapon() @ "ShouldDelayFiring:" @ ShouldDelayFiring() );
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Text = " bIsTargeting:" @ bIsTargeting;
	PC = GearPC(Controller);
	if( PC != None )
	{
		Text = Text @ "WantsToTarget:" @ PC.WantsToTarget() @ "CanTarget:" @ PC.CanTarget();
	}
	Canvas.DrawText(Text);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText(" bDoingMeleeAttack:" @ bDoingMeleeAttack @ "bWantsToMelee:" @ bWantsToMelee @ "bPendingMeleeAttack:" @ bPendingMeleeAttack @ "CanEngageMelee:" @ CanEngageMelee() @ "ShouldDelayMeleeAttack:" @ ShouldDelayMeleeAttack() );
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);
	Canvas.DrawText(" LastTookDamage:" @ TimeSince(LastTookDamageTime) );
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	LocDesc = (CurrentLink != None) ? CurrentLink.GetLocationDescription(CurrentSlotIdx) : CoverDesc_None;
	Canvas.DrawText(" CoverDescription:"@LocDesc);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	if( Controller != None && AIController(Controller) != None )
	{
		Canvas.DrawText(" Force Rotation:" @ Controller.bForceDesiredRotation @ Controller.DesiredRotation );
		out_YPos += out_YL;
		Canvas.SetPos(4, out_YPos);

		DrawDebugLine(Location, Location + Vector(Controller.DesiredRotation) * 300.f, 0, 0, 255);
	}

	//if (GearPC(Controller) != None)
	//{
	//	Canvas.DrawText(" LocalEnemies:");
	//	out_YPos += out_YL;
	//	Canvas.SetPos(4, out_YPos);

	//	for (Idx=0; Idx<GearPC(Controller).LocalEnemies.Length; ++Idx)
	//	{
	//		Canvas.DrawText("  "@GearPC(Controller).LocalEnemies[Idx].Enemy@GearPC(Controller).LocalEnemies[Idx].bSeen);
	//		out_YPos += out_YL;
	//		Canvas.SetPos(4, out_YPos);
	//	}

	//	Canvas.DrawText(" BattleStatus:");
	//	out_YPos += out_YL;
	//	Canvas.SetPos(4, out_YPos);

	//	for (Idx=0; Idx<GearGame(WorldInfo.Game).BattleMonitor.COGBattleStatus.KnownEnemies.Length; ++Idx)
	//	{
	//		Canvas.DrawText("  "@GearGame(WorldInfo.Game).BattleMonitor.COGBattleStatus.KnownEnemies[Idx]);
	//		out_YPos += out_YL;
	//		Canvas.SetPos(4, out_YPos);
	//	}
	//}
}


/**
 * Event called from native code when Pawn starts crouching.
 * Called on non owned Pawns through bIsCrouched replication.
 * Network: ALL
 *
 * @param	HeightAdjust	height difference in unreal units between default collision height, and actual crouched cylinder height.
 */
simulated event StartCrouch(float HeightAdjust)
{
	super.StartCrouch(HeightAdjust);

	// offset mesh by height adjustment
	if( Mesh != None )
	{
		Mesh.SetTranslation(Mesh.Translation + Vect(0,0,1)*HeightAdjust);
	}
}

/**
 * Event called from native code when Pawn stops crouching.
 * Called on non owned Pawns through bIsCrouched replication.
 * Network: ALL
 *
 * @param	HeightAdjust	height difference in unreal units between default collision height, and actual crouched cylinder height.
 */
simulated event EndCrouch(float HeightAdjust)
{
	super.EndCrouch(HeightAdjust);

	// offset mesh by height adjustment
	if( Mesh != None )
	{
		Mesh.SetTranslation(Mesh.Translation - Vect(0,0,1)*HeightAdjust);
	}
}


simulated function ActiveReloadSuccess( bool bDidSuperSweetReload )
{
	local GearPC		GPC;
	local GearHUD_Base	WH;
	local GearWeapon	GW;

	if (Role == ROLE_Authority)
	{
		//@STATS
		GW = GearWeapon(Weapon);
		if (bDidSuperSweetReload)
		{
			GearPRI(PlayerReplicationInfo).AggWeaponReloadStat(GW.WeaponID, 3, GW.WeaponStatIndex);
			`RecordStat(STATS_LEVEL5,'ActiveReloadSuperSuccess', Controller, Weapon.Class);
		}
		else
		{
			GearPRI(PlayerReplicationInfo).AggWeaponReloadStat(GW.WeaponID, 2, GW.WeaponStatIndex);
			`RecordStat(STATS_LEVEL5,'ActiveReloadSuccess', Controller, Weapon.Class);
		}

		if( bDidSuperSweetReload )
		{
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_SucceededActiveReload, Self);
		}
	}

	// give GearPC a crack at responding to the event
	GPC = GearPC(Controller);

	if( GPC != None )
	{
		GPC.ActiveReloadSuccess( bDidSuperSweetReload );

		// give GearHUD_Base a crack at responding to the event
		WH = GearHUD_Base(GPC.myHUD);
		if ( WH != None )
		{
			WH.ActiveReloadSuccess( bDidSuperSweetReload );
		}
	}
}


simulated function ActiveReloadFail( bool bFailedActiveReload )
{
	local GearPC		GPC;
	local GearHUD_Base	WH;
	local GearGame	WG;
	local GearWeapon GW;

	if (Role == ROLE_Authority)
	{
		//@STATS
		GW = GearWeapon(Weapon);
		GearPRI(PlayerReplicationInfo).AggWeaponReloadStat(GW.WeaponID, (bFailedActiveReload?1:0), GW.WeaponStatIndex);
		`RecordStat(STATS_LEVEL5,bFailedActiveReload?'ActiveReloadFailed':'ActiveReloadSkipped',Controller,Weapon.Class);
	}

	WG = GearGame(WorldInfo.Game);
	if (WG != None)
	{
		WG.TriggerGUDEvent(GUDEvent_FailedActiveReload, Self);
	}

	// give GearHUD_Base a crack at responding to the event
	GPC = GearPC(Controller);
	if( GPC != None )
	{
		WH = GearHUD_Base(GPC.myHUD);
		if ( WH != None )
		{
			WH.ActiveReloadFail( bFailedActiveReload );
		}
	}
}


/**
 * Respond to DoorPush kismet action.  Note that the DoorPush
 * action is actually used as the more general "interact" action,
 * in that it can spawn a door push, door kick, button push, whatever.
 */
simulated function OnDoorPush(SeqAct_DoorPush inAction)
{
	local ESpecialMove SM;

	if( inAction.InputLinks[0].bHasImpulse )
	{
		if (inAction.SpecialMove == None ||
			SpecialMoveClasses.Find(inAction.SpecialMove) != INDEX_NONE)
		{
			if (inAction.SpecialMove == None)
			{
				SM = SM_None;
			}
			else
			{
				SM = ESpecialMove(SpecialMoveClasses.Find(inAction.SpecialMove));
			}
			if (MyGearAI != None)
			{
				MyGearAI.HandleScriptedSpecialMove(SM,Actor(SeqVar_Object(inAction.VariableLinks[1].LinkedVariables[0]).GetObjectValue()));
			}
			else if (inAction.SpecialMove != None)
			{
				if( SpecialMove != SM_None )
				{
					EndSpecialMove();
				}

				DoSpecialMove(SM);

				// something to align to?
				if (inAction.VariableLinks[1].LinkedVariables.Length > 0)
				{
					Controller.SetDestinationPosition( Actor(SeqVar_Object(inAction.VariableLinks[1].LinkedVariables[0]).GetObjectValue()).Location, TRUE );
					Controller.bPreciseDestination	= TRUE;
				}
			}
		}
	}
}

/**
 * Kismet hook to force entering a vehicle
 */
simulated function OnGearEnterVehicle( SeqAct_GearEnterVehicle inAction )
{
	local bool	 bEntered;
	local GearAI AI;

	AI = MyGearAI;

	if (Vehicle(inAction.TargetVehicle) != None)
	{
		bEntered = Vehicle(inAction.TargetVehicle).TryToDrive(self);
		if (!bEntered)
		{
			`Warn("Failed to enter vehicle" @ inAction.TargetVehicle @ " (self.DrivenVehicle:" @ DrivenVehicle $ ")");
		}
	}

	// Handle AI entering/exiting a vehicle
	if( AI != None )
	{
		if( bEntered )
		{
			// Dom getting in ride reaver, give him a simple command so he's not trying to path anywhere
			AI.DefaultCommand = class'AICmd_Base_DriveVehicle';
		}
		else
		{
			AI.DefaultCommand = MyGearAI.default.DefaultCommand;
		}
	}
}

function OnSetMaterial(SeqAct_SetMaterial Action)
{
	local MaterialInstanceConstant MIC;

	Super.OnSetMaterial(Action);

	if (Action.MaterialIndex == 0)
	{
		ReplicatedMaterial = Action.NewMaterial;
		bForceNetUpdate = true;

		//@hack: really should have had a separate Kismet action for the blood control, but it's too late now...
		MIC = MaterialInstanceConstant(ReplicatedMaterial);
		if (MIC != None && MIC.ScalarParameterValues.Find('ParameterName', 'BloodOpacity') != INDEX_NONE)
		{
			SetTimer(0.1, true, nameof(UpdateReplicatedBloodOpacity));
		}
		else
		{
			ClearTimer(nameof(UpdateReplicatedBloodOpacity));
		}
	}
}

final function UpdateReplicatedBloodOpacity()
{
	local MaterialInstanceConstant MIC;

	//@hack: really should have had a separate Kismet action for the blood control, but it's too late now...
	MIC = MaterialInstanceConstant(Mesh.GetMaterial(0));
	if (MIC != None)
	{
		 MIC.GetScalarParameterValue('BloodOpacity', BloodOpacity);
	}
	else
	{
		BloodOpacity = 0.0;
		ClearTimer(nameof(UpdateReplicatedBloodOpacity));
	}
}

/**
 * Kismet hook to play custom animations
 */
simulated function OnGearPlayerAnim(SeqAct_GearPlayerAnim inAction)
{
	local GearPC PC;
	local bool bPlay;

	bPlay = InAction.InputLinks[0].bHasImpulse;

	ProcessGearPlayerAnim(InAction, bPlay);

	// if we're the server, replicate it out to non-local players
	if (WorldInfo.NetMode != NM_Client)
	{
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.ClientPlayGearPlayerAnim(self, InAction, bPlay);
		}
	}
}

/** performs the actual work for GearPlayerAnim Kismet action, separated so it can be shared with clientside code */
simulated function ProcessGearPlayerAnim(SeqAct_GearPlayerAnim InAction, bool bPlay)
{
	local float AnimDuration;

	// we need to force special moves to end as any animations it may be relying on will be disrupted
	if (Role == ROLE_Authority)
	{
		ServerEndSpecialMove();
	}
	else
	{
		EndSpecialMove();
	}

	// Setup body stance
	KismetBodyStance.AnimName.Length = 0;
	if( inAction.bPlayUpperBodyOnly )
	{
		KismetBodyStance.AnimName[BS_Std_Up] = inAction.AnimName;
	}
	else
	{
		KismetBodyStance.AnimName[BS_FullBody] = inAction.AnimName;
	}

	if (bPlay)
	{
		// Play animation
		if( InAction.PlayMode == AnimPM_Rate )
		{
			AnimDuration = BS_Play(KismetBodyStance, inAction.Rate, inAction.BlendInTime, inAction.BlendOutTime, inAction.bLooping, inAction.bOverride);
		}
		else
		{
			BS_PlayByDuration(KismetBodyStance, inAction.Duration, inAction.BlendInTime, inAction.BlendOutTime, inAction.bLooping, inAction.bOverride);
			AnimDuration = inAction.Duration;
		}

		// Support for playing animation at an arbitrary start position.
		if( inAction.StartTime > 0.f )
		{
			BS_SetPosition(KismetBodyStance, inAction.StartTime);
		}

		// If animation uses Root Motion, set it up on character.
		if( InAction.bUseRootMotion )
		{
			BS_SetRootBoneAxisOptions(KismetBodyStance, RBA_Translate, RBA_Translate, RBA_Translate);
			Mesh.RootMotionMode = RMM_Accel;

			// Set timer to turn off body stance when animation starts
			SetTimer( AnimDuration - inAction.BlendOutTime, FALSE, nameof(TurnOffKismetRootMotion) );
		}
	}
	else
	{
		// Stop body stance
		BS_Stop(KismetBodyStance, inAction.BlendOutTime);

		// If animation uses Root Motion, set it up on character.
		if( InAction.bUseRootMotion )
		{
			ClearTimer('TurnOffKismetRootMotion');
			Mesh.RootMotionMode = Mesh.default.RootMotionMode;
			BS_SetRootBoneAxisOptions(KismetBodyStance, RBA_Discard, RBA_Discard, RBA_Discard);
		}
	}
}

/** Turn off root motion set by Kismet action */
simulated function TurnOffKismetRootMotion()
{
	Mesh.RootMotionMode = Mesh.default.RootMotionMode;
	BS_SetRootBoneAxisOptions(KismetBodyStance, RBA_Discard, RBA_Discard, RBA_Discard);
}

/**
 * Duration is how long the animation will play, including transition to the animation (but not from)
 * Pass a Duration <= 0 to play indefinitely.
*/
simulated exec function PlayHeadCoverAnim(float DurationSec, optional bool bDisableRoadieRun)
{
	local bool bPlayedAnim;

	if ( (!bCoveringHead) || (Role < ROLE_Authority) )
	{
		if ( !IsDoingASpecialMove() || (bDisableRoadieRun && IsDoingSpecialMove(SM_RoadieRun)) )
		{
			BS_Play(BS_HeadCoverLoop, 1.f, 0.2f,, TRUE, TRUE);
			bPlayedAnim = TRUE;

			if (Role == ROLE_Authority)
			{
				bCoveringHead = TRUE;
				bCannotRoadieRun = bDisableRoadieRun;

				if (bCannotRoadieRun && IsDoingSpecialMove(SM_RoadieRun))
				{
					EndSpecialMove();
				}
			}
		}
	}

	if ( (Role == ROLE_Authority) && (DurationSec >= 0.f) && bPlayedAnim )
	{
		SetTimer( DurationSec, FALSE, nameof(StopPlayingHeadCoverAnim) );
	}
}

/** Called on a timer to end the headcover anim */
simulated private function StopPlayingHeadCoverAnim()
{
	if ( (bCoveringHead) || (Role < ROLE_Authority) )
	{
		BS_Stop(BS_HeadCoverLoop, 0.2f);
		bCoveringHead = FALSE;
		bCannotRoadieRun = FALSE;
	}
}

/**
 * Turns of/off the "using commlink" mode, where the pawn
 * is walking and talking on his communicator (can have different anim,
 * different abilities, etc).  bUseCommLink is only used when bConv is true.
 */
simulated event SetConversing(bool bConv, optional bool bUseCommLink)
{
	if( bConv && !bIsConversing )
	{
		if( bUseCommLink )
		{
			SetUsingCommLink(TRUE);
		}
	}
	else if( !bConv && bIsConversing )
	{
		if( bUsingCommLink || bWantToUseCommLink )
		{
			SetUsingCommLink(FALSE);
		}

		bIsConversing = FALSE;
	}

	bWantToConverse = bConv;
	if (!bWantToConverse)
	{
		bWantToUseCommLink = FALSE;
	}

	// turn off roadie run
	if ( bConv && IsDoingSpecialMove(SM_RoadieRun) )
	{
		EndSpecialMove(SM_RoadieRun);
	}
}

exec function ToggleCommLink()
{
	SetConversing(!bIsConversing, !bUsingCommLink);
}

/**
 * Pass TRUE to make pawn use the commlink.	 Pass FALSE to turn it off.
 */
simulated function SetUsingCommLink(bool bUsing)
{
	// Want to Stop using CommLink?
	if( !bUsing && bUsingCommLink )
	{
		if( IsDoingSpecialMove(SM_UsingCommLink) )
		{
			if (Role == ROLE_Authority)
			{
				ServerEndSpecialMove();
			}
			else
			{
				EndSpecialMove();
			}
			// Ending the special move will clear bUsingCommLink flag
		}
		else
		{
			bUsingCommLink = FALSE;
		}
	}

	bWantToUseCommLink = bUsing;

	// nuke any meatshields at this point to prevent edge cases of the player losing functionality
	if (bWantToUseCommLink && GearPC(Controller) != None)
	{
		GearPC(Controller).ExecuteMeatshields();
	}
}


/************************************************************************************
 * Weapon Holstering Interface
 * Controls Holstering/UnHolstering of weapon.
 * Note: This is to holster weapon temporarily, this is not the weapon switching code.
 ***********************************************************************************/

/**
 * Tells the Pawn to holster his weapon
 * returns TRUE is the weapon could be holstered
 */
simulated final function bool HolsterWeapon()
{
	// If we have a weapon in hand and weapon is not already holstered
	if( MyGearWeapon != None && !MyGearWeapon.bTemporaryHolster && MyGearWeapon.WeaponType != WT_Heavy)
	{
		MyGearWeapon.HolsterWeaponTemporarily(TRUE);
		return TRUE;
	}

	return FALSE;
}


/**
 * Tells Pawn to re-equip his holstered weapon.
 */
simulated final function UnHolsterWeapon()
{
	if( MyGearWeapon != None && MyGearWeapon.bTemporaryHolster )
	{
		MyGearWeapon.HolsterWeaponTemporarily(FALSE);
	}
}


/** Returns TRUE is weapon is currently holstered */
simulated final function bool IsWeaponHolstered()
{
	return (MyGearWeapon != None && MyGearWeapon.bTemporaryHolster);
}


/** Stops Holster related animations. */
simulated final function StopHolsterAnims()
{
	if( MyGearWeapon != None )
	{
		MyGearWeapon.StopHolsterAnims();
	}
}


/** gets called at point where weapon should attach/detach */
simulated function SetWeaponTemporarilyHolstered(bool bHolstered)
{
	if( MyGearWeapon != None )
	{
		if( bHolstered )
		{
			// Detah weapon from hand
			MyGearWeapon.DetachWeapon();

			// attach weapon to inventory slot
			AttachWeaponToSlot(MyGearWeapon);

			if( IsDoingMeleeHoldSpecialMove() )
			{
				EndSpecialMove();
			}

			// Notify current Special Move that weapon has been holstered.
			if( SpecialMove != SM_None )
			{
				SpecialMoves[SpecialMove].WeaponTemporarilyHolstered();
			}

			// Holstered weapon, ready to grab troika handles
			if( DrivenVehicle != None && DrivenVehicle.IsA('Turret_TroikaCabal') )
			{
				StartManningTurret();
			}
		}
		else
		{
			AttachWeapon();
		}
	}
}





/************************************************************************************
 * Animation
 ***********************************************************************************/

/**
 * Play a body stance animation.
 * This will play an animation on slots defined in the Pawn's AnimTree.
 * See definition of EBodyStance in GearPawn.uc
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Rate			Rate animation should be played at.
 * @param	BlendInTime		Blend in time when animation is played.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at (Anim length - BlendOutTime) seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop by BS_Stop)
 * @param	bOverride		Play same animation over again from begining only if bOverride is set to TRUE.
 * @param	GroupName		Set if all the nodes playing an animation should be part of a group.
 *							In that case they would be synchronized, and only the most relevant animation would trigger notifies.
 *
 * @return	PlayBack length of animation assuming play rate remains constant.
 */
final native simulated function float BS_Play
(
				BodyStance	Stance,
				float		Rate,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride=TRUE,
	optional	Name		GroupName
);


/**
 * Play a body stance animation.
 * This will play an animation on slots defined in the Pawn's AnimTree.
 * See definition of EBodyStance in GearPawn.uc
 *
 * @param	Stance			BodyStance animation to play.
 * @param	Duration		Duration in seconds the animation should play for.
 * @param	BlendInTime		Blend in time when animation is played.
 * @param	BlendOutTime	Time before animation ends (in seconds) to blend out.
 *							-1.f means no blend out.
 *							0.f = instant switch, no blend.
 *							otherwise it's starting to blend out at (Anim length - BlendOutTime) seconds.
 * @param	bLooping		Should the anim loop? (and play forever until told to stop by BS_Stop)
 * @param	bOverride		Play same animation over again from begining only if bOverride is set to TRUE.
 * @param	GroupName		Set if all the nodes playing an animation should be part of a group.
 *							In that case they would be synchronized, and only the most relevant animation would trigger notifies.
 *
 * @return	PlayBack length of animation assuming play rate remains constant.
 */
final native simulated function BS_PlayByDuration
(
				BodyStance	Stance,
				float		Duration,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride=TRUE,
	optional	Name		GroupName
);

/**
 * Returns TRUE if given BodyStance is being played currently.
 * Note that calling this from the AnimEnd notification is not reliable as the animation has technically stopped playing.
 */
final native simulated function bool BS_IsPlaying(BodyStance Stance);

/** Returns TRUE if animation has any weight in the tree */
final native simulated function bool BS_HasAnyWeight(BodyStance Stance);

/**
 * Returns TRUE if given AnimNodeSequence SeqNode belongs to the given BodyStance
 * ie is current triggered in the AnimTree, but not necessarily playing anymore. The animation could be stopped or blending/blended out.
 * This is more reliable when testing if a BodyStance is done playing in an AnimEnd notification.
 */
final native simulated function bool BS_SeqNodeBelongsTo(AnimNodeSequence SeqNode, BodyStance Stance);

/**
 * Stop playing a body stance.
 * This will only stop the anim nodes playing the actual stance.
 */
final native simulated function BS_Stop(BodyStance Stance, optional float BlendOutTime);

/** Set bPlaying flag on animations. */
final native simulated function BS_SetPlayingFlag(BodyStance Stance, bool bNewPlaying);

/**
 * Stop playing all body stances.
 */
final native simulated function BS_StopAll(float BlendOutTime);

/**
 * Overrides a currently playing body stance with a new one.
 * It basically just switches animations.
 */
final native simulated function BS_Override(BodyStance Stance);


/**
 * Changes the animation position of a body stance.
 */
final native simulated function BS_SetPosition(BodyStance Stance, FLOAT Position);

/**
 * Returns in seconds the time left until the animation is done playing.
 * This is assuming the play rate is not going to change.
 */
final native simulated function float BS_GetTimeLeft(BodyStance Stance);

/**
 * Returns the Play Rate of a currently playing body stance.
 * if None is found, 1.f will be returned.
 */
final native simulated function float BS_GetPlayRate(BodyStance Stance);

/** Set Play rate of the currently playing body stance. */
final native simulated function BS_SetPlayRate(BodyStance Stance, float NewRate);

/** Scale Play rate of a Body Stance */
final native simulated function BS_ScalePlayRate(BodyStance Stance, FLOAT RateScale);

/**
 * Forces an updated of the child weights.
 * Normally this is done in the tick function, but sometimes it is useful to enforce it.
 * For example when this node has already been ticked, and we want to force a new animation to play during the AnimEnd() event.
 */
final simulated function BS_AccelerateBlend(BodyStance Stance, float BlendAmount)
{
	local int i;

	for(i=0; i<Stance.AnimName.Length; i++)
	{
		if( Stance.AnimName[i] != '' && i < BodyStanceNodes.Length && BodyStanceNodes[i] != None )
		{
			BodyStanceNodes[i].AccelerateBlend(BlendAmount);
		}
	}
}

final simulated function BS_SetbZeroRootTranslation(BodyStance Stance, bool InbZeroRootTranslation)
{
	local int i;

	for(i=0; i<Stance.AnimName.Length; i++)
	{
		if( Stance.AnimName[i] != '' && i < BodyStanceNodes.Length && BodyStanceNodes[i] != None )
		{
			BodyStanceNodes[i].GetCustomAnimNodeSeq().bZeroRootTranslation = InbZeroRootTranslation;
		}
	}
}

/** Set body stances animation root bone options. */
final native simulated function BS_SetRootBoneAxisOptions
(
				BodyStance		Stance,
	optional	ERootBoneAxis	AxisX=RBA_Default,
	optional	ERootBoneAxis	AxisY=RBA_Default,
	optional	ERootBoneAxis	AxisZ=RBA_Default
);


/**
 * Mirror a body stance animation.
 * Used by mirror transitions.
 */
final native simulated function BS_SetMirrorOptions(BodyStance Stance, bool bTransitionToMirrored, bool bBeginTransition, bool bMirrorAnimation);

/** Request body stance animation to trigger OnAnimEnd event when done playing animation. */
final native simulated function BS_SetAnimEndNotify(BodyStance Stance, bool bNewStatus);

/**
 * @see bEarlyAnimEndNotify for more details.
 * By default bEarlyAnimEndNotify is TRUE, meaning AnimEnd notifications will be triggered when the animation starts blending out.
 * Which is not when the animation has actually ended, but in most cases it improves transitions, blends and responsiveness.
 * BS_Play() and BS_PlayByDuration() automatically reset that option to TRUE, so you don't need to reset it when setting it to FALSE.
 */
final native simulated function BS_SetEarlyAnimEndNotify(BodyStance Stance, bool bNewEarlyAnimEndNotify);

/** Event called when body stance animation finished playing */
simulated function BS_AnimEndNotify(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	// Forward notification to Special Move if doing any.
	if( SpecialMove != SM_None )
	{
		`LogSM("SpecialMove ==" @ SpecialMove @ "calling BS_AnimEndNotify()");
		SpecialMoves[SpecialMove].BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
	}
}


simulated private function CameraBoneAnimEndNotification(AnimNodeSequence SeqNode)
{
	local GearPC GPC;

	GPC = GearPC(Controller);
	if (GPC != None)
	{
		GPC.CameraBoneAnimEndNotification();
	}
}


/** Plays a specific camera-bone animation. */
final simulated function PlayCustomCameraBoneAnim
(
				Name		AnimName,
				float		Rate,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride,
	optional	bool		bSingleRayPenetrationOnly,
	optional	bool		bApplyFullMotion
)
{
	if( (AnimName != '') && (CustomCameraAnimNode != None) )
	{
		CustomCameraAnimNode.PlayCustomAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride);
		CustomCameraAnimProperties.bApplyFullMotion = bApplyFullMotion;
		CustomCameraAnimProperties.bSingleRayPenetrationOnly = bSingleRayPenetrationOnly;
		CustomCameraAnimNode.SetActorAnimEndNotification(TRUE);
	}
}

final simulated function StopCustomCameraBoneAnim(optional float BlendOutTime)
{
	if( CustomCameraAnimNode != None )
	{
		CustomCameraAnimNode.StopCustomAnim(BlendOutTime);
	}
}


/**
 * Returns true if any custom camera anim is playing, false otherwise.
 * SpecificAnimName: if specified, only checks to see if this particular anim is playing.
 */
final simulated event bool IsPlayingCustomCameraAnim(optional Name SpecificAnimName)
{
	local AnimNodeSequence SeqNode;

	if ( (CustomCameraAnimNode != None) && CustomCameraAnimNode.bIsPlayingCustomAnim )
	{
		if (SpecificAnimName != '')
		{
			SeqNode = CustomCameraAnimNode.GetCustomAnimNodeSeq();
			return (SeqNode.AnimSeqName == SpecificAnimName) ? TRUE : FALSE;
		}
		else
		{
			return TRUE;
		}
	}

	return FALSE;
}

/** Animation stopped playing notify */
simulated event OnAnimEnd(AnimNodeSequence SeqNode, float PlayedTime, float ExcessTime)
{
	local int	i;

	Super.OnAnimEnd(SeqNode, PlayedTime, ExcessTime);

	// See if the node that called is part of the BodyStanceNodes...
	for( i=0; i<BodyStanceNodes.Length; i++ )
	{
		if( BodyStanceNodes[i] != None )
		{
			if( SeqNode == BodyStanceNodes[i].GetCustomAnimNodeSeq() )
			{
				BS_AnimEndNotify(SeqNode, PlayedTime, ExcessTime);
				return;
			}
		}
	}

	// if not, see if it's the camera
	if( CustomCameraAnimNode != None && (SeqNode == CustomCameraAnimNode.GetCustomAnimNodeSeq()) )
	{
		CameraBoneAnimEndNotification(SeqNode);
	}
}


/** Returns TRUE is Pawn is reloading his weapon */
simulated native final function bool IsReloadingWeapon();

/** Can Pawn reload his weapon? */
final simulated function bool CanReloadWeapon()
{
	// Check if special moves can prevent weapon reloads
	if( IsDoingASpecialMove() && SpecialMoves[SpecialMove].bShouldAbortWeaponReload )
	{
		return FALSE;
	}

	if( Physics == PHYS_RigidBody )
	{
		return FALSE;
	}

	return TRUE;
}

/** Determines whether or not the pawn can be cover their head (e.g. SM_CoverHead) **/
final function simulated bool CanCoverHead()
{
	if( !IsInCover() && ( Health > 0 )	 && // we need this here as you can shot and be down but not out and end up doing this then going down
		!IsDoingSpecialMove(SM_DBNO) && // only check SM_DBNO instead of IsDBNO() since we may need to chain a new move after auto-recovering (cringe for ex)
		!IsDoingSpecialMove(SM_MidLvlJumpOver) &&
		!IsDoingSpecialMove(SM_StdLvlSwatTurn) &&
		!IsDoingSpecialMove(SM_MantleUpLowCover) &&
		!IsDoingSpecialMove(SM_MantleDown) &&
		!IsDoingSpecialMove(SM_PushOutOfCover) &&
		!IsDoingSpecialMove(SM_MantleDown) &&
		!IsDoingSpecialMove(SM_StumbleGoDown) &&
		!IsDoingSpecialMove(SM_StumbleGetUp) &&
		!IsDoingSpecialMove(SM_CoverHead) &&
		!IsDoingDeathAnimSpecialMove() &&
		!IsDoingStumbleFromMelee() &&
		!IsDoingSpecialMove(SM_ChainSawHold) &&
		!IsDoingSpecialMove(SM_ChainSawAttack) &&
		!IsDoingSpecialMove(SM_ChainSawVictim)
		)
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Cause this pawn to do a cringing action.
 * @param Duration	Length of time from start of animation to start of blend out.
 *					This encompasses blend in and loop.
 */
event Cringe(optional float Duration)
{
	if( CanCoverHead() && CanDoSpecialMove(SM_CoverHead) )
	{
		if (IsCarryingShield())
		{
			DropShield();
		}

		if (IsDBNO())
		{
			// recover immediately from ragdoll
			DoRevival(self,FALSE);
		}
		ServerDoSpecialMove(SM_CoverHead, TRUE);

		// Custom Duration set on server. Will call ServerEndSpecialMove() so clients can end properly.
		if (Duration > 0.f)
		{
			GSM_CoverHead(SpecialMoves[SM_CoverHead]).SetCustomDuration(Duration);
		}
	}
`if(`notdefined(FINAL_RELEASE))
	else
	{
		// leave this in as this caused a blocker in sinkhole rooftop collapse, so this might help track down a similar problem
		`warn(`location @ "can't do coverhead?@?" @ `showvar(CanCoverHead()) @ `showvar(CanDoSpecialMove(SM_CoverHead)) @`showvar(SpecialMove)@`showvar(Health)@`showvar(IsInCover()));
	}
`endif
}

event StopCringe()
{
	if ( IsDoingSpecialMove(SM_CoverHead) )
	{
		GSM_CoverHead(SpecialMoves[SM_CoverHead]).StopCoveringHead();
	}
}

/**
 * Returns true if this pawn is in an "aiming pose".  As in, is in a state
 * such that the weapon is up and ready.
 */
final simulated event bool IsInAimingPose()
{
	if( !IsReloadingWeapon() )
	{
		// targeting true
		// certain cover actions true
		// forcedaimtime true
		if ( (WorldInfo.TimeSeconds < ForcedAimTime) ||
				((WorldInfo.TimeSeconds - LastWeaponEndFireTime) < AimTimeAfterFiring) ||
				IsBlindFiring(CoverAction) ||
				IsLeaning() ||
				bIsTargeting
				// recently changed weapons?  make concept of readytime instead of using LastWeaponEndFireTime directly?
				)
		{
			return TRUE;
		}
	}

	return FALSE;
}

unreliable server function ServerSetWeaponAlert(byte Duration)
{
	SetWeaponAlert(float(Duration));
}

final simulated event SetWeaponAlert(float Duration)
{
	local Byte	ByteAimTime;

	ForcedAimTime = WorldInfo.TimeSeconds + Duration;

	if ( (Role == Role_Authority) && (WorldInfo.NetMode != NM_Standalone) )
	{
		// Compress for replication. Doesn't really matter if it's not super accurate.
		ByteAimTime = Byte(Duration);

		// Ensure that ReplicatedForcedAimTime is going to get replicated
		// By making sure we're assigning a different value.
		if( ByteAimTime == ReplicatedForcedAimTime )
		{
			ReplicatedForcedAimTime = ByteAimTime + 1;
		}
		else
		{
			ReplicatedForcedAimTime = ByteAimTime;
		}

		WorldInfo.bForceNetUpdate = TRUE;
	}
}

function OnSetWeaponAlert(SeqAct_SetWeaponAlert Action)
{
	SetWeaponAlert(Action.MaxAlertTime);
}


// Support for Matinee Anim Tracks.

/** Start AnimControl. Add required AnimSets. */
native function MAT_BeginAnimControl(Array<AnimSet> InAnimSets);

/** Update AnimTree from track info */
native function MAT_SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping);

/** Update AnimTree from track weights */
native function MAT_SetAnimWeights(Array<AnimSlotInfo> SlotInfos);

/** End AnimControl. Release required AnimSets */
native function MAT_FinishAnimControl();

/**
* Returns socket used for Right Hand (main weapon hand).
* When using animation mirroring, left and right hand sockets are inverted.
*/

simulated native function Name GetRightHandSocketName();

/**
* Returns the weapon hand bone position for this pawn's mesh
* @ param HandLoc is an out parameter returning the weapon hand bone location
* @ param HandRot is an out parameter returning the weapon hand bone rotation
*/
simulated native function GetWeaponHandPosition(out vector HandLoc, out rotator HandRot);

simulated native event rotator GetViewRotation();


/** Called when we start an AnimControl track operating on this Actor. Supplied is the set of AnimSets we are going to want to play from. */
simulated event BeginAnimControl(Array<AnimSet> InAnimSets)
{
	MAT_BeginAnimControl(InAnimSets);
}

/** Called each from while the Matinee action is running, with the desired sequence name and position we want to be at. */
simulated event SetAnimPosition(name SlotName, int ChannelIndex, name InAnimSeqName, float InPosition, bool bFireNotifies, bool bLooping)
{
	MAT_SetAnimPosition(SlotName, ChannelIndex, InAnimSeqName, InPosition, bFireNotifies, bLooping);
}

/** Called when we are done with the AnimControl track. */
simulated event FinishAnimControl()
{
	MAT_FinishAnimControl();
}

/**
 * Return the base location targetting should auto-aim for.
 */
simulated function vector GetBaseTargetLocation()
{
	if( Mesh != None && PelvisBoneName != '' )
	{
		return Mesh.GetBoneLocation(PelvisBoneName);
	}
	return Location;
}

/** Returns melee attack range, taking current state of the pawn into account (weapon, stance, whatever). */
simulated final function float GetMeleeAttackRange()
{
	return (MyGearWeapon != None) ? MyGearWeapon.MeleeAttackRange : UnarmedMeleeRange;
}

/** @return how close AI should get to an enemy before attempting melee
 * this doesn't affect the weapon's rules, just the AI behavior
 * so for correct behavior this should return a value equal or less than GetMeleeAttackRange()
 */
final function float GetAIMeleeAttackRange()
{
	return (MyGearWeapon != None) ? MyGearWeapon.GetAIMeleeAttackRange() : UnarmedMeleeRange;
}

/**
 *	Gives pawn a chance to override weapon melee damage
 *	Return TRUE if overridden & out_Damage updated
 *		   FALSE if should use default weapon damage
 */
simulated function bool GetMeleeDamageOverride( out float out_Damage );

/**
 * Whether the pawn attempting the special move is facing another pawn
 */
native final simulated function bool IsFacingOther(GearPawn OtherPawn, optional float MinDot = 0.5f, optional float HeightTolerancePct = 0.8f) const;

/**
 * Returns TRUE if relevant for gameplay. That is not hidden, pending destruction or dead.
 * DBNO players are considered gameplay relevant.
 */
final simulated function bool IsGameplayRelevant()
{
	return !bHidden && !bDeleteMe && !bPlayedDeath;
}

/**
 * Returns true if given target is a valid melee target for this GearPawn, false otherwise.
 */
simulated function bool TargetIsValidMeleeTarget(GearPawn TestGP, bool bLookForPawnsInStateRevival)
{
	local vector ToTargetNorm;
	local float DistToTarget;
	local GearGRI GRI;

	// can't target self or fully dead/deleted pawns
	if ( (TestGP == Self) || TestGP.bDeleteMe || TestGP.bPlayedDeath || TestGP.bInvalidMeleeTarget )
	{
		return FALSE;
	}

	// prevent attacking a teammate's meatshield
	GRI = GearGRI(WorldInfo.GRI);
	if( TestGP.IsAHostage() && !GRI.bAllowFriendlyFire && GRI.OnSameTeam(TestGP.InteractionPawn,self) )
	{
		return FALSE;
	}

	// can't target dead guys unless specifically looking for DBNO guys
	if( (TestGP.Health <= 0) && (!bLookForPawnsInStateRevival || (!TestGP.IsDBNO() && !TestGP.IsAHostage())) )
	{
		return FALSE;
	}

	// can't target teammates (unless they're hostages)
	if( !TestGP.IsAHostage() && !GRI.bAllowFriendlyFire && GRI.OnSameTeam(TestGP, Self) )
	{
		return FALSE;
	}

	ToTargetNorm = TestGP.Location - Location;
	DistToTarget = VSize(ToTargetNorm);
	ToTargetNorm /= DistToTarget;

	// can't target outside of melee attack range
	if (DistToTarget > GetMeleeAttackRange())
	{
		return FALSE;
	}

	// can't target guys protected by cover
	if (TestGP.IsProtectedByCover(ToTargetNorm, TRUE))
	{
		return FALSE;
	}

	// can't target guys protected by shields
	if (TestGP.IsDeployingShield() && TestGP.IsProtectedByShield(ToTargetNorm))
	{
		return FALSE;
	}

	// We can target him!
	return TRUE;
}


/**
 * Returns TRUE if this pawn is in a valid state to engage in a melee attack.
 */
simulated function bool CanEngageMelee()
{
	local GearPC PC;
	if (Controller == None || Health <= 0 || bIsConversing || bInCommLinkStance || bSwitchingWeapons || Physics == PHYS_RigidBody)
	{
		return FALSE;
	}

	if( MyGearWeapon != None )
	{
		if( (WorldInfo.TimeSeconds - LastMeleeAttackTime) < MyGearWeapon.MeleeAttackCoolDownInSeconds)
		{
			return FALSE;
		}

		if( !MyGearWeapon.CanDoSpecialMeleeAttack() ||
			MyGearWeapon.CurrentFireMode == class'GearWeapon'.const.FIREMODE_FAILEDACTIVERELOAD ||
			MyGearWeapon.CurrentFireMode == class'GearWeapon'.const.FIREMODE_ACTIVERELOADSUCCESS ||
			MyGearWeapon.CurrentFireMode == class'GearWeapon'.const.FIREMODE_ACTIVERELOADSUPERSUCCESS )
		{
			return FALSE;
		}

		// disallow if chainsaw currently interrupted
		if (GearWeap_AssaultRifle(MyGearWeapon) != None && IsChainsawInterrupted())
		{
			return FALSE;
		}
	}

	// no melee while roadie running or attempting to acquire cover
	PC = GearPC(Controller);
	if (PC != None && PC.IsHoldingRoadieRunButton())
	{
		return FALSE;
	}

	if( SpecialMove != SM_None && !SpecialMoves[SpecialMove].bCanFireWeapon )
	{
		return FALSE;
	}

	// no melee while shield is deployed
	if( IsDeployingShield() )
	{
		return FALSE;
	}

	return TRUE;
}

/** Rules for delaying a melee attack */
simulated function bool ShouldDelayMeleeAttack()
{
	// if holding main fire then delay the melee attack
	return IsControllerFireInputPressed(0);
}

/** This will return whether or not this mob can be special melee attacked **/
simulated function bool CanBeSpecialMeleeAttacked( GearPawn Attacker )
{
	// Don't try to special melee attack people on a different moving platform
	if( ClampedBase != None &&
		Attacker != None &&
		Attacker.ClampedBase != None &&
		Attacker.ClampedBase != ClampedBase )
	{
		return FALSE;
	}

	// ignore pawns ragdolled or recovering from ragdoll
	if (IsDoingSpecialMove(SM_RecoverFromRagdoll) || (Physics == PHYS_RigidBody && !IsAHostage()))
	{
		return FALSE;
	}

	// Don't allow special melee attacks vs players in SP if they are already chainsawing
	if( IsHumanControlled() &&
		GearGameSP_Base(WorldInfo.Game) != None &&
		(IsDoingSpecialMove( SM_ChainSawAttack )		||
		 IsDoingSpecialMove( SM_ChainSawVictim )		||
		 IsDoingSpecialMove( SM_ChainsawDuel_Leader )	||
		 IsDoingSpecialMove( SM_ChainsawDuel_Follower ) ||
		 IsDoingSpecialMove( SM_ChainsawDuel_Draw ))	)
	{
		return FALSE;
	}

	return !bDeleteMe;
}

/**
 *	This will attempt a melee attack
 *
 *	MeleeAttacks are determined by the weapon the pawn is holding.	So a pistol maybe
 * gives you a pistol whip melee attack, the AssaultRifle gives you a chainsaw melee attack.
 *
 * To have a melee attack work you need to be:
 *
 *	-in range of your target
 *	-be targeting your target
 *	-not have done a melee attack in the last MeleeAttackCoolDownInSeconds
 *
 * If you are able to do a melee attack you will do RandRange( MeleeAttackDamageMin, MeleeAttackDamageMax )
 * for the specific weapon's melee attack.
 */
simulated function StartMeleeAttack()
{
	local GearInventoryManager Manager;
	local GearPC PC;

	if( IsCarryingAHeavyWeapon() )
	{
		if (IsDoingSpecialMove(SM_TargetMinigun) || IsDoingSpecialMove(SM_UnMountMinigun) || IsDoingSpecialMove(SM_TargetMortar) || IsDoingSpecialMove(SM_UnMountMortar))
		{
			// abort in this case!
			EndSpecialMove();
		}
		// EndSpecialMove() might have caused the weapon to be thrown already if e.g. it was out of ammo
		if (IsCarryingAHeavyWeapon())
		{
			Manager = GearInventoryManager(InvManager);
			if( Manager != None && Manager.PreviousEquippedWeapon != None )
			{
				Manager.SetCurrentWeapon(Manager.PreviousEquippedWeapon);
			}
			else
			{
				ThrowActiveWeapon();
			}
		}
	}

	if( IsChainsawInterrupted() )
	{
		if( CanDoSpecialMove(SM_StumbleFromMelee2) && !IsDoingASpecialMove() )
		{
			PC = GearPC(Controller);
			// do it client-side if possible for less wonkiness
			if (PC != None && PC.IsLocalPlayerController())
			{
				PC.DoSpecialMove(SM_StumbleFromMelee2,TRUE);
			}
			else
			{
				DoStumbleFromMeleeSpecialMove();
			}
		}
		// no melee for you!
		return;
	}

	// Set this flag indicating our desire to do a melee attack.
	// Code in Tick() will check to see if we can actually start it.
	bWantsToMelee = TRUE;
}

simulated function StopMeleeAttack(optional bool bForcedByDamage)
{
	bWantsToMelee = FALSE;
	bPendingMeleeAttack = FALSE;

	if( bForcedByDamage )
	{
		// If melee was stopped because we took damage,
		// remember when it hapened to add a delay
		LastTookDamageTime = WorldInfo.TimeSeconds;
		MyGearWeapon.StopFire(class'GearWeapon'.const.MELEE_ATTACK_FIREMODE);
	}
}

/** Called from server to forcibly stop the Pawn's Melee attack **/
function ForceStopMeleeAttack(optional bool bForcedByDamage)
{
	StopMeleeAttack(bForcedByDamage);
	if( !IsLocallyControlled() )
	{
		ClientStopMeleeAttack(bForcedByDamage);
	}
}

/** Replicated function called from server to client, to Stop Melee Attack */
reliable client function ClientStopMeleeAttack(optional bool bForcedByDamage)
{
	StopMeleeAttack(bForcedByDamage);
}

/** Returns TRUE if a Pawn is doing a Stumble because of a Melee hit */
simulated final function bool IsDoingStumbleFromMelee()
{
	return IsDoingSpecialMove(SM_StumbleFromMelee) || IsDoingSpecialMove(SM_StumbleFromMelee2);
}

/** Play Stumble Hit Reaction from a melee impact */
function DoStumbleFromMeleeSpecialMove()
{
	if( CanDoSpecialMove(SM_StumbleFromMelee2) )
	{
		if( MyGearAI != None )
		{
			class'AICmd_React_StumbleFromMelee'.static.InitCommand(MyGearAI);
		}
		else
		{
			// only use #2 since that includes some RM to move the players apart (which is what we want for gameplay reasons, if not aesthetically)
			ServerDoSpecialMove(SM_StumbleFromMelee2, TRUE);
		}
	}
}

final function bool IsDoingAStumbleGoDown()
{
	return (IsDoingSpecialMove(SM_StumbleGoDown) || IsDoingSpecialMove(SM_StumbleGoDownFromExplosion) || IsDoingSpecialMove(SM_StumbleGoDownFromCloseRangeShot));
}

/** Start the Stumble Back Special Move */
function DoStumbleGoDownSpecialMove()
{
	// make sure Pawn can actually do it. Some enemies don't support the animations.
	if( CanDoSpecialMove(SM_StumbleGoDown) )
	{
		ServerDoSpecialMove(SM_StumbleGoDown, TRUE);
	}
}

/** Get Back up from a knock down special move */
final function GetBackUpFromKnockDown()
{
	// Clear Timer to be safe
	ClearTimer('GetBackUpFromKnockDown');

	if( IsDoingAStumbleGoDown() && CanDoSpecialMove(SM_StumbleGetUp) )
	{
		ServerDoSpecialMove(SM_StumbleGetUp, TRUE);
	}
}


/**
 * Event called when melee attack begins.
 * This means when weapon is enterring melee attacking state on local player and server.
 * When Pawn.FiringMode == MELEE_ATTACK_FIREMODE is replicated on remote clients.
 */
simulated function MeleeAttackStarted(GearWeapon Weap)
{
	local GearPC			GPC;
	local GearPawn		AdhesionTarget;
	local float			TargetHealthPct;

	bDoingMeleeAttack = TRUE;

	// abort any special moves
	if( Role == Role_Authority )
	{
		if( IsDoingASpecialMove()
			&& !IsDoingSpecialMeleeAttack()
			&& !IsDoingSpecialMove(SM_ChainSawHold)
			&& !IsDBNO()
			&& !IsAHostage()
			&& !IsAKidnapper()
			)
		{
			ServerEndSpecialMove();
		}
	}

	GPC = GearPC(Controller);
	if( GPC != None )
	{
		if( Weap.bDoMeleeAdhesion )
		{
			//`log( "StartMeleeAttack starting Adhesion" );
			// try to find best target around that we can turn towards
			AdhesionTarget = GPC.AttemptMeleeAdhesion();
		}

		// Break from cover
		if( GPC.IsInCoverState() )
		{
			GPC.ClientBreakFromCover();
		}
	}

	// lancer has custom melee, skip the normal efforts, etc
	if( GearWeap_AssaultRifle(Weap) != None )
	{
		return;
	}

	// attack effort sound
	if( AdhesionTarget == None )
	{
		//`log( "StartMeleeAttack 5	 AdhesionTarget == none " );
		SoundGroup.PlayEffort(self, GearEffort_MeleeAttackSmallEffort, true);
	}
	else
	{
		TargetHealthPct = AdhesionTarget.Health / float(AdhesionTarget.DefaultHealth);
		if (TargetHealthPct < 0.3f)
		{
			SoundGroup.PlayEffort(self, GearEffort_MeleeAttackLargeEffort, true);
		}
		else if (TargetHealthPct < 0.6f)
		{
			SoundGroup.PlayEffort(self, GearEffort_MeleeAttackMediumEffort, true);
		}
		else
		{
			SoundGroup.PlayEffort(self, GearEffort_MeleeAttackSmallEffort, true);
		}
	}
}


/**
 * Event called when melee attack ends.
 * This means when weapon leaves melee attacking state on local player and server.
 * When Pawn.FiringMode != MELEE_ATTACK_FIREMODE is replicated on remote clients, and previous firing mode was MELEE_ATTACK_FIREMODE.
 */
simulated function MeleeAttackEnded(GearWeapon Weap)
{
	local GearPC	PC;

	LastMeleeAttackTime = WorldInfo.TimeSeconds;
	bDoingMeleeAttack = FALSE;

	PC = GearPC(Controller);
	if( PC != None )
	{
		PC.StopMeleeAdhesion();
	}
}


/**
 * Function allowing the Pawn to override a weapon melee attack with his own version.
 * @param	out_AnimTime	Play length of the animation played.
 * @return					TRUE to override, FALSE to let weapon play default animations.
 */
simulated function bool OverridePlayMeleeAttackAnim(out float Out_AnimTime)
{
	return FALSE;
}

/** Have us track a specific pawn */
final simulated function SetAdhesionTarget(GearPawn TargetPawn)
{
	local GearPC	PC;

	PC = GearPC(Controller);
	if( PC != None )
	{
		PC.ForcedAdhesionTarget = TargetPawn;
	}
}


final simulated function GearPawn GetAdhesionTarget()
{
	local GearPC	PC;

	PC = GearPC(Controller);
	if( PC != None )
	{
		return PC.ForcedAdhesionTarget;
	}

	return None;
}

/** Called after a gored pawn has been around too long. */
simulated private function GoreAroundTooLong()
{
	bScalingToZero = TRUE;
}


/**
 * This will create the gore skeleton which will be constructed to break apart.
 */
simulated final function CreateGoreSkeleton(SkeletalMesh TheSkeletalMesh, PhysicsAsset ThePhysicsAsset)
{
	local Array<Attachment> PreviousAttachments;
	local int				i, Idx;
	local array<texture>	Textures;

	// Need proper physics asset and mesh
	if( TheSkeletalMesh == None || ThePhysicsAsset == None || WorldInfo.GRI.ShouldShowGore() == FALSE )
	{
		return;
	}

	// Don't set gore skeleton during death animation, as current ragdoll is using motors.
	if( IsDoingDeathAnimSpecialMove() )
	{
		`log("Can't create gore skeleton while in SM_DeathAnim"@self);
		return;
	}

	// Freeze animations upon going ragdoll except when being headshotted. we'll do it once we switch to ragdoll.
	if( bPlayedDeath && !bHasPlayedHeadShotDeath )
	{
		// This will capture a pose and clear the tree
		StopAllAnimations();
	}

	// Do this if we haven't switched to the gore skeleton yet
	if( !bIsGore )
	{
		PreviousAttachments = Mesh.Attachments;
		SetCollisionSize(1.0f, 1.0f);
		CylinderComponent.SetTraceBlocking(FALSE, FALSE);

		// so only if the phys asset is different shall we change it out
		if( ThePhysicsAsset != Mesh.PhysicsAsset )
		{
			Mesh.SetPhysicsAsset(None);
		}

		SetMainBodyMaterialToNoneToClearItForGoreMaterial();


		Mesh.bDisableWarningWhenAnimNotFound = TRUE;
		Mesh.SetSkeletalMesh( TheSkeletalMesh, TRUE, TRUE );
		Mesh.bDisableWarningWhenAnimNotFound = FALSE;

		// Update MorphTargets
		if( GoreMorphSets.Length > 0 )
		{
			Mesh.MorphSets = GoreMorphSets;
		}

		// so only if the phys asset is different shall we change it out
		if( ThePhysicsAsset != Mesh.PhysicsAsset )
		{
			Mesh.SetPhysicsAsset(ThePhysicsAsset);
		}

		Mesh.MotionBlurScale = 0.0f;
		Mesh.CastShadow = FALSE;  // turn off shadow casting for perf

		// so if we have not had stretchies fixed then hard weight everything
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

		if( bEnableEncroachCheckOnRagdoll )
		{
			// If we are checking encroachment (ie looking for physics volumes) and we are gibbed - need to do a bit more checking
			// This is to handle just one gib going into a volume.
			Mesh.bPerBoneVolumeEffects = TRUE;
			bAlwaysEncroachCheck = TRUE;
		}

/*		`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Created gore skeleton!");*/
	}

	if( IsAHostage() )
	{
		// Hostage needs to remain animated, so make sure we fix all bodies.
		Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
		// Unfix Dangle bones as well
		Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);
		// Enable full physics on body
		Mesh.PhysicsWeight = 1.f;
		// Set physics to be updated from Kinematic data
		Mesh.bUpdateJointsFromAnimation = TRUE;
		Mesh.bUpdateKinematicBonesFromAnimation = TRUE;
		Mesh.MinDistFactorForKinematicUpdate = 0.f;
		// Make it start simulating
		Mesh.WakeRigidBody();
	}
	else
	{
		// Make sure all bodies are unfixed
		if( Mesh.PhysicsAssetInstance != none )
		{
			Mesh.PhysicsAssetInstance.SetAllBodiesFixed(FALSE);

			// Turn off motors
			Mesh.PhysicsAssetInstance.SetAllMotorsAngularPositionDrive(FALSE, FALSE, Mesh, TRUE);
		}
	}

	bIsGore = TRUE;
}

/** This will do what ever is needed to reset materials to allow the gore overlay material to show up **/
simulated function SetMainBodyMaterialToNoneToClearItForGoreMaterial()
{
	local int idx;

	for( Idx = 0; Idx < Mesh.Materials.Length; ++Idx )
	{
		//`log( "clearing material to make gore work" @ Mesh.Materials[Idx] );
		Mesh.SetMaterial( Idx, None ); // this is needed as we have to "clear" the material instance on the mesh to make the gore material which is affecting the same verts can be seen.
	}
}


/**
 * Setup Gore Skeleton to be gibbed.
 * This turns on some optimizations so Gibs don't kill performance.
 */
simulated final function SetupDeathGibsOptimizations()
{
	// Do this only once for Gore Skeletons
	// Don't setup those for non dead characters. (ie Hostages)
	if( bGoreSetupForDeath || !bIsGore || !bPlayedDeath )
	{
		return;
	}

	bGoreSetupForDeath = TRUE;

	// When you gore the skeleton, the bounding box can get very large, so the pawn can be considered 'always seen'
	// This forces it to be cleaned up after a certain period.
	SetTimer( 60.0, FALSE, nameof(GoreAroundTooLong) );

	// Freeze the light environment state at the state when the pawn was gibbed; this prevents the light environment being computed
	// for the large aggregate bounds of the gibs, which usually doesn't give good results.
	// we need to wait a number of seconds here so we do not capture the explosion light
	// NOTE: this can look bad when the gib is near a bright colored light and then is kicked out away from it into darkness
	SetTimer( 5.0f, FALSE, nameof(SetLightEnvironmentToNotBeDynamic) );

	// turn off shadows and SH lights
	LightEnvironment.bCastShadows = FALSE;
	LightEnvironment.bSynthesizeSHLight = FALSE;
}

/** This will turn "off" the light environment so it will no longer update **/
simulated final function SetLightEnvironmentToNotBeDynamic()
{
	LightEnvironment.bDynamic = FALSE;
}

simulated function UpdatePawnLightEnvironment(LightEnvironmentComponent NewComponent)
{
	if( Mesh != None )
	{
		Mesh.SetLightEnvironment(NewComponent);
	}
	if( EquippedShield != None )
	{
		EquippedShield.Mesh.SetLightEnvironment(NewComponent);
	}
}

simulated event ConstraintBrokenNotify( Actor ConOwner, RB_ConstraintSetup ConSetup, RB_ConstraintInstance ConInstance	)
{
	local ParticleSystemComponent BloodSpray;
	local ParticleSystemComponent BloodSpray2;
	//local vector ConstraintLocation;

	local SkeletalMeshComponent SMC;
	local vector BoneRelativeLocation;
	local rotator BoneRelativeRotation;
	//local vector WorldRelativeLocation;
	//local rotator WorldRelativeRotation;

	// make sure we should bother spawning an effect
	if(( NumConstraintEffectsSpawned > NumConstraintEffectsSpawnedMax )
		|| ( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 2048, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
		)
	{
		return;
	}

	if( ( ConInstance != None ) && ( WorldInfo.GRI.ShouldShowGore() ) )
	{
		NumConstraintEffectsSpawned++;

		// use this to spawn the "breaking of the constraint effect"
		//ConstraintLocation = ConInstance.GetConstraintLocation();
		//War_Effects.Blood.P_Blood_Hit_Large

		SMC = SkeletalMeshComponent(ConInstance.OwnerComponent);
		//`log( "SMC: " $ SMC );
		//`log( "ConSetup.ConstraintBone1: " $ ConSetup.ConstraintBone1 );
		//`log( "ConSetup.ConstraintBone2: " $ ConSetup.ConstraintBone2 );

		// nice effect to use for seeing the x axis
		//BloodSpray.SetTemplate( ParticleSystem'COG_AssaultRifle.Effects.P_COG_AssaultRifle_MuzzleFlash_Constant'	);


		// now spawn the persistent spraying of blood effects

		// CHILD
		//BloodSpray = new(Outer) class'ParticleSystemComponent';
		//BloodSpray.bAutoActivate = FALSE;
		//BloodSpray.SetTemplate( ParticleSystem'War_Effects.Blood.P_Blood_flying_bodypart'	 );
		//BloodSpray.SetTemplate( ParticleSystem'War_Effects.PS.P_Test_Arrow' ); // red

		BloodSpray = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( ParticleSystem'Effects_Gameplay.Blood.P_Blood_flying_bodypart' );


		BoneRelativeLocation = (ConSetup.Pos1 * 50);
		BoneRelativeRotation = rotator( (ConSetup.Pos1 * 50) * vect(-1,0,0) );

		//SMC.TransformFromBoneSpace( ConSetup.ConstraintBone1, BoneRelativeLocation, BoneRelativeRotation, WorldRelativeLocation, WorldRelativeRotation );

		SMC.AttachComponent( BloodSpray, ConSetup.ConstraintBone1, BoneRelativeLocation, BoneRelativeRotation );
		BloodSpray.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(BloodSpray.Template, ConInstance.GetConstraintLocation()));
		BloodSpray.ActivateSystem();

		// PARENT
		// don't spawn the constrain breaking effect if we are gore exploding
		// bHasGoreExploded is set AFTER the various constraint breaking has occurred.  So at that point you can spawn the break
		// as that means someone is playing with the gore.  The same is for bPlayedDeath.  Means he died and you want to be able to break stuff off
		if( bHasGoreExploded || bPlayedDeath )
		{
			//BloodSpray2 = new(Outer) class'ParticleSystemComponent';
			//BloodSpray2.bAutoActivate = FALSE;
			//BloodSpray2.SetTemplate( ParticleSystem'Effects_Gameplay.Blood.P_Bloodspray_hit_effect'  );
			//BloodSpray2.SetTemplate( ParticleSystem'War_Effects.PS.P_Test_Arrow2' ); // blue

			BloodSpray2 = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( ParticleSystem'Effects_Gameplay.Blood.P_Bloodspray_hit_effect' );

			BoneRelativeLocation = (ConSetup.Pos2 * 50);
			BoneRelativeRotation = rotator( (ConSetup.Pos2 * 50) * vect(1,0,0) );

			//SMC.TransformFromBoneSpace( ConSetup.ConstraintBone2, BoneRelativeLocation, BoneRelativeRotation, WorldRelativeLocation, WorldRelativeRotation );

			SMC.AttachComponent( BloodSpray2, ConSetup.ConstraintBone2, BoneRelativeLocation, BoneRelativeRotation );
			BloodSpray2.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse(BloodSpray2.Template, ConInstance.GetConstraintLocation()));
			BloodSpray2.ActivateSystem();

			// 50% of the time spawn decal
			if( bSpawnBloodTrailDecals && FRand() > 0.50f )
			{
				// when limbs break we need to spawn some blood on the ground
				SpawnABloodTrail_LimbBreak( ConInstance.GetConstraintLocation() );
			}
		}


		/*
		BoneRelativeLocation = (ConSetup.Pos1 * 50);
		BoneRelativeRotation = rotator( (ConSetup.Pos1 * 50) * vect(1,0,0) );

		SMC.TransformFromBoneSpace( ConSetup.ConstraintBone1, BoneRelativeLocation, BoneRelativeRotation, WorldRelativeLocation, WorldRelativeRotation );

		SMC.AttachComponent( BloodSpray, ConSetup.ConstraintBone1, BoneRelativeLocation, BoneRelativeRotation );


		// transform from one bone's world space into the other bone's relative
		SMC.TransformFromBoneSpace( ConSetup.ConstraintBone1, BoneRelativeLocation, BoneRelativeRotation, WorldRelativeLocation, WorldRelativeRotation );

		SMC.TransformToBoneSpace( ConSetup.ConstraintBone2, WorldRelativeLocation, WorldRelativeRotation, BoneRelativeLocation, BoneRelativeRotation );


		SMC.AttachComponent( BloodSpray2, ConSetup.ConstraintBone2, BoneRelativeLocation, BoneRelativeRotation );
*/

		// reset the count so when you go and shoot limbs later you will get correct effects
		SetTimer( 1.0f, FALSE, nameof(ResetNumConstraintEffectsSpawned) );
	}
}

/** This will reset the counter for the constraint effects spawned. **/
simulated function ResetNumConstraintEffectsSpawned()
{
	NumConstraintEffectsSpawned = 0;
}

simulated function ShutDown()
{
	if (Controller != None)
	{
		Controller.PawnDied(self);
		Controller.Destroy();
	}
	Super.ShutDown();
}

/**
 * Play effects for running into the wall during a run to cover.
 */
simulated function DoRun2CoverWallHitEffects()
{
	local GearPC PC;
	local float ShakeScale;
	local ScreenShakeStruct Shake;

	local Emitter DustEmitter;
	local vector SpawnLocation, SocketLocation;
	local rotator SpawnRotation;
	local bool bSocketFound;

	local vector out_HitLocation;
	local vector out_HitNormal;
	local vector TraceDest;
	local vector TraceStart;
	local vector TraceExtent;
	local TraceHitInfo HitInfo;


	if( WorldInfo.NetMode == NM_DedicatedServer )
	{
		return;
	}

	// do a screenshake to add weight to the impact
	if( IsHumanControlled() && IsLocallyControlled() )
	{
		// play camera shake only for PCs
		PC = GearPC(Controller);
		if( PC != None )
		{
			if( LastCoverAcquireDistanceSq > Run2CoverMinCameraShakeDistSqThreshold )
			{
				// scale the shake based on distance we ran to enter cover
				ShakeScale = (LastCoverAcquireDistanceSq - Run2CoverMinCameraShakeDistSqThreshold) / (Run2CoverMaxCameraShakeDistSqThreshold - Run2CoverMinCameraShakeDistSqThreshold);
				ShakeScale = FClamp(ShakeScale, 0.f, 1.f);

				Shake = Run2CoverCameraShake;
				Shake.FOVAmplitude *= ShakeScale;
				Shake.FOVFrequency *= ShakeScale;
				Shake.RotAmplitude *= ShakeScale;
				Shake.RotFrequency *= ShakeScale;
				Shake.LocFrequency *= ShakeScale;
				Shake.LocAmplitude *= ShakeScale;

				PC.ClientPlayCameraShake(Shake);

				PC.ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.EnterCoverHard);
			}
		}
	}

	// only play an impact grunt if we have slide a decent distance
	//`log( "LastCoverAcquireDistanceSq: " $ LastCoverAcquireDistanceSq );
	if (Role == ROLE_Authority && LastCoverAcquireDistanceSq > 36000)
	{
		SoundGroup.PlayEffort(self, GearEffort_CoverSlamEffort, true);
	}


	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 2048, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		// play shoulder dust
		if( bIsMirrored )
		{
			// play the "enter cover" particle effect
			bSocketFound = Mesh.GetSocketWorldLocationAndRotation( 'LeftShoulderCoverDust', SocketLocation );
		}
		else
		{
			// play the "enter cover" particle effect
			bSocketFound = Mesh.GetSocketWorldLocationAndRotation( 'RightShoulderCoverDust', SocketLocation );
		}

		if( bSocketFound == TRUE )
		{
			// move spawnlocation to be on the wall, but at shoulder height (determined by the socket).
			// using just the socket position tends to put the emitter inside the wall due to how the animation is.
			SpawnLocation = Location - (GetCollisionRadius() - 20) * AcquiredCoverInfo.Normal;
			SpawnLocation.Z = SocketLocation.Z;

			SpawnRotation = rotator(AcquiredCoverInfo.Normal);

			TraceStart = SpawnLocation;
			TraceDest = TraceStart - (64 * AcquiredCoverInfo.Normal);

			// trace down and see what we are standing on
			Trace( out_HitLocation, out_HitNormal, TraceDest, TraceStart, false, TraceExtent, HitInfo, TRACEFLAG_PhysicsVolumes );
			//DrawDebugLine( TraceStart, TraceDest, 255, 0, 0, TRUE);

			if( IsHumanControlled() && IsLocallyControlled() )
			{
				GearPC(Controller).ClientSpawnCameraLensEffect( class'GearPhysicalMaterialProperty'.static.DetermineSlideIntoCoverCameraLensEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( HitInfo ), WorldInfo) );

				DustEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( class'GearPhysicalMaterialProperty'.static.DetermineSlideIntoCoverEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( HitInfo ), WorldInfo), SpawnLocation, SpawnRotation );
			}
			else
			{
				DustEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( class'GearPhysicalMaterialProperty'.static.DetermineSlideIntoCoverNonPlayerEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( HitInfo ), WorldInfo), SpawnLocation, SpawnRotation );
			}

			DustEmitter.ParticleSystemComponent.ActivateSystem();
		}
	}

	// blood trail along wall if hurt
	// for now just do it a decent amount to test it
	if( bSpawnBloodTrailDecals && (Health < (0.65f * DefaultHealth)) )
	{
		GearPawnFX.LeaveADecal( GearPawnFX.BloodDecalTrace_CoverBehindPawnMiddleOfBody, GearPawnFX.BloodDecalChoice_Wall, GearPawnFX.BloodDecalTimeVaryingParams_Wall );
		StartBloodTrail( 'SpawnABloodTrail_Wall' );
	}

	// play sound effects for both PCs and NPCs
	if( Controller != None )
	{
		// check to see if we are going into standing or midlevel cover
		if( CoverType == CT_MidLevel )
		{
			SoundGroup.PlayFoleySound( self, GearFoley_EnterCoverLowFX );
		}
		else if( CoverType == CT_Standing )
		{
			SoundGroup.PlayFoleySound( self, GearFoley_EnterCoverHighFX );
		}

		SoundGroup.PlayFoleySound( self, GearFoley_BodyWallImpactFX );
	}
}


/** sound to play when running around  (triggered from script notify on animation)	**/
simulated function PlayBodyMovementSounds_Running()
{
	SoundGroup.PlayFoleySound(self, GearFoley_BodyMovement_RunningFX);
}

/** sound to play when roadie running (triggered from script notify on animation) **/
simulated function PlayBodyMovementSounds_RoadieRun()
{
	SoundGroup.PlayFoleySound(self, GearFoley_BodyMovement_RoadieRunFX);
}

simulated function PlayBodyMovementSounds_GruntSound()
{
	SoundGroup.PlayEffort(self, GearEffort_MantleEffort);
}
simulated function PlayBodyMovementSounds_MartyrRollover()
{
	// this one sounds pretty good
	SoundGroup.PlayFoleySound(self, GearFoley_EnterCoverHighFX);
}

/**
 * Handle notification that this pawn was killed.
 */
function NotifyKilled(Controller Killer)
{
	// Store killer.  Note that Pawn::Died() could potentially modify it's input
	// killer parameter, so we store the killer here to make sure that function
	// has had a chance to do it's thing and pass the information up.
	if( Killer != None )
	{
		KilledByPawn = GearPawn(Killer.Pawn);
	}
	else
	{
		KilledByPawn = None;
	}
}


/**
 * Checks for nearby pickups, returns the first one found.
 */
final simulated function Actor FindNearbyPickup()
{
	local Actor				A, BestPickup;
	local float				Dist, BestDist;

	// use fast touching list
	ForEach TouchingActors(class'Actor', A)
	{
		if( CanPickUpActor(A) )
		{
			Dist = VSize(Location-A.Location);
			if( BestPickup == None || Dist < BestDist )
			{
				BestPickup	= A;
				BestDist	= Dist;
			}
		}
	}

	return BestPickup;
}

/** Returns true if Pawn can pickup actor A. */
final simulated function bool CanPickUpActor(Actor A)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;

	PF = GearPickupFactory(A);
	DP = GearDroppedPickup(A);
	return( (PF != None && PF.CanBePickedUpBy(Self)) || (DP != None && DP.CanBePickedUpBy(Self)) );
}

/**
 * Checks for nearby discoverables, returns the first one found.
 */
final simulated function GearDiscoverablesPickupFactoryBase FindNearbyDiscoverables()
{
	local Actor				A;
	local GearDiscoverablesPickupFactoryBase Discoverable;

	// use fast touching list
	ForEach TouchingActors(class'Actor', A)
	{
		Discoverable = GearDiscoverablesPickupFactoryBase(A);
		if (Discoverable != None)
		{
			return Discoverable;
		}
	}

	return None;
}


/**
 * What interaction can we do with this pickup?
 */
final simulated function Name FindInteractionWithPickup(Actor A)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;

	DP = GearDroppedPickup(A);
	if( DP != None )
	{
		return DP.FindInteractionWith(Self,false);
	}

	PF = GearPickupFactory(A);
	if( PF != None )
	{
		return PF.FindInteractionWith(Self,false);
	}

	return '';
}

/** Only play kick up animation on "shoulder" weapons. Not pistol, not grenades or any other type of pickups. */
final simulated function bool ShouldPlayKickUpAnim(Actor A)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;
	local GearWeapon			Weap;
	local class<GearWeapon>	WeapClass;

	// Don't play kick up animation when just grabbing ammo from the gun.
	if( FindInteractionWithPickup(A) == 'TakeAmmo' )
	{
		return FALSE;
	}

	DP = GearDroppedPickup(A);
	if( DP != None )
	{
		Weap = GearWeapon(DP.Inventory);
		return Weap != None && Weap.WeaponType == WT_Normal;
	}

	PF = GearPickupFactory(A);
	if( PF != None )
	{
		WeapClass = Class<GearWeapon>(PF.InventoryType);
		return WeapClass != None && WeapClass.default.WeaponType == WT_Normal;
	}

	return FALSE;
}

final simulated function bool IsPickupAHeavyWeapon(Actor A)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;
	local GearWeapon			Weap;
	local class<GearWeapon>	WeapClass;

	if( FindInteractionWithPickup(A) == 'TakeAmmo' )
	{
		return FALSE;
	}

	DP = GearDroppedPickup(A);
	if( DP != None )
	{
		Weap = GearWeapon(DP.Inventory);
		return Weap != None && Weap.WeaponType == WT_Heavy;
	}

	PF = GearPickupFactory(A);
	if( PF != None )
	{
		WeapClass = Class<GearWeapon>(PF.InventoryType);
		return WeapClass != None && WeapClass.default.WeaponType == WT_Heavy;
	}

	return FALSE;
}

/** Tell Pawn to pick up weapon A */
reliable server function PickupWeapon(Actor A)
{
	if (CanPickUpActor(A) && CanDoSpecialMove(SM_WeaponPickup))
	{
		// Claim pickup so noone else can pick it up
		ClaimPickup(A);
		VerifySMHasBeenInstanced(SM_WeaponPickup);
		// Let our special move know about it, so we can pick it up later. (based on animation timing).
		GSM_Pickup(SpecialMoves[SM_WeaponPickup]).PickupActor = A;

		if( ShouldPlayKickUpAnim(A) )
		{
			ServerDoSpecialMove(SM_WeaponPickup, TRUE, None, class'GSM_Pickup'.static.PackSpecialMoveFlags(ePT_KickUp));
		}
		else if( IsPickupAHeavyWeapon(A) )
		{
			ServerDoSpecialMove(SM_WeaponPickup, TRUE, None, class'GSM_Pickup'.static.PackSpecialMoveFlags(ePT_PickupHeavyWeapon));
		}
		else
		{
			ServerDoSpecialMove(SM_WeaponPickup, TRUE, None, class'GSM_Pickup'.static.PackSpecialMoveFlags(ePT_Pickup));
		}
	}
}

/**
 * grab a pickup from the ground.
 * @NetMode: Server only
 */
final function GrabPickup(Actor Pickup)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;

	DP = GearDroppedPickup(Pickup);
	if( DP != None )
	{
		DP.GiveTo(Self);
		return;
	}

	PF = GearPickupFactory(Pickup);
	if( PF != None )
	{
		PF.GiveTo(Self);
		return;
	}
}

final function ClaimPickup(Actor Pickup)
{
	local GearPickupFactory	PF;
	local GearDroppedPickup	DP;

	DP = GearDroppedPickup(Pickup);
	if( DP != None )
	{
		DP.ClaimPickUp(Self);
		return;
	}

	PF = GearPickupFactory(Pickup);
	if( PF != None )
	{
		PF.ClaimPickUp(Self);
		return;
	}
}

final function PlayPickupSound()
{
	if (AmmoPickupSound != None)
	{
		PlaySound( AmmoPickupSound );
	}
}

event PhysicsVolumeChange(PhysicsVolume NewVolume)
{
	if (WalkVolume(NewVolume) != None)
	{
		if (WalkVolume(NewVolume).bActive && IsDoingSpecialMove(SM_RoadieRun))
		{
			EndSpecialMove(SM_RoadieRun);
		}
	}
	Super.PhysicsVolumeChange(NewVolume);
}

/**
 * Cause this pawn to speak the specified line.
 *
 * @param Addressee Who the line is addressing, used by headtracking.  Ok if it's none.
 * @param Audio The Audio data
 * @param DebugText Poor man's subtitle, useful for debugging.
 * @param DelaySec Time in seconds to delay before actually playing the line.
 * @param Priority Used to determine when to interrupt.
 * @param IntCondition Used to determine when to interrupt.
 * @param bNoHeadTrack True to suppress headtracking for this line
 * @param BroadcastFilter controls who gets to hear this line in network games (see ESpeakLineBroadcastFilter for values)
 * @param bSuppressSubtitle TRUE means this line will have no subtitle, regardless.	 FALSE means normal subtitle rules apply.
 * @param InExtraHeadTrackTime Extra time to keep looking at addressee after line is finished (only valid when bNoHeadTrack==false)
 */
simulated native final function bool SpeakLine(GearPawn Addressee, SoundCue Audio, String DebugText, float DelaySec, optional ESpeechPriority Priority, optional ESpeechInterruptCondition IntCondition, optional bool bNoHeadTrack, optional int BroadcastFilter, optional bool bSuppressSubtitle, optional float InExtraHeadTrackTime, optional bool bClientSide);

simulated private native final function bool ShouldSuppressSubtitlesForQueuedSpeakLine(bool bVersusMulti);

/** Return true if speech from this pawn should be rejected based on the given filter. */
simulated native private final function bool ShouldFilterOutSpeech(ESpeakLineBroadcastFilter Filter, GearPawn Addressee);


function OnInterruptSpeech(SeqAct_InterruptSpeech Action)
{
	if (CurrentlySpeakingLine != None)
	{
		CurrentlySpeakingLine.FadeOut(0.2f, 0.f);
		SpeakLineFinished();
	}
}

simulated native private final function PlayQueuedSpeakLine();


/**
 * Control headtracking for this pawn.
 *
 * @param ActorToTrack Who you want to look at, or None to look at nothing
 * @param LookatBoneName Bone to lookat on target, if it has bones that is
 * @param bOverride if FALSE, will only set new lookat actor if currently None
 * @Return TRUE if tracking new object, FALSE if request ignored
 */
final simulated event bool SetHeadTrackActor(Actor ActorToTrack, optional Name LookatBoneName, optional bool bOverride)
{
	Local controller ControllerActor;

	if( HeadControl == None )
	{
		return FALSE;
	}

	if (ActorToTrack != None)
	{
		ControllerActor = Controller(ActorToTrack);
		if(ControllerActor != none && ControllerActor.Pawn != none)
		{
			ActorToTrack = ControllerActor.Pawn;
		}
		//MessagePlayer(GetFuncName()@ActorToTrack@LookatBoneName@bOverride);
		// Don't view self, it causes bad jiterring!
		if( ActorToTrack != Self && ((HeadLookAtActor == None) || (bOverride)) )
		{
			HeadTrackTime = (HeadTrackDuration - HeadTrackTime);

			HeadLookAtActor = ActorToTrack;
			HeadControl.DesiredTargetLocation = GetHeadLookTargetLocation();
			HeadControl.SetSkelControlActive(TRUE);
			HeadLookAtBoneName = LookatBoneName;
			//`log(self@"now tracking"@ActorToTrack);
			return TRUE;
		}
		// else do nothing -- current headtrack stays in effect
	}
	else
	{
		// stop tracking
		HeadLookAtActor = None;
		HeadControl.SetSkelControlActive(FALSE);
		HeadLookAtBoneName = '';
		//`log(self@"now tracking"@ActorToTrack);
		return TRUE;
	}

	return FALSE;
}

/**
 * Called when this pawn's spoken line is finished (naturally or interrupted), to give us
 * an opportunity to clean up.
 */
final simulated event SpeakLineFinished()
{
	local SpeakLineParamStruct EmptyLine;

	bSpeaking = FALSE;

	if(CurrentlySpeakingLine != None)
	{
		GearGRI(WorldInfo.GRI).SpeechManager.NotifyDialogueFinish(self, CurrentlySpeakingLine.SoundCue);

		CurrentlySpeakingLine = None;
	}

	// this function can be called from other ways than SetTimer as well, so
	// cancel the timer to handle those cases
	ClearTimer('SpeakLineFinished');

	// stop looking at addressee
	if ( (HeadLookAtActor != None) && (HeadLookAtActor == CurrentSpeakLineParams.Addressee) )
	{
		//MessagePlayer("Disabling headtrack in "$CurrentSpeakLineParams.ExtraHeadTrackTime);
		if(CurrentSpeakLineParams.ExtraHeadTrackTime > 0.f)
		{
			SetTimer( CurrentSpeakLineParams.ExtraHeadTrackTime,false,nameof(DisableHeadTrack) );
		}
		else
		{
			DisableHeadTrack();
		}

	}

	// clear text so the line doesn't get replicated to newly joined clients after it's over
	ReplicatedSpeakLineParams = EmptyLine;
}

final simulated function DisableHeadTrack()
{
	SetHeadTrackActor(None);
}


simulated function DoSpawnABloodPool()
{
	bSpawnABloodPool++;
	if( WorldInfo.NetMode != NM_DedicatedServer )
	{
		SpawnBloodPool();
	}
}


simulated function SpawnBloodPool()
{
	if( WorldInfo.GRI.ShouldShowGore() && bSpawnBloodTrailDecals )
	{
		GearPawnFX.SpawnABloodTrail_BloodPool();
	}
}


final simulated function AddToLocalDBNOList()
{
	local PlayerController PC;

	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		if ( (PC.Pawn != None) && (PC.Pawn != self) )
		{
			if ( (PC.PlayerReplicationInfo != None) && (PC.PlayerReplicationInfo.Team.TeamIndex == GetTeamNum()) )
			{
				GearPC(PC).AddDBNOTeammate( self );
			}
		}
	}
}

final simulated function EndGameCondition( bool bGib )
{
	local GearGRI GRI;
	local GearPC PC;

	// Cause game over if Dom dies
	GRI = GearGRI(WorldInfo.GRI);
	if( GRI != None )
	{
		if ( AIController(Controller) != None )
		{
			foreach WorldInfo.AllControllers( class'GearPC', PC )
			{
				if (!PC.PlayingDevLevel())
				{
					// play the anya sound
					// JF: ProcessGameOver will play it
					//if (Class == class'GearPawn_COGDom')
					//{
					//	GearPRI(PC.PlayerReplicationInfo).ClientPlayCoopSplitDeathSound( false );
					//}
					// one last failsafe so if someone does somehow die in the player's squad, but they weren't supposed to don't fail the game.
					if (Class == class'GearPawn_COGDom' || class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo).default.bAllowAICOGDeathWhenDBNO)
					{
						`log("Ending game from AI death:"@`showvar(self));
						PC.SetTimer(2.0, false, nameof(PC.ProcessGameOver));
					}
				}
			}
		}
	}
}


function DoRevival(GearPawn Reviver, optional bool bRevivedByKantus);

/**
 * Kismet interface for toggling the revivability of this pawn.  (Primarily used for tutorial at the moment.)
 */
function OnToggleRevive(SeqAct_ToggleRevive Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		bCanRecoverFromDBNO = TRUE;
	}
	else
	if (Action.InputLinks[1].bHasImpulse)
	{
		bCanRecoverFromDBNO = FALSE;
	}
	else
	{
		bCanRecoverFromDBNO = !bCanRecoverFromDBNO;
	}
}
/** Knock a player down and apply the specified impulses. */
reliable server function Knockdown(vector LinearVelocityImpulse, vector AngularVelocityImpulse)
{
	local PhysicsImpulseInfo NewKnockdownImpulse;

	if (Role < ROLE_Authority)
	{
`if(`notdefined(FINAL_RELEASE))
		`warn(GetFuncName()@"only allowed on server");
		ScriptTrace();
`endif
		return;
	}

	// only one knockdown allowed at a time
	if (Physics == PHYS_RigidBody || bPlayedDeath)
	{
		return;
	}
	// if holding a meatshield
	else if (IsAKidnapper())
	{
		ServerEndSpecialMove(SpecialMove);
		return;
	}
	// if carrying a heavy weapon, drop it
	else if (IsCarryingAHeavyWeapon())
	{
		ThrowActiveWeapon();
	}
	// if carrying a shield, drop it
	else if (IsCarryingShield())
	{
		DropShield();
	}

	StopFiring();

	`RecordStat(STATS_LEVEL1,'PlayerKnockedDown',Controller,"Scripted");

	// set the knockdown information for use in ApplyKnockdownImpulse()
	//@note - have to use local struct and assign, otherwise bNetDirty won't be properly flagged on the struct
	NewKnockdownImpulse.LinearVelocity = LinearVelocityImpulse;
	NewKnockdownImpulse.AngularVelocity = AngularVelocityImpulse;
	KnockdownImpulse = NewKnockdownImpulse;
	// transition to the KnockedDown state
	GotoState('KnockedDown');
}

simulated function float GetKnockDownFailSafeTimeout()
{
	return 6.f;
}
/** Applies the contents of KnockdownImpulse.  Clients shouldn't call this function directly, only from repnotify. */
simulated protected function ApplyKnockdownImpulse()
{
	`log("!!!"@`showvar(KnockdownImpulse.LinearVelocity)@`showvar(Location)@`showvar(Mesh.GetPosition()));
	// Record when knockdown happened
	KnockDownStartTime = WorldInfo.TimeSeconds;
	KnockdownStartingPosition = Location;
	// bail out of cover
	if (IsInCover())
	{
		BreakFromCover();
	}
	if (Physics != PHYS_RigidBody || Mesh != CollisionComponent)
	{
		// first send them to ragdoll
		PlayFallDown();
	}
	if (Physics == PHYS_RigidBody)
	{
		// and apply the forces additively
		if (!IsZero(KnockdownImpulse.AngularVelocity))
		{
			Mesh.SetRBAngularVelocity(KnockdownImpulse.AngularVelocity,TRUE);
		}
		Mesh.SetRBLinearVelocity(KnockdownImpulse.LinearVelocity,TRUE);
		if (Role == ROLE_Authority)
		{
			SetTimer(GetKnockDownFailSafeTimeout(),FALSE,nameof(KnockdownFailsafe));
			bCheckKnockdownFall = FALSE;
			SetTimer(0.5f,FALSE,nameof(EnableKnockdownFallChecking));
		}
	}
	else
	{
		`warn(self@"failed to transition to RigidBody for"@GetFuncName());
	}
}

final event KnockdownFailsafe()
{
	// don't kill off dudes if they are in unlimited health as we use that for testing and you can get into places where multiple maulers will keep you knocked down for quite a while
	if( !bUnlimitedHealth )
	{
       `warn( "KnockdownFailsafe is triggering!!!" @ self @ Controller );
		Died(Controller,class'GDT_ScriptedRagdoll',Location);
	}
}

final function EnableKnockdownFallChecking()
{
	bCheckKnockdownFall = TRUE;
}

/** Util to update motor strength of motors after being knocked down */
native function TickKnockDownMotorStrength(float DeltaSeconds);

/**
 * Knocked down state used on the server for non-lethal ragdoll scenarios.
 * Server only.
 */
state KnockedDown
{
	function BeginState(Name PrevStatName)
	{
		if (MyGearAI != None)
		{
			MyGearAI.NotifyKnockDownStart();
		}
	}

	simulated function Tick(float DeltaTime)
	{
		// if using motorised-anim during knockdown, update motor strengths
		if( bPlayMotorAnimOnKnockDown )
		{
			TickKnockDownMotorStrength(DeltaTime);
		}

		FadeSkinEffects(DeltaTime);
	}

Begin:
	// apply the impulse
	ApplyKnockdownImpulse();
	// Set a max timer so Pawn doesn't stay in this state forever.
	SetTimer( 5.f, FALSE, nameof(EndKnockedDownState) );
	do
	{
		// delay
		Sleep(0.35f);

		// we can be doing a transition, and in that case the weapon will show up a bit later.
		if( Role == ROLE_Authority && IsCarryingAHeavyWeapon() )
		{
			ThrowActiveWeapon();
		}
		// until we're snapped out of physics, stopped moving, or the physics has gone to sleep
	} until (Physics != PHYS_RigidBody || VSize(Velocity) < 8.f || !Mesh.RigidBodyIsAwake());

	EndKnockedDownState();
}

simulated function float GetKnockdownZThreshold()
{
	return 256.f;
}

function EndKnockedDownState()
{
	ClearTimer(nameof(EndKnockedDownState));
	ClearTimer(nameof(CheckGetUp));
	ClearTimer(nameof(KnockdownFailsafe));
	ClearTimer(nameof(EnableKnockdownFallChecking));

	`log(GetFuncName()@Abs(KnockdownStartingPosition.Z - Location.Z));
	// if we've fallen a significant distance then assume we're dead
	if (Abs(KnockdownStartingPosition.Z - Location.Z) > GetKnockdownZThreshold())
	{
		KnockdownFailsafe();
	}

	KnockdownImpulse.LinearVelocity = vect(0,0,0);
	KnockdownImpulse.AngularVelocity = vect(0,0,0);

	// if we're still alive at this point
	if( Health > 0 && Physics == PHYS_RigidBody )
	{
		ServerDoSpecialMove(SM_RecoverFromRagdoll, TRUE);
		GotoState('');
	}
}

function NotifyWaitingForRevive()
{
	local GearAI AI;
	foreach WorldInfo.AllControllers(class'GearAI', AI)
	{
		if( AI != Controller )
		{
			AI.NotifyWaitingForRevive(None, Controller, Self);
		}
	}
}

/**
 * Raises player's hand while DBNO, called on server/clients.
 */
simulated function RaiseHandWhileCrawling()
{
	if (DBNOGuardBlendNode != None)
	{
		DBNOGuardBlendNode.SetBlendTarget(1.f,0.5f);
		SetTimer( 1.f,FALSE,nameof(LowerHandWhileCrawling) );
	}
	// notify the server if owning client
	if (Role != ROLE_Authority && PlayerController(Controller) != None)
	{
		ServerRaiseHandWhileCrawling();
	}
	else
	{
		bRaiseHandWhileCrawling = TRUE;
	}
	if (GearGame(WorldInfo.Game) != None)
	{
		NeedsRevivedReminder(TRUE);
	}
}

/**
 * Server's version of hand raising, sets the flag to be replicated to non-owning clients for simulation.
 */
unreliable server function ServerRaiseHandWhileCrawling()
{
	bRaiseHandWhileCrawling = TRUE;
	RaiseHandWhileCrawling();
}

function LowerHandWhileCrawling()
{
	local GearPC PC;

	bRaiseHandWhileCrawling = FALSE;

	if (DBNOGuardBlendNode != None)
	{
		DBNOGuardBlendNode.SetBlendTarget(0.f,0.5f);
	}

	PC = GearPC(Controller);

	if( PC != None )
	{
		PC.SetTimer( 0.5f, FALSE, nameof(PC.HandLoweredWhileCrawling) );
	}
}

function AlterCrawlSpeed(float Delta)
{
	local bool bIncreaseDBNO;
	bIncreaseDBNO = FALSE;
	//`log(`location@Delta@`showvar(CrawlSpeedModifier)@`showvar(TimeOfDBNO));
	// if this is a speed increase then delay the DBNO bleedout
	if (!GearGRI(WorldInfo.GRI).bGameIsExecutionRules && Delta > 0.f && DBNOTimeExtensionCnt < 7 && FRand() > (DBNOTimeExtensionCnt/7.f))
	{
		bIncreaseDBNO = TRUE;
		DBNOTimeExtensionCnt = DBNOTimeExtensionCnt + 1;
		TimeOfDBNO += 1.f;
	}
	CrawlSpeedModifier = FClamp(CrawlSpeedModifier + Delta,0.f,0.7f);
	if (DBNOCrawlNode != None)
	{
		DBNOCrawlNode.Rate = (1.f + CrawlSpeedModifier);
	}
	if (Role != ROLE_Authority)
	{
		ServerSetCrawlSpeed(CrawlSpeedModifier,bIncreaseDBNO);
	}
}

unreliable server function ServerSetCrawlSpeed(float NewSpeed, bool bIncreaseDBNO)
{
	//`log(`location@NewSpeed@`showVar(CrawlSpeedModifier)@`showvar(TimeOfDBNO));
	// if this is a speed increase then delay the DBNO bleedout
	if (bIncreaseDBNO)
	{
		TimeOfDBNO += 1.f;
	}
	CrawlSpeedModifier = NewSpeed;
	ReplicatedEvent('CrawlSpeedModifier');
}

/** Get a Pawn to enter DBNO state. */
final function EnterDBNO(Controller InControllerWhoPutMeDBNO, class<GearDamageType> InDamageTypeThatPutMeDBNO)
{
	`LogSM("");

	// if we don't support DBNO, pretend this didn't happen
	if (!bCanDBNO)
	{
		Health = Max(Health, 1);
		return;
	}

	// Can't send someone to DBNO state if he's already doing the special move or if he's a hostage.
	if (IsDoingSpecialMove(SM_DBNO) || IsAHostage())
	{
		`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "Cannot be sent to DBNO state. SpecialMove:" @ SpecialMove);
		ScriptTrace();
		return;
	}

	// Save controller for future use (if we bleed out to death)
	ControllerWhoPutMeDBNO = InControllerWhoPutMeDBNO;
	DamageTypeThatPutMeDBNO	= InDamageTypeThatPutMeDBNO;

	// If we should wait for current special move to finish before enterring DBNO state
	if (IsInState('KnockedDown') || IsDoingSpecialMove(SM_MidLvlJumpOver) || IsDoingSpecialMove(SM_RecoverFromRagdoll))
	{
		// This will call EnterDBNO again after the current special move is done.
		bDelayedDBNO = TRUE;
	}
	// Or enter now
	else
	{
		GotoState('DBNO');
	}
}

// called at the end of fights and such whenever everyone is being auto-revived
function ReviveFromAutoRevive(GearPawn GP)
{
	if(bAllowAutoRevive)
	{
		DoRevival(GP);
	}
}

/** Kismet action to send someone into DBNO state */
function OnForceDBNO( SeqAct_ForceDBNO Action )
{
	//@fixme, need to figure out the instigator for this, and a damagetype?
	EnterDBNO(None, class'GearDamageType');
	bAllowAutoRevive=Action.bAllowAutoRevive;
}

/** Returns TRUE if Pawn is DBNO. */
simulated final native function bool IsDBNO() const;

final function bool ShouldTakeDamageWhenDBNO()
{
	// if we're COG AI, don't take damage while DBNO unless difficulty settings are high enough
	if (GetTeamNum() == 0 && !WorldInfo.GRI.IsMultiplayerGame())
	{
		if (GearAI(Controller) != None)
		{
			return false;
		}
		else if (GearPC(Controller) != None)
		{
			return GearPRI(PlayerReplicationInfo).Difficulty.default.bHumanCOGCanBeExecuted;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}

}

function DoFatalDeath(Controller InKillerController, class<GearDamageType> InDamageType, Vector InHitLocation, Vector InMomentum);

/**
 * DBNO state, only called on server.
 */
state DBNO
{
	// ignored while DBNO
	reliable server function Knockdown(vector LinearVelocityImpulse, vector AngularVelocityImpulse) {}
	simulated protected function ApplyKnockdownImpulse() {}
	function DoStumbleFromMeleeSpecialMove() {}

	event BeginState(Name PrevStateName)
	{
		local array<SequenceEvent> EngageEvents;
		local SeqEvt_Engage EngageEvent;

		`LogSM("");

		// cancel engage trigger if necessary
		if (EngageTrigger != None && (SpecialMove == SM_Engage_Start || SpecialMove == SM_Engage_Loop || SpecialMove == SM_Engage_Idle))
		{
			EngageTrigger.EngagedPawn = None;

			// fire proper kismet event
			if (EngageTrigger.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents))
			{
				EngageEvent = SeqEvt_Engage(EngageEvents[0]);
				if (EngageEvent != None && EngageEvent.bEnabled)
				{
					// don't check fov since the camera could be anywhere
					EngageEvent.bCheckInteractFOV = false;
					// fire the event
					EngageTrigger.TriggerEventClass(class'SeqEvt_Engage', self, eENGAGEOUT_Stopped);
				}
			}
		}

		// server side only stuff
		bDelayedDBNO = FALSE;

		// do this before health reaches zero
		KickPawnOffTurret();

		// Set health to zero so we can count down to death amount
		Health = 0;
		// keeps track of how many times this pawn has gone DBNO
		DownCount++;
		// keep track of DBNO time to give a second of invulnerability
		TimeStampEnteredRevivingState = WorldInfo.TimeSeconds;

		// track the DBNO for the victim
		if( ControllerWhoPutMeDBNO != None )
		{
			GearPRI(PlayerReplicationInfo).ScoreKnockdown(GearPRI(ControllerWhoPutMeDBNO.PlayerReplicationInfo), DamageTypeThatPutMeDBNO);
		}

		GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Down;

		// notify the damage type handler for any DBNO fx
		if( HitDamageType != None )
		{
			class<GearDamageType>(HitDamageType).static.HandleDBNOPawn(self);
		}

		// notify the AI that we need reviving
		SetTimer( 0.5f, FALSE, nameof(NotifyWaitingForRevive) );

		// "so-and-so went down!" guds
		// note that in multi, we don't do this if you're the only one left, since there's nobody to revive you
		if ( (GearGameMP_Base(WorldInfo.Game) == None) || (GearGameMP_Base(WorldInfo.Game).GetNumPlayersAlive(GetTeamNum()) > 1) )
		{
			if (WentDownGUDSEvent != GUDEvent_None)
			{
				GearGame(WorldInfo.Game).TriggerGUDEvent(WentDownGUDSEvent, self, None);
			}
			else
			{
				GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GenericWentDown, self, None);
			}

			// no reminders during splits, since you might not be able to get there
			if (!GearGame(WorldInfo.Game).bInCoopSplit)
			{
				NeedsRevivedReminder();
				NeedsRevivedReminderCount = 0;
			}
		}

		// if carrying a heavy weapon, drop it
		if( IsCarryingAHeavyWeapon() )
		{
			ThrowActiveWeapon();
			// and select another weapon
			GearInventoryManager(InvManager).SetWeaponFromSlot(EASlot_LeftShoulder);
		}

		// if carrying a shield, drop it
		if( IsCarryingShield() )
		{
			DropShield();
		}

		StopFiring();

		// don't let AIs think they can mantle when they're DBNO
		bCanMantle  = FALSE;
		bCanClimbUp = FALSE;
		// don't let AIs think they can climb ladders when they're dbno
		bCanClimbLadders = FALSE;

		// Send Pawn into DBNO special move.
		ServerDoSpecialMove(SM_DBNO, TRUE);

		// notify the gametype
		GearGame(WorldInfo.Game).NotifyDBNO(ControllerWhoPutMeDBNO, Controller, Self);
	}

	event EndState(Name NextStateName)
	{
		local GearAI OtherAI;

		`LogSM("");

		bIsBleedingOut = FALSE;

		if( Role == ROLE_Authority )
		{
			ClearTimer('NeedsRevivedReminder');
		}

		// smolder a little longer and go out
		if( bBledOut && (OnFireState == POF_Smoldering) )
		{
			IgnitePawn(POF_Smoldering, 8.f);
		}

		ClearTimer( 'NotifyWaitingForRevive' );

		foreach WorldInfo.AllControllers(class'GearAI', OtherAI)
		{
			if( OtherAI != MyGearAI )
			{
				OtherAI.NotifyLeftDBNO(self);
			}
		}

		// restore mantle setting since we turned it off when entering dbno
		bCanMantle	= default.bCanMantle;
		bCanClimbUp	= default.bCanClimbUp;

		// restore ladder setting since we turned it off when entering dbno
		bCanClimbLadders = default.bCanClimbLadders;
	}

	/**
	 * If you are in state reviving and you take damage then you are FATALLY KILLED!
	 */
	function TakeDamage(int Damage, Controller InstigatedBy, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		local class<GearDamageType> GearDamageType;
		//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ DamageType);


		// only count "fatal" damage if they have been dead for a second
		// we do this as there are some order issues with shotgun bullets and other bullets
		// also we want there to be a clear separation between taking someone out.	and fatally killing them
		if( Role < ROLE_Authority || WorldInfo.TimeSeconds - TimeStampEnteredRevivingState < 1.0f )
		{
			return;
		}

		`LogSM("");

		GearDamageType = class<GearDamageType>(DamageType);

		// if passed an invalid damagetype then use the default
		if( GearDamageType == None )
		{
			GearDamageType = class'GearDamageType';
		}

		// if this damage type isn't lethal then ignore
		if( !GearDamageType.default.bLethal )
		{
			return;
		}

		// if we're COG AI, don't take damage while DBNO unless difficulty settings are high enough
		if ( !ShouldTakeDamageWhenDBNO() )
		{
			return;
		}

		// if the hit location is invalid then adjust to base location
		if( IsZero(HitLocation) )
		{
			HitLocation = GetBaseTargetLocation();
		}

		// ensure a valid momentum
		if( VSize(Momentum) > 4.f )
		{
			Momentum = Normal(Momentum);
		}

		// first allow the damage type a first-pass modifier
		GearDamageType.static.ModifyDamage(self, InstigatedBy, Damage, HitInfo, HitLocation, Momentum);

		// if the damage wasn't eliminated
		if( Damage > 0 )
		{
			// notify the game info of the damage, possibly scaling the values
			WorldInfo.Game.ReduceDamage(Damage, Self, InstigatedBy, HitLocation, Momentum, DamageType);
		}

		if( Damage > 0 )
		{
			if (InstigatedBy == None)
			{
				InstigatedBy = ControllerWhoPutMeDBNO;
			}

			// Update health
			Health				-= Damage;
			LastTookDamageTime	 = WorldInfo.TimeSeconds;

			// play any associated fx/sounds
			DoDamageEffects(Damage, InstigatedBy.Pawn, HitLocation, DamageType, Momentum, HitInfo);

			// Don't kill right away if we're a hostage
			if( Health <= DBNODeathThreshold )
			{
				DoFatalDeath(InstigatedBy, GearDamageType, HitLocation, Momentum);
			}
		}
	}

	function DoFatalDeath(Controller InKillerController, class<GearDamageType> InDamageType, Vector InHitLocation, Vector InMomentum)
	{
		local GearPRI KillerPRI;

		`LogSM("");

		TearOffMomentum = InMomentum;
		Health			= -1;

		// grab the last person we know of who hurt us
		if( InKillerController != None )
		{
			KillerPRI = GearPRI(InKillerController.PlayerReplicationInfo);
			if( KillerPRI != None )
			{
				KillerPRI.ScoreExecution(GearPRI(PlayerReplicationInfo), InDamageType, GDT_NORMAL);
			}
		}
		// perform the appropriate death
		Died(InKillerController, InDamageType, InHitLocation);
	}

	function DoRevival(GearPawn Reviver, optional bool bRevivedByKantus)
	{
		local GearPC GPC;

		`LogSM("");

		if( !bCanRecoverFromDBNO )
		{
			`Warn(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ "bCanRecoverFromDBNO is FALSE!! Aborted. Reviver:" @ Reviver);
			ScriptTrace();
			return;
		}

		// reset to default
		bAllowAutoRevive = default.bAllowAutoRevive;

		// give the revive credit if not an auto revive
		if( Reviver != self )
		{
			GearPRI(Reviver.PlayerReplicationInfo).ScoreRevive(GearPRI(PlayerReplicationInfo));
		}
		else
		{
			// otherwise just reset the damage lists
			GearPRI(PlayerReplicationInfo).PlayerDamageList.Length = 0;
		}

		GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Alive;

		// Clear this variable since we only want this information if the player died from the DBNO
		GearPRI(PlayerReplicationInfo).LastToDBNOMePRI = None;

		// return with reduced health
		if(bRevivedByKantus)
		{
			// if we've been revived by the kantus, go to 15% health, until after the revive is done
			Health = DefaultHealth * 0.15f;
		}
		else
		{
			if (HealthRechargeDelay > 0.f)
			{
				Health = DefaultHealth * 0.8f;
			}
			else
			{
				Health = DefaultHealth;
			}
		}

		// notify kismet of Revival
		TriggerEventClass( class'SeqEvt_EnteredRevivalState', Reviver, 1 );

		GPC = GearPC(Controller);
		if( GPC != None )
		{
			GPC.GoToPlayerWalking();
			if( (Role == ROLE_Authority) && !GPC.IsLocalPlayerController() )
			{
				GPC.bRequestClientVerifyState = TRUE;
			}

			// Have the HUD draw who revived them
			if( WorldInfo.GRI.IsMultiplayerGame() )
			{
				GPC.ClientStartDrawingRevival( Reviver );
			}
		}
		else
		{
			if( MyGearAI != None )
			{
				MyGearAI.PawnWasRevived();
			}
		}

		if(bRevivedByKantus)
		{
			KantusReviver = GearPawn_LocustKantusBase(Reviver);
		}
		// Start recovery special move
		ServerDoSpecialMove(SM_RecoverFromDBNO, TRUE, None, class'GSM_RecoverFromDBNO'.static.PackSpecialMoveFlags(bRevivedByKantus));

		// transition back to the default state
		GotoState('');

		// start the recharge process
		if (!bRevivedByKantus && HealthRechargeDelay > 0.f)
		{
			EnableHealthRecharge();
		}
	}


	function bool ShouldBleedOut()
	{
		// if a hostage or cheating
		if (IsAHostage() || GearGame(WorldInfo.Game).bDBNOForeverCheat)
		{
			// no bleed out
			return FALSE;
		}
		// if AI controlled (and not MP)
		if (!IsHumanControlled() && GetTeamNum() == 0 && !WorldInfo.GRI.IsMultiplayerGame() && GearAI_TDM(Controller) == None)
		{
			// bleed out based on difficulty
			return GearPRI(PlayerReplicationInfo).Difficulty.default.bAllowAICOGDeathWhenDBNO;
		}
		// otherwise bleed out
		return TRUE;
	}

Begin:

	// handle stopping any current speech, if any
	if (CurrentlySpeakingLine != None && CurrentSpeakLineParams.Priority == Speech_Scripted)
	{
		`SpeechLog(self@"DBNO'd, aborting speech"@CurrentlySpeakingLine.SoundCue);
		CurrentlySpeakingLine.FadeOut(0.2f, 0.f);
		SpeakLineFinished();
	}

	// if we're the server and not a hostage
	if (Role == ROLE_Authority)
	{
		if (ShouldBleedOut())
		{
			bIsBleedingOut = TRUE;
			// start the bleed out cycle
			while( TRUE )
			{
				if( IsAHostage() )
				{
					break;
				}

				// check that we didn't become unable to bleed out (e.g. due to Campaign game becoming co-op)
				if (!WorldInfo.GRI.IsMultiplayerGame() && !ShouldBleedOut())
				{
					Goto('Begin');
				}

				if ( (TimeOfDBNO > 0) && ((WorldInfo.TimeSeconds - TimeOfDBNO) > GearGRI(WorldInfo.GRI).InitialRevivalTime) )
				{
					// if we should auto revive
					if( GearGame(WorldInfo.Game).AutoReviveOnBleedOut(self) )
					{
						// then do so
						DoRevival(self);
					}
					else
					{
						// otherwise go ahead and die
						bBledOut = TRUE;
						DoFatalDeath(ControllerWhoPutMeDBNO, class'GDT_BledOut', Location, vect(0,0,1));
					}
				}
				Sleep(0.5f);
			}
		}
		else if (!WorldInfo.GRI.IsMultiplayerGame())
		{
			// we need to check if we become able to bleed out in SP since that state may change
			// if a player leaves the game
			while (!IsAHostage())
			{
				if (ShouldBleedOut())
				{
					Goto('Begin');
				}
				Sleep(1.0);
			}
		}
	}
}


/** Called on a timer while this pawn is in the "down but not out" state */
final function NeedsRevivedReminder(optional bool bManuallyCalled)
{
	if (NeedsRevivedGUDSEvent != GUDEvent_None)
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(NeedsRevivedGUDSEvent, self, None);
	}
	else
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_GenericNeedsRevived, self, None);
	}
	if (++NeedsRevivedReminderCount < 5 && !bManuallyCalled)
	{
		SetTimer( RandRange(5.f,10.f),FALSE,nameof(NeedsRevivedReminder) );
	}
}

/** check to see if I'm near a team mate*/
final function bool IsNearATeammate(optional float DistThresh=384.0f)
{
	local GearPawn GP;

	foreach WorldInfo.AllPawns(class'GearPawn', GP, Location, DistThresh)
	{
		if (GP != None && GP.IsAliveAndWell() && WorldInfo.GRI.OnSameTeam(GP, self))
		{
			return TRUE;
		}
	}

	return false;
}

/** Helper function to fade out an audio component if it is valid */
simulated final function FadeOutAudioComponent(AudioComponent AC, float Duration, optional float Volume)
{
	if (AC != None)
	{
		AC.FadeOut(Duration,Volume);
	}
}

/************************************************************************************
 * Meat Shield
 ***********************************************************************************/


/** Returns TRUE if this Pawn is a hostage. */
simulated final native function bool IsAHostage() const;

/** Returns TRUE if this Pawn is a kidnapper. */
simulated final native function bool IsAKidnapper() const;

/** Implemented in Kidnapped state */
function FreeHostage(GearPawn Kidnapper);

/**
 * Kidnapped state, only called on server.
 * State used for Hostages set from GSM_Hostage.
 */
state Kidnapped
{
	reliable server function Knockdown(vector LinearVelocityImpulse, vector AngularVelocityImpulse) {}
	simulated protected function ApplyKnockdownImpulse() {}
	function DoStumbleFromMeleeSpecialMove() {}

	event BeginState(Name PrevStateName)
	{
		`LogSM("");

		// Giving hostage a second of immortality
		TimeStampEnteredRevivingState = WorldInfo.TimeSeconds;

		// Clear this variable since we only want this information if the player died from the DBNO
		GearPRI(PlayerReplicationInfo).LastToDBNOMePRI = None;
	}

	/**
	 * If you are in state reviving and you take damage then you are FATALLY KILLED!
	 */
	function TakeDamage(int Damage, Controller InstigatedBy, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional Actor DamageCauser)
	{
		local class<GearDamageType> GearDamageType;
		local bool bIsMeatFlag;

		//`log(WorldInfo.TimeSeconds @ Self @ GetFuncName() @ DamageType);

		// only count "fatal" damage if they have been dead for a second
		// we do this as there are some order issues with shotgun bullets and other bullets
		// also we want there to be a clear separation between taking someone out.	and fatally killing them
		if( Role < ROLE_Authority || WorldInfo.TimeSeconds - TimeStampEnteredRevivingState < 1.0f )
		{
			return;
		}

		`LogSM("");

		GearDamageType = class<GearDamageType>(DamageType);

		// if passed an invalid damagetype then use the default
		if( GearDamageType == None )
		{
			GearDamageType = class'GearDamageType';
		}

		// if this damage type isn't lethal then ignore
		if( !GearDamageType.default.bLethal )
		{
			return;
		}

		// if the hit location is invalid then adjust to base location
		if( IsZero(HitLocation) )
		{
			HitLocation = GetBaseTargetLocation();
		}

		// ensure a valid momentum
		if( VSize(Momentum) > 4.f )
		{
			Momentum = Normal(Momentum);
		}

		// first allow the damage type a first-pass modifier
		GearDamageType.static.ModifyDamage(self, InstigatedBy, Damage, HitInfo, HitLocation, Momentum);

		// if the damage wasn't eliminated
		if( Damage > 0 )
		{
			// notify the game info of the damage, possibly scaling the values
			WorldInfo.Game.ReduceDamage(Damage, Self, InstigatedBy, HitLocation, Momentum, DamageType);
		}

		if( Damage > 0 )
		{
			HostageHealth		-= Damage;
			LastTookDamageTime	 = WorldInfo.TimeSeconds;

			// if this is a meatflag then pass a portion of the damage to the kidnapper to prevent "god mode"
			if (IsA('GearPawn_MeatFlagBase'))
			{
				bIsMeatFlag = TRUE;
				// if it's explosive then don't pass through since this will already cause a break
				if (ClassIsChildOf(DamageType,class'GDT_Explosive'))
				{
					return;
				}
				InteractionPawn.TakeDamage(Damage * (1.f - MeatflagProtectionPct),InstigatedBy,HitLocation,Momentum,DamageType);
			}

			// Update Hostage health effects
			UpdateHostageHealth();

			// play any associated fx/sounds
			DoDamageEffects(Damage, InstigatedBy.Pawn, HitLocation, DamageType, Momentum, HitInfo);

			// Explosive damage will always kill the hostage
			// Conventional weapons cannot kill MeatFlag (check for CanBeSpecialMeleeAttacked)
			if (!bIsMeatFlag && (ClassIsChildOf(DamageType, class'GDT_Explosive') || HostageHealth <= 0))
			{
				// award bonus points to the instigator if this is the meatflag
				if (GearGameCTM_Base(WorldInfo.Game) != None && GearPRI(InstigatedBy.PlayerReplicationInfo) != None && IsA('GearPawn_MeatFlagBase'))
				{
					GearPRI(InstigatedBy.PlayerReplicationInfo).ScoreGameSpecific2('FlagBreak',"",50);
				}

				TearOffMomentum = Momentum;
				Health			= -1;

				if (ClassIsChildOf(DamageType, class'GDT_Explosive'))
				{
					if (ClassIsChildOf(DamageType, class'GDT_Mortar'))
					{
						// if it's a mortar then delay longer since it's far likely we'll die soon
						InteractionPawn.SetTimer(0.5f,FALSE,nameof(SavedByAMeatshieldCallback));
					}
					else
					{
						InteractionPawn.SetTimer(0.1f,FALSE,nameof(SavedByAMeatshieldCallback));
					}
				}

				// perform the appropriate death
				Died(InstigatedBy, GearDamageType, HitLocation);

				// If somehow we're still hostage after dying.... Force a release.
				if( IsAHostage() )
				{
					InteractionPawn.ServerEndSpecialMove();
				}
			}
		}
	}

	/** Free Hostage. Let him walk away. */
	function FreeHostage(GearPawn Kidnapper)
	{
		local GearPC	GPC;

		`LogSM("");

		if (!SetLocation(Location + vect(0,0,1)))
		{
			`Log("Recovering hostage stuck in a wall, teleporting to nearest nav point");
			SetLocation(class'NavigationPoint'.static.GetNearestNavToActor(self).Location);
		}

		GearPRI(PlayerReplicationInfo).PlayerStatus = WPS_Alive;

		// return with reduced health, but instantly recharging
		Health = DefaultHealth * 0.5f;
		EnableHealthRecharge();

		// notify kismet of Revival
		// @fixme laurent - probably need another event there
		TriggerEventClass(class'SeqEvt_EnteredRevivalState', Self, 1);

		GPC = GearPC(Controller);
		if( GPC != None )
		{
			GPC.GoToPlayerWalking();
			if( (Role == ROLE_Authority) && !GPC.IsLocalPlayerController() )
			{
				GPC.bRequestClientVerifyState = TRUE;
			}

			// Have the HUD draw who revived them
			if( WorldInfo.GRI.IsMultiplayerGame() )
			{
				GPC.ClientStartDrawingRevival(Self);
			}
		}
		// tell AI that it was released/revived
		else if (MyGearAI != None)
		{
			MyGearAI.PawnWasRevived();
		}

		// if pawn is on fire or smoldering, put him out
		ExtinguishPawn();

		// Start recovery special move
		ServerDoSpecialMove(SM_RecoverFromDBNO, TRUE);

		// transition back to the default state
		GotoState('');
	}
}

`if(`notdefined(FINAL_RELEASE))
exec function HurtMyHostage()
{
	if(InteractionPawn != None && InteractionPawn.IsAHostage())
	{
		InteractionPawn.HostageHealth /= 2;
		InteractionPawn.UpdateHostageHealth();
	}
}
`endif

/**
 * Reflect Hostage's health through gradual gibbing.
 */
simulated function UpdateHostageHealth()
{
	local INT					BucketNum, HealthLost, i;
	local Vector				HitLoc, Momentum;
	local RB_ConstraintInstance	ConstraintInst;

	//`log(WorldInfo.TimeSeconds @ Self @ GetStateName() $ "::" $ GetFuncName() @ "Hostage:" @ IsAHostage() @ "CanBeSpecialMeleeAttacked:" @ CanBeSpecialMeleeAttacked(InteractionPawn) );

	// This is the check for the meatflag, he cannot be gibbed!
	if( !IsAHostage() || !CanBeSpecialMeleeAttacked(InteractionPawn) )
	{
		return;
	}

	HealthLost = Max(HostageDefaultHealth - HostageHealth, 0);
	BucketNum = INT((FLOAT(HealthLost) / (HostageDefaultHealth-(HostageDefaultHealth*0.10f))) * FLOAT(HostageHealthBuckets.Length)); // 10% so we get all of the bones broken off

// 	`log("  HealthLost:" @ HealthLost @ "BucketNum:" @ BucketNum @ "HostageHealthBuckets.Length:" @ HostageHealthBuckets.Length);

	for(i=0; i<BucketNum && i<HostageHealthBuckets.Length; i++)
	{
		//`log("  HostageHealthBuckets:" @ HostageHealthBuckets[i]);
		ConstraintInst = Mesh.PhysicsAssetInstance.FindConstraintInstance(HostageHealthBuckets[i], Mesh.PhysicsAsset);
		if( ConstraintInst != None && !ConstraintInst.bTerminated )
		{
			if( !bIsGore )
			{
				CreateGoreSkeleton(GoreSkeletalMesh, GorePhysicsAsset);
				return; // first time just create the gore skeleton
			}

			//`log("    Breaking constraint for" @ HostageHealthBuckets[i]);
			HitLoc = Mesh.GetBoneLocation(HostageHealthBuckets[i]);
			Momentum = Normal(HitLoc - Location) * 10.f;
			BreakConstraint(Momentum, HitLoc, HostageHealthBuckets[i]);
		}
	}
}


/**
 * Turn Hostage into a pseudo rag doll, so he can be gibbed.
 * Pawn is not actually dead yet.
 */
simulated function SetMeatShieldRagDoll()
{
	if( Mesh == None || Mesh.PhysicsAsset == None || Mesh.PhysicsAssetInstance == None )
	{
		return;
	}

	// Turn off physics hit reactions, so there is no collision.
	if( PhysicsImpactBlendTimeToGo > 0.f )
	{
		BodyImpactBlendOutNotify();
	}

	bInMeatShieldRagDoll = TRUE;

	// Unfix MeatShield physics bones, fix all others
	Mesh.PhysicsAssetInstance.SetNamedBodiesFixed(FALSE, MeatShieldUnfixedBoneList, Mesh, TRUE);
	// Unfix Dangle bones as well
	Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);

	// Make it start simulating
	Mesh.WakeRigidBody();

	// Enable full physics on body
	Mesh.PhysicsWeight = 1.f;
}

/** Restores mesh to original state. */
final simulated function CancelMeatShieldRagDoll()
{
	bInMeatShieldRagDoll	= FALSE;
	Mesh.PhysicsWeight		= 0.f;

	// Fix all bones back to animation. Return to being fully Kinematic.
	Mesh.PhysicsAssetInstance.SetAllBodiesFixed(TRUE);
	Mesh.PhysicsAssetInstance.SetFullAnimWeightBonesFixed(FALSE, Mesh);
}


event Touch(Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal)
{
	local GearPC PC;
	local AIAvoidanceCylinderComponent AvoidCyl;
	local Trigger_DoorInteraction	DoorTrigger;
	local Trigger_Engage Engage;
	local Trigger_LadderInteraction	LT;
	local GearDroppedPickup_Shield Shield;

	Super.Touch(Other, OtherComp, HitLocation, HitNormal);

	// look for a door trigger to interact with
	DoorTrigger = Trigger_DoorInteraction(Other);
	if ( DoorTrigger != None )
	{
		PC = GearPC(Controller);
		if( PC != None )
		{
			PC.OnEnterDoorTrigger( DoorTrigger );
		}
		return;
	}

	if (Role == ROLE_Authority)
	{
		Engage = Trigger_Engage(Other);
		if ( Engage != None )
		{
			if (EngageTrigger == None || !IsDoingASpecialMove() || GSM_Engage(SpecialMoves[SpecialMove]) == None)
			{
				EngageTrigger = Engage;
			}
			return;
		}
	}

	LT = Trigger_LadderInteraction(Other);
	if( LT != None )
	{
		LadderTrigger = LT;
		return;
	}

	// check for roadie running into a deployed shield
	if (Role == ROLE_Authority && IsDoingSpecialMove(SM_RoadieRun))
	{
		Shield = GearDroppedPickup_Shield(Other);
		if (Shield != None &&
			Shield.CanKickOver(self))
		{
			Shield.SetTimer( 0.85f,FALSE,nameof(Shield.KickOver) );
			ServerDoSpecialMove(SM_DoorKick, TRUE);
			return;
		}
	}

	if (MyGearAI != None)
	{
		AvoidCyl = AIAvoidanceCylinderComponent(OtherComp);
		if (AvoidCyl != None)
		{
			//`log(GetFuncName()@Other@OtherComp@AvoidCyl.TeamThatShouldFleeMe);
			AvoidCyl.AIEnteredAvoidanceCylinder(MyGearAI);
		}
	}
}

event UnTouch(Actor Other)
{
	local GearPC						PC;
	local Trigger_DoorInteraction	DoorTrigger;
	local Trigger_LadderInteraction	LT;
	//local Trigger_Engage Engage;

	Super.UnTouch(Other);

	//if (Role == ROLE_Authority)
	//{
	//	Engage = Trigger_Engage(Other);
	//	if ( Engage != None && EngageTrigger == Engage )
	//	{
	//		EngageTrigger = None;
	//		return;
	//	}
	//}

	DoorTrigger = Trigger_DoorInteraction(Other);
	if ( DoorTrigger != None )
	{
		PC = GearPC(Controller);
		if( PC != None )
		{
			PC.OnExitDoorTrigger();
		}
		return;
	}

	LT = Trigger_LadderInteraction(Other);
	if( LT != None && LT == LadderTrigger )
	{
		LadderTrigger = None;
	}
}

final event BumpLevel(Vector HitLocation, Vector HitNormal)
{
	local Emitter DustEmitter;
	local rotator SpawnRotation;
	local Vector HitOffset;

	HitOffset = (Location)+(HitNormal*10);
	if(WorldInfo.TimeSeconds > LastEvadeIntoWallTime + 3.0 && isEvading())
	{
		if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitOffset, 2000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
		{
			SpawnRotation = rotator(HitNormal);
			DustEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Effects_Gameplay.Player_Movement.P_Player_Cover_Hit', HitOffset, SpawnRotation );
			DustEmitter.ParticleSystemComponent.ActivateSystem();
		}

		LastEvadeIntoWallTime = WorldInfo.TimeSeconds;
	}
}
/**
 * Overridden to avoid the extra trace in Actor.
 */
simulated function bool CalcCamera(float DeltaTime, out vector out_CamLoc, out rotator out_camRot, out float out_FOV)
{
	return FALSE;
}


/************************************************************************************
 * Firing Code
 ***********************************************************************************/


function bool StopFiring()
{
	bPendingFire = FALSE;
	bPendingAltFire = FALSE;
	bPendingMeleeAttack = FALSE;

	if( Weapon != None )
	{
		Weapon.StopFire(Weapon.CurrentFireMode);
	}

	return TRUE;
}

simulated function HandleWeaponFiring()
{
	local bool	bIsControllerFireInputPressed, bIsWeaponPendingFire, bShouldDelayFiring, bCanEngageMelee;
	local byte	MeleeFiringMode, AltFiringMode;

	// Normal weapon firing (FiringMode 0)
	bIsControllerFireInputPressed	= IsControllerFireInputPressed(0);
	bIsWeaponPendingFire			= IsWeaponPendingFire(0);
	bShouldDelayFiring				= ShouldDelayFiring();

	// If weapon firing was delayed (bPendingFire=TRUE) and
	// the weapon can't fire or the player has let go of the trigger
	if( bPendingFire && !CanFireWeapon() )
	{
		bPendingFire = FALSE;
	}

	//debug
	`AILog_Ext( GetFuncName()@bIsControllerFireInputPressed@Controller.bFire@bIsWeaponPendingFire@bShouldDelayFiring@PawnCommitToFiring(0)@CanFireWeapon(), 'WeaponFiring', MyGearAI );

// 	if( InvManager != None )
// 	{
// 		`LogInv("bIsWeaponPendingFire:" @ bIsWeaponPendingFire @ "PawnCommitToFiring:" @ PawnCommitToFiring(0) @ "bIsControllerFireInputPressed:" @ bIsControllerFireInputPressed @ "CanFireWeapon:" @ CanFireWeapon() @ "bShouldDelayFiring:" @ bShouldDelayFiring @ "PendingFire:" @ InvManager.PendingFire[0]);
// 	}


	// If weapon is not about to fire and the pawn is trying to fire
	if( !bIsWeaponPendingFire && PawnCommitToFiring(0) )
	{
		// If need to delay firing because of animation transitions...
		if( bShouldDelayFiring )
		{
			if (!bPendingFire)
			{
				// firing was delayed, but set alert time anyway, so we stay alert if the shot was aborted
				// checking bPendingFire so we only do this on the first frame of the delay
				SetWeaponAlert(AimTimeAfterFiring);

				// tell server too, if we're the client, so everyone else gets the aiming anim started
				if (WorldInfo.NetMode == NM_Client)
				{
					ServerSetWeaponAlert(byte(AimTimeAfterFiring));
				}
			}

			// Delay because of animation transitions.
			// Example scenario: player ducks behind cover, and taps fire once.
			// The animations need to blend to the blind fire pose before the weapon can fire,
			// But if the player released fire before the animations blended, the weapon won't fire.
			// To fix this problem we set a bPendingFire flag when shots are delayed because of animations.
			bPendingFire = TRUE;
		}
		else
		{
			//`log("starting fire, bPendingFire?"@bPendingFire@"input pressed?"@(IsControllerFireInputPressed(0) && CanFireWeapon()));

			//debug
			`AILog_Ext( "StartFire 0", 'WeaponFiring', MyGearAI );

			// Start firing and cancel pending flag
			NotifyActuallyFiring();
			StartFire(0);
			bPendingFire = FALSE;
		}
	}
	else
	// Otherwise, if weapon is about to fire and is prevented from firing
	if( bIsWeaponPendingFire && (!bIsControllerFireInputPressed || !CanFireWeapon() || bShouldDelayFiring) )
	{
		//debug
		`AILog_Ext( "StopFire 0", 'WeaponFiring', MyGearAI );

		// Stop firing
		StopFire(0);
	}

	// Notify Special Move when fire button is pressed. Even if the pawn can't fire his weapon.
	if( bIsControllerFireInputPressed && SpecialMove != SM_None && SpecialMoves[SpecialMove].bForwardFirePressedNotification )
	{
		SpecialMoves[SpecialMove].FirePressedNotification();
	}

	// BEGIN ALT FIRE
	if( MyGearWeapon != None && MyGearWeapon.bAltFireWeapon )
	{
		AltFiringMode = class'GearWeapon'.const.ALTFIRE_FIREMODE;
		bIsWeaponPendingFire = IsWeaponPendingFire(AltFiringMode);

//		`log( self@bIsWeaponPendingFire@bShouldDelayFiring@bPendingAltFire@CanFireWeapon()@IsControllerFireInputPressed(AltFiringMode)@PawnCommitToFiring(AltFiringMode)@PawnCommitToFiring(AltFiringMode) );

		if( bPendingAltFire && !CanFireWeapon() )
		{
			bPendingAltFire = FALSE;
		}

		if( !bIsWeaponPendingFire && PawnCommitToFiring(AltFiringMode) )
		{
			// If no need to delay firing because of animation transitions, fire now
			if( bShouldDelayFiring )
			{
				// Delay because of animation transitions.
				bPendingAltFire = TRUE;
			}
			else
			{
				StartFire(AltFiringMode);
				bPendingAltFire = FALSE;
			}
		}
		else if( bIsWeaponPendingFire && (!IsControllerFireInputPressed(AltFiringMode) || !CanFireWeapon() || bShouldDelayFiring) )
		{
			StopFire(AltFiringMode);
		}
	}
	/// END ALT FIRE

	// Melee Attacks (FiringMode MELEE_ATTACK_FIREMODE)
	// Mimics normal firing code.
	MeleeFiringMode = class'GearWeapon'.const.MELEE_ATTACK_FIREMODE;

	bIsWeaponPendingFire = IsWeaponPendingFire(MeleeFiringMode);
	bIsControllerFireInputPressed = IsControllerFireInputPressed(MeleeFiringMode);
	bCanEngageMelee = CanEngageMelee();

	// If bPendingMeleeAttack and can't trigger a melee attack, then cancel bPendingMeleeAttack.
	if( bPendingMeleeAttack && (!bCanEngageMelee || !bIsControllerFireInputPressed) )
	{
		bPendingMeleeAttack = FALSE;
	}

	if( !bIsWeaponPendingFire && bCanEngageMelee && (bIsControllerFireInputPressed || bPendingMeleeAttack) )
	{
		if( !ShouldDelayMeleeAttack() )
		{
			bPendingMeleeAttack = FALSE;

			// break them out of cover if necessary
			if( IsInCover() )
			{
				BreakFromCover();
			}

			// Check if we have a DBNO player around us, and we can do a finishing move!
			if( CanDoSpecialMove(SM_CQCMove_B) )
			{
				ServerDoCQCMove(GB_B);
			}
			else
			{
				//`log("starting melee");
				// Make sure weapon is not firing
				StartFire(MeleeFiringMode);
			}
		}
		else
		{
			bPendingMeleeAttack = TRUE;
		}
	}
	else if( bIsWeaponPendingFire && (!IsControllerFireInputPressed(MeleeFiringMode) || Health <= 0 || Controller == None) )
	{
		// Only abort if not in the middle of a chainsaw attack
		if( !IsDoingASpecialMove() ||
				(!IsDoingSpecialMove(SM_ChainSawAttack) &&
				 !IsDoingSpecialMove(SM_ChainSawVictim) &&
				 !IsDoingSpecialMove(SM_ChainsawDuel_Leader) &&
				 !IsDoingSpecialMove(SM_ChainsawDuel_Follower) &&
				 !IsDoingSpecialMove(SM_ChainsawDuel_Draw))	)
		{
			StopFire(MeleeFiringMode);
		}
	}
}

simulated function NotifyActuallyFiring();

/**
 * Returns TRUE if our Pawn's weapon's trigger is being pressed.
 */
final simulated function bool IsWeaponPendingFire(Byte InFiringMode)
{
	return (InvManager != None && InvManager.PendingFire[InFiringMode] > 0);
}


/**
 * Is Controller Input requesting to fire for FiringMode InFiringMode?
 */
final simulated function bool IsControllerFireInputPressed(Byte InFiringMode)
{
	if( InFiringMode == class'GearWeapon'.const.DEFAULT_FIREMODE )
	{
		return (Controller != None && Controller.bFire == 1);
	}

	if( InFiringMode == class'GearWeapon'.const.MELEE_ATTACK_FIREMODE )
	{
		return (Controller != None && bWantsToMelee);
	}

	if( InFiringMode == class'GearWeapon'.const.ALTFIRE_FIREMODE )
	{
		return (GearPC(Controller) != None && GearPC(Controller).IsButtonActive(GB_LeftTrigger) );
	}

	return FALSE;
}

/**
* Player willing to fire? Or Player is already firing?
* Weapon may still not fire because of ShouldDelayFiring()
* But player would still be in a firing posture (CoverAction/Animations)
*/
final simulated function bool PawnCommitToFiring( Byte InFiringMode )
{
	if( IsWeaponPendingFire(InFiringMode) )
	{
		return TRUE;
	}

	if (GearWeap_GrenadeBase(Weapon) != None && Weapon.IsTimerActive('StartThrow'))
	{
		return TRUE;
	}

	if( IsControllerFireInputPressed(InFiringMode) ||
		(bPendingFire	 && InFiringMode == class'GearWeapon'.const.DEFAULT_FIREMODE ) ||
		(bPendingAltFire && InFiringMode == class'GearWeapon'.const.ALTFIRE_FIREMODE ) )
	{
		return CanFireWeapon();
	}
	return FALSE;
}

/**
 * Returns true if Pawn can fire his weapon.
 * Used by Weapon to figure when it can be fired, and by HUD to draw crosshair indicator.
 */
native simulated function bool CanFireWeapon(optional bool bTestingForTargeting);

/**
 * If TRUE, firing should be delayed.
 * This is used to keep the player animations in a firing stance, but prevents the gun from actually be fired.
 * This prevents firing during anim blend transitions.
 */
simulated function final bool ShouldDelayFiring()
{
	// If the weapon doesn't want to delay firing because of animations, then ignore this function.
	if( MyGearWeapon != None && MyGearWeapon.bNoAnimDelayFiring )
	{
		return FALSE;
	}

	// If in cover and CA_Default can't fire unless in 360 aiming.
	// Outside of 360 aiming, an action is required (blind firing or leaning).
	if( CoverType != CT_None && CoverAction == CA_Default && !bDoing360Aiming && IsHumanControlled() )
	{
		// we do allow firing if completely out of ammo though, so we can get the no-ammo feedback
		if (MyGearWeapon == None || MyGearWeapon.HasAnyAmmo())
		{
			return TRUE;
		}
	}

	// Check for animation transitions
	if( DoingAnimationTransition() )
	{
		return TRUE;
	}

	// When not in cover, delay if not in aiming stance.
	if( (CoverType == CT_None) && bTargetingNodeIsInIdleChannel && MyGearWeapon != None && (MyGearWeapon.bAllowAimingStance || MyGearWeapon.bAllowDownsightsStance) )
	{
		return TRUE;
	}

	return FALSE;
}


/** Returns TRUE is Pawn is doing an Animation Transition. */
simulated function final bool DoingAnimationTransition()
{
	local bool	bAllowFiring;

	// Check for mirror transitions
	if( bDoingMirrorTransition && !bDoing360Aiming )
	{
		return TRUE;
	}

	// Delay during animation transitions, or if we're expecting one to happen...
	if( (bDoingCoverActionAnimTransition || (CoverType != CT_None && CoverAction != CoverActionAnim)) )
	{
		// Some transitions allow firing
		bAllowFiring = (IsCoverActionLeaning(CoverActionAnim) && IsCoverActionPopUp(LastCoverActionAnim))
			|| (IsCoverActionPopUp(CoverActionAnim) && IsCoverActionLeaning(LastCoverActionAnim)
			|| (bWasDoing360Aiming && (IsCoverActionLeaning(CoverActionAnim) ||  IsCoverActionPopUp(CoverActionAnim)))
			|| (bDoing360Aiming && (IsCoverActionLeaning(LastCoverActionAnim) || IsCoverActionPopUp(LastCoverActionAnim)))
			);

		if( !bAllowFiring )
		{
			return TRUE;
		}
	}

	return FALSE;
}

simulated final function bool IsCoverActionLeaning(ECoverAction InAction)
{
	return (InAction == CA_LeanLeft || InAction == CA_LeanRight);
}

simulated final function bool IsCoverActionPopUp(ECoverAction InAction)
{
	return (InAction == CA_PopUp);
}

/** Updates the number of available fire tickets */
final function SetNumberOfFireTickets( int inNum )
{
	NumFireTickets = inNum;
	FireTickets.Length = Max(0,NumFireTickets);
}

/** Ask for a fire ticket for the target */
final function bool ClaimFireTicket( Controller inClaimer, optional bool bClaimAll, optional bool bTest )
{
	local int Idx, Num;
	local bool bResult;

	if(NumFireTickets < 0)
	{
		return TRUE;
	}

	// If not claiming all
	if( !bClaimAll )
	{
		// Check to see if we already have a ticket
		Idx = FireTickets.Find( inClaimer );
		if( Idx >= 0 )
		{
			return TRUE;
		}
	}

	Num = FireTickets.Length;
	for( Idx = 0; Idx < Num; Idx++ )
	{
		if( bClaimAll ||
			FireTickets[Idx] == None || FireTickets[Idx].bDeleteMe || FireTickets[Idx].IsInState('Dead') )
		{
			if( !bTest )
			{
				FireTickets[Idx] = inClaimer;
			}

			bResult = TRUE;

			if( !bClaimAll )
			{
				break;
			}
		}
	}

	return bResult;
}

/** Release all of ticket claims for given controller */
final function bool ReleaseFireTicket( Controller inReleaser )
{
	local int Idx, Num;
	local bool bResult;

	Num = FireTickets.Length;
	for( Idx = 0; Idx < Num; Idx++ )
	{
		if( FireTickets[Idx] == inReleaser )
		{
			FireTickets[Idx] = None;
			bResult = TRUE;
		}
	}

	return bResult;
}

function PossessedBy(Controller C, bool bVehicleTransition)
{
	local Weapon W;

	Super.PossessedBy(C, bVehicleTransition);

	if (PlayerController(C) == None && GearAI_TDM(C) == None)
	{
		//`log(self@C@GetFuncName()@CharacterName);
		// if singleplayer or an AI in multiplayer use the character name
		if(PlayerReplicationInfo != none)
		{
			PlayerReplicationInfo.SetPlayerName(CharacterName);
		}
	}

	if( (Role == ROLE_Authority) && ( C.IsPlayerOwned() ) )
	{
		// kick off an initial to create the first entry
		PlayerStatsUpdate();
		// and set a timer to refresh the time/location/rotation
		SetTimer( 1.f,TRUE,nameof(PlayerStatsUpdate) );
	}

	SetLightEnvironmentSettings();

	NotifyTeamChanged();

	MyGearAI = GearAI(Controller);

	if (bVehicleTransition && Weapon == None && InvManager != None && PlayerController(C) != None)
	{
		// make sure player equips a weapon
		foreach InvManager.InventoryActors(class'Weapon', W)
		{
			W.ClientWeaponSet(true);
		}
	}
}


/** Helper function to set the DLE settings on the pawn (can be used to look at gametypes or other types of config settings) **/
simulated function SetLightEnvironmentSettings()
{
	// If we are playing horde then we want to turn off the SH light for all AI enemies
	if( ClassIsChildOf( WorldInfo.GetGameClass(), class'GearGameHorde_Base') == TRUE )
	{
		if( IsHumanControlled() == FALSE )
		{
			LightEnvironment.bSynthesizeSHLight = FALSE;
		}
	}

	// local humans controlled get the better DLE
	if( ( IsHumanControlled() == TRUE ) && ( IsLocallyControlled() == TRUE ) )
	{
		LightEnvironment.LightShadowMode = LightShadow_ModulateBetter;
	}
}


function UnPossessed()
{
	local PlayerReplicationInfo PRI;
	PRI = PlayerReplicationInfo;
	super.unpossessed();
	// preserve the PRI past death since we still refer to it via HUD icons, etc
	PlayerReplicationInfo = PRI;
	// reset our DPG
	SetDepthPriorityGroup(SDPG_World);
	// notify weapon
	if ( MyGearWeapon != None )
	{
		MyGearWeapon.NotifyUnpossessed();
	}
	// clear ai ref
	MyGearAI = None;
}

function DetachFromController(optional bool bDestroyController)
{
	Super.DetachFromController(bDestroyController);

	MyGearAI = None;
}


function bool BeginAttackRun( GearPawn NewVictim );

function bool IsAttacking();


/**
 *	Cleanup stuff after attack run is complete
 */
function EndAttackRun();


// FaceFX hooks

/** Handler for Matinee wanting to play FaceFX animations in the game. */
simulated event bool PlayActorFaceFXAnim(FaceFXAnimSet AnimSet, String GroupName, String SeqName, SoundCue SoundCueToPlay )
{
	//`log("Play FaceFX animation from code for" @ Self @ "GroupName:" @ GroupName @ "AnimName:" @ SeqName @ SoundCueToPlay);
	return Mesh.PlayFaceFXAnim(AnimSet, SeqName, GroupName, SoundCueToPlay);
}

/** Handler for Matinee wanting to stop FaceFX animations in the game. */
simulated event StopActorFaceFXAnim()
{
	Mesh.StopFaceFXAnim();
}

/** Used to let FaceFX know what component to play dialogue audio on. */
simulated event AudioComponent GetFaceFXAudioComponent()
{
	return FacialAudioComp;
}

/** Function for handling the SeqAct_PlayFaceFXAnim Kismet action working on this Actor. */
simulated function OnPlayFaceFXAnim(SeqAct_PlayFaceFXAnim inAction)
{
	//`log("Play FaceFX animation from KismetAction for" @ Self @ "GroupName:" @ inAction.FaceFXGroupName @ "AnimName:" @ inAction.FaceFXAnimName);
	Mesh.PlayFaceFXAnim(inAction.FaceFXAnimSetRef, inAction.FaceFXAnimName, inAction.FaceFXGroupName, inAction.SoundCueToPlay);
}

/**
* Returns TRUE if Actor is playing a FaceFX anim.
* Implement in sub-class.
*/
simulated function bool IsActorPlayingFaceFXAnim()
{
	return (Mesh != None && Mesh.IsPlayingFaceFXAnim());
}

/**
* Returns FALSE if Actor can play. For pawn, if he's speaking, no it can't play
* Implement in sub-class.
*/
simulated function bool CanActorPlayFaceFXAnim()
{
	// if pawn is speaking, then no actor can't play facefx
	return (!bSpeaking);
}

/**
 * Called via delegate when FacialAudioComp is finished.
 */
final simulated function FaceFXAudioFinished(AudioComponent AC)
{
	if ( (AC == FacialAudioComp) && (CurrentlySpeakingLine == FacialAudioComp) )
	{
		CurrentlySpeakingLine = None;
	}
}

simulated function OnAddRemoveFaceFXAnimSet(SeqAct_AddRemoveFaceFXAnimSet InAction)
{
	local int i, j;

	Super.OnAddRemoveFaceFXAnimSet(InAction);

	if (InAction.InputLinks[0].bHasImpulse)
	{
		for (i = 0; i < InAction.FaceFXAnimSets.length; i++)
		{
			for (j = 0; j < ArrayCount(ReplicatedFaceFXAnimSets); j++)
			{
				if (ReplicatedFaceFXAnimSets[j] == None)
				{
					ReplicatedFaceFXAnimSets[j] = InAction.FaceFXAnimSets[i];
					break;
				}
			}
			if (j == ArrayCount(ReplicatedFaceFXAnimSets))
			{
				`Warn("Not enough space in ReplicatedFaceFXAnimSets array for" @ InAction.FaceFXAnimSets[i]);
			}
		}
	}
	else if (InAction.InputLinks[1].bHasImpulse)
	{
		for (i = 0; i < InAction.FaceFXAnimSets.length; i++)
		{
			for (j = 0; j < ArrayCount(ReplicatedFaceFXAnimSets); j++)
			{
				if (ReplicatedFaceFXAnimSets[j] == InAction.FaceFXAnimSets[i])
				{
					ReplicatedFaceFXAnimSets[j] = None;
					break;
				}
			}
		}
	}
}

/** Kismet action to forcefully drop a heavy weapon if this pawn has one */
function OnForceDropHeavyWeapon( SeqAct_ForceDropHeavyWeapon Action )
{
	if( GearWeap_HeavyBase(MyGearWeapon) != none )
	{
		ThrowActiveWeapon();
	}
}


/**
 * Overridden to prevent weapon drops if flagged (for AI, etc).
 */
function ThrowActiveWeapon(optional class<DamageType> DamageType)
{
	// Cancel any weapon specific special moves
	if( IsDoingSpecialMove(SM_TargetMinigun) || IsDoingSpecialMove(SM_TargetMortar) || IsDoingSpecialMove(SM_UnMountMinigun) || IsDoingSpecialMove(SM_UnMountMortar) )
	{
		ServerEndSpecialMove();
	}

	if( MyGearWeapon.bCanThrowActiveWeapon )
	{
		Super.ThrowActiveWeapon(DamageType);
	}

	// If we end up without a weapon, make sure we switch to one.
	if( Weapon == None && InvManager.PendingWeapon == None && Health > 0 )
	{
		GearInventoryManager(InvManager).AutoSwitchWeapon();
	}
}

function TossInventory( Inventory Inv, optional vector ForceVelocity, optional class<DamageType> DamageType )
{
	local rotator ViewRot;
	local vector TossVel;
	local GearInventoryManager GIM;

	GIM = GearInventoryManager(InvManager);
	if ( IsZero(ForceVelocity) && (GIM != None) )
	{
		// for gears, we'll standardize the toss velocities unless overridden
		// rather than letting Pawn do it for us
		ViewRot = GetViewRotation();
		ViewRot.Pitch = GIM.InventoryTossPitch;
		TossVel = (vector(ViewRot) * GIM.InventoryTossSpeed) + Velocity;
	}
	else
	{
		TossVel = ForceVelocity;
	}

	Super.TossInventory(Inv, TossVel);
}

//@HACK: workaround for client seeing server not attached to the Reaver correctly
simulated final function ReaverAttachmentHack()
{
	if ( Vehicle_RideReaver_Base(DrivenVehicle) != None ||
		(GearWeaponPawn(DrivenVehicle) != None && Vehicle_RideReaver_Base(DrivenVehicle.GetVehicleBase()) != None) )
	{
		DrivenVehicle.AttachDriver(self);
	}
}

/**
 * StartDriving() and StopDriving() also called on clients
 * on transitions of DrivenVehicle variable.
 * Network: ALL
 */
simulated event StartDriving(Vehicle V)
{
	local GearAI AI;

	Super.StartDriving(V);

	AI = GearAI(Controller);
	if( AI != None )
	{
		AI.NotifyStartDriving(V);
	}

	if( V.IsA('Turret_TroikaCabal') )
	{
		bIsOnTurret = TRUE;
		// Holster Weapon first if we have one
		if( MyGearWeapon != None )
		{
			MyGearWeapon.HolsterWeaponTemporarily(TRUE);
		}
		else
		{
			StartManningTurret();
		}
	}

	//@HACK: workaround for client seeing server not attached to the Reaver correctly
	if ( WorldInfo.NetMode == NM_Client && V.bAttachDriver &&
		( Vehicle_RideReaver_Base(DrivenVehicle) != None ||
			(GearWeaponPawn(DrivenVehicle) != None && Vehicle_RideReaver_Base(DrivenVehicle.GetVehicleBase()) != None) ) )
	{
		SetTimer(1.0, false, nameof(ReaverAttachmentHack));
	}
}

/** event called when Pawn is ready to man turret */
final simulated function StartManningTurret()
{
	// Start troika manning animations
	TroikaBlendNode.SetActiveChild(0, 0.2f);

	// Disable turn in place node
	bCanDoTurnInPlaceAnim = FALSE;

	// Hook up IK, to hands are attached to handles on troika
	IKCtrl_RightHand.EffectorLocationSpace = BCS_WorldSpace;
	IKCtrl_LeftHand.EffectorLocationSpace = BCS_WorldSpace;
	IKCtrl_RightHand.SetSkelControlActive(TRUE);
	IKCtrl_LeftHand.SetSkelControlActive(TRUE);

	IKRotCtrl_RightHand.SetSkelControlActive(TRUE);
	IKRotCtrl_LeftHand.SetSkelControlActive(TRUE);
	IKRotCtrl_RightHand.bApplyRotation = TRUE;
	IKRotCtrl_LeftHand.bApplyRotation = TRUE;
	IKRotCtrl_RightHand.BoneRotationSpace = BCS_WorldSpace;
	IKRotCtrl_LeftHand.BoneRotationSpace = BCS_WorldSpace;
}

reliable client function ForceClientStopDriving(Vehicle V)
{
	StopDriving(V);
}

/**
 * StartDriving() and StopDriving() also called on clients
 * on transitions of DrivenVehicle variable.
 * Network: ALL
 */
simulated event StopDriving(Vehicle V)
{
	local GearAI AI;
	local GearPC GPC;
	local LocalPlayer LP;
	local int i;

	Super.StopDriving(V);

	AI = GearAI(Controller);
	if (AI != None)
	{
		AI.NotifyStopDriving(V);
	}

	if (Mesh != None)
	{
		if (Mesh.AnimTreeTemplate != default.Mesh.AnimTreeTemplate)
		{
			Mesh.SetAnimTreeTemplate(default.Mesh.AnimTreeTemplate);
			ClearAnimNodes();
			CacheAnimNodes();
		}
		Mesh.SetTranslation(default.Mesh.Translation);

		KismetAnimSets[0] = None;
		KismetAnimSets[1] = None;
		UpdateAnimSetList();
	}

	// Make sure any overlay is off
	GPC = GearPC(Controller);
	if( GPC != None && GPC.bHasCentaurOverlay )
	{
		LP = LocalPlayer(GPC.Player);
		if (LP != None)
		{
			for (i = 0; i < LP.PlayerPostProcessChains.length; i++)
			{
				if (LP.PlayerPostProcessChains[i].FindPostProcessEffect('MotionEffect') != None)
				{
					LP.RemovePostProcessingChain(i);
					i--;
				}
			}
		}

		GPC.bHasCentaurOverlay = FALSE;
	}

	if (bIsOnTurret)
	{
		bIsOnTurret = FALSE;

		// Enable turn in place node
		bCanDoTurnInPlaceAnim = default.bCanDoTurnInPlaceAnim;

		// Stop troika manning animations
		TroikaBlendNode.SetActiveChild(1, 0.2f);

		// Disable hand IKs
		IKCtrl_RightHand.SetSkelControlActive(FALSE);
		IKCtrl_LeftHand.SetSkelControlActive(FALSE);
		IKRotCtrl_RightHand.SetSkelControlActive(FALSE);
		IKRotCtrl_LeftHand.SetSkelControlActive(FALSE);

		// Unholster weapon
		MyGearWeapon.HolsterWeaponTemporarily(FALSE);

		if ( V != None && (Role == ROLE_Authority) && (InterpActor_GearBasePlatform(V.Base) != None) )
		{
			// the super put us into falling with no base, but we don't want that on moving platforms or we can shoot backwards
			SetBase(V.Base);
			SetPhysics(PHYS_Walking);
		}

		// make sure collision got restored
		SetCollisionSize(default.CylinderComponent.CollisionRadius, default.CylinderComponent.CollisionHeight);
	}
	else if (Role < ROLE_Authority && RemoteWeaponRepInfo.WeaponClass != None)
	{
		UpdateRemoteWeapon();
	}
	else if (MyGearWeapon != None)
	{
		AttachWeapon();
	}

	RefreshRemoteAttachments();
}


/**
 * Used to draw info over pawns in MP.
 * Things we care about:
 *
 * -if in "cover" or not (for the viewing pawn)
 * -HP numbers
 * -ActiveReload active
 *
 **/
simulated function DrawDebug_MP( GearHUD_Base HUD, Pawn P )
{
	local Texture2D InCoverIcon;
	local float		PercentDamageReduction;

	if( IsProtectedByCover( Location - P.Location ) == TRUE )
	{
		InCoverIcon = Texture2D'AI-Icons.AICON-H';	// cool sunglasses for being cool in cover
	}
	else
	{
		InCoverIcon = Texture2D'EditorResources.S_NavP'; // standy guy as you are running around like a maniac
	}

	PercentDamageReduction = MultiplayerCoverDamageAdjust( (Location - P.Location), P, Location, 100 );

	DrawIconOverhead( HUD, InCoverIcon, PercentDamageReduction );
}


simulated function DrawIconOverhead( GearHUD_Base HUD, Texture2D Icon, float PercentDamageReduction )
{
	local GearPC		PC;
	local Canvas	Canvas;
	local Vector	CameraLoc, ScreenLoc;
	local Rotator	CameraRot;
	local String	Str;
	local float		X, Y, HealthPct;

	if (Icon == None)
	{
		return;
	}

	PC = GearPC(HUD.PlayerOwner);
	Canvas = HUD.Canvas;
	Canvas.SetDrawColor(255,255,255);

	// project location onto the hud
	PC.GetPlayerViewPoint( CameraLoc, CameraRot );
	ScreenLoc = Canvas.Project( Location + vect(0,0,1) * GetCollisionHeight() * 1.5f );
	Canvas.SetPos( ScreenLoc.X - Icon.SizeX/2, ScreenLoc.Y - Icon.SizeY/2 );
	Canvas.DrawTexture( Icon, 1.f );

	Canvas.SetDrawColor(255,255,255);

	X = ScreenLoc.X + Icon.SizeX/2 + 5;
	Y = ScreenLoc.Y - Icon.SizeY/2;
	Canvas.SetPos( X, Y );

	Str = "H:" $ Health $ " AR: " $ bActiveReloadBonusActive $ " %DamGiven: " $ PercentDamageReduction;
	Canvas.DrawText( Str, TRUE );

	Canvas.SetDrawColor(255,255,255);

	Canvas.SetDrawColor(0,255,0);
	HealthPct = (Health/float(DefaultHealth));
	Canvas.DrawTile(Texture2D'WhiteSquareTexture',64.f * HealthPct,4,0,0,2,2);
	Canvas.SetDrawColor(255,0,0);
	//Canvas.SetPos(Canvas.CurX + HealthPct * 64.f,Canvas.CurY);
	Canvas.DrawTile(Texture2D'WhiteSquareTexture',64.f * (1.f - HealthPct),4,0,0,2,2);

	Canvas.SetDrawColor(255,255,255);
}


/**
 * Determines whether or not the pawn can be curbstomped.
 * This is also used for execution validity
 */
function simulated bool CanBeCurbStomped()
{
	return FALSE;
}

/**
 * We do not want to be encroached.
 * @todo:  make this check for GearPawns but let vehicles encroach us?
 **/
event EncroachedBy( actor Other )
{
}

event bool EncroachingOn(Actor Other)
{
	if ( !bScriptInitialized )
	{
		`log( "EncroachedOn self: " $ self $ " other: " $ Other $" at "$Other.Location );
		return false;
	}
	return Super.EncroachingOn(Other);
}


function SetMovementPhysics()
{
	// No movement physics if on a vehicle
	if ( DrivenVehicle != None || Pawn(Base) != None ||
		(InterpActor_GearBasePlatform(Base) != None && InterpActor_GearBasePlatform(Base).bDisallowPawnMovement) )
	{
		SetPhysics( PHYS_None );
	}
	else
	{
		super.SetMovementPhysics();
	}
}

/** This will start the RoadieRun Particle Effect **/
final simulated function StartRoadieRunPSC()
{
	local ParticleSystem RoadieRunSystem;

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 2000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return;
	}

	// we prob want to IsEffectRelevant for this.  And when people come into our view from roadie running 10000 units without stopping
	// we will just not see their effect.
	RoadieRunSystem = class'GearPhysicalMaterialProperty'.static.DetermineRoadieRunParticleEffect( class'GearPhysicalMaterialProperty'.static.GetImpactPhysicalMaterialHitInfo( LastFootStepTraceInfo ), WorldInfo);
	if( RoadieRunSystem != none )
	{
		if (PSC_RoadieRun != None)
		{
			PSC_RoadieRun.DeActivateSystem();
			PSC_RoadieRun.SetTemplate(RoadieRunSystem);
		}
		else
		{
			PSC_RoadieRun = new(self) class'ParticleSystemComponent';
			PSC_RoadieRun.SetTemplate(RoadieRunSystem);
			PSC_RoadieRun.bAutoActivate = FALSE;
			Mesh.AttachComponent( PSC_RoadieRun, RightFootBoneName, vect(0,0,0),,);
		}

		PSC_RoadieRun.SetHidden( FALSE );
		PSC_RoadieRun.ActivateSystem();
	}

	ClearTimer( 'HideRoadieRunPSC' );

	LastFootStepPhysMat = LastFootStepTraceInfo.PhysMaterial;
	//`log( PSC_RoadieRun @ LastFootStepPhysMat @ LastFootStepTraceInfo.PhysMaterial );
}

final simulated function HideRoadieRunPSC()
{
	Mesh.DetachComponent(PSC_RoadieRun);
	PSC_RoadieRun = None;
}

simulated function PlayBoltEject()
{
	local GearWeap_SniperRifle snipe;
	snipe = GearWeap_SniperRifle(Weapon);
	if(snipe != none)
		snipe.PlayBoltEject();
}

simulated function PistolEjectClip()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.ejectclip();
}

simulated function PistolCockOpen()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.cockopen();
}
simulated function PistolCockClose()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.cockclose();
}
simulated function PistolClipImpact()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.clipimpact();
}
simulated function PistolInsertClip()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.insertclip();
}

simulated function PistolJammed()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.jammed();
}

simulated function PistolHandSlam()
{
	local GearWeap_PistolBase pistol;
	pistol = GearWeap_PistolBase(Weapon);
	if(pistol != none)
		pistol.handslam();
}



/**
 * Notification that root motion mode changed.
 * Called only from SkelMeshComponents that have bRootMotionModeChangeNotify set.
 * This is useful for synchronizing movements.
 * For intance, when using RMM_Translate, and the event is called, we know that root motion will kick in on next frame.
 * It is possible to kill in-game physics, and then use root motion seemlessly.
 */
simulated event RootMotionModeChanged(SkeletalMeshComponent SkelComp)
{
	// Forward to current special move
	if( SpecialMove != SM_None && SpecialMoves[SpecialMove] != None )
	{
		SpecialMoves[SpecialMove].RootMotionModeChanged(SkelComp);
	}
}

/**
 * Notification called after root motion has been extracted, and before it's been used.
 * This notification can be used to alter extracted root motion before it is forwarded to physics.
 * It is only called when bRootMotionExtractedNotify is TRUE on the SkeletalMeshComponent.
 * @note: It is fairly slow in Script, so enable only when really needed.
 */
simulated event RootMotionExtracted(SkeletalMeshComponent SkelComp, out BoneAtom ExtractedRootMotionDelta)
{
	// Forward to current special move
	if( SpecialMove != SM_None && SpecialMoves[SpecialMove] != None )
	{
		SpecialMoves[SpecialMove].RootMotionExtracted(SkelComp, ExtractedRootMotionDelta);
	}
}

//@todo: FIXME: temp debug code, remove me!!!
exec function DebugFall()
{
	SetPhysics(PHYS_Falling);
	ServerDebugFall();
}
server reliable function ServerDebugFall()
{
	SetPhysics(PHYS_Falling);
}

/**
 * Returns camera "no render" cylinder.
 * @param bViewTarget TRUE to return the cylinder for use when this pawn is the camera target, false to return the non-viewtarget dimensions
 */
final simulated native function GetCameraNoRenderCylinder(out float Radius, out float Height, bool bViewTarget, bool bHiddenLocally);


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

	if( WorldInfo.GRI.ShouldShowGore() && bSpawnBloodTrailDecals )
	{
		ImpactEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( GoreImpactParticle, HitLoc, rotator(HitNorm) );
		ImpactEmitter.ParticleSystemComponent.ActivateSystem();

		//`log( "blood " $ HitLoc @ SquaredForce @ EffectCount );
		GearPawnFX.SpawnABloodTrail_GibImpact( HitLoc );
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

simulated event bool InFreeCam()
{
	local PlayerController	PC;

	PC = PlayerController(Controller);
	return (PC != None && PC.PlayerCamera != None && (PC.PlayerCamera.CameraStyle == 'FreeCam' || PC.PlayerCamera.CameraStyle == 'FaceCam') );
}

event Landed(vector HitNormal, Actor FloorActor)
{
	local GearSpawner_EmergenceHoleBase EHole;
	EHole = GearSpawner_EmergenceHoleBase(FloorActor);
	if (EHole != None &&
		Location.Z < EHole.Location.Z)
	{
		TakeDamage(99999,None,Location,vect(0,0,0),class'GDT_ScriptedRagdoll',,EHole);
	}
	Super.Landed(HitNormal,FloorActor);
}


/**
 * This will set the DurationBeforeDestroyingDueToNotBeingSeen to the value the Kismet action wants it.
 * This is useful for keeping certain bodies around longer than others fo
 **/
simulated function OnModifyDurationBeforeDestroyingDueToNotBeingSeen( SeqAct_ModifyDurationBeforeDestroyingDueToNotBeingSeen Action )
{
	DurationBeforeDestroyingDueToNotBeingSeen = Action.NewDurationBeforeDestroyingDueToNotBeingSeen;
}


simulated function GetAimFrictionExtent(out float Width, out float Height, out vector Center)
{
	if (!bCanBeFrictionedTo)
	{
		Width = 0.f;
		Height = 0.f;
	}
	else if ( bIsCrouched || ((CoverType == CT_MidLevel) && (CoverAction != CA_PopUp)) )
	{
		// smaller cylinder for crouched pawns
		Height = CrouchedFrictionCylinder.Height;
		Width = CrouchedFrictionCylinder.Radius;
	}
	else
	{
		super.GetAimFrictionExtent(Width, Height, Center);
	}

	Center = GetBaseTargetLocation();
}


simulated function GetAimAdhesionExtent(out float Width, out float Height, out vector Center)
{
	if (!bCanBeAdheredTo)
	{
		Width = 0.f;
		Height = 0.f;
	}
	else if ( bIsCrouched || ((CoverType == CT_MidLevel) && (CoverAction != CA_PopUp)) )
	{
		// smaller cylinder for crouched pawns
		Height = CrouchedFrictionCylinder.Height;
		Width = CrouchedFrictionCylinder.Radius;
	}
	else
	{
		super.GetAimAdhesionExtent(Width, Height, Center);
	}

	// reduce the size a bit to allow adhesion to move the crosshair onto the character
	Height *= 0.65f;
	Width *= 0.65f;

	Center = GetBaseTargetLocation();
}


final simulated function vector GetDeathCamLookatPos()
{
	if ( bOverrideDeathCamLookat && (DeathCamLookatPawn != None) )
	{
		if (DeathCamLookatBoneName != '')
		{
			return DeathCamLookatPawn.Mesh.GetBoneLocation(DeathCamLookatBoneName);
		}
		else
		{
			return DeathCamLookatPawn.Location;
		}
	}
	else if (bIsGore)
	{
		// look where you used to be, and mourn your lack of l33tness
		return LocationWhenKilled;
	}
	else
	{
		// track head of your corpse
		return Mesh.GetBoneLocation(NeckBoneName);
	}
}

/** This is set when we want the mob to not use AdjustPawnDamage or ReduceDamage functions @see GearProj_Grenade.DoExplosion **/
//@fixme - renamed bDontModifyDamage, but need to rename function and fixup refs to avoid the confusion!
simulated function SetDontModifyDamage( bool NewValue )
{
	bAllowDamageModification = !NewValue;
}

function OnSetPawnAudio( SeqAct_SetPawnAudio Action )
{
	if ( (Action.InputLinks[0].bHasImpulse) && (Action.PawnClass != None) )
	{
		// copy settings
		SoundGroup = Action.PawnClass.default.SoundGroup;
//		MasterGUDBankClass = Action.PawnClass.default.MasterGUDBankClass;
		NeedsRevivedGUDSEvent = Action.PawnClass.default.NeedsRevivedGUDSEvent;
		WentDownGUDSEvent = Action.PawnClass.default.WentDownGUDSEvent;
	}
	else
	{
		// back to my own defaults
		SoundGroup = default.SoundGroup;
//		MasterGUDBankClass = default.MasterGUDBankClass;
		NeedsRevivedGUDSEvent = default.NeedsRevivedGUDSEvent;
		WentDownGUDSEvent = default.WentDownGUDSEvent;
	}
}

/** Sets this pawn on fire. */
simulated exec function IgnitePawn(PawnOnFireState NewState, optional float Duration)
{
	local ParticleSystem FirePS;
	local SoundCue FireSound;

	if (OnFireState != NewState)
	{
		// extinguish old state
		if (OnFireState != POF_None)
		{
			ExtinguishPawn();
			ClearTimer('ExtinguishPawn');
		}

		if (NewState != POF_None)
		{
			// pick effects
			switch (NewState)
			{
			case POF_Blazing:
				FirePS = OnFireBlazingParticleSystem;
				FireSound = OnFireBlazingBurningSound;
				break;
			case POF_Smoldering:
				FirePS = OnFireSmolderParticleSystem;
				FireSound = OnFireSmolderingBurningSound;
				break;
			default:
				`log("Unhandled OnFire state in"@GetFuncName());
			}

			// start particle effect
			PSC_OnFire = new(Outer) class'ParticleSystemComponent';
			if (PSC_OnFire != None)
			{
				PSC_OnFire.SetTemplate(FirePS);
				PSC_OnFire.SetAbsolute(, true);
				PSC_OnFire.SetRotation(rot(0,0,0));
				Mesh.AttachComponent(PSC_OnFire, TorsoBoneName);
			}

			// start audio loop
			OnFireAudioBurningLoop = CreateAudioComponent(FireSound, false, true);
			if (OnFireAudioBurningLoop != None)
			{
				OnFireAudioBurningLoop.bAutoDestroy = true;
				Mesh.AttachComponent(OnFireAudioBurningLoop, TorsoBoneName);
				OnFireAudioBurningLoop.FadeIn(0.6f, 1.f);
			}
		}

		OnFireState = NewState;
		if (Role == ROLE_Authority)
		{
			ReplicatedOnFireState = OnFireState;
		}
	}

	if (Duration != 0.f)
	{
		SetTimer( Duration, false, nameof(ExtinguishPawn) );
	}
}

/** Called when pawn fire effect is done, so we can clean up. */
simulated function OnPawnFireEffectFinished(ParticleSystemComponent PSC)
{
	Mesh.DetachComponent(PSC);
}

/** Puts out the fire on this pawn. */
simulated exec function ExtinguishPawn()
{
	if (OnFireState != POF_None)
	{
		// stop particle effect
		if (PSC_OnFire != None)
		{
			PSC_OnFire.DeactivateSystem();
			PSC_OnFire.OnSystemFinished = OnPawnFireEffectFinished;
			PSC_OnFire = None;
		}

		// stop audio
		if (OnFireAudioBurningLoop != None)
		{
			OnFireAudioBurningLoop.FadeOut(1.f, 0.0f);
			OnFireAudioBurningLoop = None;
		}

		CurrentSkinHeatMin = 0.0f; // allow the pawn to fade his burny look to nothing
		SkinHeatFadeTime = 1.0f; // fade out nicely
		OnFireState = POF_None;
	}
}

/** Sets this pawn to start imulsion effects */
simulated function StartImulsion( float Duration )
{
	if ( PSC_Imulsion == None )
	{
		// start particle effect
		PSC_Imulsion = new(Outer) class'ParticleSystemComponent';
		if ( PSC_Imulsion != None )
		{
			PSC_Imulsion.SetTemplate(ImulsionParticleSystem);
			PSC_Imulsion.SetAbsolute(, true);
			PSC_Imulsion.SetRotation(rot(0,0,0));
			Mesh.AttachComponent(PSC_Imulsion, TorsoBoneName);
		}

		// start audio loop
		ImulsionAudioBurningLoop = CreateAudioComponent(ImulsionBurningSound, false, true);
		if (ImulsionAudioBurningLoop != None)
		{
			ImulsionAudioBurningLoop.bAutoDestroy = true;
			Mesh.AttachComponent(ImulsionAudioBurningLoop, TorsoBoneName);
			ImulsionAudioBurningLoop.FadeIn(0.6f, 1.f);
		}
	}

	SetTimer( Duration, false, nameof(StopImulsion) );
}

/** Called when pawn imulsion effect is done, so we can clean up. */
simulated function OnPawnImulsionEffectFinished(ParticleSystemComponent PSC)
{
	Mesh.DetachComponent(PSC);
}

/** Stops the effects for taking imulsion damage */
simulated function StopImulsion()
{
	// stop particle effect
	if ( PSC_Imulsion != None )
	{
		PSC_Imulsion.DeactivateSystem();
		PSC_Imulsion.OnSystemFinished = OnPawnImulsionEffectFinished;
		PSC_Imulsion = None;
	}

	// stop audio
	if ( ImulsionAudioBurningLoop != None )
	{
		ImulsionAudioBurningLoop.FadeOut(1.f, 0.0f);
		ImulsionAudioBurningLoop = None;
	}
}

/************************************************************************************
 * Boomer Shield
 ***********************************************************************************/

/** Attachment bone and offsets for Shield. */
simulated function GetShieldAttachBone(out Name OutBoneName, out Rotator OutRelativeRotation, out Vector OutRelativeLocation, out Vector OutRelativeScale)
{
	local SkeletalMeshSocket	ShieldSocket;

	ShieldSocket = Mesh.GetSocketByName('Shield');
	if( ShieldSocket != None )
	{
		OutBoneName = ShieldSocket.BoneName;
		OutRelativeRotation = ShieldSocket.RelativeRotation;
		OutRelativeLocation = ShieldSocket.RelativeLocation;
		// Make shield smaller for non boomer guys.
		OutRelativeScale = Vect(0.75f, 0.75f, 0.75f);
	}
	else
	{
		// Defaults for Marcus Skeleton
		OutBoneName = 'b_MF_ForeTwist2_L';
		OutRelativeRotation.Roll = -16384;
		// Make shield smaller for non boomer guys.
		OutRelativeScale = Vect(0.75f, 0.75f, 0.75f);
	}
}

/** Equips the shield that is currently in the inventory. */
simulated function EquipShield(GearShield ShieldInv)
{
	local GearInventoryManager	GIM;
	local MeshComponent			ShieldMesh;
	local Name					AttachName;
	local Rotator				AttachRotation;
	local Vector				AttachLocation, AttachScale;
	local GearPC				MyGearPC;

	if( ShieldInv != None )
	{
		ShieldMesh = ShieldInv.Mesh;

		// attach shield mesh to pawn's mesh
		AttachScale = Vect(1,1,1);
		GetShieldAttachBone(AttachName, AttachRotation, AttachLocation, AttachScale);

		Mesh.AttachComponent(ShieldMesh, AttachName, AttachLocation, AttachRotation, AttachScale);
		ShieldMesh.SetShadowParent(Mesh);
		ShieldMesh.SetLightEnvironment(LightEnvironment);
		ShieldMesh.SetActorCollision(true, false);
		ShieldMesh.SetBlockRigidBody(true);

		// switch to a one-handed weapon, if necessary (e.g. pistol)
		if (MyGearWeapon == None || !MyGearWeapon.bCanEquipWithShield || InvManager.PendingWeapon != None)
		{
			GIM = GearInventoryManager(InvManager);
			GIM.SwitchToOneHandedWeapon();
		}

		EquippedShield = ShieldInv;

		// replicate to clients
		if (Role == ROLE_Authority)
		{
			AttachClass_Shield = ShieldInv.Class;
			bForceNetUpdate = TRUE;
		}

		// Call the delegates in the PC for equipping the shield
		MyGearPC = GearPC(Controller);
		if ( MyGearPC != None )
		{
			MyGearPC.TriggerGearEventDelegates( eGED_Shield );
		}

		ShieldInv.SetMaterialBasedOnTeam( ShieldMesh, GetMPWeaponColorBasedOnClass() );

	}
}

simulated reliable server function ServerDropShield()
{
	DropShield();
}

simulated function DropShield(optional bool bSkipToss)
{
	local MeshComponent ShieldMesh;

	// cancel the SM if it's current
	if( IsDoingSpecialMove(SM_DeployShield) || IsDoingSpecialMove(SM_PlantShield) )
	{
		EndSpecialMove();
	}

	if (EquippedShield != None)
	{
		// detach shield mesh from pawn's mesh
		ShieldMesh = EquippedShield.Mesh;
		if (Mesh.IsComponentAttached(ShieldMesh))
		{
			Mesh.DetachComponent(ShieldMesh);
		}
		ShieldMesh.SetShadowParent(None);
		ShieldMesh.SetLightEnvironment(None);
		ShieldMesh.SetActorCollision(FALSE, FALSE);
		ShieldMesh.SetBlockRigidBody(FALSE);

		// throw it from the inventory if allowed
		if ( (!bSkipToss) && (Role == ROLE_Authority) )
		{
			TossInventory(EquippedShield);

			// replicate to clients
			AttachClass_Shield = None;
			bForceNetUpdate = TRUE;
		}

		// and clear it off the pawn.
		EquippedShield = None;
	}
}

simulated final event bool IsCarryingShield()
{
	return (EquippedShield != None);
}

simulated final event bool IsDeployingShield()
{
	return ( IsCarryingShield() && IsDoingSpecialMove(SM_DeployShield) );
}

/** Returns true if this pawn is protected by a shield for attacks from the given direction. */
final simulated function bool IsProtectedByShield(vector ShotDirectionNorm, optional bool bIgnoreMeatshields, optional vector ShotOrigin)
{
	local float DotAngle, ProtAngleDeg;
	DotAngle = (-ShotDirectionNorm) dot vector(Rotation);
	// if passed an origin then project back along the normal to catch origins near the player's feet
	if (!IsZero(ShotOrigin))
	{
		ShotDirectionNorm = Normal((Location + vector(Rotation) * 64.f) - ShotOrigin);
	}
	DotAngle = (-ShotDirectionNorm) dot vector(Rotation);
	if (IsCarryingShield())
	{
		ProtAngleDeg = IsDeployingShield() ? DeployedShieldHalfAngleOfProtection : ShieldHalfAngleOfProtection;
		return (DotAngle > Cos(ProtAngleDeg * DegToRad));
	}
	else if (!bIgnoreMeatShields && IsAKidnapper())
	{
		ProtAngleDeg = ShieldHalfAngleOfProtection;
		return (DotAngle > Cos(ProtAngleDeg * DegToRad));
	}
	return FALSE;
}

/** Return true if the given component is a shield I have equipped, false otherwise. */
simulated final function bool ComponentIsAnEquippedShield(PrimitiveComponent Component)
{
	return ( (EquippedShield != None) && (EquippedShield.Mesh == Component) );
};

/** debug */
simulated exec function Pratfall()
{
	Knockdown(vect(0,0,0), vect(0,0,0));
}

/** Returns true if shield will be raised, false otherwise. */
function bool TryToRaiseShieldOverHead()
{
	if (CanDoSpecialMove(SM_RaiseShieldOverHead))
	{
		ServerDoSpecialMove(SM_RaiseShieldOverHead);
		return TRUE;
	}

	return FALSE;
}

/************************************************************************************
 * Grappling Hook
 ***********************************************************************************/

/**
 * Function to let us know that a Pawn can use the grappling hook or not.
 * Mainly if he has animations to make this work.
 */
function bool CanUseGrapplingHook()
{
	return FALSE;
}

function OnAIGrapple(SeqAct_AIGrapple Action)
{
	// Kismet Sequence to use grappling hook. Pawn has to be able to support it to initiate it.
	if( Action.InputLinks[0].bHasImpulse )
	{
		// new grappling hook code
		if( Gear_GrappleHookMarker(Action.GrappleTargets[0]) != None )
		{
			GrappleTo( Gear_GrappleHookMarker(Action.GrappleTargets[0]) );
		}
		// old grappling hook code
		else
		{
			AttachedGrappleRope = Spawn( Action.GrappleRopeClass, self,,,,, TRUE );
			AttachedGrappleRope.LaunchTo(self, Action.GrappleTargets[0]);
		}
	}
}

function GrappleTo(Gear_GrappleHookMarker InGrappleHookMarker)
{
	if( !CanUseGrapplingHook() )
	{
		`Warn(Self @ "tried to grapple to" @ InGrappleHookMarker @ "but doesn't support Grappling!!");
		return;
	}

	CurrentGrapplingMarker = InGrappleHookMarker;
	ServerDoSpecialMove(SM_GrapplingHook_Climb, TRUE);

}

/** Get grappling hook ground marker location */
simulated function vector GetGrapplingHookGroundMarker()
{
	local Vector	TraceStart, TraceEnd, HitLocation, HitNormal, X, Y, Z, MarkerOffset;

	GetAxes(CurrentGrapplingMarker.Rotation, X, Y, Z);
	MarkerOffset = CurrentGrapplingMarker.MarkerOffset;

	// Test, no scaling, just use Marker default height
	if( TRUE )
	{
		return CurrentGrapplingMarker.Location + X * MarkerOffset.X + Y * MarkerOffset.Y + Z * MarkerOffset.Z;
	}
	// Otherwise trace to ground and perform scaling of animations
	else
	{
		// Scale offset by a little bit to trace further than default height.
		MarkerOffset = 1.5f * MarkerOffset;
		TraceStart = CurrentGrapplingMarker.Location;
		TraceEnd = TraceStart + X * MarkerOffset.X + Y * MarkerOffset.Y + Z * MarkerOffset.Z;

		if( Trace(HitLocation, HitNormal, TraceEnd, TraceStart, FALSE) != None )
		{
			return HitLocation;
		}
	}

	// If we couldn't hit anything, then return max range...
	return TraceEnd;
}

simulated function CreateGrapplingHook(Vector MarkerLocation, Rotator MarkerRotation);
simulated function DestroyGrapplingHook();

simulated function GearAnim_Slot GetGrapplingHookAnimSlot()
{
	return None;
}

/** Force all local player controllers to spectate this pawn. */
function ViewMe()
{
	local GearPC	OutGPC;

	foreach WorldInfo.LocalPlayerControllers(class'GearPC', OutGPC)
	{
		OutGPC.SetViewTarget(Self);
	}
}

function CancelViewMe()
{
	local GearPC	OutGPC;

	foreach WorldInfo.LocalPlayerControllers(class'GearPC', OutGPC)
	{
		if( OutGPC.ViewTarget == Self )
		{
			OutGPC.SetViewTarget(OutGPC.Pawn);
		}
	}
}

//
// OLD GRAPPLING HOOK CODE
//

function NotifyGrappleRopeIsAttached(GrappleRopeBase Rope)
{
	if (Rope == AttachedGrappleRope)
	{
		ClimbGrappleRope();
	}
}

function ClimbGrappleRope()
{
	local vector Dest;

	local Actor RopeBase;

	// do climb special move
	// hack for now, just teleport to the place to start climbing
	if (AttachedGrappleRope != None)
	{
		// ordering of calls were was arrived at through considerable pain.  change at your own risk.
		Dest = AttachedGrappleRope.LaunchTargetActor.Location + (AttachedGrappleRope.TmpStartClimbOffset >> AttachedGrappleRope.LaunchTargetActor.Rotation);
//		DrawDebugSphere(Dest, 64, 10, 255, 255, 0, TRUE);
		SetCollision(FALSE, bBlockActors);		// special move will restore this when finished
		SetLocation(Dest);
		SetRotation(AttachedGrappleRope.LaunchTargetActor.Rotation);
		RopeBase = AttachedGrappleRope.LaunchTargetActor.Base;

		DismountGrappleRope();

		if (RopeBase != None)
		{
//			SetHardAttach(TRUE);		// can't do this, root motion won't work
			SetBase(RopeBase);
		}

	}
}

function DismountGrappleRope()
{
	// do dismount special move
	DoSpecialMove(SM_Emerge_Type1);

	// detach rope
	AttachedGrappleRope.DetachFromOriginActor();
	AttachedGrappleRope = None;
}

//
// END OLD GRAPPLING HOOK CODE
//

/** Set new MeshTranslationOffset */
native final simulated function SetMeshTranslationOffset(vector NewOffset, optional bool bForce);

final simulated function CheckShieldDeployment()
{
	local bool		bShouldDeployShield;

	// Only driven by locally controlled player
	if( !IsLocallyControlled() )
	{
		return;
	}

	// if sliding to cover don't do anything
	if( IsDoingSpecialMove(SM_Run2MidCov) || IsDoingSpecialMove(SM_Run2StdCov) )
	{
		return;
	}

	bShouldDeployShield = ShouldDeployShield();

// 	`DLog("bShouldDeployShield" @ bShouldDeployShield);
	if( bShouldDeployShield && SpecialMove != SM_DeployShield )
	{
		if( CanDoSpecialMove(SM_DeployShield) )
		{
			LocalDoSpecialMove(SM_DeployShield, TRUE);
		}
// 		else
// 		{
// 			`DLog("failed to start special move SM_DeployShield. Current SpecialMove:" @ SpecialMove);
// 		}
	}
	else if( !bShouldDeployShield && IsDoingSpecialMove(SM_DeployShield) )
	{
		LocalEndSpecialMove(SM_DeployShield);
	}
}


/**
 * Check if shield should be deployed or not.
 */
simulated function bool ShouldDeployShield()
{
	if( !IsCarryingShield() )
	{
		return FALSE;
	}

	// Otherwise targeting mode indicates that shield should be deployed
	return bIsTargeting;
}

/************************************************************************************
 * Heavy Weapons
 ***********************************************************************************/

final simulated function bool IsCarryingAHeavyWeapon()
{
	return (MyGearWeapon != None && MyGearWeapon.WeaponType == WT_Heavy);
}

/**
 * See if heavy weapons should be mounted or unmounted.
 * @param	bCheckingFromSpecialMove	if check is done within a special move transition, and therefore special move checks should be bypassed.
 */
final simulated function CheckHeavyWeaponMounting(optional bool bCheckingFromSpecialMove)
{
	local GearWeap_HeavyMiniGunBase MiniGunWeapon;
	local GearWeap_HeavyMortarBase	MortarWeapon;
	local bool bShouldMountHeavyWeap;
	local INT	PackedCoverState;
	local GearPC PC;

	// Pawn has to be locally controlled to instigate heavy weapon mounting/unmounting.
	if( !IsLocallyControlled() )
	{
		return;
	}

	// if sliding to cover don't do anything
	if (IsDoingSpecialMove(SM_Run2MidCov) || IsDoingSpecialMove(SM_Run2StdCov))
	{
		return;
	}

	//`log("CheckHeavyWeaponMounting. bCheckingFromSpecialMove:" @ bCheckingFromSpecialMove);
	PC = GearPC(Controller);

	// ** MiniGun
	bShouldMountHeavyWeap = ShouldMountMiniGun();
	MiniGunWeapon = GearWeap_HeavyMiniGunBase(Weapon);
	if( MiniGunWeapon != None )
	{
		// attempt to run to cover
		if (bShouldMountHeavyWeap && !IsInCover() && PC != None && PC.TryToRunToCover(FALSE,0.5f,DCLICK_None,TRUE))
		{
			return;
		}
		//`log("Checking for MiniGun. bShouldMountMiniGun" @ bShouldMountMiniGun @ "MyGearPawn.SpecialMove:" @ MyGearPawn.SpecialMove);

		// Check if MiniGun should be mounted
		if( bCheckingFromSpecialMove || (SpecialMove != SM_TargetMinigun && SpecialMove != SM_UnMountMinigun) )
		{
			//`log("CanDoSpecialMove SM_TargetMinigun" @ CanDoSpecialMove(SM_TargetMinigun));
			if( bShouldMountHeavyWeap && CanDoSpecialMove(SM_TargetMinigun) )
			{
				// Pack cover state, as it's used to pick the correct animation. Replicated here, to make sure it's consistent.
				// As variables and functions are replicated in different orders.
				PackedCoverState = (CoverAction << 2) + CoverType;
				LocalDoSpecialMove(SM_TargetMinigun, TRUE, None, PackedCoverState);
			}
		}
	}
	// Check if MiniGun should be unmounted. Also done is Pawn isn't carrying heavy weapon anymore.
	if( SpecialMove == SM_TargetMinigun )
	{
		//`log("CanDoSpecialMove SM_UnMountMinigun" @ CanDoSpecialMove(SM_UnMountMinigun));
		if( !bShouldMountHeavyWeap )
		{
			if( CanDoSpecialMove(SM_UnMountMinigun) )
			{
				LocalDoSpecialMove(SM_UnMountMinigun, TRUE);
			}
		}
	}

	// ** Mortar
	bShouldMountHeavyWeap = ShouldMountMortar();
	MortarWeapon = GearWeap_HeavyMortarBase(Weapon);
	if( MortarWeapon != None )
	{
		// Check if Mortar should be mounted
		if( bCheckingFromSpecialMove || (SpecialMove != SM_TargetMortar && SpecialMove != SM_UnMountMortar) )
		{
			//`log("CanDoSpecialMove SM_TargetMinigun" @ CanDoSpecialMove(SM_TargetMinigun));
			if( bShouldMountHeavyWeap && CanDoSpecialMove(SM_TargetMortar) )
			{
				PackedCoverState = (CoverAction << 2) + CoverType;
				LocalDoSpecialMove(SM_TargetMortar, TRUE, None, PackedCoverState);
			}
		}
	}
	// Check if Mortar should be unmounted. Also done is Pawn isn't carrying heavy weapon anymore.
	if( SpecialMove == SM_TargetMortar )
	{
		//`log("CanDoSpecialMove SM_UnMountMinigun" @ CanDoSpecialMove(SM_UnMountMinigun));
		if( !bShouldMountHeavyWeap )
		{
			if( CanDoSpecialMove(SM_UnMountMortar) )
			{
				LocalDoSpecialMove(SM_UnMountMortar, TRUE);
			}
		}
	}
}

/**
 * Check if minigun should be mounted or not.
 */
final simulated function bool ShouldMountMiniGun()
{
	if( Weapon == None || !Weapon.IsA('GearWeap_HeavyMiniGun') || Weapon.IsInState('PuttingDownHeavyWeapon') )
	{
		return FALSE;
	}

	return Weapon.HasAnyAmmo() && bIsTargeting;
}

/**
 * Check if mortar should be mounted or not.
 */
final simulated function bool ShouldMountMortar()
{
	if( Weapon == None || !Weapon.IsA('GearWeap_HeavyMortar') || Weapon.IsInState('PuttingDownHeavyWeapon') )
	{
		return FALSE;
	}

	// don't allow mounting when reloading mortar
	if (IsReloadingWeapon() && !IsDoingSpecialMove(SM_TargetMortar))
	{
		return FALSE;
	}

	return Weapon.HasAnyAmmo() && bIsTargeting;
}

simulated function Notify_WeaponMountedImpact()
{
	local GearWeap_HeavyBase HWeap;
	HWeap = GearWeap_HeavyBase(Weapon);
	if (HWeap != None)
	{
		HWeap.Notify_MountedImpact();
	}
}


/**
 * This will start a trail timer.
 * @TODO: make a nice way to determine the time period to drop.  Or change this to me some like distance based updating.
 **/
simulated function StartBloodTrail( name TrailFuncName, optional float SpawnRate )
{
	if( SpawnRate == 0.0f )
	{
		SpawnRate = 0.5f;
	}

	SetTimer( SpawnRate, TRUE, TrailFuncName );
}


/** This will stop the head shot spurting blood trail @see PlayHeadShotDeath **/
simulated final function StopBloodTrail_HeadShot()
{
	ClearTimer( nameof(SpawnABloodTrail_HeadShot) );
}

/**
 * This will stop a trail timer.
 * @TODO: make a nice way to determine the time period to drop.  Or change this to me some like distance based updating.
 **/
simulated final function StopBloodTrail( name TrailFuncName )
{
	ClearTimer( TrailFuncName );
	LocationOfLastBloodTrail = vect(0,0,0);
}


/** This will leave a trail of blood when you are DBNO **/
simulated function SpawnABloodTrail_DBNO()
{
	GearPawnFX.SpawnABloodTrail_DBNO();
}

/** This will leave a trail of blood when you are MeatBagging **/
simulated function SpawnABloodTrail_MeatBag()
{
	GearPawnFX.SpawnABloodTrail_MeatBag();
}

/** This will leave a trail of blood when you are in cover **/
simulated function SpawnABloodTrail_Wall()
{
	if (Health < DefaultHealth)
	{
		GearPawnFX.SpawnABloodTrail_Wall();
	}
	else
	{
		StopBloodTrail(nameof(SpawnABloodTrail_Wall));
	}
}

simulated function SpawnABloodTrail_GibExplode_Ground()
{
	GearPawnFX.SpawnABloodTrail_GibExplode_Ground();
}

/** This will unleash a blast of gore on the walls, ceiling, and floor around the character**/
simulated function SpawnABloodTrail_GibExplode_360()
{
	GearPawnFX.SpawnABloodTrail_GibExplode_360();
}

/** This will leave a trail of blood on the ground when you chainsaw someone **/
simulated function SpawnABloodTrail_ChainsawSpray_Ground()
{
	GearPawnFX.SpawnABloodTrail_ChainsawSpray_Ground();
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated function SpawnABloodTrail_ChainsawSpray_Wall()
{
	GearPawnFX.SpawnABloodTrail_ChainsawSpray_Wall();
}

/** This will leave a trail of blood on the wall when you chainsaw someone **/
simulated function SpawnABloodTrail_GibImpact( vector HitLoc )
{
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, HitLoc, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		GearPawnFX.SpawnABloodTrail_GibImpact( HitLoc );
	}
}

/** This will leave a trail of blood from your headshot stump **/
simulated function SpawnABloodTrail_HeadShot()
{
	GearPawnFX.SpawnABloodTrail_HeadShot();
}

/** This will leave a small splat of blood **/
simulated function SpawnABloodTrail_HitByABullet()
{
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 1000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistance ) )
	{
		GearPawnFX.SpawnABloodTrail_HitByABullet();
	}
}

/** This will leave some blood as we are really hurt **/
simulated function SpawnABloodTrail_PawnIsReallyHurt()
{
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 2000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		GearPawnFX.SpawnABloodTrail_PawnIsReallyHurt();
	}

}

/** This will leave some blood when one of our limbs breaks **/
simulated function SpawnABloodTrail_LimbBreak( vector InStartLocation )
{
	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, InStartLocation, 1000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		GearPawnFX.SpawnABloodTrail_LimbBreak( InStartLocation );
	}
}

/**
 * This is our function which will "remove" decals from our pawn.
 * We don't actually remove anything as the decal will do that itself.  But we need to keep track
 * of how many we have attached.
 * @todo: refactor this.  (works for bennu demo tho!)
 **/
simulated final function RemoveDecalsFromSelf()
{
	local int Idx;

	for( Idx = 0; Idx < MAX_DECALS_ATTACHED; ++Idx )
	{
		if( (Worldinfo.TimeSeconds - DecalsAttached[Idx]) > 15.0f )
		{
			NumDecalsAttachedCurr--;
		}
	}

	if( NumDecalsAttachedCurr != 0 )
	{
		SetTimer( 2.0f, FALSE, nameof(RemoveDecalsFromSelf) );
	}
}


exec function TestReaverAttack()
{
	local Vehicle_Reaver_Base R;

	foreach WorldInfo.AllPawns(class'Vehicle_Reaver_Base', R)
	{
		R.LegAttackRaise(Location, FRand() < 0.5);
	}
}

exec function TestHeatMat(float HeatVal)
{
	HeatSkin(HeatVal);
}

exec function TestCharMat(float CharVal)
{
	CharSkin(CharVal);
}

/** Heats up the skin by the given amount.  Skin heat constantly diminishes in Tick(). */
simulated function HeatSkin(float HeatIncrement)
{
	CurrentSkinHeat += HeatIncrement;
	CurrentSkinHeat = FClamp(CurrentSkinHeat, 0.f, 1.f);
	MIC_PawnMat.SetScalarParameterValue('Heat', CurrentSkinHeat);

	if( MIC_PawnHair != none )
	{
		MIC_PawnHair.SetScalarParameterValue('Heat', CurrentSkinHeat);
	}

	if (HeatIncrement > 0.f)
	{
		bSkipHeatFade = TRUE;
		SetTimer(SkinHeatFadeDelay, FALSE, nameof(SkinHeatFadeDelayExpired));
	}
}

/** Char skin to the given absolute char amount [0..1].  Charring does not diminish over time. */
simulated function CharSkin(float CharIncrement)
{
	CurrentSkinChar += CharIncrement;
	CurrentSkinChar = FClamp(CurrentSkinChar, 0.f, 1.f);
	MIC_PawnMat.SetScalarParameterValue('Burn', CurrentSkinChar);

	if( MIC_PawnHair != none )
	{
		MIC_PawnHair.SetScalarParameterValue('Burn', CurrentSkinChar);
	}

	if (CharIncrement > 0.f)
	{
		bSkipCharFade = TRUE;
	}
}

final simulated function SkinHeatFadeDelayExpired()
{
	bSkipHeatFade = FALSE;
}

final simulated function FadeSkinEffects(float DeltaTime)
{
	local float KantusFadeDelta;
	// fade skin heat
	if ( (CurrentSkinHeat > CurrentSkinHeatMin) && !bSkipHeatFade && (SkinHeatFadeTime > 0.f) )
	{
		HeatSkin(-DeltaTime / SkinHeatFadeTime);
	}
//	bSkipHeatFade = FALSE;


	// fade charring
	if ( (CurrentSkinChar > CurrentSkinCharMin) && !bSkipCharFade && (SkinCharFadeTime > 0.f) )
	{
		CharSkin(-DeltaTime / SkinCharFadeTime);
	}
	bSkipCharFade = FALSE;


	// kantus revive skin effect
	if( DesiredKantusFadeVal != CurrentKantusFadeVal )
	{
		KantusFadeDelta = DesiredKantusFadeVal - CurrentKantusFadeVal;
		KantusFadeDelta = KantusFadeDelta / abs(KantusFadeDelta);
		CurrentKantusFadeVal = fclamp(CurrentKantusFadeVal + KantusFadeDelta * DeltaTime/KantusSkinEffectFadeDuration,0.f,1.f);
		MIC_PawnMat.SetScalarParameterValue('ResValue',CurrentKantusFadeVal);

		if(DesiredKantusFadeVal>=1.0f && KantusRevivePSC == none && KantusReviver != none)
		{
			KantusReviver.DoKantusReviveEffects(self);
		}
		if(DesiredKantusFadeVal <= 0.f && KantusRevivePSC != none)
		{
			KantusRevivePSC.DeactivateSystem();
			DetachComponent(KantusRevivePSC);
			KantusRevivePSC=none;
		}
	}
}



/** Notification forwarded from RB_BodyInstance, when a spring is over extended and disabled. */
simulated event OnRigidBodySpringOverextension(RB_BodyInstance BodyInstance)
{
	Super.OnRigidBodySpringOverextension(BodyInstance);

	// Forward notification to special move.
	if( SpecialMove != SM_None )
	{
		SpecialMoves[SpecialMove].OnRigidBodySpringOverextension(BodyInstance);
	}
}

simulated function ExitedCameraVolume(CameraVolume Volume)
{
	CameraVolumes.RemoveItem(Volume);
}

simulated function EnteredCameraVolume(CameraVolume Volume)
{
	// store which camera volume we're in.  camera will read this and react accordingly.
	if (CameraVolumes.Find(Volume) == INDEX_NONE)
	{
		CameraVolumes.AddItem(Volume);

		if (IsDoingSpecialMove(SM_RoadieRun))
		{
			EndSpecialMove(SM_RoadieRun);
		}
	}
}

static function MutatePawn(GearPawn Victim)
{
	local int Idx;

	Victim.MutatedClass = default.Class;

	// Detach weapon from right hand
	Victim.MyGearWeapon.DetachWeapon();

	Victim.GoreSkeletalMesh = default.GoreSkeletalMesh;
	Victim.GorePhysicsAsset = default.GorePhysicsAsset;

	if( Victim.Mesh.SkeletalMesh != default.Mesh.SkeletalMesh )
	{
		Victim.Mesh.SetSkeletalMesh(default.Mesh.SkeletalMesh);
	}
	if( Victim.Mesh.PhysicsAsset != default.Mesh.PhysicsAsset )
	{
		Victim.Mesh.SetPhysicsAsset(default.Mesh.PhysicsAsset);
	}
	// Replace AnimTree only if it is different, because it will reset the animation state.
	if( Victim.Mesh.AnimTreeTemplate != default.Mesh.AnimTreeTemplate )
	{
		if( Victim.IsDoingASpecialMove() )
		{
			`warn("mutating" @ Victim.class @ "to" @ default.class @ "while doing special move:" @ Victim.SpecialMove @ "that could be bad! Aborting special move!");
			Victim.EndSpecialMove();
		}

		Victim.Mesh.SetAnimTreeTemplate(default.Mesh.AnimTreeTemplate);
		Victim.ClearAnimNodes();
		Victim.CacheAnimNodes();
	}

	for (Idx = 0; Idx < default.Mesh.GetNumElements(); Idx++)
	{
		Victim.Mesh.SetMaterial(Idx,default.Mesh.GetMaterial(Idx));
	}

	// Reattach weapon to right hand.
	Victim.AttachWeapon();
}

function OnMutate(SeqAct_Mutate Action)
{
	if (Action.PawnClass != None)
	{
		Action.PawnClass.static.MutatePawn(self);
	}
}

/** @return whether this Pawn is currently taking damage due to a pain volume it is touching */
function bool IsInPainVolume()
{
	local PhysicsVolume V;

	foreach TouchingActors(class'PhysicsVolume', V)
	{
		if (V.bPainCausing && V.DamagePerSec > 0.0)
		{
			return true;
		}
	}

	return false;
}

/** Splash some chainaw blood on the screen! **/
event SplashChainsawBloodOnScreen()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_Chainsaw' );
	}
}

/** Splash some blood on the screen! **/
event SplashPunchFaceBloodOnScreen()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
	}
}

/** Splash some blood on the screen! **/
event SplashBloodOnScreen_ExecutionBoltok()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
	}
}

/** Splash some blood on the screen! **/
event SplashBloodOnScreen_PunchFace()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
	}
}

/** Splash some blood on the screen! **/
event SplashBloodOnScreen_SmashFace()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
	}
}

/** Splash some blood on the screen! **/
event SplashBloodOnScreen_TorqueBow()
{
	if (GearPC(Controller) != None)
	{
		GearPC(Controller).ClientSpawnCameraLensEffect( class'Emit_CameraBlood_PunchSplatter' );
	}
}


function ExecutionParticleSpawner( const ParticleSystem PS )
{
	local bool bFoundSpawnLoc;
	local vector Loc;
	local rotator Rot;
	local Emitter AnEmitter;
	//local array<name> BoneNames;

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 1024, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return;
	}

	if( WorldInfo.GRI.ShouldShowGore() == TRUE )
	{
		if( Mesh.GetSocketWorldLocationAndRotation( HeadSocketName, Loc, Rot ) == TRUE )
		{
			bFoundSpawnLoc = TRUE;
		}
		// for now we assume there is always going to be a head socket
		// 	else
		// 	{
		// 		Mesh.GetBoneNames( BoneNames );
		// 		if( BoneNames.Find('b_MF_Spine_01') != INDEX_NONE )
		// 		Loc = Mesh.GetBoneLocation( 'b_MF_Spine_01' );
		// 		bFoundSpawnLoc = TRUE;
		// 	}

		// if we have a spawn loc
		if( bFoundSpawnLoc == TRUE )
		{
			AnEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( PS, Loc, Rot );
			AnEmitter.ParticleSystemComponent.ActivateSystem();
		}
	}
}


event SpawnExecutionEffect_Boltok()
{
	ExecutionParticleSpawner( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Head_Boltok_Execution' );
}

event SpawnExecutionEffect_LongShot()
{
	local rotator Rot;
	local Emitter AnEmitter;

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 1024, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) )
	{
		AnEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Body_Longshot_Execution', Mesh.GetBoneLocation(PelvisBoneName), Rot );
		AnEmitter.ParticleSystemComponent.ActivateSystem();
	}


	GearPawnFX.LeaveADecal( GearPawnFX.BloodDecalTrace_GroundBelowThePawn, GearPawnFX.BloodDecalChoice_GibExplode_Ground, GearPawnFX.BloodDecalTimeVaryingParams_Default );
}

event SpawnExecutionEffect_TorqueBow()
{
	//Execution head explosion and blood spray (duplicate of the headshot blood fountain):
	ExecutionParticleSpawner( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Head_Torquebow_Execution' );
	//Also needs: Blood decals on ground like the headshot spray from the neck
}

event SpawnExecutionEffect_PunchRightToLeft()
{
	ExecutionParticleSpawner( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Head_FacePunch_Bloodspray' );

	GearPawnFX.LeaveADecal( GearPawnFX.BloodDecalTrace_FromNeckForFacePunch, GearPawnFX.BloodDecalChoice_PunchFace, GearPawnFX.BloodDecalTimeVaryingParams_Default );
}

event SpawnExecutionEffect_PunchLeftToRight()
{
	ExecutionParticleSpawner( ParticleSystem'Effects_Gameplay.Blood.P_Blood_Head_FacePunch_Bloodspray' );

	GearPawnFX.LeaveADecal( GearPawnFX.BloodDecalTrace_FromNeckForFacePunch, GearPawnFX.BloodDecalChoice_PunchFace, GearPawnFX.BloodDecalTimeVaryingParams_Default );
}


//Small directional blood sprays to spawn from the face after each punch, ideally we could spawn then in the direction the punch is going (think slomo rocky blood from mouth)

/**
 * Called by AnimNotify_PlayParticleEffect
 * Looks for a socket name first then bone name
 *
 * @param AnimNotifyData The AnimNotify_PlayParticleEffect which will have all of the various params on it
 */
event PlayParticleEffect( const AnimNotify_PlayParticleEffect AnimNotifyData )
{
	local vector Loc;
	local rotator Rot;
	local ParticleSystemComponent PSC;
	local Emitter AnEmitter;

	// if this is extreme content and we can't show extreme content then return
	if( ( AnimNotifyData.bIsExtremeContent == TRUE )
		&& ( WorldInfo.GRI.ShouldShowGore() == FALSE )
		)
	{
		return;
	}

	if( GearGRI(WorldInfo.GRI).IsEffectRelevant( Instigator, Location, 4000, FALSE, GearGRI(WorldInfo.GRI).CheckEffectDistance_SpawnWithinCullDistanceAndInFront ) == FALSE )
	{
		return;
	}

	// find the location
	if( AnimNotifyData.SocketName != '' )
	{
		Mesh.GetSocketWorldLocationAndRotation( AnimNotifyData.SocketName, Loc, Rot );
	}
	else if( AnimNotifyData.BoneName != '' )
	{
		Loc = Mesh.GetBoneLocation( AnimNotifyData.BoneName );
	}


	// now go ahead and spawn the particle system based on whether we need to attach it or not
	if( AnimNotifyData.bAttach == TRUE )
	{
		PSC = GearGRI(WorldInfo.GRI).GOP.GetImpactParticleSystemComponent( AnimNotifyData.PSTemplate );
		PSC.SetLODLevel(GearGRI(WorldInfo.GRI).GetLODLevelToUse( PSC.Template, Loc ) );

		if( AnimNotifyData.SocketName != '' )
		{
			Mesh.AttachComponentToSocket( PSC, AnimNotifyData.SocketName );
		}
		else if( AnimNotifyData.BoneName != '' )
		{
			Mesh.AttachComponent( PSC, AnimNotifyData.BoneName );
		}

		PSC.ActivateSystem();
	}
	else
	{
		AnEmitter = GearGRI(WorldInfo.GRI).GOP.GetImpactEmitter( AnimNotifyData.PSTemplate, Loc, Rot );
		AnEmitter.ParticleSystemComponent.ActivateSystem();
	}

}


simulated function CarriedCrateStatusChanged()
{
	local GearPC GPC;
	local GearWeapon Weap;
	local rotator ViewRot;

	// GRAB CRATE
	if(CarriedCrate != None)
	{
		if(Role == ROLE_Authority || IsLocallyControlled())
		{
			SetPhysics(PHYS_None);
			bCollideWorld = FALSE;
		}

		bLockRotation = TRUE;

		GPC = GearPC(Controller);
		if(GPC != None)
		{
			ViewRot.Yaw = CarriedCrate.Rotation.Yaw;
			GPC.SetRotation(ViewRot);

			GPC.GoToState('PlayerDrivingCrate');
		}

		// Init animation
		ClearAnimNodes();
		Mesh.SetAnimTreeTemplate(AnimTree'COG_MarcusFenix_CrateTree.AT_Marcus_CarryCrate');
		CacheCrateAnimNodes();

		// Turn on mirroring for Dom
		if(GearPawn_COGDom(self) != None)
		{
			CrateMirrorNode.bEnableMirroring = TRUE;
			//GearAnim_AimOffset(AimOffsetNodes[0]).bIsMirrorTransition = TRUE;
			CrateIKRightHand.SetSkelControlActive(TRUE);
			if (GearWeapon(Weapon) != None)
			{
				Weapon.DetachWeapon();
				AttachWeapon();
			}
		}
		else
		{
			CrateIKLeftHand.SetSkelControlActive(TRUE);
		}

		// force the kidnapper to switch to pistols
		// Do this only on locally controlled Pawn. So the new weapon properly replicates to all.
		if( IsLocallyControlled() )
		{
			foreach InvManager.InventoryActors(class'GearWeapon', Weap)
			{
				if( Weap.WeaponType == WT_Holster )
				{
					`log("Forcing carrier to switch to:"@Weap);
					InvManager.SetCurrentWeapon(Weap);
					break;
				}
			}
		}

		bCanDBNO = FALSE;

		if(MyGearAI != none && MyGearAI.CommandList != none)
		{
			MyGearAI.AbortCommand(MyGearAI.CommandList);
		}

	}
	// RELEASE CRATE
	else
	{
		if(Role == ROLE_Authority || IsLocallyControlled())
		{
			SetPhysics(PHYS_Walking);
			bCollideWorld = default.bCollideWorld;
		}

		bLockRotation = FALSE;

		// Finish up anim stuff
		ClearCrateAnimNodes();
		Mesh.SetAnimTreeTemplate(default.Mesh.AnimTreeTemplate);
		CacheAnimNodes();

		// Turn off mirroring for Dom
		if(GearPawn_COGDom(self) != None)
		{
			if (GearWeapon(Weapon) != None)
			{
				Weapon.DetachWeapon();
				AttachWeapon();
			}
		}

		GPC = GearPC(Controller);
		if(GPC != None)
		{
			GPC.GoToState('PlayerWalking');
		}

		bCanDBNO = default.bCanDBNO;
	}
}


/** Hook called from HUD actor. Gives access to HUD and Canvas */
simulated function DrawHUD(HUD H)
{
	Super.DrawHUD(H);

	// Hook for HUD access from SpecialMoves.
	if( SpecialMove != SM_None )
	{
		SpecialMoves[SpecialMove].DrawHUD(H);
	}

	`if(`notdefined(FINAL_RELEASE))
	if( bDebugStoppingPower )
	{
		DrawDebugStoppingPower(H);
	}
	`endif
}

function OnSetPawnSpeed(SeqAct_SetPawnSpeed InAction)
{
	GroundSpeed = InAction.NewSpeed;
}

simulated function bool ShouldTorqueBowArrowGoThroughMe(GearProjectile Proj, TraceHitInfo HitInfo, vector HitLocation, vector Momentum, optional out byte out_bHasHelmet)
{
	return (WorldInfo.GRI == None || !WorldInfo.GRI.OnSameTeam(Proj.Instigator, self) && TookHeadShot(HitInfo.BoneName, HitLocation, Momentum));
}

exec function PlayGUDSAction(EGUDActionID ActionID, optional GearPawn Addressee, optional GearPawn ReferringTo, optional ESpeakLineBroadcastFilter MPBroadcastFilter)
{
	GearGame(WorldInfo.Game).UnscriptedDialogueManager.PlayGUDSAction(ActionID, self, Addressee, ReferringTo, MPBroadcastFilter);
}

simulated function OnPawnMuteGUDS(SeqAct_PawnMuteGUDS Action)
{
	// 0 is mute, 1 is unmute
	bMuteGUDS = Action.InputLinks[0].bHasImpulse;
}

simulated exec event PlaySpeechGesture(Name GestureAnim)
{
	if( BS_IsPlaying(SpeechGestureBodyStance) )
	{
		BS_Stop(SpeechGestureBodyStance, 0.67f);
	}

	if( !IsDoingASpecialMove() && !IsCarryingAHeavyWeapon() && !IsDeployingShield() && !bIsConversing )
	{
		SpeechGestureBodyStance.AnimName[BS_Std_Up] = GestureAnim;
		SpeechGestureBodyStance.AnimName[BS_Std_Idle_Lower] = GestureAnim;
		BS_Play(SpeechGestureBodyStance, 0.85f, 0.67f, 0.67f);
	}
}

simulated function OnGearExitVehicle( SeqAct_GearExitVehicle inAction )
{
	local GearTurret GT;
	local GearVehicle GV;
	if(InAction.TargetVehicle != none)
	{
		GT = GearTurret(InAction.TargetVehicle);
		if(GT != none && GT.Driver == self) // only exit if we are on designated turret
		{
			GT.OnGearExitVehicle(InAction);
		}
		else
		{
			GV = GearVehicle(InAction.TargetVehicle);
			if(GV!=none)
			{
				// reset default command
				if(MyGearAI != none)
				{
					MyGearAI.DefaultCommand = MyGearAI.Default.DefaultCommand;
				}
				GV.OnGearExitVehicle(inAction);
			}
		}
	}
}

/** Stub for subclasses */
simulated function TelegraphAttack();

/** Stub for subclasses */
simulated function TelegraphCharge();

/** Stub for subclasses */
simulated function FlameTankAboutToBlow()
{
	// say "ahhh!"
	if (Health > 0)
	{
		SoundGroup.PlayEffort(self, GearEffort_OnFireDeath,, TRUE);
	}
}

function PathConstraint CreatepathConstraint(class<PathConstraint> ConstraintClass )
{
	return GearGRI(WorldInfo.GRI).GOP.GetPathConstraintFromCache(ConstraintClass,self);
}

function PathGoalEvaluator CreatePathGoalEvaluator(class<PathGoalEvaluator> GoalEvalClass )
{
	return GearGRI(WorldInfo.GRI).GOP.GetPathGoalEvaluatorFromCache(GoalEvalClass,self);
}

/** Stub for subclasses */
simulated function OpenShield( bool bModGroundSpeed, optional float ModGroundSpeed, optional bool bDoSpecialMove );
/** Stub for subclasses */
simulated function CloseShield();

/**
 * Will return the TeamNum of the weapon owner based on class.  So for the MP Classes they will have correct class based colors
 * Additionally, the TeamNum is getting set WAY after the weapons have been attached so TeamNum() doesn't really work at getting
 * the correct colors set.
 **/
simulated function byte GetMPWeaponColorBasedOnClass();

simulated function OnSetPawnBaseHealth(SeqAct_SetPawnBaseHealth Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		DefaultHealth = Action.NewHealth;
		Health = Action.NewHealth;
	}
}

simulated function bool ShouldTurnCursorRedFor(GearPC PC)
{
	return (GetTeamNum() != 254 && !WorldInfo.GRI.OnSameTeam(PC, self));
}

function int GetMaxDownCount()
{
	if(!WorldInfo.GRI.IsMultiPlayerGame())
	{
		return -1; // inf downs in SP
	}
	return MaxDownCount;
}

function float GetAIAccuracyModWhenTargetingMe(GearAI GuyTargetingMe)
{
	return 0.f;
}

event bool IsFiringHOD()
{
	return ((Weapon != None) && Weapon.IsA('GearWeap_HOD') && Weapon.IsFiring());
}


/** This is a debug function to verify that none OF the pawns have incorrect bones in their break lists **/
simulated function CheckGoreJoints()
{
	local int HostageIdx;
	local name HostageBoneName;
	local int BreakableIdx;
	local bool bFound;

	for( HostageIdx = 0; HostageIdx < HostageHealthBuckets.length; ++HostageIdx )
	{
		HostageBoneName = HostageHealthBuckets[HostageIdx];

		// look for the HostageName
		for( BreakableIdx = 0; BreakableIdx < GoreBreakableJoints.length; ++BreakableIdx )
		{
			if( HostageBoneName == GoreBreakableJoints[BreakableIdx] )
			{
				bFound = TRUE;
				break;
			}
		}

		if( bFound == FALSE )
		{
			`warn( "Found Bone in HostageBreak list that was not in GoreBreakableJoints list!!!!" );
			`log( "Bone: " $ HostageBoneName );
		}

		// reset for next round
		bFound = FALSE;
	}

}

/** Toggle MeatShieldMorph */
simulated function SetActiveMeatShieldMorph(bool bInActivate)
{
	bActivateMeatShieldMorph = bInActivate;
	MeatShieldMorphTimeToGo = MeatShieldMorphBlendTime;
}

/**
 * Returns location to use when testing if this pawn was hit by a melee attack.  Useful to override, e.g., for
 * small creatures close to the ground.
 */
simulated function vector GetMeleeHitTestLocation()
{
	return Location;
}

/** Notification called when one of our meshes gets his AnimTree updated */
simulated event AnimTreeUpdated(SkeletalMeshComponent SkelMesh)
{
	`LogInv(SkelMesh.AnimTreeTemplate);
	if( MyGearWeapon != None && SkelMesh == MyGearWeapon.Mesh )
	{
		MyGearWeapon.UpdateAnimNodeReferences();
	}

	Super.AnimTreeUpdated(SkelMesh);
}

defaultproperties
{
	MeatShieldMorphBlendTime=0.4f
	EffectCount=0
	bAlwaysRelevant=TRUE  // we want to make our pawns always relevant as our levels are small enough and it solves the issues of being shot by a non relevant pawn where you still take damage but don't get a damage locator

	HeadSocketName="Head"
	RightHandSocketName="RightHand"
	LeftHandSocketName="LeftHand"

	PelvisBoneName="b_MF_Pelvis"
	PickupFocusBoneName="b_MF_Weapon_L_End"
	PickupFocusBoneNameKickup="b_MF_Weapon_R_End"

	FullBodyNodeName="Custom_FullBody"

	CharacterFootStepType=CFST_Generic

	AttachSlots(EASlot_Holster)=(SocketName="Holster")
	AttachSlots(EASlot_Belt)=(SocketName="Belt1")
	AttachSlots(EASlot_LeftShoulder)=(SocketName="LeftBackMount")
	AttachSlots(EASlot_RightShoulder)=(SocketName="RightBackMount")

	ControllerClass=class'GearAI'
	InventoryManagerClass=class'GearInventoryManager'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_Empty'
	MeleeRange=+20.0
	bMuffledHearing=TRUE

	bBlockActors=TRUE
	bBlocksNavigation=TRUE

	ViewPitchMin=-12000
	ViewPitchMax=10000

	Buoyancy=+000.99000000
	UnderWaterTime=+00020.000000
	BaseEyeHeight=+00060.000000
	EyeHeight=+00060.000000
	CrouchHeight=+64.0
	CrouchRadius=+34.0
	GroundSpeed=+00300.000000
	AirSpeed=+00600.000000
	WaterSpeed=+00300.000000
	AccelRate=+02048.000000
	JumpZ=+00540.000000
	bCanStrafe=TRUE
	bCanSwim=TRUE
	RotationRate=(Pitch=50000,Yaw=50000,Roll=50000)
	AirControl=+0.35
	bStasis=FALSE
	bCanCrouch=FALSE

	bJumpCapable=FALSE
	bCanJump=FALSE
	bStopAtLedges=TRUE
	bAvoidLedges=TRUE
	bCanDoStepsSmoothing=TRUE
	bCanClimbLadders=TRUE
	bCanPickupInventory=TRUE
	bCanPickupFactoryWeapons=TRUE
	SightRadius=+12000.0

	DurationBeforeDestroyingDueToNotBeingSeen=8.0

	Components.Remove(Sprite)

	Begin Object Name=CollisionCylinder
		BlockZeroExtent=FALSE
	End Object

	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		LightShadowMode=LightShadow_Modulate
		bSynthesizeSHLight=TRUE
		ModShadowFadeoutTime=0.75f
	End Object
	LightEnvironment=MyLightEnvironment
	Components.Add(MyLightEnvironment)

	Begin Object Class=SkeletalMeshComponent Name=GearPawnMesh
		BlockZeroExtent=TRUE
		//BlockNonZeroExtent=TRUE
		CollideActors=TRUE
		BlockRigidBody=TRUE
		RBChannel=RBCC_Pawn
		RBCollideWithChannels=(Default=TRUE,Pawn=TRUE,DeadPawn=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,FracturedMeshPart=TRUE,SoftBody=TRUE)
		bIgnoreControllersWhenNotRendered=TRUE
		LightEnvironment=MyLightEnvironment
		MinDistFactorForKinematicUpdate=0.2
		bAcceptsStaticDecals=FALSE
		bAcceptsDynamicDecals=TRUE
		bCullModulatedShadowOnBackfaces=FALSE
		bAllowAmbientOcclusion=FALSE
		MotionBlurScale=0.0
	End Object
	Mesh=GearPawnMesh
	Components.Add(GearPawnMesh)

	bTranslateMeshByCollisionHeight=TRUE
	MeshTranslationNudgeOffset=3.f

	WalkingPct=+0.35
	MovementPct=1.0
	CoverMovementPct=(0.f,1.0f,1.0f)
	CoverProtectionFOV=(X=60,Y=30)

	MaxStepHeight=40.f

	LastCoverTime=-999.f
	LastCoverActionTime=-999.f
	NextCoverActionTime=0.f // KEEP default as ZERO!

	// Animation
	bUsesBodyStances=TRUE
	bCanDo360AimingInCover=TRUE
	bCanDoRun2Cover=TRUE
	bCanPlayHeadShotDeath=FALSE
	bPlayDeathAnimations=FALSE
	bTargetingNodeIsInIdleChannel=FALSE

//	bRevive=true
	bNoEncroachCheck=TRUE

//	PathSearchType=PST_NewBestPathTo
	PathSearchType=PST_Constraint

	SpecialMoveClasses(SM_DeathAnim)=class'GSM_DeathAnim'

	LastShotAtTime=-9999.f
	LastMeleeAttackTime=-9999.f
	LastWeaponStartFireTime=-9999.f
	LastWeaponEndFireTime=-9999.f
	ForcedAimTime=-9999.f

	Run2CoverCameraShake=(FOVAmplitude=2,FOVFrequency=5,LocAmplitude=(X=4,Y=2,Z=2),LocFrequency=(X=30,Y=20,Z=20),RotAmplitude=(X=50,Y=50,Z=120),RotFrequency=(X=60,Y=0,Z=205),TimeDuration=0.6)
	Run2CoverMaxCameraShakeDistSqThreshold=16000
	Run2CoverMinCameraShakeDistSqThreshold=1000

	CoverActionSpecialMoves=(SM_MantleUpLowCover,SM_CoverSlip,SM_MidLvlJumpOver,SM_StdLvlSwatTurn,SM_EvadeFromCoverCrouching,SM_EvadeFromCoverStanding,SM_PushObject)
	ActionSpecialMoves=(SM_ReviveTeammate,SM_LadderClimbDown,SM_LadderClimbUp)

	// Physics Hit Reaction
	PhysicsImpactBlendOutTime=0.45f
	PhysicsHitReactionImpulseScale=1.f
	PhysicsImpactMassEffectScale=0.9f
	PhysHRMotorStrength=(X=5000,Y=0)
	PhysHRSpringStrength=(X=5,Y=5)
	bEnableHitReactionBoneSprings=TRUE
	bCanPlayPhysicsHitReactions=TRUE

	PawnRotationInterpolationTime=0.15f
	bCanDoTurnInPlaceAnim=TRUE

	LoadedGUDBank=-1

	Begin Object Class=AudioComponent Name=FaceAudioComponent
	End Object
	FacialAudioComp=FaceAudioComponent
	Components.Add(FaceAudioComponent)

	NeedsRevivedGUDSEvent=GUDEvent_None
	WentDownGUDSEvent=GUDEvent_None
	NoticedGUDSPriority=1
	NoticedGUDSEvent=GUDEvent_NoticedEnemyGeneric

	BS_Junker_Driving_Idle=(AnimName[BS_FullBody]="Driver_Idle")
	BS_Junker_Passenger_A_Idle=(AnimName[BS_FullBody]="Passenger_A_Idle")
	BS_Junker_Passenger_B_Idle=(AnimName[BS_FullBody]="Passenger_B_Idle")

	BS_HeadCoverLoop={(
		AnimName[BS_Std_Up]				="AR_Injured_Headcover1_Idle",
		AnimName[BS_CovStdIdle_Up]		="AR_Injured_Headcover1_Idle",
		AnimName[BS_CovMidIdle_Up]		="AR_Injured_Headcover1_Idle"
	)}

	HeadChunks(00)=StaticMesh'COG_Gore.Head_Chunk11' // this is the head
	HeadChunks(01)=StaticMesh'COG_Gore.Head_Chunk2'
	HeadChunks(02)=StaticMesh'COG_Gore.Head_Chunk3'
	HeadChunks(03)=StaticMesh'COG_Gore.Head_Chunk4'
	HeadChunks(04)=StaticMesh'COG_Gore.Head_Chunk5'
	HeadChunks(05)=StaticMesh'COG_Gore.Head_Chunk6'
	HeadChunks(06)=StaticMesh'COG_Gore.Head_Chunk7'
	HeadChunks(07)=StaticMesh'COG_Gore.Head_Chunk8'
	HeadChunks(08)=StaticMesh'COG_Gore.Head_Chunk9'
	HeadChunks(09)=StaticMesh'COG_Gore.Head_Chunk10'
	HeadChunks(10)=StaticMesh'COG_Gore.Head_Chunk1'
	HeadChunks(11)=StaticMesh'COG_Gore.Head_Chunk12'
	HeadChunks(12)=StaticMesh'COG_Gore.Head_Chunk13'
	HeadChunks(13)=StaticMesh'COG_Gore.Head_Chunk14'
	HeadChunks(14)=StaticMesh'COG_Gore.Head_Chunk15'

	GibKillDistance=2048.0f

	SuperDamageMultiplier=1.f

	PS_RoadieRun=ParticleSystem'War_Level_Effects2.Smoke.P_Player_RoadieRun_Dust'
	PS_BurnEffect=ParticleSystem'COG_HOD.Effects.P_HOD_Burn'

	LastTeamNum=255
	GUDLineRepeatMin=30.0f
	MinTimeBetweenAnyGUDS=4.f

	CameraNoRenderCylinder_High_ViewTarget=(Radius=40,Height=86)
	CameraNoRenderCylinder_Low_ViewTarget=(Radius=40,Height=36)
	CameraNoRenderCylinder_High=(Radius=40,Height=88)
	CameraNoRenderCylinder_Low=(Radius=40,Height=44)
	CameraNoRenderCylinder_FlickerBuffer=(Radius=4,Height=4)
	bBlockCamera=TRUE

	CrouchedFrictionCylinder=(Radius=64.f,Height=40.f)

	bKillDuringLevelTransition=TRUE
	HealthRechargeStartSound=SoundCue'Interface_Audio.Interface.BurstDamageStop01Cue'

	MPHeadOffset=(X=3,Y=-2,Z=0)
	MPHeadRadius=11

	DBNO_BloodCrawlSpurtTemplate=ParticleSystem'Effects_Gameplay.Blood.P_Blood_DBNO_Crawl'
	GoreImpactParticle=ParticleSystem'Effects_Gameplay.Blood.P_Blood_groundimpact_bodypart'
	bShowFatalDeathBlood=TRUE

	GibEffectsSpeedThreshold=125.0
	TimeBetweenGibEffects=0.25
	DeadBodyImpactThreshold = 10.0;

	LastCringeTime=-999999
	LastCoverGuessTime=-99999

	bReplicateRigidBodyLocation=TRUE

	OnFireBlazingParticleSystem=ParticleSystem'FX_Flamethrower.Particles.P_FX_Flamethrower_BodyBurn_01'
	OnFireBlazingBurningSound=SoundCue'Weapon_Grenade.Weapons.PropaneGasFireCue'
	OnFireSmolderParticleSystem=ParticleSystem'FX_Flamethrower.Particles.P_FX_Flamethrower_BodyBurn_01'
	OnFireSmolderingBurningSound=SoundCue'Vehicle_APC.Vehicles.VehicleSurfaceTireDirtCue'

	ImulsionParticleSystem=ParticleSystem'FX_Flamethrower.Particles.P_FX_Flamethrower_BodyBurn_01'
	ImulsionBurningSound=SoundCue'Ambient_Loops.Water_G2.water_waterfalllarge01Cue'

	SimpleDeathEffect=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Crowd_Gib_Medium'
	SimpleDeathEffectNonGore=ParticleSystem'Effects_Gameplay.Blood.P_Blood_Crowd_Gib_Medium_NoGore'
	SimpleDeathEffectScale=1.0

	bAllowDamageModification=TRUE
	bCanRecoverFromDBNO=TRUE
	bModifyReachSpecCost=TRUE
	bRespondToExplosions=TRUE
	bCanBeRunOver=TRUE

	bAllowInventoryDrops=TRUE
	bAllowHitImpactDecalsOnSkelMesh=TRUE
	bSpawnHitEffectDecals=TRUE
	bSpawnBloodTrailDecals=TRUE
	NumConstraintEffectsSpawnedMax=4

	bDisableDeathScreams=FALSE

	MaxFallSpeed=+10000.0
	AIMaxFallSpeedFactor=1.0

	SkinHeatFadeTime=0.4f
	SkinHeatFadeDelay=0.3f
	SkinHeatDamagePctToFullyHeat=0.3f
	CurrentSkinHeatMin=0.0f
	SkinCharFadeTime=1.f

	SightBoneName="LookingDirection"

	GearPawnFXClass=class'GearPawnFX'

	bAllowTurnSmoothing=TRUE
	TurningRadius=64.f
	AccelConvergeFalloffDistance=400.f
	bAllowAccelSmoothing=TRUE

	GoreBreakableJointsTest=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_L","b_MF_UpperArm_R","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_L","b_MF_Foot_R")
	bAllowAutoRevive=true

	MeshFloorConformTransSpeed=20.0
	MeshFloorConformRotSpeed=30.0
	SpeechPitchMultiplier=1.f

	ShootingThroughWallTraceDistance=1000.f

	HordeEnemyIndex=INDEX_NONE

	ProhibitedFindAnchorPhysicsModes=(PHYS_Flying,PHYS_Interpolating)
}
