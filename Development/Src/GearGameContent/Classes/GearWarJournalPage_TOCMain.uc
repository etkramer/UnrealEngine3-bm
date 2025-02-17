/**
 * Main Table of Contents for War Journal - contains the list of buttons for navigating to each section of the War Journal.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearWarJournalPage_TOCMain extends GearWarJournalPage_TOCBase;

var	transient	GearUINavigationList	lstNavigation;

/* == Delegates == */

/* == Natives == */

/* == Events == */

/* == UnrealScript == */

/* == SequenceAction handlers == */

/* == Delegate handlers == */
/**
 * Called when the user presses Enter (or any other action bound to UIKey_SubmitListSelection) while this list has focus.
 *
 * @param	Sender	the list that is submitting the selection
 */
function NavListSelectionSubmitted( UIList Sender, optional int PlayerIndex=GetBestPlayerIndex() )
{
	local UIScene SceneResource;
	local GearUIScene_Base GearSceneResource;
	local GearUISceneWJ_PageBase WarJournalScene;
	local GearUINavigationList NavList;
	local string ScenePath;

	NavList = GearUINavigationList(Sender);
	if ( NavList != None )
	{
		ScenePath = NavList.GetSelectedScenePath();
		if ( ScenePath != "" )
		{
			SceneResource = UIScene(DynamicLoadObject(ScenePath, class'UIScene'));
			`log(`location @ "attempting to open scene '" $ ScenePath $ "':" @ `showobj(SceneResource));
			if ( SceneResource != None )
			{
				GearSceneResource = GearUIScene_Base(SceneResource);

				if ( GearSceneResource.bRequiresNetwork && !HasLinkConnection() )
				{
					GearSceneResource.DisplayErrorMessage("NoLinkConnection_Message", "NoLinkConnection_Title", "GenericContinue", GetPlayerOwner(PlayerIndex));
					return;
				}

				// validate that the player is able to open the leaderboards.
				if ( GearSceneResource.bRequiresOnlineService && !IsLoggedIn(class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex), true) )
				{
					GearSceneResource.DisplayErrorMessage("NeedGoldTierLeaderboard_Message", "NeedGoldTier_Title", "GenericContinue", GetPlayerOwner(PlayerIndex));
					return;
				}

				WarJournalScene = GearUISceneWJ_PageBase(SceneResource.OpenScene(SceneResource, GetPlayerOwner(PlayerIndex)));
				if ( WarJournalScene != None )
				{
					WarJournalScene.SetCurrentPage(0);
				}

				PlayUISound('G2UI_JournalNavigateCue');
			}
		}
	}
}



/* == GearWarJournalPage_Base interface == */
/**
 * Assigns all member variables to the appropriate child widgets in this panel.
 */
function InitializePageReferences()
{
	lstNavigation = GearUINavigationList(FindChild('lstTOCNav',true));
	lstNavigation.OnSubmitSelection = NavListSelectionSubmitted;

	Super.InitializePageReferences();
}

DefaultProperties
{

}
