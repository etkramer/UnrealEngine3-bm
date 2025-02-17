/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGRI extends GameReplicationInfo
	dependson(GearHUD_Base)
	native
	nativereplication;

enum EGameStatus
{
	GS_None,
	GS_WaitingForHost,
	GS_PreMatch,
	GS_RoundInProgress,
	GS_RoundOver,
	GS_EndMatch,
	GS_Loading,
};

/** Timer Notification types for MP */
enum ETimerNotification
{
	TN_60Secs,
	TN_45Secs,
	TN_30Secs,
	TN_Count,
};

/** The states the Timer notifications can be */
enum ETimerNotificationState
{
	TNS_NotStarted,
	TNS_IsActive,
	TNS_IsDone,
};

var repnotify EGameStatus GameStatus;

var repnotify bool bOverTime;

/** How long before the game to wait for players entering.  If this is a listen server, this value is ignored and
    the host starts the match */
var int AutoStartDelay;

/** How long between when the host is ready and when the game starts */
var int HostStartDelay;

/** How long to wait after a round before the next one begins */
var int EndOfRoundDelay;

/** Delay before transitioning to next game */
var int EndOfGameDelay;

/** Round Timing/Count information */

var int RoundTime;						// How much more time in this round
var int RoundCount;						// How many rounds have been played
var int RoundDuration;					// How long is each round
var int RoundEndTime;					// Amount of time the last complete round took

/** This is true if the server has enough players to start the match */
var bool bHasEnoughPlayers;

var localized array<string> GameStatusText;

/** This is our global gear object pool that will hold all of our pools of objects **/
var GearObjectPool GOP;

/** Ref to the omnipresent companion to the HOD weapon */
var HOD_BeamManagerBase HODBeamManager;

/** Gamewide manager object for speech/dialogue */
var GearSpeechManager	SpeechManager;

/** Is this a coop game? */
var bool bIsCoop;

/** This is true if this server is a dedicated listen server */
var bool bIsDedicatedListenServer;

/** How much time does a dead character have to be revived */
var int InitialRevivalTime;

/** player who got the final kill of the round */
var PlayerReplicationInfo FinalKiller;
/** player who died to end the round */
var PlayerReplicationInfo FinalVictim;

/** Enum of the countdown state */
enum ECountdownState
{
	eCDS_None,
	eCDS_Active,
	eCDS_Stopped,
	eCDS_Expired
};
/** The state of the countdown timer */
var ECountdownState ECDState;
/** Countdown for all players to display */
var repnotify float EndTimeForCountdown;

/** Array of HeadTrackTargets we maintain for efficiency.  Storing in GRI so clients can have them too (so we don't need to replicate headtrack state in GearPawn). */
var() array<HeadTrackTarget>	HeadTrackTargetCache;
/** We update 1 target per frame, this is the index of the next one to update. */
var transient int				NextHeadTrackTargetToUpdate;

/** Array of aim-assist actors we maintain for efficiency. */
var() array<GearAimAssistActor>		AimAssistActorCache;

/** Final kill by Friendly Fire (in Assassination) */
var bool bFinalFriendlyKill;
/** Need to replciate this out as we have various simulated projectiles that need to know this @see proj_arrow**/
var bool bAllowFriendlyFire;

/** For MP games: The names of the last player alive on each team, needed for end of round screen */
var String LastStandingOnTeamName[5];
/** For MP games: The order in which a team lost all of its' players */
var int TeamIndexLossOrder[5];

/**
 * List of bools that determine if the MP timer has displayed itself for the various times
 * False = has not notified the timer yet
 * Does not need replicated since this is only maintained on the server.
 */
var array<ETimerNotificationState> MPTimerNotifcationFlags;

/** Total number of waves in the Invasion gametype */
var int InvasionNumTotalWaves;
/** The current wave the Invasion gametype is on */
var int InvasionCurrentWaveIndex;
/** percentage of wave points that are still alive (not spawned or not killed) - compressed to byte */
var byte WavePointsAlivePct;
/** enemy difficulty (Horde only) */
var class<DifficultySettings> EnemyDifficulty;
/** (Horde only) the number of times we've wrapped around into more difficult "extended" rounds */
var repnotify int ExtendedRestartCount;

/** Total number of enemies that still need to be killed in the current round(wave).  Used in Invasion but could work in any gametype */
var repnotify byte EnemiesLeftThisRound;
/** Number of seconds to tick before the next round starts - currently only used by Invasion */
var int NumSecondsUntilNextRound;

/** Number of respawns a team has remaining */
var int NumTeamRespawns[5];
/** Total Number of respawns for each team */
var int TotalNumTeamRespawns;

/**
 * TeamScore used by particular GameTypes as intermediate scores,
 * allows continued use of TeamInfo.Score for round counting.
 */
var float GameScore[5];

/** Team index for the Command Point in WarGameAnnex - NOTE: -1 means no one controls the CP */
var int CPControlTeam;
/** The current Command Point in WarGameAnnex */
var GearWeaponPickupFactory CommandPoint;
/** Current CP control percentage */
var int	CPControlPct;
/** Current amount of CP resource points left */
var int CPResourceLeft;

