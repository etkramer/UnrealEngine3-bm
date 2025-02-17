/**
 * This class is needed for the engine to be able to load SP maps and have a native package class to be
 * used in
 * [Engine.GameInfo]
 * DefaultGame=GearGame.GearGameGeneric
 * DefaultServerGame=GearGame.GearGameGeneric
 *
 * Nothing should ever go in here it is just helper class for the engine.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameGeneric extends GameInfo
	native;


struct native GameTypePrefix
{
	/** map prefix, e.g. "DM" */
	var string Prefix;
	/** gametype used if none specified on the URL */
	var string GameType;
	/** additional gametypes supported by this map prefix via the URL (used for cooking) */
	var array<string> AdditionalGameTypes;
};

/** Used for loading appropriate game type if non-specified in URL */
var array<GameTypePrefix> DefaultMapPrefixes;
/** Used for loading appropriate game type if non-specified in URL */
var globalconfig array<GameTypePrefix> CustomMapPrefixes;




/** This is the gametype to fall back after everything else has failed. **/
var string SuperDuperFallBackGameType;

cpptext
{
	virtual void AddSupportedGameTypes(AWorldInfo* Info, const TCHAR* WorldFilename) const;
}


event PostBeginPlay()
{
	`warn( "This map did not have a game type associated with it.  You need to open it and resave it" );
	`warn( "This map did not have a game type associated with it.  You need to open it and resave it" );
	`warn( "This map did not have a game type associated with it.  You need to open it and resave it" );
	`warn( "This map did not have a game type associated with it.  You need to open it and resave it" );
	`warn( "This map did not have a game type associated with it.  You need to open it and resave it" );

	super.PostBeginPlay();
}

static event string GetDefaultGameClassPath(string MapName, string Options, string Portal)
{
	local string ThisMapPrefix;
	local int Idx;

	ThisMapPrefix = Left(MapName, InStr(MapName, "_"));
	Idx = class'GearGameGeneric'.default.DefaultMapPrefixes.Find('Prefix', ThisMapPrefix);
	if (Idx != INDEX_NONE)
	{
		return class'GearGameGeneric'.Default.DefaultMapPrefixes[Idx].GameType;
	}
	else
	{
		return Super.GetDefaultGameClassPath(MapName, Options, Portal);
	}
}

static event class<GameInfo> SetGameType(string MapName, string Options, string Portal)
{
	local string ThisMapPrefix;
	local int i,pos;
	local class<GameInfo> NewGameType;
	local string GameOption;

	// if we're in the menu level, use the menu gametype
	if ( class'WorldInfo'.static.IsMenuLevel(MapName) )
	{
		return class'GearMenuGame'.static.SetGameType(MapName, Options, Portal);
	}

	// allow commandline to override game type setting
	GameOption = ParseOption( Options, "Game" );
	if ( GameOption != "" )
	{
		return Default.class;
	}

	// strip the UEDPIE_ from the filename, if it exists (meaning this is a Play in Editor game)
	if ( Left(MapName, 6) ~= "UEDPIE" )
	{
		MapName = Right(MapName, Len(MapName) - 6);
		if ( Left(MapName, 3) ~= "MP_" )
		{
			NewGameType = class<GameInfo>(DynamicLoadObject( default.SuperDuperFallBackGameType,class'Class'));
			if ( NewGameType != None )
			{
				return NewGameType;
			}
			//return Super.SetGameType(MapName, Options, Portal);
		}
	}
	else if ( Left(MapName, 5) ~= "UEDPC" )
	{
		MapName = Right(MapName, Len(MapName) - 5);
	}
	else if ( Left(MapName, 6) ~= "UEDPS3" )
	{
		MapName = Right(MapName, Len(MapName) - 6);
	}
	else if ( Left(MapName, 6) ~= "UED360" )
	{
		MapName = Right(MapName, Len(MapName) - 6);
	}

	// replace self with appropriate gametype if no game specified
	pos = InStr( MapName, "_" );
	ThisMapPrefix = left(MapName,pos);
	for (i = 0; i < class'GearGame'.default.MapPrefixes.length; i++)
	{
		if (class'GearGame'.default.MapPrefixes[i] ~= ThisMapPrefix)
		{
			return Default.class;
		}
	}

	// change game type
	for ( i=0; i<class'GearGameGeneric'.Default.DefaultMapPrefixes.Length; i++ )
	{
		if ( class'GearGameGeneric'.Default.DefaultMapPrefixes[i].Prefix ~= ThisMapPrefix )
		{
			NewGameType = class<GameInfo>(DynamicLoadObject(class'GearGameGeneric'.Default.DefaultMapPrefixes[i].GameType,class'Class'));
			if ( NewGameType != None )
			{
				return NewGameType;
			}
		}
	}
	for ( i=0; i<class'GearGameGeneric'.Default.CustomMapPrefixes.Length; i++ )
	{
		if ( class'GearGameGeneric'.Default.CustomMapPrefixes[i].Prefix ~= ThisMapPrefix )
		{
			NewGameType = class<GameInfo>(DynamicLoadObject(class'GearGameGeneric'.Default.CustomMapPrefixes[i].GameType,class'Class'));
			if ( NewGameType != None )
			{
				return NewGameType;
			}
		}
	}


	NewGameType = class<GameInfo>(DynamicLoadObject( default.SuperDuperFallBackGameType,class'Class'));
	if ( NewGameType != None )
	{
		return NewGameType;
	}


	// if no gametype found, use DemoGame
	return Super.SetGameType( MapName, Options, Portal );
}


auto State PendingMatch
{
	function CheckStartMatch()
	{
		// start the match immediately
		StartMatch();
	}

Begin:
	CheckStartMatch();
	Sleep(1.f);
	Goto('Begin');
}


/**
 * Initializes the gameplay stats upload object
 */
event InitGame(string Options, out string ErrorMessage)
{
	Super.InitGame(Options, ErrorMessage);

	ParseAutomatedTestingOptions( Options );
}


/** This is our overridden StartMatch() function **/
function StartMatch()
{
	local GearPC PC;

	if( bDoingASentinelRun == TRUE )
	{
		`log( "DoingASentinelRun!" );

		// this will take over the normal match rules and do its own thing
		if( SentinelTaskDescription == "TravelTheWorld" )
		{
			DoTravelTheWorld();
			return;
		}
		// any of these types are going to run in addition to what ever the player is doing
		// they just go gather stats based on a timer
		else
		{
			BeginSentinelRun( SentinelTaskDescription, SentinelTaskParameter, SentinelTagDesc );
			// for now we are just hard coding what we want to gather
			foreach WorldInfo.AllControllers( class'GearPC', PC )
			{
				//PC.ConsoleCommand( "stat " );
			}

			SetTimer( 3.0f, TRUE, nameof(DoTimeBasedSentinelStatGathering) );
		}
	}

	`log("Starting match...");
	Super.StartMatch();

	// transition to the new in progress state
	GotoState('MatchInProgress');


	// BugIt functionality
	// need to delay here as in MP maps there is a delay in starting the match even with QuickStart on
	SetTimer( 5.0f, FALSE, nameof(CheckForBugItCommand) );


	if (WorldInfo.Game.bAutomatedTestingWithOpen == true)
	{
		WorldInfo.Game.IncrementNumberOfMatchesPlayed();
	}
	else
	{
		foreach WorldInfo.AllControllers(class'GearPC', PC)
		{
			PC.IncrementNumberOfMatchesPlayed();
			break;
		}
	}

	WorldInfo.Game.IncrementAutomatedTestingMapIndex();

	if( bCheckingForFragmentation == TRUE )
	{
		//ConsoleCommand( "killparticles" );
		ConsoleCommand( "MemFragCheck" );
	}

	if( AutomatedTestingExecCommandToRunAtStartMatch != "" )
	{
		`log( "AutomatedTestingExecCommandToRunAtStartMatch: " $ AutomatedTestingExecCommandToRunAtStartMatch );
		ConsoleCommand( AutomatedTestingExecCommandToRunAtStartMatch );
	}

}

