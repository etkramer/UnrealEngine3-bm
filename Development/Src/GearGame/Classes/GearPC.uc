/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearPC extends GamePlayerController
	config(Pawn)
	dependson(GearTypes,GearPawn,GearDiscoverablesPickupFactoryBase,GearPointOfInterest,GearGRI)
	native(PC)
	nativereplication;

cpptext
{
	virtual void PreBeginPlay();

	virtual FGuid* GetGuid() { return &MyGuid; }

	virtual UBOOL WantsLedgeCheck();

	FLOAT ScoreFrictionTarget(AActor const* Actor, FLOAT MaxDistance, FRotator const& CamRot);

	virtual UBOOL HearSound(USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed);

	virtual UBOOL Tick(FLOAT DeltaSeconds, ELevelTick TickType);

	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );

	virtual void physWalking(FLOAT deltaTime, INT Iterations);

	virtual void	ShowSelf(){/*SILENCE!*/} // this is handled by the ai visibility manager
	void UpdateCanDoSpecialMoveCacheList(TArrayNoInit<UBOOL>& CacheList, TArrayNoInit<BYTE>& PawnIndexList, FLOAT& LastUpdateTime, INT& LastUpdatedIndex);

	virtual void PostNetReceive();
}

/** The type of match during which a screenshot was taken . */
enum eScreenshotMatchType
{
	SMT_Private, // = 0
	SMT_Public   // = 1
};

/** Render target used to save the last screenshot for later display. */
var transient Texture2DDynamic LastCapturedShot;
/** Flags that indicate the state of the screenshot system */
var transient bool bRequestingShot;
var transient bool bCompressingShot;
var transient bool bSavingShot;
/** Info for the last screenshot that was taken. */
var transient ScreenshotInfo LastShotInfo;
/** Used to track the async task that compresses screenshots. */
var native transient const pointer CompressScreenshotTask{void};
/** The screenshot manager for this player. */
var transient GearScreenshotManager ScreenshotManager;
var transient array<SavedScreenshotInfo> EnumeratedScreenshots;

/** List of stored variables used to persist data through certain map transitions */
struct native StoredKismetVariable
{
	var Name VariableName;
	var string StringValue;
	var float FloatValue;
	var Object ObjectValue;
	var int IntValue;
	var bool BoolValue;
};
var transient array<StoredKismetVariable> StoredKismetVariables;

/** Struct containing all data pertaining to an Objective */
struct native ObjectiveInfo
{
	/** Name of objective for scripting purposes */
	var() Name ObjectiveName;
	/** Objective description to draw on the HUD */
	var() string ObjectiveDesc;
	/** Was this objective updated? */
	var() bool bUpdated;
	/** Is this objective completed? */
	var() bool bCompleted;
	/** Is this objective failed? */
	var() bool bFailed;
	/** Time at which this objective was received/updated/completed */
	var() float UpdatedTime;
	/** Whether to update the player's screen */
	var bool bNotifyPlayer;
};

struct native CheckpointMusicRecord extends MusicTrackDataStructures.MusicTrackStruct
{
	var string TheSoundCuePath;
};

struct CheckpointRecord
{
	var Guid SavedGuid; // our GUID at save time
	var float PawnHealthPct;
	var string PawnClassName;
	var string MutatedClassName;
	var string PawnPathName; // used when Pawn at save time was a level placed actor
	var vector Location;
	var rotator Rotation;
	var string BasePathName;
	var byte TeamIndex;
	var string SquadFormationName;
	var string SlotMarkerPathName; // path name of slot marker we were taking cover on (if we were taking cover)
	var array<InventoryRecord> InventoryRecords;
	var array<ObjectiveInfo> Objectives;
	var string CratePathName; // path name to carried crate (if any)
	var CheckpointMusicRecord Music;
	var string MountedFaceFX[3];
	var array<StoredKismetVariable> StoredKismetVariables;
	var string KismetAnimSets[2];
	var float GUDSFrequencyMultiplier;
};

/** Cached version of MyGearPawn */
var GearPawn MyGearPawn;

/** Squad this player belongs to */
var GearSquad Squad;

/** Struct to define values for different post processing presets. */
struct native PostProcessInfo
{
	var ETVType Preset;
	var float Shadows;
	var float MidTones;
	var float HighLights;
	var float Desaturation;
};

/** Enum of canvas icons supported */
enum GearIconTextureType
{
	eGEARICON_A,
	eGEARICON_A_PRESSED,
	eGEARICON_B,
	eGEARICON_B_PRESSED,
	eGEARICON_X,
	eGEARICON_Y,
	eGEARICON_Chainsaw,
	eGEARICON_Chainsaw_PRESSED,
};
/** List of textures mapped by GearIconTextureType */
var array<CanvasIcon> GearIcons;

/** Cover the server wants the client to enter as soon as it replicates (used primarily for spawned cover where the channel may not exist yet for an RPC) */
var repnotify CoverSlotMarker ServerDictatedCover;

/** Array of possible preset values for post process. */
var transient array<PostProcessInfo>	PostProcessPresets;

/** Indicates if this GearPC currently has the centaur PP overlay */
var	bool	bHasCentaurOverlay;

/** Actor camera is looking at */
var Actor	CameraLookAtFocusActor;

/** a bit kludgey here.  system need some architectural love */
var bool	bCameraLookAtIsFromKismet;

/** Show feedback effects in god mode */
var()	config bool		bShowFeedbackFXInGodMode;

/** Debugging AI vars */
var config bool bDebugAI;
var array<name> DebugAICategories;
var config bool bInvisible;

/** Used to keep the state of whether MP debugging is on**/
var bool bShowDebugMP;

/** DeadZone used as a threshold for player controls */
var(Cover) config float	DeadZoneThreshold;

var() config bool   bShowSpecialMoveTips;

/** Flag indicating that player is currently targeting. */
var			bool	bIsTargeting;

/** Disable double click for movement */
var config	bool	bDisableDoubleClickMovement;
/** Allow mouse wheel scroll to zoom when targeting */
var config	bool	bMouseWheelZoom;

//
// Cover
//

/** Allow player to enter cover from any direction, or restrict it to the forward region? */
var(Cover) config		bool	bRun2CoverAnyDirection;

/** Threshold to break from cover when pulling back on the stick. */
var(Cover)	config		float	BreakFromCoverHoldTime;
/** Internal property used to track current amount of time stick has been held to break from cover. */
var transient float BreakFromCoverHoldTimer;

/** Bool when desiring to break from cover, to delay until full input/movement has been processed */
var			transient	bool	bBreakFromCover;
/** If TRUE, press A to break from cover. */
var			config		bool	bPressAToBreakFromCover;

/** Camera relative controls: Left/Right. */
var	float	RemappedJoyRight;
/** Camera relative controls: Up/Down */
var float	RemappedJoyUp;
/** Rotation used to remap controls */
var Rotator	ControlsRemapRotation;

/** Minimum delay between 2 cover moves */
var(Cover)	config	float	EvadeRetriggerDelay;

/** debug cover flag */
var(Cover)	config	bool	bDebugCover;

/** Animation debugging. */
var bool bDebugAnimation;
var float DebugTextMaxLen;

/** Amount of time the player is delayed when moving between slots with a lean */
var()	config		float	CoverTransitionTime;
var		transient	float	CoverTransitionCountHold;
var		transient	float	CoverTransitionCountDown;

/** Toggle for assess mode */
var bool bAssessMode;

/** Kismet events for entering and leaving cover */
var const array<SeqEvt_EnteredCover> EnteredCoverEvents;
var const array<SeqEvt_LeftCover> LeftCoverEvents;

/** Last friction target */
var transient Actor LastFrictionTarget;
/** Last friction target acquire time */
var transient float LastFrictionTargetTime;

/** in range [0..1], how hard is the forward accel control depressed. */
var transient float VehicleGasPressedAmount;
/** in range [0..1], how hard is the reverse accel control depressed. */
var transient float VehicleReversePressedAmount;

/** Pawn class to use for this controller, used mainly for coop games when joining as Dom vs Marcus. */
var	class<Pawn> DefaultPawnClass;

/** Amount of time before starting a roadie run */
var() config float RoadieRunTimer;

/** Direction double click was captured last */
var EDoubleClickDir CurrentDoubleClickDir;

/** Last time mouse wheel was used to cycle weapons */
var() transient float LastCycleWeaponTime;

/** Take Damage Scale for Near Death */
struct native ScaleDamageInfo
{
	/** If health is less than this, scale */
	var() float		HealthLimitPct;
	/** Scale damage by this amount */
	var() float		Scale;
};
var() config array<ScaleDamageInfo>	ScaleDamageList;

/**
 * Defines the cone from the player that determines whether or not it is ok
 * to do melee-attack adhesion.  This is the cosine of the half-angle of the cone.
 */
var() float MinMeleeAdhesionDotValue;

/** Target we are forcibly adhering to. */
var GearPawn	ForcedAdhesionTarget;

/** Number of orders given to AI which the AI refused to do */
var Byte	RefusedOrderCount;

/** Is god mode currently enabled by a camera look at? */
var bool bCameraGodMode;

/** Are we currently looking at a point of interest? */
var bool bLookingAtPointOfInterest;

/** Played when we get a new POI **/
var SoundCue PointOfInterestAdded;
/** Last Time we played a POI Sound */
var float LastPoITime;
/** How long between POI Sounds*/
var() float PoIRepeatTime;

/**
 * Time which we stop players from Pushing out of cover when pressing 'A'
 * This is used to stop people mashing the 'A' button and getting into cover and then
 * immediately leaving cover as they hit 'A' again
 **/
var config float TimeBeforeAButtonPressLeavesCover;

/** all other things being equal, would we prefer to lean out of mid level cover or pop up over it? */
var transient bool bPreferLeanOverPopup;

/**
 * The current GearPawn being targetedusing the tooltip targeting feature
 */
var GearPawn CurrentTooltipTargetGearPawn;

//
// Input
//

/** Stack of player inputs being used by the controller */
var array<PlayerInput> PlayerInputStack;
/** Reference to the main GearPlayerInput object so that update calls (like camera reorienting) can still happen */
var GearPlayerInput MainPlayerInput;

/** Whether to use the alternate controller scheme or not - set via SetAlternateControls() */
var const bool bUseAlternateControls;

/** List of Kismet input events */
var const array<SeqEvt_Input> InputEvents;

/** Whether the input is enabled or disabled */
var int InputIsDisabledCount;

/** The state of the inputs from cinematic mode */
var bool bCinemaDisableInputButtons;

/** Is this player in combat?  Set by IsInCombat(), and should never be referenced directly. */
var private float LastCheckIsInCombatTime;

/** List of enabled POIs */
var	private array<GearPointOfInterest>	EnabledPointsOfInterest;
/** List that matches EnabledPointsOfInterest which determines if the POI in the same index has been looked at yet */
var private array<bool>					POILookedAtList;
var GearPointOfInterest					CurrLookedAtPOI;

/** Interp speed range for looking at points of interest.  bonus points for long var name. */
var() vector2d PointOfInterestLookatInterpSpeedRange;

/** Current camera point viewing */
var GearSpectatorPoint CurrSpectatorPoint;
/** String representing what the player is currenlty spectating */
var transient string CurrSpectatingString;
/** If the player is spectating, this bool tells whether the player is spectating another player or not (if not, they are spectating cameras ) */
var transient bool bPlayerSpectatingOtherPlayer;
/** The PRI of the player we are spectating */
var GearPRI SpectatingPRI;

/** Separate pitch value for ghost spectating since the controller pitch will get zero'd by physwalking. */
var int GhostSpectatingPitch;

/** Holy alliteration batman!  Celebratory Screenshake to play when you get a supersuccess active reload */
var() const protected ScreenShakeStruct	ARSuperSuccessScreenShake;

/** Data for doing door triggers */
struct native DoorTriggerData
{
	/** Whether the player is inside a door trigger or not */
	var bool bInsideDoorTrigger;
	/** The object being triggered */
	var Trigger_DoorInteraction DoorTrigger;
	/** The special move type of the door interaction */
	var ESpecialMove eSpecialMoveType;
};
var DoorTriggerData DoorTriggerDatum;

struct native LocalEnemy
{
	/** reference to the enemy warpawn */
	var GearPawn	Enemy;

	/** holds results of last LOS check */
	var bool	bVisible;

	/** true if enemy has ever been visible */
	var bool	bSeen;

	/** true if there is LOS...  this does NOT exclude enemies behind you like bVisible does */
	var bool	bHasLOS;
};

/** cache to store LOS status of enemies */
var array<LocalEnemy> LocalEnemies;

/** List used to turn crosshair red for given actors */
var Actor UnfriendlyActorList[16];

/** Idx in LocalEnemies array of next enemy to check for LOS (we only check one per frame */
var transient int NextLocalEnemyToCheckLOS;

/** The max distance an enemy can be before their excluded from the death camera looking at them */
var const float DeathCameraEnemyMaxDistance;
/** Biggest threat to the player when in reviving state */
var GearPawn BiggestThreatWP;

/** action info associated with looking at a focus point, and the timer that goes with it. */
var const ActionInfo ActionLookAt;

/** action info associated with valve turning. */
var const ActionInfo ActionValveTurn;

/** The ActionInfo which is used when a teammate has died **/
var ActionInfo ActionLookAtDownedTeammate;

/** Action icon for mashing button to stay alive during revive state */
var const ActionInfo ActionStayingAlive;

/** Action icon for suiciding with a grenade */
var const ActionInfo ActionSuicideBomb;

/** Action icon for displaying action icons through kismet */
var ActionInfo ActionKismetButton;

/** The profile data associated with this player */
var GearProfileSettings ProfileSettings;

/** whether active cover type was received by servermove currently being processed */
var bool bServerMoveCoverActive;

/** Last pawn I possessed */
var Pawn LastPawn;

/** Offset from the top of the viewport to set the UIScene's position for fitting splitscreen properly */
var float SceneYOffsetForSplitscreen;

/** indicates whether this player is a dedicated server spectator (so disable rendering and don't allow it to participate at all) */
var bool bDedicatedServerSpectator;

/** request call to ClientVerifyState().  Done asynchronously to make sure only one sent per tick */
var bool bRequestClientVerifyState;

/** request call to ServerVerifyState().  Done asynchronously to make sure only one sent per tick */
var bool bRequestServerVerifyState;

/** Enum of the different post process effects */
enum GearPostProcessFXType
{
	eGPP_None,
	eGPP_Pause,
	eGPP_BleedOut,
	eGPP_Gameover,
};
var GearPostProcessFXType CurrentPPType;

/**
 * Post process settings for gameover and pause UI scenes.
 */
var PostProcessSettings GameoverPPSettings;
var PostProcessSettings PausePPSettings;

/** Structure defining necessary data to play a camera-bone animation. */
struct native CameraBoneAnimation
{
	var Name		AnimName;
	var vector		CollisionTestVector;
};


/** current index into profile maplist that we are playing (local server player only) */
var int MapListIndex;

/**
 * Structure to contain the configurable data needed to modify a player's health if they
 * receive too much burst damage in a small amount of time.
 */
struct native BurstDamageModConfig
{
	/** The percentage of the total health of the player that if damaged this amount will make us save him and give him one last try */
	var const float	HealthPercentForLifeSave;
	/** The percentage of the total health of the player we will subtract per second from CurrentBurstDamageTaken */
	var const float	HealthPercentWipedPerSecond;
	/** The amount of time we will give the player invincibility if we save their life */
	var const float	InvincibilityLengthInSecs;
	/** The number of units at which the enemy can still hit you when invincible */
	var const float	InvincibilityRangeInUnits;
};
var config BurstDamageModConfig BurstDamageModConfigData;

const Player_BulletWhipDistance = 500;
const Player_NearMissDistance	= 200;

/** Current sound mode in use */
var name CurrentSoundMode;

/** The current hud cast to a GearHUD_Base */
var GearHud_Base MyGearHud;

/** Current Kismet interaction event player would activate if he presses Use now. Determined on server, replicated to client */
var SeqEvt_Interaction InteractEvent;

/** List of currently Down But Not Out teammates.  Pawns are added when they enter the reviving state, and removed when list is checked and they fail a condition */
var array<GearPawn> DBNOTeammates;

/** true if level has vehicles, so player should be checking for available HUD actions related to vehicles */
var bool bCheckVehicles;

/** set when camera is being controlled by Matinee, so we don't force the server's ViewTarget on the client */
var bool bInMatinee;

var bool bWasKicked;

/** Used to track how many pawns were killed with a given explosive round */
var int NumExplosiveKills;
/** Used to see how fast the number of kills occurred. If 3 within .5 seconds, then increment ClusterLuck */
var float ExplosiveKillTimeStamp;

/** True if this PC has contributed his info to the initialization of the COG Tags */
var transient bool bHasInitedCogTags;

/** Extra rotation to apply this tick in UpdateOrient */
var transient rotator ExtraRot;

var transient Actor	SearchGoalActor;

/** Controller sensitivity multipliers for rotation */
var() config float	RotationSensitivityLow;
var() config float	RotationSensitivityMedium;
var() config float	RotationSensitivityHigh;
/** Controller sensitivity multipliers for rotation when targetting */
var() config float	TargettingRotationSensitivityLow;
var() config float	TargettingRotationSensitivityMedium;
var() config float	TargettingRotationSensitivityHigh;
/** Controller sensitivity multipliers for rotation when zooming */
var() config float	ZoomRotationSensitivityLow;
var() config float	ZoomRotationSensitivityMedium;
var() config float	ZoomRotationSensitivityHigh;

/** For driving mode, rotation of view relative to vehicle. */
var	rotator RelativeToVehicleViewRot;
/** Last output rotation when using 'relative to vehicle' mode - for interpolation. */
var rotator LastVehicleSpaceViewRotation;

/** Channel the player is associated with regardless of their state */
var int VoiceChannel;

/** If true we are in 'warmup paused' state to allow textures to stream in, and should not allow unpausing. */
var transient bool bIsWarmupPaused;

/** Sounds and components for the gameover screen */
var AudioComponent GameoverLoop;
var SoundCue OpenGameoverCue;
var SoundCue LoadCheckpointCue;

/** Rotation.Yaw when we entered TargetingMortar mode. */
var transient float TargetHeavyWeaponBaseRotationYaw;

/** Is invulnerable due to respawning */
var bool bIsRespawnInvincible;
/** Is this player currently waiting to respawn? */
var bool bWaitingToRespawn;

/** The distance of the trace to determine if a player is friendly or not */
var() config float DistanceForEnemyTrace;

/** The data store that holds data needed to translate bound input events to strings for UI */
var GearUIDataStore_StringAliasBindingsMap BoundEventsStringDataStore;
/** The data store that holds data needed to translate unbound input events to strings for UI */
var GearUIDataStore_StringAliasMap StringDataStore;

/** The object that manages the objectives for the player. */
var GearObjectiveManager ObjectiveMgr;

/**
 * The progress manager receives notification about various player progress
 * and decides whether to display a notification (in the form of a Toast).
 */
var GearAlertManager AlertManager;

/** The object that manages the tutorials for the player. */
var GearTutorialManager TutorialMgr;
/**
 * Whether the tutorial system has been on (used when we need to turn it off for special UI events like timers)
 * NOTE: should only be set at the time that the TutorialMgr is created and from the OnManageTutorials kismet call
 */
var bool bTutorialSystemWasOn;

/** Enum of events that will trigger a list of delegates when an event occurs */
enum EGearEventDelegateType
{
	eGED_CoverAcquired,
	eGED_CoverLeave,
	eGED_Mantle,
	eGED_Evade,
	eGED_CoverSlip,
	eGED_SwatTurn,
	eGED_GrenadeToss,
	eGED_KickDoor,
	eGED_Pickup,
	eGED_Revive,
	eGED_Chainsaw,
	eGED_ClimbDownLadder,
	eGED_RoadieRun,
	eGED_Reload,
	eGED_Executions,
	eGED_MeatShield,
	eGED_Crawling,
	eGED_Shield,
	eGED_Turret,
	eGED_NumTypes	// used to set the length of GearEventDelegates
};

/** Struct to hold anything related to an event that will trigger delegates when an event occurs */
struct native GearEventData
{
	/** Array of delegates that will be triggered when the event occurs */
	var array<delegate<OnGearEvent> > GearEventDelegates;
};

/** Array of GearEventData indexed by EGearEventDelegateType that will trigger delegates when an event occurs */
var array<GearEventData> GearEventList;

/** Array of delegates to be fired when a weapon is equipped */
var array<delegate<OnGearWeaponEquip> > GearWeaponEquipDelegates;

/** Whether we've used the Loaded exec and have access to all of the weapons (this is so we can switch between mousewheeling functionality for debug) */
var bool bWeaponsLoaded;

/** Played when we go into sudden death **/
var SoundCue SuddenDeathSound;
/** Text to draw when we go into sudden death */
var localized string SuddenDeathString;
/** Text to draw when get revived by a teammate */
var localized string RevivedString;

/** The discoverable that is currently being looked at */
var EGearDiscoverableType DiscoverType;

/** True if this player is engaged in a user-abortable conversation.  See SeqAct_ToggleConversation. */
var transient bool bHavingAnAbortableConversation;

/** True if player has requested to abort the conversation camera.  Used on the server. */
var protected transient bool bPendingConversationAbort;

/** Time between horn sounds on vehicle */
var float LastHornPlayedTime;

/** Rain emitter that's currently attached to us. */
var protected Emit_Rain WeatherEmitter;

/************************************************************************
	HELPER VARIABLES FOR TRACKING ACHIEVEMENTS
 ************************************************************************/
/** Minigun achievement variables */
var bool bIsMountedWithMinigun;

/************************************************************************
	PROFILE AND ONLINE VARIABLES
 ************************************************************************/

/** Whether we need to update the profile settings or not */
var bool bProfileSettingsUpdated;

/** If this is true, then our connection with the server has timed out */
var bool bHasLostContactWithServer;

/** The XUID of the host for reporting them as dropped */
var UniqueNetId HostId;

/** GUID for checkpoint saving so that references to us can be saved even though we may be recreated */
var Guid MyGuid;

// the last time we updated one of the entries in our cache lists
var float LastCoverActionSpecialMoveUpdateTime,LastGlobalActionSpecialMoveUpdateTime;
// the last index we updated in our cached lists
var int LastCoverActionSpecialMoveUpdateIdx,LastGlobalActionSpecialMoveUpdateIdx;
// the actual cando cache, (one of which is updated each frame)
var array<bool> CachedCanDoCoverActionSpecialMoves;
var array<bool> CachedCanDoGlobalActionSpecialMoves;

var transient bool bDebugFaceCam;
var protected transient bool bDebugGUDBrowser;
var protected transient bool bDebugEffortBrowser;

/** indicates this is a fake player created to load checkpoint data that was saved when playing co-op,
 * but is being loaded in single player
 * it is destroyed immediately after parsing the checkpoint data
 */
var transient const bool bCheckpointDummy;

/** client holder for GUDS sound references, since GUDSManager does not exist on the client
 * should not be used on the server
 */
struct native GUDSReferenceHolder
{
	/** name of GUD bank package */
	var name PackageName;
	/** GUDBank object */
	var GUDBank Bank;
	/** FaceFX animset */
	var FaceFXAnimSet FaceFXData;
};
var array<GUDSReferenceHolder> ClientGUDSReferences;

var transient float LastCheckpointSaveTime;

var transient	SeqAct_SpectatorCameraPath	BattleCamPath;

/** Used to temporarily disable targeting DOF */
var transient bool							bDisableCameraTargetingDOF;

/** number of events (anything you want) that have occurred so we can do mem profiler dumps **/
var transient int Automation_NumMemoryEvents;

/** Ignore close connection errors when explicitly traveling */
var bool bIsReturningFromMatch;

/** set when client is loading checkpoint until we receive the confirmation notify */
var transient bool bClientLoadingCheckpoint;

replication
{
	// Things the server should send to the client.
	if( bNetOwner && Role==ROLE_Authority )
		InteractEvent, ServerDictatedCover, bWaitingToRespawn, UnfriendlyActorList;
}

simulated event ReplicatedEvent(Name VarName)
{
	local CovPosInfo CovInfo;

	if (VarName == nameof(ServerDictatedCover))
	{
		if (ServerDictatedCover != None)
		{
			if (MyGearPawn != None)
			{
				MyGearPawn.SetCoverInfoFromSlotInfo(CovInfo, ServerDictatedCover.OwningSlot);
			}
			else
			{
				CovInfo.Link = ServerDictatedCover.OwningSlot.Link;
				CovInfo.LtSlotIdx = ServerDictatedCover.OwningSlot.SlotIdx;
				CovInfo.RtSlotIdx = ServerDictatedCover.OwningSlot.SlotIdx;
				CovInfo.LtToRtPct = 0.f;
				CovInfo.Location = ServerDictatedCover.Location;
				CovInfo.Normal = vector(ServerDictatedCover.Rotation) * -1;
			}
			AcquireCover(CovInfo);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

event InitInputSystem()
{
	local bool bPushInputObject;
	local LocalPlayer LP;
	local GearGameSP_Base SPGame;

	if ( PlayerInput == None )
	{
		bPushInputObject = true;
	}

	Super.InitInputSystem();

	LP = LocalPlayer(Player);

	if (bDedicatedServerSpectator)
	{
		LP.ViewportClient.bDisableWorldRendering = true;
	}

	// Push the newly created playerinput onto the player input stack if there wasn't one already.
	if ( bPushInputObject )
	{
		MainPlayerInput = GearPlayerInput(PlayerInput);
		MainPlayerInput.OnInputObjectCreate();
		PushPlayerInput( PlayerInput );
	}

	if (WorldInfo.NetMode == NM_Client)
	{
		ServerSetAlternateControls(bUseAlternateControls);
	}

	// reset the post processing when we get a new PC
	LP.RemoveAllPostProcessingChains();
	LP.InsertPostProcessingChain(LP.Outer.DefaultPostProcess, INDEX_NONE, true);

	// clear the loading screen if it's up
	SPGame = GearGameSP_Base(WorldInfo.Game);
	if (WorldInfo.NetMode == NM_Client || SPGame == None || !SPGame.bStartupLoadCheckpoint)
	{
		ClientShowLoadingMovie(false, GetURLMap() ~= `CAMPAIGN_ENTRY_LEVEL);
	}
}

/** Pushes a player input object to the stack and sets the current player input to be that new object. */
final function PushPlayerInput( PlayerInput NewPlayerInput )
{
	local GearPlayerInput_Base PrevInputObject;
	local int InteractionIdx;

	if ( NewPlayerInput != None )
	{
		if ( PlayerInputStack.length > 0 )
		{
			PrevInputObject = GearPlayerInput_Base(PlayerInputStack[PlayerInputStack.length-1]);
		}

		// Add the new input
		PlayerInputStack.AddItem( NewPlayerInput );

		// Set the current input object to the new one
		InteractionIdx = Interactions.Find(PlayerInput);
		PlayerInput = NewPlayerInput;
		if ( InteractionIdx == INDEX_NONE )
		{
			Interactions[Interactions.Length] = PlayerInput;
		}
		else
		{
			Interactions[InteractionIdx] = PlayerInput;
		}

		// Call the function that allows the new input object to perform any needed tasks when getting pushed to the top of the stack.
		GearPlayerInput_Base(PlayerInput).PushedOnTopOfInputStack( PrevInputObject, true );

		// If there was already an input object on the stack call the function that lets it handle
		// it's being pushed back from the front of the stack.
		if ( PrevInputObject != None )
		{
			PrevInputObject.PushedBackOnInputStack();
		}
	}

	UpdateControllerConfig();
}

/** Pops a player input object from the stack and sets the current player input to be the next input object in the stack. */
final function PopPlayerInput( optional PlayerInput PlayerInputToPop )
{
	local GearPlayerInput_Base PrevInputObject;
	local int IndexToRemove, InteractionIdx;
	local bool bRemovedCurrentInputHandler;

	if (  PlayerInputStack.length > 0 )
	{
		// Find the index to remove
		IndexToRemove = (PlayerInputToPop != None) ? PlayerInputStack.Find(PlayerInputToPop) : PlayerInputStack.length-1;

		// Could not find the input to remove so bail
		if ( IndexToRemove == INDEX_NONE )
		{
			return;
		}

		// Set the input object to be removed
		PrevInputObject = GearPlayerInput_Base(PlayerInputStack[IndexToRemove]);

		// Determine if the input object to be removed is the currently active input handler
		bRemovedCurrentInputHandler = (PlayerInput == PrevInputObject) ? true : false;

		// Remove it
		PlayerInputStack.RemoveItem( PrevInputObject );
	}

	// If the input handler that got removed was active, to some cleanup
	if ( bRemovedCurrentInputHandler )
	{
		if ( PlayerInputStack.length > 0 )
		{
			// Set the input object to be the last in the list
			InteractionIdx = Interactions.Find(PlayerInput);
			PlayerInput = PlayerInputStack[PlayerInputStack.length-1];
			if ( InteractionIdx == INDEX_NONE )
			{
				Interactions[Interactions.Length] = PlayerInput;
			}
			else
			{
				Interactions[InteractionIdx] = PlayerInput;
			}

			// Call the function that will allow the old input object to perform any needed tasks now that it's becoming the front of the list.
			GearPlayerInput_Base(PlayerInput).PoppedToTopOfInputStack( PrevInputObject, true );
		}

		// Let the input object that is being popped off of the stack to perform any needed tasks if it was active
		if ( PrevInputObject != None )
		{
			PrevInputObject.PoppedOffOfInputStack();
		}
	}

	UpdateControllerConfig();
}

/** Delegate used to control whether we can unpause during warmup. We never allow this. */
function bool CanUnpauseWarmup()
{
	return !bIsWarmupPaused;
}

/** Function used by loading code to pause and unpause the game during texture-streaming warm-up. */
event WarmupPause(bool bDesiredPauseState)
{
	local color FadeColor;
	local PlayerController PC;

	bIsWarmupPaused = bDesiredPauseState;

	SetPause(bDesiredPauseState, CanUnpauseWarmup);

	// When pausing, start a fade in
	// the fade won't actually go away until after the pause is turned off,
	// but starting it on pause instead of unpause prevents any delays between the loading movie vanishing and pause being disabled
	// from causing us to be viewing some random place in the world for a few frames
	if (!bDesiredPauseState)
	{
		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			GearPC(PC).ClientColorFade(FadeColor, 255, 0, 2.0);
		}
	}
}

/**
 * Sets the player name to the client's gamertag so it can be replciated
 * to all clients
 *
 * @param Gamertag the new name to use
 */
reliable server function ServerSetGamerTag(string Gamertag)
{
	PlayerReplicationInfo.SetPlayerName(Gamertag);
}

/**
 * Called when the local player is about to travel to a new map or IP address.  Provides subclass with an opportunity
 * to perform cleanup or other tasks prior to the travel.
 *
 * Overloaded so we can save the profile before traveling
 */
event PreClientTravel( string PendingURL, ETravelType TravelType, bool bIsSeamlessTravel )
{
	// Only save if we are NOT in the menu level
	if (!class'WorldInfo'.static.IsMenuLevel())
	{
		SaveProfile();
	}

	Super.PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);
}

/** Client function so that the server can tell this player to save it's profile */
reliable client function ClientSaveProfile()
{
	SaveProfile();
}

simulated event PostBeginPlay()
{
	local GearPC PC;

	Super.PostBeginPlay();

	// clear lingering blur from seamless travel case
	SetTimer(2.f,FALSE,nameof(ClearBlurPP));

	PreCacheAnimations();

	if (MainPlayerInput != None)
	{
		MainPlayerInput.LastInputTime = WorldInfo.TimeSeconds;
		PlayerInput.bInvertMouse = class'GearGameSettings'.default.InvertLook.bValue;
	}

	// store the points of interest for clients
	StorePointsOfInterest();

	// init the post processing settings
	InitPausePostProcessSettings();
	InitGameoverPostProcessSettings();

	// set the default sound mode
	SetSoundMode( 'Default' );

	bHasInitedCogTags = FALSE;

	// Create the Objective Manager if it hasn't been created yet.
	if ( ObjectiveMgr == None )
	{
		ObjectiveMgr = new(self) class'GearObjectiveManager';
	}

	// Create a new Progress Manager if we do not have one yet.
	if ( AlertManager == None )
	{
		AlertManager = new(self) class'GearAlertManager';
		AlertManager.InitGearAlertManager(self);
	}

	// Ensure the profile settings are initially set
	bProfileSettingsUpdated = TRUE;

`if(`notdefined(FINAL_RELEASE))
	CheatManager = new(Self) CheatClass;
`endif

// so if we are auto hording then start the killing in N seconds
    if( GearGame(WorldInfo.Game) != None && GearGame(WorldInfo.Game).bDoAutomatedHordeTesting == TRUE )
	{
		foreach WorldInfo.AllControllers( class'GearPC', PC )
		{
			PC.SetTimer( 10.0f, FALSE, nameof(PC.Automation_KillMobs) );
		}
	}
}

/** Clear the contents of the command to bind key cache */
simulated function ClearStringAliasBindingMapCache()
{
	BoundEventsStringDataStore.ClearBoundKeyCache();
}

/** Initialized the tutorial system if it hasn't been inited yet */
reliable client final function ClientInitializeTutorialSystem( EGearAutoInitTutorialTypes TutorialListType, optional bool bAddNonOptionControlledTutorials )
{
	if ( TutorialMgr == None )
	{
		TutorialMgr = new(self) class'GearTutorialManager';
		bTutorialSystemWasOn = true;
		TutorialMgr.StartSystem( bTutorialSystemWasOn, true, TutorialListType );
		// Hack to start the auto tutorials that are not save in the profile
		if (bAddNonOptionControlledTutorials)
		{
			TutorialMgr.AddAutoInitiatedTutorials(false);
		}
	}
}

/** Start the tutorial system */
final function RestartTutorialSystem( optional bool bTurnSystemOn = true, optional bool bWipeTutorials = true )
{
	if ( TutorialMgr != None )
	{
		TutorialMgr.StartSystem( bTurnSystemOn, bWipeTutorials );
		TutorialMgr.AddAutoInitiatedTutorials();
	}
}

/** Stop the tutorial system */
final function StopTutorialSystem( optional bool bWipeTutorials = true )
{
	if ( TutorialMgr != None )
	{
		TutorialMgr.StopSystem( bWipeTutorials );
	}
}

/** Sets a timer that will stop the weapon from firing */
final function StopFiringWeaponAfterTime( float Time )
{
	SetTimer( Time, false, nameof(StopFiringWeapon) );
}

/** Stop the weapon from firing */
final function StopFiringWeapon()
{
	bFire = 0;
	StopFire(0);
	SetLastWeaponInfoTime(WorldInfo.TimeSeconds);
}

/**
* Creates and initializes the "PlayerOwner" and "PlayerSettings" data stores.  This function assumes that the PlayerReplicationInfo
* for this player has not yet been created, and that the PlayerOwner data store's PlayerDataProvider will be set when the PRI is registered.
*
* Overloaded so we can initialize game specific player data stores.
*/
simulated protected function RegisterCustomPlayerDataStores()
{
	local LocalPlayer LP;
	local DataStoreClient DataStoreManager;
	local class<GearUIDataStore_StringAliasBindingsMap> StringAliasBindingsMapDataStoreClass;
	local class<GearUIDataStore_StringAliasMap> StringAliasMapDataStoreClass;

`if(`notdefined(FINAL_RELEASE))
	local string PlayerName;
	PlayerName = PlayerReplicationInfo != None ? PlayerReplicationInfo.PlayerName : "None";
`endif

	// Unregister old profile, clean profile callbacks etc.
	if ( ProfileSettings != None )
	{
		UnregisterPlayerDataStores();
	}

	// only create player data store for local players
	LP = LocalPlayer(Player);

	`log(">>" @ `location @ "(" $ PlayerName $ ")" @ `showobj(LP),,'DevDataStore');
	Super.RegisterCustomPlayerDataStores();
	if ( LP != None )
	{
		// Get the warfare specific profile data
		ProfileSettings = GearProfileSettings(OnlinePlayerData.ProfileProvider.Profile);

		// get a reference to the main data store client
		DataStoreManager = class'UIInteraction'.static.GetDataStoreClient();
		if ( DataStoreManager != None )
		{
			// find the "PlayerOwner" data store registered for this player; there shouldn't be one...
			BoundEventsStringDataStore = GearUIDataStore_StringAliasBindingsMap(DataStoreManager.FindDataStore('StringAliasBindings',LP));
			if ( BoundEventsStringDataStore == None )
			{
				// find the appropriate class to use for the PlayerSettings data store
				StringAliasBindingsMapDataStoreClass = class<GearUIDataStore_StringAliasBindingsMap>(DataStoreManager.FindDataStoreClass(class'GearUIDataStore_StringAliasBindingsMap'));
				if ( StringAliasBindingsMapDataStoreClass != None )
				{
					// create the PlayerOwner data store
					BoundEventsStringDataStore = DataStoreManager.CreateDataStore(StringAliasBindingsMapDataStoreClass);
					if ( BoundEventsStringDataStore != None )
					{
						// and register it
						if ( !DataStoreManager.RegisterDataStore(BoundEventsStringDataStore, LP) )
						{
							`log("Failed to register 'StringAliasBindings' data store for player:"@ Self @ "(" $ PlayerName $ ")");
						}
					}
					else
					{
						`log("Failed to create 'StringAliasBindings' data store for player:"@ Self @ "(" $ PlayerName $ ") using class" @ StringAliasBindingsMapDataStoreClass,,'DevDataStore');
					}
				}
			}
			else
			{
				`log("'StringAliasBindings' data store already registered for player:"@ Self @ "(" $ PlayerName $ ")",,'DevDataStore');
			}

			// find the "PlayerOwner" data store registered for this player; there shouldn't be one...
			StringDataStore = GearUIDataStore_StringAliasMap(DataStoreManager.FindDataStore('StringAliasMap',LP));
			if ( StringDataStore == None )
			{
				// find the appropriate class to use for the PlayerSettings data store
				StringAliasMapDataStoreClass = class<GearUIDataStore_StringAliasMap>(DataStoreManager.FindDataStoreClass(class'GearUIDataStore_StringAliasMap'));
				if ( StringAliasMapDataStoreClass != None )
				{
					// create the PlayerOwner data store
					StringDataStore = DataStoreManager.CreateDataStore(StringAliasMapDataStoreClass);
					if ( StringDataStore != None )
					{
						// and register it
						if ( !DataStoreManager.RegisterDataStore(StringDataStore, LP) )
						{
							`log("Failed to register 'StringAlias' data store for player:"@ Self @ "(" $ PlayerName $ ")");
						}
					}
					else
					{
						`log("Failed to create 'StringAlias' data store for player:"@ Self @ "(" $ PlayerName $ ") using class" @ StringAliasMapDataStoreClass,,'DevDataStore');
					}
				}
			}
			else
			{
				`log("'StringAlias' data store already registered for player:"@ Self @ "(" $ PlayerName $ ")",,'DevDataStore');
			}
		}
	}

	`log("<<" @ `location @ "(" $ PlayerName $ ")",,'DevDataStore');
}

/**
* Unregisters the "PlayerOwner" data store for this player.  Called when this PlayerController is destroyed.
*
* Overloaded so we can unregister game specific player data stores.
*/
simulated function UnregisterPlayerDataStores()
{
	local LocalPlayer LP;
	local DataStoreClient DataStoreManager;

`if(`notdefined(FINAL_RELEASE))
	local string PlayerName;
	PlayerName = PlayerReplicationInfo != None ? PlayerReplicationInfo.PlayerName : "None";
`endif

	`log(">>" @ `location @ "(" $ PlayerName $ ")",,'DevDataStore');
	// only execute for local players
	LP = LocalPlayer(Player);
	if ( LP != None )
	{

		// Clear profile ref
		ProfileSettings = None;

		// unregister it from the data store client and clear our reference
		// get a reference to the main data store client
		DataStoreManager = class'UIInteraction'.static.GetDataStoreClient();
		if ( DataStoreManager != None )
		{
			// unregister the bound events string data store
			if ( BoundEventsStringDataStore != None )
			{
				if ( !DataStoreManager.UnregisterDataStore(BoundEventsStringDataStore) )
				{
					`log("Failed to unregister 'StringAliasBindings' data store for player:"@ Self @ "(" $ PlayerName $ ")");
				}

				// clear the reference
				BoundEventsStringDataStore = None;
			}
			else
			{
				`log("'StringAliasBindings' data store not registered for player:" @ Self @ "(" $ PlayerName $ ")",,'DevDataStore');
			}

			// unregister the string data store
			if ( StringDataStore != None )
			{
				if ( !DataStoreManager.UnregisterDataStore(StringDataStore) )
				{
					`log("Failed to unregister 'StringAlias' data store for player:"@ Self @ "(" $ PlayerName $ ")");
				}

				// clear the reference
				StringDataStore = None;
			}
			else
			{
				`log("'StringAlias' data store not registered for player:" @ Self @ "(" $ PlayerName $ ")",,'DevDataStore');
			}
		}
		else
		{
			`log("Data store client not found!",,'DevDataStore');
		}

	}

	Super.UnregisterPlayerDataStores();
	`log("<<" @ `location @ "(" $ PlayerName $ ")",,'DevDataStore');
}

/** Overridden from PlayerController to set the default camera mode. */
function SpawnPlayerCamera()
{
	super.SpawnPlayerCamera();

	// default camera mode
	SetCameraMode('default');
}

// MSSTART justinmc@microsoft.com 4/8/2007: debug hooks
native exec function MS_GetPlayerLoc(int MessageGUID);
// MSEND justinmc@microsoft.com 4/8/2007: debug hooks

// MSSTART brantsch@microsoft.com 5/12/2008: Movement hooks
native exec function MS_GetPlayerRotation(int MessageGUID);
native exec function MS_GetCameraRotation(int MessageGUID);
// MSEND brantsch@microsoft.com 5/12/2008: Movement hooks
// MSSTART brantsch@microsoft.com 5/12/2008: Movement hooks
exec function MS_SetPlayerRotation(int Yaw, int Pitch, int Roll, int MessageGUID = 0)
{
	local vector SetLocation;
	local rotator SetNewRotation;
	GetPlayerViewPoint( SetLocation, SetNewRotation );
	SetNewRotation.Yaw = Yaw;
	// so passing in -1 for Pitch, Yaw, or Roll will leave the current value, otherwise update it with the new value
	if(Roll !=-1)
	{
		SetNewRotation.Roll = Roll;
	}
	if(Pitch !=-1)
	{
		SetNewRotation.Pitch = Pitch;
	}
	if (Yaw != -1)
	{
		SetNewRotation.Yaw = Yaw;
	}
	ClientSetRotation(SetNewRotation);
	class'GearCheatManager'.static.MS_SendDebugMessage("UNREAL!UDC!" $  string(Rand(500)) $  "|1|" $  string(MessageGUID) $  "|1");
}
// MSEND brantsch@microsoft.com 5/12/2008: Movement hooks

// MSSTART brantsch@microsoft.com 04/16/2008 - profile commands for MS
exec native function MS_DumpProfile();
exec native function MS_GetProfileSettingName(int ProfileSettingId);
exec native function MS_GetProfileSettingValueList(int ProfileSettingId);
exec native function MS_GetProfileSettingValueName(int ProfileSettingId);
exec native function MS_SetProfileSettingValueId(int ProfileSettingId,int Value);
exec native function MS_SetProfileSettingValueByName(int ProfileSettingId, const out string NewValue);
exec function MS_SaveProfile()
{
	SaveProfile( MS_OnProfileWriteComplete );
}
/** Called when the profile is done writing */
function MS_OnProfileWriteComplete(byte LocalUserNum, bool bWasSuccessful)
{
		ClearSaveProfileDelegate( MS_OnProfileWriteComplete );
		// Update the controller with the new settings
		UpdateLocalCacheOfProfileSettings();
		`log("<MS_OnProfileWriteComplete LocalUserNum = " $ LocalUserNum $ " WasSuccessful=" $ bWasSuccessful $ ">");
}
// MSEND brantsch@microsoft.com 04/16/2008 - profile commands for MS

/**
 * Force load anims (matinee and specified animsets) from lazy arrays to
 * get rid of load hitches.
 */
native final function PreCacheAnimations();

event ConditionalPause(bool bDesiredPauseState)
{
	// client can't pause
	if (WorldInfo.NetMode != NM_Client)
	{
		Super.ConditionalPause(bDesiredPauseState);
	}
}

/** Find and store all the points of interest in the level */
simulated function StorePointsOfInterest()
{
	local GearPointOfInterest POI;

	foreach DynamicActors(class'GearPointOfInterest',POI)
	{
		if ( POI.bEnabled )
		{
			AddPointOfInterest( POI );
		}
	}
}

/** Whether the player has line of sight to the POI or not */
simulated function bool HasLineOfSightToPOI( GearPointOfInterest POI )
{
	local Actor HitActor;
	local vector ActualLookatLoc, HitLoc, HitNormal, StartTrace, StartTraceAdj;

	// See if we should trace for line of sight
	if ( POI.bForceLookCheckLineOfSight )
	{
		ActualLookatLoc = POI.GetActualLookatLocation();
		StartTrace = Pawn.Location + vect(0,0,1) * Pawn.GetCollisionHeight();

		if (MyGearPawn != None)
		{
			// adjust starttrace right a little, to account for camera offset
			StartTraceAdj = vect(0,64,0);
			if (MyGearPawn.bIsMirrored)
			{
				StartTraceAdj.Y = -StartTraceAdj.Y;
			}
			StartTrace += StartTraceAdj >> rotator(ActualLookatLoc-StartTrace);
		}

		HitActor = Trace( HitLoc, HitNormal, ActualLookatLoc, StartTrace );

		if ( (HitActor != None) && (HitActor != POI.AttachedToActor) )
		{
			return false;
		}
	}

	return true;
}

/** Force the player to look at a POI */
simulated function ProcessPOI( optional GearPointOfInterest POI, optional bool bUserInstigated )
{
	local GearPointOfInterest BestPOI, CurrPOI;

	// First see which POI is best to look at
	BestPOI = FindBestPOIToLookAt( POI );
	// If a POI was passed in and it's not the best one or there is no best one, do nothing
	if ( (BestPOI == None) || ((POI != None) && (POI != BestPOI)) )
	{
		return;
	}

	// Do a line of sight check if we need to, returning if failed
	if ( (BestPOI.ForceLookType != ePOIFORCELOOK_None) && BestPOI.bForceLookCheckLineOfSight )
	{
		if ( !HasLineOfSightToPOI(BestPOI) )
		{
			return;
		}
	}

	// See if we need to stop an old look at or return because we're about to look at the same one
	if ( bLookingAtPointOfInterest && (CameraLookAtFocusActor != None) )
	{
		CurrPOI = GearPointOfInterest(CameraLookAtFocusActor);

		// Same POI so return
		if ( CurrPOI == BestPOI )
		{
			return;
		}

		// Different POI, so stop it
		if ( CurrPOI != None )
		{
			StopForceLookAtPointOfInterest( bUserInstigated );
		}
	}

	// Look at the POI
	ForceLookAtPointOfInterest( bUserInstigated, BestPOI );
}

/**
 * Add a point of interest this client can look at, optionally forced.
 */
simulated function AddPointOfInterest( GearPointOfInterest POI )
{
	local int Idx, CurrPriority;

	if (POI != None)
	{
		// Set locally in case of networked games
		POI.bEnabled = TRUE;

		// Add to list
		if (EnabledPointsOfInterest.Find(POI) == INDEX_NONE)
		{
			EnabledPointsOfInterest[EnabledPointsOfInterest.length] = POI;
			POILookedAtList[POILookedAtList.length] = false;
		}

		// If this poi wants us to disable the other scripted pois, then do so
		if ( POI.bDisableOtherPOIs )
		{
			ClearFocusPoint( false );
			for ( Idx = 0; Idx < EnabledPointsOfInterest.length; Idx++ )
			{
				if ( (EnabledPointsOfInterest[Idx] != POI) && EnabledPointsOfInterest[Idx].bEnabled )
				{
					CurrPriority = EnabledPointsOfInterest[Idx].GetLookAtPriority( self );
					if ( CurrPriority >= class'GearPointOfInterest'.Default.POIPriority_ScriptedEvent )
					{
						EnabledPointsOfInterest[Idx].bEnabled = false;
					}
				}
			}
		}

		// If we are doing an automatic force-look, or the player is already looking at a POI, try and process it
		if ( (POI.ForceLookType == ePOIFORCELOOK_Automatic) || (bLookingAtPointOfInterest && IsHoldingPOIButton()) )
		{
			ProcessPOI( POI, (bLookingAtPointOfInterest && IsHoldingPOIButton()) );
		}
	}
}

/**
* Removes a point of interest from the potential list.
*/
simulated function RemovePointOfInterest( GearPointOfInterest POI, optional EPOIForceLookType ForceLookType )
{
	local int Idx;

	if ( POI != None )
	{
		Idx = EnabledPointsOfInterest.Find( POI );
		if ( Idx >= 0 )
		{
			EnabledPointsOfInterest.Remove( Idx, 1 );
			POILookedAtList.Remove( Idx, 1 );
			if (CameraLookAtFocusActor == POI)
			{
				StopForceLookAtPointOfInterest();
			}
		}
	}
}

/**
 * Tell HUD to display ammo message
 */
reliable client function ClientAddAmmoMessage(class<GearDamageType> DamageClass, int AmountAdded)
{
	if ( (myHUD != None) && myHUD.bShowHUD && (DamageClass != None) )
	{
		MyGearHud.LastWeaponInfoTime = WorldInfo.TimeSeconds;
		MyGearHud.AddAmmoMessage( PlayerReplicationInfo, DamageClass, AmountAdded );
	}
}

function SetLastWeaponInfoTime(float NewTime)
{
	if ( (MyGearHud != None) && myHUD.bShowHUD )
	{
		MyGearHud.LastWeaponInfoTime = NewTime;
	}
}

simulated function OnPlayWaveForm( SeqAct_PlayWaveForm inAction )
{
	ClientPlayForceFeedbackWaveform( inAction.WaveForm );
}

/**
 * @return	TRUE if starting a force feedback waveform is allowed;  child classes should override this method to e.g. temporarily disable
 * 			force feedback
 */
simulated function bool IsForceFeedbackAllowed()
{
	return Super.IsForceFeedbackAllowed() && MyGearHud != None && MyGearHud.PauseUISceneInstance == None;
}

/**
 *  Play forcefeedback if needed
 */
simulated function TryForceFeedback( const out ScreenShakeStruct ShakeData )
{
	local int ShakeLevel;
	local float RotMag, LocMag, FOVMag;

	// figure out if we're "big", "medium", or nothing
	RotMag = VSize(ShakeData.RotAmplitude);
	if (RotMag > 40.f)
	{
		ShakeLevel = 2;
	}
	else if (RotMag > 20.f)
	{
		ShakeLevel = 1;
	}

	if (ShakeLevel < 2)
	{
		LocMag = VSize(ShakeData.LocAmplitude);
		if (LocMag > 10.f)
		{
			ShakeLevel = 2;
		}
		else if (LocMag > 5.f)
		{
			ShakeLevel = 1;
		}

		FOVMag = ShakeData.FOVAmplitude;
		if (ShakeLevel < 2)
		{
			if (FOVMag > 5.f)
			{
				ShakeLevel = 2;
			}
			else if (FOVMag > 2.f)
			{
				ShakeLevel = 1;
			}
		}
	}

	//`log("level="@ShakeLevel@"rotmag"@VSize(ShakeData.RotAmplitude)@"locmag"@VSize(ShakeData.LocAmplitude)@"fovmag"@ShakeData.FOVAmplitude);

	if (ShakeLevel == 2)
	{
		if( ShakeData.TimeDuration <= 1 )
		{
			ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeBigShort);
		}
		else
		{
			ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeBigLong);
		}
	}
	else if (ShakeLevel == 1)
	{
		if( ShakeData.TimeDuration <= 1 )
		{
			ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeMediumShort);
		}
		else
		{
			ClientPlayForceFeedbackWaveform(class'GearWaveForms'.default.CameraShakeMediumLong);
		}
	}
}

/**
 * Scripting hook for camera shakes.
 */
function OnGearCameraShake(SeqAct_GearCameraShake inAction)
{
	local byte Mode;

	if (inAction.InputLinks[0].bHasImpulse)
	{
		// normal, timed
		Mode = 0;
	}
	else if (inAction.InputLinks[1].bHasImpulse)
	{
		// continuous
		Mode = 1;
	}
	else if (inAction.InputLinks[2].bHasImpulse)
	{
		// stop
		Mode = 2;
	}
	ClientPlayKismetCameraShake(InAction, Mode);
}


/** plays a Kismet controlled camera shake on the client */
unreliable client function ClientPlayKismetCameraShake(SeqAct_GearCameraShake InAction, byte Mode)
{
	local ScreenShakeStruct GearCameraShake;
	local ScreenShakeAnimStruct CameraShakeAnim;
	local float AmplitudeScale, FrequencyScale;
	local GearPlayerCamera GearCam;

	if (InAction == None)
	{
		return;
	}

	GearCam = GearPlayerCamera(PlayerCamera);
	if (GearCam == None)
	{
		// no camera?  bail.
		return;
	}

	if ( InAction.bRadialFalloff && (InAction.LocationActor == None) )
	{
		`Warn("Location actor needed for bRadialFalloff camera shake.");
		return;
	}

	if (InAction.bDoCodeDrivenShake)
	{
		AmplitudeScale = inAction.CodeShake_GlobalScale * inAction.CodeShake_GlobalAmplitudeScale;
		FrequencyScale = inAction.CodeShake_GlobalScale * inAction.CodeShake_GlobalFrequencyScale;

		// Copy shake, so we don't change its properties directly.
		GearCameraShake = inAction.CodeShake_Params;

		// include package name here, since regular name can be duplicated between sub-levels.
		GearCameraShake.ShakeName = name(string(GetPackageName()) $ "_" $ inAction.Name);

		GearCameraShake.TimeDuration *= inAction.CodeShake_GlobalScale;
		GearCameraShake.RotAmplitude *= AmplitudeScale;
		GearCameraShake.RotFrequency *= FrequencyScale;
		GearCameraShake.LocAmplitude *= AmplitudeScale;
		GearCameraShake.LocFrequency *= FrequencyScale;
		GearCameraShake.FOVAmplitude *= AmplitudeScale;
		GearCameraShake.FOVFrequency *= FrequencyScale;

		if (InAction.bRadialFalloff)
		{
			switch (Mode)
			{
			case 0:
				// normal timed
				GearCam.PlayRadialCameraShake(GearCameraShake, InAction.LocationActor.Location, InAction.RadialShake_InnerRadius, InAction.RadialShake_OuterRadius, InAction.RadialShake_Falloff, inAction.bDoControllerVibration);
				break;
			case 1:
				// continuous
				GearCameraShake.TimeDuration = 9999999.f;
				GearCam.PlayRadialCameraShake(GearCameraShake, InAction.LocationActor.Location, InAction.RadialShake_InnerRadius, InAction.RadialShake_OuterRadius, InAction.RadialShake_Falloff, inAction.bDoControllerVibration);
				break;
			case 2:
				// stop
				GearCam.GearCamMod_ScreenShake.RemoveScreenShake(GearCameraShake.ShakeName);
				break;
			default:
				`Warn("Unknown mode" @ Mode);
				break;
			}
		}
		else
		{
			switch (Mode)
			{
			case 0:
				// normal timed
				ClientPlayCameraShake(GearCameraShake, inAction.bDoControllerVibration);
				break;
			case 1:
				// continuous
				GearCameraShake.TimeDuration = 9999999.f;
				ClientPlayCameraShake(GearCameraShake, inAction.bDoControllerVibration);
				break;
			case 2:
				// stop
				GearCam.GearCamMod_ScreenShake.RemoveScreenShake(GearCameraShake.ShakeName);
				break;
			default:
				`Warn("Unknown mode" @ Mode);
				break;
			}
		}
	}

	// play any anim-driven shakes.  note these aren't mutually exclusive with code-driven shakes
	if (InAction.bDoAnimDrivenShake)
	{
		// Copy shake, so we don't change its properties directly.
		CameraShakeAnim = InAction.AnimShake_Params;

		if (InAction.bRadialFalloff)
		{
			switch (Mode)
			{
			case 0:
				// normal timed
				GearCam.PlayRadialCameraShakeAnim(CameraShakeAnim, InAction.LocationActor.Location, InAction.RadialShake_InnerRadius, InAction.RadialShake_OuterRadius, InAction.RadialShake_Falloff, inAction.bDoControllerVibration);
				break;
			case 1:
				// continuous
				CameraShakeAnim.bRandomSegment = TRUE;		// must be true for continuous shake
				CameraShakeAnim.RandomSegmentDuration = 9999999.f;
				GearCam.PlayRadialCameraShakeAnim(CameraShakeAnim, InAction.LocationActor.Location, InAction.RadialShake_InnerRadius, InAction.RadialShake_OuterRadius, InAction.RadialShake_Falloff, inAction.bDoControllerVibration);
				break;
			case 2:
				// stop
				// @fixme, store the playing instance and just stop that one!
				GearCam.StopAllCameraAnimsByType(CameraShakeAnim.Anim);
				break;
			default:
				`Warn("Unknown mode" @ Mode);
				break;
			}
		}
		else
		{
			switch (Mode)
			{
			case 0:
				// normal timed
				GearCam.PlayCameraShakeAnim(CameraShakeAnim);
				break;
			case 1:
				// continuous
				CameraShakeAnim.bRandomSegment = TRUE;		// must be true for continuous shake
				CameraShakeAnim.RandomSegmentDuration = 9999999.f;
				GearCam.PlayCameraShakeAnim(CameraShakeAnim);
				break;
			case 2:
				// stop
				// @fixme, store the playing instance and just stop that one!
				GearCam.StopAllCameraAnimsByType(CameraShakeAnim.Anim);
				break;
			default:
				`Warn("Unknown mode" @ Mode);
				break;
			}
		}
	}
}


/**
 * Camera Shake
 * Plays camera shake effect
 *
 * @param	Duration			Duration in seconds of shake
 * @param	newRotAmplitude		view rotation amplitude (pitch,yaw,roll)
 * @param	newRotFrequency		frequency of rotation shake
 * @param	newLocAmplitude		relative view offset amplitude (x,y,z)
 * @param	newLocFrequency		frequency of view offset shake
 * @param	newFOVAmplitude		fov shake amplitude
 * @param	newFOVFrequency		fov shake frequency
 */
function CameraShake
(
	float	Duration,
	vector	newRotAmplitude,
	vector	newRotFrequency,
	vector	newLocAmplitude,
	vector	newLocFrequency,
	float	newFOVAmplitude,
	float	newFOVFrequency
)
{
	if( GearPlayerCamera(PlayerCamera) != None )
	{
		GearPlayerCamera(PlayerCamera).CameraShake( Duration, newRotAmplitude, newRotFrequency, newLocAmplitude, newLocFrequency, newFOVAmplitude, newFOVFrequency );
	}
}

/** Play Camera Shake */
unreliable client function ClientPlayCameraShake( ScreenShakeStruct ScreenShake, optional bool bTryForceFeedback )
{
	if( GearPlayerCamera(PlayerCamera) != None )
	{
		GearPlayerCamera(PlayerCamera).PlayCameraShake( ScreenShake );
	}

	if ( bTryForceFeedback )
	{
		TryForceFeedback( ScreenShake );
	}
}

/** Play Camera Shake */
unreliable client function ClientPlayCameraShakeAnim(ScreenShakeAnimStruct Shake)
{
	if( GearPlayerCamera(PlayerCamera) != None )
	{
		GearPlayerCamera(PlayerCamera).PlayCameraShakeAnim(Shake);
	}
}

reliable client function ClientCameraLookAtFinished(SeqAct_CameraLookAt Action)
{
	CameraLookAtFinished(Action);
}

/**
 * The function that gets called when the cameralookat is done.
 * This cleans up the CameraLookAt mode for kismet.
 */
simulated event CameraLookAtFinished(SeqAct_CameraLookAt Action)
{
	local GearPlayerCamera	Cam;
	local SkelControlLookAt	HeadControl;

	Focus = None;

	if( Action.bAffectCamera )
	{
		CameraLookAtFocusActor = None;
		Cam = GearPlayerCamera(PlayerCamera);
		ClearCameraFocus(Action, Cam);
	}

	if( Action.bAffectHead && MyGearPawn != None)
	{
		HeadControl = MyGearPawn.HeadControl;
		if( HeadControl != None )
		{
			MyGearPawn.HeadLookAtActor = None;
			HeadControl.SetSkelControlActive(FALSE);
		}
	}

	if (LocalPlayer(Player) == None)
	{
		ClientCameraLookAtFinished(Action);
	}
}

/** Undo the CameraLookAt */
simulated function ClearCameraFocus(SeqAct_CameraLookAt Action, GearPlayerCamera Cam)
{
	ClearFocusPoint(TRUE, Action.bLeaveCameraRotation );
	// clear camera god mode
	if ( bCameraGodMode )
	{
		bCameraGodMode = FALSE;
	}

	// clear the tooltip
	if ( MyGearHud != None )
	{
		MyGearHud.ClearActionInfoByType(AT_ForceLook);
	}

	// Turn input back on.
	if ( Action.bDisableInput )
	{
		EnableInput(true,true,true);
	}
}

function OnCameraBlood(SeqAct_CameraBlood Action)
{
	ClientSpawnCameraLensEffect(class<Emit_CameraLensEffectBase>(Action.Emitters[Action.Type]));
}

function OnCameraLookAt(SeqAct_CameraLookAt Action)
{
	local array<Object> ObjVars;
	local int Idx;
	local Actor FocusActor;
	local Controller C;

	// Find the first supplied actor
	Action.GetObjectVars(ObjVars, "Focus");
	for( Idx = 0; Idx < ObjVars.Length && FocusActor == None; Idx++ )
	{
		FocusActor = Actor(ObjVars[Idx]);

		// If its a player variable, look at pawn not controller
		C = Controller(FocusActor);
		if( C != None && C.Pawn != None )
		{
			FocusActor = C.Pawn;
		}
	}

	ProcessCameraLookAt(Action, FocusActor);
	if (LocalPlayer(Player) == None)
	{
		ClientProcessCameraLookAt(Action, FocusActor);
	}
}

simulated function ProcessCameraLookAt(SeqAct_CameraLookAt Action, Actor FocusActor)
{
	local GearPlayerCamera		Cam;
	local SkelControlLookAt		HeadControl;
	local bool					bDoNoInterpolate;
	local ActionInfo			HUDAction;
	local SkeletalMeshComponent ComponentIt;
	local vector				FocusLocation, HitLoc, HitNormal, StartTrace, StartTraceAdj;
	local actor					HitActor;

	if (Action.bCheckLineOfSight)
	{
		FocusLocation = FocusActor.Location;
		if( Action.FocusBoneName != 'None' && Action.FocusBoneName != '' )
		{
			// find a skeletal mesh component on the focus actor
			foreach FocusActor.ComponentList(class'SkeletalMeshComponent', ComponentIt)
			{
				FocusLocation = ComponentIt.GetBoneLocation(Action.FocusBoneName);
				break;
			}
		}

		StartTrace = Pawn.Location + vect(0,0,1) * Pawn.GetCollisionHeight();

		if (MyGearPawn != None)
		{
			// adjust starttrace right a little, to account for camera offset
			StartTraceAdj = vect(0,64,0);
			if (MyGearPawn.bIsMirrored)
			{
				StartTraceAdj.Y = -StartTraceAdj.Y;
			}
			StartTrace += StartTraceAdj >> rotator(FocusLocation-StartTrace);
		}

		HitActor = Trace(HitLoc, HitNormal, FocusLocation, StartTrace);

		if ( (HitActor != None) && (HitActor != FocusActor) )
		{
			// activate failed output
			Action.OutputLinks[3].bHasImpulse = TRUE;

			// and BAIL!
			return;
		}
	}


	Focus = FocusActor;

	// If we have a focus, update camera focuspoint. These params are used by the camera to look at focus point.
	if( Action.bAffectCamera )
	{
		Cam = GearPlayerCamera(PlayerCamera);

		if( FocusActor != None )
		{
			SetFocusPoint(
				TRUE,
				FocusActor,
				Action.InterpSpeedRange,
				Action.InFocusFOV,
				Action.CameraFOV,
				Action.bAlwaysFocus,
				!Action.bTurnInPlace,
				Action.bIgnoreTrace,
				Action.FocusBoneName );

			if ( (MyGearHud != None) && (Len(Action.TextDisplay) > 0) )
			{
				HUDAction.ToolTipText = Action.TextDisplay;
				MyGearHud.SetActionInfo(AT_ForceLook, HUDAction);
			}

			if ( Action.bToggleGodMode )
			{
				// if not already in god mode
				if (!bGodMode)
				{
					bCameraGodMode = TRUE;
				}
			}

			// Turn input off if disabled and the action has a duration
			if (Action.bDisableInput) // && Action.bUsedTimer) comment due to legacy kimset in gears
			{
				DisableInput(true,true,true);
			}
		}
		else
		{
			ClearCameraFocus(Action, Cam);
		}
	}

	if( Action.bAffectHead )
	{
		HeadControl = MyGearPawn.HeadControl;
		if( HeadControl != None )
		{
			// set for the first time, do not interpolate
			// use interpolation for in between target transitions.
			if( MyGearPawn.HeadLookAtActor == None )
			{
				bDoNoInterpolate = TRUE;
			}
			MyGearPawn.HeadLookAtActor = FocusActor;

			if( MyGearPawn.HeadLookAtActor != None )
			{
				MyGearPawn.HeadLookAtBoneName = Action.FocusBoneName;
				HeadControl.DesiredTargetLocation = MyGearPawn.GetHeadLookTargetLocation();
				if( bDoNoInterpolate )
				{
					HeadControl.TargetLocation = HeadControl.DesiredTargetLocation;
				}
				HeadControl.SetSkelControlActive(TRUE);
			}
			else
			{
				HeadControl.SetSkelControlActive(FALSE);
			}
		}
	}

	// activate succeeded output
	Action.OutputLinks[2].bHasImpulse = TRUE;
}

/** calls ProcessCameraLookAt on the client
 * @warning: assumes the various properties of Action aren't changed at runtime, since those changes won't happen on the client
 */
reliable client function ClientProcessCameraLookAt(SeqAct_CameraLookAt Action, Actor FocusActor)
{
	ProcessCameraLookAt(Action, FocusActor);
}

reliable client function ClientSetForcedCameraFOV(float inFOV)
{
	local GearPlayerCamera Cam;

	Cam = GearPlayerCamera(PlayerCamera);
	Cam.bUseForcedCamFOV = TRUE;
	Cam.ForcedCamFOV = inFOV;
}

reliable client function ClientClearForcedCameraFOV()
{
	local GearPlayerCamera Cam;

	Cam = GearPlayerCamera(PlayerCamera);
	Cam.bUseForcedCamFOV = FALSE;
}

function OnCameraSetFOV(SeqAct_CameraSetFOV Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		ClientSetForcedCameraFOV(Action.DesiredFOV);
	}
	else
	{
		ClientClearForcedCameraFOV();
	}
}


function OnSpawnCameraLensEffect(SeqAct_SpawnCameraLensEffect Action)
{
	ClientSpawnCameraLensEffect( Action.LensEffectToSpawn );
}


/** Lookup function because input class is object and doesn't have access to timers */
function GetInputTargets()
{
	MainPlayerInput.GetInputTargets();
}

/** Make the AI go back to normal mood since we are shutting taccom off */
function ResetTaccomAIMoods()
{
	local GearAI AI;
	local GearPRI AIPRI;

	foreach WorldInfo.AllControllers(class'GearAI',AI)
	{
		AIPRI = GearPRI(AI.PlayerReplicationInfo);
		// if they're in the same squad
		if ( AIPRI != None )
		{
			AI.SetCombatMood(AICM_Normal);
		}
	}
}

/** Function that gets called when a coop player joins a campaign game */
function CoopPlayerJoinedCampaign()
{
	ResetTaccomAIMoods();
}

/**
 * Server/SP only function for changing whether the player is in cinematic mode.  Updates values of various state variables, then replicates the call to the client
 * to sync the current cinematic mode.
 *
 * @param	bInCinematicMode	specify TRUE if the player is entering cinematic mode; FALSE if the player is leaving cinematic mode.
 * @param	bHidePlayer			specify TRUE to hide the player's pawn (only relevant if bInCinematicMode is TRUE)
 * @param	bAffectsHUD			specify TRUE if we should show/hide the HUD to match the value of bCinematicMode
 * @param	bAffectsMovement	specify TRUE to disable movement in cinematic mode, enable it when leaving
 * @param	bAffectsTurning		specify TRUE to disable turning in cinematic mode or enable it when leaving
 * @param	bAffectsButtons		specify TRUE to disable button input in cinematic mode or enable it when leaving.
 */
function SetCinematicMode( bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsButtons )
{
	local bool bAdjustButtonInput;
	local GearAI AI;

	if (Role < ROLE_Authority)
	{
		`Warn("Not supported on client");
		return;
	}

	// Calling super will apply the value of bInCinematicMode to bCinematicMode
	Super.SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning, bAffectsButtons);

	// Make player invisible to AI during cinematics
	bInvisible = bCinematicMode;
	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		AI.StopFiring();
	}

	bAdjustButtonInput = bAffectsButtons && (bCinematicMode != bCinemaDisableInputButtons);

	if( Pawn != None )
	{
		Pawn.SetCinematicMode( bCinematicMode );
	}

	if (bCinematicMode)
	{
		ClientStopForceFeedbackWaveform();
		ClientBreakFromCover();
		ExecuteMeatshields();
		ReviveTeammates();
		ServerReviveSelf();
		StopPostProcessOverride(CurrentPPType);
		// reset taccom FX as well
		if (MyGearHud != None)
		{
			MyGearHud.ResetTaccomOpacity();
			MyGearHud.CloseDiscoverableScene();
		}
	}

	if ( bAdjustButtonInput )
	{
		if ( bCinematicMode )
		{
			DisableInput(TRUE, FALSE, FALSE);
			if (LocalPlayer(Player) == None)
			{
				ClientDisableButtons();
			}
		}
		else
		{
			EnableInput(TRUE, FALSE, FALSE);
			if (LocalPlayer(Player) == None)
			{
				ClientEnableButtons();
			}
		}
		bCinemaDisableInputButtons = bCinematicMode;
	}

	// disable guds during cinematics
	if (GearGame(WorldInfo.Game).UnscriptedDialogueManager != None)
	{
		GearGame(WorldInfo.Game).UnscriptedDialogueManager.SetDisabled(bCinematicMode);
	}
}

/** called by the server to synchronize cinematic transitions with the client */
reliable client function ClientSetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsHUD)
{
	Super.ClientSetCinematicMode(bInCinematicMode, bAffectsMovement, bAffectsTurning, bAffectsHUD);

	if (myGearHUD != None)
	{
		myGearHUD.ActiveAction.bActive = FALSE;  // when we are in cinematic mode we need to clear any active ActionIcons

		// if the cinematic mode toggles the HUD off, we have to clean up the HUD
		if (myGearHUD.bShowHUD)
		{
			myGearHUD.ClearHUDAfterShowToggle();
		}

		if (bCinematicMode)
		{
			// reset taccom FX as well
			MyGearHud.ResetTaccomOpacity();
			MyGearHud.CloseDiscoverableScene();
		}
	}

	// Deactivate the active tutorial if there is one
	if (bCinematicMode && TutorialMgr != none)
	{
		TutorialMgr.RebootSystem();
	}

	// let the UI know that cinematic mode has [potentially] changed, so it should update cached viewport sizes for any open scenes
	UpdateUIViewportSizes();
}

simulated function UpdateUIViewportSizes()
{
	local GameUISceneClient SceneClient;
	local bool bWasForcingFullscreen;
	local LocalPlayer SecondPlayer;

	SceneClient = class'UIRoot'.static.GetSceneClient();

	// only relevant if we're playing split-screen - otherwise all scenes are always fullscreen
	if ( SceneClient != None && IsSplitscreenPlayer() )
	{
		SecondPlayer = class'UIInteraction'.static.GetLocalPlayer(1);

		bWasForcingFullscreen = SecondPlayer.Size.Y == 0;
		if ( SceneClient.ShouldForceFullscreenViewport() != bWasForcingFullscreen )
		{
			SceneClient.bUpdateSceneViewportSizes = true;
		}
	}

	RefreshAllSafeZoneViewports();
}

/** called by the server to synchronize cinematic transitions with the client */
reliable client function ClientDisableButtons()
{
	DisableInput(true, false, false);
}
reliable client function ClientEnableButtons()
{
	EnableInput(true, false, false);
}

/** Returns TRUE if you are in a coop-enabled mode. */
final function bool IsCoop()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	return (GRI != None && (GRI.bIsCoop || GRI.IsCoopMultiplayerGame()));
}

/** returns TRUE if we are in a coop-enabled mode and more than 1 player is active */
simulated final event bool IsActuallyPlayingCoop()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None)
	{
		return GRI.IsActuallyPlayingCoop();
	}

	return FALSE;
}

/** Whether there is a teammate to spectator or not */
final function bool CanSpectateTeammate()
{
	local int Idx;
	local GearPRI CurrPRI;

	if (PlayerReplicationInfo.bOnlySpectator)
	{
		return TRUE;
	}
	if ( WorldInfo.GRI != None )
	{
		for ( Idx = 0; Idx < WorldInfo.GRI.PRIArray.length; Idx++ )
		{
			CurrPRI = GearPRI(WorldInfo.GRI.PRIArray[Idx]);
			if ( PlayerReplicationInfo != CurrPRI &&
				 !CurrPRI.bIsDead )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

unreliable server function ServerViewNearestPlayer()
{
	local vector CamLoc;
	local rotator CamRot;
	local float BestValue, Value;
	local GearPawn GP, BestGP;
	BestValue = 999999.f;
	GetPlayerViewPoint(CamLoc,CamRot);
	foreach WorldInfo.AllPawns(class'GearPawn',GP)
	{
		if (GP != None && GP.PlayerReplicationInfo != None && GP.Health > 0 && WorldInfo.Game.CanSpectate(self,GP.PlayerReplicationInfo))
		{
			Value = VSize(CamLoc - GP.Location) * (1.f - (Normal(GP.Location - CamLoc) dot vector(CamRot)));
			if (Value < BestValue && Value > 0)
			{
				BestValue = Value;
				BestGP = GP;
			}
		}
	}
	if (BestGP != None)
	{
		SetViewTarget(BestGP.PlayerReplicationInfo);
		`log("viewing:"@`showvar(BestGP));
	}
	else
	{
		ServerViewNextPlayer();
	}
}

unreliable server function ServerViewNextPlayer()
{
	local GearPawn_COGGear Gear;

	if (IsSpectating() && GearGRI(WorldInfo.GRI).GameStatus != GS_RoundOver )
	{
		// if coop,
		if (IsActuallyPlayingCoop())
		{
			// view the first ai-controlled gear
			foreach WorldInfo.AllPawns(class'GearPawn_COGGear',Gear)
			{
				if (WorldInfo.Game.CanSpectate(self, Gear.PlayerReplicationInfo) &&
					GearPC(Gear.Controller) == None)
				{
					SetViewTarget(Gear);
					ClientSetViewTarget(Gear);
					return;
				}
			}

		}
		// otherwise cycle through players normally
		Super.ServerViewNextPlayer();
	}
}

unreliable server function ServerViewPrevPlayer()
{
	local GearPawn_COGGear Gear;

	if (IsSpectating())
	{
		// if coop,
		if (IsActuallyPlayingCoop())
		{
			// view the first ai-controlled gear
			foreach WorldInfo.AllPawns(class'GearPawn_COGGear',Gear)
			{
				if (WorldInfo.Game.CanSpectate(self, Gear.PlayerReplicationInfo) &&
					GearPC(Gear.Controller) == None)
				{
					SetViewTarget(Gear);
					ClientSetViewTarget(Gear);
					return;
				}
			}

		}
		// otherwise cycle through players normally
		Super.ServerViewPrevPlayer();
	}
}

function OnAIForceSwitchWeapon(SeqAct_AIForceSwitchWeapon Action)
{
	local Weapon FoundWeapon;
	FoundWeapon = Weapon(Pawn.InvManager.FindInventoryType(Action.ForcedWeaponType));
	if (FoundWeapon != None)
	{
		Pawn.InvManager.SetCurrentWeapon(FoundWeapon);
	}
}

/**
 * Sets the viewtarget for the first battle camera found, called mid-transitions to keep the camera seamless (especially for clients).
 */
simulated function Actor LocallySetInitialViewTarget(optional bool bSkipSetViewTarget)
{
	local Sequence GameSequence;
	local int Idx;
	local array<SequenceObject> Objs;
	// locally look for a battle cam to view
	GameSequence = WorldInfo.GetGameSequence();
	GameSequence.FindSeqObjectsByClass(class'SeqAct_SpectatorCameraPath',TRUE,Objs);
	for (Idx = 0; Idx < Objs.Length; Idx++)
	{
		BattleCamPath = SeqAct_SpectatorCameraPath(Objs[Idx]);
		break;
	}
	if (BattleCamPath != None)
	{
		BattleCamPath.ResetToStartingPosition();
		if (!bSkipSetViewTarget)
		{
			ClientSetViewTarget(BattleCamPath.GetAssociatedCameraActor());
		}
		return BattleCamPath.GetAssociatedCameraActor();
	}
	return None;
}

auto state PlayerWaiting
{
	function PickInitialViewTarget()
	{
		ViewFirstSpectatorPoint();
	}

	function PickNextViewTarget()
	{
		ViewNextSpectatorPoint();
	}

	function PickPrevViewTarget()
	{
		ViewPrevSpectatorPoint();
	}

	simulated function BeginState(Name PreviousStateName)
	{
		super.BeginState( PreviousStateName );

		// look for the initial battle cam
		LocallySetInitialViewTarget();

		if ( GetPRI() != None )
		{
			GetPRI().PlayerStatus = WPS_Spectating;
		}

		bIsUsingStreamingVolumes = FALSE;
	}

	simulated function EndState(Name NextStateName)
	{
		Super.EndState(NextStateName);

		bIsUsingStreamingVolumes = true;
	}

	function PlayerMove(float DeltaTime)
	{
		// don't allow player movement during startup spectating
		if (GetStateName() != 'PlayerWaiting')
		{
			Super.PlayerMove(DeltaTime);
		}
	}

	reliable client function ClientSetHUD(class<HUD> newHUDType, class<Scoreboard> newScoringType)
	{
		Global.ClientSetHUD(newHUDType,newScoringType);

		// if still spectating then pick a viewtarget
		ViewFirstSpectatorPoint();
		RefreshAllSafeZoneViewports();
	}

	function UpdateRotation(float DeltaTime)
	{
		local rotator DeltaRot, ViewRotation;
		local float MaxDeltaRot;
		local GearSpectatorPoint SpectatorPt;

		SpectatorPt = GearSpectatorPoint(GetViewTarget());
		if (SpectatorPt != None)
		{
			// Calculate Delta to be applied on ViewRotation
			DeltaRot.Yaw	= PlayerInput.aTurn;
			DeltaRot.Pitch	= PlayerInput.aLookUp;

			MaxDeltaRot = SpectatorPt.MaxRotationRate * DeltaTime;
			DeltaRot.Yaw = FClamp(DeltaRot.Yaw, -MaxDeltaRot, MaxDeltaRot);
			DeltaRot.Pitch = FClamp(DeltaRot.Pitch, -MaxDeltaRot, MaxDeltaRot);

			ViewRotation = Rotation;
			ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
			SetRotation( ViewRotation );
		}
		else
		{
			super.UpdateRotation(DeltaTime);
		}
	}

	function PlayerTick( float DeltaTime )
	{
		Global.PlayerTick(DeltaTime);

		if (BattleCamPath != None)
		{
			BattleCamPath.UpdateCameraPosition(DeltaTime);
		}
	}
}

reliable server function ServerSetBattleCamPath(int Id)
{
	BattleCamPath = GearGRI(WorldInfo.GRI).GetBattleCamPath(Id);
}

function ProcessGameOver()
{
	local GearPC PC;

	// play the anya sound
	PlayAnyaCoopDeathSound();

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		PC.ClientGameOver();
	}
}

/** displays the game over screen because a character has died */
reliable client event ClientGameOver()
{
	GearHUD_Base(MyHUD).ShowGameOverScreen();
}

state Dead
{
	ignores SeePlayer, HearNoise, KilledBy;

	function EnableAssessMode();

	function BeginState(Name PreviousStateName)
	{
		local GearPawn_CarryCrate_Base Crate;

		Super.BeginState(PreviousStateName);

		LastPawn = Pawn(ViewTarget);
		GetPRI().PlayerStatus = WPS_Dead;

		// Turn off targeting when you die
		bIsTargeting = FALSE;
		SetTargetingMode( FALSE );

		DisableAssessMode();

		if (IsLocalPlayerController())
		{
			StartPostProcessOverride(eGPP_BleedOut);
			// forcefully clear any vibration
			SetTimer(1.f,FALSE,nameof(ClientStopForceFeedbackWaveform));
		}

		// if we are not in a MP game this is a fail condition outright
		if( !WorldInfo.GRI.IsMultiplayerGame() )
		{
			if (Role == ROLE_Authority && !WorldInfo.IsPlayInEditor() && !PlayingDevLevel())
			{
				//@fixme - special hack to end the game quicker if we're in the crate carry scenario
				foreach WorldInfo.AllPawns(class'GearPawn_CarryCrate_Base',Crate)
				{
					if (!Crate.bDeleteMe && (Crate.MarcusPawn != None || Crate.DomPawn != None))
					{
						break;
					}
					Crate = None;
				}
				`log("Ending game from player death"@`showvar(self)@`showvar(Crate));
				if (Crate != None)
				{
					SetTimer(2.f, false, nameof(ProcessGameOver));
				}
				else
				{
					SetTimer(5.f, false, nameof(ProcessGameOver));
				}
			}
		}
		else
		{
			// every time we die on the client go ahead and collect garbage
			if( Role < Role_Authority )
			{
				WorldInfo.ForceGarbageCollection();
			}
		}

		// Deactivate the active tutorial if there is one
		if (TutorialMgr != none)
		{
			TutorialMgr.RebootSystem();
		}
	}

	function EndState(Name NextStateName)
	{
		local GearPRI GPRI;

		Super.EndState(NextStateName);

		GPRI = GetPRI();
		if ( GPRI != None )
		{
			GPRI.SetPlayerStatus(WPS_Dead);
		}
		// if we're being destroyed and not somehow resurrected, force the game over to happen immediately
		// None is passed when ending state due to destruction
		if (NextStateName == 'None' && LocalPlayer(Player) == None && IsTimerActive(nameof(ProcessGameOver)))
		{
			ProcessGameOver();
		}
		ClearTimer(nameof(ProcessGameOver));

		BiggestThreatWP = None;
		ClearFocusPoint(FALSE);		// clear both kismet and nonkismet lookats
		ClearFocusPoint(TRUE);

		if( MyGearHUD != None )
		{
			if (WorldInfo.GRI != None && !WorldInfo.GRI.IsMultiplayerGame())
			{
				MyGearHUD.ClearActionInfoByType(AT_StayingAlive);
				MyGearHUD.ClearActionInfoByType(AT_SuicideBomb);
			}
			MyGearHUD.ResetTaccomOpacity();
		}
		ResetPlayerMovementInput();

		if (IsLocalPlayerController())
		{
			StopPostProcessOverride(eGPP_BleedOut);
		}
	}

	exec function StartFire( optional Byte FireModeNum )
	{
		// overridden to prevent the player from respawning
	}

Begin:
	// notify the HUD so it can cleanup
	if( myHUD != None )
	{
		myHUD.PlayerOwnerDied();
	}
	if (Role == Role_Authority && WorldInfo.GRI.IsMultiplayerGame())
	{
		// setup the respawn timer if gametype supports it
		if ( GearGame(WorldInfo.Game).ShouldRespawnPC( PlayerReplicationInfo.Team.TeamIndex, self ) )
		{
			// call this on a separate timer since we can transition out of the dead state early if the user chooses to spectate immediately
			SetTimer( 3.f,FALSE,nameof(WaitForRespawn) );
		}
	}
	if (LastPawn != None)
	{
		// if we died as a meatshield then view the player who grabbed us
		if (GearPawn(LastPawn) != None && GearPawn(LastPawn).IsAHostage() && GearPawn(LastPawn).InteractionPawn != None)
		{
			// and transition immediately
			TransitionFromDeadState();
		}
		else
		{
			// otherwise focus our body
			SetViewTarget(LastPawn);
		}
		// if we died from a GDT
		if (class<GearDamageType>(LastPawn.HitDamageType) != None)
		{
			// call the static handler for any special cases
			class<GearDamageType>(LastPawn.HitDamageType).static.HandleDeadPlayer(self);
		}
	}
	if (Role == Role_Authority && WorldInfo.GRI.IsMultiplayerGame())
	{
		// have the server automatically transition after a delay to spectating
		Sleep(5.f);
		if (MyGearHUD != None)
		{
			MyGearHUD.SpectatorCameraFadeOut(FALSE,0.3f);
		}
		Sleep(0.3f);
		TransitionFromDeadState();
	}
}

/**
 * Sends the player to the correct state from dead, handles respawn checking.
 */
final function TransitionFromDeadState()
{
	if (WorldInfo.GRI.IsMultiplayerGame())
	{
		// if we died as a meatshield then view the player who grabbed us
		if (GearPawn(LastPawn) != None && GearPawn(LastPawn).IsAHostage() && GearPawn(LastPawn).InteractionPawn != None)
		{
			SetViewTarget(GearPawn(LastPawn).InteractionPawn);
			TransitionToSpectate('PlayerSpectating');
		}
		else
		{
			TransitionToSpectate(GearGRI(WorldInfo.GRI).DefaultSpectatingState);
		}
	}
}

/** Server call to send PC to the respawning state */
final function WaitForRespawn()
{
	GetPRI().PlayerStatus = WPS_Respawning;
	bWaitingToRespawn = TRUE;
	GearGame(WorldInfo.Game).UseTeamRespawn( PlayerReplicationInfo.Team.TeamIndex );
}

function Restart(bool bVehicleTransition)
{
	if (bWaitingToRespawn)
	{
		bWaitingToRespawn = FALSE;
		StartRespawnInvincibility();
	}
	Super.Restart(bVehicleTransition);

	// turn off scripted walks when taking over a Pawn to avoid issues where you get stuck in it forever
	if (MyGearPawn != None && !IsLocalPlayerController())
	{
		MyGearPawn.bScriptedWalking = false;
		MyGearPawn.bWantToUseCommLink = false;
		MyGearPawn.bWantToConverse = false;
	}
}

/** Begins the invincibility timer for respawing */
final function StartRespawnInvincibility()
{
	SetTimer( GearGame(WorldInfo.Game).RespawnInvincibilityTimer, FALSE, nameof(StopRespawnInvincibility) );
	bIsRespawnInvincible = TRUE;
}

/** Function callback to remove invincibility when respawned */
final function StopRespawnInvincibility()
{
	bIsRespawnInvincible = FALSE;
}

final function GearPRI GetViewTargetPRI()
{
	local Actor VT;

	VT = GetViewTarget();
	if ( Controller( VT ) != none )
	{
		return GearPRI(Controller( VT ).PlayerReplicationInfo);
	}
	else if ( Pawn( VT ) != none )
	{
		return GearPRI(Pawn(VT).PlayerReplicationInfo);
	}
	else
	{
		return none;
	}
}

reliable server function ServerUnPossess()
{
	local Pawn TmpPawn;
	if (Pawn != None)
	{
		TmpPawn = Pawn;
		UnPossess();
		TmpPawn.Destroy();
		GotoState('Dead');
	}
}

function TransitionToSpectateFromEndOfRound()
{
	if (Pawn != None)
	{
		ServerUnPossess();
	}
	// everyone is forced to battle cam eor
	if (GetStateName() != 'CustomSpectating')
	{
		SetTimer( 0.5f,FALSE,nameof(TransitionToSpectate) );
	}
}

/**
 * Should be to instigate a transition to spectating, automatically handles notifying server or notifying the owning client as necessary.
 */
exec function TransitionToSpectate(optional Name SpectateType)
{
	if (bWaitingToRespawn && GearGRI(WorldInfo.GRI).RespawnTime <= 1)
	{
		`log("delaying spectate change due to impending respawn");
		return;
	}
	// default to a valid spectating state
	if (SpectateType == 'None' || (SpectateType != 'CustomSpectating' && SpectateType != 'GhostSpectating' && SpectateType != 'PlayerSpectating'))
	{
		SpectateType = 'CustomSpectating';
	}
	if (IsLocalPlayerController())
	{
		// simulate
		ClientTransitionToSpectate(SpectateType);
		if (Role != ROLE_Authority)
		{
			// notify the server
			ServerSpectate(SpectateType);
		}
	}
	else
	{
		ServerSpectate(SpectateType);
		// tell the owning client to spectate as well
		ClientTransitionToSpectate(SpectateType);
	}
}

/**
 * Called when the server instigates a transition to spectating (i.e. after death).  Call TransitionToSpectate() if instigating from a local client.
 */
reliable client function ClientTransitionToSpectate(Name SpectateType)
{
	if (GetStateName() != SpectateType)
	{
		// do a simple fade between transitions (if not already viewing a battle cam)
		if (MyGearHUD != None && GetStateName() != 'PlayerWaiting' && CameraActor(ViewTarget) == None)
		{
			MyGearHUD.SpectatorCameraFadeIn();
		}
		GotoState(SpectateType);
	}
}

/**
 * Called when a client instigates a transition to spectating via TransitionToSpectate().
 */
reliable server function ServerSpectate(Name SpectateType)
{
	if (IsDead() || IsSpectating())
	{
		GotoState(SpectateType);
	}
	else
	{
		// out of sync - remind the client which state is current
		ClientGotoState(GetStateName());
	}
}

/** Stubs to be overridden by individual spectating states to handle viewtarget selection. */
function PickInitialViewTarget();
function PickNextViewTarget();
function PickPrevViewTarget();

/** @return whether we want the spectator UI scene to be displayed */
function bool WantsSpectatorUI()
{
	// yes if spectating, unless in screenshotmode
	// bunch of extra checks for SP to turn off when viewing cinematics, in splitscreen, or stuff hasn't had time to replicate
	return ( IsSpectating() && !bInMatinee && !bCinematicMode && WorldInfo.TimeSeconds - CreationTime > 1.0 &&
		(WorldInfo.GRI == None || WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.NetMode == NM_Client) &&
		 !PlayerInput.IsA('GearPlayerInput_Screenshot') );
}

/**
 * This is used for automated testing atm.
 * @todo:  move this to a specialized Spectating state for automated perf testing
 **/
state Spectating
{
	function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);

		// Follow bots around after they are spawned if automated perf testing is enabled.
		if( GearGame(WorldInfo.Game)!=None && GearGame(WorldInfo.Game).bAutomatedPerfTesting )
		{
			SetTimer( 10.0f, FALSE, nameof(SetServerViewNextPlayer) ); // get the spectating going asap instead of waiting for 30secs.  need to wait some as the player will not be spawned yet
			SetTimer( 30.0f, TRUE, nameof(ServerViewNextPlayer) );
		}

		// Deactivate the active tutorial if there is one
		if (TutorialMgr != none)
		{
			TutorialMgr.RebootSystem();
		}
	}


	function SetServerViewNextPlayer()
	{
		ServerViewNextPlayer();
	}
}

/**
 * Returns the ControllerId of a given Controller.
 * This only works for localplayers in split screen.
 */
simulated function int GetControllerId()
{
	local LocalPlayer		LP;

	LP = LocalPlayer(Player);
	if( LP != None )
	{
		return LP.ControllerId;
	}

	return -1;
}




/**
 * Default spectating state that attempts to use the level placed cameras to spectate.
 */
state CustomSpectating extends Spectating
{
	reliable client function ClientRestart(Pawn NewPawn)
	{
		Global.ClientRestart(NewPawn);
	}

	function PickInitialViewTarget()
	{
		ViewFirstSpectatorPoint();
	}

	function PickNextViewTarget()
	{
		ViewNextSpectatorPoint();
	}

	function PickPrevViewTarget()
	{
		ViewPrevSpectatorPoint();
	}

	function bool IsSpectating()
	{
		return true;
	}

	function BeginState(Name PreviousStateName)
	{
		local GearGRI GRI;
		local int PlayerIndex;
		GRI = GearGRI(WorldInfo.GRI);
		if (IsSplitScreenPlayer(PlayerIndex) && PlayerIndex != 0)
		{
			BattleCamPath = GRI.GetBattleCamPath(PlayerIndex);
			if (Role < ROLE_Authority)
			{
				ServerSetBattleCamPath(PlayerIndex);
			}
		}
		else
		{
			BattleCamPath = GRI.GetBattleCamPath(0);
		}

		super.BeginState( PreviousStateName );
		PickInitialViewTarget();
	}

	function EndState(Name NextStateName)
	{
		if (NextStateName != 'GhostSpectating' && NextStateName != 'PlayerSpectating')
		{
			Super.EndState(NextStateName);
		}
		DiscardScreenshot();
	}

	function UpdateRotation(float DeltaTime)
	{
		local rotator DeltaRot, ViewRotation;
		local float MaxDeltaRot;
		local GearSpectatorPoint SpectatorPt;

		SpectatorPt = GearSpectatorPoint(GetViewTarget());
		if (SpectatorPt != None)
		{
			// Calculate Delta to be applied on ViewRotation
			DeltaRot.Yaw	= PlayerInput.aTurn;
			DeltaRot.Pitch	= PlayerInput.aLookUp;

			MaxDeltaRot = SpectatorPt.MaxRotationRate * DeltaTime;
			DeltaRot.Yaw = FClamp(DeltaRot.Yaw, -MaxDeltaRot, MaxDeltaRot);
			DeltaRot.Pitch = FClamp(DeltaRot.Pitch, -MaxDeltaRot, MaxDeltaRot);

			ViewRotation = Rotation;
			ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
			SetRotation( ViewRotation );
		}
		else
		{
			super.UpdateRotation(DeltaTime);
		}
	}

	function PlayerTick( float DeltaTime )
	{
		local GearPlayerCamera GearCam;

		Global.PlayerTick(DeltaTime);

		if (BattleCamPath != None)
		{
			BattleCamPath.UpdateCameraPosition(DeltaTime);

			// pass along control info
			GearCam = GearPlayerCamera(PlayerCamera);
			if (GearCam != None)
			{
				GearCam.SpectatorCam.ControlInfo_LookRight = PlayerInput.RawJoyLookRight;
				GearCam.SpectatorCam.ControlInfo_LookUp = PlayerInput.RawJoyLookUp;
				GearCam.SpectatorCam.ControlInfo_Zoom = PlayerInput.RawJoyUp;
			}
		}
	}
}

/**
 * Allows the player to walk around the world with only world collision to spectate.
 */
state GhostSpectating extends CustomSpectating
{
	function PickInitialViewTarget()
	{
		local ViewTargetTransitionParams TransParms;
		TransParms.BlendTime = 0.5f;
		ServerViewSelf(TransParms);
	}

	function PickNextViewTarget();
	function PickPrevViewTarget();

	event Touch( Actor Other, PrimitiveComponent OtherComp, vector HitLocation, vector HitNormal )
	{
		// bounce off of other players when moving
		if (GearPawn(Other) != None)
		{
			Velocity -= Normal(Other.Location - Location) * 2048.f;
		}
	}

	function PlayerMove(float DeltaTime)
	{
		local vector X,Y,Z;

		GetAxes(PlayerCamera.Rotation,X,Y,Z);
		Acceleration = PlayerInput.aForward*X + PlayerInput.aStrafe*Y + PlayerInput.aUp*vect(0,0,1);
		UpdateRotation(DeltaTime);

		if (Role < ROLE_Authority) // then save this move and replicate it
		{
			ReplicateMove(DeltaTime, Acceleration, DCLICK_None, rot(0,0,0));
		}
		else
		{
			ProcessMove(DeltaTime, Acceleration, DCLICK_None, rot(0,0,0));
		}
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local float		VelSize;

		/* smoothly accelerate and decelerate */
		Acceleration = Normal(NewAccel) * SpectatorCameraSpeed;

		VelSize = VSize(Velocity);
		if( VelSize > 0 )
		{
			Velocity = Velocity - (Velocity - Normal(Acceleration) * VelSize) * FMin(DeltaTime * 8, 1);
		}

		Velocity = Velocity + Acceleration * DeltaTime;
		if( VSize(Velocity) > SpectatorCameraSpeed )
		{
			Velocity = Normal(Velocity) * SpectatorCameraSpeed;
		}

		LimitSpectatorVelocity();

		if (Role == ROLE_AutonomousProxy)
		{
			AutonomousPhysics(DeltaTime);
		}
	}

	event Landed(vector HitNormal, Actor FloorActor)
	{
		// set physics to walking otherwise it'll default back to none
		SetPhysics(PHYS_Walking);
	}

	function BeginState(Name PreviousStateName)
	{
		local vector TraceDir, HitL, HitN, CamLoc;
		local Actor HitA;
		local rotator CamRot;
		local NavigationPoint Nav;
		local vector CurrCameraLocation;

		CurrCameraLocation = GearPlayerCamera(PlayerCamera).Location;
		Super.BeginState(PreviousStateName);

		// try to place the ghost camera intelligently
		if ( PreviousStateName != GetStateName() )
		{
			if (IsLocalPlayerController() )
			{
				CamLoc = GearPlayerCamera(PlayerCamera).Location;
				TraceDir = vector(GearPlayerCamera(PlayerCamera).Rotation);
				HitA = Trace(HitL, HitN, CamLoc + TraceDir * 5000.f, CamLoc, TRUE, vect(1,1,1));
				if (HitA != None)
				{
					// try to find the nearest anchor point
					Nav = class'NavigationPoint'.static.GetNearestNavToPoint(HitA,HitL);
					if (Nav != None)
					{
						SetLocation(Nav.Location + vect(0,0,32));
						ServerSetSpectatorLocation(Location);
						CamRot = GearPlayerCamera(PlayerCamera).Rotation;
						CamRot.Pitch = 0;
						SetRotation(CamRot);
					}
				}
			}
			Velocity += vector(Rotation) * 2048.f;
		}
		else
		{
			if (IsLocalPlayerController() )
			{
				SetLocation(CurrCameraLocation);
				ServerSetSpectatorLocation(CurrCameraLocation);
				CamRot = GearPlayerCamera(PlayerCamera).Rotation;
				CamRot.Pitch = 0;
				SetRotation(CamRot);
			}
		}

		SetCollision(TRUE,FALSE);
		SetCollisionSize(22.f,35.f);
		SetPhysics(PHYS_Walking);
	}

	function EndState(Name NextStateName)
	{
		SetPhysics(PHYS_None);
		SetCollisionSize(22.f,22.f);
		Super.EndState(NextStateName);
	}


	function PlayerTick( float DeltaTime )
	{
		Global.PlayerTick(DeltaTime);
	}

	function UpdateRotation(float DeltaTime)
	{
		local rotator NewRot;
		Global.UpdateRotation(DeltaTime);
		// make sure no roll sneaked in
		if (Rotation.Roll != 0)
		{
			NewRot = Rotation;
			NewRot.Roll = 0;
			SetRotation(NewRot);
		}
	}
}

/**
 * Allows the player to view other players to spectate.
 */
state PlayerSpectating extends CustomSpectating
{
	function PickInitialViewTarget()
	{
		ServerViewNearestPlayer();
		// if there is no available target then go to battle cam
		if (Role == ROLE_Authority && (ViewTarget == None || ViewTarget == self))
		{
			TransitionToSpectate('CustomSpectating');
		}
	}

	function PickNextViewTarget()
	{
		ServerViewNextPlayer();
	}

	function PickPrevViewTarget()
	{
		ServerViewPrevPlayer();
	}

	function PlayerTick( float DeltaTime )
	{
		if ( ViewTarget == None || ViewTarget.bDeleteMe || ViewTarget == self ||
			(GearPawn(ViewTarget) != None && ViewTarget.IsInState('Dying') && WorldInfo.TimeSeconds - GearPawn(ViewTarget).TimeOfDeath > 6.0f) )
		{
			PickInitialViewTarget();
		}
		Super.PlayerTick(DeltaTime);
	}
}
/** used when the client needs to tell the server about raw button presses */
reliable server function ServerHandleButtonPress(name ButtonName);
function TriggerEngageLoop();

state Engaging extends PlayerWalking
{
	function EndState( Name NextStateName )
	{
		local BodyStance IdleStance;

		if (!IsDoingSpecialMove(SM_Engage_ForceOff) && !MyGearPawn.IsDBNO())
		{
			if (MyGearPawn != None)
			{
				// stop the idle
				IdleStance.AnimName[BS_FullBody]=MyGearPawn.EngageTrigger.ENGAGE_AnimName_Idle;
				MyGearPawn.BS_Stop( IdleStance, 0.2f );
			}

			// release the engage actor
			if (LocalPlayer(Player) != None)
			{
				DoSpecialMove(SM_Engage_End, IsDoingSpecialMove(SM_Engage_Idle));
			}
		}
	}

	function HandleButtonPress(coerce name ButtonName)
	{
		local bool bDoInput;

		bDoInput = (InputIsDisabledCount == 0) ? TRUE : FALSE;

		// need to tell server to do the engage thing
		if ( (ButtonName == 'X' || ButtonName == 'A') && bDoInput )
		{
			ServerHandleButtonPress(ButtonName);
		}
	}

	reliable server function ServerHandleButtonPress(name ButtonName)
	{
		local Trigger_Engage Engage;
		local bool bCanDoMove;
		local GearPC OtherPC;
		local array<SequenceEvent>	EngageEvents;
		bCanDoMove = FALSE;
		switch( ButtonName )
		{
			// Release from the engage actor and reset the state.
			case 'X':
				if ( IsDoingSpecialMove(SM_Engage_Idle) )
				{
					GotoState('PlayerWalking');
					ClientGotoState('PlayerWalking');
				}
			break;

			// Do the engage loop if we can
			case 'A':
				Engage = MyGearPawn.EngageTrigger;
				if ( Engage != None && Engage.LoopCounter > 0 && (WorldInfo.TimeSeconds - Engage.LastTurnTime >= Engage.ENGAGE_LoopDelay) && Engage.FindEventsOfClass(class'SeqEvt_Engage',EngageEvents) && EngageEvents[0].bEnabled && !IsDoingSpecialMove(SM_Engage_Start) && !IsDoingSpecialMove(SM_Engage_Loop) && !IsDoingSpecialMove(SM_Engage_End) && !IsDoingSpecialMove(SM_Engage_ForceOff))
				{
					// check for a linked trigger
					if (Engage.LinkedCoopTrigger != None)
					{
						Engage.LastAttemptTurnTime = WorldInfo.TimeSeconds;
						if (Engage.LinkedCoopTrigger.EngagedPawn != None && Abs(Engage.LastAttemptTurnTime - Engage.LinkedCoopTrigger.LastAttemptTurnTime) < 1.f)
						{
							bCanDoMove = TRUE;
							// start the move on the other trigger
							OtherPC = GearPC(Engage.LinkedCoopTrigger.EngagedPawn.Controller);
							if (OtherPC != None)
							{
								OtherPC.TriggerEngageLoop();
							}
							// else AI, which polls for the LastAttemptTurnTime anyways, so no action needed
						}
					}
					else
					{
						// no restriction for unlinked triggers
						bCanDoMove = TRUE;
					}
					if (bCanDoMove)
					{
						TriggerEngageLoop();
					}
				}
			break;
		}
	}

	function TriggerEngageLoop()
	{
		ServerDictateSpecialMove(SM_Engage_Loop);
	}

	/** Override and ignore */
	function HandleButtonRelease(coerce Name ButtonName);
	function CheckForInteractionEvents();

	function PlayerMove(float DeltaTime)
	{
		if( Pawn == None )
		{
			GotoState('dead');
			return;
		}

		UpdateRotation( DeltaTime );

		if( Role < ROLE_Authority ) // then save this move and replicate it
		{
			ReplicateMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		}
		else
		{
			ProcessMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		}
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot);
}



simulated function ViewFirstSpectatorPoint()
{
	// looking for first point, so reset to beginning
	if (BattleCamPath != None)
	{
		BattleCamPath.ResetToStartingPosition();
	}

	ServerViewFirstSpectatorPoint();
}

unreliable server function ServerViewFirstSpectatorPoint()
{
	local CameraActor SpectatorPathCamera;
	// if it's coop then only allow them to view players
	if ( IsSpectating() && !IsActuallyPlayingCoop() )
	{
		if (BattleCamPath != None)
		{
			// attach to the camera path actor
			SpectatorPathCamera = BattleCamPath.GetAssociatedCameraActor();
			// set view target to the camera actor connected
			SetViewTarget(SpectatorPathCamera);
			ClientSetViewTarget(SpectatorPathCamera);
		}
		else
		{
			CurrSpectatorPoint = GearGame(WorldInfo.Game).GetFirstSpectatorPoint();
			if ( CurrSpectatorPoint != None )
			{
				SetViewTarget( CurrSpectatorPoint );
				ClientSetViewTarget( CurrSpectatorPoint );
			}
			else
			{
				// otherwise send them to a player
				ServerViewNextPlayer();
			}
		}
	}
}

simulated function ViewNextSpectatorPoint()
{
	if ( IsSpectating() && !IsActuallyPlayingCoop() )
	{
		if (BattleCamPath != None)
		{
			if (CameraActor(ViewTarget) == None)
			{
				ServerViewFirstSpectatorPoint();
			}
			else
			{
				// we handle spectator camera paths client-side, since we're not changing viewtargets.
				if (BattleCamPath.bPaused || !BattleCamPath.bIsPlaying)
				{
					PlaySound(SoundCue'Music_Stingers.CameraZoom.CameraZoomA_Cue', TRUE);
				}
				BattleCamPath.MoveToNextStop();
			}
		}
		else
		{
			ServerViewNextSpectatorPoint();
		}
	}
}

unreliable server function ServerViewNextSpectatorPoint()
{
	if ( IsSpectating() && !IsActuallyPlayingCoop() )
	{
		if ( (CurrSpectatorPoint == None) || (CurrSpectatorPoint == GetViewTarget()) )
		{
			CurrSpectatorPoint = GearGame(WorldInfo.Game).GetNextSpectatorPoint( CurrSpectatorPoint );
		}

		if ( CurrSpectatorPoint != None )
		{
			SetViewTarget( CurrSpectatorPoint );
			ClientSetViewTarget( CurrSpectatorPoint );
		}
		else
		{
			// otherwise send them to a player
			ServerViewNextPlayer();
		}
	}
}

simulated function ViewPrevSpectatorPoint()
{
//	`log("***"@GetFuncName()@IsSpectating()@!IsCoop()@WorldInfo.GRI@GearGRI(WorldInfo.GRI).CurrentLevelSpectatorCameraPath);

	if ( IsSpectating() && !IsActuallyPlayingCoop() )
	{
		if (BattleCamPath != None)
		{
			// we handle spectator camera paths client-side, since we're not changing viewtargets.
			if (BattleCamPath.bPaused || !BattleCamPath.bIsPlaying)
			{
				PlaySound(SoundCue'Music_Stingers.CameraZoom.CameraZoomA_Cue', TRUE);
			}
			BattleCamPath.MoveToPrevStop();
		}
		else
		{
			ServerViewPrevSpectatorPoint();
		}
	}
}

unreliable server function ServerViewPrevSpectatorPoint()
{
	if ( IsSpectating() && !IsActuallyPlayingCoop() )
	{
		if ( (CurrSpectatorPoint == None) || (CurrSpectatorPoint == GetViewTarget()) )
			CurrSpectatorPoint = GearGame(WorldInfo.Game).GetPrevSpectatorPoint( CurrSpectatorPoint );
		if ( CurrSpectatorPoint != None )
		{
			SetViewTarget( CurrSpectatorPoint );
			ClientSetViewTarget( CurrSpectatorPoint );
		}
		else
		{
			// otherwise send them to a player
			ServerViewNextPlayer();
		}
	}
}

function PawnDied(Pawn inPawn)
{
	if (inPawn != Pawn)
	{
		return;
	}
	// unclaim the cover if necessary
	if (MyGearPawn != None &&
		MyGearPawn.CurrentLink != None)
	{
		MyGearPawn.CurrentLink.UnClaim(MyGearPawn,-1,TRUE);
	}
	MyGearPawn = None;
	Super.PawnDied(inPawn);
}

/**
 * list important GearPController variables on canvas. HUD will call DisplayDebug() on the current ViewTarget when
 * the ShowDebug exec is used
 *
 * @param	HUD		- HUD with canvas to draw on
 * @input	out_YL		- Height of the current font
 * @input	out_YPos	- Y position on Canvas. out_YPos += out_YL, gives position to draw text for next debug line.
 */
simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
	local Canvas Canvas;

	Canvas = HUD.Canvas;
	Canvas.SetPos(0,0);

	if ( Enemy != None )
		Canvas.DrawText("     STATE: "$GetStateName()$" Timer: "$GetTimerCount()$" Enemy "$Enemy.GetHumanReadableName(), false);
	else
		Canvas.DrawText("     STATE: "$GetStateName()$" Timer: "$GetTimerCount()$" NO Enemy ", false);
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	if( PlayerReplicationInfo == None )
		Canvas.DrawText("     NO PLAYERREPLICATIONINFO", false);
	else
		PlayerReplicationInfo.DisplayDebug(HUD, out_YL, out_YPos);

	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	if (HUD.ShouldDisplayDebug('camera'))
	{
		if( PlayerCamera != None )
		{
			PlayerCamera.DisplayDebug( HUD, out_YL, out_YPos );
		}
		else
		{
			Canvas.SetDrawColor(255,0,0);
			Canvas.DrawText("NO CAMERA");
			out_YPos += out_YL;
			Canvas.SetPos(4, out_YPos);
		}
	}
	if ( HUD.ShouldDisplayDebug('input') )
	{
		HUD.Canvas.SetDrawColor(255,0,0);
		HUD.Canvas.DrawText("Input ignoremove "$bIgnoreMoveInput$" ignore look "$bIgnoreLookInput);
		out_YPos += out_YL;
		HUD.Canvas.SetPos(4, out_YPos);
	}

	if( Pawn == None )
	{
		super.DisplayDebug(HUD, out_YL, out_YPos);
		return;
	}

	Canvas.SetDrawColor(255,255,255,255);
	Canvas.DrawText("CONTROLLER "$GetItemName(string(self))$" Pawn "$GetItemName(string(Pawn)));
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	Canvas.DrawText("Team Num:"@GetTeamNum()@"Pawn Team Num:"@Pawn.GetTeamNum());
	out_YPos += out_YL;
	Canvas.SetPos(4, out_YPos);

	if (MyGearPawn != None && MyGearPawn.IsDBNO())
	{
		out_YPos += out_YL;
		Canvas.DrawText("StartTimeOfDBNO:"@MyGearPawn.TimeOfDBNO);
		out_YPos += out_YL;
		Canvas.SetPos(4, out_YPos);
	}


}

/*
 * MP cover debugging
*/
exec function MPCoverCheck()
{
	ServerCoverCheck();
}

unreliable server function ServerCoverCheck()
{
	local bool bReachPreciseDestination, bReachedPreciseDestination, bReachPreciseRotation, bReachedPreciseRotation;

	if ( (MyGearPawn.SpecialMove != SM_None) && (MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove] != None) )
	{
		bReachPreciseDestination = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachPreciseDestination;
		bReachedPreciseDestination = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachedPreciseDestination;
		bReachPreciseRotation = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachPreciseRotation;
		bReachedPreciseRotation = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachedPreciseRotation;
	}

	ClientCoverCheck(GetStateName(), MyGearPawn.IsInCover(), MyGearPawn.IsDoingMove2IdleTransition(), MyGearPawn.SpecialMove,
					bBreakFromCover, bIgnoreMoveInput, MyGearPawn.bIsMirrored, MyGearPawn.CoverAction,
					bPreciseDestination, MyGearPawn.CurrentLink, MyGearPawn.CurrentSlotIdx, bReachPreciseDestination, bReachedPreciseDestination,
					bReachPreciseRotation, bReachedPreciseRotation);
}

unreliable client function ClientCoverCheck(name CurrentStateName, bool bServerIsInCover, bool bServerIsDoingMove2IdleTransition, ESpecialMove ServerSpecialMove,
					bool bServerBreakFromCover, byte bServerIgnoreMoveInput, bool bServerIsMirrored,
					ECoverAction ServerCoverAction, bool bServerPreciseDestination, CoverLink ServerLink, int ServerSlotIdx,
					bool bServerReachPreciseDestination, bool bServerReachedPreciseDestination,
					bool bServerReachPreciseRotation, bool bServerReachedPreciseRotation)
{
	local string Result;
	local bool bFoundDiff;
	local bool bReachPreciseDestination, bReachedPreciseDestination, bReachPreciseRotation, bReachedPreciseRotation;

	Result = "MPCoverCheck ";

	if ( (MyGearPawn.SpecialMove != SM_None) && (MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove] != None) )
	{
		bReachPreciseDestination = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachPreciseDestination;
		bReachedPreciseDestination = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachedPreciseDestination;
		bReachPreciseRotation = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachPreciseRotation;
		bReachedPreciseRotation = MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bReachedPreciseRotation;
	}

	if ( GetStateName() != CurrentStateName )
	{
		bFoundDiff = true;
		Result = Result$" State Client "$GetStateName()$" Server "$CurrentStateName;
	}
	else
	{
		Result = Result@GetStateName();
	}

	if ( bServerIsInCover != MyGearPawn.IsInCover() )
	{
		bFoundDiff = true;
		Result = Result$" IsInCover Client "$MyGearPawn.IsInCover()$" Server "$bServerIsInCover;
	}
	if ( bServerIsDoingMove2IdleTransition != MyGearPawn.IsDoingMove2IdleTransition() )
	{
		bFoundDiff = true;
		Result = Result$" Move2Idle Client "$MyGearPawn.IsDoingMove2IdleTransition()$" Server "$bServerIsDoingMove2IdleTransition;
	}
	if ( ServerSpecialMove != MyGearPawn.SpecialMove )
	{
		bFoundDiff = true;
		Result = Result$" SpecialMove Client "$MyGearPawn.SpecialMove$" Server "$ServerSpecialMove;
	}
	if ( bBreakFromCover != bServerBreakFromCover )
	{
		bFoundDiff = true;
		Result = Result$" BreakFromCover Client "$bBreakFromCover$" Server "$bServerBreakFromCover;
	}
	if ( bServerIsMirrored != MyGearPawn.bIsMirrored )
	{
		bFoundDiff = true;
		Result = Result$" bIsMirrored Client "$MyGearPawn.bIsMirrored$" Server "$bServerIsMirrored;
	}
	if ( ServerCoverAction != MyGearPawn.CoverAction )
	{
		bFoundDiff = true;
		Result = Result$" CoverAction Client "$MyGearPawn.CoverAction$" Server "$ServerCoverAction;
	}
	if ( bServerPreciseDestination != bPreciseDestination )
	{
		bFoundDiff = true;
		Result = Result$" bPreciseDestination Client "$bPreciseDestination$" Server "$bServerPreciseDestination;
	}
	if ( ServerLink != MyGearPawn.CurrentLink )
	{
		bFoundDiff = true;
		Result = Result$" CurrentLink Client "$MyGearPawn.CurrentLink$" Server "$ServerLink;
	}
	if ( ServerSlotIdx != MyGearPawn.CurrentSlotIdx )
	{
		bFoundDiff = true;
		Result = Result$" SlotIdx Client "$MyGearPawn.CurrentSlotIdx$" Server "$ServerSlotIdx;
	}

	if ( bServerIgnoreMoveInput != bIgnoreMoveInput )
	{
		bFoundDiff = true;
		Result = Result$" bIgnoreMoveInput Client "$bIgnoreMoveInput$" Server "$bServerIgnoreMoveInput;
	}
	else if ( bIgnoreMoveInput != 0 )
	{
		bFoundDiff = true;
		Result = Result$" bIgnoreMoveInput is "$bIgnoreMoveInput;
	}
	if ( !bFoundDiff )
	{
		Result = "MPCoverCheck in state "$GetStateName()$" - no synch issues found.";
	}

	if ( bServerReachPreciseDestination != bReachPreciseDestination )
	{
		bFoundDiff = true;
		Result = Result$" bReachPreciseDestination Client "$bReachPreciseDestination$" Server "$bServerReachPreciseDestination;
	}
	if ( bServerReachedPreciseDestination != bReachedPreciseDestination )
	{
		bFoundDiff = true;
		Result = Result$" bReachedPreciseDestination Client "$bReachedPreciseDestination$" Server "$bServerReachedPreciseDestination;
	}
	if ( bServerReachPreciseRotation != bReachPreciseRotation )
	{
		bFoundDiff = true;
		Result = Result$" bReachPreciseRotation Client "$bReachPreciseRotation$" Server "$bServerReachPreciseRotation;
	}
	if ( bServerReachedPreciseRotation != bReachedPreciseRotation )
	{
		bFoundDiff = true;
		Result = Result$" bReachedPreciseRotation Client "$bReachedPreciseRotation$" Server "$bServerReachedPreciseRotation;
	}

	if ( bFoundDiff )
		`log(Result);
	if ( (MyGearPawn != None) && (Role < ROLE_Authority) )
		ServerCoverCheck();
}

exec function SetRadius(float newradius)
{
	Pawn.SetCollisionSize(NewRadius,Pawn.GetCollisionHeight());
}

exec function GiveItem(string InvClassName)
{
	ServerGiveItem(InvClassName);
}

reliable server function ServerGiveItem(string InvClassName)
{
 	local class<Inventory> IClass;
	local Inventory Item;

    if ( InStr(InvClassName,".") == -1 )
	{
    	InvClassName = "GearGame." $ InvClassName;
	}

    if (Pawn==None)
    {
    	`log("ServerGiveItem: Needs a pawn");
		return;
    }

   	IClass = class<Inventory>(DynamicLoadObject(InvClassName, class'class'));
	if (IClass != None)
	{
		Item = Spawn(IClass,,, Pawn.Location);
		if (Item!=None)
		{
			Item.GiveTo(Pawn);
		}
    	else
		{
			`log("ServerGiveItem: Could Not Spawn");
		}
    }
}

/**
 * Called by PlayerController and PlayerInput to set bIsWalking flag, affecting Pawn's velocity
 */
function HandleWalking()
{
	if (MyGearPawn != None)
	{
		// if currently forced (scripting), input, weapon + targeting
		if ( ( bRun == 0 ) ||
			 ( bIsTargeting && (MyGearPawn.MyGearWeapon != None) && MyGearPawn.MyGearWeapon.bForceWalkWhenTargeting ) ||
			 ( (MyGearPawn.CameraVolumes.length > 0) && MyGearPawn.CameraVolumes[0].bForcePlayerToWalk )
			)
		{
			MyGearPawn.SetWalking(TRUE);
		}
		else
		{
			MyGearPawn.SetWalking(FALSE);
		}
	}
}


/************************************************************************************
 * Targeting Mode
 ***********************************************************************************/

/** return TRUE if wants to Target */
final function bool WantsToTarget()
{
	if ( !IsButtonActive(GB_LeftTrigger) &&
		( (MyGearPawn == None) || !MyGearPawn.ShouldForceTargeting() ) )
	{
		return FALSE;
	}

	if (IsDead())
	{
		return FALSE;
	}

	if (MyGearPawn != None)
	{
		if ( MyGearPawn.ShouldPreventTargeting() )
		{
			return FALSE;
		}
	}

	return TRUE;
}


/**
 * Returns TRUE if player is able to target.
 */
final function bool CanTarget()
{
	local GearTurret			GT;
	local GearWeap_HeavyBase	MyHeavyWeapon;
	local bool					bReloadingWeaponPreventsTargeting;

	if( MyGearPawn == None )
	{
		// check if it's a turret
		GT = GearTurret(Pawn);
		if (GT == None)
		{
			return FALSE;
		}
		else
		{
			return GT.bAllowTargetingCamera;
		}
	}
	else
	{
		// Need to be alive and have a weapon to target.
		if( IsDead() || MyGearPawn.MyGearWeapon == None )
		{
			return FALSE;
		}

		bReloadingWeaponPreventsTargeting = MyGearPawn.MyGearWeapon.ShouldReloadingPreventTargeting();
		// Reloading weapon breaks targeting mode.
		if( bReloadingWeaponPreventsTargeting && MyGearPawn.IsReloadingWeapon() )
		{
			return FALSE;
		}

		// See if we can fire weapon. Special test for targeting, as some weapons shouldn't block targeting while reloading. (heavy/shield).
		if( !MyGearPawn.CanFireWeapon(TRUE) )
		{
			return FALSE;
		}

		// If in cover and CA_Default can't target unless in 360 aiming.
		// Outside of 360 aiming, an action is required (blind firing or leaning).
		if( MyGearPawn.CoverType != CT_None && MyGearPawn.CoverAction == CA_Default && !MyGearPawn.bDoing360Aiming )
		{
			return FALSE;
		}

		// cannot target while jumping around
		MyHeavyWeapon = GearWeap_HeavyBase(MyGearPawn.MyGearWeapon);
		if( MyHeavyWeapon != None && (MyGearPawn.SpecialMove == SM_CoverSlip || MyGearPawn.SpecialMove == SM_MidLvlJumpOver || MyGearPawn.IsEvading()) )
		{
			return FALSE;
		}
	}

	return TRUE;
}


/**
 * Changes the state of the targeting mode flag, and replicates it to the server.
 */
final function SetTargetingMode(bool bNewIsTargeting)
{
	bIsTargeting = bNewIsTargeting;
	TargetingModeChanged();
	// update the server if necessary
	if( Role < Role_Authority )
	{
		ServerSetTargetingMode(bNewIsTargeting);
	}
}

/**
 * Server update of the targeting mode flag.
 */
reliable server final function ServerSetTargetingMode(bool bNewIsTargeting)
{
	if( bIsTargeting != bNewIsTargeting )
	{
		SetTargetingMode(bNewIsTargeting);
	}
}

/**
 * Called when TargetingMode state changes from SetTargetinMode().
 */
final function TargetingModeChanged()
{
	// Update Pawn with new flag. Which will get replicated to remote clients,
	// and affect things such as animations and camera.
	if( MyGearPawn != None )
	{
		MyGearPawn.SetTargetingMode(bIsTargeting);
	}
}


/************************************************************************************
 * Cover
 ***********************************************************************************/

/** Special cover debug log formatting */
function CoverLog(string msg, coerce string function)
{
	`log( "[" $ WorldInfo.TimeSeconds $"]" @ "(" $ GetStateName() $ "::" $ function $ ")" @ msg, bDebugCover);
}

reliable client function ClientAcquireCover(CovPosInfo CovInfo, optional bool bNoCameraAutoAlign)
{
	if (IsLocalPlayerController())
	{
		AcquireCover(CovInfo, bNoCameraAutoAlign);
	}
}

/*
 * Cover acquisition pitch:
 * Cover can only be acquired on a client or standalone game by calling AcquireCover()
 * Cover is replicated to the server through GearServerMove(), and confirmed from the server in GearClientAdjustPosition()
 */

/**
 * Tell Player to acquire given cover spot.
 * @param	CovInfo				Desired cover spot
 * @param	bNoCameraAutoAlign	TRUE to forcibly deny camera reorientation upon entering cover
 */
function AcquireCover(CovPosInfo CovInfo, optional bool bNoCameraAutoAlign)
{
	// should only call AcquireCover() if locally controlled!
	if ( !IsLocalPlayerController() )
	{
		`log(WorldInfo.TimeSeconds@GetFuncName() @ "has to be called from local player!");
		ScriptTrace();
		return;
	}

	if( MyGearPawn == None )
	{
		return;
	}

	CoverLog("", GetFuncName());

	// Acquire cover
	CoverAcquired(CovInfo, bNoCameraAutoAlign);
}

function DumpCoverInfo(CovPosInfo CovInfo)
{
	`log(" Link:" @ CovInfo.Link @ "Location:" @ CovInfo.Location @ "Normal:" @ CovInfo.Normal);
	`log(" LtSlotIdx:" @ CovInfo.LtSlotIdx @ "RtSlotIdx:" @ CovInfo.RtSlotIdx @ "LtToRtPct:" @ CovInfo.LtToRtPct);
}

/**
 * Event called when cover has been acquired.
 */
function CoverAcquired(CovPosInfo CovInfo, optional bool bNoCameraAutoAlign)
{
	local rotator TowardsCoverRot, CamRot;
	local float CamTurnDelta;
	local vector CamLoc;
	local ECoverDirection InitialCoverDirection;
	local SeqEvt_EnteredCover Evt;

	foreach EnteredCoverEvents(Evt)
	{
		Evt.CheckActivate(self,MyGearPawn,FALSE);
	}

	CoverLog("Link:"@CovInfo.Link@"Slot:"@CovInfo.LtSlotIdx$"/"$CovInfo.RtSlotIdx@"LtToRt:"@CovInfo.LtToRtPct, GetFuncName());
	//DumpCoverInfo(CovInfo);

	// figure out the initial cover direction
	//@NOTE - THIS HAS TO HAPPEN BEFORE CoverAcquired() otherwise server will not have the corrected ReplicatedCoverDirection
	InitialCoverDirection = MyGearPawn.FindBestCoverSideFor( CovInfo );
	//`log(WorldInfo.TimeSeconds@GetFuncName()@InitialCoverDirection);
	if( InitialCoverDirection != CD_Default )
	{
		MyGearPawn.SetMirroredSide( InitialCoverDirection == CD_Left );
	}

	// Tell Pawn that we've acquired cover
	MyGearPawn.CoverAcquired(CovInfo);

	// Should we notify that we've reached a slot?
	if( CovInfo.LtToRtPct == 0.f || CovInfo.LtToRtPct == 1.f )
	{
		NotifyReachedCoverSlot(MyGearPawn.CurrentSlotIdx, -1);
	}

	// disable roadie run timer, which would pull player back in cover.
	ClearTimer('TryToRoadieRun');

	// experimental
	// spin camera to face cover wall
	if (!bNoCameraAutoAlign && ShouldAutoAlignCameraWithCover())
	{
		// spin camera, experimental
		GetPlayerViewPoint(CamLoc, CamRot);
		TowardsCoverRot = rotator(-CovInfo.Normal);
		CamTurnDelta = NormalizeRotAxis(TowardsCoverRot.Yaw - CamRot.Yaw);
		GearPlayerCamera(PlayerCamera).GameplayCam.BeginTurn(0, CamTurnDelta, 0.5f, 0.f, TRUE);
	}

	// transition to the cover state
	GotoState('PlayerTakingCover');

	// Call the AcquiredCover delegates
	TriggerGearEventDelegates( eGED_CoverAcquired );
}


simulated function bool ShouldAutoAlignCameraWithCover()
{
	local GearPawn	TestPawn;
	local vector	CoverNormal, PawnFVec, PawnRVec, PawnUVec;
	local vector	CamLoc, CamDir, TestDir;
	local rotator	CamRot;
	local int SlotIdx;

	if ( !IsLocalPlayerController() )
		return false;

	if (MyGearPawn == None)
	{
		// bah
		return FALSE;
	}

	// if player is doing something with the stick, don't fight him
	if ( (PlayerInput.aTurn != 0) || (PlayerInput.aLookUp != 0) )
	{
		return FALSE;
	}

	CoverNormal = MyGearPawn.AcquiredCoverInfo.Normal;
	SlotIdx = MyGearPawn.GetSlotIdxByPct();

	if (MyGearPawn.CurrentLink.Slots[SlotIdx].bLeanLeft)
	{
		GetAxes(rotator(MyGearPawn.Velocity), PawnFVec, PawnRVec, PawnUVec);

		if ( ( (VSizeSq(MyGearPawn.Velocity) == 0.f) || ((PawnRVec dot CoverNormal) > 0.f) ) && (RemappedJoyUp != 0 || RemappedJoyRight != 0) )
		{
			return FALSE;
		}
	}
	else if (MyGearPawn.CurrentLink.Slots[SlotIdx].bLeanRight)
	{
		GetAxes(rotator(MyGearPawn.Velocity), PawnFVec, PawnRVec, PawnUVec);

		if ( ( (VSizeSq(MyGearPawn.Velocity) == 0.f) || ((PawnRVec dot CoverNormal) < 0.f) ) && (RemappedJoyUp != 0 || RemappedJoyRight != 0) )
		{
			return FALSE;
		}
	}
	else
	{
		// can't lean
		return FALSE;
	}

	// am I, roughly, aiming at an enemy?
	// this is the slower check, do it last

	GetPlayerViewPoint(CamLoc, CamRot);
	CamDir = vector(CamRot);
	foreach WorldInfo.AllPawns(class'GearPawn', TestPawn)
	{
		// 0.9238f is cos(22.5 deg), making a detection fov of 45deg
		TestDir = TestPawn.Location - CamLoc;
		if ( VSize(TestDir) <= 8192.f && (CamDir dot TestDir) > 0.9238f &&
			LineOfSightTo(TestPawn) )
		{
			return false;
		}
	}

	// survived the gauntlet, return true!
	return TRUE;
}

/**
 * Clears the cover claim and resets the state.
 *
 * NET: Server/Client
 */
simulated function LeaveCover(optional bool bPushOut)
{
	local SeqEvt_LeftCover Evt;
	if( MyGearPawn != None )
	{
		CoverLog("Leaving cover"@MyGearPawn.CurrentLink, GetFuncName());

		// Call the LeaveCover delegates
		TriggerGearEventDelegates( eGED_CoverLeave );

		// revoke our claim on the cover
		if (MyGearPawn.CurrentLink != None)
		{
			MyGearPawn.CurrentLink.UnClaim( MyGearPawn, MyGearPawn.CurrentSlotIdx, TRUE );
		}

		foreach LeftCoverEvents(Evt)
		{
			Evt.CheckActivate(self,MyGearPawn,FALSE);
		}

		MyGearPawn.LeaveCover();
		MyGearPawn.SetAnchor(None);

		if (MyGearHud != None)
		{
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}

		// transition to proper state
		GotoState('PlayerWalking');
	}
}

/**
 * Called when we've reached a slot, checks for stationary slots.
 */
function NotifyReachedCoverSlot(int SlotIdx, int OldSlotIdx)
{
	local CoverLink Link;
	Link = MyGearPawn.CurrentLink;
	// Handle stationary slots
	if( Link != None )
	{
		CoverLog("Slot:"@SlotIdx,GetFuncName()@"Cur"@MyGearPawn.CurrentSlotIdx@"L"@MyGearPawn.LeftSlotIdx@"R"@MyGearPawn.RightSlotIdx@Link.IsStationarySlot(SlotIdx));

		// Unclaim old slot
		if( OldSlotIdx >= 0 )
		{
			Link.UnClaim( MyGearPawn, OldSlotIdx, FALSE );
		}

		// Claim slot
		Link.Claim( MyGearPawn, SlotIdx );

		// set anchor so that the AI can avoid us
		Pawn.SetAnchor(Link.Slots[SlotIdx].SlotMarker);

		// if doing an auto-coverrun
		if (IsLocalPlayerController() && IsDoingSpecialMove(SM_CoverRun) && !IsHoldingRoadieRunButton())
		{
			// and no longer targeting, or hit a cover that we can pop up/out of
			if (!WantsToTarget() || Link.IsStationarySlot(SlotIdx) || Link.Slots[SlotIdx].bCanPopUp)
			{
				// stop the auto run
				EndSpecialMove();
			}
		}

		if( Link.IsStationarySlot(SlotIdx) )
		{
			// prevent movement to handle cover slot stickiness, but only where we have a transition (ie not on edges)
			if( !Link.IsEdgeSlot(SlotIdx) )
			{
				CoverTransitionCountHold = 0.f;
			}

			// this allows us to lean out when you have standing and then midlevel cover attached
			if( !IsDoingSpecialMove(SM_CoverRun) && !MyGearPawn.bDoing360Aiming )
			{
				SetIsInStationaryCover(TRUE);
			}
		}

		// stop evade move
		if( MyGearPawn.IsEvading() )
		{
			StopEvade();
		}
	}
}

function ClientUpdatePosition()
{
	local name CurrentStateName;

	CurrentStateName = GetStateName();
	Super.ClientUpdatePosition();
	if ( CurrentStateName != GetStateName() )
	{
		`log("switching back from "$GetStateName()$" to "$CurrentStateName);
		GotoState(CurrentStateName);
	}
}

unreliable server function ServerCoverTransition(CoverLink Link, byte SlotIdx, byte LeftIdx, byte RightIdx, float SlotPct, ECoverDirection ClientCoverDirection)
{
	local CovPosInfo CovInfo;
	local vector SlotToSlot;

	if (MyGearPawn != None && MyGearPawn.Role == ROLE_Authority)
	{
		// if the Pawn can't be in this cover, tell the client to get out of it
		if (MyGearPawn.IsAKidnapper() || !Link.IsValidClaim(MyGearPawn, SlotIdx, true, true) )
		{
			//`log("ClientInvalidCoverClaim because not valid claim");
			ClientInvalidCoverClaim();
			return;
		}

		MyGearPawn.ReplicatedCoverDirection = ClientCoverDirection;
		MyGearPawn.FreshCoverDirectionTime = WorldInfo.TimeSeconds;

		// Acquire cover
		CovInfo.Link = Link;
		CovInfo.LtSlotIdx = LeftIdx;
		CovInfo.RtSlotIdx = RightIdx;
		CovInfo.LtToRtPct = SlotPct;

		SlotToSlot = CovInfo.Link.GetSlotLocation(RightIdx) - CovInfo.Link.GetSlotLocation(LeftIdx);
		CovInfo.Location = CovInfo.Link.GetSlotLocation(LeftIdx) + (SlotToSlot * SlotPct);
		CovInfo.Normal = vector(CovInfo.Link.GetSlotRotation(SlotIdx)) * -1.f;
		CoverAcquired(CovInfo, false);
	}
}

unreliable client function ClientInvalidCoverClaim()
{
	LeaveCover();
}

unreliable client function ClientRequestCoverTransition()
{
	if ( (MyGearPawn != None) && (MyGearPawn.CoverType != CT_None) && (MyGearPawn.CurrentLink != None) )
	{
		assert( (MyGearPawn.CurrentSlotIdx >= 0) && (MyGearPawn.LeftSlotIdx >= 0) && (MyGearPawn.RightSlotIdx >= 0) );
		ServerCoverTransition(MyGearPawn.CurrentLink, MyGearPawn.CurrentSlotIdx, MyGearPawn.LeftSlotIdx, MyGearPawn.RightSlotIdx, MyGearPawn.CurrentSlotPct, MyGearPawn.ReplicatedCoverDirection);
	}
}

function CallServerMove
(
 SavedMove NewMove,
 vector ClientLoc,
 byte ClientRoll,
 int View,
 SavedMove OldMove
 )
{
	local GearSavedMove PendingGearMove;

	// @TODO compress old move if it exists
	if ( OldMove != None )
	{
		PendingGearMove = GearSavedMove(OldMove);
		OldMove = None;
		SendServerMove(PendingGearMove, vect(1,2,3), ClientRoll, View, None);
	}

	// @TODO - combine pending move?
	// @TODO - compress cover info in CoverServerMove?
	if ( PendingMove != None )
	{
		PendingGearMove = GearSavedMove(PendingMove);
		PendingMove = None;
		SendServerMove(PendingGearMove, vect(1,2,3), ClientRoll, View, None);
	}
	SendServerMove(NewMove, ClientLoc, ClientRoll, View, OldMove);
}

final function SendServerMove
(
 SavedMove NewMove,
 vector ClientLoc,
 byte ClientRoll,
 int View,
 SavedMove OldMove
 )
{
	local GearSavedMove NewGearMove;

	NewGearMove = GearSavedMove(NewMove);
	if ( MyGearPawn != None )
	{
		if ( MyGearPawn.SpecialMove == SM_MidLvlJumpOver )
		{
			MantleServerMove
			(
			NewMove.TimeStamp,
			NewMove.RMVelocity * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View,
			(NewGearMove.CoverType != CT_None)
			);
		}
		else if ( NewMove.bForceRMVelocity )
		{
			RMServerMove
			(
			NewMove.TimeStamp,
			NewMove.RMVelocity * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View,
			(NewGearMove.CoverType != CT_None)
			);
		}
		else if (NewGearMove.CoverType != CT_None)
		{
		//`log("CoverServerMove in state "$GetStateName());
			CoverServerMove
			(
			NewMove.TimeStamp,
			NewMove.Acceleration * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View,
			NewGearMove.CoverType,
			NewGearMove.CoverAction,
			NewGearMove.CoverDirection,
			NewGearMove.bWantsToBeMirrored,
			NewGearMove.CurrentSlotDirection,
			NewGearMove.bIsInStationaryCover
			);
		}
		else if ( MyGearPawn.SpecialMove == SM_RoadieRun )
		{
			RoadieServerMove
			(
			NewMove.TimeStamp,
			NewMove.Acceleration * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View
			);
		}
		else
		{
			ServerMove
			(
			NewMove.TimeStamp,
			NewMove.Acceleration * 10,
			ClientLoc,
			NewMove.CompressedFlags(),
			ClientRoll,
			View
			);
		}
	}
	else
	{
		ServerMove
		(
	    NewMove.TimeStamp,
	    NewMove.Acceleration * 10,
	    ClientLoc,
		NewMove.CompressedFlags(),
		ClientRoll,
	    View
		);
	}
}

/**
 * Make sure server and client get synchronized properly in and out of reviving
 */
unreliable server event ServerVerifyState(name ClientState)
{
	if ( ClientState != GetStateName() )
	{
		bRequestClientVerifyState = TRUE;
	}
}


unreliable client event ClientVerifyState(name ServerState)
{
	UpdateStateFromAdjustment(ServerState);
}

reliable client event ClientPrepareMapChange(name LevelName, bool bFirst, bool bLast)
{
	Super.ClientPrepareMapChange(LevelName,bFirst,bLast);
	// clear any current objectives on a map transition
	ClientClearObjectives();
}

/**
Called by LongClientAdjustPosition()
@param NewState is the state recommended by the server
*/
function UpdateStateFromAdjustment(name NewState)
{
	// don't change states between cover and playerwalking
	if( GetStateName() != NewState )
	{
		if ( MyGearPawn != None )
		{
			if ( MyGearPawn.IsDBNO() )
			{
				if ( NewState != 'Reviving' )
				{
					bRequestServerVerifyState = true;
					return;
				}
			}
			else if ( NewState == 'Reviving' )
			{
				bRequestServerVerifyState = true;
				return;
			}
		}
		if ( IsInCoverState() )
		{
			if ( NewState == 'PlayerWalking' )
			{
				return;
			}
		}
		else if ( GetStateName() == 'PlayerWalking' )
		{
			if (NewState == 'PlayerTakingCover')
				return;
		}
		GotoState(newstate);
	}
}

unreliable server function CoverServerMove
(
 float	TimeStamp,
 vector	InAccel,
 vector	ClientLoc,
 byte	MoveFlags,
 byte	ClientRoll,
 int		View,
 ECoverType InCoverType,
 ECoverAction InCoverAction,
 ECoverDirection InCoverDirection,
 bool InbWantsToBeMirrored,
 ECoverDirection InCurrentSlotDirection,
 bool InbIsInStationaryCover
 )
{
	// If this move is outdated, discard it.
	if( CurrentTimeStamp >= TimeStamp )
	{
		return;
	}

	if (MyGearPawn != None)
	{
		if ( (MyGearPawn.CoverType == CT_None) || !IsInCoverState() )
		{
			ClientRequestCoverTransition();
			//`log("COVERSERVERMOVE WITH NO TRANSITION!");
			// @FIXME - maybe recover better (always send info to enter cover), or ClientClearCover();
		}
		else
		{
			//if ( InCoverType != MyGearPawn.CoverType )`log("set covertype "$Incovertype@timestamp);
			SetPawnCoverType( InCoverType );
			SetPawnCoverAction( InCoverAction );
			SetCoverDirection( InCoverDirection );
			MyGearPawn.SetMirroredSide( InbWantsToBeMirrored );
			MyGearPawn.CurrentSlotDirection = InCurrentSlotDirection;
			MyGearPawn.bIsInStationaryCover = InbIsInStationaryCover;
		}
	}
	bServerMoveCoverActive = true;
	ServerMove(TimeStamp,InAccel,ClientLoc,MoveFlags,ClientRoll,View);
	bServerMoveCoverActive = false;
}

/* RMServerMove()
- replicated function sent by client to server - contains client movement and firing info.
 * This version sent when client has bForceRMVelocity
*/
unreliable server function RMServerMove
(
	float	TimeStamp,
	vector	InAccel,
	vector	ClientLoc,
	byte	MoveFlags,
	byte	ClientRoll,
	int		View,
	bool	bIsInCover
)
{
	// If this move is outdated, discard it.
	if( CurrentTimeStamp >= TimeStamp )
	{
		return;
	}

	if (MyGearPawn == None)
	{
		MyGearPawn = GearPawn(Pawn);
	}

	if (MyGearPawn != None)
	{
		//`log(TimeStamp@self$" RMservermove in state "$GetStateName());
		MyGearPawn.bForceRMVelocity = true;
		MyGearPawn.RMVelocity = InAccel * 0.1;
		InAccel = MyGearPawn.AccelRate * Normal(InAccel);

		if (bIsInCover && MyGearPawn.CoverType == CT_None)
		{
			ClientRequestCoverTransition();
		}
	}
	else
	{
		if ( bIsInCover && !IsInCoverState() )
		{
			ClientRequestCoverTransition();
		}
	}
	bServerMoveCoverActive = bIsInCover;
	ServerMove(TimeStamp,InAccel,ClientLoc,MoveFlags,ClientRoll,View);
	bServerMoveCoverActive = false;

	if (MyGearPawn != None)
	{
		MyGearPawn.bForceRMVelocity = false;
	}
}
/* MantleServerMove()
- replicated function sent by client to server - contains client movement and firing info.
 * This version sent when client is doing SM_MidLvlJumpOver
*/
unreliable server function MantleServerMove
(
	float	TimeStamp,
	vector	InAccel,
	vector	ClientLoc,
	byte	MoveFlags,
	byte	ClientRoll,
	int		View,
	bool	bIsInCover
)
{
	local rotator ViewRot;

	// If this move is outdated, discard it.
	if (CurrentTimeStamp >= TimeStamp)
	{
		return;
	}
	// make sure we're in the right state
	else if (!IsInState('PlayerWalking') && !IsInState('PlayerTakingCover'))
	{
		//`log("Mantleservermove in state "$GetStateName());
		return;
	}
	// make sure we have a GearPawn
	else if ( MyGearPawn == None )
	{
		// client has incorrect Pawn
		if (Pawn != None)
		{
			GivePawn(Pawn);
		}
		return;
	}
	// make sure we're actually doing a mantle
	else if (MyGearPawn.SpecialMove != SM_MidLvlJumpOver)
	{
		//`log("Mantleserver move with invalid special move "@MyGearPawn.SpecialMove);
		return;
	}


	// client and server update mantle in parallel, only replicating Rotation updates,
	// then resyncronize position after the move is complete

	// just update timestamps and rotation
	CurrentTimeStamp = TimeStamp;
	LastActiveTime = WorldInfo.TimeSeconds;
	// ServerTimeStamp forced to current time during mantle update in AGearPC::Tick() so no extra physics updates occur
	ViewRot.Pitch = (View & 65535);
	ViewRot.Yaw = (View >> 16);
	ViewRot.Roll = 0;
	SetRotation(ViewRot);

	// acknowledge receipt of this successful servermove()
	PendingAdjustment.TimeStamp = TimeStamp;
	PendingAdjustment.bAckGoodMove = 1;
}

/* RoadieServerMove()
- replicated function sent by client to server - contains client movement and firing info.
 * This version sent when client has RoadieRun active
*/
unreliable server function RoadieServerMove
(
	float	TimeStamp,
	vector	InAccel,
	vector	ClientLoc,
	byte	MoveFlags,
	byte	ClientRoll,
	int		View
)
{
	local vector X,Y,Z;
	local SMStruct	NewMoveStruct;

	// If this move is outdated, discard it.
	if( CurrentTimeStamp >= TimeStamp )
	{
		return;
	}

	if ( (MyGearPawn != None) && (MyGearPawn.SpecialMove != SM_RoadieRun) )
	{
		GetAxes( Rotation, X, Y, Z );
		RemappedJoyUp = InAccel dot X;
		RemappedJoyRight = InAccel dot Y;

		NewMoveStruct = MyGearPawn.FillSMStructFromParams(SM_RoadieRun);
		ClientToServerDoSpecialMove(NewMoveStruct, FALSE, RemappedJoyUp , RemappedJoyRight, MyGearPawn.Rotation.Yaw);
	}
	ServerMove(TimeStamp,InAccel,ClientLoc,MoveFlags,ClientRoll,View);
}

/* ServerMove()
- replicated function sent by client to server - contains client movement and firing info.
*/
unreliable server function ServerMove
(
	float	TimeStamp,
	vector	InAccel,
	vector	ClientLoc,
	byte	MoveFlags,
	byte	ClientRoll,
	int		View
)
{
	// If this move is outdated, discard it.
	if( CurrentTimeStamp >= TimeStamp )
	{
		return;
	}

	// force the acceleration to be rejected if performing a special move that prevents movement
	// (so the client can't lie to us)
	if (MyGearPawn != None && MyGearPawn.IsDoingASpecialMove() && MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bDisableMovement)
	{
		InAccel = vect(0,0,0);
	}

	if ( (Pawn != None) && !Pawn.bForceRMVelocity)
	{
		if ( (Pawn.Mesh != None) && ((Pawn.Mesh.RootMotionMode == RMM_Accel) || (Pawn.Mesh.RootMotionMode == RMM_Velocity)) )
		{
			//`log(TimeStamp@self$" Force regular velocity with specialmove "$Gearpawn(Pawn).SpecialMove@vsize(pawn.velocity)@vsize(0.1*inaccel)@Pawn.Physics);
			Pawn.bForceRegularVelocity = true;
		}

		bPreciseDestination = false;
	}
	Super.ServerMove(TimeStamp,InAccel,ClientLoc,MoveFlags,ClientRoll,View);
	if ( Pawn != None )
	{
		Pawn.bForceRegularVelocity = false;
	}
}

unreliable client function LongClientAdjustPosition
(
	float TimeStamp,
	name newState,
	EPhysics newPhysics,
	float NewLocX,
	float NewLocY,
	float NewLocZ,
	float NewVelX,
	float NewVelY,
	float NewVelZ,
	Actor NewBase,
	float NewFloorX,
	float NewFloorY,
	float NewFloorZ
)
{
	local InterpActor_GearBasePlatform Platform;
	local vector NewLoc;

	//@hack: ignore the call if we're already clamped to the correct base
	if (Pawn != None && Pawn.Physics == PHYS_None && Pawn.Base == NewBase)
	{
		Platform = InterpActor_GearBasePlatform(Pawn.Base);
		if (Platform != None && Platform.bDisallowPawnMovement)
		{
			NewLoc.X = NewLocX;
			NewLoc.Y = NewLocY;
			NewLoc.Z = NewLocZ;
			NewLoc += Platform.Location;
			if (VSize(NewLoc - Pawn.Location) < 50.0)
			{
				return;
			}
		}
	}

	Super.LongClientAdjustPosition(TimeStamp, NewState, NewPhysics, NewLocX, NewLocY, NewLocZ, NewVelX, NewVelY, NewVelZ, NewBase, NewFloorX, NewFloorY, NewFloorZ);
}

function UpdateRotation( float DeltaTime )
{
	local Rotator	DeltaRot, NewRotation, ViewRotation;

	ViewRotation	= Rotation;
	DesiredRotation = ViewRotation; //save old rotation

	if (bLookingAtPointOfInterest && IsHoldingPOIButton(TRUE))
	{
		if ((PlayerInput.aForward != 0.f || PlayerInput.aStrafe != 0.f ) && !IsInState('PlayerTurreting'))
		{
			DeltaRot.Yaw = GearPlayerCamera(PlayerCamera).GameplayCam.LastYawAdjustment;
			DeltaRot.Pitch = GearPlayerCamera(PlayerCamera).GameplayCam.LastPitchAdjustment;
		}
	}
	else
	{
		// Calculate Delta to be applied on ViewRotation
		DeltaRot.Yaw	= PlayerInput.aTurn;
		DeltaRot.Pitch	= PlayerInput.aLookUp;

		DeltaRot += ExtraRot;
		ExtraRot = rot(0,0,0);

		// Check that Adhesion target is still valid
		if( ForcedAdhesionTarget != None && !ForcedAdhesionTarget.IsValidTargetFor(self) )
		{
			StopForcedAdhesion();
		}

		// Apply adhesion
		ApplyAdhesion(DeltaTime, DeltaRot.Yaw, DeltaRot.Pitch);
	}

	ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
	GearPlayerCamera(PlayerCamera).GameplayCam.AdjustFocusPointInterpolation(DeltaRot);			// adjust for focus points

	if(Pawn != None && GearPawn(Pawn) != None && GearPawn(Pawn).CarriedCrate != None)
	{
		ClampRotation( ViewRotation, Pawn.Rotation, rot(16384,16384,-1), rot(16384,16384,-1) );
	}

	SetRotation( ViewRotation );

	ViewShake( DeltaTime );

	NewRotation = ViewRotation;
	NewRotation.Roll = Rotation.Roll;

	if( Pawn != None )
	{
		Pawn.FaceRotation(NewRotation, DeltaTime);
	}
}

final function ApplyAdhesion(float DeltaTime, out int out_YawRot, out int out_PitchRot)
{
	local rotator NewRotation, DeltaRot;
	local vector TargetCenter;
	local float TargetWidth, TargetHeight;

	if ( (GearAimAssistActor(LastFrictionTarget) != None) && LastFrictionTarget.bCanBeAdheredTo )
	{
		GearPlayerInput(PlayerInput).ViewAdhesion(DeltaTime, GearWeapon(Pawn.Weapon), out_YawRot, out_PitchRot);
	}

	if( MyGearPawn != None && ForcedAdhesionTarget != None && !ForcedAdhesionTarget.bDeleteMe )
	{
		ForcedAdhesionTarget.GetAimFrictionExtent(TargetWidth, TargetHeight, TargetCenter);

		// we've got "forced adhesion" going, do that
		NewRotation = rotator(TargetCenter - MyGearPawn.Location);

		NewRotation = RInterpTo(Normalize(Rotation), Normalize(NewRotation), DeltaTime, 8.f);

		DeltaRot		= NewRotation - Rotation;

		// hard override
		out_YawRot		= DeltaRot.Yaw;
	}
}

final simulated function bool WeaponHasAnyAmmo()
{
	return (MyGearPawn != None && MyGearPawn.MyGearWeapon != None && MyGearPawn.MyGearWeapon.HasAnyAmmo());
}

reliable client function ClientBreakFromCover();

simulated function TransitionFromSwatToRoadieRun()
{
	// if still holding a and swat turning then interrupt the swat turn
	if (IsHoldingRoadieRunButton() && IsDoingSpecialMove(SM_StdLvlSwatTurn))
	{
		// if not too close to the destination
		if (VSize(GetDestinationPosition() - Pawn.Location) > Pawn.GetCollisionRadius() * 2.f)
		{
			// break from cover
			LeaveCover();
			// and transition directly into roadie run
			DoSpecialMove(SM_RoadieRun,TRUE);
		}
		// otherwise try to chain a slip/mantle
		else if (CanDoSpecialMove(SM_CoverSlip))
		{
			DoSpecialMove(SM_CoverSlip);
		}
		else if (CanDoSpecialMove(SM_MidLvlJumpOver))
		{
			MyGearPawn.VerifySMHasBeenInstanced(SM_MidLvlJumpOver);
			DoSpecialMove(SM_MidLvlJumpOver, FALSE, None, GSM_MidLvlJumpOver(MyGearPawn.SpecialMoves[SM_MidLvlJumpOver]).PackSpecialMoveFlags());
		}
		// and if all else fails do nothing
	}
}

simulated state PlayerTakingCover extends PlayerWalking
{
	exec function bool TryASpecialMove(bool bRunActions)
	{
		local bool bDoMonkeyMoveChecks;

		`LogSM("Try a cover special move");

		// if swat turning
		if (IsDoingSpecialMove(SM_StdLvlSwatTurn))
		{
			if (!IsTimerActive('TransitionFromSwatToRoadieRun'))
			{
				// only set the timer once to prevent button mashing issues
				SetTimer( 0.2f,FALSE,nameof(TransitionFromSwatToRoadieRun) );
			}
			return TRUE;
		}
		else if (!Global.TryASpecialMove(bRunActions))
		{
			bDoMonkeyMoveChecks = !bUseAlternateControls || !bRunActions;

			if( bDoMonkeyMoveChecks && CanDoSpecialMove(SM_MantleUpLowCover) )
			{
				// try to jump up on cover
				DoSpecialMove(SM_MantleUpLowCover);
				return TRUE;
			}
			else if( bDoMonkeyMoveChecks && CanDoSpecialMove(SM_CoverSlip) )
			{
				// break out of cover
				DoSpecialMove(SM_CoverSlip);
				return TRUE;
			}
			else if( bDoMonkeyMoveChecks && CanDoSpecialMove(SM_MidLvlJumpOver) )
			{
				// try to jump over cover
				MyGearPawn.VerifySMHasBeenInstanced(SM_MidLvlJumpOver);
				DoSpecialMove(SM_MidLvlJumpOver, FALSE, None, GSM_MidLvlJumpOver(MyGearPawn.SpecialMoves[SM_MidLvlJumpOver]).PackSpecialMoveFlags());
				return TRUE;
			}
			// swat turn
			else if( bDoMonkeyMoveChecks && CanDoSpecialMove(SM_StdLvlSwatTurn) )
			{
				DoSpecialMove(SM_StdLvlSwatTurn);
				return TRUE;
			}
			else if( bPressAToBreakFromCover && MyGearPawn.IsCarryingAHeavyWeapon() && (!bUseAlternateControls || bRunActions) )
			{
				ClientBreakFromCover();
				return TRUE;
			}
			else if (MyGearPawn.IsAtCoverEdge(MyGearPawn.bIsMirrored,TRUE,0.5f) && TryToEvade())
			{
				// TryToEvade will starts the special move if successful
			}
			else if( !bUseAlternateControls || bRunActions )
			{
				// setup the timer for evade/roadierun
				SetTimer( RoadieRunTimer, FALSE, nameof(TryToRoadieRun) );
			}
		}
		return FALSE;
	}

	reliable client function ClientBreakFromCover()
	{
		bBreakFromCover = TRUE;
	}

	event PlayerTick(float DeltaTime)
	{
		Global.PlayerTick(DeltaTime);

		// if we should break from cover, let's do it now
		if( bBreakFromCover )
		{
			CoverLog("bBreakFromCover == TRUE, LeaveCover."@MyGearPawn.CurrentLink, GetFuncName());
			bBreakFromCover = FALSE;
			LeaveCover();
		}
	}

	/**
	 * Yes, we are in a cover state.
	 */
	simulated function bool IsInCoverState()
	{
		return TRUE;
	}

	/**
	 * @See GearPC::TryToEvade
	 */
	exec function bool TryToEvade( optional EDoubleClickDir DoubleClickInput )
	{
		if( IsLocalPlayerController() && MyGearPawn != None && CanEvade() && Abs(RemappedJoyRight) > DeadZoneThreshold )
		{
			// test on the right
			if( RemappedJoyRight > 0 )
			{
				ClientBreakFromCover();
				DoSpecialMove(SM_EvadeRt);
				return TRUE;
			}
			else
			{
				ClientBreakFromCover();
				DoSpecialMove(SM_EvadeLt);
				return TRUE;
			}
		}
		else
		{
			//`log(`showvar(CanEvade()));
		}

		return FALSE;
	}

	/* ServerMove()
	- replicated function sent by client to server - contains client movement and firing info.
	*/
	unreliable server function ServerMove
	(
		float	TimeStamp,
		vector	InAccel,
		vector	ClientLoc,
		byte	MoveFlags,
		byte	ClientRoll,
		int		View
	)
	{
		// If this move is outdated, discard it.
		if( CurrentTimeStamp >= TimeStamp )
		{
			return;
		}

		if ( !bServerMoveCoverActive && (ClientLoc != vect(1,2,3)) )
		{
			CoverLog("Servermove LeaveCover.", GetFuncName());
			LeaveCover();
		}
		Global.ServerMove(TimeStamp,InAccel,ClientLoc,MoveFlags,ClientRoll,View);
	}

	/**
	 * Overridden to handle initial cover setup.
	 */
	function BeginState(Name PreviousStateName)
	{
		local vector	CamLoc;		//, CoverNormal;
		local rotator	CamRot;

		super.BeginState(PreviousStateName);

		//old: default to popup on every new cover entry as that is generally the safer choice to make
		//new: default to lean when possible
		bPreferLeanOverPopup = TRUE;

		CoverLog("PreviousStateName:" @ PreviousStateName, GetFuncName());
		CoverTransitionCountHold = 0.f;

		// Set initial rotation to remap controls
		GetPlayerViewPoint(CamLoc, CamRot);
		ControlsRemapRotation = CamRot;

		bBreakFromCover = FALSE;

		// Always cancel double click direction when getting into cover
		FinishDoubleClick( TRUE );
	}

	function EndState(Name NextStateName)
	{
		super.EndState(NextStateName);

		CoverLog("NextStateName:" @ NextStateName, GetFuncName());

		// reset the cover state to be not in cover
		SetPawnCoverAction(CA_Default);
		SetCoverDirection(CD_Default);
		SetPawnCoverType(CT_None);
		SetIsInStationaryCover(FALSE);
	}

	/**
	 * Overridden to update controls.
	 */
	function PlayerMove(float DeltaTime)
	{
		local ECoverType		PawnCT;
		local ECoverAction		PawnCA;
		local ECoverDirection	PawnCD;
		local int				bNewLeftSided;

		// Update this first, this will set 360 aiming flag, that we need before doing all the cover action stuff.
		MyGearPawn.UpdateMeshBoneControllers(DeltaTime);

		if( !IsMoveInputIgnored() )
		{
			// get the current state
			PawnCT			= MyGearPawn.FindCoverType();
			PawnCA			= MyGearPawn.CoverAction;
			PawnCD			= GetCoverDirection();
			bNewLeftSided	= int(MyGearPawn != None && MyGearPawn.bWantsToBeMirrored);
			// Update player posture
			// But not if breaking from cover
			if( !bBreakFromCover && !IsDoingSpecialMove(SM_PushOutOfCover) )
			{
				UpdatePlayerPosture(DeltaTime, PawnCT, PawnCA, PawnCD, bNewLeftSided);
			}
			else
			{
				// Force action to be default
				PawnCA = CA_Default;
			}
			// update the state
			SetPawnCoverType(PawnCT);
			SetPawnCoverAction(PawnCA);
			SetCoverDirection(PawnCD);

			// If breaking from cover, don't mess with mirroring transitions.
			if( !bBreakFromCover )
			{
				if (bool(bNewLeftSided) != MyGearPawn.bWantsToBeMirrored)
				{
					// we do this here instead of in SetMirriredSide because that function gets called for
					// many other mirror-changing situations where we don't want to do the sound
					MyGearPawn.SoundGroup.PlayFoleySound(MyGearPawn, GearFoley_BodyTurnFX, TRUE);
				}

				MyGearPawn.SetMirroredSide(bool(bNewLeftSided));
			}
		}

		// normal update
		Super.PlayerMove(DeltaTime);
	}

	simulated function DetermineLeanDirection
	(
		GearPawn				P,
		bool					bIsAtSlot,
		out const CoverSlot		CurrentSlot,
		int						bNewLeftSided,
		out ECoverAction		OutPawnCA,
		out	ECoverDirection		OutPawnCD
	)
	{
		local bool bJoyUpDominant, bJoyRightDominant, bTargeting, bCanPopUp;

		// No actions while reloading or switching weapons
		if( (P.IsReloadingWeapon() && P.MyGearWeapon.ShouldReloadingPreventTargeting()) || P.bSwitchingWeapons || P.bDoing360Aiming )
		{
			OutPawnCD = CD_Default;
			OutPawnCA = CA_Default;
			return;
		}

		// Reset cover action by default
		OutPawnCD = CD_Default;
		OutPawnCA = CA_Default;

		// determine which direction is dominant
		bJoyUpDominant = Abs(RemappedJoyUp) > DeadZoneThreshold && Abs(RemappedJoyUp) > Abs(RemappedJoyRight);
		bJoyRightDominant = Abs(RemappedJoyRight) > DeadZoneThreshold && Abs(RemappedJoyRight) > Abs(RemappedJoyUp);
		bTargeting = WantsToTarget();
		// can we popup?  based on slot setting, and if it's midlevel cover.
		bCanPopUp = CurrentSlot.bCanPopUp && P.CoverType == CT_MidLevel;

		// if targeting,
		if( bTargeting )
		{
			// if we can popup, then by default popup
			if (bCanPopUp)
			{
				OutPawnCD = CD_Up;
				OutPawnCA = CA_PopUp;
			}
			// Otherwise, if we are at a left edge and facing left
			if( bIsAtSlot && bNewLeftSided == 1 && CurrentSlot.bLeanLeft )
			{
				// if we can lean or popup
				// Decide if we should lean or pop up
				// Lean if...
				if( (!bCanPopUp) ||																			// Not allowed to popup because of slot type OR
					(!bJoyRightDominant && !bJoyUpDominant && (bPreferLeanOverPopup || P.CoverType == CT_Standing)) ||							// no dominant axis and we leaned last time
					(bJoyRightDominant && RemappedJoyRight < -DeadZoneThreshold) )								// Pressing left
				{
					// Set lean left
					OutPawnCD = CD_Left;
					OutPawnCA = CA_LeanLeft;
				}
			}
			else
			// Otherwise, if we are at a right edge and facing right and can lean right
			if( bIsAtSlot && bNewLeftSided == 0 && CurrentSlot.bLeanRight )
			{
				// Decide if we should lean or pop up
				// Lean if...
				if( (!bCanPopUp) ||																			// Not allowed to popup because of slot type OR
					(!bJoyRightDominant && !bJoyUpDominant && (bPreferLeanOverPopup || P.CoverType == CT_Standing)) ||							// no dominant axis and we leaned last time
					(bJoyRightDominant && RemappedJoyRight > DeadZoneThreshold) )								// Pressing right
				{
					// Set lean right
					OutPawnCD = CD_Right;
					OutPawnCA = CA_LeanRight;
				}
			}
		}
		// blindfiring
		else
		{
			// if attempting to fire, or recently blindfired (and not recently hurt),
			if( (WeaponHasAnyAmmo() && P.PawnCommitToFiring(0)) ||
				(TimeSince(P.LastWeaponBlindFireTime) < P.AimTimeAfterFiring && TimeSince(P.LastTookDamageTime) > 0.3f && IsBlindFiring(P.CoverAction)) )
			{
				// default to blind up if possible
				if( bCanPopUp && P.CoverType == CT_MidLevel )
				{
					OutPawnCD = CD_Up;
					OutPawnCA = CA_BlindUp;
				}

				if (bIsAtSlot)
				{
					if( bNewLeftSided == 1 && CurrentSlot.bLeanLeft )
					{
						// if we can't popup, or
						// if pressing left, or
						// not pressing anything and we prefer leans
						if( !bCanPopUp ||
							(P.AimOffsetPct.X < 0.2f &&
								((bJoyRightDominant && RemappedJoyRight < -DeadZoneThreshold) || (!bJoyRightDominant && !bJoyUpDominant && bPreferLeanOverPopup)) ) )
						{
							// blind left baby!
							OutPawnCD = CD_Left;
							OutPawnCA = CA_BlindLeft;
						}
					}
					else
					if( bNewLeftSided == 0 && CurrentSlot.bLeanRight )
					{
						if( !bCanPopUp ||
							(P.AimOffsetPct.X > -0.2f &&
								((bJoyRightDominant && RemappedJoyRight > DeadZoneThreshold) || (!bJoyRightDominant && !bJoyUpDominant && bPreferLeanOverPopup)) ) )
						{
							OutPawnCD = CD_Right;
							OutPawnCA = CA_BlindRight;
						}
					}
				}
			}
		}

		// If Pawn is not allowed to do a leaning transition, then prevent him from doing so.
		if( !P.CanDoLeaningTransition() )
		{
			if( P.CoverAction != OutPawnCA && P.IsALeaningAction(P.CoverAction) && P.IsALeaningAction(OutPawnCA) )
			{
				OutPawnCA = P.CoverAction;
				OutPawnCD = P.CoverDirection;
			}
		}

		if( OutPawnCD != CD_Default )
		{
			// set the lean preference based on our last action
			bPreferLeanOverPopup = (OutPawnCD != CD_Up);
		}
	}

	/**
	 * Update Player posture. Return true if allowed to move
	 *
	 * @param out	out_PawnCT	Pawn's Cover type
	 * @param out	out_PawnCA	Pawn's Cover Action
	 * @param out	out_PawnCD	Cover Direction
	 *
	 * @return true if allowed to move
	 */
	simulated function UpdatePlayerPosture
	(
			float				DeltaTime,
		out	ECoverType			OutPawnCT,
		out ECoverAction		OutPawnCA,
		out	ECoverDirection		OutPawnCD,
		out	int					bNewLeftSided
	)
	{
		local bool			bIsAtSlot, bJoyUpDominant, bJoyRightDominant, bTargeting, bJoyIsLeft, bJoyIsRight;
		local CoverSlot		CurrentSlot;
		local int			SlotIdx, EdgeSlotIdx;

		// grab the slot we're at
		SlotIdx		= MyGearPawn.PickClosestCoverSlot(TRUE,0.5f,TRUE);
		if (SlotIdx == -1)
		{
			bIsAtSlot = FALSE;
			SlotIdx = MyGearPawn.CurrentSlotIdx;
		}
		else
		{
			bIsAtSlot = TRUE;
		}
		CurrentSlot	= MyGearPawn.CurrentLink.Slots[SlotIdx];

		// Check which joy axis is dominant
		bJoyUpDominant		= Abs(RemappedJoyUp) > Abs(RemappedJoyRight);
		bJoyRightDominant	= Abs(RemappedJoyRight) > Abs(RemappedJoyUp);
		bJoyIsLeft			= RemappedJoyRight < -DeadZoneThreshold;
		bJoyIsRight			= RemappedJoyRight > DeadZoneThreshold;

		bTargeting			= MyGearPawn.bIsTargeting;

		// reset any pawn movement
		MyGearPawn.CurrentSlotDirection = CD_Default;

		// if not holding back
		if (!bJoyUpDominant || Abs(RemappedJoyUp) < DeadZoneThreshold)
		{
			// reset the break from cover timer
			BreakFromCoverHoldTimer = 0.f;
		}

		// vertical axis
		if( bJoyUpDominant )
		{
			// If pulling away from cover, then break from it
			if( RemappedJoyUp < -DeadZoneThreshold && IsLocalPlayerController() && (!IsDoingASpecialMove() || MyGearPawn.IsDoingMove2IdleTransition()) )
			{
				// disallow cover break when popping up with a heavy weapon
				if ( !MyGearPawn.IsCarryingAHeavyWeapon() || (MyGearPawn.CoverAction != CA_PopUp) )
				{
					// increment the timer
					BreakFromCoverHoldTimer += DeltaTime;
					// if exceeded the threshold then break from cover now
					if (BreakFromCoverHoldTimer >= BreakFromCoverHoldTime)
					{
						// Don't play push out transition is popping up or leaning.
						if( MyGearPawn.CoverAction != CA_PopUp )
						{
							DoSpecialMove(SM_PushOutOfCover, TRUE);
						}
						else
						{
							// otherwise delay a tick and do a normal leavecover
							ClientBreakFromCover();
						}
						// no need to process further
						return;
					}
				}
			}
		}
		else
		// horizontal axis
		if( bJoyRightDominant && Abs(RemappedJoyRight) > DeadZoneThreshold )
		{
			// if they are locked into a slot
			if( MyGearPawn.bIsInStationaryCover )
			{
				// if not actually at a slot origin
				if( !bIsAtSlot )
				{
					// then unlock them
					SetIsInStationaryCover(FALSE);
				}
				// if they are trying to move away from the slot
				else if ((CurrentSlot.bLeanLeft && bJoyIsRight && !MyGearPawn.IsAtRightEdgeSlot()) ||
						(CurrentSlot.bLeanRight && bJoyIsLeft && !MyGearPawn.IsAtLeftEdgeSlot()))
				{
					// then unlock them
					SetIsInStationaryCover(FALSE);
				}
				// if not currently targeting check for delayed transitions,
				else if( !bTargeting )
				{
					if( bJoyIsLeft )
					{
						// if we're allowed a transition
						if( MyGearPawn.CurrentLink.AllowLeftTransition(MyGearPawn.CurrentSlotIdx) )
						{
							// if this slot can't even lean left,
							if( !CurrentSlot.bLeanLeft || IsDoingSpecialMove(SM_CoverRun) )
							{
								// then unlock immediately
								SetIsInStationaryCover(FALSE);
							}
							else
							{
								// otherwise increment the counter
								CoverTransitionCountHold += DeltaTime;
								// if the counter has exceeded the threshold,
								if( CoverTransitionCountHold > CoverTransitionTime )
								{
									CoverTransitionCountHold = 0.f;
									// unlock them from cover
									SetIsInStationaryCover(FALSE);
								}
							}
						}
					}
					else if( bJoyIsRight )
					{
						// if we're allowed a transition
						if( MyGearPawn.CurrentLink.AllowRightTransition(MyGearPawn.CurrentSlotIdx) )
						{
							// if this slot can't even lean right, or is running
							if( !CurrentSlot.bLeanRight || IsDoingSpecialMove(SM_CoverRun) )
							{
								// then unlock immediately
								SetIsInStationaryCover(FALSE);
							}
							else
							{
								// otherwise increment the counter
								CoverTransitionCountHold += DeltaTime;
								// if the counter has exceeded the threshold,
								if( CoverTransitionCountHold > CoverTransitionTime )
								{
									CoverTransitionCountHold = 0.f;
									// unlock them from cover
									SetIsInStationaryCover(FALSE);
								}
							}
						}
					}
				}
			}

			if( !IsDoingSpecialMove(SM_CoverRun) )
			{
				// no movement while popping up in standing cover
				if( MyGearPawn.CoverType == CT_Standing && OutPawnCA == CA_PopUp )
				{
					OutPawnCD = CD_Default;
				}
				// check to see if the player is trying to switch directions
				// current right but pushing left?
				else if( !bTargeting && bJoyIsLeft && bNewLeftSided == 0 )
				{
					// Don't mirror in 360 aiming.
					if( !MyGearPawn.bDoing360Aiming )
					{
						//`log("Turn Left, Not in 360");
						bNewLeftSided = 1;
					}
					OutPawnCD = CD_Left;
				}
				// currently left but pushing right?
				else if( !bTargeting && bJoyIsRight && bNewLeftSided == 1 )
				{
					// Don't mirror in 360 aiming.
					if( !MyGearPawn.bDoing360Aiming )
					{
						//`log("Turn Right, Not in 360");
						bNewLeftSided = 0;
					}
					OutPawnCD = CD_Right;
				}
				else
				{
					// otherwise pick direction based on joystick
					OutPawnCD = bJoyIsLeft ? CD_Left : CD_Right;
				}
			}

			// if not locked into a slot
			if( !MyGearPawn.bIsInStationaryCover && !IsMoveInputIgnored() )
			{
				// then set the slot direction so physics will move the player
				MyGearPawn.CurrentSlotDirection = OutPawnCD;
			}
			else
			{
				// otherwise keep them stationary
				MyGearPawn.CurrentSlotDirection = CD_Default;
			}
		}

		// if not already performing a cover special move
		if (!IsDoingSpecialMove(SM_CoverRun) && !IsDoingSpecialMove(SM_CoverSlip) && !IsDoingSpecialMove(SM_MidLvlJumpOver))
		{
			// Setup lean/popup action and direction
			DetermineLeanDirection( MyGearPawn, bIsAtSlot, CurrentSlot, bNewLeftSided, OutPawnCA, OutPawnCD );

			// if we want to target in standing cover but can't,
			if (!MyGearPawn.IsReloadingWeapon() && !MyGearPawn.bDoing360Aiming)
			{
				//`define automovedebug(msg)	`log(`msg)
				`define automovedebug(msg)
				if (WantsToTarget() && !CurrentSlot.bCanPopUp && CurrentSlot.CoverType == CT_Standing && MyGearPawn.CoverAction == OutPawnCA && OutPawnCA == CA_Default)
				{
					`automovedebug("blah:"@MyGearPawn.PickClosestCoverSlot(FALSE)@MyGearPawn.PickClosestCoverSlot(TRUE)@MyGearPawn.bIsMirrored);
					// if it's because we're in standing cover facing the wrong direction
					if (MyGearPawn.IsAtCoverEdge(!MyGearPawn.bIsMirrored,TRUE,0.5f,EdgeSlotIdx,SlotIdx))
					{
						`automovedebug("flipping player to cover edge"@EdgeSlotIdx);
						// then flip to the correct direction
						bNewLeftSided = MyGearPawn.CurrentLink.Slots[EdgeSlotIdx].bLeanLeft ? 1 : 0;
						//(EdgeSlotIdx <= SlotIdx ? 1 : 0);
					}
					else
					// otherwise trigger a cover run to the edge as long as the player isn't pushing the opposite direction
					if (!bIsAtSlot && MyGearPawn.IsAtCoverEdge(MyGearPawn.bIsMirrored,TRUE,2.f) && ((!MyGearPawn.bIsMirrored && !bJoyIsLeft) || (MyGearPawn.bIsMirrored && !bJoyIsRight)))
					{
						`automovedebug("triggering cover run to edge"@MyGearPawn.IsAtCoverEdge(MyGearPawn.bIsMirrored,TRUE,0.5f));
						MyGearPawn.bIsInStationaryCover = FALSE;
						DoSpecialMove(SM_CoverRun,FALSE,None,1);
					}
				}
				else if (CurrentSlot.CoverType == CT_Standing && MyGearPawn.PawnCommitToFiring(0) && OutPawnCA == CA_Default && MyGearPawn.IsAtCoverEdge(!MyGearPawn.bIsMirrored,TRUE,0.5f,EdgeSlotIdx,SlotIdx))
				{
					`automovedebug("mirroring to fire");
					bNewLeftSided = MyGearPawn.CurrentLink.Slots[EdgeSlotIdx].bLeanLeft ? 1 : 0;
				}
			}
		}

		// save the slot we're closest too
		MyGearPawn.ClosestSlotIdx = SlotIdx;
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local GearPawn P;
		local vector CamLoc;
		local rotator CamRot;

		super.ProcessMove( DeltaTime, NewAccel, DoubleClickMove, DeltaRot );

		P = GearPawn(Pawn);
		if( P == None )
		{
			return;
		}

		if( IsLocalPlayerController() )
		{
			if( DoubleClickMove != DCLICK_None )
			{
				// remap the double clicks based on camera orientation
				GetPlayerViewPoint(CamLoc,CamRot);
				// if facing backwards then just swap them all
				if (vector(CamRot) dot vector(Pawn.Rotation) < -0.3)
				{
					if (DoubleClickMove == DCLICK_Forward)
					{
						DoubleClickMove = DCLICK_Back;
					}
					else if (DoubleClickMove == DCLICK_Back)
					{
						DoubleClickMove = DCLICK_Forward;
					}
					else if (DoubleClickMove == DCLICK_Left)
					{
						DoubleClickMove = DCLICK_Right;
					}
					else if (DoubleClickMove == DCLICK_Right)
					{
						DoubleClickMove = DCLICK_Left;
					}
				}
				// if facing to the side
				else if (vector(CamRot) dot vector(Pawn.Rotation) < 0.3)
				{
					//  if facing to the right
					if (Normal(vector(CamRot) cross vect(0,0,1)) dot vector(Pawn.Rotation) > 0)
					{
						// rotate to the right
						if (DoubleClickMove == DCLICK_Forward)
						{
							DoubleClickMove = DCLICK_Right;
						}
						else if (DoubleClickMove == DCLICK_Back)
						{
							DoubleClickMove = DCLICK_Left;
						}
						else if (DoubleClickMove == DCLICK_Left)
						{
							DoubleClickMove = DCLICK_Forward;
						}
						else if (DoubleClickMove == DCLICK_Right)
						{
							DoubleClickMove = DCLICK_Back;
						}
					}
					else
					{
						// rotate to the left
						if (DoubleClickMove == DCLICK_Forward)
						{
							DoubleClickMove = DCLICK_Left;
						}
						else if (DoubleClickMove == DCLICK_Back)
						{
							DoubleClickMove = DCLICK_Right;
						}
						else if (DoubleClickMove == DCLICK_Left)
						{
							DoubleClickMove = DCLICK_Back;
						}
						else if (DoubleClickMove == DCLICK_Right)
						{
							DoubleClickMove = DCLICK_Forward;
						}
					}
				}
				if( DoubleClickMove == DCLICK_Forward )
				{
					if( TryDoubleTapMove(SM_MantleUpLowCover) ||
						TryDoubleTapMove(SM_CoverSlip) ||
						TryDoubleTapMove(SM_MidLvlJumpOver)	)
					{
						// Successfully climbed up, over, or around
						FinishDoubleClick(TRUE);
					}
				}
				else if( DoubleClickMove == DCLICK_Left || DoubleClickMove == DCLICK_Right )
				{
					if( TryDoubleTapMove(SM_StdLvlSwatTurn) || TryToEvade(DoubleClickMove) )
					{
						// Successful swat turn or evade
						FinishDoubleClick(TRUE);
					}
				}

				if( DoubleClickMove == DCLICK_Active )
				{
					// If not giving any input once you get into cover, remove double click
					if( PlayerInput.RawJoyUp == 0 &&
						PlayerInput.RawJoyRight == 0 )
					{
						if( IsDoingSpecialMove(SM_CoverRun) || IsDoingSpecialMove(SM_RoadieRun) )
						{
							EndSpecialMove();
						}
						else
						{
							FinishDoubleClick( FALSE );
						}
					}
				}
				else if( DoubleClickMove != DCLICK_Done && DoubleClickDir  != DCLICK_Done )
				{
					// Set click direction as active
					CurrentDoubleClickDir = DoubleClickDir;
					DoubleClickDir = DCLICK_Active;
				}
			}
			else if( !MainPlayerInput.IsButtonActive(bUseAlternateControls?GB_X:GB_A) && PlayerInput.RawJoyUp == 0 && PlayerInput.RawJoyUp == 0 && MyGearPawn.SpecialMoveFlags == 0 )
			{
				if( IsDoingSpecialMove(SM_CoverRun) || IsDoingSpecialMove(SM_RoadieRun) )
				{
					EndSpecialMove();
				}
			}
		}
	}
}

/** For trying special moves when using the double tap movement */
function bool TryDoubleTapMove( ESpecialMove TapMove )
{
	if ( IsInCoverState() && CanDoSpecialMove(TapMove, true) )
	{
		DoSpecialMove( TapMove );
		return true;
	}
	return false;
}

final function FinishDoubleClick( bool bAllowInstant )
{
	if( PlayerInput != None )
	{
		MainPlayerInput.DoubleClickTimer = bAllowInstant ? -0.15f : 0.f;
	}
	DoubleClickDir = DCLICK_Done;
}

final function ResetDoubleClick()
{
	DoubleClickDir = DCLICK_None;
}

/** Whether the action button is being held or not */
simulated final function bool IsHoldingActionButton()
{
	return ((MainPlayerInput != None && MainPlayerInput.IsButtonActive(GB_A)) || DoubleClickDir == DCLICK_Active);
}

/** Whether the roadie run button is being held or not */
simulated final function bool IsHoldingRoadieRunButton()
{
	return ((MainPlayerInput != None && MainPlayerInput.IsButtonActive(bUseAlternateControls?GB_X:GB_A)) || DoubleClickDir == DCLICK_Active);
}

/** Whether the POI button is being held or not */
simulated final function bool IsHoldingPOIButton( optional bool bRaw )
{
	return (MainPlayerInput != None && MainPlayerInput.IsButtonActive(bUseAlternateControls?GB_RightStick_Push:GB_Y, true));
}

simulated function NotifyCoverDisabled( CoverLink Link, int SlotIdx, optional bool bAdjacentIdx )
{
	local bool bBreakCover;

	if( !bAdjacentIdx )
	{
		bBreakCover = TRUE;
	}
	else
	{
		// If given slot is to my right
		if( SlotIdx > MyGearPawn.CurrentSlotIdx ||
			(SlotIdx == 0 && MyGearPawn.CurrentSlotIdx == Link.Slots.length - 1) )
		{
			if( MyGearPawn.RightSlotIdx == SlotIdx )
			{
				bBreakCover = TRUE;
			}

		}

		// If given slot is to my left
		if(  SlotIdx < MyGearPawn.CurrentSlotIdx ||
			(MyGearPawn.CurrentSlotIdx == 0 && SlotIdx == Link.Slots.Length - 1) )
		{
			if( MyGearPawn.LeftSlotIdx == SlotIdx )
			{
				bBreakCover = TRUE;
			}
		}
	}

	if( bBreakCover )
	{
		// break from cover normally
		ClientBreakFromCover();
	}
}

simulated event NotifyCoverAdjusted()
{
	if (MyGearPawn != None)
	{
		MyGearPawn.SetCurrentCoverType();
	}
}

/**
 * Sets new cover direction.
 * Which represents where the camera is focusing.
 * (CD_Right for a right lean or right blind firing).
 *
 * @param	NewCD	new cover direction
 */

final function SetCoverDirection( ECoverDirection NewCD )
{
	if( MyGearPawn != None &&
		MyGearPawn.CoverDirection != NewCD )
	{
		CoverLog("NewCD:" @ NewCD, GetFuncName() );
		MyGearPawn.CoverDirection = NewCD;
	}
}

/**
 * Return cover direction
 * Which represents where the camera is focusing.
 * (CD_Right for a right lean or right blind firing).
 *
 * @return cover direction
 */
final native function ECoverDirection GetCoverDirection();

/**
 * Assign new cover action to Pawn. (Defined in CoverLink.uc)
 *
 * @param	NewCA	New CoverAction.
 */
final function SetPawnCoverAction( ECoverAction NewCA )
{
	if( MyGearPawn != None &&
		MyGearPawn.CoverAction != NewCA )
	{
		MyGearPawn.SetCoverAction( NewCA );
	}
}

/**
 * Assign new cover type to Pawn. (Defined in CoverLink.uc)
 *
 * @param	NewCT	New CoverType.
 */
final function SetPawnCoverType( ECoverType NewCT )
{
	if( MyGearPawn != None )
	{
		MyGearPawn.SetCoverType( NewCT );
	}
}

/**
 * Set the new is in stationary cover flag on the pawn, and update
 * the replication flag.
 */
final function SetIsInStationaryCover(bool NewbIsInStationaryCover)
{
	if (MyGearPawn != None &&
		NewbIsInStationaryCover != MyGearPawn.bIsInStationaryCover)
	{
		MyGearPawn.bIsInStationaryCover = NewbIsInStationaryCover;
	}
}

simulated event NotifyDirectorControl(bool bNowControlling)
{
	local GearPlayerCamera GearCam;

	bInMatinee = bNowControlling;

	// if we were waiting to spawn in to avoid interfering with matinee control, try it now
	if (!bNowControlling && (IsInState('PlayerWaiting') || IsTimerActive('ServerRestartPlayer')))
	{
		ClearTimer('ServerRestartPlayer');
		ServerRestartPlayer();
	}

	// sanity check that matinee didn't reset view to the PC when it should be the Pawn
	//@hackish: and that it didn't get left on the final camera (especially can happen when they do camera-related Kismet simultaneously)
	if (!bNowControlling && Pawn != None && (GetViewTarget() == self || CameraActor(GetViewTarget()) != None))
	{
		if (Role == ROLE_Authority)
		{
			SetViewTarget(Pawn);
		}
		else
		{
			ServerVerifyViewTarget();
		}
	}

	if (Role == ROLE_Authority && GearGame(WorldInfo.Game).bSkipAllMatinees)
	{
		`log( "bSkipAllMatinees == TRUE so we are canceling this cine" );
		SetTimer( 2.0f, FALSE, nameof(CancelCine) );
	}

	if (!bNowControlling)
	{
		GearCam = GearPlayerCamera(PlayerCamera);
		if (GearCam != None)
		{
			// cancel any camera shakes
			GearCam.StopAllCameraAnims(TRUE);
			GearCam.GearCamMod_ScreenShake.RemoveAllScreenShakes();
		}
	}

	//@HACK: Big hack for Assault to make the already dead, torn off brumak go away for the cinematic
	// in the future should probably make Kismet not work on bTearOff stuff so LDs don't do things like this
	if (bNowControlling && WorldInfo.NetMode == NM_Client)
	{
		ClientDestroyDeadBrumaks();
	}

	super.NotifyDirectorControl(bNowControlling);
}

//@HACK: Big hack for Assault to make the already dead, torn off brumak go away for the cinematic
// in the future should probably make Kismet not work on bTearOff stuff so LDs don't do things like this
reliable client final function ClientDestroyDeadBrumaks()
{
	local GearPawn_LocustBrumakBase Brumak;

	foreach WorldInfo.AllPawns(class'GearPawn_LocustBrumakBase', Brumak)
	{
		if (Brumak.bTearOff)
		{
			Brumak.Destroy();
		}
	}
}

function TransitionFromRoadieRunToCover( CovPosInfo FoundCovInfo, optional bool bNoCameraAutoAlign )
{
	local bool bSeamlessTransition;

	bSeamlessTransition = FoundCovInfo.Normal dot vector(MyGearPawn.Rotation) > -0.83f;

	// temporarily disable the slam into cover to make it a seamless transition
	MyGearPawn.bCanDoRun2Cover = !bSeamlessTransition;

	EndSpecialMove();
	AcquireCover( FoundCovInfo, bNoCameraAutoAlign );

    // Play Wall impact effects
	MyGearPawn.DoRun2CoverWallHitEffects();

	if ( !bUseAlternateControls )
	{
		// if we didn't suck into an edge
		if (bSeamlessTransition && !MyGearPawn.IsAtLeftEdgeSlot() && !MyGearPawn.IsAtRightEdgeSlot())
		{
			// try a cover run
			DoSpecialMove(SM_CoverRun);
		}
		else
		{
			// otherwise release the 'A' button from roadie run to prevent getting popped out unintentionally
			ForceButtonRelease(bUseAlternateControls?GB_X:GB_A,TRUE);
		}
	}

	MyGearPawn.bCanDoRun2Cover = MyGearPawn.default.bCanDoRun2Cover;
}

// Update Targeting Mode state
simulated function UpdateTargetingStatus()
{
	local bool	bNewIsTargeting;

	if( !IsLocalPlayerController() )
	{
		return;
	}

	bNewIsTargeting = CanTarget() && WantsToTarget();
	if( bIsTargeting != bNewIsTargeting )
	{
		SetTargetingMode(bNewIsTargeting);
	}
}

/**  */
event PlayerTick(float DeltaTime)
{
	local CovPosInfo	FoundCovInfo;
	local Turret		TurretTest;
	local vector		CheckDir, CamLoc;
	local rotator		CamRot;
	local GearPawn		GP;
	local Name			DesiredSoundMode;

	// if we were looking at a POI that got deleted (streamed out), stop it
	if (bLookingAtPointOfInterest && CameraLookAtFocusActor == None)
	{
		StopForceLookAtPointOfInterest(false);
	}

	// make sure to update isincombat, as this will revive any teammates that are dnbo
	// when combat is over
	IsInCombat();
	if ( IsLocalPlayerController() )
	{
		// Update Targeting Mode state
		// note: doing this early in the tick, since later code depends
		// on this being set properly (don't want 1-frame delays in stuff)
		UpdateTargetingStatus();

		if (MyGearPawn != None)
		{
			// try to slide into cover when doing roadie run
			//@fixme - move this out of this tick and into GSM_RoadieRun
			if( !bUseAlternateControls && IsDoingSpecialMove(SM_RoadieRun) && !MyGearPawn.IsCarryingShield())
			{
				// Try to find cover to run to
				MyGearPawn.CoverAcquireFOV = MyGearPawn.RoadieRunCoverAcquireFOV;
				// if the stick is dramatically pushed then use that value for the check direction
				if (PlayerInput.RawJoyUp > DeadZoneThreshold || abs(PlayerInput.RawJoyRight) > DeadZoneThreshold)
				{
					GetPlayerViewPoint(CamLoc,CamRot);
					CheckDir.X = PlayerInput.RawJoyUp;
					CheckDir.Y = PlayerInput.RawJoyRight;
					CheckDir = Normal(CheckDir >> CamRot);
				}
				else
				{
					// otherwise default to the old style of player direction
					CheckDir = vector(Pawn.Rotation);
				}
				// if the check direction is relatively forward, then do the check for cover
				if( (CheckDir dot vector(Pawn.Rotation)) > 0.6 && MyGearPawn.CanPrepareRun2Cover(MyGearPawn.RoadieRun2CoverSlideDist, FoundCovInfo, CheckDir, MyGearPawn.PawnAcquireCoverFOVRoadieRun) )
				{
					TransitionFromRoadieRunToCover( FoundCovInfo );
				}
				MyGearPawn.CoverAcquireFOV = MyGearPawn.default.CoverAcquireFOV;
			}

			UpdateHUDActions(DeltaTime);

			if (Role == Role_Authority)
			{
				if (bCinematicMode)
				{
					MainPlayerInput.LastInputTime = WorldInfo.TimeSeconds;
				}
				else
				{
					if ( !MyGearPawn.bHidden && ((WorldInfo.TimeSeconds - MainPlayerInput.LastInputTime) > 60.f) && !IsActuallyPlayingCoop())
					{
						// throw guds event
						if (WorldInfo.Game != None)
						{
							GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PlayerHasntMoved, MyGearPawn);
						}

						// don't throw again for awhile.
						MainPlayerInput.LastInputTime = WorldInfo.TimeSeconds;
					}
				}
			}
		}
		else
		{
			TurretTest = Turret(Pawn);
			if ( TurretTest != None )
			{
				UpdateHUDActions(DeltaTime);
			}
		}
	}

	DesiredSoundMode = FindBestSoundMode();
	if (DesiredSoundMode != CurrentSoundMode)
	{
		SetSoundMode(DesiredSoundMode);
	}

	Super.PlayerTick(DeltaTime);

	MaintainEnemyList();

	// Checks to see if the profile settings should update the player settings
	CheckForProfileUpdate();

	// do the RIMShader FOV checking here.  We need to do this for all cases
	if (WorldInfo.GRI != None && WorldInfo.GRI.IsMultiplayerGame())
	{
		// Each tick we have to update the FOV information for the material
		ForEach WorldInfo.AllPawns(class'GearPawn', GP)
		{
			if( GP.MPRimShader != None )
			{
				if( !IsSpectating() && !GP.bTearOff && !GP.IsAHostage() )
				{
					// if we are a wingman game we need to colorize everyone except our buddy
					if( ClassIsChildOf(WorldInfo.GRI.GameClass, class'GearGameWingman_Base') )
					{
						if( WorldInfo.GRI.OnSameTeam(Pawn, GP) )
						{
							GP.MPRimShader.SetScalarParameterValue( 'LODDistanceFactor', 0 );
						}
						// we need to set to 0 here as the default value in the material is 1.0f
						else
						{
							GP.MPRimShader.SetScalarParameterValue( 'LODDistanceFactor', LODDistanceFactor );
						}
					}
					// else if we are not a wingman game then we colorize everyone
					else
					{
						GP.MPRimShader.SetScalarParameterValue( 'LODDistanceFactor', LODDistanceFactor );
					}

				}
				// if we are spectating then set the LODDistanceFactory on the RimShader to be 0 so we will have no rim shader at all
				else
				{
					GP.MPRimShader.SetScalarParameterValue( 'LODDistanceFactor', 0 );
				}
			}
		}
	}
}

/** Checks to see if the profile settings should update the player settings */
function CheckForProfileUpdate()
{
	// See if we need to update the game because of the profile setting being updated
	if ( bProfileSettingsUpdated && (ProfileSettings.ProfileSettings.length) > 0 && (GearPRI(PlayerReplicationInfo) != None) && (GearGRI(WorldInfo.GRI) != None) )
	{
		UpdateLocalCacheOfProfileSettings();
		bProfileSettingsUpdated = FALSE;
	}
}

/************************************************************************
	PROFILE/ONLINE FUNCTIONS
 ************************************************************************/
/** Registers a delegate so we get a callback when the player changes anything related to live. */
function RegisterOnlineDelegates()
{
	local OnlinePlayerInterface PlayerInterface;
	local OnlinePlayerInterfaceEx PlayerInterfaceEx;
	local LocalPlayer LocPlayer;

	Super.RegisterOnlineDelegates();

	LocPlayer = LocalPlayer(Player);

	// Figure out if we are a local player and have an online subsystem registered
	if (LocPlayer != None && OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			// Set the login delegate for the player
			PlayerInterface.AddLoginChangeDelegate(NotifyLoginChange,LocPlayer.ControllerId);
			PlayerInterface.AddLoginChangeDelegate(NotifyGlobalLoginChange);
			PlayerInterface.AddReadProfileSettingsCompleteDelegate(LocPlayer.ControllerId,NotifyProfileReadComplete);
		}

		PlayerInterfaceEx = OnlineSub.PlayerInterfaceEx;
		if(PlayerInterfaceEx != None)
		{
			PlayerInterfaceEx.AddDeviceSelectionDoneDelegate(LocPlayer.ControllerId,NotifyDeviceSelectComplete);
		}

		if (OnlineSub.SystemInterface != None)
		{
			// Set the system wide connection changed callback
			OnlineSub.SystemInterface.AddConnectionStatusChangeDelegate(OnConnectionStatusChange);
			OnlineSub.SystemInterface.AddLinkStatusChangeDelegate(OnLinkStatusChange);
		}
	}
}

/** Clears the online delegates for this PC. */
function ClearOnlineDelegates()
{
	local OnlinePlayerInterface PlayerInterface;
	local OnlinePlayerInterfaceEx PlayerInterfaceEx;
	local LocalPlayer LocPlayer;

	Super.ClearOnlineDelegates();

	LocPlayer = LocalPlayer(Player);

	// Figure out if we are a local player and have an online subsystem registered
	if (LocPlayer != None && OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			// Clear our delegate
			PlayerInterface.ClearLoginChangeDelegate(NotifyLoginChange,LocPlayer.ControllerId);
			PlayerInterface.ClearLoginChangeDelegate(NotifyGlobalLoginChange);
			PlayerInterface.ClearReadProfileSettingsCompleteDelegate(LocPlayer.ControllerId,NotifyProfileReadComplete);
		}

		PlayerInterfaceEx = OnlineSub.PlayerInterfaceEx;
		if(PlayerInterfaceEx != None)
		{
			PlayerInterfaceEx.ClearDeviceSelectionDoneDelegate(LocPlayer.ControllerId,NotifyDeviceSelectComplete);
		}

		if (OnlineSub.SystemInterface != None)
		{
			// Set the system wide connection changed callback
			OnlineSub.SystemInterface.ClearConnectionStatusChangeDelegate(OnConnectionStatusChange);
			OnlineSub.SystemInterface.ClearLinkStatusChangeDelegate(OnLinkStatusChange);
		}
	}
}

/**
 * login changed handler.  Usually returns the player to the title screen.
 *
 * @param	NewStatus		The new login state of the player.
 */
function OnLoginChanged(ELoginStatus NewStatus)
{
	local OnlineGameSettings PartySettings;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
		if (PartySettings != None)
		{
			// Check for being signed out altogether
			if (NewStatus == LS_NotLoggedIn)
			{
				ButtonAliases.AddItem('GenericAccept');
				GameSceneClient = class'UIInteraction'.static.GetSceneClient();
				GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedProfileForMultiplayer_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedProfileForMultiplayer_Message>",
					"",
					ButtonAliases, ReturnToMainMenuUponAccept, LocalPlayer(Player));
			}
			// Check for being signed out of Live (ok for system link)
			else if (NewStatus < LS_LoggedIn && !PartySettings.bIsLanMatch)
			{
				ButtonAliases.AddItem('GenericAccept');
				GameSceneClient = class'UIInteraction'.static.GetSceneClient();
				GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTier_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTier_Message>",
					"",
					ButtonAliases, ReturnToMainMenuUponAccept, LocalPlayer(Player));
			}
		}
		// Check for offline<->online
		HasSigninChanged();
	}
}

/** @return true if the player signed in a different player, false if the same */
function bool HasSigninChanged()
{
	local UniqueNetId CurrentId;
	local byte ControllerId;

	// Figure out if we have an online subsystem registered
	if (OnlineSub != None && OnlineSub.PlayerInterface != None)
	{
		ControllerId = LocalPlayer(Player).ControllerId;

		if (OnlineSub.PlayerInterface.GetUniquePlayerId(ControllerId,CurrentId))
		{
			// If the IDs match, they are the same player
			if (CurrentId == PlayerReplicationInfo.UniqueId)
			{
				return false;
			}
			else
			{
				// They don't match so compare their name
				// NOTE: Local only profiles can't go from offline<->online so skip those as this means a change happened
				if (!OnlineSub.PlayerInterface.IsLocalLogin(ControllerId) &&
					OnlineSub.PlayerInterface.GetPlayerNickname(ControllerId) ~= PlayerReplicationInfo.PlayerName)
				{
					// They swapped from offline to online or vice versa
					PlayerReplicationInfo.UniqueId = CurrentId;
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

`if(`notdefined(ShippingPC))
`define	debugexec exec
`else
`define debugexec
`endif

/**
* Called when a system level connection change notification occurs. If we are
* playing a Live match, we may need to notify and go back to the menu. Otherwise
* silently ignore this.
*
* @param ConnectionStatus the
*/
`{debugexec} function OnConnectionStatusChange(EOnlineServerConnectionStatus ConnectionStatus)
{
	local OnlineGameSettings GameSettings;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;
	local GearGRI GRI;

	// We need to always bail in this case
	if (ConnectionStatus == OSCS_DuplicateLoginDetected)
	{
		ButtonAliases.AddItem('GenericAccept');
		GameSceneClient = class'UIInteraction'.static.GetSceneClient();
		GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
			"<Strings:GearGameUI.MessageBoxErrorStrings.DuplicateSignin_Title>",
			"<Strings:GearGameUI.MessageBoxErrorStrings.DuplicateSignin_Message>",
			"",
			ButtonAliases, ReturnToMainMenuUponAccept, LocalPlayer(Player));
	}
	else if ( !IsCoop() )
	{
		// We know we have an online subsystem or this delegate wouldn't be called
		// Check the party session, because a Game session isn't always created
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Party');
		if (GameSettings != None || WorldInfo.NetMode == NM_Standalone)
		{
			// If we are a Live match, this really matters
			if ((GameSettings != None && !GameSettings.bIsLanMatch) ||
				(WorldInfo.NetMode == NM_Standalone && !IsSplitscreenPlayer() && !WorldInfo.bIsMenuLevel))
			{
				GRI = GearGRI(WorldInfo.GRI);
				// Make sure this isn't Training Grounds
				if (GRI == none || GRI.TrainingGroundsID < 0)
				{
					// We are playing a Live match. Determine whether the connection
					// status change requires us to drop and go to the menu
					switch (ConnectionStatus)
					{
						case OSCS_ConnectionDropped:
						case OSCS_NoNetworkConnection:
						case OSCS_ServiceUnavailable:
						case OSCS_UpdateRequired:
						case OSCS_ServersTooBusy:
							ButtonAliases.AddItem('GenericAccept');
							GameSceneClient = class'UIInteraction'.static.GetSceneClient();
							GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
								"<Strings:GearGameUI.MessageBoxErrorStrings.XboxLive_Title>",
								"<Strings:GearGameUI.MessageBoxErrorStrings.LostLiveConnection_Message>",
								"",
								ButtonAliases, ReturnToMainMenuUponAccept, LocalPlayer(Player));
							break;
					}
				}
			}
		}
	}
}

/**
 * Displays a message when the physical network is lost during a networked game
 *
 * @param bIsConnected the new connection state
 */
`{debugexec} function OnLinkStatusChange(bool bIsConnected)
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (!bIsConnected &&
		// Ignore this for local matches (matches without a party session)
		OnlineSub.GameInterface.GetGameSettings('Party') != None)
	{
		// Check for host tampering and deny them the win
		if (WorldInfo.NetMode != NM_Client && !GRI.bIsCoop)
		{
			GearGame(WorldInfo.Game).SetHostTamperedWithMatch();
			// Mark the server as having a problem
			WorldInfo.Game.bHasNetworkError = true;
			// Force off the scoreboard
			MyGearHud.ToggleScoreboard(FALSE);
			MyGearHud.bRestrictScoreboard = true;
		}
		// Ignore this change if standalone
		if (WorldInfo.NetMode != NM_Standalone)
		{
			// If we are playing networked co-op and are a client,
			// or are in a versus match, show the message
			if (WorldInfo.NetMode == NM_Client || !GRI.bIsCoop)
			{
				ShowMessageBox("<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.LostLink_Message>",
					'ConnectionError');
				ReturnToMainMenu();
			}
		}
	}
}

/**
 * Returns the player to the main menu when the connection is lost
 */
function bool ReturnToMainMenuUponAccept(UIMessageBoxBase Sender,name SelectedInputAlias,int PlayerIndex)
{
	if (WorldInfo.Game == None || WorldInfo.Game.Class != class'GearMenuGame')
	{
		ReturnToMainMenu();
	}
	return true;
}

/**
 * total hack - returns false in horde so that we don't pause the game in horde when the host's gamepad is disconnected
 */
function bool AllowPauseForControllerRemovalHack()
{
	return true;
}

/**
 * Displays the pause menu when the controller becomes disconnected
 *
 * @param ControllerId the controller id that was disconnected
 * @param bIsConnected whether the controller was removed or inserted
 */
function OnControllerChanged(int ControllerId,bool bIsConnected)
{
	local LocalPlayer LP/*, OtherPlayer*/;
	local bool bIsTraveling, bMenuLevel, bOpenPauseMenu, bApplyPostProcess;
	local OnlinePlayerInterface PlayerInterface;
	local byte PlayerID;
	local int /*PlayerIndex, */NumPlayers;
	local string ActiveMovieName;

	LP = LocalPlayer(Player);
	if ( LP != None && WorldInfo.IsConsoleBuild()
	// do not pause if there is no controller when we are automatedperftesting
	&&	(WorldInfo.Game == None || !WorldInfo.Game.bAutomatedPerfTesting))
	{
		PlayerID = LP.ControllerId;
		if ( PlayerID == ControllerId )
		{
			// Update our local state of the controller, used for the unpause delegate.
			bIsControllerConnected = bIsConnected;

			`log("Received gamepad connection change for player" @ class'UIInteraction'.static.GetPlayerIndex(ControllerId) $ ": gamepad" @ ControllerId @ "is now" @ (bIsConnected ? "connected" : "disconnected"));

			bMenuLevel = class'WorldInfo'.static.IsMenuLevel();

			// Figure out if we have an online subsystem registered
			if (OnlineSub != None)
			{
				// Grab the player interface to verify the subsystem supports it
				PlayerInterface = OnlineSub.PlayerInterface;
				if (PlayerInterface != None)
				{
					// Don't pause if we are getting ready to travel or playing a movie
					GetCurrentMovie(ActiveMovieName);
					bIsTraveling = GameEngine(LP.Outer).TravelURL != "" || ActiveMovieName ~= class'GearUIInteraction'.const.LOADING_MOVIE;

					// if the gamepad was disconnected and we aren't travelling
					if ( !bIsTraveling && !bIsConnected )
					{
						bApplyPostProcess = !bMenuLevel;
						if ( WorldInfo.Game != None )
						{
							NumPlayers = GearGame(WorldInfo.Game).GetHumanPlayerCount();
							if ( NumPlayers == class'UIInteraction'.static.GetPlayerCount() || GearGRI(WorldInfo.GRI).bIsCoop )
							{
								bOpenPauseMenu = AllowPauseForControllerRemovalHack();
							}
							else
							{
								// we have one or more players connected to our machine - can't pause
							}
						}
						else
						{
							// we're a client - no pausing.
						}
					}
				}
			}

			if ( bOpenPauseMenu )
			{
				PauseGame(CanUnpauseControllerConnected);
			}
			else if ( bApplyPostProcess )
			{
				LP.OverridePostProcessSettings( PausePPSettings, WorldInfo.RealTimeSeconds );
			}
			else if ( bIsConnected )
			{
				if ( bMenuLevel )
				{
					SetPause(false);
				}
				else if (LP.bOverridePostProcessSettings && LP.PostProcessSettingsOverride == PausePPSettings
					&&	(MyGearHUD == None || MyGearHUD.PauseUISceneInstance == None))
				{
					LP.ClearPostProcessSettingsOverride();

					// unhide the hud
					//?? UnhideHUDCleanup();

					// might have to start another post process
					CheckForPostProcessReset( Self );
				}
			}
		}
	}
}

/** Callback for when the player's profile is finished loading. */
function NotifyProfileReadComplete(byte LocalUserNum,bool bWasSuccessful)
{
	local OnlinePlayerInterface PlayerInterface;
	local UniqueNetId CurrentId;
	local byte ControllerId;

	// Figure out if we have an online subsystem registered
	if (OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			ControllerId = LocalPlayer(Player).ControllerId;
			`log("Profile Read Complete: Player " $ ControllerId @ `showvar(LocalUserNum) @ `showvar(bWasSuccessful));

			// Update local profile cache next tick
			bProfileSettingsUpdated = TRUE;

			// If the read completes and the IDs don't match, re-init the IDs
			if (PlayerInterface.GetUniquePlayerId(ControllerId,CurrentId))
			{
				if (CurrentId != PlayerReplicationInfo.UniqueId)
				{
					`Log("Re-initing the unique id");
					InitUniquePlayerId();
				}
			}
		}
	}


	if ( WorldInfo.GRI == None || GearGRI(WorldInfo.GRI).IsMultiPlayerGame() )
	{
		SetShowSubtitles( FALSE );
	}

	// Check for whether we've posted to MCP yet or not
	if (ProfileSettings != None)
	{
		if ( GetUIPlayerIndex() == 0 )
		{
			ProfileSettings.ApplyNavPathAttractionToProvider();
		}

		if ( !ProfileSettings.HaveHardwareStatsBeenUploaded() )
		{
			UploadProfileToMCP();
		}
	}
}

/**
 * Determines whether creating or removing local players is allowed in the current match.  Not supported when in the front-end.
 *
 * @return	TRUE to allow players to be created or removed in response to login-status changes; FALSE to end the game and
 *			return to main menu if login-status has changed.
 */
function bool IsAllowedToCreatePlayers()
{
	local bool bResult;

	// in the front-end, allow the scenes to handle the logic for creating/destroying players
	if ( !class'WorldInfo'.static.IsMenuLevel() )
	{
//@todo joeg/robm - add additional logic for allowing players to be created or destroyed during various MP game modes (system-link, etc.)
		if ( WorldInfo.GRI != None )
		{
			// don't allow players to be created in multi-player.
			bResult = IsCoop() || !WorldInfo.GRI.IsMultiplayerGame();
		}
	}


	return bResult;
}

/**
 * Determines whether sign-in status changes are allowed in the current match.  Not supported when in the front-end.
 *
 * @return	TRUE to allow players to be created or removed in response to login-status changes; FALSE to end the game and
 *			return to main menu if login-status has changed.
 */
function bool IsSigninChangeAllowed()
{
	local GearGRI GGRI;
	local bool bResult;

	bResult = true;

	// in the front-end, allow the scenes to handle the logic for sign-in status changes
	if ( !class'WorldInfo'.static.IsMenuLevel() )
	{
		GGRI = GearGRI(WorldInfo.GRI);
		if ( GGRI != None )
		{
			if ( GGRI.TrainingGroundsID > INDEX_NONE )
			{
				bResult = false;
			}
			else if ( GGRI.IsMultiPlayerGame() )
			{
				// playing a multi-player match (everything except campaign)
				bResult = false;
			}

//@todo joeg/robm - add additional logic for disallowing sign-in changes (i.e. in live mp match, etc.) or fix this logic if it's not right
		}
	}

	return bResult;
}

/** Callback for when ANY player changes their currently signed in gamer tag */
function NotifyGlobalLoginChange()
{
	local GameUISceneClient GameSceneClient;
	local LocalPlayer LP;
	local OnlinePlayerInterface PlayerInterface;

	if ( IsPrimaryPlayer() )
	{
		// if we don't yet have a GRI, try again in .2 seconds
		if ( WorldInfo.GRI == None )
		{
			SetTimer( 0.2, false, nameof(NotifyGlobalLoginChange) );
			return;
		}

		if ( IsSigninChangeAllowed() )
		{
			if ( IsAllowedToCreatePlayers() )
			{
				GameSceneClient = class'UIInteraction'.static.GetSceneClient();
				if ( GameSceneClient != None )
				{
					// do not allow player removal or OnLoginChange will not be called for players that leave.
					GameSceneClient.SynchronizePlayers(,,false);
				}
			}
		}
		else
		{
			PlayerInterface = OnlineSub.PlayerInterface;
			LP = LocalPlayer(Player);
			if (PlayerInterface.GetLoginStatus(LP.ControllerId) == LS_NotLoggedIn)
			{
				class'GearUIScene_Base'.static.ProcessIllegalPlayerLoginStatusChange(LP.ControllerId, true);
		    }
		}
	}
}

/** Callback for when the player changes their currently signed in gamer tag */
function NotifyLoginChange()
{
	local OnlinePlayerInterface PlayerInterface;
	local GearGRI GGRI;
	local GearEngine Engine;

	`if(`notdefined(FINAL_RELEASE))
		local byte PlayerID;
	local ELoginStatus LoginStatus;
	`endif

	// Reset saving of checkpoints so that later sign ins can save their data
	Engine = GearEngine(Player.Outer);
	if (Engine != None)
	{
		Engine.bShouldWriteCheckpointToDisk = true;
	}

	// Figure out if we have an online subsystem registered
	if (OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			`if(`notdefined(FINAL_RELEASE))
				PlayerID = LocalPlayer(Player).ControllerId;
			// See if the player is still logged in
			LoginStatus = PlayerInterface.GetLoginStatus(PlayerID);

			`log("Login Change: Player " $ PlayerID $ " changed login status to " $ LoginStatus);
			`endif

			GGRI = GearGRI(WorldInfo.GRI);
			if ( GGRI != None )
			{
				if ( GGRI.TrainingGroundsID > INDEX_NONE )
				{
					class'GearUIScene_Base'.static.DisplaySigninChangeDetected_ErrorMessage();
					ReturnToMainMenu();
				}
			}

			// Check for host tampering and deny them the win
			if (WorldInfo.NetMode != NM_Client &&
				!GearGRI(WorldInfo.GRI).bIsCoop &&
				!OnlineSub.SystemInterface.HasLinkConnection())
			{
				GearGame(WorldInfo.Game).SetHostTamperedWithMatch();
				// Mark the server as having a problem
				WorldInfo.Game.bHasNetworkError = true;
				// Force off the scoreboard
				MyGearHud.ToggleScoreboard(FALSE);
				MyGearHud.bRestrictScoreboard = true;
			}

			// Check the player's login status immediately for in-game events.
			UpdateLoginStatus();
		}
	}
}

/** Retrieves the current status and calls the OnLoginChanged function. */
function UpdateLoginStatus()
{
	local OnlinePlayerInterface PlayerInterface;
	local ELoginStatus LoginStatus;
	local byte PlayerID;

	PlayerID = LocalPlayer(Player).ControllerId;

	UpdateGamerTag();

	// Figure out if we have an online subsystem registered
	if (OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			// See if the player is still logged in
			LoginStatus = PlayerInterface.GetLoginStatus(PlayerID);
			OnLoginChanged(LoginStatus);
		}
	}
}

/** Callback for when the player selects a storage device. */
function NotifyDeviceSelectComplete(bool bWasSuccessful)
{
	local int DeviceID;
	local string DeviceName;
	local OnlinePlayerInterfaceEx PlayerInterfaceEx;

	// If they changed their storage device, update the profile with the new device.
	if(bWasSuccessful && ProfileSettings != none)
	{
		if (OnlineSub != None)
		{
			// Grab the player interface to verify the subsystem supports it
			PlayerInterfaceEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerInterfaceEx != None)
			{
				DeviceID = PlayerInterfaceEx.GetDeviceSelectionResults(LocalPlayer(Player).ControllerId, DeviceName);
				ProfileSettings.SetCurrentDeviceID(DeviceID);

				`Log("Storage Device Change: Player " $ (LocalPlayer(Player).ControllerId) $ " changed Device ID to " $ DeviceID $ "(" $ DeviceName $ ")");
			}
		}
	}
}

/** Set the player name */
function UpdateGamerTag()
{
	local LocalPlayer LocPlayer;
	local string Gamertag;

	if (OnlineSub != None)
	{
		LocPlayer = LocalPlayer(Player);
		if (LocPlayer != None)
		{
			// Read the gamertag for this player and change our name to it
			Gamertag = OnlineSub.PlayerInterface.GetPlayerNickname(LocPlayer.ControllerId);
			// Use a server function to do this
			ServerSetGamerTag(Gamertag);
		}
	}
}

/** Used to assign the gamertag as the player name for multiplayer matches */
event InitUniquePlayerId()
{
	local LocalPlayer LocPlayer;
	local OnlineGameSettings GameSettings;
	local UniqueNetId PlayerId;
	local UniqueNetId ZeroId;

	UpdateGamerTag();

	if ( PlayerReplicationInfo != None )
	{
		LocPlayer = LocalPlayer(Player);
		// If we have both a local player and the online system, register ourselves
		if (LocPlayer != None &&
			PlayerReplicationInfo != None &&
			OnlineSub != None &&
			OnlineSub.PlayerInterface != None)
		{
			// Get our local id from the online subsystem
			OnlineSub.PlayerInterface.GetUniquePlayerId(LocPlayer.ControllerId,PlayerId);
			PlayerReplicationInfo.SetUniqueId(PlayerId);

			if (WorldInfo.NetMode == NM_Client)
			{
				if (WorldInfo.IsConsoleBuild() && PlayerId == ZeroId)
				{
					OnLoginChanged(LS_NotLoggedIn);
				}
				else
				{
					// Grab the game so we can check for being invited
					if (OnlineSub.GameInterface != None)
					{
						GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
					}

					ServerSetUniquePlayerId(PlayerId, GameSettings != None && GameSettings.bWasFromInvite);
					bReceivedUniqueId = true;

					// Don't send a skill if we don't have one, or it hasn't been read yet
					if (PlayerReplicationInfo.PlayerSkill > 0)
					{
						ServerSetPlayerSkill(PlayerReplicationInfo.PlayerSkill);
					}
				}
			}
		}
	}
	else
	{
		`log(`location $ ": PlayerReplicationInfo is None....aborting.",,'DevOnline');
	}
}

/** Create a player if the number of local players is less than the number of logged in controllers */
function CreatePlayerForNewlyLoggedInController()
{
	local OnlinePlayerInterface PlayerInt;
	local int PlayerIdx, NumLoggedIn, ControllerIdMask, NumLocalPlayers;
	local GameViewportClient VPClient;
	local bool bIsLoggedIn;
	local String Error;

	VPClient = LocalPlayer(Player).ViewportClient;
	// Figure out if we have an online subsystem registered
	if ( (OnlineSub != None) && (VPClient != None) && !class'WorldInfo'.static.IsMenuLevel() )
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			NumLocalPlayers = VPClient.GamePlayers.length;
			// if we already have 2 localplayers, do nothing
			if ( NumLocalPlayers < 2 )
			{
				// calculate the controller mask and find out how many players are logged in
				for ( PlayerIdx = 0; PlayerIdx < 4; PlayerIdx++ )
				{
					if( PlayerInt.GetLoginStatus(PlayerIdx) != LS_NotLoggedIn )
					{
						ControllerIdMask = ControllerIdMask | (1 << PlayerIdx);
						NumLoggedIn++;
					}
				}

				// if we have more logged in controllers than players, create a player
				if ( NumLocalPlayers < NumLoggedIn )
				{

					`Log("Trying to spawn new PC for coop split, IsActuallyPlayingCoop: " $ IsActuallyPlayingCoop());

					// If we aren't playing coop over the net already, create a PC.
					if(IsActuallyPlayingCoop()==false && WorldInfo.NetMode!=NM_Client)
					{
						// create a player for the first controller we find that doesn't have one
						for ( PlayerIdx = 0; PlayerIdx < 4; PlayerIdx++ )
						{
							bIsLoggedIn = ((ControllerIdMask & (1 << PlayerIdx)) != 0);
							// if this controller is logged in but w/o a player make one
							if ( bIsLoggedIn && (VPClient.FindPlayerByControllerId(PlayerIdx) == None) )
							{
								if ( VPClient.CreatePlayer(PlayerIdx, Error, true) != None )
								{
									// found the new controller and spawned the player so exit now
									return;
								}
							}
						}
					}
// 					else
// 					{
// 						// Kick the user back to the main menu, letting them know that only 1 player can play online at once.
// 						DisplayErrorMessageBox("<Strings:WarfareGame.ErrorMessages.OnePlayerOnlyCoop_Title>",
// 							"<Strings:WarfareGame.ErrorMessages.OnePlayerOnlyCoop_Message>",
// 							OkToTravel);
// 					}
				}
			}
		}
	}
}

/**
 * Cleans up any Live related code that wasn't gracefully cleaned up in the
 * previous match. This is a catch all for cleaning up once you've arrived
 * back in the menus. This code should be unnecessary, but it's better to
 * be safe than sorry.
 */
function CleanupUngracefulReturnToMenu()
{
	// Check for being in the menus
	if (class'WorldInfo'.static.IsMenuLevel() || GetURLMap() == "")
	{
		// Now see if there is a left over game session going
		if (OnlineSub != None &&
			OnlineSub.GameInterface != None &&
			OnlineSub.GameInterface.GetGameSettings('Game') != None)
		{
			`Log("^^^^^ Cleaning up left over game settings");
			// Set the end delegate so we can know when that is complete and call destroy
			OnlineSub.GameInterface.AddEndOnlineGameCompleteDelegate(OnEndDuringCleanupComplete);
			// We have a left over game, so clean up
			OnlineSub.GameInterface.EndOnlineGame('Game');
		}
	}
}

/**
 * Called when the end online game has completed
 *
 * @param SessionName the name of the session that just ended
 * @param bWasSuccessful whether it worked ok or not
 */
function OnEndDuringCleanupComplete(name SessionName,bool bWasSuccessful)
{
	OnlineSub.GameInterface.ClearEndOnlineGameCompleteDelegate(OnEndDuringCleanupComplete);
	// Set the destroy delegate so we can know when that is complete
	OnlineSub.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyDuringCleanupComplete);
	// Now we can destroy the game
	OnlineSub.GameInterface.DestroyOnlineGame(SessionName);
}

/**
 * Called when the destroy online game has completed. At this point it is safe
 * to travel back to the menus
 *
 * @param SessionName the name of the session that was destroyed
 * @param bWasSuccessful whether it worked ok or not
 */
function OnDestroyDuringCleanupComplete(name SessionName,bool bWasSuccessful)
{
	OnlineSub.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyDuringCleanupComplete);
//	FinishOnQuitToMainMenu();
}

/** Display a message to the user */
function NotifyInviteFailed()
{
	ClientShowLoadingMovie(false);
	class'GearUIScene_Base'.static.DisplayErrorMessage("InviteHandlingFailed_Message", "InviteHandlingFailed_Title", "GenericAccept");
}

/** Display a message to the user */
function NotifyNotAllPlayersCanJoinInvite()
{
	ClientShowLoadingMovie(false);
	class'GearUIScene_Base'.static.DisplayErrorMessage("NotAllPlayersCanJoin_Message", "InviteHandlingFailed_Title", "GenericAccept");
}

/** Display a message to the user */
function NotifyNotEnoughSpaceInInvite()
{
	ClientShowLoadingMovie(false);
	class'GearUIScene_Base'.static.DisplayErrorMessage("NotEnoughSpaceToJoin_Message", "InviteHandlingFailed_Title", "GenericAccept");
}

/**
 * Delegate called when the user accepts a game invite externally. This allows
 * the game code a chance to clean up before joining the game via
 * AcceptGameInvite() call.
 *
 * NOTE: There must be space for all signed in players to join the game. All
 * players must also have permission to play online too.
 *
 * @param GameInviteSettings the settings for the game we're to join
 */
function OnGameInviteAccepted(OnlineGameSettings GameInviteSettings)
{
	local OnlineGameSettings GameSettings;
	local OnlineGameSettings PartySettings;

	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		if (GameInviteSettings != None)
		{
			// Make sure the new game has space
			if (InviteHasEnoughSpace(GameInviteSettings))
			{
				// Make sure everyone logged in can play online
				if (CanAllPlayersPlayOnline())
				{
					bIsReturningFromMatch = true;
					bIgnoreNetworkMessages = true;
					if (WorldInfo.NetMode != NM_Standalone)
					{
						WorldInfo.GRI.bNeedsOnlineCleanup = false;
						GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
						if (GameSettings != None)
						{
							// Write arbitration data, if required
							if (GameSettings.bUsesArbitration)
							{
								// Write out our version of the scoring before leaving
								ClientWriteOnlinePlayerScores(WorldInfo.GRI.GameClass != None ? WorldInfo.GRI.GameClass.default.ArbitratedLeaderBoardId : 0);
							}
							// Set the end delegate, where we'll destroy the game and then join
							OnlineSub.GameInterface.AddEndOnlineGameCompleteDelegate(OnEndGameForInviteComplete);
							// Force the flush of the stats
							OnlineSub.GameInterface.EndOnlineGame('Game');
						}
						else
						{
							// Check for party and clean up
							PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
							if (PartySettings != None)
							{
								// Destroy the party and then accept the invite
								OnlineSub.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyPartyForInviteComplete);
								OnlineSub.GameInterface.DestroyOnlineGame('Party');
							}
							else
							{
								// Set the delegate for notification of the join completing
								OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
								// We can immediately accept since there is no online game
								OnlineSub.GameInterface.AcceptGameInvite(LocalPlayer(Player).ControllerId,'Party');
							}
						}
					}
					else
					{
						// Set the delegate for notification of the join completing
						OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);

						// We can immediately accept since there is no online game
						if (!OnlineSub.GameInterface.AcceptGameInvite(LocalPlayer(Player).ControllerId,'Party'))
						{
							OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
						}
					}
				}
				else
				{
					// Display an error message
					NotifyNotAllPlayersCanJoinInvite();
				}
			}
			else
			{
				// Display an error message
				NotifyNotEnoughSpaceInInvite();
			}
		}
		else
		{
			// Display an error message
			NotifyInviteFailed();
		}
	}
}

/**
 * Delegate called once the destroy of an online game before accepting an invite
 * is complete. From here, the game invite can be accepted
 *
 * @param SessionName the name of the session that was ended
 * @param bWasSuccessful whether the end completed properly or not
 */
function OnEndGameForInviteComplete(name SessionName,bool bWasSuccessful)
{
	// Clear the end delegate
	OnlineSub.GameInterface.ClearEndOnlineGameCompleteDelegate(OnEndGameForInviteComplete);
	// Set the destroy delegate so we can know when that is complete
	OnlineSub.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyGameForInviteComplete);
	// Now we can destroy the game
	OnlineSub.GameInterface.DestroyOnlineGame('Game');
}

/**
 * Delegate called once the destroy of an online game before accepting an invite
 * is complete. From here, the game invite can be accepted
 *
 * @param SessionName the name of the session that was destroyed
 * @param bWasSuccessful whether the destroy completed properly or not
 */
function OnDestroyGameForInviteComplete(name SessionName,bool bWasSuccessful)
{
	local OnlineGameSettings PartySettings;

	OnlineSub.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyGameForInviteComplete);

	// Check for party and clean up
	PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
	if (PartySettings != None)
	{
		// Destroy the party and then accept the invite
		OnlineSub.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyPartyForInviteComplete);
		OnlineSub.GameInterface.DestroyOnlineGame('Party');
	}
	else
	{
		// Set the delegate for notification of the join completing
		OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
		// This will have us join async
		OnlineSub.GameInterface.AcceptGameInvite(LocalPlayer(Player).ControllerId,'Party');
	}
}

/**
 * Delegate called once the destroy of an online game before accepting an invite
 * is complete. From here, the game invite can be accepted
 *
 * @param SessionName the name of the session that was destroyed
 * @param bWasSuccessful whether the destroy completed properly or not
 */
function OnDestroyPartyForInviteComplete(name SessionName,bool bWasSuccessful)
{
	OnlineSub.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyPartyForInviteComplete);
	// Set the delegate for notification of the join completing
	OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
	// This will have us join async
	OnlineSub.GameInterface.AcceptGameInvite(LocalPlayer(Player).ControllerId,'Party');
}

/**
 * Once the join completes, use the platform specific connection information
 * to connect to it
 *
 * @param SessionName the name of the session that was joined
 * @param bWasSuccessful whether the end completed properly or not
 */
function OnInviteJoinComplete(name SessionName,bool bWasSuccessful)
{
	local string URL;

	// Forcefully stop the movie, no matter what
	ClientStopMovie(0.0, false, true, true);
	CloseAllUIMessageBoxes();
	if (bWasSuccessful)
	{
		if (OnlineSub != None && OnlineSub.GameInterface != None)
		{
			OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
			// Get the platform specific information
			if (OnlineSub.GameInterface.GetResolvedConnectString('Party',URL))
			{
				URL $= "?bIsFromInvite";

				bIsReturningFromMatch = false;

				`Log("Resulting url is ("$URL$")");
				// And now start the loading movie while we travel
				ClientShowLoadingMovie(true);
				ConsoleCommand("start "$URL);
			}
		}
	}
	else
	{
		ClientShowLoadingMovie(false);
		// Do some error handling
		class'GearUIScene_Base'.static.DisplayErrorMessage("InviteHandlingFailed_Message", "InviteHandlingFailed_Title", "GenericAccept");
	}
}

/** Closes all message boxes that might be open */
function CloseAllUIMessageBoxes()
{
	local GameUISceneClient GameSceneClient;
	local int SceneIndex;
	local UIScene Scene;

	GameSceneClient = class'UIInteraction'.static.GetSceneClient();
	// Find all message box scenes and close them
	// NOTE: These are all uniquely tagged, so you can't use find by tag
	SceneIndex = 0;
	do
	{
		Scene = GameSceneClient.GetSceneAtIndex(SceneIndex);
		if (UIMessageBoxBase(Scene) != None)
		{
			Scene.CloseScene(Scene,true,true);
		}
		SceneIndex++;
	}
	until (Scene == None);
}

/**
 * Attempts to pause/unpause the game when the UI opens/closes. Note: pausing
 * only happens in standalone mode
 *
 * @param bIsOpening whether the UI is opening or closing
 */
function OnExternalUIChanged(bool bIsOpening)
{
	local UIInteraction UIController;
	local string CurrentMovieName;

	`log(`location @ `showvar(bIsOpening) @ `showvar(bIsExternalUIOpen));
	if ( bDeleteMe )
	{
		bIsOpening = false;
	}
	else if( bIsExternalUIOpen == bIsOpening )
	{
//		`log( "ERROR: Guide is already in that state"@bIsOpening );
		return;
	}

	if ( bIsOpening )
	{
		bRun = 0;
		bFire = 0;

		// Turn off targeting when you go to Guide
		bIsTargeting = FALSE;
		SetTargetingMode( FALSE );

		IgnoreMoveInput( TRUE );
		IgnoreLookInput( TRUE );
	}
	else
	{
		IgnoreMoveInput( FALSE );
		IgnoreLookInput( FALSE );
	}

	// Do pause the if you open a blade (even coop game - TTP 62072)
	UIController = class'UIRoot'.static.GetCurrentUIController();
	bIsExternalUIOpen = bIsOpening;

	if ( IsPrimaryPlayer() || !UIController.ShouldForceFullscreenViewport() )
	{
		// Don't pause the game if we're waiting on a load screen or watching a cutscene.  This
		// can actually cause a soft-lock due to how post-loading movie pausing works
		GetCurrentMovie(CurrentMovieName);
		if( CurrentMovieName == "" )
		{
			if ( bIsOpening )
			{
				PauseGame(CanUnpauseExternalUI);
			}
			else
			{
				SetPause(false);
			}
		}
	}
}

/**
 * Tells the clients to write the stats using the specified stats object
 *
 * @param OnlineStatsWriteClass the stats class to write with
 */
reliable client function ClientWriteLeaderboardStats(class<OnlineStatsWrite> OnlineStatsWriteClass)
{
 	local GearLeaderboardWriteBase Stats;
 	local GearPRI PRI;
 	local GearGRI GRI;
 	local int Index;
	local UniqueNetId ZeroId;
	local OnlineGameSettings GameSettings;
	local int RoundsWon;
	local int RoundsLost;
	local int RoundCount;
	local int TeamIndex;

	// Only calc this if the subsystem can write stats
 	if (OnlineSub != None &&
 		OnlineSub.StatsInterface != None &&
 		OnlineSub.GameInterface != None &&
 		OnlineStatsWriteClass != None)
 	{
		// Get the game setting so we can determine whether to write stats for none, one or all players
		GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
		if (GameSettings != None)
		{
			// Store the score data for review in the party lobby
			StoreLastMatchData(OnlineStatsWriteClass,GameSettings.bUsesArbitration);
			// Create the write objec that we are going to submit to live
 			Stats = GearLeaderboardWriteBase(new OnlineStatsWriteClass);
			// Get the GRI so we can iterate players
			GRI = GearGRI(WorldInfo.GRI);
			// Round count is not updated on the last round for some reason
			RoundCount = GRI.RoundCount + 1;
			// Arbitration requires us to report for everyone, whereas non-arbitrated is just for ourselves
			if (GameSettings.bUsesArbitration)
			{
 				// Iterate through the active PRI list updating the stats
 				for (Index = 0; Index < GRI.PRIArray.Length; Index++)
 				{
 					PRI = GearPRI(GRI.PRIArray[Index]);
 					if (!PRI.bIsInactive &&
						PRI.UniqueId != ZeroId)
 					{
						// Update the round information
						RoundsWon = Min(RoundCount,PRI.Team.Score);
						RoundsLost = 0;
						// Figure out how many rounds lost to other team(s)
						for (TeamIndex = 0; TeamIndex < GRI.Teams.Length; TeamIndex++)
						{
							// If this is a different team, then add to the loss total
							if (PRI.Team.TeamIndex != GRI.Teams[TeamIndex].TeamIndex)
							{
								RoundsLost += GRI.Teams[TeamIndex].Score;
							}
						}
						// Copy the stats from the PRI to the object
						Stats.UpdateFromPRI(PRI,RoundCount,RoundsWon,RoundsLost);
 						// This will copy them into the online subsystem where they will be
 						// sent via the network in EndOnlineGame()
 						OnlineSub.StatsInterface.WriteOnlineStats(PRI.SessionName,PRI.UniqueId,Stats);
 					}
 				}
			}
			else
			{
				PRI = GearPRI(PlayerReplicationInfo);
				// Update the round information
				RoundsWon = Min(RoundCount,PRI.Team.Score);
				RoundsLost = 0;
				// Figure out how many rounds lost to other team(s)
				for (TeamIndex = 0; TeamIndex < GRI.Teams.Length; TeamIndex++)
				{
					// If this is a different team, then add to the loss total
					if (PRI.Team.TeamIndex != GRI.Teams[TeamIndex].TeamIndex)
					{
						RoundsLost += GRI.Teams[TeamIndex].Score;
					}
				}
				// Copy the stats from the PRI to the object
				Stats.UpdateFromPRI(PRI,RoundCount,RoundsWon,RoundsLost);
				// This will copy them into the online subsystem where they will be
				// sent via the network in EndOnlineGame()
				OnlineSub.StatsInterface.WriteOnlineStats(PRI.SessionName,PRI.UniqueId,Stats);
			}
		}
 	}
}

/**
 * Stores all of the stats from this match for view in the party lobby
 *
 * @param OnlineStatsWriteClass the stats class to write with
 * @param bUsesArbitration whether this is an arbitrated match or not
 */
simulated function StoreLastMatchData(class<OnlineStatsWrite> OnlineStatsWriteClass,bool bUsesArbitration)
{
	local GearRecentPlayersList PlayersList;
	local int Index;
	local GearPRI PRI;
	local UniqueNetId ZeroId;

	// Since the object is a singleton, it only needs to be done once per box
	if (IsPrimaryPlayer())
	{
		// Get the recent player data for the last match stats
		PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
		if (PlayersList != None)
		{
			// Update the final state for each player
			for (Index = 0; Index < WorldInfo.GRI.PRIArray.Length; Index++)
			{
				PRI = GearPRI(WorldInfo.GRI.PRIArray[Index]);
				if (PRI.UniqueId != ZeroId)
				{
					PlayersList.UpdatePlayer(PRI);
				}
				else
				{
					PlayersList.AddBot(PRI);
				}
			}
			// Now add the ratio columns
			PlayersList.LastMatchResults.OnReadComplete();
		}
	}
}

/**
 * Initializes the object that tracks post game stats so we can track people that drop
 *
 * @param OnlineStatsWriteClass the stats class to write with
 * @param bUsesArbitration whether this is an arbitrated match or not
 */
reliable client function ClientInitLastMatchResults(class<OnlineStatsWrite> OnlineStatsWriteClass,bool bUsesArbitration)
{
	local GearRecentPlayersList PlayersList;
	local int Index;
	local GearPRI PRI;
	local UniqueNetId ZeroId;
	local OnlineGameSettings GameSettings;

	// Since the object is a singleton, it only needs to be done once per box
	if (IsPrimaryPlayer() && OnlineSub != None && OnlineSub.GameInterface != None)
	{
		// Get the recent player data for the last match stats
		PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
		if (PlayersList != None)
		{
			// Get the game setting so we can determine whether to write stats for none, one or all players
			GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
			if (GameSettings != None)
			{
				PlayersList.InitRecentMatchResults(OnlineStatsWriteClass,WorldInfo.GetMapName(),bUsesArbitration);
				// Add each player so we can track those that drop
				for (Index = 0; Index < WorldInfo.GRI.PRIArray.Length; Index++)
				{
					PRI = GearPRI(WorldInfo.GRI.PRIArray[Index]);
					// Ignore bots, because they get handled at match end
					if (PRI.UniqueId != ZeroId)
					{
						PlayersList.AddPlayer(PRI);
					}
				}
			}
		}
	}
}

function PlayerOwnerDataStore GetCurrentPlayerData()
{
	return CurrentPlayerData;
}

final event NoticeEnemy(Pawn Noticer, Pawn Noticee)
{
	local GearGameSP_Base	SPGame;
	SPGame = GearGameSP_Base(WorldInfo.Game);
	if (SPGame != None)
	{
		SPGame.BattleMonitor.NotifyEnemyNoticed(Noticer, Noticee);
	}
}

/** Deal with list of nearby enemies, keep it up to date and all that. */
final native function MaintainEnemyList();

/** Returns which target we should use for friction (or adhesion), or None if none are appropriate. */
final native function Actor GetFrictionAdhesionTarget(float MaxDistance, optional bool bAdhesion);


/************************************************************************************
 * Firing Code
 ***********************************************************************************/

/**
 * Inherited, called from button bindings.
 */
exec function StartFire(optional byte FireModeNum)
{
	// @laurent -- Pawn is responsible for starting/stopping when bFire is set
	// do not call StartFire. See "Firing Code" in GearPawn.
}

/**
 * POIs need their icon durations zeroed out once the player hits the forcelook button
 */
function ClearPOIIconDurations()
{
	local GearPointOfInterest POI;
	foreach EnabledPointsOfInterest(POI)
	{
		POI.CurrIconDuration = 0.f;
	}
}

function bool FindVehicleToDrive()
{
	local Vehicle Vehic;
	Vehic = GetFoundVehicleToDrive();
	return (Vehic != None && Vehic.TryToDrive(Pawn));
}

/** Tries to find a vehicle to drive within a limited radius. Returns true if successful
 *  This is a modified version of the one in PlayerController.uc which returns which vehicle
 *  you can interact with.  I would have put this change in PlayerController but we are locked out of engine.
 */
final function Vehicle GetFoundVehicleToDrive()
{
	local Vehicle V, Best;
	local vector ViewDir, PawnLoc2D, VLoc2D;
	local float NewDot, BestDot;

	if (Vehicle(Pawn.Base) != None  && Vehicle(Pawn.Base).CanEnterVehicle(Pawn))
	{
		return None;
	}

	if (MyGearPawn.IsCarryingAHeavyWeapon() || MyGearPawn.IsCarryingShield())
	{
		return None;
	}

	// Pick best nearby vehicle
	PawnLoc2D = Pawn.Location;
	PawnLoc2D.Z = 0;
	ViewDir = vector(Pawn.Rotation);

	ForEach Pawn.OverlappingActors(class'Vehicle', V, Pawn.VehicleCheckRadius)
	{
		// Prefer vehicles that Pawn is facing
		VLoc2D = V.Location;
		Vloc2D.Z = 0;
		NewDot = Normal(VLoc2D-PawnLoc2D) Dot ViewDir;
		if ( (Best == None) || (NewDot > BestDot) )
		{
			// check that vehicle is visible
			if ( FastTrace(V.Location,Pawn.Location) )
			{
				Best = V;
				BestDot = NewDot;
			}
		}
	}

	if (Best != None && Best.CanEnterVehicle(Pawn))
	{
		return Best;
	}
	return None;
}

/**
 * Checks for a SeqEvt_Interaction that the player could interact with and sets InteractEvent to it if so
 * for replication and for use later in UpdateHUDActions()
 * server only
 */
event CheckForInteractionEvents()
{
	local Actor TouchActor;
	local int EvtIdx;

	InteractEvent = None;
	if (MyGearPawn != None)
	{
		// try to interact with anything the pawn is touching
		foreach Pawn.TouchingActors(class'Actor', TouchActor)
		{
			if (!TouchActor.bDeleteMe)
			{
				// iterate through each event looking for interaction
				for (EvtIdx = 0; EvtIdx < TouchActor.GeneratedEvents.Length; EvtIdx++)
				{
					InteractEvent = SeqEvt_Interaction(TouchActor.GeneratedEvents[EvtIdx]);
					if (InteractEvent != None && InteractEvent.CanDoInteraction(self) && InteractEvent.CheckActivate(TouchActor, Pawn, TRUE))
					{
						// we have an event!
						break;
					}
					// otherwise clear the event
					InteractEvent = None;
				}
				// if there is an event to interact with
				if (InteractEvent != None)
				{
					// stop looking
					break;
				}
			}
		}
	}
}

/** check for smoke grenade blocker in between the specified points */
final function bool IsSmokeGrenadeBlockingSight(vector StartTrace, vector EndTrace)
{
	local GearGRI GRI;
	local int i;
	local vector HitLocation, HitNormal;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None)
	{
		for (i = 0; i < GRI.SmokeVolumes.length; i++)
		{
			if (TraceComponent(HitLocation, HitNormal, GRI.SmokeVolumes[i].VisBlocker, EndTrace, StartTrace))
			{
				return true;
			}
		}
	}

	return false;
}

function bool CanPickupWeapons()
{
	return TRUE;
}


// will update the cando status of one special move per frame, and we use the cached results
final private native function UpdateCanDoSpecialMoveCache(bool bInCover);

// checks cover and global action special moves and returns TRUE if one was found
final private native function bool CheckForSpecialMove();

//`define UpdateHudActionsPerf
`if(`isdefined(UpdateHudActionsPerf))
`define	CClock(Time) Clock(`Time)
`define CUnClock(Time) UnClock(`Time)
`else
`define CUnClock(Time)
`define	CClock(Time)
`endif

/**
*  Check to see if there is any possibility for a POI alert
*  Code broken out from UpdateHUDActions()
*/
final function CheckEnabledPointsOfInterest(float DeltaTime)
{
	local int Idx;

	// check to see if there's a poi force look available
	if ((EnabledPointsOfInterest.Length > 0) && (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_Scripted))
	{
		for (Idx = 0; Idx < EnabledPointsOfInterest.Length; Idx++)
		{
			// make sure the poi didn't get deleted behind our back
			if ( EnabledPointsOfInterest[Idx] == None )
			{
				// remove from the list, back the index up, and continue through loop
				EnabledPointsOfInterest.Remove( Idx, 1 );
				POILookedAtList.Remove( Idx, 1 );
				Idx--;
				continue;
			}

			// if this poi has an icon duration...
			if ( EnabledPointsOfInterest[Idx].bEnabled && (EnabledPointsOfInterest[Idx].CurrIconDuration > 0.f) )
			{
				EnabledPointsOfInterest[Idx].CurrIconDuration -= DeltaTime;

				if ( EnabledPointsOfInterest[Idx].CurrIconDuration > 0.f )
				{
					if(MyGearPawn != none && MyGearPawn.IsLocallyControlled() && LastPoITime+PoIRepeatTime < WorldInfo.TimeSeconds)
					{
						PlaySound( PointOfInterestAdded, true );
						LastPoITime = WorldInfo.TimeSeconds;
					}
					if ( MyGearHud.SetActionInfo(AT_Scripted, ActionLookAt) )
					{
						CurrLookedAtPOI = EnabledPointsOfInterest[Idx];
					}
				}
				else
				{
					MyGearHud.ClearActionInfoByType(AT_Scripted);
				}
			}
		}
	}
}

/**
*  Check to see if there is any possibility for interaction with a pickup
*  Code broken out from UpdateHUDActions(), expects that the CanPickupWeapons() function returns true
*/
final function CheckForPickups()
{
	local ActionInfo Action;
	local Actor TouchActor;
	local GearDroppedPickup	DroppedPickup;
	local GearPickupFactory	FactoryPickup;

	if (MyGearPawn.IsDoingSpecialMove(SM_WeaponPickup))
	{
		MyGearHud.ClearActionInfoByType(AT_Pickup);
		return;
	}

	// look for any pickups
	foreach MyGearPawn.TouchingActors(class'Actor', TouchActor)
	{
		DroppedPickup = GearDroppedPickup(TouchActor);
		if (DroppedPickup != None)
		{
			if (DroppedPickup.GetPickupAction(Action,Pawn))
			{
				MyGearHud.SetActionInfo(AT_Pickup,Action);
				break;
			}
		}
		FactoryPickup = GearPickupFactory(TouchActor);
		if ( FactoryPickup != None )
		{
			/*
			if ( !MyGearPawn.bCanPickupFactoryWeapons && (GearWeaponPickupFactory(FactoryPickup) != None) )
			{
				FactoryPickup = None;
			}
			*/

			if (FactoryPickup != None && FactoryPickup.ClaimedBy != Pawn)
			{
				if (FactoryPickup.GetPickupAction(Action,Pawn))
				{
					MyGearHud.SetActionInfo(AT_Pickup,Action);
					break;
				}
			}
		}
	}
	// if we don't have an interaction then clear the last one
	if (TouchActor == None)
	{
		MyGearHud.ClearActionInfoByType(AT_Pickup);
	}
}

/**
*  Check to see if there is any possibility for a special move hud icon
*  Code broken out from UpdateHUDActions(), expects that the bShowSpecialMoveTips flag is true
*/
final function CheckSpecialMoves()
{
	local bool						bFoundSpecialMove;
	local class<GearSpecialMove>	MoveClass;
	local GearPawn					out_InteractionPawn;
	local ESpecialMove				out_SpecialMove;

	bFoundSpecialMove = CheckForSpecialMove();

	if( !bFoundSpecialMove )
	{
		// Check for Pawn to Pawn interactions
		if( MyGearPawn.CheckPawnToPawnInteractions(out_SpecialMove, out_InteractionPawn) )
		{
			MoveClass = MyGearPawn.SpecialMoveClasses[out_SpecialMove];
			if (MoveClass.default.Action.ActionName == 'GrabMeatShield')
			{
				// Trigger the meatshield delegates
				TriggerGearEventDelegates( eGED_MeatShield );

				// Check for TG tutorial
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_Exe, eGEARTRAIN_Execution);
			}

			// then notify the hud and stop looking
			MyGearHud.SetActionInfo(AT_SpecialMove, MoveClass.default.Action, MyGearPawn.bIsMirrored);
			bFoundSpecialMove = TRUE;
		}
	}

	// if we don't have an interaction then clear the last one
	if (!bFoundSpecialMove)
	{
		// if not doing a special move
		if (!IsDoingASpecialMove())
		{
			// clear the default
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
		// otherwise check for the special case interactions
		else if (MyGearHud.ActiveAction.ActionName == 'PushObject' && !IsDoingSpecialMove(SM_PushObject))
		{
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
		else if (MyGearHud.ActiveAction.ActionName == 'SawDuel' && !IsDoingSpecialMove(SM_ChainsawDuel_Follower) && !IsDoingSpecialMove(SM_ChainsawDuel_Leader) && !MyGearPawn.bInDuelingMiniGame)
		{
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
	}
}

/**
 *  Check to see if there is any possibility for interaction with a vehicle
 *  Code broken out from UpdateHUDActions(), expects that the bCheckForVehicles flag is true
 */
final function CheckForVehicles()
{
	local Vehicle Vehic;
	local Turret TurretVehicle;

	// try to find a vehicle
	Vehic = GetFoundVehicleToDrive();
	// see if we found a vehicle
	if ( Vehic != None )
	{
		// now that we have a vehicle, see if it's a turret
		TurretVehicle = Turret(Vehic);
		if ( TurretVehicle != None )
		{
			MyGearHud.SetActionInfo(AT_Vehicle,TurretVehicle.InteractAction);
		}
	}
	// if we don't have a vehicle then clear the last one
	else
	{
		MyGearHud.ClearActionInfoByType(AT_Vehicle);
	}
}

final function AssignHitTargets(const vector StartTrace, const vector EndTrace, const vector TraceDir, out string HitPawnName)
{
	local vector				HitLoc, HitNormal;
	local GearPawn				HitWP;
	local Vehicle_Reaver_Base	HitReaver;
	local GearProj_Grenade		HitGrenade;
	local ImpactInfo			Impact;
	local Actor					HitActor;
	local int i;

	if (!IsSmokeGrenadeBlockingSight(StartTrace, EndTrace))
	{
		// prevent meat shield from being hit by aiming trace
		if (MyGearPawn != None && MyGearPawn.IsAKidnapper() && MyGearPawn.InteractionPawn != None)
		{
			MyGearPawn.InteractionPawn.Mesh.SetTraceBlocking(false, false);
		}

		// do the trace to see what's in our sights
		CalcWeaponFire( StartTrace, EndTrace, HitLoc, HitNormal, HitActor, Impact );

		HitWP = GearPawn( HitActor );
		HitReaver = Vehicle_Reaver_Base(HitActor);

		// restore meat shield flags
		if (MyGearPawn != None && MyGearPawn.IsAKidnapper() && MyGearPawn.InteractionPawn != None)
		{
			MyGearPawn.InteractionPawn.Mesh.SetTraceBlocking(true, false);
		}

		// redirect to kidnapper when hitting hostage
		if (HitWP != None && HitWP.IsAHostage())
		{
			HitWP = HitWP.InteractionPawn;
		}

		//Record the name of the hit target
		if (HitWP != None && HitWP.PlayerReplicationInfo != None)
		{
			HitPawnName = HitWP.PlayerReplicationInfo.PlayerName;
		}

		// This code section moved inside the above IsSmokeGrenadeBlockingSight check because HitWP can only possibly be assigned inside it

		// Allow HUD to handle displaying UI for when a pawn is targetted
		if ( WorldInfo.GRI != None && bIsTargeting )
		{
			if ( WorldInfo.GRI.IsMultiplayerGame() && (HitWP != None) && ((HitWP.Health >= 0) || HitWP.IsDBNO()) && !HitWP.bTearOff )
			{
				MyGearHud.HandlePawnTargetted( HitWP );
			}
		}

		//debug log for what we hit to track down why its not turning
		`log( GetFuncName()@"Hit..."@HitActor, bDebug );

		// we only want enemy targets for this feature (pawn or vehicle reavers)
		if( (HitWP != None && HitWP.ShouldTurnCursorRedFor(self)) ||
			(HitReaver != None && HitReaver.GetTeamNum() != 254 && !WorldInfo.GRI.OnSameTeam(self, HitReaver)) )
		{
			if( ( (HitWP != None) &&
				( (HitWP.Health >= 0 || HitWP.IsDBNO()) &&
				(!HitWP.bTearOff) &&
				(HitWP.EquippedShield == None || Impact.HitInfo.HitComponent != HitWP.EquippedShield.Mesh)
				)
				) ||
				( (HitReaver != None) &&
				( (HitReaver.Health >= 0) &&
				(!HitReaver.bTearOff)
				)
				) )
			{
				if ( WorldInfo.GRI.IsMultiplayerGame() && !WorldInfo.GRI.IsCoopMultiplayerGame() )
				{
					// the reavers don't get a MultiplayerCoverDamageAdjust
					if( HitWP != none )
					{
						MyGearHud.bUnfriendlySpotted = (HitWP.MultiplayerCoverDamageAdjust(TraceDir, Pawn, HitLoc, 100) != 0);
					}
				}
				else
				{
					MyGearHud.bUnfriendlySpotted = TRUE;
				}
			}

			// clear so that we don't draw the name
			HitPawnName = "";
		}
		else
		{
			// display red crosshair if over an enemy planted grenade
			HitGrenade = GearProj_Grenade(HitActor);
			if ( HitGrenade != None && HitGrenade.MeleeVictim != None && !HitGrenade.MeleeVictim.IsA('Pawn') &&
				!WorldInfo.GRI.OnSameTeam(self, HitGrenade) )
			{
				MyGearHud.bUnfriendlySpotted = TRUE;
			}
			// Hit skel mesh actor which can do thi
			else
			if ( (SkelMeshActor_Unfriendly(HitActor) != None) && SkelMeshActor_Unfriendly(HitActor).HitAreaIsUnfriendly(Impact.HitInfo) )
			{
				MyGearHud.bUnfriendlySpotted = TRUE;
			}
			// hit hydra boss?
			else
			if ( (Hydra_Base(HitActor) != None) && Hydra_Base(HitActor).HitWillCauseDamage(Impact.HitInfo) )
			{
				MyGearHud.bUnfriendlySpotted = TRUE;
			}
			else
			// Hit crowd actor
			if( FlockTestLocust(HitActor) != None )
			{
				MyGearHud.bUnfriendlySpotted = TRUE;
			}
			else
			if( GearPawn_LocustLeviathanBase(HitActor) != None )
			{
				MyGearHud.bUnfriendlySpotted = GearPawn_LocustLeviathanBase(HitActor).HitAttackingTentacle( Impact.HitInfo );
				HitPawnName = "";
			}
			else if(HitActor != None)
			{
				// Actor is "unfriendly"
				for (i = 0; i < ArrayCount(UnfriendlyActorList); i++)
				{
					if (UnfriendlyActorList[i] == HitActor)
					{
						MyGearHud.bUnfriendlySpotted = TRUE;
						break;
					}
				}
			}
		}
	}
}

simulated function OnMarkUnfriendlyActor( SeqAct_MarkUnfriendlyActor Action )
{
	local int Idx, LocalIdx;

	if( Action.bOverwriteExisting )
	{
		for (Idx = 0; Idx < ArrayCount(UnfriendlyActorList); Idx++)
		{
			UnfriendlyActorList[Idx] = None;
		}
	}

	for( Idx = 0; Idx < Action.UnfriendlyList.Length; Idx++ )
	{
		if (Action.UnfriendlyList[Idx] != None)
		{
			if (LocalIdx == ArrayCount(UnfriendlyActorList))
			{
				`Warn("Ran out of free UnfriendlyActorList entries"@Action);
`if(`notdefined(FINAL_RELEASE))
				ClientMessage( "Ran out of free UnfriendlyActorList entries"@Action );
`endif
			}
			else
			{
				for (LocalIdx = LocalIdx; LocalIdx < ArrayCount(UnfriendlyActorList); LocalIdx++)
				{
					if (UnfriendlyActorList[LocalIdx] == None || UnfriendlyActorList[LocalIdx].IsPendingKill())
					{
						UnfriendlyActorList[LocalIdx] = Action.UnfriendlyList[Idx];
						LocalIdx++;
						break;
					}
				}
			}
		}
	}
}

/**
* Checks various special moves to show the tip icon if necessary.
*/
final function UpdateHUDActions(float DeltaTime)
{
	local vector					StartTrace, EndTrace, TraceDir;
	local rotator					CamRot;
	local ActionInfo				Action;
	local bool						bShowingValve;
	local Vehicle					VehicleTest;
	local Trigger_Engage			Engage;
	local string					HitPawnName;

`if(`isdefined(UpdateHudActionsPerf))
	local int	Idx;
	local float Overall,SumTotal;
	local float Time[20];
`endif

	if ( MyGearHUD == None )
	{
		return;
	}

	if ( CurrentPPType == eGPP_GameOver )
	{
		MyGearHud.ClearActiveActionInfo();
		return;
	}

	// If the player is DBNO
	if ( (MyGearHud.ActiveAction.ActionType == AT_StayingAlive) || (MyGearHud.ActiveAction.ActionType == AT_SuicideBomb) )
	{
		return;
	}

	`CClock(Overall);
	VehicleTest = Vehicle(Pawn);

	// test the crosshair
	MyGearHud.bUnfriendlySpotted = false;

	`CClock(Time[0]);
	// start the trace from the camera location
	GetPlayerViewPoint(StartTrace, CamRot);
	TraceDir = vector(CamRot);
	`CUnClock(Time[0]);

	`CClock(Time[1]);
	// limit the trace for the flamethrower so players can judge the damage effectiveness better
	if(Pawn != None && GearWeapon(Pawn.Weapon) != None && GearWeapon(Pawn.Weapon).bOverrideDistanceForEnemyTrace)
	{
		EndTrace = StartTrace + (TraceDir * GearWeapon(Pawn.Weapon).GetWeaponDistanceForEnemyTrace());
	}
	else
	{
		EndTrace = StartTrace + (TraceDir * DistanceForEnemyTrace);
	}
	`CUnClock(Time[1]);

	`CClock(Time[2]);
	//Figures out who/what we are hitting (HitPawn/Reaver)
	if ((MyGearPawn != None && MyGearPawn.bIsTargeting) || (VehicleTest != None))
	{
		AssignHitTargets(StartTrace, EndTrace, TraceDir, HitPawnName);
	}

	`CUnClock(Time[2]);

	`CClock(Time[5]);
	// if we are showing a scripted icon and the poi for it was disabled, turn off the icon
	if ( MyGearHud.ActiveAction.bActive && MyGearHud.ActiveAction.ActionType == AT_Scripted && CurrLookedAtPOI != None &&
		(!CurrLookedAtPOI.bEnabled || CurrLookedAtPOI.CurrIconDuration <= 0.0) )
	{
		MyGearHud.ClearActionInfoByType(AT_Scripted);
	}

	// if we have a pawn, hud, and aren't looking at a POI, nor doing a special move already
	if ((MyGearPawn != None || VehicleTest != None)
		&& !bLookingAtPointOfInterest
		&& (MyGearPawn == None || IsInState('Engaging') || (!MyGearPawn.IsDoingASpecialMove() || MyGearPawn.IsDoingSpecialMove(SM_RoadieRun) || MyGearPawn.IsDoingSpecialMove(SM_ChainsawHold) || MyGearPawn.IsDoingMove2IdleTransition() || MyGearPawn.IsAKidnapper() || MyGearPawn.IsDoingSpecialMove(SM_UsingCommLink) || MyGearPawn.IsDoingSpecialMove(SM_DeployShield)))
		)
	{
		CheckEnabledPointsOfInterest(DeltaTime);
		`CUnClock(Time[5]);

		`CClock(Time[6]);
		// check to see if we're looking at a player
		if (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_Player)
		{
			if( bIsTargeting )
			{
				// if we're looking at a friendly player
				if (HitPawnName != "")
				{
					Action.ToolTipText = HitPawnName;
					MyGearHud.SetActionInfo(AT_Player, Action);
				}
				else
				{
					// otherwise clear any players
					MyGearHud.ClearActionInfoByType(AT_Player);
				}
			}
			else
			{
				// must clear action info if targeting was turned off
				MyGearHud.ClearActionInfoByType(AT_Player);
			}
		}

		// See if we need to show the engaging icon
		if ( IsInState('Engaging') )
		{
			if (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_StateAction)
			{
				Engage = MyGearPawn.EngageTrigger;
				// make sure we can still turn it and that we're in a position to turn it
				if ( (Engage != None) && (Engage.LoopCounter > 0) && IsDoingSpecialMove(SM_Engage_Idle) )
				{
					MyGearHud.SetActionInfo(AT_StateAction, ActionValveTurn);
					bShowingValve = true;
				}
			}
		}
		if ( (MyGearHud.ActiveAction.ActionType == AT_StateAction) && !bShowingValve )
		{
			// clear the action
			MyGearHud.ClearActionInfoByType(AT_StateAction);
		}
		`CUnClock(Time[6]);

		// At this point, if we're manning a vehicle, we don't want any icons below this
		// priority to show up.
		if ( VehicleTest == None )
		{
			`CClock(Time[7]);
			// look for vehicles
			if ( bCheckVehicles && (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_Vehicle) )
			{
				CheckForVehicles();
			}
			`CUnClock(Time[7]);

			// if not targeting and not conversing
			if (!bIsTargeting && (MyGearPawn == None || !MyGearPawn.bIsConversing))
			{
				`CClock(Time[8]);
				// look for any possible trigger interactions
				// we don't want trigger icons if we're in the Engaging state
				if ( !IsInState('Engaging') && (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_Trigger))
				{
					if (InteractEvent != None && !MyGearPawn.IsInCover())
					{
						MyGearHud.SetActionInfo(AT_Trigger, InteractEvent.InteractAction);
					}
					// if we don't have an interaction then clear the last one
					else
					{
						MyGearHud.ClearActionInfoByType(AT_Trigger);
					}
				}
				`CUnClock(Time[8]);

				// look for any pickups
				`CClock(Time[9]);
				if( CanPickupWeapons() && MyGearPawn != None && (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_Pickup) )
				{
					CheckForPickups();
				}
				`CUnClock(Time[9]);

				// look for any special moves
				`CClock(Time[10]);
				if ( bShowSpecialMoveTips && (!MyGearHud.ActiveAction.bActive || MyGearHud.ActiveAction.ActionType >= AT_SpecialMove) )
				{
					CheckSpecialMoves();
				}
				`CUnClock(Time[10]);
			}
			else
			{
				`CClock(Time[11]);
				if ( (MyGearHud != None) && MyGearHUD.ActiveAction.bActive )
				{
					// clear the types not allowed when targeting
					MyGearHud.ClearActionInfoByType(AT_Trigger);
					MyGearHud.ClearActionInfoByType(AT_Pickup);
					MyGearHud.ClearActionInfoByType(AT_SpecialMove);
				}
				`CUnClock(Time[11]);
			}
		}
		else
		{
			`CClock(Time[13]);
			if ( (MyGearHud != None) && MyGearHUD.ActiveAction.bActive )
			{
				MyGearHud.ClearActionInfoByType(AT_Vehicle);
				MyGearHud.ClearActionInfoByType(AT_Trigger);
				MyGearHud.ClearActionInfoByType(AT_Pickup);
				MyGearHud.ClearActionInfoByType(AT_SpecialMove);
			}
			`CUnClock(Time[13]);
		}
	}
	else if ( (MyGearHud != None) && MyGearHUD.ActiveAction.bActive )
	{
		// clear all types
		`CClock(Time[14]);
		MyGearHud.ClearActionInfoByType(AT_Player);
		MyGearHud.ClearActionInfoByType(AT_Trigger);
		MyGearHud.ClearActionInfoByType(AT_Pickup);
		if (MyGearHud.ActiveAction.ActionName == 'PushObject' && !IsDoingSpecialMove(SM_PushObject))
		{
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
		else if (MyGearHud.ActiveAction.ActionName == 'SawDuel' && !IsDoingSpecialMove(SM_ChainsawDuel_Follower) && !IsDoingSpecialMove(SM_ChainsawDuel_Leader) && !MyGearPawn.bInDuelingMiniGame)
		{
			MyGearHud.ClearActionInfoByType(AT_SpecialMove);
		}
		`CClock(Time[14]);
	}
	`CUnClock(Overall);

`if(`isdefined(UpdateHudActionsPerf))
	GearCheatManager(CheatManager).ForceLog(GetFuncName()@"STATS _--------------------------------------------------_");
	for(Idx=0;Idx<20;Idx++)
	{
		if (Time[Idx] > 0)
		{
			GearCheatManager(CheatManager).ForceLog(GetFuncName()@" -- STAT ["$Idx$"]"@Time[Idx]@"("@(Time[Idx]/Overall)*100.f@"%)");
			SumTotal += Time[Idx];
		}
	}
	GearCheatManager(CheatManager).ForceLog(GetFuncName()@"TOTALS --------------- Overall:"@Overall@"Error:"@Abs(Overall-SumTotal));
`endif
}

/**
 * Return true if player is in cover.
 */
simulated function bool IsInCoverState()
{
	return FALSE;
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
simulated final function bool IsLeaning(ECoverAction PawnCA)
{
	return (PawnCA == CA_LeanRight || PawnCA == CA_LeanLeft || PawnCA == CA_PopUp);
}

simulated function CalcWeaponFire( Vector StartTrace, Vector EndTrace, out Vector out_HitLocation, out Vector out_HitNormal, out Actor out_HitActor, out ImpactInfo out_HitImpact )
{
	if( Pawn != None && Pawn.Weapon != None)
	{
		out_HitImpact = Pawn.Weapon.CalcWeaponFire( StartTrace, EndTrace );
		out_HitLocation = out_HitImpact.HitLocation;
		out_HitNormal	= out_HitImpact.HitNormal;
		out_HitActor	= out_HitImpact.HitActor;
		if( out_HitActor == None )
		{
			out_HitLocation = EndTrace;
		}
	}
	else
	{
		// use old-school trace. this might hit triggers and such.
		out_HitActor = Trace(out_HitLocation, out_HitNormal, EndTrace, StartTrace, TRUE,, out_HitImpact.HitInfo, TRACEFLAG_Bullet);
	}
}

/**
 * Test code for bullet-attractor method of autoaim.  Experimental
 * for now, we'll see how it feels to the masses.
 */
function Rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
{
	local Actor			BestTarget;
	local GearPawn		TargetWP;
	local GearWeapon	WW;
	local vector		AttractPt, BaseAimDir, ToAttractorNorm, BestToAttrNorm;
	local Rotator		BaseAimRot;
	local float			Alpha, BestAttrAlpha, AimAssistScale;
	local int			AttractorIdx, NumAttractors;
	local AimAttractor	Attr;
	local vector		UnusedVec1, UnusedVec2;
	local GearAimAssistActor TargetGAAA;

	// Get default rot so we can return it or base
	// aim assistance calcs off of it, as necessary
	BaseAimRot = (Pawn != None) ? Pawn.GetBaseAimRotation() : Rotation;

	WW = GearWeapon(W);
	if (WW == None)
	{
		return BaseAimRot;
	}

	BestTarget = GetFrictionAdhesionTarget(WW.WeaponRange);
	ShotTarget = Pawn(BestTarget);

	// this is a bit kludgey, but for some weapons (hod), we want steady aiming
	// regardless of camera shake and stuff like that.
	if (WW.bUsePreModCameraRotForAiming)
	{
		BaseAimRot = GearPlayerCamera(PlayerCamera).GameplayCam.LastPreModifierCameraRot;
	}

	if (!WW.bInstantHit)
	{
		// not firing a Gearweapon, what's going on?
		return BaseAimRot;
	}

	// early out check, so we can maybe skip any traces or other geom queries.
	AimAssistScale = bIsTargeting ? WW.AimAssistScaleWhileTargeting : WW.AimAssistScale;
	if( AimAssistScale <= 0.f )
	{
		return BaseAimRot;
	}

	BaseAimDir = vector(BaseAimRot);

	if ( BestTarget == None )
	{
		// no one around worth autoaiming at
		return BaseAimRot;
	}

	TargetWP = GearPawn(BestTarget);
	TargetGAAA = GearAimAssistActor(BestTarget);

	// If no target OR is aiming and close to the target and not shotgun OR target is protected by cover
	if( ((TargetWP == None) && (TargetGAAA == None))
		|| ( VSize(BestTarget.Location - Pawn.Location) < 128 )
		|| ( bIsTargeting && ( VSize(BestTarget.Location - Pawn.Location) < 512 ) && ( GearWeap_ShotGun(WW) == none ) )
		|| ((TargetWP != None) && TargetWP.IsInCover() && TargetWP.IsProtectedByCover(Vector(BaseAimRot)))
		)
	{
		// No aim adjust
		return BaseAimRot;
	}

	// aim at target - help with leading also
	// @todo (if this passes poc stage)
	// - handle non-instant-hit weapons
	// - it's possible in multiplayer to have different autoaim params,
	//    it would be good to unify the values so this isn't a subtle bug later

	BaseAimDir = vector(BaseAimRot);

	if (TargetWP != None)
	{
		// don't do aim attraction if base aim will hit target (it's presumably unneeded
		if (!TraceComponent(UnusedVec1, UnusedVec2, TargetWP.Mesh, (StartFireLoc + BaseAimDir*50000), StartFireLoc))
		{
			NumAttractors = TargetWP.AimAttractors.Length;

			for (AttractorIdx=0; AttractorIdx<NumAttractors; ++AttractorIdx)
			{
				Attr = TargetWP.AimAttractors[AttractorIdx];

				// get world pos of attractor
				AttractPt = TargetWP.Mesh.GetBoneLocation(Attr.BoneName);

				EvalAimAttractor(Attr, Alpha, ToAttractorNorm, StartFireLoc, BaseAimDir, AttractPt, AimAssistScale);

				// decide which is the best attractor and use it
				if ( (Alpha > BestAttrAlpha) && FastTrace(AttractPt, StartFireLoc) )
				{
					//`log("Attr " @ AttractorIdx @ Alpha@BestTarget@AttractDot@DotExtentOuter@DotExtentInner@AttractPt);
					BestAttrAlpha = Alpha;
					BestToAttrNorm = ToAttractorNorm;
				}
			}
		}
	}
	else if (TargetGAAA != None)
	{
		TargetGAAA.GetAimAttractorRadii(Attr.InnerRadius, Attr.OuterRadius);
		EvalAimAttractor(Attr, BestAttrAlpha, BestToAttrNorm, StartFireLoc, BaseAimDir, TargetGAAA.Location, AimAssistScale);
	}

	//`log("AimAttracted to "@TargetWP@BestAttrAlpha);
	return RLerp(BaseAimRot, rotator(BestToAttrNorm), BestAttrAlpha, TRUE);
}

simulated protected final function EvalAimAttractor(const out AimAttractor Attr, out float AttractorAlpha, out vector ToAttractorNorm, vector StartFireLoc, vector BaseAimDir, vector AttractPt, float AimAssistScale)
{
	local float ToAttractorDist, DotExtentInner, DotExtentOuter;
	local float AttractDot, ToAttractorDistSq;

	// init to no attraction
	AttractorAlpha = 0.f;

	// vector from shot origin to attractor
	ToAttractorNorm = AttractPt - StartFireLoc;
	ToAttractorDist = VSize(ToAttractorNorm);
	if (ToAttractorDist >= 0.f)
	{
		ToAttractorDistSq = Square(ToAttractorDist);
		ToAttractorNorm /= ToAttractorDist;

		// actual dot of my aim
		AttractDot = BaseAimDir dot ToAttractorNorm;

		// in front?
		if (AttractDot > 0.0f)
		{
			// solving for cos(angle)
			DotExtentOuter = ToAttractorDist / Sqrt(Square(Attr.OuterRadius * AimAssistScale) + ToAttractorDistSq);
			DotExtentInner = ToAttractorDist / Sqrt(Square(Attr.InnerRadius * AimAssistScale) + ToAttractorDistSq);

			// how much attraction do we get?
			AttractorAlpha = (AttractDot - DotExtentOuter) / (DotExtentInner - DotExtentOuter);
			AttractorAlpha = FClamp(AttractorAlpha, 0.f, 1.f);
		}
	}
}


/************************************************************************************
 * Assess Mode
 ***********************************************************************************/

/**
 * Enables the assess mode, so as to notify the HUD that it should
 * start rendering all possible interactions.
 */
function EnableAssessMode()
{
	`assert(!bAssessMode);
	bAssessMode = TRUE;
	// notify hud
	if (MyGearHud != None)
	{
		MyGearHud.EnableAssessMode();
	}
	// play an opening sound
	PlaySound(SoundCue'Interface_Audio.Interface.TaccomOpen01Cue', true);
}

/**
 * Disables the assess mode.
 */
function DisableAssessMode()
{
	if (bAssessMode)
	{
		bAssessMode = FALSE;
		// notify hud
		if (MyGearHud != None)
		{
			MyGearHud.DisableAssessMode();
		}
		// play closing sound
		PlaySound(SoundCue'Interface_Audio.Interface.TaccomClose01Cue', true);
	}
}

/**
 * Returns our GearPRI.  Wow.
 */
final function GearPRI GetPRI()
{
	return GearPRI(PlayerReplicationInfo);
}

/**
 * Disable any current input and set the flag that will prevent future input
 * @PARAM bIsPOI - whether we want to force the POI button to call it's release function,
 *		since the POI button CAN be one of the buttons that reenables the input we sometimes want
 *		to force it and sometimes not.
 */
function DisableInput(bool bDisableButtons, bool bDisableMovement, bool bDisableLook, bool bIsPOI=false)
{
	if (bDisableButtons)
	{
		// If the input was previously enabled, disable it.
		if ( InputIsDisabledCount == 0 )
		{
			GearPlayerInput_Base(PlayerInput).DisableButtonInput(bIsPOI);
		}
		// increment the disabled count
		InputIsDisabledCount++;
	}
	// disable movement input
	if (bDisableMovement)
	{
		IgnoreMoveInput(TRUE);
	}
	// disable camera input
	if (bDisableLook)
	{
		IgnoreLookInput(TRUE);
	}
}

/** Enable the input */
function EnableInput(bool bEnableButtons, bool bEnableMovement, bool bEnableLook)
{
	if (bEnableButtons && InputIsDisabledCount > 0)
	{
		// decrement the disabled counter
		InputIsDisabledCount--;
		if (InputIsDisabledCount == 0)
		{
			GearPlayerInput_Base(PlayerInput).EnableButtonInput();
		}
	}
	// free movement input
	if (bEnableMovement)
	{
		IgnoreMoveInput(FALSE);
	}
	// free camera input
	if (bEnableLook)
	{
		IgnoreLookInput(FALSE);
	}
}

/** This will apply the values from the SensistivityScaling which are found in the .ini **/
function ApplySensitivityScaling()
{
	local float MultiPlier;

	MultiPlier = GetSensitivityMultiplier();

	PlayerInput.aTurn *= MultiPlier;
	PlayerInput.aLookUp	*= MultiPlier;
}

/** notification that client pressed the button to keep pushing an object */
reliable server function ServerKeepPushing()
{
	if (MyGearPawn != None)
	{
		// if we're already pushing an object
		if (MyGearPawn.IsDoingSpecialMove(SM_PushObject))
		{
			// then keep pushing
			GSM_PushObject(MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove]).ButtonPressed();
		}
		else if (MyGearPawn.CanDoSpecialMove(SM_PushObject))
		{
			// otherwise start a new push
			ServerDictateSpecialMove(SM_PushObject);
		}
		else
		{
			// can't do anything, force client to stop pushing
			ServerDictateSpecialMove(SM_None);
		}
	}
}

/** Returns TRUE if the button is currently held. */
final function bool IsButtonActive(EGameButtons Button, optional bool bRaw)
{
	if ( PlayerInput == None )
	{
		`warn("PlayerInput is NULL - should NOT be called on nonlocal players!");
		ScriptTrace();
		return false;
	}

	return GearPlayerInput_Base(PlayerInput).IsButtonActive( Button, bRaw );
}

/** Forces a button to release, optionally skipping the handler */
final function ForceButtonRelease(EGameButtons Button, optional bool bSilent)
{
	if (GearPlayerInput_Base(PlayerInput) != None)
	{
		GearPlayerInput_Base(PlayerInput).ForceButtonRelease( Button, bSilent );
	}
}

simulated final function bool CanSwitchWeapons()
{
	local GearWeapon Weap;
	if( MyGearPawn != None )
	{
		if (MyGearPawn.IsDBNO() || GearPawn_LocustBrumakBase(MyGearPawn) != None)
		{
			return FALSE;
		}

		// No changing weapons when holding crate.
		if( MyGearPawn.CarriedCrate != None )
		{
			return FALSE;
		}

		// no weapon switch if special move disallows it
		if (MyGearPawn.IsDoingASpecialMove() && (!MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].bCanFireWeapon || MyGearPawn.SpecialMove == SM_UnMountMortar || MyGearPawn.SpecialMove == SM_UnMountMinigun))
		{
			return FALSE;
		}
		Weap = MyGearPawn.MyGearWeapon;
		if (Weap != None &&
			Weap.CurrentFireMode == class'GearWeapon'.const.FIREMODE_FAILEDACTIVERELOAD || Weap.ShouldPreventWeaponSwitch() )
		{
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Overridden to remove the UT pickup message.
 */
function HandlePickup(Inventory Inv)
{
}

/** Switches the player's weapon based on the slot passed in */
final function ProcessWeaponSwitch( GearPawn.EAttachSlot Slot )
{
	local GearInventoryManager InvManager;
	local Inventory InvInSlot;

	// if we can switch weapons at the moment
	if (MyGearPawn != None && CanSwitchWeapons())
	{
		// Bring up the selector UI
		InvManager = GearInventoryManager(Pawn.InvManager);
		if (InvManager != None)
		{
			InvManager.bShowWeaponSlots = TRUE;
			InvManager.WeapSelHUDTimeCount = 0;
			InvInSlot = InvManager.GetInventoryInSlot(Slot);

			if (InvInSlot != None)
			{
				InvManager.SetHighlightedWeaponSlot(Slot);
			}
			else
			{
				InvManager.SetHighlightedWeaponSlot(GearWeapon(MyGearPawn.Weapon).CharacterSlot);
			}
			if (InvInSlot != Pawn.Weapon)
			{
				// Drop a meatshield if we're a kidnapper
				if (MyGearPawn.IsAKidnapper())
				{
					EndSpecialMove();
				}
				InvManager.SetWeaponFromSlot(Slot);
			}
		}
	}
}

/** Closes the weapon switch selector UI */
final function StopShowingWeaponSelector()
{
	local GearInventoryManager InvManager;

	InvManager = GearInventoryManager(Pawn.InvManager);
	if (InvManager != None)
	{
		InvManager.bShowWeaponSlots = FALSE;
		InvManager.WeapSelHUDTimeCountSpeedMult = 1.0f;
	}
}

exec function PlayVehicleHorn()
{
	local GearVehicle V;

	V = GearVehicle(Pawn);
	if ( (V != None) && (V.Health > 0) && (WorldInfo.TimeSeconds - LastHornPlayedTime > 3.0)  )
	{
		ServerPlayVehicleHorn();
		LastHornPlayedTime = WorldInfo.TimeSeconds;
	}
}

unreliable server function ServerPlayVehicleHorn()
{
	local GearVehicle V;

	V = GearVehicle(Pawn);
	if ( (V != None) && (V.Health > 0) )
	{
		V.PlayHorn();
	}
}

/** Called by GearPlayerInput to switch weapons */
final function PressedDPad(EGameButtons Button)
{
	local GearPawn.EAttachSlot Slot;

	// Figure out which slot was pressed
	switch ( Button )
	{
		case GB_DPad_Up:	Slot = EASlot_Belt;				break;
		case GB_DPad_Left:	Slot = EASlot_LeftShoulder;		break;
		case GB_DPad_Right:	Slot = EASlot_RightShoulder;	break;
		case GB_DPad_Down:	Slot = EASlot_Holster;			break;
		default:			Slot = EASlot_None;				break;
	}

	ProcessWeaponSwitch( Slot );
}

/** Released the Dpad - called by GearPlayerInput */
final function ReleasedDPad(EGameButtons Button)
{
	local bool bAnyDPadPressed;

	if (MyGearPawn != None)
	{
		bAnyDPadPressed = IsButtonActive(GB_DPad_Up) || IsButtonActive(GB_DPad_Left) || IsButtonActive(GB_DPad_Down) || IsButtonActive(GB_DPad_Right);

		// if not pressing any direction then turn off the selector
		if (!bAnyDPadPressed)
		{
			StopShowingWeaponSelector();
		}
	}
}

final function bool CheckWeaponSwitchByWheel()
{
	local GearInventoryManager InvManager;
	local GearPawn.EAttachSlot Slot;
	local bool bResult;

	if( (MyGearPawn != None) && !MyGearPawn.bIsConversing )
	{
		InvManager = GearInventoryManager(Pawn.InvManager);
		if( InvManager != None )
		{
			if( CanSwitchWeapons() )
			{
				Slot = InvManager.HighlightedWeaponSlot;
				if( (Slot != EASlot_None) && (InvManager.GetInventoryInSlot(Slot) != None) )
				{
					// Only switch weapons if it is actually a change in slots
					if( Slot != MyGearPawn.MyGearWeapon.CharacterSlot )
					{
						InvManager.WeapSelHUDTimeCountSpeedMult = 3.0f;
						ClientPlaySound( SoundCue'Interface_Audio.Interface.WeaponSelectConfirmCue' );
						InvManager.SetWeaponFromSlot( Slot );
						bResult = TRUE;
					}
				}
			}

			// Turn off selector
			InvManager.bShowWeaponSlots = FALSE;
		}
	}

	// Clear timer in case function was called before timer expired
	ClearTimer( 'CheckWeaponSwitchByWheel' );

	return bResult;
}

/** Switch weapon based on direction */
final function SwitchWeapon( bool bForward )
{
	local GearPawn.EAttachSlot Slot;
	local GearInventoryManager InvManager;
	local bool bWeaponOverride;

	// Check for weapon override
	if( MyGearPawn != None && MyGearPawn.MyGearWeapon != None )
	{
		bWeaponOverride = bForward ? MyGearPawn.MyGearWeapon.DoOverrideNextWeapon() : MyGearPawn.MyGearWeapon.DoOverridePrevWeapon();
	}

	if ( (MyGearPawn != None) && !MyGearPawn.bIsConversing && !bIsTargeting && !bWeaponOverride )
	{
		InvManager = GearInventoryManager(Pawn.InvManager);
		if( InvManager != None )
		{
			// init slot
			Slot = EASlot_None;

			if( TimeSince( LastCycleWeaponTime ) > 0.f )
			{
				// Show slots
				InvManager.bShowWeaponSlots = TRUE;

				// Otherwise, grab previous weapon slot
				Slot = bForward ?  InvManager.GetNextSlot(InvManager.HighlightedWeaponSlot) : InvManager.GetPrevSlot(InvManager.HighlightedWeaponSlot);
			}
			LastCycleWeaponTime = WorldInfo.TimeSeconds;

			if( Slot != EASlot_None )
			{
				// Highlight the appropriate slot
				InvManager.SetHighlightedWeaponSlot( Slot );
				InvManager.WeapSelHUDTimeCount = 0;

				// Set timer to actually switch weapons
				SetTimer( 0.75f, FALSE, nameof(CheckWeaponSwitchByWheel) );
			}
		}
	}
	else if( bMouseWheelZoom && bIsTargeting )
	{
		// Otherwise, toggle zoom when targeting
		if( MyGearPawn.MyGearWeapon != None )
		{
			MyGearPawn.MyGearWeapon.HandleButtonPress( bForward ? 'ZoomOut' : 'ZoomIn' );
		}
	}
}

/** Overridden to support mouse wheel weapon switching and target zooming */
exec function PrevWeapon()
{
	// If weapons are all loaded (debug) we will use the old weapon switch method
	if ( bWeaponsLoaded )
	{
		Super.PrevWeapon();
	}
	// Else use the new method which will clamp the weapons to the 4 slots
	else
	{
		SwitchWeapon( FALSE );
	}
}

/** Overridden to support mouse wheel weapon switching and target zooming */
exec function NextWeapon()
{
	// If weapons are all loaded (debug) we will use the old weapon switch method
	if ( bWeaponsLoaded )
	{
		Super.NextWeapon();
	}
	// Else use the new method which will clamp the weapons to the 4 slots
	else
	{
		SwitchWeapon( TRUE );
	}
}

simulated function Pawn GetMyRealPawn()
{
	local Pawn MyPawn;
	local Vehicle WPVehicle;
	local GearPawn GP;

	MyPawn = Pawn;
	WPVehicle = Vehicle(Pawn);
	if ( WPVehicle != None )
	{
		GP = GearPawn(WPVehicle.Driver);
		if ( GP != None )
		{
			MyPawn = WPVehicle.Driver;
		}
		else
		{
			GP = GearPawn(ViewTarget);
			if ( GP != None )
			{
				MyPawn = Pawn(ViewTarget);
			}
		}
	}

	return MyPawn;
}

/** Player induced look at - pressed the "look at" input */
final function TryPOILookAt()
{
	ProcessPOI( None, true );
	ClearPOIIconDurations();
}

/** Player induced look away from POI - unpressed the "look at" input */
final function TryPOILookAway()
{
	local GearPointOfInterest CurrPOI;

	// See if there is a current POI
	if ( CameraLookAtFocusActor != None )
	{
		CurrPOI = GearPointOfInterest(CameraLookAtFocusActor);
		if ( CurrPOI != None )
		{
			// See if the current POI is still being forced
			if ( IsTimerActive('ForceLookDurationExpired') )
			{
				return;
			}
		}
	}

	StopForceLookAtPointOfInterest( true );
}

/** Timer callback function called when the force-look duration is over */
final function ForceLookDurationExpired()
{
	if ( !bLookingAtPointOfInterest || !IsHoldingPOIButton(TRUE) )
	{
		StopForceLookAtPointOfInterest( false );
	}
}


/**
 * Find the best POI to look at
 * @param POIToTest - a POI to test against, if the best POI has the same priorty as this one, the POIToTest will take precedence
 */
final simulated function GearPointOfInterest FindBestPOIToLookAt( optional GearPointOfInterest POIToTest )
{
	local GearPointOfInterest POI, BestPOI;
	local GearPawn GP;
	local int POIPriority, CurrPriority;
	local bool bSetBestPOI;
	local Pawn Myself;

	// set the initial ratings
	BestPOI = POIToTest;
	POIPriority = (BestPOI == None) ? -1 : BestPOI.LookAtPriority;

	// then look for an active point of interest based on priority
	foreach EnabledPointsOfInterest(POI)
	{
		bSetBestPOI = FALSE;
		Myself = GetMyRealPawn();
		// if enabled, and not attached or not attached to my pawn
		if ( POI.bEnabled && (POI.AttachedToActor == None || POI.AttachedToActor != Myself) )
		{
			CurrPriority = POI.GetLookAtPriority( self );
			if ( CurrPriority >= 0 )
			{
				if ( (BestPOI == None) ||
					(CurrPriority > POIPriority) ||
					((CurrPriority == POIPriority) && (POIToTest == None || BestPOI != POIToTest) && (VSizeSq(POI.Location - Pawn.Location) < VSizeSq(BestPOI.Location - Pawn.Location))) )
				{

					// If we're reviving, we only care about teammates
					if ( MyGearPawn != None && MyGearPawn.IsDBNO() )
					{
						// See if this is a gearpawn
						if ( POI.AttachedToActor != None )
						{
							GP = GearPawn(POI.AttachedToActor);
							if ( (GP != None) && (GP != Pawn) && GP.IsSameTeam(Pawn) )
							{
								bSetBestPOI = TRUE;
							}
						}
					}
					// if we're not reviving we care about all POIs
					else
					{
						bSetBestPOI = TRUE;
					}

					// set the best POI
					if ( bSetBestPOI )
					{
						BestPOI = POI;
						POIPriority = CurrPriority;
					}
				}
			}
		}
	}

	return BestPOI;
}

final simulated function ForceLookAtPointOfInterest( optional bool bUserInstigated, optional GearPointOfInterest POIToForce )
{
	local GearPlayerCamera Cam;
	local ActionInfo Action;
	local float POICamFOV;
	local int IndexInList;
	local bool bDoForceLookTimer;

	// disallow forcelook while chainsawing
	if ( bUserInstigated && (ForcedAdhesionTarget != None) )
	{
		return;
	}

	// If we have a camera
	Cam = GearPlayerCamera(PlayerCamera);
	if ( Cam != None )
	{
		// Find the best POI if one was not provided
		if ( POIToForce == None )
		{
			POIToForce = FindBestPOIToLookAt();
		}

		// Already looking at this POI so return
		if ( CameraLookAtFocusActor == POIToForce )
		{
			return;
		}
		// Stop an already existing force look
		else if ( bLookingAtPointOfInterest )
		{
			StopForceLookAtPointOfInterest(FALSE);
		}

		// Focus the camera on the new POI
		if ( POIToForce != None )
		{
			// Set the camera up to look at the POI
			POICamFOV = (Vehicle(Pawn) == None || GearVehicle(Pawn) != None || GearWeaponPawn(Pawn) != None) ? POIToForce.GetDesiredFOV(Cam.Location) : 0.f;
			SetFocusPoint(FALSE, POIToForce, PointOfInterestLookatInterpSpeedRange, vect2d(1.f,1.f), POICamFOV, TRUE, TRUE, TRUE);
			bLookingAtPointOfInterest = TRUE;

			// Play a sound if the player pressed the button
			if( bUserInstigated )
			{
				Pawn.PlaySound( SoundCue'Interface_Audio.Forcelook_Enter_cue' );
			}

			// Stop the player from pressing buttons
			DisableInput(true,false,false,true);

			// Show the text to the HUD
			if ( MyGearHud != None )
			{
				Action.ToolTipText = POIToForce.GetDisplayName();
				MyGearHud.SetActionInfo( AT_ForceLook, Action );
			}

			// If there is a force look duration
			if ( (POIToForce.ForceLookType != ePOIFORCELOOK_None) && (POIToForce.ForceLookDuration > 0.0f) )
			{
				bDoForceLookTimer = true;

				// If this is a player-induced force look, make sure we haven't done it already
				if ( POIToForce.ForceLookType != ePOIFORCELOOK_None )
				{
					IndexInList = EnabledPointsOfInterest.Find( POIToForce );
					if ( (IndexInList == INDEX_NONE) || POILookedAtList[IndexInList] )
					{
						bDoForceLookTimer = false;
					}
				}

				if ( bDoForceLookTimer )
				{
					SetTimer( POIToForce.ForceLookDuration, FALSE, nameof(ForceLookDurationExpired) );

				}
			}

			// Mark this POI as having been seen
			IndexInList = EnabledPointsOfInterest.Find( POIToForce );
			if ( IndexInList != INDEX_NONE )
			{
				POILookedAtList[IndexInList] = true;
			}

			// Have the server see if it needs to fire a kismet link
			ServerFirePOIActionOutputLink( POIToForce, eMPOIOUTPUT_LookAt );
		}
	}
}

/** Stop the current POI from being looked at */
final function StopForceLookAtPointOfInterest( optional bool bUserInstigated )
{
	local GearPlayerCamera Cam;
	local GearPointOfInterest POI;

	ClearTimer( 'ForceLookDurationExpired' );

	Cam = GearPlayerCamera(PlayerCamera);
	if ( bLookingAtPointOfInterest )
	{
		POI = GearPointOfInterest(CameraLookAtFocusActor);
		if ( POI != None )
		{
			// Have the server see if it needs to fire a kismet link
			ServerFirePOIActionOutputLink( POI, eMPOIOUTPUT_LookAway );
		}

		bLookingAtPointOfInterest = FALSE;

		if ( MyGearHud != None )
		{
			MyGearHud.ClearActionInfoByType( AT_ForceLook );
		}

		if ( Cam != None )
		{
			ClearFocusPoint(FALSE);
			ClientClearForcedCameraFOV();
			if( bUserInstigated == TRUE )
			{
				Pawn.PlaySound( SoundCue'Interface_Audio.Forcelook_Exit_cue' );
			}
			EnableInput(true,false,false);
		}
	}
}

/** Function that has the server fire the output link for the action of a POI */
final reliable server function ServerFirePOIActionOutputLink( GearPointOfInterest POI, int POIOutputType )
{
	if ( (POI != None) && (POI.POIAction != None) )
	{
		// Fire link
		POI.POIAction.OutputLinks[POIOutputType].bHasImpulse = true;

		// Set the enable flag false if we're looking away
		if ( POIOutputType == eMPOIOUTPUT_LookAway )
		{
			POI.POIAction.POI_bEnabled = false;
		}
	}
}


simulated function SetFocusPoint(bool bFromKismet, Actor FocusActor, vector2d InterpSpeedRange, vector2d InFocusFOV, optional float CameraFOV, optional bool bAlwaysFocus, optional bool bAdjustCamera, optional bool bIgnoreTrace, optional Name FocusBoneName)
{
	// tell camera to focus on the given actor
	GearPlayerCamera(PlayerCamera).GameplayCam.SetFocusOnActor(FocusActor, FocusBoneName, InterpSpeedRange, InFocusFOV, CameraFOV, bAlwaysFocus, bAdjustCamera, bIgnoreTrace);

	// needs to happen AFTER setfocuspoint, since setfocuspoint will clear it
	CameraLookAtFocusActor = FocusActor;
	bCameraLookAtIsFromKismet = bFromKismet;
}

simulated function ClearFocusPoint(bool bFromKismet, optional bool bForceLeaveRotation)
{
	local bool bLeaveRotation;

	if ( bFromKismet == bCameraLookAtIsFromKismet )
	{
		bLeaveRotation = bForceLeaveRotation ||
						( (GearPointOfInterest(CameraLookAtFocusActor) != None) && GearPointOfInterest(CameraLookAtFocusActor).bLeavePlayerFacingPOI );

		// Don't for rotation of pawn when on a turret
		if(Turret(Pawn) != None)
		{
			bLeaveRotation = FALSE;
		}

		CameraLookAtFocusActor = None;
		GearPlayerCamera(PlayerCamera).GameplayCam.ClearFocusPoint(bLeaveRotation);
	}
}


simulated function OnForceCombatGUDS(SeqAct_ForceCombatGUDS Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		// enable
		if (GearGame(WorldInfo.Game).UnscriptedDialogueManager != None)
		{
			GearGame(WorldInfo.Game).UnscriptedDialogueManager.bForceInCombatGUDS = TRUE;
		}
		Action.OutputLinks[1].bHasImpulse = true;
	}
	else
	{
		// disable
		if (GearGame(WorldInfo.Game).UnscriptedDialogueManager != None)
		{
			GearGame(WorldInfo.Game).UnscriptedDialogueManager.bForceInCombatGUDS = FALSE;
		}
		Action.OutputLinks[2].bHasImpulse = true;
	}

	Action.OutputLinks[0].bHasImpulse = true;
}


/** Returns whether or not we think the player is in combat */
event bool IsInCombat()
{
	local Controller C;
	local bool bWasInCombat;
	// if we don't have a gear pawn we can't be in combast
	if (MyGearPawn == None)
	{
		return FALSE;
	}
	// server replicates combat state to client
	if (Role < ROLE_Authority)
	{
		// so just return the cached version
		return MyGearPawn.bIsInCombat;
	}
	// always return false in MP - kind of backwards but it was this way unintentionally for a long time
	// and we decided to keep it
	if (GearGameMP_Base(WorldInfo.Game) != None)
	{
		MyGearPawn.bIsInCombat = false;
		return false;
	}
	// only update the cached value if it's been a while
	if (WorldInfo.TimeSeconds - LastCheckIsInCombatTime > 1.0f)
	{
		bWasInCombat = MyGearPawn.bIsInCombat;
		LastCheckIsInCombatTime = WorldInfo.TimeSeconds;

		MyGearPawn.bIsInCombat = FALSE;
		foreach WorldInfo.AllControllers( class'Controller', C )
		{
			// If there is a controller that has an enemy on the other team
			if (C != self &&
				C.Pawn	!= None &&
				C.Enemy != None &&
				(GearPawn(C.Pawn) == None || !GearPawn(C.Pawn).IsAHostage()) &&
				!C.Enemy.IsSameTeam( C.Pawn ) )
			{
				// Consider them in combat
				MyGearPawn.bIsInCombat = TRUE;
				break;
			}
		}

		// if not in combat
		if( !PlayingDevLevel() && !MyGearPawn.bIsInCombat && bWasInCombat )
		{
			SetTimer( RandRange(5.f,15.f),FALSE,nameof(ReviveTeammates) );
			SetTimer( RandRange(5.f,10.f),FALSE,nameof(ExecuteMeatshields) );
		}

	}
	return MyGearPawn.bIsInCombat;
}

function ExecuteMeatshields()
{
	local GearPawn GP;
	if (!IsInCombat() || bCinematicMode)
	{
		// delete any meatshields
		foreach WorldInfo.AllPawns(class'GearPawn',GP)
		{
			if (GP.IsAKidnapper())
			{
				GP.ServerDoSpecialMove(SM_Kidnapper_Execution,TRUE,GP.InteractionPawn);
			}
		}
    }
}

function ReviveTeammates()
{
	local GearAI_COGGear Gear;
	local GearPawn GP;

	// check to revive friendly AI
	foreach WorldInfo.AllControllers(class'GearAI_COGGear',Gear)
	{
		GP = GearPawn(Gear.Pawn);
		if( GP != None && GP.IsDBNO() )
		{
			GP.ReviveFromAutoRevive(MyGearPawn);
		}
	}
}

/* epic ===============================================
* ::GameHasEnded
*
* Called from game info upon end of the game, used to
* transition to proper state.
*
* =====================================================
*/
function GameHasEnded(optional Actor EndGameFocus, optional bool bIsWinner)
{
}

event MayFall()
{
	local vector HitLocation, HitNormal;

	// if we have a clamped base and can't find it straight down, reverse the move so we don't fall
	if ( MyGearPawn != None && MyGearPawn.ClampedBase != None &&
		!TraceComponent(HitLocation, HitNormal, MyGearPawn.ClampedBase.CollisionComponent, Pawn.Location - vect(0,0,1000), Pawn.Location, Pawn.GetCollisionExtent() * vect(0.5,0.5,1)) )
	{
		Pawn.bCanJump = false;
	}
}

state PlayerWalking
{
	exec function bool TryASpecialMove(bool bRunActions)
	{
		`LogSM("Try a walking special move");
		if (!Global.TryASpecialMove(bRunActions))
		{
			if (!bIsTargeting)
			{
				if( (!bUseAlternateControls || !bRunActions) && CanDoSpecialMove(SM_MantleDown) )
				{
					DoSpecialMove(SM_MantleDown);
					return TRUE;
				}
				else if ( !bUseAlternateControls || bRunActions )
				{
					// setup the timer for evade/roadierun
					SetTimer( RoadieRunTimer, FALSE, nameof(TryToRoadieRun) );
				}
			}
		}
		return FALSE;
	}

	/**
	 * Overridden to not clobber PHYS_None when performing special moves as it will conflict
	 * with root motion.  If the origin of the PlayerController hack is ever discovered then
	 * hopefully this hack will be unnecessary.
	 */
	function BeginState(Name PreviousStateName)
	{
		GetPRI().PlayerStatus = WPS_Alive;

		// Reset double click if it's not active
		// (keep it if it is active so we can roadie run after a special move)
		if( DoubleClickDir != DCLICK_Active && DoubleClickDir != DCLICK_Back )
		{
			ResetDoubleClick();
		}

		StopPostProcessOverride(CurrentPPType);

		bPressedJump = false;
		GroundPitch = 0;
		if ( Pawn != None )
		{
			Pawn.ShouldCrouch(false);
			if (MyGearPawn == None || MyGearPawn.SpecialMove == SM_None)
			{
				// FIXME HACK!!!
				if ( Pawn.Physics != PHYS_Falling && Pawn.Physics != PHYS_RigidBody &&
					( Pawn.Physics != PHYS_None || InterpActor_GearBasePlatform(Pawn.Base) == None ||
						!InterpActor_GearBasePlatform(Pawn.Base).bDisallowPawnMovement ) )
				{
					Pawn.SetPhysics(PHYS_Walking);
				}
			}
		}
	}

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot)
	{
		local GearPawn P;

		super.ProcessMove( DeltaTime, NewAccel, DoubleClickMove, DeltaRot );

		P = GearPawn(Pawn);
		if( P == None )
		{
			return;
		}

		if( IsLocalPlayerController() )
		{
			if( DoubleClickMove != DCLICK_None )
			{
				if( IsDoingSpecialMove(SM_RoadieRun) &&
					DoubleClickMove != DCLICK_Back	 &&
					DoubleClickMove != DCLICK_Active &&
					DoubleClickMove != DCLICK_Done )
				{
					if( !TryToRunToCover( TRUE ) )
					{
						TryToEvade(DoubleClickMove);
					}
					else
					{
						FinishDoubleClick(TRUE);
					}
				}
				else if( DoubleClickMove == DCLICK_Forward )
				{
					// Try to get into cover OR mantle down
					if( TryDoubleTapMove(SM_MantleDown) )
					{
						// successfully ran to cover or hopped down
					}

					// Set timer to try Roadie Run - shorter for double tap
					SetTimer( RoadieRunTimer * 0.5, FALSE, nameof(TryToRoadieRun) );
				}
				else if( DoubleClickMove != DCLICK_Active && DoubleClickMove != DCLICK_Done )
				{
					if( !IsInCoverState() && TryToRunToCover(,, DoubleClickMove) )
					{
						// successfully ran to cover
					}
					else
					{
						TryToEvade(DoubleClickMove);
					}
					FinishDoubleClick(TRUE);
				}

				if( DoubleClickMove == DCLICK_Active )
				{
					if( IsDoingSpecialMove(SM_RoadieRun) )
					{
						if ( bUseAlternateControls )
						{
							if( !MainPlayerInput.IsButtonActive(GB_X) && PlayerInput.RawJoyUp == 0 )
							{
								MainPlayerInput.HandleButtonInput_X( false );
							}
						}
						else
						{
							if( !MainPlayerInput.IsButtonActive(GB_A) && PlayerInput.RawJoyUp == 0 )
							{
								MainPlayerInput.HandleButtonInput_A( false );
							}
						}
					}
					else if( !IsDoingASpecialMove() && PlayerInput.RawJoyUp == 0 )
							// JF: Commented this.  Seems intended to handle forward doubletaps, and we want to be able to
							// doubletap forward while running right or left.
							// && PlayerInput.RawJoyRight == 0 )
					{
						MainPlayerInput.HandleButtonInput_A( false );
						FinishDoubleClick(TRUE);
					}
				}
				else if( DoubleClickMove != DCLICK_Done && DoubleClickDir  != DCLICK_Done )
				{
					// Set click direction as active
					CurrentDoubleClickDir = DoubleClickDir;
					DoubleClickDir = DCLICK_Active;
				}
			}
			else if( IsDoingSpecialMove(SM_RoadieRun) )
			{
				if ( bUseAlternateControls )
				{
					if( !MainPlayerInput.IsButtonActive(GB_X) && PlayerInput.RawJoyUp == 0 )
					{
						MainPlayerInput.HandleButtonInput_X( false );
					}
				}
				else
				{
					if( !MainPlayerInput.IsButtonActive(GB_A) && PlayerInput.RawJoyUp == 0 )
					{
						MainPlayerInput.HandleButtonInput_A( false );
					}
				}
			}
		}
	}
}


/************************************************************************************
 * SpecialMoves
 ***********************************************************************************/

/** Returns TRUE if player is doing a special move */
function bool IsDoingASpecialMove()
{
	return (MyGearPawn != None && MyGearPawn.IsDoingASpecialMove());
}


/**
 * Returns TRUE if player is current performing TestMove.
 */
simulated function bool IsDoingSpecialMove(ESpecialMove AMove)
{
	return (MyGearPawn != None && MyGearPawn.IsDoingSpecialMove(AMove));
}


/**
 * returns true if player can perform requested special move
 * @param bForceCheck - Allows you to skip the single frame condition (which will be incorrect on clients since LastCanDoSpecialMoveTime isn't replicated)
 */
final native function bool CanDoSpecialMove(ESpecialMove AMove, optional bool bForceCheck);

final event bool AllowEvadeOffLedge()
{
	return (MyGearPawn.IsEvading()) && MyGearPawn.CanPerformMantleDown(class'GSM_MantleDown'.default.MinMantleHeight,class'GSM_MantleDown'.default.MaxMantleHeight,rotator(Pawn.Velocity));
}

reliable final server function ServerAbortRun2Cover()
{
	AbortRun2Cover();
}

simulated function AbortRun2Cover()
{
	if (Role < ROLE_Authority)
	{
		ServerAbortRun2Cover();
	}
	LeaveCover();
	if (MyGearPawn != None &&
		(MyGearPawn.IsDoingSpecialMove(SM_Run2MidCov) || MyGearPawn.IsDoingSpecialMove(SM_Run2StdCov)))
	{
		MyGearPawn.PendingSpecialMoveStruct = MyGearPawn.FillSMStructFromParams(SM_None);
		MyGearPawn.EndSpecialMove();
	}
}

/**
 * Perform a special move locally, and replicate it to server if necessary
 * @network: local player.
 */
final event DoSpecialMove(ESpecialMove NewMove, optional bool bForceMove=FALSE, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0)
{
	local SMStruct	NewMoveStruct;

	`if(`notdefined(FINAL_RELEASE))
	if( LocalPlayer(Player) == None )
	{
		`Warn(GetFuncName() @ "has to be called from local player!");
		ScriptTrace();
		return;
	}
`endif

	// if we're doing a move2idle then force the special move to start immediately
	bForceMove = bForceMove || MyGearPawn.IsDoingMove2IdleTransition();

	if( bForceMove || CanDoSpecialMove(NewMove) )
	{
		// Build new move struct
		NewMoveStruct = MyGearPawn.FillSMStructFromParams(NewMove, InInteractionPawn, InSpecialMoveFlags);

		// simulate the move locally
		MyGearPawn.DoSpecialMoveFromStruct(NewMoveStruct, bForceMove);

		// Replicate action to the server w/ updated inputs
		if( Role < Role_Authority )
		{
			ClientToServerDoSpecialMove(NewMoveStruct, bForceMove, RemappedJoyUp, RemappedJoyRight, Pawn.Rotation.Yaw);
		}
	}
}


/**
 * Perform a special move on the server.
 * Called from DoSpecialMove().
 * @network: server.
 */
reliable server final function ClientToServerDoSpecialMove(SMStruct NewMoveStruct, bool bForceMove, float PlayerJoyUp, float PlayerJoyRight, int RotYaw)
{
	local rotator NewRot;

	//`log("serverdospecialmove "$NewMove@GetStateName());
	if( MyGearPawn != None )
	{
		//`log("serverdospecialmove "$NewMove);
		// Copy over
		RemappedJoyRight = PlayerJoyRight;
		RemappedJoyUp	 = PlayerJoyUp;

		NewRot		= Pawn.Rotation;
		NewRot.Yaw	= RotYaw;
		Pawn.SetRotation(NewRot);
		bForceMove = bForceMove || MyGearPawn.IsDoingMove2IdleTransition();
		MyGearPawn.DoSpecialMoveFromStruct(NewMoveStruct, bForceMove);
	}
}

/** Replicated function to owning client, called only from ServerDictateSpecialMove() */
reliable client final function ServerToClientDoSpecialMove(SMStruct NewMoveStruct, bool bForceMove)
{
	if( MyGearPawn != None )
	{
		MyGearPawn.DoSpecialMoveFromStruct(NewMoveStruct, bForceMove);
	}
}

/** Stop current special move */
final function EndSpecialMove()
{
	if( IsDoingASpecialMove() )
	{
		ResetDoubleClick();

		if( MyGearPawn != None )
		{
			if( MyGearPawn.IsLocallyControlled() )
			{
				MyGearPawn.LocalEndSpecialMove();
			}
			else
			{
				MyGearPawn.ServerEndSpecialMove();
			}
		}
	}
}


/** Server forces a special move to be replicated to the owning player. */
final event ServerDictateSpecialMove(ESpecialMove NewMove, optional GearPawn InInteractionPawn, optional INT InSpecialMoveFlags=0)
{
	local SMStruct	NewMoveStruct;

	if( WorldInfo.NetMode == NM_Client )
	{
		`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Called on non servers!!" );
		ScriptTrace();
		return;
	}

	// Build new move struct
	NewMoveStruct = MyGearPawn.FillSMStructFromParams(NewMove, InInteractionPawn, InSpecialMoveFlags);

	// if dedicated server, replicate special move to local player (client)
	if( LocalPlayer(Player) == None )
	{
		ServerToClientDoSpecialMove(NewMoveStruct, TRUE);
	}

	// Do special move right away on server.
	MyGearPawn.DoSpecialMoveFromStruct(NewMoveStruct, TRUE);
}

/**
 * Notification called when a new special move is started.
 * Note: It is NOT safe to start a different special move from here!!
 */
function SpecialMoveStarted(ESpecialMove NewMove)
{
	// Clear roadie run timer when starting a new special move
	// besides a transition move, we still want to be able to abort it with a slide/evade
	if( NewMove != SM_Run2Idle && NewMove != SM_Walk2Idle )
	{
		ClearTimer('TryToRoadieRun');
	}
}

function bool TryALadderClimb()
{
	// Test for ladder climbing first.
	if ( CanDoSpecialMove(SM_LadderClimbDown) )
	{
		if (MyGearPawn.IsCarryingAHeavyWeapon())
		{
			MyGearPawn.ThrowActiveWeapon();
		}
		DoSpecialMove(SM_LadderClimbDown, TRUE);
		return TRUE;
	}
	if ( CanDoSpecialMove(SM_LadderClimbUp) )
	{
		if (MyGearPawn.IsCarryingAHeavyWeapon())
		{
			MyGearPawn.ThrowActiveWeapon();
		}
		DoSpecialMove(SM_LadderClimbUp, TRUE);
		return TRUE;
	}
	return FALSE;
}


/**
 * Tries to perform a context sensitive special move
 * @param bRunActions - Only used with alternate controls to determine if we are calling this from the action button or the run/evade button (TRUE means the Run/Evade variety)
 */
exec function bool TryASpecialMove(bool bRunActions)
{
	local GearPawn		out_InteractionPawn;
	local ESpecialMove	out_SpecialMove;

	`LogSM("Try a global special move");
	if (MyGearPawn != None && !bIsTargeting && (!bRunActions || !bUseAlternateControls))
	{
		// Check Pawn to Pawn interactions
		if(	MyGearPawn.CheckPawnToPawnInteractions(out_SpecialMove, out_InteractionPawn) )
		{
			MyGearPawn.DoPawnToPawnInteraction(out_SpecialMove, out_InteractionPawn);
			return TRUE;
		}
		if (TryALadderClimb())
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
 * Tries to perform the Roadie Run special move.
 */
final function TryToRoadieRun()
{
	// if still holding the 'a' button and can do the special move (skip while evading since that is handled via GSM_Evade.SpecialMoveEnded)
    if (IsHoldingRoadieRunButton() && !MyGearPawn.IsEvading() && !MyGearPawn.IsDoingSpecialMove(SM_MidLvlJumpOver) && !MyGearPawn.IsDoingSpecialMove(SM_WeaponPickup) )
    {
		// then start running
    	if (CanDoSpecialMove(SM_RoadieRun))
    	{
    		DoSpecialMove(SM_RoadieRun);
		}
		else if (CanDoSpecialMove(SM_CoverRun))
		{
			DoSpecialMove(SM_CoverRun);
		}
	}
}


/** Return TRUE if Pawn can try to run to cover */
function bool CanTryToRunToCover()
{
	if( MyGearPawn == None || (MyGearPawn.IsDoingASpecialMove() && !MyGearPawn.IsDoingMove2IdleTransition() && !MyGearPawn.IsDoingSpecialMove(SM_PushOutOfCover) &&
		(!bUseAlternateControls || !MyGearPawn.IsDoingSpecialMove(SM_RoadieRun))) )
	{
`if(`notdefined(FINAL_RELEASE))
		if( MyGearPawn != None && bDebugCover )
		{
			`log(WorldInfo.TimeSeconds @ GetFuncName() @ "Failed to Run2Cover because doing SpecialMove:" @ MyGearPawn.SpecialMove );
		}
`endif
		return FALSE;
	}
	else if (MyGearPawn.bIsConversing || MyGearPawn.IsCarryingShield() || IsTimerActive(nameof(PickingUpAWeapon)))
	{
		return FALSE;
	}

	return TRUE;
}

/** See if player can perform run 2 cover move */
function bool TryToRunToCover(optional bool bSkipSpecialMoveCheck, optional float CheckScale, optional EDoubleClickDir DblClickDirToCheck=DCLICK_None, optional bool bPreventSideEntry)
{
	local CovPosInfo	FoundCovInfo;
	local int			NoCameraAutoAlign;

	if( CanRunToCover(FoundCovInfo, NoCameraAutoAlign, bSkipSpecialMoveCheck, CheckScale, DblClickDirToCheck,bPreventSideEntry) )
	{
		AcquireCover(FoundCovInfo, (NoCameraAutoAlign == 1));
		return TRUE;
	}

	return FALSE;
}

/**
 * See if player can perform run 2 cover move
 * @param CoverCanRunTo - set to the cover that you can run to if successful
 * @param NoCameraAutoAlign - set if successful (acts as a bool)
 */
function bool CanRunToCover(out CovPosInfo CoverCanRunTo, out int NoCameraAutoAlign, optional bool bSkipSpecialMoveCheck, optional float CheckScale, optional EDoubleClickDir DblClickDirToCheck=DCLICK_None, optional bool bPreventSideEntry)
{
	local vector		CheckDir, CamLoc;
	local rotator		CamRot;
	local bool          bEnteringCoverSideways;
	local float         DistanceToCheckToEnterCover;

	NoCameraAutoAlign		= 0;
	bEnteringCoverSideways	= FALSE;

	if( MyGearPawn != None && PlayerInput != None )
	{
		if( CheckScale <= 0.f )
		{
			CheckScale = 1.f;
		}

		// Check if move can be done
		if( MyGearPawn.CoverType != CT_None || (!bSkipSpecialMoveCheck && !CanTryToRunToCover()) )
		{
			return FALSE;
		}

		// Try to find cover to run to
		if (DblClickDirToCheck != DCLICK_None)
		{
			switch (DblClickDirToCheck)
			{
			case DCLICK_Back:
				CheckDir = vect(-1.f, 0.f, 0.f);
				break;
			case DCLICK_Forward:
				CheckDir = vect(1.f, 0.f, 0.f);
				break;
			case DCLICK_Left:
				CheckDir = vect(0.f, -1.f, 0.f);
				bEnteringCoverSideways = TRUE;
				break;
			case DCLICK_Right:
				CheckDir = vect(0.f, 1.f, 0.f);
				bEnteringCoverSideways = TRUE;
				break;
			}
			GetPlayerViewPoint(CamLoc,CamRot);
			CheckDir = Normal(CheckDir >> CamRot);

			NoCameraAutoAlign = 1;
		}
		else if( Sqrt(Square(PlayerInput.RawJoyRight) + Square(PlayerInput.RawJoyUp)) > DeadZoneThreshold )
		{
			// if any direction allowed then use the controls relative to the camera
			CheckDir.X = PlayerInput.RawJoyUp;
			CheckDir.Y = PlayerInput.RawJoyRight;
			GetPlayerViewPoint(CamLoc,CamRot);
			CheckDir = Normal(CheckDir >> CamRot);

			// Stick direction is horizontal
			if( Abs(PlayerInput.RawJoyRight) > Abs(PlayerInput.RawJoyUp) )
			{
				bEnteringCoverSideways = TRUE;
			}
			// Vertical
			else
			{
				if( PlayerInput.RawJoyUp < -DeadZonethreshold )
				{
					// backing up into cover, dont align camera
					//NoCameraAutoAlign = 1;
					return FALSE;
				}
			}

			NoCameraAutoAlign = 1;
		}
		else
		{
			// otherwise only check forward
			CheckDir = vector(Pawn.Rotation);
			NoCameraAutoAlign = 0;
		}


		if( bEnteringCoverSideways )
		{
			if (bPreventSideEntry)
			{
				return FALSE;
			}
			DistanceToCheckToEnterCover = MyGearPawn.Run2CoverPerpendicularMaxDist;
		}
		else
		{
			DistanceToCheckToEnterCover = MyGearPawn.Run2CoverMaxDist * CheckScale;
		}
		//`log( "bEnteringCoverSideways: " $ bEnteringCoverSideways );

		if( MyGearPawn.CanPrepareRun2Cover(DistanceToCheckToEnterCover, CoverCanRunTo, CheckDir) )
		{
			return TRUE;
		}
	}

	return FALSE;
}


/************************************************************************************
 * Evade
 ***********************************************************************************/

function ESpecialMove ChooseEvadeMoveBasedOnStickDir()
{
	local ESpecialMove EvadeMove;

	// fwd by default
	EvadeMove = SM_EvadeFwd;

	if (abs(RemappedJoyUp) > abs(RemappedJoyRight))
	{
		if (RemappedJoyUp > DeadZoneThreshold)
		{
			EvadeMove = SM_EvadeFwd;
		}
		else
			if (RemappedJoyUp < -DeadZoneThreshold)
			{
				EvadeMove = SM_EvadeBwd;
			}
	}
	else
	{
		if (RemappedJoyRight > DeadZoneThreshold)
		{
			EvadeMove = SM_EvadeRt;
		}
		else
			if (RemappedJoyRight < -DeadZoneThreshold)
			{
				EvadeMove = SM_EvadeLt;
			}
	}

	return EvadeMove;
}

exec function bool TryToEvade( optional EDoubleClickDir DoubleClickInput )
{
	local ESpecialMove	EvadeMove;
	local Rotator NewRotation;

	if( !CanEvade() )
	{
		return FALSE;
	}

	// By default: go forward
	EvadeMove = SM_EvadeFwd;
	if( DoubleClickInput != DCLICK_None )
	{
		if( DoubleClickInput == DCLICK_Forward )
		{
			EvadeMove = SM_EvadeFwd;
		}
		else if( DoubleClickInput == DCLICK_Back )
		{
			EvadeMove = SM_EvadeBwd;
		}
		else if( DoubleClickInput == DCLICK_Left )
		{
			EvadeMove = SM_EvadeLt;
		}
		else if( DoubleClickInput == DCLICK_Right )
		{
			EvadeMove = SM_EvadeRt;
		}
	}
	else
	{
		EvadeMove = ChooseEvadeMoveBasedOnStickDir();
	}

	CurrentDoubleClickDir = DoubleClickInput;

	// Trigger Evade
	CoverLog("Evade Move:" @ EvadeMove, "PlayerEvade");
	if( EvadeMove != SM_None )
	{
		NewRotation.Yaw = Rotation.Yaw;
		MyGearPawn.SetRotation(NewRotation);
		DoSpecialMove(EvadeMove);
		return TRUE;
	}

	return FALSE;
}


/**
 * Stop Evade Move.
 * Network: Local Player.
 */
final function StopEvade()
{
	if( MyGearPawn.IsEvading() )
	{
		EndSpecialMove();
	}
}


/**
 * Return true if Can perform an Evade move.
 * Network: LocalPlayer
 */
final function bool CanEvade()
{
	return(	MyGearPawn != None &&
			!MyGearPawn.bWantsToMelee &&
			!MyGearPawn.bIsConversing &&
			MyGearPawn.Weapon.CurrentFireMode != class'GearWeapon'.const.MELEE_ATTACK_FIREMODE &&
			MyGearPawn.bWantsToBeMirrored == MyGearPawn.bIsMirrored &&	            // can abort mirror transition, but only once the mesh has been mirrored.
			(!IsMoveInputIgnored() || MyGearPawn.IsDoingSpecialMove(SM_PushOutOfCover)) &&
			!IsTimerActive( 'CannotRetriggerEvade' ) &&
			!MyGearPawn.IsEvading() &&
			!MyGearPawn.IsCarryingAHeavyWeapon()
			);


}

/**
 * Called once our last evade has finished.
 */
simulated function NotifyEvadeFinished()
{
	// Set delay before allowing next evade move
	SetTimer( EvadeRetriggerDelay, FALSE, nameof(CannotRetriggerEvade) );
	CurrentDoubleClickDir = DCLICK_None;
}


/** Dummy functions, used for timers */
function CannotRetriggerEvade();


/************************************************************************************
 * CQC (Close Quarter Combat)
 ***********************************************************************************/

reliable server function ServerDoCQCMove(EGameButtons Button)
{
	if( MyGearPawn != None )
	{
		MyGearPawn.ServerDoCQCMove(Button);
	}
}

/**
 * 0 - curb stomp + revival
 * 1 - door kick
 * 2 - button push
 * 3 - push object
 * 4 - weapon pickup
 */
unreliable server function ServerUse()
{
	local Actor							TouchActor;
	local array<SequenceEvent>			EngageEvents;
	local SeqEvt_Engage					EngageEvent;
	local GearPC						PC;
	local bool							bAlreadyInUse;

	if( MyGearPawn == None )
	{
		// normal use events. Like getting in/out of vehicles/turrets.
		Super.ServerUse();
		return;
	}

	// check for leaving/entering a vehicle
	if( Vehicle(Pawn) != None )
	{
		Vehicle(Pawn).DriverLeave(FALSE);
		return;
	}

	// if we're doing any other special move
	if( MyGearPawn.IsDoingASpecialMove() && !MyGearPawn.IsDoingSpecialMove(SM_UsingCommLink) )
	{
		// then don't allow any other interactions
		return;
	}

	// Revive Special Move
	if( CanDoSpecialMove(SM_ReviveTeamMate) )
	{
		ServerDictateSpecialMove(SM_ReviveTeamMate);
		return;
	}

	// Close Quarter Combat - CurbStomp or PunchFace
	if( CanDoSpecialMove(SM_CQCMove_CurbStomp) )
	{
		ServerDictateSpecialMove(SM_CQCMove_CurbStomp, GSM_CQC_Killer_Base(MyGearPawn.SpecialMoves[SM_CQCMove_CurbStomp]).Follower);
		return;
	}

	// do a bunch of other things if not in cover
	if( !MyGearPawn.IsInCover() )
	{
		// these special moves interrupt reloads, so prevent them if reloading
		if( MyGearPawn.Weapon == None ||
			(MyGearPawn.Weapon.CurrentFireMode != class'GearWeapon'.const.FIREMODE_ACTIVERELOADSUCCESS &&
			 MyGearPawn.Weapon.CurrentFireMode != class'GearWeapon'.const.FIREMODE_ACTIVERELOADSUPERSUCCESS) )
		{
			// check for door kick
			if( DoorTriggerDatum.bInsideDoorTrigger )
			{
				DoDoorOpen(SM_DoorKick);
				return;
			}
		}

		// look for interaction triggers
		if (!MyGearPawn.bIsConversing)
		{
			foreach MyGearPawn.TouchingActors(class'Actor', TouchActor)
			{
				if( Trigger_ButtonInteraction(TouchActor) != None )
				{
					if( TouchActor.TriggerEventClass(class'SeqEvt_Interaction', MyGearPawn) )
					{
						MyGearPawn.SpecialMoveLocation = TouchActor.Location;
						ServerDictateSpecialMove(SM_PushButton);
						Trigger_ButtonInteraction(TouchActor).NotifyTriggered();
						// no need to check further
						return;
					}
				}
				else if( (Trigger_Engage(TouchActor) != None) && (Trigger_Engage(TouchActor).LoopCounter > 0) )
				{
					// make sure another player is not already activating this trigger
					bAlreadyInUse = FALSE;
					foreach WorldInfo.AllControllers(class'GearPC', PC)
					{
						if( PC.IsInState('Engaging') && GearPawn(PC.Pawn) != None && GearPawn(PC.Pawn).EngageTrigger == TouchActor )
						{
							bAlreadyInUse = TRUE;
							break;
						}
					}
					// we need to make sure the event can fire once for every turn, plus the finish, so increase the MaxTriggerCount
					if( !bAlreadyInUse && TouchActor.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
					{
						EngageEvent = SeqEvt_Engage(EngageEvents[0]);
						if( (EngageEvent != None) && EngageEvent.bEnabled && CanDoSpecialMove(SM_Engage_Start) )
						{
							// will disable in the special move code when the turning is done, make sure this doesn't backdoor us
							EngageEvent.MaxTriggerCount = 0;
							// set the pawn for moving to the engage actor
							ServerDictateSpecialMove(SM_Engage_Start);
							GotoState('Engaging');
							ClientGotoState('Engaging');
							// no need to check further
							return;
						}
					}
				}
			}
		}
	}
	// check for push object
	else
	{
		// push object
		if( CanDoSpecialMove(SM_PushObject) )
		{
			ServerDictateSpecialMove(SM_PushObject);
			return;
		}
	}

	if( !MyGearPawn.bIsConversing && FindVehicleToDrive() )
	{
		return;
	}

	// normal use events
	TriggerInteracted();
}

reliable server function ServerDoShieldKickOver(GearDroppedPickup_Shield Shield)
{
	if (Shield.CanKickOver(MyGearPawn))
	{
		Shield.SetTimer(0.85f,FALSE,nameof(Shield.KickOver));
		MyGearPawn.ServerDoSpecialMove(SM_DoorKick, TRUE);
	}
}

//`define debugpickup(msg) `log(`msg)
`define debugpickup(msg)

final function bool TryToPickupAWeapon()
{
	local GearDroppedPickup_Shield		Shield;
	local Actor A;
	local Name PickupInteraction;
	`debugpickup("Looking for pickups"@`showvar(MyGearPawn.SpecialMove));
	// Pickups
	if (CanTryToPickupWeapons())
	{
		A = MyGearPawn.FindNearbyPickup();
		if( A != None )
		{
			`debugpickup("Found pickup"@`showvar(A));
			Shield = GearDroppedPickup_Shield(A);
			if (Shield != None && Shield.CanKickOver(MyGearPawn))
			{
				ServerDoShieldKickOver(Shield);
				return TRUE;
			}

			if (GearDiscoverablesPickupFactoryBase(A) == None)
			{
				// See if we can pick up that weapon.
				if( CanDoSpecialMove(SM_WeaponPickup) )
				{
					`debugpickup("Attempting to pickup"@`showvar(A));
					if (GearDroppedPickup(A) != None)
					{
						PickupInteraction = GearDroppedPickup(A).FindInteractionWith(Pawn,FALSE);
					}
					else
					{
						PickupInteraction = GearPickupFactory(A).FindInteractionWith(Pawn,FALSE);
					}
					if (PickupInteraction != 'SwapWithCurrent' && PickupInteraction != 'TakeAmmo' && MyGearPawn.IsCarryingShield())
					{
						MyGearPawn.DropShield();
						PopState();
					}
					// set a timer to delay further interactions to prevent special move interaction issues in situations with latency
					SetTimer( 0.35f,FALSE,nameof(PickingUpAWeapon) );
					// tell the server we want to pickup the item
					MyGearPawn.PickUpWeapon(A);
					// do a local claim to prevent icon shenanigans
					if (GearPickupFactory(A) != None)
					{
						GearPickupFactory(A).ClaimedBy = MyGearPawn;
					}
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

function bool CanTryToPickupWeapons()
{
	return MyGearPawn != None && Vehicle(Pawn) == None && (!MyGearPawn.IsDoingASpecialMove() || MyGearPawn.IsDoingMove2IdleTransition() || MyGearPawn.IsDoingSpecialMove(SM_UsingCommLink));

}

function DisablePickupAction()
{
}

function PickingUpAWeapon()
{
	// just a stub
}

/** Function called to see if there is any client-side-only pickups to grab */
final function bool ClientUse()
{
	local Actor							PickupActor;
	local Actor							TouchActor;
	local array<SequenceEvent>			EngageEvents;
	local SeqEvt_Engage					EngageEvent;
	local GearDiscoverablesPickupFactoryBase Discoverable;

	// no interactions when dead
	if (MyGearPawn != None && MyGearPawn.Health <= 0)
	{
		return TRUE;
	}

	// carrying a crate?  can't do anything!
	if (MyGearPawn.CarriedCrate != None)
	{
		return TRUE;
	}

	// if recently attempted to pickup a weapon prevent any other interactions
	if (IsTimerActive('PickingUpAWeapon'))
	{
		return TRUE;
	}

	// do an extra client side check to make sure we revive/execute before picking anything up
	if( CanDoSpecialMove(SM_ReviveTeamMate) || CanDoSpecialMove(SM_CQCMove_CurbStomp) )
	{
		// return false so that we still call the server version which starts the revive/execute
		return FALSE;
	}

	// check for potential interactions
	foreach MyGearPawn.TouchingActors(class'Actor', TouchActor)
	{
		if( Trigger_ButtonInteraction(TouchActor) != None )
		{
			if( TouchActor.TriggerEventClass(class'SeqEvt_Interaction', MyGearPawn, -1, TRUE) )
			{
				return FALSE;
			}
	    }
		else if (Trigger_Engage(TouchActor) != None && Trigger_Engage(TouchActor).LoopCounter > 0)
		{
			if( TouchActor.FindEventsOfClass(class'SeqEvt_Engage', EngageEvents) )
			{
				EngageEvent = SeqEvt_Engage(EngageEvents[0]);
				if( (EngageEvent != None) && EngageEvent.bEnabled && CanDoSpecialMove(SM_Engage_Start) )
				{
					return FALSE;
				}
			}
		}
    }


	if (TryALadderClimb())
	{
		return TRUE;
	}

	if (TryToPickupAWeapon())
	{
		return TRUE;
	}

	// Pickups
	if (MyGearPawn != None)
	{
		PickupActor = MyGearPawn.FindNearbyDiscoverables();
	}
	if( PickupActor != None )
	{
		Discoverable = GearDiscoverablesPickupFactoryBase(PickupActor);
		if( Discoverable != None )
		{
			return TryToPickUpDiscoverable(Discoverable);
		}
	}

	return FALSE;
}

/** Must run test to see if player needs this tag on the client */
final function bool TryToPickUpDiscoverable( GearDiscoverablesPickupFactoryBase Dicoverable )
{
	if ( Dicoverable.CanBePickedUpBy(MyGearPawn) && CanDoSpecialMove(SM_WeaponPickup) )
	{
		Dicoverable.GiveTo( MyGearPawn );
		return TRUE;
	}

	return FALSE;
}

/** See if any local controllers need to turn discoverables on/off */
final function CheckForDiscoverableVisibility()
{
	local GearDiscoverablesPickupFactoryBase Factory;

	// Loop through each factory
	foreach WorldInfo.AllNavigationPoints( class'GearDiscoverablesPickupFactoryBase', Factory )
	{
		Factory.CheckForDiscoverableVisibility();
	}
}

/** Whether or not a discoverable has already been discovered or not */
final simulated event bool HasFoundDiscoverable( EGearDiscoverableType DiscType )
{
	// If there is no online subsystem (no profile)
	if ( OnlineSub == None || ProfileSettings == None )
	{
		return FALSE;
	}

	return ProfileSettings.HasDiscoverableBeenFound( DiscType );
}

/** Function called by a discoverable to tell the PC that it picked it up */
final function PickedUpDiscoverable( GearDiscoverablesPickupFactoryBase Discoverable )
{
	// Perform the server-side actions that will only happen on the PC who picked up the discoverable
	ServerPickedUpDiscoverable( PlayerReplicationInfo, Discoverable.DISC_DiscoverableType, Discoverable.DISC_PickupType, Discoverable.Location, Discoverable.DISC_PickupSound );

	// Perform actions that will only happen on the PC who picked up the discoverable
	PerformActionsForDiscoverablePickup( Discoverable );
}

// Perform actions that will only happen on the PC who picked up the discoverable
final function PerformActionsForDiscoverablePickup( GearDiscoverablesPickupFactoryBase Discoverable )
{
	ClientPerformDiscoverableActions( Discoverable.DISC_DiscoverableType );
}

/** Perform client-side actions on all GearPCs so they get credit and display what they need to display */
reliable simulated client function ClientPerformDiscoverableActions( EGearDiscoverableType DiscType )
{
	local GearPC PC;

	DiscoverType = DiscType;

	// Mark as having been picked up by all local players
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		// Mark the pickup as found in the profile
		if ( PC.ProfileSettings != None )
		{
			// Set and mark the profile
			PC.ProfileSettings.MarkDiscoverableAsFoundButNotViewed( DiscType, PC );
		}
	}

	// Call the check so the discoverable can be told to hide, but wait for some time so the anim can play
	SetTimer( 0.5f, FALSE, nameof(CheckForDiscoverableVisibility) );

	// Call the UI to get opened after some time
	SetTimer( 1.0f, FALSE, nameof(StartDiscoverableUI) );
}

final function StartDiscoverableUI()
{
	// Have the HUD open the discoverable scene
	if ( MyGearHud != None )
	{
		MyGearHud.OpenDiscoverableScene();
	}
}

/** Function called on the server when a discoverable is picked up */
reliable server function ServerPickedUpDiscoverable( PlayerReplicationInfo PickupPRI, EGearDiscoverableType DiscType, EGearDiscoverablePickupType DiscPUType, vector DiscLocation, SoundCue DiscSoundCue )
{
	local GearPC CurrPC;

	// Send GUDs
	if ( WorldInfo.Game != None )
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpCollectible, MyGearPawn, None, 0.5f);
	}

	// Trigger kismet event
	TriggerEventClass(class'SeqEvent_PickupStatusChange', MyGearPawn, 1);

	// Do wall grab special move
	if ( DiscPUType == eGDPT_Wall )
	{
		MyGearPawn.SpecialMoveLocation = DiscLocation;
		ServerDictateSpecialMove(SM_PushButton);
	}
	// Do pickup special move
	else
	{
		// make sure it doesn't think we're picking up some old other Actor
		GSM_Pickup(MyGearPawn.SpecialMoves[SM_WeaponPickup]).PickupActor = None;
		ServerDictateSpecialMove(SM_WeaponPickup);
	}

	// Play sound
	if ( DiscSoundCue != None )
	{
		PlaySound( DiscSoundCue, FALSE );
	}

	// If there are any networked players, perform the client-side actions
	// on the controllers who did not pick the discoverable up.
	// If this is splitscreen we will not do it.
	if (ShouldReplicateDiscoverable())
	{
		ForEach WorldInfo.AllControllers(class'GearPC', CurrPC)
		{
			if (CurrPC.PlayerReplicationInfo != PickupPRI)
			{
				CurrPC.ClientPerformDiscoverableActions( DiscType );
			}
		}
	}
}

/** Whether we should replicate the discoverable UI to clients or not */
function bool ShouldReplicateDiscoverable()
{
	local GearPC PC;

	// We will replicate to clients if there is a networked one (non-splitscreen)
	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		if (!PC.IsLocalPlayerController())
		{
			// Found a networked player!
			return true;
		}
	}
	return false;
}

function GearPawn FindClosestFriendInRevivingState( GearPawn SourcePawn )
{
	local FLOAT		ClosestDistance, CurrDistance;
	local GearPawn	ClosestGearPawn, GP;

	ClosestDistance = MAXINT;

	foreach WorldInfo.AllPawns(class'GearPawn', GP)
	{
		if ( GP.IsDBNO() )
		{
			if ( (SourcePawn != GP) && WorldInfo.GRI.OnSameTeam(SourcePawn, GP) )
			{
				CurrDistance = VSizeSq(SourcePawn.Location-GP.Location);
				if ( CurrDistance < ClosestDistance )
				{
					ClosestDistance = CurrDistance;
					ClosestGearPawn = GP;
				}
			}
		}
	}

	return ClosestGearPawn;
}

unreliable server function ServerReviveSelf()
{
	// if not MP DM  revive self
	if( /*!WorldInfo.GRI.IsMultiplayerGame() &&*/
	    MyGearPawn != None &&
		MyGearPawn.IsDBNO())
	{
		ServerReviveTeamMate(MyGearPawn);
	}
}


reliable server function ServerReviveTeamMate( GearPawn GP )
{
	//`log(self@GetFuncName()@P);

	if (GP.IsDBNO())
	{
		GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_Revived, Pawn, GP);
	}

	GP.DoRevival(MyGearPawn);
}


/**
 * TurretServerMove()
 * compressed version of server move for PlayerTurreting state
 */
unreliable server function TurretServerMove
(
	float	TimeStamp,
	vector	ClientLoc,
	bool	NewbDuck,
	byte	ClientRoll,
	int		View
)
{
	ServerMove(TimeStamp, Vect(0,0,0), ClientLoc, 0, ClientRoll, View);
}

/* DualTurretServerMove()
compressed version of server move for PlayerTurreting state
*/
unreliable server function DualTurretServerMove
(
	float	TimeStamp0,
	bool	NewbDuck0,
	byte	ClientRoll0,
	int		View0,
	float	TimeStamp,
	vector	ClientLoc,
	bool	NewbDuck,
	byte	ClientRoll,
	int		View
)
{
	ServerMove(TimeStamp0, vect(0,0,0), vect(0,0,0), 0, ClientRoll0, View0);
	ServerMove(TimeStamp, vect(0,0,0), ClientLoc, 0, ClientRoll, View);
}

state PlayerTurreting
{
ignores SeePlayer, HearNoise, Bump;

	unreliable server function TurretServerMove
	(
		float	TimeStamp,
		vector	ClientLoc,
		bool	NewbDuck,
		byte	ClientRoll,
		int		View
	)
	{
		Global.ServerMove(TimeStamp, Vect(0,0,0), ClientLoc, 0, ClientRoll, View);
	}

    function PlayerMove(float DeltaTime)
    {
		if( Pawn == None )
		{
			GotoState('dead');
			return;
		}

		UpdateRotation( DeltaTime );

		if( Role < ROLE_Authority ) // then save this move and replicate it
		{
			ReplicateMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		}
		else
		{
			ProcessMove(DeltaTime, Pawn.Acceleration, DCLICK_None, rot(0,0,0));
		}
    }

	function ProcessMove(float DeltaTime, vector NewAccel, eDoubleClickDir DoubleClickMove, rotator DeltaRot);

    function BeginState(Name PreviousStateName)
    {
		Pawn.MaxPitchLimit = 16384; // extending pitch limit (limits network weapon aiming)
		TriggerGearEventDelegates( eGED_Turret );
    }

    function EndState(Name NextStateName)
    {
		if ( Pawn != None )
		{
			Pawn.MaxPitchLimit = Pawn.default.MaxPitchLimit; // restoring pitch limit
		}
	}

Begin:
}

reliable client final function ClientColorFade(Color FadeColor, byte FromAlpha, byte ToAlpha, float FadeTime)
{
	if (MyGearHud != None)
	{
		MyGearHud.ColorFade( FadeColor, (FromAlpha == 0 ? MyGearHud.FadeAlpha : float(FromAlpha)), ToAlpha, FadeTime );
	}
}

reliable client event ClientSetViewTarget( Actor A, optional ViewTargetTransitionParams TransitionParams )
{
	//`log(WorldInfo.TimeSeconds@GetFuncName()@self@`showvar(A)@`showvar(ViewTarget));
	//ScriptTrace();
	if (A != None || ViewTarget == None)
	{
		Super.ClientSetViewTarget(A,TransitionParams);
	}
	else
	{
`if(`notdefined(FINAL_RELEASE))
		`log(WorldInfo.TimeSeconds@GetFuncName()@self@"ignored null viewtarget"@`showvar(ViewTarget));
`endif
		if( A == None && (WorldInfo.GRI == None || !WorldInfo.GRI.IsMultiplayerGame()))
		{
			ServerVerifyViewTarget();
		}
	}
}

function SetViewTarget( Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams  )
{
	local GearPlayerCamera GearCam;
	local GearSpectatorPoint SpectatorPt;
	local Pawn PawnVT;
	local Actor BattleCamVT;

	//`log(WorldInfo.TimeSeconds@GetFuncName()@self@`showvar(NewViewTarget)@`showvar(ViewTarget));
	//ScriptTrace();

	// intercept transitions to the default camera
	if (Role == ROLE_Authority && Pawn == None && (NewViewTarget == self || NewViewTarget == None) && !IsInState('GhostSpectating'))
	{
		// and replace with battle cam!
		BattleCamVT = LocallySetInitialViewTarget(TRUE);
		if (BattleCamVT != None)
		{
			`log(WorldInfo.TimeSeconds@self@getfuncname()@"Replacing"@`showvar(NewViewTarget)@"with"@`showvar(BattleCamVT));
			if (ViewTarget == BattleCamVT)
			{
				`log("- already viewtarget, ignored redundant call");
				return;
			}
			NewViewTarget = BattleCamVT;
		}
	}

	super.SetViewTarget(NewViewTarget, TransitionParams);

	// if viewing from a spectator point, align controller rot with it
	SpectatorPt = GearSpectatorPoint(NewViewTarget);
	if (SpectatorPt != None)
	{
		// put the display name on the hud
		if (MyGearHud != None)
		{
			CurrSpectatingString = SpectatorPt.DisplayText;
			bPlayerSpectatingOtherPlayer = FALSE;
		}

		// init controller rot to the rot of the spectator point
		SetRotation(SpectatorPt.Rotation);
	}
	else if (IsSpectating())
	{
		PawnVT = Pawn(ViewTarget);
		if (PawnVT != None && PawnVT != Pawn)
		{
			CurrSpectatingString = PawnVT.PlayerReplicationInfo.PlayerName;
			bPlayerSpectatingOtherPlayer = TRUE;
			SpectatingPRI = GearPRI(PawnVT.PlayerReplicationInfo);
		}
		else
		{
			CurrSpectatingString = "";
			bPlayerSpectatingOtherPlayer = FALSE;
			SpectatingPRI = None;
		}
	}

	GearCam = GearPlayerCamera(PlayerCamera);
	if (GearCam != None)
	{
		// remove camera blood when switching viewtargets
		GearCam.ClearCameraLensEffects();
		if (TransitionParams.BlendTime <= 0.f)
		{
			// force immediate camera update
			GearCam.ResetInterpolation();
			GearCam.UpdateCamera(0.0);
		}
	}
}

simulated function bool IsViewingTarget(Actor TargetActor)
{
	if (GetViewTarget() == TargetActor)
	{
		// we don't consider "fixed" cameramode as viewing the target
		if ( (PlayerCamera != None) && (PlayerCamera.CameraStyle != 'Fixed') )
		{
			return TRUE;
		}
	}

	return FALSE;
};

// Possess a pawn
function Possess(Pawn aPawn, bool bVehicleTransition)
{
	MyGearPawn = GearPawn(aPawn);

	//`log(PlayerReplicationInfo.PlayerName$" Possess with old pawn "$Pawn$" targeting "$bIsTargeting$" assess "$bAssessMode);
	//`log(bBreakFromCover@bServerMoveCoverActive@bSuccessfullyTookCover@bSuccessfullyExitedCover@bHoldingAFromRoadieRun@CoverTransitionCountHold@CoverTransitionCountDown);
	bPreciseDestination = false;
	bIgnoreMoveInput = 0;
	bIgnoreLookInput = 0;
	bBreakFromCover = false;
	bServerMoveCoverActive = false;
	CoverTransitionCountHold = 0;
	CoverTransitionCountDown = 0;

	// Remove AI hacks in case a player controller is possessing us (ie dom in split screen!)
	// See GearAI::Possess()
	if( MyGearPawn != None )
	{
		MyGearPawn.bDoWalk2IdleTransitions	= MyGearPawn.default.bDoWalk2IdleTransitions;
		MyGearPawn.bAllowSpeedInterpolation = MyGearPawn.default.bAllowSpeedInterpolation;
		MyGearPawn.Run2CoverMaxDist			= MyGearPawn.default.Run2CoverMaxDist;
		MyGearPawn.bCanJump					= MyGearPawn.default.bCanJump;
		MyGearPawn.bCanCrouch				= MyGearPawn.default.bCanCrouch;
	}

	Super.Possess(aPawn, bVehicleTransition);
}

reliable client function ClientRestart(Pawn NewPawn)
{
	//`log(PlayerReplicationInfo.PlayerName$" ClientRestart with old pawn "$Pawn$" targeting "$bIsTargeting$" assess "$bAssessMode);
	//`log(bBreakFromCover@bServerMoveCoverActive@bSuccessfullyTookCover@bSuccessfullyExitedCover@bHoldingAFromRoadieRun@CoverTransitionCountHold@CoverTransitionCountDown);
	bPreciseDestination = false;
	bIgnoreMoveInput = 0;
	bIgnoreLookInput = 0;
	InputIsDisabledCount = 0;
	bFire = 0;

	bBreakFromCover = false;
	bServerMoveCoverActive = false;
	CoverTransitionCountHold = 0;
	CoverTransitionCountDown = 0;
	ResetPlayerMovementInput();

	Pawn = NewPawn;
	MyGearPawn = GearPawn(NewPawn);
	if ( (Pawn != None) && Pawn.bTearOff )
	{
		UnPossess();
		Pawn = None;
	}
	AcknowledgePossession(Pawn);
	if ( Pawn == None )
	{
		GotoState('WaitingForPawn');
		return;
	}
	if ( class<GearPawn>(NewPawn.class) != None )
	{
		GearPRI(PlayerReplicationInfo).SetPawnClass(class<GearPawn>(NewPawn.class));
	}
	Pawn.ClientRestart();

	`LogTrace( 'Gear_VehicleSystem', "GearPC: Setting ViewTarget for: " $ self $ " to pawn: " $ Pawn ) ;
	if (Role < ROLE_Authority)
	{
		SetViewTarget(Pawn);
		ResetCameraMode();

		EnterStartState();
	}
	CleanOutSavedMoves();

	ClearTimer(nameof(StartDiscoverableUI));

	// if we have a valid HUD and Pawn then force the scoreboard off
	if (MyGearHud != None && MyGearPawn != None)
	{
		MyGearHud.Reset();
		MyGearHud.bShowScores = FALSE;
		// show the tac/com briefly at the beginning of a round
		if (WorldInfo.GRI.IsMultiplayerGame())
		{
			MyGearHud.ShowSquadLocators(3.f);
			// always fade in from black when spawning
			MyGearHud.SpectatorCameraFadeIn();
		}
	}

	GearPlayerInput(PlayerInput).ForcePitchCentering(FALSE);

	//@HACK: failsafe to make sure client doesn't get stuck faded to black due to
	//	matinee/Kismet/camera fighting over the same properties (plus network ordering issues)
	if (WorldInfo.NetMode == NM_Client && WorldInfo.GRI != None && !WorldInfo.GRI.IsMultiplayerGame())
	{
		SetTimer(1.0, true, nameof(CameraFadeVerificationHack));
	}
}

//@HACK: failsafe to make sure client doesn't get stuck faded to black due to
//	matinee/Kismet/camera fighting over the same properties (plus network ordering issues)
final simulated function CameraFadeVerificationHack()
{
	if ( WorldInfo.NetMode == NM_Client && !bCinematicMode && !bInMatinee && PlayerCamera != None &&
		PlayerCamera.bEnableFading && PlayerCamera.FadeAmount >= 1.0 && PlayerCamera.FadeTimeRemaining <= 0.0 )
	{
		ServerVerifyCameraFade();
	}
}
unreliable server function ServerVerifyCameraFade()
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if (PC.PlayerCamera != None && (!PC.PlayerCamera.bEnableFading || PC.PlayerCamera.FadeAmount <= 0.0))
		{
			ClientSetCameraFade(PC.PlayerCamera.bEnableFading, PC.PlayerCamera.FadeColor, PC.PlayerCamera.FadeAlpha, 0.01);
		}
	}
}

/** Overridden to clear any cover claims this pawn might have had. */
function UnPossess()
{
	// unclaim the cover if necessary
	if (MyGearPawn != None &&
		MyGearPawn.CurrentLink != None)
	{
		MyGearPawn.CurrentLink.UnClaim(MyGearPawn,-1,TRUE);
	}
	MyGearPawn = None;
	if ( Pawn != None )
	{
		SetLocation(Pawn.Location);
		Pawn.RemoteRole = ROLE_SimulatedProxy;
		Pawn.UnPossessed();
		CleanOutSavedMoves();  // don't replay moves previous to unpossession
	}
	Pawn = None;
	SetViewTarget(self);
}

event ResetCameraMode()
{
	local GearPlayerCamera GearCam;

	// clear out any shakes when resetting the camera
	GearCam = GearPlayerCamera(PlayerCamera);
	if (GearCam != None)
	{
		GearCam.GearCamMod_ScreenShake.Shakes.length = 0;
	}

	if (Pawn == None)
	{
		SetCameraMode('Default');
	}
	else
	{
		Super.ResetCameraMode();
	}
}

// Kill non-player pawns and their controllers
function KillAllPawns(class<Pawn> aClass)
{
	local Pawn P;

	WorldInfo.Game.KillBots();
	ForEach DynamicActors(class'Pawn', P)
		if ( ClassIsChildOf(P.Class, aClass)
			&& !P.IsPlayerPawn() )
		{
			if ( P.Controller != None )
				P.Controller.Destroy();
			P.Destroy();
		}
}

exec function KillPawns()
{
	KillAllPawns(class'Pawn');
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//@NOTE - DO NOT REMOVE, NEEDED FOR SCRIPTING
exec function KillAll(class<actor> aClass)
{
	local Actor A;
	if ( ClassIsChildOf(aClass, class'AIController') )
	{
		WorldInfo.Game.KillBots();
		return;
	}
	if ( ClassIsChildOf(aClass, class'Pawn') )
	{
		KillAllPawns(class<Pawn>(aClass));
		return;
	}
	ForEach DynamicActors(class 'Actor', A)
		if ( ClassIsChildOf(A.class, aClass) )
			A.Destroy();
}

/**
 * Additional console commands for killing various players, strictly for
 * testing purposes.
 */
exec function Kill(optional string type)
{
	local Controller chkPlayer;
	local GearPawn deadPawn;
	local Vehicle_Reaver_Base deadReaver;
	local Vehicle V;

	local AIController ai;
	if (type ~= "ai" || type ~= "bots" || type ~= "all")
	{
		foreach DynamicActors(class'AIController',ai)
		{
			V = Vehicle(ai.Pawn);
			if( V != None )
			{
				V.DriverLeave( TRUE );
			}

			deadPawn = GearPawn(ai.Pawn);
			if (deadPawn != None)
			{
				deadPawn.ScriptedDeath();
			}
			ai.Destroy();
		}
	}
	else if (type ~= "enemy" || type ~= "enemies" || type ~= "badguys")
	{
		foreach DynamicActors(class'Controller',chkPlayer)
		{
			if (chkPlayer.PlayerReplicationInfo.Team != PlayerReplicationInfo.Team)
			{
				// Handle reaver
				deadReaver = Vehicle_Reaver_Base(chkPlayer.Pawn);
				V = Vehicle(chkPlayer.Pawn);

				if( deadReaver != None)
				{
					deadReaver.VehicleScriptedDeath();
				}
				else if( V != None )
				{
					V.DriverLeave( TRUE );
				}

				deadPawn = GearPawn(chkPlayer.Pawn);
				if (deadPawn != None)
				{
					deadPawn.ScriptedDeath();
				}
				chkPlayer.Destroy();
			}
		}
	}
	else if( type ~= "friend" || type ~= "friends" || type ~= "goodguys" || type ~= "friendlies" )
	{
		foreach DynamicActors( class'Controller', chkPlayer )
		{
			if( chkPlayer != self && chkPlayer.PlayerReplicationInfo.Team == PlayerReplicationInfo.Team )
			{
				V = Vehicle(chkPlayer.Pawn);
				if( V != None )
				{
					V.DriverLeave( TRUE );
				}

				deadPawn = GearPawn(chkPlayer.Pawn);
				if( deadPawn != None )
				{
					chkPlayer.bGodMode = FALSE;
					deadPawn.TakeDamage(deadPawn.Health+1, None, deadPawn.Location, vect(0,0,0), class'GDT_ScriptedRagdoll');
					//deadPawn.Died( self, class'DamageType', deadPawn.Location + vect(0,0,32) );
					//deadPawn.Destroy();
				}
				//chkPlayer.Destroy();
			}
		}
	}
	else
	{
		foreach DynamicActors( class'Controller', chkPlayer )
		{
			if( chkPlayer != self &&
				(String(chkPlayer.Name) ~= type ||
				(chkPlayer.Pawn != None && String(chkPlayer.Pawn.Name) ~= type)) )
			{
				deadPawn = GearPawn(chkPlayer.Pawn);
				if( deadPawn != None )
				{
					deadPawn.ScriptedDeath();
				}
				chkPlayer.Destroy();
			}
		}
	}
}

reliable client function ClientResetCameraInterpolation()
{
	GearPlayerCamera(PlayerCamera).ResetInterpolation();
	StopCameraBoneAnim(0.f, FALSE);
}

// Player Driving a vehicle.
state PlayerDriving
{
	ignores SeePlayer, HearNoise, Bump;

	function BeginState( Name PreviousStateName )
	{
		//`log( "Gear PlayerDriving State" );
		GearPlayerCamera(PlayerCamera).ResetInterpolation();
		Super.EndState( PreviousStateName );

		// Reset gas pedal parameters to 0.0
		VehicleGasPressedAmount = 0.0f;
		VehicleReversePressedAmount = 0.0f;
	}


	function EndState( Name NextStateName )
	{
		Super.EndState( NextStateName );
	}

	function PlayerTick( float DeltaTime )
	{
		local float JRight, JUp;
		local GearVehicleBase GearV;
		local Name DesiredSoundMode;
		local bool bNewIsTargeting;

		local GearGameplayCamera Cam;
		Cam = GearPlayerCamera(PlayerCamera).GameplayCam;

		// Checks to see if the profile settings should update the player settings
		CheckForProfileUpdate();

		if (Cam != None && !WorldInfo.bPlayersOnly)
		{
			JRight = PlayerInput.RawJoyLookRight;
			JUp = PlayerInput.RawJoyLookUp;

			// pass arong right stick values to the camera
			if (JRight > 0.7f)
			{
				Cam.DirectLookYaw = 16384;
			}
			else if (JRight < -0.7f)
			{
				Cam.DirectLookYaw = -16384;
			}
			else if (JUp < -0.7f)
			{
				Cam.DirectLookYaw = 32768;
			}
			else
			{
				Cam.DirectLookYaw = 0;
			}
		}

		UpdateHUDActions(DeltaTime);

		GearV = GearVehicleBase(Pawn);
		if (GearV != None && GearV.IsLocallyControlled())
		{
			bNewIsTargeting = GearV.AllowTargetting() && WantsToTarget();
			if( bIsTargeting != bNewIsTargeting )
			{
				SetTargetingMode(bNewIsTargeting);
			}
		}

		// Need to do this so we can do aim-assist while aiming in a vehicle
		MaintainEnemyList();

		DesiredSoundMode = FindBestSoundMode();
		if (DesiredSoundMode != CurrentSoundMode)
		{
			SetSoundMode(DesiredSoundMode);
		}

		// @fixme: this won't eventually call the global GearPC.PlayerTick!
		Super.PlayerTick(DeltaTime);
	}

	function ProcessViewRotation( float DeltaTime, out Rotator out_ViewRotation, Rotator DeltaRot )
	{
		local GearVehicle		MyGearVehicle;
		local GearWeaponPawn	MyWeapPawn;
		local Rotator		BaseRot, DesiredViewRotation, LocalViewRot;
		local int			CamCenterDelta;
		local bool			bPassenger;
		local quat			BaseQuat, RelViewQuat, ViewQuat;
		local bool			bVehicleSpaceCam;

		// If in passenger seat - look to main vehicle for cam stuff
		MyWeapPawn = GearWeaponPawn(Pawn);
		if(MyWeapPawn != None)
		{
			MyGearVehicle = MyWeapPawn.MyVehicle;
			bPassenger = TRUE;
		}
		else
		{
			MyGearVehicle = GearVehicle(Pawn);
		}

		// Look at relevant 'vehicle space cam' flag.
		if(MyGearVehicle != None)
		{
			bVehicleSpaceCam = bPassenger ? MyGearVehicle.bPassengerVehicleSpaceCamera : MyGearVehicle.bVehicleSpaceCamera;
		}

		if(MyGearVehicle != None && !MyGearVehicle.bUsingLookSteer && bVehicleSpaceCam)
		{
			if(MyGearVehicle.ShouldCenterCamSpaceCamera())
			{
				// avoid overshoot
				if(RelativeToVehicleViewRot.Yaw > 0)
				{
					CamCenterDelta = -(DeltaTime * MyGearVehicle.VehicleSpaceCamCenterSpeed);
					CamCenterDelta = Max(CamCenterDelta, -RelativeToVehicleViewRot.Yaw);
				}
				else if(RelativeToVehicleViewRot.Yaw < 0)
				{
					CamCenterDelta = (DeltaTime * MyGearVehicle.VehicleSpaceCamCenterSpeed);
					CamCenterDelta = Min(CamCenterDelta, -RelativeToVehicleViewRot.Yaw);
				}

				RelativeToVehicleViewRot.Yaw += CamCenterDelta;
			}
			else
			{
				// Add Delta Rotation
				RelativeToVehicleViewRot += DeltaRot;

				// If desired, clamp yaw of cam relative to vehicle
				if(MyGearVehicle.MaxVehicleSpaceCamYaw != 0)
				{
					RelativeToVehicleViewRot.Yaw = Clamp(RelativeToVehicleViewRot.Yaw, -MyGearVehicle.MaxVehicleSpaceCamYaw, MyGearVehicle.MaxVehicleSpaceCamYaw);
				}
			}

			// Make sure in sensible range
			RelativeToVehicleViewRot = Normalize(RelativeToVehicleViewRot);

			// Limit pitch
			RelativeToVehicleViewRot = LimitViewRotation( RelativeToVehicleViewRot, MyGearVehicle.ViewPitchMin, MyGearVehicle.ViewPitchMax );

			BaseRot = MyGearVehicle.GetVehicleSpaceCamRotation(DeltaTime, bPassenger);

			if(MyGearVehicle.bOnlyInheritVehicleSpaceYaw)
			{
				BaseRot.Pitch = 0;
				BaseRot.Roll = 0;
				DesiredViewRotation = BaseRot + RelativeToVehicleViewRot;
			}
			else
			{
				BaseQuat = QuatFromRotator(BaseRot);
				RelViewQuat = QuatFromRotator(RelativeToVehicleViewRot);
				ViewQuat = QuatProduct(BaseQuat, RelViewQuat);
				DesiredViewRotation = QuatToRotator(ViewQuat);
			}

			if(MyGearVehicle.VehicleSpaceCamBlendTime <= 0.0)
			{
				out_ViewRotation = DesiredViewRotation;
			}
			else
			{
				out_ViewRotation = RInterpTo(LastVehicleSpaceViewRotation, DesiredViewRotation, DeltaTime, 1.0/MyGearVehicle.VehicleSpaceCamBlendTime);
			}
			LastVehicleSpaceViewRotation = out_ViewRotation;

			DeltaRot = rot(0,0,0);
		}
		else if(MyGearVehicle != None && MyGearVehicle.bVehicleSpaceViewLimits)
		{
			// First, update view rotation
			out_ViewRotation += DeltaRot;

			// Get vehicle ref frame
			BaseRot = MyGearVehicle.GetVehicleSpaceCamRotation(DeltaTime, bPassenger);
			BaseQuat = QuatFromRotator(BaseRot);

			// Transform rotator into vehicle space
			ViewQuat = QuatFromRotator(out_ViewRotation);
			RelViewQuat = QuatProduct(QuatInvert(BaseQuat), ViewQuat);
			LocalViewRot = QuatToRotator(RelViewQuat);

			LocalViewRot.Roll = 0;

			// Limit yaw (vehicle space)
			MyGearVehicle.bPushingAgainstViewLimit = FALSE;
			if(MyGearVehicle.MaxVehicleSpaceCamYaw != 0)
			{
				// If beyond lower limit
				if( LocalViewRot.Yaw < -MyGearVehicle.MaxVehicleSpaceCamYaw )
				{
					// On owning client - And pushing left - set 'pushing' flag
					if(LocalPlayer(Player) != None && PlayerInput.RawJoyLookRight < -0.1)
					{
						MyGearVehicle.bPushingAgainstViewLimit = TRUE;
						// keep pitch constant when against the limit
						LocalViewRot.Pitch = LastVehicleSpaceViewRotation.Pitch;
					}
					LocalViewRot.Yaw = -MyGearVehicle.MaxVehicleSpaceCamYaw;
				}
				// If beyond upper limit
				else if( LocalViewRot.Yaw	> MyGearVehicle.MaxVehicleSpaceCamYaw )
				{
					// On owning client - And pushing right - set 'pushing' flag
					if(LocalPlayer(Player) != None && PlayerInput.RawJoyLookRight > 0.1)
					{
						MyGearVehicle.bPushingAgainstViewLimit = TRUE;
						LocalViewRot.Pitch = LastVehicleSpaceViewRotation.Pitch;
					}
					LocalViewRot.Yaw = MyGearVehicle.MaxVehicleSpaceCamYaw;
				}
			}

			// Limit pitch (vehicle space)
			LocalViewRot = LimitViewRotation( LocalViewRot, MyGearVehicle.ViewPitchMin, MyGearVehicle.ViewPitchMax );
			// Save old rotation in case we want to keep pitch constant
			LastVehicleSpaceViewRotation = LocalViewRot;

			// Transform rotator into world space
			RelViewQuat = QuatFromRotator(LocalViewRot);
			ViewQuat = QuatProduct(BaseQuat, RelViewQuat);
			out_ViewRotation = QuatToRotator(ViewQuat);
			out_ViewRotation.Roll = 0;
		}
		else
		{
			Super.ProcessViewRotation(DeltaTime, out_ViewRotation, DeltaRot);
		}
	}

	function PlayerMove( float DeltaTime )
	{
		local GearVehicle	MyGearVehicle;
		local FLOAT			Forward, Strafe, Up;

		// update 'looking' rotation
		UpdateRotation(DeltaTime);

		// Remap input
		MyGearVehicle = GearVehicle(Pawn);
		if( MyGearVehicle != None )
		{
			MyGearVehicle.RemapPlayerInput(Self, Forward, Strafe, Up);
		}

		// TODO: Don't send things like aForward and aStrafe for gunners who don't need it
		// Only servers can actually do the driving logic.
		ProcessDrive(Forward, Strafe, Up, bPressedJump);
		if (Role < ROLE_Authority)
		{
			ServerDrive(Forward, Strafe, Up, bPressedJump, ((Rotation.Yaw & 65535) << 16) + (Rotation.Pitch & 65535));
		}

		bPressedJump = FALSE;
	}
}


/* epic ===============================================
* ::CheckNearMiss
*
* Check if bullet went close by my pawn.
*
* =====================================================
*/
function CheckNearMiss(Pawn Shooter, GearWeapon GearWeap, const out ImpactInfo Impact, class<DamageType> InDamageType)
{
	local FLOAT		NearMissDist;
	local Vector	ClosestPoint;

	// Find closest distance to segment bullet travelled.
	NearMissDist = PointDistToSegment(MyGearPawn.Location, Impact.StartTrace, Impact.HitLocation, ClosestPoint);

	// Play bullet whip sounds
	if( (NearMissDist < Player_BulletWhipDistance) && (MyGearPawn.LastBulletWhipTime + GearWeap.MinTimeBetweenBulletWhips) < WorldInfo.TimeSeconds )
	{
		// only play the sound on the clients
		if( WorldInfo.NetMode != NM_DedicatedServer && Impact.HitActor != MyGearPawn )
		{
			//`log( "Should Hear a Whip sound" @ GearWeapon(W).WeaponWhipSound );
			// only play a whip sound if the bullet flew past you and
			// if it did NOT impact something
			WorldInfo.PlaySound(GearWeap.WeaponWhipSound, false, true,, ClosestPoint);
		}

		MyGearPawn.LastBulletWhipTime = WorldInfo.TimeSeconds;
	}

	// Let our Pawn know we got a near miss.
	// Cringes
	if( NearMissDist < Player_NearMissDistance )
	{
		MyGearPawn.RegisterNearHitMiss(Impact, InDamageType);
	}
}


/** Toggle animation debugging. */
exec function DebugAnimation()
{
	local GearPC	PC;

	FlushPersistentDebugLines();

	// Toggle debug for all local players
	ForEach WorldInfo.AllControllers(class'GearPC', PC)
	{
		// Toggle
		PC.bDebugAnimation = !PC.bDebugAnimation;

		// and hook/unhook our debug draw
		PC.MyGearHud.SetDebugDraw(DebugDraw_Animation, PC.bDebugAnimation);
	}
}

function DrawDebugText(GearHUD_Base H, bool bShadowPass, String Text)
{
	local float XL, YL;

	if( H.Canvas.CurY >= H.Canvas.SizeY * 0.95f )
	{
		H.Canvas.SetPos(H.Canvas.CurX + DebugTextMaxLen + 8.f, H.Canvas.SizeY * 0.05f);
		DebugTextMaxLen = 0.f;

		if( bShadowPass )
		{
			H.Canvas.SetPos(H.Canvas.CurX, H.Canvas.CurY + 1.f);
		}
	}

	H.Canvas.StrLen(Text, XL, YL);
	if( XL > DebugTextMaxLen )
	{
		DebugTextMaxLen = XL;
	}

	H.Canvas.DrawText(Text);
}


function DebugDraw_Animation(GearHUD_Base H)
{
	//DebugDraw_Animation_Worker(H, TRUE);
	DebugDraw_Animation_Worker(H, FALSE);
}

function DebugDraw_Animation_Worker(GearHUD_Base H, bool bShadowPass)
{
	local Canvas							Canvas;
	local GearPawn							DebugPawn;
	local AnimNode							Node;
	local GearAnim_BlendAnimsByDirection	BlendByDirNode;
	local GearAnim_UpperBodyIKHack			IKHackNode;
	local int								i;
	local FLOAT								XO, YO, XD, YD;
	local SkelControlBase					SkelControl;
	local String							Text;
	local AnimNodeSequence					SeqNode;
	local AnimTree.AnimGroup				AnimGroup;

	Canvas = H.Canvas;
	if( PlayerCamera != None && PlayerCamera.ViewTarget.Target != None )
	{
		DebugPawn = GearPawn(PlayerCamera.ViewTarget.Target);
		if( DebugPawn == None && Controller(PlayerCamera.ViewTarget.Target) != None )
		{
			DebugPawn = GearPawn(Controller(PlayerCamera.ViewTarget.Target).Pawn);
		}
	}
	else
	{
		DebugPawn = MyGearPawn;
	}

	Canvas.Font = class'Engine'.Static.GetTinyFont();
	Canvas.SetDrawColor(0, 255, 0, 255);
	Canvas.SetPos(Canvas.SizeX * 0.05f, Canvas.SizeY * 0.05f);

	if( bShadowPass )
	{
		Canvas.SetPos(Canvas.CurX + 1.f, Canvas.CurY + 1.f);
		Canvas.SetDrawColor(0, 0, 0, 255);
	}

	DebugTextMaxLen = 0.f;

	DrawDebugText(H, bShadowPass, "* AnimNodeSequences *");
	ForEach DebugPawn.Mesh.AllAnimNodes(class'AnimNodeSequence', SeqNode)
	{
		// Skip non relevant nodes
		if( SeqNode.NodeTotalWeight < 0.00001f )
		{
			continue;
		}

		if( !bShadowPass )
		{
			Canvas.SetDrawColor(255, 0, 255, 255);
		}
		DrawDebugText(H, bShadowPass, SeqNode.Class @ SeqNode.AnimSeqName @ "Weight:" @ int(SeqNode.NodeTotalWeight*100.f) @ "Pos:" @ int(SeqNode.GetNormalizedPosition()*100.f));
	}

	Canvas.SetDrawColor(0, 255, 0, 255);
	DrawDebugText(H, bShadowPass, "");
	DrawDebugText(H, bShadowPass, "* AnimTree *");

	ForEach DebugPawn.Mesh.AllAnimNodes(class'AnimNode', Node)
	{
		// Skip non relevant nodes
		if( Node.NodeTotalWeight < 0.00001f )
		{
			if( !bShadowPass )
			{
				Canvas.SetDrawColor(255, 0, 0, 255);
			}
			continue;
		}
		else if( !bShadowPass )
		{
			Canvas.SetDrawColor(0, 255, 0, 255);
		}

		SeqNode = AnimNodeSequence(Node);
		if( SeqNode != None )
		{
			if( !bShadowPass )
			{
				Canvas.SetDrawColor(255, 0, 255, 255);
			}
			DrawDebugText(H, bShadowPass, Node.Class @ SeqNode.AnimSeqName @ "Weight:" @ int(Node.NodeTotalWeight*100.f) );
		}
		else
		{
			Text = ((Node.NodeName != '' && Node.NodeName != 'None') ? Node.Class @ Node.NodeName : String(Node.Class));
			DrawDebugText(H, bShadowPass, Text @ "Weight:" @ int(Node.NodeTotalWeight*100.f) );
		}

		BlendByDirNode = GearAnim_BlendAnimsByDirection(Node);
		if( BlendByDirNode != None )
		{
			DrawDebugText(H, bShadowPass, "  DirAngle:" @ int(BlendByDirNode.DirAngle*100) @ BlendByDirNode.AnimSeqName );
			/*
			for(i=0; i<BlendByDirNode.Anims.Length; i++)
			{
				DrawDebugText(H, "    " @ i @ int(BlendByDirNode.Anims[i].Weight * 100) @ BlendByDirNode.Anims[i].AnimName);
			}
			*/

			if( !bShadowPass )
			{
				// Draw Direction on a circle.
				XO = Canvas.CurX + Canvas.SizeX * 0.1f;
				YO = Canvas.CurY - Canvas.SizeY * 0.1f;
				XD = XO - 32 * Sin(BlendByDirNode.DirAngle);
				YD = YO - 32 * Cos(BlendByDirNode.DirAngle);
				Canvas.Draw2DLine(XO, YO, XD, YD, MakeColor(255, 0, 0, 255));
			}
		}

		IKHackNode = GearAnim_UpperBodyIKHack(Node);
		if( IKHackNode != None )
		{
			DrawDebugText(H, bShadowPass, "  bNodeDisabled:" @ IKHackNode.bNodeDisabled);
		}
	}

	if( !bShadowPass )
	{
		Canvas.SetDrawColor(0, 255, 0, 255);
	}

	H.Canvas.SetPos(H.Canvas.CurX + DebugTextMaxLen + 8.f, H.Canvas.SizeY * 0.05f);
	DebugTextMaxLen = 0.f;
	if( bShadowPass )
	{
		H.Canvas.SetPos(H.Canvas.CurX, H.Canvas.CurY + 1.f);
	}

	DrawDebugText(H, bShadowPass, "* Bone Controllers *");
	if( DebugPawn.AnimTreeRootNode != None )
	{
		for(i=0; i<DebugPawn.AnimTreeRootNode.SkelControlLists.Length; i++)
		{
			SkelControl = DebugPawn.AnimTreeRootNode.SkelControlLists[i].ControlHead;

			if( SkelControl == None )
			{
				continue;
			}

			// Bone name
			if( !bShadowPass )
			{
				Canvas.SetDrawColor(0, 255, 0, 255);
			}
			DrawDebugText(H, bShadowPass, String(DebugPawn.AnimTreeRootNode.SkelControlLists[i].BoneName) );

			while( SkelControl != None )
			{
				if( !bShadowPass )
				{
					if( SkelControl.ControlStrength < 0.00001f )
					{
						Canvas.SetDrawColor(255, 0, 0, 255);
					}
					else
					{
						Canvas.SetDrawColor(0, 255, 0, 255);
					}
				}

				if( TRUE ) //SkelControl.ControlStrength > 0.00001f )
				{
					DrawDebugText(H, bShadowPass, "  " @ SkelControl.Class @ "Name:" @ SkelControl.ControlName @ "Weight:" @ int(SkelControl.ControlStrength*100.f) );
				}

				SkelControl = SkelControl.NextControl;
			}
		}
	}

	H.Canvas.SetPos(H.Canvas.CurX + DebugTextMaxLen + 8.f, H.Canvas.SizeY * 0.05f);
	DebugTextMaxLen = 0.f;
	if( bShadowPass )
	{
		H.Canvas.SetPos(H.Canvas.CurX, H.Canvas.CurY + 1.f);
	}

	DrawDebugText(H, bShadowPass, "* Anim Groups *");

	if( DebugPawn.AnimTreeRootNode != None )
	{
		for(i=0; i<DebugPawn.AnimTreeRootNode.AnimGroups.Length; i++)
		{
			AnimGroup = DebugPawn.AnimTreeRootNode.AnimGroups[i];
			if( !bShadowPass )
			{
				if( AnimGroup.SynchMaster == None || AnimGroup.SynchMaster.NodeTotalWeight < 0.00001f )
				{
					Canvas.SetDrawColor(255, 0, 0, 255);
				}
				else
				{
					Canvas.SetDrawColor(0, 255, 0, 255);
				}
			}

			DrawDebugText(H, bShadowPass, AnimGroup.GroupName @ "RateScale:" @ AnimGroup.RateScale @ "NumNodes:" @ AnimGroup.SeqNodes.Length @ "Pos:" @ DebugPawn.AnimTreeRootNode.GetGroupRelativePosition(AnimGroup.GroupName));
			if( AnimGroup.SynchMaster != None )
			{
				DrawDebugText(H, bShadowPass, "  SynchMaster:" @ AnimGroup.SynchMaster @ "Weight:" @ int(AnimGroup.SynchMaster.NodeTotalWeight*100.f));
			}
			else
			{
				DrawDebugText(H, bShadowPass, "  SynchMaster: None");
			}

			if( AnimGroup.NotifyMaster != None )
			{
				DrawDebugText(H, bShadowPass, "  NotifyMaster:" @ AnimGroup.NotifyMaster @ "Weight:" @ int(AnimGroup.NotifyMaster.NodeTotalWeight*100.f));
			}
			else
			{
				DrawDebugText(H, bShadowPass, "  NotifyMaster: None");
			}
		}
	}

	if( DebugPawn.InteractionPawn != None )
	{
		DrawDebugText(H, bShadowPass, "* InteractionPawn AnimGroups *");

		DebugPawn = DebugPawn.InteractionPawn;
		for(i=0; i<DebugPawn.AnimTreeRootNode.AnimGroups.Length; i++)
		{
			AnimGroup = DebugPawn.AnimTreeRootNode.AnimGroups[i];
			if( !bShadowPass )
			{
				if( AnimGroup.SynchMaster == None || AnimGroup.SynchMaster.NodeTotalWeight < 0.00001f )
				{
					Canvas.SetDrawColor(255, 0, 0, 255);
				}
				else
				{
					Canvas.SetDrawColor(0, 255, 0, 255);
				}
			}

			DrawDebugText(H, bShadowPass, AnimGroup.GroupName @ "RateScale:" @ AnimGroup.RateScale @ "NumNodes:" @ AnimGroup.SeqNodes.Length @ "Pos:" @ DebugPawn.AnimTreeRootNode.GetGroupRelativePosition(AnimGroup.GroupName) );
			if( AnimGroup.SynchMaster != None )
			{
				DrawDebugText(H, bShadowPass, "  SynchMaster:" @ AnimGroup.SynchMaster @ "Weight:" @ int(AnimGroup.SynchMaster.NodeTotalWeight*100.f));
			}
			else
			{
				DrawDebugText(H, bShadowPass, "  SynchMaster: None");
			}

			if( AnimGroup.NotifyMaster != None )
			{
				DrawDebugText(H, bShadowPass, "  NotifyMaster:" @ AnimGroup.NotifyMaster @ "Weight:" @ int(AnimGroup.NotifyMaster.NodeTotalWeight*100.f));
			}
			else
			{
				DrawDebugText(H, bShadowPass, "  NotifyMaster: None");
			}
		}
	}
}

/** toggle cover code debugging */
exec function DebugCover()
{
	local GearPC	PC;

	FlushPersistentDebugLines();

	// Toggle debug for all local players
	ForEach WorldInfo.AllControllers(class'GearPC', PC)
	{
		// toggle
		PC.bDebugCover = !bDebugCover;

		PC.SetBasicCoverDebug(GearPawn(PC.Pawn), PC.bDebugCover);

		// and hook/unhook our debug draw
		PC.MyGearHud.SetDebugDraw(DebugDraw_Cover, PC.bDebugCover);
	}
}


/** Special version for MP, logs only */
exec function MPDebugCover()
{
	bDebugCover = !bDebugCover;

	ServerDebugCover(bDebugCover);
}


reliable server function ServerDebugCover(bool bNewDebugCover)
{
	bDebugCover = bNewDebugCover;
}

exec function DebugWeapon()
{
	if( MyGearPawn != None )
	{
		MyGearPawn.bWeaponDebug_Accuracy = !MyGearPawn.bWeaponDebug_Accuracy;
	}
}

/** Set basic cover debugging flags */
function SetBasicCoverDebug(GearPawn P, bool bFlag)
{
	if( P != None )
	{
		P.bCoverDebug_PlayerFOV			= bFlag;
		P.bCoverDebug_CoverFOV			= bFlag;
		P.bCoverDebug_CoverVolume		= bFlag;
		P.bCoverDebug_FoundCover		= bFlag;
		P.bCoverDebug_ValidatedCover	= bFlag;
		P.bCoverDebug_ConsideredLinks	= bFlag;
	}
}

function DebugDraw_Cover(GearHUD_Base H)
{
	local vector	CamLoc;
	local rotator	CamRot;
	local Canvas	Canvas;
	local GearPawn	WP;
	local string	Text;
	local float		FOVAngle1;
	local CoverSlot	Slot;

	Canvas = H.Canvas;
	GetPlayerViewPoint(CamLoc, CamRot);

	if( PlayerCamera != None && PlayerCamera.ViewTarget.Target != None )
	{
		WP	= GearPawn(PlayerCamera.ViewTarget.Target);
		if( WP == None && Controller(PlayerCamera.ViewTarget.Target) != None )
		{
			WP	= GearPawn(Controller(PlayerCamera.ViewTarget.Target).Pawn);
		}
	}
	else
	{
		WP = MyGearPawn;
	}


	if( WP != None )
	{
		DrawDebugCoordinateSystem(WP.Location+(vect(0,22,42) >> (WP.Rotation)),CamRot,16.f,FALSE);
		DrawDebugCoordinateSystem(WP.Location+(vect(0,-22,42) >> (WP.Rotation)),ControlsRemapRotation,16.f,FALSE);

		// Update cover debug flags
		SetBasicCoverDebug(WP, bDebugCover);

		if( IsInCoverState() )
		{
			// draw cover protection FOV
			FOVAngle1 = Sin(WP.CoverProtectionFOV.Y*Pi/180.f);
			DrawDebugCone(WP.Location, vector(WP.Rotation), 200.f, FOVAngle1, FOVAngle1, 16, MakeColor(255,192,64,255), FALSE);
		}

		Canvas.Font = class'Engine'.Static.GetSmallFont();
		Canvas.SetDrawColor(255, 0, 255, 255);

		Text = "Pawn CoverType:" @ WP.CoverType @ "CoverAction:" @ WP.CoverAction @ "CoverDirection:" @ WP.CoverDirection @ "At LeftEdge:"@ WP.IsAtCoverEdge(TRUE) $ "/" $ WP.IsAtCoverEdge(TRUE,TRUE) @ "At RightEdge:"@WP.isAtCoverEdge(FALSE) $ "/" $ WP.IsAtCoverEdge(FALSE,TRUE);
		Canvas.SetPos( 10, Canvas.SizeY * 0.1 );
		Canvas.DrawText(Text);

		Text = "Link:" @ WP.CurrentLink @ "bIsInStationaryCover:" @ WP.bIsInStationaryCover @ "CurrentSlotDirection:" $ WP.CurrentSlotDirection;
		Canvas.DrawText(Text);

		Text = "CurrentSlotIdx:" @ WP.CurrentSlotIdx @ "LeftSlotIdx:" @ WP.LeftSlotIdx @ "RightSlotIdx:" @ WP.RightSlotIdx @ "CurrentSlotPct:" @ WP.CurrentSlotPct @ "Closest:" @ WP.ClosestSlotIdx;
		Canvas.DrawText(Text);

		if( WP.CurrentLink != None )
		{
			if (WP.LeftSlotIdx >= 0 && WP.LeftSlotIdx < WP.CurrentLink.Slots.Length)
			{
				Slot = WP.CurrentLink.Slots[WP.LeftSlotIdx];
				Text = " L - bEnabled:" @ Slot.bEnabled @ "bLeanLeft:" @ Slot.bLeanLeft @ "bLeanRight:" @ Slot.bLeanRight @ "Mantle:" @ Slot.bCanMantle;
				Canvas.DrawText(Text);
			}

			if (WP.RightSlotIdx >= 0 && WP.RightSlotIdx < WP.CurrentLink.Slots.Length)
			{
				Slot = WP.CurrentLink.Slots[WP.RightSlotIdx];
				Text = " R - bEnabled:" @ Slot.bEnabled @ "bLeanLeft:" @ Slot.bLeanLeft @ "bLeanRight:" @ Slot.bLeanRight @ "Mantle:" @ Slot.bCanMantle;
				Canvas.DrawText(Text);
			}
		}
	}

	// joy move
	Text = "JoyRight:"$PlayerInput.RawJoyRight$"/"$RemappedJoyRight @ "JoyUp:"$PlayerInput.RawJoyUp$"/"$RemappedJoyUp;
	Canvas.DrawText( Text );

	// joy look
	Text = "RawJoyLookRight:"$PlayerInput.RawJoyLookRight$"/"$PlayerInput.aTurn @ "RawJoyLookUp:"$PlayerInput.RawJoyLookUp $"/"$PlayerInput.aLookUp;
	Canvas.DrawText( Text );

	if (WP != None && WP.Mesh != None)
	{
		Canvas.DrawText("Root motion mode:"@WP.Mesh.RootMotionMode);
		Canvas.DrawText("Precise destination?:"@bPreciseDestination@"Physics:"@WP.Physics@"Collide World?:"@WP.bCollideWorld);
	}
}


/** toggle ai debugging */
exec function DebugAI(optional coerce name Category)
{
	local int i;

	if(Category == '')
	{
		Category = 'Default';
	}

	if(Category == 'none')
	{
		GearGame(WorldInfo.Game).AIVisMan.bDrawVisTests = false;
		DebugAICategories.length=0;
		bDebugAI=false;
	}
	else
	{
		if(Category == 'sighttests')
		{
			GearGame(WorldInfo.Game).AIVisMan.bDrawVisTests = !GearGame(WorldInfo.Game).AIVisMan.bDrawVisTests;
		}

		i = DebugAICategories.Find(Category);
		if(i > -1)
		{
			ClientMessage( "DebugAI category"@Category@"disabled." );
			// if it's in the list already, then toggle it off
			DebugAICategories.Remove(i,1);
			bDebugAI=(DebugAICategories.length > 0);
		}
		else
		{
			ClientMessage( "DebugAI category"@Category@"enabled." );
			DebugAICategories.AddItem(Category);
			bDebugAI=true;
		}
	}

	// and hook/unhook our debug draw
	MyGearHud.SetDebugDraw(DebugDraw_AI,bDebugAI);

}


function DebugDraw_AI(GearHUD_Base H)
{
	local vector CamLoc;
	local rotator CamRot;
	local GearAI AI;
	local String Str;
	local float	X, Y;
	local int i;
	local Vector LeaderLoc;

	H.Canvas.SetDrawColor(255,255,255);
	GetPlayerViewPoint(CamLoc,CamRot);
	foreach WorldInfo.AllControllers(class'GearAI', AI)
	{
		if (AI.Pawn != None && (AI.Pawn.Location - CamLoc) dot vector(CamRot) > 0.f)
		{
			for(i=0;i<DebugAICategories.length;i++)
			{
				AI.DrawDebug(H,DebugAICategories[i]);
			}
		}
	}

	if( Squad != None && Squad.Leader == self )
	{
		if( Squad.SquadRoute != None )
		{
			LeaderLoc = Squad.GetSquadLeaderLocation();
			if(Pawn != none)
			{
				DrawDebugLine( Pawn.Location, LeaderLoc, 255, 128, 255 );
			}
			DrawDebugCylinder( LeaderLoc, LeaderLoc, 128.f, 10, 255, 128, 255 );
		}
	}

	X = H.SizeX - 160;
	Y = H.SizeY - 20;
	H.Canvas.SetPos( X, Y );

	Str = "Frame Time:"@WorldInfo.TimeSeconds;
	H.Canvas.DrawText( Str, TRUE );
}


/** Turns on MP Debugging **/
exec simulated function DebugMP()
{
	bShowDebugMP = !bShowDebugMP;
	MyGearHud.SetDebugDraw( DebugDraw_MP, bShowDebugMP );
}

/** This will draw stats we care about in MP on each of the pawns in our view **/
simulated function DebugDraw_MP( GearHUD_Base H )
{
	local vector CamLoc;
	local rotator CamRot;
	local GearPAwn GP;

	H.Canvas.SetDrawColor(255,255,255);
	GetPlayerViewPoint(CamLoc,CamRot);

	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		if( GP != None && (GP.Location - CamLoc) dot vector(CamRot) > 0.0f )
		{
			GP.DrawDebug_MP( H, Pawn );
		}
	}
}


/**
 * Overridden to hook any debug draw functions.
 */
reliable client function ClientSetHUD(class<HUD> newHUDType, class<Scoreboard> newScoringType)
{
	//local color FadeColor;

	Super.ClientSetHUD(newHUDType, newScoringType);

	MyGearHud = GearHud_Base(MyHud);
	if (MyGearHud != None)
	{
		MyGearHud.SetDebugDraw(DebugDraw_Cover,bDebugCover);
		MyGearHud.SetDebugDraw(DebugDraw_AI,bDebugAI);
		RefreshAllSafeZoneViewports();
	}
}



/**
 * Try to find a nearby target to adhere to, used for melee attacks.  See also
 * StopMeleeAdhesion.
 * Returns Pawn we decided to adhere to, or None if there wasn't one
 */
simulated function GearPawn AttemptMeleeAdhesion()
{
	local GearPawn	TestWP;
	local vector	ToTestWPNorm;
	local float		ToTestWPDist;
	local GearPawn	BestTarget;
	local float		BestTargetScore, DotToTarget;
	local float		DistScore, DirScore, TotalScore, SearchRadius;

	SearchRadius = MyGearPawn.GetMeleeAttackRange();

	BestTargetScore = -99999.f;
	BestTarget = None;

	// @fixme, increase range a bit in case we are moving towards enemy?
	foreach MyGearPawn.CollidingActors( class'GearPawn', TestWP, SearchRadius )
	{
		//`log( "AttemptMeleeAdhesion TestWP: " $ TestWP );

		if( MyGearPawn.TargetIsValidMeleeTarget( TestWP, FALSE ) )
		{
			// get mag and normalize with only 1 sqrt
			ToTestWPNorm = TestWP.Location - MyGearPawn.Location;
			ToTestWPDist = VSize(TestWP.Location - MyGearPawn.Location);
			ToTestWPNorm /= ToTestWPDist;

			// distance score (-1 pt per unit distance ftw)
			// maybe take velocity into account when determining distance score?
			DistScore = SearchRadius - ToTestWPDist;

			// directional score (512 dead ahead, -512 behind, 0 at the sides)
			DotToTarget = ToTestWPNorm dot vector(MyGearPawn.Rotation);

			// make sure dir is within limits
			if( DotToTarget < MinMeleeAdhesionDotValue )
			{
				continue;
			}
			DirScore = DotToTarget * SearchRadius;

			TotalScore = DirScore + DistScore;
			if( TotalScore > BestTargetScore )
			{
				BestTarget = TestWP;
				BestTargetScore = TotalScore;
			}
		}
	}

	//`log("chose"@BestTarget@"as best melee target to lock onto.  Score:"@BestTargetScore);
	if( BestTarget != None )
	{
		ForcedAdhesionTarget = BestTarget;
	}

	return BestTarget;
}

/**
 * Stops any melee adhesion that is going on.
 */
simulated function StopMeleeAdhesion()
{
	StopForcedAdhesion();
}

/**
 * Turns off forced adhesion.
 */
simulated function StopForcedAdhesion()
{
	ForcedAdhesionTarget = None;
}

native function ZoomToMap( string MapName );
native function string GetCurrMapName();
native function string GetCurrGameType();

exec function Revive()
{
	ServerReviveSelf();
}


// our old friend "the rez system" has returned to pay us a visit


function GotoReviving()
{
	//`log( "GotoReviving" );
	GotoState( 'Reviving' );
}

function GotoPlayerWalking()
{
	ServerGotoPlayerWalking();
	//`log( "GotoPlayerWalking" );
	GotoState( 'PlayerWalking' );
	Pawn.SetPhysics( PHYS_Falling );

}

unreliable server function ServerGotoPlayerWalking()
{
	//`log( "ServerGotoPlayerWalking" );
	GotoState( 'PlayerWalking' );
	Pawn.SetPhysics( PHYS_Falling );
}

simulated function SetReviveIconAnimationSpeed()
{
	local float PercentAlive, MinSpeed, MaxSpeed;

	if ( (MyGearPawn != None) && (MyGearHUD.ActiveAction.ActionType == AT_StayingAlive) )
	{
		MinSpeed = 0.5f;
		MaxSpeed = 0.05f;
		PercentAlive = MyGearHUD.CalculateRevivePercentAlive( MyGearPawn );
		MyGearHUD.ActiveAction.IconAnimationSpeed = (MinSpeed - MaxSpeed) * PercentAlive + MaxSpeed;
	}
}

/**
 *	Pass DBNO notification to squad for possible action
 */
final private function NotifySquadDBNO()
{
	if( Squad != None )
	{
		Squad.NotifyMemberDBNO( GearPawn(Pawn) );
	}
}

function PressedRightTrigger_Reviving()
{
	if (!MyGearPawn.IsTimerActive('LowerHandWhileCrawling'))
	{
		MyGearPawn.RaiseHandWhileCrawling();
		IgnoreMoveInput(TRUE);
		IgnoreLookInput(TRUE);
	}
}

function HandLoweredWhileCrawling()
{
	IgnoreMoveInput(FALSE);
	IgnoreLookInput(FALSE);
}

/** Handles logic for boosting the crawl speed when mashing the action button */
function PressedActionButton_Reviving()
{
	if (!IsTimerActive('CrawlSpeedRetriggerDelay'))
	{
		if (MyGearPawn.CrawlSpeedModifier < 0.7f)
		{
			MyGearPawn.AlterCrawlSpeed(0.1f);
		}
		// reset the retrigger delay even if speed didn't change to avoid spam at max modifier
		SetTimer( 0.2f,FALSE,nameof(CrawlSpeedRetriggerDelay) );
	}
	// reset the decay timer so that repeated mashes sustain the current speed
	SetTimer( 0.5f,FALSE,nameof(ReduceCrawlSpeed) );
}

/** Stub function to prevent retriggering the boost for a fixed period. */
function CrawlSpeedRetriggerDelay();

/** Reduces the crawl speed boost. */
function ReduceCrawlSpeed()
{
	MyGearPawn.AlterCrawlSpeed(-0.1f);
	if (MyGearPawn.CrawlSpeedModifier > 0.f)
	{
		SetTimer( 0.3f,FALSE,nameof(ReduceCrawlSpeed) );
	}
}

/** Internal.  Returns true if playing a debug/development level, false otherwise. */
simulated function bool PlayingDevLevel()
{
/*
`if(`notdefined(FINAL_RELEASE))
	local string MapName;

	MapName = Locs(GetCurrMapName());

	if ( (InStr(MapName, 'poc_') == 0) || (InStr(MapName, 'dp_') == 0) )
	{
		return TRUE;
	}
`endif
*/
	return FALSE;
}

final function CheckSurvivalChance()
{
	if (!PlayingDevLevel() && !GearGame(WorldInfo.Game).TeamHasAnyChanceOfSurvival(GetTeamNum()))
	{
		MyGearPawn.bCanRecoverFromDBNO = false;
		MyGearPawn.DoFatalDeath(self,class'GDT_BledOut',MyGearPawn.Location,vect(0,0,1));
		`log("Ending game from player death, no more survival chance"@`showvar(self));
		if (!IsTimerActive(nameof(ProcessGameOver)))
		{
			SetTimer(3.0, false, nameof(ProcessGameOver));
		}
		ClearTimer(nameof(CheckSurvivalChance));
	}
}

simulated state Reviving extends PlayerWalking
{
	event BeginState(Name PreviousStateName)
	{
		local GearInventoryManager	InvManager;

		if ( (Role == ROLE_Authority) && !IsLocalPlayerController() )
		{
			bRequestClientVerifyState = true;
		}
		// Turn off targeting when you die
		bIsTargeting = FALSE;
		SetTargetingMode( FALSE );

		if (IsLocalPlayerController())
		{
			StartPostProcessOverride(eGPP_BleedOut);
		}

		if( !WorldInfo.GRI.IsMultiplayerGame() )
		{
			if (MyGearHUD != None)
			{
				MyGearHUD.SetActionInfo( AT_StayingAlive, ActionStayingAlive );
			}
			SetReviveIconAnimationSpeed();

			// if we are in a solo split, there is no one to revive us so this is game over
			if (Role == ROLE_Authority && !WorldInfo.IsPlayInEditor())// && !PlayingDevLevel())
			{
				if (!PlayingDevLevel() && !GearGame(WorldInfo.Game).TeamHasAnyChanceOfSurvival(GetTeamNum()))
				{
					MyGearPawn.bCanRecoverFromDBNO = false;
					`log("Ending game from player DNBO, no survival chance"@`showvar(self));
					SetTimer(3.0, false, nameof(ProcessGameOver));
				}
				else
				{
					SetTimer(1.0, true, nameof(CheckSurvivalChance));
				}
			}
		}
		else if( MyGearHUD != None ) // won't have HUD for remote clients on server
		{
			if ( (MyGearPawn != None) && (MyGearPawn.MyGearWeapon != None) &&
				 ClassIsChildOf(MyGearPawn.MyGearWeapon.Class, class'GearWeap_GrenadeBase') &&
				 MyGearPawn.MyGearWeapon.HasAmmo(0) &&
				 MyGearPawn.MyGearWeapon.IsInState('Active') )
			{
				MyGearHUD.SetActionInfo(AT_SuicideBomb, ActionSuicideBomb);
				// See if we need to do a TG tutorial
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_GrenCrawl, -1);
			}
			else
			{
				MyGearHUD.SetActionInfo(AT_StayingAlive, ActionStayingAlive);
				SetReviveIconAnimationSpeed();
				// See if we need to do a TG tutorial
				AttemptTrainingGroundTutorial(GEARTUT_TRAIN_Crawl, -1);
			}
		}

		InvManager = GearInventoryManager(Pawn.InvManager);
		if ( InvManager != None )
		{
			InvManager.bShowWeaponSlots = FALSE;
		}

		bFire = 0;
		StopFire(0);

		// Notify squad that I need to be revived
		SetTimer( 0.5f, FALSE, nameof(NotifySquadDBNO) );

		// Trigger is crawling delegate
		TriggerGearEventDelegates( eGED_Crawling );
	}

	event EndState(Name NextStateName)
	{
		local GearWeapon WWeapon;

		ClearTimer(nameof(CheckSurvivalChance));

		GearPlayerCamera(PlayerCamera).SetDesiredColorScale(vect(1.0f,1.0f,1.0f),1.0f);

		BiggestThreatWP = None;
		ClearFocusPoint(FALSE);		// clear both kismet and nonkismet lookats
		ClearFocusPoint(TRUE);

		ClearTimer(nameof(CheckSurvivalChance));

		if (NextStateName != 'Dead')
		{
			StopPostProcessOverride(eGPP_BleedOut);
		}

		if( !WorldInfo.GRI.IsMultiplayerGame() )
		{
			MyGearHUD.ClearActionInfoByType(AT_StayingAlive);
		}
		else if( MyGearHUD != None ) // won't have HUD for remote clients on server
		{
			// stop pp effect for bleed out
			MyGearHUD.ClearActionInfoByType(AT_StayingAlive);
			MyGearHUD.ClearActionInfoByType(AT_SuicideBomb);
		}

		// unholster the weapon if needed
		if ( MyGearPawn != None )
		{
			WWeapon = MyGearPawn.MyGearWeapon;
			if ( WWeapon != None && WWeapon.bTemporaryHolster )
			{
				WWeapon.HolsterWeaponTemporarily(FALSE);
			}
		}

		// Clear notification
		ClearTimer( 'NotifySquadDBNO' );
	}

	event PlayerTick( float DeltaTime )
	{
		if( Pawn == None )
		{
			GotoState( 'Dead' );
		}
		else
		{
			SetReviveIconAnimationSpeed();

			// make sure to call parent so that we crawl around, as well as process input, etc
			Super.PlayerTick(DeltaTime);
		}
	}

	simulated function bool IsDead()
	{
		return TRUE;
	}

	function StartFire( optional byte FireModeNum )
	{
	}

	function StopFire( optional byte FireModeNum )
	{
	}

	function StartAltFire( optional byte FireModeNum )
	{
	}

	function StopAltFire( optional byte FireModeNum )
	{
	}

	function bool CanTryToEnterCover()
	{
		return FALSE;
	}

	exec function bool TryASpecialMove(bool bRunActions)
	{
		return FALSE;
	}

Begin:
}

/** Play Anya's "Dom, Marcus is down" or "Marcus, Dom is down" line if one of them
 *  goes down during a split or is killed.
 */
simulated function PlayAnyaCoopDeathSound()
{
	local GearPC PC;
	local bool bMarcusDied;

	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		if ( (PC != None) && (PC != self) && (Pawn != None) && (PC.PlayerReplicationInfo != None) && !PC.IsDead() )
		{
			bMarcusDied = (GearPawn_COGMarcus(Pawn) != None);
			GearPRI(PC.PlayerReplicationInfo).ClientPlayCoopSplitDeathSound( bMarcusDied );
		}
	}
}

// streamingServer
exec function ShowLevelStatus()
{
	ServerShowLevelStatus();
}

unreliable server function ServerShowLevelStatus()
{
`if(`notdefined(FINAL_RELEASE))
	local int LevelIndex;
	local LevelStreaming TheLevel;

 	// Tell the player controller the current streaming level status
 	for (LevelIndex = 0; LevelIndex < WorldInfo.StreamingLevels.Length; LevelIndex++)
 	{
		TheLevel = WorldInfo.StreamingLevels[LevelIndex];

		`log( "ServerShowLevelStatus levelStatus: " $ TheLevel.PackageName $ " "
			$ TheLevel.bShouldBeVisible  $ " "
			$ TheLevel.bIsVisible  $ " "
			$ TheLevel.bShouldBeLoaded  $ " "
			$ TheLevel.LoadedLevel  $ " "
			$ TheLevel.bHasLoadRequestPending  $ " "
			) ;
	}
`endif
}

function bool SetPause(bool bPause, optional delegate<CanUnpause> CanUnpauseDelegate = CanUnpause)
{
	if( bPause )
	{
		if( MyGearPawn != None )
		{
			MyGearPawn.bWantsToMelee = FALSE;
		}
	}
	else
	{
		MainPlayerInput.LastInputTime = WorldInfo.TimeSeconds;
	}

	return super.SetPause(bPause,CanUnpauseDelegate);
}

/** @return the current DifficultyLevel */
final event EDifficultyLevel GetCurrentDifficultyLevel()
{
	return GearPRI(PlayerReplicationInfo).Difficulty.default.DifficultyLevel;
}

reliable client function ClientSetRotation( rotator NewRotation, optional bool bResetCamera )
{
	local GearPlayerCamera Cam;
	Super.ClientSetRotation(NewRotation, bResetCamera);

	if (bResetCamera)
	{
		// clear camera orientation as well
		Cam = GearPlayerCamera(PlayerCamera);
		if (Cam != None)
		{
			Cam.ResetInterpolation();
		}

		// @fixme gears 3: every place we reset the camera, consider doing this, as well as
		// killing all camera anims
		// only doing this much for g2 to fix a specific bug, since it's late.
		StopCameraBoneAnim(0.f, FALSE);
	}
}

reliable client function ClientSetJustControllerRotation(rotator NewRotation)
{
	SetRotation(NewRotation);
}

reliable client event ClientSetLocationAndBase(vector NewLocation, Actor NewBase)
{
	if (Pawn != None)
	{
		Pawn.SetLocation(NewLocation);
		Pawn.SetBase(NewBase);
	}
}

exec function NextViewTarget()
{
	local GearPawn WP;
	WP = GearPawn(ViewTarget);
	if (WP == None || WP.NextPawn == None)
	{
		SetViewTarget(WorldInfo.PawnList);
	}
	else
	{
		SetViewTarget(WP.NextPawn);
	}
	if (ViewTarget == MyGearPawn)
	{
		GotoState('PlayerWalking');
	}
	else
	{
		GotoState('PlayerSpectating');
	}
}

event bool FindAvailableTeleportSpot( Pawn TestPawn, optional NavigationPoint TeleportAnchor, optional bool bSkipRadiusSearch)
{
	local array<NavigationPoint> NavList;
	local NavigationPoint Nav;
	local vector ChkLoc, NavLoc;
	local GearAI AI;
	local GearPawn GP;
	local GearTurret GT;
	local int Idx;
	local bool bSecondPass;
	local rotator TeleportRot;

	if (GearWeap_SniperRifle(TestPawn.Weapon) != None)
	{
		`log("- sniper no see hack for checkpoints");
		GearWeap_SniperRifle(TestPawn.Weapon).SetOwnerNoSee(FALSE);
	}

	GP = GearPawn(TestPawn);
	if( GP != None )
	{
		ServerReviveTeamMate(GP);
		GP.ServerEndSpecialMove();
		if (GearPC(GP.Controller) != None)
		{
			GearPC(GP.Controller).ClientBreakFromCover();
		}
		else
		{
			GP.BreakFromCover();
		}
	}

	// if we're on a troika, get off before the teleport
	GT = GearTurret(TestPawn);
	if(GT != none)
	{
		`log("Forcing "@TestPawn.Controller@"off turret for checkpoint teleport");
		GT.DriverLeave(TRUE);
	}

	AI = GearAI(TestPawn.Controller);
	// if an anchor was specified
	if (TeleportAnchor != None)
	{
		// then attempt to spawn at that anchor
		Nav = TeleportAnchor;
		NavLoc = Nav.Location;

		if ( class'SeqAct_AIFactory'.static.CanSpawnAtLocation(NavLoc,TestPawn.GetCollisionExtent(),Nav) )
		{
			if (AI != None)
			{
				if (AI.TeleportToLocation(NavLoc,Nav.Rotation))
				{
					return TRUE;
				}
			}
			else if (TestPawn.SetLocation(Nav.Location))
			{
				if (TestPawn.Controller != None)
				{
					TeleportRot = Nav.Rotation;
					TeleportRot.Roll = 0;
					TestPawn.Controller.SetRotation(TeleportRot);
					TestPawn.Controller.ClientSetLocation(TestPawn.Location, TeleportRot);
					TestPawn.Controller.ClientSetRotation(TeleportRot, TRUE);
				}
				TestPawn.SetPhysics(PHYS_Falling);
				return TRUE;
			}
		}
		// otherwise use the anchor as the check location
		ChkLoc = TeleportAnchor.Location;
	}
	else
	{
		// use current pawn location if possible
		ChkLoc = Pawn != None ? Pawn.Location : Location;
	}
	if (!bSkipRadiusSearch)
	{
		// grab nodes in a radius
		// first try to spawn behind this player - if that doesn't work try anything
		class'NavigationPoint'.static.GetAllNavInRadius(TestPawn, ChkLoc, 1024.f, NavList);
		for (Idx = 0; Idx < NavList.Length; Idx++)
		{
			Nav = NavList[Idx];
			if ( !Nav.IsA('CoverSlotMarker') && Abs(Nav.Location.Z - ChkLoc.Z) < 78.f &&
				( bSecondPass || Pawn == None ||
					( VSize(Nav.Location - Pawn.Location) > 100.0 &&
						vector(Rotation) dot Normal(Nav.Location - Pawn.Location) < 0.0 &&
						FastTrace(Nav.Location, Pawn.Location) ) ) )
			{
				NavLoc = Nav.Location;
				if ( class'SeqAct_AIFactory'.static.CanSpawnAtLocation(NavLoc, TestPawn.GetCollisionExtent(), Nav) )
				{
					if (AI != None)
					{
						if (AI.TeleportToLocation(NavLoc,Nav.Rotation))
						{
							return TRUE;
						}
					}
					else if (TestPawn.SetLocation(NavLoc))
					{
						if (TestPawn.Controller != None)
						{
							TeleportRot = Nav.Rotation;
							TeleportRot.Roll = 0;
							TestPawn.Controller.SetRotation(TeleportRot);
							TestPawn.Controller.ClientSetLocation(TestPawn.Location, TeleportRot);
							TestPawn.Controller.ClientSetRotation(TeleportRot, TRUE);
						}
						TestPawn.SetPhysics(PHYS_Falling);
						`Log("FindAvailableTeleportSpot: Successfully teleported" @ TestPawn @ "to" @ Nav);
						return TRUE;
					}
				}
			}
			if (Idx == NavList.length - 1 && !bSecondPass)
			{
				bSecondPass = true;
				Idx = -1;
			}
		}
	}
	`warn("Failed to find teleport destination for"@TestPawn@"near"@self);
	return FALSE;
}

unreliable server function TeleportToPlayer()
{
	local GearPC PC;
	local bool bTeleportedToPlayer;
	local PlayerStart Start;

	ClientBreakFromCover();

	// find another human player
	bTeleportedToPlayer = FALSE;
	foreach WorldInfo.AllControllers(class'GearPC',PC)
	{
		if (PC != self)
		{
			`log("- attempting teleport to:"@PC);
			// teleport the player's location
			if (PC.FindAvailableTeleportSpot(Pawn))
			{
				bTeleportedToPlayer = TRUE;
				break;
			}
		}
	}
	if (!bTeleportedToPlayer)
	{
		// teleport to the first available playerstart
		foreach WorldInfo.AllNavigationPoints(class'PlayerStart',Start)
		{
			if (Pawn.SetLocation(Start.Location))
			{
				Pawn.SetPhysics(PHYS_Falling);
				break;
			}
		}
	}
}

event CurrentLevelUnloaded()
{
	// only teleport if this is a multiplayer game
	if (Role == Role_Authority && WorldInfo.Game.NumPlayers > 1)
	{
		TeleportToPlayer();
	}
}

reliable client function YouAreTheKing()
{
	if (ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_ItsGoodToBeTheKing, self);
	}
}

/** Checks for killing 3 enemies while mounted on cover */
function CheckForMinigunAchievement(class<GearDamageType> GDTClass)
{
	if (bIsMountedWithMinigun &&
		ClassIsChildOf(GDTClass, class'GDT_HeavyMiniGun') &&
		ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_DIYTurret, self);
	}
}

/** Checks for killing 3 enemies with 1 mortar shot */
function CheckForMortarAchievement(class<GearDamageType> GDTClass)
{
	if (ClassIsChildOf(GDTClass, class'GDT_Mortar') &&
		ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_ShockAndAwe, self);
	}
}

/** Mark training lesson as completed and travel back to the training screen */
reliable client function ClientCompletedTraining( EGearTrainingType TrainType, bool bOnWinningTeam )
{
	local int Idx;

	if ( ProfileSettings != None && bOnWinningTeam )
	{
		// Haven't completed this one yet
		if ( !ProfileSettings.HasCompletedTraining(TrainType) )
		{
			// Mark completed
			ProfileSettings.MarkTrainingCompleted(TrainType, self);

			// If this was "Basic" mark all the other ones as accessible and needs viewed
			if ( TrainType == eGEARTRAIN_Basic )
			{
				for ( Idx = 0; Idx < eGEARTRAIN_MAX; Idx++ )
				{
					if ( Idx != eGEARTRAIN_Basic )
					{
						ProfileSettings.MarkTrainingAccessible(EGearTrainingType(Idx));
						//ProfileSettings.MarkTrainingNeedsViewed(EGearTrainingType(Idx));
					}
				}
			}
		}
	}

	// Travel back to main menu
	//ClientShowLoadingMovie(true);
	class'GearUIScene_Base'.static.SetTransitionValue("CompletedTraining", "1");
	ClientTravel("?closed", TRAVEL_Absolute);
}

/** Checks for killing enemies with the FlameThrower */
function CheckForFlameThrowerAchievement( class<GearDamageType> GDTClass )
{
	if ((GDTClass == class'GDT_FlamethrowerSpray' || GDTClass == class'GDT_FireDOT') &&
		ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_ShakeAndBake, self);
	}
}

/** Checks for killing with a sticky grenade */
function CheckForStickyAchievement( class<GearDamageType> GDTClass )
{
	if (GDTClass == class'GDT_FragSticky' && ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_SaidTheSpiderToTheFly, self);
	}
}

/** Checks for killing while holding a boomshield */
function CheckForBoomShieldKillAchievement( class<GearDamageType> GDTClass )
{
	if (MyGearPawn != None &&
		MyGearPawn.EquippedShield != None &&
		(GDTClass == class'GDT_ShieldBash' || GDTClass == class'GDT_QuickExecution') &&
		ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_PeekABoo, self);
	}
}

/** Checks for meleeing tickers achievement */
reliable client function ClientCheckForMeleeTickerAchievement()
{
	if (ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_TickerLicker, self);
	}
}

/** Special case for adding hit locators when in vehicles, normal locators are handled from GearDamageType */
unreliable client function ClientAddHitLocator(Actor InstigatorA,int DamageDone,vector HitLocation)
{
	if (MyGearHud != None)
	{
		MyGearHud.AddNewHitLocatorData(InstigatorA,DamageDone,HitLocation,TRUE);
	}
}

// @param bIgnoreForHUD - will not display in the HUD
reliable client function AddDeathMessage( PlayerReplicationInfo Killer, PlayerReplicationInfo Other, class<DamageType> damageType, EGearDeathType DeathType, optional int PointsAwarded, optional bool bIgnoreForHUD, optional bool bSkipBlood )
{
	local class<GearDamageType> GDTClass;

	GDTClass = class<GearDamageType>(damageType);

	if( MyGearHud != None && !bIgnoreForHUD )
	{
		MyGearHud.AddDeathMessageData( Killer, Other, GDTClass, DeathType, PointsAwarded, bSkipBlood );
	}

	//`log(GetFuncName()@`showvar(damageType)@`showvar(Killer == PlayerReplicationInfo));

	if ( Killer != None && Killer == PlayerReplicationInfo && ProfileSettings != None && GDTClass != class'GDT_KnockedDown' && !GearPRI(Other).bIsMeatFlag )
	{
		// Make sure we don't give credit for killing teammates
		if ( Other == None || Killer.GetTeamNum() != Other.GetTeamNum() )
		{
			ProfileSettings.UpdateKillingProgression( GDTClass, self );
			ProfileSettings.UpdateExecutionProgression( GDTClass, self );
			CheckForMinigunAchievement( GDTClass );
			CheckForMortarAchievement( GDTClass );
			CheckForFlameThrowerAchievement( GDTClass );
			CheckForStickyAchievement( GDTClass );
			CheckForBoomShieldKillAchievement( GDTClass );
		}
	}
}

unreliable client function AddScoreGameMessage(int Id, int Pts, optional GearPRI Victim)
{
	if (MyGearHud != None)
	{
		MyGearHud.AddScoreGameMessage(Id,Pts,Victim);
	}
}

reliable client function UpdateSeriously()
{
	if (ProfileSettings != None)
	{
		ProfileSettings.UpdateKillingProgression( None, self );
	}
}

reliable client function AddWeaponTakenMessage( PlayerReplicationInfo WeaponTaker, class<DamageType> damageType )
{
	if( MyGearHud != None )
	{
		MyGearHud.AddWeaponTakenMessage( WeaponTaker, class<GearDamageType>(damageType) );
	}
}


/* ClientReset()
reset actor to initial state - used when restarting level without reloading.
*/
reliable client function ClientReset()
{
	if ( MyGearHud != None )
	{
		MyGearHud.Reset();
	}

	Super.Reset();
}

function TakeMeleeDamage( Pawn DamageDealer )
{
}

function PawnReviving( GearPawn Victim )
{
}

/** called to replicate a Kismet animation action on a Pawn */
unreliable client function ClientPlayGearPlayerAnim(GearPawn AnimTarget, SeqAct_GearPlayerAnim InAction, bool bPlay)
{
	if (AnimTarget != None && InAction != None)
	{
		AnimTarget.ProcessGearPlayerAnim(InAction, bPlay);
	}
}

/**
 * Overridden to handle replacing players with AI in coop.
 */
event Destroyed()
{
	local SavedMove Next;
	local GearPawn_COGGear Gear;
	local GearAI_Dom AI;
	local Pawn OldPawn;
	local array<SequenceObject> AllLeavingEvents;
	local Sequence GameSeq;
	local int i;

	Gear = GearPawn_COGGear(Pawn);
	if (Gear == None && Vehicle(Pawn) != None)
	{
		Gear = GearPawn_COGGear(Vehicle(Pawn).Driver);
	}

	StopTutorialSystem();
	GearEventList.Length = 0;
	GearWeaponEquipDelegates.Length = 0;

	// if standalone or not coop or not a gear, then don't give a damn
	if (!bCheckpointDummy && (!IsCoop() || GearGameSP_Base(WorldInfo.Game) == None || Gear == None))
	{
		Super.Destroyed();
	}
	// if you disconnect while chainsaw dueling, you die
	// this prevents various animation and AI issues and makes reasonable sense anyway
	else if (Gear.IsDoingSpecialMove(SM_ChainsawDuel_Leader) || Gear.IsDoingSpecialMove(SM_ChainsawDuel_Follower))
	{
		Gear.Health = 0;
		Gear.Died(self, class'GDT_ScriptedGib', Gear.Location);
	}
	else
	{
		// dismount from any turrets first
		if (Turret(Pawn) != None)
		{
			Turret(Pawn).DriverLeave(true);
		}
		// stop doing any engage actions
		if ( Gear.IsDoingSpecialMove(SM_Engage_Idle) || Gear.IsDoingSpecialMove(SM_Engage_Loop) ||
			Gear.IsDoingSpecialMove(SM_Engage_Start) )
		{
			Gear.DoSpecialMove(SM_Engage_End, true);
		}
		// turn off conversing anim since it seems to get stuck in some cases
		Gear.SetConversing(false);
		// give up our pawn
		OldPawn = Pawn;
		UnPossess();
		// and make an AI for them
		AI = Spawn(class'GearAI_Dom');
		if (bCheckpointDummy)
		{
			// copy GUID in this case so AI gets hooked up to Kismet co-op player was connected to
			AI.MyGuid = MyGuid;
		}
		AI.SetTeam(PlayerReplicationInfo.Team.TeamIndex);
		AI.Possess(OldPawn, false);
		// have him join the previous squad
		AI.SetSquadName(GearPRI(PlayerReplicationInfo).SquadName, false);
		GearPRI(AI.PlayerReplicationInfo).bForceShowInTaccom = GearPRI(PlayerReplicationInfo).bForceShowInTaccom;
		// toggle cinematic mode if necessary
		if (bCinematicMode)
		{
			AI.WaitForEvent('OnToggleCinematicMode');
		}
		// put AI in god mode while riding the brumak
		if (GearPawn_LocustBrumakPlayerBase(AI.Pawn.Base) != None)
		{
			AI.bGodMode = true;
		}
		// fire off a Kismet event so the level can tell the AI what to do if progression requires it
		GameSeq = WorldInfo.GetGameSequence();
		if (GameSeq != None)
		{
			GameSeq.FindSeqObjectsByClass(class'SeqEvent_CoopPlayerLeft', true, AllLeavingEvents);
			for (i = 0; i < AllLeavingEvents.length; i++)
			{
				SeqEvent_CoopPlayerLeft(AllLeavingEvents[i]).CheckActivate(AI, AI);
			}
		}

		// since we're skipping the PlayerController implementation, need to call this again.
		ForceClearUnpauseDelegates();

		// PlayerController cleanup that we need to deal with
		if ( myHUD != None )
		{
			myHud.Destroy();
		}

		while ( FreeMoves != None )
		{
			Next = FreeMoves.NextMove;
			//FreeMoves.Destroy();
			FreeMoves = Next;
		}

		while ( SavedMoves != None )
		{
			Next = SavedMoves.NextMove;
			//SavedMoves.Destroy();
			SavedMoves = Next;
		}

		if( PlayerCamera != None )
		{
			PlayerCamera.Destroy();
			PlayerCamera = None;
		}

		// Clear the online delegates for this PC that was registered in PostBeginPlay.
		ClearOnlineDelegates();

		// remove this player's data store from the registered data stores..
		UnregisterPlayerDataStores();

		super(Controller).Destroyed();
	}
}

simulated function ActiveReloadSuccess( bool bDidSuperSweetReload )
{
	if( bDidSuperSweetReload == TRUE )
	{
		ClientPlayCameraShake( ARSuperSuccessScreenShake );
	}
}

/**
* Called to open any door we're able to interact with currently.
*/
reliable server function DoDoorOpen(ESpecialMove eMoveType)
{
	local FLOAT TimeToTriggerDoor;

	// if we can trigger this door and we're not already triggering one
	if (DoorTriggerDatum.DoorTrigger != None && !IsTimerActive('DoDoorOpenCallback') && DoorTriggerDatum.DoorTrigger.TriggerEventClass(class'SeqEvt_DoorOpen',Pawn,,TRUE))
	{
		if (IsDoingSpecialMove(SM_RoadieRun))
		{
			EndSpecialMove();
		}
		// set the method of door opening
		DoorTriggerDatum.eSpecialMoveType = eMoveType;
		ServerDictateSpecialMove(DoorTriggerDatum.eSpecialMoveType);
		// if we should align to a spot
		if ( DoorTriggerDatum.DoorTrigger.AlignToObj != None )
		{
			// then set that value to start moving the player
			SetDestinationPosition( DoorTriggerDatum.DoorTrigger.AlignToObj.Location, TRUE );
			bPreciseDestination = TRUE;
		}

		TimeToTriggerDoor = ((DoorTriggerDatum.eSpecialMoveType == SM_DoorKick) ? 0.95f : 0.65f) * MyGearPawn.SpecialMoveClasses[DoorTriggerDatum.eSpecialMoveType].default.SpeedModifier;

		// set the timer to actually trigger the door open event
		SetTimer( TimeToTriggerDoor , FALSE, nameof(DoDoorOpenCallback) );

		// If can't trigger the door anymore
		if( !DoorTriggerDatum.DoorTrigger.TriggerEventClass(class'SeqEvt_DoorOpen',Pawn,,TRUE) )
		{
			// Stop testing this door trigger
			DoorTriggerDatum.bInsideDoorTrigger = FALSE;
			ClientClearDoorTrigger();
		}
	}
}

reliable client function ClientClearDoorTrigger()
{
	DoorTriggerDatum.bInsideDoorTrigger = false;
}

/**
* Delayed triggering of the event for opening a door.
*/
function DoDoorOpenCallback()
{
	// fire off the event
	if (DoorTriggerDatum.DoorTrigger != None && DoorTriggerDatum.DoorTrigger.TriggerEventClass(class'SeqEvt_DoorOpen', Pawn, (DoorTriggerDatum.eSpecialMoveType == SM_DoorKick) ? 1 : 0))
	{
		DoorTriggerDatum.DoorTrigger.TriggerEventClass(class'SeqEvt_DoorOpen',Pawn,,TRUE);
	}
}

/**
* Called when the pawn touches a door trigger, sets up the data and checks
* for a roadie run kick open.
*/
simulated function OnEnterDoorTrigger( Trigger_DoorInteraction DoorTrigger )
{
	// save the info
	DoorTriggerDatum.DoorTrigger = DoorTrigger;
	DoorTriggerDatum.bInsideDoorTrigger = TRUE;
	// check to see if we can activate it now
	if( IsDoingSpecialMove(SM_RoadieRun) )
	{
		DoDoorOpen( SM_DoorKick );
	}
}

/**
* Called when the pawn is no longer touching a door.
*/
simulated function OnExitDoorTrigger()
{
	// clear the info
	DoorTriggerDatum.bInsideDoorTrigger = false;
	DoorTriggerDatum.DoorTrigger = None;
}

/**
* Set the colorization settings for post process
*/
simulated function SetPausePostProcessColorizationSettings( out PostProcessSettings PPSettings )
{
	PPSettings.bEnableSceneEffect = true;
	PPSettings.Scene_InterpolationDuration = 1.f;
	PPSettings.Scene_Desaturation = 0.8f;
	PPSettings.Scene_HighLights = vect( 1.1f, 1.1f, 1.1f );
	PPSettings.Scene_MidTones = vect( 1.2f, 1.2f, 1.2f );
	PPSettings.Scene_Shadows = vect( 0.f, 0.f, 0.f );
}

/**
 * Set the blur effect for the pause and gameover screens.
 */
simulated function SetPostProcessBlurSettings( out PostProcessSettings PPSettings )
{
	PPSettings.bEnableBloom = true;
	PPSettings.Bloom_InterpolationDuration = 1.f;
	PPSettings.Bloom_Scale = 1.f;

	PPSettings.bEnableDOF = true;
	PPSettings.DOF_InterpolationDuration = 1.f;
	PPSettings.DOF_BlurKernelSize = 5.f;
	PPSettings.DOF_FalloffExponent = 1.f;
	PPSettings.DOF_FocusDistance = 0.f;
	PPSettings.DOF_FocusInnerRadius = 0.f;
	PPSettings.DOF_FocusPosition = vect(0,0,0);
	PPSettings.DOF_FocusType = FOCUS_Distance;
	PPSettings.DOF_MaxFarBlurAmount = 0.7f;
	PPSettings.DOF_MaxNearBlurAmount = 0.7f;
}

/**
 * Get the post process settings for the pause screen.
 */
simulated function InitPausePostProcessSettings()
{
	SetPostProcessBlurSettings( PausePPSettings );
	SetPausePostProcessColorizationSettings( PausePPSettings );
}

/**
 * Cancel cinematics, or if no cine's, then pause the game
 *
 * @InitialNoSkipTime Optional time to disallow skipping at the start of a cinematic
 */
exec function CancelCine(optional float InitialNoSkipTime)
{
	local string CurrentMovieName;
	//local string CancelResult;

	// if we are playing a movie, then cancel the movie now
	GetCurrentMovie(CurrentMovieName);
	if (CurrentMovieName != "")
	{
		// @todo: What about server/client issues?
//		ClientStopMovie("");
	}
	else
	{
		// if we are in a skippable matinee then cancel it
		ConsoleCommand("CANCELMATINEE " $ InitialNoSkipTime);
	}
}

/**
 * Pause the game if there is no movie playing
 */
exec function ConditionalPauseGame()
{
	local string CurrentMovieName;
	//local string CancelResult;

	// if we are playing a movie, then don't pause
	GetCurrentMovie(CurrentMovieName);
	if (CurrentMovieName == "")
	{
		PauseGame();
	}
}

/** Whether we can open the pause screen or not */
function bool CanPauseGame( GearPC PC )
{
//	if ( (PC.MyWarHud != none) && !PC.IsTimerActive('OpenGameoverSceneFromDelay', PC) && !PC.IsTimerActive('StartProcessGameOverTimer', PC) && ((PC.OpenedUISceneGameover == None) || (PC.OpenedUISceneGameover.SceneTag != PC.GameoverScene.SceneTag)) )
//	{
		return true;
//	}

//	return false;
}

/**
 * Pause the game and open the pause screen.
 */
function PauseGame( optional delegate<PlayerController.CanUnpause> CanUnpauseDelegate=CanUnpause )
{
	local GearPC PC;
	local PlayerController APlayer;
	local bool bCanPauseGame;
	local delegate<PlayerController.CanUnpause> DefaultUnpauseDelegate;

	DefaultUnpauseDelegate = CanUnpause;

	// default this flag to true
	bCanPauseGame = true;

	// search for any local players that can't pause the game
	foreach LocalPlayerControllers(class'PlayerController', APlayer)
	{
		PC = GearPC(APlayer);
		if ( PC != None )
		{
			// duh, can't pause
			if ( !CanPauseGame(PC) )
			{
				bCanPauseGame = false;
			}
		}
	}

	if ( bCanPauseGame )
	{
		if ( MyGearHud != None )
		{
			if ( CanUnpauseDelegate == DefaultUnpauseDelegate )
			{
				// if we're using the HUD to pause the game, don't use the default delegate
				MyGearHUD.PauseGame();
			}
			else
			{
				MyGearHud.PauseGame(CanUnpauseDelegate);
			}
		}
		else
		{
			SetPause(true, CanUnpauseDelegate);
		}

		if ( Role == ROLE_Authority && WorldInfo.Pauser != None &&
			(!WorldInfo.GRI.IsMultiplayerGame() || WorldInfo.GRI.IsCoopMultiplayerGame()) )
		{
			foreach WorldInfo.AllControllers(class'GearPC', PC)
			{
				if (LocalPlayer(PC.Player) == None)
				{
					PC.ClientUpdateHostPause(true);
				}
			}
		}
	}
}

/** input handler for the "host has paused the game" scene */
function bool ProcessClientPauseKey(out InputEventParameters EventParms)
{
	if (EventParms.InputKeyName == 'XboxTypeS_Start' && EventParms.EventType == IE_Released)
	{
		PauseGame();

	}
	return true;
}

/** @hack: called on a timer while we think the host has paused the game to ask it for sure
 * because in rare cases it seems we get out of sync
 */
unreliable server final function ServerVerifyPausedState()
{
	`Log("TEST");
	if (WorldInfo.Pauser == None)
	{
		UnreliableClientClearHostPause();
	}
}
unreliable client final function UnreliableClientClearHostPause()
{
	ClientUpdateHostPause(false);
}

/** notification that the host paused a campaign game */
reliable client function ClientUpdateHostPause(bool bNowPaused)
{
	local array<name> ButtonArray;
	local UIMessageBoxBase MessageBox;
	local GameUISceneClient GameSceneClient;
	local UIScene PauseScene;

	if (WorldInfo.NetMode == NM_Client)
	{
		GameSceneClient = class'UIRoot'.static.GetSceneClient();
		if (bNowPaused)
		{
			ButtonArray.length = 0; // shut up the compiler
			GameSceneClient.ShowUIMessage( 'HostPaused', "<Strings:GearGameUI.MessageBoxStrings.HostPaused_Title>",
							"<Strings:GearGameUI.MessageBoxStrings.HostPaused_Text>", "", ButtonArray, None,,
							MessageBox, class'GearUIScene_Base'.const.GEAR_SCENE_PRIORITY_COOPSPECTATE );
			if (MessageBox != None)
			{
				MessageBox.OnRawInputKey = ProcessClientPauseKey;
			}

			//@hack: periodically check that we didn't get out of sync somehow
			// note that this obviously won't update while paused, which is perfect
			// since we should be able to have the menu up incorrectly when unpaused
			// but we can't just check for that because we might simply be behind on variable replication
			SetTimer(0.5, true, nameof(ServerVerifyPausedState));
		}
		else
		{
			PauseScene = GameSceneClient.FindSceneByTag('HostPaused', None);
			if (PauseScene != None)
			{
				GameSceneClient.CloseScene(PauseScene);
			}
			ClearTimer(nameof(ServerVerifyPausedState));
		}
	}
}


/** Function to handle entering a UI scene that wants to hide the HUD */
simulated function HideHUDCleanup()
{
}

/** Function to handle coming back from a screen that wants to hide the hud */
simulated function UnHideHUDCleanup()
{
}

/**
 * Set the colorization settings for post process
 */
simulated function SetGameoverPostProcessColorizationSettings( out PostProcessSettings PPSettings )
{
	PPSettings.bEnableSceneEffect = true;
	PPSettings.Scene_InterpolationDuration = 1.f;
	PPSettings.Scene_Desaturation = 0.f;
	PPSettings.Scene_HighLights = vect( 0.5f, 1.f, 1.f );
	PPSettings.Scene_MidTones = vect( 1.f, 3.5f, 3.5f );
	PPSettings.Scene_Shadows = vect( 0.005f, 0.005f, 0.005f );
}

/**
* Get the post process settings for the gameover screen.
*/
simulated function InitGameoverPostProcessSettings()
{
	SetPostProcessBlurSettings( GameoverPPSettings );
	SetGameoverPostProcessColorizationSettings( GameoverPPSettings );
}

/**
 * Attaches a post-process effect to the player's view.
 *
 * @param	PPType			the post-process type to attach.
 * @param	bAllPlayers		specify TRUE to force all local players to apply the post-process effect (only relevant in split-screen).
 */
simulated function StartPostProcessOverride( GearPostProcessFXType PPType, optional bool bAllPlayers, optional bool bRecoverFromPause )
{
	local LocalPlayer LP;
	local GearPC PC;

	if (PPType == CurrentPPType)
	{
		return;
	}
	CurrentPPType = PPType;

	`log(GetFuncName()@`showvar(PPType));

	switch ( PPType )
	{
	case eGPP_Gameover:
		LP = LocalPlayer(Player);
		LP.OverridePostProcessSettings( GameoverPPSettings, WorldInfo.RealTimeSeconds );
		break;

	case eGPP_Pause:
		if ( bAllPlayers )
		{
			foreach LocalPlayerControllers(class'GearPC', PC)
			{
				// hide the hud
				PC.HideHUDCleanup();
				LP = LocalPlayer(PC.Player);
				LP.OverridePostProcessSettings( PC.PausePPSettings, WorldInfo.RealTimeSeconds );
			}
		}
		else
		{
			LP = LocalPlayer(Player);

			HideHUDCleanup();
			LP.OverridePostProcessSettings(PausePPSettings, WorldInfo.RealTimeSeconds);
		}
		break;

	case eGPP_BleedOut:
		LP = LocalPlayer(Player);
		if (bRecoverFromPause)
		{
			MyGearHud.SetBleedOutEffectSettings( FMax(0.1f,GearGRI(WorldInfo.GRI).InitialRevivalTime - TimeSince(MyGearPawn.TimeOfDBNO)) );
		}
		else
		{
			MyGearHud.SetBleedOutEffectSettings( GearGRI(WorldInfo.GRI).InitialRevivalTime );
		}
		LP.OverridePostProcessSettings( MyGearHud.BleedOutPPSettings, WorldInfo.RealTimeSeconds );
		break;
	}
}

/** Stop the post process effect */
simulated function StopPostProcessOverride( GearPostProcessFXType PPType )
{
	local LocalPlayer LP;
	local GearPC PC;

	if (PPType != CurrentPPType)
	{
		`log("Warning, attempting to stop invalid post process type:"@PPType@`showvar(CurrentPPType));
		return;
	}
	CurrentPPType = eGPP_None;

	LP = LocalPlayer(Player);
	switch ( PPType )
	{
		case eGPP_Pause:
			if ( MyGearHUD.GetPauseSceneReference().GetSceneRenderMode() == SPLITRENDER_Fullscreen )
			{
				ForEach LocalPlayerControllers(class'GearPC', PC)
				{
					LP = LocalPlayer(PC.Player);
					LP.ClearPostProcessSettingsOverride();
					// unhide the hud
					PC.UnHideHUDCleanup();
					// might have to start another post process
					CheckForPostProcessReset(PC);
				}
			}
			else
			{
				LP.ClearPostProcessSettingsOverride();
				UnhideHUDCleanup();
				CheckForPostProcessReset(Self);
			}
			break;

		case eGPP_BleedOut:
		case eGPP_Gameover:
			LP.ClearPostProcessSettingsOverride();
			break;
	}
}

// if the game was paused while this person was dead, restore the PP settings.
simulated function CheckForPostProcessReset( GearPC PC )
{
	local GearPawn GP;
	`log(GetFuncName()@`showvar(PC));
	if (PC != None)
	{
		GP = GearPawn(PC.Pawn);
		if (GP != None && GP.Health <= 0)
		{
			if (GP.IsDBNO())
			{
				PC.StartPostProcessOverride( eGPP_BleedOut, FALSE, TRUE );
			}
			else
			{
				PC.StartPostProcessOverride( eGPP_Gameover );
			}
		}
		else if (PC.MyGearHUD != None)
		{
			if ( PC.MyGearHUD.PauseUISceneInstance != None )
			{
				PC.StartPostProcessOverride( eGPP_Pause, PC.MyGearHUD.PauseUISceneInstance.GetSceneRenderMode() == SPLITRENDER_Fullscreen );
			}
		}
	}
}

function SpawnDefaultHUD()
{
	if ( LocalPlayer(Player) == None )
		return;

	MyGearHUD = Spawn(class'GearHUD_Base',self);
	MyHud = MyGearHud;
}

/**
 * This should probably put you into CinematicMode
 *
 * @see OnToggleCinematicMode
 **/
simulated event ShowCinematic()
{
	MyGearHud.PlayCinematic();
}

/** This will display an ActionInfo for the downed teammate. **/
exec function ShowDownedTeammateAction(GearPawn WP)
{
	if ( (WP != None) && ((MyGearHud.ActiveAction.ActionType != AT_StayingAlive) || (MyGearHud.ActiveAction.ActionType != AT_SuicideBomb)) )
	{
		PlaySound( PointOfInterestAdded );
		ActionLookAtDownedTeammate.ActionIconDatas[ActionLookAtDownedTeammate.ActionIconDatas.length-1].ActionIcons[0] = WP.HeadIcon;
		MyGearHud.SetActionInfo(AT_DownedTeammate, ActionLookAtDownedTeammate);
		SetTimer( 3.0f, FALSE, nameof(ClearDownedTeammateAction) );
	}
}

/** This will clear the Downed Teammate action info **/
exec function ClearDownedTeammateAction()
{
	MyGearHud.ClearActionInfoByType(AT_DownedTeammate);
}


native function Shot_360();

exec function Mofo()
{
	Shot_360();
}


/**
 * Sets invis mode based on the activated link.
 */
function OnToggleInvisMode( SeqAct_ToggleInvisMode inAction )
{
	if( inAction.InputLinks[0].bHasImpulse )
	{
		bInvisible = TRUE;
	}
	else
	if( inAction.InputLinks[1].bHasImpulse )
	{
		bInvisible = FALSE;
	}
	else
	{
		bInvisible = !bInvisible;
	}
}

/** starts playing the specified movie */
native final reliable client event ClientPlayMovie(string MovieName);

/**
 * Stops the currently playing movie
 *
 * @param	DelayInSeconds			number of seconds to delay before stopping the movie.
 * @param	bAllowMovieToFinish		indicates whether the movie should be stopped immediately or wait until it's finished.
 * @param	bForceStopNonSkippable	indicates whether a movie marked as non-skippable should be stopped anyway; only relevant if the specified
 *									movie is marked non-skippable (like startup movies).
 * @param	bForceStopLoadingMovie	If false then don't stop the movie if it's the loading movie.
 */
native final reliable client event ClientStopMovie(float DelayInSeconds, bool bAllowMovieToFinish, bool bForceStopNonSkippable, bool bForceStopLoadingMovie);

/** returns the name of the currently playing movie, or an empty string if no movie is currently playing
 * @todo: add an out param for current time in playback for synchronizing clients that join in the middle
 */
native final function GetCurrentMovie(out string MovieName);

/**
 * Sets the current gamma value.
 *
 * @param New Gamma Value, must be between 0.0 and 1.0
 */
native function SetGamma(float GammaValue);

/**
 * Sets the post process settings for this player.
 *
 * @param Preset	Preset to use to set the 4 PP values.
 */
native function SetPostProcessValues(ETVType Preset);

/**
 * @return Returns the index of this PC in the GamePlayers array.
 */
native function int GetUIPlayerIndex();

/**
 * Save the profile
 * WriteProfileSettingsCompleteDelegate - optional parameter that will allow a callback to be called when the save has completed
 *		- DELGATE MUST BE CLEARED ONCE CALLED
 */
function SaveProfile( optional delegate<OnlinePlayerInterface.OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate=OnProfileWriteComplete )
{
	local OnlinePlayerInterface PlayerInt;
	local int ControllerId;

	ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetUIPlayerIndex());

	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			if ( ControllerId != INDEX_NONE )
			{
				if (!HasSigninChanged())
				{
					PlayerInt.AddWriteProfileSettingsCompleteDelegate(ControllerId,WriteProfileSettingsCompleteDelegate);
`log("-------> TEMP SAVE PROFILE TRACKING");
ScriptTrace();
					PlayerInt.WriteProfileSettings(ControllerId, ProfileSettings);
				}
				else
				{
					`Log("Ignoring write request because the profiles have changed");
					// Not signed in, so just act like it worked
					WriteProfileSettingsCompleteDelegate(ControllerId, true);
				}
			}
		}
	}
	else
	{
		if ( WriteProfileSettingsCompleteDelegate != None )
		{
			WriteProfileSettingsCompleteDelegate(ControllerId, true);
		}
	}

	// so this saveprofile gets called when things have been updated.  When that occurs we need to call this function
	// which will set all of the correct runtime settings.
	UpdateLocalCacheOfProfileSettings();
	bProfileSettingsUpdated = FALSE;
}

/** Clear a SaveProfile delegate from the online player interface */
function ClearSaveProfileDelegate( delegate<OnlinePlayerInterface.OnWriteProfileSettingsComplete> WriteProfileSettingsCompleteDelegate )
{
	local OnlinePlayerInterface PlayerInt;
	local int ControllerId;

	if ( WriteProfileSettingsCompleteDelegate != None )
	{
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if (PlayerInt != None)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetUIPlayerIndex());
				if ( ControllerId != INDEX_NONE )
				{
					PlayerInt.ClearWriteProfileSettingsCompleteDelegate(ControllerId,WriteProfileSettingsCompleteDelegate);
				}
			}
		}
	}
}

/** Called when the profile is done writing */
function OnProfileWriteComplete(byte LocalUserNum,bool bWasSuccessful)
{
	`log(`location @ `showvar(LocalUserNum) @ `showvar(bWasSuccessful));
	ClearSaveProfileDelegate(OnProfileWriteComplete);
}

/**
 * Sends the profile data up to MCP for stats gathering
 */
function UploadProfileToMCP()
{
	local OnlineEventsInterface Uploader;
	local UniqueNetId UniqueId;
	local int ControllerId;

	if (OnlineSub != None)
	{
		// Ask for the interface by name and cast to our well known type
		Uploader = OnlineEventsInterface(OnlineSub.GetNamedInterface('McpUpload'));
		if (Uploader != None)
		{
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(GetUIPlayerIndex());
			OnlineSub.PlayerInterface.GetUniquePlayerId(ControllerId,UniqueId);
			Uploader.UploadProfileData(UniqueId,OnlineSub.PlayerInterface.GetPlayerNickname(ControllerId),ProfileSettings);
			// Upload the hardware stats if they haven't been done yet
			if (ProfileSettings != None && !ProfileSettings.HaveHardwareStatsBeenUploaded())
			{
				Uploader.UploadHardwareData(UniqueId,OnlineSub.PlayerInterface.GetPlayerNickname(ControllerId));
				// Mark the profile as having uploaded
				ProfileSettings.MarkHardwareStatsAsUploaded();
			}
		}
	}
}

/**
 * Copies profile settings from the datastore and stores them in local variables.
 */
exec function UpdateLocalCacheOfProfileSettings()
{
	local int PlayerIndex;
	local PlayerController PC;
	local bool bEnableGore;
	local bool bUsingAutoTutorials;
	local GearPRI GPRI;
	local StaticMeshActorBasedOnExtremeContent SMAExtremeContent;
	local SkeletalMeshActorBasedOnExtremeContent SKAExtremeContent;

	PlayerIndex = GetUIPlayerIndex();

	`Log("Updating Local Cache of Profile Settings for UI PlayerIndex " $ PlayerIndex);
	GPRI = GearPRI(PlayerReplicationInfo);

	// the crazy the dynamic / static hud on whether to draw the weapon indicator or not (cept in cines)
	GPRI.bAlwaysDrawWeaponIndicatorInHUD = !ProfileSettings.GetHudOnOffConfigOption();

	GPRI.TrigConfig	= ProfileSettings.GetTrigConfigOption();
	GPRI.StickConfig	= ProfileSettings.GetStickConfigOption();

	UpdateControllerConfig();

	GPRI.bShowPictograms = ProfileSettings.GetPictogramTooltipsConfigOption();
	SetPreferredCharacters( ProfileSettings.GetPreferredClassIndex(0), ProfileSettings.GetPreferredClassIndex(1) );
	GPRI.ControllerSensitivityConfig = ProfileSettings.GetControllerSensitivityValue();
	GPRI.TargetSensitivityConfig = ProfileSettings.GetTargetSensitivityValue();
	GPRI.ZoomSensitivityConfig = ProfileSettings.GetZoomSensitivityValue();

	PlayerReplicationInfo.bControllerVibrationAllowed	= ProfileSettings.GetControllerVibrationOption();

	UpdateControlScheme();

	UpdateDifficultySetting();

	// Allow tutorial system to auto-initialize it's tutorials
	if ( TutorialMgr != None )
	{
		bUsingAutoTutorials = ProfileSettings.GetTutorialConfigOption();
		TutorialMgr.bGameIsUsingAutoTutorials = bUsingAutoTutorials;
		if (bUsingAutoTutorials)
		{
			TutorialMgr.AddAutoInitiatedTutorials();
		}
		else
		{
			TutorialMgr.RemoveAutoInitiatedTutorials();
		}
	}

	// Only allow player 0 to set shared options.
	if ( PlayerIndex == 0 )
	{
		RestoreShowSubtitles( );

		SetAudioGroupVolume( 'Voice', ProfileSettings.GetDialogVolume() );
		SetAudioGroupVolume( 'Effects', ProfileSettings.GetFxVolume() );
		SetAudioGroupVolume( 'Music', ProfileSettings.GetMusicVolume() );

		SetGamma(ProfileSettings.GetGammaSetting());
		SetAllowMatureLanguage( ProfileSettings.GetMatureLanguageConfigOption() );

		bEnableGore	= ProfileSettings.GetGoreConfigOption();
		// Loop through all PRI's and set gore.
		foreach LocalPlayerControllers(class'PlayerController', PC)
		{
			GearPRI(PC.PlayerReplicationInfo).bShowGore = bEnableGore;
		}

		// this is rough, but after each change of the extreme content flag we need to iterate over all BasedOnExtremeContent actors
		// and all their function
		foreach AllActors( class'StaticMeshActorBasedOnExtremeContent', SMAExtremeContent )
		{
			SMAExtremeContent.SetMaterialBasedOnExtremeContent();
		}

		foreach AllActors( class'SkeletalMeshActorBasedOnExtremeContent', SKAExtremeContent )
		{
			SKAExtremeContent.SetMaterialBasedOnExtremeContent();
		}


		if ( LocalPlayer(Player) != None )
		{
			LocalPlayer(Player).ViewportClient.SetSplitscreenConfiguration(ProfileSettings.GetPreferredSplitscreenType());
		}
	}

	SetPostProcessValues(ProfileSettings.GetTVType());

	// See if any discoverables need to be made visible
	CheckForDiscoverableVisibility();

	// Make sure the currently selected weapon still exists (DLC could've been removed)
	CheckForWeaponAvailability();
}

/** Make sure the currently selected weapon still exists (DLC could've been removed), reset to default if not found */
function CheckForWeaponAvailability()
{
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> WeaponList;
	local GearGameWeaponSummary WeaponInfo;
	local int Idx;
	local int CurrWeaponId;
	local bool bFoundWeapon;

	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if (GameResourceDS != None)
	{
		// Get the current setting from the profile
		CurrWeaponId = ProfileSettings.GetPreferredWeaponId();
		// Get the weapon providers
		GameResourceDS.GetResourceProviders('Weapons', WeaponList);
		// Loop through the weapon providers and see if the one we are using is still available
		for (Idx = 0; Idx < WeaponList.Length; Idx++)
		{
			WeaponInfo = GearGameWeaponSummary(WeaponList[Idx]);
			if (WeaponInfo != none)
			{
				// If we found it break out
				if (CurrWeaponId == WeaponInfo.WeaponId)
				{
					bFoundWeapon = true;
					break;
				}
			}
		}
	}
	// If we didn't find it set the profile to it's default value (Lancer)
	if (!bFoundWeapon)
	{
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.DefaultMPWeapon, 13);
	}
}

/** Sets the difficulty on the PRI from the profile.  The PRI will then apply the difficulty on the server */
final function UpdateDifficultySetting()
{
	local int ValueId;
	local class<DifficultySettings> DiffSettingsClass;
	local GearPRI MyPRI;

	if ( WorldInfo.GRI != None &&
		 !WorldInfo.GRI.IsMultiplayerGame() )
	{
		if ( PlayerReplicationInfo != None &&
			 ProfileSettings != None &&
			 ProfileSettings.GetProfileSettingValueId(ProfileSettings.GameIntensity, ValueId) )
		{
			switch ( ValueId )
			{
				case DL_Casual:
					DiffSettingsClass = class'DifficultySettings_Casual';
					break;
				case DL_Normal:
					DiffSettingsClass = class'DifficultySettings_Normal';
					break;
				case DL_Hardcore:
					DiffSettingsClass = class'DifficultySettings_Hardcore';
					break;
				case DL_Insane:
					DiffSettingsClass = class'DifficultySettings_Insane';
					break;
				default:
					`log(`location@"Illegal difficulty stored in profile!"@`showvar(ValueId));
					return;
			}

			MyPRI = GearPRI(PlayerReplicationInfo);
			if ( MyPRI != None && MyPRI.Difficulty != DiffSettingsClass )
			{
				ServerSetDifficulty( DiffSettingsClass );
			}
		}
	}
}

/** Sets the difficulty of the player on the server */
final reliable server function ServerSetDifficulty( class<DifficultySettings> DiffSettings )
{
	local GearPRI MyPRI, OtherPRI;
	local int i;

	if (WorldInfo.GRI != None && !WorldInfo.GRI.IsMultiplayerGame() && !GearGame(WorldInfo.Game).bURLDifficultyOverride)
	{
		MyPRI = GearPRI(PlayerReplicationInfo);
		if ( MyPRI != None )
		{
			MyPRI.Difficulty = DiffSettings;
			MyPRI.Difficulty.static.ApplyDifficultySettings( self );
			for (i = 0; i < WorldInfo.GRI.PRIArray.length; i++)
			{
				OtherPRI = GearPRI(WorldInfo.GRI.PRIArray[i]);
				if (OtherPRI != None && AIController(OtherPRI.Owner) != None)
				{
					OtherPRI.Difficulty = class'DifficultySettings'.static.GetLowestPlayerDifficultyLevel(WorldInfo);
					OtherPRI.Difficulty.static.ApplyDifficultySettings(Controller(OtherPRI.Owner));
				}
			}
		}
	}
}

/** activates or deactivates the alternate control scheme and if we are client notifies the server of the change */
native final function SetAlternateControls(bool bNewUseAlternateControls);

reliable server event ServerSetAlternateControls(bool bNewUseAlternateControls)
{
	if (WorldInfo.NetMode != NM_Client && LocalPlayer(Player) == None)
	{
		SetAlternateControls(bNewUseAlternateControls);
	}
}

/** Set the control scheme */
final function UpdateControlScheme()
{
	SetAlternateControls(ProfileSettings.GetControlScheme() == GCS_Alternate);
}

/**
 * This will set the bindings array with the updated key/value pair for the various controllers configs
 * we support.
 *
 * @todo:  when updating this look at the UTConsolePlayerController  LoadSettingsFromProfile
 **/
function UpdateControllerConfig()
{
	local GearPRI PRI;

	if (ProfileSettings != none)
	{
		PlayerInput.bInvertMouse = ProfileSettings.GetInvertMouseOption();
		PlayerInput.bInvertTurn = ProfileSettings.GetInvertTurnOption();
	}

	PRI = GearPRI(PlayerReplicationInfo);
	if (PRI != None)
	{
		if( PRI.StickConfig == WSCO_Default )
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftX', "GBA_Strafe" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftY', "GBA_Movement" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightX', "GBA_LookX" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightY', "GBA_LookY" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftThumbstick', "GBA_Crouch" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightThumbstick', "GBA_Zoom" );
		}
		else if( PRI.StickConfig == WSCO_Legacy )
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftX', "GBA_LookX" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftY', "GBA_Movement" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightX', "GBA_Strafe" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightY', "GBA_LookY" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftThumbstick', "GBA_Zoom" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightThumbstick', "GBA_Crouch" );
		}
		else if( PRI.StickConfig == WSCO_SouthPaw )
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftX', "GBA_LookX" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftY', "GBA_LookY_Flip" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightX', "GBA_Strafe" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightY', "GBA_Movement_Flip" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftThumbstick', "GBA_Zoom" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightThumbstick', "GBA_Crouch" );
		}
		else if( PRI.StickConfig == WSCO_LegacySouthpaw )
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftX', "GBA_Strafe" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftY', "GBA_LookY_Flip" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightX', "GBA_LookX" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightY', "GBA_Movement_Flip" );
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftThumbstick', "GBA_Crouch" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightThumbstick', "GBA_Zoom" );
		}

		if ( PRI.TrigConfig == WTCO_SouthPaw )
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftTrigger', "GBA_Shoot" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightTrigger', "GBA_Target" );

			UpdateControllerSettings_Worker( 'XboxTypeS_LeftShoulder', "GBA_Reload" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightShoulder', "GBA_Taccom" );
		}
		// use defaults
		else
		{
			UpdateControllerSettings_Worker( 'XboxTypeS_LeftTrigger', "GBA_Target" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightTrigger', "GBA_Shoot" );

			UpdateControllerSettings_Worker( 'XboxTypeS_LeftShoulder', "GBA_Taccom" );
			UpdateControllerSettings_Worker( 'XboxTypeS_RightShoulder', "GBA_Reload" );
		}
	}
}

private function UpdateControllerSettings_Worker( name TheName, string TheCommand )
{
	local int BindIndex;

	for( BindIndex = 0;BindIndex < PlayerInput.Bindings.Length; ++BindIndex )
	{
		if( PlayerInput.Bindings[BindIndex].Name == TheName )
		{
			PlayerInput.Bindings[BindIndex].Command = TheCommand;
		}
	}
}


/** Set the preferred characters to the server PRI */
reliable server function ServerSetPreferredCharacters( int COGIndex, int LocustIndex )
{
	GearPRI(PlayerReplicationInfo).PreferredCOGIndex = COGIndex;
	GearPRI(PlayerReplicationInfo).PreferredLocustIndex = LocustIndex;
}

/** Set the preferred characters to the PRI */
function SetPreferredCharacters( int COGIndex, int LocustIndex )
{
	GearPRI(PlayerReplicationInfo).PreferredCOGIndex = COGIndex;
	GearPRI(PlayerReplicationInfo).PreferredLocustIndex = LocustIndex;

	if ( Role < ROLE_Authority )
	{
		ServerSetPreferredCharacters( COGIndex, LocustIndex );
	}
}

/** Returns the preferred MP character and weapon for this player */
reliable client function ClientSetPreferredMPOptions(int TeamIndex, optional bool bDoWeaponOnly)
{
	local int CharacterIndex;
	local int ProviderIndex;
	local class<GearPawn> PrefPawn;
	local int PrefWeapon;

	// Make sure the team index is valid
	if (TeamIndex >= 0 && TeamIndex < 255)
	{
		PrefPawn = none;
		if (!bDoWeaponOnly)
		{
			// Set the preferred character class
			CharacterIndex = ProfileSettings.GetPreferredClassIndex(TeamIndex);
			ProviderIndex = class'GearGame'.static.FindCharacterProviderIndex(TeamIndex, CharacterIndex);
			if (ProviderIndex != INDEX_NONE)
			{
				PrefPawn = class'GearGame'.static.GetCharacterProviderClass(TeamIndex, ProviderIndex);
			}
		}

		// Set the preferred weapon class
		PrefWeapon = ProfileSettings.GetPreferredWeaponId();

		// Set the values on the server
		ServerSetPreferredMPOptions(PrefPawn, PrefWeapon);
	}
}

/** Set the preferred MP character and weapon for this player on the server */
reliable server function ServerSetPreferredMPOptions(class<GearPawn> PrefPawn, int PrefWeapon)
{
	if (PrefPawn != none)
	{
		GearPRI(PlayerReplicationInfo).SetPawnClass(PrefPawn);
	}
	GearPRI(PlayerReplicationInfo).InitialWeaponType = PrefWeapon;
}

/** Debug function to print out the profile settings **/
exec function ShowProfileSettings()
{
	ClientMessage( "ShowGore Profile: " $ ProfileSettings.GetGoreConfigOption() );
	ClientMessage( "ShowGore Game: " $ GearPRI(PlayerReplicationInfo).bShowGore );
	ClientMessage( "AllowMatureLanguage: " $ ProfileSettings.GetMatureLanguageConfigOption() );
}

event NotifyLoadedWorld(name WorldPackageName, bool bFinalDest)
{
	local GearPC PC;
	local LocalPlayer LP;

	// clear out any input locks we managed to carry through the level change
	bIgnoreMoveInput = 0;
	bIgnoreLookInput = 0;
	InputIsDisabledCount = 0;
	bCinematicMode = false;
	bInvisible = FALSE;

	// look for the initial battle cam
	LocallySetInitialViewTarget();

	// if we didn't find a view target then default to normal behavior
	if (ViewTarget == None)
	{
		Super.NotifyLoadedWorld(WorldPackageName,bFinalDest);
	}

	// Blur the background
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		LP = LocalPlayer(PC.Player);
		LP.OverridePostProcessSettings( PC.PausePPSettings, WorldInfo.RealTimeSeconds );
	}

	SetTimer(8.f,FALSE,nameof(ClearBlurPP));
}

simulated function ClearBlurPP()
{
	local LocalPlayer LP;
	local GearPC PC;

	// Turn off the background blur
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		LP = LocalPlayer(PC.Player);
		LP.ClearPostProcessSettingsOverride();
	}
}

final unreliable client function ClientPlayCameraAnim
(
	Name					AnimName,
	optional	float		Rate=1.f,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride,
	optional	bool		bSingleRayPenetrationOnly,
	optional	bool		bApplyFullMotion,
	optional	bool		bForceLevelCamera
 )
{
	if (MyGearPawn != None)
	{
		MyGearPawn.PlayCustomCameraBoneAnim(AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride, bSingleRayPenetrationOnly, bApplyFullMotion);

		if (bForceLevelCamera)
		{
			MainPlayerInput.ForcePitchCentering(TRUE, FALSE);
		}
	}
}

simulated function CameraBoneAnimEndNotification()
{
	MainPlayerInput.ForcePitchCentering(FALSE);
}

/** Returns index of chosen camera anim, or -1 if nothing suitable could be found. */
private native function int ChooseCameraBoneAnim(const out array<CameraBoneAnimation> Anims);

/**
 * Returns index of chosen camera anim, or -1 if nothing suitable could be found.
 * bDoNotRandomize: start from index 0.
 */
simulated native final function int		ChooseRandomCameraAnim(out const array<CameraAnim> Anims, optional FLOAT Scale = 1.f, optional bool bDoNotRandomize);
/**
 * Returns true if there is enough space to play the camera anim
 */
simulated native final function bool	CameraAnimHasEnoughSpace( CameraAnim Anim, optional float  Scale = 1.f );

final function PlayRandomCameraBoneAnim
(
	const out array<CameraBoneAnimation> Anims,
	optional	float		Rate=1.f,
	optional	float		BlendInTime,
	optional	float		BlendOutTime,
	optional	bool		bLooping,
	optional	bool		bOverride,
	optional	bool		bSingleRayPenetrationOnly,
	optional	bool		bApplyFullMotion,
	optional	bool		bForceLevelCamera
 )
{
	local int AnimIdx;

	if( MyGearPawn != None && MainPlayerInput != None )
	{
		AnimIdx = ChooseCameraBoneAnim(Anims);
		if (AnimIdx >= 0)
		{
			MyGearPawn.PlayCustomCameraBoneAnim(Anims[AnimIdx].AnimName, Rate, BlendInTime, BlendOutTime, bLooping, bOverride, bSingleRayPenetrationOnly, bApplyFullMotion);

			if (bForceLevelCamera)
			{
				MainPlayerInput.ForcePitchCentering(TRUE);
			}
		}
	}
}


final function StopCameraBoneAnim(optional FLOAT BlendOutTime, optional bool bForceLevelCamera)
{
	if( MyGearPawn != None )
	{
		MyGearPawn.StopCustomCameraBoneAnim(BlendOutTime);
	}

	if( bForceLevelCamera && MainPlayerInput != None )
	{
		MainPlayerInput.ForcePitchCentering(FALSE);
	}
}

simulated function Name FindBestSoundMode()
{
	// priority order
	// only do these if we're not in split screen
	if(!IsSplitscreenPlayer())
	{
		if ( (MyGearPawn != None) && MyGearPawn.IsDBNO() )
		{
			return 'DBNO';
		}

		if (IsDead() && !IsSpectating())
		{
			return 'Death';
		}

		if (bAssessMode)
		{
			return 'TacCom';
		}

		if (IsDoingSpecialMove(SM_RoadieRun))
		{
			return 'RoadieRun';
		}
	}

	if (WorldInfo.GRI != None && GearGRI(WorldInfo.GRI).SpeechManager != None && GearGRI(WorldInfo.GRI).SpeechManager.IsSpeechActive(Speech_Scripted))
	{
		return 'DialogVolumeDucking';
	}

	return 'Default';
}


/** Set the sound mode of the audio system for special EQing */
simulated protected function SetSoundMode(Name SoundMode)
{
	local AudioDevice Audio;

	CurrentSoundMode = SoundMode;

	Audio = class'Engine'.static.GetAudioDevice();
	if (Audio != None)
	{
		Audio.SetSoundMode(SoundMode);
	}
}

/** GOW specific stuff to do when HUD is toggled */
simulated function OnToggleHUD(SeqAct_ToggleHUD inAction)
{
	local GearHUD_Base WH;

	Super.OnToggleHUD(inAction);

	// if we just toggled the hud back on, remove any old messages
	if ( (myHUD != None) && myHUD.bShowHUD )
	{
		WH = MyGearHud;
		WH.ClearHUDAfterShowToggle();
	}
}


exec function DoConeHurt()
{
	local vector HitLoc, HitNormal, TraceEnd, TraceStart;
	local rotator CamRot;
	local vector CamLoc;
	local GearPawn HitWP;
	local float AimError;

	AimError = GearWeapon(Pawn.Weapon).GetPlayerAimError();
	GetPlayerViewPoint( CamLoc, CamRot );

	TraceStart = CamLoc;
	TraceEnd = TraceStart + (vector(CamRot) * 4500.0f);
	// trace out from the camera
	HitWP = GearPawn( Trace(HitLoc, HitNormal, TraceEnd, TraceStart, TRUE) );
	GearWeapon(Pawn.Weapon).ClientFlushPersistentDebugLines();
	DrawDebugLine( TraceStart+vect(0,0,5), TraceStart + vector(CamRot) * 4500.0f, 255, 1, 1, true );

	ConeHurt( HitWP, CamLoc, HitNormal, AimError );
}


native function ConeHurt( GearPawn TargetPawn, vector CamLoc, vector HitNormal, float AimError );

/** test function to repair all wardestructible objects */
exec function RepairWDOs()
{
	local GearDestructibleObject WDO;

	foreach AllActors(class'GearDestructibleObject', WDO)
	{
		WDO.UnDestroy();
	}
}

/** Spawn a camera lens effect (e.g. blood).*/
unreliable client event ClientSpawnCameraLensEffect( class<EmitterCameraLensEffectBase> LensEffectEmitterClass )
{
	local GearPlayerCamera Cam;

	Cam = GearPlayerCamera(PlayerCamera);
	if ( (Cam != None) )
	{
		//@hack - don't allow camera blood if chainsawing something w/o blood (this will obviously break in certain edge cases, but fixing this properly just isn't feasible at the moment)
		if (LensEffectEmitterClass != class'Emit_CameraBlood_Chainsaw' || GearWeap_AssaultRifle(Pawn.Weapon) == None || !GearWeap_AssaultRifle(Pawn.Weapon).bDisableChainsawBloodFX)
		{
			Cam.AddCameraLensEffect(class<Emit_CameraLensEffectBase>(LensEffectEmitterClass));
		}
	}
}

function OnPossessTurret( SetAct_PossessTurret inAction )
{
	local Turret T;
	local Vector EntryLoc;

	T = inAction.Turret;
	if( T != None )
	{
		EntryLoc = T.GetEntryLocation();
		EntryLoc.Z += 32.f;
		if( Pawn.SetLocation( EntryLoc ) )
		{
			if( T.DriverEnter( Pawn ) )
			{
				T.bUnableToLeave = TRUE;
			}
		}
	}
	else
	if( Turret(Pawn) != None )
	{
		T = Turret(Pawn);
		T.DriverLeave( TRUE );
	}
}

function OnChangeSquad( SeqAct_ChangeSquad Action )
{
	SetSquadName( Action.SquadName );
}

function OnAISquadController( SeqAct_AISquadController inAction )
{
	if( Squad != None )
	{
		Squad.OnAISquadController( inAction );
	}
}

event SetSquadName(name NewSquadName)
{
	local GearPRI PRI;
	local GearTeamInfo Team;

	PRI = GearPRI(PlayerReplicationInfo);
	if (PRI != None)
	{
		if( Squad != None )
		{
			Squad.UnregisterSquadMember( self );
		}

		PRI.SquadName = NewSquadName;
		Team = GearTeamInfo(Pawn.GetTeam());
		if (Team != None)
		{
			// players are always squad leaders
			Team.JoinSquad(PRI.SquadName,self,TRUE);
		}
	}
}

/**
 * Add DBNO teammate to the list
 */
simulated function AddDBNOTeammate( GearPawn WP )
{
	local int i;
	local GearPawn_COGGear COGPawn;
 	local bool bAlreadyDown;

	COGPawn = GearPawn_COGGear(WP);

	// always allow teammates to be added in MP, but in campaign all teammates must be COGs (we don't
	// want and strandeds in taccom)
	if ( WorldInfo.GRI.IsMultiplayerGame() || (COGPawn != None) )
	{
		// since there is a time period where a guy could go down while
		// the interface still shows him reviving we need to get rid of the
		// old one before adding the new one.
		for ( i=0; i<DBNOTeammates.Length; i++ )
		{
			if ( DBNOTeammates[i] == WP )
			{
				DBNOTeammates.Remove(i--, 1);
 				bAlreadyDown = true;
			}
		}

		`log(WorldInfo.TimeSeconds@"adding DBNO teammate:"@`showvar(WP));
		DBNOTeammates[DBNOTeammates.Length] = WP;

		// See if we should play a TG tutorial
		if (!bAlreadyDown &&
			WP.PlayerReplicationInfo != none &&
			MyGearPawn != none &&
			MyGearPawn.Health > 0)
		{
			AttemptTrainingGroundTutorial(GEARTUT_TRAIN_Revive, -1);
		}

// Removed to see if anyone notices the feature missing
// 		if ( !bAlreadyDown && (WP.PlayerReplicationInfo != None) )
// 		{
// 			ShowDownedTeammateAction( WP );
// 		}
	}
}

/**
 * Handle ChangeDefaultCamera Kismet action.
 */
simulated function OnChangeDefaultCamera(SeqAct_ChangeDefaultCamera Action)
{
	//@fixme JF: reimplement?  or is this not needed anymore?
	//if (Action.InputLinks[0].bHasImpulse)
	//{
	//	// start
	//	GearPlayerCamera(PlayerCamera).StartCustomDefaultCameraMode(Action.NewDefaultCamera);
	//	GearPlayerInput(PlayerInput).PitchAutoCenterHorizonOffset = Action.PitchOffsetDegrees * 182.044f;
	//}
	//else if (Action.InputLinks[1].bHasImpulse)
	//{
	//	// stop
	//	GearPlayerCamera(PlayerCamera).ClearCustomDefaultCameraMode();
	//	GearPlayerInput(PlayerInput).PitchAutoCenterHorizonOffset = 0;
	//}
}

/**
 * @return	a reference to the progress message scene, if it's already open.
 */
function UIMessageBoxBase FindUIMessageScene( optional name SceneTag='ProgressMessage' )
{
	local UIMessageBoxBase MessageScene;
	local GameUISceneClient SceneClient;

	SceneClient = class'UIInteraction'.static.GetSceneClient();
	if ( SceneClient != None )
	{
		MessageScene = UIMessageBoxBase(SceneClient.FindSceneByTag(SceneTag, LocalPlayer(Player)));
	}

	return MessageScene;
}

//static function UIMessageBoxBase CreateUIMessageBox( name SceneTag, optional class<UIMessageBoxBase> CustomMessageBoxClass=default.MessageBoxClass, optional UIMessageBoxBase SceneTemplate )
/**
 * Opens the scene which is used to display connection/download progress & error messages.  If the scene is already open, will
 * return a reference to the existing scene rather than creating another one.
 *
 * @return	a reference to an instance of UTUIScene_ConnectionStatus which is fully initialized and ready to be used.
 */
function UIMessageBoxBase OpenUIMessageScene( string Title, string Message, optional string ButtonAliasString, optional delegate<UIMessageBoxBase.OnOptionSelected> SelectionCallback,
	optional name SceneTag='ProgressMessage', optional byte ForcedPriority=0 )
{
	local int i;
	local GameUISceneClient SceneClient;
	local array<name> ButtonAliases;
	local array<string> ButtonAliasStringArray;
	local UIMessageBoxBase Result;

	ButtonAliasStringArray = SplitString(ButtonAliasString);
	for ( i = 0; i < ButtonAliasStringArray.Length; i++ )
	{
		ButtonAliases[i] = name(ButtonAliasStringArray[i]);
	}

	// make sure we have a valid scene class name
//	if ( ProgressMessageSceneClassName == "" )
//	{
//		ProgressMessageSceneClassName = "UTGame.UTUIScene_ConnectionStatus";
//	}

	// load the scene class
	SceneClient = class'UIInteraction'.static.GetSceneClient();
	if ( SceneClient != None )
	{
		ForceCloseUIMessageScene(SceneTag, true);
		SceneClient.ShowUIMessage(SceneTag, Title, Message, "", ButtonAliases, SelectionCallback, LocalPlayer(Player), Result, ForcedPriority);
	}

	return Result;
}

/**
 * Manually closes the progress message scene, if open.  Normally the progress message scene would be closed when the user
 * clicks one of its buttons.
 *
 * @param	bSimulateCancel		if TRUE, will set the message box's selection to the index of the Cancel button; otherwise,
 *								just closes the scene without touching the selection value.
 */
function ForceCloseUIMessageScene( optional name SceneTag='ProgressMessage', optional bool bSimulateCancel=true )
{
	local GameUISceneClient SceneClient;

	SceneClient = class'UIInteraction'.static.GetSceneClient();
	if ( SceneClient != None )
	{
		//@todo - implement simulating cancel
		SceneClient.ClearUIMessageScene(SceneTag, LocalPlayer(Player));
	}
}

/**
 * Sets or updates the any current progress message being displayed.
 *
 * @param	MessageType	the type of progress message
 * @param	Message		the message to display
 * @param	Title		the title to use for the progress message.
 */
reliable client function ClientSetProgressMessage( EProgressMessageType MessageType, string Message, optional string Title, optional bool bIgnoreFutureNetworkMessages )
{
	switch ( MessageType )
	{
		case PMT_Information:
		case PMT_DownloadProgress:
			break;

		case PMT_ConnectionFailure:
			if (!bIsReturningFromMatch)
			{
				if (WorldInfo.NetMode == NM_Client)
				{
					// Stash the strings and travel to main menu
					Super.ClientSetProgressMessage( MessageType, Message, Title, bIgnoreFutureNetworkMessages );
					ClientReturnToMainMenu();
				}
				else
				{
					// We must be in the main menu so display it
					NotifyConnectionError(Message, Title);
				}
			}
			break;

		case PMT_Clear:
			break;

		case PMT_SocketFailure:
			if ( !bIgnoreNetworkMessages )
			{
				// Stash the strings and travel to main menu
				Super.ClientSetProgressMessage( MessageType, Localize("Errors", "ConnectionFailed", "Engine"), Localize("Errors", "ConnectionFailed_Title", "Engine"), true );
				ClientReturnToMainMenu();
			}
			break;

		default:
			Super.ClientSetProgressMessage( MessageType, Message, Title, bIgnoreFutureNetworkMessages );
			break;
	}
	if ( !bIgnoreNetworkMessages )
	{
		bIgnoreNetworkMessages = bIgnoreFutureNetworkMessages;
	}
}

reliable simulated client event ClientWasKicked()
{
	bWasKicked = true;
	ClientSetProgressMessage(PMT_ConnectionFailure, Localize("GearPC", "KickedMsg", "GearGame"), Localize("GearPC", "NetworkDisconnectTitle", "GearGame"));
}

/** @return determines whether we should ignore or handle network errors */
native function bool ShouldIgnoreNetworkErrors();

/**
 * Notifies the player that an attempt to connect to a remote server failed, or an existing connection was dropped.
 *
 * @param	Message		a description of why the connection was lost
 * @param	Title		the title to use in the connection failure message.
 */
function NotifyConnectionError( optional string Message=Localize("Errors", "ConnectionFailed", "Engine"), optional string Title=Localize("Errors", "ConnectionFailed_Title", "Engine") )
{
	local UIMessageBoxBase MessageScene;

	`log(`location @ `showvar(bIgnoreNetworkMessages,Ignored) @ `showvar(ShouldIgnoreNetworkErrors(),NativeIgnored) @ `showvar(Title) @ `showvar(Message) @ `showenum(ENetMode,WorldInfo.NetMode,NetMode) @ `showvar(GetURLMap(),Map) ,,'DevNet');

	if ( ShouldIgnoreNetworkErrors() )
	{
		return;
	}

	bIgnoreNetworkMessages = true;

	// server may have paused game before disconnecting, let's reset that for cleanup
	WorldInfo.Pauser = None;

	// make sure there's no loading movie over the message box
	ClientShowLoadingMovie(FALSE);

	if ( MyGearHud != None )
	{
		MyGearHud.bShowScores = FALSE;
	}

	if ( Title == "" )
	{
		Title = "<Strings:Engine.Errors.ConnectionLost>";
	}

	MessageScene = OpenUIMessageScene(Title, Message, "GenericAccept", CancelPendingConnection, 'ConnectionError', class'UIScene'.const.DEFAULT_SCENE_PRIORITY + 5);
	MessageScene.bPauseGameWhileActive = false;
	MessageScene.bExemptFromAutoClose = true;
	MessageScene.bCloseOnLevelChange = false;
	MessageScene.bMenuLevelRestoresScene = true;
}

/**
 * Shows a message box with generic accept as the button
 *
 * @param Title the title to display
 * @param Message the message to show in the box body
 * @param TagName the tag name for the UI scene for finding, etc.
 */
function ShowMessageBox(string Title, string Message, name TagName)
{
	local UIMessageBoxBase MessageScene;

	`Log("ShowMessageBox("$Title$","$Message$","$TagName$")");
	MessageScene = OpenUIMessageScene(Title, Message, "GenericAccept", None, TagName, class'UIScene'.const.DEFAULT_SCENE_PRIORITY + 5);
	MessageScene.bPauseGameWhileActive = false;
	MessageScene.bExemptFromAutoClose = true;
	MessageScene.bCloseOnLevelChange = false;
	MessageScene.bMenuLevelRestoresScene = true;
}

/**
 * Handler for the ProgressMessageScene's OnSelection delegate.  Kills any existing online game sessions.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
simulated function bool CancelPendingConnection( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	ForceCloseUIMessageScene();
	ConsoleCommand("CANCEL");

	ReturnToMainMenu();
	return true;
}

/**
 * Iterates over the PRI array and unregisters voice for all players
 */
simulated function ClearVoiceRegistrants()
{
	local int PriIndex;
	local PlayerReplicationInfo PRI;
	local UniqueNetId ZeroId;

	if (OnlineSub != None &&
		OnlineSub.VoiceInterface != None)
	{
		// Remove each registered player from the voice channels
		for (PriIndex = 0; PriIndex < WorldInfo.GRI.PRIArray.Length; PriIndex++)
		{
			PRI = WorldInfo.GRI.PRIArray[PriIndex];
			if (PRI.UniqueId != ZeroId)
			{
				OnlineSub.VoiceInterface.UnregisterRemoteTalker(PRI.UniqueId);
			}
		}
	}
}

/**
 * Used when a host is telling a client to return to their party host from the
 * current session. It looks for the session named 'Party' and does a travel to
 * it. If it's not available, it just does a "disconnect"
 */
reliable client function ClientReturnToParty()
{
	local string URL;
	local OnlineGameSettings Party;
	local OnlineGameSettings GameSettings;
	local GearRecentPlayersList PlayersList;

// Temp, will be removed later
`Log("");
`Log("");
`Log("Client has been told to return to party");

	// Unregister all voice registrants
	ClearVoiceRegistrants();

	bIsReturningFromMatch = true;
	bIgnoreNetworkMessages = true;
	// Show the loading movie
	ClientShowLoadingMovie(true);

	// make sure we're not paused
	if (MyGearHUD != None && MyGearHUD.PauseUISceneInstance != None)
	{
		MyGearHUD.UnPauseGame(MyGearHUD.PauseUISceneInstance);
	}
	else
	{
		SetPause(false);
	}

	if (OnlineSub != None &&
		OnlineSub.GameInterface != None &&
		OnlineSub.PlayerInterface != None)
	{
		if (IsPrimaryPlayer())
		{
`Log(self$" is the primary player");
			GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
			if (GameSettings != None &&
				WorldInfo.NetMode == NM_Client)
			{
`Log(self$" destroying 'Game'");
				OnlineSub.GameInterface.DestroyOnlineGame('Game');
			}
			else
			{
`Log(self$" is skipping destroying game for server to avoid secure key loss");
			}
			Party = OnlineSub.GameInterface.GetGameSettings('Party');
			// Find the party settings to verify that a party is registered
			if (Party != None)
			{
				// Now see if we are the party host or not
				if (IsPartyLeader())
				{
					UnregisterRemotePlayers();
					// We are the party host so create the session and listen for returning clients
					URL = GetPartyMapName() $ "?game=" $ GetPartyGameTypeName() $ "?listen";
`Log(self$" is the party leader and is traveling with "$URL);
					// Transition to being the party host without notifying clients and traveling absolute
					WorldInfo.ServerTravel(URL,true,true);
				}
				else
				{
					// See if the party leader completed the match or not
					PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
					// Local and system link this is none
					if (GameSettings == None ||
						// Private match for this case
						!GameSettings.bUsesArbitration ||
						// Horde match for this one
						(PlayersList != None && PlayersList.LastMatchResults == None) ||
						// Public match for this one
						(PlayersList != None && PlayersList.DidPlayerCompleteMatch(Party.OwningPlayerId)))
					{
						// We are joining so grab the connect string to use
						if (OnlineSub.GameInterface.GetResolvedConnectString('Party',URL))
						{
`Log(self$" is the NOT the party leader and is traveling with "$URL);
							ClientTravel(URL, TRAVEL_Absolute);
						}
						else
						{
							OnlineSub.GameInterface.DestroyOnlineGame('Party');
							ClientTravel("?failed", TRAVEL_Absolute);
						}
					}
					else
					{
`Log(self$" is the NOT the party leader and their party leader didn't finish the match...returning to main menu");
						OnlineSub.GameInterface.DestroyOnlineGame('Party');
						ClientTravel("?failed", TRAVEL_Absolute);
					}
				}
			}
			// If we are splitscreen or local, then return to the lobby
			else
			{
				URL = GetPartyMapName() $ "?game=" $ GetPartyGameTypeName() $ "?listen";
				// Transition to being the party host without notifying clients and traveling absolute
				WorldInfo.ServerTravel(URL,true,true);
			}
		}
		else
		{
`Log(self$" is not the primary player and is doing nothing");
		}
	}
`Log("");
`Log("");
}

/**
 * Since LoadMap does not destroy PCs, we need to manually remove players
 */
simulated function UnregisterRemotePlayers()
{
	local GearPC PC;
	local UniqueNetId ZeroId;

	if (OnlineSub != None &&
		OnlineSub.GameInterface != None)
	{
		foreach WorldInfo.AllControllers(class'GearPC',PC)
		{
			// Don't do this for local players
			if (!PC.IsLocalPlayerController())
			{
				// If they are a valid player, remove them from the session
				if (PC.PlayerReplicationInfo != None &&
					PC.PlayerReplicationInfo.UniqueId != ZeroId)
				{
					OnlineSub.GameInterface.UnregisterPlayer('Party',PC.PlayerReplicationInfo.UniqueId);
				}
			}
		}
	}
}

/**
 * Tell this player to return to the main menu.  This version takes care of cleaning up the player's online sessions.
 */
simulated function ReturnToMainMenu()
{
	local OnlineGameSettings GameSettings;
	local bool bShouldTravel;
	local GearRecentPlayersList PlayersList;

	bShouldTravel = true;
	bIsReturningFromMatch = true;
	bIgnoreNetworkMessages = true;
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
		if (GameSettings != None)
		{
			// Write arbitration data, if required
			if (WorldInfo.NetMode == NM_Client &&
				GameSettings.bUsesArbitration)
			{
				PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
				if (PlayersList != None)
				{
					// Don't write stats or scores if not in progress
					if (GameSettings.GameState == OGS_InProgress)
					{
						bShouldTravel = false;
						// Write out our version of the scoring before leaving
						ClientWriteLeaderboardStats(PlayersList.LastStatsWriteClass);
						ClientWriteOnlinePlayerScores(WorldInfo.GRI.GameClass != None ? WorldInfo.GRI.GameClass.default.ArbitratedLeaderBoardId : 0);
						// Set the end delegate, where we'll destroy the game and then return to the menu
						OnlineSub.GameInterface.AddEndOnlineGameCompleteDelegate(OnEndGameForArbitratedWriteComplete);
						OnlineSub.GameInterface.EndOnlineGame('Game');
					}
					else
					{
						OnlineSub.GameInterface.DestroyOnlineGame('Game');
					}
				}
			}
			else
			{
				// It's ok to clean up immediately
				OnlineSub.GameInterface.DestroyOnlineGame('Game');
			}
		}
		if (OnlineSub.GameInterface.GetGameSettings('Party') != None)
		{
			OnlineSub.GameInterface.DestroyOnlineGame('Party');
		}
	}

	if (bShouldTravel)
	{
		ClientTravel("?Closed", TRAVEL_Absolute);
	}
}

/**
 * Called once the ending (flushing) of the arbitrated stats has completed
 *
 * @param SessionName the name of the session that was ended
 * @param bWasSuccessful whether the call succeeded or not
 */
function OnEndGameForArbitratedWriteComplete(name SessionName,bool bWasSuccessful)
{
	// Set the end delegate, where we'll destroy the game and then join
	OnlineSub.GameInterface.ClearEndOnlineGameCompleteDelegate(OnEndGameForArbitratedWriteComplete);

	OnlineSub.GameInterface.DestroyOnlineGame('Game');
	if (OnlineSub.GameInterface.GetGameSettings('Party') != None)
	{
		OnlineSub.GameInterface.DestroyOnlineGame('Party');
	}
	ClientTravel("?closed", TRAVEL_Absolute);
}

/** Callable version from the server to force the client out */
reliable client function ClientReturnToMainMenu()
{
	ReturnToMainMenu();
}

/** Client function to start the Countdown */
reliable client function StartCountdownTime(float CountdownAmount)
{
	if ( MyGearHud != None )
	{
		MyGearHud.CountdownEndTime = WorldInfo.TimeSeconds + CountdownAmount;
		MyGearHud.CountdownStartTime = WorldInfo.TimeSeconds;
		MyGearHud.CountdownStopTime = 0;
		MyGearHud.CountdownTimeToDraw = MyGearHud.CountdownEndTime;
	}
}

/** Client function to clear the Countdown */
reliable client function ClearCountdownTime(bool bInstantClear)
{
	if ( MyGearHud != None )
	{
		if ( bInstantClear )
		{
			MyGearHud.CountdownEndTime = 0.0f;
			MyGearHud.CountdownStartTime = 0.0f;
			MyGearHud.CountdownStopTime = 0.0f;
			MyGearHud.CountdownTimeToDraw = 0.0f;
		}
		else
		{
			MyGearHud.CountdownStopTime = WorldInfo.TimeSeconds;
		}
	}
}

/**
 * Retrieve the rotation sensitivity multiplier based on sensitivity level
 */
final simulated function float GetSensitivityMultiplier()
{
	local GearPRI WPRI;
	local bool bIsZooming;

	WPRI = GearPRI(PlayerReplicationInfo);
	bIsZooming = (MyGearPawn != None && MyGearPawn.bIsZoomed);

	if ( WPRI != None )
	{
		// See which sensitivity config we should use then return it
		if ( bIsZooming )
		{
			switch ( WPRI.ZoomSensitivityConfig )
			{
				case PCSO_Medium:	return ZoomRotationSensitivityMedium;
				case PCSO_Low:		return ZoomRotationSensitivityLow;
				case PCSO_High:		return ZoomRotationSensitivityHigh;
				default:			return ZoomRotationSensitivityMedium;
			}
		}
		else if ( bIsTargeting )
		{
			switch ( WPRI.TargetSensitivityConfig )
			{
				case PCSO_Medium:	return TargettingRotationSensitivityMedium;
				case PCSO_Low:		return TargettingRotationSensitivityLow;
				case PCSO_High:		return TargettingRotationSensitivityHigh;
				default:			return TargettingRotationSensitivityMedium;
			}
		}
		else
		{
			switch ( WPRI.ControllerSensitivityConfig )
			{
				case PCSO_Medium:	return RotationSensitivityMedium;
				case PCSO_Low:		return RotationSensitivityLow;
				case PCSO_High:		return RotationSensitivityHigh;
				default:			return RotationSensitivityMedium;
			}
		}
	}
	else
	{
		if ( bIsZooming )
		{
			return ZoomRotationSensitivityMedium;
		}
		else if ( bIsTargeting )
		{
			return TargettingRotationSensitivityMedium;
		}
		else
		{
			return RotationSensitivityMedium;
		}
	}
}

/**
 * Starts/stops the loading movie
 *
 * @param bShowMovie true to show the movie, false to stop it
 * @param bPauseAfterHide (optional) If TRUE, this will pause the game/delay movie stop to let textures stream in
 * @param PauseDuration (optional) allows overriding the default pause duration specified in .ini (only used if bPauseAfterHide is true)
 * @param KeepPlayingDuration (optional) keeps playing the movie for a specified more seconds after it's supposed to stop
 */
native static final function ShowLoadingMovie(bool bShowMovie, optional bool bPauseAfterHide, optional float PauseDuration, optional float KeepPlayingDuration);

/**
 * Keep playing the loading movie if it's currently playing and abort any StopMovie calls that may be pending through the FDelayedUnpauser.
 */
native static final function KeepPlayingLoadingMovie();

/**
 * Replicated call to tell client to keep playing the loading movie if it's currently playing,
 * and abort any StopMovie calls that may be pending through the FDelayedUnpauser.
 */
reliable client event ClientKeepPlayingLoadingMovie()
{
	if ( IsPrimaryPlayer() )
	{
		KeepPlayingLoadingMovie();
	}
}

/**
 * replicated call to tell client to start/stop the loading movie (for loading checkpoints, etc)
 * @param bShowMovie true to show the movie, false to stop it
 * @param bPauseAfterHide (optional) If TRUE, this will pause the game/delay movie stop to let textures stream in
 * @param PauseDuration (optional) allows overriding the default pause duration specified in .ini (only used if bPauseAfterHide is true)
 * @param KeepPlayingDuration (optional) keeps playing the movie for a specified more seconds after it's supposed to stop
 * @param OverridePreviousDelays (optional) whether to cancel previous delayed StopMovie or not (defaults to FALSE)
 */
reliable client event ClientShowLoadingMovie(bool bShowMovie, optional bool bPauseAfterHide, optional float PauseDuration, optional float KeepPlayingDuration, optional bool OverridePreviousDelays)
{
	if ( IsPrimaryPlayer() )
	{
		if ( OverridePreviousDelays )
		{
			KeepPlayingLoadingMovie();
		}
		ShowLoadingMovie(bShowMovie, bPauseAfterHide, PauseDuration, KeepPlayingDuration);
	}
}

/** notification from the client to the server that it has loaded the checkpoint locally */
reliable server event ServerNotifyClientLoadedCheckpoint()
{
	local Actor A;

	if (bClientLoadingCheckpoint)
	{
		bClientLoadingCheckpoint = false;

		if (LocalPlayer(Player) == None)
		{
			// force update all infrequently updated relevant actors
			foreach AllActors(class'Actor', A)
			{
				if (A.RemoteRole != ROLE_None)
				{
					if (A.NetUpdateFrequency < 1.0)
					{
						A.bForceNetUpdate = true;
					}
					// force a full update for matinees
					else if (A.IsA('MatineeActor'))
					{
						A.bNetDirty = true;
					}
				}
			}
		}
	}
}

/** == CHECKPOINT INTERFACE == */

function DelayedSaveCheckpoint()
{
	// don't allow user controlled saving during cinematics so we don't have to try to
	// recreate the state of the whole thing
	if (!bCinematicMode)
	{
		SaveCheckpoint();
	}
}

function OnCheckpoint(SeqAct_Checkpoint Action)
{
	local Controller C;
	local GearPC PC, TeleportToPC;
	local GearAI AI;
	local name SquadName;

	if (Action.InputLinks[0].bHasImpulse)
	{
		`Log("Save Checkpoint triggered from Kismet action" @ PathName(Action));

		SaveCheckpoint();

		// make sure squadmates are nearby; if not, teleport them
		TeleportToPC = GearPC(Action.GetController(Action.TeleportTargetPlayer));
		if (TeleportToPC == None || TeleportToPC.Pawn == None)
		{
			TeleportToPC = self;
		}
		if (TeleportToPC.Pawn != None)
		{
			foreach WorldInfo.AllControllers(class'Controller', C)
			{
				PC = GearPC(C);
				if (PC != None)
				{
					SquadName = (PC.Squad != None) ? PC.Squad.SquadName : '';
				}
				else
				{
					AI = GearAI(C);
					SquadName = (AI != None) ? AI.GetSquadName() : '';
				}
				if ( C != TeleportToPC && C.Pawn != None &&
					(Squad == None || GearPC(C) != None || Action.TeleportClasses.length > 0 || SquadName == Squad.SquadName) &&
					WorldInfo.GRI.OnSameTeam(C, self) &&
					Action.ShouldTeleport(C.Pawn, TeleportToPC.Pawn.Location) &&
					(Action.TeleportClasses.length == 0 || Action.TeleportClasses.Find(C.Pawn.Class) != INDEX_NONE) )
				{
					`Log("Attempting to teleport" @ C.Pawn @ "to" @ self @ "[" $ Pawn $ "]");
					if (!TeleportToPC.FindAvailableTeleportSpot(C.Pawn))
					{
						`Warn("Checkpoint failed to teleport" @ C.Pawn @ "to" @ TeleportToPC.Pawn);
					}
				}
			}
		}
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		LoadCheckpoint();
	}
}

/**
 * Primary entry method for saving a checkpoint for this player.
 */
exec function SaveCheckpoint()
{
	local GearEngine Engine;
	local GearPC PC;

	// don't allow checkpointing when dead/DBNO
	// don't allow multiple saves in a row to avoid wasted startup time, particularly in chapter starts
	// where we autosave but the LDs have sometimes also added a Kismet save shortly after startup
	if ( IsPrimaryPlayer() && Pawn != None && Pawn.Health > 0 && Role == ROLE_Authority && WorldInfo.TimeSeconds - LastCheckpointSaveTime > 0.5)
	{
		// verify that we haven't actually lost the game and are just in the delay before the screen comes up
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			if (PC.IsTimerActive(nameof(PC.ProcessGameOver)) || (PC.Pawn != None && PC.Pawn.Health <= 0))
			{
				return;
			}
		}

		Engine = GearEngine(Player.Outer);
		`log(`location @ `showvar(WorldInfo.TimeSeconds) @ `showvar(Engine) @ `showvar(Engine.GetCurrentDeviceID(),CurrentDeviceId)
			@ `showvar(Engine.IsCurrentDeviceValid(),ValidDeviceSelected)
			@ `showvar(Engine.IsCurrentDeviceValid(Engine.const.MAX_DATASIZE_FOR_ALL_CHECKPOINTS),DeviceHasFreeSpace)
			@ `showvar(Engine.HasStorageDeviceBeenRemoved(),DeviceBeenRemovedSinceLastSave),,'Gear_CheckpointSystem');

		if ( Engine != None )
		{
			// if we have a valid storage device with enough space, proceed
			if ( Engine.IsCurrentDeviceValid(Engine.const.MAX_DATASIZE_FOR_ALL_CHECKPOINTS) )
			{
				Engine.bShouldWriteCheckpointToDisk = true;
				SaveCheckpointWorker();
			}

			// we didn't have enough room on the current storage device; see if we have a valid storage device at all - if not, display
			// a prompt providing the user with the option to choose one.
			else if ( Engine.IsCurrentDeviceValid() )
			{
				DisplayInsufficientStorageSpacePrompt();
			}

			// we don't have a valid storage device at all.  next we check if we used to have one but it was removed.  If so, display
			// a prompt providing the user with the option to choose a storage device.
			else if ( Engine.HasStorageDeviceBeenRemoved() )
			{
				DisplayInvalidStorageDevicePrompt();
			}

			// don't have a storage device, and never had one - no need to do anything special
			else
			{
				SaveCheckpointWorker();
			}
		}
	}
}

/**
 * Performs the steps necessary to actually initiate the checkpoint save.  This method is called after all required conditions have been
 * checked and dealt with.
 */
private final function SaveCheckpointWorker()
{
	local GearEngine Engine;
	local GearPC PC;

	`assert(IsPrimaryPlayer());
	`assert(Pawn != None);
	`assert(Pawn.Health>0);
	`assert(Role == ROLE_Authority);

	Engine = GearEngine(Player.Outer);
	LastCheckpointSaveTime = WorldInfo.TimeSeconds;

	Engine.PendingCheckpointAction = Checkpoint_Save;
	Engine.PendingCheckpointLocation = MyGearPawn != None ? MyGearPawn.Location : Location;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		if ( LocalPlayer(PC.Player) == None )
		{
			PC.ClientNotifySavingCheckpoint();
		}
		else if ( PC.IsPrimaryPlayer() )
		{
			if ( Engine.AreStorageWritesAllowed() )
			{
				// Save the profile when we save checpoints
				SaveProfile();
				class'Checkpoint'.static.ShowSavingContentWarning();
			}
			else
			{
				PC.ClientNotifySavingCheckpoint();
			}
		}
		else
		{
			PC.ClientNotifySavingCheckpoint();
		}
	}
}

/**
 * Notifies the HUD that a checkpoint has occurred so that it can draw the "CHECKPOINT" text.
 */
reliable client function ClientNotifySavingCheckpoint()
{
	if ( MyGearHud != None && IsPrimaryPlayer() )
	{
		MyGearHud.TimeOfLastCheckpoint = WorldInfo.TimeSeconds;
	}

	// Save the profile when we save checpoints
	SaveProfile();
}

/**
 * Utility method for opening a message box prompting the user about a storage device problem.  Sets slightly different flags on the message
 * box scene.  All parameters are identical to GameUISceneClient.OpenUIMessageBox
 */
final function bool OpenStorageDevicePrompt( name SceneTag, string Title, string Message, string Question, array<name> ButtonAliases, delegate<UIMessageBoxBase.OnOptionSelected> SelectionCallback )
{
	local UIScene ExistingScene;
	local UIMessageBoxBase MessageBox;
	local GameUISceneClient GameSceneClient;
	local bool bResult;
	local LocalPlayer LP;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if ( GameSceneClient != None )
	{
		LP = LocalPlayer(Player);

		// see if there is already a message box with the specified tag
		ExistingScene = GameSceneClient.FindSceneByTag(SceneTag, LP);
		if ( ExistingScene == None )
		{
			ExistingScene = GameSceneClient.CreateUIMessageBox(SceneTag);
		}

		if ( ExistingScene != None )
		{
			ExistingScene.bPauseGameWhileActive = true;
			ExistingScene.bExemptFromAutoClose = true;
			ExistingScene.bSaveSceneValuesOnClose = false;
			ExistingScene.bFlushPlayerInput = true;
			ExistingScene.bCaptureMatchedInput = true;

			ExistingScene = ExistingScene.OpenScene(ExistingScene, LP/*, ForcedPriority*/);
			if ( ExistingScene != None )
			{
				MessageBox = UIMessageBoxBase(ExistingScene);
				MessageBox.SetupMessageBox(Title, Message, Question, ButtonAliases, SelectionCallback);
				bResult = true;
			}
		}
	}

	return bResult;
}

/**
 * Displays a message box notifying the user that the currently selected storage device doesn't contain enough space for the checkpoint
 * save.  Provides the user with two options: choose a storage device or continue without saving.
 */
final function DisplayInsufficientStorageSpacePrompt()
{
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('CancelContinueAnyway');		// continue without saving
	ButtonAliases.AddItem('AcceptSelectDevice');		// choose another device
	OpenStorageDevicePrompt(
		'InsufficientStorageSpace',
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDeviceFull_Title>",
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDeviceFull_Message>",
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDevice_Question>",
		ButtonAliases, OnStorageDeviceFull_OptionChosen
		);
}

/**
 * Displays a message box notifying the user that the currently selected storage device is no longer valid or has been removed.  Provides
 * the user with two options: choose a storage device or continue without saving.
 */
final function DisplayInvalidStorageDevicePrompt()
{
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('CancelContinueAnyway');		// continue without saving
	ButtonAliases.AddItem('AcceptSelectDevice');		// choose another device
	OpenStorageDevicePrompt(
		'StorageDeviceLost',
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDeviceUnavailable_Title>",
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDeviceUnavailable_Message>",
		"<Strings:GearGameUI.MessageBoxErrorStrings.StorageDevice_Question>",
		ButtonAliases, OnStorageDeviceLost_OptionChosen
		);
}

/**
 * Handler for the 'selected storage device is full' prompt's seletion chosen.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnStorageDeviceFull_OptionChosen( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearEngine Engine;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local LocalPlayer LP;
	local bool bResult;

	LP = LocalPlayer(Player);
	if ( SelectedInputAlias == 'AcceptSelectDevice' )
	{
		if ( OnlineSub != None )
		{
			// Register our callback
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			PlayerIntEx.AddDeviceSelectionDoneDelegate(LP.ControllerId, OnDeviceSelectionComplete);

			// Show the UI
			if ( !PlayerIntEx.ShowDeviceSelectionUI(LP.ControllerId, class'GearEngine'.const.MAX_DATASIZE_FOR_ALL_CHECKPOINTS, true, true) )
			{
				// Don't wait if there was an error
				`Log("Error occurred while trying to display device selection UI");
				PlayerIntEx.ClearDeviceSelectionDoneDelegate(LP.ControllerId, OnDeviceSelectionComplete);
			}
		}
		else
		{
			`warn("No OnlineSubsystem found!");
		}

		// allow the message box to close itself
		bResult = true;
	}
	else if ( SelectedInputAlias == 'CancelContinueAnyway' )
	{
		Engine = GearEngine(Player.Outer);

		// make sure that the user doesn't continue to get bothered by these prompts
		Engine.SetCurrentDeviceID(INDEX_NONE, true);
		Engine.bShouldWriteCheckpointToDisk = false;

		// user elected to continue without a storage device.  do the default behavior (so we alert any connencted players)
		SaveCheckpointWorker();

		bResult = true;
	}

	return bResult;
}

/**
 * Handler for the 'selected storage device unavailable' prompt's seletion chosen.
 *
 * @param	Sender				the message box that generated this call
 * @param	SelectedInputAlias	the alias of the button that the user selected.  Should match one of the aliases passed into
 *								this message box.
 * @param	PlayerIndex			the index of the player that selected the option.
 *
 * @return	TRUE to indicate that the message box should close itself.
 */
function bool OnStorageDeviceLost_OptionChosen( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local GearEngine Engine;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local LocalPlayer LP;
	local bool bResult;

	LP = LocalPlayer(Player);
	if ( SelectedInputAlias == 'AcceptSelectDevice' )
	{
		if ( OnlineSub != None )
		{
			// Register our callback
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			PlayerIntEx.AddDeviceSelectionDoneDelegate( LP.ControllerId, OnDeviceSelectionComplete );

			// Show the UI
			if ( !PlayerIntEx.ShowDeviceSelectionUI( LP.ControllerId, class'GearEngine'.const.MAX_DATASIZE_FOR_ALL_CHECKPOINTS, true ) )
			{
				// Don't wait if there was an error
				`Log("Error occurred while trying to display device selection UI");
				PlayerIntEx.ClearDeviceSelectionDoneDelegate( LP.ControllerId, OnDeviceSelectionComplete );
			}
		}
		else
		{
			`warn("No OnlineSubsystem found!");
		}

		// allow the message box to close itself
		bResult = true;
	}
	else if ( SelectedInputAlias == 'CancelContinueAnyway' )
	{
		Engine = GearEngine(Player.Outer);

		// make sure that the user doesn't continue to get bothered by these prompts
		Engine.SetCurrentDeviceID(INDEX_NONE, true);
		Engine.bShouldWriteCheckpointToDisk = false;

		// user elected to continue without a storage device.  do the default behavior (so we alert any connencted players)
		SaveCheckpointWorker();
		bResult = true;
	}

	return bResult;
}


/**
 * Reads the results of the user's device choice
 *
 * @param bWasSuccessful true if the action completed without error, false if there was an error
 */
function OnDeviceSelectionComplete( bool bWasSuccessful )
{
	local GearEngine Engine;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local LocalPlayer LP;
	local string UnusedDeviceName;
	local int DeviceID;

	LP = LocalPlayer(Player);
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;

		// Unregister our callback
		PlayerIntEx.ClearDeviceSelectionDoneDelegate(LP.ControllerId,OnDeviceSelectionComplete);

		// Don't read the information unless it was successful
		if ( bWasSuccessful )
		{
			// Read the per user results
			DeviceID = PlayerIntEx.GetDeviceSelectionResults(LP.ControllerId,UnusedDeviceName);
		}
	}

	// If they selected a device
	Engine = GearEngine(Player.Outer);
	if ( DeviceID != 0 )
	{
		Engine.SetCurrentDeviceID(DeviceID);
	}
	else
	{
		// otherwise, make it so that the user doesn't continue to get bothered by these prompts
		Engine.SetCurrentDeviceID(INDEX_NONE, true);

		//@note - we don't disable bShouldWriteCheckpointToDisk, as that indicates that the user does not want to save at all.  This way,
		// if they choose another storage device later on, they'll begin saving checkpoints again.
		//Engine.bShouldWriteCheckpointToDisk = false;
	}

	// now we can finally save the checkpoint
	SaveCheckpointWorker();
}


exec function LoadCheckpoint()
{
	local GearEngine Engine;

	if (Role == ROLE_Authority && WorldInfo.NextURL == "")
	{
		Engine = GearEngine(Player.Outer);
		`log(WorldInfo.TimeSeconds@GetFuncName()@"Engine:"@Engine);
		if (Engine != None)
		{
			// do the slow reload if we need to reclaim memory
			if (Engine.NeedMemoryHack() && WorldInfo.TimeSeconds > 1.0)
			{
				`Log("MEMORY HACK: Doing full server travel to load checkpoint");
				ClientShowLoadingMovie(true);
				WorldInfo.ServerTravel("geargame_p?loadcheckpoint=1?chapter=" $ int(GearGameSP_Base(WorldInfo.Game).CurrentChapter));
			}
			else
			{
				Engine.PendingCheckpointAction = Checkpoint_Load;
			}
		}
	}
}

exec function RestartChapter()
{
	local GearGameSP_Base Game;

	Game = GearGameSP_Base(WorldInfo.Game);
	if (Game != None)
	{
		// explicit call to show loading movie because this transition occurs via ServerTravel, which doesn't always call ClientTravel
		ClientShowLoadingMovie(true);
		WorldInfo.ServerTravel("geargame_p?loadcheckpoint=0?chapter=" $ int(Game.CurrentChapter));
	}
}

/** triggers the clientside part of loading checkpoints */
reliable client event ClientLoadCheckpoint(vector CheckpointLocation)
{
	local GearEngine Engine;

	Engine = GearEngine(Player.Outer);
	if (Engine != None)
	{
		Engine.PendingCheckpointAction = Checkpoint_Load;
		Engine.PendingCheckpointLocation = CheckpointLocation;
	}
}

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	local InventoryRecord InvRecord;
	local GearWeapon Weapon;
	local GearShield Shield;
	local int i;
	local Pawn BasePawn, InvHolder;

	//@hack: kick the player off the troika here as that does bad things to the streaming for some reason
	if (Turret_TroikaBase(Pawn) != None)
	{
		Turret_TroikaBase(Pawn).DriverLeave(true);
	}

	Record.SavedGuid = MyGuid;
	if (MyGearPawn != None)
	{
		// create a GUID for our Pawn that is relative to ours
		//@warning: must match ApplyCheckpointRecord()
		Record.PawnClassName = GearPawn_LocustBrumakBase(Pawn.Base) != None ? PathName(Pawn.Base.Class) : PathName(Pawn.Class);
		if (MyGearPawn.MutatedClass != None)
		{
			Record.MutatedClassName = PathName(MyGearPawn.MutatedClass);
		}
		Record.Location = Pawn.Location;
		Record.Rotation = Pawn.Rotation;
		if (Pawn.Base != None && Pawn.Base.bNoDelete && !Pawn.Base.bStatic && !Pawn.Base.bWorldGeometry)
		{
			Record.BasePathName = PathName(Pawn.Base);
		}
		// if we are in cover save the current cover slot
		if (IsInCoverState() && MyGearPawn.CurrentLink != None)
		{
			Record.SlotMarkerPathName = PathName(MyGearPawn.CurrentLink.Slots[MyGearPawn.CurrentSlotIdx].SlotMarker);
		}
		if (MyGearPawn.CarriedCrate != None)
		{
			Record.CratePathName = PathName(MyGearPawn.CarriedCrate);
		}
		if (MyGearPawn.Mesh != None && MyGearPawn.Mesh.SkeletalMesh != None && MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset != None)
		{
			for (i = 0; i < ArrayCount(MyGearPawn.ReplicatedFaceFXAnimSets); i++)
			{
				Record.MountedFaceFX[i] = PathName(MyGearPawn.ReplicatedFaceFXAnimSets[i]);
			}
		}
		for (i = 0; i < ArrayCount(MyGearPawn.KismetAnimSets); i++)
		{
			Record.KismetAnimSets[i] = PathName(MyGearPawn.KismetAnimSets[i]);
		}
	}
	else
	{
		if (Pawn != None)
		{
			//@warning: assumption that player controlled non-GearPawns are level placed
			if (GearWeaponPawn(Pawn) != None)
			{
				Record.PawnPathName = PathName(Pawn.GetVehicleBase());
			}
			else
			{
				Record.PawnPathName = PathName(Pawn);
			}
			Record.Location = Pawn.Location;
			Record.Rotation = Pawn.Rotation;
		}
		else
		{
			Record.Location = Location;
			Record.Rotation = Rotation;
		}
	}
	if (Pawn != None)
	{
		// create records for each inventory item
		InvHolder = (Vehicle(Pawn) != None) ? Vehicle(Pawn).Driver : Pawn;
		foreach InvHolder.InvManager.InventoryActors(class'GearWeapon', Weapon)
		{
			Weapon.CreateCheckpointRecord(InvRecord);
			Record.InventoryRecords.AddItem(InvRecord);
		}
		foreach InvHolder.InvManager.InventoryActors(class'GearShield', Shield)
		{
			Shield.CreateCheckpointRecord(InvRecord);
			Record.InventoryRecords.AddItem(InvRecord);
		}
		// record health
		BasePawn = (GearWeaponPawn(Pawn) != None) ? Pawn.GetVehicleBase() : Pawn;
		Record.PawnHealthPct = float(BasePawn.Health) / float(BasePawn.HealthMax);
	}
	Record.TeamIndex = GetTeamNum();
	Record.SquadFormationName = string(GearPRI(PlayerReplicationInfo).SquadName);
	if (ObjectiveMgr != None)
	{
		Record.Objectives = ObjectiveMgr.Objectives;
	}
	Record.Music = WorldInfo.CurrentMusicTrack;
	Record.Music.TheSoundCuePath = PathName(Record.Music.TheSoundCue);
	Record.Music.TheSoundCue = None;
	Record.StoredKismetVariables = StoredKismetVariables;
	Record.GUDSFrequencyMultiplier = GearGame(WorldInfo.Game).UnscriptedDialogueManager.GlobalChanceToPlayMultiplier;
}

/** this is used to delay entering cover after loading a checkpoint because it seems GearPawn's animations can't handle
 * being put into cover on the same tick it was spawned (various anim nodes get into a completely hosed state)
 */
function CheckpointCoverHack()
{
	local CovPosInfo RecordedCover;

	if (LocalPlayer(Player) != None)
	{
		if ( ServerDictatedCover.OwningSlot.Link != None && ServerDictatedCover.OwningSlot.Link.IsEnabled() &&
			ServerDictatedCover.OwningSlot.Link.Slots[ServerDictatedCover.OwningSlot.SlotIdx].bEnabled )
		{
			MyGearPawn.SetCoverInfoFromSlotInfo(RecordedCover, ServerDictatedCover.OwningSlot);
			AcquireCover(RecordedCover);
		}
		else
		{
			ServerDictatedCover = None;
		}
	}
}

/** called to recreate the player's camera */
final reliable client function ClientResetCamera()
{
	if (PlayerCamera != None)
	{
		PlayerCamera.Destroy();
		SpawnPlayerCamera();
	}

	RelativeToVehicleViewRot = rot(0,0,0);
	LastVehicleSpaceViewRotation = rot(0,0,0);
}

/** resets the object pool to its initial state */
reliable client final function ClientResetObjectPool()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None && GRI.GOP != None)
	{
		GRI.GOP.CleanupForPToPTransition();
	}
}


/** This will flush all of the various pools in the GearObjectPool */
reliable client final function ClientObjectPoolForceFlush()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None && GRI.GOP != None)
	{
		GRI.GOP.CleanUpPools();
	}
}


/** This will flush all of the various pools in the GearObjectPool */
reliable client final function ClientObjectPoolCreatePools()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None && GRI.GOP != None)
	{
		GRI.GOP.CreatePools();
	}
}


/** resets various state after loading a checkpoint */
reliable client final function ClientCheckpointReset()
{
	local FracturedStaticMeshActor FracActor;
	local GearPawn P;

	// clear cinematic flags
	bCinematicMode = false;
	bHavingAnAbortableConversation = false;
	bInvisible = FALSE;
	bInMatinee = false;
	if (myHUD != None)
	{
		myHUD.bShowHUD = true;
	}
	// clear HUD info
	StopPostProcessOverride(eGPP_BleedOut);
	if ( MyGearHud != None )
	{
		MyGearHud.ClearActiveActionInfo();
	}
	// clear trigger flags
	if (DoorTriggerDatum.bInsideDoorTrigger)
	{
		OnExitDoorTrigger();
	}
	// reset camera
	ClientResetCamera();
	// close any menus that were up
	ClientCloseAllMenus();
	// restore subtitles (game over screen turns them off)
	RestoreShowSubtitles();
	// Reset the tutorials
	if (TutorialMgr != none)
	{
		TutorialMgr.RebootSystem();
	}
	// reset object pools and fracturables
	ClientResetObjectPool();
	foreach DynamicActors(class'FracturedStaticMeshActor', FracActor)
	{
		FracActor.ResetVisibility();
	}
	if (WorldInfo.MyFractureManager != None)
	{
		WorldInfo.MyFractureManager.ResetPoolVisibility();
	}

	// destroy any old, dead, torn off Pawns
	foreach WorldInfo.AllPawns(class'GearPawn', P)
	{
		if (P.bTearOff && P.Health <= 0)
		{
			P.Destroy();
		}
	}

	// if we got a Pawn, make sure it got a weapon
	if (Pawn != None && Pawn.Weapon == None)
	{
		ClientSwitchToBestWeapon();
	}

	// and stop the loading movie after a delay to let textures stream in
	ShowLoadingMovie(false, true);

	// slight delay to make sure we finish up any lingering streaming tasks
	SetTimer(0.5, false, nameof(ServerCheckpointResetComplete));

	// let the UI know that we're no longer in cinematic mode so it should update cached viewport sizes for any open scenes
	UpdateUIViewportSizes();
}

/** called after the client has done all its post-checkpoint tasks and is ready to go */
reliable server function ServerCheckpointResetComplete()
{
	local Actor A;
	local Weapon W;

	if (!IsLocalPlayerController())
	{
		// force networking update
		foreach AllActors(class'Actor', A)
		{
			if (A.NetUpdateFrequency < 1.0 && !A.bOnlyRelevantToOwner)
			{
				A.SetNetUpdateTime(FMin(A.NetUpdateTime, WorldInfo.TimeSeconds + 0.2 * FRand()));
			}
		}

		//@HACK: reselect weapon if it's not in the persistent level, since the replication might have failed
		//	due to client not having the level loaded yet
		//	need to make selecting initial weapon not rely on RPCs...
		if (Vehicle(Pawn) != None)
		{
			if (Pawn.Weapon != None)
			{
				Pawn.Weapon.ClientWeaponSet(false);
			}
			else
			{
				foreach Pawn.InvManager.InventoryActors(class'Weapon', W)
				{
					W.ClientWeaponSet(false);
					break;
				}
			}
		}
	}
}

/** converter from CheckpointMusicRecord to MusicTrackStruct as the script compiler can't do it automatically */
native final function CopyCheckpointMusicInfo(const out CheckpointMusicRecord SourceInfo, out MusicTrackStruct DestInfo);

/** applies inventory records to our Pawn */
private function ApplyInventoryRecords(const out array<InventoryRecord> Records)
{
	local class<Inventory> InvClass;
	local InventoryRecord InvRecord;
	local Inventory InvItem;
	local bool bFoundActiveWeapon;
	local GearWeapon W;

	// discard any current inventory
	Pawn.InvManager.DiscardInventory();
	// create any recorded inventory items
	foreach Records(InvRecord)
	{
		InvClass = class<Inventory>(FindObject(InvRecord.InventoryClassPath, class'Class'));
		if (InvClass == None)
		{
			`Warn("Failed to find class" @ InvRecord.InventoryClassPath @ "in memory");
		}
		else
		{
			InvItem = Pawn.InvManager.Spawn(InvClass);
			if (InvItem == None)
			{
				`Warn("Couldn't spawn inventory of class" @ InvClass);
			}
			else
			{
				if (GearWeapon(InvItem) != None)
				{
					GearWeapon(InvItem).ApplyCheckpointRecord(InvRecord);
				}
				Pawn.InvManager.AddInventory(InvItem, !InvRecord.bIsActiveWeapon);
				bFoundActiveWeapon = bFoundActiveWeapon || InvRecord.bIsActiveWeapon;
			}
		}
	}
	// make sure a valid weapon was selected
	if (!bFoundActiveWeapon)
	{
		foreach Pawn.InvManager.InventoryActors(class'GearWeapon', W)
		{
			if (W.HasAnyAmmo())
			{
				W.ClientWeaponSet(true);
			}
			break;
		}
	}
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	local class<GearPawn_LocustBrumakBase> BrumakClass;
	local Pawn FoundPawn;
	local Vehicle FoundVehicle;
	local class<Pawn> PawnClass;
	local GearPawn_LocustBrumakBase Brumak;
	local GearPawn_CarryCrate_Base Crate;
	local MusicTrackStruct NewMusic;
	local int i;
	local FaceFXAnimSet FaceFXAnim;
	local Actor OldBase;

	MyGuid = Record.SavedGuid;
	// if we were in cover then bail now
	if (IsInCoverState())
	{
		LeaveCover();
	}
	// eliminate our existing pawn
	if (MyGearPawn != None)
	{
		MyGearPawn.LifeSpan = 0.1;
	}
	else if (Vehicle(Pawn) != None)
	{
		Vehicle(Pawn).DriverLeft();
	}
	UnPossess();
	if (Record.PawnClassName != "")
	{
		PawnClass = class<Pawn>(FindObject(Record.PawnClassName, class'Class'));
	}
	if (PawnClass != None)
	{
		// set the pawn class and restart this player to create the pawn
		DefaultPawnClass = PawnClass;
		// special case for Brumak
		BrumakClass = class<GearPawn_LocustBrumakBase>(DefaultPawnClass);
		if (BrumakClass != None)
		{
			WorldInfo.Game.RestartPlayer(self);
			foreach DynamicActors(class'GearPawn_LocustBrumakBase', Brumak)
			{
				if (Brumak.Class == BrumakClass)
				{
					break;
				}
			}
			if (Brumak == None)
			{
				Brumak = Spawn(BrumakClass,,, Record.Location, Record.Rotation,, true);
			}
			DriveBrumak(Brumak, Brumak.Driver == None || !Brumak.IsHumanControlled());
			if (Pawn == Brumak)
			{
				Pawn.SetLocation(Record.Location);
			}
			// snap the camera to the pawn rotation, otherwise it will retain the previous direction at the time of load
			SetRotation(Record.Rotation);
			ClientSetRotation(Record.Rotation);
			WorldInfo.Game.ChangeTeam(self, Record.TeamIndex, false);
			SetSquadName(name(Record.SquadFormationName));
		}
		// general case
		else if (DefaultPawnClass != None)
		{
			WorldInfo.Game.RestartPlayer(self);
			if (Pawn != None)
			{
				ApplyInventoryRecords(Record.InventoryRecords);
				// move the pawn to the correct location
				Pawn.SetLocation(Record.Location);
				Pawn.SetRotation(Record.Rotation);

				// ensure we don't have a stale anchor lying around from before we teleported
				if(!Pawn.ValidAnchor())
				{
					Pawn.SetAnchor(none);
				}

				if (Record.BasePathName != "")
				{
					Pawn.SetBase(Actor(FindObject(Record.BasePathName, class'Actor')));
				}
				// snap the camera to the pawn rotation, otherwise it will retain the previous direction at the time of load
				SetRotation(Record.Rotation);
				ClientSetRotation(Record.Rotation);
				WorldInfo.Game.ChangeTeam(self, Record.TeamIndex, false);
				SetSquadName(name(Record.SquadFormationName));
				if (Record.MutatedClassName != "")
				{
					class<GearPawn>(FindObject(Record.MutatedClassName, class'Class')).static.MutatePawn(MyGearPawn);
				}
				if (Record.SlotMarkerPathName != "")
				{
					ServerDictatedCover = CoverSlotMarker(FindObject(Record.SlotMarkerPathName, class'CoverSlotMarker'));
					if ( ServerDictatedCover.OwningSlot.Link != None && ServerDictatedCover.OwningSlot.Link.IsEnabled() &&
						ServerDictatedCover.OwningSlot.Link.Slots[ServerDictatedCover.OwningSlot.SlotIdx].bEnabled )
					{
						SetTimer( 0.5, false, nameof(CheckpointCoverHack) );
					}
					else
					{
						ServerDictatedCover = None;
					}
				}
				else
				{
					GotoState('PlayerWalking');
				}
				if (MyGearPawn != None)
				{
					if (Record.CratePathName != "")
					{
						Crate = GearPawn_CarryCrate_Base(FindObject(Record.CratePathName, class'GearPawn_CarryCrate_Base'));
						if (Crate != None)
						{
							if (MyGearPawn.IsA('GearPawn_COGMarcus'))
							{
								Crate.MarcusPawn = MyGearPawn;
							}
							else
							{
								Crate.DomPawn = MyGearPawn;
							}
							Crate.SetPawnToCarryMode(MyGearPawn);
						}
					}
					if (MyGearPawn.Mesh != None && MyGearPawn.Mesh.SkeletalMesh != None && MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset != None)
					{
						for (i = 0; i < ArrayCount(Record.MountedFaceFX); i++)
						{
							FaceFXAnim = FaceFXAnimSet(FindObject(Record.MountedFaceFX[i], class'FaceFXAnimSet'));
							if (FaceFXAnim != None)
							{
								MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(FaceFXAnim);
								MyGearPawn.ReplicatedFaceFXAnimSets[i] = FaceFXAnim;
							}
						}
					}
					for (i = 0; i < ArrayCount(MyGearPawn.KismetAnimSets); i++)
					{
						MyGearPawn.KismetAnimSets[i] = AnimSet(FindObject(Record.KismetAnimSets[i], class'AnimSet'));
					}
					MyGearPawn.UpdateAnimSetList();
				}
			}
		}
	}
	else if (Record.PawnPathName != "")
	{
		// find the pawn in the level
		FoundPawn = Pawn(FindObject(Record.PawnPathName, class'Pawn'));
		if (FoundPawn != None)
		{
			FoundPawn.SetHidden(false);
			FoundVehicle = Vehicle(FoundPawn);
			if (FoundVehicle != None)
			{
				if (FoundVehicle != Pawn)
				{
					WorldInfo.Game.RestartPlayer(self);
					ApplyInventoryRecords(Record.InventoryRecords);
					if (!FoundVehicle.TryToDrive(Pawn))
					{
						`Warn("Failed to enter vehicle" @ FoundVehicle @ "with driver" @ Pawn);
					}
				}
				if (Pawn == FoundVehicle)
				{
					OldBase = FoundPawn.Base;
					FoundPawn.SetLocation(Record.Location);
					//@HACK: ignore record rotation for the ride reaver, breaks the matinee for some reason
					if (Vehicle_RideReaver_Base(Pawn) == None)
					{
						FoundPawn.SetRotation(Record.Rotation);
					}
					if (SVehicle(FoundVehicle) != None)
					{
						FoundVehicle.Mesh.SetRBPosition(Record.Location);
						FoundVehicle.Mesh.SetRBRotation(Record.Rotation);
					}
					FoundPawn.SetBase(OldBase);
				}
			}
			else
			{
				FoundPawn.SetLocation(Record.Location);
				FoundPawn.SetRotation(Record.Rotation);
				if (FoundPawn != Pawn)
				{
					Possess(FoundPawn, false);
				}
			}
			// snap the camera to the pawn rotation, otherwise it will retain the previous direction at the time of load
			//@HACK: not for the ride reaver, causes the whole thing to rotate for some reason
			if (Vehicle_RideReaver_Base(Pawn) == None)
			{
				SetRotation(Record.Rotation);
				ClientSetRotation(Record.Rotation);
			}
			WorldInfo.Game.ChangeTeam(self, Record.TeamIndex, false);
			SetSquadName(name(Record.SquadFormationName));
		}
	}
	if (Pawn != None)
	{
		Pawn.Health = Record.PawnHealthPct * Pawn.HealthMax;
		if (GearVehicle(Pawn) != None)
		{
			GearVehicle(Pawn).ReceivedHealthChange(Pawn.default.Health);
		}
		if (Pawn.Health < Pawn.HealthMax && GearPawn(Pawn) != None)
		{
			GearPawn(Pawn).ResetHealthRecharge();
		}
	}
	if (ObjectiveMgr != None && LocalPlayer(Player) != None)
	{
		ObjectiveMgr.Objectives = Record.Objectives;
		// reset UpdateTime for objectives so that old ones never display and currently active ones start out onscreen
		for (i = 0; i < ObjectiveMgr.Objectives.length; i++)
		{
			ObjectiveMgr.Objectives[i].UpdatedTime = (ObjectiveMgr.Objectives[i].bCompleted) ? -100.0 : WorldInfo.TimeSeconds;
		}
	}
	if (LocalPlayer(Player) != None)
	{
		CopyCheckpointMusicInfo(Record.Music, NewMusic);
		NewMusic.TheSoundCue = SoundCue(FindObject(Record.Music.TheSoundCuePath, class'SoundCue'));
		WorldInfo.UpdateMusicTrack(NewMusic);

		StoredKismetVariables = Record.StoredKismetVariables;

		GearGame(WorldInfo.Game).UnscriptedDialogueManager.GlobalChanceToPlayMultiplier = Record.GUDSFrequencyMultiplier;
	}

	// if we were spawned just to parse the data, now destroy and let the AI take over
	if (bCheckpointDummy)
	{
		`Log("Destroying checkpoint dummy GearPC");
		Destroy();
	}
}

//@HACK: fix up mesh animtree and translation after a map transition because the vehicle exit code doesn't seem to
//	trigger/work correctly in some cases for some reason
reliable client function ClientMeshFixupHack(GearPawn P)
{
	if (P != None && P.Mesh != None)
	{
		if (P.Mesh.AnimTreeTemplate != P.default.Mesh.AnimTreeTemplate)
		{
			P.Mesh.SetAnimTreeTemplate(P.default.Mesh.AnimTreeTemplate);
			P.ClearAnimNodes();
			P.CacheAnimNodes();
		}
		P.Mesh.SetTranslation(P.default.Mesh.Translation);
	}

	//@HACK: workaround for weapon sometimes not showing up for client after transition
	if ( P != None && WorldInfo.NetMode == NM_Client && !P.IsLocallyControlled() &&
			P.InvManager != None && P.InvManager.Role == ROLE_Authority )
	{
		P.InvManager.Destroy();
		P.Weapon = None;
		P.MyGearWeapon = None;
		P.RefreshRemoteAttachments();
	}
}

final reliable client function ClientUnlockChapter(EChapterPoint UnlockedChapter, EChapterPoint CompletedChapter, EDifficultyLevel Difficulty)
{
	ProfileSettings.UnlockChapterByDifficulty(UnlockedChapter, Difficulty, self);
}

final reliable client function ClientDisplayChapterTitle(EChapterPoint DisplayChapter, float TotalDisplayTime, float TotalFadeTime)
{
	if (MyGearHud != None)
	{
		MyGearHud.StartDrawingDramaticText( class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(DisplayChapter).DisplayName,
						class'GearUIDataStore_GameResource'.static.GetChapterDataProvider(DisplayChapter).DisplayName,
						TotalDisplayTime, TotalFadeTime, TotalFadeTime,
						-1, 2.0f, 2.0f );
	}
	if (PlayerReplicationInfo != None)
	{
		// Update rich presence with the same information
		ClientSetCoopRichPresence(GearPRI(PlayerReplicationInfo).PawnClass,DisplayChapter);
	}
}

/** Returns the number of pixels to move the dramatic text down for MP */
function int NumPixelsToMoveDownForDramaText()
{
	if (IsSplitscreenPlayer())
	{
		return 10;
	}
	else
	{
		return 100;
	}
}

/**
 * Replicated function to use when setting the rich presence string for coop
 *
 * @param PawnClass the pawn this player is playing as
 * @param Chapter the chapter point to convert to act/chapter information
 */
reliable client function ClientSetCoopRichPresence(class<Pawn> PawnClass,EChapterPoint Chapter);

reliable client final event ClientSetConversationMode(bool bEnabled, optional bool bAbortable)
{
	bHavingAnAbortableConversation = bEnabled && bAbortable;

	// clear abort flag
	bPendingConversationAbort = FALSE;
}

reliable server final function ServerAbortConversation()
{
	// set the flag for pending abort.  The latent kismet action will detect this and
	// properly abort everyone affected.
	bPendingConversationAbort = TRUE;
}


simulated function MortarPitchAdjDone()
{
	EnableInput(FALSE, FALSE, TRUE);
}

simulated final protected function rotator ClampYaw(rotator Rot, int BaseYaw, int MaxDelta)
{
	local int	DeltaFromCenter, YawAdj;
	local rotator OutRot;

	DeltaFromCenter = NormalizeRotAxis(Rot.Yaw - BaseYaw);

	if( DeltaFromCenter > MaxDelta )
	{
		YawAdj	= MaxDelta - DeltaFromCenter;
	}
	else if( DeltaFromCenter < -MaxDelta )
	{
		YawAdj	= -(DeltaFromCenter + MaxDelta);
	}

	OutRot = Rot;
	OutRot.Yaw += YawAdj;
	return OutRot;
};

event Rotator LimitViewRotation( Rotator ViewRotation, float ViewPitchMin, float ViewPitchMax )
{
	local Turret_TroikaBase GT;
	local Rotator Rot;
	local vector TurrX, TurrY, TurrZ;

	// Give a chance to Special Moves to limit view Rotation.
	if( MyGearPawn != None && MyGearPawn.SpecialMove != SM_None )
	{
		MyGearPawn.SpecialMoves[MyGearPawn.SpecialMove].LimitViewRotation(ViewRotation);
	}

	// handle non-level Turrets (e.g turrets on derricks in Gears 2)
	if (MyGearPawn == None)
	{
		GT = Turret_TroikaBase(Pawn);
		if (GT != None)		// && (GT->Base != None) && (GT->Base )
		{
			Rot = QuatToRotator(GT.Mesh.GetBoneQuaternion(GT.Pivot_Latitude_BoneName));
			GetAxes(Rot, TurrX, TurrY, TurrZ);
			Rot = Rotator(TurrY);		// compensates for base bone orientation
			ViewPitchMin += Rot.Pitch;
			ViewPitchMax += Rot.Pitch;
		}
	}

	return Super.LimitViewRotation(ViewRotation, ViewPitchMin, ViewPitchMax);
}

//debug
exec function CoverSucks()
{
	local GearAI AI;
	foreach WorldInfo.AllControllers( class'GearAI', AI )
	{
		AI.SetAcquireCover( ACT_Immediate );
	}
}

//stub
unreliable server function ServerPlayBrumakRoar();

state PlayerBrumakDriver extends PlayerWalking
{
	exec function PrevWeapon();
	exec function NextWeapon();
	function bool CanPickupWeapons();
	function TransitionFromRoadieRunToCover( CovPosInfo FoundCovInfo, optional bool bNoCameraAutoAlign );
	function bool CanRunToCover(out CovPosInfo CoverCanRunTo, out int NoCameraAutoAlign, optional bool bSkipSpecialMoveCheck, optional float CheckScale, optional EDoubleClickDir DblClickDirToCheck=DCLICK_None, optional bool bPreventSideEntry);

	// Brumak is never walking
	function HandleWalking()
	{
		MyGearPawn.SetWalking( FALSE );
	}

	function AcquireCover(CovPosInfo CovInfo, optional bool bNoCameraAutoAlign)
	{
		`log( self@GetFuncName()@MyGearPawn@"NO GETTING INTO COVER ON BRUMAK" );
		ScriptTrace();
	}

	function BeginState(Name PreviousStateName)
	{
		super.BeginState( PreviousStateName );

		MyGearPawn.MyGearWeapon = GearWeapon(MyGearPawn.Weapon);
	}

	function UpdateRotation( float DeltaTime )
	{
		local Rotator	DeltaRot, NewRotation, ViewRotation;
		local GearPawn_LocustBrumakBase Brumak;

		Brumak = GearPawn_LocustBrumakBase(Pawn);
		if( Brumak == None )
			return;

		ViewRotation	= Rotation;
		DesiredRotation = ViewRotation; //save old rotation

		// Calculate Delta to be applied on ViewRotation
		DeltaRot.Yaw	= PlayerInput.aTurn;
		DeltaRot.Pitch	= PlayerInput.aLookUp;

		DeltaRot += ExtraRot;
		ExtraRot = rot(0,0,0);

		// Check that Adhesion target is still valid
		if( ForcedAdhesionTarget != None && !ForcedAdhesionTarget.IsValidEnemyTargetFor(PlayerReplicationInfo,FALSE) )
		{
			StopForcedAdhesion();
		}

		// Apply adhesion
		ApplyAdhesion(DeltaTime, DeltaRot.Yaw, DeltaRot.Pitch);

		ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
		GearPlayerCamera(PlayerCamera).GameplayCam.AdjustFocusPointInterpolation(DeltaRot);			// adjust for focus points

		SetRotation( ViewRotation );

		ViewShake( DeltaTime );

		NewRotation = ViewRotation;
		NewRotation.Roll = 0;
		if( Pawn != None )
		{
			Pawn.FaceRotation( NewRotation, DeltaTime );
		}
	}

	function ProcessViewRotation( float DeltaTime, out Rotator out_ViewRotation, Rotator DeltaRot )
	{
		local GearPawn_LocustBrumakBase Brumak;

		super.ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );

		Brumak = GearPawn_LocustBrumakBase(Pawn);
		if( Brumak.IsDoingSpecialMove(GSM_Brumak_CannonFire) )
		{
			ClampRotation( out_ViewRotation, Brumak.Rotation, rot(14563,8190,-1), rot(14563,8190,-1) );
		}
	}

	function Rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
	{
		local Rotator		CamRot;
		local Vector		HitLocation, HitNormal, EndTrace, StartTrace, CamLoc;
		local Actor			HitActor;
		local ImpactInfo	Impact;

		GetPlayerViewPoint( CamLoc, CamRot );
		StartTrace	= CamLoc;
		EndTrace	= CamLoc + vector(CamRot)*10000;
		CalcWeaponFire( StartTrace, EndTrace, HitLocation, HitNormal, HitActor, Impact );

		return rotator(HitLocation-StartFireLoc);
	}

	unreliable server function ServerPlayBrumakRoar()
	{
		local GearPawn_LocustBrumakBase Brumak;
		Brumak = GearPawn_LocustBrumakBase(Pawn);
		Brumak.UpdateRoarCount();
	}
}

unreliable server function ServerVerifyBrumak();
unreliable client function ClientSetBrumak(GearPawn_LocustBrumakBase Brumak);

state PlayerBrumakGunner
{
	exec function PrevWeapon();
	exec function NextWeapon();
	function bool CanPickupWeapons();

	function BeginState(Name PreviousStateName)
	{
		// Stop AI from targeting or damaging the gunner... brumak is the target
		bGodMode = TRUE;
		MyGearPawn.bIgnoreBaseRotation	= TRUE;
//		MyGearPawn.bNeverAValidEnemy	= TRUE;

		MyGearPawn.MyGearWeapon = GearWeapon(MyGearPawn.Weapon);
		SetViewTarget( Pawn.Base );
	}

	function EndState( Name NextStateName )
	{
		bGodMode = FALSE;
		MyGearPawn.bIgnoreBaseRotation	= FALSE;
//		MyGearPawn.bNeverAValidEnemy	= FALSE;
	}

	function PlayerMove( float DeltaTime )
	{
		// Update rotation.
		UpdateRotation( DeltaTime );

		if (Role < ROLE_Authority)
		{
			ServerUpdateRotation(((Rotation.Yaw & 65535) << 16) + (Rotation.Pitch & 65535));
		}
	}

	//@HACK: workaround for issue where client's losing Pawn.Base and exploding
	unreliable server function ServerVerifyBrumak()
	{
		if (Pawn != None && GearPawn_LocustBrumakBase(Pawn.Base) != None)
		{
			ClientSetBrumak(GearPawn_LocustBrumakBase(Pawn.Base));
		}
	}
	unreliable client function ClientSetBrumak(GearPawn_LocustBrumakBase Brumak)
	{
		if (Pawn != None && Brumak != None)
		{
			Pawn.SetBase(Brumak);
		}
	}

	function UpdateRotation( float DeltaTime )
	{
		local Rotator	DeltaRot, ViewRotation;
		local GearPawn_LocustBrumakBase Brumak;

		Brumak = GearPawn_LocustBrumakBase(Pawn.Base);
		if( Brumak == None )
		{
			if (Role < ROLE_Authority)
			{
				ServerVerifyBrumak();
			}
			return;
		}

		ViewRotation	= Rotation;
		DesiredRotation = ViewRotation; //save old rotation

		// Calculate Delta to be applied on ViewRotation
		DeltaRot.Yaw	= PlayerInput.aTurn;
		DeltaRot.Pitch	= PlayerInput.aLookUp;

		// Check that Adhesion target is still valid
		if( ForcedAdhesionTarget != None && !ForcedAdhesionTarget.IsValidEnemyTargetFor(PlayerReplicationInfo,FALSE) )
		{
			StopForcedAdhesion();
		}

		// Apply adhesion
		ApplyAdhesion(DeltaTime, DeltaRot.Yaw, DeltaRot.Pitch);

		ProcessViewRotation( DeltaTime, ViewRotation, DeltaRot );
		SetRotation( ViewRotation );

		ViewShake( DeltaTime );
	}

	function ProcessViewRotation( float DeltaTime, out Rotator out_ViewRotation, Rotator DeltaRot )
	{
		local GearPawn_LocustBrumakBase Brumak;

		super.ProcessViewRotation( DeltaTime, out_ViewRotation, DeltaRot );

		Brumak = GearPawn_LocustBrumakBase(Pawn.Base);
		ClampRotation( out_ViewRotation, Brumak.Rotation, rot(12000,14563,-1), rot(14563,14563,-1) );
	}

	function Rotator GetAdjustedAimFor(Weapon W, vector StartFireLoc)
	{
		local Rotator		CamRot;
		local Vector		HitLocation, HitNormal, EndTrace, StartTrace, CamLoc;
		local Actor			HitActor;
		local ImpactInfo	Impact;

		GetPlayerViewPoint( CamLoc, CamRot );
		StartTrace	= CamLoc;
		EndTrace	= CamLoc + vector(CamRot)*10000;
		CalcWeaponFire( StartTrace, EndTrace, HitLocation, HitNormal, HitActor, Impact );

		return rotator(HitLocation-StartFireLoc);
	}
}

function OnDriveBrumak( SeqAct_DriveBrumak inAction )
{
	// Mount
	if( inAction.InputLinks[0].bHasImpulse )
	{
		DriveBrumak( GearPawn_LocustBrumakBase(inAction.BrumakPawn), (inAction.Driver == self || inAction.Driver == Pawn) );
	}
	else
	// Dismount
	if( inAction.InputLinks[1].bHasImpulse )
	{
		DismountBrumak( GearPawn_LocustBrumakBase(inAction.BrumakPawn), ((inAction.BrumakPawn == Pawn) ? inAction.DismountDest[0] : inAction.DismountDest[1]) );
	}

}

function DriveBrumak( GearPawn_LocustBrumakBase Brumak, bool bTakeDriverSeat )
{
	local GearPawn OldPawn;

	if( Brumak == None )
	{
		return;
	}
	OldPawn = MyGearPawn;
	if( OldPawn == None )
	{
		return;
	}

	`DLog("Brumak:" @ Brumak @ "bTakeDriverSeat:" @ bTakeDriverSeat);

	OldPawn.Health = OldPawn.DefaultHealth;

	if( bTakeDriverSeat )
	{
		if( Brumak.Driver == None || !Brumak.IsHumanControlled() )
		{
			UnPossess();
			Brumak.SetDriver( OldPawn );
			Possess( Brumak, TRUE );
		}
	}
	// Take gunner seat
	else
	if( (Brumak.LeftGunPawn  == None || !Brumak.LeftGunPawn.IsHumanControlled())  &&
		(Brumak.RightGunPawn == None || !Brumak.RightGunPawn.IsHumanControlled()) )
	{
		Brumak.SetGunner( OldPawn, 0 );
		EnterStartState();
	}
	else
	{
		`warn( "Failed to drive brumak - already occupied by human" );
	}
}

function DismountBrumak( GearPawn_LocustBrumakBase Brumak, Actor Dest )
{
	local Pawn		OldPawn;

	if( Brumak == None || Dest == None )
	{
		return;
	}

	`DLog("Brumak:" @ Brumak @ "Dest:" @ Dest @ "Pawn:" @ Pawn @ "MyGearPawn:" @ MyGearPawn);

	if( Pawn == Brumak )
	{
		OldPawn = Brumak.Driver;
		Brumak.SetDriver( None );
		UnPossess();
		Possess( OldPawn, TRUE );
	}
	else if( Brumak.HumanGunner == Pawn )
	{
		OldPawn = Brumak.HumanGunner;
		Brumak.SetGunner( None, 0 );
		SetViewTarget(OldPawn);
		EnterStartState();
		ClientGotoState(GetStateName());
	}

	OldPawn.SetLocation( Dest.Location );
	OldPawn.DropToGround();
	OldPawn.SetCollision( TRUE, TRUE, FALSE );
	OldPawn.bCollideWorld = TRUE;
	OldPawn.SetPhysics(PHYS_Falling);
}

/** used to replicate rotation to the server in places where we only care about rotation input */
unreliable server function ServerUpdateRotation(int View)
{
	local rotator ViewRot;

	ViewRot.Pitch = (View & 65535);
	ViewRot.Yaw = (View >> 16);
	SetRotation(ViewRot);
}

/************************************************************************/
/*  CRATE                                                                     */
/************************************************************************/
state PlayerDrivingCrate
{
	/** Used to take input and pass it to the crate on the server */
	function PlayerMove( float DeltaTime )
	{
		local FLOAT	Forward, Strafe;

		// update 'looking' rotation
		UpdateRotation(DeltaTime);

		// Remap input
		Forward = PlayerInput.RawJoyUp;
		Strafe = PlayerInput.RawJoyRight;

		if(Role < ROLE_Authority)
		{
			//`log("CALLING TO SERVER");
			ServerPassInputToCrate(Forward, Strafe);
			ServerUpdateRotation(((Rotation.Yaw & 65535) << 16) + (Rotation.Pitch & 65535));
		}
		else
		{
			PassInputToCrate(Forward, Strafe);
		}
	}

	event BeginState(name PreviousStateName)
	{
		local GearWeapon Weap;

		if (Pawn != None && LocalPlayer(Player) != None)
		{
			foreach Pawn.InvManager.InventoryActors(class'GearWeapon', Weap)
			{
				if (Weap.WeaponType == WT_Holster)
				{
					`log("Forcing carrier to switch to:"@Weap);
					Pawn.InvManager.SetCurrentWeapon(Weap);
					break;
				}
			}
		}
	}
}

/** Use to send input from client to server to then pass to crate on server */
reliable server function ServerPassInputToCrate(FLOAT Forward, FLOAT Strafe)
{
	PassInputToCrate(Forward, Strafe);
}

/** Function that passes control input for each carrier to the held crate */
function PassInputToCrate(FLOAT Forward, FLOAT Strafe)
{
	if(MyGearPawn != None && MyGearPawn.CarriedCrate != None)
	{
		MyGearPawn.CarriedCrate.SetInputForPawn(MyGearPawn, Forward, Strafe);
	}
}

function EnterStartState()
{
	local GearPawn_LocustBrumakBase Brumak;
	local name NewState;

	if( Pawn.PhysicsVolume.bWaterVolume )
	{
		if( Pawn.HeadVolume.bWaterVolume )
		{
			Pawn.BreathTime = Pawn.UnderWaterTime;
		}
		NewState = Pawn.WaterMovementState;
	}
	else
	{
		Brumak = GearPawn_LocustBrumakBase(Pawn.Base);
		if( Brumak != None )
		{
			NewState = Brumak.GunnerState;
		}
		else
		{
			NewState = Pawn.LandMovementState;
		}
	}

	if( IsInState( NewState ) )
	{
		BeginState( NewState );
	}
	else
	{
		GotoState( NewState );
	}
}

/** Client function so that the MP game objects can force the scoreboard up. */
reliable client function ClientToggleScoreboard( bool bOn )
{
	MyGearHud.ToggleScoreboard( bOn );
}

/** Called by the Annex game object to have the Command Point Indicator glow */
reliable client function ClientSetCommandPointIndicatorGlow( int NumPulses, float LengthOfPulse = 1.f )
{
	if ( MyGearHud != None )
	{
		MyGearHud.SetCommandPointIndicatorGlow( NumPulses, LengthOfPulse );
	}
}

/** Client call to tell HUD to draw an annex message */
reliable client function ClientAddAnnexMessage( PlayerReplicationInfo PlayerPRI, String AnnexMessage )
{
	if ( MyGearHud != None )
	{
		MyGearHud.AddAnnexMessage( PlayerPRI, AnnexMessage );
	}
}

/** Client call to allow us to do HUD effects and sounds because of the end of the game approaching */
reliable client function ClientEndOfGameWarning( int ScoreDiff, int TeamIndex )
{
	if ( MyGearHud != None )
	{
		MyGearHud.DoEndOfGameWarning( ScoreDiff, TeamIndex );
	}
}

/** Handles the ObjectiveManager kismet action by forwarding the action to the objective manager on the client */
simulated function OnManageObjectives( SeqAct_ManageObjectives Action )
{
	if ( ObjectiveMgr != None )
	{
		ObjectiveMgr.OnManageObjectives( Action );
	}
}

/** Replicate the updated objective to the client */
reliable client final function ClientReplicateObjective( ObjectiveInfo UpdatedInfo, int ObjIdx )
{
	if ( ObjectiveMgr != None )
	{
		ObjectiveMgr.Objectives[ObjIdx] = UpdatedInfo;
		ObjectiveMgr.Objectives[ObjIdx].UpdatedTime = WorldInfo.TimeSeconds;
	}
}

/** clears the objective list on the client */
reliable client final function ClientClearObjectives()
{
	if (ObjectiveMgr != None)
	{
		ObjectiveMgr.Objectives.length = 0;
	}
}

/** Have the HUD show a kismet message */
function OnDrawMessage(SeqAct_DrawMessage Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		ClientDrawKismetMessage( Action.MessageText, Action.DisplayTimeSeconds );
	}
	else
	{
		ClientClearKismetMessage();
	}
}

/** Start drawing a kismet message on the HUD */
reliable client final function ClientDrawKismetMessage( string MessageText, float DisplayTime )
{
	MyGearHud.KismetMessageText = MessageText;
	if ( DisplayTime > 0 )
	{
		MyGearHud.KismetMessageEndTime = WorldInfo.TimeSeconds + DisplayTime;
	}
	else
	{
		MyGearHud.KismetMessageEndTime = -1;
	}
}

/** Stop drawing a kismet message on the HUD */
reliable client final function ClientClearKismetMessage()
{
	MyGearHud.KismetMessageText = "";
	MyGearHud.KismetMessageEndTime = -1;
}

exec function TestPitchCentering(float GoalPitch)
{
	MainPlayerInput.ForcePitchCentering(TRUE, TRUE, GoalPitch);
}

native function CauseHitch(float Sec);

exec function Hitch(float sec)
{
	CauseHitch(Sec);
}

/** Handles the call from kismet to the tutorial system. */
final function OnManageTutorials( SeqAct_ManageTutorials Action )
{
	local EManageTutorialInputs TutInput;
	local GearPC PC;

	if (TutorialMgr != None)
	{
		// Figure out which input was hit
		if (Action.InputLinks[eMTINPUT_AddTutorial].bHasImpulse)
		{
			TutInput = eMTINPUT_AddTutorial;
		}
		else if (Action.InputLinks[eMTINPUT_RemoveTutorial].bHasImpulse)
		{
			TutInput = eMTINPUT_RemoveTutorial;
		}
		else if (Action.InputLinks[eMTINPUT_StartTutorial].bHasImpulse)
		{
			TutInput = eMTINPUT_StartTutorial;
		}
		else if (Action.InputLinks[eMTINPUT_StopTutorial].bHasImpulse)
		{
			TutInput = eMTINPUT_StopTutorial;
		}
		else if (Action.InputLinks[eMTINPUT_CompleteTutorial].bHasImpulse)
		{
			TutInput = eMTINPUT_CompleteTutorial;
		}
		else if (Action.InputLinks[eMTINPUT_SystemOn].bHasImpulse)
		{
			TutInput = eMTINPUT_SystemOn;
		}
		else if (Action.InputLinks[eMTINPUT_SystemOff].bHasImpulse)
		{
			TutInput = eMTINPUT_SystemOff;
		}
		else if (Action.InputLinks[eMTINPUT_AutosOn].bHasImpulse)
		{
			TutInput = eMTINPUT_AutosOn;
		}
		else if (Action.InputLinks[eMTINPUT_AutosOff].bHasImpulse)
		{
			TutInput = eMTINPUT_AutosOff;
		}

		// Loop through all players and process the tutorial action
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.ClientManageTutorialsFromKismet(Action, TutInput);
		}
	}
}

/** Handles the tutorial system functionality that was triggered by kismet on the server */
reliable client function ClientManageTutorialsFromKismet(SeqAct_ManageTutorials Action, EManageTutorialInputs TutInput)
{
	local bool bIsHostPlayer;

	bIsHostPlayer = (WorldInfo.NetMode < NM_Client) && IsPrimaryPlayer();

	if (TutorialMgr != None)
	{
		switch (TutInput)
		{
			// Add Tutorial
			case eMTINPUT_AddTutorial:
				Action.InputLinks[eMTINPUT_AddTutorial].bHasImpulse = false;
				// If the tutorial is already complete we will fire the "Already Complete" output and not add it
				if (TutorialMgr.IsTutorialCompleted(Action.Tutorial_Type))
				{
					Action.OutputLinks[eMTOUTPUT_AlreadyComplete].bHasImpulse = true;
					Action.bTutorialAlreadyCompleted = true;
					Action.bActionIsDone = true;
				}
				// Not complete yet so add the tutorial and start it if it should be started
				else
				{
					if (TutorialMgr.AddTutorial(Action.Tutorial_Type, bIsHostPlayer ? Action : None, true))
					{
						Action.OutputLinks[eMTOUTPUT_Added].bHasImpulse = true;
					}

					// Start the tutorial if the action says to
					if (Action.bStartTutorialWhenAdded)
					{
						TutorialMgr.StartTutorial(Action.Tutorial_Type);
					}
				}
				break;
			// Remove Tutorial
			case eMTINPUT_RemoveTutorial:
				Action.InputLinks[eMTINPUT_RemoveTutorial].bHasImpulse = false;
				if (TutorialMgr.RemoveTutorial(Action.Tutorial_Type))
				{
					Action.OutputLinks[eMTOUTPUT_Removed].bHasImpulse = true;
				}
				break;
			// Start Tutorial
			case eMTINPUT_StartTutorial:
				Action.InputLinks[eMTINPUT_StartTutorial].bHasImpulse = false;
				if (TutorialMgr.StartTutorial(Action.Tutorial_Type))
				{
					Action.OutputLinks[eMTOUTPUT_Started].bHasImpulse = true;
				}
				break;
			// Stop Tutorial
			case eMTINPUT_StopTutorial:
				Action.InputLinks[eMTINPUT_StopTutorial].bHasImpulse = false;
				if (TutorialMgr.StopTutorial(Action.Tutorial_Type))
				{
					Action.OutputLinks[eMTOUTPUT_Stopped].bHasImpulse = true;
				}
				break;
			// Complete Tutorial
			case eMTINPUT_CompleteTutorial:
				Action.InputLinks[eMTINPUT_CompleteTutorial].bHasImpulse = false;
				TutorialMgr.OnTutorialCompletedFromKismet(Action.Tutorial_Type, false);
				break;
			// System On
			case eMTINPUT_SystemOn:
				bTutorialSystemWasOn = true;
				RestartTutorialSystem(bTutorialSystemWasOn, Action.bWipeTutorialsOnSystemOn);
				TutorialMgr.AddAutoInitiatedTutorials();
				Action.OutputLinks[eMTOUTPUT_SystemOn].bHasImpulse = true;
				Action.bActionIsDone = true;
				break;
			// System Off
			case eMTINPUT_SystemOff:
				bTutorialSystemWasOn = false;
				StopTutorialSystem(Action.bWipeTutorialsOnSystemOff);
				Action.OutputLinks[eMTOUTPUT_SystemOff].bHasImpulse = true;
				Action.bActionIsDone = true;
				break;
			// Autos On
			case eMTINPUT_AutosOn:
				TutorialMgr.UnsuspendAutoTutorials();
				Action.OutputLinks[eMTOUTPUT_AutosOn].bHasImpulse = true;
				Action.bActionIsDone = true;
				break;
			// Autos Off
			case eMTINPUT_AutosOff:
				TutorialMgr.SuspendAutoTutorials();
				Action.OutputLinks[eMTOUTPUT_AutosOff].bHasImpulse = true;
				Action.bActionIsDone = true;
				break;
		}
	}
}

/** The actual server RPC from TellServerTutorialCompletedFromClient in the tutorial manager */
reliable server function ServerClientHasCompletedTutorial(EGearTutorialType TutType)
{
	local GearPC PC;
	// Find the host and complete his tutorial
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if (PC.IsPrimaryPlayer())
		{
			if (PC.TutorialMgr != None)
			{
				PC.TutorialMgr.OnTutorialCompletedFromKismet(TutType, false);
			}
			break;
		}
	}
}

/** Called by the host player when he has completed a tutorial so that the client can complete theirs on his behalf */
final function TellClientTutorialCompletedFromServer(EGearTutorialType TutType)
{
	local GearPC PC;

	// Find the client and complete his tutorial
	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		if (!PC.IsPrimaryPlayer() ||
			LocalPlayer(PC.Player) == None)
		{
			PC.ClientServerHasCompletedTutorial(TutType);
		}
	}
}

/** The actual client RPC from TellServerTutorialCompletedFromClient in the tutorial manager */
reliable client function ClientServerHasCompletedTutorial(EGearTutorialType TutType)
{
	TutorialMgr.OnTutorialCompletedFromKismet(TutType, true);
}

/** Delegate fired when the player triggers an event in the GearEventDelegates array */
delegate OnGearEvent();

/**
 * Adds the delegate to the list to be notified when the player triggers an event
 *	@param GearEventType - the event type you want to add a delegate to
 *	@param GearEventDelegate - the delegate to add to the event
 */
final function AddGearEventDelegate( EGearEventDelegateType GearEventType, delegate<OnGearEvent> GearEventDelegate )
{
	// Set the size of the event list if we are outside the bounds of the array
	if ( GearEventList.length <= GearEventType )
	{
		GearEventList.length = eGED_NumTypes;
	}

	// Only add to the list once
	if ( GearEventList[GearEventType].GearEventDelegates.Find(GearEventDelegate) == INDEX_NONE )
	{
		GearEventList[GearEventType].GearEventDelegates.AddItem(GearEventDelegate);
	}
}

/**
* Removes the delegate from the list to be notified when the player triggers an event
*	@param GearEventType - the event type you want to add a delegate to
*	@param GearEventDelegate - the delegate to add to the event
*/
final function ClearGearEventDelegate( EGearEventDelegateType GearEventType, delegate<OnGearEvent> GearEventDelegate )
{
	// Set the size of the event list if we are outside the bounds of the array
	if ( GearEventList.length <= GearEventType )
	{
		GearEventList.length = eGED_NumTypes;
	}

	GearEventList[GearEventType].GearEventDelegates.RemoveItem( GearEventDelegate );
}

/**
 * Calls all of the delegates associated with an event
 *	@param GearEventType - the event type you want to trigger the delegates for
 */
final function TriggerGearEventDelegates( EGearEventDelegateType GearEventType )
{
	local int DelegateIdx;
	local delegate<OnGearEvent> OnEventDelegate;

	// Set the size of the event list if we are outside the bounds of the array
	if ( GearEventList.length <= GearEventType )
	{
		GearEventList.length = eGED_NumTypes;
	}

	for ( DelegateIdx = 0; DelegateIdx < GearEventList[GearEventType].GearEventDelegates.length; DelegateIdx++ )
	{
		OnEventDelegate = GearEventList[GearEventType].GearEventDelegates[DelegateIdx];
		OnEventDelegate();
	}
}

/** Delegate fired when the player picks up a weapon */
delegate OnGearWeaponEquip( class<GearWeapon> WeaponClass );

/**
 * Adds the delegate to the list to be notified when the player equips a weapon
 *	@param GearWeapEquipDelegate - the delegate to add
 */
final function AddGearWeaponEquipDelegate( delegate<OnGearWeaponEquip> GearWeapEquipDelegate )
{
	// Only add to the list once
	if ( GearWeaponEquipDelegates.Find(GearWeapEquipDelegate) == INDEX_NONE )
	{
		GearWeaponEquipDelegates.AddItem(GearWeapEquipDelegate);
	}
}

/**
 * Removes the delegate from the list to be notified when the player equips a weapon
 *	@param GearWeapEquipDelegate - the delegate to add to the event
 */
final function ClearGearWeaponEquipDelegate( delegate<OnGearWeaponEquip> GearWeapEquipDelegate )
{
	GearWeaponEquipDelegates.RemoveItem( GearWeapEquipDelegate );
}

/**
 * Calls all of the weapon equipped delegates
 */
final function TriggerGearWeaponEquipDelegates( class<GearWeapon> WeaponClass )
{
	local int DelegateIdx;
	local delegate<OnGearWeaponEquip> OnWeaponEquipDelegate;

	for ( DelegateIdx = 0; DelegateIdx < GearWeaponEquipDelegates.length; DelegateIdx++ )
	{
		OnWeaponEquipDelegate = GearWeaponEquipDelegates[DelegateIdx];
		OnWeaponEquipDelegate( WeaponClass );
	}
}

/** Opens a UI scene */
client reliable event UIScene ClientOpenScene( UIScene SceneReference, optional byte OverrideEmulateButtonPress )
{
	local UIScene OpenedScene;
	if ( SceneReference != None )
	{
		OpenedScene = SceneReference.OpenScene(SceneReference, LocalPlayer(Player));
	}
	return OpenedScene;
}

/** Closes a UI scene */
client reliable event ClientCloseScene( UIScene SceneInstance )
{
	if ( SceneInstance != None )
	{
		SceneInstance.CloseScene();
	}
}

/** closes all menus that are up */
reliable client function ClientCloseAllMenus()
{
	local GameUISceneClient SceneClient;
	local UIScene SceneToClose;

	// closing child scenes stops when it hits a scene that has a higher priority, so iterate backwards through the scene stack
	// closing each one individually.
	SceneClient = class'UIRoot'.static.GetSceneClient();
	foreach SceneClient.AllActiveScenes(class'UIScene', SceneToClose, true, INDEX_NONE, SceneClient.SCENEFILTER_ReceivesFocus)
	{
		// skip over any 'reconnect controller' scenes
		if ( SceneToClose.SceneTag != 'ReconnectController' )
		{
			SceneClient.CloseScene(SceneToClose, false, true);
		}
	}
}

/** Function called when the path choice action is initiated */
function OnDisplayPathChoice( SeqAct_DisplayPathChoice Action )
{
	local LocalPlayer LP;
	local UIScene SceneInstance;
	local int PlayerIndex;

	LP = LocalPlayer(Player);

	// only open the 'choose path' scene if we're the first player (relevant only in split-screen)
	if ( LP != None )
	{
		MainPlayerInput.ForceButtonRelease(bUseAlternateControls?GB_X:GB_A, true);
		DoubleClickDir = DCLICK_None;

		// Open the scene
		if ( MyGearHud != None && MyGearHud.PathChoiceSceneRef != None && Role == ROLE_Authority
		&&	(!IsSplitscreenPlayer(PlayerIndex) || PlayerIndex == 0) )
		{
			SceneInstance = ClientOpenScene( MyGearHud.PathChoiceSceneRef );
			if ( SceneInstance != None )
			{
				Action.PathChoiceSceneInstance = GearUIScene_PathChoice(SceneInstance);
				if ( Action.PathChoiceSceneInstance != None )
				{
					Action.InitializePathChoice( SceneInstance, LP );
				}
			}
		}
	}
}

/**
 * Called when an arbitrated match has ended and we need to return to our party
 */
reliable client function ClientArbitratedMatchEnded()
{
	local OnlineGameSettings PartySettings;

	// Show the scoreboard which will go to the main menu when done
	MyGearHud.ToggleScoreboard(TRUE);

	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
	}
	if (PartySettings == None)
	{
		Super.ClientArbitratedMatchEnded();
	}
}

exec function OpenShield()
{
	GearPawn(Pawn).EquippedShield.Expand();
}
exec function CloseShield()
{
	GearPawn(Pawn).EquippedShield.Retract();
}


exec function TestBloodTrail( string TrailType )
{
	if( TrailType ~= "MeatBag" )
	{
		GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_MeatBag' );
	}
	else if( TrailType ~= "Wall" )
	{
		GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_Wall' );
	}
	else if( TrailType ~= "DBNO" )
	{
		Pawn.GotoState( 'DBNO' );
		//DoSpecialMove(SM_DBNO,TRUE);
		//GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_MeatBag' );
	}
	else if( TrailType ~= "ChainSawSpray" )
	{
		GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Ground' );
		GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Wall' );
	}
	else if( TrailType ~= "GoreBreak" )
	{
		//GearPawn(Pawn).StartBloodTrail( 'SpawnABloodTrail_MeatBag' );
	}
	else
	{
		`log( "TestBloodTrail TrailType not Recognized" );
		`log( "Valid TrailTypes: MeatBag Wall DBNO ChainSawSpray GoreBreak" );
	}
}


exec function StopBloodTrails()
{
	GearPawn(Pawn).StopBloodTrail( 'SpawnABloodTrail_DBNO' );
	GearPawn(Pawn).StopBloodTrail( 'SpawnABloodTrail_MeatBag' );
	GearPawn(Pawn).StopBloodTrail( 'SpawnABloodTrail_Wall' );
	GearPawn(Pawn).StopBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Ground' );
	GearPawn(Pawn).StopBloodTrail( 'SpawnABloodTrail_ChainsawSpray_Wall' );
}

/** turns the player into a spectator and gives the AI control over his Pawn */
exec function MakeMeABot()
{
	ServerMakeMeABot();
}
reliable server function ServerMakeMeABot()
{
	local GearAI AI;
	local Pawn OldPawn;

	if (Pawn != None)
	{
		OldPawn = Pawn;
		UnPossess();
		if (GearGameMP_Base(WorldInfo.Game) != None)
		{
			AI = Spawn(class'GearAI_TDM');
			AI.Possess(OldPawn, false);
		}
		else
		{
			AI = Spawn(class'GearAI_CogGear');
			AI.Possess(OldPawn, false);
			AI.SetSquadName('Alpha');
		}

		if (PlayerReplicationInfo.Team != None)
		{
			PlayerReplicationInfo.Team.AddToTeam(AI);
			PlayerReplicationInfo.Team.RemoveFromTeam(self);
			PlayerReplicationInfo.Team = None;
			if (GearGameMP_Base(WorldInfo.Game) != None)
			{
				GearTeamInfo(AI.PlayerReplicationInfo.Team).CreateMPSquads();
			}
		}
		PlayerReplicationInfo.bOnlySpectator = true;
		TransitionToSpectate('PlayerSpectating');
		SetViewTarget(OldPawn);
	}
}

/** Start showing the sudden death text */
final reliable client function ClientStartSuddenDeathText()
{
	local float DrawYPos;
	PlaySound( SuddenDeathSound, true );

	DrawYPos = MyGearHud.GetCountdownYPosition() + MyGearHud.GetCountdownHeight() + NumPixelsToMoveDownForDramaText();
	MyGearHud.StartDrawingDramaticText( SuddenDeathString, "", 5.0f, 0.25f, 2.0f, DrawYPos, 0.25f, 1.0f );

	// See if we need to teach sudden death in TG
	AttemptTrainingGroundTutorial(GEARTUT_TRAIN_SudDeath, -1);
}

/**
 * Checks to see if we are in training grounds and checks to see if it's the correct game mode for the tutorial
 *
 * @param TutType - the tutorial type to attempt to start
 * @param TrainType - the specific training ground this tutorial is supposed to play on (-1 mean all of them)
 */
final function AttemptTrainingGroundTutorial(EGearTutorialType TutType, int TrainType)
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != none && TutorialMgr != none)
	{
		if (GRI.TrainingGroundsID >= 0 &&
			(TrainType == -1 || TrainType == GRI.TrainingGroundsID))
		{
			TutorialMgr.StartTutorial(TutType);
		}
	}
}

/** Start showing the revival text */
final reliable client function ClientStartDrawingRevival( GearPawn PawnWhoRevivedMe )
{
	local string StringToDraw;

	if ( (PawnWhoRevivedMe != None) && (PawnWhoRevivedMe != MyGearPawn) && (MyGearHud != None) )
	{
		StringToDraw = RevivedString @ PawnWhoRevivedMe.PlayerReplicationInfo.PlayerName;
		StringToDraw = WorldInfo.GRI.Caps(StringToDraw);
		MyGearHud.StartDrawingSubtitleText( StringToDraw, 3.0f, 0.5f, 1.0f );
	}
}

simulated function StopWeatherEffects(EWeatherType WeatherType)
{
	if (WeatherEmitter != None)
	{
		WeatherEmitter.FadeOutRain();
		WeatherEmitter = None;
	}
}

simulated function StartWeatherEffects(EWeatherTYpe WeatherType)
{
	switch (WeatherType)
	{
	case WeatherType_Rain:
		WeatherEmitter = Spawn(class'Emit_Rain');
		break;
	case WeatherType_Hail:
		WeatherEmitter = Spawn(class'Emit_Hail');
		break;
	case WeatherType_Clear:
		// nothing to do!
		break;
	}

	if (WeatherEmitter != None)
	{
		WeatherEmitter.SetBasedOnPlayer(self);
	}
}

simulated function SetWeatherEmitterHeight(float Height)
{
	if (WeatherEmitter != None)
	{
		WeatherEmitter.SetHeight(Height);
	}
}



// simulated function float DetermineWhichDecalToUse( out MaterialInstance out_MI_Decal )
// {
// 	local float Retval;
//
// 	local Vector Loc;
// 	local Rotator Rot;
// 	local float DirOfHit_L;
//
// 	local vector	AxisX, AxisY, AxisZ;
//
// 	local vector ShotDirection;
//
// 	local bool bIsInFront;
// 	local vector2D	AngularDist;
// 	local Pawn P;
// 	//local GearHud_Base WH;
//
// 	local float Multiplier;
// 	local float PositionInQuadrant;
// 	local float QuadrantStart;
//
// 	P = Pawn;
// 	P.GetAxes(P.Rotation, AxisX, AxisY, AxisZ);
//
// 	ShotDirection = -1.f * Normal( P.Location - LocationOfLastBloodTrail );
//
//
// 	bIsInFront = GetAngularDistance( AngularDist, ShotDirection, AxisX, AxisY, AxisZ );
// 	GetAngularDegreesFromRadians( AngularDist );
//
// 	Multiplier = 0.26f / 90.f;
//
// 	PositionInQuadrant = Abs(AngularDist.X) * Multiplier;
//
//
// //	Retval = ((abs(Pawn.Rotation.Yaw)%65535.0f)/65535.0f)*360.0f;
//
// 	if( Rotator(Pawn.Location - LocationOfLastBloodTrail).Yaw >= 0 )
// 	{
// 		Retval = abs((Rotator(Pawn.Location - LocationOfLastBloodTrail).Yaw)/65535.0f) * -360.0f;
// 	}
// 	else
// 	{
// 		Retval = abs((Rotator(Pawn.Location - LocationOfLastBloodTrail).Yaw)/65535.0f) * 360.0f;
// 	}
//
// 	//Retval = ((Rotator(Pawn.Location - LocationOfLastBloodTrail).Yaw)/65535.0f) * 360.0f;
//
// 	`log( "Rotation: " $ Pawn.Rotation.Yaw @ Retval );
//
// 	if( LocationOfLastBloodTrail == Vect(0,0,0) )
// 	{
// 		// leave a bloood pool
// 		`log( "leave a blood pool" );
// 		out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
// 	}
// 	else //if( PositionInQuadrant < 0.13 )
// 	{
// 		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_3' );
// 		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_6' );
// 		//out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
// 		//		out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_8' );  // awesome for crawling
//
// // 		if( FRand() > 0.40f )
// // 		{
// // 			out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_5' );
// // 		}
// // 		else
// // 		{
// // 			out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Spot_1' );
// // 		}
//
//
//
// 		if( FRand() > 0.40f )
// 		{
// 			out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Decals.BloodSplatter02' );
// 		}
// 		else
// 		{
// 			out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_Straight_5' );
// 		}
// 	}
//
// 	if( FALSE ) //else
// 	{
//
// 		`log( "bIsInFront: " $ bIsInFront @ "AngularDist: " $ AngularDist.X @ "LocationOfLastBloodTrail" @ LocationOfLastBloodTrail @ "PositionInQuadrant" @ PositionInQuadrant );
//
//
// 		// 0 - .25  UpperRight
// 		// .25 - .50 LowerRight
// 		// .50 - .75 LowerLeft
// 		// .75 - 1 UpperLeft
// 		if( bIsInFront == TRUE )
// 		{
// 			if( AngularDist.X > 0 )
// 			{
// 				QuadrantStart = 0;
// 				DirOfHit_L = QuadrantStart + PositionInQuadrant;
// 				`log( "Lower Left" );
// 				out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_CurveLeft_3' );
// 			}
// 			else
// 			{
// 				QuadrantStart = 0;
// 				DirOfHit_L = QuadrantStart - PositionInQuadrant;
// 				`log( "Lower Right" );
// 				out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_CurveRight_3' );
// 			}
// 		}
// 		else
// 		{
// 			if( AngularDist.X > 0 )
// 			{
// 				QuadrantStart = 0.52;
// 				DirOfHit_L = QuadrantStart - PositionInQuadrant;
// 				`log( "Upper Left" );
// 				out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_CurveRight_3' );
// 			}
// 			else
// 			{
// 				QuadrantStart = 0.52;
// 				DirOfHit_L = QuadrantStart + PositionInQuadrant;
// 				`log( "Upper Right" );
// 				out_MI_Decal.SetParent( MaterialInstance'Gear_Blood.Materials.DM_BloodSmear_Var01_CurveLeft_3' );
//
// 			}
// 		}
// 	}
//
// 	return Retval;
// }

state ScreenshotMode extends Spectating
{
	function BeginState(Name PreviousStateName)
	{
		LastPawn = MyGearPawn;
		Super.BeginState(PreviousStateName);
		SetPhysics(PHYS_Flying);
	}

	function PlayerMove(float DeltaTime)
	{
		if (!IsButtonActive(GB_LeftTrigger))
		{
			Super.PlayerMove(DeltaTime);
		}
	}

	function UpdateRotation(float DeltaTime )
	{
		Super(GearPC).UpdateRotation(DeltaTime);
	}

	function EndState(Name NextStateName)
	{
		SetPhysics(PHYS_None);
		Super.EndState(NextStateName);
		Possess(LastPawn,FALSE);
	}
};

exec function Stuck()
{
	`log("Player stuck:"@GetStateName()@MyGearPawn@MyGearPawn.GetStateName()@MyGearPawn.SpecialMove@IsMoveInputIgnored()@IsLookInputIgnored()@MyGearPawn.CurrentLink@MyGearPawn.CoverType@MyGearPawn.CoverAction);
	ClientBreakFromCover();
	DoSpecialMove(SM_None,TRUE);
	MyGearPawn.SetMovementPhysics();
}

/** Message sent from Assassination game mode that a leader has died */
reliable client function ClientLeaderDied( GearPRI LeaderPRI )
{
	if ( MyGearHud != None )
	{
		MyGearHud.StartLeaderDiedMessage( LeaderPRI );
	}
}

/** Message sent from Wingman game mode that your buddy has died */
reliable client function ClientBuddyDied( GearPRI BuddyPRI )
{
	if ( MyGearHud != None )
	{
		MyGearHud.StartBuddyDiedMessage( BuddyPRI );
	}
}

/**
 * Returns the party map name for this game
 */
static function string GetPartyMapName()
{
	return "GearStart";
}

/**
 * Returns the party game info name for this game
 */
static function string GetPartyGameTypeName()
{
	return "GearGameContent.GearPartyGame";
}

/**
 * Get the completion amount for a game achievement.
 *
 * @param	AchievementId	the id for the achievement to get the completion percetage for
 * @param	CurrentValue	the current number of times the event required to unlock the achievement has occurred.
 * @param	MaxValue		the value that represents 100% completion.
 *
 * @return	TRUE if the AchievementId specified represents an progressive achievement.  FALSE if the achievement is a one-time event
 *			or if the AchievementId specified is invalid.
 */
event bool GetAchievementProgression( int AchievementId, out float CurrentValue, out float MaxValue )
{
	local bool bResult;

	if ( ProfileSettings != None )
	{
		bResult = ProfileSettings.GetAchievementProgression(EGearAchievement(AchievementId), CurrentValue, MaxValue);
	}

	return bResult;
}

/** Function to stop playing the looping music when a discoverable is being looked at in game */
final function DiscoverableSceneClosed( float FadeOut )
{
	WorldInfo.bPlayersOnly = false;
	if ( MyGearHUD != None )
	{
		MyGearHUD.DiscoverUISceneInstance = None;
	}
}

/** called when seamless travelling and the specified PC is being replaced by this one
 * copy over data that should persist
 * (not called if PlayerControllerClass is the same for the from and to gametypes)
 */
function SeamlessTravelFrom(PlayerController OldPC)
{
	local GearTeamInfo MyTeamInfo;
	local int Idx;

	Super.SeamlessTravelFrom(OldPC);

	MyTeamInfo = GearTeamInfo(PlayerReplicationInfo.Team);
	Idx = MyTeamInfo.TeamMembers.Find( OldPC );
	if ( Idx != INDEX_NONE )
	{
		MyTeamInfo.TeamMembers[Idx] = self;
	}

	// Ensure the profile settings are initially set
	bProfileSettingsUpdated = TRUE;

	RefreshAllSafeZoneViewports();
}

/** Loop through all controllers and refresh their safe zone values */
function RefreshAllSafeZoneViewports()
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if (PC.MyGearHud != none)
		{
			PC.MyGearHud.bRefreshSafeZone = true;
		}
	}
}

/** Scripting hook for kismet driven achievement unlocking */
function OnGearAchievementUnlock(SeqAct_GearAchievementUnlock inAction)
{
	if ( inAction.InputLinks[0].bHasImpulse && inAction.Achievement != eGLACHIEVE_None )
	{
		ClientUnlockAchievement( inAction.AchievementMap[inAction.Achievement] );
	}
}

/** Scripting hook for kismet driven unlockable unlocking */
function OnGearUnlockableUnlock(SeqAct_GearUnlockableUnlock inAction)
{
// 	if ( inAction.InputLinks[0].bHasImpulse && ProfileSettings != None )
// 	{
// 		ProfileSettings.MarkUnlockableAsUnlockedButNotViewed( inAction.UnlockableMap[inAction.Unlockable], self );
// 	}
}

/** Client RPC to unlock an achievement - first adds an "achievement complete" delegate then unlocks it */
reliable client function ClientUnlockAchievement( EGearAchievement Achievement, optional bool bFromUpdateKillingProgression )
{
	local LocalPlayer LocPlayer;

	if (!bFromUpdateKillingProgression && Achievement == eGA_Seriously2)
	{
		// special handling for seriously
		ProfileSettings.UpdateKillingProgression(class'GDT_Ballistic', self);
	}
	else
	{
		`log("Unlocked the " $ Achievement $ " achievement!");
		if ( OnlineSub != None )
		{
			// if the extended interface is supported
			if ( OnlineSub.PlayerInterfaceEx != None )
			{
				LocPlayer = LocalPlayer(Player);
				if ( !OnlineSub.PlayerInterfaceEx.UnlockAchievement(LocPlayer.ControllerId, Achievement) )
				{
					`log("UnlockAchievement("$LocPlayer.ControllerId$","$Achievement$") failed");
				}
				// Successful, see if we should unlock a player pick
				else
				{
					if (Achievement == eGA_ICantQuitYouDom)
					{
						OnlineSub.PlayerInterfaceEx.UnlockGamerPicture(LocPlayer.ControllerId, 2);
					}
					else if (Achievement == eGA_Commander)
					{
						OnlineSub.PlayerInterfaceEx.UnlockGamerPicture(LocPlayer.ControllerId, 1);
					}
				}
			}
			else
			{
				`Log("Interface is not supported. Can't unlock an achievement");
			}
		}
		else
		{
			`Log("No online subsystem. Can't unlock an achievement");
		}

		if ( ProfileSettings != None )
		{
			ProfileSettings.MarkAchievementAsCompleted(Achievement, Self);
		}
	}
}

/** Client RPC to update the profile for the duel chainsaw achievement */
reliable client function ClientUpdateChainsawDuelWonProgress()
{
	if (ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_CrossedSwords, self);
	}
}

/** Client RPC to update the profile for the meatshield achievement */
reliable client function ClientUpdateMeatshieldProgress()
{
	if (ProfileSettings != None)
	{
		ProfileSettings.UpdateAchievementProgression(eGA_PoundOfFlesh, self);
	}
}

/** Called by GSM_TargetMinigun so we know when the minigun is mounted and unmounted */
simulated function MinigunCoverMountedStateChanged( bool bMounted )
{
	bIsMountedWithMinigun = bMounted;
}

simulated function float RateScene()
{
	local GearPawn P;
	local GearProjectile Proj;
	local float Rating, PawnRating, Dist;
	local vector CamLoc;
	local rotator CamRot;
	GetPlayerViewpoint(CamLoc,CamRot);
	Rating = 0.f;
	foreach WorldInfo.AllPawns(class'GearPawn',P)
	{
		if (P.LastRenderTime == LastShotInfo.WorldTime && (vector(CamRot) dot Normal(P.Location - CamLoc) > 0.f) && VSize(P.Location - CamLoc) < 2048.f)
		{
			PawnRating = P.SceneRatingValue;
			if (P.bIsGore)
			{
				PawnRating *= 2.f;
			}
			`log("Initial PawnRating for:"@P@PawnRating);
			// more value for dead/dying
			if (P.Health <= 0)
			{
				PawnRating += 1.5f;
				if (TimeSince(P.TimeOfDeath) < 1.5f)
				{
					PawnRating += 1.f;
				}
				`log("- post-health:"@PawnRating);
			}
			// adjust for active special moves
			if (P.IsDoingASpecialMove())
			{
				PawnRating += P.SpecialMoveClasses[P.SpecialMove].default.SceneRatingValue;
				`log("- post-sm:"@PawnRating);
			}
			// adjust if firing
			if (P.LastWeaponStartFireTime > P.LastWeaponEndFireTime)
			{
				PawnRating += 0.5f;
				`log("- post-firing:"@PawnRating);
			}
			// adjust if blindfiring
			if (P.CoverAction == CA_BlindUp || P.CoverAction == CA_BlindLeft || P.CoverAction == CA_BlindRight)
			{
				PawnRating += 0.5f;
				`log("- post-cover:"@PawnRating);
			}
			// scale in favor of closer objects
			Dist = VSize(P.Location - CamLoc);
			if (Dist < 1024.f)
			{
				PawnRating += 5.f * (1.f - Dist/1024.f);
				`log("- post-distance:"@PawnRating);
				PawnRating += vector(CamRot) dot Normal(P.Location - CamLoc);
				`log("- post-dot:"@PawnRating);
			}
			`log("Final PawnRating for:"@P@PawnRating);
			Rating += PawnRating;
		}
	}
	// adjust for all projectiles flying
	foreach WorldInfo.AllActors(class'GearProjectile',Proj)
	{
		if (!Proj.bHidden && TimeSince(Proj.LastRenderTime) < 0.1f && vector(CamRot) dot Normal(Proj.Location - CamLoc) > 0.f)
		{
			Rating += RandRange(0.25f,0.5f);
			// if attached to a player then adjust
			if (Pawn(Proj.Base) != None)
			{
				Rating += 0.25f;
			}
			`log("Post project rating for:"@Proj@Rating);
		}
	}
	`log("Final rating:"@Rating);
	return Rating;
}

/** Gets the match type for the current match */
function eScreenshotMatchType GetMatchType()
{
	local OnlineGameSettings GameSettings;
	if(OnlineSub != none)
	{
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Game');
		if(GameSettings != none)
		{
			if(GameSettings.bUsesArbitration)
			{
				return SMT_Public;
			}
		}
	}
	return SMT_Private;
}

/** Gets the current "real world" time. */
native function GetRealtime(out int OutInt1, out int OutInt2);

/**
 * Ensures that ScreenshotManager is loaded
 **/
function bool LoadScreenshotManager()
{
	if ( ScreenshotManager == None )
	{
		ScreenshotManager = new(Self) class'GearScreenshotManager';
		if( !ScreenshotManager.Init())
		{
			`log("ScreenshotManager.Init failed");
			ScreenshotManager = None;
		}
	}
	return ScreenshotManager != None;
}

function CleanupScreenshotManager()
{
	if ( ScreenshotManager != None )
	{
		ScreenshotManager.Cleanup();
		ScreenshotManager = None;
	}
}

function ForceCloseSavingScreenshotMessage()
{
	local GameUISceneClient GameSceneClient;
	local GearUISceneFE_Updating ExistingScene;

	GameSceneClient = class'UIRoot'.static.GetSceneClient();
	if (GameSceneClient != None)
	{
		ExistingScene = GearUISceneFE_Updating(GameSceneClient.FindSceneByTag('SavingScreenshot'));
		if ( ExistingScene != None )
		{
			ExistingScene.StopSceneAnimation(ExistingScene.SceneAnimation_Close, true);
		}
	}
}

/**
 * Show (or hide) the message that indicates a screenshot is being saved.
 * This is almost entirely copied from GearUISceneWJ_ScreenshotManagement.ShowUpdatingScene().
 */
function ShowSavingScreenshotMessage(optional bool bShow = true, optional bool bFullMessage)
{
	local GearUISceneFE_Updating UpdatingScene;
	local UIPanel Panel;
	local string TitleString;
	local string MessageString;

	if (!bFullMessage)
	{
		TitleString = "SavingContent_ShortMessage";
		MessageString = "SavingContent_Message";
	}
	else
	{
		TitleString = "";
		MessageString = "StorageDeviceFull_Title";
		if (bShow)
		{
			SetTimer(1.5f,FALSE,nameof(CloseSavingMessage));
		}
	}

	if ( bShow )
	{
		// this is needed to make sure any previous saving message is closed
		ForceCloseSavingScreenshotMessage();

		UpdatingScene = class'GearGameUISceneClient'.static.OpenUpdatingScene(TitleString, MessageString,0.0,'SavingScreenshot');
		if ( UpdatingScene != None )
		{
			Panel = UIPanel(UpdatingScene.FindChild('pnlCheckpoint',true));
			Panel.SetVisibility(false);

			Panel = UIPanel(UpdatingScene.FindChild('pnlUpdating', true));
			Panel.SetVisibility(true);
		}

		if (!bFullMessage)
		{
			// Alert the user that a screenshot has been taken.
			AlertManager.Alert(eALERT_Screenshot);
		}
	}
	else
	{
		class'GearGameUISceneClient'.static.CloseUpdatingScene('SavingScreenshot');
	}
}

function CloseSavingMessage()
{
	ShowSavingScreenshotMessage(FALSE,TRUE);
}

/** Delegate called when a screenshot finishes saving. */
function SaveScreenshotComplete(bool bWasSuccessful)
{
	bSavingShot = false;
	`log("SaveScreenshotComplete: "$bWasSuccessful);

	if ( ScreenshotManager != None )
	{
		ScreenshotManager.ClearSaveScreenshotCompleteDelegate(SaveScreenshotComplete);
	}
	DiscardScreenshot();
	ShowSavingScreenshotMessage(false);
	ForceButtonRelease(GB_B,TRUE);
}

/**
 * Saves a screenshot to disk.
 * SaveScreenshotComplete will be called when the attempt finishes.
 *
 * @param Shot The compressed JPG to be saved to disk.
 *
 * @return True if the attempt is started successfully.
 */
function bool SaveScreenshot(const out array<byte> Shot, const out array<byte> Thumbnail)
{
	local bool bWasSuccessful;

	if(bSavingShot)
	{
		`log("SaveScreenshot: already saving shot");
		return false;
	}

	if ( LoadScreenshotManager() )
	{
		ScreenshotManager.AddSaveScreenshotCompleteDelegate(SaveScreenshotComplete);
		bWasSuccessful = ScreenshotManager.SaveScreenshot(Shot, Thumbnail, LastShotInfo);
		if(bWasSuccessful)
		{
			`log("ScreenshotManager.SaveScreenshot completed");
			bSavingShot = true;
		}
		else
		{
			`log("ScreenshotManager.SaveScreenshot failed");
			ScreenshotManager.ClearSaveScreenshotCompleteDelegate(SaveScreenshotComplete);
			ShowSavingScreenshotMessage(false);
		}
	}
	else
	{
		`log(`location @ "has no screenshot manager");
	}
	return bWasSuccessful;
}

/**
 * This event is triggered when async screenshot compression completes.
 */
event ScreenshotCompressed(const out array<byte> CompressedShot, const out array<byte> Thumbnail)
{
	local bool bSaveSuccessful;

	`log("Received ScreenshotCompressed event");
	if(CompressedShot.length != 0)
	{
	bSaveSuccessful = SaveScreenshot(CompressedShot, Thumbnail);

	`log(`location @ "Saved screenshot -" @ `showvar(bSaveSuccessful));

	// fool the compiler >
	bSaveSuccessful = bSaveSuccessful;
	// fool the compiler <
	}
	else
	{
		`log(`location @ "CompressedShot is empty");
		ShowSavingScreenshotMessage(false);
	}

	bCompressingShot = false;
	DiscardScreenshot();
}

/** Native code that starts the async compression task for the last screenshot taken. */
native function bool CompressLastScreenshot();

/** Starts the process of compressing the last screenshot taken. */
function CompressScreenshot()
{
	if( !bCompressingShot )
	{
		`log("Compressing screenshot");
		bCompressingShot = CompressLastScreenshot();
		if(!bCompressingShot)
		{
			`log("CompressLastScreenshot() failed");
			DiscardScreenshot();
			ShowSavingScreenshotMessage(false);
		}
	}
	else
	{
		`log("Trying to compress when already compressing");
	}
}

/** Screenshot cleanup. */
exec function DiscardScreenshot()
{
	if ( IsLocalPlayerController() )
	{
		// if we're doing something async, don't discard until done
		`log(`location @ `showvar(bCompressingShot) @ `showvar(bSavingShot));
		if (!bCompressingShot && !bSavingShot)
		{
			if (LastCapturedShot != None)
			{
				// let GC clean this up
				//LastCapturedShot = None;
				//WorldInfo.ForceGarbageCollection();
			}
			bRequestingShot = FALSE;
		}
	}
}

/** Show the current state of the screenshot system */
exec function ScreenshotState()
{
	`log("ScreenshotState: Requesting  = "$bRequestingShot);
	`log("ScreenshotState: Compressing = "$bCompressingShot);
	`log("ScreenshotState: Saving      = "$bSavingShot);
}

/** Returns true if the screenshot system is currently doing anything */
function bool IsScreenshotActive()
{
	return (bRequestingShot || bCompressingShot || bSavingShot);
}

/** Determines if a pawn is visible in the current screenshot */
function bool IsVisibleInScreenshot(const Pawn P)
{
	local vector CamLoc;
	local rotator CamRot;
	local bool Result;
	// dead players don't count
	if(P.Health <= 0)
	{
		return false;
	}
	GetPlayerViewpoint(CamLoc,CamRot);
	// check if the pawn was rendered in the shot, is in front of the camera, and is in the line of sight
	Result = (P.LastRenderTime == LastShotInfo.WorldTime && (vector(CamRot) dot Normal(P.Location - CamLoc) > 0.f) && LineOfSightTo(P, CamLoc));
	//`log("IsVisibleInScreenshot:" @ Result @ "LastRenderTime:" @ P.LastRenderTime @ "LastShotWorldTime:" @ LastShotInfo.WorldTime @ "DotProd:" @ (vector(CamRot) dot Normal(P.Location - CamLoc)));
	return Result;
}

/** Gets an array of corners for a bounding box. */
function GetBoxCorners(box BBox, out array<vector> Corners)
{
	Corners.length = 8;
	Corners[0].X = BBox.Min.X; Corners[0].Y = BBox.Min.Y; Corners[0].Z = BBox.Min.Z;
	Corners[1].X = BBox.Max.X; Corners[1].Y = BBox.Min.Y; Corners[1].Z = BBox.Min.Z;
	Corners[2].X = BBox.Min.X; Corners[2].Y = BBox.Max.Y; Corners[2].Z = BBox.Min.Z;
	Corners[3].X = BBox.Min.X; Corners[3].Y = BBox.Min.Y; Corners[3].Z = BBox.Max.Z;
	Corners[4].X = BBox.Max.X; Corners[4].Y = BBox.Max.Y; Corners[4].Z = BBox.Min.Z;
	Corners[5].X = BBox.Min.X; Corners[5].Y = BBox.Max.Y; Corners[5].Z = BBox.Max.Z;
	Corners[6].X = BBox.Max.X; Corners[6].Y = BBox.Min.Y; Corners[6].Z = BBox.Max.Z;
	Corners[7].X = BBox.Max.X; Corners[7].Y = BBox.Max.Y; Corners[7].Z = BBox.Max.Z;
}

/** Projects a bounding box from world-space coordinates to screen-space coordinates. */
function bool WorldBoxToScreenBox(Canvas Canvas, vector CamLoc, rotator CamRot, box WorldBox, out box ScreenBox)
{
	local array<vector> WorldCorners;
	local array<vector> Corners;
	local int Index;
	GetBoxCorners(WorldBox, WorldCorners);
	for(Index = 0 ; Index < WorldCorners.length ; Index++)
	{
		// make sure this corner is in front of the camera
		if(vector(CamRot) dot Normal(WorldCorners[Index] - CamLoc) > 0.f)
		{
			Corners.length = Corners.length + 1;
			Corners[Corners.length - 1] = Canvas.Project(WorldCorners[Index]);
		}
	}
	if(Corners.length == 0)
	{
		// all corners were behind the camera
		return false;
	}
	//`log("WorldBoxToScreenBox:" @ Corners.length @ "corners in front of the camera");
	ScreenBox.Min = Corners[0];
	ScreenBox.Max = Corners[0];
	for(Index = 1 ; Index < Corners.length ; Index++)
	{
		ScreenBox.Min.X = FMin(ScreenBox.Min.X, Corners[Index].X);
		ScreenBox.Max.X = FMax(ScreenBox.Max.X, Corners[Index].X);
		ScreenBox.Min.Y = FMin(ScreenBox.Min.Y, Corners[Index].Y);
		ScreenBox.Max.Y = FMax(ScreenBox.Max.Y, Corners[Index].Y);
	}
	ScreenBox.Min.Z = 0;
	ScreenBox.Max.Z = 0;
	return true;
}

/** Clips a screenspace bounding box to the screen. */
function bool ClipBoxToScreen(out box BBox, float ClipX, float ClipY)
{
	// check if the box is entirely off the screen
	if((BBox.Max.X < 0) || (BBox.Min.X >= ClipX) || (BBox.Max.Y < 0) || (BBox.Min.Y >= ClipY))
	{
		return false;
	}

	// check for clipping
	BBox.Min.X = FMax(BBox.Min.X, 0);
	BBox.Max.X = FMin(BBox.Max.X, ClipX - 1);
	BBox.Min.Y = FMax(BBox.Min.Y, 0);
	BBox.Max.Y = FMin(BBox.Max.Y, ClipY - 1);

	return true;
}

/** Converts a player's bounding box to a screenspace bounding box. */
function bool PlayerBoxToScreenBox(Canvas Canvas, vector CamLoc, rotator CamRot, box PlayerBox, out box ScreenBox)
{
	if(WorldBoxToScreenBox(Canvas, CamLoc, CamRot, PlayerBox, ScreenBox))
	{
		//`log("PlayerBoxToScreenBox(screen): bboxmin:" @ ScreenBox.Min @ "bboxmax:" @ ScreenBox.Max @ "dimensions:" @ (ScreenBox.Max - ScreenBox.Min));
		if(ClipBoxToScreen(ScreenBox, Canvas.ClipX, Canvas.ClipY))
		{
			return true;
		}
	}
	return false;
}

/** Gets a player's bounding box. */
function GetPlayerBox(const Pawn P, out box PlayerBox)
{
	PlayerBox.Min = P.Mesh.Bounds.Origin - P.Mesh.Bounds.BoxExtent;
	PlayerBox.Max = P.Mesh.Bounds.Origin + P.Mesh.Bounds.BoxExtent;
	//`log("GetPlayerBox(world,mesh-bounds): bboxmin:" @ PlayerBox.Min @ "bboxmax:" @ PlayerBox.Max @ "dimensions:" @ (PlayerBox.Max - PlayerBox.Min));
	//`log("GetPlayerBox(health):" @ P.Health);
}

/** Gets a Pawn from a PRI. */
function bool GetPRIPawn(const PlayerReplicationInfo PRI, out Pawn P)
{
	ForEach DynamicActors(class'Pawn', P)
	{
		if ( P.PlayerReplicationInfo == PRI )
		{
			return true;
		}
	}
	return false;
}

/** Gets info for players (including bots) in the current screenshot. */
function AddPlayersToScreenshot(Canvas Canvas, out array<ScreenshotPlayerInfo> Players)
{
	local int Index;
	local PlayerReplicationInfo PRI;
	local int PlayerIndex;
	local box PlayerBox;
	local box ScreenBox;
	local bool Result;
	local vector CamLoc;
	local rotator CamRot;
	local Pawn PlayerPawn;
	local UniqueNetId BotXuid;
	GetPlayerViewpoint(CamLoc,CamRot);
	Players.length = 0;
	//`log("Camera Location" @ CamLoc);
	for (Index = 0; Index < WorldInfo.GRI.PRIArray.Length; Index++)
	{
		PRI = WorldInfo.GRI.PRIArray[Index];
		if (!PRI.bFromPreviousLevel)
		{
			if(GetPRIPawn(PRI, PlayerPawn))
			{
				PlayerIndex = Players.length;
				Players.length = Players.length + 1;
				if(!PRI.bBot)
				{
					Players[PlayerIndex].Xuid = PRI.UniqueId;
					Players[PlayerIndex].Nick = "";
				}
				else
				{
					Players[PlayerIndex].Xuid = BotXuid;
					Players[PlayerIndex].Nick = PRI.PlayerName;
				}
				Players[PlayerIndex].IsVisible = false;
				Players[PlayerIndex].Location = PlayerPawn.Location;
				if(IsVisibleInScreenshot(PlayerPawn))
				{
					GetPlayerBox(PlayerPawn, PlayerBox);
					Result = PlayerBoxToScreenBox(Canvas, CamLoc, CamRot, PlayerBox, ScreenBox);
					if(Result)
					{
						Players[PlayerIndex].ScreenSpaceBoundingBox = ScreenBox;
						Players[PlayerIndex].IsVisible = true;
					}
				}
			}
		}
	}
}

/** Gets the friendly (localized) map name */
function string GetFriendlyMapName(string MapName)
{
	local GearGameMapSummary MapData;
	MapData = class'GearUIDataStore_GameResource'.static.GetMapSummaryFromMapName( MapName );
	if ( MapData != None )
	{
		return MapData.DisplayName;
	}
	return MapName;
}

function string GetFriendlyGameType(EGearMPTypes GameTypeId, string GameTypeString)
{
	local GearGameInfoSummary GameType;
	GameType = class'GearUIDataStore_GameResource'.static.GetGameTypeProvider( GameTypeId );
	if ( GameType != None )
	{
		return GameType.GameName;
	}
	return GameTypeString;
}

/** This event is called after the rendering system captured a screenshot */
event TookScreenshot(Canvas Canvas)
{
	`log(`location);
	bRequestingShot = FALSE;
	if ((Canvas != None) && (LastCapturedShot != None))
	if (LastCapturedShot != None)
	{
		LastShotInfo.Description = "";
		LastShotInfo.Rating = int(RateScene()) * 100 + Rand(500);
		LastShotInfo.GameType = GetCurrGameType();
		LastShotInfo.GameTypeId = int(class'GearUIScene_Base'.static.GetMPGameTypeStatic(GearGRI(WorldInfo.GRI)));
		LastShotInfo.GameTypeFriendly = GetFriendlyGameType(EGearMPTypes(LastShotInfo.GameTypeId), LastShotInfo.GameType);
		LastShotInfo.MatchType = GetMatchType();
		LastShotInfo.MapName = GetCurrMapName();
		// hack, copied from GearUIScene_Base.GetMapNameString()
		if (LastShotInfo.MapName == "MP_Training")
		{
			LastShotInfo.MapName = "MP_DayOne";
		}
		LastShotInfo.MapNameFriendly = GetFriendlyMapName(LastShotInfo.MapName);
		GetRealtime(LastShotInfo.Realtime.A, LastShotInfo.Realtime.B);
		LastShotInfo.MatchId = GearGRI(WorldInfo.GRI).StatsGameplaySessionID;
		GetPlayerViewpoint(LastShotInfo.CamLoc,LastShotInfo.CamRot);
		LastShotInfo.Width = LastCapturedShot.SizeX;
		LastShotInfo.Height = LastCapturedShot.SizeY;
		AddPlayersToScreenshot(Canvas, LastShotInfo.Players);
		CompressScreenshot();
	}
	else
	{
		`log("no last captured shot or canvas" @ `showvar(LastCapturedShot) @ `showvar(Canvas));
		ShowSavingScreenshotMessage(false);
	}
}

/** Returns true if this is a platform that supports screenshots */
final native function bool IsScreenshotPlatform();

/** Determines if the player can take a screenshot. */
final function bool AllowedToTakeScreenshots()
{
	local byte PlayerID;
	local bool bIsLoggedIn;
	PlayerID = LocalPlayer(Player).ControllerId;
	bIsLoggedIn = class'UIInteraction'.static.IsLoggedIn(PlayerID);
	return (bIsLoggedIn && !IsSplitscreenPlayer() && IsLocalPlayerController() && IsSpectating() && !IsScreenshotActive()); // && OnlineCommunityContentInterface(OnlineSub.GetNamedInterface('CommunityContent')) != None);
}

/**
 * Starts the processing of taking a screenshot.
 * After the shot is taken, it will be compressed and then saved to disk automatically.
 */
exec function TakeScreenshot()
{
	local GearEngine Engine;
	local vector2d ViewportSize;
	if(IsScreenshotPlatform())
	{
		if (AllowedToTakeScreenshots())
		{
			Engine = GearEngine(Player.Outer);
			if(Engine.IsCurrentDeviceValid(500 * 1024))
			{
				`log(`location);
				if (LastCapturedShot == None)
				{
					LocalPlayer(Player).ViewportClient.GetViewportSize(ViewportSize);
					LastCapturedShot = class'Texture2DDynamic'.static.Create(ViewportSize.X,ViewportSize.Y,PF_A8R8G8B8,TRUE);
				}
				bRequestingShot = TRUE;
				ShowSavingScreenshotMessage();
			}
		}
	}
}

/** This will tell the GearsStatsObject to dump all stats to the DB **/
exec simulated function DumpGameEventsToDB()
{
	if( GearGame(WorldInfo.Game).StatsObject != None )
	{
		`log( "Sending all Game Events to the DB!" );
		GearGame(WorldInfo.Game).StatsObject.SendGameEventsToDB();
	}
}

/**
 * Sets the player's skill with the value the host has and saves the profile so it persists
 *
 * @param PlayerSkill the new skill to store for the player
 */
reliable client function ClientSetPlayerSkill(int PlayerSkill)
{
	if (ProfileSettings != None)
	{
		ProfileSettings.SetProfileSettingValueInt(ProfileSettings.const.PlayerSkill,PlayerSkill);
		SaveProfile();
	}
}

/**
 * Sends the player's persistent skill to the host to be replicated to everyone
 *
 * @param PlayerSkill the new skill to replicate to everyone
 */
reliable server function ServerSetPlayerSkill(int PlayerSkill)
{
	PlayerReplicationInfo.PlayerSkill = PlayerSkill;
}

/**
 * Tells the client that the server needs the player's persistent skill information
 */
reliable client function ClientGetPlayerSkill()
{
	local int PlayerSkill;

	if (ProfileSettings != None)
	{
		ProfileSettings.GetProfileSettingValueInt(ProfileSettings.const.PlayerSkill,PlayerSkill);
		ServerSetPlayerSkill(PlayerSkill);
	}
	else
	{
		// No profile means no skill
		ServerSetPlayerSkill(0);
	}
}

/** Sends a server RPC for each local player so that the players can communicate what DLC that have */
function UpdateLocalPlayersDLCValues()
{
	local GearPC PC;
	local GearUIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> MapProviders;
	local GearGameMapSummary MapData;
	local int ProvIdx, DLCFlagResult;

	// First build the DLCFlag for this machine
	DLCFlagResult = 0;
	GameResourceDS = GearUIDataStore_GameResource(class'UIRoot'.static.StaticResolveDataStore(class'GearUIDataStore_GameResource'.default.Tag));
	if ( GameResourceDS != None )
	{
		if ( GameResourceDS.GetResourceProviders('Maps', MapProviders) )
		{
			for ( ProvIdx = 0; ProvIdx < MapProviders.length; ProvIdx++ )
			{
				MapData = GearGameMapSummary(MapProviders[ProvIdx]);
				if ( MapData != None && MapData.DLCId > 0 )
				{
					DLCFlagResult = DLCFlagResult | (1 << MapData.DLCId);
				}
			}
		}
	}

	// Set the DLCFlag on all local controller PRIs via server RPC if the value is different
	foreach LocalPlayerControllers(class'GearPC', PC)
	{
		if (DLCFlagResult != GearPRI(PC.PlayerReplicationInfo).DLCFlag)
		{
			PC.ServerSetDLCFlag(DLCFlagResult);
		}
	}
}

/** Sets the DLC flag value of the player on the PRI on the server */
reliable server function ServerSetDLCFlag( int DLCFlag )
{
	GearPRI(PlayerReplicationInfo).DLCFlag = DLCFlag;
}

/** Requests a team change from the server */
unreliable server function ServerRequestChangeTeam( int NumPlayers )
{
	local GearPreGameGRI PreGameGRI;
	local GearPreGameLobbyPRI PreGamePRI;

	PreGameGRI = GearPreGameGRI(WorldInfo.GRI);
	PreGamePRI = GearPreGameLobbyPRI(PlayerReplicationInfo);
	if ( PreGameGRI != None && PreGamePRI != None )
	{
		PreGameGRI.RequestChangeTeam( PreGamePRI, NumPlayers );
	}
}

/** Sets the difficulty of this player in the campaign lobby */
reliable server function ServerSetCampaignLobbyDifficulty( EDifficultyLevel Diff )
{
	local GearCampaignLobbyPRI PRI;
	PRI = GearCampaignLobbyPRI(PlayerReplicationInfo);
	if (PRI != None)
	{
		PRI.CampDifficulty = Diff;
	}
}

/** Called by the server to ask the client to set their default difficulty */
reliable client function ClientGetCampaignLobbyDifficulty()
{
	local int Value;

	ProfileSettings.GetProfileSettingValueId(ProfileSettings.GameIntensity,Value);
	ServerSetCampaignLobbyDifficulty(EDifficultyLevel(Value));
}

/**
 * Turns a special action icon on and off
 */
function OnToggleButtonMash( SeqAct_ToggleButtonMash inAction )
{
	// Start
	if( inAction.InputLinks[0].bHasImpulse )
	{
		ClientToggleKismetIcon( true, inAction.IconToDisplay, inAction.AnimationSpeed );
	}
	// Stop
	else
	{
		ClientToggleKismetIcon( false, inAction.IconToDisplay, inAction.AnimationSpeed );
	}
}

/** Client function that turns a special action icon on induced by kismet */
reliable client function ClientToggleKismetIcon( bool bTurnOn, eGearKismetIconType IconType, float AnimationSpeed )
{
	if ( bTurnOn )
	{
		// Wipe any old icons
		ActionKismetButton.ActionIconDatas.length = 0;
		ActionKismetButton.IconAnimationSpeed = 0;

		// Make a new icon data
		ActionKismetButton.ActionIconDatas.length = 1;
		switch ( IconType )
		{
		case eGEARBUTTON_A:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 1;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_A];
			break;
		case eGEARBUTTON_A_MASH:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 2;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_A];
			ActionKismetButton.ActionIconDatas[0].ActionIcons[1] = GearIcons[eGEARICON_A_PRESSED];
			ActionKismetButton.IconAnimationSpeed = AnimationSpeed;
			break;
		case eGEARBUTTON_B:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 1;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_B];
			break;
		case eGEARBUTTON_B_MASH:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 2;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_B];
			ActionKismetButton.ActionIconDatas[0].ActionIcons[1] = GearIcons[eGEARICON_B_PRESSED];
			ActionKismetButton.IconAnimationSpeed = AnimationSpeed;
			break;
		case eGEARBUTTON_X:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 1;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_X];
			break;
		case eGEARBUTTON_Y:
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 1;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_Y];
			break;
		case eGEARBUTTON_ChainsawMash:
			ActionKismetButton.ActionIconDatas.length = 2;
			ActionKismetButton.ActionIconDatas[0].ActionIcons.length = 2;
			ActionKismetButton.ActionIconDatas[0].ActionIcons[0] = GearIcons[eGEARICON_B];
			ActionKismetButton.ActionIconDatas[0].ActionIcons[1] = GearIcons[eGEARICON_B_PRESSED];
			if (GearWeap_AssaultRifle(MyGearPawn.Weapon) != None)
			{
				ActionKismetButton.ActionIconDatas[1].ActionIcons.length = 2;
				ActionKismetButton.ActionIconDatas[1].ActionIcons[0] = GearIcons[eGEARICON_Chainsaw];
				ActionKismetButton.ActionIconDatas[1].ActionIcons[1] = GearIcons[eGEARICON_Chainsaw_PRESSED];
			}
			else
			{
				ActionKismetButton.ActionIconDatas.length = 1;
			}
			ActionKismetButton.IconAnimationSpeed = AnimationSpeed;
			break;
		}

		MyGearHUD.SetActionInfo( AT_KismetButton, ActionKismetButton );
	}
	else
	{
		MyGearHUD.ClearActionInfoByType( AT_KismetButton );
	}
}

/**
 * This will test the mature language features
 * Allow:  "Fuck you fucking fuck!"
 * Disallow:  "Stack em high!"
 **/
exec function TestMatureLanguage()
{
	local AudioComponent AC;

	AC = CreateAudioComponent( SoundCue'Test_SoundNodes.MatureLanguage', FALSE, TRUE );
	if( AC != None )
	{
		AC.bAutoDestroy = TRUE;
		AC.SubtitlePriority = 10000;
		AC.bSuppressSubtitles = FALSE;
		AC.FadeIn( 0.0f, 1.f);
	}

}

/** This is for the QA team who don't use UFE nor commandline :-( **/
exec simulated function BeginBVT( optional coerce string TagDesc )
{
	local GearPC PC;

	WorldInfo.Game.BeginSentinelRun( "BVT", "", TagDesc );

	// for now we are just hard coding what we want to gather
	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		//PC.ConsoleCommand( "stat " );
	}

	WorldInfo.Game.SetTimer( 3.0f, TRUE, nameof(WorldInfo.Game.DoTimeBasedSentinelStatGathering) );
}


/** This will toggle Muzzle flashes on/off all weapons **/
exec function ToggleMuzzleFlashes()
{
	local GearWeapon GW;

	foreach AllActors( class'GearWeapon', GW )
	{
		GW.bSuppressMuzzleFlash = !GW.bSuppressMuzzleFlash;
	}
}

/** This will toggle tracers on/off all weapons **/
exec function ToggleTracers()
{
	local GearWeapon GW;

	foreach AllActors( class'GearWeapon', GW )
	{
		GW.bSuppressTracers = !GW.bSuppressTracers;
	}
}

/** This will toggle impact effects on/off all weapons **/
exec function ToggleImpactEffects()
{
	local GearWeapon GW;

	foreach AllActors( class'GearWeapon', GW )
	{
		GW.bSuppressImpactFX = !GW.bSuppressImpactFX;
	}
}

/** This will toggle Weapon audio on/off all weapons **/
exec function ToggleWeaponAudio()
{
	local GearWeapon GW;

	foreach AllActors( class'GearWeapon', GW )
	{
		GW.bSuppressAudio = !GW.bSuppressAudio;
	}
}


/** This will toggle bDropDetail_Test **/
exec function ToggleDropDetail()
{
	GearGRI(WorldInfo.GRI).bDropDetail_Test = !GearGRI(WorldInfo.GRI).bDropDetail_Test;
	`log( "bDropDetail_Test " $ GearGRI(WorldInfo.GRI).bDropDetail_Test );
}


/** This will toggle bAggressiveLOD_Test **/
exec function ToggleAggressiveLOD()
{
	GearGRI(WorldInfo.GRI).bAggressiveLOD_Test = !GearGRI(WorldInfo.GRI).bAggressiveLOD_Test;
	`log( "bAggressiveLOD_Test " $ GearGRI(WorldInfo.GRI).bAggressiveLOD_Test );
}


/**
 * This is a function which is called when sentinel is able to start TravelTheWorld.  This allows the specific game
 * to do things such as turning off UI/HUD and to not enter some default starting the game state.
 **/
function Sentinel_SetupForGamebasedTravelTheWorld()
{
	MyGearHud.bShowHUD = FALSE;
	GearGame(WorldInfo.Game).bAllowTutorials = FALSE;
	StopTutorialSystem( TRUE );
	MyGearHud.ToggleScoreboard( FALSE );
	GearHUDMP_Base(MyGearHud).CloseScoreboardScene();
	// need to do this here to escape the clutches of spectator cameras
	GotoState( 'BaseSpectating' );
	SetViewTarget( self );
}

/**
 * This function is called before we acquire the travel points.
 **/
function Sentinel_PreAcquireTravelTheWorldPoints()
{
	Kill( "all" ); // kills off AI's spawned on level loaded and visible
}

/**
 * This function is called after we acquire the travel points right before we start traveling.
 **/
function Sentinel_PostAcquireTravelTheWorldPoints()
{
	Kill( "all" ); // kills off AI's spawned on level loaded and visible
	// need to do this here to escape the clutches of spectator cameras
	GotoState( 'BaseSpectating' );
	SetViewTarget( self );
}


/** This is used to play the GoreTest anim on the pawn we are facing.  So the animators can test out the gore skeleton **/
exec function GoreTest()
{
	local GearPawn GP;
	local GearPawn.BodyStance GoreTestStance;
	local int i;

	GoreTestStance.AnimName[BS_FullBody] = 'GoreTest';

	// look at all pawns and see if we should try to play a GoreTest anim on them
	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		if( !GP.IsHumanControlled()
			&& IsAimingAt( GP, 0.1f ) // if we are semi looking in the direction of the pawn
			)
		{
			GP.bUsingNewSoftWeightedGoreWhichHasStretchiesFixed = TRUE;
			GP.CreateGoreSkeleton( GP.GoreSkeletalMesh, GP.GorePhysicsAsset );

			if( GP.GoreBreakableJointsTest.Length == 0 )
			{
				GP.GoreBreakableJointsTest = GP.GoreBreakableJoints;
			}

			for( i = 0; i < GP.GoreBreakableJointsTest.Length; ++i )
			{
				`log("  Breaking Joint:" @ GP.GoreBreakableJointsTest[i] );
				GP.BreakConstraint( vect(0,0,0), vect(0,0,0), GP.GoreBreakableJointsTest[i] );
			}

			`log( "Attempting to play GoreTest on: " $ GP );
			GP.BS_Play( GoreTestStance, 1.0f, 0.2f/1.0f, 0.2f/1.0f, FALSE, TRUE );
		}
	}
}


/**
 *
 * Usage:  GoreTestJointList (foo,bar,baz)
 **/
exec function GoreTestJointList( array<name> JointList )
{
	local GearPawn GP;
	local GearPawn.BodyStance GoreTestStance;
	local int i;

	GoreTestStance.AnimName[BS_FullBody] = 'GoreTest';

	// look at all pawns and see if we should try to play a GoreTest anim on them
	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		if( !GP.IsHumanControlled()
			&& IsAimingAt( GP, 0.1f ) // if we are semi looking in the direction of the pawn
			)
		{
			GP.bUsingNewSoftWeightedGoreWhichHasStretchiesFixed = TRUE;
			GP.CreateGoreSkeleton( GP.GoreSkeletalMesh, GP.GorePhysicsAsset );

			for( i = 0; i < JointList.Length; ++i )
			{
				`log("  Breaking Joint:" @ JointList[i] );
				GP.BreakConstraint( vect(0,0,0), vect(0,0,0), JointList[i] );
			}

			`log( "Attempting to play GoreTestJointList on: " $ GP );
			GP.BS_Play( GoreTestStance, 1.0f, 0.2f/1.0f, 0.2f/1.0f, FALSE, TRUE );
		}
	}
}

/**
 *
 * Usage:  GoreTestRagdoll (foo,bar,baz) / GoreTestRagdoll ()
 **/
exec function GoreTestRagdoll( array<name> JointList )
{
	local GearPawn GP;

	// look at all pawns and see if we should try to play a GoreTest anim on them
	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		if( !GP.IsHumanControlled()
			&& IsAimingAt( GP, 0.1f ) // if we are semi looking in the direction of the pawn
			)
		{
			GP.bUsingNewSoftWeightedGoreWhichHasStretchiesFixed = TRUE;
			if( JointList.Length > 0 )
			{
				GP.GoreBreakableJointsTest = JointList;
				GP.GoreBreakableJoints = JointList;
			}

			GP.CreateGoreSkeleton( GP.GoreSkeletalMesh, GP.GorePhysicsAsset );
			GP.TakeDamage( 10000.0f, self, GP.Location, vect(0,0,0), class'GDT_Explosive' );
		}
	}
}



// client cheat commands for debugging
`if(`notdefined(FINAL_RELEASE))
exec function Cheat(string Cmd)
{
	ServerCheat(Cmd);
}

reliable server final function ServerCheat(string Cmd)
{
	ClientMessage(ConsoleCommand(Cmd));
}
`endif

/**
 * Called when the join for the travel destination has completed
 *
 * @param SessionName the name of the session the event is for
 * @param bWasSuccessful whether it worked or not
 */
function OnJoinTravelToSessionComplete(name SessionName,bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		bIgnoreNetworkMessages = true;
	}

	Super.OnJoinTravelToSessionComplete(SessionName,bWasSuccessful);
}

reliable client function ClientTravelToSession(name SessionName,class<OnlineGameSearch> SearchClass,byte PlatformSpecificInfo[68])
{
	bIgnoreNetworkMessages = true;
	Super.ClientTravelToSession(SessionName,SearchClass,PlatformSpecificInfo);
}

/**
 * loads/unloads GUD banks and updates ClientGUDSReferences to keep a reference to them
 * Client only, should not be called on server, even if that server is also a client
 */
reliable client native event ClientLoadGUDBank(name BankName);
reliable client native event ClientUnloadGUDBank(name BankName);

/**
 * Common method for setting the rich presence string
 *
 * @param StringId the rich presence string to set
 */
simulated function SetRichPresenceString(int StringId)
{
	local LocalPlayer LP;
	local array<LocalizedStringSetting> StringSettings;
	local array<SettingsProperty> Properties;

	LP = LocalPlayer(Player);
	if (LP != None &&
		OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		OnlineSub.PlayerInterface.SetOnlineStatus(LP.ControllerId,StringId,StringSettings,Properties);
	}
}

/**
 * Common method for the host to use to set the rich presence string
 *
 * @param StringId the rich presence string to set
 */
reliable client function ClientSetRichPresenceString(int StringId)
{
	SetRichPresenceString(StringId);
}

/**
 * Adds the meatflag to the recent players so that he shows up in the post game lobby
 *
 * @param OnlineStatsWriteClass the stats class to write with
 * @param MeatflagId the unique id to associate with the meatflag
 */
reliable client function ClientAddMeatflagToRecentPlayers(class<OnlineStatsWrite> OnlineStatsWriteClass,UniqueNetId MeatflagId)
{
	local int PriIndex;
	local GearRecentPlayersList PlayersList;
	local GearPRI PRI;
	local UniqueNetId ZeroId;

	if (IsPrimaryPlayer())
	{
		// Get the recent player data for the last match stats
		PlayersList = GearRecentPlayersList(OnlineSub.GetNamedInterface('RecentPlayersList'));
		if (PlayersList != None)
		{
			// Iterate through the active PRI list searching for the meatflag (team 255)
			for (PriIndex = 0; PriIndex < WorldInfo.GRI.PRIArray.Length; PriIndex++)
			{
				PRI = GearPRI(WorldInfo.GRI.PRIArray[PriIndex]);
				if (PRI != None && PRI.GetTeamNum() == 255)
				{
					// Change his xuid and team for purposes of adding to recent players
					PRI.UniqueId = MeatflagId;
					// Add to the recent players for stats
					PlayersList.AddPlayer(PRI);
					PlayersList.UpdatePlayer(PRI);
					// Reset the PRI data
					PRI.UniqueId = ZeroId;
				}
			}
		}
	}
}

/** forces the client to keep Marcus/Dom's textures in memory for the duration of the game because we are playing SP */
reliable client function ClientForceStreamSPTextures()
{
	class'GearPawn_COGMarcus'.default.Mesh.PrestreamTextures(float(MaxInt / 2), false);
	class'GearPawn_COGDom'.default.Mesh.PrestreamTextures(float(MaxInt / 2), false);
}

/** forces the client to preload textures for the specified pawn class */
reliable client function ClientPrestreamTexturesFor(class<Pawn> PawnClass)
{
	if (PawnClass != None)
	{
		PawnClass.default.Mesh.PrestreamTextures(5.0, false);
	}
}

/**
 * Tells the client to iterate through its teammates skins and set the force
 * streaming status flag.
 *
 * @param Seconds	Number of seconds to force all mips to stream in
 */
reliable client function ClientPrestreamTeamSkins(optional float Seconds)
{
	// Pre-stream the textures for the startup weapons.
	class'GearWeap_Shotgun'.default.Mesh.PrestreamTextures( Seconds, true );
	class'GearWeap_AssaultRifle'.default.Mesh.PrestreamTextures( Seconds, true );
	class'GearWeap_LocustAssaultRifle'.default.Mesh.PrestreamTextures( Seconds, true );
	class'GearWeap_SmokeGrenade'.default.Mesh.PrestreamTextures( Seconds, true );
	class'GearWeap_COGPistol'.default.Mesh.PrestreamTextures( Seconds, true );
}

/**
 * Sets the invite flags for the party so that clients can match what the host has set
 *
 * @param bAllowInvites whether invites are allowed or not
 * @param bAllowJoinInProgress whether join in progress is allowed or not
 * @param bAllowJoinInProgressFriendsOnly whether friends only join in progress is allowed or not
 */
reliable client function ClientSetPartyInviteFlags(bool bAllowInvites,bool bAllowJoinInProgress,bool bAllowJoinInProgressFriendsOnly)
{
	local OnlineGameSettings PartySettings;

	if (OnlineSub != None &&
		OnlineSub.GameInterface != None &&
		IsPrimaryPlayer())
	{
		PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
		if (PartySettings != None)
		{
			PartySettings.bAllowInvites = bAllowInvites;
			PartySettings.bAllowJoinViaPresence = bAllowJoinInProgress;
			PartySettings.bAllowJoinViaPresenceFriendsOnly = bAllowJoinInProgressFriendsOnly;
			// Now update the Live state
			OnlineSub.GameInterface.UpdateOnlineGame('Party',PartySettings,true);
		}
	}
}

/**
 * Sets the rich presence for training grounds if playing training grounds
 */
reliable client function ClientSetOnlineStatus()
{
	local GearGRI GRI;

	GRI = GearGRI(WorldInfo.GRI);
	if (GRI != None && GRI.TrainingGroundsID > -1)
	{
		SetRichPresenceString(class'GearVersusGameSettings'.const.CONTEXT_PRESENCE_TRAININGGROUNDSPRESENCE);
	}
	Super.ClientSetOnlineStatus();
}

/** reloads config files for the specified gametype on the client */
reliable client final function ClientLoadGameTypeConfigs(string GameTypeExtension)
{
	class'GearGame'.static.LoadGameTypeConfigs(GameTypeExtension);
}

simulated function OnOverrideRedReticleTraceDistance(SeqAct_OverrideRedReticleTraceDistance Action)
{
	if (Action.InputLinks[0].bHasImpulse)
	{
		DistanceForEnemyTrace = Action.TraceDistance;
	}
	else
	{
		DistanceForEnemyTrace = default.DistanceForEnemyTrace;
	}
}

/** Needed to pass a start IE_Released event to the UI **/
native function Sentinel_PressStartKeyAtStartMenu();

/** This will make a bugit with the current language.  So we can then copy them to the PC without them over writing each other **/
native function Sentinel_DoBugitWithLang();

simulated function OnToggleTargetingDOF(SeqAct_ToggleTargetingDOF Action)
{
	bDisableCameraTargetingDOF = Action.InputLinks[1].bHasImpulse;
}


/** Only spawn one MP friend of each type **/
protected simulated function AutomationSpawnAnMPFriend( const string MPPawnClassName, const string SpawnCommand )
{
	local GearPawn_COGGear GP;
	local bool bNotAliveYet;

	bNotAliveYet = TRUE;

	foreach WorldInfo.AllPawns( class'GearPawn_COGGear', GP )
	{
		if( string(GP.Class) == MPPawnClassName )
		{
			bNotAliveYet = FALSE;
			break;
		}
	}

	if( bNotAliveYet == TRUE )
	{
		ConsoleCommand( "spawnpawn " @ SpawnCommand );
	}

	foreach WorldInfo.AllPawns( class'GearPawn_COGGear', GP )
	{
		GP.bUnlimitedHealth = TRUE;
	}

}

exec simulated function Automation_KillMobs( optional float TimeBetweenKills )
{
	if( TimeBetweenKills == 0.0f )
	{
		TimeBetweenKills = 20.0f;
	}

	 GearPawn(Pawn).bUnlimitedHealth = TRUE;

	 // every round try to spawn our friends
	 AutomationSpawnAnMPFriend( "GearPawn_COGDizzyMP", "DizzyMP" );
	 AutomationSpawnAnMPFriend( "GearPawn_COGDomMP", "DomMP" );
	 AutomationSpawnAnMPFriend( "GearPawn_COGGusMP", "GusMP" );
	 AutomationSpawnAnMPFriend( "GearPawn_COGTaiMP", "TaiMP" );


	 SetTimer( TimeBetweenKills, TRUE, nameof( Automation_KillAMob) );

	 ConsoleCommand( "FractureAllMeshesToMaximizeMemoryUsage" );
}


/** Function to wrap all of the memory checking / tracking that we do **/
exec simulated function Automation_MemoryChecking()
{
	local int WarmUpEvents;
	local int EveryNEvents;
	local int DumpAllocsEvent;


	Automation_NumMemoryEvents++;

	if( GearGame(WorldInfo.Game).bCheckingForMemLeaks == TRUE )
	{
		ConsoleCommand( "memleakcheck" );
	}

	// these will just "pass" through if we are not running the MallocProfiler
	if( GearGame(WorldInfo.Game).bCheckingForMemLeaks == TRUE )
	{
		// here we need to check for which game type we are playing and adjust the snapshotting accordingly

		// horde
		if( GearGameHorde_Base(WorldInfo.Game) != None )
		{
			WarmUpEvents = 2;
			EveryNEvents = 10;
			DumpAllocsEvent = 49;
		}
		// annex
		else if( GearGameAnnex_Base(WorldInfo.Game) != None )
		{
			WarmUpEvents = 2;
			EveryNEvents = 5;
			DumpAllocsEvent = 20;
		}
		// other MP
		else
		{
			WarmUpEvents = 1;
			EveryNEvents = 1;
			DumpAllocsEvent = 5;
		}


		// allow for Events to "warmup some"
		if( Automation_NumMemoryEvents == WarmUpEvents )
		{
			ConsoleCommand( "SNAPSHOTMEMORY" );
		}

		// every 10 Events snap shot
		if( (Automation_NumMemoryEvents % EveryNEvents) == 0 )
		{
			ConsoleCommand( "SNAPSHOTMEMORY" );
		}

		// last Event dump out
		if( Automation_NumMemoryEvents == DumpAllocsEvent )
		{
			ConsoleCommand( "DUMPALLOCSTOFILE" );
			ClearTimer( nameof( Automation_KillAMob) );  // so we don't go back to main menu and have mem issues/defrag issues and will allow the mem profiler data to copy
		}
	}


	if( GearGame(WorldInfo.Game).bCheckingForFragmentation == TRUE )
	{
		ConsoleCommand( "MemFragCheck" );
	}
}


exec simulated function Automation_KillAMob()
{
	local GearPawn GP;

	// look at all pawns and see if we should kill it!
	foreach WorldInfo.AllPawns( class'GearPawn', GP )
	{
		// don't kill yourself
		// nor ragdolled pawns
		// nor your COGGear friends
		if( !GP.IsHumanControlled() && !GP.bTearOff && GearPawn_COGGear(GP) == none )
		{
			GP.TakeDamage( 10000.0f, self, GP.Location, vect(0,0,1), class'GDT_Explosive' );
			break;
		}
	}
}


function RestoreShowSubtitles()
{
	// MP has no subtitles!
	if (GearGRI(WorldInfo.GRI) != None && GearGRI(WorldInfo.GRI).IsMultiPlayerGame())
	{
		SetShowSubtitles( FALSE );
	}
	else
	{
		SetShowSubtitles( ProfileSettings.GetSubtitleConfigOption() );
	}
}

exec reliable client event ClientFlushGUDSVarietyBanks()
{
	// disable guds during cinematics
	if (GearGame(WorldInfo.Game).UnscriptedDialogueManager != None)
	{
		GearGame(WorldInfo.Game).UnscriptedDialogueManager.FlushOutAllVarietyBanks();
	}
}

reliable client event ClientPlayForceFeedbackWaveform(ForceFeedbackWaveform FFWaveform)
{
	local string MovieName;

	GetCurrentMovie(MovieName);
	if (MovieName == "" || MovieName != class'GearUIInteraction'.const.LOADING_MOVIE)
	{
		Super.ClientPlayForceFeedbackWaveform(FFWaveform);
	}
}

defaultproperties
{
    FovAngle=+00085.000000
	CameraClass=class'GearPlayerCamera'
	CheatClass=class'GearCheatManager'
	InputClass=class'GearPlayerInput'

	MinMeleeAdhesionDotValue=0.2588f		// 75 deg

	InputIsDisabledCount = 0
	bCinemaDisableInputButtons = false

	PostProcessPresets(0)=(Preset=TVT_Default,Shadows=1.0, Midtones=1.0, Highlights=1.0, Desaturation=1.0)
	PostProcessPresets(1)=(Preset=TVT_Soft,Shadows=0.8, Midtones=0.9, Highlights=1.1, Desaturation=1.1)
	PostProcessPresets(2)=(Preset=TVT_Lucent,Shadows=0.7, Midtones=0.8, Highlights=1.2, Desaturation=1.2)
	PostProcessPresets(3)=(Preset=TVT_Vibrant,Shadows=1.0, Midtones=1.0, Highlights=0.8, Desaturation=0.8)

	SavedMoveClass=class'GearSavedMove'

	PointOfInterestLookatInterpSpeedRange=(X=7.f,Y=7.f)

	DeathCameraEnemyMaxDistance=2048

	ARSuperSuccessScreenShake=(FOVAmplitude=0,LocAmplitude=(X=0,Y=0,Z=0),RotAmplitude=(X=0,Y=35,Z=35),RotFrequency=(X=0,Y=125,Z=125),TimeDuration=0.3)

	ActionLookAt={(
		ActionName=LookAtSomething,
		IconAnimationSpeed=0.35f,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=176,V=314,UL=35,VL=64), // Y Button
										  (Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=176,V=314,UL=35,VL=64))),
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=293,V=270,UL=74,VL=85),
										  (Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=367,V=270,UL=74,VL=85))) ),
		)}

	ActionLookAtDownedTeammate={(
		ActionName=DBNO,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=176,V=314,UL=35,VL=64))), // Y Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=119,V=108,UL=48,VL=63)))	), // replace this with the icon of the dude who died
		)}

	ActionValveTurn={(
		ActionName=ValveTurnable,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43))), // A Button
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_main',U=109,V=429,UL=68,VL=71)))	),
		)}

	ActionStayingAlive={(
		ActionName=StayingAlive,
		IconAnimationSpeed=0.1f,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=134,V=327,UL=93,VL=90), // Stick move icon
										  (Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=134,V=327,UL=93,VL=90))),
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=145,V=275,UL=75,VL=49), // Can move icon
										  (Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=145,V=275,UL=75,VL=49))),
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43),	// mash A button
										  (Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=248,V=314,UL=35,VL=43))) ),
		)}

	ActionSuicideBomb={(
		ActionName=SuicideBomb,
		ActionIconDatas=(	(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=99,V=375,UL=34,VL=42))), // Right trigger
							(ActionIcons=((Texture=Texture2D'Warfare_HUD.HUD.Gears_WeaponIcons',U=57,V=299,UL=78,VL=80)))	),
		)}

	ActionKismetButton=(ActionName=KismetButton)

	GearIcons(eGEARICON_A)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=212,V=314,UL=35,VL=43)
	GearIcons(eGEARICON_A_PRESSED)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=248,V=314,UL=35,VL=43)
	GearIcons(eGEARICON_B)=(Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=0,UL=45,VL=32)
	GearIcons(eGEARICON_B_PRESSED)=(Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=83,V=33,UL=45,VL=32)
	GearIcons(eGEARICON_X)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=330,V=314,UL=45,VL=32)
	GearIcons(eGEARICON_Y)=(Texture=Texture2D'Warfare_HUD.WarfareHUD_ActionIcons',U=176,V=314,UL=35,VL=64)
	GearIcons(eGEARICON_Chainsaw)=(Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=0,UL=83,VL=66)
	GearIcons(eGEARICON_Chainsaw_PRESSED)=(Texture=Texture2D'Warfare_HUD.HUD.HUD_ActionIcons2',U=0,V=68,UL=83,VL=66)

	CurrentSoundMode=None

	PointOfInterestAdded=SoundCue'Interface_Audio.Objectives.PointOfInterestAdded'
	SuddenDeathSound=SoundCue'Music_Stingers.stinger_instantdeath01cue'
	LastPoITime= 0.0f
	PoIRepeatTime = 15.0f

	Begin Object Class=AudioComponent Name=GameoverComp
		//SoundCue=SoundCue'Interface_Audio.Interface.ObjectiveFailLoopCue'
	End Object
	GameoverLoop=GameoverComp
	Components.Add(GameoverComp)

	OpenGameoverCue=SoundCue'Interface_Audio.Interface.ObjectiveFailEnterCue'
	LoadCheckpointCue=SoundCue'Interface_Audio.Interface.ObjectiveFailExitCue'

	InUseNodeCostMultiplier=10.0

	RotationRate=(Pitch=0,Yaw=0,Roll=0)

	LastCheckpointSaveTime=-10000.0
}