/** The time of when respawns may happen */
var repnotify int RespawnTime;
/** The time interval that respawns will occur */
var int RespawnTimeInterval;

/** Whether an MP gametype being played is using execution rules or not (needed for displaying correct information in EndOrRound screen */
var bool bGameIsExecutionRules;
/** Whether the gametype being played is a KOTH version of Annex or not */
var bool bAnnexIsKOTHRules;
/** The resource goal for Annex/KOTH (for use in displaying in the scoreboard) */
var int AnnexResourceGoal;

/** The player to capture the Meatflag in CTM */
var GearPRI MeatflagKidnapper;
/** The reference to the GearPawn of the Meatflag in CTM */
var GearPawn MeatflagPawn;

/** Whether a round should have a time limit or not */
var bool bInfiniteRoundDuration;

/** PRI of the player to kill the COG leader */
var GearPRI KillerOfCOGLeaderPRI;
/** PRI of the player to kill the Locust leader */
var GearPRI KillerOfLocustLeaderPRI;

/** Remember last Death animation played. This is to prevent playing the same death animation twice consecutively. */
var transient byte	LastDeathAnimHighFwdIndex, LastDeathAnimHighBwdIndex, LastDeathAnimStdFwdIndex, LastDeathAnimStdBwdIndex, LastDeathAnimStdLtIndex, LastDeathAnimStdRtIndex;

/** list of smoke grenade fog volumes to specifically check against for PC red crosshair checks, since there's no magic combination of flags
 * that will make them only block that specific trace without engine hackery
 */
var array<GearFogVolume_Spawnable> SmokeVolumes;

/** Test bools for the LOD settings**/
var bool bAggressiveLOD_Test;
var bool bDropDetail_Test;

/** The playlist id last used */
var repnotify int PlaylistId;

/** The stats match ID */
var GUID StatsGameplaySessionID;

/**
 * Indexes of the character classes randomly generated by the GRI for wingman
 * NOTE: 0,2,4 = COG classes and are the values in ECogMPCharacter
 *       1,3 = Locust classes and are the values in ELocustMPCharacter
 */
var int WingmanClassIndexes[5];

/** default val is used in gearpawn to controll damage text rendering. */
var config bool bDebugShowDamage;


enum EWeatherType
{
	WeatherType_Clear,
	WeatherType_Rain,
	WeatherType_Hail,
};

struct native ReplicatedWeatherData
{
	var EWeatherType	WeatherType;
	var bool			bOverrideEmitterHeight;
	var float			EmitterHeight;
};

/** Replicated variable to broadcast desired weather state. */
var protected transient repnotify ReplicatedWeatherData		ReplicatedWeather;
/** Current weather state on this user. */
var protected transient EWeatherType						CurrentWeather;

/** Spectator camera action for the level.  There should be 2, one for each potential local player. */
var transient SeqAct_SpectatorCameraPath	CurrentLevelSpectatorCameraPaths[2];

var Name DefaultSpectatingState;

/** Training Grounds scenario ID (if this is -1 we are not in training) */
var int TrainingGroundsID;

/** Last played time of infantry idle break animations */
var	Array<FLOAT>	InfantryIdleBreakLastPlayTime;

