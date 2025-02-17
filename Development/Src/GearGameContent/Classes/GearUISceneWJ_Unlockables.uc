/**
 * Displays the list of unlockable content.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneWJ_Unlockables extends GearUISceneWJ_PageBase;

/* === GearUISceneWJ_Base interface === */
/**
 * Calculates the total number of pages required by this scene.
 *
 * @return	the total number of pages needed to display all content for this journal type.
 */
function int CalculateRequiredPageCount( int PlayerIndex=GetBestPlayerIndex() )
{
	return 2;
}

/* === UIScene interface === */
event SceneDeactivated()
{
	local GearWarJournalPage_UnlockablesList ListPage;
	local GearPC PC;

	Super.SceneDeactivated();

	ListPage = GearWarJournalPage_UnlockablesList(LeftPage);
	if ( ListPage != None && ListPage.bSaveProfile )
	{
		PC = GetGearPlayerOwner();
		if ( PC != None )
		{
			PC.SaveProfile();
		}
	}
}

DefaultProperties
{
	PreviousChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.CollectablesJournal'
	NextChapterScene=GearUISceneWJ_Base'UI_Scenes_WarJournal.AchievementsJournal'
}
