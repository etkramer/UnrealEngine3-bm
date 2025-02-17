/*=============================================================================
	FogRendering.cpp: Fog rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "AmbientOcclusionRendering.h"

/** Binds the parameters. */
void FHeightFogVertexShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	FogMinHeightParameter.Bind(ParameterMap,TEXT("FogMinHeight"), TRUE);
	FogMaxHeightParameter.Bind(ParameterMap,TEXT("FogMaxHeight"), TRUE);
	FogDistanceScaleParameter.Bind(ParameterMap,TEXT("FogDistanceScale"), TRUE);
	FogExtinctionDistanceParameter.Bind(ParameterMap,TEXT("FogExtinctionDistance"), TRUE);
	FogInScatteringParameter.Bind(ParameterMap,TEXT("FogInScattering"), TRUE);
	FogStartDistanceParameter.Bind(ParameterMap,TEXT("FogStartDistance"), TRUE);
}

/** 
* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
* @param VertexShader - the vertex shader to set the parameters on
*/
void FHeightFogVertexShaderParameters::Set(const FMaterialRenderProxy* MaterialRenderProxy, const FSceneView* View, FShader* VertexShader) const
{
	FViewInfo* ViewInfo = (FViewInfo*)View;

	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	// Set the fog constants.
	//@todo - translucent decals on translucent receivers currently don't handle fog
	if ( !Material->IsDecalMaterial() && Material->AllowsFog() )
	{
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogMinHeightParameter,ViewInfo->FogMinHeight);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogMaxHeightParameter,ViewInfo->FogMaxHeight);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogInScatteringParameter,ViewInfo->FogInScattering);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogDistanceScaleParameter,ViewInfo->FogDistanceScale);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogExtinctionDistanceParameter,ViewInfo->FogExtinctionDistance);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogStartDistanceParameter,ViewInfo->FogStartDistance);
	}
	else
	{
		// Decals get the default ('fog-less') values.
		static const FLOAT DefaultFogMinHeight[4] = { 0.f, 0.f, 0.f, 0.f };
		static const FLOAT DefaultFogMaxHeight[4] = { 0.f, 0.f, 0.f, 0.f };
		static const FLOAT DefaultFogDistanceScale[4] = { 0.f, 0.f, 0.f, 0.f };
		static const FLOAT DefaultFogStartDistance[4] = { 0.f, 0.f, 0.f, 0.f };
		static const FLOAT DefaultFogExtinctionDistance[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
		static const FLinearColor DefaultFogInScattering[4] = { FLinearColor::Black, FLinearColor::Black, FLinearColor::Black, FLinearColor::Black };

		SetVertexShaderValue(VertexShader->GetVertexShader(),FogMinHeightParameter,DefaultFogMinHeight);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogMaxHeightParameter,DefaultFogMaxHeight);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogInScatteringParameter,DefaultFogInScattering);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogDistanceScaleParameter,DefaultFogDistanceScale);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogExtinctionDistanceParameter,DefaultFogExtinctionDistance);
		SetVertexShaderValue(VertexShader->GetVertexShader(),FogStartDistanceParameter,DefaultFogStartDistance);
	}
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FHeightFogVertexShaderParameters& Parameters)
{
	Ar << Parameters.FogDistanceScaleParameter;
	Ar << Parameters.FogExtinctionDistanceParameter;
	Ar << Parameters.FogMinHeightParameter;
	Ar << Parameters.FogMaxHeightParameter;
	Ar << Parameters.FogInScatteringParameter;
	Ar << Parameters.FogStartDistanceParameter;
	return Ar;
}


/** A vertex shader for rendering height fog. */
template<UINT NumLayers>
class THeightFogVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(THeightFogVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	THeightFogVertexShader( )	{ }
	THeightFogVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"));
		FogMinHeightParameter.Bind(Initializer.ParameterMap,TEXT("FogMinHeight"));
		FogMaxHeightParameter.Bind(Initializer.ParameterMap,TEXT("FogMaxHeight"));
		ScreenToWorldParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"));
	}

	void SetParameters(const FViewInfo& View)
	{
		// Set the transform from screen coordinates to scene texture coordinates.
		// NOTE: Need to set explicitly, since this is a vertex shader!
		SetVertexShaderValue(GetVertexShader(),ScreenPositionScaleBiasParameter,View.ScreenPositionScaleBias);

		// Set the fog constants.
		SetVertexShaderValue(GetVertexShader(),FogMinHeightParameter,View.FogMinHeight);
		SetVertexShaderValue(GetVertexShader(),FogMaxHeightParameter,View.FogMaxHeight);

		FMatrix ScreenToWorld = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,(1.0f - Z_PRECISION),1),
			FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
			) *
			View.InvTranslatedViewProjectionMatrix;

		// Set the view constants, as many as were bound to the parameter.
		SetVertexShaderValue(GetVertexShader(),ScreenToWorldParameter,ScreenToWorld);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ScreenPositionScaleBiasParameter;
		Ar << FogMinHeightParameter;
		Ar << FogMaxHeightParameter;
		Ar << ScreenToWorldParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter ScreenPositionScaleBiasParameter;
	FShaderParameter FogMinHeightParameter;
	FShaderParameter FogMaxHeightParameter;
	FShaderParameter ScreenToWorldParameter;
};

