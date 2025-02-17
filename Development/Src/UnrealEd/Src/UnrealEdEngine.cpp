/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "FileHelpers.h"
#include "InterpEditor.h"
#include "GenericBrowser.h"
#include "LevelBrowser.h"
#include "Kismet.h"
#include "Menus.h"
#include "Properties.h"
#include "PropertyWindowManager.h"
#include "EngineAnimClasses.h"
#include "EngineAudioDeviceClasses.h"
#include "BusyCursor.h"
#include "DlgGenericComboEntry.h"
#include "Sentinel.h"

/**
 * Initialize the engine.
 */
void UUnrealEdEngine::Init()
{
	Super::Init();

	if( !GIsUCCMake )
	{
		WxGenericBrowser::InitSelection();
	}

	InitBuilderBrush();

	// Iterate over all always fully loaded packages and load them.
	for( INT PackageNameIndex=0; PackageNameIndex<PackagesToBeFullyLoadedAtStartup.Num(); PackageNameIndex++ )
	{
		const FString& PackageName = PackagesToBeFullyLoadedAtStartup(PackageNameIndex);
		FString Filename;
		// Load package if it's found in the package file cache.
		if( GPackageFileCache->FindPackageFile( *PackageName, NULL, Filename ) )
		{
			LoadPackage( NULL, *Filename, LOAD_None );
		}
	}
}

void UUnrealEdEngine::FinishDestroy()
{
	if ( !GIsUCCMake && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		WxGenericBrowser::DestroySelection();
	}

	Super::FinishDestroy();
}

void UUnrealEdEngine::Tick(FLOAT DeltaSeconds)
{
	Super::Tick( DeltaSeconds );

	// Increment the "seconds since last autosave" counter, then try to autosave.
	AutosaveCount += DeltaSeconds;
	AttemptLevelAutosave();
}


/**
 * Resets the autosave timer.
 */
void UUnrealEdEngine::ResetAutosaveTimer()
{
	//debugf( TEXT("Resetting autosave timer") );

	// Reset the "seconds since last autosave" counter.
	AutosaveCount = 0.0f;
}



/**
 * Increases the amount of time before the next autosave
 *
 * @param InSeconds		Number of seconds to add to the autosave countdown timer
 */
void UUnrealEdEngine::BumpAutosaveTimer( const FLOAT InSeconds )
{
	// Adjust the countdown timer
	AutosaveCount -= InSeconds;

	// Don't bump beyond the maximum autosave time
	if( AutosaveCount < 0.0f )
	{
		AutosaveCount = 0.0f;
	}
}


/**
 * Creates an editor derived from the wxInterpEd class.
 *
 * @return		A heap-allocated WxInterpEd instance.
 */
WxInterpEd *UUnrealEdEngine::CreateInterpEditor( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp )
{
	return new WxInterpEd( InParent, InID, InInterp );
}

/**
* Creates an editor derived from the WxCameraAnimEd class.
*
* @return		A heap-allocated WxCameraAnimEd instance.
*/
WxCameraAnimEd *UUnrealEdEngine::CreateCameraAnimEditor( wxWindow* InParent, wxWindowID InID, class USeqAct_Interp* InInterp )
{
	return new WxCameraAnimEd( InParent, InID, InInterp );
}

void UUnrealEdEngine::ShowActorProperties()
{
	// See if we have any unlocked property windows available.  If not, create a new one.

	INT x;

	for( x = 0 ; x < ActorProperties.Num() ; ++x )
	{
		if( !ActorProperties(x)->IsLocked() )
		{
			ActorProperties(x)->Show();
			ActorProperties(x)->Raise();
			break;
		}
	}

	// If no unlocked property windows are available, create a new one

	if( x == ActorProperties.Num() )
	{
		WxPropertyWindowFrame* pw = new WxPropertyWindowFrame;
		pw->Create( GApp->EditorFrame, -1, 1, this );

		//tell this property window to execute ACTOR DESELECT when the escape key is pressed
		pw->GetPropertyWindow()->SetExecDeselectOnEscape(TRUE);

		ActorProperties.AddItem( pw );
		UpdatePropertyWindows();
		pw->Show();
		pw->Raise();
	}
}

void UUnrealEdEngine::ShowWorldProperties()
{
	if( !LevelProperties )
	{
		LevelProperties = new WxPropertyWindowFrame;
		LevelProperties->Create( GApp->EditorFrame, -1, 0, this );
	}
	LevelProperties->SetObject( GWorld->GetWorldInfo(), 0, 1, 1 );
	LevelProperties->Show();
}


/**
 * Pastes clipboard text into a clippad entry
 */
