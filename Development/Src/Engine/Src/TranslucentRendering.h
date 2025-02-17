/*=============================================================================
	TranslucentRendering.h: Translucent rendering definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
* Vertex shader used for combining LDR translucency with scene color when floating point blending isn't supported
*/
class FLDRExtractVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLDRExtractVertexShader,Global);

	/** 
	* Only cache these shaders for SM2
	*
	* @param Platform - current platform being compiled
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		//this is used for shader complexity, so needs to compile for all platforms
		return TRUE;
	}

	/** Default constructor. */
	FLDRExtractVertexShader() {}

public:

	/** Initialization constructor. */
	FLDRExtractVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:	FGlobalShader(Initializer)
	{
	}
};

/**
* Translucent draw policy factory.
* Creates the policies needed for rendering a mesh based on its material
*/
class FTranslucencyDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = TRUE };
	struct ContextType {};

	/**
	* Render a dynamic mesh using a translucent draw policy 
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawDynamicMesh(
		const FViewInfo& View,
		ContextType DrawingContext,
		const FMeshElement& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	/**
	* Render a dynamic mesh using a translucent draw policy 
	* @return TRUE if the mesh rendered
	*/
	static UBOOL DrawStaticMesh(
		const FViewInfo* View,
		ContextType DrawingContext,
		const FStaticMesh& StaticMesh,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		);

	static UBOOL IsMaterialIgnored(const FMaterialRenderProxy* MaterialRenderProxy)
	{
		return !IsTranslucentBlendMode(MaterialRenderProxy->GetMaterial()->GetBlendMode());
	}
};
