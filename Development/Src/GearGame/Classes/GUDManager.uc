/**
 * Class: GUDManager
 * High level manager for the overall Gears Unscripted Dialog system.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GUDManager extends Actor
	config(Game)
	dependson(GearTypes,GUDTypes,GUDBank,GearSoundGroup)
	notplaceable
	native(Sound);

/** These define how the GUDActionIDs are arranged in the GUDEvent.ActionIDs array.  They can be used as indices. */
enum GUDEventActionMap_Normal
{
	GUDActionMap_Instigator,
	GUDActionMap_Recipient,
	GUDActionMap_TeamWitness,
	GUDActionMap_EnemyWitness,
};
enum GUDEventActionMap_Directional
{
	GUDActionMap_Dir_Above,
	GUDActionMap_Dir_Below,
	GUDActionMap_Dir_Right,
	GUDActionMap_Dir_Left,
	GUDActionMap_Dir_Ahead,
	GUDActionMap_Dir_Behind,
};
enum GUDActionMap_LocationDescription
{
	/** NOTE: The first part of this enum should mirror the Coverlink.ECoverLocationDescription enum. */
	GUDActionMap_LocDesc_None,
	GUDActionMap_LocDesc_InWindow,
	GUDActionMap_LocDesc_InDoorway,
	GUDActionMap_LocDesc_BehindCar,
	GUDActionMap_LocDesc_BehindTruck,
	GUDActionMap_LocDesc_OnTruck,
	GUDActionMap_LocDesc_BehindBarrier,
	GUDActionMap_LocDesc_BehindColumn,
	GUDActionMap_LocDesc_BehindCrate,
	GUDActionMap_LocDesc_BehindWall,
	GUDActionMap_LocDesc_BehindStatue,
	GUDActionMap_LocDesc_BehindSandbags,

	/** end ECoverLocationDescription mirroring */
	GUDActionMap_LocDesc_InTheOpen,
};

/** Specifies how to decide which GUDAction to play in response to an event. */
enum GUDChooseActionMethod
{
	GUDChooseMethod_Normal,
	GUDChooseMethod_Directional,
	GUDChooseMethod_LocationDescription
};


/**
 * Structure to encapsulate the properties for a particular type of event.
 * Properties are set in the defaultproperties block below.
 */
struct native GUDEventProperties
{
	/** percent change (range [0..1]) that this event will cause a line to be played */
	var()	float ChanceToPlay;

	/** Separate chance for multiplayer, if desired.  Set to <0.f to use regular ChangeToPlay in MP */
	var()	float ChanceToPlayMP;

	/** minimum time between lines being played for this event */
	var()	float MinTimeBetweenSec;

	/** time that this event was last played */
	var		float						TimeLastPlayed;

	/** indexed via the actionmap */
	var		array<EGUDActionID>			ActionIDs;

	/** Which method to use to choose an action when this event occurs. */
	var		GUDChooseActionMethod		ChooseActionMethod;

	/** delay to use when playing Witness lines, basically to account for reaction time */
	var		float						ObserverDelay;

	/**
	 * A linked event shares the same min time between.  as in, if event A plays,
	 * A, B, and C won't play again until min time elapses.
	 */
	var		array<EGUDEventID>			LinkedEvents;

	/** Controls who hears lines from this event in Versus MP.  See ESpeakLineBroadcastFilter in GearPawn. */
	var		ESpeakLineBroadcastFilter	MPBroadcastFilter;

	/** If you want to force the speaker for this event to come from a particular role.  If GUDRole_None, speaker is random. */
	var		EGUDRole					ForcedSpeakerRole;

	/** Only consider witness within this distance. */
	var		float						MaxWitnessDistSq;

	structdefaultproperties {
		TimeLastPlayed=-999.f
		ChanceToPlay=1.f
		ChanceToPlayMP=-1.f;
		MinTimeBetweenSec=20.f
		ObserverDelay=1.f
		MaxWitnessDistSq=67108864.f				// 8192*8192

		ChooseActionMethod=GUDChooseMethod_Normal
		MPBroadcastFilter=SLBFilter_None
		ForcedSpeakerRole=GUDRole_None
	}
};
/** One property entry per event type */
var protected array<GUDEventProperties> EventProperties;


/**
 * Structure to store a record of an actual event that has occurred.
 */
struct native GUDEvent
{
	/** Who caused the event */
	var GearPawn	Instigator;
	/** Who the event happened to */
	var GearPawn	Recipient;
	/** Which event type it was. */
	var EGUDEventID	ID;
	/** How long, in seconds, until this event should be handled */
	var	float		DelayTimeRemainingSec;
	/** Who this event was "referring to", if appropriate.  Mainly used for responses. */
	var GearPawn	ReferencedPawn;
	/** TRUE if this event was triggered in response to another GUDS line. */
	var bool		bIsResponseEvent;
};
/** Queue of events that have been received since the last Tick */
var	protected transient array<GUDEvent>		QueuedEvents;

/**
 *	The TableOfContents - if running on a cooked build...
 */
var transient GUDToC	TableOfContents;

/**
 *	Structure used to encapsulate loaded GUD items.
 */
struct native LoadedGUDAsset
{
	/** Time tracking for loaded GUD banks. */
	var transient float			LoadTime;
	/** Array of banks loaded. */
	var transient GUDBank		Bank;
	/** FaceFXAnimSets that go with the banks. */
	var transient FaceFXAnimSet	AnimSet;

	structcpptext
	{
		virtual void Serialize(FArchive& Ar)
		{
			Ar << LoadTime;
			Ar << Bank;
			Ar << AnimSet;
		}

		friend FArchive& operator<<(FArchive& Ar, FLoadedGUDAsset& LoadedAsset)
		{
			return Ar << LoadedAsset.LoadTime << LoadedAsset.Bank << LoadedAsset.AnimSet;
		}
	}
};

/**
 *	Structure used to store loaded GUD banks.
 *	This could be a single GUDData (un-cooked run),
 *	or could be a collection of GUDData banks broken up for a master list.
 */
struct native GUDBankCollection
{
	/** Time tracking for when the refcount went to zero. */
	var transient float					ZeroRefCountTime;
	/** The maximum number of additional banks available (from the TOC, if present). */
	var transient int					NumAvailableBanks;
	/** Array of loaded assets. */
	var transient array<LoadedGUDAsset>	LoadedAssets;
	/** Array of actors using this collection. */
	var transient array<actor>			Speakers;

	structcpptext
	{
		virtual void Serialize(FArchive& Ar)
		{
			Ar << ZeroRefCountTime;
			Ar << NumAvailableBanks;
			Ar << LoadedAssets;
			Ar << Speakers;
		}

		friend FArchive& operator<<(FArchive& Ar, FGUDBankCollection& Collection)
		{
			return Ar << Collection.ZeroRefCountTime
					  << Collection.NumAvailableBanks
					  << Collection.LoadedAssets
					  << Collection.Speakers;
		}
	}
};
/** Mapping of 'master GUD bank' name to GUDBankCollection */
var transient native Map_Mirror		GUDBankCollectionsMap{TMap<FString,FGUDBankCollection>};

/** How much memory is used by all loaded banks. (approximate, not precise) */
var transient int ApproxTotalUsedMemory;

/** Structure for tracking pending GUD bank loads back to the speaker. */
struct native GUDSpeakers
{
	var transient string		GUDBankName;
	var transient string		LoadGUDBankName;
	var transient array<actor>	Speakers;
	var transient int			ApproxLoadBankSize;
	var transient int			VarietyIndex;

	structcpptext
	{
		virtual void Serialize(FArchive& Ar)
		{
			Ar << GUDBankName;
			Ar << LoadGUDBankName;
			Ar << Speakers;
			Ar << ApproxLoadBankSize;
			Ar << VarietyIndex;
		}

		friend FArchive& operator<<(FArchive& Ar, FGUDSpeakers& Source)
		{
			return Ar << Source.GUDBankName << Source.LoadGUDBankName << Source.Speakers << Source.ApproxLoadBankSize << Source.VarietyIndex;
		}
	}
};

/**
 *	Pending GUDBanks that are being loaded.
 *	Maps package name to the GUD bank collection that it will belong to.
 */
var transient native Map_Mirror		PendingGUDBanksMap{TMap<FString,FGUDSpeakers>};

/**
 *	Pending packages that are being loaded.
 *	Maps the GUD name being loaded to the GUDName used by the pawns...
 */
var transient native Map_Mirror		PendingGUDBanksToPawnGUDBanksMap{TMap<FString,FString>};

/**
 *	Pending packages that are being loaded.
 *	Used to handle GUDBanks that get registered prior to the ToC finishing load.
 */
