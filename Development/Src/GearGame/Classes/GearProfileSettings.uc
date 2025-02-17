/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * List of profile settings for Gears
 *
 * NOTE: This class will normally be code generated, but the tool doesn't exist yet
 */
class GearProfileSettings extends OnlineProfileSettings
	native
	config(Game);


const MAX_VOLUME_VALUE	=	10.f;
const NUM_BITS_PER_INT	=	32;

/** Control scheme choices */
enum EGearControlScheme
{
	GCS_Default,
	GCS_Alternate
};

/** Enums for various stick config options */
enum EGearStickConfigOpts
{
	WSCO_Default,
	WSCO_Legacy,
	WSCO_SouthPaw,
	WSCO_LegacySouthpaw
};

/** Enums for various stick config options */
enum EGearTriggerConfigOpts
{
	WTCO_Default,
	WTCO_SouthPaw
};

/** Common yes/no options */
enum EGearOptsYesNo
{
	WOYN_Yes,
	WOYN_No
};

/** Common on/off options */
enum EGearOptsOnOff
{
	WOOO_On,
	WOOO_Off
};

/** COG character choices */
enum ECogMPCharacter
{
	CMPC_Marcus,
	CMPC_Minh,
	CMPC_Cole,
	CMPC_Baird,
	CMPC_Dom,
	CMPC_Carmine,
	CMPC_BenCarmine,
	CMPC_Tai,
	CMPC_Dizzy,
	CMPC_Hoffman
};

/** Locust character preference */
enum ELocustMPCharacter
{
	LMPC_Drone,
	LMPC_DroneSniper,
	LMPC_Hunter,
	LMPC_HunterNoArmor,
	LMPC_Theron,
	LMPC_TheronHelmet,
	LMPC_DroneChopper,
	LMPC_BeastLord,
	LMPC_Flamer,
	LMPC_Kantus,
	LMPC_Skorge,
	LMPC_GeneralRaam
};

/** TV Type of the user. */
enum ETVType
{
	TVT_Default,
	TVT_Soft,
	TVT_Lucent,
	TVT_Vibrant
};

/** Enum to use for the COG tags */
enum ECogTag
{
	COGTAG_None
};

/** Types of weapons that can be replaced */
enum EWeaponReplaceTypes
{
	WEAPRT_Default,
	WEAPRT_Remove
};

/** Types of versus parties there can be */
enum EGearVersusMatchType
{
	/**
	 * ranked public xbox live match
	 * - game options locked
	 * - map vote forced
	 * -
	 */
	eGVMT_Official,

	/**
	 * non-ranked xbox live match
	 * - game options configurable
	 * - map vote available
	 * - only friends or invite
	 */
	eGVMT_Custom,

	/**
	 * non-ranked non-live match
	 * - game options configurable
	 * - joinable
	 */
	eGVMT_SystemLink,

	/** non-network match */
	eGVMT_Local,
};

/** Types of public join types there can be */
enum EGearVersusPartyType
{
	/** party leader must invite players to join the party */
	eGVPT_InviteRequired,

	/** friends of the party leader may join the party without being invited */
	eGVPT_FriendsOnly,

	/** any player may join the party */
	eGVPT_Public,
};

/** Types of coop join types there can be */
enum EGearCoopInviteType
{
	/** host must invite players to join */
	eGCIT_InviteRequired,

	/** friends of the host may join without being invited */
	eGCIT_FriendsOnly,

	/** any player may join */
	eGCIT_Public,
};

/** Types of campaign modes */
enum EGearCampMode
{
	eGCM_LivePublic,
	eGCM_LivePrivate,
	eGCM_SystemLink,
};

/** Types of voting for private matches */
enum EGearMapSelectType
{
	eGEARMAPSELECT_VOTE,
	eGEARMAPSELECT_HOSTSELECT,
};

/** Enum for checkpoint options */
enum EGearCheckpointUsage
{
	eGEARCHECKPOINT_UseLast,
	eGEARCHECKPOINT_Restart,
};

/** Enum of the memory slots used in the campaign */
enum EGearCampaignMemorySlot
{
	eGEARCAMPMEMORYSLOT_0,
	eGEARCAMPMEMORYSLOT_1,
	eGEARCAMPMEMORYSLOT_2,
};

/** Enum of all the UI navigation paths that might want to show a (NEW!) icon */
enum EGearLookAtNavPathType
{
	eNAVPATH_MainMenu_Training,
	eNAVPATH_MainMenu_WarJournal,
	eNAVPATH_WarJournal_Discover,
	eNAVPATH_WarJournal_Unlocks,
	eNAVPATH_WarJournal_Achieve,
};

/** Number of kills it takes to reach the Seriously2 limit */
const Seriously2KillRequirement = 100000;
/** Number of kills it takes to reach the Martyr limit */
const MartyrKillRequirement = 10;
/** Number of chainsaw duel wins for the eGA_CrossedSwords achievement */
const ChainsawDuelRequirement = 10;
/** Number of meatshield hostage taking for the eGA_PoundOfFlesh achievement */
const MeatshieldRequirement = 10;
/** Number of mulcher kills for the eGA_DIYTurret achievement */
const MulcherKillRequirement = 30;
/** Number of mortar kills for the eGA_ShockAndAwe achievement */
const MortarKillRequirement = 30;
/** Number of grenade plant kills for the eGA_SaidTheSpiderToTheFly achievement */
const GrenadePlantKillRequirement = 10;
/** Number of kills when having the boomshield equipped for the eGA_PeekABoo achievement */
const BoomShieldKillRequirement = 10;
/** Number of flamethrower kills for the eGA_ShakeAndBake achievement */
const FlameKillRequirement = 30;
/** Number of Wingman matches won for the eGA_ItTakesTwo achievement */
const WingmanMatchesRequirement = 3;
/** Number of meatflag caps for the eGA_TheOldBallAndChain achievement */
const MeatflagCapRequirement = 10;
/** Number of guardian wins as the leader for the eGA_ItsGoodToBeTheKing achievement */
const LeaderWinRequirement = 10;
/** Number of KOTH wins for the eGA_YouGoAheadIllBeFine achievement */
const KOTHWinRequirement = 3;
/** Number of rounds won for the eGA_Party1999 achievement */
const NumRoundsRequirement = 1999;
/** Number of consecutive waves needed in Horde to get the first Horde achievement */
const NumHordeWavesCompleted_FirstAchieve = 10;
/** Number of waves needed in Horde to get the Horde completed achievement */
const NumHordeWavesCompleted_AllAchieve = 50;
/** Number of chapters to complete in coop to receive the eGA_DomCurious achievement */
const NumChaptersCompleteFor_DomCurious = 1;
/** Number of chapters to complete in coop to receive the eGA_Domination achievement */
const NumChaptersCompleteFor_Domination = 10;
/** Number of perfect active reloads to get eGA_ActiveReload achievement */
const NumActiveReloadsRequirement = 30;
/** Number of Ticker melees to get the eGA_TickerLicker achievement */
const NumTickerMeleeRequirement = 30;

/** Constants for determining when discoverable achievements are achieved */
const DISCOVERABLE_TOURIST	= 5;
const DISCOVERABLE_PACKRAT	= 20;
const DISCOVERABLE_COMPLETE	= 41;

// PSI_MAX is where to start. Can't inherit enums so hard code
const ProfileMode				= 35;
const HudOnOff					= 36;
const MatureLang				= 37;
const GearStickConfig			= 38;
const GearTriggerConfig			= 39;
const Subtitles					= 40;
const MusicVolume				= 41;
const FxVolume					= 42;
const DialogueVolume			= 43;
const CogMPCharacter			= 44;
const LocustMPCharacter			= 45;
const Gore						= 46;
const GameIntensity				= 47;
const PictogramTooltips			= 48;
const GammaSetting				= 49;
const DefaultMPWeapon			= 50;
const GearControlScheme			= 51;
const SelectedDeviceID			= 52;
const TelevisionType			= 53;
const SPTutorials1				= 54;
const SPTutorials2				= 55;
const MPTutorials1				= 56;
const MPTutorials2				= 57;
const TurnInversion				= 58;
const TargetSensitivity			= 59;
const ZoomSensitivity			= 60;
const DiscoverFound1			= 61;
const DiscoverFound2			= 62;
const DiscoverNeedsViewed1		= 63;
const DiscoverNeedsViewed2		= 64;
const MAP_SELECTION_MODE		= 65;
const UnlockedChaptersCasual1	= 66;
const UnlockedChaptersCasual2	= 67;
const UnlockedChaptersNormal1	= 68;
const UnlockedChaptersNormal2	= 69;
const UnlockedChaptersHard1		= 70;
const UnlockedChaptersHard2		= 71;
const UnlockedChaptersInsane1	= 72;
const UnlockedChaptersInsane2	= 73;
const UnlockedUnlockables		= 74;
const UnlockableNeedsViewed		= 75;
const CoopInviteType			= 76;
const CampCheckpointUsage		= 77;
const LastCheckpointSlot		= 78;
const CoopCompletedChapters1	= 79;
const CoopCompletedChapters2	= 80;
const NumChainsawDuelWins		= 81;
const MPMapsCompleted			= 82;
const WeaponsKilledWith			= 83;
const ExecutionsCompleted		= 84;
const NumEnemiesKilled			= 85;
const NumMartyrKills			= 86;
const TrainingAccess			= 87;
const TrainingNeedsViewed		= 88;
const TrainingCompleted			= 89;
const NavigationNeedsViewed		= 90;
const PlaylistId				= 91;
const PREFERRED_SPLIT_TYPE		= 92;
const PlayerSkill				= 93;
const MPGameTypesCompleted		= 94;
const UnlockedChapterAccess1	= 95;
const UnlockedChapterAccess2	= 96;
const WeapTutorialsOn			= 97;

const VERSUS_GAMETYPE			= 100;
const VERSUS_MATCH_MODE			= 101;
const VERSUS_PARTY_TYPE			= 102;
const VERSUS_SELECTED_MAP		= 103;
const CAMP_MODE					= 104;

// Generic MP settings
const MPFriendlyFire			= 120;
const MPRoundTime				= 121;
const MPBleedout				= 122;
const MPWeaponSwap				= 123;
const MPRoundsToWin				= 124;
const MPBotDiff					= 125;
const MPNumBots					= 126;

const COMPLETED_ACHIEVEMENT_MASK_A	=	140;
const COMPLETED_ACHIEVEMENT_MASK_B	=	141;
const UNVIEWED_ACHIEVEMENT_MASK_A	=	142;
const UNVIEWED_ACHIEVEMENT_MASK_B	=	143;

// Annex settings
const AnnexRoundScore			= 186;

// KOTH settings
const KOTHRoundScore			= 206;

// Meatflag settings
const MeatflagVictimDiff		= 226;

// Wingman settings
const WingmanScore				= 247;

// Horde settings
const HordeEnemyDiff			= 266;
const HordeWave					= 267;

// Weapons Swap Settings
const WeapSwap_FragGrenade		= 280;
const WeapSwap_InkGrenade		= 281;
const WeapSwap_Boomshot			= 282;
const WeapSwap_Flame			= 283;
const WeapSwap_Sniper			= 284;
const WeapSwap_Bow				= 285;
const WeapSwap_Mulcher			= 286;
const WeapSwap_Mortar			= 287;
const WeapSwap_HOD				= 288;
const WeapSwap_Gorgon			= 289;
const WeapSwap_Boltok			= 290;
const WeapSwap_Shield			= 291;

// Whether the user has uploaded stats before or not
const HardwareStatsUploaded		= 300;

// More counts for tracking achievements
const NumMeatshieldsUsed		= 310;
const NumMulcherKills			= 311;
const NumMortarKills			= 312;
const NumStickyKills			= 313;
const NumKillsWithBoomShield	= 314;
const NumFlameKills				= 315;
const NumWingmanWins			= 316;
const NumMeatflagCaps			= 317;
const NumLeaderWins				= 318;
const NumKOTHWins				= 319;
const NumRoundsWon				= 320;
const HordeWavesBeat1			= 321;
const HordeWavesBeat2			= 322;
const NumActiveReloads			= 323;
const NumTickerMelees			= 324;

const SPTutorials3				= 330;
const MPTutorials3				= 331;
const TrainTutorials1			= 332;
const TrainTutorials2			= 333;
const TrainTutorials3			= 334;


/**
 * Struct for holding any data needed by the UI when representing or manipulating multiple checkpoints at once.  Eliminates the need
 * to mount and unmount the savegame device to check each checkpoint individually.
 */
struct native transient CheckpointEnumerationResult
{
	var		byte				bCheckpointFileExists[EGearCampaignMemorySlot];
	var		byte				bCheckpointFileContainsData[EGearCampaignMemorySlot];
	var		byte				bCheckpointFileCorrupted[EGearCampaignMemorySlot];
	var		CheckpointTime		CheckpointTimestamp[EGearCampaignMemorySlot];
	var		EDifficultyLevel	CheckpointDifficulty[EGearCampaignMemorySlot];
	var		EChapterPoint		CheckpointChapter[EGearCampaignMemorySlot];

	structcpptext
	{
		FCheckpointEnumerationResult()
		{
			appMemzero(this, sizeof(FCheckpointEnumerationResult));
		}
		FCheckpointEnumerationResult(EEventParm)
		{
			appMemzero(this, sizeof(FCheckpointEnumerationResult));
		}
	}
};

/**
 * Ini option for specifying gametypes which should be hidden (used for demo builds, etc.)
 */
var		config	array<EGearMPTypes>		ExcludedGameTypeIds;

cpptext
{
private:
	/**
	 * Sets all of the profile settings to their default values and then does
	 * a check for your language setting and swaps the subtitle setting based
	 * off of that.
	 */
    virtual void SetToDefaults();

public:
}

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specific profile setting and then returns a array of value mappings.
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Value the out param that gets the value copied to it
 *
 * @return true if found, false otherwise
 */
native function bool GetProfileSettingMappings(int ProfileSettingId,out array<IdToStringMapping> Values);

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then returns the index of the currently selected value.
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Value the out param that gets the index copied ino it.
 *
 * @return true if found, false otherwise
 */
native function bool GetProfileSettingValueMappingIndex(int ProfileSettingId,out int ValueIndex);

/**
 * Hooks to allow child classes to dynamically adjust available profile settings or mappings based on e.g. ini values.
 */
event ModifyAvailableProfileSettings()
{
	Super.ModifyAvailableProfileSettings();

	RemoveExcludedGametypes();
}

/**
 * Removes any gametypes which were configured in the .ini to be disabled from the ProfileMappings array of the VERSUS_GAMETYPE setting.
 */
native final function RemoveExcludedGameTypes();


/** Will return the value the user has set for Gore */
function bool GetGoreConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( Gore, Value );

	switch( Value )
	{
	case WOOO_On:
		Retval = TRUE;
		break;
	case WOOO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}

/** Will return the value the user has set for YInversion */
function bool GetInvertMouseOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( PSI_YInversion, Value );

	switch( Value )
	{
	case PYIO_On:
		Retval = TRUE;
		break;
	case PYIO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}

/** Will return the value the user has set for XInversion */
function bool GetInvertTurnOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( TurnInversion, Value );

	switch( Value )
	{
	case PXIO_On:
		Retval = TRUE;
		break;
	case PXIO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}

