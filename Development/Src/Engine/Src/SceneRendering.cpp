/*=============================================================================
	SceneRendering.cpp: Scene rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "ScenePrivate.h"

/*-----------------------------------------------------------------------------
	Globals
-----------------------------------------------------------------------------*/

/**
	This debug variable is toggled by the 'toggleocclusion' console command.
	Turning it on will still issue all queries but ignore the results, which
	makes it possible to analyze those queries in NvPerfHUD. It will also
	stabilize the succession of draw calls in a paused game.
*/
UBOOL GIgnoreAllOcclusionQueries = FALSE;

/**
	This debug variable is set by the 'FullMotionBlur [N]' console command.
	Setting N to -1 or leaving it blank will make the Motion Blur effect use the
	default setting (whatever the game chooses).
	N = 0 forces FullMotionBlur off.
	N = 1 forces FullMotionBlur on.
 */
INT GMotionBlurFullMotionBlur = -1;

/** The minimum projected screen radius for a primitive to be drawn in the depth pass, as a fraction of half the horizontal screen width. */
static const FLOAT MinScreenRadiusForDepthPrepass = 0.05f;
static const FLOAT MinScreenRadiusForDepthPrepassSquared = Square(MinScreenRadiusForDepthPrepass);

/*-----------------------------------------------------------------------------
	FViewInfo
-----------------------------------------------------------------------------*/

/** 
 * Initialization constructor. Passes all parameters to FSceneView constructor
 */
FViewInfo::FViewInfo(
	const FSceneViewFamily* InFamily,
	FSceneViewStateInterface* InState,
	INT InParentViewIndex,
	const FSceneViewFamily* InParentViewFamily,
	FSynchronizedActorVisibilityHistory* InActorVisibilityHistory,
	const AActor* InViewActor,
	const UPostProcessChain* InPostProcessChain,
	const FPostProcessSettings* InPostProcessSettings,
	const FPostProcessMaskBase* InPostProcessMask,
	FViewElementDrawer* InDrawer,
	FLOAT InX,
	FLOAT InY,
	FLOAT InSizeX,
	FLOAT InSizeY,
	const FMatrix& InViewMatrix,
	const FMatrix& InProjectionMatrix,
	const FLinearColor& InBackgroundColor,
	const FLinearColor& InOverlayColor,
	const FLinearColor& InColorScale,
	const TSet<UPrimitiveComponent*>& InHiddenPrimitives,
	FLOAT InLODDistanceFactor
	)
	:	FSceneView(
			InFamily,
			InState,
			InParentViewIndex,
			InParentViewFamily,
			InActorVisibilityHistory,
			InViewActor,
			InPostProcessChain,
			InPostProcessSettings,
			InPostProcessMask,
			InDrawer,
			InX,
			InY,
			InSizeX,
			InSizeY,
			InViewMatrix,
			InProjectionMatrix,
			InBackgroundColor,
			InOverlayColor,
			InColorScale,
			InHiddenPrimitives,
			InLODDistanceFactor
			)
	,	bRequiresVelocities( FALSE )
	,	bRequiresPrevTransforms( FALSE )
	,	bPrevTransformsReset( FALSE )
	,	bIgnoreExistingQueries( FALSE )
	,	bDisableQuerySubmissions( FALSE )
	,	bOneLayerHeightFogRenderedInAO( FALSE)
	,	NumVisibleStaticMeshElements(0)
	,	NumVisibleDynamicPrimitives(0)
	,	IndividualOcclusionQueries((FSceneViewState*)InState,1)
	,	GroupedOcclusionQueries((FSceneViewState*)InState,FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	ActorVisibilitySet(NULL)
{
	PrevViewProjMatrix.SetIdentity();
	PrevViewRotationProjMatrix.SetIdentity();
	PrevViewOrigin = FVector(0,0,0);

	// Clear fog constants.
	for(INT LayerIndex = 0; LayerIndex < ARRAY_COUNT(FogDistanceScale); LayerIndex++)
	{
		FogMinHeight[LayerIndex] = FogMaxHeight[LayerIndex] = FogDistanceScale[LayerIndex] = 0.0f;
		FogStartDistance[LayerIndex] = 0.f;
		FogInScattering[LayerIndex] = FLinearColor::Black;
		FogExtinctionDistance[LayerIndex] = FLT_MAX;
	}
}

/** 
 * Initialization constructor. 
 * @param InView - copy to init with
 */
FViewInfo::FViewInfo(const FSceneView* InView)
	:	FSceneView(*InView)
	,	bHasTranslucentViewMeshElements( 0 )
	,	bHasDistortionViewMeshElements( 0 )
	,	bRequiresVelocities( FALSE )
	,	bRequiresPrevTransforms( FALSE )
	,	bPrevTransformsReset( FALSE )
	,	bIgnoreExistingQueries( FALSE )
	,	bDisableQuerySubmissions( FALSE )
	,	bOneLayerHeightFogRenderedInAO( FALSE)
	,	NumVisibleStaticMeshElements(0)
	,	NumVisibleDynamicPrimitives(0)
	,	IndividualOcclusionQueries((FSceneViewState*)InView->State,1)
	,	GroupedOcclusionQueries((FSceneViewState*)InView->State,FOcclusionQueryBatcher::OccludedPrimitiveQueryBatchSize)
	,	ActorVisibilitySet(NULL)
{
	PrevViewProjMatrix.SetIdentity();
	PrevViewRotationProjMatrix.SetIdentity();
	PrevViewOrigin = FVector(0,0,0);

	// Clear fog constants.
	for(INT LayerIndex = 0; LayerIndex < ARRAY_COUNT(FogDistanceScale); LayerIndex++)
	{
		FogMinHeight[LayerIndex] = FogMaxHeight[LayerIndex] = FogDistanceScale[LayerIndex] = 0.0f;
		FogStartDistance[LayerIndex] = 0.f;
		FogInScattering[LayerIndex] = FLinearColor::Black;
		FogExtinctionDistance[LayerIndex] = FLT_MAX;
	}

	if( PostProcessChain )
	{
		// create render proxies for any post process effects in this view
		for( INT EffectIdx=0; EffectIdx < PostProcessChain->Effects.Num(); EffectIdx++ )
		{
			UPostProcessEffect* Effect = PostProcessChain->Effects(EffectIdx);
			// only add a render proxy if the effect is enabled
			if( Effect && Effect->IsShown(InView) )
			{
				FPostProcessSceneProxy* PostProcessSceneProxy = Effect->CreateSceneProxy(
					PostProcessSettings && Effect->bUseWorldSettings ? PostProcessSettings : NULL
					);
				if( PostProcessSceneProxy )
				{
					PostProcessSceneProxies.AddRawItem(PostProcessSceneProxy);
					bRequiresVelocities = bRequiresVelocities || PostProcessSceneProxy->RequiresVelocities( MotionBlurParameters );
					bRequiresPrevTransforms = bRequiresPrevTransforms || PostProcessSceneProxy->RequiresPreviousTransforms(*this);
				}
			}
		}

		// Mark the final post-processing effect so that we can render it directly to the view render target.
		UINT DPGIndex = SDPG_PostProcess;
		INT FinalIdx = -1;
		for( INT ProxyIdx=0; ProxyIdx < PostProcessSceneProxies.Num(); ProxyIdx++ )
		{
			if( PostProcessSceneProxies(ProxyIdx).GetDepthPriorityGroup() == DPGIndex
				&& !PostProcessSceneProxies(ProxyIdx).GetAffectsLightingOnly())
			{
				FinalIdx = ProxyIdx;
				PostProcessSceneProxies(FinalIdx).TerminatesPostProcessChain( FALSE );
			}
		}
		if (FinalIdx != -1)
		{
			PostProcessSceneProxies(FinalIdx).TerminatesPostProcessChain( TRUE );
		}
	}
}

/** 
 * Destructor. 
 */
FViewInfo::~FViewInfo()
{
	for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
	{
		DynamicResources(ResourceIndex)->ReleasePrimitiveResource();
	}
}

/** 
 * Initializes the dynamic resources used by this view's elements. 
 */
void FViewInfo::InitDynamicResources()
{
	for(INT ResourceIndex = 0;ResourceIndex < DynamicResources.Num();ResourceIndex++)
	{
		DynamicResources(ResourceIndex)->InitPrimitiveResource();
	}
}


/*-----------------------------------------------------------------------------
FSceneRenderer
-----------------------------------------------------------------------------*/

FSceneRenderer::FSceneRenderer(const FSceneViewFamily* InViewFamily,FHitProxyConsumer* HitProxyConsumer,const FMatrix& InCanvasTransform)
:	Scene(InViewFamily->Scene ? (FScene*)InViewFamily->Scene->GetRenderScene() : NULL)
,	ViewFamily(*InViewFamily)
,	CanvasTransform(InCanvasTransform)
,	PreviousFrameTime(0)
{
	// Collect dynamic shadow stats if the view family has the appropriate container object.
	if( InViewFamily->DynamicShadowStats )
	{
		bShouldGatherDynamicShadowStats = TRUE;
	}
	else
	{
		bShouldGatherDynamicShadowStats = FALSE;
	}

	// Copy the individual views.
	Views.Empty(InViewFamily->Views.Num());
	for(INT ViewIndex = 0;ViewIndex < InViewFamily->Views.Num();ViewIndex++)
	{
#if !FINAL_RELEASE
		for(INT ViewIndex2 = 0;ViewIndex2 < InViewFamily->Views.Num();ViewIndex2++)
		{
			if (ViewIndex != ViewIndex2 && InViewFamily->Views(ViewIndex)->State != NULL)
			{
				// Verify that each view has a unique view state, as the occlusion query mechanism depends on it.
				check(InViewFamily->Views(ViewIndex)->State != InViewFamily->Views(ViewIndex2)->State);
			}
		}
#endif
		// Construct a FViewInfo with the FSceneView properties.
		FViewInfo* ViewInfo = new(Views) FViewInfo(InViewFamily->Views(ViewIndex));
		ViewFamily.Views(ViewIndex) = ViewInfo;
		ViewInfo->Family = &ViewFamily;

		// Batch the view's elements for later rendering.
		if(ViewInfo->Drawer)
		{
			FViewElementPDI ViewElementPDI(ViewInfo,HitProxyConsumer);
			ViewInfo->Drawer->Draw(ViewInfo,&ViewElementPDI);
		}
	}
	
	if(HitProxyConsumer)
	{
		// Set the hit proxies show flag.
		ViewFamily.ShowFlags |= SHOW_HitProxies;
	}

	// Calculate the screen extents of the view family.
	UBOOL bInitializedExtents = FALSE;
	FLOAT MinFamilyX = 0;
	FLOAT MinFamilyY = 0;
	FLOAT MaxFamilyX = 0;
	FLOAT MaxFamilyY = 0;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FSceneView* View = &Views(ViewIndex);
		if(!bInitializedExtents)
		{
			MinFamilyX = View->X;
			MinFamilyY = View->Y;
			MaxFamilyX = View->X + View->SizeX;
			MaxFamilyY = View->Y + View->SizeY;
			bInitializedExtents = TRUE;
		}
		else
		{
			MinFamilyX = Min(MinFamilyX,View->X);
			MinFamilyY = Min(MinFamilyY,View->Y);
			MaxFamilyX = Max(MaxFamilyX,View->X + View->SizeX);
			MaxFamilyY = Max(MaxFamilyY,View->Y + View->SizeY);
		}
	}
	FamilySizeX = appTrunc(MaxFamilyX - MinFamilyX);
	FamilySizeY = appTrunc(MaxFamilyY - MinFamilyY);

	// Allocate the render target space to the views.
	check(bInitializedExtents);
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		View.RenderTargetX = appTrunc(View.X - MinFamilyX);
		View.RenderTargetY = appTrunc(View.Y - MinFamilyY);
		View.RenderTargetSizeX = Min<INT>(appTrunc(View.SizeX),ViewFamily.RenderTarget->GetSizeX());
		View.RenderTargetSizeY = Min<INT>(appTrunc(View.SizeY),ViewFamily.RenderTarget->GetSizeY());

		// Set the vector used by shaders to convert projection-space coordinates to texture space.
		View.ScreenPositionScaleBias =
			FVector4(
				View.SizeX / GSceneRenderTargets.GetBufferSizeX() / +2.0f,
				View.SizeY / GSceneRenderTargets.GetBufferSizeY() / -2.0f,
				(View.SizeY / 2.0f + GPixelCenterOffset + View.RenderTargetY) / GSceneRenderTargets.GetBufferSizeY(),
				(View.SizeX / 2.0f + GPixelCenterOffset + View.RenderTargetX) / GSceneRenderTargets.GetBufferSizeX()
				);
	}

#if !FINAL_RELEASE
	// copy off the requests
	// (I apologize for the const_cast, but didn't seem worth refactoring just for the freezerendering command)
	bHasRequestedToggleFreeze = const_cast<FRenderTarget*>(InViewFamily->RenderTarget)->HasToggleFreezeCommand();
#endif

#if CONSOLE
	bUseDepthOnlyPass = TRUE;
#else
	// Use a depth only pass if we are using full blown directional lightmaps
	// Otherwise base pass pixel shaders will be cheap and there will be little benefit to rendering a depth only pass
	bUseDepthOnlyPass = GSystemSettings.bAllowDirectionalLightMaps;
#endif
}

/**
 * Helper structure for sorted shadow stats.
 */
struct FSortedShadowStats
{
	/** Light/ primitive interaction.												*/
	FLightPrimitiveInteraction* Interaction;
	/** Shadow stat.																*/
	FCombinedShadowStats		ShadowStat;
};

