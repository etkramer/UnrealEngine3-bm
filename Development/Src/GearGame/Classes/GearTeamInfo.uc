/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearTeamInfo extends TeamInfo
	native;

enum EGearTeam
{
	TEAM_COG,
	TEAM_LOCUST,
	TEAM_EVERYONE,
};

/** List of all players on this team */
var array<Controller> TeamMembers;

/** List of squads */
var array<GearSquad> Squads;

// Stored AI Info
var float	LastWretchScreamTime;

/** total team score across all rounds (Horde only) */
var int TotalScore;

replication
{
	if (bNetDirty)
		TotalScore;
}

function bool AddToTeam( Controller Other )
{
	local GearPRI		PRI;

	if( Other != None &&
		Other.PlayerReplicationInfo != None )
	{
		Size++;
		Other.PlayerReplicationInfo.SetPlayerTeam(Self);

		// figure out squad assignment
		PRI = GearPRI(Other.PlayerReplicationInfo);
		// force the player squad
		if (Other.IsA('PlayerController') &&
			PRI != None)
		{
			PRI.SquadName = 'Player';
			// Create the squad
			JoinSquad('Alpha',Other,TRUE);
		}

		//debug
		//`Log( "Adding"@Other@"to team"@PRI.SquadName@PRI.FireTeamIndex@PRI.bFireTeamLeader );
		TeamMembers[TeamMembers.Length] = Other;

		return true;
	}
	else
	{
		return false;
	}
}

function RemoveFromTeam( Controller Other )
{
	local int Idx;
	local GearPC PC;
	local GearAI AI;

	Idx = TeamMembers.Find(Other);
	if (Idx != -1)
	{
		TeamMembers.Remove(Idx,1);
	}

	PC = GearPC(Other);
	if (PC != None)
	{
		if (PC.Squad != None && Squads.Find(PC.Squad) != INDEX_NONE)
		{
			PC.Squad.UnregisterSquadMember(PC);
		}
	}
	else
	{
		AI = GearAI(Other);
		if (AI != None && AI.Squad != None && Squads.Find(AI.Squad) != INDEX_NONE)
		{
			AI.Squad.UnregisterSquadMember(AI);
		}
	}
	Size--;
	if ( Other != None && Other.PlayerReplicationInfo != None )
	{
		Other.PlayerReplicationInfo.SetPlayerTeam(None);
	}
}

function NotifyKilled(Controller Killer, Controller Killed, Pawn KilledPawn)
{
}

/**
 * Called by individual pawns upon being damaged.  Allows team to track damage,
 * specifically for morale modifiers.
 */
function NotifyTakeDamage(Controller Victim, Controller Damager)
{
}

/** Creates a squad if necessary, otherwise returns the existing one. */
private final function GearSquad ConditionalCreateSquad( Name SquadName, optional class<GearSquad> SquadClass = class'GearSquad' )
{
	local int Idx;
	local GearSquad Squad;
	local array<Controller> OldMembers;
	local Controller OldLeader;

	// look for an existing squad
	for( Idx = 0; Idx < Squads.Length; Idx++ )
	{
		// cull null entries
		if (Squads[Idx] == None || Squads[Idx].bDeleteMe)
		{
			Squads.Remove(Idx--,1);
		}
		// if it matches the squad name
		else if (Squads[Idx].SquadName == SquadName)
		{
			Squad = Squads[Idx];
			// if the squad class doesn't match
			if( SquadClass != None &&
				!ClassIsChildOf(Squad.Class,SquadClass))
			{
				// grab the members of the current squad
				Squad.GetMembers(OldMembers);
				OldLeader = Squad.Leader;
				// and destroy it so it can be created below
				Squad.Destroy();
				Squad = None;
				Squads.Remove(Idx--,1);
			}
			break;
		}
	}

	if( Squad == None )
	{
		// none found, so create one now
		if( SquadClass == None )
		{
			SquadClass = class'GearSquad';
		}

		Squad = Spawn(SquadClass);
		Squad.SquadName = SquadName;
		Squad.Team = self;

		//debug
		//`log( "created squad"@Squad@Squad.Name );

		// restore any previous members of the squad
		if( OldLeader != None )
		{
			Squad.RegisterSquadMember( OldLeader, TRUE );
		}

		for( Idx = 0; Idx < OldMembers.Length; Idx++ )
		{
			Squad.RegisterSquadMember(OldMembers[Idx],FALSE);
		}

		// and add to the list of squads
		Squads[Squads.Length] = Squad;
	}
	return Squad;
}

