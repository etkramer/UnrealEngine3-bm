/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for messages a user has received.
 */
class UTUITabPage_Messages extends UTTabPage
	placeable
	dependson(UTUIScene_MessageBox);

var transient UIList	MessageList;

/** Reference to the online player interface. */
var transient OnlinePlayerInterface PlayerInt;

/** Reference to the player messages provider. */
var transient UIDataProvider_OnlineFriendMessages PlayerMessages;

/** Reference to the online player datastore. */
var transient UIDataStore_OnlinePlayerData PlayerData;

/** Reference to a message box scene. */
var transient UTUIScene_MessageBox MessageBoxReference;

/** Net ID of the player we are going to send a friend request to. */
var transient UniqueNetId TargetPlayerNetId;

/** Name of the player we are going to send a friend request to. */
var transient string TargetPlayerName;

/** Reference to the send friend request screen. */
var transient string SendFriendRequestScene;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Get widget references
	MessageList = UIList(FindChild('lstMessages', true));
	MessageList.OnSubmitSelection = OnMessagesList_SubmitSelection;
	MessageList.OnValueChanged = OnMessagesList_ValueChanged;

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.Messages>");

	PlayerInt = UTUIScene(GetScene()).GetPlayerInterface();
	PlayerData = UIDataStore_OnlinePlayerData(UTUIScene(GetScene()).FindDataStore('OnlinePlayerData', GetPlayerOwner()));
	PlayerMessages = PlayerData.FriendMessagesProvider;
}


function UniqueNetId GetNetIdFromIndex(int MessageIdx)
{
	return PlayerMessages.Messages[MessageIdx].SendingPlayerId;
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(MessageList.GetCurrentItem()!=INDEX_NONE)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.ViewMessage>", OnButtonBar_ViewMessage);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.DeleteMessage>", OnButtonBar_DeleteMessage);
	}
}

/** Shows the currently message to the player. */
function OnViewMessage()
{
	local int MessageIdx;
	local string FinalString;
	local array<string> MessageBoxOptions;
	local array<PotentialOptionKeys> PotentialOptionKeyMappings;

	MessageIdx = MessageList.GetCurrentItem();

	// see if we are displaying a friend invite or a normal message
	if(PlayerMessages.Messages[MessageIdx].bIsFriendInvite)
	{

		// Pop up a message box asking the user if they want to accept, decline, or cancel the message
		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

		if(MessageBoxReference != none)
		{
			FinalString = Localize("MessageBox", "ViewFriendRequest_Message", "UTGameUI");
			FinalString = Repl(FinalString, "\`PlayerName\`", PlayerMessages.Messages[MessageIdx].SendingPlayerNick);
			FinalString = Repl(FinalString, "\`Message\`", PlayerMessages.Messages[MessageIdx].Message);

			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.AcceptFriendRequest>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DeclineFriendRequest>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.Generic.Cancel>");

			PotentialOptionKeyMappings.length = 3;
			PotentialOptionKeyMappings[0].Keys.AddItem('XboxTypeS_A');
			PotentialOptionKeyMappings[1].Keys.AddItem('XboxTypeS_X');
			PotentialOptionKeyMappings[2].Keys.AddItem('XboxTypeS_B');
			PotentialOptionKeyMappings[2].Keys.AddItem('Escape');

			MessageBoxReference.SetPotentialOptionKeyMappings(PotentialOptionKeyMappings);
			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.ViewFriendRequest_Title>", OnFriendInvite_Confirm);
		}


	}
	else if(PlayerMessages.Messages[MessageIdx].bIsGameInvite)
	{
		// Pop up a message box asking the user if they want to accept, decline, or cancel the message
		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

		if(MessageBoxReference != none)
		{
			FinalString = Localize("MessageBox", "ViewGameInvite_Message", "UTGameUI");
			FinalString = Repl(FinalString, "\`PlayerName\`", PlayerMessages.Messages[MessageIdx].SendingPlayerNick);
			FinalString = Repl(FinalString, "\`Message\`", PlayerMessages.Messages[MessageIdx].Message);

			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.AcceptGameInvite>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DeclineGameInvite>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.Generic.Cancel>");

			PotentialOptionKeyMappings.length = 3;
			PotentialOptionKeyMappings[0].Keys.AddItem('XboxTypeS_A');
			PotentialOptionKeyMappings[1].Keys.AddItem('XboxTypeS_X');
			PotentialOptionKeyMappings[2].Keys.AddItem('XboxTypeS_B');
			PotentialOptionKeyMappings[2].Keys.AddItem('Escape');

			MessageBoxReference.SetPotentialOptionKeyMappings(PotentialOptionKeyMappings);
			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
			MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.ViewGameInvite_Title>", OnGameInvite_Confirm);
		}
	}
	else
	{
		FinalString = Localize("MessageBox", "ViewMessage_Message", "UTGameUI");
		FinalString = Repl(FinalString, "\`PlayerName\`", PlayerMessages.Messages[MessageIdx].SendingPlayerNick);
		FinalString = Repl(FinalString, "\`Message\`", PlayerMessages.Messages[MessageIdx].Message);
		UTUIScene(GetScene()).DisplayMessageBox(FinalString, "<Strings:UTGameUI.MessageBox.ViewMessage_Title>");
	}
}