/** Comparison operator used by sort. Sorts by light and then by pass number.	*/
IMPLEMENT_COMPARE_CONSTREF( FSortedShadowStats, SceneRendering,	{ if( B.Interaction->GetLightId() == A.Interaction->GetLightId() ) { return A.ShadowStat.ShadowPassNumber - B.ShadowStat.ShadowPassNumber; } else { return A.Interaction->GetLightId() - B.Interaction->GetLightId(); } } )

/**
 * Destructor, stringifying stats if stats gathering was enabled. 
 */
FSceneRenderer::~FSceneRenderer()
{
	if(Scene)
	{
		// Destruct the projected shadow infos.
		for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
		{
			FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightIt.GetIndex());
			for(INT ShadowIndex = 0;ShadowIndex < VisibleLightInfo.ProjectedShadows.Num();ShadowIndex++)
			{
				VisibleLightInfo.ProjectedShadows(ShadowIndex)->~FProjectedShadowInfo();
			}
			VisibleLightInfo.ProjectedShadows.Empty();
		}
	}

#if STATS
	// Stringify stats.
	if( bShouldGatherDynamicShadowStats )
	{
		check(ViewFamily.DynamicShadowStats);

		// Move from TMap to TArray and sort shadow stats.
		TArray<FSortedShadowStats> SortedShadowStats;
		for( TMap<FLightPrimitiveInteraction*,FCombinedShadowStats,SceneRenderingSetAllocator>::TIterator It(InteractionToDynamicShadowStatsMap); It; ++It )
		{
			FSortedShadowStats SortedStat;
			SortedStat.Interaction	= It.Key();
			SortedStat.ShadowStat	= It.Value();
			SortedShadowStats.AddItem( SortedStat );
		}

		// Only sort if there is something to sort.
		if( SortedShadowStats.Num() )
		{
			Sort<USE_COMPARE_CONSTREF(FSortedShadowStats,SceneRendering)>( SortedShadowStats.GetTypedData(), SortedShadowStats.Num() );
		}

		const ULightComponent* PreviousLightComponent = NULL;

		// Iterate over sorted list and stringify entries.
		for( INT StatIndex=0; StatIndex<SortedShadowStats.Num(); StatIndex++ )
		{
			const FSortedShadowStats&	SortedStat		= SortedShadowStats(StatIndex);
			FLightPrimitiveInteraction* Interaction		= SortedStat.Interaction;
			const FCombinedShadowStats&	ShadowStat		= SortedStat.ShadowStat;
			const ULightComponent*		LightComponent	= Interaction->GetLight()->LightComponent;
			
			// Only emit light row if the light has changed.
			if( PreviousLightComponent != LightComponent )
			{
				PreviousLightComponent = LightComponent;
				FDynamicShadowStatsRow StatsRow;
				
				// Figure out light name and add it.
				FString	LightName;
				if( LightComponent->GetOwner() )
				{
					LightName = LightComponent->GetOwner()->GetName();
				}
				else
				{
					LightName = FString(TEXT("Ownerless ")) + LightComponent->GetClass()->GetName();
				}
				new(StatsRow.Columns) FString(*LightName);
				
				// Add radius information for pointlight derived classes.
				FString Radius;
				if( LightComponent->IsA(UPointLightComponent::StaticClass()) )
				{
					Radius = FString::Printf(TEXT("Radius: %i"),appTrunc(((UPointLightComponent*)LightComponent)->Radius));
				}
				new(StatsRow.Columns) FString(*Radius);

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Class name.
				new(StatsRow.Columns) FString(LightComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString(*LightComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")));

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}

			// Subjects
			for( INT SubjectIndex=0; SubjectIndex<ShadowStat.SubjectPrimitives.Num(); SubjectIndex++ )
			{
				const FPrimitiveSceneInfo*	PrimitiveSceneInfo	= ShadowStat.SubjectPrimitives(SubjectIndex);
				UPrimitiveComponent*		PrimitiveComponent	= PrimitiveSceneInfo->Component;
				FDynamicShadowStatsRow		StatsRow;

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Figure out actor name and add it.
				FString	PrimitiveName;
				if( PrimitiveComponent->GetOwner() )
				{
					PrimitiveName = PrimitiveComponent->GetOwner()->GetName();
				}
				else if( PrimitiveComponent->IsA(UModelComponent::StaticClass()) )
				{
					PrimitiveName = FString(TEXT("BSP"));
				}
				else
				{
					PrimitiveName = FString(TEXT("Ownerless"));
				}
				new(StatsRow.Columns) FString(*PrimitiveName);

				// Shadow type.
				FString ShadowType;
				if( ShadowStat.ShadowResolution != INDEX_NONE )
				{
					ShadowType = FString::Printf(TEXT("Projected (Res: %i)"), ShadowStat.ShadowResolution);
				}
				else
				{
					ShadowType = TEXT("Volume");
				}
				new(StatsRow.Columns) FString(*ShadowType);

				// Shadow pass number.
				FString ShadowPassNumber = TEXT("");
				if( ShadowStat.ShadowResolution != INDEX_NONE )
				{
					if( ShadowStat.ShadowPassNumber != INDEX_NONE )
					{
						ShadowPassNumber = FString::Printf(TEXT("%i"),ShadowStat.ShadowPassNumber);
					}
					else
					{
						ShadowPassNumber = TEXT("N/A");
					}
				}
				new(StatsRow.Columns) FString(*ShadowPassNumber);

				// Class name.
				new(StatsRow.Columns) FString(PrimitiveComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString( PrimitiveComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")) + TEXT( " " ) + PrimitiveComponent->GetDetailedInfo() );

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}

			// PreShadow
			for( INT PreShadowIndex=0; PreShadowIndex<ShadowStat.PreShadowPrimitives.Num(); PreShadowIndex++ )
			{
				const FPrimitiveSceneInfo*	PrimitiveSceneInfo	= ShadowStat.PreShadowPrimitives(PreShadowIndex);
				UPrimitiveComponent*		PrimitiveComponent	= PrimitiveSceneInfo->Component;
				FDynamicShadowStatsRow		StatsRow;

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Figure out actor name and add it.
				FString	PrimitiveName;
				if( PrimitiveComponent->GetOwner() )
				{
					PrimitiveName = PrimitiveComponent->GetOwner()->GetName();
				}
				else if( PrimitiveComponent->IsA(UModelComponent::StaticClass()) )
				{
					PrimitiveName = FString(TEXT("BSP"));
				}
				else
				{
					PrimitiveName = FString(TEXT("Ownerless"));
				}
				new(StatsRow.Columns) FString(*PrimitiveName);

				// Empty column.
				new(StatsRow.Columns) FString(TEXT(""));

				// Class name.
				new(StatsRow.Columns) FString(PrimitiveComponent->GetClass()->GetName());

				// Fully qualified name.
				new(StatsRow.Columns) FString(*PrimitiveComponent->GetPathName().Replace(PLAYWORLD_PACKAGE_PREFIX,TEXT("")));

				// Add row.
				INT Index = ViewFamily.DynamicShadowStats->AddZeroed();
				(*ViewFamily.DynamicShadowStats)(Index) = StatsRow;
			}
		}
	}
#endif
}

/**
 * Helper for InitViews to detect large camera movement, in both angle and position.
 */
static bool IsLargeCameraMovement(FSceneView& View, const FMatrix& PrevViewMatrix, const FVector& PrevViewOrigin, FLOAT CameraRotationThreshold, FLOAT CameraTranslationThreshold)
{
	FLOAT RotationThreshold = appCos(CameraRotationThreshold * PI / 180.0f);
	FLOAT ViewRightAngle = View.ViewMatrix.GetColumn(0) | PrevViewMatrix.GetColumn(0);
	FLOAT ViewUpAngle = View.ViewMatrix.GetColumn(1) | PrevViewMatrix.GetColumn(1);
	FLOAT ViewDirectionAngle = View.ViewMatrix.GetColumn(2) | PrevViewMatrix.GetColumn(2);

	FVector Distance = FVector(View.ViewOrigin) - PrevViewOrigin;
	return 
		ViewRightAngle < RotationThreshold ||
		ViewUpAngle < RotationThreshold ||
		ViewDirectionAngle < RotationThreshold ||
		Distance.SizeSquared() > CameraTranslationThreshold * CameraTranslationThreshold;
}

extern UINT GTilingMode;

/**
* Helper used to set device viewports for Render*Pass
*/
void FSceneRenderer::ViewSetViewport(UINT ViewIndex, UBOOL bTiledRenderingPass, UBOOL bReverseZ)
{
	FViewInfo &View = Views(ViewIndex);
	FLOAT Z = bReverseZ ? 0.0001f : 1.0f;

	RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,Z);
	
	if( GUseTilingCode &&
		bTiledRenderingPass )
	{
		RHIMSAAFixViewport();
	}
}

/**
 * Initialize scene's views.
 * Check visibility, sort translucent items, etc.
 */
void FSceneRenderer::InitViews()
{
	SCOPED_DRAW_EVENT(InitViews)(DEC_SCENE_ITEMS,TEXT("InitViews"));

	SCOPE_CYCLE_COUNTER(STAT_InitViewsTime);

	// Setup motion blur parameters (also check for camera movement thresholds)
	PreviousFrameTime = ViewFamily.CurrentRealTime;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		FSceneViewState* ViewState = (FSceneViewState*) View.State;
		static UBOOL bEnableTimeScale = TRUE;

		// We can't use LatentOcclusionQueries when doing TiledScreenshot because in that case
		// 1-frame lag = 1-tile lag = clipped geometry, so we turn off occlusion queries
		// Occlusion culling is also disabled for hit proxy rendering.
		extern UBOOL GIsTiledScreenshot;
		const UBOOL bIsHitTesting = (ViewFamily.ShowFlags & SHOW_HitProxies) != 0;
		if (GIsTiledScreenshot || (GGameScreenshotCounter != 0) || GIgnoreAllOcclusionQueries || bIsHitTesting)
		{
			View.bDisableQuerySubmissions = TRUE;
			View.bIgnoreExistingQueries = TRUE;
		}

		if ( ViewState )
		{
			// determine if we are initializing or we should reset the persistent state
			const FLOAT DeltaTime = View.Family->CurrentRealTime - ViewState->LastRenderTime;
			const UBOOL bFirstFrameOrTimeWasReset = DeltaTime < -0.0001f || ViewState->LastRenderTime < 0.0001f;

			// detect conditions where we should reset occlusion queries
			if (bFirstFrameOrTimeWasReset || 
				ViewState->LastRenderTime + GEngine->PrimitiveProbablyVisibleTime < View.Family->CurrentRealTime ||
				IsLargeCameraMovement(
					View, 
				    ViewState->PrevViewMatrixForOcclusionQuery, 
				    ViewState->PrevViewOriginForOcclusionQuery, 
				    GEngine->CameraRotationThreshold, GEngine->CameraTranslationThreshold))
			{
				View.bIgnoreExistingQueries = TRUE;
			}
			ViewState->PrevViewMatrixForOcclusionQuery = View.ViewMatrix;
			ViewState->PrevViewOriginForOcclusionQuery = View.ViewOrigin;


			// detect conditions where we should reset motion blur 
			if (View.bRequiresPrevTransforms)
			{
				if (bFirstFrameOrTimeWasReset || 
					IsLargeCameraMovement(View, 
					ViewState->PrevViewMatrix, ViewState->PrevViewOrigin, 
					View.MotionBlurParameters.RotationThreshold, 
					View.MotionBlurParameters.TranslationThreshold))
				{
					ViewState->PrevProjMatrix				= View.ProjectionMatrix;
					ViewState->PendingPrevProjMatrix		= View.ProjectionMatrix;
					ViewState->PrevViewMatrix				= View.ViewMatrix;
					ViewState->PendingPrevViewMatrix		= View.ViewMatrix;
					ViewState->PrevViewOrigin				= View.ViewOrigin;
					ViewState->PendingPrevViewOrigin		= View.ViewOrigin;
					ViewState->MotionBlurTimeScale			= 1.0f;

					// PT: If the motion blur shader is the last shader in the post-processing chain then it is the one that is
					//     adjusting for the viewport offset.  So it is always required and we can't just disable the work the
					//     shader does.  The correct fix would be to disable the effect when we don't need it and to properly mark
					//     the uber-postprocessing effect as the last effect in the chain.

					//View.bRequiresVelocities				= FALSE;
					View.bPrevTransformsReset				= TRUE;
				}
				else
				{
					// New frame detected.
					ViewState->PrevViewMatrix				= ViewState->PendingPrevViewMatrix;
					ViewState->PrevViewOrigin				= ViewState->PendingPrevViewOrigin;
					ViewState->PrevProjMatrix				= ViewState->PendingPrevProjMatrix;
					ViewState->PendingPrevProjMatrix		= View.ProjectionMatrix;
					ViewState->PendingPrevViewMatrix		= View.ViewMatrix;
					ViewState->PendingPrevViewOrigin		= View.ViewOrigin;
					ViewState->MotionBlurTimeScale			= bEnableTimeScale ? (1.0f / (DeltaTime * 30.0f)) : 1.0f;
				}

				View.PrevViewProjMatrix = ViewState->PrevViewMatrix * ViewState->PrevProjMatrix;
				View.PrevViewRotationProjMatrix = ViewState->PrevViewMatrix.RemoveTranslation() * ViewState->PrevProjMatrix;
				View.PrevViewOrigin = ViewState->PrevViewOrigin;
			}

			PreviousFrameTime = ViewState->LastRenderTime;
			ViewState->LastRenderTime = View.Family->CurrentRealTime;
		}

		/** Each view starts out rendering to the HDR scene color */
		View.bUseLDRSceneColor = FALSE;
	}

	// Allocate the visible light info.
	if (Scene->Lights.GetMaxIndex() > 0)
	{
		VisibleLightInfos.AddZeroed(Scene->Lights.GetMaxIndex());
	}

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);

		// Initialize the view elements' dynamic resources.
		View.InitDynamicResources();

		// Allocate the view's visibility maps.
		View.PrimitiveVisibilityMap.Init(FALSE,Scene->Primitives.GetMaxIndex());
		View.StaticMeshVisibilityMap.Init(FALSE,Scene->StaticMeshes.GetMaxIndex());
		View.StaticMeshOccluderMap.Init(FALSE,Scene->StaticMeshes.GetMaxIndex());
		View.StaticMeshVelocityMap.Init(FALSE,Scene->StaticMeshes.GetMaxIndex());
		View.DecalStaticMeshVisibilityMap.Init(FALSE,Scene->DecalStaticMeshes.GetMaxIndex());

		View.VisibleLightInfos.Empty(Scene->Lights.GetMaxIndex());
		for(INT LightIndex = 0;LightIndex < Scene->Lights.GetMaxIndex();LightIndex++)
		{
#ifndef _DEBUG
			if (LightIndex < Scene->Lights.GetMaxIndex()+2)
			{
				if (LightIndex > 2)
				{
					FLUSH_CACHE_LINE(&View.VisibleLightInfos(LightIndex-2));
				}
				CONSOLE_PREFETCH(&View.VisibleLightInfos(LightIndex+2));
				CONSOLE_PREFETCH(&View.VisibleLightInfos(LightIndex+1));
			}
#endif
			new(View.VisibleLightInfos) FVisibleLightViewInfo();
		}

		View.PrimitiveViewRelevanceMap.Empty(Scene->Primitives.GetMaxIndex());
		View.PrimitiveViewRelevanceMap.AddZeroed(Scene->Primitives.GetMaxIndex());
		
		// If this is the visibility-parent of other views, reset its ParentPrimitives list.
		FSceneViewState* ViewState = (FSceneViewState*)View.State;
		const UBOOL bIsParent = ViewState && ViewState->IsViewParent();
		if ( bIsParent )
		{
			ViewState->ParentPrimitives.Empty();
		}

		// Create the view's actor visibility set.
		if(View.ActorVisibilityHistory)
		{
			View.ActorVisibilitySet = new FActorVisibilitySet;
		}
	}

	// Cull the scene's primitives against the view frustum.
	INT NumOccludedPrimitives = 0;
	INT NumCulledPrimitives = 0;
	for(FScenePrimitiveOctree::TConstIterator<SceneRenderingAllocator> PrimitiveOctreeIt(Scene->PrimitiveOctree);
		PrimitiveOctreeIt.HasPendingNodes();
		PrimitiveOctreeIt.Advance())
	{
		const FScenePrimitiveOctree::FNode& PrimitiveOctreeNode = PrimitiveOctreeIt.GetCurrentNode();
		const FOctreeNodeContext& PrimitiveOctreeNodeContext = PrimitiveOctreeIt.GetCurrentContext();

		// Find children of this octree node that may contain relevant primitives.
		FOREACH_OCTREE_CHILD_NODE(ChildRef)
		{
			if(PrimitiveOctreeNode.HasChild(ChildRef))
			{
				// Check that the child node is in the frustum for at least one view.
				const FOctreeNodeContext ChildContext = PrimitiveOctreeNodeContext.GetChildContext(ChildRef);
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{
					FViewInfo& View = Views(ViewIndex);
					if(View.ViewFrustum.IntersectBox(ChildContext.Bounds.Center,ChildContext.Bounds.Extent))
					{
						PrimitiveOctreeIt.PushChild(ChildRef);
						break;
					}
				}
			}
		}

		// Find the primitives in this node that are visible.
		for(FScenePrimitiveOctree::ElementConstIt NodePrimitiveIt(PrimitiveOctreeNode.GetElementIt());NodePrimitiveIt;++NodePrimitiveIt)
		{
			const FPrimitiveSceneInfoCompact& CompactPrimitiveSceneInfo = *NodePrimitiveIt;
			PREFETCH(&CompactPrimitiveSceneInfo);
			TBitArray<> ViewVisibilityMap(FALSE, Views.Num());
			UBOOL bNeedsPreRenderView = FALSE;

			PREFETCH(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);
			PREFETCH(CompactPrimitiveSceneInfo.Proxy);

			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{		
				FViewInfo& View = Views(ViewIndex);
				FSceneViewState* ViewState = (FSceneViewState*)View.State;

#if !FINAL_RELEASE
				if( ViewState )
				{
#if !CONSOLE
					// For visibility child views, check if the primitive was visible in the parent view.
					const FSceneViewState* const ViewParent = (FSceneViewState*)ViewState->GetViewParent();
					if(ViewParent && !ViewParent->ParentPrimitives.Contains(CompactPrimitiveSceneInfo.Component) )
					{
						continue;
					}
#endif

					// For views with frozen visibility, check if the primitive is in the frozen visibility set.
					if(ViewState->bIsFrozen && !ViewState->FrozenPrimitives.Contains(CompactPrimitiveSceneInfo.Component) )
					{
						continue;
					}
				}
#endif

				// Distance to camera in perspective viewports.
				FLOAT DistanceSquared = 0.0f;

				CONSOLE_PREFETCH(&CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes);

				// Cull primitives based on distance to camera in perspective viewports and calculate distance, also used later for static
				// mesh elements.
				if( View.ViewOrigin.W > 0.0f )
				{
					// Compute the distance between the view and the primitive.
					DistanceSquared = (CompactPrimitiveSceneInfo.Bounds.Origin - View.ViewOrigin).SizeSquared();
					// Cull the primitive if it is further from the view origin than its max draw distance, or closer than its min draw distance.
					if( DistanceSquared > Square(CompactPrimitiveSceneInfo.MaxDrawDistance) || DistanceSquared < Square(CompactPrimitiveSceneInfo.MinDrawDistance) )
					{
						NumCulledPrimitives++;
						continue;
					}
				}

				// Skip primitives which are in the view's HiddenPrimitives set.
				if(View.HiddenPrimitives.Find(CompactPrimitiveSceneInfo.Component))
				{
					continue;
				}

				// Check the primitive's bounding box against the view frustum.
				if(CompactPrimitiveSceneInfo.bAlwaysVisible || View.ViewFrustum.IntersectSphere(CompactPrimitiveSceneInfo.Bounds.Origin,CompactPrimitiveSceneInfo.Bounds.SphereRadius))
				{
					INT PrimitiveId = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->Id;

					// Check whether the primitive is occluded this frame.
					UBOOL bIsDefinitelyUnoccluded = CompactPrimitiveSceneInfo.bAlwaysVisible;
					const UBOOL bIsOccluded = ViewState 
						&& !CompactPrimitiveSceneInfo.bAlwaysVisible
						&& ViewState->UpdatePrimitiveOcclusion(CompactPrimitiveSceneInfo,View,ViewFamily.CurrentRealTime,bIsDefinitelyUnoccluded)
						// Check for wireframe after updating the primitive's occlusion, so that the primitive's LastRenderTime will be updated
						&& !(ViewFamily.ShowFlags & SHOW_Wireframe);

					if(!bIsOccluded)
					{
						if (CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes.Num() > 0)
						{
							CONSOLE_PREFETCH(&CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes(0));
							CONSOLE_PREFETCH((char*)(&CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes(0)) + CACHE_LINE_SIZE);
						}

						// Compute the primitive's view relevance.
						FPrimitiveViewRelevance& ViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveId);
						ViewRelevance = CompactPrimitiveSceneInfo.Proxy->GetViewRelevance(&View);

						if(ViewRelevance.bNeedsPreRenderView)
						{
							// Allow the primitive to process the view.
							bNeedsPreRenderView = TRUE;
						}

						if(ViewRelevance.bStaticRelevance)
						{
							// Mark the primitive's static meshes as visible.
							const INT StaticMeshesNum = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes.Num();
							for(INT MeshIndex = 0;MeshIndex < StaticMeshesNum;MeshIndex++)
							{
								const FStaticMesh& StaticMesh = CompactPrimitiveSceneInfo.PrimitiveSceneInfo->StaticMeshes(MeshIndex);
								const FLOAT LODFactorDistanceSquared = DistanceSquared * Square(View.LODDistanceFactor);
								if(	LODFactorDistanceSquared >= StaticMesh.MinDrawDistanceSquared &&
									LODFactorDistanceSquared < StaticMesh.MaxDrawDistanceSquared)
								{
									// Mark static mesh as visible for rendering
									View.StaticMeshVisibilityMap(StaticMesh.Id) = TRUE;
									View.NumVisibleStaticMeshElements++;

									// If the static mesh is an occluder, check whether it covers enough of the screen to be used as an occluder.
									if(	StaticMesh.bUseAsOccluder && 
										Square(CompactPrimitiveSceneInfo.Bounds.SphereRadius) > MinScreenRadiusForDepthPrepassSquared * LODFactorDistanceSquared )
									{
										View.StaticMeshOccluderMap(StaticMesh.Id) = TRUE;
									}

									// If the static mesh is an occluder, check whether it covers enough of the screen to be used as an occluder.
									extern FLOAT MinScreenRadiusForVelocityPassSquared;
									if(	!CompactPrimitiveSceneInfo.PrimitiveSceneInfo->bStaticShadowing && 
										Square(CompactPrimitiveSceneInfo.Bounds.SphereRadius) > MinScreenRadiusForVelocityPassSquared * LODFactorDistanceSquared )
									{
										View.StaticMeshVelocityMap(StaticMesh.Id) = TRUE;
									}
								}
							}
						}
	
						if(ViewRelevance.bDynamicRelevance)
						{
							// Keep track of visible dynamic primitives.
							View.VisibleDynamicPrimitives.AddItem(CompactPrimitiveSceneInfo.PrimitiveSceneInfo);

							View.NumVisibleDynamicPrimitives++;
						}

						if( ViewRelevance.bTranslucentRelevance )
						{
							for (UINT CheckDPGIndex = 0; CheckDPGIndex < SDPG_MAX_SceneRender; CheckDPGIndex++)
							{
								if (ViewRelevance.GetDPG(CheckDPGIndex) == TRUE)
								{
									if( ViewRelevance.bTranslucentRelevance	)
									{
										// Add to set of dynamic translucent primitives
										View.TranslucentPrimSet[CheckDPGIndex].AddScenePrimitive(CompactPrimitiveSceneInfo.PrimitiveSceneInfo,View,ViewRelevance.bUsesSceneColor);

										if( ViewRelevance.bDistortionRelevance )
										{
											// Add to set of dynamic distortion primitives
											View.DistortionPrimSet[CheckDPGIndex].AddScenePrimitive(CompactPrimitiveSceneInfo.PrimitiveSceneInfo,View);
										}
									}
								}
							}
						}

						if( ViewRelevance.bDecalStaticRelevance )
						{
							// determine static mesh visibility generated by decal interactions
							for( INT DecalIdx = 0; DecalIdx < CompactPrimitiveSceneInfo.Proxy->Decals.Num(); DecalIdx++ )
							{
								FDecalInteraction* Decal = CompactPrimitiveSceneInfo.Proxy->Decals(DecalIdx);
								if( Decal &&
									Decal->DecalStaticMesh &&
									DistanceSquared >= Decal->DecalStaticMesh->MinDrawDistanceSquared && 
									DistanceSquared < Decal->DecalStaticMesh->MaxDrawDistanceSquared )
								{
									// Distance cull using decal's CullDistance (perspective views only)
									FLOAT SquaredDistanceToDecal = 0.0f;
									if( View.ViewOrigin.W > 0.0f )
									{
										// Compute the distance between the view and the decal
										SquaredDistanceToDecal = ( Decal->DecalState.Bounds.GetCenter() - View.ViewOrigin ).SizeSquared();
									}

									// Distance cull using decal's CullDistance
									const FLOAT SquaredCullDistance = Decal->DecalState.SquaredCullDistance;
									if( SquaredCullDistance == 0.0f || SquaredDistanceToDecal < SquaredCullDistance )
									{
										// Frustum check
										const FBox& DecalBounds = Decal->DecalState.Bounds;
										if( View.ViewFrustum.IntersectBox( DecalBounds.GetCenter(), DecalBounds.GetExtent() ) )
										{
											// Mark the decal static mesh as visible for rendering
											View.DecalStaticMeshVisibilityMap(Decal->DecalStaticMesh->Id) = TRUE;
										}
									}
								}
							}
						}

						if( ViewRelevance.IsRelevant() )
					    {
							if( ViewRelevance.bDecalDynamicRelevance )
							{
								// Find set of dynamic decals with different material relevancies
								UBOOL bHasDistortion = FALSE;
								UBOOL bHasOpaque = FALSE;
								UBOOL bHasTranslucent = FALSE;
								for( INT DecalIdx = 0; DecalIdx < CompactPrimitiveSceneInfo.Proxy->Decals.Num(); DecalIdx++ )
								{
									FDecalInteraction* Decal = CompactPrimitiveSceneInfo.Proxy->Decals(DecalIdx);
									if( Decal)
									{
										bHasDistortion |= Decal->DecalState.MaterialViewRelevance.bDistortion;
										bHasOpaque |= Decal->DecalState.MaterialViewRelevance.bOpaque;
										bHasOpaque |= Decal->DecalState.MaterialViewRelevance.bMasked;
										bHasTranslucent |= Decal->DecalState.MaterialViewRelevance.bTranslucency;
									}
								}
								// add relevant dynamic decals to their DPG lists
								if( bHasDistortion | bHasOpaque | bHasTranslucent )
								{
									for (UINT CheckDPGIndex = 0; CheckDPGIndex < SDPG_MAX_SceneRender; CheckDPGIndex++)
									{
										if (ViewRelevance.GetDPG(CheckDPGIndex) == TRUE)
										{
											if (bHasDistortion)
											{
												// Add to set of dynamic distortion primitives
												View.DistortionPrimSet[CheckDPGIndex].AddScenePrimitive( CompactPrimitiveSceneInfo.PrimitiveSceneInfo,View );								
											}
											if (bHasOpaque)
											{
												// Add to set of dynamic opaque decal primitives
												View.VisibleOpaqueDynamicDecalPrimitives[CheckDPGIndex].AddItem( CompactPrimitiveSceneInfo.PrimitiveSceneInfo );
											}
											if (bHasTranslucent)
											{
												// Add to set of dynamic translucent decal primitives
												View.VisibleTranslucentDynamicDecalPrimitives[CheckDPGIndex].AddItem( CompactPrimitiveSceneInfo.PrimitiveSceneInfo );
											}
										}
									}
								}
							}

							FPrimitiveSceneInfo* PrimitiveSceneInfo = CompactPrimitiveSceneInfo.PrimitiveSceneInfo;

							// This primitive is in the view frustum, view relevant, and unoccluded; it's visible.
							View.PrimitiveVisibilityMap(PrimitiveId) = TRUE;
							ViewVisibilityMap(ViewIndex) = TRUE;

							// If the primitive's static meshes need to be updated before they can be drawn, update them now.
							PrimitiveSceneInfo->ConditionalUpdateStaticMeshes();

							// Check to see if the primitive has a decal which is using a lit material
							const UBOOL bHasLitDecals = CompactPrimitiveSceneInfo.Proxy->HasLitDecals(&View) && ViewRelevance.bDecalDynamicRelevance;

							// Iterate over the lights affecting the primitive.
							for(const FLightPrimitiveInteraction* Interaction = PrimitiveSceneInfo->LightList;
								Interaction;
								Interaction = Interaction->GetNextLight()
								)
							{
								CONSOLE_PREFETCH(Interaction->GetNextLight());

								// The light doesn't need to be rendered if it only affects light-maps or if it is a skylight.
								const FLightSceneInfo* LightSceneInfo = Interaction->GetLight();
								const UBOOL bForceLightDynamic = ViewRelevance.bForceDirectionalLightsDynamic && LightSceneInfo->LightType == LightType_Directional;
								const UBOOL bIsNotSkyLight = !Interaction->IsLightMapped() && LightSceneInfo->LightType != LightType_Sky;
								const UBOOL bIsBasePassLit = LightSceneInfo->LightEnvironment 
									// Check if the current light is a directional light and it will be rendered in the base pass
									&& (PrimitiveSceneInfo->DirectionalLightSceneInfo && LightSceneInfo->LightType == LightType_Directional 
									// Or check if the current light is a SH light (the directional light must also exist), 
									|| LightSceneInfo->LightType == LightType_SphericalHarmonic && PrimitiveSceneInfo->DirectionalLightSceneInfo 
									// And it is allowed in the base pass 
									&& (PrimitiveSceneInfo->bRenderSHLightInBasePass 
									// Or if it is not allowed but the primitive is in the foreground DPG and foreground self-shadowing is disabled.
									// In this case there will not be a modulated shadow pass between the base pass and the SH light so it should be merged.
									|| PrimitiveSceneInfo->SHLightSceneInfo && ViewRelevance.GetDPG(SDPG_Foreground) && !GSystemSettings.bEnableForegroundSelfShadowing));
								// Render dynamic, non-skylight lights that are not going to be merged into the base pass.
								const UBOOL bRenderLight = (bForceLightDynamic || bIsNotSkyLight) && !bIsBasePassLit;

								if ( bRenderLight )
								{
									FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(Interaction->GetLightId());
									for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
									{
										if(ViewRelevance.GetDPG(DPGIndex))
										{
											// indicate that the light is affecting some static or dynamic lit primitives
											VisibleLightViewInfo.DPGInfo[DPGIndex].bHasVisibleLitPrimitives = TRUE;

											if( ViewRelevance.bDynamicRelevance )
											{
												// Add dynamic primitives to the light's list of visible dynamic affected primitives.
												VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleDynamicLitPrimitives.AddItem(PrimitiveSceneInfo);
											}
											if ( bHasLitDecals )
											{
												// Add to the light's list of The primitives which are visible, affected by this light and receiving lit decals.
												VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives.AddItem(PrimitiveSceneInfo);
												CompactPrimitiveSceneInfo.Proxy->InitLitDecalFlags(DPGIndex);
											}
										}
									}
								}
							}

							// If the primitive's last render time is older than last frame, consider it newly visible and update its visibility change time
							if (PrimitiveSceneInfo->LastRenderTime < ViewFamily.CurrentWorldTime - ViewFamily.DeltaWorldTime - DELTA)
							{
								PrimitiveSceneInfo->LastVisibilityChangeTime = ViewFamily.CurrentWorldTime;
							}
							PrimitiveSceneInfo->LastRenderTime = ViewFamily.CurrentWorldTime;

							// If the primitive is definitely unoccluded (rather than only estimated to be unoccluded), then update the
							// signal the game thread that it's visible.
							if(bIsDefinitelyUnoccluded)
							{
								// Update the PrimitiveComponent's LastRenderTime.
								PrimitiveSceneInfo->Component->LastRenderTime = ViewFamily.CurrentWorldTime;

								if(PrimitiveSceneInfo->Owner)
								{
									if(View.ActorVisibilitySet)
									{
										// Add the actor to the visible actor set.
										View.ActorVisibilitySet->AddActor(PrimitiveSceneInfo->Owner);
									}

									// Update the actor's LastRenderTime.
									PrimitiveSceneInfo->Owner->LastRenderTime = ViewFamily.CurrentWorldTime;
								}

								// Store the primitive for parent occlusion rendering.
								if ( ViewState && ViewState->IsViewParent() )
								{
									ViewState->ParentPrimitives.Add(PrimitiveSceneInfo->Component);
								}
								// update last render time for decals rendered on this receiver primitive
								if( PrimitiveSceneInfo->Proxy )
								{
									for( INT DecalIdx=0; DecalIdx < PrimitiveSceneInfo->Proxy->Decals.Num(); DecalIdx++ )
									{
										FDecalInteraction* DecalInteraction = PrimitiveSceneInfo->Proxy->Decals(DecalIdx);
										if( DecalInteraction && 
											DecalInteraction->Decal )
										{
											DecalInteraction->Decal->LastRenderTime = ViewFamily.CurrentWorldTime;
										}
									}
								}
							}
							#if !FINAL_RELEASE
								// if we are freezing the scene, then remember the primitive was rendered
								if (ViewState && ViewState->bIsFreezing)
								{
									ViewState->FrozenPrimitives.Add(PrimitiveSceneInfo->Component);
								}
							#endif
						}
					}
					else
					{
						NumOccludedPrimitives++;
					}
				}
				else
				{
					NumCulledPrimitives++;
				}
			}

			// If the primitive's relevance indicated it needed a PreRenderView for any view, call it now.
			if (bNeedsPreRenderView)
			{
				CompactPrimitiveSceneInfo.Proxy->PreRenderView(&ViewFamily, ViewVisibilityMap, GetGlobalSceneRenderState()->FrameNumber);
			}
		}
	}
	INC_DWORD_STAT_BY(STAT_OccludedPrimitives,NumOccludedPrimitives);
	INC_DWORD_STAT_BY(STAT_CulledPrimitives,NumCulledPrimitives);

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{		
		FViewInfo& View = Views(ViewIndex);

		// sort the translucent primitives
		for( INT DPGIndex=0; DPGIndex < SDPG_MAX_SceneRender; DPGIndex++ )
		{
			View.TranslucentPrimSet[DPGIndex].SortPrimitives();
		}
	}

	// determine visibility of each light
	for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
	{
		const FLightSceneInfoCompact& LightSceneInfoCompact = *LightIt;
		const FLightSceneInfo* LightSceneInfo = LightSceneInfoCompact.LightSceneInfo;

		// view frustum cull lights in each view
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{		
			FViewInfo& View = Views(ViewIndex);
			FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightIt.GetIndex());

			// check if the light has any visible lit primitives 
			// or uses modulated shadows since they are always rendered
			if( LightSceneInfo->LightShadowMode == LightShadow_Modulate || 
				LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter ||
				VisibleLightViewInfo.HasVisibleLitPrimitives() )
			{
				// dir lights are always visible, and point/spot only if in the frustum
				if (LightSceneInfo->LightType == LightType_Point || LightSceneInfo->LightType == LightType_Spot)
				{
					if (View.ViewFrustum.IntersectSphere(LightSceneInfo->GetOrigin(), LightSceneInfo->GetRadius()))
					{
						VisibleLightViewInfo.bInViewFrustum = TRUE;
					}
				}
				else
				{
					VisibleLightViewInfo.bInViewFrustum = TRUE;
				}
			}
		}
	}

	if( (ViewFamily.ShowFlags & SHOW_DynamicShadows) && GSystemSettings.RenderThreadSettings.bAllowDynamicShadows )
	{
		// Setup dynamic shadows.
		InitDynamicShadows();
	}

	// Initialize the fog constants.
	InitFogConstants();
}

