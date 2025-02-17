/*=============================================================================
	MaterialInstanceConstant.cpp: MaterialInstanceConstant implementation.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "MaterialInstanceConstant.h"

IMPLEMENT_CLASS(UMaterialInstanceConstant);

/** A mapping for UMaterialInstanceConstant's scalar parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MICScalarParameterMapping,
	FMaterialInstanceConstantResource,
	FLOAT,
	FScalarParameterValue,
	ScalarParameterValues,
	ScalarParameterMap,
	{ Value = Parameter.ParameterValue; }
	);

/** A mapping for UMaterialInstanceConstant's vector parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MICVectorParameterMapping,
	FMaterialInstanceConstantResource,
	FLinearColor,
	FVectorParameterValue,
	VectorParameterValues,
	VectorParameterMap,
	{ Value = Parameter.ParameterValue; }
	);

/** A mapping from UMaterialInstanceConstant's texture parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MICTextureParameterMapping,
	FMaterialInstanceConstantResource,
	const UTexture*,
	FTextureParameterValue,
	TextureParameterValues,
	TextureParameterMap,
	{ Value = Parameter.ParameterValue; }
	);

/** A mapping from UMaterialInstanceConstant's font parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MICFontParameterMapping,
	FMaterialInstanceConstantResource,
	const UTexture*,
	FFontParameterValue,
	FontParameterValues,
	TextureParameterMap,
	{
		Value = NULL;
		// add font texture to the texture parameter map
		if( Parameter.FontValue && Parameter.FontValue->Textures.IsValidIndex(Parameter.FontPage) )
		{
			// get the texture for the font page
			Value = Parameter.FontValue->Textures(Parameter.FontPage);
		}
	});

/** Initializes a MIC's rendering thread mirror of the game thread parameter array. */
template<typename MappingType>
void InitMICParameters(const UMaterialInstanceConstant* Instance)
{
	if(!GIsUCCMake && !Instance->HasAnyFlags(RF_ClassDefaultObject))
	{
		const TArray<typename MappingType::ParameterType>& Parameters = MappingType::GetParameterArray(Instance);
		for(INT ParameterIndex = 0;ParameterIndex < Parameters.Num();ParameterIndex++)
		{
			MappingType::GameThread_UpdateParameter(
				Instance,
				Parameters(ParameterIndex)
				);
		}
	}
}

UBOOL FMaterialInstanceConstantResource::GetScalarValue(
	const FName& ParameterName, 
	FLOAT* OutValue,
	const FMaterialRenderContext& Context
	) const
{
	checkSlow(IsInRenderingThread());
	const FLOAT* Value = ScalarParameterMap.Find(ParameterName);
	if(Value)
	{
		*OutValue = *Value;
		return TRUE;
	}
	else if(Parent)
	{
		return Parent->GetRenderProxy(bSelected)->GetScalarValue(ParameterName, OutValue, Context);
	}
	else
	{
		return FALSE;
	}
}

UBOOL FMaterialInstanceConstantResource::GetVectorValue(
	const FName& ParameterName, 
	FLinearColor* OutValue,
	const FMaterialRenderContext& Context
	) const
{
	checkSlow(IsInRenderingThread());
	const FLinearColor* Value = VectorParameterMap.Find(ParameterName);
	if(Value)
	{
		*OutValue = *Value;
		return TRUE;
	}
	else if(Parent)
	{
		return Parent->GetRenderProxy(bSelected)->GetVectorValue(ParameterName, OutValue, Context);
	}
	else
	{
		return FALSE;
	}
}

UBOOL FMaterialInstanceConstantResource::GetTextureValue(
	const FName& ParameterName,
	const FTexture** OutValue,
	const FMaterialRenderContext& Context
	) const
{
	checkSlow(IsInRenderingThread());
	const UTexture* Value = TextureParameterMap.FindRef(ParameterName);
	if(Value)
	{
		*OutValue = Value->Resource;
		return TRUE;
	}
	else if(Parent)
	{
		return Parent->GetRenderProxy(bSelected)->GetTextureValue(ParameterName,OutValue,Context);
	}
	else
	{
		return FALSE;
	}
}

