/*=============================================================================
	TextureDensityRendering.cpp: Implementation for rendering texture density.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"

// This define must match the corresponding one in TextureDensityShader.usf
#define MAX_LOOKUPS 16

/**
 * A vertex shader for rendering texture density.
 */
class FTextureDensityVertexShader : public FShader
{
	DECLARE_SHADER_TYPE(FTextureDensityVertexShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shaders for the default material or if it's masked.
		return Material->GetUniform2DTextureExpressions().Num() > 0 
			&& Material->GetUserTexCoordsUsed() > 0 
			&& (Material->IsSpecialEngineMaterial() || Material->IsMasked())
			&& Platform != SP_PCD3D_SM2;
	}

	FTextureDensityVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer),
		VertexFactoryParameters(Initializer.VertexFactoryType,Initializer.ParameterMap)
	{
	}
	FTextureDensityVertexShader() {}
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

IMPLEMENT_MATERIAL_SHADER_TYPE(,FTextureDensityVertexShader,TEXT("TextureDensityShader"),TEXT("MainVertexShader"),SF_Vertex,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);

/**
 * A pixel shader for rendering texture density.
 */
class FTextureDensityPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(FTextureDensityPixelShader,MeshMaterial);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		// Only compile the shaders for the default material or if it's masked.
		return Material->GetUniform2DTextureExpressions().Num() > 0 
			&& Material->GetUserTexCoordsUsed() > 0 
			&& (Material->IsSpecialEngineMaterial() || Material->IsMasked())
			&& Platform != SP_PCD3D_SM2;
	}

	FTextureDensityPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.Material,Initializer.ParameterMap);
		TextureDensityParameters.Bind(Initializer.ParameterMap,TEXT("TextureDensityParameters"));
		TextureLookupInfo.Bind(Initializer.ParameterMap,TEXT("TextureLookupInfo"));
	}

	FTextureDensityPixelShader() {}

	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView& View, const FMaterialRenderProxy* OriginalRenderProxy)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy, View.Family->CurrentWorldTime, View.Family->CurrentRealTime, &View);
		const FMaterial* OriginalMaterial = OriginalRenderProxy->GetMaterial();
		const TArray<FMaterial::FTextureLookup> &LookupInfo = OriginalMaterial->GetTextureLookupInfo();
		INT NumLookups = Min( LookupInfo.Num(), MAX_LOOKUPS );

		FVector4 LookupParameters[ MAX_LOOKUPS ];
		FVector4 DensityParameters(
			NumLookups,
			GEngine->MinTextureDensity * GEngine->MinTextureDensity,
			GEngine->IdealTextureDensity * GEngine->IdealTextureDensity,
			GEngine->MaxTextureDensity * GEngine->MaxTextureDensity );

		for( INT LookupIndex=0; LookupIndex < NumLookups; ++LookupIndex )
		{
			const FMaterial::FTextureLookup &Lookup = LookupInfo( LookupIndex );
			const FTexture* Texture = NULL;
			OriginalMaterial->GetUniform2DTextureExpressions()( Lookup.TextureIndex )->GetTextureValue( MaterialRenderContext, &Texture );
			check( Texture );
			LookupParameters[LookupIndex][0] = FLOAT(Texture->GetSizeX()) * Lookup.UScale;
			LookupParameters[LookupIndex][1] = FLOAT(Texture->GetSizeY()) * Lookup.VScale;
			LookupParameters[LookupIndex][2] = Lookup.TexCoordIndex;
		}

		SetPixelShaderValues( GetPixelShader(), TextureLookupInfo, LookupParameters, NumLookups );
		SetPixelShaderValue( GetPixelShader(), TextureDensityParameters, DensityParameters );

		MaterialParameters.Set( this, MaterialRenderContext);
	}

	void SetMesh(const FMeshElement& Mesh,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh( this, Mesh, View, bBackFace);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		Ar << TextureDensityParameters;
		Ar << TextureLookupInfo;
		return bShaderHasOutdatedParameters;
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderParameter TextureDensityParameters;
	FShaderParameter TextureLookupInfo;
};

IMPLEMENT_MATERIAL_SHADER_TYPE(,FTextureDensityPixelShader,TEXT("TextureDensityShader"),TEXT("MainPixelShader"),SF_Pixel,VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);


//=============================================================================
/** FTextureDensityDrawingPolicy - Policy to wrap FMeshDrawingPolicy with new shaders. */

FTextureDensityDrawingPolicy::FTextureDensityDrawingPolicy(
	const FVertexFactory* InVertexFactory,
	const FMaterialRenderProxy* InMaterialRenderProxy,
	const FMaterialRenderProxy* InOriginalRenderProxy
	)
	:	FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy)
	,	OriginalRenderProxy(InOriginalRenderProxy)
{
	const FMaterialShaderMap* MaterialShaderIndex = InMaterialRenderProxy->GetMaterial()->GetShaderMap();
	const FMeshMaterialShaderMap* MeshShaderIndex = MaterialShaderIndex->GetMeshShaderMap(InVertexFactory->GetType());
	UBOOL HasVertexShader = MeshShaderIndex->HasShader(&FTextureDensityVertexShader::StaticType);
	UBOOL HasPixelShader = MeshShaderIndex->HasShader(&FTextureDensityPixelShader::StaticType);
	VertexShader = HasVertexShader ? MeshShaderIndex->GetShader<FTextureDensityVertexShader>() : NULL;
	PixelShader = HasPixelShader ? MeshShaderIndex->GetShader<FTextureDensityPixelShader>() : NULL;
}

