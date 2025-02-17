/*=============================================================================
	ShadowVolumeRendering.cpp: Implementation for rendering shadow volumes.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"

//
FShadowVolumeCache::~FShadowVolumeCache()
{
	// Remove the cache from the resource list.
	check(IsInRenderingThread());
	ReleaseResource();
}

//
FShadowVolumeCache::FCachedShadowVolume* FShadowVolumeCache::AddShadowVolume(const class FLightSceneInfo* LightSceneInfo,FShadowIndexBuffer& IndexBuffer)
{
	// If the cache isn't already in the resource list, add it.
	check(IsInRenderingThread());
	InitResource();

	FCachedShadowVolume ShadowVolume;
	ShadowVolume.NumTriangles = IndexBuffer.Indices.Num() / 3;

	DWORD Size = IndexBuffer.Indices.Num() * sizeof(WORD);
	if( Size )
	{
		ShadowVolume.IndexBufferRHI = RHICreateIndexBuffer( sizeof(WORD), Size, NULL, RUF_Static /*RUF_Dynamic*/ );

		// Initialize the buffer.
		void* Buffer = RHILockIndexBuffer(ShadowVolume.IndexBufferRHI,0,Size);
		appMemcpy(Buffer,&IndexBuffer.Indices(0),Size);
		RHIUnlockIndexBuffer(ShadowVolume.IndexBufferRHI);
	}

	return &CachedShadowVolumes.Set(LightSceneInfo,ShadowVolume);
}

//
void FShadowVolumeCache::RemoveShadowVolume(const class FLightSceneInfo* LightSceneInfo)
{
	CachedShadowVolumes.Remove(LightSceneInfo);
}

//
const FShadowVolumeCache::FCachedShadowVolume* FShadowVolumeCache::GetShadowVolume(const class FLightSceneInfo* LightSceneInfo) const
{
	return CachedShadowVolumes.Find(LightSceneInfo);
}

//
void FShadowVolumeCache::ReleaseRHI()
{
	// Release the cached shadow volume index buffers.
	CachedShadowVolumes.Empty();
}

/**
 * A vertex shader for rendering a shadow volume.
 */
class FShadowVolumeVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FShadowVolumeVertexShader,Global);
public:

	/**
	 * Returns whether the vertex shader should be cached or not.
	 *
	 * @return	Always returns TRUE
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	/** Empty default constructor */
	FShadowVolumeVertexShader( )	{ }

	/**
	 * Constructor that initializes the shader parameters.
	 *
	 * @param Initializer	Used for finding the shader parameters
	 */
	FShadowVolumeVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		LightPosition.Bind(Initializer.ParameterMap,TEXT("LightPosition"));
		BaseExtrusion.Bind(Initializer.ParameterMap,TEXT("BaseExtrusion"));
		LocalToWorld.Bind(Initializer.ParameterMap,TEXT("LocalToWorld"),TRUE);
	}

	/**
	 * Activates the vertex shaders for rendering and sets the shader parameters that are common
	 * to all shadow volumes.
	 *
	 * @param View				Current view
	 * @param LightSceneInfo	Represents the current light
	 */
	void SetParameters( const FSceneView* View, const FLightSceneInfo* LightSceneInfo )
	{
		// Set the homogenous position of the light.
		SetVertexShaderValue(
			GetVertexShader(),
			LightPosition,
			LightSceneInfo->GetPosition() + FVector4(View->PreViewTranslation * LightSceneInfo->GetPosition().W,0)
			);
	}

	/**
	 * Sets the shader parameters that are different between shadow volumes.
	 *
	 * @param InLocalToWorld	Transformation matrix from local space to world space
	 * @param InBaseExtrusion	Amount to extrude front-facing triangles to handle z-fighting in some cases (usually 0.0f)
	 */
	void SetInstanceParameters(const FSceneView* View,const FMatrix& InLocalToWorld,FLOAT InBaseExtrusion)
	{
		// Set a small extrusion for front-facing polygons.
		SetVertexShaderValue(GetVertexShader(),BaseExtrusion,InBaseExtrusion);

		// Set the local to world matrix.
		SetVertexShaderValue(GetVertexShader(),LocalToWorld,InLocalToWorld.ConcatTranslation(View->PreViewTranslation));
	}

	/**
	 * Serialize the vertex shader.
	 *
	 * @param Ar	Archive to serialize to/from
	 */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << LightPosition;
		Ar << BaseExtrusion;
		Ar << LocalToWorld;
		return bShaderHasOutdatedParameters;
	}

	virtual EShaderRecompileGroup GetRecompileGroup() const
	{
		return SRG_GLOBAL_MISC_SHADOW;
	}


