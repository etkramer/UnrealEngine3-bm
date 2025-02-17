/*=============================================================================
	SceneHitProxyRendering.cpp: Scene hit proxy rendering.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

/**
 * A vertex shader for rendering the depth of a mesh.
 */
class FHitProxyVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FHitProxyVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the hit proxy vertex shader on PC
		return (Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_SM3 || Platform == SP_PCD3D_SM2) && 
			// and only compile for the default material or materials that are masked.
			(Material->IsSpecialEngineMaterial() || Material->IsMasked());
	}

	FHitProxyVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
	}
	FHitProxyVertexShader() {}
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

IMPLEMENT_MATERIAL_SHADER_TYPE(,FHitProxyVertexShader,TEXT("HitProxyVertexShader"),TEXT("Main"),SF_Vertex,0,0);

/**
 * A pixel shader for rendering the depth of a mesh.
 */
class FHitProxyPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FHitProxyPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the hit proxy vertex shader on PC
		return (Platform == SP_PCD3D_SM4 || Platform == SP_PCD3D_SM3 || Platform == SP_PCD3D_SM2) 
			// and only compile for default materials or materials that are masked.
			&& (Material->IsSpecialEngineMaterial() || Material->IsMasked());
	}

	FHitProxyPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		HitProxyIdParameter.Bind(Initializer.ParameterMap,TEXT("HitProxyId"));
	}

	FHitProxyPixelShader() {}

	void SetParameters(const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView& View)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View.Family->CurrentWorldTime, View.Family->CurrentRealTime, &View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,Mesh,View,bBackFace);
	}

	void SetHitProxyId(FHitProxyId HitProxyId)
	{
		SetPixelShaderValue(GetPixelShader(),HitProxyIdParameter,HitProxyId.GetColor().ReinterpretAsLinear());
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << HitProxyIdParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter HitProxyIdParameter;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FHitProxyPixelShader,TEXT("HitProxyPixelShader"),TEXT("Main"),SF_Pixel,0,0);

FHitProxyDrawingPolicy::FHitProxyDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy
	):
	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
{
	const FMaterial* MaterialResource = InMaterialRenderProxy->GetMaterial();
	if (MaterialResource->IsMasked())
	{
		VertexShader = MaterialResource->GetShader<FHitProxyVertexShader>(InVertexFactory->GetType());
		PixelShader = MaterialResource->GetShader<FHitProxyPixelShader>(InVertexFactory->GetType());
	}
	else
	{
		// Override with the default material's shaders.  
		// Two-sided materials are still handled since we are only overriding which material the shaders come from.
		const FMaterial* DefaultMaterialResource = GEngine->DefaultMaterial->GetRenderProxy(FALSE)->GetMaterial();
		VertexShader = DefaultMaterialResource->GetShader<FHitProxyVertexShader>(InVertexFactory->GetType());
		PixelShader = DefaultMaterialResource->GetShader<FHitProxyPixelShader>(InVertexFactory->GetType());
	}
}

void FHitProxyDrawingPolicy::DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
{
	// Set the depth-only shader parameters for the material.
	VertexShader->SetParameters(VertexFactory,*View);
	PixelShader->SetParameters(MaterialRenderProxy,*View);

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared(View);

	// Set the actual shader & vertex declaration state
	RHISetBoundShaderState( BoundShaderState);
}

/** 
* Create bound shader state using the vertex decl from the mesh draw policy
* as well as the shaders needed to draw the mesh
* @param DynamicStride - optional stride for dynamic vertex data
* @return new bound shader state object
*/
FBoundShaderStateRHIRef FHitProxyDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
{
	FVertexDeclarationRHIRef VertexDeclaration;
	DWORD StreamStrides[MaxVertexElementCount];

	FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration, StreamStrides);
	if (DynamicStride)
	{
		StreamStrides[0] = DynamicStride;
	}

	return RHICreateBoundShaderState(VertexDeclaration, StreamStrides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader());	
}

void FHitProxyDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const FHitProxyId HitProxyId
	) const
{
	VertexShader->SetMesh(Mesh,View);
	PixelShader->SetMesh(Mesh,View,bBackFace);
	PixelShader->SetHitProxyId( HitProxyId);	
	FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
}

void FHitProxyDrawingPolicyFactory::AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType)
{
	checkSlow( Scene->RequiresHitProxies() );

	// Add the static mesh to the DPG's hit proxy draw list.
	const FMaterialRenderProxy* MaterialRenderProxy = StaticMesh->MaterialRenderProxy;
	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	if ( !Material->IsMasked() && !Material->IsTwoSided() )
	{
		// Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
		MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
	}
	Scene->DPGs[StaticMesh->DepthPriorityGroup].HitProxyDrawList.AddMesh(
		StaticMesh,
		StaticMesh->HitProxyId,
		FHitProxyDrawingPolicy(StaticMesh->VertexFactory,MaterialRenderProxy)
		);
}

UBOOL FHitProxyDrawingPolicyFactory::DrawDynamicMesh(
	const FSceneView& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	if (!PrimitiveSceneInfo || PrimitiveSceneInfo->bSelectable)
	{
		const FMaterialRenderProxy* MaterialRenderProxy = Mesh.MaterialRenderProxy;
		const FMaterial* Material = MaterialRenderProxy->GetMaterial();
		if ( !Material->IsMasked() && !Material->IsTwoSided() )
		{
			// Default material doesn't handle masked, and doesn't have the correct bIsTwoSided setting.
			MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
		}
		FHitProxyDrawingPolicy DrawingPolicy( Mesh.VertexFactory, MaterialRenderProxy );
		DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,HitProxyId);
		DrawingPolicy.DrawMesh(Mesh);
		return TRUE;
	}
	return FALSE;
}

