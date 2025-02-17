/*=============================================================================
	TranslucentRendering.cpp: Translucent rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

/** Controls whether translucents resolve to the raw format or not */
UBOOL GRenderMinimalTranslucency= TRUE;

/**
* Pixel shader used to extract LDR scene color from an HDR surface
*/
class FLDRExtractPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLDRExtractPixelShader,Global);

	/** 
	* Only cache these shaders for SM2
	*
	* @param Platform - current platform being compiled
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return Platform == SP_PCD3D_SM2;
	}

	/** Default constructor. */
	FLDRExtractPixelShader() {}

public:

	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderResourceParameter SceneColorScratchTextureParameter;

	/** Initialization constructor. */
	FLDRExtractPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		SceneColorScratchTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorScratchTexture"),TRUE);
	}

	// FShader interface.
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << SceneColorScratchTextureParameter;
		return bShaderHasOutdatedParameters;
	}
};

IMPLEMENT_SHADER_TYPE(,FLDRExtractPixelShader,TEXT("LDRExtractPixelShader"),TEXT("Main"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(,FLDRExtractVertexShader,TEXT("LDRExtractVertexShader"),TEXT("Main"),SF_Vertex,0,0);

FGlobalBoundShaderState FSceneRenderer::LDRExtractBoundShaderState;

extern TGlobalResource<FFilterVertexDeclaration> GFilterVertexDeclaration;

/** The parameters used to draw a translucent mesh. */
class FDrawTranslucentMeshAction
{
public:

	const FViewInfo& View;
	UBOOL bBackFace;
	FHitProxyId HitProxyId;

	/** Initialization constructor. */
	FDrawTranslucentMeshAction(
		const FViewInfo& InView,
		const UBOOL bInBackFace,
		const FHitProxyId InHitProxyId
		):
		View(InView),
		bBackFace(bInBackFace),
		HitProxyId(InHitProxyId)
	{}

	ESceneDepthPriorityGroup GetDPG(const FProcessBasePassMeshParameters& Parameters) const
	{
		return (ESceneDepthPriorityGroup)Parameters.PrimitiveSceneInfo->Proxy->GetDepthPriorityGroup(&View);
	}

	/** Draws the translucent mesh with a specific light-map type, and fog volume type */
	template<typename LightMapPolicyType,typename FogDensityPolicyType>
	void Process(
		const FProcessBasePassMeshParameters& Parameters,
		const LightMapPolicyType& LightMapPolicy,
		const typename LightMapPolicyType::ElementDataType& LightMapElementData,
		const typename FogDensityPolicyType::ElementDataType& FogDensityElementData
		) const
	{
		const UBOOL bIsLitMaterial = Parameters.LightingModel != MLM_Unlit;

		TBasePassDrawingPolicy<LightMapPolicyType,FogDensityPolicyType> DrawingPolicy(
			Parameters.Mesh.VertexFactory,
			Parameters.Mesh.MaterialRenderProxy,
			LightMapPolicy,
			Parameters.BlendMode,
			Parameters.PrimitiveSceneInfo && Parameters.PrimitiveSceneInfo->HasDynamicSkyLighting() && bIsLitMaterial,
			View.Family->ShowFlags & SHOW_ShaderComplexity
			);
		DrawingPolicy.DrawShared(
			&View,
			DrawingPolicy.CreateBoundShaderState(Parameters.Mesh.GetDynamicVertexStride())
			);
		DrawingPolicy.SetMeshRenderState(
			View,
			Parameters.PrimitiveSceneInfo,
			Parameters.Mesh,
			bBackFace,
			typename TBasePassDrawingPolicy<LightMapPolicyType,FogDensityPolicyType>::ElementDataType(
				LightMapElementData,
				FogDensityElementData
				)
			);
		DrawingPolicy.DrawMesh(Parameters.Mesh);
	}
};

/**
 * Render a dynamic mesh using a translucent draw policy
 * @return TRUE if the mesh rendered
 */
UBOOL FTranslucencyDrawingPolicyFactory::DrawDynamicMesh(
	const FViewInfo& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty = FALSE;

	// Determine the mesh's material and blend mode.
	const FMaterial* Material = Mesh.MaterialRenderProxy->GetMaterial();
	const EBlendMode BlendMode = Material->GetBlendMode();

	// Only render translucent materials.
	if(IsTranslucentBlendMode(BlendMode))
	{
		// Handle the fog volume case.
		if (Material->IsUsedWithFogVolumes())
		{
			bDirty = RenderFogVolume( &View, Mesh, bBackFace, bPreFog, PrimitiveSceneInfo, HitProxyId );
		}
		else
		{
			UBOOL bOverrideBlendWithOpaque = FALSE;

			{
				const UBOOL bDisableDepthTesting = Material->NeedsDepthTestDisabled();
				// Allow the material to turn off depth testing if desired
				if (bDisableDepthTesting)
				{
					// No depth tests or depth writes
					RHISetDepthState( TStaticDepthState<FALSE,CF_Always>::GetRHI() );
				}
				const UBOOL bOneLayerDistortion = Material->UsesOneLayerDistortion();
				// Disable alpha writes with one layer distortion as the material will be using an opaque blend mode
				if (bOneLayerDistortion)
				{
					RHISetColorWriteMask(CW_RGB);
				}

				ProcessBasePassMesh(
					FProcessBasePassMeshParameters(
						Mesh,
						Material,
						PrimitiveSceneInfo, 
						!bPreFog
						),
					FDrawTranslucentMeshAction(
						View,
						bBackFace,
						HitProxyId
						)
					);
				bDirty = TRUE;

				if (bDisableDepthTesting)
				{
					// Restore translucency default depth state (depth tests, no depth writes)
					RHISetDepthState( TStaticDepthState<FALSE,CF_LessEqual>::GetRHI() );
				}
				if (bOneLayerDistortion)
				{
					RHISetColorWriteMask(CW_RGBA);
				}
			}
		}
	}
	return bDirty;
}

/**
 * Render a static mesh using a translucent draw policy
 * @return TRUE if the mesh rendered
 */
UBOOL FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
	const FViewInfo* View,
	ContextType DrawingContext,
	const FStaticMesh& StaticMesh,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty = FALSE;

	const FMaterial* Material = StaticMesh.MaterialRenderProxy->GetMaterial();
	const EMaterialLightingModel LightingModel = Material->GetLightingModel();
	const UBOOL bNeedsBackfacePass =
		Material->IsTwoSided() &&
		(LightingModel != MLM_NonDirectional) &&
		(LightingModel != MLM_Unlit) &&
		!CanAccessFacingRegister(GRHIShaderPlatform);
	const UINT NumBackfacePasses = bNeedsBackfacePass ? 2 : 1;
	for(UINT bBackFace = 0;bBackFace < NumBackfacePasses;bBackFace++)
	{
		bDirty |= DrawDynamicMesh(
			*View,
			DrawingContext,
			StaticMesh,
			bBackFace,
			bPreFog,
			PrimitiveSceneInfo,
			HitProxyId
			);
	}

	return bDirty;
}

/*-----------------------------------------------------------------------------
FTranslucentPrimSet
-----------------------------------------------------------------------------*/

/**
* Iterate over the sorted list of prims and draw them
* @param ViewInfo - current view used to draw items
* @param DPGIndex - current DPG used to draw items
* @param bSceneColorPass - TRUE if only the translucent prims that read from scene color should be drawn
* @return TRUE if anything was drawn
*/
UBOOL FTranslucentPrimSet::Draw(
	const FViewInfo* View,
	UINT DPGIndex,
	UBOOL bSceneColorPass
	)
{
	UBOOL bDirty=FALSE;

	const TArray<FSortedPrim,SceneRenderingAllocator>& PhaseSortedPrimitives =
		bSceneColorPass ?
			SortedSceneColorPrims :
			SortedPrims;

	if( PhaseSortedPrimitives.Num() )
	{
		// For drawing scene prims with dynamic relevance.
		TDynamicPrimitiveDrawer<FTranslucencyDrawingPolicyFactory> Drawer(
			View,
			DPGIndex,
			FTranslucencyDrawingPolicyFactory::ContextType(),
			FALSE
			);

		UBOOL bResolvedForOneLayerDistortion = FALSE;

		// Draw sorted scene prims
		for( INT PrimIdx=0; PrimIdx < PhaseSortedPrimitives.Num(); PrimIdx++ )
		{
			FPrimitiveSceneInfo* PrimitiveSceneInfo = PhaseSortedPrimitives(PrimIdx).PrimitiveSceneInfo;
			const FPrimitiveViewRelevance& ViewRelevance = View->PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			if (ViewRelevance.bOneLayerDistortionRelevance && !bResolvedForOneLayerDistortion)
			{
				// Resolve scene color for the first one layer distortion primitive encountered
				GSceneRenderTargets.SaveSceneColorRaw(TRUE);
				GSceneRenderTargets.BeginRenderingSceneColor();
				// Restore states changed by the resolve
				RHISetViewport(View->RenderTargetX,View->RenderTargetY,0.0f,View->RenderTargetX + View->RenderTargetSizeX,View->RenderTargetY + View->RenderTargetSizeY,1.0f);
				// Enable depth test, disable depth writes.
				RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
				bResolvedForOneLayerDistortion = TRUE;
			}

			// Render dynamic scene prim
			if( ViewRelevance.bDynamicRelevance )
			{
				Drawer.SetPrimitive(PrimitiveSceneInfo);
				PrimitiveSceneInfo->Proxy->DrawDynamicElements(
					&Drawer,
					View,
					DPGIndex
					);
			}
			// Render static scene prim
			if( ViewRelevance.bStaticRelevance )
			{
				// Render static meshes from static scene prim
				for( INT StaticMeshIdx=0; StaticMeshIdx < PrimitiveSceneInfo->StaticMeshes.Num(); StaticMeshIdx++ )
				{
					FStaticMesh& StaticMesh = PrimitiveSceneInfo->StaticMeshes(StaticMeshIdx);
					if (View->StaticMeshVisibilityMap(StaticMesh.Id)
						// Only render static mesh elements using translucent materials
						&& StaticMesh.IsTranslucent() )
					{
						bDirty |= FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
							View,
							FTranslucencyDrawingPolicyFactory::ContextType(),
							StaticMesh,
							FALSE,
							PrimitiveSceneInfo,
							StaticMesh.HitProxyId
							);
					}
				}
			}

			if( ViewRelevance.IsDecalRelevant() )
			{
				SCOPE_CYCLE_COUNTER(STAT_DecalRenderUnlitTime);

				// render dynamic translucent decals on translucent receivers
				if( ViewRelevance.bDecalDynamicRelevance )
				{
					Drawer.SetPrimitive(PrimitiveSceneInfo);
					PrimitiveSceneInfo->Proxy->DrawDynamicDecalElements(
						&Drawer,
						View,
						DPGIndex,
						FALSE,
						TRUE
						);
				}
				// render static translucent decals on translucent receivers
				if( ViewRelevance.bDecalStaticRelevance )
				{
					// Render static meshes from static scene prim
					for( INT DecalIdx=0; DecalIdx < PrimitiveSceneInfo->Proxy->Decals.Num(); DecalIdx++ )
					{
						FDecalInteraction* Decal = PrimitiveSceneInfo->Proxy->Decals(DecalIdx);
						if( Decal && 
							Decal->DecalStaticMesh &&
							View->DecalStaticMeshVisibilityMap(Decal->DecalStaticMesh->Id) &&
							Decal->DecalStaticMesh->IsTranslucent() )
						{
							bDirty |= FTranslucencyDrawingPolicyFactory::DrawStaticMesh(
								View,
								FTranslucencyDrawingPolicyFactory::ContextType(),
								*Decal->DecalStaticMesh,
								FALSE,
								PrimitiveSceneInfo,
								Decal->DecalStaticMesh->HitProxyId
								);								
						}
					}
				}
			}
		}
		// Mark dirty if dynamic drawer rendered
		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}

/**
* Add a new primitive to the list of sorted prims
* @param PrimitiveSceneInfo - primitive info to add. Origin of bounds is used for sort.
* @param ViewInfo - used to transform bounds to view space
* @param bUsesSceneColor - primitive samples from scene color
*/
void FTranslucentPrimSet::AddScenePrimitive(FPrimitiveSceneInfo* PrimitiveSceneInfo,const FViewInfo& ViewInfo, UBOOL bUsesSceneColor)
{
	SCOPE_CYCLE_COUNTER(STAT_TranslucencySetupTime);

	FLOAT SortKey=0.f;
	FFogVolumeDensitySceneInfo** FogDensityInfoRef = PrimitiveSceneInfo->Scene->FogVolumes.Find(PrimitiveSceneInfo->Component);
	UBOOL bIsFogVolume = FogDensityInfoRef != NULL;
	if (bIsFogVolume)
	{
		const FFogVolumeDensitySceneInfo* FogDensityInfo = *FogDensityInfoRef;
		check(FogDensityInfo);
		if (FogDensityInfo->bAffectsTranslucency)
		{
			//sort by view space depth + primitive radius, so that intersecting translucent objects are drawn first,
			//which is needed for fogging the translucent object.
			SortKey = ViewInfo.ViewMatrix.TransformFVector(PrimitiveSceneInfo->Bounds.Origin).Z + 0.7f * PrimitiveSceneInfo->Bounds.SphereRadius;
		}
		else
		{
			//sort based on view space depth
			SortKey = ViewInfo.ViewMatrix.TransformFVector(PrimitiveSceneInfo->Bounds.Origin).Z;
		}
	}
	else
	{
		//sort based on view space depth
		SortKey = ViewInfo.ViewMatrix.TransformFVector(PrimitiveSceneInfo->Bounds.Origin).Z;

		PrimitiveSceneInfo->FogVolumeSceneInfo = NULL;
		INT DPGIndex = PrimitiveSceneInfo->Proxy->GetDepthPriorityGroup(&ViewInfo);
		FLOAT LargestFogVolumeRadius = 0.0f;
		//find the largest fog volume this translucent object is intersecting with
		for( TMap<const UPrimitiveComponent*, FFogVolumeDensitySceneInfo*>::TIterator FogVolumeIt(PrimitiveSceneInfo->Scene->FogVolumes); FogVolumeIt; ++FogVolumeIt )
		{
			const UPrimitiveComponent* FogVolumePrimComponent = FogVolumeIt.Key();
			FFogVolumeDensitySceneInfo* FogVolumeDensityInfo = FogVolumeIt.Value();
			if (FogVolumePrimComponent 
				&& FogVolumeDensityInfo 
				&& FogVolumeDensityInfo->bAffectsTranslucency
				&& FogVolumeDensityInfo->DPGIndex == DPGIndex)
			{
				const FLOAT FogVolumeRadius = FogVolumePrimComponent->Bounds.SphereRadius;
				const FLOAT TranslucentObjectRadius = PrimitiveSceneInfo->Bounds.SphereRadius;
				if (FogVolumeRadius > LargestFogVolumeRadius)
				{
					const FLOAT DistSquared = (FogVolumePrimComponent->Bounds.Origin - PrimitiveSceneInfo->Bounds.Origin).SizeSquared();
					if (DistSquared < FogVolumeRadius * FogVolumeRadius + TranslucentObjectRadius * TranslucentObjectRadius)
					{
						LargestFogVolumeRadius = FogVolumeRadius;
						PrimitiveSceneInfo->FogVolumeSceneInfo = FogVolumeDensityInfo;
					}
				}
			}
		}
	}

	if( bUsesSceneColor )
	{
		// add to list of translucent prims that use scene color
		new(SortedSceneColorPrims) FSortedPrim(PrimitiveSceneInfo,SortKey,PrimitiveSceneInfo->TranslucencySortPriority);
	}
	else
	{
		// add to list of translucent prims
		new(SortedPrims) FSortedPrim(PrimitiveSceneInfo,SortKey,PrimitiveSceneInfo->TranslucencySortPriority);
	}
}

/**
* Sort any primitives that were added to the set back-to-front
*/
void FTranslucentPrimSet::SortPrimitives()
{
	SCOPE_CYCLE_COUNTER(STAT_TranslucencySetupTime);
	// sort prims based on depth
	Sort<USE_COMPARE_CONSTREF(FSortedPrim,TranslucentRender)>(&SortedPrims(0),SortedPrims.Num());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// FSceneRenderer::RenderTranslucency
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Renders the scene's translucency.
 *
 * @param	DPGIndex	Current DPG used to draw items.
 * @return				TRUE if anything was drawn.
 */
UBOOL FSceneRenderer::RenderTranslucency(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventTranslucent)(DEC_SCENE_ITEMS,TEXT("Translucency"));

#if PS3 && !USE_NULL_RHI
	extern UBOOL GEnableDrawCounter;
	extern UBOOL GTriggerDrawCounter;
	if ( GEnableDrawCounter )
	{
		GTriggerDrawCounter = TRUE;
	}
#endif

	UBOOL bRender=FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		const UBOOL bViewHasTranslucentViewMeshElements = View.bHasTranslucentViewMeshElements & (1<<DPGIndex);
		if( View.TranslucentPrimSet[DPGIndex].NumPrims() > 0 || bViewHasTranslucentViewMeshElements )
		{
			bRender=TRUE;
			break;
		}
	}

	UBOOL bDirty = FALSE;
	if( bRender )
	{
		// Use the scene color buffer.
		GSceneRenderTargets.BeginRenderingSceneColor();

		// Enable depth test, disable depth writes.
		RHISetDepthState( TStaticDepthState<FALSE,CF_LessEqual>::GetRHI() );

		//reset the fog apply stencil index for this frame
		ResetFogVolumeIndex();

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

			FViewInfo& View = Views(ViewIndex);
			// viewport to match view size
			RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

			// Draw the view's mesh elements with the translucent drawing policy.
			bDirty |= DrawViewElements<FTranslucencyDrawingPolicyFactory>(View,FTranslucencyDrawingPolicyFactory::ContextType(),DPGIndex,FALSE);

			// Draw only translucent prims that don't read from scene color
			bDirty |= View.TranslucentPrimSet[DPGIndex].Draw(&View,DPGIndex,FALSE);

			if (View.TranslucentPrimSet[DPGIndex].NumSceneColorPrims() > 0)
			{
				// begin by assuming we will resolve the entire view family
				FResolveParams ResolveParams = FResolveParams(0, 0, FamilySizeX, FamilySizeY);

#if XBOX
				// when enabled, determine the minimal screen space area to work in
				if (GRenderMinimalTranslucency)
				{
					FIntRect PixelRect;
					if (ComputeTranslucencyResolveRectangle(DPGIndex, PixelRect))
					{
						// update the custom resolve parameters
						ResolveParams.X1 = PixelRect.Min.X;
						ResolveParams.X2 = PixelRect.Max.X;
						ResolveParams.Y1 = PixelRect.Min.Y;
						ResolveParams.Y2 = PixelRect.Max.Y;
					}
					else
					{
						// if no custom bounds were found, assume we do not need to do any further work
						bRender = FALSE;
					}
				}
#endif
				
				if (bRender)
				{
					// resolve scene color
					GSceneRenderTargets.SaveSceneColorRaw(TRUE, ResolveParams);
					
					GSceneRenderTargets.BeginRenderingSceneColor();
					// Restore states changed by the resolve
					RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
					// Enable depth test, disable depth writes.
					RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());

#if XBOX
#if FINAL_RELEASE
					// Add a scissor rect to match the previously resolved area
					// We do this to prevent primitives with innacurate bounds from rendering outside the resolved area.
					// In non-final builds, we allow the out-of-bounds rendering to occur, helping visually identify these primitives.
					RHISetScissorRect(TRUE, ResolveParams.X1, ResolveParams.Y1, ResolveParams.X2, ResolveParams.Y2);
#endif
#endif
					// Draw only translucent prims that read from scene color
					bDirty |= View.TranslucentPrimSet[DPGIndex].Draw(&View,DPGIndex,TRUE);

#if XBOX
#if FINAL_RELEASE
					// clear the scissor rect
					RHISetScissorRect(FALSE, 0, 0, 0, 0);
#endif
#endif
				}
			}
		}
	}

