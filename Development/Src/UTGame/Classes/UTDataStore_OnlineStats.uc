/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Specific derivation of UIDataStore_OnlineStats to expose the TDM leaderboards
 */
class UTDataStore_OnlineStats extends UIDataStore_OnlineStats
	native;

`include(UTOnlineConstants.uci)

/** Different read types. */
enum EStatsDetailsReadType
{
	UTSR_GeneralAndRewards,
	UTSR_Weapons,
	UTSR_Vehicles,
	UTSR_VehicleWeapons,
};

var transient EStatsDetailsReadType	CurrentDetailsReadType;

/** Whether or not we are currently in a read. */
var transient bool bInRead;

/** Whether or not we are currently pumping the read queue. */
var transient bool bInQueuePump;

/** NetId of the player we are retrieving detailed stats on. */
var transient UniqueNetId	DetailsPlayerNetId;

/** Nick of the player we are retrieving detailed stats on. */
var transient string		DetailsPlayerNick;

/** Game mode we are currently displaying stats for */
var transient int		GameModeId;
/** Match Type we are currently displaying stats for */
var transient int		MatchTypeId;
/** Start index for this set of online stats currently being read */
var transient int		StatsReadObjectStartIndex;

/** Row index to use for detailed stats info */
var transient int	DetailedStatsRowIndex;

/** The class to load and instance */
var class<Settings> LeaderboardSettingsClass;

/** The set of settings that are to be exposed to the UI for filtering the leaderboards */
var Settings LeaderboardSettings;

/** The data provider that will expose the leaderboard settings to the ui */
var UIDataProvider_Settings SettingsProvider;

/** Reference to the dataprovider that will provide general stats details for a stats row. */
var transient UTUIDataProvider_StatsGeneral GeneralProvider;

/** Reference to the dataprovider that will provide weapons details for a stats row. */
var transient UTUIDataProvider_StatsWeapons WeaponsProvider;

/** Reference to the dataprovider that will provide Vehicle stats details for a stats row. */
var transient UTUIDataProvider_StatsVehicles VehiclesProvider;

/** Reference to the dataprovider that will provide Vehicle Weapons stats details for a stats row. */
var transient UTUIDataProvider_StatsVehicleWeapons VehicleWeaponsProvider;

/** Reference to the dataprovider that will provide rewards stats details for a stats row. */
var transient UTUIDataProvider_StatsRewards RewardsProvider;

/** Array of stat events to read in order. */
var transient array<OnlineStatsRead> ReadQueue;

/**
* Delegate used to notify the caller when stats read has completed
*/
delegate OnStatsReadComplete(bool bWasSuccessful);



cpptext
{
	/**
	 * Initializes the dataproviders for all of the various character parts.
	 */
	virtual void InitializeDataStore();

	/**
	 * Builds a list of available fields from the array of properties in the
	 * game settings object
	 *
	 * @param OutFields	out value that receives the list of exposed properties
	 */
	virtual void GetSupportedDataFields(TArray<FUIDataProviderField>& OutFields);

	/* === UIListElementProvider === */

	/**
	 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
	 *
	 * @return	the list of tags supported by this element provider which correspond to list element data.
	 */
	virtual TArray<FName> GetElementProviderTags();

	/**
	 * Returns the number of list elements associated with the data tag specified.
	 *
	 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *						from GetElementProviderTags.
	 *
	 * @return	the total number of elements that are required to fully represent the data specified.
	 */
	virtual INT GetElementCount( FName FieldName );

	/**
	 * Retrieves the list elements associated with the data tag specified.
	 *
	 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *							from GetElementProviderTags.
	 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
	 *
	 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
	 */
	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the available cells for this list element.
	 * Used by the UI editor to know which cells are available for binding to individual list cells.
	 *
	 * @param	FieldName		the tag of the list element data field that we want the schema for.
	 *
	 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
	 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
	 * of the list element indicated by CellValueProvider.DataSourceIndex
	 *
	 * @param	FieldName		the tag of the list element data field that we want the values for
	 * @param	ListIndex		the list index for the element to get values for
	 *
	 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellValueProvider( FName FieldName, INT ListIndex );

   	virtual TScriptInterface<class IUIListElementProvider> ResolveListElementProvider( const FString& PropertyName );
}

/** 
 * @param FieldName		Name of the field to return the provider for.
 *
 * @return Returns a stats element provider given its field name. 
 */
event UTUIDataProvider_StatsElementProvider GetElementProviderFromName(name FieldName)
{
	if(FieldName=='GeneralStats')
	{
		return GeneralProvider;
	}
	else if(FieldName=='WeaponStats')
	{
		return WeaponsProvider;
	}
	else if(FieldName=='VehicleStats')
	{
		return VehiclesProvider;
	}
	else if(FieldName=='VehicleWeaponStats')
	{
		return VehicleWeaponsProvider;
	}
	else if(FieldName=='RewardStats')
	{
		return RewardsProvider;
	}

	return None;
}

/**
 * Game specific function that figures out what type of search to do
 */
function SetStatsReadInfo()
{
	local int ObjectIndex;
	local int PlayerFilterType;

	// Figure out which set of leaderboards to use by gamemode
	LeaderboardSettings.GetStringSettingValue(LF_GameMode,GameModeId);
	LeaderboardSettings.GetStringSettingValue(LF_MatchType,MatchTypeId);

	ObjectIndex = 0;
	switch(MatchTypeId)
	{
	case MTS_Player:
		ObjectIndex += 0;
		break;
	case MTS_Ranked:
		ObjectIndex += 1;
		break;
	}

	//NumTables (4) * Mode (Ranked/NonRanked)
	StatsReadObjectStartIndex = (4 * ObjectIndex) % StatsReadObjects.length;

	// Choose the read object based upon which filter they selected
	StatsRead = StatsReadObjects[StatsReadObjectStartIndex];
	
	// Read the set of players they want to view
	LeaderboardSettings.GetStringSettingValue(LF_PlayerFilterType,PlayerFilterType);
	switch (PlayerFilterType)
	{
		case PFS_Player:
			CurrentReadType = SFT_Player;
			break;
		case PFS_CenteredOnPlayer:
			CurrentReadType = SFT_CenteredOnPlayer;
			break;
		case PFS_Friends:
			CurrentReadType = SFT_Friends;
			break;
		case PFS_TopRankings:
			CurrentReadType = SFT_TopRankings;
			break;
	}
}

/**
 * @param InStatsType	Type to use to determine which read object to return.
 * 
 * @return Returns a read object depending on the current match settings.
 */
function OnlineStatsRead GetReadObjectFromType(EStatsDetailsReadType InStatsType)
{
	local OnlineStatsRead Result;
	local bool bIsCached;

	bIsCached=false;
	Result = None;

	// Make sure that there isnt already a stats read object cached.
	switch(InStatsType)
	{
	case UTSR_Weapons:
		bIsCached = (WeaponsProvider.ReadObject!=None);
		break;
	case UTSR_Vehicles:
		bIsCached = (VehiclesProvider.ReadObject!=None);
		break;
	case UTSR_VehicleWeapons:
		bIsCached = (VehicleWeaponsProvider.ReadObject!=None);
		break;
	default:
		bIsCached=true;
		break;
	}

	if(bIsCached==false)
	{
		Result = StatsReadObjects[StatsReadObjectStartIndex + InStatsType];
	}

	return Result;
}

/** Sets the index for the current player, so we know which stats row to use for results. */
function SetDetailedStatsRowIndex(int InIndex)
{
	DetailedStatsRowIndex = InIndex;

	// Store the ID of the player that we are getting row info for
	DetailsPlayerNetId = StatsRead.Rows[InIndex].PlayerId;
	DetailsPlayerNick = StatsRead.Rows[InIndex].NickName;

	// Invalidate all existing detailed cached reads
	WeaponsProvider.ReadObject=None;
	VehiclesProvider.ReadObject=None;
	VehicleWeaponsProvider.ReadObject=None;

	// Clear the read queue
	ReadQueue.length = 0;
}

/**
 * Adds a read object to the queue of stuff to read.
 *
 * @param ReadObj	Object to add to queue.
 */
function AddToReadQueue(OnlineStatsRead ReadObj)
{
	ReadQueue.length = ReadQueue.length+1;
	ReadQueue[ReadQueue.length-1] = ReadObj;

	TryPumpingQueue();
}

/** Attempts to start a stats read using the queue. */
function TryPumpingQueue()
{
	local array<UniqueNetId> Players;

	if(bInRead==false && ReadQueue.length > 0)
	{
		bInRead = true;

		// Pop first element off the read queue
		StatsRead=ReadQueue[0];
		ReadQueue.Remove(0, 1);

		// Try to start a stats read
		Players[0] = DetailsPlayerNetId;
		if (StatsInterface.ReadOnlineStats(Players, StatsRead) == false)
		{
			`warn("TryPumpingQueue::Read failed"@StatsRead);
			bInRead=false;
		}
	}
}

