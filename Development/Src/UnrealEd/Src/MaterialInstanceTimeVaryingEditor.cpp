/**
* MaterialInstanceTimeVaryingEditor.cpp: Material instance editor class.
*
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

#include "UnrealEd.h"
#include "EngineMaterialClasses.h"
#include "Properties.h"
#include "GenericBrowser.h"
#include "MaterialEditorBase.h"
#include "MaterialEditorToolBar.h"
#include "MaterialInstanceTimeVaryingEditor.h"
#include "MaterialEditorPreviewScene.h"
#include "NewMaterialEditor.h"
#include "PropertyWindowManager.h"	// required for access to GPropertyWindowManager
#include "MaterialInstanceTimeVaryingHelpers.h"

//////////////////////////////////////////////////////////////////////////
// UMaterialEditorInstanceTimeVarying
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UMaterialEditorInstanceTimeVarying);

void UMaterialEditorInstanceTimeVarying::PostEditChange(UProperty* Property)
{
	if(Property->GetName()==TEXT("Parent"))
	{
		// If the parent was changed to the source instance, set it to NULL.
		if(Parent==SourceInstance)
		{
			Parent = NULL;
		}

		SourceInstance->SetParent(Parent);
		RegenerateArrays();
		FGlobalComponentReattachContext RecreateComponents;
	}

	CopyToSourceInstance();

	// Tell our source instance to update itself so the preview updates.
	SourceInstance->PostEditChange(Property);
}

/** Regenerates the parameter arrays. */
void UMaterialEditorInstanceTimeVarying::RegenerateArrays()
{
	// Clear out existing parameters.
	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	TextureParameterValues.Empty();
	FontParameterValues.Empty();
	StaticSwitchParameterValues.Empty();
	StaticComponentMaskParameterValues.Empty();
	VisibleExpressions.Empty();

	if(Parent)
	{
		TArray<FGuid> Guids;

		// Only operate on base materials, fallback material parameters are required to be a subset of the main material.
		UMaterial* ParentMaterial = Parent->GetMaterial(MSP_SM3);
		SourceInstance->UpdateParameterNames();	// Update any parameter names that may have changed.

		// Loop through all types of parameters for this material and add them to the parameter arrays.
		TArray<FName> ParameterNames;

		// Vector Parameters.
		ParentMaterial->GetAllVectorParameterNames(ParameterNames, Guids);
		VectorParameterValues.AddZeroed(ParameterNames.Num());

		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorVectorParameterValueOverTime& ParameterValue = VectorParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FLinearColor Value;
			FInterpCurveInitVector Values;
			
			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetVectorCurveParameterValue(ParameterName, Values))
			{
				ParameterValue.ParameterValueCurve = Values;
			}

			if(SourceInstance->GetVectorParameterValue(ParameterName, Value))
			{
				ParameterValue.ParameterValue = Value;
			}

			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->VectorParameterValues.Num(); ParameterIdx++)
			{
				FVectorParameterValueOverTime& SourceParam = SourceInstance->VectorParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValueCurve = SourceParam.ParameterValueCurve;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
					
					ParameterValue.bLoop = SourceParam.bLoop;
					ParameterValue.bAutoActivate = SourceParam.bAutoActivate;
					ParameterValue.CycleTime = SourceParam.CycleTime;
					ParameterValue.bNormalizeTime = SourceParam.bNormalizeTime;

					ParameterValue.OffsetTime = SourceParam.OffsetTime;
					ParameterValue.bOffsetFromEnd = SourceParam.bOffsetFromEnd;
				}
			}
		}

		// Scalar Parameters.
		ParentMaterial->GetAllScalarParameterNames(ParameterNames, Guids);
		ScalarParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorScalarParameterValueOverTime& ParameterValue = ScalarParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FInterpCurveInitFloat Values;

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetScalarCurveParameterValue(ParameterName, Values))
			{
				ParameterValue.ParameterValueCurve = Values;
			}

			FLOAT Value;
			if(SourceInstance->GetScalarParameterValue(ParameterName, Value))
			{
				ParameterValue.ParameterValue = Value;
			}


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->ScalarParameterValues.Num(); ParameterIdx++)
			{
				FScalarParameterValueOverTime& SourceParam = SourceInstance->ScalarParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValueCurve = SourceParam.ParameterValueCurve;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;

					ParameterValue.bLoop = SourceParam.bLoop;
					ParameterValue.bAutoActivate = SourceParam.bAutoActivate;
					ParameterValue.CycleTime = SourceParam.CycleTime;
					ParameterValue.bNormalizeTime = SourceParam.bNormalizeTime;

					ParameterValue.OffsetTime = SourceParam.OffsetTime;
					ParameterValue.bOffsetFromEnd = SourceParam.bOffsetFromEnd;
				}
			}
		}


		// Texture Parameters.
		ParentMaterial->GetAllTextureParameterNames(ParameterNames, Guids);
		TextureParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorTextureParameterValueOverTime& ParameterValue = TextureParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			UTexture* Value;

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetTextureParameterValue(ParameterName, Value))
			{
				ParameterValue.ParameterValue = Value;
			}


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->TextureParameterValues.Num(); ParameterIdx++)
			{
				FTextureParameterValueOverTime& SourceParam = SourceInstance->TextureParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;

					ParameterValue.bLoop = SourceParam.bLoop;
					ParameterValue.bAutoActivate = SourceParam.bAutoActivate;
					ParameterValue.CycleTime = SourceParam.CycleTime;
					ParameterValue.bNormalizeTime = SourceParam.bNormalizeTime;

					ParameterValue.OffsetTime = SourceParam.OffsetTime;
					ParameterValue.bOffsetFromEnd = SourceParam.bOffsetFromEnd;
				}
			}
		}

		// Font Parameters.
		ParentMaterial->GetAllFontParameterNames(ParameterNames, Guids);
		FontParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorFontParameterValueOverTime& ParameterValue = FontParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			UFont* FontValue;
			INT FontPage;

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetFontParameterValue(ParameterName, FontValue,FontPage))
			{
				ParameterValue.FontValue = FontValue;
				ParameterValue.FontPage = FontPage;
			}


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->FontParameterValues.Num(); ParameterIdx++)
			{
				FFontParameterValueOverTime& SourceParam = SourceInstance->FontParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.FontValue = SourceParam.FontValue;
					ParameterValue.FontPage = SourceParam.FontPage;

					ParameterValue.bLoop = SourceParam.bLoop;
					ParameterValue.bAutoActivate = SourceParam.bAutoActivate;
					ParameterValue.CycleTime = SourceParam.CycleTime;
					ParameterValue.bNormalizeTime = SourceParam.bNormalizeTime;

					ParameterValue.OffsetTime = SourceParam.OffsetTime;
					ParameterValue.bOffsetFromEnd = SourceParam.bOffsetFromEnd;
				}
			}
		}

		// Static Switch Parameters
		ParentMaterial->GetAllStaticSwitchParameterNames(ParameterNames, Guids);
		StaticSwitchParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorStaticSwitchParameterValueOverTime& EditorParameter = StaticSwitchParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			UBOOL Value = FALSE;
			FGuid ExpressionId = Guids(ParameterIdx);

			EditorParameter.bOverride = FALSE;
			EditorParameter.ParameterName = ParameterName;
			EditorParameter.ExpressionId = Guids(ParameterIdx);

			//get the settings from the parent in the MIC chain
			if(SourceInstance->Parent->GetStaticSwitchParameterValue(ParameterName, Value, ExpressionId))
			{
				EditorParameter.ParameterValue = Value;
			}
			EditorParameter.ExpressionId = ExpressionId;

			//if the SourceInstance is overriding this parameter, use its settings
			for(INT ParameterIdx = 0; ParameterIdx < SourceInstance->StaticParameters[MSP_SM3]->StaticSwitchParameters.Num(); ParameterIdx++)
			{
				const FStaticSwitchParameter& StaticSwitchParam = SourceInstance->StaticParameters[MSP_SM3]->StaticSwitchParameters(ParameterIdx);

				if(ParameterName == StaticSwitchParam.ParameterName)
				{
					EditorParameter.bOverride = StaticSwitchParam.bOverride;
					if (StaticSwitchParam.bOverride)
					{
						EditorParameter.ParameterValue = StaticSwitchParam.Value;
					}
				}
			}
		}


		// Static Component Mask Parameters
		ParentMaterial->GetAllStaticComponentMaskParameterNames(ParameterNames, Guids);
		StaticComponentMaskParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorStaticComponentMaskParameterValueOverTime& EditorParameter = StaticComponentMaskParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			UBOOL R = FALSE;
			UBOOL G = FALSE;
			UBOOL B = FALSE;
			UBOOL A = FALSE;
			FGuid ExpressionId = Guids(ParameterIdx);

			EditorParameter.bOverride = FALSE;
			EditorParameter.ParameterName = ParameterName;

			//get the settings from the parent in the MIC chain
			if(SourceInstance->Parent->GetStaticComponentMaskParameterValue(ParameterName, R, G, B, A, ExpressionId))
			{
				EditorParameter.ParameterValue.R = R;
				EditorParameter.ParameterValue.G = G;
				EditorParameter.ParameterValue.B = B;
				EditorParameter.ParameterValue.A = A;
			}
			EditorParameter.ExpressionId = ExpressionId;

			//if the SourceInstance is overriding this parameter, use its settings
			for(INT ParameterIdx = 0; ParameterIdx < SourceInstance->StaticParameters[MSP_SM3]->StaticComponentMaskParameters.Num(); ParameterIdx++)
			{
				const FStaticComponentMaskParameter& StaticComponentMaskParam = SourceInstance->StaticParameters[MSP_SM3]->StaticComponentMaskParameters(ParameterIdx);

				if(ParameterName == StaticComponentMaskParam.ParameterName)
				{
					EditorParameter.bOverride = StaticComponentMaskParam.bOverride;
					if (StaticComponentMaskParam.bOverride)
					{
						EditorParameter.ParameterValue.R = StaticComponentMaskParam.R;
						EditorParameter.ParameterValue.G = StaticComponentMaskParam.G;
						EditorParameter.ParameterValue.B = StaticComponentMaskParam.B;
						EditorParameter.ParameterValue.A = StaticComponentMaskParam.A;
					}
				}
			}
		}

		WxMaterialEditor::GetVisibleMaterialParameters(ParentMaterial, SourceInstance, VisibleExpressions);
	}
}

