/*=============================================================================
	VelocityRendering.h: Velocity rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define MAX_PIXELVELOCITY	(16.0f/1280.0f)

/**
 * Outputs a 2d velocity vector.
 */
class FVelocityDrawingPolicy : public FMeshDrawingPolicy
{
public:
	FVelocityDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy
		);

	// FMeshDrawingPolicy interface.
	UBOOL Matches(const FVelocityDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other) && VertexShader == Other.VertexShader && PixelShader == Other.PixelShader;
	}
	void DrawShared( const FSceneView* View, FBoundShaderStateRHIRef ShaderState ) const;
	void SetMeshRenderState(
		const FViewInfo& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const;

	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0);

	friend INT Compare(const FVelocityDrawingPolicy& A,const FVelocityDrawingPolicy& B);

	UBOOL SupportsVelocity( ) const;

	/** Determines whether this primitive has motionblur velocity to render */
	static UBOOL HasVelocity(const FViewInfo& View, const FPrimitiveSceneInfo* PrimitiveSceneInfo);

private:
	class FVelocityVertexShader* VertexShader;
	class FVelocityPixelShader* PixelShader;
};

/**
 * A drawing policy factory for rendering motion velocity.
 */
class FVelocityDrawingPolicyFactory : public FDepthDrawingPolicyFactory
{
public:
	static void AddStaticMesh(FScene* Scene,FStaticMesh* StaticMesh,ContextType = ContextType());
	static UBOOL DrawDynamicMesh(	
		const FViewInfo& View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);
};
