/*=============================================================================
	MaterialShared.cpp: Shared material implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "EnginePhysicsClasses.h"

IMPLEMENT_CLASS(UMaterialInterface);
IMPLEMENT_CLASS(AMaterialInstanceActor);


FArchive& operator<<(FArchive& Ar, FMaterial::FTextureLookup& Ref)
{
	Ref.Serialize( Ar );
	return Ar;
}

FMaterialViewRelevance UMaterialInterface::GetViewRelevance()
{
	// Find the interface's concrete material.
	const UMaterial* Material = GetMaterial();
	if(Material)
	{
		const UBOOL bIsTranslucent = IsTranslucentBlendMode((EBlendMode)Material->BlendMode);
		// Determine the material's view relevance.
		FMaterialViewRelevance MaterialViewRelevance;
		MaterialViewRelevance.bOpaque = !bIsTranslucent;
		MaterialViewRelevance.bMasked = Material->bIsMasked;
		MaterialViewRelevance.bTranslucency = bIsTranslucent;
		MaterialViewRelevance.bDistortion = Material->HasDistortion();
		MaterialViewRelevance.bOneLayerDistortionRelevance = bIsTranslucent && Material->bUseOneLayerDistortion;
		MaterialViewRelevance.bLit = Material->LightingModel != MLM_Unlit;
		MaterialViewRelevance.bUsesSceneColor = Material->UsesSceneColor();
		return MaterialViewRelevance;
	}
	else
	{
		return FMaterialViewRelevance();
	}
}

INT UMaterialInterface::GetWidth() const
{
	return ME_PREV_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

INT UMaterialInterface::GetHeight() const
{
	return ME_PREV_THUMBNAIL_SZ+ME_CAPTION_HEIGHT+(ME_STD_BORDER*2);
}

/**
 * Forces the streaming system to disregard the normal logic for the specified duration and
 * instead always load all mip-levels for all textures used by this material.
 *
 * @param ForceDuration	- Number of seconds to keep all mip-levels in memory, disregarding the normal priority logic.
 */
void UMaterialInterface::SetForceMipLevelsToBeResident( float ForceDuration )
{
	TArray<UTexture*> Textures;
	GetTextures( Textures );
	for ( INT TextureIndex=0; TextureIndex < Textures.Num(); ++TextureIndex )
	{
		UTexture2D* Texture = Cast<UTexture2D>(Textures(TextureIndex));
		if ( Texture )
		{
			Texture->SetForceMipLevelsToBeResident( ForceDuration );
		}
	}
}

UBOOL UMaterialInterface::IsReadyForFinishDestroy()
{
	UBOOL bIsReady = Super::IsReadyForFinishDestroy();
	bIsReady = bIsReady && ParentRefFence.GetNumPendingFences() == 0; 
	return bIsReady;
}

void UMaterialInterface::BeginDestroy()
{
	ParentRefFence.BeginFence();

	Super::BeginDestroy();
}

//
//	UMaterialInterface::execGetMaterial
//
void UMaterialInterface::execGetMaterial(FFrame& Stack,RESULT_DECL)
{
	P_FINISH;
	*(UMaterial**) Result = GetMaterial();
}

void UMaterialInterface::execGetPhysicalMaterial(FFrame& Stack,RESULT_DECL)
{
	P_FINISH;
	*(UPhysicalMaterial**) Result = GetPhysicalMaterial();
}

UBOOL UMaterialInterface::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInterface::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInterface::GetScalarCurveParameterValue(FName ParameterName, FInterpCurveFloat& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInterface::GetVectorCurveParameterValue(FName ParameterName, FInterpCurveVector& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInterface::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	return FALSE;
}

UBOOL UMaterialInterface::GetFontParameterValue(FName ParameterName,class UFont*& OutFontValue,INT& OutFontPage)
{
	return FALSE;
}

TLinkedList<FMaterialUniformExpressionType*>*& FMaterialUniformExpressionType::GetTypeList()
{
	static TLinkedList<FMaterialUniformExpressionType*>* TypeList = NULL;
	return TypeList;
}

TMap<FName,FMaterialUniformExpressionType*>& FMaterialUniformExpressionType::GetTypeMap()
{
	static TMap<FName,FMaterialUniformExpressionType*> TypeMap;

	// Move types from the type list to the type map.
	TLinkedList<FMaterialUniformExpressionType*>* TypeListLink = GetTypeList();
	while(TypeListLink)
	{
		TLinkedList<FMaterialUniformExpressionType*>* NextLink = TypeListLink->Next();
		FMaterialUniformExpressionType* Type = **TypeListLink;

		TypeMap.Set(FName(Type->Name),Type);
		TypeListLink->Unlink();
		delete TypeListLink;

		TypeListLink = NextLink;
	}

	return TypeMap;
}

FMaterialUniformExpressionType::FMaterialUniformExpressionType(
	const TCHAR* InName,
	SerializationConstructorType InSerializationConstructor
	):
	Name(InName),
	SerializationConstructor(InSerializationConstructor)
{
	// Put the type in the type list until the name subsystem/type map are initialized.
	(new TLinkedList<FMaterialUniformExpressionType*>(this))->Link(GetTypeList());
}

FArchive& operator<<(FArchive& Ar,FMaterialUniformExpression*& Ref)
{
	// Serialize the expression type.
	if(Ar.IsSaving())
	{
		// Write the type name.
		check(Ref);
		FName TypeName(Ref->GetType()->Name);
		Ar << TypeName;
	}
	else if(Ar.IsLoading())
	{
		// Read the type name.
		FName TypeName = NAME_None;
		Ar << TypeName;

		// Find the expression type with a matching name.
		FMaterialUniformExpressionType* Type = FMaterialUniformExpressionType::GetTypeMap().FindRef(TypeName);
		check(Type);

		// Construct a new instance of the expression type.
		Ref = (*Type->SerializationConstructor)();
	}

	// Serialize the expression.
	Ref->Serialize(Ar);

	return Ar;
}

INT FMaterialCompiler::Errorf(const TCHAR* Format,...)
{
	TCHAR	ErrorText[2048];
	GET_VARARGS( ErrorText, ARRAY_COUNT(ErrorText), ARRAY_COUNT(ErrorText)-1, Format, Format );
	return Error(ErrorText);
}

//
//	FExpressionInput::Compile
//

INT FExpressionInput::Compile(FMaterialCompiler* Compiler)
{
	if(Expression)
	{
		if(Mask)
		{
			INT ExpressionResult = Compiler->CallExpression(Expression,Compiler);
			if(ExpressionResult != INDEX_NONE)
			{
				return Compiler->ComponentMask(
					ExpressionResult,
					MaskR,MaskG,MaskB,MaskA
					);
			}
			else
			{
				return INDEX_NONE;
			}
		}
		else
		{
			return Compiler->CallExpression(Expression,Compiler);
		}
	}
	else
		return INDEX_NONE;
}

//
//	FColorMaterialInput::Compile
//

INT FColorMaterialInput::Compile(FMaterialCompiler* Compiler,const FColor& Default)
{
	if(UseConstant)
	{
		FLinearColor	LinearColor(Constant);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
	else if(Expression)
	{
		INT ResultIndex = FExpressionInput::Compile(Compiler);
		if (ResultIndex == INDEX_NONE)
		{
			FLinearColor	LinearColor(Default);
			return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
		}
		else
		{
			return ResultIndex;
		}
	}
	else
	{
		FLinearColor	LinearColor(Default);
		return Compiler->Constant3(LinearColor.R,LinearColor.G,LinearColor.B);
	}
}

//
//	FScalarMaterialInput::Compile
//

INT FScalarMaterialInput::Compile(FMaterialCompiler* Compiler,FLOAT Default)
{
	if(UseConstant)
	{
		return Compiler->Constant(Constant);
	}
	else if(Expression)
	{
		INT ResultIndex = FExpressionInput::Compile(Compiler);
		if (ResultIndex == INDEX_NONE)
		{
			return Compiler->Constant(Default);
		}
		else
		{
			return ResultIndex;
		}
	}
	else
	{
		return Compiler->Constant(Default);
	}
}

//
//	FVectorMaterialInput::Compile
//

INT FVectorMaterialInput::Compile(FMaterialCompiler* Compiler,const FVector& Default)
{
	if(UseConstant)
	{
		return Compiler->Constant3(Constant.X,Constant.Y,Constant.Z);
	}
	else if(Expression)
	{
		INT ResultIndex = FExpressionInput::Compile(Compiler);
		if (ResultIndex == INDEX_NONE)
		{
			return Compiler->Constant3(Default.X,Default.Y,Default.Z);
		}
		else
		{
			return ResultIndex;
		}
	}
	else
	{
		return Compiler->Constant3(Default.X,Default.Y,Default.Z);
	}
}

//
//	FVector2MaterialInput::Compile
//

INT FVector2MaterialInput::Compile(FMaterialCompiler* Compiler,const FVector2D& Default)
{
	if(UseConstant)
	{
		return Compiler->Constant2(Constant.X,Constant.Y);
	}
	else if(Expression)
	{
		INT ResultIndex = FExpressionInput::Compile(Compiler);
		if (ResultIndex == INDEX_NONE)
		{
			return Compiler->Constant2(Default.X,Default.Y);
		}
		else
		{
			return ResultIndex;
		}
	}
	else
	{
		return Compiler->Constant2(Default.X,Default.Y);
	}
}

EMaterialValueType GetMaterialPropertyType(EMaterialProperty Property)
{
	switch(Property)
	{
	case MP_EmissiveColor: return MCT_Float3;
	case MP_Opacity: return MCT_Float;
	case MP_OpacityMask: return MCT_Float;
	case MP_Distortion: return MCT_Float2;
	case MP_TwoSidedLightingMask: return MCT_Float3;
	case MP_DiffuseColor: return MCT_Float3;
	case MP_DiffusePower: return MCT_Float;
	case MP_SpecularColor: return MCT_Float3;
	case MP_SpecularPower: return MCT_Float;
	case MP_Normal: return MCT_Float3;
	case MP_CustomLighting: return MCT_Float3;
	};
	return MCT_Unknown;
}

/**
 * Null any material expression references for this material
 */
void FMaterial::RemoveExpressions()
{
	TextureDependencyLengthMap.Empty();
}

void FMaterial::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	for(TMap<UMaterialExpression*,INT>::TConstIterator DependencyLengthIt(TextureDependencyLengthMap);
		DependencyLengthIt;
		++DependencyLengthIt)
	{
		UObject::AddReferencedObject(ObjectArray,DependencyLengthIt.Key());
	}
}

void FMaterial::Serialize(FArchive& Ar)
{
	Ar << CompileErrors;
	// Cook out editor-only texture dependency map information.
	if( Ar.IsSaving() && (GCookingTarget & UE3::PLATFORM_Console) )
	{
		TMap<UMaterialExpression*,INT> DummyMap;
		Ar << DummyMap;
	}
	else
	{
		Ar << TextureDependencyLengthMap;
	}
	Ar << MaxTextureDependencyLength;
	Ar << Id;
	Ar << NumUserTexCoords;
	Ar << UniformVectorExpressions;
	Ar << UniformScalarExpressions;
	Ar << Uniform2DTextureExpressions;
	Ar << UniformCubeTextureExpressions;
	Ar << bUsesSceneColor;
	Ar << bUsesSceneDepth;
	if (Ar.Ver() >= VER_DYNAMICPARAMETERS_ADDED)
	{
		Ar << bUsesDynamicParameter;
	}
	Ar << UsingTransforms;
	
	if(Ar.Ver() >= VER_MIN_COMPILEDMATERIAL && Ar.LicenseeVer() >= LICENSEE_VER_MIN_COMPILEDMATERIAL)
	{
		bValidCompilationOutput = TRUE;
	}
	
	Ar << TextureLookups;
	Ar << DroppedFallbackComponents;
}

/** Initializes the material's shader map. */
UBOOL FMaterial::InitShaderMap(EShaderPlatform Platform, UBOOL bCompileFallback)
{
	FStaticParameterSet EmptySet(Id);
	return InitShaderMap(&EmptySet, Platform, bCompileFallback);
}

/** Initializes the material's shader map. */
UBOOL FMaterial::InitShaderMap(FStaticParameterSet * StaticParameters, EShaderPlatform Platform, UBOOL bCompileFallback)
{
	UBOOL bSucceeded = FALSE;
	if(!Id.IsValid())
	{
		// If the material doesn't have a valid ID, create a new one.
		// This can happen if it is loaded from an old package or newly created and hasn't had CacheShaders() called yet.
		Id = appCreateGuid();
	}

	//only update the static parameter set's Id if it isn't valid
	if(!StaticParameters->BaseMaterialId.IsValid())
	{
		StaticParameters->BaseMaterialId = Id;
	}

#if CONSOLE
	if( !GUseSeekFreeLoading )
#endif
	{
		// make sure the shader cache is loaded
		GetLocalShaderCache(Platform);
	}

	// Find the material's cached shader map.
	ShaderMap = FMaterialShaderMap::FindId(*StaticParameters, Platform);
	if(!bValidCompilationOutput || !ShaderMap || !ShaderMap->IsComplete(this, TRUE))
	{
		if(bValidCompilationOutput)
		{
			const TCHAR* ShaderMapCondition;
			if(ShaderMap)
			{
				ShaderMapCondition = TEXT("Incomplete");
			}
			else
			{
				ShaderMapCondition = TEXT("Missing");
			}
			debugf(TEXT("%s cached shader map for material %s"),ShaderMapCondition,*GetFriendlyName());
		}
		else
		{
			debugf(TEXT("Material %s has outdated uniform expressions; regenerating."),*GetFriendlyName());
		}

#if CONSOLE
		if( GUseSeekFreeLoading )
		{
			if (IsSpecialEngineMaterial())
			{
				//assert if the default material's shader map was not found, since it will cause problems later
				appErrorf(TEXT("Failed to find shader map for default material %s!  Please make sure cooking was successful."), *GetFriendlyName());
			}
			else
			{
				debugf(TEXT("Can't compile %s with seekfree loading path on console, will attempt to use default material instead"), *GetFriendlyName());
			}
			// Reset the shader map so the default material will be used.
			ShaderMap = NULL;
		}
		else
#endif
		{
			// If there's no cached shader map for this material, compile a new one.
			// This will only happen if the local shader cache happens to get out of sync with a modified material package, which should only
			// happen in exceptional cases; the editor crashed between saving the shader cache and the modified material package, etc.
			bSucceeded = Compile(StaticParameters, Platform, ShaderMap, FALSE, bCompileFallback);
			if(!bSucceeded)
			{
				// If it failed to compile the material, reset the shader map so the material isn't used.
				ShaderMap = NULL;
				if (IsSpecialEngineMaterial())
				{
					// Assert if the default material could not be compiled, since there will be nothing for other failed materials to fall back on.
					appErrorf(TEXT("Failed to compile default material %s!"), *GetFriendlyName());
				}
			}
		}
	}
	else
	{
		ShaderMap->BeginInit();
		bSucceeded = TRUE;
	}
	return bSucceeded;
}

/** Flushes this material's shader map from the shader cache if it exists. */
void FMaterial::FlushShaderMap()
{
	if(ShaderMap)
	{
		// If there's a shader map for this material, flush any shaders cached for it.
		UShaderCache::FlushId(ShaderMap->GetMaterialId());
		ShaderMap = NULL;
	}

}

UBOOL IsTranslucentBlendMode(EBlendMode BlendMode)
{
	return BlendMode != BLEND_Opaque && BlendMode != BLEND_Masked;
}

UBOOL FMaterialResource::IsTwoSided() const { return Material->TwoSided; }
UBOOL FMaterialResource::NeedsDepthTestDisabled() const { return Material->bDisableDepthTest; }
UBOOL FMaterialResource::AllowsFog() const { return Material->bAllowFog; }
UBOOL FMaterialResource::UsesOneLayerDistortion() const { return Material->bUseOneLayerDistortion && IsTranslucentBlendMode((EBlendMode)Material->BlendMode); }
UBOOL FMaterialResource::IsWireframe() const { return Material->Wireframe; }
UBOOL FMaterialResource::IsUsedWithFogVolumes() const { return Material->bUsedWithFogVolumes; }
UBOOL FMaterialResource::IsLightFunction() const { return Material->bUsedAsLightFunction; }
UBOOL FMaterialResource::IsSpecialEngineMaterial() const { return Material->bUsedAsSpecialEngineMaterial; }
UBOOL FMaterialResource::IsTerrainMaterial() const { return FALSE; }

UBOOL FMaterialResource::IsDecalMaterial() const
{
	return FALSE;
}

UBOOL FMaterialResource::IsUsedWithSkeletalMesh() const
{
	return Material->bUsedWithSkeletalMesh;
}

UBOOL FMaterialResource::IsUsedWithFracturedMeshes() const
{
	return Material->bUsedWithFracturedMeshes;
}

UBOOL FMaterialResource::IsUsedWithSpeedTree() const
{
	return Material->bUsedWithSpeedTree;
}

UBOOL FMaterialResource::IsUsedWithParticleSystem() const
{
	return Material->bUsedWithParticleSprites || Material->bUsedWithBeamTrails || Material->bUsedWithParticleSubUV;
}

UBOOL FMaterialResource::IsUsedWithParticleSprites() const
{
	return Material->bUsedWithParticleSprites;
}

UBOOL FMaterialResource::IsUsedWithBeamTrails() const
{
	return Material->bUsedWithBeamTrails;
}

UBOOL FMaterialResource::IsUsedWithParticleSubUV() const
{
	return Material->bUsedWithParticleSubUV;
}

UBOOL FMaterialResource::IsUsedWithFoliage() const
{
	return Material->bUsedWithFoliage;
}

UBOOL FMaterialResource::IsUsedWithStaticLighting() const
{
	return Material->bUsedWithStaticLighting;
}

UBOOL FMaterialResource::IsUsedWithLensFlare() const
{
	return Material->bUsedWithLensFlare;
}

UBOOL FMaterialResource::IsUsedWithGammaCorrection() const
{
	return Material->bUsedWithGammaCorrection;
}

UBOOL FMaterialResource::IsUsedWithInstancedMeshParticles() const
{
	return Material->bUsedWithInstancedMeshParticles;
}

UBOOL FMaterialResource::IsUsedWithFluidSurfaces() const
{
	return Material->bUsedWithFluidSurfaces;
}

UBOOL FMaterialResource::IsUsedWithMaterialEffect() const
{
	return Material->bUsedWithMaterialEffect;
}

UBOOL FMaterialResource::IsUsedWithDecals() const
{
	return Material->bUsedWithDecals;
}

/**
 * Should shaders compiled for this material be saved to disk?
 */
UBOOL FMaterialResource::IsPersistent() const { return TRUE; }

EBlendMode FMaterialResource::GetBlendMode() const { return (EBlendMode)Material->BlendMode; }

EMaterialLightingModel FMaterialResource::GetLightingModel() const { return (EMaterialLightingModel)Material->LightingModel; }

FLOAT FMaterialResource::GetOpacityMaskClipValue() const { return Material->OpacityMaskClipValue; }

/**
* Check for distortion use
* @return TRUE if material uses distoriton
*/
UBOOL FMaterialResource::IsDistorted() const { return Material->bUsesDistortion && !Material->bUseOneLayerDistortion; }

/**
 * Check if the material is masked and uses an expression or a constant that's not 1.0f for opacity.
 * @return TRUE if the material uses opacity
 */
UBOOL FMaterialResource::IsMasked() const { return Material->bIsMasked; }

/** @return TRUE if the material has few enough samplers to support an SH light in the base pass */
UBOOL FMaterialResource::SupportsSinglePassSHLight() const { return Material->bSupportsSinglePassSHLight; }

FString FMaterialResource::GetFriendlyName() const { return *Material->GetName(); }

/** Allows the resource to do things upon compile. */
UBOOL FMaterialResource::Compile( FStaticParameterSet* StaticParameters, EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile, UBOOL bCompileFallback, UBOOL bDebugDump)
{
	UBOOL bOk;
	STAT(DOUBLE MaterialCompilingTime = 0);
	{
		SCOPE_SECONDS_COUNTER(MaterialCompilingTime);
		check(Material);

		const INT NumSamplersUsed = GetSamplerUsage();
		Material->bSupportsSinglePassSHLight = NumSamplersUsed <= MAX_SHLIGHT_BASEPASS_SAMPLERS;

		bOk = FMaterial::Compile( StaticParameters, Platform, OutShaderMap, bForceCompile, bCompileFallback, bDebugDump);
		if ( bOk )
		{
			RebuildTextureLookupInfo( Material );
		}
	}
	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_MaterialShaders,(FLOAT)MaterialCompilingTime);
	return bOk;
}