/** 
* @return text description for each DPG 
*/
TCHAR* GetSceneDPGName( ESceneDepthPriorityGroup DPG )
{
	switch(DPG)
	{
	case SDPG_UnrealEdBackground:
		return TEXT("UnrealEd Background");
	case SDPG_World:
		return TEXT("World");
	case SDPG_Foreground:
		return TEXT("Foreground");
	case SDPG_UnrealEdForeground:
		return TEXT("UnrealEd Foreground");
	case SDPG_PostProcess:
		return TEXT("PostProcess");
	default:
		return TEXT("Unknown");
	};
}

/** 
* Renders the view family. 
*/
void FSceneRenderer::Render()
{
	// Allocate the maximum scene render target space for the current view family.
	GSceneRenderTargets.Allocate( ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY() );

	// Find the visible primitives.
	InitViews();

	const UBOOL bIsWireframe = (ViewFamily.ShowFlags & SHOW_Wireframe) != 0;

	// Whether to clear the scene color buffer before rendering color for the first DPG. When using tiling, this 
	// gets cleared later automatically.
	UBOOL bRequiresClear = !GUseTilingCode && (ViewFamily.bClearScene || bIsWireframe);

	// Determines whether to resolve scene color once right before post processing, or at the end of each DPG.
	// Not tested on PS3, but shouldn't matter anyway since PS3 does not actually do resolves (see RHICopyToResolveTarget())
#if PS3
	UBOOL bDeferPrePostProcessResolve = FALSE;
#else
	UBOOL bDeferPrePostProcessResolve = TRUE;
#endif

	// Disable the optimization if any post process effects run before SDPG_PostProcess,
	// Since they might need to read from scene color at the end of a DPG.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);
		for( INT EffectIdx=0; EffectIdx < View.PostProcessSceneProxies.Num(); EffectIdx++ )
		{
			const FPostProcessSceneProxy* PPEffectProxy = &View.PostProcessSceneProxies(EffectIdx);
			if( PPEffectProxy
				&& PPEffectProxy->GetDepthPriorityGroup() != SDPG_PostProcess
				// Post process effects that affect lighting only are expected to not require a scene color resolve (ie by using alpha blending)
				&& !PPEffectProxy->GetAffectsLightingOnly())
			{
				bDeferPrePostProcessResolve = FALSE;
				break;
			}
		}
	}

	for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		// WARNING: if any Xbox360 rendering uses SDPG_UnrealEdBackground, it will not show up with tiling enabled
		// unless you go out of your way to Resolve the scene color and Restore it at the beginning of the tiling pass.
		// SDPG_World and SDPG_Foreground work just fine, however.
		UBOOL bWorldDpg = (DPGIndex == SDPG_World);

