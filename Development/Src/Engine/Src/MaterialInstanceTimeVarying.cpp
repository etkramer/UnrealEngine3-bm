/**
 *	 
 *	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "MaterialInstanceTimeVarying.h"

IMPLEMENT_CLASS(UMaterialInstanceTimeVarying);

/** Translates the game thread MITV parameter values into the rendering thread's value struct. */
template<typename ParameterType,typename ValueType>
UBOOL GetValueFromMITVParameter(
	const UMaterialInstanceTimeVarying* Instance,
	const ParameterType& InParameter,
	ValueType& OutValue
	)
{
	if( ( ( InParameter.bAutoActivate == TRUE )
		|| ( Instance->bAutoActivateAll == TRUE )
		|| ( InParameter.StartTime >= 0.0f ) 
		|| ( (InParameter.StartTime == -1.0f) && (Instance->Duration > 0.0f) ) //if we have set a duration but the start time is not set
		)
		)
	{
		// this guy needs to have the correct values for the MITV properties
		OutValue.Value = InParameter.ParameterValue;
		OutValue.TheCurve = InParameter.ParameterValueCurve;

		OutValue.bLoop = InParameter.bLoop;
		OutValue.CycleTime = InParameter.CycleTime;
		OutValue.bNormalizeTime = InParameter.bNormalizeTime;

		OutValue.OffsetTime = InParameter.OffsetTime;
		OutValue.bOffsetFromEnd = InParameter.bOffsetFromEnd;

		// determine the StartTime depending on whether or not we are offsetting from the end and by what amount
		//OutValue.StartTime = Instance->ScalarParameterValues(ValueIndex).StartTime;

		FLOAT StartTime = -1.0f;
		// if we have had a StartTime set then we want to use that
		// @NOTE: this might not be exactly what we want tho
		if( InParameter.StartTime > 0.0f )
		{
			StartTime = InParameter.StartTime;
		}
		else
		{
			StartTime = (GWorld != NULL ? GWorld->GetTimeSeconds() : 0.0f);
		}

		if( OutValue.bOffsetFromEnd == FALSE )
		{
			OutValue.StartTime = StartTime + OutValue.OffsetTime;
		}
		else
		{
			// this is where the Duration has been set
			const FLOAT StartTimeValue = (Instance->Duration - OutValue.OffsetTime);
			OutValue.StartTime = StartTime + StartTimeValue;
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** A mapping from UMaterialInstanceTimeVarying's scalar-over-time parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MITVScalarParameterMapping,
	FMaterialInstanceTimeVaryingResource,
	FTimeVaryingScalarDataType,
	FScalarParameterValueOverTime,
	ScalarParameterValues,
	ScalarOverTimeParameterMap,
	{ bHasValue = GetValueFromMITVParameter(Instance,Parameter,Value); }
	);

/** A mapping from UMaterialInstanceTimeVarying's vector-over-time parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MITVVectorParameterMapping,
	FMaterialInstanceTimeVaryingResource,
	FTimeVaryingVectorDataType,
	FVectorParameterValueOverTime,
	VectorParameterValues,
	VectorOverTimeParameterMap,
	{ bHasValue = GetValueFromMITVParameter(Instance,Parameter,Value); }
	);

/** A mapping from UMaterialInstanceConstant's texture parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MITVTextureParameterMapping,
	FMaterialInstanceTimeVaryingResource,
	const UTexture*,
	FTextureParameterValueOverTime,
	TextureParameterValues,
	TextureParameterMap,
	{ Value = Parameter.ParameterValue; }
	);

/** A mapping from UMaterialInstanceConstant's font parameters. */
DEFINE_MATERIALINSTANCE_PARAMETERTYPE_MAPPING(
	MITVFontParameterMapping,
	FMaterialInstanceTimeVaryingResource,
	const UTexture*,
	FFontParameterValueOverTime,
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

/** Initializes a MITV's rendering thread mirror of the game thread parameter array. */
template<typename MappingType>
void InitMITVParameters(const UMaterialInstanceTimeVarying* DestInstance)
{
	if(!GIsUCCMake && !DestInstance->HasAnyFlags(RF_ClassDefaultObject))
	{
		TSet<FName> DefinedParameterNames;
		for(const UMaterialInstanceTimeVarying* CurrentInstance = DestInstance;
			CurrentInstance;
			CurrentInstance = Cast<UMaterialInstanceTimeVarying>(CurrentInstance->Parent))
		{
			const TArray<typename MappingType::ParameterType>& Parameters = MappingType::GetParameterArray(CurrentInstance);
			for(INT ParameterIndex = 0;ParameterIndex < Parameters.Num();ParameterIndex++)
			{
				const typename MappingType::ParameterType& Parameter = Parameters(ParameterIndex);
				if(!DefinedParameterNames.Find(Parameter.ParameterName))
				{
					DefinedParameterNames.Add(Parameter.ParameterName);
					MappingType::GameThread_UpdateParameter(
						DestInstance,
						Parameter
						);
				}
			}
		}
	}
}

UBOOL FMaterialInstanceTimeVaryingResource::GetScalarValue(const FName& ParameterName,FLOAT* OutValue, const FMaterialRenderContext& Context) const
{
	checkSlow(IsInRenderingThread());

	const FTimeVaryingScalarDataType* Value = ScalarOverTimeParameterMap.Find(ParameterName);

	//debugf( TEXT( "%s Looking for: %s  SizeOfMap: %d"), *Owner->GetName(), *ParameterName.ToString(), ScalarOverTimeParameterMap.Num() );

	if(Value)
	{
		if( Value->TheCurve.Points.Num() > 0 )
		{
			const FLOAT StartTime = Value->StartTime;
			FLOAT EvalTime = Context.CurrentTime - StartTime;

			if( Value->CycleTime > 0.0f )
			{
				if( Value->bLoop == TRUE )
				{
					EvalTime = appFmod( EvalTime, Value->CycleTime );
					// check for StartTime being in the future as in the Editor this can often be the case
					if( EvalTime < 0.0f )
					{
						EvalTime += Value->CycleTime;
					}
				}

				if( Value->bNormalizeTime == TRUE )
				{
					EvalTime /= Value->CycleTime;
				}
			}

			const FLOAT EvalValue = Value->TheCurve.Eval(EvalTime, 0.0f);

			//debugf( TEXT( "*Owner: %s   %s  CurrentTime %f StarTime %f, EvalTime %f EvalValue %f" ), *Owner->GetName(), *ParameterName.ToString(), Context.CurrentTime, StartTime, EvalTime, EvalValue );

			*OutValue = EvalValue;
		}
		else
		{
			*OutValue = Value->Value;
		}
		
		return TRUE;
	}
	else if(Parent)
	{
		//debugf( TEXT( "*calling parent: %s   %s  " ), *Owner->GetName(), *ParameterName.ToString(), Context.CurrentTime );
		return Parent->GetRenderProxy(bSelected)->GetScalarValue(ParameterName, OutValue, Context);
	}
	else
	{
		return FALSE;
	}
}

UBOOL FMaterialInstanceTimeVaryingResource::GetVectorValue(const FName& ParameterName,FLinearColor* OutValue, const FMaterialRenderContext& Context) const
{
	checkSlow(IsInRenderingThread());
	const FTimeVaryingVectorDataType* Value = VectorOverTimeParameterMap.Find(ParameterName);

	if(Value)
	{
		if( Value->TheCurve.Points.Num() > 0 )
		{
			const FLOAT StartTime = Value->StartTime;
			FLOAT EvalTime = Context.CurrentTime - StartTime;

			if( Value->CycleTime > 0.0f )
			{
				if( Value->bLoop == TRUE )
				{
					EvalTime = appFmod( EvalTime, Value->CycleTime );
					// check for StartTime being in the future as in the Editor this can often be the case
					if( EvalTime < 0.0f )
					{
						EvalTime += Value->CycleTime;
					}
				}

				if( Value->bNormalizeTime == TRUE )
				{
					EvalTime /= Value->CycleTime;
				}
			}

			FVector EvalValue = Value->TheCurve.Eval( EvalTime, FVector(0.0f,0.0f,0.0f) );

			OutValue->R = EvalValue.X;
			OutValue->G = EvalValue.Y;
			OutValue->B = EvalValue.Z;
			OutValue->A = 0.0f;
		}
		else
		{
			*OutValue = Value->Value;
		}
		
		return TRUE;
	}
	else if(Parent)
	{
		return Parent->GetRenderProxy(bSelected)->GetVectorValue(ParameterName,OutValue,Context);
	}
	else
	{
		return FALSE;
	}
}

UBOOL FMaterialInstanceTimeVaryingResource::GetTextureValue(const FName& ParameterName,const FTexture** OutValue,const FMaterialRenderContext& Context) const
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

UMaterialInstanceTimeVarying::UMaterialInstanceTimeVarying()
{
	// GIsUCCMake is not set when the class is initialized
	if(!GIsUCCMake && !HasAnyFlags(RF_ClassDefaultObject))
	{
		Resources[0] = new FMaterialInstanceTimeVaryingResource(this,FALSE);
		if(GIsEditor)
		{
			Resources[1] = new FMaterialInstanceTimeVaryingResource(this,TRUE);
		}
		InitResources();
	}
}

void UMaterialInstanceTimeVarying::InitResources()
{
	Super::InitResources();

	InitMITVParameters<MITVScalarParameterMapping>(this);
	InitMITVParameters<MITVVectorParameterMapping>(this);
	InitMITVParameters<MITVTextureParameterMapping>(this);
	InitMITVParameters<MITVFontParameterMapping>(this);
}

void UMaterialInstanceTimeVarying::PostLoad()
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


void UMaterialInstanceTimeVarying::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);
	InitResources();

	UpdateStaticPermutation();
}

/**
* Refreshes parameter names using the stored reference to the expression object for the parameter.
*/
void UMaterialInstanceTimeVarying::UpdateParameterNames()
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
			bMarkDirty = UpdateParameterSet<FScalarParameterValueOverTime, UMaterialExpressionScalarParameter>(ScalarParameterValues, ParentMaterial) || bMarkDirty;

			// Vector parameters
			bMarkDirty = UpdateParameterSet<FVectorParameterValueOverTime, UMaterialExpressionVectorParameter>(VectorParameterValues, ParentMaterial) || bMarkDirty;

			// Texture parameters
			bMarkDirty = UpdateParameterSet<FTextureParameterValueOverTime, UMaterialExpressionTextureSampleParameter>(TextureParameterValues, ParentMaterial) || bMarkDirty;

			// Font parameters
			bMarkDirty = UpdateParameterSet<FFontParameterValueOverTime, UMaterialExpressionFontSampleParameter>(FontParameterValues, ParentMaterial) || bMarkDirty;

			for (INT PlatformIndex = 0; PlatformIndex < MSP_MAX; PlatformIndex++)
			{
				// Static switch parameters
				bMarkDirty = UpdateParameterSet<FStaticSwitchParameter, UMaterialExpressionStaticSwitchParameter>(StaticParameters[PlatformIndex]->StaticSwitchParameters, ParentMaterial) || bMarkDirty;

				// Static component mask parameters
				bMarkDirty = UpdateParameterSet<FStaticComponentMaskParameter, UMaterialExpressionStaticComponentMaskParameter>(StaticParameters[PlatformIndex]->StaticComponentMaskParameters, ParentMaterial) || bMarkDirty;

			}

			// At least 1 parameter changed, mark the package dirty.
			if(bMarkDirty)
			{
				MarkPackageDirty();
				InitResources();
			}
		}
	}
}

