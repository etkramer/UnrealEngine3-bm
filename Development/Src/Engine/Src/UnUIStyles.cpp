/*=============================================================================
	UnUIStyles.cpp: UI skin, style, and style data implementations
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// engine classes
#include "EnginePrivate.h"

// widgets and supporting UI classes
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

IMPLEMENT_CLASS(UUIStyle);
IMPLEMENT_CLASS(UUIStyle_Data);
	IMPLEMENT_CLASS(UUIStyle_Image);
	IMPLEMENT_CLASS(UUIStyle_Text);
	IMPLEMENT_CLASS(UUIStyle_Combo);

IMPLEMENT_CLASS(UUISkin);
	IMPLEMENT_CLASS(UUICustomSkin);

IMPLEMENT_COMPARE_CONSTPOINTER(UUIStyle,UnUIStyles, { return appStricmp(*A->GetStyleName(),*B->GetStyleName()); })

/* ==========================================================================================================
	UUISkin
========================================================================================================== */
/**
 * Called when this skin is set to be the UI's active skin.  Initializes all styles and builds the lookup tables.
 */
void UUISkin::Initialize()
{
	// make sure our archetype skin is initialized
	UUISkin* BaseSkin = Cast<UUISkin>(GetArchetype());
	if ( BaseSkin != NULL && !BaseSkin->HasAnyFlags(RF_ClassDefaultObject) )
	{
		BaseSkin->Initialize();

		// remove any style groups that were inherited from base skins - these will go in the StyleGroupMap instead
		for ( INT GroupIndex = 0; GroupIndex < BaseSkin->StyleGroupMap.Num(); GroupIndex++ )
		{
			StyleGroups.RemoveItem(BaseSkin->StyleGroupMap(GroupIndex));
		}
	}

	// build the style lookup table
	StyleLookupTable.Empty();
	StyleNameMap.Empty();
	SoundCueMap.Empty();
	StyleGroupMap.Empty();

	InitializeLookupTables(StyleLookupTable, StyleNameMap, SoundCueMap, StyleGroupMap);

	StyleLookupTable.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
	StyleNameMap.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();

#if 0
	// this temporary code is used for adding new cursors until the UI for adding/editing cursors is done
	if ( CursorMap.Num() == 0 && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		UUISkin* BaseSkin = Cast<UUISkin>(GetArchetype());
		if ( BaseSkin != NULL && BaseSkin->HasAnyFlags(RF_ClassDefaultObject) )
		{
			FUIMouseCursor ArrowCursor, SplitVCursor, SplitHCursor;
			ArrowCursor.CursorStyle = SplitVCursor.CursorStyle = SplitHCursor.CursorStyle = TEXT("CursorImageStyle");

			UUITexture* ArrowTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), this);
			UUITexture* SplitVTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), this);
			UUITexture* SplitHTexture = ConstructObject<UUITexture>(UUITexture::StaticClass(), this);

			ArrowTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.Arrow"), NULL, LOAD_None, NULL);
			SplitVTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterVert"), NULL, LOAD_None, NULL);
			SplitHTexture->ImageTexture = LoadObject<UTexture>(NULL, TEXT("EngineResources.Cursors.SplitterHorz"), NULL, LOAD_None, NULL);

			ArrowCursor.Cursor = ArrowTexture;
			SplitHCursor.Cursor = SplitHTexture;
			SplitVCursor.Cursor = SplitVTexture;

			AddCursorResource(TEXT("Arrow"), ArrowCursor);
			AddCursorResource(TEXT("SplitterHorz"), SplitHCursor);
			AddCursorResource(TEXT("SplitterVert"), SplitVCursor);
		}
	}
#endif

	for ( TMap<FName,FUIMouseCursor>::TIterator It(CursorMap); It; ++It )
	{
		const FName& CursorName = It.Key();
		FUIMouseCursor& CursorResource = It.Value();

		if ( CursorResource.Cursor != NULL )
		{
			UUIStyle* CursorStyle = StyleNameMap.FindRef(CursorResource.CursorStyle);
			if ( CursorStyle != NULL )
			{
				CursorResource.Cursor->SetImageStyle(Cast<UUIStyle_Image>(CursorStyle->GetStyleForStateByClass(UUIState_Enabled::StaticClass())));
			}

			if ( !CursorResource.Cursor->HasValidStyleData() )
			{
				debugf(NAME_Error, TEXT("UISkin '%s': Error loading style for cursor '%s': specified cursor style not found in skin package: '%s'"), *GetName(), *CursorName.ToString(), *CursorResource.CursorStyle.ToString());
			}
		}
		else
		{
			debugf(NAME_Error, TEXT("UISkin '%s': Missing resource for cursor '%s'"), *GetName(), *CursorName.ToString());
		}
	}

	for ( INT StyleIndex = 0; StyleIndex < Styles.Num(); StyleIndex++ )
	{
		UUIStyle* Style = Styles(StyleIndex);

		Style->InitializeStyle(this);
	}
}

/**
 * Fills the values of the specified maps with the styles contained by this skin and all this skin's archetypes.
 */
void UUISkin::InitializeLookupTables( TMap<FSTYLE_ID,UUIStyle*>& out_StyleIdMap, TMap<FName,UUIStyle*>& out_StyleNameMap, TMap<FName,class USoundCue*>& out_SoundCueMap, TLookupMap<FString>& out_GroupNameMap )
{
	UUISkin* BaseSkin = Cast<UUISkin>(GetArchetype());
	if ( BaseSkin != NULL )
	{
		BaseSkin->InitializeLookupTables(out_StyleIdMap, out_StyleNameMap, out_SoundCueMap, out_GroupNameMap);
	}


	for ( INT StyleIndex = 0; StyleIndex < Styles.Num(); StyleIndex++ )
	{
		UUIStyle* Style = Styles(StyleIndex);

		//verify that our data hasn't been corrupted
		check(Style);
		check(Style->StyleID.IsValid());
		check(Style->StyleTag!=NAME_None);

		out_StyleIdMap.Set(Style->StyleID, Style);
		out_StyleNameMap.Set(Style->StyleTag, Style);
	}

	for ( INT SoundCueIndex = 0; SoundCueIndex < SoundCues.Num(); SoundCueIndex++ )
	{
		FUISoundCue& SoundCue = SoundCues(SoundCueIndex);
		if ( SoundCue.SoundName != NAME_None )
		{
			out_SoundCueMap.Set(SoundCue.SoundName, SoundCue.SoundToPlay);
		}
	}

	for ( INT GroupIndex = 0; GroupIndex < StyleGroups.Num(); GroupIndex++ )
	{
		if ( !out_GroupNameMap.HasKey(StyleGroups(GroupIndex)) )
		{
			out_GroupNameMap.AddItem(*StyleGroups(GroupIndex));
		}
	}
}

/**
 * Retrieve the style ID associated with a particular style name
 */
FSTYLE_ID UUISkin::FindStyleID( FName StyleName ) const
{
	FSTYLE_ID Result;
	Result.A = Result.B = Result.C = Result.D = 0;

	UUIStyle* Style = FindStyle(StyleName);
	if ( Style != NULL )
	{
		Result = Style->StyleID;
	}

	return Result;
}

/**
 * Retrieve the style associated with a particular style name
 */
UUIStyle* UUISkin::FindStyle( FName StyleName ) const
{
	UUIStyle* Result = StyleNameMap.FindRef(StyleName);
	return Result;
}

/**
 * Determines whether the specified style is contained in this skin
 *
 * @param	bIncludeInheritedStyles		if FALSE, only returns true if the specified style is in this skin's Styles
 *										array; otherwise, also returns TRUE if the specified style is contained by
 *										any base skins of this one.
 *
 * @return	TRUE if the specified style is contained by this skin, or one of its base skins
 */
UBOOL UUISkin::ContainsStyle( UUIStyle* StyleToSearchFor, UBOOL bIncludeInheritedStyles/*=FALSE*/ ) const
{
	UBOOL bResult = FALSE;

	if ( Styles.ContainsItem(StyleToSearchFor) )
	{
		bResult = TRUE;
	}
	else if ( bIncludeInheritedStyles == TRUE )
	{
		UUISkin* BaseSkin = Cast<UUISkin>(GetArchetype());
		if ( BaseSkin != NULL )
		{
			bResult = BaseSkin->ContainsStyle(StyleToSearchFor,TRUE);
		}
	}

	return bResult;
}

/**
 * Retrieve the list of styles available from this skin.
 *
 * @param	out_Styles					filled with the styles available from this UISkin, including styles contained by parent skins.
 * @param	bIncludeInheritedStyles		if TRUE, out_Styles will also contain styles inherited from parent styles which
 *										aren't explicitely overridden in this skin
 */
void UUISkin::GetAvailableStyles( TArray<UUIStyle*>& out_Styles, UBOOL bIncludeInheritedStyles/*=TRUE*/ )
{
	out_Styles.Empty();
	for ( TMap<FSTYLE_ID,UUIStyle*>::TIterator It(StyleLookupTable); It; ++It )
	{
		UUIStyle* Style = It.Value();
		if ( bIncludeInheritedStyles || Style->GetOuter() == this )
		{
			out_Styles.AddUniqueItem(It.Value());
		}
	}
}

/**
 * Looks up the cursor resource associated with the specified name in this skin's CursorMap.
 *
 * @param	CursorName	the name of the cursor to retrieve.
 *
 * @return	a pointer to an instance of the resource associated with the cursor name specified, or NULL if no cursors
 *			exist that are using that name
 */
UUITexture* UUISkin::GetCursorResource(FName CursorName)
{
	UUITexture* Result = NULL;

	if ( CursorName != NAME_None )
	{
		FUIMouseCursor* Resource = CursorMap.Find(CursorName);
		if ( Resource != NULL )
		{
			Result = Resource->Cursor;
		}
		else
		{
			// search for this cursor in our archetype
			UUISkin* BaseSkin = Cast<UUISkin>(GetArchetype());
			if ( BaseSkin != NULL && !BaseSkin->HasAnyFlags(RF_ClassDefaultObject) )
			{
				Result = BaseSkin->GetCursorResource(CursorName);
			}
		}
	}

	return Result;
}

/**
 * Adds a new sound cue mapping to this skin's list of UI sound cues.
 *
 * @param	SoundCueName	the name to use for this UISoundCue.  should correspond to one of the values of the UIInteraction.SoundCueNames array.
 * @param	SoundToPlay		the sound cue that should be associated with this name; NULL values are OK.
 *
 * @return	TRUE if the sound mapping was successfully added to this skin; FALSE if the specified name was invalid or wasn't found in the UIInteraction's
 *			array of available sound cue names.
 */
UBOOL UUISkin::AddUISoundCue( FName SoundCueName, USoundCue* SoundToPlay )
{
	UBOOL bResult = FALSE;

	if ( SoundCueName != NAME_None )
	{
		// find the active UIInteraction
		UUIInteraction* UIController = UUIRoot::GetCurrentUIController();
		if ( UIController == NULL )
		{
			// in the editor, we don't have an active UIInteraction so use the default instead
			UIController = UUIRoot::GetDefaultUIController();
		}

		if ( UIController != NULL )
		{
			// verify that the SoundCueName specified is registered with the UI controller
			if ( UIController->UISoundCueNames.ContainsItem(SoundCueName) )
			{
				Modify();

				UBOOL bAddToSoundCueArray = TRUE;

				// first, determine whether we already have a UISoundCue using this name;  if so, we'll replace the existing value with the new one
				for ( INT SoundCueIndex = 0; SoundCueIndex < SoundCues.Num(); SoundCueIndex++ )
				{
					FUISoundCue& SoundCue = SoundCues(SoundCueIndex);
					if ( SoundCue.SoundName == SoundCueName )
					{
						bAddToSoundCueArray = FALSE;
						SoundCue.SoundToPlay = SoundToPlay;
						break;
					}
				}

				if ( bAddToSoundCueArray == TRUE )
				{
					// add a new entry to the array
					INT Index = SoundCues.AddZeroed();
					SoundCues(Index).SoundName = SoundCueName;
					SoundCues(Index).SoundToPlay = SoundToPlay;
				}

				SoundCueMap.Set(SoundCueName, SoundToPlay);
				bResult = TRUE;
			}
			else
			{
				debugf(NAME_Warning, TEXT("UUISkin::AddUISoundCue: the specified sound cue name '%s' is not registered in the UIInteraction's array of UISoundCueNames."), *SoundCueName.ToString());
			}
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("UUISkin::AddUISoundCue: SoundCueName is 'None'"));
	}

	return bResult;
}

/**
 * Removes the specified sound cue name from this skin's list of UISoundCues
 *
 * @param	SoundCueName	the name of the UISoundCue to remove.  should correspond to one of the values of the UIInteraction.SoundCueNames array.
 *
 * @return	TRUE if the sound mapping was successfully removed from this skin or this skin didn't contain any sound cues using that name;
 */
UBOOL UUISkin::RemoveUISoundCue( FName SoundCueName )
{
	UBOOL bResult = FALSE;
	if ( SoundCueName != NAME_None )
	{
		UBOOL bSoundCueRemoved = FALSE;

		// find the sound cue using the name specified
		for ( INT SoundCueIndex = 0; SoundCueIndex < SoundCues.Num(); SoundCueIndex++ )
		{
			FUISoundCue& SoundCue = SoundCues(SoundCueIndex);
			if ( SoundCue.SoundName == SoundCueName )
			{
				bSoundCueRemoved = TRUE;
				SoundCues.Remove(SoundCueIndex);
				break;
			}
		}

		SoundCueMap.Remove(SoundCueName);
		bResult = bSoundCueRemoved;
	}
	else
	{
		debugf(NAME_Warning, TEXT("UUISkin::RemoveUISoundCue: SoundCueName is 'None'"));
	}

	return bResult;
}

/**
 * Retrieves the SoundCue associated with the specified UISoundCue name.
 *
 * @param	SoundCueName	the name of the sound cue to find.  should correspond to the SoundName for a UISoundCue contained by this skin
 * @param	out_UISoundCue	will receive the value for the sound cue associated with the sound cue name specified; might be NULL if there
 *							is no actual sound cue associated with the sound cue name specified, or if this skin doesn't contain a sound cue
 *							using that name (use the return value to determine which of these is the case)
 *
 * @return	TRUE if this skin contains a UISoundCue that is using the sound cue name specified, even if that sound cue name is not assigned to
 *			a sound cue object; FALSE if this skin doesn't contain a UISoundCue using the specified name.
 */
