/*=============================================================================
	LightMapRendering.h: Light map rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LIGHTMAPRENDERING_H__
#define __LIGHTMAPRENDERING_H__

/**
 */
class FNullLightMapShaderComponent
{
public:
	void Bind(const FShaderParameterMap& ParameterMap)
	{}
	void Serialize(FArchive& Ar)
	{}
};

/**
 * A policy for shaders without a light-map.
 */
class FNoLightMapPolicy
{
public:

	typedef FNullLightMapShaderComponent VertexParametersType;
	typedef FNullLightMapShaderComponent PixelParametersType;
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	// Translucent lit materials without a light-map should be drawn unlit.
	static const UBOOL bDrawLitTranslucencyUnlit = TRUE;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType,UBOOL bEnableSkyLight=FALSE)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_DIRECTIONAL_LIGHTMAP_COEF));
	}

	void Set(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set();
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl
	* @param StreamStrides - output array of vertex stream strides
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const ElementDataType& ElementData
		) const
	{}

	friend UBOOL operator==(const FNoLightMapPolicy A,const FNoLightMapPolicy B)
	{
		return TRUE;
	}

	friend INT Compare(const FNoLightMapPolicy&,const FNoLightMapPolicy&)
	{
		return 0;
	}

};

/**
 * Base policy for shaders with vertex lightmaps.
 */
class FVertexLightMapPolicy
{
public:

	typedef FLightMapInteraction ElementDataType;

	static const UBOOL bDrawLitTranslucencyUnlit = FALSE;

	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightMapScaleParameter.Bind(ParameterMap,TEXT("LightMapScale"));
		}
		void SetLightMapScale(FShader* VertexShader,const FLightMapInteraction& LightMap) const
		{
			SetVertexShaderValues(VertexShader->GetVertexShader(),LightMapScaleParameter,LightMap.GetScaleArray(),LightMap.GetNumLightmapCoefficients());
		}
		void Serialize(FArchive& Ar)
		{
			Ar << LightMapScaleParameter;
		}
	private:
		FShaderParameter LightMapScaleParameter;
	};

	typedef FNullLightMapShaderComponent PixelParametersType;

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return Material->GetLightingModel() != MLM_Unlit
			&& VertexFactoryType->SupportsStaticLighting()
			&& (Material->IsUsedWithStaticLighting() || Material->IsSpecialEngineMaterial())
			//terrain never uses vertex lightmaps
			&& !Material->IsTerrainMaterial();
	}

	void Set(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView* View
		) const
	{}
};

/**
 * Policy for directional vertex lightmaps, where lighting is stored along each of the lightmap bases.
 */
class FDirectionalVertexLightMapPolicy : public FVertexLightMapPolicy
{
public:

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl
	* @param StreamStrides - output array of vertex stream strides
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetVertexLightMapStreamStrides(StreamStrides,TRUE);
		VertexDeclaration = VertexFactory->GetVertexLightMapDeclaration(TRUE);
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FLightMapInteraction& LightMapInteraction
		) const
	{
		check(VertexFactory);
		VertexFactory->SetVertexLightMap(LightMapInteraction.GetVertexBuffer(),TRUE);
		VertexShaderParameters->SetLightMapScale(VertexShader,LightMapInteraction);
	}

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType,UBOOL bEnableSkyLight=FALSE)
	{
		// if material requires simple static lighting then don't cache full directional shader
		if( Material && Material->RequiresSimpleStaticLighting(GetMaterialPlatform(Platform)) )
		{
			return FALSE;
		}
		else
		{
			return FVertexLightMapPolicy::ShouldCache(Platform, Material, VertexFactoryType);
		}
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("VERTEX_LIGHTMAP"),TEXT("1"));
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_DIRECTIONAL_LIGHTMAP_COEF));
	}

	friend UBOOL operator==(const FDirectionalVertexLightMapPolicy A,const FDirectionalVertexLightMapPolicy B)
	{
		return TRUE;
	}

	friend INT Compare(const FDirectionalVertexLightMapPolicy&,const FDirectionalVertexLightMapPolicy&)
	{
		return 0;
	}
};

/**
 * Policy for simple vertex lightmaps, where lighting is only stored along the surface normal.
 */