cpptext
{
	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

replication
{
	if (Role == ROLE_Authority)
		HODBeamManager,GameStatus, RoundCount, bIsCoop, RoundTime, bHasEnoughPlayers, FinalKiller, FinalVictim,
		EndTimeForCountdown, LastStandingOnTeamName, TeamIndexLossOrder,
		bFinalFriendlyKill, bAllowFriendlyFire, InvasionCurrentWaveIndex, EnemiesLeftThisRound,
		TotalNumTeamRespawns, NumTeamRespawns, GameScore, CPControlTeam, CommandPoint, CPControlPct, CPResourceLeft,
		RespawnTime, bInfiniteRoundDuration, RoundEndTime, NumSecondsUntilNextRound, MeatflagKidnapper, MeatflagPawn,
		ReplicatedWeather, KillerOfCOGLeaderPRI, KillerOfLocustLeaderPRI, DefaultSpectatingState,
		ExtendedRestartCount, PlaylistId, WingmanClassIndexes, WavePointsAlivePct, bOverTime, StatsGameplaySessionID;

	if (bNetInitial && (Role == ROLE_Authority))
		AutoStartDelay, RoundDuration, EndOfRoundDelay, EndOfGameDelay, InitialRevivalTime, bGameIsExecutionRules,
		bAnnexIsKOTHRules, InvasionNumTotalWaves, EnemyDifficulty, AnnexResourceGoal;
}

function SetGameStatus(EGameStatus NewGameStatus)
{
	local GearPC PC;

	GameStatus = NewGameStatus;

	switch (GameStatus)
	{
		case GS_WaitingForHost:
			ResetRoundClock(AutoStartDelay);
			break;

		case GS_PreMatch:
			foreach LocalPlayerControllers(class'GearPC', PC)
			{
				PC.TransitionToSpectate();
			}
			ResetRoundClock( HostStartDelay );
			CleanupForNewRound();
			break;

		case GS_RoundOver:
			ResetRoundClock( EndOfRoundDelay );
			break;

		case GS_RoundInProgress:
			CleanupForNewRound();
			RoundEndTime = 0;
			ResetRoundClock( RoundDuration );
			break;

		case GS_EndMatch:
			ResetRoundClock( EndOfGameDelay );
			break;
	}

	NotifyHUDOfGameStatusChange();
}

//simulated protected native function CreateBattleCamPathForSecondPlayer();

simulated function SeqAct_SpectatorCameraPath GetBattleCamPath(int ControllerId)
{
	local SeqAct_SpectatorCameraPath BC;

	if ( (ControllerId == 0) || (ControllerId == 1) )
	{
		BC = CurrentLevelSpectatorCameraPaths[ControllerId];
		if (BC == None)
		{
			CacheBattleCams();
			BC = CurrentLevelSpectatorCameraPaths[ControllerId];
		}
		return BC;
	}

	return None;
}

simulated private function CacheBattleCams()
{
	local Sequence GameSequence;
	local array<SequenceObject> Objs;
	local int Idx, NumFound;

	// look for the level camera paths
	GameSequence = WorldInfo.GetGameSequence();
	GameSequence.FindSeqObjectsByClass(class'SeqAct_SpectatorCameraPath',TRUE,Objs);
	for (Idx = 0; Idx < Objs.Length; Idx++)
	{
		CurrentLevelSpectatorCameraPaths[NumFound] = SeqAct_SpectatorCameraPath(Objs[Idx]);
		if (CurrentLevelSpectatorCameraPaths[NumFound] != None)
		{
			NumFound++;
			if (NumFound >= 2)
			{
				break;
			}
		}
	}
}

simulated event PostBeginPlay()
{
	Super.PostBeginPlay();

	CacheBattleCams();

	// create the object pool
	GOP = Spawn( class'GearObjectPool' );

	if ( Role == ROLE_Authority )
	{
		bInfiniteRoundDuration = GearGame(WorldInfo.Game).bInfiniteRoundDuration;
	}

	// both clients and servers get one of these
	SpeechManager = Spawn( class'GearSpeechManager' );
}

/** This is used to notify clients when they have received the GameClass so they can do things based off that. **/
simulated function ReceivedGameClass()
{
	Super.ReceivedGameClass();

	// so if our gametype is horde we want to do some performance settings
	if( ClassIsChildOf(GameClass, class'GearGameHorde_Base') == TRUE )
	{
		GameClass.static.DoGameSpecificPerformanceSettings( WorldInfo );
	}
}

/**
 * return true if multiplayer round has ended
 */
simulated function bool RoundEnded()
{
	return (GameStatus == GS_RoundOver) || (GameStatus == GS_EndMatch);
}

simulated event ReplicatedEvent( name VarName )
{
	local GearHUDInvasion_Base InvHUD;
	local GearPC PC;
	if (VarName == nameof(ExtendedRestartCount))
	{
		CheckHordeMessage();
	}
	if (VarName == 'bOverTime')
	{
		if (bOverTime)
		{
			foreach LocalPlayerControllers(class'GearPC',PC)
			{
				if (PC.MyGearHud != None)
				{
					PC.MyGearHud.DrawOverTime();
				}
			}
		}
	}
	// if the game status has changed
	else if (VarName == 'GameStatus')
	{
		// then notify player HUDs of the event
		NotifyHUDOfGameStatusChange();
		// clean up decals and such, unless we just got into the match as in that case there's nothing to destroy
		if ((GameStatus == GS_PreMatch || GameStatus == GS_RoundInProgress) && CreationTime != WorldInfo.TimeSeconds)
		{
			CleanupForNewRound();
		}
		if (GameStatus == GS_PreMatch)
		{
			foreach LocalPlayerControllers(class'GearPC', PC)
			{
				PC.TransitionToSpectate();
			}
		}
		/*
		// clean up end match to workaround a potential crash
		if (GameStatus == GS_EndMatch)
		{
			SetTimer(5.f,FALSE,nameof(CleanupForNewRound));
		}
		*/
	}
	else if (VarName == 'ReplicatedWeather')
	{
		SetCurrentWeather(ReplicatedWeather.WeatherType);
		if (ReplicatedWeather.bOverrideEmitterHeight)
		{
			SetWeatherEmitterHeight(ReplicatedWeather.EmitterHeight);
		}
	}
	else if (VarName == nameof(EnemiesLeftThisRound))
	{
		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			InvHUD = GearHUDInvasion_Base(PC.myHUD);
			if (InvHUD != None)
			{
				InvHUD.EnemyCountChanged();
			}
		}
	}
	else if (VarName == nameof(RespawnTime))
	{
		RespawnTimeUpdated();
	}
	else if (VarName == 'PlaylistId')
	{
		OnPlaylistIdChanged();
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

simulated function CheckHordeMessage()
{
	local GearPC PC;
	if (ExtendedRestartCount > 0)
	{
		foreach LocalPlayerControllers(class'GearPC',PC)
		{
			if (PC.MyGearHUD != None)
			{
				PC.MyGearHUD.AttemptHordeModOpen();
				// only open for one since it's fullscreen
				break;
			}
		}
	}
}

simulated function RespawnTimeUpdated()
{
	local GearPC PC;
	// play a beep for all players about to respawn
	if (RespawnTime <= 3)
	{
		foreach LocalPlayerControllers(class'GearPC', PC)
		{
			if (PC.bWaitingToRespawn)
			{
				PC.PlaySound(SoundCue'Interface_Audio.Interface.TaccomOpen01Cue',TRUE);
			}
		}
	}
}

/** Called when any change is made to the playlist id */
delegate OnPlaylistIdChanged();

simulated function CleanupForNewRound()
{
	local Actor A;
	local FracturedStaticMeshActor FracturedActor;

	// destroy all dead bodies
	foreach DynamicActors(class'Actor', A)
	{
		if ((A.IsA('GearPawn') && A.bTearOff) || A.IsA('Emit_SmokeGrenade') || A.IsA('GearFogVolume_SmokeGrenade'))
		{
			A.Destroy();
		}
	}

	// clean up any decals
	while (WorldInfo.MyDecalManager.ActiveDecals.length > 0)
	{
		WorldInfo.MyDecalManager.DecalFinished(WorldInfo.MyDecalManager.ActiveDecals[0].Decal);
		WorldInfo.MyDecalManager.ActiveDecals.Remove(0, 1);
	}

	GOP.CleanupForNewRound();

	foreach AllActors(class'FracturedStaticMeshActor', FracturedActor)
	{
		FracturedActor.ResetVisibility();
	}

	WorldInfo.MyFractureManager.ResetPoolVisibility();

	WorldInfo.ForceGarbageCollection();
}

/**
 * Notifies all local HUDs of game status change so they can display the appropriate info.
 */
simulated function NotifyHUDOfGameStatusChange()
{
	local GearPC PC;

	if (GameStatus == GS_RoundOver || GameStatus == GS_EndMatch)
	{
		foreach LocalPlayerControllers(class'GearPC',PC)
		{
			if (PC.MyGearHUD != None)
			{
				PC.MyGearHUD.SignalEndOfRoundOrMatch(self);
			}
		}
	}
	else if (GameStatus == GS_RoundInProgress)
	{
		foreach LocalPlayerControllers(class'GearPC',PC)
		{
			if (PC.MyGearHUD != None)
			{
				PC.MyGearHUD.SignalStartOfRoundOrMatch(self);
			}
		}
	}
}

/**
 * Resets the round clock.
 *
 * @Param StartingValue 	- The value to reset to.  The Clock always counts up, so for a countdown, use a negative number.
 *							  The widget that uses this value needs to be smart enough to abs the value
 */

function ResetRoundClock(int StartingValue)
{
	if ( bInfiniteRoundDuration && GameStatus == GS_RoundInProgress )
	{
		RoundTime = 0;
	}
	else
	{
		RoundTime = StartingValue;
	}

	if ( Role == ROLE_Authority && IsMultiPlayerGame() )
	{
		// wipe and reset the flags
		MPTimerNotifcationFlags.length = 0;
		MPTimerNotifcationFlags.length = TN_Count;
		StopCountdown(true);
	}
}

/**
 * Increment the various timers.
 */

simulated function Timer()
{
	if (Role == ROLE_Authority)
	{
		if ( bMatchHasBegun && (GameStatus == GS_RoundInProgress) )
		{
			RoundEndTime++;
		}

		if ( bInfiniteRoundDuration && (GameStatus == GS_RoundInProgress) )
		{
			if ( bMatchHasBegun )
			{
				RoundTime++;
			}
		}
		else if ( GameStatus == GS_PreMatch || GameStatus == GS_RoundInProgress || GameStatus == GS_RoundOver || GameStatus == GS_EndMatch )
		{
			RoundTime--;
			RoundTime = (RoundTime < 0) ? 0 : RoundTime;
		}
	}

	if ( bMatchHasBegun )
	{
		ElapsedTime++;
	}

	if ( CurrentGameData != None )
	{
		// give the current game data store a chance to update its state
		CurrentGameData.Timer();
	}

	SetTimer(WorldInfo.TimeDilation, true);
}


simulated function bool InOrder( PlayerReplicationInfo P1, PlayerReplicationInfo P2 )
{
	local GearPRI WP1, WP2;
	local float AggScore1, AggScore2;

	WP1 = GearPRI(P1);
	WP2 = GearPRI(P2);
	AggScore1= WP1.Score;
	AggScore2= WP2.Score;

	if( WP1.bOnlySpectator )
	{
		if( WP2.bOnlySpectator )
			return true;
		else
			return false;
	}
	else if ( WP2.bOnlySpectator )
		return true;

	if( AggScore1 < AggScore2 )
		return false;
	if( AggScore1 == AggScore2 )
	{
		if ( WP1.Deaths > WP2.Deaths )
			return false;
		if ( (WP1.Deaths == WP2.Deaths) && (PlayerController(WP2.Owner) != None) && (LocalPlayer(PlayerController(WP2.Owner).Player) != None) )
			return false;
	}
	return true;
}


final simulated delegate bool IsWithinMaxEffectDistance( const out PlayerController PC, const out vector EffectLocation, const float CullDistance )
{
	`log( "IsWithinMaxEffectDistance Delegate was not set" );
	ScriptTrace();
	return FALSE;
}


final simulated function bool CheckEffectDistance_SpawnBehindIfNear( const out PlayerController PC, const out vector EffectLocation, const float CullDistance )
{
	local Vector Loc;
	local Rotator Rot;
	local float Dist;

	// grab camera location/rotation for checking Dist
	PC.GetPlayerViewPoint( Loc, Rot );
	Dist = VSize(EffectLocation - Loc);

	//`log( " Behind us?" @ ( (Normal(EffectLocation- Loc) dot vector(Rot)) < 0.0f ) @ (Dist < 1600.0f));
	//	ScriptTrace();
	if( (Normal(EffectLocation - Loc) dot vector(Rot)) < 0.0f )
	{
		return (Dist < 1600.0f);
	}

	//`log( (Dist * PC.LODDistanceFactor) @ CullDistance );
	// if the effect is is outside our CullDistance
	if( (Dist * PC.LODDistanceFactor) > CullDistance )
	{
		return FALSE;
	}

	return TRUE; // this used to be !BeyondFogDistance() which thunks and then just returns always returns false
}


final simulated function bool CheckEffectDistance_SpawnWithinCullDistanceAndInFront( const out PlayerController PC, const out vector EffectLocation, const float CullDistance )
{
	local Vector Loc;
	local Rotator Rot;
	local float Dist;

	// grab camera location/rotation for checking Dist
	PC.GetPlayerViewPoint( Loc, Rot );

	//`log( " Behind us?" @ ( (Normal(EffectLocation - Loc) dot vector(Rot)) < 0.0f ) );
	//	ScriptTrace();
	if( (Normal(EffectLocation - Loc) dot vector(Rot)) < 0.0f )
	{
		return FALSE;
	}

	Dist = VSize(EffectLocation - Loc);

	//`log( (Dist * PC.LODDistanceFactor) @ CullDistance );
	// if the effect is is outside our CullDistance
	if( (Dist * PC.LODDistanceFactor) > CullDistance )
	{
		return FALSE;
	}

	return TRUE; // this used to be !BeyondFogDistance() which thunks and then just returns always returns false
}


final simulated function bool CheckEffectDistance_SpawnWithinCullDistance( const out PlayerController PC, const out vector EffectLocation, const float CullDistance )
{
	local Vector Loc;
	local Rotator Rot;
	local float Dist;

	// grab camera location/rotation for checking Dist
	PC.GetPlayerViewPoint( Loc, Rot );

	Dist = VSize(EffectLocation - Loc);

	//`log( " " @ (Dist * PC.LODDistanceFactor) @ CullDistance );
	// if the effect is is outside our CullDistance
	if( (Dist * PC.LODDistanceFactor) > CullDistance )
	{
		return FALSE;
	}

	return TRUE; // this used to be !BeyondFogDistance() which thunks and then just returns always returns false
}



/**
 * This is our first line of defense for spawning things we don't "need" to spawn.
 *
 * We check to see if the Effect is going to be relevant to use based on:
 *  -Who created the effect (e.g. locally controlled humans the effect should always be "relevant"
 *  -Effect Distance to locally controlled humans
 *
 *
 **/
final simulated function bool IsEffectRelevant( const out Pawn InInstigator, const out vector EffectLocation, const float CullDistance, const bool bForceRelevant, delegate<IsWithinMaxEffectDistance> DistanceFunc )
{
	//local bool Retval;
	local PlayerController PC;

	// if we were told to always be relevant
	if( bForceRelevant )
	{
		return TRUE;
	}

	// check right off the bat to see if the Framerate is low for this instigator and then just say this effect is not relevant
	// note: we probably want to just do bAggressive check here???  As the code for particles and such will use the cheaper version of the PS
// 	if( ShouldDisableEffectsDueToFramerate( InInstigator ) == TRUE )
// 	{
// 		`log( "ShouldDisableEffectsDueToFramerate was TRUE" @ "DropDetail:" @ WorldInfo.bDropDetail @ "AggressiveLOD:" @ WorldInfo.bAggressiveLOD @ "HumanControlled:" @ ((InInstigator.IsLocallyControlled() == TRUE) && (InInstigator.IsHumanControlled() == TRUE)) @ WorldInfo.DeltaSeconds @ 1/WorldInfo.DeltaSeconds );
// 		//ScriptTrace();
// 		return FALSE;
// 	}

	// these are are for dummy fire that are "always relevant" ??  (seems not always true if they are on other side of the map)
// 	if( Instigator == None )
// 	{
// 		return TRUE;
// 	}

	// 1) Additionally, we need to check for Effects that are just staying within a radius of the instigator's location (e.g. footstep effects)
	// (aka poor man's frustum check).  This instigator may never move and thus not have been rendered in a while.
	// We do not want them be relevant by default.
 	if( (InInstigator != None) && (VSize(EffectLocation - InInstigator.Location) <= 128.0f) && ( (WorldInfo.TimeSeconds - InInstigator.LastRenderTime) > 2.0f ) )
 	{
 		//`log( "Have not rendered and effect is close:" @ InInstigator @ VSize(EffectLocation - InInstigator.Location) );
 		return FALSE;
 	}


	// so if the one making this effect is a human and locally controlled (e.g. oneself or a splitscreen friend) then we want to it to be considered
	// relevant as we just did it and want to be able to see it
	if( (InInstigator != None) && (InInstigator.IsHumanControlled()) && (InInstigator.IsLocallyControlled()) )
	{
		return TRUE;
	}

	// 1) The LastRenderTime check then come in and saves the day for stationary effects.  Saying: if we have been rendered in the last second then
	// our effects are going to be relevant.  Don't even bother to do a distance check
    if( (InInstigator != None) && ( (WorldInfo.TimeSeconds - InInstigator.LastRenderTime) < 1.0f ) )
    {
        return TRUE;
    }


	// so if the instigator has been rendered in the last second then the effect they are causing is _probably_ going to be relevant
	// but we need to check to see if they are within the distance of "relevant" effects
	//
	// 0)) Dummy fire falls through to this.  Where we still want to make certain the effect is within our culldistances.

	IsWithinMaxEffectDistance = DistanceFunc;

	foreach LocalPlayerControllers( class'PlayerController', PC )
	{
		if( IsWithinMaxEffectDistance( PC, EffectLocation, CullDistance ) )
		{
			IsWithinMaxEffectDistance = None;
			return TRUE;
		}
	}
	IsWithinMaxEffectDistance = none;
	

	return FALSE;
}



//!!!!!!
//
//
//


/**
 * This function determines if we should disable effects due to framerate.
 * Currently this is:
 * used to turn off MuzzleFlashLight on all people if the FPS is super low
 * used to turn off the MuzzleFlashLight in low FPS areas on the AI / other non local humans
 **/
final simulated function bool ShouldDisableEffectsDueToFramerate( const out Pawn InInstigator )
{
	//`log( "ShouldDisableEffectsDueToFramerate" @ WorldInfo.bDropDetail @ (InInstigator == None) @ (InInstigator.IsLocallyControlled() == TRUE) @ (InInstigator.IsHumanControlled() == TRUE)) ;

	// the game is running super slow so start turning off everything to get it back up
	if( ( WorldInfo.bAggressiveLOD ) || bAggressiveLOD_Test )
	{
		//`log( "dropping detail bAggressiveLOD" );
		return TRUE;
	}

	// do not show effects if the effect is NOT from you (or SS friend) and the game is slow
	else if( (( WorldInfo.bDropDetail ) || bDropDetail_Test )
		&& ( (InInstigator == None) || (InInstigator.IsLocallyControlled() == FALSE) || (InInstigator.IsHumanControlled() == FALSE) )
		)
	{
		//`log( "dropping detail bDropDetail" @ WorldInfo.bDropDetail @ (InInstigator == None) @ (InInstigator.IsLocallyControlled() == FALSE) @ (InInstigator.IsHumanControlled() == FALSE)) ;
		return TRUE;
	}

	// show effects from all as the game is not that slowa
	return FALSE;
}


/**
 * This will determine which LOD to use based off the specific ParticleSystem passed in
 * and the distance to where that PS is being displayed.
 *
 * NOTE:  This is distance based LOD not perf based.  Perf and distance are orthogonal concepts.
 *        We use the _ForSlowPerf effects when perf is slow.
 **/
final simulated function int GetLODLevelToUse(ParticleSystem PS, vector EffectLocation)
{
	local int				Retval;
	local int				LODIdx;
	local PlayerController	PC;
	local Vector			POVLoc;
	local Rotator			POVRot;
	local float				DistanceToEffect, LODDistance;

	// No particle system, ignore
	if( PS == None )
	{
		return 0;
	}

	// Run this for all local player controllers.
	// If several are found (split screen?). Take the closest for highest LOD.
	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		PC.GetPlayerViewPoint(POVLoc, POVRot);

		DistanceToEffect = VSize(POVLoc - EffectLocation);

		//`log( PS @ "LODDistanceFactor: " $ PC.LODDistanceFactor $ " POVLoc: " $ POVLoc $ " DistanceToEffect: " $ DistanceToEffect );

		// Take closest
		if( LODDistance == 0.f || DistanceToEffect < LODDistance )
		{
			LODDistance = DistanceToEffect;
		}
	}

	// Find appropriate LOD based on distance
	Retval = PS.LODDistances.Length - 1;
	for( LODIdx = 1; LODIdx < PS.LODDistances.Length; ++LODIdx )
	{
		if( DistanceToEffect < PS.LODDistances[LODIdx] )
		{
			Retval = LODIdx-1;
			break;
		}
	}

	//`log( PS @ "GetLODLevelToUse" @ Retval );

	return Retval;
}


/** Used to determine whether this game is an MP game or not**/
simulated function bool IsMultiPlayerGame()
{
	return ClassIsChildOf(GameClass, class'GearGameMP_Base');
}

/**
 * Used to determine whether this game is a COOP MP game such as Invasion or Horde.
 * This is needed because in some logic we want these types of games to
 * perform like Single Player Campaign games, but at other times we want
 * the behavior of a Multiplayer game.
 * Example: we don't want AI going DBNO in Invasion, but we still want other
 * systems to treat this gametype like an MP game.
 */
simulated function bool IsCoopMultiplayerGame()
{
	return ( ClassIsChildOf(GameClass, class'GearGameHorde_Base') );
}

simulated function HeadTrackTarget GetNextHeadTrackTargetToUpdate()
{
	if (HeadTrackTargetCache.Length == 0)
	{
		return None;
	}

	if ( (NextHeadTrackTargetToUpdate >= HeadTrackTargetCache.Length) || (NextHeadTrackTargetToUpdate < 0) )
	{
		// back to beginning of cycle
		NextHeadTrackTargetToUpdate = 0;
	}

	return HeadTrackTargetCache[NextHeadTrackTargetToUpdate];
}

/**
* Notification that we have a new headtrackactor, add it to out list.
* Server and client both.
*/
simulated function RegisterHeadTrackTarget(HeadTrackTarget HTT)
{
	local int Idx;

	// clean up list here to make sure it doesn't run away
	for (Idx=0; Idx<HeadTrackTargetCache.Length; ++Idx)
	{
		if (HeadTrackTargetCache[Idx] == None)
		{
			HeadTrackTargetCache.Remove(Idx, 1);
			Idx--;
		}
	}

	// add our new one
	HeadTrackTargetCache[HeadTrackTargetCache.Length] = HTT;
}

/**
* Notification that we have a new aim assist actor, add it to out list.
* Server and client both.
*/
simulated function RegisterAimAssistActor(GearAimAssistActor GAAA)
{
	local int Idx;

	// clean up list here to make sure it doesn't run away
	for (Idx=0; Idx<AimAssistActorCache.Length; ++Idx)
	{
		if (AimAssistActorCache[Idx] == None)
		{
			AimAssistActorCache.Remove(Idx, 1);
			Idx--;
		}
	}

	// add our new one
	AimAssistActorCache[AimAssistActorCache.Length] = GAAA;
}


/** Start the countdown for all players */
event StartCountdown( float CountdownAmount )
{
	local GearPC PC;

	EndTimeForCountdown = WorldInfo.TimeSeconds + CountdownAmount;
	ECDState = eCDS_Active;

	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		if ( PC != None )
		{
			PC.StartCountdownTime(CountdownAmount);
		}
	}
}