/** 
 * Copies the parameter array values back to the source instance. 
 *
 */
void UMaterialEditorInstanceTimeVarying::CopyToSourceInstance()
{
	SourceInstance->MarkPackageDirty();

	SourceInstance->VectorParameterValues.Empty();
	SourceInstance->ScalarParameterValues.Empty();
	SourceInstance->TextureParameterValues.Empty();
	SourceInstance->FontParameterValues.Empty();

	// Scalar Parameters
	for(INT ParameterIdx=0; ParameterIdx<ScalarParameterValues.Num(); ParameterIdx++)
	{
		FEditorScalarParameterValueOverTime& ParameterValue = ScalarParameterValues(ParameterIdx);

		//debugf( TEXT( "CopyToSourceInstance: %s"), *ParameterValue.ParameterName.ToString() );

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetScalarCurveParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValueCurve);
			SourceInstance->SetScalarParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);

			UpdateParameterValueOverTimeValues<UMaterialInstanceTimeVarying, FScalarParameterValueOverTime>( SourceInstance, ParameterValue.ParameterName, ParameterValue.bLoop, ParameterValue.bAutoActivate, ParameterValue.CycleTime, ParameterValue.bNormalizeTime, ParameterValue.OffsetTime, ParameterValue.bOffsetFromEnd );
		}
	}

	// Texture Parameters
	for(INT ParameterIdx=0; ParameterIdx<TextureParameterValues.Num(); ParameterIdx++)
	{
		FEditorTextureParameterValueOverTime& ParameterValue = TextureParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetTextureParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);

			UpdateParameterValueOverTimeValues<UMaterialInstanceTimeVarying, FTextureParameterValueOverTime>( SourceInstance, ParameterValue.ParameterName, ParameterValue.bLoop, ParameterValue.bAutoActivate, ParameterValue.CycleTime, ParameterValue.bNormalizeTime, ParameterValue.OffsetTime, ParameterValue.bOffsetFromEnd );
		}
	}

	// Font Parameters
	for(INT ParameterIdx=0; ParameterIdx<FontParameterValues.Num(); ParameterIdx++)
	{
		FEditorFontParameterValueOverTime& ParameterValue = FontParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetFontParameterValue(ParameterValue.ParameterName,ParameterValue.FontValue,ParameterValue.FontPage);

			UpdateParameterValueOverTimeValues<UMaterialInstanceTimeVarying, FFontParameterValueOverTime>( SourceInstance, ParameterValue.ParameterName, ParameterValue.bLoop, ParameterValue.bAutoActivate, ParameterValue.CycleTime, ParameterValue.bNormalizeTime, ParameterValue.OffsetTime, ParameterValue.bOffsetFromEnd );
		}
	}

	// Vector Parameters
	for(INT ParameterIdx=0; ParameterIdx<VectorParameterValues.Num(); ParameterIdx++)
	{
		FEditorVectorParameterValueOverTime& ParameterValue = VectorParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetVectorCurveParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValueCurve);
			SourceInstance->SetVectorParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);

			UpdateParameterValueOverTimeValues<UMaterialInstanceTimeVarying, FVectorParameterValueOverTime>( SourceInstance, ParameterValue.ParameterName, ParameterValue.bLoop, ParameterValue.bAutoActivate, ParameterValue.CycleTime, ParameterValue.bNormalizeTime, ParameterValue.OffsetTime, ParameterValue.bOffsetFromEnd );
		}
	}

	CopyStaticParametersToSourceInstance();

	// Copy phys material back to source instance
	SourceInstance->PhysMaterial = PhysMaterial;
	SourceInstance->bAutoActivateAll = bAutoActivateAll;      

	// Update object references and parameter names.
	SourceInstance->UpdateParameterNames();
}

