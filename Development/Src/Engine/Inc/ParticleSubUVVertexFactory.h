/*=============================================================================
	ParticleSubUVVertexFactory.h: Particle SubUV vertex factory definitions.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 *	
 */
class FParticleSubUVVertexFactory : public FParticleVertexFactory
{
	//@todo.SAS. Need to implement this once VertexFactory classes can declare 'non-compilable' materials.
	DECLARE_VERTEX_FACTORY_TYPE(FParticleSubUVVertexFactory);

public:
	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	// FRenderResource interface.
	virtual void InitRHI();

private:
};

/**
 *	
 */
class FParticleSubUVDynamicParameterVertexFactory : public FParticleSubUVVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FParticleSubUVDynamicParameterVertexFactory);

public:
	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
	 */
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment);

	// FRenderResource interface.
	virtual void InitRHI();

private:
};