void UUnrealEdEngine::PasteClipboardIntoClipPad()
{
	AWorldInfo* Info = GWorld->GetWorldInfo();
	FString ClipboardText = appClipboardPaste();

	WxDlgGenericComboEntry* dlg = new WxDlgGenericComboEntry( FALSE, FALSE );

	// Gather up the existing clip pad entry names

	TArray<FString> ClipPadNames;

	for( int x = 0 ; x < Info->ClipPadEntries.Num() ; ++x )
	{
		ClipPadNames.AddItem( Info->ClipPadEntries(x)->Title );
	}

	// Ask where the user wants to save this clip

	if( dlg->ShowModal( TEXT("ClipPadSaveAs"), TEXT("SaveAs"), ClipPadNames, 0, TRUE ) == wxID_OK )
	{
		// If the name chosen matches an existing clip, overwrite it.  Otherwise, create a new clip entry.

		int idx = -1;

		for( int x = 0 ; x < Info->ClipPadEntries.Num() ; ++x )
		{
			if( Info->ClipPadEntries(x)->Title == dlg->GetComboBoxString() )
			{
				idx = x;
				break;
			}
		}

		if( idx == -1 )
		{
			Info->ClipPadEntries.AddItem( ConstructObject<UClipPadEntry>( UClipPadEntry::StaticClass(), Info ) );
			idx = Info->ClipPadEntries.Num() - 1;
		}

		Info->ClipPadEntries( idx )->Title = dlg->GetComboBoxString();
		Info->ClipPadEntries( idx )->Text = ClipboardText;
	}
}


/**
* Updates the mouse position status bar field.
*
* @param PositionType	Mouse position type, used to decide how to generate the status bar string.
* @param Position		Position vector, has the values we need to generate the string.  These values are dependent on the position type.
*/
void UUnrealEdEngine::UpdateMousePositionText( EMousePositionType PositionType, const FVector &Position )
{
	FString PositionString;

	switch(PositionType)
	{
	case MP_WorldspacePosition:
		{
			PositionString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("StatusBarMouseWorldspacePosition"), (INT)(Position.X + 0.5f), (INT)(Position.Y + 0.5f), (INT)(Position.Z + 0.5f)));
		}
		break;
	case MP_Translate:
		{
			PositionString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("StatusBarWidgetPosition"), Position.X, Position.Y, Position.Z));
		}	
		break;
	case MP_Rotate:
		{
			FVector NormalizedRotation;

			for ( INT Idx = 0; Idx < 3; Idx++)
			{
				NormalizedRotation[Idx] = 360.f * (Position[Idx] / 65536.f);

				const INT Revolutions = NormalizedRotation[Idx] / 360.f;
				NormalizedRotation[Idx] -= Revolutions * 360;
			}

			//PositionString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("StatusBarWidgetRotation"), NormalizedRotation.X, TEXT('?), NormalizedRotation.Y, TEXT('?), NormalizedRotation.Z, TEXT('?)));
			// 0x00B0 is the Unicode code point for the Degrees symbol.
			PositionString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("StatusBarWidgetRotation"), NormalizedRotation.X, TEXT("\xB0"), NormalizedRotation.Y, TEXT("\xB0"), NormalizedRotation.Z, TEXT("\xB0")));

		}	
		break;
	case MP_Scale:
		{
			FLOAT ScaleFactor;
			FVector FinalScale = Position;
			FinalScale /= FVector(GEditor->Constraints.GetGridSize(),GEditor->Constraints.GetGridSize(),GEditor->Constraints.GetGridSize());

			if(GEditor->Constraints.SnapScaleEnabled)
			{
				ScaleFactor = GEditor->Constraints.ScaleGridSize;
			}
			else
			{
				ScaleFactor = 10.0f;
			}

			FinalScale *= FVector(ScaleFactor, ScaleFactor, ScaleFactor);
			PositionString = FString::Printf(LocalizeSecure(LocalizeUnrealEd("StatusBarWidgetScale"), 
				FinalScale.X, TEXT('%'), FinalScale.Y, TEXT('%'), FinalScale.Z, TEXT('%')));
		}	
		break;
	
	case MP_CameraSpeed:
		{
			PositionString = FString::Printf( LocalizeSecure( LocalizeUnrealEd( "StatusBarCameraSpeed_F" ), Position.X ) );
		}
		break;

	case MP_None:
		{
			PositionString = TEXT("");
		}
		break;
	case MP_NoChange:
		{
			PositionString = LocalizeUnrealEd("StatusBarNoChange");
		}
		break;
	default:
		checkMsg(0, "Unhandled mouse position type.");
                break;
	}

	GApp->EditorFrame->StatusBars[SB_Standard]->SetMouseWorldspacePositionText(*PositionString);
}


/**
* Returns whether or not the map build in progressed was cancelled by the user. 
*/
UBOOL UUnrealEdEngine::GetMapBuildCancelled() const
{
	return GApp->GetMapBuildCancelled();
}

/**
* Sets the flag that states whether or not the map build was cancelled.
*
* @param InCancelled	New state for the cancelled flag.
*/
void UUnrealEdEngine::SetMapBuildCancelled( UBOOL InCancelled )
{
	GApp->SetMapBuildCancelled( InCancelled );
}


/**
*  Cancels color pick mode and resets all property windows to not handle color picking.
*/
void UUnrealEdEngine::CancelColorPickMode()
{
	SetColorPickModeEnabled( FALSE );

	// Loop through all property windows and notify them that color pick mode was cancelled.
	for (INT PropertyWindowIdx = 0; PropertyWindowIdx < GPropertyWindowManager->PropertyWindows.Num(); PropertyWindowIdx++)
	{
		WxPropertyWindow* PropertyWindow = GPropertyWindowManager->PropertyWindows( PropertyWindowIdx );
		PropertyWindow->SetColorPickModeEnabled( FALSE );
	}

	::wxSetCursor( wxCursor( wxCURSOR_ARROW ) );
}

