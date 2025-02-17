/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "GearGame.h"

#include "GearGameSequenceClasses.h"
#include "GearGameVehicleClasses.h"
#include "GearGameWeaponClasses.h"
#include "GearGameUIClasses.h"
#include "GearGameUIPrivateClasses.h"

/************************************************************************/
/* Base class of all GearPlayerInputs                                   */
/************************************************************************/
IMPLEMENT_CLASS(UGearPlayerInput_Base)

/************************************************************************/
/* UGearPlayerInput                                                     */
/************************************************************************/
IMPLEMENT_CLASS(UGearPlayerInput)

/** Will return the BindName based on the BindCommand */
FString UGearPlayerInput::GetGearBindNameFromCommand(const FString& BindCommand)
{
	FString NameSearch = TEXT("");
	FString CommandToFind = BindCommand;
#if XBOX
	UBOOL bGamepad = TRUE;
#else
	UBOOL bGamepad = bUsingGamepad;
#endif

	// Get the bind command using the Mapped FieldName as the key
	if ( CommandToFind.Len() > 0 )
	{
		// We have a potential 2nd pass check in case we are looking for a gamepad binding
		// and we didn't find one.  The reason is that we may have appeneded "_Gamepad" to the end for
		// the special cases where a bind was modified to work special on the gamepad.
		INT NumAttempts = bGamepad ? 2 : 1;
		for ( INT AttemptIndex = 0; AttemptIndex < NumAttempts; AttemptIndex++ )
		{
			INT BindIndex = -1;

			// If this is the 2nd attempt, try appending "_Flip" for the crazy axis switching
			if ( AttemptIndex > 0 )
			{
				CommandToFind += TEXT("_Flip");
			}

			// Loop for bind names until we get a match that is dependent on whether a controller or keyboard/mouse are used.
			do 
			{
				// Get bind name
				NameSearch = GetBindNameFromCommand( CommandToFind, &BindIndex );
				// See if it starts with the controller prefix.
				if ( NameSearch.StartsWith(TEXT("XboxTypeS")) )
				{
					// Is a controller prefix so if we are using the gamepad break and return this bind name.
					if ( bGamepad )
					{
						break;
					}
				}
				else
				{
					// Is not a controller prefix so if we are not using the gamepad break and return this bind name.
					if ( !bGamepad )
					{
						break;
					}
				}

				// Decrement the index.
				BindIndex--;

			} while( BindIndex >= 0 );

			// If we found a match break out.
			if ( NameSearch.Len() > 0 )
			{
				break;
			}
		}
	}

	return NameSearch;
}


/************************************************************************/
/* UGearGameSettings                                                    */
/************************************************************************/

IMPLEMENT_CLASS(UGearGameSettings)

void UGearGameSettings::LoadSceneValues( UUIScene* Scene )
{
	TArray<UUIObject*> SceneChildren = Scene->GetChildren(TRUE);
	for ( INT ChildIndex = 0; ChildIndex < SceneChildren.Num(); ChildIndex++ )
	{
		LoadWidgetValue(SceneChildren(ChildIndex));
	}
}

void UGearGameSettings::SaveSceneValues( UUIScene* Scene )
{
	UBOOL bNeedsSave = FALSE;

	TArray<UUIObject*> SceneChildren = Scene->GetChildren(TRUE);
	for ( INT ChildIndex = 0; ChildIndex < SceneChildren.Num(); ChildIndex++ )
	{
		if ( SaveWidgetValue(SceneChildren(ChildIndex)) )
		{
			bNeedsSave = TRUE;
		}
	}

	if ( bNeedsSave )
	{
		SaveConfig();
	}
}

UBOOL UGearGameSettings::GetWidgetIntSetting( class UUIObject* Widget, FSettingInfo_Int** IntSetting )
{
	UBOOL bResult = FALSE;

	if ( Widget->WidgetTag == Gore.WidgetName )
	{
		*IntSetting = &Gore;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == DisplayDevice.WidgetName )
	{
		*IntSetting = &DisplayDevice;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == Splitscreen.WidgetName )
	{
		*IntSetting = &Splitscreen;
		bResult = TRUE;
	}

	return bResult;
}

