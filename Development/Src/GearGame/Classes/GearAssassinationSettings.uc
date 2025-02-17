/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * Holds the base configuration settings for Assassination match.
 * NOTE: This class will normally be code generated
 */
class GearAssassinationSettings extends GearVersusGameSettings;

defaultproperties
{
	// Assassination is the sub-game mode (const)
	LocalizedSettings(1)=(Id=CONTEXT_VERSUSMODES,ValueIndex=eGEARMP_KTL,AdvertisementType=ODAT_OnlineService)

	// Assassination specific default values for inherited settings
	LocalizedSettings(7)=(Id=CONTEXT_ROUNDSTOWIN,ValueIndex=CONTEXT_ROUNDSTOWIN_THREE,AdvertisementType=ODAT_DontAdvertise)
}
