/*=============================================================================
	ParticleVertexFactory.cpp: Particle vertex factory implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "EnginePrivate.h"
#include "EngineParticleClasses.h"

/**
 * The particle system vertex declaration resource type.
 */
class FParticleSystemVertexDeclaration : public FRenderResource
{
public:

	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor.
	virtual ~FParticleSystemVertexDeclaration() {}

	virtual void FillDeclElements(FVertexDeclarationElementList& Elements, INT& Offset)
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
		Elements.AddItem(FVertexElement(0,Offset,VET_Float2,VEU_TextureCoordinate,0));
		Offset += sizeof(FLOAT) * 2;
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
static TGlobalResource<FParticleSystemVertexDeclaration> GParticleSystemVertexDeclaration;

/**
 * The particle system dynamic parameter vertex declaration resource type.
 */
class FParticleSystemDynamicParameterVertexDeclaration : public FParticleSystemVertexDeclaration
{
public:
	// Destructor.
	virtual ~FParticleSystemDynamicParameterVertexDeclaration() {}

	virtual void FillDeclElements(FVertexDeclarationElementList& Elements, INT& Offset)
	{
		FParticleSystemVertexDeclaration::FillDeclElements(Elements, Offset);
		/** The stream to read the dynamic parameter from.	*/
		Elements.AddItem(FVertexElement(0,Offset,VET_Float4,VEU_TextureCoordinate,3));
		Offset += sizeof(FLOAT) * 4;
	}
};

/** The simple element vertex declaration. */
static TGlobalResource<FParticleSystemDynamicParameterVertexDeclaration> GParticleSystemDynamicParameterVertexDeclaration;

UBOOL FParticleVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithParticleSprites() && !Material->GetUsesDynamicParameter()) || Material->IsSpecialEngineMaterial();
}

/**
 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
 */
void FParticleVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("PARTICLES_ALLOW_AXIS_ROTATION"),TEXT("1"));
	// There are only 2 slots required for the axis rotation vectors.
	OutEnvironment.Definitions.Set(TEXT("NUM_AXIS_ROTATION_VECTORS"),TEXT("2"));
}

/**
 *	Initialize the Render Hardare Interface for this vertex factory
 */
void FParticleVertexFactory::InitRHI()
{
	SetDeclaration(GParticleSystemVertexDeclaration.VertexDeclarationRHI);
}

/**
 * Should we cache the material's shadertype on this platform with this vertex factory? 
 */
UBOOL FParticleDynamicParameterVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (Material->IsUsedWithParticleSprites() && Material->GetUsesDynamicParameter()) || Material->IsSpecialEngineMaterial();
}

/**
 * Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
 */
void FParticleDynamicParameterVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
{
	FParticleVertexFactory::ModifyCompilationEnvironment(Platform, OutEnvironment);
	OutEnvironment.Definitions.Set(TEXT("USE_DYNAMIC_PARAMETERS"),TEXT("1"));
}

/**
 *	
 */
void FParticleDynamicParameterVertexFactory::InitRHI()
{
	SetDeclaration(GParticleSystemDynamicParameterVertexDeclaration.VertexDeclarationRHI);
}

//
void FParticleVertexFactoryShaderParameters::Set(FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView& View) const
{
	FParticleVertexFactory* ParticleVF = (FParticleVertexFactory*)VertexFactory;

	FVector4 CameraRight, CameraUp;
	FVector UpRightScalarParam(0.0f);

	const FVector4 TranslatedViewOrigin = View.ViewOrigin + FVector4(View.PreViewTranslation,0);
	SetVertexShaderValue(VertexShader->GetVertexShader(),CameraWorldPositionParameter,FVector4(TranslatedViewOrigin, 0.0f));

	BYTE LockAxisFlag = ParticleVF->GetLockAxisFlag();
	if (LockAxisFlag == EPAL_NONE)
	{
		CameraUp	= -View.InvViewProjectionMatrix.TransformNormal(FVector(1.0f,0.0f,0.0f)).SafeNormal();
		CameraRight	= -View.InvViewProjectionMatrix.TransformNormal(FVector(0.0f,1.0f,0.0f)).SafeNormal();

		if (ParticleVF->GetScreenAlignment() == PSA_Velocity)
		{
			UpRightScalarParam.Y = 1.0f;
		}
		else
		{
			UpRightScalarParam.X = 1.0f;
		}
	}
	else
	if ((LockAxisFlag >= EPAL_ROTATE_X) && (LockAxisFlag <= EPAL_ROTATE_Z))
	{
		switch (LockAxisFlag)
		{
		case EPAL_ROTATE_X:
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 1.0f, 0.0f, 0.0f, 1.0f), 0);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 0.0f, 0.0f, 0.0f, 0.0f), 1);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorSourceIndexParameter, 0.0f);
			UpRightScalarParam.Z = 1.0f;
			break;
		case EPAL_ROTATE_Y:
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 0.0f, 1.0f, 0.0f, 1.0f), 0);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 0.0f, 0.0f, 0.0f, 0.0f), 1);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorSourceIndexParameter, 0.0f);
			UpRightScalarParam.Z = 1.0f;
			break;
		case EPAL_ROTATE_Z:
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 0.0f, 0.0f, 0.0f, 0.0f), 0);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorsArrayParameter, FVector4( 0.0f, 0.0f,-1.0f,-1.0f), 1);
			SetVertexShaderValue(VertexShader->GetVertexShader(), AxisRotationVectorSourceIndexParameter, 1.0f);
			UpRightScalarParam.Z = 1.0f;
			break;
		}
	}
	else
	{
		CameraUp	= FVector4(ParticleVF->GetLockAxisUp(), 0.0f);
		CameraRight	= FVector4(ParticleVF->GetLockAxisRight(), 0.0f);
		UpRightScalarParam.X = 1.0f;
	}

	SetVertexShaderValue(VertexShader->GetVertexShader(),CameraRightParameter,CameraRight);
	SetVertexShaderValue(VertexShader->GetVertexShader(),CameraUpParameter,CameraUp);
	SetVertexShaderValue(VertexShader->GetVertexShader(),ScreenAlignmentParameter,FVector4((FLOAT)ParticleVF->GetScreenAlignment(),0.0f,0.0f,0.0f));
	SetVertexShaderValue(VertexShader->GetVertexShader(), ParticleUpRightResultScalarsParameter, UpRightScalarParam);
}

void FParticleVertexFactoryShaderParameters::SetMesh(FShader* VertexShader,const FMeshElement& Mesh,const FSceneView& View) const
{
	SetVertexShaderValue(
		VertexShader->GetVertexShader(),
		LocalToWorldParameter,
		Mesh.LocalToWorld.ConcatTranslation(View.PreViewTranslation)
		);
}

IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleVertexFactory,FParticleVertexFactoryShaderParameters,"ParticleSpriteVertexFactory",TRUE,FALSE,FALSE, VER_CONTENT_RESAVE_AUGUST_2007_QA_BUILD,0);
IMPLEMENT_VERTEX_FACTORY_TYPE(FParticleDynamicParameterVertexFactory,FParticleVertexFactoryShaderParameters,"ParticleSpriteVertexFactory",TRUE,FALSE,FALSE, VER_DYNAMICPARAMETERS_ADDED,0);
