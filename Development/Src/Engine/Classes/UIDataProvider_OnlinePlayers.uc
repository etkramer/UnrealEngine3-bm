/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This class is responsible for retrieving the players list from the online
 * subsystem and populating the UI with that data.
 */
class UIDataProvider_OnlinePlayers extends UIDataProvider_OnlinePlayerDataBase
	native(inherit)
	implements(UIListElementCellProvider)
	dependson(OnlineSubsystem)
	transient;

/** Gets a copy of the players data from the online subsystem */
//var array<OnlinePlayer> PlayersList;

cpptext
{
/* === IUIListElement interface === */

	/**
	 * Returns the names of the exposed members in OnlinePlayer
	 *
	 * @see OnlinePlayer structure in OnlineSubsystem
	 */
	virtual void GetElementCellTags( FName FieldName, TMap<FName,FString>& out_CellTags )
	{
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
	virtual UBOOL GetCellFieldType( FName FieldName, const FName& CellTag, BYTE& out_CellFieldType )
	{
		return FALSE;
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
	virtual UBOOL GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE )
	{
		return FALSE;
	}

/* === UIDataProvider interface === */

	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
	{
		new(out_Fields) FUIDataProviderField( FName(TEXT("Players")), DATATYPE_Collection );
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
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE )
	{
		return FALSE;
	}
}

/**
 * Binds the player to this provider. Starts the async friends list gathering
 *
 * @param InPlayer the player that we are retrieving friends for
 */
event OnRegister(LocalPlayer InPlayer)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInterface;

	Super.OnRegister(InPlayer);
	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	// If the player is None, we are in the editor
	if (Player != None)
	{
		if (OnlineSub != None)
		{
			// Grab the player interface to verify the subsystem supports it
			PlayerInterface = OnlineSub.PlayerInterface;
			if (PlayerInterface != None)
			{
				// Set our callback function per player
//				PlayerInterface.SetReadPlayersComplete(Player.ControllerId,OnPlayersReadComplete);
				// Start the async task
/*				if (PlayerInterface.ReadPlayersList(Player.ControllerId) == false)
				{
					`Warn("Can't retrieve recent players list for player ("$Player.ControllerId$")");
				}
*/
			}
			else
			{
				`Warn("OnlineSubsystem does not support the player interface. Can't retrieve recent players list for player ("$
					Player.ControllerId$")");
			}
		}
		else
		{
			`Warn("No OnlineSubsystem present. Can't retrieve recent players list for player ("$
				Player.ControllerId$")");
		}
	}
}

/**
 * Handles the notification that the async read of the players data is done
 */
function OnPlayersReadComplete()
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInterface;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		// Grab the player interface to verify the subsystem supports it
		PlayerInterface = OnlineSub.PlayerInterface;
		if (PlayerInterface != None)
		{
			// Make a copy of the players data for the UI
//			PlayerInterface.GetPlayersList(Player.ControllerId,PlayersList);
		}
	}
}
