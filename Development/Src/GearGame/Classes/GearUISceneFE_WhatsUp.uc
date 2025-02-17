/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class GearUISceneFE_WhatsUp extends GearUISceneFrontEnd_Base
	Config(UI);


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The list of friends in the screen */
var transient UIList FriendList;

/** The top button bar */
var transient UICalloutButtonPanel ButtonBarTop;
/** The bottom button bar */
var transient UICalloutButtonPanel ButtonBarBottom;

/** Friends online label */
var transient UILabel FriendsLabel;

/** Friends online playing gears label */
var transient UILabel GearFriendsLabel;

/** Whether we are able to perform invites */
var transient bool bCanInvite;

/** Whether we can do the mass invite or not */
var transient bool bCanMassInvite;

/** Whether the player can play online or not */
var transient bool bCanPlayOnline;

/** Used to determine if we can send invites */
var transient int NumFriends;
var transient int NumOnlineFriends;
var transient int NumGearFriends;

/** Localized text for the FriendsLabel */
var localized string FriendsString;

/** Localized text for the GearFriendsLabel */
var localized string GearFriendsString;

/** Timer used to refresh the friends list */
var transient float RefreshTimer;


/************************************************************************/
/* Script Functions                                                     */
/************************************************************************/

/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	Super.PostInitialize();

	if ( !IsEditor() )
	{
		InitializeWidgetReferences();
	}
}

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	FriendList = UIList(FindChild('listWhatsUpFriends', true));
	ButtonBarTop = UICalloutButtonPanel(FindChild('btnbarwhatsup', true));
	ButtonBarBottom = UICalloutButtonPanel(FindChild('btnbarMain', true));
	FriendsLabel = UILabel(FindChild('lblFriendsOnline', true));
	GearFriendsLabel = UILabel(FindChild('lblGearsFriendsOnline', true));

	FriendList.OnValueChanged = OnPlayerListSelectionChanged;
	FriendList.OnOverrideListElementState = OverrideElementState;

	ButtonBarTop.SetButtonCallback('Invite', OnInvitePress);
	ButtonBarTop.SetButtonCallback('JoinGame', OnJoinGamePress);
	ButtonBarTop.SetButtonCallback('MassInvite', OnMassInvitePress);
	ButtonBarBottom.SetButtonCallback('ShowGamercard', OnPlayerCardPress);
	ButtonBarBottom.SetButtonCallback('GenericBack', OnBackPress);
}

/** Refreshes the buttonbar so that the "Join" enables/disables properly */
final function RefreshButtonBars(int PlayerIndex)
{
	ButtonBarBottom.EnableButton('ShowGamercard', PlayerIndex, NumFriends > 0, false);

	ButtonBarTop.EnableButton('JoinGame', PlayerIndex, CanJoinGame(), false);

	// Determine if we can perform invites
	ButtonBarTop.EnableButton('Invite', PlayerIndex, CanSendInvite(), false);
	ButtonBarTop.EnableButton('MassInvite', PlayerIndex, CanSendMassInvite(), false);

}

/** Determines if there are any slots available for invites */
function bool IsSessionFull()
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameSettings GameSettings;
	local WorldInfo WI;
	local int HumanCount;
	local int PriIndex;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None && OnlineSub.GameInterface != None)
	{
		// Only the party session allows invites
		GameSettings = OnlineSub.GameInterface.GetGameSettings('Party');
		WI = GetWorldInfo();
		if (WI != None)
		{
			// Count the number of humans in the session
			for (PriIndex = 0; PriIndex < WI.GRI.PRIArray.Length; PriIndex++)
			{
				if (!WI.GRI.PRIArray[PriIndex].bBot)
				{
					HumanCount++;
				}
			}
			// Compare the number of slots to the number of players
			return (GameSettings.NumPublicConnections + GameSettings.NumPrivateConnections) <= HumanCount;
		}
	}
	return true;
}

/** Whether an invite can be sent or not */
function bool CanSendInvite()
{
	local OnlineFriend Friend;
	local int FriendIndex;

	// Get the friend at the currently selected index index
	FriendIndex = FriendList.GetCurrentItem();
	Friend = GetFriendDataFromList(FriendList, FriendIndex);

	return bCanInvite && NumFriends > 0 && Friend.bIsOnline && !Friend.bHaveInvited && !IsSessionFull() && !IsInSession(Friend.UniqueId);
}

