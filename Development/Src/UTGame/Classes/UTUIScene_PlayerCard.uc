/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Player card scene for UT3, allows players to add friends, invite to games, etc.
 */
class UTUIScene_PlayerCard extends UTUIFrontEnd_BasicMenu;

// Possible options for the player card screen
enum PlayerCardOptions
{
	PCO_SendFriendRequest,
	PCO_SendMessage,
	PCO_InviteToGame,
	PCO_Follow,
	PCO_Kick,
	PCO_Ban
};

/** Reference to the string list datastore. */
var transient UTUIDataStore_StringList StringListDataStore;

/** Reference to a label that displays the name of the player that this card belongs to. */
var transient UILabel		PlayerNameLabel;

/** Reference to the label that displays the account name */
var transient UILabel AccountLabel;

/** Current array of options visible to the player. */
var transient array<PlayerCardOptions> CurrentOptions;

/** Net ID of the player we are showing the card for. */
var transient UniqueNetId PlayerNetId;

/** Name of the player we are showing the card for. */
var transient string PlayerName;

/** Reference to the online player interface. */
var transient OnlinePlayerInterface PlayerInt;

/** Reference to the send friend request screen. */
var transient string SendFriendRequestScene;

/** Reference to the send online message screen. */
var transient string SendOnlineMessageScene;

/** Whether or not we were able to join a friend's game in progress. */
var transient bool bWasAbleToJoin;

var string DescLinks[6];

event PostInitialize()
{
	Super.PostInitialize();

	// Get a reference to the string list datastore
	StringListDataStore = UTUIDataStore_StringList(StaticResolveDataStore('UTStringList'));

	// Get widget references
	PlayerNameLabel = UILabel(FindChild('lblPlayerName',true));

    AccountLabel = UILabel(FindChild('lblAccountName',true));

	// Get reference to the player interface
	PlayerInt = GetPlayerInterface();
}

/** Setup the scene's buttonbar. */
function SetupButtonBar()
{
	ButtonBar.Clear();

	if(bSupportBackButton)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Back>", OnButtonBar_Back);
	}

	if(CurrentOptions.length>0)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Select>", OnButtonBar_Select);
	}
}

/**
 * Gets the OnlineFriend struct for the player we're displaying this player card for.
 *
 * @param	OwnerPC			the player controller associated with the player to get friend info from (i.e. the local player)
 * @param	FriendNetId		the unique network id associated with the friend to get info for (i.e. the friend)
 * @param	out_FriendData	receives the online friend data
 *
 * @return	FALSE if this player isn't one of our friends or the OnlineFriend data couldn't be found.
 */
static function bool GetOnlineFriendData( UTPlayerController OwnerPC, const out OnlineSubsystem.UniqueNetId FriendNetId, out OnlineSubsystem.OnlineFriend out_FriendData )
{
	local bool bResult;
	local int FriendIdx;
	local UIDataProvider_OnlineFriends FriendsProvider;
	local OnlinePlayerInterface PlayerInterface;

	PlayerInterface = GetPlayerInterface();
	if (PlayerInterface != None && OwnerPC != None && OwnerPC.OnlinePlayerData != None
	&&	PlayerInterface.IsFriend(LocalPlayer(OwnerPC.Player).ControllerId, FriendNetId) )
	{
		FriendsProvider = OwnerPC.OnlinePlayerData.FriendsProvider;
		if ( FriendsProvider != None )
		{
			for ( FriendIdx=0; FriendIdx< FriendsProvider.FriendsList.length; FriendIdx++ )
			{
				if ( FriendsProvider.FriendsList[FriendIdx].UniqueId == FriendNetId )
				{
					out_FriendData = FriendsProvider.FriendsList[FriendIdx];
					bResult = true;
					break;
				}
			}
		}
	}

	return bResult;
}

/** Sets the current player for this player card. */
function SetPlayer(UniqueNetId InNetId, string InPlayerName, optional string InAccountName, optional bool bIncludeKickOption, optional bool bIncludeBanOption)
{
	PlayerName = InPlayerName;
	PlayerNetId = InNetId;

	PlayerNameLabel.SetDataStoreBinding(PlayerName);
	if (InAccountName != "")
	{
		AccountLabel.SetDataStoreBinding(InAccountName);
		AccountLabel.SetVisibility(true);
	}
	else
	{
		AccountLabel.SetVisibility(false);
	}

	GenerateOptions(bIncludeKickOption, bIncludeBanOption);
}