/**
* Picks a color from the current viewport by getting the viewport buffer and then sampling a pixel at the click position.
*
* @param Viewport	Viewport to sample color from.
* @param ClickX	X position of pixel to sample.
* @param ClickY	Y position of pixel to sample.
*
*/
void UUnrealEdEngine::PickColorFromViewport(FViewport* Viewport, INT ClickX, INT ClickY)
{

	// Read pixels from viewport.
	TArray<FColor> OutputBuffer;

	Viewport->ReadPixels(OutputBuffer);

	// Sample the color we want.
	const INT PixelIdx = ClickX + ClickY * (INT)Viewport->GetSizeX();

	if(PixelIdx < OutputBuffer.Num())
	{
		const FColor PixelColor = OutputBuffer(PixelIdx);

		// Loop through all property windows and notify them that we selected a color.
		for (INT PropertyWindowIdx = 0; PropertyWindowIdx < GPropertyWindowManager->PropertyWindows.Num(); PropertyWindowIdx++)
		{
			WxPropertyWindow* PropertyWindow = GPropertyWindowManager->PropertyWindows( PropertyWindowIdx );
			PropertyWindow->OnColorPick( PixelColor );
		}
	}
}

/**
 * @return Returns the global instance of the editor options class.
 */
UUnrealEdOptions* UUnrealEdEngine::GetUnrealEdOptions()
{
	if(EditorOptionsInst == NULL)
	{
		EditorOptionsInst = ConstructObject<UUnrealEdOptions>(UUnrealEdOptions::StaticClass());
	}

	return EditorOptionsInst;
}


/**
* Closes the main editor frame.
*/ 
void UUnrealEdEngine::CloseEditor()
{
	GApp->EditorFrame->Close();
}

void UUnrealEdEngine::ShowUnrealEdContextMenu()
{
	WxMainContextMenu ContextMenu;
	FTrackPopupMenu tpm( GApp->EditorFrame, &ContextMenu );
	tpm.Show();
}

void UUnrealEdEngine::ShowUnrealEdContextSurfaceMenu()
{
	WxMainContextSurfaceMenu SurfaceMenu;
	FTrackPopupMenu tpm( GApp->EditorFrame, &SurfaceMenu );
	tpm.Show();
}

void UUnrealEdEngine::ShowUnrealEdContextCoverSlotMenu(ACoverLink *Link, FCoverSlot &Slot)
{
	WxMainContextCoverSlotMenu CoverSlotMenu(Link,Slot);
	FTrackPopupMenu tpm( GApp->EditorFrame, &CoverSlotMenu );
	tpm.Show();
}

/**
* Redraws all level editing viewport clients.
 *
 * @param	bInvalidateHitProxies		[opt] If TRUE (the default), invalidates cached hit proxies too.
 */
void UUnrealEdEngine::RedrawLevelEditingViewports(UBOOL bInvalidateHitProxies)
{
	if( GApp && GApp->EditorFrame && GApp->EditorFrame->ViewportConfigData )
	{
		for( INT ViewportIndex = 0 ; ViewportIndex < GApp->EditorFrame->ViewportConfigData->GetViewportCount() ; ++ViewportIndex )
		{
			FVCD_Viewport& CurViewport = GApp->EditorFrame->ViewportConfigData->AccessViewport(ViewportIndex);
			if( CurViewport.bEnabled && CurViewport.ViewportWindow != NULL )
			{
				if ( bInvalidateHitProxies )
				{
					// Invalidate hit proxies and display pixels.
					CurViewport.ViewportWindow->Viewport->Invalidate();
				}
				else
				{
					// Invalidate only display pixels.
					CurViewport.ViewportWindow->Viewport->InvalidateDisplay();
				}
			}
		}
	}
}

void UUnrealEdEngine::SetCurrentClass( UClass* InClass )
{
	USelection* SelectionSet = GetSelectedObjects();
	SelectionSet->SelectNone( UClass::StaticClass() );
	SelectionSet->Select( InClass );
}

// Fills a TArray with loaded UPackages

void UUnrealEdEngine::GetPackageList( TArray<UPackage*>* InPackages, UClass* InClass )
{
	InPackages->Empty();

	for( FObjectIterator It ; It ; ++It )
	{
		if( It->GetOuter() && It->GetOuter() != UObject::GetTransientPackage() )
		{
			UObject* TopParent = NULL;

			if( InClass == NULL || It->IsA( InClass ) )
				TopParent = It->GetOutermost();

			if( Cast<UPackage>(TopParent) )
				InPackages->AddUniqueItem( (UPackage*)TopParent );
		}
	}
}

/**
 * Actually save a package.
 *
 * @param Package			The package to save.
 * @param Filename			The name on disk of the package ("....\\MyPackage.upk")
 * @param AssociatedWorld	Non-NULL if the package contains a map.
 *
 * @return					FALSE if the package failed and the user wants to cancel the whole operation and return to the editor
 */