UMaterialInstanceConstant::UMaterialInstanceConstant()
{
	// GIsUCCMake is not set when the class is initialized
	if(!GIsUCCMake && !HasAnyFlags(RF_ClassDefaultObject))
	{
		Resources[0] = new FMaterialInstanceConstantResource(this,FALSE);
		if(GIsEditor)
		{
			Resources[1] = new FMaterialInstanceConstantResource(this,TRUE);
		}
		InitResources();
	}
}

void UMaterialInstanceConstant::InitResources()
{
	Super::InitResources();

	InitMICParameters<MICScalarParameterMapping>(this);
	InitMICParameters<MICVectorParameterMapping>(this);
	InitMICParameters<MICTextureParameterMapping>(this);
	InitMICParameters<MICFontParameterMapping>(this);
}

void UMaterialInstanceConstant::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::SetVectorParameterValue"));
#endif

	FVectorParameterValue* ParameterValue = MICVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(VectorParameterValues) FVectorParameterValue;
		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MICVectorParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

void UMaterialInstanceConstant::SetScalarParameterValue(FName ParameterName, FLOAT Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::SetScalarParameterValue"));
#endif

	FScalarParameterValue* ParameterValue = MICScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(ScalarParameterValues) FScalarParameterValue;
		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MICScalarParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

//
//  UMaterialInstanceConstant::SetTextureParameterValue
//
void UMaterialInstanceConstant::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::SetTextureParameterValue"));
#endif

	FTextureParameterValue* ParameterValue = MICTextureParameterMapping::FindParameterByName(this,ParameterName);

	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(TextureParameterValues) FTextureParameterValue;
		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MICTextureParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

/**
* Sets the value of the given font parameter.  
*
* @param	ParameterName	The name of the font parameter
* @param	OutFontValue	New font value to set for this MIC
* @param	OutFontPage		New font page value to set for this MIC
*/
void UMaterialInstanceConstant::SetFontParameterValue(FName ParameterName,class UFont* FontValue,INT FontPage)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::SetFontParameterValue"));
#endif

	FFontParameterValue* ParameterValue = MICFontParameterMapping::FindParameterByName(this,ParameterName);
	if( !ParameterValue )
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(FontParameterValues) FFontParameterValue;
		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->FontValue = FontValue;
	ParameterValue->FontPage = FontPage;

	// Update the material instance data in the rendering thread.
	MICFontParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

/** Removes all parameter values */
void UMaterialInstanceConstant::ClearParameterValues()
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::ClearParameterValues"));
#endif

	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	TextureParameterValues.Empty();
	FontParameterValues.Empty();

	// Update the material instance data in the rendering thread.
	InitResources();
}

UBOOL UMaterialInstanceConstant::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	UBOOL bFoundAValue = FALSE;

	if(ReentrantFlag)
	{
		return FALSE;
	}

	FVectorParameterValue* ParameterValue = MICVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue)
	{
		OutValue = ParameterValue->ParameterValue;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetVectorParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	UBOOL bFoundAValue = FALSE;

	if(ReentrantFlag)
	{
		return FALSE;
	}

	FScalarParameterValue* ParameterValue = MICScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue)
	{
		OutValue = ParameterValue->ParameterValue;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetScalarParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FTextureParameterValue* ParameterValue = MICTextureParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue && ParameterValue->ParameterValue)
	{
		OutValue = ParameterValue->ParameterValue;
		return TRUE;
	}
	else if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetTextureParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

UBOOL UMaterialInstanceConstant::GetFontParameterValue(FName ParameterName,class UFont*& OutFontValue, INT& OutFontPage)
{
	if( ReentrantFlag )
	{
		return FALSE;
	}

	FFontParameterValue* ParameterValue = MICFontParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue && ParameterValue->FontValue)
	{
		OutFontValue = ParameterValue->FontValue;
		OutFontPage = ParameterValue->FontPage;
		return TRUE;
	}
		//@todo sz
		// 		// try the parent if values were invalid
		// 		else if( Parent )
		// 		{
		// 			FMICReentranceGuard	Guard(this);
		// 			Result = Parent->GetFontParameterValue(ParameterName,OutFontValue,OutFontPage);
		// 		}
	else
	{
		return FALSE;
	}
}


