/**
 * UTUI_FrontEnd.cpp: Implementation file for all front end UnrealScript UI classes.
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
#include "UTGame.h"
#include "UTGameUIClasses.h"
#include "UTGameUIFrontEndClasses.h"

#include "FConfigCacheIni.h"
#include "CanvasScene.h"
#include "DownloadableContent.h"

#if PS3
	#include "FFileManagerPS3.h"
#endif

IMPLEMENT_CLASS(UUTUIFrontEnd_CustomScreen)
IMPLEMENT_CLASS(UUTUIFrontEnd_MapSelection)
IMPLEMENT_CLASS(UUTUIScene_DemoSell);

//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd)

/** Start the mod importing process. */
void UUTUIFrontEnd::BeginImportMod()
{
#if PS3
	ImportState=MODIMPORT_Started;
	GFileManagerPS3->BeginImportModFromMedia();
#endif
}

/** Starts the PS3 install process. */
void UUTUIFrontEnd::BeginInstallPS3()
{
#if PS3
	bInstallingPS3 = TRUE;
	GFileManagerPS3->BeginInstallFiles();
#endif
}

/** Cancels the PS3 install. */
void UUTUIFrontEnd::CancelInstallPS3()
{
#if PS3
	GFileManagerPS3->CancelInstallFiles();
#endif
}

void UUTUIFrontEnd::Render_Scene(FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup)
{
	if( UIPostProcessGroup != UIPostProcess_None )
	{
		return;
	}

	Super::Render_Scene(Canvas,UIPostProcessGroup);

	// If there is a version font and text, create a canvas object and draw the version number
	if ( VersionFont )
	{
		if ( VersionText != TEXT("") )
		{
			// Create a temporary canvas if there isn't already one.
			UCanvas* CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
			if( !CanvasObject )
			{
				CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
				CanvasObject->AddToRoot();
			}
			if (CanvasObject)
			{
				CanvasObject->Canvas = Canvas;
				FVector2D ViewportSize;
				GetViewportSize(ViewportSize);
				CanvasObject->SizeX = appTrunc( ViewportSize.X ); 
				CanvasObject->SizeY = appTrunc( ViewportSize.Y ); 

				CanvasObject->SceneView = NULL;
				CanvasObject->Update();

				CanvasObject->OrgX = 0; 
				CanvasObject->OrgY = 0; 
				CanvasObject->Init();

				// Draw the string

				CanvasObject->CurX = ViewportSize.X * VersionPos.X;
				CanvasObject->CurY = ViewportSize.Y * VersionPos.Y;
				CanvasObject->DrawColor = FColor(64,64,64,255);
				INT X=0;
				INT Y=0;
				CanvasObject->WrappedPrint( 1, X,Y, VersionFont, 0.75f, 0.75f, FALSE, *VersionText ); 

				CanvasObject->Update();
			}
		}
	}
}

/**
 * Ticks the scene.
 *
 * @param DeltaTime		Time elapsed since last tick.
 */
void UUTUIFrontEnd::Tick( FLOAT DeltaTime )
{
	Super::Tick(DeltaTime);

#if PS3
	if(ImportState == MODIMPORT_Started || ImportState == MODIMPORT_Unpacking)
	{

		EModImportStage CurrentImportStage = GFileManagerPS3->TickImportMod();
		EModImport CurrentScriptState;

		switch(CurrentImportStage)
		{
		case MIS_Inactive:	
		case MIS_Importing:
		case MIS_ReadyToUnpack:
			CurrentScriptState=MODIMPORT_Started;
			break;
		case MIS_Unpacking:
			CurrentScriptState=MODIMPORT_Unpacking;
			break;
		case MIS_Succeeded:
			if(GDownloadableContent != NULL && GPlatformDownloadableContent != NULL)
			{
				GDownloadableContent->RemoveAllDownloadableContent();
				GPlatformDownloadableContent->FindDownloadableContent();
			}
			CurrentScriptState=MODIMPORT_Finished;
			break;
		case MIS_Failed:
			CurrentScriptState=MODIMPORT_Failed;
			break;
		case MIS_PackageName:
			CurrentScriptState=MODIMPORT_PackageName;
			break;
		default:
			CurrentScriptState=MODIMPORT_Finished;
		}

		eventUpdateModState(CurrentScriptState);
	}

	// Tick the PS3 install process.
	if(bInstallingPS3)
	{
		INT CurrentFile;
		INT TotalFiles;
		UBOOL bHasFinishedCanceling;
		UBOOL bHasError;

		GFileManagerPS3->TickInstallFiles(CurrentFile, TotalFiles, bHasFinishedCanceling, bHasError);
		eventUpdatePS3InstallState(CurrentFile, TotalFiles, bHasFinishedCanceling, bHasError);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd_LoginScreen
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd_LoginScreen)

void UUTUIFrontEnd_LoginScreen::CheckLoginProperties()
{
	FUIProviderFieldValue FieldValue(EC_EventParm);
	FieldValue.PropertyType=DATATYPE_Property;

	if(bSavePassword)
	{
		// Set the stored password
		PasswordEditBox->SetDataStoreBinding(GetPassword());
		if(bAutoLogin)
		{
			if(	!GetDataStoreFieldValue(TEXT("<Registry:AttemptedAutoLogin>"), FieldValue)
			||	FieldValue.StringValue!=TEXT("1") )
			{
				bLoginOnShow = TRUE;
			}
		}
	}

	FieldValue.StringValue=TEXT("1");
	SetDataStoreFieldValue(TEXT("<Registry:AttemptedAutoLogin>"), FieldValue);
}

//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd_WeaponPreference
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd_WeaponPreference)

void UUTUIFrontEnd_WeaponPreference::LoadINIDefaults()
{
	const FString ConfigName = TEXT( "Weapon" );

	if( GUseSeekFreeLoading && CONSOLE )
	{
		const FString IniPrefix = "";
		const FString DefaultIniFilename= appGameConfigDir() + FString(GGameName) + FString::Printf( TEXT( "%s%s.ini"), *ConfigName, TEXT("Defaults") );

		FConfigFile* ExistingConfigFile = GConfig->FindConfigFile(*DefaultIniFilename);
		check(ExistingConfigFile);
		//GConfig->SetFile(*ConfigNameToBeReplaced, ExistingConfigFile);
		GConfig->SetFile( TEXT("PlatformWeapon.ini"), ExistingConfigFile);
		//warnf( TEXT( "UUTUIFrontEnd_WeaponPreference DefaultIniFilename: %s" ), *DefaultIniFilename );
	}
	else
	{
		const FString IniPrefix = PC_DEFAULT_INI_PREFIX;
		const FString DefaultIniFilename = appGameConfigDir() * IniPrefix + *ConfigName;

		// build a new .ini file for the specified platform
		FConfigFile PlatformWeaponIni;
		PlatformWeaponIni.NoSave = TRUE;
		LoadAnIniFile(*DefaultIniFilename, PlatformWeaponIni, FALSE);

		// add it to the config cache so that LoadConfig() can find it
		static_cast<FConfigCacheIni*>(GConfig)->Set(TEXT("PlatformWeapon.ini"), PlatformWeaponIni);

		//warnf( TEXT( "UUTUIFrontEnd_WeaponPreference ConfigNameToBeReplaced: %s" ), *DefaultIniFilename );
	}


	for(INT ClassIdx=0; ClassIdx<WeaponClasses.Num(); ClassIdx++)
	{
		if(WeaponClasses(ClassIdx)!=NULL)
		{
			WeaponClasses(ClassIdx)->GetDefaultObject<AUTWeapon>()->ReloadConfig(NULL, TEXT("PlatformWeapon.ini"));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd_TitleScreen
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd_TitleScreen)

/** Creates a local player for every signed in controller. */
void UUTUIFrontEnd_TitleScreen::UpdateGamePlayersArray()
{
	FString Error;

	// If a player is logged in and doesn't have a LP yet, then create one.
	for(INT ControllerId=0; ControllerId<UCONST_MAX_SUPPORTED_GAMEPADS; ControllerId++)
	{
		const INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);

		if(eventIsLoggedIn(ControllerId))
		{
			if(PlayerIndex==INDEX_NONE)
			{
				debugf(TEXT("UUTUIFrontEnd_TitleScreen::UpdateGamePlayersArray() - Creating LocalPlayer with Controller Id: %i"), ControllerId);
				GEngine->GameViewport->eventCreatePlayer(ControllerId, Error, TRUE);
			}
		}
	}

	// If a player is not logged in, and has a LP, then remove it, but make sure there is always 1 LP in existence.
	for(INT ControllerId=0; ControllerId<UCONST_MAX_SUPPORTED_GAMEPADS && GEngine->GamePlayers.Num() > 1; ControllerId++)
	{
		if(eventIsLoggedIn(ControllerId)==FALSE)
		{
			const INT PlayerIndex = UUIInteraction::GetPlayerIndex(ControllerId);

			if(PlayerIndex!=INDEX_NONE)
			{
				debugf(TEXT("UUTUIFrontEnd_TitleScreen::UpdateGamePlayersArray() - Removing LocalPlayer(Index: %i) with Controller Id: %i"), PlayerIndex, ControllerId);
				GEngine->GameViewport->eventRemovePlayer(GEngine->GamePlayers(PlayerIndex));
			}
		}
	}

	// Update the profile labels with the new login status of the players.
	eventUpdateProfileLabels();
}

/** Tick function for the scene, launches attract mode movie. */
void UUTUIFrontEnd_TitleScreen::Tick(FLOAT DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only try to launch attract movie if we are the top scene in the stack.
	UGameUISceneClient* TheSceneClient = GetSceneClient();

	if(TheSceneClient)
	{
		if ( TheSceneClient->GetActiveScene() == this )
		{
			if(bInMovie==FALSE)
			{
				TimeElapsed += DeltaTime;
				if(TimeElapsed > TimeTillAttractMovie)
				{
					StartMovie();
				}
			}
			else
			{
				UpdateMovieStatus();
			}
		}
	}

	if(bUpdatePlayersOnNextTick)
	{
		UpdateGamePlayersArray();
		bUpdatePlayersOnNextTick=FALSE;
	}
}

/** Starts the attract mode movie. */
void UUTUIFrontEnd_TitleScreen::StartMovie()
{
	debugf(TEXT("UUTUIFrontEnd_TitleScreen::StartMovie() - Starting Attract Mode Movie"));

	if( GFullScreenMovie )
	{
		// Play movie and block on playback.
		GFullScreenMovie->GameThreadPlayMovie(MM_PlayOnceFromStream, TEXT("Attract_Movie"));
	}

	bInMovie = TRUE;
}

/** Stops the currently playing movie. */
void UUTUIFrontEnd_TitleScreen::StopMovie()
{
	debugf(TEXT("UUTUIFrontEnd_TitleScreen::StopMovie() - Stopping Attract Mode Movie"));

	bInMovie = FALSE;
	TimeElapsed = 0.0f;

	if( GFullScreenMovie )
	{
		// Stop Movie
		GFullScreenMovie->GameThreadStopMovie();
	}
}

/** Checks to see if a movie is done playing. */
void UUTUIFrontEnd_TitleScreen::UpdateMovieStatus()
{
	if(GFullScreenMovie && GFullScreenMovie->GameThreadIsMovieFinished(TEXT("Attract_Movie")))
	{
		StopMovie();
	}
}

//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd_Credits
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd_Credits)

