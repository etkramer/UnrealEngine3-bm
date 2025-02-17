/*=============================================================================
	SkyLightComponent.cpp: SkyLightComponent implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

IMPLEMENT_CLASS(USkyLightComponent);

extern TGlobalResource<FShadowFrustumVertexDeclaration> GShadowFrustumVertexDeclaration;

/**
 * Information used to render a sky light.
 */
class FSkyLightSceneInfo : public FLightSceneInfo, public FLightSceneDPGInfoInterface
{
public:
	
	/** Initialization constructor. */
	FSkyLightSceneInfo(const USkyLightComponent* Component):
		FLightSceneInfo(Component),
		LowerColor(FLinearColor(Component->LowerColor) * Component->LowerBrightness)
	{
	}

	// FLightSceneInfo interface.
	virtual void AttachPrimitive(const FLightPrimitiveInteraction& Interaction)
	{
		if(!Interaction.IsLightMapped())
		{
			Interaction.GetPrimitiveSceneInfo()->UpperSkyLightColor += Color;
			Interaction.GetPrimitiveSceneInfo()->LowerSkyLightColor += LowerColor;

			// Update the primitive's static meshes, to ensure they use the right version of the base pass shaders.
			Interaction.GetPrimitiveSceneInfo()->BeginDeferredUpdateStaticMeshes();
		}
	}
	virtual void DetachPrimitive(const FLightPrimitiveInteraction& Interaction)
	{
		if(!Interaction.IsLightMapped())
		{
			Interaction.GetPrimitiveSceneInfo()->UpperSkyLightColor -= Color;
			Interaction.GetPrimitiveSceneInfo()->LowerSkyLightColor -= LowerColor;

			// Update the primitive's static meshes, to ensure they use the right version of the base pass shaders.
			Interaction.GetPrimitiveSceneInfo()->BeginDeferredUpdateStaticMeshes();
		}
	}
	virtual const FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex) const
	{
		check(DPGIndex < SDPG_MAX_SceneRender);
		return this;
	}
	virtual FLightSceneDPGInfoInterface* GetDPGInfo(UINT DPGIndex)
	{
		check(DPGIndex < SDPG_MAX_SceneRender);
		return this;
	}
	/**
	* @return modulated shadow projection pixel shader for this light type
	*/
	virtual class FShadowProjectionPixelShaderInterface* GetModShadowProjPixelShader() const
	{
		return NULL;
	}

	/**
	* @return Branching PCF modulated shadow projection pixel shader for this light type
	*/
	virtual class FBranchingPCFProjectionPixelShaderInterface* GetBranchingPCFModProjPixelShader() const
	{
		return NULL;
	}

	/**
	* @return modulated shadow projection pixel shader for this light type
	*/
	virtual class FModShadowVolumePixelShader* GetModShadowVolumeShader() const
	{
		return NULL;
	}

	/**
	* @return modulated shadow projection bound shader state for this light type
	*/
	virtual FGlobalBoundShaderState* GetModShadowProjBoundShaderState() const
	{
		return &ModShadowProjBoundShaderState;
	}

#if SUPPORTS_VSM
	/**
	* @return VSM modulated shadow projection pixel shader for this light type
	*/
	virtual class FVSMModProjectionPixelShader* GetVSMModProjPixelShader() const
	{
		return NULL;
	}
	/**
	* @return VSM modulated shadow projection bound shader state for this light type
	*/
	virtual FBoundShaderStateRHIParamRef GetVSMModProjBoundShaderState() const
	{
		if (!IsValidRef(VSMModProjBoundShaderState))
		{
			DWORD Strides[MaxVertexElementCount];
			appMemzero(Strides, sizeof(Strides));
			Strides[0] = sizeof(FVector);

			TShaderMapRef<FModShadowProjectionVertexShader> ModShadowProjVertexShader(GetGlobalShaderMap());

			VSMModProjBoundShaderState = RHICreateBoundShaderState(GShadowFrustumVertexDeclaration.VertexDeclarationRHI, Strides, ModShadowProjVertexShader->GetVertexShader(), FPixelShaderRHIRef());
		}

		return VSMModProjBoundShaderState;
	}
#endif //#if SUPPORTS_VSM

	/**
	* @return PCF Branching modulated shadow projection bound shader state for this light type
	*/
	virtual FGlobalBoundShaderState* GetBranchingPCFModProjBoundShaderState() const
	{
		//allow the BPCF implementation to choose which of the loaded bound shader states should be used
		FGlobalBoundShaderState* CurrentBPCFBoundShaderState = ChooseBPCFBoundShaderState(ShadowFilterQuality, &ModBranchingPCFLowQualityBoundShaderState, 
			&ModBranchingPCFMediumQualityBoundShaderState, &ModBranchingPCFHighQualityBoundShaderState);

		return CurrentBPCFBoundShaderState;
	}


	/**
	* @return modulated shadow volume bound shader state for this light type
	*/
	virtual FBoundShaderStateRHIParamRef GetModShadowVolumeBoundShaderState() const
	{
		if (!IsValidRef(ModShadowVolumeBoundShaderState))
		{
			DWORD Strides[MaxVertexElementCount];
			appMemzero(Strides, sizeof(Strides));
			Strides[0] = sizeof(FSimpleElementVertex);

			TShaderMapRef<FModShadowVolumeVertexShader> ModShadowVolumeVertexShader(GetGlobalShaderMap());
			
			ModShadowVolumeBoundShaderState = RHICreateBoundShaderState(GSimpleElementVertexDeclaration.VertexDeclarationRHI, Strides, ModShadowVolumeVertexShader->GetVertexShader(), FPixelShaderRHIRef());
		}

		return ModShadowVolumeBoundShaderState;
	}

	// FLightSceneDPGInfoInterface
	virtual UBOOL DrawStaticMeshesVisible(
		const FViewInfo& View,
		const TBitArray<SceneRenderingBitArrayAllocator>& StaticMeshVisibilityMap,
		ELightPassDrawListType DrawType
		) const
	{
		return FALSE;
	}

	virtual UBOOL DrawStaticMeshesAll(
		const FViewInfo& View,
		ELightPassDrawListType DrawType
		) const
	{
		return FALSE;
	}

	virtual ELightInteractionType AttachStaticMesh(const FLightSceneInfo* LightSceneInfo,FStaticMesh* Mesh)
	{
		return LIT_Uncached;
	}

	virtual void DetachStaticMeshes()
	{
	}

	virtual UBOOL DrawDynamicMesh(
		const FSceneView& View,
		const FLightSceneInfo* LightSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		) const
	{
		return FALSE;
	}

private:

	/** The light color of the lower hemisphere of the sky light. */
	FLinearColor LowerColor;
};

FLightSceneInfo* USkyLightComponent::CreateSceneInfo() const
{
	return new FSkyLightSceneInfo(this);
}

FVector4 USkyLightComponent::GetPosition() const
{
	return FVector4(0,0,1,0);
}

/**
* @return ELightComponentType for the light component class 
*/
ELightComponentType USkyLightComponent::GetLightType() const
{
	return LightType_Sky;
}

/**
 * Called when a property is being changed.
 *
 * @param PropertyThatChanged	Property that changed or NULL if unknown or multiple
 */
void  USkyLightComponent::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange( PropertyThatChanged );
}

/**
 * Called after data has been serialized.
 */
void USkyLightComponent::PostLoad()
{
	Super::PostLoad();
}




