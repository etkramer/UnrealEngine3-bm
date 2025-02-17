/**
* Online player data store for Gears 2.
* This adds screenshot functionality to the base engine class.
*
* @todo
* - when player takes a new screenshot, fire a notification that this provider subscribes to so that we know when to re-enumerate screenshots
*
* Copyright 2008 Epic Games, Inc. All Rights Reserved
*/
class GearUIDataProvider_Screenshots extends UIDataProvider_OnlinePlayerDataBase
	native(UIPrivate)
	implements(UIListElementCellProvider)
	transient;

/** The list of screenshots on disk */
var array<SavedScreenshotInfo> Screenshots;

/** The column name to display in the UI */
var localized string RatingCol;

/** The column name to display in the UI */
var localized string GameTypeCol;

/** The column name to display in the UI */
var localized string MapNameCol;

/** The column name to display in the UI */
var localized string DateTimeCol;

cpptext
{
/* === IUIListElement interface === */

	/**
	 * Retrieves the list of tags that can be bound to individual cells in a single list element, along with the human-readable,
	 * localized string that should be used in the header for each cell tag (in lists which have column headers enabled).
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
	 */
	virtual void GetElementCellTags(FName FieldName, TMap<FName,FString>& CellTags)
	{
		CellTags.Set(FName(TEXT("Rating")),*RatingCol);
		CellTags.Set(FName(TEXT("GameType")),*GameTypeCol);
		CellTags.Set(FName(TEXT("MapName")),*MapNameCol);
		CellTags.Set(FName(TEXT("DateTime")),*DateTimeCol);
	}

	/**
	 * Retrieves the field type for the specified cell.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	CellTag				the tag for the element cell to get the field type for
	 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
	 *
	 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
	 */
	virtual UBOOL GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& CellFieldType)
	{
		CellFieldType = DATATYPE_Property;
		return TRUE;
	}

	/**
	 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
	 *
	 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
	 *							instance provides element cells for multiple collection data fields.
	 * @param	CellTag			the tag for the element cell to resolve the value for
	 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
	 *							do not provide unique UIListElement objects for each element.
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
	 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
	 *							to a data collection.
	 */
	virtual UBOOL GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

/* === UIDataProvider interface === */

	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
	{
		new(out_Fields) FUIDataProviderField( FName(TEXT("Screenshots")), DATATYPE_Collection );
	}

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );
}

/* == Delegates == */

/* == Natives == */

/* == Events == */
/** Re-reads the screenshots to freshen any cached data */
event RefreshScreenshots()
{
	EnumerateScreenshots();
}

/** Sorts the screenshots */
native function SortScreenshots();