/** Whether mass invites can be sent or not */
function bool CanSendMassInvite()
{
	return bCanMassInvite && NumOnlineFriends > 0 && !IsSessionFull();
}

/** Determines whether the selected friend is joinable or not */
function bool CanJoinGame()
{
	local OnlineFriend Friend;
	local int FriendIndex;

	// Get the friend at the currently selected index index
	FriendIndex = FriendList.GetCurrentItem();
	Friend = GetFriendDataFromList(FriendList, FriendIndex);

	return bCanPlayOnline && NumFriends > 0 && (Friend.bIsJoinable || Friend.bHasInvitedYou) && !IsInSession(Friend.UniqueId);
}

/**
 * Determines whether a player is in the same session or not
 *
 * @param NetId the id of the player to search for
 *
 * @return true if they are in the session, false otherwise
 */
function bool IsInSession(UniqueNetId NetId)
{
	local WorldInfo WI;
	local int PriIndex;

	WI = GetWorldInfo();
	if (WI != None)
	{
		// Check each controller to see if they are the player in question
		for (PriIndex = 0; PriIndex < WI.GRI.PRIArray.Length; PriIndex++)
		{
			if (WI.GRI.PRIArray[PriIndex].UniqueId == NetId)
			{
				return true;
			}
		}
	}
	return false;
}

/** Refreshes the number of friends strings */
final function RefreshNumFriendsStrings()
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local OnlineFriend Friend;
	local int FriendIndex;
	local string StringToDraw;

	// Clear these so they don't grow over time
	NumFriends = 0;
	NumOnlineFriends = 0;
	NumGearFriends = 0;

	if (FriendList != None)
	{
		PlayerData = FriendList.DataProvider;
		if (PlayerData != None &&
			PlayerData.FriendsProvider != None)
		{
			for (FriendIndex = 0; FriendIndex < PlayerData.FriendsProvider.FriendsList.length; FriendIndex++)
			{
				Friend = PlayerData.FriendsProvider.FriendsList[FriendIndex];
				NumFriends++;
				if (Friend.bIsOnline)
				{
					NumOnlineFriends++;
					if (Friend.bIsPlayingThisGame)
					{
						NumGearFriends++;
					}
				}
			}
		}
	}

	StringToDraw = NumOnlineFriends @ FriendsString;
	FriendsLabel.SetDataStoreBinding(StringToDraw);
	StringToDraw = NumGearFriends @ GearFriendsString;
	GearFriendsLabel.SetDataStoreBinding(StringToDraw);
}

/**
 * Called when the value of the playerlist is changed.
 *
 * @param	Sender			the UIObject whose value changed
 * @param	PlayerIndex		the index of the player that generated the call to this method; used as the PlayerIndex when activating
 *							UIEvents; if not specified, the value of GetBestPlayerIndex() is used instead.
 */
function OnPlayerListSelectionChanged(UIObject Sender, int PlayerIndex)
{
	RefreshButtonBars(PlayerIndex);
}