class FSimpleVertexLightMapPolicy : public FVertexLightMapPolicy
{
public:

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl
	* @param StreamStrides - output array of vertex stream strides
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetVertexLightMapStreamStrides(StreamStrides,FALSE);
		VertexDeclaration = VertexFactory->GetVertexLightMapDeclaration(FALSE);
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FLightMapInteraction& LightMapInteraction
		) const
	{
		check(VertexFactory);
		VertexFactory->SetVertexLightMap(LightMapInteraction.GetVertexBuffer(),FALSE);
		VertexShaderParameters->SetLightMapScale(VertexShader,LightMapInteraction);
	}

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType,UBOOL bEnableSkyLight=FALSE)
	{
		return FVertexLightMapPolicy::ShouldCache(Platform, Material, VertexFactoryType)
			//only compile for PC
			&& (Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_SM3 || Platform == SP_PCD3D_SM2);
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("SIMPLE_VERTEX_LIGHTMAP"),TEXT("1"));
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_SIMPLE_LIGHTMAP_COEF));
	}

	friend UBOOL operator==(const FSimpleVertexLightMapPolicy A,const FSimpleVertexLightMapPolicy B)
	{
		return TRUE;
	}

	friend INT Compare(const FSimpleVertexLightMapPolicy&,const FSimpleVertexLightMapPolicy&)
	{
		return 0;
	}
};

/**
 * Base policy for shaders with lightmap textures.
 */
class FLightMapTexturePolicy
{
public:

	typedef FLightMapInteraction ElementDataType;

	static const UBOOL bDrawLitTranslucencyUnlit = FALSE;

	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightMapTexturesParameter.Bind(ParameterMap,TEXT("LightMapTextures"),TRUE);
			LightMapScaleParameter.Bind(ParameterMap,TEXT("LightMapScale"),TRUE);
		}
		void SetLightMapTextures(
			FShader* PixelShader,
			const UTexture2D* const * LightMapTextures,
			UINT NumLightmapTextures) const
		{
			for(UINT CoefficientIndex = 0;CoefficientIndex < NumLightmapTextures;CoefficientIndex++)
			{
				SetTextureParameter(
					PixelShader->GetPixelShader(),
					LightMapTexturesParameter,
					LightMapTextures[CoefficientIndex]->Resource,
					CoefficientIndex
					);
			}
		}
		void SetLightMapScale(FShader* PixelShader,const FLightMapInteraction& LightMapInteraction) const
		{
			SetPixelShaderValues(PixelShader->GetPixelShader(),LightMapScaleParameter,LightMapInteraction.GetScaleArray(),LightMapInteraction.GetNumLightmapCoefficients());
		}
		void Serialize(FArchive& Ar)
		{
			Ar << LightMapTexturesParameter;
			Ar << LightMapScaleParameter;
		}
	private:
		FShaderResourceParameter LightMapTexturesParameter;
		FShaderParameter LightMapScaleParameter;
	};

	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			ShadowCoordinateScaleBiasParameter.Bind(ParameterMap,TEXT("ShadowCoordinateScaleBias"),TRUE);
		}
		void SetCoordinateTransform(FShader* VertexShader,const FVector2D& ShadowCoordinateScale,const FVector2D& ShadowCoordinateBias) const
		{
			SetVertexShaderValue(VertexShader->GetVertexShader(),ShadowCoordinateScaleBiasParameter,FVector4(
				ShadowCoordinateScale.X,
				ShadowCoordinateScale.Y,
				ShadowCoordinateBias.Y,
				ShadowCoordinateBias.X
				));
		}
		void Serialize(FArchive& Ar)
		{
			Ar << ShadowCoordinateScaleBiasParameter;
		}
	private:
		FShaderParameter ShadowCoordinateScaleBiasParameter;
	};

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return Material->GetLightingModel() != MLM_Unlit && VertexFactoryType->SupportsStaticLighting() && (Material->IsUsedWithStaticLighting() || Material->IsSpecialEngineMaterial());
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl
	* @param StreamStrides - output array of vertex stream strides
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory)
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FLightMapInteraction& LightMapInteraction
		) const
	{
		VertexShaderParameters->SetCoordinateTransform(
			VertexShader,
			LightMapInteraction.GetCoordinateScale(),
			LightMapInteraction.GetCoordinateBias()
			);
		if(PixelShaderParameters)
		{
			PixelShaderParameters->SetLightMapScale(PixelShader,LightMapInteraction);
		}
	}
};

/**
 * Policy for directional texture lightmaps, where lighting is stored along each of the lightmap bases.
 */
