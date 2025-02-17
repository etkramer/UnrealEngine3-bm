//=============================================================================
// GameRules.
//
// The GameRules class handles game rule modifications for the GameInfo such as scoring,
// finding player starts, and damage modification.
//
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
//=============================================================================
class GameRules extends Info;

var GameRules NextGameRules;

function AddGameRules(GameRules GR)
{
	if ( NextGameRules == None )
		NextGameRules = GR;
	else
		NextGameRules.AddGameRules(GR);
}

/* Override GameInfo FindPlayerStart() - called by GameInfo.FindPlayerStart()
if a NavigationPoint is returned, it will be used as the playerstart
*/
function NavigationPoint FindPlayerStart( Controller Player, optional byte InTeam, optional string incomingName )
{
	if ( NextGameRules != None )
		return NextGameRules.FindPlayerStart(Player,InTeam,incomingName);

	return None;
}

/* return string containing game rules information
*/
function string GetRules()
{
	local string ResultSet;

	if ( NextGameRules == None )
		ResultSet = ResultSet$NextGameRules.GetRules();

	return ResultSet;
}

//
// Restart the game.
//
function bool HandleRestartGame()
{
	if ( (NextGameRules != None) && NextGameRules.HandleRestartGame() )
		return true;
	return false;
}

/* CheckEndGame()
Allows modification of game ending conditions.  Return false to prevent game from ending
*/
function bool CheckEndGame(PlayerReplicationInfo Winner, string Reason)
{
	if ( NextGameRules != None )
		return NextGameRules.CheckEndGame(Winner,Reason);

	return true;
}

/** OverridePickupQuery()
 * when pawn wants to pickup something, gamerules given a chance to modify it.  If this function
 * returns true, bAllowPickup will determine if the object can be picked up.
 * @param Other the Pawn that wants the item
 * @param ItemClass the Inventory class the Pawn can pick up
 * @param Pickup the Actor containing that item (this may be a PickupFactory or it may be a DroppedPickup)
 * @param bAllowPickup (out) whether or not the Pickup actor should give its item to Other (0 == false, anything else == true)
 * @return whether or not to override the default behavior with the value of
 */
function bool OverridePickupQuery(Pawn Other, class<Inventory> ItemClass, Actor Pickup, out byte bAllowPickup)
{
	if (NextGameRules != None && NextGameRules.OverridePickupQuery(Other, ItemClass, Pickup, bAllowPickup))
	{
		return true;
	}
	return false;
}

function bool PreventDeath(Pawn Killed, Controller Killer, class<DamageType> damageType, vector HitLocation)
{
	if ( (NextGameRules != None) && NextGameRules.PreventDeath(Killed,Killer, damageType,HitLocation) )
		return true;
	return false;
}

function ScoreObjective(PlayerReplicationInfo Scorer, Int Score)
{
	if ( NextGameRules != None )
		NextGameRules.ScoreObjective(Scorer,Score);
}

function ScoreKill(Controller Killer, Controller Killed)
{
	if ( NextGameRules != None )
		NextGameRules.ScoreKill(Killer,Killed);
}

function NetDamage(int OriginalDamage, out int Damage, pawn Injured, Controller InstigatedBy, vector HitLocation, out vector Momentum, class<DamageType> DamageType)
{
	if (NextGameRules != None)
	{
		NextGameRules.NetDamage(OriginalDamage, Damage, Injured, InstigatedBy, HitLocation, Momentum, DamageType);
	}
}

defaultproperties
{
}
