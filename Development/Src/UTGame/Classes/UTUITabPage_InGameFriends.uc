/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUITabPage_InGameFriends extends UTTabPage_MidGame;

var transient UIList	FriendsList;
var transient UTUIScene_MidGameMenu MidGameOwner;

/** Post initialization event - Setup widget delegates.*/
event PostInitialize()
{
	Super.PostInitialize();

	// Get widget references
	FriendsList = UIList(FindChild('lstFriends', true));
	FriendsList.OnSubmitSelection = OnFriendsList_SubmitSelection;
	FriendsList.OnValueChanged = OnFriendsList_ValueChanged;

	// Set the button tab caption.
	SetDataStoreBinding("<Strings:UTGameUI.Community.Friends>");
	MidGameOwner = UTUIScene_MidGameMenu(GetScene());
}

/** Callback allowing the tabpage to setup the button bar for the current scene. */
function SetupButtonBar(UTUIButtonBar ButtonBar)
{
	if(FriendsList.GetCurrentItem()!=INDEX_NONE)
	{
		ButtonBar.AppendButton("<Strings:UTGameUI.ButtonCallouts.Details>", OnButtonBar_Details);
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



/** Shows the details screen for the player. */
function OnDetails()
{
	local UIDataStore_OnlinePlayerData PlayerData;
	local int CurrentIndex;
	local UTUIScene OwnerUTScene;
	local UTUIScene_PlayerCard CardScene;

	OwnerUTScene = UTUIScene(GetScene());



	CurrentIndex = FriendsList.GetCurrentItem();
	PlayerData = UIDataStore_OnlinePlayerData(OwnerUTScene.FindDataStore('OnlinePlayerData', GetPlayerOwner()));

	if(PlayerData != None && CurrentIndex < PlayerData.FriendsProvider.FriendsList.length)
	{
		CardScene = OwnerUTScene.ShowPlayerCard(PlayerData.FriendsProvider.FriendsList[CurrentIndex].UniqueId, PlayerData.FriendsProvider.FriendsList[CurrentIndex].NickName);
		if ( CardScene != none )
		{
			MidGameOwner.FindChild('MidGameSafeRegion',true).PlayUIAnimation('FadeOut',,,3.0);
			CardScene.OnSCeneDeactivated = OnSceneDeactivated;
		}
	}
}


function OnSceneDeactivated( UIScene DeactivatedScene )
{
	MidGameOwner.FindChild('MidGameSafeRegion',true).PlayUIAnimation('FadeIn',,,3.0);
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
	local UTUIScene UTOwnerScene;

	UTOwnerScene = UTUIScene(GetScene());
	if ( UTOwnerScene != None )
	{
		UTOwnerScene.SetupButtonBar();
	}
}

function bool OnButtonBar_Details(UIScreenObject InButton, int PlayerIndex)
{
	OnDetails();

	return true;
}

DefaultProperties
{
}
