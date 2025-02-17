/*=============================================================================
	UnMaterial.cpp: Shader implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "EngineDecalClasses.h"

IMPLEMENT_CLASS(UMaterial);

FMaterialResource::FMaterialResource(UMaterial* InMaterial):
	Material(InMaterial)
{
}

INT FMaterialResource::CompileProperty(EMaterialShaderPlatform MatPlatform,EMaterialProperty Property,FMaterialCompiler* Compiler) const
{
	INT SelectionColorIndex = Compiler->ComponentMask(Compiler->VectorParameter(NAME_SelectionColor,FLinearColor::Black),1,1,1,0);

	switch(Property)
	{
	case MP_EmissiveColor:
		return Compiler->Add(Compiler->ForceCast(Material->EmissiveColor.Compile(Compiler,FColor(0,0,0)),MCT_Float3),SelectionColorIndex);
	case MP_Opacity: return Material->Opacity.Compile(Compiler,1.0f);
	case MP_OpacityMask: return Material->OpacityMask.Compile(Compiler,1.0f);
	case MP_Distortion: return Material->Distortion.Compile(Compiler,FVector2D(0,0));
	case MP_TwoSidedLightingMask: return Compiler->Mul(Compiler->ForceCast(Material->TwoSidedLightingMask.Compile(Compiler,0.0f),MCT_Float),Material->TwoSidedLightingColor.Compile(Compiler,FColor(255,255,255)));
	case MP_DiffuseColor:
		return Compiler->Mul(Compiler->ForceCast(Material->DiffuseColor.Compile(Compiler,FColor(0,0,0)),MCT_Float3),Compiler->Sub(Compiler->Constant(1.0f),SelectionColorIndex));
	case MP_DiffusePower:
		return Material->DiffusePower.Compile(Compiler,1.0f);
	case MP_SpecularColor: return Material->SpecularColor.Compile(Compiler,FColor(0,0,0));
	case MP_SpecularPower: return Material->SpecularPower.Compile(Compiler,15.0f);
	case MP_Normal: return Material->Normal.Compile(Compiler,FVector(0,0,1));
	case MP_CustomLighting: return Material->CustomLighting.Compile(Compiler,FColor(0,0,0));
	default:
		return INDEX_NONE;
	};
}

/**
 * A resource which represents the default instance of a UMaterial to the renderer.
 * Note that default parameter values are stored in the FMaterialUniformExpressionXxxParameter objects now.
 * This resource is only responsible for the selection color.
 */
class FDefaultMaterialInstance : public FMaterialRenderProxy
{
public:

	// FMaterialRenderProxy interface.
	virtual const class FMaterial* GetMaterial() const
	{
		const FMaterialResource * MaterialResource = Material->GetMaterialResource();
		if (MaterialResource && MaterialResource->GetShaderMap())
		{
			return MaterialResource;
		}
	
		// this check is to stop the infinite "retry to compile DefaultMaterial" which can occur when MSP types are mismatched or another similar error state
		check(this != GEngine->DefaultMaterial->GetRenderProxy(bSelected));	
		return GEngine->DefaultMaterial->GetRenderProxy(bSelected)->GetMaterial();
	}
	virtual UBOOL GetVectorValue(const FName& ParameterName, FLinearColor* OutValue, const FMaterialRenderContext& Context) const
	{
		const FMaterialResource * MaterialResource = Material->GetMaterialResource();
		if(MaterialResource && MaterialResource->GetShaderMap())
		{
			if(ParameterName == NAME_SelectionColor)
			{
				static const FLinearColor SelectionColor(10.0f/255.0f,5.0f/255.0f,60.0f/255.0f,1);
				*OutValue = bSelected ? SelectionColor : FLinearColor::Black;
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetRenderProxy(bSelected)->GetVectorValue(ParameterName, OutValue, Context);
		}
	}
	virtual UBOOL GetScalarValue(const FName& ParameterName, FLOAT* OutValue, const FMaterialRenderContext& Context) const
	{
		const FMaterialResource * MaterialResource = Material->GetMaterialResource();
		if(MaterialResource && MaterialResource->GetShaderMap())
		{
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetRenderProxy(bSelected)->GetScalarValue(ParameterName, OutValue, Context);
		}
	}
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue, const FMaterialRenderContext& Context) const
	{
		const FMaterialResource * MaterialResource = Material->GetMaterialResource();
		if(MaterialResource && MaterialResource->GetShaderMap())
		{
			return FALSE;
		}
		else
		{
			return GEngine->DefaultMaterial->GetRenderProxy(bSelected)->GetTextureValue(ParameterName,OutValue,Context);
		}
	}

	// Constructor.
	FDefaultMaterialInstance(UMaterial* InMaterial,UBOOL bInSelected):
		Material(InMaterial),
		bSelected(bInSelected)
	{}

private:

	UMaterial* Material;
	UBOOL bSelected;
};

UMaterial::UMaterial()
{
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		DefaultMaterialInstances[FALSE] = new FDefaultMaterialInstance(this,FALSE);
		if(GIsEditor)
		{
			DefaultMaterialInstances[TRUE] = new FDefaultMaterialInstance(this,TRUE);
		}
	}

	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		MaterialResources[PlatformIndex] = NULL;
	}
}

/** @return TRUE if the material uses distortion */
UBOOL UMaterial::HasDistortion() const
{
    return !bUseOneLayerDistortion && bUsesDistortion && IsTranslucentBlendMode((EBlendMode)BlendMode);
}

/** @return TRUE if the material uses the scene color texture */
UBOOL UMaterial::UsesSceneColor() const
{
	check(MaterialResources[GCurrentMaterialPlatform]);
	return MaterialResources[GCurrentMaterialPlatform]->GetUsesSceneColor();
}

/**
* Allocates a material resource off the heap to be stored in MaterialResource.
*/
FMaterialResource* UMaterial::AllocateResource()
{
	return new FMaterialResource(this);
}

/**
 * Gathers the textures used to render the material instance.
 * 
 * @param OutTextures	Upon return contains the textures used to render the material instance.
 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
 */
void UMaterial::GetTextures(TArray<UTexture*>& OutTextures, UBOOL bOnlyAddOnce )
{
	// Gather texture references from every material resource
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		UMaterial* Material = GetMaterial((EMaterialShaderPlatform)PlatformIndex);
		if(!Material)
		{
			// If the material instance has no material, use the default material.
			GEngine->DefaultMaterial->GetTextures(OutTextures);
			continue;
		}

		check(Material->MaterialResources[PlatformIndex]);

		// Iterate over both the 2D textures and cube texture expressions.
		const TArray<TRefCountPtr<FMaterialUniformExpression> >* ExpressionsByType[2] =
		{
			&Material->MaterialResources[PlatformIndex]->GetUniform2DTextureExpressions(),
			&Material->MaterialResources[PlatformIndex]->GetUniformCubeTextureExpressions()
		};
		for(INT TypeIndex = 0;TypeIndex < ARRAY_COUNT(ExpressionsByType);TypeIndex++)
		{
			const TArray<TRefCountPtr<FMaterialUniformExpression> >& Expressions = *ExpressionsByType[TypeIndex];

			// Iterate over each of the material's texture expressions.
			for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
			{
				FMaterialUniformExpression* Expression = Expressions(ExpressionIndex);

				// Evaluate the expression in terms of this material instance.
				UTexture* Texture = NULL;
				Expression->GetGameThreadTextureValue(this,&Texture);

				// Add the expression's value to the output array.
				if( bOnlyAddOnce )
				{
					OutTextures.AddUniqueItem(Texture);
				}
				else
				{
					OutTextures.AddItem(Texture);
				}
			}
		}
	}
}

