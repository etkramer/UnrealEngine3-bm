/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "DlgActorFactory.h"
#include "DlgBuildProgress.h"
#include "DlgGeometryTools.h"
#include "DlgMapCheck.h"
#include "DlgTipOfTheDay.h"
#include "DlgTransform.h"
#include "DebugToolExec.h"
#include "UnTexAlignTools.h"
#include "SourceControlIntegration.h"
#include "..\..\Launch\Resources\resource.h"
#include "Properties.h"
#include "PropertyWindowManager.h"
#include "FileHelpers.h"
#include "DlgActorSearch.h"
#include "TerrainEditor.h"
#include "ScopedTransaction.h"
#include "EnginePhysicsClasses.h"
#include "BSPOps.h"
#include "BusyCursor.h"

WxUnrealEdApp*		GApp;
UUnrealEdEngine*	GUnrealEd;
WxDlgAddSpecial*	GDlgAddSpecial;

/*-----------------------------------------------------------------------------
	FAutoConvexCollision
-----------------------------------------------------------------------------*/

void FAutoConvexCollision::DoDecomp(INT Depth, INT MaxVerts, FLOAT CollapseThresh)
{
	const FScopedTransaction Transaction( TEXT("Create Blocking Volume") );
	const FScopedBusyCursor BusyCursor;
	GWorld->GetBrush()->Modify();

	// If we have previously created a blocking volume, get rid of it.

	if( LastCreatedVolume != NULL )
	{
		GWorld->DestroyActor( LastCreatedVolume );
	}

	// Gather a list of selected polygons.

	TArray<FPoly*> SelectedPolys = GetSelectedPolygons();

	// Generate a bounding box for the selected polygons.  Will be used to compute a scaling factor below.

	FBox bbox(0);

	for( INT p = 0 ; p < SelectedPolys.Num() ; ++p )
	{
		FPoly* poly = SelectedPolys(p);

		for( INT v = 0 ; v < poly->Vertices.Num() ; ++v )
		{
			bbox += poly->Vertices(v);
		}
	}

	// Compute the scaling factor to get these polygons within a 256x256x256 cube.  The tools seem to work best if meshes aren't scaled to large values.

	FVector extent = bbox.GetExtent();
	FLOAT MaxAxis = Max( extent.Z, Max( extent.X, extent.Y ) );

	FLOAT ScaleFactor = MaxAxis / 128.0f;

	// Create vertex/index buffers from the selected polygon list

	INT NumVerts = SelectedPolys.Num(), idx = 0;
	TArray<FVector> Verts;
	TArray<INT> Indices;
	for( INT p = 0 ; p < SelectedPolys.Num() ; ++p )
	{
		FPoly* poly = SelectedPolys(p);

		for( INT v = 0 ; v < poly->Vertices.Num() ; ++v )
		{
			Verts.AddItem( poly->Vertices(v) / ScaleFactor );
			Indices.AddItem( idx );
			idx++;
		}
	}

	// Create a bodysetup to store the soon to be created collision primitives.  This will be thrown away at the end.

	URB_BodySetup* bs = ConstructObject<URB_BodySetup>(URB_BodySetup::StaticClass(), GWorld->CurrentLevel);

	// Run actual util to do the work
	DecomposeMeshToHulls( &(bs->AggGeom), Verts, Indices, Depth, 0.1f, CollapseThresh, MaxVerts );

	// Generate an offset vector.  This will be used to ensure that the newly created blocking volumes location matches the location
	// of the last selected static mesh.

	AActor* Actor = GEditor->GetSelectedActors()->GetTop<AActor>();
	FVector Offset(0,0,0);
	if( Actor )
	{
		Offset = Actor->Location;
	}

	// Create a new builder brush from the hull triangles.

	GEditor->ResetPivot();

	GWorld->GetBrush()->Location = FVector(0,0,0);
	GWorld->GetBrush()->Brush->Polys->Element.Empty();

	for( int ce = 0 ; ce < bs->AggGeom.ConvexElems.Num() ; ++ce )
	{
		FKConvexElem* convex = &bs->AggGeom.ConvexElems(ce);

		for( INT f = 0 ; f < convex->FaceTriData.Num() ; f += 3 )
		{
			FVector v0 = (convex->VertexData( convex->FaceTriData(f+0) ) * ScaleFactor ) - Offset;
			FVector v1 = (convex->VertexData( convex->FaceTriData(f+1) ) * ScaleFactor ) - Offset;
			FVector v2 = (convex->VertexData( convex->FaceTriData(f+2) ) * ScaleFactor ) - Offset;

			FPoly* Polygon = new FPoly();

			Polygon->Init();
			Polygon->PolyFlags = PF_DefaultFlags;

			Polygon->Vertices.AddItem( v0 );
			Polygon->Vertices.AddItem( v2 );
			Polygon->Vertices.AddItem( v1 );

			Polygon->CalcNormal(1);

			GWorld->GetBrush()->Brush->Polys->Element.AddItem( *Polygon );
		}
	}

	// Clean up the builder brush

	GWorld->GetBrush()->Location = Offset;

	GWorld->GetBrush()->ClearComponents();
	GWorld->GetBrush()->ConditionalUpdateComponents();

	// Create the blocking volume

	LastCreatedVolume = (ABlockingVolume*)GWorld->SpawnActor(ABlockingVolume::StaticClass(),NAME_None,GWorld->GetBrush()->Location);
	if( LastCreatedVolume )
	{
		LastCreatedVolume->PreEditChange(NULL);

		FBSPOps::csgCopyBrush
			(
			LastCreatedVolume,
			GWorld->GetBrush(),
			0,
			RF_Transactional,
			1,
			TRUE
			);

		// Set the texture on all polys to NULL.  This stops invisible texture
		// dependencies from being formed on volumes.

		if( LastCreatedVolume->Brush )
		{
			for( INT poly = 0 ; poly < LastCreatedVolume->Brush->Polys->Element.Num() ; ++poly )
			{
				FPoly* Poly = &(LastCreatedVolume->Brush->Polys->Element(poly));
				Poly->Material = NULL;
			}
		}

		LastCreatedVolume->PostEditChange(NULL);
	}

	// Set up collision on selected actors

	SetCollisionTypeOnSelectedActors( COLLIDE_BlockWeapons );

	// Clean up

	for( INT x = 0 ; x < SelectedPolys.Num() ; ++x )
	{
		delete SelectedPolys(x);
	}

	SelectedPolys.Empty();

	// Update screen.

	GEditor->RedrawLevelEditingViewports();
}

