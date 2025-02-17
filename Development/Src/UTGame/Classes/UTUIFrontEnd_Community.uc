/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *
 * Community scene for UT3.
 */
class UTUIFrontEnd_Community extends UTUIFrontEnd_BasicMenu;

/** Possible options for this scene. */
enum CommunityOptions
{
	COMMUNITY_OPTION_NEWS,
	
	/*
	COMMUNITY_OPTION_DLC,
	COMMUNITY_OPTION_EPICCONTENT,
	COMMUNITY_OPTION_MYCONTENT,
	*/

	COMMUNITY_OPTION_FRIENDS,
	COMMUNITY_OPTION_MESSAGES,
	COMMUNITY_OPTION_STATS,
	COMMUNITY_OPTION_DEMOPLAYBACK,
	COMMUNITY_OPTION_ACHIEVEMENTS,
};

/** Scene references. */
var string	StatsScene;
var string	NewsScene;
var string	DemoPlaybackScene;
var string	FriendsScene;

/** PostInitialize event - Sets delegates for the scene. */
event PostInitialize( )
{
	Super.PostInitialize();

	MenuList.SetFocus(None);
}

/**
 * Executes a action based on the currently selected menu item.
 */
function OnSelectItem(int PlayerIndex=0)
{
	local int SelectedItem;
	SelectedItem = MenuList.GetCurrentItem();

	switch(SelectedItem)
	{
	case COMMUNITY_OPTION_NEWS:
		if(CheckLoginAndError(INDEX_NONE, true))
		{
			OnShowNews();
		}
		break;
	/* - @todo: Disabled for now.  Maybe enable for patch.
	case COMMUNITY_OPTION_DLC:
	case COMMUNITY_OPTION_EPICCONTENT:
		if(CheckLoginAndError(INDEX_NONE, true))
		{
			OnShowDLC();
		}
		break;
	case COMMUNITY_OPTION_MYCONTENT:
		OnShowMyContent();
		break;

*/
	case COMMUNITY_OPTION_ACHIEVEMENTS:
		if(CheckLoginAndError(INDEX_NONE,true))
		{
			OnShowAchievements(PlayerIndex);
		}
		break;
	case COMMUNITY_OPTION_FRIENDS:
		if(CheckLinkConnectionAndError() && CheckLoginAndError(INDEX_NONE,true) && CheckCommunicationPrivilegeAndError() && CheckContentPrivilegeAndError())
		{
			OnShowFriends();
		}
		break;
	case COMMUNITY_OPTION_MESSAGES:
		if(CheckLinkConnectionAndError() && CheckLoginAndError(INDEX_NONE,true) && CheckCommunicationPrivilegeAndError() && CheckContentPrivilegeAndError())
		{
			OnShowMessages();
		}
		break;
	case COMMUNITY_OPTION_STATS:
		if(CheckLinkConnectionAndError() && CheckLoginAndError(INDEX_NONE, true) && CheckOnlinePrivilegeAndError() )
		{
			OnShowStats();
		}
		break;
	case COMMUNITY_OPTION_DEMOPLAYBACK:
		OnShowDemoPlayback();
		break;
}
}

/** Shows the news scene. */
function OnShowNews()
{
	OpenSceneByName(NewsScene);
}

/** Shows the DLC scene or blade. */
function OnShowDLC()
{
	local OnlineSubsystem OnlineSub;

	if( IsConsole(CONSOLE_Xbox360) )	// Display DLC blade
	{
		OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
		if (OnlineSub != None)
		{
			if (OnlineSub.PlayerInterfaceEx != None)
			{
				if (OnlineSub.PlayerInterfaceEx.ShowContentMarketplaceUI(GetPlayerOwner().ControllerId) == false)
				{
					`Log("Failed to show the marketplace UI");
				}
			}
			else
			{
				`Log("OnlineSubsystem does not support the extended player interface");
			}
		}
	}
	else
	{
		OpenSceneByName(NewsScene, false, OnShowDLC_Finish);
	}
}

/**
 * Callback for when the NewsScene has finished opening, activates the Epic Content tab.
 *
 * @param OpenedScene	Reference to the opened news scene.
 */
function OnShowDLC_Finish(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_News NewsSceneInst;

	if ( bInitialActivation )
	{
		NewsSceneInst=UTUIFrontEnd_News(OpenedScene);
		NewsSceneInst.TabControl.ActivatePage(NewsSceneInst.EpicContentTab, GetPlayerIndex());
	}
}

/** Shows the my content tab. */
function OnShowMyContent()
{
	OpenSceneByName(NewsScene, false, OnShowMyContent_Finish);
}

/**
 * Callback for when the NewsScene has finished opening, activates the MyContent tab.
 *
 * @param OpenedScene	Reference to the opened news scene.
 */
function OnShowMyContent_Finish(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_News NewsSceneInst;

	if ( bInitialActivation )
	{
		NewsSceneInst=UTUIFrontEnd_News(OpenedScene);
		NewsSceneInst.TabControl.ActivatePage(NewsSceneInst.MyContentTab, GetPlayerIndex());
	}
}

/** Shows the stats scene. */
function OnShowStats()
{
	OpenSceneByName(StatsScene);
}

/** Shows the demo playback scene. */
function OnShowDemoPlayback()
{
	if ( GetWorldInfo().IsDemoBuild() )
	{
		DisplayMEssageBox("The demo playback is not available in the UT3 demo.");
	}
	else
	{
		OpenSceneByName(DemoPlaybackScene);
	}	
}

/** Achievements option selected, displays the achievements blade for the specified PlayerIndex. */
function OnShowAchievements(int PlayerIndex)
{
	local OnlineSubsystem OnlineSub;
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	if (OnlineSub != None)
	{
		if (OnlineSub.PlayerInterfaceEx != None)
		{
			// Show them for the requesting player
			if (OnlineSub.PlayerInterfaceEx.ShowAchievementsUI(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex)) == false)
			{
				`Log("Failed to show the achievements UI");
			}
		}
		else
		{
			`Log("OnlineSubsystem does not support the extended player interface");
		}
	}
}

/** Shows the friends screen. */
function OnShowFriends()
{
	OpenSceneByName(FriendsScene);
}

/** Shows the friends screen. */
function OnShowMessages()
{
	OpenSceneByName(FriendsScene, false, OnMessagesScene_Opened);
}

/** Callback for when the messages scene has opened. */
function OnMessagesScene_Opened(UIScene OpenedScene, bool bInitialActivation)
{
	local UTUIFrontEnd_Friends FriendsSceneInst;

	if ( bInitialActivation )
	{
		FriendsSceneInst = UTUIFrontEnd_Friends(OpenedScene);
		FriendsSceneInst.TabControl.ActivatePage(FriendsSceneInst.MessagesTab, GetPlayerIndex());
	}
}

defaultproperties
{
	StatsScene="UI_Scenes_FrontEnd.Scenes.Leaderboards"
	NewsScene="UI_Scenes_FrontEnd.Scenes.News"
	DemoPlaybackScene="UI_Scenes_ChrisBLayout.Scenes.DemoPlayback"
	FriendsScene="UI_Scenes_FrontEnd.Scenes.Friends"
}
