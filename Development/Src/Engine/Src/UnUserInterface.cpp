/*=============================================================================
	UnUserInterface.cpp: UI system structs, utility, and helper class implementations.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// engine classes
#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "CanvasScene.h"
#include "ScenePrivate.h"

// widgets and supporting UI classes
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

// UI kismet classes
#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

// Utility classes
#include "ScopedObjectStateChange.h"

IMPLEMENT_CLASS(UUIRoot);
	IMPLEMENT_CLASS(UUISceneClient);
		IMPLEMENT_CLASS(UGameUISceneClient);

	IMPLEMENT_CLASS(UUITexture);
	IMPLEMENT_CLASS(UUIState);
		IMPLEMENT_CLASS(UUIState_Enabled);
		IMPLEMENT_CLASS(UUIState_Disabled);
		IMPLEMENT_CLASS(UUIState_Active);
		IMPLEMENT_CLASS(UUIState_Focused);
		IMPLEMENT_CLASS(UUIState_Pressed);
		IMPLEMENT_CLASS(UUIState_TargetedTab);

IMPLEMENT_CLASS(UUILayerBase);

#define TEMP_SPLITSCREEN_INDEX UCONST_TEMP_SPLITSCREEN_INDEX

DECLARE_STATS_GROUP(TEXT("UI"),STATGROUP_UI);
DECLARE_CYCLE_STAT(TEXT("UI Kismet Time"),STAT_UIKismetTime,STATGROUP_UI);
DECLARE_CYCLE_STAT(TEXT("UI Scene Render Time"),STAT_UISceneRenderTime,STATGROUP_UI);
DECLARE_CYCLE_STAT(TEXT("UI Drawing Time"),STAT_UIDrawingTime,STATGROUP_UI);
DECLARE_CYCLE_STAT(TEXT("UI Scene Tick Time"),STAT_UISceneTickTime,STATGROUP_UI);
DECLARE_CYCLE_STAT(TEXT("UI Tick Time"),STAT_UITickTime,STATGROUP_UI);
//DECLARE_CYCLE_STAT(TEXT("ResolvePosition Time (General)"),STAT_UIResolvePosition,STATGROUP_UI);
//DECLARE_CYCLE_STAT(TEXT("CalculateBaseValue Time"),STAT_UICalculateBaseValue,STATGROUP_UI);
//DECLARE_CYCLE_STAT(TEXT("GetPositionValue Time"),STAT_UIGetPositionValue,STATGROUP_UI);
//DECLARE_CYCLE_STAT(TEXT("SetPositionValue Time"),STAT_UISetPositionValue,STATGROUP_UI);
//DECLARE_CYCLE_STAT(TEXT(""),STAT,STATGROUP_UI);

HUIHitProxy::HUIHitProxy( UUIScreenObject* InObject )
: HObject(InObject), UIObject(InObject)
{
}

/**
 * Creates and initializes an instance of a UDataStoreClient.
 *
 * @param	InOuter		the object to use for Outer when creating the global data store client
 *
 * @return	a pointer to a fully initialized instance of the global data store client class.
 */
UDataStoreClient* FGlobalDataStoreClientManager::CreateGlobalDataStoreClient( UObject* InOuter ) const
{
	UDataStoreClient* Result = NULL;
	if ( GEngine->DataStoreClientClass != NULL )
	{
		Result = ConstructObject<UDataStoreClient>(GEngine->DataStoreClientClass, InOuter, NAME_None, RF_Transient);
		if ( Result != NULL )
		{
			Result->AddToRoot();
		}
	}

	return Result;
}

/* ==========================================================================================================
	UUIInputConfiguration
========================================================================================================== */
/**
 * Returns the list of widget class input aliases.
 */
TArray<FUIInputAliasClassMap>& UUIInputConfiguration::GetInputAliasList()
{
	return WidgetInputAliases;
}

/**
 * Loads all widget classes in the WidgetInputAliases array.
 */
void UUIInputConfiguration::LoadInputAliasClasses()
{
	checkf(!HasAnyFlags(RF_ClassDefaultObject), TEXT("Modifying object references in class default objects isn't allowed! Create an instance instead."));

	for ( INT ClassIndex = 0; ClassIndex < WidgetInputAliases.Num(); ClassIndex++ )
	{
		FUIInputAliasClassMap& ClassInputAliasMappings = WidgetInputAliases(ClassIndex);
		if ( ClassInputAliasMappings.WidgetClass == NULL && ClassInputAliasMappings.WidgetClassName.Len() > 0 )
		{
			ClassInputAliasMappings.WidgetClass = LoadClass<UUIScreenObject>(NULL, *ClassInputAliasMappings.WidgetClassName, NULL, LOAD_None, NULL);
		}

		ClassInputAliasMappings.WidgetStates.Shrink();

		for ( INT StateIndex = 0; StateIndex < ClassInputAliasMappings.WidgetStates.Num(); StateIndex++ )
		{
			FUIInputAliasStateMap& StateAliasMappings = ClassInputAliasMappings.WidgetStates(StateIndex);

			// load the state class references from the StateClassName value
			if ( StateAliasMappings.State == NULL && StateAliasMappings.StateClassName.Len() > 0 )
			{
				StateAliasMappings.State = LoadClass<UUIState>(NULL, *StateAliasMappings.StateClassName, NULL, LOAD_None, NULL);
			}
		}
	}

	// now find all widget classes which didn't have an entry in the ini
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		if ( It->IsChildOf(UUIScreenObject::StaticClass())
		&&	!It->HasAnyClassFlags(CLASS_Abstract) )
		{
			UBOOL bAddClass = TRUE;
			for ( INT InputIndex = 0; InputIndex < WidgetInputAliases.Num(); InputIndex++ )
			{
				if ( WidgetInputAliases(InputIndex).WidgetClass == *It )
				{
					bAddClass = FALSE;
					break;
				}
			}

			if ( bAddClass )
			{
				// and add a blank entry for it.
				FUIInputAliasClassMap* DummyEntry = new(WidgetInputAliases) FUIInputAliasClassMap(EC_EventParm);
				DummyEntry->WidgetClass = *It;
				DummyEntry->WidgetClassName = It->GetPathName();
			}
		}
	}
}

/**
 * Called when the current map is being unloaded.  Cleans up any references which would prevent garbage collection.
 */
void UUIInputConfiguration::NotifyGameSessionEnded()
{
	checkf(!HasAnyFlags(RF_ClassDefaultObject), TEXT("Modifying object references in class default objects isn't allowed! Create an instance instead."));

	if ( GIsGame )
	{
		debugf(TEXT("%s: received notification that %s is being unloaded.  Clearing references to all non-native classes"), *GetName(), *GWorld->GetOutermost()->GetName());
		for ( INT ClassIndex = 0; ClassIndex < WidgetInputAliases.Num(); ClassIndex++ )
		{
			FUIInputAliasClassMap& ClassInputAliasMappings = WidgetInputAliases(ClassIndex);
			if ( ClassInputAliasMappings.WidgetClass != NULL && !ClassInputAliasMappings.WidgetClass->HasAnyFlags(RF_Native|RF_RootSet|RF_DisregardForGC) )
			{
				ClassInputAliasMappings.WidgetClass = NULL;
			}
		}
	}
}

/* ==========================================================================================================
	UUITickableObjectProxy
========================================================================================================== */
/**
 * Called each frame to allow the object to perform work.
 *
 * @param	PreviousFrameSeconds	amount of time (in seconds) between the start of this frame and the start of the previous frame.
 */
void UUITickableObjectProxy::Tick( FLOAT PreviousFrameSeconds )
{
	if ( DELEGATE_IS_SET(OnScriptTick) )
	{
		delegateOnScriptTick(this, PreviousFrameSeconds);
	}

	eventScriptTick(PreviousFrameSeconds);
}
IMPLEMENT_CLASS(UUITickableObjectProxy);

/* ==========================================================================================================
	UUISceneClient
========================================================================================================== */
/**
 * Handles processing console commands.
 *
 * @param	Cmd		the text that was entered into the console
 * @param	Ar		the archive to use for serializing responses
 *
 * @return	TRUE if the command specified was processed
 */
UBOOL UUISceneClient::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if ( ScriptConsoleExec(Cmd, Ar, this) )
	{
		return TRUE;
	}
	return FALSE;
}

/**
 * Performs any initialization for the UISceneClient.
 *
 * @param	InitialSkin		UISkin that should be set to the initial ActiveSkin
 */
void UUISceneClient::InitializeClient( UUISkin* InitialSkin )
{
	if( OpacityParameter == NULL )
	{
		OpacityParameter = ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass() );

		OpacityParameter->ScalarParameterValues.Empty();
		OpacityParameter->ScalarParameterValues.AddZeroed( 1 );
		OpacityParameter->ScalarParameterValues(0).ParameterName	= OpacityParameterName;
		OpacityParameter->ScalarParameterValues(0).ParameterValue	= 1.f;
	}

	// use the default post process chain for UI scenes
	if( UIScenePostProcess == NULL )
	{
		UIScenePostProcess = GEngine->DefaultUIScenePostProcess;
	}

	SetActiveSkin(InitialSkin);

	eventInitializeSceneClient();
}

/**
 * Assigns the viewport that scenes will use for rendering.
 *
 * @param	inViewport	the viewport to use for rendering scenes
 */
void UUISceneClient::SetRenderViewport( FViewport* SceneViewport )
{
	RenderViewport = SceneViewport;
	if( SceneViewport != NULL )
	{
		if ( GCallbackEvent != NULL )
		{
			GCallbackEvent->Send(CALLBACK_ViewportResized, SceneViewport, 0);
		}
	}
}

/**
 * Changes the active skin to the skin specified
 */
void UUISceneClient::SetActiveSkin( UUISkin* NewActiveSkin )
{
	if ( NewActiveSkin == NULL || GIsRequestingExit )
		return;

	//@todo - perform any cleanup on the existing ActiveSkin
	ActiveSkin = NewActiveSkin;

	OnActiveSkinChanged();
}


/**
 * Set the mouse position to the coordinates specified
 *
 * @param	NewX	the X position to move the mouse cursor to (in pixels)
 * @param	NewY	the Y position to move the mouse cursor to (in pixels)
 */
void UUISceneClient::SetMousePosition( INT NewMouseX, INT NewMouseY )
{
}

/**
 * Changes the resource that is currently being used as the mouse cursor.  Called by widgets as they changes states, or when
 * some action occurs which affects the mouse cursor.
 *
 * @param	CursorName	the name of the mouse cursor resource to use.  Should correspond to a name from the active UISkin's
 *						MouseCursorMap
 *
 * @return	TRUE if the cursor was successfully changed.
 */
UBOOL UUISceneClient::ChangeMouseCursor( FName CursorName )
{
	return FALSE;
}

/**
 * Retrieves the size of the viewport for the scene specified.
 *
 * @param	out_ViewportSize	[out] will be filled in with the width & height that the scene should use as the viewport size
 *
 * @return	TRUE if the viewport size was successfully retrieved
 */
UBOOL UUISceneClient::GetViewportSize( const UUIScene* Scene, FVector2D& out_ViewportSize )
{
	UBOOL bResult = FALSE;

	if ( RenderViewport != NULL )
	{
		out_ViewportSize.X = RenderViewport->GetSizeX();
		out_ViewportSize.Y = RenderViewport->GetSizeY();
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Returns the current local to world screen projection matrix.
 *
 * @param	Widget	if specified, the returned matrix will include the widget's tranformation matrix as well.
 *
 * @return	a matrix which can be used to project 2D pixel coordines into 3D screen coordinates.
 */
FMatrix UUISceneClient::GetCanvasToScreen( const UUIObject* Widget/*=NULL*/ ) const
{
	if ( Widget != NULL )
	{
		return Widget->GenerateTransformMatrix() * CanvasToScreen;
	}

	return CanvasToScreen;
}

/**
 * Returns the inverse of the local to world screen projection matrix.
 *
 * @param	Widget	if specified, the returned matrix will include the widget's tranformation matrix as well.
 *
 * @return	a matrix which can be used to transform normalized device coordinates (i.e. origin at center of screen) into
 *			into 0,0 based pixel coordinates.
 */
FMatrix UUISceneClient::GetInverseCanvasToScreen( const UUIObject* Widget/*=NULL*/ ) const
{
	if ( Widget != NULL )
	{
		return (Widget->GenerateTransformMatrix() * CanvasToScreen).Inverse();
	}

 	return InvCanvasToScreen;
}

/**
 * Initializes the specified scene without opening it.
 *
 * @param	Scene				the scene to initialize;  if the scene specified is contained in a content package, a copy of the scene
 *								will be created, and that scene will be initialized instead.
 * @param	SceneOwner			the player that should be associated with the new scene.  Will be assigned to the scene's
 *								PlayerOwner property.
 * @param	InitializedScene	the scene that was actually initialized.  If Scene is located in a content package, InitializedScene will be
 *								the copy of the scene that was created.  Otherwise, InitializedScene will be the same as the scene passed in.
 *
 * @return	TRUE if the scene was successfully initialized
 */
UBOOL UUISceneClient::InitializeScene( UUIScene* Scene, ULocalPlayer* SceneOwner/*=NULL*/, UUIScene** InitializedScene/*=NULL*/ )
{
	UBOOL bResult = FALSE;
	if ( Scene != NULL )
	{
#ifdef DEBUG_UIPERF
		DOUBLE StartTime = appSeconds();
#endif
		UUIScene* SceneResource = Scene;

		// If this scene was built in the UI editor, instance the scene rather than opening it directly
		if ( SceneResource->GetOutermost() != GetTransientPackage() )
		{
			// generate a unique name based on the scene's SceneTag so that we can easily identify what this scene is based on.
			FName SceneName = MakeUniqueObjectName(GetTransientPackage(), SceneResource->GetClass(), Scene->SceneTag);

			QWORD ClearFlags = RF_RootSet|RF_Standalone|RF_DisregardForGC;
			Scene = Cast<UUIScene>(StaticDuplicateObject(Scene, Scene, GetTransientPackage(), *SceneName.ToString(), (Scene->GetFlags() & ~ClearFlags)|RF_Transient, NULL, TRUE));

			// the Scene's SceneTag will be set to the name of the scene (which is now something like FooScene_2) in UUIScene::PostDuplicate;
			// so change it back to match the scene resource's tag manually
			Scene->SceneTag = SceneResource->SceneTag;

#ifdef DEBUG_UIPERF
			debugf(TEXT("\t%s duplication took (%5.2f ms)"), *Scene->GetTag(), (appSeconds() - StartTime) * 1000);
#endif
		}

		Scene->SceneClient = this;
		Scene->PlayerOwner = SceneOwner;

		Scene->InitializePlayerTracking();
		if ( !Scene->bInitialized )
		{
#ifdef DEBUG_UIPERF
			DOUBLE LocalStartTime = appSeconds();
			Scene->Initialize(NULL);
			debugf(TEXT("\t%s initialize call took (%5.2f ms)"), *Scene->GetTag(), (appSeconds() - LocalStartTime) * 1000);
#else
			Scene->Initialize(NULL);

			Scene->eventPostInitialize();
#endif
		}

		// if the caller needs a reference to the scene copy that was initialized, set that now
		if ( InitializedScene != NULL )
		{
			*InitializedScene = Scene;
		}

		bResult = TRUE;

#ifdef DEBUG_UIPERF
		debugf(TEXT("%s InitializeScene took (%5.2f ms)"), *Scene->GetTag(), (appSeconds() - StartTime) * 1000);
#endif
	}

	return bResult;
}

/**
 * UI implementation of post process mask which allows UI elements to mask post process effects
 **/
class FPostProcessMaskUI : public FPostProcessMaskBase
{
public:

	/**
	 * Constructor
	 **/
	FPostProcessMaskUI()
		:	MaskCanvas(NULL)
		,	bCanvasHasBatchesToRender(FALSE)
	{
	}

	/**
	 * Destructor
	 **/
	virtual ~FPostProcessMaskUI()
	{
		delete MaskCanvas;
	}

	/**
	* @return TRUE if the masking should be rendered
	**/
	virtual UBOOL ShouldRender() const
	{
		return (MaskCanvas && bCanvasHasBatchesToRender) ? TRUE : FALSE;
	}

	/**
	* Begin stencil mask for post process 
	**/
	virtual void BeginStencilMask() const
	{
		if( MaskCanvas &&
			bCanvasHasBatchesToRender )
		{
			// clear stencil to 0
			RHISetRenderTarget(MaskCanvas->GetRenderTarget()->GetRenderTargetSurface(), FSceneDepthTargetProxy().GetDepthTargetSurface());
			RHISetViewport(0,0,0.0f,MaskCanvas->GetRenderTarget()->GetSizeX(),MaskCanvas->GetRenderTarget()->GetSizeY(),1.0f);
			RHIClear(FALSE,FLinearColor::Black,FALSE,0,TRUE,0);			

			// disable color writes
			RHISetColorWriteEnable(FALSE);
			// set stencil write enable to one			
			RHISetStencilState(TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,1>::GetRHI());

			// flush batch from MaskCanvas, note that batched items will persist between flush calls
			// and will get removed when the MaskCanvas is deleted
			MaskCanvas->SetAllowedModes(FCanvas::Allow_Flush);
			MaskCanvas->Flush(TRUE);

			// reenable color writes
			RHISetColorWriteEnable(TRUE);
			// set stencil state to only render to the masked region
			RHISetStencilState(TStaticStencilState<TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,0>::GetRHI());
			// disable alpha blend mode set by batched rendering
			RHISetBlendState(TStaticBlendState<>::GetRHI());
		}

	}

	/**
	* End stencil mask for post process 
	**/
	virtual void EndStencilMask() const
	{
		if( MaskCanvas && 
			bCanvasHasBatchesToRender )
		{
			// reset stencil state
			RHISetStencilState(TStaticStencilState<>::GetRHI());			
		}
	}

	/**
	 * Called during each of the UI scene post process passes to batch any UI elements that need to mask post process
	 *
	 * @param	SceneClient			UI client that is rendering the scene
	 * @param	Canvas				the canvas to use for rendering.
	 * @param	Scene				the UIScene to render
	 * @param	UIPostProcessGroup	Group determines current pp pass that needs to be rendered
	 **/
	void BatchMaskedUIElements( UUISceneClient* SceneClient, FCanvas* Canvas, UUIScene* Scene, EUIPostProcessGroup UIPostProcessGroup )
	{
		// delete existing canvas
		if( MaskCanvas )
		{
			delete MaskCanvas;
		}
		// create a new canvas but disable all the render-specific features, including flushing
		// the canvas is used to batch all masked UI elements and will be rendered for each 
		// post process mask pass.  The batched items in the canvas persist between flush calls
		MaskCanvas = new FCanvas(Canvas->GetRenderTarget(),NULL);
		MaskCanvas->CopyTransformStack(*Canvas);
		MaskCanvas->SetAllowedModes(0);

		// render scene UI elements that need to mask for the current post process pass
		SceneClient->Render_Scene(MaskCanvas,Scene,UIPostProcessGroup);

		// keep track if there is anything in the canvas to render
		bCanvasHasBatchesToRender = MaskCanvas->HasBatchesToRender();
	}

private:
	/** canvas used to batch UI elements */
	FCanvas* MaskCanvas;
	/** TRUE if the MaskCanvas had anything to render */
	UBOOL bCanvasHasBatchesToRender;
};

/**
* Renders the specified scene's post process
*
* @param	Canvas	the canvas to use for rendering.
* @param	Scene	the UIScene to render
* @param	UIPostProcessGroup	Group determines current pp pass that needs to be rendered
*/
void UUISceneClient::Render_Scene_PostProcess( FCanvas* Canvas, UUIScene* Scene, EUIPostProcessGroup UIPostProcessGroup )
{
	check(Canvas);
	check(Scene);

	if( bEnablePostProcess &&
		Scene->bEnableScenePostProcessing &&
		!(GIsEditor && Canvas->GetHitProxyConsumer()) )
	{
		// get the active post process chain based on the group we're rendering
		const UPostProcessChain* PostProcessChain = Scene->GetPostProcessChain(UIPostProcessGroup);
		// current viewport to render into
		const FViewport* Viewport = RenderViewport;

		if( Viewport &&
			PostProcessChain )
		{
			// create mask for only rendering post process to certain regions defined by masked UI elements 
			FPostProcessMaskUI* PostProcessMaskUI = new FPostProcessMaskUI;
			// batch masked UI elements
			PostProcessMaskUI->BatchMaskedUIElements(this, Canvas, Scene, UIPostProcessGroup);
			
			// flush the existing canvas and make sure contents get resolve to scene color texture
			// this is done by swapping render targets and back which triggers a resolve in canvas
			FRenderTarget* SavedRenderTarget = Canvas->GetRenderTarget();
			Canvas->SetRenderTarget(RenderViewport);
			Canvas->SetRenderTarget(SavedRenderTarget);

			const FLOAT TimeSeconds = appSeconds();
			const QWORD PPViewShowFlags = GIsGame ? SHOW_Game|SHOW_PostProcess : SHOW_Editor|SHOW_PostProcess;
			// Create a FSceneViewFamilyContext for rendering the post process
			FSceneViewFamilyContext PostProcessViewFamily(
				Viewport,
				NULL,
				PPViewShowFlags,
				TimeSeconds,
				0.0f,
				TimeSeconds, 
				NULL, 
				TRUE,
				FALSE,
				FALSE,	
				FALSE,	// dont clear scene color between post process passes
				FALSE	// disable final copy to viewport RT as all UI elements will render to scene color
				);

			// Compute the view's screen rectangle.
			INT X = 0;
			INT Y = 0;
			UINT SizeX = Viewport->GetSizeX();
			UINT SizeY = Viewport->GetSizeY();

			// Take screen percentage option into account if percentage != 100.
			GSystemSettings.ScaleScreenCoords(X,Y,SizeX,SizeY);

			FPostProcessSettings* OverrideSettings = NULL;
			Scene->AnimGetCurrentPPSettings(OverrideSettings);

			const FLOAT fFOV = 90.0f;
			// add the new view to the scene
			FSceneView* View = new FSceneView(
				&PostProcessViewFamily,
				NULL,
				-1,
				NULL,
				NULL,
				NULL,
				PostProcessChain,
				OverrideSettings,
				PostProcessMaskUI,
				NULL,
				X,
				Y,
				SizeX,
				SizeY,
				FCanvas::CalcViewMatrix(SizeX,SizeY,fFOV),
				FCanvas::CalcProjectionMatrix(SizeX,SizeY,fFOV,NEAR_CLIPPING_PLANE),
				FLinearColor::Black,
				FLinearColor(0,0,0,0),
				FLinearColor::White,
				TSet<UPrimitiveComponent*>()
				);
			PostProcessViewFamily.Views.AddItem(View);
			// Render the post process pass
			BeginRenderingViewFamily(Canvas,&PostProcessViewFamily);

			// delete the UI post process mask on render thread after pp scene has finished 
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				DeletePostProcessMask,
				FPostProcessMaskUI*,PostProcessMaskUI,PostProcessMaskUI,
			{
				delete PostProcessMaskUI;
			});
		}
	}
}

/**
 * Renders the specified scene and its widgets
 *
 * @param	RI		the interface for drawing to the screen
 * @param	Scene	the scene to render
 * @param	UIPostProcessGroup	Group determines current pp pass that is being rendered
 */
void UUISceneClient::Render_Scene( FCanvas* Canvas, UUIScene* Scene, EUIPostProcessGroup UIPostProcessGroup )
{
	check(Canvas);
	check(Scene);

	const UBOOL bApplyMaskingRegion
		=	Scene->GetSceneRenderMode() == SPLITRENDER_PlayerOwner
		&&	Scene->PlayerOwner != NULL
		&&	UUIInteraction::GetPlayerCount() > 1;

	if ( bApplyMaskingRegion )
	{
		FVector2D ViewportOrigin(EC_EventParm), ViewportSize(EC_EventParm);
		Scene->GetViewportOrigin(ViewportOrigin), Scene->GetViewportSize(ViewportSize);
		Canvas->PushMaskRegion(ViewportOrigin.X, ViewportOrigin.Y, ViewportSize.X, ViewportSize.Y);
	}

	Scene->Render_Scene(Canvas,UIPostProcessGroup);

	if ( bApplyMaskingRegion )
	{
		Canvas->PopMaskRegion();
	}
}

/**
 * Updates 3D primitives for the specified scene and its child widgets.
 *
 * @param	CanvasScene	the scene to use for attaching any 3D primitives
 * @param	Scene		the UIScene to update
 */
void UUISceneClient::Update_ScenePrimitives( FCanvasScene* Canvas, UUIScene* Scene )
{
	check(Canvas);
	check(Scene);

	Scene->UpdateScenePrimitives(Canvas);
}

/**
 * Returns true if the UI scenes should be rendered with post process
 *
 * @return TRUE if post process is enabled for any of the UI scenes
 */
UBOOL UUISceneClient::UsesPostProcess() const
{
	return( bEnablePostProcess && IsUIActive(SCENEFILTER_UsesPostProcessing) );
}

/* ==========================================================================================================
	UGameUISceneClient
========================================================================================================== */
/**
 * @return	the current netmode
 */
BYTE/*ENetMode*/ UGameUISceneClient::GetCurrentNetMode()
{
	return GWorld ? GWorld->GetNetMode() : NM_MAX;
}

/**
 * Handles processing console commands.
 *
 * @param	Cmd		the text that was entered into the console
 * @param	Ar		the archive to use for serializing responses
 *
 * @return	TRUE if the command specified was processed
 */
UBOOL UGameUISceneClient::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	if ( Super::Exec(Cmd,Ar) )
	{
		return TRUE;
	}

	if ( ParseCommand(&Cmd,TEXT("SHOWINPUTHANDLERS")) )
	{
		if ( IsUIActive() )
		{
			UUIScene* TargetScene=NULL;
			if ( !ParseObject<UUIScene>(Cmd, TEXT("SCENE="), TargetScene, ANY_PACKAGE) && IsUIActive(SCENEFILTER_ReceivesFocus) )
			{
				TargetScene = GetActiveScene(NULL, TRUE);
			}

			if ( TargetScene != NULL )
			{
				FName KeyName(NAME_None);
				if ( !Parse(Cmd, TEXT("KEY="), KeyName) )
				{
					FString KeyNameString;
					if ( ParseToken(Cmd,KeyNameString,1) )
					{
						KeyName = *KeyNameString;
					}
				}

				if ( KeyName != NAME_None )
				{
					FInputEventSubscription* SubscribersList=NULL;
					if ( TargetScene->GetInputEventSubscribers(KeyName, 0, &SubscribersList)
						&&	SubscribersList != NULL )
					{
						Ar.Logf(TEXT("%i subscribers found:"), SubscribersList->Subscribers.Num());
						for ( INT HandlerIndex = 0; HandlerIndex < SubscribersList->Subscribers.Num(); HandlerIndex++ )
						{
							Ar.Logf(TEXT("\t%i) %s"), HandlerIndex, *SubscribersList->Subscribers(HandlerIndex)->GetFullName());
						}
					}
					else
					{
						Ar.Logf(TEXT("SHOWINPUTHANDLERS: No subscribers found in scene '%s' for input key %s!"), *TargetScene->GetName(), *KeyName.ToString());
					}
				}
				else
				{
					Ar.Logf(TEXT("SHOWINPUTHANDLERS: No key specified or key not found!  Syntax: ShowInputHandlers SCENE=ScenePathName KEY=KeyName"));
				}
			}
			else
			{
				Ar.Logf(TEXT("SHOWINPUTHANDLERS: No valid scenes found!  Syntax: ShowInputHandlers SCENE=ScenePathName KEY=KeyName"));
			}
		}
		else
		{
			Ar.Logf(TEXT("ShowInputHandlers: No active UI scenes!"));
		}

		return TRUE;
	}
	else if ( ParseCommand(&Cmd,TEXT("ShowUnresolvedPositions")) )
	{
		if ( IsUIActive() )
		{
			UUIScene* TargetScene = NULL;
			if ( !ParseObject<UUIScene>(Cmd, TEXT("SCENE="), TargetScene, ANY_PACKAGE) )
			{
				TargetScene = ActiveScenes.Last();
			}

			TArray<UUIObject*> SceneChildren;
			TargetScene->GetChildren(SceneChildren, TRUE);
			Ar.Logf(TEXT("Checking %s for unresolved positions (%i children)..."), *TargetScene->GetName(), SceneChildren.Num());

			UBOOL bReportedObject = FALSE;
			for ( INT ChildIndex = 0; ChildIndex < SceneChildren.Num(); ChildIndex++ )
			{
				UUIObject* CheckObj = SceneChildren(ChildIndex);

				UBOOL bPositionValid[UIFACE_MAX]		= {1,1,1,1};
				UBOOL bWasResolved[UIFACE_MAX]			= {1,1,1,1};
				UBOOL bMatchedRenderBounds[UIFACE_MAX]	= {1,1,1,1};
				UBOOL bReportObject = FALSE;

				for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
				{
					EUIWidgetFace Face = static_cast<EUIWidgetFace>(FaceIndex);
					bPositionValid[Face] = CheckObj->Position.IsPositionCurrent(CheckObj, Face);
					bWasResolved[Face] = CheckObj->HasPositionBeenResolved(Face);
					bMatchedRenderBounds[Face] = ARE_FLOATS_EQUAL(CheckObj->RenderBounds[Face],CheckObj->Position.GetPositionValue(CheckObj, Face, EVALPOS_PixelViewport));
					bReportObject = bReportObject || !bPositionValid[Face] || !bWasResolved[Face] || !bMatchedRenderBounds[Face];
				}

				if ( bReportObject )
				{
					bReportedObject = TRUE;
					Ar.Logf(TEXT("    Position outdated for (%i) %s:  Position_Current [%i,%i,%i,%i]    Resolved_Correctly [%i,%i,%i,%i]   Position_Matched_RenderBounds [%i,%i,%i,%i]"),
						ChildIndex, *CheckObj->GetName(),
						bPositionValid[0], bPositionValid[1], bPositionValid[2], bPositionValid[3],
						bWasResolved[0], bWasResolved[1], bWasResolved[2], bWasResolved[3],
						bMatchedRenderBounds[0], bMatchedRenderBounds[1], bMatchedRenderBounds[2], bMatchedRenderBounds[3]);
				}
			}

			if ( bReportedObject )
			{
				Ar.Logf(TEXT("***  DONE  ***"));
			}
		}
		else
		{
			Ar.Logf(TEXT("ShowInputHandlers: No active UI scenes!"));
		}
		return TRUE;
	}

	return FALSE;
}

/**
 * Performs any initialization for the UISceneClient.
 *
 * @param	InitialSkin		UISkin that should be set to the initial ActiveSkin
 */
void UGameUISceneClient::InitializeClient( UUISkin* InitialSkin )
{
	Super::InitializeClient(InitialSkin);
}

/**
 * Changes the active skin to the skin specified, initializes the skin and performs all necessary cleanup and callbacks.
 * This method should only be called from script.
 *
 * @param	NewActiveScene	The skin to activate
 *
 * @return	TRUE if the skin was successfully changed.
 */
UBOOL UGameUISceneClient::ChangeActiveSkin( UUISkin* NewActiveSkin )
{
	UBOOL bResult = FALSE;

	// in the game, everything is done in OnActiveSkinChanged, so really nothing to do here...
	UUISkin* PreviouslyActiveSkin = ActiveSkin;
	SetActiveSkin(NewActiveSkin);

	return bResult;
}

/**
 * Refreshes all existing UI elements with the styles from the currently active skin.
 */
void UGameUISceneClient::OnActiveSkinChanged()
{
	checkSlow(DataStoreManager);

	UUIDataStore* StylesDataStore = DataStoreManager->FindDataStore(TEXT("Styles"));
	if ( StylesDataStore != NULL )
	{
		DataStoreManager->UnregisterDataStore(StylesDataStore);
	}

	if ( DataStoreManager->RegisterDataStore(ActiveSkin) )
	{
		// initialize the new skin
		ActiveSkin->Initialize();

		for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
		{
			UUIScene* CurrentScene = ActiveScenes(SceneIndex);
			if ( CurrentScene != NULL && CurrentScene->IsInitialized() )
			{
				CurrentScene->NotifyActiveSkinChanged();
			}
		}
	}
}


/**
 * Called when the UI controller receives a CALLBACK_ViewportResized notification.
 *
 * @param	SceneViewport	the viewport that was resized
 */
void UGameUISceneClient::NotifyViewportResized( FViewport* SceneViewport )
{
	if ( RenderViewport == SceneViewport && RenderViewport != NULL )
	{
		UpdateCanvasToScreen();
		for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
		{
			UUIScene* Scene = ActiveScenes(SceneIndex);
			const FVector2D OriginalSceneViewportSize = Scene->CurrentViewportSize;

			GetViewportSize(Scene, Scene->CurrentViewportSize);
			const FVector2D NewViewportSize = Scene->CurrentViewportSize;
			Scene->NotifyResolutionChanged(OriginalSceneViewportSize, NewViewportSize);

			if ( OBJ_DELEGATE_IS_SET(Scene,NotifyResolutionChanged) )
			{
				Scene->delegateNotifyResolutionChanged(OriginalSceneViewportSize, NewViewportSize);
			}
		}
	}
}

/**
 * Retrieves the point of origin for the viewport for the scene specified.  This should always be 0,0 during the game,
 * but may be different in the UI editor if the editor window is configured to have a gutter around the viewport.
 *
 * @param	out_ViewportOrigin	[out] will be filled in with the position of the starting point of the viewport.
 *
 * @return	TRUE if the viewport origin was successfully retrieved
 */
UBOOL UGameUISceneClient::GetViewportOrigin( const UUIScene* Scene, FVector2D& out_ViewportOrigin )
{
	UBOOL bResult = Super::GetViewportOrigin(Scene,out_ViewportOrigin);
	if ( bResult == TRUE && Scene != NULL && Scene->PlayerOwner != NULL && Scene->GetSceneRenderMode() == SPLITRENDER_PlayerOwner )
	{
		FVector2D TotalViewportSize(0,0);
		if ( Super::GetViewportSize(Scene, TotalViewportSize) )
		{
			out_ViewportOrigin += Scene->PlayerOwner->Origin * TotalViewportSize;
		}
	}

	return bResult;
}

/**
 * Retrieves the size of the viewport for the scene specified.
 *
 * @param	out_ViewportSize	[out] will be filled in with the width & height that the scene should use as the viewport size
 *
 * @return	TRUE if the viewport size was successfully retrieved
 */
UBOOL UGameUISceneClient::GetViewportSize( const UUIScene* Scene, FVector2D& out_ViewportSize )
{
	UBOOL bResult = Super::GetViewportSize(Scene,out_ViewportSize);
	if ( bResult == TRUE && Scene != NULL && Scene->PlayerOwner != NULL && Scene->GetSceneRenderMode() == SPLITRENDER_PlayerOwner )
	{
		out_ViewportSize *= Scene->PlayerOwner->Size;
	}

	return bResult;
}


/**
 * Resets the time and mouse position values used for simulating double-click events to the current value or invalid values.
 */
void UGameUISceneClient::ResetDoubleClickTracking( UBOOL bClearValues )
{
	if ( bClearValues )
	{
		DoubleClickStartTime = INDEX_NONE;
		DoubleClickStartPosition = FIntPoint(-1,-1);
	}
	else
	{
		DoubleClickStartTime = appSeconds();
		DoubleClickStartPosition = MousePosition;
	}
}


/**
 * Checks the current time and mouse position to determine whether a double-click event should be simulated.
 */
UBOOL UGameUISceneClient::ShouldSimulateDoubleClick() const
{
	UUIInteraction* UIController = GetOuterUUIInteraction();
	return	appSeconds() - DoubleClickStartTime < (DOUBLE)UIController->DoubleClickTriggerSeconds
		&&	Abs(MousePosition.X - DoubleClickStartPosition.X) <= UIController->DoubleClickPixelTolerance
		&&	Abs(MousePosition.Y - DoubleClickStartPosition.Y) <= UIController->DoubleClickPixelTolerance;
}

/**
 * Recalculates the matrix used for projecting local coordinates into screen (normalized device)
 * coordinates.  This method should be called anytime the viewport size or origin changes.
 */
void UGameUISceneClient::UpdateCanvasToScreen()
{
	// get the current viewport size
	FVector2D ViewportSize;
	if ( !GetViewportSize(NULL, ViewportSize) )
	{
		ViewportSize.X = UCONST_DEFAULT_SIZE_X;
		ViewportSize.Y = UCONST_DEFAULT_SIZE_Y;
	}

	// local space to world space to normalized device coord.
	CanvasToScreen = FCanvas::CalcBaseTransform3D(
		appTrunc(ViewportSize.X), appTrunc(ViewportSize.Y),
		90.0f, NEAR_CLIPPING_PLANE
		);

	InvCanvasToScreen = CanvasToScreen.Inverse();
}

/**
 * Determines which widget is currently under the mouse cursor by performing hit tests against bounding regions.
 * @todo - need to separate this into two functions; one for hit detection and one for modifying the currently active control
 *
 * @fixme - there is much room for optimization here:
 *	- this doesn't need to done every frame; it could be done only when something happens which could potentially affect the active control
 *  - could eliminate groups of widgets earlier using some sort of spacial partition tree
 */