void FAutoConvexCollision::DecompOptionsClosed()
{
	DecompNewVolume();
}

void FAutoConvexCollision::DecompNewVolume()
{
	LastCreatedVolume = NULL;
}

/**
* Gathers up a list of selection FPolys from selected static meshes.
*
* @return	A TArray containing FPolys representing the triangles in the selected static meshes (note that these
*           triangles are transformed into world space before being added to the array.
*/

TArray<FPoly*> FAutoConvexCollision::GetSelectedPolygons()
{
	// Build a list of polygons from all selected static meshes

	TArray<FPoly*> SelectedPolys;

	for( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );

		for(INT j=0; j<Actor->AllComponents.Num(); j++)
		{
			// If its a static mesh component, with a static mesh
			UActorComponent* Comp = Actor->AllComponents(j);
			UStaticMeshComponent* SMComp = Cast<UStaticMeshComponent>(Comp);
			if(SMComp && SMComp->StaticMesh)
			{
				UStaticMesh* StaticMesh = SMComp->StaticMesh;
				const FStaticMeshTriangle* RawTriangleData = (FStaticMeshTriangle*) StaticMesh->LODModels(0).RawTriangles.Lock(LOCK_READ_ONLY);

				if( SMComp->StaticMesh->LODModels(0).RawTriangles.GetElementCount() )
				{
					for( INT TriangleIndex = 0 ; TriangleIndex < StaticMesh->LODModels(0).RawTriangles.GetElementCount() ; TriangleIndex++ )
					{
						const FStaticMeshTriangle& Triangle = RawTriangleData[TriangleIndex];
						FPoly* Polygon = new FPoly;

						// Add the poly

						Polygon->Init();
						Polygon->PolyFlags = PF_DefaultFlags;

						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[2] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[1] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[0] ) );

						Polygon->CalcNormal(1);
						Polygon->Fix();
						if( Polygon->Vertices.Num() > 2 )
						{
							if( !Polygon->Finalize( NULL, 1 ) )
							{
								SelectedPolys.AddItem( Polygon );
							}
						}

						// And add a flipped version of it to account for negative scaling

						Polygon = new FPoly;

						Polygon->Init();
						Polygon->PolyFlags = PF_DefaultFlags;

						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[2] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[0] ) );
						new(Polygon->Vertices) FVector(Actor->LocalToWorld().TransformFVector( Triangle.Vertices[1] ) );

						Polygon->CalcNormal(1);
						Polygon->Fix();
						if( Polygon->Vertices.Num() > 2 )
						{
							if( !Polygon->Finalize( NULL, 1 ) )
							{
								SelectedPolys.AddItem( Polygon );
							}
						}
					}
				}
				SMComp->StaticMesh->LODModels(0).RawTriangles.Unlock();
			}
		}
	}

	return SelectedPolys;
}

/**
* Utility function to quickly set the collision type on selected static meshes.
*
* @param	InCollisionType		The collision type to use (COLLIDE_??)
*/

void FAutoConvexCollision::SetCollisionTypeOnSelectedActors( BYTE InCollisionType )
{
	for( FSelectionIterator It( GEditor->GetSelectedActorIterator() ) ; It ; ++It )
	{
		AActor* Actor = static_cast<AActor*>( *It );
		checkSlow( Actor->IsA(AActor::StaticClass()) );
		AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);

		if( StaticMeshActor )
		{
			StaticMeshActor->Modify();
			StaticMeshActor->CollisionType = InCollisionType;
			StaticMeshActor->SetCollisionFromCollisionType();
		}
	}
}

/*-----------------------------------------------------------------------------
	WxUnrealEdApp
-----------------------------------------------------------------------------*/

/**
 * Function for getting the editor frame window without knowing about it
 */
class wxWindow* GetEditorFrame(void)
{
	return GApp->EditorFrame;
}

/**
 * Uses INI file configuration to determine which class to use to create
 * the editor's frame. Will fall back to Epic's hardcoded frame if it is
 * unable to create the configured one properly
 */
WxEditorFrame* WxUnrealEdApp::CreateEditorFrame(void)
{
	// Look up the name of the frame we are creating
	FString EditorFrameName;
	GConfig->GetString(TEXT("EditorFrame"),TEXT("FrameClassName"),
		EditorFrameName,GEditorIni);
	// In case the INI is messed up
	if (EditorFrameName.Len() == 0)
	{
		EditorFrameName = TEXT("WxEditorFrame");
	}
	// Use the wxWindows' RTTI system to create the window
	wxObject* NewObject = wxCreateDynamicObject(*EditorFrameName);
	if (NewObject == NULL)
	{
		debugf(TEXT("Failed to create the editor frame class %s"),
			*EditorFrameName);
		debugf(TEXT("Falling back to WxEditorFrame"));
		// Fallback to the default frame
		NewObject = new WxEditorFrame();
	}
	// Make sure it's the right type too
	if (wxDynamicCast(NewObject,WxEditorFrame) == NULL)
	{
		debugf(TEXT("Class %s is not derived from WxEditorFrame"),
			*EditorFrameName);
		debugf(TEXT("Falling back to WxEditorFrame"));
		delete NewObject;
		NewObject = new WxEditorFrame();
	}
	WxEditorFrame* Frame = wxDynamicCast(NewObject,WxEditorFrame);
	check(Frame);
	// Now do the window intialization
	Frame->Create();
	return Frame;
}