/**
* Called when the widget is no longer being pressed.  Not implemented by all widget types.
*
* The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
* widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
* when the key was released, which might not necessarily be the widget that received the key press.
*
* @param EventObject	Object that issued the event.
* @param PlayerIndex	Player that performed the action that issued the event.
*
* @return	return TRUE to prevent the kismet OnClick event from firing.
*/
function bool OnInvitePress(UIScreenObject EventObject, int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local OnlineFriend Friend;
	local UniqueNetId ZeroId;
	local int ControllerId;
	local int FriendIndex;
	local UIDataStore_OnlinePlayerData PlayerData;
	local string AlertText;

	if (bCanInvite)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if ( PlayerInt != None )
			{
				FriendIndex = FriendList.GetCurrentItem();
				Friend = GetFriendDataFromList(FriendList, FriendIndex);
				if (Friend.UniqueId != ZeroId && !Friend.bHaveInvited)
				{
					ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
					PlayerInt.SendGameInviteToFriend(ControllerId, Friend.UniqueId);

					PlayerData = FriendList.DataProvider;
					// Mark the player as invited
					PlayerData.FriendsProvider.FriendsList[FriendIndex].bHaveInvited = true;
					// Refresh the UI list with the updated info
					PlayerData.FriendsProvider.NotifyPropertyChanged();

					// Pop up an alert regarding this invite
					`log("WHAT'S UP?! Inviting"@Friend.NickName);
					AlertText = Localize("AlertStrings", "WhatsUpSent", "GearGameUI");
					AlertText = Repl(AlertText, "\`NAME\`", Friend.NickName);
					GetGearPlayerOwner().AlertManager.Alert(eALERT_FriendAlert, Localize("AlertStrings", "InviteSentTitle", "GearGameUI"), AlertText);
				}
			}
		}
	}
	RefreshButtonBars(PlayerIndex);
	return true;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnMassInvitePress(UIScreenObject EventObject, int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local OnlineFriend Friend;
	local UniqueNetId ZeroId;
	local int ControllerId;
	local array<UniqueNetId> NetIdList;
	local int FriendIdx;
	local UIDataStore_OnlinePlayerData PlayerData;
	local string AlertText;

	// Make sure they can actually invite
	if (bCanMassInvite)
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if ( PlayerInt != None )
			{
				// Grab the player datastore
				PlayerData = FriendList.DataProvider;
				if (PlayerData != None &&
					PlayerData.FriendsProvider != None)
				{
					// Loop through the friend list and find all the net ids that are valid
					for (FriendIdx = 0; FriendIdx < PlayerData.FriendsProvider.FriendsList.length; FriendIdx++)
					{
						Friend = PlayerData.FriendsProvider.FriendsList[FriendIdx];
						if (Friend.UniqueId != ZeroId && !Friend.bHaveInvited)
						{
							NetIdList.AddItem(Friend.UniqueId);

							// Mark the player as invited
							PlayerData.FriendsProvider.FriendsList[FriendIdx].bHaveInvited = true;
						}
					}
					// If there are any friends in the list send the bulk invite
					if (NetIdList.length > 0)
					{
						ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
						PlayerInt.SendGameInviteToFriends(ControllerId, NetIdList);
						// Only allow one mass invite per instance of this menu being opened
						bCanMassInvite = false;
						// Refresh the button bars since we altered the state of the mass invite
						RefreshButtonBars(PlayerIndex);

						// Pop up an alert regarding the invite.
						`log("WHAT'S UP?! Inviting ALL my peeps!");
						AlertText = Localize("AlertStrings", "WhatsUpSendAll", "GearGameUI");
						AlertText = Repl(AlertText, "\`NUMFRIENDS\`", string(NetIdList.length));
						GetGearPlayerOwner().AlertManager.Alert(eALERT_FriendAlert, Localize("AlertStrings", "InviteSentTitle", "GearGameUI"), AlertText);
					}
				}
				// Refresh the UI list with the updated info
				PlayerData.FriendsProvider.NotifyPropertyChanged();
			}
		}
	}
	RefreshButtonBars(PlayerIndex);
	return true;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnPlayerCardPress(UIScreenObject EventObject, int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;
	local OnlineFriend Friend;
	local UniqueNetId ZeroId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if ( OnlineSub != None )
	{
		PlayerIntEx = OnlineSub.PlayerInterfaceEx;
		if ( PlayerIntEx != None )
		{
			Friend = GetFriendDataFromList(FriendList, FriendList.GetCurrentItem());
			if (Friend.UniqueId != ZeroId)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				PlayerIntEx.ShowGamerCardUI(ControllerId,  Friend.UniqueId);
			}
		}
	}
	return true;
}

