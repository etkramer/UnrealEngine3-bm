/**
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */

/** Specialized GRI for dealing with campaign game informaton */
class GearCampaignGRI extends GearGRI;

/** The number of slots filled by people that are loading into the game */
var int ConnectingPlayerCount;

/** The current chapter set in the lobby */
var repnotify EChapterPoint CurrChapter;

/** The current act set in the lobby */
var EGearAct CurrAct;

/** The normalized index of the CurrChapter in the act it is in (index in the widget of the chapter) */
var int NormalizedChapterIndex;

/** Whether the campaign is currently set up for a checkpoint or not */
var repnotify EGearCheckpointUsage CheckpointUsage;

/** The current campaign mode of the lobby */
var repnotify EGearCampMode CampaignMode;

/** The current invite policy of the lobby */
var repnotify EGearCoopInviteType InviteType;

/** Whether we are blocking the player from accessing crucial areas of the lobby */
var bool bIsInputGuarding;

replication
{
	if (bNetDirty)
		ConnectingPlayerCount, CurrChapter, CheckpointUsage, CampaignMode, InviteType, bIsInputGuarding;
}


/** Called when a change is made to the CurrChapter */
delegate OnCurrentChapterChanged();

/** Called when a change is made to the CheckpointUsage */
delegate OnCheckpointChanged();

/** Called when a change is made to the match mode */
delegate OnCampaignModeChanged();

/** Called when a change is made to the invite type of the lobby */
delegate OnInviteTypeChanged();


/** Sets the current chapter from the lobby */
simulated function SetCurrentChapter(EChapterPoint NewChapter)
{
	CurrChapter = NewChapter;
	NormalizedChapterIndex = class'GearUIDataStore_GameResource'.static.GetActChapterProviderIndexFromChapterId(CurrChapter);
	CurrAct = GetActFromChapter(CurrChapter);
}

/** Returns the act that the chapter is in */
simulated function EGearAct GetActFromChapter( EChapterPoint Chapter )
{
	local GearCampaignActData ActData;

	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(Chapter);
	if (ActData != None)
	{
		return ActData.ActType;
	}
	return INDEX_NONE;
}

/**
 * Does per replicated variable handling on the client
 *
 * @param VarName the variable that was just replicated
 */
simulated event ReplicatedEvent(name VarName)
{
	if (VarName == 'CurrChapter')
	{
		SetCurrentChapter(CurrChapter);
		OnCurrentChapterChanged();
	}
	else if (VarName == 'CheckpointUsage')
	{
		OnCheckpointChanged();
	}
	else if (VarName == 'CampaignMode')
	{
		OnCampaignModeChanged();
	}
	else if (VarName == 'InviteType')
	{
		OnInviteTypeChanged();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

DefaultProperties
{
}