/**
 *  Updates text and value for various progress meters.
 *
 *	@param StatusText				New status text
 *	@param ProgressNumerator		Numerator for the progress meter (its current value).
 *	@param ProgressDenominitator	Denominiator for the progress meter (its range).
 */
void WxUnrealEdApp::StatusUpdateProgress(const TCHAR* StatusText, INT ProgressNumerator, INT ProgressDenominator)
{
	// Clean up deferred cleanup objects from rendering thread every once in a while.
	static DOUBLE LastTimePendingCleanupObjectsWhereDeleted;
	if( appSeconds() - LastTimePendingCleanupObjectsWhereDeleted > 1 )
	{
		// Get list of objects that are pending cleanup.
		FPendingCleanupObjects* PendingCleanupObjects = GetPendingCleanupObjects();
		// Flush rendering commands in the queue.
		FlushRenderingCommands();
		// It is now save to delete the pending clean objects.
		delete PendingCleanupObjects;
		// Keep track of time this operation was performed so we don't do it too often.
		LastTimePendingCleanupObjectsWhereDeleted = appSeconds();
	}

	// Update build progress dialog if it is visible.
	const UBOOL bBuildProgressDialogVisible = DlgBuildProgress->IsShown();

	if( bBuildProgressDialogVisible )
	{
		if( StatusText != NULL )
		{
			DlgBuildProgress->SetBuildStatusText( StatusText );
		}
		DlgBuildProgress->SetBuildProgressPercent( ProgressNumerator, ProgressDenominator );
		::wxSafeYield(DlgBuildProgress);
	}
}

/**
 * Returns whether or not the map build in progressed was cancelled by the user. 
 */
UBOOL WxUnrealEdApp::GetMapBuildCancelled() const
{
	return bCancelBuild;
}

/**
 * Sets the flag that states whether or not the map build was cancelled.
 *
 * @param InCancelled	New state for the cancelled flag.
 */
void WxUnrealEdApp::SetMapBuildCancelled( UBOOL InCancelled )
{
	bCancelBuild = InCancelled;
}

