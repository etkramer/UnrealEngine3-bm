/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearUISceneMP_Base extends GearUIScene_Base
	abstract
	native(UI)
	Config(inherit);


/**
 * Callback function called when the scene is activated
 * Will see if it needs to make itself visible or not
 */
function OnSceneActivatedCallback( UIScene ActivatedScene, bool bInitialActivation )
{
	local int SceneStackLocation;
	local GameUISceneClient GameSceneClient;
	local UIScene CurrScene, CurrMPScene;

	GameSceneClient = GetSceneClient();

	if ( GameSceneClient != None )
	{
		// Hide this scene if it's the initial activation of the scene but a higher priority scene is in front of it
		if ( bInitialActivation )
		{
			SceneStackLocation = GameSceneClient.FindSceneIndex(Self);
			if ( SceneStackLocation != INDEX_NONE )
			{
				foreach GameSceneClient.AllActiveScenes(class'UIScene', CurrScene, false, SceneStackLocation + 1, class'UISceneClient'.const.SCENEFILTER_ReceivesFocus)
				{
					if ( CurrScene.SceneStackPriority > SceneStackPriority &&
						 CurrScene.SceneStackPriority < GEAR_SCENE_PRIORITY_BLOCKASYNC )
					{
						SetVisibility(false);
						break;
					}
				}
			}
		}
		// Show all the MP scenes if this is NOT the initial activation
		else
		{
			// Find all the MP scenes and show them
			foreach GameSceneClient.AllActiveScenes(class, CurrMPScene)
			{
				CurrMPScene.SetVisibility(true);
			}
		}
	}
}

/**
 * Callback function called when another scene is made the top scene
 * Will see if it needs to make itself invisible or not
 */
function OnTopSceneChangedCallback( UIScene NewTopScene )
{
	local GameUISceneClient GameSceneClient;
	local GearUISceneMP_Base CurrScene;

	GameSceneClient = GetSceneClient();
	if ( GameSceneClient != None )
	{
		// Find all the MP scenes and hide them if the new topmost scene is of higher priority
		foreach GameSceneClient.AllActiveScenes(class'GearUISceneMP_Base', CurrScene,,, class'UISceneClient'.const.SCENEFILTER_ReceivesFocus)
		{
			if ( CurrScene.SceneStackPriority < NewTopScene.SceneStackPriority &&
				 NewTopScene.SceneStackPriority < GEAR_SCENE_PRIORITY_BLOCKASYNC )
			{
				CurrScene.SetVisibility(false);
			}
		}
	}
}

/** Finds the first valid PRI of a team indexed by TeamIdx */
final function GearPRI FindPRIOnTeam( int TeamIdx, GearGRI MyGRI )
{
	local int PRIIdx;

	if (MyGRI != None)
	{
		for ( PRIIdx = 0; PRIIdx < MyGRI.PRIArray.length; PRIIdx++ )
		{
			if ( (MyGRI.PRIArray[PRIIdx] != None) && (MyGRI.PRIArray[PRIIdx].GetTeamNum() == TeamIdx) )
			{
				return GearPRI(MyGRI.PRIArray[PRIIdx]);
			}
		}
	}

	return None;
}

/** Returns the race of the PRI, with the option to use TeamInfo as a fallback */
final function EGearRaceTypes GetPRIRace( GearPRI PlayerPRI, optional GearTeamInfo TeamOfPRI )
{
	// Use PRI to find the race of the character
	if ( PlayerPRI != None )
	{
		return GetRaceUsingPRIOnly( PlayerPRI );
	}
	// PRI was null see if we can use the TeamInfo
	else if ( TeamOfPRI != None )
	{
		return ((TeamOfPRI.TeamIndex == 1) ? eGEARRACE_Locust : eGEARRACE_COG);
	}

	return eGEARRACE_COG;
}

/** Returns the race of the PRI */
static final function EGearRaceTypes GetRaceUsingPRIOnly( GearPRI PlayerPRI )
{
	local string ClassNameString;

	// Use PRI to find the race of the character
	if ( PlayerPRI != None )
	{
		ClassNameString = string(PlayerPRI.PawnClass);

		if ( InStr(ClassNameString, "COG") != -1 )
		{
			return eGEARRACE_COG;
		}
		else if ( InStr(ClassNameString, "Locust") != -1 )
		{
			return eGEARRACE_Locust;
		}
	}

	return eGEARRACE_COG;
}


