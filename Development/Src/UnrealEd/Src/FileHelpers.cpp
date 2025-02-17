/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FileHelpers.h"
#include "LevelUtils.h"
#include "UnLinkedObjEditor.h"
#include "Kismet.h"
#include "Sentinel.h"
#include "BusyCursor.h"
#include "Database.h"

/**
 * Queries the user if they want to quit out of interpolation editing before save.
 *
 * @return		TRUE if in interpolation editing mode, FALSE otherwise.
 */
static UBOOL InInterpEditMode()
{
	// Must exit Interpolation Editing mode before you can save - so it can reset everything to its initial state.
	if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
	{
		const UBOOL ExitInterp = appMsgf( AMT_YesNo, *LocalizeUnrealEd("Prompt_21") );
		if(!ExitInterp)
		{
			return TRUE;
		}

		GEditorModeTools().SetCurrentMode( EM_Default );
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////

class WxDlgNewLevel : public wxDialog
{
public:
	WxDlgNewLevel(wxWindow* InParent)
	{
		const bool bSuccess = wxXmlResource::Get()->LoadDialog( this, InParent, TEXT("ID_DLG_NEW_LEVEL") );
		check( bSuccess );
		AdditiveRadio = (wxRadioButton*)FindWindow( XRCID( "IDRB_ADDITIVE" ) );
		check( AdditiveRadio != NULL );
		FLocalizeWindow( this );
	}

	wxRadioButton *AdditiveRadio;
};

/**
 * Maps loaded level packages to the package filenames.
 */
static TMap<FName, FFilename> LevelFilenames;

/**
 * Clears out all registered level package filenames, adds an 
 */
static void ResetLevelFilenames()
{
	// Empty out any existing filenames.
	LevelFilenames.Empty();

	// Register a blank filename
	const FName PackageName(*GWorld->GetOutermost()->GetName());
	const FFilename EmptyFilename;
	LevelFilenames.Set( PackageName, EmptyFilename );

	GApp->EditorFrame->RefreshCaption( &EmptyFilename );
}

static void RegisterLevelFilename(UObject* Object, const FFilename& NewLevelFilename)
{
	const FName PackageName(*Object->GetOutermost()->GetName());
	//debugf(TEXT("RegisterLevelFilename: package %s to name %s"), *PackageName, *NewLevelFilename );
	FFilename* ExistingFilenamePtr = LevelFilenames.Find( PackageName );
	if ( ExistingFilenamePtr )
	{
		// Update the existing entry with the new filename.
		*ExistingFilenamePtr = NewLevelFilename;
	}
	else
	{
		// Set for the first time.
		LevelFilenames.Set( PackageName, NewLevelFilename );
	}

	// Mirror the world's filename to UnrealEd's title bar.
	if ( Object == GWorld )
	{
		GApp->EditorFrame->RefreshCaption( &NewLevelFilename );
	}
}

///////////////////////////////////////////////////////////////////////////////

static FFilename GetFilename(const FName& PackageName)
{
	FFilename* Result = LevelFilenames.Find( PackageName );
	if ( !Result )
	{
		//debugf(TEXT("GetFilename with package %s, returning EMPTY"), *PackageName );
		return FFilename(TEXT(""));
	}

	//debugf(TEXT("GetFilename with package %s, returning %s"), *PackageName, **Result );
	return *Result;
}

static FFilename GetFilename(UObject* LevelObject)
{
	return GetFilename( LevelObject->GetOutermost()->GetFName() );
}

///////////////////////////////////////////////////////////////////////////////

static const FString& GetDefaultDirectory()
{
	return GApp->LastDir[LD_UNR];
}

///////////////////////////////////////////////////////////////////////////////

enum EFileInteraction
{
	FI_Load,
	FI_Save,
	FI_Import,
	FI_Export
};

/**
* Returns a file filter string appropriate for a specific file interaction.
*
* @param	Interaction		A file interaction to get a filter string for.
* @return					A filter string.
*/
static FString GetFilter(EFileInteraction Interaction)
{
	FString Result;

	switch( Interaction )
	{
	case FI_Load:
	case FI_Save:
		Result = FString::Printf( TEXT("Map files (*.%s)|*.%s|All files (*.*)|*.*"), *GApp->MapExt, *GApp->MapExt );
		break;

	case FI_Import:
		Result = TEXT("Unreal Text (*.t3d)|*.t3d|All Files|*.*");
		break;

	case FI_Export:
		Result = TEXT("Supported formats (*.t3d,*.stl)|*.t3d;*.stl|Unreal Text (*.t3d)|*.t3d|Stereo Litho (*.stl)|*.stl|Object (*.obj)|*.obj|All Files|*.*");
		break;

	default:
		checkMsg( 0, "Unkown EFileInteraction" );
	}

	return Result;
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Writes current viewport views into the specified world in preparation for saving.
 */
static void SaveViewportViews(UWorld* World)
{
	// @todo: Support saving and restoring floating viewports with level?

	// Check that this world is GWorld to avoid stomping on the saved views of sub-levels.
	if ( World == GWorld )
	{
		for(INT i=0; i<4; i++)
		{
			WxLevelViewportWindow* VC = GApp->EditorFrame->ViewportConfigData->Viewports[i].ViewportWindow;
			if(VC && (VC->ViewportType != LVT_None))
			{
				check(VC->ViewportType >= 0 && VC->ViewportType < 4);
				World->EditorViews[VC->ViewportType] = FLevelViewportInfo( VC->ViewLocation, VC->ViewRotation, VC->OrthoZoom );	
			}
		}
	}
}

/**
 * Reads views from the specified world into the current views.
 */
static void RestoreViewportViews(UWorld* World)
{
	// @todo: Support saving and restoring floating viewports with level?

	// Check that this world is GWorld to avoid stomping on the saved views of sub-levels.
	if ( World == GWorld )
	{
		for( INT i=0 ; i<4; i++)
		{
			WxLevelViewportWindow* VC = GApp->EditorFrame->ViewportConfigData->Viewports[i].ViewportWindow;
			if( VC && (VC->ViewportType != LVT_None) )
			{
				check(VC->ViewportType >= 0 && VC->ViewportType < 4);
				VC->ViewLocation	= World->EditorViews[VC->ViewportType].CamPosition;
				VC->ViewRotation	= World->EditorViews[VC->ViewportType].CamRotation;
				VC->OrthoZoom		= World->EditorViews[VC->ViewportType].CamOrthoZoom;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

/**
 * Presents the user with a file dialog whose contents are intially InFilename.
 *
 * @return		TRUE if OK was clicked and OutFilename contains a result, FALSE otherwise.
 */
static UBOOL PresentFileDialog(const FFilename& InFilename, FFilename& OutFilename)
{
	WxFileDialog dlg( GApp->EditorFrame,
					  *LocalizeUnrealEd("SaveAs"),
					  *GetDefaultDirectory(),
					  *InFilename.GetCleanFilename(),
					  *GetFilter(FI_Save),
					  wxSAVE | wxOVERWRITE_PROMPT );

	// Disable autosaving while the "Save As..." dialog is up.
	const UBOOL bOldAutoSave = GUnrealEd->AutoSave;
	GUnrealEd->AutoSave = FALSE;

	const UBOOL bOK = (dlg.ShowModal() == wxID_OK);

	// Restore autosaving to its previous state.
	GUnrealEd->AutoSave = bOldAutoSave;

	if( bOK )
	{
		OutFilename = FFilename( dlg.GetPath() );
	}

	return bOK;
}

/**
 * @param	World					The world to save.
 * @param	ForceFilename			If non-NULL, save the level package to this name (full path+filename).
 * @param	OverridePath			If non-NULL, override the level path with this path.
 * @param	FilenamePrefix			If non-NULL, prepend this string the the level filename.
 * @param	bCollectGarbage			If TRUE, request garbage collection before saving.
 * @param	bRenamePackageToFile	If TRUE, rename the level package to the filename if save was successful.
 * @param	bCheckDirty				If TRUE, don't save the level if it is not dirty.
 * @param	FinalFilename			[out] The full path+filename the level was saved to.
 * @param	bAutosaving				Should be set to TRUE if autosaving; passed to UWorld::SaveWorld.
 * @param	bPIESaving				Should be set to TRUE if saving for PIE; passed to UWorld::SaveWorld.
 * @return							TRUE if the level was saved.
 */
static UBOOL SaveWorld(UWorld* World,
					   const FFilename* ForceFilename,
					   const TCHAR* OverridePath,
					   const TCHAR* FilenamePrefix,
					   UBOOL bCollectGarbage,
					   UBOOL bRenamePackageToFile,
					   UBOOL bCheckDirty,
					   FString& FinalFilename,
					   UBOOL bAutosaving,
					   UBOOL bPIESaving)
{
	if ( !World )
	{
		return FALSE;
	}

	UPackage* Package = Cast<UPackage>( World->GetOuter() );
	if ( !Package )
	{
		return FALSE;
	}

	// Don't save if the world doesn't need saving.
	if ( bCheckDirty && !Package->IsDirty() )
	{
		return FALSE;
	}

	FString PackageName = Package->GetName();

	FFilename	ExistingFilename;
	FString		Path;
	FFilename	CleanFilename;

	// Does a filename already exist for this package?
	const UBOOL bPackageExists = GPackageFileCache->FindPackageFile( *PackageName, NULL, ExistingFilename );

	if ( ForceFilename )
	{
		Path				= ForceFilename->GetPath();
		CleanFilename		= ForceFilename->GetCleanFilename();
	}
	else if ( bPackageExists )
	{
		// We're not forcing a filename, so go with the filename that exists.
		Path			= ExistingFilename.GetPath();
		CleanFilename	= ExistingFilename.GetCleanFilename();
	}
	else
	{
		// No package filename exists and none was specified, so save the package in the autosaves folder.
		Path			= GEditor->AutoSaveDir;
		CleanFilename	= PackageName + TEXT(".") + FURL::DefaultMapExt;
	}

	// Optionally override path.
	if ( OverridePath )
	{
		FinalFilename = FString(OverridePath) + PATH_SEPARATOR;
	}
	else
	{
		FinalFilename = Path + PATH_SEPARATOR;
	}

	// Apply optional filename prefix.
	if ( FilenamePrefix )
	{
		FinalFilename += FString(FilenamePrefix);
	}

	// Munge remaining clean filename minus path + extension with path and optional prefix.
	FinalFilename += CleanFilename;

	// Before doing any work, check to see if 1) the package name is in use by another object; and 2) the file is writable.
	UBOOL bSuccess = FALSE;

	if ( Package->Rename( *CleanFilename.GetBaseFilename(), NULL, REN_Test ) == FALSE )
	{
		appMsgf(AMT_OK,	*LocalizeUnrealEd("Error_PackageNameExists"));
	}
	else if(GFileManager->IsReadOnly(*FinalFilename) == FALSE)
	{
		// Save the world package after doing optional garbage collection.
		const FScopedBusyCursor BusyCursor;

		FString MapFileName = FFilename( FinalFilename ).GetCleanFilename();
		const FString LocalizedSavingMap(
			*FString::Printf( LocalizeSecure( LocalizeUnrealEd( TEXT("SavingMap_F") ), *MapFileName ) ) );
		GWarn->BeginSlowTask( *LocalizedSavingMap, TRUE );

		bSuccess = World->SaveWorld( FinalFilename, bCollectGarbage, bAutosaving, bPIESaving );
		GWarn->EndSlowTask();

		if( bRenamePackageToFile )
		{
			// If the package was saved successfully, make sure the UPackage has the same name as the file.
			if ( bSuccess )
			{
				Package->Rename( *CleanFilename.GetBaseFilename(), NULL, REN_ForceNoResetLoaders );
			}
		}
	}
	else
	{
		appMsgf(AMT_OK,	LocalizeSecure(LocalizeUnrealEd("PackageFileIsReadOnly"), *FinalFilename));
	}

	return bSuccess;
}


/** Renames a single level, preserving the common suffix */
UBOOL RenameStreamingLevel( FString& LevelToRename, const FString& OldBaseLevelName, const FString& NewBaseLevelName )
{
	// Make sure the level starts with the original level name
	if( LevelToRename.StartsWith( OldBaseLevelName ) )	// Not case sensitive
	{
		// Grab the tail of the streaming level name, basically everything after the old base level name
		FString SuffixToPreserve = LevelToRename.Right( LevelToRename.Len() - OldBaseLevelName.Len() );

		// Rename the level!
		LevelToRename = NewBaseLevelName + SuffixToPreserve;

		return TRUE;
	}

	return FALSE;
}



/**
 * Renames streaming level names in all sequence objects and sub-sequence objects
 */
UBOOL RenameStreamingLevelsInSequenceRecursively( USequence* Sequence, const FString& OldBaseLevelName, const FString& NewBaseLevelName, const UBOOL bDoReplace )
{
	UBOOL bFoundAnythingToRename = FALSE;

	if( Sequence != NULL )
	{
		// Iterate over Objects in this Sequence
		for ( INT OpIndex = 0; OpIndex < Sequence->SequenceObjects.Num(); OpIndex++ )
		{
			USequenceOp* SeqOp = Cast<USequenceOp>( Sequence->SequenceObjects(OpIndex) );
			if ( SeqOp != NULL )
			{
				// If this is a subsequence, then recurse
				USequence* SubSeq = Cast<USequence>( SeqOp );
				if( SubSeq != NULL )
				{
					if( RenameStreamingLevelsInSequenceRecursively( SubSeq, OldBaseLevelName, NewBaseLevelName, bDoReplace ) )
					{
						bFoundAnythingToRename = TRUE;
					}
				}
				else
				{
					// Process USeqAct_LevelStreaming nodes
					USeqAct_LevelStreaming* LevelStreamingSeqAct = Cast< USeqAct_LevelStreaming >( SeqOp );
					if( LevelStreamingSeqAct != NULL )
					{
						FString LevelNameToRename = LevelStreamingSeqAct->LevelName.ToString();
						if( RenameStreamingLevel( LevelNameToRename, OldBaseLevelName, NewBaseLevelName ) )
						{
							bFoundAnythingToRename = TRUE;
							if( bDoReplace )
							{
								// Level was renamed!
								LevelStreamingSeqAct->LevelName = FName( *LevelNameToRename );
								LevelStreamingSeqAct->MarkPackageDirty();
							}
						}
					}

					// Process USeqAct_LevelVisibility nodes
					USeqAct_LevelVisibility* LevelVisibilitySeqAct = Cast< USeqAct_LevelVisibility >( SeqOp );
					if( LevelVisibilitySeqAct != NULL )
					{
						FString LevelNameToRename = LevelVisibilitySeqAct->LevelName.ToString();
						if( RenameStreamingLevel( LevelNameToRename, OldBaseLevelName, NewBaseLevelName ) )
						{
							bFoundAnythingToRename = TRUE;
							if( bDoReplace )
							{
								// Level was renamed!
								LevelVisibilitySeqAct->LevelName = FName( *LevelNameToRename );
								LevelVisibilitySeqAct->MarkPackageDirty();
							}
						}
					}

					// Process USeqAct_MultiLevelStreaming nodes
					USeqAct_MultiLevelStreaming* MultiLevelStreamingSeqAct = Cast< USeqAct_MultiLevelStreaming >( SeqOp );
					if( MultiLevelStreamingSeqAct != NULL )
					{
						for( INT CurLevelIndex = 0; CurLevelIndex < MultiLevelStreamingSeqAct->Levels.Num(); ++CurLevelIndex )
						{
							FString LevelNameToRename = MultiLevelStreamingSeqAct->Levels( CurLevelIndex ).LevelName.ToString();
							if( RenameStreamingLevel( LevelNameToRename, OldBaseLevelName, NewBaseLevelName ) )
							{
								bFoundAnythingToRename = TRUE;
								if( bDoReplace )
								{
									// Level was renamed!
									MultiLevelStreamingSeqAct->Levels( CurLevelIndex ).LevelName = FName( *LevelNameToRename );
									MultiLevelStreamingSeqAct->MarkPackageDirty();
								}
							}
						}
					}
				}
			}
		}
	}

	return bFoundAnythingToRename;
}


/**
 * Prompts the user with a dialog for selecting a filename.
 */
static UBOOL SaveAsImplementation( const FFilename& DefaultFilename, const UBOOL bAllowStreamingLevelRename )
{
	WxFileDialog dlg( GApp->EditorFrame,
					  *LocalizeUnrealEd("SaveAs"),
					  *GetDefaultDirectory(),
					  *DefaultFilename.GetCleanFilename(),
					  *GetFilter(FI_Save),
					  wxSAVE | wxOVERWRITE_PROMPT );

	// Disable autosaving while the "Save As..." dialog is up.
	const UBOOL bOldAutoSave = GUnrealEd->AutoSave;
	GUnrealEd->AutoSave = FALSE;

	UBOOL bStatus = FALSE;
	if( dlg.ShowModal() == wxID_OK )
	{
		// Check to see if there are streaming level associated with the P map, and if so, we'll
		// prompt to rename those and fixup all of the named-references to levels in the maps.
		UBOOL bCanRenameStreamingLevels = FALSE;
		FString OldBaseLevelName, NewBaseLevelName;

		if( bAllowStreamingLevelRename )
		{
			const FString OldLevelName = DefaultFilename.GetBaseFilename();
			const FString NewLevelName = FFilename( dlg.GetPath() ).GetBaseFilename();

			// The old and new level names must have a common suffix.  We'll detect that now.
			INT NumSuffixChars = 0;
			{
				for( INT CharsFromEndIndex = 0; ; ++CharsFromEndIndex )
				{
					const INT OldLevelNameCharIndex = ( OldLevelName.Len() - 1 ) - CharsFromEndIndex;
					const INT NewLevelNameCharIndex = ( NewLevelName.Len() - 1 ) - CharsFromEndIndex;
					
					if( OldLevelNameCharIndex <= 0 || NewLevelNameCharIndex <= 0 )
					{
						// We've processed all characters in at least one of the strings!
						break;
					}

					if( appToUpper( OldLevelName[ OldLevelNameCharIndex ] ) != appToUpper( NewLevelName[ NewLevelNameCharIndex ] ) )
					{
						// Characters don't match.  We have the common suffix now.
						break;
					}

					// We have another common character in the suffix!
					++NumSuffixChars;
				}

			}

			
			// We can only proceed if we found a common suffix
			if( NumSuffixChars > 0 )
			{
				FString CommonSuffix = NewLevelName.Right( NumSuffixChars );

				OldBaseLevelName = OldLevelName.Left( OldLevelName.Len() - CommonSuffix.Len() );
				NewBaseLevelName = NewLevelName.Left( NewLevelName.Len() - CommonSuffix.Len() );


				// OK, make sure this is really the persistent level
				if( GWorld->CurrentLevel == GWorld->PersistentLevel )
				{
					// Check to see if we actually have anything to rename
					UBOOL bAnythingToRename = FALSE;
					{
						// Check for Kismet sequences to rename
						USequence* LevelKismet = GWorld->GetGameSequence( NULL ); // CurLevel );
						if( LevelKismet != NULL )
						{
							const UBOOL bDoReplace = FALSE;
							if( RenameStreamingLevelsInSequenceRecursively( LevelKismet, OldBaseLevelName, NewBaseLevelName, bDoReplace ) )
							{
								bAnythingToRename = TRUE;
							}
						}

						// Check for contained streaming levels
						AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
						if( WorldInfo != NULL )
						{
							for( INT CurStreamingLevelIndex = 0; CurStreamingLevelIndex < WorldInfo->StreamingLevels.Num(); ++CurStreamingLevelIndex )
							{
								ULevelStreaming* CurStreamingLevel = WorldInfo->StreamingLevels( CurStreamingLevelIndex );
								if( CurStreamingLevel != NULL )
								{
									// Update the package name
									FString PackageNameToRename = CurStreamingLevel->PackageName.ToString();
									if( RenameStreamingLevel( PackageNameToRename, OldBaseLevelName, NewBaseLevelName ) )
									{
										bAnythingToRename = TRUE;
									}
								}
							}
						}
					}

					if( bAnythingToRename )
					{
						// OK, we can go ahead and rename levels
						bCanRenameStreamingLevels = TRUE;
					}
				}
			}
		}


		if( bCanRenameStreamingLevels )
		{
			// Prompt to update streaming levels and such
			// Return value:  0 = yes, 1 = no, 2 = cancel
			const INT DlgResult =
				appMsgf( AMT_YesNoCancel,
						 *FString::Printf( LocalizeSecure( LocalizeUnrealEd( "SaveLevelAs_PromptToRenameStreamingLevels_F" ), *DefaultFilename.GetBaseFilename(), *FFilename( dlg.GetPath() ).GetBaseFilename() ) ) );
			
			if( DlgResult != 2 )	// Cancel?
			{
				if( DlgResult == 0 )	// Yes?
				{
					// Rename all named level references in Kismet
					USequence* LevelKismet = GWorld->GetGameSequence( NULL ); // CurLevel );
					if( LevelKismet != NULL )
					{
						const UBOOL bDoReplace = TRUE;
						if( RenameStreamingLevelsInSequenceRecursively( LevelKismet, OldBaseLevelName, NewBaseLevelName, bDoReplace ) )
						{
							// We renamed something!
						}
					}

					// Update streaming level names
					AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
					if( WorldInfo != NULL )
					{
						for( INT CurStreamingLevelIndex = 0; CurStreamingLevelIndex < WorldInfo->StreamingLevels.Num(); ++CurStreamingLevelIndex )
						{
							ULevelStreaming* CurStreamingLevel = WorldInfo->StreamingLevels( CurStreamingLevelIndex );
							if( CurStreamingLevel != NULL )
							{
								// Update the package name
								FString PackageNameToRename = CurStreamingLevel->PackageName.ToString();
								if( RenameStreamingLevel( PackageNameToRename, OldBaseLevelName, NewBaseLevelName ) )
								{
									CurStreamingLevel->PackageName = FName( *PackageNameToRename );

									// Level was renamed!
									CurStreamingLevel->MarkPackageDirty();
								}
							}
						}
					}
				}

				// Save the level!
				bStatus = FEditorFileUtils::SaveMap( GWorld, FString( dlg.GetPath() ) );
			}
			else
			{
				// User canceled, nothing to do.
			}
		}
		else
		{
			// Save the level
			bStatus = FEditorFileUtils::SaveMap( GWorld, FString( dlg.GetPath() ) );
		}

	}

	// Restore autosaving to its previous state.
	GUnrealEd->AutoSave = bOldAutoSave;

	return bStatus;

}

/**
 * Saves the file, checking first if we have a valid filename to save the file as.
 *
 * @param	LevelObject		The level object.
 * @param	Filename		The filename to use.
 * @return					TRUE if a valid filename exists, or FALSE otherwise.
 */
static UBOOL CheckSaveAs(UObject* LevelObject, const FString& Filename)
{
	if( !Filename.Len() )
	{
		const UBOOL bAllowStreamingLevelRename = FALSE;
		return SaveAsImplementation( Filename, bAllowStreamingLevelRename );
	}
	else
	{
		return FEditorFileUtils::SaveMap( GWorld, Filename );
	}
}

static void ImportFile(const FFilename& InFilename, UBOOL bMerging)
{
	const FScopedBusyCursor BusyCursor;

	FString MapFileName = InFilename.GetCleanFilename();
	const FString LocalizedImportingMap(
		*FString::Printf( LocalizeSecure( LocalizeUnrealEd( TEXT("ImportingMap_F") ), *MapFileName ) ) );

	GWarn->BeginSlowTask( *LocalizedImportingMap, TRUE );

	if( bMerging )
	{
		GUnrealEd->Exec( *FString::Printf( TEXT("MAP IMPORTADD FILE=\"%s\""), *InFilename ) );
	}
	else
	{
		GUnrealEd->Exec( *FString::Printf( TEXT("MAP IMPORT FILE=\"%s\""), *InFilename ) );
		ResetLevelFilenames();
	}

	GWarn->EndSlowTask();

	GUnrealEd->RedrawLevelEditingViewports();

	GApp->LastDir[LD_UNR] = InFilename.GetPath();

	GCallbackEvent->Send(CALLBACK_RefreshEditor_AllBrowsers);
}

/**
 * @return		TRUE if GWorld's package is dirty.
 */
static UBOOL IsWorldDirty()
{
	UPackage* Package = CastChecked<UPackage>(GWorld->GetOuter());
	return Package->IsDirty();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FEditorFileUtils
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Does a saveAs for the specified level.
 *
 * @param	Level		The level to be SaveAs'd.
 * @return				TRUE if the level was saved.
 */
UBOOL FEditorFileUtils::SaveAs(UObject* LevelObject)
{
	const FFilename DefaultFilename = GetFilename( LevelObject );

	// We'll allow the map to be renamed when saving a level as a new file name this way
	const UBOOL bAllowStreamingLevelRename = TRUE;

	return SaveAsImplementation( DefaultFilename, bAllowStreamingLevelRename );
}

/**
 * Checks to see if GWorld's package is dirty and if so, asks the user if they want to save it.
 * If the user selects yes, does a save or SaveAs, as necessary.
 */	
UBOOL FEditorFileUtils::AskSaveChanges()
{
	if( IsWorldDirty() )
	{
		const FFilename LevelPackageFilename = GetFilename( GWorld );
		const FString Question = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_20"), ( LevelPackageFilename.Len() ? *LevelPackageFilename.GetBaseFilename() : *LocalizeUnrealEd("Untitled") )) );
		if( appMsgf( AMT_YesNo, *Question ) )
		{
			return CheckSaveAs( GWorld, LevelPackageFilename );
		}
	}

	// Update cull distance volumes (and associated primitives).
	GWorld->UpdateCullDistanceVolumes();

	// Get the set of all reference worlds, excluding GWorld.
	TArray<UWorld*> WorldsArray;
	FLevelUtils::GetWorlds( WorldsArray, FALSE );

	// Calling FLevelUtils::GetWorlds forces a garbage collect, so we shouldn't need to collect garbage again
	UBOOL bAlreadyCollectedGarbage = TRUE;

	for ( INT WorldIndex = 0 ; WorldIndex < WorldsArray.Num() ; ++WorldIndex )
	{
		UWorld* World = WorldsArray( WorldIndex );
		UPackage* Package = Cast<UPackage>( World->GetOuter() );
		check( Package );

		// If this world needs saving . . .
		if ( Package->IsDirty() )
		{
			FString FinalFilename;
			const FString Question = FString::Printf( LocalizeSecure(LocalizeUnrealEd("Prompt_20"), *Package->GetName()) );
			if( appMsgf( AMT_YesNo, *Question ) )
			{
				SaveWorld( World, NULL,
						   NULL, NULL,
						   !bAlreadyCollectedGarbage,
						   TRUE, FALSE,
						   FinalFilename,
						   FALSE, FALSE );

				// Collect garbage if necessary.
				if ( !bAlreadyCollectedGarbage )
				{
					bAlreadyCollectedGarbage = TRUE;
				}
			}
		}
	}

	return TRUE;
}

/**
 * Presents the user with a file dialog for importing.
 * If the import is not a merge (bMerging is FALSE), AskSaveChanges() is called first.
 *
 * @param	bMerge	If TRUE, merge the file into this map.  If FsALSE, merge the map into a blank map.
 */
void FEditorFileUtils::Import(UBOOL bMerge)
{
	// If we are importing into the existing map, we don't need to try to save the existing one, as it's not going away.
	if ( !bMerge )
	{
		// If there are any unsaved changes to the current level, see if the user wants to save those first.
		AskSaveChanges();
	}

	WxFileDialog dlg( GApp->EditorFrame, *LocalizeUnrealEd("Import"), *GetDefaultDirectory(), TEXT(""), *GetFilter(FI_Import), wxOPEN | wxFILE_MUST_EXIST );
	if( dlg.ShowModal() == wxID_OK )
	{
		ImportFile( FString( dlg.GetPath() ), bMerge );
	}
}

/**
 * Saves the specified level.  SaveAs is performed as necessary.
 *
 * @param	Level		The level to be saved.
 * @return				TRUE if the level was saved.
 */
UBOOL FEditorFileUtils::SaveLevel(ULevel* Level)
{
	UBOOL bLevelWasSaved = FALSE;

	// Update cull distance volumes (and associated primitives).
	GWorld->UpdateCullDistanceVolumes();

	// Disallow the save if in interpolation editing mode and the user doesn't want to exit interpolation mode.
	if ( Level && !InInterpEditMode() )
	{
		// Check and see if this is a new map.
		const UBOOL bIsPersistentLevelCurrent = (Level == GWorld->PersistentLevel);

		// If the user trying to save the persistent level?
		if ( bIsPersistentLevelCurrent )
		{
			// Check to see if the persistent level is a new map (ie if it has been saved before).
			const FFilename Filename = GetFilename( GWorld );
			if( !Filename.Len() )
			{
				// Present the user with a SaveAs dialog.
				const UBOOL bAllowStreamingLevelRename = FALSE;
				bLevelWasSaved = SaveAsImplementation( Filename, bAllowStreamingLevelRename );
				return bLevelWasSaved;
			}
		}

		////////////////////////////////
		// At this point, we know the level we're saving has been saved before,
		// so don't bother checking the filename.

		UWorld* WorldToSave = Cast<UWorld>( Level->GetOuter() );
		if ( WorldToSave )
		{
			// Only save the camera views for GWorld.  Otherwise, this would stomp on camera
			// views for levels saved on their own and then included in another world.
			if ( WorldToSave == GWorld )
			{
				SaveViewportViews( WorldToSave );
			}

			FString FinalFilename;
			bLevelWasSaved = SaveWorld( WorldToSave, NULL,
										NULL, NULL,
										TRUE, TRUE, FALSE,
										FinalFilename,
										FALSE, FALSE );

			if ( bLevelWasSaved )
			{
				// Refresh the level browser now that the level's dirty flag has been unset.
				GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );

				// We saved the map, so unless there are any other dirty levels, go ahead and reset the autosave timer
				if( !GUnrealEd->AnyWorldsAreDirty() )
				{
					GUnrealEd->ResetAutosaveTimer();
				}
			}
		}
	}

	return bLevelWasSaved;
}

void FEditorFileUtils::Export(UBOOL bExportSelectedActorsOnly)
{
	// Can't export from cooked packages.
	if ( GWorld->GetOutermost()->PackageFlags & PKG_Cooked )
	{
		return;
	}

	// @todo: extend this to multiple levels.
	const FFilename LevelFilename = GetFilename( GWorld );//->GetOutermost()->GetName() );
	WxFileDialog dlg( GApp->EditorFrame, *LocalizeUnrealEd("Export"), *GetDefaultDirectory(), *LevelFilename.GetBaseFilename(), *GetFilter(FI_Export), wxSAVE | wxOVERWRITE_PROMPT );
	if( dlg.ShowModal() == wxID_OK )
	{
		const FFilename DlgFilename( dlg.GetPath() );
		GUnrealEd->ExportMap( *DlgFilename, bExportSelectedActorsOnly );
		GApp->LastDir[LD_UNR] = DlgFilename.GetPath();
	}
}

/** If TRUE, FWindowsViewport::UpdateModifierState() will enqueue rather than process immediately. */
extern UBOOL GShouldEnqueueModifierStateUpdates;

/**
 * If a PIE world exists, give the user the option to terminate it.
 *
 * @return				TRUE if a PIE session exists and the user refused to end it, FALSE otherwise.
 */
static UBOOL ShouldAbortBecauseOfPIEWorld()
{
	// If a PIE world exists, warn the user that the PIE session will be terminated.
	if ( GEditor->PlayWorld )
	{
		const FString Question = FString::Printf( *LocalizeUnrealEd("Prompt_ThisActionWillTerminatePIEContinue") );
		if( appMsgf( AMT_YesNo, *Question ) )
		{
			// End the play world.
			GEditor->EndPlayMap();
		}
		else
		{
			// User didn't want to end the PIE session -- abort the load.
			return TRUE;
		}
	}
	return FALSE;
}
/**
 * Prompts the user to save the current map if necessary, the presents a load dialog and
 * loads a new map if selected by the user.
 */
void FEditorFileUtils::LoadMap()
{
	// If there are any unsaved changes to the current level, see if the user wants to save those first.
	AskSaveChanges();

	GShouldEnqueueModifierStateUpdates = TRUE;
	WxFileDialog dlg( GApp->EditorFrame, *LocalizeUnrealEd("Open"), *GetDefaultDirectory(), TEXT(""), *GetFilter(FI_Load), wxOPEN | wxFILE_MUST_EXIST );
	if( dlg.ShowModal() == wxID_OK )
	{
		LoadMap( FString( dlg.GetPath() ) );
	}
	GShouldEnqueueModifierStateUpdates = FALSE;
}

/**
 * Loads the specified map.  Does not prompt the user to save the current map.
 *
 * @param	Filename		Map package filename, including path.
 */
void FEditorFileUtils::LoadMap(const FFilename& Filename)
{
	DOUBLE LoadStartTime = appSeconds();

	const FScopedBusyCursor BusyCursor;

	// If a PIE world exists, warn the user that the PIE session will be terminated.
	// Abort if the user refuses to terminate the PIE session.
	if ( ShouldAbortBecauseOfPIEWorld() )
	{
		return;
	}

	// Change out of Matinee when opening new map, so we avoid editing data in the old one.
	if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
	{
		GEditorModeTools().SetCurrentMode( EM_Default );
	}

	// Before opening new file, close any windows onto existing level.
	WxKismet::CloseAllKismetWindows();

	// Close Sentinel if its open
	if(GApp->SentinelTool)
	{
		GApp->SentinelTool->Close(true);
	}

	GUnrealEd->Exec( *FString::Printf( TEXT("MAP LOAD FILE=\"%s\""), *Filename ) );
	ResetLevelFilenames();
	RegisterLevelFilename( GWorld, Filename );

	GApp->LastDir[LD_UNR] = Filename.GetPath();

	GApp->EditorFrame->GetMRUFiles()->AddItem( *Filename );

	// Restore camera views to current viewports
	RestoreViewportViews( GWorld );

	GCallbackEvent->Send(CALLBACK_RefreshEditor_AllBrowsers);

	// Check for deprecated actor classes.
	GEditor->Exec( TEXT("MAP CHECKDEP DONTCLEARMESSAGES") );
	GWarn->MapCheck_ShowConditionally();

	// Track time spent loading map.
	GTaskPerfTracker->AddTask( TEXT("Editor map load"), *Filename.GetBaseFilename(), appSeconds() - LoadStartTime );
}

/**
 * Saves the specified map package, returning TRUE on success.
 *
 * @param	World			The world to save.
 * @param	Filename		Map package filename, including path.
 * @return					TRUE if the map was saved successfully.
 */
UBOOL FEditorFileUtils::SaveMap(UWorld* World, const FFilename& Filename)
{
	UBOOL bLevelWasSaved = FALSE;

	// Disallow the save if in interpolation editing mode and the user doesn't want to exit interpolation mode.
	if ( !InInterpEditMode() )
	{
		DOUBLE SaveStartTime = appSeconds();

		// Update cull distance volumes (and associated primitives).
		GWorld->UpdateCullDistanceVolumes();

		// Save the camera views for this world.
		SaveViewportViews( World );

		FString FinalFilename;
		bLevelWasSaved = SaveWorld( World, &Filename,
									NULL, NULL,
									TRUE, TRUE, FALSE,
									FinalFilename,
									FALSE, FALSE );

		// If the map saved, then put it into the MRU and mark it as not dirty.
		if ( bLevelWasSaved )
		{
			// Set the map filename.
			RegisterLevelFilename( World, Filename );

			World->MarkPackageDirty( FALSE );

			GApp->EditorFrame->GetMRUFiles()->AddItem( Filename );
			GApp->LastDir[LD_UNR] = Filename.GetPath();
			GCallbackEvent->Send( CALLBACK_LevelDirtied );

			// We saved the map, so unless there are any other dirty levels, go ahead and reset the autosave timer
			if( !GUnrealEd->AnyWorldsAreDirty() )
			{
				GUnrealEd->ResetAutosaveTimer();
			}
		}

		GUnrealEd->ResetTransaction( *LocalizeUnrealEd("MapSaved") );

		GFileManager->SetDefaultDirectory();

		// Track time spent saving map.
		GTaskPerfTracker->AddTask( TEXT("Editor map save"), *Filename.GetBaseFilename(), appSeconds() - SaveStartTime );
	}

	return bLevelWasSaved;
}

/**
 * Prompts the user to save the current map if necessary, then creates a new (blank) map.
 */
void FEditorFileUtils::NewMap()
{
	WxDlgNewLevel dlg( GApp->EditorFrame );
	if( dlg.ShowModal() == wxID_OK )
	{
		// If a PIE world exists, warn the user that the PIE session will be terminated.
		// Abort if the user refuses to terminate the PIE session.
		if ( ShouldAbortBecauseOfPIEWorld() )
		{
			return;
		}

		// If there are any unsaved changes to the current level, see if the user wants to save those first.
		AskSaveChanges();

		const FScopedBusyCursor BusyCursor;

		// Change out of Matinee when opening new map, so we avoid editing data in the old one.
		if( GEditorModeTools().GetCurrentModeID() == EM_InterpEdit )
		{
			GEditorModeTools().SetCurrentMode( EM_Default );
		}

		// Before opening new file, close any windows onto existing level.
		WxKismet::CloseAllKismetWindows();

		// Close Sentinel if its open
		if(GApp->SentinelTool)
		{
			GApp->SentinelTool->Close(true);
		}

		const UBOOL bAdditiveGeom = dlg.AdditiveRadio->GetValue();
		GUnrealEd->NewMap( bAdditiveGeom );

		ResetLevelFilenames();
	}
}

/**
 * Saves all levels to the specified directory.
 *
 * @param	AbsoluteAutosaveDir		Autosave directory.
 * @param	AutosaveIndex			Integer prepended to autosave filenames..
 */
UBOOL FEditorFileUtils::AutosaveMap(const FString& AbsoluteAutosaveDir, INT AutosaveIndex)
{
	const FScopedBusyCursor BusyCursor;
	DOUBLE SaveStartTime = appSeconds();

	UBOOL bLevelWasSaved = FALSE;

	// Update cull distance volumes (and associated primitives).
	GWorld->UpdateCullDistanceVolumes();

	// Get the set of all reference worlds.
	TArray<UWorld*> WorldsArray;
	FLevelUtils::GetWorlds( WorldsArray, TRUE );

	if ( WorldsArray.Num() > 0 )
	{
		FString FinalFilename;
		for ( INT WorldIndex = 0 ; WorldIndex < WorldsArray.Num() ; ++WorldIndex )
		{
			UWorld* World = WorldsArray( WorldIndex );
			UPackage* Package = Cast<UPackage>( World->GetOuter() );
			check( Package );

			// If this world needs saving . . .
			if ( Package->IsDirty() )
			{
				// Come up with a meaningful name for the autosave file.
				const FString PackageName = Package->GetName();
				const FString AutosaveFilename( FFilename(PackageName).GetBaseFilename() );

				// Create an autosave filename.
				const FFilename Filename( AbsoluteAutosaveDir * *FString::Printf( TEXT("%s_Auto%i.%s"), *AutosaveFilename, AutosaveIndex, *GApp->MapExt ) );
				//debugf( NAME_Log, TEXT("Autosaving '%s'"), *Filename );
				bLevelWasSaved |= SaveWorld( World, &Filename,
											 NULL, NULL,
											 FALSE, FALSE, TRUE,
											 FinalFilename,
											 TRUE, FALSE );

				// Remark the package as being dirty, as saving will have undiritied the package.
				Package->MarkPackageDirty();
			}
			else
			{
				//debugf( NAME_Log, TEXT("No need to autosave '%s', not dirty"), *Package->GetName() );
			}
		}

		// Track time spent saving map.
		GTaskPerfTracker->AddTask( TEXT("Editor map autosave (incl. sublevels)"), *GWorld->GetOutermost()->GetName(), appSeconds() - SaveStartTime );
	}

	return bLevelWasSaved;
}

/**
 * Save all packages corresponding to the specified UWorlds, with the option to override their path and also
 * apply a prefix.
 *
 * @param	WorldsArray		The set of UWorlds to save.
 * @param	OverridePath	Path override, can be NULL
 * @param	Prefix			Optional prefix for base filename, can be NULL
 * @param	bIncludeGWorld	If TRUE, save GWorld along with other worlds.
 * @param	bCheckDirty		If TRUE, don't save level packages that aren't dirty.
 * @param	bPIESaving		Should be set to TRUE if saving for PIE; passed to UWorld::SaveWorld.
 * @return					TRUE if at least one level was saved.
 *							If bPIESaving, will be TRUE is ALL worlds were saved.
 */
UBOOL appSaveWorlds(const TArray<UWorld*>& WorldsArray, const TCHAR* OverridePath, const TCHAR* Prefix, UBOOL bIncludeGWorld, UBOOL bCheckDirty, UBOOL bPIESaving)
{
	const FScopedBusyCursor BusyCursor;

	// Update cull distance volumes (and associated primitives).
	GWorld->UpdateCullDistanceVolumes();

	// Apply level prefixes.
	FString WorldPackageName = GWorld->GetOutermost()->GetName();
	if ( Prefix )
	{
		WorldPackageName = FString(Prefix) + WorldPackageName;

		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for ( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if ( StreamingLevel )
			{
				// Apply prefix so this level references the soon to be saved other world packages.
				if( StreamingLevel->PackageName != NAME_None )
				{
					StreamingLevel->PackageName = FName( *(FString(Prefix) + StreamingLevel->PackageName.ToString()) );
				}
			}
		}
	}

	// Save all packages containing levels that are currently "referenced" by the global world pointer.
	UBOOL bSavedAll = TRUE;
	UBOOL bAtLeastOneLevelWasSaved = FALSE;
	FString FinalFilename;
	for ( INT WorldIndex = 0 ; WorldIndex < WorldsArray.Num() ; ++WorldIndex )
	{
		UWorld* World = WorldsArray(WorldIndex);
		const UBOOL bLevelWasSaved = SaveWorld( World, NULL,
												OverridePath, Prefix,
												FALSE, TRUE, bCheckDirty,
												FinalFilename,
												FALSE, bPIESaving );
		if ( bLevelWasSaved )
		{
			bAtLeastOneLevelWasSaved = TRUE;
		}
		else
		{
			bSavedAll = FALSE;
		}
	}

	// Remove prefix from referenced level names in AWorldInfo.
	if ( Prefix )
	{
		// Iterate over referenced levels and remove prefix.
		AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
		for ( INT LevelIndex=0; LevelIndex<WorldInfo->StreamingLevels.Num(); LevelIndex++ )
		{
			ULevelStreaming* StreamingLevel = WorldInfo->StreamingLevels(LevelIndex);
			if( StreamingLevel && StreamingLevel->PackageName != NAME_None )
			{
				const FString PrefixedName = StreamingLevel->PackageName.ToString();
				StreamingLevel->PackageName = FName( *PrefixedName.Right( PrefixedName.Len() - appStrlen( Prefix ) ) );
			}
		}
	}

	return (bPIESaving ? bSavedAll : bAtLeastOneLevelWasSaved);
}

/**
 * Save all packages containing UWorld objects with the option to override their path and also
 * apply a prefix.
 *
 * @param	OverridePath	Path override, can be NULL
 * @param	Prefix			Optional prefix for base filename, can be NULL
 * @param	bIncludeGWorld	If TRUE, save GWorld along with other worlds.
 * @param	bCheckDirty		If TRUE, don't save level packages that aren't dirty.
 * @param	bPIESaving		Should be set to TRUE if saving for PIE; passed to UWorld::SaveWorld.
 * @return					TRUE if at least one level was saved.
 */
UBOOL appSaveAllWorlds(const TCHAR* OverridePath, const TCHAR* Prefix, UBOOL bIncludeGWorld, UBOOL bCheckDirty, UBOOL bPIESaving)
{
	// Get the set of all reference worlds.
	TArray<UWorld*> WorldsArray;
	FLevelUtils::GetWorlds( WorldsArray, bIncludeGWorld );

	return appSaveWorlds( WorldsArray, OverridePath, Prefix, bIncludeGWorld, bCheckDirty, bPIESaving );
}

/**
 * Saves all levels associated with GWorld.
 *
 * @param	bCheckDirty		If TRUE, don't save level packages that aren't dirty.
 */
void FEditorFileUtils::SaveAllLevels(UBOOL bCheckDirty)
{
	// Disallow the save if in interpolation editing mode and the user doesn't want to exit interpolation mode.
	if ( !InInterpEditMode() )
	{
		// Special case for the persistent level which may not yet have been saved and thus
		// have no filename associated with it.
		const FFilename Filename = GetFilename( GWorld );
		CheckSaveAs( GWorld, Filename );

		// Check other levels.
		if ( appSaveAllWorlds( NULL, NULL, FALSE, bCheckDirty, FALSE ) )
		{
			// Refresh the level browser now that a levels' dirty flag has been unset.
			GCallbackEvent->Send( CALLBACK_RefreshEditor_LevelBrowser );

			GEditor->ResetAutosaveTimer();
		}
	}
}
