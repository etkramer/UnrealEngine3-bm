/*=============================================================================
	LocalDecalVertexFactory.h: Local vertex factory bound to a shader that computes decal texture coordinates.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __LOCALDECALVERTEXFACTORY_H__
#define __LOCALDECALVERTEXFACTORY_H__

/**
 * A vertex factory which simply transforms explicit vertex attributes from local to world space.
 */
class FLocalDecalVertexFactory : public FLocalVertexFactory, public FDecalVertexFactoryBase
{
	DECLARE_VERTEX_FACTORY_TYPE(FLocalDecalVertexFactory);
public:

	virtual FVertexFactory* CastToFVertexFactory()
	{
		return static_cast<FVertexFactory*>( this );
	}

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		return (Material->IsUsedWithDecals() || Material->IsDecalMaterial() || Material->IsSpecialEngineMaterial()) 
			&& FLocalVertexFactory::ShouldCache( Platform, Material, ShaderType );
	}

	/**
	* Modify compile environment to enable the decal codepath
	* @param OutEnvironment - shader compile environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("DECAL_FACTORY"),TEXT("1"));
	}
};

#endif // __LOCALDECALVERTEXFACTORY_H__