IMPLEMENT_SHADER_TYPE(template<>,THeightFogVertexShader<1>,TEXT("HeightFogVertexShader"),TEXT("OneLayerMain"),SF_Vertex,0,0);
IMPLEMENT_SHADER_TYPE(template<>,THeightFogVertexShader<4>,TEXT("HeightFogVertexShader"),TEXT("FourLayerMain"),SF_Vertex,0,0);


/** A vertex shader for rendering height fog. */
template<UINT NumLayers>
class THeightFogPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(THeightFogPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		// Only compile the downsampled version (NumLayers == 0) for xbox
		return NumLayers != 0 || Platform == SP_XBOXD3D;
	}

	/**
	* Add any compiler flags/defines required by the shader
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		//The HLSL compiler for xenon will not always use predicated instructions without this flag.  
		//On PC the compiler consistently makes the right decision.
		new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_PreferFlowControl);
		if( Platform == SP_XBOXD3D )
		{
			//The xenon compiler complains about the [ifAny] attribute
			new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_SkipValidation);
		}
	}

	THeightFogPixelShader( )	{ }
	THeightFogPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		FogDistanceScaleParameter.Bind(Initializer.ParameterMap,TEXT("FogDistanceScale"));
		FogExtinctionDistanceParameter.Bind(Initializer.ParameterMap,TEXT("FogExtinctionDistance"));
		FogInScatteringParameter.Bind(Initializer.ParameterMap,TEXT("FogInScattering"), TRUE);
		FogStartDistanceParameter.Bind(Initializer.ParameterMap,TEXT("FogStartDistance"));
		FogMinStartDistanceParameter.Bind(Initializer.ParameterMap,TEXT("FogMinStartDistance"), TRUE);
		EncodePowerParameter.Bind(Initializer.ParameterMap,TEXT("EncodePower"), TRUE);
	}

	void SetParameters(const FViewInfo& View, INT NumSceneFogLayers, UBOOL bIncreaseNearPrecision)
	{
		check(NumSceneFogLayers > 0);
		SceneTextureParameters.Set( &View, this);

		// Set the fog constants.
		SetPixelShaderValue(GetPixelShader(),FogInScatteringParameter,View.FogInScattering);
		SetPixelShaderValue(GetPixelShader(),FogDistanceScaleParameter,View.FogDistanceScale);
		SetPixelShaderValue(GetPixelShader(),FogExtinctionDistanceParameter,View.FogExtinctionDistance);
		SetPixelShaderValue(GetPixelShader(),FogStartDistanceParameter,View.FogStartDistance);
		SetPixelShaderValue(GetPixelShader(),FogMinStartDistanceParameter,*MinElement(View.FogStartDistance, View.FogStartDistance+NumSceneFogLayers));
		SetPixelShaderValue(GetPixelShader(),EncodePowerParameter, bIncreaseNearPrecision ? 8.0f : 1.0f);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << FogDistanceScaleParameter;
		Ar << FogExtinctionDistanceParameter;
		Ar << FogInScatteringParameter;
		Ar << FogStartDistanceParameter;
		Ar << FogMinStartDistanceParameter;
		Ar << EncodePowerParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderParameter FogDistanceScaleParameter;
	FShaderParameter FogExtinctionDistanceParameter;
	FShaderParameter FogInScatteringParameter;
	FShaderParameter FogStartDistanceParameter;
	FShaderParameter FogMinStartDistanceParameter;
	FShaderParameter EncodePowerParameter;
};

IMPLEMENT_SHADER_TYPE(template<>,THeightFogPixelShader<1>,TEXT("HeightFogPixelShader"),TEXT("OneLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);
IMPLEMENT_SHADER_TYPE(template<>,THeightFogPixelShader<0>,TEXT("HeightFogPixelShader"),TEXT("DownsampleDepthAndFogMain"),SF_Pixel,0,0);
IMPLEMENT_SHADER_TYPE(template<>,THeightFogPixelShader<4>,TEXT("HeightFogPixelShader"),TEXT("FourLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

/** The fog vertex declaration resource type. */
class FFogVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor
	virtual ~FFogVertexDeclaration() {}

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
TGlobalResource<FFogVertexDeclaration> GFogVertexDeclaration;

