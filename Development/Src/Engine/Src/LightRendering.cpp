/*=============================================================================
	LightRendering.cpp: Light rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LightRendering.h"

/**
 * Helper function to encapsulate the logic for whether a light should be rendered,
 * which is needed in more than one place.
 */
static UBOOL ShouldRenderLight(const FVisibleLightViewInfo& VisibleLightViewInfo, UINT DPGIndex)
{
	// only render the light if it has visible primitives and is in the view frustum,
	// since only lights whose volumes are visible will contribute to the visible primitives.
	return VisibleLightViewInfo.DPGInfo[DPGIndex].bHasVisibleLitPrimitives && VisibleLightViewInfo.bInViewFrustum;
}

ELightInteractionType FMeshLightingDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,FLightSceneInfo* Light)
{
	if(!StaticMesh->IsTranslucent() && !StaticMesh->IsDistortion())
	{
		// Don't draw the light on the mesh if it's unlit.
		const FMaterial* const Material = StaticMesh->MaterialRenderProxy->GetMaterial();
		if(Material->GetLightingModel() != MLM_Unlit)
		{
			return Light->GetDPGInfo(StaticMesh->DepthPriorityGroup)->AttachStaticMesh(Light,StaticMesh);
		}
	}

	return LIT_CachedIrrelevant;
}

UBOOL FMeshLightingDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	const FLightSceneInfo* Light,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	if(!Mesh.IsTranslucent() && !Mesh.IsDistortion())
	{
		// Don't draw the light on the mesh if it's unlit.
		const FMaterial* const Material = Mesh.MaterialRenderProxy->GetMaterial();
		if(!IsTranslucentBlendMode(Material->GetBlendMode()) && Material->GetLightingModel() != MLM_Unlit)
		{
			// Draw the light's effect on the primitive.
			return Light->GetDPGInfo(Mesh.DepthPriorityGroup)->DrawDynamicMesh(
				View,
				Light,
				Mesh,
				bBackFace,
				bPreFog,
				PrimitiveSceneInfo,
				HitProxyId
				);
		}
	}

	return FALSE;
}

UBOOL FSceneRenderer::RenderLights(UINT DPGIndex,UBOOL bAffectedByModulatedShadows, UBOOL bWasSceneColorDirty)
{
	UBOOL bSceneColorDirty = bWasSceneColorDirty;
	UBOOL bStencilBufferDirty = FALSE;	// The stencil buffer should've been cleared to 0 already

	// Draw each light.
	for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights);LightIt;++LightIt)
	{
		const FLightSceneInfoCompact& LightSceneInfoCompact = *LightIt;

		// The light is affected by modulated shadows if it's a static light, or it has the bAffectedByModulatedShadows flag.
		const UBOOL bIsLightAffectedbyModulatedShadows =
			LightSceneInfoCompact.bStaticLighting ||
			LightSceneInfoCompact.bCastCompositeShadow;
		if(XOR(bIsLightAffectedbyModulatedShadows,bAffectedByModulatedShadows))
		{
			continue;
		}

		const UBOOL bIsLightBlack =
			Square(LightSceneInfoCompact.Color.R) < DELTA &&
			Square(LightSceneInfoCompact.Color.G) < DELTA &&
			Square(LightSceneInfoCompact.Color.B) < DELTA;

		// Nothing to do for black lights as modulated shadows are rendered independently.
		if( bIsLightBlack )
		{
			continue;
		}

		const INT LightId = LightIt.GetIndex();
		// Check if the light is visible in any of the views.
		UBOOL bLightIsVisibleInAnyView = FALSE;
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			if (ShouldRenderLight(Views(ViewIndex).VisibleLightInfos(LightId), DPGIndex))
			{
				bLightIsVisibleInAnyView = TRUE;
				break;
			}
		}

		if(bLightIsVisibleInAnyView)
		{
			const FLightSceneInfo* const LightSceneInfo = LightIt->LightSceneInfo;
	
			UBOOL bUseAttenuationBuffer = FALSE;
			UBOOL bUseStencilBuffer = FALSE;
			UBOOL bSavedRawSceneColor = FALSE;

			SCOPED_DRAW_EVENT(EventLightPass)(DEC_SCENE_ITEMS,*LightSceneInfo->GetLightName().ToString());

			// Check for a need to use the attenuation buffer
			const UBOOL bDrawShadows =	(ViewFamily.ShowFlags & SHOW_DynamicShadows) 
									&&	GSystemSettings.RenderThreadSettings.bAllowDynamicShadows
									&&	!(LightSceneInfo->LightShadowMode == LightShadow_Modulate || LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter)
									// Don't allow shadows in the foreground DPG for SM2 since it doesn't have access to those depths
									&&  (CanBlendWithFPRenderTarget(GRHIShaderPlatform) || DPGIndex == SDPG_World);
			if(bDrawShadows)
			{
				// Render non-modulated projected shadows to the attenuation buffer.
				bUseAttenuationBuffer |= CheckForProjectedShadows( LightSceneInfo, DPGIndex );

				// Render shadow volumes to the scene depth stencil buffer.
				bUseStencilBuffer |= CheckForShadowVolumes( LightSceneInfo, DPGIndex );
			}

			// Render light function to the attenuation buffer.
			bUseAttenuationBuffer |= CheckForLightFunction( LightSceneInfo, DPGIndex );

			if (bUseAttenuationBuffer)
			{
				if (bSceneColorDirty)
				{
					// Save the color buffer
					GSceneRenderTargets.SaveSceneColorRaw();
					bSavedRawSceneColor = TRUE;
				}

				// Clear the light attenuation surface to white.
				GSceneRenderTargets.BeginRenderingLightAttenuation();
				RHIClear(TRUE,FLinearColor::White,FALSE,0,FALSE,0);
				
				if(bDrawShadows)
				{
					// Render non-modulated projected shadows to the attenuation buffer.
					RenderProjectedShadows( LightSceneInfo, DPGIndex );
				}
			}

			if (bUseStencilBuffer && bDrawShadows)
			{
				// Render shadow volumes to the scene depth stencil buffer.
				bStencilBufferDirty |= RenderShadowVolumes( LightSceneInfo, DPGIndex );
			}
	
			if (bUseAttenuationBuffer)
			{
				// Render light function to the attenuation buffer.
				RenderLightFunction( LightSceneInfo, DPGIndex );

				// Resolve light attenuation buffer
				GSceneRenderTargets.FinishRenderingLightAttenuation();
			}

			GSceneRenderTargets.SetLightAttenuationMode(bUseAttenuationBuffer);

			// Rendering the light pass to the scene color buffer.

			if( bSavedRawSceneColor )
			{
				GSceneRenderTargets.RestoreSceneColorRaw();
				GSceneRenderTargets.BeginRenderingSceneColor(FALSE);
			}
			else
			{
				GSceneRenderTargets.BeginRenderingSceneColor(bUseAttenuationBuffer);
			}

			// Render the light to the scene color buffer, conditionally using the attenuation buffer or a 1x1 white texture as input 
			bSceneColorDirty |= RenderLight( LightSceneInfo, DPGIndex );

			// Do not resolve to scene color texture, this is done lazily
			GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
			
	        // Only clear the stencil buffer if it has been written to since the last clear.
	        if(bStencilBufferDirty)
	        {
		        GSceneRenderTargets.BeginRenderingShadowVolumes();
		        RHIClear(FALSE,FLinearColor::Black,FALSE,0,TRUE,0);
		        GSceneRenderTargets.FinishRenderingShadowVolumes();
        
		        bStencilBufferDirty = FALSE;
	        }

		}
	}

	// Restore the default mode
	GSceneRenderTargets.SetLightAttenuationMode(TRUE);

	return bSceneColorDirty;
}

