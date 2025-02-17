/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_AIFactory extends SequenceAction
	native(Sequence);

cpptext
{
	virtual void PostLoad();

	virtual void PostEditChange( UProperty* PropertyThatChanged );

	/**
	 * This will set the hard references for this Factory.  We need to only have the enums be by name
	 * and then when the user chooses what to spawn then we set the reference so we don't have
	 * to have in memory every possible thing we can spawn (due to the hard references).
	 **/
	void SetTheHardReferences();

	virtual void UpdateDynamicLinks();

	virtual void Activated();
	virtual UBOOL UpdateOp(FLOAT DeltaTime);
	virtual void DeActivated();
	void ResetSpawnSets();
	void UpdateSpawnSets(FLOAT DeltaTime);
	AActor* PickSpawnPoint(FVector& SpawnExtent, FVector& SpawnLocation, FRotator& SpawnRotation, AGearSpawner** Spawner, INT& SpawnerSlotIdx, INT& SpawnPointIdx, UBOOL bSkipCollisionCheck = FALSE);

	/** If we are using a DoNotUse enum then we need to set it to a valid enum **/
	void RepairAITypeSetToDoNotUseEnum();

	/** Remove dead pawns from the watchlist */
	void RemoveDeadSpawnsFromWatchList();

	/**
	 * Called when a spawn is killed.  Increments the Death counter, removes the spawn from the watch list of the set
	 * and decrements the number of spawns to kill in the GRI.
	 * @SpawnSet - the spawn set of the spawn who died
	 * @WatchListIdx - the index in the WatchList array of the SpawnSet of the spawn who died
	 */
	void SpawnHasDied( FAISpawnInfo &SpawnSet, INT WatchListIdx );
}

enum EAITypes
{
	// !!!! add NEW entries at the end !!!!
	AIType_Drone,
	AIType_Wretch,
	AIType_Boomer,
	AIType_Berserker,
	AIType_Nemacyst,
	AIType_DarkWretch,
	AIType_Theron,
	AIType_Dom,
	AIType_Baird,
	AIType_Gus,
	AIType_Minh,
	AIType_Redshirt,
	AIType_UNUSED,
	AIType_Skorge,
	AIType_DroneGrenadier,
	AIType_DroneSniper,
	AIType_HunterArmorNoGrenades,
	AIType_HunterNoArmorNoGrenades,
	AIType_Hoffman,
	AIType_TroikaGunner,
	AIType_TroikaSpotter,
	AIType_Carmine,
	AIType_NPCFranklin,
	// !!!! add NEW entries at the end !!!!
	AIType_NPCMaleWithHatAndBag,
	AIType_NPCMaleOld,
	AIType_NPCGirl,
	AIType_NPCMaleWithDoorag,
	AIType_NPCBoy,
	AIType_NPCWomanWithBun,
	AIType_NPCFemaleWithBackpackOfCans,
	AIType_NPCMaleWithHeadband,
	AIType_ReaverWithDrone,
	AIType_ReaverWithTheron,
	AIType_ReaverLowHealth,
	AIType_ReaverBig,
	AIType_NPCCOGMaleWithHatAndBag,
	AIType_Kantus,
	AIType_NPCCOGFranklinWithNoDreads,
	AIType_NPCCOGDrunkMan,
	AIType_NPCCOGManWithBandana,
	// !!!! add NEW entries at the end !!!!
	AIType_NPCCOGOldMan,
	AIType_Gus_Resonator,
	AIType_FriendlyWretch,	// Test for SynchedAnimations
	AIType_Lynx,
	AIType_Brumak,
	AIType_BoomerFlail,
	AIType_BoomerGatling,
	AIType_FlameDrone,
	AIType_PalaceGuard,
	AIType_BoomerFlame,
	AIType_ChiggerBomb,
	AIType_BloodMountWithDrone,
	AIType_RockWorm,
	// !!!! add NEW entries at the end !!!!
	AIType_NemaSlug,
	AIType_Sire,
	AITYpe_SecurityNemacyst,
	AIType_FlyingSecurityBot,
	AIType_Jack,
	AIType_TaiKaliso,
	AIType_BenjaminCarmine,
	AIType_NilesSamson,
	AIType_RandyBraash,
	AIType_Dizzy,
	AIType_LocustBeastRider,
	// !!!! add NEW entries at the end !!!!
	AIType_LocustSpeedy,
	AIType_UNUSED2,
	AIType_LocustLancerGuard,
	AIType_Brumak_Big,
	AIType_LocustGrapplingDrone,
	AIType_BloodMountRiderless,
	AIType_DroneNoHelmet,
	AIType_BoomerButcher,
	AIType_BoomerMechanic,
	AIType_BloodMountWithPalaceGuard,
	AIType_DroneMortar,
	AIType_Brumak_Kadar,
	AIType_ReaverNoGunner,
	AIType_Brumak_ResistSmallArms,
	// !!!! add NEW entries below here !!!!
};