void UMaterialInstanceTimeVarying::SetParent(UMaterialInterface* NewParent)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetParent"));
#endif

	Super::SetParent( NewParent );

	InitResources();
}

/** 
 * For MITVs you can utilize both single Scalar values and InterpCurve values.
 *
 * If there is any data in the InterpCurve, then the MITV will utilize that. Else it will utilize the Scalar value
 * of the same name.
 **/
void UMaterialInstanceTimeVarying::SetScalarParameterValue(FName ParameterName, FLOAT Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetScalarParameterValue"));
#endif

	FScalarParameterValueOverTime* ParameterValue = MITVScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(ScalarParameterValues) FScalarParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;
		ParameterValue->StartTime = -1.f;
		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	// Set the parameter's value.
	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MITVScalarParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}


void UMaterialInstanceTimeVarying::SetScalarCurveParameterValue(FName ParameterName, FInterpCurveFloat Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetScalarCurveParameterValue"));
#endif

	FScalarParameterValueOverTime* ParameterValue = MITVScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(ScalarParameterValues) FScalarParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ParameterValue = 0.0f;
		ParameterValue->StartTime = -1.f;
		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	// Set the parameter's value.
	ParameterValue->ParameterValueCurve = Value;

	// Update the material instance data in the rendering thread.
	MITVScalarParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

/** This sets how long the MITV will be around (i.e. this MITV is owned by a decal which lasts N seconds) **/
void UMaterialInstanceTimeVarying::SetDuration(FLOAT Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetDuration"));
#endif

	Duration = Value;

	// Update the material instance data in the rendering thread.
	InitResources();
}


void UMaterialInstanceTimeVarying::SetScalarStartTime(FName ParameterName, FLOAT Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetScalarStartTime"));
#endif

	FScalarParameterValueOverTime* ParameterValue = MITVScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// basically this will always be true when you have made a gameplay side MITV and set the parent to be the one the content team has made
		// need to look up the tree to find the named param and copy the values from it into this

		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(ScalarParameterValues) FScalarParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;

		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		if( Parent != NULL )
		{
			FLOAT NewParamValue = 0.0f;
			Parent->GetScalarParameterValue( ParameterName, NewParamValue );
			ParameterValue->ParameterValue = NewParamValue;
		}

		// we need to go ask our parent to find us the Curves we are going to add
		UMaterialInstanceTimeVarying* MITV = Cast<UMaterialInstanceTimeVarying>(Parent);
		if( MITV != NULL )
		{
			FInterpCurveInitFloat Values;
			MITV->GetScalarCurveParameterValue( ParameterName, Values );
			//debugf( TEXT( "NUM PTS %d"), Values.Points.Num() );
			ParameterValue->ParameterValueCurve = Values;
		}

		ParameterValue->ExpressionGUID.Invalidate();
	}
	
	// Set the parameter's start time.
	ParameterValue->StartTime = GWorld->GetTimeSeconds() + Value;

	// Update the material instance data in the rendering thread.
	MITVScalarParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

void UMaterialInstanceTimeVarying::SetVectorParameterValue(FName ParameterName, FLinearColor Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetVectorParameterValue"));
#endif

	FVectorParameterValueOverTime* ParameterValue = MITVVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(VectorParameterValues) FVectorParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;
		ParameterValue->StartTime = -1.f;
		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	// Set the parameter's value.
	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MITVVectorParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

void UMaterialInstanceTimeVarying::SetVectorCurveParameterValue(FName ParameterName, FInterpCurveVector Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetVectorCurveParameterValue"));
#endif

	FVectorParameterValueOverTime* ParameterValue = MITVVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(VectorParameterValues) FVectorParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;
		ParameterValue->ParameterValue = FLinearColor(0.0f,0.0f,0.0f,0.0f);

		ParameterValue->StartTime = -1.f;
		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	// Set the parameter's value.
	ParameterValue->ParameterValueCurve = Value;

	// Update the material instance data in the rendering thread.
	MITVVectorParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

void UMaterialInstanceTimeVarying::SetVectorStartTime(FName ParameterName, FLOAT Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetVectorStartTime"));
#endif

	FVectorParameterValueOverTime* ParameterValue = MITVVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(VectorParameterValues) FVectorParameterValueOverTime;
		appMemzero( &ParameterValue->ParameterValueCurve.Points, sizeof(ParameterValue->ParameterValueCurve.Points) );

		ParameterValue->ParameterName = ParameterName;

		if( Parent != NULL )
		{
			FLinearColor Value;
			Parent->GetVectorParameterValue( ParameterName, Value );
			ParameterValue->ParameterValue = Value;
		}

		// we need to go ask our parent to find us the Curves we are going to add
		UMaterialInstanceTimeVarying* MITV = Cast<UMaterialInstanceTimeVarying>(Parent);
		if( MITV != NULL )
		{
			FInterpCurveInitVector Values;
			MITV->GetVectorCurveParameterValue( ParameterName, Values );
			//debugf( TEXT( "NUM PTS %d"), Values.Points.Num() );
			ParameterValue->ParameterValueCurve = Values;
		}

		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	// Set the parameter's start time.
	ParameterValue->StartTime = GWorld->GetTimeSeconds() + Value;

	// Update the material instance data in the rendering thread.
	MITVVectorParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}

void UMaterialInstanceTimeVarying::SetTextureParameterValue(FName ParameterName, UTexture* Value)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetTextureParameterValue"));
#endif

	FTextureParameterValueOverTime* ParameterValue = MITVTextureParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		ParameterValue = new(TextureParameterValues) FTextureParameterValueOverTime;
		ParameterValue->ParameterName = ParameterName;
		ParameterValue->StartTime = -1.0f;
		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->ParameterValue = Value;

	// Update the material instance data in the rendering thread.
	MITVTextureParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}




/**
* Sets the value of the given font parameter.  
*
* @param	ParameterName	The name of the font parameter
* @param	OutFontValue	New font value to set for this MIC
* @param	OutFontPage		New font page value to set for this MIC
*/
void UMaterialInstanceTimeVarying::SetFontParameterValue(FName ParameterName,class UFont* FontValue,INT FontPage)
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::SetFontParameterValue"));
#endif

	FFontParameterValueOverTime* ParameterValue = MITVFontParameterMapping::FindParameterByName(this,ParameterName);
	if(!ParameterValue)
	{
		// If there's no element for the named parameter in array yet, add one.
		ParameterValue = new(FontParameterValues) FFontParameterValueOverTime;
		ParameterValue->ParameterName = ParameterName;

		ParameterValue->bLoop = FALSE;
		ParameterValue->bAutoActivate = FALSE;
		ParameterValue->CycleTime = 1.0f;
		ParameterValue->bNormalizeTime = FALSE;
		ParameterValue->OffsetTime = 0.0f;
		ParameterValue->bOffsetFromEnd = FALSE;

		ParameterValue->ExpressionGUID.Invalidate();
	}

	ParameterValue->FontValue = FontValue;
	ParameterValue->FontPage = FontPage;

	// Update the material instance data in the rendering thread.
	MITVFontParameterMapping::GameThread_UpdateParameter(this,*ParameterValue);
}


/** Removes all parameter values */
void UMaterialInstanceTimeVarying::ClearParameterValues()
{
#if !FINAL_RELEASE
	CheckSafeToModifyInGame(TEXT("MITV::ClearParameterValues"));
#endif

	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	TextureParameterValues.Empty();
	FontParameterValues.Empty();

	// Update the material instance data in the rendering thread.
	InitResources();
}


/** This will interrogate all of the parameter and see what the max duration needed for them is.  Useful for setting the Duration / or knowing how long this MITV will take **/
FLOAT UMaterialInstanceTimeVarying::GetMaxDurationFromAllParameters()
{
	FLOAT Retval = -1.0f;

	// look over each of the various curves to get the max 

	FLOAT MaxDuration = 0.0f;

	UMaterialInstance* MI = this;

	// look up the hierarchy
	do 
	{ 
		UMaterialInstanceTimeVarying* MITV = Cast<UMaterialInstanceTimeVarying>(MI);
		
		if( MITV != NULL )
		{
			for( INT Idx = 0; Idx < MITV->ScalarParameterValues.Num(); ++Idx )
			{
				const FScalarParameterValueOverTime& TheStruct = MITV->ScalarParameterValues(Idx);

				// we must be at least as long as the OffsetTime time
				if( MaxDuration < TheStruct.OffsetTime )
				{
					MaxDuration = TheStruct.OffsetTime;
				}

				const FInterpCurveFloat& TheCurve = TheStruct.ParameterValueCurve;

				if( TheCurve.Points.Num() > 0 )
				{
					const INT MaxPoints = TheCurve.Points.Num()-1;

					//warnf( TEXT( " %d %f" ), MaxPoints, TheCurve.Points(MaxPoints).InVal  );

					if( MaxDuration < TheCurve.Points(MaxPoints).InVal )
					{
						MaxDuration = TheCurve.Points(MaxPoints).InVal;
					}
				}
			}

			for( INT Idx = 0; Idx < MITV->VectorParameterValues.Num(); ++Idx )
			{
				const FVectorParameterValueOverTime& TheStruct = MITV->VectorParameterValues(Idx);

				// we must be at least as long as the OffsetTime time
				if( MaxDuration < TheStruct.OffsetTime )
				{
					MaxDuration = TheStruct.OffsetTime;
				}

				const FInterpCurveVector& TheCurve = TheStruct.ParameterValueCurve;

				if( TheCurve.Points.Num() > 0 )
				{
					const INT MaxPoints = TheCurve.Points.Num()-1;
					if( MaxDuration < TheCurve.Points(MaxPoints).InVal )
					{
						MaxDuration = TheCurve.Points(MaxPoints).InVal;
					}
				}
			}

		}

		MI = Cast<UMaterialInstance>(MI->Parent);

	} while( MI != NULL );

	Retval = MaxDuration;

	//warnf( TEXT( "GetMaxDurationFromAllParameters: %f" ), Retval );

	return Retval;
}


UBOOL UMaterialInstanceTimeVarying::GetVectorParameterValue(FName ParameterName, FLinearColor& OutValue)
{
	UBOOL bFoundAValue = FALSE;

	if(ReentrantFlag)
	{
		return FALSE;
	}

	FVectorParameterValueOverTime* ParameterValue = MITVVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue)
	{
		if( ParameterValue->ParameterValueCurve.Points.Num() > 0 )
		{
			if( ( ParameterValue->bAutoActivate == TRUE )
				|| ( bAutoActivateAll == TRUE )
				|| ( ParameterValue->StartTime >= 0.0f )
				)
			{
				FLOAT EvalTime = (GWorld->GetTimeSeconds() - ParameterValue->StartTime);
				const FLOAT CycleTime = ParameterValue->CycleTime;

				if( CycleTime > 0.0f )
				{
					if( ParameterValue->bLoop == TRUE )
					{
						EvalTime = appFmod(EvalTime,CycleTime);
						// check for StartTime being in the future as in the Editor this can often be the case
						if( EvalTime < 0.0f )
						{
							EvalTime += CycleTime;
						}
					}
					if( ParameterValue->bNormalizeTime == TRUE )
					{
						EvalTime /= CycleTime;
					}
				}

				const FVector& EvalValue = ParameterValue->ParameterValueCurve.Eval( EvalTime, FVector(0.0f,0.0f,0.0f) );
				
				OutValue.R = EvalValue.X;
				OutValue.G = EvalValue.Y;
				OutValue.B = EvalValue.Z;
				OutValue.A = 0.0f;

				return TRUE;
			}
		}
		else
		{
			OutValue = ParameterValue->ParameterValue;
			return TRUE;
		}
	}

	if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetVectorParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}