#if XBOX
		// On Xbox, use non-tiling MSAA split screen mode for 2P horizontal; only available if we're already using tiling mode 1
		GUseMSAASplitScreen = (GEngine->GameViewport->GetCurrentSplitscreenType() == eSST_2P_HORIZONTAL) 
			&& GUseTilingCode
			&& (GTilingMode == 1)
			&& bWorldDpg
			&& Views.Num() > 1;

		if (GUseMSAASplitScreen)
		{
			// Assumption in non-tiling MSAA mode: view 0 is the top view
			check(Views(0).RenderTargetY < Views(1).RenderTargetY);
		}
#endif

		// Skip Editor-only DPGs for game rendering.
		if( GIsGame && (DPGIndex == SDPG_UnrealEdBackground || DPGIndex == SDPG_UnrealEdForeground) )
		{
			continue;
		}

		SCOPED_DRAW_EVENT(EventDPG)(DEC_SCENE_ITEMS,TEXT("DPG %s"),GetSceneDPGName((ESceneDepthPriorityGroup)DPGIndex));

		// force using occ queries for wireframe if rendering is parented or frozen in the first view
		check(Views.Num());
		UBOOL bIsOcclusionAllowed = (DPGIndex == SDPG_World) && !GIgnoreAllOcclusionQueries;