private:
	/** Represents the shader registers for the homogenous world space position of light */
	FShaderParameter	LightPosition;

	/** Represents the shader registers for the amount to extrude front-facing triangles */
	FShaderParameter	BaseExtrusion;

	/** Represents the shader registers for the matrix that transforms from local space to world space */
	FShaderParameter	LocalToWorld;
};

IMPLEMENT_SHADER_TYPE(,FShadowVolumeVertexShader,TEXT("ShadowVolumeVertexShader"),TEXT("Main"),SF_Vertex,0,0);


/**
 * Helper class that is used by FSceneRenderer::RenderShadowVolumes and FPrimitiveSceneProxy.
 */
class FMyShadowVolumeDrawInterface : public FShadowVolumeDrawInterface
{
public:
	/**
	 * Constructor
	 *
	 * @param InLightSceneInfo	Represents the current light
	 */
	FMyShadowVolumeDrawInterface( const FLightSceneInfo* InLightSceneInfo,UINT InDepthPriorityGroup ):
		ShadowVolumeVertexShader(GetGlobalShaderMap()),
		View(NULL),
		LightSceneInfo(InLightSceneInfo),
		DepthPriorityGroup(InDepthPriorityGroup),
		bDirty(FALSE)
	{
	}

	/** Destructor */
	~FMyShadowVolumeDrawInterface()
	{
		// Restore states if necessary.
		if ( bDirty )
		{
			RHISetDepthState(TStaticDepthState<>::GetRHI());
			RHISetColorWriteEnable(TRUE);
			RHISetStencilState(TStaticStencilState<>::GetRHI());
			RHISetRasterizerState(TStaticRasterizerState<>::GetRHI());
		}
		GSceneRenderTargets.FinishRenderingShadowVolumes();
	}

	/**
	 * Returns whether the render target has been modified or not; that is, if any
	 * shadow volumes got rendered.
	 *
	 * @return	TRUE if any shadow volume was rendered
	 */
	UBOOL IsDirty( )
	{
		return bDirty;
	}

	/**
	 * FPrimitiveSceneProxy will call back to the virtual this->DrawShadowVolume().
	 *
	 * @param Interaction	Represents the current view
	 */
	void SetView( const FViewInfo *InView )
	{
		View = InView;

		GSceneRenderTargets.BeginRenderingShadowVolumes();

		// Set the device viewport for the view.
		RHISetViewport(View->RenderTargetX,View->RenderTargetY,0.0f,View->RenderTargetX + View->RenderTargetSizeX,View->RenderTargetY + View->RenderTargetSizeY,1.0f);
		RHISetViewParameters( View, View->TranslatedViewProjectionMatrix, View->ViewOrigin );

		// Set the light's scissor rectangle.
		LightSceneInfo->SetScissorRect(View);
	}