/**
* Called when the widget is no longer being pressed.  Not implemented by all widget types.
*
* The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
* widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
* when the key was released, which might not necessarily be the widget that received the key press.
*
* @param EventObject	Object that issued the event.
* @param PlayerIndex	Player that performed the action that issued the event.
*
* @return	return TRUE to prevent the kismet OnClick event from firing.
*/
function bool OnBackPress(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnJoinGamePress(UIScreenObject EventObject, int PlayerIndex)
{
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	ButtonAliases.AddItem('GenericCancel');
	ButtonAliases.AddItem('GenericContinue');
	GameSceneClient = GetSceneClient();
	GameSceneClient.ShowUIMessage('ConfirmJoin',
		"<Strings:GearGameUI.MessageBoxStrings.JoinFriendGame_Title>",
		"<Strings:GearGameUI.MessageBoxStrings.JoinFriendGame_Message>",
		"",
		ButtonAliases, OnJoinConfirmed, GetPlayerOwner(PlayerIndex));

	return true;
}

/** Starts the join process after confirmation */
function bool OnJoinConfirmed( UIMessageBoxBase Sender, name SelectedInputAlias, int PlayerIndex )
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local OnlineFriend Friend;
	local UniqueNetId ZeroId;
	local int ControllerId;

	if (SelectedInputAlias == 'GenericContinue')
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			PlayerInt = OnlineSub.PlayerInterface;
			if (PlayerInt != None)
			{
				Friend = GetFriendDataFromList(FriendList, FriendList.GetCurrentItem());
				if (Friend.UniqueId != ZeroId &&
					(Friend.bIsJoinable || Friend.bHasInvitedYou) &&
					Friend.bIsPlayingThisGame)
				{
					ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
					`log("WHAT'S UP?! Joining"@Friend.NickName);
					// Join the game and close this scene since this will trigger like an invite
					if (PlayerInt.JoinFriendGame(ControllerId, Friend.UniqueId))
					{
						GetGearPlayerOwner(TEMP_SPLITSCREEN_INDEX).ClientShowLoadingMovie(true);
					}
					CloseScene();
				}
			}
		}
	}
	return true;
}

/** Get the friend data in the list */
final function OnlineFriend GetFriendDataFromList(UIList lstFriends, int ListIdx)
{
	local OnlineFriend Friend;
	local UIDataStore_OnlinePlayerData PlayerData;

	if (lstFriends != None && ListIdx >= 0)
	{
		PlayerData = lstFriends.DataProvider;
		if (PlayerData != None &&
			PlayerData.FriendsProvider != None &&
			PlayerData.FriendsProvider.FriendsList.length > ListIdx)
		{
			Friend = PlayerData.FriendsProvider.FriendsList[ListIdx];
		}
	}
	return Friend;
}

/**
 * Handler for this scene's GetSceneInputModeOverride delegate.  Forces INPUTMODE_Locked if in the front-end.
 */
function EScreenInputMode OverrideSceneInputMode()
{
	local EScreenInputMode Result;

	Result = GetSceneInputMode(true);
	if ( class'WorldInfo'.static.IsMenuLevel() )
	{
		Result = INPUTMODE_Locked;
	}

	return Result;
}

/** Called when the scene is activated so we can set the difficulty strings */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local OnlineSubsystem OnlineSub;
	local OnlineGameInterface GameInt;
	local OnlineGameSettings GameSettings;
	local int ControllerId;
	local GameUISceneClient GameSceneClient;
	local array<name> ButtonAliases;

	// Refresh the strings that say how many friends we have in the list
	RefreshNumFriendsStrings();

	ControllerId = GetBestControllerId();

	if ( bInitialActivation )
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None &&
			OnlineSub.PlayerInterface != None)
		{
			// Make sure they are logged into an online profile
			if (OnlineSub.PlayerInterface.GetLoginStatus(ControllerId) == LS_LoggedIn)
			{
				// Make sure that profile isn't a guest
				if (!OnlineSub.PlayerInterface.IsGuestLogin(ControllerId))
				{
					// Determine if we are able to invite other players
					GameInt = OnlineSub.GameInterface;
					if (GameInt != None)
					{
						GameSettings = GameInt.GetGameSettings('Party');
						if (GameSettings != None &&
							!GameSettings.bIsLanMatch &&
							GameSettings.bAllowInvites)
						{
							bCanInvite = true;
							bCanMassInvite = true;
						}
					}

					bCanPlayOnline = OnlineSub.PlayerInterface.CanPlayOnline(ControllerId) == FPL_Enabled;

					// Refresh the friends list
					RefreshFriendsList();
				}
				else
				{
					ButtonAliases.AddItem('GenericAccept');
					GameSceneClient = class'UIInteraction'.static.GetSceneClient();
					GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
						"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTier_Title>",
						"<Strings:GearGameUI.MessageBoxErrorStrings.NeedNonGuestProfileForFriendsList_Message>",
						"",
						ButtonAliases, CloseOnAccept, GetPlayerOwner());
				}
			}
			else
			{
				ButtonAliases.AddItem('GenericAccept');
				GameSceneClient = class'UIInteraction'.static.GetSceneClient();
				GameSceneClient.ShowUIMessage('ConfirmNetworkLost',
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedGoldTier_Title>",
					"<Strings:GearGameUI.MessageBoxErrorStrings.NeedProfileForFriendsList_Message>",
					"",
					ButtonAliases, CloseOnAccept, GetPlayerOwner());
			}
		}
	}

	// Refresh the button bars
	RefreshButtonBars(GetBestPlayerIndex());
}

/** Handler to close the scene on click */
function bool CloseOnAccept(UIMessageBoxBase Sender,name SelectedInputAlias,int PlayerIndex)
{
	CloseScene();
	return true;
}

/**
 * Delegate used when the friends read request has completed
 *
 * @param bWasSuccessful true if the async action completed without error, false if there was an error
 */
function OnRefreshFriendsComplete(bool bWasSuccessful)
{
	local OnlineSubsystem OnlineSub;
	local OnlinePlayerInterface PlayerInt;
	local int PlayerIndex;
	local int ControllerId;

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		PlayerInt = OnlineSub.PlayerInterface;
		if (PlayerInt != None)
		{
			PlayerIndex = GetBestPlayerIndex();
			ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
			PlayerInt.ClearReadFriendsCompleteDelegate(ControllerId, OnRefreshFriendsComplete);
		}
	}

	// Refresh the strings that say how many friends we have in the list
	RefreshNumFriendsStrings();

	// Refresh the button bars
	RefreshButtonBars(GetBestPlayerIndex());

	// Stop the updating scene
	class'GearUIScene_Base'.static.CloseUpdatingScene();
}


/**
 * Handler for the friends list's OnOverrideListElementState delegate.  Changes the element state based on certain conditions such as whether
 * the player is playing Gears2, if they're online, etc.
 *
 * @param	Sender			the list calling the delegate
 * @param	ElementIndex	the index [into the list's Items array] for the element being set; Items[ElementIndex] would be the index into
 *							the source data provider's collection.
 * @param	CurrentState	the element's current cell state.
 * @param	NewElementState	the cell state that is being set on the element.
 *
 * @return	the cell state that should be set on the element.
 */
function EUIListElementState OverrideElementState( UIList Sender, int ElementIndex, EUIListElementState CurrentState, EUIListElementState NewElementState )
{
	local OnlineFriend FriendItem;

	FriendItem = GetFriendDataFromList(Sender, Sender.Items[ElementIndex]);
	if ( Sender.IsElementSelected(ElementIndex) )
	{
		NewElementState = FriendItem.bIsPlayingThisGame ? ELEMENT_Selected : ELEMENT_UnderCursor;
	}
	else
	{
		NewElementState = FriendItem.bIsPlayingThisGame ? ELEMENT_Normal : ELEMENT_Active;
	}

	return NewElementState;
}

/**
 * Updates the friends list when requested
 */
function RefreshFriendsList()
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local OnlineSubsystem OnlineSub;
	local int ControllerId;
	local GearUISceneFE_Updating SceneInstance;

	RefreshTimer = 30.0;

	ControllerId = GetBestControllerId();

	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None &&
		OnlineSub.PlayerInterface != None)
	{
		// Make sure they are logged into an online profile
		if (OnlineSub.PlayerInterface.GetLoginStatus(ControllerId) == LS_LoggedIn)
		{
			// Make sure that profile isn't a guest
			if (!OnlineSub.PlayerInterface.IsGuestLogin(ControllerId))
			{
				PlayerData = FriendList.DataProvider;
				if (PlayerData != None &&
					PlayerData.FriendsProvider != None)
				{
					OnlineSub.PlayerInterface.AddReadFriendsCompleteDelegate(ControllerId, OnRefreshFriendsComplete);
					PlayerData.FriendsProvider.FriendsList.Length = 0;
					PlayerData.FriendsProvider.NotifyPropertyChanged();
					PlayerData.FriendsProvider.RefreshFriendsList();

					// While refreshing, block input with a refresh scene
					SceneInstance = class'GearUIScene_Base'.static.OpenUpdatingScene();
					if (SceneInstance != None)
					{
						SceneInstance.InitializeUpdatingScene("RefreshTitle", "FriendRefreshDesc", 0.5f);
					}
				}
			}
		}
	}
}

/**
 * Used to auto refresh the friends list
 */
function OnSceneTick(float DeltaTime)
{
	RefreshTimer -= DeltaTime;
	if (RefreshTimer < 0)
	{
		RefreshFriendsList();
	}
}

defaultproperties
{
	OnSceneActivated=OnSceneActivatedCallback
	bMenuLevelRestoresScene=true
	SceneRenderMode=SPLITRENDER_PlayerOwner
	SceneInputMode=INPUTMODE_MatchingOnly
	GetSceneInputModeOverride=OverrideSceneInputMode
	bRequiresOnlineService=true
	OnGearUISceneTick=OnSceneTick
}