/** 
 * For MITVs you can utilize both single Scalar values and InterpCurve values.
 *
 * If there is any data in the InterpCurve, then the MITV will utilize that. Else it will utilize the Scalar value
 * of the same name.
 **/
UBOOL UMaterialInstanceTimeVarying::GetScalarParameterValue(FName ParameterName, FLOAT& OutValue)
{
	UBOOL bFoundAValue = FALSE;

	if(ReentrantFlag)
	{
		return FALSE;
	}

	FScalarParameterValueOverTime* ParameterValue = MITVScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue)
	{
		if( ParameterValue->ParameterValueCurve.Points.Num() > 0 )
		{
			if( ( ParameterValue->bAutoActivate == TRUE )
				|| ( bAutoActivateAll == TRUE )
				|| ( ParameterValue->StartTime >= 0.0f )
				)
			{
				FLOAT EvalTime = (GWorld->GetTimeSeconds() - ParameterValue->StartTime);
				const FLOAT CycleTime = ParameterValue->CycleTime;

				if ( CycleTime > 0.0f )
				{
					if( ParameterValue->bLoop == TRUE )
					{
						EvalTime = appFmod(EvalTime,CycleTime);
						// check for StartTime being in the future as in the Editor this can often be the case
						if( EvalTime < 0.0f )
						{
							EvalTime += CycleTime;
						}
					}
					if( ParameterValue->bNormalizeTime == TRUE )
					{
						EvalTime /= CycleTime;
					}
				}
				OutValue = ParameterValue->ParameterValueCurve.Eval( EvalTime, 0.0f );
				return TRUE;
			}
		}
		else
		{
			OutValue = ParameterValue->ParameterValue;
			return TRUE;
		}

	}

	if(Parent)
	{
		FMICReentranceGuard	Guard(this);
		return Parent->GetScalarParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}