static UBOOL EditorSavePackage(UPackage* Package, const FString& Filename, UWorld* AssociatedWorld)
{
	// The name of the package ("MyPackage").
	FString PackageName = Package->GetName();

	// attempt the save
	UBOOL bWasSuccessful;
	if ( AssociatedWorld )
	{
		// have a Helper attempt to save the map
		bWasSuccessful = FEditorFileUtils::SaveMap( AssociatedWorld, Filename );
	}
	else
	{
		// normally, we just save the package
		bWasSuccessful = GUnrealEd->Exec(*FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=TRUE"), *PackageName, *Filename));
	}

	// Handle all failures the same way.
	if ( !bWasSuccessful )
	{
		// ask the user what to do if we failed
		const INT CancelRetryContinueReply = appMsgf( AMT_CancelRetryContinue, LocalizeSecure(LocalizeUnrealEd("Prompt_26"), *PackageName, *Filename) );
		switch ( CancelRetryContinueReply )
		{
			case 0: // Cancel
				// if this happens, the user wants to stop everything
				return FALSE;
			case 1: // Retry
				// call this function again to retry
				return EditorSavePackage( Package, Filename, AssociatedWorld );
			case 2: // Continue
				return TRUE; // this is if it failed to save, but the user wants to skip saving it, so we "pretend" it was successful
		}
	}
	return TRUE;
}

/**
 * Returns whether saving the specified package is allowed
 */
UBOOL UUnrealEdEngine::CanSavePackage( UPackage* PackageToSave )
{
	WxGenericBrowser* GenericBrowser = GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
	return GenericBrowser == NULL || GenericBrowser->AllowPackageSave(PackageToSave);
}

/**
 * Looks at all currently loaded packages and prompts the user to save them
 * if their "bDirty" flag is set.
 *
 * @param	bShouldSaveMap				TRUE if the function should save the map first before other packages.
 * @param	bForcePackagesToFullyLoad	If TRUE, fully load dirty packages without first prompting the user.
 * @return								TRUE on success, FALSE on fail.
 */
UBOOL UUnrealEdEngine::SaveDirtyPackages(UBOOL bShouldSaveMap, UBOOL bForcePackagesToFullyLoad)
{
	// Packages the user said should not be fully loaded, and thus won't get saved.
	TArray<UPackage*> RootPackagesNotFullyLoaded;
	// The list of dirty root packages that the user specified should be fully loaded.
	TArray<UPackage*> RootPackagesThatWereFullyLoaded;

	// Fully loading a package could mark other packages dirty, so recurse until we have no longer fully loaded a package.
	INT LastNum;
	// Flag storing user's "Yes/No" response for unsaved packages.
	INT YesNoCancelReply = bForcePackagesToFullyLoad ? ART_YesAll : ART_No;
	do
	{
		LastNum = RootPackagesNotFullyLoaded.Num();
		for ( TObjectIterator<UPackage> It ; It ; ++It )
		{
			UPackage*	Package					= *It;
			UBOOL		bShouldIgnorePackage	= FALSE;
			// Only look at root packages.
			bShouldIgnorePackage |= Package->GetOuter() != NULL;
			// Don't try to save "Transient" package.
			bShouldIgnorePackage |= Package == UObject::GetTransientPackage();
			// Ignore packages that haven't been modified.
			bShouldIgnorePackage = bShouldIgnorePackage || !Package->IsDirty();
			// Reject packages that the user has said should be ignored.
			bShouldIgnorePackage |= RootPackagesNotFullyLoaded.ContainsItem( Package );
			if( !bShouldIgnorePackage && !Package->IsFullyLoaded() )
			{
				// Prompt the user if a 'To All' response wasn't already given.
				if( YesNoCancelReply != ART_YesAll && YesNoCancelReply != ART_NoAll )
				{
					YesNoCancelReply = appMsgf( AMT_YesNoYesAllNoAllCancel, LocalizeSecure(LocalizeUnrealEd("SaveDirtyPackagesNotFullyLoadedQ"), *Package->GetName()) );
				}
				switch (YesNoCancelReply)
				{
					case ART_Yes: // Yes
					case ART_YesAll: // Yes to All
					{
						// Fully load the package.
						const FScopedBusyCursor BusyCursor;
						Package->FullyLoad();
						// Mark that the package the user has already approved this package for saving.
						RootPackagesThatWereFullyLoaded.AddItem( Package );
						break;
					}
					case ART_No: // No
					case ART_NoAll: // No to All
						// The user wants to skip this one file, so do nothing.
						RootPackagesNotFullyLoaded.AddItem( Package );
						break;

					case ART_Cancel: // Cancel
						// If the user hit cancel, stop and return to the editor.
						return FALSE;
				};
			}
		}
	} while ( RootPackagesNotFullyLoaded.Num() != LastNum );

	// Make a list of file types.
	FString FileTypes( *LocalizeUnrealEd("AllFiles") );
	for(INT i=0; i<GSys->Extensions.Num(); i++)
	{
		FileTypes += FString::Printf( TEXT("|(*.%s)|*.%s"), *GSys->Extensions(i), *GSys->Extensions(i) );
	}

	// Iterate over all loaded top packages and see whether they have been modified.
	// Only do the first map pass if the flag is passed in
	for (INT SavePass = bShouldSaveMap ? 0 : 1; SavePass < 2; SavePass++)
	{
		// Iterate over all packages, halting if the user doesn't want to save any packages.
		for ( TObjectIterator<UPackage> It ; It && YesNoCancelReply != ART_NoAll ; ++It )
		{
			UPackage*	Package					= *It;
			UBOOL		bShouldIgnorePackage	= 0;
			UWorld*		AssociatedWorld			= static_cast<UWorld*>( StaticFindObject(UWorld::StaticClass(), Package, TEXT("TheWorld")) );
			const UBOOL	bIsMapPackage			= AssociatedWorld != NULL;

			// Ignore packages that haven't been modified.
			bShouldIgnorePackage |= !Package->IsDirty();
			// Reject packages that the user has said should be ignored.
			bShouldIgnorePackage |= RootPackagesNotFullyLoaded.ContainsItem( Package );
			// First pass, save the map, second pass, save everything else
			bShouldIgnorePackage |= bIsMapPackage == SavePass;
				
			if( !bShouldIgnorePackage )
			{
				FString ExistingFile, Filename, Directory;
				UBOOL bCanSaveOnExistingFile = FALSE;

				if( GPackageFileCache->FindPackageFile( *Package->GetName(), NULL, ExistingFile ) )
				{
					FString BaseFilename, Extension;
					GPackageFileCache->SplitPath( *ExistingFile, Directory, BaseFilename, Extension );
					Filename = FString::Printf( TEXT("%s.%s"), *BaseFilename, *Extension );
					bCanSaveOnExistingFile = TRUE;
				}
				else
				{
					Directory = *GFileManager->GetCurrentDirectory();
					if (bIsMapPackage)
					{
						Filename = FString::Printf( TEXT("Untitled.%s"), *GApp->MapExt );
					}
					else
					{
						Filename = FString::Printf( TEXT("%s.upk"), *Package->GetName() );
					}
				}
			

				// we might want to retry if the user hit Cancel on the Save As dialog, we shouldn't assume they want to skip this one file
				UBOOL bShouldRetrySave;
				do
				{
					// by default, we should stop
					bShouldRetrySave = FALSE;

					// Find out what the user wants to do with this dirty package.
					enum { SPS_No=0, SSP_Yes, SSP_Cancel } ShouldSavePackage = SPS_No;
					if( RootPackagesThatWereFullyLoaded.ContainsItem( Package ) )
					{
						// If the user already specified that the package should be
						// fully loaded, just go ahead and save it.
						ShouldSavePackage = SSP_Yes;
					}
					else
					{
						// Prompt the user for what to do if a 'To All' response wasn't already given.
						if( YesNoCancelReply != ART_YesAll && YesNoCancelReply != ART_NoAll )
						{
							YesNoCancelReply = appMsgf( AMT_YesNoYesAllNoAllCancel , LocalizeSecure(LocalizeUnrealEd("Prompt_17"), *Package->GetName(), *Filename, *Filename) );
						}

						if ( YesNoCancelReply == ART_Cancel )
						{
							ShouldSavePackage = SSP_Cancel;
						}
						else if (YesNoCancelReply == ART_Yes || YesNoCancelReply == ART_YesAll )
						{
							ShouldSavePackage = SSP_Yes;
						}
					}

					// act on it
					if ( ShouldSavePackage == SSP_Yes )
					{
						// if we can't save on top of an existing file (because it doesn't exist), then we have to bring up a Save As dialog
						FString SaveFileName;
						if (!bCanSaveOnExistingFile)
						{
							// let the user choose another name to save under
							WxFileDialog SaveFileDialog( NULL, *LocalizeUnrealEd("SavePackage"), *Directory, *Filename,	*FileTypes, wxSAVE,	wxDefaultPosition );

							if( SaveFileDialog.ShowModal() == wxID_OK )
							{
								SaveFileName = FString( SaveFileDialog.GetPath() );
							}
							else
							{
								// if the user hit cancel on the Save dialog, ask again what the user wants to do, 
								// we shouldn't assume they want to skip the file
								bShouldRetrySave = TRUE;

								// Reset the user's 'ToAll' result, because the cancelled the save in the file dialog.
								YesNoCancelReply = ART_No;
								continue;
							}
						}
						else
						{
							// if we are saving over the package, the Filename is the ExistingFile
							SaveFileName = ExistingFile;
						}

						// if the save fails, and the user wants to stop saving and return to the editor, this will return FALSE
						if (!EditorSavePackage(Package, SaveFileName, AssociatedWorld))
						{
							GFileManager->SetDefaultDirectory();
							return FALSE;
						}
					}
					else if ( ShouldSavePackage == SSP_Cancel )
					{
						// If the user hit cancel, then stop and return to the editor.
						GFileManager->SetDefaultDirectory();
						return FALSE;
					}
				} while (bShouldRetrySave);
			}
		}
	}
		
	GFileManager->SetDefaultDirectory();

	return TRUE;
}

/** Returns the thumbnail manager and creates it if missing */
UThumbnailManager* UUnrealEdEngine::GetThumbnailManager()
{
	// Create it if we need to
	if (ThumbnailManager == NULL)
	{
		if (ThumbnailManagerClassName.Len() > 0)
		{
			// Try to load the specified class
			UClass* Class = LoadObject<UClass>(NULL,*ThumbnailManagerClassName,
				NULL,LOAD_None,NULL);
			if (Class != NULL)
			{
				// Create an instance of this class
				ThumbnailManager = ConstructObject<UThumbnailManager>(Class);
			}
		}
		// If the class couldn't be loaded or is the wrong type, fallback to ours
		if (ThumbnailManager == NULL)
		{
			ThumbnailManager = ConstructObject<UThumbnailManager>(UThumbnailManager::StaticClass());
		}
		// Tell it to load all of its classes
		ThumbnailManager->Initialize();
	}
	return ThumbnailManager;
}

/**
 * Returns the browser manager and creates it if missing
 */
UBrowserManager* UUnrealEdEngine::GetBrowserManager(void)
{
	// Create it if we need to
	if (BrowserManager == NULL)
	{
		if (BrowserManagerClassName.Len() > 0)
		{
			// Try to load the specified class
			UClass* Class = LoadObject<UClass>(NULL,*BrowserManagerClassName,
				NULL,LOAD_None,NULL);
			if (Class != NULL)
			{
				// Create an instance of this class
				BrowserManager = ConstructObject<UBrowserManager>(Class);
			}
		}
		// If the class couldn't be loaded or is the wrong type, fallback to ours
		if (BrowserManager == NULL)
		{
			BrowserManager = ConstructObject<UBrowserManager>(UBrowserManager::StaticClass());
		}
	}
	return BrowserManager;
}

/**
 * Serializes this object to an archive.
 *
 * @param	Ar	the archive to serialize to.
 */
void UUnrealEdEngine::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	Ar << ThumbnailManager << BrowserManager;
	Ar << MaterialCopyPasteBuffer;
	Ar << AnimationCompressionAlgorithms;
	Ar << MatineeCopyPasteBuffer;
}