var transient native Map_Mirror		PendingGUDLoadsMap{TMap<FString,FGUDSpeakers>};

/**
 *	Pending packages that are being loaded.
 *	Used to handle LOC files leading to the content files.
 */
var transient native Map_Mirror		PendingGUDLocalizedLoadsMap{TMap<FString,FGUDSpeakers>};

/** true if the manager is in debug mode, false otherwise */
var transient config bool bDebugGUDEvents;
var transient bool bDebugGUDSLogging;

/** true to show spoken gud lines on the screen (handy for debugging */
var() transient bool bShowGUDDebugText;

/** Handy hook for manipulating frequencies of the overall GUDS system. */
var float GlobalChanceToPlayMultiplier;

/** TRUE if GUDs should be disabled, FALSE otherwise */
var private transient bool bDisabled;

/** TRUE if GUDS is disabled on a temporary basis */
var private transient byte TeamChannel[3];

var transient bool bForceInCombatGUDS;

var array<Actor> DeferredSpeakerRegistrations;


//
/** GUDS/Efforts browser stuff (debug) **/
//


/** GUDBrowser will stuff this value into the bCookerProcessed field of a line if someone marks it "looks good", etc */
const GUDBROWSER_LOOKSGOOD = 133;
const GUDBROWSER_LOOKSBAD = 134;

var int GUDBrowser_CurrentAction;
var int GUDBrowser_CurrentLine;
var bool GUDBrowser_CurrentMP;
var class<GUDBank> GUDBrowser_CurrentMasterBankClass;

struct native GUDBrowserCharType
{
	var name Name;
	var name SetPlayerCharName;
	var string MasterBankClassName;
	var array<FaceFXAnimSet> LoadedFaceFXAnimSets;
};
var private array<GUDBrowserCharType> GUDBrowserChars;
var int GUDBrowser_CurrentCharType;

/** Internal, for GUDS Browser debug functionality. */
var transient bool bEnableGUDSBrowsing;



const EFFORTSBROWSER_LOOKSGOOD = 1.019f;
const EFFORTSBROWSER_LOOKSBAD = 0.994f;

struct native EffortBrowserCharType
{
	var name SetPlayerCharName;
	var string SoundGroupObjectName;
	var FaceFXAnimSet LoadedFaceFXAnimSet;
};
var array<EffortBrowserCharType> EffortsBrowserChars;

var int EffortsBrowser_CurrentCharType;
var int EffortsBrowser_CurrentEffortID;
var int EffortsBrowser_CurrentSoundIdx;

var transient GearSoundGroup EffortsBrowser_CurrentGSG;

var transient bool bEnableEffortsBrowsing;





//
/** Streaming vars **/
//

var protected transient bool	bDisableGUDSStreaming;
/** Time of last streaming update. */
var protected transient float	LastStreamingUpdateTime;
/** How often to update streaming. */
var protected const float		StreamingUpdateIntervalSec;
/** */
var protected const float		LastVarietyBankSwapTime;
/** How often to attempt to cycle out variety banks for new ones. */
var protected const float		VarietyBankSwapIntervalSec;
/** Upper limit on GUDS data that can be loaded at once. */
var protected const int			GUDSMemoryBudget;
/** How long to leave an orphaned root bank (no pawns left referencing it) before unloading it. */
var protected const float		OrphanedRootBankUnloadDelay;
/** Max number of variety banks to load for a particular character.  Keeps memory use down in low-char-count situations. */
var protected const int			VarietyBankCap;

cpptext
{
public:
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);

	// AActor interface...
	virtual void Spawned();
	/**
	 *	TickSpecial	- called by AActor::Tick
	 *	@param	DeltaSeconds	The time since the last tick.
	 */
	virtual void TickSpecial(FLOAT DeltaSeconds);

protected:
	UBOOL IsObserverLine(EGUDEventID EventID, EGUDActionID ActionID) const;
	UBOOL SpeakerHasLineForAction(AActor& Speaker, UINT ActionID) const;
	EGUDActionID ChooseActionToPlay_Normal(const struct FGUDEvent& Event, class AActor*& Speaker) const;
	EGUDActionID ChooseActionToPlay_Directional(const struct FGUDEvent& Event, class AActor*& Speaker) const;
	EGUDActionID ChooseActionToPlay_LocationDescription(const struct FGUDEvent& Event, class AActor*& Speaker) const;
	AActor* FindWitnessSpeaker(FGUDEvent const& Event, UINT ActionID, UBOOL bIncludeTeammates, UBOOL bIncludeEnemies, UBOOL bIncludeRemoteSpeakers) const;

	UBOOL LoadGUDBank(AActor* Speaker, const FString& GUDBankName, const FString& FaceFXAnimSetName);
	UBOOL UnloadGUDBank(AActor* Speaker);

	INT GetAvailableGUDSMemory() const;

public:
	/**
	 *	Process the given package, treating it as the TableOfContents.
	 *
	 *	@param	InToCPackage	The ToC package.
	 *	@return	UBOOL			TRUE if ToC was loaded; FALSE otherwise.
	 */
	UBOOL ProcessTableOfContentsPackage(UObject* InToCPackage);

	/**
	 *	Process the given package, treating it as a LocalizedGUDBank.
	 *
	 *	@param	InLocalizedGUDBank	The package to process.
	 *	@return	UBOOL				TRUE if it was processed; FALSE otherwise.
	 */
	UBOOL ProcessLocalizedGUDBankPackage(UObject* InLocalizedGUDBank);

	/**
	 *	Process the given package, treating it as a GUDBank.
	 *
	 *	@param	InGUDBank		The package to process.
	 *	@return	UBOOL			TRUE if it was processed; FALSE otherwise.
	 */
	UBOOL ProcessGUDBankPackage(UObject* InGUDBank);

	/**
	 *	Kick off a load of the localized version of the given GUDBank package...
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *	@param	LoadGUDBankName		The load (package) name of the GUDBank.
	 *	@param	Speaker				The speaker to register for it.
	 *	@param	Speakers			The array of speakers to register for it.
	 *	@param	VarietyIndex		The variety index being loaded, or -1 for root.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed (ie, there is no loc version).
	 */
	UBOOL LoadLocalizedPackage(const FString& GUDBankName, const FString& GUDBankLoadName, AActor* Speaker, TArrayNoInit<AActor*>* Speakers, INT VarietyIndex=-1);

	/**
	 *	Kick off a load of the given GUDBank package...
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *	@param	LoadGUDBankName		The load (package) name of the GUDBank.
	 *	@param	Speaker				The speaker to register for it.
	 *	@param	Speakers			The array of speakers to register for it.
	 *	@param	VarietyIndex		The variety index being loaded, or -1 for root.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
	 */
	UBOOL LoadGUDBankPackage(const FString& GUDBankName, const FString& LoadGUDBankName, AActor* Speaker, TArrayNoInit<AActor*>* Speakers, INT VarietyIndex=-1);

	/**
	 *	Completely remove the given GUDBank.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if not.
	 */
	UBOOL FlushGUDBank(const FString& GUDBankName);

	/**
	 *	See if the given variety bank is loaded.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *	@param	VarietyIndex		The index of the bank to check.
	 *
	 *	@return	UBOOL				TRUE if loaded; FALSE if not.
	 */
	UBOOL IsVarietyBankLoaded(const FString& GUDBankName, INT VarietyIndex);

	/**
	 *	Load the given variety bank for the GUDBankName collection.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *	@param	VarietyIndex		The index of the bank to load. If -1, pick a random one.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
	 */
	UBOOL LoadVarietyGUDBank(const FString& GUDBankName, INT VarietyIndex);

	/**
	 *	Unload the given variety bank for the GUDBankName collection.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *	@param	VarietyIndex		The index of the bank to load.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
	 */
	UBOOL UnloadVarietyGUDBank(const FString& GUDBankName, INT VarietyIndex);

	/**
	 *	Load all the variety banks for the given source GUDBank.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
	 */
	UBOOL LoadAllVarietyBanks(const FString& GUDBankName);

	/**
	 *	Unload all the variety banks for the given source GUDBank.
	 *
	 *	@param	GUDBankName			The name of the GUDBank.
	 *
	 *	@return	UBOOL				TRUE if successful; FALSE if it failed.
	 */
	UBOOL UnloadAllVarietyBanks(const FString& GUDBankName);

	// Test function...
	void RunTest(const FString& InPawnName, TArray<FString>& Tokens);

	/** This will log out all of the currently loaded guds data to the passed in FOutputDevice **/
	void LogoutStreamedPackageData( FOutputDevice& Ar );
}