/**
 * Gets instruction counts that best represent the likely usage of this material based on lighting model and other factors.
 * @param Descriptions - an array of descriptions to be populated
 * @param InstructionCounts - an array of instruction counts matching the Descriptions.  
 *		The dimensions of these arrays are guaranteed to be identical and all values are valid.
 */
void FMaterialResource::GetRepresentativeInstructionCounts(TArray<FString> &Descriptions, TArray<INT> &InstructionCounts) const
{
	TArray<FString> ShaderTypeNames;
	TArray<FString> ShaderTypeDescriptions;

	//when adding a shader type here be sure to update FPreviewMaterial::ShouldCache()
	//so the shader type will get compiled with preview materials
	const FMaterialShaderMap* MaterialShaderMap = GetShaderMap();
	if (MaterialShaderMap)
	{
		if (IsUsedWithFogVolumes())
		{
			new (ShaderTypeNames) FString(TEXT("FFogVolumeApplyPixelShader"));
			new (ShaderTypeDescriptions) FString(TEXT("Fog Volume Apply Pixel Shader"));
		}
		else
		{
			if (GetLightingModel() == MLM_Phong)
			{
				//lit materials are usually lightmapped
				new (ShaderTypeNames) FString(TEXT("TBasePassPixelShaderFDirectionalLightMapTexturePolicyNoSkyLight"));
				new (ShaderTypeDescriptions) FString(TEXT("Emissive pixel shader with light map"));

				//also show a dynamically lit shader
				new (ShaderTypeNames) FString(TEXT("TLightPixelShaderFPointLightPolicyFNoStaticShadowingPolicy"));
				new (ShaderTypeDescriptions) FString(TEXT("Point light pixel shader"));
			}
			else
			{
				//unlit materials are never lightmapped
				new (ShaderTypeNames) FString(TEXT("TBasePassPixelShaderFNoLightMapPolicyNoSkyLight"));
				new (ShaderTypeDescriptions) FString(TEXT("Emissive pixel shader without light map"));
			}

			if (IsDistorted())
			{
				//distortion requires an extra pass
				new (ShaderTypeNames) FString(TEXT("TDistortionMeshPixelShader<FDistortMeshAccumulatePolicy>"));
				new (ShaderTypeDescriptions) FString(TEXT("Distortion pixel shader"));
			}
		}

		const FMeshMaterialShaderMap* MeshShaderMap = MaterialShaderMap->GetMeshShaderMap(&FLocalVertexFactory::StaticType);
		if (MeshShaderMap)
		{
			Descriptions.Empty();
			InstructionCounts.Empty();

			for (INT InstructionIndex = 0; InstructionIndex < ShaderTypeNames.Num(); InstructionIndex++)
			{
				FShaderType* ShaderType = FindShaderTypeByName(*ShaderTypeNames(InstructionIndex));
				if (ShaderType)
				{
					const FShader* Shader = MeshShaderMap->GetShader(ShaderType);
					if (Shader)
					{
						//if the shader was found, add it to the output arrays
						InstructionCounts.Push(Shader->GetNumInstructions());
						Descriptions.Push(ShaderTypeDescriptions(InstructionIndex));
					}
				}
			}
		}
	}

	check(Descriptions.Num() == InstructionCounts.Num());
}

/** @return the number of components in a vector type. */
UINT GetNumComponents(EMaterialValueType Type)
{
	switch(Type)
	{
		case MCT_Float:
		case MCT_Float1: return 1;
		case MCT_Float2: return 2;
		case MCT_Float3: return 3;
		case MCT_Float4: return 4;
		default: return 0;
	}
}

/** @return the vector type containing a given number of components. */
EMaterialValueType GetVectorType(UINT NumComponents)
{
	switch(NumComponents)
	{
		case 1: return MCT_Float;
		case 2: return MCT_Float2;
		case 3: return MCT_Float3;
		case 4: return MCT_Float4;
		default: return MCT_Unknown;
	};
}

/**
 */
class FMaterialUniformExpressionConstant: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionConstant);
public:
	FMaterialUniformExpressionConstant() {}
	FMaterialUniformExpressionConstant(const FLinearColor& InValue,BYTE InValueType):
		Value(InValue),
		ValueType(InValueType)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Value << ValueType;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue = Value;
	}
	virtual UBOOL IsConstant() const
	{
		return TRUE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionConstant* OtherConstant = (FMaterialUniformExpressionConstant*)OtherExpression;
		return OtherConstant->ValueType == ValueType && OtherConstant->Value == Value;
	}

private:
	FLinearColor Value;
	BYTE ValueType;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionConstant);

/**
 * A texture expression.
 */
class FMaterialUniformExpressionTexture: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTexture);
public:

	FMaterialUniformExpressionTexture() {}
	FMaterialUniformExpressionTexture(UTexture* InValue):
		Value(InValue)
	{
		if (!Value)
		{
			appDebugBreak();
		}
	}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Value;
		if (Ar.IsLoading() && !Value)
		{
			//if a null texture reference was saved, use the default texture
			Value = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.DefaultTexture"), NULL, LOAD_None, NULL);
		}
	}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const
	{
		*OutValue = Value ? Value->Resource : NULL;
	}
	virtual void GetGameThreadTextureValue(UMaterialInterface* MaterialInterface,UTexture** OutValue) const
	{
		*OutValue = Value;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionTexture* OtherParameter = (FMaterialUniformExpressionTexture*)OtherExpression;
		return Value == OtherParameter->Value;
	}

private:
	UTexture* Value;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTexture);

/**
 */
class FMaterialUniformExpressionTime: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTime);
public:

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = Context.CurrentTime;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		return TRUE;
	}
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTime);

/**
 */
class FMaterialUniformExpressionRealTime: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionRealTime);
public:

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = Context.CurrentRealTime;
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		return TRUE;
	}
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionRealTime);

/**
 */
class FMaterialUniformExpressionVectorParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionVectorParameter);
public:

	FMaterialUniformExpressionVectorParameter() {}
	FMaterialUniformExpressionVectorParameter(FName InParameterName,const FLinearColor& InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if(!Context.MaterialRenderProxy->GetVectorValue(ParameterName, &OutValue, Context))
		{
			OutValue = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionVectorParameter* OtherParameter = (FMaterialUniformExpressionVectorParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	FLinearColor DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionVectorParameter);

/**
 */
class FMaterialUniformExpressionScalarParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionScalarParameter);
public:

	FMaterialUniformExpressionScalarParameter() {}
	FMaterialUniformExpressionScalarParameter(FName InParameterName,FLOAT InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if(!Context.MaterialRenderProxy->GetScalarValue(ParameterName, &OutValue.R, Context))
		{
			OutValue.R = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionScalarParameter* OtherParameter = (FMaterialUniformExpressionScalarParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	FLOAT DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionScalarParameter);

/**
 * A texture parameter expression.
 */
class FMaterialUniformExpressionTextureParameter: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTextureParameter);
public:

	FMaterialUniformExpressionTextureParameter() {}
	FMaterialUniformExpressionTextureParameter(FName InParameterName,UTexture* InDefaultValue):
		ParameterName(InParameterName),
		DefaultValue(InDefaultValue)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << ParameterName << DefaultValue;
	}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const
	{
		*OutValue = NULL;
		if(!Context.MaterialRenderProxy->GetTextureValue(ParameterName,OutValue,Context) && DefaultValue)
		{
			*OutValue = DefaultValue->Resource;
		}
	}
	virtual void GetGameThreadTextureValue(UMaterialInterface* MaterialInterface,UTexture** OutValue) const
	{
		*OutValue = NULL;
		if(!MaterialInterface->GetTextureParameterValue(ParameterName,*OutValue))
		{
			*OutValue = DefaultValue;
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionTextureParameter* OtherParameter = (FMaterialUniformExpressionTextureParameter*)OtherExpression;
		return ParameterName == OtherParameter->ParameterName && DefaultValue == OtherParameter->DefaultValue;
	}

private:
	FName ParameterName;
	UTexture* DefaultValue;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionTextureParameter);

/**
 * A flipbook texture parameter expression.
 */
class FMaterialUniformExpressionFlipBookTextureParameter : public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFlipBookTextureParameter);
public:

	FMaterialUniformExpressionFlipBookTextureParameter() {}
	FMaterialUniformExpressionFlipBookTextureParameter(UTextureFlipBook* InTexture) :
		FlipBookTexture(InTexture)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << FlipBookTexture;
	}

	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if (FlipBookTexture)
		{
			FlipBookTexture->GetTextureOffset_RenderThread(OutValue);
		}
	}
	virtual UBOOL IsConstant() const
	{
		return FALSE;
	}

	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFlipBookTextureParameter* OtherParameter = (FMaterialUniformExpressionFlipBookTextureParameter*)OtherExpression;
		return (FlipBookTexture == OtherParameter->FlipBookTexture);
	}