UBOOL UGearGameSettings::GetWidgetBoolSetting( class UUIObject* Widget, FSettingInfo_Bool** BoolSetting )
{
	UBOOL bResult = FALSE;

	if ( Widget->WidgetTag ==LanguageFilter.WidgetName )
	{
		*BoolSetting = &LanguageFilter;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == ClosedCaptioning.WidgetName )
	{
		*BoolSetting = &ClosedCaptioning;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == InvertLook.WidgetName )
	{
		*BoolSetting = &InvertLook;
		bResult = TRUE;
	}

	return bResult;
}

UBOOL UGearGameSettings::GetWidgetFloatSetting( class UUIObject* Widget, FSettingInfo_Float** FloatSetting )
{
	UBOOL bResult = FALSE;

	if ( Widget->WidgetTag == Brightness.WidgetName )
	{
		*FloatSetting = &Brightness;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == MusicVolume.WidgetName )
	{
		*FloatSetting = &MusicVolume;
		bResult = TRUE;
	}
	else if ( Widget->WidgetTag == EffectsVolume.WidgetName )
	{
		*FloatSetting = &EffectsVolume;
		bResult = TRUE;
	}

	return bResult;
}


void UGearGameSettings::LoadWidgetValue( UUIObject* Widget )
{
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;
	FSettingInfo_Float* FloatSetting = NULL;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		LoadIntValue(*IntSetting, Cast<UUILabelButton>(Widget));
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		LoadBoolValue(*BoolSetting, Cast<UUILabelButton>(Widget));
	}
	else if ( GetWidgetFloatSetting(Widget, &FloatSetting) )
	{
		LoadFloatValue(*FloatSetting, Cast<UUISlider>(Widget));
	}
}

UBOOL UGearGameSettings::SaveWidgetValue( UUIObject* Widget )
{
	UBOOL bResult = FALSE;
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;
	FSettingInfo_Float* FloatSetting = NULL;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		SaveIntValue(*IntSetting, Cast<UUILabelButton>(Widget));
		bResult = TRUE;
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		SaveBoolValue(*BoolSetting, Cast<UUILabelButton>(Widget));
		bResult = TRUE;
	}
	else if ( GetWidgetFloatSetting(Widget, &FloatSetting) )
	{
		SaveFloatValue(*FloatSetting, Cast<UUISlider>(Widget));
		bResult = TRUE;
	}

	return bResult;
}

void UGearGameSettings::GetNextValue( UUIObject* Widget )
{
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString CurrentValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			INT Index = IntSetting->ValuesDesc.FindItemIndex(CurrentValue);

			if ( Index == INDEX_NONE )
			{
				debugf(TEXT("CurrentValue '%s' not found for setting assigned to widget:%s"), *CurrentValue, *Widget->GetName());
			}
			else
			{
				if ( Index < IntSetting->ValuesDesc.Num() - 1 )
				{
					Index++;
					LabelButtonWidget->SetDataStoreBinding(IntSetting->ValuesDesc(Index));
				}
			}
		}
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			if ( StringValue == BoolSetting->ValuesDesc[0] )
			{
				LabelButtonWidget->SetDataStoreBinding(BoolSetting->ValuesDesc[1]);
			}
		}
	}
}

void UGearGameSettings::GetPrevValue( UUIObject* Widget )
{
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString CurrentValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			INT Index = IntSetting->ValuesDesc.FindItemIndex(CurrentValue);

			if ( Index == INDEX_NONE )
			{
				debugf(TEXT("CurrentValue '%s' not found for setting assigned to widget:%s"), *CurrentValue, *Widget->GetName());
			}
			else
			{
				if ( Index > 0 )
				{
					Index--;
					LabelButtonWidget->SetDataStoreBinding(IntSetting->ValuesDesc(Index));
				}
			}
		}
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			if ( StringValue == BoolSetting->ValuesDesc[1] )
			{
				LabelButtonWidget->SetDataStoreBinding(BoolSetting->ValuesDesc[0]);
			}
		}
	}
}

void UGearGameSettings::LoadIntValue( FSettingInfo_Int& Setting, UUILabelButton* LabelButtonWidget )
{
	if ( LabelButtonWidget != NULL )
	{
		FString Value;
		if ( Setting.ValuesDesc.IsValidIndex(Setting.Value) )
		{
			Value = Setting.ValuesDesc(Setting.Value);
		}
		else if ( Setting.PotentialValues.Num() > 0 )
		{
			Value = Setting.ValuesDesc(0);
		}

		if ( Value.Len() == 0 )
		{
			Value = TEXT("INVALID");
		}

		LabelButtonWidget->SetDataStoreBinding(Value);
	}
}

