/**
 * This class is responsible for providing access to information about the achievements available to the player
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIDataProvider_PlayerAchievements extends UIDataProvider_OnlinePlayerDataBase
	native(inherit)
	implements(UIListElementCellProvider)
	dependson(OnlineSubsystem)
	transient;

/*
/** Holds the information contained in Live's achievements for this title */
struct native AchievementDetails
{
	/** The ID of the achievement */
	var const int Id;
	/** The name of the achievement */
	var const string AchievementName;
	/** The description of the achievement */
	var const string Description;
	/** The description of how to meet the requirements of the achievement */
	var const string HowTo;
	/** The image associated with the achievement */
	var Texture2D Image;
	/** How much this achievement is worth */
	var const int GamerPoints;
	/** Whether the achievement is secret (hidden if not achieved) or not */
	var const bool bIsSecret;
	/** Whether the achievement awarded online or not */
	var const bool bWasAchievedOnline;
	/** Whether the achievement awarded offline or not */
	var const bool bWasAchievedOffline;
};
*/
var	transient	array<AchievementDetails>					Achievements;

cpptext
{
	/* === IUIListElement interface === */
	/**
	 * Returns the names of the exposed members in OnlineFriend
	 *
	 * @see OnlineFriend structure in OnlineSubsystem
	 */
	virtual void GetElementCellTags(FName FieldName, TMap<FName,FString>& CellTags);

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
	virtual UBOOL GetCellFieldType(FName FieldName, const FName& CellTag,BYTE& CellFieldType);

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
	virtual void GetSupportedDataFields( TArray<FUIDataProviderField>& out_Fields );

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );
}

/**
 * Returns the number of gamer points this profile has accumulated across all achievements
 *
 * @return	a number between 0 and the maximum number of gamer points allocated for each game (currently 1000), representing the total
 * gamer points earned from all achievements for this profile.
 */
native final function int GetTotalGamerScore() const;

/**
 * Loads the achievement icons from the .ini and applies them to the list of achievements.
 */
function PopulateAchievementIcons();

/**
 * Wrapper for retrieving the path name of an achievement's icon.
 */
function string GetAchievementIconPathName( int AchievementId, optional bool bReturnLockedIcon );

/**
 * Gets achievement details based on achievement id
 *
 * @param AchievementId	EGearAchievement for which to find details
 * @param OutAchievementDetails	AchievementDetails struct to be populated
 *
 */
function GetAchievementDetails(const int AchievementId, out AchievementDetails OutAchievementDetails)
{
	local int idx;

	idx = Achievements.Find('Id', AchievementId);

	//`Log("Looking for Id: " $ string(AchievementId) $ " Found at " $ idx $ " of " $ string(Achievements.Length));
	if( idx != -1 )
	{
		OutAchievementDetails = Achievements[idx];
	}
}

/**
 * Called when the async achievements read has completed
 *
 * @param TitleId the title id that the read was for (0 means current title)
 */
function OnPlayerAchievementsChanged( int TitleId )
{
	local OnlineSubsystem OnlineSub;
	local EOnlineEnumerationReadState Result;

	if ( Player != None )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None && OnlineSub.PlayerInterfaceEx != None && TitleId == 0 )
		{
			Result = OnlineSub.PlayerInterfaceEx.GetAchievements(Player.ControllerId, Achievements, TitleId);
			if ( Result == OERS_Done )
			{
				PopulateAchievementIcons();

				NotifyPropertyChanged(nameof(Achievements));
				NotifyPropertyChanged('TotalGamerPoints');
			}
		}
	}
}

/**
 * Handler for online service's callback for the player unlocking an achievement.
 */