/** Generates the options for the player card menu based on the users's current relationship to the player in question. */
function GenerateOptions(bool bIncludeKickOption, bool bIncludeBanOption)
{
	local UniqueNetId LocalNetId;
	local UTPlayerController UTPC;
	local LocalPlayer LP;
	local int FriendIdx;
	local bool bIsFriend;

	CurrentOptions.length=0;

	UTPC = GetUTPlayerOwner();
	LP = LocalPlayer(UTPC.Player);
	if ( PlayerInt != None && UTPC!=None && LP != None )
	{
		if(PlayerInt.GetUniquePlayerId(LP.ControllerId, LocalNetId))
		{
			StringListDataStore.Empty('PlayerCardOptions', true);

			if(LocalNetId != PlayerNetId)
			{
				bIsFriend = PlayerInt.IsFriend(LP.ControllerId, PlayerNetId);
				if(bIsFriend==false)
				{
					CurrentOptions[CurrentOptions.length]=PCO_SendFriendRequest;
					StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","SendFriendRequest","UTGameUI"), true);
				}

				CurrentOptions[CurrentOptions.length]=PCO_SendMessage;
				StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","SendMessage","UTGameUI"), true);

				// Only allow the ability invite people if we aren't in a standalone game
				if(UTPC.WorldInfo.NetMode != NM_Standalone)
				{
					CurrentOptions[CurrentOptions.length]=PCO_InviteToGame;
					StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","InviteToGame","UTGameUI"), true);
				}

				// Only allow the ability to follow players if they are on our friends list.
				if(bIsFriend && GetWorldInfo().NetMode != NM_ListenServer)
				{
					for(FriendIdx=0; FriendIdx<UTPC.OnlinePlayerData.FriendsProvider.FriendsList.length; FriendIdx++)
					{
						if(UTPC.OnlinePlayerData.FriendsProvider.FriendsList[FriendIdx].UniqueId==PlayerNetId)
						{
							if(UTPC.OnlinePlayerData.FriendsProvider.FriendsList[FriendIdx].bIsJoinable)
							{
								CurrentOptions[CurrentOptions.length]=PCO_Follow;
								StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","Follow","UTGameUI"));
							}

							break;
						}
					}
				}
			}

		}
	}

	if (bIncludeKickOption)
	{
		CurrentOptions[CurrentOptions.length]=PCO_Kick;
		StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","Kick","UTGameUI"), true);
	}

	if (bIncludeBanOption)
	{
		CurrentOptions[CurrentOptions.length]=PCO_Ban;
		StringListDataStore.AddStr('PlayerCardOptions', Localize("PlayerCard","Ban","UTGameUI"), true);
	}

	MenuList.RefreshSubscriberValue();

	// Show/Hide description label depending on if we have any options.
	DescriptionLabel.SetVisibility(CurrentOptions.length>0);

	SetupButtonBar();
}

/**
 * Executes a action based on the currently selected menu item.
 */
function OnSelectItem(int PlayerIndex=0)
{
	local int SelectedItem;
	SelectedItem = MenuList.GetCurrentItem();

	// Resolve the current list index into a possible option.
	if(SelectedItem < CurrentOptions.length)
	{
		SelectedItem = CurrentOptions[SelectedItem];

		switch(SelectedItem)
		{
		case PCO_SendFriendRequest:
			OnSendFriendRequest();
			break;
		case PCO_SendMessage:
			OnSendMessage();
			break;
		case PCO_InviteToGame:
			OnInviteToGame();
			break;
		case PCO_Follow:
			OnFollow();
			break;

		case PCO_Kick:
			OnKick();
			break;

		case PCO_Ban:
			OnBan();
			break;
		}
	}
}

/** Sends a friend request to the owner of this player card. */
function OnSendFriendRequest()
{
	OpenSceneByName(SendFriendRequestScene, false, OnSendFriendRequest_Opened);
}

/** Callback for when the send message scene has actually been opened.  Sets the target player for the scene. */
function OnSendFriendRequest_Opened(UIScene OpenedScene, bool bInitialActivation)
{
	if ( bInitialActivation )
	{
		UTUIScene_SendFriendRequest(OpenedScene).SetTargetPlayer(PlayerNetId, PlayerName);
	}
}

