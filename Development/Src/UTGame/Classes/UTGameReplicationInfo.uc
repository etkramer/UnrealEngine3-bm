/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTGameReplicationInfo extends GameReplicationInfo
	config(Game)
	native;

var float WeaponBerserk;
var int MinNetPlayers;
var int BotDifficulty;		// for bPlayersVsBots

var bool		bWarmupRound;	// Amount of Warmup Time Remaining
/** forces other players to be viewed on this machine with the default character */
var globalconfig bool bForceDefaultCharacter;

/** We hold a reference to the live scoreboard to adapt for split screen */
var UTUIScene_Scoreboard ScoreboardScene;

var string SinglePlayerBotNames[4];

enum EFlagState
{
    FLAG_Home,
    FLAG_HeldFriendly,
    FLAG_HeldEnemy,
    FLAG_Down,
};

var EFlagState FlagState[2];

/** If this is set, the game is running in story mode */
var bool bStoryMode;

/** Holds the Mission index of the current mission */
var int SinglePlayerMissionID;

/** whether the server is a console so we need to make adjustments to sync up */
var bool bConsoleServer;

/** Which input types are allowed for this game **/
var bool bAllowKeyboardAndMouse;

/** set by level Kismet to disable announcements during tutorials/cinematics/etc */
var bool bAnnouncementsDisabled;

/** Whether or not heroes are allowed in the game */
var bool bHeroesAllowed;

var repnotify bool bShowMOTD;

var databinding string MutatorList;
var databinding string RulesString;

//********** Map Voting **********8/

var int MapVoteTimeRemaining;

/** weapon overlays that are available in this map - figured out in PostBeginPlay() from UTPowerupPickupFactories in the level
 * each entry in the array represents a bit in UTPawn's WeaponOverlayFlags property
 * @see UTWeapon::SetWeaponOverlayFlags() for how this is used
 */
var array<MaterialInterface> WeaponOverlays;
/** vehicle weapon effects available in this map - works exactly like WeaponOverlays, except these are meshes
 * that get attached to the vehicle mesh when the corresponding bit is set on the driver's WeaponOverlayFlags
 */
struct native MeshEffect
{
	/** mesh for the effect */
	var StaticMesh Mesh;
	/** material for the effect */
	var MaterialInterface Material;
};
var array<MeshEffect> VehicleWeaponEffects;


//===================================================================
/*	These are client-side variables that hold references to the mid game menu.
    We store them here so that split-screen doesn't double up						*/
//===================================================================

/** Holds the current Mid Game Menu Scene */
var UTUIScene_MidGameMenu CurrentMidGameMenu;
var name LastUsedMidgameTab;

var bool bShowMenuOnDeath;

var bool bRequireReady;

replication
{
	if (bNetInitial)
		WeaponBerserk, MinNetPlayers, BotDifficulty, bStoryMode, bConsoleServer, bShowMOTD, MutatorList, RulesString, bRequireReady, bHeroesAllowed;

	if (bNetDirty)
		bWarmupRound, FlagState, MapVoteTimeRemaining, bAnnouncementsDisabled, bAllowKeyboardAndMouse;
}

simulated function PostBeginPlay()
{
	local UTPowerupPickupFactory Powerup;
	local Sequence GameSequence;
	local array<SequenceObject> AllFactoryActions;
	local SeqAct_ActorFactory FactoryAction;
	local UTActorFactoryPickup Factory;
	local int i;
	local UTGameUISceneClient SC;
	local UIScene NextScene;

	Super.PostBeginPlay();

	// using DynamicActors here so the overlays don't break if the LD didn't build paths
	foreach DynamicActors(class'UTPowerupPickupFactory', Powerup)
	{
		Powerup.AddWeaponOverlay(self);
	}

	// also check if any Kismet actor factories spawn powerups
	GameSequence = WorldInfo.GetGameSequence();
	if (GameSequence != None)
	{
		GameSequence.FindSeqObjectsByClass(class'SeqAct_ActorFactory', true, AllFactoryActions);
		for (i = 0; i < AllFactoryActions.length; i++)
		{
			FactoryAction = SeqAct_ActorFactory(AllFactoryActions[i]);
			Factory = UTActorFactoryPickup(FactoryAction.Factory);
			if (Factory != None && ClassIsChildOf(Factory.InventoryClass, class'UTInventory'))
			{
				class<UTInventory>(Factory.InventoryClass).static.AddWeaponOverlay(self);
			}
		}
	}

	// Look for a mid game menu and if it's there fix it up
	SC = UTGameUISceneClient(class'UIRoot'.static.GetSceneClient());
	if (SC != none)
	{
		CurrentMidGameMenu = UTUIScene_MidGameMenu(SC.FindSceneByTag('MidGameMenu'));
		if ( CurrentMidGameMenu != none )
		{
			CurrentMidGameMenu.Reset();
		}

		// also close any scoreboards that are up
		i = 0;
		while ( i < SC.GetActiveSceneCount() )
		{
			NextScene = SC.GetSceneAtIndex(i);
			if ( UTUIScene_Scoreboard(NextScene) == None || !NextScene.CloseScene(NextScene, false, true))
			{
				i++;
			}
		}
	}

	if (WorldInfo.NetMode != NM_DedicatedServer)
	{
		SetTimer(1.0, false, 'CharacterProcessingComplete');
	}
}

