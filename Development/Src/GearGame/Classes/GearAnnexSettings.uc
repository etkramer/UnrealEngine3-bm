/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for Annex match.
 * NOTE: This class will normally be code generated
 */
class GearAnnexSettings extends GearVersusGameSettings;

defaultproperties
{
	// Annex is the sub-game mode (const)
	LocalizedSettings(1)=(Id=CONTEXT_VERSUSMODES,ValueIndex=eGEARMP_Annex,AdvertisementType=ODAT_OnlineService)

	// Annex specific default values for inherited settings
	LocalizedSettings(7)=(Id=CONTEXT_ROUNDSTOWIN,ValueIndex=CONTEXT_ROUNDSTOWIN_TWO,AdvertisementType=ODAT_DontAdvertise)

	/************************************************************************/
	/* NOTE: VERY IMPORTANT!!! MUST START INDEXING AT THE END OF THE		*/
	/*       SUPERCLASS LIST!!!!											*/
	/************************************************************************/
	// Annex settings that are only configurable in private matches
	LocalizedSettings.Add((Id=CONTEXT_ROUNDSCORE,ValueIndex=CONTEXT_ROUNDSCORE_ANNEX_180,AdvertisementType=ODAT_DontAdvertise))
	LocalizedSettingsMappings.Add({
		(Id=CONTEXT_ROUNDSCORE,Name="RoundScore",ValueMappings=(
			(Id=CONTEXT_ROUNDSCORE_ANNEX_120),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_180),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_240),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_300),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_360),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_420),
			(Id=CONTEXT_ROUNDSCORE_ANNEX_480)))
		})
}