/** Copies static parameters to the source instance, which will be marked dirty if a compile was necessary */
void UMaterialEditorInstanceTimeVarying::CopyStaticParametersToSourceInstance()
{
	//build a static parameter set containing all static parameter settings
	FStaticParameterSet StaticParameters;

	// Static Switch Parameters
	for(INT ParameterIdx = 0; ParameterIdx < StaticSwitchParameterValues.Num(); ParameterIdx++)
	{
		FEditorStaticSwitchParameterValueOverTime& EditorParameter = StaticSwitchParameterValues(ParameterIdx);
		UBOOL SwitchValue = EditorParameter.ParameterValue;
		FGuid ExpressionIdValue = EditorParameter.ExpressionId;
		if (!EditorParameter.bOverride)
		{
			if (Parent)
			{
				//use the parent's settings if this parameter is not overridden
				SourceInstance->Parent->GetStaticSwitchParameterValue(EditorParameter.ParameterName, SwitchValue, ExpressionIdValue);
			}
		}
		FStaticSwitchParameter * NewParameter = 
			new(StaticParameters.StaticSwitchParameters) FStaticSwitchParameter(EditorParameter.ParameterName, SwitchValue, EditorParameter.bOverride, ExpressionIdValue);
	}

	// Static Component Mask Parameters
	for(INT ParameterIdx = 0; ParameterIdx < StaticComponentMaskParameterValues.Num(); ParameterIdx++)
	{
		FEditorStaticComponentMaskParameterValueOverTime& EditorParameter = StaticComponentMaskParameterValues(ParameterIdx);
		UBOOL MaskR = EditorParameter.ParameterValue.R;
		UBOOL MaskG = EditorParameter.ParameterValue.G;
		UBOOL MaskB = EditorParameter.ParameterValue.B;
		UBOOL MaskA = EditorParameter.ParameterValue.A;
		FGuid ExpressionIdValue = EditorParameter.ExpressionId;

		if (!EditorParameter.bOverride)
		{
			if (Parent)
			{
				//use the parent's settings if this parameter is not overridden
				SourceInstance->Parent->GetStaticComponentMaskParameterValue(EditorParameter.ParameterName, MaskR, MaskG, MaskB, MaskA, ExpressionIdValue);
			}
		}
		FStaticComponentMaskParameter * NewParameter = new(StaticParameters.StaticComponentMaskParameters) 
			FStaticComponentMaskParameter(EditorParameter.ParameterName, MaskR, MaskG, MaskB, MaskA, EditorParameter.bOverride, ExpressionIdValue);
	}

	if (SourceInstance->SetStaticParameterValues(&StaticParameters))
	{
		//mark the package dirty if a compile was needed
		SourceInstance->MarkPackageDirty();
	}
}


/**  
 * Sets the source instance for this object and regenerates arrays. 
 *
 * @param MaterialInterface		Instance to use as the source for this material editor instance.
 */
void UMaterialEditorInstanceTimeVarying::SetSourceInstance(UMaterialInstanceTimeVarying* MaterialInterface)
{
	check(MaterialInterface);
	SourceInstance = MaterialInterface;
	Parent = SourceInstance->Parent;
	PhysMaterial = SourceInstance->PhysMaterial;
	bAutoActivateAll = SourceInstance->bAutoActivateAll;

	RegenerateArrays();

	//propagate changes to the base material so the instance will be updated if it has a static permutation resource
	CopyStaticParametersToSourceInstance();
	SourceInstance->UpdateStaticPermutation();
}


//////////////////////////////////////////////////////////////////////////
// WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter, WxCustomPropertyItem_ConditionalItem);

BEGIN_EVENT_TABLE(WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter, WxCustomPropertyItem_ConditionalItem)
	EVT_BUTTON(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_RESETTODEFAULT, OnResetToDefault)
END_EVENT_TABLE()

WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter() : 
WxCustomPropertyItem_ConditionalItem()
{
	ResetToDefault = NULL;
	bAllowEditing = FALSE;
}

/**
 * Initialize this property window.  Must be the first function called after creating the window.
 */
void WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::Create(	wxWindow* InParent,
					WxPropertyWindow_Base* InParentItem,
					WxPropertyWindow* InTopPropertyWindow,
					UProperty* InProperty,
					INT InPropertyOffset,
					INT	 InArrayIdx,
					UBOOL bInSupportsCustomControls )
{
	WxCustomPropertyItem_ConditionalItem::Create(InParent, InParentItem, InTopPropertyWindow, InProperty, InPropertyOffset, InArrayIdx, bInSupportsCustomControls);

	// Create a new button and add it to the button array.
	if(ResetToDefault==NULL)
	{
		ResetToDefault = new wxBitmapButton( this, ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_RESETTODEFAULT, GPropertyWindowManager->Prop_ResetToDefaultB );
		IndentX += 15;

		// Generate tooltip text for this button.
		UMaterialEditorInstanceTimeVarying* Instance = GetInstanceObject();

		if(Instance && Instance->Parent)
		{
			FString ToolTipText = *LocalizeUnrealEd("ResetToDefault");
			FName PropertyName = PropertyStructName;

			FName ScalarArrayName(TEXT("ScalarParameterValues"));
			FName TextureArrayName(TEXT("TextureParameterValues"));
			FName FontArrayName(TEXT("FontParameterValues"));
			FName VectorArrayName(TEXT("VectorParameterValues"));
			FName StaticSwitchArrayName(TEXT("StaticSwitchParameterValues"));
			FName StaticComponentMaskArrayName(TEXT("StaticComponentMaskParameterValues"));

			if(PropertyName==ScalarArrayName)
			{
				FLOAT OutValue;
				if(Instance->Parent->GetScalarParameterValue(DisplayName, OutValue))
				{
					ToolTipText += TEXT(" ");
					ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceFloatValue_F"), OutValue));
				}
			}
			else if(PropertyName==TextureArrayName)
			{
				UTexture* OutValue;
				if(Instance->Parent->GetTextureParameterValue(DisplayName, OutValue))
				{
					if(OutValue)
					{
						ToolTipText += TEXT(" ");
						ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceTextureValue_F"), *OutValue->GetName()));
					}
				}				
			}
			else if(PropertyName==FontArrayName)
			{
				UFont* OutFontValue;
				INT OutFontPage;
				if(Instance->Parent->GetFontParameterValue(DisplayName, OutFontValue,OutFontPage))
				{
					if(OutFontValue)
					{
						ToolTipText += TEXT(" ");
						ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceFontValue_F"), *OutFontValue->GetName(),OutFontPage));
					}
				}				
			}
			else if(PropertyName==VectorArrayName)
			{
				FLinearColor OutValue;
				if(Instance->Parent->GetVectorParameterValue(DisplayName, OutValue))
				{
					ToolTipText += TEXT(" ");
					ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceVectorValue_F"), OutValue.R, OutValue.G, OutValue.B, OutValue.A));
				}				
			}
			else if(PropertyName==StaticSwitchArrayName)
			{
				UBOOL OutValue;
				FGuid TempGuid(0,0,0,0);
				if(Instance->Parent->GetStaticSwitchParameterValue(DisplayName, OutValue, TempGuid))
				{
					ToolTipText += TEXT(" ");
					ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceStaticSwitchValue_F"), (INT)OutValue));
				}				
			}
			else if(PropertyName==StaticComponentMaskArrayName)
			{
				UBOOL OutValue[4];
				FGuid TempGuid(0,0,0,0);

				if(Instance->Parent->GetStaticComponentMaskParameterValue(DisplayName, OutValue[0], OutValue[1], OutValue[2], OutValue[3], TempGuid))
				{
					ToolTipText += TEXT(" ");
					ToolTipText += FString::Printf(LocalizeSecure(LocalizeUnrealEd("MaterialInstanceStaticComponentMaskValue_F"), (INT)OutValue[0], (INT)OutValue[1], (INT)OutValue[2], (INT)OutValue[3]));
				}				
			}

			ResetToDefault->SetToolTip(*ToolTipText);
		}
	}
}