/** @return Returns the current read object index. */
function int GetReadObjectIndex()
{
	return StatsReadObjects.Find(StatsRead);
}

/**
 * Tells the online subsystem to re-read the stats data using the current read
 * mode and the current object to add the results to
 */
event bool RefreshStats(byte ControllerIndex)
{
	bInRead=true;
	return Super.RefreshStats(ControllerIndex);
}

/**
 * Called by the online subsystem when the stats read has completed
 *
 * @param bWasSuccessful whether the stats read was successful or not
 */
function OnReadComplete(bool bWasSuccessful)
{
	local EStatsDetailsReadType ReadType;

	// Clear in the InRead flag.
	bInRead=false;
	
	// Cache the results of the read in the appropriate provider(s) depending on what we just read.
	if(bWasSuccessful)
	{
		if (StatsRead != None)
		{
		ReadType = EStatsDetailsReadType(GetReadObjectIndex()%UTSR_MAX);
		switch(ReadType)
		{
		case UTSR_GeneralAndRewards:
			GeneralProvider.ReadObject=StatsRead;
			RewardsProvider.ReadObject=StatsRead;
			break;
		case UTSR_Weapons:
			WeaponsProvider.ReadObject=StatsRead;
			break;
		case UTSR_Vehicles:
			VehiclesProvider.ReadObject=StatsRead;
			break;
		case UTSR_VehicleWeapons:
			VehicleWeaponsProvider.ReadObject=StatsRead;
			break;
		}
		}

		// Try starting another stats read
		TryPumpingQueue();
	}
	else
	{
		`log("OnlineStats::OnReadComplete was not successful");
	}

	Super.OnReadComplete(bWasSuccessful);
}

/** Resets the stats read object to use the general provider so that the data exposed to the UI is the full list of stats. */
function ResetToDefaultRead()
{
	if(GeneralProvider.ReadObject != None)
	{
		StatsRead=GeneralProvider.ReadObject;
	}
}

defaultproperties
{
	LeaderboardSettingsClass=class'UTGame.UTLeaderboardSettings'

    //NumTablesPerMode*EMatchTypeSettings*NumGameModes

	//
	// NOTE THAT THE SIZE OF EACH COLLECTION MUST MATCH THE STATSREADTYPE ENUM!
	//

	//These are in the order defined in CONTEXT_GAMEMODES 

	//One 'collection' that makes up all stats for NonRanked
	StatsReadClasses(0)=class'UTGame.UTLeaderboardReadDM'
	StatsReadClasses(1)=class'UTGame.UTLeaderboardReadWeaponsDM'
	StatsReadClasses(2)=class'UTGame.UTLeaderboardReadVehiclesDM'
	StatsReadClasses(3)=class'UTGame.UTLeaderboardReadVehicleWeaponsDM'

	//One 'collection' that makes up all stats for Ranked
	StatsReadClasses(4)=class'UTGame.UTLeaderboardReadPureDM'
	StatsReadClasses(5)=class'UTGame.UTLeaderboardReadWeaponsPureDM'
	StatsReadClasses(6)=class'UTGame.UTLeaderboardReadVehiclesPureDM'
	StatsReadClasses(7)=class'UTGame.UTLeaderboardReadVehicleWeaponsPureDM'

	Tag=UTLeaderboards
}