#if FINAL_RELEASE
		const UBOOL bIsViewFrozen = FALSE;
		const UBOOL bHasViewParent = FALSE;
#else
		const UBOOL bIsViewFrozen = Views(0).State && ((FSceneViewState*)Views(0).State)->bIsFrozen;
		const UBOOL bHasViewParent = Views(0).State && ((FSceneViewState*)Views(0).State)->HasViewParent();
#endif
		const UBOOL bIsOcclusionTesting = bIsOcclusionAllowed && (!bIsWireframe || bIsViewFrozen || bHasViewParent);

		if (!GUseMSAASplitScreen)
		{
			if( GUseTilingCode && bWorldDpg )
			{
				RHIMSAAInitPrepass();
			}
			// Draw the scene pre-pass
			RenderPrePass(DPGIndex,bIsOcclusionTesting, -1);

			if( GUseTilingCode && bWorldDpg )
			{
				RHIMSAABeginRendering(ViewFamily.bClearScene || bIsWireframe);
			}
			else
			{
				if ( CanBlendWithFPRenderTarget(GRHIShaderPlatform) || !bWorldDpg )
				{
					// Begin rendering to scene color
					GSceneRenderTargets.BeginRenderingSceneColor();
				}
				else
				{
					// Begin rendering to the HDR scene color scratch pad for the world DPG in SM2
					GSceneRenderTargets.BeginRenderingSceneColorScratch();
					// Force a clear of the scratch pad for this DPG in the editor for SM2
					// @todo - support the background DPG in the editor for SM2 by copying scene color to the scratch pad
					bRequiresClear |= GIsEditor;
				}
			}
		}

		UBOOL bBasePassDirtiedColor = FALSE;

		// Clear scene color buffer if necessary.
		if( bRequiresClear )
		{
			SCOPED_DRAW_EVENT( EventClear )( DEC_SCENE_ITEMS, TEXT( "ClearView" ) );

			// Clear the entire viewport to make sure no post process filters in any data from an invalid region
			RHISetViewport( 0, 0, 0.0f, ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY(), 1.0f);
			RHIClear( TRUE, FLinearColor::Black, FALSE, 0, FALSE, 0 );

			// Clear the viewports to their background color
			if( GIsEditor )
			{
				// Clear each viewport to its own background color
				for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
				{
					SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("ClearView%d"),ViewIndex);

					FViewInfo& View = Views(ViewIndex);

					// Set the device viewport for the view.
					RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);

					// Clear the scene color surface when rendering the first DPG.
					RHIClear(TRUE,View.BackgroundColor,FALSE,0,FALSE,0);
				}

				// Clear the depths to max depth so that depth bias blend materials always show up
				ClearSceneColorDepth();
			}
			// Only clear once.
			bRequiresClear = FALSE;
		}

		if ( ViewFamily.ShowFlags & SHOW_TextureDensity )
		{
			// Override the base pass with the texture density pass if the viewmode is enabled.
			bBasePassDirtiedColor |= RenderTextureDensities(DPGIndex);
		}
		else
		{
			// Draw the base pass pass for all visible primitives in this DPG.
			// First base pass is for the Begin/End tiling block
			bBasePassDirtiedColor |= RenderBasePass(DPGIndex,TRUE);
		}

		if ( bWorldDpg )
		{
			// Extract LDR scene color from the HDR scratch pad that the world DPG base pass was rendered to in SM2
			ExtractLDRSceneColor(DPGIndex);
		}

		UBOOL bSceneColorDirty = bBasePassDirtiedColor;

		if( GUseTilingCode && bWorldDpg )
		{
			if (!GUseMSAASplitScreen)
			{
				RHISetBlendState( TStaticBlendState<>::GetRHI());
				RHIMSAAEndRendering(GSceneRenderTargets.GetSceneDepthTexture(), GSceneRenderTargets.GetSceneColorTexture(), 0);

				// Scenecolor has been resolved now.
				bSceneColorDirty = FALSE;
			}
		}
#if !XBOX
		else if (CanBlendWithFPRenderTarget(GRHIShaderPlatform))
		{
			// For SM3 platforms that store depth in scene color alpha, this resolve is needed so shadow passes can read the scene depth.
			GSceneRenderTargets.FinishRenderingSceneColor(bBasePassDirtiedColor||bWorldDpg, FResolveParams(0, 0, FamilySizeX, FamilySizeY));

			// Scenecolor has been resolved now.
			bSceneColorDirty = FALSE;
		}
#endif

		if( bWorldDpg )
		{
			if( GUseTilingCode )
			{
				// Need to set the render targets before we restore the color and depth data
				GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
				RHIRestoreColorDepth(GSceneRenderTargets.GetSceneColorTexture(), GSceneRenderTargets.GetSceneDepthTexture());
				if (!GUseMSAASplitScreen)
				{
					// render a second pass for base pass items which should not use MSAA predicated tiling
					// this will include anything which uses dynamic data that needs to allocate space on the command buffer
					// since that could easily cause the static buffer used for tiling to run out of space
					UBOOL bBasePassDirtiedColorUntiled = RenderBasePass(DPGIndex,FALSE);
					bBasePassDirtiedColor |= bBasePassDirtiedColorUntiled;
					bSceneColorDirty |= bBasePassDirtiedColorUntiled;

					if( bBasePassDirtiedColorUntiled )
					{
						// resolve depth buffer
						GSceneRenderTargets.ResolveSceneDepthTexture(); 
					}				
				}
			}
			else
			{
				// Scene depth values resolved to scene depth texture
				// only resolve depth values from world dpg
				GSceneRenderTargets.ResolveSceneDepthTexture();
			}
		}

		if ( bIsOcclusionTesting )
		{
			// Issue occlusion tests if they were not already done before the base pass
			BeginOcclusionTests();
		}

		if(ViewFamily.ShowFlags & SHOW_Lighting)
		{
			{
				SCOPED_DRAW_EVENT(EventShadowedLights)(DEC_SCENE_ITEMS,TEXT("ShadowedLights"));
				// Render the scene lights that are affected by modulated shadows.
				bSceneColorDirty |= RenderLights(DPGIndex,TRUE,bSceneColorDirty);
			}

			if( !(ViewFamily.ShowFlags & SHOW_ShaderComplexity) 
				&& ViewFamily.ShowFlags & SHOW_DynamicShadows
				&& GSystemSettings.RenderThreadSettings.bAllowDynamicShadows )
			{
				// Render the modulated shadows.
				bSceneColorDirty |= RenderModulatedShadows(DPGIndex);
			}

			{
				SCOPED_DRAW_EVENT(EventUnshadowedLights)(DEC_SCENE_ITEMS,TEXT("UnshadowedLights"));
				// Render the scene lights that aren't affected by modulated shadows.
				bSceneColorDirty |= RenderLights(DPGIndex,FALSE,bSceneColorDirty);
			}
		}

		if( ViewFamily.ShowFlags & SHOW_Decals )
		{
			GSceneRenderTargets.BeginRenderingSceneColor();

			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				const FViewInfo& View = Views(ViewIndex);

				// Set the device viewport for the view and the global shader params for the view
				RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
				RHISetViewParameters(&View, View.TranslatedViewProjectionMatrix, View.ViewOrigin);

				// render translucent decals 
				bSceneColorDirty |= RenderDecals(View,DPGIndex,TRUE);
			}

			GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
		}

		// Update the quarter-sized depth buffer with the current contents of the scene depth texture.
		// This needs to happen after modulated shadows, whose render targets overlap with the small depth surface in EDRAM,
		// And before ambient occlusion, which makes use of the small depth buffer.
		if ( bWorldDpg )
		{
			GSceneRenderTargets.UpdateSmallDepthSurface();
		}

		if(ViewFamily.ShowFlags & SHOW_Lighting)
		{
			// Render post process effects that affect lighting only
			bSceneColorDirty |= RenderPostProcessEffects(DPGIndex, TRUE);
		}

		if(ViewFamily.ShowFlags & SHOW_Fog)
		{
			// Render the scene fog.
			bSceneColorDirty |= RenderFog(DPGIndex);
		}

		if( ViewFamily.ShowFlags & SHOW_UnlitTranslucency )
		{
			// Distortion pass
			bSceneColorDirty |= RenderDistortion(DPGIndex);
		}

		// Only resolve here if scene color is dirty and we don't need to resolve at the end of each DPG
		if(bSceneColorDirty && !bDeferPrePostProcessResolve)
		{
			// Save the color buffer if any uncommitted changes have occurred
			GSceneRenderTargets.ResolveSceneColor(FResolveParams(0, 0, FamilySizeX, FamilySizeY));
			bSceneColorDirty = FALSE;
		}

		if( ViewFamily.ShowFlags & SHOW_UnlitTranslucency )
		{
			SCOPE_CYCLE_COUNTER(STAT_TranslucencyDrawTime);
			// Translucent pass.
			const UBOOL bTranslucencyDirtiedColor = RenderTranslucency( DPGIndex );

			// If any translucent elements were drawn, render post-fog decals for translucent receivers.
			if( bTranslucencyDirtiedColor )
			{
				// Only need to resolve if a PP effect might need to read from scene color at the end of the DPG.
				if ( !bDeferPrePostProcessResolve )
				{
					// Finish rendering scene color after rendering translucency for this DPG.
					GSceneRenderTargets.FinishRenderingSceneColor( TRUE, FResolveParams(0, 0, FamilySizeX, FamilySizeY) );
				}
			}
		}

		// Render the velocities of movable objects for the motion blur effect.
		if( bWorldDpg && GSystemSettings.RenderThreadSettings.bAllowMotionBlur )
		{
			RenderVelocities(DPGIndex);
		}

		// post process effects pass for scene DPGs
		RenderPostProcessEffects(DPGIndex);
	}

	// If all post process effects are in SDPG_PostProcess, then only one resolve is necessary after the scene is rendered.
	if( bDeferPrePostProcessResolve )
	{
		GSceneRenderTargets.ResolveSceneColor(FResolveParams(0, 0, FamilySizeX, FamilySizeY));
	}

	// post process effects pass for post process DPGs
	RenderPostProcessEffects(SDPG_PostProcess);

	// Finish rendering for each view.
	{
		SCOPED_DRAW_EVENT(EventFinish)(DEC_SCENE_ITEMS,TEXT("FinishRendering"));
		
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{	
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

			FinishRenderViewTarget(&Views(ViewIndex));
		}
	}

#if !FINAL_RELEASE
	// display a message saying we're frozen
	{
		SCOPED_DRAW_EVENT(EventFrozenText)(DEC_SCENE_ITEMS,TEXT("FrozenText"));

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{	
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

			FSceneViewState* ViewState = (FSceneViewState*)Views(ViewIndex).State;
			if (ViewState && (ViewState->HasViewParent()
				|| ViewState->bIsFrozen
				))
			{
				// this is a helper class for FCanvas to be able to get screen size
				class FRenderTargetFreeze : public FRenderTarget
				{
				public:
					UINT SizeX, SizeY;
					FRenderTargetFreeze(UINT InSizeX, UINT InSizeY)
						: SizeX(InSizeX), SizeY(InSizeY)
					{}
					UINT GetSizeX() const
					{
						return SizeX;
					};
					UINT GetSizeY() const
					{
						return SizeY;
					};
				} TempRenderTarget(Views(ViewIndex).RenderTargetSizeX, Views(ViewIndex).RenderTargetSizeY);

				// create a temporary FCanvas object with the temp render target
				// so it can get the screen size
				FCanvas Canvas(&TempRenderTarget, NULL);
				const FString StateText =
					ViewState->bIsFrozen ?
					Localize(TEXT("ViewportStatus"),TEXT("RenderingFrozenE"),TEXT("Engine"))
					:
				Localize(TEXT("ViewportStatus"),TEXT("OcclusionChild"),TEXT("Engine"));
				DrawShadowedString(&Canvas, 10, 130, *StateText, GEngine->GetSmallFont(), FLinearColor(0.8,1.0,0.2,1.0));
				Canvas.Flush();
			}
		}
	}
#endif //!FINAL_RELEASE

	// Save the actor and primitive visibility states for the game thread.
	SaveVisibilityState();

	// Save the post-occlusion visibility stats for the frame and freezing info
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		INC_DWORD_STAT_BY(STAT_VisibleStaticMeshElements,View.NumVisibleStaticMeshElements);
		INC_DWORD_STAT_BY(STAT_VisibleDynamicPrimitives,View.NumVisibleDynamicPrimitives);

#if !FINAL_RELEASE
		// update freezing info
		FSceneViewState* ViewState = (FSceneViewState*)View.State;
		if (ViewState)
		{
			// if we're finished freezing, now we are frozen
			if (ViewState->bIsFreezing)
			{
				ViewState->bIsFreezing = FALSE;
				ViewState->bIsFrozen = TRUE;
			}

			// handle freeze toggle request
			if (bHasRequestedToggleFreeze)
			{
				// do we want to start freezing?
				if (!ViewState->bIsFrozen)
				{
					ViewState->bIsFrozen = FALSE;
					ViewState->bIsFreezing = TRUE;
					ViewState->FrozenPrimitives.Empty();
				}
				// or stop?
				else
				{
					ViewState->bIsFrozen = FALSE;
					ViewState->bIsFreezing = FALSE;
					ViewState->FrozenPrimitives.Empty();
				}
			}
		}
#endif
	}

#if !FINAL_RELEASE
	// clear the commands
	bHasRequestedToggleFreeze = FALSE;
#endif
}