namespace
{
	static DOUBLE DoubleStartTime;
}

/** Sets up the scene objects to begin fading/scrolling the credits widgets. */
void UUTUIFrontEnd_Credits::SetupScene()
{
	// Front
	PhotoImage[0] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Front_1"),TRUE));
	PhotoImage[1] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Front_2"),TRUE));
	PhotoImage[2] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Front_3"),TRUE));
	QuoteLabels[0] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Front_1"),TRUE));
	QuoteLabels[1] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Front_2"),TRUE));
	QuoteLabels[2] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Front_3"),TRUE));

	// Back
	PhotoImage[3] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Back_1"),TRUE));
	PhotoImage[4] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Back_2"),TRUE));
	PhotoImage[5] = Cast<UUIImage>(FindChild(TEXT("imgPhoto_Back_3"),TRUE));
	QuoteLabels[3] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Back_1"),TRUE));
	QuoteLabels[4] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Back_2"),TRUE));
	QuoteLabels[5] = Cast<UUILabel>(FindChild(TEXT("lblQuote_Back_3"),TRUE));

	TextLabels[0] = Cast<UUILabel>(FindChild(TEXT("credits_01"),TRUE));
	TextLabels[1] = Cast<UUILabel>(FindChild(TEXT("credits_02"),TRUE));
	TextLabels[2] = Cast<UUILabel>(FindChild(TEXT("credits_03"),TRUE));

	DoubleStartTime = GWorld->GetRealTimeSeconds();

	for(INT WidgetIdx=0; WidgetIdx < 6; WidgetIdx++)
	{
		if(PhotoImage[WidgetIdx])
		{
			PhotoImage[WidgetIdx]->Opacity = 0.0f;
		}
	}
}

/**
 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
 *
 * By default this function recursively calls itself on all of its children.
 */
void UUTUIFrontEnd_Credits::PreInitialSceneUpdate()
{
	Super::PreInitialSceneUpdate();

	if(GIsGame)
	{
		UpdateWidgets(TRUE);
		UpdateCreditsText();
	}
}

/** 
 * Updates the position and text for credits widgets.
 */
void UUTUIFrontEnd_Credits::UpdateCreditsText()
{
	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);

	const FLOAT CurrentTime = (FLOAT)(GWorld->GetRealTimeSeconds() - DoubleStartTime);
	const FLOAT LabelHeight = ViewportSize.Y + TextLabels[0]->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) - TextLabels[0]->GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	const FLOAT Offset = CurrentTime / SceneTimeInSec * LabelHeight;

	if(CurrentTextSet==0)
	{
		// Setup initial labels.
		TextLabels[0]->SetDataStoreBinding(TextSets(0));
		TextLabels[0]->RefreshPosition();

		CurrentTextSet=1;
	}

	((UUIScreenObject*)TextLabels[0])->SetPosition(INT(ViewportSize.Y - Offset), UIFACE_Top, EVALPOS_PixelViewport);
}

/** 
 * Changes the datastore bindings for widgets to the next image set.
 *
 * @param bFront Whether we are updating front or back widgets.
 */
void UUTUIFrontEnd_Credits::UpdateWidgets(UBOOL bFront)
{
	if(ImageSets.IsValidIndex(CurrentImageSet))
	{
		FCreditsImageSet &ImageSet = ImageSets(CurrentImageSet);
		const INT ImgIdx = bFront ? 0 : 1;

		for(INT LabelIdx=0; LabelIdx < 3; LabelIdx++)
		{
			const INT FinalIdx = LabelIdx + ImgIdx*3;

			if(PhotoImage[FinalIdx])
			{
				if ( ImageSet.ImageData.IsValidIndex(LabelIdx) )
				{
					PhotoImage[FinalIdx]->ImageComponent->SetCoordinates( ImageSet.ImageData(LabelIdx).TexCoords );
					PhotoImage[FinalIdx]->SetValue(ImageSet.ImageData(LabelIdx).TexImage);
					PhotoImage[FinalIdx]->eventSetVisibility(TRUE);
				}
				else
				{
					PhotoImage[FinalIdx]->eventSetVisibility(FALSE);
				}

// 				if(CurrentImageSet==ImageSets.Num() - 1)
// 				{
// 					const FLOAT Top = PhotoImage[ImgIdx]->GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
// 					PhotoImage[ImgIdx]->SetPosition(Top+ImageSet.TexCoords.VL, UIFACE_Bottom,EVALPOS_PixelViewport);
// 				}
			}

			if(QuoteLabels[FinalIdx])
			{	
				if(ImageSet.ImageData.IsValidIndex(LabelIdx))
				{
					QuoteLabels[FinalIdx]->SetDataStoreBinding(ImageSet.ImageData(LabelIdx).LabelMarkup);
					QuoteLabels[FinalIdx]->eventSetVisibility(TRUE);
				}
				else
				{
					QuoteLabels[FinalIdx]->eventSetVisibility(FALSE);
				}
			}
		}

		CurrentImageSet++;
	}
}

/**
 * The scene's tick function, updates the different objects in the scene.
 */
void UUTUIFrontEnd_Credits::Tick( FLOAT DeltaTime  )
{
	Super::Tick(DeltaTime);

	if(GIsGame)
	{
		const FLOAT CurrentTime = (FLOAT)(GWorld->GetRealTimeSeconds() - DoubleStartTime);

		if(CurrentTime < SceneTimeInSec)
		{
			const FLOAT SetTime = ((SceneTimeInSec-(DelayBeforePictures+DelayAfterPictures)) / ImageSets.Num()) * 2.0f;	// We multiply by 2 because each period consists of 2 image sets.
			const FLOAT FadeTime = SetTime * 0.1f;

			// We need an extra fade to fade out the last picture set.
			if(CurrentTime > DelayBeforePictures && CurrentTime < (SceneTimeInSec - DelayAfterPictures + FadeTime))
			{
				static UBOOL bUpdateSet = FALSE;

				const FLOAT SolidTime = SetTime * 0.5f - FadeTime;
				const FLOAT LocalTime = appFmod(CurrentTime-DelayBeforePictures, SetTime);
				UBOOL bFrontVisible = FALSE;
				UBOOL bBackVisible = FALSE;
				FLOAT FrontAlpha;

				// The fading of images is done in a piecewise fashion for a period of SetTime->
				// 1) Front widgets fade in / Back fade out
				// 2) Front widgets stay at 1.0f opacity for some time.
				// 3) Front fade out/ Back fade in
				// 4) Back stay at 1.0f opacity.
				if(LocalTime <= FadeTime)
				{
					FrontAlpha = CubicInterp<FLOAT>(0.0f, 0.0f, 1.0f, 0.0f, LocalTime / FadeTime);	

					if(CurrentImageSet < ImageSets.Num())
					{
						bUpdateSet = TRUE;
						bFrontVisible = TRUE;
					}

					bBackVisible = TRUE;
				}
				else if(LocalTime <= (FadeTime + SolidTime))
				{
					FrontAlpha = 1.0f;
					bFrontVisible = TRUE;

					if(bUpdateSet)
					{
						UpdateWidgets(FALSE);
						bUpdateSet = FALSE;
					}
				}
				else if(LocalTime <= (FadeTime*2 + SolidTime))
				{
					FrontAlpha = 1.0f - CubicInterp<FLOAT>(0.0f, 0.0f, 1.0f, 0.0f, (LocalTime - (FadeTime + SolidTime)) / FadeTime);	
					bFrontVisible = TRUE;
					if(CurrentImageSet < ImageSets.Num())
					{
						bUpdateSet = TRUE;
						bBackVisible = TRUE;
					}
				}
				else
				{
					FrontAlpha = 0.0f;
					bBackVisible = TRUE;

					if(bUpdateSet)
					{
						UpdateWidgets(TRUE);
						bUpdateSet = FALSE;
					}
				}

				if(PhotoImage[0] && PhotoImage[1])
				{
					FLOAT BackAlpha = 1.0f - FrontAlpha;

					if(CurrentImageSet==ImageSets.Num()  && (CurrentImageSet%2)==0 && bFrontVisible==FALSE)
					{
						FrontAlpha = 0.0f;
					}
				
					// make sure we arent fading out the back widgets while we fade in for the first time.
					if(CurrentImageSet==1 || (CurrentImageSet==ImageSets.Num()  && (CurrentImageSet%2)==1 && bBackVisible==FALSE))
					{
						BackAlpha = 0.0f;
					}
					
					PhotoImage[0]->Opacity = FrontAlpha;
					PhotoImage[1]->Opacity = FrontAlpha;
					PhotoImage[2]->Opacity = FrontAlpha;
					PhotoImage[3]->Opacity = BackAlpha;
					PhotoImage[4]->Opacity = BackAlpha;
					PhotoImage[5]->Opacity = BackAlpha;

					for(INT LabelIdx=0; LabelIdx<3;LabelIdx++)
					{
						QuoteLabels[LabelIdx]->Opacity = FrontAlpha;
						QuoteLabels[LabelIdx+3]->Opacity = BackAlpha;
					}
				}
			}
			else
			{
				// Hide picture/quote widgets when we are near the start or end of the credits cycle.
				for(INT WidgetIdx=0; WidgetIdx < 6; WidgetIdx++)
				{
					if(PhotoImage[WidgetIdx])
					{
						PhotoImage[WidgetIdx]->Opacity = 0.0f;
					}
				}

				for(INT WidgetIdx =0; WidgetIdx < 6; WidgetIdx++)
				{
					if(QuoteLabels[WidgetIdx])
					{
						QuoteLabels[WidgetIdx]->Opacity = 0.0f;
					}
				}
			}

			// Update scrolling text
			UpdateCreditsText();
		}
		else
		{
			eventOnCreditsFinished();
		}
	}
}