/**
 * If all selected actors belong to the same level, that level is made the current level.
 */
void UUnrealEdEngine::MakeSelectedActorsLevelCurrent()
{
	ULevel* LevelToMakeCurrent = NULL;

	// Look to the selected actors for the level to make current.
	// If actors from multiple levels are selected, do nothing.
	for ( FSelectionIterator It( GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		ULevel* ActorLevel = Actor->GetLevel();

		if ( !LevelToMakeCurrent )
		{
			// First assignment.
			LevelToMakeCurrent = ActorLevel;
		}
		else if ( LevelToMakeCurrent != ActorLevel )
		{
			// Actors from multiple levels are selected -- abort.
			LevelToMakeCurrent = NULL;
			break;
		}
	}

	if ( LevelToMakeCurrent )
	{
		// Update the level browser with the new current level.
		WxLevelBrowser* LevelBrowser = GetBrowser<WxLevelBrowser>( TEXT("LevelBrowser") );
		LevelBrowser->MakeLevelCurrent( LevelToMakeCurrent );

		// If there are any kismet windows open . . .
		if ( GApp->KismetWindows.Num() > 0 )
		{
			// . . . and if the level has a kismet sequence associate with it . . .
			USequence* LevelSequence = GWorld->GetGameSequence( LevelToMakeCurrent );
			if ( LevelSequence )
			{
				// Grab the first one and set the workspace to be that of the level.
				WxKismet* FirstKismetWindow = GApp->KismetWindows(0);
				FirstKismetWindow->ChangeActiveSequence( LevelSequence, TRUE );
			}
		}
	}
}

void UUnrealEdEngine::SentinelStatRender(FEditorLevelViewportClient* ViewportClient, const FSceneView* View, FCanvas* Canvas)
{
	check(GApp);
	if(GApp->SentinelTool)
	{
		GApp->SentinelTool->RenderStats(ViewportClient, View, Canvas);
	}
}

void UUnrealEdEngine::SentinelStatRender3D(const FSceneView* View,class FPrimitiveDrawInterface* PDI)
{
	check(GApp);
	if(GApp->SentinelTool)
	{
		GApp->SentinelTool->RenderStats3D(View, PDI);
	}
}

void UUnrealEdEngine::SentinelMouseMove(FEditorLevelViewportClient* ViewportClient, INT X, INT Y)
{
	check(GApp);
	if(GApp->SentinelTool)
	{
		GApp->SentinelTool->MouseMoved(ViewportClient, X, Y);
	}
}

void UUnrealEdEngine::SentinelInputKey(FEditorLevelViewportClient* ViewportClient, FName Key,EInputEvent Event)
{
	check(GApp);
	if(GApp->SentinelTool)
	{
		GApp->SentinelTool->InputKey(ViewportClient, Key, Event);
	}
}


//////////////////////////////////////////////////////////////////////////
// UUnrealEdOptions
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUnrealEdOptions);
IMPLEMENT_CLASS(UUnrealEdKeyBindings);