void UGameUISceneClient::UpdateActiveControl()
{
	UBOOL bHitDetected = FALSE;

	// first, determine which scene the mouse cursor is currently hovering over
	INT ActiveStateIndex=0, PlayerIndex = 0;
	for ( INT SceneIndex = ActiveScenes.Num() - 1; !bHitDetected && SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		PlayerIndex = Max(0, UUIInteraction::GetPlayerIndex(Scene->PlayerOwner));

		if (MousePosition.X >= Scene->GetPosition(UIFACE_Left,EVALPOS_PixelViewport)
		&&	MousePosition.X <= Scene->GetPosition(UIFACE_Right,EVALPOS_PixelViewport)
		&&	MousePosition.Y >= Scene->GetPosition(UIFACE_Top,EVALPOS_PixelViewport)
		&&	MousePosition.Y <= Scene->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport) )
		{
			TArray<UUIObject*>& RenderStack = Scene->RenderStack;

			// the mouse is within this scene's bounds...query the scene for an active control
			for ( INT RenderIndex = RenderStack.Num() - 1; RenderIndex >= 0; RenderIndex-- )
			{
				UUIObject* Widget = RenderStack(RenderIndex);
				if ( Widget != NULL && Widget->IsEnabled(PlayerIndex,FALSE) && Widget->ContainsPoint(MousePosition) )
				{
					// the mouse cursor is within this widget's bounds....attempt to change the state for this widget to the ActiveState
					if ( Widget->IsActive(PlayerIndex,&ActiveStateIndex) )
					{
						// widget is already in the active state - just make sure it's the active control
						if ( Widget != ActiveControl )
						{
							// this should never actually happen....need to figure out if this ever happens
							debugf(NAME_Warning, TEXT("ActiveControl not up to date!  Current: '%s (%s)'  New: '%s (%s)'"),
								ActiveControl ? *ActiveControl->GetWidgetPathName() : TEXT("NULL"),
								ActiveControl && ActiveControl->GetCurrentState() ? *ActiveControl->GetCurrentState()->GetName() : TEXT("NULL"),
								*Widget->GetWidgetPathName(), *Widget->GetCurrentState()->GetName());

							SetActiveControl(Widget);
						}

						Widget->ActivateState(Widget->StateStack(ActiveStateIndex), PlayerIndex);
						bHitDetected = TRUE;
						break;
					}
					else
					{
						// if we already had an ActiveControl, we must first deactivate the "active" state of the currently active control
						if ( ActiveControl != NULL )
						{
							// this condition should never occur, since it means that the widget set as the ActiveControl is not in the active state
							checkf(ActiveControl != Widget || ActiveControl->IsPressed(PlayerIndex),
								TEXT("State inconsistency for scene client's ActiveControl (UIState_Active not in state stack): %s (%s)"),
								*ActiveControl->GetPathName(), *ActiveControl->GetWidgetPathName());

							if ( !ActiveControl->DeactivateStateByClass(UUIState_Active::StaticClass(),PlayerIndex) )
							{
								if ( ActiveControl != Widget )
								{
									// the active control refused to leave the "active" state; this is generally caused by the control
									// "capturing" the mouse, such as when dragging a slider's marker.  Don't update the active control.
									bHitDetected = TRUE;
									break;
								}
							}
						}

						// if bResult is still FALSE, Widget should be put into the Active state, making it our new ActiveControl
						if ( Widget->ActivateStateByClass(UUIState_Active::StaticClass(),PlayerIndex) )
						{
							bHitDetected = TRUE;
							break;
						}
					}
				}
			}
		}

		if ( bRestrictActiveControlToFocusedScene )
		{
			break;
		}
	}

	if ( !bHitDetected && ActiveControl != NULL )
	{
		ActiveControl->DeactivateStateByClass(UUIState_Active::StaticClass(),PlayerIndex);
	}
}


/**
 * Changes this scene client's ActiveControl to the specified value, which might be NULL.
 *
 * @param	NewActiveControl	the widget that should become to ActiveControl, or NULL to clear the ActiveControl.
 *
 * @return	TRUE if the ActiveControl was updated successfully.
 */
UBOOL UGameUISceneClient::SetActiveControl( UUIObject* NewActiveControl )
{
	UBOOL bResult = FALSE;

	// if we have an existing ActiveControl and we're changing it to something else,
	if ( ActiveControl != NULL && ActiveControl != NewActiveControl )
	{
		// kill any active tooltips
		UUIScene* ActiveScene = ActiveControl->GetScene();
		if ( ActiveScene != NULL )
		{
			UUIToolTip* ActiveSceneToolTip = ActiveScene->GetActiveToolTip();
			if ( ActiveSceneToolTip != NULL )
			{
				ActiveSceneToolTip->delegateDeactivateToolTip();
			}
		}
	}

	// if NewActiveControl is NULL, we want to clear the currently ActiveControl.
	if ( NewActiveControl == NULL )
	{
		ActiveControl = NULL;
		bResult = TRUE;
	}
	else
	{
		ActiveControl = NewActiveControl;
		bResult = TRUE;

		// play an optional "mouse over" sound
		ActiveControl->PlayUISound(ActiveControl->MouseEnterCue, TEMP_SPLITSCREEN_INDEX);

		// activate this control's tool-tip timer
		if ( eventCanShowToolTips() )
		{
			UUIScene* ActiveScene = ActiveControl->GetScene();
			if ( ActiveScene != NULL && ActiveScene->GetActiveContextMenu() == NULL )
			{
				ActiveScene->SetActiveToolTip(ActiveControl->ActivateToolTip());
			}
		}
	}

	return bResult;
}

/**
 * Set the mouse position to the coordinates specified
 *
 * @param	NewX	the X position to move the mouse cursor to (in pixels)
 * @param	NewY	the Y position to move the mouse cursor to (in pixels)
 */
void UGameUISceneClient::SetMousePosition( INT NewMouseX, INT NewMouseY )
{
	if ( RenderViewport != NULL )
	{
		RenderViewport->SetMouse(NewMouseX,NewMouseY);
	}

	UpdateMousePosition();

	Super::SetMousePosition(NewMouseX, NewMouseY);
}

/**
 * Sets the values of MouseX & MouseY to the current position of the mouse
 */
void UGameUISceneClient::UpdateMousePosition()
{
	if ( RenderViewport != NULL )
	{
		RenderViewport->GetMousePos(MousePosition);

		// if necessary, notify the ActiveControl that the mouse is hovering on it
		if (ActiveControl != NULL
		&&	ActiveControl->NeedsActiveCursorUpdates())
		{
			FVector2D MousePos(MousePosition.X, MousePosition.Y);
			if ( ActiveControl->ContainsPoint(MousePos) )
			{
				//@todo ronp - need a NotifyMouseEnter and NotifyMouseLeave
				ActiveControl->NotifyMouseOver(MousePos);
			}
		}
	}
}

/**
 * Gets the size (in pixels) of the mouse cursor current in use.
 *
 * @return	TRUE if MouseXL/YL were filled in; FALSE if there is no mouse cursor or if the UI is configured to not render a mouse cursor.
 */
UBOOL UGameUISceneClient::GetCursorSize( FLOAT& MouseXL, FLOAT& MouseYL )
{
	UBOOL bResult = FALSE;

	if ( bRenderCursor && CurrentMouseCursor != NULL )
	{
		// @todo: scaling support
		// @todo: material adjustment support
		CurrentMouseCursor->CalculateExtent(MouseXL, MouseYL);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Changes the resource that is currently being used as the mouse cursor.  Called by widgets as they changes states, or when
 * some action occurs which affects the mouse cursor.
 *
 * @param	CursorName	the name of the mouse cursor resource to use.  Should correspond to a name from the active UISkin's
 *						MouseCursorMap
 *
 * @return	TRUE if the cursor was successfully changed.
 */
UBOOL UGameUISceneClient::ChangeMouseCursor( FName CursorName )
{
	UBOOL bResult = FALSE;

	//@todo - should blank cursors be allowed?
	if ( ActiveSkin != NULL && CursorName != NAME_None )
	{
		UUITexture* NewCursorResource = ActiveSkin->GetCursorResource(CursorName);
		if ( NewCursorResource != NULL )
		{
			CurrentMouseCursor = NewCursorResource;
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Adds the specified scene to the list of active scenes, loading the scene and performing initialization as necessary.
 *
 * @param	Scene			the scene to open; if the scene specified is contained in a content package, a copy of the scene will be created
 *							and the copy will be opened instead.
 * @param	SceneOwner		the player that should be associated with the new scene.  Will be assigned to the scene's
 *							PlayerOwner property.
 * @param	OpenedScene		the scene that was actually opened.  If Scene is located in a content package, OpenedScene will be
 *							the copy of the scene that was created.  Otherwise, OpenedScene will be the same as the scene passed in.
 * @param	ForcedPriority	overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 *
 * @return TRUE if the scene was successfully opened
 */
UBOOL UGameUISceneClient::OpenScene( UUIScene* Scene, ULocalPlayer* SceneOwner/*=NULL*/, UUIScene** OpenedScene/*=NULL*/, BYTE ForcedPriority/*=0*/ )
{
	return InsertScene(ActiveScenes.Num(), Scene, SceneOwner, OpenedScene, NULL, ForcedPriority);
}

/**
 * Instances, initializes, and activates the specified scene, inserting it into the scene stack at the specified location.
 *
 * @param	DesiredInsertIndex	the index [into the ActiveScenes array] to insert the scene.  the scene's SceneStackPriority will take precedence over this value.
 * @param	Scene				the scene to open; if the scene specified is contained in a content package, a copy of the scene will be created
 *								and the copy will be opened instead.
 * @param	SceneOwner			the player that should be associated with the new scene.  Will be assigned to the scene's
 *								PlayerOwner property.
 * @param	OpenedScene			the scene that was actually opened.  If Scene is located in a content package, OpenedScene will be
 *								the copy of the scene that was created.  Otherwise, OpenedScene will be the same as the scene passed in.
 * @param	ActualInsertIndex	receives the location where the scene was actually inserted into the scene stack.
 * @param	ForcedPriority		overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 *
 * @return TRUE if the scene was successfully activated and inserted into the scene stack (although not necessarily at the DesiredSceneIndex)
 */
UBOOL UGameUISceneClient::InsertScene( INT DesiredInsertIndex, UUIScene* Scene, ULocalPlayer* SceneOwner/*=NULL*/, UUIScene** OpenedScene/*=NULL*/, INT* ActualInsertIndex/*=NULL*/, BYTE ForcedPriority/*=0*/ )
{
	UBOOL bResult = FALSE;
#ifdef DEBUG_UIPERF
	DOUBLE StartTime = appSeconds();
#endif

	if ( Scene != NULL )
	{
		// Initially set the opened scene to NULL to make writing over scene references more noticable
		if ( OpenedScene != NULL )
		{
			*OpenedScene = NULL;
		}

		//@todo ronp
		// allow the currently active scene to abort opening of this scene

		//@todo - should I also check for duplicate scene by address?
		UUIScene* ExistingScene = FindSceneByTag(Scene->SceneTag,SceneOwner);
		if ( ExistingScene == NULL )
		{
			if ( InitializeScene(Scene,SceneOwner,&ExistingScene) )
			{
				//@todo ronp - notify all scenes from index => ActiveScenes.Num() that their position in the stack has changed
				// i.e. NotifySceneStackPositionChanged() => ActivateEventByClass(UUIEvent_StackPositionChanged::StaticClass())
				ActivateScene(ExistingScene, DesiredInsertIndex, ForcedPriority);

				// if the caller needs a reference to the scene copy that was activated, set that now
				if ( OpenedScene != NULL )
				{
					*OpenedScene = ExistingScene;
				}

				const INT FinalSceneLocation = FindSceneIndex(ExistingScene);
				if ( ActualInsertIndex != NULL )
				{
					*ActualInsertIndex = FinalSceneLocation;
				}

				bResult = FinalSceneLocation != INDEX_NONE;
			}
		}
		else
		{
			debugf(NAME_Warning, TEXT("InsertScene: Duplicate scene tag (%s) detected: NewScene '%s' ExistingScene '%s'"), *Scene->SceneTag.ToString(), *Scene->GetFullName(), *ExistingScene->GetFullName());
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("InsertScene called with NULL Scene!"));
	}

#if DEBUG_UIPERF
	debugf( TEXT("%s InsertScene took (%5.2f ms)"), *Scene->GetTag(), (appSeconds() - StartTime) * 1000 );
#endif

	return bResult;
}

/**
 * Instances, initializes, and activates a scene, replacing an existing scene's location in the scene stack.  The existing scene will be deactivated and no longer part
 * of the scene stack.  The location in the scene stack for the new scene instance may be modified if its SceneStackPriority requires the scene stack to be resorted.
 *
 * @param	SceneInstanceToReplace	the scene that should be replaced.
 * @param	SceneToOpen				the scene that will replace the existing scene.  If the scene specified is contained in a content package, the scene will be duplicated and
 *									the duplicate will be added instead.
 * @param	SceneOwner				the player that should be associated with the new scene.  Will be assigned to the scene's
 *									PlayerOwner property.
 * @param	OpenedScene				the scene that was actually opened.  If Scene is located in a content package, OpenedScene will be
 *									the copy of the scene that was created.  Otherwise, OpenedScene will be the same as the scene passed in.
 * @param	ForcedPriority			overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 *
 * @return TRUE if the scene was successfully activated and inserted into the scene stack (although not necessarily at the DesiredSceneIndex)
 */
UBOOL UGameUISceneClient::ReplaceScene( UUIScene* SceneInstanceToReplace, UUIScene* SceneToOpen, ULocalPlayer* SceneOwner/*=NULL*/, UUIScene** OpenedScene/*=NULL*/, BYTE ForcedPriority/*=0*/ )
{
	UBOOL bResult = FALSE;

	if ( SceneInstanceToReplace != NULL )
	{
		//@todo ronp - notify all scenes from index => ActiveScenes.Num() that their position in the stack has changed
		// i.e. NotifySceneStackPositionChanged() => ActivateEventByClass(UUIEvent_StackPositionChanged::StaticClass())
		bResult = ReplaceSceneAtIndex(FindSceneIndex(SceneInstanceToReplace), SceneToOpen, SceneOwner, OpenedScene, ForcedPriority);
	}
	else
	{
		debugf(NAME_Warning, TEXT("ReplaceScene called with NULL SceneInstanceToReplace!"));
	}

	return bResult;
}

/**
 * Instances, initializes, and activates a scene, replacing an existing scene's location in the scene stack.  The existing scene will be deactivated and no longer part
 * of the scene stack.  The location in the scene stack for the new scene instance may be modified if its SceneStackPriority requires the scene stack to be resorted.
 *
 * @param	IndexOfSceneToReplace	the index into the stack of scenes for the scene to be replaced.
 * @param	SceneToOpen				the scene that will replace the existing scene.  If the scene specified is contained in a content package, the scene will be duplicated and
 *									the duplicate will be added instead.
 * @param	SceneOwner				the player that should be associated with the new scene.  Will be assigned to the scene's
 *									PlayerOwner property.
 * @param	OpenedScene				the scene that was actually opened.  If Scene is located in a content package, OpenedScene will be
 *									the copy of the scene that was created.  Otherwise, OpenedScene will be the same as the scene passed in.
 * @param	ForcedPriority			overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 *
 * @return TRUE if the scene was successfully activated and inserted into the scene stack (although not necessarily at the DesiredSceneIndex)
 */
UBOOL UGameUISceneClient::ReplaceSceneAtIndex( INT IndexOfSceneToReplace, UUIScene* SceneToOpen, ULocalPlayer* SceneOwner/*=NULL*/, UUIScene** OpenedScene/*=NULL*/, BYTE ForcedPriority/*=0*/ )
{
	UBOOL bResult = FALSE;
#ifdef DEBUG_UIPERF
	DOUBLE StartTime = appSeconds();
#endif

	if ( ActiveScenes.IsValidIndex(IndexOfSceneToReplace) )
	{
		if ( SceneToOpen != NULL )
		{
			// Initially set the opened scene to NULL to make writing over scene references more noticable
			if ( OpenedScene != NULL )
			{
				*OpenedScene = NULL;
			}

			//@todo ronp
			// allow the scene being replaced to abort replacement

			//@todo - should I also check for duplicate scene by address?
			UUIScene* ExistingScene = FindSceneByTag(SceneToOpen->SceneTag,SceneOwner);
			if ( ExistingScene == NULL )
			{
				UUIScene* SceneToReplace = ActiveScenes(IndexOfSceneToReplace);
				//@todo ronp - notify all scenes from index => ActiveScenes.Num() that their position in the stack has changed
				// i.e. NotifySceneStackPositionChanged() => ActivateEventByClass(UUIEvent_StackPositionChanged::StaticClass())
				if ( DeactivateScene(SceneToReplace, FALSE) )
				{
					if ( InitializeScene(SceneToOpen,SceneOwner,&ExistingScene) )
					{
						ActivateScene(ExistingScene, IndexOfSceneToReplace, ForcedPriority);

						// if the caller needs a reference to the scene copy that was activated, set that now
						if ( OpenedScene != NULL )
						{
							*OpenedScene = ExistingScene;
						}

						bResult = ActiveScenes.ContainsItem(ExistingScene);
					}
				}
			}
			else
			{
				debugf(NAME_Warning, TEXT("ReplaceSceneAtIndex: Duplicate scene tag (%s) detected: NewScene '%s' ExistingScene '%s'"), *SceneToOpen->SceneTag.ToString(), *SceneToOpen->GetFullName(), *ExistingScene->GetFullName());
			}
		}
		else
		{
			debugf(NAME_Warning, TEXT("ReplaceSceneAtIndex called with NULL SceneToOpen!"));
		}
	}
	else
	{
		debugf(NAME_Warning, TEXT("ReplaceSceneAtIndex: Invalid scene index specified - %i (%i scenes open)"), IndexOfSceneToReplace, ActiveScenes.Num());
	}

#if DEBUG_UIPERF
	debugf( TEXT("%s ReplaceSceneAtIndex took (%5.2f ms)"), *SceneToOpen->GetTag(), (appSeconds() - StartTime) * 1000 );
#endif

	return bResult;
}

/**
 * Deactivates the specified scene and removes it from the stack of scenes.
 *
 * @param	Scene				the scene to deactivate
 * @param	bCloseChildScenes	normally any scenes which are higher in the stack than the scene being closed are also closed.  Specify
 *								FALSE To override this behavior.
 * @param	bForceCloseImmediately
 *								indicates that the result of calling the scene's OnQueryCloseSceneAllowed delegate should be ignored; used
 *								when closing all scenes as the result of a map change, for example.
 *
 * @return true if the scene was successfully deactivated
 */
UBOOL UGameUISceneClient::CloseScene( UUIScene* Scene, UBOOL bCloseChildScenes/*=TRUE*/, UBOOL bForceCloseImmediately/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	if ( Scene != NULL )
	{
		INT SceneIndex = FindSceneIndexByTag(Scene->SceneTag, Scene->PlayerOwner);
		bResult = CloseSceneAtIndex(SceneIndex, bCloseChildScenes, bForceCloseImmediately);
	}
	else
	{
		debugf(NAME_Warning, TEXT("CloseScene called with NULL Scene!"));
	}

	return bResult;
}

/**
 * Deactivates the scene located at the specified index in the stack of scenes.
 *
 * @param	SceneStackIndex		the index in the stack of scenes for the scene that should be deactivated
 * @param	bCloseChildScenes	normally any scenes which are higher in the stack than the scene being closed are also closed.  Specify
 *								FALSE To override this behavior.
 * @param	bForceCloseImmediately
 *								indicates that the result of calling the scene's OnQueryCloseSceneAllowed delegate should be ignored; used
 *								when closing all scenes as the result of a map change, for example.
 *
 * @return true if the scene was successfully deactivated
 */
UBOOL UGameUISceneClient::CloseSceneAtIndex( INT SceneStackIndex, UBOOL bCloseChildScenes/*=TRUE*/, UBOOL bForceCloseImmediately/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	if ( ActiveScenes.IsValidIndex(SceneStackIndex) )
	{
		UUIScene* SceneToDeactivate = ActiveScenes(SceneStackIndex);
		if ( SceneToDeactivate != NULL )
		{
			if (!OBJ_DELEGATE_IS_SET(SceneToDeactivate,OnQueryCloseSceneAllowed)
			||	SceneToDeactivate->delegateOnQueryCloseSceneAllowed(SceneToDeactivate,bCloseChildScenes,bForceCloseImmediately))
			{
				//@todo ronp - notify all scenes from index => ActiveScenes.Num() that their position in the stack has changed
				// i.e. NotifySceneStackPositionChanged() => ActivateEventByClass(UUIEvent_StackPositionChanged::StaticClass())
				DeactivateScene(SceneToDeactivate, bCloseChildScenes, bForceCloseImmediately);
				bResult = !ActiveScenes.ContainsItem(SceneToDeactivate);
			}
		}
	}

	return bResult;
}

	
/**
 * Determines whether the scene will become the topmost scene and (if necessary) adjusts the desired stack index and requested priority
 * if the requested values aren't compatible with the scene client's current state and active scenes.
 *
 * @param	SceneToActivate			the scene that is being activated
 * @param	DesiredStackIndex		the desired location [into the ActiveScenes array] for the new scene.  A value of INDEX_NONE indicates
 *									that the scene should be added to the top of the stack.  Value will be set to the actual index that the
 *									scene should be inserted.
 * @param	DesiredScenePriority	the priority to use for the new scene.  Any scenes with a SceneStackPriority higher than this value
 *									will remain on top.
 *
 * @return	TRUE if the new scene will become the topmost scene.
 */
UBOOL UGameUISceneClient::ValidateDesiredStackIndex( UUIScene* SceneToActivate, INT& DesiredStackIndex, INT& DesiredStackPriority ) const
{
	// INDEX_NONE means we are putting the scene on top
	if ( DesiredStackIndex == INDEX_NONE )
	{
		DesiredStackIndex = ActiveScenes.Num();
	}
	DesiredStackIndex = Clamp<INT>(DesiredStackIndex, 0, ActiveScenes.Num());
	

	INT ActualStackPriority = DesiredStackPriority;

	// if the scene is going to be to top-most scene, factor in the scene's SceneStackPriority
	INT ActualInsertIndex = DesiredStackIndex;
	if ( ActualInsertIndex == ActiveScenes.Num() )
	{
		ActualStackPriority = DesiredStackPriority > 0 ? DesiredStackPriority : SceneToActivate->SceneStackPriority;
		for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
		{
			UUIScene* SceneToTest = ActiveScenes(SceneIndex);
			if ( SceneToTest != NULL && SceneToTest->SceneStackPriority > ActualStackPriority )
			{
				ActualInsertIndex = SceneIndex;
				break;
			}
		}
	}

	INT TopSceneIndex = ActiveScenes.Num();
	if ( !SceneToActivate->IsNeverFocused() )
	{
		for ( ; TopSceneIndex > ActualInsertIndex; TopSceneIndex-- )
		{
			UUIScene* Scene = ActiveScenes(TopSceneIndex - 1);
			if ( !Scene->IsNeverFocused() )
			{
				break;
			}
		}
		// now TopSceneIndex corresponds to the index to place a scene in order for it to become the topmost scene


		// now, if the new scene has a valid player owner, the scene at TopSceneIndex might not be the "topmost" scene for the new one
		if ( SceneToActivate->PlayerOwner != NULL )
		{
			const INT PlayerIndex = UUIInteraction::GetPlayerIndex(SceneToActivate->PlayerOwner);
			for ( ; TopSceneIndex > 0; TopSceneIndex-- )
			{
				// if the next scene down....
				UUIScene* Scene = ActiveScenes(TopSceneIndex - 1);
				if ( Scene->IsNeverFocused() )
				{
					continue;
				}

				EScreenInputMode SceneInputMode = Scene->GetSceneInputMode();

				// has the same player owner
				if ( Scene->PlayerOwner == SceneToActivate->PlayerOwner

					// or this scene accepts input from all players
					||	SceneInputMode == INPUTMODE_Free

					// or this scene does accept input
					||	(SceneInputMode != INPUTMODE_None

					// and its input mode is not matching-only (which means it isn't relevant to this player)
					&&	SceneInputMode != INPUTMODE_MatchingOnly

					// and the scene doesn't support input from multiple players or it doesn't accept input from this player
					&&	((SceneInputMode != INPUTMODE_Selective && SceneInputMode != INPUTMODE_Simultaneous) || !Scene->AcceptsPlayerInput(PlayerIndex))) )
				{
					// then we've found our scene
					break;
				}
			}
		}
		// now TopSceneIndex represents the minimum index for the new scene in order for it to kill focus on the scene which currently
		// has focus and is accepting input from that player
	}

	const UBOOL bIsBecomingTopmostScene = ActualInsertIndex >= TopSceneIndex;

	debugf(NAME_DevUI, TEXT("UGameUISceneClient::ValidateDesiredStackIndex - SceneToActivate(%s)  Scenes:%i   DesiredStackIndex:%i  ActualInsertIndex:%i  TopSceneIndex:%i   bIsBecomingTopmostScene:%i"),
		*SceneToActivate->GetFullName(SceneToActivate->GetOutermost()), ActiveScenes.Num(),
		DesiredStackIndex, ActualInsertIndex, TopSceneIndex, bIsBecomingTopmostScene);

	DesiredStackIndex = ActualInsertIndex;
	DesiredStackPriority = ActualStackPriority;
	return bIsBecomingTopmostScene;
}

/**
 * Adds the specified scene to the list of active scenes.
 *
 * @param	SceneToActivate		the scene to activate
 * @param	DesiredStackIndex	the location in the list of active scenes to put the new scene.  If INDEX_NONE
 *								is specified, the scene is added to the top of the stack.
 * @param	ForcedPriority		overrides the scene's SceneStackPriority value to allow callers to modify where the scene is placed in the stack.
 */
void UGameUISceneClient::ActivateScene( UUIScene* SceneToActivate, INT DesiredStackIndex/*=INDEX_NONE*/, BYTE ForcedScenePriority/*=0*/ )
{
	if ( SceneToActivate == NULL )
	{
		warnf(TEXT("%s: NULL scene passed to ActivateScene!"), *GetName());
		return;
	}

	// grab the index for the new scene's player
	const INT PlayerIndex = SceneToActivate->PlayerOwner != NULL ? UUIInteraction::GetPlayerIndex(SceneToActivate->PlayerOwner) : 0;

	INT NewScenePriority = ForcedScenePriority;
	const UBOOL bIsBecomingTopmostScene = ValidateDesiredStackIndex(SceneToActivate, DesiredStackIndex, NewScenePriority);
	SceneToActivate->SceneStackPriority = NewScenePriority;

	// If we have existing active scenes, notify the previous top-most scene that a new scene is being activated
	if ( ActiveScenes.Num() > 0 && bIsBecomingTopmostScene && !SceneToActivate->IsNeverFocused() )
	{
		UUIScene* TopmostFocusedScene = GetActiveScene(SceneToActivate->PlayerOwner, TRUE);
		if ( TopmostFocusedScene == NULL )
		{
			TopmostFocusedScene = GetActiveScene(NULL, TRUE);
			if ( TopmostFocusedScene != NULL )
			{
				// if the topmost focused scene has a different player owner, but it's configured to allow input from any player, send it the
				// notification instead
				EScreenInputMode InputMode = TopmostFocusedScene->GetSceneInputMode();
				if ((InputMode != INPUTMODE_Free && InputMode != INPUTMODE_Simultaneous && InputMode != INPUTMODE_Selective)
				||	!TopmostFocusedScene->AcceptsPlayerInput(PlayerIndex))
				{
					TopmostFocusedScene = NULL;
				}
			}
		}
		if ( TopmostFocusedScene != NULL )
		{
			TopmostFocusedScene->NotifyTopSceneChanged(SceneToActivate);

			const INT SupportedPlayerCount = TopmostFocusedScene->GetSupportedPlayerCount();
			const INT OriginalPlayerIndex = TopmostFocusedScene->LastPlayerIndex;

			for ( INT TempPlayerIndex = 0; TempPlayerIndex < SupportedPlayerCount; TempPlayerIndex++ )
			{
				TopmostFocusedScene->LastPlayerIndex = TempPlayerIndex;
				TopmostFocusedScene->KillFocus(NULL, TempPlayerIndex);
			}

			TopmostFocusedScene->LastPlayerIndex = OriginalPlayerIndex;
		}
	}

	debugf(NAME_DevUI, TEXT("%-24s (%s) %-30s %-10i Location:%i  ScenePriority: %d   ForcedScenePriority: %d   FinalPriorityUsed: %d  IsBecomingTopmost:%s >>>>>>"), TEXT(">>>>>> OpenScene"), *SceneToActivate->GetWidgetPathName(),
		*SceneToActivate->GetArchetype()->GetFullName(), PlayerIndex, DesiredStackIndex, SceneToActivate->SceneStackPriority, ForcedScenePriority, NewScenePriority, bIsBecomingTopmostScene ? GTrue : GFalse);
	ActiveScenes.InsertItem(SceneToActivate, DesiredStackIndex);

	SceneToActivate->LastPlayerIndex = PlayerIndex;
	SceneToActivate->Activate();

	//@todo - call a delegate to let the scene know it's being restored, if this is the case...

	if ( SceneToActivate->bUsesPrimitives )
	{
		// re-initialize all primitives...
		//@todo ronp - just set a flag that is checked in UpdateActivePrimitives, which calls InitializePrimitives on any new scenes.
		RequestPrimitiveReinitialization(SceneToActivate);
	}

	if ( bIsBecomingTopmostScene && !SceneToActivate->IsNeverFocused() )
	{
		const INT ActivePlayers = SceneToActivate->GetSupportedPlayerCount();
		if ( ActivePlayers > 0 )
		{
			// initialize focus for all connected players
			for ( INT PlayerIdx = 0; PlayerIdx < ActivePlayers; PlayerIdx++ )
			{
				SceneToActivate->LastPlayerIndex = PlayerIdx;
				if ( SceneToActivate->CanAcceptFocus(PlayerIdx) )
				{
					SceneToActivate->SetFocus(NULL, PlayerIdx);
				}
			}

			SceneToActivate->LastPlayerIndex = PlayerIndex;
		}
		else
		{
			if ( SceneToActivate->CanAcceptFocus(PlayerIndex) )
			{
				SceneToActivate->SetFocus(NULL, PlayerIndex);
			}
		}
	}

	SceneStackModified(PlayerIndex);
	SceneToActivate->OnSceneActivated(TRUE);

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if ( bBlockUpdatesAfterStackModification )
	{
		bDebugResolveScene = bBlockSceneUpdates = TRUE;
		bBlockUpdatesAfterStackModification = FALSE;
	}
#endif
}

/**
 * Removes the specified scene from the list of active scenes.  If this scene is not the top-most scene, all
 * scenes which occur after the specified scene in the ActiveScenes array will be deactivated as well.
 *
 * @param	SceneToDeactivate	the scene to remove
 * @param	bCloseChildScenes	normally any scenes which are higher in the stack than the scene being closed are also closed.  Specify
 *								FALSE To override this behavior.
 * @param	bForceCloseImmediately
 *								indicates that the result of calling the scene's OnQueryCloseSceneAllowed delegate should be ignored; used
 *								when closing all scenes as the result of a map change, for example.
 *
 * @return	TRUE if the scene was successfully removed from the list of active scenes.
 */
UBOOL UGameUISceneClient::DeactivateScene( UUIScene* SceneToDeactivate, UBOOL bCloseChildScenes/*=TRUE*/, UBOOL bForceCloseImmediately/*=FALSE*/ )
{
	// used for detecting re-entrant calls to DeactivateScene
	static INT DeactivationMutex = 0;
	DeactivationMutex++;

	UBOOL bResult = FALSE;

	INT SceneStackLocation = ActiveScenes.FindItemIndex(SceneToDeactivate);
	if ( SceneStackLocation != INDEX_NONE )
	{
		// if this scene is not the top-most, deactivate all scenes which were activated after this one if bCloseChildScenes is TRUE
		INT LastSceneToCloseIndex = bCloseChildScenes ? ActiveScenes.Num() - 1 : SceneStackLocation;
		// see if we need to reset the LastSceneToCloseIndex because of running into a higher priority scene
		for ( INT Idx = SceneStackLocation+1; Idx < ActiveScenes.Num(); Idx++ )
		{
			if ( ActiveScenes.IsValidIndex(Idx) && (ActiveScenes(Idx)->SceneStackPriority > SceneToDeactivate->SceneStackPriority) )
			{
				LastSceneToCloseIndex = Idx - 1;
				break;
			}
		}

		// find the top-most scene that has the same PlayerOwner as the scene being deactivated.
		UUIScene* CurrentlyActiveScene = GetActiveScene(SceneToDeactivate->PlayerOwner, TRUE);
		UUIScene* NextSceneToActivate = NULL;
		INT TopSceneIndex = ActiveScenes.Num() - 1;
		{
			for ( ; TopSceneIndex >= 0; TopSceneIndex-- )
			{
				UUIScene* Scene = ActiveScenes(TopSceneIndex);
				if ( !Scene->IsNeverFocused() )
				{
					break;
				}
			}
			// now TopSceneIndex corresponds to the index to place a scene in order for it to become the topmost scene


			// now, if the new scene has a valid player owner, the scene at TopSceneIndex might not be the "topmost" scene for the new one
			if ( SceneToDeactivate->PlayerOwner != NULL )
			{
				const INT PlayerIndex = UUIInteraction::GetPlayerIndex(SceneToDeactivate->PlayerOwner);
				for ( ; TopSceneIndex >= 0; TopSceneIndex-- )
				{
					// if the next scene down....
					UUIScene* Scene = ActiveScenes(TopSceneIndex);
					if ( Scene != SceneToDeactivate && !Scene->IsNeverFocused() )
					{
						EScreenInputMode SceneInputMode = Scene->GetSceneInputMode();

						// has the same player owner
						if ( Scene->PlayerOwner == SceneToDeactivate->PlayerOwner

							// or this scene accepts input from all players
							||	SceneInputMode == INPUTMODE_Free

							// or it accepts input from multiple players and this player is one of those
							||	((SceneInputMode == INPUTMODE_Selective || SceneInputMode == INPUTMODE_Simultaneous) && Scene->AcceptsPlayerInput(PlayerIndex)) )
						{
							// then we've found our scene
							break;
						}
					}
				}
			}
			else
			{
				for ( ; TopSceneIndex >= 0; TopSceneIndex-- )
				{
					// if the next scene down....
					UUIScene* Scene = ActiveScenes(TopSceneIndex);
					if ( Scene != SceneToDeactivate && !Scene->IsNeverFocused() )
					{
						break;
					}
				}
			}

			// now TopSceneIndex represents the index of the topmost scene that can accept input/focus from the old scene's player owner
			if ( ActiveScenes.IsValidIndex(TopSceneIndex)
			&&	(TopSceneIndex < SceneStackLocation || TopSceneIndex > LastSceneToCloseIndex) )
			{
				NextSceneToActivate = ActiveScenes(TopSceneIndex);
			}
		}

		for ( INT SceneIndex = LastSceneToCloseIndex; SceneIndex >= SceneStackLocation; SceneIndex-- )
		{
			// double-check that SceneIndex is still valid in case calling Scene->Deactivate() results in a
			// recursive call to DeactivateScene which closes scenes below this one in the stack.
			if ( ActiveScenes.IsValidIndex(SceneIndex) )
			{
				UUIScene* Scene = ActiveScenes(SceneIndex);

				// if this scene has is owned by the same player, and it isn't exempt from being closed due to parent scene closing
				// or it is the scene we wanted to close, do it!
				if ( Scene->PlayerOwner == SceneToDeactivate->PlayerOwner
				&&	(!Scene->bExemptFromAutoClose || SceneIndex == SceneStackLocation) )
				{
					debugf(NAME_DevUI, TEXT("%-24s (%s) %-40s <<<<<<<"), TEXT("<<<<<< CloseScene"), *Scene->GetWidgetPathName(), *Scene->GetArchetype()->GetFullName());
					if ( Scene->bUsesPrimitives )
					{
						// if this scene renders 3D primitives, re-initialize all UI primitives so that they are removed
						RequestPrimitiveReinitialization(NULL);
					}

					if (!SceneToDeactivate->IsNeverFocused()
					&&	(NextSceneToActivate == NULL || NextSceneToActivate != Scene) )
					{
						const INT ActivePlayers = Scene->GetSupportedPlayerCount();
						if ( ActivePlayers > 0 )
						{
							const INT OriginalPlayerIndex = UUIInteraction::GetPlayerIndex(SceneToDeactivate->PlayerOwner);

							// initialize focus for all connected players
							for ( INT PlayerIdx = 0; PlayerIdx < ActivePlayers; PlayerIdx++ )
							{
								Scene->LastPlayerIndex = PlayerIdx;
								Scene->KillFocus(NULL, PlayerIdx);
							}

							Scene->LastPlayerIndex = OriginalPlayerIndex;
						}
						else
						{
							Scene->KillFocus(NULL, UUIInteraction::GetPlayerIndex(SceneToDeactivate->PlayerOwner));
						}
					}

					ActiveScenes.Remove(SceneIndex);

					// update the ActiveControl as it might have been owned by the scene we just removed
					UpdateActiveControl();
					Scene->Deactivate();
				}
			}

			bResult = TRUE;
		}

		if ( bResult )
		{
			const INT PlayerIndex = NextSceneToActivate != NULL
				? (NextSceneToActivate->PlayerOwner != NULL ? UUIInteraction::GetPlayerIndex(NextSceneToActivate->PlayerOwner) : 0)
				: (SceneToDeactivate != NULL && SceneToDeactivate->PlayerOwner != NULL ? UUIInteraction::GetPlayerIndex(SceneToDeactivate->PlayerOwner) : 0);

			UBOOL bNotifySceneStackStackChange = TRUE;
			if ( DeactivationMutex == 1 )
			{
				// if there is another scene on the stack, let it know that it is once again the active scene.
				if ( ActiveScenes.Num() > 0 )
				{
					UUIScene* NewActiveScene = GetActiveScene(SceneToDeactivate->PlayerOwner, TRUE);

					// a new scene has become the topmost scene
					const UBOOL bNewTopmostScene = NewActiveScene != NULL && NewActiveScene != CurrentlyActiveScene;
					const UBOOL bReapplyFocus = NextSceneToActivate != NULL;
					if ( bReapplyFocus )
					{
						SceneStackModified(PlayerIndex);
						bNotifySceneStackStackChange = FALSE;

						const INT ActivePlayers = NextSceneToActivate->GetSupportedPlayerCount();
						if ( ActivePlayers > 0 )
						{
							// initialize focus for all connected players
							for ( INT PlayerIdx = 0; PlayerIdx < ActivePlayers; PlayerIdx++ )
							{
								NextSceneToActivate->LastPlayerIndex = PlayerIdx;
								NextSceneToActivate->SetFocus(NULL, PlayerIdx);
							}

							NextSceneToActivate->LastPlayerIndex = PlayerIndex;
						}
						else
						{
							NextSceneToActivate->SetFocus(NULL, PlayerIndex);
						}
						NextSceneToActivate->OnSceneActivated(FALSE);
					}

					debugf(NAME_DevUI, TEXT("DeactivateScene (%s) - NewActiveScene(%s)  NextSceneToActivate(%s)  bNewTopmostScene:%i   bReapplyFocus:%i"),
						*SceneToDeactivate->GetName(), *NewActiveScene->GetName(), *NextSceneToActivate->GetName(), bNewTopmostScene, bReapplyFocus);
				}
			}

			if ( bNotifySceneStackStackChange )
			{
				SceneStackModified(PlayerIndex);
			}
		}
	}

	DeactivationMutex--;

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if ( bBlockUpdatesAfterStackModification )
	{
		bDebugResolveScene = bBlockSceneUpdates = TRUE;
		bBlockUpdatesAfterStackModification = FALSE;
	}
#endif
	return bResult;
}

/**
 * Called whenever a scene is added or removed from the list of active scenes.  Calls any functions that handle updating the
 * status of various tracking variables, such as whether the UI is currently capable of processing input.
 *
 * @param	PlayerIndex		the index of the player that owns the scene that was just added or removed, or 0 if the scene didn't have
 *							a player owner.
 */
void UGameUISceneClient::SceneStackModified( INT PlayerIndex )
{
	RequestCursorRenderUpdate();
	RequestInputProcessingUpdate();
	UpdatePausedState(Clamp(PlayerIndex, 0, GEngine->GamePlayers.Num() - 1));

	if ( bRenderDebugInfo && DebugTarget != NULL )
	{
		UUIScene* DebugTargetScene = DebugTarget->GetScene();
		if ( DebugTargetScene != NULL && !ActiveScenes.ContainsItem(DebugTargetScene) )
		{
			DebugTarget = NULL;
		}
	}
}

/**
 * Determines whether the any active scenes process axis input.
 *
 * @param	bProcessAxisInput	receives the flags for whether axis input is needed for each player.
 */
void UGameUISceneClient::CheckAxisInputSupport( UBOOL* bProcessAxisInput[UCONST_MAX_SUPPORTED_GAMEPADS] ) const
{
	//@todo - generate these dynamically as opposed to hardcoding them
	TLookupMap<FName> NavAliases;
 	//NavAliases.AddItem(UIKEY_NextControl);
 	//NavAliases.AddItem(UIKEY_PrevControl);
	NavAliases.AddItem(UIKEY_NavFocusUp);
	NavAliases.AddItem(UIKEY_NavFocusDown);
	NavAliases.AddItem(UIKEY_NavFocusLeft);
	NavAliases.AddItem(UIKEY_NavFocusRight);

	TLookupMap<FName> AxisInputKeys;
	AxisInputKeys.AddItem(KEY_Gamepad_LeftStick_Up);
	AxisInputKeys.AddItem(KEY_Gamepad_LeftStick_Down);
	AxisInputKeys.AddItem(KEY_Gamepad_LeftStick_Right);
	AxisInputKeys.AddItem(KEY_Gamepad_LeftStick_Left);
	AxisInputKeys.AddItem(KEY_Gamepad_RightStick_Up);
	AxisInputKeys.AddItem(KEY_Gamepad_RightStick_Down);
	AxisInputKeys.AddItem(KEY_Gamepad_RightStick_Right);
	AxisInputKeys.AddItem(KEY_Gamepad_RightStick_Left);
	AxisInputKeys.AddItem(KEY_SIXAXIS_AccelX);
	AxisInputKeys.AddItem(KEY_SIXAXIS_AccelY);
	AxisInputKeys.AddItem(KEY_SIXAXIS_AccelZ);
	AxisInputKeys.AddItem(KEY_SIXAXIS_Gyro);
	AxisInputKeys.AddItem(KEY_XboxTypeS_LeftX);
	AxisInputKeys.AddItem(KEY_XboxTypeS_LeftY);
	AxisInputKeys.AddItem(KEY_XboxTypeS_RightX);
	AxisInputKeys.AddItem(KEY_XboxTypeS_RightY);

	UBOOL bAllPlayersSupported = FALSE;
	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene->CheckAxisInputSupport(bProcessAxisInput, NavAliases, AxisInputKeys) )
		{
			bAllPlayersSupported = TRUE;
			break;
		}
	}

// 	debugf(TEXT("%s::CheckAxisInputSupport  bAllPlayersSupported:%i     [%i,%i,%i,%i]"), *GetName(), bAllPlayersSupported, *bProcessAxisInput[0], *bProcessAxisInput[1], *bProcessAxisInput[2], *bProcessAxisInput[3]);
}

/**
 * Updates the value of UIInteraction.bProcessingInput to reflect whether any scenes are capable of processing input.
 */
void UGameUISceneClient::UpdateInputProcessingStatus()
{
	UBOOL bProcessAxisInput[UCONST_MAX_SUPPORTED_GAMEPADS] = { FALSE, FALSE, FALSE, FALSE };
	UBOOL* pProcessAxisInput[UCONST_MAX_SUPPORTED_GAMEPADS] = { &bProcessAxisInput[0], &bProcessAxisInput[1], &bProcessAxisInput[2], &bProcessAxisInput[3] };
	CheckAxisInputSupport(pProcessAxisInput);

	UBOOL bUIProcessesInput = FALSE;
	for ( INT Idx = 0; Idx < UCONST_MAX_SUPPORTED_GAMEPADS; Idx++ )
	{
		if ( bProcessAxisInput[Idx] )
		{
			bUIProcessesInput = TRUE;
			break;
		}
	}

	UBOOL bShouldFlushPlayerInput = FALSE;
	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( !bUIProcessesInput )
		{
			EScreenInputMode SceneInputMode = Scene->GetSceneInputMode();
			if ( SceneInputMode != INPUTMODE_None )
			{
				bUIProcessesInput = TRUE;
			}
		}

		if ( Scene->bFlushPlayerInput )
		{
			bShouldFlushPlayerInput = TRUE;
		}
	}

	UUIInteraction* UIController = GetOuterUUIInteraction();

	// enable/disable the axis emulation for all players
	for ( INT PlayerIndex = 0; PlayerIndex < UCONST_MAX_SUPPORTED_GAMEPADS; PlayerIndex++ )
	{
		UIController->AxisInputEmulation[PlayerIndex].EnableAxisEmulation(bProcessAxisInput[PlayerIndex]);
	}

	const UBOOL bCurrentlyProcessingInput = UIController->bProcessInput;
	UIController->bProcessInput = bUIProcessesInput || (bEnableDebugInput && bRenderDebugInfo);

	if ( bShouldFlushPlayerInput && bUIProcessesInput && !bCurrentlyProcessingInput )
	{
		FlushPlayerInput();
	}
}

/**
 * Searches all scenes to determine if any are configured to display a cursor.  Sets the value of bRenderCursor accordingly.
 */
void UGameUISceneClient::UpdateCursorRenderStatus()
{
	UBOOL bMouseAvailable = FALSE;

	// make sure that we have a mouse available
	if ( GEngine != NULL && GEngine->GameViewport != NULL )
	{
		for ( INT PlayerIndex = 0; PlayerIndex < GEngine->GamePlayers.Num(); PlayerIndex++ )
		{
			ULocalPlayer* LP = GEngine->GamePlayers(PlayerIndex);
			if ( LP != NULL )
			{
				if ( GEngine->GameViewport->IsMouseAvailable(LP->ControllerId) )
				{
					bMouseAvailable = TRUE;
					break;
				}
			}
		}
	}

	if ( bMouseAvailable )
	{
		// we always want to display the cursor if we're in the UI is in debug mode
		UBOOL bDisplayCursor = bEnableDebugInput && bRenderDebugInfo;
		if ( ActiveScenes.Num() > 0 )
		{
			UUIScene* ActiveScene = ActiveScenes(ActiveScenes.Num()-1);
			if (ActiveScene && ActiveScene->bDisplayCursor && ActiveScene->IsVisible())
			{
				bDisplayCursor = TRUE;
			}
		}

		// bRenderCursor is used by the scene client to indicate whether it should render the cursor
		bRenderCursor = bDisplayCursor;
		if ( bRenderCursor && CurrentMouseCursor == NULL )
		{
			ChangeMouseCursor(TEXT("Arrow"));
		}
	}
	else
	{
		bRenderCursor = FALSE;
	}
}

/**
 * Ensures that the game's paused state is appropriate considering the state of the UI.  If any scenes are active which require
 * the game to be paused, pauses the game...otherwise, unpauses the game.
 *
 * @param	PlayerIndex		the index of the player that owns the scene that was just added or removed, or 0 if the scene didn't have
 *							a player owner.
 */
void UGameUISceneClient::UpdatePausedState( INT PlayerIndex )
{
	eventPauseGame(IsUIActive(SCENEFILTER_PausersOnly), PlayerIndex);
}

/**
 * Callback which allows the UI to prevent unpausing if scenes which require pausing are still active.
 * @see PlayerController.SetPause
 */
UBOOL UGameUISceneClient::CanUnpauseInternalUI()
{
	return !IsUIActive(SCENEFILTER_PausersOnly);
}

/**
 * Clears the arrays of pressed keys for all local players in the game; used when the UI begins processing input.  Also
 * updates the InitialPressedKeys maps for all players.
 */
void UGameUISceneClient::FlushPlayerInput()
{
	for ( INT PlayerIndex = 0; PlayerIndex < GEngine->GamePlayers.Num(); PlayerIndex++ )
	{
		ULocalPlayer* Player = GEngine->GamePlayers(PlayerIndex);
		if ( Player != NULL && Player->Actor != NULL && Player->Actor->PlayerInput != NULL )
		{
			//@todo ronp - in some cases, we only want to do this for the player that opened the scene

			// record each key that was pressed when the UI began processing input so we can ignore the released event that will be generated when that key is released
			TArray<FName>* PressedPlayerKeys = InitialPressedKeys.Find(Player->ControllerId);
			if ( PressedPlayerKeys == NULL )
			{
				PressedPlayerKeys = &InitialPressedKeys.Set(Player->ControllerId, TArray<FName>());
			}

			if ( PressedPlayerKeys != NULL )
			{
				for ( INT KeyIndex = 0; KeyIndex < Player->Actor->PlayerInput->PressedKeys.Num(); KeyIndex++ )
				{
					FName Key = Player->Actor->PlayerInput->PressedKeys(KeyIndex);
					PressedPlayerKeys->AddUniqueItem(Key);
				}
			}

			// sending the IE_Released events should have cleared the pressed keys array, but clear them manually
			Player->Actor->PlayerInput->ResetInput();
		}
	}
}

/**
 * Get a reference to the transient scene, which is used to contain transient widgets that are created by unrealscript
 *
 * @return	pointer to the UIScene that owns transient widgets
 */
UUIScene* UGameUISceneClient::GetTransientScene() const
{
	UUIScene* TransientScene = FindSceneByTag(NAME_Transient);
	check(TransientScene);

	return TransientScene;
}

void UGameUISceneClient::execCreateScene( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass,SceneClass);
	P_GET_NAME_OPTX(SceneTag,NAME_None);
	P_GET_OBJECT_OPTX(UUIScene,SceneTemplate,NULL);
	P_FINISH;

	if ( SceneClass == NULL )
	{
		debugf(NAME_Warning, TEXT("CreateScene called with NULL SceneClass - defaulting to UIScene"));
		SceneClass = UUIScene::StaticClass();
	}

	if ( SceneTemplate == NULL || !SceneClass->IsChildOf(SceneTemplate->GetClass()) )
	{
		SceneTemplate = SceneClass->GetDefaultObject<UUIScene>();
	}
	*(UUIScene**)Result=CreateScene(SceneTemplate, GetTransientPackage(), SceneTag, SceneClass);
}

/**
 * Creates a new instance of the scene class specified.
 *
 * @param	SceneTemplate	the template to use for the new scene
 * @param	InOuter			the outer for the scene
 * @param	SceneTag		if specified, the scene will be given this tag when created
 * @param	SceneClass		the class to use for creating the new scene; if not specified, uses the SceneTemplate's class
 *
 * @return	a UIScene instance of the class specified
 */
UUIScene* UGameUISceneClient::CreateScene( UUIScene* SceneTemplate, UObject* InOuter, FName SceneTag/*=NAME_None*/, UClass* SceneClass/*=NULL*/ )
{
	UUIScene* CreatedScene = NULL;
	check(SceneTemplate!=NULL);

	if ( SceneTag == NAME_None )
	{
		SceneTag = SceneTemplate->SceneTag;
	}
	if ( SceneClass == NULL || !SceneClass->IsChildOf(SceneTemplate->GetClass()) )
	{
		SceneClass = SceneTemplate->GetClass();
	}

	debugf(NAME_DevUI, TEXT("Creating scene - class (%s)  template (%s)  tag (%s)"),
		*SceneClass->GetName(), *SceneTemplate->GetFullName(), *SceneTag.ToString());

	// generate a unique name based on the scene's SceneTag so that we can easily identify what this scene is based on.
	FName SceneName = UObject::MakeUniqueObjectName(InOuter, SceneTemplate->GetClass(), SceneTag);

	FObjectDuplicationParameters Parameters(SceneTemplate, GetTransientPackage());
	Parameters.DestName = SceneName;
	Parameters.DestClass = SceneClass;
	Parameters.FlagMask = ~(RF_RootSet|RF_Standalone|RF_DisregardForGC|RF_ArchetypeObject);
	Parameters.ApplyFlags = RF_Transient;
	Parameters.bMigrateArchetypes = TRUE;

	CreatedScene = Cast<UUIScene>(StaticDuplicateObjectEx(Parameters));

	// the Scene's SceneTag will be set to the name of the scene (which is now something like FooScene_2) in UUIScene::PostDuplicate;
	// so change it back to match the scene resource's tag manually
	CreatedScene->SceneTag = SceneTag;

#ifdef DEBUG_UIPERF
	debugf(TEXT("\t%s duplication took (%5.2f ms)"), *Scene->GetTag(), (appSeconds() - StartTime) * 1000);
#endif

	CreatedScene->SceneClient = this;
	CreatedScene->Created(NULL);
	return CreatedScene;
}

/**
 * Creates the scene that will be used to contain transient widgets that are created in unrealscript
 */
void UGameUISceneClient::CreateTransientScene()
{
	// make sure the transient scene doesn't already exist in the list of scenes
	UUIScene* TransientScene = FindSceneByTag(NAME_Transient);
	if ( TransientScene == NULL )
	{
		TransientScene = ConstructObject<UUIScene>(UUIScene::StaticClass(), UObject::GetTransientPackage(), NAME_None, RF_Transient);
		TransientScene->SceneClient = this;
		TransientScene->bDisplayCursor = FALSE;
		TransientScene->SceneTag = NAME_Transient;
		TransientScene->SetSceneInputMode(INPUTMODE_None);
		TransientScene->bPauseGameWhileActive = FALSE;

		TransientScene->Created(NULL);
		TransientScene->InitializePlayerTracking();
		TransientScene->Initialize(NULL);

		// remove the focused state from the transient scene's list of available states.
		//@todo - encapsulate this into a function
		for ( INT StateIndex = TransientScene->InactiveStates.Num() - 1; StateIndex >= 0; StateIndex-- )
		{
			UUIState_Focused* FocusedState = Cast<UUIState_Focused>(TransientScene->InactiveStates(StateIndex));
			if ( FocusedState != NULL )
			{
				TransientScene->InactiveStates.Remove(StateIndex);
			}
		}

		ActivateScene(TransientScene);
	}
}


/**
 * Create a temporary widget for presenting data from unrealscript
 *
 * @param	WidgetClass		the widget class to create
 *
 * @return	a pointer to a fully initialized widget of the class specified, contained within the transient scene
 * @todo - add support for metacasting using a property flag (i.e. like spawn auto-casts the result to the appropriate type)
 */
UUIObject* UGameUISceneClient::CreateTransientWidget( UClass* WidgetClass, FName WidgetTag, UUIObject* Owner /*=NULL*/ )
{
	UUIScene* TransientScene = GetTransientScene();
	UUIObject* Result = TransientScene->CreateWidget(TransientScene, WidgetClass);
	if ( Result != NULL )
	{
		Result->WidgetTag = WidgetTag;
		if ( Owner != NULL && TransientScene->FindChild(Owner->WidgetTag, TRUE) != NULL )
		{
			Owner->InsertChild(Result);
		}
		else
		{
			TransientScene->InsertChild(Result);
		}
	}
	return Result;
}

/**
 * Searches through the ActiveScenes array for a UIScene with the tag specified
 *
 * @param	SceneTag	the name of the scene to locate
 *
 * @return	pointer to the UIScene that has a SceneTag matching SceneTag, or NULL if no scenes in the ActiveScenes
 *			stack have that name
 */
UUIScene* UGameUISceneClient::FindSceneByTag( FName SceneTag, ULocalPlayer* SceneOwner/*=NULL*/ ) const
{
	UUIScene* Result = NULL;

	INT SceneIndex = FindSceneIndexByTag(SceneTag,SceneOwner);
	if ( ActiveScenes.IsValidIndex(SceneIndex) )
	{
		Result = ActiveScenes(SceneIndex);
	}

	return Result;
}

/**
 * Searches through the ActiveScenes array for a UIScene with the tag specified
 *
 * @param	SceneToFind	the scene to locate
 *
 * @return	index [into the ActiveScenes array] for the scene specified, or INDEX_NONE
 *			if it isn't found.
 */
INT UGameUISceneClient::FindSceneIndex( const UUIScene* SceneToFind ) const
{
	return ((TArrayNoInit<const UUIScene*>&)ActiveScenes).FindItemIndex(SceneToFind);
}

/**
 * Searches through the ActiveScenes array for a UIScene with the tag specified
 *
 * @param	SceneTag	the name of the scene to locate
 *
 * @return	index [into the ActiveScenes array] for the scene specified, or INDEX_NONE
 *			if it isn't found.
 */
INT UGameUISceneClient::FindSceneIndexByTag( FName SceneTag, ULocalPlayer* SceneOwner/*=NULL*/ ) const
{
	INT Result = INDEX_NONE;

	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene->SceneTag == SceneTag )
		{
			if ( SceneOwner == NULL || Scene->PlayerOwner == SceneOwner )
			{
				Result = SceneIndex;
				break;
			}
		}
	}

	return Result;
}