/** Will return the value the user has set for ControllerVibratio */
function bool GetControllerVibrationOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( PSI_ControllerVibration, Value );

	`log( "GetControllerVibrationOption: " $ Value );

	switch( Value )
	{
	case PCVTO_On:
		Retval = TRUE;
		break;
	case PCVTO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = TRUE;
		break;
	}

	return Retval;
}




function bool GetHudOnOffConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( HudOnOff, Value );

	switch( Value )
	{
		case WOOO_On:
			Retval = TRUE;
			break;
		case WOOO_Off:
			Retval = FALSE;
			break;
		default:
			Retval = FALSE;
			break;
	}

	return Retval;
}

function bool GetTutorialConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( WeapTutorialsOn, Value );

	switch( Value )
	{
	case WOOO_On:
		Retval = TRUE;
		break;
	case WOOO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}

function bool GetPictogramTooltipsConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( PictogramTooltips, Value );

	switch( Value )
	{
	case WOOO_On:
		Retval = TRUE;
		break;
	case WOOO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}


/** Will return the value the user has set for MatureLanguage */
function bool GetMatureLanguageConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( MatureLang, Value );

	switch( Value )
	{
	case WOOO_On:
		Retval = TRUE;
		break;
	case WOOO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}


/** Will return the value the user has set for Subtitles */
function bool GetSubtitleConfigOption()
{
	local bool Retval;
	local int Value;

	GetProfileSettingValueId( Subtitles, Value );

	switch( Value )
	{
	case WOOO_On:
		Retval = TRUE;
		break;
	case WOOO_Off:
		Retval = FALSE;
		break;
	default:
		Retval = FALSE;
		break;
	}

	return Retval;
}


function EGearTriggerConfigOpts GetTrigConfigOption()
{
	local EGearTriggerConfigOpts RetVal;
	local int Value;

	GetProfileSettingValueId( GearTriggerConfig, Value );
	switch( Value )
	{
		case WTCO_SouthPaw:
			RetVal = WTCO_SouthPaw;
			break;
		case WTCO_Default:
		default:
			RetVal = WTCO_Default;
			break;
	}

	return RetVal;
}

function EGearStickConfigOpts GetStickConfigOption()
{
	local EGearStickConfigOpts RetVal;
	local int Value;

	GetProfileSettingValueId( GearStickConfig, Value );
	switch( Value )
	{
		case WSCO_Legacy:
			RetVal = WSCO_Legacy;
			break;
		case WSCO_SouthPaw:
			RetVal = WSCO_SouthPaw;
			break;
		case WSCO_LegacySouthpaw:
			RetVal = WSCO_LegacySouthpaw;
			break;
		case WSCO_Default:
		default:
			RetVal = WSCO_Default;
			break;
	}

	return RetVal;
}

function EProfileControllerSensitivityOptions GetControllerSensitivityValue()
{
	local EProfileControllerSensitivityOptions RetVal;
	local int Value;

	GetProfileSettingValueId( PSI_ControllerSensitivity, Value );
	switch ( Value )
	{
		case PCSO_Medium:
			RetVal = PCSO_Medium;
			break;
		case PCSO_Low:
			RetVal = PCSO_Low;
			break;
		case PCSO_High:
			RetVal = PCSO_High;
			break;
		default:
			RetVal = PCSO_Medium;
			break;
	}

	return RetVal;
}

function EProfileControllerSensitivityOptions GetTargetSensitivityValue()
{
	local EProfileControllerSensitivityOptions RetVal;
	local int Value;

	GetProfileSettingValueId( TargetSensitivity, Value );
	switch ( Value )
	{
	case PCSO_Medium:
		RetVal = PCSO_Medium;
		break;
	case PCSO_Low:
		RetVal = PCSO_Low;
		break;
	case PCSO_High:
		RetVal = PCSO_High;
		break;
	default:
		RetVal = PCSO_Medium;
		break;
	}

	return RetVal;
}

function EProfileControllerSensitivityOptions GetZoomSensitivityValue()
{
	local EProfileControllerSensitivityOptions RetVal;
	local int Value;

	GetProfileSettingValueId( ZoomSensitivity, Value );
	switch ( Value )
	{
	case PCSO_Medium:
		RetVal = PCSO_Medium;
		break;
	case PCSO_Low:
		RetVal = PCSO_Low;
		break;
	case PCSO_High:
		RetVal = PCSO_High;
		break;
	default:
		RetVal = PCSO_Medium;
		break;
	}

	return RetVal;
}

/** Will return the Percentage to set the Dialog Volume to **/
function float GetDialogVolume()
{
	local float Value;

	GetProfileSettingValueFloat( DialogueVolume, Value );

	return Value;
}


/** Will return the Percentage to set the Fx Volume to **/
function float GetFxVolume()
{
	local float Value;

	GetProfileSettingValueFloat( FxVolume, Value );

	return Value;
}


/** Will return the Percentage to set the Music Volume to **/
function float GetMusicVolume()
{
	local float Value;

	GetProfileSettingValueFloat( MusicVolume, Value );

	return Value;
}

/**
 * Gets the users gamma setting from the profile
 */
function float GetGammaSetting()
{
	local float Value;

	GetProfileSettingValueFloat(GammaSetting,Value);

	return Value;
}

/**
 * @return Returns the user's TV type.
 */
function ETVType GetTVType()
{
	local int Value;
	GetProfileSettingValueId(TelevisionType,Value);

	return ETVType(Value);
}

/** Get the saved tutorial variables */
function GetTutorialValues( out int SPTut1, out int SPTut2, out int SPTut3, out int MPTut1, out int MPTut2, out int MPTut3, out int TrainTut1, out int TrainTut2, out int TrainTut3 )
{
	GetProfileSettingValueInt(SPTutorials1, SPTut1);
	GetProfileSettingValueInt(SPTutorials2, SPTut2);
	GetProfileSettingValueInt(SPTutorials3, SPTut3);
	GetProfileSettingValueInt(MPTutorials1, MPTut1);
	GetProfileSettingValueInt(MPTutorials2, MPTut2);
	GetProfileSettingValueInt(MPTutorials3, MPTut3);
	GetProfileSettingValueInt(TrainTutorials1, TrainTut1);
	GetProfileSettingValueInt(TrainTutorials2, TrainTut2);
	GetProfileSettingValueInt(TrainTutorials3, TrainTut3);
}

/** Set the tutorial values and save the profile */
function SetTutorialValues( int SPTut1, int SPTut2, int SPTut3, int MPTut1, int MPTut2, int MPTut3, int TrainTut1, int TrainTut2, int TrainTut3, GearPC GPC, bool bShouldSave )
{
	SetProfileSettingValueInt(SPTutorials1, SPTut1);
	SetProfileSettingValueInt(SPTutorials2, SPTut2);
	SetProfileSettingValueInt(SPTutorials3, SPTut3);
	SetProfileSettingValueInt(MPTutorials1, MPTut1);
	SetProfileSettingValueInt(MPTutorials2, MPTut2);
	SetProfileSettingValueInt(MPTutorials3, MPTut3);
	SetProfileSettingValueInt(TrainTutorials1, TrainTut1);
	SetProfileSettingValueInt(TrainTutorials2, TrainTut2);
	SetProfileSettingValueInt(TrainTutorials3, TrainTut3);

// 	if ( bShouldSave )
// 	{
// 		GPC.SaveProfile();
// 	}
}

/** Whether a bit is set or not in a 32 bit integer */
final function bool BitIsSet( int IntegerValue, int BitIndex )
{
	return (IntegerValue & (1<<BitIndex)) != 0;
}

/** Whether a discoverable has been found yet */
function bool HasDiscoverableBeenFound( EGearDiscoverableType DiscType )
{
	local int FoundResultValue, ShiftValue;

	// Calculate the shift value
	ShiftValue = DiscType % NUM_BITS_PER_INT;

	// Get the correct variable
	if ( DiscType < NUM_BITS_PER_INT )
	{
		GetProfileSettingValueInt( DiscoverFound1, FoundResultValue );
	}
	else
	{
		GetProfileSettingValueInt( DiscoverFound2, FoundResultValue );
	}

	return BitIsSet( FoundResultValue, ShiftValue );
}

/** Returns how many discoverables have been found */
function int GetNumDiscoverablesFound()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < eDISC_MAX; Idx++ )
	{
		if ( HasDiscoverableBeenFound( EGearDiscoverableType(Idx) ) )
		{
			Count++;
		}
	}

	return Count;
}

/**
 * @return	the number of discoverables that have been found but not viewed
 */
function int GetNumUnviewedDiscoverables()
{
	local int Idx, Result;

	for ( Idx = 0; Idx < eDISC_MAX; Idx++ )
	{
		if ( IsDiscoverableMarkedForAttract(EGearDiscoverableType(Idx)) )
		{
			Result++;
		}
	}

	return Result;
}

/** Whether a newly discovered discoverable has been viewed in the front end yet */
function bool IsDiscoverableMarkedForAttract( EGearDiscoverableType DiscType )
{
	local int NeedsViewedResultValue, ShiftValue;
	local bool bResult;

	if ( HasDiscoverableBeenFound(DiscType) )
	{
		// Calculate the shift value
		ShiftValue = DiscType % NUM_BITS_PER_INT;

		// Get the correct variable
		if ( DiscType < NUM_BITS_PER_INT )
		{
			GetProfileSettingValueInt( DiscoverNeedsViewed1, NeedsViewedResultValue );
		}
		else
		{
			GetProfileSettingValueInt( DiscoverNeedsViewed2, NeedsViewedResultValue );
		}

		bResult = BitIsSet( NeedsViewedResultValue, ShiftValue );
	}

	return bResult;
}

/** Checks to see if the UI should send a progress message or unlock an achievement for discoverables */
function UpdateDiscoverableProgression( GearPC GPC , optional bool bShouldSuppressAlert)
{
	local int NumDiscoverablesFound;

	NumDiscoverablesFound = GetNumDiscoverablesFound();

	// Check for unlocking eGA_Tourist - if DISCOVERABLE_TOURIST changes, xlast must be fixed!!!!
	if (NumDiscoverablesFound <= DISCOVERABLE_TOURIST)
	{
		if (NumDiscoverablesFound == DISCOVERABLE_TOURIST)
		{
			GPC.ClientUnlockAchievement(eGA_Tourist);
		}
		else if (GPC.AlertManager != None && !bShouldSuppressAlert)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_Tourist);
		}
	}
	// Check for unlocking eGA_PackRat - if DISCOVERABLE_PACKRAT changes, xlast must be fixed!!!!
	else if (NumDiscoverablesFound <= DISCOVERABLE_PACKRAT)
	{
		if (NumDiscoverablesFound == DISCOVERABLE_PACKRAT)
		{
			GPC.ClientUnlockAchievement(eGA_PackRat);
		}
		else if (GPC.AlertManager != None && !bShouldSuppressAlert)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_PackRat);
		}
	}
	// Check for unlocking eGA_Completionist - if DISCOVERABLE_COMPLETE changes, xlast must be fixed!!!!
	else if (NumDiscoverablesFound <= DISCOVERABLE_COMPLETE)
	{
		if (NumDiscoverablesFound == DISCOVERABLE_COMPLETE)
		{
			GPC.ClientUnlockAchievement(eGA_Completionist);
		}
		else if (GPC.AlertManager != None && !bShouldSuppressAlert)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_Completionist);
		}
	}
}

/** Mark a discoverable as found but yet not viewed */
function MarkDiscoverableAsFoundButNotViewed( EGearDiscoverableType DiscType, GearPC GPC, optional bool bSaveProfile=true )
{
	local int FoundResultValue, NeedsViewedResultValue, ShiftValue;

	// Make sure to only mark the discoverable if it hasn't already been marked since the "viewed"
	// field could get screwed up on accidental coop completes
	if ( !HasDiscoverableBeenFound(DiscType) )
	{
		// First grab the correct variables from the profile and determine how much bit shifting to do
		if ( DiscType < 32 )
		{
			GetProfileSettingValueInt( DiscoverFound1, FoundResultValue );
			GetProfileSettingValueInt( DiscoverNeedsViewed1, NeedsViewedResultValue );
			ShiftValue = DiscType;
		}
		else
		{
			GetProfileSettingValueInt( DiscoverFound2, FoundResultValue );
			GetProfileSettingValueInt( DiscoverNeedsViewed2, NeedsViewedResultValue );
			ShiftValue = DiscType - 32;
		}

		// Calculate the new values to be set in the profile
		FoundResultValue = FoundResultValue | (1 << ShiftValue);
		NeedsViewedResultValue = NeedsViewedResultValue | (1 << ShiftValue);

		// Set the profile
		if ( DiscType < 32 )
		{
			SetProfileSettingValueInt( DiscoverFound1, FoundResultValue );
			SetProfileSettingValueInt( DiscoverNeedsViewed1, NeedsViewedResultValue );
		}
		else
		{
			SetProfileSettingValueInt( DiscoverFound2, FoundResultValue );
			SetProfileSettingValueInt( DiscoverNeedsViewed2, NeedsViewedResultValue );
		}

		// Check for discoverable progression
		UpdateDiscoverableProgression( GPC );

		// Now mark the path to WarJournal as needing viewed
		MarkNavPathNeedsViewed( eNAVPATH_MainMenu_WarJournal );
		MarkNavPathNeedsViewed( eNAVPATH_WarJournal_Discover );

		// Save the profile
		if ( bSaveProfile )
		{
			GPC.SaveProfile();
		}
	}
}

/** Mark a discoverable as viewed */
function bool MarkDiscoverableAsViewed( EGearDiscoverableType DiscType )
{
	local int Discover1_AttractMask, Discover2_AttractMask, ShiftValue, NegateMaskValue, AttractMask;
	local bool bResult;

	if ( HasDiscoverableBeenFound(DiscType) )
	{
		GetProfileSettingValueInt( DiscoverNeedsViewed1, Discover1_AttractMask );
		GetProfileSettingValueInt( DiscoverNeedsViewed2, Discover2_AttractMask );

		// First grab the correct variables from the profile and determine how much bit shifting to do
		if ( DiscType < 32 )
		{
			AttractMask = Discover1_AttractMask;
			ShiftValue = DiscType;
		}
		else
		{
			AttractMask = Discover2_AttractMask;
			ShiftValue = DiscType - 32;
		}

		// Calculate the new values to be set in the profile
		NegateMaskValue = ~(1 << ShiftValue);
		AttractMask = AttractMask & NegateMaskValue;

		// Set the profile
		if ( DiscType < 32 )
		{
			Discover1_AttractMask = AttractMask;
			SetProfileSettingValueInt( DiscoverNeedsViewed1, AttractMask );
		}
		else
		{
			Discover2_AttractMask = AttractMask;
			SetProfileSettingValueInt( DiscoverNeedsViewed2, AttractMask );
		}

		// if no collectibles are unviewed, remove the nav path attract icon.
		if ( Discover1_AttractMask == 0 && Discover2_AttractMask == 0 )
		{
			MarkNavPathAsViewed( eNAVPATH_WarJournal_Discover );
		}

		// Now mark the path to WarJournal to no longer have attract icon (if all things have been viewed)
		if ( !WarJournalNeedsViewed() )
		{
			MarkNavPathAsViewed( eNAVPATH_MainMenu_WarJournal );
		}

		bResult = true;
	}

	return bResult;
}

/**
 * Marks an achievement as completed and sets the bit for displaying the attract icon in war journal.
 */
function bool MarkAchievementAsCompleted( EGearAchievement AchievementId, GearPC GPC, optional bool bSaveProfile=true )
{
	local int ProfileId, CurrentCompletedAchievementMask, CurrentUnviewedAchievementMask, NewUnviewedAchievementMask;
	local bool bResult;

	if ( !HasCompletedAchievement(AchievementId) )
	{
		// mark it as completed
		ProfileId = AchievementId < NUM_BITS_PER_INT ? COMPLETED_ACHIEVEMENT_MASK_A : COMPLETED_ACHIEVEMENT_MASK_B;
		GetProfileSettingValueInt(ProfileId, CurrentCompletedAchievementMask);
		SetProfileSettingValueInt(ProfileId, CurrentCompletedAchievementMask | (1 << (AchievementId % NUM_BITS_PER_INT)));

		// now set the bit to show the attract icon for achievements in war journal
		ProfileId = AchievementId < NUM_BITS_PER_INT ? UNVIEWED_ACHIEVEMENT_MASK_A : UNVIEWED_ACHIEVEMENT_MASK_B;
		GetProfileSettingValueInt(ProfileId, CurrentUnviewedAchievementMask);

		NewUnviewedAchievementMask = CurrentUnviewedAchievementMask | (1 << (AchievementId % NUM_BITS_PER_INT));
		if ( NewUnviewedAchievementMask != CurrentUnviewedAchievementMask )
		{
			SetProfileSettingValueInt(ProfileId, NewUnviewedAchievementMask);

			MarkNavPathNeedsViewed(eNAVPATH_MainMenu_WarJournal);
			MarkNavPathNeedsViewed(eNAVPATH_WarJournal_Achieve);

			if ( bSaveProfile )
			{
				GPC.SaveProfile();
			}
		}

		bResult = true;
	}

	return bResult;
}

/**
 * Marks the achievement as having been viewed by the user.
 */
function bool MarkAchievementAsViewed( EGearAchievement AchievementId )
{
	local int ProfileId, AchievementMaskValueA, AchievementMaskValueB, NegateMaskValue, AttractMask;
	local bool bResult;

	if ( HasCompletedAchievement(AchievementId) && IsAchievementMarkedForAttract(AchievementId) )
	{
		GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_A, AchievementMaskValueA );
		GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_B, AchievementMaskValueB );

		NegateMaskValue = ~(1 << (AchievementId % NUM_BITS_PER_INT));
		if ( AchievementId < NUM_BITS_PER_INT )
		{
			ProfileId = UNVIEWED_ACHIEVEMENT_MASK_A;
			AttractMask = AchievementMaskValueA & NegateMaskValue;
			bResult = AchievementMaskValueA != AttractMask;

			AchievementMaskValueA = AttractMask;
		}
		else
		{
			ProfileId = UNVIEWED_ACHIEVEMENT_MASK_B;
			AttractMask = AchievementMaskValueB & NegateMaskValue;
			bResult = AchievementMaskValueB != AttractMask;

			AchievementMaskValueB = AttractMask;
		}

		SetProfileSettingValueInt(ProfileId, AttractMask);

		if ( AchievementMaskValueA == 0 && AchievementMaskValueB == 0 )
		{
			MarkNavPathAsViewed( eNAVPATH_WarJournal_Achieve );
		}

		// Now mark the path to WarJournal to no longer have attract icon (if all things have been viewed)
		if ( !WarJournalNeedsViewed() )
		{
			MarkNavPathAsViewed( eNAVPATH_MainMenu_WarJournal );
		}
	}

	return bResult;
}

/**
 * @return	the number of discoverables that have been found but not viewed
 */
function int GetNumUnviewedAchievements()
{
	local int Idx, AchievementMaskValueA, AchievementMaskValueB, Result;

	GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_A, AchievementMaskValueA );
	GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_B, AchievementMaskValueB );

	for ( Idx = 0; Idx < NUM_BITS_PER_INT; Idx++ )
	{
		if ( (AchievementMaskValueA & (1 << Idx)) != 0 )
		{
			Result++;
		}
	}
	for ( Idx = 0; Idx < eGA_DLC1 - NUM_BITS_PER_INT; Idx++ )
	{
		if ( (AchievementMaskValueB & (1 << Idx)) != 0 )
		{
			Result++;
		}
	}

	return Result;
}

/** @return	TRUE if the user completed the specified achievement */
function bool HasCompletedAchievement( EGearAchievement AchievementId )
{
	local int ProfileId, CompletionMaskValue;
	local bool bResult;

	ProfileId = AchievementId < NUM_BITS_PER_INT ? COMPLETED_ACHIEVEMENT_MASK_A : COMPLETED_ACHIEVEMENT_MASK_B;
	if ( GetProfileSettingValueInt(ProfileId, CompletionMaskValue) )
	{
		bResult = (CompletionMaskValue & (1 << (AchievementId % NUM_BITS_PER_INT))) != 0;
	}

	return bResult;
}

/** @return	TRUE if the user has viewed the achievement in the war journal */
function bool IsAchievementMarkedForAttract( EGearAchievement AchievementId )
{
	local int ProfileId, UnviewedAchievementMask;
	local bool bResult;

	ProfileId = AchievementId < NUM_BITS_PER_INT ? UNVIEWED_ACHIEVEMENT_MASK_A : UNVIEWED_ACHIEVEMENT_MASK_B;
	if ( GetProfileSettingValueInt(ProfileId, UnviewedAchievementMask) )
	{
		bResult = (UnviewedAchievementMask & (1 << (AchievementId%32))) != 0;
	}

	return bResult;
}

/**
* @return Returns the user's TV type.
*/
function EGearControlScheme GetControlScheme()
{
	local int Value;
	GetProfileSettingValueId(GearControlScheme,Value);

	return EGearControlScheme(Value);
}

function int GetPreferredClassIndex(int Team)
{
	local int Value;

	GetProfileSettingValueInt( (Team == 1 ? LocustMPCharacter : CogMPCharacter) , Value );
	return Value;
}

function int GetPreferredWeaponId()
{
	local int Value;

	if ( !GetProfileSettingValueInt(DefaultMPWeapon, Value) )
	{
		Value = 13;
	}

	return Value;
}

function bool IsMapVoteEnabled()
{
	local int Value;

	if ( !GetProfileSettingValueId(MAP_SELECTION_MODE, Value) )
	{
		Value = eGEARMAPSELECT_HOSTSELECT;
	}

	return Value != eGEARMAPSELECT_HOSTSELECT;
}

function ESplitScreenType GetPreferredSplitscreenType()
{
	local ESplitscreenType Result;
	local int iResult;

	Result = eSST_2P_HORIZONTAL;
	if ( GetProfileSettingValueId(PREFERRED_SPLIT_TYPE,iResult) )
	{
		Result = ESplitscreenType(iResult);
	}

	return Result;
}

function SetPreferredSplitscreenType( byte Type )
{
	local UIInteraction UIController;

	if ((Type == eSST_2P_HORIZONTAL || Type == eSST_2P_VERTICAL)
	&&	GetPreferredSplitscreenType() != Type )
	{
		SetProfileSettingValueId(PREFERRED_SPLIT_TYPE, Type);
		UIController = class'UIRoot'.static.GetCurrentUIController();
		if ( UIController != None )
		{
			UIController.SetSplitscreenConfiguration(ESplitscreenType(Type));
		}
	}
}

/**
 * Sets the selected device id for this profile.
 *
 * @param DeviceID	New Device ID
 */
function SetCurrentDeviceID(int DeviceID)
{
	local int CurrValue;

	GetProfileSettingValueInt(SelectedDeviceID, CurrValue);
	if ( CurrValue != DeviceID )
	{
		// Set the new device id
		SetProfileSettingValueInt(SelectedDeviceID, DeviceID);

		// clear the last slot used since the device has changed
		SetProfileSettingValueInt(LastCheckpointSlot, -1);
	}
}

/** Returns the correct profile ID of the chapter unlock based on chatper and difficulty */
function int GetChapterUnlockProfileID( EChapterPoint Chapter, EDifficultyLevel Difficulty )
{
	local bool bUseFirstInt;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	switch ( Difficulty )
	{
		case DL_Casual:
			return bUseFirstInt ? UnlockedChaptersCasual1 : UnlockedChaptersCasual2;
		case DL_Normal:
			return bUseFirstInt ? UnlockedChaptersNormal1 : UnlockedChaptersNormal2;
		case DL_Hardcore:
			return bUseFirstInt ? UnlockedChaptersHard1 : UnlockedChaptersHard2;
		case DL_Insane:
			return bUseFirstInt ? UnlockedChaptersInsane1 : UnlockedChaptersInsane2;
	}

	return -1;
}

/** Whether the player has ever unlocked any chapters (excluding the first since that is free) */
function bool HasUnlockedAnyChapters()
{
	local int Value;

	GetProfileSettingValueInt(UnlockedChapterAccess1, Value);
	if (Value > 1)
	{
		return true;
	}

	GetProfileSettingValueInt(UnlockedChapterAccess2, Value);
	if (Value > 0)
	{
		return true;
	}

	return false;
}

/** Whether a chapter has been unlocked yet in a particular difficulty */
function bool HasChapterBeenUnlocked( EChapterPoint Chapter, EDifficultyLevel Difficulty )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable from the helper function
	ProfileID = GetChapterUnlockProfileID( Chapter, Difficulty );

	GetProfileSettingValueInt( ProfileID, ResultValue );

	return BitIsSet( ResultValue, ShiftValue );
}

/** Whether a chapter has been unlocked for access */
function bool HasChapterBeenUnlockedForAccess( EChapterPoint Chapter )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable from the helper function
	ProfileID = bUseFirstInt ? UnlockedChapterAccess1 : UnlockedChapterAccess2;

	GetProfileSettingValueInt( ProfileID, ResultValue );

	return BitIsSet( ResultValue, ShiftValue );
}

/** Whether an act has been unlocked yet in a particular difficulty */
function bool HasActBeenUnlocked( EGearAct Act, EDifficultyLevel Difficulty )
{
	local array<EChapterPoint> ChapterList;
	local int Idx;

	class'GearUIDataStore_GameResource'.static.GetChapterPointsFromAct(Act, ChapterList);
	for ( Idx = 0; Idx < ChapterList.length; Idx++ )
	{
		if ( HasChapterBeenUnlocked(ChapterList[Idx], Difficulty) )
		{
			return true;
		}
	}

	return false;
}

/** Whether an act has been completed yet using any variety of difficulties */
function bool HasActBeenCompletedAcrossDifficulties(EGearAct Act)
{
	local array<EChapterPoint> ChapterList;
	local int DiffIndex;
	local int ChapterIndex;
	local bool bHasUnlocked;

	class'GearUIDataStore_GameResource'.static.GetChapterPointsFromAct(Act, ChapterList);
	// Loop through all chapters in this act
	for (ChapterIndex = 0; ChapterIndex < ChapterList.Length; ChapterIndex++)
	{
		bHasUnlocked = false;
		// Loop through all difficulties
		for (DiffIndex = 0; DiffIndex < DL_MAX; DiffIndex++)
		{
			if (HasChapterBeenUnlocked(EChapterPoint(ChapterList[ChapterIndex]+1), EDifficultyLevel(DiffIndex)))
			{
				bHasUnlocked = true;
				break;
			}
		}
		// If was not unlocked bail
		if (!bHasUnlocked)
		{
			return false;
		}
	}
	return true;
}

/** Whether an act has been completed yet in a particular difficulty */
function bool HasActBeenCompleted( EGearAct Act, EDifficultyLevel Difficulty )
{
	local array<EChapterPoint> ChapterList;
	local int Idx;

	class'GearUIDataStore_GameResource'.static.GetChapterPointsFromAct(Act, ChapterList);
	for ( Idx = 0; Idx < ChapterList.length; Idx++ )
	{
		// we're checking for the chapter to be completed so we need to add 1 to the index
		if ( !HasChapterBeenUnlocked(EChapterPoint(ChapterList[Idx]+1), Difficulty) )
		{
			return false;
		}
	}

	return true;
}

/** Whether an act has been unlocked for access */
function bool HasActBeenUnlockedForAccess( EGearAct Act )
{
	local array<EChapterPoint> ChapterList;
	local int Idx;

	class'GearUIDataStore_GameResource'.static.GetChapterPointsFromAct(Act, ChapterList);
	for ( Idx = 0; Idx < ChapterList.length; Idx++ )
	{
		if ( HasChapterBeenUnlockedForAccess(ChapterList[Idx]) )
		{
			return true;
		}
	}

	return false;
}

/** Marks the chapter for access in the menus */
function MarkChapterForAccess( EChapterPoint Chapter )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable
	ProfileID = bUseFirstInt ? UnlockedChapterAccess1 : UnlockedChapterAccess2;

	// Get the current value in the profile
	GetProfileSettingValueInt( ProfileID, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << ShiftValue);

	// Set the profile
	SetProfileSettingValueInt( ProfileID, ResultValue );
}

/** Whether a chapter has been completed when playing in a coop game */
function bool HasChapterBeenCompletedInCoop( EChapterPoint Chapter )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable
	ProfileID = bUseFirstInt ? CoopCompletedChapters1 : CoopCompletedChapters2;

	GetProfileSettingValueInt( ProfileID, ResultValue );

	return BitIsSet( ResultValue, ShiftValue );
}

/** Return the number of chapters completed in coop */
function int GetNumChaptersCompletedInCoop()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < CHAP_Gameover; Idx++ )
	{
		if ( HasChapterBeenCompletedInCoop( EChapterPoint(Idx) ) )
		{
			Count++;
		}
	}

	return Count;
}

/** Checks to see if an achievement for coop chapter completion is ready */
function UpdateCoopChapterCompleteProgression( GearPC GPC, bool bAlreadyComplete )
{
	local int NumCoopChaptersComplete;

	NumCoopChaptersComplete = GetNumChaptersCompletedInCoop();

	// Check for unlocking eGA_DomCurious
	if (NumCoopChaptersComplete == NumChaptersCompleteFor_DomCurious)
	{
		GPC.ClientUnlockAchievement(eGA_DomCurious);
	}
	else if (NumCoopChaptersComplete <= NumChaptersCompleteFor_Domination)
	{
		// Check for unlocking eGA_Domination
		if (NumCoopChaptersComplete == NumChaptersCompleteFor_Domination)
		{
			GPC.ClientUnlockAchievement(eGA_Domination);
		}
		else if (GPC.AlertManager != None && !bAlreadyComplete)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_Domination);
		}
	}
	else if (NumCoopChaptersComplete <= CHAP_Gameover)
	{
		// Check for unlocking eGA_ICantQuitYouDom
		if (NumCoopChaptersComplete == CHAP_Gameover)
		{
			GPC.ClientUnlockAchievement(eGA_ICantQuitYouDom);
		}
		else if (GPC.AlertManager != None && !bAlreadyComplete)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_ICantQuitYouDom);
		}
	}
}

/** Marks the CoopCompletedChapters(s) variables to reflect completing the chapter passed in - profile will be save by the caller */
function MarkCoopChapterComplete( EChapterPoint Chapter, GearPC GPC )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;
	local bool bAlreadyComplete;

	bAlreadyComplete = HasChapterBeenCompletedInCoop(Chapter);

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable
	ProfileID = bUseFirstInt ? CoopCompletedChapters1 : CoopCompletedChapters2;

	// Get the current value in the profile
	GetProfileSettingValueInt( ProfileID, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << ShiftValue);

	// Set the profile
	SetProfileSettingValueInt( ProfileID, ResultValue );

	// Check for achievement progress
	UpdateCoopChapterCompleteProgression( GPC, bAlreadyComplete );
}

/**
 * Unlocks a chapter for a given difficulty
 *
 * @param ChapterNum - the chapter to unlock
 * @param Difficulty - the difficulty of the current game
 * @param GPC - GearPC of the player unlocking the chapter
 * #param bSkipSave - prevents the profile from being saved (useful for recursively calling this function for unlocking lesser difficulties)
 */
function UnlockChapterByDifficulty( EChapterPoint Chapter, EDifficultyLevel Difficulty, GearPC GPC, optional bool bSkipSave )
{
	local int ResultValue, ShiftValue, ProfileID;
	local bool bUseFirstInt;
	local bool bShouldSave;

	// Determine if we're using the first packed integer or the second
	bUseFirstInt = (Chapter < 32);

	// Calculate the shift value
	ShiftValue = bUseFirstInt ? int(Chapter) : Chapter - 32;

	// Get the correct variable from the helper function
	ProfileID = GetChapterUnlockProfileID( Chapter, Difficulty );

	// Get the current value in the profile
	GetProfileSettingValueInt( ProfileID, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << ShiftValue);

	// Set the profile
	SetProfileSettingValueInt( ProfileID, ResultValue );

	// See if the player just finished the game so we can unlock
	// the Insane difficulty and some game completion achievements
	if ( HasCompletedGame(Difficulty) )
	{
		// Unlock insane difficulty for completing the game
		MarkUnlockableAsUnlockedButNotViewed( eUNLOCK_InsaneDifficulty, GPC );

		// Unlock game completion achievements
		// NOTE: There are no breaks in this switch on purpose so that a player who beats
		//	the game on a higher difficulty gets credit for all difficulties!
		switch ( Difficulty )
		{
			case DL_Insane:
				GPC.ClientUnlockAchievement( eGA_Commander );
			case DL_Hardcore:
				GPC.ClientUnlockAchievement( eGA_CommissionedOfficer );
			case DL_Normal:
				GPC.ClientUnlockAchievement( eGA_NonCommissionedOfficer );
			case DL_Casual:
				GPC.ClientUnlockAchievement( eGA_Reservist );
		}
	}

	// Unlock this chapter at lesser difficulties
	if ( Difficulty > DL_Casual )
	{
		// Will recursively unlock chapters at lesser difficulties but will not save the profile
		// until the recursion unwinds back to the root.
		UnlockChapterByDifficulty( Chapter, EDifficultyLevel(Difficulty-1), GPC, true );
	}

	// Mark the chapter as accessible now
	if ( !HasChapterBeenUnlockedForAccess(EChapterPoint(Chapter)) )
	{
		MarkChapterForAccess(Chapter);
	}

	// Mark the chapter they just completed too since they could have jumped here via JIP
	if (Chapter > 0 &&
		!HasChapterBeenUnlockedForAccess(EChapterPoint(Chapter-1)) )
	{
		MarkChapterForAccess(EChapterPoint(Chapter-1));
	}

	bShouldSave = true;

	// See if we should check for coop chapter completion
	// Hack: make sure they've been playing the game for at least 5 minutes so they can't ninja
	// the achievement
	if (GPC.IsActuallyPlayingCoop() &&
		(GPC.WorldInfo.TimeSeconds - GPC.CreationTime) > 90)
	{
		MarkCoopChapterComplete( EChapterPoint(Chapter-1), GPC );
		bShouldSave = true;
	}

	// See if we need to unlock an unlockable and save the profile if needed
	if (UpdateUnlockingUnlockable(EChapterPoint(Chapter-1), GPC))
	{
		bShouldSave = true;
	}

	// Save the profile
	if (bShouldSave && !bSkipSave)
	{
		GPC.SaveProfile();
	}
}

/** Checks the profile settings and determines if all of the chapters in a given act have been completed */
function bool HasCompletedAct( EGearAct Act, EDifficultyLevel Difficulty )
{
	local array<EChapterPoint> ChapterList;
	local int Idx;

	class'GearUIDataStore_GameResource'.static.GetChapterPointsFromAct(Act, ChapterList);
	for ( Idx = 0; Idx < ChapterList.length; Idx++ )
	{
		// Must check for "unlock" on the following chapter to determine if the chapter
		// in question has been "completed"
		if ( !HasChapterBeenUnlocked(EChapterPoint(ChapterList[Idx]+1), Difficulty) )
		{
			return false;
		}
	}

	// We must special case the final chapter since there isn't a chapter that will unlock it
	// NOTE: CHAP_Gameover is a fake chapter for determining whether the player has completed the final
	//	chapter in the game.
	if ( Act == GEARACT_V &&
		 !HasChapterBeenUnlocked(CHAP_Gameover, Difficulty) )
	{
		return false;
	}

	return true;
}

/** Checks the profile settings and determines if all of the chapters in the entire game have been completed within the same difficulty */
function bool HasCompletedGame( EDifficultyLevel Difficulty )
{
	local array<EGearAct> ActList;
	local int Idx;

	class'GearUIDataStore_GameResource'.static.GetGameCompletionActList(ActList);
	for ( Idx = 0; Idx < ActList.length; Idx++ )
	{
		if ( !HasCompletedAct(ActList[Idx], Difficulty) )
		{
			return false;
		}
	}

	return true;
}

/** Returns the number of chapters that have been completed on a particular difficulty */
function int GetNumChaptersCompleted( EDifficultyLevel Difficulty )
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 1; Idx <= CHAP_Gameover; Idx++ )
	{
		if ( HasChapterBeenUnlocked( EChapterPoint(Idx), Difficulty ) )
		{
			Count++;
		}
	}

	return Count;
}

/** Have the Gears1 characters been unlocked or not? */
function bool Gears1CharactersAreUnlocked()
{
	if ( HasUnlockableBeenUnlocked(eUNLOCK_Character_Carmine1) &&
		 HasUnlockableBeenUnlocked(eUNLOCK_Character_Minh) &&
		 HasUnlockableBeenUnlocked(eUNLOCK_Character_Raam) )
	{
		return true;
	}

	return false;
}

/** Whether an unlockable has been unlocked or not */
function bool HasUnlockableBeenUnlocked( EGearUnlockable Unlockable )
{
	local int ResultValue;

	GetProfileSettingValueInt( UnlockedUnlockables, ResultValue );

	return BitIsSet( ResultValue, Unlockable );
}

/** Whether a newly unlocked unlockable has been viewed in the front-end yet */
function bool IsUnlockableMarkedForAttract( EGearUnlockable Unlockable )
{
	local int MarkedForAttractValue;

	GetProfileSettingValueInt( UnlockableNeedsViewed, MarkedForAttractValue );
	return BitIsSet(MarkedForAttractValue, Unlockable);
}

/**
 * Checks to see if this is the chapter that complete an act and attempts to unlock an unlockable because of it
 *
 * @return - whether something was actually unlocked or not
 */
function bool UpdateUnlockingUnlockable(EChapterPoint ChapterCompleted, GearPC GPC)
{
	local GearCampaignActData ActData;
	local EGearUnlockable Unlockable;

	ActData = class'GearUIDataStore_GameResource'.static.GetActDataProviderUsingChapter(ChapterCompleted);
	if (ActData != none)
	{
		if (HasActBeenCompletedAcrossDifficulties(ActData.ActType))
		{
			switch (ActData.ActType)
			{
				case GEARACT_I:
					Unlockable = eUNLOCK_Character_Dizzy;
					break;
				case GEARACT_II:
					Unlockable = eUNLOCK_Character_Kantus;
					break;
				case GEARACT_III:
					Unlockable = eUNLOCK_Character_Tai;
					break;
				case GEARACT_IV:
					Unlockable = eUNLOCK_Character_PalaceGuard;
					break;
				case GEARACT_V:
					Unlockable = eUNLOCK_Character_Skorge;
					break;
			}
			return MarkUnlockableAsUnlockedButNotViewed(Unlockable, GPC);
		}
	}
	return false;
}

/** Mark an unlockable as unlocked but not yet viewed in the front-end */
function bool MarkUnlockableAsUnlockedButNotViewed(EGearUnlockable Unlockable, GearPC GPC)
{
	local int ResultValue, NeedsViewedResultValue;

	// Make sure to only mark the unlockable if it hasn't already been marked since the "viewed"
	// field could get screwed up on accidental coop completes
	if ( !HasUnlockableBeenUnlocked(Unlockable) )
	{
		if (GPC.AlertManager != None)
		{
			GPC.AlertManager.MadeProgress(ePROG_Unlockable, Unlockable);
		}

		GetProfileSettingValueInt( UnlockedUnlockables, ResultValue );
		GetProfileSettingValueInt( UnlockableNeedsViewed, NeedsViewedResultValue );

		// Calculate the new values to be set in the profile
		ResultValue = ResultValue | (1 << Unlockable);
		NeedsViewedResultValue = NeedsViewedResultValue | (1 << Unlockable);

		// Set the profile
		SetProfileSettingValueInt( UnlockedUnlockables, ResultValue );
		SetProfileSettingValueInt( UnlockableNeedsViewed, NeedsViewedResultValue );

		// Now mark the path to WarJournal as needing viewed
		MarkNavPathNeedsViewed( eNAVPATH_MainMenu_WarJournal );
		MarkNavPathNeedsViewed( eNAVPATH_WarJournal_Unlocks );

		return true;
	}
	return false;
}

/** Mark an unlockable as viewed in the front-end */
function bool MarkUnlockableAsViewed( EGearUnlockable Unlockable )
{
	local int NegateMaskValue, AttractMask;

	if ( HasUnlockableBeenUnlocked(Unlockable) )
	{
		GetProfileSettingValueInt( UnlockableNeedsViewed, AttractMask );

		// Calculate the new values to be set in the profile
		NegateMaskValue = ~(1 << Unlockable);
		AttractMask = AttractMask & NegateMaskValue;

		// Set the profile
		SetProfileSettingValueInt( UnlockableNeedsViewed, AttractMask );

		// if there are no more unlockables to view, remove the attract icon from the nav path
		if ( AttractMask == 0 )
		{
			MarkNavPathAsViewed( eNAVPATH_WarJournal_Unlocks );
		}

		if ( !WarJournalNeedsViewed() )
		{
			MarkNavPathAsViewed( eNAVPATH_MainMenu_WarJournal );
		}

		return true;
	}

	return false;
}

/** Whether a match has been won on a particular map */
function bool HasWonMPOnMap( EGearMapsShipped MapType )
{
	local int ResultValue;

	GetProfileSettingValueInt( MPMapsCompleted, ResultValue );

	return BitIsSet( ResultValue, MapType );
}

/** Get the number of completed MP maps */
function int GetNumCompletedMPMaps()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < eGEARMAP_MAX; Idx++ )
	{
		if ( HasWonMPOnMap(EGearMapsShipped(Idx)) )
		{
			Count++;
		}
	}

	return Count;
}

/** Get the number of MP maps we're shipping with */
function int GetNumShippedMPMaps()
{
	// Subtract 1 because the first index in the enum is None
	return (eGEARMAP_MAX - 1);
}

/** Mark the MP map as having been won on (completed) */
function MarkMPMapCompleted( EGearMapsShipped MapType )
{
	local int ResultValue;

	// Get the current value in the profile
	GetProfileSettingValueInt( MPMapsCompleted, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << MapType);

	// Set the profile
	SetProfileSettingValueInt( MPMapsCompleted, ResultValue );
}

/** Checks for achievements dealing with winning MP gametypes (eGA_AroundTheWorld) */
function UpdateMPMatchProgression( EGearMPTypes MPType, EGearMapsShipped MapType, GearPC GPC )
{
	local bool bHasFinishedMap;

	`log("GearProfileSettings:UpdateMPMatchProgression:"@MPType@MapType@GPC);

	// Check for eGA_AroundTheWorld
	if ( MapType != eGEARMAP_None )
	{
		bHasFinishedMap = HasWonMPOnMap(MapType);

		MarkMPMapCompleted(MapType);

		if ( GetNumCompletedMPMaps() >= GetNumShippedMPMaps() )
		{
			GPC.ClientUnlockAchievement( eGA_AroundTheWorld );
		}
		else if (GPC.AlertManager != None && !bHasFinishedMap)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_AroundTheWorld);
		}
	}
}