/** Displays the send online message scene for the owner of this card. */
function OnSendMessage()
{
	OpenSceneByName(SendOnlineMessageScene, false, OnSendMessageScene_Opened);
}

/** Callback for when the send message scene has actually been opened.  Sets the target player for the scene. */
function OnSendMessageScene_Opened(UIScene OpenedScene, bool bInitialActivation)
{
	if ( bInitialActivation )
	{
		UTUIScene_SendOnlineMessage(OpenedScene).SetTargetPlayer(PlayerNetId, PlayerName);
	}
}

/** Invites the owner of this player card to the local player's game. */
function OnInviteToGame()
{
	local string FinalMsg;
	if(PlayerInt != None)
	{
		if(PlayerInt.SendGameInviteToFriend(GetPlayerOwner().ControllerId, PlayerNetId))
		{
			FinalMsg = Repl(Localize("MessageBox", "SentGameInvite_Message", "UTGameUI"), "\`PlayerName\`", PlayerName);
			DisplayMessageBox(FinalMsg,"<Strings:UTGameUI.MessageBox.SentGameInvite_Title>");
		}
		else
		{
			FinalMsg = Repl(Localize("Errors", "SendInviteFailed_Message", "UTGameUI"), "\`PlayerName\`", PlayerName);
			DisplayMessageBox(FinalMsg,"<Strings:UTGameUI.Errors.SendInviteFailed_Title>");
		}
	}
}

/**
 * Determines whether we need to supply a password in order to join this friend's match.
 *
 * @return	TRUE if the friend's server is password-protected; FALSE otherwise.
 */
static function bool FollowRequiresPassword( UTPlayerController UTPlayerOwner, const out OnlineSubsystem.UniqueNetId FriendNetId )
{
	local bool bResult;
	local OnlineSubsystem.OnlineFriend FriendData;

	if ( GetOnlineFriendData(UTPlayerOwner, FriendNetId, FriendData) )
	{
		bResult = InStr(FriendData.PresenceInfo, "bRequiresPassword") != INDEX_NONE;
	}

	return bResult;
}

/**
 * Displays a dialog to the user which allows him to enter the password for the currently selected server.
 */
private function PromptForServerPassword()
{
	local UTUIScene_InputBox PasswordInputScene;

	PasswordInputScene = GetInputBoxScene();
	if ( PasswordInputScene != None )
	{
		PasswordInputScene.SetPasswordMode(true);
		PasswordInputScene.DisplayAcceptCancelBox(
			"<Strings:UTGameUI.MessageBox.EnterServerPassword_Message>",
			"<Strings:UTGameUI.MessageBox.EnterServerPassword_Title>",
			OnPasswordDialog_Closed
			);
	}
	else
	{
		`log("Failed to open the input box scene (" $ InputBoxScene $ ")");
	}
}

/**
 * The user has made a selection of the choices available to them.
 */
private function OnPasswordDialog_Closed(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local UTUIScene_InputBox PasswordInputScene;
	local string ServerPassword;

	PasswordInputScene = UTUIScene_InputBox(MessageBox);
	if ( PasswordInputScene != None && SelectedOption == 0 )
	{
		// strip out all
		ServerPassword = class'UTUIFrontEnd_HostGame'.static.StripInvalidPasswordCharacters(PasswordInputScene.GetValue());
		OnFollow(ServerPassword);
	}
}

/** Tries to follow the player that owns this playercard by joining their current game. */
function OnFollow( optional string ServerPassword )
{
	if ( FollowRequiresPassword(GetUTPlayerOwner(), PlayerNetId) && ServerPassword == "" )
	{
		PromptForServerPassword();
	}
	else
	{
		SetDataStoreStringValue("<Registry:ConnectPassword>", ServerPassword, Self, GetPlayerOwner());
		ProcessFollow();
	}
}
private function ProcessFollow()
{
	if(PlayerInt != None)
	{
		`Log("UTUIScene_PlayerCard::OnFollow() - Joining friend's game...");

		PlayerInt.AddJoinFriendGameCompleteDelegate(OnFollowComplete);
		if(PlayerInt.JoinFriendGame(GetPlayerOwner().ControllerId, PlayerNetId)==false)
		{
			OnFollowComplete(false);
		}
	}
}