/**
 * Accessor for getting a reference to a scene at a given index
 *
 * @param	SceneIndex	should be an index into the ActiveScenes array for the scene to get a reference to.
 *
 * @return	the scene instance at the specified location in the ActiveScenes array, or None or the index was invalid.
 */
UUIScene* UGameUISceneClient::GetSceneAtIndex( INT SceneIndex ) const
{
	UUIScene* Result = NULL;

	if ( ActiveScenes.IsValidIndex(SceneIndex) )
	{
		Result = ActiveScenes(SceneIndex);
	}

	return Result;
}

/**
 * Accessor for getting the number of currently active scenes.
 *
 * @param	MatchingPlayerOwner		if specified, only scenes with this PlayerOwner will be counted.
 * @param	bIgnoreUnfocusedScenes	specify TRUE to skip scenes which cannot accept focus (i.e. bNeverFocus = true)
 *
 * @return	the number of active scenes which meet the criteria specified by the input parameters.
 */
INT UGameUISceneClient::GetActiveSceneCount( ULocalPlayer* MatchingPlayerOwner/*=NULL*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	INT Result = 0;

	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene != NULL )
		{
			if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
			{
				continue;
			}

			if ( MatchingPlayerOwner == NULL || Scene->PlayerOwner == MatchingPlayerOwner )
			{
				Result++;
			}
		}
	}

	return Result;
}

/**
 * Wrapper for getting a reference to the currently active scene.
 *
 * @param	MatchingPlayerOwner		if specified, the top-most scene with this player as its PlayerOwner will be returned.
 * @param	bIgnoreUnfocusedScenes	specify TRUE to only return the top-most scenes that can accept focus.
 *
 * @return	a reference to the top-most scene in the scene stack which meets the conditions of the input parameters.
 */
UUIScene* UGameUISceneClient::GetActiveScene( ULocalPlayer* MatchingPlayerOwner/*=NULL*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	UUIScene* Result = NULL;

	for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene != NULL )
		{
			if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
			{
				continue;
			}

			if ( MatchingPlayerOwner == NULL || Scene->PlayerOwner == MatchingPlayerOwner )
			{
				Result = Scene;
				break;
			}
		}
	}

	return Result;
}

/**
 * Accessor for getting a reference to a scene's "parent" scene.
 *
 * @param	SourceScene						the scene to find a parent scene for
 * @param	bRequireMatchingPlayerOwner		indicates whether the returned scene must be a scene opened by the same player as SourceScene.
 * @param	bIgnoreUnfocusedScenes			indicates that scenes which cannot accept focus should be skipped.
 *
 * @return	a reference to the next scene below SourceScene in the ActiveScenes array which meets the criteria specified by the input
 *			parameters, or None if there isn't one.
 */
UUIScene* UGameUISceneClient::GetPreviousScene( const UUIScene* SourceScene, UBOOL bRequireMatchingPlayerOwner/*=TRUE*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	UUIScene* Result = NULL;

	if ( SourceScene != NULL )
	{
		INT CurrentSceneIndex = FindSceneIndex(SourceScene);
		if ( CurrentSceneIndex != INDEX_NONE )
		{
			Result = GetPreviousSceneFromIndex(CurrentSceneIndex, bRequireMatchingPlayerOwner ? SourceScene->PlayerOwner : NULL, bIgnoreUnfocusedScenes);
		}
		else
		{
			const UUIScene* CurrentScene = SourceScene;

			// CurrentSceneIndex is INDEX_NONE if this scene hasn't been fully initialized yet
			for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
			{
				UUIScene* Scene = ActiveScenes(SceneIndex);
				if ( Scene != NULL )
				{
					if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
					{
						continue;
					}

					if ( !bRequireMatchingPlayerOwner || CurrentScene->PlayerOwner == NULL || CurrentScene->PlayerOwner == Scene->PlayerOwner )
					{
						Result = Scene;
						break;
					}
				}
			}
		}

	}
	else
	{
		debugf(TEXT("UGameUISceneClient::GetPreviousScene - NULL scene specified."));
	}

	return Result;
}

/**
 * Accessor for getting a reference to a scene's "parent" scene based on the scene's index in the ActiveScenes array.
 *
 * @param	StartingSceneIndex		the index into the ActiveScenes array of the scene to find a parent scene for
 * @param	MatchingPlayerOwner		if specified, only scenes owned by this player will be considered.
 * @param	bIgnoreUnfocusedScenes	indicates that scenes which cannot accept focus should be skipped.
 *
 * @return	a reference to the next scene below the source scene in the ActiveScenes array which meets the criteria specified by the input
 *			parameters, or None if there isn't one.
 */
UUIScene* UGameUISceneClient::GetPreviousSceneFromIndex( INT StartingSceneIndex, ULocalPlayer* MatchingPlayerOwner/*=NULL*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	UUIScene* Result = NULL;

	for ( INT SceneIndex = StartingSceneIndex - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene != NULL )
		{
			if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
			{
				continue;
			}

			if ( MatchingPlayerOwner == NULL || Scene->PlayerOwner == MatchingPlayerOwner )
			{
				Result = Scene;
				break;
			}
		}
	}

	return Result;
}

/**
 * Accessor for getting a reference to the first parent scene of SourceScene, that can process input for SourceScene's PlayerOwner.
 *
 * @param	SourceScene						the scene to find a parent scene for
 * @param	bIgnoreUnfocusedScenes			indicates that scenes which cannot accept focus should be skipped.
 *
 * @return	a reference to the next scene below SourceScene in the ActiveScenes array which meets the criteria specified by the input
 *			parameters, or None if there isn't one.
 */
UUIScene* UGameUISceneClient::GetPreviousInputProcessingScene( const UUIScene* SourceScene, UBOOL bIgnoreUnfocusedScenes/*=TRUE*/ ) const
{
	UUIScene* Result = NULL;

	if ( SourceScene != NULL )
	{
		INT CurrentSceneIndex = FindSceneIndex(SourceScene);
		if ( CurrentSceneIndex == INDEX_NONE )
		{
			// CurrentSceneIndex is INDEX_NONE if this scene hasn't been fully initialized yet
			CurrentSceneIndex = ActiveScenes.Num() - 1;
		}
		else
		{
			CurrentSceneIndex--;
		}

		INT PlayerIndex = UUIInteraction::GetPlayerIndex(SourceScene->PlayerOwner);

		// CurrentSceneIndex is INDEX_NONE if this scene hasn't been fully initialized yet
		for ( INT SceneIndex = CurrentSceneIndex; SceneIndex >= 0; SceneIndex-- )
		{
			UUIScene* Scene = ActiveScenes(SceneIndex);
			if ( Scene != NULL )
			{
				if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
				{
					continue;
				}

				EScreenInputMode SceneInputMode = Scene->GetSceneInputMode();

				// has the same player owner
				if ( Scene->PlayerOwner == SourceScene->PlayerOwner

				// or its input mode isn't matchingonly and
				||	(SceneInputMode != INPUTMODE_MatchingOnly

				// it isn't selective or it doesn't accept input from this player
				&&	(SceneInputMode != INPUTMODE_Selective || !Scene->AcceptsPlayerInput(PlayerIndex))) )
				{
					//@fixme - currently, this method will return scenes which do not process input....
					Result = Scene;
					break;
				}
			}
		}
	}
	else
	{
		debugf(TEXT("UGameUISceneClient::GetPreviousInputProcessingScene - NULL scene specified."));
	}

	return Result;
}


/**
 * Accessor for getting a reference to a scene's "child" scene.
 *
 * @param	SourceScene						the scene to find a child scene for
 * @param	bRequireMatchingPlayerOwner		indicates whether the returned scene must be a scene opened by the same player as SourceScene.
 * @param	bIgnoreUnfocusedScenes			indicates that scenes which cannot accept focus should be skipped.
 *
 * @return	a reference to the next scene above SourceScene in the ActiveScenes array which meets the criteria specified by the input
 *			parameters, or None if there isn't one.
 */
UUIScene* UGameUISceneClient::GetNextScene( const UUIScene* SourceScene, UBOOL bRequireMatchingPlayerOwner/*=TRUE*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	UUIScene* Result = NULL;

	if ( SourceScene != NULL )
	{
		INT CurrentSceneIndex = FindSceneIndex(SourceScene);
		if ( CurrentSceneIndex != INDEX_NONE )
		{
			Result = GetNextSceneFromIndex(CurrentSceneIndex, bRequireMatchingPlayerOwner ? SourceScene->PlayerOwner : NULL, bIgnoreUnfocusedScenes);
		}
		else
		{
			// CurrentSceneIndex is INDEX_NONE if this scene hasn't been fully initialized yet - in this case, there is no next scene
#if 0
			UUIScene* CurrentScene = SourceScene;

			for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
			{
				UUIScene* Scene = ActiveScenes(SceneIndex);
				if ( Scene != NULL )
				{
					if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
					{
						continue;
					}

					if ( !bRequireMatchingPlayerOwner || CurrentScene->PlayerOwner == NULL || CurrentScene->PlayerOwner == Scene->PlayerOwner )
					{
						Result = Scene;
						break;
					}
				}
			}
#endif
		}
	}
	else
	{
		debugf(TEXT("UGameUISceneClient::GetPreviousScene - NULL scene specified."));
	}

	return Result;
}

/**
 * Accessor for getting a reference to a scene's "child" scene.
 *
 * @param	StartingSceneIndex		the scene to find a child scene for
 * @param	MatchingPlayerOwner		if specified, only scenes owned by this player will be considered.
 * @param	bIgnoreUnfocusedScenes	indicates that scenes which cannot accept focus should be skipped.
 *
 * @return	a reference to the next scene above the source scene in the ActiveScenes array which meets the criteria specified by the input
 *			parameters, or None if there isn't one.
 */
UUIScene* UGameUISceneClient::GetNextSceneFromIndex( INT StartingSceneIndex, ULocalPlayer* MatchingPlayerOwner/*=NULL*/, UBOOL bIgnoreUnfocusedScenes/*=FALSE*/ ) const
{
	UUIScene* Result = NULL;

	if ( StartingSceneIndex + 1 >= 0 )
	{
		for ( INT SceneIndex = StartingSceneIndex + 1; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
		{
			UUIScene* Scene = ActiveScenes(SceneIndex);
			if ( Scene != NULL )
			{
				if ( bIgnoreUnfocusedScenes && Scene->IsNeverFocused() )
				{
					continue;
				}

				if ( MatchingPlayerOwner == NULL || Scene->PlayerOwner == MatchingPlayerOwner )
				{
					Result = Scene;
					break;
				}
			}
		}
	}

	return Result;
}

/**
 * Iterates over all scenes in the active scenes array.
 *
 * @param	SceneClass			only scenes derived from this class will be returned.
 * @param	OutScene			receives the value of each scene in the ActiveScenes array which meets the criteria of the input parameters.
 * @param	bIterateBackwards	indicates that the scenes should be returned in reverse order.
 * @param	StartingIndex		allows callers to specify a specific location into the ActiveScenes array to start the iteration.
 * @param	SceneFilterMask		allows callers to control the types of scenes allowed.  The value should be a bitmask of the SCENEFILTER_
 *								const values defined in UISceneClient.
 */
void UGameUISceneClient::execAllActiveScenes( FFrame& Stack, RESULT_DECL )
{
	// Parse parms from stack
	P_GET_OBJECT(UClass,SceneBaseClass);
	P_GET_OBJECT_REF(UUIScene,OutScene);
	P_GET_UBOOL_OPTX(bIterateBackwards,FALSE);
	P_GET_INT_OPTX(StartingIndex,INDEX_NONE);
	P_GET_INT_OPTX(SceneFilterMask,SCENEFILTER_Any);
	P_FINISH;

	if ( SceneBaseClass == NULL && OutScene != NULL )
	{
		SceneBaseClass = OutScene->GetClass();
	}

	if ( SceneBaseClass != NULL )
	{
		checkSlow(SceneBaseClass->IsChildOf(UUIScene::StaticClass()));

		INT IndexStart, IndexDelta;
		if ( bIterateBackwards )
		{
			IndexStart = StartingIndex == INDEX_NONE ? ActiveScenes.Num() - 1 : StartingIndex;
			IndexDelta = -1;
		}
		else
		{
			IndexStart = StartingIndex == INDEX_NONE ? 0 : StartingIndex;
			IndexDelta = 1;
		}

		INT CurrentIndex = IndexStart;

		// make a copy so that any script modifications to the scene stack (such as closing or opening scenes) don't affect us
		TArray<UUIScene*> ActiveScenesCopy = ActiveScenes;

		PRE_ITERATOR
			OutScene = NULL;
			while ( ActiveScenesCopy.IsValidIndex(CurrentIndex) && OutScene == NULL )
			{
				UUIScene* Scene = ActiveScenesCopy(CurrentIndex);
				CurrentIndex += IndexDelta;

				if (Scene != NULL && Scene->IsA(SceneBaseClass)
				&&	SceneMatchesFilter(SceneFilterMask, Scene))
				{
					OutScene = Scene;
				}
			}

			if ( OutScene == NULL )
			{
				Stack.Code = &Stack.Node->Script(wEndOffset + 1);
				break;
			}
		POST_ITERATOR
	}
	else
	{
		Stack.Logf(NAME_ScriptWarning, TEXT("Invalid value specified for SceneBaseClass in call to AllActiveScenes iterator, and the desired class could not be deduced using an existing value for the 'Scene' parameter.  Skipping iterator call."));
		
		// advance the execution position past the iterator code; after the following lines, the code pointer should be at the first bytecode after the iterator block
		const INT wEndOffset = Stack.ReadWord();
		Stack.Code = &Stack.Node->Script(wEndOffset + 1);
	}
}

/**
 * Triggers a call to UpdateInputProcessingStatus on the next Tick().
 */
void UGameUISceneClient::RequestInputProcessingUpdate()
{
	bUpdateInputProcessingStatus = TRUE;
}

void UGameUISceneClient::RequestCursorRenderUpdate()
{
	bUpdateCursorRenderStatus = TRUE;
}

/**
 * Called once a frame to update the UI's state.
 *
 * @param	DeltaTime - The time since the last frame.
 */
void UGameUISceneClient::Tick(FLOAT DeltaTime)
{
	// Update the cached delta time
	LatestDeltaTime = DeltaTime;

	if ( bUpdateInputProcessingStatus == TRUE )
	{
		bUpdateInputProcessingStatus = FALSE;
		UpdateInputProcessingStatus();
	}

	if( bUpdateCursorRenderStatus == TRUE )
	{
		bUpdateCursorRenderStatus = FALSE;
		UpdateCursorRenderStatus();
	}

	if ( bUpdateSceneViewportSizes && RenderViewport != NULL )
	{
		bUpdateSceneViewportSizes = FALSE;

		// LayoutPlayers isn't called until viewports are redrawn, which is after Tick is called....which means that the split-screen
		// configuration might be out of date.  Force the players to update their viewport origins and sizes now!
		GetOuterUUIInteraction()->GetOuterUGameViewportClient()->eventLayoutPlayers();
		GCallbackEvent->Send(CALLBACK_ViewportResized, RenderViewport, 0);
	}

	// update the current mouse position
	UpdateMousePosition();

	// Update all scene tooltips.  This must be done BEFORE updating the active control, otherwise
	// newly activated tooltips will skip a Tick.
	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);

		// if we have an active tooltip, tick it
		UUIToolTip* SceneToolTip = Scene->GetActiveToolTip();
		if ( SceneToolTip != NULL )
		{
			SceneToolTip->TickToolTip(DeltaTime);
		}

	}
	// update the active control
	if ( bRenderCursor )
	{
		UpdateActiveControl();
	}

	{
		SCOPE_CYCLE_COUNTER(STAT_UIKismetTime);

		// update the sequences for all scenes
		for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
		{
			UUIScene* Scene = ActiveScenes(SceneIndex);
			Scene->TickSequence(DeltaTime);
		}
	}

	// now tick all the scenes
	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		SCOPE_CYCLE_COUNTER(STAT_UISceneTickTime);

		UUIScene* Scene = ActiveScenes(SceneIndex);
		Scene->Tick(DeltaTime);
	}

#if !SHIPPING_PC_GAME && !FINAL_RELEASE
	if ( bDebugResolveScene )
	{
		bBlockSceneUpdates = TRUE;
	}
#endif
}

/**
 * Gets the list of scenes that should be rendered.  Some active scenes might not be rendered if scenes later in the
 * scene stack prevent it, for example.
 */
void UGameUISceneClient::GetScenesToRender( TArray<UUIScene*>& ScenesToRender )
{
	// tracks whether scene rendering is enabled for each player as well as global scenes
	TArray<UBOOL> bSceneRenderingEnabled;
	bSceneRenderingEnabled.AddZeroed(GEngine->GamePlayers.Num() + 1);
	for ( INT AllowRenderIndex = 0; AllowRenderIndex < bSceneRenderingEnabled.Num(); AllowRenderIndex++ )
	{
		bSceneRenderingEnabled(AllowRenderIndex) = TRUE;
	}

	// when a scene is configured to hide parent scenes, normally this would only hide parent scenes that have the same
	// PlayerOwner; we'll also turn off rendering for global scenes if a player specific scene is configured to hide parent scenes
	UBOOL& bRenderGlobalScenes = bSceneRenderingEnabled.Last();

	// figure out exactly which scenes we'll be rendering based on the settings of each scene's bRenderParentScenes/bAlwaysRenderScene values
	for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);

		// since we are handling the translation of the available rendering region ourselves (in GetViewportOrigin/GetViewortSize), there is no
		// need to render only the scenes associated with a particular player in a separate pass
		if ( Scene != NULL /*&& Scene->PlayerOwner == CurrentPlayer*/ )
		{
			// find the entry for the flag which indicates whether a previously rendered scene wants to hide its parent scenes
			INT AllowRenderIndex = GEngine->GamePlayers.FindItemIndex(Scene->PlayerOwner);
			if ( AllowRenderIndex == INDEX_NONE )
			{
				AllowRenderIndex = bSceneRenderingEnabled.Num() - 1;
			}

			UBOOL& bPlayerSceneRenderingEnabled = bSceneRenderingEnabled(AllowRenderIndex);
			UBOOL bRenderScene = bPlayerSceneRenderingEnabled;

			// if scene rendering is enabled for this player's scenes, but the scene is configured to render fullscreen, we also
			// need to verify that global scene rendering is enabled
			if ( bRenderScene == TRUE && Scene->GetSceneRenderMode() == SPLITRENDER_Fullscreen )
			{
				bRenderScene = bRenderGlobalScenes;
			}

			// if we haven't encountered a scene that disables parent scene rendering yet, or this scene is configured to always be rendered,
			// add this scene to the list of scenes that we'll render
			if ( bRenderScene == TRUE || Scene->bAlwaysRenderScene )
			{
				ScenesToRender.InsertItem(Scene, 0);
			}

			if ( !Scene->ShouldRenderParentScenes() )
			{
				// if a player specific scene disables rendering, disable rendering of global scenes as well, but still allow other player-specific
				// scenes to be rendered (unless they're configured to render fullscreen)
				bPlayerSceneRenderingEnabled = bRenderGlobalScenes = FALSE;
			}
		}
	}
}

/**
 * Re-initializes all primitives in the specified scene.  Will occur on the next tick.
 *
 * @param	Sender	the scene to re-initialize primitives for.
 */
void UGameUISceneClient::RequestPrimitiveReinitialization( UUIScene* Sender )
{
	//@todo ronp - add support for only re-initializing prims for the scene that needs it.  currently we re-initialize everything
	GetOuterUUIInteraction()->bIsUIPrimitiveSceneInitialized = FALSE;
}

/**
 * Gives all UIScenes a chance to create, attach, and/or initialize any primitives contained in the UIScene.
 *
 * @param	CanvasScene		the scene to use for attaching any 3D primitives
 */
void UGameUISceneClient::InitializePrimitives( FCanvasScene* CanvasScene )
{
	check(CanvasScene);
	for ( INT SceneIndex = 0; SceneIndex < ActiveScenes.Num(); SceneIndex++ )
	{
		ActiveScenes(SceneIndex)->InitializePrimitives(CanvasScene);
	}
}

/**
 * Updates 3D primitives for all active scenes
 *
 * @param	CanvasScene		the scene to use for attaching any 3D primitives
 */
void UGameUISceneClient::UpdateActivePrimitives( FCanvasScene* CanvasScene )
{
	TArray<UUIScene*> ScenesToUpdate;
	GetScenesToRender(ScenesToUpdate);

	for ( INT SceneIndex = 0; SceneIndex < ScenesToUpdate.Num(); SceneIndex++ )
	{
		UUIScene* Scene = ScenesToUpdate(SceneIndex);
		if ( Scene->IsVisible() )
		{
			Update_ScenePrimitives(CanvasScene, Scene);
		}
	}
}


/**
 * Renders all active scenes.
 */
void UGameUISceneClient::RenderScenes( FCanvas* Canvas )
{
	TArray<UUIScene*> ScenesToRender;
	GetScenesToRender(ScenesToRender);
	if ( ScenesToRender.Num() > 0 )
	{
		UUIInteraction* UIController = GetOuterUUIInteraction();
		// detect active ui scene with primitives
		UBOOL bUISceneHasPrimitives = UIController->UsesUIPrimitiveScene() && UIController->GetUIPrimitiveScene() && UIController->GetUIPrimitiveScene()->GetNumPrimitives() > 0;

		// Set 3D Projection
		Canvas->SetBaseTransform(CanvasToScreen);

		// Setup the alpha modulation stack
		TArray<FLOAT> AlphaModulationStack;
		TArray<FLOAT> CurrentAlphaModulation;
		for ( INT PlayerIndex = 0; PlayerIndex < UUIInteraction::GetPlayerCount(); PlayerIndex++ )
		{
			CurrentAlphaModulation.AddItem(1.f);
		}

		for ( INT SceneIndex = ScenesToRender.Num() - 1; SceneIndex >= 0; SceneIndex-- )
		{
			UUIScene* Scene = ScenesToRender(SceneIndex);
			INT PlayerIndex = 0;
			if ( Scene->PlayerOwner != NULL )
			{
				PlayerIndex = UUIInteraction::GetPlayerIndex(Scene->PlayerOwner);
			}

			AlphaModulationStack.InsertItem(Scene->bAlwaysRenderScene ? 1.f : CurrentAlphaModulation(PlayerIndex), 0);

			FLOAT AlphaModulationPercent = OverlaySceneAlphaModulation;
			if ( Scene->ShouldModulateBackgroundAlpha(AlphaModulationPercent) )
			{
				CurrentAlphaModulation(PlayerIndex) *= AlphaModulationPercent;
			}
		}

		for ( INT SceneIndex = 0; SceneIndex < ScenesToRender.Num(); SceneIndex++ )
		{
			UUIScene* Scene = ScenesToRender(SceneIndex);
			if ( Scene->IsVisible() )
			{
				SCOPE_CYCLE_COUNTER(STAT_UISceneRenderTime);

				// render background ui post process pass
				Render_Scene_PostProcess(Canvas, Scene, UIPostProcess_Background);

				// turn on depth testing against the primitive ui scene if depth testing is enabled on the scene
				Canvas->SetDepthTestingEnabled(bUISceneHasPrimitives && Scene->bEnableSceneDepthTesting);

				// store the current global alpha modulation
				const FLOAT OriginalAlphaModulation = Canvas->AlphaModulate;

				// apply the scene's opacity to the global alpha modulation
				Canvas->AlphaModulate *= AlphaModulationStack(SceneIndex);

				Render_Scene(Canvas, Scene, UIPostProcess_None);

				// restore the previous global alpha modulation
				Canvas->AlphaModulate = OriginalAlphaModulation;

				// restore depth test to disabled
				Canvas->SetDepthTestingEnabled(FALSE);

				// render foreground ui post process pass
				Render_Scene_PostProcess(Canvas, Scene, UIPostProcess_Foreground);
			}
		}

		// Restore 2D Projection
		Canvas->SetBaseTransform(FCanvas::CalcBaseTransform2D(RenderViewport->GetSizeX(),
			RenderViewport->GetSizeY()));

		// render info about the focused and active controls.
		if ( bRenderDebugInfo )
		{
			RenderDebugInfo(Canvas);
		}

		UGameViewportClient* GameViewportClient = GetOuterUUIInteraction()->GetOuterUGameViewportClient();
		UBOOL bHasFocus = (GameViewportClient && GameViewportClient->Viewport) ? GameViewportClient->IsFocused( GameViewportClient->Viewport ) : TRUE;

		// Only render the game cursor when drawing the global viewport, and if it has focus.
		UBOOL bShouldRenderGameCursor = bRenderCursor && CurrentMouseCursor != NULL && bHasFocus;

		// OnShowUIMouseCursor() lets the game engine know that the UI software mouse is being rendered (prevents MouseLock)
		GameViewportClient->OnShowUIMouseCursor( bShouldRenderGameCursor );

		// Don't render the game cursor if the mouse is over the title bar (MousePosition.Y is less than zero)
		if ( bShouldRenderGameCursor && MousePosition.Y >= 0 )
		{
			// @todo: scaling support
			// @todo: material adjustment support
			FRenderParameters Parameters(RenderViewport ? RenderViewport->GetSizeY() : 0.f);
			Parameters.DrawX = MousePosition.X;
			Parameters.DrawY = MousePosition.Y;

			// get the size of the cursor material; by default, we'll render the cursor at full size...
			// might want to add additional logic here in the future to scale the cursor, etc.
			CurrentMouseCursor->CalculateExtent(Parameters.DrawXL, Parameters.DrawYL);

			Canvas->PushDepthSortKey(-100);
			CurrentMouseCursor->Render_Texture(Canvas, Parameters);
			Canvas->PopDepthSortKey();
		}
	}
}

#if CONSOLE
#define SAVE_CONFIG
#else
#define SAVE_CONFIG SaveConfig()
#endif

#define LOG_ACTION_RESULT(var)	debugf(TEXT("UIDEBUG: Value of %s is now: %s"), TEXT(#var), var ? GTrue : GFalse)

/**
 * Process an input event which interacts with the in-game scene debugging overlays
 *
 * @param	Key		the key that was pressed
 * @param	Event	the type of event received
 *
 * @return	TRUE if the input event was processed; FALSE otherwise.
 */
UBOOL UGameUISceneClient::DebugInputKey( FName Key, EInputEvent Event )
{
	UBOOL bResult = FALSE;
	if ( bInteractiveMode || IsCtrlDown(RenderViewport) )
	{
		UUIScene* ActiveScene = NULL;

		// first, find the topmost visible scene
		for ( INT SceneIdx = ActiveScenes.Num() - 1; SceneIdx >= 0; SceneIdx-- )
		{
			UUIScene* Scene = ActiveScenes(SceneIdx);
			if ( !Scene->IsNeverFocused()
			&&	(!bSelectVisibleTargetsOnly || Scene->IsVisible(TRUE)) )
			{
				ActiveScene = Scene;
				if ( Key != KEY_LeftMouseButton || ActiveScene->ContainsPoint(MousePosition) )
				{
					break;
				}
			}
		}

		if ( ActiveScene != NULL )
		{
			const INT PlayerIndex = Max(0, UUIInteraction::GetPlayerIndex(ActiveScene->PlayerOwner));
			if (Key == KEY_Tab
			||	Key == KEY_LeftMouseButton
			||	Key == KEY_XboxTypeS_RightShoulder 
			||	Key == KEY_XboxTypeS_LeftShoulder )
			{
				if ( Event == IE_Released )
				{
					TArray<UUIObject*> SceneChildren;
					ActiveScene->GetChildren(SceneChildren, TRUE);

					if ( DebugTarget == NULL )
					{
						DebugTarget = ActiveScene;
					}
					else if ( DebugTarget == ActiveScene )
					{
						if ( SceneChildren.Num() > 0 )
						{
							if ( (Key == KEY_Tab && IsShiftDown(RenderViewport)) || Key == KEY_XboxTypeS_LeftShoulder )
							{
								DebugTarget = SceneChildren.Last();
							}
							else
							{
								DebugTarget = SceneChildren(0);
							}
						}
						else
						{
							DebugTarget = NULL;
						}
					}
					else
					{
						UUIScreenObject* NewDebugTarget = NULL;
						INT Idx = SceneChildren.FindItemIndex(Cast<UUIObject>(DebugTarget));
						if ( SceneChildren.IsValidIndex(Idx) )
						{
							INT StartIdx, EndIdx, IncrementCount;
							if ( Key == KEY_XboxTypeS_LeftShoulder || (Key == KEY_Tab && IsShiftDown(RenderViewport)) )
							{
								// going backwards through the list
								StartIdx = Idx - 1;
								EndIdx = -1;
								IncrementCount = -1;
							}
							else
							{
								StartIdx = Idx+1;
								EndIdx = SceneChildren.Num();
								IncrementCount = 1;
							}

							for ( INT ChildIdx = StartIdx; ChildIdx != EndIdx; ChildIdx += IncrementCount )
							{
								UBOOL bEligibleWidget = TRUE;
								if ( bSelectVisibleTargetsOnly )
								{
									for ( UUIScreenObject* CheckObj = SceneChildren(ChildIdx); CheckObj; CheckObj = CheckObj->GetParent() )
									{
										if ( !CheckObj->IsVisible() )
										{
											bEligibleWidget = FALSE;
											break;
										}
									}
								}

								if ( bEligibleWidget )
								{
									if ( Key == KEY_Tab || SceneChildren(ChildIdx)->ContainsPoint(FVector2D(MousePosition.X,MousePosition.Y)) )
									{
										NewDebugTarget = SceneChildren(ChildIdx);
										break;
									}
								}
							}
						}

						DebugTarget = NewDebugTarget;
					}
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_MouseScrollDown || Key == KEY_XboxTypeS_LeftTrigger )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					DebugTarget = DebugTarget->GetParent();
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_MouseScrollUp || Key == KEY_XboxTypeS_RightTrigger )
			{
				if ( Event == IE_Released )
				{
					if ( DebugTarget != NULL )
					{
						UUIObject* FocusedControl = DebugTarget->GetFocusedControl(FALSE, PlayerIndex);
						if ( FocusedControl != NULL )
						{
							DebugTarget = FocusedControl;
						}
					}
					else
					{
						DebugTarget = ActiveScene;
					}
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Left || Key == KEY_XboxTypeS_DPad_Left )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					FLOAT PosA = DebugTarget->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) - NumPixels;
					FLOAT PosB = DebugTarget->GetPosition(UIFACE_Right, EVALPOS_PixelViewport) - NumPixels;

					DebugTarget->SetPosition(PosA, UIFACE_Left, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(PosB, UIFACE_Right, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Up || Key == KEY_XboxTypeS_DPad_Up )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					FLOAT PosA = DebugTarget->GetPosition(UIFACE_Top, EVALPOS_PixelViewport) - NumPixels;
					FLOAT PosB = DebugTarget->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) - NumPixels;

					DebugTarget->SetPosition(PosA, UIFACE_Top, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(PosB, UIFACE_Bottom, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Right || Key == KEY_XboxTypeS_DPad_Right )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					FLOAT PosA = DebugTarget->GetPosition(UIFACE_Left, EVALPOS_PixelViewport) + NumPixels;
					FLOAT PosB = DebugTarget->GetPosition(UIFACE_Right, EVALPOS_PixelViewport) + NumPixels;

					DebugTarget->SetPosition(PosA, UIFACE_Left, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(PosB, UIFACE_Right, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Down || Key == KEY_XboxTypeS_DPad_Down )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					FLOAT PosA = DebugTarget->GetPosition(UIFACE_Top, EVALPOS_PixelViewport) + NumPixels;
					FLOAT PosB = DebugTarget->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) + NumPixels;

					DebugTarget->SetPosition(PosA, UIFACE_Top, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(PosB, UIFACE_Bottom, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Add || Key == KEY_Gamepad_RightStick_Up )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					const FLOAT CurrentPos = DebugTarget->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(CurrentPos + NumPixels, UIFACE_Bottom, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Subtract || Key == KEY_Gamepad_RightStick_Down )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					const FLOAT CurrentPos = DebugTarget->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(CurrentPos - NumPixels, UIFACE_Bottom, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Multiply || Key == KEY_Gamepad_RightStick_Right )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					const FLOAT CurrentPos = DebugTarget->GetPosition(UIFACE_Right, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(CurrentPos + NumPixels, UIFACE_Right, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_Divide || Key == KEY_Gamepad_RightStick_Left )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					const INT NumPixels = IsAltDown(RenderViewport) ? 10 : 1;
					const FLOAT CurrentPos = DebugTarget->GetPosition(UIFACE_Right, EVALPOS_PixelViewport);
					DebugTarget->SetPosition(CurrentPos - NumPixels, UIFACE_Right, EVALPOS_PixelViewport);
				}
				bResult = TRUE;
			}

			// the rest of the cases just toggle switches that modify how the debug overlay is rendered
			else if ( Key == KEY_A )
			{
				// toggle whether active control info is shown
				if ( Event == IE_Released )
				{
					bRenderActiveControlInfo = !bRenderActiveControlInfo;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bRenderActiveControlInfo);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_B || Key == KEY_XboxTypeS_LeftThumbstick )
			{
				if ( Event == IE_Released )
				{
					UUIObject* FocusedControl = ActiveScene->GetFocusedControl(TRUE, PlayerIndex);
					if ( FocusedControl != NULL )
					{
						DebugTarget = FocusedControl;
					}
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_C )
			{
				// toggle whether mouse position is shown
				if ( Event == IE_Released )
				{
					bShowMousePos = !bShowMousePos;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bShowMousePos);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_D )
			{
				// toggle debug mode
				if ( IsAltDown(RenderViewport) && IsCtrlDown(RenderViewport) )
				{
					if ( Event == IE_Released )
					{
						bRenderDebugInfo = !bRenderDebugInfo;
						if ( !bRenderDebugInfo )
						{
							DebugTarget = NULL;
						}
						UpdateCursorRenderStatus();

						SAVE_CONFIG;
						LOG_ACTION_RESULT(bRenderDebugInfo);
					}
					bResult = TRUE;
				}
			}
#if !CONSOLE
			else if ( Key == KEY_E )
			{
				extern UBOOL GUsewxWindows;
				if( GUsewxWindows )
				{
					// show a property window for the targeted widget
					if ( Event == IE_Released && DebugTarget != NULL )
					{
						debugf(TEXT("Attempting to open property window for %s"), *DebugTarget->GetPathName());
						if ( GEngine->GamePlayers.Num() > 0 && GEngine->GamePlayers(0)->Actor != NULL )
						{
							APlayerController* PC = GEngine->GamePlayers(0)->Actor;
							PC->ConsoleCommand(FString::Printf(TEXT("EDITOBJECT %s"), *DebugTarget->GetPathName()));
						}
						else
						{
							for ( TObjectIterator<AActor> ActorIt; ActorIt; ++ActorIt )
							{
								ActorIt->ConsoleCommand(FString::Printf(TEXT("EDITOBJECT %s"), *DebugTarget->GetPathName()));
								break;
							}
						}
					}
				}
				else
				{
					warnf(TEXT("You must pass -WXWINDOWS on the command-line when starting the game to enable property windows!"));
				}
				bResult = TRUE;
			}
#endif
			else if ( Key == KEY_F )
			{
				// toggle whether focused control info is shown
				if ( Event == IE_Released )
				{
					bRenderFocusedControlInfo = !bRenderFocusedControlInfo;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bRenderFocusedControlInfo);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_I || Key == KEY_XboxTypeS_RightThumbstick )
			{
				// toggle interactive mode (eliminates need to hold Ctrl)
				if ( Event == IE_Released )
				{
					bInteractiveMode = !bInteractiveMode;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bInteractiveMode);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_J )
			{
				if ( Event == IE_Released )
				{
					bBlockSceneUpdates = FALSE;
					LOG_ACTION_RESULT(bBlockSceneUpdates);
				}

				bResult = TRUE;
			}
			else if ( Key == KEY_K )
			{
				if ( Event == IE_Released )
				{
					bBlockUpdatesAfterStackModification = TRUE;
					LOG_ACTION_RESULT(bBlockUpdatesAfterStackModification);
				}

				bResult = TRUE;
			}
			else if ( Key == KEY_L )
			{
				if ( Event == IE_Released )
				{
					bBlockSceneUpdates = bDebugResolveScene = !bDebugResolveScene;
					LOG_ACTION_RESULT(bDebugResolveScene);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_M )
			{
				// toggle whether current state is shown for focused,active,target controls
				if ( Event == IE_Released )
				{
					bShowCurrentState = !bShowCurrentState;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bShowCurrentState);
				}
			}
			else if ( Key == KEY_P )
			{
				// toggle whether full path or just name/tag is shown
				if ( Event == IE_Released )
				{
					bDisplayFullPaths = !bDisplayFullPaths;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bDisplayFullPaths);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_R )
			{
				// toggle whether render bounds are shown
				if ( Event == IE_Released )
				{
					bShowRenderBounds = !bShowRenderBounds;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bShowRenderBounds);
				}
			}
			else if ( Key == KEY_S )
			{
				// switch the location of the debug info
				if ( Event == IE_Released )
				{
					bRenderDebugInfoAtTop = !bRenderDebugInfoAtTop;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bRenderDebugInfoAtTop);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_T )
			{
				// toggle whether target info is shown
				if ( Event == IE_Released )
				{
					bRenderTargetControlInfo = !bRenderTargetControlInfo;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bRenderTargetControlInfo);
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_U )
			{
				if ( Event == IE_Released && DebugTarget != NULL )
				{
					debugf(TEXT("Attempting to dump properties for %s"), *DebugTarget->GetPathName());
					UObject::StaticExec( *FString::Printf(TEXT("OBJ DUMP %s"), *DebugTarget->GetPathName()) );
				}
				bResult = TRUE;
			}
			else if ( Key == KEY_V )
			{
				// toggle whether Ctrl+Tab or Ctrl+LMB will visit widgets that are hidden
				if ( Event == IE_Released )
				{
					bSelectVisibleTargetsOnly = !bSelectVisibleTargetsOnly;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bSelectVisibleTargetsOnly);
				}
			}
			else if ( Key == KEY_W )
			{
				// toggles whether the widtet's path name or widget path are shown
				if ( Event == IE_Released )
				{
					bShowWidgetPath = !bShowWidgetPath;
					SAVE_CONFIG;
					LOG_ACTION_RESULT(bShowWidgetPath);
				}
			}
			else if ( Key == KEY_Z )
			{
				if ( Event == IE_Released )
				{
					if ( ActiveScene != NULL )
					{
						TArray<UUIObject*> SceneChildren = ActiveScene->GetChildren(TRUE);
						for ( INT ChildIndex = 0; ChildIndex < SceneChildren.Num(); ChildIndex++ )
						{
							for ( BYTE FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
							{
								SceneChildren(ChildIndex)->InvalidatePosition(FaceIndex);
							}
						}
						ActiveScene->RequestSceneUpdate(TRUE, TRUE, TRUE, TRUE);
						ActiveScene->LoadSceneDataValues();
					}
				}
				bResult = TRUE;
			}
		}
	}


	return bResult;
}

/**
 * Renders debug information to the screen canvas.
 */
void UGameUISceneClient::RenderDebugInfo( FCanvas* Canvas )
{
	if ( GEngine->SmallFont == NULL )
	{
		return;
	}

	// first, calculate the height of the lines
	FLOAT DefaultCharWidth, DefaultCharHeight;
	GEngine->SmallFont->GetCharSize(TCHAR('0'), DefaultCharWidth, DefaultCharHeight);
	if ( DefaultCharWidth == 0 )
	{
		// this font doesn't contain '0', try 'A'
		GEngine->SmallFont->GetCharSize(TCHAR('A'), DefaultCharWidth, DefaultCharHeight);
	}

	UUIScene* TopScene = NULL;
	FVector2D ViewportOrigin(0.f, 0.f), ViewportSize(RenderViewport ? RenderViewport->GetSizeX() : 0.f, RenderViewport ? RenderViewport->GetSizeY() : 0.f);
	if ( IsUIActive(SCENEFILTER_ReceivesFocus) )
	{
		TopScene = GetActiveScene(NULL, TRUE);
		if ( TopScene != NULL )
		{
			TopScene->GetViewportOrigin(ViewportOrigin);
			TopScene->GetViewportSize(ViewportSize);
		}
	}

	FLOAT StartX = ViewportOrigin.X;
	FLOAT StartY = ViewportOrigin.Y;

#if CONSOLE
	// apply the safe zone border
	const FLOAT SafeZonePercentage = 0.9f;

	const FLOAT ViewportWidth = ViewportSize.X;
	const FLOAT ViewportHeight = ViewportSize.Y;
	const FLOAT ScreenWidth = ViewportWidth * SafeZonePercentage;
	const FLOAT ScreenHeight = ViewportHeight * SafeZonePercentage;

	const FLOAT ClipX = ViewportWidth - ((1.f - SafeZonePercentage) * 0.5f);
	const FLOAT ClipY = ViewportHeight - ((1.f - SafeZonePercentage) * 0.5f);

	StartX += (ViewportWidth * 0.5f - ScreenWidth * 0.5f);
	StartY += (ViewportHeight * 0.5f - ScreenHeight * 0.5f);
#else
	const FLOAT ScreenWidth = ViewportSize.X;
	const FLOAT ScreenHeight = ViewportSize.Y;
	const FLOAT ClipX = ScreenWidth;
	const FLOAT ClipY = ScreenHeight;

#endif

	INT CurrentLine = 0;
	if ( !bRenderDebugInfoAtTop )
	{
		StartY = ClipY;
	}

	// show the current mouse position, right justified
	FString MousePositionString = FString::Printf(TEXT("Mouse: %i,%i"), MousePosition.X, MousePosition.Y);

	FRenderParameters RenderParms(GEngine->SmallFont, 1.f, 1.f, ViewportSize.Y);
	UUIString::StringSize(RenderParms, *MousePositionString);

	const FLOAT MouseRenderX = (bShowMousePos ? ClipX - RenderParms.DrawXL : ClipX);

	FLOAT CurY = StartY;
	if ( bRenderActiveControlInfo )
	{
		// Draw information about the scene client's ActiveControl
		if ( ActiveControl != NULL )
		{
			FString WidgetNameString = ActiveControl->GetClass()->GetDescription() + TEXT(" ") + (bDisplayFullPaths
				? (bShowWidgetPath
					? *ActiveControl->GetWidgetPathName()
					: *ActiveControl->GetPathName())
				: (bShowWidgetPath
					? *ActiveControl->GetTag().ToString()
					: *ActiveControl->GetName()));

			FString RenderBoundString;
			if ( bShowRenderBounds )
			{
				RenderBoundString = FString::Printf(TEXT(" [%.2f,%.2f,%.2f,%.2f]"),
					ActiveControl->RenderBounds[UIFACE_Left], ActiveControl->RenderBounds[UIFACE_Top],
					ActiveControl->RenderBounds[UIFACE_Right], ActiveControl->RenderBounds[UIFACE_Bottom]);

				if ( !bDisplayFullPaths )
				{
					RenderBoundString += FString::Printf(TEXT(" (%.2fx%.2f)"), 
						ActiveControl->RenderBounds[UIFACE_Right] - ActiveControl->RenderBounds[UIFACE_Left],
						ActiveControl->RenderBounds[UIFACE_Bottom] - ActiveControl->RenderBounds[UIFACE_Top]);
				}
			}

			FString CurrentStateString;
			if ( bShowCurrentState )
			{
				CurrentStateString = FString::Printf(TEXT(" (%s)"), *ActiveControl->GetCurrentState()->GetClass()->GetDescription());
			}

			FString InfoString = FString::Printf(TEXT("Active: %s%s%s"), *WidgetNameString, *RenderBoundString, *CurrentStateString);

			RenderParms.DrawX = StartX;
			RenderParms.DrawXL = MouseRenderX;
			RenderParms.DrawY = RenderParms.DrawYL = 0.f;

			TArray<FWrappedStringElement> Strings;
			UUIString::WrapString(RenderParms, StartX, *InfoString, Strings);

			if ( !bRenderDebugInfoAtTop )
			{
				for ( INT i = 0; i < Strings.Num(); i++ )
				{
					CurY -= Strings(i).LineExtent.Y;
				}
			}

			const FLOAT RestoreY = CurY;
			for ( INT i = 0; i < Strings.Num(); i++ )
			{
				// draw a black background so that it's easy to read the debug text
				DrawTile( Canvas, StartX, CurY, ScreenWidth, Strings(i).LineExtent.Y, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
				DrawString(Canvas, StartX, CurY, *Strings(i).Value, GEngine->SmallFont, FLinearColor(1.f,0.f,0.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );
				CurY += Strings(i).LineExtent.Y;
			}

			if ( !bRenderDebugInfoAtTop )
			{
				CurY = RestoreY;
			}
		}
		else
		{
			if ( !bRenderDebugInfoAtTop )
			{
				CurY -= DefaultCharHeight;
			}
			// draw a black background so that it's easy to read the debug text
			DrawTile( Canvas, StartX, CurY, ScreenWidth, DefaultCharHeight, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
			DrawString(Canvas, StartX, CurY, TEXT("Active: NULL"), GEngine->SmallFont, FLinearColor(1.f,0.f,0.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );

			if ( bRenderDebugInfoAtTop )
			{
				CurY += DefaultCharHeight;
			}
		}
	}

	if ( bRenderFocusedControlInfo )
	{
		// Draw information about the active scene's focused control
		UUIObject* FocusedControl = NULL;
		if ( IsUIActive(SCENEFILTER_ReceivesFocus) )
		{
			UUIScene* TopScene = GetActiveScene(NULL, TRUE);
			FocusedControl = TopScene->GetFocusedControl(TRUE, Max(0, UUIInteraction::GetPlayerIndex(TopScene->PlayerOwner)));
		}
		if ( FocusedControl != NULL )
		{
			FString WidgetNameString = FocusedControl->GetClass()->GetDescription() + TEXT(" ") + (bDisplayFullPaths
				? (bShowWidgetPath
					? *FocusedControl->GetWidgetPathName()
					: *FocusedControl->GetPathName())
				: (bShowWidgetPath
					? *FocusedControl->GetTag().ToString()
					: *FocusedControl->GetName()));

			FString RenderBoundString;
			if ( bShowRenderBounds )
			{
				RenderBoundString = FString::Printf(TEXT(" [%.2f,%.2f,%.2f,%.2f]"),
					FocusedControl->RenderBounds[UIFACE_Left], FocusedControl->RenderBounds[UIFACE_Top],
					FocusedControl->RenderBounds[UIFACE_Right], FocusedControl->RenderBounds[UIFACE_Bottom]);

				if ( !bDisplayFullPaths )
				{
					RenderBoundString += FString::Printf(TEXT(" (%.2fx%.2f)"), 
						FocusedControl->RenderBounds[UIFACE_Right] - FocusedControl->RenderBounds[UIFACE_Left],
						FocusedControl->RenderBounds[UIFACE_Bottom] - FocusedControl->RenderBounds[UIFACE_Top]);
				}
			}

			FString CurrentStateString;
			if ( bShowCurrentState )
			{
				CurrentStateString = FString::Printf(TEXT(" (%s)"), *FocusedControl->GetCurrentState()->GetClass()->GetDescription());
			}

			FString InfoString = FString::Printf(TEXT("Focused: %s%s%s"), *WidgetNameString, *RenderBoundString, *CurrentStateString);

			RenderParms.DrawX = StartX;
			RenderParms.DrawXL = bRenderActiveControlInfo ? ClipX : MouseRenderX;
			RenderParms.DrawY = RenderParms.DrawYL = 0.f;
// 			RenderParms.DrawX = RenderParms.DrawY = RenderParms.DrawYL = 0;
// 			RenderParms.DrawXL = bRenderActiveControlInfo ? ScreenWidth : MouseRenderX;
			TArray<FWrappedStringElement> Strings;
			UUIString::WrapString( RenderParms, StartX, *InfoString, Strings);

			if ( !bRenderDebugInfoAtTop )
			{
				for ( INT i = 0; i < Strings.Num(); i++ )
				{
					CurY -= Strings(i).LineExtent.Y;
				}
			}

			const FLOAT RestoreY = CurY;
			for ( INT i = 0; i < Strings.Num(); i++ )
			{
				// draw a black background so that it's easy to read the debug text
				DrawTile( Canvas, StartX, CurY, ScreenWidth, Strings(i).LineExtent.Y, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
				DrawString(Canvas, StartX, CurY, *Strings(i).Value, GEngine->SmallFont, FLinearColor(0.f,0.f,1.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );
				CurY += Strings(i).LineExtent.Y;
			}

			if ( !bRenderDebugInfoAtTop )
			{
				CurY = RestoreY;
			}
		}
		else
		{
			if ( !bRenderDebugInfoAtTop )
			{
				CurY -= DefaultCharHeight;
			}
			// draw a black background so that it's easy to read the debug text
			DrawTile( Canvas, StartX, CurY, ScreenWidth, DefaultCharHeight, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
			DrawString(Canvas, StartX, CurY, TEXT("Focused: NULL"), GEngine->SmallFont, FLinearColor(0.f,0.f,1.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );

			if ( bRenderDebugInfoAtTop )
			{
				CurY += DefaultCharHeight;
			}
		}
	}

	if ( bRenderTargetControlInfo )
	{
		// Draw information about the current debug target
		if ( DebugTarget != NULL )
		{
			FString WidgetNameString = DebugTarget->GetClass()->GetDescription() + TEXT(" ") + (bDisplayFullPaths
				? (bShowWidgetPath
					? *DebugTarget->GetWidgetPathName()
					: *DebugTarget->GetPathName())
				: (bShowWidgetPath
					? *DebugTarget->GetTag().ToString()
					: *DebugTarget->GetName()));

			FString RenderBoundString;
			if ( bShowRenderBounds )
			{
				RenderBoundString = FString::Printf(TEXT(" [%.2f,%.2f,%.2f,%.2f]"),
					DebugTarget->GetPosition(UIFACE_Left,EVALPOS_PixelViewport),
					DebugTarget->GetPosition(UIFACE_Top,EVALPOS_PixelViewport),
					DebugTarget->GetPosition(UIFACE_Right,EVALPOS_PixelViewport),
					DebugTarget->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport));

				if ( !bDisplayFullPaths )
				{
					RenderBoundString += FString::Printf(TEXT(" (%.2fx%.2f)"), 
						DebugTarget->GetBounds(UIORIENT_Horizontal, EVALPOS_PixelViewport),
						DebugTarget->GetBounds(UIORIENT_Vertical, EVALPOS_PixelViewport));
				}
			}

			FString CurrentStateString;
			if ( bShowCurrentState )
			{
				CurrentStateString = FString::Printf(TEXT(" (%s)"), *DebugTarget->GetCurrentState()->GetClass()->GetDescription());
			}
			FString InfoString = FString::Printf(TEXT("Target: %s%s%s"), *WidgetNameString, *RenderBoundString, *CurrentStateString);

			RenderParms.DrawX = StartX;
			RenderParms.DrawXL = bRenderActiveControlInfo || bRenderFocusedControlInfo ? ClipX : MouseRenderX;
			RenderParms.DrawY = RenderParms.DrawYL = 0.f;
// 			RenderParms.DrawX = RenderParms.DrawY = RenderParms.DrawYL = 0;
// 			RenderParms.DrawXL = bRenderActiveControlInfo || bRenderFocusedControlInfo ? ScreenWidth : MouseRenderX;
			TArray<FWrappedStringElement> Strings;
			UUIString::WrapString( RenderParms, StartX, *InfoString, Strings);

			if ( !bRenderDebugInfoAtTop )
			{
				for ( INT i = 0; i < Strings.Num(); i++ )
				{
					CurY -= Strings(i).LineExtent.Y;
				}
			}

			const FLOAT RestoreY = CurY;
			for ( INT i = 0; i < Strings.Num(); i++ )
			{
				// draw a black background so that it's easy to read the debug text
				DrawTile( Canvas, StartX, CurY, ScreenWidth, Strings(i).LineExtent.Y, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
				DrawString(Canvas, StartX, CurY, *Strings(i).Value, GEngine->SmallFont, FLinearColor(0.f,1.f,0.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );
				CurY += Strings(i).LineExtent.Y;
			}
			if ( !bRenderDebugInfoAtTop )
			{
				CurY = RestoreY;
			}

			UUIObject* WidgetTarget = Cast<UUIObject>(DebugTarget);
			if ( WidgetTarget != NULL )
			{
				// now draw an outline around the widget
				// Set 3D Projection
				Canvas->SetBaseTransform(CanvasToScreen);
				Canvas->PushAbsoluteTransform(WidgetTarget->GenerateTransformMatrix(TRUE));
			}

			DrawBox2D( Canvas,
				FVector2D(DebugTarget->GetPosition(UIFACE_Left,EVALPOS_PixelViewport,TRUE),DebugTarget->GetPosition(UIFACE_Top,EVALPOS_PixelViewport,TRUE)),
				FVector2D(DebugTarget->GetPosition(UIFACE_Right,EVALPOS_PixelViewport,TRUE),DebugTarget->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport,TRUE)),
				FLinearColor(0.f, 1.f, 0.f, 1.f) );

			if ( WidgetTarget != NULL )
			{
				Canvas->PopTransform();

				// Restore 2D Projection
				Canvas->SetBaseTransform(FCanvas::CalcBaseTransform2D(appTrunc(ViewportSize.X), appTrunc(ViewportSize.Y)));
			}
		}
		else
		{
			if ( !bRenderDebugInfoAtTop )
			{
				CurY -= DefaultCharHeight;
			}
			// draw a black background so that it's easy to read the debug text
			DrawTile( Canvas, StartX, CurY, ScreenWidth, DefaultCharHeight, 0, 0, 0, 0, FLinearColor(0,0,0,1) );
			DrawString(Canvas, StartX, CurY, TEXT("Target: NULL"), GEngine->SmallFont, FLinearColor(0.f,1.f,0.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );

			if ( bRenderDebugInfoAtTop )
			{
				CurY += DefaultCharHeight;
			}
		}
	}

	if ( bShowMousePos )
	{
		DrawString( Canvas, MouseRenderX, bRenderDebugInfoAtTop ? StartY : ClipY - DefaultCharHeight, *MousePositionString,
			GEngine->SmallFont, FLinearColor(0.f,0.f,1.f,1.f), 1.f, 1.f, 0.f, &RenderParms.ViewportHeight );
	}
}

/**
 * @return	TRUE if the scene meets the conditions defined by the bitmask specified.
 */
UBOOL UGameUISceneClient::SceneMatchesFilter( DWORD FilterFlagMask, UUIScene* TestScene ) const
{
	checkSlow(TestScene);

	if ( (FilterFlagMask&SCENEFILTER_Any) == SCENEFILTER_Any )
	{
		return TRUE;
	}

	if ( (FilterFlagMask&SCENEFILTER_IncludeTransient) != 0 && TestScene != GetTransientScene() )
	{
		return FALSE;
	}

	if ( (FilterFlagMask&SCENEFILTER_InputProcessorOnly) != 0 && TestScene->GetSceneInputMode() == INPUTMODE_None )
	{
		return FALSE;
	}

	if ( (FilterFlagMask&SCENEFILTER_PausersOnly) != 0 && !TestScene->bPauseGameWhileActive )
	{
		return FALSE;
	}

	if ( (FilterFlagMask&SCENEFILTER_PrimitiveUsersOnly) != 0 && !TestScene->bUsesPrimitives )
	{
		return FALSE;
	}

	if ( (FilterFlagMask&SCENEFILTER_ReceivesFocus) != 0 && TestScene->IsNeverFocused() )
	{
		return FALSE;
	}

	static const DOUBLE InvMaxColor = 1.f / 255.f;
	if ((FilterFlagMask&SCENEFILTER_UsesPostProcessing) != 0
	&&	(!TestScene->bEnableScenePostProcessing || !TestScene->IsVisible() || TestScene->Opacity + DELTA <= InvMaxColor
	||	(UIScenePostProcess == NULL && TestScene->GetPostProcessChain(UIPostProcess_Background) == NULL && TestScene->GetPostProcessChain(UIPostProcess_Foreground) == NULL)))
	{
		return FALSE;
	}

	return TRUE;
}

/**
 * Returns true if there is an unhidden fullscreen UI active
 *
 * @param	Flags	modifies the logic which determines whether the UI is active
 *
 * @return TRUE if the UI is currently active
 */
UBOOL UGameUISceneClient::IsUIActive( DWORD Flags/*=SCENEFILTER_Any*/ ) const
{
	UBOOL bResult = FALSE;

	for (INT SceneIndex = 0; SceneIndex < ActiveScenes.Num() ; SceneIndex++)
	{
		if ( SceneMatchesFilter(Flags, ActiveScenes(SceneIndex)) )
		{
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}

/**
 * Returns whether the specified scene has been fully initialized.  Different from UUIScene::IsInitialized() in that this
 * method returns true only once all objects related to this scene have been created and initialized (e.g. in the UI editor
 * only returns TRUE once the editor window for this scene has finished creation).
 *
 * @param	Scene	the scene to check.
 */
UBOOL UGameUISceneClient::IsSceneInitialized( const UUIScene* Scene ) const
{
	return Scene != NULL && Scene->IsInitialized()

		// first condition is to ensure that the call to UGameUISceneClient::ActivateScene has completed
		// second condition is for scenes which were fully initialized but have just been removed from the ActiveScenes array
		&& (ActiveScenes.ContainsItem(const_cast<UUIScene*>(Scene)) || Scene->bPerformedInitialUpdate);
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
UBOOL UGameUISceneClient::InputKey(INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed/*=1.f*/,UBOOL bGamepad/*=FALSE*/)
{
	UBOOL bResult = FALSE;

	// first check to see if this is a key-release event we should ignore (see FlushPlayerInput)
	if ( InitialPressedKeys.Num() > 0 && (Event == IE_Repeat || Event == IE_Released) )
	{
		TArray<FName>* PressedPlayerKeys = InitialPressedKeys.Find(ControllerId);
		if ( PressedPlayerKeys != NULL )
		{
			INT KeyIndex = PressedPlayerKeys->FindItemIndex(Key);
			if ( KeyIndex != INDEX_NONE )
			{
				// this key was found in the InitialPressedKeys array for this player, which means that the key was already
				// pressed when the UI began processing input - ignore this key event
				if ( Event == IE_Released )
				{
					// if the player has released the key, remove the key from the list of keys to ignore
					PressedPlayerKeys->Remove(KeyIndex);
				}

				// and swallow this input event
				bResult = TRUE;
			}
		}
	}

	if ( bEnableDebugInput && !bResult && IsUIActive(SCENEFILTER_Any) )
	{
		// see if the input corresponds to a debug command
		bResult = DebugInputKey(Key, Event);
	}

	if ( !bResult )
	{
		for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
		{
			UUIScene* Scene = ActiveScenes(SceneIndex);
			if ( Scene->InputKey(ControllerId,Key,Event,AmountDepressed,bGamepad) )
			{
				bResult = TRUE;
				break;
			}
		}
	}

	return bResult || bCaptureUnprocessedInput;
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
UBOOL UGameUISceneClient::InputAxis( INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad )
{
	UBOOL bResult = FALSE;

	for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene->InputAxis(ControllerId,Key,Delta,DeltaTime) )
		{
			bResult = TRUE;
			break;
		}
	}

	return bResult || bCaptureUnprocessedInput;
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
UBOOL UGameUISceneClient::InputChar(INT ControllerId,TCHAR Character)
{
	UBOOL bResult = FALSE;

	for ( INT SceneIndex = ActiveScenes.Num() - 1; SceneIndex >= 0; SceneIndex-- )
	{
		UUIScene* Scene = ActiveScenes(SceneIndex);
		if ( Scene->InputChar(ControllerId,Character) )
		{
			bResult = TRUE;
			break;
		}
	}

	return bResult || bCaptureUnprocessedInput;
}

/* ==========================================================================================================
	FUIRangeData
========================================================================================================== */
UBOOL FUIRangeData::operator==( const FUIRangeData& Other ) const
{
	UBOOL bResult = FALSE;
	if ( bIntRange )
	{
		bResult 
			=	Other.bIntRange
			&&	appRound(CurrentValue)	== appRound(Other.CurrentValue)
			&&	appRound(MinValue)		== appRound(Other.MinValue)
			&&	appRound(MaxValue)		== appRound(Other.MaxValue)
			&&	appRound(NudgeValue)	== appRound(Other.NudgeValue);
	}
	else
	{
		bResult
			=	!Other.bIntRange
			&&	ARE_FLOATS_EQUAL(CurrentValue, Other.CurrentValue)
			&&	ARE_FLOATS_EQUAL(MinValue, Other.MinValue)
			&&	ARE_FLOATS_EQUAL(MaxValue, Other.MaxValue)
			&&	ARE_FLOATS_EQUAL(NudgeValue, Other.NudgeValue);
	}
	return bResult;
}
UBOOL FUIRangeData::operator!=( const FUIRangeData& Other ) const
{
	return !(FUIRangeData::operator==(Other));
}

/**
 * Returns true if any values in this struct are non-zero.
 */
UBOOL FUIRangeData::HasValue() const
{
	return CurrentValue != 0 || MinValue != 0 || MaxValue != 0 || NudgeValue != 0 || bIntRange == TRUE;
}

/**
 * Returns the amount that this range should be incremented/decremented when nudging.
 */
FLOAT FUIRangeData::GetNudgeValue() const
{
	FLOAT Result = NudgeValue;

	// if NudgeValue is 0, nudge the value by 1% of the slider's total range
	if ( Result == 0.f )
	{
		Result = (MaxValue - MinValue) * 0.01;
	}

	return Result;
}

/**
 * Returns the current value of this UIRange.
 */
FLOAT FUIRangeData::GetCurrentValue() const
{
	return (bIntRange == TRUE) ? appRound(CurrentValue) : CurrentValue;
}

/**
 * Sets the value of this UIRange.
 *
 * @param	NewValue				the new value to assign to this UIRange.
 * @param	bClampInvalidValues		specify TRUE to automatically clamp NewValue to a valid value for this UIRange.
 *
 * @return	TRUE if the value was successfully assigned.  FALSE if NewValue was outside the valid range and
 *			bClampInvalidValues was FALSE or MinValue <= MaxValue.
 */
UBOOL FUIRangeData::SetCurrentValue( FLOAT NewValue, UBOOL bClampInvalidValues/*=TRUE*/ )
{
	UBOOL bResult = FALSE;

	if ( bClampInvalidValues == TRUE && MaxValue > MinValue )
	{
		NewValue = Clamp<FLOAT>(NewValue, MinValue, MaxValue);
	}

	if ( bIntRange == TRUE )
	{
		NewValue = appRound(NewValue);
	}

	if ( NewValue >= MinValue && NewValue <= MaxValue )
	{
		CurrentValue = NewValue;
		bResult = TRUE;
	}

	return bResult;
}

/* ==========================================================================================================
	FInputEventParameters
========================================================================================================== */
/** Default constructor */
FInputEventParameters::FInputEventParameters()
: PlayerIndex(INDEX_NONE)
, InputKeyName(NAME_None)
, EventType(IE_MAX)
, InputDelta(0.f)
, DeltaTime(0.f)
, bAltPressed(FALSE)
, bCtrlPressed(FALSE)
, bShiftPressed(FALSE)
{
}

/** Input Key Event constructor */
FInputEventParameters::FInputEventParameters( INT InPlayerIndex, INT InControllerId, FName KeyName, EInputEvent Event, UBOOL bAlt, UBOOL bCtrl, UBOOL bShift, FLOAT AmountDepressed/*=1.f*/ )
: PlayerIndex(InPlayerIndex)
, ControllerId(InControllerId)
, InputKeyName(KeyName)
, EventType(Event)
, InputDelta(AmountDepressed)
, DeltaTime(0.f)
, bAltPressed(bAlt)
, bCtrlPressed(bCtrl)
, bShiftPressed(bShift)
{
}

/** Input Axis Event constructor */
FInputEventParameters::FInputEventParameters( INT InPlayerIndex, INT InControllerId, FName KeyName, FLOAT AxisAmount, FLOAT InDeltaTime, UBOOL bAlt, UBOOL bCtrl, UBOOL bShift )
: PlayerIndex(InPlayerIndex)
, ControllerId(InControllerId)
, InputKeyName(KeyName)
, EventType(IE_Axis)
, InputDelta(AxisAmount)
, DeltaTime(InDeltaTime)
, bAltPressed(bAlt)
, bCtrlPressed(bCtrl)
, bShiftPressed(bShift)
{
}

/* ==========================================================================================================
	FSubscribedInputEventParameters
========================================================================================================== */
/** Default constructor */
FSubscribedInputEventParameters::FSubscribedInputEventParameters() : FInputEventParameters(), InputAliasName(NAME_None)
{
}

/** Input Key Event constructor */
FSubscribedInputEventParameters::FSubscribedInputEventParameters( INT InPlayerIndex, INT InControllerId, FName KeyName, EInputEvent Event, FName InInputAliasName, UBOOL bAlt, UBOOL bCtrl, UBOOL bShift, FLOAT AmountDepressed/*=1.f*/ )
: FInputEventParameters(InPlayerIndex, InControllerId, KeyName, Event, bAlt, bCtrl, bShift, AmountDepressed), InputAliasName(InInputAliasName)
{
}

/** Input Axis Event constructor */
FSubscribedInputEventParameters::FSubscribedInputEventParameters( INT InPlayerIndex, INT InControllerId, FName KeyName, FName InInputAliasName, FLOAT AxisAmount, FLOAT InDeltaTime, UBOOL bAlt, UBOOL bCtrl, UBOOL bShift )
: FInputEventParameters(InPlayerIndex, InControllerId, KeyName, AxisAmount, InDeltaTime, bAlt, bCtrl, bShift), InputAliasName(InInputAliasName)
{
}

/** Copy constructor */
FSubscribedInputEventParameters::FSubscribedInputEventParameters( const FSubscribedInputEventParameters& Other )
: FInputEventParameters((const FInputEventParameters&)Other), InputAliasName(Other.InputAliasName)
{
}
FSubscribedInputEventParameters::FSubscribedInputEventParameters( const FInputEventParameters& Other, FName InInputAliasName )
: FInputEventParameters(Other), InputAliasName(InInputAliasName)
{
}

/* ==========================================================================================================
	FUIScreenValue
========================================================================================================== */
/**
 * Calculates the origin and extent for the position value of a single widget face
 *
 * @param	OwnerWidget			the widget that owns this position
 * @param	Face				the face to evaluate
 * @param	Type				indicates how the base values will be used, how they should be formatted
 * @param	BaseValue			[out] absolute pixel values for the base of this position for the specified face.  For example,
 *								if the Face is UIFACE_Left, BaseValue will represent the X position of the OwnerWidget's container,
 * @param	bInternalPosition	specify TRUE to indicate that BaseValue should come from OwnerWidget; FALSE to indicate that BaseValue should come from
 *								OwnerWidget's parent widget.
 * @param	bIgnoreDockPadding	used to prevent recursion when evaluting docking links
 */
void FUIScreenValue::CalculateBaseValue( const UUIScreenObject* OwnerWidget, EUIWidgetFace Face, EPositionEvalType Type, FLOAT& BaseValue, FLOAT& BaseExtent, UBOOL bInternalPosition/*=FALSE*/, UBOOL bIgnoreDockPadding/*=FALSE*/ )
{
	//SCOPE_CYCLE_COUNTER(STAT_UICalculateBaseValue);
	check(OwnerWidget);

	const UUIScreenObject* OwnerWidgetContainer = NULL;
	BaseValue = 0.f;
	BaseExtent = 1.f;

	switch ( Type )
	{
	case EVALPOS_PixelOwner:
	case EVALPOS_PercentageOwner:
		{
			// Check if we should be using the widget's parent or the widget itself.
			const UUIScreenObject* WidgetParent = bInternalPosition ? OwnerWidget : OwnerWidget->GetParent();
			if ( WidgetParent != NULL )
			{
				OwnerWidgetContainer = WidgetParent;
				break;
			}
		}

		// falls through
	case EVALPOS_PixelScene:
	case EVALPOS_PercentageScene:
		{
			const UUIScene* OwnerScene = OwnerWidget->GetScene();
			if ( OwnerScene != OwnerWidget )
			{
				OwnerWidgetContainer = OwnerScene;
				break;
			}
		}

		// falls through
	case EVALPOS_PercentageViewport:
		{
			FVector2D ViewportSize;
			if ( OwnerWidget->GetViewportSize(ViewportSize) )
			{
				switch ( Face )
				{
				case UIFACE_Left:
				case UIFACE_Right:
					BaseExtent = ViewportSize.X;
					break;

				case UIFACE_Top:
				case UIFACE_Bottom:
					BaseExtent = ViewportSize.Y;
					break;
				}

				break;
			}
		}

		switch ( Face )
		{
		case UIFACE_Left:
		case UIFACE_Right:
			BaseExtent = UCONST_DEFAULT_SIZE_X;
			break;

		case UIFACE_Top:
		case UIFACE_Bottom:
			BaseExtent = UCONST_DEFAULT_SIZE_Y;
			break;
		}

		break;

	case EVALPOS_PixelViewport:
		{
			// this is set at the top of the function, but just to be sure, reset this to 1.f
			BaseExtent = 1.f;
			break;
		}
	}


	if ( OwnerWidgetContainer != NULL )
	{
		switch ( Face )
		{
		case UIFACE_Left:
			BaseValue = OwnerWidgetContainer->GetPosition(Face, EVALPOS_PixelViewport, TRUE, bIgnoreDockPadding);
			if ( Type != EVALPOS_PixelOwner && Type != EVALPOS_PixelScene )
			{
				// type must be either PercentageScene or PercentageOwner
				BaseExtent = OwnerWidgetContainer->GetPosition(UIFACE_Right, EVALPOS_PixelViewport, TRUE, bIgnoreDockPadding) - BaseValue;
			}
			break;

		case UIFACE_Top:
			BaseValue = OwnerWidgetContainer->GetPosition(Face, EVALPOS_PixelViewport, TRUE, bIgnoreDockPadding);
			if ( Type != EVALPOS_PixelOwner && Type != EVALPOS_PixelScene )
			{
				// type must be either PercentageScene or PercentageOwner
				BaseExtent = OwnerWidgetContainer->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport, TRUE, bIgnoreDockPadding) - BaseValue;
			}
			break;

		case UIFACE_Right:
		case UIFACE_Bottom:
			BaseValue = OwnerWidget->GetPosition(UUIRoot::GetOppositeFace(Face), EVALPOS_PixelViewport, TRUE, bIgnoreDockPadding );
			if ( Type != EVALPOS_PixelOwner && Type != EVALPOS_PixelScene )
			{
				// type must be either PercentageScene or PercentageOwner
				EUIOrientation Orientation = Face == UIFACE_Right
					? UIORIENT_Horizontal
					: UIORIENT_Vertical;

				BaseExtent = OwnerWidgetContainer->Position.GetBoundsExtent(OwnerWidgetContainer, Orientation, EVALPOS_PixelViewport, bIgnoreDockPadding);
			}
			break;
		}
	}
	else
	{
		FVector2D ViewportOrigin(EC_EventParm);
		OwnerWidget->GetViewportOrigin(ViewportOrigin);
		BaseValue = ViewportOrigin[Face % UIORIENT_MAX];
	}
}

/**
 * Evaluates the value stored in this UIScreenValue
 *
 * @param	OwnerWidget	the widget that contains this screen value
 * @param	OutputType	determines the format of the result.
 *						EVALPOS_None:
 *							return value is formatted using this screen position's ScaleType for the specified face
 *						EVALPOS_PercentageOwner:
 *						EVALPOS_PercentageScene:
 *						EVALPOS_PercentageViewport:
 *							return a value between 0.0 and 1.0, which represents the percentage of the corresponding
 *							base's actual size.  If OwnerWidget isn't specified, the size of the
 *							entire viewport is used.
 *						EVALPOS_PixelOwner
 *						EVALPOS_PixelScene
 *						EVALPOS_PixelViewport
 *							return the actual pixel values represented by this UIScreenValue, relative to the corresponding base.
 * @param	bInternalPosition
 *						specify TRUE if this UIScreenValue represents a point or distance inside of OwnerWidget, in which case any
 *						relative scale types will use OwnerWidget as the base.  Specify FALSE if it represents a point/distance outside
 *						OwnerWidget, in which case OwnerWidget's parent will be used as a base.
 *
 * @return	the actual value for this UIScreenValue, in pixels or percentage, for the face specified.
 */
FLOAT FUIScreenValue::GetValue( const UUIScreenObject* OwnerWidget, EPositionEvalType OutputType/*=EVALPOS_None*/, UBOOL bInternalPosition/*=TRUE*/ ) const
{
	FLOAT Result = Value;
	if ( OwnerWidget == NULL || OutputType == EVALPOS_None )
		return Result;

	EUIWidgetFace Face = (Orientation == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top);

	FLOAT BaseValue,BaseExtent;

	// first, determine the actual value of this screen position
	FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType, BaseValue, BaseExtent, bInternalPosition);
	Result = BaseValue + (BaseExtent * Value);

	if ( OutputType != EVALPOS_PixelViewport )
	{
		// the output type might be
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, OutputType, BaseValue, BaseExtent, bInternalPosition);

		Result -= BaseValue;
		Result /= BaseExtent;
	}

	return Result;
}

/**
 * Convert the input value into the appropriate type for this UIScreenValue, and assign that Value
 *
 * @param	OwnerWidget		the widget that contains this screen value
 * @param	NewValue		the new value (in pixels or percentage) to use
 * @param	InputType		indicates the format of the input value
 *							EVALPOS_None:
 *								NewValue is assumed to be formatted with what this screen position's ScaleType is for the specified face
 *							EVALPOS_PercentageOwner:
 *							EVALPOS_PercentageScene:
 *							EVALPOS_PercentageViewport:
 *								Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
 *								base's actual size.
 *							EVALPOS_PixelOwner
 *							EVALPOS_PixelScene
 *							EVALPOS_PixelViewport
 *								Indicates that NewValue is an actual pixel value, relative to the corresponding base.
 */
void FUIScreenValue::SetValue( class UUIScreenObject* OwnerWidget, FLOAT NewValue, EPositionEvalType InputType/*=EVALPOS_PixelViewport*/ )
{
	if ( OwnerWidget == NULL || InputType == EVALPOS_None )
		return;

	FLOAT ConvertedValue = NewValue;
	if ( InputType != ScaleType )
	{
		EUIWidgetFace Face = (Orientation == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top);

		UUIScreenObject* OwnerWidgetContainer=NULL;
		FLOAT BaseValue, BaseExtent;

		// first, convert the input value into absolute pixel values, if necessary
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, InputType, BaseValue, BaseExtent, TRUE);
		ConvertedValue = BaseValue + (BaseExtent * NewValue);

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( ScaleType != EVALPOS_PixelViewport )
		{
			FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType, BaseValue, BaseExtent, TRUE);

			ConvertedValue -= BaseValue;
			ConvertedValue /= BaseExtent;
		}
	}

	Value = ConvertedValue;
}

/**
 * Changes the scale type for the specified face to the value specified, and converts the Value for that face into the new type.
 *
 * @param	OwnerWidget			the widget that contains this screen value
 * @param	NewEvalType			the evaluation type to set for the specified face
 * @param	bAutoConvertValue	if TRUE, the current value of the position will be converted into the equivalent value for the new type
 */
void FUIScreenValue::ChangeScaleType( class UUIScreenObject* OwnerWidget, EPositionEvalType NewEvalType, UBOOL bAutoConvertValue/*=TRUE*/ )
{
	ScaleType = NewEvalType;

	if( bAutoConvertValue )
	{
		Value = GetValue( OwnerWidget, (EPositionEvalType)ScaleType );
	}
}

/* ==========================================================================================================
	FUIScreenValue_Position
========================================================================================================== */

/**
* Evaluates the value stored in this UIScreenValue. It assumes that a Dimension of UIORIENT_Horizontal will correspond to the Left face and
* that a Dinemsion of UIORIENT_Vertical will correspond to the Right face.
*
* @param	Dimension		indicates which element of the Value array to evaluate
* @param	InputType		indicates the format of the input value
*							EVALPOS_None:
*								NewValue is assumed to be formatted with what this screen position's ScaleType is for the specified face
*							EVALPOS_PercentageOwner:
*							EVALPOS_PercentageScene:
*							EVALPOS_PercentageViewport:
*								Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
*								base's actual size.
*							EVALPOS_PixelOwner:
*							EVALPOS_PixelScene:
*							EVALPOS_PixelViewport
*								Indicates that NewValue is an actual pixel value, relative to the corresponding base.
* @param	OwnerWidget		the widget that contains this screen value
*
* @return	the actual value for this UIScreenValue, in pixels or percentage, for the dimension specified.
*/
FLOAT FUIScreenValue_Position::GetValue( EUIOrientation Dimension, EPositionEvalType OutputType/*=EVALPOS_None*/, const UUIScreenObject* OwnerWidget/*=NULL*/ ) const
{
	return GetValue( Dimension, (Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top), OutputType, OwnerWidget );
}

/**
* Evaluates the value stored in this UIScreenValue
*
* @param	Dimension		indicates which element of the Value array to evaluate
* @param	Face			indicates which face on the owner widget the element from the Value array will be relative to (if InputType is
*							applicable ).
* @param	InputType		indicates the format of the input value
*							EVALPOS_None:
*								NewValue is assumed to be formatted with what this screen position's ScaleType is for the specified face
*							EVALPOS_PercentageOwner:
*							EVALPOS_PercentageScene:
*							EVALPOS_PercentageViewport:
*								Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
*								base's actual size.
*							EVALPOS_PixelOwner:
*							EVALPOS_PixelScene:
*							EVALPOS_PixelViewport
*								Indicates that NewValue is an actual pixel value, relative to the corresponding base.
* @param	OwnerWidget		the widget that contains this screen value
*
* @return	the actual value for this UIScreenValue, in pixels or percentage, for the dimension specified.
*/
FLOAT FUIScreenValue_Position::GetValue( EUIOrientation Dimension, EUIWidgetFace Face, EPositionEvalType OutputType, const UUIScreenObject* OwnerWidget ) const
{
	FLOAT Result = Value[Dimension];

	if ( OwnerWidget == NULL || OutputType == EVALPOS_None )
		return Result;

	FLOAT BaseValue,BaseExtent;

	// first, determine the actual value of this screen position
	FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Dimension], BaseValue, BaseExtent, TRUE);
	Result = BaseValue + (BaseExtent * Value[Dimension]);

	if ( OutputType != EVALPOS_PixelViewport )
	{
		// the output type might be
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, OutputType, BaseValue, BaseExtent, TRUE);

		Result -= BaseValue;
		Result /= BaseExtent;
	}

	return Result;
}

/**
 * Convert the input value into the appropriate type for this UIScreenValue, and assign that Value
 *
 * @param	OwnerWidget		the widget that contains this screen value
 * @param	Dimension		indicates which element of the Value array to evaluate
 * @param	NewValue		the new value (in pixels or percentage) to use
 * @param	InputType		indicates the format of the input value
 *							EVALPOS_None:
 *								NewValue is assumed to be formatted with what this screen position's ScaleType is for the specified face
 *							EVALPOS_PercentageOwner:
 *							EVALPOS_PercentageScene:
 *							EVALPOS_PercentageViewport:
 *								Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
 *								base's actual size.
 *							EVALPOS_PixelOwner
 *							EVALPOS_PixelScene
 *							EVALPOS_PixelViewport
 *								Indicates that NewValue is an actual pixel value, relative to the corresponding base.
 */
void FUIScreenValue_Position::SetValue( const UUIScreenObject* OwnerWidget, EUIOrientation Dimension, FLOAT NewValue, EPositionEvalType InputType/*=EVALPOS_PixelViewport*/ )
{
	if ( OwnerWidget == NULL || InputType == EVALPOS_None )
		return;

	FLOAT ConvertedValue = NewValue;
	if ( InputType != ScaleType[Dimension] )
	{
		EUIWidgetFace Face = (Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top);

		FLOAT BaseValue, BaseExtent;

		// first, convert the input value into absolute pixel values, if necessary
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, InputType, BaseValue, BaseExtent, TRUE);
		ConvertedValue = BaseValue + (BaseExtent * NewValue);

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( ScaleType[Dimension] != EVALPOS_PixelViewport )
		{
			FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Dimension], BaseValue, BaseExtent, TRUE);
			ConvertedValue -= BaseValue;
			ConvertedValue /= BaseExtent;
		}
	}

	Value[Dimension] = ConvertedValue;
}

/* ==========================================================================================================
	FUIScreenValue_Bounds
========================================================================================================== */
/**
 * Evaluates the value stored in this UIScreenValue
 *
 * @param	OwnerWidget	the widget that contains this screen value
 * @param	Face		indicates which element of the Value array to evaluate
 * @param	OutputType	determines the format of the result.
 *						EVALPOS_None:
 *							return value is formatted using this struct's ScaleType
 *						EVALPOS_PercentageOwner:
 *						EVALPOS_PercentageScene:
 *						EVALPOS_PercentageViewport:
 *							return a value between 0.0 and 1.0, which represents the percentage of the corresponding
 *							base's actual size.  If OwnerWidget isn't specified, the size of the
 *							entire viewport is used.
 *						EVALPOS_Pixels
 *							return the actual pixel values represented by this UIScreenValue.
 * @param	bIgnoreDockPadding
 *						used to prevent recursion when evaluting docking links
 *
 * @return	the actual value for this UIScreenValue, in pixels or percentage, for the face specified.
 */
FLOAT FUIScreenValue_Bounds::GetPositionValue( const UUIScreenObject* OwnerWidget, EUIWidgetFace Face, EPositionEvalType OutputType/*=EVALPOS_None*/, UBOOL bIgnoreDockPadding/*=FALSE*/ ) const
{
	// invalid if we're docked
	FLOAT Result = Value[Face];
	if ( OutputType == EVALPOS_None )
		return Result;

	//SCOPE_CYCLE_COUNTER(STAT_UIGetPositionValue);
	FLOAT BaseValue,BaseExtent;

	// first, determine the actual value of this screen position
	const UUIObject* OwnerWidgetObject = ConstCast<UUIObject>(OwnerWidget);
	if ( OwnerWidgetObject != NULL )
	{
		if ( IsPositionCurrent(OwnerWidgetObject,Face) )
		{
			Result = OwnerWidgetObject->RenderBounds[Face];
		}
		else if ( OwnerWidgetObject->DockTargets.IsDocked(Face) )
		{
			UUIObject* DockTarget = OwnerWidgetObject->DockTargets.GetDockTarget(Face);
			if ( DockTarget != NULL )
			{
				Result = DockTarget->GetPosition(OwnerWidgetObject->DockTargets.GetDockFace(Face), EVALPOS_PixelViewport, TRUE);
				if ( !bIgnoreDockPadding )
				{
					Result += OwnerWidgetObject->DockTargets.GetDockPadding(Face);
				}
			}
			else if ( OwnerWidgetObject->GetScene() != NULL )
			{
				Result = OwnerWidgetObject->GetScene()->GetPosition(OwnerWidgetObject->DockTargets.GetDockFace(Face), EVALPOS_PixelViewport, TRUE);
				if ( !bIgnoreDockPadding )
				{
					Result += OwnerWidgetObject->DockTargets.GetDockPadding(Face);
				}
			}
			else
			{
				FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Face], BaseValue, BaseExtent, FALSE, bIgnoreDockPadding);
				Result = BaseValue + (BaseExtent * Value[Face]);
			}
		}
		else
		{
			FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Face], BaseValue, BaseExtent, FALSE, bIgnoreDockPadding);
			Result = BaseValue + (BaseExtent * Value[Face]);
		}
	}
	else
	{
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Face], BaseValue, BaseExtent, FALSE, bIgnoreDockPadding);
		Result = BaseValue + (BaseExtent * Value[Face]);
	}

	if ( OutputType != EVALPOS_PixelViewport )
	{
		// the output type might be
		FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, OutputType, BaseValue, BaseExtent, FALSE, bIgnoreDockPadding);

		Result -= BaseValue;
		Result /= BaseExtent;
	}

	return Result;
}

