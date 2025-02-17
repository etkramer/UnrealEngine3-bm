/*=============================================================================
	SphericalHarmonicLightComponent.cpp: SphericalHarmonicLightComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "LightRendering.h"

IMPLEMENT_CLASS(USphericalHarmonicLightComponent);

/** A set of cubemap textures containing the spherical harmonic basis function values. */
class FSHBasisCubeTextures : public FRenderResource
{
public:

	TArray<FTextureCubeRHIRef> Textures;

	// FRenderResource interface.
	virtual void InitRHI()
	{
		// A lookup table mapping face to the 3D coordinates of the face on a 2x2x2 cube at the origin.
		static const FVector FaceBases[6][3] =
		{
			{ FVector(+1,+1,+1), FVector(+0,+0,-2), FVector(+0,-2,+0) },
			{ FVector(-1,+1,-1), FVector(+0,+0,+2), FVector(+0,-2,+0) },
			{ FVector(-1,+1,-1), FVector(+2,+0,+0), FVector(+0,+0,+2) },
			{ FVector(-1,-1,+1), FVector(+2,+0,+0), FVector(+0,+0,-2) },
			{ FVector(-1,+1,+1), FVector(+2,+0,+0), FVector(+0,-2,+0) },
			{ FVector(+1,+1,-1), FVector(-2,+0,+0), FVector(+0,-2,+0) }
		};

		// The parameters used to create the textures.
		static const UINT Size = 16;
		static const UINT NumMips = appCeilLogTwo(Size);
		static const FLOAT Scale = 255.0f / 2.0f;
		static const FLOAT Bias = 1.0f;

		for(INT BasisIndex = 1;BasisIndex < MAX_SH_BASIS;BasisIndex += 4)
		{
			// Create the texture and add it to the array.
			FTextureCubeRHIRef TextureRHI = RHICreateTextureCube(Size,PF_A8R8G8B8,NumMips,TexCreate_Uncooked,NULL);
			Textures.AddItem(TextureRHI);

			for(UINT FaceIndex = 0;FaceIndex < 6;FaceIndex++)
			{
				for(UINT MipIndex = 0;MipIndex < NumMips;MipIndex++)
				{
					const UINT MipWidth = Max<UINT>(Size >> MipIndex,1);
					const UINT MipHeight = Max<UINT>(Size >> MipIndex,1);
					const FVector FaceTexelX = FaceBases[FaceIndex][1] / (MipWidth - 1);
					const FVector FaceTexelY = FaceBases[FaceIndex][2] / (MipHeight - 1);

					// Lock the cube-map face mip-map.
					UINT StrideY = 0;
					void* Data = RHILockTextureCubeFace(TextureRHI,FaceIndex,MipIndex,TRUE,StrideY,FALSE);

					for(UINT Y = 0;Y < MipHeight;Y++)
					{
						FColor*	DestColor = (FColor*) (((BYTE*)Data) + Y * StrideY);

						for(UINT X = 0;X < MipWidth;X++)
						{
							// Compute the SH basis value for the direction this texel represents.
							const FVector FaceTexelPosition = FaceBases[FaceIndex][0] + FaceTexelX * X + FaceTexelY * Y;
							const FSHVector SH = SHBasisFunction(FaceTexelPosition.SafeNormal());

							// Quantize and write the SH basis values to the texture data.
							*DestColor++ = FColor(
								(BasisIndex + 0) >= MAX_SH_BASIS ? 128 : Clamp<INT>(appTrunc((SH.V[BasisIndex + 0] + Bias) * Scale),0,255),
								(BasisIndex + 1) >= MAX_SH_BASIS ? 128 : Clamp<INT>(appTrunc((SH.V[BasisIndex + 1] + Bias) * Scale),0,255),
								(BasisIndex + 2) >= MAX_SH_BASIS ? 128 : Clamp<INT>(appTrunc((SH.V[BasisIndex + 2] + Bias) * Scale),0,255),
								(BasisIndex + 3) >= MAX_SH_BASIS ? 128 : Clamp<INT>(appTrunc((SH.V[BasisIndex + 3] + Bias) * Scale),0,255)
								);
						}
					}

					// Unlock the cube-map face mip-map.
					RHIUnlockTextureCubeFace(TextureRHI,FaceIndex,MipIndex,FALSE);
				}
			}
		}
	}
	virtual void ReleaseRHI()
	{
		Textures.Empty();
	}
};

/** The global set of SH basis cubemaps. */
static TGlobalResource<FSHBasisCubeTextures> GSHBasisCubeTextures;

/**
 * The SH light policy for TMeshLightingDrawingPolicy.
 */