/** Gets the value associated with the given usage flag. */
UBOOL UMaterial::GetUsageByFlag(EMaterialUsage Usage)
{
	UBOOL UsageValue = FALSE;
	switch(Usage)
	{
		case MATUSAGE_SkeletalMesh: UsageValue = bUsedWithSkeletalMesh; break;
		case MATUSAGE_FracturedMeshes: UsageValue = bUsedWithFracturedMeshes; break;
		case MATUSAGE_ParticleSprites: UsageValue = bUsedWithParticleSprites; break;
		case MATUSAGE_BeamTrails: UsageValue = bUsedWithBeamTrails; break;
		case MATUSAGE_ParticleSubUV: UsageValue = bUsedWithParticleSubUV; break;
		case MATUSAGE_Foliage: UsageValue = bUsedWithFoliage; break;
		case MATUSAGE_SpeedTree: UsageValue = bUsedWithSpeedTree; break;
		case MATUSAGE_StaticLighting: UsageValue = bUsedWithStaticLighting; break;
		case MATUSAGE_GammaCorrection: UsageValue = bUsedWithGammaCorrection; break;
		case MATUSAGE_LensFlare: UsageValue = bUsedWithLensFlare; break;
		case MATUSAGE_InstancedMeshParticles: UsageValue = bUsedWithInstancedMeshParticles; break;
		case MATUSAGE_FluidSurface: UsageValue = bUsedWithFluidSurfaces; break;
		case MATUSAGE_Decals: UsageValue = bUsedWithDecals; break;
		case MATUSAGE_MaterialEffect: UsageValue = bUsedWithMaterialEffect; break;
		default: appErrorf(TEXT("Unknown material usage: %u"), (INT)Usage);
	};
	return UsageValue;
}

/** Sets the value associated with the given usage flag. */
void UMaterial::SetUsageByFlag(EMaterialUsage Usage, UBOOL NewValue)
{
	switch(Usage)
	{
		case MATUSAGE_SkeletalMesh: bUsedWithSkeletalMesh = NewValue; break;
		case MATUSAGE_FracturedMeshes: bUsedWithFracturedMeshes = NewValue; break;
		case MATUSAGE_ParticleSprites: bUsedWithParticleSprites = NewValue; break;
		case MATUSAGE_BeamTrails: bUsedWithBeamTrails = NewValue; break;
		case MATUSAGE_ParticleSubUV: bUsedWithParticleSubUV = NewValue; break;
		case MATUSAGE_Foliage: bUsedWithFoliage = NewValue; break;
		case MATUSAGE_SpeedTree: bUsedWithSpeedTree = NewValue; break;
		case MATUSAGE_StaticLighting: bUsedWithStaticLighting = NewValue; break;
		case MATUSAGE_GammaCorrection: bUsedWithGammaCorrection = NewValue; break;
		case MATUSAGE_LensFlare: bUsedWithLensFlare = NewValue; break;
		case MATUSAGE_InstancedMeshParticles: bUsedWithInstancedMeshParticles = NewValue; break;
		case MATUSAGE_FluidSurface: bUsedWithFluidSurfaces = NewValue; break;
		case MATUSAGE_Decals: bUsedWithDecals = NewValue; break;
		case MATUSAGE_MaterialEffect: bUsedWithMaterialEffect = NewValue; break;
		default: appErrorf(TEXT("Unknown material usage: %u"), (INT)Usage);
	};
}

/** Gets the name of the given usage flag. */
FString UMaterial::GetUsageName(EMaterialUsage Usage)
{
	FString UsageName = TEXT("");
	switch(Usage)
	{
		case MATUSAGE_SkeletalMesh: UsageName = TEXT("bUsedWithSkeletalMesh"); break;
		case MATUSAGE_FracturedMeshes: UsageName = TEXT("bUsedWithFracturedMeshes"); break;
		case MATUSAGE_ParticleSprites: UsageName = TEXT("bUsedWithParticleSprites"); break;
		case MATUSAGE_BeamTrails: UsageName = TEXT("bUsedWithBeamTrails"); break;
		case MATUSAGE_ParticleSubUV: UsageName = TEXT("bUsedWithParticleSubUV"); break;
		case MATUSAGE_Foliage: UsageName = TEXT("bUsedWithFoliage"); break;
		case MATUSAGE_SpeedTree: UsageName = TEXT("bUsedWithSpeedTree"); break;
		case MATUSAGE_StaticLighting: UsageName = TEXT("bUsedWithStaticLighting"); break;
		case MATUSAGE_GammaCorrection: UsageName = TEXT("bUsedWithGammaCorrection"); break;
		case MATUSAGE_LensFlare: UsageName = TEXT("bUsedWithLensFlare"); break;
		case MATUSAGE_InstancedMeshParticles: UsageName = TEXT("bUsedWithInstancedMeshParticles"); break;
		case MATUSAGE_FluidSurface: UsageName = TEXT("bUsedWithFluidSurfaces"); break;
		case MATUSAGE_Decals: UsageName = TEXT("bUsedWithDecals"); break;
		case MATUSAGE_MaterialEffect: UsageName = TEXT("bUsedWithMaterialEffect"); break;
		default: appErrorf(TEXT("Unknown material usage: %u"), (INT)Usage);
	};
	return UsageName;
}

/**
 * Checks if the material can be used with the given usage flag.
 * If the flag isn't set in the editor, it will be set and the material will be recompiled with it.
 * @param Usage - The usage flag to check
 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
 */
UBOOL UMaterial::CheckMaterialUsage(EMaterialUsage Usage)
{
	UBOOL bNeedsRecompile = FALSE;
	return SetMaterialUsage(bNeedsRecompile, Usage);
}

/**
 * Sets the given usage flag.
 * @param bNeedsRecompile - TRUE if the material was recompiled for the usage change
 * @param Usage - The usage flag to set
 * @return UBOOL - TRUE if the material can be used for rendering with the given type.
 */