/** Get the number of weapons in the game you can kill with */
function int GetNumWeaponsForKilling()
{
	// Subtract 1 because the last index in the enum is Disabled
	// Subtract 1 because you can't kill someone with a smoke grenade
	// Subtract 1 because of the shield
	// Subtract 2 for the golden weapon DLCs
	return (eGEARWEAP_MAX - 5);
}

/** Whether a weapon was used to kill someone */
function bool HasKilledWithWeapon( EGearWeaponType WeaponType )
{
	local int ResultValue;

	GetProfileSettingValueInt( WeaponsKilledWith, ResultValue );

	return BitIsSet( ResultValue, WeaponType );
}

/** Get the number of weapons we killed with */
function int GetNumWeaponsKilledWith()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < eGEARWEAP_MAX; Idx++ )
	{
		if ( HasKilledWithWeapon(EGearWeaponType(Idx)) )
		{
			Count++;
		}
	}

	return Count;
}

/** Mark the weapon as having killed someone with it */
function MarkWeaponWeKilledWith( EGearWeaponType WeaponType, GearPC GPC )
{
	local int ResultValue;
	local bool bKilledWithAlready;

	bKilledWithAlready = HasKilledWithWeapon(WeaponType);

	// Get the current value in the profile
	GetProfileSettingValueInt( WeaponsKilledWith, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << WeaponType);

	// Set the profile
	SetProfileSettingValueInt( WeaponsKilledWith, ResultValue );

	// Check for achievement and progression
	if ( GetNumWeaponsKilledWith() >= GetNumWeaponsForKilling() )
	{
		GPC.ClientUnlockAchievement( eGA_VarietyIsTheSpiceOfDeath );
	}
	else if (GPC.AlertManager != None && !bKilledWithAlready)
	{
		GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_VarietyIsTheSpiceOfDeath);
	}
}

