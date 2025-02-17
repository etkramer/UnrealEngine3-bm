/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTUIScene_MapVote extends UTUIScene_Hud
	native(UI);

var transient UTDrawMapVotePanel MapList;
var transient UILabel TimeRemaining;
var transient UTVoteReplicationInfo VoteRI;

cpptext
{
	virtual void Tick( FLOAT DeltaTime );
}


event PostInitialize()
{
	super.PostInitialize();

	MapList = UTDrawMapVotePanel( FindChild('MapList',true));
	TimeRemaining = UILabel( FindChild('TimeRemaining',true));
}

function InitializeVRI(UTVoteReplicationInfo NewVoteRI)
{
	if ( MapList != none )
	{
		MapList.InitializeVRI(NewVoteRI);
	}
}



defaultproperties
{
	SceneInputMode=INPUTMODE_MatchingOnly
	SceneRenderMode=SPLITRENDER_PlayerOwner
	bDisplayCursor=true
	bRenderParentScenes=false
	bAlwaysRenderScene=true
	bCloseOnLevelChange=true
}