class FDirectionalLightMapTexturePolicy : public FLightMapTexturePolicy
{
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType,UBOOL bEnableSkyLight=FALSE)
	{
		// if material requires simple static lighting then don't cache full directional shader
		if( Material && Material->RequiresSimpleStaticLighting(GetMaterialPlatform(Platform)) )
		{
			return FALSE;
		}
		else
		{
			return FLightMapTexturePolicy::ShouldCache(Platform,Material,VertexFactoryType);
		}
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("TEXTURE_LIGHTMAP"),TEXT("1"));
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_DIRECTIONAL_LIGHTMAP_COEF));
	}

	void Set(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set();
		if(PixelShaderParameters)
		{
			PixelShaderParameters->SetLightMapTextures(PixelShader,LightMapTextures,NUM_DIRECTIONAL_LIGHTMAP_COEF);
		}
	}

	/** Initialization constructor. */
	FDirectionalLightMapTexturePolicy(const FLightMapInteraction& LightMapInteraction)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
		{
			LightMapTextures[CoefficientIndex] = LightMapInteraction.GetTexture(CoefficientIndex);
		}
	}

	friend UBOOL operator==(const FDirectionalLightMapTexturePolicy A,const FDirectionalLightMapTexturePolicy B)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_DIRECTIONAL_LIGHTMAP_COEF;CoefficientIndex++)
		{
			if(A.LightMapTextures[CoefficientIndex] != B.LightMapTextures[CoefficientIndex])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	friend INT Compare(const FDirectionalLightMapTexturePolicy& A,const FDirectionalLightMapTexturePolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[0]);
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[1]);
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[2]);
		return 0;
	}

private:
	const UTexture2D* LightMapTextures[NUM_DIRECTIONAL_LIGHTMAP_COEF];
};

/**
 * Policy for simple texture lightmaps, where lighting is only stored along the surface normal.
 */
class FSimpleLightMapTexturePolicy : public FLightMapTexturePolicy
{
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType,UBOOL bEnableSkyLight=FALSE)
	{
		return FLightMapTexturePolicy::ShouldCache(Platform,Material,VertexFactoryType)
			//only compile for PC
			&& (Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_SM3 || Platform == SP_PCD3D_SM2);
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("SIMPLE_TEXTURE_LIGHTMAP"),TEXT("1"));
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_SIMPLE_LIGHTMAP_COEF));
	}

	/** Initialization constructor. */
	FSimpleLightMapTexturePolicy(const FLightMapInteraction& LightMapInteraction)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_SIMPLE_LIGHTMAP_COEF;CoefficientIndex++)
		{
			LightMapTextures[CoefficientIndex] = LightMapInteraction.GetTexture(CoefficientIndex);
		}
	}

	void Set(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set();
		if (PixelShaderParameters)
		{
			PixelShaderParameters->SetLightMapTextures(PixelShader,LightMapTextures,NUM_SIMPLE_LIGHTMAP_COEF);
		}
	}

	friend UBOOL operator==(const FSimpleLightMapTexturePolicy A,const FSimpleLightMapTexturePolicy B)
	{
		for(UINT CoefficientIndex = 0;CoefficientIndex < NUM_SIMPLE_LIGHTMAP_COEF;CoefficientIndex++)
		{
			if(A.LightMapTextures[CoefficientIndex] != B.LightMapTextures[CoefficientIndex])
			{
				return FALSE;
			}
		}
		return TRUE;
	}

	friend INT Compare(const FSimpleLightMapTexturePolicy& A,const FSimpleLightMapTexturePolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(LightMapTextures[0]);
		return 0;
	}

private:
	const UTexture2D* LightMapTextures[NUM_SIMPLE_LIGHTMAP_COEF];
};

// A light map policy for rendering a directional light in the base pass.
class FDirectionalLightLightMapPolicy
{
public:
	struct VertexParametersType
	{
		FShaderParameter LightDirectionParameter;

		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightDirectionParameter.Bind(ParameterMap, TEXT("LightDirection"), TRUE);
		}