function OnPlayerAchievementUnlocked( bool bWasSuccessful )
{
	if ( bWasSuccessful )
	{
		UpdateAchievements();
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
	local OnlinePlayerInterfaceEx PlayerInterfaceEx;

	Super.OnRegister(InPlayer);

	// If the player is None, we are in the editor
	if (Player != None)
	{
		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			if ( OnlineSub.PlayerInterface != None )
			{
				// Register that we are interested in any sign in change for this player
				OnlineSub.PlayerInterface.AddLoginChangeDelegate(OnLoginChange,Player.ControllerId);
			}

			// Grab the player interface to verify the subsystem supports it
			PlayerInterfaceEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerInterfaceEx != None)
			{
				// Set our callback function per player
				PlayerInterfaceEx.AddReadAchievementsCompleteDelegate(Player.ControllerId, OnPlayerAchievementsChanged);
				PlayerInterfaceEx.AddUnlockAchievementCompleteDelegate(Player.ControllerId, OnPlayerAchievementUnlocked);

				// Start the async task
				if ( !PlayerInterfaceEx.ReadAchievements(Player.ControllerId) )
				{
					`warn("Can't retrieve achievements for player ("$Player.ControllerId$")");
				}
			}
			else
			{
				`warn("OnlineSubsystem does not support the player interface. Can't retrieve achievements for player ("$
					Player.ControllerId$")");
			}
		}
		else
		{
			`warn("No OnlineSubsystem present. Can't retrieve achievements for player ("$
				Player.ControllerId$")");
		}
	}
}

/**
 * Clears our delegate for getting login change notifications
 */
event OnUnregister()
{
	local OnlineSubsystem OnlineSub;

	// Figure out if we have an online subsystem registered
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		if ( OnlineSub.PlayerInterface != None )
		{
			// Clear our callback function per player
			OnlineSub.PlayerInterface.ClearLoginChangeDelegate(OnLoginChange,Player.ControllerId);
		}

		// Grab the player interface to verify the subsystem supports it
		if (OnlineSub.PlayerInterfaceEx != None)
		{
			OnlineSub.PlayerInterfaceEx.ClearUnlockAchievementCompleteDelegate(Player.ControllerId, OnPlayerAchievementUnlocked);
			OnlineSub.PlayerInterfaceEx.ClearReadAchievementsCompleteDelegate(Player.ControllerId, OnPlayerAchievementsChanged);
		}
	}

	ClearAchievements();
	Super.OnUnregister();
}

/**
 * Handlers for the player's login changed delegate.  Refreshes the list of achievements.
 */
function OnLoginChange()
{
	UpdateAchievements();
}

/**
 * Queries the online service for the player's list of achievements.
 */
function UpdateAchievements()
{
	local OnlineSubsystem OnlineSub;
	local bool bResult;

	if ( Player != None && class'UIInteraction'.static.IsLoggedIn(Player.ControllerId) )
	{
		// Figure out if we have an online subsystem registered
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None && OnlineSub.PlayerInterfaceEx != None )
		{
			// Start the async task
			bResult = OnlineSub.PlayerInterfaceEx.ReadAchievements(Player.ControllerId);
			if ( !bResult )
			{
				`warn("Can't retrieve achievements for player ("$Player.ControllerId$")");
			}
		}
	}

	if ( !bResult )
	{
		ClearAchievements();
	}
}

/**
 * Clears the list of achievements pulled from the online service.
 */
function ClearAchievements()
{
	local OnlineSubsystem OnlineSub;

	if ( Achievements.Length > 0 )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if ( OnlineSub != None && OnlineSub.PlayerInterfaceEx != None )
		{
			Achievements.Length = 0;
			NotifyPropertyChanged(nameof(Achievements));
		}
		else if ( Achievements != default.Achievements )
		{
			Achievements = default.Achievements;
			NotifyPropertyChanged(nameof(Achievements));
		}
	}
}

DefaultProperties
{
	//@fixme - temp filler data for development; remove once achievement data has been propped.
	Achievements(0)=(Id=10,AchievementName="DomCurious",Description="Dom-Curious",HowTo="Complete 10 different co-op chapters as Dominic Santiago on any difficult",GamerPoints=10,bIsSecret=false)
	Achievements(1)=(Id=20,AchievementName="Seriously",Description="Seriously??",HowTo="Kill a total of 10,000 opponenets in versus ranked matches",GamerPoints=10,bIsSecret=false)
	Achievements(2)=(Id=30,AchievementName="DownWithEPIC",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=20,bIsSecret=true)
	Achievements(3)=(Id=10,AchievementName="DomCurious1",Description="Dom-Curious",HowTo="Complete 10 different co-op chapters as Dominic Santiago on any difficult",GamerPoints=10,bIsSecret=false)
	Achievements(4)=(Id=20,AchievementName="Seriously1",Description="Seriously??",HowTo="Kill a total of 10,000 opponenets in versus ranked matches",GamerPoints=10,bIsSecret=false)
	Achievements(5)=(Id=30,AchievementName="DownWithEPIC1",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=20,bIsSecret=true)
	Achievements(6)=(Id=30,AchievementName="DownWithEPIC2",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=40,bIsSecret=true)
	Achievements(7)=(Id=20,AchievementName="Seriously",Description="Seriously??",HowTo="Kill a total of 10,000 opponenets in versus ranked matches",GamerPoints=10,bIsSecret=false)
	Achievements(8)=(Id=30,AchievementName="DownWithEPIC",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=20,bIsSecret=true)
	Achievements(9)=(Id=10,AchievementName="DomCurious1",Description="Dom-Curious",HowTo="Complete 10 different co-op chapters as Dominic Santiago on any difficult",GamerPoints=10,bIsSecret=false)
	Achievements(10)=(Id=20,AchievementName="Seriously1",Description="Seriously??",HowTo="Kill a total of 10,000 opponenets in versus ranked matches",GamerPoints=10,bIsSecret=false)
	Achievements(11)=(Id=30,AchievementName="DownWithEPIC1",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=20,bIsSecret=true)
	Achievements(12)=(Id=30,AchievementName="DownWithEPIC2",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=40,bIsSecret=true)
	Achievements(13)=(Id=20,AchievementName="Seriously1",Description="Seriously??",HowTo="Kill a total of 10,000 opponenets in versus ranked matches",GamerPoints=10,bIsSecret=false)
	Achievements(14)=(Id=30,AchievementName="DownWithEPIC1",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=20,bIsSecret=true)
	Achievements(15)=(Id=30,AchievementName="DownWithEPIC2",Description="You Down With E.P.I.C.?",HowTo="Win 20 multiplayer matches of 3+ rounds in any game type on the Subway multiplayer map",GamerPoints=40,bIsSecret=true)
}