/** Checks for achievements dealing with killing enemies */
function UpdateKillingProgression( class<GearDamageType> DmgType, GearPC GPC )
{
	local int NumTotalKills, NumTotalMartyrKills;
	local EGearWeaponType WeaponType;

	// Check for generic killing enemies achievement
	if ( GetProfileSettingValueInt( NumEnemiesKilled, NumTotalKills ) )
	{
		NumTotalKills++;
		SetProfileSettingValueInt( NumEnemiesKilled, NumTotalKills );
		if ( NumTotalKills >= Seriously2KillRequirement )
		{
			GPC.ClientUnlockAchievement( eGA_Seriously2, TRUE );
		}
		else if (GPC.AlertManager != None)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_Seriously2);
		}
	}

	// Check for killing enemies via martyring
	if ( DmgType == class'GDT_FragMartyr' || DmgType == class'GDT_InkMartyr' )
	{
		if ( GetProfileSettingValueInt( NumMartyrKills, NumTotalMartyrKills ) )
		{
			NumTotalMartyrKills++;
			SetProfileSettingValueInt( NumMartyrKills, NumTotalMartyrKills );
			if ( NumTotalMartyrKills >= MartyrKillRequirement )
			{
				GPC.ClientUnlockAchievement( eGA_Martyr );
			}
			else if (GPC.AlertManager != None)
			{
				GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_Martyr);
			}
		}
	}

	//@hack: special case replacements because GetWeaponType() only supports 1 to 1 matches
	switch (DmgType)
	{
		case class'GDT_FragMartyr':
		case class'GDT_FragSticky':
		case class'GDT_FragTag':
			DmgType = class'GDT_FragGrenade';
			break;
		case class'GDT_InkMartyr':
		case class'GDT_InkSticky':
		case class'GDT_InkTag':
			DmgType = class'GDT_InkGrenade';
			break;
		case class'GDT_Chainsaw':
			DmgType = class'GDT_AssaultRifle';
			break;
		case class'GDT_TorqueBow_Impact':
			DmgType = class'GDT_TorqueBow_Explosion';
			break;
		case class'GDT_MortarUntargeted':
			DmgType = class'GDT_Mortar';
			break;
		default:
			break;
	}

	if ( class'GearTypes'.static.GetWeaponType(DmgType, WeaponType) )
	{
		//`log("GearProfileSettings:UpdateKillingProgression:"@DmgType@WeaponType@GPC);

		// Check for eGA_VarietyIsTheSpiceOfDeath
		MarkWeaponWeKilledWith(WeaponType, GPC);
	}
}

/** Checks for completion of progressive achievements */
function UpdateAchievementProgression(EGearAchievement Achievement, GearPC GPC)
{
	local int NumTotal;
	local int Requirement;
	local int ProfileId;

	switch (Achievement)
	{
		case eGA_CrossedSwords:
			Requirement = ChainsawDuelRequirement;
			ProfileId = NumChainsawDuelWins;
			break;
		case eGA_PoundOfFlesh:
			Requirement = MeatshieldRequirement;
			ProfileId = NumMeatshieldsUsed;
			break;
		case eGA_DIYTurret:
			Requirement = MulcherKillRequirement;
			ProfileId = NumMulcherKills;
			break;
		case eGA_ShockAndAwe:
			Requirement = MortarKillRequirement;
			ProfileId = NumMortarKills;
			break;
		case eGA_SaidTheSpiderToTheFly:
			Requirement = GrenadePlantKillRequirement;
			ProfileId = NumStickyKills;
			break;
		case eGA_PeekABoo:
			Requirement = BoomShieldKillRequirement;
			ProfileId = NumKillsWithBoomShield;
			break;
		case eGA_ShakeAndBake:
			Requirement = FlameKillRequirement;
			ProfileId = NumFlameKills;
			break;
		case eGA_ItTakesTwo:
			Requirement = WingmanMatchesRequirement;
			ProfileId = NumWingmanWins;
			break;
		case eGA_TheOldBallAndChain:
			Requirement = MeatflagCapRequirement;
			ProfileId = NumMeatflagCaps;
			break;
		case eGA_ItsGoodToBeTheKing:
			Requirement = LeaderWinRequirement;
			ProfileId = NumLeaderWins;
			break;
		case eGA_YouGoAheadIllBeFine:
			Requirement = KOTHWinRequirement;
			ProfileId = NumKOTHWins;
			break;
		case eGA_Party1999:
			Requirement = NumRoundsRequirement;
			ProfileId = NumRoundsWon;
			break;
		case eGA_ActiveReload:
			Requirement = NumActiveReloadsRequirement;
			ProfileId = NumActiveReloads;
			break;
		case eGA_TickerLicker:
			Requirement = NumTickerMeleeRequirement;
			ProfileId = NumTickerMelees;
			break;
		default:
			return;
	}

	if (GetProfileSettingValueInt(ProfileId, NumTotal))
	{
		NumTotal++;
		SetProfileSettingValueInt(ProfileId, NumTotal);
		if (NumTotal >= Requirement)
		{
			GPC.ClientUnlockAchievement(Achievement);
		}
		else
		{
			if (GPC.AlertManager != None)
			{
				GPC.AlertManager.MadeProgress(ePROG_Achievement, Achievement);
			}
		}
	}
}

