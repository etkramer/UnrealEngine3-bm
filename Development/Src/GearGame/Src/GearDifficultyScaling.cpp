/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


#include "GearGame.h"



IMPLEMENT_CLASS(USeqAct_ModifyDifficulty);


void USeqAct_ModifyDifficulty::Activated()
{
	Super::Activated();
}


void USeqAct_ModifyDifficulty::DeActivated()
{
	Super::DeActivated();
}


void USeqAct_ModifyDifficulty::PostEditChange( UProperty* PropertyThatChanged )
{
	if( PropertyThatChanged != NULL ) 
	{
		// Rating can only only be between min and max values allowed
		AccuracyRating	= Clamp( AccuracyRating, -2, 2 );
		AcquisitionTimeRating	= Clamp( AcquisitionTimeRating, -2, 2 );
		HealthRating 	= Clamp( HealthRating, -2, 2 );
	}

	Super::PostEditChange(PropertyThatChanged);
}


FLOAT DetermineMultiplierHelper( INT RatingToSwitchOn, const FAttributeRatingStruct& RatingStruct )
{
	FLOAT Retval = 0.0f;

	if( RatingStruct.Values.Num() != 5 )
	{
		warnf( TEXT( "Passed in RatingStruct does not have 5 entries.  Using default value of 1.0f" ));
		return 1.0f;
	}


	switch( RatingToSwitchOn )
	{
	case -2:
		{
			Retval = RatingStruct.Values(0);
			break;
		}

	case -1:
		{
			Retval = RatingStruct.Values(1);
			break;
		}

	case 0:
		{
			Retval = RatingStruct.Values(2);
			break;
		}

	case 1:
		{
			Retval = RatingStruct.Values(3);
			break;
		}

	case 2:
		{
			Retval = RatingStruct.Values(4);
			break;
		}

	default:
		{
			Retval = 1.00f;
			break;
		}
	}

	return Retval;
}



FLOAT USeqAct_ModifyDifficulty::DetermineAccuracyMultiplier() const
{
	FLOAT Retval = 0.0f;

	Retval = DetermineMultiplierHelper( AccuracyRating, AcurracyMultiplierValues );

	return Retval;
}


FLOAT USeqAct_ModifyDifficulty::DetermineAcquisitionTimeMultiplier() const
{
	FLOAT Retval = 0.0f;

	Retval = DetermineMultiplierHelper( AcquisitionTimeRating, AcquisitionTimeMultiplierValues );

	return Retval;
}


FLOAT USeqAct_ModifyDifficulty::DetermineHealthMultiplier() const
{
	FLOAT Retval = 0.0f;

	Retval = DetermineMultiplierHelper( HealthRating, HealthMultiplierValues );

	return Retval;
}




#include "FConfigCacheIni.h"

namespace 
{
	void DifficultySetter_Helper( const TCHAR* IniPrefix, const FName ConfigName, const FString& DifficultyLevel )
	{
		// this will generate the correct .ini from the Defaults of that difficulty level

		FString ConfigNameToReplace = appGameConfigDir() + FString(IniPrefix) + FString(GGameName) + FString::Printf( TEXT( "%s.ini"), *ConfigName.ToString() );
		FString ConfigFilenameGenerated = appGameConfigDir() + FString(GGameName) + FString::Printf( TEXT( "%s%s.ini"), *ConfigName.ToString(), *DifficultyLevel );

		warnf( TEXT( "DifficultySetter_Helper ConfigNameToReplace: %s ConfigFilenameGenerated: %s "), *ConfigNameToReplace, *ConfigFilenameGenerated );

		if( GUseSeekFreeLoading && CONSOLE )
		{
			// replace the base one (ie GearAI.ini) with particular one (ie GearAIEasy.ini)
			FConfigFile* ExistingConfigFile = GConfig->FindConfigFile(*ConfigFilenameGenerated);
			check(ExistingConfigFile);
			GConfig->SetFile(*ConfigNameToReplace, ExistingConfigFile);
		}
		else
		{
			FString ConfigFilenameDefault = appGameConfigDir() + FString::Printf( TEXT( "Default%s%s.ini"), *ConfigName.ToString(), *DifficultyLevel );
			UINT YesToAll = ART_YesAll;
			// this will generate the new ini
			const UBOOL bTryToPreserveContents = FALSE;
			appCheckIniForOutdatedness(*ConfigFilenameGenerated,*ConfigFilenameDefault,bTryToPreserveContents,YesToAll);
			// load the new ini
			FConfigFile GeneratedIniFile;
			LoadAnIniFile(*ConfigFilenameGenerated,GeneratedIniFile,FALSE);
			// make sure it doesn't get saved
			GeneratedIniFile.NoSave = TRUE;
			// and replace the default ini 
			GConfig->SetFile(*ConfigNameToReplace,&GeneratedIniFile);
		}

		// so now that we have updated the "default" data we need to update all of our classes to use that
		// new difficulty based data
		for( FObjectIterator It; It; ++It )
		{
			UObject* Object = *It;
			if( Object->GetClass()->ClassConfigName == ConfigName )
			{
				Object->ReloadConfig();
				Object->ReloadLocalized();
			}
		}

	}


	void DifficultySetter( const FString& DifficultyLevel )
	{
		const FName ConfigName_AI = TEXT("AI");
		//const FName ConfigName_Game = TEXT("Game");
		const FName ConfigName_Pawn = TEXT("Pawn");
		const FName ConfigName_Weapon = TEXT("Weapon");

		debugf( TEXT( "GGameIni: %s, DifficultyLevel: %s" ), GGameIni, *DifficultyLevel );  // GGameIni: ..\GearGame\Config\Xe-GearGame.ini
		//GConfig->Dump( *GLog );
		DifficultySetter_Helper( TEXT(""), ConfigName_AI, DifficultyLevel );
		DifficultySetter_Helper( TEXT(""), ConfigName_Pawn, DifficultyLevel );
		DifficultySetter_Helper( TEXT(""), ConfigName_Weapon, DifficultyLevel );
		//GConfig->Dump( *GLog );
	}

} // anon namespace

/**
 * @todo:  need to pass in an Enum here for the the TYPE of game we are loading so we can have
 *         specific ini files for each game:  Omega Man, TeamDeathMatch, BoomerMatch, etc.
 **/
void AGearGame::LoadGameTypeConfigs( const FString& GameTypeExtension )
{
	debugf(TEXT("Loading game type configs: %s"),*GameTypeExtension);
	const FName ConfigName_Weapon = TEXT("Weapon");
	DifficultySetter_Helper( TEXT(""), ConfigName_Weapon, *GameTypeExtension );

	const FName ConfigName_Pawn = TEXT("Pawn");
	DifficultySetter_Helper( TEXT(""), ConfigName_Pawn, *GameTypeExtension );
}