/**
 * Convert the value specified into the appropriate type for this screen value, and set that as the value for the face specified.
 *
 * @param	OwnerWidget		the widget that contains this screen value
 * @param	NewValue		the new value (in pixels or percentage) to use
 * @param	Face			indicates which element of the Value array to modify
 * @param	InputType		indicates the format of the input value
 * @param	bResolveChange	indicates whether a scene update should be requested if NewValue does not match the current value.
 */
void FUIScreenValue_Bounds::SetPositionValue( UUIScreenObject* OwnerWidget, FLOAT NewValue, EUIWidgetFace Face, EPositionEvalType InputType/*=EVALPOS_PixelOwner*/, UBOOL bResolveChange/*=TRUE*/ )
{
	//SCOPE_CYCLE_COUNTER(STAT_UISetPositionValue);
	FLOAT ConvertedValue = NewValue;
	if ( InputType != ScaleType[Face] )
	{
		UUIScreenObject* OwnerWidgetContainer=NULL;
		FLOAT BaseValue, BaseExtent;

		if ( InputType != EVALPOS_PixelViewport )
		{
			// first, convert the input value into absolute pixel values, if necessary
			FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, InputType, BaseValue, BaseExtent);
			ConvertedValue = BaseValue + (BaseExtent * NewValue);
		}

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( ScaleType[Face] != EVALPOS_PixelViewport )
		{
			FUIScreenValue::CalculateBaseValue(OwnerWidget, Face, (EPositionEvalType)ScaleType[Face], BaseValue, BaseExtent);

			ConvertedValue -= BaseValue;
			ConvertedValue /= BaseExtent;
		}
	}

	// check against DELTA to avoid spurious scene updates due to float precision
	if ( Abs(Value[Face] - ConvertedValue) > DELTA && bResolveChange )
	{
		OwnerWidget->RequestSceneUpdate(FALSE,TRUE);
		OwnerWidget->InvalidatePosition(Face);

		Value[Face] = ConvertedValue;

		OwnerWidget->RefreshPosition();
	}
	else
	{
		Value[Face] = ConvertedValue;
	}
}

