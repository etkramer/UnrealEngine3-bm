/*=============================================================================
	DepthRendering.cpp: Depth rendering implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/**
 * A vertex shader for rendering the depth of a mesh.
 */
template <UBOOL bUsePositionOnlyStream>
class TDepthOnlyVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(TDepthOnlyVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only the local vertex factory supports the position-only stream
		if (bUsePositionOnlyStream)
		{
			if (appStrstr(VertexFactoryType->GetName(), TEXT("FLocalVertex")))
			{
				return Material->IsSpecialEngineMaterial();
			}
			return FALSE;
		}

		// Only compile for the default material and masked materials
		return Material->IsSpecialEngineMaterial() || Material->IsMasked();
	}

	TDepthOnlyVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
	}
	TDepthOnlyVertexShader() {}
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		return bShaderHasOutdatedParameters;
	}
	void SetParameters(const FVertexFactory* VertexFactory,const FSceneView& View)
	{
		VertexFactoryParameters.Set(this,VertexFactory,View);
	}
	void SetMesh(const FMeshElement& Mesh,const FSceneView& View)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,View);
	}

private:
	FVertexFactoryParameterRef VertexFactoryParameters;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TDepthOnlyVertexShader<TRUE>,TEXT("PositionOnlyDepthVertexShader"),TEXT("Main"),SF_Vertex,0,0);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TDepthOnlyVertexShader<FALSE>,TEXT("DepthOnlyVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
* A pixel shader for rendering the depth of a mesh.
*/
class FDepthOnlyPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FDepthOnlyPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile for materials that are masked
		return Material->IsMasked();
	}

	FDepthOnlyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
	FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
	}

	FDepthOnlyPixelShader() {}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View->Family->CurrentWorldTime, View->Family->CurrentRealTime, View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,Mesh,View,bBackFace);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
#if PS3
		// @todo hack: this works around a gcc 411 compiler issue where is strips this function out with the -Wl,--gc-sections linker option
		// it just needs to access a global variable to keep the function from being stripped
		GNumHardwareThreads = 2;
#endif
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FDepthOnlyPixelShader,TEXT("DepthOnlyPixelShader"),TEXT("Main"),SF_Pixel,0,0);


FDepthDrawingPolicy::FDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy
	):
	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();

	// The primitive needs to be rendered with the material's pixel and vertex shaders if it is masked
	bNeedsPixelShader = MaterialResource->IsMasked();

	if (bNeedsPixelShader)
	{
		VertexShader = MaterialResource->GetShader<TDepthOnlyVertexShader<FALSE> >(InVertexFactory->GetType());
		PixelShader = MaterialResource->GetShader<FDepthOnlyPixelShader>(InVertexFactory->GetType());
	}
	else
	{
		// Override with the default material's vertex shaders.
		// Two-sided materials are still handled since we are only overriding which material the shaders come from.
		const FMaterial* DefaultMaterialResource = GEngine->DefaultMaterial->GetRenderProxy(FALSE)->GetMaterial();
		VertexShader = DefaultMaterialResource->GetShader<TDepthOnlyVertexShader<FALSE> >(InVertexFactory->GetType());
		PixelShader = NULL;
	}
}

void FDepthDrawingPolicy::DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material.
	VertexShader->SetParameters(VertexFactory,*View);

	if (bNeedsPixelShader)
	{
		PixelShader->SetParameters(VertexFactory,MaterialRenderProxy,View);
	}

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared(View);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState(BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);

	if (DynamicStride)
	{
        StreamStrides[0] = DynamicStride;
	}	
	
	// Use the compiled pixel shader for masked materials, since they need to clip,
	// and the NULL pixel shader for opaque materials to get double speed z on Xenon.
	return RHICreateBoundShaderState(
		VertexDeclaration, 
		StreamStrides, 
		VertexShader->GetVertexShader(), 
		bNeedsPixelShader ? PixelShader->GetPixelShader() : FPixelShaderRHIRef());
}

void FDepthDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetMesh(Mesh,View);
	if (bNeedsPixelShader)
	{
		PixelShader->SetMesh(Mesh,View,bBackFace);
	}
	FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FDepthDrawingPolicy& A,const FDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(bNeedsPixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}


FPositionOnlyDepthDrawingPolicy::FPositionOnlyDepthDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy
	):
	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();
	VertexShader = MaterialResource->GetShader<TDepthOnlyVertexShader<TRUE> >(InVertexFactory->GetType());
}

