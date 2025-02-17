/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for KOTH match.
 * NOTE: This class will normally be code generated
 */
class GearKOTHSettings extends GearVersusGameSettings;

defaultproperties
{
	// KOTH is the sub-game mode (const)
	LocalizedSettings(1)=(Id=CONTEXT_VERSUSMODES,ValueIndex=eGEARMP_KOTH,AdvertisementType=ODAT_OnlineService)

	// KOTH specific default values for inherited settings
	LocalizedSettings(7)=(Id=CONTEXT_ROUNDSTOWIN,ValueIndex=CONTEXT_ROUNDSTOWIN_TWO,AdvertisementType=ODAT_DontAdvertise)

	/************************************************************************/
	/* NOTE: VERY IMPORTANT!!! MUST START INDEXING AT THE END OF THE		*/
	/*       SUPERCLASS LIST!!!!											*/
	/************************************************************************/
	// KOTH settings that are only configurable in private matches
	LocalizedSettings.Add((Id=CONTEXT_ROUNDSCORE,ValueIndex=CONTEXT_ROUNDSCORE_KOTH_120,AdvertisementType=ODAT_DontAdvertise))
	LocalizedSettingsMappings.Add({
		(Id=CONTEXT_ROUNDSCORE,Name="RoundScore",ValueMappings=(
			(Id=CONTEXT_ROUNDSCORE_KOTH_60),
			(Id=CONTEXT_ROUNDSCORE_KOTH_90),
			(Id=CONTEXT_ROUNDSCORE_KOTH_120),
			(Id=CONTEXT_ROUNDSCORE_KOTH_150),
			(Id=CONTEXT_ROUNDSCORE_KOTH_180),
			(Id=CONTEXT_ROUNDSCORE_KOTH_210),
			(Id=CONTEXT_ROUNDSCORE_KOTH_240)))
		})
}