void UGearGameSettings::LoadFloatValue( FSettingInfo_Float& Setting, UUISlider* SliderWidget )
{
	if ( SliderWidget != NULL )
	{
		SliderWidget->SliderValue.MinValue = Setting.MinValue;
		SliderWidget->SliderValue.MaxValue = Setting.MaxValue;
		SliderWidget->SetValue(Setting.Value);
	}
}

void UGearGameSettings::LoadBoolValue( FSettingInfo_Bool& Setting, UUILabelButton* LabelButtonWidget )
{
	if ( LabelButtonWidget != NULL )
	{
		FString Value = Setting.ValuesDesc[Setting.bValue];
		LabelButtonWidget->SetDataStoreBinding(Value);
	}
}

void UGearGameSettings::SaveIntValue( FSettingInfo_Int& Setting, UUILabelButton* LabelButtonWidget )
{
	if ( LabelButtonWidget != NULL )
	{
		FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);

		INT ValueIndex = Setting.ValuesDesc.FindItemIndex(StringValue);
		if ( Setting.PotentialValues.IsValidIndex(ValueIndex) )
		{
			Setting.Value = ValueIndex;
		}
	}
}

void UGearGameSettings::SaveFloatValue( FSettingInfo_Float& Setting, UUISlider* SliderWidget )
{
	if ( SliderWidget != NULL )
	{
		Setting.Value = SliderWidget->GetValue();
	}
}

void UGearGameSettings::SaveBoolValue( FSettingInfo_Bool& Setting, UUILabelButton* LabelButtonWidget )
{
	if ( LabelButtonWidget != NULL )
	{
		FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
		if ( StringValue == Setting.ValuesDesc[0] )
		{
			Setting.bValue = FALSE;
		}
		else
		{
			Setting.bValue = TRUE;
		}
	}
}

void UGearGameSettings::SaveInvertLook( UBOOL bLook )
{
	InvertLook.bValue = bLook;
	SaveConfig();
}


/** @return Whether or not the specified widget can navigate right. */
UBOOL UGearGameSettings::HasNextValue( class UUIObject* Widget )
{
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;
	UBOOL bResult = TRUE;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString CurrentValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			INT Index = IntSetting->ValuesDesc.FindItemIndex(CurrentValue);

			if ( Index == INDEX_NONE )
			{
				debugf(TEXT("CurrentValue '%s' not found for setting assigned to widget:%s"), *CurrentValue, *Widget->GetName());
			}
			else
			{

				bResult = Index < (IntSetting->ValuesDesc.Num() - 1);
			}
		}
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			if ( StringValue == BoolSetting->ValuesDesc[0] )
			{
				bResult = TRUE;
			}
			else
			{
				bResult = FALSE;
			}
		}
	}

	return bResult;
}

/** @return Whether or not the specified widget can navigate left. */
UBOOL UGearGameSettings::HasPrevValue( class UUIObject* Widget )
{
	FSettingInfo_Int* IntSetting=NULL;
	FSettingInfo_Bool* BoolSetting = NULL;
	UBOOL bResult = TRUE;

	if ( GetWidgetIntSetting(Widget, &IntSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString CurrentValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			INT Index = IntSetting->ValuesDesc.FindItemIndex(CurrentValue);

			if ( Index == INDEX_NONE )
			{
				debugf(TEXT("CurrentValue '%s' not found for setting assigned to widget:%s"), *CurrentValue, *Widget->GetName());
			}
			else
			{

				bResult = Index > 0;
			}
		}
	}
	else if ( GetWidgetBoolSetting(Widget, &BoolSetting) )
	{
		UUILabelButton* LabelButtonWidget = Cast<UUILabelButton>(Widget);
		if ( LabelButtonWidget != NULL )
		{
			FString StringValue = LabelButtonWidget->StringRenderComponent->GetValue(TRUE);
			if ( StringValue == BoolSetting->ValuesDesc[1] )
			{
				bResult = TRUE;
			}
			else
			{
				bResult = FALSE;
			}
		}
	}

	return bResult;
}


/************************************************************************/
/* UGearProfileSettings                                                 */
/************************************************************************/

IMPLEMENT_CLASS(UGearProfileSettings)

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then returns a array of value mappings.
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Value the out param that gets the value copied to it
 *
 * @return true if found, false otherwise
 */