/** Renders only the final post processing for the view */
void FSceneRenderer::RenderPostProcessOnly() 
{
	// post process effects passes
	for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		RenderPostProcessEffects(DPGIndex);
	}	
	RenderPostProcessEffects(SDPG_PostProcess);

	// Finish rendering for each view.
	{
		SCOPED_DRAW_EVENT(EventFinish)(DEC_SCENE_ITEMS,TEXT("FinishRendering"));
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)		
		{	
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
			FinishRenderViewTarget(&Views(ViewIndex));
		}
	}
}

/** Renders the scene's prepass and occlusion queries */
UBOOL FSceneRenderer::RenderPrePass(UINT DPGIndex,UBOOL bIsOcclusionTesting,UINT ViewIndex)
{
	SCOPED_DRAW_EVENT(EventPrePass)(DEC_SCENE_ITEMS,TEXT("PrePass"));

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty=0;

	if( !GUseTilingCode || !bWorldDpg )
	{
		GSceneRenderTargets.BeginRenderingPrePass();
	}

	// If no view was specified, loop through all views
	if (ViewIndex == -1)
	{
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			bDirty |= RenderPrePassInner(DPGIndex, bIsOcclusionTesting, ViewIndex);
		}
	}
	else
	{
		bDirty |= RenderPrePassInner(DPGIndex, bIsOcclusionTesting, ViewIndex);
	}

	if( !GUseTilingCode || !bWorldDpg )
	{
		GSceneRenderTargets.FinishRenderingPrePass();
	}

	return bDirty;
}

/** Renders the scene's prepass and occlusion queries */
UBOOL FSceneRenderer::RenderPrePassInner(UINT DPGIndex,UBOOL bIsOcclusionTesting,UINT ViewIndex)
{
	SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

	UBOOL bDirty=0;
	FViewInfo &View = Views(ViewIndex);
	UBOOL bWorldDpg = (DPGIndex == SDPG_World);

	RHISetViewParameters(  &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

	// Set the device viewport for the view.
	ViewSetViewport(ViewIndex, DPGIndex == SDPG_World, FALSE);

	//@GEMINI_TODO: the Editor currently relies on the prepass clearing the depth.
	if( GIsEditor || bIsOcclusionTesting || (DPGIndex == SDPG_World) || (DPGIndex == SDPG_Foreground) )
	{
		// Clear the depth buffer as required
		RHIClear(FALSE,FLinearColor::Black,TRUE,1.0f,TRUE,0);
	}

	// Opaque blending, depth tests and writes.
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());

	// Draw a depth pass to avoid overdraw in the other passes.
	if(bUseDepthOnlyPass)
	{
		// Write the depths of primitives which were unoccluded the previous frame.
		{
			SCOPE_CYCLE_COUNTER(STAT_DepthDrawTime);
#if !XBOX
			// Draw foreground occluders in the world DPG to maximize ZCull efficiency during the world BasePass
			if( bWorldDpg )
			{
				// Set the device viewport for the view.  Set the z range so that the foreground primitives will be in front of everything in the world.
				ViewSetViewport(ViewIndex, DPGIndex == SDPG_World, TRUE);				

				SCOPED_DRAW_EVENT(EventForegroundInWorldPrePass)(DEC_SCENE_ITEMS,TEXT("ForegroundOccluders"));
				bDirty |= RenderDPGPrePass(SDPG_Foreground, View);
			}
#endif

			// Set the device viewport for the view, so we only clear our portion of the screen.
			ViewSetViewport(ViewIndex, DPGIndex == SDPG_World, FALSE);

			// Draw this DPG's occluders
			bDirty |= RenderDPGPrePass(DPGIndex, View);
		}
	}
	return bDirty;
}

/**
 * Renders the prepass for the given DPG and View.
 */
UBOOL FSceneRenderer::RenderDPGPrePass(UINT DPGIndex, FViewInfo& View)
{
	UBOOL bDirty = FALSE;
	// Draw the static occluder primitives using a depth drawing policy.
	{
		// Draw opaque occluders which support a separate position-only vertex buffer to minimize vertex fetch bandwidth,
		// which is often the bottleneck during the depth only pass.
		SCOPED_DRAW_EVENT(EventPosOnly)(DEC_SCENE_ITEMS,TEXT("PosOnly Opaque"));
		bDirty |= Scene->DPGs[DPGIndex].PositionOnlyDepthDrawList.DrawVisible(View,View.StaticMeshOccluderMap);
	}
	{
		// Draw opaque occluders, using double speed z where supported.
		SCOPED_DRAW_EVENT(EventOpaque)(DEC_SCENE_ITEMS,TEXT("Opaque"));
		bDirty |= Scene->DPGs[DPGIndex].DepthDrawList.DrawVisible(View,View.StaticMeshOccluderMap);
	}

	// Draw the dynamic occluder primitives using a depth drawing policy.
	TDynamicPrimitiveDrawer<FDepthDrawingPolicyFactory> Drawer(&View,DPGIndex,DDM_NonMaskedOnly,TRUE);
	{
		SCOPED_DRAW_EVENT(EventDynamic)(DEC_SCENE_ITEMS,TEXT("Dynamic"));
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			const FLOAT LODFactorDistanceSquared = (PrimitiveSceneInfo->Bounds.Origin - View.ViewOrigin).SizeSquared() * Square(View.LODDistanceFactor);
			if( PrimitiveSceneInfo->bUseAsOccluder &&
				PrimitiveViewRelevance.GetDPG(DPGIndex) && 
				Square(PrimitiveSceneInfo->Bounds.SphereRadius) > MinScreenRadiusForDepthPrepassSquared * LODFactorDistanceSquared &&
				// Only draw opaque primitives
				PrimitiveViewRelevance.bOpaqueRelevance
				)
			{
				Drawer.SetPrimitive(PrimitiveSceneInfo);
				PrimitiveSceneInfo->Proxy->DrawDynamicElements(
					&Drawer,
					&View,
					DPGIndex
					);
			}
		}
	}
	bDirty |= Drawer.IsDirty();
	return bDirty;
}

/**
 * Renders the scene's base pass 
 *
 * @param DPGIndex - current depth priority group index
 * @param bIsTiledRendering - TRUE if currently within a Begin/End tiled rendering block
 * @return TRUE if anything was rendered
 */