bool WxUnrealEdApp::OnInit()
{
	GApp = this;
	GApp->EditorFrame = NULL;

	UBOOL bIsOk = WxLaunchApp::OnInit( );
	if ( !bIsOk )
	{
		appHideSplash();
		return 0;
	}

	// Register all callback notifications
	GCallbackEvent->Register(CALLBACK_SelChange,this);
	GCallbackEvent->Register(CALLBACK_SurfProps,this);
	GCallbackEvent->Register(CALLBACK_ActorProps,this);
	GCallbackEvent->Register(CALLBACK_ActorPropertiesChange,this);
	GCallbackEvent->Register(CALLBACK_ObjectPropertyChanged,this);
	GCallbackEvent->Register(CALLBACK_RefreshPropertyWindows,this);
	GCallbackEvent->Register(CALLBACK_EndPIE,this);
	GCallbackEvent->Register(CALLBACK_DisplayLoadErrors,this);
	GCallbackEvent->Register(CALLBACK_RedrawAllViewports,this);
	GCallbackEvent->Register(CALLBACK_UpdateUI,this);
	GCallbackEvent->Register(CALLBACK_Undo,this);
	GCallbackEvent->Register(CALLBACK_MapChange,this);
	GCallbackEvent->Register(CALLBACK_RefreshEditor,this);
	GCallbackEvent->Register(CALLBACK_ChangeEditorMode,this);
	GCallbackEvent->Register(CALLBACK_EditorModeEnter,this);
	GCallbackEvent->Register(CALLBACK_PackageSaved,this);

	TerrainEditor = NULL;
	DlgActorSearch = NULL;
	DlgGeometryTools = NULL;
	SentinelTool = NULL;

	// Get the editor
	EditorFrame = CreateEditorFrame();
	check(EditorFrame);
//	EditorFrame->Maximize();

	GEditorModeTools().Init();
	GEditorModeTools().FindMode( EM_Default )->SetModeBar( new WxModeBarDefault( (wxWindow*)GApp->EditorFrame, -1 ) );
	GEditorModeTools().FindMode( EM_Geometry )->SetModeBar( new WxModeBarDefault( (wxWindow*)GApp->EditorFrame, -1 ) );
	GEditorModeTools().FindMode( EM_Texture )->SetModeBar( new WxModeBarTexture( (wxWindow*)GApp->EditorFrame, -1 ) );
	GEditorModeTools().FindMode( EM_CoverEdit )->SetModeBar( new WxModeBarDefault( (wxWindow*)GApp->EditorFrame, -1 ) );

	GEditorModeTools().Modes.AddItem( new FEdModeInterpEdit() );
	GEditorModeTools().FindMode( EM_InterpEdit )->SetModeBar( new WxModeBarInterpEdit( GApp->EditorFrame, -1 ) );

	// Global dialogs.
	GDlgAddSpecial = new WxDlgAddSpecial();

	// Initialize "last dir" array.
	GApp->LastDir[LD_UNR]				= GConfig->GetStr( TEXT("Directories"), TEXT("UNR"),			GEditorUserSettingsIni );
	GApp->LastDir[LD_BRUSH]				= GConfig->GetStr( TEXT("Directories"), TEXT("BRUSH"),			GEditorUserSettingsIni );
	GApp->LastDir[LD_2DS]				= GConfig->GetStr( TEXT("Directories"), TEXT("2DS"),			GEditorUserSettingsIni );
	GApp->LastDir[LD_PSA]				= GConfig->GetStr( TEXT("Directories"), TEXT("PSA"),			GEditorUserSettingsIni );
	GApp->LastDir[LD_COLLADA_ANIM]		= GConfig->GetStr( TEXT("Directories"), TEXT("COLLADAAnim"),	GEditorUserSettingsIni );
	GApp->LastDir[LD_GENERIC_IMPORT]	= GConfig->GetStr( TEXT("Directories"), TEXT("GenericImport"),	GEditorUserSettingsIni );
	GApp->LastDir[LD_GENERIC_EXPORT]	= GConfig->GetStr( TEXT("Directories"), TEXT("GenericExport"),	GEditorUserSettingsIni );
	GApp->LastDir[LD_GENERIC_OPEN]		= GConfig->GetStr( TEXT("Directories"), TEXT("GenericOpen"),	GEditorUserSettingsIni );
	GApp->LastDir[LD_GENERIC_SAVE]		= GConfig->GetStr( TEXT("Directories"), TEXT("GenericSave"),	GEditorUserSettingsIni );

	// Remap editor directories to the user directory in shipped builds.
	// Also remapped \Content\ to \CookedPC\.
#if !SHIPPING_PC_GAME
	if ( ParseParam(appCmdLine(), TEXT("installed")) )
#endif
	{
		for ( INT PathIndex = 0 ; PathIndex < LD_MAX ; ++PathIndex )
		{
			GApp->LastDir[PathIndex] = GFileManager->ConvertAbsolutePathToUserPath( *GFileManager->ConvertToAbsolutePath(*GApp->LastDir[PathIndex]) );
			GApp->LastDir[PathIndex] = GApp->LastDir[PathIndex].Replace(TEXT("\\Content\\"),TEXT("\\CookedPC\\"),TRUE);
		}
	}

	if( !GConfig->GetString( TEXT("URL"), TEXT("MapExt"), GApp->MapExt, GEngineIni ) )
	{
		appErrorf(*LocalizeUnrealEd("Error_MapExtUndefined"));
	}
	GEditorModeTools().SetCurrentMode( EM_Default );
	GUnrealEd->Exec( *FString::Printf(TEXT("MODE MAPEXT=%s"), *GApp->MapExt ) );

#if HAVE_SCC
	// Init source control.
	SCC = new FSourceControlIntegration;
#else
	SCC = NULL;
#endif

	// Init the editor tools.
	GTexAlignTools.Init();

	EditorFrame->ButtonBar = new WxButtonBar;
	EditorFrame->ButtonBar->Create( (wxWindow*)EditorFrame, -1 );
	EditorFrame->ButtonBar->Show();

	DlgActorSearch = new WxDlgActorSearch( EditorFrame );
	DlgActorSearch->Show(0);

	DlgMapCheck = new WxDlgMapCheck( EditorFrame );
	DlgMapCheck->Show(0);

	DlgActorFactory = new WxDlgActorFactory( );
	DlgActorFactory->Show(0);

	DlgBuildProgress = new WxDlgBuildProgress( EditorFrame );
	DlgBuildProgress->Show(0);

	DlgTransform = new WxDlgTransform( EditorFrame );
	DlgTransform->Show(false);

	AutoConvexHelper = new FAutoConvexCollision();

	DlgAutoConvexCollision = new WxConvexDecompOptions( AutoConvexHelper, EditorFrame, TRUE, TRUE );
	DlgAutoConvexCollision->Show(false);

	// Generate mapping from wx keys to unreal keys.
	GenerateWxToUnrealKeyMap();

	DlgBindHotkeys = new WxDlgBindHotkeys( EditorFrame );
	DlgBindHotkeys->Show(FALSE);

	//joegtemp -- Set the handle to use for GWarn->MapCheck_Xxx() calls
	GWarn->winEditorFrame	= (DWORD)EditorFrame;
	GWarn->hWndEditorFrame	= (DWORD)EditorFrame->GetHandle();

	DlgLoadErrors = new WxDlgLoadErrors( EditorFrame );

	DlgSurfProp = new WxDlgSurfaceProperties();
	//DlgSurfProp->Show( 0 );

	ObjectPropertyWindow = NULL;
	CurrentPropertyWindow = NULL;

	EditorFrame->OptionProxyInit();

	// Resources.
	MaterialEditor_RightHandleOn	= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_RightHandleOn"), NULL, LOAD_None, NULL ));
	MaterialEditor_RightHandle		= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_RightHandle"), NULL, LOAD_None, NULL ));
	MaterialEditor_RGBA				= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_RGBA"), NULL, LOAD_None, NULL ));
	MaterialEditor_R				= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_R"), NULL, LOAD_None, NULL ));
	MaterialEditor_G				= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_G"), NULL, LOAD_None, NULL ));
	MaterialEditor_B				= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_B"), NULL, LOAD_None, NULL ));
	MaterialEditor_A				= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_A"), NULL, LOAD_None, NULL ));
	MaterialEditor_LeftHandle		= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_LeftHandle"), NULL, LOAD_None, NULL ));
	MaterialEditor_ControlPanelFill = CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_ControlPanelFill"), NULL, LOAD_None, NULL ));
	MaterialEditor_ControlPanelCap	= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_ControlPanelCap"), NULL, LOAD_None, NULL ));
	UpArrow							= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_Up"), NULL, LOAD_None, NULL ));
	DownArrow						= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_Down"), NULL, LOAD_None, NULL ));
	MaterialEditor_LabelBackground	= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_LabelBackground"), NULL, LOAD_None, NULL ));
	MaterialEditor_Delete			= CastChecked<UTexture2D>(UObject::StaticLoadObject( UTexture2D::StaticClass(), ANY_PACKAGE, TEXT("EditorResources.MaterialEditor_Delete"), NULL, LOAD_None, NULL ));

	UClass* Parent = UMaterialExpression::StaticClass();
	INT ID = IDM_NEW_SHADER_EXPRESSION_START;

	if( Parent )
	{
		for(TObjectIterator<UClass> It;It;++It)
			if(It->IsChildOf(UMaterialExpression::StaticClass()) && !(It->ClassFlags & CLASS_Abstract))
			{
				ShaderExpressionMap.Set( ID, *It );
				ID++;
			}
	}


	// ToolTip preferences
	{
#if !PLATFORM_UNIX && !PLATFORM_MACOSX
		// ToolTip delay
		INT ToolTipDelayInMS = -1;
		GConfig->GetInt( TEXT( "ToolTips" ), TEXT( "DelayInMS" ), ToolTipDelayInMS, GEditorUserSettingsIni );

		// ToolTip duration
		INT ToolTipDurationInMS = 20000;
		GConfig->GetInt( TEXT( "ToolTips" ), TEXT( "DurationInMS" ), ToolTipDurationInMS, GEditorUserSettingsIni );

		// Send the tool tip settings to WxWidgets.  Note that values of -1 mean 'use defaults'.
		wxToolTip::SetDelay( ToolTipDelayInMS );
		wxToolTip::SetDisplayTime( ToolTipDurationInMS );

		// Save preferences
		GConfig->SetInt( TEXT( "ToolTips" ), TEXT( "DelayInMS" ), ToolTipDelayInMS, GEditorUserSettingsIni );
		GConfig->SetInt( TEXT( "ToolTips" ), TEXT( "DurationInMS" ), ToolTipDurationInMS, GEditorUserSettingsIni );
#endif
	}


	// Do final set up on the editor frame and show it

	EditorFrame->SetUp();
	appHideSplash();
	EditorFrame->Show();

	// Load the viewport configuration from the INI file.

	EditorFrame->ViewportConfigData = new FViewportConfig_Data;
	if( !EditorFrame->ViewportConfigData->LoadFromINI() )
	{
		EditorFrame->ViewportConfigData->SetTemplate( VC_2_2_Split );
		EditorFrame->ViewportConfigData->Apply( EditorFrame->ViewportContainer );
	}

	GEditorModeTools().SetCurrentMode( EM_Default );

	// Show tip of the day dialog.
	DlgTipOfTheDay = new WxDlgTipOfTheDay( EditorFrame);

	// see if the user specified an initial map to load on the commandline
	const TCHAR* ParsedCmdLine = appCmdLine();

	// Tick the client so it can deliver deferred window messages.
	// This is needed to ensure the D3D RHI has been initialized when the map is loaded and its scene's static mesh draw lists created.
	// When the static mesh draw lists are created, it creates the RHI bound shader states, so the RHI needs to be initialized.
	check(GEngine);
	check(GEngine->Client);
	GEngine->Client->Tick(0.0f);

	FString ParsedMapName;
	if ( ParseToken(ParsedCmdLine, ParsedMapName, FALSE) )
	{
		FFilename InitialMapName;
		// if the specified package exists
		if ( GPackageFileCache->FindPackageFile(*ParsedMapName, NULL, (FString&)InitialMapName) &&

			// and it's a valid map file
			InitialMapName.GetExtension() == MapExt )
		{
			FEditorFileUtils::LoadMap(InitialMapName);
		}
	}

	if(DlgTipOfTheDay->GetShowAtStartup())
	{
		DlgTipOfTheDay->Show();
	}

	GWarn->MapCheck_ShowConditionally();

	return 1;
}