/** Stop the countdown for all players */
event StopCountdown(optional bool bInstantClear, optional bool bIsExpired)
{
	local GearPC PC;

	EndTimeForCountdown = 0;
	ECDState = bIsExpired ? eCDS_Expired : eCDS_Stopped;

	foreach WorldInfo.AllControllers( class'GearPC', PC )
	{
		if ( PC != None )
		{
			PC.ClearCountdownTime(bInstantClear);
		}
	}
}

/** Update the countdown for all players */
function UpdateCountdown()
{
	if ( EndTimeForCountdown > 0 )
	{
		if ( EndTimeForCountdown < WorldInfo.TimeSeconds )
		{
			StopCountdown(false, true);
		}
	}
}

event KillPlayers(array<Controller> Players, bool bShouldGib)
{
	local Controller Player;
	local Pawn DeadPawn;
	local Vehicle Vehicle;
	foreach Players(Player)
	{
		// kick them out of their vehicle
		if (Player.IsA('PlayerController'))
		{
			Vehicle = Vehicle(Player.Pawn);
			if (Vehicle != None)
			{
				Vehicle.DriverLeave(TRUE);
			}
		}
		DeadPawn = Player.Pawn;
		if (DeadPawn != None)
		{
			DeadPawn.Died(Player,bShouldGib?class'GDT_ScriptedGib':class'GDT_ScriptedRagdoll',DeadPawn.Location + vect(0,0,32));
		}
		if (!Player.IsA('PlayerController'))
		{
			Player.Destroy();
		}
	}
}