/**
 * Toggles the value of the property being used as the condition for editing this property.
 *
 * @return	the new value of the condition (i.e. TRUE if the condition is now TRUE)
 */
UBOOL WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::ToggleConditionValue()
{	
	UMaterialEditorInstanceTimeVarying* Instance = GetInstanceObject();

	if(Instance)
	{
		FName PropertyName = PropertyStructName;
		FName ScalarArrayName(TEXT("ScalarParameterValues"));
		FName TextureArrayName(TEXT("TextureParameterValues"));
		FName FontArrayName(TEXT("FontParameterValues"));
		FName VectorArrayName(TEXT("VectorParameterValues"));
		FName StaticSwitchArrayName(TEXT("StaticSwitchParameterValues"));
		FName StaticComponentMaskArrayName(TEXT("StaticComponentMaskParameterValues"));

		if(PropertyName==ScalarArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->ScalarParameterValues.Num();ParamIdx++)
			{
				FEditorScalarParameterValueOverTime& Param = Instance->ScalarParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==TextureArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->TextureParameterValues.Num();ParamIdx++)
			{
				FEditorTextureParameterValueOverTime& Param = Instance->TextureParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==FontArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->FontParameterValues.Num();ParamIdx++)
			{
				FEditorFontParameterValueOverTime& Param = Instance->FontParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==VectorArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->VectorParameterValues.Num();ParamIdx++)
			{
				FEditorVectorParameterValueOverTime& Param = Instance->VectorParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==StaticSwitchArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->StaticSwitchParameterValues.Num();ParamIdx++)
			{
				FEditorStaticSwitchParameterValueOverTime& Param = Instance->StaticSwitchParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==StaticComponentMaskArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->StaticComponentMaskParameterValues.Num();ParamIdx++)
			{
				FEditorStaticComponentMaskParameterValueOverTime& Param = Instance->StaticComponentMaskParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}

		// Notify the instance that we modified an override so it needs to update itself.
		Instance->PostEditChange(Property);
	}

	// Always allow editing even if we aren't overriding values.
 	return TRUE;
}


/**
 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
 * associated property should be allowed.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::IsOverridden()
{
	UMaterialEditorInstanceTimeVarying* Instance = GetInstanceObject();

	if(Instance)
	{
		FName PropertyName = PropertyStructName;
		FName ScalarArrayName(TEXT("ScalarParameterValues"));
		FName TextureArrayName(TEXT("TextureParameterValues"));
		FName FontArrayName(TEXT("FontParameterValues"));
		FName VectorArrayName(TEXT("VectorParameterValues"));
		FName StaticSwitchArrayName(TEXT("StaticSwitchParameterValues"));
		FName StaticComponentMaskArrayName(TEXT("StaticComponentMaskParameterValues"));

		if(PropertyName==ScalarArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->ScalarParameterValues.Num();ParamIdx++)
			{
				FEditorScalarParameterValueOverTime& Param = Instance->ScalarParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==TextureArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->TextureParameterValues.Num();ParamIdx++)
			{
				FEditorTextureParameterValueOverTime& Param = Instance->TextureParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==FontArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->FontParameterValues.Num();ParamIdx++)
			{
				FEditorFontParameterValueOverTime& Param = Instance->FontParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==VectorArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->VectorParameterValues.Num();ParamIdx++)
			{
				FEditorVectorParameterValueOverTime& Param = Instance->VectorParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==StaticSwitchArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->StaticSwitchParameterValues.Num();ParamIdx++)
			{
				FEditorStaticSwitchParameterValueOverTime& Param = Instance->StaticSwitchParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==StaticComponentMaskArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->StaticComponentMaskParameterValues.Num();ParamIdx++)
			{
				FEditorStaticComponentMaskParameterValueOverTime& Param = Instance->StaticComponentMaskParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
	}

	return bAllowEditing;
}


/**
 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
 * associated property should be allowed.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::IsConditionMet()
{
	return TRUE;
}

/** @return Returns the instance object this property is associated with. */
UMaterialEditorInstanceTimeVarying* WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::GetInstanceObject()
{
	WxPropertyWindow_Objects* ItemParent = FindObjectItemParent();
	UMaterialEditorInstanceTimeVarying* MaterialInterface = NULL;

	if(ItemParent)
	{
		for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
		{
			MaterialInterface = Cast<UMaterialEditorInstanceTimeVarying>(*It);
			break;
		}
	}

	return MaterialInterface;
}

/**
 * Renders the left side of the property window item.
 *
 * This version is responsible for rendering the checkbox used for toggling whether this property item window should be enabled.
 *
 * @param	RenderDeviceContext		the device context to use for rendering the item name
 * @param	ClientRect				the bounding region of the property window item
 */
void WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect )
{
	const UBOOL bItemEnabled = IsOverridden();

	// determine which checkbox image to render
	const WxMaskedBitmap& bmp = bItemEnabled
		? GPropertyWindowManager->CheckBoxOnB
		: GPropertyWindowManager->CheckBoxOffB;

	INT CurrentX = ClientRect.x + IndentX - PROP_Indent - 15;
	INT CurrentY = ClientRect.y + ((PROP_DefaultItemHeight - bmp.GetHeight()) / 2);

	ResetToDefault->SetSize(CurrentX, CurrentY, 15, 15);
	CurrentX += 15;

	// render the checkbox bitmap
	RenderDeviceContext.DrawBitmap( bmp, CurrentX, CurrentY, 1 );
	CurrentX += PROP_Indent;

	INT W, H;
	RenderDeviceContext.GetTextExtent( *DisplayName.ToString(), &W, &H );

	const INT YOffset = (PROP_DefaultItemHeight - H) / 2;
	RenderDeviceContext.DrawText( *DisplayName.ToString(), CurrentX+( bCanBeExpanded ? 16 : 0 ),ClientRect.y+YOffset );

	RenderDeviceContext.DestroyClippingRegion();
}