#if PS3 && !USE_NULL_RHI
	GTriggerDrawCounter = FALSE;
#endif

	return bDirty;
}

/** 
 * Extracts LDR scene color from HDR scene color
 */
void FSceneRenderer::ExtractLDRSceneColor(UINT DPGIndex)
{
	if (!CanBlendWithFPRenderTarget(GRHIShaderPlatform))
	{
		checkSlow(DPGIndex == SDPG_World);
		SCOPED_DRAW_EVENT(EventExtractLDRSceneColor)(DEC_SCENE_ITEMS,TEXT("ExtractLDRSceneColor"));

		// Render to the scene color buffer.
		GSceneRenderTargets.BeginRenderingSceneColor();

		TShaderMapRef<FLDRExtractVertexShader> ExtractVertexShader(GetGlobalShaderMap());
		TShaderMapRef<FLDRExtractPixelShader> ExtractPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState( LDRExtractBoundShaderState, GFilterVertexDeclaration.VertexDeclarationRHI, *ExtractVertexShader, *ExtractPixelShader, sizeof(FFilterVertex) );

		// Sample the HDR Scratch Scene Color texture
		SetTextureParameter(
			ExtractPixelShader->GetPixelShader(),
			ExtractPixelShader->SceneColorScratchTextureParameter,
			TStaticSamplerState<SF_Point>::GetRHI(),
			GSceneRenderTargets.GetSceneColorScratchTexture()
			);

		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		// Disable depth test and writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			SCOPED_CONDITIONAL_DRAW_EVENT(CombineEventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("CombineView%d"),ViewIndex);

			FViewInfo& View = Views(ViewIndex);

			RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );
			RHISetViewport( View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);				

			const UINT SceneColorBufferSizeX = GSceneRenderTargets.GetBufferSizeX();
			const UINT SceneColorBufferSizeY = GSceneRenderTargets.GetBufferSizeY();

			DrawDenormalizedQuad(
				View.RenderTargetX,View.RenderTargetY,
				View.RenderTargetSizeX,View.RenderTargetSizeY,
				0,0,
				View.RenderTargetSizeX,View.RenderTargetSizeY,
				View.RenderTargetSizeX,View.RenderTargetSizeY,
				SceneColorBufferSizeX,SceneColorBufferSizeY
				);
		}
	}
}