function UpdateMPCountdown()
{
	// Start the 30 second notification
	if ( RoundTime <= 31 && (MPTimerNotifcationFlags[TN_30Secs] == TNS_NotStarted) )
	{
		StartCountdown( RoundTime );
		MPTimerNotifcationFlags[TN_30Secs] = TNS_IsActive;
	}
	// Start the 45 second notification
	else if ( RoundTime <= 46 && (MPTimerNotifcationFlags[TN_45Secs] == TNS_NotStarted) )
	{
		StartCountdown( RoundTime );
		MPTimerNotifcationFlags[TN_45Secs] = TNS_IsActive;
	}
	// Stop the 45 second notification
	else if ( RoundTime < 45 && (MPTimerNotifcationFlags[TN_45Secs] == TNS_IsActive) )
	{
		StopCountdown();
		MPTimerNotifcationFlags[TN_45Secs] = TNS_IsDone;
	}
	// Start the 60 second notification
	else if ( RoundTime <= 61 && (MPTimerNotifcationFlags[TN_60Secs] == TNS_NotStarted) )
	{
		StartCountdown( RoundTime );
		MPTimerNotifcationFlags[TN_60Secs] = TNS_IsActive;
	}
	// Stop the 60 second notification
	else if ( RoundTime < 60 && (MPTimerNotifcationFlags[TN_60Secs] == TNS_IsActive) )
	{
		StopCountdown();
		MPTimerNotifcationFlags[TN_60Secs] = TNS_IsDone;
	}
}