UBOOL UMaterial::SetMaterialUsage(UBOOL &bNeedsRecompile, EMaterialUsage Usage)
{
	bNeedsRecompile = FALSE;
	UBOOL bFallbackCanBeUsed = TRUE;
	if (FallbackMaterial && !bIsFallbackMaterial)
	{
		// Also set usage on fallback material
		bFallbackCanBeUsed = FallbackMaterial->SetMaterialUsage(bNeedsRecompile, Usage);
	}

	// Check that the material has been flagged for use with the given usage flag.
	if(!GetUsageByFlag(Usage) && !bUsedAsSpecialEngineMaterial)
	{
        // For materials which do not have their bUsedWith____ correctly set the DefaultMaterial<type> should be used in game
        // Leaving this GIsEditor ensures that in game on PC will not look different than on the Consoles as we will not be compiling shaders on the fly
		if( GIsEditor == TRUE )
		{
			// If the flag is missing in the editor, set it, and recompile shaders.
			SetUsageByFlag(Usage, TRUE);
			bNeedsRecompile = TRUE;
			CacheResourceShaders(GRHIShaderPlatform);

			// Go through all loaded material instances and recompile their static permutation resources if needed
			// This is necessary since the parent UMaterial stores information about how it should be rendered, (eg bUsesDistortion)
			// but the child can have its own shader map which may not contain all the shaders that the parent's settings indicate that it should.
			// this code is duplicated in NewMaterialEditor.cpp WxMaterialEditor::UpdateOriginalMaterial()
			for (TObjectIterator<UMaterialInstance> It; It; ++It)
			{
				UMaterialInstance* CurrentMaterialInstance = *It;
				UMaterial* BaseMaterial = CurrentMaterialInstance->GetMaterial();
				//only recompile if the instance is a child of the material that got updated
				if (this == BaseMaterial)
				{
					CurrentMaterialInstance->InitStaticPermutation();
				}
			}

			warnf(NAME_Warning, TEXT("Material %s needed to have new flag set %s !"), *GetPathName(), *GetUsageName(Usage));
			// Mark the package dirty so that hopefully it will be saved with the new usage flag.
			MarkPackageDirty();
		}
		else
		{
			warnf(NAME_Warning, TEXT("Material %s missing %s=True!"), *GetPathName(), *GetUsageName(Usage));

			// Return failure if the flag is missing in game, since compiling shaders in game is not supported on some platforms.
			return FALSE;
		}
	}
	// Only allow rendering with this material if both the fallback and base materials were able to set the usage.
	// This avoids needing to know the current platform that usage is being requested for so that it can be set from a commandlet on any platform.
	return bFallbackCanBeUsed;
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of vector parameter names used in this material.
 */
void UMaterial::GetAllVectorParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionVectorParameter* VectorSampleParameter =
			Cast<UMaterialExpressionVectorParameter>(Expressions(ExpressionIndex));

		if(VectorSampleParameter)
		{
			INT CurrentSize = OutParameterNames.Num();
			OutParameterNames.AddUniqueItem(VectorSampleParameter->ParameterName);
			
			if(CurrentSize != OutParameterNames.Num())
			{
				OutParameterIds.AddItem(VectorSampleParameter->ExpressionGUID);
			}
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of scalar parameter names used in this material.
 */
void UMaterial::GetAllScalarParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionScalarParameter* ScalarSampleParameter =
			Cast<UMaterialExpressionScalarParameter>(Expressions(ExpressionIndex));

		if(ScalarSampleParameter)
		{
			INT CurrentSize = OutParameterNames.Num();
			OutParameterNames.AddUniqueItem(ScalarSampleParameter->ParameterName);
			
			if(CurrentSize != OutParameterNames.Num())
			{
				OutParameterIds.AddItem(ScalarSampleParameter->ExpressionGUID);
			}
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of texture parameter names used in this material.
 */
void UMaterial::GetAllTextureParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionTextureSampleParameter* TextureSampleParameter =
			Cast<UMaterialExpressionTextureSampleParameter>(Expressions(ExpressionIndex));

		if(TextureSampleParameter)
		{
			INT CurrentSize = OutParameterNames.Num();
			OutParameterNames.AddUniqueItem(TextureSampleParameter->ParameterName);
			
			if(CurrentSize != OutParameterNames.Num())
			{
				OutParameterIds.AddItem(TextureSampleParameter->ExpressionGUID);
			}
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of font parameter names used in this material.
 */
void UMaterial::GetAllFontParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionFontSampleParameter* FontSampleParameter =
			Cast<UMaterialExpressionFontSampleParameter>(Expressions(ExpressionIndex));

		if(FontSampleParameter)
		{
			INT CurrentSize = OutParameterNames.Num();
			OutParameterNames.AddUniqueItem(FontSampleParameter->ParameterName);
			
			if(CurrentSize != OutParameterNames.Num())
			{
				OutParameterIds.AddItem(FontSampleParameter->ExpressionGUID);
			}
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of static switch parameter names used in this material.
 */
void UMaterial::GetAllStaticSwitchParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticSwitchParameter* SwitchParameter =
			Cast<UMaterialExpressionStaticSwitchParameter>(Expressions(ExpressionIndex));

		if(SwitchParameter)
		{
			INT CurrentSize = OutParameterNames.Num();
			OutParameterNames.AddUniqueItem(SwitchParameter->ParameterName);
			
			if(CurrentSize != OutParameterNames.Num())
			{
				OutParameterIds.AddItem(SwitchParameter->ExpressionGUID);
			}
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * @param	OutParameterNames		Storage array for the parameter names we are returning.
 * @param	OutParameterIds			Storage array for the parameter id's we are returning.
 *
 * @return	Returns a array of static component mask parameter names used in this material.
 */
void UMaterial::GetAllStaticComponentMaskParameterNames(TArray<FName> &OutParameterNames, TArray<FGuid> &OutParameterIds)
{
	OutParameterNames.Empty();
	OutParameterIds.Empty();
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticComponentMaskParameter* ComponentMaskParameter =
			Cast<UMaterialExpressionStaticComponentMaskParameter>(Expressions(ExpressionIndex));

		if(ComponentMaskParameter)
		{
			INT Index = OutParameterNames.AddUniqueItem(ComponentMaskParameter->ParameterName);
			OutParameterIds.InsertItem(ComponentMaskParameter->ExpressionGUID, Index);
		}
	}

	check(OutParameterNames.Num() == OutParameterIds.Num());
}

/**
 * Builds a string of parameters in the fallback material that do not exist in the base material.
 * These parameters won't be set by material instances, which get their parameter list from the base material.
 *
 * @param ParameterMismatches - string of unmatches material names to populate
 */
void UMaterial::GetFallbackParameterInconsistencies(FString &ParameterMismatches)
{
	ParameterMismatches.Empty();
	if (FallbackMaterial)
	{
		const UINT NumParamTypes = 5;
		TArray<FName> BaseParameterNames[NumParamTypes];
		TArray<FName> FallbackParameterNames[NumParamTypes];
		TArray<FGuid> Guids;

		//gather parameter names from the fallback material and the base material
		GetAllVectorParameterNames(BaseParameterNames[0], Guids);
		FallbackMaterial->GetAllVectorParameterNames(FallbackParameterNames[0], Guids);

		GetAllScalarParameterNames(BaseParameterNames[1], Guids);
		FallbackMaterial->GetAllScalarParameterNames(FallbackParameterNames[1], Guids);

		GetAllTextureParameterNames(BaseParameterNames[2], Guids);
		FallbackMaterial->GetAllTextureParameterNames(FallbackParameterNames[2], Guids);

		GetAllStaticSwitchParameterNames(BaseParameterNames[3], Guids);
		FallbackMaterial->GetAllStaticSwitchParameterNames(FallbackParameterNames[3], Guids);

		GetAllStaticComponentMaskParameterNames(BaseParameterNames[4], Guids);
		FallbackMaterial->GetAllStaticComponentMaskParameterNames(FallbackParameterNames[4], Guids);

		INT FoundIndex = 0;

		//go through each parameter type
		for (INT ParamTypeIndex = 0; ParamTypeIndex < NumParamTypes; ParamTypeIndex++)
		{
			//and iterate over each parameter in the fallback material
			for (INT ParamIndex = 0; ParamIndex < FallbackParameterNames[ParamTypeIndex].Num(); ParamIndex++)
			{
				//check if the fallback material parameter exists in the base material (this material)
				if (!BaseParameterNames[ParamTypeIndex].FindItem(FallbackParameterNames[ParamTypeIndex](ParamIndex), FoundIndex))
				{
					//the parameter wasn't found, add a warning
					FString ParameterName;
					FallbackParameterNames[ParamTypeIndex](ParamIndex).ToString(ParameterName);
					ParameterMismatches += FString(FString::Printf( TEXT("\n	 Parameter '%s' not found"), *ParameterName));
				}
			}
		}
	}
}

/**
 * Get the material which this is an instance of.
 * Warning - This is platform dependent!  Do not call GetMaterial(GCurrentMaterialPlatform) and save that reference,
 * as it will be different depending on the current platform.  Instead call GetMaterial(MSP_BASE) to get the base material and save that.
 * When getting the material for rendering/checking usage, GetMaterial(GCurrentMaterialPlatform) is fine.
 *
 * @param Platform - The platform to get material for.
 */
UMaterial* UMaterial::GetMaterial(EMaterialShaderPlatform Platform)
{
	//pass the request to the fallback material if one exists and this is a sm2 platform
	if (FallbackMaterial && Platform == MSP_SM2)
	{
		return FallbackMaterial->GetMaterial(Platform);
	}

	const FMaterialResource * MaterialResource = GetMaterialResource(Platform);

	if(MaterialResource)
	{
		return this;
	}
	else
	{
		return GEngine ? GEngine->DefaultMaterial : NULL;
	}
}

UBOOL UMaterial::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionVectorParameter* VectorParameter =
			Cast<UMaterialExpressionVectorParameter>(Expressions(ExpressionIndex));

		if(VectorParameter && VectorParameter->ParameterName == ParameterName)
		{
			OutValue = VectorParameter->DefaultValue;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

UBOOL UMaterial::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionScalarParameter* ScalarParameter =
			Cast<UMaterialExpressionScalarParameter>(Expressions(ExpressionIndex));

		if(ScalarParameter && ScalarParameter->ParameterName == ParameterName)
		{
			OutValue = ScalarParameter->DefaultValue;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

UBOOL UMaterial::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionTextureSampleParameter* TextureSampleParameter =
			Cast<UMaterialExpressionTextureSampleParameter>(Expressions(ExpressionIndex));

		if(TextureSampleParameter && TextureSampleParameter->ParameterName == ParameterName)
		{
			OutValue = TextureSampleParameter->Texture;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

UBOOL UMaterial::GetFontParameterValue(FName ParameterName,class UFont*& OutFontValue,INT& OutFontPage)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionFontSampleParameter* FontSampleParameter =
			Cast<UMaterialExpressionFontSampleParameter>(Expressions(ExpressionIndex));

		if(FontSampleParameter && FontSampleParameter->ParameterName == ParameterName)
		{
			OutFontValue = FontSampleParameter->Font;
			OutFontPage = FontSampleParameter->FontTexturePage;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

/**
 * Gets the value of the given static switch parameter
 *
 * @param	ParameterName	The name of the static switch parameter
 * @param	OutValue		Will contain the value of the parameter if successful
 * @return					True if successful
 */
UBOOL UMaterial::GetStaticSwitchParameterValue(FName ParameterName,UBOOL &OutValue,FGuid &OutExpressionGuid)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticSwitchParameter* StaticSwitchParameter =
			Cast<UMaterialExpressionStaticSwitchParameter>(Expressions(ExpressionIndex));

		if(StaticSwitchParameter && StaticSwitchParameter->ParameterName == ParameterName)
		{
			OutValue = StaticSwitchParameter->DefaultValue;
			OutExpressionGuid = StaticSwitchParameter->ExpressionGUID;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

/**
 * Gets the value of the given static component mask parameter
 *
 * @param	ParameterName	The name of the parameter
 * @param	R, G, B, A		Will contain the values of the parameter if successful
 * @return					True if successful
 */
UBOOL UMaterial::GetStaticComponentMaskParameterValue(FName ParameterName, UBOOL &OutR, UBOOL &OutG, UBOOL &OutB, UBOOL &OutA, FGuid &OutExpressionGuid)
{
	UBOOL bSuccess = FALSE;
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticComponentMaskParameter* StaticComponentMaskParameter =
			Cast<UMaterialExpressionStaticComponentMaskParameter>(Expressions(ExpressionIndex));

		if(StaticComponentMaskParameter && StaticComponentMaskParameter->ParameterName == ParameterName)
		{
			OutR = StaticComponentMaskParameter->DefaultR;
			OutG = StaticComponentMaskParameter->DefaultG;
			OutB = StaticComponentMaskParameter->DefaultB;
			OutA = StaticComponentMaskParameter->DefaultA;
			OutExpressionGuid = StaticComponentMaskParameter->ExpressionGUID;
			bSuccess = TRUE;
			break;
		}
	}
	return bSuccess;
}

FMaterialRenderProxy* UMaterial::GetRenderProxy(UBOOL Selected) const
{
	check(!Selected || GIsEditor);
	return DefaultMaterialInstances[Selected];
}

UPhysicalMaterial* UMaterial::GetPhysicalMaterial() const
{
	return PhysMaterial;
}

/**
* Compiles a FMaterialResource on the given platform with the given static parameters
*
* @param StaticParameters - The set of static parameters to compile for
* @param StaticPermutation - The resource to compile
* @param Platform - The platform to compile for
* @param MaterialPlatform - The material platform to compile for
* @param bFlushExistingShaderMaps - Indicates that existing shader maps should be discarded
* @return TRUE if compilation was successful or not necessary
*/
UBOOL UMaterial::CompileStaticPermutation(
	FStaticParameterSet* StaticParameters, 
	FMaterialResource* StaticPermutation,  
	EShaderPlatform Platform,
	EMaterialShaderPlatform MaterialPlatform,
	UBOOL bFlushExistingShaderMaps,
	UBOOL bDebugDump)
{
	UBOOL CompileSucceeded = FALSE;

	const UBOOL bIsSM2Platform = MaterialPlatform == MSP_SM2;

	//pass the request to the fallback material if one exists and this is a sm2 platform
	if (FallbackMaterial && bIsSM2Platform)
	{
		return FallbackMaterial->CompileStaticPermutation(StaticParameters, StaticPermutation, Platform, MaterialPlatform, bFlushExistingShaderMaps, bDebugDump);
	}

	//update the static parameter set with the base material's Id
	StaticParameters->BaseMaterialId = MaterialResources[MaterialPlatform]->GetId();

	SetStaticParameterOverrides(StaticParameters);

#if CONSOLE
	if (GUseSeekFreeLoading)
	{
		//uniform expressions are guaranteed to be updated since they are always generated during cooking
		CompileSucceeded = StaticPermutation->InitShaderMap(StaticParameters, Platform, bIsSM2Platform && !bIsFallbackMaterial);
	}
	else
#endif
	{
		//material instances with static permutations always need to regenerate uniform expressions, so InitShaderMap() is not used
		CompileSucceeded = StaticPermutation->CacheShaders(StaticParameters, Platform, bIsSM2Platform && !bIsFallbackMaterial, bFlushExistingShaderMaps, bDebugDump);
	}
	
	ClearStaticParameterOverrides();
	
	return CompileSucceeded;
}

/**
 * Sets overrides in the material's static parameters
 *
 * @param	Permutation		The set of static parameters to override and their values	
 */
void UMaterial::SetStaticParameterOverrides(const FStaticParameterSet* Permutation)
{
	check(IsInGameThread());

	//go through each expression
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticSwitchParameter* StaticSwitchExpression =
			Cast<UMaterialExpressionStaticSwitchParameter>(Expressions(ExpressionIndex));

		//if it's a static switch, setup the override
		if (StaticSwitchExpression)
		{
			for(INT SwitchIndex = 0;SwitchIndex < Permutation->StaticSwitchParameters.Num();SwitchIndex++)
			{
				const FStaticSwitchParameter * InstanceSwitchParameter = &Permutation->StaticSwitchParameters(SwitchIndex);
				if(StaticSwitchExpression->ParameterName == InstanceSwitchParameter->ParameterName)
				{
					StaticSwitchExpression->InstanceOverride = InstanceSwitchParameter;
					break;
				}
			}
		}
		else
		{
			UMaterialExpressionStaticComponentMaskParameter* StaticComponentMaskExpression =
				Cast<UMaterialExpressionStaticComponentMaskParameter>(Expressions(ExpressionIndex));

			//if it's a static component mask, setup the override
			if (StaticComponentMaskExpression)
			{
				for(INT ComponentMaskIndex = 0;ComponentMaskIndex < Permutation->StaticComponentMaskParameters.Num();ComponentMaskIndex++)
				{
					const FStaticComponentMaskParameter * InstanceComponentMaskParameter = &Permutation->StaticComponentMaskParameters(ComponentMaskIndex);
					if(StaticComponentMaskExpression->ParameterName == InstanceComponentMaskParameter->ParameterName)
					{
						StaticComponentMaskExpression->InstanceOverride = InstanceComponentMaskParameter;
						break;
					}
				}
			}
		}
	}

}

/**
 * Clears static parameter overrides so that static parameter expression defaults will be used
 *	for subsequent compiles.
 */
void UMaterial::ClearStaticParameterOverrides()
{
	//go through each expression
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpressionStaticSwitchParameter* StaticSwitchExpression =
			Cast<UMaterialExpressionStaticSwitchParameter>(Expressions(ExpressionIndex));

		//if it's a static switch, clear the override
		if(StaticSwitchExpression)
		{
			StaticSwitchExpression->InstanceOverride = NULL;
		}
		else
		{
			UMaterialExpressionStaticComponentMaskParameter* StaticComponentMaskExpression =
				Cast<UMaterialExpressionStaticComponentMaskParameter>(Expressions(ExpressionIndex));

			//if it's a static component mask, clear the override
			if(StaticComponentMaskExpression)
			{
				StaticComponentMaskExpression->InstanceOverride = NULL;
			}
		}
	}
}

/** Helper functions for text output of properties... */
#ifndef CASE_TEXT
#define CASE_TEXT(txt) case txt: return TEXT(#txt)
#endif

#ifndef TEXT_TO_ENUM
#define TEXT_TO_ENUM(eVal, txt)		if (appStricmp(TEXT(#eVal), txt) == 0)	return eVal;
#endif

const TCHAR* UMaterial::GetMaterialLightingModelString(EMaterialLightingModel InMaterialLightingModel)
{
	switch (InMaterialLightingModel)
	{
    CASE_TEXT(MLM_Phong);
    CASE_TEXT(MLM_NonDirectional);
    CASE_TEXT(MLM_Unlit);
    CASE_TEXT(MLM_SHPRT);
    CASE_TEXT(MLM_Custom);
	}
	return TEXT("MLM_Phong");
}

EMaterialLightingModel UMaterial::GetMaterialLightingModelFromString(const TCHAR* InMaterialLightingModelStr)
{
    TEXT_TO_ENUM(MLM_Phong, InMaterialLightingModelStr);
    TEXT_TO_ENUM(MLM_NonDirectional, InMaterialLightingModelStr);
    TEXT_TO_ENUM(MLM_Unlit, InMaterialLightingModelStr);
    TEXT_TO_ENUM(MLM_SHPRT, InMaterialLightingModelStr);
    TEXT_TO_ENUM(MLM_Custom, InMaterialLightingModelStr);

	return MLM_Phong;
}

const TCHAR* UMaterial::GetBlendModeString(EBlendMode InBlendMode)
{
	switch (InBlendMode)
	{
    CASE_TEXT(BLEND_Opaque);
    CASE_TEXT(BLEND_Masked);
    CASE_TEXT(BLEND_Translucent);
    CASE_TEXT(BLEND_Additive);
    CASE_TEXT(BLEND_Modulate);
	}
	return TEXT("BLEND_Opaque");
}

EBlendMode UMaterial::GetBlendModeFromString(const TCHAR* InBlendModeStr)
{
    TEXT_TO_ENUM(BLEND_Opaque, InBlendModeStr);
    TEXT_TO_ENUM(BLEND_Masked, InBlendModeStr);
    TEXT_TO_ENUM(BLEND_Translucent, InBlendModeStr);
    TEXT_TO_ENUM(BLEND_Additive, InBlendModeStr);
    TEXT_TO_ENUM(BLEND_Modulate, InBlendModeStr);

	return BLEND_Opaque;
}

/**
 * Compiles material resources for the current platform if the shader map for that resource didn't already exist.
 *
 * @param ShaderPlatform - platform to compile for
 * @param bFlushExistingShaderMaps - forces a compile, removes existing shader maps from shader cache.
 * @param bForceAllPlatforms - compile for all platforms, not just the current.
 */
void UMaterial::CacheResourceShaders(EShaderPlatform ShaderPlatform, UBOOL bFlushExistingShaderMaps, UBOOL bForceAllPlatforms)
{
	const EMaterialShaderPlatform RequestedMaterialPlatform = GetMaterialPlatform(ShaderPlatform);
	const EShaderPlatform OtherPlatformsToCompile[] = {SP_PCD3D_SM3, SP_PCD3D_SM2};

	check(ARRAY_COUNT(OtherPlatformsToCompile) == MSP_MAX);

	//go through each material resource
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		//allocate it if it hasn't already been
		if(!MaterialResources[PlatformIndex])
		{
			MaterialResources[PlatformIndex] = AllocateResource();
		}

		//don't ever compile for sm2 if a fallback material is present, 
		//since the fallback material overrides this material's sm2 material resource
		if (PlatformIndex == MSP_SM2 && FallbackMaterial)
		{
			continue;
		}

		const UBOOL bIsSM2Platform = PlatformIndex == MSP_SM2;
		EShaderPlatform CurrentShaderPlatform = ShaderPlatform;
		//if we're not compiling for the same platform that was requested, 
		//lookup what platform we should compile for based on the current material platform
		if (PlatformIndex != RequestedMaterialPlatform)
		{
			CurrentShaderPlatform = OtherPlatformsToCompile[PlatformIndex];
		}

		//don't compile fallback materials for sm3 unless we're running the editor (where it will be needed for the preview material)
		if (PlatformIndex == MSP_SM3 && bIsFallbackMaterial && !GIsEditor)
		{
			continue;
		}

		//mark the package dirty if the material resource has never been compiled
		if (GIsEditor && !MaterialResources[PlatformIndex]->GetId().IsValid())
		{
			MarkPackageDirty();
		}

		//if the current material platform matches the one that we're running under, compile the material resource.
		if (bForceAllPlatforms || PlatformIndex == RequestedMaterialPlatform)
		{
			UBOOL bSuccess = FALSE;
			//only try to generate a fallback if we are running sm2 and the current material is not a user-specified fallback material
			if (bFlushExistingShaderMaps)
			{
				bSuccess = MaterialResources[PlatformIndex]->CacheShaders(CurrentShaderPlatform, bIsSM2Platform && !bIsFallbackMaterial);
			}
			else
			{
				bSuccess = MaterialResources[PlatformIndex]->InitShaderMap(CurrentShaderPlatform, bIsSM2Platform && !bIsFallbackMaterial);
			}

			if (!bSuccess && FMaterial::ShouldWarnOnBrokenMaterial(CurrentShaderPlatform))
			{
				warnf(NAME_Warning, TEXT("Failed to compile Material %s for platform %s, Default Material will be used in game."), 
					*GetPathName(), 
					ShaderPlatformToText(CurrentShaderPlatform));
			}
		}
	}
}

/**
 * Flushes existing resource shader maps and resets the material resource's Ids.
 */
void UMaterial::FlushResourceShaderMaps()
{
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		if(MaterialResources[PlatformIndex])
		{
			MaterialResources[PlatformIndex]->FlushShaderMap();
			MaterialResources[PlatformIndex]->SetId(FGuid(0,0,0,0));
		}
	}
}

/**
 * Gets the material resource based on the input platform
 * @return - the appropriate FMaterialResource if one exists, otherwise NULL
 */
FMaterialResource * UMaterial::GetMaterialResource(EMaterialShaderPlatform Platform)
{
	//if a fallback material is specified and the sm2 platform is requested, pass it on to the fallback
	if (Platform == MSP_SM2 && FallbackMaterial)
	{
		return FallbackMaterial->GetMaterialResource(Platform);
	}

	return MaterialResources[Platform];
}

/**
 * Called before serialization on save to propagate referenced textures. This is not done
 * during content cooking as the material expressions used to retrieve this information will
 * already have been dissociated via RemoveExpressions
 */
void UMaterial::PreSave()
{
	Super::PreSave();

	//make sure all material resources for all platforms have been initialized (and compiled if needed)
	//so that the material resources will be complete for saving.  Don't do this during script compile,
	//since only dummy materials are saved, or when cooking, since compiling material shaders is handled explicitly by the commandlet.
	if (!GIsUCCMake && !GIsCooking)
	{
		CacheResourceShaders(GRHIShaderPlatform, FALSE, TRUE);
	}
}

void UMaterial::AddReferencedObjects(TArray<UObject*>& ObjectArray)
{
	Super::AddReferencedObjects(ObjectArray);

	if(MaterialResources[MSP_SM3])
	{
		MaterialResources[MSP_SM3]->AddReferencedObjects(ObjectArray);
	}

	if(MaterialResources[MSP_SM2])
	{
		MaterialResources[MSP_SM2]->AddReferencedObjects(ObjectArray);
	}
}

void UMaterial::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if(!MaterialResources[MSP_SM3])
	{
		if(!IsTemplate())
		{
			// Construct the material resource.
			MaterialResources[MSP_SM3] = AllocateResource();
		}
	}

	if(MaterialResources[MSP_SM3])
	{
		// Serialize the material resource.
		MaterialResources[MSP_SM3]->Serialize(Ar);
	}

	if (!MaterialResources[MSP_SM2] && !IsTemplate())
	{
		MaterialResources[MSP_SM2] = AllocateResource();
	}

	if (MaterialResources[MSP_SM2])
	{
		MaterialResources[MSP_SM2]->Serialize(Ar);
	}
	
	// This fixup should never be needed on consoles, since it will have been done during cooking.
	// Most of the expressions will be removed during cooking as well.
#if !CONSOLE
	if (Ar.IsLoading() && Ar.Ver() < VER_FIXED_SCENECOLOR_USAGE)
	{
		// Fixup old content whose bUsesSceneColor was not set correctly.
		// Mark the material as using scene color based on the presence of material expressions that use it.
		// Note that this is an overly conservative test, just because the expression exists in the material doesn't mean it was actually used in the compiled material.  
		// It could have been disconnected from the graph or culled by a static switch parameter.
		UBOOL bUsesSceneColor = FALSE;
		for( INT ExpressionIdx=0; ExpressionIdx < Expressions.Num(); ExpressionIdx++ )
		{
			UMaterialExpression * Expr = Expressions(ExpressionIdx);
			UMaterialExpressionDepthBiasedBlend* DepthBiasedBlendExpr = Cast<UMaterialExpressionDepthBiasedBlend>(Expr);
			UMaterialExpressionDepthBiasBlend* DepthBiasBlendExpr = Cast<UMaterialExpressionDepthBiasBlend>(Expr);
			UMaterialExpressionSceneTexture* SceneTextureExpr = Cast<UMaterialExpressionSceneTexture>(Expr);
			UMaterialExpressionDestColor* DestColorExpr = Cast<UMaterialExpressionDestColor>(Expr);
			if( DepthBiasedBlendExpr || DepthBiasBlendExpr || SceneTextureExpr || DestColorExpr )
			{
				bUsesSceneColor = TRUE;
				break;
			}
		}

		for (INT i = 0; i < MSP_MAX; i++)
		{
			if (MaterialResources[i])
			{
				MaterialResources[i]->SetUsesSceneColor(bUsesSceneColor);
			}
		}
	}
#endif
}

void UMaterial::PostDuplicate()
{
	// Reset each FMaterial's Id on duplication since it needs to be unique for each material.
	// This will be regenerated when it gets compiled.
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		if (MaterialResources[PlatformIndex])
		{
			MaterialResources[PlatformIndex]->SetId(FGuid(0,0,0,0));
		}
	}
}

void UMaterial::PostLoad()
{
	Super::PostLoad();

	if (GCookingTarget & UE3::PLATFORM_Windows)
	{
		//cache shaders for all PC platforms if we are cooking for PC
		CacheResourceShaders(SP_PCD3D_SM2, FALSE, FALSE);
		CacheResourceShaders(SP_PCD3D_SM3, FALSE, FALSE);
		CacheResourceShaders(SP_PCD3D_SM4, FALSE, FALSE);
	}
	else
	{
		//make sure the material's resource shaders are cached
		CacheResourceShaders(GRHIShaderPlatform, FALSE, FALSE);
	}

	if (FallbackMaterial)
	{
		MaterialResources[MSP_SM2]->ClearDroppedFallbackComponents();
	}

	if( !IsTemplate() )
	{
		// Ensure that the ReferencedTextures array is up to date.
		ReferencedTextures.Empty();
		GetTextures(ReferencedTextures);
	}
}

void UMaterial::PreEditChange(UProperty* PropertyThatChanged)
{
	Super::PreEditChange(PropertyThatChanged);

	// Flush all pending rendering commands.
	FlushRenderingCommands();
}

void UMaterial::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	if( PropertyThatChanged )
	{
		if(PropertyThatChanged->GetName()==TEXT("bIsFallbackMaterial"))
		{
			//don't allow setting bIsFallbackMaterial to true if a fallback material has been specified
			if (FallbackMaterial)
			{
				bIsFallbackMaterial = FALSE;
				return;
			}
		}
		else if(PropertyThatChanged->GetName()==TEXT("FallbackMaterial"))
		{
			//don't allow setting a fallback material if bIsFallbackMaterial has already been specified on this material
			if (bIsFallbackMaterial)
			{
				FallbackMaterial = NULL;
				return;
			}

			//don't allow setting a fallback material that hasn't been marked as a fallback material
			if (FallbackMaterial && !FallbackMaterial->bIsFallbackMaterial)
			{
				const FString ErrorMsg = FString::Printf(LocalizeSecure(LocalizeUnrealEd("Error_MaterialEditorSelectedMaterialNotFallback"), *FallbackMaterial->GetName()));
				appMsgf(AMT_OK, *ErrorMsg);
				FallbackMaterial = NULL;
				return;
			}
		}
		else if(PropertyThatChanged->GetName()==TEXT("bUsedWithFogVolumes") && bUsedWithFogVolumes)
		{
			//check that an emissive input has been supplied, otherwise nothing will be rendered
			if ((!EmissiveColor.UseConstant && EmissiveColor.Expression == NULL))
			{
				const FString ErrorMsg = FString::Printf(*LocalizeUnrealEd("Error_MaterialEditorFogVolumeMaterialNotSetup"));
				appMsgf(AMT_OK, *ErrorMsg);
				bUsedWithFogVolumes = FALSE;
				return;
			}

			//set states that are needed by fog volumes
			BlendMode = BLEND_Additive;
			LightingModel = MLM_Unlit;
		}
	}

	// check for distortion in material 
	bUsesDistortion = FALSE;
	// can only have distortion with translucent blend modes
	if(IsTranslucentBlendMode((EBlendMode)BlendMode))
	{
		// check for a distortion value
		if( Distortion.Expression ||
			(Distortion.UseConstant && !Distortion.Constant.IsNearlyZero()) )
		{
			bUsesDistortion = TRUE;
		}
	}

	// Check if the material is masked and uses a custom opacity (that's not 1.0f).
	bIsMasked = EBlendMode(BlendMode) == BLEND_Masked && (OpacityMask.Expression || (OpacityMask.UseConstant && OpacityMask.Constant<0.999f));

	UBOOL bRequiresCompilation = TRUE;
	if( PropertyThatChanged ) 
	{
		// Don't recompile the material if we only changed the PhysMaterial property.
		if( PropertyThatChanged->GetName() == TEXT("PhysMaterial"))
		{
			bRequiresCompilation = FALSE;
		}
		else if (PropertyThatChanged->GetName() == TEXT("FallbackMaterial"))
		{
			bRequiresCompilation = FALSE;
			if (FallbackMaterial)
			{
				MaterialResources[MSP_SM2]->ClearDroppedFallbackComponents();
			}
		}
	}

	if (bRequiresCompilation)
	{
		FlushResourceShaderMaps();
		CacheResourceShaders(GRHIShaderPlatform, TRUE, bIsFallbackMaterial);
		// Ensure that any components with static elements using this material are reattached so changes
		// are propagated to them. The preview material is only applied to the preview mesh component,
		// and that reattach is handled by the material editor.
		if( !bIsPreviewMaterial )
		{
			FGlobalComponentReattachContext RecreateComponents;
		}
	}
} 

/**
 * Adds an expression node that represents a parameter to the list of material parameters.
 *
 * @param	Expression	Pointer to the node that is going to be inserted if it's a parameter type.
 */
UBOOL UMaterial::AddExpressionParameter(UMaterialExpression* Expression)
{
	if(!Expression)
	{
		return FALSE;
	}

	UBOOL bRet = FALSE;

	if(Expression->IsA(UMaterialExpressionParameter::StaticClass()))
	{
		UMaterialExpressionParameter *Param = (UMaterialExpressionParameter*)Expression;

		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(Param->ParameterName);

		if(!ExpressionList)
		{
			ExpressionList = &EditorParameters.Set(Param->ParameterName, TArray<UMaterialExpression*>());
		}

		ExpressionList->AddItem(Param);
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
	{
		UMaterialExpressionTextureSampleParameter *Param = (UMaterialExpressionTextureSampleParameter*)Expression;

		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(Param->ParameterName);

		if(!ExpressionList)
		{
			ExpressionList = &EditorParameters.Set(Param->ParameterName, TArray<UMaterialExpression*>());
		}

		ExpressionList->AddItem(Param);
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionFontSampleParameter::StaticClass()))
	{
		UMaterialExpressionFontSampleParameter *Param = (UMaterialExpressionFontSampleParameter*)Expression;

		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(Param->ParameterName);

		if(!ExpressionList)
		{
			ExpressionList = &EditorParameters.Set(Param->ParameterName, TArray<UMaterialExpression*>());
		}

		ExpressionList->AddItem(Param);
		bRet = TRUE;
	}

	return bRet;
}

/**
 * Removes an expression node that represents a parameter from the list of material parameters.
 *
 * @param	Expression	Pointer to the node that is going to be removed if it's a parameter type.
 */
UBOOL UMaterial::RemoveExpressionParameter(UMaterialExpression* Expression)
{
	FName ParmName;

	if(GetExpressionParameterName(Expression, ParmName))
	{
		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(ParmName);

		if(ExpressionList)
		{
			return ExpressionList->RemoveItem(Expression) > 0;
		}
	}

	return FALSE;
}

/**
 * Returns TRUE if the provided expression node is a parameter.
 *
 * @param	Expression	The expression node to inspect.
 */
UBOOL UMaterial::IsParameter(UMaterialExpression* Expression)
{
	UBOOL bRet = FALSE;

	if(Expression->IsA(UMaterialExpressionParameter::StaticClass()))
	{
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
	{
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionFontSampleParameter::StaticClass()))
	{
		bRet = TRUE;
	}

	return bRet;
}

/**
 * Returns TRUE if the provided expression node is a dynamic parameter.
 *
 * @param	Expression	The expression node to inspect.
 */
UBOOL UMaterial::IsDynamicParameter(UMaterialExpression* Expression)
{
	if (Expression->IsA(UMaterialExpressionDynamicParameter::StaticClass()))
	{
		return TRUE;
	}

	return FALSE;
}


/**
 * Iterates through all of the expression nodes in the material and finds any parameters to put in EditorParameters.
 */
void UMaterial::BuildEditorParameterList()
{
	EmptyEditorParameters();

	for(INT MaterialExpressionIndex = 0 ; MaterialExpressionIndex < Expressions.Num() ; ++MaterialExpressionIndex)
	{
		AddExpressionParameter(Expressions( MaterialExpressionIndex ));
	}
}

/**
 * Returns TRUE if the provided expression parameter has duplicates.
 *
 * @param	Expression	The expression parameter to check for duplicates.
 */
UBOOL UMaterial::HasDuplicateParameters(UMaterialExpression* Expression)
{
	FName ExpressionName;

	if(GetExpressionParameterName(Expression, ExpressionName))
	{
		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(ExpressionName);

		if(ExpressionList)
		{
			for(INT ParmIndex = 0; ParmIndex < ExpressionList->Num(); ++ParmIndex)
			{
				UMaterialExpression *CurNode = (*ExpressionList)(ParmIndex);
				if(CurNode != Expression && CurNode->GetClass() == Expression->GetClass())
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/**
 * Returns TRUE if the provided expression dynamic parameter has duplicates.
 *
 * @param	Expression	The expression dynamic parameter to check for duplicates.
 */
UBOOL UMaterial::HasDuplicateDynamicParameters(UMaterialExpression* Expression)
{
	UMaterialExpressionDynamicParameter* DynParam = Cast<UMaterialExpressionDynamicParameter>(Expression);
	if (DynParam)
	{
		for (INT ExpIndex = 0; ExpIndex < Expressions.Num(); ExpIndex++)
		{
			UMaterialExpressionDynamicParameter* CheckDynParam = Cast<UMaterialExpressionDynamicParameter>(Expressions(ExpIndex));
			if (CheckDynParam != Expression)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/**
 * Iterates through all of the expression nodes and fixes up changed names on 
 * matching dynamic parameters when a name change occurs.
 *
 * @param	Expression	The expression dynamic parameter.
 */
void UMaterial::UpdateExpressionDynamicParameterNames(UMaterialExpression* Expression)
{
	UMaterialExpressionDynamicParameter* DynParam = Cast<UMaterialExpressionDynamicParameter>(Expression);
	if (DynParam)
	{
		for (INT ExpIndex = 0; ExpIndex < Expressions.Num(); ExpIndex++)
		{
			UMaterialExpressionDynamicParameter* CheckParam = Cast<UMaterialExpressionDynamicParameter>(Expressions(ExpIndex));
			if (CheckParam && (CheckParam != DynParam))
			{
				for (INT NameIndex = 0; NameIndex < 4; NameIndex++)
				{
					CheckParam->ParamNames(NameIndex) = DynParam->ParamNames(NameIndex);
				}
			}
		}
	}
}

/**
 * A parameter with duplicates has to update its peers so that they all have the same value. If this step isn't performed then
 * the expression nodes will not accurately display the final compiled material.
 *
 * @param	Parameter	Pointer to the expression node whose state needs to be propagated.
 */
void UMaterial::PropagateExpressionParameterChanges(UMaterialExpression* Parameter)
{
	FName ParmName;
	UBOOL bRet = GetExpressionParameterName(Parameter, ParmName);

	if(bRet)
	{
		TArray<UMaterialExpression*> *ExpressionList = EditorParameters.Find(ParmName);

		if(ExpressionList && ExpressionList->Num() > 1)
		{
			for(INT Index = 0; Index < ExpressionList->Num(); ++Index)
			{
				CopyExpressionParameters(Parameter, (*ExpressionList)(Index));
			}
		}
		else if(!ExpressionList)
		{
			bRet = FALSE;
		}
	}
}

/**
 * This function removes the expression from the editor parameters list (if it exists) and then re-adds it.
 *
 * @param	Expression	The expression node that represents a parameter that needs updating.
 */
void UMaterial::UpdateExpressionParameterName(UMaterialExpression* Expression)
{
	FName ExpressionName;

	for(TMap<FName, TArray<UMaterialExpression*> >::TIterator Iter(EditorParameters); Iter; ++Iter)
	{
		if(Iter.Value().RemoveItem(Expression) > 0)
		{
			if(Iter.Value().Num() == 0)
			{
				EditorParameters.Remove(Iter.Key());
			}

			AddExpressionParameter(Expression);
			break;
		}
	}
}

/**
 * Gets the name of a parameter.
 *
 * @param	Expression	The expression to retrieve the name from.
 * @param	OutName		The variable that will hold the parameter name.
 * @return	TRUE if the expression is a parameter with a name.
 */
UBOOL UMaterial::GetExpressionParameterName(UMaterialExpression* Expression, FName& OutName)
{
	UBOOL bRet = FALSE;

	if(Expression->IsA(UMaterialExpressionParameter::StaticClass()))
	{
		OutName = ((UMaterialExpressionParameter*)Expression)->ParameterName;
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
	{
		OutName = ((UMaterialExpressionTextureSampleParameter*)Expression)->ParameterName;
		bRet = TRUE;
	}
	else if(Expression->IsA(UMaterialExpressionFontSampleParameter::StaticClass()))
	{
		OutName = ((UMaterialExpressionFontSampleParameter*)Expression)->ParameterName;
		bRet = TRUE;
	}

	return bRet;
}

/**
 * Copies the values of an expression parameter to another expression parameter of the same class.
 *
 * @param	Source			The source parameter.
 * @param	Destination		The destination parameter that will receive Source's values.
 */
UBOOL UMaterial::CopyExpressionParameters(UMaterialExpression* Source, UMaterialExpression* Destination)
{
	if(!Source || !Destination || Source == Destination || Source->GetClass() != Destination->GetClass())
	{
		return FALSE;
	}

	UBOOL bRet = TRUE;

	if(Source->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
	{
		UMaterialExpressionTextureSampleParameter *SourceTex = (UMaterialExpressionTextureSampleParameter*)Source;
		UMaterialExpressionTextureSampleParameter *DestTex = (UMaterialExpressionTextureSampleParameter*)Destination;

		DestTex->Modify();
		DestTex->Texture = SourceTex->Texture;
	}
	else if(Source->IsA(UMaterialExpressionVectorParameter::StaticClass()))
	{
		UMaterialExpressionVectorParameter *SourceVec = (UMaterialExpressionVectorParameter*)Source;
		UMaterialExpressionVectorParameter *DestVec = (UMaterialExpressionVectorParameter*)Destination;

		DestVec->Modify();
		DestVec->DefaultValue = SourceVec->DefaultValue;
	}
	else if(Source->IsA(UMaterialExpressionStaticSwitchParameter::StaticClass()))
	{
		UMaterialExpressionStaticSwitchParameter *SourceVec = (UMaterialExpressionStaticSwitchParameter*)Source;
		UMaterialExpressionStaticSwitchParameter *DestVec = (UMaterialExpressionStaticSwitchParameter*)Destination;

		DestVec->Modify();
		DestVec->DefaultValue = SourceVec->DefaultValue;
	}
	else if(Source->IsA(UMaterialExpressionStaticComponentMaskParameter::StaticClass()))
	{
		UMaterialExpressionStaticComponentMaskParameter *SourceVec = (UMaterialExpressionStaticComponentMaskParameter*)Source;
		UMaterialExpressionStaticComponentMaskParameter *DestVec = (UMaterialExpressionStaticComponentMaskParameter*)Destination;

		DestVec->Modify();
		DestVec->DefaultR = SourceVec->DefaultR;
		DestVec->DefaultG = SourceVec->DefaultG;
		DestVec->DefaultB = SourceVec->DefaultB;
		DestVec->DefaultA = SourceVec->DefaultA;
	}
	else if(Source->IsA(UMaterialExpressionScalarParameter::StaticClass()))
	{
		UMaterialExpressionScalarParameter *SourceVec = (UMaterialExpressionScalarParameter*)Source;
		UMaterialExpressionScalarParameter *DestVec = (UMaterialExpressionScalarParameter*)Destination;

		DestVec->Modify();
		DestVec->DefaultValue = SourceVec->DefaultValue;
	}
	else if(Source->IsA(UMaterialExpressionFontSampleParameter::StaticClass()))
	{
		UMaterialExpressionFontSampleParameter *SourceFont = (UMaterialExpressionFontSampleParameter*)Source;
		UMaterialExpressionFontSampleParameter *DestFont = (UMaterialExpressionFontSampleParameter*)Destination;

		DestFont->Modify();
		DestFont->Font = SourceFont->Font;
		DestFont->FontTexturePage = SourceFont->FontTexturePage;
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;
}

void UMaterial::BeginDestroy()
{
	Super::BeginDestroy();

	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		if(MaterialResources[PlatformIndex])
		{
			MaterialResources[PlatformIndex]->ReleaseFence.BeginFence();
		}
	}
}

UBOOL UMaterial::IsReadyForFinishDestroy()
{
	UBOOL bReady = Super::IsReadyForFinishDestroy();

	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		bReady = bReady && (!MaterialResources[PlatformIndex] || !MaterialResources[PlatformIndex]->ReleaseFence.GetNumPendingFences());
	}

	return bReady;
}

void UMaterial::FinishDestroy()
{
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		if (MaterialResources[PlatformIndex])
		{
			delete MaterialResources[PlatformIndex];
			MaterialResources[PlatformIndex] = NULL;
		}
	}

	delete DefaultMaterialInstances[FALSE];
	delete DefaultMaterialInstances[TRUE];
	Super::FinishDestroy();
}

/**
 * @return		Sum of the size of textures referenced by this material.
 */
INT UMaterial::GetResourceSize()
{
	INT ResourceSize = 0;
	TArray<UTexture*> TheReferencedTextures;
	for ( INT ExpressionIndex= 0 ; ExpressionIndex < Expressions.Num() ; ++ExpressionIndex )
	{
		UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>( Expressions(ExpressionIndex) );
		if ( TextureSample && TextureSample->Texture )
		{
			UTexture* Texture						= TextureSample->Texture;
			const UBOOL bTextureAlreadyConsidered	= TheReferencedTextures.ContainsItem( Texture );
			if ( !bTextureAlreadyConsidered )
			{
				TheReferencedTextures.AddItem( Texture );
				ResourceSize += Texture->GetResourceSize();
			}
		}
	}
	return ResourceSize;
}

/** === USurface interface === */
/** 
 * Method for retrieving the width of this surface.
 *
 * This implementation returns the maximum width of all textures applied to this material - not exactly accurate, but best approximation.
 *
 * @return	the width of this surface, in pixels.
 */
FLOAT UMaterial::GetSurfaceWidth() const
{
	FLOAT MaxTextureWidth = 0.f;

	TArray<UTexture*> Textures;
	const_cast<UMaterial*>(this)->GetTextures(Textures, TRUE);
	for ( INT TextureIndex = 0; TextureIndex < Textures.Num(); TextureIndex++ )
	{
		UTexture* AppliedTexture = Textures(TextureIndex);
		if ( AppliedTexture != NULL )
		{
			MaxTextureWidth = Max(MaxTextureWidth, AppliedTexture->GetSurfaceWidth());
		}
	}

	if ( Abs(MaxTextureWidth) < DELTA )
	{
		MaxTextureWidth = GetWidth();
	}

	return MaxTextureWidth;
}
/** 
 * Method for retrieving the height of this surface.
 *
 * This implementation returns the maximum height of all textures applied to this material - not exactly accurate, but best approximation.
 *
 * @return	the height of this surface, in pixels.
 */
FLOAT UMaterial::GetSurfaceHeight() const
{
	FLOAT MaxTextureHeight = 0.f;

	TArray<UTexture*> Textures;
	const_cast<UMaterial*>(this)->GetTextures(Textures, TRUE);

	for ( INT TextureIndex = 0; TextureIndex < Textures.Num(); TextureIndex++ )
	{
		UTexture* AppliedTexture = Textures(TextureIndex);
		if ( AppliedTexture != NULL )
		{
			MaxTextureHeight = Max(MaxTextureHeight, AppliedTexture->GetSurfaceHeight());
		}
	}

	if ( Abs(MaxTextureHeight) < DELTA )
	{
		MaxTextureHeight = GetHeight();
	}

	return MaxTextureHeight;
}


/**
 * Used by various commandlets to purge Editor only data from the object.
 * 
 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
 */
void UMaterial::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform); 
}

/**
 * Null any material expression references for this material
 */
void UMaterial::RemoveExpressions()
{
	for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
	{
		if (MaterialResources[PlatformIndex])
		{
			MaterialResources[PlatformIndex]->RemoveExpressions();
		}
	}

	// Remove all non-parameter expressions from the material's expressions array.
	for(INT ExpressionIndex = 0;ExpressionIndex < Expressions.Num();ExpressionIndex++)
	{
		UMaterialExpression* Expression = Expressions(ExpressionIndex);
		
		// Skip the expression if it is a parameter expression
		if(Expression)
		{
			if(Expression->IsA(UMaterialExpressionScalarParameter::StaticClass()))
			{
				continue;
			}
			if(Expression->IsA(UMaterialExpressionVectorParameter::StaticClass()))
			{
				continue;
			}
			if(Expression->IsA(UMaterialExpressionTextureSampleParameter::StaticClass()))
			{
				continue;
			}
		}

		// Otherwise, remove the expression.
		Expressions.Remove(ExpressionIndex--,1);
	}
	Expressions.Shrink();

	DiffuseColor.Expression = NULL;
	DiffusePower.Expression = NULL;
	SpecularColor.Expression = NULL;
	SpecularPower.Expression = NULL;
	Normal.Expression = NULL;
	EmissiveColor.Expression = NULL;
	Opacity.Expression = NULL;
	OpacityMask.Expression = NULL;
	Distortion.Expression = NULL;
	CustomLighting.Expression = NULL;
	TwoSidedLightingMask.Expression = NULL;
	TwoSidedLightingColor.Expression = NULL;
}

/**
 * Goes through every material, flushes the specified types and re-initializes the material's shader maps.
 */
void UMaterial::UpdateMaterialShaders(TArray<FShaderType*>& ShaderTypesToFlush, TArray<FVertexFactoryType*>& VFTypesToFlush)
{
	FlushRenderingCommands();
	for( TObjectIterator<UMaterial> It; It; ++It )
	{
		UMaterial* Material = *It;
		if( Material )
		{
			FMaterialResource* MaterialResource = Material->GetMaterialResource();
			if (MaterialResource && MaterialResource->GetShaderMap())
			{
				FMaterialShaderMap* ShaderMap = MaterialResource->GetShaderMap();

				for(INT ShaderTypeIndex = 0; ShaderTypeIndex < ShaderTypesToFlush.Num(); ShaderTypeIndex++)
				{
					ShaderMap->FlushShadersByShaderType(ShaderTypesToFlush(ShaderTypeIndex));
				}
				for(INT VFTypeIndex = 0; VFTypeIndex < VFTypesToFlush.Num(); VFTypeIndex++)
				{
					ShaderMap->FlushShadersByVertexFactoryType(VFTypesToFlush(VFTypeIndex));
				}

				Material->CacheResourceShaders(GRHIShaderPlatform, FALSE, FALSE);
			}
		}
	}
	FGlobalComponentReattachContext RecreateComponents;
}

