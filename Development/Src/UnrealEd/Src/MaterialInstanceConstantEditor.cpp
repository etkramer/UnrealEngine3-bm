/**
 * MaterialInstanceConstantEditor.cpp: Material instance editor class.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "UnrealEd.h"
#include "EngineMaterialClasses.h"
#include "Properties.h"
#include "GenericBrowser.h"
#include "MaterialEditorBase.h"
#include "MaterialEditorToolBar.h"
#include "MaterialInstanceConstantEditor.h"
#include "MaterialEditorPreviewScene.h"
#include "NewMaterialEditor.h"
#include "PropertyWindowManager.h"	// required for access to GPropertyWindowManager


//////////////////////////////////////////////////////////////////////////
// UMaterialEditorInstanceConstant
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UMaterialEditorInstanceConstant);

void UMaterialEditorInstanceConstant::PostEditChange(UProperty* Property)
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
void UMaterialEditorInstanceConstant::RegenerateArrays()
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
		// Only operate on base materials, fallback material parameters are required to be a subset of the main material.
		UMaterial* ParentMaterial = Parent->GetMaterial(MSP_SM3);
		SourceInstance->UpdateParameterNames();	// Update any parameter names that may have changed.

		// Loop through all types of parameters for this material and add them to the parameter arrays.
		TArray<FName> ParameterNames;
		TArray<FGuid> Guids;

		// Vector Parameters.
		ParentMaterial->GetAllVectorParameterNames(ParameterNames, Guids);
		VectorParameterValues.AddZeroed(ParameterNames.Num());

		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorVectorParameterValue& ParameterValue = VectorParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FLinearColor Value;
			
			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetVectorParameterValue(ParameterName, Value))
			{
				ParameterValue.ParameterValue = Value;
			}

			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->VectorParameterValues.Num(); ParameterIdx++)
			{
				FVectorParameterValue& SourceParam = SourceInstance->VectorParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}

		// Scalar Parameters.
		ParentMaterial->GetAllScalarParameterNames(ParameterNames, Guids);
		ScalarParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorScalarParameterValue& ParameterValue = ScalarParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FLOAT Value;
			
			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ExpressionId = Guids(ParameterIdx);

			if(SourceInstance->GetScalarParameterValue(ParameterName, Value))
			{
				ParameterValue.ParameterValue = Value;
			}


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->ScalarParameterValues.Num(); ParameterIdx++)
			{
				FScalarParameterValue& SourceParam = SourceInstance->ScalarParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}

		// Texture Parameters.
		ParentMaterial->GetAllTextureParameterNames(ParameterNames, Guids);
		TextureParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorTextureParameterValue& ParameterValue = TextureParameterValues(ParameterIdx);
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
				FTextureParameterValue& SourceParam = SourceInstance->TextureParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}

		// Font Parameters.
		ParentMaterial->GetAllFontParameterNames(ParameterNames, Guids);
		FontParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorFontParameterValue& ParameterValue = FontParameterValues(ParameterIdx);
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
				FFontParameterValue& SourceParam = SourceInstance->FontParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.FontValue = SourceParam.FontValue;
					ParameterValue.FontPage = SourceParam.FontPage;
				}
			}
		}

		// Get all static parameters from the source instance.  This will handle inheriting parent values.
		FStaticParameterSet SourceStaticParameters;
		SourceInstance->GetStaticParameterValues(&SourceStaticParameters);

		// Copy Static Switch Parameters
		StaticSwitchParameterValues.AddZeroed(SourceStaticParameters.StaticSwitchParameters.Num());
		for(INT ParameterIdx=0; ParameterIdx<SourceStaticParameters.StaticSwitchParameters.Num(); ParameterIdx++)
		{
			StaticSwitchParameterValues(ParameterIdx) = FEditorStaticSwitchParameterValue(SourceStaticParameters.StaticSwitchParameters(ParameterIdx));
		}

		// Copy Static Component Mask Parameters
		StaticComponentMaskParameterValues.AddZeroed(SourceStaticParameters.StaticComponentMaskParameters.Num());
		for(INT ParameterIdx=0; ParameterIdx<SourceStaticParameters.StaticComponentMaskParameters.Num(); ParameterIdx++)
		{
			StaticComponentMaskParameterValues(ParameterIdx) = FEditorStaticComponentMaskParameterValue(SourceStaticParameters.StaticComponentMaskParameters(ParameterIdx));
		}

		WxMaterialEditor::GetVisibleMaterialParameters(ParentMaterial, SourceInstance, VisibleExpressions);
	}
}

/** Copies the parameter array values back to the source instance. */
void UMaterialEditorInstanceConstant::CopyToSourceInstance()
{
	if(SourceInstance->IsTemplate(RF_ClassDefaultObject) == FALSE )
	{
		SourceInstance->MarkPackageDirty();

		SourceInstance->VectorParameterValues.Empty();
		SourceInstance->ScalarParameterValues.Empty();
		SourceInstance->TextureParameterValues.Empty();
		SourceInstance->FontParameterValues.Empty();

		// Scalar Parameters
		for(INT ParameterIdx=0; ParameterIdx<ScalarParameterValues.Num(); ParameterIdx++)
		{
			FEditorScalarParameterValue& ParameterValue = ScalarParameterValues(ParameterIdx);

			if(ParameterValue.bOverride)
			{
				SourceInstance->SetScalarParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
			}
		}

		// Texture Parameters
		for(INT ParameterIdx=0; ParameterIdx<TextureParameterValues.Num(); ParameterIdx++)
		{
			FEditorTextureParameterValue& ParameterValue = TextureParameterValues(ParameterIdx);

			if(ParameterValue.bOverride)
			{
				SourceInstance->SetTextureParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
			}
		}

		// Font Parameters
		for(INT ParameterIdx=0; ParameterIdx<FontParameterValues.Num(); ParameterIdx++)
		{
			FEditorFontParameterValue& ParameterValue = FontParameterValues(ParameterIdx);

			if(ParameterValue.bOverride)
			{
				SourceInstance->SetFontParameterValue(ParameterValue.ParameterName,ParameterValue.FontValue,ParameterValue.FontPage);
			}
		}

		// Vector Parameters
		for(INT ParameterIdx=0; ParameterIdx<VectorParameterValues.Num(); ParameterIdx++)
		{
			FEditorVectorParameterValue& ParameterValue = VectorParameterValues(ParameterIdx);

			if(ParameterValue.bOverride)
			{
				SourceInstance->SetVectorParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
			}
		}

		CopyStaticParametersToSourceInstance();

		// Copy phys material back to source instance
		SourceInstance->PhysMaterial = PhysMaterial;

		// Update object references and parameter names.
		SourceInstance->UpdateParameterNames();
	}
}