//////////////////////////////////////////////////////////////////////////
// UUTUIStatsList
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIStatsList)

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
void UUTUIStatsList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
}

/** Uses the currently bound list element provider to generate a set of child option widgets. */
void UUTUIStatsList::RegenerateOptions()
{
	GeneratedObjects.Empty();

	// Only generate options ingame.
	if(GIsGame)
	{
		// Generate new options
		if ( DataSource.ResolveMarkup(this) )
		{
			DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());

			if(DataProvider)
			{
				TScriptInterface<IUIListElementCellProvider> SchemaProvider = DataProvider->GetElementCellSchemaProvider(DataSource.DataStoreField);
				TMap<FName,FString> TagMap;
				SchemaProvider->GetElementCellTags(DataSource.DataStoreField,TagMap);
					
				TArray<FName> Tags;
				TArray<FString> Headers;
				TagMap.GenerateKeyArray(Tags);
				TagMap.GenerateValueArray(Headers);

				for(INT TagIdx=0; TagIdx<Tags.Num(); TagIdx++)
				{
					// Create a label for the field.
					UUILabel* KeyObj = Cast<UUILabel>(CreateWidget(this, UUILabel::StaticClass()));
					UUILabel* ValueObj = Cast<UUILabel>(CreateWidget(this, UUILabel::StaticClass()));
					
					if(KeyObj && ValueObj)
					{
						InsertChild(KeyObj);
						InsertChild(ValueObj);

						KeyObj->SetDataStoreBinding(Headers(TagIdx));
						ValueObj->SetDataStoreBinding(TEXT("XXXX"));
						
						// Store referneces to the generated objects.
						FGeneratedStatisticInfo StatInfo;
						StatInfo.DataTag = Tags(TagIdx);
						StatInfo.KeyObj = KeyObj;
						StatInfo.ValueObj = ValueObj;
						GeneratedObjects.AddItem(StatInfo);
					}
				}
			}
		}
	}
}

/**
 * Repositions all option widgets.
 */
void UUTUIStatsList::ResolveFacePosition( EUIWidgetFace Face )
{
	// if we haven't yet resolved any faces, reposition all options now (in case any are docked)
	if ( GetNumResolvedFaces() == 0 )
	{
		RepositionOptions();
	}

	Super::ResolveFacePosition(Face);
}

/** Repositions the previously generated options. */
void UUTUIStatsList::RepositionOptions()
{
	if ( GIsGame )
	{
		const FLOAT OptionOffsetPercentage = 0.6f;
		const FLOAT OptionHeight = 32.0f;
		const FLOAT OptionPadding = 8.0f;
		FLOAT OptionY = 0.0f;

		for(INT OptionIdx = 0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
		{
			UUIObject* KeyObj = GeneratedObjects(OptionIdx).KeyObj;
			UUIObject* ValueObj = GeneratedObjects(OptionIdx).ValueObj;

			// Position Label
			KeyObj->SetPosition(OptionY, UIFACE_Top, EVALPOS_PixelOwner);
			KeyObj->SetPosition(OptionHeight, UIFACE_Bottom, EVALPOS_PixelOwner);

			// Position Widget
			ValueObj->Position.ChangeScaleType(ValueObj, UIFACE_Left, EVALPOS_PercentageOwner);
			ValueObj->Position.ChangeScaleType(ValueObj, UIFACE_Right, EVALPOS_PercentageOwner);
			ValueObj->Position.ChangeScaleType(ValueObj, UIFACE_Top, EVALPOS_PercentageOwner);
			ValueObj->Position.ChangeScaleType(ValueObj, UIFACE_Bottom, EVALPOS_PercentageOwner);

			ValueObj->SetPosition(OptionOffsetPercentage, UIFACE_Left, EVALPOS_PercentageOwner);
			ValueObj->SetPosition(1.0f - OptionOffsetPercentage, UIFACE_Right, EVALPOS_PercentageOwner);
			ValueObj->SetPosition(OptionY, UIFACE_Top, EVALPOS_PixelOwner);
			ValueObj->SetPosition(OptionHeight, UIFACE_Bottom);

			// Increment position
			OptionY += OptionHeight + OptionPadding;
		}
	}
}

/** Sets which result row to get stats values from and then retrieves the stats values. */
void UUTUIStatsList::SetStatsIndex(INT ResultIdx)
{
	if(DataProvider)
	{
		TScriptInterface<IUIListElementCellProvider> ValueProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ResultIdx);

		if(ValueProvider)
		{
			for(INT TagIdx=0; TagIdx<GeneratedObjects.Num(); TagIdx++)
			{
				FUIProviderFieldValue FieldValue(EC_EventParm);
				if(ValueProvider->GetCellFieldValue(DataSource.DataStoreField,GeneratedObjects(TagIdx).DataTag, ResultIdx, FieldValue))
				{
					GeneratedObjects(TagIdx).ValueObj->SetDataStoreBinding(FieldValue.StringValue);
				}
			}
		}
	}
}

/** === UIDataSourceSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UUTUIStatsList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		SetDefaultDataBinding(MarkupText,BindingIndex);
	}
	else if ( DataSource.MarkupString != MarkupText )
	{
		Modify();
        DataSource.MarkupString = MarkupText;

		RefreshSubscriberValue(BindingIndex);

		// Regenerate options.
		RegenerateOptions();
	}
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUTUIStatsList::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return GetDefaultDataBinding(BindingIndex);
	}
	return DataSource.MarkupString;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUIStatsList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		return TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return ResolveDefaultDataBinding(BindingIndex);
	}
	else
	{
		return TRUE;
	}
}

/**
 * Handler for the UIDataStore.OnDataStoreValueUpdated delegate.  Used by data stores to indicate that some data provided by the data
 * has changed.  Subscribers should use this function to refresh any data store values being displayed with the updated value.
 * notify subscribers when they should refresh their values from this data store.
 *
 * @param	SourceDataStore		the data store that generated the refresh notification; useful for subscribers with multiple data store
 *								bindings, to tell which data store sent the notification.
 * @param	PropertyTag			the tag associated with the data field that was updated; Subscribers can use this tag to determine whether
 *								there is any need to refresh their data values.
 * @param	SourceProvider		for data stores which contain nested providers, the provider that contains the data which changed.
 * @param	ArrayIndex			for collection fields, indicates which element was changed.  value of INDEX_NONE indicates not an array
 *								or that the entire array was updated.
 */
void UUTUIStatsList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	//@fixme - for now, just pass through to RefreshSubscriberValue
	RefreshSubscriberValue();
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUTUIStatsList::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);
	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUTUIStatsList::ClearBoundDataStores()
{
	TMultiMap<FName,FUIDataStoreBinding*> DataBindingMap;
	GetDataBindings(DataBindingMap);

	TArray<FUIDataStoreBinding*> DataBindings;
	DataBindingMap.GenerateValueArray(DataBindings);
	for ( INT BindingIndex = 0; BindingIndex < DataBindings.Num(); BindingIndex++ )
	{
		FUIDataStoreBinding* Binding = DataBindings(BindingIndex);
		Binding->ClearDataBinding();
	}

	TArray<UUIDataStore*> DataStores;
	GetBoundDataStores(DataStores);

	for ( INT DataStoreIndex = 0; DataStoreIndex < DataStores.Num(); DataStoreIndex++ )
	{
		UUIDataStore* DataStore = DataStores(DataStoreIndex);
		DataStore->eventSubscriberDetached(this);
	}
}



//////////////////////////////////////////////////////////////////////////
// UUTUIOptionList
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIOptionList)

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
void UUTUIOptionList::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	// Since UITabPage used to be a child of UIScrollFrame, we need to clear the NotifyPositionChanged delegate for
	// any children that still have that delegate pointing to OnChildRepositioned.  OnChildRepositioned is a method in
	// UIScrollFrame that doesn't exist in UIPanel
	TArray<UUIObject*> TabPageChildren;
	GetChildren(TabPageChildren, TRUE);

	for ( INT ChildIndex = 0; ChildIndex < TabPageChildren.Num(); ChildIndex++ )
	{
		UUIObject* Child = TabPageChildren(ChildIndex);
		if ( OBJ_DELEGATE_IS_SET(Child,NotifyPositionChanged) )
		{
			FScriptDelegate& Delegate = Child->__NotifyPositionChanged__Delegate;
			if ( Delegate.FunctionName == TEXT("OnChildRepositioned") && Delegate.Object == this )
			{
				Delegate.FunctionName = NAME_None;
				Delegate.Object = NULL;
			}
		}
	}

	Super::Initialize(inOwnerScene, inOwner);
}

/** Initializes the vertical scrollbar for the optionlist. */
void UUTUIOptionList::InitializeScrollbars()
{
	if(GIsGame && VerticalScrollbar == NULL)
	{
		VerticalScrollbar = Cast<UUIScrollbar>(CreateWidget(this, UUIScrollbar::StaticClass()));
		VerticalScrollbar->ScrollbarOrientation = UIORIENT_Vertical;

		InsertChild( VerticalScrollbar );
	}
}

#define USE_CONSOLE_WIDGETS CONSOLE

