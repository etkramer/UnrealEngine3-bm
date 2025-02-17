/**
 * Base class for scenes that show detailed stats for one or more players.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class GearUISceneFE_PlayerStatBase extends GearUISceneFE_PostGameBase;

`include(GearOnlineConstants.uci)

/************************************************************************/
/* Constants, enums, structs, etc.										*/
/************************************************************************/

/** Structure to store the data needed for mapping a stat to the stat system and to localized text */
struct GearStatData
{
	/** The property in the stat system for this stat */
	var const int StatId;
	/** The human readable name for the stat to display in UI */
	var const localized string StatName;
};

/** Struct to store the data needed for one stat */
struct GearStatWidgetData
{
	/** Reference to the stat description */
	var transient UILabel StatName;
	/** Reference to the lifetime version of the stat */
	var transient UILabel StatLife;
	/** Reference to the previous game version of the stat */
	var transient UILabel StatGame;
};

/** Struct to store all the data needed for the player */
struct GearStatPlayerData
{
	/** The image of the character the player used from the previous game */
	var transient UIImage CharacterImage;
	/** The label to display the player's name */
	var transient UILabel NameLabel;
	/** The label to display the player's skill */
	var transient UILabel RankLabel;
	/** The label to display the player's leaderboard rank */
	var transient UILabel LeaderRankLabel;
	/** List of all the stats to display */
	var transient array<GearStatWidgetData> StatsData;
	/** Image of the title background for the player */
	var transient UIImage NameBackgroundImage;
};


/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** Access to the online subsystem for grabbing stats data */
var transient OnlineSubsystem OnlineSub;

/** The network id of the player we are viewing the stats of */
var transient UniqueNetId PlayerNetworkId;

/** The game type that was played */
var transient EGearMPTypes GameType;

/** Reference to the button bar */
var transient UICalloutButtonPanel ButtonBar;

/** All the widget data needed for this player */
var transient GearStatPlayerData PlayerData;

/** Stat mappings for all game types */
var array<GearStatData> StatMappingsGeneric;
/** Stat mappings for Annex specific stats */
var array<GearStatData> StatMappingsAnnex;
/** Stat mappings for KOTH specific stats */
var array<GearStatData> StatMappingsKOTH;
/** Stat mappings for Meatflag specific stats */
var array<GearStatData> StatMappingsMeatflag;

/** the number of stats being displayed in this scene */
var	transient	const		int		NumStatGroups;


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

/**
 * Assigns all member variables to appropriate child widget from this scene.
 */
function InitializeWidgetReferences()
{
	// Buttonbar
	ButtonBar = UICalloutButtonPanel(FindChild('pnlButtonbar', true));

	// Character image
	PlayerData.CharacterImage = UIImage(FindChild('imgPlayerModel', true));

	// Player name
	PlayerData.NameLabel = UILabel(FindChild('lblName', true));

	// Player rank (skill)
	PlayerData.RankLabel = UILabel(FindChild('lblRank', true));

	// Player leaderboard rank
	PlayerData.LeaderRankLabel = UILabel(FindChild('lblLeaderboardRank', true));

	// Player name background
	PlayerData.NameBackgroundImage = UIImage(FindChild('imgTitleBar', true));
}

/**
 * Assigns delegates in important child widgets to functions in this scene class.
 */
function SetupCallbacks()
{
	ButtonBar.SetButtonCallback('ShowGamercard', OnGamercardPress);
	ButtonBar.SetButtonCallback('GenericBack', OnBackPress);
}