void FSceneRenderer::InitFogConstants()
{
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);

		// set fog consts based on height fog components
		if(View.Family->ShowFlags & SHOW_Fog)
		{
			// Remap the fog layers into back to front order.
			INT FogLayerMap[4];
			INT NumFogLayers = 0;
			for(INT AscendingFogIndex = Min(Scene->Fogs.Num(),4) - 1;AscendingFogIndex >= 0;AscendingFogIndex--)
			{
				const FHeightFogSceneInfo& FogSceneInfo = Scene->Fogs(AscendingFogIndex);
				if(FogSceneInfo.Height > View.ViewOrigin.Z)
				{
					for(INT DescendingFogIndex = 0;DescendingFogIndex <= AscendingFogIndex;DescendingFogIndex++)
					{
						FogLayerMap[NumFogLayers++] = DescendingFogIndex;
					}
					break;
				}
				FogLayerMap[NumFogLayers++] = AscendingFogIndex;
			}

			// Calculate the fog constants.
			for(INT LayerIndex = 0;LayerIndex < NumFogLayers;LayerIndex++)
			{
				// remapped fog layers in ascending order
				const FHeightFogSceneInfo& FogSceneInfo = Scene->Fogs(FogLayerMap[LayerIndex]);
				// log2(1-density)
				View.FogDistanceScale[LayerIndex] = appLoge(1.0f - FogSceneInfo.Density) / appLoge(2.0f);
				if(FogLayerMap[LayerIndex] + 1 < NumFogLayers)
				{
					// each min height is adjusted to aligned with the max height of the layer above
					View.FogMinHeight[LayerIndex] = Scene->Fogs(FogLayerMap[LayerIndex] + 1).Height + View.PreViewTranslation.Z;
				}
				else
				{
					// lowest layer extends down
					View.FogMinHeight[LayerIndex] = -HALF_WORLD_MAX;
				}
				// max height is set by the actor's height
				View.FogMaxHeight[LayerIndex] = FogSceneInfo.Height + View.PreViewTranslation.Z;
				// This formula is incorrect, but must be used to support legacy content.  The in-scattering color should be computed like this:
				// FogInScattering[LayerIndex] = FLinearColor(FogComponent->LightColor) * (FogComponent->LightBrightness / (appLoge(2.0f) * FogDistanceScale[LayerIndex]));
				View.FogInScattering[LayerIndex] = FogSceneInfo.LightColor / appLoge(0.5f);
				// anything beyond the extinction distance goes to full fog
				View.FogExtinctionDistance[LayerIndex] = FogSceneInfo.ExtinctionDistance;
				// start distance where fog affects the scene
				View.FogStartDistance[LayerIndex] = Max( 0.f, FogSceneInfo.StartDistance );			
			}
		}
	}
}

FGlobalBoundShaderState OneLayerFogBoundShaderState;
FGlobalBoundShaderState DownsampleDepthAndFogBoundShaderState;
FGlobalBoundShaderState FourLayerFogBoundShaderState;