/**
 * Retrieves the value of the width or height of this widget's bounds.
 *
 * @param	OwnerWidget	the widget that contains this screen value
 * @param	Dimension	determines whether width or height is desired.  Specify UIORIENT_Horizontal to get the width, or UIORIENT_Vertical to get the height.
 * @param	OutputType	determines the format of the result.
 *						EVALPOS_None:
 *							return value is formatted using this screen position's ScaleType for the specified face
 *						EVALPOS_PercentageOwner:
 *						EVALPOS_PercentageScene:
 *						EVALPOS_PercentageViewport:
 *							return a value between 0.0 and 1.0, which represents the percentage of the corresponding
 *							base's actual size.  If OwnerWidget isn't specified, the size of the
 *							entire viewport is used.
 *						EVALPOS_PixelOwner
 *						EVALPOS_PixelScene
 *						EVALPOS_PixelViewport
 *							return the actual pixel values represented by this UIScreenValue, relative to the corresponding base.
 * @param	bIgnoreDockPadding
 *						used to prevent recursion when evaluting docking links
 *
 * @return	the value of the width/height of this UIScreenValue, in pixels or percentage.
 */
FLOAT FUIScreenValue_Bounds::GetBoundsExtent( const UUIScreenObject* OwnerWidget, EUIOrientation Dimension, EPositionEvalType OutputType/*=EVALPOS_PixelOwner*/, UBOOL bIgnoreDockPadding/*=FALSE*/ ) const
{
	FLOAT MinValue = GetPositionValue(OwnerWidget, Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top, EVALPOS_PixelViewport, bIgnoreDockPadding);
	FLOAT MaxValue = GetPositionValue(OwnerWidget, Dimension == UIORIENT_Horizontal ? UIFACE_Right : UIFACE_Bottom, EVALPOS_PixelViewport, bIgnoreDockPadding);

	EUIExtentEvalType EvalType=UIEXTENTEVAL_Pixels;
	switch(OutputType)
	{
	case EVALPOS_PercentageViewport:
		EvalType = UIEXTENTEVAL_PercentViewport;
		break;
	case EVALPOS_PercentageScene:
		EvalType = UIEXTENTEVAL_PercentScene;
		break;
	case EVALPOS_PercentageOwner:
		EvalType = UIEXTENTEVAL_PercentOwner;
	    break;
	}
	return FUIScreenValue_Extent(MaxValue - MinValue, UIEXTENTEVAL_Pixels, Dimension).GetValue(OwnerWidget, EvalType);
}

/**
 * Changes the scale type for the specified face to the value specified, and converts the Value for that face into the new type.
 *
 * @param	Face				indicates which element of the Value array to modify
 * @param	OwnerWidget			the widget that contains this screen value
 * @param	NewEvalType			the evaluation type to set for the specified face
 * @param	bAutoConvertValue	if TRUE, the current value of the position will be converted into the equivalent value for the new type
 */
void FUIScreenValue_Bounds::ChangeScaleType( UUIScreenObject* OwnerWidget, EUIWidgetFace Face, EPositionEvalType NewEvalType, UBOOL bAutoConvertValue/*=TRUE*/ )
{
	if ( OwnerWidget != NULL && Face != UIFACE_MAX && NewEvalType != EVALPOS_MAX )
	{
		if ( bAutoConvertValue )
		{
			const FLOAT AbsolutePixelValue = GetPositionValue(OwnerWidget, Face, EVALPOS_PixelViewport);

			ScaleType[Face] = NewEvalType;
			SetPositionValue(OwnerWidget, AbsolutePixelValue, Face, EVALPOS_PixelViewport, FALSE);
		}
		else
		{
			OwnerWidget->InvalidatePosition(Face);
			ScaleType[Face] = NewEvalType;
			OwnerWidget->RefreshPosition();
		}
	}
}

/**
 * Changes the value for the specified face without performing any conversion.
 *
 * @param	Face			indicates which element of the Value array to modify
 * @param	NewValue		the new value (in pixels or percentage) to use
 * @param	NewScaleType	if specified, modified the ScaleType for this face as well.
 */
void FUIScreenValue_Bounds::SetRawPositionValue( BYTE Face, FLOAT NewValue, EPositionEvalType NewScaleType/*=EVALPOS_None*/ )
{
	checkSlow(Face<UIFACE_MAX);
	Value[Face] = NewValue;
	if ( NewScaleType != EVALPOS_None )
	{
		SetRawScaleType(Face, NewScaleType);
	}
}

/**
 * Changes the ScaleType for the specified face without performing any conversion.
 *
 * @param	Face			indicates which element of the Value array to modify
 * @param	NewScaleType	the new scale type to use.
 */
void FUIScreenValue_Bounds::SetRawScaleType( BYTE Face, EPositionEvalType NewScaleType )
{
	checkSlow(Face<UIFACE_MAX);
	checkSlow(NewScaleType<EVALPOS_MAX);

	ScaleType[Face] = NewScaleType;
}

/**
 * Changes the AspectRatioMode for this screen value.
 *
 * @param	NewAspectRatioMode	the new aspect ratio mode; must be one of the EUIAspectRatioConstraint values.
 */
void FUIScreenValue_Bounds::SetAspectRatioMode( BYTE NewAspectRatioMode )
{
	check(NewAspectRatioMode<UIASPECTRATIO_MAX);
	AspectRatioMode = NewAspectRatioMode;
}

/**
 * Returns whether the Value for the specified face has been modified since that face was last resolved.
 *
 * @param	OwnerWidget		the widget that contains this screen value (NULL if this screen value is for a UIScene)
 * @param	Face			the face to modify; value must be one of the EUIWidgetFace values.
 */
UBOOL FUIScreenValue_Bounds::IsPositionCurrent( const UUIObject* OwnerWidget, EUIWidgetFace Face ) const
{
	checkSlow(Face<UIFACE_MAX);

	UBOOL bResult = TRUE;
	if ( bInvalidated[Face] != 0 )
	{
		bResult = FALSE;
	}
	else if ( OwnerWidget != NULL && OwnerWidget->DockTargets.IsDocked(Face, TRUE) )
	{
		bResult = FALSE;

		UUIObject* DockTarget = OwnerWidget->DockTargets.GetDockTarget(Face);
		EUIWidgetFace TargetFace = OwnerWidget->DockTargets.GetDockFace(Face);
		if ( DockTarget != NULL )
		{
			bResult = DockTarget->Position.IsPositionCurrent(DockTarget, TargetFace);
		}
		else
		{
			const UUIScene* OwnerScene = OwnerWidget->GetScene();
			if ( OwnerScene != NULL )
			{
				bResult = OwnerScene->Position.IsPositionCurrent(NULL, TargetFace);
			}
		}
	}

	return bResult;
}


/* ==========================================================================================================
	FUIRotation
========================================================================================================== */

/**
 * Sets the location of the anchor of rotation for this widget.
 *
 * @param	AnchorPos		New location for the anchor of rotation.
 */
void FUIRotation::SetAnchorLocation(const UUIScreenObject* OwnerWidget, const FVector& AnchorPos, EPositionEvalType InputType/*=EVALPOS_PixelViewport*/)
{
	AnchorPosition.SetValue(OwnerWidget, UIORIENT_Horizontal, AnchorPos.X, InputType);
	AnchorPosition.SetValue(OwnerWidget, UIORIENT_Vertical, AnchorPos.Y, InputType);
}

/* ==========================================================================================================
	FScreenPositionRange
========================================================================================================== */

/**
* Retrieves the value of the distance between the endpoints of this region
*
* @param	Dimension	indicates which element of the Value array to evaluate
* @param	OutputType	determines the format of the result.
*						EVALPOS_None:
*							return value is formatted using this screen position's ScaleType for the specified face
*						EVALPOS_PercentageOwner:	(only valid when OwnerWidget is specified)
*						EVALPOS_PercentageScene:	(only valid when OwnerWidget is specified)
*						EVALPOS_PercentageViewport:
*							return a value between 0.0 and 1.0, which represents the percentage of the corresponding
*							base's actual size.  If OwnerWidget isn't specified, the size of the
*							entire viewport is used.
*						EVALPOS_PixelOwner:			(only valid when OwnerWidget is specified)
*						EVALPOS_PixelScene:			(only valid when OwnerWidget is specified)
*						EVALPOS_PixelViewport
*							return the actual pixel values represented by this UIScreenValue, relative to the corresponding base.
* @param	OwnerWidget	the widget that contains this screen value
*
* @return	the value of the width of this UIScreenValue, in pixels or percentage.
*/
FLOAT FScreenPositionRange::GetRegionValue( EUIOrientation Dimension, EPositionEvalType OutputType, UUIScreenObject* OwnerWidget ) const
{
	EUIWidgetFace Face = Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top;

	//@fixme ronp - this might need to be rewritten...double-check the logic here
	FLOAT MinValue = GetValue( UIORIENT_Horizontal, Face, OutputType, OwnerWidget );
	FLOAT MaxValue = GetValue( UIORIENT_Vertical, UUIRoot::GetOppositeFace( Face ), OutputType, OwnerWidget );

	return MaxValue - MinValue;
}

/** Comparison */
UBOOL FScreenPositionRange::operator ==(const FScreenPositionRange& Other ) const
{
	UBOOL bResult = TRUE;
	for ( INT i = 0;i < UIORIENT_MAX; i++ )
	{
		if ( Value[i] != Other.Value[i] )
		{
			bResult = FALSE;
			break;
		}

		if ( ScaleType[i] != Other.ScaleType[i] )
		{
			bResult = FALSE;
			break;
		}
	}

	return bResult;
}
UBOOL FScreenPositionRange::operator !=(const FScreenPositionRange& Other ) const
{
	return !((*this) == Other);
}

/* ==========================================================================================================
	FUIScreenValue_DockPadding
========================================================================================================== */
/**
 * Calculates the size of the base region used for formatting the padding value of a single widget face
 *
 * @param	OwnerWidget			the widget that owns this padding
 * @param	EvalFace			the face to evaluate
 * @param	EvalType			indicates which type of base value is desired
 * @param	BaseExtent			[out] the base extent for the specified face, in absolute pixel values.  BaseExtent is defined as the size of the widget associated with
 *								the specified dock padding type and face's orientation.
 */