UBOOL FSceneRenderer::RenderBasePass(UINT DPGIndex, UBOOL bIsTiledRendering)
{
	SCOPED_DRAW_EVENT(EventBasePass)(DEC_SCENE_ITEMS,TEXT("BasePass"));

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty=0;

	// only allow rendering of mesh elements with dynamic data if not rendering in a Begin/End tiling block (only applies to world dpg and if tiling is enabled)
	UBOOL bRenderDynamicData = !bIsTiledRendering || !bWorldDpg || !GUseTilingCode;
	// only allow rendering of mesh elements with static data if within the Begin/End tiling block (only applies to world dpg and if tiling is enabled)
	UBOOL bRenderStaticData = bIsTiledRendering || !bWorldDpg || !GUseTilingCode;

	// in non-tiled MSAA split screen mode, render both dynamic & static data
	if (GUseMSAASplitScreen)
	{
		bRenderDynamicData = bRenderStaticData = TRUE;
	}

	// Draw the scene's emissive and light-map color.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
		SCOPE_CYCLE_COUNTER(STAT_BasePassDrawTime);

		FViewInfo& View = Views(ViewIndex);

		// In non tiled MSAA split screen mode, do a per-view Z prepass
		if (GUseMSAASplitScreen)
		{
			RHIMSAABeginRendering(FALSE);
			RenderPrePass(DPGIndex,TRUE,ViewIndex);
		}

		// Opaque blending, depth tests and writes.
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
 
		if (!CanBlendWithFPRenderTarget(GRHIShaderPlatform) && !bWorldDpg)
		{
			// Only allow alpha writes during the World DPG in SM2, since they will overwrite the luminance scale in other DPG's
			RHISetColorWriteMask(CW_RGB);
		}

		// Set the device viewport for the view.
		ViewSetViewport(ViewIndex, DPGIndex == SDPG_World && bIsTiledRendering, FALSE);
		RHISetViewParameters(&View, View.TranslatedViewProjectionMatrix, View.ViewOrigin);

		if( bRenderStaticData )
		{
			// render opaque decals
			bDirty |= RenderDecals(View,DPGIndex,FALSE);
		}

		if( bRenderStaticData )
		{
			// Draw the scene's base pass draw lists.
			FDepthPriorityGroup::EBasePassDrawListType MaskedDrawType = FDepthPriorityGroup::EBasePass_Masked;
			{
				SCOPED_DRAW_EVENT(EventStaticNoLightmap)(DEC_SCENE_ITEMS,TEXT("StaticMaskedNoLightmap"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassNoLightMapDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticMaskedVertexLightmap)(DEC_SCENE_ITEMS,TEXT("StaticMaskedVertexLightmapped"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalVertexLightMapDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSimpleVertexLightMapDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticMaskedTextureLightmap)(DEC_SCENE_ITEMS,TEXT("StaticMaskedTextureLightmapped"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalLightMapTextureDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSimpleLightMapTextureDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticDynamicallyLit)(DEC_SCENE_ITEMS,TEXT("StaticMaskedDynamicallyLit"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalLightDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSHLightDrawList[MaskedDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}

			FDepthPriorityGroup::EBasePassDrawListType OpaqueDrawType = FDepthPriorityGroup::EBasePass_Default;
			{
				SCOPED_DRAW_EVENT(EventStaticNoLightmap)(DEC_SCENE_ITEMS,TEXT("StaticOpaqueNoLightmap"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassNoLightMapDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticVertexLightmap)(DEC_SCENE_ITEMS,TEXT("StaticOpaqueVertexLightmapped"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalVertexLightMapDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSimpleVertexLightMapDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticTextureLightmapped)(DEC_SCENE_ITEMS,TEXT("StaticOpaqueTextureLightmapped"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalLightMapTextureDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSimpleLightMapTextureDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
			{
				SCOPED_DRAW_EVENT(EventStaticTextureLightmapped)(DEC_SCENE_ITEMS,TEXT("StaticOpaqueDynamicallyLit"));
				bDirty |= Scene->DPGs[DPGIndex].BasePassDirectionalLightDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
				bDirty |= Scene->DPGs[DPGIndex].BasePassSHLightDrawList[OpaqueDrawType].DrawVisible(View,View.StaticMeshVisibilityMap);
			}
		}

		{
			SCOPED_DRAW_EVENT(EventDynamic)(DEC_SCENE_ITEMS,TEXT("Dynamic"));

			if( View.VisibleDynamicPrimitives.Num() > 0 )
			{
				// Draw the dynamic non-occluded primitives using a base pass drawing policy.
				TDynamicPrimitiveDrawer<FBasePassOpaqueDrawingPolicyFactory> Drawer(
					&View,DPGIndex,FBasePassOpaqueDrawingPolicyFactory::ContextType(),TRUE);
				for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
				{
					const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
					const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

					const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
					const UBOOL bRelevantDPG = PrimitiveViewRelevance.GetDPG(DPGIndex) != 0;

					// Only draw the primitive if it's visible and relevant in the current DPG
					if( bVisible && bRelevantDPG && 
						// only draw opaque and masked primitives if wireframe is disabled
						(PrimitiveViewRelevance.bOpaqueRelevance || ViewFamily.ShowFlags & SHOW_Wireframe)
						// only draw static prims or dynamic prims based on tiled block (see above)
						&& (bRenderStaticData || bRenderDynamicData && PrimitiveViewRelevance.bUsesDynamicMeshElementData) )
					{
						DWORD Flags = bRenderStaticData ? 0 : FPrimitiveSceneProxy::DontAllowStaticMeshElementData;
						Flags |= bRenderDynamicData ? 0 : FPrimitiveSceneProxy::DontAllowDynamicMeshElementData;

						Drawer.SetPrimitive(PrimitiveSceneInfo);
						PrimitiveSceneInfo->Proxy->DrawDynamicElements(
							&Drawer,
							&View,
							DPGIndex,
							Flags					
							);
					}
				}
				bDirty |= Drawer.IsDirty(); 
			}
		}
  
		if( bRenderDynamicData )
		{
			// Draw the base pass for the view's batched mesh elements.
			bDirty |= DrawViewElements<FBasePassOpaqueDrawingPolicyFactory>(View,FBasePassOpaqueDrawingPolicyFactory::ContextType(),DPGIndex,TRUE);

			// Draw the view's batched simple elements(lines, sprites, etc).
			bDirty |= View.BatchedViewElements[DPGIndex].Draw(View.ViewProjectionMatrix,appTrunc(View.SizeX),appTrunc(View.SizeY),FALSE);
		}

#if XBOX
		// Render foreground occluders into the depth buffer so that the resolved scene depth texture will have both foreground and world DPG depths, 
		// which is necessary for shadows, PP effects, etc. 
		// This only needs to happen once per frame.
		if( bWorldDpg && (!bIsTiledRendering || GUseMSAASplitScreen) )
		{
			// Set the device viewport for the view
			ViewSetViewport(ViewIndex, TRUE, FALSE);
			RHISetViewParameters(&View, View.TranslatedViewProjectionMatrix, View.ViewOrigin);

			SCOPED_DRAW_EVENT(EventForegroundInWorldPrePass)(DEC_SCENE_ITEMS,TEXT("ForegroundOccluders"));
			bDirty |= RenderDPGPrePass(SDPG_Foreground, View);
		}
#endif

		if (GUseMSAASplitScreen && bWorldDpg)
		{
			// In non-tiled MSAA mode, resolve each view
			RHISetBlendState( TStaticBlendState<>::GetRHI());
			RHIMSAAEndRendering(GSceneRenderTargets.GetSceneDepthTexture(), GSceneRenderTargets.GetSceneColorTexture(), ViewIndex);
		}
	}

	// restore color write mask
	RHISetColorWriteMask(CW_RGBA);

	return bDirty;
}

/** 
* Renders the post process effects for a view. 
* @param DPGIndex - current depth priority group (DPG)
*/
UBOOL FSceneRenderer::RenderPostProcessEffects(UINT DPGIndex, UBOOL bAffectLightingOnly)
{
	SCOPED_DRAW_EVENT(EventPP)(DEC_SCENE_ITEMS,TEXT("PostProcessEffects%s"), bAffectLightingOnly ? TEXT(" LightingOnly") : TEXT(""));

	UBOOL bSetAllocations = FALSE;
	UBOOL bSceneColorDirty = FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

		// render any custom post process effects
		for( INT EffectIdx=0; EffectIdx < View.PostProcessSceneProxies.Num(); EffectIdx++ )
		{
			FPostProcessSceneProxy* PPEffectProxy = &View.PostProcessSceneProxies(EffectIdx);
			if( PPEffectProxy
				&& PPEffectProxy->GetDepthPriorityGroup() == DPGIndex 
				&& PPEffectProxy->GetAffectsLightingOnly() == bAffectLightingOnly)
			{
				if (!bSetAllocations)
				{
					// allocate more GPRs for pixel shaders
					RHISetShaderRegisterAllocation(32, 96);
					bSetAllocations = TRUE;
				}
				// render the effect
				bSceneColorDirty |= PPEffectProxy->Render( Scene,DPGIndex,View,CanvasTransform);
			}
		}
	}

	if (bSetAllocations)
	{
		// restore default GPR allocation
		RHISetShaderRegisterAllocation(64, 64);
	}
	return bSceneColorDirty;
}

/**
* Clears the scene color depth (stored in alpha channel) to max depth
* This is needed for depth bias blend materials to show up correctly
*/
void FSceneRenderer::ClearSceneColorDepth()
{
	//@todo: setup scene color depth for platforms that don't support floating point blending, and therefore color write mask.
	if (CanBlendWithFPRenderTarget(GRHIShaderPlatform))
	{
		SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("Clear Depth"));

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);

			ViewSetViewport(ViewIndex,FALSE,FALSE);

			const FLOAT MaxDistance = 1000000.0f;
			const FLOAT MaxDepth = View.InvDeviceZToWorldZTransform.Y + View.InvDeviceZToWorldZTransform.X / MaxDistance;
			const FLinearColor ClearDepthColor(0,0,0,MaxDepth);
		
			FBatchedElements BatchedElements;
			INT V00 = BatchedElements.AddVertex(FVector4(-1,-1,0,1),FVector2D(0,0),ClearDepthColor,FHitProxyId());
			INT V10 = BatchedElements.AddVertex(FVector4(1,-1,0,1),FVector2D(1,0),ClearDepthColor,FHitProxyId());
			INT V01 = BatchedElements.AddVertex(FVector4(-1,1,0,1),FVector2D(0,1),ClearDepthColor,FHitProxyId());
			INT V11 = BatchedElements.AddVertex(FVector4(1,1,0,1),FVector2D(1,1),ClearDepthColor,FHitProxyId());

			// No alpha blending, no depth tests or writes, no backface culling.
			RHISetBlendState(TStaticBlendState<>::GetRHI());
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
			RHISetColorWriteMask(CW_ALPHA);

			// Draw a quad using the generated vertices.
			BatchedElements.AddTriangle(V00,V10,V11,GWhiteTexture,BLEND_Opaque);
			BatchedElements.AddTriangle(V00,V11,V01,GWhiteTexture,BLEND_Opaque);
			BatchedElements.Draw(
				FMatrix::Identity,
				ViewFamily.RenderTarget->GetSizeX(),
				ViewFamily.RenderTarget->GetSizeY(),
				FALSE
				);

			RHISetColorWriteMask(CW_RGBA);
		}
	}
}


/** 
* Renders the scene to capture target textures 
*/
void FSceneRenderer::RenderSceneCaptures()
{
	SCOPED_DRAW_EVENT(Event)(DEC_SCENE_ITEMS,TEXT("Scene Captures"));

	// Disable texture fading.
	FLOAT PrevMipLevelFadingState = GEnableMipLevelFading;
	GEnableMipLevelFading = -1.0f;

	// disable tiling for rendering captures
	TGuardValue<UBOOL> GuardUseTilingCode(GUseTilingCode, FALSE);

	for( TSparseArray<FCaptureSceneInfo*>::TConstIterator CaptureIt(Scene->SceneCapturesRenderThread); CaptureIt; ++CaptureIt )
	{
		SCOPE_CYCLE_COUNTER(STAT_SceneCaptureRenderingTime);
		FCaptureSceneInfo* CaptureInfo = *CaptureIt;
        CaptureInfo->CaptureScene(this);
	}

	// Restore texture fading to its previous state.
	GEnableMipLevelFading = PrevMipLevelFadingState;
}

/** Updates the game-thread actor and primitive visibility states. */
void FSceneRenderer::SaveVisibilityState()
{
	// Update LastRenderTime for the primitives which were visible.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);

		// Replace the view's actor visibility history with the new visibility set.
		if(View.ActorVisibilityHistory)
		{
			check(View.ActorVisibilitySet);
			View.ActorVisibilityHistory->SetStates(View.ActorVisibilitySet);
		}
		else
		{
			check(!View.ActorVisibilitySet);
		}
	}
}

/** 
* Global state shared by all FSceneRender instances 
* @return global state
*/
FGlobalSceneRenderState* FSceneRenderer::GetGlobalSceneRenderState()
{
	static FGlobalSceneRenderState GlobalSceneRenderState;
	return &GlobalSceneRenderState;
}

/*-----------------------------------------------------------------------------
Computing Minimal Resolve Rectangles
-----------------------------------------------------------------------------*/

/** 
* template function to compute the viewspace bounds of a ViewBox along a given axis.
*
* @param PrimVerts - an array of vertices describing a convex hull in view space
* @param PrimBounds - [out] the view space bounds of the hull 
* @return TRUE is a valid set of bounds were found, FALSE if the convex hull is completely offscreen
*/
template <INT AXIS>
UBOOL CalculateAxisBounds(const TArray<FVector4>& PrimVerts, FBox& PrimBounds)
{
	UBOOL VisibleResultsFound = FALSE;
	const FLOAT VMin = -1.0f;
	const FLOAT VMax = +1.0f;
	const INT	MinOut = 2;
	const INT	MaxOut = 1;


	PrimBounds.IsValid = TRUE;
	PrimBounds.Min[AXIS] = VMax;
	PrimBounds.Max[AXIS] = VMin;

	INT Ocumulate = 0;
	INT Acumulate = ~0;
	INT AnythingVisible = 0;

	// Allocate an array of flags based on the primitive vertex count
	const INT NumPrimVerts = PrimVerts.Num();
	TArray<INT> OutFlags(NumPrimVerts);

	for (int i=0; i<NumPrimVerts; ++i)
	{
		const FVector4& Vertex = PrimVerts(i);
		INT& OutFlag = OutFlags(i);
		
		OutFlag = 0;

		FLOAT MinOffset = Vertex[AXIS] - (VMin * Vertex.W);
		FLOAT MaxOffset = Vertex[AXIS] - (VMax * Vertex.W);

		// set min/max out flags for this Axis
		if (MinOffset < 0)
		{
			// point is offscreen to the min side
			OutFlag |= MinOut;
		}
		if (MaxOffset > 0)
		{
			// point is offscreen to the max side
			OutFlag |= MaxOut;
		}

		// keep running OR and AND of flags
		Ocumulate |= OutFlag;
		Acumulate &= OutFlag;

		// update our bounds prediction if fully visible
		if (OutFlag == 0)
		{
			AnythingVisible = 1;

			FLOAT PrimBoundsMin = Vertex[AXIS] - (PrimBounds.Min[AXIS] * Vertex.W);
			FLOAT PrimBoundsMax = Vertex[AXIS] - (PrimBounds.Max[AXIS] * Vertex.W);

			if (PrimBoundsMin < 0)
			{
				PrimBounds.Min[AXIS] = Vertex[AXIS] / Vertex.W;
			}
			if (PrimBoundsMax > 0)
			{
				PrimBounds.Max[AXIS] = Vertex[AXIS] / Vertex.W;
			}
		}
	}

	if (Ocumulate == 0)
	{
		// Everything was fully onscreen, bounds are good as-is
		VisibleResultsFound = TRUE;
	}
	else if (Acumulate != 0)
	{
		// Everything was fully offscreen
		VisibleResultsFound = FALSE;
	}
	else if (AnythingVisible == 0)
	{
		// Everything was offscreen, but still may span across the screen.
		// We have to use the full range to be safe
		PrimBounds.Min[AXIS] = VMin;
		PrimBounds.Max[AXIS] = VMax;
		VisibleResultsFound = TRUE;
	}
	else
	{
		// we have a mix of onscreen and offscreen points. We'll need a second pass to determine the true bounds
		VisibleResultsFound = TRUE;

		for (int i=0; i<NumPrimVerts; ++i)
		{
			const FVector4& Vertex = PrimVerts(i);
			INT& OutFlag = OutFlags(i);

			if ((OutFlag & MinOut) &&
				(Vertex[AXIS] - (PrimBounds.Min[AXIS] * Vertex.W) < 0))
			{
				PrimBounds.Min[AXIS] = VMin;
			}

			if ((OutFlag & MaxOut) &&
				(Vertex[AXIS] - (PrimBounds.Max[AXIS] * Vertex.W) > 0))
			{
				PrimBounds.Max[AXIS] = VMax;
			}
		}
	}

	return VisibleResultsFound;
}

/** 
* Helper function to compute the viewspace bounds of a ViewBox in two dimentions 
* @param PrimVerts - an array of vertices describing a convex hull in view space
* @param PrimBounds - [out] the view space bounds of the hull 
* @return TRUE is a valid set of bounds were found, FALSE if the convex hull is completely offscreen
*/
static UBOOL CalculateViewBounds(const TArray<FVector4>& PrimVerts, FBox& PrimBounds)
{
	// we are onscreen if both axis report an onscreen result
	return CalculateAxisBounds<0>(PrimVerts, PrimBounds) && CalculateAxisBounds<1>(PrimVerts, PrimBounds);
}

/** 
* Helper interface to fetch distortion primitive data from a view 
*/
class DistortionPrimitiveFetchInterface
{
public:
	static INT FetchCount(const FViewInfo& View, INT DPGIndex) 
	{ 
		return View.DistortionPrimSet[DPGIndex].NumPrims(); 
	}
	static const FPrimitiveSceneInfo* FetchPrimitive(const FViewInfo& View, INT DPGIndex, INT PrimIndex) 
	{ 
		return View.DistortionPrimSet[DPGIndex].GetPrim(PrimIndex); 
	}
};

/** 
* Helper interface to fetch translucent primitive data from a view 
*/
class TranslucentPrimitiveFetchInterface
{
public:
	static INT FetchCount(const FViewInfo& View, INT DPGIndex) 
	{ 
		return View.TranslucentPrimSet[DPGIndex].NumSceneColorPrims(); 
	}
	static const FPrimitiveSceneInfo* FetchPrimitive(const FViewInfo& View, INT DPGIndex, INT PrimIndex) 
	{ 
		return View.TranslucentPrimSet[DPGIndex].GetSceneColorPrim(PrimIndex); 
	}
};

/** 
* template function to determine the screen pixel extents of a set of primitives, using one of the fetch interfaces above 
*
* @param Views - set of views to find screen extents for as the rect can span multiple views
* @param DPGIndex - current depth group pass being rendered
* @param PixelRect - [out] screen space min,max bounds for primitives
* @return TRUE if any of the primitives had visible bounds on screen
*/
template <typename FETCH_INTERFACE>
UBOOL ComputePixelBoundsOfPrimitives(const TArray<FViewInfo>& Views, INT DPGIndex, FIntRect& PixelRect)
{
	FBox ScreenBounds(0);
	FIntRect RenderTargetBounds(0,0,0,0);
	UBOOL VisibleBoundsFound= FALSE;

	PixelRect.Min.X= 0;
	PixelRect.Min.Y= 0;
	PixelRect.Max.X= 0;
	PixelRect.Max.Y= 0;
	
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		const INT NumPrims= FETCH_INTERFACE::FetchCount(View, DPGIndex);
		
		// update our global render target bounds for all views
		if (ViewIndex == 0)
		{
			RenderTargetBounds.Min.X = View.RenderTargetX;
			RenderTargetBounds.Min.Y = View.RenderTargetY;
			RenderTargetBounds.Max.X = View.RenderTargetX + View.RenderTargetSizeX;
			RenderTargetBounds.Max.Y = View.RenderTargetY + View.RenderTargetSizeY;
		}
		else
		{
			RenderTargetBounds.Min.X = Min(RenderTargetBounds.Min.X, View.RenderTargetX);
			RenderTargetBounds.Min.Y = Min(RenderTargetBounds.Min.Y, View.RenderTargetY);
			RenderTargetBounds.Max.X = Max(RenderTargetBounds.Max.X, View.RenderTargetX + View.RenderTargetSizeX);
			RenderTargetBounds.Max.Y = Max(RenderTargetBounds.Max.Y, View.RenderTargetY + View.RenderTargetSizeY);
		}

		FBox ViewBounds(0);

		// build an array to hold all the bounding-box vertices in this view
		FVector WorldVerts[8];
		TArray<FVector4> ViewSpaceVerts(8);

		for (INT PrimIndex=0; PrimIndex<NumPrims; ++PrimIndex)
		{
			// get the primitive
			const FPrimitiveSceneInfo* Prim= FETCH_INTERFACE::FetchPrimitive(View, DPGIndex, PrimIndex);
			const FBox PrimBox= Prim->Bounds.GetBox();

			// extract the eight vertices of the bounding box
			WorldVerts[0] = FVector(PrimBox.Min);
			WorldVerts[1] = FVector(PrimBox.Min.X, PrimBox.Min.Y, PrimBox.Max.Z);
			WorldVerts[2] = FVector(PrimBox.Min.X, PrimBox.Max.Y, PrimBox.Min.Z);
			WorldVerts[3] = FVector(PrimBox.Max.X, PrimBox.Min.Y, PrimBox.Min.Z);
			WorldVerts[4] = FVector(PrimBox.Max.X, PrimBox.Max.Y, PrimBox.Min.Z);
			WorldVerts[5] = FVector(PrimBox.Max.X, PrimBox.Min.Y, PrimBox.Max.Z);
			WorldVerts[6] = FVector(PrimBox.Min.X, PrimBox.Max.Y, PrimBox.Max.Z);
			WorldVerts[7] = FVector(PrimBox.Max);


			// transform each to homogenous view space to setup our ViewVert elements
			ViewSpaceVerts(0) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[0]);
			ViewSpaceVerts(1) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[1]);
			ViewSpaceVerts(2) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[2]);
			ViewSpaceVerts(3) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[3]);
			ViewSpaceVerts(4) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[4]);
			ViewSpaceVerts(5) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[5]);
			ViewSpaceVerts(6) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[6]);
			ViewSpaceVerts(7) = View.ViewProjectionMatrix.TransformFVector(WorldVerts[7]);

			// determine the screen space bounds of this box
			FBox PrimBounds(0);
			if (CalculateViewBounds(ViewSpaceVerts, PrimBounds))
			{
				ViewBounds += PrimBounds;
			}
		}

		if (ViewBounds.IsValid)
		{
			// convert the view bounds to screen space
			FIntRect ViewPixels;

			// mirror the view bounds on the Y axis
			FLOAT OldMinY= ViewBounds.Min.Y;
			ViewBounds.Min.Y= -ViewBounds.Max.Y;
			ViewBounds.Max.Y= -OldMinY;

			// convert each axis from (-1,+1) to (0,1) range
			const FVector HalfScale(0.5f, 0.5f, 0.0f);
			ViewBounds.Min = (ViewBounds.Min * HalfScale) + HalfScale;
			ViewBounds.Max = (ViewBounds.Max * HalfScale) + HalfScale;
			ViewBounds.Min.X= Clamp(ViewBounds.Min.X, 0.0f, 1.0f);
			ViewBounds.Min.Y= Clamp(ViewBounds.Min.Y, 0.0f, 1.0f);
			ViewBounds.Max.X= Clamp(ViewBounds.Max.X, 0.0f, 1.0f);
			ViewBounds.Max.Y= Clamp(ViewBounds.Max.Y, 0.0f, 1.0f);

			// scale and offset to the pixel dimensions of this view
			ViewBounds.Min.X *= (FLOAT)View.RenderTargetSizeX;
			ViewBounds.Min.Y *= (FLOAT)View.RenderTargetSizeY;
			ViewBounds.Max.X *= (FLOAT)View.RenderTargetSizeX;
			ViewBounds.Max.Y *= (FLOAT)View.RenderTargetSizeY;

			ViewBounds.Min.X += (FLOAT)View.RenderTargetX;
			ViewBounds.Min.Y += (FLOAT)View.RenderTargetY;
			ViewBounds.Max.X += (FLOAT)View.RenderTargetX;
			ViewBounds.Max.Y += (FLOAT)View.RenderTargetY;

			// add to the render target bounds
			ScreenBounds += ViewBounds;
		}
	}

	if (ScreenBounds.IsValid &&
		RenderTargetBounds.Area() > 0)
	{
		// scale the final screen bounds to pixel dimensions
		PixelRect.Min.X = appTrunc(ScreenBounds.Min.X) - 1;
		PixelRect.Min.Y = appTrunc(ScreenBounds.Min.Y) - 1;
		PixelRect.Max.X = appTrunc(ScreenBounds.Max.X) + 1;
		PixelRect.Max.Y = appTrunc(ScreenBounds.Max.Y) + 1;

		// snap to 32-pixel alignment
		PixelRect.Min.X = (PixelRect.Min.X   ) & ~31; //round down
		PixelRect.Max.X = (PixelRect.Max.X+31) & ~31; //round up	
		PixelRect.Min.Y = (PixelRect.Min.Y   ) & ~31;	
		PixelRect.Max.Y = (PixelRect.Max.Y+31) & ~31;

		// clamp to the total render target bounds we found
		PixelRect.Min.X = Clamp(PixelRect.Min.X, RenderTargetBounds.Min.X, RenderTargetBounds.Max.X);
		PixelRect.Max.X = Clamp(PixelRect.Max.X, RenderTargetBounds.Min.X, RenderTargetBounds.Max.X);
		PixelRect.Min.Y = Clamp(PixelRect.Min.Y, RenderTargetBounds.Min.Y, RenderTargetBounds.Max.Y);
		PixelRect.Max.Y = Clamp(PixelRect.Max.Y, RenderTargetBounds.Min.Y, RenderTargetBounds.Max.Y);

		// make sure it's legit
		INT XSize= PixelRect.Max.X - PixelRect.Min.X;
		INT YSize= PixelRect.Max.Y - PixelRect.Min.Y;
		if (XSize > 0 && YSize > 0)
		{
			// valid onscreen bounds were found
			VisibleBoundsFound = TRUE;
		}
	}

	return VisibleBoundsFound;
}