/** Uses the currently bound list element provider to generate a set of child option widgets. */
void UUTUIOptionList::RegenerateOptions()
{
	GeneratedObjects.Empty();

	// Only generate options ingame.
	if(GIsGame)
	{
		// Remove all children.
		CurrentIndex=PreviousIndex=0;

		TArray<UUIObject*> ChildrenToRemove = Children;
		ChildrenToRemove.RemoveItem(VerticalScrollbar);
		RemoveChildren(ChildrenToRemove);

		// Generate new options
		if ( DataSource.MarkupString.Len() && DataSource.ResolveMarkup(this) )
		{
			DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());

			if(DataProvider)
			{
				TArray<INT> ListElements;
				DataProvider->GetListElements(DataSource.DataStoreField, ListElements);


					
				for(INT ElementIdx=0; ElementIdx<ListElements.Num(); ElementIdx++)
				{
					TScriptInterface<IUIListElementCellProvider> ElementProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ListElements(ElementIdx));

					if(ElementProvider)
					{
						UUTUIDataProvider_MenuOption* OptionProvider = Cast<UUTUIDataProvider_MenuOption>(ElementProvider->GetUObjectInterfaceUIListElementCellProvider());

						if(OptionProvider)
						{
							FName NewOptionName(NAME_None), NewOptionLabelName(NAME_None);
							{
								// to make it possible for us to search for specific option widgets, use a deterministic name
								// based on their data binding
								INT DelimPos = OptionProvider->DataStoreMarkup.InStr(TEXT("."), TRUE, TRUE);
								if ( DelimPos == INDEX_NONE )
								{
									DelimPos = OptionProvider->DataStoreMarkup.InStr(TEXT(":"), TRUE, TRUE);
								}
								if ( DelimPos != INDEX_NONE )
								{
									// also strip off the trailing >
									FString OptionName = OptionProvider->DataStoreMarkup.Mid(DelimPos+1, OptionProvider->DataStoreMarkup.Len() - DelimPos - 2);
									NewOptionName = *OptionName;
									NewOptionLabelName = *(OptionName+TEXT("Label"));
								}
							}

							// Create a label for the option.
							UUIObject* NewOptionObj = NULL;
							UUIObject* NewOptionLabelObject = CreateWidget(this, UUILabel::StaticClass(), NULL, NewOptionLabelName);
							UUILabel* NewOptionLabel = Cast<UUILabel>(NewOptionLabelObject);
							
							if(NewOptionLabel)
							{
								InsertChild(NewOptionLabel);
								if(OptionProvider->FriendlyName.Len())
								{
									NewOptionLabel->SetDataStoreBinding(OptionProvider->FriendlyName);
								}
								else
								{
									NewOptionLabel->SetDataStoreBinding(OptionProvider->CustomFriendlyName);
								}
								NewOptionLabel->SetEnabled(FALSE);
								NewOptionLabel->StringRenderComponent->StringStyle.DefaultStyleTag = TEXT("OptionListLabel");
							}

							switch((EUTOptionType)OptionProvider->OptionType)
							{
							case UTOT_Spinner:
							#if !USE_CONSOLE_WIDGETS
								{
									UUTUINumericEditBox* NewOption = Cast<UUTUINumericEditBox>(CreateWidget(this, UUTUINumericEditBox::StaticClass(), NULL, NewOptionName));
									if(NewOption)
									{	
										NewOptionObj = NewOption;
										NewOptionObj->TabIndex = ElementIdx;
										InsertChild(NewOption);

										NewOption->NumericValue = OptionProvider->RangeData;
										NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
									}	
								}
								break;
							#endif
							case UTOT_Slider:
								{
									UUTUISlider* NewOption = Cast<UUTUISlider>(CreateWidget(this, UUTUISlider::StaticClass(), NULL, NewOptionName));
					
									if(NewOption)
									{	
										NewOptionObj = NewOption;
										NewOptionObj->TabIndex = ElementIdx;
										InsertChild(NewOption);

										NewOption->SliderValue = OptionProvider->RangeData;
										NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
									}				
								}
								break;
							case UTOT_EditBox:
								{
									UUTUIEditBox* NewOption = Cast<UUTUIEditBox>(CreateWidget(this, UUTUIEditBox::StaticClass(), NULL, NewOptionName));

									if(NewOption)
									{	
										NewOptionObj = NewOption;
										NewOption->MaxCharacters = OptionProvider->EditBoxMaxLength;
										NewOption->TabIndex = ElementIdx;
										NewOption->InitialValue=TEXT("");
										NewOption->CharacterSet = OptionProvider->EditboxAllowedChars;
										InsertChild(NewOption);
										NewOption->SetValue(TEXT(""));
										NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
									}	
								}
								break;
							case UTOT_CheckBox:
							#if !USE_CONSOLE_WIDGETS
								{
									UUICheckbox* NewOption = Cast<UUICheckbox>(CreateWidget(this, UUICheckbox::StaticClass(), NULL, NewOptionName));

									if(NewOption)
									{	
										NewOptionObj = NewOption;
										NewOptionObj->TabIndex = ElementIdx;
										InsertChild(NewOption);
										NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
									}	
								}
								break;
							#endif
							case UTOT_CollectionCheckBox:
							#if !USE_CONSOLE_WIDGETS
								{
									UUTUICollectionCheckBox* NewOption = Cast<UUTUICollectionCheckBox>(CreateWidget(this, UUTUICollectionCheckBox::StaticClass(), NULL, NewOptionName));

									if(NewOption)
									{	
										NewOptionObj = NewOption;
										NewOptionObj->TabIndex = ElementIdx;
										InsertChild(NewOption);
										NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
									}	
								}
								break;
							#endif
							default:
								{
									// If we are on Console, create an option button, otherwise create a combobox.
									 #if USE_CONSOLE_WIDGETS
										UUTUIOptionButton* NewOption = Cast<UUTUIOptionButton>(CreateWidget(this, UUTUIOptionButton::StaticClass(), NULL, NewOptionName));

										if(NewOption)
										{	
											NewOptionObj = NewOption;
											NewOptionObj->TabIndex = ElementIdx;
											InsertChild(NewOption);

											NewOption->SetDataStoreBinding(OptionProvider->DataStoreMarkup);
										}							
									#else
										UUTUIComboBox* NewOption = Cast<UUTUIComboBox>(CreateWidget(this, UUTUIComboBox::StaticClass(), NULL, NewOptionName));

										if(NewOption)
										{	
											NewOptionObj = NewOption;
											NewOptionObj->TabIndex = ElementIdx;
											InsertChild(NewOption);

											NewOption->SetupChildStyles();	// Need to call this to set the default combobox value since we changed the markup for the list.
											NewOption->ComboEditbox->bReadOnly = (OptionProvider->bEditableCombo==FALSE);
											
											if(OptionProvider->bNumericCombo)
											{
												NewOption->ComboEditbox->CharacterSet=CHARSET_NumericOnly;
											}
										}
									#endif
								}
								break;
							}

							// Store a reference to the new object
							if(NewOptionObj)
							{
							    FGeneratedObjectInfo OptionInfo;
								OptionInfo.LabelObj = NewOptionLabelObject;
								OptionInfo.OptionObj = NewOptionObj;
								OptionInfo.OptionProviderName = OptionProvider->GetFName();
								OptionInfo.OptionProvider = OptionProvider;
								GeneratedObjects.AddItem(OptionInfo);

								NewOptionLabelObject->SetEnabled(FALSE);
							}
						}
					}
				}

				// Manually setup focus chain
				for(INT OptionIdx=0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
				{
					if(OptionIdx > 0)
					{
						UUIObject* CurrentWidget = GeneratedObjects(OptionIdx).OptionObj;
						UUIObject* PrevWidget = GeneratedObjects(OptionIdx-1).OptionObj;

						CurrentWidget->SetForcedNavigationTarget(UIFACE_Top, PrevWidget);
						PrevWidget->SetForcedNavigationTarget(UIFACE_Bottom, CurrentWidget);
					}
				}

				// Make focus for the first and last objects wrap around.
				if(GeneratedObjects.Num())
				{
					UUIObject* FirstWidget = GeneratedObjects(0).OptionObj;
					UUIObject* LastWidget = GeneratedObjects(GeneratedObjects.Num()-1).OptionObj;

					if(FirstWidget && LastWidget)
					{
						FirstWidget->SetForcedNavigationTarget(UIFACE_Top, LastWidget);
						LastWidget->SetForcedNavigationTarget(UIFACE_Bottom, FirstWidget);
					}
				}
			}
		}

		// Instance a prefab BG.
		if(BGPrefab)
		{
			if(BGPrefabInstance == NULL)
			{
				BGPrefabInstance = BGPrefab->InstancePrefab(Owner, TEXT("BGPrefab"));
			}

			if(BGPrefabInstance!=NULL)
			{
				// Set some private behavior for the prefab.
				BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_NotEditorSelectable, TRUE);
				BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_TreeHiddenRecursive, TRUE);

				// Add the prefab to the list.
				InsertChild(BGPrefabInstance,0);
			}
		}

		InitializeScrollbars();

		eventSetupOptionBindings();

		RequestSceneUpdate(FALSE,TRUE,FALSE,TRUE);

		if ( GeneratedObjects.Num() && IsFocused(GetBestPlayerIndex()) )
		{
			GeneratedObjects(0).OptionObj->SetFocus(NULL);
		}
	}
}

/**
 * Repositions all option widgets.
 */
void UUTUIOptionList::ResolveFacePosition( EUIWidgetFace Face )
{
	// if we haven't yet resolved any faces, reposition all options now (in case any are docked)
	if ( GetNumResolvedFaces() == 0 && !bAnimatingBGPrefab )
	{
		RepositionOptions();
	}

	Super::ResolveFacePosition(Face);
}

/**
* Routes rendering calls to children of this screen object.
*
* @param	Canvas	the canvas to use for rendering
* @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
*/
void UUTUIOptionList::Render_Children( FCanvas* Canvas, EUIPostProcessGroup UIPostProcessGroup )
{
	UUIScene* OwnerScene = GetScene();

	// store the current global alph modulation
	const FLOAT CurrentAlphaModulation = Canvas->AlphaModulate;

	// if we're focused, we'll need to render any focused children last so that they always render on top.
	// the easiest way to do this is to build a list of Children array indexes that we'll render - indexes for
	// focused children go at the end; indexes for non-focused children go just before the index of the first focused child.
	TArray<UUIObject*> RenderList = Children;
	{
		for ( INT PlayerIndex = 0; PlayerIndex < FocusControls.Num(); PlayerIndex++ )
		{
			UUIObject* FocusedPlayerControl = FocusControls(PlayerIndex).GetFocusedControl();
			if ( FocusedPlayerControl != NULL )
			{
				INT Idx = RenderList.FindItemIndex(FocusedPlayerControl);
				if ( Idx != INDEX_NONE )
				{
					RenderList.Remove(Idx);
					RenderList.AddItem(FocusedPlayerControl);
				}
			}
		}
	}

	for ( INT i = 0; i < RenderList.Num(); i++ )
	{
		UUIObject* Child = RenderList(i);

		// apply the widget's rotation
		if ( Child->IsVisible() && Child->Opacity > KINDA_SMALL_NUMBER )
		{
			// add this widget to the scene's render stack
			OwnerScene->RenderStack.Push(Child);

			// apply the widget's transform matrix
			Canvas->PushRelativeTransform(Child->GenerateTransformMatrix(FALSE));

			// use the widget's ZDepth as the sorting key for the canvas
			Canvas->PushDepthSortKey(appCeil(Child->GetZDepth()));

			// now render the child
			Render_Child(Canvas, Child, UIPostProcessGroup);

			// restore the previous sort key
			Canvas->PopDepthSortKey();

			// restore the previous transform
			Canvas->PopTransform();
		}
	}

	// restore the previous global fade value
	Canvas->AlphaModulate = CurrentAlphaModulation;
}