native simulated function bool RegisterSpeaker(Actor Speaker);
native simulated function bool UnregisterSpeaker(Actor Speaker);

/** Functions for loading/unloading variety banks for a given pawn. */
/**
 *	Get the GUDBankName of the given speaker.
 *
 *	@param	Speaker		The speaker to get the GUDBankName for.
 *	@return	string		The GUDBankName.
 */
native function string GetSpeakerGUDBankName(Actor Speaker);

/**
 *	Load in BankCount random variety banks for the given Speaker.
 *
 *	@param	Speaker				The speaker to key on.
 *	@param	BankCount			The number of banks to load.
 *	@param	bUnloadEqualNumber	If TRUE, the same number of banks loaded will be unloaded.
 *	@return bool				TRUE if successful; FALSE if not.
 */
native function bool LoadRandomVarietyBanks(Actor Speaker, int BankCount, bool bUnloadEqualNumber);
/**
 *	Unload the oldest BankCount variety banks for the given Speaker.
 *
 *	@param	Speaker			The speaker to key on.
 *	@param	BankCount		The number of banks to unload.
 *	@return bool			TRUE if successful; FALSE if not.
 */
native function bool UnloadOldestVarietyBanks(Actor Speaker, int BankCount);
/**
 *	Unload all of the variety banks for the given Speaker.
 *
 *	@param	Speaker			The speaker to key on.
 *	@return bool			TRUE if successful; FALSE if not.
 */
native function bool UnloadAllVarietyBanks(Actor Speaker);
/**
 *	Unload all of the variety banks for ALL Speakers.
 *
 *	@return bool			TRUE if successful; FALSE if not.
 */
native function bool FlushOutAllVarietyBanks();

simulated function PostBeginPlay()
{
	EventProperties.Length = GUDEvent_MAX;
	EventProperties[GUDEvent_PickedUpNewWeapon].LinkedEvents[0] = GUDEvent_PickedUpAmmo;
	EventProperties[GUDEvent_PickedUpAmmo].LinkedEvents[0] = GUDEvent_PickedUpNewWeapon;
}

final function SetDisabled(bool bNewDisabled)
{
	bDisabled = bNewDisabled;
}

private final function TemporaryDisableFinished_0()
{
	TeamChannel[0] = 0;
}
private final function TemporaryDisableFinished_1()
{
	TeamChannel[1] = 0;
}
private final function TemporaryDisableFinished_2()
{
	TeamChannel[2] = 0;
}


/** Tell GUDS that someone is saying something we don't want to be talked over. */
native final function NotifyExclusiveSpeech(GearPawn WP, SoundCue Audio, ESpeechPriority SpeechPriority);

/** Returns TRUE if GUDS is cool to play, false otherwise */
native final function bool IsOkToPlayGUDSLine(int TeamNum, optional bool bResponseEvent) const;

/**
 * Instigator or Recipient can be None if appropriate for the event.
 */
final function TriggerGUDEvent(EGUDEventID ID, Pawn InInstigator, optional Pawn Recipient, optional float DelaySec)
{
	TriggerGUDEventInternal(ID, InInstigator, Recipient, None, DelaySec, FALSE);
}

/** */
private native final function TriggerGUDEventInternal(EGUDEventID ID, Pawn InInstigator, Pawn Recipient, Pawn ReferringTo, float DelaySec, bool bResponseEvent);

/** Internal.  Retrieves relevant GearPawn, handling turrets, vehicles, etc. */
native private final function GearPawn GetGUDSPawn(Pawn P) const;

simulated function Tick(float DeltaTime)
{
	local int Idx;
	local GUDEvent CurrentEvent;

	super.Tick(DeltaTime);

	/*
	if ( !IsOkToPlayGUDSLine() )
	{
		`log("Flushing guds lines");
		// flush array during temp disables.  this takes care of a temp disable coming in when delayed events are
		// lingering in the queue
		QueuedEvents.Length = 0;
	}
	*/

	// spin through the queued events and handle them in the order they came in.
	for(Idx=0; Idx<QueuedEvents.Length; ++Idx)
	{
		if (QueuedEvents[Idx].DelayTimeRemainingSec <= 0.f)
		{
			// check mintimebetween again, since multiple copies of the same event can come in in the same frame
			// so an instance of it may have been handled earlier in this loop
			if ( ( (WorldInfo.TimeSeconds - EventProperties[QueuedEvents[Idx].ID].TimeLastPlayed) > EventProperties[QueuedEvents[Idx].ID].MinTimeBetweenSec ) || bDebugGUDEvents )
			{
				// compiler won't let us pass individual elements of a dynamic array as the value for an out parm, so use a local
				//@fixme ronp - TTPRO #40059
				CurrentEvent = QueuedEvents[Idx];
				if ( (CurrentEvent.Instigator == None) || IsOkToPlayGUDSLine(CurrentEvent.Instigator.GetTeamNum(), CurrentEvent.bIsResponseEvent) )
				{
					HandleGUDEvent(CurrentEvent);
				}
				else
				{
					// skip this line
					`GUDSLog("Discarding GUDEvent:"@QueuedEvents[Idx].ID@"bDisabled="@bDisabled@"TeamChannel"@CurrentEvent.Instigator.GetTeamNum()@"="@TeamChannel[CurrentEvent.Instigator.GetTeamNum()]@"GlobalChanceToPlayMultiplier="@GlobalChanceToPlayMultiplier);
				}
			}
//			else
//			{
//				`log("ignoring GUDEvent"@QueuedEvents[Idx].ID@EventProperties[QueuedEvents[Idx].ID].TimeLastPlayed@WorldInfo.TimeSeconds@EventProperties[QueuedEvents[Idx].ID].MinTimeBetweenSec);
//			}

			// event got a chance to be handled, remove it from the queue
			QueuedEvents.Remove(Idx, 1);
			--Idx;
		}
		else
		{
			QueuedEvents[Idx].DelayTimeRemainingSec -= DeltaTime;
		}
	}

	if ( (Role == ROLE_Authority) && (bDisabled || (GlobalChanceToPlayMultiplier <= 0.f)) )
	{
		// tell BSM we're force disabled.  Note that we don't want to do this
		// for bTempDisabled
		GearGame(WorldInfo.Game).BattleMonitor.NotifyGUDSDisabled();
	}
}

/**
 * Given an Action, find a voice line to play.  Will return None for the speaker if no line is chosen.
 */
native protected simulated final function ChooseLineFromAction(EGUDActionID ActionID, Actor Speaker, out GUDLine Line, optional out GearPawn Addressee, optional out GearPawn ReferringTo, optional const out GUDEvent Event);

/**
 * Given an event, find a voice line to play.  Will return None for the speaker if no line is chosen.
 */
native protected simulated final function ChooseLineFromEvent(const out GUDEvent Event, out GUDLine Line, out Actor Speaker, out GearPawn Addressee, out GearPawn ReferringTo, out EGUDActionID Action);


simulated function string DebugGetSpeakerRole(EGUDActionID ActionID, EGUDEventID EventID)
{
	local int ActionIDIdx;

	ActionIDIdx = EventProperties[EventID].ActionIDs.Find(ActionID);

	if (ActionIDIdx == INDEX_NONE)
	{
		return "UNKNOWN ROLE";
	}
	if (EventProperties[EventID].ChooseActionMethod == GUDChooseMethod_Normal)
	{
		switch (ActionIDIdx)
		{
		case GUDActionMap_Instigator:
			return "Instigator";
		case GUDActionMap_Recipient:
			return "Recipient";
		case GUDActionMap_TeamWitness:
			return "Teammate Witness";
		case GUDActionMap_EnemyWitness:
			return "Enemy Witness";
		}
	}
	else if (EventProperties[EventID].ChooseActionMethod == GUDChooseMethod_Directional)
	{
		return "Directional,fixme";
	}
	else if (EventProperties[EventID].ChooseActionMethod == GUDChooseMethod_LocationDescription)
	{
		return "LocationDescription,fixme";
	}

	return "UNKNOWN ROLE";
}

/** Used to perform a GUDS action without going through the event system.  Useful for specific one-off lines, e.g. Boomer "boom!" or certain Anya lines.  */
simulated native final function PlayGUDSAction(EGUDActionID ActionID, Actor Speaker, optional GearPawn Addressee, optional GearPawn ReferringTo, optional ESpeakLineBroadcastFilter MPBroadcastFilter);

/** An event happened, try to play something. */
protected native final function HandleGUDEvent(const out GUDEvent Event);

/** Returns true if a line was actually spoken successfully. */
native final protected simulated function bool PlayGUDSLineInternal(const out GUDLine LineToPlay, Actor Speaker, GearPawn Addressee, GearPawn ReferringTo, optional float DelaySec, optional ESpeakLineBroadcastFilter MPBroadcastFilter);

/** tells the client to load all GUDS banks we have loaded. For use when a player joins midgame.
 * No effect if player is local or a child connection (split-screen client)
 */
native final function ReplicateLoadedGUDBanks(GearPC PC);

simulated native function DoDeferredSpeakerRegistrations();


/** hacktastic guds browser */
function GUDBrowserRender(Canvas Canvas)
{
	local float OldHUDCursorX, OldHUDCursorY;
	local Font		OldFont;
	local EGUDActionID ActionID;
	local GUDLine CurLine;
	local int LastGUDActionIdx;

	// save
	OldHUDCursorX = Canvas.CurX;
	OldHUDCursorY = Canvas.CurY;
	OldFont = Canvas.Font;

	Canvas.Font = class'Engine'.static.GetSmallFont();

	// start pos
	Canvas.SetPos(50,200);
	Canvas.SetDrawColor(255, 255, 255);
	Canvas.DrawText("Bank (LT/RT):"@GUDBrowser_CurrentMasterBankClass@GUDBrowser_CurrentCharType);
	Canvas.DrawText("     MP? (Y):"@GUDBrowser_CurrentMP);
	ActionID = EGUDActionID(GUDBrowser_CurrentAction);
	LastGUDActionIdx = GUDAction_MAX - 1;
	Canvas.DrawText("Action (LB/RB):"@ActionID@GUDBrowser_CurrentAction $ "/" $ LastGUDActionIdx);
	
	CurLine = GUDBrowserGetCurrentGUDLine();
	Canvas.DrawText("Line (X/B, A to play):"@CurLine.Audio@GUDBrowser_CurrentLine+1 $ "/" $ GUDBrowserGetNumLines());

	if (CurLine.bCookerProcessed == GUDBROWSER_LOOKSGOOD)
	{
		Canvas.SetDrawColor(32, 255, 32);
		Canvas.DrawText("Rated: Looks Good");
	}
	else if (CurLine.bCookerProcessed == GUDBROWSER_LOOKSBAD)
	{
		Canvas.SetDrawColor(255, 32, 32);
		Canvas.DrawText("Rated: Looks Bad");
	}
	else
	{
		Canvas.DrawText("unrated (L3=good, R3=bad)");
	}

	Canvas.SetPos(OldHUDCursorX, OldHUDCursorY);
	Canvas.Font = OldFont;
}


private function int GUDBrowserGetNumLines()
{
	if (GUDBrowser_CurrentMasterBankClass == None)
	{
		return 0;
	}

	if (GUDBrowser_CurrentAction >= GUDBrowser_CurrentMasterBankClass.default.GUDActions.length)
	{
		return 0;
	}

	return GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].LineIndices.length +
		   GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].CombatOnlyLineIndices.length;
}