//Signal that all player controllers character processing is complete
simulated function CharacterProcessingComplete()
{
	local UTPlayerController UTPC;

	foreach LocalPlayerControllers(class'UTPlayerController', UTPC)
	{
		UTPC.CharacterProcessingComplete();
	}
}

simulated function ReplicatedEvent(name VarName)
{
	if ( VarName == 'bShowMOTD' )
	{
		DisplayMOTD();
	}
}

function SetFlagHome(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_Home;
	bForceNetUpdate = TRUE;
}

simulated function bool FlagIsHome(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_Home );
}

simulated function bool FlagsAreHome()
{
	return ( FlagState[0] == FLAG_Home && FlagState[1] == FLAG_Home );
}

function SetFlagHeldFriendly(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_HeldFriendly;
}

simulated function bool FlagIsHeldFriendly(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_HeldFriendly );
}

function SetFlagHeldEnemy(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_HeldEnemy;
}

simulated function bool FlagIsHeldEnemy(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_HeldEnemy );
}

function SetFlagDown(int TeamIndex)
{
	FlagState[TeamIndex] = FLAG_Down;
}

simulated function bool FlagIsDown(int TeamIndex)
{
	return ( FlagState[TeamIndex] == FLAG_Down );
}

simulated function Timer()
{
	local byte TimerMessageIndex;
	local PlayerController PC;

	super.Timer();

	if ( WorldInfo.NetMode == NM_Client )
	{
		if ( bWarmupRound && RemainingTime > 0 )
			RemainingTime--;
	}

    if (WorldInfo.NetMode != NM_DedicatedServer && MapVoteTimeRemaining > 0)
    {
    	MapVoteTimeRemaining--;
    }

	// check if we should broadcast a time countdown message
	if (WorldInfo.NetMode != NM_DedicatedServer && (bMatchHasBegun || bWarmupRound) && !bStopCountDown && !bMatchIsOver && Winner == None)
	{
		switch (RemainingTime)
		{
			case 300:
				TimerMessageIndex = 16;
				break;
			case 180:
				TimerMessageIndex = 15;
				break;
			case 120:
				TimerMessageIndex = 14;
				break;
			case 60:
				TimerMessageIndex = 13;
				break;
			case 30:
				TimerMessageIndex = 12;
				break;
			default:
				if (RemainingTime <= 10 && RemainingTime > 0)
				{
					TimerMessageIndex = RemainingTime;
				}
				break;
		}
		if (TimerMessageIndex != 0)
		{
			foreach LocalPlayerControllers(class'PlayerController', PC)
			{
				PC.ReceiveLocalizedMessage(class'UTTimerMessage', TimerMessageIndex);
			}
		}
	}
}

/**
 * Displays the message of the day by finding a hud and passing off the call.
 */
simulated function DisplayMOTD()
{
	local PlayerController PC;
	local UTPlayerController UTPC;

	return;

	ForEach LocalPlayerControllers(class'PlayerController', PC)
	{
		UTPC = UTPlayerController(PC);
		if ( UTPC != none )
		{
			UTHud(UTPC.MyHud).DisplayMOTD();
		}

		break;
	}
}

simulated function PopulateMidGameMenu(UTSimpleMenu Menu)
{
	if ( CanChangeTeam() )
	{
		Menu.AddItem("<Strings:UTGameUI.MidGameMenu.ChangeTeam>",0);
	}

	Menu.AddItem("<Strings:UTGameUI.MidGameMenu.Settings>",1);

	if ( WorldInfo.NetMode == NM_Client )
	{
		Menu.AddItem("<Strings:UTGameUI.MidGameMenu.Reconnect>",2);
	}

	Menu.AddItem("<Strings:UTGameUI.MidGameMenu.LeaveGame>",3);
}

