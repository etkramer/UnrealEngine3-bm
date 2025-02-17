//=============================================================================
// Mutator.
//
// Mutators allow modifications to gameplay while keeping the game rules intact.
// Mutators are given the opportunity to modify player login parameters with
// ModifyLogin(), to modify player pawn properties with ModifyPlayer(), or to modify, remove,
// or replace all other actors when they are spawned with CheckRelevance(), which
// is called from the PreBeginPlay() function of all actors except those (Decals,
// Effects and Projectiles for performance reasons) which have bGameRelevant==true.
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class Mutator extends Info
	native
	abstract;

var Mutator NextMutator;
/** list of groups this mutator is in. Mutators that share any group cannot be activated simultaneously */
var() array<string> GroupNames;
var bool bUserAdded;

/* Don't call Actor PreBeginPlay() for Mutator
*/
event PreBeginPlay()
{
	if ( !MutatorIsAllowed() )
		Destroy();
}

function bool MutatorIsAllowed()
{
	return !WorldInfo.IsDemoBuild();
}

event Destroyed()
{
	WorldInfo.Game.RemoveMutator(self);
	Super.Destroyed();
}

function Mutate(string MutateString, PlayerController Sender)
{
	if ( NextMutator != None )
		NextMutator.Mutate(MutateString, Sender);
}

function ModifyLogin(out string Portal, out string Options)
{
	if ( NextMutator != None )
		NextMutator.ModifyLogin(Portal, Options);
}

/* called by GameInfo.RestartPlayer()
	change the players jumpz, etc. here
*/
function ModifyPlayer(Pawn Other)
{
	if ( NextMutator != None )
		NextMutator.ModifyPlayer(Other);
}

function AddMutator(Mutator M)
{
	if ( NextMutator == None )
		NextMutator = M;
	else
		NextMutator.AddMutator(M);
}

/* Force game to always keep this actor, even if other mutators want to get rid of it
*/
function bool AlwaysKeep(Actor Other)
{
	if ( NextMutator != None )
		return ( NextMutator.AlwaysKeep(Other) );
	return false;
}

function bool IsRelevant(Actor Other)
{
	local bool bResult;

	bResult = CheckReplacement(Other);
	if ( bResult && (NextMutator != None) )
		bResult = NextMutator.IsRelevant(Other);

	return bResult;
}

function bool CheckRelevance(Actor Other)
{
	local bool bResult;

	if ( AlwaysKeep(Other) )
		return true;

	// allow mutators to remove actors

	bResult = IsRelevant(Other);

	return bResult;
}

/**
 * Returns true to keep this actor
 */

function bool CheckReplacement(Actor Other)
{
	return true;
}

//
// server querying
//
function GetServerDetails( out GameInfo.ServerResponseLine ServerState )
{
	// append the mutator name.
	local int i;
	i = ServerState.ServerInfo.Length;
	ServerState.ServerInfo.Length = i+1;
	ServerState.ServerInfo[i].Key = "Mutator";
	ServerState.ServerInfo[i].Value = GetHumanReadableName();
}

function GetServerPlayers( out GameInfo.ServerResponseLine ServerState )
{
}

// jmw - Allow mod authors to hook in to the %X var parsing

function string ParseChatPercVar(Controller Who, string Cmd)
{
	if (NextMutator !=None)
		Cmd = NextMutator.ParseChatPercVar(Who,Cmd);

	return Cmd;
}

function NotifyLogout(Controller Exiting)
{
	if ( NextMutator != None )
		NextMutator.NotifyLogout(Exiting);
}

function NotifyLogin(Controller NewPlayer)
{
	if ( NextMutator != None )
		NextMutator.NotifyLogin(NewPlayer);
}

function DriverEnteredVehicle(Vehicle V, Pawn P)
{
	if ( NextMutator != None )
		NextMutator.DriverEnteredVehicle(V, P);
}

function bool CanLeaveVehicle(Vehicle V, Pawn P)
{
	if ( NextMutator != None )
		return NextMutator.CanLeaveVehicle(V, P);
	return true;
}

function DriverLeftVehicle(Vehicle V, Pawn P)
{
	if ( NextMutator != None )
		NextMutator.DriverLeftVehicle(V, P);
}

/**
 * This function can be used to parse the command line parameters when a server
 * starts up
 */

function InitMutator(string Options, out string ErrorMessage)
{
	if (NextMutator != None)
	{
		NextMutator.InitMutator(Options, ErrorMessage);
	}
}

/** called on the server during seamless level transitions to get the list of Actors that should be moved into the new level
 * PlayerControllers, Role < ROLE_Authority Actors, and any non-Actors that are inside an Actor that is in the list
 * (i.e. Object.Outer == Actor in the list)
 * are all automatically moved regardless of whether they're included here
 * only dynamic (!bStatic and !bNoDelete) actors in the PersistentLevel may be moved (this includes all actors spawned during gameplay)
 * this is called for both parts of the transition because actors might change while in the middle (e.g. players might join or leave the game)
 * @param bToEntry true if we are going from old level -> entry, false if we are going from entry -> new level
 * @param ActorList (out) list of actors to maintain
 */
function GetSeamlessTravelActorList(bool bToEntry, out array<Actor> ActorList)
{
	// by default, keep ourselves around until we switch to the new level
	if (bToEntry)
	{
		ActorList[ActorList.length] = self;
	}

	if (NextMutator != None)
	{
		NextMutator.GetSeamlessTravelActorList(bToEntry, ActorList);
	}
}

defaultproperties
{
}