UBOOL UGearProfileSettings::GetProfileSettingMappings(INT ProfileSettingId, TArray<FIdToStringMapping> &OutArray)
{
	UBOOL bResult = FALSE;

	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then find the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						OutArray = MetaData.ValueMappings;
						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

/**
 * Finds the human readable name for a profile setting's value. Searches the
 * profile settings mappings for the specifc profile setting and then returns the index of the currently selected value.
 *
 * @param ProfileSettingId the id to look up in the mappings table
 * @param Value the out param that gets the index copied ino it.
 *
 * @return true if found, false otherwise
 */
UBOOL UGearProfileSettings::GetProfileSettingValueMappingIndex(INT ProfileSettingId, INT &OutIndex)
{
	UBOOL bResult = FALSE;

	// Search for the profile setting id in the mappings
	for (INT Index = 0; Index < ProfileMappings.Num(); Index++)
	{
		FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(Index);
		if (MetaData.Id == ProfileSettingId)
		{
			// Find the profile setting that matches this id
			for (INT Index2 = 0; Index2 < ProfileSettings.Num(); Index2++)
			{
				FOnlineProfileSetting& Setting = ProfileSettings(Index2);
				if (Setting.ProfileSetting.PropertyId == ProfileSettingId)
				{
					// If this is ID mapped, then find the ID
					if (MetaData.MappingType == PVMT_IdMapped)
					{
						INT ValueIndex;

						// Read the index so we can find its name
						Setting.ProfileSetting.Data.GetData(ValueIndex);

						// Now search for the value index mapping
						for (INT Index3 = 0; Index3 < MetaData.ValueMappings.Num(); Index3++)
						{
							const FIdToStringMapping& Mapping = MetaData.ValueMappings(Index3);
							if (Mapping.Id == ValueIndex)
							{
								OutIndex = Index3;
								bResult = TRUE;
							}
						}
					}
				}
			}
		}
	}	

	return bResult;
}

/**
 * Removes any gametypes which were configured in the .ini to be disabled from the ProfileMappings array of the VERSUS_GAMETYPE setting.
 */
void UGearProfileSettings::RemoveExcludedGameTypes()
{
	INT GameTypeMappingIndex = FindProfileMappingIndex(UCONST_VERSUS_GAMETYPE);
	if ( GameTypeMappingIndex != INDEX_NONE )
	{
		FSettingsPropertyPropertyMetaData& MetaData = ProfileMappings(GameTypeMappingIndex);
		for ( INT ExclusionIdx = 0; ExclusionIdx < ExcludedGameTypeIds.Num(); ExclusionIdx++ )
		{
			EGearMPTypes MPType = static_cast<EGearMPTypes>(ExcludedGameTypeIds(ExclusionIdx));
			for ( INT ValueIndex = MetaData.ValueMappings.Num() - 1; ValueIndex >= 0; ValueIndex-- )
			{
				FIdToStringMapping& ValueMapping = MetaData.ValueMappings(ValueIndex);
				if ( ValueMapping.Id == MPType )
				{
					MetaData.ValueMappings.Remove(ValueIndex);
					break;
				}
			}
		}

		// we'll need to reset the value if one of the values we excluded was the profile's current value
		// so retrieve the current value
		INT CurrentValue=0;
		if ( GetProfileSettingValueId(UCONST_VERSUS_GAMETYPE, CurrentValue) )
		{
			// attempt to set it back - if that fails, we probably removed the existing value
			if ( !SetProfileSettingValueId(UCONST_VERSUS_GAMETYPE, CurrentValue) )
			{
				if ( MetaData.ValueMappings.Num() > 0 )
				{
					SetProfileSettingValueId(UCONST_VERSUS_GAMETYPE, MetaData.ValueMappings(0).Id);
				}
			}
		}
	}
}

/* ==========================================================================================================
	UGearUIInteraction
========================================================================================================== */
IMPLEMENT_CLASS(UGearUIInteraction);

#define ATTRACT_MOVIE	TEXT("Attract_Opening_Cinematic")

/**
 * Begin playing the attract-mode movie.
 */
void UGearUIInteraction::BeginAttractMovie()
{
	debugf(TEXT("Starting attract mode - showing movie: %s"), ATTRACT_MOVIE);
	IdleSeconds = 0;
	if ( GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL )
	{
		AGearPC* GearPC = Cast<AGearPC>(GEngine->GamePlayers(0)->Actor);
		if ( GearPC != NULL )
		{
			// start the movie
			bAttractMoviePlaying = TRUE;
			GearPC->eventClientPlayMovie(ATTRACT_MOVIE);
		}
		else if ( GFullScreenMovie )
		{
			// start the movie
			bAttractMoviePlaying = TRUE;
			GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, ATTRACT_MOVIE);
		}
	}
	else if ( GFullScreenMovie )
	{
		// start the movie
		bAttractMoviePlaying = TRUE;
		GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, ATTRACT_MOVIE);
	}
}