private:
	UTextureFlipBook* FlipBookTexture;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFlipBookTextureParameter);

/**
 */
class FMaterialUniformExpressionSine: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSine);
public:

	FMaterialUniformExpressionSine() {}
	FMaterialUniformExpressionSine(FMaterialUniformExpression* InX,UBOOL bInIsCosine):
		X(InX),
		bIsCosine(bInIsCosine)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X << bIsCosine;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueX = FLinearColor::Black;
		X->GetNumberValue(Context,ValueX);
		OutValue.R = bIsCosine ? appCos(ValueX.R) : appSin(ValueX.R);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionSine* OtherSine = (FMaterialUniformExpressionSine*)OtherExpression;
		return X == OtherSine->X && bIsCosine == OtherSine->bIsCosine;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
	UBOOL bIsCosine;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSine);

/**
 */
class FMaterialUniformExpressionSquareRoot: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSquareRoot);
public:

	FMaterialUniformExpressionSquareRoot() {}
	FMaterialUniformExpressionSquareRoot(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueX = FLinearColor::Black;
		X->GetNumberValue(Context,ValueX);
		OutValue.R = appSqrt(ValueX.R);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionSquareRoot* OtherSqrt = (FMaterialUniformExpressionSquareRoot*)OtherExpression;
		return X == OtherSqrt->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionSquareRoot);

/**
 */
enum EFoldedMathOperation
{
	FMO_Add,
	FMO_Sub,
	FMO_Mul,
	FMO_Div,
	FMO_Dot
};

/** Converts an arbitrary number into a safe divisor. i.e. Abs(Number) >= DELTA */
static FLOAT GetSafeDivisor(FLOAT Number)
{
	if(Abs(Number) < DELTA)
	{
		if(Number < 0.0f)
		{
			return -DELTA;
		}
		else
		{
			return +DELTA;
		}
	}
	else
	{
		return Number;
	}
}

class FMaterialUniformExpressionFoldedMath: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFoldedMath);
public:

	FMaterialUniformExpressionFoldedMath() {}
	FMaterialUniformExpressionFoldedMath(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB,BYTE InOp):
		A(InA),
		B(InB),
		Op(InOp)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B << Op;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueA = FLinearColor::Black;
		FLinearColor ValueB = FLinearColor::Black;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		switch(Op)
		{
			case FMO_Add: OutValue = ValueA + ValueB; break;
			case FMO_Sub: OutValue = ValueA - ValueB; break;
			case FMO_Mul: OutValue = ValueA * ValueB; break;
			case FMO_Div: 
				OutValue.R = ValueA.R / GetSafeDivisor(ValueB.R);
				OutValue.G = ValueA.G / GetSafeDivisor(ValueB.G);
				OutValue.B = ValueA.B / GetSafeDivisor(ValueB.B);
				OutValue.A = ValueA.A / GetSafeDivisor(ValueB.A);
				break;
			case FMO_Dot: 
				{
					FLOAT DotProduct = ValueA.R * ValueB.R + ValueA.G * ValueB.G + ValueA.B * ValueB.B + ValueA.A * ValueB.A;
					OutValue.R = OutValue.G = OutValue.B = OutValue.A = DotProduct;
				}
				break;
			default: appErrorf(TEXT("Unknown folded math operation: %08x"),(INT)Op);
		};
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpressionFoldedMath* OtherExpression)
	{
		FMaterialUniformExpressionFoldedMath* OtherMath = (FMaterialUniformExpressionFoldedMath*)OtherExpression;
		return A == OtherMath->A && B == OtherMath->B && Op == OtherMath->Op;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
	BYTE Op;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFoldedMath);

/**
 * A hint that only the fractional part of this expession's value matters.
 */
class FMaterialUniformExpressionPeriodic: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionPeriodic);
public:

	FMaterialUniformExpressionPeriodic() {}
	FMaterialUniformExpressionPeriodic(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor TempValue = FLinearColor::Black;
		X->GetNumberValue(Context,TempValue);

		OutValue.R = TempValue.R - appFloor(TempValue.R);
		OutValue.G = TempValue.G - appFloor(TempValue.G);
		OutValue.B = TempValue.B - appFloor(TempValue.B);
		OutValue.A = TempValue.A - appFloor(TempValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionPeriodic* OtherPeriodic = (FMaterialUniformExpressionPeriodic*)OtherExpression;
		return X == OtherPeriodic->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionPeriodic);

/**
 */
class FMaterialUniformExpressionAppendVector: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAppendVector);
public:

	FMaterialUniformExpressionAppendVector() {}
	FMaterialUniformExpressionAppendVector(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB,UINT InNumComponentsA):
		A(InA),
		B(InB),
		NumComponentsA(InNumComponentsA)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B << NumComponentsA;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueA = FLinearColor::Black;
		FLinearColor ValueB = FLinearColor::Black;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = NumComponentsA >= 1 ? ValueA.R : (&ValueB.R)[0 - NumComponentsA];
		OutValue.G = NumComponentsA >= 2 ? ValueA.G : (&ValueB.R)[1 - NumComponentsA];
		OutValue.B = NumComponentsA >= 3 ? ValueA.B : (&ValueB.R)[2 - NumComponentsA];
		OutValue.A = NumComponentsA >= 4 ? ValueA.A : (&ValueB.R)[3 - NumComponentsA];
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionAppendVector* OtherAppend = (FMaterialUniformExpressionAppendVector*)OtherExpression;
		return A == OtherAppend->A && B == OtherAppend->B && NumComponentsA == OtherAppend->NumComponentsA;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
	UINT NumComponentsA;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAppendVector);

/**
 */
class FMaterialUniformExpressionMin: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMin);
public:

	FMaterialUniformExpressionMin() {}
	FMaterialUniformExpressionMin(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB):
		A(InA),
		B(InB)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueA = FLinearColor::Black;
		FLinearColor ValueB = FLinearColor::Black;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = Min(ValueA.R, ValueB.R);
		OutValue.G = Min(ValueA.G, ValueB.G);
		OutValue.B = Min(ValueA.B, ValueB.B);
		OutValue.A = Min(ValueA.A, ValueB.A);
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionMin* OtherMin = (FMaterialUniformExpressionMin*)OtherExpression;
		return A == OtherMin->A && B == OtherMin->B;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMin);

/**
 */
class FMaterialUniformExpressionMax: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMax);
public:

	FMaterialUniformExpressionMax() {}
	FMaterialUniformExpressionMax(FMaterialUniformExpression* InA,FMaterialUniformExpression* InB):
		A(InA),
		B(InB)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << A << B;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueA = FLinearColor::Black;
		FLinearColor ValueB = FLinearColor::Black;
		A->GetNumberValue(Context, ValueA);
		B->GetNumberValue(Context, ValueB);

		OutValue.R = Max(ValueA.R, ValueB.R);
		OutValue.G = Max(ValueA.G, ValueB.G);
		OutValue.B = Max(ValueA.B, ValueB.B);
		OutValue.A = Max(ValueA.A, ValueB.A);
	}
	virtual UBOOL IsConstant() const
	{
		return A->IsConstant() && B->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionMax* OtherMax = (FMaterialUniformExpressionMax*)OtherExpression;
		return A == OtherMax->A && B == OtherMax->B;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> A;
	TRefCountPtr<FMaterialUniformExpression> B;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionMax);

/**
 */
class FMaterialUniformExpressionClamp: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionClamp);
public:

	FMaterialUniformExpressionClamp() {}
	FMaterialUniformExpressionClamp(FMaterialUniformExpression* InInput,FMaterialUniformExpression* InMin,FMaterialUniformExpression* InMax):
		Input(InInput),
		Min(InMin),
		Max(InMax)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << Input << Min << Max;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		FLinearColor ValueMin = FLinearColor::Black;
		FLinearColor ValueMax = FLinearColor::Black;
		FLinearColor ValueInput = FLinearColor::Black;
		Min->GetNumberValue(Context, ValueMin);
		Max->GetNumberValue(Context, ValueMax);
		Input->GetNumberValue(Context, ValueInput);

		OutValue.R = Clamp(ValueInput.R, ValueMin.R, ValueMax.R);
		OutValue.G = Clamp(ValueInput.G, ValueMin.G, ValueMax.G);
		OutValue.B = Clamp(ValueInput.B, ValueMin.B, ValueMax.B);
		OutValue.A = Clamp(ValueInput.A, ValueMin.A, ValueMax.A);
	}
	virtual UBOOL IsConstant() const
	{
		return Input->IsConstant() && Min->IsConstant() && Max->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionClamp* OtherClamp = (FMaterialUniformExpressionClamp*)OtherExpression;
		return Input == OtherClamp->Input && Min == OtherClamp->Min && Max == OtherClamp->Max;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> Input;
	TRefCountPtr<FMaterialUniformExpression> Min;
	TRefCountPtr<FMaterialUniformExpression> Max;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionClamp);

/**
 */
class FMaterialUniformExpressionFloor: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFloor);
public:

	FMaterialUniformExpressionFloor() {}
	FMaterialUniformExpressionFloor(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = appFloor(OutValue.R);
		OutValue.G = appFloor(OutValue.G);
		OutValue.B = appFloor(OutValue.B);
		OutValue.A = appFloor(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFloor* OtherFloor = (FMaterialUniformExpressionFloor*)OtherExpression;
		return X == OtherFloor->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFloor);

/**
 */
class FMaterialUniformExpressionCeil: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionCeil);
public:

	FMaterialUniformExpressionCeil() {}
	FMaterialUniformExpressionCeil(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = appCeil(OutValue.R);
		OutValue.G = appCeil(OutValue.G);
		OutValue.B = appCeil(OutValue.B);
		OutValue.A = appCeil(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionCeil* OtherCeil = (FMaterialUniformExpressionCeil*)OtherExpression;
		return X == OtherCeil->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionCeil);

/**
 */
class FMaterialUniformExpressionFrac: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFrac);
public:

	FMaterialUniformExpressionFrac() {}
	FMaterialUniformExpressionFrac(FMaterialUniformExpression* InX):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);

		OutValue.R = OutValue.R - appFloor(OutValue.R);
		OutValue.G = OutValue.G - appFloor(OutValue.G);
		OutValue.B = OutValue.B - appFloor(OutValue.B);
		OutValue.A = OutValue.A - appFloor(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionFrac* OtherFrac = (FMaterialUniformExpressionFrac*)OtherExpression;
		return X == OtherFrac->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionFrac);

/**
 * Absolute value evaluator for a given input expression
 */
class FMaterialUniformExpressionAbs: public FMaterialUniformExpression
{
	DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAbs);
public:

	FMaterialUniformExpressionAbs() {}
	FMaterialUniformExpressionAbs( FMaterialUniformExpression* InX ):
		X(InX)
	{}

	// FMaterialUniformExpression interface.
	virtual void Serialize(FArchive& Ar)
	{
		Ar << X;
	}
	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const
	{
		X->GetNumberValue(Context, OutValue);
		OutValue.R = Abs<FLOAT>(OutValue.R);
		OutValue.G = Abs<FLOAT>(OutValue.G);
		OutValue.B = Abs<FLOAT>(OutValue.B);
		OutValue.A = Abs<FLOAT>(OutValue.A);
	}
	virtual UBOOL IsConstant() const
	{
		return X->IsConstant();
	}
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const
	{
		FMaterialUniformExpressionAbs* OtherAbs = (FMaterialUniformExpressionAbs*)OtherExpression;
		return X == OtherAbs->X;
	}

private:
	TRefCountPtr<FMaterialUniformExpression> X;
};
IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(FMaterialUniformExpressionAbs);

struct FShaderCodeChunk
{
	FString Code;
	TRefCountPtr<FMaterialUniformExpression> UniformExpression;
	EMaterialValueType Type;
	DWORD Flags;
	INT TextureDependencyLength;

	FShaderCodeChunk(const TCHAR* InCode,EMaterialValueType InType,DWORD InFlags,INT InTextureDependencyLength):
		Code(InCode),
		UniformExpression(NULL),
		Type(InType),
		Flags(InFlags),
		TextureDependencyLength(InTextureDependencyLength)
	{}

	FShaderCodeChunk(FMaterialUniformExpression* InUniformExpression,const TCHAR* InCode,EMaterialValueType InType,DWORD InFlags):
		Code(InCode),
		UniformExpression(InUniformExpression),
		Type(InType),
		Flags(InFlags),
		TextureDependencyLength(0)
	{}
};

/**
 * Destructor
 */
FMaterial::~FMaterial()
{
	if (ShaderMap)
	{
		ShaderMap->BeginRelease();
	}
}

class FHLSLMaterialTranslator : public FMaterialCompiler
{
public:
	enum EShaderCodeChunkFlags
	{
		SCF_RGBE_4BIT_EXPONENT			= 1,
		SCF_RGBE_8BIT_EXPONENT			= 2,
		SCF_RGBE						= SCF_RGBE_4BIT_EXPONENT | SCF_RGBE_8BIT_EXPONENT,
		SCF_GREYSCALE_TEXTURE			= 4,
		SCF_A8R8G8B8_TEXTURE			= 8,
	};