private event int GUDBrowserGetCurrentGUDLineIndex()
{
	local int NumCombatLines, NumNonCombatLines, LineIndex, GUDLineIndex;

	if (GUDBrowser_CurrentMasterBankClass == None)
	{
		return INDEX_NONE;
	}

	NumNonCombatLines = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].LineIndices.length;
	NumCombatLines = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].CombatOnlyLineIndices.length;

	if ((NumCombatLines + NumNonCombatLines) == 0)
	{
		return INDEX_NONE;
	}

	// maybe fix line index
	if (GUDBrowser_CurrentLine >= NumNonCombatLines)
	{
		LineIndex = GUDBrowser_CurrentLine - NumNonCombatLines;
		GUDLineIndex = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].CombatOnlyLineIndices[LineIndex];
	}
	else
	{
		LineIndex = GUDBrowser_CurrentLine;
		GUDLineIndex = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].LineIndices[LineIndex];
	}

	return GUDLineIndex;
}


private function GUDLine GUDBrowserGetCurrentGUDLine()
{
	local GUDLine NullLine;
	local int GUDLineIndex;

	GUDLineIndex = GUDBrowserGetCurrentGUDLineIndex();

	if (GUDLineIndex == INDEX_NONE)
	{
		NullLine.Audio = None;
		return NullLine;
	}

	return GUDBrowser_CurrentMasterBankClass.default.GUDLines[GUDLineIndex];
}

private function SoundCue GUDBrowserGetCurrentSoundCue()
{
	local GUDLine Line;
	Line = GUDBrowserGetCurrentGUDLine();
	return Line.Audio;
}

private function GUDBrowserPlayLine()
{
	local GearPC GPC;
	foreach LocalPlayerControllers(class'GearPC', GPC) break;
	GPC.MyGearPawn.SpeakLine(None, GUDBrowserGetCurrentSoundCue(), "", 0.f, Speech_Immediate, SIC_Always, TRUE);
}


private function GUDBrowserLoadMasterClass()
{
	local string MasterClassName;
	local FaceFXAnimSet FaceFX;
	local GearPC GPC;
	local int Idx;

	// change character
	ConsoleCommand("setplayerchar "$GUDBrowserChars[GUDBrowser_CurrentCharType].SetPlayerCharName);

	// get gud data class
	MasterClassName = "GearGameContent." $ GUDBrowserChars[GUDBrowser_CurrentCharType].MasterBankClassName;
	if (GUDBrowser_CurrentMP)
	{
		MasterClassName = MasterClassName $ "MP";
	}
	GUDBrowser_CurrentMasterBankClass = class<GUDBank>(DynamicLoadObject(MasterClassName, class'class'));


	// load facefx
	foreach LocalPlayerControllers(class'GearPC', GPC) break;
	for (Idx=0; Idx<GPC.MyGearPawn.FAS_ChatterNames.length; ++Idx)
	{
		// load and mount facefx
		FaceFX = FaceFXAnimSet( DynamicLoadObject(GPC.MyGearPawn.FAS_ChatterNames[Idx], class'FaceFXAnimSet'));
		GPC.MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(FaceFX);
		GUDBrowserChars[GUDBrowser_CurrentCharType].LoadedFaceFXAnimSets[Idx] = FaceFX;
	}

	

	//for (Idx=0; Idx<GUDBrowserChars[GUDBrowser_CurrentCharType].FaceFXAnimSetName.length; ++Idx)
	//{
	//	if (GUDBrowserChars[GUDBrowser_CurrentCharType].FaceFXAnimSetName[Idx] != "")
	//	{
	//		// find pawn
	//		foreach LocalPlayerControllers(class'GearPC', GPC) break;

	//		// load and mount facefx
	//		FaceFX = FaceFXAnimSet( DynamicLoadObject(GUDBrowserChars[GUDBrowser_CurrentCharType].FaceFXAnimSetName[Idx], class'FaceFXAnimSet'));
	//		GPC.MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(FaceFX);
	//		GUDBrowserChars[GUDBrowser_CurrentCharType].LoadedFaceFXAnimSets[Idx] = FaceFX;
	//	}
	//}

}
private function GUDBrowserToggleMPBank()
{
	GUDBrowser_CurrentMP = !GUDBrowser_CurrentMP;
	GUDBrowserLoadMasterClass();
}
private function GUDBrowserGotoPrevBank()
{
	if (--GUDBrowser_CurrentCharType < 0)
	{
		GUDBrowser_CurrentCharType = GUDBrowserChars.length - 1;
	}
	GUDBrowserLoadMasterClass();


	GUDBrowser_CurrentAction = 0;
	GUDBrowser_CurrentLine = 0;
	GUDBrowserPlayLine();
}
private function GUDBrowserGotoNextBank()
{
	if (++GUDBrowser_CurrentCharType >= GUDBrowserChars.length)
	{
		GUDBrowser_CurrentCharType = 0;
	}
	GUDBrowserLoadMasterClass();

	GUDBrowser_CurrentAction = 0;
	GUDBrowser_CurrentLine = 0;
	GUDBrowserPlayLine();
}
private function GUDBrowserGotoPrevAction()
{
	local int StartAction;
	StartAction = GUDBrowser_CurrentAction;

	do
	{
		if (--GUDBrowser_CurrentAction < 0)
		{
			GUDBrowser_CurrentAction = GUDAction_MAX - 1;
		}
	}
	until ( (GUDBrowserGetNumLines() > 0) || (GUDBrowser_CurrentAction == StartAction) );

	GUDBrowser_CurrentLine = 0;
	GUDBrowserPlayLine();
}
private function GUDBrowserGotoNextAction()
{
	local int StartAction;
	StartAction = GUDBrowser_CurrentAction;

	do
	{
		if (++GUDBrowser_CurrentAction >= GUDAction_MAX)
		{
			GUDBrowser_CurrentAction = 0;
		}
	}
	until ( (GUDBrowserGetNumLines() > 0) || (GUDBrowser_CurrentAction == StartAction) );

	GUDBrowser_CurrentLine = 0;
	GUDBrowserPlayLine();
}
private function GUDBrowserGotoNextLine()
{
	local int NumLines;
	NumLines = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].LineIndices.length + GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].CombatOnlyLineIndices.length;
	if (++GUDBrowser_CurrentLine >= NumLines)
	{
		GUDBrowser_CurrentLine = 0;
	}
	GUDBrowserPlayLine();
}
private function GUDBrowserGotoPrevLine()
{
	local int NumLines;
	if (--GUDBrowser_CurrentLine < 0)
	{
		NumLines = GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].LineIndices.length + GUDBrowser_CurrentMasterBankClass.default.GUDActions[GUDBrowser_CurrentAction].CombatOnlyLineIndices.length;
		GUDBrowser_CurrentLine = NumLines - 1;
	}
	GUDBrowserPlayLine();
}