/** 
* Helper used to compute the minimual screen bounds of all translucent primitives which require scene color 
*
* @param DPGIndex - current depth group pass being rendered
* @param PixelRect - [out] screen space min,max bounds for primitives
* @return TRUE if any of the primitives had visible bounds on screen
*/
UBOOL FSceneRenderer::ComputeTranslucencyResolveRectangle(INT DPGIndex, FIntRect& PixelRect)
{
	return ComputePixelBoundsOfPrimitives<TranslucentPrimitiveFetchInterface>(Views, DPGIndex, PixelRect);
}

/** 
* Helper used to compute the minimual screen bounds of all distortion primitives which require scene color 
* 
* @param DPGIndex - current depth group pass being rendered
* @param PixelRect - [out] screen space min,max bounds for primitives
* @return TRUE if any of the primitives had visible bounds on screen
*/
UBOOL FSceneRenderer::ComputeDistortionResolveRectangle(INT DPGIndex, FIntRect& PixelRect)
{
	return ComputePixelBoundsOfPrimitives<DistortionPrimitiveFetchInterface>(Views, DPGIndex, PixelRect);
}

/*-----------------------------------------------------------------------------
BeginRenderingViewFamily
-----------------------------------------------------------------------------*/

/**
 * Helper function performing actual work in render thread.
 *
 * @param SceneRenderer	Scene renderer to use for rendering.
 */
static void RenderViewFamily_RenderThread( FSceneRenderer* SceneRenderer )
{
	FMemMark MemStackMark(GRenderingThreadMemStack);

	{
		SCOPE_CYCLE_COUNTER(STAT_TotalSceneRenderingTime);

		// keep track of global frame number
		SceneRenderer->GetGlobalSceneRenderState()->FrameNumber++;

		if(SceneRenderer->ViewFamily.ShowFlags & SHOW_HitProxies)
		{
			// Render the scene's hit proxies.
			SceneRenderer->RenderHitProxies();
		}
		else
		{
			if(SceneRenderer->ViewFamily.ShowFlags & SHOW_SceneCaptureUpdates)
			{
				// Render the scene for each capture
				SceneRenderer->RenderSceneCaptures();
			}

			// Render the scene.
			SceneRenderer->Render();
		}

		// Delete the scene renderer.
		delete SceneRenderer;
	}

#if WANTS_DRAW_MESH_EVENTS && PLATFORM_SUPPORTS_DRAW_MESH_EVENTS
	INC_DWORD_STAT_BY(STAT_DrawEvents,FDrawEvent::Counter);
	STAT(FDrawEvent::Counter=0);
#endif

#if STATS && CONSOLE
	/** Update STATS with the total GPU time taken to render the last frame. */
	SET_CYCLE_COUNTER(STAT_TotalGPUFrameTime, RHIGetGPUFrameCycles(), 1);
#endif
}

void BeginRenderingViewFamily(FCanvas* Canvas,const FSceneViewFamily* ViewFamily)
{
	// Enforce the editor only show flags restrictions.
	check(GIsEditor || !(ViewFamily->ShowFlags & SHOW_EditorOnly_Mask));

	// Flush the canvas first.
	Canvas->Flush();

	if( ViewFamily->Scene )
	{
		// Set the world's "needs full lighting rebuild" flag if the scene has any uncached static lighting interactions.
		FScene* const Scene = (FScene*)ViewFamily->Scene->GetRenderScene();
		UWorld* const World = Scene->GetWorld();
		if(World && Scene->NumUncachedStaticLightingInteractions)
		{
			World->GetWorldInfo()->SetMapNeedsLightingFullyRebuilt(TRUE);
		}

#if GEMINI_TODO
		// We need to pass the scene's hit proxies through to the hit proxy consumer!
		// Otherwise its possible the hit proxy consumer will cache a hit proxy ID it doesn't have a reference for.
		// Note that the effects of not doing this correctly are minor: clicking on a primitive that moved without invalidating the viewport's
		// cached hit proxies won't work.  Is this worth the pain?
#endif

		// Construct the scene renderer.  This copies the view family attributes into its own structures.
		FSceneRenderer* SceneRenderer = ::new FSceneRenderer(ViewFamily,Canvas->GetHitProxyConsumer(),Canvas->GetFullTransform());

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FDrawSceneCommand,
			FSceneRenderer*,SceneRenderer,SceneRenderer,
		{
			RenderViewFamily_RenderThread(SceneRenderer);
		});
	}
	else
	{
		// Construct the scene renderer.  This copies the view family attributes into its own structures.
		FSceneRenderer* SceneRenderer = ::new FSceneRenderer(ViewFamily,Canvas->GetHitProxyConsumer(),Canvas->GetFullTransform());

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FDrawSceneCommandPP,
			FSceneRenderer*,SceneRenderer,SceneRenderer,
		{
			SceneRenderer->RenderPostProcessOnly();
			delete SceneRenderer;
		});
	}

	// We need to flush rendering commands if stats gathering is enabled to ensure that the stats are valid/ captured
	// before this function returns.
	if( ViewFamily->DynamicShadowStats )
	{
		FlushRenderingCommands();
	}
}

//@todo debug -
UBOOL FActorVisibilitySet::DebugVerifyHash(const AActor* VisibleActor)
{
	// verify that the elements in the set hash are valid
	if( !VisibleActors.VerifyHashElementsKey(VisibleActor) )
	{
		// if failed then dumps info about the hash and 
		warnf(TEXT("FActorVisibilitySet::DebugVerifyHash failed on actor = %s"),
			*VisibleActor->GetPathName());
		
		VisibleActors.DumpHashElements(*GWarn);
		GWarn->Flush();

		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*-----------------------------------------------------------------------------
	Stat declarations.
-----------------------------------------------------------------------------*/

DECLARE_STATS_GROUP(TEXT("SceneRendering"),STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("BeginOcclusionTests time"),STAT_BeginOcclusionTestsTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Occlusion Result time"),STAT_OcclusionResultTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("InitViews time"),STAT_InitViewsTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Dynamic shadow setup time"),STAT_DynamicShadowSetupTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Translucency setup time"),STAT_TranslucencySetupTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Scene capture rendering time"),STAT_SceneCaptureRenderingTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Total CPU rendering time"),STAT_TotalSceneRenderingTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Total GPU rendering time"),STAT_TotalGPUFrameTime,STATGROUP_SceneRendering);

DECLARE_CYCLE_STAT(TEXT("Depth drawing time"),STAT_DepthDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Base pass drawing time"),STAT_BasePassDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Shadow volume drawing time"),STAT_ShadowVolumeDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Light function drawing time"),STAT_LightFunctionDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Lighting drawing time"),STAT_LightingDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Proj Shadow drawing time"),STAT_ProjectedShadowDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Mod Shadow drawing time"),STAT_ModulatedShadowDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Translucency drawing time"),STAT_TranslucencyDrawTime,STATGROUP_SceneRendering);
DECLARE_CYCLE_STAT(TEXT("Velocity drawing time"),STAT_VelocityDrawTime,STATGROUP_SceneRendering);

DECLARE_DWORD_COUNTER_STAT(TEXT("Culled primitives"),STAT_CulledPrimitives,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Occluded primitives"),STAT_OccludedPrimitives,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Occlusion queries"),STAT_OcclusionQueries,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Projected shadows"),STAT_ProjectedShadows,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible static mesh elements"),STAT_VisibleStaticMeshElements,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Visible dynamic primitives"),STAT_VisibleDynamicPrimitives,STATGROUP_SceneRendering);
DECLARE_DWORD_COUNTER_STAT(TEXT("Draw events"),STAT_DrawEvents,STATGROUP_SceneRendering);

DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Lights"),STAT_SceneLights,STATGROUP_SceneRendering);

