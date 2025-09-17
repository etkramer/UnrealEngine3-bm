/*=============================================================================
	LightFunctionRendering.cpp: Implementation for rendering light functions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"


/**
 * A vertex shader for projecting a light function onto the scene.
 */
class FLightFunctionVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FLightFunctionVertexShader,Material);
public:

	/**
	  * Makes sure only shaders for materials that are explicitly flagged
	  * as 'UsedAsLightFunction' in the Material Editor gets compiled into
	  * the shader cache.
	  */
	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial* Material)
	{
		return Material->IsLightFunction();
	}

	FLightFunctionVertexShader( )	{ }
	FLightFunctionVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
	}

	void SetParameters( const FSceneView* View )
	{
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		return FShader::Serialize(Ar);
	}
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FLightFunctionVertexShader,TEXT("LightFunctionVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
 * A pixel shader for projecting a light function onto the scene.
 */
class FLightFunctionPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FLightFunctionPixelShader,Material);
public:

	/**
	  * Makes sure only shaders for materials that are explicitly flagged
	  * as 'UsedAsLightFunction' in the Material Editor gets compiled into
	  * the shader cache.
	  */
	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial* Material)
	{
		return Material->IsLightFunction();
	}

	FLightFunctionPixelShader() {}
	FLightFunctionPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		SceneColorTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneColorTexture"),TRUE);
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap,TEXT("SceneDepthTexture"),TRUE);
		SceneDepthCalcParameter.Bind(Initializer.ParameterMap,TEXT("MinZ_MaxZRatio"),TRUE);
		ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
		ScreenToLightParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToLight"),TRUE);
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
	}

	void SetParameters( const FSceneView* View, const FLightSceneInfo* LightSceneInfo )
	{
		// Set the texture containing the scene depth.
		if(SceneColorTextureParameter.IsBound())
		{
			SetTextureParameter(
				GetPixelShader(),
				SceneColorTextureParameter,
				TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
				GSceneRenderTargets.GetSceneColorTexture()
				);
		}
		else if(SceneDepthTextureParameter.IsBound())
		{
			if (GSupportsDepthTextures && IsValidRef(GSceneRenderTargets.GetSceneDepthTexture()))
			{
				SetTextureParameter(
					GetPixelShader(),
					SceneDepthTextureParameter,
					TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					GSceneRenderTargets.GetSceneDepthTexture()
					);
			}
			else if (GRHIShaderPlatform == SP_PCD3D_SM2 && IsValidRef(GSceneRenderTargets.GetSceneColorScratchTexture()))
			{
				SetTextureParameter(
					GetPixelShader(),
					SceneDepthTextureParameter,
					TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),
					GSceneRenderTargets.GetSceneColorScratchTexture()
					);
			}
		}

		RHISetViewPixelParameters( View, GetPixelShader(), &SceneDepthCalcParameter, &ScreenPositionScaleBiasParameter );

		// Set the transform from screen space to light space.
		if ( ScreenToLightParameter.IsBound() )
		{
			FVector InverseScale = FVector( 1.f / LightSceneInfo->LightFunctionScale.X, 1.f / LightSceneInfo->LightFunctionScale.Y, 1.f / LightSceneInfo->LightFunctionScale.Z );
			FMatrix WorldToLight = LightSceneInfo->WorldToLight * FScaleMatrix(FVector(InverseScale));	
			FMatrix ScreenToLight = 
				FMatrix(
				FPlane(1,0,0,0),
				FPlane(0,1,0,0),
				FPlane(0,0,(1.0f - Z_PRECISION),1),
				FPlane(0,0,-View->NearClippingDistance * (1.0f - Z_PRECISION),0)
				) * View->InvViewProjectionMatrix * WorldToLight;

			SetPixelShaderValue( GetPixelShader(), ScreenToLightParameter, ScreenToLight );
		}

		// Set additional material parameters used by the emissive shader expression.
		FMaterialRenderContext MaterialRenderContext(LightSceneInfo->LightFunction, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneColorTextureParameter;
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthCalcParameter;
		Ar << ScreenPositionScaleBiasParameter;
		Ar << ScreenToLightParameter;
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter SceneColorTextureParameter;
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderParameter SceneDepthCalcParameter;
	FShaderParameter ScreenPositionScaleBiasParameter;
	FShaderParameter ScreenToLightParameter;
	FMaterialPixelShaderParameters MaterialParameters;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FLightFunctionPixelShader,TEXT("LightFunctionPixelShader"),TEXT("Main"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/** The light function vertex declaration resource type. */
class FLightFunctionVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	/** Destructor. */
	virtual ~FLightFunctionVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float2,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** Vertex declaration for the light function fullscreen 2D quad. */
TGlobalResource<FLightFunctionVertexDeclaration> GLightFunctionVertexDeclaration;

//@GEMINI_TODO: Use a quadlist
/** 
* Simple vertex buffer class for full screen quad
*/
class FLightFunctionVertexBuffer : public FVertexBuffer
{
public:
	/** 
	* Initialize the RHI for this rendering resource 
	*/
	virtual void InitRHI()
	{
		// create a static vertex buffer
		VertexBufferRHI = RHICreateVertexBuffer(4 * sizeof(FVector2D), NULL, RUF_Static);
		FVector2D* Vertices = (FVector2D*)RHILockVertexBuffer(VertexBufferRHI, 0, 4 * sizeof(FVector2D), FALSE);
		Vertices[0] = FVector2D(-1, -1);
		Vertices[1] = FVector2D(-1, +1);
		Vertices[2] = FVector2D(+1, +1);
		Vertices[3] = FVector2D(+1, -1);
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};
/** Vertex buffer for the light function fullscreen 2D quad. */
TGlobalResource<FLightFunctionVertexBuffer> GLightFunctionVertexBuffer;

/** 
* Simple index buffer class for full screen quad
*/
class FLightFunctionIndexBuffer : public FIndexBuffer
{
	/** 
	* Initialize the RHI for this rendering resource 
	*/
	virtual void InitRHI()
	{
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(WORD), 6 * sizeof(WORD), NULL, RUF_Static);
		WORD* Indices = (WORD*)RHILockIndexBuffer(IndexBufferRHI, 0, 6 * sizeof(WORD));
		Indices[0] = 0; Indices[1] = 1; Indices[2] = 2; 
		Indices[3] = 0; Indices[4] = 2; Indices[5] = 3; 
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};
/** Index buffer for the light function fullscreen 2D quad. */
TGlobalResource<FLightFunctionIndexBuffer> GLightFunctionIndexBuffer;

/**
* Used by RenderLights to figure out if light functions need to be rendered to the attenuation buffer.
*
* @param LightSceneInfo Represents the current light
* @return TRUE if anything got rendered
*/
UBOOL FSceneRenderer::CheckForLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	// NOTE: The extra check is necessary because there could be something wrong with the material.
	if( LightSceneInfo->LightFunction && 
		LightSceneInfo->LightFunction->GetMaterial()->IsLightFunction() )
	{
		for (INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);
			if(View.VisibleLightInfos(LightSceneInfo->Id).DPGInfo[DPGIndex].bHasVisibleLitPrimitives)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
 * Used by RenderLights to render a light function to the attenuation buffer.
 *
 * @param LightSceneInfo Represents the current light
 * @return TRUE if anything got rendered
 */
UBOOL FSceneRenderer::RenderLightFunction( const FLightSceneInfo* LightSceneInfo, UINT DPGIndex )
{
	SCOPE_CYCLE_COUNTER(STAT_LightFunctionDrawTime);

	UBOOL bAttenuationBufferDirty = FALSE;

	// NOTE: The extra check is necessary because there could be something wrong with the material.
	if( LightSceneInfo->LightFunction && 
		LightSceneInfo->LightFunction->GetMaterial()->IsLightFunction() )
	{
		SCOPED_DRAW_EVENT(EventLightFunction)(DEC_SCENE_ITEMS,TEXT("Light Function"));

		const FMaterialShaderMap* MaterialShaderMap = LightSceneInfo->LightFunction->GetMaterial()->GetShaderMap();
		FLightFunctionVertexShader* VertexShader = MaterialShaderMap->GetShader<FLightFunctionVertexShader>();
		FLightFunctionPixelShader* PixelShader = MaterialShaderMap->GetShader<FLightFunctionPixelShader>();

		// Set the states to modulate the light function with the render target.
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetColorWriteEnable(TRUE);
		RHISetBlendState(TStaticBlendState<BO_Add,BF_DestColor,BF_Zero>::GetRHI());
		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		RHISetStencilState(TStaticStencilState<TRUE,CF_Equal>::GetRHI());
		RHISetStreamSource(0, GLightFunctionVertexBuffer.VertexBufferRHI, sizeof(FVector2D),FALSE,0,1);

		if (!IsValidRef(LightSceneInfo->LightFunctionBoundShaderState))
		{
			DWORD Strides[MaxVertexElementCount];
			appMemzero(Strides, sizeof(Strides));
			Strides[0] = sizeof(FVector2D);

			LightSceneInfo->LightFunctionBoundShaderState = RHICreateBoundShaderState(GLightFunctionVertexDeclaration.VertexDeclarationRHI, Strides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());
		}

		// Render to the light attenuation buffer for all views.
		GSceneRenderTargets.BeginRenderingLightAttenuation();
		for (INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);
			
			const FViewInfo& View = Views(ViewIndex);
			if(View.VisibleLightInfos(LightSceneInfo->Id).DPGInfo[DPGIndex].bHasVisibleLitPrimitives)
			{
				// Set the device viewport for the view.
				RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
				RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

				// Set the light's scissor rectangle.
				LightSceneInfo->SetScissorRect(&View);
				LightSceneInfo->SetDepthBounds(&View);

				// Render a fullscreen quad.
				VertexShader->SetParameters(&View);
				PixelShader->SetParameters(&View, LightSceneInfo);
				RHISetBoundShaderState(LightSceneInfo->LightFunctionBoundShaderState);

				RHIDrawIndexedPrimitive(GLightFunctionIndexBuffer.IndexBufferRHI, PT_TriangleList, 0, 0, 4, 0, 2);

				// Mark the attenuation buffer as dirty.
				bAttenuationBufferDirty = TRUE;
			}
		}
		GSceneRenderTargets.FinishRenderingLightAttenuation();

		// Restore states.
		RHISetDepthState(TStaticDepthState<>::GetRHI());
		RHISetColorWriteEnable(TRUE);
		RHISetBlendState(TStaticBlendState<>::GetRHI());
		RHISetStencilState(TStaticStencilState<>::GetRHI());
		RHISetScissorRect(FALSE,0,0,0,0);
		RHISetDepthBoundsTest( FALSE, FVector4(0.0f,0.0f,0.0f,1.0f), FVector4(0.0f,0.0f,1.0f,1.0f));
	}
	return bAttenuationBufferDirty;
}