/** Returns the current progression value for a progressive achievement */
final function int GetCurrentProgressValueForAchievement(EGearAchievement Achievement)
{
	local int NumTotal;
	local int ProfileId;

	switch (Achievement)
	{
		case eGA_CrossedSwords:
			ProfileId = NumChainsawDuelWins;
			break;
		case eGA_PoundOfFlesh:
			ProfileId = NumMeatshieldsUsed;
			break;
		case eGA_DIYTurret:
			ProfileId = NumMulcherKills;
			break;
		case eGA_ShockAndAwe:
			ProfileId = NumMortarKills;
			break;
		case eGA_SaidTheSpiderToTheFly:
			ProfileId = NumStickyKills;
			break;
		case eGA_PeekABoo:
			ProfileId = NumKillsWithBoomShield;
			break;
		case eGA_ShakeAndBake:
			ProfileId = NumFlameKills;
			break;
		case eGA_ItTakesTwo:
			ProfileId = NumWingmanWins;
			break;
		case eGA_TheOldBallAndChain:
			ProfileId = NumMeatflagCaps;
			break;
		case eGA_ItsGoodToBeTheKing:
			ProfileId = NumLeaderWins;
			break;
		case eGA_YouGoAheadIllBeFine:
			ProfileId = NumKOTHWins;
			break;
		case eGA_Party1999:
			ProfileId = NumRoundsWon;
			break;
		case eGA_ActiveReload:
			ProfileId = NumActiveReloads;
			break;
		case eGA_TickerLicker:
			ProfileId = NumTickerMelees;
			break;
		default:
			`log("ERROR!!! Unsupported achievement"@Achievement@"is looking for a current progression value!!!");
			return 0;
	}

	GetProfileSettingValueInt(ProfileId, NumTotal);
	return NumTotal;
}

/** Sets the current progression value for a progressive achievement (only should be used in a cheat for testing!!!!) */
final function SetCurrentProgressValueForAchievement(EGearAchievement Achievement, int Value)
{
	local int ProfileId;

	switch (Achievement)
	{
		case eGA_CrossedSwords:
			ProfileId = NumChainsawDuelWins;
			break;
		case eGA_PoundOfFlesh:
			ProfileId = NumMeatshieldsUsed;
			break;
		case eGA_DIYTurret:
			ProfileId = NumMulcherKills;
			break;
		case eGA_ShockAndAwe:
			ProfileId = NumMortarKills;
			break;
		case eGA_SaidTheSpiderToTheFly:
			ProfileId = NumStickyKills;
			break;
		case eGA_PeekABoo:
			ProfileId = NumKillsWithBoomShield;
			break;
		case eGA_ShakeAndBake:
			ProfileId = NumFlameKills;
			break;
		case eGA_ItTakesTwo:
			ProfileId = NumWingmanWins;
			break;
		case eGA_TheOldBallAndChain:
			ProfileId = NumMeatflagCaps;
			break;
		case eGA_ItsGoodToBeTheKing:
			ProfileId = NumLeaderWins;
			break;
		case eGA_YouGoAheadIllBeFine:
			ProfileId = NumKOTHWins;
			break;
		case eGA_Party1999:
			ProfileId = NumRoundsWon;
			break;
		case eGA_Martyr:
			ProfileId = NumMartyrKills;
			break;
		case eGA_Seriously2:
			ProfileId = NumEnemiesKilled;
			break;
		case eGA_ActiveReload:
			ProfileId = NumActiveReloads;
			break;
		case eGA_TickerLicker:
			ProfileId = NumTickerMelees;
			break;
		default:
			`log("ERROR!!! Unsupported achievement"@Achievement@"is looking for a current progression value!!!");
			return;
	}

	SetProfileSettingValueInt(ProfileId, Value);
}

/** Whether we've executed someone with this specific type of execution */
function bool HasDoneExecution( EGearExecutionType ExecutionType )
{
	local int ResultValue;

	GetProfileSettingValueInt( ExecutionsCompleted, ResultValue );

	return BitIsSet( ResultValue, ExecutionType );
}

/** Get the number of executions we've completed */
function int GetNumExecutionsCompleted()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < eGET_MAX; Idx++ )
	{
		if ( HasDoneExecution(EGearExecutionType(Idx)) )
		{
			Count++;
		}
	}

	return Count;
}

/** Mark the execution as having been completed */
function MarkWeaponWeExecutedWith( EGearExecutionType ExecutionType )
{
	local int ResultValue;

	// Get the current value in the profile
	GetProfileSettingValueInt( ExecutionsCompleted, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << ExecutionType);

	// Set the profile
	SetProfileSettingValueInt( ExecutionsCompleted, ResultValue );
}

/** Checks for achievements dealing with executing enemies */
function UpdateExecutionProgression( class<GearDamageType> DmgType, GearPC GPC )
{
	local EGearExecutionType ExecutionType;

	if ( class'GearTypes'.static.GetExecutionType(DmgType, ExecutionType) )
	{
		// Check for eGA_MultitalentedExecutioner
		AttemptMarkForExecutionAchievement(ExecutionType, GPC);
	}
}

/** Checks for the execution achievement */
function AttemptMarkForExecutionAchievement( EGearExecutionType ExecutionType, GearPC GPC )
{
	local bool bHasAlreadyExecuted;

	bHasAlreadyExecuted = HasDoneExecution(ExecutionType);

	MarkWeaponWeExecutedWith(ExecutionType);

	if ( GetNumExecutionsCompleted() >= eGET_MAX )
	{
		GPC.ClientUnlockAchievement( eGA_MultitalentedExecutioner );
	}
	else if (GPC.AlertManager != None && !bHasAlreadyExecuted)
	{
		GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_MultitalentedExecutioner);
	}
}

/** Whether a training scenario has been completed or not */
function bool HasCompletedTraining( EGearTrainingType TrainType )
{
	local int ResultValue;

	GetProfileSettingValueInt( TrainingCompleted, ResultValue );

	return BitIsSet( ResultValue, TrainType );
}

/** Whether a newly available training mission has been played yet */
function bool HasTrainingBeenViewed( EGearTrainingType TrainType )
{
	local int NeedsViewedResultValue;

	GetProfileSettingValueInt( TrainingNeedsViewed, NeedsViewedResultValue );

	return BitIsSet( NeedsViewedResultValue, TrainType );
}

/** Whether a training mission is accessible or not */
function bool IsTrainingAccessible( EGearTrainingType TrainType )
{
	return true;
// 	local int ResultValue;
//
// 	GetProfileSettingValueInt( TrainingAccess, ResultValue );
//
// 	return BitIsSet( ResultValue, TrainType );
}

/** Get the number of completed training scenarios */
function int GetNumCompletedTraining()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < eGEARTRAIN_MAX; Idx++ )
	{
		if ( HasCompletedTraining(EGearTrainingType(Idx)) )
		{
			Count++;
		}
	}

	return Count;
}

/** Mark the training scenario as having been completed */
function MarkTrainingCompleted( EGearTrainingType TrainType, GearPC GPC )
{
	local int ResultValue;
	local bool bHasDoneTraining;

	bHasDoneTraining = HasCompletedTraining(TrainType);

	// Get the current value in the profile
	GetProfileSettingValueInt( TrainingCompleted, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << TrainType);

	// Set the profile
	SetProfileSettingValueInt( TrainingCompleted, ResultValue );

	if ( GetNumCompletedTraining() >= eGEARTRAIN_MAX )
	{
		GPC.ClientUnlockAchievement( eGA_BackToBasic );
	}
	else if (GPC.AlertManager != None && !bHasDoneTraining)
	{
		GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_BackToBasic);
	}

	GPC.SaveProfile();
}

/** Mark the training scenario as needing viewed */
function MarkTrainingNeedsViewed( EGearTrainingType TrainType )
{
	local int NeedsViewedResultValue;

	// Get the current value in the profile
	GetProfileSettingValueInt( TrainingNeedsViewed, NeedsViewedResultValue );

	// Calculate the new values to be set in the profile
	NeedsViewedResultValue = NeedsViewedResultValue | (1 << TrainType);

	// Set the profile
	SetProfileSettingValueInt( TrainingNeedsViewed, NeedsViewedResultValue );

	// Now mark the path to training as needing viewed
	MarkNavPathNeedsViewed( eNAVPATH_MainMenu_Training );
}

/** Mark the training scenario as accessible */
function MarkTrainingAccessible( EGearTrainingType TrainType )
{
	local int ResultValue;

	// Get the current value in the profile
	GetProfileSettingValueInt( TrainingAccess, ResultValue );

	// Calculate the new values to be set in the profile
	ResultValue = ResultValue | (1 << TrainType);

	// Set the profile
	SetProfileSettingValueInt( TrainingAccess, ResultValue );
}

/** Mark a training as played */
function MarkTrainingAsViewed( EGearTrainingType TrainType, GearPC GPC )
{
	local int NeedsViewedResultValue, NegateMaskValue;

	GetProfileSettingValueInt( TrainingNeedsViewed, NeedsViewedResultValue );

	// Calculate the new values to be set in the profile
	NegateMaskValue = ~(1 << TrainType);
	NeedsViewedResultValue = NeedsViewedResultValue & NegateMaskValue;

	SetProfileSettingValueInt( TrainingNeedsViewed, NeedsViewedResultValue );

	if ( NeedsViewedResultValue == 0 )
	{
		MarkNavPathAsViewed(eNAVPATH_MainMenu_Training);
	}
}

/**
 * Wrapper function for retrieving a reference to a navigation data provider.
 *
 * @param	NavType		indicates which navigation data provider to return.
 *
 * @return	a reference to a data provider which provdies navigation functionality for the specified navtype.
 */
function GearDataProvider_SceneNavigationData FindSceneNavigationDataProvider( EGearLookAtNavPathType NavType )
{
	local UIDataStore_GameResource GameResourceDS;
	local array<UIResourceDataProvider> Providers;
	local GearDataProvider_SceneNavigationData NavProvider, Result;
	local int ProviderIndex;

	GameResourceDS = class'GearUIScene_Base'.static.GetGameResourceDataStore();
	if ( GameResourceDS != None && GameResourceDS.GetResourceProviders('NavigationMenuItems', Providers) )
	{
		for ( ProviderIndex = 0; ProviderIndex < Providers.Length; ProviderIndex++ )
		{
			NavProvider = GearDataProvider_SceneNavigationData(Providers[ProviderIndex]);
			if ( NavProvider != None && NavProvider.FindNavItemIndexByAttractId(string(NavType)) != INDEX_NONE )
			{
				Result = NavProvider;
				break;
			}
		}
	}

	return Result;
}

/**
 * Marks the scene navigation data provider with any nav paths that need to be viewed.  Used to update the provider after the player
 * signs into a profile
 */
function ApplyNavPathAttractionToProvider()
{
	local byte NavIndex;
	local EGearLookAtNavPathType NavType;
	local GearDataProvider_SceneNavigationData Provider;

	for ( NavIndex = 0; NavIndex < eNAVPATH_MAX; NavIndex++ )
	{
		NavType = EGearLookAtNavPathType(NavIndex);
		Provider = FindSceneNavigationDataProvider(NavType);
		if ( Provider != None )
		{
			Provider.SetAttractMark(string(NavType), NavigationPathNeedsViewed(NavType));
		}
	}
}

/**
 * Wrapper for determining whether anything in the war journal needs to be viewed.
 */
final function bool WarJournalNeedsViewed()
{
	local int CollectibleViewMask1, CollectibleViewMask2, UnlockableViewMask, UnviewedAchievementMask1, UnviewedAchievementMask2;

	// First grab the correct variables from the profile and determine how much bit shifting to do
	GetProfileSettingValueInt( DiscoverNeedsViewed1, CollectibleViewMask1 );
	GetProfileSettingValueInt( DiscoverNeedsViewed2, CollectibleViewMask2 );
	GetProfileSettingValueInt( UnlockableNeedsViewed, UnlockableViewMask );
	GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_A, UnviewedAchievementMask1 );
	GetProfileSettingValueInt( UNVIEWED_ACHIEVEMENT_MASK_B, UnviewedAchievementMask2 );

	return	UnlockableViewMask != 0
		||	CollectibleViewMask1 != 0 || CollectibleViewMask2 != 0
		||	UnviewedAchievementMask1 != 0 || UnviewedAchievementMask2 != 0;
}

/** Whether a navigation path should show the (NEW!) icon */
function bool NavigationPathNeedsViewed( EGearLookAtNavPathType NavType )
{
	local int ResultValue;

	GetProfileSettingValueInt( NavigationNeedsViewed, ResultValue );

	return BitIsSet( ResultValue, NavType );
}

/** Mark a navigation path as needing viewed (LOOK AT ME!) */
function MarkNavPathNeedsViewed( EGearLookAtNavPathType NavType )
{
	local int NeedsViewedResultValue;
	local GearDataProvider_SceneNavigationData Provider;

	// Get the current value in the profile
	GetProfileSettingValueInt( NavigationNeedsViewed, NeedsViewedResultValue );

	// Calculate the new values to be set in the profile
	NeedsViewedResultValue = NeedsViewedResultValue | (1 << NavType);

	// Set the profile
	SetProfileSettingValueInt( NavigationNeedsViewed, NeedsViewedResultValue );

	Provider = FindSceneNavigationDataProvider(NavType);
	if ( Provider != None )
	{
		Provider.SetAttractMark(string(NavType),true);
	}
}

/** Mark a navigation path as viewed */
function MarkNavPathAsViewed( EGearLookAtNavPathType NavType )
{
	local int AttractMask, NegateAttractMask;
	local GearDataProvider_SceneNavigationData Provider;

	if ( NavigationPathNeedsViewed(NavType) )
	{
		GetProfileSettingValueInt( NavigationNeedsViewed, AttractMask );

		// Calculate the new values to be set in the profile
		NegateAttractMask = ~(1 << NavType);
		AttractMask = AttractMask & NegateAttractMask;

		SetProfileSettingValueInt( NavigationNeedsViewed, AttractMask );

		Provider = FindSceneNavigationDataProvider(NavType);
		if ( Provider != None )
		{
			Provider.SetAttractMark(string(NavType),false);
		}
	}
}

/**
 * Get the completion amount for a game achievement.
 *
 * @param	AchievementId	the id for the achievement to get the completion percetage for
 * @param	CurrentValue	the current number of times the event required to unlock the achievement has occurred.
 * @param	MaxValue		the value that represents 100% completion.
 *
 * @return	TRUE if the AchievementId specified represents an progressive achievement.  FALSE if the achievement is a one-time event
 *			or if the AchievementId specified is invalid.
 */