/** Reset to default button event. */
void WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::OnResetToDefault(wxCommandEvent &Event)
{
	UMaterialEditorInstanceTimeVarying* Instance = GetInstanceObject();

	if(Instance && Instance->Parent)
	{
		FName PropertyName = PropertyStructName;

		FName ScalarArrayName(TEXT("ScalarParameterValues"));
		FName TextureArrayName(TEXT("TextureParameterValues"));
		FName FontArrayName(TEXT("FontParameterValues"));
		FName VectorArrayName(TEXT("VectorParameterValues"));
		FName StaticSwitchArrayName(TEXT("StaticSwitchParameterValues"));
		FName StaticComponentMaskArrayName(TEXT("StaticComponentMaskParameterValues"));

		if(PropertyName==ScalarArrayName)
		{
			FInterpCurveInitFloat OutValues;
			if(Instance->Parent->GetScalarCurveParameterValue(DisplayName, OutValues))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->ScalarParameterValues.Num(); PropertyIdx++)
				{
					FEditorScalarParameterValueOverTime& Value = Instance->ScalarParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValueCurve = OutValues;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}

			FLOAT OutValue;
			if(Instance->Parent->GetScalarParameterValue(DisplayName, OutValue))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->ScalarParameterValues.Num(); PropertyIdx++)
				{
					FEditorScalarParameterValueOverTime& Value = Instance->ScalarParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValue = OutValue;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}
		}
		else if(PropertyName==TextureArrayName)
		{
			UTexture* OutValue;
			if(Instance->Parent->GetTextureParameterValue(DisplayName, OutValue))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->TextureParameterValues.Num(); PropertyIdx++)
				{
					FEditorTextureParameterValueOverTime& Value = Instance->TextureParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValue = OutValue;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}				
		}
		else if(PropertyName==FontArrayName)
		{
			UFont* OutFontValue;
			INT OutFontPage;
			if(Instance->Parent->GetFontParameterValue(DisplayName, OutFontValue,OutFontPage))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->FontParameterValues.Num(); PropertyIdx++)
				{
					FEditorFontParameterValueOverTime& Value = Instance->FontParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.FontValue = OutFontValue;
						Value.FontPage = OutFontPage;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}				
		}
		else if(PropertyName==VectorArrayName)
		{
			FInterpCurveInitVector OutValues;
			if(Instance->Parent->GetVectorCurveParameterValue(DisplayName, OutValues))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->VectorParameterValues.Num(); PropertyIdx++)
				{
					FEditorVectorParameterValueOverTime& Value = Instance->VectorParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValueCurve = OutValues;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}

			FLinearColor OutValue;
			if(Instance->Parent->GetVectorParameterValue(DisplayName, OutValue))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->VectorParameterValues.Num(); PropertyIdx++)
				{
					FEditorVectorParameterValueOverTime& Value = Instance->VectorParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValue = OutValue;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}				
		}
		else if(PropertyName==StaticSwitchArrayName)
		{
			UBOOL OutValue;
			FGuid TempGuid(0,0,0,0);
			if(Instance->Parent->GetStaticSwitchParameterValue(DisplayName, OutValue, TempGuid))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->StaticSwitchParameterValues.Num(); PropertyIdx++)
				{
					FEditorStaticSwitchParameterValueOverTime& Value = Instance->StaticSwitchParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValue = OutValue;
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}				
		}
		else if(PropertyName==StaticComponentMaskArrayName)
		{
			UBOOL OutValue[4];
			FGuid TempGuid(0,0,0,0);

			if(Instance->Parent->GetStaticComponentMaskParameterValue(DisplayName, OutValue[0], OutValue[1], OutValue[2], OutValue[3], TempGuid))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->StaticComponentMaskParameterValues.Num(); PropertyIdx++)
				{
					FEditorStaticComponentMaskParameterValueOverTime& Value = Instance->StaticComponentMaskParameterValues(PropertyIdx);
					if(Value.ParameterName == DisplayName)
					{
						Value.ParameterValue.R = OutValue[0];
						Value.ParameterValue.G = OutValue[1];
						Value.ParameterValue.B = OutValue[2];
						Value.ParameterValue.A = OutValue[3];
						Instance->CopyToSourceInstance();
						break;
					}
				}
			}				
		}

		// Rebuild property window to update the values.
		GetPropertyWindow()->Rebuild();
 	}
}