enum EInventoryTypes
{
	WEAP_Lancer,
	WEAP_Gnasher,
	WEAP_Longshot,
	WEAP_Hammerburst,
	WEAP_Boltock,
	WEAP_Boomshot,
	WEAP_Snub,
	WEAP_TorqueBow,
	WEAP_HOD,
	WEAP_Mortar,
	WEAP_Minigun,
	WEAP_FragGrenade,
};

var const array<Name> InventoryTypeNames;

/**
 * Contains info about an AI type to spawn.
 */
struct native AISpawnInfo
{
	/** What type of AI should be spawned? */
	var() EAITypes AIType<Sorted>;

	/** How many should be spawned of this type? */
	var() int SpawnTotal;

	/** How many should be alive at any one point? */
	var() int MaxAlive;

	/** Total number that have been spawned from this set */
	var int SpawnedCount;

	/** List of currently spawned */
	var array<Pawn> WatchList;

	/** Delay between spawns */
	var() float MaxSpawnDelay, MinSpawnDelay;

	/** Current delay before allowed to try spawning again */
	var float CurrentDelay;

	/**
	 * What inventory items should be given to the AI.
	 * This can not be named "LoadOut" or "LoadOutNames" until content has been resaved to
	 * remove maps with the older AIFactories which have those named properties
	 */
	var() array<EInventoryTypes> WeaponLoadOut;

	/** List of variable links associated with this set */
	var array<string> VarLinkDescs;

	/** List of output links associated with this set */
	var array<string> OutLinkDescs;

	/** AI in the spawning process (ex. crawling out of a hole) */
	var array<Pawn> CurrentSpawns;

	/** Use the move targets? */
	var() bool bAssignMoveTarget;

	/** Use the cover groups? */
	var() bool bAssignCombatZones;

	// these are private vars which we store the actual hard reference to the
	// classes we need for this spawn
	var class<Pawn> GearPawnClass;
	var class<AIController> ControllerClass;
	var array<class<Inventory> > LoadoutClasses;

	/** Should this AI drop inventory on death? */
	var() bool bAllowInventoryDrops;

	/** Should this AI allow hit impact decals on its skel mesh**/
	var() bool bAllowHitImpactDecalsOnSkelMesh;

	/** Should this AI spawn hit effect decals **/
	var() bool bSpawnHitEffectDecals;

	/** Should this AI spawn blood trail decals **/
	var() bool bSpawnBloodTrailDecals;

	/** Squad this AI should join */
	var() Name SquadName;
	var() bool bSquadLeader;
	/** Actor Tag to apply to spawned AI */
	var() Name ActorTag;

	/** Automatically acquire the nearest enemy after spawning */
	var() bool bAutoAcquireEnemy;
	/** Automatically notify enemies of spawn event */
	var() bool bAutoNotifyEnemy;

	/** Spawn only if coop */
	var() bool bSpawnOnlyIfCoop;

    var const int SetVersion;

	/** String displayed above any AI spawned from this set */
	var() string AutoDebugText;

	/** turn this on to disable this pawn from ever going DBNO */
	var() bool bDontAllowDBNO;

	/** turn this on to allow this pawn to be removed when it hasn't done anything useful in a long time */
	var() bool bAllowDeleteWhenStale;

	/** Shadow mode to set on the spawned pawn */
	var() ELightShadowMode ShadowMode;

	/** Chance AI should play an initial reaction animation when they first spot enemies */
	var() float PlayIntialCombatReactionChance;

	/** whether or not to disable shadow casting on this pawn */
	var() bool bDisableShadowCasting;