		void Serialize(FArchive& Ar)
		{
			Ar << LightDirectionParameter;
		}
	};

	struct PixelParametersType
	{
		FShaderParameter LightColorParameter;

		void Bind(const FShaderParameterMap& ParameterMap)
		{
			LightColorParameter.Bind(ParameterMap, TEXT("LightColor"), TRUE);
		}

		void Serialize(FArchive& Ar)
		{
			Ar << LightColorParameter;
		}
	};

	typedef const FPrimitiveSceneInfo* ElementDataType;

	// Translucent lit materials with a directional light should not be drawn unlit.
	static const UBOOL bDrawLitTranslucencyUnlit = FALSE;

	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType, UBOOL bEnableSkyLight=FALSE)
	{
		return !IsTranslucentBlendMode(Material->GetBlendMode()) 
			&& Material->GetLightingModel() != MLM_Unlit
			// Don't compile for sm2 as there are not enough ALU instructions in ps 2.0 to combine lights in the base pass
			&& Platform != SP_PCD3D_SM2;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("NUM_LIGHTMAP_COEFFICIENTS"),*FString::Printf(TEXT("%u"),NUM_DIRECTIONAL_LIGHTMAP_COEF));
		OutEnvironment.Definitions.Set(TEXT("ENABLE_DIRECTIONAL_LIGHT"), TEXT("1"));
	}

	void Set(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FSceneView* View
		) const
	{
		check(VertexFactory);
		VertexFactory->Set();
	}

	/**
	* Get the decl and stream strides for this policy type and vertexfactory
	* @param VertexDeclaration - output decl
	* @param StreamStrides - output array of vertex stream strides
	* @param VertexFactory - factory to be used by this policy
	*/
	void GetVertexDeclarationInfo(FVertexDeclarationRHIParamRef &VertexDeclaration, DWORD *StreamStrides, const FVertexFactory* VertexFactory) const	// RAVEN made function const
	{
		check(VertexFactory);
		VertexFactory->GetStreamStrides(StreamStrides);
		VertexDeclaration = VertexFactory->GetDeclaration();
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const ElementDataType& PrimitiveSceneInfo
		) const
	{
		const FLightSceneInfo* const Light = PrimitiveSceneInfo->DirectionalLightSceneInfo;
		if (Light)
		{
			SetVertexShaderValue(VertexShader->GetVertexShader(), VertexShaderParameters->LightDirectionParameter, -(Light->GetDirection()));

			if (PixelShaderParameters)
			{
				SetPixelShaderValue(PixelShader->GetPixelShader(), PixelShaderParameters->LightColorParameter, Light->Color);
			}
		}
	}

	friend UBOOL operator==(const FDirectionalLightLightMapPolicy& A,const FDirectionalLightLightMapPolicy& B)
	{
		return TRUE;
	}

	friend INT Compare(const FDirectionalLightLightMapPolicy& A,const FDirectionalLightLightMapPolicy& B)
	{
		return 0;
	}
};

// A light map policy for rendering an SH + point light in the base pass.
class FSHLightLightMapPolicy : public FDirectionalLightLightMapPolicy
{
	typedef FDirectionalLightLightMapPolicy Super;
public:

	struct PixelParametersType : public Super::PixelParametersType
	{
		FShaderParameter WorldIncidentLightingParameter;
		FShaderResourceParameter SHBasisCubeTexturesParameter;

		void Bind(const FShaderParameterMap& ParameterMap)
		{
			Super::PixelParametersType::Bind(ParameterMap);
			WorldIncidentLightingParameter.Bind(ParameterMap,TEXT("WorldIncidentLighting"),TRUE);
			SHBasisCubeTexturesParameter.Bind(ParameterMap,TEXT("SHBasisCubeTextures"),TRUE);
		}

		void Serialize(FArchive& Ar)
		{
			Super::PixelParametersType::Serialize(Ar);
			Ar << WorldIncidentLightingParameter;
			Ar << SHBasisCubeTexturesParameter;
		}
	};

	static UBOOL ShouldCache(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType, UBOOL bEnableSkyLight=FALSE)
	{
		return Material->SupportsSinglePassSHLight() && Super::ShouldCache(Platform, Material, VertexFactoryType, bEnableSkyLight);
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		Super::ModifyCompilationEnvironment(Platform, OutEnvironment);
		OutEnvironment.Definitions.Set(TEXT("ENABLE_SH_LIGHT"), TEXT("1"));
	}

	void SetMesh(
		const VertexParametersType* VertexShaderParameters,
		const PixelParametersType* PixelShaderParameters,
		FShader* VertexShader,
		FShader* PixelShader,
		const FVertexFactory* VertexFactory,
		const FMaterialRenderProxy* MaterialRenderProxy,
		const ElementDataType& PrimitiveSceneInfo
		) const;
};


#endif // __LIGHTMAPRENDERING_H__