/**
 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
 * and (if the property window item is expandable) to toggle expansion state.
 *
 * @param	Event	the mouse click input that generated the event
 *
 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter::ClickedPropertyItem( wxMouseEvent& Event )
{
	UBOOL bShouldGainFocus = TRUE;

	// if this property is edit-const, it can't be changed
	// or if we couldn't find a valid condition property, also use the base version
	if ( Property == NULL || (Property->PropertyFlags & CPF_EditConst) != 0 )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}

	// if they clicked on the checkbox, toggle the edit condition
	else if ( ClickedCheckbox(Event.GetX(), Event.GetY()) )
	{
		
		NotifyPreChange(Property);
		bShouldGainFocus = !bCanBeExpanded;
		if ( ToggleConditionValue() == FALSE )
		{
			bShouldGainFocus = FALSE;

			// if we just disabled the condition which allows us to edit this control
			// collapse the item if this is an expandable item
			if ( bCanBeExpanded )
			{
				Collapse();
			}
		}

		if ( !bCanBeExpanded && ParentItem != NULL )
		{
			ParentItem->Refresh();
		}
		else
		{
			Refresh();
		}


		// Note the current property window so that CALLBACK_ObjectPropertyChanged
		// doesn't destroy the window out from under us.
		WxPropertyWindow* PreviousPropertyWindow = NULL;
		if ( GApp )
		{
			PreviousPropertyWindow = GApp->CurrentPropertyWindow;
			GApp->CurrentPropertyWindow = GetPropertyWindow();
		}
		NotifyPostChange(Property);

		// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
		if ( GApp )
		{
			GApp->CurrentPropertyWindow = PreviousPropertyWindow;
		}
	}
	// if the condition for editing this control has been met (i.e. the checkbox is checked), pass the event back to the base version, which will do the right thing
	// based on where the user clicked
	else if ( IsConditionMet() )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}
	else
	{
		// the condition is false, so this control isn't allowed to do anything - swallow the event.
		bShouldGainFocus = FALSE;
	}

	return bShouldGainFocus;
}


//////////////////////////////////////////////////////////////////////////
// WxPropertyWindow_MaterialInstanceTimeVaryingParameters
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_MaterialInstanceTimeVaryingParameters, WxPropertyWindow_Item);

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_MaterialInstanceTimeVaryingParameters::CreateChildItems()
{
	FName PropertyName = Property->GetFName();
	UStructProperty* StructProperty = Cast<UStructProperty>(Property,CLASS_IsAUStructProperty);
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property,CLASS_IsAUObjectProperty);

	// Copy IsSorted() flag from parent.  Default sorted to TRUE if no parent exists.
	SetSorted( ParentItem ? ParentItem->IsSorted() : 1 );

	if( Property->ArrayDim > 1 && ArrayIndex == -1 )
	{
		// Expand array.
		SetSorted( 0 );
		for( INT i = 0 ; i < Property->ArrayDim ; i++ )
		{
			WxPropertyWindow_Item* pwi = CreatePropertyItem(Property,i,this);
			pwi->Create( this, this, TopPropertyWindow, Property, i*Property->ElementSize, i, bSupportsCustomControls );
			ChildItems.AddItem(pwi);
		}
	}
	else if( ArrayProperty )
	{
		// Expand array.
		SetSorted( 0 );

		FScriptArray* Array = NULL;
		TArray<BYTE*> Addresses;
		if ( GetReadAddress( this, bSingleSelectOnly, Addresses ) )
		{
			Array = (FScriptArray*)Addresses(0);
		}

		WxPropertyWindow_Objects* ItemParent = FindObjectItemParent();
		UMaterialEditorInstanceTimeVarying* MaterialInterface = NULL;
		UMaterial* Material = NULL;


		if(ItemParent)
		{
			for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
			{
				MaterialInterface = Cast<UMaterialEditorInstanceTimeVarying>(*It);
				Material = MaterialInterface->SourceInstance->GetMaterial(MSP_SM3);
				break;
			}
		}

		if( Array && Material )
		{
			FName ParameterValueName(TEXT("ParameterValue"));
			FName ScalarArrayName(TEXT("ScalarParameterValues"));
			FName TextureArrayName(TEXT("TextureParameterValues"));
			FName FontArrayName(TEXT("FontParameterValues"));
			FName VectorArrayName(TEXT("VectorParameterValues"));
			FName StaticSwitchArrayName(TEXT("StaticSwitchParameterValues"));
			FName StaticComponentMaskArrayName(TEXT("StaticComponentMaskParameterValues"));

			// Make sure that the inner of this array is a material instance parameter struct.
			UStructProperty* StructProperty = Cast<UStructProperty>(ArrayProperty->Inner);
		
			if(StructProperty)
			{	
				// Iterate over all possible fields of this struct until we find the value field, we want to combine
				// the name and value of the parameter into one property window item.  We do this by adding a item for the value
				// and overriding the name of the item using the name from the parameter.
				for( TFieldIterator<UProperty,CASTCLASS_UProperty> It(StructProperty->Struct); It; ++It )
				{
					UProperty* StructMember = *It;
					if( GetPropertyWindow()->ShouldShowNonEditable() || (StructMember->PropertyFlags&CPF_Edit) )
					{
						// Loop through all elements of this array and add properties for each one.
						for( INT ArrayIdx = 0 ; ArrayIdx < Array->Num() ; ArrayIdx++ )
						{	
							WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter* PropertyWindowItem = wxDynamicCast(CreatePropertyItem(StructMember,INDEX_NONE,this), WxCustomPropertyItem_MaterialInstanceTimeVaryingParameter);


							if( PropertyWindowItem == NULL )
							{
								debugf( TEXT( "PropertyName was NULL: %s "), *StructMember->GetName() );
								continue;
							}

							if(StructMember->GetFName() == ParameterValueName)
							{
								// Find a name for the parameter property we are adding.
								FName OverrideName = NAME_None;
								BYTE* ElementData = ((BYTE*)Array->GetData())+ArrayIdx*ArrayProperty->Inner->ElementSize;
								FGuid ExpressionId;

								//debugf( TEXT( "PropertyName is: %s "), *PropertyName.ToString() );

								if(PropertyName==ScalarArrayName)
								{
									FEditorScalarParameterValue* Param = (FEditorScalarParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}
								else if(PropertyName==TextureArrayName)
								{
									FEditorTextureParameterValue* Param = (FEditorTextureParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}
								else if(PropertyName==FontArrayName)
								{
									FEditorFontParameterValue* Param = (FEditorFontParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}
								else if(PropertyName==VectorArrayName)
								{
									FEditorVectorParameterValue* Param = (FEditorVectorParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}
								else if(PropertyName==StaticSwitchArrayName)
								{
									FEditorStaticSwitchParameterValue* Param = (FEditorStaticSwitchParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}
								else if(PropertyName==StaticComponentMaskArrayName)
								{
									FEditorStaticComponentMaskParameterValue* Param = (FEditorStaticComponentMaskParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
									ExpressionId = Param->ExpressionId;
								}

								WxMaterialInstanceTimeVaryingEditor *Win = wxDynamicCast(this->GetPropertyWindow()->GetGrandParent()->GetParent(), WxMaterialInstanceTimeVaryingEditor);
								check(Win);
								
								if(Win->ToolBar->GetToolState(ID_MATERIALINSTANCE_CONSTANT_EDITOR_SHOWALLPARAMETERS) || MaterialInterface->VisibleExpressions.ContainsItem(ExpressionId))
								{
									// Add the property.
									PropertyWindowItem->PropertyStructName = PropertyName;
									PropertyWindowItem->DisplayName = OverrideName;
									PropertyWindowItem->Create( this, this, TopPropertyWindow, StructMember, ArrayIdx*ArrayProperty->Inner->ElementSize+StructMember->Offset, 
										INDEX_NONE, bSupportsCustomControls );

									ChildItems.AddItem(PropertyWindowItem);
								}
							}
						}
					}
				}
			}
		}
	}

	SortChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//	WxMaterialInstanceTimeVaryingEditor
//
//////////////////////////////////////////////////////////////////////////

/**
 * wxWidgets Event Table
 */
BEGIN_EVENT_TABLE(WxMaterialInstanceTimeVaryingEditor, WxMaterialEditorBase)
	EVT_MENU(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_SYNCTOGB, OnMenuSyncToGB)
	EVT_MENU(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_OPENEDITOR, OnMenuOpenEditor)
	EVT_LIST_ITEM_ACTIVATED(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_LIST, OnInheritanceListDoubleClick)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_LIST, OnInheritanceListRightClick)
	EVT_TOOL(ID_MATERIALINSTANCE_CONSTANT_EDITOR_SHOWALLPARAMETERS, OnShowAllMaterialParameters)
END_EVENT_TABLE()


WxMaterialInstanceTimeVaryingEditor::WxMaterialInstanceTimeVaryingEditor( wxWindow* Parent, wxWindowID id, UMaterialInterface* InMaterialInterface ) :	
        WxMaterialEditorBase( Parent, id, InMaterialInterface ),   
		FDockingParent(this)
{
	// Set the static mesh editor window title to include the static mesh being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("MaterialInstanceEditorCaption_F"), *InMaterialInterface->GetPathName()) ) );

	// Construct a temp holder for our instance parameters.
	UMaterialInstanceTimeVarying* InstanceConstant = Cast<UMaterialInstanceTimeVarying>(InMaterialInterface);
	MaterialEditorInstance = ConstructObject<UMaterialEditorInstanceTimeVarying>(UMaterialEditorInstanceTimeVarying::StaticClass());
	MaterialEditorInstance->SetSourceInstance(InstanceConstant);
	
	// Create toolbar
	ToolBar = new WxMaterialInstanceTimeVaryingEditorToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );
	PropertyWindow->SetCustomControlSupport( TRUE );
	PropertyWindow->SetObject( MaterialEditorInstance, 1,1,0 );

	SetSize(1024,768);
	FWindowUtil::LoadPosSize( TEXT("MaterialInstanceEditor"), this, 64,64,800,450 );

	// Add inheritance list.
	InheritanceList = new WxListView(this, ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
	RebuildInheritanceList();

	// Add docking windows.
	{
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd(TEXT("PropertiesCaption_F")), *MaterialInterface->GetPathName())), *LocalizeUnrealEd(TEXT("Properties")));
		AddDockingWindow(InheritanceList, FDockingParent::DH_Left, *FString::Printf(LocalizeSecure(LocalizeUnrealEd(TEXT("MaterialInstanceParent_F")), *MaterialInterface->GetPathName())), *LocalizeUnrealEd(TEXT("MaterialInstanceParent")));

		SetDockHostSize(FDockingParent::DH_Left, 300);

		AddDockingWindow( ( wxWindow* )PreviewWin, FDockingParent::DH_None, NULL );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}

	// Add docking menu
	wxMenuBar* MenuBar = new wxMenuBar();
	AppendWindowMenu(MenuBar);
	SetMenuBar(MenuBar);

	// Load editor settings.
	LoadSettings();

	// Set the preview mesh for the material.  This call must occur after the toolbar is initialized.
	if ( !SetPreviewMesh( *InMaterialInterface->PreviewMesh ) )
	{
		// The material preview mesh couldn't be found or isn't loaded.  Default to our primitive type.
		SetPrimitivePreview();
	}
}