/** Copies static parameters to the source instance, which will be marked dirty if a compile was necessary */
void UMaterialEditorInstanceConstant::CopyStaticParametersToSourceInstance()
{
	if(SourceInstance->IsTemplate(RF_ClassDefaultObject) == FALSE )
	{
		//build a static parameter set containing all static parameter settings
		FStaticParameterSet StaticParameters;

		// Static Switch Parameters
		for(INT ParameterIdx = 0; ParameterIdx < StaticSwitchParameterValues.Num(); ParameterIdx++)
		{
			FEditorStaticSwitchParameterValue& EditorParameter = StaticSwitchParameterValues(ParameterIdx);
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
			FEditorStaticComponentMaskParameterValue& EditorParameter = StaticComponentMaskParameterValues(ParameterIdx);
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
}


/**  
 * Sets the source instance for this object and regenerates arrays. 
 *
 * @param MaterialInterface		Instance to use as the source for this material editor instance.
 */
void UMaterialEditorInstanceConstant::SetSourceInstance(UMaterialInstanceConstant* MaterialInterface)
{
	check(MaterialInterface);
	SourceInstance = MaterialInterface;
	Parent = SourceInstance->Parent;
	PhysMaterial = SourceInstance->PhysMaterial;

	RegenerateArrays();

	//propagate changes to the base material so the instance will be updated if it has a static permutation resource
	CopyStaticParametersToSourceInstance();
	SourceInstance->UpdateStaticPermutation();
}


//////////////////////////////////////////////////////////////////////////
// WxCustomPropertyItem_MaterialInstanceConstantParameter
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxCustomPropertyItem_MaterialInstanceConstantParameter, WxCustomPropertyItem_ConditionalItem);

BEGIN_EVENT_TABLE(WxCustomPropertyItem_MaterialInstanceConstantParameter, WxCustomPropertyItem_ConditionalItem)
	EVT_BUTTON(ID_MATERIALINSTANCE_CONSTANT_EDITOR_RESETTODEFAULT, OnResetToDefault)
END_EVENT_TABLE()

WxCustomPropertyItem_MaterialInstanceConstantParameter::WxCustomPropertyItem_MaterialInstanceConstantParameter() : 
WxCustomPropertyItem_ConditionalItem()
{
	ResetToDefault = NULL;
	bAllowEditing = FALSE;
}

/**
 * Initialize this property window.  Must be the first function called after creating the window.
 */
void WxCustomPropertyItem_MaterialInstanceConstantParameter::Create(	wxWindow* InParent,
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
		ResetToDefault = new WxPropertyItemButton( this, ID_MATERIALINSTANCE_CONSTANT_EDITOR_RESETTODEFAULT, GPropertyWindowManager->Prop_ResetToDefaultB );
		IndentX += 15;

		// Generate tooltip text for this button.
		UMaterialEditorInstanceConstant* Instance = GetInstanceObject();

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
UBOOL WxCustomPropertyItem_MaterialInstanceConstantParameter::ToggleConditionValue()
{	
	UMaterialEditorInstanceConstant* Instance = GetInstanceObject();

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
				FEditorScalarParameterValue& Param = Instance->ScalarParameterValues(ParamIdx);

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
				FEditorTextureParameterValue& Param = Instance->TextureParameterValues(ParamIdx);

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
				FEditorFontParameterValue& Param = Instance->FontParameterValues(ParamIdx);

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
				FEditorVectorParameterValue& Param = Instance->VectorParameterValues(ParamIdx);

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
				FEditorStaticSwitchParameterValue& Param = Instance->StaticSwitchParameterValues(ParamIdx);

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
				FEditorStaticComponentMaskParameterValue& Param = Instance->StaticComponentMaskParameterValues(ParamIdx);

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
UBOOL WxCustomPropertyItem_MaterialInstanceConstantParameter::IsOverridden()
{
	UMaterialEditorInstanceConstant* Instance = GetInstanceObject();

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
				FEditorScalarParameterValue& Param = Instance->ScalarParameterValues(ParamIdx);

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
				FEditorTextureParameterValue& Param = Instance->TextureParameterValues(ParamIdx);

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
				FEditorFontParameterValue& Param = Instance->FontParameterValues(ParamIdx);

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
				FEditorVectorParameterValue& Param = Instance->VectorParameterValues(ParamIdx);

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
				FEditorStaticSwitchParameterValue& Param = Instance->StaticSwitchParameterValues(ParamIdx);

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
				FEditorStaticComponentMaskParameterValue& Param = Instance->StaticComponentMaskParameterValues(ParamIdx);

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
UBOOL WxCustomPropertyItem_MaterialInstanceConstantParameter::IsConditionMet()
{
	return TRUE;
}

/** @return Returns the instance object this property is associated with. */
UMaterialEditorInstanceConstant* WxCustomPropertyItem_MaterialInstanceConstantParameter::GetInstanceObject()
{
	WxPropertyWindow_Objects* ItemParent = FindObjectItemParent();
	UMaterialEditorInstanceConstant* MaterialInterface = NULL;

	if(ItemParent)
	{
		for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
		{
			MaterialInterface = Cast<UMaterialEditorInstanceConstant>(*It);
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
void WxCustomPropertyItem_MaterialInstanceConstantParameter::RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect )
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
void WxCustomPropertyItem_MaterialInstanceConstantParameter::OnResetToDefault(wxCommandEvent &Event)
{
	UMaterialEditorInstanceConstant* Instance = GetInstanceObject();

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
			FLOAT OutValue;
			if(Instance->Parent->GetScalarParameterValue(DisplayName, OutValue))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->ScalarParameterValues.Num(); PropertyIdx++)
				{
					FEditorScalarParameterValue& Value = Instance->ScalarParameterValues(PropertyIdx);
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
					FEditorTextureParameterValue& Value = Instance->TextureParameterValues(PropertyIdx);
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
					FEditorFontParameterValue& Value = Instance->FontParameterValues(PropertyIdx);
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
			FLinearColor OutValue;
			if(Instance->Parent->GetVectorParameterValue(DisplayName, OutValue))
			{
				for(INT PropertyIdx=0; PropertyIdx<Instance->VectorParameterValues.Num(); PropertyIdx++)
				{
					FEditorVectorParameterValue& Value = Instance->VectorParameterValues(PropertyIdx);
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
					FEditorStaticSwitchParameterValue& Value = Instance->StaticSwitchParameterValues(PropertyIdx);
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
					FEditorStaticComponentMaskParameterValue& Value = Instance->StaticComponentMaskParameterValues(PropertyIdx);
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
UBOOL WxCustomPropertyItem_MaterialInstanceConstantParameter::ClickedPropertyItem( wxMouseEvent& Event )
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
// WxPropertyWindow_MaterialInstanceConstantParameters
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_MaterialInstanceConstantParameters, WxPropertyWindow_Item);

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_MaterialInstanceConstantParameters::CreateChildItems()
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
		UMaterialEditorInstanceConstant* MaterialInterface = NULL;
		UMaterial* Material = NULL;


		if(ItemParent)
		{
			for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
			{
				MaterialInterface = Cast<UMaterialEditorInstanceConstant>(*It);
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
							WxCustomPropertyItem_MaterialInstanceConstantParameter* PropertyWindowItem = wxDynamicCast(CreatePropertyItem(StructMember,INDEX_NONE,this), WxCustomPropertyItem_MaterialInstanceConstantParameter);


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

								WxMaterialInstanceConstantEditor *Win = wxDynamicCast(this->GetPropertyWindow()->GetParent(), WxMaterialInstanceConstantEditor);
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
//	WxMaterialInstanceConstantEditor
//
//////////////////////////////////////////////////////////////////////////

/**
 * wxWidgets Event Table
 */
BEGIN_EVENT_TABLE(WxMaterialInstanceConstantEditor, WxMaterialEditorBase)
	EVT_MENU(ID_MATERIALINSTANCE_CONSTANT_EDITOR_SYNCTOGB, OnMenuSyncToGB)
	EVT_MENU(ID_MATERIALINSTANCE_CONSTANT_EDITOR_OPENEDITOR, OnMenuOpenEditor)
	EVT_LIST_ITEM_ACTIVATED(ID_MATERIALINSTANCE_CONSTANT_EDITOR_LIST, OnInheritanceListDoubleClick)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_MATERIALINSTANCE_CONSTANT_EDITOR_LIST, OnInheritanceListRightClick)
	EVT_TOOL(ID_MATERIALINSTANCE_CONSTANT_EDITOR_SHOWALLPARAMETERS, OnShowAllMaterialParameters)
END_EVENT_TABLE()

WxMaterialInstanceConstantEditor::WxMaterialInstanceConstantEditor( wxWindow* Parent, wxWindowID id, UMaterialInterface* InMaterialInterface ) :	
        WxMaterialEditorBase( Parent, id, InMaterialInterface ),   
		FDockingParent(this)
{
	// Set the static mesh editor window title to include the static mesh being edited.
	SetTitle( *FString::Printf( LocalizeSecure(LocalizeUnrealEd("MaterialInstanceEditorCaption_F"), *InMaterialInterface->GetPathName()) ) );

	// Construct a temp holder for our instance parameters.
	UMaterialInstanceConstant* InstanceConstant = Cast<UMaterialInstanceConstant>(InMaterialInterface);
	MaterialEditorInstance = ConstructObject<UMaterialEditorInstanceConstant>(UMaterialEditorInstanceConstant::StaticClass());
	MaterialEditorInstance->SetSourceInstance(InstanceConstant);
	
	// Create toolbar
	ToolBar = new WxMaterialInstanceConstantEditorToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );
	PropertyWindow->SetCustomControlSupport( TRUE );
	PropertyWindow->SetObject( MaterialEditorInstance, 1,1,0 );

	SetSize(1024,768);
	FWindowUtil::LoadPosSize( TEXT("MaterialInstanceEditor"), this, 64,64,800,450 );


	// Add inheritance list.
	InheritanceList = new WxListView(this, ID_MATERIALINSTANCE_CONSTANT_EDITOR_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
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

WxMaterialInstanceConstantEditor::~WxMaterialInstanceConstantEditor()
{
	SaveSettings();

	PropertyWindow->SetObject( NULL, 0,0,0 );
	delete PropertyWindow;
}

void WxMaterialInstanceConstantEditor::Serialize(FArchive& Ar)
{
	WxMaterialEditorBase::Serialize(Ar);

	// Serialize our custom object instance
	Ar<<MaterialEditorInstance;

	// Serialize all parent material instances that are stored in the list.
	Ar<<ParentList;
}

/** Pre edit change notify for properties. */
void WxMaterialInstanceConstantEditor::NotifyPreChange(void* Src, UProperty* PropertyThatChanged)
{
	// If they changed the parent, kill the current object in the property window since we will need to remake it.
	if(PropertyThatChanged->GetName()==TEXT("Parent"))
	{
		// Collapse all of the property arrays, since they will be changing with the new parent.
		PropertyWindow->CollapseItem(TEXT("VectorParameterValues"));
		PropertyWindow->CollapseItem(TEXT("ScalarParameterValues"));
		PropertyWindow->CollapseItem(TEXT("TextureParameterValues"));
		PropertyWindow->CollapseItem(TEXT("FontParameterValues"));
		PropertyWindow->CollapseItem(TEXT("StaticSwitchParameterValues"));
		PropertyWindow->CollapseItem(TEXT("StaticComponentMaskParameterValues"));
	}
}

/** Post edit change notify for properties. */
void WxMaterialInstanceConstantEditor::NotifyPostChange(void* Src, UProperty* PropertyThatChanged)
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

			UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(MaterialInterface);
			if(MIC)
			{
				MIC->SetParent(NULL);
			}
		}

		RebuildInheritanceList();
	}

	//rebuild the property window to account for the possibility that the item changed was
	//a static switch
	if(PropertyThatChanged->Category == FName(TEXT("EditorStaticSwitchParameterValue")))
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
void WxMaterialInstanceConstantEditor::RebuildInheritanceList()
{
	InheritanceList->DeleteAllColumns();
	InheritanceList->DeleteAllItems();
	ParentList.Empty();
	
	InheritanceList->Freeze();
	{
		InheritanceList->InsertColumn(0, *LocalizeUnrealEd("Parent"));
		InheritanceList->InsertColumn(1, *LocalizeUnrealEd("Name"));

		// Travel up the parent chain for this material instance until we reach the root material.
		UMaterialInstanceConstant* InstanceConstant = Cast<UMaterialInstanceConstant>(MaterialInterface);

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
				InstanceConstant = Cast<UMaterialInstanceConstant>(Parent);

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

/**
 * Draws messages on the canvas.
 */
void WxMaterialInstanceConstantEditor::DrawMessages(FViewport* Viewport,FCanvas* Canvas)
{
	Canvas->PushAbsoluteTransform(FMatrix::Identity);
	if (MaterialEditorInstance->Parent)
	{
		const FMaterialResource* MaterialResource = MaterialEditorInstance->SourceInstance->GetMaterialResource();
		INT DrawPositionY = 5;
		DrawMaterialInfoStrings(Canvas, MaterialResource, DrawPositionY);
	}
	Canvas->PopTransform();
}

/** Saves editor settings. */
void WxMaterialInstanceConstantEditor::SaveSettings()
{
	FWindowUtil::SavePosSize( TEXT("MaterialInstanceEditor"), this );

	SaveDockingLayout();

	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bShowGrid"), bShowGrid, GEditorUserSettingsIni);
	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bDrawGrid"), PreviewVC->IsRealtime(), GEditorUserSettingsIni);
	GConfig->SetInt(TEXT("MaterialInstanceEditor"), TEXT("PrimType"), PreviewPrimType, GEditorUserSettingsIni);
}

/** Loads editor settings. */
void WxMaterialInstanceConstantEditor::LoadSettings()
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
void WxMaterialInstanceConstantEditor::SyncSelectedParentToGB()
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
void WxMaterialInstanceConstantEditor::OpenSelectedParentEditor()
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
			else if(SelectedMaterialInstance->IsA(UMaterialInstanceConstant::StaticClass()))
			{
				// Show material instance editor
				UMaterialInstanceConstant* MaterialInstanceConstant = Cast<UMaterialInstanceConstant>(SelectedMaterialInstance);
				wxFrame* MaterialInstanceEditor = new WxMaterialInstanceConstantEditor( (wxWindow*)GApp->EditorFrame,-1, MaterialInstanceConstant );
				MaterialInstanceEditor->SetSize(1024,768);
				MaterialInstanceEditor->Show();
			}
		}
	}
}

/** Event handler for when the user wants to sync the GB to the currently selected parent. */
void WxMaterialInstanceConstantEditor::OnMenuSyncToGB(wxCommandEvent &Event)
{
	SyncSelectedParentToGB();
}

/** Event handler for when the user wants to open the editor for the selected parent material. */
void WxMaterialInstanceConstantEditor::OnMenuOpenEditor(wxCommandEvent &Event)
{
	OpenSelectedParentEditor();
}

/** Double click handler for the inheritance list. */
void WxMaterialInstanceConstantEditor::OnInheritanceListDoubleClick(wxListEvent &ListEvent)
{
	OpenSelectedParentEditor();
}

/** Event handler for when the user right clicks on the inheritance list. */
void WxMaterialInstanceConstantEditor::OnInheritanceListRightClick(wxListEvent &ListEvent)
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

			ContextMenu->Append(ID_MATERIALINSTANCE_CONSTANT_EDITOR_OPENEDITOR, *Label);
		}

		ContextMenu->Append(ID_MATERIALINSTANCE_CONSTANT_EDITOR_SYNCTOGB, *LocalizeUnrealEd("SyncGenericBrowser"));

		PopupMenu(ContextMenu);
	}
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxMaterialInstanceConstantEditor::GetDockingParentName() const
{
	return TEXT("MaterialInstanceEditor");
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxMaterialInstanceConstantEditor::GetDockingParentVersion() const
{
	return 1;
}

/** Event handler for when the user wants to toggle showing all material parameters. */
void WxMaterialInstanceConstantEditor::OnShowAllMaterialParameters(wxCommandEvent &Event)
{
	PropertyWindow->PostRebuild();
}