/**
 * Stop playing the attract mode movie.
 */
void UGearUIInteraction::EndAttractMovie()
{
	debugf(TEXT("Stopping attract mode - movie: %s"), ATTRACT_MOVIE);
	if ( GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0) != NULL )
	{
		AGearPC* GearPC = Cast<AGearPC>(GEngine->GamePlayers(0)->Actor);
		if ( GearPC != NULL )
		{
			GearPC->eventClientStopMovie(0.0f, FALSE, TRUE, FALSE);
			bAttractMoviePlaying = FALSE;
		}
		else if ( GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(ATTRACT_MOVIE) )
		{
			GFullScreenMovie->GameThreadStopMovie();
			bAttractMoviePlaying = FALSE;
		}
	}
	else if ( GFullScreenMovie && GFullScreenMovie->GameThreadIsMoviePlaying(ATTRACT_MOVIE) )
	{
		GFullScreenMovie->GameThreadStopMovie();
		bAttractMoviePlaying = FALSE;
	}
	IdleSeconds = 0;
}

/** 
 * @return Whether or not the loading movie is currently playing.
 */
UBOOL IsGearLoadingMoviePlaying()
{
	UBOOL bResult = FALSE;
	if( GFullScreenMovie
	&&	GFullScreenMovie->GameThreadIsMoviePlaying(TEXT("")) )
	{
		bResult = (GFullScreenMovie->GameThreadGetLastMovieName() == UCONST_LOADING_MOVIE);
	}
	return bResult;
}

/**
 * Called once a frame to update the interaction's state.
 *
 * @param	DeltaTime - The time since the last frame.
 */
void UGearUIInteraction::Tick( FLOAT DeltaTime )
{
	if ( bAttractModeAllowed && GIsGame && MaxIdleSeconds > 0.f )
	{
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();

		// force delta time to maintain at least 10 fps
		const FLOAT RealDeltaTime = WorldInfo && WorldInfo->TimeDilation > 0.f ? DeltaTime / WorldInfo->TimeDilation : DeltaTime;
		IdleSeconds += Min(RealDeltaTime, 0.1f);

		if ( IsAttractMoviePlaying() )
		{
			// check to see if the movie is finished and update bAttractMoviePlaying accordingly
			if( GFullScreenMovie == NULL || !GFullScreenMovie->GameThreadIsMoviePlaying(ATTRACT_MOVIE) )
			{
				bAttractMoviePlaying = FALSE;
				IdleSeconds = 0.f;
			}
		}
		else if ( IdleSeconds > MaxIdleSeconds )
		{
			BeginAttractMovie();
		}
	}

	Super::Tick(DeltaTime);
}

/**
 * Check a key event received by the viewport.
 *
 * @param	Viewport - The viewport which the key event is from.
 * @param	ControllerId - The controller which the key event is from.
 * @param	Key - The name of the key which an event occured for.
 * @param	Event - The type of event which occured.
 * @param	AmountDepressed - For analog keys, the depression percent.
 * @param	bGamepad - input came from gamepad (ie xbox controller)
 *
 * @return	True to consume the key event, false to pass it on.
 */
UBOOL UGearUIInteraction::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.f*/,UBOOL bGamepad/*=FALSE*/)
{
	// Always swallow input when playing a movie
	UBOOL bResult = FALSE;
	if ( bProcessInput && bAttractModeAllowed )
	{
		if ( IsAttractMoviePlaying() )
		{
			if ( Event == IE_Released && IdleSeconds > 0.5f )
			{
				EndAttractMovie();
			}

			bResult = TRUE;
		}
		else
		{
			IdleSeconds = 0.f;
		}
	}

	return bResult || IsGearLoadingMoviePlaying() || Super::InputKey(ControllerId, Key, Event, AmountDepressed,bGamepad);
}