UBOOL FSceneRenderer::RenderFog(UINT DPGIndex)
{
	const INT NumSceneFogLayers = Scene->Fogs.Num();
	if (DPGIndex == SDPG_World && NumSceneFogLayers > 0)
	{
		SCOPED_DRAW_EVENT(EventFog)(DEC_SCENE_ITEMS,TEXT("Fog"));

		static const FVector2D Vertices[4] =
		{
			FVector2D(-1,-1),
			FVector2D(-1,+1),
			FVector2D(+1,+1),
			FVector2D(+1,-1),
		};
		static const WORD Indices[6] =
		{
			0, 1, 2,
			0, 2, 3
		};

		GSceneRenderTargets.BeginRenderingSceneColor();
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);
			if (View.bOneLayerHeightFogRenderedInAO)
			{
				// Skip one layer height fog for this view since it was already rendered with ambient occlusion
				continue;
			}

			// Set the device viewport for the view.
			RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

			// No depth tests, no backface culling.
			RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

			RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_SourceAlpha>::GetRHI());

			//use the optimized one layer version if there is only one height fog layer
			if (NumSceneFogLayers == 1)
			{
				TShaderMapRef<THeightFogVertexShader<1> > VertexShader(GetGlobalShaderMap());
				TShaderMapRef<THeightFogPixelShader<1> > OneLayerHeightFogPixelShader(GetGlobalShaderMap());

				SetGlobalBoundShaderState(OneLayerFogBoundShaderState, GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *OneLayerHeightFogPixelShader, sizeof(FVector2D));
				VertexShader->SetParameters(View);
				OneLayerHeightFogPixelShader->SetParameters(View, NumSceneFogLayers, FALSE);
			}
			//otherwise use the four layer version
			else
			{
				TShaderMapRef<THeightFogVertexShader<4> > VertexShader(GetGlobalShaderMap());
				TShaderMapRef<THeightFogPixelShader<4> > FourLayerHeightFogPixelShader(GetGlobalShaderMap());

				SetGlobalBoundShaderState(FourLayerFogBoundShaderState, GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *FourLayerHeightFogPixelShader, sizeof(FVector2D));
				VertexShader->SetParameters(View);
				FourLayerHeightFogPixelShader->SetParameters(View, NumSceneFogLayers, FALSE);
			}

			// disable alpha writes in order to preserve scene depth values on PC
			RHISetColorWriteMask(CW_RED|CW_GREEN|CW_BLUE);

			// Draw a quad covering the view.
			RHIDrawIndexedPrimitiveUP(
				PT_TriangleList,
				0,
				ARRAY_COUNT(Vertices),
				2,
				Indices,
				sizeof(Indices[0]),
				Vertices,
				sizeof(Vertices[0])
				);

			// restore color write mask
			RHISetColorWriteMask(CW_RED|CW_GREEN|CW_BLUE|CW_ALPHA);
		}

		//no need to resolve since we used alpha blending
		GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
		return TRUE;
	}

	return FALSE;
}

/** 
 * Renders fog scattering values and max depths into 1/4 size RT0 and RT1. 
 * Called from AmbientOcclusion's DownsampleDepth()
 *
 * @param Scene - current scene
 * @param View - current view
 * @param DPGIndex - depth priority group
 * @param DownsampleDimensions - dimensions for downsampling
 */
UBOOL RenderQuarterDownsampledDepthAndFog(const FScene* Scene, const FViewInfo& View, UINT DPGIndex, const FDownsampleDimensions& DownsampleDimensions)
{
#if XBOX
	const INT NumSceneFogLayers = Scene->Fogs.Num();
	// Currently only works with one layer height fog
	if (DPGIndex == SDPG_World && NumSceneFogLayers == 1)
	{
		// Verify that the ambient occlusion downsample factor matches the fog downsample factor
		checkSlow(DownsampleDimensions.Factor == 2);

		SCOPED_DRAW_EVENT(EventFog)(DEC_SCENE_ITEMS,TEXT("DepthAndFogDownsample"));

		static const FVector2D Vertices[4] =
		{
			FVector2D(-1,-1),
			FVector2D(-1,+1),
			FVector2D(+1,+1),
			FVector2D(+1,-1),
		};
		static const WORD Indices[6] =
		{
			0, 1, 2,
			0, 2, 3
		};

		GSceneRenderTargets.BeginRenderingFogBuffer();

		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		// Disable depth test and writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		// Set the viewport to the current view in the occlusion buffer.
		RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

		TShaderMapRef<THeightFogVertexShader<1> > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<THeightFogPixelShader<0> > DownsampleDepthAndFogPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(DownsampleDepthAndFogBoundShaderState, GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *DownsampleDepthAndFogPixelShader, sizeof(FVector2D));
		VertexShader->SetParameters(View);
		DownsampleDepthAndFogPixelShader->SetParameters(View, NumSceneFogLayers, Scene->Fogs(0).bIncreaseNearPrecision);

		// Draw a quad covering the view.
		RHIDrawIndexedPrimitiveUP(
			PT_TriangleList,
			0,
			ARRAY_COUNT(Vertices),
			2,
			Indices,
			sizeof(Indices[0]),
			Vertices,
			sizeof(Vertices[0])
			);

		GSceneRenderTargets.FinishRenderingFogBuffer(
			FResolveParams(
				DownsampleDimensions.TargetX,
				DownsampleDimensions.TargetY, 
				DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
				DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
				)
			);

		return TRUE;
	}
#endif
	return FALSE;
}