/** Repositions the previously generated options. */
void UUTUIOptionList::RepositionOptions()
{
	if ( GIsGame )
	{
		if(bRegenOptions)
		{
			UBOOL bIsFocused = IsFocused(GetBestPlayerIndex());
			if(GeneratedObjects.Num()>0 && GeneratedObjects(0).OptionObj && GeneratedObjects(0).OptionObj->IsFocused(GetBestPlayerIndex()))
			{
				GeneratedObjects(0).OptionObj->KillFocus(NULL);
			}

			RegenerateOptions();
			InitializeComboboxWidgets();
			bRegenOptions=FALSE;

			if( GeneratedObjects.Num() && bIsFocused)
			{
				GeneratedObjects(0).OptionObj->SetFocus(NULL);
			}
		}

		FVector2D ViewportSize;
		GetViewportSize(ViewportSize);

		const FLOAT OwnerWidth = GetBounds(UIORIENT_Horizontal, EVALPOS_PixelOwner);
		const FLOAT AnimationTime = 0.1f;
		const FLOAT TextPaddingPercentage = 0.125f;
		const FLOAT ResScale = ViewportSize.Y / 768.0f; // The option height and padding amounts are for 1024x768 so use that as our scaling factor.
		const FLOAT OptionHeight = 32.0f*ResScale;
		const FLOAT OptionPadding = 12.0f*ResScale;
		const FLOAT OptionTopMargin = -3.0f*ResScale;
		const FLOAT OptionRightMargin = 10.0f*ResScale;
		const FLOAT FinalOptionHeight = (OptionPadding+OptionHeight);
		MaxVisibleItems = appFloor(GetBounds(UIORIENT_Vertical, EVALPOS_PixelOwner) / FinalOptionHeight);
		FLOAT CenteringPadding = 0.0f;		//Padding used to center options when there arent enough to scroll.

		if(VerticalScrollbar != NULL)
		{
			VerticalScrollbar->eventSetVisibility((GeneratedObjects.Num() > MaxVisibleItems));
		}

		const FLOAT FinalArrowWidth = (VerticalScrollbar!=NULL) ? 0.0125f : ScrollArrowWidth;
		const FLOAT FinalScrollBarWidth = ((VerticalScrollbar != NULL && VerticalScrollbar->IsVisible()) ? VerticalScrollbar->GetScrollZoneWidth() : 0.0f)+8.0f*ResScale;
		const FLOAT OptionOffsetPercentage = 0.6f-FinalScrollBarWidth/OwnerWidth;

		FLOAT OptionY = 0.0f;

		// Max number of visible options
		INT MiddleItem = MaxVisibleItems / 2;
		INT TopItem = 0;
		INT OldTopItem = 0;

		if(GeneratedObjects.Num() > MaxVisibleItems)
		{
			// Calculate old top item
			if(PreviousIndex > MiddleItem)
			{
				OldTopItem = PreviousIndex - MiddleItem;
			}
			OldTopItem = Clamp<INT>(OldTopItem, 0, GeneratedObjects.Num()-MaxVisibleItems);

			// Calculate current top item
			if(CurrentIndex > MiddleItem)
			{
				TopItem = CurrentIndex - MiddleItem;
			}
			TopItem = Clamp<INT>(TopItem, 0, GeneratedObjects.Num()-MaxVisibleItems);
		}
		else	// Center the list if we dont have a scrollbar.
		{
			FLOAT ListHeight = GetBounds(UIORIENT_Vertical, EVALPOS_PixelOwner);
			CenteringPadding = (ListHeight-GeneratedObjects.Num()*FinalOptionHeight) / 2.0f;
		}

		// Loop through all generated objects and reposition them.
		FLOAT InterpTop = 1.0f;

		if ( bAnimatingBGPrefab )
		{
			FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
			if(TimeElapsed > AnimationTime)
			{
				TimeElapsed = AnimationTime;
			}
			
			InterpTop = TimeElapsed / AnimationTime;

			// Ease In
			InterpTop = 1.0f - InterpTop;
			InterpTop = appPow(InterpTop, 2);
			InterpTop = 1.0f - InterpTop;
		}

		// Animate list movement 
		InterpTop = InterpTop * (TopItem-OldTopItem) + OldTopItem;
		OptionY = -InterpTop * FinalOptionHeight + CenteringPadding;
		const FLOAT FadeDist = 1.0f;

		for(INT OptionIdx = 0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
		{
			UUIObject* NewOptionObj = GeneratedObjects(OptionIdx).OptionObj;
			UUIObject* NewOptionLabelObject = GeneratedObjects(OptionIdx).LabelObj;
			FLOAT WidgetOpacity = 1.0f;
			
			// Calculate opacity for elements off the visible area of the screen.
			FLOAT CurrentRelativeTop = OptionIdx - InterpTop;
			if(CurrentRelativeTop < 0.0f)
			{
				CurrentRelativeTop = Max<FLOAT>(CurrentRelativeTop, -FadeDist);
				WidgetOpacity = 1.0f - CurrentRelativeTop / -FadeDist;
			}
			else if(CurrentRelativeTop - MaxVisibleItems + 1 > 0.0f)
			{
				CurrentRelativeTop = Min<FLOAT>(CurrentRelativeTop - MaxVisibleItems + 1, FadeDist);
				WidgetOpacity = 1.0f - CurrentRelativeTop / FadeDist;
			}

			// Position Label
			NewOptionLabelObject->Opacity = WidgetOpacity;
			NewOptionLabelObject->SetPosition(FinalArrowWidth, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionLabelObject->SetPosition(1.0f, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionLabelObject->SetPosition(OptionY+OptionHeight*TextPaddingPercentage, UIFACE_Top, EVALPOS_PixelOwner);
			NewOptionLabelObject->SetPosition(OptionHeight-OptionHeight*TextPaddingPercentage*2, UIFACE_Bottom, EVALPOS_PixelOwner);

			// Position Widget
			NewOptionObj->Opacity = WidgetOpacity;
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Top, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Bottom, EVALPOS_PercentageOwner);

			FLOAT OptionX = OptionRightMargin;
			if(NewOptionObj->IsA(UUICheckbox::StaticClass()))	// Keep checkbox's square
			{
				NewOptionObj->SetPosition(OwnerWidth-OptionHeight-FinalScrollBarWidth-OptionRightMargin, UIFACE_Left, EVALPOS_PixelOwner);
				NewOptionObj->SetPosition(OptionHeight, UIFACE_Right, EVALPOS_PixelOwner);
			}
			else
			{
				NewOptionObj->SetPosition(OptionOffsetPercentage, UIFACE_Left, EVALPOS_PercentageOwner);
				NewOptionObj->SetPosition(1.0f - OptionOffsetPercentage-(FinalScrollBarWidth+OptionRightMargin)/OwnerWidth, UIFACE_Right, EVALPOS_PercentageOwner);
			}
			NewOptionObj->SetPosition(OptionY+OptionTopMargin, UIFACE_Top, EVALPOS_PixelOwner);
			NewOptionObj->SetPosition(OptionHeight, UIFACE_Bottom);

			// Store bounds
			GeneratedObjects(OptionIdx).OptionX = 0;
			GeneratedObjects(OptionIdx).OptionWidth = OwnerWidth-FinalScrollBarWidth;
			GeneratedObjects(OptionIdx).OptionY = OptionY;
			GeneratedObjects(OptionIdx).OptionHeight = OptionHeight;

			// Increment position
			OptionY += FinalOptionHeight;
		}

		// Position the background prefab.
		if(BGPrefabInstance)
		{
			FLOAT TopFace = GeneratedObjects.IsValidIndex(CurrentIndex) ? GeneratedObjects(CurrentIndex).OptionY : 0.0f;

			if(bAnimatingBGPrefab)
			{
				FLOAT Distance = GeneratedObjects(CurrentIndex).OptionY - 
					GeneratedObjects(PreviousIndex).OptionY;
				FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
				if(TimeElapsed > AnimationTime)
				{
					bAnimatingBGPrefab = FALSE;
					TimeElapsed = AnimationTime;
				}

				// Used to interpolate the scaling to create a smooth looking selection effect.
				FLOAT Alpha = 1.0f - (TimeElapsed / AnimationTime);
			
				// Ease in
				Alpha = appPow(Alpha,2);

				TopFace += -Distance*Alpha;
			}


			// Make the entire prefab percentage owner
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Top, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Bottom, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Right, EVALPOS_PercentageOwner);

			BGPrefabInstance->SetPosition(0.0f, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(1.0f-FinalScrollBarWidth/OwnerWidth, UIFACE_Right, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(TopFace-OptionPadding/2.0f, UIFACE_Top, EVALPOS_PixelOwner);
			BGPrefabInstance->SetPosition(FinalOptionHeight, UIFACE_Bottom);
		}

		// Refresh scrollbars
		if(VerticalScrollbar != NULL && GeneratedObjects.Num()>0)
		{
			// Initialize the Vertical Scrollbar values
			const INT TotalItems = GeneratedObjects.Num();

			// Initialize marker size to be proportional to the visible area of the list
			FLOAT MarkerSize = (FLOAT)1.0f / TotalItems;
			VerticalScrollbar->SetMarkerSize( MarkerSize );

			// Initialize the nudge value to be the size of one item
			VerticalScrollbar->SetNudgeSizePercent( 1.f / TotalItems );

			// Since we do not have a horizontal scrollbar yet, we can disable the corner padding
			VerticalScrollbar->EnableCornerPadding(FALSE);

			// Initialize the marker position only if we are not scrolling.
			UUIInteraction* UIController = GetCurrentUIController();
			if ( UIController != NULL )
			{
				if(UIController->GetOuterUGameViewportClient()->bUIMouseCaptureOverride==FALSE)
				{
					VerticalScrollbar->SetMarkerPosition( ((FLOAT)CurrentIndex) / (TotalItems-1) );
				}
			}
		}
	}
}

void UUTUIOptionList::SetSelectedOptionIndex(INT OptionIdx)
{
	PreviousIndex = CurrentIndex;
	CurrentIndex = OptionIdx;

	// Change widget state
	if(GeneratedObjects.IsValidIndex(PreviousIndex) && GeneratedObjects(PreviousIndex).LabelObj)
	{
		GeneratedObjects(PreviousIndex).LabelObj->SetEnabled(FALSE);
	}

	if(GeneratedObjects.IsValidIndex(CurrentIndex) && GeneratedObjects(CurrentIndex).LabelObj)
	{
		GeneratedObjects(CurrentIndex).LabelObj->SetEnabled(TRUE);
	}

	bAnimatingBGPrefab = TRUE;
	StartMovementTime = GWorld->GetRealTimeSeconds();
}

void UUTUIOptionList::Tick_Widget(FLOAT DeltaTime)
{
	if ( bAnimatingBGPrefab )
	{
		RepositionOptions();
	}
}

// UObject interface
void UUTUIOptionList::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames)
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddItem(UIKEY_Clicked);
}

/**
 * Callback that happens the first time the scene is rendered, any widget positioning initialization should be done here.
 *
 * By default this function recursively calls itself on all of its children.
 */
void UUTUIOptionList::PreInitialSceneUpdate()
{
	Super::PreInitialSceneUpdate();

	InitializeComboboxWidgets();
}

/** Initializes combobox widgets. */
void UUTUIOptionList::InitializeComboboxWidgets()
{
	FVector2D ViewportSize;
	GetViewportSize(ViewportSize);

	FLOAT ResScale = ViewportSize.Y / 768.0f;	// Use 1024x768 as our bases for all pixel values.

	// Setup combobox bindings
	for(INT OptionIdx=0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
	{
		UUTUIComboBox* NewOption = Cast<UUTUIComboBox>(GeneratedObjects(OptionIdx).OptionObj);
		UUTUIDataProvider_MenuOption* OptionData = Cast<UUTUIDataProvider_MenuOption>(GeneratedObjects(OptionIdx).OptionProvider);

		if(NewOption!=NULL && OptionData!=NULL)
		{
			FUIProviderFieldValue CurrentStringValue(EC_EventParm);

			NewOption->ComboList->RowHeight.SetValue(NewOption->ComboList, 28.0f*ResScale, UIEXTENTEVAL_Pixels);
			NewOption->ComboList->SetDataStoreBinding(OptionData->DataStoreMarkup);
		}
	}
}

/** === UIDataSourceSubscriber interface === */
/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UUTUIOptionList::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=-1*/ )
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		SetDefaultDataBinding(MarkupText,BindingIndex);
	}
	else if ( DataSource.MarkupString != MarkupText )
	{
		Modify();
        DataSource.MarkupString = MarkupText;

		RefreshSubscriberValue(BindingIndex);

		// Regenerate options.
		RegenerateOptions();
	}
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUTUIOptionList::GetDataStoreBinding( INT BindingIndex/*=-1*/ ) const
{
	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return GetDefaultDataBinding(BindingIndex);
	}
	return DataSource.MarkupString;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUTUIOptionList::RefreshSubscriberValue( INT BindingIndex/*=INDEX_NONE*/ )
{
	if ( DELEGATE_IS_SET(OnRefreshSubscriberValue) && delegateOnRefreshSubscriberValue(this, BindingIndex) )
	{
		return TRUE;
	}
	else if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		return ResolveDefaultDataBinding(BindingIndex);
	}
	else 
		return TRUE;
}