	/**
	 * Called by FSceneRenderer::RenderShadowVolumes() for each primitive affected by the light.
	 *
	 * @param Interaction	Represents the light to draw shadows from
	 */
	void DrawShadowVolume( const FLightSceneInfo* LightSceneInfo, const FPrimitiveSceneInfo* PrimitiveSceneInfo )
	{
		// Compute the primitive's view relevance.  Note that the view won't necessarily have it cached,
		// since the primitive might not be visible.
		FPrimitiveViewRelevance ViewRelevance = PrimitiveSceneInfo->Proxy->GetViewRelevance(View);

		// Only draw shadow volumes for primitives which are relevant in this view and DPG.
		if(ViewRelevance.IsRelevant() && ViewRelevance.GetDPG(DepthPriorityGroup))
		{		
			// Whether the shadow volume has been rendered or not.
			UBOOL bShadowVolumeWasRendered = FALSE;

			// Only render shadow volumes for lights above a certain threshold.
			if( (!LightSceneInfo->bStaticShadowing && !LightSceneInfo->bStaticLighting)
			||	LightSceneInfo->GetRadius() == 0.f 
			||  LightSceneInfo->GetRadius() > GSystemSettings.ShadowVolumeLightRadiusThreshold )
			{
				FVector4 ScreenPosition = View->WorldToScreen(PrimitiveSceneInfo->Bounds.Origin);

				// Determine screen space percentage of bounding sphere.
				FLOAT ScreenPercentage = Max(
					1.0f / 2.0f * View->ProjectionMatrix.M[0][0],
					1.0f / 2.0f * View->ProjectionMatrix.M[1][1]
				) 
				*	PrimitiveSceneInfo->Bounds.SphereRadius
				/	Max(Abs(ScreenPosition.W),1.0f);

				// Only draw shadow volumes for primitives with bounds covering more than a specific percentage of the screen.
				if( (!LightSceneInfo->bStaticShadowing && !LightSceneInfo->bStaticLighting)
				||	ScreenPercentage > GSystemSettings.ShadowVolumePrimitiveScreenSpacePercentageThreshold )
				{
					PrimitiveSceneInfo->Proxy->DrawShadowVolumes( this, View, LightSceneInfo, DepthPriorityGroup );
					bShadowVolumeWasRendered = TRUE;
				}
			}

			// Remove cached data if it hasn't been used to render this view. This is only done in the Editor to avoid
			// running out of memory. The code doesn't handle splitscreen or multiple perspective windows.
			if( GIsEditor && !bShadowVolumeWasRendered )
			{
				PrimitiveSceneInfo->Proxy->RemoveCachedShadowVolumeData( LightSceneInfo );
			}
		}
	}

	/**
	  * Called by FPrimitiveSceneProxy to render shadow volumes to the stencil buffer.
	  *
	  * @param IndexBuffer Shadow volume index buffer, indexing into the original vertex buffer
	  * @param VertexFactory Vertex factory that represents the shadow volume vertex format
	  * @param LocalToWorld World matrix for the primitive
	  * @param FirstIndex First index to use in the index buffer
	  * @param NumPrimitives Number of triangles in the triangle list
	  * @param MinVertexIndex Lowest index used in the index buffer
	  * @param MaxVertexIndex Highest index used in the index buffer
	  */
	virtual void DrawShadowVolume(
		FIndexBufferRHIParamRef IndexBuffer,
		const FLocalShadowVertexFactory& VertexFactory,
		const FMatrix& LocalToWorld,
		UINT FirstIndex,
		UINT NumPrimitives,
		UINT MinVertexIndex,
		UINT MaxVertexIndex
		)
	{
		// Set states if necessary.
		if ( !bDirty )
		{
			RHISetDepthState(TStaticDepthState<FALSE,CF_Less>::GetRHI());
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
			ShadowVolumeVertexShader->SetParameters( View, LightSceneInfo);
			bDirty = TRUE;
		}

		// Check the determinant of the LocalToWorld transform to determine the winding of the shadow volume.
		const FLOAT LocalToWorldDeterminant = LocalToWorld.Determinant();
		if(IsNegativeFloat(LocalToWorldDeterminant))
		{
			RHISetStencilState(TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Increment,SO_Keep,TRUE,CF_Always,SO_Keep,SO_Decrement,SO_Keep>::GetRHI());
		}
		else
		{
			RHISetStencilState(TStaticStencilState<TRUE,CF_Always,SO_Keep,SO_Decrement,SO_Keep,TRUE,CF_Always,SO_Keep,SO_Increment,SO_Keep>::GetRHI());
		}

		ShadowVolumeVertexShader->SetInstanceParameters( View, LocalToWorld, VertexFactory.GetBaseExtrusion());
		
		VertexFactory.Set();

		/** bound shader state for volume rendering */
		FBoundShaderStateRHIRef ShadowVolumeBoundShaderState;
		DWORD Strides[MaxVertexElementCount];
		VertexFactory.GetStreamStrides(Strides);
		ShadowVolumeBoundShaderState = RHICreateBoundShaderState(
			VertexFactory.GetDeclaration(), 
			Strides, 
			ShadowVolumeVertexShader->GetVertexShader(), 
			FPixelShaderRHIRef()
			);
		
		RHISetBoundShaderState( ShadowVolumeBoundShaderState);

		RHIDrawIndexedPrimitive( IndexBuffer, PT_TriangleList, 0, MinVertexIndex, MaxVertexIndex-MinVertexIndex, FirstIndex, NumPrimitives);
	}

private:
	/** Vertex shader that extrudes the shadow volume. */
	TShaderMapRef<FShadowVolumeVertexShader> ShadowVolumeVertexShader;	