/** Whether a player can change teams or not.  Used by menus and such. */
simulated function bool CanChangeTeam()
{
	if (UTGame(WorldInfo.Game) != None && UTGame(WorldInfo.Game).IsConsoleDedicatedServer())
	{
		return false;
	}
	else
	{
		return (GameClass.default.bTeamGame && !bStoryMode);
	}
}

simulated function bool MidMenuMenu(UTPlayerController UTPC, UTSimpleList List, int Index)
{
	//local UTUIScene S;
	switch ( List.List[Index].Tag)
	{
		case 0:
			UTPC.ChangeTeam();
			return true;
			break;

		case 1:
			/*
			S = UTUIScene( List.GetScene() );
			S.CloseScene(,,true);
			UTPC.OpenUIScene(class'UTUIFrontEnd_MainMenu'.default.SettingsScene);
			*/
			break;

		case 2:
			UTPC.ConsoleCommand("Reconnect");
			break;

		case 3:
			UTPC.QuitToMainMenu();
			return true;
			break;
	}


	return false;
}

/** @return whether the given team is Necris (used by e.g. vehicle factories to control whether they can activate for this team) */
simulated function bool IsNecrisTeam(byte TeamNum);


/**
 * In a single player campaign, this function will be called to insure all of
 * the players are properly assigned to a given team.  It insure that the host
 * of the game is assigned to Reaper, and then assigns the other players in order.
 *
 *@Param	PRI		The PRI of the player to Assign a character for
 */


function AssignSinglePlayerCharacters(UTPlayerReplicationInfo PRI)
{
	local int i, CharIndex;
	local UTPlayerController PC;
	local UTPlayerReplicationInfo Chars[4], OtherPRI;
	local CharacterInfo CharInfo;
	local UTBot B;

    // We only do this in story mode

	if ( !bStoryMode )
	{
		return;
	}

	PC = UTPlayerController(PRI.Owner);
	if (PC != None && !PRI.bOnlySpectator)
	{
		if ( PRI.SinglePlayerCharacterIndex >= 0 )
		{
			// We already have an assigned player.  Validate it.
			if (PRI.SinglePlayerCharacterIndex == 0) 	// Reaper
			{
				if ( PC.Player != none && LocalPlayer(PC.Player) != None )
				{
					Chars[0] = PRI;
				}
				else
				{
					// We are not the local host, or there is already local
					// Reaper, so reset this player.
					PRI.SinglePlayerCharacterIndex = -1;
				}
			}
		}

		// figure out who we already have
		for (i = 0; i < PRIArray.length; i++)
		{
			OtherPRI = UTPlayerReplicationInfo(PRIArray[i]);
			if (OtherPRI != None && OtherPRI.SinglePlayerCharacterIndex >= 0)
			{
				if ( OtherPRI != PRI && OtherPRI.SinglePlayerCharacterIndex != PRI.SinglePlayerCharacterIndex &&
					PRI.SinglePlayerCharacterIndex != 0 )
				{
					PRI.SinglePlayerCharacterIndex = -1;
				}
				Chars[OtherPRI.SinglePlayerCharacterIndex] = OtherPRI;
			}
		}

		// At this point, a player is either seeded correctly
		// or waiting for a seed (Index <0).  If needed, seed him now

		if ( PRI.SinglePlayerCharacterIndex < 0 )
		{
			for (i=0;i<4;i++)
			{
				if ( Chars[i] == none )
				{
					PRI.SinglePlayerCharacterIndex = i;
					Chars[i] = PRI;
					break;
				}
			}

		}

		// One final check.  If we get here and the player
		// hasn't been seeded, there are too many players in the game.
		// If this occurs, set them to spectators for now.  The game
		// will attempt to reseed them next map.


		if ( PRI.SinglePlayerCharacterIndex < 0 )
		{
			`log("Server has found an additional Player.  Setting to Spectator!");
			UTGame(WorldInfo.Game).BecomeSpectator(PC);
			return;
		}

		// Set the player's Character
		CharIndex = class'UTCharInfo'.default.Characters.Find('CharName', SinglePlayerBotNames[PRI.SinglePlayerCharacterIndex]);
		CharInfo  = class'UTCharInfo'.default.Characters[CharIndex];
		PRI.CharClassInfo = class'UTCharInfo'.static.FindFamilyInfo(CharInfo.FamilyID);

		// kick the bot playing this character, if it's present
		foreach WorldInfo.AllControllers(class'UTBot', B)
		{
			if (B.PlayerReplicationInfo.PlayerName ~= SinglePlayerBotNames[PRI.SinglePlayerCharacterIndex])
			{
				B.Destroy();
			}
		}
	}
}

/**
 * Open the mid-game menu
 */
simulated function UTUIScene_MidGameMenu ShowMidGameMenu(UTPlayerController InstigatorPC, optional name TabTag,optional bool bEnableInput)
{
	local UIScene Scene;
	local UTUIScene Template;
	local class<UTGame> UTGameClass;

	if (TabTag == '')
	{
		if (LastUsedMidgameTab != '')
		{
			TabTag = LastUsedMidGameTab;
		}
	}

	if ( CurrentMidGameMenu != none )
	{
		if ( TabTag != '' && TabTag != 'ChatTab' )
		{
//			CurrentMidGameMenu.ActivateTab(TabTag);
		}
		return CurrentMidGameMenu;
	}

	if ( ScoreboardScene != none )	// Force the scoreboards to close
	{
		ShowScores(false, none, none );
	}


	UTGameClass = class<UTGame>(GameClass);
	if (UTGameClass == none)
	{
		return None;
	}

	Template = UTGameClass.Default.MidGameMenuTemplate;

	if ( Template != none )
	{
		Scene = OpenUIScene(InstigatorPC,Template);
		if ( Scene != none )
		{
			CurrentMidGameMenu = UTUIScene_MidGameMenu(Scene);
			ToggleViewingMap(true);

			if (bMatchIsOver)
			{
				CurrentMidGameMenu.TabControl.RemoveTabByTag('SettingsTab');
			}

			if ( TabTag != '' )
			{
				CurrentMidGameMenu.ActivateTab(TabTag);
			}
		}
		else
		{
			`log("ERROR - Could not open the mid-game menu:"@Template);
		}
	}

	if ( CurrentMidGameMenu != none && bEnableInput)
	{
		CurrentMidGameMenu.SetSceneInputMode(INPUTMODE_Free);
	}

	return CurrentMidGameMenu;
}