/**
 * Generates a mapping from commnads to their parent sets for quick lookup.
 */
void UUnrealEdOptions::GenerateCommandMap()
{
	CommandMap.Empty();
	for(INT CmdIdx=0; CmdIdx<EditorCommands.Num(); CmdIdx++)
	{
		FEditorCommand &Cmd = EditorCommands(CmdIdx);

		CommandMap.Set(Cmd.CommandName, CmdIdx);
	}
}

/**
 * Attempts to execute a command bound to a hotkey.
 *
 * @param Key			Key name
 * @param bAltDown		Whether or not ALT is pressed.
 * @param bCtrlDown		Whether or not CONTROL is pressed.
 * @param bShiftDown	Whether or not SHIFT is pressed.
 * @param EditorSet		Set of bindings to search in.
 */
void UUnrealEdOptions::ExecuteBinding(FName Key, UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName EditorSet)
{
	
	FString Cmd = GetExecCommand(Key, bAltDown, bCtrlDown, bShiftDown, EditorSet);
	if(Cmd.Len())
	{
		GUnrealEd->Exec(*Cmd);
	}
}

/**
 * Attempts to locate a command bound to a hotkey.
 *
 * @param Key			Key name
 * @param bAltDown		Whether or not ALT is pressed.
 * @param bCtrlDown		Whether or not CONTROL is pressed.
 * @param bShiftDown	Whether or not SHIFT is pressed.
 * @param EditorSet		Set of bindings to search in.
 */