event Tick( float DeltaTime )
{
	if ( Role == ROLE_Authority )
	{
		// Check for turning the countdown on/off for round time running out
		if ( IsMultiPlayerGame() && (GameStatus == GS_RoundInProgress) && !bInfiniteRoundDuration )
		{
			if ( GearGame(WorldInfo.Game).TimeLimit > 0 )
			{
				UpdateMPCountdown();
			}
		}

		// Check on the countdown to death
		UpdateCountdown();
	}
}


/**
 * Welcome to the utter pain.  The GearPawn only has a class and not actual weapon
 * and we need to get a non default property from it that is based on the class and the team
 * so we have this monstrosity to get the values back from the worldinfo :-(
 **/
simulated function LinearColor GetWeaponEmisColorByClass( class<GearWeapon> WeapClass, int TeamNum );

/**
* Determines if gore should be Shown.  We need to look at all PlayerControllers as a gore sensitive person
* may be playing splitscreen.
**/
simulated function bool ShouldShowGore()
{
	local PlayerController			PC;
	local GearPRI					PRI;

	// Check gore settings for local pc
	foreach LocalPlayerControllers(class'PlayerController', PC)
	{
		PRI = GearPRI(PC.PlayerReplicationInfo);
		//`log( "ShouldShowGore: PC" @ PC @ "PRI: " $ PRI @ PRI.bShowGore  );
		if( PRI != None && !PRI.bShowGore )
		{
			// If any player controller on this machine should not show gore
			// NO GORE
			return FALSE;
		}
	}

	return TRUE;
}


