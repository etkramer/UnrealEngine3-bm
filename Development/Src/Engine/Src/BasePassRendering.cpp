/*=============================================================================
	BasePassRendering.cpp: Base pass rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

// This is necessary because the C preprocessor thinks the comma in the template parameter list is a comma in the macro parameter list.
#define IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FogDensityPolicyType) \
	typedef TBasePassVertexShader<LightMapPolicyType,FogDensityPolicyType> TBasePassVertexShader##LightMapPolicyType##FogDensityPolicyType; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBasePassVertexShader##LightMapPolicyType##FogDensityPolicyType,TEXT("BasePassVertexShader"),TEXT("Main"),SF_Vertex,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

#define IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType,bEnableSkyLight,SkyLightShaderName) \
	typedef TBasePassPixelShader<LightMapPolicyType,bEnableSkyLight> TBasePassPixelShader##LightMapPolicyType##SkyLightShaderName; \
	IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TBasePassPixelShader##LightMapPolicyType##SkyLightShaderName,TEXT("BasePassPixelShader"),TEXT("Main"),SF_Pixel,0,0);

#define IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(LightMapPolicyType) \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FNoDensityPolicy); \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FConstantDensityPolicy); \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FLinearHalfspaceDensityPolicy); \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FSphereDensityPolicy); \
	IMPLEMENT_BASEPASS_VERTEXSHADER_TYPE(LightMapPolicyType,FConeDensityPolicy); \
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType,FALSE,NoSkyLight); \
	IMPLEMENT_BASEPASS_PIXELSHADER_TYPE(LightMapPolicyType,TRUE,SkyLight);

IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FNoLightMapPolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FDirectionalVertexLightMapPolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FSimpleVertexLightMapPolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FDirectionalLightMapTexturePolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FSimpleLightMapTexturePolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FDirectionalLightLightMapPolicy);
IMPLEMENT_BASEPASS_LIGHTMAPPED_SHADER_TYPE(FSHLightLightMapPolicy);

/** The action used to draw a base pass static mesh element. */
class FDrawBasePassStaticMeshAction
{
public:

	FScene* Scene;
	FStaticMesh* StaticMesh;

	/** Initialization constructor. */
	FDrawBasePassStaticMeshAction(FScene* InScene,FStaticMesh* InStaticMesh):
		Scene(InScene),
		StaticMesh(InStaticMesh)
	{}

	ESceneDepthPriorityGroup GetDPG(const FProcessBasePassMeshParameters& Parameters) const
	{
		return (ESceneDepthPriorityGroup)Parameters.PrimitiveSceneInfo->Proxy->GetStaticDepthPriorityGroup();
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
		FDepthPriorityGroup::EBasePassDrawListType DrawType = FDepthPriorityGroup::EBasePass_Default;		
 
		if( StaticMesh->IsDecal() )
		{
			// handle decal case by adding to the decal base pass draw lists
			if( StaticMesh->IsTranslucent() )
			{
				// transparent decals rendered in the base pass are handled separately
				DrawType = FDepthPriorityGroup::EBasePass_Decals_Translucent;
			}
			else
			{
				DrawType = FDepthPriorityGroup::EBasePass_Decals;
			}
		}
		else if (StaticMesh->IsMasked())
		{
			DrawType = FDepthPriorityGroup::EBasePass_Masked;	
		}

		// Find the appropriate draw list for the static mesh based on the light-map policy type.
		TStaticMeshDrawList<TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy> >& DrawList =
			Scene->DPGs[StaticMesh->DepthPriorityGroup].GetBasePassDrawList<LightMapPolicyType>(DrawType);

		// Add the static mesh to the draw list.
		DrawList.AddMesh(
			StaticMesh,
			typename TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy>::ElementDataType(LightMapElementData,FNoDensityPolicy::ElementDataType()),
			TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy>(
			StaticMesh->VertexFactory,
			StaticMesh->MaterialRenderProxy,
			LightMapPolicy,
			Parameters.BlendMode,
			Parameters.LightingModel != MLM_Unlit && StaticMesh->PrimitiveSceneInfo->HasDynamicSkyLighting()
			)
			);
	}
};

void FBasePassOpaqueDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	// Determine the mesh's material and blend mode.
	const FMaterial* Material = StaticMesh->MaterialRenderProxy->GetMaterial();
	const EBlendMode BlendMode = Material->GetBlendMode();

	// Only draw opaque, non-distorted materials.
	if( (!IsTranslucentBlendMode(BlendMode) && !Material->IsDistorted()) ||
		// allow for decals to batch using translucent materials when rendered on opaque meshes
		StaticMesh->IsDecal() )
	{
		ProcessBasePassMesh(
			FProcessBasePassMeshParameters(
				*StaticMesh,
				Material,
				StaticMesh->PrimitiveSceneInfo,
				FALSE
				),
			FDrawBasePassStaticMeshAction(Scene,StaticMesh)
			);
	}
}

/** The action used to draw a base pass dynamic mesh element. */
class FDrawBasePassDynamicMeshAction
{
public:

	const FSceneView& View;
	UBOOL bBackFace;
	UBOOL bPreFog;
	FHitProxyId HitProxyId;

	/** Initialization constructor. */
	FDrawBasePassDynamicMeshAction(
		const FSceneView& InView,
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

	/** Draws the translucent mesh with a specific light-map type, fog volume type, and shader complexity predicate. */
	template<typename LightMapPolicyType>
	void Process(
		const FProcessBasePassMeshParameters& Parameters,
		const LightMapPolicyType& LightMapPolicy,
		const typename LightMapPolicyType::ElementDataType& LightMapElementData
		) const
	{
		const UBOOL bIsLitMaterial = Parameters.LightingModel != MLM_Unlit;

		TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy> DrawingPolicy(
			Parameters.Mesh.VertexFactory,
			Parameters.Mesh.MaterialRenderProxy,
			LightMapPolicy,
			Parameters.BlendMode,
			(Parameters.PrimitiveSceneInfo && Parameters.PrimitiveSceneInfo->HasDynamicSkyLighting()) && bIsLitMaterial,
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
			typename TBasePassDrawingPolicy<LightMapPolicyType,FNoDensityPolicy>::ElementDataType(
				LightMapElementData,
				FNoDensityPolicy::ElementDataType()
				)
			);
		DrawingPolicy.DrawMesh(Parameters.Mesh);
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
		if(View.Family->ShowFlags & SHOW_Lighting)
		{
			Process<LightMapPolicyType>(Parameters,LightMapPolicy,LightMapElementData);
		}
		else
		{
			Process<FNoLightMapPolicy>(Parameters,FNoLightMapPolicy(),FNoLightMapPolicy::ElementDataType());
		}
	}
};

UBOOL FBasePassOpaqueDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// Determine the mesh's material and blend mode.
	const FMaterial* Material = Mesh.MaterialRenderProxy->GetMaterial();
	const EBlendMode BlendMode = Material->GetBlendMode();

	// Only draw opaque, non-distorted materials.
	if(!IsTranslucentBlendMode(BlendMode) && !Material->IsDistorted())
	{
		ProcessBasePassMesh(
			FProcessBasePassMeshParameters(
				Mesh,
				Material,
				PrimitiveSceneInfo,
				!bPreFog
				),
			FDrawBasePassDynamicMeshAction(
				View,
				bBackFace,
				HitProxyId
				)
			);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