UBOOL UMaterialInstanceTimeVarying::GetScalarCurveParameterValue(FName ParameterName,FInterpCurveFloat& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FScalarParameterValueOverTime* ParameterValue = MITVScalarParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue && ParameterValue->ParameterValueCurve.Points.Num())
	{
		OutValue = ParameterValue->ParameterValueCurve;
		return TRUE;
	}
	else if(Cast<UMaterialInstanceTimeVarying>(Parent))
	{
		FMICReentranceGuard	Guard(this);
		return Cast<UMaterialInstanceTimeVarying>(Parent)->GetScalarCurveParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}


UBOOL UMaterialInstanceTimeVarying::GetVectorCurveParameterValue(FName ParameterName,FInterpCurveVector& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FVectorParameterValueOverTime* ParameterValue = MITVVectorParameterMapping::FindParameterByName(this,ParameterName);
	if(ParameterValue && ParameterValue->ParameterValueCurve.Points.Num())
	{
		OutValue = ParameterValue->ParameterValueCurve;
		return TRUE;
	}
	else if(Cast<UMaterialInstanceTimeVarying>(Parent))
	{
		FMICReentranceGuard	Guard(this);
		return Cast<UMaterialInstanceTimeVarying>(Parent)->GetVectorCurveParameterValue(ParameterName,OutValue);
	}
	else
	{
		return FALSE;
	}
}


UBOOL UMaterialInstanceTimeVarying::GetTextureParameterValue(FName ParameterName, UTexture*& OutValue)
{
	if(ReentrantFlag)
	{
		return FALSE;
	}

	FTextureParameterValueOverTime* ParameterValue = MITVTextureParameterMapping::FindParameterByName(this,ParameterName);
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

/**
* Gets the value of the given font parameter.  If it is not found in this instance then
* the request is forwarded up the MIC chain.
*
* @param	ParameterName	The name of the font parameter
* @param	OutFontValue	Will contain the font value of the font value if successful
* @param	OutFontPage		Will contain the font value of the font page if successful
* @return					True if successful
*/
UBOOL UMaterialInstanceTimeVarying::GetFontParameterValue(FName ParameterName,class UFont*& OutFontValue, INT& OutFontPage)
{
	if( ReentrantFlag )
	{
		return FALSE;
	}

	FFontParameterValueOverTime* ParameterValue = MITVFontParameterMapping::FindParameterByName(this,ParameterName);
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

	return TRUE;
}