	/** Current view. */
	const FViewInfo *View;

	/** Current light. */
	const FLightSceneInfo *LightSceneInfo;

	/** Current DPG. */
	BITFIELD DepthPriorityGroup : UCONST_SDPG_NumBits;

	/** Flag that tells whether any shadow volumes got rendered to the render target. */
	UBOOL bDirty;
};

/**
* Used by RenderLights to figure out if shadow volumes need to be rendered to the stencil buffer.
*
* @param LightSceneInfo Represents the current light
* @return TRUE if anything needs to be rendered
*/
UBOOL FSceneRenderer::CheckForShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	UBOOL bResult=FALSE;
	if( UEngine::ShadowVolumesAllowed() )
	{
		const FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);
		if (VisibleLightInfo.ShadowVolumePrimitives.Num())
		{
			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				const FViewInfo& View = Views(ViewIndex);
				const FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightSceneInfo->Id);
				const FVisibleLightViewInfo::FDPGInfo& VisibleLightDPGInfo = VisibleLightViewInfo.DPGInfo[DPGIndex];
				// shadow volume is rendered if any lit primitives affected by this light are visible
				// or if it is using modulated shadows and is visible in the view frustum
				if( VisibleLightDPGInfo.bHasVisibleLitPrimitives ||
					(LightSceneInfo->LightShadowMode == LightShadow_Modulate && VisibleLightViewInfo.bInViewFrustum) ||
					(LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter && VisibleLightViewInfo.bInViewFrustum) )
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
* Used by RenderLights to figure out if shadow volumes need to be rendered to the attenuation buffer.
*
* @param LightSceneInfo Represents the current light
* @return TRUE if anything needs to be rendered
*/
UBOOL FSceneRenderer::CheckForShadowVolumeAttenuation( const FLightSceneInfo* LightSceneInfo )
{
	return(	LightSceneInfo->LightShadowMode == LightShadow_Modulate || 
			LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter );
}

/**
 * Used by FSceneRenderer::RenderLights to render shadow volumes to the attenuation buffer.
 *
 * @param LightSceneInfo Represents the current light
 * @return TRUE if anything got rendered
 */