int WxUnrealEdApp::OnExit()
{
	// Save out default file directories
	GConfig->SetString( TEXT("Directories"), TEXT("UNR"),				*GApp->LastDir[LD_UNR],				GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("BRUSH"),				*GApp->LastDir[LD_BRUSH],			GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("2DS"),				*GApp->LastDir[LD_2DS],				GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("PSA"),				*GApp->LastDir[LD_PSA],				GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("COLLADAAnim"),		*GApp->LastDir[LD_COLLADA_ANIM],	GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("GenericImport"),		*GApp->LastDir[LD_GENERIC_IMPORT],	GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("GenericExport"),		*GApp->LastDir[LD_GENERIC_EXPORT],	GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("GenericOpen"),		*GApp->LastDir[LD_GENERIC_OPEN],	GEditorUserSettingsIni );
	GConfig->SetString( TEXT("Directories"), TEXT("GenericSave"),		*GApp->LastDir[LD_GENERIC_SAVE],	GEditorUserSettingsIni );

	// Unregister all events
	GCallbackEvent->UnregisterAll(this);

#if HAVE_SCC
	delete SCC;
	SCC = NULL;
#endif

	if( GLogConsole )
	{
		GLogConsole->Show( FALSE );
	}

	return WxLaunchApp::OnExit();
}

/**
 * Performs any required cleanup in the case of a fatal error.
 */
void WxUnrealEdApp::ShutdownAfterError()
{
	WxLaunchApp::ShutdownAfterError();

#if HAVE_SCC
	// We need to unbind the Perforce DLL before GMalloc shuts down as it seems to use new/ delete.
	delete SCC;
	SCC = NULL;
#endif
}