/** Initializes the widgets in the scene and sets their bindings */
final function InitializePlayerData()
{
	local UIPanel StatPanel;
	local int StatIdx;
	local String WidgetName;
	local String LabelString;
	local int PlayerIndex;
	local bool bHasValidNetId;
	local UniqueNetId ZeroId;
	local int SlotIndex;
	local name BackgroundStyleName;

	// Buttonbar
	PlayerIndex = GetBestPlayerIndex();
	bHasValidNetId = (PlayerNetworkId != ZeroId);
	ButtonBar.EnableButton('ShowGamercard', PlayerIndex, bHasValidNetId, false);

	// Character image
	SetCharacterImage(PlayerData.CharacterImage);

	// Player name
	LabelString = GetPlayerName();
	PlayerData.NameLabel.SetDataStoreBinding(LabelString);

	// Player rank (skill)
	LabelString = GetPlayerRank();
	PlayerData.RankLabel.SetDataStoreBinding(LabelString);

	// Player leaderboard rank
	LabelString = GetPlayerLeaderRank();
	PlayerData.LeaderRankLabel.SetDataStoreBinding(LabelString);

	// Name background
	if (IsPostGameStats())
	{
		BackgroundStyleName = GetPlayerBackgroundStyleName();
		PlayerData.NameBackgroundImage.SetWidgetStyleByName('Image Style', BackgroundStyleName);
	}

	// Parent panel of all the stats (used for finding the widgets quicker)
	StatPanel = UIPanel(FindChild('pnlStats', true));
	PlayerData.StatsData.length = NumStatGroups;
	for (SlotIndex = 0; SlotIndex < PlayerData.StatsData.length; SlotIndex++)
	{
		// Remove all "rounds" stats from Wingman
		if (SlotIndex < StatMappingsGeneric.Length &&
			StatMappingsGeneric[SlotIndex].StatId == STATS_COLUMN_ROUNDSPLAYED)
		{
			if (GameType == eGEARMP_Wingman)
			{
				// Stop this loop since we are done
				StatIdx = StatMappingsGeneric.Length;
			}
		}

		// Remove "rounds drawn" for annex/koth/meatflag
		if (SlotIndex < StatMappingsGeneric.Length &&
			StatMappingsGeneric[SlotIndex].StatId == STATS_COLUMN_ROUNDSDRAW)
		{
			if (GameType == eGEARMP_Annex || GameType == eGEARMP_KOTH || GameType == eGEARMP_CTM)
			{
				StatIdx++;
			}
		}

		// Stat name
		WidgetName = "lblStat" $ SlotIndex;
		PlayerData.StatsData[SlotIndex].StatName = UILabel(StatPanel.FindChild(Name(WidgetName), true));
		LabelString = GetStatNameString(StatIdx);
		PlayerData.StatsData[SlotIndex].StatName.SetDataStoreBinding(LabelString);

		// Stat lifetime
		WidgetName = "lblStat" $ SlotIndex $ "Value";
		PlayerData.StatsData[SlotIndex].StatLife = UILabel(StatPanel.FindChild(Name(WidgetName), true));
		LabelString = GetStatString(StatIdx, true);
		PlayerData.StatsData[SlotIndex].StatLife.SetDataStoreBinding(LabelString);

		// Stat of previous game
		WidgetName = "lblStat" $ SlotIndex $ "Diff";
		PlayerData.StatsData[SlotIndex].StatGame = UILabel(StatPanel.FindChild(Name(WidgetName), true));
		LabelString = GetStatString(StatIdx, false);
		PlayerData.StatsData[SlotIndex].StatGame.SetDataStoreBinding(LabelString);
		StatIdx++;
	}
}

/** Whether this is the post game stats screen or not */
function bool IsPostGameStats()
{
	return false;
}

/** Returns the background style name for the style to use for this player */
function name GetPlayerBackgroundStyleName()
{
	return '';
}

/** Sets the image component in the UIImage using the netId to find the portrait to draw */
function SetCharacterImage(UIImage ImageWidget)
{
}

/** Returns the string of the name of the player using the netId as the key */
function String GetPlayerName()
{
	return "RobRooster";
}

/** Returns the string of the rank of the player using the netId as the key */
function String GetPlayerRank()
{
	return "D";
}

/** Returns the string of the leaderboard rank of the player, using the netId as the key */
function String GetPlayerLeaderRank()
{
	return " ";
}

/** Returns the string of the stat for the player using the netId as the key */
function String GetPlayerStatValue(bool bIsLifetime,int StatId)
{
	return "0";
}

/**
 * Returns the localized string name of the stat at index StatIdx
 *
 * @param StatIdx - the index in the PlayerData.StatsData array
 */