	/** Whether or not this pawn should play the death scream. (i.e. aerial combat or mobs that are far away don't need this on at all)**/
	var() bool bDisableDeathScreams;

	/** Should this player always appear in TacCom for its team regardless of squad? */
	var() bool bForceShowInTaccom;

	structcpptext
	{
		/** Constructors */
		FAISpawnInfo() {}
		FAISpawnInfo(EEventParm)
		{
			appMemzero(this, sizeof(FAISpawnInfo));
		}
		void InitToDefaults()
		{
			appMemzero(this, sizeof(FAISpawnInfo));
			bAllowDeleteWhenStale=FALSE;
			SpawnTotal=1;
			MaxAlive=1;
			bAssignMoveTarget=TRUE;
			bAssignCombatZones=TRUE;
			bAllowInventoryDrops=TRUE;
			bAllowHitImpactDecalsOnSkelMesh=TRUE;
			bSpawnHitEffectDecals=TRUE;
			bSpawnBloodTrailDecals=TRUE;
			bAutoAcquireEnemy=FALSE;
			bAutoNotifyEnemy=FALSE;
			SquadName=FName(TEXT("Alpha"));
			ShadowMode=LightShadow_Modulate;
			bDisableShadowCasting=FALSE;
			bDisableDeathScreams=FALSE;
			PlayIntialCombatReactionChance=0.5;
		}
		FAISpawnInfo(ENativeConstructor)
		{
			InitToDefaults();
		}
	}

	structdefaultproperties
	{
		bAllowDeleteWhenStale=FALSE
		SpawnTotal=1
		MaxAlive=1
		bAssignMoveTarget=TRUE
		bAssignCombatZones=TRUE
		bAllowInventoryDrops=TRUE
		bAllowHitImpactDecalsOnSkelMesh=TRUE
		bSpawnHitEffectDecals=TRUE
		bSpawnBloodTrailDecals=TRUE
		bAutoAcquireEnemy=FALSE
		bAutoNotifyEnemy=FALSE
		SquadName=Alpha
		ShadowMode=LightShadow_Modulate
		bDisableShadowCasting=FALSE
		PlayIntialCombatReactionChance=0.5
	}
};

/** List of groups of AIs to spawn */
var(Spawn) array<AISpawnInfo> SpawnSets;

/** List of info associated with the AITypes enum used for spawning */
struct native AITypeInfo
{
	var name ControllerClassName;
	var name PawnClassName;
	var int TeamIdx;
	var bool bUnique;
};
var const array<AITypeInfo> SpawnInfo;

/** List of points to spawn AI at */
var(Spawn) array<Actor> SpawnPoints;

/** Have we spawned everything? */
var bool bAllSpawned;

/** Has everything we spawned died? */
var bool bAllDead;

/** Should we prevent any further spawns? */
var bool bAbortSpawns;

/** Should we suppress the AllDead output? */
var bool bSuppressAllDead;

/** Number of dudes dead from this spawner */
var int DeadCount;
/** Number dead in last frame before firing Num Dead output */
var int NumDead;

/** Number of dudes to die before activating matching link */
var() int ActivateDeadLinkCount;

/** Have we activated the dead link? */
var bool bActivatedDeadLink;
/** Reset the dead link count after each firing */
var() bool bResetDeadLinkCount;

/** How are move targets and cover assigned? */
enum EAIAssignment
{
	EA_UniquePerSpawn,		/** Assigned out once per spawn til none are left */
	EA_Shared,				/** Shared across all spawned */
};

/** Move targets to give the spawned AI */
var(Move) EAIAssignment MoveAssignment;
var(Move) array<Actor> MoveTargets;
/** Is the move interruptable? */
var(Move) bool bMoveInterruptable;
/** For use with EA_Shared */
var int MoveTargetIdx;

/** Combat zones to assign */
var(Cover) array<CombatZone> CombatZones;

/** Perception mood to start in */
var() EPerceptionMood	PerceptionMood;
/** Combat mood to start in */
var() ECombatMood		CombatMood;

enum ESpawnPointSelectionMethod
{
	SPSM_Shuffle,
	SPSM_Linear,
};
var() ESpawnPointSelectionMethod SpawnPointSelectionMethod;

/**
 * If a unique character is greater than this distance then
 * teleport them to the spawn point.
 */