/** Generate a mapping of wx keys to unreal key names */
void WxUnrealEdApp::GenerateWxToUnrealKeyMap()
{
	WxKeyToUnrealKeyMap.Set(WXK_LBUTTON,KEY_LeftMouseButton);
	WxKeyToUnrealKeyMap.Set(WXK_RBUTTON,KEY_RightMouseButton);
	WxKeyToUnrealKeyMap.Set(WXK_MBUTTON,KEY_MiddleMouseButton);

	WxKeyToUnrealKeyMap.Set(WXK_BACK,KEY_BackSpace);
	WxKeyToUnrealKeyMap.Set(WXK_TAB,KEY_Tab);
	WxKeyToUnrealKeyMap.Set(WXK_RETURN,KEY_Enter);
	WxKeyToUnrealKeyMap.Set(WXK_PAUSE,KEY_Pause);

	WxKeyToUnrealKeyMap.Set(WXK_CAPITAL,KEY_CapsLock);
	WxKeyToUnrealKeyMap.Set(WXK_ESCAPE,KEY_Escape);
	WxKeyToUnrealKeyMap.Set(WXK_SPACE,KEY_SpaceBar);
	WxKeyToUnrealKeyMap.Set(WXK_PRIOR,KEY_PageUp);
	WxKeyToUnrealKeyMap.Set(WXK_NEXT,KEY_PageDown);
	WxKeyToUnrealKeyMap.Set(WXK_END,KEY_End);
	WxKeyToUnrealKeyMap.Set(WXK_HOME,KEY_Home);

	WxKeyToUnrealKeyMap.Set(WXK_LEFT,KEY_Left);
	WxKeyToUnrealKeyMap.Set(WXK_UP,KEY_Up);
	WxKeyToUnrealKeyMap.Set(WXK_RIGHT,KEY_Right);
	WxKeyToUnrealKeyMap.Set(WXK_DOWN,KEY_Down);

	WxKeyToUnrealKeyMap.Set(WXK_INSERT,KEY_Insert);
	WxKeyToUnrealKeyMap.Set(WXK_DELETE,KEY_Delete);

	WxKeyToUnrealKeyMap.Set(0x30,KEY_Zero);
	WxKeyToUnrealKeyMap.Set(0x31,KEY_One);
	WxKeyToUnrealKeyMap.Set(0x32,KEY_Two);
	WxKeyToUnrealKeyMap.Set(0x33,KEY_Three);
	WxKeyToUnrealKeyMap.Set(0x34,KEY_Four);
	WxKeyToUnrealKeyMap.Set(0x35,KEY_Five);
	WxKeyToUnrealKeyMap.Set(0x36,KEY_Six);
	WxKeyToUnrealKeyMap.Set(0x37,KEY_Seven);
	WxKeyToUnrealKeyMap.Set(0x38,KEY_Eight);
	WxKeyToUnrealKeyMap.Set(0x39,KEY_Nine);

	WxKeyToUnrealKeyMap.Set(0x41,KEY_A);
	WxKeyToUnrealKeyMap.Set(0x42,KEY_B);
	WxKeyToUnrealKeyMap.Set(0x43,KEY_C);
	WxKeyToUnrealKeyMap.Set(0x44,KEY_D);
	WxKeyToUnrealKeyMap.Set(0x45,KEY_E);
	WxKeyToUnrealKeyMap.Set(0x46,KEY_F);
	WxKeyToUnrealKeyMap.Set(0x47,KEY_G);
	WxKeyToUnrealKeyMap.Set(0x48,KEY_H);
	WxKeyToUnrealKeyMap.Set(0x49,KEY_I);
	WxKeyToUnrealKeyMap.Set(0x4A,KEY_J);
	WxKeyToUnrealKeyMap.Set(0x4B,KEY_K);
	WxKeyToUnrealKeyMap.Set(0x4C,KEY_L);
	WxKeyToUnrealKeyMap.Set(0x4D,KEY_M);
	WxKeyToUnrealKeyMap.Set(0x4E,KEY_N);
	WxKeyToUnrealKeyMap.Set(0x4F,KEY_O);
	WxKeyToUnrealKeyMap.Set(0x50,KEY_P);
	WxKeyToUnrealKeyMap.Set(0x51,KEY_Q);
	WxKeyToUnrealKeyMap.Set(0x52,KEY_R);
	WxKeyToUnrealKeyMap.Set(0x53,KEY_S);
	WxKeyToUnrealKeyMap.Set(0x54,KEY_T);
	WxKeyToUnrealKeyMap.Set(0x55,KEY_U);
	WxKeyToUnrealKeyMap.Set(0x56,KEY_V);
	WxKeyToUnrealKeyMap.Set(0x57,KEY_W);
	WxKeyToUnrealKeyMap.Set(0x58,KEY_X);
	WxKeyToUnrealKeyMap.Set(0x59,KEY_Y);
	WxKeyToUnrealKeyMap.Set(0x5A,KEY_Z);

	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD0,KEY_NumPadZero);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD1,KEY_NumPadOne);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD2,KEY_NumPadTwo);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD3,KEY_NumPadThree);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD4,KEY_NumPadFour);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD5,KEY_NumPadFive);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD6,KEY_NumPadSix);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD7,KEY_NumPadSeven);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD8,KEY_NumPadEight);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD9,KEY_NumPadNine);

	WxKeyToUnrealKeyMap.Set(WXK_MULTIPLY,KEY_Multiply);

	WxKeyToUnrealKeyMap.Set(WXK_ADD,KEY_Add);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD_ADD,KEY_Add);

	WxKeyToUnrealKeyMap.Set(WXK_SUBTRACT,KEY_Subtract);
	WxKeyToUnrealKeyMap.Set(WXK_NUMPAD_SUBTRACT,KEY_Subtract);

	WxKeyToUnrealKeyMap.Set(WXK_DECIMAL,KEY_Decimal);
	WxKeyToUnrealKeyMap.Set(WXK_DIVIDE,KEY_Divide);

	WxKeyToUnrealKeyMap.Set(WXK_F1,KEY_F1);
	WxKeyToUnrealKeyMap.Set(WXK_F2,KEY_F2);
	WxKeyToUnrealKeyMap.Set(WXK_F3,KEY_F3);
	WxKeyToUnrealKeyMap.Set(WXK_F4,KEY_F4);
	WxKeyToUnrealKeyMap.Set(WXK_F5,KEY_F5);
	WxKeyToUnrealKeyMap.Set(WXK_F6,KEY_F6);
	WxKeyToUnrealKeyMap.Set(WXK_F7,KEY_F7);
	WxKeyToUnrealKeyMap.Set(WXK_F8,KEY_F8);
	WxKeyToUnrealKeyMap.Set(WXK_F9,KEY_F9);
	WxKeyToUnrealKeyMap.Set(WXK_F10,KEY_F10);
	WxKeyToUnrealKeyMap.Set(WXK_F11,KEY_F11);
	WxKeyToUnrealKeyMap.Set(WXK_F12,KEY_F12);

	WxKeyToUnrealKeyMap.Set(WXK_NUMLOCK,KEY_NumLock);

	WxKeyToUnrealKeyMap.Set(WXK_SCROLL,KEY_ScrollLock);

	WxKeyToUnrealKeyMap.Set(WXK_SHIFT,KEY_LeftShift);
	//WxKeyToUnrealKeyMap.Set(WXK_SHIFT,KEY_RightShift);
	WxKeyToUnrealKeyMap.Set(WXK_CONTROL,KEY_LeftControl);
	//WxKeyToUnrealKeyMap.Set(WXK_RCONTROL,KEY_RightControl);
	WxKeyToUnrealKeyMap.Set(WXK_ALT,KEY_LeftAlt);
	//WxKeyToUnrealKeyMap.Set(WXK_RMENU,KEY_RightAlt);

	WxKeyToUnrealKeyMap.Set(0x3B,KEY_Semicolon);
	WxKeyToUnrealKeyMap.Set(0x2B,KEY_Equals);
	WxKeyToUnrealKeyMap.Set(0x2C,KEY_Comma);
	WxKeyToUnrealKeyMap.Set(0x2D,KEY_Underscore);	// NOTE: Really should be 'Subtract', but UE is weird.
	WxKeyToUnrealKeyMap.Set(0x5F,KEY_Underscore);
	WxKeyToUnrealKeyMap.Set(0x2E,KEY_Period);
	WxKeyToUnrealKeyMap.Set(0x2F,KEY_Slash);
	WxKeyToUnrealKeyMap.Set(0x7E,KEY_Tilde);
	WxKeyToUnrealKeyMap.Set(0x5B,KEY_LeftBracket);
	WxKeyToUnrealKeyMap.Set(0x5C,KEY_Backslash);
	WxKeyToUnrealKeyMap.Set(0x5D,KEY_RightBracket);
	WxKeyToUnrealKeyMap.Set(0x2F,KEY_Quote);
}