function bool GetAchievementProgression( EGearAchievement AchievementId, out float CurrentValue, out float MaxValue )
{
	local bool bResult;
	local int ProfileValue;
	local EGearExecutionType ExecType;
	local EGearTrainingType TrainType;
	local int ScriptHatesMe;

	switch ( AchievementId )
	{
		case eGA_Tourist:
			MaxValue = DISCOVERABLE_TOURIST;
			CurrentValue = GetNumDiscoverablesFound();
			bResult = true;
		break;

		case eGA_PackRat:
			MaxValue = DISCOVERABLE_PACKRAT;
			CurrentValue = GetNumDiscoverablesFound();
			bResult = true;
		break;

		case eGA_Completionist:
			MaxValue = DISCOVERABLE_COMPLETE;
			CurrentValue = GetNumDiscoverablesFound();
			bResult = true;
		break;

		case eGA_Domination:
			MaxValue = NumChaptersCompleteFor_Domination;
			CurrentValue = GetNumChaptersCompletedInCoop();
			bResult = true;
		break;

		case eGA_ICantQuitYouDom:
			ScriptHatesMe = CHAP_Gameover;
			MaxValue = float(ScriptHatesMe);
			CurrentValue = GetNumChaptersCompletedInCoop();
			bResult = true;
		break;

		case eGA_CrossedSwords:
			MaxValue = ChainsawDuelRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_CrossedSwords);
			bResult = true;
		break;

		case eGA_PoundOfFlesh:
			MaxValue = MeatshieldRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_PoundOfFlesh);
			bResult = true;
		break;

		case eGA_DIYTurret:
			MaxValue = MulcherKillRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_DIYTurret);
			bResult = true;
		break;

		case eGA_ShockAndAwe:
			MaxValue = MortarKillRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_ShockAndAwe);
			bResult = true;
		break;

		case eGA_SaidTheSpiderToTheFly:
			MaxValue = GrenadePlantKillRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_SaidTheSpiderToTheFly);
			bResult = true;
		break;

		case eGA_PeekABoo:
			MaxValue = BoomShieldKillRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_PeekABoo);
			bResult = true;
		break;

		case eGA_ShakeAndBake:
			MaxValue = FlameKillRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_ShakeAndBake);
			bResult = true;
		break;

		case eGA_VarietyIsTheSpiceOfDeath:
			CurrentValue = GetNumWeaponsKilledWith();
			MaxValue = GetNumWeaponsForKilling();
			bResult = true;
		break;

		case eGA_MultitalentedExecutioner:
			CurrentValue = GetNumExecutionsCompleted();
			ExecType = eGET_MAX;
			MaxValue = ExecType;
			bResult = true;
		break;

		case eGA_Seriously2:
			GetProfileSettingValueInt( NumEnemiesKilled, ProfileValue );
			CurrentValue = ProfileValue;
			MaxValue = Seriously2KillRequirement;
			bResult = true;
		break;

		case eGA_ItTakesTwo:
			MaxValue = WingmanMatchesRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_ItTakesTwo);
			bResult = true;
		break;

		case eGA_TheOldBallAndChain:
			MaxValue = MeatflagCapRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_TheOldBallAndChain);
			bResult = true;
		break;

		case eGA_ItsGoodToBeTheKing:
			MaxValue = LeaderWinRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_ItsGoodToBeTheKing);
			bResult = true;
		break;

		case eGA_YouGoAheadIllBeFine:
			MaxValue = KOTHWinRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_YouGoAheadIllBeFine);
			bResult = true;
		break;

		case eGA_BackToBasic:
			CurrentValue = GetNumCompletedTraining();
			TrainType = eGEARTRAIN_MAX;
			MaxValue = TrainType;
			bResult = true;
		break;

		case eGA_Martyr:
			GetProfileSettingValueInt( NumMartyrKills, ProfileValue );
			CurrentValue = ProfileValue;
			MaxValue = MartyrKillRequirement;
			bResult = true;
		break;

		case eGA_Party1999:
			MaxValue = NumRoundsRequirement;
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_Party1999);
			bResult = true;
		break;

		case eGA_AroundTheWorld:
			CurrentValue = GetNumCompletedMPMaps();
			MaxValue = GetNumShippedMPMaps();
			bResult = true;
		break;

		case eGA_SurvivedToTen:
			CurrentValue = GetNumConsecutiveHordeWavesCompleted(NumHordeWavesCompleted_FirstAchieve);
			MaxValue = NumHordeWavesCompleted_FirstAchieve;
			bResult = true;
		break;

		case eGA_SurvivedToFifty:
			CurrentValue = GetNumHordeWavesCompleted();
			MaxValue = NumHordeWavesCompleted_AllAchieve;
			bResult = true;
		break;

		case eGA_ActiveReload:
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_ActiveReload);
			MaxValue = NumActiveReloadsRequirement;
			bResult = true;
		break;

		case eGA_TickerLicker:
			CurrentValue = GetCurrentProgressValueForAchievement(eGA_TickerLicker);
			MaxValue = NumTickerMeleeRequirement;
			bResult = true;
		break;

		default:
			// not a progression achievement
			bResult = false;
		break;
	}

	return bResult;
}

/** Whether a horde wave has been completed */
function bool HasCompletedHordeWave( int HordeWaveIdx )
{
	local int FoundResultValue, ShiftValue;

	// Calculate the shift value
	ShiftValue = (HordeWaveIdx < 32) ? HordeWaveIdx : HordeWaveIdx - 32;

	// Get the correct variable
	if ( HordeWaveIdx < 32 )
	{
		GetProfileSettingValueInt( HordeWavesBeat1, FoundResultValue );
	}
	else
	{
		GetProfileSettingValueInt( HordeWavesBeat2, FoundResultValue );
	}

	return BitIsSet( FoundResultValue, ShiftValue );
}

/** Returns how many Horde waves have been completed */
function int GetNumHordeWavesCompleted()
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < NumHordeWavesCompleted_AllAchieve; Idx++ )
	{
		if ( HasCompletedHordeWave(Idx) )
		{
			Count++;
		}
	}

	return Count;
}

/** Returns how many of the first consecutive Horde waves have been completed */
function int GetNumConsecutiveHordeWavesCompleted(int NumWaves)
{
	local int Idx, Count;

	Count = 0;
	for ( Idx = 0; Idx < NumWaves; Idx++ )
	{
		if ( HasCompletedHordeWave(Idx) )
		{
			Count++;
		}
		else
		{
			break;
		}
	}

	return Count;
}

/** Checks to see if the UI should send a progress message or unlock an achievement for completing a Horde wave */
function UpdateHordeWaveProgression( bool bDoConsecutiveCheck, GearPC GPC, bool bAlreadyCompleted )
{
	local int NumWavesCompleted;
	local int NumConsecutiveWavesCompleted;

	NumWavesCompleted = GetNumHordeWavesCompleted();

	// Check for unlocking eGA_SurvivedToFifty
	if ( NumWavesCompleted == NumHordeWavesCompleted_AllAchieve )
	{
		GPC.ClientUnlockAchievement( eGA_SurvivedToFifty );
	}
	else if (GPC.AlertManager != None && !bAlreadyCompleted)
	{
		GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_SurvivedToFifty);
	}

	// Check for unlocking eGA_SurvivedToTen
	if ( bDoConsecutiveCheck )
	{
		NumConsecutiveWavesCompleted = GetNumConsecutiveHordeWavesCompleted(NumHordeWavesCompleted_FirstAchieve);
		if ( NumConsecutiveWavesCompleted == NumHordeWavesCompleted_FirstAchieve )
		{
			GPC.ClientUnlockAchievement( eGA_SurvivedToTen );
		}
		else if (GPC.AlertManager != None && !bAlreadyCompleted)
		{
			GPC.AlertManager.MadeProgress(ePROG_Achievement, eGA_SurvivedToTen);
		}
	}
}

/** Mark a Horde wave as completed */
function MarkHordeWaveAsCompleted( int HordeWaveIdx, GearPC GPC )
{
	local int FoundResultValue, ShiftValue;
	local bool bCheckForConsecutiveAchievement;
	local bool bAlreadyCompleted;

	bAlreadyCompleted = HasCompletedHordeWave(HordeWaveIdx);

	// See if the Progression/Achievement update should check for the eGA_SurvivedToTen achievement/progression
	bCheckForConsecutiveAchievement = (HordeWaveIdx < NumHordeWavesCompleted_FirstAchieve);

	// First grab the correct variables from the profile and determine how much bit shifting to do
	if ( HordeWaveIdx < 32 )
	{
		GetProfileSettingValueInt( HordeWavesBeat1, FoundResultValue );
		ShiftValue = HordeWaveIdx;
	}
	else
	{
		GetProfileSettingValueInt( HordeWavesBeat2, FoundResultValue );
		ShiftValue = HordeWaveIdx - 32;
	}

	// Calculate the new values to be set in the profile
	FoundResultValue = FoundResultValue | (1 << ShiftValue);

	// Set the profile
	if ( HordeWaveIdx < 32 )
	{
		SetProfileSettingValueInt( HordeWavesBeat1, FoundResultValue );
	}
	else
	{
		SetProfileSettingValueInt( HordeWavesBeat2, FoundResultValue );
	}

	// Check for progression and achievement unlock
	UpdateHordeWaveProgression(bCheckForConsecutiveAchievement, GPC, bAlreadyCompleted);

	GPC.SaveProfile();
}

/** Returns the last playlist id that the player used */
function int GetPlaylistID()
{
	local int PlaylistIdResult;
	local OnlinePlaylistProvider PlaylistProvider;

	if ( !GetProfileSettingValueInt(PlaylistId, PlaylistIdResult) )
	{
		PlaylistIdResult = 1;
	}

	PlaylistProvider = class'GearUIDataStore_GameResource'.static.GetOnlinePlaylistProvider( PlaylistIdResult );
	if ( PlaylistProvider == None )
	{
		PlaylistIdResult = 1;
	}

	return PlaylistIdResult;
}

/** @return Whether the hardware stats have been uploaded or not */
function bool HaveHardwareStatsBeenUploaded()
{
	local int UploadFlag;

	if (GetProfileSettingValueInt(HardwareStatsUploaded,UploadFlag))
	{
		return UploadFlag > 0;
	}
	return false;
}

/** Sets the upload flag to true */
function MarkHardwareStatsAsUploaded()
{
	SetProfileSettingValueInt(HardwareStatsUploaded,1);
}