FString UUnrealEdOptions::GetExecCommand(FName Key, UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName EditorSet)
{
	TArray<FEditorKeyBinding> &KeyBindings = EditorKeyBindings->KeyBindings;
	FString Result;

	for(INT BindingIdx=0; BindingIdx<KeyBindings.Num(); BindingIdx++)
	{
		FEditorKeyBinding &Binding = KeyBindings(BindingIdx);
		INT* CommandIdx = CommandMap.Find(Binding.CommandName);

		if(CommandIdx && EditorCommands.IsValidIndex(*CommandIdx))
		{
			FEditorCommand &Cmd = EditorCommands(*CommandIdx);

			if(Cmd.Parent == EditorSet)
			{
				// See if this key binding matches the key combination passed in.
				if(bAltDown == Binding.bAltDown && bCtrlDown == Binding.bCtrlDown && bShiftDown == Binding.bShiftDown && Key == Binding.Key)
				{
					INT* EditorCommandIdx = CommandMap.Find(Binding.CommandName);

					if(EditorCommandIdx && EditorCommands.IsValidIndex(*EditorCommandIdx))
					{
						FEditorCommand &EditorCommand = EditorCommands(*EditorCommandIdx);
						Result = EditorCommand.ExecCommand;
					}
					break;
				}
			}
		}
	}

	return Result;
}

/**
 * Attempts to locate a command name bound to a hotkey.
 *
 * @param Key			Key name
 * @param bAltDown		Whether or not ALT is pressed.
 * @param bCtrlDown		Whether or not CONTROL is pressed.
 * @param bShiftDown	Whether or not SHIFT is pressed.
 * @param EditorSet		Set of bindings to search in.
 *
 * @return Name of the command if found, NAME_None otherwise.
 */
FName UUnrealEdOptions::GetCommand(FName Key, UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName EditorSet)
{
	TArray<FEditorKeyBinding> &KeyBindings = EditorKeyBindings->KeyBindings;
	FName Result(NAME_None);

	for(INT BindingIdx=0; BindingIdx<KeyBindings.Num(); BindingIdx++)
	{
		FEditorKeyBinding &Binding = KeyBindings(BindingIdx);
		INT* CommandIdx = CommandMap.Find(Binding.CommandName);

		if(CommandIdx && EditorCommands.IsValidIndex(*CommandIdx))
		{
			FEditorCommand &Cmd = EditorCommands(*CommandIdx);

			if(Cmd.Parent == EditorSet)
			{
				// See if this key binding matches the key combination passed in.
				if(bAltDown == Binding.bAltDown && bCtrlDown == Binding.bCtrlDown && bShiftDown == Binding.bShiftDown && Key == Binding.Key)
				{
					INT* EditorCommandIdx = CommandMap.Find(Binding.CommandName);

					if(EditorCommandIdx && EditorCommands.IsValidIndex(*EditorCommandIdx))
					{
						FEditorCommand &EditorCommand = EditorCommands(*EditorCommandIdx);
						Result = EditorCommand.CommandName;
					}
					break;
				}
			}
		}
	}

	return Result;
}


/**
 * Checks to see if a key is bound yet.
 *
 * @param Key			Key name
 * @param bAltDown		Whether or not ALT is pressed.
 * @param bCtrlDown		Whether or not CONTROL is pressed.
 * @param bShiftDown	Whether or not SHIFT is pressed.
 * @return Returns whether or not the specified key event is already bound to a command or not.
 */
UBOOL UUnrealEdOptions::IsKeyBound(FName Key, UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName EditorSet)
{
	UBOOL bResult = FALSE;

	TArray<FEditorKeyBinding> &KeyBindings = EditorKeyBindings->KeyBindings;
	for(INT BindingIdx=0; BindingIdx<KeyBindings.Num(); BindingIdx++)
	{
		FEditorKeyBinding &Binding = KeyBindings(BindingIdx);
		INT* CommandIdx = CommandMap.Find(Binding.CommandName);

		if(CommandIdx && EditorCommands.IsValidIndex(*CommandIdx))
		{
			FEditorCommand &Cmd = EditorCommands(*CommandIdx);

			if(Cmd.Parent == EditorSet)
			{
				if(bAltDown == Binding.bAltDown && bCtrlDown == Binding.bCtrlDown && bShiftDown == Binding.bShiftDown && Key == Binding.Key)
				{
					bResult = TRUE;
					break;
				}
			}
		}
	}

	return bResult;
}

/**
 * Binds a hotkey.
 *
 * @param Key			Key name
 * @param bAltDown		Whether or not ALT is pressed.
 * @param bCtrlDown		Whether or not CONTROL is pressed.
 * @param bShiftDown	Whether or not SHIFT is pressed.
 * @param Command	Command to bind to.
 */