UBOOL FSceneRenderer::RenderShadowVolumes( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	SCOPED_DRAW_EVENT(EventShadowVolume)(DEC_SCENE_ITEMS,TEXT("Shadow volume"));
	SCOPE_CYCLE_COUNTER(STAT_ShadowVolumeDrawTime);

	UBOOL bResult=FALSE;
	if( UEngine::ShadowVolumesAllowed() )
	{
		const FVisibleLightInfo& VisibleLightInfo = VisibleLightInfos(LightSceneInfo->Id);
		if(VisibleLightInfo.ShadowVolumePrimitives.Num())
		{
			// Render to the stencil buffer for all views.
			FMyShadowVolumeDrawInterface SVDI(  LightSceneInfo, DPGIndex );
			for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
			{
				SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

				const FViewInfo& View = Views(ViewIndex);
				const FVisibleLightViewInfo& VisibleLightViewInfo = View.VisibleLightInfos(LightSceneInfo->Id);
				const FVisibleLightViewInfo::FDPGInfo& VisibleLightDPGInfo = VisibleLightViewInfo.DPGInfo[DPGIndex];
				// shadow volume is rendered if any lit primitives affected by this light are visible
				// or if it is using modulated shadows and is visible in the view frustum
				if( VisibleLightDPGInfo.bHasVisibleLitPrimitives ||
					(LightSceneInfo->LightShadowMode == LightShadow_Modulate && VisibleLightViewInfo.bInViewFrustum) ||
					(LightSceneInfo->LightShadowMode == LightShadow_ModulateBetter && VisibleLightViewInfo.bInViewFrustum) )
				{
					SVDI.SetView( &View );

					// Iterate over all primitives that interacts with this light.
					for(INT PrimitiveIndex = 0;PrimitiveIndex < VisibleLightInfo.ShadowVolumePrimitives.Num();PrimitiveIndex++)
					{
						const FPrimitiveSceneInfo* PrimitiveSceneInfo = VisibleLightInfo.ShadowVolumePrimitives(PrimitiveIndex);
						#if STATS
							if( bShouldGatherDynamicShadowStats )
							{
								// Find the interaction.
								FLightPrimitiveInteraction* MatchingInteraction = NULL;
								for(FLightPrimitiveInteraction* Interaction = PrimitiveSceneInfo->LightList;
									Interaction;
									Interaction = Interaction->GetNextLight())
								{
									if(Interaction->GetLight() == LightSceneInfo)
									{
										MatchingInteraction = Interaction;
										break;
									}
								}

								FCombinedShadowStats ShadowStat;
								ShadowStat.SubjectPrimitives.AddItem( PrimitiveSceneInfo );
								InteractionToDynamicShadowStatsMap.Set( MatchingInteraction, ShadowStat );
							}
						#endif
						SVDI.DrawShadowVolume( LightSceneInfo, PrimitiveSceneInfo );
					}

					// Reset the scissor rectangle.
					RHISetScissorRect(FALSE,0,0,0,0);
				}
			}

			bResult = SVDI.IsDirty();
		}
	}
	return bResult;	
}

/*-----------------------------------------------------------------------------
FModShadowVolumeVertexShader
-----------------------------------------------------------------------------*/

/**
* Constructor - binds all shader params
* @param Initializer - init data from shader compiler
*/
FModShadowVolumeVertexShader::FModShadowVolumeVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FGlobalShader(Initializer)
{		
}

/**
* @param Platform - current platform being compiled
* @return TRUE if this global shader should be compiled
*/
UBOOL FModShadowVolumeVertexShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

/**
* Sets the current vertex shader
* @param View - current view
* @param ShadowInfo - projected shadow info for a single light
*/
void FModShadowVolumeVertexShader::SetParameters(
									   const FSceneView* View,
									   const FLightSceneInfo* LightSceneInfo
									   )
{
}

/**
* Serialize the parameters for this shader
* @param Ar - archive to serialize to
*/
UBOOL FModShadowVolumeVertexShader::Serialize(FArchive& Ar)
{
	return FShader::Serialize(Ar);
}