/** Whether this PRI is valid or not */
final function bool IsValidPRI( GearPRI PlayerPRI )
{
	if ( PlayerPRI != None && !PlayerPRI.bIsInactive )
	{
		return true;
	}

	return false;
}

/** Whether this PRI is on a team or not */
final function bool IsPRIOnValidTeam( GearPRI PlayerPRI )
{
	if ( (PlayerPRI.Team != None) && (PlayerPRI.Team.TeamIndex >= 0 && PlayerPRI.Team.TeamIndex < 10) )
	{
		return true;
	}

	return false;
}

/** Whether this player is dead or not */
final function bool PlayerIsDead( GearGRI MyGRI, GearPRI PlayerPRI )
{
	if ( PlayerPRI != None )
	{
		if ( ((MyGRI.GameStatus == GS_RoundInProgress) && ((PlayerPRI.PlayerStatus == WPS_Dead) || (PlayerPRI.PlayerStatus == WPS_Respawning))) ||
			((MyGRI.GameStatus > GS_RoundInProgress) && ((PlayerPRI.EndOfRoundPlayerStatus == WPS_Dead) || (PlayerPRI.EndOfRoundPlayerStatus == WPS_Respawning))) )
		{
			return true;
		}
	}

	return false;
}

/** Whether this player is DBNO or not */
final function bool PlayerIsDBNO( GearGRI MyGRI, GearPRI PlayerPRI )
{
	if ( (PlayerPRI != None) && (MyGRI.GameStatus == GS_RoundInProgress) && (PlayerPRI.PlayerStatus == WPS_Down) )
	{
		return true;
	}

	return false;
}

/** Sets the local player PRIs - (1) = First player (2) = Splitscreen player */
final function GetLocalPRIs( out GearPRI LocalPRI1, out GearPRI LocalPRI2 )
{
	local GearPC PC;

	if (PlayerOwner != None && PlayerOwner.Actor != None)
	{
		foreach PlayerOwner.Actor.LocalPlayerControllers(class'GearPC', PC)
		{
			if (LocalPRI1 == None)
			{
				LocalPRI1 = GearPRI(PC.PlayerReplicationInfo);
			}
			else
			{
				LocalPRI2 = GearPRI(PC.PlayerReplicationInfo);
				break;
			}
		}
	}
}

/** Figures out the team name of the team this player is on - uses the PawnClass */
final function string GetTeamName( GearPRI PlayerPRI, optional GearGRI GGRI, optional int TeamIndex = -1 )
{
	local int Idx;

	if ( PlayerPRI != None )
	{
		return PlayerPRI.PawnClass.default.CharacterName;
	}
	else if ( GGRI != None && TeamIndex != -1 )
	{
		for ( Idx = 0; Idx < GGRI.PRIArray.length; Idx++ )
		{
			if ( GGRI.PRIArray[Idx].GetTeamNum() == TeamIndex )
			{
				return GearPRI(GGRI.PRIArray[Idx]).PawnClass.default.CharacterName;
			}
		}
	}

	return "";
}

/** Takes an int and returns a string in the form HH:MM:SS */
final function string FormatTime( int TimeToFormat )
{
	local int Minutes, Hours, Seconds;
	local string Result;

	// Figure out how many seconds we are dealing with first, we'll format it later
	Seconds = TimeToFormat;

	// Calculate hours and start building the string if there are hours
	if( Seconds > 3600 )
	{
		Hours = Seconds / 3600;
		Seconds -= Hours * 3600;
		Result = string(Hours)$":";
	}

	// Calculate minutes
	Minutes = Seconds / 60;

	// Calculate seconds
	Seconds -= Minutes * 60;

	// Build the string for minutes
	if ( Minutes < 10 )
	{
		if ( Hours == 0 )
			Result = Result$"0";
		else
			Result = Result$"0";
	}
	Result = Result$Minutes$":";

	// Build the string for seconds
	if ( Seconds < 10 )
	{
		Result = Result$"0";
	}
	Result = Result$Seconds;

	return Result;
}

DefaultProperties
{
	SceneStackPriority=GEAR_SCENE_PRIORITY_MPSCENE
	OnSceneActivated=OnSceneActivatedCallback
	OnTopSceneChanged=OnTopSceneChangedCallback
}

