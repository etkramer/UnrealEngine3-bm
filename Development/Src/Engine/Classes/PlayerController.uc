class PlayerController extends Controller
    native
    nativereplication
    config(Game)
    notplaceable
    dependson(OnlineSubsystem,OnlineGameSearch,SeqAct_ControlMovieTexture);

const MAXPOSITIONERRORSQUARED = 3.0;
const MAXNEARZEROVELOCITYSQUARED = 9.0;
const MAXVEHICLEPOSITIONERRORSQUARED = 900.0;
const CLIENTADJUSTUPDATECOST = 180.0;
const MAXCLIENTUPDATEINTERVAL = 0.25;

enum EInputTypes
{
    IT_XAxis,                       // 0
    IT_YAxis,                       // 1
    IT_MAX                          // 2
};

enum EInputMatchAction
{
    IMA_GreaterThan,                // 0
    IMA_LessThan,                   // 1
    IMA_MAX                         // 2
};

enum EProgressMessageType
{
    PMT_Clear,                      // 0
    PMT_Information,                // 1
    PMT_AdminMessage,               // 2
    PMT_DownloadProgress,           // 3
    PMT_ConnectionFailure,          // 4
    PMT_SocketFailure,              // 5
    PMT_MAX                         // 6
};

struct native ClientAdjustment
{
    var float TimeStamp;
    var Actor.EPhysics newPhysics;
    var Vector NewLoc;
    var Vector NewVel;
    var Actor NewBase;
    var Vector NewFloor;
    var byte bAckGoodMove;

    structdefaultproperties
    {
        TimeStamp=0.0000000
        newPhysics=PHYS_None
        NewLoc=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        NewVel=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        NewBase=none
        NewFloor=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        bAckGoodMove=0
    }
};

struct native InputEntry
{
    var PlayerController.EInputTypes Type;
    var float Value;
    var float TimeDelta;
    var PlayerController.EInputMatchAction Action;

    structdefaultproperties
    {
        Type=IT_XAxis
        Value=0.0000000
        TimeDelta=0.0000000
        Action=IMA_GreaterThan
    }
};

struct native InputMatchRequest
{
    var array<InputEntry> Inputs;
    var Actor MatchActor;
    var name MatchFuncName;
    var name FailedFuncName;
    var name RequestName;
    var transient int MatchIdx;
    var transient float LastMatchTime;

    structdefaultproperties
    {
        MatchActor=none
        MatchFuncName="None"
        FailedFuncName="None"
        RequestName="None"
        MatchIdx=0
        LastMatchTime=0.0000000
    }
};

struct native DebugTextInfo
{
    var Actor SrcActor;
    var Vector SrcActorOffset;
    var Vector SrcActorDesiredOffset;
    var string DebugText;
    var transient float TimeRemaining;
    var float Duration;
    var Color TextColor;

    structdefaultproperties
    {
        SrcActor=none
        SrcActorOffset=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        SrcActorDesiredOffset=(X=0.0000000,Y=0.0000000,Z=0.0000000)
        DebugText=""
        TimeRemaining=0.0000000
        Duration=0.0000000
        TextColor=(R=0,G=0,B=0,A=0)
    }
};

struct native AmbientSoundStruct
{
    var int AmbientSound_ReferenceNumber;
    var SoundCue AmbientSound_Cue;
    var int AmbientSound_Priority;
    var float AmbientSound_Time;

    structdefaultproperties
    {
        AmbientSound_ReferenceNumber=0
        AmbientSound_Cue=none
        AmbientSound_Priority=0
        AmbientSound_Time=0.0000000
    }
};

var const Player Player;
var Camera PlayerCamera;
var const class<Camera> CameraClass;
var DebugCameraController DebugCameraControllerRef;
var class<DebugCameraController> DebugCameraControllerClass;
var const class<PlayerOwnerDataStore> PlayerOwnerDataStoreClass;
var protected PlayerOwnerDataStore CurrentPlayerData;
var bool bFrozen;
var bool bPressedJump;
var bool bDoubleJump;
var bool bUpdatePosition;
var bool bUpdating;
var globalconfig bool bNeverSwitchOnPickup;
var bool bCheatFlying;
var bool bCameraPositionLocked;
var globalconfig bool bNoTextToSpeechVoiceMessages;
var globalconfig bool bTextToSpeechTeamMessagesOnly;
var bool bShortConnectTimeOut;
var const bool bPendingDestroy;
var bool bWasSpeedHack;
var const bool bWasSaturated;
var globalconfig bool bDynamicNetSpeed;
var globalconfig bool bAimingHelp;
var bool bClientSimulatingViewTarget;
var bool bHasVoiceHandshakeCompleted;
var bool bCinematicMode;
var bool bCinemaDisableInputMove;
var bool bCinemaDisableInputLook;
var bool bIgnoreNetworkMessages;
var bool bReplicateAllPawns;
var bool bIsUsingStreamingVolumes;
var bool bIsExternalUIOpen;
var bool bIsControllerConnected;
var bool bCheckSoundOcclusion;
var globalconfig bool bLogHearSoundOverflow;
var globalconfig bool bCheckRelevancyThroughPortals;
var transient bool bControllerWasDisconnected;
var transient bool bDidLoseFocusDeferPause;
var bool bReceivedUniqueId;
var float MaxResponseTime;
var float WaitDelay;
var Pawn AcknowledgedPawn;
var Actor.EDoubleClickDir DoubleClickDir;
var byte bIgnoreMoveInput;
var byte bIgnoreLookInput;
var input byte bRun;
var input byte bDuck;
var duplicatetransient const byte NetPlayerIndex;
var const Actor ViewTarget;
var PlayerReplicationInfo RealViewTarget;
var transient InterpTrackInstDirector ControllingDirTrackInst;
var float FOVAngle;
var float DesiredFOV;
var float DefaultFOV;
var const float LODDistanceFactor;
var Rotator TargetViewRotation;
var float TargetEyeHeight;
var Rotator BlendedTargetViewRotation;
var HUD myHUD;
var class<SavedMove> SavedMoveClass;
var SavedMove SavedMoves;
var SavedMove FreeMoves;
var SavedMove PendingMove;
var Vector LastAckedAccel;
var float CurrentTimeStamp;
var float LastUpdateTime;
var float ServerTimeStamp;
var float TimeMargin;
var float ClientUpdateTime;
var float MaxTimeMargin;
var float LastActiveTime;
var int ClientCap;
var globalconfig float DynamicPingThreshold;
var float LastPingUpdate;
var float OldPing;
var float LastSpeedHackLog;
var ClientAdjustment PendingAdjustment;
var string ProgressMessage[2];
var float ProgressTimeOut;
var const localized string QuickSaveString;
var const localized string NoPauseMessage;
var const localized string ViewingFrom;
var const localized string OwnCamera;
var int GroundPitch;
var Vector OldFloor;
var transient CheatManager CheatManager;
var class<CheatManager> CheatClass;
var() editinline transient PlayerInput PlayerInput;
var class<PlayerInput> InputClass;
var const Vector FailedPathStart;
var export editinline CylinderComponent CylinderComponent;
var config string ForceFeedbackManagerClassName;
var transient ForceFeedbackManager ForceFeedbackManager;
var transient array<Interaction> Interactions;
var array<UniqueNetId> VoiceMuteList;
var array<UniqueNetId> GameplayVoiceMuteList;
var array<UniqueNetId> VoicePacketFilter;
var OnlineSubsystem OnlineSub;
var OnlineVoiceInterface VoiceInterface;
var UIDataStore_OnlinePlayerData OnlinePlayerData;
var config float InteractDistance;
var name DelayedJoinSessionName;
var array<InputMatchRequest> InputRequests;
var float LastBroadcastTime;
var string LastBroadcastString[4];
var array<name> PendingMapChangeLevelNames;
var CoverReplicator MyCoverReplicator;
var private array<DebugTextInfo> DebugTextList;
var float SpectatorCameraSpeed;
var duplicatetransient const NetConnection PendingSwapConnection;
var float MinRespawnDelay;
var globalconfig int MaxConcurrentHearSounds;
var export editinline array<export editinline AudioComponent> HearSoundActiveComponents;
var export editinline array<export editinline AudioComponent> HearSoundPoolComponents;
var array<Actor> HiddenActors;
var array<AmbientSoundStruct> AmbientSoundStack;
var export editinline AudioComponent AmbCurrentSoundPtr;
var export editinline AudioComponent AmbOtherSoundPtr;
var float LastSpectatorStateSynchTime;
var SeqAct_Latent ActiveDialogueOptions;
//var delegate<CanUnpause> __CanUnpause__Delegate;

cpptext
{
	//  PlayerController interface.
	void SetPlayer( UPlayer* Player );
	void UpdateViewTarget(AActor* NewViewTarget);
	virtual void SmoothTargetViewRotation(APawn* TargetPawn, FLOAT DeltaSeconds);
	/** allows the game code an opportunity to modify post processing settings
	 * @param PPSettings - the post processing settings to apply
	 */
	virtual void ModifyPostProcessSettings(FPostProcessSettings& PPSettings) const;

	// AActor interface.
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
	virtual UBOOL Tick( FLOAT DeltaTime, enum ELevelTick TickType );
	virtual UBOOL IsNetRelevantFor(APlayerController* RealViewer, AActor* Viewer, const FVector& SrcLocation);
	virtual UBOOL LocalPlayerController();
	virtual UBOOL WantsLedgeCheck();
	virtual UBOOL StopAtLedge();
	virtual APlayerController* GetAPlayerController() { return this; }
	virtual UBOOL IgnoreBlockingBy( const AActor *Other ) const;
	virtual UBOOL HearSound(USoundCue* InSoundCue, AActor* SoundPlayer, const FVector& SoundLocation, UBOOL bStopWhenOwnerDestroyed);
	/** checks whether the passed in SoundPlayer is valid for replicating in a HearSound() call and sets it to NULL if not */
	void ValidateSoundPlayer(AActor*& SoundPlayer);
	virtual void PostScriptDestroyed();
	virtual FLOAT GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, APlayerController* Viewer, UActorChannel* InChannel, FLOAT Time, UBOOL bLowBandwidth);

	/** disables SeePlayer() and SeeMonster() checks for PlayerController, since they aren't used for most games */
	virtual UBOOL ShouldCheckVisibilityOf(AController* C) { return FALSE; }
	virtual void UpdateHiddenActors(FVector ViewLocation) {}
	virtual void HearNoise(AActor* NoiseMaker, FLOAT Loudness, FName NoiseType);

	/**
	 * Sets the Matinee director track instance that's currently possessing this player controller
	 *
	 * @param   NewControllingDirector    The director track instance that's now controlling this player controller (or NULL for none)
	 */
	void SetControllingDirector( UInterpTrackInstDirector* NewControllingDirector );

	/**
	 * Returns the Matinee director track that's currently possessing this player controller, or NULL for none
	 */
	UInterpTrackInstDirector* GetControllingDirector();

}

reliable client simulated function ClientDrawCoordinateSystem(Vector AxisLoc, Rotator AxisRot, float Scale, optional bool bPersistentLines)
{
    LogInternal("ClientDrawCoordinateSystem");
    DrawDebugCoordinateSystem(AxisLoc, AxisRot, Scale, bPersistentLines);
    //return;    
}

// Export UPlayerController::execAllowConsole(FFrame&, void* const)
native final function bool AllowConsole();

// Export UPlayerController::execSetNetSpeed(FFrame&, void* const)
native final function SetNetSpeed(int NewSpeed);

// Export UPlayerController::execGetPlayerNetworkAddress(FFrame&, void* const)
native final function string GetPlayerNetworkAddress();

// Export UPlayerController::execGetServerNetworkAddress(FFrame&, void* const)
native final function string GetServerNetworkAddress();

// Export UPlayerController::execConsoleCommand(FFrame&, void* const)
native function string ConsoleCommand(string Command, optional bool bWriteToLog = true);

// Export UPlayerController::execClientTravel(FFrame&, void* const)
reliable client native simulated event ClientTravel(string URL, Actor.ETravelType TravelType, optional bool bSeamless = false, init optional Guid MapPackageGuid);

// Export UPlayerController::execUpdateURL(FFrame&, void* const)
native(546) final function UpdateURL(string NewOption, string NewValue, bool bSave1Default);

// Export UPlayerController::execGetDefaultURL(FFrame&, void* const)
native final function string GetDefaultURL(string Option);

// Export UPlayerController::execCopyToClipboard(FFrame&, void* const)
native function CopyToClipboard(string Text);

// Export UPlayerController::execPasteFromClipboard(FFrame&, void* const)
native function string PasteFromClipboard();

// Export UPlayerController::execSetAllowMatureLanguage(FFrame&, void* const)
native function SetAllowMatureLanguage(bool bAllowMatureLanguge);

// Export UPlayerController::execClientConvolve(FFrame&, void* const)
private reliable client native final simulated event ClientConvolve(string C, int H);

// Export UPlayerController::execServerProcessConvolve(FFrame&, void* const)
private reliable server native final event ServerProcessConvolve(string C, int H);

// Export UPlayerController::execCheckSpeedHack(FFrame&, void* const)
native final function bool CheckSpeedHack(float DeltaTime);

// Export UPlayerController::execFindStairRotation(FFrame&, void* const)
native(524) final function int FindStairRotation(float DeltaTime);

// Export UPlayerController::execCleanUpAudioComponents(FFrame&, void* const)
native function CleanUpAudioComponents();

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
    //return;    
}

function ForceClearUnpauseDelegates()
{
    // End:0x30
    if(WorldInfo.Game != none)
    {
        WorldInfo.Game.ForceClearUnpauseDelegates(self);
    }
    //return;    
}

function OnExternalUIChanged(bool bIsOpening)
{
    bIsExternalUIOpen = bIsOpening;
    SetPause(bIsOpening, CanUnpauseExternalUI);
    //return;    
}

function bool CanUnpauseExternalUI()
{
    return ((!bIsExternalUIOpen || bPendingDelete) || bPendingDestroy) || bDeleteMe;
    //return ReturnValue;    
}

function OnControllerChanged(int ControllerId, bool bIsConnected)
{
    local LocalPlayer LP;

    LP = LocalPlayer(Player);
    // End:0x146
    if((((LP != none) && LP.ControllerId == ControllerId) && WorldInfo.IsConsoleBuild()) && (WorldInfo.Game == none) || !WorldInfo.Game.bAutomatedPerfTesting)
    {
        bIsControllerConnected = bIsConnected;
        bControllerWasDisconnected = !bIsConnected;
        LogInternal((((("Received gamepad connection change for player" @ string(Class'UIInteraction'.static.GetPlayerIndex(ControllerId))) $ ": gamepad") @ string(ControllerId)) @ "is now") @ ((bIsConnected) ? "connected" : "disconnected"));
        // End:0x146
        if(!bIsConnected)
        {
            Pause();
        }
    }
    //return;    
}

function bool CanUnpauseControllerConnected()
{
    return bIsControllerConnected;
    //return ReturnValue;    
}

function CoverReplicator SpawnCoverReplicator()
{
    // End:0x5A
    if(((MyCoverReplicator == none) && Role == ROLE_Authority) && LocalPlayer(Player) == none)
    {
        MyCoverReplicator = Spawn(Class'CoverReplicator', self);
        MyCoverReplicator.ReplicateInitialCoverInfo();
    }
    return MyCoverReplicator;
    //return ReturnValue;    
}

simulated event PostBeginPlay()
{
    super.PostBeginPlay();
    ResetCameraMode();
    MaxTimeMargin = Class'GameInfo'.default.MaxTimeMargin;
    MaxResponseTime = default.MaxResponseTime * WorldInfo.TimeDilation;
    // End:0x68
    if(WorldInfo.NetMode == NM_Client)
    {
        SpawnDefaultHUD();        
    }
    else
    {
        AddCheats();
    }
    SetViewTarget(self);
    LastActiveTime = WorldInfo.TimeSeconds;
    OnlineSub = Class'GameEngine'.static.GetOnlineSubsystem();
    LogInternal("Set OnlineSub = " @ string(OnlineSub));
    //return;    
}

simulated event ReceivedPlayer()
{
    local LocalPlayer LP;
    local PlayerController FirstPlayer;

    // End:0xB5
    if((PlayerReplicationInfo != none) && IsSplitscreenPlayer())
    {
        // End:0x9C
        if(NetPlayerIndex != 0)
        {
            LP = LocalPlayer(Player);
            FirstPlayer = LP.ViewportClient.Outer.GamePlayers[0].Actor;
            // DebugMode: 0
            assert(FirstPlayer != self);
            FirstPlayer.PlayerReplicationInfo.SetSplitscreenIndex(0);
        }
        PlayerReplicationInfo.SetSplitscreenIndex(NetPlayerIndex);
    }
    RegisterPlayerDataStores();
    //return;    
}

event PreRender(Canvas Canvas)
{
    //return;    
}

function ResetTimeMargin()
{
    TimeMargin = -0.1000000;
    MaxTimeMargin = Class'GameInfo'.default.MaxTimeMargin;
    //return;    
}

reliable server function ServerShortTimeout()
{
    //return;    
}

function ServerGivePawn()
{
    GivePawn(Pawn);
    //return;    
}

event KickWarning()
{
    ReceiveLocalizedMessage(Class'GameMessage', 15);
    //return;    
}

function AddCheats()
{
    // End:0x56
    if(((CheatManager == none) && WorldInfo.Game != none) && WorldInfo.Game.AllowCheats(self))
    {
        CheatManager = new (self) CheatClass;
    }
    //return;    
}

exec function EnableCheats()
{
    AddCheats();
    //return;    
}

function SpawnDefaultHUD()
{
    // End:0x12
    if(LocalPlayer(Player) == none)
    {
        return;
    }
    LogInternal(string(GetFuncName()));
    myHUD = Spawn(Class'HUD', self);
    //return;    
}

function Reset()
{
    // End:0x24
    if(Pawn != none)
    {
        PawnDied(Pawn);
        UnPossess();
    }
    super.Reset();
    SetViewTarget(self);
    ResetCameraMode();
    WaitDelay = WorldInfo.TimeSeconds + float(2);
    FixFOV();
    // End:0x89
    if(PlayerReplicationInfo.bOnlySpectator)
    {
        GotoState('Spectating');        
    }
    else
    {
        GotoState('PlayerWaiting');
    }
    //return;    
}

reliable client simulated function ClientReset()
{
    ResetCameraMode();
    SetViewTarget(self);
    GotoState(((PlayerReplicationInfo.bOnlySpectator) ? 'Spectating' : 'PlayerWaiting'));
    //return;    
}

function CleanOutSavedMoves()
{
    SavedMoves = none;
    PendingMove = none;
    //return;    
}

function PreControllerIdChange()
{
    local LocalPlayer LP;

    LP = LocalPlayer(Player);
    // End:0x39
    if(LP != none)
    {
        ClientStopNetworkedVoice();
        ClearOnlineDelegates();
        UnregisterPlayerDataStores();
    }
    //return;    
}

function PostControllerIdChange()
{
    local LocalPlayer LP;

    LP = LocalPlayer(Player);
    // End:0x7E
    if(LP != none)
    {
        InitUniquePlayerId();
        RegisterPlayerDataStores();
        RegisterOnlineDelegates();
        ClientSetOnlineStatus();
        // DebugMode: 0
        assert(WorldInfo.Game != none);
        // End:0x7E
        if(!WorldInfo.Game.bRequiresPushToTalk)
        {
            ClientStartNetworkedVoice();
        }
    }
    //return;    
}

final simulated function OnlineSubsystem GetOnlineSubsystem()
{
    // End:0x21
    if(OnlineSub == none)
    {
        OnlineSub = Class'GameEngine'.static.GetOnlineSubsystem();
    }
    return OnlineSub;
    //return ReturnValue;    
}

event InitInputSystem()
{
    local class<ForceFeedbackManager> FFManagerClass;
    local int I;
    local Sequence GameSeq;
    local array<SequenceObject> AllInterpActions;

    // End:0x27
    if(PlayerInput == none)
    {
        // DebugMode: 0
        assert(InputClass != none);
        PlayerInput = new (self) InputClass;
    }
    // End:0x50
    if(Interactions.Find(PlayerInput) == -1)
    {
        Interactions[Interactions.Length] = PlayerInput;
    }
    // End:0x93
    if(ForceFeedbackManagerClassName != "")
    {
        FFManagerClass = class<ForceFeedbackManager>(DynamicLoadObject(ForceFeedbackManagerClassName, Class'Core.Class'));
        // End:0x93
        if(FFManagerClass != none)
        {
            ForceFeedbackManager = new (self) FFManagerClass;
        }
    }
    RegisterOnlineDelegates();
    // End:0x126
    if(Role < ROLE_Authority)
    {
        GameSeq = WorldInfo.GetGameSequence();
        // End:0x126
        if(GameSeq != none)
        {
            GameSeq.FindSeqObjectsByClass(Class'SeqAct_Interp', true, AllInterpActions);
            I = 0;
            J0xF0:

            // End:0x126 [Loop If]
            if(I < AllInterpActions.Length)
            {
                SeqAct_Interp(AllInterpActions[I]).AddPlayerToDirectorTracks(self);
                I++;
                // [Loop Continue]
                goto J0xF0;
            }
        }
    }
    SetOnlyUseControllerTiltInput(false);
    SetUseTiltForwardAndBack(true);
    SetControllerTiltActive(false);
    //return;    
}

simulated event ReplicatedEvent(name VarName)
{
    super.ReplicatedEvent(VarName);
    // End:0x35
    if((VarName == 'PlayerReplicationInfo') && !bReceivedUniqueId)
    {
        InitUniquePlayerId();
    }
    //return;    
}

event InitUniquePlayerId()
{
    local LocalPlayer LocPlayer;
    local OnlineGameSettings GameSettings;
    local UniqueNetId PlayerID;

    // End:0x17F
    if(PlayerReplicationInfo != none)
    {
        LocPlayer = LocalPlayer(Player);
        // End:0x17C
        if((((LocPlayer != none) && PlayerReplicationInfo != none) && OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.PlayerInterface, none))
        {
            OnlineSub.PlayerInterface.GetUniquePlayerId(byte(LocPlayer.ControllerId), PlayerID);
            PlayerReplicationInfo.SetUniqueId(PlayerID);
            // End:0x17C
            if(WorldInfo.NetMode == NM_Client)
            {
                // End:0x11A
                if(NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
                {
                    GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
                }
                ServerSetUniquePlayerId(PlayerID, (GameSettings != none) && GameSettings.bWasFromInvite);
                bReceivedUniqueId = true;
                // End:0x17C
                if(PlayerReplicationInfo.PlayerSkill > 0)
                {
                    ServerSetPlayerSkill(PlayerReplicationInfo.PlayerSkill);
                }
            }
        }        
    }
    else
    {
        LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ": PlayerReplicationInfo is None....aborting.", 'DevOnline');
    }
    //return;    
}

reliable server function ServerSetUniquePlayerId(UniqueNetId UniqueId, bool bWasInvited)
{
    local UniqueNetId ZeroId;
    local OnlineGameSettings GameSettings;

    // End:0x26A
    if(!bPendingDestroy && !bReceivedUniqueId)
    {
        // End:0x77
        if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
        {
            GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
        }
        // End:0xE7
        if(WorldInfo.Game.AccessControl.IsIDBanned(UniqueId))
        {
            LogInternal(PlayerReplicationInfo.GetPlayerAlias() @ "is banned, kicking...");
            ClientWasKicked();
            Destroy();            
        }
        else
        {
            // End:0x184
            if(((WorldInfo.IsConsoleBuild() && GameSettings != none) && !GameSettings.bIsLanMatch) && UniqueId == ZeroId)
            {
                LogInternal(PlayerReplicationInfo.GetPlayerAlias() @ "is not validated/signed in, kicking...");
                ClientWasKicked();
                Destroy();                
            }
            else
            {
                PlayerReplicationInfo.SetUniqueId(UniqueId);
                // End:0x20B
                if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
                {
                    OnlineSub.GameInterface.RegisterPlayer(PlayerReplicationInfo.SessionName, PlayerReplicationInfo.UniqueId, bWasInvited);
                }
                // End:0x262
                if(WorldInfo.NetMode != NM_Client)
                {
                    WorldInfo.Game.UpdateGameplayMuteList(self);
                    WorldInfo.Game.RecalculateSkillRating();
                }
                bReceivedUniqueId = true;
            }
        }
    }
    //return;    
}

reliable server function ServerSetPlayerSkill(int PlayerSkill)
{
    PlayerReplicationInfo.PlayerSkill = PlayerSkill;
    //return;    
}

reliable client simulated function ClientInitializeDataStores()
{
    LogInternal(">> PlayerController::ClientInitializeDataStores for player" @ string(self), 'DevDataStore');
    RegisterPlayerDataStores();
    LogInternal("<< PlayerController::ClientInitializeDataStores for player" @ string(self), 'DevDataStore');
    //return;    
}

final simulated function RegisterPlayerDataStores()
{
    RegisterCustomPlayerDataStores();
    RegisterStandardPlayerDataStores();
    //return;    
}