/**
 * @return Returns a unreal key name given a wx key event.
 */
FName WxUnrealEdApp::GetKeyName(wxKeyEvent &Event)
{
	FName* KeyName = WxKeyToUnrealKeyMap.Find(Event.GetKeyCode());

	if(KeyName)
	{
		return *KeyName;
	}
	else
	{
		return NAME_None;
	}
}


// Current selection changes (either actors or BSP surfaces).

void WxUnrealEdApp::CB_SelectionChanged()
{
	DlgSurfProp->RefreshPages();
	EditorFrame->UpdateUI();
}


// Shows the surface properties dialog

void WxUnrealEdApp::CB_SurfProps()
{
	DlgSurfProp->Show( 1 );

}

// Shows the actor properties dialog

void WxUnrealEdApp::CB_ActorProps()
{
	GUnrealEd->ShowActorProperties();

}

// Called whenever the user changes the camera mode

void WxUnrealEdApp::CB_CameraModeChanged()
{
	EditorFrame->ModeBar = GEditorModeTools().GetCurrentMode()->GetModeBar();
	wxSizeEvent DummyEvent;
	EditorFrame->OnSize( DummyEvent );
}

// Called whenever an actor has one of it's properties changed

void WxUnrealEdApp::CB_ActorPropertiesChanged()
{
}

// Called whenever an object has its properties changed

void WxUnrealEdApp::CB_ObjectPropertyChanged(UObject* Object)
{
	// all non-active actor prop windows that have this actor need to refresh
	for (INT WindowIndex = 0; WindowIndex < GPropertyWindowManager->PropertyWindows.Num(); WindowIndex++ )
	{
		// CurrentPropertyWindow denotes the current property window, which we don't update
		// because we don't want to destroy the window out from under it.
		if(GPropertyWindowManager->PropertyWindows(WindowIndex) != CurrentPropertyWindow)
		{
			GPropertyWindowManager->PropertyWindows(WindowIndex)->Rebuild(Object);
		}
	}
}

void WxUnrealEdApp::CB_RefreshPropertyWindows()
{
	for (INT WindowIndex = 0; WindowIndex < GPropertyWindowManager->PropertyWindows.Num(); WindowIndex++ )
	{
		GPropertyWindowManager->PropertyWindows(WindowIndex)->Refresh();
	}
}

void WxUnrealEdApp::CB_DisplayLoadErrors()
{
	DlgLoadErrors->Update();
	DlgLoadErrors->Show();
}

void WxUnrealEdApp::CB_RefreshEditor()
{
	if ( GEditorModeTools().GetCurrentMode()->GetModeBar() )
	{
		GEditorModeTools().GetCurrentMode()->GetModeBar()->Refresh();
	}

	GCallbackEvent->Send( CALLBACK_RefreshEditor_AllBrowsers );

	if( DlgActorSearch )
	{
		DlgActorSearch->UpdateResults();
	}
}

// Tells the editor that something has been done to change the map.  Can be
// anything from loading a whole new map to changing the BSP.

void WxUnrealEdApp::CB_MapChange( DWORD InFlags )
{
	// Clear property coloration settings.
	const FString EmptyString;
	GEditor->SetPropertyColorationTarget( EmptyString, NULL, NULL, NULL );

	// Rebuild the collision hash if this map change is something major ("new", "open", etc).
	// Minor things like brush subtraction will set it to "0".

	if( InFlags != MapChangeEventFlags::Default )
	{	
		GEditor->ClearComponents();
		GWorld->CleanupWorld();
	}

	GEditor->UpdateComponents();

	// Reset UOptions.
#if 0
	EditorFrame->OptionProxyInit();
#endif
	GEditorModeTools().GetCurrentMode()->MapChangeNotify();

	// Refresh the editor mode.
	const EEditorMode CurrentEditorMode = GEditorModeTools().GetCurrentModeID();
	GEditorModeTools().SetCurrentMode( CurrentEditorMode );

	if ( GEditor->LevelProperties )
	{
		GEditor->LevelProperties->SetObject( NULL, FALSE, TRUE, TRUE, FALSE );
		GEditor->LevelProperties->Show( FALSE );
		GEditor->LevelProperties->ClearLastFocused();
	}

	GPropertyWindowManager->ClearReferencesAndHide();

	CB_RefreshEditor();

	// Only reset the auto save timer if we've created or loaded a new map
	if( InFlags & MapChangeEventFlags::NewMap )
	{
		GEditor->ResetAutosaveTimer();
	}
}