/**
 * Called once the join task has completed
 *
 * @param bWasSuccessful the session was found and is joinable, false otherwise
 */
function OnFollowComplete(bool bWasSuccessful)
{
	PlayerInt.ClearJoinFriendGameCompleteDelegate(OnFollowComplete);
	bWasAbleToJoin = bWasSuccessful;
}

/** Callback for when the joining messagebox has closed. */
function OnJoiningMessage_Closed()
{
	`Log("UTUIScene_PlayerCard::OnJoiningMessage_Closed() - bWasAbleToJoin:"@bWasAbleToJoin);

	SetDataStoreStringValue("<Registry:ConnectPassword>", "", Self, GetPlayerOwner());
	if(bWasAbleToJoin==false)
	{
		DisplayMessageBox("<Strings:UTGameUI.Errors.JoinFriendFailed_Message>","<Strings:UTGameUI.Errors.JoinFriendFailed_Title>");
	}
}

function OnKick()
{
	local UTUIScene_MessageBox MessageBoxReference;

	MessageBoxReference = GetMessageBoxScene();
	if (MessageBoxReference!=none)
	{
		MessageBoxReference.DisplayAcceptCancelBox("<Strings:UTGameUI.PlayerCard.KickWarning>"@PlayerName,"<Strings:UTGameUI.Campaign.Confirmation", KickSelection);
	}
}

function KickSelection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local UTPlayerController PC;
	local string Target;
	local int i;
	local WorldInfo WI;

	WI = GetWorldInfo();

	Target = PlayerName;
	for (i=0;i<	WI.GRI.PRIArray.Length;i++)
	{
		if ( WI.GRI.PRIArray[i].UniqueId == PlayerNetId )
		{
			Target = WI.GRI.PRIArray[i].PlayerName;
			break;
		}
	}
	PC = GetUTPlayerOwner();
	if ( PC != none && SelectedOption == 0 )
	{
		PC.ServerKickBan(Target,false);
		CloseScene(Self);
	}
}

function OnBan()
{
	local UTUIScene_MessageBox MessageBoxReference;

	MessageBoxReference = GetMessageBoxScene();
	if (MessageBoxReference!=none)
	{
		MessageBoxReference.DisplayAcceptCancelBox("<Strings:UTGameUI.PlayerCard.BanWarning>"@PlayerName,"<Strings:UTGameUI.Campaign.Confirmation", BanSelection);
	}
}

function BanSelection(UTUIScene_MessageBox MessageBox, int SelectedOption, int PlayerIndex)
{
	local UTPlayerController PC;
	local string Target;
	local int i;
	local WorldInfo WI;

	WI = GetWorldInfo();

	Target = PlayerName;
	for (i=0;i<	WI.GRI.PRIArray.Length;i++)
	{
		if ( WI.GRI.PRIArray[i].UniqueId == PlayerNetId )
		{
			Target = WI.GRI.PRIArray[i].PlayerName;
			break;
		}
	}

	PC = GetUTPlayerOwner();
	if ( PC != none && SelectedOption == 0 )
	{
		PC.ServerKickBan(Target,true);
		CloseScene(Self);
	}
}

/**
 * Called when the user changes the currently selected list item.
 */
function OnMenu_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	local int SelectedItem;
	local string StringValue;

	SelectedItem = MenuList.GetCurrentItem();
	if ( SelectedItem < CurrentOptions.Length )
	{
		SelectedItem = CurrentOptions[SelectedItem];

		StringValue = DescLinks[SelectedItem];
		StringValue = Localize("PlayerCard",StringValue,"UTGameUI");
		DescriptionLabel.SetDatastoreBinding(StringValue);
	}
}



defaultproperties
{
	SendFriendRequestScene="UI_Scenes_Common.SendFriendRequest"
	SendOnlineMessageScene="UI_Scenes_Common.SendOnlineMessage"

	DescLinks(0)="SendFriendRequest_Desc"
	DescLinks(1)="SendMessage_Desc"
	DescLinks(2)="InviteToGame_Desc"
	DescLinks(3)="Follow_Desc"
	DescLinks(4)="Kick_Desc"
	DescLinks(5)="Ban_Desc"
}