simulated event SetCurrentWeather(EWeatherType NewWeather)
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC',PC)
	{
		//@fixme, do some sort of blending?
		PC.StopWeatherEffects(CurrentWeather);
		PC.StartWeatherEffects(NewWeather);
	}

	CurrentWeather = NewWeather;

	if (Role == ROLE_Authority)
	{
		// this will broadcast to all clients
		ReplicatedWeather.WeatherType = NewWeather;

		// must call SetWeatherEmitterHeight to override again.
		ReplicatedWeather.bOverrideEmitterHeight = FALSE;
	}
}



simulated event SetWeatherEmitterHeight(float Height)
{
	local GearPC PC;

	foreach LocalPlayerControllers(class'GearPC',PC)
	{
		PC.SetWeatherEmitterHeight(Height);
	}

	if (Role == ROLE_Authority)
	{
		// this will broadcast to all clients
		ReplicatedWeather.bOverrideEmitterHeight = TRUE;
		ReplicatedWeather.EmitterHeight = Height;
	}

}

/**
 * Called by the native survey code when the user has finished the survey question. The data
 * is submitted then via Vince
 */
event LogSurveyResult(string Question, string QuestionId, string Answer, int AnswerNumber, string Context)
{
	local GearGameplayStatsUploader Uploader;
	local OnlineSubsystem OnlineSub;

	// Ask for the interface by name and cast to our well known type
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();
	Uploader = GearGameplayStatsUploader(OnlineSub.GetNamedInterface('StatsUpload'));
	if (Uploader != None)
	{
		// Submit to Vince or our LSP
		Uploader.BeginLog();
		Uploader.BeginEvent("SurveyAnswer");
		Uploader.AddParamString("Question", Question);
		Uploader.AddParamString("QuestionId", QuestionId);
		Uploader.AddParamString("Answer", Answer);
		Uploader.AddParamInt("AnswerNumber", AnswerNumber);
		Uploader.AddParamString("Context", Context);
		Uploader.EndEvent();
		Uploader.EndLog();
	}
}

simulated function bool IsActuallyPlayingCoop()
{
	local int NumPlayers;

	if (bIsCoop && !class'WorldInfo'.static.IsMenuLevel())
	{
		if (WorldInfo.NetMode == NM_Client)
		{
			return TRUE;
		}
		else
		{
			NumPlayers = GearGame(WorldInfo.Game).GetHumanPlayerCount();
			if (NumPlayers > 1)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}

defaultproperties
{
	GameStatus=GS_None
	InitialRevivalTime=20
	DefaultSpectatingState=CustomSpectating
	TrainingGroundsID=-1
	EnemiesLeftThisRound=255
	WavePointsAlivePct=255

	// default wingman classes for PC testing
	WingmanClassIndexes[0]=CMPC_Marcus
	WingmanClassIndexes[1]=LMPC_HunterNoArmor
	WingmanClassIndexes[2]=CMPC_BenCarmine
	WingmanClassIndexes[3]=LMPC_Kantus
	WingmanClassIndexes[4]=CMPC_Dom
}
