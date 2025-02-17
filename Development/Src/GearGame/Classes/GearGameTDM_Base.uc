/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearGameTDM_Base extends GearGameMP_Base
	native
	abstract
	config(Game);


/**
 * Set the game rules via the online game settings
 * @return - whether the game was able to set the rules via the gamesettings object
 */
function bool SetGameRules()
{
	local int IntValue;
	local GearVersusGameSettings MyGameSettings;

	MyGameSettings = GetGameSettingsObject();

	if ( MyGameSettings != None )
	{
		// See if this is Execution
		if ( MyGameSettings.GetStringSettingValue(MyGameSettings.const.CONTEXT_VERSUSMODES, IntValue) )
		{
			if ( IntValue == eGEARMP_Execution )
			{
				SetExecutionRules();
				bExecutionRules = true;
				// Change leaderboards, so that this writes to execution and not warzone
				OnlineStatsWriteClass=class'GearLeaderboardWriteExecution';
			}
		}

		return Super.SetGameRules();
	}

	return FALSE;
}

/** Any initialization that needs to happen after the level is loaded but before the game actually begins. */
event InitGame( string Options, out string ErrorMessage )
{
	Super.InitGame(Options,ErrorMessage);

	// Round Duration
	if ( HasOption(Options, "RoundDuration") )
	{
		RoundDuration = Clamp( GetIntOption(Options, "RoundDuration", RoundDuration), 0, 3600 );
	}

	// Time limit
	if ( HasOption(Options, "TimeLimit") )
	{
		TimeLimit = Max( 0, GetIntOption(Options, "TimeLimit", TimeLimit) );
	}
}

/**
 * Overridden to clear any left-over pawns still in the world before restarting players.
 */
function StartMatch()
{
	// Again, make sure all the teams are seeded
	//AutoSeedTeams();

	// start the match
	Super.StartMatch();
}

/**
* Call the Multiplayer version of adding inventory.
* @see RestartPlayer
* @see AddDefaultInventory
* @see SetPlayerDefaults
**/
function AddDefaultInventory( Pawn P )
{
	// for DM we are going to randomize the set of weapons a pawn gets
	GearPawn(P).AddDefaultInventoryTDM();

	// don't allow them to respawn
	P.Controller.PlayerReplicationInfo.bOutOfLives = TRUE;
}

function SetAICombatMoodFor(GearAI AI)
{
	local int i, j, AITeamIndex;
	local array<float> SurvivorCount;
	local ECombatMood NewCombatMood;

	NewCombatMood = AICM_Aggressive;
	if (!bIsInSuddenDeath)
	{
		// set aggressive if this AI seriously outnumbers the enemy
		SurvivorCount.length = Teams.length;
		for (i = 0; i < Teams.length; i++)
		{
			for (j = 0; j < Teams[i].TeamMembers.length; j++)
			{
				if (PlayerIsAlive(Teams[i].TeamMembers[j], None))
				{
					SurvivorCount[i] += 1.0;
				}
			}
		}


		AITeamIndex = AI.GetTeamNum();
		for (i = 0; i < Teams.length; i++)
		{
			if (i != AITeamIndex && SurvivorCount[i] > 0 && SurvivorCount[AITeamIndex] / SurvivorCount[i] < 1.66)
			{
				NewCombatMood = AICM_Normal;
				break;
			}
		}
	}

	if (AI.CombatMood != NewCombatMood)
	{
		AI.SetCombatMood(NewCombatMood);
	}
}


defaultproperties
{
	OnlineStatsWriteClass=class'GearLeaderboardWarzoneWrite'
}