void FUIScreenValue_DockPadding::CalculateBaseExtent( const UUIObject* OwnerWidget, EUIWidgetFace EvalFace, EUIDockPaddingEvalType EvalType, FLOAT& BaseExtent )
{
	checkSlow(OwnerWidget);
	checkSlow(EvalFace<UIFACE_MAX);
	checkSlow(EvalType<UIPADDINGEVAL_MAX);

	BaseExtent = 1.f;

	// first, determine which object the padding value is relative to
	const UUIScreenObject* BaseObject=NULL;

	switch ( EvalType )
	{
	case UIPADDINGEVAL_PercentOwner:
		BaseObject = OwnerWidget;
		break;

	case UIPADDINGEVAL_PercentTarget:
		BaseObject = OwnerWidget->DockTargets.GetDockTarget(EvalFace);
		if ( BaseObject != NULL )
		{
			break;
		}

		// falls through
	case UIPADDINGEVAL_PercentScene:
		BaseObject = OwnerWidget->GetScene();
		break;

	case UIPADDINGEVAL_Pixels:
	case UIPADDINGEVAL_PercentViewport:
		// nothing.
		break;
	}

	EUIOrientation Orientation = UUIRoot::GetFaceOrientation(EvalFace);
	if ( BaseObject != NULL )
	{
		BaseExtent = BaseObject->GetBounds(Orientation, EVALPOS_PixelViewport, TRUE);
	}
	else if ( EvalType != UIPADDINGEVAL_Pixels )
	{
		FVector2D ViewportSize;
		if ( OwnerWidget->GetViewportSize(ViewportSize) )
		{
			BaseExtent = ViewportSize[Orientation];
		}
		else
		{
			BaseExtent = Orientation == UIORIENT_Horizontal ? UCONST_DEFAULT_SIZE_X : UCONST_DEFAULT_SIZE_Y;
		}
	}
}

/**
 * Evaluates the value stored in this UIScreenValue_DockPadding
 *
 * @param	OwnerWidget		the widget that contains this screen value
 * @param	Face			indicates which element of the Value array to evaluate
 * @param	OutputType		indicates the desired format for the result
 *							UIPADDINGEVAL_Pixels:
 *								NewValue is in pixels
 *							UIPADDINGEVAL_PercentTarget:
 *								NewValue is a percentage of the dock target extent in the corresponding orientation
 *							UIPADDINGEVAL_PercentOwner:
 *								NewValue is a percentage of OwnerWidget parent's extent in the corresponding orientation
 *							UIPADDINGEVAL_PercentScene:
 *								NewValue is a percentage of the scene
 *							UIPADDINGEVAL_PercentViewport:
 *								NewValue is a percentage of the viewport.
 *
 * @return	the actual value for this UIScreenValue, in pixels or percentage, for the face specified.
 */
FLOAT FUIScreenValue_DockPadding::GetPaddingValue( const UUIObject* OwnerWidget, EUIWidgetFace Face, EUIDockPaddingEvalType OutputType/*=UIPADDINGEVAL_Pixels*/ ) const
{
	checkSlow(Face<UIFACE_MAX);
	checkSlow(OutputType<UIPADDINGEVAL_MAX);

	FLOAT Result = PaddingValue[Face];
	if ( OwnerWidget == NULL || OutputType == PaddingScaleType[Face] )
		return Result;

	// first, convert the padding value for this face into absolute pixels

	// get the size of base region
	FLOAT BaseExtent;
	CalculateBaseExtent(OwnerWidget, Face, (EUIDockPaddingEvalType)PaddingScaleType[Face], BaseExtent);

	// multiply by the current value to get absolute pixels
	Result *= BaseExtent;

	// now result is in pixels
	if ( OutputType != UIPADDINGEVAL_Pixels )
	{
		CalculateBaseExtent(OwnerWidget, Face, OutputType, BaseExtent);
		Result /= BaseExtent;
	}

	return Result;
}


/**
 * Convert the value specified into the appropriate format and assign the converted value to the Value element for the face specified.
 *
 * @param	OwnerWidget		the widget that contains this screen value
 * @param	NewValue		the new value (in pixels or percentage) to use
 * @param	Face			indicates which element of the Value array to modify
 * @param	InputType		indicates the desired format for the result
 *							UIEXTENTEVAL_Pixels:
 *								NewValue is in pixels
 *							UIEXTENTEVAL_PercentOwner:
 *								NewValue is a percentage of OwnerWidget parent's extent in the corresponding orientation
 *							UIEXTENTEVAL_PercentScene:
 *								NewValue is a percentage of the scene
 *							UIEXTENTEVAL_PercentViewport:
 *								NewValue is a percentage of the viewport.
 * @param	bResolveChange	indicates whether a scene update should be requested if NewValue does not match the current value.
 */
void FUIScreenValue_DockPadding::SetPaddingValue( UUIObject* OwnerWidget, FLOAT NewValue, EUIWidgetFace Face, EUIDockPaddingEvalType InputType/*=EVALPOS_PixelOwner*/, UBOOL bResolveChange/*=TRUE*/ )
{
	checkSlow(Face<UIFACE_MAX);
	checkSlow(InputType<UIPADDINGEVAL_MAX);

	FLOAT ConvertedValue = NewValue;
	if ( InputType != PaddingScaleType[Face] || OwnerWidget == NULL )
	{
		// first, convert the input value into absolute pixel values, if necessary
		FLOAT BaseExtent;
		CalculateBaseExtent(OwnerWidget, Face, InputType, BaseExtent);

		ConvertedValue *= BaseExtent;

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( PaddingScaleType[Face] != UIPADDINGEVAL_Pixels )
		{
			CalculateBaseExtent(OwnerWidget, Face, (EUIDockPaddingEvalType)PaddingScaleType[Face], BaseExtent);
			ConvertedValue /= BaseExtent;
		}
	}

	if ( PaddingValue[Face] != ConvertedValue && bResolveChange )
	{
		OwnerWidget->RequestSceneUpdate(FALSE,TRUE);
	}

	PaddingValue[Face] = ConvertedValue;
}

/**
 * Changes the scale type for the specified face to the value specified, optionally converting the Value for that face into the new type.
 *
 * @param	OwnerWidget			the widget that contains this screen value
 * @param	Face				indicates which element of the Value array to modify
 * @param	NewEvalType			the evaluation type to set for the specified face
 * @param	bAutoConvertValue	if TRUE, the current value of the position will be converted into the equivalent value for the new type
 */
void FUIScreenValue_DockPadding::ChangePaddingScaleType( UUIObject* OwnerWidget, EUIWidgetFace Face, EUIDockPaddingEvalType NewEvalType, UBOOL bAutoConvertValue/*=TRUE*/ )
{
	if ( OwnerWidget != NULL && Face != UIFACE_MAX && NewEvalType < UIPADDINGEVAL_MAX )
	{
		if ( bAutoConvertValue )
		{
			const FLOAT AbsolutePixelValue = GetPaddingValue(OwnerWidget, Face, UIPADDINGEVAL_Pixels);

			PaddingScaleType[Face] = NewEvalType;
			SetPaddingValue(OwnerWidget, AbsolutePixelValue, Face, UIPADDINGEVAL_Pixels, FALSE);
		}
		else
		{
// 			OwnerWidget->InvalidatePosition(Face);
			PaddingScaleType[Face] = NewEvalType;
		}
	}
}

/* ==========================================================================================================
	FUIScreenValue_AutoSizeRegion
========================================================================================================== */
/**
 * Calculates the extent to use as the base for evaluating percentage values.
 *
 * @param	Orientation		indicates which orientation to use for evaluating the actual extent of the widget's parent
 * @param	EvaluationType	indicates which base to use for calculating the base extent
 * @param	OwnerWidget		the widget that this auto-size region is for
 * @param	BaseExtent		[out] set to the size of the region that will be used for evaluating this auto-size region as a percentage; actual pixels
 */
void FUIScreenValue_AutoSizeRegion::CalculateBaseValue( EUIOrientation Orientation, EUIExtentEvalType EvaluationType, UUIScreenObject* OwnerWidget, FLOAT& BaseExtent )
{
	checkSlow(Orientation<UIORIENT_MAX);
	checkSlow(EvaluationType<UIEXTENTEVAL_MAX);
	checkSlow(OwnerWidget);

	UUIScreenObject* BaseWidget = OwnerWidget;

	// if calculating as a percentage of the viewport, the base extent should be the size of the viewport
	if ( EvaluationType == UIEXTENTEVAL_PercentViewport )
	{
		FVector2D ViewportSize;
		if ( OwnerWidget->GetViewportSize(ViewportSize) )
		{
			BaseExtent = Orientation == UIORIENT_Horizontal ? ViewportSize.X : ViewportSize.Y;
		}
		else
		{
			BaseExtent = Orientation == UIORIENT_Horizontal ? UCONST_DEFAULT_SIZE_X : UCONST_DEFAULT_SIZE_Y;
		}
	}
	else
	{
		// otherwise, the base extent should be the size of the parent widget for the appropriate orientation (width for horizontal, height for vertical)
		if ( EvaluationType != UIEXTENTEVAL_PercentSelf )
		{
			BaseWidget = OwnerWidget->GetParent();
		}
		if ( BaseWidget == NULL || EvaluationType == UIEXTENTEVAL_PercentScene )
		{
			BaseWidget = OwnerWidget->GetScene();
		}

		checkSlow(BaseWidget);
		BaseExtent = BaseWidget->GetBounds(Orientation, EVALPOS_PixelViewport);
	}
}

/**
 * Resolves the value stored in this AutoSizeRegion according to the specified output type.
 *
 * @param	ValueType	indicates whether to return the min or max value.
 * @param	Orientation	indicates which orientation to use for e.g. evaluting values as percentage of the owning widget's parent
 * @param	OutputType	indicates the desired format for the result
 *						UIEXTENTEVAL_Pixels:
 *							Result should be the actual number of pixels
 *						UIEXTENTEVAL_PercentOwner:
 *							result should be formatted as a percentage of the widget's parent
 *						UIEXTENTEVAL_PercentScene:
 *							result should be formatted as a percentage of the scene
 *						UIEXTENTEVAL_PercentViewport:
 *							result should be formatted as a percentage of the viewport
 * @param	OwnerWidget	the widget that this auto-size region is for
 *
 * @return	the value of the auto-size region's min or max value
 */
FLOAT FUIScreenValue_AutoSizeRegion::GetValue( EUIAutoSizeConstraintType ValueType, EUIOrientation Orientation, EUIExtentEvalType OutputType, UUIScreenObject* OwnerWidget ) const
{
	checkSlow(ValueType<UIAUTOSIZEREGION_MAX);
	checkSlow(OutputType<UIEXTENTEVAL_MAX);
	checkSlow(Orientation<UIORIENT_MAX);
	checkSlow(OwnerWidget);

	FLOAT Result = Value[ValueType];

	if ( Result > 0 && EvalType[ValueType] != OutputType )
	{
		FLOAT BaseExtent = 0.f;

		// first, convert the value into pixels
		if ( EvalType[ValueType] != UIEXTENTEVAL_Pixels )
		{
			CalculateBaseValue(Orientation, static_cast<EUIExtentEvalType>(EvalType[ValueType]), OwnerWidget, BaseExtent);
			Result *= BaseExtent;
		}

		// then, convert the value into the expected output type if it isn't pixels
		if ( OutputType != UIEXTENTEVAL_Pixels )
		{
			CalculateBaseValue(Orientation, OutputType, OwnerWidget, BaseExtent);
			Result /= BaseExtent;
		}
	}

	return Result;
}

/**
 * Convert the input value into the appropriate type for this UIScreenValue, and assign that Value
 *
 * @param	ValueType	indicates whether to set the min or max value.
 * @param	Orientation	indicates which orientation to use for e.g. evaluting values as percentage of the owning widget's parent
 * @param	OwnerWidget	the widget that contains this extent value
 * @param	NewValue	the new value (in pixels or percentage) to use
 * @param	OutputType	specifies how NewValue should be interpreted format for the result
 *						UIEXTENTEVAL_Pixels:
 *							NewValue is in absolute pixels
 *						UIEXTENTEVAL_PercentOwner:
 *							NewValue is a percentage of the OwnerWidget
 *						UIEXTENTEVAL_PercentScene:
 *							NewValue is a percentage of the scene
 *						UIEXTENTEVAL_PercentViewport:
 *							NewValue is a percentage of the viewport
 */
void FUIScreenValue_AutoSizeRegion::SetValue( EUIAutoSizeConstraintType ValueType, EUIOrientation Orientation, UUIScreenObject* OwnerWidget, FLOAT NewValue, EUIExtentEvalType InputType/*=UIEXTENTEVAL_Pixels*/ )
{
	checkSlow(ValueType<UIAUTOSIZEREGION_MAX);
	checkSlow(InputType<UIEXTENTEVAL_MAX);
	checkSlow(Orientation<UIORIENT_MAX);
	checkSlow(OwnerWidget);

	FLOAT ConvertedValue = NewValue;

	if ( InputType != EvalType[ValueType] )
	{
		// first, convert the input value into absolute pixel values, if necessary
		// first, convert the value into pixels
		if ( InputType != UIEXTENTEVAL_Pixels )
		{
			FLOAT BaseExtent;
			CalculateBaseValue(Orientation, InputType, OwnerWidget, BaseExtent);

			ConvertedValue *= BaseExtent;
		}

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( EvalType[ValueType] != UIEXTENTEVAL_Pixels )
		{
			FLOAT BaseExtent;
			CalculateBaseValue(Orientation, static_cast<EUIExtentEvalType>(EvalType[ValueType]), OwnerWidget, BaseExtent);
			ConvertedValue /= BaseExtent;
		}
	}

	Value[ValueType] = ConvertedValue;
}

/**
 * Changes the scale type for this extent to the type specified, optionally converting the current Value into the new type.
 *
 * @param	ValueType			indicates whether to set the min or max value.
 * @param	Orientation			indicates which orientation to use for e.g. evaluting values as percentage of the owning widget's parent
 * @param	OwnerWidget			the widget that contains this screen value
 * @param	NewEvalType			the new evaluation type to ise
 * @param	bAutoConvertValue	if TRUE, the current value of the position will be converted into the equivalent value for the new type
 */
void FUIScreenValue_AutoSizeRegion::ChangeScaleType( EUIAutoSizeConstraintType ValueType, EUIOrientation Orientation, UUIScreenObject* OwnerWidget, EUIExtentEvalType NewEvalType, UBOOL bAutoConvertValue/*=TRUE*/ )
{
	if ( OwnerWidget != NULL && NewEvalType < UIEXTENTEVAL_MAX )
	{
		if ( bAutoConvertValue )
		{
			const FLOAT AbsolutePixelValue = GetValue(ValueType, Orientation, UIEXTENTEVAL_Pixels, OwnerWidget);

			EvalType[ValueType] = NewEvalType;
			SetValue(ValueType, Orientation, OwnerWidget, AbsolutePixelValue, UIEXTENTEVAL_Pixels);
		}
		else
		{
			EvalType[ValueType] = NewEvalType;
		}
	}
}

/* ==========================================================================================================
	FUIScreenValue_Extent
========================================================================================================== */
/**
 * Calculates the extent to use as the base for evaluating percentage values.
 *
 * @param	OwnerWidget		the widget that contains this extent value
 * @param	EvaluationType	indicates which base to use for calculating the base extent
 * @param	BaseExtent		[out] set to the size of the region that will be used for evaluating this extent as a percentage; actual pixels
 */
void FUIScreenValue_Extent::CalculateBaseExtent( const UUIScreenObject* OwnerWidget, EUIExtentEvalType EvalType, FLOAT& BaseExtent ) const
{
	checkSlow(Orientation<UIORIENT_MAX);
	checkSlow(EvalType<UIEXTENTEVAL_MAX);
	checkSlow(OwnerWidget);

	BaseExtent = 1.f;

	// if calculating as a percentage of the viewport, the base extent should be the size of the viewport
	if ( EvalType == UIEXTENTEVAL_PercentViewport )
	{
		FVector2D ViewportSize;
		if ( OwnerWidget->GetViewportSize(ViewportSize) )
		{
			BaseExtent = Orientation == UIORIENT_Horizontal ? ViewportSize.X : ViewportSize.Y;
		}
		else
		{
			BaseExtent = Orientation == UIORIENT_Horizontal ? UCONST_DEFAULT_SIZE_X : UCONST_DEFAULT_SIZE_Y;
		}
	}
	else
	{
		// otherwise, the base extent should be the size of the parent widget for the appropriate orientation (width for horizontal, height for vertical)
		const UUIScreenObject* BaseWidget = EvalType == UIEXTENTEVAL_PercentSelf ? OwnerWidget : OwnerWidget->GetParent();
		if ( BaseWidget == NULL || EvalType == UIEXTENTEVAL_PercentScene )
		{
			BaseWidget = OwnerWidget->GetScene();
		}

		checkSlow(BaseWidget);
		BaseExtent = BaseWidget->GetBounds(Orientation, EVALPOS_PixelViewport);
	}
}

/**
 * Resolves the value stored in this extent according to the specified output type.
 *
 * @param	OwnerWidget		the widget that contains this extent value
 * @param	OutputType	indicates the desired format for the result
 *						UIEXTENTEVAL_Pixels:
 *							Result should be the actual number of pixels
 *						UIEXTENTEVAL_PercentOwner:
 *							result should be formatted as a percentage of the widget's parent
 *						UIEXTENTEVAL_PercentScene:
 *							result should be formatted as a percentage of the scene
 *						UIEXTENTEVAL_PercentViewport:
 *							result should be formatted as a percentage of the viewport
 *
 * @return	the value of the auto-size region's min or max value
 */
FLOAT FUIScreenValue_Extent::GetValue( const UUIScreenObject* OwnerWidget, EUIExtentEvalType OutputType/*=UIEXTENTEVAL_Pixels*/ ) const
{
	checkSlow(OutputType<UIEXTENTEVAL_MAX);

	FLOAT Result = Value;
	if ( OwnerWidget == NULL || OutputType == ScaleType )
	{
		return Result;
	}

	if ( Result != 0 && ScaleType != OutputType )
	{
		FLOAT BaseExtent;

		// first, convert the value into pixels
		if ( ScaleType != UIEXTENTEVAL_Pixels )
		{
			// get the size of the base region
			CalculateBaseExtent(OwnerWidget, static_cast<EUIExtentEvalType>(ScaleType), BaseExtent);

			// multiple by Value to convert to absolute pixels
			Result *= BaseExtent;
		}

		// at this point, Result is in absolute pixels

		// then, convert the value into the expected output type if it isn't pixels
		if ( OutputType != UIEXTENTEVAL_Pixels )
		{
			CalculateBaseExtent(OwnerWidget, OutputType, BaseExtent);
			Result /= BaseExtent;
		}
	}

	return Result;
}

/**
 * Convert the input value into the appropriate type for this UIScreenValue, and assign that Value
 *
 * @param	OwnerWidget	the widget that contains this extent value
 * @param	NewValue	the new value (in pixels or percentage) to use
 * @param	OutputType	specifies how NewValue should be interpreted format for the result
 *						UIEXTENTEVAL_Pixels:
 *							NewValue is in absolute pixels
 *						UIEXTENTEVAL_PercentOwner:
 *							NewValue is a percentage of the OwnerWidget
 *						UIEXTENTEVAL_PercentScene:
 *							NewValue is a percentage of the scene
 *						UIEXTENTEVAL_PercentViewport:
 *							NewValue is a percentage of the viewport
 */
void FUIScreenValue_Extent::SetValue( UUIScreenObject* OwnerWidget, FLOAT NewValue, EUIExtentEvalType InputType/*=UIEXTENTEVAL_Pixels*/ )
{
	checkSlow(InputType<UIEXTENTEVAL_MAX);

	FLOAT ConvertedValue = NewValue;

	if ( InputType != ScaleType && OwnerWidget != NULL )
	{
		// first, convert the input value into absolute pixel values, if necessary
		FLOAT BaseExtent;
		if ( InputType != UIEXTENTEVAL_Pixels )
		{
			CalculateBaseExtent(OwnerWidget, InputType, BaseExtent);

			ConvertedValue *= BaseExtent;
		}

		// next, if the ScaleType for this face isn't in absolute pixels, translate the value into that format
		if ( ScaleType != UIEXTENTEVAL_Pixels )
		{
			CalculateBaseExtent(OwnerWidget, static_cast<EUIExtentEvalType>(ScaleType), BaseExtent);
			ConvertedValue /= BaseExtent;
		}
	}

	Value = ConvertedValue;
}

/**
 * Changes the scale type for this extent to the type specified, optionally converting the current Value into the new type.
 *
 * @param	OwnerWidget			the widget that contains this screen value
 * @param	NewEvalType			the new evaluation type to ise
 * @param	bAutoConvertValue	if TRUE, the current value of the position will be converted into the equivalent value for the new type
 */
void FUIScreenValue_Extent::ChangeScaleType( UUIScreenObject* OwnerWidget, EUIExtentEvalType NewEvalType, UBOOL bAutoConvertValue/*=TRUE*/ )
{
	if ( OwnerWidget != NULL && NewEvalType < UIEXTENTEVAL_MAX )
	{
		if ( bAutoConvertValue )
		{
			const FLOAT AbsolutePixelValue = GetValue(OwnerWidget, UIEXTENTEVAL_Pixels);

			ScaleType = NewEvalType;
			SetValue(OwnerWidget, AbsolutePixelValue, UIEXTENTEVAL_Pixels);
		}
		else
		{
			ScaleType = NewEvalType;
		}
	}
}

/* ==========================================================================================================
	FAutoSizeData
========================================================================================================== */
/**
 * Evaluates and returns the padding value stored in this AutoSizeData
 *
 * @param	ValueType	indicates which element of the Value array to evaluate
 * @param	Orientation	indicates which orientation to use for e.g. evaluting values as percentage of the owning widget's parent
 * @param	OutputType	specifies how the result should be formatted
 *						UIEXTENTEVAL_Pixels:
 *							NewValue is in absolute pixels
 *						UIEXTENTEVAL_PercentOwner:
 *							NewValue is a percentage of the OwnerWidget
 *						UIEXTENTEVAL_PercentScene:
 *							NewValue is a percentage of the scene
 *						UIEXTENTEVAL_PercentViewport:
 *							NewValue is a percentage of the viewport
 * @param	OwnerWidget		the widget that contains this screen value
 *
 * @return	the actual padding value for this AutoSizeData, in pixels or percentage, for the dimension specified.
 */
FLOAT FAutoSizeData::GetPaddingValue( EUIAutoSizeConstraintType ValueType, EUIOrientation Orientation, EUIExtentEvalType OutputType, class UUIScreenObject* OwnerWidget ) const
{
	return bAutoSizeEnabled ? Padding.GetValue( ValueType, Orientation, OutputType, OwnerWidget ) : 0.0f;
}

/**
 * Returns the minimum allowed size for this auto-size region.
 *
 * @param	OutputType		indicates how the result should be formatted.
 * @param	Orientation		indicates which axis this auto-size region is associated with on the owner widget.
 * @param	OwnerWidget		the widget that this auto-size region is used by.
 *
 * @return	the minimum size allowed for this auto-size region, or 0 if this auto-size region is disabled.
 */
FLOAT FAutoSizeData::GetMinValue( EUIExtentEvalType OutputType, EUIOrientation Orientation, UUIScreenObject* OwnerWidget ) const
{
	FLOAT Result = 0.f;

	if ( bAutoSizeEnabled )
	{
		Result = Extent.GetValue(UIAUTOSIZEREGION_Minimum, Orientation, OutputType, OwnerWidget);
	}

	return Result;
}

/**
 * Returns the maximum allowed size for this auto-size region.
 *
 * @param	OutputType		indicates how the result should be formatted.
 * @param	Orientation		indicates which axis this auto-size region is associated with on the owner widget.
 * @param	OwnerWidget		the widget that this auto-size region is used by.
 *
 * @return	the maximum size allowed for this auto-size region, or 0 if there is no max size configured or this auto-size region
 *			is not enabled.
 */
FLOAT FAutoSizeData::GetMaxValue( EUIExtentEvalType OutputType, EUIOrientation Orientation, UUIScreenObject* OwnerWidget ) const
{
	FLOAT Result = 0.f;

	if ( bAutoSizeEnabled )
	{
		Result = Extent.GetValue(UIAUTOSIZEREGION_Maximum, Orientation, OutputType, OwnerWidget);
	}

	return Result;
}

/* ==========================================================================================================
	FProtectedBufferPixels
========================================================================================================== */

/* ==========================================================================================================
	FUIDockingSet
========================================================================================================== */
/**
 * Evaluate the widget's Position into an absolute pixel value, and store that value in the corresponding
 * member of the widget's RenderBounds array.
 * This function assumes that UpdateDockingSet has already been called for the TargetFace of the TargetWidget.
 * This function should only be called from ResolveScenePositions.
 *
 * @param	Face			the face that needs to be resolved
 */
void FUIDockingSet::UpdateDockingSet( EUIWidgetFace Face )
{
	//SCOPE_CYCLE_COUNTER(STAT_UIResolvePosition);
	checkSlow(Face<UIFACE_MAX);
	checkSlow(OwnerWidget);

	const UBOOL bShouldResolvePosition = !OwnerWidget->Position.IsPositionCurrent(NULL, Face);
	if ( !IsResolved(Face) || bShouldResolvePosition )
	{
		const EUIWidgetFace OppositeFace = UUIRoot::GetOppositeFace(Face);
		const EUIWidgetFace DockTargetFace = (EUIWidgetFace)TargetFace[Face];
		UBOOL bPositionAdjusted = FALSE;

		if ( bShouldResolvePosition )
		{
			FLOAT TargetValue;
			if ( IsDocked(Face) )
			{
				UUIObject* DockTarget = GetDockTarget(Face);
				if ( DockTarget != NULL )
				{
					TargetValue = DockTarget->GetPosition( DockTargetFace, EVALPOS_PixelViewport, TRUE );
				}
				else
				{
					TargetValue = OwnerWidget->OwnerScene->GetPosition(DockTargetFace, EVALPOS_PixelViewport, TRUE);
				}

				if ( (Face % UIORIENT_MAX) != (DockTargetFace % UIORIENT_MAX) )
				{
					//@todo - docking to a perpendicular face
				}
				else
				{
					// docking to a parallel face

					// add in the dock padding
					TargetValue += GetDockPadding(Face);
				}
				FUIScreenValue_Bounds& OwnerPosition = OwnerWidget->Position;

				// TargetValue will include the viewport origin offset, but the viewport origin offset will be considered when this face's Position is evaluated
				// into actual pixels, so subtract it from the value we assign to the Position value for this face
				FVector2D ViewportOrigin;
				if ( !OwnerWidget->GetViewportOrigin(ViewportOrigin) )
				{
					ViewportOrigin.X = ViewportOrigin.Y = 0;
				}

				FLOAT PositionValue = TargetValue - ViewportOrigin[Face % UIORIENT_MAX];

				// set this widget's position to the evaluated position; if the widget's height or width is locked we also need to adjust
				// the resolved value of the opposite face by the same amount so that the width/height remains constant
				if ( !IsDocked(OppositeFace) )
				{
					const EUIOrientation Orientation = UUIRoot::GetFaceOrientation(Face);

					if ( (Face == UIFACE_Left && IsWidthLocked()) || (Face == UIFACE_Top && IsHeightLocked()) )
					{
						FLOAT Extent = OwnerPosition.GetBoundsExtent(OwnerWidget, Orientation, EVALPOS_PixelViewport);
						// updating the position of the right face
						if ( Abs(Extent - OwnerPosition.GetPositionValue(OwnerWidget, OppositeFace, EVALPOS_PixelOwner)) > DELTA || OwnerPosition.GetScaleType(OppositeFace) != EVALPOS_PixelOwner )
						{
							bPositionAdjusted = TRUE;
							OwnerWidget->InvalidatePosition(OppositeFace);
						}
						OwnerWidget->Position.SetPositionValue(OwnerWidget, Extent, OppositeFace, EVALPOS_PixelOwner, FALSE);
					}
					else if ( (Face == UIFACE_Right && IsWidthLocked()) || (Face == UIFACE_Bottom && IsHeightLocked()) )
					{
						FLOAT Extent;

						//@fixme ronp - this doesn't work correctly if the scale type for the left/top is PixelOwner and the OwnerWidget->Owner changes the position
						// of that face; what happens is the value calculated for Extent is based on the new position of the parent widget's left/top face, not on the previous
						// position, so we don't actually lock the width/height at all.  To get the correct result in this case, we'd need to set Extent to
						// RenderBounds[Face] - RenderBounds[OppositeFace];  only problem is this might not work correctly the first frame...., hmmm.
						if ( OwnerWidget->RenderBounds[Face] != 0.f || OwnerWidget->RenderBounds[OppositeFace] != 0.f )
						{
							Extent = OwnerWidget->RenderBounds[Face] - OwnerWidget->RenderBounds[OppositeFace];
						}
						else
						{
							Extent = PositionValue - OwnerWidget->Position.GetPositionValue(OwnerWidget, OppositeFace, EVALPOS_PixelViewport);
						}

						const FLOAT RelativeValue = PositionValue - Extent;

						// updating the position of the left face
						if ( Abs(RelativeValue - OwnerPosition.GetPositionValue(OwnerWidget, OppositeFace, EVALPOS_PixelViewport)) > DELTA )
						{
							bPositionAdjusted = TRUE;
							OwnerWidget->InvalidatePosition(OppositeFace);
						}
						OwnerWidget->Position.SetPositionValue(OwnerWidget, RelativeValue, OppositeFace, EVALPOS_PixelViewport, FALSE);
					}
					else
					{
						OwnerWidget->Position.SetRawPositionValue(Face, PositionValue, EVALPOS_PixelViewport);
					}
				}
				else
				{
					OwnerWidget->Position.SetRawPositionValue(Face, PositionValue, EVALPOS_PixelViewport);
				}
			}
			else
			{
				// no docking - evaluate normally
				TargetValue = OwnerWidget->GetPosition(Face, EVALPOS_PixelViewport, TRUE);
			}

			OwnerWidget->RenderBounds[Face] = TargetValue;
		}

		MarkResolved(Face);
		OwnerWidget->Position.ValidatePosition(Face);

		// if the Position value for the face we adjusted has already been resolved during this update, we need to re-update the corresponding
		// RenderBounds value so that we don't require a full scene update.
		if ( bPositionAdjusted && OwnerWidget->HasPositionBeenResolved(OppositeFace) )
		{
			MarkResolved(OppositeFace,0);
			OwnerWidget->Position.InvalidatePosition(OppositeFace);
			UpdateDockingSet(OppositeFace);
		}
	}
}

/**
 * Retrieves the target widget for the specified face in this docking set.
 *
 * @param	SourceFace		the face to retrieve the dock target for
 *
 * @return	a pointer to the widget that the specified face is docked to.  NULL if the face is not docked or is docked to the scene.
 *			If return value is NULL, IsDocked() can be used to determine whether the face is docked to the scene or not.
 */
UUIObject* FUIDockingSet::GetDockTarget( EUIWidgetFace SourceFace ) const
{
	checkSlow(SourceFace<UIFACE_MAX);

	UUIObject* Result = NULL;

	// if TargetWidget is pointing to the OwnerWidget, it means that this face is docked to the scene; in that case, return NULL
	if ( IsDocked(SourceFace, FALSE) && TargetWidget[SourceFace] != OwnerWidget )
	{
		Result = TargetWidget[SourceFace];
	}

	return Result;
}

/**
 * Retrieves the target face for the specified source face in this docking set.
 *
 * @param	SourceFace		the face to retrieve the dock target face for
 *
 * @return	the face of the dock target that SourceFace is docked to, or UIFACE_MAX if SourceFace is not docked.
 */
EUIWidgetFace FUIDockingSet::GetDockFace( EUIWidgetFace SourceFace ) const
{
	checkSlow(SourceFace<UIFACE_MAX);
	EUIWidgetFace Result = UIFACE_MAX;

	//@todo ronp - hmmm, there might be code that relies on GetDockFace returning UIFACE_MAX if IsDocked returns FALSE;
	// if so, that code needs to be fixed because this method needs to return the docked face regardless of whether IsDocked returns TRUE
	//if ( IsDocked(SourceFace, FALSE) )
	{
		Result = (EUIWidgetFace)TargetFace[SourceFace];
	}

	return Result;
}


/**
 *	Returns the ammount of padding for the specified face.
 */
FLOAT FUIDockingSet::GetDockPadding( EUIWidgetFace SourceFace, EUIDockPaddingEvalType OutputType/*=UIPADDINGEVAL_Pixels*/ ) const
{
	checkSlow(SourceFace<UIFACE_MAX);
	return DockPadding.GetPaddingValue(OwnerWidget, SourceFace, OutputType);
}


/**
 * Returns the dock padding eval type for the specified face.
 */
EUIDockPaddingEvalType FUIDockingSet::GetDockPaddingType( EUIWidgetFace SourceFace ) const
{
	checkSlow(SourceFace<UIFACE_MAX);
	return DockPadding.GetPaddingScaleType(SourceFace);
}

/**
 * Changes the configured dock target and face for the specified face.
 *
 * @param	SourceFace	the face to set the value for
 * @param	DockTarget	the widget that SourceFace should be docked to, or NULL to indicate that this face should no longer be docked.
 * @param	DockFace	the face on the dock target that SourceFace should be docked to.
 *
 * @return	TRUE indicates that the dock target values for the specified face were successfully changed.
 */
UBOOL FUIDockingSet::SetDockTarget( EUIWidgetFace SourceFace, UUIScreenObject* DockTarget, EUIWidgetFace DockFace )
{
	UBOOL bResult = FALSE;

	if ( SourceFace < UIFACE_MAX )
	{
		if ( DockTarget != NULL )
		{
			// if DockTarget is a UIScene, TargetWidget[SourceFace] should be set to OwnerWidget
			UUIObject* DockWidget = Cast<UUIObject>(DockTarget);
			if ( DockWidget != NULL )
			{
                TargetWidget[SourceFace] = DockWidget;
			}
			else
			{
				// if DockTarget is a UIScene, TargetWidget[SourceFace] should be set to OwnerWidget
				TargetWidget[SourceFace] = OwnerWidget;
			}

			TargetFace[SourceFace] = DockFace;
			bResult = TRUE;
		}
		else
		{
			TargetWidget[SourceFace] = NULL;
			TargetFace[SourceFace] = DockFace;
			bResult = TRUE;
		}
	}
	else
	{
		// invalid SourceFace specified
	}

	return bResult;
}

/**
 * Changes the dock padding value for the specified face.
 *
 * @param	SourceFace			the face to change padding for
 * @param	NewValue			the new value to use for padding
 * @param	InputType			the format to use for interpreting NewValue.
 * @param	bChangeScaleType	specify TRUE to permanently change the scale type for the specified face to InputType.
 *
 * @return	TRUE indicates that the dock padding values for the specified face were successfully changed.
 */