/** Delegate called when a screenshot enumeration completes. */
function EnumScreenshotsComplete(bool bWasSuccessful)
{
	local GearPC PC;
	
	`log(`location @ `showvar(bWasSuccessful));

	SortScreenshots();

	// Notify any subscribers that we have new data
	NotifyPropertyChanged();

	// If the player is None, we are in the editor
	if (Player != None)
	{
		PC = GearPC(Player.Actor);
		if ( PC != None )
		{
			if ( PC.LoadScreenshotManager() )
			{
				PC.ScreenshotManager.ClearEnumerateScreenshotsCompleteDelegate(EnumScreenshotsComplete);
			}
		}
	}
}

/** Updates the Screenshots member with the list of screenshots on disk. */
function EnumerateScreenshots( optional delegate<GearScreenshotManager.OnEnumerateScreenshotsComplete> EnumerationCompleteDelegate )
{
	local GearPC PC;
	local bool bWasSuccessful;

	// If the player is None, we are in the editor
	if (Player != None)
	{
		PC = GearPC(Player.Actor);
		if ( PC != None )
		{
			if ( PC.LoadScreenshotManager() )
			{
				PC.ScreenshotManager.AddEnumerateScreenshotsCompleteDelegate(EnumScreenshotsComplete);
				if ( EnumerationCompleteDelegate != None )
				{
					PC.ScreenshotManager.AddEnumerateScreenshotsCompleteDelegate(EnumerationCompleteDelegate);
				}

				bWasSuccessful = PC.ScreenshotManager.EnumerateScreenshots(Screenshots);
				if ( !bWasSuccessful )
				{
					`warn("Can't enumerate screenshots - unknown error.");
					PC.ScreenshotManager.ClearEnumerateScreenshotsCompleteDelegate(EnumScreenshotsComplete);
					if ( EnumerationCompleteDelegate != None )
					{
						// simulate failure.
						EnumerationCompleteDelegate(false);
					}
				}

				return;
			}
			else
			{
				`warn("Failed to enumerate screenshots - no screenshot manager for this player");
			}
		}
		else
		{
			`warn("Screenshot provider failed to get PC");
		}
	}
	else
	{
		`warn("Screenshot provider failed to get Player");
	}

	if ( EnumerationCompleteDelegate != None )
	{
		// simulate failure.
		EnumerationCompleteDelegate(false);
	}
}

function bool DeleteScreenshot(int Index)
{
	local bool bResult;
	local GearPC PC;

	if((Index >= 0) && (Index < Screenshots.length))
	{
		// If the player is None, we are in the editor
		if (Player != None)
		{
			PC = GearPC(Player.Actor);
			if ( PC != None )
			{
				if ( PC.LoadScreenshotManager() )
				{
					bResult = true;
					PC.ScreenshotManager.DeleteScreenshot(Screenshots[Index].ID, Screenshots[Index].DeviceID);
					Screenshots.Remove(Index,1);
					NotifyPropertyChanged();
				}
			}
		}
	}

	return bResult;
}

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Handles the notification that the async read of the profile data is done
 *
 * @param bWasSuccessful whether the call succeeded or not
 */
function OnReadProfileComplete( byte LocalUserNum, bool bWasSuccessful )
{
	if ( bWasSuccessful && class'UIInteraction'.static.IsLoggedIn(Player.ControllerId) )
	{
		//RefreshScreenshots();
	}
}

/**
 * Executes a refetching of the profile data when the login for this player
 * changes
 */
function OnLoginChange()
{
	if ( Player != None && !class'UIInteraction'.static.IsLoggedIn(Player.ControllerId) )
	{
		`log(`location $ ":" @ Player @ "is not logged in - clearing screenshot cache.",,'DevDataStore');
		if ( Screenshots.Length > 0 )
		{
			Screenshots.Length = 0;

			NotifyPropertyChanged();
		}
	}
}

/* === UIDataProvider_OnlinePlayerDataBase interface === */
/**
 * Binds the player to this provider.
 *
 * @param InPlayer the player that we are binding to.
 */
event OnRegister(LocalPlayer InPlayer)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInterface;

	Super.OnRegister(InPlayer);

	if ( !IsEditor() && Player != None )
	{
		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			// Grab the player interface to verify the subsystem supports it
			PlayerInterface = OnlineSub.PlayerInterface;
			if (PlayerInterface != None)
			{
				// Register that we are interested in any sign in change for this player
				PlayerInterface.AddLoginChangeDelegate(OnLoginChange,Player.ControllerId);
				// Set our callback function per player
				PlayerInterface.AddReadProfileSettingsCompleteDelegate(Player.ControllerId,OnReadProfileComplete);
			}
		}
	}
}

/**
 * Unbinds the provider from the player.
 */
event OnUnregister()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInterface;
	local GearPC GearPlayer;

	Screenshots.length = 0;
	if ( Player != None )
	{
		GearPlayer = GearPC(Player.Actor);
		if ( GearPlayer != None && GearPlayer.ScreenshotManager != None )
		{
			//@fixme ronp - this should really be done in GearPC by adding a function to GearPC which cleans up its screenshot manager
			GearPlayer.CleanupScreenshotManager();
			GearPlayer.ScreenshotManager = None;
		}

		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			// Grab the player interface to verify the subsystem supports it
			PlayerInterface = OnlineSub.PlayerInterface;
			if (PlayerInterface != None)
			{
				// Clear our delegate
				PlayerInterface.ClearLoginChangeDelegate(OnLoginChange,Player.ControllerId);
				// Clear our callback function per player
				PlayerInterface.ClearReadProfileSettingsCompleteDelegate(Player.ControllerId,OnReadProfileComplete);
			}
		}
	}

	Super.OnUnRegister();
}