	TIndirectArray<FShaderCodeChunk> CodeChunks;
	TArray<UMaterialExpression*> ExpressionStack;

	/** A map from material expression to the index into CodeChunks of the code for the material expression. */
	TMap<UMaterialExpression*,INT> ExpressionCodeMap;

	UBOOL bSuccess;

	FMaterial* Material;

	EShaderPlatform Platform;

	FHLSLMaterialTranslator(FMaterial* InMaterial,EShaderPlatform InPlatform):
	bSuccess(TRUE),
		Material(InMaterial),
		Platform(InPlatform)
	{
		Material->CompileErrors.Empty();
		Material->TextureDependencyLengthMap.Empty();
		Material->MaxTextureDependencyLength = 0;

		Material->NumUserTexCoords = 0;

		// decals need at least texcoord[0].xy for clipping
		// and texcoord[1].zw for attenuation and distance clipping
		if( Material->IsUsedWithDecals() )
		{
			Material->NumUserTexCoords = 2;				
		}

		Material->UniformScalarExpressions.Empty();
		Material->UniformVectorExpressions.Empty();
		Material->Uniform2DTextureExpressions.Empty();
		Material->UniformCubeTextureExpressions.Empty();
		Material->UsingTransforms = UsedCoord_None;
		Material->bUsesSceneDepth = FALSE;
		Material->bUsesSceneColor = FALSE;
	}

	const TCHAR* DescribeType(EMaterialValueType Type) const
	{
		switch(Type)
		{
		case MCT_Float1:		return TEXT("float1");
		case MCT_Float2:		return TEXT("float2");
		case MCT_Float3:		return TEXT("float3");
		case MCT_Float4:		return TEXT("float4");
		case MCT_Float:			return TEXT("float");
		case MCT_Texture2D:		return TEXT("texture2D");
		case MCT_TextureCube:	return TEXT("textureCube");
		default:				return TEXT("unknown");
		};
	}