UBOOL FUIDockingSet::SetDockPadding( EUIWidgetFace SourceFace, float NewValue, EUIDockPaddingEvalType InputType/*=UIPADDINGEVAL_Pixels*/, UBOOL bChangeScaleType/*=FALSE*/ )
{
	UBOOL bResult = FALSE;

	if ( OwnerWidget != NULL && SourceFace < UIFACE_MAX && InputType < UIPADDINGEVAL_MAX )
	{
		if ( bChangeScaleType )
		{
			DockPadding.ChangePaddingScaleType(OwnerWidget, SourceFace, InputType, FALSE);
		}

		DockPadding.SetPaddingValue(OwnerWidget, NewValue, SourceFace, InputType);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Initializes the value of this docking set's OwnerWidget and convert UIDockingSets over to the new behavior
 * (where TargetFace == OwnerWidget if docked to the scene)
 *
 * @param	inOwnerWidget	the widget that contains this docking set.
 */
void FUIDockingSet::InitializeDockingSet( UUIObject* inOwnerWidget )
{
	check(inOwnerWidget);
	OwnerWidget = inOwnerWidget;
}

/* ==========================================================================================================
	FPlayerInteractionData
========================================================================================================== */
/**
 * Changes the FocusedControl to the widget specified
 *
 * @param	NewFocusedControl	the widget that should become the focused control
 */
void FPlayerInteractionData::SetFocusedControl( UUIObject* NewFocusedControl )
{
	FocusedControl = NewFocusedControl;

	// when a widget loses focus, it calls SetFocusedControl(NULL) to clear its FocusedControl; we store this widget
	// as the LastFocusedControl so that the next time the parent widget receives focus, we re-focus the child that previously
	// had focus
	if ( NewFocusedControl != NULL )
	{
		SetLastFocusedControl(NewFocusedControl);
	}
}

/**
 * Gets the currently focused control.
 */
UUIObject* FPlayerInteractionData::GetFocusedControl() const
{
	return FocusedControl;
}

/**
 * Changes the FocusedControl to the widget specified
 *
 * @param	Widget	the widget that should become the LastFocusedControl control
 */
void FPlayerInteractionData::SetLastFocusedControl( UUIObject* Widget )
{
	LastFocusedControl = Widget;
}

/**
 * Gets the previously focused control.
 */
UUIObject* FPlayerInteractionData::GetLastFocusedControl() const
{
	return LastFocusedControl;
}

/* ==========================================================================================================
	FUIFocusPropagationData
========================================================================================================== */
/**
 * Returns the child widget that is configured as the first focus target for this widget.
 */
UUIObject* FUIFocusPropagationData::GetFirstFocusTarget() const
{
	return FirstFocusTarget;
}

/**
 * Returns the child widget that is configured as the last focus target for this widget.
 */
UUIObject* FUIFocusPropagationData::GetLastFocusTarget() const
{
	return LastFocusTarget;
}

/**
 * Returns the sibling widget that is configured as the next focus target for tab navigation.
 */
UUIObject* FUIFocusPropagationData::GetNextFocusTarget() const
{
	return NextFocusTarget;
}

/**
 * Returns the sibling widget that is configured as the previous focus target for tab navigation.
 */
UUIObject* FUIFocusPropagationData::GetPrevFocusTarget() const
{
	return PrevFocusTarget;
}

/**
 * Sets the default first focus target for this widget.
 *
 * @param	FocusTarget			the child of this widget that should become the first focus target for this widget
 *
 * @return	TRUE if the navigation link for the specified face changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUIFocusPropagationData::SetFirstFocusTarget( UUIObject* FocusTarget )
{
	UBOOL bNavLinkChanged = FirstFocusTarget != FocusTarget;
	FirstFocusTarget = FocusTarget;

	return bNavLinkChanged;
}

/**
 * Sets the default last focus target for this widget.
 *
 * @param	FocusTarget			the child of this widget that should become the last focus target for this widget.
 *
 * @return	TRUE if the focus target changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUIFocusPropagationData::SetLastFocusTarget( UUIObject* FocusTarget )
{
	UBOOL bNavLinkChanged = FocusTarget != LastFocusTarget;
	LastFocusTarget = FocusTarget;
	return bNavLinkChanged;
}

/**
 * Sets the next tab-nav focus target for this widget.
 *
 * @param	FocusTarget			a sibling of this widget that should become the next tab-nav target for this widget.
 *
 * @return	TRUE if the focus target changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUIFocusPropagationData::SetNextFocusTarget( UUIObject* FocusTarget )
{
	UBOOL bNavLinkChanged = FocusTarget != NextFocusTarget;
	NextFocusTarget = FocusTarget;
	return bNavLinkChanged;
}

/**
 * Sets the previous tab-nav focus target for this widget.
 *
 * @param	FocusTarget			a sibling of this widget that should become the previous tab-nav target for this widget.
 *
 * @return	TRUE if the focus target changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUIFocusPropagationData::SetPrevFocusTarget( UUIObject* FocusTarget )
{
	UBOOL bNavLinkChanged = FocusTarget != PrevFocusTarget;
	PrevFocusTarget = FocusTarget;
	return bNavLinkChanged;
}

/* ==========================================================================================================
	FUINavigationData
========================================================================================================== */
/**
 * Sets the actual navigation target for the specified face.
 *
 * @param	Face			the face to set the navigation link for
 * @param	NewNavTarget	the widget to set as the link for the specified face
 *
 * @return	TRUE if the navigation link for the specified face changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUINavigationData::SetNavigationTarget( EUIWidgetFace Face, UUIObject* NewNavTarget )
{
	UBOOL bNavLinkChanged = FALSE;
	if ( Face < UIFACE_MAX )
	{
		// Make sure the widget we are setting as the nav target can accept focus, otherwise we could loose scene focus entirely.
		if ( NewNavTarget == NULL || !NewNavTarget->IsPrivateBehaviorSet( UCONST_PRIVATE_NotFocusable ) )
		{
			bNavLinkChanged = (NavigationTarget[Face] != NewNavTarget);
			NavigationTarget[Face] = NewNavTarget;
		}
	}

	return bNavLinkChanged;
}
UBOOL FUINavigationData::SetNavigationTarget( UUIObject* LeftTarget, UUIObject* TopTarget, UUIObject* RightTarget, UUIObject* BottomTarget )
{
	UBOOL bNavLinksChanged =
		NavigationTarget[UIFACE_Left]	!= LeftTarget	||
        NavigationTarget[UIFACE_Top]	!= TopTarget	||
        NavigationTarget[UIFACE_Right]	!= RightTarget	||
        NavigationTarget[UIFACE_Bottom]	!= BottomTarget;

	NavigationTarget[UIFACE_Left]		= LeftTarget;
	NavigationTarget[UIFACE_Top]		= TopTarget;
	NavigationTarget[UIFACE_Right]		= RightTarget;
	NavigationTarget[UIFACE_Bottom]		= BottomTarget;

	return bNavLinksChanged;
}

/**
 * Sets the designer-specified navigation target for the specified face.  When navigation links for the scene are rebuilt,
 * the designer-specified navigation target will always override any auto-calculated targets.
 *
 * @param	Face				the face to set the navigation link for
 * @param	NavTarget			the widget to set as the link for the specified face
 * @param	bIsNullOverride		if NavTarget is NULL, specify TRUE to indicate that this face's nav target should not
 *								be automatically calculated.
 *
 * @return	TRUE if the navigation link for the specified face changed.  FALSE if the new value wasn't applied or if the
 *			the new value was the same as the current value.
 */
UBOOL FUINavigationData::SetForcedNavigationTarget( EUIWidgetFace Face, UUIObject* NavTarget, UBOOL bIsNullOverride/*=FALSE*/ )
{
	UBOOL bNavLinkChanged = FALSE;
	if ( Face != UIFACE_MAX )
	{
		// bIsNullOverride is only relevant when NavTarget is NULL
		if ( NavTarget != NULL )
		{
			bIsNullOverride = FALSE;
		}

		bNavLinkChanged = (ForcedNavigationTarget[Face] != NavTarget) || (bNullOverride[Face] != bIsNullOverride);
		ForcedNavigationTarget[Face] = NavTarget;
		bNullOverride[Face] = bIsNullOverride;
	}

	return bNavLinkChanged;
}

UBOOL FUINavigationData::SetForcedNavigationTarget( UUIObject* LeftTarget, UUIObject* TopTarget, UUIObject* RightTarget, UUIObject* BottomTarget )
{
	UBOOL bNavLinksChanged =
		ForcedNavigationTarget[UIFACE_Left]		!= LeftTarget	||
		ForcedNavigationTarget[UIFACE_Top]		!= TopTarget	||
		ForcedNavigationTarget[UIFACE_Right]	!= RightTarget	||
		ForcedNavigationTarget[UIFACE_Bottom]	!= BottomTarget;

	ForcedNavigationTarget[UIFACE_Left]			= LeftTarget;
	ForcedNavigationTarget[UIFACE_Top]			= TopTarget;
	ForcedNavigationTarget[UIFACE_Right]		= RightTarget;
	ForcedNavigationTarget[UIFACE_Bottom]		= BottomTarget;

	return bNavLinksChanged;
}

/**
 * Gets the navigation target for the specified face.  If a designer-specified nav target is set for the specified face,
 * that object is returned.
 *
 * @param	Face		the face to get the nav target for
 * @param	LinkType	specifies which navigation link type to return.
 *							NAVLINK_MAX: 		return the designer specified navigation target, if set; otherwise returns the auto-generated navigation target
 *							NAVLINK_Automatic:	return the auto-generated navigation target, even if the designer specified nav target is set
 *							NAVLINK_Manual:		return the designer specified nav target, even if it isn't set
 *
 * @return	a pointer to a widget that will be the navigation target for the specified direction, or NULL if there is
 *			no nav target for that face.
 */
UUIObject* FUINavigationData::GetNavigationTarget( EUIWidgetFace Face, ENavigationLinkType LinkType/*=NAVLINK_MAX*/ ) const
{
	UUIObject* NavTarget = NULL;
	if ( Face != UIFACE_MAX )
	{
		switch ( LinkType )
		{
		case NAVLINK_Automatic:
			NavTarget = NavigationTarget[Face];
			break;

		case NAVLINK_Manual:
			NavTarget = ForcedNavigationTarget[Face];
			break;

		case NAVLINK_MAX:
			NavTarget = ForcedNavigationTarget[Face];
			if ( NavTarget == NULL && bNullOverride[Face] == 0 )
			{
				NavTarget = NavigationTarget[Face];
			}
			break;
		}
	}

	return NavTarget;
}

/**
 * Determines whether the designer has overriden all navigation targets.
 *
 * @return	FALSE if an override target has been specified for all faces.
 */
UBOOL FUINavigationData::NeedsLinkGeneration() const
{
	UBOOL bResult = FALSE;
	for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
	{
		if ( ForcedNavigationTarget[FaceIndex] == NULL )
		{
			bResult = TRUE;
			break;
		}
	}

	return bResult;
}

/* ==========================================================================================================
	UUITexture statics
========================================================================================================== */
/**
 * Modifies the parameters for rendering a single dimension of an image according to the criteria specified in an image style.
 *
 * @param	Orientation	the orientation dimension to process
 * @param	Surface		the material being rendered
 * @param	StyleData	the style used for rendering the image
 * @param	Parameters	the bounding region where the image will be rendered
 * @param	out_Result	[out] filled with the appropriate rendering parameters for the style specified
 */
static void AdjustRenderParameter( EUIOrientation Orientation, USurface* Surface, const FUICombinedStyleData& StyleData, const FRenderParameters& Parameters, FRenderParameters& out_Result )
{
	checkSlow(Orientation<UIORIENT_MAX);

	// the size of the texture that is being rendered
	FLOAT SurfaceSize = 0.f;
	if ( Orientation == UIORIENT_Horizontal )
	{
		SurfaceSize = Parameters.ImageExtent.X > 0
			? Parameters.ImageExtent.X
			: Surface->GetSurfaceWidth();
	}
	else
	{
		SurfaceSize = Parameters.ImageExtent.Y > 0
			? Parameters.ImageExtent.Y
			: Surface->GetSurfaceHeight();
	}

	// get the starting position of the bounding region that will be used when rendering the image
	const FLOAT& BoundingRegionPos = Orientation == UIORIENT_Horizontal ? Parameters.DrawX : Parameters.DrawY;

	// get the size of the bounding region to use for rendering this image
	const FLOAT& BoundingRegionSize = Orientation == UIORIENT_Horizontal ? Parameters.DrawXL : Parameters.DrawYL;

	// get the location where we'll start rendering the image
	FLOAT& DrawLocation = Orientation == UIORIENT_Horizontal ? out_Result.DrawX : out_Result.DrawY;

	// get the size of the area that the image should be rendered to
	FLOAT& DrawSize = Orientation == UIORIENT_Horizontal ? out_Result.DrawXL : out_Result.DrawYL;

	// get the pixel location in the source texture to start sampling
	FLOAT& SamplePos = Orientation == UIORIENT_Horizontal ? out_Result.DrawCoords.U : out_Result.DrawCoords.V;

	// get the size (in pixels) of the area we'll be sampling from the source texture
	FLOAT& SampleSize = Orientation == UIORIENT_Horizontal ? out_Result.DrawCoords.UL : out_Result.DrawCoords.VL;
	if ( SampleSize == 0 )
	{
		SampleSize = SurfaceSize;
	}

	switch ( StyleData.AdjustmentType[Orientation].AdjustmentType )
	{
	/** no modification to material - if material is larger than target dimension, material is clipped */
	case ADJUST_None:	//	in editor: Clipped
		{
			// get the difference between the size of the texture and the bounding region
			FLOAT Remainder = Abs<FLOAT>(SurfaceSize - BoundingRegionSize);

			// if the source texture is larger than the area we want to render to, clip the image by adjusting the sampling positions
			if ( SurfaceSize > BoundingRegionSize )
			{
				SampleSize *= (BoundingRegionSize / SurfaceSize);

				// determine where we should start drawing the image, based on alignment
				switch ( StyleData.AdjustmentType[Orientation].Alignment )
				{
				case UIALIGN_Left:
					// do nothing
					break;

				case UIALIGN_Center:
					SamplePos += Remainder * 0.5f;
					break;

				case UIALIGN_Right:
					SamplePos += Remainder;
					break;
				}
			}
			else
			{
				// don't scale the image if it's smaller than the target dimension
				DrawSize = SurfaceSize;

				// determine where we should start drawing the image, based on alignment
				switch ( StyleData.AdjustmentType[Orientation].Alignment )
				{
				case UIALIGN_Left:
					// do nothing
					break;

				case UIALIGN_Center:
					DrawLocation += Remainder * 0.5f;
					break;

				case UIALIGN_Right:
					DrawLocation += Remainder;
					break;
				}
			}
		}
		break;

	/** target's dimensions will be adjusted to match material dimension */
	case ADJUST_Bound:	// in editor: bound
		{
			DrawSize = SurfaceSize;

			FLOAT Remainder = BoundingRegionSize - SurfaceSize;

			// determine where we should start drawing the image, based on alignment
			switch ( StyleData.AdjustmentType[Orientation].Alignment )
			{
			case UIALIGN_Left:
				// do nothing
				break;

			case UIALIGN_Center:
				DrawLocation += Remainder * 0.5f;
				break;

			case UIALIGN_Right:
				DrawLocation += Remainder;
				break;
			}
		}
		// falls through

	/** material will be scaled to fit the target dimension */
	case ADJUST_Normal:	// in editor: scaled

		break;

	/** material will be scaled to fit the target dimension, maintaining aspect ratio */
	case ADJUST_Justified:	// in editor: Uniformly Scaled
		if ( Orientation == UIORIENT_Horizontal )
		{
			FLOAT VerticalSurfaceSize = Parameters.ImageExtent.Y > 0
				? Parameters.ImageExtent.Y
				: Surface->GetSurfaceHeight();

			FLOAT& VerticalDrawLocation = out_Result.DrawY;
			FLOAT& VerticalDrawSize = out_Result.DrawYL;

			const FLOAT SurfaceAspectRatio = VerticalSurfaceSize / SurfaceSize;
			const FLOAT DrawRegionAspectRatio = VerticalDrawSize / DrawSize;

			if ( SurfaceAspectRatio > DrawRegionAspectRatio )
			{
				// the height of the image will be scaled to match the height of the target region; scale the width proportionately
				DrawSize *= DrawRegionAspectRatio / SurfaceAspectRatio;

				// get the difference between the size of the texture and the bounding region
				const FLOAT Remainder = Abs<FLOAT>(DrawSize - BoundingRegionSize);

				// determine where we should start drawing the image, based on alignment
				switch ( StyleData.AdjustmentType[UIORIENT_Horizontal].Alignment )
				{
				case UIALIGN_Left:
					// do nothing
					break;

				case UIALIGN_Center:
					DrawLocation += Remainder * 0.5f;
					break;

				case UIALIGN_Right:
					DrawLocation += Remainder;
					break;
				}
			}
			else if ( DrawRegionAspectRatio > SurfaceAspectRatio )
			{
				// the width of the image will be scaled to match the width of the target region; scale the height proportionately
				VerticalDrawSize *= SurfaceAspectRatio / DrawRegionAspectRatio;

				// get the difference between the size of the texture and the bounding region
				const FLOAT Remainder = Abs<FLOAT>(VerticalDrawSize - Parameters.DrawYL);

				// determine where we should start drawing the image, based on alignment
				switch ( StyleData.AdjustmentType[UIORIENT_Vertical].Alignment )
				{
				case UIALIGN_Left:
					// do nothing
					break;

				case UIALIGN_Center:
					VerticalDrawLocation += Remainder * 0.5f;
					break;

				case UIALIGN_Right:
					VerticalDrawLocation += Remainder;
					break;
				}
			}
		}

		break;

	/** material will be stretched to fit target dimension */
	case ADJUST_Stretch:	// in editor: stretched
		break;
	}
}

/**
 * Modifies the parameters for rendering an image according to the criteria specified in an image style.
 *
 * @param	Surface		the material being rendered
 * @param	StyleData	the style used for rendering the image
 * @param	Parameters	the bounding region where the image will be rendered
 * @param	out_Result	[out] filled with the appropriate rendering parameters for the style specified
 */
static void AdjustRenderParameters( USurface* Surface, const FUICombinedStyleData& StyleData, const FRenderParameters& Parameters, FRenderParameters& out_Result )
{
	checkSlow(Surface);
	checkSlow(StyleData.IsInitialized());

	AdjustRenderParameter(UIORIENT_Horizontal, Surface, StyleData, Parameters, out_Result);
	AdjustRenderParameter(UIORIENT_Vertical, Surface, StyleData, Parameters, out_Result);
}

/**
 * Utility function for rendering a texture to the screen using DrawTile.  Determines the appropriate overload of DrawTile to call,
 * depending on the type of surface being rendered, and translates the UV values into percentages as this is what DrawTile expects.
 *
 * @param	Canvas			the FCanvas to use for rendering the image
 * @param	Surface			the texture to be rendered
 * @param	StyleData		this is [currently] only used when rendering UTexture surfaces to get the color to use for rendering
 * @param	X				the horizontal screen location (in pixels) where the image should be rendered
 * @param	Y				the vertical screen location (in pixels) where the image should be rendered
 * @param	XL				the width of the region (in pixels) where this image will be rendered
 * @param	YL				the height of the region (in pixels) where this image will be rendered
 * @param	U				the horizontal location (in pixels) to begin sampling the texture data
 * @param	V				the vertical location (in pixels) to begin sampling the texture data
 * @param	UL				the width (in pixels) of the texture data sampling region
 * @param	VL				the height (in pixels) of the texture data sampling region
 */
void UUITexture::DrawTile( FCanvas* Canvas, USurface* Surface, const FUICombinedStyleData& StyleData,
					 FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL )
{
	FLOAT mW = Surface->GetSurfaceWidth();
	FLOAT mH = Surface->GetSurfaceHeight();
	U /= mW; UL /= mW;
	V /= mH; VL /= mH;

	UTexture* Texture = Cast<UTexture>(Surface);
	if ( Texture != NULL )
	{
		FTexture* RawTexture = Texture->Resource;
		::DrawTile(Canvas, X, Y, XL, YL, U, V, UL, VL, StyleData.ImageColor, RawTexture);
	}
	else
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(Surface);
		if ( Material != NULL )
		{
			// try to find the name to use for setting the opacity parameter in the material instance
			FName OpacityParameterName = NAME_None;
			UMaterialInstanceConstant* OpacityMIC = NULL;

			UUISceneClient* SceneClient = GetSceneClient();
			if ( SceneClient == NULL && GIsEditor )
			{
				UUIInteraction* DefaultUIController = GetDefaultUIController();
				if ( DefaultUIController != NULL && DefaultUIController->SceneClientClass != NULL )
				{
					SceneClient = DefaultUIController->SceneClientClass->GetDefaultObject<UUISceneClient>();
					if( SceneClient->OpacityParameter == NULL )
					{
						SceneClient->OpacityParameter = ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass() );

						SceneClient->OpacityParameter->ScalarParameterValues.Empty();
						SceneClient->OpacityParameter->ScalarParameterValues.AddZeroed( 1 );
						SceneClient->OpacityParameter->ScalarParameterValues(0).ParameterName	= OpacityParameterName;
						SceneClient->OpacityParameter->ScalarParameterValues(0).ParameterValue	= 1.f;
					}
				}
			}

			if ( SceneClient != NULL )
			{
				OpacityParameterName = SceneClient->OpacityParameterName;
				OpacityMIC = SceneClient->OpacityParameter;
			}

			if ( OpacityParameterName != NAME_None && OpacityMIC != NULL
			&&	(!ARE_FLOATS_EQUAL(StyleData.ImageColor.A, 1.f) || !ARE_FLOATS_EQUAL(Canvas->AlphaModulate,1.f)) )
			{
				OpacityMIC->SetScalarParameterValue(OpacityParameterName, Canvas->AlphaModulate * StyleData.ImageColor.A);
				OpacityMIC->SetParent(Material);

				::DrawTile(Canvas, X, Y, XL, YL, U, V, UL, VL, OpacityMIC->GetRenderProxy(0));

				// Material instance parent gets reset so can't keep a batch to render.
				//@todo sz - needs to be fixed to work with canvas sorting
				Canvas->Flush();
				OpacityMIC->SetParent(NULL);
			}
			else
			{
				::DrawTile(Canvas, X, Y, XL, YL, U, V, UL, VL, Material->GetRenderProxy(0));
			}
		}
	}
}

/**
 * Render the specified image.  The protected regions of the image will not be scaled in the directions of their repective dimensions,
 * i.e. the left protected region will not be scaled in the horizontal dimension with the rest of the image.  The protected regions are
 * defined by a value that indicates the distance from their respective face to the opposite edge of the region.  The perpendicular
 * faces of a given protected region extent to the edges of the full image.
 *
 * @param	RI				the render interface to use for rendering the image
 * @param	Surface			the texture to be rendered
 * @param	StyleData		used to determine which orientations can be stretched in the image
 * @param	Parameters		describes the bounds and sample locations for rendering this tile.  See the documentation for DrawTile
 *							for more details about each individual member.
 */
void UUITexture::DrawTileProtectedRegions( FCanvas* Canvas, USurface* Surface, const FUICombinedStyleData& StyleData, const FRenderParameters& Parameters )
{
// 	const FLOAT GutterPixels[UIFACE_MAX] =
// 	{
// 		StyleData.AdjustmentType[UIORIENT_Horizontal].ProtectedRegion[0].GetValue(this),
// 		StyleData.AdjustmentType[UIORIENT_Vertical].ProtectedRegion[0].GetValue(this),
// 		StyleData.AdjustmentType[UIORIENT_Horizontal].ProtectedRegion[1].GetValue(this),
// 		StyleData.AdjustmentType[UIORIENT_Vertical].ProtectedRegion[1].GetValue(this)
// 	};
// 
// 	FLOAT& X = Parameters.DrawX;
// 	FLOAT& Y = Parameters.DrawY;
// 	FLOAT& XL = Parameters.DrawXL;
// 	FLOAT& YL = Parameters.DrawYL;
// 	FLOAT& U = Parameters.DrawCoords.U;
// 	FLOAT& V = Parameters.DrawCoords.V;
// 	FLOAT& UL = Parameters.DrawCoords.UL;
// 	FLOAT& VL = Parameters.DrawCoords.VL;
// 
// 	FUICombinedStyleData AlternateStyleData = StyleData;
// 	StyleData.AdjustmentType[UIORIENT_Horizontal].AdjustmentType = ADJUST_None;
// 	StyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType = ADJUST_None;
// 
// 	// draw the left gutter portion
// 	DrawTile(Canvas, Surface, AlternateStyleData, X, Y, GutterPixels[UIFACE_Left], YL, U, V, GutterPixels[UIFACE_Left], VL);
// 
// 	// draw the right gutter portion
// 	DrawTile(Canvas, Surface, AlternateStyleData, X + XL - GutterPixels[UIFACE_Left], YL, U + UL - GutterPixels[UIFACE_Left], VL);
// 
// 	// draw the top gutter portion
// 	DrawTile(Canvas, Surface, AlternateStyleData, X, Y, XL, GutterPixels[UIFACE_Top], U, V, UL, GutterPixels[UIFACE_Top]);
// 
// 	// draw the bottom gutter portion
// 	DrawTile(Canvas, Surface, AlternateStyleData, X, Y + YL - GutterPixels[UIFACE_Bottom], XL, GutterPixels[UIFACE_Bottom], U, V + VL - GutterPixels[UIFACE_Bottom], UL, GutterPixels[UIFACE_Bottom]);
// 
// 	// draw the middle
// 	DrawTile(Canvas, Surface, StyleData, X + GutterPixels[UIFACE_Left], Y + GutterPixels[UIFACE_Top], XL - (GutterPixels[UIFACE_Left] + GutterPixels[UIFACE_Right]), YL - (GutterPixels[UIFACE_Top] + GutterPixels[UIFACE_Bottom]),
// 		U + GutterPixels[UIFACE_Left], V + GutterPixels[UIFACE_Top], UL - (GutterPixels[UIFACE_Left] + GutterPixels[UIFACE_Right]), VL - (GutterPixels[UIFACE_Top] + GutterPixels[UIFACE_Bottom]));
}

/**
 * Render the specified image.  If the target region is larger than the image being rendered, the image is stretched by duplicating the pixels at the
 * images midpoint to fill the additional space.  If the target region is smaller than the image being rendered, the image will be scaled to fit the region.
 *
 * @param	RI				the render interface to use for rendering the image
 * @param	Surface			the texture to be rendered
 * @param	StyleData		used to determine which orientations can be stretched in the image
 * @param	Parameters		describes the bounds and sample locations for rendering this tile.  See the documentation for DrawTile
 *							for more details about each individual member.
 */
void UUITexture::DrawTileStretched( FCanvas* Canvas, USurface* Surface, const FUICombinedStyleData& StyleData, const FRenderParameters& Parameters )
{
	FLOAT SurfaceWidth = Surface->GetSurfaceWidth();
	FLOAT SurfaceHeight = Surface->GetSurfaceHeight();

	UTexture* Texture = Cast<UTexture>(Surface);
	UMaterialInterface* Material = Cast<UMaterialInterface>(Surface);

	FLOAT X		= Parameters.DrawX;
	FLOAT Y		= Parameters.DrawY;
	FLOAT SizeX	= Parameters.DrawXL;
	FLOAT SizeY	= Parameters.DrawYL;
	FLOAT mU	= Parameters.DrawCoords.U;
	FLOAT mV	= Parameters.DrawCoords.V;
	FLOAT SizeU	= Parameters.DrawCoords.UL;
	FLOAT SizeV	= Parameters.DrawCoords.VL;

	// Get the size of the image
	FLOAT mW = SizeU;
	FLOAT mH = SizeV;

	// Get the midpoints of the image
	FLOAT MidX = mW * 0.5f;
	FLOAT MidY = mH * 0.5f;

	// Grab info about the scaled image
	FLOAT AbsWidth = Abs<FLOAT>(SizeX);
	FLOAT AbsHeight = Abs<FLOAT>(SizeY);

	// this represents the size of the region that will be filled
	FLOAT FillerTileW = mW > 0 ? AbsWidth - mW : AbsWidth + mW; //appCopySign(AbsWidth - mW, SizeX);
	FLOAT FillerTileH = mH > 0 ? AbsHeight - mH : AbsWidth + mH; //appCopySign(AbsHeight - mH, SizeY);

	FLOAT AbsTileW, AbsTileY;	// absolute values of TileW and TileY; these are calculated using known non-negative values and then copysign is used to
	FLOAT TileW, TileY;			// set the signed values here. AbsTileW is always used when refering into the image and TileW to refer to the screen.


	UBOOL bStretchHorizontally	= mW < AbsWidth && StyleData.AdjustmentType[UIORIENT_Horizontal].AdjustmentType == ADJUST_Stretch;
	UBOOL bStretchVertically	= mH < AbsHeight && StyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType == ADJUST_Stretch;
	FLOAT MaterialTileW = MidX, MaterialTileH = MidY;
	// Draw the spans first - these are the sections of the image that are stretched to fill the gap between the corners of the region

	// Top and Bottom
	if ( bStretchHorizontally )
	{
		// Need to stretch material horizontally
		AbsTileW = MidX;
		if ( !bStretchVertically )
		{
			// if we're not stretching the vertical orientation, we should scale it
			AbsTileY = AbsHeight * 0.5f;
		}
		else
		{
			AbsTileY = MidY;
		}

		TileW = appCopySign(AbsTileW, SizeX);
		TileY = appCopySign(AbsTileY, SizeY);

		// draw the upper middle stretched portion of the image
		DrawTile(Canvas,Surface,StyleData,X + TileW,	Y,				FillerTileW,		TileY,		mU+MidX,	mV,						1,		MaterialTileH);

		// draw the lower middle stretched portion of the image
		DrawTile(Canvas,Surface,StyleData,X + TileW,	Y+SizeY-TileY,	FillerTileW,		TileY,		mU+MidX,	mV+mH-MaterialTileH,	1,		MaterialTileH);
	}
	else
	{
		AbsTileW = AbsWidth * 0.5f;
	}

	// Left and Right
	if ( bStretchVertically )
	{
		// Need to stretch material vertically
		AbsTileY = MidY;
		if ( !bStretchHorizontally )
		{
			// if we're not stretching the horizontal orientation, scale it
			AbsTileW = AbsWidth * 0.5f;
		}
		else
		{
			AbsTileW = MidX;
		}

		TileW = appCopySign(AbsTileW, SizeX);
		TileY = appCopySign(AbsTileY, SizeY);

		// draw the left middle stretched portion of the image
		DrawTile(Canvas,Surface,StyleData,X,				Y+TileY,	TileW,	FillerTileH,	mU,						mV+AbsTileY, MaterialTileW,	1);

		// draw the right middle stretched portion of the image
		DrawTile(Canvas,Surface,StyleData,X+SizeX-TileW,	Y+TileY,	TileW,	FillerTileH,	mU+mW-MaterialTileW,	mV+AbsTileY, MaterialTileW,	1);
	}
	else
	{
		AbsTileY = AbsHeight * 0.5f;
	}

	// Center
	TileW = appCopySign(AbsTileW, SizeX);
	TileY = appCopySign(AbsTileY, SizeY);

	// If we had to stretch the material both ways, repeat the middle pixels of the image to fill the gap
	if ( bStretchHorizontally && bStretchVertically )
	{
		DrawTile(Canvas,Surface,StyleData,X+TileW, Y+TileY, FillerTileW, FillerTileH, mU+MaterialTileW, mV+MaterialTileH, 1, 1);
	}

	// Draw the 4 corners - each quadrant is scaled if its area is smaller than the destination area
	DrawTile(Canvas,Surface,StyleData,X,				Y,				TileW, TileY,	mU,						mV,						MaterialTileW, MaterialTileH);		//topleft
	DrawTile(Canvas,Surface,StyleData,X+SizeX-TileW,	Y,				TileW, TileY,	mU+mW-MaterialTileW,	mV,						MaterialTileW, MaterialTileH);		//topright
	DrawTile(Canvas,Surface,StyleData,X,				Y+SizeY-TileY,	TileW, TileY,	mU,						mV+mH-MaterialTileH,	MaterialTileW, MaterialTileH);		//bottomleft
	DrawTile(Canvas,Surface,StyleData,X+SizeX-TileW,	Y+SizeY-TileY,	TileW, TileY,	mU+mW-MaterialTileW,	mV+mH-MaterialTileH,	MaterialTileW, MaterialTileH);		//bottomright
}

/* ==========================================================================================================
	UUITexture
========================================================================================================== */
/**
 * Render this image using the parameters specified.
 *
 * @param	Canvas		the FCanvas to use for rendering this widget
 * @param	Parameters	the bounds for the region that this texture can render to.
 */
void UUITexture::Render_Texture( FCanvas* Canvas, const FRenderParameters& Parameters )
{
	UBOOL bUsingFallbackTexture = FALSE;

	USurface* Image = GetSurface();
	if ( Image == NULL && ImageStyleData.IsInitialized() )
	{
		// if we don't have an image specifically assigned to this UITexture, use the style's fallback texture instead
		Image = ImageStyleData.FallbackImage;
		bUsingFallbackTexture = TRUE;
	}

	if ( Image != NULL && ImageStyleData.IsInitialized() )
	{
		FRenderParameters AdjustedParameters = Parameters;

		// if no coordinates were specified on the instance, fallback to the coordinates specified in the style if we're also using the fallback texture
		if ( AdjustedParameters.DrawCoords.IsZero() && bUsingFallbackTexture )
		{
			AdjustedParameters.DrawCoords = ImageStyleData.AtlasCoords;
		}

		// if the scale mode is justified, ensure that there aren't any conflicting parameters
		{
			if ( ImageStyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType == ADJUST_Justified )
			{
				ImageStyleData.AdjustmentType[UIORIENT_Horizontal].AdjustmentType = ADJUST_Justified;
				ImageStyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType = ADJUST_Normal;
			}
			else if ( ImageStyleData.AdjustmentType[UIORIENT_Horizontal].AdjustmentType == ADJUST_Justified )
			{
				ImageStyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType = ADJUST_Normal;
			}

			AdjustRenderParameters(Image, ImageStyleData, Parameters, AdjustedParameters);

			// now apply any padding from the style
			AdjustedParameters.DrawX += ImageStyleData.ImagePadding[UIORIENT_Horizontal];
			AdjustedParameters.DrawY += ImageStyleData.ImagePadding[UIORIENT_Vertical];
			AdjustedParameters.DrawXL -= ImageStyleData.ImagePadding[UIORIENT_Horizontal] * 2;
			AdjustedParameters.DrawYL -= ImageStyleData.ImagePadding[UIORIENT_Vertical] * 2;
		}

/*
		if( Abs(ImageStyleData.AdjustmentType[UIORIENT_Horizontal].ProtectedRegion.Value[0]) > KINDA_SMALL_NUMBER ||
			Abs(ImageStyleData.AdjustmentType[UIORIENT_Horizontal].ProtectedRegion.Value[1]) > KINDA_SMALL_NUMBER ||
			Abs(ImageStyleData.AdjustmentType[UIORIENT_Vertical].ProtectedRegion.Value[0]) > KINDA_SMALL_NUMBER ||
			Abs(ImageStyleData.AdjustmentType[UIORIENT_Vertical].ProtectedRegion.Value[1]) > KINDA_SMALL_NUMBER )
		{
			// One or more of the protected regions for this style is defined.
			DrawTileProtectedRegions( Canvas, Image, ImageStyleData, AdjustedParameters );
		}
		else 
*/
		if (
			ImageStyleData.AdjustmentType[UIORIENT_Horizontal].AdjustmentType == ADJUST_Stretch
		||	ImageStyleData.AdjustmentType[UIORIENT_Vertical].AdjustmentType == ADJUST_Stretch )
		{
			DrawTileStretched( Canvas, Image, ImageStyleData, AdjustedParameters );
		}
		else
		{
			DrawTile(
				Canvas, Image, ImageStyleData,
				AdjustedParameters.DrawX, AdjustedParameters.DrawY,
				AdjustedParameters.DrawXL, AdjustedParameters.DrawYL,
				AdjustedParameters.DrawCoords.U, AdjustedParameters.DrawCoords.V,
				AdjustedParameters.DrawCoords.UL, AdjustedParameters.DrawCoords.VL
				);
		}
	}
}

/**
 * Fills in the extent with the size of this UITexture's source texture
 *
 * @param	Extent	set to the width/height of this UITexture's source texture
 */
void UUITexture::CalculateExtent( FVector2D& Extent ) const
{
	CalculateExtent(Extent.X, Extent.Y);
}

/**
 * Fills in the extent with the size of this texture's material
 *
 * @param	SizeX	[out] filled in with the width this texture's material
 * @param	SizeY	[out] filled in with the height of this texture's material
 */
void UUITexture::CalculateExtent( FLOAT& out_SizeX, FLOAT& out_SizeY ) const
{
	UBOOL bSuccess = FALSE;
	if ( ImageTexture != NULL )
	{
		UTexture* Texture = Cast<UTexture>(ImageTexture);
		if ( Texture != NULL )
		{
			out_SizeX = Texture->GetSurfaceWidth();
			out_SizeY = Texture->GetSurfaceHeight();
			bSuccess = TRUE;
		}
		else
		{
			UMaterialInterface* Material = Cast<UMaterialInterface>(ImageTexture);
			if ( Material != NULL )
			{
				out_SizeX = Material->GetSurfaceWidth();
				out_SizeY = Material->GetSurfaceHeight();
				bSuccess = TRUE;
			}
		}
	}

	// if we haven't calculated an extent by now, use the default image
	if ( !bSuccess && ImageStyleData.IsInitialized() && ImageStyleData.FallbackImage != NULL )
	{
		out_SizeX = ImageStyleData.FallbackImage->GetSurfaceWidth();
		out_SizeY = ImageStyleData.FallbackImage->GetSurfaceHeight();
	}
}

/**
 * Initializes ImageStyleData using the specified image style.
 *
 * @param	NewImageStyle	the image style to copy values from
 */
void UUITexture::SetImageStyle( UUIStyle_Image* NewImageStyle )
{
	if ( NewImageStyle != NULL )
	{
		ImageStyleData.InitializeStyleDataContainer(NewImageStyle);
	}
}

/**
 * Copies the style data specified to this UITexture's ImageStyleData.
 */
void UUITexture::SetImageStyle( const FUICombinedStyleData& NewStyleData )
{
	ImageStyleData = NewStyleData;
}

/**
 * Determines whether this UITexture has been assigned style data.
 *
 * @return	TRUE if ImageStyleData has been initialized; FALSE otherwise
 */
UBOOL UUITexture::HasValidStyleData() const
{
	return ImageStyleData.IsInitialized();
}

/**
 * Wrapper for retrieving the widget that owns this UITexture, if it's owned by a widget.
 */
UUIScreenObject* UUITexture::GetOwnerWidget( UUIComponent** OwnerComponent/*=NULL*/ ) const
{
	UUIScreenObject* WidgetOwner = NULL;
	UUIComponent* UIComponentOwner = NULL;
	for ( UObject* NextOuter = GetOuter(); NextOuter; NextOuter = NextOuter->GetOuter() )
	{
		if ( NextOuter->IsA(UUIComponent::StaticClass()) )
		{
			UIComponentOwner = CastChecked<UUIComponent>(NextOuter);
		}
		else if ( NextOuter->IsA(UUIScreenObject::StaticClass()) )
		{
			WidgetOwner = CastChecked<UUIScreenObject>(NextOuter);
			break;
		}
	}

	if ( OwnerComponent != NULL )
	{
		*OwnerComponent = UIComponentOwner;
	}

	return WidgetOwner;
}

/* === UObject interface === */
/**
 * Determines whether this object is contained within a UIPrefab.
 *
 * @param	OwnerPrefab		if specified, receives a pointer to the owning prefab.
 *
 * @return	TRUE if this object is contained within a UIPrefab; FALSE if this object IS a UIPrefab or is not
 *			contained within a UIPrefab.
 */
UBOOL UUITexture::IsAPrefabArchetype( UObject** OwnerPrefab/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;

	UUIScreenObject* WidgetOwner = NULL;
	UUIComponent* UIComponentOwner = NULL;
	for ( UObject* NextOuter = GetOuter(); NextOuter; NextOuter = NextOuter->GetOuter() )
	{
		if ( NextOuter->IsA(UUIComponent::StaticClass()) )
		{
			UIComponentOwner = CastChecked<UUIComponent>(NextOuter);
			break;
		}
		else if ( NextOuter->IsA(UUIScreenObject::StaticClass()) )
		{
			WidgetOwner = CastChecked<UUIScreenObject>(NextOuter);
			break;
		}
	}

	if ( UIComponentOwner != NULL )
	{
		bResult = UIComponentOwner->IsAPrefabArchetype(OwnerPrefab);
	}
	else if ( WidgetOwner != NULL )
	{
		bResult = WidgetOwner->IsAPrefabArchetype(OwnerPrefab);
	}
	else
	{
		bResult = Super::IsAPrefabArchetype(OwnerPrefab);
	}

	return bResult;
}

/**
 * @return	TRUE if the object is contained within a UIPrefabInstance.
 */
