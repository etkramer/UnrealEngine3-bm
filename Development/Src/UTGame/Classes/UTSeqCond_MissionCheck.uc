/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSeqCond_MissionCheck extends SequenceCondition;

/** This is the mission we are checking for */
var() int MissionIDToTestFor;

event Activated()
{
	local WorldInfo WI;
	local UTGameReplicationInfo GRI;

	WI = GetWorldInfo();
	if ( WI != none )
	{
		GRI = UTGameReplicationInfo(WI.GRI);
		if ( GRI != none )
		{
			if (GRI.bStoryMode && GRI.SinglePlayerMissionID == MissionIDToTestFor)
			{
				OutputLinks[0].bHasImpulse = true;
				return;
			}
		}
	}

	OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="Single Player Mission Check"
	OutputLinks(0)=(LinkDesc="Mission Matches")
	OutputLinks(1)=(LinkDesc="Mission Does not Match")
}
