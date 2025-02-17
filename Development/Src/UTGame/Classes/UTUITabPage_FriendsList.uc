/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Tab page for a user's friends list.
 */
class UTUITabPage_FriendsList extends UTTabPage
	placeable;

var transient UIList	FriendsList;
var string SendFriendRequestScene;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local UTUIScene OwnerUTScene;

	Super.PostInitialize();

	// Get widget references
	FriendsList = UIList(FindChild('lstFriends', true));
	FriendsList.OnSubmitSelection = OnFriendsList_SubmitSelection;
	FriendsList.OnValueChanged = OnFriendsList_ValueChanged;

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.Friends>");

	OwnerUTScene = UTUIScene(GetScene());
	PlayerData = UIDataStore_OnlinePlayerData(OwnerUTScene.FindDataStore('OnlinePlayerData', GetPlayerOwner()));
	// Tell the friends list to refresh itself
	if (PlayerData != None && PlayerData.FriendsProvider != None)
	{
		PlayerData.FriendsProvider.RefreshFriendsList();
	}
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.AddFriend>", OnButtonBar_AddFriend);

	if(FriendsList.GetCurrentItem()!=INDEX_NONE)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Details>", OnButtonBar_Details);
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.RemoveFriend>", OnButtonBar_RemoveFriend);
	}
}

/**
 * Gets the nickname for the currently selected friend.
 *
 * @param FriendNick	String to store nickname in.
 *
 * @return TRUE if a nickname was found, FALSE otherwise.
 */
function bool GetCurrentFriendNick(out string FriendNick)
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local int CurrentIndex;
	local UTUIScene OwnerUTScene;
	local bool Result;

	Result = false;
	CurrentIndex = FriendsList.GetCurrentItem();

	if(CurrentIndex!=INDEX_NONE)
	{
		OwnerUTScene = UTUIScene(GetScene());
		PlayerData = UIDataStore_OnlinePlayerData(OwnerUTScene.FindDataStore('OnlinePlayerData', GetPlayerOwner()));

		if(PlayerData != None && CurrentIndex < PlayerData.FriendsProvider.FriendsList.length)
		{
			FriendNick = PlayerData.FriendsProvider.FriendsList[CurrentIndex].NickName;
			Result = true;
		}
	}

	return Result;
}

/**
 * Gets the netid for the currently selected friend.
 *
 * @param FriendNetId	Object to store netid in.
 *
 * @return TRUE if a netid was found, FALSE otherwise.
 */
function bool GetCurrentFriendNetId(out UniqueNetId FriendNetId)
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local int CurrentIndex;
	local UTUIScene OwnerUTScene;
	local bool Result;

	OwnerUTScene = UTUIScene(GetScene());

	CurrentIndex = FriendsList.GetCurrentItem();
	PlayerData = UIDataStore_OnlinePlayerData(OwnerUTScene.FindDataStore('OnlinePlayerData', GetPlayerOwner()));

	Result = false;
	if(PlayerData != None && CurrentIndex < PlayerData.FriendsProvider.FriendsList.length)
	{
		FriendNetId = PlayerData.FriendsProvider.FriendsList[CurrentIndex].UniqueId;
		Result = true;
	}

	return Result;
}


/** Shows the add friend screen for the player. */
function OnAddFriend()
{
	UTUIScene(GetScene()).OpenSceneByName(SendFriendRequestScene);
}

/** Shows the details screen for the player. */
function OnDetails()
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local int CurrentIndex;
	local UTUIScene OwnerUTScene;

	OwnerUTScene = UTUIScene(GetScene());

	CurrentIndex = FriendsList.GetCurrentItem();
	PlayerData = UIDataStore_OnlinePlayerData(OwnerUTScene.FindDataStore('OnlinePlayerData', GetPlayerOwner()));

	if(PlayerData != None && CurrentIndex < PlayerData.FriendsProvider.FriendsList.length)
	{
		OwnerUTScene.ShowPlayerCard(PlayerData.FriendsProvider.FriendsList[CurrentIndex].UniqueId, PlayerData.FriendsProvider.FriendsList[CurrentIndex].NickName);
	}
}

/** Removes the current friend from the user's friend list. */
function OnRemoveFriend()
{
	local array<string> MessageBoxOptions;
	local string FinalMsg;
	local string FriendNick;
	local UTUIScene_MessageBox MessageBoxReference;

	if(GetCurrentFriendNick(FriendNick))
	{
		// Display a message to the user to confirm they want to remove the friend.
		MessageBoxReference = UTUIScene(GetScene()).GetMessageBoxScene();

		if(MessageBoxReference != none)
		{
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.RemoveFriendConfirm>");
			MessageBoxOptions.AddItem("<Strings:UTGameUI.ButtonCallouts.Cancel>");
			MessageBoxReference.SetPotentialOptions(MessageBoxOptions);

			FinalMsg = Repl(Localize("MessageBox", "RemoveFriend_Message", "UTGameUI"),"\`PlayerName\`",FriendNick);
			MessageBoxReference.Display(FinalMsg, "<Strings:UTGameUI.MessageBox.RemoveFriend_Title>", OnRemoveFriend_Confirm, 1);
		}

		MessageBoxReference = None;
	}
}

/** Confirmation for the exit game dialog. */
function OnRemoveFriend_Confirm(UTUIScene_MessageBox MessageBox, int SelectedItem, int PlayerIndex)
{
	local UniqueNetId FriendNetId;
	local OnlinePlayerInterface PlayerInt;

	PlayerInt = UTUIScene(GetScene()).GetPlayerInterface();
	if(SelectedItem==0 && GetCurrentFriendNetId(FriendNetId) && PlayerInt!=None)
	{
		PlayerInt.RemoveFriend(GetPlayerOwner().ControllerId, FriendNetId);
		FriendsList.RemoveElement(FriendsList.GetCurrentItem());
	}
}

/**
 * Called when the user submits the current list index.
 *
 * @param	Sender	the list that is submitting the selection
 */
function OnFriendsList_SubmitSelection( UIList Sender, optional int PlayerIndex=0 )
{
	OnDetails();
}


/** Callback for when the user changes the currently selected list item. */
function OnFriendsList_ValueChanged( UIObject Sender, optional int PlayerIndex=0 )
{
	UTUIFrontEnd(GetScene()).SetupButtonBar();
}

/** Buttonbar Callbacks */
function bool OnButtonBar_AddFriend(UIScreenObject InButton, int PlayerIndex)
{
	OnAddFriend();

	return true;
}

function bool OnButtonBar_Details(UIScreenObject InButton, int PlayerIndex)
{
	OnDetails();

	return true;
}

function bool OnButtonBar_RemoveFriend(UIScreenObject InButton, int PlayerIndex)
{
	OnRemoveFriend();

	return true;
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
		if(EventParms.InputKeyName=='XboxTypeS_Y')
		{
			OnAddFriend();
			bResult=true;
		}
		else if(EventParms.InputKeyName=='XboxTypeS_X')
		{
			OnRemoveFriend();
			bResult=true;
		}
	}

	return bResult;
}

DefaultProperties
{
	SendFriendRequestScene="UI_Scenes_Common.SendFriendRequest"
}
