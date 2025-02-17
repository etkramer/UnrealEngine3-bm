/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __AMBIENTOCCLUSIONRENDERING_H__
#define __AMBIENTOCCLUSIONRENDERING_H__

struct FDownsampleDimensions
{
	UINT Factor;
	INT TargetX;
	INT TargetY;
	INT TargetSizeX;
	INT TargetSizeY;
	FLOAT ViewSizeX;
	FLOAT ViewSizeY;

	FDownsampleDimensions() {}
	FDownsampleDimensions(const FViewInfo& View);
};

/** Sets common ambient occlusion parameters */
class FAmbientOcclusionParams
{
public:

	/** Binds the parameters using a compiled shader's parameter map. */
	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets the scene texture parameters for the given view. */
	void Set(const FDownsampleDimensions& DownsampleDimensions, FShader* PixelShader, ESamplerFilter AOFilter);

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FAmbientOcclusionParams& P);

	FVector4 AOScreenPositionScaleBias;

private:
	FShaderResourceParameter AmbientOcclusionTextureParameter;
	FShaderResourceParameter AOHistoryTextureParameter;
	FShaderParameter AOScreenPositionScaleBiasParameter;
	FShaderParameter ScreenEdgeLimitsParameter;
};

struct FAmbientOcclusionSettings
{
	//See AmbientOcclusionEffect.uc for descriptions
	FLinearColor OcclusionColor;
	FLOAT OcclusionPower;
	FLOAT OcclusionScale;
	FLOAT OcclusionBias;
	FLOAT MinOcclusion;
	FLOAT OcclusionRadius;
	FLOAT OcclusionAttenuation;
	EAmbientOcclusionQuality OcclusionQuality;
	FLOAT OcclusionFadeoutMinDistance;
	FLOAT OcclusionFadeoutMaxDistance;
	FLOAT HaloDistanceThreshold;
	FLOAT HaloDistanceScale;
	FLOAT HaloOcclusion;
	FLOAT EdgeDistanceThreshold;
	FLOAT EdgeDistanceScale;
	FLOAT FilterDistanceScale;
	INT FilterSize;
	FLOAT HistoryOcclusionConvergenceTime;
	FLOAT HistoryWeightConvergenceTime;

	FAmbientOcclusionSettings(const UAmbientOcclusionEffect* InEffect);
};

template<UBOOL>
class TAOMeshVertexShader;

enum EAOMaskType
{
	AO_OcclusionMask,
	AO_HistoryMask,
	AO_HistoryUpdate,
	AO_HistoryMaskManualDepthTest,
	AO_HistoryUpdateManualDepthTest
};

template<EAOMaskType>
class TAOMaskPixelShader;

/**
* Drawing policy for rendering movable primitives which need to overwrite the occlusion history to hide occlusion streaking.
*/
class FAOMaskDrawingPolicy : public FMeshDrawingPolicy
{
public:
	static FDownsampleDimensions DownsampleDimensions;
	static FLOAT OcclusionConvergenceRate;
	static FLOAT WeightConvergenceRate;

	FAOMaskDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		UBOOL bAllowAmbientOcclusion,
		FLOAT DepthBias
		);

	// FMeshDrawingPolicy interface.
	UBOOL Matches(const FAOMaskDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) 
			&& MaskType == Other.MaskType
			&& HistoryUpdateVertexShader == Other.HistoryUpdateVertexShader 
			&& MaskVertexShader == Other.MaskVertexShader;
	}
	void DrawShared(const FViewInfo& View, FBoundShaderStateRHIRef ShaderState) const;
	void SetMeshRenderState(
		const FViewInfo& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const;

	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0);

	friend INT Compare(const FAOMaskDrawingPolicy& A,const FAOMaskDrawingPolicy& B);

	/** Indicates whether the primitive has moved since last frame. */
	static UBOOL HasMoved(const FPrimitiveSceneInfo* PrimitiveSceneInfo);

private:
	EAOMaskType MaskType;
	class TAOMeshVertexShader<TRUE>* HistoryUpdateVertexShader;
	class TAOMeshVertexShader<FALSE>* MaskVertexShader;
	TAOMaskPixelShader<AO_OcclusionMask>* OcclusionMaskPixelShader;
	TAOMaskPixelShader<AO_HistoryMask>* HistoryMaskPixelShader;
	TAOMaskPixelShader<AO_HistoryUpdate>* HistoryUpdatePixelShader;
	TAOMaskPixelShader<AO_HistoryMaskManualDepthTest>* HistoryMaskManualDepthTestPixelShader;
	TAOMaskPixelShader<AO_HistoryUpdateManualDepthTest>* HistoryUpdateManualDepthTestPixelShader;
};

/**
* A drawing policy factory for rendering movable primitives.
*/
class FAOMaskDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	struct ContextType
	{
		ESceneDepthPriorityGroup DPG;
		FLOAT DepthBias;

		ContextType(ESceneDepthPriorityGroup InDPG, FLOAT InDepthBias) :
			DPG(InDPG),
			DepthBias(InDepthBias)
		{}
	};
	static UBOOL DrawDynamicMesh(	
		const FViewInfo& View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);
	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy);
};

#endif