/** This will check to see if there is a BugIt command active and then call the BugItGo function **/
function CheckForBugItCommand()
{
	local GearPC PC;

	foreach WorldInfo.AllControllers(class'GearPC', PC)
	{
		break;
	}

	if( BugLocString != "" || BugRotString != "" )
	{
		if( PC.CheatManager != none )
		{
			//`log( "BugLocString:" @ BugLocString );
			//`log( "BugRotString:" @ BugRotString );
			PC.BugItGoString( BugLocString, BugRotString );
		}
	}
}


state MatchInProgress
{
	function BeginState(Name PreviousStateName)
	{
		Super.BeginState(PreviousStateName);
	}

Begin:

	if( CauseEventCommand != "" )
	{
		// wait until all levels are streamed back in
		do
		{
			bSentinelStreamingLevelStillLoading = FALSE;

			for( SentinelIdx = 0; SentinelIdx < WorldInfo.StreamingLevels.length; ++SentinelIdx )
			{
				if( WorldInfo.StreamingLevels[SentinelIdx].bHasLoadRequestPending == TRUE )
				{
					`log( "levels not streamed in yet sleeping 1s" );
					bSentinelStreamingLevelStillLoading = TRUE;
					Sleep( 1.0f );
					break;
				}
			}

		} until( bSentinelStreamingLevelStillLoading == FALSE );

		// check to see if we should fire off the FlyThrough event again as preround starting usually stops the first event
		if( CauseEventCommand != "" )
		{
			foreach WorldInfo.AllControllers(class'PlayerController', SentinelPC)
			{
				`log( "CauseEventCommand" @ CauseEventCommand );
				SentinelPC.ConsoleCommand( "ce " $ CauseEventCommand );
				break;
			}
		}

		// wait 500 ms to let the switching camera Hitch work itself out
		if( ( SentinelTaskDescription == "FlyThrough" ) || ( SentinelTaskDescription == "FlyThroughSplitScreen" ) )
		{
			SetTimer( 0.500f, TRUE, nameof(DoTimeBasedSentinelStatGathering) );
		}
	}

}




defaultproperties
{
	PlayerReplicationInfoClass=class'GearGame.GearPRI'
	PlayerControllerClass=class'GearGame.GearPC'
	DefaultPawnClass=class'GearGame.GearPawn_COGMarcus'
	GameReplicationInfoClass=class'GearGame.GearGRI'

	SuperDuperFallBackGameType="GearGameContent.GearGameSP"

	DefaultMapPrefixes(0)=(Prefix="MP",GameType="GearGameContent.GearGameTDM", AdditionalGameTypes=("GearGameContent.GearGameKTL","GearGameContent.GearGameAnnex","GearGameContent.GearGameWingman","GearGameContent.GearGameCTM","GearGameContent.GearGameHorde"))
	DefaultMapPrefixes(1)=(Prefix="POC",GameType="GearGameContent.GearGameSP", AdditionalGameTypes=("GearGameContent.GearGameTDM","GearGameContent.GearGameKTL","GearGameContent.GearGameAnnex","GearGameContent.GearGameWingman","GearGameContent.GearGameCTM"))
	DefaultMapPrefixes(2)=(Prefix="SP",GameType="GearGameContent.GearGameSP")
	DefaultMapPrefixes(3)=(Prefix="DP",GameType="GearGameContent.GearGameSP", AdditionalGameTypes=("GearGameContent.GearGameTDM","GearGameContent.GearGameKTL","GearGameContent.GearGameAnnex","GearGameContent.GearGameWingman","GearGameContent.GearGameCTM"))
	DefaultMapPrefixes(4)=(Prefix="GearGame_P",GameType="GearGameContent.GearGameSP")
}