void UUnrealEdOptions::BindKey(FName Key, UBOOL bAltDown, UBOOL bCtrlDown, UBOOL bShiftDown, FName Command)
{
	UBOOL bFoundKey = FALSE;

	INT* InCmdIdx = CommandMap.Find(Command);

	if(InCmdIdx && EditorCommands.IsValidIndex(*InCmdIdx))
	{
		FEditorCommand &InCmd = EditorCommands(*InCmdIdx);
		FName EditorSet = InCmd.Parent;
		TArray<FEditorKeyBinding> KeysToKeep;

		// Loop through all existing bindings and see if there is a key bound with the same parent, if so, replace it.
		TArray<FEditorKeyBinding> &KeyBindings = EditorKeyBindings->KeyBindings;
		for(INT BindingIdx=0; BindingIdx<KeyBindings.Num(); BindingIdx++)
		{
			UBOOL bKeepKey = TRUE;
			FEditorKeyBinding &Binding = KeyBindings(BindingIdx);
			INT* CommandIdx = CommandMap.Find(Binding.CommandName);

			if(CommandIdx && EditorCommands.IsValidIndex(*CommandIdx))
			{
				FEditorCommand &Cmd = EditorCommands(*CommandIdx);

				if(Cmd.Parent == EditorSet)
				{
					if(bAltDown == Binding.bAltDown && bCtrlDown == Binding.bCtrlDown && bShiftDown == Binding.bShiftDown && Binding.Key == Key)
					{
						bKeepKey = FALSE;
					}
					else if(Cmd.CommandName==Command)
					{
						bKeepKey = FALSE;
					}
				}
			}

			if(bKeepKey==FALSE)
			{
				KeyBindings.Remove(BindingIdx);
				BindingIdx--;
			}
		}

		// Make a new binding
		FEditorKeyBinding NewBinding;
		NewBinding.bAltDown = bAltDown;
		NewBinding.bCtrlDown = bCtrlDown;
		NewBinding.bShiftDown = bShiftDown;
		NewBinding.Key = Key;
		NewBinding.CommandName = Command;
		
		KeyBindings.AddItem(NewBinding);
	}
}


/**
 * Retreives a editor key binding for a specified command.
 *
 * @param Command		Command to retrieve a key binding for.
 *
 * @return A pointer to a keybinding if one exists, NULL otherwise.
 */
FEditorKeyBinding* UUnrealEdOptions::GetKeyBinding(FName Command)
{
	FEditorKeyBinding *Result = NULL;

	TArray<FEditorKeyBinding> &KeyBindings = EditorKeyBindings->KeyBindings;
	for(INT BindingIdx=0; BindingIdx<KeyBindings.Num(); BindingIdx++)
	{
		FEditorKeyBinding &Binding = KeyBindings(BindingIdx);

		if(Binding.CommandName == Command)
		{
			Result = &Binding;
			break;
		}
	}

	return Result;
}



/**
 * Creates an embedded Play In Editor viewport window (if possible)
 *
 * @param ViewportClient The viewport client the new viewport will be associated with
 * @param InPlayInViewportIndex Viewport index to play in, or -1 for "don't care"
 *
 * @return TRUE if successful
 */
UBOOL UUnrealEdEngine::CreateEmbeddedPIEViewport( UGameViewportClient* ViewportClient, INT InPlayInViewportIndex )
{
	// NOTE: Overridden from parent class

	// Select an level editor viewport window to use for Play-In-Editor
	FVCD_Viewport* TargetViewport = NULL;
	{
		// If we were supplied a specific viewport index, try that first
		if( InPlayInViewportIndex != -1 )
		{
			const UINT NumViewports = GApp->EditorFrame->ViewportConfigData->GetViewportCount();
			if( InPlayInViewportIndex >= 0 && ( UINT )InPlayInViewportIndex < NumViewports )
			{
				TargetViewport = &GApp->EditorFrame->ViewportConfigData->AccessViewport( InPlayInViewportIndex );
			}
		}

		// If there was no specific viewport index and if we have an active level editor viewport, prefer that
		if( TargetViewport == NULL && GCurrentLevelEditingViewportClient != NULL )
		{
			const UINT NumViewports = GApp->EditorFrame->ViewportConfigData->GetViewportCount();
			for( UINT CurViewportIndex = 0; CurViewportIndex < NumViewports; ++CurViewportIndex )
			{
				FVCD_Viewport* CurrentViewport = &GApp->EditorFrame->ViewportConfigData->AccessViewport( CurViewportIndex );
				if( CurrentViewport->ViewportWindow == GCurrentLevelEditingViewportClient )
				{
					// Use the currently active level editor viewport
					TargetViewport = CurrentViewport;
					break;
				}
			}
		}

		// Still haven't found anything?  We'll look for any perspective viewports
		if( TargetViewport == NULL )
		{
			// Use a perspective viewport if we still haven't found anything
			if( GCurrentLevelEditingViewportClient != NULL )
			{
				const UINT NumViewports = GApp->EditorFrame->ViewportConfigData->GetViewportCount();
				for( UINT CurViewportIndex = 0; CurViewportIndex < NumViewports; ++CurViewportIndex )
				{
					FVCD_Viewport* CurrentViewport = &GApp->EditorFrame->ViewportConfigData->AccessViewport( CurViewportIndex );
					if( CurrentViewport->bEnabled && CurrentViewport->ViewportType == LVT_Perspective )
					{
						// Use a perspective viewport
						TargetViewport = CurrentViewport;
						break;
					}
				}
			}
		}
	}
		
	if( TargetViewport != NULL )
	{
		WxPIEContainerWindow* PIEContainerWindow =
			WxPIEContainerWindow::CreatePIEWindowAndPossessViewport( ViewportClient, TargetViewport );
		if( PIEContainerWindow != NULL )
		{
			// Success!
			return TRUE;
		}
	}

	return FALSE;
}