void FPositionOnlyDepthDrawingPolicy::DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material.
	VertexShader->SetParameters(VertexFactory,*View);

	// Set the shared mesh resources.
	VertexFactory->SetPositionStream();

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState( BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FPositionOnlyDepthDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIParamRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];
	
	VertexFactory->GetPositionStreamStride(StreamStrides);
	VertexDeclaration = VertexFactory->GetPositionDeclaration();

	if (DynamicStride)
	{
        StreamStrides[0] = DynamicStride;
	}	

	checkSlow(MaterialRenderProxy->GetMaterial()->GetBlendMode() == BLEND_Opaque);
	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), FPixelShaderRHIRef());
}

void FPositionOnlyDepthDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetMesh(Mesh,View);
	FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,ElementData);
}

INT Compare(const FPositionOnlyDepthDrawingPolicy& A,const FPositionOnlyDepthDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}

void FDepthDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType DepthDrawingMode)
{
	const FMaterialRenderProxy* MaterialRenderProxy = StaticMesh->MaterialRenderProxy;
	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	const EBlendMode BlendMode = Material->GetBlendMode();

	if (BlendMode == BLEND_Opaque)
	{
		if (StaticMesh->VertexFactory->SupportsPositionOnlyStream() && !Material->IsTwoSided())
		{
			// Add the static mesh to the position-only depth draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].PositionOnlyDepthDrawList.AddMesh(
				StaticMesh,
				FPositionOnlyDepthDrawingPolicy::ElementDataType(),
				FPositionOnlyDepthDrawingPolicy(StaticMesh->VertexFactory,GEngine->DefaultMaterial->GetRenderProxy(FALSE))
				);
		}
		else
		{
			if (!Material->IsTwoSided())
			{
				// Override with the default material for everything but opaque two sided materials
				MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
			}

			// Add the static mesh to the opaque depth-only draw list.
			Scene->DPGs[StaticMesh->DepthPriorityGroup].DepthDrawList.AddMesh(
				StaticMesh,
				FDepthDrawingPolicy::ElementDataType(),
				FDepthDrawingPolicy(StaticMesh->VertexFactory,MaterialRenderProxy)
				);
		}
	}

	// FDepthDrawingPolicyFactory does not currently support masked materials through the static rendering path
	// A new TStaticMeshDrawList needs to be added for masked materials if needed
	check(!Material->IsMasked());
}

UBOOL FDepthDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	ContextType DepthDrawingMode,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	UBOOL bDirty = FALSE;

	//Do a per-FMeshElement check on top of the proxy check in RenderPrePass to handle the case where a proxy that is relevant 
	//to the depth only pass has to submit multiple FMeshElements but only some of them should be used as occluders.
	if (Mesh.bUseAsOccluder)
	{
		const FMaterialRenderProxy* MaterialRenderProxy = Mesh.MaterialRenderProxy;
		const FMaterial* Material = MaterialRenderProxy->GetMaterial();
		const EBlendMode BlendMode = Material->GetBlendMode();

		if (BlendMode == BLEND_Opaque && Mesh.VertexFactory->SupportsPositionOnlyStream() && !Material->IsTwoSided())
		{
			//render opaque primitives that support a separate position-only vertex buffer
			FPositionOnlyDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory,GEngine->DefaultMaterial->GetRenderProxy(FALSE));
			DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
			DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FPositionOnlyDepthDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Mesh);
			bDirty = TRUE;
		}
		else if (!IsTranslucentBlendMode(BlendMode))
		{
			const UBOOL bMaterialMasked = Material->IsMasked();

			if (DepthDrawingMode == DDM_AllOccluders
				|| DepthDrawingMode == DDM_NonMaskedOnly && !bMaterialMasked)
			{
				if (!bMaterialMasked && !Material->IsTwoSided())
				{
					// Override with the default material for opaque materials that are not two sided
					MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
				}

				FDepthDrawingPolicy DrawingPolicy(Mesh.VertexFactory,MaterialRenderProxy);
				DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
				DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
				DrawingPolicy.DrawMesh(Mesh);
				bDirty = TRUE;
			}
		}
	}
	
	return bDirty;
}

UBOOL FDepthDrawingPolicyFactory::IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
{
	return IsTranslucentBlendMode(MaterialRenderProxy->GetMaterial()->GetBlendMode());
}
