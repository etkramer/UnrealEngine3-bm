/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearStatsObject extends OnlineGameplayEvents
	config(Game)
	native;

`define DECLARE_STATS(dummy)
`include ( GearGame\GearStats.uci )
`undefine(DECLARE_STATS)

/** This is the RunID that this Stats object is associated with.  It gets set in the BeginGameplaySession() **/
var int GearsStatsRunID;

/** This is an estimate of the current number of bytes allocated for stats**/
var int EstimatedStatBytes;

var config int AllowedStatsMask;

/** It returns.. this is the safest way, I promise.  It's true if we are in a horde match */
var bool bUglyHordeHackFlag;

var array<name> HordeIgnoreEvents;

cpptext
{
	void SummarizeGameplayEvents(TArray<FGameplayEvent> &Events, FArchive *Ar);
	INT ResolvePlayerIndex(AController *Player);
}

/** Mark a new session, clear existing events, etc */
final native function BeginGameplaySession();

/** Simple interface to log gameplay events */
final native function LogGameplayEvent(int EventMask, Name EventName, Controller Player, coerce string Desc, optional Controller TargetPlayer, optional vector EventLocation);

/** Write out the current session's gameplay events */
final native function EndGameplaySession();

/** This will send all of the GameEvents to the DB **/
final native function SendGameEventsToDB();

/** debug function to generate log files of stats **/
final native function DumpGameplayStats();

defaultproperties
{
	// set the 0 index as a NULL entry
	PlayerList(0)=(PlayerName="No Player",ControllerName="NULL")

	GearsStatsRunID=-1

	HordeIgnoreEvents(0)=PlayerKill
	HordeIgnoreEvents(1)=PlayerDeath
	HordeIgnoreEvents(2)=PlayerSpawn

}