WxMaterialInstanceTimeVaryingEditor::~WxMaterialInstanceTimeVaryingEditor()
{
	SaveSettings();

	PropertyWindow->SetObject( NULL, 0,0,0 );
	delete PropertyWindow;
}

void WxMaterialInstanceTimeVaryingEditor::Serialize(FArchive& Ar)
{
	WxMaterialEditorBase::Serialize(Ar);

	// Serialize our custom object instance
	Ar<<MaterialEditorInstance;

	// Serialize all parent material instances that are stored in the list.
	Ar<<ParentList;
}

/** Pre edit change notify for properties. */
void WxMaterialInstanceTimeVaryingEditor::NotifyPreChange(void* Src, UProperty* PropertyThatChanged)
{
	// If they changed the parent, kill the current object in the property window since we will need to remake it.
	if(PropertyThatChanged->GetName()==TEXT("Parent"))
	{
		// Collapse all of the property arrays, since they will be changing with the new parent.
		PropertyWindow->CollapseItem(TEXT("VectorParameterValues"));
		PropertyWindow->CollapseItem(TEXT("ScalarParameterValues"));
		PropertyWindow->CollapseItem(TEXT("TextureParameterValues"));
		PropertyWindow->CollapseItem(TEXT("FontParameterValues"));
	}
}

/** Post edit change notify for properties. */
void WxMaterialInstanceTimeVaryingEditor::NotifyPostChange(void* Src, UProperty* PropertyThatChanged)
{
	// Update the preview window when the user changes a property.
	RefreshPreviewViewport();

	// If they changed the parent, regenerate the parent list.
	if(PropertyThatChanged->GetName()==TEXT("Parent"))
	{
		UBOOL bSetEmptyParent = FALSE;

		// Check to make sure they didnt set the parent to themselves.
		if(MaterialEditorInstance->Parent==MaterialInterface)
		{
			bSetEmptyParent = TRUE;
		}

		//don't allow setting a fallback material as the parent
		if (MaterialEditorInstance->Parent && MaterialEditorInstance->Parent->IsFallbackMaterial())
		{
			const FString ErrorMsg = FString::Printf(LocalizeSecure(LocalizeUnrealEd("Error_MaterialInstanceEditorSetFallbackMaterialParent"), *MaterialEditorInstance->Parent->GetName()));
			appMsgf(AMT_OK, *ErrorMsg);
			bSetEmptyParent = TRUE;
		}

		if (bSetEmptyParent)
		{
			MaterialEditorInstance->Parent = NULL;

			UMaterialInstanceTimeVarying* MITV = Cast<UMaterialInstanceTimeVarying>(MaterialInterface);
			if(MITV)
			{
				MITV->SetParent(NULL);
			}
		}

		RebuildInheritanceList();
	}

	//rebuild the property window to account for the possibility that the item changed was
	//a static switch
	if(PropertyThatChanged->Category == FName(TEXT("EditorStaticSwitchParameterValueOverTime")))
	{
		TArray<FGuid> PreviousExpressions(MaterialEditorInstance->VisibleExpressions);
		MaterialEditorInstance->VisibleExpressions.Empty();
		WxMaterialEditor::GetVisibleMaterialParameters(MaterialEditorInstance->Parent->GetMaterial(MSP_SM3), MaterialEditorInstance->SourceInstance, MaterialEditorInstance->VisibleExpressions);
		
		//check to see if it was the override button that was clicked or the value of the static switch
		//by comparing the values of the previous and current visible expression lists
		UBOOL bHasChanged = PreviousExpressions.Num() != MaterialEditorInstance->VisibleExpressions.Num();

		if(!bHasChanged)
		{
			for(INT Index = 0; Index < PreviousExpressions.Num(); ++Index)
			{
				if(PreviousExpressions(Index) != MaterialEditorInstance->VisibleExpressions(Index))
				{
					bHasChanged = TRUE;
					break;
				}
			}
		}

		if(bHasChanged)
		{
			PropertyWindow->PostRebuild();
		}
	}
}

/** Rebuilds the inheritance list for this material instance. */
void WxMaterialInstanceTimeVaryingEditor::RebuildInheritanceList()
{
	InheritanceList->DeleteAllColumns();
	InheritanceList->DeleteAllItems();
	ParentList.Empty();
	
	InheritanceList->Freeze();
	{
		InheritanceList->InsertColumn(0, *LocalizeUnrealEd("Parent"));
		InheritanceList->InsertColumn(1, *LocalizeUnrealEd("Name"));

		// Travel up the parent chain for this material instance until we reach the root material.
		UMaterialInstance* InstanceConstant = Cast<UMaterialInstance>(MaterialInterface);

		if(InstanceConstant)
		{
			long CurrentIdx = InheritanceList->InsertItem(0, *InstanceConstant->GetName());
			wxFont CurrentFont = InheritanceList->GetItemFont(CurrentIdx);
			CurrentFont.SetWeight(wxFONTWEIGHT_BOLD);
			InheritanceList->SetItemFont(CurrentIdx, CurrentFont);
			InheritanceList->SetItem(CurrentIdx, 1, *InstanceConstant->GetName());

			ParentList.AddItem(InstanceConstant);

			// Add all parents
			UMaterialInterface* Parent = InstanceConstant->Parent;
			while(Parent && Parent != InstanceConstant)
			{
				long ItemIdx = InheritanceList->InsertItem(0, *(Parent->GetName()));
				InheritanceList->SetItem(ItemIdx, 1, *Parent->GetName());

				ParentList.InsertItem(Parent,0);

				// If the parent is a material then break.
				InstanceConstant = Cast<UMaterialInstance>(Parent);

				if(InstanceConstant)
				{
					Parent = InstanceConstant->Parent;
				}
				else
				{
					break;
				}
			}

			// Loop through all the items and set their first column.
			INT NumItems = InheritanceList->GetItemCount();
			for(INT ItemIdx=0; ItemIdx<NumItems; ItemIdx++)
			{
				if(ItemIdx==0)
				{
					InheritanceList->SetItem(ItemIdx, 0, *LocalizeUnrealEd("Material"));
				}
				else
				{
					if(ItemIdx < NumItems - 1)
					{
						InheritanceList->SetItem(ItemIdx,0,
							*FString::Printf(TEXT("%s %i"), *LocalizeUnrealEd("Parent"), NumItems-1-ItemIdx));
					}
					else
					{
						InheritanceList->SetItem(ItemIdx, 0, *LocalizeUnrealEd("Current"));
					}
				}
			}
		}

		// Autosize columns
		InheritanceList->SetColumnWidth(0, wxLIST_AUTOSIZE);
		InheritanceList->SetColumnWidth(1, wxLIST_AUTOSIZE);
	}
	InheritanceList->Thaw();
}