/**
 * Used by RenderLights to render a light to the scene color buffer.
 *
 * @param LightSceneInfo Represents the current light
 * @param LightIndex The light's index into FScene::Lights
 * @return TRUE if anything got rendered
 */
UBOOL FSceneRenderer::RenderLight( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	SCOPE_CYCLE_COUNTER(STAT_LightingDrawTime);
	SCOPED_DRAW_EVENT(EventRenderLight)(DEC_SCENE_ITEMS,TEXT("Light"));

	UBOOL bDirty = FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		const FViewInfo& View = Views(ViewIndex);
		const FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightSceneInfo->Id);

		if( ShouldRenderLight(VisibleLightViewInfo, DPGIndex) )
		{
			// Set the device viewport for the view.
			RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters(&View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

			// Set the light's scissor rectangle.
 			LightSceneInfo->SetScissorRect(&View);
 			// Set the light's depth bounds
 			LightSceneInfo->SetDepthBounds(&View);

			if (CanBlendWithFPRenderTarget(GRHIShaderPlatform))
			{
				// Additive blending, depth tests, no depth writes, preserve destination alpha.
				RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_One,BO_Add,BF_Zero,BF_One>::GetRHI());
			}
			else
			{
				// Additive blending, depth tests, no depth writes, accumulate scale factor in alpha.
				RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_One,BO_Add,BF_One,BF_One>::GetRHI());
			}
			
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());
			RHISetStencilState(TStaticStencilState<TRUE,CF_Equal>::GetRHI());

			// Draw the light's effect on static meshes.
			bDirty |= LightSceneInfo->GetDPGInfo(DPGIndex)->DrawStaticMeshesVisible(View,View.StaticMeshVisibilityMap,FLightSceneDPGInfoInterface::ELightPass_Default);

			// Draw the static batched decals affected by the light
			{
				SCOPE_CYCLE_COUNTER(STAT_DecalRenderLitTime);

				UBOOL bAnyDecalsRendered =
					LightSceneInfo->GetDPGInfo(DPGIndex)->DrawStaticMeshesVisible(
						View,View.DecalStaticMeshVisibilityMap,FLightSceneDPGInfoInterface::ELightPass_Decals);

				// If we rendered any decals, then its likely that the scissor rect was changed by the
				// decal drawing code (FMeshDrawingPolicy::SetMeshRenderState), so we'll reset the light's
				// scissor rect
				if( bAnyDecalsRendered )
				{
					LightSceneInfo->SetScissorRect( &View );
				}

				bDirty |= bAnyDecalsRendered;
			}

			{
				// Draw the light's effect on dynamic meshes.
				TDynamicPrimitiveDrawer<FMeshLightingDrawingPolicyFactory> Drawer(&View,DPGIndex,LightSceneInfo,TRUE);
				{
					for(INT PrimitiveIndex = 0;PrimitiveIndex < VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleDynamicLitPrimitives.Num();PrimitiveIndex++)
					{
						const FPrimitiveSceneInfo* PrimitiveSceneInfo = VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleDynamicLitPrimitives(PrimitiveIndex);
						if(View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
						{
							Drawer.SetPrimitive(PrimitiveSceneInfo);
							PrimitiveSceneInfo->Proxy->DrawDynamicElements(&Drawer,&View,DPGIndex);
						}
					}
					bDirty |= Drawer.IsDirty();
				}

				// Render decals for visible primitives affected by this light.
				{
					Drawer.ClearDirtyFlag();
					for(INT PrimitiveIndex = 0;PrimitiveIndex < VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives.Num();PrimitiveIndex++)
					{
						const FPrimitiveSceneInfo* PrimitiveSceneInfo = VisibleLightViewInfo.DPGInfo[DPGIndex].VisibleLitDecalPrimitives(PrimitiveIndex);
						if(View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id))
						{
							SCOPE_CYCLE_COUNTER(STAT_DecalRenderLitTime);

							Drawer.SetPrimitive(PrimitiveSceneInfo);
							PrimitiveSceneInfo->Proxy->DrawDynamicDecalElements(&Drawer,&View,DPGIndex,TRUE,FALSE);
						}
					}

					// If we rendered any decals, then its likely that the scissor rect was changed by the
					// decal drawing code (FMeshDrawingPolicy::SetMeshRenderState), so we'll reset the light's
					// scissor rect
					if( Drawer.IsDirty() )
					{
						LightSceneInfo->SetScissorRect( &View );
					}

					bDirty |= Drawer.IsDirty();
				}
			}

			// Reset the scissor rectangle and stencil state.
			RHISetScissorRect(FALSE,0,0,0,0);
			RHISetDepthBoundsTest( FALSE, FVector4(0.0f,0.0f,0.0f,1.0f), FVector4(0.0f,0.0f,1.0f,1.0f));
			RHISetStencilState(TStaticStencilState<>::GetRHI());
		}
	}
	return bDirty;
}