/** Refreshes the message list. */
function OnMessageListChanged()
{
	local int CurrentIndex;

	CurrentIndex = MessageList.Index;
	MessageList.RefreshSubscriberValue();
	MessageList.SetIndex(CurrentIndex);
}

/** Deletes the currently selected message. */
function OnDeleteMessage()
{
	local int MessageIdx;
	local string FinalString;
	local array<string> MessageBoxOptions;

	MessageIdx = MessageList.GetCurrentItem();

	// Display messagebox to confirm deletion.
	MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

	if(MessageBoxReference != none)
	{

		FinalString = Localize("MessageBox", "DeleteMessage_Message", "UTGameUI");
		FinalString = Repl(FinalString, "\`PlayerName\`", PlayerMessages.Messages[MessageIdx].SendingPlayerNick);

		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.DeleteMessage>");
		MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");

		MessageBoxReference.SetPotentialOptions(MessageBoxOptions);
		MessageBoxReference.Display(FinalString, "<Strings:UTGameUI.MessageBox.DeleteMessage_Title>", OnDeleteMessage_Confirm, 1);
	}
}

/** Confirmation for the delete message dialog. */
function OnDeleteMessage_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local int CurrentMessageIdx;
	CurrentMessageIdx = MessageList.GetCurrentItem();

	MessageBoxReference = None;

	if(SelectedItem==0)
	{
		PlayerInt.DeleteMessage(GetPlayerOwner().ControllerId,CurrentMessageIdx);
		PlayerMessages.Messages.Remove(CurrentMessageIdx,1);
		OnMessageListChanged();
	}
}

/** Confirmation for the accept friend invite dialog. */
function OnFriendInvite_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local int CurrentMessageIdx;

	CurrentMessageIdx = MessageList.GetCurrentItem();
	MessageBoxReference = None;

	switch(SelectedItem)
	{
	case 0:
		PlayerInt.AcceptFriendInvite(GetPlayerOwner().ControllerId, GetNetIdFromIndex(CurrentMessageIdx));

		TargetPlayerName=PlayerMessages.Messages[CurrentMessageIdx].SendingPlayerNick;
		TargetPlayerNetId=GetNetIdFromIndex(CurrentMessageIdx);

		// If the other user isnt a friend already display the add friend screen with their name
		if(PlayerInt.IsFriend(GetPlayerOwner().ControllerId, TargetPlayerNetId)==false && Len(TargetPlayerName)>0)
		{
			OnSendFriendRequest();
		}


		// Remove invite from list
		PlayerInt.DeleteMessage(GetPlayerOwner().ControllerId,CurrentMessageIdx);
		PlayerMessages.Messages.Remove(CurrentMessageIdx,1);
		OnMessageListChanged();
		break;
	case 1:
		PlayerInt.DenyFriendInvite(GetPlayerOwner().ControllerId, GetNetIdFromIndex(CurrentMessageIdx));

		// Remove invite from list
		PlayerInt.DeleteMessage(GetPlayerOwner().ControllerId,CurrentMessageIdx);
		PlayerMessages.Messages.Remove(CurrentMessageIdx,1);
		OnMessageListChanged();
		break;
	default:
		// Do nothing for the default case
	}
}

/** Sends a friend request to the owner of this player card. */
function OnSendFriendRequest()
{
	UTUIScene(GetScene()).OpenSceneByName(SendFriendRequestScene, false, OnSendFriendRequest_Opened);
}

/** Callback for when the send message scene has actually been opened.  Sets the target player for the scene. */
function OnSendFriendRequest_Opened(UIScene OpenedScene, bool bInitialActivation)
{
	if ( bInitialActivation )
	{
		UTUIScene_SendFriendRequest(OpenedScene).SetTargetPlayer(TargetPlayerNetId, TargetPlayerName);
	}
}

/** Confirmation for the accept game invite dialog. */
function OnGameInvite_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local int CurrentMessageIdx;

	CurrentMessageIdx = MessageList.GetCurrentItem();
	MessageBoxReference = None;

	switch(SelectedItem)
	{
	case 0:
		ProcessGameInvite(CurrentMessageIdx);
		break;

	case 1:
		// Remove invite from list
		PlayerInt.DeleteMessage(GetPlayerOwner().ControllerId,CurrentMessageIdx);
		PlayerMessages.Messages.Remove(CurrentMessageIdx,1);
		OnMessageListChanged();
		break;
	default:
		// Do nothing for the default case
	}
}

/**
 * Displays a dialog to the user which allows him to enter the password for the currently selected server.
 */