void FSceneRenderer::RenderHitProxies()
{
#if !CONSOLE
	// Allocate the maximum scene render target space for the current view family.
	GSceneRenderTargets.Allocate( ViewFamily.RenderTarget->GetSizeX(), ViewFamily.RenderTarget->GetSizeY() );

	// Write to the hit proxy render target.
	GSceneRenderTargets.BeginRenderingHitProxies();

	// Clear color for each view.
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);
		RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
		RHIClear(TRUE,FLinearColor::White,FALSE,1,FALSE,0);
	}

	// Find the visible primitives.
	InitViews();

	// Draw the hit proxy IDs for all visible primitives.
	for(UINT DPGIndex = 0;DPGIndex < SDPG_MAX_SceneRender;DPGIndex++)
	{
		// Depth tests + writes, no alpha blending.
		RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);

			// Set the device viewport for the view.
			RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters(  &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

			// Clear the depth buffer for each DPG.
			RHIClear(FALSE,FLinearColor::Black,TRUE,1.0f,TRUE,0);

			// Draw the scene's hit proxy draw lists.
			Scene->DPGs[DPGIndex].HitProxyDrawList.DrawVisible(View,View.StaticMeshVisibilityMap);

			// Draw all dynamic primitives using the hit proxy drawing policy.
			DrawDynamicPrimitiveSet<FHitProxyDrawingPolicyFactory>(
				View,
				FHitProxyDrawingPolicyFactory::ContextType(),
				DPGIndex,
				TRUE,
				TRUE
				);

			// Draw the view's batched simple elements(lines, sprites, etc).
			View.BatchedViewElements[DPGIndex].Draw(View.ViewProjectionMatrix,View.SizeX,View.SizeY,TRUE);
		}
	}

	// Finish drawing to the hit proxy render target.
	GSceneRenderTargets.FinishRenderingHitProxies();

	// After scene rendering, disable the depth buffer.
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());

	//
	// Copy the hit proxy buffer into the view family's render target.
	//
	
	// Set up a FTexture that is used to draw the hit proxy buffer to the view family's render target.
	FTexture HitProxyRenderTargetTexture;
	HitProxyRenderTargetTexture.TextureRHI = GSceneRenderTargets.GetHitProxyTexture();
	HitProxyRenderTargetTexture.SamplerStateRHI = TStaticSamplerState<>::GetRHI();

	// Generate the vertices and triangles mapping the hit proxy RT pixels into the view family's RT pixels.
	FBatchedElements BatchedElements;
	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		const FViewInfo& View = Views(ViewIndex);

		const FLOAT U = (FLOAT)View.RenderTargetX / (FLOAT)GSceneRenderTargets.GetBufferSizeX();
		const FLOAT V = (FLOAT)View.RenderTargetX / (FLOAT)GSceneRenderTargets.GetBufferSizeY();
		const FLOAT SizeU = (FLOAT)View.RenderTargetSizeX / (FLOAT)GSceneRenderTargets.GetBufferSizeX();
		const FLOAT SizeV = (FLOAT)View.RenderTargetSizeY / (FLOAT)GSceneRenderTargets.GetBufferSizeY();

		const INT V00 = BatchedElements.AddVertex(FVector4(View.X,				View.Y,					0,1),FVector2D(U,			V),			FLinearColor::White,FHitProxyId());
		const INT V10 = BatchedElements.AddVertex(FVector4(View.X + View.SizeX,	View.Y,					0,1),FVector2D(U + SizeU,	V),			FLinearColor::White,FHitProxyId());
		const INT V01 = BatchedElements.AddVertex(FVector4(View.X,				View.Y + View.SizeY,	0,1),FVector2D(U,			V + SizeV),	FLinearColor::White,FHitProxyId());
		const INT V11 = BatchedElements.AddVertex(FVector4(View.X + View.SizeX,	View.Y + View.SizeY,	0,1),FVector2D(U + SizeU,	V + SizeV),	FLinearColor::White,FHitProxyId());

		BatchedElements.AddTriangle(V00,V10,V11,&HitProxyRenderTargetTexture,BLEND_Opaque);
		BatchedElements.AddTriangle(V00,V11,V01,&HitProxyRenderTargetTexture,BLEND_Opaque);
	}

	// Generate a transform which maps from view family RT pixel coordinates to Normalized Device Coordinates.
	const UINT RenderTargetSizeX = ViewFamily.RenderTarget->GetSizeX();
	const UINT RenderTargetSizeY = ViewFamily.RenderTarget->GetSizeY();
	const FMatrix PixelToView =
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) *
			FMatrix(
				FPlane(	1.0f / ((FLOAT)RenderTargetSizeX / 2.0f),	0.0,										0.0f,	0.0f	),
				FPlane(	0.0f,										-1.0f / ((FLOAT)RenderTargetSizeY / 2.0f),	0.0f,	0.0f	),
				FPlane(	0.0f,										0.0f,										1.0f,	0.0f	),
				FPlane(	-1.0f,										1.0f,										0.0f,	1.0f	)
				);

	// Draw the triangles to the view family's render target.
	RHISetRenderTarget(ViewFamily.RenderTarget->GetRenderTargetSurface(),FSurfaceRHIRef());
	BatchedElements.Draw(
				PixelToView,
				RenderTargetSizeX,
				RenderTargetSizeY,
				FALSE,
				1.0f
				);
#endif
}
