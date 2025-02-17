/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for Meatflag match.
 * NOTE: This class will normally be code generated
 */
class GearMeatflagSettings extends GearVersusGameSettings;

defaultproperties
{
	// Meatflag is the sub-game mode (const)
	LocalizedSettings(1)=(Id=CONTEXT_VERSUSMODES,ValueIndex=eGEARMP_CTM,AdvertisementType=ODAT_OnlineService)

	// Meatflag specific default values for inherited settings
	LocalizedSettings(7)=(Id=CONTEXT_ROUNDSTOWIN,ValueIndex=CONTEXT_ROUNDSTOWIN_TWO,AdvertisementType=ODAT_DontAdvertise)

	/************************************************************************/
	/* NOTE: VERY IMPORTANT!!! MUST START INDEXING AT THE END OF THE		*/
	/*       SUPERCLASS LIST!!!!											*/
	/************************************************************************/
	// Meatflag settings that are only configurable in private matches
	LocalizedSettings.Add((Id=CONTEXT_MEATFLAGDIFFICULTY,ValueIndex=CONTEXT_AIDIFFICULTY_NORMAL,AdvertisementType=ODAT_DontAdvertise))
	LocalizedSettingsMappings.Add({
		(Id=CONTEXT_MEATFLAGDIFFICULTY,Name="MeatflagDifficulty",ValueMappings=(
			(Id=CONTEXT_AIDIFFICULTY_CASUAL),
			(Id=CONTEXT_AIDIFFICULTY_NORMAL),
			(Id=CONTEXT_AIDIFFICULTY_HARDCORE),
			(Id=CONTEXT_AIDIFFICULTY_INSANE)))
		})
}