UBOOL UUITexture::IsInPrefabInstance() const
{
	UBOOL bResult = FALSE;

	UUIComponent* UIComponentOwner = NULL;
	UUIScreenObject* WidgetOwner = GetOwnerWidget(&UIComponentOwner);

	if ( UIComponentOwner != NULL )
	{
		bResult = UIComponentOwner->IsInPrefabInstance();
	}
	else if ( WidgetOwner != NULL )
	{
		bResult = WidgetOwner->IsInPrefabInstance();
	}
	else
	{
		bResult = Super::IsInPrefabInstance();
	}
	return bResult;
}

/* ==========================================================================================================
	FUIImageAdjustmentData
========================================================================================================== */
/** Comparison */
UBOOL FUIImageAdjustmentData::operator ==( const FUIImageAdjustmentData& Other ) const
{
	return	ProtectedRegion[UIORIENT_Horizontal] == Other.ProtectedRegion[UIORIENT_Horizontal]
		&&	ProtectedRegion[UIORIENT_Vertical] == Other.ProtectedRegion[UIORIENT_Vertical]
		&&	AdjustmentType == Other.AdjustmentType
		&&	Alignment == Other.Alignment;
}
UBOOL FUIImageAdjustmentData::operator !=( const FUIImageAdjustmentData& Other ) const
{
	return !((*this) == Other);
}

/* ==========================================================================================================
	FInputKeyAction
========================================================================================================== */
/** Copy constructor */
FInputKeyAction::FInputKeyAction( const FInputKeyAction& Other )
: InputKeyName(Other.InputKeyName), InputKeyState(Other.InputKeyState), TriggeredOps(Other.TriggeredOps)
{
	appMemzero(&ActionsToExecute, sizeof(ActionsToExecute));
}
UBOOL FInputKeyAction::operator ==( const FInputKeyAction& Other ) const
{
	return
		Other.InputKeyName == InputKeyName &&
		Other.InputKeyState == InputKeyState;
}

/** Serialization operator */
FArchive& operator<<(FArchive& Ar,FInputKeyAction& MyInputKeyAction)
{
	Ar << MyInputKeyAction.InputKeyName << MyInputKeyAction.InputKeyState;
	if ( Ar.IsLoading() && Ar.Ver() < VER_MADE_INPUTKEYACTION_OUTPUT_LINKS )
	{
		Ar << MyInputKeyAction.ActionsToExecute;
		MyInputKeyAction.TriggeredOps.Empty(MyInputKeyAction.ActionsToExecute.Num());
		for ( INT Idx = 0; Idx < MyInputKeyAction.ActionsToExecute.Num(); Idx++ )
		{
			new(MyInputKeyAction.TriggeredOps) FSeqOpOutputInputLink(MyInputKeyAction.ActionsToExecute(Idx));
		}
	}
	else
	{
		Ar << MyInputKeyAction.TriggeredOps;
	}

	return Ar;
}

UBOOL FInputKeyAction::IsLinkedTo( const USequenceOp* CheckOp ) const
{
	UBOOL bResult = FALSE;

	if ( CheckOp != NULL )
	{
		for ( INT OpIndex = 0; OpIndex < TriggeredOps.Num(); OpIndex++ )
		{
			const FSeqOpOutputInputLink& OpLink = TriggeredOps(OpIndex);
			if ( OpLink.LinkedOp == CheckOp )
			{
				bResult = TRUE;
				break;
			}
		}
	}
	return bResult;
}

/* ==========================================================================================================
	FUIInputAliasClassMap
========================================================================================================== */
/**
 * Initializes the runtime lookup table with the aliases stored in WidgetInputAliases
 *
 * @param	InputAliasList	the list of input alias mappings for all registered UI classes.
 */
void FUIInputAliasClassMap::InitializeLookupTable( const TMap<UClass*,FUIInputAliasClassMap*>& InputAliasList )
{
	StateLookupTable.Empty();
	StateReverseLookupTable.Empty();

	TArray<UClass*> WidgetClassHierarchy;

	// first, generate a list of widget classes that will contribute to this class map's input aliases
	if ( GIsGame )
	{
		UClass* CurrentWidgetClass = WidgetClass;
		while ( CurrentWidgetClass && CurrentWidgetClass != UUIRoot::StaticClass() )
		{
			WidgetClassHierarchy.AddItem(CurrentWidgetClass);
			CurrentWidgetClass = CurrentWidgetClass->GetSuperClass();
		}
	}
	else
	{
		// if we're in the editor, we only want to see the input aliases for a single class
		WidgetClassHierarchy.AddItem(WidgetClass);
	}

	while ( WidgetClassHierarchy.Num() > 0 )
	{
		const FUIInputAliasClassMap* CurrentClassInputAliasMap = InputAliasList.FindRef(WidgetClassHierarchy.Last());

		if ( CurrentClassInputAliasMap != NULL )
		{
			// Create a lookup map that translates unreal keys into UI event keys.
			for( INT StateIdx = 0; StateIdx < CurrentClassInputAliasMap->WidgetStates.Num(); StateIdx++ )
			{
				const FUIInputAliasStateMap& StateInputAliasList = CurrentClassInputAliasMap->WidgetStates(StateIdx);

				// Find the list of reverse lookup tables for this state, and add an entry for this class
				// If not found, create one.  Each element in the array corresponds to the reverse lookup table in each parent class for this state.
				TArray<const FUIInputAliasStateMap*>* ReverseLookupTableArray = StateReverseLookupTable.Find(StateInputAliasList.State);
				if ( ReverseLookupTableArray == NULL )
				{
					ReverseLookupTableArray = &StateReverseLookupTable.Set(StateInputAliasList.State, TArray<const FUIInputAliasStateMap*>());
				}
				// It is usually not a good idea to keep a pointer to an element in a TArray, but this case is OK because the WidgetStates array never changes.
				ReverseLookupTableArray->AddItem(&StateInputAliasList);

				// find the input key => input alias map for this state, or create one if it can't be found.
				FUIInputAliasMap* InputMap = StateLookupTable.Find(StateInputAliasList.State);
				if ( InputMap == NULL )
				{
					InputMap = &StateLookupTable.Set(StateInputAliasList.State, FUIInputAliasMap());
				}

				// Add a entry for each unreal key that is bound to an alias in the input alias lookup table.
				for(INT ActionIdx = 0; ActionIdx < StateInputAliasList.StateInputAliases.Num(); ActionIdx++)
				{
					const FUIInputActionAlias& InputAliasKeyList = StateInputAliasList.StateInputAliases(ActionIdx);
					for ( INT KeyIdx = 0; KeyIdx < InputAliasKeyList.LinkedInputKeys.Num(); KeyIdx++ )
					{
						const FRawInputKeyEventData& InputKeyData = InputAliasKeyList.LinkedInputKeys(KeyIdx);

						FUIInputAliasValue AliasValue(InputKeyData.ModifierKeyFlags, InputAliasKeyList.InputAliasName);

						// any aliases that have the same InputKeyName ModifierKeyFlags are aliases that were inherited
						// from a parent class.  If this class has an alias mapped to the same key and modifier state then
						// this class wants to override the alias mapping so we'll need to remove the old one first.
						TArray<FUIInputAliasValue> ExistingAliases;
						InputMap->InputAliasLookupTable.MultiFind(InputKeyData.InputKeyName, ExistingAliases);

						for ( INT MappingIndex = 0; MappingIndex < ExistingAliases.Num(); MappingIndex++ )
						{
							FUIInputAliasValue& Mapping = ExistingAliases(MappingIndex);
							if ( Mapping.ModifierFlagMask == AliasValue.ModifierFlagMask )
							{
								InputMap->InputAliasLookupTable.RemovePair(InputKeyData.InputKeyName, Mapping);
							}
						}

						InputMap->InputAliasLookupTable.Add(InputKeyData.InputKeyName, AliasValue);
					}
				}
			}
		}

		WidgetClassHierarchy.Pop();
	}

	StateLookupTable.Shrink();
	StateReverseLookupTable.Shrink();
}

/* ==========================================================================================================
	FUIInputAliasValue
========================================================================================================== */
/**
 * Returns FALSE if this input alias value's ModifierFlagMask disallows the provided modifier key states.
 */
UBOOL FUIInputAliasValue::MatchesModifierState( UBOOL bAltPressed, UBOOL bCtrlPressed, UBOOL bShiftPressed ) const
{
	if ( bAltPressed )
	{
		if ( (ModifierFlagMask&KEYMODIFIER_AltExcluded) != 0 )
		{
			return FALSE;
		}
	}
	else if ( (ModifierFlagMask&KEYMODIFIER_AltRequired) != 0 )
	{
		return FALSE;
	}

	if ( bCtrlPressed )
	{
		if ( (ModifierFlagMask&KEYMODIFIER_CtrlExcluded) != 0 )
		{
			return FALSE;
		}
	}
	else if ( (ModifierFlagMask&KEYMODIFIER_CtrlRequired) != 0 )
	{
		return FALSE;
	}

	if ( bShiftPressed )
	{
		if ( (ModifierFlagMask&KEYMODIFIER_ShiftExcluded) != 0 )
		{
			return FALSE;
		}
	}
	else if ( (ModifierFlagMask&KEYMODIFIER_ShiftRequired) != 0 )
	{
		return FALSE;
	}

	return TRUE;
}

/* ==========================================================================================================
	UUIState
========================================================================================================== */
/**
 * Called when the state is created.
 */
void UUIState::Created()
{
	if ( StateSequence == NULL )
	{
		CreateStateSequence();
	}
}

/**
 * Creates and initializes a UIStateSequence for this UIState.
 *
 * @param	SequenceName	the name for the new sequence.  only specified when importing (copy/paste) to ensure that
 *							the new sequence's name matches the name for any references to that sequence in the t3d text
 */
void UUIState::CreateStateSequence( FName SequenceName/*=NAME_None*/ )
{
	UUIScreenObject* Owner = GetOwner();
	check(Owner);

	// owner widget doesn't support events.
	if ( Owner->EventProvider == NULL )
	{
		return;
	}

	Modify();

	// if this state is owned by a widget instanced from a UIPrefab, find the correct archetype before
	// creating the sequence
	UUIStateSequence* SequenceTemplate = NULL;
	EObjectFlags Flags = RF_Transactional|GetMaskedFlags(RF_Transient|RF_ArchetypeObject);

	if ( Owner->IsInPrefabInstance() )
	{
		SequenceTemplate = GetArchetype<UUIState>()->StateSequence;
	}
	else if ( Owner->HasAnyFlags(RF_ArchetypeObject) || Owner->IsInUIPrefab() || Owner->IsA(UUIPrefab::StaticClass()) )
	{
		Flags |= RF_Public;
	}

	if ( GIsGame )
	{
		StateSequence = ConstructObject<UUIStateSequence>(UUIStateSequence::StaticClass(), this, SequenceName,
			Flags, SequenceTemplate);
	}
	else
	{
		// if we're in the editor, create the sequence in the transient package with a transient name then set the correct outer/name
		// after calling Modify().  This is so that the same name can be used the next time, after undoing the creation of this sequence.
		StateSequence = ConstructObject<UUIStateSequence>(UUIStateSequence::StaticClass(), UObject::GetTransientPackage(), NAME_None,
			Flags, SequenceTemplate);

		StateSequence->Modify();

		const EObjectFlags OriginalFlags = StateSequence->GetFlags();
		StateSequence->ClearFlags(RF_Public);

		StateSequence->Rename(SequenceName != NAME_None ? *SequenceName.ToString() : *StateSequence->GetName(), this, REN_ForceNoResetLoaders);

		StateSequence->SetFlags(OriginalFlags);
	}

	StateSequence->ObjName = FString::Printf(TEXT("%s_Sequence"), *GetName());
	StateSequence->ParentSequence = Owner->EventProvider->EventContainer;
}

/**
 * Changes this state's StackPriority to the specified value.
 *
 * @param	PlayerIndex			the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	NewStackPriority	the new priority to assign to this state
 * @param	bSkipNotification	specify TRUE to prevent the widget from re-resolving its style (useful when calling
 *								this method on several states at a time)
 */
void UUIState::SetStatePriority( INT PlayerIndex, BYTE NewStackPriority, UBOOL bSkipNotification/*=FALSE*/ )
{
	StackPriority = NewStackPriority;
	OnStackPriorityChanged(PlayerIndex, !bSkipNotification);
}

/**
 * Resets this state's StackPriority to its default value.
 *
 * @param	PlayerIndex			the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bSkipNotification	specify TRUE to prevent the widget from re-resolving its style (useful when calling
 *								this method on several states at a time)
 */
void UUIState::ResetStatePriority( INT PlayerIndex, UBOOL bSkipNotification/*=FALSE*/ )
{
	UUIState* StateArchetype = GetArchetype<UUIState>();
	if ( StateArchetype != NULL )
	{
		StackPriority = StateArchetype->StackPriority;
	}
	else
	{
		StackPriority = GetClass()->GetDefaultObject<UUIState>()->StackPriority;
	}

	OnStackPriorityChanged(PlayerIndex, !bSkipNotification);
}

/**
 * Called when this state's StackPriority is changed at runtime.  Moves the state to the appropriate location in the
 * the owning widget's list of active states, if applicable.
 *
 * @param	PlayerIndex			the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bSendNotification	specify TRUE to re-resolve the owning widget's style if the top-most state changed as
 *								a result of this state's StackPriority changing.
 */
void UUIState::OnStackPriorityChanged( INT PlayerIndex, UBOOL bSendNotification )
{
	UUIScreenObject* OwnerWidget = GetOwner();
	if ( OwnerWidget != NULL )
	{
		// first, see if this state is already in the Target's StateStack
		INT ExistingStackIndex = OwnerWidget->StateStack.FindItemIndex(this);
		if ( ExistingStackIndex != INDEX_NONE )
		{
			UUIState* PreviouslyActiveState = NULL;
			if ( OwnerWidget->StateStack.Num() > 0 )
			{
				PreviouslyActiveState = OwnerWidget->StateStack.Last();
			}

			// this state is already active - just verify that it's in the correct position in the state stack
			for ( INT StackIndex = ExistingStackIndex + 1; StackIndex < OwnerWidget->StateStack.Num(); StackIndex++ )
			{
				UUIState* OtherState = OwnerWidget->StateStack(StackIndex);
				if ( OtherState->StackPriority < StackPriority )
				{
					OwnerWidget->StateStack.SwapItems(ExistingStackIndex, StackIndex);
					ExistingStackIndex = StackIndex;
				}
			}

			for ( INT StackIndex = ExistingStackIndex - 1; StackIndex >= 0; StackIndex-- )
			{
				UUIState* OtherState = OwnerWidget->StateStack(StackIndex);
				if ( OtherState->StackPriority > StackPriority )
				{
					OwnerWidget->StateStack.SwapItems(ExistingStackIndex, StackIndex);
					ExistingStackIndex = StackIndex;
				}
			}

			// if the top-most state changed, fire script notifications and resolve the widget's style.
			if ( bSendNotification && PreviouslyActiveState != NULL && PreviouslyActiveState != OwnerWidget->StateStack.Last() )
			{
				// script notifications
				// fire the delegate in the widget
				OwnerWidget->PropagateStateChangeNotification(PlayerIndex, this, PreviouslyActiveState);

				if ( OwnerWidget->IsA(UUIObject::StaticClass()) )
				{
					static_cast<UUIObject*>(OwnerWidget)->ResolveStyles(FALSE);
				}
			}
		}
	}
}

/**
 * Returns the widget that contains this UIState.
 */
UUIScreenObject* UUIState::GetOwner() const
{
	UUIScreenObject* Result=NULL;

	for ( UObject* Owner = GetOuter(); Owner && Result == NULL; Owner = Owner->GetOuter() )
	{
		Result = Cast<UUIScreenObject>(Owner);
	}

	return Result;
}

/**
* Adds the specified PlayerIndex to this state's PlayerIndexMask, indicating that this state is now active for that
* player.
*/
void UUIState::EnablePlayerIndex( INT PlayerIndex )
{
	checkSlow(PlayerIndex>=0&&PlayerIndex<UCONST_MAX_SUPPORTED_GAMEPADS);

	PlayerIndexMask |= (1 << PlayerIndex);
}

/**
* Removes the specified PlayerIndex from this state's PlayerIndexMask, indicating that this state is now active for that
* player.
*/
void UUIState::DisablePlayerIndex( INT PlayerIndex )
{
	checkSlow(PlayerIndex>=0&&PlayerIndex<UCONST_MAX_SUPPORTED_GAMEPADS);

	PlayerIndexMask &= ~(1 << PlayerIndex);
}

/**
 * Determines whether this state has been activated for the specified player index
 *
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] of the player to check for
 *
 * @return	TRUE if this state has been activated for the specified player.
 */
UBOOL UUIState::IsActiveForPlayer( INT PlayerIndex ) const
{
	return (PlayerIndexMask & (1 << PlayerIndex)) != 0;
}

/**
 * Activate this state for the specified target.
 *
 * @param	Target			the widget that is activating this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE if Target's StateStack was modified; FALSE if this state couldn't be activated for the specified
 *			Target or this state was already part of the Target's state stack.
 */
UBOOL UUIState::ActivateState( UUIScreenObject* Target, INT PlayerIndex )
{
	check(Target);
	UBOOL bResult = TRUE;

	// first, see if this state is already in the Target's StateStack
	INT ExistingStackIndex = Target->StateStack.FindItemIndex(this);
	if ( ExistingStackIndex == INDEX_NONE || !IsActiveForPlayer(PlayerIndex) )
	{
		// first, check if any states currently in Target's StateStack want to disallow this
		// state from being activated
		for ( INT StateIndex = 0; StateIndex < Target->StateStack.Num(); StateIndex++ )
		{
			if ( !Target->StateStack(StateIndex)->eventIsStateAllowed(Target,this,PlayerIndex) )
			{
				bResult = FALSE;
				break;
			}
		}

		if ( bResult )
		{
			if ( eventActivateState(Target,PlayerIndex) )
			{
				// send notifications
				UBOOL bNewlyActivated = PlayerIndexMask == 0 || ExistingStackIndex == INDEX_NONE;
				EnablePlayerIndex(PlayerIndex);

				UUIState* PreviouslyActiveState = Target->GetCurrentState();
				OnActivate(Target,PlayerIndex,bNewlyActivated);

				// script notifications
				// fire the delegate in the widget
				Target->PropagateStateChangeNotification(PlayerIndex, Target->GetCurrentState(), PreviouslyActiveState);

				// then the event in the state itself.
				eventOnActivate(Target,PlayerIndex,bNewlyActivated);
			}
			else
			{
				bResult = FALSE;
			}
		}
	}
	else
	{
		ResetStatePriority(PlayerIndex,TRUE);
	}

	return bResult;
}

/**
 * Deactivate this state for the specified target.
 *
 * @param	Target			the widget that is deactivating this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE if Target's StateStack was modified; FALSE if this state couldn't be deactivated for the specified
 *			Target or this state wasn't part of the Target's state stack.
 */
UBOOL UUIState::DeactivateState( UUIScreenObject* Target, INT PlayerIndex )
{
	check(Target);
	UBOOL bResult = FALSE;

	INT StackIndex = Target->StateStack.FindItemIndex(this);
	if ( StackIndex != INDEX_NONE )
	{
		if ( eventDeactivateState(Target,PlayerIndex) )
		{
			DisablePlayerIndex(PlayerIndex);

			// remember the previously active state
			UUIState* PreviouslyActiveState = Target->GetCurrentState();

			// if this is the last player that activated this state, remove the state from the state stack.
			const UBOOL bPopState = PlayerIndexMask==0;

			// change our state priority back to the default value
			ResetStatePriority(PlayerIndex, TRUE);

			// remove and unregister this state
			OnDeactivate(Target,PlayerIndex,bPopState);

			UUIState* CurrentlyActiveState = Target->GetCurrentState();
			if ( CurrentlyActiveState != PreviouslyActiveState )
			{
				// script notifications
				// fire the delegate in the widget
				Target->PropagateStateChangeNotification(PlayerIndex, CurrentlyActiveState, PreviouslyActiveState);
			}

			eventOnDeactivate(Target,PlayerIndex,bPopState);


			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Notification that Target has made this state its active state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 */
void UUIState::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	check(Target);

	if ( bPushState )
	{
		// add the state to the target's list of states at the appropriate location
		INT InsertIndex=0;
		for ( InsertIndex = 0; InsertIndex < Target->StateStack.Num(); InsertIndex++ )
		{
			UUIState* CurrentState = Target->StateStack(InsertIndex);
			if ( CurrentState->StackPriority > StackPriority )
			{
				break;
			}
		}

		Target->StateStack.InsertItem(this, InsertIndex);
	}

	// trigger the kismet event to notify the widget that it is entering a new state
	Target->ActivateEventByClass(PlayerIndex, UUIEvent_OnEnterState::StaticClass(), this);

	if ( GIsGame && Target->EventProvider != NULL )
	{
		//@todo - this could also be done in the Activated method of the UIEvent_OnEnterState, but I'm not sure
		// which way is better...

		if ( bPushState )
		{
			// now add the sequence for this state to the widget's main sequence
			Target->EventProvider->PushStateSequence(StateSequence);
		}

		Target->EventProvider->RegisterInputEvents(this,PlayerIndex);
	}
}

/**
 * Notification that Target has just deactivated this state.
 *
 * @param	Target			the widget that deactivated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPopState		TRUE if this state needs to be removed from the owning widget's StateStack; FALSE if this state is
 *							still active for at least one player (i.e. in splitscreen)
 */
void UUIState::OnDeactivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPopState )
{
	check(Target);

	// trigger the kismet event to notify the widget that it's leaving this state
	Target->ActivateEventByClass(PlayerIndex, UUIEvent_OnLeaveState::StaticClass(), this);

	if ( bPopState == TRUE )
	{
		// remove this state from the Target's list of states
		Target->StateStack.RemoveItem(this);
	}

	if ( GIsGame && Target->EventProvider != NULL )
	{
		if ( bPopState == TRUE )
		{
			// now remove the sequence for this state from the widget's main sequence
			Target->EventProvider->PopStateSequence(StateSequence);
		}

		Target->EventProvider->UnregisterInputEvents(this,PlayerIndex);
	}
}


/**
 * Adds the specified InputAction to this UIState's StateInputActions array, if it doesn't already exist.
 *
 * @param	InputAction		the key/action combo that will be scoped by this UIState
 */
void UUIState::AddInputAction( const FInputKeyAction& InputAction )
{
	//@todo ronp - this relies on FInputKeyAction::operator== not checking the TriggeredOps array
	if ( !StateInputActions.ContainsItem(InputAction) )
	{
		FScopedObjectStateChange StateActionNotifier(this);

		new(StateInputActions) FInputKeyAction(InputAction);

		//@todo ronp - this relies on FInputKeyAction::operator== not checking the TriggeredOps array
		INT DisabledInputIndex = DisabledInputActions.FindItemIndex(InputAction);
		if ( DisabledInputIndex != INDEX_NONE )
		{
			// this input action was previously removed by the designer, but now we want to add it back so remove
			// it from the disabled array so that it will be instanced from now on.
			DisabledInputActions.Remove(DisabledInputIndex);
		}
	}

	// do some other stuff
}

/**
 * Removes the specified InputAction from this UIState's StateInputActions array.  If the input action was instanced
 * from a default input action in the widget class's default properties, adds the input action to the state's DisabledInputActions array
 *
 * @param	InputAction		the key/action combo to remove from this state's list of input keys
 */
void UUIState::RemoveInputAction( const FInputKeyAction& InputAction )
{
	//@todo ronp - this relies on FInputKeyAction::operator== not checking the TriggeredOps array
	INT ActionIndex = StateInputActions.FindItemIndex(InputAction);
	if ( ActionIndex != INDEX_NONE )
	{
		FScopedObjectStateChange StateActionNotifier(this);
		StateInputActions.Remove(ActionIndex);
	}
}

/**
 * Retrieves the UIEvents contained by this container.
 *
 * @param	out_Events	will be filled with the UIEvent instances stored in by this container
 * @param	LimitClass	if specified, only events of the specified class (or child class) will be added to the array
 */
void UUIState::GetUIEvents( TArray<UUIEvent*>& out_Events, UClass* LimitClass/*=NULL*/ )
{
	if ( StateSequence != NULL )
	{
		StateSequence->GetUIEvents(out_Events,LimitClass);
	}
}

/**
 * Adds a new SequenceObject to this containers's list of ops
 *
 * @param	NewObj		the sequence object to add.
 * @param	bRecurse	unused in this implementation...
 *
 * @return	TRUE if the object was successfully added to the sequence.
 */
UBOOL UUIState::AddSequenceObject( USequenceObject* NewObj, UBOOL bRecurse/*=FALSE*/ )
{
	UBOOL bResult = FALSE;
	if ( NewObj != NULL && StateSequence != NULL )
	{
		bResult = StateSequence->AddSequenceObject(NewObj);
	}

	return bResult;
}

/**
 * Removes the specified SequenceObject from this container's list of ops.
 *
 * @param	ObjectToRemove	the sequence object to remove
 */
void UUIState::RemoveSequenceObject( USequenceObject* ObjectToRemove )
{
	if ( StateSequence != NULL )
	{
		StateSequence->RemoveObject(ObjectToRemove);
	}
}

/**
 * Removes the specified SequenceObjects from this container's list of ops.
 *
 * @param	ObjectsToRemove		the objects to remove from this sequence
 */
void UUIState::RemoveSequenceObjects( const TArray<USequenceObject*>& ObjectsToRemove )
{
	if ( StateSequence != NULL )
	{
		StateSequence->RemoveObjects(ObjectsToRemove);
	}
}

/* === UObject interface === */
/**
 * Determines whether this object is contained within a UIPrefab.
 *
 * @param	OwnerPrefab		if specified, receives a pointer to the owning prefab.
 *
 * @return	TRUE if this object is contained within a UIPrefab; FALSE if this object IS a UIPrefab or is not
 *			contained within a UIPrefab.
 */
UBOOL UUIState::IsAPrefabArchetype( UObject** OwnerPrefab/*=NULL*/ ) const
{
	UBOOL bResult = FALSE;

	UUIScreenObject* WidgetOwner = GetOwner();
	if ( WidgetOwner != NULL )
	{
		bResult = WidgetOwner->IsAPrefabArchetype(OwnerPrefab);
	}
	else
	{
		bResult = Super::IsAPrefabArchetype(OwnerPrefab);
	}
	return bResult;
}

/**
 * @return	TRUE if the object is contained within a UIPrefabInstance.
 */
UBOOL UUIState::IsInPrefabInstance() const
{
	UBOOL bResult = FALSE;

	UUIScreenObject* WidgetOwner = GetOwner();
	if ( WidgetOwner != NULL )
	{
		bResult = WidgetOwner->IsInPrefabInstance();
	}
	else
	{
		bResult = Super::IsInPrefabInstance();
	}
	return bResult;
}

/**
 * Called after the object has loaded.
 */
void UUIState::PostLoad()
{
	Super::PostLoad();

	if ( GetLinkerVersion() < VER_MADE_INPUTKEYACTION_OUTPUT_LINKS )
	{
		for ( INT ActionIndex = 0; ActionIndex < StateInputActions.Num(); ActionIndex++ )
		{
			FInputKeyAction& Action = StateInputActions(ActionIndex);
			Action.TriggeredOps.Empty(Action.ActionsToExecute.Num());

			for ( INT OpIndex = 0; OpIndex < Action.ActionsToExecute.Num(); OpIndex++ )
			{
				new(Action.TriggeredOps) FSeqOpOutputInputLink(Action.ActionsToExecute(OpIndex));
			}
		}

		for ( INT ActionIndex = 0; ActionIndex < DisabledInputActions.Num(); ActionIndex++ )
		{
			FInputKeyAction& Action = DisabledInputActions(ActionIndex);
			Action.TriggeredOps.Empty(Action.ActionsToExecute.Num());

			for ( INT OpIndex = 0; OpIndex < Action.ActionsToExecute.Num(); OpIndex++ )
			{
				new(Action.TriggeredOps) FSeqOpOutputInputLink(Action.ActionsToExecute(OpIndex));
			}
		}
	}
}

/* ==========================================================================================================
	UUIState_Active
========================================================================================================== */
/**
 * Activate this state for the specified target.  This version ensures that the StackPriority for the Active and
 * Pressed states have been reset to their default values.
 *
 * @param	Target			the widget that is activating this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE if Target's StateStack was modified; FALSE if this state couldn't be activated for the specified
 *			Target or this state was already part of the Target's state stack.
 */
UBOOL UUIState_Active::ActivateState( UUIScreenObject* Target, INT PlayerIndex )
{
	UBOOL bResult = Super::ActivateState(Target, PlayerIndex);
	if ( bResult )
	{
		INT StateIndex=0;
		if ( Target != NULL && Target->IsPressed(PlayerIndex, &StateIndex) )
		{
			Target->ActivateState(Target->StateStack(StateIndex), PlayerIndex);
		}

		UUIInteraction* UIController = GetCurrentUIController();
		if ( UIController == NULL )
		{
			UIController = GetDefaultUIController();
		}

		checkSlow(UIController);
		if ( UIController->bFocusOnActive && Target->CanAcceptFocus(PlayerIndex) )
		{
			Target->SetFocus(NULL, PlayerIndex);
		}
	}

	return bResult;
}

/**
 * Deactivate this state for the specified target.  This version changes the StackPriority on the Active and Pressed states
 * so that the widget uses the style data from whichever state the widget was previously in.
 *
 * @param	Target			the widget that is deactivating this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE if Target's StateStack was modified; FALSE if this state couldn't be deactivated for the specified
 *			Target or this state wasn't part of the Target's state stack.
 */
UBOOL UUIState_Active::DeactivateState( UUIScreenObject* Target, INT PlayerIndex )
{
	UBOOL bResult = TRUE;

	INT PressedStateIndex=0;

	// If the widget is currently in the pressed state...
	if ( Target != NULL && Target->IsPressed(PlayerIndex, &PressedStateIndex) )
	{
		// unless the widget's scene has been deactivated, keep the widget in the active state until it's no longer in the pressed state.  This is so that
		// the widget remains the scene client's ActiveControl as long as the user is still holding e.g. the mouse button.
		UUIScene* OwnerScene = Target->GetScene();
		if ( OwnerScene != NULL )
		{
			UUIState* PressedState = Target->StateStack(PressedStateIndex);
			if ( OwnerScene->IsSceneActive(TRUE) )
			{
				const INT HidePriority = UUIState_Enabled::StaticClass()->GetDefaultObject<UUIState_Enabled>()->StackPriority - 1;
				PressedState->SetStatePriority(PlayerIndex, HidePriority,TRUE);
				SetStatePriority(PlayerIndex, HidePriority - 1);
				bResult = FALSE;
			}
			else
			{
				// otherwise, if our scene is no longer the topmost scene and we're being de-activated, also deactivate the pressed state
				Target->DeactivateState(PressedState, PlayerIndex);
				PressedState->ResetStatePriority(PlayerIndex, TRUE);
				ResetStatePriority(PlayerIndex);
			}
		}
	}

	return bResult && Super::DeactivateState(Target, PlayerIndex);
}

/**
 * Notification that Target has made this state its active state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
 *							in the state stack of the owning widget and is being activated for additional split-screen players.
 */
void UUIState_Active::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	//@fixme splitscreen - do we need an ActiveControl for each player?
	if ( Target != NULL && Target->GetScene() != NULL )
	{
		UGameUISceneClient* GameSceneClient = Cast<UGameUISceneClient>(Target->GetScene()->SceneClient);
		if ( GameSceneClient != NULL )
		{
			GameSceneClient->SetActiveControl(Cast<UUIObject>(Target));
		}
	}

	Super::OnActivate(Target,PlayerIndex,bPushState);
}

/**
 * Notification that Target has just deactivated this state.
 *
 * @param	Target			the widget that deactivated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPopState		TRUE if this state needs to be removed from the owning widget's StateStack; FALSE if this state is
 *							still active for at least one player (i.e. in splitscreen)
 */
void UUIState_Active::OnDeactivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPopState )
{
	Super::OnDeactivate(Target,PlayerIndex,bPopState);

	//@fixme splitscreen - do we need an ActiveControl for each player?
	if ( bPopState && Target != NULL && Target->GetScene() != NULL )
	{
		if ( !Target->HasActiveStateOfClass(UUIState_Pressed::StaticClass(), PlayerIndex) )
		{
			UGameUISceneClient* GameSceneClient = Cast<UGameUISceneClient>(Target->GetScene()->SceneClient);
			if ( GameSceneClient != NULL )
			{
				GameSceneClient->SetActiveControl(NULL);
			}
		}
	}
}


/* ==========================================================================================================
	UUIState_Focused
========================================================================================================== */
/**
 * Notification that Target has made this state its active state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
 *							in the state stack of the owning widget and is being activated for additional split-screen players.
 */
void UUIState_Focused::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	if ( bPushState == TRUE )
	{
		// add the state to the target's list of states at the appropriate location
		INT InsertIndex=0;
		for ( InsertIndex = 0; InsertIndex < Target->StateStack.Num(); InsertIndex++ )
		{
			UUIState* CurrentState = Target->StateStack(InsertIndex);
			if ( CurrentState->StackPriority > StackPriority )
			{
				break;
			}
		}

		Target->StateStack.InsertItem(this, InsertIndex);
	}

	// trigger the kismet event to notify the widget that it is entering a new state
	Target->ActivateEventByClass(PlayerIndex, UUIEvent_OnEnterState::StaticClass(), this);

	if ( GIsGame && Target->EventProvider != NULL )
	{
		//@todo - this could also be done in the Activated method of the UIEvent_OnEnterState, but I'm not sure
		// which way is better...

		if ( bPushState == TRUE )
		{
			// now add the sequence for this state to the widget's main sequence
			Target->EventProvider->PushStateSequence(StateSequence);

			// this version does not register input events, as this must be done after the parent of Target has set focus
			// @see UUIScreenObject::GainFocus()
		}
	}
}

/**
 * Notification that Target has just deactivated this state.
 *
 * @param	Target			the widget that deactivated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPopState		TRUE if this state needs to be removed from the owning widget's StateStack; FALSE if this state is
 *							still active for at least one player (i.e. in splitscreen)
 */
void UUIState_Focused::OnDeactivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPopState )
{
	Super::OnDeactivate(Target,PlayerIndex,bPopState);

	// if we're leaving the focused state, also deactivate the pressed state, if applicable.
	Target->DeactivateStateByClass(UUIState_Pressed::StaticClass(),PlayerIndex);
}

/* ==========================================================================================================
	UUIState_Pressed
========================================================================================================== */
/**
 * Notification that Target has made this state its active state.
 *
 * This version also activates the focused state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
 *							in the state stack of the owning widget and is being activated for additional split-screen players.
 */
void UUIState_Pressed::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	// Set focus to the control that was pressed.
	Target->SetFocus(NULL,PlayerIndex);

	Super::OnActivate(Target,PlayerIndex,bPushState);
}

/* ==========================================================================================================
	UUIState_Enabled
========================================================================================================== */
/**
 * Notification that Target has made this state its active state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
 *							in the state stack of the owning widget and is being activated for additional split-screen players.
 */
void UUIState_Enabled::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	Super::OnActivate(Target, PlayerIndex, bPushState);

	// Remove the disabled state from the stack.
	if( Target != NULL )
	{
		INT DisabledStateIndex = INDEX_NONE;
		if ( Target->HasActiveStateOfClass(UUIState_Disabled::StaticClass(), PlayerIndex, &DisabledStateIndex) )
		{
			Target->DeactivateState(Target->StateStack(DisabledStateIndex), PlayerIndex);
		}

		UUIObject* WidgetTarget = Cast<UUIObject>(Target);
		if ( WidgetTarget != NULL && WidgetTarget->IsPrivateBehaviorSet(UCONST_PRIVATE_PropagateState) )
		{
			UUIObject* SceneFocusHint = NULL;
			if ( WidgetTarget->bSupportsFocusHint )
			{
				UUIScene* TargetOwnerScene = WidgetTarget->GetScene();
				if ( TargetOwnerScene != NULL )
				{
					SceneFocusHint = TargetOwnerScene->eventGetFocusHint(TRUE);
				}
			}
			for ( INT ChildIndex = 0; ChildIndex < WidgetTarget->Children.Num(); ChildIndex++ )
			{
				UUIObject* Child = WidgetTarget->Children(ChildIndex);
				if ( Child != SceneFocusHint && Child->IsInitialized() )
				{
					Child->SetEnabled(TRUE, PlayerIndex);
				}
			}
		}
	}
}

/* ==========================================================================================================
	UUIState_Disabled
========================================================================================================== */
/**
 * Notification that Target has made this state its active state.
 *
 * @param	Target			the widget that activated this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
 *							in the state stack of the owning widget and is being activated for additional split-screen players.
 */
void UUIState_Disabled::OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState )
{
	Super::OnActivate(Target, PlayerIndex, bPushState);

	// Remove the enabled state from the stack.
	if( Target != NULL )
	{
		INT EnabledStateIndex = INDEX_NONE;
		if ( Target->HasActiveStateOfClass(UUIState_Enabled::StaticClass(), PlayerIndex, &EnabledStateIndex) )
		{
			Target->DeactivateState(Target->StateStack(EnabledStateIndex), PlayerIndex);
		}

		UUIObject* WidgetTarget = Cast<UUIObject>(Target);
		if ( WidgetTarget != NULL && WidgetTarget->IsPrivateBehaviorSet(UCONST_PRIVATE_PropagateState) )
		{
			UUIObject* SceneFocusHint = NULL;
			if ( WidgetTarget->bSupportsFocusHint )
			{
				UUIScene* TargetOwnerScene = WidgetTarget->GetScene();
				if ( TargetOwnerScene != NULL )
				{
					SceneFocusHint = TargetOwnerScene->eventGetFocusHint(TRUE);
				}
			}

			for ( INT ChildIndex = 0; ChildIndex < WidgetTarget->Children.Num(); ChildIndex++ )
			{
				UUIObject* Child = WidgetTarget->Children(ChildIndex);
				if ( Child != SceneFocusHint && Child->IsInitialized() )
				{
					Child->SetEnabled(FALSE, PlayerIndex);
				}
			}
		}
	}
}