function String GetStatNameString(int StatIdx)
{
	if (StatIdx > -1)
	{
		// Display a generic stat name
		if (StatIdx < StatMappingsGeneric.length)
		{
			return StatMappingsGeneric[StatIdx].StatName;
		}
		else
		{
			switch (GameType)
			{
			case eGEARMP_Annex:
				// Display an Annex stat name
				if (StatIdx < StatMappingsGeneric.length + StatMappingsAnnex.length)
				{
					return StatMappingsAnnex[StatIdx-StatMappingsGeneric.length].StatName;
				}
				break;
			case eGEARMP_KOTH:
				// Display a KOTH stat name
				if (StatIdx < StatMappingsGeneric.length + StatMappingsKOTH.length)
				{
					return StatMappingsKOTH[StatIdx-StatMappingsGeneric.length].StatName;
				}
				break;
			case eGEARMP_CTM:
				// Display a Meatflag stat name
				if (StatIdx < StatMappingsGeneric.length + StatMappingsMeatflag.length)
				{
					return StatMappingsMeatflag[StatIdx-StatMappingsGeneric.length].StatName;
				}
				break;
			}
		}
	}
	return " ";
}

/**
 * Returns the string of the stat at index StatIdx
 *
 * @param StatIdx - the index in the PlayerData.StatsData array
 * @param bIsLifetime - whether the stat is for the entire lifetime of playing this gametype or just the last time you played
 */
function String GetStatString(int StatIdx, bool bIsLifetime)
{
	local String ReturnString;
	local String NegativeString;

	if (StatIdx > -1)
	{
		// Display a generic stat
		if (StatIdx < StatMappingsGeneric.length)
		{
			ReturnString = GetPlayerStatValue(bIsLifetime,StatMappingsGeneric[StatIdx].StatId);;
		}
		else
		{
			switch (GameType)
			{
				case eGEARMP_Annex:
					if (StatIdx < StatMappingsGeneric.length + StatMappingsAnnex.length)
					{
						ReturnString = GetPlayerStatValue(bIsLifetime,StatMappingsAnnex[StatIdx - StatMappingsGeneric.Length].StatId);
					}
					break;
				case eGEARMP_KOTH:
					if (StatIdx < StatMappingsGeneric.length + StatMappingsKOTH.length)
					{
						ReturnString = GetPlayerStatValue(bIsLifetime,StatMappingsKOTH[StatIdx - StatMappingsGeneric.Length].StatId);
					}
					break;
				case eGEARMP_CTM:
					if (StatIdx < StatMappingsGeneric.length + StatMappingsMeatflag.length)
					{
						ReturnString = GetPlayerStatValue(bIsLifetime,StatMappingsMeatflag[StatIdx - StatMappingsGeneric.Length].StatId);
					}
					break;
			}
		}
	}

	// If a valid stat was found, format it
	if (ReturnString != "")
	{
		// If this is a lifetime stat, just return it
		if (bIsLifetime)
		{
			return ReturnString;
		}
		// If this is just a previous game stat, add the special characters
		else
		{
			NegativeString = "[";
			// Don't add the "+" if the number is negative
			if (!IsStringNegative(ReturnString))
			{
				NegativeString $= "+";
			}
			NegativeString $= ReturnString;
			NegativeString $= "]";
			return NegativeString;
		}
	}
	return " ";
}

/**
 * Checks a string to see if it has a '-' as the first character
 *
 * @param Source the source string to look at
 *
 * @return true if the string is negative, false otherwise
 */
function bool IsStringNegative(string Source)
{
	return Left(Source,1) == "-";
}

/**
 * Opens the gamecard screen for the currently focused player
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnGamercardPress(UIScreenObject EventObject, int PlayerIndex)
{
	local UniqueNetId ZeroId;
	local OnlinePlayerInterfaceEx PlayerIntEx;
	local int ControllerId;

	if (PlayerNetworkId != ZeroId)
	{
		if (OnlineSub != None)
		{
			PlayerIntEx = OnlineSub.PlayerInterfaceEx;
			if (PlayerIntEx != None)
			{
				ControllerId = class'UIInteraction'.static.GetPlayerControllerId(PlayerIndex);
				PlayerIntEx.ShowGamerCardUI(ControllerId, PlayerNetworkId);
			}
		}
	}
	return true;
}

/**
 * Called when the widget is no longer being pressed.  Not implemented by all widget types.
 *
 * The difference between this delegate and the OnPressRelease delegate is that OnClick will only be called on the
 * widget that received the matching key press. OnPressRelease will be called on whichever widget was under the cursor
 * when the key was released, which might not necessarily be the widget that received the key press.
 *
 * @param EventObject	Object that issued the event.
 * @param PlayerIndex	Player that performed the action that issued the event.
 *
 * @return	return TRUE to prevent the kismet OnClick event from firing.
 */