UBOOL UUISkin::GetUISoundCue( FName SoundCueName, USoundCue*& out_UISoundCue )
{
	UBOOL bResult = FALSE;

	if ( SoundCueName != NAME_None )
	{
		USoundCue** pSoundCue = SoundCueMap.Find(SoundCueName);
		if ( pSoundCue != NULL )
		{
			bResult = TRUE;
			out_UISoundCue = *pSoundCue;
		}
		//else
		//{
		//	debugfSuppressed(NAME_DevUISound, TEXT("UUISkin::GetUISoundCue: Invalid SoundCueName specified '%s'"), *SoundCueName.ToString());
		//}
	}

	return bResult;
}

void UUISkin::GetSkinSoundCues( TArray<FUISoundCue>& out_SoundCues )
{
	out_SoundCues = SoundCues;
}

/**
 * @return	TRUE if the specified group name exists and was inherited from this skin's base skin; FALSE if the group name
 *			doesn't exist or belongs to this skin.
 */
UBOOL UUISkin::IsInheritedGroupName( const FString& StyleGroupName ) const
{
	UBOOL bResult = FALSE;

	if ( StyleGroupName.Len() > 0 && StyleGroupMap.HasKey(*StyleGroupName) )
	{
		bResult = !StyleGroups.ContainsItem(StyleGroupName);
	}

	return bResult;
}

/**
 * Adds a new style group to this skin.
 *
 * @param	StyleGroupName	the style group name to add
 *
 * @return	TRUE if the group name was successfully added.
 */
UBOOL UUISkin::AddStyleGroupName( const FString& StyleGroupName )
{
	UBOOL bResult = FALSE;

	if ( StyleGroupName.Len() > 0 && !StyleGroupMap.HasKey(*StyleGroupName) )
	{
		Modify();

		new(StyleGroups) FString(StyleGroupName);

		TArray<UUISkin*> SkinsToUpdate;
		SkinsToUpdate.AddItem(this);

		GetDerivedSkins(this, SkinsToUpdate);
		for ( INT SkinIndex = 0; SkinIndex < SkinsToUpdate.Num(); SkinIndex++ )
		{
			UUISkin* CurrentSkin = SkinsToUpdate(SkinIndex);

			// it's possible that we're adding a style group that already exists in a derived skin.  If this is the case,
			// remove the entry from the derived skin's StyleGroups array so that we don't have two copies
			if ( CurrentSkin != this && CurrentSkin->StyleGroups.ContainsItem(StyleGroupName) )
			{
				CurrentSkin->StyleGroups.RemoveItem(StyleGroupName);
			}
			CurrentSkin->StyleGroupMap.AddItem(*StyleGroupName);
		}

		bResult = TRUE;
	}

	return bResult;
}

/**
 * Removes a style group name from this skin.
 *
 * @param	StyleGroupName	the group name to remove
 *
 * @return	TRUE if this style group was successfully removed from this skin.
 */