var() float TeleportDistance;
var() float TeleportZDistance;
/**
 * If a unique character isn't teleported, then force them
 * to move to the spawn location.
 */
var() bool bForceMoveToSpawnLocation;

/** Used to set the bEnableEncroachCheckOnRagdoll flag on the spawned Pawn. */
var() bool bEnableEncroachCheckOnRagdoll;

/** Use damage for kill all instead of calling died directly */
var() bool bUseDamageForKillAll;

/** Check if we've fixed up the defaults for Auto Acquire/Notify Enemy, remove some day */
var const bool bFixedUpAutoEnemyDefaults;

/**
 * The round(wave) that the spawned characters from this factory will count toward
 * the total enemy count displayed in the HUD. 0 means the factory will not count toward the count.
 */
var() int RoundToCountSpawnsInHUD;

/** Prevents Combat Entered output from firing more than once */
var	bool bEnteredCombat;

native function NotifySpawnerDisabled(GearSpawner Spawner);
native function NotifyCombatEntered();

static native function bool CanSpawnAtLocation(out vector ChkLocation, vector ChkExtent, Actor SpawnPointActor );

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 47;
}

defaultproperties
{
	ObjName="AI Factory"
	ObjCategory="AI"

	TeleportDistance=4096.f
	TeleportZDistance=512.f
	ActivateDeadLinkCount=1

	bLatentExecution=TRUE
	bAutoActivateOutputLinks=FALSE

	InputLinks(0)=(LinkDesc="Spawn")
	InputLinks(1)=(LinkDesc="Abort Spawn")
	InputLinks(2)=(LinkDesc="Kill All")

	OutputLinks(0)=(LinkDesc="All Spawned")
	OutputLinks(1)=(LinkDesc="All Dead")
	OutputLinks(2)=(LinkDesc="Spawns Disabled",bHidden=TRUE)
	OutputLinks(3)=(LinkDesc="Spawned",bHidden=TRUE)
	OutputLinks(4)=(LinkDesc="# Dead",bHidden=TRUE)
	OutputLinks(5)=(LinkDesc="Entered Combat", bHidden=TRUE)

	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Spawn Points",PropertyName=SpawnPoints)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object',LinkDesc="Last Spawned",bHidden=TRUE)
	VariableLinks(2)=(ExpectedType=class'SeqVar_ObjectList',LinkDesc="Spawned",bHidden=TRUE)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Int',LinkDesc="Num Dead",bWriteable=TRUE,bHidden=TRUE,PropertyName=NumDead)

	SpawnInfo(AIType_Drone)=(ControllerClassName="GearAI_LocustDrone",PawnClassName="GearPawn_LocustDrone",TeamIdx=1)
	SpawnInfo(AIType_Boomer)=(ControllerClassName="GearAI_Boomer",PawnClassName="GearPawn_LocustBoomer",TeamIdx=1)
	SpawnInfo(AIType_BoomerFlail)=(ControllerClassName="GearAI_Boomer_Shield",PawnClassName="GearPawn_LocustBoomerFlail",TeamIdx=1)
	SpawnInfo(AIType_BoomerGatling)=(ControllerClassName="GearAI_Boomer_Gatling",PawnClassName="GearPawn_LocustBoomerGatling",TeamIdx=1)
	SpawnInfo(AIType_BoomerFlame)=(ControllerClassName="GearAI_Boomer_Flame",PawnClassName="GearPawn_LocustBoomerFlame",TeamIdx=1)
	SpawnInfo(AIType_BoomerButcher)=(ControllerClassName="GearAI_Boomer_Melee",PawnClassName="GearPawn_LocustBoomerButcher",TeamIdx=1)
	SpawnInfo(AIType_BoomerMechanic)=(ControllerClassName="GearAI_Boomer_Melee",PawnClassName="GearPawn_LocustBoomerMechanic",TeamIdx=1)

	SpawnInfo(AIType_Berserker)=(ControllerClassName="GearAI_Berserker",PawnClassName="GearPawn_LocustBerserker",TeamIdx=1)
	SpawnInfo(AIType_Nemacyst)=(ControllerClassName="GearAI_Nemacyst",PawnClassName="GearPawn_LocustNemacyst",TeamIdx=1)
	SpawnInfo(AIType_Theron)=(ControllerClassName="GearAI_Theron",PawnClassName="GearPawn_LocustTheron",TeamIdx=1)
	SpawnInfo(AIType_PalaceGuard)=(ControllerClassName="GearAI_Theron",PawnClassName="GearPawn_LocustPalaceGuard",TeamIdx=1)
	SpawnInfo(AIType_Kantus)=(ControllerClassName="GearAI_Kantus",PawnClassName="GearPawn_LocustKantus",TeamIdx=1)

	SpawnInfo(AIType_Wretch)=(ControllerClassName="GearAI_Wretch",PawnClassName="GearPawn_LocustWretch",TeamIdx=1)
	SpawnInfo(AIType_DarkWretch)=(ControllerClassName="GearAI_Wretch",PawnClassName="GearPawn_LocustDarkWretch",TeamIdx=1)
	SpawnInfo(AIType_FriendlyWretch)=(ControllerClassName="GearAI_FriendlyWretch",PawnClassName="GearPawn_LocustWretchFriendly",TeamIdx=0)
	SpawnInfo(AIType_ChiggerBomb)=(ControllerClassName="GearAI_Ticker",PawnClassName="GearPawn_LocustTicker",TeamIdx=1)

	SpawnInfo(AIType_Dom)=(ControllerClassName="GearAI_Dom",PawnClassName="GearPawn_COGDom",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Gus)=(ControllerClassName="GearAI_Gus",PawnClassName="GearPawn_COGGus",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Gus_Resonator)=(ControllerClassName="GearAI_Gus",PawnClassName="GearPawn_COGGus_Resonator",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Baird)=(ControllerClassName="GearAI_Baird",PawnClassName="GearPawn_COGBaird",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Dizzy)=(ControllerClassName="GearAI_Dizzy",PawnClassName="GearPawn_COGDizzy",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Minh)=(ControllerClassName="GearAI_Dom",PawnClassName="GearPawn_COGMinh",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_Redshirt)=(ControllerClassName="GearAI_Dom",PawnClassName="GearPawn_COGRedShirt",TeamIdx=0)
	SpawnInfo(AIType_Carmine)=(ControllerClassName="GearAI_Carmine",PawnClassName="GearPawn_COGBenjaminCarmine",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_BenjaminCarmine)=(ControllerClassName="GearAI_Carmine",PawnClassName="GearPawn_COGBenjaminCarmine",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_TaiKaliso)=(ControllerClassName="GearAI_Tai",PawnClassName="GearPawn_COGTai",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_NilesSamson)=(ControllerClassName="GearAI_Niles",PawnClassName="GearPawn_COGNilesSamson",TeamIdx=0,bUnique=TRUE)
	SpawnInfo(AIType_RandyBraash)=(ControllerClassName="GearAI_Randy",PawnClassName="GearPawn_COGRandyBraash",TeamIdx=0,bUnique=TRUE)

	SpawnInfo(AIType_DroneSniper)=(ControllerClassName="GearAI_Locust",PawnClassName="GearPawn_LocustSniper",TeamIdx=1)

	// can modify GearAI_Hunter into different types to have different strategies based on visuals maybe :-)
	SpawnInfo(AIType_DroneGrenadier)=(ControllerClassName="GearAI_Hunter",PawnClassName="GearPawn_LocustHunterArmorNoGrenades",TeamIdx=1)
	SpawnInfo(AIType_HunterArmorNoGrenades)=(ControllerClassName="GearAI_Hunter",PawnClassName="GearPawn_LocustHunterArmorNoGrenades",TeamIdx=1)
	SpawnInfo(AIType_HunterNoArmorNoGrenades)=(ControllerClassName="GearAI_Hunter",PawnClassName="GearPawn_LocustHunterNoArmorNoGrenades",TeamIdx=1)
	SpawnInfo(AIType_FlameDrone)=(ControllerClassName="GearAI_FlameDrone",PawnClassName="GearPawn_LocustFlameDrone",TeamIdx=1)

	SpawnInfo(AIType_Hoffman)=(ControllerClassName="GearAI_Hoffman",PawnClassName="GearPawn_COGHoffman",TeamIdx=0,bUnique=TRUE)

	SpawnInfo(AIType_TroikaGunner)=(ControllerClassName="GearAI_Locust",PawnClassName="GearPawn_LocustTroikaGunner",TeamIdx=1)
	SpawnInfo(AIType_TroikaSpotter)=(ControllerClassName="GearAI_Locust",PawnClassName="GearPawn_LocustTroikaSpotter",TeamIdx=1)

	SpawnInfo(AIType_NPCFranklin)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCFranklin",TeamIdx=0)
	SpawnInfo(AIType_NPCMaleWithHatAndBag)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCMaleWithHatAndBag",TeamIdx=0)
	SpawnInfo(AIType_NPCMaleOld)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCMaleOld",TeamIdx=0)
	SpawnInfo(AIType_NPCGirl)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCGirl",TeamIdx=0)
	SpawnInfo(AIType_NPCMaleWithDoorag)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCMaleWithDoorag",TeamIdx=0)
	SpawnInfo(AIType_NPCBoy)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCBoy",TeamIdx=0)
	SpawnInfo(AIType_NPCWomanWithBun)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCWomanWithBun",TeamIdx=0)
	SpawnInfo(AIType_NPCFemaleWithBackpackOfCans)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCFemaleWithBackpackOfCans",TeamIdx=0)
	SpawnInfo(AIType_NPCMaleWithHeadband)=(ControllerClassName="GearAI_NPC",PawnClassName="GearPawn_NPCMaleWithHeadband",TeamIdx=0)

	SpawnInfo(AIType_ReaverWithDrone)=(ControllerClassName="GearAI_Reaver",PawnClassName="Vehicle_Reaver",TeamIdx=1)
	SpawnInfo(AIType_ReaverWithTheron)=(ControllerClassName="GearAI_Reaver",PawnClassName="Vehicle_ReaverWithTheron",TeamIdx=1)
	SpawnInfo(AIType_ReaverNoGunner)=(ControllerClassName="GearAI_Reaver",PawnClassName="Vehicle_ReaverNoGunner",TeamIdx=1)
	SpawnInfo(AIType_ReaverLowHealth)=(ControllerClassName="GearAI_Reaver",PawnClassName="Vehicle_ReaverLowHealth",TeamIdx=1)
	SpawnInfo(AIType_ReaverBig)=(ControllerClassName="GearAI_Reaver",PawnClassName="Vehicle_ReaverBig",TeamIdx=1)

	SpawnInfo(AIType_NPCCOGMaleWithHatAndBag)=(ControllerClassName="GearAI_NPCCOG",PawnClassName="GearPawn_NPCCOGMaleWithHatAndBag",TeamIdx=0)

	SpawnInfo(AIType_NPCCOGFranklinWithNoDreads)=(ControllerClassName="GearAI_NPCCOG",PawnClassName="GearPawn_NPCCOGFranklinWithNoDreads",TeamIdx=0)
	SpawnInfo(AIType_NPCCOGDrunkMan)=(ControllerClassName="GearAI_NPCCOG",PawnClassName="GearPawn_NPCCOGDrunkMan",TeamIdx=0)
	SpawnInfo(AIType_NPCCOGManWithBandana)=(ControllerClassName="GearAI_NPCCOG",PawnClassName="GearPawn_NPCCOGManWithBandana",TeamIdx=0)
	SpawnInfo(AIType_NPCCOGOldMan)=(ControllerClassName="GearAI_NPCCOG",PawnClassName="GearPawn_NPCCOGOldMan",TeamIdx=0,bUnique=TRUE)

	SpawnInfo(AIType_Lynx)=(ControllerClassName="GearAI_Locust",PawnClassName="GearPawn_LocustNinja",TeamIdx=1)
	SpawnInfo(AIType_Skorge)=(ControllerClassName="GearAI_Skorge",PawnClassName="GearPawn_LocustSkorge",TeamIdx=1)

	SpawnInfo(AIType_Brumak)=(ControllerClassName="GearAI_Brumak",PawnClassName="GearPawn_LocustBrumak",TeamIdx=1)
	SpawnInfo(AIType_Brumak_Big)=(ControllerClassName="GearAI_Brumak",PawnClassName="GearPawn_LocustBrumak_Big",TeamIdx=1)
	SpawnInfo(AIType_Brumak_Kadar)=(ControllerClassName="GearAI_Brumak_Kadar",PawnClassName="GearPawn_LocustBrumak",TeamIdx=1)
	SpawnInfo(AIType_Brumak_ResistSmallArms)=(ControllerClassName="GearAI_Brumak",PawnClassName="GearPawn_LocustBrumak_ResistSmallArms",TeamIdx=1)

	SpawnInfo(AIType_BloodMountWithDrone)=(ControllerClassName="GearAI_Bloodmount",PawnClassname="GearPawn_LocustBloodMountWithDrone",TeamIdx=1)
	SpawnInfo(AIType_BloodMountRiderless)=(ControllerClassName="GearAI_Bloodmount",PawnClassname="GearPawn_LocustBloodMountRiderless",TeamIdx=1)
	SpawnInfo(AIType_BloodMountWithPalaceGuard)=(ControllerClassName="GearAI_Bloodmount",PawnClassname="GearPawn_LocustBloodMountWithPalaceGuard",TeamIdx=1)

	SpawnInfo(AIType_RockWorm)=(ControllerClassName="GearAI_RockWorm",PawnClassName="GearPawn_RockWorm",TeamIdx=254)

	SpawnInfo(AIType_NemaSlug)=(ControllerClassname="GearAI_NemaSlug",PawnClassName="GearPawn_LocustNemaSlug",TeamIdx=1)

	SpawnInfo(AIType_Sire)=(ControllerClassname="GearAI_Sire",PawnClassName="GearPawn_LocustSire",TeamIdx=1)

	SpawnInfo(AITYpe_SecurityNemacyst)=(ControllerClassname="GearAI_SecurityNemacyst",PawnClassName="GearPawn_LocustSecurityNemacyst",TeamIdx=1)
	SpawnInfo(AIType_FlyingSecurityBot)=(ControllerClassname="GearAI_SecurityBotFlying",PawnClassName="GearPawn_SecurityBotFlying",TeamIdx=2)

	SpawnInfo(AIType_Jack)=(ControllerClassname="GearAI_Jack",PawnClassName="Vehicle_Jack",TeamIdx=0,bUnique=TRUE)


	SpawnInfo(AIType_LocustSpeedy)=(ControllerClassName="GearAI_Speedy",PawnClassName="GearPawn_LocustSpeedy",TeamIdx=1)
	SpawnInfo(AIType_LocustBeastRider)=(ControllerClassName="GearAI_Hunter",PawnClassName="GearPawn_LocustBeastRider",TeamIdx=1)
	SpawnInfo(AIType_LocustLancerGuard)=(ControllerClassName="GearAI_LocustDrone",PawnClassName="GearPawn_LocustLancerGuard",TeamIdx=1)
	SpawnInfo(AIType_LocustGrapplingDrone)=(ControllerClassName="GearAI_LocustDrone",PawnClassName="GearPawn_LocustGrapplingDrone",TeamIdx=1)

	SpawnInfo(AIType_DroneNoHelmet)=(ControllerClassName="GearAI_LocustDrone",PawnClassName="GearPawn_LocustDroneNoHelmet",TeamIdx=1)

	SpawnInfo(AIType_DroneMortar)=(ControllerClassName="GearAI_LocustDrone",PawnClassName="GearPawn_LocustDroneMortar",TeamIdx=1)


	bMoveInterruptable=TRUE

	InventoryTypeNames(WEAP_Lancer)="GearWeap_AssaultRifle"
	InventoryTypeNames(WEAP_Gnasher)="GearWeap_Shotgun"
	InventoryTypeNames(WEAP_Longshot)="GearWeap_SniperRifle"
	InventoryTypeNames(WEAP_Hammerburst)="GearWeap_LocustAssaultRifle"
	InventoryTypeNames(WEAP_Boltock)="GearWeap_LocustPistol"
	InventoryTypeNames(WEAP_Boomshot)="GearGameContent.GearWeap_Boomshot"
	InventoryTypeNames(WEAP_Snub)="GearWeap_COGPistol"
	InventoryTypeNames(WEAP_TorqueBow)="GearGameContent.GearWeap_Bow"
	InventoryTypeNames(WEAP_HOD)="GearGameContent.GearWeap_HOD"
	InventoryTypeNames(WEAP_Mortar)="GearGameContent.GearWeap_HeavyMortar"
	InventoryTypeNames(WEAP_Minigun)="GearGameContent.GearWeap_HeavyMiniGun"
	InventoryTypeNames(WEAP_FragGrenade)="GearWeap_FragGrenade"
}