function bool OnBackPress(UIScreenObject EventObject, int PlayerIndex)
{
	CloseScene(self);
	return true;
}

/* === UIScreenObject interface === */
/**
 * Called after this screen object's children have been initialized.  While the Initialized event is only called when
 * a widget is initialized for the first time, PostInitialize() will be called every time this widget receives a call
 * to Initialize(), even if the widget was already initialized.  Examples would be reparenting a widget.
 */
event PostInitialize()
{
	OnlineSub = class'GameEngine'.static.GetOnlineSubsystem();

	InitializeWidgetReferences();

	Super.PostInitialize();

	SetupCallbacks();
}

/** Truncates a float string to one decimel place */
function string TruncateFloatString(string FloatString)
{
	local int Decimel;

	Decimel = InStr(FloatString,".");
	if (Decimel > -1)
	{
		Decimel += 2;
		return Left(FloatString,Decimel);
	}
	return FloatString;
}

defaultproperties
{
	// the maximum number of stat groups we need to display (num generic stats + max num game-specific stats)
	NumStatGroups=22

	StatMappingsGeneric(0)=(StatId=STATS_COLUMN_POINTS)
	StatMappingsGeneric(1)=(StatId=STATS_COLUMN_POINTSPERMATCH_RATIO)
	StatMappingsGeneric(2)=(StatId=STATS_COLUMN_POINTSPERROUND_RATIO)
	StatMappingsGeneric(3)=(StatId=-1)

	StatMappingsGeneric(4)=(StatId=STATS_COLUMN_KILLS)
	StatMappingsGeneric(5)=(StatId=STATS_COLUMN_DEATHS)
	StatMappingsGeneric(6)=(StatId=STATS_COLUMN_KILLDEATH_RATIO)
	StatMappingsGeneric(7)=(StatId=STATS_COLUMN_REVIVES)
	StatMappingsGeneric(8)=(StatId=STATS_COLUMN_TAKEDOWNS)
	StatMappingsGeneric(9)=(StatId=-1)
	StatMappingsGeneric(10)=(StatId=-1)

	StatMappingsGeneric(11)=(StatId=STATS_COLUMN_MATCHESPLAYED)
	StatMappingsGeneric(12)=(StatId=STATS_COLUMN_MATCHESWON)
	StatMappingsGeneric(13)=(StatId=STATS_COLUMN_MATCHESLOST)
	StatMappingsGeneric(14)=(StatId=-1)

	StatMappingsGeneric(15)=(StatId=STATS_COLUMN_ROUNDSPLAYED)
	StatMappingsGeneric(16)=(StatId=STATS_COLUMN_ROUNDSWON)
	StatMappingsGeneric(17)=(StatId=STATS_COLUMN_ROUNDSLOST)
	StatMappingsGeneric(18)=(StatId=STATS_COLUMN_ROUNDSDRAW)
	StatMappingsGeneric(19)=(StatId=-1)

	StatMappingsAnnex(0)=(StatId=STATS_COLUMN_ANNEX_CAPTURES)
	StatMappingsAnnex(1)=(StatId=STATS_COLUMN_ANNEX_BREAKS)

	StatMappingsKOTH(0)=(StatId=STATS_COLUMN_KOTH_RINGPOINTS)
	StatMappingsKOTH(1)=(StatId=STATS_COLUMN_ANNEX_CAPTURES)
	StatMappingsKOTH(2)=(StatId=STATS_COLUMN_ANNEX_BREAKS)

	StatMappingsMeatflag(0)=(StatId=STATS_COLUMN_MEATFLAG_MEATFLAGCAPTURES)
}