/**
 * Check an axis movement received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Key - The name of the axis which moved.
 * @param	Delta - The axis movement delta.
 * @param	DeltaTime - The time since the last axis update.
 *
 * @return	True to consume the axis movement, false to pass it on.
 */
UBOOL UGearUIInteraction::InputAxis(INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime, UBOOL bGamepad)
{
	return Super::InputAxis(ControllerId,Key,Delta,DeltaTime,bGamepad);
}

/**
 * Check a character input received by the viewport.
 *
 * @param	Viewport - The viewport which the axis movement is from.
 * @param	ControllerId - The controller which the axis movement is from.
 * @param	Character - The character.
 *
 * @return	True to consume the character, false to pass it on.
 */
UBOOL UGearUIInteraction::InputChar(INT ControllerId,TCHAR Character)
{
	// Always swallow input when playing a movie
	UBOOL bResult = FALSE;
	if ( bProcessInput && bAttractModeAllowed )
	{
		if ( IsAttractMoviePlaying() )
		{
			if ( IdleSeconds > 0.5f )
			{
				EndAttractMovie();
			}

			bResult = TRUE;
		}
		else
		{
			IdleSeconds = 0.f;
		}
	}
	return bResult || IsGearLoadingMoviePlaying() || Super::InputChar(ControllerId, Character);
}

/**
 * @return	TRUE if mature language is supported by the current language/region/etc.
 */
UBOOL UGearUIInteraction::IsMatureLanguageSupported() const
{
	return appStricmp(GetLanguage(), TEXT("INT")) == 0;
}

#if STORAGE_MANAGER_IMPLEMENTED
/************************************************************************/
/* UGearStorageDeviceMananger                                           */
/************************************************************************/
IMPLEMENT_CLASS(UGearStorageDeviceManager);

/**
 * kick off a task to enumerate the connected storage devices....
 */
void UGearStorageDeviceManager::EnumerateStorageDeviceInfo( INT ControllerId/*=255*/ )
{
	INT FirstIndex = INDEX_NONE, LastIndex = INDEX_NONE;
	if ( ControllerId != 255 )
	{
		FirstIndex = eventFindPlayerContentByControllerId(ControllerId);
		LastIndex = FirstIndex+1;
	}

	if ( FirstIndex == INDEX_NONE )
	{
		FirstIndex = 0;
		LastIndex = StoredPlayerContentData.Num();
	}

	for ( INT PlayerIndex = FirstIndex; PlayerIndex < LastIndex; PlayerIndex++ )
	{
		FPlayerContentData& PlayerContentData = StoredPlayerContentData(PlayerIndex);
// 		for ( BYTE ContentIndex = 0; ContentIndex < SCT_MAX; ContentIndex++ )
		BYTE ContentIndex = SCT_SaveGame;
		{
			FStorageDeviceContent& StorageContentData = PlayerContentData.StoredContent[ContentIndex];

			FStorageDeviceInfo& StorageDeviceInfo = StorageContentData.DeviceInfo;
			StorageDeviceInfo.ContentType = ContentIndex;
		}
	}
}
#endif

/************************************************************************/
/* Base class for all scenes in Gears                                   */
/************************************************************************/
IMPLEMENT_CLASS(UGearUIScene_Base)

/** Overloaded to call our update function */
void UGearUIScene_Base::Tick( FLOAT DeltaTime )
{
	if ( GIsGame )
	{
#if !SHIPPING_PC_GAME && !FINAL_RELEASE
		UGameUISceneClient* GameSceneClient = Cast<UGameUISceneClient>(SceneClient);
		if ( GameSceneClient != NULL && GameSceneClient->bBlockSceneUpdates )
		{
			Super::Tick( DeltaTime );
			return;
		}
#endif

		if ( ToolTipTimerTimeRemaining > 0)
		{
			ToolTipTimerTimeRemaining -= DeltaTime;

			if ( ToolTipTimerTimeRemaining <= 0 && DELEGATE_IS_SET(OnToolTipTimerExpired) )
			{
				delegateOnToolTipTimerExpired();
			}
		}
		
		if ( DELEGATE_IS_SET(OnGearUISceneTick) )
		{
			delegateOnGearUISceneTick(DeltaTime);
		}
	}

	Super::Tick( DeltaTime );
}