/** Saves editor settings. */
void WxMaterialInstanceTimeVaryingEditor::SaveSettings()
{
	FWindowUtil::SavePosSize( TEXT("MaterialInstanceEditor"), this );

	SaveDockingLayout();

	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bShowGrid"), bShowGrid, GEditorUserSettingsIni);
	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bDrawGrid"), PreviewVC->IsRealtime(), GEditorUserSettingsIni);
	GConfig->SetInt(TEXT("MaterialInstanceEditor"), TEXT("PrimType"), PreviewPrimType, GEditorUserSettingsIni);
}

/** Loads editor settings. */
void WxMaterialInstanceTimeVaryingEditor::LoadSettings()
{
	UBOOL bRealtime=FALSE;

	GConfig->GetBool(TEXT("MaterialInstanceEditor"), TEXT("bShowGrid"), bShowGrid, GEditorUserSettingsIni);
	GConfig->GetBool(TEXT("MaterialInstanceEditor"), TEXT("bDrawGrid"), bRealtime, GEditorUserSettingsIni);

	INT PrimType;
	if(GConfig->GetInt(TEXT("MaterialInstanceEditor"), TEXT("PrimType"), PrimType, GEditorUserSettingsIni))
	{
		PreviewPrimType = (EThumbnailPrimType)PrimType;
	}
	else
	{
		PreviewPrimType = TPT_Sphere;
	}

	if(PreviewVC)
	{
		PreviewVC->SetShowGrid(bShowGrid);
		PreviewVC->SetRealtime(bRealtime);
	}
}

/** Syncs the GB to the selected parent in the inheritance list. */
void WxMaterialInstanceTimeVaryingEditor::SyncSelectedParentToGB()
{
	INT SelectedItem = (INT)InheritanceList->GetFirstSelected();
	if(ParentList.IsValidIndex(SelectedItem))
	{
		UMaterialInterface* SelectedMaterialInstance = ParentList(SelectedItem);
		TArray<UObject*> Objects;

		Objects.AddItem(SelectedMaterialInstance);

		// Show the GB
		WxGenericBrowser* GenericBrowser = GUnrealEd->GetBrowser<WxGenericBrowser>( TEXT("GenericBrowser") );
		if ( GenericBrowser )
		{
			GUnrealEd->GetSelectedObjects()->DeselectAll();
			GUnrealEd->GetBrowserManager()->ShowWindow(GenericBrowser->GetDockID(),TRUE);
			GenericBrowser->SyncToObjects(Objects);
		}
	}
}

/** Opens the editor for the selected parent. */
void WxMaterialInstanceTimeVaryingEditor::OpenSelectedParentEditor()
{
	INT SelectedItem = (INT)InheritanceList->GetFirstSelected();
	if(ParentList.IsValidIndex(SelectedItem))
	{
		UMaterialInterface* SelectedMaterialInstance = ParentList(SelectedItem);

		// See if its a material or material instance constant.  Don't do anything if the user chose the current material instance.
		if(MaterialInterface!=SelectedMaterialInstance)
		{
			if(SelectedMaterialInstance->IsA(UMaterial::StaticClass()))
			{
				// Show material editor
				UMaterial* Material = Cast<UMaterial>(SelectedMaterialInstance);
				wxFrame* MaterialEditor = new WxMaterialEditor( (wxWindow*)GApp->EditorFrame,-1,Material );
				MaterialEditor->SetSize(1024,768);
				MaterialEditor->Show();
			}
			else if(SelectedMaterialInstance->IsA(UMaterialInstanceTimeVarying::StaticClass()))
			{
				// Show material instance editor
				UMaterialInstanceTimeVarying* MaterialInstanceTimeVarying = Cast<UMaterialInstanceTimeVarying>(SelectedMaterialInstance);
				wxFrame* MaterialInstanceEditor = new WxMaterialInstanceTimeVaryingEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInstanceTimeVarying );
				MaterialInstanceEditor->SetSize(1024,768);
				MaterialInstanceEditor->Show();
			}
		}
	}
}

/** Event handler for when the user wants to sync the GB to the currently selected parent. */
void WxMaterialInstanceTimeVaryingEditor::OnMenuSyncToGB(wxCommandEvent &Event)
{
	SyncSelectedParentToGB();
}

/** Event handler for when the user wants to open the editor for the selected parent material. */
void WxMaterialInstanceTimeVaryingEditor::OnMenuOpenEditor(wxCommandEvent &Event)
{
	OpenSelectedParentEditor();
}

/** Double click handler for the inheritance list. */
void WxMaterialInstanceTimeVaryingEditor::OnInheritanceListDoubleClick(wxListEvent &ListEvent)
{
	OpenSelectedParentEditor();
}

/** Event handler for when the user right clicks on the inheritance list. */
void WxMaterialInstanceTimeVaryingEditor::OnInheritanceListRightClick(wxListEvent &ListEvent)
{
	INT SelectedItem = (INT)InheritanceList->GetFirstSelected();
	if(ParentList.IsValidIndex(SelectedItem))
	{
		UMaterialInterface* SelectedMaterialInstance = ParentList(SelectedItem);

		wxMenu* ContextMenu = new wxMenu();

		if(SelectedMaterialInstance != MaterialInterface)
		{
			FString Label;

			if(SelectedMaterialInstance->IsA(UMaterial::StaticClass()))
			{
				Label = LocalizeUnrealEd("MaterialEditor");
			}
			else
			{
				Label = LocalizeUnrealEd("MaterialInstanceEditor");
			}

			ContextMenu->Append(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_OPENEDITOR, *Label);
		}

		ContextMenu->Append(ID_MATERIALINSTANCE_TIME_VARYING_EDITOR_SYNCTOGB, *LocalizeUnrealEd("SyncGenericBrowser"));

		PopupMenu(ContextMenu);
	}
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxMaterialInstanceTimeVaryingEditor::GetDockingParentName() const
{
	return TEXT("MaterialInstanceEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxMaterialInstanceTimeVaryingEditor::GetDockingParentVersion() const
{
	return 1;
}

/** Event handler for when the user wants to toggle showing all material parameters. */
void WxMaterialInstanceTimeVaryingEditor::OnShowAllMaterialParameters(wxCommandEvent &Event)
{
	PropertyWindow->PostRebuild();
}