/** Called by players to join the squad */
function JoinSquad( Name SquadName, Controller NewMember, optional bool bLeader, optional class<GearSquad> SquadClass)
{
	local GearSquad Squad;
	// handle the default player squad
	if( SquadName == 'Player' )
	{
		SquadName = 'Alpha';
	}

	// get the squad
	Squad = ConditionalCreateSquad(SquadName,SquadClass);
	if( Squad!= None )
	{
		// if this is already a player squad, don't mess with leader
		if(PlayerController(NewMember) != none && Squad.bPlayerSquad && Squad.Leader != none)
		{
			bLeader = false;
		}
		Squad.RegisterSquadMember(NewMember,bLeader);
	}
	else
	{
		`warn("Failed to find/create squad:"@SquadName@"for"@NewMember);
	}
}

/** creates squads for an MP game mode
 * basically, this means, give each human a unique squad and divide up any bots between those squads
 */
function CreateMPSquads()
{
	local array<Controller> RemainingMembers;
	local int i, NumSquads, SquadIdx;
	local GearPC PC;
	local GearAI_TDM AI;
	local bool bSecondPass;

	if (TeamMembers.length == 0)
	{
		return;
	}

	TeamMembers.RemoveItem(None);
	RemainingMembers = TeamMembers;

	NumSquads = FCeil(float(RemainingMembers.length) / float(GearGameMP_Base(WorldInfo.Game).MaxSquadSize));
	SquadIdx = 0;

	// give humans leadership first
	while (i < RemainingMembers.length)
	{
		PC = GearPC(RemainingMembers[i]);
		if (PC != None)
		{
			// remove any current squad
			if (PC.Squad != None)
			{
				PC.Squad.UnregisterSquadMember(PC);
			}
			// give the player a unique squad
			JoinSquad(name("MPSquad_" $ SquadIdx), PC, true);
			GearPRI(PC.PlayerReplicationInfo).SquadName = PC.Squad.SquadName;
			SquadIdx++;
			// expand number of squads if necessary to give each human a unique squad
			NumSquads = Max(NumSquads, SquadIdx);
			RemainingMembers.Remove(i, 1);
		}
		else
		{
			i++;
		}
	}

	if (SquadIdx < NumSquads)
	{
		// give AI team leader a squad if it exists
		// then create any remaining required squads out of arbitrary AI guys
		i = 0;
		while (i < RemainingMembers.length && SquadIdx < NumSquads)
		{
			AI = GearAI_TDM(RemainingMembers[i]);
			if (AI != None && (bSecondPass || GearPRI(AI.PlayerReplicationInfo).bIsLeader))
			{
				if (AI.Squad != None)
				{
					AI.Squad.UnregisterSquadMember(AI);
				}
				JoinSquad(name("MPSquad_" $ SquadIdx), AI, true);
				GearPRI(AI.PlayerReplicationInfo).SquadName = AI.Squad.SquadName;
				// have secondary squads start by going for some superweapons
				AI.bForceWeaponRush = (SquadIdx != 0);
				SquadIdx++;
				RemainingMembers.Remove(i, 1);
			}
			else
			{
				i++;
				if (i >= RemainingMembers.length && !bSecondPass)
				{
					bSecondPass = true;
					i = 0;
				}
			}
		}
	}

	// now fill the created squads with any remaining bots
	SquadIdx = 0;
	// bias the squad filling so squads other than the leader get any "extra" bots
	// this is optimal for our only leader gametype (Kill the Leader/Guardian)
	// where we want the majority to be trying to kill the opposition leader
	// although if we supported other leader gametypes, this would have to be generalized a bit
	if (NumSquads > 1)
	{
		for (i = 0; i < Squads.length; i++)
		{
			if (Squads[i].SquadName == 'MPSquad_0')
			{
				if (Squads[i].Leader != None && GearPRI(Squads[i].Leader.PlayerReplicationInfo).bIsLeader)
				{
					SquadIdx++;
				}
				break;
			}
		}
	}
	while (RemainingMembers.length > 0)
	{
		AI = GearAI_TDM(RemainingMembers[0]);
		RemainingMembers.Remove(0, 1);
		if (AI != None)
		{
			if (AI.Squad != None)
			{
				AI.Squad.UnregisterSquadMember(AI);
			}
			JoinSquad(name("MPSquad_" $ SquadIdx), AI, false);
			GearPRI(AI.PlayerReplicationInfo).SquadName = AI.Squad.SquadName;
			// have secondary squads start by going for some superweapons
			AI.bForceWeaponRush = (SquadIdx != 0);
			SquadIdx = (SquadIdx + 1) % NumSquads;
		}
	}

	// for all MP squads, set the flag that allows the squads to communicate stimuli to each other
	for (i = 0; i < Squads.length; i++)
	{
		if (Left(string(Squads[i].SquadName), 7) ~= "MPSquad")
		{
			Squads[i].bInterSquadCommunication = true;
		}
	}
}

/** @return the number of team members that are still alive */
final function int GetLiveMembers()
{
	local int i, Num;

	for (i = 0; i < TeamMembers.length; i++)
	{
		if (TeamMembers[i] != None && TeamMembers[i].Pawn != None && !TeamMembers[i].IsDead())
		{
			Num++;
		}
	}

	return Num;
}

defaultproperties
{
}