void UMaterialInstanceConstant::SetParent(UMaterialInterface* NewParent)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MIC::SetParent"));
#endif

	Super::SetParent( NewParent );

	InitResources();
}

void UMaterialInstanceConstant::PostLoad()
{
	// Ensure that the instance's parent is PostLoaded before the instance.
	if(Parent)
	{
		Parent->ConditionalPostLoad();
	}

	// Add references to the expression object if we do not have one already, and fix up any names that were changed.
	UpdateParameterNames();

	// We have to make sure the resources are created for all used textures.
	for( INT ValueIndex=0; ValueIndex<TextureParameterValues.Num(); ValueIndex++ )
	{
		// Make sure the texture is postloaded so the resource isn't null.
		UTexture* Texture = TextureParameterValues(ValueIndex).ParameterValue;
		if( Texture )
		{
			Texture->ConditionalPostLoad();
		}
	}
	// do the same for font textures
	for( INT ValueIndex=0; ValueIndex < FontParameterValues.Num(); ValueIndex++ )
	{
		// Make sure the font is postloaded so the resource isn't null.
		UFont* Font = FontParameterValues(ValueIndex).FontValue;
		if( Font )
		{
			Font->ConditionalPostLoad();
		}
	}

	Super::PostLoad();

	InitResources();
}


void UMaterialInstanceConstant::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
	InitResources();

	UpdateStaticPermutation();
}


/**
* Refreshes parameter names using the stored reference to the expression object for the parameter.
*/
void UMaterialInstanceConstant::UpdateParameterNames()
{
	if(IsTemplate(RF_ClassDefaultObject)==FALSE)
	{
		// Get a pointer to the parent material.
		UMaterial* ParentMaterial = NULL;
		UMaterialInstance* ParentInst = this;
		while(ParentInst && ParentInst->Parent)
		{
			if(ParentInst->Parent->IsA(UMaterial::StaticClass()))
			{
				ParentMaterial = Cast<UMaterial>(ParentInst->Parent);
				break;
			}
			else
			{
				ParentInst = Cast<UMaterialInstance>(ParentInst->Parent);
			}
		}

		if(ParentMaterial)
		{
			UBOOL bMarkDirty = FALSE;

			// Scalar parameters
			bMarkDirty = UpdateParameterSet<FScalarParameterValue, UMaterialExpressionScalarParameter>(ScalarParameterValues, ParentMaterial) || bMarkDirty;

			// Vector parameters
			bMarkDirty = UpdateParameterSet<FVectorParameterValue, UMaterialExpressionVectorParameter>(VectorParameterValues, ParentMaterial) || bMarkDirty;

			// Texture parameters
			bMarkDirty = UpdateParameterSet<FTextureParameterValue, UMaterialExpressionTextureSampleParameter>(TextureParameterValues, ParentMaterial) || bMarkDirty;

			// Font parameters
			bMarkDirty = UpdateParameterSet<FFontParameterValue, UMaterialExpressionFontSampleParameter>(FontParameterValues, ParentMaterial) || bMarkDirty;

			for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
			{
				// Static switch parameters
				bMarkDirty = UpdateParameterSet<FStaticSwitchParameter, UMaterialExpressionStaticSwitchParameter>(StaticParameters[PlatformIndex]->StaticSwitchParameters, ParentMaterial) || bMarkDirty;

				// Static component mask parameters
				bMarkDirty = UpdateParameterSet<FStaticComponentMaskParameter, UMaterialExpressionStaticComponentMaskParameter>(StaticParameters[PlatformIndex]->StaticComponentMaskParameters, ParentMaterial) || bMarkDirty;

			}

			// Atleast 1 parameter changed, mark the package dirty.
			if(bMarkDirty)
			{
				MarkPackageDirty();
				InitResources();
			}
		}
	}
}