	// AddCodeChunk - Adds a formatted code string to the Code array and returns its index.
	INT AddCodeChunk(EMaterialValueType Type,DWORD Flags,INT TextureDependencyDepth,const TCHAR* Format,...)
	{
		INT		BufferSize		= 256;
		TCHAR*	FormattedCode	= NULL;
		INT		Result			= -1;

		while(Result == -1)
		{
			FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT( FormattedCode, BufferSize, BufferSize-1, Format, Format, Result );
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;

		INT	CodeIndex = CodeChunks.Num();
		new(CodeChunks) FShaderCodeChunk(FormattedCode,Type,Flags,TextureDependencyDepth);

		appFree(FormattedCode);

		return CodeIndex;
	}

	// AddUniformExpression - Adds an input to the Code array and returns its index.
	INT AddUniformExpression(FMaterialUniformExpression* UniformExpression,EMaterialValueType Type,DWORD Flags,const TCHAR* Format,...)
	{
		if (Type == MCT_Unknown)
		{
			return INDEX_NONE;
		}

		// Search for an existing code chunk with the same uniform expression.
		for(INT ChunkIndex = 0;ChunkIndex < CodeChunks.Num();ChunkIndex++)
		{
			FMaterialUniformExpression* TestExpression = CodeChunks(ChunkIndex).UniformExpression;
			if(TestExpression && TestExpression->GetType() == UniformExpression->GetType() && TestExpression->IsIdentical(UniformExpression))
			{
				// This code chunk has an identical uniform expression to the new expression.  Reuse it.
				check(Type == CodeChunks(ChunkIndex).Type);
				delete UniformExpression;
				return ChunkIndex;
			}
		}

		INT		BufferSize		= 256;
		TCHAR*	FormattedCode	= NULL;
		INT		Result			= -1;

		while(Result == -1)
		{
			FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT( FormattedCode, BufferSize, BufferSize-1, Format, Format, Result );
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;

		INT	CodeIndex = CodeChunks.Num();
		new(CodeChunks) FShaderCodeChunk(UniformExpression,FormattedCode,Type,Flags);

		appFree(FormattedCode);

		return CodeIndex;
	}

	// AccessUniformExpression - Adds code to access the value of a uniform expression to the Code array and returns its index.
	INT AccessUniformExpression(INT Index)
	{
		check(Index >= 0 && Index < CodeChunks.Num());

		const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);

		check(CodeChunk.UniformExpression);

		TCHAR FormattedCode[MAX_SPRINTF]=TEXT("");
		if(CodeChunk.Type == MCT_Float)
		{
			INT	ScalarInputIndex = Material->UniformScalarExpressions.AddUniqueItem(CodeChunk.UniformExpression);
			// Update the above appMalloc if this appSprintf grows in size, e.g. %s, ...
			appSprintf(FormattedCode,TEXT("UniformScalar_%u"),ScalarInputIndex);
		}
		else if(CodeChunk.Type & MCT_Float)
		{
			INT VectorInputIndex = Material->UniformVectorExpressions.AddUniqueItem(CodeChunk.UniformExpression);
			const TCHAR* Mask;
			switch(CodeChunk.Type)
			{
			case MCT_Float:
			case MCT_Float1: Mask = TEXT(".r"); break;
			case MCT_Float2: Mask = TEXT(".rg"); break;
			case MCT_Float3: Mask = TEXT(".rgb"); break;
			default: Mask = TEXT(""); break;
			};
			appSprintf(FormattedCode,TEXT("UniformVector_%u%s"),VectorInputIndex,Mask);
		}
		else if(CodeChunk.Type & MCT_Texture)
		{
			INT TextureInputIndex = INDEX_NONE;
			const TCHAR* BaseName = TEXT("");
			switch(CodeChunk.Type)
			{
			case MCT_Texture2D:
				TextureInputIndex = Material->Uniform2DTextureExpressions.AddUniqueItem(CodeChunk.UniformExpression);
				BaseName = TEXT("Texture2D");
				break;
			case MCT_TextureCube:
				TextureInputIndex = Material->UniformCubeTextureExpressions.AddUniqueItem(CodeChunk.UniformExpression);
				BaseName = TEXT("TextureCube");
				break;
			default: appErrorf(TEXT("Unrecognized texture material value type: %u"),(INT)CodeChunk.Type);
			};
			appSprintf(FormattedCode,TEXT("%s_%u"),BaseName,TextureInputIndex);
		}
		else
		{
			appErrorf(TEXT("User input of unknown type: %s"),DescribeType(CodeChunk.Type));
		}

		INT	CodeIndex = CodeChunks.Num();
		new(CodeChunks) FShaderCodeChunk(FormattedCode,CodeChunks(Index).Type,0,0);			

		return CodeIndex;
	}

	// GetParameterCode

	const TCHAR* GetParameterCode(INT Index)
	{
		check(Index >= 0 && Index < CodeChunks.Num());
		if(!CodeChunks(Index).UniformExpression || CodeChunks(Index).UniformExpression->IsConstant())
		{
			return *CodeChunks(Index).Code;
		}
		else
		{
			return *CodeChunks(AccessUniformExpression(Index)).Code;
		}
	}

	// CoerceParameter
	FString CoerceParameter(INT Index,EMaterialValueType DestType)
	{
		check(Index >= 0 && Index < CodeChunks.Num());
		const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);
		if( CodeChunk.Type == DestType )
		{
			return GetParameterCode(Index);
		}
		else
			if( (CodeChunk.Type & DestType) && (CodeChunk.Type & MCT_Float) )
			{
				switch( DestType )
				{
				case MCT_Float1:
					return FString::Printf( TEXT("float(%s)"), GetParameterCode(Index) );
				case MCT_Float2:
					return FString::Printf( TEXT("float2(%s,%s)"), GetParameterCode(Index), GetParameterCode(Index) );
				case MCT_Float3:
					return FString::Printf( TEXT("float3(%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
				case MCT_Float4:
					return FString::Printf( TEXT("float4(%s,%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
				default: 
					return FString::Printf( TEXT("%s"), GetParameterCode(Index) );
				}
			}
			else
			{
				Errorf(TEXT("Coercion failed: %s: %s -> %s"),*CodeChunk.Code,DescribeType(CodeChunk.Type),DescribeType(DestType));
				return TEXT("");
			}
	}

	// GetFixedParameterCode
	const TCHAR* GetFixedParameterCode(INT Index) const
	{
		if(Index != INDEX_NONE)
		{
			check(Index >= 0 && Index < CodeChunks.Num());
			check(!CodeChunks(Index).UniformExpression || CodeChunks(Index).UniformExpression->IsConstant());
			return *CodeChunks(Index).Code;
		}
		else
		{
			return TEXT("0");
		}
	}

	// GetParameterType
	EMaterialValueType GetParameterType(INT Index) const
	{
		check(Index >= 0 && Index < CodeChunks.Num());
		return CodeChunks(Index).Type;
	}

	// GetParameterUniformExpression
	FMaterialUniformExpression* GetParameterUniformExpression(INT Index) const
	{
		check(Index >= 0 && Index < CodeChunks.Num());
		return CodeChunks(Index).UniformExpression;
	}

	// GetParameterFlags
	DWORD GetParameterFlags(INT Index) const
	{
		check(Index >= 0 && Index < CodeChunks.Num());
		return CodeChunks(Index).Flags;
	}

	/**
	* Finds the texture dependency length of the given code chunk.
	* @param CodeChunkIndex - The index of the code chunk.
	* @return The texture dependency length of the code chunk.
	*/
	INT GetTextureDependencyLength(INT CodeChunkIndex)
	{
		if(CodeChunkIndex != INDEX_NONE)
		{
			check(CodeChunkIndex >= 0 && CodeChunkIndex < CodeChunks.Num());
			return CodeChunks(CodeChunkIndex).TextureDependencyLength;
		}
		else
		{
			return 0;
		}
	}

	/**
	* Finds the maximum texture dependency length of the given code chunks.
	* @param CodeChunkIndex* - A list of code chunks to find the maximum texture dependency length from.
	* @return The texture dependency length of the code chunk.
	*/
	INT GetTextureDependencyLengths(INT CodeChunkIndex0,INT CodeChunkIndex1 = INDEX_NONE,INT CodeChunkIndex2 = INDEX_NONE)
	{
		return ::Max(
			::Max(
			GetTextureDependencyLength(CodeChunkIndex0),
			GetTextureDependencyLength(CodeChunkIndex1)
			),
			GetTextureDependencyLength(CodeChunkIndex2)
			);
	}

	// GetArithmeticResultType
	EMaterialValueType GetArithmeticResultType(EMaterialValueType TypeA,EMaterialValueType TypeB)
	{
		if(!(TypeA & MCT_Float) || !(TypeB & MCT_Float))
		{
			Errorf(TEXT("Attempting to perform arithmetic on non-numeric types: %s %s"),DescribeType(TypeA),DescribeType(TypeB));
		}

		if(TypeA == TypeB)
		{
			return TypeA;
		}
		else if(TypeA & TypeB)
		{
			if(TypeA == MCT_Float)
			{
				return TypeB;
			}
			else
			{
				check(TypeB == MCT_Float);
				return TypeA;
			}
		}
		else
		{
			Errorf(TEXT("Arithmetic between types %s and %s are undefined"),DescribeType(TypeA),DescribeType(TypeB));
			return MCT_Unknown;
		}
	}

	EMaterialValueType GetArithmeticResultType(INT A,INT B)
	{
		check(A >= 0 && A < CodeChunks.Num());
		check(B >= 0 && B < CodeChunks.Num());

		EMaterialValueType	TypeA = CodeChunks(A).Type,
			TypeB = CodeChunks(B).Type;

		return GetArithmeticResultType(TypeA,TypeB);
	}

	// FMaterialCompiler interface.

	virtual INT Error(const TCHAR* Text)
	{
		//add the error string to the material's CompileErrors array
		new (Material->CompileErrors) FString(Text);
		bSuccess = FALSE;
		return INDEX_NONE;
	}

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* Compiler)
	{
		// Check if this expression has already been translated.
		INT* ExistingCodeIndex = ExpressionCodeMap.Find(MaterialExpression);
		if(ExistingCodeIndex)
		{
			return *ExistingCodeIndex;
		}
		else
		{
			// Disallow reentrance.
			if(ExpressionStack.FindItemIndex(MaterialExpression) != INDEX_NONE)
			{
				return Error(TEXT("Reentrant expression"));
			}

			// The first time this expression is called, translate it.
			ExpressionStack.AddItem(MaterialExpression);
			INT Result = MaterialExpression->Compile(Compiler);
			check(ExpressionStack.Pop() == MaterialExpression);

			// Save the texture dependency depth for the expression.
			if (Material->IsTerrainMaterial() == FALSE)
			{
				const INT TextureDependencyLength = GetTextureDependencyLength(Result);
				Material->TextureDependencyLengthMap.Set(MaterialExpression,TextureDependencyLength);
				Material->MaxTextureDependencyLength = ::Max(Material->MaxTextureDependencyLength,TextureDependencyLength);
			}

			// Cache the translation.
			ExpressionCodeMap.Set(MaterialExpression,Result);

			return Result;
		}
	}

	virtual EMaterialValueType GetType(INT Code)
	{
		if(Code != INDEX_NONE)
		{
			return GetParameterType(Code);
		}
		else
		{
			return MCT_Unknown;
		}
	}

	virtual INT ForceCast(INT Code,EMaterialValueType DestType)
	{
		if(Code == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(Code) && !GetParameterUniformExpression(Code)->IsConstant())
		{
			return ForceCast(AccessUniformExpression(Code),DestType);
		}

		EMaterialValueType	SourceType = GetParameterType(Code);

		if(SourceType & DestType)
		{
			return Code;
		}
		else if((SourceType & MCT_Float) && (DestType & MCT_Float))
		{
			UINT	NumSourceComponents = GetNumComponents(SourceType),
				NumDestComponents = GetNumComponents(DestType);

			if(NumSourceComponents > NumDestComponents) // Use a mask to select the first NumDestComponents components from the source.
			{
				const TCHAR*	Mask;
				switch(NumDestComponents)
				{
				case 1: Mask = TEXT(".r"); break;
				case 2: Mask = TEXT(".rg"); break;
				case 3: Mask = TEXT(".rgb"); break;
				default: appErrorf(TEXT("Should never get here!")); return INDEX_NONE;
				};

				return AddCodeChunk(DestType,0,GetTextureDependencyLength(Code),TEXT("%s%s"),GetParameterCode(Code),Mask);
			}
			else if(NumSourceComponents < NumDestComponents) // Pad the source vector up to NumDestComponents.
			{
				UINT	NumPadComponents = NumDestComponents - NumSourceComponents;
				return AddCodeChunk(
					DestType,
					0,
					GetTextureDependencyLength(Code),
					TEXT("%s(%s%s%s%s)"),
					DescribeType(DestType),
					GetParameterCode(Code),
					NumPadComponents >= 1 ? TEXT(",0") : TEXT(""),
					NumPadComponents >= 2 ? TEXT(",0") : TEXT(""),
					NumPadComponents >= 3 ? TEXT(",0") : TEXT("")
					);
			}
			else
			{
				return Code;
			}
		}
		else
		{
			return Errorf(TEXT("Cannot force a cast between non-numeric types."));
		}
	}

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue)
	{
		return AddUniformExpression(new FMaterialUniformExpressionVectorParameter(ParameterName,DefaultValue),MCT_Float4,0,TEXT(""));
	}

	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue)
	{
		return AddUniformExpression(new FMaterialUniformExpressionScalarParameter(ParameterName,DefaultValue),MCT_Float,0,TEXT(""));
	}

	virtual INT FlipBookOffset(UTexture* InFlipBook)
	{
		UTextureFlipBook* FlipBook = CastChecked<UTextureFlipBook>(InFlipBook);
		return AddUniformExpression(new FMaterialUniformExpressionFlipBookTextureParameter(FlipBook), MCT_Float4, 0, TEXT(""));
	}

	virtual INT Constant(FLOAT X)
	{
		return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,0,0,0),MCT_Float),MCT_Float,0,TEXT("(%0.8f)"),X);
	}

	virtual INT Constant2(FLOAT X,FLOAT Y)
	{
		return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,0,0),MCT_Float2),MCT_Float2,0,TEXT("float2(%0.8f,%0.8f)"),X,Y);
	}

	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z)
	{
		return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,Z,0),MCT_Float3),MCT_Float3,0,TEXT("float3(%0.8f,%0.8f,%0.8f)"),X,Y,Z);
	}

	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W)
	{
		return AddUniformExpression(new FMaterialUniformExpressionConstant(FLinearColor(X,Y,Z,W),MCT_Float4),MCT_Float4,0,TEXT("float4(%0.8f,%0.8f,%0.8f,%0.8f)"),X,Y,Z,W);
	}

	virtual INT GameTime()
	{
		return AddUniformExpression(new FMaterialUniformExpressionTime(),MCT_Float,0,TEXT(""));
	}

	virtual INT RealTime()
	{
		return AddUniformExpression(new FMaterialUniformExpressionRealTime(),MCT_Float,0,TEXT(""));
	}

	virtual INT PeriodicHint(INT PeriodicCode)
	{
		if(PeriodicCode == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(PeriodicCode))
		{
			return AddUniformExpression(new FMaterialUniformExpressionPeriodic(GetParameterUniformExpression(PeriodicCode)),GetParameterType(PeriodicCode),0,TEXT("%s"),GetParameterCode(PeriodicCode));
		}
		else
		{
			return PeriodicCode;
		}
	}

	virtual INT Sine(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionSine(GetParameterUniformExpression(X),0),MCT_Float,0,TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
		}
		else
		{
			return AddCodeChunk(MCT_Float,0,GetTextureDependencyLength(X),TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
		}
	}

	virtual INT Cosine(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionSine(GetParameterUniformExpression(X),1),MCT_Float,0,TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
		}
		else
		{
			return AddCodeChunk(MCT_Float,0,GetTextureDependencyLength(X),TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
		}
	}

	virtual INT Floor(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFloor(GetParameterUniformExpression(X)),GetParameterType(X),0,TEXT("floor(%s)"),GetParameterCode(X));
		}
		else
		{
			return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("floor(%s)"),GetParameterCode(X));
		}
	}

	virtual INT Ceil(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionCeil(GetParameterUniformExpression(X)),GetParameterType(X),0,TEXT("ceil(%s)"),GetParameterCode(X));
		}
		else
		{
			return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("ceil(%s)"),GetParameterCode(X));
		}
	}

	virtual INT Frac(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFrac(GetParameterUniformExpression(X)),GetParameterType(X),0,TEXT("frac(%s)"),GetParameterCode(X));
		}
		else
		{
			return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("frac(%s)"),GetParameterCode(X));
		}
	}

	/**
	* Creates the new shader code chunk needed for the Abs expression
	*
	* @param	X - Index to the FMaterialCompiler::CodeChunk entry for the input expression
	* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
	*/	
	virtual INT Abs( INT X )
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		// get the user input struct for the input expression
		FMaterialUniformExpression* pInputParam = GetParameterUniformExpression(X);
		if( pInputParam )
		{
			FMaterialUniformExpressionAbs* pUniformExpression = new FMaterialUniformExpressionAbs( pInputParam );
			return AddUniformExpression( pUniformExpression, GetParameterType(X), 0, TEXT("abs(%s)"), GetParameterCode(X) );
		}
		else
		{
			return AddCodeChunk( GetParameterType(X), 0, GetTextureDependencyLength(X), TEXT("abs(%s)"), GetParameterCode(X) );
		}
	}

	virtual INT ReflectionVector()
	{
		return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentReflectionVector"));
	}

	virtual INT CameraVector()
	{
		return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentCameraVector"));
	}

	virtual INT CameraWorldPosition()
	{
		return AddCodeChunk(MCT_Float3,0,0,TEXT("CameraWorldPos"));
	}

	virtual INT LightVector()
	{
		return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.TangentLightVector"));
	}

	virtual INT ScreenPosition(  UBOOL bScreenAlign )
	{
		if( bScreenAlign )
		{
			return AddCodeChunk(MCT_Float4,0,0,TEXT("ScreenAlignedPosition(Parameters.ScreenPosition)"));		
		}
		else
		{
			return AddCodeChunk(MCT_Float4,0,0,TEXT("Parameters.ScreenPosition"));		
		}	
	}

	virtual INT WorldPosition()
	{
		Material->UsingTransforms |= UsedCoord_WorldPos;
		return AddCodeChunk(MCT_Float3,0,0,TEXT("Parameters.WorldPosition"));		
	}

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB)
	{
		if(A == INDEX_NONE || B == INDEX_NONE || AGreaterThanB == INDEX_NONE || AEqualsB == INDEX_NONE || ALessThanB == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		EMaterialValueType ResultType = GetArithmeticResultType(GetParameterType(AGreaterThanB),GetArithmeticResultType(AEqualsB,ALessThanB));

		INT CoercedAGreaterThanB = ForceCast(AGreaterThanB,ResultType);
		INT CoercedAEqualsB = ForceCast(AEqualsB,ResultType);
		INT CoercedALessThanB = ForceCast(ALessThanB,ResultType);

		return AddCodeChunk(
			ResultType,
			0,
			::Max(GetTextureDependencyLengths(A,B),GetTextureDependencyLengths(AGreaterThanB,AEqualsB,ALessThanB)),
			TEXT("((%s >= %s) ? (%s > %s ? %s : %s) : %s)"),
			GetParameterCode(A),
			GetParameterCode(B),
			GetParameterCode(A),
			GetParameterCode(B),
			GetParameterCode(CoercedAGreaterThanB),
			GetParameterCode(CoercedAEqualsB),
			GetParameterCode(CoercedALessThanB)
			);
	}

	virtual INT TextureCoordinate(UINT CoordinateIndex)
	{
		Material->NumUserTexCoords = ::Max(CoordinateIndex + 1,Material->NumUserTexCoords);
		if (Material->NumUserTexCoords > 0)
		{
			if (Material->IsUsedWithDecals())
			{
				return AddCodeChunk(
					MCT_Float2,0,0,
					TEXT("Parameters.TexCoords[0].xy"),
					CoordinateIndex
					);
			}
			else
			{
				return AddCodeChunk(
					MCT_Float2,0,0,
					TEXT("Parameters.TexCoords[%u].xy"),
					CoordinateIndex
					);
			}
		}
		else
		{
			return AddCodeChunk(
				MCT_Float2,0,0,
				TEXT("float2(0,0)"),
				CoordinateIndex
				);
		}
	}

	virtual INT TextureSample(INT TextureIndex,INT CoordinateIndex)
	{
		if(TextureIndex == INDEX_NONE || CoordinateIndex == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		EMaterialValueType	TextureType = GetParameterType(TextureIndex);
		DWORD				Flags		= GetParameterFlags(TextureIndex);

		FString				SampleCode;

		switch(TextureType)
		{
		case MCT_Texture2D:
			SampleCode = TEXT("tex2D(%s,%s)");
			break;
		case MCT_TextureCube:
			SampleCode = TEXT("texCUBE(%s,%s)");
			break;
		}

		if( Flags & SCF_RGBE_4BIT_EXPONENT )
		{
			SampleCode = FString::Printf( TEXT("ExpandCompressedRGBE(%s)"), *SampleCode );
		}

		if( Flags & SCF_RGBE_8BIT_EXPONENT )
		{
			SampleCode = FString::Printf( TEXT("ExpandRGBE(%s)"), *SampleCode );
		}

		if( Flags & SCF_GREYSCALE_TEXTURE )
		{
			// Sampling a greyscale texture in D3D9 gives: (G,G,G)
			// Sampling a greyscale texture in D3D10 gives: (G,0,0)
			// This replication reproduces the D3D9 behavior in all cases.
			SampleCode = FString::Printf( TEXT("(%s).rrrr"), *SampleCode );
		}

		if((Flags & SCF_A8R8G8B8_TEXTURE) && Platform == SP_PCD3D_SM4)
		{
			// Sampling a ARGB texture in D3D9 gives  (R,G,B,A)
			// Sampling a ARGB texture in D3D10 gives (B,G,R,A)
			// This swizzle reproduces the D3D9 behavior in all cases.
			SampleCode = FString::Printf(TEXT("(%s).bgra"),*SampleCode);
		}

		switch(TextureType)
		{
		case MCT_Texture2D:
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIndex) + 1,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_Texture2D),
				*CoerceParameter(CoordinateIndex,MCT_Float2)
				);
		case MCT_TextureCube:
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIndex) + 1,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_TextureCube),
				*CoerceParameter(CoordinateIndex,MCT_Float3)
				);
		default:
			Errorf(TEXT("Sampling unknown texture type: %s"),DescribeType(TextureType));
			return INDEX_NONE;
		};
	}

	/**
	* Add the shader code for sampling from the scene texture
	* @param	TexType - scene texture type to sample from
	* @param	CoordinateIdx - index of shader code for user specified tex coords
	* @param	ScreenAlign - Whether to map [0,1] UVs to the view within the back buffer
	*/
	virtual INT SceneTextureSample( BYTE TexType, INT CoordinateIdx, UBOOL ScreenAlign )
	{
		// use the scene texture type 
		FString SceneTexCode;
		switch(TexType)
		{
		case SceneTex_Lighting:
			SceneTexCode = FString(TEXT("SceneColorTexture"));
			break;
		default:
			Errorf(TEXT("Scene texture type not supported."));
			return INDEX_NONE;
		}

		Material->bUsesSceneColor = TRUE;
		if ( ScreenAlign && CoordinateIdx != INDEX_NONE )
		{
			// sampler
			FString SampleCode( TEXT("tex2D(%s,ScreenAlignedUV(%s))") );
			// add the code string
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIdx) + 1,
				*SampleCode,
				*SceneTexCode,
				*CoerceParameter(CoordinateIdx,MCT_Float2)
				);
		}
		else
		{
			// sampler
			FString	SampleCode( TEXT("tex2D(%s,%s)") );
			// replace default tex coords with user specified coords if available
			FString DefaultScreenAligned(TEXT("float2(ScreenAlignedPosition(Parameters.ScreenPosition).xy)"));
			FString TexCoordCode( (CoordinateIdx != INDEX_NONE) ? CoerceParameter(CoordinateIdx,MCT_Float2) : DefaultScreenAligned );
			// add the code string
			return AddCodeChunk(
				MCT_Float4,
				0,
				GetTextureDependencyLength(CoordinateIdx) + 1,
				*SampleCode,
				*SceneTexCode,
				*TexCoordCode
				);
		}
	}

	/**
	* Add the shader code for sampling the scene depth
	* @param	bNormalize - @todo implement
	* @param	CoordinateIdx - index of shader code for user specified tex coords
	*/
	virtual INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)
	{
		Material->bUsesSceneDepth = TRUE;

		// sampler
		FString	UserDepthCode( TEXT("CalcSceneDepth(%s)") );
		// replace default tex coords with user specified coords if available
		FString DefaultScreenAligned(TEXT("float2(ScreenAlignedPosition(Parameters.ScreenPosition).xy)"));
		FString TexCoordCode( (CoordinateIdx != INDEX_NONE) ? CoerceParameter(CoordinateIdx,MCT_Float2) : DefaultScreenAligned );
		// add the code string
		return AddCodeChunk(
			MCT_Float1,
			0,
			GetTextureDependencyLength(CoordinateIdx) + 1,
			*UserDepthCode,
			*TexCoordCode
			);
	}

	virtual INT PixelDepth(UBOOL bNormalize)
	{
		return AddCodeChunk(MCT_Float1, 0, 0, TEXT("Parameters.ScreenPosition.w"));		
	}

	virtual INT DestColor()
	{
		// note: can just call
		// SceneTextureSample(SceneTex_Lighting,INDEX_NONE);

		Material->bUsesSceneColor = TRUE;
		FString	UserColorCode(TEXT("PreviousLighting(%s)"));
		FString	ScreenPosCode(TEXT("Parameters.ScreenPosition"));
		// add the code string
		return AddCodeChunk(
			MCT_Float4,
			0,
			1,
			*UserColorCode,
			*ScreenPosCode
			);
	}

	virtual INT DestDepth(UBOOL bNormalize)
	{
		// note: can just call
		// SceneTextureDepth(FALSE,INDEX_NONE);

		Material->bUsesSceneDepth = TRUE;
		FString	UserDepthCode(TEXT("PreviousDepth(%s)"));
		FString	ScreenPosCode(TEXT("Parameters.ScreenPosition"));
		// add the code string
		return AddCodeChunk(
			MCT_Float1,
			0,
			1,
			*UserDepthCode,
			*ScreenPosCode
			);
	}

	/**
	* Generates a shader code chunk for the DepthBiasedAlpha expression
	* using the given inputs
	* @param SrcAlphaIdx = index to source alpha input expression code chunk
	* @param BiasIdx = index to bias input expression code chunk
	* @param BiasScaleIdx = index to a scale expression code chunk to apply to the bias
	*/
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx )
	{
		INT ResultIdx = INDEX_NONE;

		// all inputs must be valid expressions
		if ((SrcAlphaIdx != INDEX_NONE) &&
			(BiasIdx != INDEX_NONE) &&
			(BiasScaleIdx != INDEX_NONE))
		{
			Material->bUsesSceneDepth = TRUE;
			FString CodeChunk(TEXT("DepthBiasedAlpha(Parameters,%s,%s,%s)"));
			ResultIdx = AddCodeChunk(
				MCT_Float1,
				0,
				::Max(GetTextureDependencyLengths(SrcAlphaIdx,BiasIdx,BiasScaleIdx),1),
				*CodeChunk,
				*CoerceParameter(SrcAlphaIdx,MCT_Float1),
				*CoerceParameter(BiasIdx,MCT_Float1),
				*CoerceParameter(BiasScaleIdx,MCT_Float1)
				);
		}

		return ResultIdx;
	}

	/**
	* Generates a shader code chunk for the DepthBiasedBlend expression
	* using the given inputs
	* @param SrcColorIdx = index to source color input expression code chunk
	* @param BiasIdx = index to bias input expression code chunk
	* @param BiasScaleIdx = index to a scale expression code chunk to apply to the bias
	*/
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx )
	{
		INT ResultIdx = INDEX_NONE;

		// all inputs must be valid expressions
		if( SrcColorIdx != INDEX_NONE && 
			BiasIdx != INDEX_NONE &&
			BiasScaleIdx != INDEX_NONE )
		{
			FString CodeChunk( TEXT("DepthBiasedBlend(Parameters,%s,%s,%s)") );
			ResultIdx = AddCodeChunk(
				MCT_Float3,
				0,
				::Max(GetTextureDependencyLengths(SrcColorIdx,BiasIdx,BiasScaleIdx),1),
				*CodeChunk,
				*CoerceParameter(SrcColorIdx,MCT_Float3),
				*CoerceParameter(BiasIdx,MCT_Float1),
				*CoerceParameter(BiasScaleIdx,MCT_Float1)
				);

			Material->bUsesSceneDepth = TRUE;
			Material->bUsesSceneColor = TRUE;
		}

		return ResultIdx;			
	}

	static DWORD GetTextureFlags(UTexture* InTexture)
	{
		// Determine the texture's format.
		UTexture2D* Texture2D = Cast<UTexture2D>(InTexture);
		UTextureCube* TextureCube = Cast<UTextureCube>(InTexture);
		BYTE Format = PF_Unknown;
		if(Texture2D)
		{
			Format = Texture2D->Format;
		}
		else if(TextureCube)
		{
			Format = TextureCube->Format;
		}

		// Determine the appropriate flags for the texture based on its format.
		DWORD Flags = 0;
		if(InTexture->RGBE)
		{
			Flags |= (Format == PF_A8R8G8B8) ? SCF_RGBE_8BIT_EXPONENT : SCF_RGBE_4BIT_EXPONENT;
		}
		if(Format == PF_G8)
		{
			Flags |= SCF_GREYSCALE_TEXTURE;
		}
		if(Format == PF_A8R8G8B8)
		{
			Flags |= SCF_A8R8G8B8_TEXTURE;
		}
		return Flags;
	}

	virtual INT Texture(UTexture* InTexture)
	{
		EMaterialValueType ShaderType = InTexture->GetMaterialType();
		return AddUniformExpression(new FMaterialUniformExpressionTexture(InTexture),ShaderType,GetTextureFlags(InTexture),TEXT(""));
	}

	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultValue)
	{
		EMaterialValueType ShaderType = DefaultValue->GetMaterialType();
		return AddUniformExpression(new FMaterialUniformExpressionTextureParameter(ParameterName,DefaultValue),ShaderType,GetTextureFlags(DefaultValue),TEXT(""));
	}

	virtual INT VertexColor()
	{
		return AddCodeChunk(MCT_Float4,0,0,TEXT("Parameters.VertexColor"));
	}

	virtual INT Add(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Add),GetArithmeticResultType(A,B),0,TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
		}
	}

	virtual INT Sub(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Sub),GetArithmeticResultType(A,B),0,TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
		}
	}

	virtual INT Mul(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Mul),GetArithmeticResultType(A,B),0,TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			EMaterialValueType TypeA = GetType(A);
			EMaterialValueType TypeB = GetType(B);

			//work around a compiler bug with the 180 PS3 SDK 
			//where UniformVector_0.rgb * UniformVector_0.a is incorrectly optimized as UniformVector_0.rgb
			//but UniformVector_0.a * UniformVector_0.rgb compiles correctly
			if (TypeA != TypeB && TypeB == MCT_Float)
			{
				//swap the multiplication order
				INT SwapTemp = A;
				A = B;
				B = SwapTemp;
			}

			return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
		}
	}

	virtual INT Div(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Div),GetArithmeticResultType(A,B),0,TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			return AddCodeChunk(GetArithmeticResultType(A,B),0,GetTextureDependencyLengths(A,B),TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
		}
	}

	virtual INT Dot(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionFoldedMath(GetParameterUniformExpression(A),GetParameterUniformExpression(B),FMO_Dot),MCT_Float,0,TEXT("dot(%s,%s)"),GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			return AddCodeChunk(MCT_Float,0,GetTextureDependencyLengths(A,B),TEXT("dot(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
		}
	}

	virtual INT Cross(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		return AddCodeChunk(MCT_Float3,0,GetTextureDependencyLengths(A,B),TEXT("cross(%s,%s)"),*CoerceParameter(A,MCT_Float3),*CoerceParameter(B,MCT_Float3));
	}

	virtual INT Power(INT Base,INT Exponent)
	{
		if(Base == INDEX_NONE || Exponent == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		FString ExponentCode = CoerceParameter(Exponent,MCT_Float);
		if (CodeChunks(Exponent).UniformExpression && CodeChunks(Exponent).UniformExpression->IsConstant())
		{
			//chop off the parenthesis
			FString NumericPortion = ExponentCode.Mid(1, ExponentCode.Len() - 2);
			FLOAT ExponentValue = appAtof(*NumericPortion); 
			//check if the power was 1.0f to work around a xenon HLSL compiler bug in the Feb XDK
			//which incorrectly optimizes pow(x, 1.0f) as if it were pow(x, 0.0f).
			if (fabs(ExponentValue - 1.0f) < (FLOAT)KINDA_SMALL_NUMBER)
			{
				return Base;
			}
		}

		return AddCodeChunk(GetParameterType(Base),0,GetTextureDependencyLengths(Base,Exponent),TEXT("pow(%s,%s)"),GetParameterCode(Base),*ExponentCode);
	}

	virtual INT SquareRoot(INT X)
	{
		if(X == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X))
		{
			return AddUniformExpression(new FMaterialUniformExpressionSquareRoot(GetParameterUniformExpression(X)),GetParameterType(X),0,TEXT("sqrt(%s)"),GetParameterCode(X));
		}
		else
		{
			return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLength(X),TEXT("sqrt(%s)"),GetParameterCode(X));
		}
	}

	virtual INT Lerp(INT X,INT Y,INT A)
	{
		if(X == INDEX_NONE || Y == INDEX_NONE || A == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		EMaterialValueType ResultType = GetArithmeticResultType(X,Y);
		return AddCodeChunk(ResultType,0,GetTextureDependencyLengths(X,Y,A),TEXT("lerp(%s,%s,%s)"),*CoerceParameter(X,ResultType),*CoerceParameter(Y,ResultType),*CoerceParameter(A,MCT_Float1));
	}

	virtual INT Min(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionMin(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(A),0,TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
		}
		else
		{
			return AddCodeChunk(GetParameterType(A),0,GetTextureDependencyLengths(A,B),TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
		}
	}

	virtual INT Max(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionMax(GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(A),0,TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
		}
		else
		{
			return AddCodeChunk(GetParameterType(A),0,GetTextureDependencyLengths(A,B),TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
		}
	}

	virtual INT Clamp(INT X,INT A,INT B)
	{
		if(X == INDEX_NONE || A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		if(GetParameterUniformExpression(X) && GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionClamp(GetParameterUniformExpression(X),GetParameterUniformExpression(A),GetParameterUniformExpression(B)),GetParameterType(X),0,TEXT("min(max(%s,%s),%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
		}
		else
		{
			return AddCodeChunk(GetParameterType(X),0,GetTextureDependencyLengths(X,A,B),TEXT("min(max(%s,%s),%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
		}
	}

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A)
	{
		if(Vector == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		EMaterialValueType	VectorType = GetParameterType(Vector);

		if(	A && (VectorType & MCT_Float) < MCT_Float4 ||
			B && (VectorType & MCT_Float) < MCT_Float3 ||
			G && (VectorType & MCT_Float) < MCT_Float2 ||
			R && (VectorType & MCT_Float) < MCT_Float1)
			Errorf(TEXT("Not enough components in (%s: %s) for component mask %u%u%u%u"),GetParameterCode(Vector),DescribeType(GetParameterType(Vector)),R,G,B,A);

		EMaterialValueType	ResultType;
		switch((R ? 1 : 0) + (G ? 1 : 0) + (B ? 1 : 0) + (A ? 1 : 0))
		{
		case 1: ResultType = MCT_Float; break;
		case 2: ResultType = MCT_Float2; break;
		case 3: ResultType = MCT_Float3; break;
		case 4: ResultType = MCT_Float4; break;
		default: Errorf(TEXT("Couldn't determine result type of component mask %u%u%u%u"),R,G,B,A); return INDEX_NONE;
		};

		return AddCodeChunk(
			ResultType,
			0,
			GetTextureDependencyLength(Vector),
			TEXT("%s.%s%s%s%s"),
			GetParameterCode(Vector),
			R ? TEXT("r") : TEXT(""),
			G ? TEXT("g") : TEXT(""),
			B ? TEXT("b") : TEXT(""),
			A ? TEXT("a") : TEXT("")
			);
	}

	virtual INT AppendVector(INT A,INT B)
	{
		if(A == INDEX_NONE || B == INDEX_NONE)
		{
			return INDEX_NONE;
		}

		INT					NumResultComponents = GetNumComponents(GetParameterType(A)) + GetNumComponents(GetParameterType(B));
		EMaterialValueType	ResultType = GetVectorType(NumResultComponents);

		if(GetParameterUniformExpression(A) && GetParameterUniformExpression(B))
		{
			return AddUniformExpression(new FMaterialUniformExpressionAppendVector(GetParameterUniformExpression(A),GetParameterUniformExpression(B),GetNumComponents(GetParameterType(A))),ResultType,0,TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
		}
		else
		{
			return AddCodeChunk(ResultType,0,GetTextureDependencyLengths(A,B),TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
		}
	}

	/**
	* Generate shader code for transforming a vector
	*
	* @param	CoordType - type of transform to apply. see EMaterialVectorCoordTransform 
	* @param	A - index for input vector parameter's code
	*/
	virtual INT TransformVector(BYTE CoordType,INT A)
	{
		INT Result = INDEX_NONE;
		if(A != INDEX_NONE)
		{
			INT NumInputComponents = GetNumComponents(GetParameterType(A));
			// only allow float3/float4 transforms
			if( NumInputComponents < 3 )
			{
				Result = Errorf(TEXT("input must be a vector (%s: %s)"),GetParameterCode(A),DescribeType(GetParameterType(A)));
			}
			else
			{
				// code string to transform the input vector
				FString CodeStr;
				switch( CoordType )
				{
				case TRANSFORM_World:
					// transform from tangent to world space
					Material->UsingTransforms |= UsedCoord_World;
					CodeStr = FString(TEXT("TransformTangentVectorToWorld(Parameters,%s)"));
					break;
				case TRANSFORM_View:
					// transform from tangent to view space
					Material->UsingTransforms |= UsedCoord_View;
					CodeStr = FString(TEXT("TransformTangentVectorToView(Parameters,%s)"));
					break;
				case TRANSFORM_Local:
					// transform from tangent to local space
					Material->UsingTransforms |= UsedCoord_Local;
					CodeStr = FString(TEXT("TransformTangentVectorToLocal(Parameters,%s)"));
					break;
				default:
					appErrorf( TEXT("Invalid CoordType. See EMaterialVectorCoordTransform") );
				}

				// we are only transforming vectors (not points) so only return a float3
				Result = AddCodeChunk(
					MCT_Float3,
					0,
					GetTextureDependencyLength(A),
					*CodeStr,
					*CoerceParameter(A,MCT_Float3)
					);
			}
		}
		return Result; 
	}

	/**
	* Generate shader code for transforming a position
	*
	* @param	CoordType - type of transform to apply. see EMaterialExpressionTransformPosition 
	* @param	A - index for input vector parameter's code
	*/
	virtual INT TransformPosition(BYTE CoordType,INT A)
	{
		INT Result = INDEX_NONE;
		if(A != INDEX_NONE)
		{
			INT NumInputComponents = GetNumComponents(GetParameterType(A));
			// only allow float4 transforms
			if( NumInputComponents < 3 )
			{
				Result = Errorf(TEXT("Input must be a float4 (%s: %s)"),GetParameterCode(A),DescribeType(GetParameterType(A)));
			}
			else if( NumInputComponents < 4 )
			{
				Result = Errorf(TEXT("Input must be a float4, append 1 to the vector (%s: %s)"),GetParameterCode(A),DescribeType(GetParameterType(A)));
			}
			else
			{
				// code string to transform the input vector
				FString CodeStr;
				switch( CoordType )
				{
				case TRANSFORMPOS_World:
					// transform from post projection to world space
					CodeStr = FString(TEXT("MulMatrix(InvViewProjectionMatrix, %s)"));
					break;
				default:
					appErrorf( TEXT("Invalid CoordType. See EMaterialExpressionTransformPosition") );
				}

				// we are transforming points, not vectors, so return a float4
				Result = AddCodeChunk(
					MCT_Float4,
					0,
					GetTextureDependencyLength(A),
					*CodeStr,
					*CoerceParameter(A,MCT_Float4)
					);
			}
		}
		return Result; 
	}

	INT LensFlareIntesity()
	{
		INT ResultIdx = INDEX_NONE;
		FString CodeChunk(TEXT("GetLensFlareIntensity(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float1,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}

	INT LensFlareOcclusion()
	{
		INT ResultIdx = INDEX_NONE;
		FString CodeChunk(TEXT("GetLensFlareOcclusion(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float1,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}

	INT LensFlareRadialDistance()
	{
		INT ResultIdx = INDEX_NONE;
		FString CodeChunk(TEXT("GetLensFlareRadialDistance(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float1,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}

	INT LensFlareRayDistance()
	{
		INT ResultIdx = INDEX_NONE;
		FString CodeChunk(TEXT("GetLensFlareRayDistance(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float1,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}

	INT LensFlareSourceDistance()
	{
		INT ResultIdx = INDEX_NONE;
		FString CodeChunk(TEXT("GetLensFlareSourceDistance(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float1,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}

	INT DynamicParameter()
	{
		Material->bUsesDynamicParameter = TRUE;

		INT ResultIdx = INDEX_NONE;
		//@todo.SAS. Verify InParamaterIndex is in proper range...
		FString CodeChunk = FString::Printf(TEXT("GetDynamicParameter(Parameters)"));
		ResultIdx = AddCodeChunk(
			MCT_Float4,
			0,
			0,
			*CodeChunk
			);
		return ResultIdx;
	}
};

/**
* Compiles this material for Platform, storing the result in OutShaderMap
*
* @param StaticParameters - the set of static parameters to compile
* @param Platform - the platform to compile for
* @param OutShaderMap - the shader map to compile
* @param bForceCompile - force discard previous results 
* @param bCompileFallback - indicates that if Platform is a sm2 platform and the normal compile failed
*							drop components until the material compiles.
* @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
* @return - TRUE if compile succeeded or was not necessary (shader map for StaticParameters was found and was complete)
*/
UBOOL FMaterial::Compile(FStaticParameterSet* StaticParameters, EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile, UBOOL bCompileFallback, UBOOL bDebugDump)
{
	if( GIsEditor && !GIsUCC )
	{
		GWarn->PushStatus();
		GWarn->StatusUpdatef( -1, -1, *FString( FString( TEXT( "Compiling shaders for: " ) ) + GetFriendlyName() ) );
	}

	// Generate the material shader code.
	FHLSLMaterialTranslator MaterialTranslator(this,Platform);
	INT NormalChunk, EmissiveColorChunk, DiffuseColorChunk, DiffusePowerChunk, SpecularColorChunk, SpecularPowerChunk, OpacityChunk, MaskChunk, DistortionChunk, TwoSidedLightingMaskChunk, CustomLightingChunk;

	STAT(DOUBLE HLSLTranslateTime = 0);
	{
		SCOPE_SECONDS_COUNTER(HLSLTranslateTime);
		const EMaterialShaderPlatform MatPlatform = GetMaterialPlatform(Platform);
		NormalChunk					= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_Normal				,&MaterialTranslator),MCT_Float3);
		EmissiveColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_EmissiveColor		,&MaterialTranslator),MCT_Float3);
		DiffuseColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_DiffuseColor			,&MaterialTranslator),MCT_Float3);
		DiffusePowerChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_DiffusePower			,&MaterialTranslator),MCT_Float1);
		SpecularColorChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_SpecularColor		,&MaterialTranslator),MCT_Float3);
		SpecularPowerChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_SpecularPower		,&MaterialTranslator),MCT_Float1);
		OpacityChunk				= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_Opacity				,&MaterialTranslator),MCT_Float1);
		MaskChunk					= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_OpacityMask			,&MaterialTranslator),MCT_Float1);
		DistortionChunk				= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_Distortion			,&MaterialTranslator),MCT_Float2);
		TwoSidedLightingMaskChunk	= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_TwoSidedLightingMask	,&MaterialTranslator),MCT_Float3);
		CustomLightingChunk			= MaterialTranslator.ForceCast(CompileProperty(MatPlatform, MP_CustomLighting		,&MaterialTranslator),MCT_Float3);
	}
	INC_FLOAT_STAT_BY(STAT_ShaderCompiling_HLSLTranslation,(FLOAT)HLSLTranslateTime);

	// Don't allow opaque and masked materials to sample scene color or scene depth as the results are undefined,
	// unless it's for a PP material effect.
	if (!IsTranslucentBlendMode(GetBlendMode()) && !IsUsedWithMaterialEffect())
	{
		if (bUsesSceneDepth)
		{
			MaterialTranslator.Errorf(TEXT("Only transparent materials can read from scene depth."));
		}
		if (bUsesSceneColor)
		{
			MaterialTranslator.Errorf(TEXT("Only transparent materials can read from scene color."));
		}
	}

	// Don't allow decals to sample scene color as we don't currently resolve scene color before decals
	if (bUsesSceneColor && IsUsedWithDecals())
	{
		MaterialTranslator.Errorf(TEXT("Materials used with decals must not read from scene color."));
	}

	UBOOL bSuccess = MaterialTranslator.bSuccess;

	FString InputsString = TEXT("");
	for(INT VectorIndex = 0;VectorIndex < UniformVectorExpressions.Num();VectorIndex++)
	{
		InputsString += FString::Printf(TEXT("float4 UniformVector_%i;\r\n"),VectorIndex);
	}
	for(INT ScalarIndex = 0;ScalarIndex < UniformScalarExpressions.Num();ScalarIndex++)
	{
		InputsString += FString::Printf(TEXT("float UniformScalar_%i;\r\n"),ScalarIndex);
	}
	for(INT TextureIndex = 0;TextureIndex < Uniform2DTextureExpressions.Num();TextureIndex++)
	{
		InputsString += FString::Printf(TEXT("sampler2D Texture2D_%i;\r\n"),TextureIndex);
	}
	for(INT TextureIndex = 0;TextureIndex < UniformCubeTextureExpressions.Num();TextureIndex++)
	{
		InputsString += FString::Printf(TEXT("samplerCUBE TextureCube_%i;\r\n"),TextureIndex);
	}

	FString MaterialTemplate = LoadShaderSourceFile(TEXT("MaterialTemplate"));

	const TCHAR * NormalCodeChunk = MaterialTranslator.GetFixedParameterCode(NormalChunk);
	const TCHAR * EmissiveColorCodeChunk = MaterialTranslator.GetFixedParameterCode(EmissiveColorChunk);
	const TCHAR * DiffuseColorCodeChunk = MaterialTranslator.GetFixedParameterCode(DiffuseColorChunk);
	const TCHAR * DiffusePowerCodeChunk = MaterialTranslator.GetFixedParameterCode(DiffusePowerChunk);
	const TCHAR * SpecularColorCodeChunk = MaterialTranslator.GetFixedParameterCode(SpecularColorChunk);
	const TCHAR * SpecularPowerCodeChunk = MaterialTranslator.GetFixedParameterCode(SpecularPowerChunk);
	const TCHAR * OpacityCodeChunk = MaterialTranslator.GetFixedParameterCode(OpacityChunk);
	const TCHAR * MaskCodeChunk = MaterialTranslator.GetFixedParameterCode(MaskChunk);
	const TCHAR * DistortionCodeChunk = MaterialTranslator.GetFixedParameterCode(DistortionChunk);
	const TCHAR * TwoSidedLightingMaskCodeChunk = MaterialTranslator.GetFixedParameterCode(TwoSidedLightingMaskChunk);
	const TCHAR * CustomLightingCodeChunk = MaterialTranslator.GetFixedParameterCode(CustomLightingChunk);
	const TCHAR * OpacityMaskClipValue = *FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue());


	FString MaterialShaderCode = FString::Printf(
		*MaterialTemplate,
		NumUserTexCoords,
		*InputsString,
		NormalCodeChunk,
		EmissiveColorCodeChunk,
		DiffuseColorCodeChunk,
		DiffusePowerCodeChunk,
		SpecularColorCodeChunk,
		SpecularPowerCodeChunk,
		OpacityCodeChunk,
		MaskCodeChunk,
		*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
		DistortionCodeChunk,
		TwoSidedLightingMaskCodeChunk,
		CustomLightingCodeChunk
		);

	if(bSuccess)
	{
		DroppedFallbackComponents = DroppedFallback_None;
		// Don't log compile errors if a fallback is being compiled, or if material shader errors are being ignored for this platform/game combination.
		const UBOOL bSilentCompile = bCompileFallback || !FMaterial::ShouldWarnOnBrokenMaterial(Platform);
		bSuccess = FallbackCompile(StaticParameters, Platform, OutShaderMap, MaterialShaderCode, bForceCompile, bSilentCompile, bDebugDump);

		//if the material didn't compile unchanged and we are compiling a fallback, try to remove components of the material until it compiles
		if (!bSuccess && bCompileFallback)
		{
			//remove specular
			DroppedFallbackComponents |= DroppedFallback_Specular;
			MaterialShaderCode = FString::Printf(
				*MaterialTemplate,
				NumUserTexCoords,
				*InputsString,
				NormalCodeChunk,
				EmissiveColorCodeChunk,
				DiffuseColorCodeChunk,
				DiffusePowerCodeChunk,
				TEXT("float3(0,0,0)"),
				TEXT("0"),
				OpacityCodeChunk,
				MaskCodeChunk,
				*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
				DistortionCodeChunk,
				TwoSidedLightingMaskCodeChunk,
				CustomLightingCodeChunk
				);

			CompileErrors.Empty();
			bSuccess = FallbackCompile(StaticParameters, Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE, bDebugDump);

			if (!bSuccess)
			{
				//remove specular and normal
				DroppedFallbackComponents |= DroppedFallback_Normal;
				MaterialShaderCode = FString::Printf(
					*MaterialTemplate,
					NumUserTexCoords,
					*InputsString,
					TEXT("float3(0,0,1)"),
					EmissiveColorCodeChunk,
					DiffuseColorCodeChunk,
					DiffusePowerCodeChunk,
					TEXT("float3(0,0,0)"),
					TEXT("0"),
					OpacityCodeChunk,
					MaskCodeChunk,
					*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
					DistortionCodeChunk,
					TwoSidedLightingMaskCodeChunk,
					CustomLightingCodeChunk
					);

				CompileErrors.Empty();
				bSuccess = FallbackCompile(StaticParameters, Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE, bDebugDump);

				if (!bSuccess)
				{
					//remove specular, normal and diffuse
					DroppedFallbackComponents |= DroppedFallback_Diffuse;
					MaterialShaderCode = FString::Printf(
						*MaterialTemplate,
						NumUserTexCoords,
						*InputsString,
						TEXT("float3(0,0,1)"),
						EmissiveColorCodeChunk,
						// Green diffuse so that it is obvious
						TEXT("float3(0,1,0)"),
						TEXT("1.0"),
						TEXT("float3(0,0,0)"),
						TEXT("0"),
						OpacityCodeChunk,
						MaskCodeChunk,
						*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
						DistortionCodeChunk,
						TwoSidedLightingMaskCodeChunk,
						CustomLightingCodeChunk
						);

					CompileErrors.Empty();
					bSuccess = FallbackCompile(StaticParameters, Platform, OutShaderMap, MaterialShaderCode, bForceCompile, TRUE, bDebugDump);

					if (!bSuccess)
					{
						//remove specular, normal, diffuse and emissive
						DroppedFallbackComponents |= DroppedFallback_Emissive;
						MaterialShaderCode = FString::Printf(
							*MaterialTemplate,
							NumUserTexCoords,
							*InputsString,
							TEXT("float3(0,0,1)"),
							// Red emissive, don't want these to go unnoticed
							TEXT("float3(1,0,0)"),
							TEXT("1.0"),
							TEXT("float3(0,0,0)"),
							TEXT("float3(0,0,0)"),
							TEXT("0"),
							OpacityCodeChunk,
							MaskCodeChunk,
							*FString::Printf(TEXT("%.5f"),GetOpacityMaskClipValue()),
							DistortionCodeChunk,
							TwoSidedLightingMaskCodeChunk,
							CustomLightingCodeChunk
							);

						CompileErrors.Empty();

						//Final chance for automatic fallback to work.  
						bSuccess = FallbackCompile(StaticParameters, Platform, OutShaderMap, MaterialShaderCode, bForceCompile, !FMaterial::ShouldWarnOnBrokenMaterial(Platform), bDebugDump);
					}
				}
			}

			if (!bSuccess)
			{
				DroppedFallbackComponents |= DroppedFallback_Failed;
				if (!GIsUnattended)
				{
					warnf(TEXT("Automatic SM2 Fallback for Material %s failed!  Please provide a Fallback Material."), *GetFriendlyName());
				}
			}
		}
	}


	if( GIsEditor && !GIsUCC )
	{
		// We called StatusUpdatef earlier, so we want to restore whatever the previous status was now
		GWarn->PopStatus();
	}

	return bSuccess;
}

/**
* Compiles OutShaderMap using the shader code from MaterialShaderCode on Platform
*
* @param StaticParameters - the set of static parameters to compile
* @param Platform - the platform to compile for
* @param OutShaderMap - the shader map to compile
* @param MaterialShaderCode - a filled out instance of MaterialTemplate.usf to compile
* @param bForceCompile - force discard previous results 
* @param bSilent - indicates that no error message should be outputted on shader compile failure
* @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
* @return - TRUE if compile succeeded or was not necessary (shader map for StaticParameters was found and was complete)
*/
UBOOL FMaterial::FallbackCompile( 
	 FStaticParameterSet* StaticParameters, 
	 EShaderPlatform Platform, 
	 TRefCountPtr<FMaterialShaderMap>& OutShaderMap, 
	 FString MaterialShaderCode, 
	 UBOOL bForceCompile, 
	 UBOOL bSilent,
	 UBOOL bDebugDump)
{

	FMaterialShaderMap* ExistingShaderMap = NULL;

	// if we want to force compile the material, there's no reason to check for an existing one
	if (bForceCompile)
	{
		UShaderCache::FlushId(*StaticParameters, Platform);
		ShaderMap = NULL;
	}
	else
	{
		// see if it's already compiled
		ExistingShaderMap = FMaterialShaderMap::FindId(*StaticParameters, Platform);
	}

	OutShaderMap = ExistingShaderMap;
	if (!OutShaderMap)
	{
		// Create a shader map for the material on the given platform
		OutShaderMap = new FMaterialShaderMap;
	}

	UBOOL bSuccess = TRUE;
	if(!ExistingShaderMap || !ExistingShaderMap->IsComplete(this, FALSE))
	{
		// Compile the shaders for the material.
		bSuccess = OutShaderMap->Compile(this,StaticParameters,*MaterialShaderCode,Platform,CompileErrors,bSilent,bDebugDump);
	}

	if(bSuccess)
	{
		OutShaderMap->BeginInit();
	}
	else
	{
		// Clear the shader map if the compilation failed, to ensure that the incomplete shader map isn't used.
		OutShaderMap = NULL;
	}

	// Store that we have up date compilation output.
	bValidCompilationOutput = TRUE;

	return bSuccess;
}

/**
* Caches the material shaders for this material with no static parameters on the given platform.
*/
UBOOL FMaterial::CacheShaders(EShaderPlatform Platform, UBOOL bCompileFallback)
{
	// Discard the ID and make a new one.
	Id = appCreateGuid();

	FStaticParameterSet EmptySet(Id);
	return CacheShaders(&EmptySet, Platform, bCompileFallback, TRUE);
}

/**
* Caches the material shaders for the given static parameter set and platform
*/
UBOOL FMaterial::CacheShaders(FStaticParameterSet* StaticParameters, EShaderPlatform Platform, UBOOL bCompileFallback, UBOOL bFlushExistingShaderMap, UBOOL bDebugDump)
{
	//flush the render command queue before changing the ShaderMap, since the rendering thread may be reading from it
	FlushRenderingCommands();

	if (bFlushExistingShaderMap)
	{
		FlushShaderMap();
	}

	if(!Id.IsValid())
	{
		// If the material doesn't have a valid ID, create a new one. 
		// This can happen if it is a new StaticPermutationResource, since it will never go through InitShaderMap().
		// On StaticPermutationResources, this Id is only used to track dirty state and not to find the matching shader map.
		Id = appCreateGuid();
	}

	// Reset the compile errors array.
	CompileErrors.Empty();

	//if we are compiling for a different platform than the existing shader map, release the reference to it
	if (ShaderMap && ShaderMap->GetShaderPlatform() != Platform)
	{
		ShaderMap = NULL;
	}

	// Compile the material shaders for the current platform.
	return Compile(StaticParameters, Platform, ShaderMap, bFlushExistingShaderMap, bCompileFallback, bDebugDump);
}

/**
 * Should the shader for this material with the given platform, shader type and vertex 
 * factory type combination be compiled
 *
 * @param Platform		The platform currently being compiled for
 * @param ShaderType	Which shader is being compiled
 * @param VertexFactory	Which vertex factory is being compiled (can be NULL)
 *
 * @return TRUE if the shader should be compiled
 */
UBOOL FMaterial::ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const
{
	// @GEMINI_TODO: Hook up to the precompiler to see if this material is used with this vertex factory
	// to stop compiling shaders for materials that are never on the given vertex factory
	return TRUE;
}

//
// FColoredMaterialRenderProxy implementation.
//

const FMaterial* FColoredMaterialRenderProxy::GetMaterial() const
{
	return Parent->GetMaterial();
}

/**
 * Finds the shader matching the template type and the passed in vertex factory, asserts if not found.
 */
FShader* FMaterial::GetShader(FMeshMaterialShaderType* ShaderType, FVertexFactoryType* VertexFactoryType) const
{
	const FMeshMaterialShaderMap* MeshShaderMap = ShaderMap->GetMeshShaderMap(VertexFactoryType);
	FShader* Shader = MeshShaderMap ? MeshShaderMap->GetShader(ShaderType) : NULL;
	if (!Shader)
	{
		// Get the ShouldCache results that determine whether the shader should be compiled
		UBOOL bMaterialShouldCache = ShouldCache(GRHIShaderPlatform, ShaderType, VertexFactoryType);
		UBOOL bVFShouldCache = VertexFactoryType->ShouldCache(GRHIShaderPlatform, this, ShaderType);
		UBOOL bShaderShouldCache = ShaderType->ShouldCache(GRHIShaderPlatform, this, VertexFactoryType);
		FString MaterialUsage = GetMaterialUsageDescription();

		// Assert with detailed information if the shader wasn't found for rendering.  
		// This is usually the result of an incorrect ShouldCache function.
		appErrorf(
			TEXT("Couldn't find Shader %s for Material Resource %s!\n")
			TEXT("		With VF=%s, Platform=%s \n")
			TEXT("		ShouldCache: Mat=%u, VF=%u, Shader=%u \n")
			TEXT("		Material Usage = %s"),
			ShaderType->GetName(), 
			*GetFriendlyName(),
			VertexFactoryType->GetName(),
			ShaderPlatformToText(GRHIShaderPlatform),
			bMaterialShouldCache,
			bVFShouldCache,
			bShaderShouldCache,
			*MaterialUsage
			);
	}

	return Shader;
}

/** Returns a string that describes the material's usage for debugging purposes. */
FString FMaterial::GetMaterialUsageDescription() const
{
	FString Part1 = FString::Printf(TEXT("%s, %s, Special=%u, TwoSided=%u, Masked=%u, Distorted=%u, Skinned=%u, "), 
		*GetLightingModelString(GetLightingModel()), *GetBlendModeString(GetBlendMode()), 
		IsSpecialEngineMaterial(), IsTwoSided(), IsMasked(), IsDistorted(), IsUsedWithSkeletalMesh());
	FString Part2 = FString::Printf(TEXT("Particle=%u, ParticleSprites=%u, ParticleSubUV=%u, BeamTrails=%u, Terrain=%u, Decal=%u, "),
		IsUsedWithParticleSystem(), IsUsedWithParticleSprites(), IsUsedWithParticleSubUV(), IsUsedWithBeamTrails(), IsTerrainMaterial(), IsUsedWithDecals());
	FString Part3 = FString::Printf(TEXT("Speedtree=%u, Foliage=%u, LensFlare=%u, StaticLighting=%u, Gamma=%u, InstMesh=%u, FluidSurf=%u, Fracture=%u"), 
		IsUsedWithSpeedTree(), IsUsedWithFoliage(), IsUsedWithLensFlare(), IsUsedWithStaticLighting(), IsUsedWithGammaCorrection(), 
		IsUsedWithInstancedMeshParticles(), IsUsedWithFluidSurfaces(), IsUsedWithFracturedMeshes());

	return FString::Printf(TEXT("%s%s%s"), *Part1, *Part2, *Part3);
}

/** Indicates whether warnings should be logged on the given platform. */
UBOOL FMaterial::ShouldWarnOnBrokenMaterial(EShaderPlatform ShaderPlatform)
{
#if GAMENAME == GEARGAME
	if (ShaderPlatform == SP_PCD3D_SM2)
	{
		return FALSE;
	}
#endif
	return TRUE;
}

/** Rebuilds the information about all texture lookups. */
void FMaterial::RebuildTextureLookupInfo( UMaterial *Material )
{
	TextureLookups.Empty();

	INT NumExpressions = Material->Expressions.Num();
	for(INT ExpressionIndex = 0;ExpressionIndex < NumExpressions; ExpressionIndex++)
	{
		UMaterialExpression* Expression = Material->Expressions(ExpressionIndex);
		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Expression);
		UMaterialExpressionTextureSampleParameter2D* TextureSampleParameter = Cast<UMaterialExpressionTextureSampleParameter2D>(Expression);

		if(TextureSample)
		{
			FTextureLookup Lookup;
			Lookup.TexCoordIndex = 0;
			Lookup.TextureIndex = -1;
			Lookup.UScale = 1.0f;
			Lookup.VScale = 1.0f;

			//@TODO: Check to see if this texture lookup is actually used.

			if ( TextureSample->Coordinates.Expression )
			{
				UMaterialExpressionTextureCoordinate* TextureCoordinate =
					Cast<UMaterialExpressionTextureCoordinate>( TextureSample->Coordinates.Expression );
				if ( TextureCoordinate )
				{
					// Use the specified texcoord.
					Lookup.TexCoordIndex = TextureCoordinate->CoordinateIndex;
					Lookup.UScale = TextureCoordinate->UTiling;
					Lookup.VScale = TextureCoordinate->VTiling;
				}
				else
				{
					// Too complex texcoord expression, ignore.
					continue;
				}
			}

			// Find where the texture is stored in the Uniform2DTextureExpressions array.
			if ( TextureSampleParameter )
			{
				FMaterialUniformExpressionTextureParameter TextureExpression(TextureSampleParameter->ParameterName, TextureSampleParameter->Texture);
				Lookup.TextureIndex = FindExpression( Uniform2DTextureExpressions, TextureExpression );
			}
			else if ( TextureSample->Texture )
			{
				FMaterialUniformExpressionTexture TextureExpression(TextureSample->Texture);
				Lookup.TextureIndex = FindExpression( Uniform2DTextureExpressions, TextureExpression );
			}

			if ( Lookup.TextureIndex >= 0 )
			{
				TextureLookups.AddItem( Lookup );
			}
		}
	}
}

/** Returns the index to the Expression in the Expressions array, or -1 if not found. */
INT FMaterial::FindExpression( const TArray<TRefCountPtr<FMaterialUniformExpression> >&Expressions, const FMaterialUniformExpression &Expression )
{
	for (INT ExpressionIndex = 0; ExpressionIndex < Expressions.Num(); ++ExpressionIndex)
	{
		if ( Expressions(ExpressionIndex)->IsIdentical(&Expression) )
		{
			return ExpressionIndex;
		}
	}
	return -1;
}


/** Serialize a texture lookup info. */
void FMaterial::FTextureLookup::Serialize(FArchive& Ar)
{
	Ar << TexCoordIndex;
	Ar << TextureIndex;
	if( Ar.Ver() < VER_FONT_FORMAT_AND_UV_TILING_CHANGES )
	{
		// Legacy versions only stored a single scalar for texture tiling
		FLOAT UAndVScale = 1.0f;
		Ar << UAndVScale;
		UScale = UAndVScale;
		VScale = UAndVScale;
	}
	else
	{
		Ar << UScale;
		Ar << VScale;
	}
}

/*-----------------------------------------------------------------------------
FColoredMaterialRenderProxy
-----------------------------------------------------------------------------*/

UBOOL FColoredMaterialRenderProxy::GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
{
	if(ParameterName == NAME_Color)
	{
		*OutValue = Color;
		return TRUE;
	}
	else
	{
		return Parent->GetVectorValue(ParameterName, OutValue, Context);
	}
}

UBOOL FColoredMaterialRenderProxy::GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetScalarValue(ParameterName, OutValue, Context);
}

