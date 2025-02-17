//=============================================================================
// Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
// Confidential.
//=============================================================================

#include "GearGame.h"

/**
 * Sets all of the profile settings to their default values and then does
 * a check for your language setting and swaps the subtitle setting based
 * off of that.
 */
void UGearProfileSettings::SetToDefaults(void)
{
	Super::SetToDefaults();
#if _XBOX
	// Replace the subtitle setting with always on for these
	FOnlineProfileSetting* Setting = FindSetting(UCONST_Subtitles);
	if (Setting != NULL)
	{
		// Check for Chinese or Korean
		DWORD Lang = XGetLanguage();
		switch (Lang)
		{
			case XC_LANGUAGE_ENGLISH:
			case XC_LOCALE_CZECH_REPUBLIC:
			case XC_LOCALE_SLOVAK_REPUBLIC:
			case XC_LOCALE_HUNGARY:
			case XC_LANGUAGE_POLISH:
			case XC_LANGUAGE_RUSSIAN:
			case XC_LANGUAGE_SCHINESE:
			case XC_LANGUAGE_TCHINESE:
			case XC_LANGUAGE_KOREAN:
				debugf(TEXT("Auto-enabling subtitles for the languages that default them to on"));
				// Force to on
				Setting->ProfileSetting.Data.SetData((INT)WOOO_On);
				break;
			default:
				// Default all others to off
				Setting->ProfileSetting.Data.SetData((INT)WOOO_Off);
				break;
		}
	}
#endif
}
