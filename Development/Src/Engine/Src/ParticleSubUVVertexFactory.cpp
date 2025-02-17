/*=============================================================================
	ParticleSubUVVertexFactory.cpp: Particle vertex factory implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"

/**
 * The particle system sub UV vertex declaration resource type.
 */
class FParticleSystemSubUVVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor.
	virtual ~FParticleSystemSubUVVertexDeclaration() {}

	virtual void FillDeclElements(FVertexDeclarationElementList& Elements, INT&	Offset)
	{
		/** The stream to read the vertex position from.		*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Position,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the vertex old position from.	*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Normal,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the vertex size from.			*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float3,VEU_Tangent,0));
		Offset += sizeof(FLOAT) * 3;
		/** The stream to read the rotation from.				*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float1,VEU_BlendWeight,0));
		Offset += sizeof(FLOAT) * 1;
		/** The stream to read the color from.					*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,1));
		Offset += sizeof(FLOAT) * 4;
		/** The stream to read the texture coordinates from.	*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,0));
		Offset += sizeof(FLOAT) * 4;
		/** The stream to read the interoplation value from.	*/
		/** SHARED WITH the size scaling information.			*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,2));
		Offset += sizeof(FLOAT) * 4;
	}


	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		INT	Offset = 0;

		FillDeclElements(Elements, Offset);

		// Create the vertex declaration for rendering the factory normally.
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** The simple element vertex declaration. */
static TGlobalResource<FParticleSystemSubUVVertexDeclaration> GParticleSystemSubUVVertexDeclaration;

/**
 * The particle system sub UV dynamic parameter vertex declaration resource type.
 */
class FParticleSystemSubUVDynamicParamVertexDeclaration : public FParticleSystemSubUVVertexDeclaration
{
public:
	// Destructor.
	virtual ~FParticleSystemSubUVDynamicParamVertexDeclaration() {}

	virtual void FillDeclElements(FVertexDeclarationElementList& Elements, INT&	Offset)
	{
		FParticleSystemSubUVVertexDeclaration::FillDeclElements(Elements, Offset);
		/** The stream to read the dynamic parameter value from. */
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,3));
		Offset += sizeof(FLOAT) * 4;
	}
};

/** The simple element vertex declaration. */
static TGlobalResource<FParticleSystemSubUVDynamicParamVertexDeclaration> GParticleSystemSubUVDynamicParameterVertexDeclaration;

UBOOL FParticleSubUVVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithParticleSubUV() && !Material->GetUsesDynamicParameter()) || Material->IsSpecialEngineMaterial();
}

/**
 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
 */
void FParticleSubUVVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FParticleVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("SUBUV_PARTICLES"),TEXT("1"));
}

/**
 *	Initialize the Render Hardare Interface for this vertex factory
 */
void FParticleSubUVVertexFactory::InitRHI()
{
	SetDeclaration(GParticleSystemSubUVVertexDeclaration.VertexDeclarationRHI);
}

/**
 *	
 */
/**
 * Should we cache the material's shadertype on this platform with this vertex factory? 
 */
UBOOL FParticleSubUVDynamicParameterVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithParticleSubUV() && Material->GetUsesDynamicParameter()) || Material->IsSpecialEngineMaterial();
}

/**
 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
 */
void FParticleSubUVDynamicParameterVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FParticleSubUVVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("USE_DYNAMIC_PARAMETERS"),TEXT("1"));
}

// FRenderResource interface.
void FParticleSubUVDynamicParameterVertexFactory::InitRHI()
{
	SetDeclaration(GParticleSystemSubUVDynamicParameterVertexDeclaration.VertexDeclarationRHI);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleSubUVVertexFactory,FParticleVertexFactoryShaderParameters,"ParticleSpriteVertexFactory",TRUE,FALSE,FALSE, VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleSubUVDynamicParameterVertexFactory,FParticleVertexFactoryShaderParameters,"ParticleSpriteVertexFactory",TRUE,FALSE,FALSE, VER_DYNAMICPARAMETERS_ADDED,0);