/** modifying class defaults, need to be in native land. */
private native function GUDBrowserMarkLineLooksGood();
private native function GUDBrowserMarkLineLooksBad();

function bool HandleButtonPress(Name ButtonName)
{
	if (bEnableGUDSBrowsing)
	{
		switch (ButtonName)
		{
		case 'L2':
			GUDBrowserGotoPrevBank();
			break;
		case 'R2':
			GUDBrowserGotoNextBank();
			break;
		case 'L1':
			GUDBrowserGotoPrevAction();
			break;
		case 'R1':
			GUDBrowserGotoNextAction();
			break;
		case 'X':
			GUDBrowserGotoPrevLine();
			break;
		case 'B':
			GUDBrowserGotoNextLine();
			break;
		case 'A':
			GUDBrowserPlayLine();
			break;
		case 'Y':
			GUDBrowserToggleMPBank();
			break;
		case 'L3':
			GUDBrowserMarkLineLooksGood();
			break;
		case 'R3':
			GUDBrowserMarkLineLooksBad();
			break;
		}
		return TRUE;
	}
	else if (bEnableEffortsBrowsing)
	{
		switch (ButtonName)
		{
		case 'L2':
			EffortsBrowserGotoPrevGroup();
			break;
		case 'R2':
			EffortsBrowserGotoNextGroup();
			break;
		case 'L1':
			EffortsBrowserGotoPrevEffort();
			break;
		case 'R1':
			EffortsBrowserGotoNextEffort();
			break;
		case 'X':
			EffortsBrowserGotoPrevCue();
			break;
		case 'B':
			EffortsBrowserGotoNextCue();
			break;
		case 'A':
			EffortsBrowserPlayLine();
			break;
		case 'L3':
			EffortsBrowserMarkLineLooksGood();
			break;
		case 'R3':
			EffortsBrowserMarkLineLooksBad();
			break;
		}

		return true;
	}
}


/** hacktastic effects browser */
function EffortsBrowserRender(Canvas Canvas)
{
	local float OldHUDCursorX, OldHUDCursorY;
	local Font		OldFont;
	local GearVoiceEffortID EffortID;
	local int LastEffortIdx;
	local SoundCue Cue;

	Cue = EffortsBrowserGetCurrentCue();

	// save
	OldHUDCursorX = Canvas.CurX;
	OldHUDCursorY = Canvas.CurY;
	OldFont = Canvas.Font;

	Canvas.Font = class'Engine'.static.GetSmallFont();

	// start pos
	Canvas.SetPos(50,200);
	Canvas.SetDrawColor(255, 255, 255);
	Canvas.DrawText("SoundGroup (LT/RT):"@EffortsBrowserChars[EffortsBrowser_CurrentCharType].SoundGroupObjectName@EffortsBrowser_CurrentCharType@"/"@EffortsBrowserChars.length-1);
	EffortID = GearVoiceEffortID(EffortsBrowser_CurrentEffortID);
	LastEffortIdx = GearEffort_MAX - 1;
	Canvas.DrawText("Effort (LB/RB):"@EffortID@EffortsBrowser_CurrentEffortID $ "/" $ LastEffortIdx);

	Canvas.DrawText("SoundCue (X/B, A to play):"@Cue@EffortsBrowser_CurrentSoundIdx+1 $ "/" $ EffortsBrowserGetNumCues());

	if (Cue != None)
	{
		if (Cue.VolumeMultiplier == EFFORTSBROWSER_LOOKSGOOD)
		{
			Canvas.SetDrawColor(32, 255, 32);
			Canvas.DrawText("Rated: Looks Good");
		}
		else if (Cue.VolumeMultiplier == EFFORTSBROWSER_LOOKSBAD)
		{
			Canvas.SetDrawColor(255, 32, 32);
			Canvas.DrawText("Rated: Looks Bad");
		}
		else
		{
			Canvas.DrawText("unrated (L3=good, R3=bad)");
		}
	}

	Canvas.SetPos(OldHUDCursorX, OldHUDCursorY);
	Canvas.Font = OldFont;
}

private function int EffortsBrowserGetNumCues()
{
	if (EffortsBrowser_CurrentGSG != None)
	{
		return EffortsBrowser_CurrentGSG.VoiceEfforts[EffortsBrowser_CurrentEffortID].Sounds.length;
	}

	return 0;
}
private function SoundCue EffortsBrowserGetCurrentCue()
{
	if ( (EffortsBrowser_CurrentGSG != None) && (EffortsBrowser_CurrentGSG.VoiceEfforts[EffortsBrowser_CurrentEffortID].Sounds.length > 0) )
	{
		return EffortsBrowser_CurrentGSG.VoiceEfforts[EffortsBrowser_CurrentEffortID].Sounds[EffortsBrowser_CurrentSoundIdx];
	}

	return None;
}


private function EffortsBrowserGotoPrevGroup()
{
	if (--EffortsBrowser_CurrentCharType < 0)
	{
		EffortsBrowser_CurrentCharType = EffortsBrowserChars.length - 1;
	}

	EffortsBrowserInitCurrentGroup();

	EffortsBrowser_CurrentSoundIdx = 0;
	EffortsBrowser_CurrentEffortID = 1;
	EffortsBrowserPlayLine();
}

private function EffortsBrowserInitCurrentGroup()
{
//	local GearPC GPC;
//	local FaceFXAnimSet FaceFX;

	ConsoleCommand("setplayerchar "$EffortsBrowserChars[EffortsBrowser_CurrentCharType].SetPlayerCharName);

	EffortsBrowser_CurrentGSG = GearSoundGroup(DynamicLoadObject(EffortsBrowserChars[EffortsBrowser_CurrentCharType].SoundGroupObjectName,class'GearSoundGroup'));

	//if (EffortsBrowserChars[EffortsBrowser_CurrentCharType].FaceFXAnimSetName != "")
	//{
	//	// find pawn
	//	foreach LocalPlayerControllers(class'GearPC', GPC) break;

	//	// load and mount facefx
	//	FaceFX = FaceFXAnimSet( DynamicLoadObject(EffortsBrowserChars[EffortsBrowser_CurrentCharType].FaceFXAnimSetName, class'FaceFXAnimSet'));
	//	GPC.MyGearPawn.Mesh.SkeletalMesh.FaceFXAsset.MountFaceFXAnimSet(FaceFX);
	//	EffortsBrowserChars[EffortsBrowser_CurrentCharType].LoadedFaceFXAnimSet = FaceFX;
	//}
}


private function EffortsBrowserGotoNextGroup()
{
	if (++EffortsBrowser_CurrentCharType >= EffortsBrowserChars.length)
	{
		EffortsBrowser_CurrentCharType = 0;
	}

	EffortsBrowserInitCurrentGroup();


	EffortsBrowser_CurrentSoundIdx = 0;
	EffortsBrowser_CurrentEffortID = 1;
	EffortsBrowserPlayLine();
}
private function EffortsBrowserGotoPrevEffort()
{
	if (--EffortsBrowser_CurrentEffortID < 1)			// 0 is empty
	{
		EffortsBrowser_CurrentEffortID = GearEffort_MAX - 1;
	}

	EffortsBrowser_CurrentSoundIdx = 0;
	EffortsBrowserPlayLine();
}
private function EffortsBrowserGotoNextEffort()
{
	if (++EffortsBrowser_CurrentEffortID >= GearEffort_MAX)
	{
		EffortsBrowser_CurrentEffortID = 1;				// 0 is empty
	}

	EffortsBrowser_CurrentSoundIdx = 0;
	EffortsBrowserPlayLine();
}
private function EffortsBrowserGotoPrevCue()
{
	if (--EffortsBrowser_CurrentSoundIdx < 0)
	{
		EffortsBrowser_CurrentSoundIdx = EffortsBrowserGetNumCues() - 1;
	}

	EffortsBrowserPlayLine();
}
private function EffortsBrowserGotoNextCue()
{
	if (++EffortsBrowser_CurrentSoundIdx >= EffortsBrowserGetNumCues())
	{
		EffortsBrowser_CurrentSoundIdx = 0;
	}

	EffortsBrowserPlayLine();
}
private function EffortsBrowserPlayLine()
{
	local GearPC GPC;
	foreach LocalPlayerControllers(class'GearPC', GPC) break;
	GPC.MyGearPawn.SpeakLine(None, EffortsBrowserGetCurrentCue(), "", 0.f, Speech_Immediate, SIC_Always, TRUE);
}