UBOOL UUISkin::RemoveStyleGroupName( const FString& StyleGroupName )
{
	UBOOL bResult = FALSE;

	if ( StyleGroupName.Len() > 0 )
	{
		INT GroupNameIndex = StyleGroups.FindItemIndex(StyleGroupName);
		if ( GroupNameIndex != INDEX_NONE )
		{
			Modify();

			StyleGroups.Remove(GroupNameIndex);

			TArray<UUISkin*> SkinsToUpdate;
			SkinsToUpdate.AddItem(this);

			GetDerivedSkins(this, SkinsToUpdate);
			for ( INT SkinIndex = 0; SkinIndex < SkinsToUpdate.Num(); SkinIndex++ )
			{
				UUISkin* CurrentSkin = SkinsToUpdate(SkinIndex);
				CurrentSkin->StyleGroups.RemoveItem(StyleGroupName);
				CurrentSkin->StyleGroupMap.RemoveItem(*StyleGroupName);

				TArray<UUIStyle*> Styles;
				CurrentSkin->GetAvailableStyles(Styles, FALSE);
				for ( INT StyleIndex = 0; StyleIndex < CurrentSkin->Styles.Num(); StyleIndex++ )
				{
					UUIStyle* Style = CurrentSkin->Styles(StyleIndex);
					if ( Style != NULL && Style->StyleGroupName == StyleGroupName )
					{
						Style->Modify();
						Style->StyleGroupName = TEXT("");

						//@todo ronp - call NotifyStyleChanged?
					}
				}				
			}

			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Renames a style group in this skin.
 *
 * @param	OldStyleGroupName	the style group to rename
 * @param	NewStyleGroupName	the new name to use for the style group
 *
 * @return	TRUE if the style group was successfully renamed; FALSE if it wasn't found or couldn't be renamed.
 */
UBOOL UUISkin::RenameStyleGroup( const FString& OldStyleGroupName, const FString& NewStyleGroupName )
{
	UBOOL bResult = FALSE;

	if ( OldStyleGroupName.Len() > 0 && NewStyleGroupName.Len() > 0 )
	{
		INT GroupNameIndex = StyleGroups.FindItemIndex(OldStyleGroupName);
		if ( GroupNameIndex != INDEX_NONE )
		{
			Modify();
			StyleGroups(GroupNameIndex) = NewStyleGroupName;

			GroupNameIndex = StyleGroupMap.FindRef(OldStyleGroupName);
			StyleGroupMap(GroupNameIndex) = NewStyleGroupName;

			TArray<UUISkin*> SkinsToUpdate;
			SkinsToUpdate.AddItem(this);

			GetDerivedSkins(this, SkinsToUpdate);
			for ( INT SkinIndex = 0; SkinIndex < SkinsToUpdate.Num(); SkinIndex++ )
			{
				UUISkin* CurrentSkin = SkinsToUpdate(SkinIndex);
				INT* pGroupNameIndex = CurrentSkin->StyleGroupMap.Find(OldStyleGroupName);
				if ( pGroupNameIndex != NULL )
				{
					CurrentSkin->StyleGroupMap(*pGroupNameIndex) = NewStyleGroupName;
				}

				TArray<UUIStyle*> Styles;
				CurrentSkin->GetAvailableStyles(Styles, FALSE);
				for ( INT StyleIndex = 0; StyleIndex < CurrentSkin->Styles.Num(); StyleIndex++ )
				{
					UUIStyle* Style = CurrentSkin->Styles(StyleIndex);
					if ( Style != NULL && Style->StyleGroupName == OldStyleGroupName )
					{
						Style->Modify();
						Style->StyleGroupName = NewStyleGroupName;

						//@todo ronp - call NotifyStyleChanged?
					}
				}				
			}

			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Gets the group name at the specified index
 *
 * @param	Index	the index [into the skin's StyleGroupMap] of the style to get
 *
 * @return	the group name at the specified index, or an empty string if the index is invalid.
 */
FString UUISkin::GetStyleGroupAtIndex( INT Index ) const
{
	FString GroupName;

	if ( StyleGroupMap.IsValidIndex(Index) )
	{
		GroupName = StyleGroupMap(Index);
	}

	return GroupName;
}

/**
 * Finds the index for the specified group name.
 *
 * @param	StyleGroupName	the group name to find
 *
 * @return	the index [into the skin's StyleGroupMap] for the specified style group, or INDEX_NONE if it wasn't found.
 */
INT UUISkin::FindStyleGroupIndex( const FString& StyleGroupName ) const
{
	INT Result = INDEX_NONE;
	if ( StyleGroupName.Len() > 0 )
	{
		const INT* pResult = StyleGroupMap.Find(StyleGroupName);
		if ( pResult )
		{
			Result = *pResult;
		}
	}
	return Result;
}

/**
 * Retrieves the full list of style group names.
 *
 * @param	StyleGroupArray	recieves the array of group names
 * @param	bIncludeInheritedGroupNames		specify FALSE to exclude group names inherited from base skins.
 */
void UUISkin::GetStyleGroups( TArray<FString>& StyleGroupArray, UBOOL bIncludeInheritedGroups/*=TRUE*/ ) const
{
	if ( bIncludeInheritedGroups )
	{
		const INT NumGroups = StyleGroupMap.Num();
		StyleGroupArray.Empty(NumGroups);

		for ( INT GroupNameIndex = 0; GroupNameIndex < StyleGroupMap.Num(); GroupNameIndex++ )
		{
			new(StyleGroupArray) FString(StyleGroupMap(GroupNameIndex));
		}
	}
	else
	{
		StyleGroupArray = StyleGroups;
	}
}

/**
 * Creates a new style within this skin.
 *
 * @param	StyleClass		the class to use for the new style
 * @param	StyleTag		the unique tag to use for creating the new style
 * @param	StyleTemplate	the template to use for the new style
 * @param	bAddToSkin		TRUE to automatically add this new style to this skin's list of styles
 */
UUIStyle* UUISkin::CreateStyle( UClass* StyleClass, FName StyleTag, UUIStyle* StyleTemplate/*=NULL*/, UBOOL bAddToSkin/*=TRUE*/ )
{
	UUIStyle* Result = NULL;

	if ( StyleClass != NULL && StyleClass->IsChildOf(UUIStyle_Data::StaticClass()) )
	{
		Result = ConstructObject<UUIStyle>
		(
			UUIStyle::StaticClass(),							// class
			this,												// outer
			StyleTag,											// name
			RF_Public|RF_Transactional|RF_PerObjectLocalized,	// flags
			StyleTemplate										// template
		);

		check(Result);

		Result->StyleID = appCreateGuid();
		Result->StyleTag = StyleTag;
		Result->StyleDataClass = StyleClass;
		Result->StateDataMap.Empty();

		// every style must be able to at least support the "enabled" state...if this style's isn't based
		// on another style that already contains a UIStyle_Data object for the enabled state, create one
		if ( Result->GetStyleForStateByClass(UUIState_Enabled::StaticClass()) == NULL )
		{
			Result->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());
		}

		if ( bAddToSkin )
		{
			// if the style couldn't be added to this skin, return NULL
			if ( !AddStyle(Result) )
			{
				Result = NULL;
			}
		}
	}

	return Result;
}

/**
 * Creates a new style using the template provided and replaces its entry in the style lookup tables. 
 * This only works if the outer of the style template is a archetype of the this skin.
 *
 * @param	StyleTemplate	the template to use for the new style
 * @return  Pointer to the style that was created to replace the archetype's style.
 */
UUIStyle* UUISkin::ReplaceStyle( class UUIStyle* StyleTemplate )
{
	UUIStyle* Result = NULL;

	if ( StyleTemplate != NULL )
	{
		UClass* StyleClass = StyleTemplate->StyleDataClass;
		UUISkin* OuterSkin = Cast<UUISkin>(StyleTemplate->GetOuter());

		if(OuterSkin != this && StyleClass != NULL && StyleClass->IsChildOf(UUIStyle_Data::StaticClass()))
		{
			Modify();

			Result = ConstructObject<UUIStyle>
				(
				UUIStyle::StaticClass(),							// class
				this,												// outer
				StyleTemplate->StyleTag,							// name
				RF_Public|RF_Transactional|RF_PerObjectLocalized,	// flags
				StyleTemplate										// template
				);

			check(Result);

			Result->StyleName = StyleTemplate->StyleName;
			Result->StyleID = StyleTemplate->StyleID;
			Result->StyleTag = StyleTemplate->StyleTag;
			Result->StyleDataClass = StyleClass;
			Result->StateDataMap.Empty();

			// every style must be able to at least support the "enabled" state...if this style's isn't based
			// on another style that already contains a UIStyle_Data object for the enabled state, create one
			if ( Result->GetStyleForStateByClass(UUIState_Enabled::StaticClass()) == NULL )
			{
				Result->AddNewState(UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>());
			}

			
			//Remove the archetype's style from our style maps.
			StyleNameMap.Remove(StyleTemplate->StyleTag);
			StyleLookupTable.Remove(StyleTemplate->StyleID);

			//Add the new style to the tables.
			Styles.AddItem(Result);
			StyleNameMap.Set(Result->StyleTag, Result);
			StyleLookupTable.Set(Result->StyleID, Result);

			StyleLookupTable.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
			StyleNameMap.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
		}
	}

	return Result;
}

/**
 * Deletes the specified style and replaces its entry in the lookup table with this skin's archetype style.  
 * This only works if the provided style's outer is this skin.
 *
 * @return	TRUE if the style was successfully deleted.
 */
UBOOL UUISkin::DeleteStyle( class UUIStyle* InStyle )
{
	UBOOL bResult = FALSE;

	if(InStyle && InStyle->IsDefaultStyle() == FALSE)
	{
		UUISkin* OuterSkin = Cast<UUISkin>(InStyle->GetOuter());

		if(OuterSkin == this)
		{
			// Get the archetype skin's version of this style.
			UUISkin* ParentSkin = Cast<UUISkin>(GetArchetype());

			if(ParentSkin != NULL)
			{
				UUIStyle* ParentStyle = ParentSkin->FindStyle(InStyle->StyleTag);

			
				Modify();

				// Remove the style from the skin.
				Styles.RemoveItem(InStyle);
				StyleNameMap.Remove(InStyle->StyleTag);
				StyleLookupTable.Remove(InStyle->StyleID);

				// If we found a style in the archetype, add the archetype's style to the lookup tables.
				if(ParentStyle != NULL)
				{
					Styles.AddUniqueItem(ParentStyle);
					StyleNameMap.Set(ParentStyle->StyleTag, ParentStyle);
					StyleLookupTable.Set(ParentStyle->StyleID, ParentStyle);
				}

				StyleLookupTable.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
				StyleNameMap.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/**
 * Adds the specified style to this skin's list of styles.
 *
 * @return	TRUE if the style was successfully added to this skin.
 */
UBOOL UUISkin::AddStyle( UUIStyle* NewStyle )
{
	UBOOL bResult = FALSE;

	if ( !Styles.ContainsItem(NewStyle) && NewStyle->StyleID.IsValid() )
	{
		Modify(TRUE);

		Styles.AddItem(NewStyle);
		StyleLookupTable.Set(NewStyle->StyleID, NewStyle);
		StyleNameMap.Set(NewStyle->StyleTag, NewStyle);

		StyleLookupTable.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
		StyleNameMap.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Makes any necessary internal changes when this style has been modified
 *
 * @param	Style	style in this skin which data has been modified
 */
void UUISkin::NotifyStyleModified( UUIStyle* Style )
{
	verify(Style);

	if(!Style)
	{
		return;
	}

	// If the style's unique style tag has changed, we must update our StyleNameMap
	// Assuming there can't be more than one tag mapped to a single style
	const FName* OldTag = StyleNameMap.FindKey(Style);
	if( OldTag && *OldTag != Style->StyleTag)
	{
		INT Ret = StyleNameMap.Remove(*OldTag);
		check(Ret == 1);
		StyleNameMap.Set(Style->StyleTag, Style);

		StyleNameMap.ValueSort<COMPARE_CONSTPOINTER_CLASS(UUIStyle,UnUIStyles)>();
	}
}

/**
 * Generate a list of UISkin objects in memory that are derived from the specified skin.
 */
void UUISkin::GetDerivedSkins( const UUISkin* ParentSkin, TArray<UUISkin*>& out_DerivedSkins, UUISkin::EDerivedType DeriveFilter/*=DERIVETYPE_All*/ )
{
	if ( ParentSkin != NULL )
	{
		for ( TObjectIterator<UUISkin> It; It; ++It )
		{
			if (!It->IsIn(GetTransientPackage())
			&&	It->IsBasedOnArchetype(ParentSkin)
			&&	(DeriveFilter == DERIVETYPE_All || It->GetArchetype() == ParentSkin) )
			{
				out_DerivedSkins.AddItem(*It);
			}
		}
	}
}

/**
 * Obtains a list of styles that derive from the ParentStyle (i.e. have ParentStyle in their archetype chain)
 *
 * @param	ParentStyle		the style to search for archetype references to
 * @param	DerivedStyles	[out] An array that will be filled with pointers to the derived styles
 * @param	DeriveType		if DERIVETYPE_DirectOnly, only styles that have ParentStyle as the value for ObjectArchetype will added
 *							out_DerivedStyles. if DERIVETYPE_All, any style that has ParentStyle anywhere in its archetype chain
 *							will be added to the list.
 * @param	SearchType		if SEARCH_SameSkinOnly, only styles contained by this skin will be considered.  if SEARCH_AnySkin, all loaded styles will be considered.
 */
void UUISkin::GetDerivedStyles( const UUIStyle* ParentStyle, TArray<UUIStyle*>& out_DerivedStyles, UUISkin::EDerivedType DeriveType, UUISkin::EStyleSearchType SearchType/*=SEARCH_AnySkin*/ )
{
	out_DerivedStyles.Empty();

	if ( SearchType == SEARCH_AnySkin )
	{
		// search all styles in memory
		for ( TObjectIterator<UUIStyle> It; It; ++It )
		{
			UUIStyle* Style = *It;

			// if the style is contained by the transient package, it is a temporary style created by the style editor so we shouldn't include it in our list of styles
			if ( Style != ParentStyle && Style->GetOutermost() != GetTransientPackage() )
			{
				// see if this style is based on ParentStyle
				UBOOL bIsDescendent = (DeriveType == DERIVETYPE_DirectOnly)
					? Style->GetArchetype() == ParentStyle
					: Style->IsBasedOnArchetype(ParentStyle);

				if ( bIsDescendent )
				{
					out_DerivedStyles.AddUniqueItem(Style);
				}
			}
		}
	}
	else
	{
		for ( INT StyleIndex = 0; StyleIndex < Styles.Num(); StyleIndex++ )
		{
			UUIStyle* Style = Styles(StyleIndex);
			if ( Style != ParentStyle )
			{
				// see if this style is based on ParentStyle
				UBOOL bIsDescendent = (DeriveType == DERIVETYPE_DirectOnly)
					? Style->GetArchetype() == ParentStyle
					: Style->IsBasedOnArchetype(ParentStyle);

				if ( bIsDescendent )
				{
					out_DerivedStyles.AddUniqueItem(Style);
				}
			}
		}
	}
}

/**
 * Checks if Tag name is currently used in this Skin
 *
 * @param	Tag		checks if this tag exists in the skin
 */
UBOOL UUISkin::IsUniqueTag( const FName & Tag )
{
	if(StyleNameMap.Find(Tag) != NULL)
	{
		return FALSE;
	}

	return TRUE;
}


/**
 * Adds a new mouse cursor resource to this skin.
 *
 * @param	CursorTag			the name to use for the mouse cursor.  this will be the name that must be used to retrieve
 *								this mouse cursor via GetCursorResource()
 * @param	CursorResource		the mouse cursor to add
 *
 * @return	TRUE if the cursor was successfully added to the skin.  FALSE if the resource was invalid or there is already
 *			another cursor using the specified tag.
 */
UBOOL UUISkin::AddCursorResource( FName CursorTag, const FUIMouseCursor& CursorResource )
{
	UBOOL bResult = FALSE;

	if ( CursorTag != NAME_None )
	{
		if ( CursorResource.Cursor != NULL )
		{
			if ( CursorResource.CursorStyle != NAME_None )
			{
				FSTYLE_ID StyleID = FindStyleID(CursorResource.CursorStyle);
				if ( StyleID.IsValid() )
				{
					// ok - everything is valid...now it's safe to add the cursor resource
					if ( !CursorResource.Cursor->HasValidStyleData() )
					{
						UUIStyle** CursorStyle = StyleLookupTable.Find(StyleID);
						check(CursorStyle != NULL);

						CursorResource.Cursor->SetImageStyle(Cast<UUIStyle_Image>((*CursorStyle)->GetStyleForStateByClass(UUIState_Enabled::StaticClass())));
					}

					Modify(TRUE);

					// make sure the cursor can be referenecd by derived skins
					CursorResource.Cursor->SetFlags(RF_Public|RF_Transactional);
					CursorMap.Set(CursorTag, CursorResource);
					bResult = TRUE;
				}
				else
				{
					debugf(NAME_Error, TEXT("UIStyle::AddCursorResource: Specified style (%s) not found in skin for new cursor '%s'"), *CursorResource.CursorStyle.ToString(), *CursorTag.ToString());
				}
			}
			else
			{
				debugf(NAME_Error, TEXT("UIStyle::AddCursorResource: Invalid style name specified for new cursor '%s'"), *CursorTag.ToString());
			}
		}
		else
		{
			debugf(NAME_Error, TEXT("UIStyle::AddCursorResource: NULL resource specified for new cursor '%s'"), *CursorTag.ToString());
		}
	}
	else
	{
		debugf(NAME_Error, TEXT("UIStyle::AddCursorResource: Invalid name specified for new cursor"));
	}

	return bResult;
}

/**
 * Searches for the Style specified and if found changes the node modifier's
 *
 * @param	StyleName	the name of the style to apply - must match the StyleTag of a style in this skin (or a base skin).
 * @param	StyleData	the style data to apply the changes to.
 *
 * @return	TRUE if a style was found with the specified StyleName, FALSE otherwise.
 */
UBOOL UUISkin::ParseStringModifier( const FString& StyleName, FUIStringNodeModifier& StyleData )
{
	UBOOL bResult = FALSE;

	if ( StyleName.Len() > 0 )
	{
		if ( StyleName == TEXT("/") )
		{
			// remove the current font from the string customizer; this will change the current font to which font was previously active
			if ( !StyleData.RemoveStyle() )
			{
				//@todo - for some reason, the top-most style couldn't be removed (probably only one style on the modifier stack, indicating that we have
				// a closing style tag without the corresponding opening style tag.....should we complain about this??
			}

			bResult = TRUE;
		}
		else
		{
			FName StyleTag = FName(*StyleName,FNAME_Find);
			if ( StyleTag != NAME_None )
			{
				// search for the style specified
				UUIStyle* Style = FindStyle(StyleTag);
				if ( Style != NULL )
				{
					// get the StyleData for the current menu state
					UUIStyle_Data* StateStyleData = Style->GetStyleForState(StyleData.GetMenuState());
					if ( StateStyleData != NULL )
					{
						StyleData.AddStyle(StateStyleData);
						bResult = TRUE;
					}
				}
			}
		}
	}

	return bResult;
}

/* === UObject interface === */
/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param Owner			the UIString that owns this node.  used to provide access to UObject::AddReferencedObject
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UUISkin::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for ( TMap<FName,FUIMouseCursor>::TIterator It(CursorMap); It; ++It )
	{
		FUIMouseCursor& Resource = It.Value();
		AddReferencedObject( ObjectArray, Resource.Cursor );
	}
}

/**
 * I/O function
 */
void UUISkin::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	//@todo - any special logic here to allow styles to override styles in base skins
	if ( Ar.IsTransacting() )
	{
		Ar << StyleLookupTable << StyleNameMap << StyleGroupMap << CursorMap << SoundCueMap;
	}
	else
	{
		Ar << CursorMap;
	}
}

/**
 * Called when this object is loaded from an archive.  Fires a callback which notifies the UI editor that this skin
 * should be re-initialized if it's the active skin.
 */
void UUISkin::PostLoad()
{
	Super::PostLoad();

	if ( !GIsGame && GCallbackEvent != NULL )
	{
		GCallbackEvent->Send(CALLBACK_UIEditor_SkinLoaded, this);
	}
}

/**
 * Callback for retrieving a textual representation of natively serialized properties.  Child classes should implement this method if they wish
 * to have natively serialized property values included in things like diffcommandlet output.
 *
 * @param	out_PropertyValues	receives the property names and values which should be reported for this object.  The map's key should be the name of
 *								the property and the map's value should be the textual representation of the property's value.  The property value should
 *								be formatted the same way that UProperty::ExportText formats property values (i.e. for arrays, wrap in quotes and use a comma
 *								as the delimiter between elements, etc.)
 * @param	ExportFlags			bitmask of EPropertyPortFlags used for modifying the format of the property values
 *
 * @return	return TRUE if property values were added to the map.
 */
UBOOL UUISkin::GetNativePropertyValues( TMap<FString,FString>& out_PropertyValues, DWORD ExportFlags/*=0*/ ) const
{
	UBOOL bResult = Super::GetNativePropertyValues(out_PropertyValues, ExportFlags);

	INT Count=0, LongestCursorName=0;

	TMap<FString,FString> PropertyValues;
	for ( TMap<FName,FUIMouseCursor>::TConstIterator It(CursorMap); It; ++It )
	{
		const FString CursorName = It.Key().ToString();
		const FUIMouseCursor& Cursor = It.Value();

		const FString TextureName = Cursor.Cursor != NULL
			? FString::Printf(TEXT("%s'%s'"), *Cursor.Cursor->GetClass()->GetName(), *Cursor.Cursor->GetPathName())
			: TEXT("NULL");

		const FString PropertyValue = FString::Printf(TEXT("(CursorStyle=%s,Cursor=%s)"), *Cursor.CursorStyle.ToString(), *TextureName);

		LongestCursorName = Max(LongestCursorName, CursorName.Len());
		PropertyValues.Set(*CursorName, *PropertyValue);
	}

	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString PropertyName = FString::Printf(TEXT("CursorMap[%i]"), Count++);
		const FString& CursorName = It.Key();
		const FString& DataName = It.Value();

		out_PropertyValues.Set(*PropertyName, *FString::Printf(TEXT("%s => %s"), *CursorName.RightPad(LongestCursorName), *DataName));
		bResult = TRUE;
	}

	return bResult || CursorMap.Num() > 0;
}

/* ==========================================================================================================
	UUICustomSkin
========================================================================================================== */

/**
 * Deletes the specified style and replaces its entry in the lookup table with this skin's archetype style.  
 * This only works if the provided style's outer is this skin.
 *
 * This version of DeleteStyle goes through the WidgetStyleMap and changes any widgets bound to the style being deleted,
 * to be bound to the archetype's style instead.
 *
 * @return	TRUE if the style was successfully removed.
 */
UBOOL UUICustomSkin::DeleteStyle( class UUIStyle* InStyle )
{
	UBOOL bResult = FALSE;

	if(InStyle)
	{
		UUISkin* OuterSkin = Cast<UUISkin>(InStyle->GetOuter());

		if(OuterSkin == this)
		{
			// Get the archetype skin's version of this style.
			UUISkin* ParentSkin = Cast<UUISkin>(GetArchetype());

			if(ParentSkin != NULL)
			{
				UUIStyle* ParentStyle = ParentSkin->FindStyle(InStyle->StyleTag);

				if(ParentStyle != NULL)
				{
					Modify();

					// Find all of the widgets bound to this style and replace their style with the parent style.
					TArray<FWIDGET_ID> RemoveList;

					for ( TMap<FWIDGET_ID,FSTYLE_ID>::TIterator It(WidgetStyleMap); It; ++It )
					{
						FSTYLE_ID& StyleID = It.Value();

						if(StyleID == InStyle->StyleID)
						{
							RemoveList.AddItem(It.Key());
						}
					}

					for (INT WidgetIdx = 0; WidgetIdx < RemoveList.Num(); WidgetIdx++)
					{
						FWIDGET_ID &WidgetId = RemoveList(WidgetIdx);
						WidgetStyleMap.Remove(WidgetId);
						WidgetStyleMap.Set(WidgetId, ParentStyle->StyleID);
					}

					// Remove the style from the skin.
					Styles.RemoveItem(InStyle);
					StyleNameMap.Remove(InStyle->StyleTag);
					StyleLookupTable.Remove(InStyle->StyleID);

					// Add the archetype's style to the lookup tables.
					Styles.AddUniqueItem(ParentStyle);
					StyleNameMap.Set(ParentStyle->StyleTag, ParentStyle);
					StyleLookupTable.Set(ParentStyle->StyleID, ParentStyle);

					bResult = TRUE;
				}
			}
		}
	}

	return bResult;
}

/**
 * Assigns the style specified to the widget and stores that mapping in the skin's persistent style list.
 *
 * @param	Widget		the widget to apply the style to.
 * @param	StyleID		the STYLEID for the style that should be assigned to the widget
 */
void UUICustomSkin::StoreWidgetStyleMapping( UUIObject* Widget, const FSTYLE_ID& StyleID )
{
	check(Widget->WidgetID.IsValid());
	check(StyleID.IsValid());
	check(StyleLookupTable.Find(StyleID) != NULL);

	FSTYLE_ID* CurrentlyAssignedStyle = WidgetStyleMap.Find(Widget->WidgetID);
	if ( CurrentlyAssignedStyle == NULL || (*CurrentlyAssignedStyle) != StyleID )
	{
		Modify(TRUE);
	}

	// add the widget->style mapping to the widget style map
	WidgetStyleMap.Set(Widget->WidgetID, StyleID);
}

/* ==========================================================================================================
	UUIStyle
========================================================================================================== */

/**
 * Callback for retrieving a textual representation of natively serialized properties.  Child classes should implement this method if they wish
 * to have natively serialized property values included in things like diffcommandlet output.
 *
 * @param	out_PropertyValues	receives the property names and values which should be reported for this object.  The map's key should be the name of
 *								the property and the map's value should be the textual representation of the property's value.  The property value should
 *								be formatted the same way that UProperty::ExportText formats property values (i.e. for arrays, wrap in quotes and use a comma
 *								as the delimiter between elements, etc.)
 * @param	ExportFlags			bitmask of EPropertyPortFlags used for modifying the format of the property values
 *
 * @return	return TRUE if property values were added to the map.
 */
UBOOL UUIStyle::GetNativePropertyValues( TMap<FString,FString>& out_PropertyValues, DWORD ExportFlags/*=0*/ ) const
{
	UBOOL bResult = Super::GetNativePropertyValues(out_PropertyValues, ExportFlags);

	INT Count=0, LongestStateName = 0;
	TMap<FString,FString> PropertyValues;
	for ( TMap<UUIState*,UUIStyle_Data*>::TConstIterator It(StateDataMap); It; ++It )
	{
		const UUIState* State = It.Key();
		const UUIStyle_Data* Data = It.Value();

		const FString StateName = FString::Printf(TEXT("%s'%s'"), *State->GetClass()->GetName(), *State->GetPathName());
		const FString PropertyValue = FString::Printf(TEXT("%s'%s'"), *Data->GetClass()->GetName(), *Data->GetPathName());

		LongestStateName = Max(LongestStateName, StateName.Len());
		PropertyValues.Set(*StateName, *PropertyValue);
	}

	for ( TMap<FString,FString>::TConstIterator It(PropertyValues); It; ++It )
	{
		const FString PropertyName = FString::Printf(TEXT("StateDataMap[%i]:"), Count++);
		const FString& StateName = It.Key();
		const FString& DataName = It.Value();

		out_PropertyValues.Set(*PropertyName, *FString::Printf(TEXT("%s => %s"), *StateName.RightPad(LongestStateName), *DataName));
		bResult = TRUE;
	}

	return bResult || StateDataMap.Num() > 0;
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param Owner			the UIString that owns this node.  used to provide access to UObject::AddReferencedObject
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UUIStyle::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
	{
		AddReferencedObject( ObjectArray, It.Key() );
		AddReferencedObject( ObjectArray, It.Value() );
	}
}

/** File I/O */
void UUIStyle::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	Ar << StateDataMap;

	if ( Ar.IsLoading() && Ar.Ver() < VER_REMOVED_PREFAB_STYLE_DATA )
	{
		for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
		{
			UUIState* StyleState = It.Key();
			if ( StyleState == NULL || !StyleState->HasAnyFlags(RF_ClassDefaultObject) )
			{
				It.RemoveCurrent();
				MarkPackageDirty();
			}
		}
	}
}

void UUIStyle::PostLoad()
{
	Super::PostLoad();

	if ( !HasAnyFlags(RF_ClassDefaultObject) )
	{
		for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
		{
			UUIState* StyleState = It.Key();
			UUIStyle_Data* StyleData = It.Value();

			UUIStyle_Data* StyleDataArchetype = GetArchetypeStyleForState(StyleState);
			if ( StyleDataArchetype != NULL )
			{
				if ( !StyleDataArchetype->HasAnyFlags(RF_ClassDefaultObject) )
				{
					if ( StyleDataArchetype != StyleData->GetArchetype() )
					{
						// this style data's archetype is not pointing to the corresponding style data
						// contained in this style's archetype (this can happen as a result of bugs in the style editor code)
						// so restore the archetype for the style data now...

						debugf(TEXT("Restoring archetype for style data %s to %s (%s)"), *StyleData->GetPathName(), *StyleDataArchetype->GetPathName(), *StyleState->GetClass()->GetName());
						RestoreStyleArchetype(StyleData, StyleDataArchetype);
					}
				}

				UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(StyleData);
				if ( ComboStyleData != NULL )
				{
					UUIStyle_Combo* ArchetypeComboData = Cast<UUIStyle_Combo>(StyleDataArchetype);

					// if this is a combo style, check the combo style's custom style data objects to make sure they have the correct archetype as well
					if( !ComboStyleData->TextStyle.IsCustomStyleDataValid() )
					{
						UUIStyle_Text* TextTemplate = Cast<UUIStyle_Text>(ArchetypeComboData->TextStyle.GetCustomStyleData());
						UUIStyle_Data* NewTextData = ConstructObject<UUIStyle_Data>
						(
							UUIStyle_Text::StaticClass(),						// class
							ComboStyleData,						    			// outer must be the combo data or this object will not be deep serialized
							NAME_None,	    									// name
							RF_Public|RF_Transactional,							// flags
							TextTemplate										// template
						);

						check(NewTextData);	
						NewTextData->Modify(TRUE);	
						NewTextData->bEnabled = FALSE;   // initially make the style use the existing data
						ComboStyleData->Modify(TRUE);
						ComboStyleData->TextStyle.SetOwnerStyle(this);
						ComboStyleData->TextStyle.SetCustomStyleData(NewTextData); 
					}
					else
					{
						// verify that the combo style's custom text data has the correct archetype
						UUIStyle_Text* CustomTextData = Cast<UUIStyle_Text>(ComboStyleData->TextStyle.GetCustomStyleData());
						UUIStyle_Text* CustomTextDataArchetype = Cast<UUIStyle_Text>(ArchetypeComboData->TextStyle.GetCustomStyleData());
						if ( CustomTextDataArchetype != NULL && CustomTextDataArchetype != CustomTextData->GetArchetype() )
						{
							debugf(TEXT("Restoring archetype for custom text style data %s to %s (%s)"), *StyleData->GetPathName(), *StyleDataArchetype->GetPathName(), *StyleState->GetClass()->GetName());
							RestoreStyleArchetype(CustomTextData, CustomTextDataArchetype);
						}
					}

					// create a default custom image style data if one does not exist 
					if( !ComboStyleData->ImageStyle.IsCustomStyleDataValid() )
					{
						UUIStyle_Image* ImageTemplate = Cast<UUIStyle_Image>(ArchetypeComboData->ImageStyle.GetCustomStyleData());
						UUIStyle_Data* NewImageData = ConstructObject<UUIStyle_Data>
						(
							UUIStyle_Image::StaticClass(),						// class
							ComboStyleData,						    			// outer must be the combo data or this object will not be deep serialized
							NAME_None,	    									// name
							RF_Public|RF_Transactional,							// flags
							ImageTemplate										// template
						);

						check(NewImageData);	
						NewImageData->Modify(TRUE);	
						NewImageData->bEnabled = FALSE;    // initially make the style use the existing data	
						ComboStyleData->Modify(TRUE);	
						ComboStyleData->ImageStyle.SetOwnerStyle(this);
						ComboStyleData->ImageStyle.SetCustomStyleData(NewImageData); 			
					}
					else
					{
						// verify that the combo style's custom image data has the correct archetype
						UUIStyle_Image* CustomImageData = Cast<UUIStyle_Image>(ComboStyleData->ImageStyle.GetCustomStyleData());
						UUIStyle_Image* CustomImageDataArchetype = Cast<UUIStyle_Image>(ArchetypeComboData->ImageStyle.GetCustomStyleData());
						if ( CustomImageDataArchetype != NULL && CustomImageDataArchetype != CustomImageData->GetArchetype() )
						{
							debugf(TEXT("Restoring archetype for custom image style data %s to %s (%s)"), *StyleData->GetPathName(), *StyleDataArchetype->GetPathName(), *StyleState->GetClass()->GetName());
							RestoreStyleArchetype(CustomImageData, CustomImageDataArchetype);
						}
					}
				}
			}
		}
	}
}

/**
 * Restores the archetype for the specified style and reinitializes the style data object against the new archetype, 
 * preserving the values serialized into StyleData
 *
 * @param	StyleData			the style data object that has the wrong archetype
 * @param	StyleDataArchetype	the style data object that should be the archetype
 */
void UUIStyle::RestoreStyleArchetype( UUIStyle_Data* StyleData, UUIStyle_Data* StyleDataArchetype )
{
	check(StyleData);
	check(StyleDataArchetype);
	check(StyleDataArchetype!=StyleData->GetArchetype());

	// mark the package as dirty
	StyleData->Modify(TRUE);

	// use an FReloadObjectArc to save the serialized values of this style data object
	FReloadObjectArc StyleAr;
	StyleAr.ActivateWriter();
	StyleAr.SerializeObject(StyleData);

	// now correct the archetype
	StyleData->SetArchetype(StyleDataArchetype);

	// now read the serialized property values back onto the style data object -
	// this will also cause the style data's values to be reinitialized from the new archetype prior to reserialization
	StyleAr.ActivateReader();
	StyleAr.SerializeObject(StyleData);
}

/**
 *	Obtain style data for the specified state from the archetype style
 *	
 *	@param StateObject	State for which the data will be extracted
 *	@return returns the corresponding state data or NULL archetype doesn't contain this state or 
 *			if this style's archetype is the class default object
 */
UUIStyle_Data* UUIStyle::GetArchetypeStyleForState( UUIState* StateObject ) const
{
	UUIStyle_Data* Result = NULL;
	UUIStyle* ArchetypeStyle = CastChecked<UUIStyle>(this->GetArchetype());

	// check this style contains a valid archetype style
	if(ArchetypeStyle != UUIStyle::StaticClass()->GetDefaultObject())
	{
		Result = ArchetypeStyle->GetStyleForState(StateObject);
	}

	return Result;
}

/**
 * Called when this style is loaded by its owner skin.
 *
 * @param	OwnerSkin	the skin that contains this style.
 */
void UUIStyle::InitializeStyle( UUISkin* OwnerSkin )
{
	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
	{
		UUIStyle_Data* StyleData = It.Value();

		StyleData->ResolveExternalReferences(OwnerSkin);
		StyleData->ValidateStyleData();
	}

	// check to see if this style had a StyleGroupName that has been removed.  This should only be the case
	// when the StyleGroupName was from an inherited skin and this style's package wasn't saved when the group
	// name was removed.
	if ( StyleGroupName.Len() > 0 && OwnerSkin->FindStyleGroupIndex(StyleGroupName) == INDEX_NONE )
	{
		Modify();
		StyleGroupName = TEXT("");
	}
}

/**
 * Get the name for this style.
 *
 * @return	If the value for StyleName is identical to the value for this style's template, returns this style's
 *			StyleTag....otherwise, returns this style's StyleName
 */
FString UUIStyle::GetStyleName() const
{
	// if this style's friendly name is the same as its template (which probably indicates that it hasn't been added to the loc file yet),
	// return the style's tag, which should be unique
	UUIStyle* StyleTemplate = GetArchetype<UUIStyle>();
	UBOOL bStyleNameLocalized = StyleTemplate != NULL && StyleName != StyleTemplate->StyleName;
	return bStyleNameLocalized ? FString(StyleName) : StyleTag.ToString();
}

/**
 * Returns the style data associated with the archetype for the UIState specified by StateObject.  If this style does not contain
 * any style data for the specified state, this style's archetype is searched, recursively.
 *
 * @param	StateObject	the UIState to search for style data for.  StateData is stored by archetype, so the StateDataMap
 *						is searched for each object in StateObject's archetype chain until a match is found or we arrive
 *						at the class default object for the state.
 *						
 *
 * @return	a pointer to style data associated with the UIState specified, or NULL if there is no style data for the specified
 *			state in this style or this style's archetypes
 */
UUIStyle_Data* UUIStyle::GetStyleForState( UUIState* StateObject ) const
{
	check(StateObject);

	UUIStyle_Data* Result = NULL;

	// first, make sure we're working with an archetype...if StateObject isn't an archetype,
	// iterate through its archetype chain until we find one
	UUIState* StateArchetype = StateObject;
	while ( StateArchetype && !StateArchetype->HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject) )
	{
		StateArchetype = StateArchetype->GetArchetype<UUIState>();
	}

	UUIStyle_Data* const* StyleData = NULL;
	// search for style data associated with the StateObject archetype...if no state data is found, try
	// the next archetype in StateObject's archetype chain, until we've hit the class default object
	while ( StateArchetype && StateArchetype->GetClass() == StateObject->GetClass() && StyleData == NULL )
	{
		StyleData = StateDataMap.Find(StateArchetype);
		StateArchetype = StateArchetype->GetArchetype<UUIState>();
	}

	if ( StyleData != NULL && *StyleData != NULL )
	{
		Result = *StyleData;
	}
	else
	{
		// attempt to find style data in an archetype of this UIStyle
		UUIStyle* Template = Cast<UUIStyle>(GetArchetype());
		if ( Template != NULL && !Template->IsTemplate() )
		{
			Result = Template->GetStyleForState(StateObject);
		}
	}

	return Result;
}

/**
 * Returns the first style data object associated with an object of the class specified.  This function is not reliable
 * in that it can return different style data objects if there are multiple states of the same class in the map (i.e.
 * two archetypes of the same class)
 *
 * @param	StateClass	the class to search for style data for
 *
 * @return	a pointer to style data associated with the UIState specified, or NULL if there is no style data for the specified
 *			state in this style or this style's archetypes
 */
UUIStyle_Data* UUIStyle::GetStyleForStateByClass( UClass* StateClass ) const
{
	check(StateClass);
	checkSlow(StateClass->IsChildOf(UUIState::StaticClass()));

	UUIStyle_Data* Result = NULL;
	for ( TMap<UUIState*,UUIStyle_Data*>::TConstIterator It(StateDataMap); It; ++It )
	{
		const UUIState* StateArchetype = It.Key();
		if ( StateArchetype->GetClass() == StateClass )
		{
			Result = *(&It.Value());
			break;
		}
	}

	if ( Result == NULL )
	{
		// attempt to find style data in an archetype of this UIStyle
		UUIStyle* Template = Cast<UUIStyle>(GetArchetype());
		if ( Template != NULL )
		{
			Result = Template->GetStyleForStateByClass(StateClass);
		}
	}

	return Result;
}

/**
 * Creates and initializes a new style data object for the UIState specified.
 *
 * @param	StateToAdd	the state to add style data for.  If StateToAdd does not have either the RF_ArchetypeObject
 *						or RF_ClassDefaultObject flags set, the new style data will be associated with StateToAdd's
 *						ObjectArchetype instead.
 */
UUIStyle_Data* UUIStyle::AddNewState( UUIState* StateToAdd, UUIStyle_Data* DataArchetype/*=NULL*/ )
{
	check(StateToAdd);
	while ( StateToAdd != NULL && !StateToAdd->HasAnyFlags(RF_ClassDefaultObject|RF_ArchetypeObject) )
	{
		StateToAdd = StateToAdd->GetArchetype<UUIState>();
	}
	
	UUIStyle_Data* Result = NULL;
	if ( StateToAdd != NULL )
	{
		// only allow one instance of each state type in our list
		for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
		{
			if ( It.Key()->GetClass() == StateToAdd->GetClass() )
			{
				Result = It.Value();
			}
		}

		if ( Result == NULL )
		{
			// since we know our own StateDataMap doesn't contain style data for this style class, StyleDataTemplate
			// will correspond to the style data stored in a template of ours, or NULL if there is no style data for this
			// state stored in our archetype chain
			UUIStyle_Data* StyleDataTemplate = DataArchetype;
			if ( StyleDataTemplate == NULL )
			{
				StyleDataTemplate = GetStyleForState(StateToAdd);
			}

			FName StyleDataName = NAME_None;
			QWORD StyleObjectFlags=RF_Public|RF_Transactional;
			if ( StyleDataTemplate != NULL )
			{
				// only use the archetype's name if the StyleDataTemplate was pulled from a template of ours
				if ( StyleDataTemplate != DataArchetype && !StyleDataTemplate->HasAnyFlags(RF_ClassDefaultObject) )
				{
					StyleDataName = StyleDataTemplate->GetFName();
				}

				StyleObjectFlags |= StyleDataTemplate->GetFlags();
			}

			Result = ConstructObject<UUIStyle_Data>
			(
				StyleDataClass,							// class
				this,									// outer
				StyleDataName,							// name
				StyleObjectFlags,						// flags
				StyleDataTemplate						// template
			);

			if ( Result != NULL )
			{
				Modify(TRUE);
				Result->Modify();

				// allow the style data to perform any initialization necessary
				Result->Created(StateToAdd);

				// add the style data to our map of state-to-style_data
				StateDataMap.Set(StateToAdd, Result);
				Result->bEnabled = TRUE;
			}
		}
	}

	return Result;
}

/**
 * Returns whether this style's data has been modified, requiring the style to be reapplied.
 *
 * @param	DataToCheck		if specified, returns whether the values have been modified for that style data only.  If not
 *							specified, checks all style data contained by this style.
 *
 * @return	TRUE if the style data contained by this style needs to be reapplied to any widgets using this style.
 */
UBOOL UUIStyle::IsDirty( UUIStyle_Data* DataToCheck/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;

	// verify that the specified style data actually lives in this UIStyle
	if ( DataToCheck != NULL && StateDataMap.FindKey(DataToCheck) != NULL )
	{
		bResult = DataToCheck->IsDirty();
	}

	return bResult;
}

/**
 * Returns whether this style's data has been modified, requiring the style to be reapplied.
 *
 * @param	StateToCheck	if specified, returns whether the values have been modified for that menu state's style data only. 
 *							If not specified, checks all style data contained by this style.
 *
 * @return	TRUE if the style data contained by this style needs to be reapplied to any widgets using this style.
 */
UBOOL UUIStyle::IsDirty( UUIState* StateToCheck/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;
	UUIStyle_Data* CheckStyleData = NULL;

	if ( StateToCheck != NULL && (CheckStyleData = GetStyleForState(StateToCheck)) != NULL )
	{
		bResult = IsDirty(CheckStyleData);
	}
	else
	{
		for ( TMap<UUIState*,UUIStyle_Data*>::TConstIterator It(StateDataMap); It; ++It )
		{
			const UUIStyle_Data* StyleData = It.Value();
			if ( StyleData != NULL && StyleData->IsDirty() )
			{
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}

/**
 * Sets or clears the dirty flag for this style, which indicates whether this style's data should be reapplied.
 *
 * @param	bIsDirty	TRUE to mark the style dirty, FALSE to clear the dirty flag
 * @param	Target		if specified, only sets the dirty flag for this style data object.  Otherwise, sets the dirty
 *						flag for all style data contained by this style.
 */
void UUIStyle::SetDirtiness( UBOOL bIsDirty, UUIStyle_Data* Target/*=NULL*/ )
{
	if ( Target != NULL )
	{
		// verify that the specified Target exists in our StateDataMap.
		//@todo - search our archetype states if we don't have this style data
		UUIState* const * StateKey = StateDataMap.FindKey(Target);
		if ( StateKey != NULL && *StateKey != NULL )
		{
			Target->SetDirtiness(bIsDirty);
			return;
		}
	}

	for ( TMap<UUIState*,UUIStyle_Data*>::TIterator It(StateDataMap); It; ++It )
	{
		UUIStyle_Data* StyleData = It.Value();
		StyleData->SetDirtiness(bIsDirty);
	}
}

/**
* Creates a newly constructed copy of the receiver with a hard copy of its StateDataMap.
* New style will be transient and cannot be saved out.
*
* @return	Pointer to a newly constructed transient copy of the passed style
*/
UUIStyle* UUIStyle::CreateTransientCopy()
{
	FObjectDuplicationParameters DupParms(this, GetOuter());
	DupParms.ApplyFlags = RF_Transient;
	DupParms.bMigrateArchetypes = FALSE;
	
	UUIStyle* Copy = Cast<UUIStyle>(StaticDuplicateObjectEx(DupParms));
	check(Copy);

	return Copy;
}

/**
 * Returns TRUE if this style indirectly references specified style through its DataMap
 */
UBOOL UUIStyle::ReferencesStyle(const UUIStyle* Style) const
{
	UBOOL bResult = this == Style;
	if ( !bResult )
	{
		const UUIStyle_Data* CurrentData = NULL;
		for(TMap<UUIState*,UUIStyle_Data*>::TConstIterator It(StateDataMap); It; ++It)
		{
			CurrentData = It.Value();
			check(CurrentData);

			if(CurrentData && CurrentData->IsReferencingStyle(Style))
			{
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult;
}

/**
* Returns TRUE if this style is one of the designated default styles
*/
UBOOL UUIStyle::IsDefaultStyle() const
{
	 return (StyleTag == TEXT("DefaultImageStyle") ||
	        StyleTag == TEXT("DefaultTextStyle")  ||
			StyleTag == TEXT("DefaultComboStyle"));
}

/* ==========================================================================================================
	UUIStyle_Data
========================================================================================================== */

/** Returns the UIStyle object that contains this style data */
UUIStyle* UUIStyle_Data::GetOwnerStyle() const
{	
	UUIStyle* Result = NULL;

	if ( GetOuter() != NULL )
	{
		Result = Cast<UUIStyle>(GetOuter());
		if(Result == NULL)
		{
			// if we are dealing with a combo style custom data then we have to call GetOuter twice to get to the UIStyle
			Result = Cast<UUIStyle>(GetOuter()->GetOuter());
		}
	}

	return Result;
}


/**
 * Returns whether this style's data has been modified, requiring the style to be reapplied.
 *
 * @return	TRUE if this style's data has been modified, indicating that it should be reapplied to any subscribed widgets.
 */
UBOOL UUIStyle_Data::IsDirty() const
{
	return bDirty;
}

/**
 * Sets or clears the dirty flag for this style data.
 *
 * @param	bIsDirty	TRUE to mark the style dirty, FALSE to clear the dirty flag
 */
void UUIStyle_Data::SetDirtiness( UBOOL bIsDirty )
{
	bDirty = bIsDirty;
}

/**
 * Returns whether the values for this style data match the values from the style specified.
 *
 * @param	OtherStyle	the style to compare this style's values against
 *
 * @return	TRUE if all style property values are the same as the other style's or if the other style is same as this one
 */
UBOOL UUIStyle_Data::MatchesStyleData( UUIStyle_Data* OtherStyle ) const
{
	UBOOL bResult = FALSE;

	if ( OtherStyle == NULL )
	{
		bResult = this == NULL;
	}
	else if ( OtherStyle == this )
	{
		bResult = TRUE;
	}
	else
	{
		bResult
			=	StyleColor == OtherStyle->StyleColor
			&&	ARE_FLOATS_EQUAL(StylePadding[UIORIENT_Horizontal],OtherStyle->StylePadding[UIORIENT_Horizontal])
			&&	ARE_FLOATS_EQUAL(StylePadding[UIORIENT_Vertical],OtherStyle->StylePadding[UIORIENT_Vertical])
			&&	(!DELEGATE_IS_SET(MatchesStyleData) || const_cast<UUIStyle_Data*>(this)->delegateMatchesStyleData(OtherStyle));
	}

	return bResult;
}

/* ==========================================================================================================
	UUIStyle_Text
========================================================================================================== */
/**
 * Returns whether the values for this style data match the values from the style specified.
 *
 * @param	OtherStyle	the style to compare this style's values against
 *
 * @return	TRUE if all style property values are the same as the other style's or if the other style is same as this one
 */
UBOOL UUIStyle_Text::MatchesStyleData( UUIStyle_Data* OtherStyle ) const
{
	UBOOL bResult = Super::MatchesStyleData(OtherStyle);

	if ( bResult )
	{
		if ( OtherStyle == NULL )
		{
			bResult = this == NULL;
		}
		else if ( OtherStyle == this )
		{
			bResult = TRUE;
		}
		else if ( OtherStyle != NULL && bResult && OtherStyle->IsA(UUIStyle_Text::StaticClass()) )
		{
			UUIStyle_Text* OtherTextStyle = Cast<UUIStyle_Text>(OtherStyle);

			// compare the properties specific to text style data
			bResult 
				=	StyleFont == OtherTextStyle->StyleFont
				&&	Attributes == OtherTextStyle->Attributes
				&&	Alignment[UIORIENT_Horizontal] == OtherTextStyle->Alignment[UIORIENT_Horizontal]
				&&	Alignment[UIORIENT_Vertical] == OtherTextStyle->Alignment[UIORIENT_Vertical]
				&&	ARE_FLOATS_EQUAL(Scale.X,OtherTextStyle->Scale.X)
				&&	ARE_FLOATS_EQUAL(Scale.Y,OtherTextStyle->Scale.Y)
				&&	ClipMode == OtherTextStyle->ClipMode
				&&	ClipAlignment == OtherTextStyle->ClipAlignment
				&&	AutoScaling == OtherTextStyle->AutoScaling
				&&	SpacingAdjust == OtherTextStyle->SpacingAdjust;

		}
	}

	return bResult;
}

/**
 * Allows the style to verify that it contains valid data for all required fields.  Called when the owning style is being initialized, after
 * external references have been resolved.
 *
 * This version verifies that this style has a valid font and if not sets the font reference to the default font.
 */
void UUIStyle_Text::ValidateStyleData()
{
	Super::ValidateStyleData();

	if ( StyleFont == NULL )
	{
		debugf(NAME_Warning, TEXT("INVALID font for %s - falling back to default font!"), *GetFullName());

		StyleFont = GEngine->GetTinyFont();
		if ( StyleFont == NULL )
		{
			StyleFont = GEngine->GetSmallFont();
			if ( StyleFont == NULL )
			{
				StyleFont = GEngine->GetMediumFont();
				if ( StyleFont == NULL )
				{
					StyleFont = GEngine->GetLargeFont();
				}
			}
		}
	}

	checkf(StyleFont, TEXT("NULL font in style '%s' and no fallback font could be found!"), *GetFullName());
}

/**
 * Called after this object has been de-serialized from disk.
 *
 * This version propagates CLIP_Scaled and CLIP_ScaledBestFit ClipMode values to the new AutoScaling property
 */
void UUIStyle_Text::PostLoad()
{
	Super::PostLoad();
}

/* ==========================================================================================================
	UUIStyle_Image
========================================================================================================== */
/**
 * Returns whether the values for this style data match the values from the style specified.
 *
 * @param	OtherStyle	the style to compare this style's values against
 *
 * @return	TRUE if all style property values are the same as the other style's or if the other style is same as this one
 */
UBOOL UUIStyle_Image::MatchesStyleData( UUIStyle_Data* OtherStyle ) const
{
	UBOOL bResult = Super::MatchesStyleData(OtherStyle);
	if ( bResult )
	{
		if ( OtherStyle == NULL )
		{
			bResult = this == NULL;
		}
		else if ( OtherStyle == this )
		{
			bResult = TRUE;
		}
		else if ( OtherStyle != NULL && bResult && OtherStyle->IsA(UUIStyle_Image::StaticClass()) )
		{
			UUIStyle_Image* OtherImageStyle = Cast<UUIStyle_Image>(OtherStyle);

			// compare the properties specific to image style data
			bResult
				=	DefaultImage == OtherImageStyle->DefaultImage
				&&	Coordinates == OtherImageStyle->Coordinates
				&&	AdjustmentType[UIORIENT_Horizontal] == OtherImageStyle->AdjustmentType[UIORIENT_Horizontal]
				&&	AdjustmentType[UIORIENT_Vertical] == OtherImageStyle->AdjustmentType[UIORIENT_Vertical];
		}
	}

	return bResult;
}

/* ==========================================================================================================
	UUIStyle_Combo
========================================================================================================== */
/**
 * Called when this style data object is added to a style.
 *
 * @param	the menu state that this style data has been created for.
 */
void UUIStyle_Combo::Created( UUIState* AssociatedState )
{
	Super::Created(AssociatedState);

	UUIStyle* OwnerStyle = GetOwnerStyle();
	check(OwnerStyle);

	UUISkin* OwnerSkin = CastChecked<UUISkin>(OwnerStyle->GetOuter());

	// if we aren't based on another style data that already had values for text and image style, set those to 
	// point to the default styles now.
	if ( TextStyle.GetStyleData() == NULL )
	{
		UUIStyle* DefaultTextStyle = OwnerSkin->FindStyle(TEXT("DefaultTextStyle"));
		check(DefaultTextStyle);

		TextStyle.SourceStyleID = DefaultTextStyle->StyleID;
		TextStyle.SourceState = AssociatedState;
	}

	if ( ImageStyle.GetStyleData() == NULL )
	{
       UUIStyle* DefaultImageStyle = OwnerSkin->FindStyle(TEXT("DefaultImageStyle"));
	   check(DefaultImageStyle);

	   ImageStyle.SourceStyleID = DefaultImageStyle->StyleID;
	   ImageStyle.SourceState = AssociatedState;
	}

	ResolveExternalReferences(OwnerSkin);
}

/**
 * Resolves any references to other styles contained in this style data object.
 *
 * @param	OwnerSkin	the skin that is currently active.
 */
void UUIStyle_Combo::ResolveExternalReferences( UUISkin* ActiveSkin )
{
	Super::ResolveExternalReferences(ActiveSkin);

	TextStyle.ResolveSourceStyle(ActiveSkin);
	ImageStyle.ResolveSourceStyle(ActiveSkin);
}

/**
 * Allows the style to verify that it contains valid data for all required fields.  Called when the owning style is being initialized, after
 * external references have been resolved.
 */
void UUIStyle_Combo::ValidateStyleData()
{
	Super::ValidateStyleData();

	UUIStyle_Data* CustomStyleData = TextStyle.GetCustomStyleData();
	if ( CustomStyleData != NULL )
	{
		CustomStyleData->ValidateStyleData();
	}

	CustomStyleData = ImageStyle.GetCustomStyleData();
	if ( CustomStyleData != NULL )
	{
		CustomStyleData->ValidateStyleData();
	}
}

/**
 * Returns whether the values for this style data match the values from the style specified.
 *
 * @param	OtherStyle	the style to compare this style's values against
 *
 * @return	TRUE if StyleToCompare has different values for any style data properties.  FALSE if the specified style is
 *			NULL, or if the specified style is the same as this one.
 */
UBOOL UUIStyle_Combo::MatchesStyleData( UUIStyle_Data* OtherStyle ) const
{
	UBOOL bResult = Super::MatchesStyleData(OtherStyle);

	if ( OtherStyle == this )
	{
		bResult = TRUE;
	}
	else if ( OtherStyle != NULL && bResult && OtherStyle->IsA(UUIStyle_Combo::StaticClass()) )
	{
		UUIStyle_Combo* OtherComboStyle = Cast<UUIStyle_Combo>(OtherStyle);
		bResult
			=	TextStyle == OtherComboStyle->TextStyle
			&&	ImageStyle == OtherComboStyle->ImageStyle;
	}

	return bResult;
}

/**
 * Returns whether this style's data has been modified, requiring the style to be reapplied.
 *
 * @return	TRUE if this style's data has been modified, indicating that it should be reapplied to any subscribed widgets.
 */
UBOOL UUIStyle_Combo::IsDirty() const
{
	return Super::IsDirty() || TextStyle.IsDirty() || ImageStyle.IsDirty();
}

/**
 * Sets or clears the dirty flag for this style data.
 *
 * @param	bIsDirty	TRUE to mark the style dirty, FALSE to clear the dirty flag
 */
void UUIStyle_Combo::SetDirtiness( UBOOL bIsDirty )
{
	Super::SetDirtiness(bIsDirty);

	UUIStyle_Data* StyleData = NULL;
	StyleData = TextStyle.GetStyleData();
	if ( StyleData != NULL )
	{
		StyleData->SetDirtiness(bIsDirty);
	}

	StyleData = ImageStyle.GetStyleData();
	if ( StyleData != NULL )
	{
		StyleData->SetDirtiness(bIsDirty);
	}
}

/** Returns TRUE if this style data references specified style */
UBOOL UUIStyle_Combo::IsReferencingStyle(const UUIStyle* Style) const
{
	UUIStyle_Data* ImageData = ImageStyle.GetStyleData();
	UUIStyle_Data* TextData = TextStyle.GetStyleData();

	return	(ImageData != NULL && ImageData->GetOwnerStyle() == Style)
		||	(TextData != NULL && TextData->GetOwnerStyle() == Style);
}

/**
 * Accessor for retrieving a reference to the active text & image styles, taking into account whether the combo style is using custom style data.
 */
UUIStyle_Text* UUIStyle_Combo::GetComboTextStyle() const
{
	return Cast<UUIStyle_Text>(TextStyle.GetStyleData());
}
UUIStyle_Image* UUIStyle_Combo::GetComboImageStyle() const
{
	return Cast<UUIStyle_Image>(ImageStyle.GetStyleData());
}

/**
 * Assigns the SourceStyleID property for the style references contained by this combo style, if this style was saved
 * prior to adding SourceStyleID to UIStyleDataReference
 */
void UUIStyle_Combo::PostLoad()
{
	Super::PostLoad();

	// if our style references have a valid style pointer, but not a valid style id, it means that this style was saved before
	// SourceStyleID was added to FStyleDataReference
	if ( TextStyle.SourceStyle != NULL && !TextStyle.SourceStyleID.IsValid() )
	{
		TextStyle.SourceStyleID = TextStyle.SourceStyle->StyleID;
	}

	if ( ImageStyle.SourceStyle != NULL && !ImageStyle.SourceStyleID.IsValid() )
	{
		ImageStyle.SourceStyleID = ImageStyle.SourceStyle->StyleID;
	}

	if( TextStyle.OwnerStyle == NULL )
	{
		TextStyle.OwnerStyle = Cast<UUIStyle>(this->GetOuter());
	}

	if( ImageStyle.OwnerStyle == NULL )
	{
		ImageStyle.OwnerStyle = Cast<UUIStyle>(this->GetOuter());
	}
}

/* ==========================================================================================================
	FStyleDataReference
========================================================================================================== */
/** Constructors */

FStyleDataReference::FStyleDataReference()
: OwnerStyle(NULL), CustomStyleData(NULL)
{
}

FStyleDataReference::FStyleDataReference( UUIStyle* InSourceStyle, UUIState* InSourceState )
: OwnerStyle(NULL), SourceStyleID(FGuid(0,0,0,0)), SourceStyle(InSourceStyle), SourceState(InSourceState), CustomStyleData(NULL)
{
	if ( SourceStyle != NULL )
	{
		SourceStyleID = SourceStyle->StyleID;
	}
}

/**
 * Resolves SourceStyleID from the specified skin and assigns the result to SourceStyle.
 *
 * @param	ActiveSkin	the currently active skin.
 */
void FStyleDataReference::ResolveSourceStyle( UUISkin* ActiveSkin )
{
	check(ActiveSkin);
	check(SourceStyleID.IsValid());

	SourceStyle = ActiveSkin->StyleLookupTable.FindRef(SourceStyleID);
}

/**
 * Enables or disables the custom style data if the OwnerStyle is the outer of the custom data
 */
void FStyleDataReference::EnableCustomStyleData( UBOOL BoolFlag )
{
	if( CustomStyleData != NULL && CustomStyleData->GetOwnerStyle() == OwnerStyle )
	{
		CustomStyleData->bEnabled = BoolFlag;
	}
}

/** 
 *	Determines if referenced custom style data is valid.
 */
UBOOL FStyleDataReference::IsCustomStyleDataValid() const
{
	// Data is invalid if the OwnerStyle is different from the outer of the style data
	if( CustomStyleData != NULL && CustomStyleData->GetOwnerStyle() == OwnerStyle )
	{
		return TRUE;
	}

	return FALSE;	
}

/**
 *	Determines if the custom style data is valid and enabled
 */
UBOOL FStyleDataReference::IsCustomStyleDataEnabled() const
{
	if ( IsCustomStyleDataValid() && CustomStyleData->bEnabled == TRUE )
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * Returns the style data object linked to this reference, if SourceStyle or SourceState are NULL then CustomStyleData will be returned instead
 */
UUIStyle_Data* FStyleDataReference::GetStyleData() const
{
	UUIStyle_Data* Result = NULL;

	if(IsCustomStyleDataEnabled())
	{
		Result = CustomStyleData;
	}
	else if ( SourceStyle != NULL && SourceState != NULL )
	{
		Result = SourceStyle->GetStyleForState(SourceState);
		if ( Result == NULL )
		{
			// if the source state doesn't contain style data for this state, use the style data for the default state
			Result = SourceStyle->eventGetDefaultStyle();
		}
	}
	
	return Result;
}

/**
 *	Sets The SourceStyle reference, makes sure that SourceState is valid for this style
 */
void FStyleDataReference::SafeSetStyle(UUIStyle* Style)
{
	check(Style);
	check(SourceState);

	// Check if new Style contains data for the SourceState reference
	if(Style->GetStyleForState(SourceState))
	{
		SourceStyle = Style;
		SourceStyleID = Style->StyleID;
	}
}

/**
 * 	Sets The SourceState reference, makes sure that SourceStyle contains this State
 */
void FStyleDataReference::SafeSetState(UUIState* State)
{
	check(State);
	check(SourceStyle);

	// Check if style reference contains data for this new State
	if(SourceStyle->GetStyleForState(State))
	{
		SourceState = State;
	}
}

/** 
 * Changes SourceStyle to the style specified, without checking whether it is valid.
 */
void FStyleDataReference::SetSourceStyle( UUIStyle* NewStyle )
{
	SourceStyle = NewStyle;
	if ( SourceStyle != NULL )
	{
		SourceStyleID = SourceStyle->StyleID;
	}
	else
	{
		SourceStyleID = FGuid(EC_EventParm);
	}
}

/**
 * Changes SourceState to the state specified, without checking whether it is valid.
 */
void FStyleDataReference::SetSourceState( UUIState* NewState )
{
	SourceState = NewState;
}

/**
 * Returns whether the styles referenced are marked as dirty
 */
UBOOL FStyleDataReference::IsDirty() const
{
	UBOOL Result;

	if(IsCustomStyleDataEnabled())
	{
		Result = CustomStyleData->IsDirty();
	}
	else
	{
		Result = SourceStyle != NULL && SourceState != NULL && SourceStyle->IsDirty(SourceState);
	}

	return Result;
}

/** Comparison */
UBOOL FStyleDataReference::operator ==( const FStyleDataReference& Other ) const
{
	UBOOL bResult
		=	SourceStyle == Other.SourceStyle
		&&	SourceState == Other.SourceState
		&&	SourceStyleID == Other.SourceStyleID
		&&	CustomStyleData == Other.CustomStyleData
		&&	OwnerStyle == Other.OwnerStyle;
	
	if ( bResult )
	{
		// compare the underlying style data for the two styles
		UUIStyle_Data* CurrentStyleData = GetStyleData();
		UUIStyle_Data* OtherStyleData = Other.GetStyleData();

		bResult =	(CurrentStyleData == NULL && OtherStyleData == NULL)
				||	(CurrentStyleData != NULL && CurrentStyleData->MatchesStyleData(OtherStyleData));
	}
	return bResult;
}

UBOOL FStyleDataReference::operator !=( const FStyleDataReference& Other ) const
{
	return !((*this) == Other);
}

/* ==========================================================================================================
	FUIStyleReference
========================================================================================================== */
/** Default constructor; don't initialize any members or we'll overwrite values serialized from disk. */
FUIStyleReference::FUIStyleReference()
{
}

/** Initialization constructor - zero initialize all members */
FUIStyleReference::FUIStyleReference(EEventParm)
: DefaultStyleTag(NAME_None)
, RequiredStyleClass(NULL)
, AssignedStyleID(EC_EventParm)
, ResolvedStyle(NULL)
{
}

/**
 * Clears the value for the resolved style.  Called whenever the resolved style is no longer valid, such as when the
 * active skin has been changed.
 */
void FUIStyleReference::InvalidateResolvedStyle()
{
	ResolvedStyle = NULL;
}

/**
 * Determines whether the specified style is a valid style for this style reference, taking into account the RequiredStyleClass.
 *
 * @param	StyleToCheck	a pointer to a UIStyle with a valid StyleID.
 * @param	bAllowNULLStyle	indicates whether a NULL value for StyleToCheck should be considered valid.
 *
 * @return	TRUE if the specified style is the right type for this style reference, or if StyleToCheck is NULL (it is always
 *			valid to assign NULL styles to a style reference) and bAllowNULLStyle is TRUE.
 */
UBOOL FUIStyleReference::IsValidStyle( UUIStyle* StyleToCheck, UBOOL bAllowNULLStyle/*=TRUE*/ ) const
{
	UBOOL bResult = FALSE;

	if ( StyleToCheck != NULL )
	{
		if ( StyleToCheck->StyleDataClass != NULL )
		{
			bResult = (RequiredStyleClass == NULL || StyleToCheck->StyleDataClass->IsChildOf(RequiredStyleClass));
		}
	}
	else
	{
		bResult = bAllowNULLStyle;
	}

	return bResult;
}

/**
 * Determines whether the specified style corresponds to the default style for this style reference.
 *
 * @param	StyleToCheck	a pointer to a UIStyle
 *
 * @return	TRUE if StyleToCheck is the same style that would be resolved by this style reference if it didn't have
 *			a valid AssignedStyleId
 */
UBOOL FUIStyleReference::IsDefaultStyle( UUIStyle* StyleToCheck ) const
{
	UBOOL bResult = FALSE;

	if ( StyleToCheck != NULL )
	{
		UUISkin* OwnerSkin = StyleToCheck->GetOuterUUISkin();

		// if this style's tag is the same as our default style tag, then this is a default style
		UBOOL bSkinContainsStyle;
		if ( StyleToCheck->StyleTag == GetDefaultStyleTag(OwnerSkin,&bSkinContainsStyle) )
		{
			bResult = TRUE;
		}
		else if ( !bSkinContainsStyle )
		{
			// not the same as our default style tag, but this could mean that the style wasn't found in the style's owner skin
			// in this case, the only thing we can do is check whether StyleToCheck is a default style
			bResult = StyleToCheck->IsDefaultStyle();
		}
	}

	return bResult;
}

/**
 * Returns the value of ResolvedStyle, optionally resolving the style reference from the currently active skin.
 *
 * @param	CurrentlyActiveSkin		if specified, will call ResolveStyleReference if the current value for ResolvedStyle is not valid
 *									for the active skin (i.e. if ResolvedStyle is NULL or isn't contained by the active skin)
 * @param	bResolvedStyleChanged	if specified, will be set to TRUE if the value for ResolvedStyle was changed during this call
 *									to GetResolvedStyle()
 *
 * @return	a pointer to the UIStyle object that has been resolved from the style id and/or default style type for this
 *			style reference.
 */
UUIStyle* FUIStyleReference::GetResolvedStyle( UUISkin* CurrentlyActiveSkin/*=NULL*/, UBOOL* bResolvedStyleChanged/*=NULL*/ )
{
	// save the value of ResolvedStyle so that we can set the value of bResolvedStyleChanged if it was passed in
	UUIStyle* PreviouslyResolvedStyle = ResolvedStyle;

	if ( CurrentlyActiveSkin != NULL )
	{
		// if we haven't resolved this style reference yet, or the current value of ResolvedStyle is not contained within the currently active skin,
		// we'll resolve the style reference first
		if ( ResolvedStyle == NULL || ResolvedStyle->GetOuter() != CurrentlyActiveSkin )
		{
			ResolveStyleReference(CurrentlyActiveSkin);
		}
	}

	// if the caller wants to know whether the resolved style changed, set that now
	if ( bResolvedStyleChanged != NULL )
	{
		*bResolvedStyleChanged = (PreviouslyResolvedStyle != ResolvedStyle);
	}

	return ResolvedStyle;
}

/**
 * Resolves the style id or default style tag for this style reference into a UIStyle from the currently active skin.
 *
 * @param	CurrentlyActiveSkin		the skin to use for resolving this style reference
 *
 * @return	TRUE if the style reference was successfully resolved
 */
UBOOL FUIStyleReference::ResolveStyleReference( UUISkin* CurrentlyActiveSkin )
{
	UBOOL bResult = FALSE;

	if ( CurrentlyActiveSkin != NULL )
	{
		FSTYLE_ID ResolvedStyleID = AssignedStyleID;
		FName DefaultTag = GetDefaultStyleTag(CurrentlyActiveSkin);

		UBOOL bAssignStyleID = TRUE;
		UUIStyle* Style = NULL;

		if ( ResolvedStyleID.IsValid() )
		{
			// lookup the style's id in the current skin
			Style = CurrentlyActiveSkin->StyleLookupTable.FindRef(ResolvedStyleID);
		}

		if ( !IsValidStyle(Style,FALSE) && DefaultTag != NAME_None )
		{
			// don't change the AssignedStyleID if we're going to use the default style for this style reference
			bAssignStyleID = FALSE;

			// if we don't have a valid style id assigned, attempt to lookup the styleID for the style corresponding to the DefaultStyleTag
			ResolvedStyleID = CurrentlyActiveSkin->FindStyleID(DefaultTag);

			// clear the reference to the resolved style (in case the style resolve from ResolvedStyleID in this skin was the wrong type)
			Style = NULL;
		}

		if ( ResolvedStyleID.IsValid() )
		{
			if ( Style == NULL )
			{
                // lookup the style's id in the current skin
				Style = CurrentlyActiveSkin->StyleLookupTable.FindRef(ResolvedStyleID);
			}

			if ( IsValidStyle(Style,FALSE) )
			{
				// only change the AssignedStyleID if this style isn't the default style for this style reference
				if ( bAssignStyleID && !IsDefaultStyle(Style) )
				{
					SetStyleID(ResolvedStyleID);
				}

				ResolvedStyle = Style;
			}
			else
			{
				ResolvedStyle = NULL;
			}

			bResult = TRUE;
		}
		else
		{
			ResolvedStyle = NULL;
		}
	}

	return bResult;
}

/**
 * Resolves the style id or default style tag for this UIStyleReference and returns the result.
 *
 * @param	CurrentlyActiveSkin		the skin to use for resolving this style reference
 *
 * @return	a pointer to the UIStyle object resolved from the specified skin
 */
UUIStyle* FUIStyleReference::ResolveStyleFromSkin( UUISkin* CurrentlyActiveSkin ) const
{
	UUIStyle* Result = NULL;
	if ( CurrentlyActiveSkin != NULL )
	{
		FSTYLE_ID ResolvedStyleID = AssignedStyleID;
		FName DefaultTag = GetDefaultStyleTag(CurrentlyActiveSkin);

		UUIStyle* Style = NULL;
		if ( ResolvedStyleID.IsValid() )
		{
			// lookup the style's id in the current skin
			Style = CurrentlyActiveSkin->StyleLookupTable.FindRef(ResolvedStyleID);
		}

		if ( !IsValidStyle(Style,FALSE) && DefaultTag != NAME_None )
		{
			// if we don't have a valid style id assigned, attempt to lookup the styleID for the style corresponding to the DefaultStyleTag
			ResolvedStyleID = CurrentlyActiveSkin->FindStyleID(DefaultTag);

			// clear the reference to the resolved style (in case the style resolve from ResolvedStyleID in this skin was the wrong type)
			Style = NULL;
		}

		if ( ResolvedStyleID.IsValid() )
		{
			if ( Style == NULL )
			{
				// lookup the style's id in the current skin
				Style = CurrentlyActiveSkin->StyleLookupTable.FindRef(ResolvedStyleID);
			}

			if ( IsValidStyle(Style,FALSE) )
			{
				Result = Style;
			}
		}
	}

	return Result;
}

/**
 * Returns the tag for the default style associated with this UIStyleReference.
 *
 * @param	CurrentlyActiveSkin		the skin to search for this style references' style tag in
 * @param	bSkinContainsStyleTag	if specified, set to TRUE if CurrentlyActiveSkin contains this style reference's
 *									DefaultStyleTag; useful for determining whether a result of e.g. "DefaultTextStyle" is
 *									because the active skin didn't contain the style corresponding to this reference's DefaultStyleTag,
 *									or whether this style reference's DefaultStyleTag is actually "DefaultTextStyle"
 *
 * @return	if DefaultStyleTag is set and a style with that tag exists in CurrentlyActiveSkin, returns that
 *			style's tag; otherwise returns the tag for the default style corresponding to RequiredStyleClass.
 */
FName FUIStyleReference::GetDefaultStyleTag( UUISkin* CurrentlyActiveSkin, UBOOL* bSkinContainsStyleTag/*=NULL*/ ) const
{
	FName Result = NAME_None;

	if ( bSkinContainsStyleTag != NULL )
	{
		*bSkinContainsStyleTag = FALSE;
	}

	// if the currently active skin contains a style that has a StyleTag matching our DefaultStyleTag, return that
	if (DefaultStyleTag != NAME_None && CurrentlyActiveSkin != NULL && CurrentlyActiveSkin->FindStyle(DefaultStyleTag) != NULL )
	{
		Result = DefaultStyleTag;
		if ( bSkinContainsStyleTag != NULL )
		{
			*bSkinContainsStyleTag = TRUE;
		}
	}
	else if ( RequiredStyleClass != NULL )
	{
		// otherwise, return the StyleTag for the base style of this type
		if ( RequiredStyleClass->IsChildOf(UUIStyle_Text::StaticClass()) )
		{
			Result = TEXT("DefaultTextStyle");
		}
		else if ( RequiredStyleClass->IsChildOf(UUIStyle_Image::StaticClass()) )
		{
			Result = TEXT("DefaultImageStyle");
		}
		else if ( RequiredStyleClass->IsChildOf(UUIStyle_Combo::StaticClass()) )
		{
			Result = TEXT("DefaultComboStyle");
		}
	}

	return Result;
}

/**
 * Returns the style data for the menu state specified.
 */
UUIStyle_Data* FUIStyleReference::GetStyleData( UUIState* MenuState ) const
{
	UUIStyle_Data* Result = NULL;

	if ( MenuState != NULL && ResolvedStyle != NULL )
	{
		Result = ResolvedStyle->GetStyleForState(MenuState);
	}

	return Result;
}

/**
 * Returns the style data for the menu state specified.
 */
UUIStyle_Data* FUIStyleReference::GetStyleDataByClass( UClass* MenuState ) const
{
	UUIStyle_Data* Result = NULL;

	if ( MenuState != NULL && ResolvedStyle != NULL )
	{
		Result = ResolvedStyle->GetStyleForStateByClass(MenuState);
	}

	return Result;
}

/**
 * Changes the style associated with this style reference.
 *
 * @param	NewStyle	the new style to assign to this style reference
 *
 * @return	TRUE if the style was successfully assigned to this reference.
 */
UBOOL FUIStyleReference::SetStyle( UUIStyle* NewStyle )
{
	UBOOL bResult = FALSE;

	if ( NewStyle != NULL )
	{
		if ( IsValidStyle(NewStyle) )
		{
			ResolvedStyle = NewStyle;

			// if the new style is the default style for this style reference, clear the assigned style id
			if ( !IsDefaultStyle(NewStyle) )
			{
				SetStyleID(NewStyle->StyleID);
			}
			else
			{
				SetStyleID(FGuid(0,0,0,0));
			}
			bResult = TRUE;
		}
	}
	else
	{
		if ( SetStyleID(FGuid(0,0,0,0)) )
		{
			InvalidateResolvedStyle();
		}
		bResult = TRUE;
	}

	return bResult;
}


/**
 * Changes the AssignedStyleID for this style reference
 *
 * @param	NewStyleID	the STYLE_ID for the UIStyle to link this style reference to
 *
 * @return	TRUE if the NewStyleID was different from the current value of AssignedStyleID
 */
UBOOL FUIStyleReference::SetStyleID( const FSTYLE_ID& NewStyleID )
{
	UBOOL bResult = AssignedStyleID != NewStyleID;
	AssignedStyleID = NewStyleID;
	return bResult;
}

/* ==========================================================================================================
	FUIStyleOverride
========================================================================================================== */
/**
 * Changes the draw color to the color specified and enables draw color override.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUIStyleOverride::SetCustomDrawColor( const FLinearColor& NewDrawColor )
{
	UBOOL bResult = DrawColor != NewDrawColor;

	DrawColor=NewDrawColor;

	return EnableCustomDrawColor() || bResult;
}


/**
 * Changes the opacity
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUIStyleOverride::SetCustomOpacity( float NewOpacity )
{
	UBOOL bResult = Opacity != NewOpacity;

	Opacity=NewOpacity;

	return EnableCustomOpacity() || bResult;
}

/**
 * Changes the padding for the specified orientation.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUIStyleOverride::SetCustomPadding( EUIOrientation Orientation, FLOAT NewPadding )
{
	checkSlow(Orientation<UIORIENT_MAX);

	const UBOOL bResult = Padding[Orientation] != NewPadding;
	Padding[Orientation] = NewPadding;

	return EnableCustomPadding() || bResult;
}

/**
 * Copies the value of DrawColor onto the specified value if draw color customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUIStyleOverride::CustomizeDrawColor( FLinearColor& OriginalColor ) const
{
	UBOOL bResult = FALSE;
	if ( IsCustomDrawColorEnabled() )
	{
		OriginalColor = DrawColor;
		CustomizeOpacity(OriginalColor);
		bResult = TRUE;
	}
	else
	{
		bResult = CustomizeOpacity(OriginalColor);
	}

	return bResult;
}

/**
 * Applies the value of Opacity tonto the specified value if draw color customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUIStyleOverride::CustomizeOpacity( FLinearColor& OriginalColor ) const
{
	if ( IsCustomOpacityEnabled() )
	{
		OriginalColor.A *= Opacity;
		return TRUE;
	}

	return FALSE;
}

/**
 * Applies the value of StylePadding onto the specified value if padding customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUIStyleOverride::CustomizePadding( EUIOrientation Orientation, FLOAT& OriginalPadding ) const
{
	checkSlow(Orientation<UIORIENT_MAX);

	if ( IsCustomPaddingEnabled() )
	{
		OriginalPadding = Padding[Orientation];
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
	FUITextStyleOverride
========================================================================================================== */
/**
 * Changes the draw font to the font specified and enables font override.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomDrawFont( UFont* NewFont )
{
	UBOOL bResult = DrawFont != NewFont;

	DrawFont = NewFont;

	return EnableCustomDrawFont() || bResult;
}

/**
 * Changes the custom attributes to the value specified and enables text attribute customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomAttributes( const FUITextAttributes& NewAttributes )
{
	UBOOL bResult = TextAttributes != NewAttributes;

	TextAttributes = NewAttributes;

	return EnableCustomAttributes() || bResult;
}

/**
 * Changes the custom text clipping mode to the value specified and enables clipmode customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomClipMode( ETextClipMode CustomClipMode )
{
	check(CustomClipMode<CLIP_MAX);

	UBOOL bResult = ClipMode != CustomClipMode;

	ClipMode = CustomClipMode;

	return EnableCustomClipMode() || bResult;
}

/**
 * Changes the custom text clip alignment to the value specified and enables clip alignment customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomClipAlignment( EUIAlignment NewClipAlignment )
{
	check(NewClipAlignment<UIALIGN_MAX);

	UBOOL bResult = ClipAlignment != NewClipAlignment;

	ClipAlignment = NewClipAlignment;

	return EnableCustomClipAlignment() || bResult;
}

/**
 * Changes the custom text alignment to the value specified and enables alignment customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomAlignment( EUIOrientation Orientation, EUIAlignment NewAlignment )
{
	check(Orientation<UIORIENT_MAX);
	check(NewAlignment<UIALIGN_MAX);

	UBOOL bResult = TextAlignment[Orientation] != NewAlignment;

	TextAlignment[Orientation] = NewAlignment;

	return EnableCustomAlignment() || bResult;
}

/**
 * Changes the custom text auto scale mode to the value specified and enables auto scale mode customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomAutoScaling( ETextAutoScaleMode NewAutoScaleMode, FLOAT NewMinScale )
{
	check(NewAutoScaleMode<UIAUTOSCALE_MAX);
	check(NewMinScale>=0.f);

	UBOOL bResult = AutoScaling.AutoScaleMode != NewAutoScaleMode || AutoScaling.MinScale != NewMinScale;

	AutoScaling.AutoScaleMode = NewAutoScaleMode;
	AutoScaling.MinScale = NewMinScale;

	return EnableCustomAutoScaleMode() || bResult;
}

/**
 * Changes the custom text auto scale mode to the value specified and enables auto scale mode customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomAutoScaling( const FTextAutoScaleValue& NewAutoScaleValue )
{
	check(NewAutoScaleValue.AutoScaleMode<UIAUTOSCALE_MAX);
	check(NewAutoScaleValue.MinScale>=0.f);

	UBOOL bResult = AutoScaling != NewAutoScaleValue;

	AutoScaling = NewAutoScaleValue;

	return EnableCustomAutoScaleMode() || bResult;
}

/**
 * Changes the custom text scale to the value specified and enables scale customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomScale( EUIOrientation Orientation, FLOAT NewScale )
{
	checkSlow(Orientation<UIORIENT_MAX);

	UBOOL bResult = DrawScale[Orientation] != NewScale;

	DrawScale[Orientation] = NewScale;

	return EnableCustomScale() || bResult;
}

/**
 * Changes the custom spacing adjust to the value specified and enables spacing adjust customization.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUITextStyleOverride::SetCustomSpacingAdjust( EUIOrientation Orientation, FLOAT NewSpacingAdjust )
{
	checkSlow(Orientation<UIORIENT_MAX);

	UBOOL bResult = SpacingAdjust[Orientation] != NewSpacingAdjust;

	SpacingAdjust[Orientation] = NewSpacingAdjust;

	return EnableCustomSpacingAdjust() || bResult;
}

/**
 * Copies the value of DrawFont onto the specified value if font customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeDrawFont( UFont*& OriginalFont ) const
{
	if ( IsCustomDrawFontEnabled() && DrawFont != NULL )
	{
		OriginalFont = DrawFont;
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of TextAttributes into the specified value if attribute customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeAttributes( FUITextAttributes& OriginalAttributes ) const
{
	if ( IsCustomAttributesEnabled() )
	{
		OriginalAttributes = TextAttributes;
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of ClipMode into the specified value if clipmode customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeClipMode( ETextClipMode& OriginalClipMode ) const
{
	if ( IsCustomClipModeEnabled() )
	{
		OriginalClipMode = static_cast<ETextClipMode>(ClipMode);
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of TextAlignment for the specified orientation into the specified value if alignment customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeAlignment( EUIOrientation Orientation, EUIAlignment& OriginalAlignment ) const
{
	check(Orientation<UIORIENT_MAX);

	if (IsCustomAlignmentEnabled()
	&&	static_cast<EUIAlignment>(TextAlignment[Orientation]) != UIALIGN_Default )
	{
		OriginalAlignment = static_cast<EUIAlignment>(TextAlignment[Orientation]);
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of ClipAlignment into the specified value if alignment customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeClipAlignment( EUIAlignment& OriginalAlignment ) const
{
	if (IsCustomClipAlignmentEnabled()
	&&	static_cast<EUIAlignment>(ClipAlignment) != UIALIGN_Default )
	{
		OriginalAlignment = static_cast<EUIAlignment>(ClipAlignment);
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of AutoScaleMode into the specified value if attribute customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeAutoScaling( FTextAutoScaleValue& OriginalAutoScaling ) const
{
	if ( IsCustomAutoScaleEnabled() )
	{
		OriginalAutoScaling = AutoScaling;
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of Scale into the specified value if attribute customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeScale( EUIOrientation Orientation, FLOAT& OriginalScale ) const
{
	if ( IsCustomScaleEnabled() )
	{
		checkSlow(Orientation<UIORIENT_MAX);

		OriginalScale = DrawScale[Orientation];
		return TRUE;
	}

	return FALSE;
}

/**
 * Copies the value of SpacingAdjust into the specified value if attribute customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUITextStyleOverride::CustomizeSpacingAdjust( EUIOrientation Orientation, FLOAT& OriginalSpacingAdjust ) const
{
	if ( IsCustomSpacingAdjustEnabled() )
	{
		checkSlow(Orientation<UIORIENT_MAX);

		OriginalSpacingAdjust = SpacingAdjust[Orientation];
		return TRUE;
	}

	return FALSE;
}

/* ==========================================================================================================
	FUIImageStyleOverride
========================================================================================================== */
/**
 * Changes the draw coordinates to the coordinates specified and enables coordinate override.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUIImageStyleOverride::SetCustomCoordinates( const FTextureCoordinates& NewCoordinates )
{
	UBOOL bResult = Coordinates != NewCoordinates;

	Coordinates=NewCoordinates;

	return EnableCustomCoordinates() || bResult;
}

/**
 * Changes the image adjustment data to the values specified and enables image adjustment data override.
 *
 * @return	TRUE if the value was changed; FALSE if the current value matched the new value or the new value
 *			otherwise couldn't be applied.
 */
UBOOL FUIImageStyleOverride::SetCustomFormatting( EUIOrientation Orientation, const FUIImageAdjustmentData& NewAdjustmentData )
{
	UBOOL bResult = Formatting[Orientation] != NewAdjustmentData;

	Formatting[Orientation]=NewAdjustmentData;

	return EnableCustomFormatting() || bResult;
}


/**
 * Copies the value of Coordinates onto the specified value if coordinates customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUIImageStyleOverride::CustomizeCoordinates( FTextureCoordinates& OriginalCoordinates ) const
{
	if ( IsCustomCoordinatesEnabled() )
	{
		OriginalCoordinates = Coordinates;
		return TRUE;
	}
	
	return FALSE;
}

/**
 * Copies the value of Formatting for the specified orientation onto the specified value if formatting customization is enabled.
 *
 * @return	TRUE if the input value was modified.
 */
UBOOL FUIImageStyleOverride::CustomizeFormatting( EUIOrientation Orientation, FUIImageAdjustmentData& OriginalFormatting ) const
{
	if ( IsCustomFormattingEnabled() )
	{
		OriginalFormatting = Formatting[Orientation];
		return TRUE;
	}
	
	return FALSE;
}

/* ==========================================================================================================
	FUICombinedStyleData
========================================================================================================== */
/** Default Constructor */
FUICombinedStyleData::FUICombinedStyleData()
{
	// zero initialize all members
	appMemzero(this, sizeof(FUICombinedStyleData));
	TextClipMode = CLIP_MAX;
}

/** Copy constructor */
FUICombinedStyleData::FUICombinedStyleData( const FUICombinedStyleData& Other )
{
	TextColor = Other.TextColor;
	ImageColor = Other.ImageColor;
	DrawFont = Other.DrawFont;
	FallbackImage = Other.FallbackImage;
	AtlasCoords = Other.AtlasCoords;

	TextAttributes = Other.TextAttributes;
	for ( INT i = 0; i < UIORIENT_MAX; i++ )
	{
		TextAlignment[i] = Other.TextAlignment[i];
		AdjustmentType[i] = Other.AdjustmentType[i];
		TextPadding[i] = Other.TextPadding[i];
		ImagePadding[i] = Other.ImagePadding[i];
	}

	TextClipMode = Other.TextClipMode;
	TextClipAlignment = Other.TextClipAlignment;
	TextAutoScaling = Other.TextAutoScaling;
	TextScale = Other.TextScale;
	TextSpacingAdjust = Other.TextSpacingAdjust;
	bInitialized = Other.bInitialized;
}

/**
 * Standard constructor
 *
 * @param	SourceStyle		the style to use for initializing this StyleDataContainer.
 */
FUICombinedStyleData::FUICombinedStyleData( UUIStyle_Data* SourceStyle )
{
	bInitialized = FALSE;
	InitializeStyleDataContainer(SourceStyle);
}

/**
 * Initializes the values of this UICombinedStyleData based on the values of the UIStyle_Data specified.
 *
 * @param	SourceStyle			the style to copy values from
 * @param	bClearUnusedData	controls whether style data that isn't found in SourceStyle should be zero'd; for example
 *								if SourceStyle is a text style, the image style data in this struct will be cleared if
 *								bClearUnusedData is TRUE, or left alone if FALSE 
 */
void FUICombinedStyleData::InitializeStyleDataContainer( UUIStyle_Data* SourceStyle, UBOOL bClearUnusedData/*=TRUE*/ )
{
	check(SourceStyle);

	// if this style data container has never been initialized, always clear the unused style data to guarantee that
	// the unused style data does not remain uninitialized.
	if ( !bInitialized )
	{
		bClearUnusedData = TRUE;
	}

	// set the flag to indicate that this style data container has been initialized at least once
	bInitialized = TRUE;

	UUIStyle_Combo* ComboStyleData = Cast<UUIStyle_Combo>(SourceStyle);
	UUIStyle_Text* TextStyleData = NULL;
	UUIStyle_Image* ImageStyleData = NULL;

	if ( ComboStyleData != NULL )
	{
		TextStyleData = Cast<UUIStyle_Text>(ComboStyleData->TextStyle.GetStyleData());
		ImageStyleData = Cast<UUIStyle_Image>(ComboStyleData->ImageStyle.GetStyleData());
	}
	else
	{
		TextStyleData = Cast<UUIStyle_Text>(SourceStyle);
		ImageStyleData = Cast<UUIStyle_Image>(SourceStyle);
	}

	if ( TextStyleData != NULL )
	{
		// copy the values from the text style data
		DrawFont = TextStyleData->StyleFont;
		checkf(DrawFont, TEXT("NULL StyleFont for text style data %s from style %s"), *SourceStyle->GetFullName(),
			SourceStyle->GetOwnerStyle() ? *SourceStyle->GetOwnerStyle()->GetStyleName() : TEXT("N/A"));

		TextColor = TextStyleData->StyleColor;
		TextAttributes = TextStyleData->Attributes;
		for ( INT i = 0; i < UIORIENT_MAX; i++ )
		{
			TextAlignment[i] = TextStyleData->Alignment[i];
			TextPadding[i] = TextStyleData->StylePadding[i];
		}
		TextClipMode = TextStyleData->ClipMode;
		TextClipAlignment = TextStyleData->ClipAlignment;
		TextAutoScaling = TextStyleData->AutoScaling;
		TextScale = TextStyleData->Scale;
		TextSpacingAdjust = TextStyleData->SpacingAdjust;
	}
	else if ( bClearUnusedData )
	{
		// reset the values for the text style data
		TextColor = FLinearColor(0.f,0.f,0.f);
		DrawFont = NULL;
		TextAttributes.Reset();
		for ( INT i = 0; i < UIORIENT_MAX; i++ )
		{
			TextAlignment[i] = UIALIGN_MAX;
			TextPadding[i] = 0.f;
		}
		TextClipMode = CLIP_MAX;
		TextClipAlignment = UIALIGN_MAX;
		TextAutoScaling = FTextAutoScaleValue(EC_EventParm);
		TextScale = FVector2D(1.0f, 1.0f);
		TextSpacingAdjust = FVector2D( 0.0f, 0.0f );
	}

	if ( ImageStyleData != NULL )
	{
		ImageColor = ImageStyleData->StyleColor;
		FallbackImage = ImageStyleData->DefaultImage;
		AtlasCoords = ImageStyleData->Coordinates;

		// copy the values from the image style data
		for ( INT i = 0; i < UIORIENT_MAX; i++ )
		{
			AdjustmentType[i] = ImageStyleData->AdjustmentType[i];
			ImagePadding[i] = ImageStyleData->StylePadding[i];
		}
	}
	else if ( bClearUnusedData )
	{
		// reset the values from the image style data
		ImageColor = FLinearColor(0.f,0.f,0.f);
		FallbackImage = NULL;
		AtlasCoords = FTextureCoordinates(0,0,0,0);
		appMemzero(AdjustmentType, sizeof(AdjustmentType));
		appMemzero(ImagePadding, sizeof(ImagePadding));
	}
}

/** Comparison operators */
UBOOL FUICombinedStyleData::operator==( const FUICombinedStyleData& Other ) const
{
	return	TextColor			== Other.TextColor
		&&	ImageColor			== Other.ImageColor
		&&	bInitialized		== Other.bInitialized
		&&	DrawFont			== Other.DrawFont
		&&	FallbackImage		== Other.FallbackImage
		&&	AtlasCoords			== Other.AtlasCoords
		&&	TextAttributes		== Other.TextAttributes
		&&	TextClipMode		== Other.TextClipMode
		&&	TextClipAlignment	== Other.TextClipAlignment
		&&	TextAutoScaling		== Other.TextAutoScaling
		&&  TextScale.Equals(Other.TextScale,DELTA)
		&&	TextSpacingAdjust.Equals(Other.TextSpacingAdjust,DELTA)
		&&	!appMemcmp(AdjustmentType, Other.AdjustmentType, sizeof(AdjustmentType))
		&&	TextAlignment[UIORIENT_Horizontal] == Other.TextAlignment[UIORIENT_Horizontal]
		&&	TextAlignment[UIORIENT_Vertical] == Other.TextAlignment[UIORIENT_Vertical]
		&&	ARE_FLOATS_EQUAL(TextPadding[UIORIENT_Horizontal],Other.TextPadding[UIORIENT_Horizontal])
		&&	ARE_FLOATS_EQUAL(TextPadding[UIORIENT_Vertical],Other.TextPadding[UIORIENT_Vertical])
		&&	ARE_FLOATS_EQUAL(ImagePadding[UIORIENT_Horizontal],Other.ImagePadding[UIORIENT_Horizontal])
		&&	ARE_FLOATS_EQUAL(ImagePadding[UIORIENT_Vertical],Other.ImagePadding[UIORIENT_Vertical]);
}
UBOOL FUICombinedStyleData::operator!=( const FUICombinedStyleData& Other ) const
{
	return !(operator==(Other));
}

/* ==========================================================================================================
	FUIStyleResolverReference
========================================================================================================== */
FUIStyleSubscriberReference::FUIStyleSubscriberReference( FName InSubscriberId, const TScriptInterface<IUIStyleResolver>& InSubscriber )
: SubscriberId(InSubscriberId), Subscriber(InSubscriber)
{
}


/* ==========================================================================================================
	FStyleReferenceId
========================================================================================================== */
/**
 * Returns the display name for this style reference
 */
FString FStyleReferenceId::GetStyleReferenceName( UBOOL bAllowDisplayName/*=!GIsGame*/ ) const
{
	if ( StyleReferenceTag != NAME_None )
	{
		return StyleReferenceTag.ToString();
	}
	else if ( StyleProperty != NULL && bAllowDisplayName )
	{
		FString DisplayName = StyleProperty->GetMetaData(TEXT("FriendlyName"));
		if ( DisplayName.Len() == 0 )
		{
			DisplayName = StyleProperty->GetName();
		}

		return DisplayName;
	}

	return TEXT("");
}


/**
 * Faster version of GetStyleReferenceName which never allows meta data localized text to be used.
 */
FName FStyleReferenceId::GetStyleReferenceTag() const
{
	if ( StyleReferenceTag != NAME_None )
	{
		return StyleReferenceTag;
	}
	else if ( StyleProperty != NULL )
	{
		return StyleProperty->GetFName();
	}

	return NAME_None;
}




// EOF

