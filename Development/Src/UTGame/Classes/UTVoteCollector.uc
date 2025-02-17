/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTVoteCollector extends Info
	native;

/**
 * The collection of maps + votes
 */
struct native MapVoteInfo
{
	var int 	MapID;			// An INT id that represents this map.
	var string 	Map;			// The Name of the map
	var byte 	NoVotes;		// Number of votes this map has

	structdefaultproperties
	{
		MapID = -1;	// Default to NO id.
	}
};

var array<MapVoteInfo> Votes;
var array<UTVoteReplicationInfo> VRIList;

var bool bVoteDecided;

var int WinningIndex;

native function int GetMapIndex(int MapID);


function string GetWinningMap()
{
	if (WinningIndex != INDEX_None)
	{
		return Votes[WinningIndex].Map;
	}
	else
	{
		return "";
	}
}

function Initialize(array<string> MapList)
{
	local UTPlayerController UTPC;
	local UTVoteReplicationInfo VoteRI;
	local int i;

	// Grab the map information from the game type.


//	`log("### VoteCollector.Initialize"@MapList.Length);

	Votes.Remove(0,Votes.Length);
	Votes.Length = MapList.Length;

	for (i=0;i<MapList.length;i++)
	{
		Votes[i].MapID = i;
		Votes[i].Map= MapList[i];
		Votes[i].NoVotes= 0;
	}

	foreach WorldInfo.AllControllers(class'UTPlayerController',UTPC)
	{
		VoteRI = Spawn(class'UTVoteReplicationInfo',UTPC);
//		`log("##VoteRI:"@VoteRI@VoteRI.Owner@VoteRI.bNetInitial@VoteRI.bNetDirty@VoteRI.bNetOwner@bSkipActorPropertyReplication);
		if ( VoteRI != none )
		{
			UTPC.VoteRI = VoteRI;
			VoteRI.Initialize(self);

			VRIList[VRIList.Length] = VoteRI;
		}
		else
		{
			`log("Could not spawn a VOTERI for"@UTPC.PlayerReplicationInfo.PlayerName@"("$UTPC$")");
		}
	}
}

function BroadcastVoteChange(int MapID, byte VoteCount)
{
	local int i;

//	`log("### Broadcasting Vote Update"@MapID@VoteCount);

	for (i=0;i<VRIList.Length;i++)
	{
//		`log("### Sending Vote Change to "@VRIList[i]);
		VRIList[i].ClientRecvMapUpdate(MapId,VoteCount);
	}
}


function RemoveVoteFor(out int CurrentVoteID)
{
	local int index;

//	`log("### Removing Vote for:"@CurrentVoteId@bVoteDecided);

	if ( bVoteDecided )
	{
		return;
	}


	Index = GetMapIndex(CurrentVoteID);
	if ( Index != INDEX_None )
	{
		Votes[Index].NoVotes = Clamp(Votes[Index].NoVotes - 1, 0, 255);
		BroadcastVoteChange(Votes[Index].MapID, Votes[Index].NoVotes);
	}
}

function int AddVoteFor(out int CurrentVoteID)
{
	local int index;

//	`log("### Adding Vote For"@CurrentVoteId@bVoteDecided);

	if ( bVoteDecided )
	{
		return INDEX_None;
	}

	Index = GetMapIndex(CurrentVoteID);
	if ( Index != INDEX_None )
	{
		Votes[Index].NoVotes = Clamp(Votes[Index].NoVotes + 1, 0, 255);
		BroadcastVoteChange(Votes[Index].MapID, Votes[Index].NoVotes);
		return Votes[Index].MapID;
	}
	return INDEX_None;
}

function TimesUp()
{
	local int i, BestIndex;

	bVoteDecided = true;

	for (i=0; i<VRIList.Length; i++)
	{
		VRIList[i].ClientTimesUp();
	}

	// Figure out which map had the most number of votes

	BestIndex = 0;
	for (i=1;i<Votes.Length;i++)
	{
		if (Votes[i].NoVotes > Votes[BestIndex].NoVotes)
		{
			BestIndex = i;
		}
	}

	if ( Votes[BestIndex].NoVotes > 0 )
	{
		WinningIndex = BestIndex;
	}

}



defaultproperties
{
	WinningIndex=-1
}