class FSphericalHarmonicLightPolicy
{
public:
	typedef class FSphericalHarmonicLightSceneInfo SceneInfoType;
	class VertexParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
		}
		void SetLight(FShader* VertexShader,const SceneInfoType* Light,const FSceneView* View) const
		{
		}
		void Serialize(FArchive& Ar)
		{
		}
	};
	class PixelParametersType
	{
	public:
		void Bind(const FShaderParameterMap& ParameterMap)
		{
			WorldIncidentLightingParameter.Bind(ParameterMap,TEXT("WorldIncidentLighting"),TRUE);
			SHBasisCubeTexturesParameter.Bind(ParameterMap,TEXT("SHBasisCubeTextures"),TRUE);
		}
		void SetLight(FShader* PixelShader,const SceneInfoType* Light,const FSceneView* View) const;
		void Serialize(FArchive& Ar)
		{
			Ar << WorldIncidentLightingParameter;
			Ar << SHBasisCubeTexturesParameter;
		}

	private:
		FShaderParameter WorldIncidentLightingParameter;
		FShaderResourceParameter SHBasisCubeTexturesParameter;
	};

	static UBOOL ShouldCacheStaticLightingShaders()
	{
		return FALSE;
	}
};

IMPLEMENT_SHADOWLESS_LIGHT_SHADER_TYPE(FSphericalHarmonicLightPolicy,TEXT("SphericalHarmonicLightVertexShader"),TEXT("SphericalHarmonicLightPixelShader"),0,0)

/**
 * The scene info for a directional light.
 */
class FSphericalHarmonicLightSceneInfo : public FLightSceneInfo
{
public:

	/** Colored SH coefficients for the light intensity, parameterized by the world-space incident angle. */
	FSHVectorRGB WorldSpaceIncidentLighting;

	/** Initialization constructor. */
	FSphericalHarmonicLightSceneInfo(const USphericalHarmonicLightComponent* Component)
	:	FLightSceneInfo(Component)
	,	WorldSpaceIncidentLighting(Component->WorldSpaceIncidentLighting)
	,	bRenderBeforeModShadows(Component->bRenderBeforeModShadows)
	{}

	virtual void DetachPrimitive(const FLightPrimitiveInteraction& Interaction) 
	{
		Interaction.GetPrimitiveSceneInfo()->SHLightSceneInfo = NULL;
	}

	virtual void AttachPrimitive(const FLightPrimitiveInteraction& Interaction)
	{
		if (LightEnvironment && LightEnvironment == Interaction.GetPrimitiveSceneInfo()->LightEnvironment)
		{
			// Set SHLightSceneInfo whether the light is allowed to be merged into the base pass or not
			// This allows later code to override the behavior
			Interaction.GetPrimitiveSceneInfo()->SHLightSceneInfo = this;
			// If the SH light is allowed to render before modulated shadows, merge it into the base pass
			Interaction.GetPrimitiveSceneInfo()->bRenderSHLightInBasePass = bRenderBeforeModShadows;
			// Update the primitive's static meshes, to ensure they use the right version of the base pass shaders.
			Interaction.GetPrimitiveSceneInfo()->BeginDeferredUpdateStaticMeshes();
		}
	}

	// FLightSceneInfo interface.
	virtual UBOOL GetProjectedShadowInitializer(const FBoxSphereBounds& SubjectBounds,FProjectedShadowInitializer& OutInitializer) const
	{
		return FALSE;
	}
	virtual const FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex) const
	{
		check(DPGIndex < SDPG_MAX_SceneRender);
		return &DPGInfos[DPGIndex];
	}
	virtual FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex)
	{
		check(DPGIndex < SDPG_MAX_SceneRender);
		return &DPGInfos[DPGIndex];
	}

	virtual class FShadowProjectionPixelShaderInterface* GetModShadowProjPixelShader() const
	{
		return NULL;
	}
	virtual class FBranchingPCFProjectionPixelShaderInterface* GetBranchingPCFModProjPixelShader() const
	{
		return NULL;
	} 
	virtual class FModShadowVolumePixelShader* GetModShadowVolumeShader() const
	{
		return NULL;
	}
	virtual FGlobalBoundShaderState* GetModShadowProjBoundShaderState() const
	{
		return NULL;
	}
	virtual FGlobalBoundShaderState* GetBranchingPCFModProjBoundShaderState() const
	{
		return NULL;
	}
	virtual FBoundShaderStateRHIParamRef GetModShadowVolumeBoundShaderState() const
	{
		static FBoundShaderStateRHIRef NullBoundShaderState;
		return NullBoundShaderState;
	}
#if SUPPORTS_VSM
	virtual class FVSMModProjectionPixelShader* GetVSMModProjPixelShader() const
	{
		return NULL;
	}
	virtual FBoundShaderStateRHIParamRef GetVSMModProjBoundShaderState() const
	{
		static FBoundShaderStateRHIRef NullBoundShaderState;
		return NullBoundShaderState;
	}
#endif