UBOOL FColoredMaterialRenderProxy::GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetTextureValue(ParameterName,OutValue,Context);
}

/*-----------------------------------------------------------------------------
FTexturedMaterialRenderProxy
-----------------------------------------------------------------------------*/

const FMaterial* FTexturedMaterialRenderProxy::GetMaterial() const
{
	return Parent->GetMaterial();
}

UBOOL FTexturedMaterialRenderProxy::GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetVectorValue(ParameterName, OutValue, Context);
}
	
UBOOL FTexturedMaterialRenderProxy::GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetScalarValue(ParameterName, OutValue, Context);
}

UBOOL FTexturedMaterialRenderProxy::GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
{
	if(ParameterName == NAME_Texture)
	{
		*OutValue = Texture;
		return TRUE;
	}
	else
	{
		return Parent->GetTextureValue(ParameterName,OutValue,Context);
	}
}

/*-----------------------------------------------------------------------------
FFontMaterialRenderProxy
-----------------------------------------------------------------------------*/

const class FMaterial* FFontMaterialRenderProxy::GetMaterial() const
{
	return Parent->GetMaterial();
}

UBOOL FFontMaterialRenderProxy::GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetVectorValue(ParameterName, OutValue, Context);
}

UBOOL FFontMaterialRenderProxy::GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
{
	return Parent->GetScalarValue(ParameterName, OutValue, Context);
}