/************************************************************************/
/* Base class for all MP scenes in Gears                                */
/************************************************************************/
IMPLEMENT_CLASS(UGearUISceneMP_Base)

/************************************************************************/
/* Base class for all front-end scenes in Gears                         */
/************************************************************************/
IMPLEMENT_CLASS(UGearUISceneFrontEnd_Base);

/************************************************************************/
/* Scene that is shown when making a path choice in campaign            */
/************************************************************************/
IMPLEMENT_CLASS(UGearUIScene_PathChoice)

IMPLEMENT_CLASS(UGearUISceneFE_Title);
/**
 * This notification is sent to the topmost scene when a different scene is about to become the topmost scene.
 * Provides scenes with a single location to perform any cleanup for its children.
 *
 * @param	NewTopScene		the scene that is about to become the topmost scene.
 */
void UGearUISceneFE_Title::NotifyTopSceneChanged( UUIScene* NewTopScene )
{
	Super::NotifyTopSceneChanged(NewTopScene);

	UGearUIInteraction* UIController = Cast<UGearUIInteraction>(GetCurrentUIController());
	if ( UIController != NULL )
	{
		if ( UIController->IsAttractMoviePlaying() )
		{
			UIController->EndAttractMovie();
		}

		UIController->bAttractModeAllowed = FALSE;
	}
}

/**
 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
 * once the scene has been completely initialized.
 * For widgets added at runtime, called after the widget has been inserted into its parent's
 * list of children.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UGearUISceneFE_Title::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize(inOwnerScene, inOwner);

#if XBOX
	DWORD CurrentLocale = XGetLocale();
	if ( CurrentLocale != XC_LOCALE_UNITED_STATES && CurrentLocale != XC_LOCALE_CANADA )
	{
		UUIObject* ESRBLabel = FindChild(TEXT("lblESRB"), TRUE);
		if ( ESRBLabel != NULL )
		{
			ESRBLabel->eventSetVisibility(FALSE);
		}
	}
#endif
}

/************************************************************************/
/* Overloaded friends provider so we can keep add gear specific data    */
/************************************************************************/
IMPLEMENT_CLASS(UGearUIDataProvider_OnlineFriends);

/**
 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
 *
 * @param	FieldName		the name of the field the desired cell tags are associated with.  Used for cases where a single data provider
 *							instance provides element cells for multiple collection data fields.
 * @param	CellTag			the tag for the element cell to resolve the value for
 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
 *							do not provide unique UIListElement objects for each element.
 * @param	out_FieldValue	receives the resolved value for the property specified.
 *							@see GetDataStoreValue for additional notes
 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
 *							to a data collection.
 */
UBOOL UGearUIDataProvider_OnlineFriends::GetCellFieldValue( FName FieldName, const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bResult = FALSE;

	if (FriendsList.IsValidIndex(ListIndex))
	{
		if (CellTag == FName(TEXT("bIsJoinable")))
		{
			UBOOL bShowJoin = GIsEditor || FriendsList(ListIndex).bIsJoinable;
			out_FieldValue.StringValue = bShowJoin ? TEXT("<Fonts:Warfare_HUD.Xbox360_18pt>r<Fonts:/>") : TEXT("");
			bResult=TRUE;
		}
		else if (CellTag == FName(TEXT("bHasInvited")))
		{
			UBOOL bShowInvite = GIsEditor || FriendsList(ListIndex).bHaveInvited;
			out_FieldValue.StringValue = bShowInvite ? TEXT("<Fonts:Warfare_HUD.Xbox360_18pt>e<Fonts:/>") : TEXT("");
			bResult=TRUE;
		}
		else if (CellTag == FName(TEXT("bHasInvitedYou")))
		{
			UBOOL bShowInvite = GIsEditor || FriendsList(ListIndex).bHasInvitedYou;
			out_FieldValue.StringValue = bShowInvite ? TEXT("<Fonts:Warfare_HUD.Xbox360_18pt>d<Fonts:/>") : TEXT("");
			bResult=TRUE;
		}
	}

	if (bResult)
	{
		out_FieldValue.PropertyTag=CellTag;
		out_FieldValue.PropertyType=DATATYPE_Property;
	}
	else
	{
		bResult = Super::GetCellFieldValue(FieldName, CellTag, ListIndex, out_FieldValue, ArrayIndex);
	}

	return bResult;
}

// EOF