void FTextureDensityDrawingPolicy::DrawShared( const FSceneView* SceneView, FBoundShaderStateRHIRef ShaderState ) const
{
	// NOTE: Assuming this cast is always safe!
	FViewInfo* View = (FViewInfo*) SceneView;

	// Set the depth-only shader parameters for the material.
	RHISetBoundShaderState( ShaderState );
	VertexShader->SetParameters( VertexFactory, *View );
	PixelShader->SetParameters( VertexFactory, MaterialRenderProxy, *View, OriginalRenderProxy );

	// Set the shared mesh resources.
	FMeshDrawingPolicy::DrawShared( View );
}

void FTextureDensityDrawingPolicy::SetMeshRenderState(
	const FSceneView& View,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	const ElementDataType& ElementData
	) const
{
	VertexShader->SetMesh( Mesh, View);
	PixelShader->SetMesh( Mesh, View, bBackFace);
	FMeshDrawingPolicy::SetMeshRenderState( View, PrimitiveSceneInfo, Mesh,bBackFace, ElementData);
}

/** 
 * Create bound shader state using the vertex decl from the mesh draw policy
 * as well as the shaders needed to draw the mesh
 * @param DynamicStride - optional stride for dynamic vertex data
 * @return new bound shader state object
 */
FBoundShaderStateRHIRef FTextureDensityDrawingPolicy::CreateBoundShaderState(DWORD DynamicStride)
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

INT Compare(const FTextureDensityDrawingPolicy& A,const FTextureDensityDrawingPolicy& B)
{
	COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
	COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
	COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
	COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
	return 0;
}


//=============================================================================
/** Policy to wrap FMeshDrawingPolicy with new shaders. */

UBOOL FTextureDensityDrawingPolicyFactory::DrawDynamicMesh(
	const FViewInfo& View,
	ContextType DrawingContext,
	const FMeshElement& Mesh,
	UBOOL bBackFace,
	UBOOL bPreFog,
	const FPrimitiveSceneInfo* PrimitiveSceneInfo,
	FHitProxyId HitProxyId
	)
{
	// Only draw opaque materials in the depth pass.
	const FMaterialRenderProxy* OriginalRenderProxy = Mesh.MaterialRenderProxy;
	const FMaterialRenderProxy* MaterialRenderProxy = OriginalRenderProxy;
	const FMaterial* Material = MaterialRenderProxy->GetMaterial();
	const FMaterial::FTextureLookupInfo& TextureLookupInfo = Material->GetTextureLookupInfo();
	if ( TextureLookupInfo.Num() > 0 )
	{
		if ( !Material->IsMasked() )
		{
			// Default material doesn't handle masked.
			MaterialRenderProxy = GEngine->DefaultMaterial->GetRenderProxy(FALSE);
		}
		FTextureDensityDrawingPolicy DrawingPolicy( Mesh.VertexFactory, MaterialRenderProxy, OriginalRenderProxy );
		DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,bBackFace,FMeshDrawingPolicy::ElementDataType());
		DrawingPolicy.DrawMesh(Mesh);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** Renders world-space texture density instead of the normal color. */
UBOOL FSceneRenderer::RenderTextureDensities(UINT DPGIndex)
{
	SCOPED_DRAW_EVENT(EventTextureDensity)(DEC_SCENE_ITEMS,TEXT("RenderTextureDensity"));

	if (GRHIShaderPlatform == SP_PCD3D_SM2)
	{
		//PS2.0 can't handle the gradient instructions
		return FALSE;
	}

	UBOOL bWorldDpg = (DPGIndex == SDPG_World);
	UBOOL bDirty = FALSE;

	// Opaque blending, enable depth tests and writes.
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		SCOPED_CONDITIONAL_DRAW_EVENT(EventView,Views.Num() > 1)(DEC_SCENE_ITEMS,TEXT("View%d"),ViewIndex);

		FViewInfo& View = Views(ViewIndex);
		const FSceneViewState* ViewState = (FSceneViewState*)View.State;

		// Set the device viewport for the view.
		RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
		RHISetViewParameters( &View, View.TranslatedViewProjectionMatrix, View.ViewOrigin );

		if (GUseTilingCode && bWorldDpg)
		{
			RHIMSAAFixViewport();
		}

		// Draw texture density for dynamic meshes.
		TDynamicPrimitiveDrawer<FTextureDensityDrawingPolicyFactory> Drawer(&View,DPGIndex,FTextureDensityDrawingPolicyFactory::ContextType(),TRUE);
		for(INT PrimitiveIndex = 0;PrimitiveIndex < View.VisibleDynamicPrimitives.Num();PrimitiveIndex++)
		{
			const FPrimitiveSceneInfo* PrimitiveSceneInfo = View.VisibleDynamicPrimitives(PrimitiveIndex);
			const FPrimitiveViewRelevance& PrimitiveViewRelevance = View.PrimitiveViewRelevanceMap(PrimitiveSceneInfo->Id);

			const UBOOL bVisible = View.PrimitiveVisibilityMap(PrimitiveSceneInfo->Id);
			if( bVisible &&		PrimitiveViewRelevance.GetDPG(DPGIndex) )
			{
				Drawer.SetPrimitive(PrimitiveSceneInfo);
				PrimitiveSceneInfo->Proxy->DrawDynamicElements(
					&Drawer,
					&View,
					DPGIndex
					);
			}
		}
		bDirty |= Drawer.IsDirty();
	}

	return bDirty;
}