private:

	/** The DPG info for the SH light. */
	TLightSceneDPGInfo<FSphericalHarmonicLightPolicy> DPGInfos[SDPG_MAX_SceneRender];

	/**
	 * If TRUE, the SH light can be combined into the base pass as an optimization.  
	 * If FALSE, the SH light will be rendered after modulated shadows.
	 */
	const BITFIELD bRenderBeforeModShadows : 1;
};

static void SetSHPixelParameters(
	FShader* PixelShader, 
	const FSphericalHarmonicLightSceneInfo* Light, 
	const FShaderParameter& WorldIncidentLightingParameter, 
	const FShaderResourceParameter& SHBasisCubeTexturesParameter)
{
	// Pack the incident lighting SH coefficients into an array in the format expected by the shader:
	// The first float4 contains the RGB constant term.
	// The next NUM_SH_VECTORS float4s contain the coefficients for the red component of the incident lighting.
	// The next NUM_SH_VECTORS float4s contain the coefficients for the green component of the incident lighting.
	// The next NUM_SH_VECTORS float4s contain the coefficients for the blue component of the incident lighting.
	enum { NUM_SH_VECTORS = (MAX_SH_BASIS - 1 + 3) / 4 };
	FLOAT PackedWorldSpaceIncidentLightingSH[4 + NUM_SH_VECTORS * 4 * 3];
	PackedWorldSpaceIncidentLightingSH[0] = Light->WorldSpaceIncidentLighting.R.V[0];
	PackedWorldSpaceIncidentLightingSH[1] = Light->WorldSpaceIncidentLighting.G.V[0];
	PackedWorldSpaceIncidentLightingSH[2] = Light->WorldSpaceIncidentLighting.B.V[0];
	PackedWorldSpaceIncidentLightingSH[3] = 0.0f;
	for(INT BasisIndex = 1;BasisIndex < MAX_SH_BASIS;BasisIndex++)
	{
		PackedWorldSpaceIncidentLightingSH[4 + 0 * NUM_SH_VECTORS * 4 + BasisIndex - 1] = Light->WorldSpaceIncidentLighting.R.V[BasisIndex];
		PackedWorldSpaceIncidentLightingSH[4 + 1 * NUM_SH_VECTORS * 4 + BasisIndex - 1] = Light->WorldSpaceIncidentLighting.G.V[BasisIndex];
		PackedWorldSpaceIncidentLightingSH[4 + 2 * NUM_SH_VECTORS * 4 + BasisIndex - 1] = Light->WorldSpaceIncidentLighting.B.V[BasisIndex];
	}

	SetPixelShaderValues(
		PixelShader->GetPixelShader(),
		WorldIncidentLightingParameter,
		PackedWorldSpaceIncidentLightingSH,
		ARRAY_COUNT(PackedWorldSpaceIncidentLightingSH)
		);

	for(INT TextureIndex = 0;
		TextureIndex < GSHBasisCubeTextures.Textures.Num() &&
			TextureIndex < (INT)SHBasisCubeTexturesParameter.GetNumResources();
		TextureIndex++)
	{
		SetTextureParameter(
			PixelShader->GetPixelShader(),
			SHBasisCubeTexturesParameter,
			TStaticSamplerState<SF_Bilinear>::GetRHI(),
			GSHBasisCubeTextures.Textures(TextureIndex),
			TextureIndex
			);
	}
}

void FSphericalHarmonicLightPolicy::PixelParametersType::SetLight(FShader* PixelShader,const SceneInfoType* Light,const FSceneView* View) const
{
	SetSHPixelParameters(PixelShader, Light, WorldIncidentLightingParameter, SHBasisCubeTexturesParameter);
}

FLightSceneInfo* USphericalHarmonicLightComponent::CreateSceneInfo() const
{
	return new FSphericalHarmonicLightSceneInfo(this);
}

FVector4 USphericalHarmonicLightComponent::GetPosition() const
{
	return FVector4(0,0,1,0);
}

ELightComponentType USphericalHarmonicLightComponent::GetLightType() const
{
	return LightType_SphericalHarmonic;
}

void FSHLightLightMapPolicy::SetMesh(
	const VertexParametersType* VertexShaderParameters,
	const PixelParametersType* PixelShaderParameters,
	FShader* VertexShader,
	FShader* PixelShader,
	const FVertexFactory* VertexFactory,
	const FMaterialRenderProxy* MaterialRenderProxy,
	const ElementDataType& PrimitiveSceneInfo
	) const
{
	Super::SetMesh(VertexShaderParameters, PixelShaderParameters, VertexShader, PixelShader, VertexFactory, MaterialRenderProxy, PrimitiveSceneInfo);
	const FSphericalHarmonicLightSceneInfo* Light = static_cast<const FSphericalHarmonicLightSceneInfo*>(PrimitiveSceneInfo->SHLightSceneInfo);
	if (Light && PixelShaderParameters)
	{
		SetSHPixelParameters(PixelShader, Light, PixelShaderParameters->WorldIncidentLightingParameter, PixelShaderParameters->SHBasisCubeTexturesParameter);
	}
}