UBOOL FSceneRenderer::RenderModulatedShadows(UINT DPGIndex)
{
	UBOOL bSceneColorDirty = FALSE;

	const UBOOL bRenderModulatedShadows = 
		// Skip shadows for SM2 in the foreground DPG, since it doesn't have access to those depths.
		CanBlendWithFPRenderTarget(GRHIShaderPlatform) || DPGIndex == SDPG_World;
	
	if (bRenderModulatedShadows)
	{
		SCOPE_CYCLE_COUNTER(STAT_ModulatedShadowDrawTime);
		SCOPED_DRAW_EVENT(EventRenderModShadow)(DEC_SCENE_ITEMS,TEXT("ModShadow"));
		
		// Render modulated projected shadows to scene color buffer.
		GSceneRenderTargets.BeginRenderingSceneColor(FALSE);

		// We just assume that the stencil buffer was left dirty by RenderLights.
		UBOOL bStencilBufferDirty = TRUE;

		// Draw each light.
		for(INT LightIndex=0; LightIndex<VisibleShadowCastingLightInfos.Num(); LightIndex++ )
		{
			const FLightSceneInfo* LightSceneInfo = VisibleShadowCastingLightInfos(LightIndex);
			
			// Only look at lights using modulated shadows.
			const UBOOL bHasModulatedShadows =
				LightSceneInfo->LightShadowMode == LightShadow_Modulate ||
				LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter;
			if(LightSceneInfo->LightType != LightType_Sky && bHasModulatedShadows)
			{
				UBOOL bUseStencilBuffer = CheckForShadowVolumes( LightSceneInfo, DPGIndex );

				bSceneColorDirty |= RenderProjectedShadows( LightSceneInfo, DPGIndex );

				// Modulate the attenuated shadow volumes with the scene color buffer.
				if(bUseStencilBuffer)
				{
					// Render shadow volumes to the scene depth stencil buffer.
					bStencilBufferDirty |= RenderShadowVolumes(LightSceneInfo,DPGIndex);

					//Set Scene Color as the render target even if no shadow volumes were rendered, 
					//because the current color buffer may have been set to something else in RenderShadowVolumes
					GSceneRenderTargets.BeginRenderingSceneColor(FALSE);

					if(bStencilBufferDirty)
					{
						// Render attenuated shadows to light attenuation target
						bSceneColorDirty |= AttenuateShadowVolumes( LightSceneInfo );
						GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
					}
				}
			}
		}	

		GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
	}
	return bSceneColorDirty;
}