IMPLEMENT_SHADER_TYPE(,FModShadowVolumeVertexShader,TEXT("ModShadowVolumeVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/*-----------------------------------------------------------------------------
FModShadowVolumePixelShader
-----------------------------------------------------------------------------*/

/**
* Constructor - binds all shader params
* @param Initializer - init data from shader compiler
*/
FModShadowVolumePixelShader::FModShadowVolumePixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
:	FGlobalShader(Initializer)
{
	SceneTextureParams.Bind(Initializer.ParameterMap);
	ShadowModulateColorParam.Bind(Initializer.ParameterMap,TEXT("ShadowModulateColor"));
	ScreenToWorldParam.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"),TRUE);
}

/**
* @param Platform - current platform being compiled
* @return TRUE if this global shader should be compiled
*/
UBOOL FModShadowVolumePixelShader::ShouldCache(EShaderPlatform Platform)
{
	return TRUE;
}

/**
* Sets the current pixel shader
* @param View - current view
* @param ShadowInfo - projected shadow info for a single light
*/
void FModShadowVolumePixelShader::SetParameters(
									  const FSceneView* View,
									  const FLightSceneInfo* LightSceneInfo
									  )
{
	SceneTextureParams.Set(View,this);

	// color to modulate shadowed areas on screen
	SetPixelShaderValue( GetPixelShader(), ShadowModulateColorParam, LightSceneInfo->ModShadowColor );
	// screen space to world space transform
	FMatrix ScreenToWorld = FMatrix(
		FPlane(1,0,0,0),
		FPlane(0,1,0,0),
		FPlane(0,0,(1.0f - Z_PRECISION),1),
		FPlane(0,0,-View->NearClippingDistance * (1.0f - Z_PRECISION),0)
		) * 
		View->InvViewProjectionMatrix;
	SetPixelShaderValue( GetPixelShader(), ScreenToWorldParam, ScreenToWorld );

}

/**
* Serialize the parameters for this shader
* @param Ar - archive to serialize to
*/
UBOOL FModShadowVolumePixelShader::Serialize(FArchive& Ar)
{
	UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
	Ar << SceneTextureParams;
	Ar << ShadowModulateColorParam;
	Ar << ScreenToWorldParam;
	return bShaderHasOutdatedParameters;
}

/**
* Attenuate the shadowed area of a shadow volume. For use with modulated shadows
* @param LightSceneInfo - Represents the current light
* @return TRUE if anything got rendered
*/
UBOOL FSceneRenderer::AttenuateShadowVolumes( const FLightSceneInfo* LightSceneInfo )
{
	SCOPE_CYCLE_COUNTER(STAT_LightingDrawTime);
	SCOPED_DRAW_EVENT(EventRenderModShadowVolume)(DEC_SCENE_ITEMS,TEXT("ModShadowVolume"));

	UBOOL bDirty = FALSE;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		const FViewInfo* View = &Views(ViewIndex);

		// Set the device viewport for the view.
		RHISetViewport(View->RenderTargetX,View->RenderTargetY,0.0f,View->RenderTargetX + View->RenderTargetSizeX,View->RenderTargetY + View->RenderTargetSizeY,1.0f);
		RHISetViewParameters(  View, FMatrix::Identity, View->ViewOrigin );

		// Set the light's scissor rectangle.
		LightSceneInfo->SetScissorRect(View);

		// set stencil to only render for the shadow volume fragments
		RHISetStencilState(TStaticStencilState<TRUE,CF_NotEqual>::GetRHI());
		// set fill/blend state
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

		// modulated blending, preserve alpha
		RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero,BO_Add,BF_Zero,BF_One>::GetRHI());	

		// turn off depth reads/writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

		// Set the shader state.
		TShaderMapRef<FModShadowVolumeVertexShader> VertexShader(GetGlobalShaderMap());
		VertexShader->SetParameters(View,LightSceneInfo);

		FModShadowVolumePixelShader* PixelShader = LightSceneInfo->GetModShadowVolumeShader();
		check(PixelShader);
		PixelShader->SetParameters(View,LightSceneInfo);

		RHISetBoundShaderState(LightSceneInfo->GetModShadowVolumeBoundShaderState());

		// Draw a quad for the view.
		const UINT BufferSizeX = GSceneRenderTargets.GetBufferSizeX();
		const UINT BufferSizeY = GSceneRenderTargets.GetBufferSizeY();
		DrawDenormalizedQuad(
			0,0,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			View->RenderTargetX,View->RenderTargetY,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			View->RenderTargetSizeX,View->RenderTargetSizeY,
			BufferSizeX,BufferSizeY
			);

		// Reset the scissor rectangle and stencil state.
		RHISetScissorRect(FALSE,0,0,0,0);
		RHISetStencilState(TStaticStencilState<>::GetRHI());

		bDirty = TRUE;
	}

	return bDirty;
}