UBOOL FFontMaterialRenderProxy::GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
{
	// find the matching font parameter
	if( ParameterName == FontParamName &&
		Font->Textures.IsValidIndex(FontPage) )
	{
		// use the texture page from the font specified for the parameter
		UTexture2D* Texture = Font->Textures(FontPage);
		if( Texture && Texture->Resource )
		{
			*OutValue = Texture->Resource;
			return TRUE;
		}		
	}
	// try parent if not valid parameter
	return Parent->GetTextureValue(ParameterName,OutValue,Context);
}


/** Returns the number of samplers used in this material. */
INT FMaterialResource::GetSamplerUsage() const
{
	INT TextureParameters = 0;
	TArray<UTexture*> Textures;

	const TArray<TRefCountPtr<FMaterialUniformExpression> >* ExpressionsByType[2] =
	{
		&GetUniform2DTextureExpressions(),
		&GetUniformCubeTextureExpressions()
	};

	for(INT TypeIndex = 0;TypeIndex < ARRAY_COUNT(ExpressionsByType);TypeIndex++)
	{
		const TArray<TRefCountPtr<FMaterialUniformExpression> >& Expressions = *ExpressionsByType[TypeIndex];

		// Iterate over each of the material's texture expressions.
		for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
		{
			UTexture* Texture = NULL;
			FMaterialUniformExpression* Expression = Expressions(ExpressionIndex);
			Expression->GetGameThreadTextureValue(Material,&Texture);

			// TextureParameter expressions always count as one sampler.
			if( Expression->GetType() == &FMaterialUniformExpressionTextureParameter::StaticType )
			{
				TextureParameters++;
			}
			else if( Texture )
			{
				Textures.AddUniqueItem(Texture);
			}
		}
	}

	return Textures.Num() + TextureParameters;
}