private function PromptForServerPassword()
{
	local UTUIScene UTSceneOwner;
	local UTUIScene_InputBox PasswordInputScene;

	UTSceneOwner = UTUIScene(GetScene());
	if ( UTSceneOwner != None )
	{
		PasswordInputScene = UTSceneOwner.GetInputBoxScene();
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
			`log("Failed to open the input box scene (" $ UTSceneOwner.InputBoxScene $ ")");
		}
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
		ProcessGameInvite(MessageList.GetCurrentItem(), ServerPassword);
	}
}

/**
 * Wrapper function which displays a prompt dialog if the server we're about to join has a password.
 *
 * @param	SelectedMessageItem		the value of the index for the message corresponding to the game invite (i.e. MessageList.Items[MessageList.Index])
 * @param	ServerPassword			the password to use when connecting to the game
 */
function ProcessGameInvite( int SelectedMessageItem, optional string ServerPassword )
{
	local OnlineSubsystem.UniqueNetId FriendNetId;
	local UTPlayerController OwnerPC;
	local UTUIScene OwnerUTScene;

	// check to see if we need to supply a password
	OwnerUTScene = UTUIScene(GetScene());
	OwnerPC = OwnerUTScene.GetUTPlayerOwner();
	FriendNetId = GetNetIdFromIndex(SelectedMessageItem);

	if (class'UTUIScene_PlayerCard'.static.FollowRequiresPassword(OwnerPC, FriendNetId)
	&&	ServerPassword == ""  )
	{
		PromptForServerPassword();
	}
	else
	{
		SetDataStoreStringValue("<Registry:ConnectPassword>", ServerPassword, GetScene(), GetPlayerOwner());
		ProcessFollow(SelectedMessageItem);
	}
}

/**
 * Final function in the "join game invite" process - interacts with the online subsystem to initiate the join.  At this
 * point, we've made sure that all conditions for a successful join are met, including prompting the user for the password (if required).
 */
private function ProcessFollow( int SelectedMessageItem )
{
	//MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();
	//MessageBoxReference.DisplayModalBox("<Strings:UTGameUI.MessageBox.JoinFriend_Message>", "");

	//GameInt.AddGameInviteAcceptedDelegate(GetPlayerOwner().ControllerId,OnGameInviteAccepted);
	PlayerInt.JoinFriendGame(GetPlayerOwner().ControllerId,GetNetIdFromIndex(SelectedMessageItem));
	//{
	//	OnGameInviteAccepted(None);
	//}

	// Remove invite from list
	PlayerInt.DeleteMessage(GetPlayerOwner().ControllerId,SelectedMessageItem);
	PlayerMessages.Messages.Remove(SelectedMessageItem,1);
	OnMessageListChanged();
}

/** Callback for when the game invite accepted operation has completed. */
function OnGameInviteAccepted(OnlineGameSettings GameInviteSettings)
{
	local OnlineGameInterface GameInt;

	GameInt = UTUIScene(GetScene()).GetGameInterface();
	GameInt.ClearGameInviteAcceptedDelegate(GetPlayerOwner().ControllerId,OnGameInviteAccepted);
	MessageBoxReference.Close();
	MessageBoxReference=None;
}


/** Marks the currently selected message as unread. */
function OnMarkAsUnread()
{

}

/** Buttonbar Callbacks */
function bool OnButtonBar_ViewMessage(UIScreenObject InButton, int PlayerIndex)
{
	OnViewMessage();

	return true;
}

function bool OnButtonBar_DeleteMessage(UIScreenObject InButton, int PlayerIndex)
{
	OnDeleteMessage();

	return true;
}

function bool OnButtonBar_MarkAsUnread(UIScreenObject InButton, int PlayerIndex)
{
	OnMarkAsUnread();

	return true;
}

/**
 * Called when the user submits the current list index.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnMessagesList_SubmitSelection( UIList Sender, optional int PlayerIndex=0 )
{
	OnViewMessage();
}


/** Callback for when the user changes the currently selected list item. */
function OnMessagesList_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	UTUIFrontEnd(GetScene()).SetupButtonBar();
}



/**
 * Provides a hook for unrealscript to respond to input using actual input key names (i.e. Left, Tab, etc.)
 *
 * Called when an input key event is received which this widget responds to and is in the correct state to process.  The
 * keys and states widgets receive input for is managed through the UI editor's key binding dialog (F8).
 *
 * This delegate is called BEFORE kismet is given a chance to process the input.
 *
 * @param	EventParms	information about the input event.
 *
 * @return	TRUE to indicate that this input key was processed; no further processing will occur on this input key event.
 */
function bool HandleInputKey( const out InputEventParameters EventParms )
{
	local bool bResult;

	bResult=false;

	if(EventParms.EventType==IE_Released)
	{
		/* - Disabled because messages dont persist after logging out.

		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnDeleteMessage();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			OnMarkAsUnread();
			bResult=true;
		}
		 */
	}

	return bResult;
}

DefaultProperties
{
	SendFriendRequestScene="UI_Scenes_Common.SendFriendRequest"
}