/**
 * Handler for the UIDataStore.OnDataStoreValueUpdated delegate.  Used by data stores to indicate that some data provided by the data
 * has changed.  Subscribers should use this function to refresh any data store values being displayed with the updated value.
 * notify subscribers when they should refresh their values from this data store.
 *
 * @param	SourceDataStore		the data store that generated the refresh notification; useful for subscribers with multiple data store
 *								bindings, to tell which data store sent the notification.
 * @param	PropertyTag			the tag associated with the data field that was updated; Subscribers can use this tag to determine whether
 *								there is any need to refresh their data values.
 * @param	SourceProvider		for data stores which contain nested providers, the provider that contains the data which changed.
 * @param	ArrayIndex			for collection fields, indicates which element was changed.  value of INDEX_NONE indicates not an array
 *								or that the entire array was updated.
 */
void UUTUIOptionList::NotifyDataStoreValueUpdated( UUIDataStore* SourceDataStore, UBOOL bValuesInvalidated, FName PropertyTag, UUIDataProvider* SourceProvider, INT ArrayIndex )
{
	const UBOOL bBoundToDataStore = (SourceDataStore == DataSource.ResolvedDataStore &&	(PropertyTag == NAME_None || PropertyTag == DataSource.DataStoreField));
	LOG_DATAFIELD_UPDATE(SourceDataStore,bValuesInvalidated,PropertyTag,SourceProvider,ArrayIndex);


// 	TArray<UUIDataStore*> BoundDataStores;
// 	GetBoundDataStores(BoundDataStores);
// 
// 	if (BoundDataStores.ContainsItem(SourceDataStore)
	//@todo ronp - rather than checking SourceDataStore against DataSource, we should call GetBoundDataStores and check whether SourceDataStore is 
	// contained in that array so that cell strings which contain data store markup can be updated from this function....but if the SourceDataStore
	// IS linked through a cell string, the data store will need to pass the correct index
	if ( bBoundToDataStore )
	{
		RefreshSubscriberValue(DataSource.BindingIndex);
	}
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUTUIOptionList::GetBoundDataStores( TArray<UUIDataStore*>& out_BoundDataStores )
{
	GetDefaultDataStores(out_BoundDataStores);
	// add overall data store to the list
	if ( DataSource )
	{
		out_BoundDataStores.AddUniqueItem(*DataSource);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUTUIOptionList::ClearBoundDataStores()
{
	TMultiMap<FName,FUIDataStoreBinding*> DataBindingMap;
	GetDataBindings(DataBindingMap);

	TArray<FUIDataStoreBinding*> DataBindings;
	DataBindingMap.GenerateValueArray(DataBindings);
	for ( INT BindingIndex = 0; BindingIndex < DataBindings.Num(); BindingIndex++ )
	{
		FUIDataStoreBinding* Binding = DataBindings(BindingIndex);
		Binding->ClearDataBinding();
	}

	TArray<UUIDataStore*> DataStores;
	GetBoundDataStores(DataStores);

	for ( INT DataStoreIndex = 0; DataStoreIndex < DataStores.Num(); DataStoreIndex++ )
	{
		UUIDataStore* DataStore = DataStores(DataStoreIndex);
		DataStore->eventSubscriberDetached(this);
	}
}


//////////////////////////////////////////////////////////////////////////
// UUTUIKeyBindingList
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIKeyBindingList)

/** Uses the currently bound list element provider to generate a set of child option widgets. */
void UUTUIKeyBindingList::RegenerateOptions()
{
	GeneratedObjects.Empty();

	// Only generate options ingame.
	if(GIsGame)
	{
		// Generate new options
		if ( DataSource.ResolveMarkup(this) )
		{
			DataProvider = DataSource->ResolveListElementProvider(DataSource.DataStoreField.ToString());

			if(DataProvider)
			{
				TArray<INT> ListElements;
				DataProvider->GetListElements(DataSource.DataStoreField, ListElements);

				LocalizedFriendlyNames.Empty();
				LocalizedFriendlyNames.AddZeroed(ListElements.Num());
				CrucialBindValues.Empty();
				CrucialBindValues.AddZeroed(ListElements.Num());

				for(INT ElementIdx=0; ElementIdx<ListElements.Num(); ElementIdx++)
				{
					TScriptInterface<IUIListElementCellProvider> ElementProvider = DataProvider->GetElementCellValueProvider(DataSource.DataStoreField, ListElements(ElementIdx));

					if(ElementProvider)
					{
						UUTUIDataProvider_KeyBinding* OptionProvider = Cast<UUTUIDataProvider_KeyBinding>(ElementProvider->GetUObjectInterfaceUIListElementCellProvider());

						if(OptionProvider && !OptionProvider->IsFiltered())
						{
							// Create a label for the option.
							UUIObject* NewOptionLabelObject = CreateWidget(this, UUILabel::StaticClass());
							UUILabel* NewOptionLabel = Cast<UUILabel>(NewOptionLabelObject);
							
							if(NewOptionLabel)
							{
								InsertChild(NewOptionLabel);
								NewOptionLabel->SetEnabled(FALSE);
								LocalizedFriendlyNames(ElementIdx) = OptionProvider->FriendlyName;
								CrucialBindValues(ElementIdx) = OptionProvider->bIsCrucialBind;
								NewOptionLabel->SetDataStoreBinding(OptionProvider->FriendlyName);
								NewOptionLabel->StringRenderComponent->StringStyle.DefaultStyleTag = TEXT("OptionListLabel");
							}

							for(INT NewObjIdx=0; NewObjIdx<NumButtons; NewObjIdx++)
							{
								UUIObject* NewOptionObj = NULL;
								UUILabelButton* NewOption = NULL;
								
								NewOption = Cast<UUILabelButton>(CreateWidget(this, UUILabelButton::StaticClass()));

								if(NewOption)
								{
									if (NewOption->BackgroundImageComponent)
									{
										NewOption->BackgroundImageComponent->ImageStyle.DefaultStyleTag = TEXT("UTEditBoxBackground");
									}
									NewOptionObj = NewOption;
									NewOption->TabIndex = GeneratedObjects.Num();

									InsertChild(NewOptionObj);

									// Store a reference to the new object.
									FGeneratedObjectInfo OptionInfo;
									OptionInfo.LabelObj = NewOptionLabelObject;
									OptionInfo.OptionObj = NewOptionObj;
									OptionInfo.OptionProviderName = OptionProvider->GetFName();
									OptionInfo.OptionProvider = OptionProvider;
									GeneratedObjects.AddItem(OptionInfo);
								}
							}
						}
					}
				}
			}

			
			// Instance a prefab BG.
			if(BGPrefab)
			{
				BGPrefabInstance = BGPrefab->InstancePrefab(Owner, TEXT("BGPrefab"));

				if(BGPrefabInstance!=NULL)
				{
					// Set some private behavior for the prefab.
					BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_NotEditorSelectable, TRUE);
					BGPrefabInstance->SetPrivateBehavior(UCONST_PRIVATE_TreeHiddenRecursive, TRUE);

					// Add the prefab to the list.
					InsertChild(BGPrefabInstance,0);
				}
			}

			// Refreshes the binding labels for all of the binding buttons we generated.
			RefreshBindingLabels();


			const INT TotalBindings = GeneratedObjects.Num() / NumButtons;

			// Make options wrap left/right
			for(INT ElementIdx=0; ElementIdx<TotalBindings; ElementIdx++)
			{
				INT ButtonIdx = ElementIdx*NumButtons;

				UUIObject* FirstObject = GeneratedObjects(ButtonIdx).OptionObj;
				UUIObject* LastObject = GeneratedObjects(ButtonIdx+NumButtons-1).OptionObj;

				FirstObject->SetForcedNavigationTarget(UIFACE_Left, LastObject);
				LastObject->SetForcedNavigationTarget(UIFACE_Right, FirstObject);
			}

			// Make options wrap up/down
			INT LastButtonIdx = (TotalBindings-1)*NumButtons;

			for(INT ButtonIdx=0; ButtonIdx<NumButtons; ButtonIdx++)
			{
				UUIObject* FirstObject = GeneratedObjects(ButtonIdx).OptionObj;
				UUIObject* LastObject = GeneratedObjects(LastButtonIdx+ButtonIdx).OptionObj;

				FirstObject->SetForcedNavigationTarget(UIFACE_Top, LastObject);
				LastObject->SetForcedNavigationTarget(UIFACE_Bottom, FirstObject);
			}
		}

		// Initialize the scrollbar
		InitializeScrollbars();

		// Initializes bindings for the options.
		eventSetupOptionBindings();

		RequestSceneUpdate(FALSE,TRUE,FALSE,TRUE);
	}
}


/** Repositions the previously generated options. */
void UUTUIKeyBindingList::RepositionOptions()
{
	if ( GIsGame )
	{
		FVector2D ViewportSize;
		GetViewportSize(ViewportSize);

		const FLOAT AnimationTime = 0.1f;
		const FLOAT TextPaddingPercentage = 0.15f;
		const FLOAT OptionOffsetPercentage = 0.6f;
		const FLOAT OptionButtonPadding = 0.025f;
		const FLOAT OptionWidth = (1.0f - OptionOffsetPercentage) / NumButtons - OptionButtonPadding * (NumButtons-1);
		const FLOAT ResScale = ViewportSize.Y / 768.0f; // The option height and padding amounts are for 1024x768 so use that as our scaling factor.
		const FLOAT OptionHeight = 32.0f*ResScale;
		const FLOAT OptionPadding = 8.0f*ResScale;
		FLOAT OptionY = 0.0f;

		// Max number of visible options
		INT MaxVisible = appFloor(GetPosition(UIFACE_Bottom, EVALPOS_PixelOwner) / (OptionHeight+OptionPadding));
		INT MiddleItem = MaxVisible / 2;
		INT TopItem = 0;
		INT OldTopItem = 0;
		INT RealCurrentIndex = CurrentIndex/NumButtons;
		INT RealPreviousIndex = PreviousIndex/NumButtons;
		INT NumElements = GeneratedObjects.Num() / NumButtons;
		if(NumElements > MaxVisible)
		{
			// Calculate old top item
			if(RealPreviousIndex > MiddleItem)
			{
				OldTopItem = RealPreviousIndex - MiddleItem;
			}
			OldTopItem = Clamp<INT>(OldTopItem, 0, NumElements-MaxVisible);

			// Calculate current top item
			if(RealCurrentIndex > MiddleItem)
			{
				TopItem = RealCurrentIndex - MiddleItem;
			}
			TopItem = Clamp<INT>(TopItem, 0, NumElements-MaxVisible);
		}

		// Loop through all generated objects and reposition them.
		FLOAT InterpTop = 1.0f;

		if(bAnimatingBGPrefab)
		{
			FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
			if(TimeElapsed > AnimationTime)
			{
				TimeElapsed = AnimationTime;
			}

			InterpTop = TimeElapsed / AnimationTime;

			// Ease In
			InterpTop = 1.0f - InterpTop;
			InterpTop = appPow(InterpTop, 2);
			InterpTop = 1.0f - InterpTop;
		}

		// Animate list movement 
		InterpTop = InterpTop * (TopItem-OldTopItem) + OldTopItem;
		OptionY = -InterpTop * (OptionHeight + OptionPadding);

		for(INT OptionIdx = 0; OptionIdx<GeneratedObjects.Num(); OptionIdx++)
		{
			UUIObject* NewOptionObj = GeneratedObjects(OptionIdx).OptionObj;
			UUIObject* NewOptionLabelObject = GeneratedObjects(OptionIdx).LabelObj;
			FLOAT WidgetOpacity = 1.0f;

			// Calculate opacity for elements off the visible area of the screen.
			INT RealIdx = OptionIdx / NumButtons;
			FLOAT CurrentRelativeTop = RealIdx - InterpTop;
			const FLOAT FadeDist = 1.0f;

			if(CurrentRelativeTop < 0.0f)
			{
				CurrentRelativeTop = Max<FLOAT>(CurrentRelativeTop, -FadeDist);
				WidgetOpacity = 1.0f - CurrentRelativeTop / -FadeDist;
			}
			else if(CurrentRelativeTop - MaxVisible + 1 > 0.0f)
			{
				CurrentRelativeTop = Min<FLOAT>(CurrentRelativeTop - MaxVisible + 1, FadeDist);
				WidgetOpacity = 1.0f - CurrentRelativeTop / FadeDist;
			}

			// Position Label
			NewOptionLabelObject->Opacity = WidgetOpacity;
			NewOptionLabelObject->SetPosition(OptionY+OptionHeight*TextPaddingPercentage, UIFACE_Top, EVALPOS_PixelOwner);
			NewOptionLabelObject->SetPosition(OptionHeight-OptionHeight*TextPaddingPercentage*2, UIFACE_Bottom);
			NewOptionLabelObject->SetPosition(ScrollArrowWidth, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionLabelObject->SetPosition(1.0f, UIFACE_Right, EVALPOS_PercentageOwner);

			// Position Widget
			NewOptionObj->Opacity = WidgetOpacity;
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Top, EVALPOS_PercentageOwner);
			NewOptionObj->Position.ChangeScaleType(NewOptionObj, UIFACE_Bottom, EVALPOS_PercentageOwner);

			NewOptionObj->SetPosition(OptionOffsetPercentage + (OptionIdx%NumButtons)*(OptionWidth + OptionButtonPadding), 
				UIFACE_Left, EVALPOS_PercentageOwner);
			NewOptionObj->SetPosition(OptionWidth, UIFACE_Right, EVALPOS_PercentageOwner);
			NewOptionObj->SetPosition(OptionY, UIFACE_Top, EVALPOS_PixelOwner);
			NewOptionObj->SetPosition(OptionHeight, UIFACE_Bottom);

			// Store bounds
			GeneratedObjects(OptionIdx).OptionY = OptionY;
			GeneratedObjects(OptionIdx).OptionHeight = OptionHeight;

			// Increment position
			if(OptionIdx%NumButtons==(NumButtons-1))
			{
				OptionY += OptionHeight + OptionPadding;
			}
		}

		// Position the background prefab.
		if(BGPrefabInstance)
		{
			FLOAT TopFace = GeneratedObjects.IsValidIndex(CurrentIndex) ? GeneratedObjects(CurrentIndex).OptionObj->GetPosition(UIFACE_Top,EVALPOS_PixelOwner) : 0.0f;

			if(bAnimatingBGPrefab)
			{
				FLOAT Distance = GeneratedObjects(CurrentIndex).OptionObj->GetPosition(UIFACE_Top,EVALPOS_PixelOwner) - 
					GeneratedObjects(PreviousIndex).OptionObj->GetPosition(UIFACE_Top,EVALPOS_PixelOwner);
				FLOAT TimeElapsed = GWorld->GetRealTimeSeconds() - StartMovementTime;
				if(TimeElapsed > AnimationTime)
				{
					bAnimatingBGPrefab = FALSE;
					TimeElapsed = AnimationTime;
				}

				// Used to interpolate the scaling to create a smooth looking selection effect.
				FLOAT Alpha = 1.0f - (TimeElapsed / AnimationTime);

				// Ease in
				Alpha = appPow(Alpha,2);

				TopFace += -Distance*Alpha;
			}


			// Make the entire prefab percentage owner
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Top, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Bottom, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->Position.ChangeScaleType(Owner, UIFACE_Right, EVALPOS_PercentageOwner);

			BGPrefabInstance->SetPosition(0.0f, UIFACE_Left, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(1.0f, UIFACE_Right, EVALPOS_PercentageOwner);
			BGPrefabInstance->SetPosition(TopFace-OptionPadding, UIFACE_Top, EVALPOS_PixelOwner);
			BGPrefabInstance->SetPosition(OptionHeight+OptionPadding*3.0f, UIFACE_Bottom);
		}

		// Refresh scrollbars
		if(VerticalScrollbar != NULL && NumElements>0)
		{
			// Initialize the Vertical Scrollbar values
			const INT TotalItems = NumElements;

			// Initialize marker size to be proportional to the visible area of the list
			FLOAT MarkerSize = (FLOAT)1.0f / TotalItems;
			VerticalScrollbar->SetMarkerSize( MarkerSize );

			// Show the vertical scrollbar since the list has elements which aren't visible.
			VerticalScrollbar->eventSetVisibility(TRUE);

			// Initialize the nudge value to be the size of one item
			VerticalScrollbar->SetNudgeSizePercent( 1.f / TotalItems );

			// Since we do not have a horizontal scrollbar yet, we can disable the corner padding
			VerticalScrollbar->EnableCornerPadding(FALSE);

			// Initialize the marker position only if we are not scrolling.
			UUIInteraction* UIController = GetCurrentUIController();
			if ( UIController != NULL )
			{
				if(UIController->GetOuterUGameViewportClient()->bUIMouseCaptureOverride==FALSE)
				{
					VerticalScrollbar->SetMarkerPosition( (FLOAT)(CurrentIndex / NumButtons) / (TotalItems-1) );
				}
			}
		}
	}
}

/** Get the binding key using the command as the key and starting from a specific place in the list. */
FString UUTUIKeyBindingList::GetBindKeyFromCommand(class UPlayerInput* PInput,const FString& Command,INT& StartIdx)
{
	UUTUIScene* UTScene = Cast<UUTUIScene>(GetScene());

#if PS3
	const UBOOL bUsingPS3 = TRUE;
#else
	const UBOOL bUsingPS3 = FALSE;
#endif

	FString BindingKey = TEXT("");

	if (UTScene && PInput)
	{
		do 
		{
			// If our start index is different from our old index then we found a command.
			BindingKey = PInput->GetBindNameFromCommand(Command, &StartIdx );

			// If we are using the PS3, skip controller bindings.
			if (bUsingPS3 && UTScene->eventIsControllerInput(*BindingKey) && StartIdx > -1)
			{
				StartIdx--;
			}
			else
			{
				break;
			}
		}
		while(1);
	}

	return BindingKey;
}

/** Refreshes the binding labels for all of the generated objects. */
void UUTUIKeyBindingList::RefreshBindingLabels()
{
	UPlayerInput* PInput = eventGetPlayerInput();
	TMap<FString, INT> StartingIndices;
	UUTUIScene* UTScene = Cast<UUTUIScene>(GetScene());

	if(UTScene != NULL)
	{
		CurrentBindings.Empty();
		CurrentBindings.AddZeroed(GeneratedObjects.Num());

		if(PInput != NULL)
		{
			// Go through all of the generated objects and update their labels using the currently bound keys in the player input object.
			for(INT ObjectIdx=0; ObjectIdx < GeneratedObjects.Num(); ObjectIdx++)
			{

				UUILabelButton* ButtonObj = Cast<UUILabelButton>(GeneratedObjects(ObjectIdx).OptionObj);
				UUTUIDataProvider_KeyBinding* KeyBindingProvider = Cast<UUTUIDataProvider_KeyBinding>(GeneratedObjects(ObjectIdx).OptionProvider);

				if(KeyBindingProvider && ButtonObj)
				{	
					// See if we already have a starting index for this command.
					const FString CommandStr = KeyBindingProvider->Command;
					INT* SearchIndex = StartingIndices.Find(CommandStr);	
					INT StartIndex = -1;

					// If we found a starting index, then use that.
					if(SearchIndex)
					{
						StartIndex = *SearchIndex;
					}
					
					FString BindingKey = GetBindKeyFromCommand(PInput, CommandStr, StartIndex);

					if(StartIndex > -1)
					{
						CurrentBindings(ObjectIdx) = BindingKey;
						ButtonObj->SetDataStoreBinding(FString::Printf(TEXT("<Strings:UTGameUI.GameMappedStrings.GMS_%s>"), *BindingKey));

						// Store the last starting index so if we have this command again we will search from where we left off.
						StartingIndices.Set(*CommandStr, StartIndex - 1);
					}
					else	// No bound key found, set the datastore binding accordingly
					{
						ButtonObj->SetDataStoreBinding(TEXT("<Strings:UTGameUI.Settings.NotBound>"));
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// UUTUITabPage_MyContent
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUITabPage_MyContent)

/**
 * Actually deletes the content from the storage device.
 *
 * @param ContentName		Filename of content to delete.
 */
void UUTUITabPage_MyContent::PerformDeleteContent(const FString &ContentName)
{
	if(GPlatformDownloadableContent != NULL)
	{
		GPlatformDownloadableContent->DeleteDownloadableContent(*ContentName);
	}
}

/**
 * Reloads all downloaded content.
 */
void UUTUITabPage_MyContent::ReloadContent()
{
	if(GDownloadableContent != NULL && GPlatformDownloadableContent != NULL)
	{
		GDownloadableContent->RemoveAllDownloadableContent();
		GPlatformDownloadableContent->FindDownloadableContent();
	}
}

//////////////////////////////////////////////////////////////////////////
// UUTUITabPage_EpicContent
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUITabPage_EpicContent)


/**
 * Launches a webbrowser to the specify URL
 *
 * @param string	URL		Page to browse to
 */
void UUTUITabPage_EpicContent::LaunchWebBrowser(const FString &URL)
{
	appLaunchURL(*URL);
}


//////////////////////////////////////////////////////////////////////////
// UUTUIFrontEnd_SettingsVideoAdvanced
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UUTUIFrontEnd_SettingsVideoAdvanced)

/** 
 * Sets the value of the video setting. 
 *
 * @param Setting	Setting to set the value of
 * @param Value		New value for the se
 */
void UUTUIFrontEnd_SettingsVideoAdvanced::SetVideoSettingValue(BYTE Setting,INT Value)
{
	FSystemSettingsFriendly FriendlySettings = GSystemSettings.ConvertToFriendlySettings();

	switch(EPossibleVideoSettings(Setting))
	{
	case PVS_ScreenPercentage:
		FriendlySettings.ScreenPercentage=Value;
		break;
	case PVS_TextureDetail:
		FriendlySettings.TextureDetail=EFriendlySettingsLevel(Value);
		break;
	case PVS_WorldDetail:
		FriendlySettings.WorldDetail=EFriendlySettingsLevel(Value);
		break;
	case PVS_VSyncValue:
		FriendlySettings.bUseVSync=(Value!=0);
		break;
	case PVS_SmoothFramerate:
		{
			UGameEngine* GE= Cast<UGameEngine>(GEngine);
			if (GE)
			{
				GE->bSmoothFrameRate = (Value!=0);
				GE->SaveConfig();
			}
		}
		break;

	case PVS_PlayerFOV:
		eventSetPlayerFOV(Value);
		break;

	}

	GSystemSettings.ApplyFriendlySettings(FriendlySettings, TRUE);
}


/** 
 * Sets the value of multiple video settings at once.
 *
 * @param Setting	Array of settings to set the value of
 * @param Value		New values for teh settings
 */
void UUTUIFrontEnd_SettingsVideoAdvanced::SetVideoSettingValueArray(const TArray<BYTE>& Settings,const TArray<INT>& Values)
{
	FSystemSettingsFriendly FriendlySettings = GSystemSettings.ConvertToFriendlySettings();

	check(Settings.Num()==Values.Num());

	for(INT ArrayIdx=0; ArrayIdx<Settings.Num(); ArrayIdx++)
	{
		BYTE Setting = Settings(ArrayIdx);
		INT Value = Values(ArrayIdx);

		switch(EPossibleVideoSettings(Setting))
		{
		case PVS_ScreenPercentage:
			FriendlySettings.ScreenPercentage=Value;
			break;
		case PVS_TextureDetail:
			FriendlySettings.TextureDetail=EFriendlySettingsLevel(Value);
			break;
		case PVS_WorldDetail:
			FriendlySettings.WorldDetail=EFriendlySettingsLevel(Value);
			break;
		case PVS_VSyncValue:
			FriendlySettings.bUseVSync=(Value!=0);
			break;
		case PVS_SmoothFramerate:
			{
				UGameEngine* GE= Cast<UGameEngine>(GEngine);
				if (GE)
				{
					GE->bSmoothFrameRate = (Value!=0);
					GE->SaveConfig();
				}
			}
			break;

		case PVS_PlayerFOV:
			eventSetPlayerFOV(Value);
			break;

		}
	}

	GSystemSettings.ApplyFriendlySettings(FriendlySettings, TRUE);
}


/**
 * @param	Setting		Setting to get the value of
 * @return				Returns the current value of the specified setting.
 */
INT UUTUIFrontEnd_SettingsVideoAdvanced::GetVideoSettingValue(BYTE Setting)
{
	INT Result=0;
	FSystemSettingsFriendly FriendlySettings = GSystemSettings.ConvertToFriendlySettings();

	switch(EPossibleVideoSettings(Setting))
	{
	case PVS_ScreenPercentage:
		Result=FriendlySettings.ScreenPercentage;
		break;
	case PVS_TextureDetail:
		Result=FriendlySettings.TextureDetail;
		break;
	case PVS_WorldDetail:
		Result=FriendlySettings.WorldDetail;
		break;
	case PVS_VSyncValue:
		Result=FriendlySettings.bUseVSync;
		break;
	case PVS_SmoothFramerate:
		{
			UGameEngine* GE= Cast<UGameEngine>(GEngine);
			if (GE)
			{
				Result = GE->bSmoothFrameRate;
			}
			else
			{
				Result = false;
			}
	}
		break;

	case PVS_PlayerFOV:
		{
			AUTPlayerController* PC = GetUTPlayerOwner();
			if (PC)
			{
				Result = appTrunc(PC->OnFootDefaultFOV);
				if (Result < 80)
				{
					Result = 90;
				}
			}
			else
			{
				Result = 90;
			}
		}
		break;
	}
	return Result;
}

/** Reads the encrypted password and decrypts the data */
FString UUTUIFrontEnd::GetPassword(void)
{
	FString ClearTextPassword;
#if _WINDOWS
	FString BlobPassword;
	// Read the stringified buffer from the ini
	if (GConfig->GetString(TEXT("UTUIFrontEnd_LoginScreen"),TEXT("LastPassword"),BlobPassword,GGameIni))
	{
		BYTE EncryptedPassword[1024];
		DWORD ExpectedSize = BlobPassword.Len() / 3;
		// Convert this to a buffer
		if (appStringToBlob(BlobPassword,EncryptedPassword,1024))
		{
			BYTE ClearTextBuffer[1024];
			DWORD Size = 1024;
			// Decrypt the contents
			if (appDecryptBuffer(EncryptedPassword,ExpectedSize,ClearTextBuffer,Size))
			{
				// Zero the buffer and treat as if its unicode
				ClearTextBuffer[Size] = 0;
				ClearTextBuffer[Size + 1] = 0;
				ClearTextPassword = (const TCHAR*)ClearTextBuffer;
			}
		}
	}
#endif
	return ClearTextPassword;
}

/** Encrypts the password and saves the data */
void UUTUIFrontEnd::SavePassword(const FString& Password)
{
#if _WINDOWS
	BYTE ClearTextBuffer[1024];
	DWORD BufferSize = Password.Len() * sizeof(TCHAR);
	// Copy the string data into the buffer
	appMemcpy(ClearTextBuffer,*Password,BufferSize);
	BYTE EncryptedPassword[1024];
	DWORD EncryptedSize = 1024;
	// Decrypt the contents
	if (appEncryptBuffer(ClearTextBuffer,BufferSize,EncryptedPassword,EncryptedSize))
	{
		FString BlobPassword = appBlobToString(EncryptedPassword,EncryptedSize);
		// Read the stringified buffer from the ini
		GConfig->SetString(TEXT("UTUIFrontEnd_LoginScreen"),TEXT("LastPassword"),*BlobPassword,GGameIni);
	}
#endif
}

void UUTUIScene_DemoSell::Sell()
{
	appLaunchURL( TEXT("http://www.unrealtournament3.com/us/order.php"));
}