defaultproperties
{
	// If you change any profile ids, increment this number!!!!
	VersionNumber=63

	// Only read the properties that we care about
	ProfileSettingIds(0)=PSI_ControllerVibration
	ProfileSettingIds(1)=PSI_YInversion
	ProfileSettingIds(2)=PSI_ControllerSensitivity

	// Defaults for the values if not specified by the online service
	DefaultSettings(0)=(Owner=OPPO_OnlineService,ProfileSetting=(PropertyId=PSI_ControllerVibration,Data=(Type=SDT_Int32,Value1=PCVTO_On)))
	DefaultSettings(1)=(Owner=OPPO_OnlineService,ProfileSetting=(PropertyId=PSI_YInversion,Data=(Type=SDT_Int32,Value1=PYIO_Off)))
	DefaultSettings(2)=(Owner=OPPO_OnlineService,ProfileSetting=(PropertyId=PSI_ControllerSensitivity,Data=(Type=SDT_Int32,Value1=PCSO_Medium)))

	// Base profile mapping (the other two come from the base class)
	ProfileMappings(2)=(Id=PSI_ControllerSensitivity,Name="Controller Sensitivity",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCSO_Low),(Id=PCSO_Medium),(Id=PCSO_High)))

	// Custom game settings follow
	//
	// To read the data
	ProfileSettingIds(22)=ProfileMode
	ProfileSettingIds(23)=HudOnOff
	ProfileSettingIds(24)=MatureLang
	ProfileSettingIds(25)=GearStickConfig
	ProfileSettingIds(26)=GearTriggerConfig
	ProfileSettingIds(27)=Subtitles
	ProfileSettingIds(28)=MusicVolume
	ProfileSettingIds(29)=FxVolume
	ProfileSettingIds(30)=DialogueVolume
	ProfileSettingIds(31)=CogMPCharacter
	ProfileSettingIds(32)=LocustMPCharacter
	ProfileSettingIds(33)=Gore
	ProfileSettingIds(34)=GameIntensity
	ProfileSettingIds(35)=PictogramTooltips
	ProfileSettingIds(36)=GammaSetting
	ProfileSettingIds(37)=DefaultMPWeapon
	ProfileSettingIds(38)=GearControlScheme
	ProfileSettingIds(39)=SelectedDeviceID
	ProfileSettingIds(40)=TelevisionType
	ProfileSettingIds(41)=VERSUS_GAMETYPE
	ProfileSettingIds(42)=VERSUS_MATCH_MODE
	ProfileSettingIds(43)=VERSUS_PARTY_TYPE
	ProfileSettingIds(44)=VERSUS_SELECTED_MAP
	ProfileSettingIds(45)=MPFriendlyFire
	ProfileSettingIds(46)=MPRoundTime
	ProfileSettingIds(47)=MPBleedout
	ProfileSettingIds(48)=MPWeaponSwap
	ProfileSettingIds(49)=MPRoundsToWin
	ProfileSettingIds(50)=MPBotDiff
	ProfileSettingIds(51)=AnnexRoundScore
	ProfileSettingIds(52)=KOTHRoundScore
	ProfileSettingIds(53)=MeatflagVictimDiff
	ProfileSettingIds(54)=HordeEnemyDiff
	ProfileSettingIds(55)=SPTutorials1
	ProfileSettingIds(56)=SPTutorials2
	ProfileSettingIds(57)=MPTutorials1
	ProfileSettingIds(58)=MPTutorials2
	ProfileSettingIds(59)=TurnInversion
	ProfileSettingIds(60)=TargetSensitivity
	ProfileSettingIds(61)=ZoomSensitivity
	ProfileSettingIds(62)=MPNumBots
	ProfileSettingIds(63)=HordeWave
	ProfileSettingIds(64)=DiscoverFound1
	ProfileSettingIds(65)=DiscoverFound2
	ProfileSettingIds(66)=DiscoverNeedsViewed1
	ProfileSettingIds(67)=DiscoverNeedsViewed2
	ProfileSettingIds(68)=MAP_SELECTION_MODE
	ProfileSettingIds(69)=WeapSwap_FragGrenade
	ProfileSettingIds(70)=WeapSwap_InkGrenade
	ProfileSettingIds(71)=WeapSwap_Boomshot
	ProfileSettingIds(72)=WeapSwap_Flame
	ProfileSettingIds(73)=WeapSwap_Sniper
	ProfileSettingIds(74)=WeapSwap_Bow
	ProfileSettingIds(75)=WeapSwap_Mulcher
	ProfileSettingIds(76)=WeapSwap_Mortar
	ProfileSettingIds(77)=WeapSwap_HOD
	ProfileSettingIds(78)=WeapSwap_Gorgon
	ProfileSettingIds(79)=WeapSwap_Boltok
	ProfileSettingIds(80)=WingmanScore
	ProfileSettingIds(81)=UnlockedChaptersCasual1
	ProfileSettingIds(82)=UnlockedChaptersCasual2
	ProfileSettingIds(83)=UnlockedChaptersNormal1
	ProfileSettingIds(84)=UnlockedChaptersNormal2
	ProfileSettingIds(85)=UnlockedChaptersHard1
	ProfileSettingIds(86)=UnlockedChaptersHard2
	ProfileSettingIds(87)=UnlockedChaptersInsane1
	ProfileSettingIds(88)=UnlockedChaptersInsane2
	ProfileSettingIds(89)=UnlockedUnlockables
	ProfileSettingIds(90)=UnlockableNeedsViewed
	ProfileSettingIds(91)=CoopInviteType
	ProfileSettingIds(92)=CampCheckpointUsage
	ProfileSettingIds(93)=LastCheckpointSlot
	ProfileSettingIds(94)=CoopCompletedChapters1
	ProfileSettingIds(95)=CoopCompletedChapters2
	ProfileSettingIds(96)=MPGameTypesCompleted
	ProfileSettingIds(97)=MPMapsCompleted
	ProfileSettingIds(98)=WeaponsKilledWith
	ProfileSettingIds(99)=ExecutionsCompleted
	ProfileSettingIds(100)=NumEnemiesKilled
	ProfileSettingIds(101)=NumMartyrKills
	ProfileSettingIds(102)=TrainingAccess
	ProfileSettingIds(103)=TrainingNeedsViewed
	ProfileSettingIds(104)=TrainingCompleted
	ProfileSettingIds(105)=NavigationNeedsViewed
	ProfileSettingIds(106)=PlaylistId
	ProfileSettingIds(107)=PREFERRED_SPLIT_TYPE
	ProfileSettingIds(108)=PlayerSkill
	ProfileSettingIds(109)=HardwareStatsUploaded
	ProfileSettingIds(110)=CAMP_MODE
	ProfileSettingIds(111)=NumChainsawDuelWins
	ProfileSettingIds(112)=NumMeatshieldsUsed
	ProfileSettingIds(113)=NumMulcherKills
	ProfileSettingIds(114)=NumMortarKills
	ProfileSettingIds(115)=NumStickyKills
	ProfileSettingIds(116)=NumKillsWithBoomShield
	ProfileSettingIds(117)=NumFlameKills
	ProfileSettingIds(118)=NumWingmanWins
	ProfileSettingIds(119)=NumMeatflagCaps
	ProfileSettingIds(120)=NumLeaderWins
	ProfileSettingIds(121)=NumKOTHWins
	ProfileSettingIds(122)=NumRoundsWon
	ProfileSettingIds(123)=HordeWavesBeat1
	ProfileSettingIds(124)=HordeWavesBeat2
	ProfileSettingIds(125)=UnlockedChapterAccess1
	ProfileSettingIds(126)=UnlockedChapterAccess2
	ProfileSettingIds(127)=WeapTutorialsOn
	ProfileSettingIds(128)=WeapSwap_Shield
	ProfileSettingIds(129)=SPTutorials3
	ProfileSettingIds(130)=MPTutorials3
	ProfileSettingIds(131)=TrainTutorials1
	ProfileSettingIds(132)=TrainTutorials2
	ProfileSettingIds(133)=TrainTutorials3
	ProfileSettingIds(134)=NumActiveReloads
	ProfileSettingIds(135)=NumTickerMelees
	ProfileSettingIds(136)=COMPLETED_ACHIEVEMENT_MASK_A
	ProfileSettingIds(137)=COMPLETED_ACHIEVEMENT_MASK_B
	ProfileSettingIds(138)=UNVIEWED_ACHIEVEMENT_MASK_A
	ProfileSettingIds(139)=UNVIEWED_ACHIEVEMENT_MASK_B

	// These are defaults for first time read
	DefaultSettings(22)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=ProfileMode,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(23)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HudOnOff,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(24)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MatureLang,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(25)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=GearStickConfig,Data=(Type=SDT_Int32,Value1=WSCO_Default)))
	DefaultSettings(26)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=GearTriggerConfig,Data=(Type=SDT_Int32,Value1=WTCO_Default)))
	DefaultSettings(27)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=Subtitles,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(28)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MusicVolume,Data=(Type=SDT_Float,Value1=1058642330)))		// 0.6
	DefaultSettings(29)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=FxVolume,Data=(Type=SDT_Float,Value1=1061997773)))			// 0.8
	DefaultSettings(30)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DialogueVolume,Data=(Type=SDT_Float,Value1=1061997773)))	// 0.8
	DefaultSettings(31)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CogMPCharacter,Data=(Type=SDT_Int32,Value1=CMPC_Marcus)))
	DefaultSettings(32)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=LocustMPCharacter,Data=(Type=SDT_Int32,Value1=LMPC_Drone)))
	DefaultSettings(33)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=Gore,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(34)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=GameIntensity,Data=(Type=SDT_Int32,Value1=DL_Normal)))
	DefaultSettings(35)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PictogramTooltips,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(36)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=GammaSetting,Data=(Type=SDT_Float,Value1=1074580685)))		// 2.2
	DefaultSettings(37)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DefaultMPWeapon,Data=(Type=SDT_Int32,Value1=13)))
	DefaultSettings(38)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=GearControlScheme,Data=(Type=SDT_Int32,Value1=GCS_Default)))
	DefaultSettings(39)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=SelectedDeviceID,Data=(Type=SDT_Int32,Value1=-1)))
	DefaultSettings(40)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TelevisionType,Data=(Type=SDT_Int32,Value1=TVT_Default)))
	DefaultSettings(41)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=VERSUS_GAMETYPE,Data=(Type=SDT_Int32,Value1=eGEARMP_Warzone)))
	DefaultSettings(42)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=VERSUS_MATCH_MODE,Data=(Type=SDT_Int32,Value1=eGVMT_Official)))
	DefaultSettings(43)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=VERSUS_PARTY_TYPE,Data=(Type=SDT_Int32,Value1=eGVPT_FriendsOnly)))
	DefaultSettings(44)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=VERSUS_SELECTED_MAP,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(45)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPFriendlyFire,Data=(Type=SDT_Int32,Value1=CONTEXT_FRIENDLYFIRE_NO)))
	DefaultSettings(46)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPRoundTime,Data=(Type=SDT_Int32,Value1=CONTEXT_ROUNDTIME_FIVE)))
	DefaultSettings(47)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPBleedout,Data=(Type=SDT_Int32,Value1=CONTEXT_BLEEDOUTTIME_FIFTEEN)))
	DefaultSettings(48)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPWeaponSwap,Data=(Type=SDT_Int32,Value1=CONTEXT_WEAPONSWAP_CYCLE)))
	DefaultSettings(49)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPRoundsToWin,Data=(Type=SDT_Int32,Value1=CONTEXT_ROUNDSTOWIN_FIVE)))
	DefaultSettings(50)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPBotDiff,Data=(Type=SDT_Int32,Value1=CONTEXT_AIDIFFICULTY_NORMAL)))
	DefaultSettings(51)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=AnnexRoundScore,Data=(Type=SDT_Int32,Value1=CONTEXT_ROUNDSCORE_ANNEX_240)))
	DefaultSettings(52)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=KOTHRoundScore,Data=(Type=SDT_Int32,Value1=CONTEXT_ROUNDSCORE_KOTH_120)))
	DefaultSettings(53)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MeatflagVictimDiff,Data=(Type=SDT_Int32,Value1=CONTEXT_AIDIFFICULTY_NORMAL)))
	DefaultSettings(54)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HordeEnemyDiff,Data=(Type=SDT_Int32,Value1=CONTEXT_AIDIFFICULTY_HARDCORE)))
	DefaultSettings(55)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=SPTutorials1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(56)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=SPTutorials2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(57)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPTutorials1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(58)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPTutorials2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(59)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TurnInversion,Data=(Type=SDT_Int32,Value1=PXIO_Off)))
	DefaultSettings(60)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TargetSensitivity,Data=(Type=SDT_Int32,Value1=PCSO_Medium)))
	DefaultSettings(61)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=ZoomSensitivity,Data=(Type=SDT_Int32,Value1=PCSO_Medium)))
	DefaultSettings(62)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPNumBots,Data=(Type=SDT_Int32,Value1=CONTEXT_NUMBOTS_0)))
	DefaultSettings(63)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HordeWave,Data=(Type=SDT_Int32,Value1=CONTEXT_HORDE_WAVE_0)))
	DefaultSettings(64)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DiscoverFound1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(65)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DiscoverFound2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(66)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DiscoverNeedsViewed1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(67)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=DiscoverNeedsViewed2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(68)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MAP_SELECTION_MODE,Data=(Type=SDT_Int32,Value1=eGEARMAPSELECT_HOSTSELECT)))
	DefaultSettings(69)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_FragGrenade,Data=(Type=SDT_Int32,Value1=eGEARWEAP_FragGrenade)))
	DefaultSettings(70)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_InkGrenade,Data=(Type=SDT_Int32,Value1=eGEARWEAP_InkGrenade)))
	DefaultSettings(71)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Boomshot,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Boomshot)))
	DefaultSettings(72)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Flame,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Flame)))
	DefaultSettings(73)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Sniper,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Sniper)))
	DefaultSettings(74)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Bow,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Bow)))
	DefaultSettings(75)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Mulcher,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Mulcher)))
	DefaultSettings(76)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Mortar,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Mortar)))
	DefaultSettings(77)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_HOD,Data=(Type=SDT_Int32,Value1=eGEARWEAP_HOD)))
	DefaultSettings(78)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Gorgon,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Gorgon)))
	DefaultSettings(79)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Boltok,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Boltok)))
	DefaultSettings(80)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WingmanScore,Data=(Type=SDT_Int32,Value1=CONTEXT_WINGMAN_SCOREGOAL_15)))
	DefaultSettings(81)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersCasual1,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(82)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersCasual2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(83)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersNormal1,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(84)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersNormal2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(85)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersHard1,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(86)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersHard2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(87)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersInsane1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(88)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChaptersInsane2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(89)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedUnlockables,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(90)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockableNeedsViewed,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(91)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CoopInviteType,Data=(Type=SDT_Int32,Value1=eGCIT_FriendsOnly)))
	DefaultSettings(92)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CampCheckpointUsage,Data=(Type=SDT_Int32,Value1=eGEARCHECKPOINT_UseLast)))
	DefaultSettings(93)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=LastCheckpointSlot,Data=(Type=SDT_Int32,Value1=-1)))
	DefaultSettings(94)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CoopCompletedChapters1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(95)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CoopCompletedChapters2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(96)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPGameTypesCompleted,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(97)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPMapsCompleted,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(98)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeaponsKilledWith,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(99)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=ExecutionsCompleted,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(100)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumEnemiesKilled,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(101)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumMartyrKills,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(102)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainingAccess,Data=(Type=SDT_Int32,Value1=2)))
	DefaultSettings(103)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainingNeedsViewed,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(104)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainingCompleted,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(105)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NavigationNeedsViewed,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(106)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PlaylistId,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(107)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PREFERRED_SPLIT_TYPE,Data=(Type=SDT_Int32,Value1=eSST_2P_HORIZONTAL)))
	DefaultSettings(108)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=PlayerSkill,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(109)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HardwareStatsUploaded,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(110)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=CAMP_MODE,Data=(Type=SDT_Int32,Value1=eGCM_LivePrivate)))
	DefaultSettings(111)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumChainsawDuelWins,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(112)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumMeatshieldsUsed,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(113)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumMulcherKills,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(114)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumMortarKills,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(115)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumStickyKills,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(116)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumKillsWithBoomShield,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(117)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumFlameKills,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(118)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumWingmanWins,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(119)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumMeatflagCaps,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(120)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumLeaderWins,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(121)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumKOTHWins,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(122)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumRoundsWon,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(123)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HordeWavesBeat1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(124)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=HordeWavesBeat2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(125)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChapterAccess1,Data=(Type=SDT_Int32,Value1=1)))
	DefaultSettings(126)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UnlockedChapterAccess2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(127)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapTutorialsOn,Data=(Type=SDT_Int32,Value1=WOOO_On)))
	DefaultSettings(128)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=WeapSwap_Shield,Data=(Type=SDT_Int32,Value1=eGEARWEAP_Shield)))
	DefaultSettings(129)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=SPTutorials3,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(130)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=MPTutorials3,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(131)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainTutorials1,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(132)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainTutorials2,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(133)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=TrainTutorials3,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(134)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumActiveReloads,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(135)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=NumTickerMelees,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(136)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=COMPLETED_ACHIEVEMENT_MASK_A,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(137)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=COMPLETED_ACHIEVEMENT_MASK_B,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(138)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UNVIEWED_ACHIEVEMENT_MASK_A,Data=(Type=SDT_Int32,Value1=0)))
	DefaultSettings(139)=(Owner=OPPO_Game,ProfileSetting=(PropertyId=UNVIEWED_ACHIEVEMENT_MASK_B,Data=(Type=SDT_Int32,Value1=0)))

	// These tell the UI what to display
	ProfileMappings(23)=(Id=HudOnOff,Name="Hud",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(24)=(Id=MatureLang,Name="Mature Language",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(25)=(Id=Subtitles,Name="Subtitles",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(26)=(Id=Gore,Name="Gore",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(27)=(Id=MusicVolume,Name="Music Volume",MappingType=PVMT_RawValue)
	ProfileMappings(28)=(Id=FxVolume,Name="FX Volume",MappingType=PVMT_RawValue)
	ProfileMappings(29)=(Id=DialogueVolume,Name="Dialogue Volume",MappingType=PVMT_RawValue)
	ProfileMappings(30)=(Id=GearStickConfig,Name="Stick Configuration",MappingType=PVMT_IdMapped,ValueMappings=((Id=WSCO_Default),(Id=WSCO_Legacy),(Id=WSCO_SouthPaw),(Id=WSCO_LegacySouthpaw)))
	ProfileMappings(31)=(Id=GearTriggerConfig,Name="Trigger Configuration",MappingType=PVMT_IdMapped,ValueMappings=((Id=WTCO_Default),(Id=WTCO_SouthPaw)))
	ProfileMappings(32)=(Id=CogMPCharacter,Name="Preferred COG Character",MappingType=PVMT_RawValue)
	ProfileMappings(33)=(Id=LocustMPCharacter,Name="Preferred Locust Character",MappingType=PVMT_RawValue)
	ProfileMappings(34)=(Id=GameIntensity,Name="Game Intensity",MappingType=PVMT_IdMapped,ValueMappings=((Id=DL_Casual),(Id=DL_Normal),(Id=DL_Hardcore),(Id=DL_Insane)))
	ProfileMappings(35)=(Id=PictogramTooltips,Name="PictogramTooltips",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(36)=(Id=GammaSetting,Name="GammaSetting",MappingType=PVMT_RawValue)
	ProfileMappings(37)=(Id=DefaultMPWeapon,Name="DefaultMPWeapon",MappingType=PVMT_RawValue)
	ProfileMappings(38)=(Id=GearControlScheme,Name="ControlScheme",MappingType=PVMT_IdMapped,ValueMappings=((Id=GCS_Default),(Id=GCS_Alternate)))
	ProfileMappings(39)=(Id=SelectedDeviceID,Name="SelectedDeviceID",MappingType=PVMT_RawValue)
	ProfileMappings(40)=(Id=TelevisionType,Name="TelevisionType",MappingType=PVMT_IdMapped,ValueMappings=((Id=TVT_Default),(Id=TVT_Soft),(Id=TVT_Lucent),(Id=TVT_Vibrant)))
	ProfileMappings(41)=(Id=VERSUS_GAMETYPE,Name="VersusGameType",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARMP_Warzone),(Id=eGEARMP_CTM),(Id=eGEARMP_Wingman),(Id=eGEARMP_CombatTrials),(Id=eGEARMP_Execution),(Id=eGEARMP_KTL),(Id=eGEARMP_Annex),(Id=eGEARMP_KOTH)))
	ProfileMappings(42)=(Id=VERSUS_MATCH_MODE,Name="VersusMatchMode",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGVMT_Official),(Id=eGVMT_Custom),(Id=eGVMT_SystemLink),(Id=eGVMT_Local)))
	ProfileMappings(43)=(Id=VERSUS_PARTY_TYPE,Name="VersusPartyType",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGVPT_InviteRequired),(Id=eGVPT_FriendsOnly),(Id=eGVPT_Public)))
	ProfileMappings(44)=(Id=VERSUS_SELECTED_MAP,Name="VersusSelectedMap",MappingType=PVMT_IdMapped,ValueMappings=((Id=CONTEXT_MAPNAME_FLOOD),(Id=CONTEXT_MAPNAME_GRIDLOCK),(Id=CONTEXT_MAPNAME_HAIL),(Id=CONTEXT_MAPNAME_WAYSTATION)))
	ProfileMappings(45)=(Id=MPFriendlyFire,Name="MPFriendlyFire",MappingType=PVMT_RawValue)
	ProfileMappings(46)=(Id=MPRoundTime,Name="MPRoundTime",MappingType=PVMT_RawValue)
	ProfileMappings(47)=(Id=MPBleedout,Name="MPBleedout",MappingType=PVMT_RawValue)
	ProfileMappings(48)=(Id=MPWeaponSwap,Name="MPWeaponSwap",MappingType=PVMT_RawValue)
	ProfileMappings(49)=(Id=MPRoundsToWin,Name="MPRoundsToWin",MappingType=PVMT_RawValue)
	ProfileMappings(50)=(Id=MPBotDiff,Name="MPBotDiff",MappingType=PVMT_RawValue)
	ProfileMappings(51)=(Id=AnnexRoundScore,Name="AnnexRoundScore",MappingType=PVMT_RawValue)
	ProfileMappings(52)=(Id=KOTHRoundScore,Name="KOTHRoundScore",MappingType=PVMT_RawValue)
	ProfileMappings(54)=(Id=HordeEnemyDiff,Name="HordeEnemyDiff",MappingType=PVMT_RawValue)
	ProfileMappings(55)=(Id=SPTutorials1,Name="SPTutorials1",MappingType=PVMT_RawValue)
	ProfileMappings(56)=(Id=SPTutorials2,Name="SPTutorials2",MappingType=PVMT_RawValue)
	ProfileMappings(57)=(Id=MPTutorials1,Name="MPTutorials1",MappingType=PVMT_RawValue)
	ProfileMappings(58)=(Id=MPTutorials2,Name="MPTutorials2",MappingType=PVMT_RawValue)
	ProfileMappings(59)=(Id=TurnInversion,Name="TurnInverion",MappingType=PVMT_IdMapped,ValueMappings=((Id=PXIO_Off),(Id=PXIO_On)))
	ProfileMappings(60)=(Id=TargetSensitivity,Name="TargetSensitivity",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCSO_Low),(Id=PCSO_Medium),(Id=PCSO_High)))
	ProfileMappings(61)=(Id=ZoomSensitivity,Name="ZoomSensitivity",MappingType=PVMT_IdMapped,ValueMappings=((Id=PCSO_Low),(Id=PCSO_Medium),(Id=PCSO_High)))
	ProfileMappings(62)=(Id=MPNumBots,Name="MPNumBots",MappingType=PVMT_RawValue)
	ProfileMappings(63)=(Id=HordeWave,Name="HordeWave",MappingType=PVMT_IdMapped,ValueMappings=((Id=CONTEXT_HORDE_WAVE_0),(Id=CONTEXT_HORDE_WAVE_1),(Id=CONTEXT_HORDE_WAVE_2),(Id=CONTEXT_HORDE_WAVE_3),(Id=CONTEXT_HORDE_WAVE_4),(Id=CONTEXT_HORDE_WAVE_5),(Id=CONTEXT_HORDE_WAVE_6),(Id=CONTEXT_HORDE_WAVE_7),(Id=CONTEXT_HORDE_WAVE_8),(Id=CONTEXT_HORDE_WAVE_9),(Id=CONTEXT_HORDE_WAVE_10),(Id=CONTEXT_HORDE_WAVE_11),(Id=CONTEXT_HORDE_WAVE_12),(Id=CONTEXT_HORDE_WAVE_13),(Id=CONTEXT_HORDE_WAVE_14),(Id=CONTEXT_HORDE_WAVE_15),(Id=CONTEXT_HORDE_WAVE_16),(Id=CONTEXT_HORDE_WAVE_17),(Id=CONTEXT_HORDE_WAVE_18),(Id=CONTEXT_HORDE_WAVE_19),(Id=CONTEXT_HORDE_WAVE_20),(Id=CONTEXT_HORDE_WAVE_21),(Id=CONTEXT_HORDE_WAVE_22),(Id=CONTEXT_HORDE_WAVE_23),(Id=CONTEXT_HORDE_WAVE_24),(Id=CONTEXT_HORDE_WAVE_25),(Id=CONTEXT_HORDE_WAVE_26),(Id=CONTEXT_HORDE_WAVE_27),(Id=CONTEXT_HORDE_WAVE_28),(Id=CONTEXT_HORDE_WAVE_29),(Id=CONTEXT_HORDE_WAVE_30),(Id=CONTEXT_HORDE_WAVE_31),(Id=CONTEXT_HORDE_WAVE_32),(Id=CONTEXT_HORDE_WAVE_33),(Id=CONTEXT_HORDE_WAVE_34),(Id=CONTEXT_HORDE_WAVE_35),(Id=CONTEXT_HORDE_WAVE_36),(Id=CONTEXT_HORDE_WAVE_37),(Id=CONTEXT_HORDE_WAVE_38),(Id=CONTEXT_HORDE_WAVE_39),(Id=CONTEXT_HORDE_WAVE_40),(Id=CONTEXT_HORDE_WAVE_41),(Id=CONTEXT_HORDE_WAVE_42),(Id=CONTEXT_HORDE_WAVE_43),(Id=CONTEXT_HORDE_WAVE_44),(Id=CONTEXT_HORDE_WAVE_45),(Id=CONTEXT_HORDE_WAVE_46),(Id=CONTEXT_HORDE_WAVE_47),(Id=CONTEXT_HORDE_WAVE_48),(Id=CONTEXT_HORDE_WAVE_49)))
	ProfileMappings(64)=(Id=DiscoverFound1,Name="DiscoverFound1",MappingType=PVMT_RawValue)
	ProfileMappings(65)=(Id=DiscoverFound2,Name="DiscoverFound2",MappingType=PVMT_RawValue)
	ProfileMappings(66)=(Id=DiscoverNeedsViewed1,Name="DiscoverNeedsViewed1",MappingType=PVMT_RawValue)
	ProfileMappings(67)=(Id=DiscoverNeedsViewed2,Name="DiscoverNeedsViewed2",MappingType=PVMT_RawValue)
	ProfileMappings(68)=(Id=MAP_SELECTION_MODE,Name="MapSelectionMode",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARMAPSELECT_VOTE),(Id=eGEARMAPSELECT_HOSTSELECT)))
	ProfileMappings(69)=(Id=WeapSwap_FragGrenade,Name="WeapSwap_FragGrenade",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(70)=(Id=WeapSwap_InkGrenade,Name="WeapSwap_InkGrenade",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(71)=(Id=WeapSwap_Boomshot,Name="WeapSwap_Boomshot",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(72)=(Id=WeapSwap_Flame,Name="WeapSwap_Flame",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(73)=(Id=WeapSwap_Sniper,Name="WeapSwap_Sniper",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(74)=(Id=WeapSwap_Bow,Name="WeapSwap_Bow",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(75)=(Id=WeapSwap_Mulcher,Name="WeapSwap_Mulcher",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(76)=(Id=WeapSwap_Mortar,Name="WeapSwap_Mortar",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(77)=(Id=WeapSwap_HOD,Name="WeapSwap_HOD",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(78)=(Id=WeapSwap_Gorgon,Name="WeapSwap_Gorgon",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(79)=(Id=WeapSwap_Boltok,Name="WeapSwap_Boltok",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(80)=(Id=WingmanScore,Name="WingmanScore",MappingType=PVMT_RawValue)
	ProfileMappings(81)=(Id=UnlockedChaptersCasual1,Name="UnlockedChaptersCasual1",MappingType=PVMT_RawValue)
	ProfileMappings(82)=(Id=UnlockedChaptersCasual2,Name="UnlockedChaptersCasual2",MappingType=PVMT_RawValue)
	ProfileMappings(83)=(Id=UnlockedChaptersNormal1,Name="UnlockedChaptersNormal1",MappingType=PVMT_RawValue)
	ProfileMappings(84)=(Id=UnlockedChaptersNormal2,Name="UnlockedChaptersNormal2",MappingType=PVMT_RawValue)
	ProfileMappings(85)=(Id=UnlockedChaptersHard1,Name="UnlockedChaptersHard1",MappingType=PVMT_RawValue)
	ProfileMappings(86)=(Id=UnlockedChaptersHard2,Name="UnlockedChaptersHard2",MappingType=PVMT_RawValue)
	ProfileMappings(87)=(Id=UnlockedChaptersInsane1,Name="UnlockedChaptersInsane1",MappingType=PVMT_RawValue)
	ProfileMappings(88)=(Id=UnlockedChaptersInsane2,Name="UnlockedChaptersInsane2",MappingType=PVMT_RawValue)
	ProfileMappings(89)=(Id=UnlockedUnlockables,Name="UnlockedUnlockables",MappingType=PVMT_RawValue)
	ProfileMappings(90)=(Id=UnlockableNeedsViewed,Name="UnlockableNeedsViewed",MappingType=PVMT_RawValue)
	ProfileMappings(91)=(Id=CoopInviteType,Name="CoopInviteType",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGCIT_InviteRequired),(Id=eGCIT_FriendsOnly),(Id=eGCIT_Public)))
	ProfileMappings(92)=(Id=CampCheckpointUsage,Name="CampCheckpointUsage",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARCHECKPOINT_UseLast),(Id=eGEARCHECKPOINT_Restart)))
	ProfileMappings(93)=(Id=LastCheckpointSlot,Name="LastCheckpointSlot",MappingType=PVMT_RawValue)
	ProfileMappings(94)=(Id=CoopCompletedChapters1,Name="CoopCompletedChapters1",MappingType=PVMT_RawValue)
	ProfileMappings(95)=(Id=CoopCompletedChapters2,Name="CoopCompletedChapters2",MappingType=PVMT_RawValue)
	ProfileMappings(96)=(Id=MPGameTypesCompleted,Name="MPGameTypesCompleted",MappingType=PVMT_RawValue)
	ProfileMappings(97)=(Id=MPMapsCompleted,Name="MPMapsCompleted",MappingType=PVMT_RawValue)
	ProfileMappings(98)=(Id=WeaponsKilledWith,Name="WeaponsKilledWith",MappingType=PVMT_RawValue)
	ProfileMappings(99)=(Id=ExecutionsCompleted,Name="ExecutionsCompleted",MappingType=PVMT_RawValue)
	ProfileMappings(100)=(Id=NumEnemiesKilled,Name="NumEnemiesKilled",MappingType=PVMT_RawValue)
	ProfileMappings(101)=(Id=NumMartyrKills,Name="NumMartyrKills",MappingType=PVMT_RawValue)
	ProfileMappings(102)=(Id=TrainingAccess,Name="TrainingAccess",MappingType=PVMT_RawValue)
	ProfileMappings(103)=(Id=TrainingNeedsViewed,Name="TrainingNeedsViewed",MappingType=PVMT_RawValue)
	ProfileMappings(104)=(Id=TrainingCompleted,Name="TrainingCompleted",MappingType=PVMT_RawValue)
	ProfileMappings(105)=(Id=NavigationNeedsViewed,Name="NavigationNeedsViewed",MappingType=PVMT_RawValue)
	ProfileMappings(106)=(Id=PlaylistId,Name="PlaylistId",MappingType=PVMT_RawValue)
	ProfileMappings(107)=(Id=PREFERRED_SPLIT_TYPE,Name="PreferredSplitscreenType",MappingType=PVMT_IdMapped,ValueMappings=((Id=eSST_2P_HORIZONTAL),(Id=eSST_2P_VERTICAL)))
	ProfileMappings(108)=(Id=PlayerSkill,Name="PlayerSkill",MappingType=PVMT_RawValue)
	ProfileMappings(109)=(Id=HardwareStatsUploaded,Name="HardwareStatsUploaded",MappingType=PVMT_RawValue)
	ProfileMappings(110)=(Id=CAMP_MODE,Name="CampaignMode",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGCM_LivePublic),(Id=eGCM_LivePrivate),(Id=eGCM_SystemLink)))
	ProfileMappings(111)=(Id=NumChainsawDuelWins,Name="NumChainsawDuelWins",MappingType=PVMT_RawValue)
	ProfileMappings(112)=(Id=NumMeatshieldsUsed,Name="NumMeatshieldsUsed",MappingType=PVMT_RawValue)
	ProfileMappings(113)=(Id=NumMulcherKills,Name="NumMulcherKills",MappingType=PVMT_RawValue)
	ProfileMappings(114)=(Id=NumMortarKills,Name="NumMortarKills",MappingType=PVMT_RawValue)
	ProfileMappings(115)=(Id=NumStickyKills,Name="NumStickyKills",MappingType=PVMT_RawValue)
	ProfileMappings(116)=(Id=NumKillsWithBoomShield,Name="NumKillsWithBoomShield",MappingType=PVMT_RawValue)
	ProfileMappings(117)=(Id=NumFlameKills,Name="NumFlameKills",MappingType=PVMT_RawValue)
	ProfileMappings(118)=(Id=NumWingmanWins,Name="NumWingmanWins",MappingType=PVMT_RawValue)
	ProfileMappings(119)=(Id=NumMeatflagCaps,Name="NumMeatflagCaps",MappingType=PVMT_RawValue)
	ProfileMappings(120)=(Id=NumLeaderWins,Name="NumLeaderWins",MappingType=PVMT_RawValue)
	ProfileMappings(121)=(Id=NumKOTHWins,Name="NumKOTHWins",MappingType=PVMT_RawValue)
	ProfileMappings(122)=(Id=NumRoundsWon,Name="NumRoundsWon",MappingType=PVMT_RawValue)
	ProfileMappings(123)=(Id=HordeWavesBeat1,Name="HordeWavesBeat1",MappingType=PVMT_RawValue)
	ProfileMappings(124)=(Id=HordeWavesBeat2,Name="HordeWavesBeat2",MappingType=PVMT_RawValue)
	ProfileMappings(125)=(Id=UnlockedChapterAccess1,Name="UnlockedChapterAccess1",MappingType=PVMT_RawValue)
	ProfileMappings(126)=(Id=UnlockedChapterAccess2,Name="UnlockedChapterAccess2",MappingType=PVMT_RawValue)
	ProfileMappings(127)=(Id=WeapTutorialsOn,Name="WeapTutorialsOn",MappingType=PVMT_IdMapped,ValueMappings=((Id=WOOO_On),(Id=WOOO_Off)))
	ProfileMappings(128)=(Id=WeapSwap_Shield,Name="WeapSwap_Shield",MappingType=PVMT_IdMapped,ValueMappings=((Id=eGEARWEAP_FragGrenade),(Id=eGEARWEAP_InkGrenade),(Id=eGEARWEAP_SmokeGrenade),(Id=eGEARWEAP_Boomshot),(Id=eGEARWEAP_Flame),(Id=eGEARWEAP_Sniper),(Id=eGEARWEAP_Bow),(Id=eGEARWEAP_Mulcher),(Id=eGEARWEAP_Mortar),(Id=eGEARWEAP_HOD),(Id=eGEARWEAP_Gorgon),(Id=eGEARWEAP_Boltok),(Id=eGEARWEAP_Pistol),(Id=eGEARWEAP_Lancer),(Id=eGEARWEAP_Hammerburst),(Id=eGEARWEAP_Shotgun),(Id=eGEARWEAP_Shield),(Id=eGEARWEAP_Disabled)))
	ProfileMappings(129)=(Id=SPTutorials3,Name="SPTutorials3",MappingType=PVMT_RawValue)
	ProfileMappings(130)=(Id=MPTutorials3,Name="MPTutorials3",MappingType=PVMT_RawValue)
	ProfileMappings(131)=(Id=TrainTutorials1,Name="TrainTutorials1",MappingType=PVMT_RawValue)
	ProfileMappings(132)=(Id=TrainTutorials2,Name="TrainTutorials2",MappingType=PVMT_RawValue)
	ProfileMappings(133)=(Id=TrainTutorials3,Name="TrainTutorials3",MappingType=PVMT_RawValue)
	ProfileMappings(134)=(Id=NumActiveReloads,Name="NumActiveReloads",MappingType=PVMT_RawValue)
	ProfileMappings(135)=(Id=NumTickerMelees,Name="NumTickerMelees",MappingType=PVMT_RawValue)
	ProfileMappings(136)=(Id=COMPLETED_ACHIEVEMENT_MASK_A,Name="CompletedAchievementMask1",MappingType=PVMT_RawValue)
	ProfileMappings(137)=(Id=COMPLETED_ACHIEVEMENT_MASK_B,Name="CompletedAchievementMask2",MappingType=PVMT_RawValue)
	ProfileMappings(138)=(Id=UNVIEWED_ACHIEVEMENT_MASK_A,Name="UnviewedAchievementMask1",MappingType=PVMT_RawValue)
	ProfileMappings(139)=(Id=UNVIEWED_ACHIEVEMENT_MASK_B,Name="UnviewedAchievementMask2",MappingType=PVMT_RawValue)
}