/**
 * Clean up
 */
function simulated MidGameMenuClosed( )
{
	ToggleViewingMap(false);
	CurrentMidGameMenu = none;
}

function ToggleViewingMap(bool bIsViewing)
{
	local UTPlayerController PC;

	foreach WorldInfo.AllControllers(class'UTPlayerController',PC)
	{
		if ( LocalPlayer(PC.Player) != none )
		{
			PC.ServerViewingMap(bIsViewing);
		}
	}
}

/** wrapper for opening UI scenes
 * @param InstigatorPC - player to open it for
 * @param Template - the scene to open
 */
simulated function UIScene OpenUIScene(UTPlayerController InstigatorPC, UIScene Template)
{
	local LocalPlayer LP;
	local UIScene s;

	// Check all replication conditions
	LP = LocalPlayer(InstigatorPC.Player);
	if ( LP != None )
	{
		S = Template.OpenScene(Template, LP);
	}

	return S;
}

simulated function ShowScores(bool bShow, UTPlayerController Host, UTUIScene_Scoreboard Template)
{
	local UIScene Scene;

	// Regardless of what's going on, if the mid game menu is up, don't ever show scores
	if ( CurrentMidGameMenu != none )
	{
		bShow = false;
	}

	if ( bShow )
	{
		if (ScoreboardScene == none )
		{
			Scene = OpenUIScene(Host, Template);
			ScoreboardScene = UTUIScene_Scoreboard(Scene);
			ScoreboardScene.Host = Host;
			SetHudShowScores(true);
		}
	}
	else
	{
		if (ScoreboardScene != none && (Host == none || ScoreboardScene.Host == Host) )
		{
			ScoreboardScene.Host = none;
			ScoreboardScene.CloseScene();
			ScoreboardScene = none;
			SetHudShowScores(false);
		}
	}
}

simulated function SetHudShowScores(bool bShow)
{
	local UTPlayerController PC;
	foreach WorldInfo.AllControllers(class'UTPlayerController',PC)
	{
		if ( PC.MyHUD != none )
		{
			PC.MyHud.bShowScores = bShow;
		}
	}
}

function AddGameRule(string Rule)
{
	RulesString $= ((RulesString != "") ? "\n" : "")$Rule;
}

defaultproperties
{
	WeaponBerserk=+1.0
	BotDifficulty=-1
	FlagState[0]=FLAG_Home
	FlagState[1]=FLAG_Home
	TickGroup=TG_PreAsyncWork

	SinglePlayerBotNames(0)="Reaper"
	SinglePlayerBotNames(1)="Othello"
	SinglePlayerBotNames(2)="Bishop"
	SinglePlayerBotNames(3)="Jester"
	bShowMenuOnDeath=false
	bHeroesAllowed=false
}