protected simulated function RegisterCustomPlayerDataStores()
{
    local LocalPlayer LP;
    local DataStoreClient DataStoreManager;
    local class<UIDataStore_OnlinePlayerData> PlayerDataStoreClass;
    local string PlayerName;

    PlayerName = ((PlayerReplicationInfo != none) ? PlayerReplicationInfo.PlayerName : "None");
    LP = LocalPlayer(Player);
    LogInternal(((((((((">>" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")") @ "LP:") $ ((LP != none) ? string(LP.Name) : "None"), 'DevDataStore');
    // End:0x588
    if(LP != none)
    {
        DataStoreManager = Class'UIInteraction'.static.GetDataStoreClient();
        // End:0x588
        if(DataStoreManager != none)
        {
            CurrentPlayerData = PlayerOwnerDataStore(DataStoreManager.FindDataStore('PlayerOwner', LP));
            // End:0x283
            if(CurrentPlayerData == none)
            {
                CurrentPlayerData = DataStoreManager.CreateDataStore(PlayerOwnerDataStoreClass);
                // End:0x213
                if(CurrentPlayerData != none)
                {
                    // End:0x17A
                    if(DataStoreManager.RegisterDataStore(CurrentPlayerData, LP))
                    {
                        // End:0x177
                        if(PlayerReplicationInfo != none)
                        {
                            PlayerReplicationInfo.BindPlayerOwnerDataProvider();
                        }                        
                    }
                    else
                    {
                        LogInternal(((((("Failed to register 'PlayerOwner' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ")") @ "CurrentPlayerData:") $ ((CurrentPlayerData != none) ? string(CurrentPlayerData.Name) : "None"), 'DevDataStore');
                    }                    
                }
                else
                {
                    LogInternal((((("Failed to create 'PlayerOwner' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ") using class") @ string(PlayerOwnerDataStoreClass), 'DevDataStore');
                }                
            }
            else
            {
                LogInternal(((("'PlayerOwner' data store already registered for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
            }
            OnlinePlayerData = UIDataStore_OnlinePlayerData(DataStoreManager.FindDataStore('OnlinePlayerData', LP));
            // End:0x529
            if(OnlinePlayerData == none)
            {
                PlayerDataStoreClass = class<UIDataStore_OnlinePlayerData>(DataStoreManager.FindDataStoreClass(Class'UIDataStore_OnlinePlayerData'));
                // End:0x493
                if(PlayerDataStoreClass != none)
                {
                    OnlinePlayerData = DataStoreManager.CreateDataStore(PlayerDataStoreClass);
                    // End:0x41E
                    if(OnlinePlayerData != none)
                    {
                        // End:0x41B
                        if(!DataStoreManager.RegisterDataStore(OnlinePlayerData, LP))
                        {
                            LogInternal(((((("Failed to register 'OnlinePlayerData' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ")") @ "OnlinePlayerData:") $ ((OnlinePlayerData != none) ? string(OnlinePlayerData.Name) : "None"), 'DevDataStore');
                        }                        
                    }
                    else
                    {
                        LogInternal((((("Failed to create 'OnlinePlayerData' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ") using class") @ string(PlayerDataStoreClass), 'DevDataStore');
                    }                    
                }
                else
                {
                    LogInternal(((("Failed to find valid data store class while attempting to register the 'OnlinePlayerData' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
                }                
            }
            else
            {
                LogInternal(((("'OnlinePlayerData' data store already registered for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
            }
        }
    }
    LogInternal((((((("<<" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
    //return;    
}

protected simulated function RegisterStandardPlayerDataStores()
{
    local LocalPlayer LP;
    local DataStoreClient DataStoreManager;
    local array< class<UIDataStore> > PlayerDataStoreClasses;
    local class<UIDataStore> PlayerDataStoreClass;
    local UIDataStore PlayerDataStore;
    local int ClassIndex;
    local string PlayerName;

    PlayerName = ((PlayerReplicationInfo != none) ? PlayerReplicationInfo.PlayerName : "None");
    LP = LocalPlayer(Player);
    // End:0x3A4
    if(LP != none)
    {
        LogInternal(((((((">>" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
        DataStoreManager = Class'UIInteraction'.static.GetDataStoreClient();
        // End:0x3A4
        if(DataStoreManager != none)
        {
            DataStoreManager.GetPlayerDataStoreClasses(PlayerDataStoreClasses);
            ClassIndex = 0;
            J0xD0:

            // End:0x3A4 [Loop If]
            if(ClassIndex < PlayerDataStoreClasses.Length)
            {
                PlayerDataStoreClass = PlayerDataStoreClasses[ClassIndex];
                // End:0x39A
                if(PlayerDataStoreClass != none)
                {
                    PlayerDataStore = DataStoreManager.FindDataStore(PlayerDataStoreClass.default.Tag, LP);
                    // End:0x334
                    if(PlayerDataStore == none)
                    {
                        LogInternal(((((((("    Registering standard player data store '" $ string(PlayerDataStoreClass.Name)) $ "' for player") @ string(self)) @ "(") $ PlayerName) $ ")") @ "LP:") $ ((LP != none) ? string(LP.Name) : "None"), 'DevDataStore');
                        PlayerDataStore = DataStoreManager.CreateDataStore(PlayerDataStoreClass);
                        // End:0x2B8
                        if(PlayerDataStore != none)
                        {
                            // End:0x2B5
                            if(!DataStoreManager.RegisterDataStore(PlayerDataStore, LP))
                            {
                                LogInternal(((((((("Failed to register '" $ string(PlayerDataStoreClass.default.Tag)) $ "' data store for player:") @ string(self)) @ "(") $ PlayerName) $ ")") @ "PlayerDataStore:") $ ((PlayerDataStore != none) ? string(PlayerDataStore.Name) : "None"), 'DevDataStore');
                            }                            
                        }
                        else
                        {
                            LogInternal((((((("Failed to create '" $ string(PlayerDataStoreClass.default.Tag)) $ "' data store for player:") @ string(self)) @ "(") $ PlayerName) $ ") using class") @ string(PlayerOwnerDataStoreClass), 'DevDataStore');
                        }
                        // [Explicit Continue]
                        goto J0x39A;
                    }
                    LogInternal(((((("'" $ string(PlayerDataStoreClass.default.Tag)) $ "' data store already registered for player:") @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
                }
                J0x39A:

                ClassIndex++;
                // [Loop Continue]
                goto J0xD0;
            }
        }
    }
    //return;    
}

simulated function UnregisterPlayerDataStores()
{
    local LocalPlayer LP;
    local DataStoreClient DataStoreManager;
    local UIDataStore_OnlinePlayerData OnlinePlayerDataStore;
    local string PlayerName;

    PlayerName = ((PlayerReplicationInfo != none) ? PlayerReplicationInfo.PlayerName : "None");
    LP = LocalPlayer(Player);
    // End:0x3AB
    if(LP != none)
    {
        LogInternal(((((((">>" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
        DataStoreManager = Class'UIInteraction'.static.GetDataStoreClient();
        // End:0x332
        if(DataStoreManager != none)
        {
            // End:0x17B
            if(CurrentPlayerData != none)
            {
                // End:0x171
                if(!DataStoreManager.UnregisterDataStore(CurrentPlayerData))
                {
                    LogInternal(((((("Failed to unregister 'PlayerOwner' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ")") @ "CurrentPlayerData:") $ ((CurrentPlayerData != none) ? string(CurrentPlayerData.Name) : "None"), 'DevDataStore');
                }
                CurrentPlayerData = none;                
            }
            else
            {
                LogInternal(((("'PlayerOwner' data store not registered for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
            }
            OnlinePlayerData = none;
            OnlinePlayerDataStore = UIDataStore_OnlinePlayerData(DataStoreManager.FindDataStore('OnlinePlayerData', LP));
            // End:0x2CA
            if(OnlinePlayerDataStore != none)
            {
                // End:0x2C7
                if(!DataStoreManager.UnregisterDataStore(OnlinePlayerDataStore))
                {
                    LogInternal(((((("Failed to unregister 'OnlinePlayerData' data store for player:" @ string(self)) @ "(") $ PlayerName) $ ")") @ "OnlinePlayerDataStore:") $ ((OnlinePlayerDataStore != none) ? string(OnlinePlayerDataStore.Name) : "None"), 'DevDataStore');
                }                
            }
            else
            {
                LogInternal(((("'OnlinePlayerData' data store not registered for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
            }
            UnregisterStandardPlayerDataStores();            
        }
        else
        {
            LogInternal("Data store client not found!", 'DevDataStore');
        }
        LogInternal((((((("<<" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
    }
    //return;    
}

simulated function UnregisterStandardPlayerDataStores()
{
    local LocalPlayer LP;
    local DataStoreClient DataStoreManager;
    local array< class<UIDataStore> > PlayerDataStoreClasses;
    local class<UIDataStore> PlayerDataStoreClass;
    local UIDataStore PlayerDataStore;
    local int ClassIndex;
    local string PlayerName;

    PlayerName = ((PlayerReplicationInfo != none) ? PlayerReplicationInfo.PlayerName : "None");
    LP = LocalPlayer(Player);
    // End:0x247
    if(LP != none)
    {
        LogInternal(((((((">>" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
        DataStoreManager = Class'UIInteraction'.static.GetDataStoreClient();
        // End:0x1F7
        if(DataStoreManager != none)
        {
            DataStoreManager.GetPlayerDataStoreClasses(PlayerDataStoreClasses);
            ClassIndex = 0;
            J0xD0:

            // End:0x1F7 [Loop If]
            if(ClassIndex < PlayerDataStoreClasses.Length)
            {
                PlayerDataStoreClass = PlayerDataStoreClasses[ClassIndex];
                // End:0x1ED
                if(PlayerDataStoreClass != none)
                {
                    PlayerDataStore = DataStoreManager.FindDataStore(PlayerDataStoreClass.default.Tag, LP);
                    // End:0x1ED
                    if(PlayerDataStore != none)
                    {
                        // End:0x1ED
                        if(!DataStoreManager.UnregisterDataStore(PlayerDataStore))
                        {
                            LogInternal(((((((("Failed to unregister '" $ string(PlayerDataStore.Tag)) $ "' data store for player:") @ string(self)) @ "(") $ PlayerName) $ ")") @ "PlayerDataStore:") $ ((PlayerDataStore != none) ? string(PlayerDataStore.Name) : "None"), 'DevDataStore');
                        }
                    }
                }
                ClassIndex++;
                // [Loop Continue]
                goto J0xD0;
            }
        }
        LogInternal((((((("<<" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ ")", 'DevDataStore');
    }
    //return;    
}

simulated function SetPlayerDataProvider(PlayerDataProvider DataProvider)
{
    local string PlayerName;

    PlayerName = ((PlayerReplicationInfo != none) ? PlayerReplicationInfo.PlayerName : "None");
    LogInternal((((((((">>" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ "):") @ string(DataProvider), 'DevDataStore');
    // End:0x93
    if(CurrentPlayerData == none)
    {
        RegisterPlayerDataStores();
    }
    // End:0xD9
    if(CurrentPlayerData != none)
    {
        // End:0xAC
        if(DataProvider != none)
        {            
        }
        else
        {
            LogInternal("NULL data provider specified!", 'DevDataStore');
        }        
    }
    else
    {
        LogInternal(((("'PlayerOwner' data store not yet registered for player:" @ string(self)) @ "(") $ PlayerName) $ ")", 'DevDataStore');
    }
    LogInternal(((((((("<<" @ "(") $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "(") $ PlayerName) $ "):") @ string(DataProvider), 'DevDataStore');
    //return;    
}

final function SetRumbleScale(float ScaleBy)
{
    // End:0x20
    if(ForceFeedbackManager != none)
    {
        ForceFeedbackManager.ScaleAllWaveformsBy = ScaleBy;
    }
    //return;    
}

final function float GetRumbleScale()
{
    local float retval;

    retval = 1.0000000;
    // End:0x2B
    if(ForceFeedbackManager != none)
    {
        retval = ForceFeedbackManager.ScaleAllWaveformsBy;
    }
    return retval;
    //return ReturnValue;    
}

// Export UPlayerController::execIsControllerTiltActive(FFrame&, void* const)
native simulated function bool IsControllerTiltActive() const;

// Export UPlayerController::execSetControllerTiltDesiredIfAvailable(FFrame&, void* const)
native simulated function SetControllerTiltDesiredIfAvailable(bool bActive);

// Export UPlayerController::execSetControllerTiltActive(FFrame&, void* const)
native simulated function SetControllerTiltActive(bool bActive);

// Export UPlayerController::execSetOnlyUseControllerTiltInput(FFrame&, void* const)
native simulated function SetOnlyUseControllerTiltInput(bool bActive);

// Export UPlayerController::execSetUseTiltForwardAndBack(FFrame&, void* const)
native simulated function SetUseTiltForwardAndBack(bool bActive);

// Export UPlayerController::execIsKeyboardAvailable(FFrame&, void* const)
native simulated function bool IsKeyboardAvailable() const;

// Export UPlayerController::execIsMouseAvailable(FFrame&, void* const)
native simulated function bool IsMouseAvailable() const;

reliable client simulated function ClientGotoState(name NewState, optional name NewLabel)
{
    // End:0x46
    if(((NewLabel == 'Begin') || NewLabel == 'None') && !IsInState(NewState))
    {
        GotoState(NewState);        
    }
    else
    {
        GotoState(NewState, NewLabel);
    }
    //return;    
}

reliable server function AskForPawn()
{
    // End:0x26
    if(GamePlayEndedState())
    {
        ClientGotoState(GetStateName(), 'Begin');        
    }
    else
    {
        // End:0x43
        if(Pawn != none)
        {
            GivePawn(Pawn);            
        }
        else
        {
            bFrozen = false;
            ServerRestartPlayer();
        }
    }
    //return;    
}

reliable client simulated function GivePawn(Pawn NewPawn)
{
    // End:0x0D
    if(NewPawn == none)
    {
        return;
    }
    Pawn = NewPawn;
    NewPawn.Controller = self;
    ClientRestart(Pawn);
    //return;    
}

event Possess(Pawn aPawn, bool bVehicleTransition)
{
    local Actor A;
    local int I;
    local SeqEvent_Touch TouchEvent;

    // End:0x146
    if(!PlayerReplicationInfo.bOnlySpectator)
    {
        // End:0x48
        if(aPawn.Controller != none)
        {
            aPawn.Controller.UnPossess();
        }
        aPawn.PossessedBy(self, bVehicleTransition);
        Pawn = aPawn;
        Pawn.bStasis = false;
        ResetTimeMargin();
        UpdateSex();
        Restart(bVehicleTransition);
        // End:0x145
        foreach Pawn.TouchingActors(Class'Actor', A)
        {
            I = 0;
            J0xC5:

            // End:0x144 [Loop If]
            if(I < A.GeneratedEvents.Length)
            {
                TouchEvent = SeqEvent_Touch(A.GeneratedEvents[I]);
                // End:0x13A
                if((TouchEvent != none) && TouchEvent.bPlayerOnly)
                {
                    TouchEvent.CheckTouchActivate(A, Pawn);
                }
                I++;
                // [Loop Continue]
                goto J0xC5;
            }            
        }        
    }
    //return;    
}

function AcknowledgePossession(Pawn P)
{
    // End:0x68
    if(LocalPlayer(Player) != none)
    {
        AcknowledgedPawn = P;
        // End:0x59
        if(P != none)
        {
            P.SetBaseEyeheight();
            P.EyeHeight = P.BaseEyeHeight;
        }
        ServerAcknowledgePossession(P);
    }
    //return;    
}

reliable server function ServerAcknowledgePossession(Pawn P)
{
    // End:0x37
    if(((P != none) && P == Pawn) && P != AcknowledgedPawn)
    {
        ResetTimeMargin();
    }
    AcknowledgedPawn = P;
    //return;    
}

event UnPossess()
{
    // End:0x6D
    if(Pawn != none)
    {
        SetLocation(Pawn.Location);
        Pawn.RemoteRole = ROLE_SimulatedProxy;
        Pawn.UnPossessed();
        CleanOutSavedMoves();
        // End:0x6D
        if((GetViewTarget()) == Pawn)
        {
            SetViewTarget(self);
        }
    }
    Pawn = none;
    //return;    
}

function PawnDied(Pawn P)
{
    // End:0x11
    if(P != Pawn)
    {
        return;
    }
    // End:0x2E
    if(Pawn != none)
    {
        Pawn.RemoteRole = ROLE_SimulatedProxy;
    }
    super.PawnDied(P);
    //return;    
}

reliable client simulated function ClientSetHUD(class<HUD> newHUDType, class<ScoreBoard> newScoringType)
{
    // End:0x18
    if(myHUD != none)
    {
        myHUD.Destroy();
    }
    // End:0x2D
    if(newHUDType == none)
    {
        myHUD = none;        
    }
    else
    {
        myHUD = Spawn(newHUDType, self);
        // End:0x68
        if(myHUD != none)
        {
            myHUD.SpawnScoreBoard(newScoringType);
        }
    }
    //return;    
}

function HandlePickup(Inventory Inv)
{
    //return;    
}

function CleanupPRI()
{
    WorldInfo.Game.AddInactivePRI(PlayerReplicationInfo, self);
    PlayerReplicationInfo = none;
    //return;    
}

reliable client simulated event ReceiveLocalizedMessage(class<LocalMessage> Message, optional int Switch, optional PlayerReplicationInfo RelatedPRI_1, optional PlayerReplicationInfo RelatedPRI_2, optional Object OptionalObject)
{
    // End:0x37
    if((WorldInfo.NetMode == NM_DedicatedServer) || WorldInfo.GRI == none)
    {
        return;
    }
    Message.static.ClientReceive(self, Switch, RelatedPRI_1, RelatedPRI_2, OptionalObject);
    //return;    
}

unreliable client simulated event ClientPlaySound(SoundCue ASound)
{
    ClientHearSound(ASound, self, Location, false, false);
    //return;    
}

simulated function HearSoundFinished(AudioComponent AC)
{
    HearSoundActiveComponents.RemoveItem(AC);
    // End:0x44
    if(!AC.IsPendingKill())
    {
        AC.ResetToDefaults();
        HearSoundPoolComponents[HearSoundPoolComponents.Length] = AC;
    }
    //return;    
}

// Export UPlayerController::execGetPooledAudioComponent(FFrame&, void* const)
native function AudioComponent GetPooledAudioComponent(SoundCue ASound, Actor SourceActor, bool bStopWhenOwnerDestroyed, optional bool bUseLocation, optional Vector SourceLocation);

unreliable client simulated event ClientHearSound(SoundCue ASound, Actor SourceActor, Vector SourceLocation, bool bStopWhenOwnerDestroyed, optional bool bIsOccluded)
{
    local AudioComponent AC;

    // End:0x69
    if(SourceActor == none)
    {
        AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed, true, SourceLocation);
        // End:0x3F
        if(AC == none)
        {
            return;
        }
        AC.bUseOwnerLocation = false;
        AC.Location = SourceLocation;        
    }
    else
    {
        // End:0xCA
        if((SourceActor == (GetViewTarget())) || SourceActor == self)
        {
            AC = GetPooledAudioComponent(ASound, none, bStopWhenOwnerDestroyed);
            // End:0xB5
            if(AC == none)
            {
                return;
            }
            AC.bAllowSpatialization = false;            
        }
        else
        {
            AC = GetPooledAudioComponent(ASound, SourceActor, bStopWhenOwnerDestroyed);
            // End:0xF9
            if(AC == none)
            {
                return;
            }
            // End:0x148
            if(!IsZero(SourceLocation) && SourceLocation != SourceActor.Location)
            {
                AC.bUseOwnerLocation = false;
                AC.Location = SourceLocation;
            }
        }
    }
    AC.Play();
    //return;    
}

simulated function bool IsClosestLocalPlayerToActor(Actor TheActor)
{
    local PlayerController PC;
    local float MyDist;

    // End:0x0D
    if(ViewTarget == none)
    {
        return false;
    }
    MyDist = VSize(ViewTarget.Location - TheActor.Location);
    // End:0xA6
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        // End:0xA5
        if(((PC != self) && PC.ViewTarget != none) && VSize(PC.ViewTarget.Location - TheActor.Location) < MyDist)
        {            
            return false;
        }        
    }    
    return true;
    //return ReturnValue;    
}

function bool IsTooFarForSubs(SoundCue ASound, Vector Loc)
{
    local float maxD;

    // End:0x98
    if(SoundNodeRandom(ASound.FirstNode) != none)
    {
        SoundNodeRandom(ASound.FirstNode).PickNextNodeInAdvance();
        maxD = SoundNodeWave(SoundNodeRandom(ASound.FirstNode).ChildNodes[SoundNodeRandom(ASound.FirstNode).NextRandomToForce]).MaxRange * float(100);        
    }
    else
    {
        // End:0xDC
        if(SoundNodeWave(ASound.FirstNode) != none)
        {
            maxD = SoundNodeWave(ASound.FirstNode).MaxRange * float(100);
        }
    }
    // End:0x107
    if(maxD < (VSize(Pawn.Location - Loc) * 0.8500000))
    {
        return true;
    }
    return false;
    //return ReturnValue;    
}

reliable client simulated event Kismet_ClientPlaySound(SoundCue ASound, Actor SourceActor, float VolumeMultiplier, float PitchMultiplier, float FadeInTime, bool bSuppressSubtitles, bool bSuppressSpatialization)
{
    local AudioComponent AC;

    // End:0x1B8
    if((SourceActor != none) && IsClosestLocalPlayerToActor(SourceActor))
    {
        // End:0x83
        if((ASound.FaceFXAnimName != "") && SourceActor.PlayActorFaceFXAnim(ASound.FaceFXAnimSetRef, ASound.FaceFXGroupName, ASound.FaceFXAnimName, ASound))
        {            
        }
        else
        {
            AC = SourceActor.CreateAudioComponent(ASound, false, true);
            // End:0x167
            if(AC != none)
            {
                AC.VolumeMultiplier = VolumeMultiplier;
                AC.PitchMultiplier = PitchMultiplier;
                AC.bAutoDestroy = true;
                AC.SubtitlePriority = 10000.0000000;
                AC.bSuppressSubtitles = bSuppressSubtitles || IsTooFarForSubs(ASound, SourceActor.Location);
                // End:0x154
                if(bSuppressSpatialization)
                {
                    AC.bAllowSpatialization = false;
                }
                AC.Play();                
            }
            else
            {
                LogInternalAudio(((("KISMETSOUND:Could not create AC for " @ string(ASound)) @ "Actor[") @ string(SourceActor)) @ "]");
            }
        }        
    }
    else
    {
        LogInternalAudio("KISMETSOUND:Couldn't find sourceactor");
    }
    //return;    
}

reliable client simulated event Kismet_ClientKeyOffSound(SoundCue ASound, Actor SourceActor)
{
    local AudioComponent CheckAC;
    local bool KeyOff;

    // End:0x16
    if(SourceActor == none)
    {
        SourceActor = WorldInfo;
    }
    // End:0x65
    foreach SourceActor.AllOwnedComponents(Class'AudioComponent', CheckAC)
    {
        // End:0x64
        if(CheckAC.SoundCue == ASound)
        {
            CheckAC.KeyOff();
            KeyOff = true;
        }        
    }    
    // End:0x105
    if(KeyOff == false)
    {
        LogInternal("#######################################");
        LogInternal("##### Script::Key off not triggered" @ string(CheckAC.SoundCue));
        LogInternal("#######################################");
    }
    //return;    
}

reliable client simulated event Kismet_ClientSetParameterSound(SoundCue ASound, Actor SourceActor, name nameToSet, float valueToSet)
{
    local AudioComponent CheckAC;

    // End:0x16
    if(SourceActor == none)
    {
        SourceActor = WorldInfo;
    }
    // End:0x67
    foreach SourceActor.AllOwnedComponents(Class'AudioComponent', CheckAC)
    {
        // End:0x66
        if(CheckAC.SoundCue == ASound)
        {
            CheckAC.SetFloatParameter(nameToSet, valueToSet);
        }        
    }    
    //return;    
}

reliable client simulated event Kismet_ClientStopSound(SoundCue ASound, Actor SourceActor, float FadeOutTime)
{
    local AudioComponent CheckAC;

    // End:0x16
    if(SourceActor == none)
    {
        SourceActor = WorldInfo;
    }
    // End:0x5D
    foreach SourceActor.AllOwnedComponents(Class'AudioComponent', CheckAC)
    {
        // End:0x5C
        if(CheckAC.SoundCue == ASound)
        {
            CheckAC.Stop();
        }        
    }    
    //return;    
}

reliable client simulated function ClientPlayActorFaceFXAnim(Actor SourceActor, FaceFXAnimSet AnimSet, string GroupName, string SeqName, SoundCue SoundCueToPlay)
{
    // End:0x33
    if(SourceActor != none)
    {
        SourceActor.PlayActorFaceFXAnim(AnimSet, GroupName, SeqName, SoundCueToPlay);
    }
    //return;    
}

reliable client simulated event ClientMessage(coerce string S, optional name Type, optional float MsgLifeTime)
{
    // End:0x35
    if((WorldInfo.NetMode == NM_DedicatedServer) || WorldInfo.GRI == none)
    {
        return;
    }
    // End:0x57
    if(Type == 'None')
    {
        Type = 'Event';
    }
    TeamMessage(PlayerReplicationInfo, S, Type, MsgLifeTime);
    //return;    
}

private final simulated function bool CanCommunicate()
{
    return true;
    //return ReturnValue;    
}

private final simulated function bool AllowTTSMessageFrom(PlayerReplicationInfo PRI)
{
    return true;
    //return ReturnValue;    
}

// Export UPlayerController::execCreateTTSSoundCue(FFrame&, void* const)
private native final simulated function SoundCue CreateTTSSoundCue(string StrToSpeak, PlayerReplicationInfo PRI);

exec function Talk()
{
    local Console PlayerConsole;
    local LocalPlayer LP;

    LP = LocalPlayer(Player);
    // End:0x85
    if(((LP != none) && CanCommunicate()) && LP.ViewportClient.ViewportConsole != none)
    {
        PlayerConsole = LocalPlayer(Player).ViewportClient.ViewportConsole;
        PlayerConsole.StartTyping("Say ");
    }
    //return;    
}

exec function TeamTalk()
{
    local Console PlayerConsole;
    local LocalPlayer LP;

    LP = LocalPlayer(Player);
    // End:0x89
    if(((LP != none) && CanCommunicate()) && LP.ViewportClient.ViewportConsole != none)
    {
        PlayerConsole = LocalPlayer(Player).ViewportClient.ViewportConsole;
        PlayerConsole.StartTyping("TeamSay ");
    }
    //return;    
}

simulated function SpeakTTS(coerce string S, optional PlayerReplicationInfo PRI)
{
    local SoundCue Cue;
    local AudioComponent AC;

    Cue = CreateTTSSoundCue(S, PRI);
    // End:0x6C
    if(Cue != none)
    {
        AC = CreateAudioComponent(Cue, false, true,,, true);
        AC.bAllowSpatialization = false;
        AC.bAutoDestroy = true;
        AC.Play();
    }
    //return;    
}

event AudioComponent PlayUICue(SoundCue Cue)
{
    local AudioComponent AC;

    // End:0x6D
    if(Cue != none)
    {
        AC = CreateAudioComponent(Cue, false, true,,, true);
        AC.bAllowSpatialization = false;
        AC.bAutoDestroy = true;
        AC.bIsUISound = true;
        AC.Play();
        return AC;
    }
    return none;
    //return ReturnValue;    
}

reliable client simulated event TeamMessage(PlayerReplicationInfo PRI, coerce string S, name Type, optional float MsgLifeTime)
{
    local bool bIsUserCreated;

    // End:0x17C
    if(CanCommunicate())
    {
        // End:0x88
        if((((Type == 'Say') || Type == 'TeamSay') && PRI != none) && AllowTTSMessageFrom(PRI))
        {
            // End:0x88
            if(!bIsUserCreated || bIsUserCreated && CanViewUserCreatedContent())
            {
                SpeakTTS(S, PRI);
            }
        }
        // End:0xBB
        if(myHUD != none)
        {
            myHUD.Message(PRI, S, Type, MsgLifeTime);
        }
        // End:0x11A
        if(((Type == 'Say') || Type == 'TeamSay') && PRI != none)
        {
            S = (PRI.PlayerName $ ": ") $ S;
            bIsUserCreated = true;
        }
        // End:0x17C
        if(Player != none)
        {
            // End:0x17C
            if(!bIsUserCreated || bIsUserCreated && CanViewUserCreatedContent())
            {
                LocalPlayer(Player).ViewportClient.ViewportConsole.OutputText(S);
            }
        }
    }
    //return;    
}

function PlayBeepSound()
{
    //return;    
}

function RegisterOnlineDelegates()
{
    // End:0x10E
    if(OnlineSub != none)
    {
        VoiceInterface = OnlineSub.VoiceInterface;
        // End:0xA0
        if(NotEqual_InterfaceInterface(OnlineSub.SystemInterface, none) && LocalPlayer(Player) != none)
        {
            OnlineSub.SystemInterface.AddExternalUIChangeDelegate(OnExternalUIChanged);
            OnlineSub.SystemInterface.AddControllerChangeDelegate(OnControllerChanged);
        }
        // End:0x10E
        if(NotEqual_InterfaceInterface(OnlineSub.GameInterface, none) && LocalPlayer(Player) != none)
        {
            OnlineSub.GameInterface.AddGameInviteAcceptedDelegate(byte(LocalPlayer(Player).ControllerId), OnGameInviteAccepted);
        }
    }
    //return;    
}

event ClearOnlineDelegates()
{
    local LocalPlayer LP;

    LogInternal((((("Clearing online delegates for" @ string(self)) @ "(") $ "Player:") $ ((Player != none) ? string(Player.Name) : "None")) $ ")");
    LP = LocalPlayer(Player);
    // End:0x16C
    if((Role < ROLE_Authority) || LP != none)
    {
        // End:0x16C
        if(OnlineSub != none)
        {
            // End:0x108
            if(NotEqual_InterfaceInterface(OnlineSub.SystemInterface, none))
            {
                OnlineSub.SystemInterface.ClearExternalUIChangeDelegate(OnExternalUIChanged);
                OnlineSub.SystemInterface.ClearControllerChangeDelegate(OnControllerChanged);
            }
            // End:0x16C
            if(NotEqual_InterfaceInterface(OnlineSub.GameInterface, none) && LP != none)
            {
                OnlineSub.GameInterface.ClearGameInviteAcceptedDelegate(byte(LP.ControllerId), OnGameInviteAccepted);
            }
        }
    }
    //return;    
}

event Destroyed()
{
    local Vehicle DrivenVehicle;

    ClientPlayForceFeedbackWaveform(none);
    // End:0x37
    if((Role < ROLE_Authority) || LocalPlayer(Player) != none)
    {
        ClearOnlineDelegates();
    }
    // End:0x9A
    if(Pawn != none)
    {
        DrivenVehicle = Vehicle(Pawn);
        // End:0x60
        if(DrivenVehicle != none)
        {            
        }
        else
        {
            Pawn.Health = 0;
            Pawn.Died(self, Class'DmgType_Suicided', Pawn.Location);
        }
    }
    // End:0xB2
    if(myHUD != none)
    {
        myHUD.Destroy();
    }
    // End:0xD1
    if(PlayerCamera != none)
    {
        PlayerCamera.Destroy();
        PlayerCamera = none;
    }
    ForceClearUnpauseDelegates();
    UnregisterPlayerDataStores();
    super.Destroyed();
    //return;    
}

function FixFOV()
{
    FOVAngle = default.DefaultFOV;
    DesiredFOV = default.DefaultFOV;
    DefaultFOV = default.DefaultFOV;
    //return;    
}

function SetFOV(float NewFOV)
{
    DesiredFOV = NewFOV;
    FOVAngle = NewFOV;
    //return;    
}

function ResetFOV()
{
    DesiredFOV = DefaultFOV;
    FOVAngle = DefaultFOV;
    //return;    
}

exec function FOV(float F)
{
    // End:0x26
    if(PlayerCamera != none)
    {
        PlayerCamera.SetFOV(F);
        return;
    }
    // End:0x88
    if(((F >= 80.0000000) || WorldInfo.NetMode == NM_Standalone) || PlayerReplicationInfo.bOnlySpectator)
    {
        DefaultFOV = FClamp(F, 80.0000000, 100.0000000);
        DesiredFOV = DefaultFOV;
    }
    //return;    
}

exec function Mutate(string MutateString)
{
    ServerMutate(MutateString);
    //return;    
}

reliable server function ServerMutate(string MutateString)
{
    // End:0x1C
    if(WorldInfo.NetMode == NM_Client)
    {
        return;
    }
    WorldInfo.Game.Mutate(MutateString, self);
    //return;    
}

function bool AllowTextMessage(string msg)
{
    local int I;

    // End:0x31
    if((WorldInfo.NetMode == NM_Standalone) || PlayerReplicationInfo.bAdmin)
    {
        return true;
    }
    // End:0x69
    if((WorldInfo.Pauser == none) && (WorldInfo.TimeSeconds - LastBroadcastTime) < float(2))
    {
        return false;
    }
    // End:0xDA
    if((WorldInfo.TimeSeconds - LastBroadcastTime) < float(5))
    {
        msg = Left(msg, Clamp(Len(msg) - 4, 8, 64));
        I = 0;
        J0xAD:

        // End:0xDA [Loop If]
        if(I < 4)
        {
            // End:0xD0
            if(LastBroadcastString[I] ~= msg)
            {
                return false;
            }
            I++;
            // [Loop Continue]
            goto J0xAD;
        }
    }
    I = 3;
    J0xE2:

    // End:0x111 [Loop If]
    if(I > 0)
    {
        LastBroadcastString[I] = LastBroadcastString[I - 1];
        I--;
        // [Loop Continue]
        goto J0xE2;
    }
    LastBroadcastTime = WorldInfo.TimeSeconds;
    return true;
    //return ReturnValue;    
}

exec function Say(string msg)
{
    msg = Left(msg, 128);
    // End:0x30
    if(AllowTextMessage(msg))
    {
        ServerSay(msg);
    }
    //return;    
}

unreliable server function ServerSay(string msg)
{
    local PlayerController PC;

    // End:0xA6
    if(PlayerReplicationInfo.bAdmin && Left(msg, 1) == "#")
    {
        msg = Right(msg, Len(msg) - 1);
        // End:0xA3
        foreach WorldInfo.AllControllers(Class'PlayerController', PC)
        {
            PC.ClearProgressMessages();
            PC.SetProgressTime(6.0000000);
            PC.SetProgressMessage(2, msg);            
        }        
        return;
    }
    WorldInfo.Game.Broadcast(self, msg, 'Say');
    //return;    
}

exec function TeamSay(string msg)
{
    msg = Left(msg, 128);
    // End:0x30
    if(AllowTextMessage(msg))
    {
        ServerTeamSay(msg);
    }
    //return;    
}

unreliable server function ServerTeamSay(string msg)
{
    LastActiveTime = WorldInfo.TimeSeconds;
    // End:0x4F
    if(!WorldInfo.GRI.GameClass.default.bTeamGame)
    {
        Say(msg);
        return;
    }
    WorldInfo.Game.BroadcastTeam(self, WorldInfo.Game.ParseMessageString(self, msg), 'TeamSay');
    //return;    
}

event PreClientTravel(string PendingURL, Actor.ETravelType TravelType, bool bIsSeamlessTravel)
{
    local UIInteraction UIController;
    local GameUISceneClient GameSceneClient;

    UIController = GetUIController();
    // End:0x6B
    if((UIController != none) && IsPrimaryPlayer())
    {
        GameSceneClient = UIController.SceneClient;
        // End:0x6B
        if(GameSceneClient != none)
        {
            GameSceneClient.NotifyClientTravel(self, PendingURL, TravelType, bIsSeamlessTravel);
        }
    }
    //return;    
}

exec function Camera(name NewMode)
{
    ServerCamera(NewMode);
    //return;    
}

reliable server function ServerCamera(name NewMode)
{
    // End:0x25
    if(NewMode == '1st')
    {
        NewMode = 'FirstPerson';        
    }
    else
    {
        // End:0x47
        if(NewMode == '3rd')
        {
            NewMode = 'ThirdPerson';
        }
    }
    SetCameraMode(NewMode);
    // End:0x7E
    if(PlayerCamera != none)
    {
        LogInternal("#### " $ string(PlayerCamera.CameraStyle));
    }
    //return;    
}

reliable client simulated function ClientSetCameraMode(name NewCamMode)
{
    // End:0x74
    if(PlayerCamera != none)
    {
        // End:0x35
        if(NewCamMode == 'nextvision')
        {
            PlayerCamera.NextVisionMode();            
        }
        else
        {
            // End:0x5F
            if(NewCamMode == 'nextview')
            {
                PlayerCamera.NextViewMode();                
            }
            else
            {
                PlayerCamera.CameraStyle = NewCamMode;
            }
        }
    }
    //return;    
}

function SetCameraMode(name NewCamMode)
{
    // End:0x86
    if((PlayerCamera != none) && IsLocalPlayerController())
    {
        // End:0x44
        if(NewCamMode == 'nextvision')
        {
            PlayerCamera.NextVisionMode();            
        }
        else
        {
            // End:0x6E
            if(NewCamMode == 'nextview')
            {
                PlayerCamera.NextViewMode();                
            }
            else
            {
                PlayerCamera.CameraStyle = NewCamMode;
            }
        }        
    }
    else
    {
        // End:0xA5
        if(Role == ROLE_Authority)
        {
            ClientSetCameraMode(NewCamMode);
        }
    }
    //return;    
}

event ResetCameraMode()
{
    // End:0x2D
    if(Pawn != none)
    {
        SetCameraMode(Pawn.GetDefaultCameraMode(self));        
    }
    else
    {
        SetCameraMode('FirstPerson');
    }
    //return;    
}

reliable client simulated event ClientSetCameraFade(bool bEnableFading, optional Color FadeColor, optional Vector2D FadeAlpha, optional float FadeTime)
{
    // End:0x8C
    if(PlayerCamera != none)
    {
        PlayerCamera.bEnableFading = bEnableFading;
        // End:0x8C
        if(PlayerCamera.bEnableFading)
        {
            PlayerCamera.FadeColor = FadeColor;
            PlayerCamera.FadeAlpha = FadeAlpha;
            PlayerCamera.FadeTime = FadeTime;
            PlayerCamera.FadeTimeRemaining = FadeTime;
        }
    }
    //return;    
}

function bool UsingFirstPersonCamera()
{
    return ((PlayerCamera == none) || PlayerCamera.CameraStyle == 'FirstPerson') && LocalPlayer(Player) != none;
    //return ReturnValue;    
}

function ClientVoiceMessage(PlayerReplicationInfo Sender, PlayerReplicationInfo Recipient, name MessageType, byte messageID)
{
    //return;    
}

function ForceDeathUpdate()
{
    LastUpdateTime = WorldInfo.TimeSeconds - float(10);
    //return;    
}

unreliable server function DualServerMove(float TimeStamp0, Vector InAccel0, byte PendingFlags, int View0, float TimeStamp, Vector InAccel, Vector ClientLoc, byte NewFlags, byte ClientRoll, int View)
{
    ServerMove(TimeStamp0, InAccel0, vect(1.0000000, 2.0000000, 3.0000000), PendingFlags, ClientRoll, View0);
    ServerMove(TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, View);
    //return;    
}

unreliable server function OldServerMove(float OldTimeStamp, byte OldAccelX, byte OldAccelY, byte OldAccelZ, byte OldMoveFlags)
{
    local Vector Accel;

    // End:0x11
    if(AcknowledgedPawn != Pawn)
    {
        return;
    }
    // End:0x1A0
    if(CurrentTimeStamp < (OldTimeStamp - 0.0010000))
    {
        Accel.X = float(OldAccelX);
        // End:0x86
        if(Accel.X > float(127))
        {
            Accel.X = -1.0000000 * (Accel.X - float(128));
        }
        Accel.Y = float(OldAccelY);
        // End:0xE5
        if(Accel.Y > float(127))
        {
            Accel.Y = -1.0000000 * (Accel.Y - float(128));
        }
        Accel.Z = float(OldAccelZ);
        // End:0x144
        if(Accel.Z > float(127))
        {
            Accel.Z = -1.0000000 * (Accel.Z - float(128));
        }
        Accel *= float(20);
        OldTimeStamp = FMin(OldTimeStamp, CurrentTimeStamp + MaxResponseTime);
        MoveAutonomous(OldTimeStamp - CurrentTimeStamp, OldMoveFlags, Accel, rot(0, 0, 0));
        CurrentTimeStamp = OldTimeStamp;
    }
    //return;    
}

unreliable server function ServerMove(float TimeStamp, Vector InAccel, Vector ClientLoc, byte MoveFlags, byte ClientRoll, int View)
{
    local float DeltaTime, clientErr;
    local Rotator DeltaRot, Rot, ViewRot;
    local Vector Accel, LocDiff;
    local int maxPitch, ViewPitch, ViewYaw;

    // End:0x11
    if(CurrentTimeStamp >= TimeStamp)
    {
        return;
    }
    // End:0x42
    if(AcknowledgedPawn != Pawn)
    {
        InAccel = vect(0.0000000, 0.0000000, 0.0000000);
        GivePawn(Pawn);
    }
    ViewPitch = View & 65535;
    ViewYaw = View >> 16;
    Accel = InAccel * 0.1000000;
    DeltaTime = FMin(MaxResponseTime, TimeStamp - CurrentTimeStamp);
    // End:0xAE
    if(Pawn == none)
    {
        bWasSpeedHack = false;
        ResetTimeMargin();        
    }
    else
    {
        // End:0x18A
        if(!CheckSpeedHack(DeltaTime))
        {
            // End:0x157
            if(!bWasSpeedHack)
            {
                // End:0x12A
                if((WorldInfo.TimeSeconds - LastSpeedHackLog) > float(20))
                {
                    LogInternal("Possible speed hack by " $ PlayerReplicationInfo.PlayerName);
                    LastSpeedHackLog = WorldInfo.TimeSeconds;
                }
                ClientMessage("Speed Hack Detected!", 'CriticalEvent');                
            }
            else
            {
                bWasSpeedHack = true;
            }
            DeltaTime = 0.0000000;
            Pawn.Velocity = vect(0.0000000, 0.0000000, 0.0000000);            
        }
        else
        {
            DeltaTime *= Pawn.CustomTimeDilation;
            bWasSpeedHack = false;
        }
    }
    CurrentTimeStamp = TimeStamp;
    ServerTimeStamp = WorldInfo.TimeSeconds;
    ViewRot.Pitch = ViewPitch;
    ViewRot.Yaw = ViewYaw;
    ViewRot.Roll = 0;
    // End:0x232
    if(InAccel != vect(0.0000000, 0.0000000, 0.0000000))
    {
        LastActiveTime = WorldInfo.TimeSeconds;
    }
    SetRotation(ViewRot);
    // End:0x24B
    if(AcknowledgedPawn != Pawn)
    {
        return;
    }
    // End:0x3CD
    if(Pawn != none)
    {
        Rot.Roll = 256 * int(ClientRoll);
        Rot.Yaw = ViewYaw;
        // End:0x2CC
        if((Pawn.Physics == 3) || Pawn.Physics == 4)
        {
            maxPitch = 2;            
        }
        else
        {
            maxPitch = 0;
        }
        // End:0x386
        if((ViewPitch > (maxPitch * Pawn.MaxPitchLimit)) && ViewPitch < (65536 - (maxPitch * Pawn.MaxPitchLimit)))
        {
            // End:0x355
            if(ViewPitch < 32768)
            {
                Rot.Pitch = maxPitch * Pawn.MaxPitchLimit;                
            }
            else
            {
                Rot.Pitch = 65536 - (maxPitch * Pawn.MaxPitchLimit);
            }            
        }
        else
        {
            Rot.Pitch = ViewPitch;
        }
        DeltaRot = Rotation - Rot;
        Pawn.FaceRotation(Rot, DeltaTime);
    }
    // End:0x40F
    if((WorldInfo.Pauser == none) && DeltaTime > float(0))
    {
        MoveAutonomous(DeltaTime, MoveFlags, Accel, DeltaRot);
    }
    // End:0x42B
    if(ClientLoc == vect(1.0000000, 2.0000000, 3.0000000))
    {
        return;        
    }
    else
    {
        // End:0x460
        if((WorldInfo.TimeSeconds - LastUpdateTime) < (180.0000000 / float(Player.CurrentNetSpeed)))
        {
            return;
        }
    }
    // End:0x480
    if(Pawn == none)
    {
        LocDiff = Location - ClientLoc;        
    }
    else
    {
        // End:0x4A9
        if(Pawn.bForceRMVelocity)
        {
            LocDiff = vect(0.0000000, 0.0000000, 0.0000000);            
        }
        else
        {
            LocDiff = Pawn.Location - ClientLoc;
        }
    }
    clientErr = LocDiff Dot LocDiff;
    // End:0x68B
    if(clientErr > 3.0000000)
    {
        // End:0x536
        if(Pawn == none)
        {
            PendingAdjustment.newPhysics = Physics;
            PendingAdjustment.NewLoc = Location;
            PendingAdjustment.NewVel = Velocity;            
        }
        else
        {
            PendingAdjustment.newPhysics = Pawn.Physics;
            PendingAdjustment.NewVel = Pawn.Velocity;
            PendingAdjustment.NewBase = Pawn.Base;
            // End:0x60A
            if((InterpActor(Pawn.Base) != none) || Vehicle(Pawn.Base) != none)
            {
                PendingAdjustment.NewLoc = Pawn.Location - Pawn.Base.Location;                
            }
            else
            {
                PendingAdjustment.NewLoc = Pawn.Location;
            }
            PendingAdjustment.NewFloor = Pawn.Floor;
        }
        LastUpdateTime = WorldInfo.TimeSeconds;
        PendingAdjustment.TimeStamp = TimeStamp;
        PendingAdjustment.bAckGoodMove = 0;        
    }
    else
    {
        PendingAdjustment.TimeStamp = TimeStamp;
        PendingAdjustment.bAckGoodMove = 1;
    }
    //return;    
}

event SendClientAdjustment()
{
    // End:0x27
    if(AcknowledgedPawn != Pawn)
    {
        PendingAdjustment.TimeStamp = 0.0000000;
        return;
    }
    // End:0x5F
    if(PendingAdjustment.bAckGoodMove == 1)
    {
        ClientAckGoodMove(PendingAdjustment.TimeStamp);        
    }
    else
    {
        // End:0x2D3
        if((Pawn == none) || Pawn.Physics != 8)
        {
            // End:0x1F1
            if(PendingAdjustment.NewVel == vect(0.0000000, 0.0000000, 0.0000000))
            {
                // End:0x160
                if(((GetStateName() == 'PlayerWalking') && Pawn != none) && Pawn.Physics == 1)
                {
                    VeryShortClientAdjustPosition(PendingAdjustment.TimeStamp, PendingAdjustment.NewLoc.X, PendingAdjustment.NewLoc.Y, PendingAdjustment.NewLoc.Z, PendingAdjustment.NewBase);                    
                }
                else
                {
                    ShortClientAdjustPosition(PendingAdjustment.TimeStamp, GetStateName(), PendingAdjustment.newPhysics, PendingAdjustment.NewLoc.X, PendingAdjustment.NewLoc.Y, PendingAdjustment.NewLoc.Z, PendingAdjustment.NewBase);
                }                
            }
            else
            {
                ClientAdjustPosition(PendingAdjustment.TimeStamp, GetStateName(), PendingAdjustment.newPhysics, PendingAdjustment.NewLoc.X, PendingAdjustment.NewLoc.Y, PendingAdjustment.NewLoc.Z, PendingAdjustment.NewVel.X, PendingAdjustment.NewVel.Y, PendingAdjustment.NewVel.Z, PendingAdjustment.NewBase);
            }            
        }
        else
        {
            LongClientAdjustPosition(PendingAdjustment.TimeStamp, GetStateName(), PendingAdjustment.newPhysics, PendingAdjustment.NewLoc.X, PendingAdjustment.NewLoc.Y, PendingAdjustment.NewLoc.Z, PendingAdjustment.NewVel.X, PendingAdjustment.NewVel.Y, PendingAdjustment.NewVel.Z, PendingAdjustment.NewBase, PendingAdjustment.NewFloor.X, PendingAdjustment.NewFloor.Y, PendingAdjustment.NewFloor.Z);
        }
    }
    PendingAdjustment.TimeStamp = 0.0000000;
    PendingAdjustment.bAckGoodMove = 0;
    //return;    
}

unreliable server function ServerDrive(float InForward, float InStrafe, float aUp, bool InJump, int View)
{
    local Rotator ViewRotation;

    ViewRotation.Pitch = View & 65535;
    ViewRotation.Yaw = View >> 16;
    ViewRotation.Roll = 0;
    SetRotation(ViewRotation);
    ProcessDrive(InForward, InStrafe, aUp, InJump);
    //return;    
}

function ProcessDrive(float InForward, float InStrafe, float InUp, bool InJump)
{
    ClientGotoState(GetStateName(), 'Begin');
    //return;    
}

function ProcessMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
{
    // End:0x3B
    if((Pawn != none) && Pawn.Acceleration != newAccel)
    {
        Pawn.Acceleration = newAccel;
    }
    //return;    
}

function MoveAutonomous(float DeltaTime, byte CompressedFlags, Vector newAccel, Rotator DeltaRot)
{
    local Actor.EDoubleClickDir DoubleClickMove;

    // End:0x22
    if((Pawn != none) && Pawn.bHardAttach)
    {
        return;
    }
    DoubleClickMove = SavedMoveClass.static.SetFlags(CompressedFlags, self);
    HandleWalking();
    ProcessMove(DeltaTime, newAccel, DoubleClickMove, DeltaRot);
    // End:0x8A
    if(Pawn != none)
    {
        Pawn.AutonomousPhysics(DeltaTime);        
    }
    else
    {
        AutonomousPhysics(DeltaTime);
    }
    bDoubleJump = false;
    //return;    
}

unreliable client simulated function VeryShortClientAdjustPosition(float TimeStamp, float NewLocX, float NewLocY, float NewLocZ, Actor NewBase)
{
    local Vector Floor;

    // End:0x20
    if(Pawn != none)
    {
        Floor = Pawn.Floor;
    }
    LongClientAdjustPosition(TimeStamp, 'PlayerWalking', 1, NewLocX, NewLocY, NewLocZ, 0.0000000, 0.0000000, 0.0000000, NewBase, Floor.X, Floor.Y, Floor.Z);
    //return;    
}

unreliable client simulated function ShortClientAdjustPosition(float TimeStamp, name NewState, Actor.EPhysics newPhysics, float NewLocX, float NewLocY, float NewLocZ, Actor NewBase)
{
    local Vector Floor;

    // End:0x20
    if(Pawn != none)
    {
        Floor = Pawn.Floor;
    }
    LongClientAdjustPosition(TimeStamp, NewState, newPhysics, NewLocX, NewLocY, NewLocZ, 0.0000000, 0.0000000, 0.0000000, NewBase, Floor.X, Floor.Y, Floor.Z);
    //return;    
}

reliable client simulated function ClientCapBandwidth(int Cap)
{
    ClientCap = Cap;
    // End:0x3C
    if((Player != none) && Player.CurrentNetSpeed > Cap)
    {
        SetNetSpeed(Cap);
    }
    //return;    
}

unreliable client simulated function ClientAckGoodMove(float TimeStamp)
{
    UpdatePing(TimeStamp);
    CurrentTimeStamp = TimeStamp;
    ClearAckedMoves();
    //return;    
}

unreliable client simulated function ClientAdjustPosition(float TimeStamp, name NewState, Actor.EPhysics newPhysics, float NewLocX, float NewLocY, float NewLocZ, float NewVelX, float NewVelY, float NewVelZ, Actor NewBase)
{
    local Vector Floor;

    // End:0x20
    if(Pawn != none)
    {
        Floor = Pawn.Floor;
    }
    LongClientAdjustPosition(TimeStamp, NewState, newPhysics, NewLocX, NewLocY, NewLocZ, NewVelX, NewVelY, NewVelZ, NewBase, Floor.X, Floor.Y, Floor.Z);
    //return;    
}

reliable server function ServerSetNetSpeed(int NewSpeed)
{
    // End:0x57
    if((WorldInfo.Game != none) && WorldInfo.NetMode == NM_ListenServer)
    {
        NewSpeed = Min(NewSpeed, WorldInfo.Game.AdjustedNetSpeed);
    }
    SetNetSpeed(NewSpeed);
    //return;    
}

final function UpdatePing(float TimeStamp)
{
    // End:0x8D
    if(PlayerReplicationInfo != none)
    {
        PlayerReplicationInfo.UpdatePing(TimeStamp);
        // End:0x8D
        if((WorldInfo.TimeSeconds - LastPingUpdate) > float(4))
        {
            OldPing = PlayerReplicationInfo.ExactPing;
            LastPingUpdate = WorldInfo.TimeSeconds;
            ServerUpdatePing(int(float(1000) * PlayerReplicationInfo.ExactPing));
        }
    }
    //return;    
}

unreliable client simulated function LongClientAdjustPosition(float TimeStamp, name NewState, Actor.EPhysics newPhysics, float NewLocX, float NewLocY, float NewLocZ, float NewVelX, float NewVelY, float NewVelZ, Actor NewBase, float NewFloorX, float NewFloorY, float NewFloorZ)
{
    local Vector NewLocation, NewVelocity, NewFloor;
    local Actor MoveActor;
    local SavedMove CurrentMove;
    local Actor TheViewTarget;

    UpdatePing(TimeStamp);
    // End:0xE5
    if(Pawn != none)
    {
        // End:0x63
        if(Pawn.bTearOff)
        {
            Pawn = none;
            // End:0x61
            if(!GamePlayEndedState() && !IsInState('Dead'))
            {
                GotoState('Dead');
            }
            return;
        }
        MoveActor = Pawn;
        TheViewTarget = GetViewTarget();
        // End:0xE2
        if((TheViewTarget != Pawn) && (TheViewTarget == self) || (Pawn(TheViewTarget) != none) && Pawn(TheViewTarget).Health <= 0)
        {
            ResetCameraMode();
            SetViewTarget(Pawn);
        }        
    }
    else
    {
        MoveActor = self;
        // End:0x1AF
        if(GetStateName() != NewState)
        {
            LogInternal((("- state change:" @ string(GetStateName())) @ "->") @ string(NewState), 'PlayerMove');
            // End:0x14B
            if(NewState == 'RoundEnded')
            {
                GotoState(NewState);                
            }
            else
            {
                // End:0x192
                if(IsInState('Dead'))
                {
                    // End:0x18D
                    if((NewState != 'PlayerWalking') && NewState != 'PlayerSwimming')
                    {
                        GotoState(NewState);
                    }
                    return;                    
                }
                else
                {
                    // End:0x1AF
                    if(NewState == 'Dead')
                    {
                        GotoState(NewState);
                    }
                }
            }
        }
    }
    // End:0x1C0
    if(CurrentTimeStamp >= TimeStamp)
    {
        return;
    }
    CurrentTimeStamp = TimeStamp;
    NewLocation.X = NewLocX;
    NewLocation.Y = NewLocY;
    NewLocation.Z = NewLocZ;
    NewVelocity.X = NewVelX;
    NewVelocity.Y = NewVelY;
    NewVelocity.Z = NewVelZ;
    CurrentMove = SavedMoves;
    J0x25A:

    // End:0x513 [Loop If]
    if(CurrentMove != none)
    {
        // End:0x509
        if(CurrentMove.TimeStamp <= CurrentTimeStamp)
        {
            SavedMoves = CurrentMove.NextMove;
            CurrentMove.NextMove = FreeMoves;
            FreeMoves = CurrentMove;
            // End:0x4E7
            if(CurrentMove.TimeStamp == CurrentTimeStamp)
            {
                LastAckedAccel = CurrentMove.Acceleration;
                FreeMoves.Clear();
                // End:0x42D
                if(((InterpActor(NewBase) != none) || Vehicle(NewBase) != none) && NewBase == CurrentMove.EndBase)
                {
                    // End:0x42A
                    if(((GetStateName() == NewState) && IsInState('PlayerWalking')) && (MoveActor.Physics == 1) || MoveActor.Physics == 2)
                    {
                        // End:0x3BB
                        if(VSizeSq(CurrentMove.SavedRelativeLocation - NewLocation) < 3.0000000)
                        {
                            CurrentMove = none;
                            return;                            
                        }
                        else
                        {
                            // End:0x42A
                            if((((Vehicle(NewBase) != none) && VSizeSq(Velocity) < 9.0000000) && VSizeSq(NewVelocity) < 9.0000000) && VSizeSq(CurrentMove.SavedRelativeLocation - NewLocation) < 900.0000000)
                            {
                                CurrentMove = none;
                                return;
                            }
                        }
                    }                    
                }
                else
                {
                    // End:0x4DD
                    if(((((VSizeSq(CurrentMove.SavedLocation - NewLocation) < 3.0000000) && VSizeSq(CurrentMove.SavedVelocity - NewVelocity) < 9.0000000) && GetStateName() == NewState) && IsInState('PlayerWalking')) && (MoveActor.Physics == 1) || MoveActor.Physics == 2)
                    {
                        CurrentMove = none;
                        return;
                    }
                }
                CurrentMove = none;                
            }
            else
            {
                FreeMoves.Clear();
                CurrentMove = SavedMoves;
            }            
        }
        else
        {
            CurrentMove = none;
        }
        // [Loop Continue]
        goto J0x25A;
    }
    // End:0x58B
    if(MoveActor.bHardAttach)
    {
        // End:0x589
        if(MoveActor.Base == none)
        {
            // End:0x55B
            if(NewBase != none)
            {
                MoveActor.SetBase(NewBase);
            }
            // End:0x584
            if(MoveActor.Base == none)
            {
                MoveActor.SetHardAttach(false);                
            }
            else
            {
                return;
            }            
        }
        else
        {
            return;
        }
    }
    NewFloor.X = NewFloorX;
    NewFloor.Y = NewFloorY;
    NewFloor.Z = NewFloorZ;
    // End:0x61F
    if(MoveActor.Base != NewBase)
    {
        LogInternal(("- base mismatch:" @ string(MoveActor.Base)) @ string(NewBase), 'PlayerMove');
    }
    // End:0x67C
    if(MoveActor.Location != NewLocation)
    {
        LogInternal("- location mismatch, delta:" @ string(VSize(MoveActor.Location - NewLocation)), 'PlayerMove');
    }
    // End:0x70F
    if(MoveActor.Velocity != NewVelocity)
    {
        LogInternal((((("- velocity mismatch, delta:" @ string(VSize(NewVelocity - MoveActor.Velocity))) @ "client:") @ string(VSize(MoveActor.Velocity))) @ "server:") @ string(VSize(NewVelocity)), 'PlayerMove');
    }
    CurrentMove = SavedMoves;
    J0x71A:

    // End:0x792 [Loop If]
    if(CurrentMove != none)
    {
        // End:0x77A
        if(CurrentMove.bForceRMVelocity)
        {
            LogInternal("- skipping position update for upcoming root motion", 'PlayerMove');
            return;
        }
        CurrentMove = CurrentMove.NextMove;
        // [Loop Continue]
        goto J0x71A;
    }
    // End:0x7CA
    if((InterpActor(NewBase) != none) || Vehicle(NewBase) != none)
    {
        NewLocation += NewBase.Location;
    }
    MoveActor.bCanTeleport = false;
    // End:0x935
    if(((((!MoveActor.SetLocation(NewLocation) && Pawn(MoveActor) != none) && Pawn(MoveActor).CylinderComponent.CollisionHeight > Pawn(MoveActor).CrouchHeight) && !Pawn(MoveActor).bIsCrouched) && newPhysics == 1) && MoveActor.Physics != 10)
    {
        MoveActor.SetPhysics(newPhysics);
        // End:0x905
        if(!MoveActor.SetLocation(NewLocation + (vect(0.0000000, 0.0000000, 1.0000000) * Pawn(MoveActor).MaxStepHeight)))
        {
            Pawn(MoveActor).ForceCrouch();
            MoveActor.SetLocation(NewLocation);            
        }
        else
        {
            MoveActor.MoveSmooth(vect(0.0000000, 0.0000000, -1.0000000) * Pawn(MoveActor).MaxStepHeight);
        }
    }
    MoveActor.bCanTeleport = true;
    // End:0x986
    if((MoveActor.Physics != 10) && newPhysics != 10)
    {
        MoveActor.SetPhysics(newPhysics);
    }
    // End:0x9AA
    if(MoveActor != self)
    {
        MoveActor.SetBase(NewBase, NewFloor);
    }
    MoveActor.Velocity = NewVelocity;
    UpdateStateFromAdjustment(NewState);
    bUpdatePosition = true;
    //return;    
}

function UpdateStateFromAdjustment(name NewState)
{
    // End:0x17
    if(GetStateName() != NewState)
    {
        GotoState(NewState);
    }
    //return;    
}

unreliable server function ServerUpdatePing(int NewPing)
{
    PlayerReplicationInfo.Ping = byte(Min(int(0.2500000 * float(NewPing)), 250));
    //return;    
}

function ClearAckedMoves()
{
    local SavedMove CurrentMove;

    CurrentMove = SavedMoves;
    J0x0B:

    // End:0xBA [Loop If]
    if(CurrentMove != none)
    {
        // End:0xB4
        if(CurrentMove.TimeStamp <= CurrentTimeStamp)
        {
            // End:0x5D
            if(CurrentMove.TimeStamp == CurrentTimeStamp)
            {
                LastAckedAccel = CurrentMove.Acceleration;
            }
            SavedMoves = CurrentMove.NextMove;
            CurrentMove.NextMove = FreeMoves;
            FreeMoves = CurrentMove;
            FreeMoves.Clear();
            CurrentMove = SavedMoves;            
        }
        else
        {
            // [Explicit Break]
            goto J0xBA;
        }
        // [Loop Continue]
        goto J0x0B;
    }
    J0xBA:

    //return;    
}

function ClientUpdatePosition()
{
    local SavedMove CurrentMove;
    local int realbRun, realbDuck;
    local bool bRealJump, bRealPreciseDestination;

    bUpdatePosition = false;
    // End:0x31
    if((Pawn != none) && Pawn.Physics == 10)
    {
        return;
    }
    realbRun = int(bRun);
    realbDuck = int(bDuck);
    bRealJump = bPressedJump;
    bUpdating = true;
    bRealPreciseDestination = bPreciseDestination;
    ClearAckedMoves();
    CurrentMove = SavedMoves;
    J0x82:

    // End:0x239 [Loop If]
    if(CurrentMove != none)
    {
        // End:0xC2
        if((PendingMove == CurrentMove) && Pawn != none)
        {
            PendingMove.SetInitialPosition(Pawn);
        }
        // End:0xEE
        if(Pawn != none)
        {
            Pawn.bForceRMVelocity = CurrentMove.bForceRMVelocity;
        }
        MoveAutonomous(CurrentMove.Delta, CurrentMove.CompressedFlags(), CurrentMove.Acceleration, rot(0, 0, 0));
        // End:0x221
        if(Pawn != none)
        {
            Pawn.bForceRMVelocity = false;
            CurrentMove.SavedLocation = Pawn.Location;
            CurrentMove.SavedVelocity = Pawn.Velocity;
            CurrentMove.EndBase = Pawn.Base;
            // End:0x221
            if((CurrentMove.EndBase != none) && !CurrentMove.EndBase.bWorldGeometry)
            {
                CurrentMove.SavedRelativeLocation = Pawn.Location - CurrentMove.EndBase.Location;
            }
        }
        CurrentMove = CurrentMove.NextMove;
        // [Loop Continue]
        goto J0x82;
    }
    bUpdating = false;
    bDuck = byte(realbDuck);
    bRun = byte(realbRun);
    bPressedJump = bRealJump;
    bPreciseDestination = bRealPreciseDestination;
    //return;    
}

final function SavedMove GetFreeMove()
{
    local SavedMove S, first;
    local int I;

    // End:0x10E
    if(FreeMoves == none)
    {
        S = SavedMoves;
        J0x16:

        // End:0x100 [Loop If]
        if(S != none)
        {
            I++;
            // End:0xE8
            if(I > 100)
            {
                first = SavedMoves;
                SavedMoves = SavedMoves.NextMove;
                first.Clear();
                first.NextMove = none;
                J0x79:

                // End:0xDB [Loop If]
                if(SavedMoves != none)
                {
                    S = SavedMoves;
                    SavedMoves = SavedMoves.NextMove;
                    S.Clear();
                    S.NextMove = FreeMoves;
                    FreeMoves = S;
                    // [Loop Continue]
                    goto J0x79;
                }
                PendingMove = none;
                return first;
            }
            S = S.NextMove;
            // [Loop Continue]
            goto J0x16;
        }
        return new (self) SavedMoveClass;        
    }
    else
    {
        S = FreeMoves;
        FreeMoves = FreeMoves.NextMove;
        S.NextMove = none;
        return S;
    }
    //return ReturnValue;    
}

function int CompressAccel(int C)
{
    // End:0x1D
    if(C >= 0)
    {
        C = Min(C, 127);        
    }
    else
    {
        C = Min(int(Abs(float(C))), 127) + 128;
    }
    return C;
    //return ReturnValue;    
}

function ReplicateMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
{
    local SavedMove NewMove, OldMove, AlmostLastMove, LastMove;
    local byte ClientRoll;
    local float NetMoveDelta;

    // End:0x0D
    if(Player == none)
    {
        return;
    }
    MaxResponseTime = default.MaxResponseTime * WorldInfo.TimeDilation;
    DeltaTime = ((Pawn != none) ? Pawn.CustomTimeDilation : CustomTimeDilation) * FMin(DeltaTime, MaxResponseTime);
    // End:0xFF
    if(SavedMoves != none)
    {
        LastMove = SavedMoves;
        AlmostLastMove = LastMove;
        OldMove = none;
        J0x86:

        // End:0xFF [Loop If]
        if(LastMove.NextMove != none)
        {
            // End:0xDC
            if(((OldMove == none) && Pawn != none) && LastMove.IsImportantMove(LastAckedAccel))
            {
                OldMove = LastMove;
            }
            AlmostLastMove = LastMove;
            LastMove = LastMove.NextMove;
            // [Loop Continue]
            goto J0x86;
        }
    }
    NewMove = GetFreeMove();
    // End:0x118
    if(NewMove == none)
    {
        return;
    }
    NewMove.SetMoveFor(self, DeltaTime, newAccel, DoubleClickMove);
    bDoubleJump = false;
    ProcessMove(NewMove.Delta, NewMove.Acceleration, NewMove.DoubleClickMove, DeltaRot);
    // End:0x337
    if((PendingMove != none) && PendingMove.CanCombineWith(NewMove, Pawn, MaxResponseTime))
    {
        Pawn.SetLocation(PendingMove.GetStartLocation());
        Pawn.Velocity = PendingMove.StartVelocity;
        // End:0x235
        if(PendingMove.StartBase != Pawn.Base)
        {
            Pawn.SetBase(PendingMove.StartBase);
        }
        Pawn.Floor = PendingMove.StartFloor;
        NewMove.Delta += PendingMove.Delta;
        NewMove.SetInitialPosition(Pawn);
        // End:0x330
        if(LastMove == PendingMove)
        {
            // End:0x2D5
            if(SavedMoves == PendingMove)
            {
                SavedMoves.NextMove = FreeMoves;
                FreeMoves = SavedMoves;
                SavedMoves = none;                
            }
            else
            {
                PendingMove.NextMove = FreeMoves;
                FreeMoves = PendingMove;
                // End:0x31C
                if(AlmostLastMove != none)
                {
                    AlmostLastMove.NextMove = none;
                    LastMove = AlmostLastMove;
                }
            }
            FreeMoves.Clear();
        }
        PendingMove = none;
    }
    // End:0x361
    if(Pawn != none)
    {
        Pawn.AutonomousPhysics(NewMove.Delta);        
    }
    else
    {
        AutonomousPhysics(DeltaTime);
    }
    NewMove.PostUpdate(self);
    // End:0x397
    if(SavedMoves == none)
    {
        SavedMoves = NewMove;        
    }
    else
    {
        LastMove.NextMove = NewMove;
    }
    // End:0x48C
    if(PendingMove == none)
    {
        // End:0x418
        if(((Player.CurrentNetSpeed > 10000) && WorldInfo.GRI != none) && WorldInfo.GRI.PRIArray.Length <= 10)
        {
            NetMoveDelta = 0.0110000;            
        }
        else
        {
            NetMoveDelta = FMax(0.0222000, (2.0000000 * WorldInfo.MoveRepSize) / float(Player.CurrentNetSpeed));
        }
        // End:0x48C
        if(((WorldInfo.TimeSeconds - ClientUpdateTime) * WorldInfo.TimeDilation) < NetMoveDelta)
        {
            PendingMove = NewMove;
            return;
        }
    }
    ClientUpdateTime = WorldInfo.TimeSeconds;
    ClientRoll = byte((Rotation.Roll >> 8) & 255);
    CallServerMove(NewMove, ((Pawn == none) ? Location : Pawn.Location), ClientRoll, ((Rotation.Yaw & 65535) << 16) + (Rotation.Pitch & 65535), OldMove);
    PendingMove = none;
    //return;    
}

function CallServerMove(SavedMove NewMove, Vector ClientLoc, byte ClientRoll, int View, SavedMove OldMove)
{
    local Vector BuildAccel;
    local byte OldAccelX, OldAccelY, OldAccelZ;

    // End:0xDE
    if(OldMove != none)
    {
        BuildAccel = (0.0500000 * OldMove.Acceleration) + vect(0.5000000, 0.5000000, 0.5000000);
        OldAccelX = byte(CompressAccel(int(BuildAccel.X)));
        OldAccelY = byte(CompressAccel(int(BuildAccel.Y)));
        OldAccelZ = byte(CompressAccel(int(BuildAccel.Z)));
        OldServerMove(OldMove.TimeStamp, OldAccelX, OldAccelY, OldAccelZ, OldMove.CompressedFlags());
    }
    // End:0x1BD
    if(PendingMove != none)
    {
        DualServerMove(PendingMove.TimeStamp, PendingMove.Acceleration * float(10), PendingMove.CompressedFlags(), ((PendingMove.Rotation.Yaw & 65535) << 16) + (PendingMove.Rotation.Pitch & 65535), NewMove.TimeStamp, NewMove.Acceleration * float(10), ClientLoc, NewMove.CompressedFlags(), ClientRoll, View);        
    }
    else
    {
        ServerMove(NewMove.TimeStamp, NewMove.Acceleration * float(10), ClientLoc, NewMove.CompressedFlags(), ClientRoll, View);
    }
    //return;    
}

function HandleWalking()
{
    // End:0x2C
    if(Pawn != none)
    {
        Pawn.SetWalking(bRun != 0);
    }
    //return;    
}

reliable server function ServerRestartGame()
{
    //return;    
}

exec function Speech(name Type, int Index, string Callsign)
{
    ServerSpeech(Type, Index, Callsign);
    //return;    
}

reliable server function ServerSpeech(name Type, int Index, string Callsign)
{
    //return;    
}

exec function RestartLevel()
{
    // End:0x32
    if(WorldInfo.NetMode == NM_Standalone)
    {
        ClientTravel("?restart", TRAVEL_Relative);
    }
    //return;    
}

exec function LocalTravel(string URL)
{
    // End:0x2D
    if(WorldInfo.NetMode == NM_Standalone)
    {
        ClientTravel(URL, 2);
    }
    //return;    
}

exec function QuickSave()
{
    // End:0x7D
    if(((Pawn != none) && Pawn.Health > 0) && WorldInfo.NetMode == NM_Standalone)
    {
        ClientMessage(QuickSaveString);        
        ConsoleCommand("DEFER SAVEGAME QUICKSAVE.SAV");
    }
    //return;    
}

exec function QuickLoad()
{
    // End:0x48
    if(WorldInfo.NetMode == NM_Standalone)
    {        
        ConsoleCommand("DEFER LOADGAME QUICKSAVE.SAV");
    }
    //return;    
}

function PauseRumbleForAllPlayers(optional bool bShouldPauseRumble = true)
{
    local PlayerController PC;

    // End:0x52
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        // End:0x51
        if(PC.ForceFeedbackManager != none)
        {
            PC.ForceFeedbackManager.PauseWaveform(bShouldPauseRumble);
        }        
    }    
    //return;    
}

delegate bool CanUnpause()
{
    return WorldInfo.Pauser == PlayerReplicationInfo;
    //return ReturnValue;    
}

function bool SetPause(bool bPause, optional delegate<CanUnpause> CanUnpauseDelegate = CanUnpause)
{
    local bool bResult;

    // End:0xB8
    if(WorldInfo.NetMode != NM_Client)
    {
        // End:0x7A
        if(bPause)
        {
            bFire = 0;
            bResult = WorldInfo.Game.SetPause(self, CanUnpauseDelegate);
            // End:0x77
            if(bResult)
            {
                PauseRumbleForAllPlayers();
            }            
        }
        else
        {
            WorldInfo.Game.ClearPause();
            // End:0xB8
            if(WorldInfo.Pauser == none)
            {
                PauseRumbleForAllPlayers(false);
            }
        }
    }
    return bResult;
    //return ReturnValue;    
}

exec function DebugPause()
{
    WorldInfo.Game.DebugPause();
    //return;    
}

final simulated function bool IsPaused()
{
    return WorldInfo.Pauser != none;
    //return ReturnValue;    
}

exec function Pause()
{
    ServerPause();
    //return;    
}

reliable server function ServerPause()
{
    // End:0x1A
    if(!IsPaused())
    {
        SetPause(true);        
    }
    else
    {
        SetPause(false);
    }
    //return;    
}

exec function ShowMenu()
{
    //return;    
}

event ConditionalPause(bool bDesiredPauseState)
{
    // End:0x22
    if(bDesiredPauseState != IsPaused())
    {
        SetPause(bDesiredPauseState);
    }
    //return;    
}

reliable server function ServerUTrace()
{
    // End:0x40
    if((WorldInfo.NetMode != NM_Standalone) && (PlayerReplicationInfo == none) || !PlayerReplicationInfo.bAdmin)
    {
        return;
    }
    UTrace();
    //return;    
}

exec function UTrace()
{
    ConsoleCommand("hidelog");
    // End:0x33
    if(Role != ROLE_Authority)
    {
        ServerUTrace();
    }
    SetUTracing(!IsUTracing());
    LogInternal((("UTracing changed to " $ string(IsUTracing())) $ " at ") $ string(WorldInfo.TimeSeconds), 'UTrace');
    //return;    
}

exec function ThrowWeapon()
{
    // End:0x24
    if((Pawn == none) || Pawn.Weapon == none)
    {
        return;
    }
    ServerThrowWeapon();
    //return;    
}

reliable server function ServerThrowWeapon()
{
    // End:0x2C
    if(Pawn.CanThrowWeapon())
    {
        Pawn.ThrowActiveWeapon();
    }
    //return;    
}

exec function PrevWeapon()
{
    // End:0x17
    if(WorldInfo.Pauser != none)
    {
        return;
    }
    // End:0x39
    if(Pawn.Weapon == none)
    {
        SwitchToBestWeapon();
        return;
    }
    // End:0x6C
    if(Pawn.InvManager != none)
    {
        Pawn.InvManager.PrevWeapon();
    }
    //return;    
}

exec function NextWeapon()
{
    // End:0x17
    if(WorldInfo.Pauser != none)
    {
        return;
    }
    // End:0x39
    if(Pawn.Weapon == none)
    {
        SwitchToBestWeapon();
        return;
    }
    // End:0x6C
    if(Pawn.InvManager != none)
    {
        Pawn.InvManager.NextWeapon();
    }
    //return;    
}

exec function StartFire(optional byte FireModeNum)
{
    // End:0x28
    if(WorldInfo.Pauser == PlayerReplicationInfo)
    {
        SetPause(false);
        return;
    }
    // End:0x59
    if((Pawn != none) && !bCinematicMode)
    {
        Pawn.StartFire(FireModeNum);
    }
    //return;    
}

exec function StopFire(optional byte FireModeNum)
{
    // End:0x25
    if(Pawn != none)
    {
        Pawn.StopFire(FireModeNum);
    }
    //return;    
}

exec function StartAltFire(optional byte FireModeNum)
{
    StartFire(1);
    //return;    
}

exec function StopAltFire(optional byte FireModeNum)
{
    StopFire(1);
    //return;    
}

function GetTriggerUseList(float interactDistanceToCheck, float crosshairDist, float minDot, bool bUsuableOnly, out array<Trigger> out_useList)
{
    local int Idx;
    local Vector cameraLoc;
    local Rotator cameraRot;
    local Trigger checkTrigger;
    local SeqEvent_Used UseSeq;

    // End:0x1E1
    if(Pawn != none)
    {
        GetPlayerViewPoint(cameraLoc, cameraRot);
        // End:0x1E0
        foreach Pawn.CollidingActors(Class'Trigger', checkTrigger, interactDistanceToCheck)
        {
            Idx = 0;
            J0x47:

            // End:0x1DF [Loop If]
            if(Idx < checkTrigger.GeneratedEvents.Length)
            {
                UseSeq = SeqEvent_Used(checkTrigger.GeneratedEvents[Idx]);
                // End:0x1D5
                if((((UseSeq != none) && !bUsuableOnly || checkTrigger.GeneratedEvents[Idx].CheckActivate(checkTrigger, Pawn, true)) && (Normal(checkTrigger.Location - cameraLoc) Dot Vector(cameraRot)) >= minDot) && ((UseSeq.bAimToInteract && IsAimingAt(checkTrigger, 0.9800000)) && VSize(Pawn.Location - checkTrigger.Location) <= UseSeq.InteractDistance) || !UseSeq.bAimToInteract && VSize(Pawn.Location - checkTrigger.Location) <= UseSeq.InteractDistance)
                {
                    out_useList[out_useList.Length] = checkTrigger;
                    Idx = checkTrigger.GeneratedEvents.Length;
                }
                Idx++;
                // [Loop Continue]
                goto J0x47;
            }            
        }        
    }
    //return;    
}

exec function Use()
{
    // End:0x1A
    if(Role < ROLE_Authority)
    {
        PerformedUseAction();
    }
    ServerUse();
    //return;    
}

unreliable server function ServerUse()
{
    PerformedUseAction();
    //return;    
}

function bool PerformedUseAction()
{
    // End:0x37
    if(WorldInfo.Pauser == PlayerReplicationInfo)
    {
        // End:0x35
        if(Role == ROLE_Authority)
        {
            SetPause(false);
        }
        return true;
    }
    // End:0x5B
    if((Pawn == none) || !Pawn.bCanUse)
    {
        return true;
    }
    // End:0x6D
    if(Role < ROLE_Authority)
    {
        return false;
    }
    return TriggerInteracted();
    //return ReturnValue;    
}

function bool TriggerInteracted()
{
    local Actor A;
    local int Idx;
    local float Weight;
    local bool bInserted;
    local Vector cameraLoc;
    local Rotator cameraRot;
    local array<Trigger> useList;
    local array<Actor> sortedList;
    local array<float> weightList;

    // End:0x1E7
    if(Pawn != none)
    {
        GetTriggerUseList(InteractDistance, 60.0000000, 0.0000000, true, useList);
        // End:0x1E7
        if(useList.Length > 0)
        {
            GetPlayerViewPoint(cameraLoc, cameraRot);
            J0x4A:

            // End:0x1A2 [Loop If]
            if(useList.Length > 0)
            {
                A = useList[useList.Length - 1];
                useList.Length = useList.Length - 1;
                Weight = Normal(A.Location - cameraLoc) Dot Vector(cameraRot);
                Weight += (1.0000000 - (VSize(A.Location - Pawn.Location) / InteractDistance));
                bInserted = false;
                Idx = 0;
                J0xE8:

                // End:0x166 [Loop If]
                if((Idx < sortedList.Length) && !bInserted)
                {
                    // End:0x15C
                    if(weightList[Idx] < Weight)
                    {
                        sortedList.Insert(Idx, 1);
                        weightList.Insert(Idx, 1);
                        sortedList[Idx] = A;
                        weightList[Idx] = Weight;
                        bInserted = true;
                    }
                    Idx++;
                    // [Loop Continue]
                    goto J0xE8;
                }
                // End:0x19F
                if(!bInserted)
                {
                    Idx = sortedList.Length;
                    sortedList[Idx] = A;
                    weightList[Idx] = Weight;
                }
                // [Loop Continue]
                goto J0x4A;
            }
            Idx = 0;
            J0x1A9:

            // End:0x1E7 [Loop If]
            if(Idx < sortedList.Length)
            {
                // End:0x1DD
                if(sortedList[Idx].UsedBy(Pawn))
                {
                    return true;
                }
                Idx++;
                // [Loop Continue]
                goto J0x1A9;
            }
        }
    }
    return false;
    //return ReturnValue;    
}

exec function Suicide()
{
    ServerSuicide();
    //return;    
}

reliable server function ServerSuicide()
{
    // End:0x66
    if((Pawn != none) && ((WorldInfo.TimeSeconds - Pawn.LastStartTime) > float(10)) || WorldInfo.NetMode == NM_Standalone)
    {
        Pawn.Suicide();
    }
    //return;    
}

exec function SetName(coerce string S)
{
    local string NewName;
    local LocalPlayer LocPlayer;

    // End:0x134
    if(S != "")
    {
        LocPlayer = LocalPlayer(Player);
        // End:0x108
        if(((LocPlayer != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none)) && NotEqual_InterfaceInterface(OnlineSub.PlayerInterface, none))
        {
            // End:0x108
            if((OnlineSub.PlayerInterface.GetLoginStatus(byte(LocPlayer.ControllerId)) == 2) && OnlineSub.GameInterface.GetGameSettings('Game') != none)
            {
                S = OnlineSub.PlayerInterface.GetPlayerNickname(byte(LocPlayer.ControllerId));
            }
        }
        NewName = S;
        ServerChangeName(NewName);
        UpdateURL("Name", NewName, true);
        SaveConfig();
    }
    //return;    
}

reliable server function ServerChangeName(coerce string S)
{
    // End:0x31
    if(S != "")
    {
        WorldInfo.Game.ChangeName(self, S, true);
    }
    //return;    
}

exec function SwitchTeam()
{
    // End:0x44
    if((PlayerReplicationInfo.Team == none) || PlayerReplicationInfo.Team.TeamIndex == 1)
    {
        ServerChangeTeam(0);        
    }
    else
    {
        ServerChangeTeam(1);
    }
    //return;    
}

exec function ChangeTeam(optional string TeamName)
{
    local int N;

    // End:0x1B
    if(TeamName ~= "blue")
    {
        N = 1;        
    }
    else
    {
        // End:0x79
        if((((TeamName ~= "red") || PlayerReplicationInfo == none) || PlayerReplicationInfo.Team == none) || PlayerReplicationInfo.Team.TeamIndex > 1)
        {
            N = 0;            
        }
        else
        {
            N = 1 - PlayerReplicationInfo.Team.TeamIndex;
        }
    }
    ServerChangeTeam(N);
    //return;    
}

reliable server function ServerChangeTeam(int N)
{
    local TeamInfo OldTeam;

    OldTeam = PlayerReplicationInfo.Team;
    WorldInfo.Game.ChangeTeam(self, N, true);
    // End:0x91
    if(WorldInfo.Game.bTeamGame && PlayerReplicationInfo.Team != OldTeam)
    {
        // End:0x91
        if(Pawn != none)
        {
            Pawn.PlayerChangedTeam();
        }
    }
    //return;    
}

exec function SwitchLevel(string URL)
{
    // End:0x51
    if((WorldInfo.NetMode == NM_Standalone) || WorldInfo.NetMode == NM_ListenServer)
    {
        WorldInfo.ServerTravel(URL);
    }
    //return;    
}

exec function ClearProgressMessages()
{
    ClientClearProgressMessages();
    //return;    
}

reliable client simulated function ClientClearProgressMessages()
{
    local int I;

    I = 0;
    J0x07:

    // End:0x2B [Loop If]
    if(I < 2)
    {
        ProgressMessage[I] = "";
        I++;
        // [Loop Continue]
        goto J0x07;
    }
    //return;    
}

exec event SetProgressMessage(PlayerController.EProgressMessageType MessageType, string Message, optional string Title)
{
    ClientSetProgressMessage(MessageType, Message, Title);
    //return;    
}

reliable client simulated function ClientSetProgressMessage(PlayerController.EProgressMessageType MessageType, string Message, optional string Title, optional bool bIgnoreFutureNetworkMessages)
{
    // End:0x1F
    if(MessageType == 0)
    {
        ClientClearProgressMessages();        
    }
    else
    {
        // End:0x46
        if(MessageType == 4)
        {
            NotifyConnectionError(Message, Title);            
        }
        else
        {
            // End:0x99
            if(MessageType != 5)
            {
                // End:0x7F
                if(Title != "")
                {
                    ProgressMessage[0] = Title;
                    ProgressMessage[1] = Message;                    
                }
                else
                {
                    ProgressMessage[1] = "";
                    ProgressMessage[0] = Message;
                }                
            }
            else
            {
                // End:0xC8
                if(MessageType == 5)
                {
                    // End:0xC8
                    if(!bIgnoreNetworkMessages)
                    {
                        NotifyConnectionError(Message, Title);
                    }
                }
            }
        }
    }
    // End:0xE0
    if(!bIgnoreNetworkMessages)
    {
        bIgnoreNetworkMessages = bIgnoreFutureNetworkMessages;
    }
    //return;    
}

exec event SetProgressTime(float T)
{
    ClientSetProgressTime(T);
    //return;    
}

reliable client simulated function ClientSetProgressTime(float T)
{
    ProgressTimeOut = T + WorldInfo.TimeSeconds;
    //return;    
}

function Restart(bool bVehicleTransition)
{
    super.Restart(bVehicleTransition);
    ServerTimeStamp = 0.0000000;
    ResetTimeMargin();
    EnterStartState();
    ClientRestart(Pawn);
    // End:0x55
    if(ControllingDirTrackInst == none)
    {
        SetViewTarget(Pawn);
    }
    ResetCameraMode();
    //return;    
}

// Export UPlayerController::execServerNotifyLoadedWorld(FFrame&, void* const)
reliable server native final event ServerNotifyLoadedWorld(name WorldPackageName);

event NotifyLoadedWorld(name WorldPackageName, bool bFinalDest)
{
    local PlayerStart P;
    local Rotator SpawnRotation;

    SetViewTarget(self);
    // End:0x72
    foreach WorldInfo.AllNavigationPoints(Class'PlayerStart', P)
    {
        SetLocation(P.Location);
        SpawnRotation.Yaw = P.Rotation.Yaw;
        SetRotation(SpawnRotation);
        // End:0x72
        break;        
    }    
    //return;    
}

// Export UPlayerController::execHasClientLoadedCurrentWorld(FFrame&, void* const)
native final function bool HasClientLoadedCurrentWorld();

function EnterStartState()
{
    local name NewState;

    // End:0x71
    if(Pawn.PhysicsVolume.bWaterVolume)
    {
        // End:0x59
        if(Pawn.HeadVolume.bWaterVolume)
        {
            Pawn.BreathTime = Pawn.UnderWaterTime;
        }
        NewState = Pawn.WaterMovementState;        
    }
    else
    {
        NewState = Pawn.LandMovementState;
    }
    // End:0xA4
    if(IsInState(NewState))
    {
        BeginState(NewState);        
    }
    else
    {
        GotoState(NewState);
    }
    //return;    
}

reliable client simulated function ClientRestart(Pawn NewPawn)
{
    ResetPlayerMovementInput();
    CleanOutSavedMoves();
    Pawn = NewPawn;
    // End:0x50
    if((Pawn != none) && Pawn.bTearOff)
    {
        UnPossess();
        Pawn = none;
    }
    AcknowledgePossession(Pawn);
    // End:0x7A
    if(Pawn == none)
    {
        GotoState('WaitingForPawn');
        return;
    }
    Pawn.ClientRestart();
    // End:0xC2
    if(Role < ROLE_Authority)
    {
        SetViewTarget(Pawn);
        ResetCameraMode();
        EnterStartState();
    }
    CleanOutSavedMoves();
    //return;    
}

function GameHasEnded(optional Actor EndGameFocus, optional bool bIsWinner)
{
    SetViewTarget(EndGameFocus);
    GotoState('RoundEnded');
    ClientGameEnded(EndGameFocus, bIsWinner);
    //return;    
}

reliable client simulated function ClientGameEnded(Actor EndGameFocus, bool bIsWinner)
{
    SetViewTarget(EndGameFocus);
    GotoState('RoundEnded');
    //return;    
}

function NotifyChangedWeapon(Weapon PreviousWeapon, Weapon NewWeapon)
{
    //return;    
}

event PlayerTick(float DeltaTime)
{
    // End:0x1D
    if(!bShortConnectTimeOut)
    {
        bShortConnectTimeOut = true;
        ServerShortTimeout();
    }
    // End:0x7E
    if(Pawn != AcknowledgedPawn)
    {
        // End:0x6F
        if(Role < ROLE_Authority)
        {
            // End:0x6F
            if((AcknowledgedPawn != none) && AcknowledgedPawn.Controller == self)
            {
                AcknowledgedPawn.Controller = none;
            }
        }
        AcknowledgePossession(Pawn);
    }
    PlayerInput.PlayerInput(DeltaTime);
    // End:0xAA
    if(bUpdatePosition)
    {
        ClientUpdatePosition();
    }
    PlayerMove(DeltaTime);
    AdjustFOV(DeltaTime);
    //return;    
}

function PlayerMove(float DeltaTime)
{
    //return;    
}

function bool AimingHelp(bool bInstantHit)
{
    return (WorldInfo.NetMode == NM_Standalone) && bAimingHelp;
    //return ReturnValue;    
}

event CameraLookAtFinished(SeqAct_CameraLookAt Action)
{
    //return;    
}

function Rotator GetAdjustedAimFor(Weapon W, Vector StartFireLoc)
{
    local Vector FireDir, AimSpot, HitLocation, HitNormal, OldAim, AimOffset;

    local Actor BestTarget, HitActor;
    local float bestAim, bestDist;
    local bool bNoZAdjust, bInstantHit;
    local Rotator BaseAimRot, AimRot;

    bInstantHit = (W == none) || W.bInstantHit;
    BaseAimRot = ((Pawn != none) ? Pawn.GetBaseAimRotation() : Rotation);
    FireDir = Vector(BaseAimRot);
    HitActor = Trace(HitLocation, HitNormal, StartFireLoc + (W.GetTraceRange() * FireDir), StartFireLoc, true);
    // End:0x104
    if((HitActor != none) && HitActor.bProjTarget)
    {
        BestTarget = HitActor;
        bNoZAdjust = true;
        OldAim = HitLocation;
        bestDist = VSize(BestTarget.Location - Pawn.Location);        
    }
    else
    {
        bestAim = 0.9000000;
        // End:0x13B
        if(AimingHelp(bInstantHit))
        {
            bestAim = AimHelpDot(bInstantHit);            
        }
        else
        {
            // End:0x14F
            if(bInstantHit)
            {
                bestAim = 1.0000000;
            }
        }
        BestTarget = PickTarget(Class'Pawn', bestAim, bestDist, FireDir, StartFireLoc, W.WeaponRange);
        // End:0x191
        if(BestTarget == none)
        {
            return BaseAimRot;
        }
        OldAim = StartFireLoc + (FireDir * bestDist);
    }
    ShotTarget = Pawn(BestTarget);
    // End:0x1D5
    if(!AimingHelp(bInstantHit))
    {
        return BaseAimRot;
    }
    FireDir = BestTarget.Location - StartFireLoc;
    AimSpot = StartFireLoc + (bestDist * Normal(FireDir));
    AimOffset = AimSpot - OldAim;
    // End:0x30E
    if(ShotTarget != none)
    {
        // End:0x256
        if(bNoZAdjust)
        {
            AimSpot.Z = OldAim.Z;            
        }
        else
        {
            // End:0x2BE
            if(AimOffset.Z < float(0))
            {
                AimSpot.Z = ShotTarget.Location.Z + (0.4000000 * ShotTarget.CylinderComponent.CollisionHeight);                
            }
            else
            {
                AimSpot.Z = ShotTarget.Location.Z - (0.7000000 * ShotTarget.CylinderComponent.CollisionHeight);
            }
        }        
    }
    else
    {
        AimSpot.Z = OldAim.Z;
    }
    // End:0x3CB
    if(!bNoZAdjust)
    {
        AimRot = Rotator(AimSpot - StartFireLoc);
        // End:0x394
        if(FOVAngle < (DefaultFOV - float(8)))
        {
            AimRot.Yaw = (AimRot.Yaw + 200) - Rand(400);            
        }
        else
        {
            AimRot.Yaw = (AimRot.Yaw + 375) - Rand(750);
        }
        return AimRot;
    }
    return Rotator(AimSpot - StartFireLoc);
    //return ReturnValue;    
}

function float AimHelpDot(bool bInstantHit)
{
    // End:0x1B
    if(FOVAngle < (DefaultFOV - float(8)))
    {
        return 0.9900000;
    }
    // End:0x2A
    if(bInstantHit)
    {
        return 0.9700000;
    }
    return 0.9300000;
    //return ReturnValue;    
}

event bool NotifyLanded(Vector HitNormal, Actor FloorActor)
{
    return bUpdating;
    //return ReturnValue;    
}

function AdjustFOV(float DeltaTime)
{
    // End:0x9F
    if(FOVAngle != DesiredFOV)
    {
        // End:0x4F
        if(FOVAngle > DesiredFOV)
        {
            FOVAngle = FOVAngle - FMax(7.0000000, (0.9000000 * DeltaTime) * (FOVAngle - DesiredFOV));            
        }
        else
        {
            FOVAngle = FOVAngle - FMin(-7.0000000, (0.9000000 * DeltaTime) * (FOVAngle - DesiredFOV));
        }
        // End:0x9F
        if(Abs(FOVAngle - DesiredFOV) <= float(10))
        {
            FOVAngle = DesiredFOV;
        }
    }
    //return;    
}

event float GetFOVAngle()
{
    return ((PlayerCamera != none) ? PlayerCamera.GetFOVAngle() : FOVAngle);
    //return ReturnValue;    
}

event float GetZoomMagnification()
{
    return 1.0000000;
    //return ReturnValue;    
}

// Export UPlayerController::execIsLocalPlayerController(FFrame&, void* const)
native function bool IsLocalPlayerController();

// Export UPlayerController::execSetViewTarget(FFrame&, void* const)
native function SetViewTarget(Actor NewViewTarget, optional ViewTargetTransitionParams TransitionParams);

reliable client simulated event ClientSetViewTarget(Actor A, optional ViewTargetTransitionParams TransitionParams)
{
    // End:0x35
    if(!bClientSimulatingViewTarget)
    {
        // End:0x21
        if(A == none)
        {
            ServerVerifyViewTarget();
        }
        SetViewTarget(A, TransitionParams);
    }
    //return;    
}

// Export UPlayerController::execGetViewTarget(FFrame&, void* const)
native function Actor GetViewTarget();

reliable server function ServerVerifyViewTarget()
{
    local Actor TheViewTarget;

    TheViewTarget = GetViewTarget();
    // End:0x1D
    if(TheViewTarget == self)
    {
        return;
    }
    ClientSetViewTarget(TheViewTarget);
    //return;    
}

event SpawnPlayerCamera()
{
    // End:0x84
    if((CameraClass != none) && IsLocalPlayerController())
    {
        PlayerCamera = Spawn(CameraClass, self);
        // End:0x54
        if(PlayerCamera != none)
        {
            PlayerCamera.InitializeFor(self);            
        }
        else
        {
            LogInternal("Couldn't Spawn Camera Actor for Player!!");
        }        
    }
    //return;    
}

simulated event GetPlayerViewPoint(out Vector out_Location, out Rotator out_Rotation)
{
    local Actor TheViewTarget;

    // End:0x7D
    if(PlayerCamera == none)
    {
        // End:0x7D
        if(CameraClass != none)
        {
            PlayerCamera = Spawn(CameraClass, self);
            // End:0x50
            if(PlayerCamera != none)
            {
                PlayerCamera.InitializeFor(self);                
            }
            else
            {
                LogInternal("Couldn't Spawn Camera Actor for Player!!");
            }
        }
    }
    // End:0xA5
    if(PlayerCamera != none)
    {
        PlayerCamera.GetCameraViewPoint(out_Location, out_Rotation);        
    }
    else
    {
        TheViewTarget = GetViewTarget();
        // End:0xED
        if(TheViewTarget != none)
        {
            out_Location = TheViewTarget.Location;
            out_Rotation = TheViewTarget.Rotation;            
        }
        else
        {
            super.GetPlayerViewPoint(out_Location, out_Rotation);
        }
    }
    //return;    
}

function ViewShake(float DeltaTime)
{
    //return;    
}

function UpdateRotation(float DeltaTime)
{
    local Rotator DeltaRot, NewRotation, ViewRotation;

    ViewRotation = Rotation;
    DesiredRotation = ViewRotation;
    DeltaRot.Yaw = int(PlayerInput.aTurn);
    DeltaRot.Pitch = int(PlayerInput.aLookUp);
    ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
    SetRotation(ViewRotation);
    ViewShake(DeltaTime);
    NewRotation = ViewRotation;
    NewRotation.Roll = Rotation.Roll;
    // End:0xDF
    if(Pawn != none)
    {
        Pawn.FaceRotation(NewRotation, DeltaTime);
    }
    //return;    
}

function ProcessViewRotation(float DeltaTime, out Rotator out_ViewRotation, Rotator DeltaRot)
{
    // End:0x2E
    if(PlayerCamera != none)
    {
        PlayerCamera.ProcessViewRotation(DeltaTime, out_ViewRotation, DeltaRot);
    }
    // End:0x5F
    if(Pawn != none)
    {
        Pawn.ProcessViewRotation(DeltaTime, out_ViewRotation, DeltaRot);        
    }
    else
    {
        out_ViewRotation += DeltaRot;
        out_ViewRotation = LimitViewRotation(out_ViewRotation, -16384.0000000, 16383.0000000);
    }
    //return;    
}

event Rotator LimitViewRotation(Rotator ViewRotation, float ViewPitchMin, float ViewPitchMax)
{
    ViewRotation.Pitch = ViewRotation.Pitch & 65535;
    // End:0xC1
    if((float(ViewRotation.Pitch) > ViewPitchMax) && float(ViewRotation.Pitch) < (float(65535) + ViewPitchMin))
    {
        // End:0xA0
        if(ViewRotation.Pitch < 32768)
        {
            ViewRotation.Pitch = int(ViewPitchMax);            
        }
        else
        {
            ViewRotation.Pitch = int(float(65535) + ViewPitchMin);
        }
    }
    return ViewRotation;
    //return ReturnValue;    
}

function ClearDoubleClick()
{
    // End:0x20
    if(PlayerInput != none)
    {
        PlayerInput.DoubleClickTimer = 0.0000000;
    }
    //return;    
}

function CheckJumpOrDuck()
{
    // End:0x30
    if(bPressedJump && Pawn != none)
    {
        Pawn.DoJump(bUpdating);
    }
    //return;    
}

function bool IsSpectating()
{
    return false;
    //return ReturnValue;    
}

unreliable server function ServerSetSpectatorLocation(Vector NewLoc)
{
    ClientGotoState(GetStateName());
    //return;    
}

unreliable server function ServerViewNextPlayer()
{
    // End:0x18
    if(IsSpectating())
    {
        ViewAPlayer(1);
    }
    //return;    
}

unreliable server function ServerViewPrevPlayer()
{
    // End:0x1C
    if(IsSpectating())
    {
        ViewAPlayer(-1);
    }
    //return;    
}

function ViewAPlayer(int Dir)
{
    local int I, CurrentIndex, NewIndex;
    local PlayerReplicationInfo PRI;
    local bool bSuccess;

    CurrentIndex = -1;
    // End:0x82
    if(RealViewTarget != none)
    {
        I = 0;
        J0x1D:

        // End:0x82 [Loop If]
        if(I < WorldInfo.GRI.PRIArray.Length)
        {
            // End:0x78
            if(RealViewTarget == WorldInfo.GRI.PRIArray[I])
            {
                CurrentIndex = I;
                // [Explicit Break]
                goto J0x82;
            }
            I++;
            // [Loop Continue]
            goto J0x1D;
        }
    }
    J0x82:

    NewIndex = CurrentIndex + Dir;
    J0x94:

    // End:0x180 [Loop If]
    if((NewIndex >= 0) && NewIndex < WorldInfo.GRI.PRIArray.Length)
    {
        PRI = WorldInfo.GRI.PRIArray[NewIndex];
        // End:0x16B
        if((((PRI != none) && Controller(PRI.Owner) != none) && Controller(PRI.Owner).Pawn != none) && WorldInfo.Game.CanSpectate(self, PRI))
        {
            bSuccess = true;
            // [Explicit Break]
            goto J0x180;
        }
        NewIndex = NewIndex + Dir;
        // [Loop Continue]
        goto J0x94;
    }
    J0x180:

    // End:0x2BB
    if(!bSuccess)
    {
        CurrentIndex = ((NewIndex < 0) ? WorldInfo.GRI.PRIArray.Length : -1);
        NewIndex = CurrentIndex + Dir;
        J0x1CF:

        // End:0x2BB [Loop If]
        if((NewIndex >= 0) && NewIndex < WorldInfo.GRI.PRIArray.Length)
        {
            PRI = WorldInfo.GRI.PRIArray[NewIndex];
            // End:0x2A6
            if((((PRI != none) && Controller(PRI.Owner) != none) && Controller(PRI.Owner).Pawn != none) && WorldInfo.Game.CanSpectate(self, PRI))
            {
                bSuccess = true;
                // [Explicit Break]
                goto J0x2BB;
            }
            NewIndex = NewIndex + Dir;
            // [Loop Continue]
            goto J0x1CF;
        }
    }
    J0x2BB:

    // End:0x2D4
    if(bSuccess)
    {
        SetViewTarget(PRI);
    }
    //return;    
}

unreliable server function ServerViewSelf(optional ViewTargetTransitionParams TransitionParams)
{
    // End:0x51
    if(IsSpectating())
    {
        ResetCameraMode();
        SetViewTarget(self, TransitionParams);
        ClientSetViewTarget(self, TransitionParams);
        ClientMessage(OwnCamera, 'Event');
    }
    //return;    
}

function bool CanRestartPlayer()
{
    return ((PlayerReplicationInfo != none) && !PlayerReplicationInfo.bOnlySpectator) && HasClientLoadedCurrentWorld();
    //return ReturnValue;    
}

function DrawHUD(HUD H)
{
    // End:0x24
    if(Pawn != none)
    {
        Pawn.DrawHUD(H);
    }
    // End:0x48
    if(PlayerInput != none)
    {
        PlayerInput.DrawHUD(H);
    }
    //return;    
}

function OnToggleInput(SeqAct_ToggleInput inAction)
{
    local bool bNewValue;

    // End:0x2D
    if(Role < ROLE_Authority)
    {
        WarnInternal("Not supported on client");
        return;
    }
    // End:0xA2
    if(inAction.InputLinks[0].bHasImpulse)
    {
        // End:0x76
        if(inAction.bToggleMovement)
        {
            IgnoreMoveInput(false);
            ClientIgnoreMoveInput(false);
        }
        // End:0x9F
        if(inAction.bToggleTurning)
        {
            IgnoreLookInput(false);
            ClientIgnoreLookInput(false);
        }        
    }
    else
    {
        // End:0x117
        if(inAction.InputLinks[1].bHasImpulse)
        {
            // End:0xEB
            if(inAction.bToggleMovement)
            {
                IgnoreMoveInput(true);
                ClientIgnoreMoveInput(true);
            }
            // End:0x114
            if(inAction.bToggleTurning)
            {
                IgnoreLookInput(true);
                ClientIgnoreLookInput(true);
            }            
        }
        else
        {
            // End:0x1C4
            if(inAction.InputLinks[2].bHasImpulse)
            {
                // End:0x17E
                if(inAction.bToggleMovement)
                {
                    bNewValue = !IsMoveInputIgnored();
                    IgnoreMoveInput(bNewValue);
                    ClientIgnoreMoveInput(bNewValue);
                }
                // End:0x1C4
                if(inAction.bToggleTurning)
                {
                    bNewValue = !IsLookInputIgnored();
                    IgnoreLookInput(bNewValue);
                    ClientIgnoreLookInput(bNewValue);
                }
            }
        }
    }
    //return;    
}

reliable client simulated function ClientIgnoreMoveInput(bool bIgnore)
{
    IgnoreMoveInput(bIgnore);
    //return;    
}

reliable client simulated function ClientIgnoreLookInput(bool bIgnore)
{
    IgnoreLookInput(bIgnore);
    //return;    
}

simulated function DisplayDebug(HUD HUD, out float out_YL, out float out_YPos)
{
    super.DisplayDebug(HUD, out_YL, out_YPos);
    // End:0xDF
    if(HUD.ShouldDisplayDebug('Camera'))
    {
        // End:0x66
        if(PlayerCamera != none)
        {
            PlayerCamera.DisplayDebug(HUD, out_YL, out_YPos);            
        }
        else
        {
            HUD.Canvas.SetDrawColor(255, 0, 0);
            HUD.Canvas.DrawText("NO CAMERA");
            out_YPos += out_YL;
            HUD.Canvas.SetPos(4.0000000, out_YPos);
        }
    }
    // End:0x1C4
    if(HUD.ShouldDisplayDebug('Input'))
    {
        HUD.Canvas.SetDrawColor(255, 0, 0);
        HUD.Canvas.DrawText((((("Input ignoremove " $ string(bIgnoreMoveInput)) $ " ignore look ") $ string(bIgnoreLookInput)) $ " aForward ") $ string(PlayerInput.aForward));
        out_YPos += out_YL;
        HUD.Canvas.SetPos(4.0000000, out_YPos);
    }
    //return;    
}

simulated function OnSetCameraTarget(SeqAct_SetCameraTarget inAction)
{
    local Actor RealCameraTarget;

    RealCameraTarget = inAction.CameraTarget;
    // End:0x3C
    if(RealCameraTarget == none)
    {
        RealCameraTarget = ((Pawn != none) ? Pawn : self);        
    }
    else
    {
        // End:0x6E
        if(RealCameraTarget.IsA('Controller'))
        {
            RealCameraTarget = Controller(RealCameraTarget).Pawn;
        }
    }
    SetViewTarget(RealCameraTarget, inAction.TransitionParams);
    //return;    
}

simulated function OnToggleHUD(SeqAct_ToggleHUD inAction)
{
    // End:0xB9
    if(myHUD != none)
    {
        // End:0x40
        if(inAction.InputLinks[0].bHasImpulse)
        {
            myHUD.bShowHUD = true;            
        }
        else
        {
            // End:0x75
            if(inAction.InputLinks[1].bHasImpulse)
            {
                myHUD.bShowHUD = false;                
            }
            else
            {
                // End:0xB9
                if(inAction.InputLinks[2].bHasImpulse)
                {
                    myHUD.bShowHUD = !myHUD.bShowHUD;
                }
            }
        }
    }
    //return;    
}

unreliable server function ServerCauseEvent(name EventName)
{
    local array<SequenceObject> AllConsoleEvents;
    local SeqEvent_Console ConsoleEvt;
    local Sequence GameSeq;
    local int Idx;
    local bool bFoundEvt;

    GameSeq = WorldInfo.GetGameSequence();
    // End:0xCF
    if((GameSeq != none) && EventName != 'None')
    {
        GameSeq.FindSeqObjectsByClass(Class'SeqEvent_Console', true, AllConsoleEvents);
        Idx = 0;
        J0x58:

        // End:0xCF [Loop If]
        if(Idx < AllConsoleEvents.Length)
        {
            ConsoleEvt = SeqEvent_Console(AllConsoleEvents[Idx]);
            // End:0xC5
            if((ConsoleEvt != none) && EventName == ConsoleEvt.ConsoleEventName)
            {
                bFoundEvt = true;
                ConsoleEvt.CheckActivate(self, Pawn);
            }
            Idx++;
            // [Loop Continue]
            goto J0x58;
        }
    }
    // End:0xE4
    if(!bFoundEvt)
    {
        ListConsoleEvents();
    }
    //return;    
}

exec function ListConsoleEvents()
{
    local array<SequenceObject> ConsoleEvents;
    local SeqEvent_Console ConsoleEvt;
    local Sequence GameSeq;
    local int Idx;

    GameSeq = WorldInfo.GetGameSequence();
    // End:0xD1
    if(GameSeq != none)
    {
        LogInternal("Console events:");
        GameSeq.FindSeqObjectsByClass(Class'SeqEvent_Console', true, ConsoleEvents);
        Idx = 0;
        J0x57:

        // End:0xD1 [Loop If]
        if(Idx < ConsoleEvents.Length)
        {
            ConsoleEvt = SeqEvent_Console(ConsoleEvents[Idx]);
            // End:0xC7
            if((ConsoleEvt != none) && ConsoleEvt.bEnabled)
            {
                LogInternal(("-" @ string(ConsoleEvt.ConsoleEventName)) @ ConsoleEvt.EventDesc);
            }
            Idx++;
            // [Loop Continue]
            goto J0x57;
        }
    }
    //return;    
}

exec function ListCE()
{
    ListConsoleEvents();
    //return;    
}

exec function ShowPlayerState()
{
    LogInternal("Dumping state stack for" @ string(self));
    DumpStateStack();
    //return;    
}

exec function ShowGameState()
{
    // End:0x93
    if(WorldInfo.Game != none)
    {
        LogInternal((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ": Dumping state stack for") @ string(WorldInfo.Game));
        WorldInfo.Game.DumpStateStack();        
    }
    else
    {
        LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ": No GameInfo found!");
    }
    //return;    
}

function NotifyTakeHit(Controller InstigatedBy, Vector HitLocation, int Damage, class<DamageType> DamageType, Vector Momentum)
{
    super.NotifyTakeHit(InstigatedBy, HitLocation, Damage, DamageType, Momentum);
    ClientPlayForceFeedbackWaveform(DamageType.default.DamagedFFWaveform);
    //return;    
}

function OnForceFeedback(SeqAct_ForceFeedback Action)
{
    // End:0x3C
    if(Action.InputLinks[0].bHasImpulse)
    {
        ClientPlayForceFeedbackWaveform(Action.FFWaveform);        
    }
    else
    {
        // End:0x71
        if(Action.InputLinks[1].bHasImpulse)
        {
            ClientStopForceFeedbackWaveform(Action.FFWaveform);
        }
    }
    //return;    
}

event PlayRumble(const AnimNotify_Rumble TheAnimNotify)
{
    // End:0x3B
    if(TheAnimNotify.PredefinedWaveForm != none)
    {
        ClientPlayForceFeedbackWaveform(TheAnimNotify.PredefinedWaveForm.default.TheWaveForm);        
    }
    else
    {
        ClientPlayForceFeedbackWaveform(TheAnimNotify.WaveForm);
    }
    //return;    
}

reliable client simulated event ClientPlayForceFeedbackWaveform(ForceFeedbackWaveform FFWaveform)
{
    // End:0x24
    if((PlayerInput != none) && !PlayerInput.bUsingGamepad)
    {
        return;
    }
    // End:0x64
    if(((ForceFeedbackManager != none) && PlayerReplicationInfo != none) && IsForceFeedbackAllowed())
    {
        ForceFeedbackManager.PlayForceFeedbackWaveform(FFWaveform);
    }
    //return;    
}

reliable client final simulated event ClientStopForceFeedbackWaveform(optional ForceFeedbackWaveform FFWaveform)
{
    // End:0x25
    if(ForceFeedbackManager != none)
    {
        ForceFeedbackManager.StopForceFeedbackWaveform(FFWaveform);
    }
    //return;    
}

simulated function bool IsForceFeedbackAllowed()
{
    return (PlayerReplicationInfo == none) || PlayerReplicationInfo.bControllerVibrationAllowed;
    //return ReturnValue;    
}

function CameraShake(float Duration, Vector newRotAmplitude, Vector newRotFrequency, Vector newLocAmplitude, Vector newLocFrequency, float newFOVAmplitude, float newFOVFrequency)
{
    //return;    
}

function OnToggleCinematicMode(SeqAct_ToggleCinematicMode Action)
{
    local bool bNewCinematicMode;

    // End:0x2D
    if(Role < ROLE_Authority)
    {
        WarnInternal("Not supported on client");
        return;
    }
    // End:0x58
    if(Action.InputLinks[0].bHasImpulse)
    {
        bNewCinematicMode = true;        
    }
    else
    {
        // End:0x83
        if(Action.InputLinks[1].bHasImpulse)
        {
            bNewCinematicMode = false;            
        }
        else
        {
            // End:0xB3
            if(Action.InputLinks[2].bHasImpulse)
            {
                bNewCinematicMode = !bCinematicMode;
            }
        }
    }
    SetCinematicMode(bNewCinematicMode, Action.bHidePlayer, Action.bHideHUD, Action.bDisableMovement, Action.bDisableTurning, Action.bDisableInput, true);
    //return;    
}

function SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsButtons, bool bCheckMovieFinishedForPause)
{
    local bool bAdjustMoveInput, bAdjustLookInput;

    bCinematicMode = bInCinematicMode;
    // End:0x86
    if(Pawn != none)
    {
        // End:0x71
        if(bCinematicMode)
        {
            // End:0x3F
            if(bHidePlayer)
            {
                Pawn.SetHidden(true);
            }
            LogInternalAudio("Stop firing sound");
            Pawn.WeaponStoppedFiring(false);            
        }
        else
        {
            Pawn.SetHidden(false);
        }
    }
    bAdjustMoveInput = bAffectsMovement && bCinematicMode != bCinemaDisableInputMove;
    bAdjustLookInput = bAffectsTurning && bCinematicMode != bCinemaDisableInputLook;
    // End:0xEC
    if(bAdjustMoveInput)
    {
        IgnoreMoveInput(bCinematicMode);
        bCinemaDisableInputMove = bCinematicMode;
    }
    // End:0x112
    if(bAdjustLookInput)
    {
        IgnoreLookInput(bCinematicMode);
        bCinemaDisableInputLook = bCinematicMode;
    }
    ClientSetCinematicMode(bCinematicMode, bAdjustMoveInput, bAdjustLookInput, bAffectsHUD);
    //return;    
}

reliable client simulated function ClientSetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsHUD)
{
    bCinematicMode = bInCinematicMode;
    // End:0x3C
    if((myHUD != none) && bAffectsHUD)
    {
        myHUD.bShowHUD = !bCinematicMode;
    }
    // End:0x55
    if(bAffectsMovement)
    {
        IgnoreMoveInput(bCinematicMode);
    }
    // End:0x6E
    if(bAffectsTurning)
    {
        IgnoreLookInput(bCinematicMode);
    }
    //return;    
}

function IgnoreMoveInput(bool bNewMoveInput)
{
    bIgnoreMoveInput = byte(Max(int(bIgnoreMoveInput) + ((bNewMoveInput) ? 1 : -1), 0));
    //return;    
}

event bool IsMoveInputIgnored()
{
    return bIgnoreMoveInput > 0;
    //return ReturnValue;    
}

function IgnoreLookInput(bool bNewLookInput)
{
    bIgnoreLookInput = byte(Max(int(bIgnoreLookInput) + ((bNewLookInput) ? 1 : -1), 0));
    //return;    
}

event bool IsLookInputIgnored()
{
    return bIgnoreLookInput > 0;
    //return ReturnValue;    
}

function ResetPlayerMovementInput()
{
    bIgnoreMoveInput = default.bIgnoreMoveInput;
    bIgnoreLookInput = default.bIgnoreLookInput;
    //return;    
}

function OnConsoleCommand(SeqAct_ConsoleCommand inAction)
{
    local string Command;

    // End:0x60
    foreach inAction.Commands(Command)
    {
        // End:0x5F
        if(!(Left(Command, 3) ~= "set") && !(Left(Command, 8) ~= "setnopec"))
        {            
            ConsoleCommand(Command);
        }        
    }    
    //return;    
}

reliable client simulated event ClientForceGarbageCollection()
{
    WorldInfo.ForceGarbageCollection();
    //return;    
}

event LevelStreamingStatusChanged(LevelStreaming LevelObject, bool bNewShouldBeLoaded, bool bNewShouldBeVisible, bool bNewShouldBlockOnLoad)
{
    ClientUpdateLevelStreamingStatus(LevelObject.PackageName, bNewShouldBeLoaded, bNewShouldBeVisible, bNewShouldBlockOnLoad);
    //return;    
}

// Export UPlayerController::execClientUpdateLevelStreamingStatus(FFrame&, void* const)
reliable client native final simulated function ClientUpdateLevelStreamingStatus(name PackageName, bool bNewShouldBeLoaded, bool bNewShouldBeVisible, bool bNewShouldBlockOnLoad);

// Export UPlayerController::execServerUpdateLevelVisibility(FFrame&, void* const)
reliable server native final event ServerUpdateLevelVisibility(name PackageName, bool bIsVisible);

reliable client simulated event ClientPrepareMapChange(name LevelName, bool bFirst, bool bLast)
{
    local PlayerController PC;

    // End:0x28
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        // End:0x24
        if(PC != self)
        {            
            return;
            // End:0x27
            continue;
        }
        // End:0x28
        break;        
    }    
    // End:0x4A
    if(bFirst)
    {
        PendingMapChangeLevelNames.Length = 0;
        ClearTimer('DelayedPrepareMapChange');
    }
    PendingMapChangeLevelNames[PendingMapChangeLevelNames.Length] = LevelName;
    // End:0x6F
    if(bLast)
    {
        DelayedPrepareMapChange();
    }
    //return;    
}

function DelayedPrepareMapChange()
{
    // End:0x29
    if(WorldInfo.IsPreparingMapChange())
    {
        SetTimer(0.0100000, false, 'DelayedPrepareMapChange');        
    }
    else
    {
        WorldInfo.PrepareMapChange(PendingMapChangeLevelNames);
    }
    //return;    
}

reliable client simulated event ClientCommitMapChange()
{
    // End:0x29
    if(IsTimerActive('DelayedPrepareMapChange'))
    {
        SetTimer(0.0100000, false, 'ClientCommitMapChange');        
    }
    else
    {
        // End:0x47
        if(Pawn != none)
        {
            SetViewTarget(Pawn);            
        }
        else
        {
            SetViewTarget(self);
        }
        WorldInfo.CommitMapChange();
    }
    //return;    
}

reliable client simulated event ClientCancelPendingMapChange()
{
    WorldInfo.CancelPendingMapChange();
    //return;    
}

// Export UPlayerController::execClientFlushLevelStreaming(FFrame&, void* const)
reliable client native final simulated event ClientFlushLevelStreaming();

reliable client simulated event ClientSetBlockOnAsyncLoading()
{
    WorldInfo.bRequestedBlockOnAsyncLoading = true;
    //return;    
}

exec function SaveClassConfig(coerce string ClassName)
{
    local Class saveClass;

    LogInternal("SaveClassConfig:" @ ClassName);
    saveClass = class<Object>(DynamicLoadObject(ClassName, Class'Core.Class'));
    // End:0x77
    if(saveClass != none)
    {
        LogInternal("- Saving config on:" @ string(saveClass));
        saveClass.static.StaticSaveConfig();        
    }
    else
    {
        LogInternal("- Failed to find class:" @ ClassName);
    }
    //return;    
}

exec function SaveActorConfig(coerce name actorName)
{
    local Actor ChkActor;

    LogInternal("SaveActorConfig:" @ string(actorName));
    // End:0x83
    foreach AllActors(Class'Actor', ChkActor)
    {
        // End:0x82
        if((ChkActor != none) && ChkActor.Name == actorName)
        {
            LogInternal("- Saving config on:" @ string(ChkActor));
            ChkActor.SaveConfig();
        }        
    }    
    //return;    
}

final function UIInteraction GetUIController()
{
    local LocalPlayer LP;
    local UIInteraction Result;

    LP = LocalPlayer(Player);
    // End:0x65
    if((LP != none) && LP.Outer.GameViewport != none)
    {
        Result = LP.Outer.GameViewport.UIController;
    }
    return Result;
    //return ReturnValue;    
}

// Export UPlayerController::execIsPlayerMuted(FFrame&, void* const)
native final function bool IsPlayerMuted(const out UniqueNetId Sender);

event GetSeamlessTravelActorList(bool bToEntry, out array<Actor> ActorList)
{
    HearSoundActiveComponents.Length = 0;
    HearSoundPoolComponents.Length = 0;
    // End:0x5E
    if(myHUD != none)
    {
        ActorList[ActorList.Length] = myHUD;
        // End:0x5E
        if(myHUD.ScoreBoard != none)
        {
            ActorList[ActorList.Length] = myHUD.ScoreBoard;
        }
    }
    //return;    
}

function SeamlessTravelTo(PlayerController NewPC)
{
    //return;    
}

function SeamlessTravelFrom(PlayerController OldPC)
{
    OldPC.PlayerReplicationInfo.Reset();
    OldPC.PlayerReplicationInfo.SeamlessTravelTo(PlayerReplicationInfo);
    OldPC.bIsPlayer = false;
    OldPC.PlayerReplicationInfo.Destroy();
    OldPC.PlayerReplicationInfo = none;
    //return;    
}

reliable client simulated function ClientSetOnlineStatus()
{
    //return;    
}

// Export UPlayerController::execGetPlayerControllerFromNetId(FFrame&, void* const)
native static function PlayerController GetPlayerControllerFromNetId(UniqueNetId PlayerNetId);

reliable client simulated function ClientVoiceHandshakeComplete()
{
    bHasVoiceHandshakeCompleted = true;
    //return;    
}

reliable client simulated event ClientMutePlayer(UniqueNetId PlayerNetId)
{
    local LocalPlayer LocPlayer;

    // End:0x5A
    if(NotEqual_InterfaceInterface(VoiceInterface, none))
    {
        LocPlayer = LocalPlayer(Player);
        // End:0x5A
        if(LocPlayer != none)
        {
            VoiceInterface.MuteRemoteTalker(byte(LocPlayer.ControllerId), PlayerNetId);
        }
    }
    //return;    
}

reliable client simulated event ClientUnmutePlayer(UniqueNetId PlayerNetId)
{
    local LocalPlayer LocPlayer;

    // End:0x5A
    if(NotEqual_InterfaceInterface(VoiceInterface, none))
    {
        LocPlayer = LocalPlayer(Player);
        // End:0x5A
        if(LocPlayer != none)
        {
            VoiceInterface.UnmuteRemoteTalker(byte(LocPlayer.ControllerId), PlayerNetId);
        }
    }
    //return;    
}

function GameplayMutePlayer(UniqueNetId PlayerNetId)
{
    // End:0x38
    if(GameplayVoiceMuteList.Find('Uid', PlayerNetId.Uid) == -1)
    {
        GameplayVoiceMuteList.AddItem(PlayerNetId);
    }
    // End:0x70
    if(VoicePacketFilter.Find('Uid', PlayerNetId.Uid) == -1)
    {
        VoicePacketFilter.AddItem(PlayerNetId);
    }
    ClientMutePlayer(PlayerNetId);
    //return;    
}

function GameplayUnmutePlayer(UniqueNetId PlayerNetId)
{
    local int RemoveIndex;
    local PlayerController Other;

    RemoveIndex = GameplayVoiceMuteList.Find('Uid', PlayerNetId.Uid);
    // End:0x42
    if(RemoveIndex != -1)
    {
        GameplayVoiceMuteList.Remove(RemoveIndex, 1);
    }
    Other = GetPlayerControllerFromNetId(PlayerNetId);
    // End:0x11F
    if(Other != none)
    {
        // End:0x11F
        if((VoiceMuteList.Find('Uid', PlayerNetId.Uid) == -1) && Other.VoiceMuteList.Find('Uid', PlayerReplicationInfo.UniqueId.Uid) == -1)
        {
            RemoveIndex = VoicePacketFilter.Find('Uid', PlayerNetId.Uid);
            // End:0x110
            if(RemoveIndex != -1)
            {
                VoicePacketFilter.Remove(RemoveIndex, 1);
            }
            ClientUnmutePlayer(PlayerNetId);
        }
    }
    //return;    
}

reliable server event ServerMutePlayer(UniqueNetId PlayerNetId)
{
    local PlayerController Other;

    // End:0x38
    if(VoiceMuteList.Find('Uid', PlayerNetId.Uid) == -1)
    {
        VoiceMuteList.AddItem(PlayerNetId);
    }
    // End:0x70
    if(VoicePacketFilter.Find('Uid', PlayerNetId.Uid) == -1)
    {
        VoicePacketFilter.AddItem(PlayerNetId);
    }
    ClientMutePlayer(PlayerNetId);
    Other = GetPlayerControllerFromNetId(PlayerNetId);
    // End:0x122
    if(Other != none)
    {
        // End:0xFF
        if(Other.VoicePacketFilter.Find('Uid', PlayerReplicationInfo.UniqueId.Uid) == -1)
        {
            Other.VoicePacketFilter.AddItem(PlayerReplicationInfo.UniqueId);
        }
        Other.ClientMutePlayer(PlayerReplicationInfo.UniqueId);
    }
    //return;    
}

reliable server event ServerUnmutePlayer(UniqueNetId PlayerNetId)
{
    local PlayerController Other;
    local int RemoveIndex;

    RemoveIndex = VoiceMuteList.Find('Uid', PlayerNetId.Uid);
    // End:0x42
    if(RemoveIndex != -1)
    {
        VoiceMuteList.Remove(RemoveIndex, 1);
    }
    Other = GetPlayerControllerFromNetId(PlayerNetId);
    // End:0x222
    if(Other != none)
    {
        // End:0xDD
        if((GameplayVoiceMuteList.Find('Uid', PlayerNetId.Uid) == -1) && Other.VoiceMuteList.Find('Uid', PlayerReplicationInfo.UniqueId.Uid) == -1)
        {
            ClientUnmutePlayer(PlayerNetId);
        }
        // End:0x222
        if((Other.VoiceMuteList.Find('Uid', PlayerReplicationInfo.UniqueId.Uid) == -1) && Other.GameplayVoiceMuteList.Find('Uid', PlayerReplicationInfo.UniqueId.Uid) == -1)
        {
            RemoveIndex = VoicePacketFilter.Find('Uid', PlayerNetId.Uid);
            // End:0x19F
            if(RemoveIndex != -1)
            {
                VoicePacketFilter.Remove(RemoveIndex, 1);
            }
            RemoveIndex = Other.VoicePacketFilter.Find('Uid', PlayerReplicationInfo.UniqueId.Uid);
            // End:0x1FF
            if(RemoveIndex != -1)
            {
                Other.VoicePacketFilter.Remove(RemoveIndex, 1);
            }
            Other.ClientUnmutePlayer(PlayerReplicationInfo.UniqueId);
        }
    }
    //return;    
}

event NotifyDirectorControl(bool bNowControlling)
{
    // End:0x3C
    if((!bNowControlling && WorldInfo.NetMode == NM_Client) && bClientSimulatingViewTarget)
    {
        ServerVerifyViewTarget();
    }
    //return;    
}

// Export UPlayerController::execSetShowSubtitles(FFrame&, void* const)
native simulated exec function SetShowSubtitles(bool bValue);

// Export UPlayerController::execIsShowingSubtitles(FFrame&, void* const)
native simulated function bool IsShowingSubtitles();

function NotifyConnectionError(optional string Message = Localize("Errors", "ConnectionFailed", "Engine"), optional string Title = Localize("Errors", "ConnectionFailed_Title", "Engine"))
{
    LogInternal((((((((((((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "Title:'") $ Title) $ "'") @ "Message:'") $ Message) $ "'") @ "NetMode:'") $ string(GetEnum(Enum'ENetMode', WorldInfo.NetMode))) $ "'") @ "Map:'") $ GetURLMap()) $ "'", 'DevNet');
    // End:0x176
    if(WorldInfo.NetMode != NM_Standalone)
    {
        // End:0x15F
        if(WorldInfo.Game != none)
        {
            WorldInfo.Game.bHasNetworkError = true;
        }
        ClientTravel("?failed", 0);
    }
    //return;    
}

reliable client simulated event ClientWasKicked()
{
    //return;    
}

reliable client simulated function ClientRegisterForArbitration()
{
    // End:0x7E
    if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
    {
        OnlineSub.GameInterface.AddArbitrationRegistrationCompleteDelegate(OnArbitrationRegisterComplete);
        OnlineSub.GameInterface.RegisterForArbitration('Game');        
    }
    else
    {
        ServerRegisteredForArbitration(true);
    }
    //return;    
}

function OnArbitrationRegisterComplete(name SessionName, bool bWasSuccessful)
{
    OnlineSub.GameInterface.ClearArbitrationRegistrationCompleteDelegate(OnArbitrationRegisterComplete);
    ServerRegisteredForArbitration(bWasSuccessful);
    //return;    
}

reliable server function ServerRegisteredForArbitration(bool bWasSuccessful)
{
    WorldInfo.Game.ProcessClientRegistrationCompletion(self, bWasSuccessful);
    //return;    
}

function OnGameInviteAccepted(OnlineGameSettings GameInviteSettings)
{
    // End:0x21D
    if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
    {
        // End:0x213
        if(GameInviteSettings != none)
        {
            // End:0x206
            if(InviteHasEnoughSpace(GameInviteSettings))
            {
                // End:0x1F9
                if(CanAllPlayersPlayOnline())
                {
                    // End:0x163
                    if(WorldInfo.NetMode != NM_Standalone)
                    {
                        WorldInfo.GRI.bNeedsOnlineCleanup = false;
                        // End:0x110
                        if(OnlineSub.GameInterface.GetGameSettings('Game').bUsesArbitration)
                        {
                            ClientWriteOnlinePlayerScores(((WorldInfo.GRI.GameClass != none) ? WorldInfo.GRI.GameClass.default.ArbitratedLeaderboardId : 0));
                        }
                        OnlineSub.GameInterface.AddEndOnlineGameCompleteDelegate(OnEndForInviteComplete);
                        OnlineSub.GameInterface.EndOnlineGame('Game');                        
                    }
                    else
                    {
                        OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
                        // End:0x1F6
                        if(!OnlineSub.GameInterface.AcceptGameInvite(byte(LocalPlayer(Player).ControllerId), 'Game'))
                        {
                            OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
                        }
                    }                    
                }
                else
                {
                    NotifyNotAllPlayersCanJoinInvite();
                }                
            }
            else
            {
                NotifyNotEnoughSpaceInInvite();
            }            
        }
        else
        {
            NotifyInviteFailed();
        }
    }
    //return;    
}

function bool InviteHasEnoughSpace(OnlineGameSettings InviteSettings)
{
    local int NumLocalPlayers;
    local PlayerController PC;

    // End:0x1B
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        NumLocalPlayers++;        
    }    
    return (InviteSettings.NumOpenPrivateConnections + InviteSettings.NumOpenPublicConnections) >= NumLocalPlayers;
    //return ReturnValue;    
}

function bool CanAllPlayersPlayOnline()
{
    local PlayerController PC;
    local LocalPlayer LocPlayer;

    // End:0xBA
    foreach LocalPlayerControllers(Class'PlayerController', PC)
    {
        LocPlayer = LocalPlayer(PC.Player);
        // End:0xB6
        if(LocPlayer != none)
        {
            // End:0xB3
            if((OnlineSub.PlayerInterface.GetLoginStatus(byte(LocPlayer.ControllerId)) != 2) || OnlineSub.PlayerInterface.CanPlayOnline(byte(LocPlayer.ControllerId)) == 0)
            {                
                return false;
            }
            // End:0xB9
            continue;
        }        
        return false;        
    }    
    return true;
    //return ReturnValue;    
}

function ClearInviteDelegates()
{
    OnlineSub.GameInterface.ClearEndOnlineGameCompleteDelegate(OnEndForInviteComplete);
    OnlineSub.GameInterface.ClearDestroyOnlineGameCompleteDelegate(OnDestroyForInviteComplete);
    OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
    //return;    
}

function OnEndForInviteComplete(name SessionName, bool bWasSuccessful)
{
    OnlineSub.GameInterface.AddDestroyOnlineGameCompleteDelegate(OnDestroyForInviteComplete);
    OnlineSub.GameInterface.DestroyOnlineGame(SessionName);
    //return;    
}

function OnDestroyForInviteComplete(name SessionName, bool bWasSuccessful)
{
    // End:0xA5
    if(bWasSuccessful)
    {
        OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
        // End:0xA2
        if(!OnlineSub.GameInterface.AcceptGameInvite(byte(LocalPlayer(Player).ControllerId), SessionName))
        {
            OnlineSub.GameInterface.ClearJoinOnlineGameCompleteDelegate(OnInviteJoinComplete);
            NotifyInviteFailed();
        }        
    }
    else
    {
        NotifyInviteFailed();
    }
    //return;    
}

function OnInviteJoinComplete(name SessionName, bool bWasSuccessful)
{
    local string URL, ConnectPassword;

    // End:0x101
    if(bWasSuccessful)
    {
        // End:0xFE
        if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
        {
            // End:0xFE
            if(OnlineSub.GameInterface.GetResolvedConnectString(SessionName, URL))
            {
                // End:0xC8
                if(Class'UIRoot'.static.GetDataStoreStringValue("<Registry:ConnectPassword>", ConnectPassword) && ConnectPassword != "")
                {                    
                    URL $= ("?Password=" $ ConnectPassword);
                }
                LogInternal(("Resulting url is (" $ URL) $ ")");
                ClientTravel(URL, 0);
            }
        }        
    }
    else
    {
        NotifyInviteFailed();
    }
    ClearInviteDelegates();
    Class'UIRoot'.static.SetDataStoreStringValue("<Registry:ConnectPassword>", "");
    //return;    
}

function NotifyInviteFailed()
{
    LogInternal("Invite handling failed");
    ClearInviteDelegates();
    //return;    
}

function NotifyNotAllPlayersCanJoinInvite()
{
    LogInternal("Not all local players have permission to join the game invite");
    //return;    
}

function NotifyNotEnoughSpaceInInvite()
{
    LogInternal("Not enough space for all local players in the game invite");
    //return;    
}

reliable client simulated function ClientArbitratedMatchEnded()
{
    ConsoleCommand("Disconnect");
    //return;    
}

reliable client simulated function ClientWriteOnlinePlayerScores(int LeaderboardId)
{
    local GameReplicationInfo GRI;
    local int Index;
    local array<OnlinePlayerScore> PlayerScores;
    local UniqueNetId ZeroUniqueId;
    local bool bIsTeamGame;
    local int ScoreIndex;

    GRI = WorldInfo.GRI;
    // End:0x248
    if(((GRI != none) && OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.StatsInterface, none))
    {
        bIsTeamGame = ((GRI.GameClass != none) ? GRI.GameClass.default.bTeamGame : false);
        Index = 0;
        J0x8D:

        // End:0x210 [Loop If]
        if(Index < GRI.PRIArray.Length)
        {
            // End:0x206
            if(GRI.PRIArray[Index].UniqueId != ZeroUniqueId)
            {
                ScoreIndex = PlayerScores.Length;
                PlayerScores.Length = ScoreIndex + 1;
                PlayerScores[ScoreIndex].PlayerID = GRI.PRIArray[Index].UniqueId;
                // End:0x1B2
                if(bIsTeamGame)
                {
                    PlayerScores[ScoreIndex].TeamID = GRI.PRIArray[Index].Team.TeamIndex;
                    PlayerScores[ScoreIndex].Score = int(GRI.PRIArray[Index].Team.Score);
                    // [Explicit Continue]
                    goto J0x206;
                }
                PlayerScores[ScoreIndex].TeamID = Index;
                PlayerScores[ScoreIndex].Score = int(GRI.PRIArray[Index].Score);
            }
            J0x206:

            Index++;
            // [Loop Continue]
            goto J0x8D;
        }
        OnlineSub.StatsInterface.WriteOnlinePlayerScores(PlayerReplicationInfo.SessionName, LeaderboardId, PlayerScores);
    }
    //return;    
}

reliable client simulated function ClientWriteLeaderboardStats(class<OnlineStatsWrite> OnlineStatsWriteClass)
{
    //return;    
}

reliable client simulated function ClientSetHostUniqueId(UniqueNetId InHostId)
{
    //return;    
}

reliable client simulated function ClientStopNetworkedVoice()
{
    local LocalPlayer LocPlayer;

    LocPlayer = LocalPlayer(Player);
    // End:0x78
    if(((LocPlayer != none) && OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.VoiceInterface, none))
    {
        OnlineSub.VoiceInterface.StopNetworkedVoice(byte(LocPlayer.ControllerId));
    }
    //return;    
}

reliable client simulated function ClientStartNetworkedVoice()
{
    local LocalPlayer LocPlayer;

    LocPlayer = LocalPlayer(Player);
    // End:0x78
    if(((LocPlayer != none) && OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.VoiceInterface, none))
    {
        OnlineSub.VoiceInterface.StartNetworkedVoice(byte(LocPlayer.ControllerId));
    }
    //return;    
}

simulated function OnDestroy(SeqAct_Destroy Action)
{
    Action.ScriptLog("Cannot use Destroy action on players");
    //return;    
}

exec function ConsoleKey(name Key)
{
    // End:0x52
    if((LocalPlayer(Player) != none) && AllowConsole())
    {
        LocalPlayer(Player).ViewportClient.ViewportConsole.InputKey(0, Key, 0);
    }
    //return;    
}

exec function SendToConsole(string Command)
{
    // End:0x4D
    if((LocalPlayer(Player) != none) && AllowConsole())
    {
        LocalPlayer(Player).ViewportClient.ViewportConsole.ConsoleCommand(Command);
    }
    //return;    
}

final simulated function DrawDebugTextList(Canvas Canvas, float RenderDelta)
{
    local Vector cameraLoc, ScreenLoc, Offset;
    local Rotator cameraRot;
    local int Idx;

    // End:0x244
    if(DebugTextList.Length > 0)
    {
        GetPlayerViewPoint(cameraLoc, cameraRot);
        Canvas.SetDrawColor(255, 255, 255);
        Canvas.Font = Class'Engine'.static.GetSmallFont();
        Idx = 0;
        J0x5E:

        // End:0x244 [Loop If]
        if(Idx < DebugTextList.Length)
        {
            // End:0x9B
            if(DebugTextList[Idx].SrcActor == none)
            {
                DebugTextList.Remove(Idx--, 1);
                // [Explicit Continue]
                goto J0x23A;
            }
            // End:0x109
            if(DebugTextList[Idx].TimeRemaining != -1.0000000)
            {
                DebugTextList[Idx].TimeRemaining -= RenderDelta;
                // End:0x109
                if(DebugTextList[Idx].TimeRemaining <= 0.0000000)
                {
                    DebugTextList.Remove(Idx--, 1);
                    // [Explicit Continue]
                    goto J0x23A;
                }
            }
            Offset = VLerp(DebugTextList[Idx].SrcActorOffset, DebugTextList[Idx].SrcActorDesiredOffset, 1.0000000 - (DebugTextList[Idx].TimeRemaining / DebugTextList[Idx].Duration));
            ScreenLoc = Canvas.Project(DebugTextList[Idx].SrcActor.Location + (Offset >> cameraRot));
            Canvas.SetPos(ScreenLoc.X, ScreenLoc.Y);
            Canvas.DrawColor = DebugTextList[Idx].TextColor;
            Canvas.DrawText(DebugTextList[Idx].DebugText);
            J0x23A:

            Idx++;
            // [Loop Continue]
            goto J0x5E;
        }
    }
    //return;    
}

reliable client final simulated event AddDebugText(string DebugText, optional Actor SrcActor, optional float Duration = -1.0000000, optional Vector Offset, optional Vector DesiredOffset, optional Color TextColor, optional bool bSkipOverwriteCheck)
{
    local int Idx;

    // End:0xCC
    if((((TextColor.R == 0) && TextColor.G == 0) && TextColor.B == 0) && TextColor.A == 0)
    {
        TextColor.R = 255;
        TextColor.G = 255;
        TextColor.B = 255;
        TextColor.A = 255;
    }
    // End:0xE2
    if(SrcActor == none)
    {
        SrcActor = Pawn;
    }
    // End:0x23B
    if(SrcActor != none)
    {
        // End:0x108
        if(Len(DebugText) == 0)
        {
            RemoveDebugText(SrcActor);            
        }
        else
        {
            // End:0x15C
            if(!bSkipOverwriteCheck)
            {
                Idx = DebugTextList.Find('SrcActor', SrcActor);
                // End:0x159
                if(Idx == -1)
                {
                    Idx = DebugTextList.Length;
                    DebugTextList.Length = Idx + 1;
                }                
            }
            else
            {
                Idx = DebugTextList.Length;
                DebugTextList.Length = Idx + 1;
            }
            DebugTextList[Idx].SrcActor = SrcActor;
            DebugTextList[Idx].SrcActorOffset = Offset;
            DebugTextList[Idx].SrcActorDesiredOffset = DesiredOffset;
            DebugTextList[Idx].DebugText = DebugText;
            DebugTextList[Idx].TimeRemaining = Duration;
            DebugTextList[Idx].Duration = Duration;
            DebugTextList[Idx].TextColor = TextColor;
        }
    }
    //return;    
}

reliable client final simulated event RemoveDebugText(Actor SrcActor)
{
    local int Idx;

    Idx = DebugTextList.Find('SrcActor', SrcActor);
    // End:0x37
    if(Idx != -1)
    {
        DebugTextList.Remove(Idx, 1);
    }
    //return;    
}

event DrawShieldDecals()
{
    //return;    
}

function EnableDebugCamera()
{
    local Player P;
    local Vector eyeLoc;
    local Rotator eyeRot;

    P = Player;
    // End:0x12B
    if(((P != none) && Pawn != none) && IsLocalPlayerController())
    {
        // End:0x54
        if(DebugCameraControllerRef == none)
        {
            DebugCameraControllerRef = Spawn(DebugCameraControllerClass);
        }
        DebugCameraControllerRef.OryginalPlayer = P;
        DebugCameraControllerRef.OryginalControllerRef = self;
        GetPlayerViewPoint(eyeLoc, eyeRot);
        DebugCameraControllerRef.SetLocation(eyeLoc);
        DebugCameraControllerRef.SetRotation(eyeRot);
        DebugCameraControllerRef.PlayerCamera.SetFOV(GetFOVAngle());
        DebugCameraControllerRef.PlayerCamera.UpdateCamera(0.0000000);
        P.SwitchController(DebugCameraControllerRef);
        DebugCameraControllerRef.OnActivate(self);
    }
    //return;    
}

reliable client simulated function ClientRegisterHostStatGuid(string StatGuid)
{
    // End:0x88
    if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.StatsInterface, none))
    {
        OnlineSub.StatsInterface.AddRegisterHostStatGuidCompleteDelegate(OnRegisterHostStatGuidComplete);
        // End:0x88
        if(OnlineSub.StatsInterface.RegisterHostStatGuid(StatGuid) == false)
        {
            OnRegisterHostStatGuidComplete(false);
        }
    }
    //return;    
}

function OnRegisterHostStatGuidComplete(bool bWasSuccessful)
{
    local string StatGuid;

    OnlineSub.StatsInterface.ClearRegisterHostStatGuidCompleteDelegateDelegate(OnRegisterHostStatGuidComplete);
    // End:0x65
    if(bWasSuccessful)
    {
        StatGuid = OnlineSub.StatsInterface.GetClientStatGuid();
        ServerRegisterClientStatGuid(StatGuid);
    }
    //return;    
}

reliable server function ServerRegisterClientStatGuid(string StatGuid)
{
    // End:0x5E
    if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.StatsInterface, none))
    {
        OnlineSub.StatsInterface.RegisterStatGuid(PlayerReplicationInfo.UniqueId, StatGuid);
    }
    //return;    
}

reliable client simulated function ClientStartOnlineGame()
{
    local OnlineGameSettings GameSettings;

    // End:0xDF
    if(((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none)) && IsPrimaryPlayer())
    {
        GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
        // End:0xDF
        if((GameSettings != none) && (GameSettings.GameState == 1) || GameSettings.GameState == 5)
        {
            OnlineSub.GameInterface.StartOnlineGame(PlayerReplicationInfo.SessionName);
        }
    }
    //return;    
}

reliable client simulated function ClientEndOnlineGame()
{
    local OnlineGameSettings GameSettings;

    // End:0xC3
    if(((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none)) && IsPrimaryPlayer())
    {
        GameSettings = OnlineSub.GameInterface.GetGameSettings(PlayerReplicationInfo.SessionName);
        // End:0xC3
        if((GameSettings != none) && GameSettings.GameState == 3)
        {
            OnlineSub.GameInterface.EndOnlineGame(PlayerReplicationInfo.SessionName);
        }
    }
    //return;    
}

function bool CanViewUserCreatedContent()
{
    local LocalPlayer LocPlayer;

    LocPlayer = LocalPlayer(Player);
    // End:0x81
    if(((LocPlayer != none) && OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.PlayerInterface, none))
    {
        return OnlineSub.PlayerInterface.CanDownloadUserContent(byte(LocPlayer.ControllerId)) == 2;
    }
    return true;
    //return ReturnValue;    
}

function IncrementNumberOfMatchesPlayed()
{
    LogInternal("  Num Matches Played: " $ string(PlayerReplicationInfo.AutomatedTestingData.NumberOfMatchesPlayed));
    PlayerReplicationInfo.AutomatedTestingData.NumberOfMatchesPlayed++;
    //return;    
}

event SoakPause(Pawn P)
{
    LogInternal("Soak pause by " $ string(P));
    SetViewTarget(P);
    SetPause(true);
    myHUD.bShowDebugInfo = true;
    //return;    
}

event OnPlayerHasBeenMoved()
{
    LogInternal(("Player " $ string(Pawn.Name)) $ " has been moved.");
    //return;    
}

exec function PathStep(optional int Cnt)
{
    Pawn.IncrementPathStep(Max(1, Cnt), myHUD.Canvas);
    //return;    
}

exec function PathChild(optional int Cnt)
{
    Pawn.IncrementPathChild(Max(1, Cnt), myHUD.Canvas);
    //return;    
}

exec function PathClear()
{
    Pawn.ClearPathStep();
    //return;    
}

reliable client simulated function ClientTravelToSession(name SessionName, class<OnlineGameSearch> SearchClass, byte PlatformSpecificInfo[68])
{
    local OnlineGameSearch Search;
    local LocalPlayer LP;
    local OnlineGameSearchResult SessionToJoin;

    LP = LocalPlayer(Player);
    // End:0xE1
    if(LP != none)
    {
        Search = new SearchClass;
        // End:0xE1
        if(OnlineSub.GameInterface.BindPlatformSpecificSessionToSearch(byte(LP.ControllerId), Search, PlatformSpecificInfo))
        {
            SessionToJoin = Search.Results[0];
            OnlineSub.GameInterface.AddJoinOnlineGameCompleteDelegate(OnJoinTravelToSessionComplete);
            OnlineSub.GameInterface.JoinOnlineGame(byte(LP.ControllerId), SessionName, SessionToJoin);
        }
    }
    //return;    
}

function OnJoinTravelToSessionComplete(name SessionName, bool bWasSuccessful)
{
    local string URL;

    // End:0x76
    if(bWasSuccessful)
    {
        // End:0x76
        if(OnlineSub.GameInterface.GetResolvedConnectString(SessionName, URL))
        {
            LogInternal(("Resulting url for 'Game' is (" $ URL) $ ")");
            ClientTravel(URL, 0);
        }
    }
    //return;    
}

reliable client simulated function ClientReturnToParty()
{
    local string URL;

    // End:0x163
    if(IsPrimaryPlayer())
    {
        // End:0x147
        if(((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none)) && NotEqual_InterfaceInterface(OnlineSub.PlayerInterface, none))
        {
            // End:0x128
            if(OnlineSub.GameInterface.GetGameSettings('Party') != none)
            {
                // End:0xE2
                if(IsPartyLeader())
                {
                    URL = (((GetPartyMapName()) $ "?game=") $ (GetPartyGameTypeName())) $ "?listen";
                    WorldInfo.ServerTravel(URL, true, true);                    
                }
                else
                {
                    // End:0x125
                    if(OnlineSub.GameInterface.GetResolvedConnectString('Party', URL))
                    {
                        ClientTravel(URL, 0);
                    }
                }                
            }
            else
            {                
                ConsoleCommand("disconnect");
            }            
        }
        else
        {            
            ConsoleCommand("disconnect");
        }
    }
    //return;    
}

simulated function bool IsPrimaryPlayer()
{
    local int SSIndex;

    return !IsSplitscreenPlayer(SSIndex) || SSIndex == 0;
    //return ReturnValue;    
}

simulated function bool IsSplitscreenPlayer(optional out int out_SplitscreenPlayerIndex)
{
    local bool bResult;
    local LocalPlayer LP;
    local NetConnection RemoteConnection;
    local ChildConnection ChildRemoteConnection;

    out_SplitscreenPlayerIndex = int(NetPlayerIndex);
    // End:0x19B
    if(Player != none)
    {
        LP = LocalPlayer(Player);
        RemoteConnection = NetConnection(Player);
        // End:0x96
        if(LP != none)
        {
            // End:0x93
            if(LP.Outer.GamePlayers.Length > 1)
            {
                out_SplitscreenPlayerIndex = LP.Outer.GamePlayers.Find(LP);
                bResult = true;
            }            
        }
        else
        {
            // End:0x12E
            if(RemoteConnection != none)
            {
                // End:0xC9
                if(RemoteConnection.Children.Length > 0)
                {
                    out_SplitscreenPlayerIndex = 0;
                    bResult = true;                    
                }
                else
                {
                    ChildRemoteConnection = ChildConnection(RemoteConnection);
                    // End:0x12B
                    if(ChildRemoteConnection != none)
                    {
                        // End:0x123
                        if(ChildRemoteConnection.Parent != none)
                        {
                            out_SplitscreenPlayerIndex = ChildRemoteConnection.Parent.Children.Find(ChildRemoteConnection) + 1;
                        }
                        bResult = true;
                    }
                }                
            }
            else
            {
                LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "NOT A LOCALPLAYER AND NOT A REMOTECONNECTION!", 'RON_DEBUG');
            }
        }        
    }
    else
    {
        LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "called without a valid Player value!");
    }
    return bResult;
    //return ReturnValue;    
}

simulated function bool HasSplitscreenPlayer(PlayerReplicationInfo PRI)
{
    local bool bResult;
    local PlayerController OwnerPC;

    // End:0xA0
    if(PRI != none)
    {
        // End:0x37
        if(PRI.IsLocalPlayerPRI())
        {
            bResult = IsSplitscreenPlayer();            
        }
        else
        {
            // End:0x80
            if(Role == ROLE_Authority)
            {
                OwnerPC = PlayerController(PRI.Owner);
                bResult = OwnerPC.IsSplitscreenPlayer();                
            }
            else
            {
                bResult = PRI.SplitscreenIndex != -1;
            }
        }        
    }
    else
    {
        WarnInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "called with a NULL PRI!");
    }
    return bResult;
    //return ReturnValue;    
}

simulated function PlayerReplicationInfo GetSplitscreenPlayerByIndex(optional int PlayerIndex = 1)
{
    local PlayerReplicationInfo Result;
    local LocalPlayer LP, SplitPlayer;
    local NetConnection MasterConnection, RemoteConnection;
    local ChildConnection ChildRemoteConnection;

    // End:0x49B
    if(Player != none)
    {
        // End:0x498
        if(IsSplitscreenPlayer())
        {
            LP = LocalPlayer(Player);
            RemoteConnection = NetConnection(Player);
            // End:0x194
            if(LP != none)
            {
                // End:0xD5
                if((PlayerIndex >= 0) && PlayerIndex < LP.ViewportClient.Outer.GamePlayers.Length)
                {
                    SplitPlayer = LP.ViewportClient.Outer.GamePlayers[PlayerIndex];
                    Result = SplitPlayer.Actor.PlayerReplicationInfo;                    
                }
                else
                {
                    WarnInternal((((((((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ":") @ "requested player at invalid index!") @ "PlayerIndex:'") $ string(PlayerIndex)) $ "'") @ "NumLocalPlayers:'") $ string(LP.ViewportClient.Outer.GamePlayers.Length)) $ "'");
                }                
            }
            else
            {
                // End:0x409
                if(RemoteConnection != none)
                {
                    // End:0x226
                    if(WorldInfo.NetMode == NM_Client)
                    {
                        WarnInternal((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ":") @ "CALLED ON CLIENT WITH VALID REMOTE NETCONNECTION!");                        
                    }
                    else
                    {
                        ChildRemoteConnection = ChildConnection(RemoteConnection);
                        // End:0x2EE
                        if(ChildRemoteConnection != none)
                        {
                            MasterConnection = ChildRemoteConnection.Parent;
                            // End:0x283
                            if(PlayerIndex == 0)
                            {
                                Result = MasterConnection.Actor.PlayerReplicationInfo;                                
                            }
                            else
                            {
                                PlayerIndex--;
                                // End:0x2EB
                                if((PlayerIndex >= 0) && PlayerIndex < MasterConnection.Children.Length)
                                {
                                    ChildRemoteConnection = MasterConnection.Children[PlayerIndex];
                                    Result = ChildRemoteConnection.Actor.PlayerReplicationInfo;
                                }
                            }                            
                        }
                        else
                        {
                            // End:0x388
                            if(RemoteConnection.Children.Length > 0)
                            {
                                // End:0x31D
                                if(PlayerIndex == 0)
                                {
                                    Result = PlayerReplicationInfo;                                    
                                }
                                else
                                {
                                    PlayerIndex--;
                                    // End:0x385
                                    if((PlayerIndex >= 0) && PlayerIndex < RemoteConnection.Children.Length)
                                    {
                                        ChildRemoteConnection = RemoteConnection.Children[PlayerIndex];
                                        Result = ChildRemoteConnection.Actor.PlayerReplicationInfo;
                                    }
                                }                                
                            }
                            else
                            {
                                LogInternal(((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ":") @ string(Player)) @ "IS NOT THE PRIMARY CONNECTION AND HAS NO CHILD CONNECTIONS!");
                            }
                        }
                    }                    
                }
                else
                {
                    LogInternal(((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ":") @ string(Player)) @ "IS NOT A LOCALPLAYER AND NOT A REMOTECONNECTION! (No valid Player reference)");
                }
            }
        }        
    }
    else
    {
        LogInternal((((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) $ ":") @ "NULL value for Player!");
    }
    return Result;
    //return ReturnValue;    
}

simulated function int GetSplitscreenPlayerCount()
{
    local LocalPlayer LP;
    local NetConnection RemoteConnection;
    local int Result;

    // End:0x180
    if(IsSplitscreenPlayer())
    {
        // End:0x127
        if(Player != none)
        {
            LP = LocalPlayer(Player);
            RemoteConnection = NetConnection(Player);
            // End:0x71
            if(LP != none)
            {
                Result = LP.ViewportClient.Outer.GamePlayers.Length;                
            }
            else
            {
                // End:0xC2
                if(RemoteConnection != none)
                {
                    // End:0xA6
                    if(ChildConnection(RemoteConnection) != none)
                    {
                        RemoteConnection = ChildConnection(RemoteConnection).Parent;
                    }
                    Result = RemoteConnection.Children.Length + 1;                    
                }
                else
                {
                    LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "NOT A LOCALPLAYER AND NOT A REMOTECONNECTION!");
                }
            }            
        }
        else
        {
            LogInternal(((("(" $ string(Name)) $ ") PlayerController::") $ string(GetFuncName())) @ "called without a valid Player value!");
        }
    }
    return Result;
    //return ReturnValue;    
}

reliable client simulated function ClientControlMovieTexture(TextureMovie MovieTexture, SeqAct_ControlMovieTexture.EMovieControlType Mode)
{
    // End:0x6D
    if(MovieTexture != none)
    {
        switch(Mode)
        {
            // End:0x2F
            case 0:
                MovieTexture.Play();
                // End:0x6D
                break;
            // End:0x4B
            case 1:
                MovieTexture.Stop();
                // End:0x6D
                break;
            // End:0x67
            case 2:
                MovieTexture.Pause();
                // End:0x6D
                break;
            // End:0xFFFF
            default:
                // End:0x6D
                break;
                break;
        }
    }
    //return;    
}

reliable client simulated event ClientSetForceMipLevelsToBeResident(MaterialInterface Material, float ForceDuration)
{
    // End:0x33
    if((Material != none) && IsPrimaryPlayer())
    {
        Material.SetForceMipLevelsToBeResident(ForceDuration);
    }
    //return;    
}

reliable client simulated event ClientPrestreamTextures(Actor ForcedActor, float ForceDuration, bool bEnableStreaming)
{
    // End:0x39
    if((ForcedActor != none) && IsPrimaryPlayer())
    {
        ForcedActor.PrestreamTextures(ForceDuration, bEnableStreaming);
    }
    //return;    
}

simulated function bool IsPartyLeader()
{
    local OnlineGameSettings PartySettings;

    // End:0x9E
    if((OnlineSub != none) && NotEqual_InterfaceInterface(OnlineSub.GameInterface, none))
    {
        PartySettings = OnlineSub.GameInterface.GetGameSettings('Party');
        // End:0x9E
        if(PartySettings != none)
        {
            // End:0x9E
            if(PlayerReplicationInfo != none)
            {
                return OnlineSub.AreUniqueNetIdsEqual(PartySettings.OwningPlayerId, PlayerReplicationInfo.UniqueId);
            }
        }
    }
    return (WorldInfo.NetMode != NM_Client) && IsPrimaryPlayer();
    //return ReturnValue;    
}

static function string GetPartyMapName()
{
    //return ReturnValue;    
}

static function string GetPartyGameTypeName()
{
    //return ReturnValue;    
}

event bool GetAchievementProgression(int AchievementId, out float CurrentValue, out float MaxValue)
{
    //return ReturnValue;    
}

simulated function OnFlyThroughHasEnded(SeqAct_FlyThroughHasEnded inAction)
{
    local PlayerController PC;

    // End:0x5F
    if(WorldInfo.Game.bDoingASentinelRun == true)
    {
        // End:0x5E
        foreach WorldInfo.AllControllers(Class'PlayerController', PC)
        {            
            PC.ConsoleCommand("quit");            
        }        
    }
    //return;    
}

function Sentinel_SetupForGamebasedTravelTheWorld()
{
    //return;    
}

function Sentinel_PreAcquireTravelTheWorldPoints()
{
    //return;    
}

function Sentinel_PostAcquireTravelTheWorldPoints()
{
    //return;    
}

unreliable client simulated event ClientSpawnCameraLensEffect(class<EmitterCameraLensEffectBase> LensEffectEmitterClass)
{
    //return;    
}

exec function BugItGo(coerce float X, coerce float Y, coerce float Z, coerce int Pitch, coerce int Yaw, coerce int Roll)
{
    local Vector TheLocation;
    local Rotator TheRotation;

    TheLocation.X = X;
    TheLocation.Y = Y;
    TheLocation.Z = Z;
    TheRotation.Pitch = Pitch;
    TheRotation.Yaw = Yaw;
    TheRotation.Roll = Roll;
    BugItWorker(TheLocation, TheRotation);
    //return;    
}

function BugItGoString(string TheLocation, string TheRotation)
{
    BugItWorker(GetFVectorFromString(TheLocation), GetFRotatorFromString(TheRotation));
    //return;    
}

function BugItWorker(Vector TheLocation, Rotator TheRotation)
{
    LogInternal(("BugItGo to:" @ string(TheLocation)) @ string(TheRotation));
    // End:0x41
    if(CheatManager != none)
    {
        CheatManager.Ghost();
    }
    ViewTarget.SetLocation(TheLocation);
    Pawn.FaceRotation(TheRotation, 0.0000000);
    SetRotation(TheRotation);
    //return;    
}

exec event BugIt(optional string ScreenShotDescription)
{
    local Vector ViewLocation;
    local Rotator ViewRotation;
    local string GoString, LocString;

    ConsoleCommand("bugscreenshot " $ ScreenShotDescription);
    GetPlayerViewPoint(ViewLocation, ViewRotation);
    // End:0x5C
    if(Pawn != none)
    {
        ViewLocation = Pawn.Location;
    }
    GoString = (((((((((("BugItGo " $ string(ViewLocation.X)) $ " ") $ string(ViewLocation.Y)) $ " ") $ string(ViewLocation.Z)) $ " ") $ string(ViewRotation.Pitch)) $ " ") $ string(ViewRotation.Yaw)) $ " ") $ string(ViewRotation.Roll);
    LogInternal(GoString);
    LocString = (((((((((((((("?BugLoc=(" $ "X=") $ string(ViewLocation.X)) $ ",Y=") $ string(ViewLocation.Y)) $ ",Z=") $ string(ViewLocation.Z)) $ ")") $ "?BugRot=(") $ "Pitch=") $ string(ViewRotation.Pitch)) $ ",Yaw=") $ string(ViewRotation.Yaw)) $ ",Roll=") $ string(ViewRotation.Roll)) $ ")";
    LogInternal(LocString);
    LogOutBugItGoToLogFile(ScreenShotDescription, GoString, LocString);
    //return;    
}

exec event BugItAI(optional string ScreenShotDescription)
{
    local Vector ViewLocation;
    local Rotator ViewRotation;
    local string GoString, LocString;

    GetPlayerViewPoint(ViewLocation, ViewRotation);
    // End:0x35
    if(Pawn != none)
    {
        ViewLocation = Pawn.Location;
    }
    GoString = (((((((((("BugItGo " $ string(ViewLocation.X)) $ " ") $ string(ViewLocation.Y)) $ " ") $ string(ViewLocation.Z)) $ " ") $ string(ViewRotation.Pitch)) $ " ") $ string(ViewRotation.Yaw)) $ " ") $ string(ViewRotation.Roll);
    LogInternal(GoString);
    LocString = (((((((((((((("?BugLoc=(" $ "X=") $ string(ViewLocation.X)) $ ",Y=") $ string(ViewLocation.Y)) $ ",Z=") $ string(ViewLocation.Z)) $ ")") $ "?BugRot=(") $ "Pitch=") $ string(ViewRotation.Pitch)) $ ",Yaw=") $ string(ViewRotation.Yaw)) $ ",Roll=") $ string(ViewRotation.Roll)) $ ")";
    LogInternal(LocString);    
    ConsoleCommand("debugai");
    SetTimer(0.1000000, false, 'DisableDebugAI');
    LogOutBugItAIGoToLogFile(ScreenShotDescription, GoString, LocString);
    //return;    
}

function DisableDebugAI()
{
    ConsoleCommand("debugai");
    //return;    
}

// Export UPlayerController::execGetFVectorFromString(FFrame&, void* const)
private native final function Vector GetFVectorFromString(string InStr);

// Export UPlayerController::execGetFRotatorFromString(FFrame&, void* const)
private native final function Rotator GetFRotatorFromString(string InStr);

// Export UPlayerController::execLogOutBugItGoToLogFile(FFrame&, void* const)
private native final function LogOutBugItGoToLogFile(const string InScreenShotDesc, const string InGoString, const string InLocString);

// Export UPlayerController::execLogOutBugItAIGoToLogFile(FFrame&, void* const)
private native final function LogOutBugItAIGoToLogFile(const string InScreenShotDesc, const string InGoString, const string InLocString);

state PlayerWalking
{
    event NotifyPhysicsVolumeChange(PhysicsVolume NewVolume)
    {
        // End:0x3C
        if(NewVolume.bWaterVolume && Pawn.bCollideWorld)
        {
            GotoState(Pawn.WaterMovementState);
        }
        //return;        
    }

    function ProcessMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        // End:0x0D
        if(Pawn == none)
        {
            return;
        }
        // End:0x3D
        if(Role == ROLE_Authority)
        {
            Pawn.SetRemoteViewPitch(Rotation.Pitch);
        }
        Pawn.Acceleration = newAccel;
        CheckJumpOrDuck();
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z, newAccel;
        local Actor.EDoubleClickDir DoubleClickMove;
        local Rotator OldRotation;
        local bool bSaveJump;

        // End:0x1C
        if(Pawn == none)
        {
            GotoState('Dead');            
        }
        else
        {
            GetAxes(Pawn.Rotation, X, Y, Z);
            newAccel = (PlayerInput.aForward * X) + (PlayerInput.aStrafe * Y);
            newAccel.Z = 0.0000000;
            newAccel = Pawn.AccelRate * Normal(newAccel);
            DoubleClickMove = PlayerInput.CheckForDoubleClickMove(DeltaTime / WorldInfo.TimeDilation);
            OldRotation = Rotation;
            UpdateRotation(DeltaTime);
            bDoubleJump = false;
            // End:0x12B
            if(bPressedJump && Pawn.CannotJumpNow())
            {
                bSaveJump = true;
                bPressedJump = false;                
            }
            else
            {
                bSaveJump = false;
            }
            // End:0x16C
            if(Role < ROLE_Authority)
            {
                ReplicateMove(DeltaTime, newAccel, DoubleClickMove, OldRotation - Rotation);                
            }
            else
            {
                ProcessMove(DeltaTime, newAccel, DoubleClickMove, OldRotation - Rotation);
            }
            bPressedJump = bSaveJump;
        }
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        DoubleClickDir = 0;
        bPressedJump = false;
        GroundPitch = 0;
        // End:0x37
        if(Pawn != none)
        {
            Pawn.ShouldCrouch(false);
        }
        //return;        
    }

    event EndState(name NextStateName)
    {
        GroundPitch = 0;
        // End:0x48
        if(Pawn != none)
        {
            Pawn.SetRemoteViewPitch(0);
            // End:0x48
            if(bDuck == 0)
            {
                Pawn.ShouldCrouch(false);
            }
        }
        //return;        
    }
Begin:

    stop;                
}

state PlayerClimbing
{
    event NotifyPhysicsVolumeChange(PhysicsVolume NewVolume)
    {
        // End:0x2A
        if(NewVolume.bWaterVolume)
        {
            GotoState(Pawn.WaterMovementState);            
        }
        else
        {
            GotoState(Pawn.LandMovementState);
        }
        //return;        
    }

    function ProcessMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        // End:0x0D
        if(Pawn == none)
        {
            return;
        }
        // End:0x3D
        if(Role == ROLE_Authority)
        {
            Pawn.SetRemoteViewPitch(Rotation.Pitch);
        }
        Pawn.Acceleration = newAccel;
        // End:0xA3
        if(bPressedJump)
        {
            Pawn.DoJump(bUpdating);
            // End:0xA3
            if(Pawn.Physics == 2)
            {
                GotoState(Pawn.LandMovementState);
            }
        }
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z, newAccel;
        local Rotator OldRotation, ViewRotation;

        GetAxes(Rotation, X, Y, Z);
        // End:0x98
        if(Pawn.OnLadder != none)
        {
            newAccel = PlayerInput.aForward * Pawn.OnLadder.ClimbDir;
            // End:0x95
            if(Pawn.OnLadder.bAllowLadderStrafing)
            {
                newAccel += (PlayerInput.aStrafe * Y);
            }            
        }
        else
        {
            newAccel = (PlayerInput.aForward * X) + (PlayerInput.aStrafe * Y);
        }
        newAccel = Pawn.AccelRate * Normal(newAccel);
        ViewRotation = Rotation;
        SetRotation(ViewRotation);
        OldRotation = Rotation;
        UpdateRotation(DeltaTime);
        // End:0x14D
        if(Role < ROLE_Authority)
        {
            ReplicateMove(DeltaTime, newAccel, 0, OldRotation - Rotation);            
        }
        else
        {
            ProcessMove(DeltaTime, newAccel, 0, OldRotation - Rotation);
        }
        bPressedJump = false;
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        Pawn.ShouldCrouch(false);
        bPressedJump = false;
        //return;        
    }

    event EndState(name NextStateName)
    {
        // End:0x31
        if(Pawn != none)
        {
            Pawn.SetRemoteViewPitch(0);
            Pawn.ShouldCrouch(false);
        }
        //return;        
    }
    stop;    
}

state PlayerSwimming
{
    event bool NotifyLanded(Vector HitNormal, Actor FloorActor)
    {
        // End:0x30
        if(Pawn.PhysicsVolume.bWaterVolume)
        {
            Pawn.SetPhysics(3);            
        }
        else
        {
            GotoState(Pawn.LandMovementState);
        }
        return bUpdating;
        //return ReturnValue;        
    }

    event NotifyPhysicsVolumeChange(PhysicsVolume NewVolume)
    {
        local Actor HitActor;
        local Vector HitLocation, HitNormal, Checkpoint, X, Y, Z;

        // End:0x29
        if(!Pawn.bCollideActors)
        {
            GotoState(Pawn.LandMovementState);
        }
        // End:0x290
        if(Pawn.Physics != 10)
        {
            // End:0x275
            if(!NewVolume.bWaterVolume)
            {
                Pawn.SetPhysics(2);
                // End:0x272
                if(Pawn.Velocity.Z > float(0))
                {
                    GetAxes(Rotation, X, Y, Z);
                    Pawn.bUpAndOut = ((X Dot Pawn.Acceleration) > float(0)) && (Pawn.Acceleration.Z > float(0)) || Rotation.Pitch > 2048;
                    // End:0x17E
                    if(Pawn.bUpAndOut && Pawn.CheckWaterJump(HitNormal))
                    {
                        Pawn.Velocity.Z = Pawn.OutofWaterZ;
                        GotoState(Pawn.LandMovementState);                        
                    }
                    else
                    {
                        // End:0x1D3
                        if((Pawn.Velocity.Z > float(160)) || !Pawn.TouchingWaterVolume())
                        {
                            GotoState(Pawn.LandMovementState);                            
                        }
                        else
                        {
                            Checkpoint = Pawn.Location;
                            Checkpoint.Z -= (Pawn.CylinderComponent.CollisionHeight + 6.0000000);
                            HitActor = Trace(HitLocation, HitNormal, Checkpoint, Pawn.Location, false);
                            // End:0x267
                            if(HitActor != none)
                            {
                                GotoState(Pawn.LandMovementState);                                
                            }
                            else
                            {
                                SetTimer(0.7000000, false);
                            }
                        }
                    }
                }                
            }
            else
            {
                ClearTimer();
                Pawn.SetPhysics(3);
            }            
        }
        else
        {
            // End:0x2B9
            if(!NewVolume.bWaterVolume)
            {
                GotoState(Pawn.LandMovementState);
            }
        }
        //return;        
    }

    function ProcessMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        Pawn.Acceleration = newAccel;
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Rotator OldRotation;
        local Vector X, Y, Z, newAccel;

        // End:0x1C
        if(Pawn == none)
        {
            GotoState('Dead');            
        }
        else
        {
            GetAxes(Rotation, X, Y, Z);
            newAccel = ((PlayerInput.aForward * X) + (PlayerInput.aStrafe * Y)) + (PlayerInput.aUp * vect(0.0000000, 0.0000000, 1.0000000));
            newAccel = Pawn.AccelRate * Normal(newAccel);
            OldRotation = Rotation;
            UpdateRotation(DeltaTime);
            // End:0xF4
            if(Role < ROLE_Authority)
            {
                ReplicateMove(DeltaTime, newAccel, 0, OldRotation - Rotation);                
            }
            else
            {
                ProcessMove(DeltaTime, newAccel, 0, OldRotation - Rotation);
            }
            bPressedJump = false;
        }
        //return;        
    }

    event Timer()
    {
        // End:0x45
        if(!Pawn.PhysicsVolume.bWaterVolume && Role == ROLE_Authority)
        {
            GotoState(Pawn.LandMovementState);
        }
        ClearTimer();
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        ClearTimer();
        // End:0x32
        if(Pawn.Physics != 10)
        {
            Pawn.SetPhysics(3);
        }
        //return;        
    }
Begin:

    stop;                
}

state PlayerFlying
{
    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z;

        GetAxes(Rotation, X, Y, Z);
        Pawn.Acceleration = ((PlayerInput.aForward * X) + (PlayerInput.aStrafe * Y)) + (PlayerInput.aUp * vect(0.0000000, 0.0000000, 1.0000000));
        Pawn.Acceleration = Pawn.AccelRate * Normal(Pawn.Acceleration);
        // End:0xEF
        if(bCheatFlying && Pawn.Acceleration == vect(0.0000000, 0.0000000, 0.0000000))
        {
            Pawn.Velocity = vect(0.0000000, 0.0000000, 0.0000000);
        }
        UpdateRotation(DeltaTime);
        // End:0x13E
        if(Role < ROLE_Authority)
        {
            ReplicateMove(DeltaTime, Pawn.Acceleration, 0, rot(0, 0, 0));            
        }
        else
        {
            ProcessMove(DeltaTime, Pawn.Acceleration, 0, rot(0, 0, 0));
        }
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        Pawn.SetPhysics(4);
        //return;        
    }
    stop;    
}

state BaseSpectating
{
    function bool IsSpectating()
    {
        return true;
        //return ReturnValue;        
    }

    function bool LimitSpectatorVelocity()
    {
        // End:0x69
        if(Location.Z > WorldInfo.StallZ)
        {
            Velocity.Z = FMin(SpectatorCameraSpeed, (WorldInfo.StallZ - Location.Z) - 2.0000000);
            return true;            
        }
        else
        {
            // End:0xCF
            if(Location.Z < WorldInfo.KillZ)
            {
                Velocity.Z = FMin(SpectatorCameraSpeed, (WorldInfo.KillZ - Location.Z) + 2.0000000);
                return true;
            }
        }
        return false;
        //return ReturnValue;        
    }

    function ProcessMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        local float VelSize;

        Acceleration = Normal(newAccel) * SpectatorCameraSpeed;
        VelSize = VSize(Velocity);
        // End:0x64
        if(VelSize > float(0))
        {
            Velocity = Velocity - ((Velocity - (Normal(Acceleration) * VelSize)) * FMin(DeltaTime * float(8), 1.0000000));
        }
        Velocity = Velocity + (Acceleration * DeltaTime);
        // End:0xA2
        if(VSize(Velocity) > SpectatorCameraSpeed)
        {
            Velocity = Normal(Velocity) * SpectatorCameraSpeed;
        }
        LimitSpectatorVelocity();
        // End:0x10E
        if(VSize(Velocity) > float(0))
        {
            MoveSmooth((float(1 + int(bRun)) * Velocity) * DeltaTime);
            // End:0x10E
            if(LimitSpectatorVelocity())
            {
                MoveSmooth((Velocity.Z * vect(0.0000000, 0.0000000, 1.0000000)) * DeltaTime);
            }
        }
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z;

        GetAxes(Rotation, X, Y, Z);
        Acceleration = ((PlayerInput.aForward * X) + (PlayerInput.aStrafe * Y)) + (PlayerInput.aUp * vect(0.0000000, 0.0000000, 1.0000000));
        UpdateRotation(DeltaTime);
        // End:0xAF
        if(Role < ROLE_Authority)
        {
            ReplicateMove(DeltaTime, Acceleration, 0, rot(0, 0, 0));            
        }
        else
        {
            ProcessMove(DeltaTime, Acceleration, 0, rot(0, 0, 0));
        }
        //return;        
    }

    unreliable server function ServerSetSpectatorLocation(Vector NewLoc)
    {
        SetLocation(NewLoc);
        // End:0x4B
        if((WorldInfo.TimeSeconds - LastSpectatorStateSynchTime) > 2.0000000)
        {
            ClientGotoState(GetStateName());
            LastSpectatorStateSynchTime = WorldInfo.TimeSeconds;
        }
        //return;        
    }

    function ReplicateMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        ProcessMove(DeltaTime, newAccel, DoubleClickMove, DeltaRot);
        ServerSetSpectatorLocation(Location);
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        bCollideWorld = true;
        //return;        
    }

    event EndState(name NextStateName)
    {
        bCollideWorld = false;
        //return;        
    }
    stop;    
}

state Spectating extends BaseSpectating
{
    ignores RestartLevel, ClientRestart, Suicide, ThrowWeapon;

    exec function StartFire(optional byte FireModeNum)
    {
        ServerViewNextPlayer();
        //return;        
    }

    exec function StartAltFire(optional byte FireModeNum)
    {
        ResetCameraMode();
        ServerViewSelf();
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        // End:0x27
        if(Pawn != none)
        {
            SetLocation(Pawn.Location);
            UnPossess();
        }
        bCollideWorld = true;
        //return;        
    }

    event EndState(name NextStateName)
    {
        // End:0x63
        if(PlayerReplicationInfo.bOnlySpectator)
        {
            LogInternal("WARNING - Spectator only player leaving spectating state to go to " $ string(NextStateName));
        }
        PlayerReplicationInfo.bIsSpectator = false;
        bCollideWorld = false;
        //return;        
    }
    stop;    
}

auto state PlayerWaiting extends BaseSpectating
{
    // ignores TakeDamage, NextWeapon, PrevWeapon, SwitchToBestWeapon, Jump, Suicide, 
	//     ServerSuicide;
    ignores TakeDamage, NextWeapon, PrevWeapon, SwitchToBestWeapon, Suicide, 
	    ServerSuicide;

    reliable server function ServerChangeTeam(int N)
    {
        WorldInfo.Game.ChangeTeam(self, N, true);
        //return;        
    }

    reliable server function ServerRestartPlayer()
    {
        // End:0x1B
        if(WorldInfo.TimeSeconds < WaitDelay)
        {
            return;
        }
        // End:0x37
        if(WorldInfo.NetMode == NM_Client)
        {
            return;
        }
        // End:0x69
        if(WorldInfo.Game.bWaitingToStartMatch)
        {
            PlayerReplicationInfo.bReadyToPlay = true;            
        }
        else
        {
            WorldInfo.Game.RestartPlayer(self);
        }
        //return;        
    }

    exec function StartFire(optional byte FireModeNum)
    {
        ServerRestartPlayer();
        //return;        
    }

    event EndState(name NextStateName)
    {
        // End:0x20
        if(PlayerReplicationInfo != none)
        {
            PlayerReplicationInfo.SetWaitingPlayer(false);
        }
        bCollideWorld = false;
        //return;        
    }

    simulated event BeginState(name PreviousStateName)
    {
        // End:0x20
        if(PlayerReplicationInfo != none)
        {
            PlayerReplicationInfo.SetWaitingPlayer(true);
        }
        bCollideWorld = true;
        //return;        
    }
    stop;    
}

state WaitingForPawn extends BaseSpectating
{
    ignores KilledBy;

    exec function StartFire(optional byte FireModeNum)
    {
        AskForPawn();
        //return;        
    }

    reliable client simulated function ClientGotoState(name NewState, optional name NewLabel)
    {
        // End:0x28
        if(NewState == 'RoundEnded')
        {
            global.ClientGotoState(NewState, NewLabel);
        }
        //return;        
    }

    unreliable client simulated function LongClientAdjustPosition(float TimeStamp, name NewState, Actor.EPhysics newPhysics, float NewLocX, float NewLocY, float NewLocZ, float NewVelX, float NewVelY, float NewVelZ, Actor NewBase, float NewFloorX, float NewFloorY, float NewFloorZ)
    {
        // End:0x1D
        if(NewState == 'RoundEnded')
        {
            GotoState(NewState);
        }
        //return;        
    }

    event PlayerTick(float DeltaTime)
    {
        global.PlayerTick(DeltaTime);
        // End:0x52
        if(Pawn != none)
        {
            Pawn.Controller = self;
            Pawn.BecomeViewTarget(self);
            ClientRestart(Pawn);            
        }
        else
        {
            // End:0x88
            if(!IsTimerActive() || GetTimerCount() > 1.0000000)
            {
                SetTimer(0.2000000, true);
                AskForPawn();
            }
        }
        //return;        
    }

    function ReplicateMove(float DeltaTime, Vector newAccel, Actor.EDoubleClickDir DoubleClickMove, Rotator DeltaRot)
    {
        ProcessMove(DeltaTime, newAccel, DoubleClickMove, DeltaRot);
        //return;        
    }

    event Timer()
    {
        AskForPawn();
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        SetTimer(0.2000000, true);
        AskForPawn();
        //return;        
    }

    event EndState(name NextStateName)
    {
        ResetCameraMode();
        SetTimer(0.0000000, false);
        //return;        
    }
    stop;    
}

state RoundEnded
{
    ignores KilledBy, TakeDamage, Suicide, ServerRestartPlayer, ThrowWeapon, Use, 
	    LongClientAdjustPosition;

    function bool IsSpectating()
    {
        return true;
        //return ReturnValue;        
    }

    event Possess(Pawn aPawn, bool bVehicleTransition)
    {
        global.Possess(aPawn, bVehicleTransition);
        // End:0x34
        if(Pawn != none)
        {
            Pawn.TurnOff();
        }
        //return;        
    }

    reliable server function ServerRestartGame()
    {
        // End:0x40
        if(WorldInfo.Game.PlayerCanRestartGame(self))
        {
            WorldInfo.Game.ResetLevel();
        }
        //return;        
    }

    exec function StartFire(optional byte FireModeNum)
    {
        // End:0x13
        if(Role < ROLE_Authority)
        {
            return;
        }
        // End:0x2B
        if(!bFrozen)
        {
            ServerRestartGame();            
        }
        else
        {
            // End:0x43
            if(!IsTimerActive())
            {
                SetTimer(1.5000000, false);
            }
        }
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z;
        local Rotator DeltaRot, ViewRotation;

        GetAxes(Rotation, X, Y, Z);
        ViewRotation = Rotation;
        DeltaRot.Yaw = int(PlayerInput.aTurn);
        DeltaRot.Pitch = int(PlayerInput.aLookUp);
        ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
        SetRotation(ViewRotation);
        ViewShake(DeltaTime);
        // End:0xD3
        if(Role < ROLE_Authority)
        {
            ReplicateMove(DeltaTime, vect(0.0000000, 0.0000000, 0.0000000), 0, rot(0, 0, 0));            
        }
        else
        {
            ProcessMove(DeltaTime, vect(0.0000000, 0.0000000, 0.0000000), 0, rot(0, 0, 0));
        }
        bPressedJump = false;
        //return;        
    }

    unreliable server function ServerMove(float TimeStamp, Vector InAccel, Vector ClientLoc, byte NewFlags, byte ClientRoll, int View)
    {
        global.ServerMove(TimeStamp, InAccel, ClientLoc, NewFlags, ClientRoll, ((Rotation.Yaw & 65535) << 16) + (Rotation.Pitch & 65535));
        //return;        
    }

    function FindGoodView()
    {
        local Rotator GoodRotation;

        GoodRotation = Rotation;
        GetViewTarget().FindGoodEndView(self, GoodRotation);
        SetRotation(GoodRotation);
        //return;        
    }

    event Timer()
    {
        bFrozen = false;
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        local Pawn P;

        FOVAngle = DesiredFOV;
        bFire = 0;
        // End:0x4E
        if(Pawn != none)
        {
            Pawn.TurnOff();
            Pawn.bSpecialHUD = false;
            StopFiring();
        }
        // End:0x6E
        if(myHUD != none)
        {
            myHUD.SetShowScores(true);
        }
        bFrozen = true;
        FindGoodView();
        SetTimer(5.0000000, false);
        // End:0xB0
        foreach DynamicActors(Class'Pawn', P)
        {
            P.TurnOff();            
        }        
        //return;        
    }

    event EndState(name NextStateName)
    {
        // End:0x20
        if(myHUD != none)
        {
            myHUD.SetShowScores(false);
        }
        //return;        
    }
Begin:

    stop;                
}

state Dead
{
    ignores KilledBy, NextWeapon, PrevWeapon, ThrowWeapon;

    function bool IsDead()
    {
        return true;
        //return ReturnValue;        
    }

    reliable server function ServerRestartPlayer()
    {
        // End:0x26
        if(!WorldInfo.Game.PlayerCanRestart(self))
        {
            return;
        }
        super.ServerRestartPlayer();
        //return;        
    }

    exec function StartFire(optional byte FireModeNum)
    {
        // End:0x35
        if(bFrozen)
        {
            // End:0x33
            if(!IsTimerActive() || GetTimerCount() > MinRespawnDelay)
            {
                bFrozen = false;
            }
            return;
        }
        ServerRestartPlayer();
        //return;        
    }

    exec function Use()
    {
        StartFire(0);
        //return;        
    }

    exec function Jump()
    {
        StartFire(0);
        //return;        
    }

    unreliable server function ServerMove(float TimeStamp, Vector Accel, Vector ClientLoc, byte NewFlags, byte ClientRoll, int View)
    {
        global.ServerMove(TimeStamp, Accel, ClientLoc, 0, ClientRoll, View);
        //return;        
    }

    function PlayerMove(float DeltaTime)
    {
        local Vector X, Y, Z;
        local Rotator DeltaRot, ViewRotation;

        // End:0xEC
        if(!bFrozen)
        {
            // End:0x28
            if(bPressedJump)
            {
                StartFire(0);
                bPressedJump = false;
            }
            GetAxes(Rotation, X, Y, Z);
            ViewRotation = Rotation;
            DeltaRot.Yaw = int(PlayerInput.aTurn);
            DeltaRot.Pitch = int(PlayerInput.aLookUp);
            ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
            SetRotation(ViewRotation);
            // End:0xE9
            if(Role < ROLE_Authority)
            {
                ReplicateMove(DeltaTime, vect(0.0000000, 0.0000000, 0.0000000), 0, rot(0, 0, 0));
            }            
        }
        else
        {
            // End:0x115
            if(!IsTimerActive() || GetTimerCount() > MinRespawnDelay)
            {
                bFrozen = false;
            }
        }
        ViewShake(DeltaTime);
        //return;        
    }

    function FindGoodView()
    {
        local Vector cameraLoc;
        local Rotator cameraRot, ViewRotation;
        local int tries, besttry;
        local float bestDist, newdist;
        local int startYaw;
        local Actor TheViewTarget;

        ViewRotation = Rotation;
        ViewRotation.Pitch = 56000;
        tries = 0;
        besttry = 0;
        bestDist = 0.0000000;
        startYaw = ViewRotation.Yaw;
        TheViewTarget = GetViewTarget();
        tries = 0;
        J0x67:

        // End:0x108 [Loop If]
        if(tries < 16)
        {
            cameraLoc = TheViewTarget.Location;
            SetRotation(ViewRotation);
            GetPlayerViewPoint(cameraLoc, cameraRot);
            newdist = VSize(cameraLoc - TheViewTarget.Location);
            // End:0xE7
            if(newdist > bestDist)
            {
                bestDist = newdist;
                besttry = tries;
            }
            ViewRotation.Yaw += 4096;
            tries++;
            // [Loop Continue]
            goto J0x67;
        }
        ViewRotation.Yaw = startYaw + (besttry * 4096);
        SetRotation(ViewRotation);
        //return;        
    }

    event Timer()
    {
        // End:0x0D
        if(!bFrozen)
        {
            return;
        }
        bFrozen = false;
        bPressedJump = false;
        //return;        
    }

    event BeginState(name PreviousStateName)
    {
        // End:0x33
        if((Pawn != none) && Pawn.Controller == self)
        {
            Pawn.Controller = none;
        }
        Pawn = none;
        FOVAngle = DesiredFOV;
        Enemy = none;
        bFrozen = true;
        bPressedJump = false;
        FindGoodView();
        SetTimer(MinRespawnDelay, false);
        CleanOutSavedMoves();
        //return;        
    }

    event EndState(name NextStateName)
    {
        CleanOutSavedMoves();
        Velocity = vect(0.0000000, 0.0000000, 0.0000000);
        Acceleration = vect(0.0000000, 0.0000000, 0.0000000);
        // End:0x4F
        if(!PlayerReplicationInfo.bOutOfLives)
        {
            ResetCameraMode();
        }
        bPressedJump = false;
        // End:0x77
        if(myHUD != none)
        {
            myHUD.SetShowScores(false);
        }
        //return;        
    }
Begin:

    // End:0x2F
    if(LocalPlayer(Player) != none)
    {
        // End:0x2F
        if(myHUD != none)
        {
            myHUD.PlayerOwnerDied();
        }
    }
    stop;                    
}

defaultproperties
{
    CameraClass=Class'Camera'
    DebugCameraControllerClass=Class'DebugCameraController'
    PlayerOwnerDataStoreClass=Class'PlayerOwnerDataStore'
    bDynamicNetSpeed=true
    bAimingHelp=true
    bIsUsingStreamingVolumes=true
    bCheckRelevancyThroughPortals=true
    MaxResponseTime=0.1250000
    FOVAngle=85.0000000
    DesiredFOV=85.0000000
    DefaultFOV=85.0000000
    LODDistanceFactor=1.0000000
    SavedMoveClass=Class'SavedMove'
    DynamicPingThreshold=400.0000000
    LastSpeedHackLog=-100.0000000
    ProgressTimeOut=8.0000000
    QuickSaveString="Quick Saving"
    NoPauseMessage="Pausa no permitida"
    ViewingFrom="Now viewing from"
    OwnCamera="Visi?n desde c?mara propia"
    CheatClass=Class'CheatManager'
    InputClass=Class'PlayerInput'
    // Reference: CylinderComponent'Default__PlayerController.CollisionCylinder'
    // TemplateOwnerClass: none
    // TemplateOwnerName: 'CollisionCylinder'
    begin object name="CollisionCylinder" class=Class'CylinderComponent'
    end object
    CylinderComponent=CollisionCylinder
    ForceFeedbackManagerClassName="WinDrv.XnaForceFeedbackManager"
    InteractDistance=512.0000000
    SpectatorCameraSpeed=600.0000000
    MinRespawnDelay=1.0000000
    bIsPlayer=true
    bCanDoSpecial=true
    Components[0]=none
    Components[1]=CollisionCylinder
    CollisionComponent=CollisionCylinder
}