private function EffortsBrowserMarkLineLooksGood()
{
	local SoundCue Cue;
	Cue = EffortsBrowserGetCurrentCue();
	Cue.VolumeMultiplier = EFFORTSBROWSER_LOOKSGOOD;		// omg hack city
	`log("** EffortsBrowser: Cue"@Cue@"marked as LOOKS GOOD.");
}
private function EffortsBrowserMarkLineLooksBad()
{
	local SoundCue Cue;
	Cue = EffortsBrowserGetCurrentCue();
	Cue.VolumeMultiplier = EFFORTSBROWSER_LOOKSBAD;		// omg hack city
	`log("** EffortsBrowser: Cue"@Cue@"marked as LOOKS BAD.");
}


/** Hack to get the enum name in C++ code. */
simulated event string GetEventName(EGUDEventID ID)
{
	return ID$"";
}
/** Hack to get the enum name in C++ code. */
simulated event string GetActionName(EGUDActionID ID)
{
	return ID$"";
}


simulated native function DrawDebugStreaming(Canvas Canvas);

defaultproperties
{
	/**
	 * Currently 10 MB.  It averages a little under 1MB per character for the root bank
	 *  so this leaves headroom for a couple variety banks in the worst case.
	 */
	GUDSMemoryBudget=10485760		// 10mb

	LastStreamingUpdateTime=-9999.f
	StreamingUpdateIntervalSec=7.0f  // this does rebalancing of variety packs
	VarietyBankSwapIntervalSec=60.0f
	OrphanedRootBankUnloadDelay=300.f
	VarietyBankCap=3

	// stuff from actor
	TickGroup=TG_DuringAsyncWork
	RemoteRole=ROLE_SimulatedProxy
	bMovable=FALSE
	bCanBeDamaged=FALSE
	bReplicateMovement=FALSE
	bShowGUDDebugText=FALSE
	GlobalChanceToPlayMultiplier=1.f

	EventProperties(GUDEvent_EnteredCombat)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_EnteredCombat_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}
	EventProperties(GUDEvent_LostTarget)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=7.f,
		ActionIDs=(GUDAction_LostTarget_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}
	EventProperties(GUDEvent_FoundTarget)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=7.f,
		ActionIDs=(GUDAction_FoundTarget_I,GUDAction_FoundTarget_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}

	EventProperties(GUDEvent_KilledEnemyGeneric)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_KilledEnemyGeneric_I,GUDAction_None,GUDAction_KilledEnemyGeneric_TW,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyHeadShot)={(
		ChanceToPlay=0.75f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_KilledEnemyHeadShot_I,GUDAction_None,GUDAction_KilledEnemyHeadShot_TW,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyChainsaw)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=20.f,
		ActionIDs=(GUDAction_KilledEnemyChainsaw_I,GUDAction_None,GUDAction_KilledEnemyChainsaw_TW,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyMelee)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_KilledEnemyMelee_I,GUDAction_None,GUDAction_KilledEnemyMelee_TW,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyCurbstomp)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_KilledEnemyCurbstomp_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyExecution)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_KilledEnemyExecution_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_KilledEnemyHOD)={(
		ChanceToPlay=0.6f,
		ActionIDs=(GUDAction_KilledEnemyHOD_I,GUDAction_None,GUDAction_KilledEnemyHOD_TW,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerTeamOnly*/)}
	EventProperties(GUDEvent_DamagedEnemy)={(
		ChanceToPlay=0.04f,
		ActionIDs=(GUDAction_DamagedEnemy_I,GUDAction_DamagedEnemy_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}
	EventProperties(GUDEvent_DamagedEnemyHeavy)={(
		ChanceToPlay=0.2f,
		ActionIDs=(GUDAction_DamagedEnemyHeavy_I,GUDAction_DamagedEnemyHeavy_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerAndAddresseeOnly)}
	EventProperties(GUDEvent_DamageTeammate)={(
		ChanceToPlay=0.2f,
		ActionIDs=(GUDAction_DamageTeammate_I,GUDAction_DamageTeammate_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_CauseEnemyStumble)={(
		ChanceToPlay=0.2f,
		ActionIDs=(GUDAction_CauseEnemyStumble_I,GUDAction_CauseEnemyStumble_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_TransToRetreatAI)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToRetreatAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToRetreatAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	EventProperties(GUDEvent_TransToHoldAI)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToHoldAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToHoldAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	EventProperties(GUDEvent_TransToAdvanceAI)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToAdvanceAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToAdvanceAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	EventProperties(GUDEvent_TransToKamikazeAI)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToKamikazeAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToKamikazeAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	EventProperties(GUDEvent_TransToMeleeAI)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=4.f,
		ActionIDs=(GUDAction_TransToMeleeAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToMeleeAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	// note: intentionally duplicated TransToMelee actions, this event exists to vary ChanceToPlay based on visibility
	EventProperties(GUDEvent_TransToMeleeAIOffscreen)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_TransToMeleeAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToMeleeAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}

	EventProperties(GUDEvent_TransToFlankAI)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToFlankAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToFlankAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}
	EventProperties(GUDEvent_TransToFleeHODAI)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_TransToFleeHODAI_I,GUDAction_None,GUDAction_None,GUDAction_TransToFleeHODAI_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}

	EventProperties(GUDEvent_ThrowingSmokeGrenade)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=7.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_ThrowingSmokeGrenade_I,GUDAction_None,GUDAction_None,GUDAction_ThrowingSmokeGrenade_EW),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_ThrowingFragGrenade)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=7.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_ThrowingFragGrenade_I,GUDAction_None,GUDAction_None,GUDAction_ThrowingFragGrenade_EW),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_FailedActiveReload)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,
		ActionIDs=(GUDAction_FailedActiveReload_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_SucceededActiveReload)={(
		ChanceToPlay=0.25f,MinTimeBetweenSec=5.f,
		ActionIDs=(GUDAction_SucceededActiveReload_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_Reloaded)={(
		ChanceToPlay=0.1f,
		ActionIDs=(GUDAction_Reloaded_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_PickedUpAmmo)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_PickedUpAmmo_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_PickedUpNewWeapon)={(
		ChanceToPlay=0.8f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_PickedUpNewWeapon_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_PickedUpCollectible)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_PickedUpCollectible_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_PickedUpGrenades)={(
		ChanceToPlay=0.75f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_PickedUpGrenades_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_PlayerHasntMoved)={(
		ChanceToPlay=0.1f,MinTimeBetweenSec=120.f,
		ActionIDs=(GUDAction_PlayerHasntMoved_I,GUDAction_None,GUDAction_PlayerHasntMoved_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	//EventProperties(GUDEvent_MysteriousEvent)=(ChanceToPlay=0.1f,ObserverDelay=0.f,TeammateWitnessActionID=GUDAction_MysteriousEvent_TW,MPBroadcastFilter=SLBFilter_SpeakerOnly)
	//EventProperties(GUDEvent_SurprisingEvent)=(ChanceToPlay=0.1f,ObserverDelay=0.f,TeammateWitnessActionID=GUDAction_SurprisingEvent_TW,MPBroadcastFilter=SLBFilter_SpeakerOnly)
	//EventProperties(GUDEvent_DangerousEvent)=(ChanceToPlay=0.1f,ObserverDelay=0.f,TeammateWitnessActionID=GUDAction_DangerousEvent_TW,MPBroadcastFilter=SLBFilter_SpeakerOnly)

	EventProperties(GUDEvent_NeedAmmo)={(
		ChanceToPlay=1.f,
		ActionIDs=(GUDAction_NeedAmmo_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}
	EventProperties(GUDEvent_NeedAmmoGrenade)={(
		ChanceToPlay=1.f,
		ActionIDs=(GUDAction_NeedAmmoGrenade_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}

	EventProperties(GUDEvent_GenericWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_GenericWentDown_I,GUDAction_None,GUDAction_GenericWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MarcusWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_MarcusWentDown_I,GUDAction_None,GUDAction_MarcusWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_DomWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_DomWentDown_I,GUDAction_None,GUDAction_DomWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_BairdWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_BairdWentDown_I,GUDAction_None,GUDAction_BairdWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MinhWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_MinhWentDown_I,GUDAction_None,GUDAction_MinhWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_GusWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_GusWentDown_I,GUDAction_None,GUDAction_GusWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_HoffmanWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_HoffmanWentDown_I,GUDAction_None,GUDAction_HoffmanWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_CarmineWentDown)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_CarmineWentDown_I,GUDAction_None,GUDAction_CarmineWentDown_TW,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_GenericNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_GenericNeedsRevived_I,GUDAction_None,GUDAction_GenericNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MarcusNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_MarcusNeedsRevived_I,GUDAction_None,GUDAction_MarcusNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_DomNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_DomNeedsRevived_I,GUDAction_None,GUDAction_DomNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_BairdNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_BairdNeedsRevived_I,GUDAction_None,GUDAction_BairdNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MinhNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_MinhNeedsRevived_I,GUDAction_None,GUDAction_MinhNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_GusNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_GusNeedsRevived_I,GUDAction_None,GUDAction_GusNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_HoffmanNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_HoffmanNeedsRevived_I,GUDAction_None,GUDAction_HoffmanNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_CarmineNeedsRevived)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_CarmineNeedsRevived_I,GUDAction_None,GUDAction_CarmineNeedsRevived_TW,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_Revived)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,
		ActionIDs=(GUDAction_Revived_I,GUDAction_Revived_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_Attack)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=25.f,
		ActionIDs=(GUDAction_Attack_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}

	EventProperties(GUDEvent_NoticedManyEnemies)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedManyEnemies_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedBoomer)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,ObserverDelay=0.5f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedBoomer_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedDrone)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedDrone_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedTheron)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedTheron_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedPalaceGuard)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedPalaceGuard_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedWretch)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedWretch_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedHunter)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedHunter_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_NoticedKantus)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedKantus_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedMauler)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedMauler_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedGrinder)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedGrinder_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedBloodmounts)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedBloodmounts_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedTickers)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedTickers_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedSnipers)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedSnipers_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedEnemyGeneric)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedEnemyGeneric_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_NoticedReinforcements)={(
		ChanceToPlay=0.2f,MinTimeBetweenSec=20.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_NoticedReinforcements_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_CombatLull)={(
		ChanceToPlay=0.5f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_CombatLull_TW,GUDAction_CombatLull_EW),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}

	EventProperties(GUDEvent_EHoleOpened)={(
		ChanceToPlay=1.f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_EHoleOpened_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_EHoleOpenReminder)={(
		ChanceToPlay=0.65f,ObserverDelay=0.f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_EHoleOpenReminder_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_EHoleClosedWithGrenade)={(
		ChanceToPlay=0.9f,
		ActionIDs=(GUDAction_EHoleClosedWithGrenade_I,GUDAction_None,GUDAction_EHoleClosedWithGrenade_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_EHoleGrenadeMissed)={(
		ChanceToPlay=0.9f,
		ActionIDs=(GUDAction_EHoleGrenadeMissed_I,GUDAction_None,GUDAction_EHoleGrenadeMissed_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_VersusRoundBegunCOG)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=0.5f,ForcedSpeakerRole=GUDRole_TeammateWitness,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_VersusRoundBegunCOG_TW,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}
	EventProperties(GUDEvent_VersusRoundBegunLocust)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=0.5f,ForcedSpeakerRole=GUDRole_EnemyWitness,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_None,GUDAction_VersusRoundBegunLocust_EW),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerTeamOnly)}

	EventProperties(GUDEvent_TeammateAnnouncedEnemySpotted)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=0.f,MaxWitnessDistSq=4000000.f,			// 2000*2000
		ForcedSpeakerRole=GUDRole_TeammateWitness,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_EnemySpottedResponse_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_TeammateAnnouncedEnemyDir)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=10.f,ObserverDelay=0.f,MaxWitnessDistSq=4000000.f,			// 2000*2000
		ForcedSpeakerRole=GUDRole_TeammateWitness,		// mintime here prevents loops
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_EnemyDirAnnouncedResponse_TW,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_EnteredCover)={(
		ChanceToPlay=0.75f,MinTimeBetweenSec=60.f,ObserverDelay=1.f,MaxWitnessDistSq=16000000.f,			// 4000*4000
		ChooseActionMethod=GUDChooseMethod_LocationDescription,ForcedSpeakerRole=GUDRole_EnemyWitness,
		ActionIDs=(GUDAction_None,GUDAction_EnemyLocationCallout_InWindow,GUDAction_EnemyLocationCallout_InDoorway,GUDAction_EnemyLocationCallout_BehindCar,GUDAction_EnemyLocationCallout_BehindTruck,GUDAction_EnemyLocationCallout_OnTruck,GUDAction_EnemyLocationCallout_BehindBarrier,GUDAction_EnemyLocationCallout_BehindColumn,GUDAction_EnemyLocationCallout_BehindCrate,GUDAction_EnemyLocationCallout_BehindWall,GUDAction_EnemyLocationCallout_BehindStatue,GUDAction_EnemyLocationCallout_BehindSandbags,GUDAction_EnemyLocationCallout_InTheOpen),			// see GUDActionMap_LocationDescription enum above
		)}

	EventProperties(GUDEvent_TeammateRequestedEnemyDir)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=1.f,MaxWitnessDistSq=4000000.f,			// 2000*2000
		ChooseActionMethod=GUDChooseMethod_Directional,ForcedSpeakerRole=GUDRole_TeammateWitness,
		ActionIDs=(GUDAction_EnemyLocationCallout_Above,GUDAction_EnemyLocationCallout_Below,GUDAction_EnemyLocationCallout_Right,GUDAction_EnemyLocationCallout_Left,GUDAction_EnemyLocationCallout_Ahead,GUDAction_EnemyLocationCallout_Behind),			// above, below, r, l, ahead, behind
		)}
	EventProperties(GUDEvent_TeammateRequestedEnemyLocDesc)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=1.f,MaxWitnessDistSq=4000000.f,			// 2000*2000
		ChooseActionMethod=GUDChooseMethod_LocationDescription,ForcedSpeakerRole=GUDRole_TeammateWitness,
		ActionIDs=(GUDAction_None,GUDAction_EnemyLocationCallout_InWindow,GUDAction_EnemyLocationCallout_InDoorway,GUDAction_EnemyLocationCallout_BehindCar,GUDAction_EnemyLocationCallout_BehindTruck,GUDAction_EnemyLocationCallout_OnTruck,GUDAction_EnemyLocationCallout_BehindBarrier,GUDAction_EnemyLocationCallout_BehindColumn,GUDAction_EnemyLocationCallout_BehindCrate,GUDAction_EnemyLocationCallout_BehindWall,GUDAction_EnemyLocationCallout_BehindStatue,GUDAction_EnemyLocationCallout_BehindSandbags,GUDAction_EnemyLocationCallout_InTheOpen),			// see GUDActionMap_LocationDescription enum above
		)}
	EventProperties(GUDEvent_DecidedToAnounceEnemyDir)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=1.f,
		ChooseActionMethod=GUDChooseMethod_Directional,ForcedSpeakerRole=GUDRole_Instigator,
		ActionIDs=(GUDAction_EnemyLocationCallout_Above,GUDAction_EnemyLocationCallout_Below,GUDAction_EnemyLocationCallout_Right,GUDAction_EnemyLocationCallout_Left,GUDAction_EnemyLocationCallout_Ahead,GUDAction_EnemyLocationCallout_Behind),			// above, below, r, l, ahead, behind
		)}
	EventProperties(GUDEvent_DecidedToAnounceEnemyLocDesc)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=0.f,ObserverDelay=1.f,
		ChooseActionMethod=GUDChooseMethod_LocationDescription,ForcedSpeakerRole=GUDRole_Instigator,
		ActionIDs=(GUDAction_None,GUDAction_EnemyLocationCallout_InWindow,GUDAction_EnemyLocationCallout_InDoorway,GUDAction_EnemyLocationCallout_BehindCar,GUDAction_EnemyLocationCallout_BehindTruck,GUDAction_EnemyLocationCallout_OnTruck,GUDAction_EnemyLocationCallout_BehindBarrier,GUDAction_EnemyLocationCallout_BehindColumn,GUDAction_EnemyLocationCallout_BehindCrate,GUDAction_EnemyLocationCallout_BehindWall,GUDAction_EnemyLocationCallout_BehindStatue,GUDAction_EnemyLocationCallout_BehindSandbags,GUDAction_EnemyLocationCallout_InTheOpen),			// see GUDActionMap_LocationDescription enum above
		)}
	EventProperties(GUDEvent_MeatFlagWentDBNO)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatflagWentDBNO_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagGrabbedByCOG)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagGrabbedByCOG_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagGrabbedByLocust)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagGrabbedByLocust_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagStillHeldByCOG)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=20.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagStillHeldByCOG_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagStillHeldByLocust)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=20.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagStillHeldByLocust_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagReleasedByCOG)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagReleasedByCOG_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagReleasedByLocust)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagReleasedByLocust_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagCapturedByCOG)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagCapturedByCOG_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagCapturedByLocust)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_None,GUDAction_MeatFlagCapturedByLocust_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagKilledFormerCOGCaptor)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_MeatFlagKilledFormerCOGCaptor_I, GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MeatFlagKilledFormerLocustCaptor)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=5.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_MeatFlagKilledFormerLocustCaptor_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_PickedUpMeatShield)={(
		ChanceToPlay=0.7f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_PickedUpMeatShield_I,GUDAction_PickedUpMeatShield_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_KilledEnemyFlamethrower)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=30.f,
		ActionIDs=(GUDAction_KilledEnemyFlamethrower_I,GUDAction_None,GUDAction_KilledEnemyFlamethrower_TW,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_PickedUpFlamethrower)={(
		ChanceToPlay=0.3f,MinTimeBetweenSec=30.f,
		ActionIDs=(GUDAction_PickedUpFlamethrower_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}

	EventProperties(GUDEvent_GatlingOverheated)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=20.f,
		ActionIDs=(GUDAction_GatlingOverheated_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_MortarLaunched)={(
		ChanceToPlay=1.f,MinTimeBetweenSec=3.f,ObserverDelay=0.75f,
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_None,GUDAction_MortarLaunched_EW),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_ThrowingInkGrenade)={(
		ChanceToPlay=1.0f,MinTimeBetweenSec=7.f,ObserverDelay=1.f,
		ActionIDs=(GUDAction_ThrowingInkGrenade_I,GUDAction_None,GUDAction_None,GUDAction_ThrowingInkGrenade_EW),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_StuckExplosiveToEnemy)={(
		ChanceToPlay=0.75f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_StuckExplosiveToEnemy_I,GUDAction_StuckExplosiveToEnemy_R,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		)}
	EventProperties(GUDEvent_GrenadeTrapSet)={(
		ChanceToPlay=0.5f,MinTimeBetweenSec=20.f,
		ActionIDs=(GUDAction_GrenadeTrapSet_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		MPBroadcastFilter=SLBFilter_SpeakerOnly)}

	EventProperties(GUDEvent_ExecutedMeatShield)={(
		ChanceToPlay=0.4f,MinTimeBetweenSec=10.f,
		ActionIDs=(GUDAction_ExecutedMeatShield_I,GUDAction_None,GUDAction_None,GUDAction_None),			// inst, rec, tw, ew
		/*MPBroadcastFilter=SLBFilter_SpeakerOnly*/)}

	EventProperties(GUDEvent_DBNOPawnCrawlingAway)={(
		ChanceToPlay=0.6f,MinTimeBetweenSec=10.f,MaxWitnessDistSq=640000.f,		// 800*800
		ActionIDs=(GUDAction_None,GUDAction_None,GUDAction_None,GUDAction_DBNOPawnCrawlingAway_EW),			// inst, rec, tw, ew
		)}

	GUDBrowserChars(0)=(Name="kantus",SetPlayerCharName="kantus",MasterBankClassName="GUDData_LocustKantus")
	GUDBrowserChars(1)=(Name="dronea",SetPlayerCharName="drone",MasterBankClassName="GUDData_LocustDrone")
	GUDBrowserChars(2)=(Name="droneb",SetPlayerCharName="drone",MasterBankClassName="GUDData_LocustDroneB")
	GUDBrowserChars(3)=(Name="drunk",SetPlayerCharName="drunk",MasterBankClassName="GUDData_NPCDrunk")
	GUDBrowserChars(4)=(Name="franklin",SetPlayerCharName="franklin",MasterBankClassName="GUDData_NPCFranklin")
	GUDBrowserChars(5)=(Name="oldman",SetPlayerCharName="oldman",MasterBankClassName="GUDData_NPCOldman")
	GUDBrowserChars(6)=(Name="grenadier",SetPlayerCharName="grenadier",MasterBankClassName="GUDData_LocustGrenadier")
	GUDBrowserChars(7)=(Name="boomera",SetPlayerCharName="boomer",MasterBankClassName="GUDData_LocustBoomer")
	GUDBrowserChars(8)=(Name="boomerb",SetPlayerCharName="butcherboomer",MasterBankClassName="GUDData_LocustBoomerB")
	GUDBrowserChars(9)=(Name="raam",SetPlayerCharName="raammp",MasterBankClassName="GUDData_LocustRaam")
	GUDBrowserChars(10)=(Name="theron",SetPlayerCharName="theron",MasterBankClassName="GUDData_LocustTheron")
	GUDBrowserChars(11)=(Name="dom",SetPlayerCharName="dom",MasterBankClassName="GUDData_COGDom")
	GUDBrowserChars(12)=(Name="gus",SetPlayerCharName="gus",MasterBankClassName="GUDData_COGGus")
	GUDBrowserChars(13)=(Name="baird",SetPlayerCharName="baird",MasterBankClassName="GUDData_COGBaird")
	GUDBrowserChars(14)=(Name="tai",SetPlayerCharName="tai",MasterBankClassName="GUDData_COGTai")
	GUDBrowserChars(15)=(Name="dizzy",SetPlayerCharName="dizzymp",MasterBankClassName="GUDData_COGDizzy")
	GUDBrowserChars(16)=(Name="hoffman",SetPlayerCharName="hoffman",MasterBankClassName="GUDData_COGHoffman")
	GUDBrowserChars(17)=(Name="minh",SetPlayerCharName="minh",MasterBankClassName="GUDData_COGMinh")
	GUDBrowserChars(18)=(Name="carmine",SetPlayerCharName="carmine",MasterBankClassName="GUDData_COGCarmine")
	GUDBrowserChars(19)=(Name="anya",SetPlayerCharName="marcus",MasterBankClassName="GUDData_COGAnya")
	GUDBrowserChars(20)=(Name="marcus",SetPlayerCharName="marcus",MasterBankClassName="GUDData_COGMarcus")
	GUDBrowserChars(21)=(Name="jack",SetPlayerCharName="marcus",MasterBankClassName="GUDData_COGJack")

	EffortsBrowserChars(0)=(SetPlayerCharName="marcus",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGMarcus")
	EffortsBrowserChars(1)=(SetPlayerCharName="dom",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGDom")
	EffortsBrowserChars(2)=(SetPlayerCharName="theron",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustTheron")
	EffortsBrowserChars(3)=(SetPlayerCharName="drone",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustDrone")
	EffortsBrowserChars(4)=(SetPlayerCharName="drone",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustDroneB")
	EffortsBrowserChars(5)=(SetPlayerCharName="baird",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGBaird")
	EffortsBrowserChars(6)=(SetPlayerCharName="carmine",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGCarmine")
	EffortsBrowserChars(7)=(SetPlayerCharName="dizzymp",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGDizzyMP")
	EffortsBrowserChars(8)=(SetPlayerCharName="gus",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGGus")
	EffortsBrowserChars(9)=(SetPlayerCharName="hoffman",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGHoffman")
	EffortsBrowserChars(10)=(SetPlayerCharName="minhmp",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGMinh")
	EffortsBrowserChars(11)=(SetPlayerCharName="tai",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_COGTai")
	EffortsBrowserChars(12)=(SetPlayerCharName="raammp",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustRaam")
	EffortsBrowserChars(13)=(SetPlayerCharName="boomer",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustBoomer")
	EffortsBrowserChars(14)=(SetPlayerCharName="boomer",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustBoomerB")
	EffortsBrowserChars(15)=(SetPlayerCharName="kantus",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_LocustKantus")
	EffortsBrowserChars(16)=(SetPlayerCharName="skorge",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_Skorge")
	EffortsBrowserChars(17)=(SetPlayerCharName="drunk",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_NPCDrunk")
	EffortsBrowserChars(18)=(SetPlayerCharName="franklin",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_NPCFranklin")
	EffortsBrowserChars(19)=(SetPlayerCharName="oldman",SoundGroupObjectName="GearSoundGroupArchetypes.GSG_NPCOldMan")
					
}