void WxUnrealEdApp::CB_RedrawAllViewports()
{
	GUnrealEd->RedrawAllViewports();
}

void WxUnrealEdApp::CB_Undo()
{
	if( GEditorModeTools().GetCurrentModeID() == EM_Geometry )
	{
		FEdModeGeometry* mode = (FEdModeGeometry*)GEditorModeTools().GetCurrentMode();
		mode->PostUndo();
	}
}

void WxUnrealEdApp::CB_EndPIE()
{
	if ( GDebugToolExec )
	{
		((FDebugToolExec*)GDebugToolExec)->CleanUpAfterPIE();
	}
}

void WxUnrealEdApp::CB_EditorModeEnter(const FEdMode& InEdMode)
{
	// Show/Hide mode bars as appropriate.
	for( INT x = 0 ; x < GEditorModeTools().Modes.Num() ; ++x )
	{
		FEdMode* CurrentMode = GEditorModeTools().Modes(x);
		WxModeBar* CurrentModeBar = CurrentMode->GetModeBar();
		if ( CurrentModeBar )
		{
			CurrentModeBar->Show( false );
		}
	}

	// Hide all dialogs.
	if ( TerrainEditor )
	{
		TerrainEditor->Show( false );
	}

	if ( DlgGeometryTools )
	{
		DlgGeometryTools->Show( false );
	}


	// Set the current modebar/dialog to show up at the top of the editor frame.
	FEdMode* CurrentMode = GEditorModeTools().GetCurrentMode();
	WxModeBar* CurrentModeBar = CurrentMode->GetModeBar();
	const EEditorMode CurrentModeID = InEdMode.GetID();
	EditorFrame->ModeBar = CurrentModeBar;
	
	switch( CurrentModeID )
	{
	case EM_TerrainEdit:
		if ( !TerrainEditor )
		{
			TerrainEditor = new WxTerrainEditor( EditorFrame );
		}

		TerrainEditor->Show( true );
		break;
	case EM_Geometry:
		if ( !DlgGeometryTools )
		{
			DlgGeometryTools = new WxDlgGeometryTools( EditorFrame );
		}

		DlgGeometryTools->Show( true  );
		break;
	default:
		CurrentModeBar->Show( true );
		break;
	}


	wxStatusBar* StatusBar = EditorFrame->GetStatusBar();
	if ( StatusBar )
	{
		StatusBar->Refresh();
	}

	// Force the frame to resize itself to accommodate the new mode bar.
	wxSizeEvent DummyEvent;
	EditorFrame->OnSize( DummyEvent );
}

/**
 * Routes the event to the appropriate handlers
 *
 * @param InType the event that was fired
 */
void WxUnrealEdApp::Send(ECallbackEventType InType)
{
	switch( InType )
	{
		case CALLBACK_SelChange:
			CB_SelectionChanged();
			break;
		case CALLBACK_SurfProps:
			CB_SurfProps();
			break;
		case CALLBACK_ActorProps:
			CB_ActorProps();
			break;
		case CALLBACK_ActorPropertiesChange:
			CB_ActorPropertiesChanged();
			break;
		case CALLBACK_DisplayLoadErrors:
			CB_DisplayLoadErrors();
			break;
		case CALLBACK_RedrawAllViewports:
			CB_RedrawAllViewports();
			break;
		case CALLBACK_UpdateUI:
			EditorFrame->UpdateUI();
			break;
		case CALLBACK_Undo:
			GApp->CB_Undo();
			break;
		case CALLBACK_EndPIE:
			GApp->CB_EndPIE();
			break;
		case CALLBACK_RefreshPropertyWindows:
			CB_RefreshPropertyWindows();
			break;
		case CALLBACK_RefreshEditor:
			CB_RefreshEditor();
			break;
	}
}

/**
 * Routes the event to the appropriate handlers
 *
 * @param InType the event that was fired
 * @param InFlags the flags for this event
 */
void WxUnrealEdApp::Send(ECallbackEventType InType,DWORD InFlag)
{
	switch( InType )
	{
		case CALLBACK_MapChange:
			CB_MapChange( InFlag );
			break;
		case CALLBACK_ChangeEditorMode:
			{
				const EEditorMode RequestedEditorMode = (EEditorMode)InFlag;
				GEditorModeTools().SetCurrentMode( RequestedEditorMode );
			}
			break;
	}
}


/**
 * Routes the event to the appropriate handlers
 *
 * @param InObject the relevant object for this event
 */
void WxUnrealEdApp::Send(ECallbackEventType InType, UObject* InObject)
{
	switch( InType )
	{
		case CALLBACK_ObjectPropertyChanged:
			CB_ObjectPropertyChanged(InObject);
			break;
	}
}

/**
 * Routes the event to the appropriate handlers
 *
 * @param InType the event that was fired
 * @param InEdMode the FEdMode that is changing
 */
void WxUnrealEdApp::Send(ECallbackEventType InType, FEdMode* InEdMode)
{
	switch( InType )
	{
		case CALLBACK_EditorModeEnter:
		{
			check( InEdMode );
			CB_EditorModeEnter( *InEdMode );
			break;
		}
	}
}

/**
 * Notifies all observers that are registered for this event type
 * that the event has fired
 *
 * @param InType the event that was fired
 * @param InString the string information associated with this event
 * @param InObject the object associated with this event
 */
void WxUnrealEdApp::Send(ECallbackEventType InType,const FString& InString, UObject* InObject)
{
	switch (InType)
	{
		// save the shader cache when we save a package
		case CALLBACK_PackageSaved:
		{
			SaveLocalShaderCaches();
			break;
		}
	}
}


